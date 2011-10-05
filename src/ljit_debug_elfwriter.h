/*
 * ljit_debug_elfwriter.h
 *
 *  Created on: 4-okt-2011
 *      Author: rtytgat
 */

#ifndef LJIT_DEBUG_ELFWRITER_H_
#define LJIT_DEBUG_ELFWRITER_H_

#include <stdio.h>
#include <libelf.h>

#define ELF_BUF_SIZE 20000
#define ELF_STRTAB_SIZE 10000
#define ELF_SYMTAB_SIZE 50


typedef struct {
	Elf* elf;
	char* elf_data;
	FILE* elf_file;

	Elf_Scn* strtab_scn;
	Elf_Scn* symtab_scn;

	const char shstrtab [] =
	{
			'\0',
			'.','s','h','s','t',  'r','t','a','b','\0',
			'.','s','t','r','t',  'a','b','\0',
			'.','s','y','m','t',  'a','b','\0'
	};

	char strtab[ELF_STRTAB_SIZE];
	unsigned int strtab_len;

	Elf32_Sym symtab[ELF_SYMTAB_SIZE];
	unsigned int symtab_len;
} debug_elf;

debug_elf* debug_elf_create(unsigned int buffer_size);
void debug_elf_add_symbol(debug_elf* de, char* name, void* addr);
void debug_elf_finalize(debug_elf* de);
char* debug_elf_getaddr(debug_elf* de);
void debug_elf_release(debug_elf* de);

#endif /* LJIT_DEBUG_ELFWRITER_H_ */
