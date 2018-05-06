#include <linux/module.h>
#include <linux/compiler.h>

#ifdef CONFIG_MODULES
// int foo; // this shall be present in sym table
// because when compiling we manually include linux/autoconf.h
// in which CONFIG_MODULES is defined
#endif

struct module __this_module
    __attribute__ ((section(".gnu.linkonce.this_module"))) = {
	.name = "hello",.init = init_module,
#ifdef CONFIG_MODULE_UNLOAD
	    .exit = cleanup_module,
#endif
.arch = MODULE_ARCH_INIT,};
// TODO: check out what arch does