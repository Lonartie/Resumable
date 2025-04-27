#include "ControlHeader.h"

inline void generalize(std::byte* state, void* main) {
   int64_t offset_a = *(char**)(state + 0) - (char*)main;
   int64_t offset_b = *(char**)(state + 8) - *(char**)(state + 0);
   *(int64_t*)(state + 0) = offset_a;
   *(int64_t*)(state + 8) = offset_b;
}

inline void specialize(std::byte* state, void* main) {
   int64_t offset_a = *(int64_t*)(state + 0);
   int64_t offset_b = *(int64_t*)(state + 8);
   *(char**)(state + 0) = (char*)main + offset_a;
   *(char**)(state + 8) = *(char**)(state + 0) + offset_b;
}

void doNotOptimize(void* data) {
   (void)(data);
}

void ControlHeader::transform_write(void* main) {
   auto transform = [=](TaskNode* node, auto& self) -> void {
      generalize((std::byte*)node->task, main);

      for (auto* child : node->children) {
         self(child, self);
      }
   };
   if (root != nullptr) {
      transform(root, transform);
   }
}

void ControlHeader::transform_read(void* main) {
   auto transform = [=](TaskNode* node, auto& self) -> void {
      specialize((std::byte*)node->task, main);

      for (auto* child : node->children) {
         self(child, self);
      }
   };
   if (root != nullptr) {
      transform(root, transform);
   }
}
