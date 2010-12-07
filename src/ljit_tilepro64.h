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
//|.type L,		lua_State,	r49
#define Dt1(_V) (int)&(((lua_State *)0)_V)
# 6 "ljit_tilepro64.dash"
//|.type BASE,		TValue,		r50
#define Dt2(_V) (int)&(((TValue *)0)_V)
# 7 "ljit_tilepro64.dash"
//|.type TOP,		TValue,		r51
#define Dt3(_V) (int)&(((TValue *)0)_V)
# 8 "ljit_tilepro64.dash"
//|.type CI,		CallInfo,	r52
#define Dt4(_V) (int)&(((CallInfo *)0)_V)
# 9 "ljit_tilepro64.dash"
//|.type LCL,		LClosure,	r53
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
//|.define LUA_TNIL,			0
//|.define LUA_TBOOLEAN,			1
//|.define LUA_TLIGHTUSERDATA,		2
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

# 6 "ljit_tilepro64.dasc"
//|.actionlist alist
static const signed long alist[7] = {
  880898,1074497536,24457280,1074497536,1044489,1074497536,2147483647
};

# 7 "ljit_tilepro64.dasc"
//|.globals JSUB_
enum {
  JSUB__MAX
};
# 8 "ljit_tilepro64.dasc"

void lol()
{
	//| add r2, r4, r23
	//| sub r0, r1, r19
	//| add r9, r0, zero
	dasm_put(Dst, 0);
# 14 "ljit_tilepro64.dasc"
}

