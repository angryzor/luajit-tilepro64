--[[

  Written by Ruben Tytgat

--]]

require("bit")
-- reg numbers to reg names
local map_reg = {	[54] = "sp",
					[55] = "lr",
					[63] = "zero" }
for i = 0, 52 do
	map_reg[i] = "r" .. i
end

----------------------------------------------------------------

-- X0 Opcode maps
-----------------------------------
local map_X0_RRR = {
	[0x003] = "add",
	[0x033] = "or",
	[0x05D] = "sub",
	[0x060] = "adds"
}

local map_X0_Imm8 = {
	[0x03] = "addi",
	[0x08] = "ori"
}

local map_X0_Unary_00B = {
	[0x05] = "fnop"
}

-- X1 Opcode maps
-----------------------------------
local map_X1_RRR = {
	[0x003] = "add",
	[0x019] = "or",
	[0x03F] = "sub",
	[0x040] = "sw",
	[0x042] = "adds"
}

local map_X1_Imm8 = {
	[0x03] = "addi",
	[0x0B] = "ori"
}

local map_X1_Unary_00B = {
	[0x0E] = "lw",
	[0x11] = "fnop"
}

----------------------------------------

local function wcb(ctx, cb)
	ctx.actions[#ctx.actions+1] = cb
end

local function wtxt(ctx, txt)
	wcb(ctx,function()
		ctx.out:write(txt)
	end)
end

local function wstmt(ctx, mnemonic, ...)
	local fmt = "%s"

	if ... then
		local siz = #{...}

		fmt = fmt .. " "
		for i = 1, siz - 1 do
			fmt = fmt .. "%s, "
		end
		if siz ~= 0 then
			fmt = fmt .. "%s"
		end
	end

	wtxt(ctx, string.format(fmt, mnemonic, ...))
end

local function newlabel(ctx, pos)
	if not ctx.labels[pos] then
		ctx.labels[pos] = "label_" .. bit.tohex(pos)
	end
end

local function signextend(x, siz)
	if bit.band(bit.rshift(x, siz-1), 1) ~= 1 then
		return x
	else
		return bit.bor(bit.lshift(-1, siz), x)
	end
end

--------------------------------------------------------------------------------

local exceptions_X0_Unary_00B_nothing = {
	[0x05] = 1
}
local function process_X0_Unary_00B(ctx, shopcex, opcex, Dst, A)
	if exceptions_X0_Unary_00B_nothing[opcex] then
		wstmt(ctx, map_X0_Unary_00B[opcex])
	else
		wstmt(ctx, map_X0_Unary_00B[opcex], map_reg[Dst], map_reg[A])
	end
end

local exceptions_X1_Unary_00B_nothing = {
	[0x11] = 1
}
local function process_X1_Unary_00B(ctx, shopcex, opcex, Dst, A)
	if exceptions_X1_Unary_00B_nothing[opcex] then
		wstmt(ctx, map_X1_Unary_00B[opcex])
	else
		wstmt(ctx, map_X1_Unary_00B[opcex], map_reg[Dst], map_reg[A])
	end
end

--
------------------------------------

local map_X0_Unary_Sh = {
	[0x00B] = process_X0_Unary_00B
}

local map_X1_Unary_Sh = {
	[0x00B] = process_X1_Unary_00B
}


-- X0 handling
----------------
local function process_X0_RRR(ctx, hi, lo)
	local opcex = bit.band(bit.rshift(lo,18), 0x1FF)
	local A = bit.band(bit.rshift(lo,6), 0x3F)
	local B = bit.band(bit.rshift(lo,12), 0x3F)
	local Dst = bit.band(lo, 0x3F)

	wstmt(ctx, map_X0_RRR[opcex], map_reg[Dst], map_reg[A], map_reg[B])
end

local function process_X0_Imm8(ctx, hi, lo)
	local opcex = bit.band(bit.rshift(lo,20), 0x7F)
	local A = bit.band(bit.rshift(lo,6), 0x3F)
	local Imm8 = bit.band(bit.rshift(lo,12), 0xFF)
	local Dst = bit.band(lo, 0x3F)

	wstmt(ctx, map_X0_Imm8[opcex], map_reg[Dst], map_reg[A], signextend(Imm8,8))
end

local function make_processor_X0_Imm16(mnemonic)
	return function(ctx, hi, lo)
		local A = bit.band(bit.rshift(lo,6), 0x3F)
		local Imm16 = bit.band(bit.rshift(lo,12), 0xFFFF)
		local Dst = bit.band(lo, 0x3F)

		wstmt(ctx, mnemonic, map_reg[Dst], map_reg[A], signextend(Imm16,16))
	end
end

local function process_X0_Unary(ctx, hi, lo)
	local shopcex = bit.band(bit.rshift(lo,17), 0x3FF)
	local opcex = bit.band(bit.rshift(lo,12), 0x1F)
	local A = bit.band(bit.rshift(lo,6), 0x3F)
	local Dst = bit.band(lo, 0x3F)

	map_X0_Unary_Sh[shopcex](ctx, shopcex, opcex, Dst, A)
end

	
local map_X0 = {
	[0] = process_X0_RRR,
	[2] = make_processor_X0_Imm16("addli"),
	[3] = make_processor_X0_Imm16("auli"),
	[4] = process_X0_Imm8,
	[7] = process_X0_Unary
}


-- X1 handling
----------------
local exceptions_X1_RRR_noDst = {
	[0x040] = 1
}
local function process_X1_RRR(ctx, hi, lo)
	local opcex = bit.band(bit.rshift(hi,17), 0x1FF)
	local A = bit.band(bit.rshift(hi,5), 0x3F)
	local B = bit.band(bit.rshift(hi,11), 0x3F)
	local Dst = bit.bor(bit.lshift(bit.band(hi, 0x1F), 1),
						bit.rshift(lo,31))
	if exceptions_X1_RRR_noDst[opcex] then
		wstmt(ctx, map_X1_RRR[opcex], map_reg[A], map_reg[B])
	else
		wstmt(ctx, map_X1_RRR[opcex], map_reg[Dst], map_reg[A], map_reg[B])
	end
end

local function process_X1_Imm8(ctx, hi, lo)
	local opcex = bit.band(bit.rshift(hi,19), 0x7F)
	local A = bit.band(bit.rshift(hi,5), 0x3F)
	local Imm8 = bit.band(bit.rshift(hi,11), 0xFF)
	local Dst = bit.bor(bit.lshift(bit.band(hi, 0x1F), 1),
						bit.rshift(lo,31))
	wstmt(ctx, map_X1_Imm8[opcex], map_reg[Dst], map_reg[A], signextend(Imm8,8))
end

local function make_processor_X1_Imm16(mnemonic)
	return function(ctx, hi, lo)
		local A = bit.band(bit.rshift(hi,5), 0x3F)
		local Imm16 = bit.band(bit.rshift(hi,11), 0xFFFF)
		local Dst = bit.bor(bit.lshift(bit.band(hi, 0x1F), 1),
						bit.rshift(lo,31))
		wstmt(ctx, mnemonic, map_reg[Dst], map_reg[A], signextend(Imm16,16))
	end
end

local function process_X1_Unary(ctx, hi, lo)
	local shopcex = bit.band(bit.rshift(hi,16), 0x3FF)
	local opcex = bit.band(bit.rshift(hi,11), 0x1F)
	local A = bit.band(bit.rshift(hi,5), 0x3F)
	local Dst = bit.bor(bit.lshift(bit.band(hi, 0x1F), 1),
						bit.rshift(lo,31))

	map_X1_Unary_Sh[shopcex](ctx, shopcex, opcex, Dst, A)
end



--------------------------------------------------------------
local function process_X1_J__extract_JOff(hi, lo)
	local j = bit.band(bit.rshift(hi, 11), 0x7FFF);
	j = bit.bor(j, bit.lshift(bit.band(bit.rshift(hi, 3), 3), 15))
	j = bit.bor(j, bit.lshift(bit.bor(bit.lshift(bit.band(hi, 7), 1), bit.rshift(lo, 31)), 17))
	j = bit.bor(j, bit.lshift(bit.band(bit.rshift(hi, 5), 0x3F), 21))
	j = bit.bor(j, bit.lshift(bit.band(bit.rshift(hi, 26), 1), 27))
	return j
end

local function process_X1_J__jb(ctx, hi, lo)
	local j = process_X1_J__extract_JOff(hi, lo)
	local nextpos = ctx.pos + bit.tobit(0x80000000) + bit.lshift(j, 3)

	newlabel(ctx, nextpos)
	wstmt(ctx, "jb", ctx.labels[nextpos])
end

local function process_X1_J__jf(ctx, hi, lo)
	local j = process_X1_J__extract_JOff(hi, lo)
	local nextpos = ctx.pos + bit.lshift(j, 3)

	newlabel(ctx, nextpos)
	wstmt(ctx, "jf", ctx.labels[nextpos])
end
	
local map_X1 = {
	[0x1] = process_X1_RRR,
	[0x3] = make_processor_X1_Imm16("addli"),
	[0x4] = make_processor_X1_Imm16("auli"),
	[0x6] = process_X1_Imm8,
	[0x8] = process_X1_Unary,
	[0xA] = process_X1_J__jf,
	[0xB] = process_X1_J__jb
}




--------------------------------------------------------------------------------
local function process_X0(ctx, hi, lo)
	local opc = bit.band(bit.rshift(lo,28), 7)
	map_X0[opc](ctx, hi, lo)
end

local function process_X1(ctx, hi, lo)
	local opc = bit.band(bit.rshift(hi,27), 0xF)
	map_X1[opc](ctx,hi,lo)
end

local function process_X(ctx, hi, lo)
	wtxt(ctx, "\t{ ")
	process_X1(ctx, hi, lo)
	wtxt(ctx, " ; ")
	process_X0(ctx, hi, lo)
	wtxt(ctx, " }\n")
end

local function process_instruction(ctx)
	local pos = ctx.pos
	local hi =	bit.bor(bit.lshift(string.byte(ctx.code,pos+7),24),
				bit.bor(bit.lshift(string.byte(ctx.code,pos+6),16),
				bit.bor(bit.lshift(string.byte(ctx.code,pos+5),8),
				string.byte(ctx.code,pos+4))))
	local lo =	bit.bor(bit.lshift(string.byte(ctx.code,pos+3),24),
				bit.bor(bit.lshift(string.byte(ctx.code,pos+2),16),
				bit.bor(bit.lshift(string.byte(ctx.code,pos+1),8),
				string.byte(ctx.code,pos))))

	-- Chceck if X type instruction
	if bit.band(0x80000000, hi) == 0 then
		process_X(ctx, hi, lo)
	else
		error("Y instruction disassembling not yet implemented")
	end
	ctx.pos = pos + 8
end

local function write_label(ctx)
	local cpos = ctx.pos
	wcb(ctx,function()
		if ctx.labels[cpos] then
			ctx.out:write(bit.tohex(cpos) .. " | \t" .. ctx.labels[cpos] .. ":\n")
		end
		ctx.out:write(bit.tohex(cpos) .. " | \t\t")
	end)
end

local function process(code)
	local ctx = {	code = "PADDING" .. code,
					pos = 8,
					labels = {},
					actions = {},
					out = io.stdout }
	while ctx.pos < string.len(ctx.code) do
		write_label(ctx)
		process_instruction(ctx)
	end
	for i,v in ipairs(ctx.actions) do
		v()
	end
	ctx.out:flush()
end

--[[
process(string.char(0x02, 0x71, 0x0D, 0x00, 0x00, 0x88, 0x0B, 0x40,
					0x40, 0x30, 0x75, 0x01, 0x00, 0x88, 0x0B, 0x40,
					0x09, 0xF0, 0x0F, 0x00, 0x00, 0x20, 0x00, 0x50,
					0x09, 0xF0, 0x0F, 0x00, 0x00, 0x88, 0x0B, 0x40,
					0x09, 0xF0, 0x0F, 0x80, 0xFF, 0xF7, 0xFF, 0x5F,
					0x40, 0x30, 0x75, 0x01, 0x00, 0x88, 0x0B, 0x40,
					0x09, 0xF0, 0x0F, 0x00, 0x00, 0x88, 0x0B, 0x40,
					0x40, 0x30, 0x75, 0x01, 0x00, 0x88, 0x0B, 0x40,
					0x09, 0xF0, 0x0F, 0x00, 0x00, 0x88, 0x0B, 0x40
			))
]]

local function process_alist(...)
	local al = {...}
	local i = 1
	local r = {}

	for i,v in ipairs(al) do
		r[#r+1] = bit.band(v, 0xFF)
		r[#r+1] = bit.band(bit.rshift(v,8), 0xFF)
		r[#r+1] = bit.band(bit.rshift(v,16), 0xFF)
		r[#r+1] = bit.band(bit.rshift(v,24), 0xFF)
	end

	process(string.char(unpack(r)))
end

process_alist(  1880510464,142720704,1077824950,1074497536,1076923823,1074497536,
  1880510464,142695904,1076940207,1074497536,1880510464,142693856,1076956591,
  1074497536,1880510464,142697952,1076972975,1074497536,1880510464,142702048,
  1076989359,1074497536,1880510464,142700000,13627499,1074497536,13627434,1074497536,
  536873650,1074497536,805309618,1074497536,-266973184,1074493017,
  13630700,1074497536,536873714,1074497536,805309618,1074497536,
  -266973184,1074493017,13630702,1074497536,536873650,1074497536,805309618,
  1074497536,-266973184,1074493017,13630701,1074497536,536873650,1074497536,
  805309618,1074497536,1880510464,1074493018,1076895027,1074497536,
  536873650,1074497536,805309618,1074497536,1880510464,142712384,
  1076989359,1074497536,-266973184,1074492918,1076972975,1074497536,1880510464,
  1074492919,1076956591,1074497536,1880510464,1074492918,1076940207,1074497536,
  1880510464,1074492917,1076923823,1074497536,-266973184,1074492917,1077005750,
  1074497536,-266973184,1074493147  ) 
