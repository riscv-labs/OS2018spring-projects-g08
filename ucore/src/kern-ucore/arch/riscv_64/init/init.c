#include <clock.h>
#include <console.h>
#include <types.h>
#include <stdio.h>
#include <intr.h>
#include <kdebug.h>
#include <monitor.h>
#include <picirq.h>
#include <pmm.h>
#include <arch.h>
#include <kio.h>
#include <string.h>
#include <trap.h>
#include <vmm.h>
#include <ide.h>
#include <swap.h>
#include <mp.h>
//#include <mod.h>

int kern_init(void) __attribute__((noreturn));
void grade_backtrace(void);
static void lab1_switch_test(void);

int kern_init(void) {
    extern char edata[], end[];
    memset(edata, 0, end - edata);

    cons_init();  // init the console

    const char *message = "(THU.CST) os is loading ...\n";
    kprintf("%s\n\n", message);

    print_kerninfo();

	/* Only to initialize lcpu_count. */
	mp_init();


	size_t nr_used_pages_store = nr_used_pages();

	//debug_init();		// init debug registers
    pmm_init();  // init physical memory management
    // pmm_init_ap();

    pic_init();  // init interrupt controller
    idt_init();  // init interrupt descriptor table

    vmm_init();                 // init virtual memory management
    sched_init();		// init scheduler
    proc_init();                // init process table
    sync_init();		// init sync struct

    ide_init();                 // init ide devices
#ifdef UCONFIG_SWAP
	swap_init();		// init swap
#endif
    // fs_init();
    // rdtime in mbare mode crashes
    clock_init();  // init clock interrupt
    //mod_init();

    intr_enable();  // enable irq interrupt

    cpu_idle();                 // run idle process
}

void __attribute__((noinline))
grade_backtrace2(int arg0, int arg1, int arg2, int arg3) {
    mon_backtrace(0, NULL, NULL);
}

void __attribute__((noinline)) grade_backtrace1(int arg0, int arg1) {
    grade_backtrace2(arg0, (int64_t)&arg0, arg1, (int64_t)&arg1);
}

void __attribute__((noinline)) grade_backtrace0(int arg0, int arg1, int arg2) {
    grade_backtrace1(arg0, arg2);
}

void grade_backtrace(void) { grade_backtrace0(0, (int64_t)kern_init, 0xffff0000); }

static void lab1_print_cur_status(void) {
    static int round = 0;
    round++;
}

static void lab1_switch_to_user(void) {
    // LAB1 CHALLENGE 1 : TODO
}

static void lab1_switch_to_kernel(void) {
    // LAB1 CHALLENGE 1 :  TODO
}

static void lab1_switch_test(void) {
    lab1_print_cur_status();
    kprintf("+++ switch to  user  mode +++\n");
    lab1_switch_to_user();
    lab1_print_cur_status();
    kprintf("+++ switch to kernel mode +++\n");
    lab1_switch_to_kernel();
    lab1_print_cur_status();
}
