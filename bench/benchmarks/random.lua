-- The Computer Language Shootout
-- http://shootout.alioth.debian.org/
-- contributed by Roberto Ierusalimschy, tuned by Mike Pall

require 'benchmarks/bench'

local IM = 139968
local IA = 3877
local IC = 29573
local LAST = 42

local function gen_random(max)
  local y = (LAST * IA + IC) % IM
  LAST = y
  return (max * y) / IM
end

for pass = 1,2 do

local N = tonumber(arg and arg[1]) or 1
for i=2,N do gen_random(100) end
io.write(string.format("%.9f\n", gen_random(100)))

logPass(pass)
end
logEnd()

