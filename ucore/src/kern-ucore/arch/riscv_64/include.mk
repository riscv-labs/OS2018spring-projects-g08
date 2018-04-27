ARCH_INLUCDES := . debug driver include libs mm numa process sync trap syscall kmodule driver/acpica/source/include
ARCH_CFLAGS := -mcmodel=medany -std=gnu99 -Wextra\
				-fno-builtin -Wall -O2 -nostdinc \
				-fno-stack-protector -ffunction-sections -fdata-sections
ARCH_LDFLAGS := -m elf64lriscv -nostdlib
