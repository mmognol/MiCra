#ifndef B75BCDFE_F60D_478B_8119_112B5E40C0B1
#define B75BCDFE_F60D_478B_8119_112B5E40C0B1

using sysname_t = unsigned int;

/**
 * @fn me
 * @internal This just returns the value of the special register id.
 * @return The current tasklet's sysname.
 */
static inline auto me() -> sysname_t
{
// Needed for clang-tidy and other tools working without DPU support
#if __has_builtin(__builtin_dpu_tid)
    return __builtin_dpu_tid();
#else
    sysname_t tid;
    asm volatile ("move %[tid], id" : [tid]"=r"(tid));
    return tid;
#endif
}
 

#endif /* B75BCDFE_F60D_478B_8119_112B5E40C0B1 */
