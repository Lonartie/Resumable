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
   if (std::filesystem::exists("heap.bin")) {
      Memory::instance().read("heap.bin");
      std::filesystem::remove("heap.bin");

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