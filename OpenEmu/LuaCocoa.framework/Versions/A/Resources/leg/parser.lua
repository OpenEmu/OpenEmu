--[[
<%
  project.title = "parser"
  project.description = "Lua 5.1 parser"
  project.version = "0.1.2"
  project.date = _G.os.date'%B %d, %Y'
  project.modules = { 'grammar', 'parser', 'scanner' }
%>

# Description

Pairing with [scanner.html scanner], this module exports Lua 5.1's syntactic rules as a grammar.

# Dependencies

* [http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg]; 
* [grammar.html grammar]; and 
* [scanner.html scanner].

# The Grammar

The [#variable_rules rules] variable implements the official [http://www.lua.org/manual/5.1/manual.html#8 Lua 5.1 grammar]. It includes all keyword and symbol rules in [scanner.html scanner], as well as the `CHUNK` rule, which matches a complete Lua source file.

[#variable_rules rules] is a table with [http://www.inf.puc-rio.br/~roberto/lpeg.html#grammar open references], not yet a LPeg pattern; to create a pattern, it must be given to `[http://www.inf.puc-rio.br/~roberto/lpeg.html#lpeg lpeg.P]`. This is done to enable users to modify the grammar to suit their particular needs. [grammar.html grammar] provides a small API for this purpose.

The code below shows the Lua 5.1 grammar in LPeg, minus spacing issues.

The following convention is used for rule names:
* **TOKENRULE**: token rules (which represent terminals) are in upper case when applicable (ex. `+, WHILE, NIL, ..., THEN, {, ==`).
* **GrammarRule**: the main grammar rules (non-terminals): Examples are `Chunk`, `FuncName`, `BinOp`, and `TableConstructor`.
* **_GrammarRule**: subdivisions of the main rules, introduced to ease captures. Examples are `_SimpleExp`, `_PrefixExpParens` and `_FieldExp`.
* **METARULE**: grammar rules with a special semantic meaning, to be used for capturing in later modules, like `BOF`, `EOF` and `EPSILON`.

``
rules = {
--   -- See peculiarities below
    IGNORED  = scanner.IGNORED -- used as spacing, not depicted below
    EPSILON = lpeg.P(true)
    EOF     = scanner.EOF -- end of file
    BOF     = scanner.BOF -- beginning of file
    Name    = ID

--   -- Default initial rule
    %[1%]     = CHUNK
    CHUNK   = scanner.BANG^-1 %* Block

    Chunk   = (Stat %* ';'^-1)^0 %* (LastStat %* ';'^-1)^-1
    Block   = Chunk

--   -- STATEMENTS
    Stat          = Assign + FunctionCall + Do + While + Repeat + If
                  + NumericFor + GenericFor + GlobalFunction + LocalFunction
                  + LocalAssign
    Assign        = VarList %* '=' %* ExpList
    Do            = 'do' %* Block %* 'end'
    While         = 'while' %* Exp %* 'do' %* Block %* 'end'
    Repeat        = 'repeat' %* Block %* 'until' %* Exp
    If            = 'if' %* Exp %* 'then' %* Block
                      %* ('elseif' %* Exp %* 'then' %* Block)^0
                      %* (('else' %* Block) + EPSILON)
                      %* 'end'
    NumericFor    = 'for' %* Name %* '='
                      %* Exp %* ',' %* Exp %* ((',' %* Exp) + EPSILON)
                      %* 'do' %* Block %* 'end'
    GenericFor    = 'for' %* NameList %* 'in' %* ExpList %* 'do' %* Block %* 'end'
    GlobalFunction = 'function' %* FuncName %* FuncBody
    LocalFunction = 'local' %* 'function' %* Name %* FuncBody
    LocalAssign   = 'local' %* NameList %* ('=' %* ExpList)^-1
    LastStat      = 'return' %* ExpList^-1
                  + 'break'

--   -- LISTS
    VarList  = Var %* (',' %* Var)^0
    NameList = Name %* (',' %* Name)^0
    ExpList  = Exp %* (',' %* Exp)^0

--   -- EXPRESSIONS
    Exp          = _SimpleExp %* (BinOp %* _SimpleExp)^0
    _SimpleExp   = 'nil' + 'false' + 'true' + Number + String + '...' + Function
                 + _PrefixExp + TableConstructor + (UnOp %* _SimpleExp)
    _PrefixExp   = ( Name                  a Var
                   + _PrefixExpParens      only an expression
                   ) %* (
                       _PrefixExpSquare    a Var
                     + _PrefixExpDot       a Var
                     + _PrefixExpArgs      a FunctionCall
                     + _PrefixExpColon     a FunctionCall
                   ) ^ 0

--   -- Extra rules for semantic actions:
    _PrefixExpParens = '(' %* Exp %* ')'
    _PrefixExpSquare = '[' %* Exp %* ']'
    _PrefixExpDot    = '.' %* ID
    _PrefixExpArgs   = Args
    _PrefixExpColon  = ':' %* ID %* _PrefixExpArgs

--   -- These rules use an internal trick to be distingished from _PrefixExp
    Var              = _PrefixExp
    FunctionCall     = _PrefixExp

--   -- FUNCTIONS
    Function     = 'function' %* FuncBody
    FuncBody     = '(' %* (ParList+EPSILON) %* ')' %* Block %* 'end'
    FuncName     = Name %* _PrefixExpDot^0 %* ((':' %* ID)+EPSILON)
    Args         = '(' %* (ExpList+EPSILON) %* ')'
                 + TableConstructor + String
    ParList      = NameList %* (',' %* '...')^-1
                 + '...'

--   -- TABLES
    TableConstructor = '{' %* (FieldList+EPSILON) %* '}'
    FieldList        = Field %* (FieldSep %* Field)^0 %* FieldSep^-1
    FieldSep         = ',' + ';'

--   -- Extra rules for semantic actions:
    _FieldSquare     = '[' %* Exp %* ']' %* '=' %* Exp
    _FieldID         = ID %* '=' %* Exp
    _FieldExp        = Exp

--   -- OPERATORS
    BinOp    = '+' + '-' + '%*' + '/' + '^' + '%' + '..'
             + '&lt;' + '&lt;=' + '&gt;' + '&gt;=' + '==' + '~='
             + 'and' + 'or'
    UnOp     = '-' + 'not' + '#'

--   -- ...plus scanner's keywords and symbols
}
``

The implementation has certain peculiarities that merit clarification:

* Spacing is matched only between two tokens in a rule, never at the beginning or the end of a rule.
 
* `EPSILON` matches the empty string, which means that it always succeeds without consuming input. Although `rule + EPSILON` can be changed to `rule^-1` without any loss of syntactic power, `EPSILON` was introduced in the parser due to it's usefulness as a placeholder for captures.

* `BOF` and `EOF` are rules used to mark the bounds of a parsing match, and are useful for semantic actions.

* `Name` versus `ID`: the official Lua grammar doesn't distinguish between them, as their syntax is exactly the same (Lua identifiers). But semantically `Name` is a variable identifier, and `ID` is used with different meanings in `_FieldID`, `FuncName`, `_PrefixExpColon` and `_PrefixExpDot`.

* In Lua's [http://www.lua.org/manual/5.1/manual.html#8 original extended BNF grammar], `Var` and `FunctionCall` are defined using left recursion, which is unavailable in PEGs. In this implementation, the problem was solved by modifying the PEG rules to eliminate the left recursion, and by setting some markers (with some LPeg chicanery) to ensure the proper pattern is being used.
--]]

-- $Id: parser.lua,v 1.3 2007/11/26 18:41:51 hanjos Exp $

-- basic modules
local _G     = _G
local table  = table
local string = string

-- basic functions
local error   = error
local require = require

-- imported modules
local m       = require 'lpeg'

local scanner = require 'leg.scanner'
local grammar = require 'leg.grammar'

-- module declaration
module 'leg.parser'

-- Searches for the last substring in s which matches pattern
local function rfind(s, pattern, init, finish)
  init = init or #s
  finish = finish or 1
  
  for i = init, finish, -1 do
    local lfind, rfind = string.find(s, pattern, i)
    
    if lfind and rfind then
      return lfind, rfind
    end
  end
  
  return nil
end

-- Counts the number of lines (separated by *'\n'*) in `subject`.
local function lines (subject)
	local inc = function (l) return l + 1 end
	local L = m.Ca( m.Cc(1) * (m.P'\n'/inc + m.P(1)) ^0 )

	return L:match(subject)
end

-- a little LPeg trick to enable a limited form of left recursion
local prefix 

-- sets a prefix used to distinguish between Var and FunctionCall
local setPrefix = function (p)
	return function (_,i)
		prefix = p
		return i
	end
end

-- matches Var and FunctionCall from _PrefixExp
local matchPrefix = function (p)
	return function (_, i)
		return (prefix == p) and i
	end
end

-- throws an error if the grammar rule `rule` doesn't match
-- `desc` is there for a slightly better error message
local function CHECK(rule, desc)
  patt, desc = m.V(rule), desc or 'chunk'
  
  return patt + m.P(function (s, i)
    local line = lines(s:sub(1, i))
    local vicinity = s:sub(i-5, i+5):gsub("\n", "<EOL>")
    
    error('Malformed '..desc..' in line '..line..', near "'..vicinity..'": a "'..rule:lower()..'" is missing!', 0)
  end)

end

-- this will be used a lot below
local S, listOf, anyOf = m.V'IGNORED', grammar.listOf, grammar.anyOf

--[[ 
A table holding the Lua 5.1 grammar. See [#section_The_Grammar The Grammar] for an extended explanation.
--]]
-- <% exp = 'table' %>
rules = {
	  IGNORED = scanner.IGNORED  -- seen as S below
	, EPSILON = m.P(true)
	, EOF     = scanner.EOF
  , BOF     = scanner.BOF
  , NUMBER  = scanner.NUMBER
	, ID      = scanner.IDENTIFIER
	, STRING  = scanner.STRING
	, Name    = m.V'ID'

  -- CHUNKS
	, [1]     = m.V'CHUNK'
	, CHUNK   = scanner.BANG^-1 * m.V'Block'

	, Chunk   = (S* m.V'Stat' *S* m.V';'^-1)^0 
            *S* (m.V'LastStat' *S* m.V';'^-1)^-1
	, Block   = m.V'Chunk'

	-- STATEMENTS
	, Stat        = m.V'Assign' + m.V'FunctionCall' + m.V'Do' 
                  + m.V'While' + m.V'Repeat' + m.V'If'
                  + m.V'NumericFor' + m.V'GenericFor' 
                  + m.V'GlobalFunction' + m.V'LocalFunction' 
                  + m.V'LocalAssign'
	, Assign      = m.V'VarList' *S* m.V'=' *S* m.V'ExpList'
	, Do          = m.V'DO' *S* m.V'Block' *S* CHECK('END', 'do block')
	, While       = m.V'WHILE' *S* m.V'Exp' *S* CHECK('DO', 'while loop')
                  *S* m.V'Block' *S* CHECK('END', 'while loop')
	, Repeat      = m.V'REPEAT' *S* m.V'Block' 
                  *S* CHECK('UNTIL', 'repeat loop') *S* m.V'Exp'
	, If          = m.V'IF' *S* m.V'Exp' *S* CHECK('THEN', 'then block') 
                  *S* m.V'Block' * (S* m.V'ELSEIF' *S* m.V'Exp' 
                  *S* CHECK('THEN', 'elseif block') *S* m.V'Block')^0
                  * ((S* m.V'ELSE' * m.V'Block') + m.V'EPSILON')
                  * S* CHECK('END', 'if statement')
  , NumericFor  = m.V'FOR' *S* m.V'Name' *S* m.V'=' *S* m.V'Exp' 
                  *S* m.V',' *S* m.V'Exp' 
                  *S* ((m.V',' *S* m.V'Exp') + m.V'EPSILON')
                  *S* CHECK('DO', 'numeric for loop') *S* m.V'Block' 
                  *S* CHECK('END', 'numeric for loop')
	, GenericFor    = m.V'FOR' *S* m.V'NameList' *S* m.V'IN' 
                      *S* m.V'ExpList' *S* CHECK('DO', 'generic for loop') *S* m.V'Block' *S* CHECK('END', 'generic for loop')
	, GlobalFunction = m.V'FUNCTION' *S* m.V'FuncName' *S* m.V'FuncBody'
	, LocalFunction = m.V'LOCAL' *S* m.V'FUNCTION' *S* m.V'Name' 
                      *S* m.V'FuncBody'
	, LocalAssign   = m.V'LOCAL' *S* m.V'NameList' 
                    * (S* m.V'=' *S* m.V'ExpList')^-1
	, LastStat      = m.V'RETURN' * (S* m.V'ExpList')^-1
                    + m.V'BREAK'

	-- LISTS
	--, VarList  = m.V'Var' * (S* m.V',' *S* m.V'Var')^0
	--, NameList = m.V'Name' * (S* m.V',' *S* m.V'Name')^0
	--, ExpList  = m.V'Exp' * (S* m.V',' *S* m.V'Exp')^0
  , VarList   = listOf(m.V'Var' , S* m.V',' *S)
  , NameList  = listOf(m.V'Name', S* m.V',' *S)
  , ExpList   = listOf(m.V'Exp' , S* m.V',' *S)

	-- EXPRESSIONS
	, Exp             = m.V'_SimpleExp' * (S* m.V'BinOp' *S* m.V'_SimpleExp')^0
	, _SimpleExp      = m.V'NIL' + m.V'FALSE' + m.V'TRUE' + m.V'NUMBER' 
                    + m.V'STRING' + m.V'...' + m.V'Function' + m.V'_PrefixExp' 
                    + m.V'TableConstructor' + (m.V'UnOp' *S* m.V'_SimpleExp')
	, _PrefixExp      = ( m.V'Name'               * setPrefix'Var'  -- Var
	                    + m.V'_PrefixExpParens'   * setPrefix(nil)) -- removes last prefix
	                    * (S* (
	                        m.V'_PrefixExpSquare' * setPrefix'Var'  -- Var
	                      + m.V'_PrefixExpDot'    * setPrefix'Var'  -- Var
	                      + m.V'_PrefixExpArgs'   * setPrefix'Call' -- FunctionCall
	                      + m.V'_PrefixExpColon'  * setPrefix'Call' -- FunctionCall
	                    )) ^ 0
	, _PrefixExpParens = m.V'(' *S* m.V'Exp' *S* CHECK(')', 'parenthesized expression')
	, _PrefixExpSquare = m.V'[' *S* m.V'Exp' *S* CHECK(']', 'index field')
	, _PrefixExpDot    = m.V'.' *S* m.V'ID'
	, _PrefixExpArgs   = m.V'Args'
	, _PrefixExpColon  = m.V':' *S* m.V'ID' *S* m.V'_PrefixExpArgs'

  -- solving the left recursion problem
	, Var          = m.V'_PrefixExp' * matchPrefix'Var'
	, FunctionCall = m.V'_PrefixExp' * matchPrefix'Call'

	-- FUNCTIONS
	, Function = m.V'FUNCTION' *S* m.V'FuncBody'
	, FuncBody = m.V'(' *S* (m.V'ParList'+m.V'EPSILON') *S* CHECK(')', 'parameter list')
             *S* m.V'Block' *S* CHECK('END', 'function body')
	, FuncName = m.V'Name' * (S* m.V'_PrefixExpDot')^0 
             * ((S* m.V':' *S* m.V'ID') + m.V'EPSILON')
	, Args     = m.V'(' *S* (m.V'ExpList'+m.V'EPSILON') *S* CHECK(')', 'argument list')
	           + m.V'TableConstructor' + m.V'STRING'
	, ParList  = m.V'NameList' * (S* m.V',' *S* m.V'...')^-1
	           + m.V'...'

	-- TABLES
	, TableConstructor = m.V'{' *S* (m.V'FieldList'+m.V'EPSILON') *S* CHECK('}', 'table constructor')
	, FieldList        = m.V'Field' * (S* m.V'FieldSep' *S* m.V'Field')^0 
                     * (S* m.V'FieldSep')^-1
	, Field            = m.V'_FieldSquare' + m.V'_FieldID' + m.V'_FieldExp'
	, _FieldSquare     = m.V'[' *S* m.V'Exp' *S* CHECK(']', 'index field') *S* CHECK('=', 'field assignment') *S* m.V'Exp'
	, _FieldID         = m.V'ID' *S* m.V'=' *S* m.V'Exp'
	, _FieldExp        = m.V'Exp'
	                   
	, FieldSep         = m.V',' + m.V';'

	-- OPERATORS
	, BinOp    = m.V'+'   + m.V'-'  + m.V'*' + m.V'/'  + m.V'^'  + m.V'%'  
             + m.V'..'  + m.V'<'  + m.V'<=' + m.V'>' + m.V'>=' + m.V'==' 
             + m.V'~='  + m.V'AND' + m.V'OR'
	, UnOp     = m.V'-' + m.V'NOT' + m.V'#'
}

-- puts all the keywords and symbols to the grammar
grammar.complete(rules, scanner.keywords)
grammar.complete(rules, scanner.symbols)

--[[
Checks if `input` is valid Lua source code.

**Parameters:**
* `input`: a string containing Lua source code.

**Returns:**
* `true`, if `input` is valid Lua source code, or `false` and an error message if the matching fails.
--]]
function check(input)
  local builder = m.P(rules)
  local result = builder:match(input)
  
  if result ~= #input + 1 then -- failure, build the error message
    local init, _ = rfind(input, '\n*', result - 1) 
    local _, finish = string.find(input, '\n*', result + 1)
    
    init = init or 0
    finish = finish or #input
    
    local line = lines(input:sub(1, result))
    local vicinity = input:sub(init + 1, finish)
    
    return false, 'Syntax error at line '..line..', near "'..vicinity..'"'
  end
  
  return true
end

--[[
Uses [grammar.html#function_apply grammar.apply] to return a new grammar, with `captures` and extra rules. [#variable_rules rules] stays unmodified.

**Parameters:**
* `extraRules`: optional, the new and modified rules. See [grammar.html#function_apply grammar.apply] for the accepted format.
* `captures`: optional, the desired captures. See [grammar.html#function_apply grammar.apply] for the accepted format.

**Returns:**
* the extended grammar.
--]]
function apply(extraRules, captures)
  return grammar.apply(rules, extraRules, captures)
end
