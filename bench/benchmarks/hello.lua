-- $Id: hello.lua,v 1.1.1.1 2004-05-19 18:10:16 bfulgham Exp $
-- http://www.bagley.org/~doug/shootout/

require 'benchmarks/bench'

for pass = 1,2 do

io.write("hello world\n")

logPass(pass)

end

logEnd()
