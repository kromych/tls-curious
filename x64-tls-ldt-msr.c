#include "lib.c"

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

void thread_0(void* param)
{
    print("Thread # "); print_h64(*(u64*)param); println();

    access_tls();

    sys_exit(0);
}

/***************************** ENTRY POINT ****************************************/

void _start()
{
    u64 thread_num = 1;
    create_thread(thread_0, &thread_num);

    sys_exit(0);
}
