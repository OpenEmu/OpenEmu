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

import op, traceback, re, proc
from copy import copy
proc_module = proc

class CrossJump(Exception):
	pass

def parse_bin(s):
	b = s.group(1)
	v = hex(int(b, 2))
	#print "BINARY: %s -> %s" %(b, v)
	return v

class cpp:
	def __init__(self, context, namespace, skip_first = 0, blacklist = [], skip_output = [], skip_dispatch_call = False, skip_addr_constants = False, header_omit_blacklisted = False, function_name_remapping = { }):
		self.namespace = namespace
		fname = namespace.lower() + ".cpp"
		header = namespace.lower() + ".h"
		banner = """/* PLEASE DO NOT MODIFY THIS FILE. ALL CHANGES WILL BE LOST! LOOK FOR README FOR DETAILS */

/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */
"""
		self.fd = open(fname, "wt")
		self.hd = open(header, "wt")
		hid = "TASMRECOVER_%s_STUBS_H__" %namespace.upper()
		self.hd.write("""#ifndef %s
#define %s

%s""" %(hid, hid, banner))
		self.context = context
		self.data_seg = context.binary_data
		self.procs = context.proc_list
		self.skip_first = skip_first
		self.proc_queue = []
		self.proc_done = []
		self.blacklist = blacklist
		self.failed = list(blacklist)
		self.skip_output = skip_output
		self.skip_dispatch_call = skip_dispatch_call
		self.skip_addr_constants = skip_addr_constants
		self.header_omit_blacklisted = header_omit_blacklisted
		self.function_name_remapping = function_name_remapping
		self.translated = []
		self.proc_addr = []
		self.used_data_offsets = set()
		self.methods = []
		self.fd.write("""%s
#include \"%s\"

namespace %s {
""" %(banner, header, namespace))

	def expand_cb(self, match):
		name = match.group(0).lower()
		if len(name) == 2 and \
			((name[0] in ['a', 'b', 'c', 'd'] and name[1] in ['h', 'x', 'l']) or name in ['si', 'di', 'es', 'ds', 'cs']):
			return "%s" %name

		if self.indirection == -1:
			try:
				offset,p,p = self.context.get_offset(name)
			except:
				pass
			else:
				print "OFFSET = %d" %offset
				self.indirection = 0
				self.used_data_offsets.add((name,offset))
				return "offset_%s" % (name,)
		
		g = self.context.get_global(name)
		if isinstance(g, op.const):
			value = self.expand_equ(g.value)
			print "equ: %s -> %s" %(name, value)
		elif isinstance(g, proc.proc):
			if self.indirection != -1:
				raise Exception("invalid proc label usage")
			value = str(g.offset)
			self.indirection = 0
		else:
			size = g.size
			if size == 0:
				raise Exception("invalid var '%s' size %u" %(name, size))
			if self.indirection == 0:
				value = "data.%s(k%s)" %("byte" if size == 1 else "word", name.capitalize())
			elif self.indirection == -1:
				value = "%s" %g.offset
				self.indirection = 0
			else:
				raise Exception("invalid indirection %d" %self.indirection)
		return value
	
	def get_size(self, expr):
		#print 'get_size("%s")' %expr
		try:
			v = self.context.parse_int(expr)
			return 1 if v < 256 else 2
		except:
			pass

		if re.match(r'byte\s+ptr\s', expr) is not None:
			return 1

		if re.match(r'word\s+ptr\s', expr) is not None:
			return 2

		if len(expr) == 2 and expr[0] in ['a', 'b', 'c', 'd'] and expr[1] in ['h', 'l']:
			return 1
		if expr in ['ax', 'bx', 'cx', 'dx', 'si', 'di', 'sp', 'bp', 'ds', 'cs', 'es', 'fs']:
			return 2

		m = re.match(r'[a-zA-Z_]\w*', expr)
		if m is not None:
			name = m.group(0)
			try:
				g = self.context.get_global(name)
				return g.size
			except:
				pass

		return 0

	def expand_equ_cb(self, match):
		name = match.group(0).lower()
		g = self.context.get_global(name)
		if isinstance(g, op.const):
			return g.value
		return str(g.offset)

	def expand_equ(self, expr):
		n = 1
		while n > 0:
			expr, n = re.subn(r'\b[a-zA-Z_][a-zA-Z0-9_]+\b', self.expand_equ_cb, expr)
		expr = re.sub(r'\b([0-9][a-fA-F0-9]*)h', '0x\\1', expr)
		return "(%s)" %expr

	def expand(self, expr, def_size = 0):
		#print "EXPAND \"%s\"" %expr
		size = self.get_size(expr) if def_size == 0 else def_size
		indirection = 0
		seg = None
		reg = True

		m = re.match(r'seg\s+(.*?)$', expr)
		if m is not None:
			return "data"
		
		match_id = True
		m = re.match(r'offset\s+(.*?)$', expr)
		if m is not None:
			indirection -= 1
			expr = m.group(1).strip()

		m = re.match(r'byte\s+ptr\s+(.*?)$', expr)
		if m is not None:
			expr = m.group(1).strip()

		m = re.match(r'word\s+ptr\s+(.*?)$', expr)
		if m is not None:
			expr = m.group(1).strip()

		m = re.match(r'\[(.*)\]$', expr)
		if m is not None:
			indirection += 1
			expr = m.group(1).strip()

		m = re.match(r'(\w{2,2}):(.*)$', expr)
		if m is not None:
			seg_prefix = m.group(1)
			expr = m.group(2).strip()
			print "SEGMENT %s, remains: %s" %(seg_prefix, expr)
		else:
			seg_prefix = "ds"

		m = re.match(r'(([abcd][xhl])|si|di|bp|sp)([\+-].*)?$', expr)
		if m is not None:
			reg = m.group(1)
			plus = m.group(3)
			if plus is not None:
				plus = self.expand(plus)
			else:
				plus = ""
			match_id = False
			#print "COMMON_REG: ", reg, plus
			expr = "%s%s" %(reg, plus)

		expr = re.sub(r'\b([0-9][a-fA-F0-9]*)h', '0x\\1', expr)
		expr = re.sub(r'\b([0-1]+)b', parse_bin, expr)
		expr = re.sub(r'"(.)"', '\'\\1\'', expr)
		if match_id:
			#print "BEFORE: %d" %indirection
			self.indirection = indirection
			expr = re.sub(r'\b[a-zA-Z_][a-zA-Z0-9_]+\b', self.expand_cb, expr)
			indirection = self.indirection
			#print "AFTER: %d" %indirection
		
		if indirection == 1:
			if size == 1:
				expr = "%s.byte(%s)" %(seg_prefix, expr)
			elif size == 2:
				expr = "%s.word(%s)" %(seg_prefix, expr)
			else:
				expr = "@invalid size 0"
		elif indirection == 0:
			pass
		elif indirection == -1:
			expr = "&%s" %expr
		else:
			raise Exception("invalid indirection %d" %indirection)
		return expr
	
	def mangle_label(self, name):
		name = name.lower()
		return re.sub(r'\$', '_tmp', name)

	def resolve_label(self, name):
		name = name.lower()
		if not name in self.proc.labels:
			try:
				offset, proc, pos = self.context.get_offset(name)
			except:
				print "no label %s, trying procedure" %name
				proc = self.context.get_global(name)
				pos = 0
				if not isinstance(proc, proc_module.proc):
					raise CrossJump("cross-procedure jump to non label and non procedure %s" %(name))
			self.proc.labels.add(name)
			for i in xrange(0, len(self.unbounded)):
				u = self.unbounded[i]
				if u[1] == proc:
					if pos < u[2]:
						self.unbounded[i] = (name, proc, pos)
				return self.mangle_label(name)
			self.unbounded.append((name, proc, pos))
		
		return self.mangle_label(name)

	def jump_to_label(self, name):
		jump_proc = False
		if name in self.blacklist:
			jump_proc = True
		
		if self.context.has_global(name) :
			g = self.context.get_global(name)
			if isinstance(g, proc_module.proc):
				jump_proc = True
		
		if jump_proc:
			if name in self.function_name_remapping:
				return "{ %s(); return; }" %self.function_name_remapping[name]
			else:
				return "{ %s(); return; }" %name
		else:
			# TODO: name or self.resolve_label(name) or self.mangle_label(name)??
			if name in self.proc.retlabels:
				return "return /* (%s) */" % (name)
			return "goto %s" %self.resolve_label(name)
	
	def _label(self, name):
		self.body += "%s:\n" %self.mangle_label(name)
	
	def schedule(self, name):
		name = name.lower()
		if name in self.proc_queue or name in self.proc_done or name in self.failed:
			return
		print "+scheduling function %s..." %name
		self.proc_queue.append(name)

	def _call(self, name):
		name = name.lower()
		if name == 'ax':
			self.body += "\t__dispatch_call(%s);\n" %self.expand('ax', 2)
			return
		if name in self.function_name_remapping:
			self.body += "\t%s();\n" %self.function_name_remapping[name]
		else:
			self.body += "\t%s();\n" %name
		self.schedule(name)

	def _ret(self):
		self.body += "\treturn;\n"

	def parse2(self, dst, src):
		dst_size, src_size = self.get_size(dst), self.get_size(src)
		if dst_size == 0:
			if src_size == 0:
				raise Exception("both sizes are 0")
			dst_size = src_size
		if src_size == 0:
			src_size = dst_size

		dst = self.expand(dst, dst_size)
		src = self.expand(src, src_size)
		return dst, src

	def _mov(self, dst, src):
		self.body += "\t%s = %s;\n" %self.parse2(dst, src)

	def _add(self, dst, src):
		self.body += "\t_add(%s, %s);\n" %self.parse2(dst, src)

	def _sub(self, dst, src):
		self.body += "\t_sub(%s, %s);\n" %self.parse2(dst, src)

	def _and(self, dst, src):
		self.body += "\t_and(%s, %s);\n" %self.parse2(dst, src)

	def _or(self, dst, src):
		self.body += "\t_or(%s, %s);\n" %self.parse2(dst, src)

	def _xor(self, dst, src):
		self.body += "\t_xor(%s, %s);\n" %self.parse2(dst, src)

	def _neg(self, dst):
		dst = self.expand(dst)
		self.body += "\t_neg(%s);\n" %(dst)

	def _cbw(self):
		self.body += "\tax.cbw();\n"

	def _shr(self, dst, src):
		self.body += "\t_shr(%s, %s);\n" %self.parse2(dst, src)

	def _shl(self, dst, src):
		self.body += "\t_shl(%s, %s);\n" %self.parse2(dst, src)

	#def _sar(self, dst, src):
	#	self.body += "\t_sar(%s%s);\n" %self.parse2(dst, src)

	#def _sal(self, dst, src):
	#	self.body += "\t_sal(%s, %s);\n" %self.parse2(dst, src)

	#def _rcl(self, dst, src):
	#	self.body += "\t_rcl(%s, %s);\n" %self.parse2(dst, src)

	#def _rcr(self, dst, src):
	#	self.body += "\t_rcr(%s, %s);\n" %self.parse2(dst, src)

	def _mul(self, src):
		src = self.expand(src)
		self.body += "\t_mul(%s);\n" %(src)

	def _div(self, src):
		src = self.expand(src)
		self.body += "\t_div(%s);\n" %(src)

	def _inc(self, dst):
		dst = self.expand(dst)
		self.body += "\t_inc(%s);\n" %(dst)

	def _dec(self, dst):
		dst = self.expand(dst)
		self.body += "\t_dec(%s);\n" %(dst)

	def _cmp(self, a, b):
		self.body += "\t_cmp(%s, %s);\n" %self.parse2(a, b)

	def _test(self, a, b):
		self.body += "\t_test(%s, %s);\n" %self.parse2(a, b)

	def _js(self, label):
		self.body += "\tif (flags.s())\n\t\t%s;\n" %(self.jump_to_label(label))

	def _jns(self, label):
		self.body += "\tif (!flags.s())\n\t\t%s;\n" %(self.jump_to_label(label)) 

	def _jz(self, label):
		self.body += "\tif (flags.z())\n\t\t%s;\n" %(self.jump_to_label(label)) 

	def _jnz(self, label):
		self.body += "\tif (!flags.z())\n\t\t%s;\n" %(self.jump_to_label(label)) 

	def _jl(self, label):
		self.body += "\tif (flags.l())\n\t\t%s;\n" %(self.jump_to_label(label)) 

	def _jg(self, label):
		self.body += "\tif (!flags.le())\n\t\t%s;\n" %(self.jump_to_label(label)) 

	def _jle(self, label):
		self.body += "\tif (flags.le())\n\t\t%s;\n" %(self.jump_to_label(label)) 

	def _jge(self, label):
		self.body += "\tif (!flags.l())\n\t\t%s;\n" %(self.jump_to_label(label)) 

	def _jc(self, label):
		self.body += "\tif (flags.c())\n\t\t%s;\n" %(self.jump_to_label(label)) 

	def _jnc(self, label):
		self.body += "\tif (!flags.c())\n\t\t%s;\n" %(self.jump_to_label(label)) 
	
	def _xchg(self, dst, src):
		self.body += "\t_xchg(%s, %s);\n" %self.parse2(dst, src)

	def _jmp(self, label):
		self.body += "\t%s;\n" %(self.jump_to_label(label)) 

	def _loop(self, label):
		self.body += "\tif (--cx)\n\t\t%s;\n" %self.jump_to_label(label)

	def _push(self, regs):
		p = str();
		for r in regs:
			r = self.expand(r)
			p += "\tpush(%s);\n" %(r)
		self.body += p

	def _pop(self, regs):
		p = str();
		for r in regs:
			self.temps_count -= 1
			i = self.temps_count
			r = self.expand(r)
			p += "\t%s = pop();\n" %r
		self.body += p

	def _rep(self):
		self.body += "\twhile(cx--)\n\t"

	def _lodsb(self):
		self.body += "\t_lodsb();\n"

	def _lodsw(self):
		self.body += "\t_lodsw();\n"

	def _stosb(self, n, clear_cx):
		self.body += "\t_stosb(%s%s);\n" %("" if n == 1 else n, ", true" if clear_cx else "")

	def _stosw(self, n, clear_cx):
		self.body += "\t_stosw(%s%s);\n" %("" if n == 1 else n, ", true" if clear_cx else "")

	def _movsb(self, n, clear_cx):
		self.body += "\t_movsb(%s%s);\n" %("" if n == 1 else n, ", true" if clear_cx else "")

	def _movsw(self, n, clear_cx):
		self.body += "\t_movsw(%s%s);\n" %("" if n == 1 else n, ", true" if clear_cx else "")

	def _stc(self):
		self.body += "\tflags._c = true;\n "

	def _clc(self):
		self.body += "\tflags._c = false;\n "

	def __proc(self, name, def_skip = 0):
		try:
			skip = def_skip
			self.temps_count = 0
			self.temps_max = 0
			if self.context.has_global(name):
				self.proc = self.context.get_global(name)
			else:
				print "No procedure named %s, trying label" %name
				off, src_proc, skip = self.context.get_offset(name)
				
				self.proc = proc_module.proc(name)
				self.proc.stmts = copy(src_proc.stmts)
				self.proc.labels = copy(src_proc.labels)
				self.proc.retlabels = copy(src_proc.retlabels)
				#for p in xrange(skip, len(self.proc.stmts)):
				#	s = self.proc.stmts[p]
				#	if isinstance(s, op.basejmp):
				#		o, p, s = self.context.get_offset(s.label)
				#		if p == src_proc and s < skip:
				#			skip = s
			
			
			self.proc_addr.append((name, self.proc.offset))
			self.body = str()
			if name in self.function_name_remapping:
				self.body += "void %sContext::%s() {\n\tSTACK_CHECK;\n" %(self.namespace, self.function_name_remapping[name]);
			else:
				self.body += "void %sContext::%s() {\n\tSTACK_CHECK;\n" %(self.namespace, name);
			self.proc.optimize()
			self.unbounded = []
			self.proc.visit(self, skip)

			#adding remaining labels:
			for i in xrange(0, len(self.unbounded)):
				u = self.unbounded[i]
				print "UNBOUNDED: ", u
				proc = u[1]
				for p in xrange(u[2], len(proc.stmts)):
					s = proc.stmts[p]
					if isinstance(s, op.basejmp):
						self.resolve_label(s.label)
			
			#adding statements
			#BIG FIXME: this is quite ugly to handle code analysis from the code generation. rewrite me!
			for label, proc, offset in self.unbounded:
				self.body += "\treturn;\n" #we need to return before calling code from the other proc
				self.body += "/*continuing to unbounded code: %s from %s:%d-%d*/\n" %(label, proc.name, offset, len(proc.stmts))
				start = len(self.proc.stmts)
				self.proc.add_label(label)
				for s in proc.stmts[offset:]:
					if isinstance(s, op.label):
						self.proc.labels.add(s.name)
					self.proc.stmts.append(s)
				self.proc.add("ret")
				print "skipping %d instructions, todo: %d" %(start, len(self.proc.stmts) - start)
				print "re-optimizing..."
				self.proc.optimize(keep_labels=[label])
				self.proc.visit(self, start)
			self.body += "}\n";
			if name not in self.skip_output:
				self.translated.insert(0, self.body)
			self.proc = None
			if self.temps_count > 0:
				raise Exception("temps count == %d at the exit of proc" %self.temps_count);
			return True
		except (CrossJump, op.Unsupported) as e:
			print "%s: ERROR: %s" %(name, e)
			self.failed.append(name)
		except:
			raise

	def get_type(self, width):
		return "uint%d_t" %(width * 8)
	
	def write_stubs(self, fname, procs):
		fd = open(fname, "wt")
		fd.write("namespace %s {\n" %self.namespace)
		for p in procs:
			if p in self.function_name_remapping:
				fd.write("void %sContext::%s() {\n\t::error(\"%s\");\n}\n\n" %(self.namespace, self.function_name_remapping[p], self.function_name_remapping[p]))
			else:
				fd.write("void %sContext::%s() {\n\t::error(\"%s\");\n}\n\n" %(self.namespace, p, p))
		fd.write("} // End of namespace  %s\n" %self.namespace)
		fd.close()


	def generate(self, start):
		#print self.prologue()
		#print context
		self.proc_queue.append(start)
		while len(self.proc_queue):
			name = self.proc_queue.pop()
			if name in self.failed or name in self.proc_done:
				continue
			if len(self.proc_queue) == 0 and len(self.procs) > 0:
				print "queue's empty, adding remaining procs:"
				for p in self.procs:
					self.schedule(p)
				self.procs = []
			print "continuing on %s" %name
			self.proc_done.append(name)
			self.__proc(name)
			self.methods.append(name)
		self.write_stubs("_stubs.cpp", self.failed)
		self.methods += self.failed
		done, failed = len(self.proc_done), len(self.failed)

		self.fd.write("\n")
		self.fd.write("\n".join(self.translated))
		self.fd.write("\n")
		print "%d ok, %d failed of %d, %.02g%% translated" %(done, failed, done + failed, 100.0 * done / (done + failed))
		print "\n".join(self.failed)
		data_bin = self.data_seg
		data_impl = "\n\tstatic const uint8 src[] = {\n\t\t"
		n = 0
		comment = str()
		for v in data_bin:
			data_impl += "0x%02x, " %v
			n += 1

			comment += chr(v) if (v >= 0x20 and v < 0x7f and v != ord('\\')) else "."
			if (n & 0xf) == 0:
				data_impl += "\n\t\t//0x%04x: %s\n\t\t" %(n - 16, comment)
				comment = str()
			elif (n & 0x3) == 0:
				comment += " "
		data_impl += "};\n\tds.assign(src, src + sizeof(src));\n"

		self.hd.write(
"""\n#include "dreamweb/runtime.h"

#include "dreamweb/structs.h"
#include "dreamweb/dreambase.h"

namespace %s {

"""
%(self.namespace))

		if self.skip_addr_constants == False:
			for name,addr in self.proc_addr:
				self.hd.write("static const uint16 addr_%s = 0x%04x;\n" %(name, addr))


		for name,addr in self.used_data_offsets:
			self.hd.write("static const uint16 offset_%s = 0x%04x;\n" %(name, addr))

		offsets = []
		for k, v in self.context.get_globals().items():
			if isinstance(v, op.var):
				offsets.append((k.capitalize(), v.offset))
			elif isinstance(v, op.const):
				offsets.append((k.capitalize(), self.expand_equ(v.value))) #fixme: try to save all constants here
		
		offsets = sorted(offsets, key=lambda t: t[1])
		for o in offsets:
			self.hd.write("static const uint16 k%s = %s;\n" %o)
		self.hd.write("\n")

		self.hd.write(
"""
class %sContext : public DreamBase, public Context {
public:
	DreamGenContext(DreamWeb::DreamWebEngine *en) : DreamBase(en), Context(this) {}

	void __start();
"""
%(self.namespace))
		if self.skip_dispatch_call == False:
			self.hd.write(
"""	void __dispatch_call(uint16 addr);
""")


		for p in set(self.methods):
			if p in self.blacklist:
				if self.header_omit_blacklisted == False:
					self.hd.write("\t//void %s();\n" %p)
			else:
				if p in self.function_name_remapping:
					self.hd.write("\tvoid %s();\n" %self.function_name_remapping[p])
				else:
					self.hd.write("\tvoid %s();\n" %p)

		self.hd.write("};\n\n} // End of namespace DreamGen\n\n#endif\n")
		self.hd.close()
		
		self.fd.write("void %sContext::__start() { %s\t%s(); \n}\n" %(self.namespace, data_impl, start))
		
		if self.skip_dispatch_call == False:
			self.fd.write("\nvoid %sContext::__dispatch_call(uint16 addr) {\n\tswitch(addr) {\n" %self.namespace)
			self.proc_addr.sort(cmp = lambda x, y: x[1] - y[1])
			for name,addr in self.proc_addr:
				self.fd.write("\t\tcase addr_%s: %s(); break;\n" %(name, name))
			self.fd.write("\t\tdefault: ::error(\"invalid call to %04x dispatched\", (uint16)ax);")
			self.fd.write("\n\t}\n}")
		
		self.fd.write("\n} // End of namespace DreamGen\n")
		self.fd.close()
