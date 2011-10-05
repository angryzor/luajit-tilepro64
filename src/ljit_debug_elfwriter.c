#include "ljit_debug_elfwriter.h"



debug_elf* create_debug_elf(unsigned int buffer_size)
{
	Elf *e;
	Elf32_Ehdr *ehdr;
	Elf_Scn *scn;
	Elf32_Shdr *shdr;


	debug_elf *de = (debug_elf *)malloc(sizeof(debug_elf));
	memset(de->strtab,0,sizeof(de->strtab));
	de->strtab_len = 1;

	de->elf_data = (char*)malloc(sizeof(char)*buffer_size);
	de->elf_file = fmemopen(de->elf_data,sizeof(char)*buffer_size,"w");

	de->elf = e = elf_begin(fileno(de->elf_file), ELF_C_WRITE, NULL);


	ehdr = elf32_newehdr(e);
	ehdr->e_ident[EI_DATA] = ELFDATA2LSB;
	ehdr->e_machine = EM_TILEPRO;


	scn = elf_newscn(e);
	shdr = elf32_getshdr(scn);
	shdr->sh_name = 1;
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;

	/* elf_setshstrndx(e, */
	ehdr->e_shstrndx = elf_ndxscn(scn);


	scn = elf_newscn(e);
	shdr = elf32_getshdr(scn);
	shdr->sh_name = 1;
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;


	scn = elf_newscn(e);
	shdr = elf32_getshdr(scn);
	shdr->sh_name = 1;
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;

}

