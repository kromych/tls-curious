#include "lib.h"

u64 sys_call0(u64 id)
{
    register u64 _id asm("rax") = id;

    asm volatile(
        "movq %0, %%rax\n"
        "syscall\n"
        :
        : "r"(_id)
        :
        );

    return _id;
}

u64 sys_call1(u64 id, u64 arg0)
{
    register u64 _id asm("rax") = id;

    register u64 _arg0 asm("rdi") = (u64)arg0;

    asm volatile(
        "movq %0, %%rax\n"
        "movq %1, %%rdi\n"
        "syscall\n"
        :
        : "r"(_id), "r"(_arg0)
        :
        );

    return _id;
}

u64 sys_call2(u64 id, u64 arg0, u64 arg1)
{
    register u64 _id asm("rax") = id;

    register u64 _arg0 asm("rdi") = (u64)arg0;
    register u64 _arg1 asm("rsi") = (u64)arg1;

    asm volatile(
        "movq %0, %%rax\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rsi\n"
        "syscall\n"
        :
        : "r"(_id), "r"(_arg0), "r"(_arg1)
        :
        );

    return _id;
}

u64 sys_call3(u64 id, u64 arg0, u64 arg1, u64 arg2)
{
    register u64 _id asm("rax") = id;

    register u64 _arg0 asm("rdi") = (u64)arg0;
    register u64 _arg1 asm("rsi") = (u64)arg1;
    register u64 _arg2 asm("rdx") = (u64)arg2;

    asm volatile(
        "movq %0, %%rax\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rsi\n"
        "movq %3, %%rdx\n"
        "syscall\n"
        :
        : "r"(_id), "r"(_arg0), "r"(_arg1), "r"(_arg2)
        :
        );

    return _id;
}

u64 sys_call4(u64 id, u64 arg0, u64 arg1, u64 arg2, u64 arg3)
{
    register u64 _id asm("rax") = id;

    register u64 _arg0 asm("rdi") = (u64)arg0;
    register u64 _arg1 asm("rsi") = (u64)arg1;
    register u64 _arg2 asm("rdx") = (u64)arg2;
    register u64 _arg3 asm("r10") = (u64)arg3;

    asm volatile(
        "movq %0, %%rax\n"
        "movq %1, %%rdi\n"
        "movq %2, %%rsi\n"
        "movq %3, %%rdx\n"
        "movq %4, %%r10\n"
        "syscall\n"
        :
        : "r"(_id), "r"(_arg0), "r"(_arg1), "r"(_arg2), "r"(_arg3)
        :
        );

    return _id;
}
