#ifndef A9862E92_2CDD_43BE_835F_9C4AFD8D8F01
#define A9862E92_2CDD_43BE_835F_9C4AFD8D8F01

#include "stdint.hpp"

#define DEPRECATED __attribute__((deprecated))

#if __STDC_VERSION__ >= 201112L
#define __NO_RETURN _Noreturn
#else
#define __NO_RETURN
#endif /* __STDC_VERSION__ */

#define __weak __attribute__((weak))
#define __section(s) __attribute__((section(s)))
#define __aligned(a) __attribute__((aligned(a)))
#define __used __attribute__((used))
#define __noinline __attribute__((noinline))
#define __atomic_bit __section(".atomic")
#define __dma_aligned __aligned(8)
#define __keep __used __section(".data.__sys_keep")
#define __host __aligned(8) __used __section(".dpu_host")

// Use this macro at variable definition to place this variable into the section
// .data.immediate_memory and then makes it possible to use this variable
// directly as an immediate into load store instructions (and then avoids the need
// to move the address into a register before): immediate values are 12 signed bits
// large.
#define __lower_data(name) __attribute__((used, section(".data.immediate_memory." name)))

/**
 * @def __mram_ptr
 * @brief An attribute declaring that a pointer is an address in MRAM.
 *
 * A typical usage is: ``unsigned int __mram_ptr * array32 = (unsigned int __mram_ptr *) 0xf000;``
 *
 * Performing a cast between a pointer in MRAM and a pointer in WRAM is not allowed by the compiler.
 *
 */
#define __mram_ptr __attribute__((address_space(255)))
#define __mram __mram_ptr __section(".mram") __dma_aligned __used
#define __mram_noinit __mram_ptr __section(".mram.noinit") __dma_aligned __used
#define __mram_keep __mram_ptr __section(".mram.keep") __dma_aligned __used
#define __mram_noinit_keep __mram_ptr __section(".mram.noinit.keep") __dma_aligned __used

#define DPU_MRAM_HEAP_POINTER ((__mram_ptr void *)(&__sys_used_mram_end))


constexpr uint32_t MAX_MRAM_BYTES = 2048;
constexpr uint32_t MRAM_ALIGNMENT_BYTES = 8;


template <uint32_t N> // Use std::size_t for sizes
[[maybe_unused]] static inline void mram_read(const __mram_ptr void *from, void *to) {
    // --- Compile-time checks using standard static_assert ---
    static_assert(N != 0, "mram_read: N (number of bytes) cannot be 0.");
    static_assert(N <= MAX_MRAM_BYTES, "mram_read: N (number of bytes) cannot exceed 2048.");
    static_assert(N % MRAM_ALIGNMENT_BYTES == 0, "mram_read: N (number of bytes) must be a multiple of 8.");

    // --- Compiler hint (GCC/Clang specific) ---
    // We know the conditions are met because of static_assert,
    // but __builtin_assume can still help the optimizer.
    #if defined(__GNUC__) || defined(__clang__)
    __builtin_assume(N >= MRAM_ALIGNMENT_BYTES && N <= MAX_MRAM_BYTES && N % MRAM_ALIGNMENT_BYTES == 0);
    #endif

    // --- Calculate immediate value for the ldma instruction ---
    // Assumes ldma immediate encodes (number_of_8_byte_blocks - 1)
    // Max N = 2048 => (2048/8)-1 = 256-1 = 255. Fits in uint8_t.
    // Min N = 8    => (8/8)-1 = 1-1 = 0. Fits in uint8_t.
    constexpr auto imm = static_cast<uint8_t>((N >> 3) - 1);

    // --- Inline assembly ---
    asm volatile(
        "ldma %[WRAM], %[MRAM], %[IMM]"
        : /* No output operands */
        : [ WRAM ] "r"(to), [ MRAM ] "r"(from), [ IMM ] "i"(imm)
        : "memory" // Clobbers memory
    );
}

template <uint32_t N> // Use std::size_t for sizes
[[maybe_unused]] static inline void mram_write(const void *from, __mram_ptr void *to) {
    // --- Compile-time checks using standard static_assert ---
    static_assert(N != 0, "mram_read: N (number of bytes) cannot be 0.");
    static_assert(N <= MAX_MRAM_BYTES, "mram_read: N (number of bytes) cannot exceed 2048.");
    static_assert(N % MRAM_ALIGNMENT_BYTES == 0, "mram_read: N (number of bytes) must be a multiple of 8.");

    #if defined(__GNUC__) || defined(__clang__)
    __builtin_assume(N >= MRAM_ALIGNMENT_BYTES && N <= MAX_MRAM_BYTES && N % MRAM_ALIGNMENT_BYTES == 0);
    #endif

    constexpr auto imm = static_cast<uint8_t>((N >> 3) - 1);

    asm volatile(
        "sdma %[WRAM], %[MRAM], %[IMM]"
        : /* No output operands */
        : [ WRAM ] "r"(to), [ MRAM ] "r"(from), [ IMM ] "i"(imm)
        : "memory" // Clobbers memory
    );
}

#endif /* A9862E92_2CDD_43BE_835F_9C4AFD8D8F01 */
