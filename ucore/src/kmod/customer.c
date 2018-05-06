/*  Kernel Programming */
#include <linux/module.h>	// Needed by all modules
#include <linux/kernel.h>	// Needed for KERN_ALERT
#include <linux/init.h>		// Needed for the macros
#include <linux/device.h>
#include <linux/kobject.h>


static int __init customer_init(void){
    kprintf("Customer requesting symbols from Seller.\n");

    seller_print_info();

    return 0;
}

static void customer_exit(void){
    kprintf("Customer exiting.\n");
}

module_init(customer_init);
module_exit(customer_exit);

MODULE_LICENSE("GPL"); 