#ifndef _ASM_ARM_MODULE_H
#define _ASM_ARM_MODULE_H

struct mod_arch_specific {
	int foo;
};

#ifndef ARCH64
#define Elf_Shdr	Elf32_Shdr
#define Elf_Sym		Symtab_S
#define Elf_Ehdr	Elf32_Ehdr
#else
#define Elf_Shdr	Elf64_Shdr
#define Elf_Sym		Elf64_Sym
#define Elf_Ehdr	Elf64_Ehdr
#endif

/*
 * Include the ARM architecture version.
 */
#define MODULE_ARCH_VERMAGIC	"ARMv" __stringify(__LINUX_ARM_ARCH__) " "

#endif /* _ASM_ARM_MODULE_H */
