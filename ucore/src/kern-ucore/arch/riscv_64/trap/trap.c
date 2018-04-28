#include <assert.h>
#include <clock.h>
#include <console.h>
#include <types.h>
#include <kdebug.h>
#include <memlayout.h>
#include <mmu.h>
#include <arch.h>
#include <stdio.h>
#include <kio.h>
#include <vmm.h>
#include <swap.h>
#include <trap.h>

#define TICK_NUM 100

static void print_ticks() {
    kprintf("%d ticks\n", TICK_NUM);
#ifdef DEBUG_GRADE
    kprintf("End of Test.\n");
    panic("EOT: kernel seems ok.");
#endif
}

/**
 * @brief      Load supervisor trap entry in RISC-V
 */
void idt_init(void) {
    extern void __alltraps(void);
    /* Set sscratch register to 0, indicating to exception vector that we are
     * presently executing in the kernel */
    write_csr(sscratch, 0);
    /* Set the exception vector address */
    write_csr(stvec, &__alltraps);
    set_csr(sstatus, SSTATUS_SIE);
    /* Allow kernel to access user memory */
    set_csr(sstatus, SSTATUS_SUM);
}

/* trap_in_kernel - test if trap happened in kernel */
bool trap_in_kernel(struct trapframe *tf) {
    return (tf->status & SSTATUS_SPP) != 0;
}

void print_trapframe(struct trapframe *tf) {
    kprintf("trapframe at %p\n", tf);
    print_regs(&tf->gpr);
    kprintf("  status   0x%08x\n", tf->status);
    kprintf("  epc      0x%08x\n", tf->epc);
    kprintf("  badvaddr 0x%08x\n", tf->badvaddr);
    kprintf("  cause    0x%08x\n", tf->cause);
}

void print_regs(struct pushregs *gpr) {
    kprintf("  zero     0x%08x\n", gpr->zero);
    kprintf("  ra       0x%08x\n", gpr->ra);
    kprintf("  sp       0x%08x\n", gpr->sp);
    kprintf("  gp       0x%08x\n", gpr->gp);
    kprintf("  tp       0x%08x\n", gpr->tp);
    kprintf("  t0       0x%08x\n", gpr->t0);
    kprintf("  t1       0x%08x\n", gpr->t1);
    kprintf("  t2       0x%08x\n", gpr->t2);
    kprintf("  s0       0x%08x\n", gpr->s0);
    kprintf("  s1       0x%08x\n", gpr->s1);
    kprintf("  a0       0x%08x\n", gpr->a0);
    kprintf("  a1       0x%08x\n", gpr->a1);
    kprintf("  a2       0x%08x\n", gpr->a2);
    kprintf("  a3       0x%08x\n", gpr->a3);
    kprintf("  a4       0x%08x\n", gpr->a4);
    kprintf("  a5       0x%08x\n", gpr->a5);
    kprintf("  a6       0x%08x\n", gpr->a6);
    kprintf("  a7       0x%08x\n", gpr->a7);
    kprintf("  s2       0x%08x\n", gpr->s2);
    kprintf("  s3       0x%08x\n", gpr->s3);
    kprintf("  s4       0x%08x\n", gpr->s4);
    kprintf("  s5       0x%08x\n", gpr->s5);
    kprintf("  s6       0x%08x\n", gpr->s6);
    kprintf("  s7       0x%08x\n", gpr->s7);
    kprintf("  s8       0x%08x\n", gpr->s8);
    kprintf("  s9       0x%08x\n", gpr->s9);
    kprintf("  s10      0x%08x\n", gpr->s10);
    kprintf("  s11      0x%08x\n", gpr->s11);
    kprintf("  t3       0x%08x\n", gpr->t3);
    kprintf("  t4       0x%08x\n", gpr->t4);
    kprintf("  t5       0x%08x\n", gpr->t5);
    kprintf("  t6       0x%08x\n", gpr->t6);
}

static inline void print_pgfault(struct trapframe *tf) {
    kprintf("page falut at 0x%08x: %c/%c\n", tf->badvaddr,
            trap_in_kernel(tf) ? 'K' : 'U',
            tf->cause == CAUSE_STORE_PAGE_FAULT ? 'W' : 'R');
}

static int pgfault_handler(struct trapframe *tf) {
    extern struct mm_struct *check_mm_struct;
    print_pgfault(tf);
    if (check_mm_struct != NULL) {
        return do_pgfault(check_mm_struct, tf->cause, tf->badvaddr);
    }
    panic("unhandled page fault.\n");
}

static volatile int in_swap_tick_event = 0;
extern struct mm_struct *check_mm_struct;


void interrupt_handler(struct trapframe *tf) {
    intptr_t cause = (tf->cause << 1) >> 1;
    switch (cause) {
        case IRQ_U_SOFT:
            kprintf("User software interrupt\n");
            break;
        case IRQ_S_SOFT:
            kprintf("Supervisor software interrupt\n");
            break;
        case IRQ_H_SOFT:
            kprintf("Hypervisor software interrupt\n");
            break;
        case IRQ_M_SOFT:
            kprintf("Machine software interrupt\n");
            break;
        case IRQ_U_TIMER:
            kprintf("User Timer interrupt\n");
        case IRQ_S_TIMER:
            // "All bits besides SSIP and USIP in the sip register are
            // read-only." -- privileged spec1.9.1, 4.1.4, p59
            // In fact, Call sbi_set_timer will clear STIP, or you can clear it
            // directly.
            // kprintf("Supervisor timer interrupt\n");
            clock_set_next_event();
            if (++ticks % TICK_NUM == 0) {
                print_ticks();
            }
            break;
        case IRQ_H_TIMER:
            kprintf("Hypervisor software interrupt\n");
            break;
        case IRQ_M_TIMER:
            kprintf("Machine software interrupt\n");
            break;
        case IRQ_U_EXT:
            kprintf("User software interrupt\n");
            break;
        case IRQ_S_EXT:
            kprintf("Supervisor external interrupt\n");
            break;
        case IRQ_H_EXT:
            kprintf("Hypervisor software interrupt\n");
            break;
        case IRQ_M_EXT:
            kprintf("Machine software interrupt\n");
            break;
        default:
            print_trapframe(tf);
            break;
    }
}

void exception_handler(struct trapframe *tf) {
    int ret;
    switch (tf->cause) {
        case CAUSE_MISALIGNED_FETCH:
            kprintf("Instruction address misaligned\n");
            break;
        case CAUSE_FETCH_ACCESS:
            kprintf("Instruction access fault\n");
            break;
        case CAUSE_ILLEGAL_INSTRUCTION:
            kprintf("Illegal instruction\n");
            break;
        case CAUSE_BREAKPOINT:
            kprintf("Breakpoint\n");
            panic("test:lc\n");
            break;
        case CAUSE_MISALIGNED_LOAD:
            kprintf("Load address misaligned\n");
            break;
        case CAUSE_LOAD_ACCESS:
            kprintf("Load access fault\n");
            if ((ret = pgfault_handler(tf)) != 0) {
                print_trapframe(tf);
                panic("handle pgfault failed. %e\n", ret);
            }
            break;
        case CAUSE_MISALIGNED_STORE:
            kprintf("AMO address misaligned\n");
            break;
        case CAUSE_STORE_ACCESS:
            kprintf("Store/AMO access fault\n");
            if ((ret = pgfault_handler(tf)) != 0) {
                print_trapframe(tf);
                panic("handle pgfault failed. %e\n", ret);
            }
            break;
        case CAUSE_USER_ECALL:
            kprintf("Environment call from U-mode\n");
            break;
        case CAUSE_SUPERVISOR_ECALL:
            kprintf("Environment call from S-mode\n");
            break;
        case CAUSE_HYPERVISOR_ECALL:
            kprintf("Environment call from H-mode\n");
            break;
        case CAUSE_MACHINE_ECALL:
            kprintf("Environment call from M-mode\n");
            break;
        case CAUSE_FETCH_PAGE_FAULT:
            kprintf("Instruction page fault\n");
            break;
        case CAUSE_LOAD_PAGE_FAULT:
            kprintf("Load page fault\n");
            if ((ret = pgfault_handler(tf)) != 0) {
                print_trapframe(tf);
                panic("handle pgfault failed. %e\n", ret);
            }
            break;
        case CAUSE_STORE_PAGE_FAULT:
            kprintf("Store/AMO page fault\n");
            if ((ret = pgfault_handler(tf)) != 0) {
                print_trapframe(tf);
                panic("handle pgfault failed. %e\n", ret);
            }
            break;
        default:
            print_trapframe(tf);
            break;
    }
}


/* trap_dispatch - dispatch based on what type of trap occurred */
static inline void trap_dispatch(struct trapframe *tf) {
    if ((intptr_t)tf->cause < 0) {
        // interrupts
        interrupt_handler(tf);
    } else {
        // exceptions
        exception_handler(tf);
    }
}

/* *
 * trap - handles or dispatches an exception/interrupt. if and when trap()
 * returns,
 * the code in kern/trap/trapentry.S restores the old CPU state saved in the
 * trapframe and then uses the iret instruction to return from the exception.
 * */
void trap(struct trapframe *tf) { trap_dispatch(tf);}

int ucore_in_interrupt()
{
	return 0;
}