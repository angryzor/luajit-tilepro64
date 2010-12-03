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
  "IMM_B", "IMM_H",
  -- action arg (1 byte) or int arg, 1 buffer pos (link):
  "LG", "PC",
  -- action arg (1 byte) or int arg, 1 buffer pos (offset):
  "LABEL_LG", "LABEL_PC",
  -- action arg (1 byte), 1 buffer pos (offset):
  "ALIGN",
  -- action arg (1 byte), no buffer pos.
  "ESC",
  -- no action arg, no buffer pos.
  "SECTION",
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
  assert(n >= -214748364 and n <= 2147483647 and n % 1 == 0, "byte out of range")
  actlist[#actlist+1] = n
end

-- Add action to list with optional arg. Advance buffer pos, too.
local function waction(action, a, num)
  wputxw(assert(map_action[action], "bad action name `"..action.."'"))
  if a then actargs[#actargs+1] = a end
  if a or num then secpos = secpos + (num or 1) end
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
local next_global = 10
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
  for i=10,next_global-1 do
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

-- Put signed byte or arg.
local function wputbarg(n)
  if type(n) == "number" then
    if n < -128 or n > 127 then
      werror("signed immediate byte out of range")
    end
--    if n < 0 then n = n + 256 end
    wputxw(n)
  else waction("IMM_B", n) end
end

-- Put signed halfword or arg.
local function wputharg(n)
  if type(n) == "number" then
    if n < -32768 or n > 32767 then
      werror("signed immediate halfword out of range")
    end
    wputxw(n);
  else waction("IMM_H", n) end
end

------------------------------------------------------------------------------

------------------------------------------------------------------------------

--[[ Label pseudo-opcode (converted from trailing colon form).
map_op[".label_2"] = function(params)
  if not params then return "[1-9] | ->global | =>pcexpr  [, addr]" end
  local a = parseoperand(params[1])
  local mode, imm = a.mode, a.imm
  if type(imm) == "number" and (mode == "iJ" or (imm >= 1 and imm <= 9)) then
    -- Local label (1: ... 9:) or global label (->global:).
    waction("LABEL_LG", nil, 1)
    wputxw(imm)
  elseif mode == "iJ" then
    -- PC label (=>pcexpr:).
    waction("LABEL_PC", imm)
  else
    werror("bad label definition")
  end
  -- SETLABEL must immediately follow LABEL_LG/LABEL_PC.
  local addr = params[2]
  if addr then
    local a = parseoperand(params[2])
    if a.mode == "iPJ" then
      waction("SETLABEL", a.imm) -- !x64 (secpos)
    else
      werror("bad label assignment")
    end
  end
end
map_op[".label_1"] = map_op[".label_2"]
--]]
------------------------------------------------------------------------------
--[==[
-- Alignment pseudo-opcode.
map_op[".align_1"] = function(params)
  if not params then return "numpow2" end
  local align = tonumber(params[1]) or map_opsizenum[map_opsize[params[1]]]
  if align then
    local x = align
    -- Must be a power of 2 in the range (2 ... 256).
    for i=1,8 do
      x = x / 2
      if x == 1 then
	waction("ALIGN", nil, 1)
	wputxw(align-1) -- Action byte is 2**n-1.
	return
      end
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
--[[
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
  if reg and not map_reg_valid_base[reg] then
    werror("bad base register `"..(map_reg_rev[reg] or reg).."'")
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


local operand = {
	patterns = {
		reg = {
			gpr = "r%d+",
			sprs = {
				zero = 63
			}
		},
		imm = {
			imm = "%-?%d+"
		},
		ref = {
			ref = "[%w_][%w%d_%[%]%.%-%>]"
		}
	}
}

function operand.parsereg(str)
	local t
	if string.match(str,operand.patterns.reg.gpr) then
		local regnum = str.sub(str,2) + 0
		if regnum > 53 then
			error("register does not exist: " .. str)
		end
		
		return regnum
	else
		t = operand.patterns.reg.sprs[str]
		if t then
			return t
		end
	end
	error("operand must be register: " .. str)
end

local function make_param(off, siz)
	return { offset = off,
			 size = siz }
end

local function make_instr(hi, lo, params)
	return { hi = hi,
			 lo = lo,
			 params = params }
end

-----------------------------------------------------------------
-- Instruction combine
function instr_combine(a, b, c)
	local combinedparams = a.params
	for i,v in ipairs(b.params) do
		combinedparams[#params] = v
	end
	local combined = make_instr(bit.bor(a.hi,b.hi), bit.bor(a.lo,b.lo), combinedparams)

	if c then
		for i,v in ipairs(c.params) do
			combinedparams[#params] = v
		end
		combined = make_instr(bit.bor(combined.hi,c.hi), bit.bor(combined.lo,c.lo), combinedparams)
	end

	return combined
end

-----------------------------------------------------------------
-- Instruction builders
local instr_builders = {}
function instr_builders.X0_RRR(opcode, opcodeextension, Dst, A, B)
	local instr = Dst
	instr = bit.bor(instr, bit.lshift(A, 6))
	instr = bit.bor(instr, bit.lshift(B, 12))
	instr = bit.bor(instr, bit.lshift(opcodeextension, 18))
	instr = bit.bor(instr, bit.lshift(opcode, 28))
	return make_instr(0, instr, {})
end

------------------------------------------------------------------
-- Instruction parsers
local instr_parsers = {}
function instr_parsers.X0_RRR(opcode, opcodeextension)
	return function(params)
		local Dst, A, B = params[1], params[2], params[3]
		local i = instr_builders.X0_RRR(opcode, opcodeextension, operand.parsereg(Dst), operand.parsereg(A), operand.parsereg(B))
		return i
	end
end

------------------------------------------------------------------
-- Temporary workaround.
local function wrap_put_nop_X1(parser)
	return function(params)
		local i = instr_combine(make_instr(0x400B8800,0,{}), parser(params))
		wputw(i.lo)
		wputw(i.hi)
	end
end

-----------------------------------------------------------------
-- ops

local map_op = {
	add_3 = wrap_put_nop_X1(instr_parsers.X0_RRR(0, 3)),
	adds_3 = wrap_put_nop_X1(instr_parsers.X0_RRR(0, 0x60)),
	sub_3 = wrap_put_nop_X1(instr_parsers.X0_RRR(0,0x5D))
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

