#pragma once
#include <vector>

struct BlockHeader {
   size_t size = 0;
   bool isFree = true;
   BlockHeader* next = nullptr;
};

struct TaskNode {
   void* task = nullptr;
   std::vector<TaskNode*> children;
};

void doNotOptimize(void* data);

struct ControlHeader {
   BlockHeader* list = nullptr;
   TaskNode* root = nullptr;

   void transform_write(void* main);
   void transform_read(void* main);
};