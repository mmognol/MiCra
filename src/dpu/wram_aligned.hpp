#ifndef DE684C30_2F56_47BF_B9EC_71F06675B8A1
#define DE684C30_2F56_47BF_B9EC_71F06675B8A1

template <typename T, unsigned int S>
struct alignas(8) WramAligned
{
    T buffer[S];
    T &operator[](int i) { return buffer[i]; }
    const T &operator[](int i) const { return buffer[i]; }

    T *begin() { return &buffer[0]; }
    T *end() { return &buffer[S]; }
};

#endif /* DE684C30_2F56_47BF_B9EC_71F06675B8A1 */
