#ifndef B9836319_6265_4644_8B8E_31AF30E16F3A
#define B9836319_6265_4644_8B8E_31AF30E16F3A

extern "C"
{
#include <mram.h>
}

#include "wram_aligned.hpp"

#ifndef NR_TASKLETS
#define NR_TASKLETS 16
#endif

#ifndef NR_THREADS
#define NR_THREADS NR_TASKLETS
#endif

template <typename T, unsigned int S>
struct MramCache
{
    WramAligned<T, S> cache;
    __mram_ptr T *mram_ptr = nullptr;
    int block_id = -1;

    inline explicit MramCache(__mram_ptr T *ptr) : mram_ptr(ptr) {}

    inline T &operator[](uint32_t i)
    {
        int id = i / S;
        int idx = i % S;

        if (id != block_id)
        {
            auto *ptr = mram_ptr;
            if (block_id != -1)
                mram_write(cache.buffer, ptr + block_id * S, S * sizeof(T));
            mram_read(ptr + id * S, cache.buffer, S * sizeof(T));
            block_id = id;
        }

        return cache[idx];
    }

    inline void push()
    {
        if (block_id != -1)
            mram_write(cache.buffer, mram_ptr + block_id * S, S * sizeof(T));
    }

    inline void pull()
    {
        if (block_id != -1)
            mram_read(mram_ptr + block_id * S, cache.buffer, S * sizeof(T));
    }

    inline void swap(uint32_t i, uint32_t j) noexcept
    {
        int id = i / S;
        int jd = j / S;

        int idx = i % S;
        int jdx = j % S;

        if (id == block_id)
        {
            if (jd == block_id)
            {
                T tmp = cache[idx];
                cache[idx] = cache[jdx];
                cache[jdx] = tmp;
            }
            else
            {
                T tmp = cache[idx];
                cache[idx] = mram_ptr[j];
                mram_ptr[j] = tmp;
            }
        }
        else if (jd == block_id)
        {
            T tmp = cache[jdx];
            cache[jdx] = mram_ptr[i];
            mram_ptr[i] = tmp;
        }
        else
        {
            T tmp = operator[](i);

            if (jd == block_id)
            {
                cache[idx] = cache[jdx];
                cache[jdx] = tmp;
            }
            else
            {
                cache[idx] = mram_ptr[j];
                mram_ptr[j] = tmp;
            }
        }
    }
};

#endif /* B9836319_6265_4644_8B8E_31AF30E16F3A */
