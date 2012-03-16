/*
 * ljit_debug_stackdump.c
 *
 *  Created on: Feb 28, 2012
 *      Author: rtytgat
 */

#include "ljit_debug_stackdump.h"
#include <stdio.h>

static FILE* f;

void ljit_debug_initstackdump()
{
	f = fopen("/home/rtytgat/.tilera/workspace/ljt64/src/log.log","a");
}

void ljit_debug_releasestackdump()
{
	fclose(f);
}

void ljit_debug_dumpstack(TValue* base, TValue* top, CallInfo* ci, lua_State* L, const char* opname, int ra, int rb, int rc, int rbx)
{
	TValue* s;

	fprintf(f,"------------------------------------------------------------------------\n");
	fprintf(f," PRE-OPCODE: %s,\t\tA = %08x, B = %08x, C = %08x, Bx = %08x\n", opname, ra, rb, rc, rbx);
	fprintf(f,"''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''\n");
	fprintf(f," L->base = %08x\n L->top = %08x\n L->ci = %08x\n L->stack = %08x\n L->stack_last = %08x\n\n", base - L->stack, top - L->stack, (unsigned int)ci, (unsigned int)L->stack, L->stack_last - L->stack);
	fprintf(f," CI->func = %08x\n CI->base = %08x\n CI->top = %08x\n\n", ci->func - L->stack, ci->base - L->stack, ci->top - L->stack);
	//fprintf(f," (CI-1)->func = %08x\n (CI-1)->base = %08x\n (CI-1)->top = %08x\n\n", (ci-1)->func - L->stack, (ci-1)->base - L->stack, (ci-1)->top - L->stack);
	fprintf(f," STACK:\n");
	for(s = L->stack; s <= L->stack_last; s++)
	{
		fprintf(f,"\t%08x | TYPE: ", s - L->stack);
		switch(s->tt)
		{
		case LUA_TNIL:
			fprintf(f,"nil\n");
			break;
		case LUA_TBOOLEAN:
			fprintf(f,"boolean: %d\n", bvalue(s));
			break;
		case LUA_TLIGHTUSERDATA:
			fprintf(f,"lightuserdata\n");
			break;
		case LUA_TNUMBER:
			fprintf(f,"number: %f\n", nvalue(s));
			break;
		case LUA_TSTRING:
			fprintf(f, "string: %s\n", getstr(rawtsvalue(s)));
			break;
		case LUA_TTABLE:
			fprintf(f, "table\n");
			break;
		case LUA_TFUNCTION:
			if(iscfunction(s))
				fprintf(f, "cfunction: %08x\n", (unsigned int)clvalue(s)->c.f);
			else
				fprintf(f, "lfunction\n");
			break;
		case LUA_TUSERDATA:
			fprintf(f, "userdata\n");
			break;
		case LUA_TTHREAD:
			fprintf(f, "thread\n");
			break;
		default:
			fprintf(f, "unknown: %d\n", s->tt);
			break;
		}
	}
	fprintf(f,"------------------------------------------------------------------------\n");
	fflush(f);
}
