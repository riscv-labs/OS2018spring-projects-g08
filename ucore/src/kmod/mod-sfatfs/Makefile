SUB_MODULES += mod-sfatfs/sfatfs.ko

SFATFS_OBJ := $(patsubst %.c,%.o,$(wildcard mod-sfatfs/*.c))

mod-sfatfs/sfatfs.ko: $(SFATFS_OBJ)
	$(LD) -r -o $@ $(SFATFS_OBJ)

$(SFATFS_OBJ): %.o: %.c
	$(CC) -o $*.o -c -Imod-sfatfs -I../kern-ucore/libs -I../kern-ucore/sync/ \
		-I../kern-ucore/arch/$(ARCH)/libs/ -I../kern-ucore/arch/$(ARCH)/sync/\
		-I../kern-ucore/arch/$(ARCH)/driver/ -I../kern-ucore/arch/$(ARCH)/mm/\
		-I../kern-ucore/fs/vfs/ -I../kern-ucore/fs/ -I../kern-ucore/fs/pipe/\
		-I../kern-ucore/fs/sfs/ -I../kern-ucore/mm/ -I../kern-ucore/fs/devs/\
		-D__KERNEL__ -DMODULE $(LKM_FLAGS) $(KERNEL_INC) $*.c
 