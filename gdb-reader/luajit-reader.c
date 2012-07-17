#include "jit-reader.h"
#include "luajit-reader.h"
#include "luajit-reader-format.h"
#include "tile64-registers.h"

#include <stdlib.h>
#include <string.h>

enum gdb_status luajit_read_debug_info (struct gdb_reader_funcs *self,
                                               struct gdb_symbol_callbacks *cb,
                                               void *memory, long memory_sz)
{
	unsigned char* position = (char*)memory;
	struct ljit_reader_func_def* funcdef = (struct ljit_reader_func_def*)position;
	position += sizeof(struct ljit_reader_func_def);

	struct gdb_object* obj = cb->object_open(cb);
	struct gdb_symtab* symtab = cb->symtab_open(cb,obj,funcdef->filename);
	struct gdb_block* block = cb->block_open(cb,symtab,NULL,funcdef->begin,funcdef->end,funcdef->name);

	cb->line_mapping_add(cb,symtab,funcdef->num_lines,(struct gdb_line_mapping*)position);

	cb->symtab_close(cb,symtab);
	cb->object_close(cb,obj);

	return GDB_SUCCESS;
}

static enum gdb_status get_frame_pointer (struct gdb_unwind_callbacks *cb,
										GDB_CORE_ADDR* frame_pointer)
{
#ifdef LUAJIT_OMIT_FRAME_POINTER
#warn Cannot unwind frames when LUAJIT_OMIT_FRAME_POINTER is enabled! Debugging will be difficult.
	return GDB_FAIL;
#endif
	
	struct gdb_reg_value* sp = cb->reg_get(cb, TILE64_REG_SP);
	if(!sp->defined || sp->size != 4)
		return GDB_FAIL;

	GDB_CORE_ADDR top_of_stack = *(GDB_CORE_ADDR*)sp->value;
	if(!cb->target_read(top_of_stack + 4, frame_pointer, sizeof(*frame_pointer)))
		return GDB_FAIL;

	return GDB_SUCCESS;
}

void luajit_reg_value_free(struct gdb_reg_value *val)
{
	free(val);
}

/* Unwind the current frame, CB is the set of unwind callbacks that
   are to be used to do this.

   Return GDB_FAIL on failure and GDB_SUCCESS on success.  */

enum gdb_status luajit_unwind_frame (struct gdb_reader_funcs *self,
                                            struct gdb_unwind_callbacks *cb)
{
	GDB_CORE_ADDR frame_pointer;

	if(!get_frame_pointer(cb,&frame_pointer))
		return GDB_FAIL;
	
	struct gdb_reg_value* new_sp = malloc(sizeof(struct gdb_reg_value) + sizeof(frame_pointer) - 1); // 4byte value
	new_sp->free = &luajit_reg_value_free;
	new_sp->defined = 1;
	new_sp->size = 4;
	memcpy(new_sp->value,&frame_pointer,sizeof(frame_pointer));

	cb->reg_set(cb, TILE64_REG_SP, new_sp);
	
	return GDB_SUCCESS;
}

/* Return the frame ID corresponding to the current frame, using C to
   read the current register values.  See the comment on struct
   gdb_frame_id.  */

struct gdb_frame_id luajit_get_frame_id (struct gdb_reader_funcs *self,
                                                struct gdb_unwind_callbacks *cb)
{
	struct gdb_frame_id fid;
	GDB_CORE_ADDR frame_pointer;
	fid.code_address = 0;
	fid.stack_address = 0;

	if(!get_frame_pointer(cb,&frame_pointer))
		return fid;

	GDB_CORE_ADDR code;
	if(!cb->target_read(frame_pointer - 4,&code,sizeof(code)))
		return fid;

	fid.code_address = code;
	fid.stack_address = frame_pointer;
}

/* Called when a reader is being unloaded.  This function should also
   free SELF, if required.  */
void luajit_destroy_reader (struct gdb_reader_funcs *self)
{
	free(self);
}

struct gdb_reader_funcs *gdb_init_reader()
{
	struct gdb_reader_funcs* f = malloc(sizeof(struct gdb_reader_funcs));

	f->reader_version = GDB_READER_INTERFACE_VERSION;
	f->read = &luajit_read_debug_info;
	f->unwind = &luajit_unwind_frame;
	f->get_frame_id = &luajit_get_frame_id;
	f->destroy = &luajit_destroy_reader;

	return f;
}


