/*
 * ljit_debug_gdb_jit_binding.h
 *
 *  Created on: Oct 5, 2011
 *      Author: rtytgat
 */

#ifndef LJIT_DEBUG_GDB_JIT_BINDING_H_
#define LJIT_DEBUG_GDB_JIT_BINDING_H_

#include <stdint.h>
/*
typedef struct {
	debug_elf* de;
} debug_module;

debug_module* debug_module_begin();
void debug_module_add_symbol(debug_module* dm, char* name, void* addr);
void debug_module_commit(debug_module* dm);
*/

void debug_commit_debug_data(void *debugdata, uint64_t size);
#endif /* LJIT_DEBUG_GDB_JIT_BINDING_H_ */
