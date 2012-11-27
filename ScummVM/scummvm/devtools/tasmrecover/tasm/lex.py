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

def parse_args(text):
	#print "parsing: [%s]" %text
	escape = False
	string = False
	result = []
	token = str()
	value = 0;
	for c in text:
		#print "[%s]%s: %s: %s" %(token, c, escape, string)
		if c == '\\':
			escape = True
			continue

		if escape:
			if not string:
				raise SyntaxError("escape found in no string: %s" %text);
	
			#print "escaping[%s]" %c
			escape = False
			token += c
			continue

		if string:
			if c == '\'' or c == '"':
				string = False

			token += c
			continue
		
		if c == '\'' or c == '"':
			string = True
			token += c
			continue

		if c == ',':
			result.append(token.strip())
			token = str()
			continue

		if c == ';': #comment, bailing out
			break
		
		token += c
	#token = token.strip()
	if len(token):
		result.append(token)
	#print result
	return result

def compile(width, data):
	print data
	return data
