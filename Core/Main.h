#pragma once
#include "Memory/Memory.h"
#include "Tasks/Task.h"
#include "Utils/ScopeGuard.h"

task start(int argc, char** argv);

int main(int argc, char** argv) {
   // start memory management
   Memory::instance().start();
   Memory::instance().setMain(reinterpret_cast<void*>(&main));
   ScopeGuard guard([] {
      Memory::instance().stop();
   });

   task root;

   // check for last checkpoint and restore it
   if (std::filesystem::exists("heap_0.bin")) {
      Memory::instance().read("heap.bin");

      for (int i = 0; true; i++) {
         std::filesystem::path path = "heap_" + std::to_string(i) + ".bin";
         if (std::filesystem::exists(path)) {
            std::filesystem::remove(path);
         } else {
            break;
         }
      }

      // restore previous root task
      root = Allocator::instance().header().root->task;
   } else {
      // create new root task
      root = start(argc, argv);
      Allocator::instance().header().root = new TaskNode{.task = root.address(), .children = {}};
   }

   // reset std::cout to default
   std::cout.copyfmt(std::ios(nullptr));

   // run the root task
   while (!root.done()) {
      root.resume();
   }

   // done
}
