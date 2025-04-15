#ifndef B7D76A7E_C31B_40F7_B0E6_B12FACA3076C
#define B7D76A7E_C31B_40F7_B0E6_B12FACA3076C

/**
 * @typedef perfcounter_t
 * @brief A value which can be stored by the performance counter.
*/
using perfcounter_t = int;
 
/**
 * @enum perfcounter_config_t
 * @brief A configuration for the performance counter, defining what should be counted.
 *
 * @var COUNT_SAME          keep the previous configuration
 * @var COUNT_CYCLES        switch to counting clock cycles
 * @var COUNT_INSTRUCTIONS  switch to counting executed instructions
 * @var COUNT_NOTHING       does not count anything
 */
using perfcounter_config_t = enum _perfcounter_config_t {
    COUNT_SAME = 0,
    COUNT_CYCLES = 1,
    COUNT_INSTRUCTIONS = 2,
    COUNT_NOTHING = 3,
};

 
 
auto perfcounter_get() -> perfcounter_t
{
    perfcounter_t value;
    
    asm volatile(
        "time %[VALUE]"
        : [ VALUE ] "=r"(value)
        : /* No input operands */
        : "memory" // Clobbers memory
    );
    
    return value;
}
 
#ifndef DPU_PROFILING

void perfcounter_config(bool instructions = false)
{
    uint32_t config = instructions ? 5 : 3;
    uint32_t tmp = 0;

    asm volatile(
        "time_cfg %[tmp], %[CONFIG]"
        : [ tmp ] "=r"(tmp)
        : [ CONFIG ] "r"(config)
        : "memory" // Clobbers memory
    );
}
 #else
 #define perfcounter_config(config, reset_value)                                                                                  \
     do {                                                                                                                         \
         _Static_assert(0, "[DPU_ERROR]: call to perfcounter_config is incompatible with -pg option.");                           \
     } while (0)
 #endif /* !DPU_PROFILING */
 

#endif /* B7D76A7E_C31B_40F7_B0E6_B12FACA3076C */
