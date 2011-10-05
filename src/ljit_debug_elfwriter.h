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


typedef struct {
	Elf* elf;
	char* elf_data;
	FILE* elf_file;

	const char shstrtab [] =
	{
			'\0',
			'.','s','h','s','t',  'r','t','a','b','\0',
			'.','s','y','m','t',  'a','b','\0',
			'.','s','t','r','t',  'a','b','\0'
	};

	char strtab[ELF_STRTAB_SIZE];
	unsigned int strtab_len;
} debug_elf;

#endif /* LJIT_DEBUG_ELFWRITER_H_ */
