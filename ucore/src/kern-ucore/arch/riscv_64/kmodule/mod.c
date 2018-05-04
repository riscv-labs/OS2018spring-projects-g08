#include <elf.h>
#include <mod.h>
#include <bitman.h>

int apply_relocate(struct secthdr *sechdrs,
				   const char *strtab,
				   unsigned int symindex,
				   unsigned int relsec, struct module *mod)
{
	// unsigned int i;
	// struct reloc_s *rel = (void *)sechdrs[relsec].sh_addr;
	// struct symtab_s *sym;
	// uint64_t *location;

	// kprintf("Applying relocate section %u to %u\n", relsec,
	// 		sechdrs[relsec].sh_info);
	// for (i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++)
	// {
		/* This is where to make the change */
	// 	location = (void *)(sechdrs[sechdrs[relsec].sh_info].sh_addr + rel[i].r_offset);
	// 	kprintf("RELOC %x\n", (unsigned long)location);
	// 	/* This is the symbol it is referring to.  Note that all
	// 	   undefined symbols have been resolved.  */
	// 	sym = (struct symtab_s *)sechdrs[symindex].sh_addr + GET_RELOC_SYM(rel[i].r_info);

	// 	switch (GET_RELOC_TYPE(rel[i].r_info))
	// 	{
	// 	case R_RISCV_64:
	// 		*location = rel[i].r_addend + sym->st_value;
	// 		break;
	// 	case R_RISCV_LO12_I:
	// 		*location = SET_BITS(*location, 20, 12, PICK_BITS(sym->st_value, 0, 12));
	// 		break;
	// 	case R_RISCV_LO12_S:
	// 		*location = SET_BITS(*location, 7, 5, PICK_BITS(sym->st_value, 0, 5));
	// 		*location = SET_BITS(*location, 25, 7, PICK_BITS(sym->st_value, 5, 7));
	// 		break;
	// 	case R_RISCV_HI20:
	// 		*location = SET_BITS(*location, 12, 20, PICK_BITS(sym->st_value, 12, 20));
	// 		break;
	// 	case R_RISCV_RELAX:
	// 		break;
	// 	default:
	// 		kprintf("apply_relocate: module %s: Unknown relocation: %u\n",
	// 				mod->name, GET_RELOC_TYPE(rel[i].r_info));
	// 		return -1;
	// 	}
	// }
	return 0;
}

/* Apply the given add relocation to the (simplified) ELF.  Return
	-error or 0 */
int apply_relocate_add(struct secthdr *sechdrs,
					   const char *strtab,
					   unsigned int symindex,
					   unsigned int relsec, struct module *mod)
{
		unsigned int i;
	struct reloc_a_s *rel = (void *)sechdrs[relsec].sh_addr;
	struct symtab_s *sym;
	uint64_t *location;

	kprintf("Applying relocate section %u to %u\n", relsec,
			sechdrs[relsec].sh_info);
	for (i = 0; i < sechdrs[relsec].sh_size / sizeof(*rel); i++)
	{
		/* This is where to make the change */
		location = (void *)(sechdrs[sechdrs[relsec].sh_info].sh_addr + rel[i].r_offset);
		kprintf("RELOC %x\n", (unsigned long)location);
		/* This is the symbol it is referring to.  Note that all
		   undefined symbols have been resolved.  */
		sym = (struct symtab_s *)sechdrs[symindex].sh_addr + GET_RELOC_SYM(rel[i].r_info);

		switch (GET_RELOC_TYPE(rel[i].r_info))
		{
		case R_RISCV_64:
			*location = rel[i].r_addend + sym->st_value;
			break;
		case R_RISCV_LO12_I:
			*location = SET_BITS(*location, 20, 12, PICK_BITS(sym->st_value, 0, 12));
			break;
		case R_RISCV_LO12_S:
			*location = SET_BITS(*location, 7, 5, PICK_BITS(sym->st_value, 0, 5));
			*location = SET_BITS(*location, 25, 7, PICK_BITS(sym->st_value, 5, 7));
			break;
		case R_RISCV_HI20:
			*location = SET_BITS(*location, 12, 20, PICK_BITS(sym->st_value, 12, 20));
			break;
		case R_RISCV_RELAX:
			break;
		default:
			kprintf("apply_relocate: module %s: Unknown relocation: %u\n",
					mod->name, GET_RELOC_TYPE(rel[i].r_info));
			return -1;
		}
	}
	return 0;
}
