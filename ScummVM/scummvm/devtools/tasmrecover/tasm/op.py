# ScummVM - Graphic Adventure Engine
#
# ScummVM is the legal property of its developers, whose names
# are too numerous to list here. Please refer to the COPYRIGHT
# file distributed with this source distribution.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#

import re
import lex

class Unsupported(Exception):
	pass

class var:
	def __init__(self, size, offset):
		self.size = size
		self.offset = offset

class const:
	def __init__(self, value):
		self.value = value

class reg:
	def __init__(self, name):
		self.name = name
	def size(self):
		return 2 if self.name[1] == 'x' else 1
	def __str__(self):
		return "<register %s>" %self.name

class unref:
	def __init__(self, exp):
		self.exp = exp
	def __str__(self):
		return "<unref %s>" %self.exp

class ref:
	def __init__(self, name):
		self.name = name
	def __str__(self):
		return "<ref %s>" %self.name

class glob:
	def __init__(self, name):
		self.name = name
	def __str__(self):
		return "<global %s>" %self.name

class segment:
	def __init__(self, name):
		self.name = name
	def __str__(self):
		return "<segment %s>" %self.name

class baseop(object):
	def parse_arg(self, arg):
		return arg

	def split(self, text):
		a, b = lex.parse_args(text)
		return self.parse_arg(a), self.parse_arg(b)
	def __str__(self):
		return str(self.__class__)

class basejmp(baseop):
	pass

class _call(baseop):
	def __init__(self, arg):
		self.name = arg
	def visit(self, visitor):
		visitor._call(self.name)
	def __str__(self):
		return "call(%s)" %self.name

class _rep(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		visitor._rep()

class _mov(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._mov(self.dst, self.src)
	def __str__(self):
		return "mov(%s, %s)" %(self.dst, self.src)

class _mov2(baseop):
	def __init__(self, dst, src):
		self.dst, self.src = dst, src
	def visit(self, visitor):
		visitor._mov(self.dst, self.src)

class _shr(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._shr(self.dst, self.src)

class _shl(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._shl(self.dst, self.src)

class _ror(baseop):
	def __init__(self, arg):
		pass

class _rol(baseop):
	def __init__(self, arg):
		pass

class _sar(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._sar(self.dst, self.src)

class _sal(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._sal(self.dst, self.src)

class _rcl(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._rcl(self.dst, self.src)

class _rcr(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._rcr(self.dst, self.src)

class _neg(baseop):
	def __init__(self, arg):
		self.arg = arg
	def visit(self, visitor):
		visitor._neg(self.arg)

class _dec(baseop):
	def __init__(self, arg):
		self.dst = arg
	def visit(self, visitor):
		visitor._dec(self.dst)

class _inc(baseop):
	def __init__(self, arg):
		self.dst = arg
	def visit(self, visitor):
		visitor._inc(self.dst)

class _add(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._add(self.dst, self.src)

class _sub(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._sub(self.dst, self.src)

class _mul(baseop):
	def __init__(self, arg):
		self.arg = self.parse_arg(arg)
	def visit(self, visitor):
		visitor._mul(self.arg)

class _div(baseop):
	def __init__(self, arg):
		self.arg = self.parse_arg(arg)
	def visit(self, visitor):
		visitor._div(self.arg)

class _and(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._and(self.dst, self.src)

class _xor(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._xor(self.dst, self.src)

class _or(baseop):
	def __init__(self, arg):
		self.dst, self.src = self.split(arg)
	def visit(self, visitor):
		visitor._or(self.dst, self.src)

class _cmp(baseop):
	def __init__(self, arg):
		self.a, self.b = self.split(arg)
	def visit(self, visitor):
		visitor._cmp(self.a, self.b)

class _test(baseop):
	def __init__(self, arg):
		self.a, self.b = self.split(arg)
	def visit(self, visitor):
		visitor._test(self.a, self.b)

class _xchg(baseop):
	def __init__(self, arg):
		self.a, self.b = self.split(arg)
	def visit(self, visitor):
		visitor._xchg(self.a, self.b)

class _jnz(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jnz(self.label)

class _jz(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jz(self.label)

class _jc(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jc(self.label)

class _jnc(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jnc(self.label)

class _js(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._js(self.label)

class _jns(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jns(self.label)

class _jl(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jl(self.label)

class _jg(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jg(self.label)

class _jle(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jle(self.label)

class _jge(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jge(self.label)

class _jmp(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._jmp(self.label)

class _loop(basejmp):
	def __init__(self, label):
		self.label = label
	def visit(self, visitor):
		visitor._loop(self.label)

class _push(baseop):
	def __init__(self, arg):
		self.regs = []
		for r in arg.split():
			self.regs.append(self.parse_arg(r))
	def visit(self, visitor):
		visitor._push(self.regs)

class _pop(baseop):
	def __init__(self, arg):
		self.regs = []
		for r in arg.split():
			self.regs.append(self.parse_arg(r))
	def visit(self, visitor):
		visitor._pop(self.regs)

class _ret(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		visitor._ret()

class _lodsb(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		visitor._lodsb()

class _lodsw(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		visitor._lodsw()

class _stosw(baseop):
	def __init__(self, arg):
		self.repeat = 1
		self.clear_cx = False
	def visit(self, visitor):
		visitor._stosw(self.repeat, self.clear_cx)

class _stosb(baseop):
	def __init__(self, arg):
		self.repeat = 1
		self.clear_cx = False
	def visit(self, visitor):
		visitor._stosb(self.repeat, self.clear_cx)

class _movsw(baseop):
	def __init__(self, arg):
		self.repeat = 1
		self.clear_cx = False
	def visit(self, visitor):
		visitor._movsw(self.repeat, self.clear_cx)

class _movsb(baseop):
	def __init__(self, arg):
		self.repeat = 1
		self.clear_cx = False
	def visit(self, visitor):
		visitor._movsb(self.repeat, self.clear_cx)

class _in(baseop):
	def __init__(self, arg):
		self.arg = arg
	def visit(self, visitor):
		raise Unsupported("input from port: %s" %self.arg)

class _out(baseop):
	def __init__(self, arg):
		self.arg = arg
	def visit(self, visitor):
		raise Unsupported("out to port: %s" %self.arg)

class _cli(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		raise Unsupported("cli")

class _sti(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		raise Unsupported("sli")

class _int(baseop):
	def __init__(self, arg):
		self.arg = arg
	def visit(self, visitor):
		raise Unsupported("interrupt: %s" %self.arg)

class _iret(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		raise Unsupported("interrupt return")

class _cbw(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		visitor._cbw()

class _nop(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		pass

class _stc(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		visitor._stc()

class _clc(baseop):
	def __init__(self, arg):
		pass
	def visit(self, visitor):
		visitor._clc()

class label(baseop):
	def __init__(self, name):
		self.name = name
	def visit(self, visitor):
		visitor._label(self.name)
