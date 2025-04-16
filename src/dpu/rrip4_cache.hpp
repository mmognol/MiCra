#ifndef AF0CA521_5AC5_45EB_A646_463406E46EB0
#define AF0CA521_5AC5_45EB_A646_463406E46EB0

#include "syslib/mram.hpp"
#include "wram_aligned.hpp"

/*
extern "C"
{
#include <profiling.h>
}

PROFILING_INIT(set_val);
PROFILING_INIT(get_val);
*/

// DPU have 64MB of MRAM, the minimal pointer size is 26 bits.
// DPU have 16 hardware threads, and a WRAM of 64KB.
// lets imagine a 32 Bytes cache line, and 8 lines per threads.
// This gives us 16*8 = 128 lines, for a total of 4KB of memory used.

// If each threads gets its own cache, they only need to manage 8 lines.


constexpr uint8_t MaxRRPV = 8; // 3 bits for the RRPV
constexpr uint8_t NoLine = 127; // 3 bits for the RRPV

constexpr uint32_t NumLine = 4;

template<uint32_t LineSize>
class RRIPCache
{
    WramAligned<uint64_t, NumLine*LineSize> cache_data;

    static constexpr uint32_t Mask = LineSize*8 - 1;

    uint32_t hits = 0;
    uint32_t misses = 0;

    uint32_t count_line_touch[NumLine]{}; // NOLINT(modernize-avoid-c-arrays)

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

    auto hit(uintptr_t ptr) -> uintptr_t
    {
        for (uint32_t i = 0; i < NumLine; ++i)
        {
            if ((uintptr_t)LinePtr[i] == ptr)
            {
                hits++;
                RRPV[i] = 0;
                return (uintptr_t)&cache_data[i * LineSize];
            }
        }
        return UINTPTR_MAX;
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

    void increase_RRPV()
    {
        for(unsigned char & i : RRPV)
            i += 1;
    }

    auto next_max_RRPV() -> uint32_t
    {
        uint32_t i = 0;
        uint32_t rrpv = *(uint32_t*)RRPV;
        constexpr uint32_t mask = 0x08080808;

        // Use count leading zeros
        uint32_t cmp = 0;

        asm ("2:"
             "add %[low], %[low], 16843009;"
             "cmpb4 %[cmp], %[low], %[mask], z, 2b;"
             "clz %[cmp], %[cmp];"
             "lsr %[i], %[cmp], 3"
             : [low] "+r"(rrpv), [i] "=r" (i), [cmp] "+r"(cmp)
             : [mask] "r"(mask));


        *(uint32_t*)RRPV = rrpv;
        return i;
    }

    void evict_line(uint32_t i)
    {
        mram_write<LineSize*sizeof(uint64_t)>(&cache_data[i * LineSize], LinePtr[i]);
    }

    void insert_line(uint32_t i, uintptr_t ptr)
    {
        LinePtr[i] = (__mram_ptr uint64_t *) ptr;
        RRPV[i] = MaxRRPV-3;
        mram_read<LineSize*sizeof(uint64_t)>(LinePtr[i], &cache_data[i * LineSize]);
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

    auto get_line_ptr(uintptr_t ptr) -> uintptr_t
    {

        ptr = ptr & ~(uintptr_t)Mask;
        if(uintptr_t res = hit(ptr); res != UINTPTR_MAX){
            return res;
        }

        misses++;
        auto line = get_new_line();

        if(LinePtr[line] != nulline)
            evict_line(line);
        insert_line(line, ptr);
        return (uintptr_t)&cache_data[line * LineSize];
    }

    template<typename T>
    auto remove_ptr_type(__mram_ptr T *ptr) -> uintptr_t
    {
        return (uintptr_t)ptr;
    }

    template<typename T>
    auto get_value(__mram_ptr T *ptr) -> T
    {
        //profiling_start(&get_val);
        auto line = get_line_ptr(remove_ptr_type(ptr));
        uintptr_t offset = (uintptr_t)ptr & (uintptr_t)Mask;
        //profiling_stop(&get_val);
        return *(T*)(line + offset);
    }

    template<typename T>
    auto set_value(__mram_ptr T *ptr, T value) -> void
    {
        //profiling_start(&set_val);
        auto line = get_line_ptr(remove_ptr_type(ptr));
        uintptr_t offset = (uintptr_t)ptr & (uintptr_t)Mask;
        *((T*)(line + offset)) = value;
        //profiling_stop(&set_val);
    }

    auto get_hits() -> uint32_t
    {
        return hits;
    }

    auto get_misses() -> uint32_t
    {
        return misses;
    }
};

#endif /* AF0CA521_5AC5_45EB_A646_463406E46EB0 */
