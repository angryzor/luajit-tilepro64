/*
 * ljit_debug_stackvalidator.c
 *
 *  Created on: Dec 12, 2011
 *      Author: rtytgat
 */

#include "ljit_debug_stackvalidator.h"
#include <stdlib.h>
#include "ldo.h"
#include "lmem.h"

static lua_State i_L;
/*
static TValue* stackadjust(lua_State* L, TValue* idx)
{
	return idx - L->stack + i_L->stack;
}

static CallInfo* ciadjust(lua_State* L, CallInfo* idx)
{
	return idx - L->base_ci + i_L->base_ci;
}
*/

#define stackadjust(L,idx) (idx - L->stack + i_L->stack)
#define ciadjust(L,idx) (idx - L->base_ci + i_L->base_ci)

static stackdump(lua_State* L, FILE* f)
{
	StkId s;
	CallInfo* c;

	fprintf(f," L->base = %08x\n L->top = %08x\n L->ci = %08x\n L->stack = %08x\n L->stack_last = %08x\n\n", L->base - L->stack, L->top - L->stack, L->ci - L->base_ci, L->stack_last - L->stack);
	fprintf(f," CI:\n\tCI->func\tCI->base\tCI->top\n");
	for(c = L->base_ci; c <= L->ci; c++)
		fprintf(f,"\t%08x\t%08x\t%08x\n", c->func - L->stack, c->base - L->stack, c->top - L->stack);
	fprintf(f,"\n STACK:\n");
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
}

static statedump(lua_State* L, const Instruction* thisins, const char* errmsg)
{
	TValue* s;

	FILE* f = fopen("log.log","a");
	fprintf(f,"xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n");
	fprintf(f," VM STATE INCONSISTENCY DETECTED: %s! \n",errmsg);
	fprintf(f," @ OPCODE: %s,\t\tA = %08x, B = %08x, C = %08x, Bx = %08x\n", luaP_opnames[GET_OPCODE(thisins)], GETARG_A(thisins), GETARG_B(thisins), GETARG_C(thisins), GETARG_Bx(thisins));
	fprintf(f,"''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''\n");
	fprintf(f,"' JIT state:                                                           '\n");
	fprintf(f,"''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''\n");
	stackdump(L,f);
	fprintf(f,"''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''\n");
	fprintf(f,"' Interpreter state:                                                   '\n");
	fprintf(f,"''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''''\n");
	stackdump(i_L,f);
	fprintf(f,"------------------------------------------------------------------------\n");
	fclose(f);
}
}

void ljit_debug_stackvalidator_init(lua_State* L)
{
	memcpy(i_L,L,sizeof(lua_State));
	i_L->stack = luaM_newvector(i_L,L->stacksize,TValue);
	i_L->base_ci = luaM_newvector(i_L,L->end_ci - L->size_ci,TValue);
	i_L->stack_last = stackadjust(L,L->stack_last);
	i_L->base = stackadjust(L,L->base);
	i_L->top = stackadjust(L,L->top);
	i_L->ci = ciadjust(L,L->ci);
	i_L->end_ci = ciadjust(L,L->end_ci);
	memcpy(i_L->stack,L->stack,L->stacksize);
	memcpy(i_L->ci_base,L->ci_base,L->size_ci);
}

int ljit_debug_stackvalidator_check(lua_State* L, const Instruction* curins)
{
	StkId i,j;
	CallInfo* k,l;
	if(		i_L->base != stackadjust(L,L->base)
		||	i_L->top != stackadjust(L,L->top)
		||	i_L->stack_last != stackadjust(L,L->stack_last)
		||	i_L->ci != ciadjust(L,L->ci)
		||	i_L->end_ci != ciadjust(L,L->end_ci)
		||	i_L->stacksize != L->stacksize
		||	i_L->size_ci != L->size_ci
		||	i_L->ci->func != stackadjust(L,L->ci->func)
		||	i_L->ci->base != stackadjust(L,L->ci->base)
		||	i_L->ci->top != stackadjust(L,L->ci->top))
	{
		statedump(L,curins,"stack or CI pointers do not match");
		return 0;
	}
#ifdef LJIT_DEBUG_STACK_CONSISTENCY_STRICT
	for(i = L->stack, j = i_L->stack; i <= L->stack_last; i++, j++)
#else
	for(i = L->stack, j = i_L->stack; i < ((L->top > L->ci->top) ? L->top : L->ci->top); i++, j++)
#endif
	{
		if(i->tt != j->tt)
		{
			char buf[200];
			sprintf(buf,"stack entry types do not match at idx %08x",i - L->stack);
			statedump(L,curins,buf);
			return 0;
		}
		if((i->tt == LUA_TBOOLEAN || i->tt == LUA_TNUMBER) && (i->value.na[0] != j->value.na[0] || i->value.na[1] != j->value.na[1]))
		{
			char buf[200];
			sprintf(buf,"boolean or numerical value does not match at idx %08x",i - L->stack);
			statedump(L,curins,buf);
			return 0;
		}
		if((i->tt == LUA_TSTRING) && strcmp(svalue(i),svalue(j) != 0))
		{
			char buf[200];
			sprintf(buf,"string value does not match at idx %08x",i - L->stack);
			statedump(L,curins,buf);
			return 0;
		}
	}
	for(k = L->base_ci, l = i_L->base_ci; k <= L->ci; i++, j++)
	{
		if(L->ci->func != i_L->ci->func || L->ci->base != i_L->ci->base || L->ci->top != i_L->ci->top)
		{
			char buf[200];
			sprintf(buf,"CI data does not match at idx %08x",k - L->base_ci);
			statedump(L,curins,"CI data does not match");
			return 0;
		}
	}
	return 1;
}

void ljit_debug_stackvalidator_step()
{

}
