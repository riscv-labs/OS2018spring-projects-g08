#include <mmu.h>
#include <memlayout.h>

    .section .text,"ax",%progbits
    .globl kern_entry
kern_entry:
    # Load VA stack pointer
    la sp, stack + KSTACKSIZE
    slli t0, a0, KSTACKSHIFT
    add sp, sp, t0

    mv s0, zero # clear frame pointer

  # call into C code.
  # beqz a0, cmain

  # call into C code.
  # j mpboot

    tail kern_init

.section .data
    .align PGSHIFT
    .comm stack, KSTACKSIZE * 8 # TODO: Most 8 processers.
    

