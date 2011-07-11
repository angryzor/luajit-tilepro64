/*
 * ljit_tilepro64_encmodes.h
 *
 *  Created on: 15-mrt-2011
 *      Author: rtytgat
 */

#ifndef LJIT_TILEPRO64_ENCMODES_H_
#define LJIT_TILEPRO64_ENCMODES_H_

typedef enum jit_encmodes {
	IEM_X0_Imm8 = 0,
	IEM_X0_Imm16,
	IEM_X1_Br,
	IEM_X1_Shift,
	IEM_X1_J,
	IEM_X1_J_jal,
	IEM_X1_J_j
} jit_encmodes;

#endif /* LJIT_TILEPRO64_ENCMODES_H_ */
