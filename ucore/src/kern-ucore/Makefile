#dirs-y := schedule syscall fs process mm libs sync kmodule sysconf numa

dirs-y := schedule syscall process mm libs sync numa fs kmodule smp

dirs-y += arch/$(ARCH)

dirs-$(UCONFIG_HAVE_LINUX_DDE_BASE) += module
dirs-$(UCONFIG_HAVE_LINUX_DDE36_BASE) += dde36
