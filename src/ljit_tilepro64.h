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
//|
//|.macro settti, val, tp; movei val.tt, tp; .endmacro
//|
//|
//|.macro copyslot, D, S
//|	move D.value, S.value
//|	move D.value.na[1], S.value.na[1]
//|	move D.tt, S.tt
//|.endmacro
//|
//|.macro loadnumber, reg1, reg2, src
//|	lw reg1, src
//|	addi reg2, src, 4
//|	lw reg2, reg2
//|.endmacro
//|
//|.macro storenumber, dst, reg1, reg2
//|	sw dst, reg1
//|	addi r25, dst, 4
//|	sw r25, reg2
//|.endmacro
//|
//|.macro loadnumberi, reg1, reg2, src
//|	addli r25, zero, lo16(src)
//|	auli r25, r25, ha16(src)
//|	lw reg1, r25
//|	addi reg2, r25, 4
//|	lw reg2, reg2
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
	DASM_IMM = 2147483636,
	DASM_REF,
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
# 7 "ljit_tilepro64.dasc"
//|.actionlist jit_actionlist
static const signed long jit_actionlist[820] = {
  DASM_SECTION,0,DASM_ALIGN,8,DASM_LABEL_G,10,1880510464,142720704,1077808566,
  1074497536,1076923801,1074497536,1880510464,142695200,1076940185,1074497536,
  1880510464,142693152,1076956569,1074497536,1880510464,142697248,1076972953,
  1074497536,1880510464,142701344,1076989337,1074497536,1880510464,142699296,
  13627499,1074497536,13627434,1074497536,536873626,1074497536,DASM_IMM,805308058,
  1074497536,DASM_IMM,-266973184,1074492237,13629164,1074497536,536873690,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,13629166,1074497536,
  536873626,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,
  13629165,1074497536,536873626,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,
  1880510464,1074492238,1076893467,1074497536,536873626,1074497536,DASM_IMM,
  805308058,1074497536,DASM_IMM,1880510464,142662464,1077022105,1074497536,
  1880510464,142611232,536873882,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,
  DASM_STOP,-266973184,1074492237,1880510464,135398240,536873626,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,13629165,1074497536,
  13630235,1074497536,536873754,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,
  1880510464,142662464,536873818,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,
  1880510464,1074492238,13629211,1074497536,536873818,1074497536,DASM_IMM,805308058,
  1074497536,DASM_IMM,1880510464,142662464,536873818,1074497536,DASM_IMM,805308058,
  1074497536,DASM_IMM,1880510464,1074492238,13629211,1074497536,536873818,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,DASM_STOP,1880510464,142662464,1077022105,
  1074497536,1880510464,1074492193,-266973184,671092800,1082134469,1074497536,
  DASM_IMM,9719940,1074497536,8397124,1074497536,1880510464,1074020482,10506372,
  1074497536,6574212,1074497536,805611,1074497536,DASM_LABEL_L,1,13631451,1074497536,
  536873754,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,142662464,
  1076890412,1074497536,DASM_IMM,22199040,1074497536,-266973184,738195481,DASM_LABEL_L,
  2,536873626,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,
  1074492238,1077933851,1074497536,536873626,1074497536,DASM_IMM,805308058,
  1074497536,DASM_IMM,1880510464,142662464,1076989337,1074497536,-266973184,
  1074492214,1076972953,1074497536,1880510464,1074492215,1076956569,1074497536,
  1880510464,1074492214,1076940185,1074497536,1880510464,1074492213,1076923801,
  1074497536,-266973184,1074492213,1082134464,1074497536,DASM_IMM,1077022134,
  1074497536,-266973184,1074493147,1880510464,135661280,DASM_ALIGN,8,DASM_LABEL_G,
  11,536873882,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,
  1074492237,13629126,1074497536,536871322,1074497536,DASM_IMM,DASM_STOP,805308058,
  1074497536,DASM_IMM,-266973184,1074492237,1085277888,1074497536,DASM_IMM,
  -266973184,671090688,536871322,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,
  -266973184,1074492237,13629125,1074497536,13627739,1074497536,536873882,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,142662464,1880510464,135790752,
  DASM_LABEL_L,1,13630299,1074497536,536873818,1074497536,DASM_IMM,805308058,
  1074497536,DASM_IMM,1880510464,142662464,13630235,1074497536,536873754,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,142662464,536873818,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,DASM_STOP,1880510464,1074492238,13629211,
  1074497536,536873818,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,
  142662464,13630080,1074497536,13630145,1074497536,1083178946,1074497536,536873626,
  1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,24492779,
  1074497536,1880510464,0,DASM_REF,-266973184,671092737,13630080,1074497536,
  1082138561,1074497536,1880510464,0,DASM_REF,DASM_LABEL_L,2,536873626,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,899819,1074497536,
  536873626,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,
  13629164,1074497536,1880510464,135661280,DASM_ALIGN,8,DASM_LABEL_G,12,DASM_ALIGN,
  8,DASM_STOP,1076890370,1074497536,DASM_IMM,536873626,1074497536,DASM_IMM,
  805308058,1074497536,DASM_IMM,-266973184,1074492237,22130816,1074497536,-266973184,
  671113216,536873626,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,
  1074492237,17414976,1074497536,1076894573,1074497536,-266973184,671115265,
  1082134464,1074497536,13630171,1074497536,536873690,1074497536,DASM_IMM,805308058,
  1074497536,DASM_IMM,1880510464,142662464,1076890347,1074497536,DASM_IMM,13630299,
  1074497536,536873818,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,
  142662464,DASM_STOP,1076890305,1074497536,DASM_IMM,DASM_STOP,13630188,1074497536,
  DASM_STOP,20978435,1074497536,-266973184,671090785,13627500,1074497536,DASM_LABEL_L,
  1,DASM_STOP,1076890305,1074497536,DASM_IMM,13627419,1074497536,536873818,
  1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,142662464,13627483,
  1074497536,536873818,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,
  142662464,13627483,1074497536,536873626,1074497536,DASM_IMM,805308058,1074497536,
  DASM_IMM,1880510464,142662464,13630171,1074497536,536873690,1074497536,DASM_IMM,
  805308058,1074497536,DASM_IMM,1880510464,142662464,13630171,1074497536,536873690,
  1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,142662464,DASM_STOP,
  13627419,1074497536,536873754,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,
  1880510464,142662464,DASM_STOP,DASM_LABEL_L,2,13627419,1074497536,536873754,
  1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,142662464,13627419,
  1074497536,536873754,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,
  142662464,1076890412,1074497536,DASM_IMM,22027011,1074497536,-266973184,738193529,
  DASM_STOP,13630080,1074497536,13630145,1074497536,1880510464,0,DASM_REF,DASM_STOP,
  536873626,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,
  13629165,1074497536,1076890347,1074497536,DASM_IMM,536873818,1074497536,DASM_IMM,
  805308058,1074497536,DASM_IMM,-266973184,1074492237,13629122,1074497536,22203072,
  1074497536,-266973184,671092736,DASM_LABEL_L,1,-266973184,1074492768,1076906731,
  1074497536,1880510464,142608448,1076904066,1074497536,22203072,1074497536,
  -266973184,738195481,DASM_LABEL_L,2,536873818,1074497536,DASM_IMM,805308058,
  1074497536,DASM_IMM,-266973184,1074492237,13629163,1074497536,1076890477,
  1074497536,DASM_IMM,13627564,1074497536,13630299,1074497536,536873818,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,142662464,1880510464,135661280,
  DASM_STOP,536873626,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,
  1074492238,1076889371,1074497536,536873626,1074497536,DASM_IMM,805308058,
  1074497536,DASM_IMM,1880510464,142662464,DASM_IMM,1076890347,1074497536,DASM_IMM,
  DASM_STOP,536873690,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,1880510464,
  1074492238,13629211,1074497536,536873690,1074497536,DASM_IMM,805308058,1074497536,
  DASM_IMM,1880510464,142662464,536873690,1074497536,DASM_IMM,805308058,1074497536,
  DASM_IMM,1880510464,1074492238,13629211,1074497536,536873690,1074497536,DASM_IMM,
  805308058,1074497536,DASM_IMM,1880510464,142662464,536873690,1074497536,DASM_IMM,
  805308058,1074497536,DASM_IMM,1880510464,1074492238,13629211,1074497536,536873690,
  1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,DASM_STOP,1880510464,142662464,
  DASM_STOP,1076890348,1074497536,DASM_IMM,1880510464,135661280,DASM_STOP,536874969,
  1074497536,DASM_IMM,805307993,1074497536,DASM_IMM,1880510464,1074492192,1076905537,
  1074497536,-266973184,1074491424,DASM_STOP,536873690,1074497536,DASM_IMM,
  805308058,1074497536,DASM_IMM,-266973184,1074492237,1880510464,1074492256,
  536873690,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,
  1076905665,1074497536,-266973184,1074491424,DASM_STOP,536873690,1074497536,
  DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,1880510464,1074492257,
  536873690,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,1074492237,
  1076905667,1074497536,-266973184,1074491489,DASM_STOP,1880510464,0,DASM_REF,
  DASM_STOP,536873690,1074497536,DASM_IMM,805308058,1074497536,DASM_IMM,-266973184,
  1074492237,1880510464,142607200,536873690,1074497536,DASM_IMM,805308058,1074497536,
  DASM_IMM,-266973184,1074492237,1076905689,1074497536,1880510464,142609184,
  DASM_STOP,1082146779,1074497536,536873690,1074497536,DASM_IMM,805308058,1074497536,
  DASM_IMM,1880510464,142662464,DASM_STOP
};

# 8 "ljit_tilepro64.dasc"
//|.immencmodes jit_encmodes
enum	jit_encmodes {
	IEM_X0_Imm8 = 0,
	IEM_J,
	IEM_X1_Br,
	IEM_X1_Shift,
	IEM_X0_Imm16,
	IEM_J_jal,
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
	dasm_put(Dst, 2, IEM_X0_Imm16, lo16(Dt1(->top)), IEM_X0_Imm16, ha16(Dt1(->top)), IEM_X0_Imm16, lo16(Dt2(->value)), IEM_X0_Imm16, ha16(Dt2(->value)), IEM_X0_Imm16, lo16(Dt1(->ci)), IEM_X0_Imm16, ha16(Dt1(->ci)), IEM_X0_Imm16, lo16(Dt1(->nCcalls)), IEM_X0_Imm16, ha16(Dt1(->nCcalls)), IEM_X0_Imm16, lo16(Dt1(->nCcalls)), IEM_X0_Imm16, ha16(Dt1(->nCcalls)), IEM_X0_Imm16, lo16(Dt5(->jit_gate)), IEM_X0_Imm16, ha16(Dt5(->jit_gate)));
# 94 "ljit_tilepro64.dasc"
	//|
	//|	// Set callinfo, set lua state 
	//|	move CI, L->ci
	//|	move L->top, TOP
	//|	move L->savedpc, CI->savedpc
	//|	move L->base, CI->base
	dasm_put(Dst, 93, IEM_X0_Imm16, lo16(Dt1(->ci)), IEM_X0_Imm16, ha16(Dt1(->ci)), IEM_X0_Imm16, lo16(Dt1(->top)), IEM_X0_Imm16, ha16(Dt1(->top)), IEM_X0_Imm16, lo16(Dt4(->savedpc)), IEM_X0_Imm16, ha16(Dt4(->savedpc)), IEM_X0_Imm16, lo16(Dt1(->savedpc)), IEM_X0_Imm16, ha16(Dt1(->savedpc)), IEM_X0_Imm16, lo16(Dt4(->base)), IEM_X0_Imm16, ha16(Dt4(->base)), IEM_X0_Imm16, lo16(Dt1(->base)), IEM_X0_Imm16, ha16(Dt1(->base)));
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
	dasm_put(Dst, 152, IEM_X0_Imm8, sizeof(TValue), IEM_X0_Imm16, lo16(Dt3(->tt)), IEM_X0_Imm16, ha16(Dt3(->tt)), IEM_X0_Imm8, (1)*sizeof(TValue), IEM_X0_Imm16, lo16(Dt1(->nCcalls)), IEM_X0_Imm16, ha16(Dt1(->nCcalls)), IEM_X0_Imm16, lo16(Dt1(->nCcalls)), IEM_X0_Imm16, ha16(Dt1(->nCcalls)), IEM_X0_Imm8, PCRC, IEM_X0_Imm16, lo16(Dt5(->p)), IEM_X0_Imm16, ha16(Dt5(->p)), IEM_X0_Imm16, lo16(DtE(->jit_status)));
# 139 "ljit_tilepro64.dasc"
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
	//|	move L->savedpc, CI->savedpc
	dasm_put(Dst, 261, IEM_X0_Imm16, ha16(DtE(->jit_status)), IEM_X0_Imm8, JIT_S_OK, IEM_X0_Imm16, lo16(DtE(->jit_mcode)), IEM_X0_Imm16, ha16(DtE(->jit_mcode)), IEM_X0_Imm16, lo16(Dt5(->jit_gate)), IEM_X0_Imm16, ha16(Dt5(->jit_gate)), IEM_X0_Imm16, lo16(Dt1(->ci)), IEM_X0_Imm16, ha16(Dt1(->ci)), IEM_X0_Imm16, lo16(Dt1(->top)), IEM_X0_Imm16, ha16(Dt1(->top)), IEM_X0_Imm16, lo16(Dt4(->savedpc)), IEM_X0_Imm16, ha16(Dt4(->savedpc)));
# 151 "ljit_tilepro64.dasc"
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
	dasm_put(Dst, 322, IEM_X0_Imm16, lo16(Dt1(->savedpc)), IEM_X0_Imm16, ha16(Dt1(->savedpc)), IEM_X0_Imm16, lo16(Dt1(->stack)), IEM_X0_Imm16, ha16(Dt1(->stack)), IEM_J_jal, (ptrdiff_t)(luaD_precall), IEM_J_jal, (ptrdiff_t)(luaV_execute), IEM_X0_Imm16, lo16(Dt1(->stack)), IEM_X0_Imm16, ha16(Dt1(->stack)), IEM_X0_Imm16, lo16(Dt1(->top)), IEM_X0_Imm16, ha16(Dt1(->top)));
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

#ifdef LUA_COMPAT_VARARG
static void jit_vararg_table(lua_State *L)
{
  Table *tab;
  StkId base, func;
  int i, num, numparams;
  luaC_checkGC(L);
  base = L->base;
  func = L->ci->func;
  numparams = clvalue(func)->l.p->numparams;
  num = base - func - numparams - 1;
  tab = luaH_new(L, num, 1);
  for (i = 0; i < num; i++)
    setobj2n(L, luaH_setnum(L, tab, i+1), base - num + i);
  setnvalue(luaH_setstr(L, tab, luaS_newliteral(L, "n")), (lua_Number)num);
  sethvalue(L, base + numparams, tab);
}
#endif

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
	//|	movei r0, 0			// Assumes: LUA_TNIL == 0
	//|	move CI->func, BASE
	//|	addidx BASE, BASE, 1
	//|   move L->ci, CI
	dasm_put(Dst, 393, IEM_X0_Imm8, stacksize, IEM_X0_Imm16, lo16(Dt1(->stack_last)), IEM_X0_Imm16, ha16(Dt1(->stack_last)), IEM_X0_Imm16, lo16(Dt1(->end_ci)), IEM_X0_Imm16, ha16(Dt1(->end_ci)), IEM_X0_Imm16, lo16(Dt4(->func)), IEM_X0_Imm16, ha16(Dt4(->func)), IEM_X0_Imm8, (1)*sizeof(TValue), IEM_X0_Imm16, lo16(Dt1(->ci)), IEM_X0_Imm16, ha16(Dt1(->ci)));
# 223 "ljit_tilepro64.dasc"
	
	if (numparams > 0) {
	  //|  addi r1, BASE, numparams
	  dasm_put(Dst, 448, IEM_X0_Imm8, numparams);
# 226 "ljit_tilepro64.dasc"
	}
	
	if (!pt->is_vararg) {  /* Fixarg function. */
		/* Must cap L->top at L->base+numparams because 1st LOADNIL is omitted. */
		if (numparams == 0) {
		  	//|  move TOP, BASE
		  	dasm_put(Dst, 452);
# 232 "ljit_tilepro64.dasc"
		} else {
		    //|	slte_u r3, TOP, r1
			//|	bnz r3, >1
			//|	move TOP, r1
			//|1:
			dasm_put(Dst, 455);
# 237 "ljit_tilepro64.dasc"
		}
		//|	addi r1, BASE, stacksize	// New ci->top.
		//|	move CI->tailcalls, r0		// 0
		//|   move CI->top, r1
		//|   move L->top, r1
		//|	move L->base, BASE
		//|	move CI->base, BASE
		dasm_put(Dst, 464, IEM_X0_Imm8, stacksize, IEM_X0_Imm16, lo16(Dt4(->tailcalls)), IEM_X0_Imm16, ha16(Dt4(->tailcalls)), IEM_X0_Imm16, lo16(Dt4(->top)), IEM_X0_Imm16, ha16(Dt4(->top)), IEM_X0_Imm16, lo16(Dt1(->top)), IEM_X0_Imm16, ha16(Dt1(->top)), IEM_X0_Imm16, lo16(Dt1(->base)), IEM_X0_Imm16, ha16(Dt1(->base)), IEM_X0_Imm16, lo16(Dt4(->base)), IEM_X0_Imm16, ha16(Dt4(->base)));
# 244 "ljit_tilepro64.dasc"
	}
# 281 "ljit_tilepro64.dasc"
	
	/* Clear undefined args and all vars. Still assumes eax = LUA_TNIL = 0. */
	/* Note: cannot clear only args because L->top has grown. */
	if (stacksize <= EXTRA_STACK) {  /* Loopless clear. May use EXTRA_STACK. */
	  int i;
	  for (i = 0; i < stacksize; i++) {
	    //|  move TOP[i].tt, r0
	    dasm_put(Dst, 518, IEM_X0_Imm16, lo16(Dt3([i].tt)), IEM_X0_Imm16, ha16(Dt3([i].tt)));
# 288 "ljit_tilepro64.dasc"
	  }
	} else {  /* Standard loop. */
	  //|2:  // Unrolled for 2 stack slots. No initial check. May use EXTRA_STACK.
	  //|  move TOP[0].tt, r0
	  //|  move TOP[1].tt, r0
	  //|  addidx TOP, TOP, 2
	  //|  slt_u r3, TOP, r1
	  //|  bnz r3, <2
	  //|// Note: TOP is undefined now. TOP is only valid across calls/open ins.
	  dasm_put(Dst, 529, IEM_X0_Imm16, lo16(Dt3([0].tt)), IEM_X0_Imm16, ha16(Dt3([0].tt)), IEM_X0_Imm16, lo16(Dt3([1].tt)), IEM_X0_Imm16, ha16(Dt3([1].tt)), IEM_X0_Imm8, (2)*sizeof(TValue));
# 297 "ljit_tilepro64.dasc"
	}

# 305 "ljit_tilepro64.dasc"
#endif
}

/* Check if we can combine 'return const'. */
static int jit_return_k(jit_State *J)
{
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
	    dasm_put(Dst, 559, IEM_J_jal, (ptrdiff_t)(luaF_close));
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
		dasm_put(Dst, 567, IEM_X0_Imm16, lo16(Dt1(->ci)), IEM_X0_Imm16, ha16(Dt1(->ci)), IEM_X0_Imm8, (rbase)*sizeof(TValue), IEM_X0_Imm16, lo16(Dt4(->func)), IEM_X0_Imm16, ha16(Dt4(->func)), IEM_X0_Imm16, lo16(Dt4(->func)), IEM_X0_Imm16, ha16(Dt4(->func)), IEM_X0_Imm8, -sizeof(CallInfo), IEM_X0_Imm16, lo16(Dt1(->ci)), IEM_X0_Imm16, ha16(Dt1(->ci)));
# 395 "ljit_tilepro64.dasc"
		return;
	}
	
	if (!J->pt->is_vararg) {  /* Fixarg function, nresults >= 0. */
	  int i;
	  //|  addi L->ci, L->ci, -#CI
	  //|// Relocate [BASE+rbase,BASE+rbase+nresults) -> [BASE-1, *).
	  //|// TODO: loop for large nresults?
	  //|  addi BASE, BASE, -#BASE
	  dasm_put(Dst, 638, IEM_X0_Imm16, lo16(Dt1(->ci)), IEM_X0_Imm16, ha16(Dt1(->ci)), IEM_X0_Imm16, lo16(Dt1(->ci)), IEM_X0_Imm16, ha16(Dt1(->ci)), IEM_X0_Imm8, -sizeof(CallInfo), IEM_X0_Imm8, -sizeof(TValue));
# 404 "ljit_tilepro64.dasc"
	  for (i = 0; i < nresults; i++) {
	    //|  copyslot BASE[i], BASE[rbase+i+1]
	    dasm_put(Dst, 661, IEM_X0_Imm16, lo16(Dt2([rbase+i+1].value)), IEM_X0_Imm16, ha16(Dt2([rbase+i+1].value)), IEM_X0_Imm16, lo16(Dt2([i].value)), IEM_X0_Imm16, ha16(Dt2([i].value)), IEM_X0_Imm16, lo16(Dt2([rbase+i+1].value.na[1])), IEM_X0_Imm16, ha16(Dt2([rbase+i+1].value.na[1])), IEM_X0_Imm16, lo16(Dt2([i].value.na[1])), IEM_X0_Imm16, ha16(Dt2([i].value.na[1])), IEM_X0_Imm16, lo16(Dt2([rbase+i+1].tt)), IEM_X0_Imm16, ha16(Dt2([rbase+i+1].tt)), IEM_X0_Imm16, lo16(Dt2([i].tt)), IEM_X0_Imm16, ha16(Dt2([i].tt)));
	    dasm_put(Dst, 714);
# 406 "ljit_tilepro64.dasc"
	  }
	  //|  addidx TOP, BASE, nresults
	  //|  jrp lr
	  dasm_put(Dst, 717, IEM_X0_Imm8, (nresults)*sizeof(TValue));
# 409 "ljit_tilepro64.dasc"
	}
# 427 "ljit_tilepro64.dasc"
}

static void jit_op_call(jit_State *J, int func, int nargs, int nresults)
{
# 497 "ljit_tilepro64.dasc"
}

static void jit_op_tailcall(jit_State *J, int func, int nargs)
{
# 600 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_move(jit_State *J, int dest, int src)
{
# 609 "ljit_tilepro64.dasc"
}

static void jit_op_loadk(jit_State *J, int dest, int kidx)
{
# 622 "ljit_tilepro64.dasc"
}

static void jit_op_loadnil(jit_State *J, int first, int last)
{
# 647 "ljit_tilepro64.dasc"
}

static void jit_op_loadbool(jit_State *J, int dest, int b, int dojump)
{
# 664 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_getupval(jit_State *J, int dest, int uvidx)
{
# 676 "ljit_tilepro64.dasc"
}

static void jit_op_setupval(jit_State *J, int src, int uvidx)
{
# 711 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

/* Optimized table lookup routines. Enter via jsub, fallback to C. */

/* Fallback for GETTABLE_*. Temporary key is in L->env. */
static void jit_gettable_fb(lua_State *L, Table *t, StkId dest)
{
# 831 "ljit_tilepro64.dasc"
}

/* Fallback for SETTABLE_*STR. Temporary (string) key is in L->env. */
static void jit_settable_fb(lua_State *L, Table *t, StkId val)
{
# 970 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_newtable(jit_State *J, int dest, int lnarray, int lnhash)
{
# 981 "ljit_tilepro64.dasc"
}

static void jit_op_getglobal(jit_State *J, int dest, int kidx)
{
# 992 "ljit_tilepro64.dasc"
}

static void jit_op_setglobal(jit_State *J, int rval, int kidx)
{
# 1003 "ljit_tilepro64.dasc"
}

enum { TKEY_KSTR = -2, TKEY_STR = -1, TKEY_ANY = 0 };

/* Optimize key lookup depending on consts or hints type. */
static int jit_keylookup(jit_State *J, int tab, int rkey)
{
# 1069 "ljit_tilepro64.dasc"
  return TKEY_ANY;  /* Use fallback. */
}

static void jit_op_gettable(jit_State *J, int dest, int tab, int rkey)
{
# 1155 "ljit_tilepro64.dasc"
}

static void jit_op_settable(jit_State *J, int tab, int rkey, int rval)
{
# 1254 "ljit_tilepro64.dasc"
}

static void jit_op_self(jit_State *J, int dest, int tab, int rkey)
{
# 1262 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_setlist(jit_State *J, int ra, int num, int batch)
{
# 1334 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_arith(jit_State *J, int dest, int rkb, int rkc, int ev)
{
  const TValue *kkb = ISK(rkb) ? &J->pt->k[INDEXK(rkb)] : NULL;
  const TValue *kkc = ISK(rkc) ? &J->pt->k[INDEXK(rkc)] : NULL;
  const Value *kval;
  int idx, rev;
  int target = (ev == TM_LT || ev == TM_LE) ? jit_jmp_target(J) : 0;
  int hastail = 0;

  /* The bytecode compiler already folds constants except for: k/0, k%0, */
  /* NaN results, k1<k2, k1<=k2. No point in optimizing these cases. */
  if (ISK(rkb&rkc)) goto fallback;

  /* Avoid optimization when non-numeric constants are present. */
  if (kkb ? !ttisnumber(kkb) : (kkc && !ttisnumber(kkc))) goto fallback;

  /* The TYPE hint selects numeric inlining and/or fallback encoding. */
  switch (ttype(hint_get(J, TYPE))) {
  case LUA_TNIL: hastail = 1; break;  /* No hint: numeric + fallback. */
  case LUA_TNUMBER: break;	      /* Numbers: numeric + deoptimization. */
  default: goto fallback;	      /* Mixed/other types: fallback only. */
  }

  /* The checks above ensure: at most one of the operands is a constant. */
  /* Reverse operation and swap operands so the 2nd operand is a variable. */
  if (kkc) { kval = &kkc->value; idx = rkb; rev = 1; }
  else { kval = kkb ? &kkb->value : NULL; idx = rkc; rev = 0; }

# 1451 "ljit_tilepro64.dasc"

  /* Check number type and load 1st operand. */
  if (kval) {
    //| // isnumber idx; jne L_DEOPTIMIZEF
    //|  loadnumberi r0, r1, kval
    dasm_put(Dst, 723, IEM_X0_Imm16, lo16(kval), IEM_X0_Imm16, ha16(kval));
# 1456 "ljit_tilepro64.dasc"
  } else {
# 1465 "ljit_tilepro64.dasc"
    //|  loadnumber r0, r1, BASE[rkb].value
    dasm_put(Dst, 736, IEM_X0_Imm16, lo16(Dt2([rkb].value)), IEM_X0_Imm16, ha16(Dt2([rkb].value)), IEM_X0_Imm16, lo16(Dt2([rkb].value)), IEM_X0_Imm16, ha16(Dt2([rkb].value)));
# 1466 "ljit_tilepro64.dasc"
  }

  /* Load 2nd operand. */
  //| loadnumber r2, r3, BASE[idx].value
  dasm_put(Dst, 759, IEM_X0_Imm16, lo16(Dt2([idx].value)), IEM_X0_Imm16, ha16(Dt2([idx].value)), IEM_X0_Imm16, lo16(Dt2([idx].value)), IEM_X0_Imm16, ha16(Dt2([idx].value)));
# 1470 "ljit_tilepro64.dasc"

  /* Encode arithmetic operation. */
  switch ((ev<<1)+rev) {
  case TM_ADD<<1:
  case (TM_ADD<<1)+1:
    //|  jal &__float64_add
    dasm_put(Dst, 782, IEM_J_jal, (ptrdiff_t)(__float64_add));
# 1476 "ljit_tilepro64.dasc"
	break;
# 1517 "ljit_tilepro64.dasc"
  default:  /* TM_LT or TM_LE. */
# 1539 "ljit_tilepro64.dasc"
  }
fpstore:
  /* Store result and set result type (if necessary). */
  //|  storenumber BASE[dest].value, r0, r1
  dasm_put(Dst, 786, IEM_X0_Imm16, lo16(Dt2([dest].value)), IEM_X0_Imm16, ha16(Dt2([dest].value)), IEM_X0_Imm16, lo16(Dt2([dest].value)), IEM_X0_Imm16, ha16(Dt2([dest].value)));
# 1543 "ljit_tilepro64.dasc"
  if (dest != rkb && dest != rkc) {
    //|  settti BASE[dest], LUA_TNUMBER
    dasm_put(Dst, 809, IEM_X0_Imm16, lo16(Dt2([dest].tt)), IEM_X0_Imm16, ha16(Dt2([dest].tt)));
# 1545 "ljit_tilepro64.dasc"
  }
# 1591 "ljit_tilepro64.dasc"
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
# 1657 "ljit_tilepro64.dasc"
}

static void jit_op_not(jit_State *J, int dest, int rb)
{
# 1677 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_concat(jit_State *J, int dest, int first, int last)
{
# 1759 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_eq(jit_State *J, int cond, int rkb, int rkc)
{
# 1852 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_test(jit_State *J, int cond, int dest, int src)
{
# 1892 "ljit_tilepro64.dasc"
}

static void jit_op_jmp(jit_State *J, int target)
{
# 1899 "ljit_tilepro64.dasc"
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
# 1972 "ljit_tilepro64.dasc"
}

static void jit_op_forloop(jit_State *J, int ra, int target)
{
# 2008 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_tforloop(jit_State *J, int ra, int nresults)
{
# 2027 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_close(jit_State *J, int ra)
{
# 2042 "ljit_tilepro64.dasc"
}

static void jit_op_closure(jit_State *J, int dest, int ptidx)
{
# 2092 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */

static void jit_op_vararg(jit_State *J, int dest, int num)
{
# 2165 "ljit_tilepro64.dasc"
}

/* ------------------------------------------------------------------------ */


