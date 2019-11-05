#ifndef __LIB_H__
#define __LIB_H__

#define THREAD_STACK_SIZE 2*1024*1024

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

#define	WNOHANG		0x1	/* Don't block waiting.  */
#define	WUNTRACED	0x2	/* Report status of stopped children.  */
#define WSTOPPED	0x2	/* Report stopped child (same as WUNTRACED). */
#define WEXITED	    0x4	/* Report dead child.  */
#define WCONTINUED	0x8	/* Report continued child.  */
#define WNOWAIT	    0x01000000 /* Don't reap, just poll status.  */

#define STDOUT_FD       0x1         /* Standard output */

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
    Unmap virtual memory
*/
u64 sys_munmap(void *addr, u64 length);

/*
    Clone current thread
*/
u64 sys_clone(u64 flags, void *stack);

/*
    Wait for a process to change status
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
typedef void (*thread_start_t)(void*);

u64 create_thread(thread_start_t thread_start, void* thread_param);

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

#endif
