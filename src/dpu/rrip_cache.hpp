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
class RRIPCache
{
    WramAligned<uint64_t, NumLine*LineSize> cache_data;

    uint64_t hits = 0;
    uint64_t misses = 0;

    uint8_t RRPV[NumLine]{}; // 3 bits for the RRPV NOLINT(modernize-avoid-c-arrays)
    __mram_ptr uint64_t* LinePtr[NumLine]{}; // NOLINT(modernize-avoid-c-arrays)
    __aligned(8) uint8_t LineLabel[NumLine]{}; // NOLINT(modernize-avoid-c-arrays)

    // Cannot use nullptr because it is a valid address in MRAM
    __mram_ptr uint64_t* nulline = (__mram_ptr uint64_t*)0xFFFFFFFFFFFFFFFFULL; // NOLINT(cppcoreguidelines-avoid-magic-numbers,readability-magic-numbers)

public:

    RRIPCache()
    {
        for (uint32_t i = 0; i < NumLine; ++i)
        {
            LinePtr[i] = nulline;
            LineLabel[i] = 0;
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

    auto get_label(__mram_ptr uint64_t *ptr) -> uint8_t
    {
        return static_cast<uint8_t>(reinterpret_cast<uintptr_t>(ptr) >> 18); // NOLINT(bad_reinterpret_cast_small_int)
    }

    auto ptr_in_line(__mram_ptr uint64_t *ptr, uint32_t i) -> bool
    {
        return LinePtr[i] == (__mram_ptr uint64_t *)((uintptr_t)ptr & ~(uintptr_t)31);
    }

    auto hit(__mram_ptr uint64_t *ptr) -> uint64_t *
    {
        auto label = get_label(ptr);

        for (uint32_t i = 0; i < NumLine; ++i)
        {
            if (LineLabel[i] == label && ptr_in_line(ptr, i))
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

    void increment_RRPV()
    {
        for (uint32_t i = 0; i < NumLine; ++i)
            RRPV[i] += 1;
    }

    void evict_line(uint32_t i)
    {
        mram_write<LineSize*sizeof(uint64_t)>(&cache_data[i * LineSize], LinePtr[i]);
    }

    void insert_line(uint32_t i, __mram_ptr uint64_t *ptr)
    {
        LinePtr[i] = ptr;
        LineLabel[i] = get_label(ptr);
        RRPV[i] = MaxRRPV;
        mram_read<LineSize*sizeof(uint64_t)>(ptr, &cache_data[i * LineSize]);
    }

    auto get_new_line() -> uint32_t
    {
        uint32_t i = max_RRPV_index();

        if(i != NoLine)
            return i;

        while(i == NoLine)
        {
            increment_RRPV();
            i = max_RRPV_index();
        }

        return i;
    }

    auto get_line_ptr(__mram_ptr uint64_t *ptr) -> uint64_t *
    {
        if(uint64_t * res = hit(ptr); res != nullptr)
            return res;

        misses++;

        auto line = get_new_line();
        if(LinePtr[line] != nulline)
            evict_line(line);
        ptr = (__mram_ptr uint64_t *)((uintptr_t)ptr & ~(uintptr_t)31);
        insert_line(line, ptr);
        return &cache_data[line * LineSize];
    }

    auto get_value(__mram_ptr uint64_t *ptr) -> uint64_t
    {
        uintptr_t offset = (uintptr_t)ptr & (uintptr_t)31;
        auto line = get_line_ptr((__mram_ptr uint64_t *)ptr);
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
};

#endif /* AF0CA521_5AC5_45EB_A646_463406E46EB0 */
