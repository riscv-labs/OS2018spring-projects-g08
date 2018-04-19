#ARCH_INLUCDES := . debug driver include libs mm numa process sync trap syscall kmodule driver/acpica/source/include
ARCH_INLUCDES := . debug driver include libs mm trap kmodule driver/acpica/source/include
ARCH_CFLAGS :=
ARCH_LDFLAGS := -m elf64lriscv -nostdlib --gc-sections
