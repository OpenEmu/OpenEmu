--[[

= ABOUT
This module uses Roberto Ierusalimschy's powerful new pattern matching library
LPeg[1] to tokenize Lua source-code in to a table of tokens. I think it handles
all of Lua's syntax, but if you find anything missing I would appreciate a
xolox aatt home ddoott nl. This lexer is based on the BNF[2] from the Lua manual.

= USAGE
I've saved my copy of this module under [$LUA_PATH/lexers/lua.lua] which means
I can use it like in the following interactive prompt:

   Lua 5.1.1  Copyright (C) 1994-2006 Lua.org, PUC-Rio
   > require 'lexers.lua'
   > tokens = lexers.lua [=[
   >> 42 or 0
   >> -- some Lua source-code in a string]=]
   > = tokens
   table: 00422E40
   > lexers.lua.print(tokens)
   line 1, number: `42`
   line 1, whitespace: ` `
   line 1, keyword: `or`
   line 1, whitespace: ` `
   line 1, number: `0`
   line 1, whitespace: `
   `
   line 2, comment: `-- some Lua source-code in a string`
   total of 7 tokens, 2 lines

The returned table [tokens] looks like this:

{
   -- type       , text, line
   { 'number'    , '42', 1 },
   { 'whitespace', ' ' , 1 },
   { 'keyword'   , 'or', 1 },
   { 'whitespace', ' ' , 1 },
   { 'number'    , '0' , 1 },
   { 'whitespace', '\n', 1 },
   { 'comment'   , '-- some Lua source-code in a string', 2 },
}

= CREDITS
Written by Peter Odding, 2007/04/04

= THANKS TO
- the Lua authors for a wonderful language;
- Roberto for LPeg;
- caffeine for keeping me awake :)

= LICENSE
Shamelessly ripped from the SQLite[3] project:

   The author disclaims copyright to this source code.  In place of a legal
   notice, here is a blessing:

      May you do good and not evil.
      May you find forgiveness for yourself and forgive others.
      May you share freely, never taking more than you give.

[1] http://www.inf.puc-rio.br/~roberto/lpeg.html
[2] http://lua.org/manual/5.1/manual.html#8
[3] http://sqlite.org

--]]

-- since this module is intended to be loaded with require() we receive the
-- name used to load us in ... and pass it on to module()
module(..., package.seeall)

-- written for LPeg .5, by the way
local lpeg = require 'lpeg'
local P, R, S, C, Cc, Ct = lpeg.P, lpeg.R, lpeg.S, lpeg.C, lpeg.Cc, lpeg.Ct

-- create a pattern which captures the lua value [id] and the input matching
-- [patt] in a table
local function token(id, patt) return Ct(Cc(id) * C(patt)) end

local digit = R('09')

-- range of valid characters after first character of identifier
local idsafe = R('AZ', 'az', '\127\255') + P '_'

-- operators
local operator = token('operator', P '==' + P '~=' + P '<=' + P '>=' + P '...'
                                          + P '..' + S '+-*/%^#=<>;:,.{}[]()')
-- identifiers
local ident = token('identifier', idsafe * (idsafe + digit + P '.') ^ 0)

-- keywords
local olua_keywords = P '@implementation' + P '@statics' + P '@end' +
	P '@try' + P '@catch' + P '@finally' + P '@throw'

local keyword = token('keyword', (P 'and' + P 'break' + P 'do' + P 'elseif' +
   P 'else' + P 'end' + P 'false' + P 'for' + P 'function' + P 'if' +
   P 'in' + P 'local' + P 'nil' + P 'not' + P 'or' + P 'repeat' + P 'return' +
   P 'then' + P 'true' + P 'until' + P 'while' + olua_keywords ) * 
   -(idsafe + digit))

-- numbers
local number_sign = S'+-'^-1
local number_decimal = digit ^ 1
local number_hexadecimal = P '0' * S 'xX' * R('09', 'AF', 'af') ^ 1
local number_float = (digit^1 * P'.' * digit^0 + P'.' * digit^1) *
                     (S'eE' * number_sign * digit^1)^-1
local number = token('number', number_hexadecimal +
                               number_float +
                               number_decimal)

-- callback for [=[ long strings ]=]
-- ps. LPeg is for Lua what regex is for Perl, which makes me smile :)
local longstringpredicate = P(function(input, index)
   local level = input:match('^%[(=*)%[', index)
   if level then
      local _, stop = input:find(']' .. level .. ']', index, true)
      if stop then return stop + 1 end
   end
end)

local longstring = #(P'[' * P'='^1 * P'[') * longstringpredicate

-- strings
local singlequoted_string = P "'" * ((1 - S "'\r\n\f\\") + (P '\\' * 1)) ^ 0 * "'"
local doublequoted_string = P '"' * ((1 - S '"\r\n\f\\') + (P '\\' * 1)) ^ 0 * '"'
local string = token('string', singlequoted_string +
                               doublequoted_string +
                               longstring)

-- comments
local singleline_comment = P '--' * (1 - S '\r\n\f') ^ 0
local multiline_comment = P '--' * (#(P'[' * P'='^0 * P'[') * longstringpredicate)
local comment = token('comment', multiline_comment + singleline_comment)

-- whitespace
local whitespace = token('whitespace', S('\r\n\f\t ')^1)

-- ordered choice of all tokens and last-resort error which consumes one character
local any_token = whitespace + number + keyword + ident +
                  string + comment + operator + token('error', 1)

-- private interface
local table_of_tokens = Ct(any_token ^ 0)

-- increment [line] by the number of line-ends in [text]
local function sync(line, text)
   local index, limit = 1, #text
   while index <= limit do
      local start, stop = text:find('\r\n', index, true)
      if not start then
         start, stop = text:find('[\r\n\f]', index)
         if not start then break end
      end
      index = stop + 1
      line = line + 1
   end
   return line
end

-- we only need to synchronize the line-counter for these token types
local multiline_tokens = { comment = true, string = true, whitespace = true }

-- public interface
function getrawtokens(self, input)
   assert(type(input) == 'string', 'bad argument #1 (expected string)')
   local line = 1
   local tokens = lpeg.match(table_of_tokens, input)
   for i, token in pairs(tokens) do
      token[3] = line
      if multiline_tokens[token[1]] then line = sync(line, token[2]) end
   end
   return tokens
end

function gettokens(self, input)
	local rawtokens = getrawtokens(self, input)
	local tokens = {}
	
	local lastrawtoken = nil
	
	local function pushtoken(rawtoken)
		if lastrawtoken and (lastrawtoken[1] ~= 'whitespace') and
				(lastrawtoken[1] ~= 'comment') then
			local t =
			{
				type = lastrawtoken[1],
				text = lastrawtoken[2],
				line = lastrawtoken[3],
				nlafter = false
			}
			
			if rawtoken and rawtoken[2]:find('\n') then
				t.nlafter = true
			end
			
			tokens[#tokens+1] = t
		end
		
		lastrawtoken = rawtoken
	end
	
	for _, rawtoken in ipairs(rawtokens) do
		pushtoken(rawtoken)
	end
	pushtoken(nil)
	
	return tokens
end

function printraw(tokens)
   local print, format = _G.print, _G.string.format
   for _, token in pairs(tokens) do
      print(format('line %i, %s: `%s`', token[3], token[1], token[2]))
   end
   print(format('total of %i tokens, %i lines', #tokens, tokens[#tokens][3]))
end

function print(tokens)
	local print, format = _G.print, _G.string.format
	for _, token in pairs(tokens) do
		local nlafter = 'false'
		if token.nlafter then
			nlafter = 'true'
		end
		
		print(token.line, token.nlafter, token.type, token.text)
		--print(format('line %i, nlafter=%s %s: `%s`', token.line, nlafter, token.type, token.text))
	end
	print(format('total of %i tokens, %i lines', #tokens, tokens[#tokens].line))
end
	
getmetatable(getfenv(1)).__call = gettokens

	