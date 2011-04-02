------------------------------------------------------------------------------
-- DynASM x86 module.
--
-- Copyright (C) 2005-2008 Mike Pall. All rights reserved.
-- See dynasm.lua for full copyright notice.
------------------------------------------------------------------------------

-- Module information:
local _info = {
  arch =	"tilepro64",
  description =	"DynASM TILEPro64 module",
  version =	"0.0.2",
  vernum =	 1,
  release =	"2010-11-19",
  author =	"Ruben Tytgat",
  license =	"MIT",
}

require("bit")
-- Exported glue functions for the arch-specific module.
local _M = { _info = _info }

-- Cache library functions.
local type, tonumber, pairs, ipairs = type, tonumber, pairs, ipairs
local assert, unpack = assert, unpack
local _s = string
local sub, format, byte, char = _s.sub, _s.format, _s.byte, _s.char
local find, match, gmatch, gsub = _s.find, _s.match, _s.gmatch, _s.gsub
local concat, sort = table.concat, table.sort
local char, unpack = string.char, unpack

-- Inherited tables and callbacks.
local g_opt, g_arch
local wline, werror, wfatal, wwarn

-- Action name list.
-- CHECK: Keep this in sync with the C code!
local action_names = {
  -- int arg, 1 buffer pos:
  "IMM",
  -- action arg (1 byte) or int arg, 1 buffer pos (link):
  "L","G", "PC",
  -- action arg (1 byte) or int arg, 1 buffer pos (offset):
  "LABEL_L","LABEL_G", "LABEL_PC",
  -- action arg (1 byte), 1 buffer pos (offset):
  "ALIGN",
  -- no action arg, no buffer pos.
  "SECTION",
  -- action arg (1 byte), no buffer pos.
  "ESC",
  -- no args, no buffer pos, terminal action:
  "STOP"
}

-- Maximum number of section buffer positions for dasm_put().
-- CHECK: Keep this in sync with the C code!
local maxsecpos = 25 -- Keep this low, to avoid excessively long C lines.

-- Action name -> action number (dynamically generated below).
local map_action = {}
-- First action number. Everything below does not need to be escaped.
local actfirst = 2147483648-#action_names

-- Action list buffer and string (only used to remove dupes).
local actlist = {}
local actstr = ""

-- Argument list for next dasm_put(). Start with offset 0 into action list.
local actargs = { 0 }

-- Current number of section buffer positions for dasm_put().
local secpos = 1

------------------------------------------------------------------------------
local ctypenum = 0
local map_type = {}
local map_op = {}
------------------------------------------------------------------------------
-- Compute action numbers for action names.
for n,name in ipairs(action_names) do
  local num = actfirst + n - 1
  map_action[name] = num
end

-- Dump action names and numbers.
local function dumpactions(out)
  out:write("DynASM encoding engine action codes:\n")
  for n,name in ipairs(action_names) do
    local num = map_action[name]
    out:write(format("  %-10s %02X  %d\n", name, num, num))
  end
  out:write("\n")
end

-- Write action list buffer as a huge static C array.
local function writeactions(out, name)
  local nn = #actlist
  local last = actlist[nn] or 255
  actlist[nn] = nil -- Remove last byte.
  if nn == 0 then nn = 1 end
  out:write("static const signed long ", name, "[", nn, "] = {\n")
  local s = "  "
  for n,b in ipairs(actlist) do
    s = s..b..","
    if #s >= 75 then
      assert(out:write(s, "\n"))
      s = "  "
    end
  end
  out:write(s, last, "\n};\n\n") -- Add last byte back.
end

------------------------------------------------------------------------------

-- Add byte to action list.
local function wputxw(n)
  assert(n >= -2147483648 and n <= 2147483647 and n % 1 == 0, "word out of range")
  actlist[#actlist+1] = n
end

-- Add action to list with optional arg. Advance buffer pos, too.
local function waction(action, a, num)
 -- wputxw(assert(map_action[action], "bad action name `"..action.."'"))
 -- TODO: debug, remove
  actlist[#actlist+1] = "DASM_" .. action
  if a then actargs[#actargs+1] = a end
  if a or num then secpos = secpos + (num or 1) end
end

local function wiem(iem)
	actlist[#actlist+1] = iem
end

-- Write action param
local function waparam(a)
	actargs[#actargs+1] = a
	secpos = secpos + 1
end

-- Add call to embedded DynASM C code.
local function wcall(func, args)
  wline(format("dasm_%s(Dst, %s);", func, concat(args, ", ")), true)
end

-- Delete duplicate action list chunks. A tad slow, but so what.
local function dedupechunk(offset)
  local al, as = actlist, actstr
  local chunk = char(unpack(al, offset+1, #al))
  local orig = find(as, chunk, 1, true)
  if orig then
    actargs[1] = orig-1 -- Replace with original offset.
    for i=offset+1,#al do al[i] = nil end -- Kill dupe.
  else
    actstr = as..chunk
  end
end

-- Flush action list (intervening C code or buffer pos overflow).
local function wflush(term)
  local offset = actargs[1]
  if #actlist == offset then return end -- Nothing to flush.
  if not term then waction("STOP") end -- Terminate action list.
  -- dedupechunk(offset)
  wcall("put", actargs) -- Add call to dasm_put().
  actargs = { #actlist } -- Actionlist offset is 1st arg to next dasm_put().
  secpos = 1 -- The actionlist offset occupies a buffer position, too.
end

-- Put escaped byte.
local function wputw(n)
  if n >= actfirst then waction("ESC") end -- Need to escape byte.
  wputxw(n)
end

------------------------------------------------------------------------------

-- Global label name -> global label number. With auto assignment on 1st use.
local next_global = 0
local map_global = setmetatable({}, { __index = function(t, name)
  if not match(name, "^[%a_][%w_]*$") then werror("bad global label") end
  local n = next_global
  if n > 246 then werror("too many global labels") end
  next_global = n + 1
  t[name] = n
  return n
end})

-- Dump global labels.
local function dumpglobals(out, lvl)
  local t = {}
  for name, n in pairs(map_global) do t[n] = name end
  out:write("Global labels:\n")
  for i=10,next_global-1 do
    out:write(format("  %s\n", t[i]))
  end
  out:write("\n")
end

-- Write global label enum.
local function writeglobals(out, prefix)
  local t = {}
  for name, n in pairs(map_global) do t[n] = name end
  out:write("enum {\n")
  for i=0,next_global-1 do
    out:write("  ", prefix, t[i], ",\n")
  end
  out:write("  ", prefix, "_MAX\n};\n")
end

------------------------------------------------------------------------------
local map_archdef = {}
local map_op = {}
-- Reverse defines for registers.
function _M.revdef(s)
  return s
end


------------------------------------------------------------------------------

-- Put action for label arg (IMM_LG, IMM_PC, REL_LG, REL_PC).
local function wputlabel(aprefix, imm, num)
  if type(imm) == "number" then
    waction(aprefix.."LG", nil, num);
    wputxw(imm)
  else
    waction(aprefix.."PC", imm, num)
  end
end

local function wputimm(immtype, expr)
	waction("IMM")
	wiem(immtype)
	waparam(expr)
end

local function wputg(immtype, expr)
	waction("G")
	wiem(immtype)
	wputxw(expr)
end

local function wputl(immtype, expr)
	waction("L")
	wiem(immtype)
	wputxw(expr)
end


------------------------------------------------------------------------------

local function table_append(...)
	local t = {}
	local args = { ... }
	for j,a in ipairs(args) do
		for i,v in ipairs(a) do
			t[#t+1] = v
		end
	end
	return t
end

------------------------------------------------------------------------------
local patterns = {
		reg = {
			gpr = "^r%d+$",
			sprs = {
				sp = 54,
				lr = 55,
				zero = 63
			}
		},
}
local cur_scratch_reg = {}
local function begin_instruction()
	cur_scratch_reg[#cur_scratch_reg+1] = 26
end

local function end_instruction()
	cur_scratch_reg[#cur_scratch_reg] = nil
end

local function get_scratch_reg()
	cur_scratch_reg[#cur_scratch_reg] = cur_scratch_reg[#cur_scratch_reg] + 1
	return cur_scratch_reg[#cur_scratch_reg]
end

local function make_operand(val, posts)
	return { val = val,
			 posts = posts }
end

local function is_reg(str)
	return string.match(str,patterns.reg.gpr) or patterns.reg.sprs[str]
end

local function parsereg(str)
	local t
	if string.match(str,patterns.reg.gpr) then
		local regnum = str.sub(str,2) + 0
		if regnum > 52 then
			error("register does not exist: " .. str)
		end
	
		return make_operand(regnum, {})
	end

	t = patterns.reg.sprs[str]
	if t then
		return make_operand(t, {})
	end

	werror("operand must be register: " .. str)
end

local function parsetype(str, isdst)
	local expr, accessor = string.match(str, "^([%w_:]+)%s*(.*)$")
	if not expr then werror("Not a type: " .. str) end
	local typename, reg_override = string.match(expr, "^([%w_]+):([%w_]+)$")
	local post = nil
	local reg

	if not typename then
		typename = expr
	end

	local t = map_type[typename]
	if not t then
		werror("not a valid type: " .. typename)
	end

	if reg_override then
		reg = reg_override
	elseif t.reg then
		reg = t.reg
	else
		werror("type needs register override")
	end

	-- is of type    move BASE, r3 ?
	if accessor == "" then
		return parsereg(reg)
	end

	-- is of type    move BASE->value, r3 ?
	local immexpr = format(t.ctypefmt, accessor)
	local screg = get_scratch_reg()
	if not isdst then
		--[[
		We need to add the offset that we get to the address already in the reg
		We do this with the following ASM code:
		]]
		map_op.addli_3({ "r26" , reg , "lo16(" .. immexpr .. ")" })
		map_op.auli_3({ "r26" , "r26" , "ha16(" .. immexpr .. ")" })
		map_op.lw_2({ "r" .. screg , "r26" })
		return make_operand(screg, {})
	else
		post = function()
			map_op.addli_3({ "r26" , reg , "lo16(" .. immexpr .. ")" })
			map_op.auli_3({ "r26" , "r26" , "ha16(" .. immexpr .. ")" })
			map_op.sw_2({ "r26" , "r" .. screg })
		end
		return make_operand(screg, {post})
	end
end


local function parseregortype(str, isdst)
	if is_reg(str) then
		return parsereg(str)
	else
		return parsetype(str,isdst)
	end
end

--[[
local function parseglobal(expr)
	return make_operand(map_global[expr], {})
end

local function parselocal(dir,num)
	return make_operand(num + (dir == ">" and 246 or 0))
end

local function parsepc(expr)
	return make_operand(
]]

----------------------------------------------------

local imm_enc_modes = {
	X0_Imm8 = "IEM_X0_Imm8",
	X0_Imm16 = "IEM_X0_Imm16",
	X1_Br = "IEM_X1_Br",
	X1_Shift = "IEM_X1_Shift",
	X1_J = "IEM_X1_J",
	X1_J_jal = "IEM_X1_J_jal"
}

-- Parse immediate expression.
local function parseimm(expr, immtype)


	local prefix = sub(expr, 1, 2)
--[[	-- =>expr (pc label reference)
	if prefix == "=>" then
		return "iJ", sub(expr, 3)
	end
--]]

	-- ->name (global label reference)
	if prefix == "->" then
		return make_operand(0, { function()
											wputg(immtype, map_global[sub(expr, 3)])
									end })
	end
	
	-- [<>][1-9] (local label reference)
	local dir, lnum = match(expr, "^([<>])([1-9])$")
	if dir then 
		return make_operand(0, { function()
											wputl(immtype, lnum + (dir == ">" and 10 or 0))
										 end})
	end

	-- constant immediate value
	local n = tonumber(expr)
	if n then
		return make_operand(n, {})
	end

	-- halfconstant immediate value
	return make_operand(0, { function()
				wputimm(immtype, expr)
			end })
end

local function make_instr(hi, lo, posts)
	return { hi = hi,
			 lo = lo,
			 posts = posts }
end

-----------------------------------------------------------------
-- Instruction combine
function instr_combine(a, b, c)
	local combined = make_instr(bit.bor(a.hi,b.hi), bit.bor(a.lo,b.lo), table_append(a.posts, b.posts))

	if c then
		combined = make_instr(bit.bor(combined.hi,c.hi), bit.bor(combined.lo,c.lo), table_append(combined.posts, c.posts))
	end

	return combined
end

-----------------------------------------------------------------
-- Instruction builders
local instr_builders = {}
function instr_builders.X0_RRR(opcode, s, opcodeextension, Dst, A, B)
	local instr = Dst.val
	instr = bit.bor(instr, bit.lshift(A.val, 6))
	instr = bit.bor(instr, bit.lshift(B.val, 12))
	instr = bit.bor(instr, bit.lshift(opcodeextension, 18))
	instr = bit.bor(instr, bit.lshift(s, 27))
	instr = bit.bor(instr, bit.lshift(opcode, 28))
	return make_instr(0, instr, table_append(Dst.posts, A.posts, B.posts))
end

function instr_builders.X0_Imm8(opcode, s, immopcodeex, Dst, A, Imm8)
	local instr = Dst.val
	instr = bit.bor(instr, bit.lshift(A.val, 6))
	instr = bit.bor(instr, bit.lshift(bit.band(Imm8.val,0xFF), 12))
	instr = bit.bor(instr, bit.lshift(immopcodeex, 20))
	instr = bit.bor(instr, bit.lshift(s, 27))
	instr = bit.bor(instr, bit.lshift(opcode,28))
	return make_instr(0, instr, table_append(Dst.posts, A.posts, Imm8.posts))
end

function instr_builders.X0_Imm16(opcode, Dst, A, Imm16)
	local instr = Dst.val
	instr = bit.bor(instr, bit.lshift(A.val, 6))
	instr = bit.bor(instr, bit.lshift(bit.band(Imm16.val,0xFFFF), 12))
	instr = bit.bor(instr, bit.lshift(opcode,28))
	return make_instr(0, instr, table_append(Dst.posts, A.posts, Imm16.posts))
end

---
function instr_builders.X1_RRR(opcode, s, opcodeex, Dst, A, B)
	local lo = bit.lshift(Dst.val, 31)
	local hi = bit.rshift(Dst.val, 1)
	hi = bit.bor(hi, bit.lshift(A.val, 5))
	hi = bit.bor(hi, bit.lshift(B.val, 11))
	hi = bit.bor(hi, bit.lshift(opcodeex, 17))
	hi = bit.bor(hi, bit.lshift(s, 26))
	hi = bit.bor(hi, bit.lshift(opcode, 27))
	return make_instr(hi, lo, table_append(Dst.posts, A.posts, B.posts))
end


function instr_builders.X1_Unary(opcode, s, shopcodeex, opcodeex, Dst, A)
	local lo = bit.lshift(Dst.val, 31)
	local hi = bit.rshift(Dst.val, 1)
	hi = bit.bor(hi, bit.lshift(A.val, 5))
	hi = bit.bor(hi, bit.lshift(opcodeex, 11))
	hi = bit.bor(hi, bit.lshift(shopcodeex, 16))
	hi = bit.bor(hi, bit.lshift(s, 26))
	hi = bit.bor(hi, bit.lshift(opcode, 27))
	return make_instr(hi, lo, table_append(Dst.posts, A.posts))
end

function instr_builders.X1_Br(opcode, s, brtype, A, Off)
	local lo = bit.lshift(brtype, 31)
	local hi = bit.rshift(brtype, 1)
	local off1400 = bit.band(Off.val, 0x7FFF)
	local off1615 = bit.band(bit.rshift(Off.val, 15), 3)
	hi = bit.bor(hi, bit.lshift(A.val, 5))
	hi = bit.bor(hi, bit.lshift(off1615, 3))
	hi = bit.bor(hi, bit.lshift(off1400, 11))
	hi = bit.bor(hi, bit.lshift(s, 26))
	hi = bit.bor(hi, bit.lshift(opcode, 27))
	return make_instr(hi, lo, table_append(A.posts, Off.posts))
end

function instr_builders.X1_Shift(opcode, s, shopcodeextension, Dst, A, ShAmt)
	local lo = bit.lshift(Dst.val, 31)
	local hi = bit.rshift(Dst.val, 1)
	hi = bit.bor(hi, bit.lshift(A.val, 5))
	hi = bit.bor(hi, bit.lshift(bit.band(ShAmt.val,0x1F), 11))
	hi = bit.bor(hi, bit.lshift(shopcodeextension, 16))
	hi = bit.bor(hi, bit.lshift(s, 26))
	hi = bit.bor(hi, bit.lshift(opcode, 27))
	return make_instr(hi, lo, table_append(Dst.posts, A.posts, ShAmt.posts))
end

function instr_builders.X1_J(opcode, off)
	local off1400 = bit.band(off.val, 0x7FFF)
	local off1615 = bit.band(bit.rshift(off.val, 15), 0x3)
	local off2017 = bit.band(bit.rshift(off.val, 17), 0xF)
	local off2621 = bit.band(bit.rshift(off.val, 21), 0x3F)
	local off2727 = bit.band(bit.rshift(off.val, 27), 0x1)
	local lo = bit.lshift(off2017, 31)
	local hi = bit.rshift(off2017, 1)
	hi = bit.bor(hi, bit.lshift(off1615, 3))
	hi = bit.bor(hi, bit.lshift(off2621, 5))
	hi = bit.bor(hi, bit.lshift(off1400, 11))
	hi = bit.bor(hi, bit.lshift(off2727, 26))
	hi = bit.bor(hi, bit.lshift(opcode, 27))
	return make_instr(hi, lo, table_append(off.posts))
end

------------------------------------------------------------------
-- Instruction parsers
local instr_parsers = {}
function instr_parsers.X0_RRR(opcode, s, opcodeextension)
	return function(params)
		local Dst, A, B = params[1], params[2], params[3]
		return instr_builders.X0_RRR(opcode, s, opcodeextension, parseregortype(Dst,true), parseregortype(A), parseregortype(B))
	end
end

function instr_parsers.X0_Imm8(opcode, s, immopcodeex)
	return function(params)
		local Dst, A, Imm8 = params[1], params[2], params[3]
		return instr_builders.X0_Imm8(opcode, s, immopcodeex, parseregortype(Dst,true), parseregortype(A), parseimm(Imm8, imm_enc_modes.X0_Imm8))
	end
end

function instr_parsers.X0_Imm16(opcode)
	return function(params)
		local Dst, A, Imm16 = params[1], params[2], params[3]
		return instr_builders.X0_Imm16(opcode, parseregortype(Dst,true), parseregortype(A), parseimm(Imm16, imm_enc_modes.X0_Imm16))
	end
end

---

function instr_parsers.X1_Unary(opcode, s, shopcodeex, opcodeex)
	return function(params)
		local Dst, A = params[1], params[2]
		return instr_builders.X1_Unary(opcode, s, shopcodeex, opcodeex, parseregortype(Dst,true), parseregortype(A))
	end
end

function instr_parsers.X1_Unary_no_Dst_and_A(opcode, s, shopcodeex, opcodeex)
	return function(params)
		return instr_builders.X1_Unary(opcode, s, shopcodeex, opcodeex, make_operand(0, {}), make_operand(0, {}))
	end
end

function instr_parsers.X1_Br(opcode,s,brtype)
	return function(params)
		local A, Off = params[1], params[2]
		return instr_builders.X1_Br(opcode, s, brtype, parseregortype(A), parseimm(Off, imm_enc_modes.X1_Br))
	end
end

function instr_parsers.X1_Shift(opcode,s,shopcodeextension)
	return function(params)
		local Dst, A, ShAmt = params[1], params[2], params[3]
		return instr_builders.X1_Shift(opcode, s, shopcodeextension, parseregortype(Dst,true), parseregortype(A), parseimm(ShAmt, imm_enc_modes.X1_Shift))
	end
end

function instr_parsers.X1_J(opcode)
	return function(params)
		local Off = params[1]
		return instr_builders.X1_J(opcode, parseimm(Off, imm_enc_modes.X1_J))
	end
end

------------------------------------------------------
local special_instr_parsers = {}
function special_instr_parsers.X1_RRR_no_Dst(opcode, s, opcodeextension)
	return function(params)
		local A, B = params[1], params[2]
		return instr_builders.X1_RRR(opcode, s, opcodeextension, make_operand(0, {}), parseregortype(A), parseregortype(B))
	end
end

function special_instr_parsers.X1_RRR_no_Dst_and_B(opcode, s, opcodeextension)
	return function(params)
		local A, B = params[1], params[2]
		return instr_builders.X1_RRR(opcode, s, opcodeextension, make_operand(0, {}), parseregortype(A), make_operand(0, {}))
	end
end

function special_instr_parsers.X1_J_jal()
	return function(params)
		local Off = params[1]
		return instr_builders.X1_J(0, parseimm(Off, imm_enc_modes.X1_J_jal))
	end
end



------------------------------------------------------------------
-- Temporary workaround.
local function wrap_put_nop_X0(parser)
	return function(params)
		begin_instruction()

	--	print("INSTR: "..(params[1] or "") .. "," .. (params[2] or "") .. "," .. (params[3] or ""))
		if secpos+2 > maxsecpos then wflush() end

		local i = instr_combine(make_instr(0,0x70165000,{}), parser(params))
		wputw(i.lo)
		wputw(i.hi)
		for i,v in ipairs(i.posts) do
			v()
		end

		end_instruction()
	end
end

local function wrap_put_nop_X1(parser)
	return function(params)
		begin_instruction()

	--	print("INSTR: "..(params[1] or "") .. "," .. (params[2] or "") .. "," .. (params[3] or ""))
		if secpos+2 > maxsecpos then wflush() end

		local i = instr_combine(make_instr(0x400B2800,0,{}), parser(params))
		wputw(i.lo)
		wputw(i.hi)
		for i,v in ipairs(i.posts) do
			v()
		end

		end_instruction()
	end
end

----------------------------------------------------------------
-- ops

map_instr_X0 = {
	-- Arithmetic opcodes
	add_3 = instr_parsers.X0_RRR(0, 0, 3),
	addi_3 = instr_parsers.X0_Imm8(4, 0, 3),
	addli_3 = instr_parsers.X0_Imm16(2),
	adds_3 = instr_parsers.X0_RRR(0, 0, 0x60),
	auli_3 = instr_parsers.X0_Imm16(3),
	sub_3 = instr_parsers.X0_RRR(0, 0, 0x5D),

	-- Compare opcodes
	seq_3 = instr_parsers.X0_RRR(0, 0, 0x42),
	seqi_3 = instr_parsers.X0_Imm8(4, 0, 0x0B),
	slt_3 = instr_parsers.X0_RRR(0, 0, 0x53),
	slt_u_3 = instr_parsers.X0_RRR(0, 0, 0x54),
	slte_u_3 = instr_parsers.X0_RRR(0, 0, 0x50),

	-- Logical opcodes
	or_3 = instr_parsers.X0_RRR(0, 0, 0x33),
	ori_3 = instr_parsers.X0_Imm8(4, 0, 8),

	-- Multiply opcodes
	mulhhsa_uu_3 = instr_parsers.X0_RRR(0, 0, 0x19),
	mulhl_uu_3 = instr_parsers.X0_RRR(0, 0, 0x25),
	mulhla_uu_3 = instr_parsers.X0_RRR(0, 0, 0x20),
	mullla_uu_3 = instr_parsers.X0_RRR(0, 0, 0x28)
}

map_instr_X1 = {
	-- Compare opcodes
	-- seq_3 = instr_parsers.X1_RRR(1, 0, 0x23),
	-- slt_3 = instr_parsers.X1_RRR(1, 0, 0x35),
	-- slt_u_3 = instr_parsers.X1_RRR(1, 0, 0x36),
	-- slte_u_3 = instr_parsers.X1_RRR(1, 0, 0x32),

	-- Control opcodes
	bnz_2 = instr_parsers.X1_Br(5, 0, 0x3),
	bnzt_2 = instr_parsers.X1_Br(5, 0, 0x3),
	bz_2 = instr_parsers.X1_Br(5, 0, 0x1),
	bzt_2 = instr_parsers.X1_Br(5, 0, 0x1),
	jal_1 = special_instr_parsers.X1_J_jal(),
	jalr_1 = special_instr_parsers.X1_RRR_no_Dst_and_B(1, 0, 0x09),
	jr_1 = special_instr_parsers.X1_RRR_no_Dst_and_B(1, 0, 0x0C),
	jrp_1 = special_instr_parsers.X1_RRR_no_Dst_and_B(1, 0, 0x0B),

	-- Logical opcodes
	shli_3 = instr_parsers.X1_Shift(8, 0, 0x4),

	-- Memory opcodes
	lw_2 = instr_parsers.X1_Unary(8, 0, 0xB, 0xE),
	sw_2 = special_instr_parsers.X1_RRR_no_Dst(1, 0, 0x40),

	-- NOP opcodes
	nop_0 = instr_parsers.X1_Unary_no_Dst_and_A(8, 0, 0xB, 0x11)
}

map_op = {
	-- Arithmetic opcodes
	add_3 = wrap_put_nop_X1(map_instr_X0["add_3"]),
	addi_3 = wrap_put_nop_X1(map_instr_X0["addi_3"]),
	addli_3 = wrap_put_nop_X1(map_instr_X0["addli_3"]),
	adds_3 = wrap_put_nop_X1(map_instr_X0["adds_3"]),
	auli_3 = wrap_put_nop_X1(map_instr_X0["auli_3"]),
	sub_3 = wrap_put_nop_X1(map_instr_X0["sub_3"]),

	-- Compare opcodes
	seq_3 = wrap_put_nop_X1(map_instr_X0["seq_3"]),
	seqi_3 = wrap_put_nop_X1(map_instr_X0["seqi_3"]),
	slt_3 = wrap_put_nop_X1(map_instr_X0["slt_3"]),
	slt_u_3 = wrap_put_nop_X1(map_instr_X0["slt_u_3"]),
	slte_u_3 = wrap_put_nop_X1(map_instr_X0["slte_u_3"]),
	
	-- Control opcodes
	bnz_2 = wrap_put_nop_X0(map_instr_X1["bnz_2"]),
	bnzt_2 = wrap_put_nop_X0(map_instr_X1["bnzt_2"]),
	bz_2 = wrap_put_nop_X0(map_instr_X1["bz_2"]),
	bzt_2 = wrap_put_nop_X0(map_instr_X1["bzt_2"]),
	jal_1 = wrap_put_nop_X0(map_instr_X1["jal_1"]),
	jalr_1 = wrap_put_nop_X0(map_instr_X1["jalr_1"]),
	jr_1 = wrap_put_nop_X0(map_instr_X1["jr_1"]),
	jrp_1 = wrap_put_nop_X0(map_instr_X1["jrp_1"]),

	-- Logical opcodes
	or_3 = wrap_put_nop_X1(map_instr_X0["or_3"]),
	ori_3 = wrap_put_nop_X1(map_instr_X0["ori_3"]),
	shli_3 = wrap_put_nop_X0(map_instr_X1["shli_3"]),

	-- Memory opcodes
	lw_2 = wrap_put_nop_X0(map_instr_X1["lw_2"]),
	sw_2 = wrap_put_nop_X0(map_instr_X1["sw_2"]),

	-- Multiply opcodes
	mulhhsa_uu_3 = wrap_put_nop_X1(map_instr_X0["mulhhsa_uu_3"]),
	mulhl_uu_3 = wrap_put_nop_X1(map_instr_X0["mulhl_uu_3"]),
	mulhla_uu_3 = wrap_put_nop_X1(map_instr_X0["mulhla_uu_3"]),
	mullla_uu_3 = wrap_put_nop_X1(map_instr_X0["mullla_uu_3"]),
	
	-- NOP opcodes
	nop_0 = wrap_put_nop_X0(map_instr_X1["nop_0"])
}


-- Pseudo-opcode to mark the position where the action list is to be emitted.
map_op[".actionlist_1"] = function(params)
  if not params then return "cvar" end
  local name = params[1] -- No syntax check. You get to keep the pieces.
  wline(function(out) writeactions(out, name) end)
end

-- Pseudo-opcode to mark the position where the global enum is to be emitted.
map_op[".globals_1"] = function(params)
  if not params then return "prefix" end
  local prefix = params[1] -- No syntax check. You get to keep the pieces.
  wline(function(out) writeglobals(out, prefix) end)
end


------------------------------------------------------------------------------

local function parse_label_def(expr)
	local prefix = sub(expr, 1, 2)
--[[	-- =>expr (pc label reference)
	if prefix == "=>" then
		return "iJ", sub(expr, 3)
	end
--]]

	-- ->name (global label reference)
	if prefix == "->" then
		waction("LABEL_G")
		wputxw(map_global[sub(expr, 3)])
		return
	end
	
	-- [<>][1-9] (local label reference)
	local lnum = match(expr, "^([1-9])$")
	if lnum then
		waction("LABEL_L")
		wputxw(lnum + 0) -- TODO: more elegant conversion
		return
	end

	werror("bad label definition")
end

---[[ Label pseudo-opcode (converted from trailing colon form).
map_op[".label_2"] = function(params)
	if not params then return "[1-9] | ->global | =>pcexpr  [, addr]" end
	parse_label_def(params[1])

	--[[ SETLABEL must immediately follow LABEL_LG/LABEL_PC.
  local addr = params[2]
  if addr then
    local a = parseoperand(params[2])
    if a.mode == "iPJ" then
      waction("SETLABEL", a.imm) -- !x64 (secpos)
    else
      werror("bad label assignment")
    end
  end
  --]]
end
map_op[".label_1"] = map_op[".label_2"]
--]]
------------------------------------------------------------------------------
---[==[
-- Alignment pseudo-opcode.
map_op[".align_1"] = function(params)
	if not params then return "numpow2" end
	local align = tonumber(params[1])
	if align then
		local l = align / 8
		if math.floor(l) == l then
			waction("ALIGN")
			wputxw(l - 1) --> bitmask to check if aligned.
			return
		end
	end
	werror("bad alignment")
end
--]==]
-- Spacing pseudo-opcode.
--[[
map_op[".space_2"] = function(params)
  if not params then return "num [, filler]" end
  waction("SPACE", params[1])
  local fill = params[2]
  if fill then
    fill = tonumber(fill)
    if not fill or fill < 0 or fill > 255 then werror("bad filler") end
  end
  wputxw(fill or 0)
end
map_op[".space_1"] = map_op[".space_2"]
--]]

------------------------------------------------------------------------------
---[[
-- Pseudo-opcode for (primitive) type definitions (map to C types).
map_op[".type_3"] = function(params, nparams)
  if not params then
    return nparams == 2 and "name, ctype" or "name, ctype, reg"
  end
  local name, ctype, reg = params[1], params[2], params[3]
  if not match(name, "^[%a_][%w_]*$") then
    werror("bad type name `"..name.."'")
  end
  local tp = map_type[name]
  if tp then
    werror("duplicate type `"..name.."'")
  end
  if reg and not is_reg(reg) then
    werror("bad base register `"..reg.."'")
  end
  -- Add #type to defines. A bit unclean to put it in map_archdef.
  map_archdef["#"..name] = "sizeof("..ctype..")"
  -- Add new type and emit shortcut define.
  local num = ctypenum + 1
  map_type[name] = {
    ctype = ctype,
    ctypefmt = format("Dt%X(%%s)", num),
    reg = reg,
  }
  wline(format("#define Dt%X(_V) (int)&(((%s *)0)_V)", num, ctype))
  ctypenum = num
end
map_op[".type_2"] = map_op[".type_3"]

--[[
-- Dump type definitions.
local function dumptypes(out, lvl)
  local t = {}
  for name in pairs(map_type) do t[#t+1] = name end
  sort(t)
  out:write("Type definitions:\n")
  for _,name in ipairs(t) do
    local tp = map_type[name]
    local reg = tp.reg and map_reg_rev[tp.reg] or ""
    out:write(format("  %-20s %-20s %s\n", name, tp.ctype, reg))
  end
  out:write("\n")
end
--]]

map_op[".actionnames_1"] = function(params)
	wline("enum	" .. params[1] .. " {")
	for i,v in ipairs(action_names) do
		if i == 1 then
			wline("\tDASM_" .. v .. " = " .. actfirst .. ",")
		else
			wline("\tDASM_" .. v .. ",")
		end
	end
	wline("};");
	wline([[

#define lo16(n) (((signed int)n << 16) >> 16)
#define hi16(n) ((signed int)n >> 16)
#define ha16(n) ((lo16(n) < 0) ? hi16(n) + 1 : hi16(n))

]]);
end

map_op[".immencmodes_0"] = function(params)
	wline("#include \"dasm_tilepro64_encmodes.h\"");
end


------------------------------------------------------------------------------

map_op[".space_1"] = function(params)
	local siz = params[1]
	-- can only place multiples of 4 bytes (32 bit)
	local i
	for i=1,siz do
		wputw(0)
	end
end

map_op[".word_1"] = function(params)
	local val = params[1]
	wputw(tonumber(val))
end

------------------------------------------------------------------------------

-- Set the current section.
function _M.section(num)
  waction("SECTION")
  wputxw(num)
  wflush(true) -- SECTION is a terminal action.
end

------------------------------------------------------------------------------

-- Dump architecture description.
function _M.dumparch(out)
  out:write(format("DynASM %s version %s, released %s\n\n",
    _info.arch, _info.version, _info.release))
  dumpregs(out)
  dumpactions(out)
end

-- Dump all user defined elements.
function _M.dumpdef(out, lvl)
  dumptypes(out, lvl)
  dumpglobals(out, lvl)
end

------------------------------------------------------------------------------

-- Pass callbacks from/to the DynASM core.
function _M.passcb(wl, we, wf, ww)
  wline, werror, wfatal, wwarn = wl, we, wf, ww
  return wflush
end

-- Setup the arch-specific module.
function _M.setup(arch, opt)
  g_arch, g_opt = arch, opt
end

-- Merge the core maps and the arch-specific maps.
function _M.mergemaps(map_coreop, map_def)
  setmetatable(map_op, { __index = map_coreop })
  setmetatable(map_def, { __index = map_archdef })
  return map_op, map_def
end

return _M

------------------------------------------------------------------------------

