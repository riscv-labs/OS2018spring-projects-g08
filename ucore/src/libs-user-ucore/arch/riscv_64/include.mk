ARCH_CFLAGS := -DARCH_RISCV64 -D__UCORE_64__\
				-mcmodel=medany -std=gnu99 -Wextra\
				-fno-builtin -Wall -O2 -nostdinc \
				-fno-stack-protector -ffunction-sections -fdata-sections
ARCH_LDFLAGS := -m elf64lriscv -nostdlib
ARCH_OBJS := clone.o syscall.o
ARCH_INITCODE_OBJ := initcode.o
