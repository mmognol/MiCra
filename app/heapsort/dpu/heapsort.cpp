
#include "../../../src/dpu/micra.hpp"
#include "../../../src/dpu/mram_cache.hpp"

#include "../../../src/dpu/rrip8_cache.hpp"

#define ARR_SIZE 524288UL
constexpr uint32_t RripCacheSize = 8;

constexpr int32_t CACHE_SIZE = 4;

constexpr uint32_t NUM_PARTITIONS = 16;
__mram_noinit int32_t g_arr[NUM_PARTITIONS * ARR_SIZE]; // NOLINT(modernize-avoid-c-arrays)

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

inline auto iParent(uint32_t i) -> uint32_t
{
    return i / 2;
}

inline auto iLeftChild(uint32_t i) -> uint32_t
{
    return 2 * i + 1;
}

inline auto iRightChild(uint32_t i) -> uint32_t
{
    return 2 * i + 2;
}

inline void siftDown(RRIPCache<RripCacheSize>& cache, span arr, uint32_t start, uint32_t end)
{
    auto root = start;

    while (iLeftChild(root) <= end)
    {
        auto child = iLeftChild(root);
        auto next = root;

        if ( cache.get_value(arr.ptr +next) <   cache.get_value(arr.ptr + child) )
            next = child;

        if (child + 1 <= end && cache.get_value(arr.ptr +next) < cache.get_value(arr.ptr + child +1))
            next = child + 1;

        if (next == root)
            return;

        auto tmp = cache.get_value(arr.ptr + root);
        cache.set_value(arr.ptr + root, cache.get_value(arr.ptr + next));
        cache.set_value(arr.ptr + next, tmp);

        root = next;
    }
}

inline void heapify(RRIPCache<RripCacheSize>& cache, span a)
{
    int start = static_cast<int>(iParent(a.size - 1));

    while (start >= 0)
    {
        siftDown(cache, a, start, a.size - 1);
        start--;
    }
}

inline void heapSort(span a)
{
    RRIPCache<RripCacheSize> rrip_cache;

    heapify(rrip_cache, a);

    auto end = a.size - 1;

    while (end > 0)
    {
        auto tmp = rrip_cache.get_value(a.ptr + end);
        rrip_cache.set_value(a.ptr + end, rrip_cache.get_value(a.ptr));
        rrip_cache.set_value(a.ptr, tmp);
        // cache.swap(a.ptr + end, a.ptr);

        end--;
        siftDown(rrip_cache, a, 0, end);
    }
}

__mram uint64_t perf = 0;

auto main() -> int
{
    heapSort((struct span){.ptr = g_arr + me() * ARR_SIZE, .size = ARR_SIZE});

    return 0;
}