/*
 * ljit_debug_gdb_jit_binding.c
 *
 *  Created on: Oct 5, 2011
 *      Author: rtytgat
 */

#include "ljit_debug_gdb_jit_binding.h"

#include <stdlib.h>
#include <string.h>

typedef enum
{
JIT_NOACTION = 0,
JIT_REGISTER_FN,
JIT_UNREGISTER_FN
} jit_actions_t;

struct jit_code_entry
{
struct jit_code_entry *next_entry;
struct jit_code_entry *prev_entry;
const char *symfile_addr;
uint64_t symfile_size;
};

struct jit_descriptor
{
uint32_t version;
/* This type should be jit_actions_t, but we use uint32_t
  to be explicit about the bitwidth.  */
uint32_t action_flag;
struct jit_code_entry *relevant_entry;
struct jit_code_entry *first_entry;
};

/* GDB puts a breakpoint in this function.  */
void __attribute__((noinline)) __jit_debug_register_code() { };

/* Make sure to specify the version statically, because the
debugger may check the version before we can set it.  */
struct jit_descriptor __jit_debug_descriptor = { 1, 0, 0, 0 };

void debug_commit_debug_data(void *debugdata, uint64_t size) {
	struct jit_code_entry* jce = (struct jit_code_entry*)malloc(sizeof(struct jit_code_entry));
	jce->prev_entry = NULL;
	jce->next_entry = __jit_debug_descriptor.first_entry;
	jce->symfile_addr = (const char*)debugdata;
	jce->symfile_size = size;
	if(__jit_debug_descriptor.first_entry)
		__jit_debug_descriptor.first_entry->prev_entry = jce;

	__jit_debug_descriptor.first_entry = jce;
	__jit_debug_descriptor.action_flag = JIT_REGISTER_FN;
	__jit_debug_register_code();

}

/*

debug_module* debug_module_begin()
{
	debug_module* dm = (debug_module*)malloc(sizeof(debug_module));
	dm->de = debug_elf_create();
	return dm;
}

void debug_module_add_symbol(debug_module* dm, char* name, void* addr)
{
	debug_elf_add_symbol(dm->de, name, addr);
}

void debug_module_commit(debug_module* dm)
{
	debug_elf_finalize(dm->de);
	struct jit_code_entry* jce = (struct jit_code_entry*)malloc(sizeof(struct jit_code_entry));
	jce->prev_entry = NULL;
	jce->next_entry = __jit_debug_descriptor.first_entry;
	jce->symfile_addr = debug_elf_getaddr(dm->de);
	jce->symfile_size = ELF_BUF_SIZE;
	if(__jit_debug_descriptor.first_entry)
		__jit_debug_descriptor.first_entry->prev_entry = jce;

	__jit_debug_descriptor.first_entry = jce;
	__jit_debug_descriptor.action_flag = JIT_REGISTER_FN;
	__jit_debug_register_code();

	//debug_elf_release(dm->de);
	//free(dm);
}
*/
