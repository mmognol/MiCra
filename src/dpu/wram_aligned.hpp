#ifndef DE684C30_2F56_47BF_B9EC_71F06675B8A1
#define DE684C30_2F56_47BF_B9EC_71F06675B8A1

#include "syslib/mram.hpp"

template <typename T, unsigned int S>
struct alignas(MRAM_ALIGNMENT_BYTES) WramAligned
{
    T buffer[S]; //NOLINT(modernize-avoid-c-arrays)
    auto operator[](int i) -> T & { return buffer[i]; }
    auto operator[](int i) const -> const T & { return buffer[i]; }

    auto begin() -> T * { return &buffer[0]; }
    auto end() -> T * { return &buffer[S]; }
};

#endif /* DE684C30_2F56_47BF_B9EC_71F06675B8A1 */
