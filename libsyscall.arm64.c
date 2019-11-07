#include "lib.h"

#ifdef __aarch64__

/* 
    AArch64 system calls take between 0 and 7 arguments.
    For kernel entry we need to move the system call id to x8 then
    load the remaining arguments to registers x0..x6.

    Linux takes system call args in registers:

	svc	0 number	x8
	arg 1		x0
	arg 2		x1
	arg 3		x2
	arg 4		x3
	arg 5		x4
	arg 6		x5
	arg 7		x6

    The Linux kernel uses and destroys internally these registers:
*/

#define CLOBBERED_BY_SYSCALL "memory"

#define sys_call0(id) \
({ \
    register i64 _x0 asm("x0") = 0; \
    register i64 _id asm("x8") = (id); \
    asm volatile( \
        "svc	0\n" \
        : "=r"(_x0) \
        : "r"(_id) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    _x0; \
})

#define sys_call1(id, arg0) \
({ \
    register i64 _x0 asm("x0") = (arg0); \
    register i64 _id asm("x8") = (id); \
    asm volatile( \
        "svc	0\n" \
        : "=r"(_x0) \
        : "r"(_id), "r"(_x0) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    _x0; \
})

#define sys_call2(id, arg0, arg1) \
({ \
    register i64 _x0 asm("x0") = (arg0); \
    register i64 _x1 asm("x1") = (arg1); \
    register i64 _id asm("x8") = (id); \
    asm volatile( \
        "svc	0\n" \
        : "=r"(_x0) \
        : "r"(_id), "r"(_x0), "r"(_x1) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    _x0; \
})

#define sys_call3(id, arg0, arg1, arg2) \
({ \
    register i64 _x0 asm("x0") = (arg0); \
    register i64 _x1 asm("x1") = (arg1); \
    register i64 _x2 asm("x2") = (arg2); \
    register i64 _id asm("x8") = (id); \
    asm volatile( \
        "svc	0\n" \
        : "=r"(_x0) \
        : "r"(_id), "r"(_x0), "r"(_x1), "r"(_x2) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    _x0; \
})

#define sys_call4(id, arg0, arg1, arg2, arg3) \
({ \
    register i64 _x0 asm("x0") = (arg0); \
    register i64 _x1 asm("x1") = (arg1); \
    register i64 _x2 asm("x2") = (arg2); \
    register i64 _x3 asm("x3") = (arg3); \
    register i64 _id asm("x8") = (id); \
    asm volatile( \
        "svc	0\n" \
        : "=r"(_x0) \
        : "r"(_id), "r"(_x0), "r"(_x1), "r"(_x2), "r"(_x3) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    _x0; \
})

#define sys_call5(id, arg0, arg1, arg2, arg3, arg4) \
({ \
    register i64 _x0 asm("x0") = (arg0); \
    register i64 _x1 asm("x1") = (arg1); \
    register i64 _x2 asm("x2") = (arg2); \
    register i64 _x3 asm("x3") = (arg3); \
    register i64 _x4 asm("x4") = (arg4); \
    register i64 _id asm("x8") = (id); \
    asm volatile( \
        "svc	0\n" \
        : "=r"(_x0) \
        : "r"(_id), "r"(_x0), "r"(_x1), "r"(_x2), "r"(_x3), "r"(_x4) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    _x0; \
})

#define sys_call6(id, arg0, arg1, arg2, arg3, arg4, arg5) \
({ \
    register i64 _x0 asm("x0") = (arg0); \
    register i64 _x1 asm("x1") = (arg1); \
    register i64 _x2 asm("x2") = (arg2); \
    register i64 _x3 asm("x3") = (arg3); \
    register i64 _x4 asm("x4") = (arg4); \
    register i64 _x5 asm("x5") = (arg5); \
    register i64 _id asm("x8") = (id); \
    asm volatile( \
        "svc	0\n" \
        : "=r"(_x0) \
        : "r"(_id), "r"(_x0), "r"(_x1), "r"(_x2), "r"(_x3), "r"(_x4), "r"(_x5) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    _x0; \
})

/* ARM64 specific */

#endif
