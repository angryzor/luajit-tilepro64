-- $Id: fibo.lua,v 1.3 2005-04-25 19:01:38 igouy-guest Exp $
-- http://www.bagley.org/~doug/shootout/

require 'benchmarks/bench'

function fib(n)
    if (n < 2) then return(1) end
    return( fib(n-2) + fib(n-1) )
end

for pass = 1,2 do

N = tonumber((arg and arg[1])) or 1
io.write(fib(N), "\n")

logPass(pass)

end

logEnd()

