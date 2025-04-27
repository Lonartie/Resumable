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
public:
    static Allocator& instance();

    void init(void* memory, size_t size);

    void* alloc(size_t size);
    void free(void* ptr);

    bool contains(void* ptr) const;

    void write_sparse(std::filesystem::path path);
    void read_sparse(std::filesystem::path path);

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
