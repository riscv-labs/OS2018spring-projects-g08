.PHONY: all

KERNEL_INC := -I$(T_BASE)/src/kern-ucore/module/include -include linux/autoconf.h -I$(T_BASE)/src/kern-ucore/glue-ucore/libs/ -I$(T_BASE)/src/kern-ucore/module/
LKM_FLAGS ?=
SUB_MODULES :=

SUB_DIRS := mod-sfatfs/Makefile

include ${T_BASE}/mk/compk.mk
include ${T_BASE}/mk/template.mk
-include $(SUB_DIRS)

LKMS := $(patsubst %.mod.c,%.ko,$(wildcard *.mod.c))

$(LKMS): %.ko: %.mod.c %.c
	@echo $(SUB_MODULES)
	$(CC) -c -D__KERNEL__ -DMODULE $(LKM_FLAGS) $(KERNEL_INC) $*.c
	$(CC) -c -D__KERNEL__ -DMODULE $(LKM_FLAGS) $(KERNEL_INC) $*.mod.c
	$(LD) -r -o $@ $*.o $*.mod.o


all: $(LKMS) $(SUB_MODULES)
