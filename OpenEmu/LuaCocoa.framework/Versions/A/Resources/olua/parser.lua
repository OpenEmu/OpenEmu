require 'olua.lexer'
require 'olua.transform'

module(..., package.seeall)

local source
local tokens
local pos

local peek
local expect
local optionalexpect

local chunk
local stat
local laststat
local doend
local ifelseend
local whiledoend
local repeatuntil
local forend
local functiondef
local assignment
local localdef
local rvaluelist
local rvalue
local rvalue2
local rvalue3
local rvalue4
local rvalue5
local rvalue6
local rvalue7
local rvalue8
local rvalue9
local rvalueleaf
local lvaluelist
local lvalue
local lvalueleaf
local lvalueidentifier
local functioncall
local tableliteral

local olua_methodcall
local olua_implementation
local olua_implementation_chunk
local olua_methoddefinition
local olua_throw
local olua_trycatchfinally

function parseerror(message)
	local t = tokens[pos]
	local line = "unknown location"
	if t and t.line then
		line = t.line
	end
	
	error("Parse error at "..line.." of "..source..": "..message)
end

local expectederror = function(type, text)
	local t = tokens[pos]
	if not t then
		t = {type="eof", text=""}
	end
	
	text = text or "(anything)"
	parseerror("expected a "..type..", '"..text.."', but got a "..t.type..", '"..t.text.."'")
end

local unexpectederror = function(token)
	parseerror("unexpected "..token.type..", '"..token.text.."'")
end
 
local statementerror = function(token)
	parseerror("expected a statement, got a "..token.type..", '"..token.text.."'")
end

local internalerror = function()
	parseerror("internal error")
end

peek = function(type, ...)
	local t = tokens[pos]
	if not t then
		return nil
	end
	
	if (t.type ~= type) then
		return false
	end
	
	local list = {...}
	if (#list == 0) then
		return true
	end
	
	for _, s in ipairs(list) do
		if (t.text == s) then
			return true
		end
	end
	return false
end

optionalexpect = function(type, ...)
	local t = tokens[pos]
	if not t then
		return nil
	end
	
	local list = {...}
	if (#list == 0) and (t.type == type) then
		pos = pos + 1
		return t
	end
	
	for _, s in ipairs(list) do
		if (t.text == s) then
			pos = pos + 1
			return t
		end
	end

	return nil
end

expect = function(type, ...)
	local t = optionalexpect(type, ...)
	if not t then
		expectederror(type, ...)
	end
	return t
end

--------------------------------------------------------------------------
--                              BNF PROPER                              --
--------------------------------------------------------------------------

local function isterminatorkeyword(s)
	return (s == "end") or (s == "else") or
				(s == "elseif") or (s == "until") or
				(s == "@end") or (s == "@catch") or
				(s == "@finally")
end

chunk = function()
	local ast = {type="chunk"}
	
	while tokens[pos] do
		local t = tokens[pos]
		if (t.type == "keyword") then
			if isterminatorkeyword(t.text) then
				break
			end
		
			if (t.text == "return") or (t.text == "break") then
				local a = laststat()
				ast[#ast+1] = a
				
				optionalexpect("operator", ";")
				break
			end
		end
		
		local a = stat()
		ast[#ast+1] = a
		
		optionalexpect("operator", ";")
	end
	
	return ast
end

stat = function()
	local t = tokens[pos]
	if (t.type == "keyword") then
		if (t.text == "do") then
			return doend()
		elseif (t.text == "while") then
			return whiledoend()
		elseif (t.text == "repeat") then
			return repeatuntil()
		elseif (t.text == "if") then
			return ifelseend()
		elseif (t.text == "for") then
			return forend()
		elseif (t.text == "function") then
			return functiondef()
		elseif (t.text == "local") then
			return localdef()
		elseif (t.text == "@implementation") then
			return olua_implementation()
		elseif (t.text == "@throw") then
			return olua_throw()
		elseif (t.text == "@try") then
			return olua_trycatchfinally()
		else
			statementerror(t)
		end
	else
		local e = rvalue()
		if peek("operator", ",") or peek("operator", "=") then
			return assignment(e)
		else
			--return functioncall(e)
			return e
		end
	end
end

laststat = function()
	local t = tokens[pos]
	if (t.type == "keyword") then
		if (t.text == "break") then
			pos = pos + 1
			return {type="break"}
		elseif (t.text == "return") then
			pos = pos + 1
			
			local value
			if (tokens[pos].type == "keyword") and
				isterminatorkeyword(tokens[pos].text) then
			else
				value = rvaluelist()
			end
				 
			return {type="return", value=value}
		end
	end
	
	statementerror(t)
end

doend = function()
	expect("keyword", "do")
	local ast = {type="doend", chunk=chunk()}
	expect("keyword", "end")
	return ast
end

ifelseend = function()
	expect("keyword", "if")
	local ast = {type="ifelseend"}
	while true do
		local condition = rvalue()
		expect("keyword", "then")
		local c = chunk()
		
		ast[#ast+1] = {condition=condition, chunk=c}

		local t = expect("keyword", "elseif", "else", "end")
		if (t.text == "end") then
			break
		end
		if (t.text == "else") then
			c = chunk()
			expect("keyword", "end")
			ast[#ast+1] = {chunk=c}
			break
		end
	end
	
	return ast
end

whiledoend = function()
	expect("keyword", "while")
	local c = rvalue()
	expect("keyword", "do")
	local b = chunk()
	expect("keyword", "end")
	
	return {type="whiledoend", condition=c, chunk=b}
end

repeatuntil = function()
	expect("keyword", "repeat")
	local b = chunk()
	expect("keyword", "until")
	local c = rvalue()
	
	return {type="repeatuntil", condition=c, chunk=b}
end

forend = function()
	expect("keyword", "for")
	local ast = {}
	
	ast.var = lvalue()
	if peek("operator", "=") then
		pos = pos + 1
		ast.type = "forend"
		
		ast.low = rvalue()
		expect("operator", ",")
		ast.high = rvalue()
		if peek("operator", ",") then
			pos = pos + 1
			ast.step = rvalue()
		end
	else
		ast.var = lvaluelist(ast.var)
		expect("keyword", "in")
		ast.type = "forinend"
		
		ast.iterator = rvalue()
	end
	
	expect("keyword", "do")
	ast.chunk = chunk()
	expect("keyword", "end") 
	
	return ast
end

functiondef = function()
	expect("keyword", "function")
	local name
	if not peek("operator", "(") then
		name = lvalue()
	end
	expect("operator", "(")
	local args = lvaluelist()
	expect("operator", ")")
	local c = chunk()
	expect("keyword", "end")
	
	return {type="functiondef", name=name, args=args, chunk=c}
end

assignment = function(e)
	if not e or peek("operator", ",") then
		e = lvaluelist(e)
	end
	expect("operator", "=")
	local a = rvaluelist()
	return {type="assignment", left=e, right=a}
end

localdef = function()
	expect("keyword", "local")
	local ast
	if peek("keyword", "function") then
		ast = functiondef()
	else
		local e = lvaluelist()
		local a = nil
		if peek("operator", "=") then
			pos = pos + 1
			a = rvaluelist()
		end
		 
		ast = {type="assignment", left=e, right=a}
	end
	
	return {type="local", value=ast}
end

rvaluelist = function(e)
	local ast = {type="list"}
	if not e then
		if peek("operator", ")") then
			return ast
		end
		e = rvalue()
	end

	ast[#ast+1] = e
	while true do
		if not peek("operator", ",") then
			return ast
		end
		pos = pos + 1
		ast[#ast+1] = rvalue()
	end
end	

local function leftassocbinop(left, precedence, type, ...)
	local e = left()
	if peek(type, ...) then
		local t = tokens[pos]
		pos = pos + 1
		
		local ast = {type="binop", operator=t, precedence=precedence,
			left=e, right=left()}
		while peek(type, ...) do
			t = tokens[pos]
			pos = pos + 1
			
			ast = {type="binop", operator=t, precedence=precedence,
				left=ast, right=left()}
		end
		return ast
	else
		return e
	end
end

local function rightassocbinop(left, precedence, type, ...)
	local e = left()
	if peek(type, ...) then
		local t = tokens[pos]
		pos = pos + 1
		return {type="binop", operator=t, precedence=precedence,
			left=e, right=rvalue()}
	else
		return e
	end
end

-- or
rvalue = function()
	return leftassocbinop(rvalue2, 1, "keyword", "or")
end

-- and
rvalue2 = function()
	return leftassocbinop(rvalue3, 2, "keyword", "and")
end

-- < > <= >= ~= ==
rvalue3 = function()
	return leftassocbinop(rvalue4, 3, "operator",
		"<", ">", "<=", ">=", "~=", "==")
end

-- ..
rvalue4 = function()
	return rightassocbinop(rvalue5, 4, "operator", "..")
end

-- + -
rvalue5 = function()
	return leftassocbinop(rvalue6, 5, "operator", "+", "-")
end

-- * / %
rvalue6 = function()
	return leftassocbinop(rvalue7, 6, "operator", "*", "/", "%")
end

-- not # unary-
rvalue7 = function()
	if peek("operator", "-") or peek("operator", "#") or
			peek("keyword", "not") then
		local t = tokens[pos]
		pos = pos + 1
		return {type="unop", precedence=7, operator=t.text, value=rvalue8()}
	end
	
	return rvalue8()
end

-- ^
rvalue8 = function()
	return rightassocbinop(rvalue9, 8, "operator", "^")
end

-- . : [] functioncalls
rvalue9 = function()
	local e = rvalueleaf()
	while true do
		t = tokens[pos]
		if not t then
			break
		end
		if (t.type == "operator") then		
			if (t.text == ".") or (t.text == ":") then
				pos = pos + 1
				e = {type="binop", operator=t,
					precedence=9, left=e, right=rvalueleaf()}
			elseif (t.text == "[") then
				-- This might be either a dereference or the beginning of an
				-- Objective Lua method call in the next statement; so we
				-- need to disambiguate and potentially backtrack.
				--
				-- It's all right for rvalue() to bail with an error because
				-- method calls start with an rvalue as well. It'd be nice to
				-- avoid parsing the rvalue twice, but it does no harm.
				
				local oldpos = pos
				pos = pos + 1
				local index = rvalue()
				if optionalexpect("operator", "]") then
					-- Dereference.
					e = {type="deref", left=e, right=index}
				else
					-- Method call. Backtrack.
					pos = oldpos
					return e
				end
			elseif (t.text == "(") or (t.text == "{") then
				e = functioncall(e)
			else
				break
			end
		elseif (t.type == "string") then
			e = functioncall(e)
		else
			break
		end
	end
	
	return e
end

local function leaf(t)
	return {type="leaf", value=t}
end
 
rvalueleaf = function()
	local t = tokens[pos]
	if (t.type == "keyword") then
		if (t.text == "nil") or (t.text == "true") or
				(t.text == "false") then
			pos = pos + 1
			return leaf(t)
		elseif (t.text == "function") then
			return functiondef()
		end
	elseif (t.type == "number") or (t.type == "string") or
			(t.type == "identifier") then
		pos = pos + 1
		return leaf(t)
	elseif (t.type == "operator") then
		if (t.text == "(") then
			pos = pos + 1
			local e = rvalue()
			expect("operator", ")")
			return e
		elseif (t.text == "{") then
			return tableliteral()
		elseif (t.text == "[") then
			return olua_methodcall()
		elseif (t.text == "...") then
			pos = pos + 1
			return leaf(t)
		end
	end
	
	unexpectederror(t)	
end

lvaluelist = function(e)
	local ast = {type="list"}
	if not e then
		if peek("operator", ")") then
			return ast
		end
		e = lvalue()
	end

	ast[#ast+1] = e
	while true do
		if not peek("operator", ",") then
			return ast
		end
		pos = pos + 1
		ast[#ast+1] = lvalue()
	end
end	

lvalue = function()
	local ast = lvalueleaf()
	while true do
		local t = tokens[pos]
		if (t.type == "operator") then
			if (t.text == ".") or (t.text == ":") then
				local t = tokens[pos]
				pos = pos + 1
				ast = {type="binop", operator=t.text, precedence=1,
					left=ast, right=lvalueleaf()}
			elseif (t.text == "[") then
				pos = pos + 1
				local index = rvalue()
				expect("operator", "]")
				ast = {type="deref", left=ast, right=index}
			else
				break
			end
		else
			break
		end
	end
	return ast
end

lvalueleaf = function()
	local t = tokens[pos]
	if (t.type == "identifier") then
		pos = pos + 1
		return {type="leaf", value=t}
	elseif (t.type == "operator") then
		if (t.text == "...") then
			pos = pos + 1
			return leaf(t)
		end
	end
	
	unexpectederror(t)	
end

lvalueidentifier = function()
	local t = expect("identifier")
	return leaf(t)
end

functioncall = function(value)
	local args
	
	if peek("string") or peek("operator", "{") then
		args = rvalueleaf()
	else
		expect("operator", "(")
		args = rvaluelist()
		expect("operator", ")")
	end
	
	
	return {type="functioncall", value=value, args=args}
end

tableliteral = function()
	local ast = {type="tableliteral"}
	expect("operator", "{")
	while not peek("operator", "}") do 
		local entry = {}
		if (tokens[pos].type == "identifier") and
				(tokens[pos+1].type == "operator") and
				(tokens[pos+1].text == "=") then
			entry.key = tokens[pos]
			pos = pos + 2
			entry.value = rvalue()
		elseif (tokens[pos].type == "operator") and
				(tokens[pos].text == "[") then
			-- Disambiguate [expr]=value from [expr selector].
			
			local oldpos = pos
			pos = pos + 1
			entry.key = rvalue()
			if not optionalexpect("operator", "]") then
				-- This must be a method call. Backtrack.
				pos = oldpos
				entry.key = nil
				entry.value = rvalue()
			else
				-- Key/value assignment.
				expect("operator", "]")
				expect("operator", "=")
				entry.value = rvalue()
			end
		else
			entry.value = rvalue()
		end
		
		ast[#ast + 1] = entry
		if not optionalexpect("operator", ",", ";") then
			break
		end
	end
	expect("operator", "}")
	
	return ast
end

--------------------------------------------------------------------------
--                        OBJECTIVE LUA EXTENSIONS                      --
--------------------------------------------------------------------------

local function peekselel()
	local t1 = tokens[pos+0]
	local t2 = tokens[pos+1]
	
	if not t1 or not t2 then
		return false
	end
	
	local isselel = ((t1.type == "identifier") or (t1.type == "keyword")) and
		(t2.type == "operator") and (t2.text == ":")
		
	return isselel, t1, t2
end

local function getselel()
	local b, t1, t2 = peekselel()
	if b then
		pos = pos + 2
		return t1.text .. ":"
	end
	return nil
end

olua_methodcall = function()
	expect("operator", "[")
	local object = nil
	if not optionalexpect("identifier", "super") then
		object = rvalue()
	end
	local selector = {}
	local args = {}
	
	if not peek("identifier") and not peek("keyword") then
		expectederror("identifier or selector element")
	end

	if not peekselel() then
		-- Single-element selector.
		selector[#selector + 1] = tokens[pos].text
		pos = pos + 1
	else
		while true do
			t = getselel()
			if not t then
				break
			end
			
			selector[#selector + 1] = t
			args[#args + 1] = rvalue9()
		end
	end
	
	while optionalexpect("operator", ",") do
		args[#args + 1] = rvalue()
	end
	
	expect("operator", "]")
	
	return olua.transform.methodcall 
		{
			type="olua_methodcall",
			object=object,
			selector=table.concat(selector),
			args=args
		}
end

olua_implementation = function()
	expect("keyword", "@implementation")
	
	local class = expect("identifier")
	local superclass = nil
	local category = nil
	if optionalexpect("operator", ":") then
		superclass = rvalue9()
	elseif optionalexpect("operator", "(") then
		category = expect("identifier")
		expect("operator", ")") 
	end
	
	local ast =
		{
			type="olua_implementation",
		 	class=class,
		 	superclass=superclass,
		 	category=category,
		 	chunk=c
		}
		
	while tokens[pos] do
		local t = tokens[pos]
		if (t.type == "keyword") then
			if (t.text == "@end") then
				break
			end
		end
		
		local a = olua_implementation_stat()
		ast[#ast+1] = a
	end
		
	expect("keyword", "@end")
	
	return olua.transform.implementation(ast)
end

olua_implementation_stat = function()
	local t = tokens[pos]
	if (t.type == "keyword") then
		if (t.text == "@statics") then
			pos = pos + 1
			local c = chunk()
			expect("keyword", "@end")
			return c
		end
	else
		if (t.type == "operator") and
				((t.text == "-") or (t.text == "+")) then
			return olua_methoddefinition()		
		end
	end
	
	statementerror(t)
end

olua_methoddefinition = function()
	local t = expect("operator", "-", "+")
	local classmethod = (t.text == "+")
	
	local selector = {}
	local argtypes = {}
	local rettype = ""
	local args = {}
	local extraargs = {}
	
	if optionalexpect("operator", "(") then
		rettype = expect("identifier").text
		expect("operator", ")")
	end
	
	if not peek("identifier") and not peek("keyword") then
		expectederror("identifier or selector element")
	end

	if not peekselel() then
		-- Single-element selector.
		selector[#selector + 1] = tokens[pos].text
		pos = pos + 1
	else
		while true do
			t = getselel()
			if not t then
				break
			end
			
			selector[#selector + 1] = t
			
			if optionalexpect("operator", "(") then
				argtypes[#argtypes + 1] = expect("identifier").text
				expect("operator", ")")
			else
				argtypes[#argtypes + 1] = ""
			end
			
			args[#args + 1] = lvalueidentifier()
		end
	end

	if optionalexpect("operator", ",") then
		extraargs = lvaluelist()
	end
	
	expect("keyword", "do")
	local c = chunk()
	expect("keyword", "end")
	
	return {
			type="olua_methoddefinition",
			selector=table.concat(selector),
			args=args,
			argtypes=argtypes,
			rettype=rettype,
			extraargs=extraargs,
			classmethod=classmethod,
			chunk=c
		}
end

olua_throw = function()
	expect("keyword", "@throw")
	local exception = rvalue()
	
	return olua.transform.throw
		{
			type = "olua_throw",
			exception = exception
		}
end

olua_trycatchfinally = function()
	local try, catch, catchvar, finally
	
	expect("keyword", "@try")
	try = chunk()
	expect("keyword", "@catch")
	expect("operator", "(")
	catchvar = lvalue()
	expect("operator", ")")
	catch = chunk()

	if optionalexpect("keyword", "@finally") then
		finally = chunk()
	end
	
	expect("keyword", "@end")
	
	return olua.transform.trycatchfinally
		{
			type = "olua_trycatchfinally",
			try = try,
			catch = catch,
			catchvar = catchvar,
			finally = finally
		}
end
 
--------------------------------------------------------------------------
--                              FRONT END                               --
--------------------------------------------------------------------------

local parser = function(self, input, _source)
	source = _source or "(unknown)"
	tokens = olua.lexer(input)
	pos = 1

	local c = chunk()
	if tokens[pos] then
		unexpectederror(tokens[pos])
	end
	return c	
end

getmetatable(getfenv(1)).__call = parser
