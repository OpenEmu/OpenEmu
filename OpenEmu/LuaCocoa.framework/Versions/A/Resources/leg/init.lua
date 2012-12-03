--[[
<%
  project.title = "Leg"
  project.description = "LPeg-powered Lua 5.1 grammar"
  project.version = "0.1.2"
  project.date = _G.os.date'%B %d, %Y'
  project.modules = { 'grammar', 'parser', 'scanner' }
%>

# Overview

Leg is a Lua library which offers a complete Lua 5.1 grammar, along with some functions to use and modify it. Some examples of projects which could benefit from Leg are a syntax highlighter, a Luadoc-style document generator, and a macro preprocessor. 

Leg uses [http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg] for pattern matching, and returns [http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg] patterns for user manipulation.

Leg is available under the same [#section_License license] as Lua 5.1.

# Dependencies

* [http://www.inf.puc-rio.br/~roberto/lpeg.html LPeg]

# Download

Leg can be downloaded from its [http://luaforge.net/projects/leg/ LuaForge page]. 

# Credits

This project is maintained by Humberto Anjos, and was adapted from an earlier project done with Francisco Sant'Anna.

# License

Copyright &#169; 2007 Humberto Saraiva Nazareno dos Anjos.
 
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. 
--]]


-- $Id: init.lua,v 1.2 2007/11/26 18:41:51 hanjos Exp $

require 'leg.grammar'
require 'leg.scanner'
require 'leg.parser'

module 'leg'