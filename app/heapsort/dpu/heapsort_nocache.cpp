
#include "../../../src/dpu/micra.hpp"
#include "../../../src/dpu/mram_cache.hpp"

#define ARR_SIZE 524288UL

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
        cache.swap(end, 0);

        end--;
        siftDown(cache, 0, end);
    }

    cache.push();
}

__mram uint64_t perf = 0;

auto main() -> int
{
    heapSort((struct span){.ptr = g_arr + me() * ARR_SIZE, .size = ARR_SIZE});

    return 0;
}