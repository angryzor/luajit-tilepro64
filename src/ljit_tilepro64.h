/*
** This file has been pre-processed with DynASM.
** http://luajit.org/dynasm.html
** DynASM version 1.1.4, DynASM tilepro64 version 0.0.2
** DO NOT EDIT! The original file is in "ljit_tilepro64.dasc".
*/

#if DASM_VERSION != 10104
#error "Version mismatch between DynASM and included encoding engine"
#endif

# 1 "ljit_tilepro64.dasc"
/*
 * Random shit
 */

//|.include ljit_tilepro64.dash
# 1 "ljit_tilepro64.dash"
//|.arch tilepro64
//|.section code
#define DASM_SECTION_CODE	0
#define DASM_MAXSECTION		1
# 3 "ljit_tilepro64.dash"
//|
//|// Types
//|.type L,		lua_State,	r42
#define Dt1(_V) (int)&(((lua_State *)0)_V)
# 6 "ljit_tilepro64.dash"
//|.type BASE,	TValue,		r43
#define Dt2(_V) (int)&(((TValue *)0)_V)
# 7 "ljit_tilepro64.dash"
//|.type TOP,		TValue,		r44
#define Dt3(_V) (int)&(((TValue *)0)_V)
# 8 "ljit_tilepro64.dash"
//|.type CI,		CallInfo,	r45
#define Dt4(_V) (int)&(((CallInfo *)0)_V)
# 9 "ljit_tilepro64.dash"
//|.type LCL,		LClosure,	r46
#define Dt5(_V) (int)&(((LClosure *)0)_V)
# 10 "ljit_tilepro64.dash"
//|.type CTOP,	TValue,		r47
#define Dt6(_V) (int)&(((TValue *)0)_V)
# 11 "ljit_tilepro64.dash"
//|
//|// Types
//|.type GL,			global_State
#define Dt7(_V) (int)&(((global_State *)0)_V)
# 14 "ljit_tilepro64.dash"
//|.type TVALUE,		TValue
#define Dt8(_V) (int)&(((TValue *)0)_V)
# 15 "ljit_tilepro64.dash"
//|.type VALUE,		Value
#define Dt9(_V) (int)&(((Value *)0)_V)
# 16 "ljit_tilepro64.dash"
//|.type CINFO,		CallInfo
#define DtA(_V) (int)&(((CallInfo *)0)_V)
# 17 "ljit_tilepro64.dash"
//|.type GCOBJECT,	GCObject
#define DtB(_V) (int)&(((GCObject *)0)_V)
# 18 "ljit_tilepro64.dash"
//|.type TSTRING,		TString
#define DtC(_V) (int)&(((TString *)0)_V)
# 19 "ljit_tilepro64.dash"
//|.type TABLE,		Table
#define DtD(_V) (int)&(((Table *)0)_V)
# 20 "ljit_tilepro64.dash"
//|.type CCLOSURE,	CClosure
#define DtE(_V) (int)&(((CClosure *)0)_V)
# 21 "ljit_tilepro64.dash"
//|.type PROTO,		Proto
#define DtF(_V) (int)&(((Proto *)0)_V)
# 22 "ljit_tilepro64.dash"
//|.type UPVAL,		UpVal
#define Dt10(_V) (int)&(((UpVal *)0)_V)
# 23 "ljit_tilepro64.dash"
//|.type NODE,		Node
#define Dt11(_V) (int)&(((Node *)0)_V)
# 24 "ljit_tilepro64.dash"
//|
//|// Definitions copied to DynASM domain to avoid unnecessary constant args.
//|// CHECK: must match with the definitions in lua.h!
//|.define LUA_TNIL,				0
//|.define LUA_TBOOLEAN,			1
//|.define LUA_TLIGHTUSERDATA,	2
//|.define LUA_TNUMBER,			3
//|.define LUA_TSTRING,			4
//|.define LUA_TTABLE,			5
//|.define LUA_TFUNCTION,			6
//|.define LUA_TUSERDATA,			7
//|.define LUA_TTHREAD,			8
//|
//|.define LUA_TNUM_NUM,		0x33
//|.define LUA_TNUM_NUM_NUM,	0x333
//|.define LUA_TSTR_STR,		0x44
//|.define LUA_TSTR_NUM,		0x43
//|.define LUA_TSTR_NUM_NUM,	0x433
//|.define LUA_TTABLE_NUM,	0x53
//|.define LUA_TTABLE_STR,	0x54
//|
//|
//|// Assembler pseudo instructions. Should actually be put in dynasm, putting them here for now
//|.macro move, dst, src; or dst, src, zero; .endmacro
//|.macro movei, dst, simm8; ori dst, zero, simm8; .endmacro
//|.macro moveli, dst, simm16; addli dst, zero, simm16; .endmacro
//|.macro movelis, dst, simm16; addlis dst, zero, simm16; .endmacro
//|.macro prefetch, src; lb_u zero, src; .endmacro
//|.macro bpt; ill; .endmacro
//|.macro info, simm8; andi zero, zero, simm8; .endmacro
//|.macro infol, simm16; auli zero, zero, simm16; .endmacro
//|
//|// Own pseudo instructions to handle 32 bit stuff easier
//|.macro movewi, dst, simm32
//|	addli dst, zero, lo16(simm32)
//|	auli dst, dst, ha16(simm32)
//|.endmacro
//|
//|.macro movemwi, dst, simm32
//|	addli r25, zero, lo16(simm32)
//|	auli r25, r25, ha16(simm32)
//|	move dst, r25
//|.endmacro
//|
//|//==================================================================
//|// Pseudo instructions specific to this project
//|// Stack-related
//|.macro prologue, size
//|	sw sp, lr
//|	addi sp, sp, -size
//|.endmacro
//|
//|.macro epilogue, size
//|	addi sp, sp, size
//|	lw lr, sp
//|.endmacro
//|
//|.macro loadfromstack, reg, idx
//|	addi r25, sp, idx
//|	lw reg, r25
//|.endmacro
//|
//|.macro storeonstack, reg, idx
//|	addi r25, sp, idx
//|	sw r25, reg
//|.endmacro
//|
//|//==========================================================================
//|
//|.macro preserveglobalregs
//|	storeonstack BASE, 8
//|	storeonstack L, 12
//|	storeonstack TOP, 16
//|	storeonstack LCL, 20
//|	storeonstack CI, 24
//|.endmacro
//|
//|.macro restoreglobalregs
//|	loadfromstack CI, 24
//|	loadfromstack LCL, 20
//|	loadfromstack TOP, 16
//|	loadfromstack L, 12
//|	loadfromstack BASE, 8
//|.endmacro
//|
//|//============================================================================
//|
//|.macro globals_LJ_L, l
//|	move L, l
//|.endmacro
//|
//|.macro globals_LJ_BASE, base
//|	move BASE, base
//|.endmacro
//|
//|.macro globals_LJ_TOP
//|	move TOP, L->top
//|.endmacro
//|
//|.macro globals_LJ_LCL
//|	move LCL, BASE->value
//|.endmacro
//|
//|.macro globals_LJ_CI
//|	move CI, L->ci
//|.endmacro
//|
//|.macro globals_LJ, base
//|	globals_LJ_BASE base
//|	globals_LJ_TOP
//|	globals_LJ_LCL
//|	globals_LJ_CI
//|.endmacro
//|
//|.macro globals_JL_BASE
//|	move L->base, BASE
//|.endmacro
//|
//|.macro globals_JL_CI
//|	move L->ci, CI
//|.endmacro
//|
//|.macro globals_JL_savedpc
//|	move L->savedpc, CI->savedpc
//|.endmacro
//|
//|.macro globals_JL_TOP
//|	move L->top, TOP
//|.endmacro
//|
//|.macro globals_JL
//|	globals_JL_BASE
//|	globals_JL_CI
//|	globals_JL_TOP
//|	globals_JL_savedpc
//|.endmacro
//|//=========================================================================
//|
//|.define LUAFUNC_STACK_SIZE, 8
//|
//|// Easier word multiply
//|.macro mulw_uu, d, a, b
//|	mulhl_uu d, a, b
//|	mulhla_uu d, b, a
//|	shli d, d, 8
//|	mullla_uu d, a, b
//|	mulhhsa_uu d, a, b
//|.endmacro
//|
//|.macro mulw_ss, d, a, b
//|	mulhl_ss d, a, b
//|	mulhla_ss d, b, a
//|	shli d, d, 8
//|	mullla_ss d, a, b
//|	mulhhsa_ss d, a, b
//|.endmacro
//|
//|.macro istt, dst, idx, tp; seqi dst, BASE[idx].tt, tp; .endmacro
//|.macro isnil, dst, idx; istt dst, idx, LUA_TNIL; .endmacro
//|.macro isnumber, dst, idx;  istt dst, idx, LUA_TNUMBER; .endmacro
//|.macro isstring, dst, idx;  istt dst, idx, LUA_TSTRING; .endmacro
//|.macro istable, dst, idx;  istt dst, idx, LUA_TTABLE; .endmacro
//|.macro isfunction, dst, idx;  istt dst, idx, LUA_TFUNCTION; .endmacro
//|
//|.macro settt, val, tp; move val.tt, tp; .endmacro
//|.macro settti, val, tp; movei val.tt, tp; .endmacro
//|
//|
//|.macro copyslot, D, S
//|	move D.value, S.value
//|	move D.value.na[1], S.value.na[1]
//|	move D.tt, S.tt
//|.endmacro
//|
//|
//|.macro copyconst, tv, tvk
//||switch (ttype(tvk)) {
//||case LUA_TNIL:
//|   setnilvalue tv
//||  break;
//||case LUA_TBOOLEAN:
//|   setbvalue tv, bvalue(tvk)
//||  break;
//||case LUA_TNUMBER: {
//|   setnvaluek tv, &(tvk)->value
//||  break;
//||}
//||case LUA_TSTRING:
//|   setsvalue tv, gcvalue(tvk)
//||  break;
//||default: lua_assert(0); break;
//||}
//|.endmacro
//|
//|
//|
//|
//|.macro loadnvaluek, reg1, reg2, src
//|	lw reg1, src
//|	addi reg2, src, 4
//|	lw reg2, reg2
//|.endmacro
//|
//|.macro storenvaluek, dst, reg1, reg2
//|	sw dst, reg1
//|	addi r25, dst, 4
//|	sw r25, reg2
//|.endmacro
//|
//|.macro loadnvalueki, reg1, reg2, src
//|	addli r25, zero, lo16(src)
//|	auli r25, r25, ha16(src)
//|	lw reg1, r25
//|	addi reg2, r25, 4
//|	lw reg2, reg2
//|.endmacro
//|
//|
//|.macro setnvaluek, tv, vptr
//|	loadnvalueki r23, r24, vptr
//|	storenvaluek tv.value, r23, r24
//|	settti tv, LUA_TNUMBER
//|.endmacro
//|
//|.macro setbvalue, tv, val		// May use edx.
//||if (val) {  /* true */
//|   movei r25, LUA_TBOOLEAN
//|   move tv.value, r25		// Assumes: LUA_TBOOLEAN == 1
//|   settt tv, r25
//||} else {  /* false */
//|   movei tv.value, 0
//|   settti tv, LUA_TBOOLEAN
//||}
//|.endmacro
//|
//|.macro setsvalue, tv, vptr
//|	movemwi tv.value, vptr
//|	settti tv, LUA_TSTRING
//|.endmacro
//|
//|.macro setnilvalue, tv
//|	settti tv, LUA_TNIL
//|.endmacro
//|
//|.macro addidx, dst, src, idx
//|	addi dst, src, (idx)*#src
//|.endmacro
//|
//|.macro subidx, dst, src, idx
//|	addi dst, src, -(idx)*#src
//|.endmacro
//|
# 6 "ljit_tilepro64.dasc"
//|.actionnames jit_actionnames
enum	jit_actionnames {
	DASM_IMM = 2147483637,
	DASM_L,
	DASM_G,
	DASM_PC,
	DASM_LABEL_L,
	DASM_LABEL_G,
	DASM_LABEL_PC,
	DASM_ALIGN,
	DASM_SECTION,
	DASM_ESC,
	DASM_STOP,
};

#define lo16(n) (((signed int)n << 16) >> 16)
#define hi16(n) ((signed int)n >> 16)
#define ha16(n) ((lo16(n) < 0) ? hi16(n) + 1 : hi16(n))


# 7 "ljit_tilepro64.dasc"
//|.immencmodes
#include "dasm_tilepro64_encmodes.h"
# 8 "ljit_tilepro64.dasc"
//|.actionlist jit_actionlist
static const signed long jit_actionlist[1398] = {
  1880510464,142720704,1077906870,1074472960,1076890313,1074472960,DASM_IMM,
  IEM_X0_Imm8,1082134465,1074472960,DASM_IMM,IEM_X0_Imm8,536873626,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,
  1074492237,22131264,1074472960,-266973184,671088641,DASM_L,IEM_X1_Br,11,1880510464,
  0,DASM_G,IEM_X1_J_jal,0,DASM_LABEL_L,1,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,17414976,
  1074472960,-266973184,671088640,DASM_L,IEM_X1_Br,12,1880510464,0,DASM_G,IEM_X1_J_jal,
  1,DASM_LABEL_L,2,1076890477,1074472960,DASM_IMM,IEM_X0_Imm8,13627995,1074472960,
  536873818,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,DASM_STOP,DASM_LABEL_L,7,1082134491,1074472960,
  536871450,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,1076888072,1074472960,DASM_IMM,IEM_X0_Imm8,
  22200832,1074472960,-266973184,671088641,DASM_L,IEM_X1_Br,7,DASM_STOP,DASM_SECTION,
  0,DASM_ALIGN,7,DASM_LABEL_G,2,1880510464,142720704,1077808566,1074472960,
  1076923801,1074472960,1880510464,142695200,1076940185,1074472960,1880510464,
  142693152,1076956569,1074472960,1880510464,142697248,1076972953,1074472960,
  1880510464,142701344,1076989337,1074472960,1880510464,142699296,13627434,
  1074472960,13627499,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629164,
  1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629166,1074472960,536873626,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,13629165,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,1076893467,
  1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1077005721,1074472960,1880510464,
  142611232,536873882,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,1880510464,135398240,1077005721,
  1074472960,1880510464,1074492193,1086320768,1074472960,-266973184,671088641,
  DASM_L,IEM_X1_Br,12,1082134469,1074472960,DASM_IMM,IEM_X0_Imm8,9719940,1074472960,
  8397124,1074472960,1880510464,1074020482,10506372,1074472960,6574212,1074472960,
  805576,1074472960,536873818,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629164,1074472960,DASM_LABEL_L,
  1,1082134491,1074472960,536871450,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1076888072,1074472960,
  DASM_IMM,IEM_X0_Imm8,22200832,1074472960,-266973184,671088641,DASM_L,IEM_X1_Br,
  1,DASM_LABEL_L,2,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,1077933851,1074472960,536873626,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  1880510464,142662464,536873818,1074472960,DASM_IMM,IEM_X0_Imm16,DASM_STOP,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629163,
  1074472960,13630171,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630299,
  1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630235,1074472960,536873626,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  1880510464,142662464,536873818,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,13629211,1074472960,
  536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,1076989337,1074472960,-266973184,1074492214,
  1076972953,1074472960,1880510464,1074492215,1076956569,1074472960,1880510464,
  1074492214,1076940185,1074472960,1880510464,1074492213,1076923801,1074472960,
  -266973184,1074492213,1082134464,1074472960,DASM_IMM,IEM_X0_Imm8,1077022134,
  1074472960,-266973184,1074493147,1880510464,135661280,DASM_ALIGN,7,DASM_LABEL_G,
  3,536873882,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,
  IEM_X0_Imm16,-266973184,1074492237,13629126,1074472960,536871322,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,
  1074492237,1085277888,1074472960,DASM_IMM,IEM_X0_Imm8,-266973184,671088640,
  DASM_L,IEM_X1_Br,11,536871322,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629125,1074472960,
  13627739,1074472960,536873882,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1880510464,135790752,
  DASM_LABEL_L,1,13630299,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,DASM_STOP,1880510464,142662464,
  13630235,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536873818,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,
  1074492238,13629211,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630080,
  1074472960,13630145,1074472960,1083178946,1074472960,536873626,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,
  1074492237,24492779,1074472960,1880510464,0,DASM_IMM,IEM_X1_J_jal,-266973184,
  671088641,DASM_L,IEM_X1_Br,12,13630080,1074472960,1082138561,1074472960,1880510464,
  0,DASM_IMM,IEM_X1_J_jal,DASM_LABEL_L,2,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,899819,1074472960,
  536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,
  IEM_X0_Imm16,-266973184,1074492237,13629164,1074472960,1880510464,135661280,
  DASM_ALIGN,7,DASM_LABEL_G,4,DASM_STOP,13630171,1074472960,536873818,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629127,1074472960,1076890347,
  1074472960,DASM_IMM,IEM_X0_Imm8,13630171,1074472960,536873818,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,13630171,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630299,
  1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630235,1074472960,536873626,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  1880510464,142662464,536873818,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,13629211,1074472960,
  536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,13630080,1074472960,536871386,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,
  1074492237,1880510464,135398240,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629163,
  1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,DASM_STOP,-266973184,1074492237,13629164,1074472960,
  536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,
  IEM_X0_Imm16,-266973184,1074492237,13629166,1074472960,536873626,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,
  1074492237,13629165,1074472960,1076890477,1074472960,DASM_IMM,IEM_X0_Imm8,
  1076890347,1074472960,DASM_IMM,IEM_X0_Imm8,1082134470,1074472960,DASM_IMM,
  IEM_X0_Imm8,9723907,1074472960,8388995,1074472960,-266973184,1074020449,10510339,
  1074472960,6578179,1074472960,24394500,1074472960,13630152,1074472960,22200577,
  1074472960,-266973184,671088672,DASM_L,IEM_X1_Br,12,DASM_LABEL_L,1,536871194,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  1880510464,1074492238,13629211,1074472960,536871450,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536871194,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  1880510464,1074492238,13629211,1074472960,536871450,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536871194,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  1880510464,1074492238,13629211,1074472960,536871450,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1076887812,
  1074472960,DASM_IMM,IEM_X0_Imm8,1076888072,1074472960,DASM_IMM,IEM_X0_Imm8,
  22200577,1074472960,-266973184,671088673,DASM_L,IEM_X1_Br,1,DASM_LABEL_L,
  2,DASM_STOP,801516,1074472960,1076923830,1074472960,-266973184,1074493147,
  1880510464,135661280,DASM_ALIGN,7,DASM_LABEL_G,0,1880510464,142720704,1077906870,
  1074472960,13630235,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630080,
  1074472960,1880510464,0,DASM_IMM,IEM_X1_J_jal,1076923830,1074472960,-266973184,
  1074493147,1880510464,135661280,DASM_ALIGN,7,DASM_LABEL_G,1,1880510464,142720704,
  1077906870,1074472960,13630235,1074472960,536873626,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630080,
  1074472960,1880510464,0,DASM_IMM,IEM_X1_J_jal,1076923830,1074472960,-266973184,
  1074493147,1880510464,135661280,DASM_ALIGN,7,DASM_STOP,DASM_ALIGN,7,65535,
  0,0,0,0,0,DASM_STOP,1076890347,1074472960,DASM_IMM,IEM_X0_Imm8,DASM_STOP,
  13628012,1074472960,1076890312,1074472960,DASM_IMM,IEM_X0_Imm8,DASM_STOP,
  13630080,1074472960,13630145,1074472960,1880510464,0,DASM_IMM,IEM_X1_J_jal,
  DASM_STOP,1076890477,1074472960,DASM_IMM,IEM_X0_Imm8,1076890347,1074472960,
  DASM_IMM,IEM_X0_Imm8,DASM_STOP,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,13629211,
  1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536873690,1074472960,DASM_IMM,
  IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,
  13629211,1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536873690,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,
  1074492238,13629211,1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,
  1076890312,1074472960,DASM_IMM,IEM_X0_Imm8,DASM_STOP,1076890348,1074472960,
  DASM_IMM,IEM_X0_Imm8,DASM_STOP,1076923830,1074472960,-266973184,1074493147,
  1880510464,135661280,DASM_STOP,1076890347,1074472960,DASM_IMM,IEM_X0_Imm8,
  DASM_STOP,1076890348,1074472960,DASM_IMM,IEM_X0_Imm8,DASM_STOP,536873690,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,13629166,1074472960,536874969,1074472960,DASM_IMM,IEM_X0_Imm16,
  805307993,1074472960,DASM_IMM,IEM_X0_Imm16,13629019,1074472960,536873818,
  1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,
  1880510464,142662464,536873882,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,1880510464,135398240,
  1076890347,1074472960,DASM_IMM,IEM_X0_Imm8,DASM_STOP,1076890312,1074472960,
  DASM_IMM,IEM_X0_Imm8,DASM_STOP,536873818,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629164,
  1074472960,DASM_STOP,1082134491,1074472960,536873690,1074472960,DASM_IMM,
  IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,
  DASM_STOP,1082138585,1074472960,13629019,1074472960,536873690,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,13629019,1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,
  1082134491,1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1082138587,1074472960,
  536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,DASM_STOP,536874969,1074472960,DASM_IMM,
  IEM_X0_Imm16,805307993,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492203,
  1076905560,1074472960,1880510464,1074492172,536873690,1074472960,DASM_IMM,
  IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,
  1880510464,142654304,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,1076905689,1074472960,
  1880510464,142656288,1082146779,1074472960,536873690,1074472960,DASM_IMM,
  IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,
  DASM_STOP,536874969,1074472960,DASM_IMM,IEM_X0_Imm16,805307993,1074472960,
  DASM_IMM,IEM_X0_Imm16,13629019,1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1082150875,
  1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,1880510464,135661280,
  DASM_STOP,536874946,1074472960,DASM_IMM,IEM_X0_Imm16,805306498,1074472960,
  DASM_IMM,IEM_X0_Imm16,1076887682,1074472960,DASM_IMM,IEM_X0_Imm8,764342209,
  1074472960,13630080,1074472960,13630235,1074472960,536873626,1074472960,DASM_IMM,
  IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,
  1880510464,0,DASM_IMM,IEM_X1_J_jal,536873754,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,13629211,
  1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536873754,1074472960,DASM_IMM,
  IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,
  13629211,1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,
  1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536873754,1074472960,
  DASM_IMM,IEM_X0_Imm16,805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,
  1074492238,13629211,1074472960,536873690,1074472960,DASM_IMM,IEM_X0_Imm16,
  805308058,1074472960,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1082134491,
  1074472960,536873754,1074472960,DASM_IMM,IEM_X0_Imm16,805308058,1074472960,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP
};

# 9 "ljit_tilepro64.dasc"
//|.globals JSUB_
enum {
  JSUB_GROW_STACK,
  JSUB_GROW_CI,
  JSUB_GATE_LJ,
  JSUB_GATE_JL,
  JSUB_GATE_JC,
  JSUB__MAX
};
# 10 "ljit_tilepro64.dasc"

/* ------------------------------------------------------------------------ */

/* Arch string. */
const char luaJIT_arch[] = "tilepro64";

/* Forward declarations for C functions called from jsubs. */
static void jit_hookins(lua_State *L, const Instruction *newpc);
static void jit_gettable_fb(lua_State *L, Table *t, StkId dest);
static void jit_settable_fb(lua_State *L, Table *t, StkId val);

/* ------------------------------------------------------------------------ */

/* Detect CPU features and set JIT flags. */
static int jit_cpudetect(jit_State *J)
{
	return JIT_S_OK;
}

/* Check some assumptions. Should compile to nop. */
static int jit_consistency_check(jit_State *J)
{
/*	do {*/
		/* Force a compiler error for inconsistent structure sizes. */
		/* Check LUA_TVALUE_ALIGN in luaconf.h, too. */
/*		||int check_TVALUE_SIZE_in_ljit_x86_dash[1+TVALUE_SIZE-sizeof(TValue)];
		int check_TVALUE_SIZE_in_ljit_x86_dash_[1+sizeof(TValue)-TVALUE_SIZE];
		((void)check_TVALUE_SIZE_in_ljit_x86_dash[0]);
		((void)check_TVALUE_SIZE_in_ljit_x86_dash_[0]);
		if (LUA_TNIL != 0 || LUA_TBOOLEAN != 1 || PCRLUA != 0) break;
		if ((int)&(((Node *)0)->i_val) != (int)&(((StkId)0)->value)) break;*/
		return JIT_S_OK;
/*	} while (0);
	J->dasmstatus = 999999999;*/  /* Recognizable error. */
/*	return JIT_S_COMPILER_ERROR;*/
}

static int jit_std_prologue(jit_State *J, int stacksize)
{
	//|	prologue LUAFUNC_STACK_SIZE
	//|
	//|	addidx r9, BASE, stacksize+1
	//|	movei r1, stacksize+1
	//|	slt_u r0, r9, L->stack_last
	//|	bnz r0, >1
	//|	jal ->GROW_STACK
	//|1:
	//|	// This is a slight overallocation (BASE[1+stacksize] would be enough).
	//|	// We duplicate luaD_precall() behaviour so we can use luaD_growstack().
	//|	seq r0, CI, L->end_ci
	//|	bz r0, >2
	//|	jal ->GROW_CI			// CI overflow?
	//|2:
	//|	addidx CI, CI, 1
	//|	move CI->top, r9
	dasm_put(Dst, 0, (stacksize+1)*sizeof(TValue), stacksize+1, lo16(Dt1(->stack_last)), ha16(Dt1(->stack_last)), lo16(Dt1(->end_ci)), ha16(Dt1(->end_ci)), (1)*sizeof(CallInfo), lo16(Dt4(->top)), ha16(Dt4(->top)));
# 65 "ljit_tilepro64.dasc"
}

static int jit_wipe_r8_to_TOP(jit_State *J)
{
	//|7:
	//|	setnilvalue TVALUE:r8[0]		// the new TOP is always set to NIL at this point. i don't care. it don't hurt.
	//|	addi r8, r8, #TVALUE
	//|	slt_u r0, TVALUE:r8, TOP
	//|	bnzt r0, <7
	dasm_put(Dst, 77, lo16(Dt8([0].tt)), ha16(Dt8([0].tt)), sizeof(TValue));
# 74 "ljit_tilepro64.dasc"
}

/* Compile JIT subroutines (once). */
static int jit_compile_jsub(jit_State *J)
{
	int status = jit_consistency_check(J);
	if (status != JIT_S_OK) return status;
	status = jit_cpudetect(J);
	if (status != JIT_S_OK) return status;
	dasm_setup(Dst, jit_actionlist);
	//|// Macros to reorder and combine JIT subroutine definitions.
	//|.macro .jsub, name
	//|.capture JSUB			// Add the entry point.
	//||//-----------------------------------------------------------------------
	//||//->name:
	//|	.align 16
	//|->name:
	//|.endmacro
	//|.macro .endjsub;  .endcapture; .endmacro
	//|.macro .dumpjsub;  .dumpcapture JSUB; .endmacro
	//|
	//|.code
	dasm_put(Dst, 103);
# 96 "ljit_tilepro64.dasc"
	//|//-----------------------------------------------------------------------
	//|// Procedure: GATE_LJ(lua_State* L, TValue* func, int nresults)
	//|// L contains the Lua state
	//|// func is the stack frame base pointer for this functions
	//|// nresults is the amount of results the caller is expecting. rest should
	//|//			be truncated
	//|//---------------------
	//|.align 64
	//|->GATE_LJ:
	//|	prologue 32
	//|	// Preserve "global type" registers.
	//|	preserveglobalregs
	//|
	//|	// Init our global types
	//|	globals_LJ_L r0
	//|	globals_LJ r1
	//|
	//|	// Prevent stackless yield. TODO: figure out what a stackless yield is.
	//|	addi L->nCcalls, L->nCcalls, 1
	//|
	//|	// Store number of requested results.
	//|	storeonstack r2, 28
	//|
	//|	// Call the gate, probably not compiled yet, but may be compiled.
	//|	jalr LCL->jit_gate
	//|
	//|
	//|	// Check if arg 3 was LUA_MULTRET
	//|	loadfromstack r2, 28
	//|	seqi r0, r2, -1
	//|	bnz r0, >2		// If so, we can skip this
	//|
	//|	// Multiply number of values on stack by value size
	//|	movei r5, #TOP
	//|	mulw_uu r4, r2, r5
	//|
	//|	// Get index right above the last nresults. All 
	//|	add r8, BASE, r4
	//|
	//|	move TOP, CI->top
	//|1:
	//|	setnilvalue TVALUE:r8[0]		// the new TOP is always set to NIL at this point. i don't care. it don't hurt.
	//|	addi r8, r8, #TVALUE
	//|	slt_u r0, TVALUE:r8, TOP
	//|	bnzt r0, <1
	//|
	//|2:
	//|	addi L->nCcalls, L->nCcalls, -1
	//|
	//|	move BASE, CI->base
	dasm_put(Dst, 105, lo16(Dt1(->top)), ha16(Dt1(->top)), lo16(Dt2(->value)), ha16(Dt2(->value)), lo16(Dt1(->ci)), ha16(Dt1(->ci)), lo16(Dt1(->nCcalls)), ha16(Dt1(->nCcalls)), lo16(Dt1(->nCcalls)), ha16(Dt1(->nCcalls)), lo16(Dt5(->jit_gate)), ha16(Dt5(->jit_gate)), sizeof(TValue), lo16(Dt4(->top)), ha16(Dt4(->top)), lo16(Dt8([0].tt)), ha16(Dt8([0].tt)), sizeof(TValue), lo16(Dt1(->nCcalls)), ha16(Dt1(->nCcalls)), lo16(Dt1(->nCcalls)), ha16(Dt1(->nCcalls)), lo16(Dt4(->base)));
# 146 "ljit_tilepro64.dasc"
	//|	// Push our "fast" state to the "slow" interpreter state
	//|	globals_JL
	//|
	//|	// Restore "global type" registers
	//|	restoreglobalregs
	//|	movei r0, PCRC
	//|	
	//|	epilogue 32
	//|	jrp lr
	//|
	//|
	//|
	//|.align 64
	//|->GATE_JL:
	//|	move PROTO:r6, LCL->p
	//|	seqi r0, PROTO:r6->jit_status, JIT_S_OK
	//|	bz r0, >1		// Already compiled?
	//|
	//|	// Yes, copy callgate...
	//|	move r5, PROTO:r6->jit_mcode
	//|	move LCL->jit_gate, r5
	//|	jr r5
	//|
	//|	// No... Compile
	//|1:
	//|	move L->ci, CI
	dasm_put(Dst, 304, ha16(Dt4(->base)), lo16(Dt1(->base)), ha16(Dt1(->base)), lo16(Dt1(->ci)), ha16(Dt1(->ci)), lo16(Dt1(->top)), ha16(Dt1(->top)), lo16(Dt4(->savedpc)), ha16(Dt4(->savedpc)), lo16(Dt1(->savedpc)), ha16(Dt1(->savedpc)), PCRC, lo16(Dt5(->p)), ha16(Dt5(->p)), lo16(DtF(->jit_status)), ha16(DtF(->jit_status)), JIT_S_OK, lo16(DtF(->jit_mcode)), ha16(DtF(->jit_mcode)), lo16(Dt5(->jit_gate)), ha16(Dt5(->jit_gate)), lo16(Dt1(->ci)), ha16(Dt1(->ci)));
# 172 "ljit_tilepro64.dasc"
	//|	move L->top, TOP
	//|	move L->savedpc, CI->savedpc
	//|	move r0, L
	//|	move r1, BASE
	//|	movei r2, -1
	//|	sub BASE, BASE, L->stack
	//|	jal &luaD_precall
	//|	bnzt r0, >2
	//|
	//|	move r0, L
	//|	movei r1, 1
	//|	jal &luaV_execute
	//|
	//|2:
	//|	add BASE, BASE, L->stack
	//|	move TOP, L->top
	//|	jrp lr
	//|
	//|.align 64
	//|->GATE_JC:
	dasm_put(Dst, 474, lo16(Dt1(->top)), ha16(Dt1(->top)), lo16(Dt4(->savedpc)), ha16(Dt4(->savedpc)), lo16(Dt1(->savedpc)), ha16(Dt1(->savedpc)), lo16(Dt1(->stack)), ha16(Dt1(->stack)), &luaD_precall, &luaV_execute, lo16(Dt1(->stack)), ha16(Dt1(->stack)), lo16(Dt1(->top)), ha16(Dt1(->top)));
# 192 "ljit_tilepro64.dasc"

	jit_std_prologue(J, LUA_MINSTACK);

	//|	move CI->func, BASE
	//|	move CCLOSURE:r7, BASE->value
	//|	addidx BASE, BASE, 1
	//|	move CI->base, BASE
	//|	globals_JL
	//|
	//|	move r0, L
	//|	jalr CCLOSURE:r7->f
	//|
	//|	globals_LJ L->base
	dasm_put(Dst, 578, lo16(Dt4(->func)), ha16(Dt4(->func)), lo16(Dt2(->value)), ha16(Dt2(->value)), (1)*sizeof(TValue), lo16(Dt4(->base)), ha16(Dt4(->base)), lo16(Dt1(->base)), ha16(Dt1(->base)), lo16(Dt1(->ci)), ha16(Dt1(->ci)), lo16(Dt1(->top)), ha16(Dt1(->top)), lo16(Dt4(->savedpc)), ha16(Dt4(->savedpc)), lo16(Dt1(->savedpc)), ha16(Dt1(->savedpc)), lo16(DtE(->f)), ha16(DtE(->f)), lo16(Dt1(->base)), ha16(Dt1(->base)), lo16(Dt1(->top)), ha16(Dt1(->top)));
# 205 "ljit_tilepro64.dasc"
	//|
	//|	subidx CI, CI, 1
	//|	subidx BASE, BASE, 1
	//|
	//|	// Assume retval != LUA_MULTRET for now.
	//|	movei r6, #TOP
	//|	mulw_uu r3, r0, r6
	//|
	//|	sub r4, TOP, r3
	//|	move r8, BASE
	//|	slt_u r1, r4, TOP
	//|	bz r1, >2
	//|1:
	//|	copyslot TVALUE:r8[0], TVALUE:r4[0]
	//|	addi r4, r4, #TVALUE
	//|	addi r8, r8, #TVALUE
	//|	slt_u r1, r4, TOP
	//|	bnzt r1, <1
	//|2:
	dasm_put(Dst, 711, lo16(Dt2(->value)), ha16(Dt2(->value)), lo16(Dt1(->ci)), ha16(Dt1(->ci)), -(1)*sizeof(CallInfo), -(1)*sizeof(TValue), sizeof(TValue), lo16(Dt8([0].value)), ha16(Dt8([0].value)), lo16(Dt8([0].value)), ha16(Dt8([0].value)), lo16(Dt8([0].value.na[1])), ha16(Dt8([0].value.na[1])), lo16(Dt8([0].value.na[1])), ha16(Dt8([0].value.na[1])), lo16(Dt8([0].tt)), ha16(Dt8([0].tt)), lo16(Dt8([0].tt)), ha16(Dt8([0].tt)), sizeof(TValue), sizeof(TValue));
# 224 "ljit_tilepro64.dasc"
	jit_wipe_r8_to_TOP(J);
	//|	add TOP, BASE, r3
	//|	
	//|	epilogue LUAFUNC_STACK_SIZE
	//|	jrp lr
	//|	
	//|.align 64
	//|->GROW_STACK:
	//|	prologue 8
	//|	globals_JL_TOP
	//|	move r0, L
	//|	jal &luaD_growstack
	//|	epilogue 8
	//|	jrp lr
	//|.align 64
	//|->GROW_CI:
	//|	prologue 8
	//|	globals_JL_TOP
	//|	move r0, L
	//|	jal &luaD_growCI
	//|	epilogue 8
	//|	jrp lr
	//|.align 64
	//|//-----------------------------------------------------------------------
	dasm_put(Dst, 858, lo16(Dt1(->top)), ha16(Dt1(->top)), &luaD_growstack, lo16(Dt1(->top)), ha16(Dt1(->top)), &luaD_growCI);
# 248 "ljit_tilepro64.dasc"

	dasm_checkstep(Dst, DASM_SECTION_CODE);
	status = luaJIT_link(J, &J->jsubmcode, &J->szjsubmcode);
	if (status != JIT_S_OK)
		return status;
	
	/* Copy the callgates from the globals to the global state. */
	G(J->L)->jit_gateLJ = (luaJIT_GateLJ)J->jsub[JSUB_GATE_LJ];
	G(J->L)->jit_gateJL = (lua_CFunction)J->jsub[JSUB_GATE_JL];
	G(J->L)->jit_gateJC = (lua_CFunction)J->jsub[JSUB_GATE_JC];
	return JIT_S_OK;
}

# 279 "ljit_tilepro64.dasc"

/* Called after the last instruction has been encoded. */
static void jit_ins_last(jit_State *J, int lastpc, int sizemfm)
{
	//|	.align 64			// Keep next section word aligned.
	//|	.word 0x0000ffff			// Terminate mfm with JIT_MFM_STOP.
	//|	.space 5 // Next mcode block pointer and size.
	//|	// The previous two awords are only word, but not aword aligned.
	//|	// Copying them is easier than aligning them and adjusting mfm handling.
	dasm_put(Dst, 933);
# 288 "ljit_tilepro64.dasc"
}


/* Encode JIT function prologue. */
static void jit_prologue(jit_State *J)
{
	Proto *pt = J->pt;
	int numparams = pt->numparams;
	int stacksize = pt->maxstacksize;
	
	jit_std_prologue(J, stacksize);

	//|//	move CI->func, BASE
	//|	addidx BASE, BASE, 1
	dasm_put(Dst, 942, (1)*sizeof(TValue));
# 302 "ljit_tilepro64.dasc"
	
	if(!pt->is_vararg)
	{
		//|	move TOP, r9
		//|	addidx TVALUE:r8, BASE, numparams
		dasm_put(Dst, 947, (numparams)*sizeof(TValue));
# 307 "ljit_tilepro64.dasc"
		jit_wipe_r8_to_TOP(J);
	}
# 367 "ljit_tilepro64.dasc"
	
# 393 "ljit_tilepro64.dasc"
}

/* Check if we can combine 'return const'. */
static int jit_return_k(jit_State *J)
{
	return 0;
# 422 "ljit_tilepro64.dasc"
}

static void jit_op_return(jit_State *J, int rbase, int nresults)
{
# 454 "ljit_tilepro64.dasc"

	/* May need to close open upvalues. */
	if (!fhint_isset(J, NOCLOSE)) {
		//|	move r0, L
		//|	move r1, BASE
	    //|	jal &luaF_close
	    dasm_put(Dst, 954, &luaF_close);
# 460 "ljit_tilepro64.dasc"
	}

	/* Go to the previous CallInfo, load the current into r7 */
	//|//	move CINFO:r7, CI
	//|	subidx CI, CI, 1
	//|	subidx BASE, BASE, 1
	dasm_put(Dst, 963, -(1)*sizeof(CallInfo), -(1)*sizeof(TValue));
# 466 "ljit_tilepro64.dasc"

	/* Previous op was open: 'return f()' or 'return ...' */
	if (nresults < 0) {
# 489 "ljit_tilepro64.dasc"
	}
	else
	{
		int i;
		for(i = 0; i < nresults; i++)
		{
			//|	copyslot BASE[i], BASE[rbase+i+1]
			dasm_put(Dst, 972, lo16(Dt2([rbase+i+1].value)), ha16(Dt2([rbase+i+1].value)), lo16(Dt2([i].value)), ha16(Dt2([i].value)), lo16(Dt2([rbase+i+1].value.na[1])), ha16(Dt2([rbase+i+1].value.na[1])), lo16(Dt2([i].value.na[1])), ha16(Dt2([i].value.na[1])), lo16(Dt2([rbase+i+1].tt)), ha16(Dt2([rbase+i+1].tt)), lo16(Dt2([i].tt)), ha16(Dt2([i].tt)));
# 496 "ljit_tilepro64.dasc"
		}
		//|	addidx TVALUE:r8, BASE, nresults
		dasm_put(Dst, 1039, (nresults)*sizeof(TValue));
# 498 "ljit_tilepro64.dasc"
		jit_wipe_r8_to_TOP(J);
		//|	addidx TOP, BASE, nresults
		dasm_put(Dst, 1044, (nresults)*sizeof(TValue));
# 500 "ljit_tilepro64.dasc"
	}

	//|	epilogue LUAFUNC_STACK_SIZE
	//|	jrp lr
	dasm_put(Dst, 1049);
# 504 "ljit_tilepro64.dasc"
}

static void jit_op_call(jit_State *J, int func, int nargs, int nresults)
{
# 512 "ljit_tilepro64.dasc"
	//|	addidx BASE, BASE, func
	//|//  move CI, L->ci
	//|//   isfunction 0			// BASE[0] is L->base[func].
	dasm_put(Dst, 1056, (func)*sizeof(TValue));
# 515 "ljit_tilepro64.dasc"
	if (nargs >= 0) {  /* Previous op was not open and did not set TOP. */
		//|	addidx TOP, BASE, nargs+1
		dasm_put(Dst, 1061, (nargs+1)*sizeof(TValue));
# 517 "ljit_tilepro64.dasc"
	}
	//|	move LCL, BASE->value
	//|	movemwi CI->savedpc, J->nextins
# 546 "ljit_tilepro64.dasc"
	//|	jalr LCL->jit_gate		// Call JIT func or GATE_JL/GATE_JC.
	//|	subidx BASE, BASE, func
	dasm_put(Dst, 1066, lo16(Dt2(->value)), ha16(Dt2(->value)), lo16(J->nextins), ha16(J->nextins), lo16(Dt4(->savedpc)), ha16(Dt4(->savedpc)), lo16(Dt5(->jit_gate)), ha16(Dt5(->jit_gate)), -(func)*sizeof(TValue));
# 548 "ljit_tilepro64.dasc"

# 568 "ljit_tilepro64.dasc"
	if (nresults >= 0) {
		//|	addidx TVALUE:r8, BASE, nresults
		dasm_put(Dst, 1115, (nresults)*sizeof(TValue));
# 570 "ljit_tilepro64.dasc"
		jit_wipe_r8_to_TOP(J);
		//|	move TOP, CI->top
		dasm_put(Dst, 1120, lo16(Dt4(->top)), ha16(Dt4(->top)));
# 572 "ljit_tilepro64.dasc"
	} 
}

static void jit_op_tailcall(jit_State *J, int func, int nargs)
{
# 676 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_move(jit_State *J, int dest, int src)
{
# 685 "ljit_tilepro64.dasc"
}

static void jit_op_loadk(jit_State *J, int dest, int kidx)
{
	const TValue *kk = &J->pt->k[kidx];
	int rk = jit_return_k(J);
	if (rk) dest = 0;
	//|	copyconst BASE[dest], kk
	switch (ttype(kk)) {
	case 0:
	dasm_put(Dst, 1133, lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
	  break;
	case 1:
	if (bvalue(kk)) {  /* true */
	dasm_put(Dst, 1146, lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
	} else {  /* false */
	dasm_put(Dst, 1173, lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
	}
	  break;
	case 3: {
	dasm_put(Dst, 1198, lo16(&(kk)->value), ha16(&(kk)->value), lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
	  break;
	}
	case 4:
	dasm_put(Dst, 1251, lo16(gcvalue(kk)), ha16(gcvalue(kk)), lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
	  break;
	default: lua_assert(0); break;
	}
# 693 "ljit_tilepro64.dasc"
	if (rk) {
		//|	jrp lr
		dasm_put(Dst, 1284);
# 695 "ljit_tilepro64.dasc"
	}
}

static void jit_op_loadnil(jit_State *J, int first, int last)
{
# 721 "ljit_tilepro64.dasc"
}

static void jit_op_loadbool(jit_State *J, int dest, int b, int dojump)
{
# 738 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_getupval(jit_State *J, int dest, int uvidx)
{
# 750 "ljit_tilepro64.dasc"
}

static void jit_op_setupval(jit_State *J, int src, int uvidx)
{
# 785 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

/* Optimized table lookup routines. Enter via jsub, fallback to C. */

/* Fallback for GETTABLE_*. Temporary key is in L->env. */
static void jit_gettable_fb(lua_State *L, Table *t, StkId dest)
{
# 905 "ljit_tilepro64.dasc"
}

/* Fallback for SETTABLE_*STR. Temporary (string) key is in L->env. */
static void jit_settable_fb(lua_State *L, Table *t, StkId val)
{
# 1044 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_newtable(jit_State *J, int dest, int lnarray, int lnhash)
{
# 1055 "ljit_tilepro64.dasc"
}

static void jit_op_getglobal(jit_State *J, int dest, int kidx)
{
	/* At this point, we just do a call to the Lua getfield function here */
	const TValue *kk = &J->pt->k[kidx];
	jit_assert(ttisstring(kk));
	//|	movewi TSTRING:r2, &kk->value.gc->ts
	//|	addi r2, r2, #TSTRING
	//|	moveli r1, -10002	// LUA_GLOBALSINDEX
	//|	move r0, L
	//|
	//|	// lua_getfield puts something on the stack top, so set L->top
	//|	globals_JL_TOP
	//|	jal &lua_getfield
	//|
	//|	// Take the TValue that fell out of it and move to R(dest)
	//|	copyslot BASE[dest], TOP[0]
	//|	setnilvalue TOP[0]
	dasm_put(Dst, 1287, lo16(&kk->value.gc->ts), ha16(&kk->value.gc->ts), sizeof(TString), lo16(Dt1(->top)), ha16(Dt1(->top)), &lua_getfield, lo16(Dt3([0].value)), ha16(Dt3([0].value)), lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt3([0].value.na[1])), ha16(Dt3([0].value.na[1])), lo16(Dt2([dest].value.na[1])), ha16(Dt2([dest].value.na[1])), lo16(Dt3([0].tt)), ha16(Dt3([0].tt)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)), lo16(Dt3([0].tt)), ha16(Dt3([0].tt)));
# 1074 "ljit_tilepro64.dasc"
}

static void jit_op_setglobal(jit_State *J, int rval, int kidx)
{
# 1085 "ljit_tilepro64.dasc"
}

enum { TKEY_KSTR = -2, TKEY_STR = -1, TKEY_ANY = 0 };

/* Optimize key lookup depending on consts or hints type. */
static int jit_keylookup(jit_State *J, int tab, int rkey)
{
# 1151 "ljit_tilepro64.dasc"
	return TKEY_ANY;  /* Use fallback. */
}

static void jit_op_gettable(jit_State *J, int dest, int tab, int rkey)
{
# 1237 "ljit_tilepro64.dasc"
}

static void jit_op_settable(jit_State *J, int tab, int rkey, int rval)
{
# 1336 "ljit_tilepro64.dasc"
}

static void jit_op_self(jit_State *J, int dest, int tab, int rkey)
{
# 1344 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_setlist(jit_State *J, int ra, int num, int batch)
{
# 1416 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_arith(jit_State *J, int dest, int rkb, int rkc, int ev)
{
# 1675 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_fallback_len(lua_State *L, StkId ra, const TValue *rb)
{
	switch (ttype(rb)) {
	case LUA_TTABLE:
		setnvalue(ra, cast_num(luaH_getn(hvalue(rb))));
		break;
	case LUA_TSTRING:
		setnvalue(ra, cast_num(tsvalue(rb)->len));
		break;
	default: {
		const TValue *tm = luaT_gettmbyobj(L, rb, TM_LEN);
		if (ttisfunction(tm)) {
			ptrdiff_t rasave = savestack(L, ra);
			setobj2s(L, L->top, tm);
			setobj2s(L, L->top+1, rb);
			luaD_checkstack(L, 2);
			L->top += 2;
			luaD_call(L, L->top - 2, 1);
			ra = restorestack(L, rasave);
			L->top--;
			setobjs2s(L, ra, L->top);
		} else {
			luaG_typeerror(L, rb, "get length of");
		}
		break;
	}
	}
}

static void jit_op_len(jit_State *J, int dest, int rb)
{
# 1741 "ljit_tilepro64.dasc"
}

static void jit_op_not(jit_State *J, int dest, int rb)
{
# 1761 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_concat(jit_State *J, int dest, int first, int last)
{
# 1843 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_eq(jit_State *J, int cond, int rkb, int rkc)
{
# 1936 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_test(jit_State *J, int cond, int dest, int src)
{
# 1976 "ljit_tilepro64.dasc"
}

static void jit_op_jmp(jit_State *J, int target)
{
# 1983 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

enum { FOR_IDX, FOR_LIM, FOR_STP, FOR_EXT };

static const char *const jit_for_coerce_error[] = {
	LUA_QL("for") " initial value must be a number",
	LUA_QL("for") " limit must be a number",
	LUA_QL("for") " step must be a number",
};

/* Try to coerce for slots with strings to numbers in place or complain. */
static void jit_for_coerce(lua_State *L, TValue *o)
{
	int i;
	for (i = FOR_IDX; i <= FOR_STP; i++, o++) {
		lua_Number num;
		if (ttisnumber(o)) continue;
		if (ttisstring(o) && luaO_str2d(svalue(o), &num)) {
			setnvalue(o, num);
		} else {
			luaG_runerror(L, jit_for_coerce_error[i]);
		}
	}
}

static void jit_op_forprep(jit_State *J, int ra, int target)
{
# 2056 "ljit_tilepro64.dasc"
}

static void jit_op_forloop(jit_State *J, int ra, int target)
{
# 2092 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_tforloop(jit_State *J, int ra, int nresults)
{
# 2111 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_close(jit_State *J, int ra)
{
# 2126 "ljit_tilepro64.dasc"
}

static void jit_op_closure(jit_State *J, int dest, int ptidx)
{
# 2176 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_vararg(jit_State *J, int dest, int num)
{
# 2249 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

void luaJIT_debugnotify(jit_State *J)
{
}

