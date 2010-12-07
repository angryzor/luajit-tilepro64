--[[

  Written by Ruben Tytgat

--]]

require("bit")
-- reg numbers to reg names
local map_reg = { [63] = "zero" }
for i = 0, 53 do
	map_reg[i] = "r" .. i
end

----------------------------------------------------------------

-- X0 Opcode maps
-----------------------------------
local map_X0_RRR = {
	[0x003] = "add",
	[0x05D] = "sub"
}

local map_X0_Unary_0 = {
	[0x11] = "fnop"
}

-- X1 Opcode maps
-----------------------------------
local map_X1_RRR = {
	[0x003] = "add",
	[0x03F] = "sub"
}

local map_X1_Unary_0 = {
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

--------------------------------------------------------------------------------

local function process_X0_Unary_0(ctx, shopcex, opcex, Dst, A)
	wstmt(ctx, map_X0_Unary_0[opcex])
end

local function process_X1_Unary_0(ctx, shopcex, opcex, Dst, A)
	wstmt(ctx, map_X1_Unary_0[opcex])
end

--
------------------------------------

local map_X0_Unary_Sh = {
	[0x00B] = process_X0_Unary_0
}

local map_X1_Unary_Sh = {
	[0x00B] = process_X1_Unary_0
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

local function process_X0_Unary(ctx, hi, lo)
	local shopcex = bit.band(bit.rshift(lo,17), 0x3FF)
	local opcex = bit.band(bit.rshift(lo,12), 0x1F)
	local A = bit.band(bit.rshift(lo,6), 0x3F)
	local Dst = bit.band(lo, 0x3F)

	map_X0_Unary_Sh[shopcex](ctx, shopcex, opcex, Dst, A)
end

	
local map_X0 = {
	[0] = process_X0_RRR,
	[7] = process_X0_Unary
}


-- X1 handling
----------------
local function process_X1_RRR(ctx, hi, lo)
	local opcex = bit.band(bit.rshift(hi,17), 0x1FF)
	local A = bit.band(bit.rshift(hi,5), 0x3F)
	local B = bit.band(bit.rshift(hi,11), 0x3F)
	local Dst = bit.bor(bit.lshift(bit.band(hi, 0x1F), 1),
						bit.rshift(hi,31))

	wstmt(ctx, map_X1_RRR[opcex], map_reg[Dst], map_reg[A], map_reg[B])
end

local function process_X1_Unary(ctx, hi, lo)
	local shopcex = bit.band(bit.rshift(hi,16), 0x3FF)
	local opcex = bit.band(bit.rshift(hi,11), 0x1F)
	local A = bit.band(bit.rshift(hi,5), 0x3F)
	local Dst = bit.bor(bit.lshift(bit.band(hi, 0x1F), 1),
						bit.rshift(hi,31))

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
