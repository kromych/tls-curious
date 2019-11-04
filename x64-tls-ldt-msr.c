#define PROT_READ	0x1		/* page can be read */
#define PROT_WRITE	0x2		/* page can be written */

#define MAP_PRIVATE	    0x02		/* Changes are private */
#define MAP_ANONYMOUS	0x20		/* don't use a file */
#define MAP_GROWSDOWN	0x0100		/* stack-like segment */

#define CLONE_VM	    0x00000100	/* set if VM shared between processes */
#define CLONE_FS	    0x00000200	/* set if fs info shared between processes */
#define CLONE_FILES	    0x00000400	/* set if open files shared between processes */
#define CLONE_SIGHAND	0x00000800	/* set if signal handlers and blocked signals shared */
#define CLONE_PARENT	0x00008000	/* set if we want to have the same parent as the cloner */
#define CLONE_THREAD	0x00010000	/* Same thread group? */
#define CLONE_IO		0x80000000	/* Clone io context */

#define STDOUT_FD       0x1     /* Standard output */

typedef unsigned char u8;
typedef signed char i8;

typedef unsigned int u32;
typedef signed int i32;

typedef unsigned long long u64;
typedef signed long long i64;

/* 
    Local descriptor entry in the CPU 
*/

/******************** PROTOTYPES **************************/

/*
    Map virtual memory
*/
u64 sys_mmap(void *addr, u64 length, u64 prot, u64 flags);

/*
    Clone current thread
*/
u64 sys_clone(u64 flags, void *stack);

/*
    Wait for a thread or a process to change status
*/
u64 sys_waitpid(u64 pid, u64 *wstatus, u64 options);

/*
    System call to write data to file fd
*/
i64 sys_write(u64 fd, const void *buf, u64 count);

/*
    System calls to write the FS.base and GS.base
    Machine Specific Registers.

    These MSRs define the base address for the FS and GS segment registers.
    In the long mode (64-bit mode), many other fields do not make any sense
    as segmentation is not used.

    MSRs cannot be read or written by the err_code running in the Ring 3,
    i.e. the user-mode err_code thus a system call is needed.

    Updating FS.base and GS.base won't change the selectors in the FS
    and GS registers.
*/

i64 sys_set_fs(u64 value);
i64 sys_set_gs(u64 value);

/*
    System calls to read the FS.base and GS.base
    Machine Specific Registers.

    These MSRs define the base address for the
    FS and GS registers. In the long mode (64-bit mode),
    many other fields in the segment registers do not make
    any sense, and segmentation is not used.

    MSRs cannot be read or written by the err_code running in the Ring 3,
    i.e. the user-mode err_code thus a system call is needed.
*/

u64 sys_get_fs();
u64 sys_get_gs();

/*
    An entry of the Local Descriptor Table
*/

#pragma pack(push,1)
typedef struct _ldt_entry_t
{
    u32 entry_number;
    u32 base_addr;
    u32 limit;
    u32 flags;
} ldt_entry_t;
#pragma pack(pop)

/*
    System calls for reading and writing Local Descriptor Table.
*/

i64 sys_read_ldt(ldt_entry_t* table, u64 byte_count);
i64 sys_write_ldt(ldt_entry_t* table, u64 byte_count);

/*
    System call to exit the process.
    Otherwise the CPU may jump into the weeds.
*/

void sys_exit(i64 err_code);

/* 
    Getters and setters for the FS and GS registers.

    Note: these functions just return what is in the segment register,
    i.e. the selector for the Global Descriptor Table or the Local Descriptor Table.
    For the details of the LDT/GDT entry, the selector point to, a system call
    is needed as the instructions for reading/writing descriptor tables are
    priviledged.
*/

u64  get_fs();
u64  get_gs();
void set_fs(u64 fs);
void set_gs(u64 gs);

/*
    Create new thread
*/
u64 create_thread(void (*thread_start)(void));

/*
    Fatal exit
*/

void fatal(char*msg, u64 err_code);

/* 
    String and I/O
*/

u64  strlen(const char* str);
void print(const char* str);
void println(void);
void print_h64(u64 number);

/************************ ROUTINES ********************************/

u64 sys_mmap(void *addr, u64 length, u64 prot, u64 flags)
{
    register u64 ret_code asm("rax") = 0;

    register u64 _addr    asm("rdi") = (u64)addr;
    register u64 _length  asm("rsi") = (u64)length;
    register u64 _prot    asm("rdx") = (u64)prot;
    register u64 _flags   asm("r10") = (u64)flags;

    asm volatile(
        "movq $9, %%rax\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rsi\n"
        "movq %3, %%rdx\n"
        "movq %4, %%r10\n"
        "syscall\n"
        : "=r"(ret_code)
        : "r"(_addr), "r"(_length), "r"(_prot), "r"(_flags)
        :
        );

    return ret_code;
}

/*
    Clone current thread
*/
u64 sys_clone(u64 flags, void *stack)
{
    register u64 ret_code asm("rax") = 0;

    register u64 _flags  asm("rdi") = (u64)flags;
    register u64 _stack  asm("rsi") = (u64)stack;

    asm volatile(
        "movq $56, %%rax\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rsi\n"
        "syscall\n"
        : "=r"(ret_code)
        : "r"(_flags), "r"(_stack)
        :
        );

    return ret_code;
}

u64 sys_waitpid(u64 pid, u64 *wstatus, u64 options)
{
    register u64 ret_code asm("rax") = 0;

    register u64 _pid     asm("rdi") = (u64)pid;
    register u64 _status  asm("rsi") = (u64)wstatus;
    register u64 _options asm("rdx") = (u64)options;

    asm volatile(
        "movq $247, %%rax\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rsi\n"
        "movq %3, %%rdx\n"
        "syscall\n"
        : "=r"(ret_code)
        : "r"(_pid), "r"(_status), "r"(options)
        :
        );

    return ret_code;
}

i64 sys_write(u64 fd, const void *buf, u64 count)
{
    register u64 ret_code asm("rax") = 0;

    register u64 _fd    asm("rdi") = (u64)fd;
    register u64 _buf   asm("rsi") = (u64)buf;
    register u64 _count asm("rdx") = (u64)count;

    asm (
        "movq $1, %%rax\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rsi\n"
        "movq %3, %%rdx\n"
        "syscall" 
        : "=r"(ret_code)
        : "r"(_fd), "r"(_buf), "r"(_count)
        :
        );

    return ret_code;
}

void sys_exit(i64 err_code)
{
    asm volatile(
        "movq $60, %%rax\n"
        "movq %0, %%rdi\n"
        "syscall\n" 
        : 
        : "D"(err_code) 
        : "rax"
        );
}

i64 sys_set_fs(u64 value)
{
    u64 ret_code = 0;

    asm volatile(
        "movq $158, %%rax\n"
        "movq $0x1002, %%rdi\n"
        "movq %1, %%rsi\n"
        "syscall\n" 
        : "=a"(ret_code)
        : "S"(value)
        : "rdi"
        );

    return ret_code;
}

i64 sys_set_gs(u64 value)
{
    u64 ret_code = 0;

    asm volatile(
        "movq $158, %%rax\n"
        "movq $0x1001, %%rdi\n"
        "movq %1, %%rsi\n"
        "syscall\n" 
        : "=a"(ret_code)
        : "S"(value)
        : "rdi"
        );

    return ret_code;
}

u64 sys_get_fs()
{
    u64 fs = -1;

    asm volatile(
        "movq $158, %%rax\n"
        "movq $0x1003, %%rdi\n"
        "movq %0, %%rsi\n"
        "syscall\n" 
        : 
        : "S"(&fs)
        : "rax", "rdi"
        );

    return fs;
}

u64 sys_get_gs()
{
    u64 gs = -1;

    asm volatile(
        "movq $158, %%rax\n"
        "movq $0x1004, %%rdi\n"
        "movq %0, %%rsi\n"
        "syscall\n" 
        : 
        : "S"(&gs)
        : "rax", "rdi"
        );

    return gs;
}

i64 sys_read_ldt(ldt_entry_t* table, u64 byte_count)
{
    u64 ret_code = 0;

    asm volatile(
        "movq $154, %%rax\n"
        "movq $0, %%rdi\n"
        "movq %1, %%rsi\n"
        "movq %2, %%rdx\n"
        "syscall\n" 
        : "=a"(ret_code)
        : "S"(table), "d"(byte_count) 
        : "rdi"
        );

    return ret_code;
}

i64 sys_write_ldt(ldt_entry_t* table, u64 byte_count)
{
    u64 ret_code = 0;

    asm volatile(
        "movq $154, %%rax\n"
        "movq $1, %%rdi\n"
        "movq %1, %%rsi\n"
        "movq %2, %%rdx\n"
        "syscall\n" 
        : "=a"(ret_code)
        : "S"(table), "d"(byte_count) 
        : "rdi"
        );

    return ret_code;
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

u64 create_thread(void (*thread_start)(void))
{
    const u64 stack_size = 1*1024*1024;
    const u64 flags = CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND |
                      CLONE_PARENT | CLONE_THREAD | CLONE_IO;
    
    /* 0 -- no preferred address */
    void* stack = sys_mmap(0, stack_size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE | MAP_GROWSDOWN);
    void *stack_top = ((char*)stack) + stack_size - 8;

    *(u64*)stack_top = (u64)thread_start;

    return sys_clone(flags, ((char*)stack) + stack_size - 16);
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

/***************************** VARIABLES ****************************************/

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

/***************************** HELPERS ********************************************/

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

void access_tls()
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
        print("FS: "); print_h64(sys_get_fs()); println();
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

            err_code = sys_write_ldt(&e, sizeof(ldt_entry_t));
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

        print("FS: "); print_h64(sys_get_fs()); println();

        /* 
            Setting FS to point to the start of the second page
            as the TLS space will be used so it ends on a page boundary.
        */
        print("Setting MSRs"); println();

        err_code = sys_set_fs(tls_start);

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

void thread_0()
{
    access_tls();

    sys_exit(0);
}

/***************************** ENTRY POINT ****************************************/

void _start()
{
    u64 wait_status = 0;

    sys_waitpid(create_thread(thread_0), &wait_status, 0);
    
    println();

    sys_exit(0);
}
