--[=[
<%
  project.title = "scanner"
  project.description = "Lua 5.1 lexical patterns"
  project.version = "0.1.2"
  project.date = _G.os.date "%B %d, %Y"
  project.modules = { 'grammar', 'parser', 'scanner' }
%>

# Description

This module exports several Lua lexical patterns, all implemented in [http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg]. None of them have captures.

# Dependencies

* [http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg]

# Example

The following example lists all the tokens in a Lua script:

``
local lpeg = require 'lpeg'
local scanner = require 'leg.scanner'

-- -- this pattern captures all tokens, ignores spaces and comments,
-- -- and matches with end of file, giving an error otherwise
patt = (lpeg.C(scanner.TOKEN) + scanner.SPACE + scanner.COMMENT)^0
     %* (scanner.EOF + scanner.error'invalid character')
patt = lpeg.Ct(patt)

-- -- opens the file passed as parameter and tries to match with patt
f = assert(io.open(arg%[1%]))

-- -- a table storing all the tokens
ALL = patt:match(f:read'%*a')
f:close()

-- -- dumps all tokens on screen
for _, tk in ipairs(ALL) do
    print(tk)
end
``
--]=]


-- $Id: scanner.lua,v 1.2 2007/11/26 18:41:51 hanjos Exp $ 

-- basic modules
local _G      = _G
local string  = string

-- basic functions
local assert    = assert
local pairs     = pairs
local print     = print
local require   = require
local tonumber  = tonumber
local type      = type

-- imported modules
local m     = require 'lpeg'

-- module declaration
module 'leg.scanner'

------------------------------ PATTERNS ---------------------------------------

-- digit pattern
local N = m.R'09'

-- alphanumeric pattern
local AZ = m.R('__','az','AZ','\127\255')     -- llex.c uses isalpha()

-- punctuation pattern. Used only in text2string
local PUNCTUATION = m.S'!@$&|?~`´'

------------------------------ FUNCTIONS --------------------------------------

-- Counts the number of lines (separated by '\n') in subject.
local function lines (subject)
	local inc = function (l) return l + 1 end
	local L = m.Ca( m.Cc(1) * (m.P'\n'/inc + m.P(1)) ^0 )
  
	return L:match(subject)
end

--[[
Returns a function which throws lexical errors.

**Parameters:**
* `msg`: the message to be concatenated to the error message.

**Returns:**
* A function built to be used as a [http://www.inf.puc-rio.br/~roberto/lpeg.html#lpeg LPeg pattern], which will throw an error when matched.

Usage example:
``
patt = intended_pattern^0 %* (EOF + error'character after EOF')
``

It may also be used as a normal function:
``
function (subject, i)
  if bad_condition then
    error'bad condition'(subject, i)
  end
end
``
--]]
function error (msg)
	return function (subject, i)
		local line = lines(string.sub(subject,1,i))
    
    _G.error('Lexical error in line '..line..', near "'
      ..(subject:sub(i-10,i)):gsub('\n','EOL')..'": '..msg, 0)
	end
end

--[[
Strips all prefixing `--` and enclosing `--%[=%*%[` from comment tokens.

**Parameters:**
* `comment`: the comment to strip.

**Returns:**
* the text without comment marking syntax.
--]]
function comment2text (comment) -- TEMP: LPeg could be used here
	local ret, i, brackets = comment:find('^%-%-%[(=*)%[', i)
	
  if ret then
		comment = comment:gsub('^%-%-%['..brackets..'%[', '')  -- removes "--[===["
		comment = comment:gsub('%]'..brackets..'%]$', '')      -- removes "]===]"
		comment = '\n' .. comment
		comment = comment:gsub('\n\n', '\n%-%-\n')             -- adjust empty lines
		comment = comment:gsub('\n%s*%-%-+ ?', '\n' )          -- removes "--+ " prefix from lines
		comment = comment:gsub('^\n\n?', '')                   -- removes empty prefix lines
		comment = comment:gsub('\n$', '')                      -- removes empty sufix lines
	else
		comment = comment:gsub('^%-%-+%s*', '')
	end
  
	return comment
end

--[[
Encloses the text with comment markers.

**Parameters:**
* `text`: the text to comment.

**Returns:**
* the text with comment marking syntax.
--]]
function text2comment (text)
	-- finds a pattern anywhere in the subject, ripped from LPeg's home page
  local function anywhere(patt)
    return m.P { m.P(patt) + 1 * m.V(1) }
  end
  
  -- searching for the largest [(=)*[ in the text
  local max = -1
  
  local updateMax = function (c) if max < #c then max = #c end end
  local openPatt = m.P'[' * m.C((m.P'=')^0) * m.P'[' / updateMax
  local closePatt = m.P']' * m.C((m.P'=')^0) * m.P']' / updateMax
  
  anywhere(openPatt):match(text)
  anywhere(closePatt):match(text)
  
  -- enclosing text with --[(=)^(max+1)[ and --](=)^(max + 1)]
  local equals = string.rep('=', max + 1)
	return '--['..equals..'[\n'..text..'--]'..equals..']'
end

-- used for escape processing in string2text
local escapeTable = {
  ['\\n'] = '\n',
  ['\\t'] = '\t',
  ['\\r'] = '\r',
  ['\\v'] = '\v',
  ['\\a'] = '\a',
  ['\\b'] = '\b',
  ['\\f'] = '\f',
  ['\\"'] = '"',
  ["\\'"] = "'",
  ['\\\\'] = '\\',
}

-- used for escape processing in text2string
local reverseEscapeTable = {}

for k, v in pairs(escapeTable) do
  reverseEscapeTable[v] = k
  reverseEscapeTable[string.byte(v)] = k
end

--[=[
Strips all enclosing `'`, `"`, and `%[=%*%[` from string tokens, and processes escape characters.

**Parameters:**
* `str`: the string to strip.

**Returns:**
* the text without string enclosers.
--]=]
function string2text(str)
  local escapeNum = m.C(N^-3) / tonumber
  local escapePatt = (
      (m.P'\\' * m.S[[ntrvabf'"\\]]) / escapeTable
    + (m.P'\\' * escapeNum) / string.char
  )
  
  local openDQuote, openQuote = m.P'"' / '', m.P"'" / ''
  local closeDQuote, closeQuote = openDQuote, openQuote
  
  local start, l = "[" * m.P"="^0 * "[", nil
  local longstring = #(m.P'[' * m.S'[=') * m.P(function (s, i)
    l = start:match(s, i)
    if not l then return nil end
    
    local p = m.P("]"..string.rep("=", l - i - 2).."]")
    p = (1 - p)^0 * p
    
    return p:match(s, l)
  end)
  
  
	local patt = m.Cs(
      (openDQuote * ((escapePatt + 1) - closeDQuote)^0 * closeDQuote)
    + (openQuote * ((escapePatt + 1) - closeQuote)^0 * closeQuote)
    + longstring / function (c) return string.sub(c, l, -l) end
  )
  
  return patt:match(str)
end

--[[
Transforms a text into a syntactically valid Lua string. Similar to `string.format` with the `'%%q'` option, but inserting escape numbers and escape codes where applicable.

**Parameters**
* `text`: a string containing the text.

**Returns:**
* a string, similar to string.format with option `'%%q'`.
--]]
function text2string(text)
  local function reverseEscape(char)
    local c = reverseEscapeTable[char]
    
    if c then 
      return c
    elseif (AZ + N + SPACE + SYMBOL + PUNCTUATION):match(char) then
      return char
    else
      return '\\'..string.byte(char)
    end
  end
  
  local escapePatt = m.P(1) / reverseEscape
  local patt = m.Cs(escapePatt^0)
  
  return '"'..patt:match(text)..'"'
end

------------------------------ TOKENS -----------------------------------------

--[[
A table with Lua keyword-matching patterns, with the keywords in uppercase 
as keys.

Examples: `keywords.WHILE`, `keywords%['ELSEIF'%]`.
--]]
keywords = {}

--[[
A table with Lua symbol-matching patterns, with the symbols themselves as 
keys.

Examples: `symbols%['{'%]`, `symbols%['+'%]`.
--]]
symbols = {}

-- Transforms strings into literal patterns of the same name.
-- If "minus" is passed, each pattern is concatenated to it.
local apply = function (dst, src, minus)
	local ret = m.P(false)
	for _, v in _G.ipairs(src) do
		local UP = string.upper(v)
		dst[UP] = m.P(v)
		if minus then
			dst[UP] = dst[UP] * minus
		end
		ret = dst[UP] + ret
	end
	return ret
end

-- A pattern which matches any Lua keyword.
-- <% exp = 'LPeg pattern' %>
KEYWORD = apply (keywords, {
	'and',      'break',    'do',       'else',     'elseif',
	'end',      'false',    'for',      'function', 'if',
	'in',       'local',    'nil',      'not',      'or',
	'repeat',   'return',   'then',     'true',     'until',    'while',
}, -(N + AZ) )

-- A pattern which matches any Lua symbol.
-- <% exp = 'LPeg pattern' %>
SYMBOL = apply (symbols, {
	'+',     '-',     '*',     '/',     '%',     '^',     '#',
	'==',    '~=',    '<=',    '>=',    '<',     '>',     '=',
	'(',     ')',     '{',     '}',     '[',     ']',
	';',     ':',     ',',     '.',     '..',    '...',
})

-- special cases (needs lookahead)
symbols['-']  = symbols['-']  - '--'
symbols['<']  = symbols['<']  - symbols['<=']
symbols['>']  = symbols['>']  - symbols['>=']
symbols['=']  = symbols['=']  - symbols['==']
symbols['[']  = symbols['[']  - '[' * m.S'[='
symbols['.']  = symbols['.']  - (symbols['..'] + N)
symbols['..'] = symbols['..'] - symbols['...']

SYMBOL = m.P(false)
for _, v in _G.pairs(symbols) do
	SYMBOL = SYMBOL + v
end

-- Matches any Lua identifier.
-- <% exp = 'LPeg pattern' %>
IDENTIFIER = AZ * (AZ+N)^0 - KEYWORD

-- tries to implement the same read_numeral() as in llex.c
local number = function (subject, i)
	--               [.\d]+     .. ( [eE]  ..   [+-]? )?    .. isalnum()*
	local patt = (m.P'.' + N)^1 * (m.S'eE' * m.S'+-'^-1)^-1 * (N+AZ)^0
	patt = patt / function(num)
    if not _G.tonumber(num) then 
      error('Malformed number: '.._G.tostring(num))(subject,i) 
    end 
  end
  
	return m.match(patt, subject, i)
end

-- Matches any Lua number.
-- <% exp = 'LPeg pattern' %>
NUMBER = #(N + (m.P'.' * N)) * number

-- LONG BRACKETS
local long_brackets = #(m.P'[' * m.P'='^0 * m.P'[') * function (subject, i1)
	local level = _G.assert( subject:match('^%[(=*)%[', i1) )
	local _, i2 = subject:find(']'..level..']', i1, true)  -- true = plain "find substring"
	return (i2 and (i2+1)) or error('unfinished long brackets')(subject, i1)
end

-- Matches any Lua string.
-- <% exp = 'LPeg pattern' %>
STRING = nil
do
	-- TEMP ddd limites: local ddd  = m.P'\\' * (N^3 + N^2 + N^1)
	          --  OPEN  and  (   (\?)    or ( anything not CLOSE or \n )^0 ) and    CLOSE
	local Str1 = m.P'"' * ( (m.P'\\' * 1) + (1 - (m.S'"\n\r\f')) )^0 * (m.P'"' + error'unfinished string')
	local Str2 = m.P"'" * ( (m.P'\\' * 1) + (1 - (m.S"'\n\r\f")) )^0 * (m.P"'" + error'unfinished string')
	local Str3 = long_brackets
	STRING = Str1 + Str2 + Str3
end

local multi  = m.P'--' * long_brackets
local single = m.P'--' * (1 - m.P'\n')^0

-- Matches any type of comment.
-- <% exp = 'LPeg pattern' %>
COMMENT = multi + single -- multi must be the first ( --[ is a one line comment )

-------------------------------------------------------------------------------
------------------------------ USEFUL PATTERNS --------------------------------
-------------------------------------------------------------------------------

-- Matches any space character.
-- <% exp = 'LPeg pattern' %>
SPACE = m.S'\n \t\r\f'

-- Matches everything ignored by the parser.
-- <% exp = 'LPeg pattern' %>
IGNORED = (SPACE + COMMENT)^0

-- Matches any Lua [#variable_IDENTIFIER identifier], [#variable_KEYWORD keyword], [#variable_SYMBOL symbol], [#variable_NUMBER number] or [#variable_STRING string].
-- <% exp = 'LPeg pattern' %>
TOKEN = IDENTIFIER + KEYWORD + SYMBOL + NUMBER + STRING

-- Matches any [#variable_TOKEN token], [#variable_COMMENT comment] or [#variable_SPACE space].
-- <% exp = 'LPeg pattern' %>
ANY = TOKEN + COMMENT + SPACE -- TEMP: + error'invalid character'

-- Matches the beginning of a file.
-- <% exp = 'LPeg pattern' %>
BOF = m.P(function(s,i) return (i==1) and i end)

-- Matches the end of a file.
-- <% exp = 'LPeg pattern' %>
EOF = m.P(-1)

-- Matches UNIX's shebang (e.g. `#!/usr/bin/lua`).
-- <% exp = 'LPeg pattern' %>
BANG = BOF * m.P'#!' * (m.P(1)-'\n')^0 * '\n'
