#include "lib.c"

/***************************** PLATFORM-INDEPENDENT ****************************************/

__attribute__((tls_model("local-exec")))
__attribute__((used))
static __thread u64 thread_local_0;

__attribute__((tls_model("local-exec")))
__attribute__((used))
static __thread u64 thread_local_1;

__attribute__((tls_model("local-exec")))
__attribute__((used))
static __thread u64 thread_local_2;

/* Four 4KiB pages to be the TLS backing store for this thread */
/* Spares a mmap call. */
u64 tls_pages[2048] __attribute__((aligned(4096)));

/*
    Look for the values in the TLS backing store
*/
void find_values_in_tls()
{
    {
        println();
        print("Looking for the values in the backing store: "); println();

        for (u64 i = 0; i < sizeof(tls_pages)/sizeof(tls_pages[0]); ++i)
        {
            if (tls_pages[i] != 0) 
            {
                print("found at offset "); print_h64(i*sizeof(tls_pages[0])); print(": value "); print_h64(tls_pages[i]); println();
            }
        }
    }
}

/***************************** HELPERS for x64 ********************************************/

#ifdef __amd64

/* 
    Getters and setters for the FS and GS registers.

    Note: these functions just return what is in the segment register,
    i.e. the selector for the Global Descriptor Table or the Local Descriptor Table.
    For the details of the LDT/GDT entry, the selector point to, a system call
    is needed as the instructions for reading/writing descriptor tables are
    priviledged.
*/

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

void access_tls_x64()
{
    i64 err_code = 0;
    u64 tls_start = (u64)(((char*)tls_pages) + 4096);

    /*
        Option 0. set_thread_area/get_thread_area are not available in 64-bit kernels.
    */
    {
    }

    /* 
        Option 1. Accessing thread-local variables using Local Descriptor Table
    */
    {
        print("FS: "); print_h64(sys_x64_get_fs()); println();
        print("Updating LDT"); println();

        for (u64 i = 0; i < 64; ++i)
        {
            /* 
                This is not the CPU format, this is what the system call accepts 
            */
            ldt_entry_t e = {
                .entry_number = i,
                .base_addr = (typeof(e.base_addr))tls_start,
                .limit = 0,
                .flags = (1 << 6) | /* Usable */
                         (1 << 7)   /* Long mode */
                /*
                    seg_32bit:       0
                    contents:        1..2
                    read_exec_only:  3
                    limit_in_pages:  4 
                    seg_not_present: 5
                    useable:         6
                    lm:              7
                */
            };

            err_code = sys_x64_write_ldt(&e, sizeof(ldt_entry_t));
            if (err_code != 0)
            {
                fatal("Error when setting an entry in LDT", err_code);
            }
        }

        print("Updating FS"); println();
        set_fs(get_ldt_selector_ring3(3)); /* Make FS point to the entry #3 of LDT, we updated 64 just in case :) */

        print("FS: "); print_h64(get_fs()); println();

        thread_local_0 = 123456789;
        thread_local_1 = 123456790;
        thread_local_2 = 123456791;

        print("thread_local_0: "); print_h64(thread_local_0); println();
        print("thread_local_1: "); print_h64(thread_local_1); println();
        print("thread_local_2: "); print_h64(thread_local_2); println();

        find_values_in_tls();
    }

    /*
        Option 2: setting MSRs.

        In the long mode (64bit mode), segment registers are not taken into account.
        MSR: FS.base or GS.base.
    */

    {
        println();
        
        print("Clearing FS"); println();
        set_fs(0);

        print("FS: "); print_h64(sys_x64_get_fs()); println();

        /* 
            Setting FS to point to the start of the second page
            as the TLS space will be used so it ends on a page boundary.
        */
        print("Setting MSRs"); println();

        err_code = sys_x64_set_fs(tls_start);

        if (err_code != 0)
        {
            fatal("Error when updating FS.base", err_code);
        }

        thread_local_0 = 123456792;
        thread_local_1 = 123456793;
        thread_local_2 = 123456794;

        print("thread_local_0: "); print_h64(thread_local_0); println();
        print("thread_local_1: "); print_h64(thread_local_1); println();
        print("thread_local_2: "); print_h64(thread_local_2); println();

        find_values_in_tls();
    }
}

#elif defined(__aarch64__)

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

void access_tls_arm64()
{
    register u64 tls_start asm ("x0") = (u64)(((char*)tls_pages) + 4096);

    asm volatile (
        "msr tpidr_el0, x0"
        :
        : "r"(tls_start)
        );

    println();

    print("Setting "); println();

    thread_local_0 = 123456792;
    thread_local_1 = 123456793;
    thread_local_2 = 123456794;

    print("thread_local_0: "); print_h64(thread_local_0); println();
    print("thread_local_1: "); print_h64(thread_local_1); println();
    print("thread_local_2: "); print_h64(thread_local_2); println();

    find_values_in_tls();
}

#endif

typedef struct _thread_context_t
{
    volatile i32    exited_futex __attribute__((aligned(4)));
    u64             thread_num;
} thread_context_t;

__attribute__((noinline))
void thread_0(void* param)
{
    thread_context_t* thread_context = (thread_context_t*)param;

    print("Thread # "); print_h64(thread_context->thread_num); println();

#ifdef __amd64
    access_tls_x64();
#elif defined(__aarch64__)
    access_tls_arm64();
#endif

    print("Thread # "); print_h64(thread_context->thread_num); print(" exited "); println();

    futex_release(&thread_context->exited_futex);

    sys_exit(0);
}

/***************************** ENTRY POINT ****************************************/

void _start()
{
    /* futex == 0: State: unavailable */
    /* futex == 1: State: available */

    thread_context_t thread_context = {
        .exited_futex = 0,
        .thread_num = 1
    };

    print("Process started\n");

    create_thread(thread_0, &thread_context);

    futex_acquire(&thread_context.exited_futex);

    print("Process exited\n");

    sys_exit(0);
}
