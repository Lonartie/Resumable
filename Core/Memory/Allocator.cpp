#include "Allocator.h"

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

   controlHeader_->freeList = initial;
}

void* Allocator::alloc(size_t size) {
   std::unique_lock lock(mutex_);
   // Align size to 8 bytes
   size = (size + 7) & ~size_t(7);

   BlockHeader* prev = nullptr;
   BlockHeader* current = controlHeader_->freeList;

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

         // Update free list head if needed
         if (prev)
            prev->next = current->next;
         else
            controlHeader_->freeList = current->next;

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

   // Insert into front of free list
   block->next = controlHeader_->freeList;
   controlHeader_->freeList = block;

   coalesce();
}

bool Allocator::contains(void* ptr) const {
   return ptr >= memoryStart_ && ptr < memoryStart_ + totalSize_;
}

void Allocator::write_sparse(std::filesystem::path path) {

}

void Allocator::read_sparse(std::filesystem::path path) {
}

ControlHeader& Allocator::header() {
   return *controlHeader_;
}

std::recursive_mutex& Allocator::mutex() {
   return mutex_;
}

void Allocator::coalesce() {
   BlockHeader* current = controlHeader_->freeList;
   while (current) {
      BlockHeader* next = current->next;

      // Self-loop protection
      if (next == current) {
         current->next = nullptr;
         break;
      }

      uint8_t* currentEnd = reinterpret_cast<uint8_t*>(current + 1) + current->size;

      if (next && reinterpret_cast<uint8_t*>(next) == currentEnd && next->isFree) {
         // merge
         current->size += sizeof(BlockHeader) + next->size;
         current->next = next->next;
      } else {
         current = next;
      }
   }
}
