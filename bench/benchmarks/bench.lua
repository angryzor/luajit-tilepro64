local lastTimeStamp = 0

--[[
local function logStart(i)
	lastTimeStamp = os.time()
end
]]

function logPass(i)
	local delta = os.clock() - lastTimeStamp

	print("bench pass" .. i .. ": iterations=1 runtime: " .. delta * 1000 .. "ms")

	collectgarbage()

	lastTimeStamp = os.clock()
end

function logEnd()
	print("bench total: iterations=1 runtime: " .. os.clock() * 1000 .. "ms") 
end

