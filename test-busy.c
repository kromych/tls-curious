#include "lib.c"

#define NUM_THREADS         20

#define FORCE_INLINE __attribute__((always_inline)) inline

u64 busy_wait_forever()
{
    u64 dummy;

#ifdef __amd64
    asm volatile(
        "   xorq    %%rax, %%rax    \n"
        "1:                         \n"
        "   pause                   \n"
        "   incq    %%rax           \n"
        "   jmp     1b              \n"
        : "=a"(dummy)   // "Output"
        :               // No inputs
        :  "memory"     // No clobbered registers
        );
#elif defined(__aarch64__)

    register u64 x0 asm("x0") = 0;

    for (;; ++x0)
    {
        asm volatile (
            "yield"
            : "+r"(x0)  // "Output"
            :           // No inputs
            : "memory"  // No clobbered registers
            );        
    }

    dummy = x0;
#endif

    return dummy;
}

u64 foo(void* param)
{
    return busy_wait_forever(param);
}

u64 bar(void* param)
{
    return foo(param);
}

u64 buzz(void* param)
{
    return bar(param);
}

u64 _start()
{
    void* param = 0;

    for (u64 i = 0; i < NUM_THREADS; ++i)
    {
        create_thread(buzz, param, NULL);
    }

    return busy_wait_forever();
}
