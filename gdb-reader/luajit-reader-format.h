#ifndef _LUAJIT_READER_FORMAT_H_
#define _LUAJIT_READER_FORMAT_H_

#include "jit-reader.h"

#define LJIT_FILENAME_BUFFER_LENGTH 50
#define LJIT_FUNC_NAME_BUFFER_LENGTH 100

struct ljit_reader_func_def
{
	char filename[LJIT_FILENAME_BUFFER_LENGTH]; 
	char name[LJIT_FUNC_NAME_BUFFER_LENGTH];
	GDB_CORE_ADDR begin;
	GDB_CORE_ADDR end;
	unsigned long num_lines;
};

typedef struct gdb_line_mapping ljit_reader_line_def;


struct ljit_reader_line_def
{
	int line;
	void* addr;
};

#endif // !defined(_LUAJIT_READER_FORMAT_H_)
