#include "Allocator.h"
#include "Memory.h"
#include <fstream>

Allocator& Allocator::instance() {
   static Allocator instance;
   return instance;
}

void Allocator::init(void* memory, size_t size) {
   memoryStart_ = reinterpret_cast<uint8_t*>(memory);
   totalSize_ = size;
   controlHeader_ = new(memory) ControlHeader();

   assert(size >= sizeof(BlockHeader*) + sizeof(BlockHeader) && "Memory too small for allocator");

   auto* initial = reinterpret_cast<BlockHeader*>(memoryStart_ + sizeof(ControlHeader));
   initial->size = totalSize_ - sizeof(ControlHeader) - sizeof(BlockHeader);
   initial->isFree = true;
   initial->next = nullptr;

   controlHeader_->list = initial;
}

void* Allocator::alloc(size_t size) {
   std::unique_lock lock(mutex_);
   // Align size to 8 bytes
   size = (size + 7) & ~size_t(7);

   BlockHeader* prev = nullptr;
   BlockHeader* current = controlHeader_->list;

   while (current) {
      if (current->isFree && current->size >= size) {
         // Can we split the block?
         if (current->size >= size + sizeof(BlockHeader) + 8) {
            // Split block
            auto* newBlock = reinterpret_cast<BlockHeader*>(
               reinterpret_cast<uint8_t*>(current + 1) + size);

            newBlock->size = current->size - size - sizeof(BlockHeader);
            newBlock->isFree = true;
            newBlock->next = current->next;

            current->size = size;
            current->next = newBlock;
         }

         current->isFree = false;

         return current + 1;
      }

      prev = current;
      current = current->next;
   }

   return nullptr; // No suitable block
}

void Allocator::free(void* ptr) {
   if (!ptr) return;
   std::unique_lock lock(mutex_);

   auto* block = reinterpret_cast<BlockHeader*>(ptr) - 1;

   if (block->isFree) {
      // IO::print("Double free detected\n");
      return;
   }

   block->isFree = true;

   coalesce();
}

bool Allocator::contains(void* ptr) const {
   return ptr >= memoryStart_ && ptr < memoryStart_ + totalSize_;
}

void Allocator::write_sparse(const std::filesystem::path& path) {
   Memory::instance().pause(); {
      size_t currentFileSize = 0;
      size_t fileIndex = 0;

      auto openNewStream = [&](std::ofstream& stream) {
         if (stream.is_open()) stream.close();
         std::filesystem::path newPath = path;
         newPath.replace_filename(path.stem().string() + "_" + std::to_string(fileIndex) + path.extension().string());
         stream.open(newPath, std::ios::binary);
         // stream.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
         currentFileSize = 0;
      };

      std::ofstream stream;
      openNewStream(stream);

      auto writeData = [&](const char* data, size_t size) {
         size_t offset = 0;
         while (offset < size) {
            size_t remainingInFile = maxFileSize - currentFileSize;
            size_t chunkSize = std::min(remainingInFile, size - offset);

            if (chunkSize == 0) {
               ++fileIndex;
               openNewStream(stream);
               continue;
            }

            stream.write(data + offset, chunkSize);
            currentFileSize += chunkSize;
            offset += chunkSize;
         }
      };

      // 1. write control header
      writeData(reinterpret_cast<const char*>(controlHeader_), sizeof(ControlHeader));

      // 2. write nodes
      const BlockHeader* current = controlHeader_->list;
      while (current) {
         // 2.1 write block header
         writeData(reinterpret_cast<const char*>(current), sizeof(BlockHeader));

         // 2.2 write block data (if used)
         if (!current->isFree) {
            writeData(reinterpret_cast<const char*>(current + 1), current->size);
         }

         // 2.3 next
         current = current->next;
      }

      stream.flush();
      stream.close();
   }
   Memory::instance().resume();
}

void Allocator::read_sparse(const std::filesystem::path& path) {
   Memory::instance().pause(); {
      size_t fileIndex = 0;
      std::ifstream stream;

      auto openStream = [&]() -> bool {
         if (stream.is_open()) stream.close();
         std::filesystem::path newPath = path;
         newPath.replace_filename(path.stem().string() + "_" + std::to_string(fileIndex) + path.extension().string());
         if (!std::filesystem::exists(newPath)) return false;
         stream.open(newPath, std::ios::binary);
         // stream.rdbuf()->pubsetbuf(nullptr, 0); // disable buffering
         return true;
      };

      if (!openStream()) {
         // File doesn't exist
         return;
      }

      auto readData = [&](char* data, size_t size) -> bool {
         size_t offset = 0;
         while (offset < size) {
            if (stream.eof()) {
               ++fileIndex;
               if (!openStream()) return false; // no more files
            }

            stream.read(data + offset, size - offset);
            size_t readBytes = static_cast<size_t>(stream.gcount());
            if (readBytes == 0 && !stream.eof()) {
               return false; // read error
            }
            offset += readBytes;
         }
         return true;
      };

      // 1. read control header
      if (!readData(reinterpret_cast<char*>(controlHeader_), sizeof(ControlHeader)))
         return;

      // 2. read nodes
      BlockHeader* current = controlHeader_->list;
      while (current) {
         // 2.1 read block header
         if (!readData(reinterpret_cast<char*>(current), sizeof(BlockHeader)))
            break;

         // 2.2 read block data (if used)
         if (!current->isFree) {
            if (!readData(reinterpret_cast<char*>(current + 1), current->size))
               break;
         }

         // 2.3 next
         current = current->next;
      }

      if (stream.is_open())
         stream.close();
   }
   Memory::instance().resume();
}

ControlHeader& Allocator::header() {
   return *controlHeader_;
}

std::recursive_mutex& Allocator::mutex() {
   return mutex_;
}

void Allocator::coalesce() {
   BlockHeader* current = controlHeader_->list;
   while (current && current->next) {
      BlockHeader* next = current->next;

      uint8_t* currentEnd = reinterpret_cast<uint8_t*>(current + 1) + current->size;

      if (current->isFree && next->isFree && reinterpret_cast<uint8_t*>(next) == currentEnd) {
         // Merge
         current->size += sizeof(BlockHeader) + next->size;
         current->next = next->next;
      } else {
         current = next;
      }
   }
}

