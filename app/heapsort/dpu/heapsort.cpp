extern "C"
{
#define _Bool bool
#include <defs.h>
#include <perfcounter.h>
#include <profiling.h>
}

#include "../../src/dpu/mram_cache.hpp"
// #include "MramDoubleCache.hpp"

#define ARR_SIZE 524288UL

constexpr int32_t CACHE_SIZE = 4;

PROFILING_INIT(heap_prof);
PROFILING_INIT(sift_prof);

__mram_noinit int32_t g_arr[16 * ARR_SIZE];

struct span
{
    __mram_ptr int32_t *ptr;
    uint32_t size;
};

template <typename T = int>
struct Range
{
    T start;
    T end;
};

inline uint32_t iParent(uint32_t i)
{
    return i / 2;
}

inline uint32_t iLeftChild(uint32_t i)
{
    return 2 * i + 1;
}

inline uint32_t iRightChild(uint32_t i)
{
    return 2 * i + 2;
}

inline void siftDown(MramCache<int32_t, CACHE_SIZE> &a, uint32_t start, uint32_t end)
{
    auto root = start;

    while (iLeftChild(root) <= end)
    {
        auto child = iLeftChild(root);
        auto next = root;

        if (a[next] < a[child])
            next = child;

        if (child + 1 <= end && a[next] < a[child + 1])
            next = child + 1;

        if (next == root)
            return;

        a.swap(root, next);
        root = next;
    }
}

inline void heapify(MramCache<int32_t, CACHE_SIZE> &arr, uint32_t size)
{
    int start = static_cast<int>(iParent(size - 1));

    while (start >= 0)
    {
        siftDown(arr, start, size - 1);
        start--;
    }
}

inline void heapSort(span a)
{
    MramCache<int32_t, CACHE_SIZE> cache{a.ptr};

    heapify(cache, a.size);

    auto end = a.size - 1;

    while (end > 0)
    {
        profiling_start(&heap_prof);
        cache.swap(end, 0);
        profiling_stop(&heap_prof);

        end--;
        profiling_start(&sift_prof);
        siftDown(cache, 0, end);
        profiling_stop(&sift_prof);
    }

    cache.push();
}

__mram uint64_t perf = 0;

int main()
{
    if (me() == 0)
        perfcounter_config(COUNT_INSTRUCTIONS, true);

    heapSort((struct span){.ptr = g_arr + me() * ARR_SIZE, .size = ARR_SIZE});

    if (me() == 0)
        perf = perfcounter_get();

    return 0;
}