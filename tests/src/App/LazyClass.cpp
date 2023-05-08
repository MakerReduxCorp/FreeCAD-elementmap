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

// Macro used to check the current number of allocations and size allocated so far.
#define CHECK_MEM(_count, _size)                                                                   \
    {                                                                                              \
        MemStats s = memStats;                                                                     \
        EXPECT_EQ(s.count, _count);                                                                \
        EXPECT_EQ(s.size, _size);                                                                  \
        memStats = s;                                                                              \
    }


// Code adapted from
// https://stackoverflow.com/questions/438515/how-to-track-memory-allocations-in-c-especially-new-delete

// Similar implementation:
// http://wyw.dcweb.cn/leakage.htm
// https://github.com/adah1972/nvwa/blob/master/nvwa/debug_new.h
// https://github.com/adah1972/nvwa/blob/master/nvwa/debug_new.cpp

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


typedef Lazy<std::string> LazyString;

TEST(LazyClass, basicAllocTest)
{
    resetMemStats();

    int* ptr = new int[5];
    CHECK_MEM(1, 5 * sizeof(int));
    delete[] ptr;
    CHECK_MEM(0, 0);
}

TEST(LazyClass, stringAllocTest)
{
    resetMemStats();
    {
        // Must be longer than 15 otherwise std::string won't allocate on heap
        std::string test = "TESTTESTTESTTEST";
        CHECK_MEM(1, 17);
    }
    CHECK_MEM(0, 0);
}

TEST(LazyClass, stringCopy)
{
    resetMemStats();

    // Must be longer than 15 otherwise std::string won't allocate on heap
    std::string test = "TESTTESTTESTTEST";
    CHECK_MEM(1, 17);
    std::string test2 = test;
    CHECK_MEM(2, 34);
    std::string test3 = test;
    CHECK_MEM(3, 51);
}

TEST(LazyClass, lazyStringCopy)
{
    resetMemStats();

    LazyString test("TEST");
    // sizeof("TEST") = 5  ==>  string will not allocate on heap
    // 1 allocation by shared_ptr of 48
    CHECK_MEM(1, 48);

    // copies are stack allocated: operator new not called.
    LazyString test2 = test;
    CHECK_MEM(1, 48);
    EXPECT_EQ(test2.hasLocalCopy(), false);

    LazyString test3 = test;
    CHECK_MEM(1, 48);
    EXPECT_EQ(test3.hasLocalCopy(), false);

    // create a copy: another shared_ptr allocation
    test2.createLocalCopy();
    CHECK_MEM(1 + 1, 48 + 48);
    EXPECT_EQ(test2.hasLocalCopy(), true);

    // access object without modification: no new allocations
    EXPECT_EQ(test3.asConst().size(), 4);
    CHECK_MEM(2, 96);
    EXPECT_EQ(test3.hasLocalCopy(), false);

    // modify object: copy gets created, another shared_ptr allocation
    test3.asMutable() += "ANOTHER";
    CHECK_MEM(2 + 1, 96 + 48);
    EXPECT_EQ(test3.hasLocalCopy(), true);
}


TEST(LazyClass, lazyStringCopyLongString)
{
    resetMemStats();

    // Must be longer than 15 otherwise std::string won't allocate on heap
    LazyString test("TESTTESTTESTTEST");
    // sizeof("TESTTESTTESTTEST") = 17
    // allocated by shared_ptr = 48
    // 65 = 17 + 48
    CHECK_MEM(2, 65);

    // stack allocated: operator new not called.
    LazyString test2 = test;
    CHECK_MEM(2, 65);
    EXPECT_EQ(test2.hasLocalCopy(), false);

    LazyString test3 = test;
    CHECK_MEM(2, 65);
    EXPECT_EQ(test3.hasLocalCopy(), false);

    // create a copy: new string gets allocated, with consequent shared_ptr allocation
    test2.createLocalCopy();
    CHECK_MEM(2 + 2, 65 + 65);
    EXPECT_EQ(test2.hasLocalCopy(), true);

    // access object without modification: no new allocations
    EXPECT_EQ(test3.asConst().size(), 16);
    CHECK_MEM(4, 130);
    EXPECT_EQ(test3.hasLocalCopy(), false);

    // modify object: copy gets created, causes string allocation of
    // 33 and dealloc of previous 17 to expand storage
    test3.asMutable() += "ANOTHER";
    CHECK_MEM(4 + 2, 130 + (65 + 33 - 17));
    EXPECT_EQ(test3.hasLocalCopy(), true);
}


// TODO complex test with LazyMappedName<LazyString>
