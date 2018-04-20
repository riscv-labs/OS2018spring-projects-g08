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
            cprintf("User Timer interrupt\n");
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
    switch (tf->cause) {
        case CAUSE_MISALIGNED_FETCH:
            break;
        case CAUSE_FAULT_FETCH:
            break;
        case CAUSE_ILLEGAL_INSTRUCTION:
            break;
        case CAUSE_BREAKPOINT:
            break;
        case CAUSE_MISALIGNED_LOAD:
            break;
        case CAUSE_FAULT_LOAD:
            break;
        case CAUSE_MISALIGNED_STORE:
            break;
        case CAUSE_FAULT_STORE:
            break;
        case CAUSE_USER_ECALL:
            break;
        case CAUSE_SUPERVISOR_ECALL:
            break;
        case CAUSE_HYPERVISOR_ECALL:
            break;
        case CAUSE_MACHINE_ECALL:
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
