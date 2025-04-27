#pragma once

#include <cstdint>
#include <cstddef>
#include <cassert>
#include <mutex>
#include <new>
#include <__filesystem/path.h>

#include "ControlHeader.h"
#include "IO.h"

class Allocator {
    static constexpr size_t maxFileSize = 512ull * 1024 * 1024; // 512 MiB

public:
    static Allocator& instance();

    void init(void* memory, size_t size);

    void* alloc(size_t size);
    void free(void* ptr);

    bool contains(void* ptr) const;

    void write_sparse(const std::filesystem::path& path);
    void read_sparse(const std::filesystem::path& path);

    ControlHeader& header();

    std::recursive_mutex& mutex();

private:
    Allocator() = default;
    void coalesce();

    uint8_t* memoryStart_;
    size_t totalSize_;
    ControlHeader* controlHeader_;
    mutable std::recursive_mutex mutex_;

};
