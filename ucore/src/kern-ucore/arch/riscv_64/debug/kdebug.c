#include <assert.h>
#include <types.h>
#include <stdio.h>
#include <kio.h>

/* *
 * print_kerninfo - print the information about kernel, including the location
 * of kernel entry, the start addresses of data and text segements, the start
 * address of free memory and how many memory that kernel has used.
 * */
void print_kerninfo(void) {
    extern char etext[], edata[], end[], kern_init[];
    kprintf("Special kernel symbols:\n");
    kprintf("  entry  0x%016llx (virtual)\n", kern_init);
    kprintf("  etext  0x%016llx (virtual)\n", etext);
    kprintf("  edata  0x%016llx (virtual)\n", edata);
    kprintf("  end    0x%016llx (virtual)\n", end);
    kprintf("Kernel executable memory footprint: %dKB\n",
            (end - kern_init + 1023) / 1024);
}

/* *
 * print_debuginfo - read and print the stat information for the address @eip,
 * and info.eip_fn_addr should be the first address of the related function.
 * */
void print_debuginfo(uintptr_t eip) { 
    // panic("Not Implemented!"); 
}


void print_stackframe(void) {
    // panic("Not Implemented!");
}

