#include "lib.h"
#include "libsyscall.c"

#define SYS_write       1
#define SYS_mmap        9
#define SYS_munmap      11
#define SYS_clone       56
#define SYS_exit        60
#define SYS_wait4       61
#define SYS_modifyldt   154
#define SYS_getsetfsgs  158

/************************ ROUTINES ********************************/

u64 sys_mmap(void *addr, u64 length, u64 prot, u64 flags)
{
    return sys_call4(SYS_mmap, (u64)addr, (u64)length, (u64)prot, (u64)flags);
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

i64 sys_write(u64 fd, const void *buf, u64 count)
{
    return sys_call3(SYS_write, (u64)fd, (u64)buf, (u64)count);
}

void sys_exit(i64 err_code)
{
    sys_call1(SYS_exit, (u64)err_code);
}

i64 sys_set_fs(u64 value)
{
    return sys_call2(SYS_getsetfsgs, 0x1002, (u64)value);
}

i64 sys_set_gs(u64 value)
{
    return sys_call2(SYS_getsetfsgs, 0x1001, (u64)value);
}

u64 sys_get_fs()
{
    u64 fs = -1;

    sys_call2(SYS_getsetfsgs, 0x1003, (u64)&fs);

    return fs;
}

u64 sys_get_gs()
{
    u64 gs = -1;

    sys_call2(SYS_getsetfsgs, 0x1004, (u64)&gs);

    return gs;
}

i64 sys_read_ldt(ldt_entry_t* table, u64 byte_count)
{
    return sys_call3(SYS_modifyldt, 0, (u64)table, (u64)byte_count);
}

i64 sys_write_ldt(ldt_entry_t* table, u64 byte_count)
{
    return sys_call3(SYS_modifyldt, 1, (u64)table, (u64)byte_count);
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

__attribute__((naked))
void thread_trampoline()
{
    /* Load the argument into %rdi and jump to the thread function*/
    asm(
        "popq %rdi\n"
        "ret\n"
    );
}

u64 create_thread(thread_start_t thread_start, void* thread_param)
{
    const u64 stack_size = THREAD_STACK_SIZE;
    const u64 flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                      CLONE_PARENT | CLONE_THREAD | CLONE_IO;
    
    /* 0 -- no preferred address */
    void* stack = (void*)sys_mmap(0, stack_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_GROWSDOWN);
    void *stack_top               = ((char*)stack) + stack_size;
    void *stack_thread_func_start = ((char*)stack_top) - 8;
    void *stack_param_loc         = ((char*)stack_top) - 16;
    void *stack_thread_trampoline = ((char*)stack_top) - 24;

    *(u64*)stack_thread_func_start  = (u64)thread_start;
    *(u64*)stack_param_loc          = (u64)thread_param;
    *(u64*)stack_thread_trampoline  = (u64)thread_trampoline;

    return sys_clone(flags, ((char*)stack_top) - 32);
}

void fatal(char*msg, u64 err_code)
{
    println();
    print("Fatal: ");
    print(msg);
    print(", error code: ");
    print_h64(err_code);
    println();

    asm volatile (".byte 0xCC");
}

u64 get_fs()
{
    u64 fs = 0;

    asm volatile (
        "movq %%fs, %%rax\n"
        : "=a"(fs)
        :
        :
        );

    return fs;
}

u64 get_gs()
{
    u64 gs = 0;

    asm volatile (
        "movq %%gs, %%rax\n"
        : "=a"(gs)
        :
        :
        );

    return gs;
}

void set_fs(u64 fs)
{
    asm volatile (
        "movq %%rsi, %%fs\n"
        : 
        : "S"(fs)
        :
        );
}

void set_gs(u64 gs)
{
    asm volatile (
        "movq %%rsi, %%gs\n"
        : 
        : "S"(gs)
        :
        );
}

void set_thread_local_0(i64 error)
{
    asm volatile (
        "mov $0xfffffffffffffff8,%%rax\n"
        "mov %0, %%fs:(%%rax)\n"
        :
        : "D"(error)
        : "rax"
    );
}

i64 get_thread_local_0()
{
    i64 ret = 0;

    asm volatile (
        "mov $0xfffffffffffffff8,%%rax\n"
        "mov %%fs:(%%rax), %%rax\n"
        : "=a"(ret)
        :
        :
    );

    return ret;
}

u64 get_ldt_selector_ring3(u64 index)
{
    /*
        Segment selector (what goes into a segment register):
            0:1     RPL (request priviledge)
            2       Table (0 - GDT, 1 - LDT)
            3:15    Index in the descriptor table
    */

   return (3ULL)        | /* Ring 3*/
          (1ULL << 2)   | /* Local Descriptor Table */
          (index << 3);
}
