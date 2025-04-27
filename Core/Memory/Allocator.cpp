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
      std::ofstream stream(path);
      // disable buffering
      stream.rdbuf()->pubsetbuf(nullptr, 0);

      // 1. write control header
      stream.write(reinterpret_cast<const char*>(controlHeader_), sizeof(ControlHeader));

      // 2. write nodes
      const auto* current = controlHeader_->list;
      while (current) {
         // 2.1 write block header
         stream.write(reinterpret_cast<const char*>(current), sizeof(BlockHeader));

         // 2.2 write block data (if used)
         if (!current->isFree) {
            stream.write(reinterpret_cast<const char*>(current + 1), current->size);
         }

         // 2.3 go to next block
         current = current->next;
      }

      stream.flush();
      stream.close();
   }
   Memory::instance().resume();
}

void Allocator::read_sparse(const std::filesystem::path& path) {
   Memory::instance().pause(); {
      std::ifstream stream(path);
      // disable buffering
      stream.rdbuf()->pubsetbuf(nullptr, 0);

      // 1. read control header
      stream.read(reinterpret_cast<char*>(controlHeader_), sizeof(ControlHeader));

      // 2. read nodes
      auto* current = controlHeader_->list;
      while (current) {
         // 2.1 read block header
         stream.read(reinterpret_cast<char*>(current), sizeof(BlockHeader));

         // 2.2 read block data (if used)
         if (!current->isFree) {
            stream.read(reinterpret_cast<char*>(current + 1), current->size);
         }

         // 2.3 go to next block
         current = current->next;
      }

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

