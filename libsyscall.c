#include "lib.h"

/* 
   The Linux/x86-64 kernel expects the system call parameters in
   registers according to the following table:

    syscall number	rax
    arg 1		rdi
    arg 2		rsi
    arg 3		rdx
    arg 4		r10
    arg 5		r8
    arg 6		r9

    The Linux kernel uses and destroys internally these registers:
    return address from syscall	    	rcx
    eflags from syscall         	    r11
*/

#define CLOBBERED_BY_SYSCALL "memory", "rcx", "r11", "cc"

#define sys_call0(id) \
({ \
    i64 err_code = 0; \
    asm volatile( \
        "syscall\n" \
        : "=a"(err_code) \
        : "0"(id) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    err_code; \
})

#define sys_call1(id, arg0) \
({ \
    i64 err_code = 0; \
    register u64 _arg0 asm("rdi") = (u64)(arg0); \
    asm volatile( \
        "syscall\n" \
        : "=a"(err_code) \
        : "0"(id), "r"(_arg0) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    err_code; \
})

#define sys_call2(id, arg0, arg1) \
({ \
    i64 err_code = 0; \
    register u64 _arg0 asm("rdi") = (u64)(arg0); \
    register u64 _arg1 asm("rsi") = (u64)(arg1); \
    asm volatile( \
        "syscall\n" \
        : "=a"(err_code) \
        : "0"(id), "r"(_arg0), "r"(_arg1) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    err_code; \
})

#define sys_call3(id, arg0, arg1, arg2) \
({ \
    i64 err_code = 0; \
    register u64 _arg0 asm("rdi") = (u64)(arg0); \
    register u64 _arg1 asm("rsi") = (u64)(arg1); \
    register u64 _arg2 asm("rdx") = (u64)(arg2); \
    asm volatile( \
        "syscall\n" \
        : "=a"(err_code) \
        : "0"(id), "r"(_arg0), "r"(_arg1), "r"(_arg2) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    err_code; \
})

#define sys_call4(id, arg0, arg1, arg2, arg3) \
({ \
    i64 err_code = 0; \
    register u64 _arg0 asm("rdi") = (u64)(arg0); \
    register u64 _arg1 asm("rsi") = (u64)(arg1); \
    register u64 _arg2 asm("rdx") = (u64)(arg2); \
    register u64 _arg3 asm("r10") = (u64)(arg3); \
    asm volatile( \
        "syscall\n" \
        : "=a"(err_code) \
        : "0"(id), "r"(_arg0), "r"(_arg1), "r"(_arg2), "r"(_arg3) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    err_code; \
})

#define sys_call5(id, arg0, arg1, arg2, arg3, arg4) \
({ \
    i64 err_code = 0; \
    register u64 _arg0 asm("rdi") = (u64)(arg0); \
    register u64 _arg1 asm("rsi") = (u64)(arg1); \
    register u64 _arg2 asm("rdx") = (u64)(arg2); \
    register u64 _arg3 asm("r10") = (u64)(arg3); \
    register u64 _arg4 asm("r8")  = (u64)(arg4); \
    asm volatile( \
        "syscall\n" \
        : "=a"(err_code) \
        : "0"(id), "r"(_arg0), "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    err_code; \
})

#define sys_call6(id, arg0, arg1, arg2, arg3, arg4, arg5) \
({ \
    i64 err_code = 0; \
    register u64 _arg0 asm("rdi") = (u64)(arg0); \
    register u64 _arg1 asm("rsi") = (u64)(arg1); \
    register u64 _arg2 asm("rdx") = (u64)(arg2); \
    register u64 _arg3 asm("r10") = (u64)(arg3); \
    register u64 _arg4 asm("r8")  = (u64)(arg4); \
    register u64 _arg5 asm("r9")  = (u64)(arg5); \
    asm volatile( \
        "syscall\n" \
        : "=a"(err_code) \
        : "0"(id), "r"(_arg0), "r"(_arg1), "r"(_arg2), "r"(_arg3), "r"(_arg4), "r"(_arg5) \
        : CLOBBERED_BY_SYSCALL \
        ); \
    err_code; \
})
