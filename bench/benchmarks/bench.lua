local lastTimeStamp = 0

--[[
local function logStart(i)
	lastTimeStamp = os.time()
end
]]

function logPass(i)
	local t = os.clock()
	local delta = t - lastTimeStamp

	lastTimeStamp = t

	print("bench pass" .. i .. ": iterations=1 runtime: " .. delta * 1000 .. "ms")
end

function logEnd()
	print("bench total: iterations=1 runtime: " .. os.clock() * 1000 .. "ms") 
end

