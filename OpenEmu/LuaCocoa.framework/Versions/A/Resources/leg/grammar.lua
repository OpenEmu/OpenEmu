--[=[
<%
  project.title = "grammar"
  project.description = "LPeg grammar manipulation"
  project.version = "0.1.2"
  project.date = _G.os.date'%B %d, %Y'
  project.modules = { 'grammar', 'parser', 'scanner' }
%>

# Description

This module defines a handful of operations which can be applied to 
[http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg] patterns and grammars in
general.

# Dependencies

* [http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg].

# Operations

## Piping

Pattern matching dissociates the notion of *matching* from the notion of 
*capturing*: matching checks if a given string follows a certain pattern, 
and capturing generates values according to the match made. This division 
allows interesting possibilities:

* different problems can be solved by applying different captures to the same grammar;
* captures may be defined separately;
* captures may be done on top of other captures.

Accounting for the first and second bullets, the grammar given in 
[parser.html parser] has no captures, enabling the user to reuse it to solve any 
problems that require a Lua grammar. One good example is documentation 
generation, described in a little more detail [#section_Example below].

The third bullet depicts a more interesting idea: a capture might take the 
result of another capture as input, doing a further transformation of the 
original data. This capture chaining, with the latter ones using the former's 
output as its input, is very similar to [http://en.wikipedia.org/wiki/Pipeline_%28Unix%29 Unix pipelines], 
so this mechanism was named **piping**.

## Completing

With piping, several levels of captures can be chained together up to the 
most appropriate for the task at hand. Yet some levels might require extra rules, and modifications to existing ones, to ensure proper matching. 

To avoid manual copying, the new grammar should redefine only the necessary 
rules, copying the rest from the older grammar. This action is dubbed 
**completing**.

## Applying

Once a new rule set is created and [#section_Completing completed], and 
all captures are correctly [#section_Piping piped], all that's left is 
to put them together, a process called **applying**. The result is a grammar ready for [http://www.inf.puc-rio.br/~roberto/lpeg.html#lpeg lpeg.P] 
consumption, whose pattern will return the intended result when a match is made.

## Example

Let's consider the problem of documenting a Lua module. In this case, comments 
must be captured before every function declaration when in the outermost scope:

``
-- -- the code to parse
subject = %[%[
--  -- Calculates the sum a+b. 
--  -- An extra line.
  function sum (a, b)
--  -- code
  end

--  -- f1: assume a variable assignment is not a proper declaration for an 
--  -- exported function
  f1 = function ()
--  -- code
  end

  while true do
--    -- this function is not in the outermost scope
    function aux() end
  end
  
  function something:other(a, ...)
--    -- a global function without comments
  end
%]%]
``

In the code above only `sum` and `something:other` should be documented, as `f1` isn't properly (by our standards) declared and `aux` is not in the outermost scope. 

By combining [http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg] and the modules [scanner.html scanner], [parser.html parser] and [grammar.html grammar], this specific problem can be solved as follows:

``
-- -- change only the initial rule and make no captures
patt = grammar.apply(parser.rules, scanner.COMMENT^-1 %* lpeg.V'GlobalFunction', nil)

-- -- transform the new grammar into a LPeg pattern
patt = lpeg.P(patt)

-- -- making a pattern that matches any Lua statement, also without captures
Stat = lpeg.P( grammar.apply(parser.rules, lpeg.V'Stat', nil) )

-- -- a pattern which matches function declarations and skips statements in
-- -- inner scopes or undesired tokens
patt = (patt + Stat + scanner.ANY)^0

-- -- matching a string
patt:match(subject)
``

These are the relevant rules in [parser.html#section_The_Grammar the grammar]:

``
GlobalFunction = 'function' %* FuncName %* FuncBody
FuncName     = ID %* ('.' %* ID)^0 %* (':' %* ID)^-1
FuncBody     = '(' %* (ParList + EPSILON) %* ')' %* Block %* 'end'
ParList      = NameList %* (',' %* '...')^-1
NameList     = ID %* (',' %* ID)^0
ID           = scanner.ID
EPSILON      = lpeg.P(true)
``

It may seem that `ParList + EPSILON` could be substituted for `ParList^-1` (optionally match `ParList`), but then no captures would be made for empty parameter lists, and `GlobalFunction` would get all strings matched by `FuncBody`. The `EPSILON` rule acts in this manner as a placeholder in the argument list, avoiding any argument list processing in the capture function.

Since no captures are being made, [http://www.inf.puc-rio.br/~roberto/lpeg.html#basic lpeg.match] doesn't return anything interesting. Here are some possible captures:

``
-- -- some interesting captures bundled up in a table. Note that the table keys
-- -- match the grammar rules we want to add captures to. Whatever rules aren't in
-- -- the rules table below will come from parser.rules .
captures = {
  %[1%] = function (...) -- the initial rule
    return '&lt;function&gt;'..table.concat{...}..'&lt;/function&gt;' 
  end,
  
  GlobalFunction = function (name, parlist)
    return '&lt;name&gt;'..name..'&lt;/name&gt;&lt;parlist&gt;'..(parlist or '')..'&lt;/parlist&gt;' 
  end,
  
  FuncName = grammar.C, -- capture the raw text
  ParList  = grammar.C, -- capture the raw text
  COMMENT  = scanner.comment2text, -- remove the comment trappings
}

-- -- spacing rule
local S = scanner.SPACE ^ 0

-- -- rules table
rules = {
  %[1%]     = ((lpeg.V'COMMENT' %*S) ^ 0) %*S%* lpeg.V'GlobalFunction',
  COMMENT = scanner.COMMENT,
}

-- -- building the new grammar and adding the captures
patt = lpeg.P( grammar.apply(parser.rules, rules, captures) )

-- -- a pattern that matches a sequence of patts and concatenates the results
patt = (patt + Stat + scanner.ANY)^0 / function(...) 
  return table.concat({...}, '\n\n') -- some line breaks for easier reading
end

-- -- finally, matching a string
print(patt:match(subject))
``

`FuncBody` needs no captures, as `Block` and all its non-terminals have none; it 
just needs to pass along any captures made by `ParList`. `NameList` and `ID` also have no captures, and the whole subject string is passed further.

The printed result is:
<pre class="example">
&lt;function&gt;Calculates the sum a+b. An extra line.&lt;name&gt;sum&lt;/name&gt;&lt;parlist&gt;a, b&lt;/parlist&gt;&lt;/function&gt;
<br/>
&lt;function&gt;&lt;name&gt;something:other&lt;/name&gt;&lt;parlist&gt;a, ...&lt;/parlist&gt;&lt;/function&gt;
</pre>
--]=]


-- $Id: grammar.lua,v 1.3 2007/11/26 18:41:51 hanjos Exp $

-- basic functions
local assert  = assert
local pairs   = pairs
local type    = type

-- imported modules
local lpeg = require 'lpeg'

-- imported functions
local P, V = lpeg.P, lpeg.V

-- module declaration
module 'leg.grammar'

--[[ 
Returns a pattern which matches any of the patterns received.

**Example:**
``
local g, s, m = require 'leg.grammar', require 'leg.scanner', require 'lpeg'

-- -- match numbers or operators, capture the numbers
print( (g.anyOf { '+', '-', '%*', '/', m.C(s.NUMBER) }):match '34.5@23 %* 56 / 45 - 45' )
-- --> prints 34.5
``

**Parameters:**
* `list`: a list of zero or more LPeg patterns or values which can be fed to [http://www.inf.puc-rio.br/~roberto/lpeg.html#lpeg lpeg.P].

**Returns:**
* a pattern which matches any of the patterns received.
--]]
function anyOf(list)
  local patt = P(false)
  
  for i = 1, #list, 1 do
    patt = P(list[i]) + patt
  end
  
  return patt
end

--[=[
Returns a pattern which matches a list of `patt`s, separated by `sep`.

**Example:** matching comma-separated values:
``
local g, m = require 'leg.grammar', require 'lpeg'

-- -- separator
local sep = m.P',' + m.P'\n'

-- -- element: anything but sep, capture it
local elem = m.C((1 - sep)^0)

-- -- pattern
local patt = g.listOf(elem, sep)

-- -- matching
print( patt:match %[%[a, b, 'christmas eve'
  d, evening; mate!
  f%]%])
-- --> prints out "a        b       'christmas eve'  d        evening; mate! f"
``

**Parameters:**
* `patt`: a LPeg pattern.
* `sep`: a LPeg pattern.

**Returns:**
* the following pattern: ``patt %* (sep %* patt)^0``
--]=]
function listOf(patt, sep)
  patt, sep = P(patt), P(sep)
  
  return patt * (sep * patt)^0
end


--[[ 
A capture function, made so that `patt / C` is equivalent to `m.C(patt)`. It's intended to be used in capture tables, such as those required by [#function_pipe pipe] and [#function_apply apply].
--]]
function C(...) return ... end

--[[ 
A capture function, made so that `patt / Ct` is equivalent to `m.Ct(patt)`. It's intended to be used in capture tables, such as those required by [#function_pipe pipe] and [#function_apply apply].
--]]
function Ct(...) return { ... } end

--[[
Creates a shallow copy of `grammar`.

**Parameters:**
* `grammar`: a regular table.

**Returns:**
* a newly created table, with `grammar`'s keys and values.
--]]
function copy(grammar)
	local newt = {}
  
	for k, v in pairs(grammar) do
		newt[k] = v
	end
  
	return newt
end

--[[
[#section_Completing Completes] `dest` with `orig`.

**Parameters:**
* `dest`: the new grammar. Must be a table.
* `orig`: the original grammar. Must be a table.

**Returns:**
* `dest`, with new rules inherited from `orig`.
--]]
function complete (dest, orig)
	for rule, patt in pairs(orig) do
		if not dest[rule] then
			dest[rule] = patt
		end
	end
  
	return dest
end

--[[
[#section_Piping Pipes] the captures in `orig` to the ones in `dest`.

`dest` and `orig` should be tables, with each key storing a capture function. Each capture in `dest` will be altered to use the results for the matching one in `orig` as input, using function composition. Should `orig` possess keys not in `dest`, `dest` will copy them.

**Parameters:**
* `dest`: a capture table.
* `orig`: a capture table.

**Returns:**
* `dest`, suitably modified.
--]]
function pipe (dest, orig)
	for k, vorig in pairs(orig) do
		local vdest = dest[k]
		if vdest then
			dest[k] = function(...) return vdest(vorig(...)) end
		else
			dest[k] = vorig
		end
	end
	
	return dest
end

--[[
[#section_Completing Completes] `rules` with `grammar` and then [#Applying applies] `captures`.     

`rules` can either be:
* a single pattern, which is taken to be the new initial rule, 
* a possibly incomplete LPeg grammar, as per [#function_complete complete], or 
* `nil`, which means no new rules are added.

`captures` can either be:
* a capture table, as per [#function_pipe pipe], or
* `nil`, which means no captures are applied.

**Parameters:**
* `grammar`: the old grammar. It stays unmodified.
* `rules`: optional, the new rules. 
* `captures`: optional, the final capture table.

**Returns:**
* `rules`, suitably augmented by `grammar` and `captures`.
--]]
function apply (grammar, rules, captures)
  if rules == nil then
    rules = {}
  elseif type(rules) ~= 'table' then
    rules = { rules }
  end
  
  complete(rules, grammar)
  
  if type(grammar[1]) == 'string' then
    rules[1] = lpeg.V(grammar[1])
  end
	
	if captures ~= nil then
		assert(type(captures) == 'table', 'captures must be a table')
    
		for rule, cap in pairs(captures) do
			rules[rule] = rules[rule] / cap
		end
	end
  
	return rules
end

