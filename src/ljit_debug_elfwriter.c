#include "ljit_debug_elfwriter.h"



debug_elf* debug_elf_create(unsigned int buffer_size)
{
	Elf *e;
	Elf32_Ehdr *ehdr;
	Elf_Scn *scn;
	Elf32_Shdr *shdr;


	debug_elf *de = (debug_elf *)malloc(sizeof(debug_elf));

	memset(de->strtab,0,sizeof(de->strtab));
	de->strtab_len = 1;
	memset(de->symtab,0,sizeof(de->symtab));
	de->symtab_len = 0;

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
	shdr->sh_name = 11;
	shdr->sh_type = SHT_STRTAB;
	shdr->sh_flags = SHF_STRINGS | SHF_ALLOC;
	shdr->sh_entsize = 0;
	de->strtab_scn = scn;

	scn = elf_newscn(e);
	shdr = elf32_getshdr(scn);
	shdr->sh_name = 19;
	shdr->sh_type = SHT_SYMTAB;
	shdr->sh_flags = SHF_ALLOC;
	shdr->sh_entsize = 0;
	shdr->sh_link = 1;
	de->symtab_scn = scn;

	return de;
}

void debug_elf_add_symbol(debug_elf* de, char* name, void* addr)
{
	Elf32_Sym* sym = &de->symtab[de->symtab_len++];
	sym->st_name = de->strtab_len;
	sym->st_value = addr;
	sym->st_info = STT_FUNC;

	strcpy(de->strtab+de->strtab_len,name);
	de->strtab_len += strlen(name) + 1;
}

void debug_elf_finalize(debug_elf* de)
{
	Elf32_Shdr* shdr = elf32_getshdr(de->strtab_scn);
	shdr->sh_entsize = de->symtab_len;

	elf_update(de->elf,ELF_C_WRITE);
}

char* debug_elf_getaddr(debug_elf* de)
{
	return de->elf_data;
}

void debug_elf_release(debug_elf* de)
{
	elf_end(de->elf);
	fclose(de->elf_file);
	free(de->elf_data);
	free(de);
}
