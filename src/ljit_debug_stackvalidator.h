/*
 * ljit_debug_stackvalidator.h
 *
 *  Created on: Dec 12, 2011
 *      Author: rtytgat
 */

#ifndef LJIT_DEBUG_STACKVALIDATOR_H_
#define LJIT_DEBUG_STACKVALIDATOR_H_

void jit_debug_stackvalidator_init(lua_State* L);
int jit_debug_stackvalidator_step(lua_State* L);
void jit_debug_stackvalidator_release();

#endif /* LJIT_DEBUG_STACKVALIDATOR_H_ */
