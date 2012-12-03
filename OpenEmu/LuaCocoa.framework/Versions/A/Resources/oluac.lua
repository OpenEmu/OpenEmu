require 'olua'

local intext = io.open((...)):read("*a")
local outtext = olua.translate(intext)
print(outtext)

