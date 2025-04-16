#ifndef A3E8434A_BD6F_4C9B_960F_F37CCD57BF10
#define A3E8434A_BD6F_4C9B_960F_F37CCD57BF10
#ifndef AF0CA521_5AC5_45EB_A646_463406E46EB0
#define AF0CA521_5AC5_45EB_A646_463406E46EB0

#include "syslib/mram.hpp"
#include "wram_aligned.hpp"

extern "C"
{
#define _Bool bool
#include <profiling.h>
}

PROFILING_INIT(sec1_prof);
PROFILING_INIT(sec2_prof);

// DPU have 64MB of MRAM, the minimal pointer size is 26 bits.
// DPU have 16 hardware threads, and a WRAM of 64KB.
// lets imagine a 32 Bytes cache line, and 8 lines per threads.
// This gives us 16*8 = 128 lines, for a total of 4KB of memory used.

// If each threads gets its own cache, they only need to manage 8 lines.


constexpr uint8_t MaxRRPV = 8; // 3 bits for the RRPV
constexpr uint8_t NoLine = 127; // 3 bits for the RRPV

constexpr uint32_t NumLine = 8;

template<uint32_t LineSize>
class RRIPCache
{
    WramAligned<uint64_t, NumLine*LineSize> cache_data;

    static constexpr uint32_t Mask = LineSize*8 - 1;

    uint64_t hits = 0;
    uint64_t misses = 0;

    uint32_t count_line_touch[8]{};

    __dma_aligned uint8_t RRPV[NumLine]{}; // 3 bits for the RRPV NOLINT(modernize-avoid-c-arrays)
    __mram_ptr uint64_t* LinePtr[NumLine]{}; // NOLINT(modernize-avoid-c-arrays)

    // Cannot use nullptr because it is a valid address in MRAM
    __mram_ptr uint64_t* nulline = (__mram_ptr uint64_t*)0xFFFFFFFFFFFFFFFFULL; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

public:

    RRIPCache()
    {
        for (uint32_t i = 0; i < NumLine; ++i)
        {
            LinePtr[i] = nulline;
            RRPV[i] = MaxRRPV;
        }
    }

    ~RRIPCache()
    {
        for (uint32_t line = 0; line < NumLine; ++line)
        {
            if(LinePtr[line] != nulline)
                evict_line(line);
        }
    }

    auto get_line_touch(uint32_t i) -> uint32_t
    {
        return count_line_touch[i];
    }

    auto hit(__mram_ptr uint64_t *ptr) -> uint64_t *
    {
        for (uint32_t i = 0; i < NumLine; ++i)
        {
            if (LinePtr[i] == ptr)
            {
                hits++;
                RRPV[i] = 0;
                return &cache_data[i * LineSize];
            }
        }
        return nullptr;
    }

    auto max_RRPV_index() -> uint32_t
    {
        for (uint32_t i = 0; i < NumLine; ++i)
        {
            if (RRPV[i] == MaxRRPV)
            {
                return i;
            }
        }
        return NoLine;
    }

    /*auto max_RRPV_index() -> uint32_t
    {
        uint32_t rrpv_low = *(uint32_t*)RRPV;
        uint32_t rrpv_high = *((uint32_t*)RRPV+1);
        uint32_t mask = 0x08080808;

        // Use count leading zeros
        uint32_t cmp=0;
        uint32_t i = 0;

        asm ("cmpb4 %[cmp], %[low], %[mask], nz, 1f;"
             "cmpb4 %[cmp], %[high], %[mask], z, 2f;"
             "move %[i], 4;"
             "1:"
             "clz %[cmp], %[cmp];"
             "lsr_add %[i], %[i], %[cmp], 3;"
             "jump 3f;"
             "2:"
             "move %[i], 127;" // NoLine
             "3:"
             : [i] "+r" (i), [cmp] "+r"(cmp)
             : [low] "r"(rrpv_low), [high] "r"(rrpv_high), [mask] "r"(mask)
             : "cc");
             
        return i;
    }*/

    void increase_RRPV()
    {
        for(uint32_t i = 0; i < NumLine; ++i)
            RRPV[i] += 1;
    }
    
    /*auto next_max_RRPV() -> uint32_t
    {
        while(true)
        {
            increase_RRPV();

            for (uint32_t i = 0; i < NumLine; ++i)
            {
                //RRPV[i] += 1;
                if (RRPV[i] == MaxRRPV)
                    return i;
            }
        }

    }*/

    auto next_max_RRPV() -> uint32_t
    {
        uint32_t i = 0;
        uint32_t rrpv_low = *(uint32_t*)RRPV;
        uint32_t rrpv_high = *((uint32_t*)RRPV+1);
        uint32_t mask = 0x08080808;

        // Use count leading zeros
        uint32_t cmp = 0;

        asm ("2:"
             "add %[high], %[high], 16843009;"
             "add %[low], %[low], 16843009;"
             "cmpb4 %[cmp], %[low], %[mask], nz, 1f;"
             "cmpb4 %[cmp], %[high], %[mask], z, 2b;"
             "move %[i], 4;"
             "1:"
             "clz %[cmp], %[cmp];"
             "lsr_add %[i], %[i], %[cmp], 3"
             : [low] "+r"(rrpv_low), [high] "+r"(rrpv_high), [i] "+r" (i), [cmp] "+r"(cmp)
             : [mask] "r"(mask));


        *(uint32_t*)RRPV = rrpv_low;
        *((uint32_t*)RRPV+1) = rrpv_high;
        return i;
    }

    void evict_line(uint32_t i)
    {
        mram_write<LineSize*sizeof(uint64_t)>(&cache_data[i * LineSize], LinePtr[i]);
    }

    void insert_line(uint32_t i, __mram_ptr uint64_t *ptr)
    {
        LinePtr[i] = ptr;
        RRPV[i] = MaxRRPV-3;
        mram_read<LineSize*sizeof(uint64_t)>(ptr, &cache_data[i * LineSize]);
    }

    auto get_new_line() -> uint32_t
    {
        auto i = max_RRPV_index();

        if(i != NoLine)
        {
            count_line_touch[i] += 1;
            return i;
        }

        return next_max_RRPV();
    }

    auto get_line_ptr(__mram_ptr uint64_t *ptr) -> uint64_t *
    {
        profiling_start(&sec1_prof);

        ptr = (__mram_ptr uint64_t *)((uintptr_t)ptr & ~(uintptr_t)Mask);
        if(uint64_t * res = hit(ptr); res != nullptr){
            profiling_stop(&sec1_prof);
            return res;
        }
        profiling_stop(&sec1_prof);


        misses++;
        auto line = get_new_line();

        if(LinePtr[line] != nulline)
            evict_line(line);
        insert_line(line, ptr);
        return &cache_data[line * LineSize];
    }

    auto get_value(__mram_ptr uint64_t *ptr) -> uint64_t
    {
        auto line = get_line_ptr(ptr);
        uintptr_t offset = (uintptr_t)ptr & (uintptr_t)Mask;
        return *(line + offset / sizeof(uint64_t));
    }

    auto set_value(__mram_ptr uint64_t *ptr, uint64_t value) -> void
    {
        auto line = get_line_ptr(ptr);
        uintptr_t offset = (uintptr_t)ptr & (uintptr_t)Mask;
        line[offset / sizeof(uint64_t)] = value;
    }

    auto get_hits() -> uint64_t
    {
        return hits;
    }

    auto get_misses() -> uint64_t
    {
        return misses;
    }
};

#endif /* AF0CA521_5AC5_45EB_A646_463406E46EB0 */


#endif /* A3E8434A_BD6F_4C9B_960F_F37CCD57BF10 */
