#ifndef UCORE_SMP_H
#define UCORE_SMP_H

#include <arch.h>
#include <sched.h>
#include <mmu.h>
#include <vmm.h>
#include <spinlock.h>

#define NCPU		UCONFIG_NR_CPUS

/* we may use lock free list */
struct __timer_list_t{
	list_entry_t tl;
	spinlock_s lock;
};

struct cpu {
  unsigned int id;                    // cpu id
  struct context *scheduler;   // swtch() here to enter scheduler
//   struct taskstate ts;         // Used by x86 to find stack for interrupt
//   struct segdesc gdt[NSEGS];   // x86 global descriptor table
  volatile unsigned int started;       // Has the CPU started?
  int ncli;                    // Depth of pushcli nesting.
  int intena;                  // Were interrupts enabled before pushcli?
  void* idle_kstack;      // kernel stack for idle process
  // Cpu-local storage variables; see below
  struct proc_struct *proc, *idle;           // The currently-running process.
  struct run_queue rqueue; // cpu specific run queue
  size_t used_pages;
  list_entry_t page_struct_free_list;
  spinlock_s rqueue_lock;
  struct __timer_list_t timer_list;
  struct proc_struct* prev;
  // and idle process
};

#include <arch_smp.h>

#define myid() (mycpu()->id)

void smp_init();
void mp_tlb_invalidate(pgd_t* pgdir, uintptr_t la);
void mp_tlb_update(pgd_t* pgdir, uintptr_t la);
void mp_set_mm_pagetable(struct mm_struct* mm);
void mp_tlb_flush();

#endif
