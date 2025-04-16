#include "../../../src/dpu/rrip4_cache.hpp"
#include "../../../src/dpu/syslib/defs.hpp"

#include "../../../src/dpu/syslib/perfcounter.hpp"


#ifndef NR_TASKLETS
#define NR_TASKLETS 16
#endif

using array_type = uint32_t;

constexpr uint32_t ARRAY_SIZE = 500'000;

__mram uint64_t decalage; // NOLINT(modernize-avoid-c-arrays)


__mram array_type arrays[NR_TASKLETS][ARRAY_SIZE]; // NOLINT(modernize-avoid-c-arrays)
__mram array_type to_add[NR_TASKLETS][4]; // NOLINT(modernize-avoid-c-arrays)


__host uint64_t hits[NR_TASKLETS]; // NOLINT(modernize-avoid-c-arrays)
__host uint64_t misses[NR_TASKLETS]; // NOLINT(modernize-avoid-c-arrays)

__host uint64_t perfcount[NR_TASKLETS]; // NOLINT(modernize-avoid-c-arrays)

__host uint32_t ltouch[NR_TASKLETS][8]; // NOLINT(modernize-avoid-c-arrays)

auto main() -> int
{
    perfcounter_config(true);

    auto *my_array = arrays[me()];
    auto *my_to_add = to_add[me()];

    constexpr uint32_t NbLines = 8;
    RRIPCache<8> my_cache;

    array_type local_add[4] = {1, 1, 1, 8};

    my_cache.set_value(my_to_add, array_type(1));
    my_cache.set_value(my_to_add+1, array_type(1));
    my_cache.set_value(my_to_add+2, array_type(1));
    my_cache.set_value(my_to_add+3, array_type(8));

    for(uint32_t i = 0; i < ARRAY_SIZE; i+=4)
    {
        my_cache.set_value(my_array+i, local_add[0] +(i/4U));
        my_cache.set_value(my_array+i+1, local_add[1] +(i/4U));
        my_cache.set_value(my_array+i+2, local_add[2] +(i/4U));
        my_cache.set_value(my_array+i+3, local_add[3] +(i/4U));

        /*my_cache.set_value(my_array+i, my_cache.get_value(my_to_add)+(i/4));
        my_cache.set_value(my_array+i+1, my_cache.get_value(my_to_add+1)+(i/4));
        my_cache.set_value(my_array+i+2, my_cache.get_value(my_to_add+2)+(i/4));
        my_cache.set_value(my_array+i+3, my_cache.get_value(my_to_add+3)+(i/4));*/
    }

    hits[me()] = my_cache.get_hits();
    misses[me()] = my_cache.get_misses();

    for(uint32_t i = 0; i < 8; ++i)
    {
        ltouch[me()][i] = my_cache.get_line_touch(i);
    }

    perfcount[me()] = perfcounter_get();

    return 0;
}