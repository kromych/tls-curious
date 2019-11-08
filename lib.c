#include "lib.h"

#ifdef __amd64

#include "libsyscall.x64.c"

#   define SYS_write       1
#   define SYS_mmap        9
#   define SYS_munmap      11
#   define SYS_clone       56
#   define SYS_exit        60
#   define SYS_wait4       61
#   define SYS_futex       202

#elif defined(__aarch64__)

#include "libsyscall.arm64.c"

#   define SYS_write       64
#   define SYS_mmap        222
#   define SYS_munmap      215
#   define SYS_clone       220
#   define SYS_exit        93
#   define SYS_wait4       260
#   define SYS_futex       98

#else
#   error "Unsupported architecture"
#endif

/************************ ROUTINES ********************************/

u64 sys_mmap(void *addr, u64 length, u64 prot, u64 flags, i64 fd, u64 offset)
{
    return sys_call6(SYS_mmap, (u64)addr, (u64)length, (u64)prot, (u64)flags, (u64)fd, (u64)offset);
}

u64 sys_munmap(void *addr, u64 length)
{
    return sys_call2(SYS_munmap, (u64)addr, (u64)length);
}

u64 sys_clone(u64 flags, void *stack)
{
    return sys_call2(SYS_clone, (u64)flags, (u64)stack);
}

u64 sys_waitpid(u64 pid, u64 *wstatus, u64 options)
{
    u8 rusage[256];

    return sys_call4(SYS_wait4, (u64)pid, (u64)wstatus, (u64)options, (u64)rusage);
}

i64 sys_futex(volatile i32 *uaddr, i64 futex_op, i32 val, const struct timespec *timeout, i32 *uaddr2, i32 val3)
{
    return sys_call6(SYS_futex, (u64)uaddr, (u64)futex_op, (u64)val, (u64)timeout, (u64)uaddr2, (u64)val3);
}

i64 sys_write(u64 fd, const void *buf, u64 count)
{
    return sys_call3(SYS_write, (u64)fd, (u64)buf, (u64)count);
}

void sys_exit(i64 err_code)
{
    sys_call1(SYS_exit, (u64)err_code);
}

u64 strlen(const char* str)
{
    u64 len = 0;

    while (*str++) ++len;

    return len;
}

void print(const char* str)
{
    sys_write(STDOUT_FD, str, strlen(str));
}

void println(void)
{
    static const char *nl = "\r\n";

    sys_write(STDOUT_FD, nl, 2);
}

void u8_to_hex(const u8* byte, char* hex)
{
    {
        u8 low_nibble = *byte & 0x0f;

        switch (low_nibble)
        {
        case 0 ... 9: 
            hex[1] = '0' + low_nibble;
            break;

        case 10 ... 15:
            hex[1] = 'a' + low_nibble - 10;
            break;

        default:
            fatal("Bad low nibble", __LINE__);
        }
    }

    {
        u8 high_nibble = *byte >> 4;

        switch (high_nibble)
        {
        case 0 ... 9: 
            hex[0] = '0' + high_nibble;
            break;

        case 10 ... 15:
            hex[0] = 'a' + high_nibble - 10;
            break;

        default:
            fatal("Bad high nibble", __LINE__);
        }
    }
}

void u16_to_hex(const u8* bytes, char* hex)
{
    u8_to_hex(bytes, hex + 2);
    u8_to_hex(bytes + 1, hex);
}

void print_h64(u64 number)
{
    char hex_str[21];

    for (u64 i = 0; i < sizeof(hex_str)/sizeof(hex_str[0]); ++i)
    {
        hex_str[i] = '0';
    }

    hex_str[1] = 'x';
    hex_str[6] = '_';
    hex_str[11] = '_';
    hex_str[16] = '_';

    u16_to_hex((const u8*)&number, hex_str + 17);
    u16_to_hex(((const u8*)&number) + 2, hex_str + 12);
    u16_to_hex(((const u8*)&number) + 4, hex_str + 7);
    u16_to_hex(((const u8*)&number) + 6, hex_str + 2);

    sys_write(STDOUT_FD, hex_str, sizeof(hex_str));
}

__attribute__((noinline))
u64 create_thread(thread_start_t thread_start, void* thread_param, void* tls)
{
    i64 err_code = 0;

    const u64 stack_size = THREAD_STACK_SIZE;
    const u64 flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                      CLONE_PARENT | CLONE_THREAD | CLONE_IO;
    
    /* 0 -- no preferred address, no file to map, no offset */
    void* stack = (void*)sys_mmap(0, stack_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_GROWSDOWN, 0, 0);
    void *stack_top               = (void*)((u64)(((char*)stack) + stack_size) & 0xfffffffffffffff0ULL);
    void *stack_thread_func_start = ((char*)stack_top) - 8;
    void *stack_param_loc         = ((char*)stack_top) - 16;
    void *stack_tls_loc           = ((char*)stack_top) - 24;

    *(u64*)stack_thread_func_start  = (u64)thread_start;
    *(u64*)stack_param_loc          = (u64)thread_param;
    *(u64*)stack_tls_loc            = (u64)tls;

    /* 
        Need very precise control here as the compiler may insert instructions between
        syscall and ret.

        The new thread will return 0 from the clone syscall, and by doing ret, 
        the new thread will jump to its code after popping the parameter from its stack.
    */

#ifdef __amd64
    /* Need an additional syscall #158 (archpr_ctrl) for setting the FS.base MSR for TLS */

    asm(
        "syscall\n"
        "orl        %%eax, %%eax\n"
        "jnz        __1f\n"
        "movq       $158, %%rax\n"
        "movq       $0x1002, %%rdi\n"
        "popq       %%rsi\n"
        "syscall    \n"
        "popq       %%rdi\n"
        "ret\n"
"__1f:\n"
        : "=a"(err_code)
        : "0"(SYS_clone), "D"(flags), "S"(stack_tls_loc)
        : "memory", "cc", "r11", "rcx" /* Clobbered by the syscall */
    );

#elif defined(__aarch64__)
    {
        /*
            AArch64 Thread pointer registers:

            Name          | Type       | Reset      | Width      | Description
        ===================================================================================================
            TPIDR_EL0     | RW         | UNK        | 64         | Thread Pointer/ID Register, EL0
            TPIDR_EL1     | RW         | UNK        | 64         | Thread Pointer/ID Register, EL1
            TPIDRRO_EL0   | RW         | UNK        | 64         | Thread Pointer/ID Register, Read-Only, EL0
            TPIDR_EL2     | RW         | UNK        | 64         | Thread Pointer/ID Register, EL2
            TPIDR_EL3     | RW         | UNK        | 64         | Thread Pointer/ID Register, EL3
        */

        register u64 _id asm("x8") = SYS_clone;
        register u64 _x0 asm("x0") = (u64)flags;
        register u64 _x1 asm("x1") = (u64)stack_tls_loc - 8; /* ARM64 stack must be 16 byte aligned */

        asm(
            "svc    0\n"
            "cbnz	x0, __1f\n"
            "ldp    x3, x2, [sp], #16\n"
            "msr    tpidr_el0, x2\n"
            "ldp    x0, x1, [sp], #16\n"
            "ret    x1\n"
    "__1f:\n"
            : "=r"(_id)
            : "r"(_id), "r"(_x0), "r"(_x1)
            : "memory", "cc" /* Clobbered by the syscall */
        );

        err_code = _id;
    }
#else
#   error "Unsupported architecture"
#endif

    return err_code;
}

void fatal(char*msg, u64 err_code)
{
    println();
    print("Fatal: ");
    print(msg);
    print(", error code: ");
    print_h64(err_code);
    println();

#ifdef __amd64
    asm volatile ("int $3");
#elif defined(__aarch64__)
    asm volatile ("brk #0");
#else
#   error "Unsupported architecture"
#endif
}

/*
    Futexes
*/

/* 
    Acquire the futex pointed to by 'futexp': wait for its value to
    become 1, and then set the value to 0. 
*/
void futex_acquire(volatile i32 *futexp)
{
    for (;;)
    {
        /* Is the futex available, i.e *futexp == 1 ? */
        if (__sync_bool_compare_and_swap(futexp, 1, 0))
        {
            /* Yes, it just was, and now it is acquired, i.e. set to 0  */
            break;
        }

        /* Futex is not available; wait */

        i64 s = sys_futex(futexp, FUTEX_WAIT, 0, NULL, NULL, 0);

        if (s != -EAGAIN && s != 0)
        {
            fatal("futex_acquire", s);
        }
    }
}

/* 
    Release the futex pointed to by 'futexp': if the futex currently
    has the value 0, set its value to 1 and the wake any futex waiters,
    so that if the peer is blocked in fpost(), it can proceed. 
*/
void futex_release(volatile i32 *futexp)
{
    if (__sync_bool_compare_and_swap(futexp, 0, 1)) 
    {
        /* Wake at most 1 waiter */
        i64 s = sys_futex(futexp, FUTEX_WAKE, 1, NULL, NULL, 0);

        if (s  < 0 && s >= -4095)
        {
            fatal("futex_release", 0);
        }
    }
}
