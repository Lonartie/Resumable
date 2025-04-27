#pragma once

#include <cstdint>
#include <filesystem>
#include "Allocator.h"

class Memory {
public:
   static Memory& instance();

   void init();

   void* allocate(size_t size);
   void deallocate(void* ptr);

   void write(std::filesystem::path const& path);
   void read(std::filesystem::path const& path);

   void* from() const;
   void* to() const;

   void start();
   void stop();

   void pause();
   void resume();

   void setMain(void* main);

private:
   Memory();
   ~Memory();

   static constexpr size_t MB_FACTOR = 1024 * 1024;
   static constexpr size_t GB_FACTOR = 1024 * MB_FACTOR;
   static inline uint8_t* target_heap = reinterpret_cast<uint8_t*>(0x602020000000);
   static inline size_t m_size = 1 * GB_FACTOR;
   uint8_t* m_heap = nullptr;
   bool m_tunnel = false;
   int fd;
   void* m_main = nullptr;
};

#define OVERWRITE
#ifdef OVERWRITE

void* operator new(size_t size);
void* operator new[](size_t size);
void* operator new(size_t size, std::align_val_t alignment);
void* operator new[](size_t size, std::align_val_t alignment);
void operator delete(void* ptr) noexcept;
void operator delete[](void* ptr) noexcept;
void operator delete(void* ptr, std::align_val_t alignment) noexcept;
void operator delete[](void* ptr, std::align_val_t alignment) noexcept;

#endif