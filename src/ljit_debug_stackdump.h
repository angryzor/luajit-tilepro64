/*
 * ljit_debug_stackdump.h
 *
 *  Created on: Feb 28, 2012
 *      Author: rtytgat
 */

#ifndef LJIT_DEBUG_STACKDUMP_H_
#define LJIT_DEBUG_STACKDUMP_H_

#include "lua.h"
#include "lobject.h"
#include "lstate.h"

void ljit_debug_initstackdump();
void ljit_debug_releasestackdump();
void ljit_debug_dumpstack(TValue* base, TValue* top, CallInfo* ci, lua_State* L, const char* opname, int ra, int rb, int rc, int rbx);

#endif /* LJIT_DEBUG_STACKDUMP_H_ */
