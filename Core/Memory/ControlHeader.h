#pragma once
#include <vector>

struct BlockHeader {
   size_t size;
   bool isFree;
   BlockHeader* next;
};

struct TaskNode {
   void* task;
   std::vector<TaskNode*> children;
};

void doNotOptimize(void* data);

struct ControlHeader {
   BlockHeader* freeList = nullptr;
   TaskNode* root = nullptr;

   void transform_write(void* main);
   void transform_read(void* main);
};