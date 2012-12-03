require 'olua.lexer'

module(..., package.seeall)

local result
local recursivelyunparse
local recursivelyunparsebinop

local function emit(s)
	result[#result + 1] = s
end

local typetable =
{
	chunk = function(ast)
		for _, statement in ipairs(ast) do
			recursivelyunparse(statement)
		end
	end,
	
	doend = function(ast)
		emit("do")
		recursivelyunparse(ast.chunk)
		emit("end")
	end,
	
	ifelseend = function(ast)
		for i, condition in ipairs(ast) do
			if (i == 1) then
				emit("if")
				recursivelyunparse(condition.condition)
				emit("then")
			else
				if condition.condition then
					emit("elseif")
					recursivelyunparse(condition.condition)
					emit("then")
				else
					emit("else")
				end
			end
			
			recursivelyunparse(condition.chunk)
		end
		emit("end")
	end,
		
	forend = function(ast)
		emit("for")
		recursivelyunparse(ast.var)
		emit("=")
		recursivelyunparse(ast.low)
		emit(",")
		recursivelyunparse(ast.high)
		if ast.step then
			emit(",")
			recursivelyunparse(ast.step)
		end
		emit("do")
		recursivelyunparse(ast.chunk)
		emit("end")
	end,
	
	forinend = function(ast)
		emit("for")
		recursivelyunparse(ast.var)
		emit("in")
		recursivelyunparse(ast.iterator)
		emit("do")
		recursivelyunparse(ast.chunk)
		emit("end")
	end,
	
	whiledoend = function(ast)
		emit("while")
		recursivelyunparse(ast.condition)
		emit("do")
		recursivelyunparse(ast.chunk)
		emit("end")
	end,
	
	repeatuntil = function(ast)
		emit("repeat")
		recursivelyunparse(ast.chunk)
		emit("until")
		recursivelyunparse(ast.condition)
	end,
	
	["return"] = function(ast)
		emit("return")
		if ast.value then
			recursivelyunparse(ast.value)
		end
	end,
	
	["break"] = function(ast)
		emit("break")
	end,
	
	functioncall = function(ast)
		recursivelyunparse(ast.value)
		emit("(")
		recursivelyunparse(ast.args)
		emit(")")
	end,
	
	functiondef = function(ast)
		if not ast.name then
			emit("(")
		end
		emit("function")
		if ast.name then
			recursivelyunparse(ast.name)
		end
		emit("(")
		recursivelyunparse(ast.args)
		emit(")")
		recursivelyunparse(ast.chunk)
		emit("end")
		if not ast.name then
			emit(")")
		end
	end,
	
	assignment = function(ast)
		recursivelyunparse(ast.left)
		if ast.right then
			emit("=")
			recursivelyunparse(ast.right)
		end
	end,
	
	list = function(ast)
		for i = 1, #ast do
			local exp = ast[i]
			if (i > 1) then
				emit(",")
			end
			recursivelyunparse(exp)
		end
	end,
	
	tableliteral = function(ast)
		emit("{")
		for i = 1, #ast do
			local entry = ast[i]
			if (i > 1) then
				emit(",")
			end
			if entry.key then
				if (entry.key.type == "identifier") then
					emit(entry.key)
				else
					emit("[")
					recursivelyunparse(entry.key)
					emit("]")
				end
				emit("=")
			end
			recursivelyunparse(entry.value)
		end
		emit("}")
	end,
	
	binop = function(ast)
		recursivelyunparsebinop(ast.left, ast.precedence)
		emit(ast.operator)
		recursivelyunparsebinop(ast.right, ast.precedence)
	end,
	
	unop = function(ast)
		emit(ast.operator)
		recursivelyunparsebinop(ast.value, ast.precedence)
	end,
	
	deref = function(ast)
		recursivelyunparse(ast.left)
		emit("[")
		recursivelyunparse(ast.right)
		emit("]")
	end,
	
	["local"] = function(ast)
		emit("local")
		recursivelyunparse(ast.value)
	end,
	
	leaf = function(ast)
		emit(ast.value)
	end,
	
	olua_methodcall = function(ast)
		emit("[object=")
		recursivelyunparse(ast.object)
		emit(" selector=")
		emit(ast.selector)
		emit(" args={")
		for i = 1, #ast.args do
			if (i > 1) then
				emit(",")
			end
			recursivelyunparse(ast.args[i])
		end
		emit("}]")
	end
}

recursivelyunparsebinop = function(ast, thisprecedence)
	if (ast.type == "binop") and
			(not ast.precedence or not thisprecedence or 
				(ast.precedence < thisprecedence)) then
		emit("(")
		recursivelyunparse(ast)
		emit(")")
	else
		recursivelyunparse(ast)
	end
end

recursivelyunparse = function(ast)
	local t = typetable[ast.type]
	if not t then
		error("Unknown AST node type "..ast.type)
	end
	
	t(ast)
end
 
local needsspace =
{
	string = true,
	number = true,
	identifier = true,
	keyword = true
}

local function unparser(self, ast)
	result = {}
	recursivelyunparse(ast)
	
	local line = 1
	local lasttype = nil
	local s = {}
	for _, t in ipairs(result) do
		if type(t) == "string" then
			local isstring = t:find("^[%w_]*$")
			if needsspace[lasttype] and isstring then
				s[#s+1] = " "
			end
						
			s[#s+1] = t
			if isstring then
				lasttype = "string"
			else
				lasttype = nil
			end
		else
			if t.line then
				while (line < t.line) do
					s[#s+1] = "\n"
					line = line + 1
					lasttype = nil
				end
			end
			
			if needsspace[lasttype] and needsspace[t.type] then
				s[#s+1] = " "
			end
						
			s[#s+1] = t.text
			
			lasttype = t.type
		end
	end
	
	return table.concat(s)
end

getmetatable(getfenv(1)).__call = unparser
