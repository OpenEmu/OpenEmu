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
import op

class proc:
	last_addr = 0xc000
	
	def __init__(self, name):
		self.name = name
		self.calls = []
		self.stmts = []
		self.labels = set()
		self.retlabels = set()
		self.__label_re = re.compile(r'^(\S+):(.*)$')
		self.offset = proc.last_addr
		proc.last_addr += 4

	def add_label(self, label):
		self.stmts.append(op.label(label))
		self.labels.add(label)

	def remove_label(self, label):
		try:
			self.labels.remove(label)
		except:
			pass
		for i in xrange(len(self.stmts)):
			if isinstance(self.stmts[i], op.label) and self.stmts[i].name == label:
				self.stmts[i] = op._nop(None)
				return
	
	def optimize_sequence(self, cls):
		i = 0
		stmts = self.stmts
		while i < len(stmts):
			if not isinstance(stmts[i], cls):
				i += 1
				continue
			if i > 0 and isinstance(stmts[i - 1], op._rep): #skip rep prefixed instructions for now
				i += 1
				continue
			j = i + 1

			while j < len(stmts):
				if not isinstance(stmts[j], cls):
					break
				j = j + 1

			n = j - i
			if n > 1:
				print "Eliminate consequtive storage instructions at %u-%u" %(i, j)
				for k in range(i+1,j):
					stmts[k] = op._nop(None)
				stmts[i].repeat = n
			else:
				i = j

		i = 0
		while i < len(stmts):
			if not isinstance(stmts[i], op._rep):
				i += 1
				continue
			if i + 1 >= len(stmts):
				break
			if isinstance(stmts[i + 1], cls):
				stmts[i + 1].repeat = 'cx'
				stmts[i + 1].clear_cx = True
				stmts[i] = op._nop(None)
			i += 1
		return
	
	def optimize(self, keep_labels=[]):
		print "optimizing..."
		#trivial simplifications
		while len(self.stmts) and isinstance(self.stmts[-1], op.label):
			print "stripping last label"
			self.stmts.pop()
		#mark labels that directly precede a ret
		for i in range(len(self.stmts)):
			if not isinstance(self.stmts[i], op.label):
				continue
			j = i
			while j < len(self.stmts) and isinstance(self.stmts[j], (op.label, op._nop)):
				j += 1
			if j == len(self.stmts) or isinstance(self.stmts[j], op._ret):
				print "Return label: %s" % (self.stmts[i].name,)
				self.retlabels.add(self.stmts[i].name)
		#merging push ax pop bx constructs
		i = 0
		while i + 1 < len(self.stmts):
			a, b = self.stmts[i], self.stmts[i + 1]
			if isinstance(a, op._push) and isinstance(b, op._pop):
				ar, br = a.regs, b.regs
				movs = []
				while len(ar) and len(br):
					src = ar.pop()
					dst = br.pop(0)
					movs.append(op._mov2(dst, src))
				if len(br) == 0:
					self.stmts.pop(i + 1)
				print "merging %d push-pops into movs" %(len(movs))
				for m in movs:
					print "\t%s <- %s" %(m.dst, m.src)
				self.stmts[i + 1:i + 1] = movs
				if len(ar) == 0:
					self.stmts.pop(i)
			else:
				i += 1
		
		#eliminating unused labels
		for s in list(self.stmts):
			if not isinstance(s, op.label):
				continue
			print "checking label %s..." %s.name
			used = s.name in keep_labels
			if s.name not in self.retlabels:
				for j in self.stmts:
					if isinstance(j, op.basejmp) and j.label == s.name:
						print "used"
						used = True
						break
			if not used:
				print self.labels
				self.remove_label(s.name)

		#removing duplicate rets and rets at end
		for i in xrange(len(self.stmts)):
			if isinstance(self.stmts[i], op._ret):
				j = i+1
				while j < len(self.stmts) and isinstance(self.stmts[j], op._nop):
					j += 1
				if j == len(self.stmts) or isinstance(self.stmts[j], op._ret):
					self.stmts[i] = op._nop(None)

		self.optimize_sequence(op._stosb);
		self.optimize_sequence(op._stosw);
		self.optimize_sequence(op._movsb);
		self.optimize_sequence(op._movsw);
	
	def add(self, stmt):
		#print stmt
		comment = stmt.rfind(';')
		if comment >= 0:
			stmt = stmt[:comment]
		stmt = stmt.strip()

		r = self.__label_re.search(stmt)
		if r is not None:
			#label
			self.add_label(r.group(1).lower())
			#print "remains: %s" %r.group(2)
			stmt = r.group(2).strip()

		if len(stmt) == 0:
			return
		
		s = stmt.split(None)
		cmd = s[0]
		cl = getattr(op, '_' + cmd)
		arg = " ".join(s[1:]) if len(s) > 1 else str()
		o = cl(arg)
		self.stmts.append(o)
	
	def __str__(self):
		r = []
		for i in self.stmts:
			r.append(i.__str__())
		return "\n".join(r)

	def visit(self, visitor, skip = 0):
		for i in xrange(skip, len(self.stmts)):
			self.stmts[i].visit(visitor)
