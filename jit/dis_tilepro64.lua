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
		fmt = fmt .. " "
		for i = 1, #... - 1 do
			fmt = fmt .. "%s, "
		end
		if #... ~= 0 then
			fmt = fmt .. "%s"
		end
	end

	wtxt(ctx, string.format(fmt, mnemonic, ...))
end

--------------------------------------------------------------------------------

local function process_X0_Unary_0(ctx, shopcex, opcex, Dest, A)
	wstmt(ctx, map_X0_Unary_0[opcex], map_reg[Dst], map_reg[A])
end

local function process_X1_Unary_0(ctx, shopcex, opcex, Dest, A)
	wstmt(ctx, map_X1_Unary_0[opcex], map_reg[Dst], map_reg[A])
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

	
local map_X1 = {
	[1] = process_X1_RRR,
	[8] = process_X1_Unary
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

local function process(code)
	local ctx = {	code = code,
					pos = 1,
					labels = {},
					actions = {},
					out = io.stdout }
	while ctx.pos < string.len(code) do
		process_instruction(ctx)
	end
	for i,v in ipairs(ctx.actions) do
		v()
	end
	ctx.out:flush()
end

process(string.char(0x02, 0x71, 0x0D, 0x00, 0x00, 0x88, 0x0B, 0x40))
