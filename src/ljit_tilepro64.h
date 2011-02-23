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
//|
//|// Types
//|.type GL,			global_State
#define Dt6(_V) (int)&(((global_State *)0)_V)
# 13 "ljit_tilepro64.dash"
//|.type TVALUE,		TValue
#define Dt7(_V) (int)&(((TValue *)0)_V)
# 14 "ljit_tilepro64.dash"
//|.type VALUE,		Value
#define Dt8(_V) (int)&(((Value *)0)_V)
# 15 "ljit_tilepro64.dash"
//|.type CINFO,		CallInfo
#define Dt9(_V) (int)&(((CallInfo *)0)_V)
# 16 "ljit_tilepro64.dash"
//|.type GCOBJECT,	GCObject
#define DtA(_V) (int)&(((GCObject *)0)_V)
# 17 "ljit_tilepro64.dash"
//|.type TSTRING,		TString
#define DtB(_V) (int)&(((TString *)0)_V)
# 18 "ljit_tilepro64.dash"
//|.type TABLE,		Table
#define DtC(_V) (int)&(((Table *)0)_V)
# 19 "ljit_tilepro64.dash"
//|.type CCLOSURE,	CClosure
#define DtD(_V) (int)&(((CClosure *)0)_V)
# 20 "ljit_tilepro64.dash"
//|.type PROTO,		Proto
#define DtE(_V) (int)&(((Proto *)0)_V)
# 21 "ljit_tilepro64.dash"
//|.type UPVAL,		UpVal
#define DtF(_V) (int)&(((UpVal *)0)_V)
# 22 "ljit_tilepro64.dash"
//|.type NODE,		Node
#define Dt10(_V) (int)&(((Node *)0)_V)
# 23 "ljit_tilepro64.dash"
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
//|	auli dst, zero, ha16(simm32)
//|.endmacro
//|
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
//|	loadnvalueki r26, r27, vptr
//|	storenvaluek tv.value, r26, r27
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
//|	movewi tv.value, vptr
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
	DASM_ESC,
	DASM_SECTION,
	DASM_STOP,
};

#define lo16(n) (((signed int)n << 16) >> 16)
#define hi16(n) ((signed int)n >> 16)
#define ha16(n) ((lo16(n) < 0) ? hi16(n) + 1 : hi16(n))


# 7 "ljit_tilepro64.dasc"
//|.immencmodes jit_encmodes
enum	jit_encmodes {
	IEM_X0_Imm8 = 0,
	IEM_X1_J,
	IEM_X1_Br,
	IEM_X1_Shift,
	IEM_X0_Imm16,
	IEM_X1_J_jal,
};
# 8 "ljit_tilepro64.dasc"
//|.actionlist jit_actionlist
static const signed long jit_actionlist[1176] = {
  DASM_SECTION,0,DASM_ALIGN,63,DASM_LABEL_G,10,1880510464,142720704,-127562,
  1074497536,1076923801,1074497536,1880510464,142695200,1076940185,1074497536,
  1880510464,142693152,1076956569,1074497536,1880510464,142697248,1076972953,
  1074497536,1880510464,142701344,1076989337,1074497536,1880510464,142699296,
  13627499,1074497536,13627434,1074497536,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629164,
  1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629166,1074497536,536873626,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,13629165,1074497536,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,1076893467,
  1074497536,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1077022105,1074497536,1880510464,
  142611232,536873882,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,1880510464,135398240,536873626,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,13629165,1074497536,13630235,1074497536,536873754,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,536873818,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,13629211,1074497536,536873818,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  1880510464,142662464,536873818,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,
  1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,13629211,1074497536,
  536873818,1074497536,DASM_IMM,IEM_X0_Imm16,DASM_STOP,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1077022105,1074497536,1880510464,
  1074492193,-266973184,671092800,1082134469,1074497536,DASM_IMM,IEM_X0_Imm8,
  9719940,1074497536,8397124,1074497536,1880510464,1074020482,10506372,1074497536,
  6574212,1074497536,805611,1074497536,DASM_LABEL_L,1,13631451,1074497536,536873754,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  1880510464,142662464,1076890412,1074497536,DASM_IMM,IEM_X0_Imm8,22199040,
  1074497536,-266973184,738197497,DASM_LABEL_L,2,536873626,1074497536,DASM_IMM,
  IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,
  -2277,1074497536,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1076989337,1074497536,-266973184,
  1074492214,1076972953,1074497536,1880510464,1074492215,1076956569,1074497536,
  1880510464,1074492214,1076940185,1074497536,1880510464,1074492213,1076923801,
  1074497536,-266973184,1074492213,1082134464,1074497536,DASM_IMM,IEM_X0_Imm8,
  1077022134,1074497536,-266973184,1074493147,1880510464,135661280,DASM_ALIGN,
  63,DASM_LABEL_G,11,536873882,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629126,1074497536,536871322,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,1085277888,1074497536,DASM_IMM,IEM_X0_Imm8,-266973184,
  671090688,536871322,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629125,1074497536,13627739,
  1074497536,536873882,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1880510464,135790752,DASM_LABEL_L,
  1,13630299,1074497536,536873818,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,
  1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630235,1074497536,
  536873754,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,
  IEM_X0_Imm16,DASM_STOP,1880510464,142662464,536873818,1074497536,DASM_IMM,
  IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,
  13629211,1074497536,536873818,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,
  1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630080,1074497536,
  13630145,1074497536,-62,1074497536,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,24492779,
  1074497536,1880510464,0,DASM_IMM,IEM_X1_J_jal,-266973184,671092737,13630080,
  1074497536,1082138561,1074497536,1880510464,0,DASM_IMM,IEM_X1_J_jal,DASM_LABEL_L,
  2,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,
  IEM_X0_Imm16,-266973184,1074492237,899819,1074497536,536873626,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,-266973184,
  1074492237,13629164,1074497536,1880510464,135661280,DASM_ALIGN,63,DASM_LABEL_G,
  12,DASM_ALIGN,63,DASM_STOP,1076890370,1074497536,DASM_IMM,IEM_X0_Imm8,536873626,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,22130816,1074497536,-266973184,671113216,536873626,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,17414976,1074497536,1076894573,1074497536,-266973184,
  671115265,13630171,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1076890347,
  1074497536,DASM_IMM,IEM_X0_Imm8,13630299,1074497536,536873818,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,DASM_STOP,1076890305,1074497536,DASM_IMM,IEM_X0_Imm8,DASM_STOP,
  13630188,1074497536,DASM_STOP,20978435,1074497536,-266973184,671090785,13627500,
  1074497536,DASM_LABEL_L,1,DASM_STOP,1076890305,1074497536,DASM_IMM,IEM_X0_Imm8,
  13631451,1074497536,536873818,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,
  1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13627483,1074497536,
  536873818,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,13627483,1074497536,536873626,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,13630171,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,13630171,
  1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,13631451,1074497536,
  536873754,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,DASM_STOP,DASM_LABEL_L,2,13631451,1074497536,
  536873754,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,13631451,1074497536,536873754,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,1076890412,1074497536,DASM_IMM,IEM_X0_Imm8,22027011,1074497536,
  -266973184,738197497,DASM_STOP,13630080,1074497536,13630145,1074497536,1880510464,
  0,DASM_IMM,IEM_X1_J_jal,DASM_STOP,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629165,
  1074497536,1076890347,1074497536,DASM_IMM,IEM_X0_Imm8,536873818,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,-266973184,
  1074492237,13629122,1074497536,22203072,1074497536,-266973184,671092736,DASM_LABEL_L,
  1,-266973184,1074492768,1076906731,1074497536,1880510464,142608448,1076904066,
  1074497536,22203072,1074497536,-266973184,738197497,DASM_LABEL_L,2,536873818,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,13629163,1074497536,1076890477,1074497536,DASM_IMM,
  IEM_X0_Imm8,13627564,1074497536,13630299,1074497536,536873818,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,1880510464,135661280,DASM_STOP,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,1076889371,
  1074497536,536873626,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_IMM,IEM_X0_Imm8,1076890347,
  1074497536,DASM_IMM,IEM_X0_Imm8,DASM_STOP,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,13629211,
  1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536873690,1074497536,DASM_IMM,
  IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,1074492238,
  13629211,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,
  1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,536873690,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  1074492238,13629211,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,
  1076890348,1074497536,DASM_IMM,IEM_X0_Imm8,1880510464,135661280,DASM_STOP,
  1076890347,1074497536,DASM_IMM,IEM_X0_Imm8,536873626,1074497536,DASM_IMM,
  IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,
  13629165,1074497536,DASM_STOP,1076890348,1074497536,DASM_IMM,IEM_X0_Imm8,
  DASM_STOP,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,13629166,1074497536,536874971,
  1074497536,536873818,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_IMM,IEM_X0_Imm16,805310427,
  1074497536,536873818,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_IMM,IEM_X0_Imm16,536873882,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  -266973184,1074492237,1880510464,135398240,1076890347,1074497536,DASM_IMM,
  IEM_X0_Imm8,13630171,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,
  13631451,1074497536,536873754,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,
  1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,1076890306,
  1074497536,DASM_IMM,IEM_X0_Imm8,DASM_LABEL_L,1,13631451,1074497536,536873754,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  1880510464,142662464,13631451,1074497536,536873754,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1076890412,
  1074497536,DASM_IMM,IEM_X0_Imm8,21768960,1074497536,-266973184,738197497,
  DASM_STOP,1076890348,1074497536,DASM_IMM,IEM_X0_Imm8,13630235,1074497536,
  536873754,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,DASM_STOP,1082134491,1074497536,536873690,
  1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,
  1880510464,142662464,DASM_STOP,1082138585,1074497536,13629019,1074497536,
  536873690,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,
  IEM_X0_Imm16,1880510464,142662464,13629019,1074497536,536873690,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,DASM_STOP,1082134491,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,1082138587,
  1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,805308058,1074497536,
  DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,536874969,1074497536,
  DASM_IMM,IEM_X0_Imm16,805307993,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  1074492205,1076905563,1074497536,-266973184,1074492269,536873690,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,-266973184,
  1074492237,1880510464,142660448,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,-266973184,1074492237,1076905689,
  1074497536,1880510464,142662432,1082146779,1074497536,536873690,1074497536,
  DASM_IMM,IEM_X0_Imm16,805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,
  142662464,DASM_STOP,536874971,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_IMM,
  IEM_X0_Imm16,805310427,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_IMM,
  IEM_X0_Imm16,1082150875,1074497536,536873690,1074497536,DASM_IMM,IEM_X0_Imm16,
  805308058,1074497536,DASM_IMM,IEM_X0_Imm16,1880510464,142662464,DASM_STOP,
  1880510464,135661280,DASM_STOP,536874946,1074497536,DASM_IMM,IEM_X0_Imm16,
  805310402,1074497536,DASM_IMM,IEM_X0_Imm16,1076891778,1074497536,-40964159,
  1074497536,13630080,1074497536,1880510464,0,DASM_IMM,IEM_X1_J_jal,DASM_STOP
};

# 9 "ljit_tilepro64.dasc"
//|.globals JSUB_
enum {
  JSUB_GATE_LJ,
  JSUB_GATE_JL,
  JSUB_GROW_STACK,
  JSUB_GROW_CI,
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
	//|  .align 16
	//|->name:
	//|.endmacro
	//|.macro .endjsub;  .endcapture; .endmacro
	//|.macro .dumpjsub;  .dumpcapture JSUB; .endmacro
	//|
	//|.code
	dasm_put(Dst, 0);
# 67 "ljit_tilepro64.dasc"
	//|//-----------------------------------------------------------------------
	//|// Procedure: GATE_LJ(lua_State* L, TValue func, int nresults)
	//|//---------------------
	//|.align 64
	//|->GATE_LJ:
	//|	prologue 32
	//|	// Preserve "global type" registers.
	//|	storeonstack BASE, 8
	//|	storeonstack L, 12
	//|	storeonstack TOP, 16
	//|	storeonstack LCL, 20
	//|	storeonstack CI, 24
	//|
	//|	// Init our global types
	//|	move BASE, r1
	//|	move L, r0
	//|	move TOP, L->top
	//|	move LCL, BASE->value
	//|	move CI, L->ci
	//|	// Prevent stackless yield. TODO: figure out what a stackless yield is.
	//|	addi L->nCcalls, L->nCcalls, 1
	//|
	//|	// Store number of requested results.
	//|	storeonstack r2, 32
	//|
	//|	// Call the gate, probably not compiled yet, but may be compiled.
	//|	jalr LCL->jit_gate
	//|
	//|	// Set callinfo, set lua state 
	//|	move CI, L->ci
	//|	move L->top, TOP
	//|	move L->savedpc, CI->savedpc
	//|	move L->base, CI->base
	dasm_put(Dst, 2, lo16(Dt1(->top)), ha16(Dt1(->top)), lo16(Dt2(->value)), ha16(Dt2(->value)), lo16(Dt1(->ci)), ha16(Dt1(->ci)), lo16(Dt1(->nCcalls)), ha16(Dt1(->nCcalls)), lo16(Dt1(->nCcalls)), ha16(Dt1(->nCcalls)), lo16(Dt5(->jit_gate)), ha16(Dt5(->jit_gate)), lo16(Dt1(->ci)), ha16(Dt1(->ci)), lo16(Dt1(->top)), ha16(Dt1(->top)), lo16(Dt4(->savedpc)), ha16(Dt4(->savedpc)), lo16(Dt1(->savedpc)), ha16(Dt1(->savedpc)), lo16(Dt4(->base)), ha16(Dt4(->base)), lo16(Dt1(->base)));
# 100 "ljit_tilepro64.dasc"
	//|
	//|	// Check if arg 3 was LUA_MULTRET
	//|	loadfromstack r2, 32 
	//|	bz r2, >2		// If so, we can skip this
	//|
	//|	// Multiply number of values on stack by value size
	//|	movei r5, #TOP
	//|	mulw_uu r4, r2, r5
	//|
	//|	// Get index right above the last nresults. All 
	//|	add BASE, BASE, r4
	//|
	//|1:
	//|	move TOP->tt, zero
	//|	addidx TOP, TOP, 1
	//|	slt_u r0, TOP, BASE
	//|	bnzt r0, <1
	//|
	//|2:
	//|	addi L->nCcalls, L->nCcalls, -1
	//|
	//|	// Restore "global type" registers
	//|	loadfromstack CI, 24
	//|	loadfromstack LCL, 20
	//|	loadfromstack TOP, 16
	//|	loadfromstack L, 12
	//|	loadfromstack BASE, 8
	//|
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
	//|	move L->top, TOP
	dasm_put(Dst, 171, ha16(Dt1(->base)), sizeof(TValue), lo16(Dt3(->tt)), ha16(Dt3(->tt)), (1)*sizeof(TValue), lo16(Dt1(->nCcalls)), ha16(Dt1(->nCcalls)), lo16(Dt1(->nCcalls)), ha16(Dt1(->nCcalls)), PCRC, lo16(Dt5(->p)), ha16(Dt5(->p)), lo16(DtE(->jit_status)), ha16(DtE(->jit_status)), JIT_S_OK, lo16(DtE(->jit_mcode)), ha16(DtE(->jit_mcode)), lo16(Dt5(->jit_gate)), ha16(Dt5(->jit_gate)), lo16(Dt1(->ci)), ha16(Dt1(->ci)), lo16(Dt1(->top)), ha16(Dt1(->top)));
# 150 "ljit_tilepro64.dasc"
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
	//|->GROW_STACK:
	//|	
	//|.align 64
	//|//-----------------------------------------------------------------------
	dasm_put(Dst, 358, lo16(Dt4(->savedpc)), ha16(Dt4(->savedpc)), lo16(Dt1(->savedpc)), ha16(Dt1(->savedpc)), lo16(Dt1(->stack)), ha16(Dt1(->stack)), &luaD_precall, &luaV_execute, lo16(Dt1(->stack)), ha16(Dt1(->stack)), lo16(Dt1(->top)), ha16(Dt1(->top)));
# 172 "ljit_tilepro64.dasc"

	dasm_checkstep(Dst, DASM_SECTION_CODE);
	status = luaJIT_link(J, &J->jsubmcode, &J->szjsubmcode);
	if (status != JIT_S_OK)
		return status;
	
	/* Copy the callgates from the globals to the global state. */
	G(J->L)->jit_gateLJ = (luaJIT_GateLJ)J->jsub[JSUB_GATE_LJ];
	G(J->L)->jit_gateJL = (lua_CFunction)J->jsub[JSUB_GATE_JL];
	//G(J->L)->jit_gateJC = (lua_CFunction)J->jsub[JSUB_GATE_JC];
	return JIT_S_OK;
}

# 203 "ljit_tilepro64.dasc"

/* Encode JIT function prologue. */
static void jit_prologue(jit_State *J)
{
	Proto *pt = J->pt;
	int numparams = pt->numparams;
	int stacksize = pt->maxstacksize;
	
	//|	addi r2, TOP, stacksize
	//|	slt_u r0, r2, L->stack_last
	//|	bz r0, ->GROW_STACK
	//|  // This is a slight overallocation (BASE[1+stacksize] would be enough).
	//|  // We duplicate luaD_precall() behaviour so we can use luaD_growstack().
	//|	seq r0, CI, L->end_ci
	//|   addi CI, CI, 1
	//|	bnz r0, ->GROW_CI			// CI overflow?
	//|	move CI->func, BASE
	//|	addidx BASE, BASE, 1
	//|   move L->ci, CI
	dasm_put(Dst, 449, stacksize, lo16(Dt1(->stack_last)), ha16(Dt1(->stack_last)), lo16(Dt1(->end_ci)), ha16(Dt1(->end_ci)), lo16(Dt4(->func)), ha16(Dt4(->func)), (1)*sizeof(TValue), lo16(Dt1(->ci)), ha16(Dt1(->ci)));
# 222 "ljit_tilepro64.dasc"
	
	if (numparams > 0) {
	  //|  addi r1, BASE, numparams
	  dasm_put(Dst, 512, numparams);
# 225 "ljit_tilepro64.dasc"
	}
	
	if (!pt->is_vararg) {  /* Fixarg function. */
		/* Must cap L->top at L->base+numparams because 1st LOADNIL is omitted. */
		if (numparams == 0) {
		  	//|  move TOP, BASE
		  	dasm_put(Dst, 517);
# 231 "ljit_tilepro64.dasc"
		} else {
		    //|	slte_u r3, TOP, r1
			//|	bnz r3, >1
			//|	move TOP, r1
			//|1:
			dasm_put(Dst, 520);
# 236 "ljit_tilepro64.dasc"
		}
		//|	addi r1, BASE, stacksize	// New ci->top.
		//|	move CI->tailcalls, zero		// 0
		//|   move CI->top, r1
		//|   move L->top, r1
		//|	move L->base, BASE
		//|	move CI->base, BASE
		dasm_put(Dst, 529, stacksize, lo16(Dt4(->tailcalls)), ha16(Dt4(->tailcalls)), lo16(Dt4(->top)), ha16(Dt4(->top)), lo16(Dt1(->top)), ha16(Dt1(->top)), lo16(Dt1(->base)), ha16(Dt1(->base)), lo16(Dt4(->base)), ha16(Dt4(->base)));
# 243 "ljit_tilepro64.dasc"
	}
# 280 "ljit_tilepro64.dasc"
	
	/* Clear undefined args and all vars. Still LUA_TNIL = 0. */
	/* Note: cannot clear only args because L->top has grown. */
	if (stacksize <= EXTRA_STACK) {  /* Loopless clear. May use EXTRA_STACK. */
	  int i;
	  for (i = 0; i < stacksize; i++) {
	    //|  move TOP[i].tt, zero
	    dasm_put(Dst, 594, lo16(Dt3([i].tt)), ha16(Dt3([i].tt)));
# 287 "ljit_tilepro64.dasc"
	  }
	} else {  /* Standard loop. */
	  //|2:  // Unrolled for 2 stack slots. No initial check. May use EXTRA_STACK.
	  //|  move TOP[0].tt, zero
	  //|  move TOP[1].tt, zero
	  //|  addidx TOP, TOP, 2
	  //|  slt_u r3, TOP, r1
	  //|  bnz r3, <2
	  //|// Note: TOP is undefined now. TOP is only valid across calls/open ins.
	  dasm_put(Dst, 607, lo16(Dt3([0].tt)), ha16(Dt3([0].tt)), lo16(Dt3([1].tt)), ha16(Dt3([1].tt)), (2)*sizeof(TValue));
# 296 "ljit_tilepro64.dasc"
	}

# 305 "ljit_tilepro64.dasc"
}

/* Check if we can combine 'return const'. */
static int jit_return_k(jit_State *J)
{
	return 0;
# 334 "ljit_tilepro64.dasc"
}

static void jit_op_return(jit_State *J, int rbase, int nresults)
{
# 366 "ljit_tilepro64.dasc"

  /* May need to close open upvalues. */
	if (!fhint_isset(J, NOCLOSE)) {
		//|	move r0, L
		//|	move r1, BASE
	    //|	jal &luaF_close
	    dasm_put(Dst, 642, &luaF_close);
# 372 "ljit_tilepro64.dasc"
	}
	
	/* Previous op was open: 'return f()' or 'return ...' */
	if (nresults < 0) {
		//|// Relocate [BASE+rbase, TOP) -> [ci->func, *).
		//|	move CI, L->ci
		//|	addidx BASE, BASE, rbase
		//|	move r2, CI->func
		//|	slt_u r0, BASE, TOP
		//|	bz r0, >2
		//|1:
		//|	lw r1, BASE
		//|	addi BASE, BASE, 4
		//|	sw r2, r1
		//|	addi r2, r2, 4
		//|	slt_u r0, BASE, TOP
		//|	bnz r0, <1
		//|2:
		//|	move BASE, CI->func
		//|	addi CI, CI, -#CI
		//|	move TOP, r2			// Relocated TOP.
		//|	move L->ci, CI
		//|	jrp lr
		dasm_put(Dst, 651, lo16(Dt1(->ci)), ha16(Dt1(->ci)), (rbase)*sizeof(TValue), lo16(Dt4(->func)), ha16(Dt4(->func)), lo16(Dt4(->func)), ha16(Dt4(->func)), -sizeof(CallInfo), lo16(Dt1(->ci)), ha16(Dt1(->ci)));
# 395 "ljit_tilepro64.dasc"
		return;
	}
	
	if (!J->pt->is_vararg) {  /* Fixarg function, nresults >= 0. */
	  int i;
	  //|  addi L->ci, L->ci, -#CI
	  //|// Relocate [BASE+rbase,BASE+rbase+nresults) -> [BASE-1, *).
	  //|// TODO: loop for large nresults?
	  //|  addi BASE, BASE, -#BASE
	  dasm_put(Dst, 732, lo16(Dt1(->ci)), ha16(Dt1(->ci)), lo16(Dt1(->ci)), ha16(Dt1(->ci)), -sizeof(CallInfo), -sizeof(TValue));
# 404 "ljit_tilepro64.dasc"
	  for (i = 0; i < nresults; i++) {
	    //|  copyslot BASE[i], BASE[rbase+i+1]
	    dasm_put(Dst, 761, lo16(Dt2([rbase+i+1].value)), ha16(Dt2([rbase+i+1].value)), lo16(Dt2([i].value)), ha16(Dt2([i].value)), lo16(Dt2([rbase+i+1].value.na[1])), ha16(Dt2([rbase+i+1].value.na[1])), lo16(Dt2([i].value.na[1])), ha16(Dt2([i].value.na[1])), lo16(Dt2([rbase+i+1].tt)), ha16(Dt2([rbase+i+1].tt)), lo16(Dt2([i].tt)), ha16(Dt2([i].tt)));
# 406 "ljit_tilepro64.dasc"
	  }
	  //|  addidx TOP, BASE, nresults
	  //|  jrp lr
	  dasm_put(Dst, 828, (nresults)*sizeof(TValue));
# 409 "ljit_tilepro64.dasc"
	}
# 427 "ljit_tilepro64.dasc"
}

static void jit_op_call(jit_State *J, int func, int nargs, int nresults)
{
# 435 "ljit_tilepro64.dasc"
  //|  addidx BASE, BASE, func
  //|  move CI, L->ci
  //|//   isfunction 0			// BASE[0] is L->base[func].
  dasm_put(Dst, 835, (func)*sizeof(TValue), lo16(Dt1(->ci)), ha16(Dt1(->ci)));
# 438 "ljit_tilepro64.dasc"
  if (nargs >= 0) {  /* Previous op was not open and did not set TOP. */
    //|  addidx TOP, BASE, 1+nargs
    dasm_put(Dst, 852, (1+nargs)*sizeof(TValue));
# 440 "ljit_tilepro64.dasc"
  }
  //|  move LCL, BASE->value
  //|  movewi CI->savedpc, J->nextins
# 469 "ljit_tilepro64.dasc"
  //|  jalr LCL->jit_gate		// Call JIT func or GATE_JL/GATE_JC.
  //|  subidx BASE, BASE, func
  //|  move L->base, BASE
  dasm_put(Dst, 857, lo16(Dt2(->value)), ha16(Dt2(->value)), lo16(Dt4(->savedpc)), ha16(Dt4(->savedpc)), lo16(J->nextins), lo16(Dt4(->savedpc)), ha16(Dt4(->savedpc)), ha16(J->nextins), lo16(Dt5(->jit_gate)), ha16(Dt5(->jit_gate)), -(func)*sizeof(TValue), lo16(Dt1(->base)), ha16(Dt1(->base)));
# 472 "ljit_tilepro64.dasc"

  /* Clear undefined results TOP <= o < func+nresults. */
  if (nresults > 0) {
    if (nresults <= EXTRA_STACK) {  /* Loopless clear. May use EXTRA_STACK. */
      int i;
      for (i = 0; i < nresults; i++) {
	//|  move TOP[i].tt, zero
	dasm_put(Dst, 926, lo16(Dt3([i].tt)), ha16(Dt3([i].tt)));
# 479 "ljit_tilepro64.dasc"
      }
    } else {  /* Standard loop. TODO: move to .tail? */
      //|  addidx r2, BASE, func+nresults
      //|1:  // Unrolled for 2 stack slots. No initial check. May use EXTRA_STACK.
      //|  move TOP[0].tt, zero			// LUA_TNIL
      //|  move TOP[1].tt, zero			// LUA_TNIL
      //|  addidx TOP, TOP, 2
      //|  slt r0, TOP, r2
      //|  bnzt r0, <1
      dasm_put(Dst, 939, (func+nresults)*sizeof(TValue), lo16(Dt3([0].tt)), ha16(Dt3([0].tt)), lo16(Dt3([1].tt)), ha16(Dt3([1].tt)), (2)*sizeof(TValue));
# 488 "ljit_tilepro64.dasc"
    }
  }

  if (nresults >= 0) {  /* Not an open ins. Restore L->top. */
    //|  addidx TOP, BASE, J->pt->maxstacksize  // Faster than getting L->ci->top.
    //|  move L->top, TOP
    dasm_put(Dst, 978, (J->pt->maxstacksize)*sizeof(TValue), lo16(Dt1(->top)), ha16(Dt1(->top)));
# 494 "ljit_tilepro64.dasc"
  }  /* Otherwise keep TOP for next instruction. */
}

static void jit_op_tailcall(jit_State *J, int func, int nargs)
{
# 598 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_move(jit_State *J, int dest, int src)
{
# 607 "ljit_tilepro64.dasc"
}

static void jit_op_loadk(jit_State *J, int dest, int kidx)
{
  const TValue *kk = &J->pt->k[kidx];
  int rk = jit_return_k(J);
  if (rk) dest = 0;
  //|  copyconst BASE[dest], kk
  switch (ttype(kk)) {
  case 0:
  dasm_put(Dst, 995, lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
    break;
  case 1:
  if (bvalue(kk)) {  /* true */
  dasm_put(Dst, 1008, lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
  } else {  /* false */
  dasm_put(Dst, 1035, lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
  }
    break;
  case 3: {
  dasm_put(Dst, 1060, lo16(&(kk)->value), ha16(&(kk)->value), lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
    break;
  }
  case 4:
  dasm_put(Dst, 1113, lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), lo16(gcvalue(kk)), lo16(Dt2([dest].value)), ha16(Dt2([dest].value)), ha16(gcvalue(kk)), lo16(Dt2([dest].tt)), ha16(Dt2([dest].tt)));
    break;
  default: lua_assert(0); break;
  }
# 615 "ljit_tilepro64.dasc"
  if (rk) {
    //|  jrp lr
    dasm_put(Dst, 1154);
# 617 "ljit_tilepro64.dasc"
  }
}

static void jit_op_loadnil(jit_State *J, int first, int last)
{
# 643 "ljit_tilepro64.dasc"
}

static void jit_op_loadbool(jit_State *J, int dest, int b, int dojump)
{
# 660 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_getupval(jit_State *J, int dest, int uvidx)
{
# 672 "ljit_tilepro64.dasc"
}

static void jit_op_setupval(jit_State *J, int src, int uvidx)
{
# 707 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

/* Optimized table lookup routines. Enter via jsub, fallback to C. */

/* Fallback for GETTABLE_*. Temporary key is in L->env. */
static void jit_gettable_fb(lua_State *L, Table *t, StkId dest)
{
# 827 "ljit_tilepro64.dasc"
}

/* Fallback for SETTABLE_*STR. Temporary (string) key is in L->env. */
static void jit_settable_fb(lua_State *L, Table *t, StkId val)
{
# 966 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_newtable(jit_State *J, int dest, int lnarray, int lnhash)
{
# 977 "ljit_tilepro64.dasc"
}

static void jit_op_getglobal(jit_State *J, int dest, int kidx)
{
  const TValue *kk = &J->pt->k[kidx];
  jit_assert(ttisstring(kk));
  //|  movewi TSTRING:r2, &kk->value.gc->ts
  //|  addi r2, r2, 1
  //|  //addidx BASE, BASE, dest
  //|  moveli r1, -10002
  //|  move r0, L
  //|  jal lua_getfield
  dasm_put(Dst, 1157, lo16(&kk->value.gc->ts), ha16(&kk->value.gc->ts), lua_getfield);
# 989 "ljit_tilepro64.dasc"
}

static void jit_op_setglobal(jit_State *J, int rval, int kidx)
{
# 1000 "ljit_tilepro64.dasc"
}

enum { TKEY_KSTR = -2, TKEY_STR = -1, TKEY_ANY = 0 };

/* Optimize key lookup depending on consts or hints type. */
static int jit_keylookup(jit_State *J, int tab, int rkey)
{
# 1066 "ljit_tilepro64.dasc"
  return TKEY_ANY;  /* Use fallback. */
}

static void jit_op_gettable(jit_State *J, int dest, int tab, int rkey)
{
# 1152 "ljit_tilepro64.dasc"
}

static void jit_op_settable(jit_State *J, int tab, int rkey, int rval)
{
# 1251 "ljit_tilepro64.dasc"
}

static void jit_op_self(jit_State *J, int dest, int tab, int rkey)
{
# 1259 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_setlist(jit_State *J, int ra, int num, int batch)
{
# 1331 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_arith(jit_State *J, int dest, int rkb, int rkc, int ev)
{
# 1590 "ljit_tilepro64.dasc"
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
# 1656 "ljit_tilepro64.dasc"
}

static void jit_op_not(jit_State *J, int dest, int rb)
{
# 1676 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_concat(jit_State *J, int dest, int first, int last)
{
# 1758 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_eq(jit_State *J, int cond, int rkb, int rkc)
{
# 1851 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_test(jit_State *J, int cond, int dest, int src)
{
# 1891 "ljit_tilepro64.dasc"
}

static void jit_op_jmp(jit_State *J, int target)
{
# 1898 "ljit_tilepro64.dasc"
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
# 1971 "ljit_tilepro64.dasc"
}

static void jit_op_forloop(jit_State *J, int ra, int target)
{
# 2007 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_tforloop(jit_State *J, int ra, int nresults)
{
# 2026 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_close(jit_State *J, int ra)
{
# 2041 "ljit_tilepro64.dasc"
}

static void jit_op_closure(jit_State *J, int dest, int ptidx)
{
# 2091 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_vararg(jit_State *J, int dest, int num)
{
# 2164 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

void luaJIT_debugnotify(jit_State *J)
{
}

