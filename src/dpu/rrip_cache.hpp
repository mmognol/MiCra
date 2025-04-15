#ifndef AF0CA521_5AC5_45EB_A646_463406E46EB0
#define AF0CA521_5AC5_45EB_A646_463406E46EB0

#include "syslib/mram.hpp"
#include "wram_aligned.hpp"



// DPU have 64MB of MRAM, the minimal pointer size is 26 bits.
// DPU have 16 hardware threads, and a WRAM of 64KB.
// lets imagine a 32 Bytes cache line, and 8 lines per threads.
// This gives us 16*8 = 128 lines, for a total of 4KB of memory used.

// If each threads gets its own cache, they only need to manage 8 lines.


constexpr uint8_t MaxRRPV = 7; // 3 bits for the RRPV
constexpr uint8_t NoLine = 127; // 3 bits for the RRPV

template<uint32_t NumLine, uint32_t LineSize>
    requires (NumLine % 4 == 0)
class RRIPCache
{
    WramAligned<uint64_t, NumLine*LineSize> cache_data;

    uint64_t hits = 0;
    uint64_t misses = 0;

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

    auto ptr_in_line(__mram_ptr uint64_t *ptr, uint32_t i) -> bool
    {
        return LinePtr[i] == (__mram_ptr uint64_t *)((uintptr_t)ptr & ~(uintptr_t)31);
    }

    auto hit(__mram_ptr uint64_t *ptr) -> uint64_t *
    {
        for (uint32_t i = 0; i < NumLine; ++i)
        {
            if (ptr_in_line(ptr, i))
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

    void increase_RRPV()
    {
        for(uint32_t i = 0; i < NumLine; ++i)
            RRPV[i] += 1;
    }
    
    auto next_max_RRPV() -> uint32_t
    {
        while(true)
        {
            increase_RRPV();

            for (uint32_t i = 0; i < NumLine; ++i)
            {
                if (RRPV[i] == MaxRRPV)
                    return i;
            }
        }

    }

    void evict_line(uint32_t i)
    {
        mram_write<LineSize*sizeof(uint64_t)>(&cache_data[i * LineSize], LinePtr[i]);
    }

    void insert_line(uint32_t i, __mram_ptr uint64_t *ptr)
    {
        LinePtr[i] = ptr;
        RRPV[i] = MaxRRPV-2;
        mram_read<LineSize*sizeof(uint64_t)>(ptr, &cache_data[i * LineSize]);
    }

    auto get_new_line() -> uint32_t
    {
        uint32_t i = max_RRPV_index();

        if(i != NoLine)
            return i;

        return next_max_RRPV();
    }

    auto get_line_ptr(__mram_ptr uint64_t *ptr) -> uint64_t *
    {
        ptr = (__mram_ptr uint64_t *)((uintptr_t)ptr & ~(uintptr_t)31);

        if(uint64_t * res = hit(ptr); res != nullptr)
            return res;

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
        uintptr_t offset = (uintptr_t)ptr & (uintptr_t)31;
        return *(line + offset / sizeof(uint64_t));
    }

    auto set_value(__mram_ptr uint64_t *ptr, uint64_t value) -> void
    {
        auto line = get_line_ptr(ptr);
        uintptr_t offset = (uintptr_t)ptr & (uintptr_t)31;
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

    void line_is_hot(__mram_ptr uint64_t *ptr)
    {
        for (uint32_t i = 0; i < NumLine; ++i)
        {
            if (ptr_in_line(ptr, i))
            {
                RRPV[i] = 0;
                return;
            }
        }
    }
};

#endif /* AF0CA521_5AC5_45EB_A646_463406E46EB0 */
