require 'olua.lexer'
require 'olua.parser'
require 'olua.unparser'

local oldloadstring = loadstring
local string_gsub = string.gsub

module("olua", package.seeall)

--------------------------------------------------------------------------
--                               RUNTIME                                --
--------------------------------------------------------------------------

-- Tests to see if this is an object.

function isobject(o)
	return (type(o) == "table")
end

-- Allocate a new globally unique object ID.

do
	local currentobjectid = 1
	function newobjectid()
		local id = currentobjectid
		currentobjectid = currentobjectid + 1
		return id
	end
end

-- Mangles a selector into Lua form.

function mangle(s)
	return string_gsub(s, ":", "_")
end

-- Unmangles a selector into printable form.

function unmangle(s)
	return string_gsub(s, "_", ":")
end 

--------------------------------------------------------------------------
--                               DRIVERS                                --
--------------------------------------------------------------------------

-- This block of code defines the olua.translate and olua.loadstring
-- functions, and adds the Objective Lua loader to the Lua package
-- system.

function translate(intext, source)
	local ast = olua.parser(intext, source)
	return olua.unparser(ast)
end

function loadstring(intext, chunkname)
	local outtext, e = translate(intext, chunkname)
	if outtext then
		return oldloadstring(outtext, "@"..chunkname)
	end
	return nil, e
end

function loadstream(file, filename)
	-- Compile and return the module
	local m, e = loadstring(assert(file:read("*a")), filename)
	if not m then
		error(e)
	end
	return function(...)
		local args = {...}
		local r, e = xpcall(
			function() m(unpack(args)) end,
			function(e)
				if (type(e) == "table") then
					return e
				end
				
				if olua.lib and olua.lib.LegacyException and
						olua.lib.LegacyException.createFrom_ then
					return olua.lib.LegacyException:createFrom_(e)
				end
				return e
			end)
		if not r then
			local tb = ""
			if (type(e) == "table") then
				if e.traceback then
					tb = "\n" .. e:traceback()
				end
			end
			error(e, 0)
		end
		return e
	end 
end

function run(filename, ...)
	local file = assert(io.open(filename, "rb"))
	local chunk = assert(loadstream(file, filename))
	file:close()
	return chunk(...)
end

local function oluaload(modulename)
	local errmsg = ""
	local modulepath = string.gsub(modulename, "%.", "/")
	for path in string.gmatch(package.oluapath, "([^;]+)") do
		local filename = string.gsub(path, "%?", modulepath)
		local file = io.open(filename, "rb")
		if file then
			--print("Loading "..filename)
			local chunk = assert(loadstream(file, filename))
			file:close()
			return chunk
		end
		errmsg = errmsg.."\n\tno file '"..filename.."' (checked with custom loader)"
	end
	return errmsg
end

-- Copy the standard Lua path to make an olua path.

package.oluapath = package.path:gsub("%.lua", ".olua")

-- Install our new loader.

table.insert(package.loaders, 2, oluaload)
