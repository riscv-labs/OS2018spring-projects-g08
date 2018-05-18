#include<linux/module.h>

extern int init_mod();
extern void cleanup_mod();

// module_init(init_mod);
// module_exit(cleanup_mod);

// MODULE_AUTHOR("lanchang")
MODULE_LICENSE("GPL"); // would be stored in section .modinfo


struct module __this_module
    __attribute__ ((section(".gnu.linkonce.this_module"))) = {
	.name = "sfatfs",.init = init_mod,
#ifdef CONFIG_MODULE_UNLOAD
	    .exit = cleanup_mod,
#endif
.arch = MODULE_ARCH_INIT,};
