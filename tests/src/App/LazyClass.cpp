// SPDX-License-Identifier: LGPL-2.1-or-later

#include "gtest/gtest.h"

#include "App/LazyClass.hpp"

#include <iostream>
#include <map>
#include <string>


struct MemStats
{
    unsigned int count = 0;
    size_t size = 0;
};

static MemStats memStats;

void resetMemStats()
{
    memStats.count = 0;
    memStats.size = 0;
}

#define CHECK_MEM(_count, _size)                                                                   \
    {                                                                                              \
        MemStats s = memStats;                                                                     \
        EXPECT_EQ(s.count, _count);                                                                \
        EXPECT_EQ(s.size, _size);                                                                  \
        memStats = s;                                                                              \
    }


// Code adapted from
// https://stackoverflow.com/questions/438515/how-to-track-memory-allocations-in-c-especially-new-delete

template<typename T>
struct AllocatorMalloc: std::allocator<T>
{
    typedef typename std::allocator<T>::pointer pointer;
    typedef typename std::allocator<T>::size_type size_type;

    template<typename U>
    struct rebind
    {
        typedef AllocatorMalloc<U> other;
    };

    AllocatorMalloc()
    {}

    template<typename U>
    AllocatorMalloc(AllocatorMalloc<U> const& u)
        : std::allocator<T>(u)
    {}

    pointer allocate(size_type size, std::allocator<void>::const_pointer = 0)
    {
        void* p = std::malloc(size * sizeof(T));
        if (p == 0) {
            throw std::bad_alloc();
        }
        return static_cast<pointer>(p);
    }

    void deallocate(pointer p, size_type)
    {
        std::free(p);
    }
};

typedef std::map<void*, std::size_t, std::less<void*>,
                 AllocatorMalloc<std::pair<void* const, std::size_t>>>
    addr_map_t;

addr_map_t* get_map()
{
    // don't use normal new to avoid infinite recursion.
    static addr_map_t* addr_map = new (std::malloc(sizeof *addr_map)) addr_map_t;
    return addr_map;
}

void* operator new(std::size_t size) throw()
{
    // we are required to return non-null
    void* mem = std::malloc(size == 0 ? 1 : size);
    if (mem == 0) {
        throw std::bad_alloc();
    }
    (*get_map())[mem] = size;
    memStats.count++;
    memStats.size += size;
    return mem;
}

void operator delete(void* mem) throw()
{
    memStats.count--;
    memStats.size -= (*get_map())[mem];
    if (get_map()->erase(mem) == 0) {
        std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}

void operator delete(void* mem, std::size_t size) throw()
{
    size_t mapSize = (*get_map())[mem];
    memStats.count--;
    memStats.size -= mapSize;
    if (mapSize != size) {
        std::cerr << "bug: memory at " << mem << " delete operator called with mismatching sizes\n";
    }
    if (get_map()->erase(mem) == 0) {
        // this indicates a serious bug
        std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}

void* operator new[](std::size_t size) throw()
{
    // we are required to return non-null
    void* mem = std::malloc(size == 0 ? 1 : size);
    if (mem == 0) {
        throw std::bad_alloc();
    }
    (*get_map())[mem] = size;
    memStats.count++;
    memStats.size += size;
    return mem;
}

void operator delete[](void* mem) throw()
{
    memStats.count--;
    memStats.size -= (*get_map())[mem];
    if (get_map()->erase(mem) == 0) {
        // this indicates a serious bug
        std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}

void operator delete[](void* mem, std::size_t size) throw()
{
    size_t mapSize = (*get_map())[mem];
    memStats.count--;
    memStats.size -= mapSize;
    if (mapSize != size) {
        std::cerr << "bug: memory at " << mem << " delete operator called with mismatching sizes\n";
    }
    if (get_map()->erase(mem) == 0) {
        // this indicates a serious bug
        std::cerr << "bug: memory at " << mem << " wasn't allocated by us\n";
    }
    std::free(mem);
}

/*---------------------------------------------------------------------------------------------------------------*/


TEST(LazyClass, basicAllocTest)
{
    resetMemStats();

    int* ptr = new int[5];
    CHECK_MEM(1, 5 * sizeof(int));
    delete[] ptr;
    CHECK_MEM(0, 0);
}
