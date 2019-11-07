#ifndef __LIB_H__
#define __LIB_H__

#define NULL              ((void*)0)

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

/* Basic futext operations, there are much more: man 2 futex */

#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#define STDOUT_FD       0x1         /* Standard output */

#define	EPERM		 1	/* Operation not permitted */
#define	ENOENT		 2	/* No such file or directory */
#define	ESRCH		 3	/* No such process */
#define	EINTR		 4	/* Interrupted system call */
#define	EIO		 5	/* I/O error */
#define	ENXIO		 6	/* No such device or address */
#define	E2BIG		 7	/* Argument list too long */
#define	ENOEXEC		 8	/* Exec format error */
#define	EBADF		 9	/* Bad file number */
#define	ECHILD		10	/* No child processes */
#define	EAGAIN		11	/* Try again */
#define	ENOMEM		12	/* Out of memory */
#define	EACCES		13	/* Permission denied */
#define	EFAULT		14	/* Bad address */
#define	ENOTBLK		15	/* Block device required */
#define	EBUSY		16	/* Device or resource busy */
#define	EEXIST		17	/* File exists */
#define	EXDEV		18	/* Cross-device link */
#define	ENODEV		19	/* No such device */
#define	ENOTDIR		20	/* Not a directory */
#define	EISDIR		21	/* Is a directory */
#define	EINVAL		22	/* Invalid argument */
#define	ENFILE		23	/* File table overflow */
#define	EMFILE		24	/* Too many open files */
#define	ENOTTY		25	/* Not a typewriter */
#define	ETXTBSY		26	/* Text file busy */
#define	EFBIG		27	/* File too large */
#define	ENOSPC		28	/* No space left on device */
#define	ESPIPE		29	/* Illegal seek */
#define	EROFS		30	/* Read-only file system */
#define	EMLINK		31	/* Too many links */
#define	EPIPE		32	/* Broken pipe */
#define	EDOM		33	/* Math argument out of domain of func */
#define	ERANGE		34	/* Math result not representable */

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
u64 sys_mmap(void *addr, u64 length, u64 prot, u64 flags, i64 fd, u64 offset);

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
    Operations on futexes
*/

struct timespec
{
    u64 tv_sec;
    u64 tv_nsec;
};

i64 sys_futex(volatile i32 *uaddr, i64 futex_op, i32 val, const struct timespec *timeout, i32 *uaddr2, i32 val3);

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
    Create new thread
*/
typedef void (*thread_start_t)(void*);

u64 create_thread(thread_start_t thread_start, void* thread_param);

/*
    Fatal exit
*/

void fatal(char*msg, u64 err_code);

/*
    Futexes
*/

/* 
    Acquire the futex pointed to by 'futexp': wait for its value to
    become 1, and then set the value to 0. 
*/
void futex_acquire(volatile i32 *futexp);

/* 
    Release the futex pointed to by 'futexp': if the futex currently
    has the value 0, set its value to 1 and the wake any futex waiters,
    so that if the peer is blocked in fpost(), it can proceed. 
*/
void futex_release(volatile i32 *futexp);


/* 
    String and I/O
*/

u64  strlen(const char* str);
void print(const char* str);
void println(void);
void print_h64(u64 number);

#endif
