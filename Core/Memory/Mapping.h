#include <cstddef>

void* align_up(void* ptr, size_t align);

void* allocate_at(void* desired_addr, size_t size = 2ULL * 1024 * 1024 * 1024);

void free_at(void* addr, size_t size);
/// @brief Maps a file into memory for read and write access.
/// @param path Path to the file
/// @param size Size to allocate (ignored if file is already large enough)
/// @param out_fd Pointer to an int where the file descriptor will be stored (can be NULL)
/// @return Pointer to mapped memory, or NULL on failure.
void* map_file(const char* path, void* target, size_t size, int* out_fd);

/// @brief Unmaps memory-mapped file and optionally closes the file descriptor.
/// @param addr Pointer to memory returned by mmap
/// @param size Size of the mapped region
/// @param fd File descriptor to close (-1 to skip)
/// @return 0 on success, -1 on failure
int unmap_file(void* addr, size_t size, int fd);

