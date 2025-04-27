#include "Mapping.h"

#include <sys/mman.h>
#include <unistd.h>
#include <cstdint>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/fcntl.h>
#include <sys/stat.h>

void* align_up(void* const ptr, const size_t align) {
   const uintptr_t p = reinterpret_cast<uintptr_t>(ptr);
   const uintptr_t aligned = (p + align - 1) & ~(align - 1);
   return reinterpret_cast<void*>(aligned);
}

void* allocate_at(void* const desired_addr, const size_t size) {
   const size_t page_size = sysconf(_SC_PAGESIZE);

   void* aligned_addr = align_up(desired_addr, page_size);
   int flags = MAP_PRIVATE | MAP_ANONYMOUS;

   void* result = mmap(aligned_addr, size, PROT_READ | PROT_WRITE, flags, -1, 0);
   if (result == MAP_FAILED || result != aligned_addr) {
      std::cerr << "mmap failed: " << strerror(errno) << "\n";
      return nullptr;
   }

   // Ensure the memory is writable (optional debugging step)
   if (mprotect(result, size, PROT_READ | PROT_WRITE) == -1) {
      std::cerr << "mprotect failed: " << strerror(errno) << "\n";
      munmap(result, size);
      return nullptr;
   }

   return result;
}

void free_at(void* const addr, const size_t size) {
   if (munmap(addr, size) == -1) {
      std::cerr << "munmap failed: " << strerror(errno) << "\n";
   }
}

void* map_file(const char* path, void* target, size_t size, int* out_fd) {
   int fd = open(path, O_RDWR | O_CREAT, 0666);
   if (fd == -1) {
      perror("open");
      return NULL;
   }

   struct stat st;
   if (fstat(fd, &st) == -1) {
      perror("fstat");
      close(fd);
      return NULL;
   }

   if ((size_t)st.st_size < size) {
      if (ftruncate(fd, size) == -1) {
         perror("ftruncate");
         close(fd);
         return NULL;
      }
   } else {
      size = st.st_size;
   }

   void* map = mmap(target, size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
   if (map == MAP_FAILED) {
      perror("mmap");
      close(fd);
      return NULL;
   }

   if (out_fd)
      *out_fd = fd;
   else
      close(fd);  // Close immediately if not needed

   return map;
}

int unmap_file(void* addr, size_t size, int fd) {
   int result = 0;
   if (munmap(addr, size) == -1) {
      perror("munmap");
      result = -1;
   }
   if (fd != -1 && close(fd) == -1) {
      perror("close");
      result = -1;
   }
   return result;
}
