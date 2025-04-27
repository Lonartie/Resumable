#include "Memory.h"

#include <cstdlib>

#include "Mapping.h"
#include <fstream>
#include "IO.h"
#include "../Utils/ScopeGuard.h"

Memory& Memory::instance() {
   static Memory instance;
   return instance;
}

void Memory::init() {
   m_heap = (uint8_t*) allocate_at(target_heap, m_size);
}

void* Memory::allocate(const size_t size) {
   if (!m_tunnel) {
      return malloc(size);
   }
   return Allocator::instance().alloc(size);
}

void Memory::deallocate(void* const ptr) {
   if (!m_tunnel || !Allocator::instance().contains(ptr)) {
      return free(ptr);
   }
   Allocator::instance().free(ptr);
}

void Memory::write(std::filesystem::path const& path) {
   Allocator::instance().header().transform_write(m_main);
   Allocator::instance().write_sparse(path);
   // const size_t fileSize = std::filesystem::file_size(path);
   // printf("Wrote %zu bytes to %s\n", fileSize, path.c_str());
   Allocator::instance().header().transform_read(m_main);
}

void Memory::read(std::filesystem::path const& path) {
   Allocator::instance().read_sparse(path);
   Allocator::instance().header().transform_read(m_main);
}

void* Memory::from() const {
   return m_heap;
}

void* Memory::to() const {
   return m_heap + m_size;
}

void Memory::start() {
   if (m_tunnel) {
      return;
   }
   // m_heap = (uint8_t*)map_file("dump.bin", target_heap, m_size, &fd);
   m_tunnel = true;
   Allocator::instance().init(m_heap, m_size);
}

void Memory::stop() {
   if (!m_tunnel) {
      return;
   }

   unmap_file(m_heap, m_size, fd);
   m_tunnel = false;
   m_heap = nullptr;
}

void Memory::pause() {
   m_tunnel = false;
}

void Memory::resume() {
   m_tunnel = true;
}

void Memory::setMain(void* main) {
   m_main = main;
}

Memory::Memory() {
   // we assume the loader lib already loaded target_heap
   // so we just use the memory the loader gave us
   m_heap = target_heap;
}

Memory::~Memory() {
   if (!m_tunnel) {
      return;
   }
   stop();
}

#ifdef OVERWRITE

void* operator new(const size_t size) {
   return Memory::instance().allocate(size);
}

void* operator new [](const size_t size) {
   return Memory::instance().allocate(size);
}

void* operator new(std::size_t __sz, std::align_val_t) {
   return Memory::instance().allocate(__sz);
}

void* operator new [](std::size_t __sz, std::align_val_t) {
   return Memory::instance().allocate(__sz);
}

void operator delete(void* __p, std::align_val_t) noexcept {
   return Memory::instance().deallocate(__p);
}

void operator delete [](void* __p, std::align_val_t) noexcept {
   return Memory::instance().deallocate(__p);
}

void operator delete(void* ptr) noexcept {
   return Memory::instance().deallocate(ptr);
}

void operator delete [](void* ptr) noexcept {
   return Memory::instance().deallocate(ptr);
}

#endif
