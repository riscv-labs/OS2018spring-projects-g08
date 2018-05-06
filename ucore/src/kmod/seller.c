/*  Kernel Programming */
#include <linux/module.h>	// Needed by all modules
#include <linux/kernel.h>	// Needed for KERN_ALERT
#include <linux/init.h>		// Needed for the macros
#include <linux/device.h>
#include <linux/kobject.h>


static void seller_print_info(void){
    kprintf("Seller info.\n");
}

static int __init seller_init(void){
    kprintf("Seller init.\n");
    return 0;
}

static void seller_exit(void){
    kprintf("Seller exiting.\n");
}

module_init(seller_init);
module_exit(seller_exit);

MODULE_LICENSE("GPL"); 

EXPORT_SYMBOL(seller_print_info);