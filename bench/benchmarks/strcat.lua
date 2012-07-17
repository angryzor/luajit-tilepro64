-- $Id: strcat.lua,v 1.3 2004-07-08 03:47:07 bfulgham Exp $
-- http://shootout.alioth.debian.org
-- contributed by Roberto Ierusalimschy

require 'benchmarks/bench'

for pass = 1,2 do

local n = tonumber((arg and arg[1]) or 1)
local buff = {}
for i=1,n do
  table.insert(buff, "hello\n")
end
local s = table.concat(buff)
print(string.len(s))

logPass(pass)
end
logEnd()

