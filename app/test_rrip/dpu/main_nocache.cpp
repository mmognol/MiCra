#include "../../../src/dpu/syslib/mram.hpp"
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

__host uint32_t ltouch[NR_TASKLETS][8]; // NOLINT(modernize-avoid-c-arrays)

auto main() -> int
{
    perfcounter_config(true);

    auto *my_array = arrays[me()];
    uint64_t my_to_add [4];

    uint64_t tmp_array[8]; // NOLINT(modernize-avoid-c-arrays)

    my_to_add[0] = 1;
    my_to_add[1] = 1;
    my_to_add[2] = 1;
    my_to_add[3] = 8;

    for(uint32_t i = 0; i < ARRAY_SIZE; i+=4)
    {
        if(i%8 == 0)
        {
            mram_read<8*sizeof(uint64_t)>(my_array+i, tmp_array);
        }

        tmp_array[i%8] = my_to_add[0] + (i/4);
        tmp_array[(i%8)+1] = my_to_add[1] + (i/4);
        tmp_array[(i%8)+2] = my_to_add[2] + (i/4);
        tmp_array[(i%8)+3] = my_to_add[3] + (i/4);
        
        if(i%8 == 4)
        {
            mram_write<8*sizeof(uint64_t)>(tmp_array, my_array+i-4);
        }
    }

    perfcount[me()] = perfcounter_get();

    return 0;
}