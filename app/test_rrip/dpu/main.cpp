#include "../../../src/dpu/rrip_cache.hpp"
#include "../../../src/dpu/syslib/defs.hpp"

#include "../../../src/dpu/syslib/perfcounter.hpp"

#ifndef NR_TASKLETS
#define NR_TASKLETS 16
#endif

constexpr uint32_t ARRAY_SIZE = 500'000;

__mram uint64_t decalage; // NOLINT(modernize-avoid-c-arrays)


__mram uint64_t arrays[NR_TASKLETS][ARRAY_SIZE]; // NOLINT(modernize-avoid-c-arrays)
__mram uint64_t to_add[NR_TASKLETS][4]; // NOLINT(modernize-avoid-c-arrays)


__host uint64_t hits[NR_TASKLETS]; // NOLINT(modernize-avoid-c-arrays)
__host uint64_t misses[NR_TASKLETS]; // NOLINT(modernize-avoid-c-arrays)

__host uint64_t perfcount[NR_TASKLETS]; // NOLINT(modernize-avoid-c-arrays)


auto main() -> int
{
    auto *my_array = arrays[me()];
    auto *my_to_add = to_add[me()];

    constexpr uint32_t NbLines = 8;
    RRIPCache<NbLines, 4> my_cache;

    my_cache.set_value(my_to_add, 1);
    my_cache.set_value(my_to_add+1, 1);
    my_cache.set_value(my_to_add+2, 1);
    my_cache.set_value(my_to_add+3, 8);

    for(uint32_t i = 0; i < ARRAY_SIZE; i+=4)
    {
        my_cache.set_value(my_array+i, my_cache.get_value(my_to_add)+(i/4));
        my_cache.set_value(my_array+i+1, my_cache.get_value(my_to_add+1)+(i/4));
        my_cache.set_value(my_array+i+2, my_cache.get_value(my_to_add+2)+(i/4));
        my_cache.set_value(my_array+i+3, my_cache.get_value(my_to_add+3)+(i/4));
        my_cache.line_is_hot(my_to_add);
    }

    hits[me()] = my_cache.get_hits();
    misses[me()] = my_cache.get_misses();

    return 0;
}