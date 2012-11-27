#!/usr/bin/env python
# encoding: utf-8

"""
 " ScummVM - Graphic Adventure Engine
 "
 " ScummVM is the legal property of its developers, whose names
 " are too numerous to list here. Please refer to the COPYRIGHT
 " file distributed with this source distribution.
 "
 " This program is free software; you can redistribute it and/or
 " modify it under the terms of the GNU General Public License
 " as published by the Free Software Foundation; either version 2
 " of the License, or (at your option) any later version.
 "
 " This program is distributed in the hope that it will be useful,
 " but WITHOUT ANY WARRANTY; without even the implied warranty of
 " MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 " GNU General Public License for more details.
 "
 " You should have received a copy of the GNU General Public License
 " along with this program; if not, write to the Free Software
 " Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 "
"""

from __future__ import with_statement
import os
import xml.dom.minidom as DOM
import struct

FILTER=''.join([(len(repr(chr(x)))==3) and chr(x) or '.' for x in range(256)])

# adapted from Activestate Snippet Cookbook
def printBinaryDump(src, length=16):
    N=0; result=''
    while src:
       s,src = src[:length],src[length:]
       hexa = ' '.join(["%02X"%ord(x) for x in s])
       s = s.translate(FILTER)
       result += "%04X   %-*s   %s\n" % (N, length*3, hexa, s)
       N+=length
    print (result)

def pbin(data):
	return str(map(lambda c: hex(ord(c)), data))
#	return " ".join(["%0.2X" % ord(c) for c in data])

class STXBinaryFile(object):
	class InvalidRGBColor(Exception):
		pass

	class InvalidResolution(Exception):
		pass

	class InvalidFontIdentifier(Exception):
		pass

	class InvalidBitmapName(Exception):
		pass

	class InvalidDialogOverlay(Exception):
		pass

	class DrawStepData(object):
		def __init__(self, isDefault, packFormat, function):
			self.isDefault = isDefault
			self.packFormat = packFormat
			self.parse = function

	BYTEORDER = '>' # big-endian format by default, platform independent

	TRUTH_VALUES = {"" : 0, "true" : 1, "false" : 0, "True" : 1, "False" : 0}

	BLOCK_HEADERS = {
		"bitmaps" 	: 0x0100000A,
		"fonts"		: 0x0100000B,
		"cursor"	: 0x0100000C,
		"drawdata"	: 0x0100000D,

		"globals"	: 0x0200000A,
		"dialog"	: 0x0200000B,
	}

	DIALOG_shading = {"none" : 0x0, "dim" : 0x1, "luminance" : 0x2}

	DS_triangleOrientations = {"top" : 0x1, "bottom" : 0x2, "left" : 0x3, "right" : 0x4}
	DS_fillModes = {"none" : 0x0, "foreground" : 0x1, "background" : 0x2, "gradient" : 0x3}
	DS_vectorAlign = {"left" : 0x1, "right" : 0x2, "bottom" : 0x3, "top" : 0x4, "center" : 0x5}

	DS_functions = {
		"void" : 0x0, "circle" : 0x1, "square" : 0x2, "roundedsq" : 0x3, "bevelsq" : 0x4,
   		"line" : 0x5, "triangle" : 0x6, "fill" : 0x7, "tab" : 0x8, "bitmap" : 0x9, "cross" : 0xA
	}

	DS_fonts = {
		"text_default" : 0x0, "text_hover" : 0x1, "text_disabled" : 0x2,
		"text_inverted" : 0x3, "text_button" : 0x4, "text_button_hover" : 0x5, "text_normal" : 0x6
	}

	DS_text_alignV = {"bottom" : 0x0, "center" : 0x1, "top" : 0x2}
	DS_text_alignH = {"left" : 0x0, "center" : 0x1, "right" : 0x2}

	DS_list = [
		"func", "fill", "stroke", "gradient_factor",
		"width", "height", "xpos", "ypos",
		"radius", "bevel", "shadow", "orientation", "file",
		"fg_color", "bg_color", "gradient_start", "gradient_end", "bevel_color"
	]

	def __init__(self, themeName, autoLoad = True, verbose = False):
		self._themeName = themeName
		self._stxFiles = []
		self._verbose = verbose

		self.DS_data = {
		#	attribute name						isDefault	pack	parse function
			"func"				: self.DrawStepData(False,	"B",	lambda f: self.DS_functions[f]),
			"fill"				: self.DrawStepData(True, 	"B",	lambda f: self.DS_fillModes[f]),
			"stroke" 			: self.DrawStepData(True,	"B",	int),
			"gradient_factor" 	: self.DrawStepData(True, 	"B",	int),
			"width"				: self.DrawStepData(False, 	"i", 	lambda w: -1 if w == 'height' else 0 if w == 'auto' else int(w)),
			"height"			: self.DrawStepData(False, 	"i",	lambda h: -1 if h == 'width' else 0 if h == 'auto' else int(h)),
			"xpos"				: self.DrawStepData(False,	"i",	lambda pos: self.DS_vectorAlign[pos] if pos in self.DS_vectorAlign else int(pos)),
			"ypos"				: self.DrawStepData(False,	"i", 	lambda pos: self.DS_vectorAlign[pos] if pos in self.DS_vectorAlign else int(pos)),
			"radius"			: self.DrawStepData(False,	"i",	lambda r: 0xFF if r == 'auto' else int(r)),
			"bevel" 			: self.DrawStepData(True, 	"B", 	int),
			"shadow" 			: self.DrawStepData(True, 	"B", 	int),
			"orientation"		: self.DrawStepData(False,	"B", 	lambda o: self.DS_triangleOrientations[o]),
			"file"				: self.DrawStepData(False,	"B",	self.__getBitmap),
			"fg_color"			: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"bg_color"			: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"gradient_start"	: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"gradient_end"		: self.DrawStepData(True, 	"4s",	self.__parseColor),
			"bevel_color"		: self.DrawStepData(True, 	"4s",	self.__parseColor)
		}

		if autoLoad:
			if not os.path.isdir(themeName) or not os.path.isfile(os.path.join(themeName, "THEMERC")):
				raise IOError

			for filename in os.listdir(themeName):
				filename = os.path.join(themeName, filename)
				if os.path.isfile(filename) and filename.endswith('.stx'):
					self._stxFiles.append(filename)

	def debug(self, text):
		if self._verbose: print (text)

	def debugBinary(self, data):
		if self._verbose:
			print ("BINARY OUTPUT (%d bytes): %s" % (len(data), " ".join(["%0.2X" % ord(c) for c in data])))

	def addSTXFile(self, filename):
		if not os.path.isfile(filename):
			raise IOError
		else:
			self._stxFiles.append(filename)

	def parse(self):
		if not self._stxFiles:
			self.debug("No files have been loaded for parsing on the theme.")
			raise IOError

		for f in self._stxFiles:
			self.debug("Parsing %s." % f)
			with open(f) as stxFile:
				self.__parseFile(stxFile)

	def __parseFile(self, xmlFile):
		stxDom = DOM.parse(xmlFile)

		for layout in stxDom.getElementsByTagName("layout_info"):
			self.__parseLayout(layout)

		for render in stxDom.getElementsByTagName("render_info"):
			self.__parseRender(render)

		stxDom.unlink()

	def __getBitmap(self, bmp):
		bmp = str(bmp)

		if bmp == "":
			return 0x0
		if bmp not in self._bitmaps:
			raise self.InvalidBitmapName

		return self._bitmaps[bmp]

	def __parseDrawStep(self, drawstepDom, localDefaults = {}):

		dstable = {}

		if drawstepDom.tagName == "defaults":
			isGlobal = drawstepDom.parentNode.tagName == "render_info"

			for ds in self.DS_list:
				if self.DS_data[ds].isDefault and drawstepDom.hasAttribute(ds):
					dstable[ds] = self.DS_data[ds].parse(drawstepDom.getAttribute(ds))

				elif isGlobal:
					dstable[ds] = None

		else:
			for ds in self.DS_data:
				if drawstepDom.hasAttribute(ds):
					dstable[ds] = self.DS_data[ds].parse(drawstepDom.getAttribute(ds))
				elif self.DS_data[ds].isDefault:
					dstable[ds] = localDefaults[ds] if ds in localDefaults else self._globalDefaults[ds]
				else:
					dstable[ds] = None

		return dstable


	def __parseDrawStepToBin(self, stepDict):
		"""
			/BBBBiiiiiBBB4s4s4s4s4sB/ ==
			function 		(byte)
			fill 			(byte)
			stroke			(byte)
			gradient_factor	(byte)
			width			(int32)
			height			(int32)
			xpos			(int32)
			ypos			(int32)
			radius			(int32)
			bevel			(byte)
			shadow			(byte)
			orientation 	(byte)
			file			(byte)
			fg_color		(4 byte)
			bg_color		(4 byte)
			gradient_start 	(4 byte)
			gradient_end	(4 byte)
			bevel_color		(4 byte)
		"""

		packLayout = ""
		packData = []

		for ds in self.DS_list:
			layout = self.DS_data[ds].packFormat
			data = stepDict[ds]

			if not data:
				size = struct.calcsize(layout)
				packLayout += "B" * size

				for d in range(size):
					packData.append(0)
			else:
				packLayout += layout
				packData.append(data)


		stepBin = struct.pack(self.BYTEORDER + packLayout, *tuple(packData))
		return stepBin


	def __parseResolutionToBin(self, resString):
		"""
			/B bHH bHH bHH/ == 1 byte + x * 9 bytes
			number of resolution sections (byte)
			exclude resolution (byte)
			resolution X (half)
			resolution Y (half)
		"""

		if resString == "":
			return struct.pack(self.BYTEORDER + "BbHH", 1, 0, 0, 0)

		resolutions = resString.split(", ")
		packFormat = "B" + "bHH" * len(resolutions)
		packData = [len(resolutions)]

		for res in resolutions:
			exclude = 0
			if res[0] == '-':
				exclude = 1
				res = res[1:]

			try:
				x, y = res.split('x')
				x = 0 if x == 'X' else int(x)
				y = 0 if y == 'Y' else int(y)
			except ValueError:
				raise InvalidResolution

			packData.append(exclude)
			packData.append(x)
			packData.append(y)

		buff = struct.pack(self.BYTEORDER + packFormat, *tuple(packData))
		return buff

	def __parseRGBToBin(self, color):
		"""
			/xBBB/ == 32 bits
			padding (byte)
			red color (byte)
			green color (byte)
			blue color (byte)
		"""

		try:
			rgb = tuple(map(int, color.split(", ")))
		except ValueError:
			raise self.InvalidRGBColor

		if len(rgb) != 3:
			raise self.InvalidRGBColor

		for c in rgb:
			if c < 0 or c > 255:
				raise self.InvalidRGBColor

		rgb = struct.pack(self.BYTEORDER + "xBBB", *tuple(rgb))

#		self.debugBinary(rgb)
		return rgb

	def __parseColor(self, color):
		try:
			color = self.__parseRGBToBin(color)
		except self.InvalidRGBColor:
			if color not in self._colors:
				raise self.InvalidRGBColor
			color = self._colors[color]

		return color


	def __parsePalette(self, paletteDom):
		self._colors = {}

		for color in paletteDom.getElementsByTagName("color"):
			color_name = color.getAttribute('name')
			color_rgb = self.__parseRGBToBin(color.getAttribute('rgb'))

			self._colors[color_name] = color_rgb
#			self.debug("COLOR: %s" % (color_name))


	def __parseBitmaps(self, bitmapsDom):
		self._bitmaps = {}
		idCount = 0xA0
		packLayout = ""
		packData = []

		for bitmap in bitmapsDom.getElementsByTagName("bitmap"):
			bmpName = str(bitmap.getAttribute("filename"))
			self._bitmaps[bmpName] = idCount

			packLayout += "B%ds" % (len(bmpName) + 1)
			packData.append(idCount)
			packData.append(bmpName)
			idCount += 1


		bitmapBinary = struct.pack(
			self.BYTEORDER + "IB" + packLayout,
			self.BLOCK_HEADERS['bitmaps'],
			len(bitmapsDom.getElementsByTagName("bitmap")),
			*tuple(packData)
		)

#		self.debug("BITMAPS:\n%s\n\n" % pbin(bitmapBinary))
		return bitmapBinary

	def __parseFonts(self, fontsDom):
		"""
			/IB Bs4ss .../
			section header (uint32)
			number of font definitions (byte)

			id for font definition (byte)
			resolution for font (byte array)
			color for font (4 bytes)
			font filename (byte array, null terminated)
		"""

		packLayout = ""
		packData = []

		for font in fontsDom.getElementsByTagName("font"):
			ident = font.getAttribute("id")

			if ident not in self.DS_fonts:
				raise self.InvalidFontIdentifier

			color = self.__parseColor(font.getAttribute("color"))
			filename = str(font.getAttribute("file"))

			if filename == 'default':
				filename = ''

			resolution = self.__parseResolutionToBin(font.getAttribute("resolution"))

			packLayout += "B%ds4s%ds" % (len(resolution), len(filename) + 1)
			packData.append(self.DS_fonts[ident])
			packData.append(resolution)
			packData.append(color)
			packData.append(filename)

		fontsBinary = struct.pack(self.BYTEORDER + \
			"IB" + packLayout,
			self.BLOCK_HEADERS['fonts'],
			len(fontsDom.getElementsByTagName("font")),
			*tuple(packData)
		)


#		self.debug("FONTS DATA:\n%s\n\n" % pbin(fontsBinary))
		return fontsBinary

	def __parseTextToBin(self, textDom):
		"""
			/BBBx/
			font identifier (byte)
			vertical alignment (byte)
			horizontal alignment (byte)
			padding until word (byte)
		"""

		font = textDom.getAttribute("font")
		if font not in self.DS_fonts:
			raise self.InvalidFontIdentifier

		textBin = struct.pack(self.BYTEORDER + "BBBx",
			self.DS_fonts[font],
			self.DS_text_alignV[textDom.getAttribute("vertical_align")],
			self.DS_text_alignH[textDom.getAttribute("horizontal_align")]
		)

		return textBin

	def __parseDrawData(self, ddDom):
		"""
			/IsIBBHss/
				Section Header (uint32)
				Resolution (byte array, word-aligned)
				DrawData id hash (uint32)
				Cached (byte)
				has text section? (byte)
				number of DD sections (uint16)
				** text segment (4 bytes)
				drawstep segments (byte array)
		"""


		localDefaults = ddDom.getElementsByTagName("defaults")
		localDefaults = localDefaults[0] if localDefaults else {}

		stepList = []

		for ds in ddDom.getElementsByTagName("drawstep"):
			dstable = self.__parseDrawStep(ds, localDefaults)
			dsbinary = self.__parseDrawStepToBin(dstable)

			stepList.append(dsbinary)

		stepByteArray = "".join(stepList)

		resolution = self.__parseResolutionToBin(ddDom.getAttribute("resolution"))

		text = ddDom.getElementsByTagName("text")
		text = self.__parseTextToBin(text[0]) if text else ""

		id_hash = str.__hash__(str(ddDom.getAttribute("id"))) & 0xFFFFFFFF
		cached = self.TRUTH_VALUES[ddDom.getAttribute("cached")]

		ddBinary = struct.pack(self.BYTEORDER + \
			"I%dsIBBH4s%ds" % (len(resolution), len(stepByteArray)),

			self.BLOCK_HEADERS['drawdata'], # Section Header (uint32)
			resolution,						# Resolution (byte array, word-aligned)
			id_hash,						# DrawData id hash (uint32)
			cached,							# Cached (byte)
			0x1 if text else 0x0,			# has text section? (byte)
			len(stepList),					# number of DD sections (uint16)
			text,							# ** text segment (byte array)
			stepByteArray					# drawstep segments (byte array)
		)

#		self.debug("DRAW DATA %s (%X): \n" % (ddDom.getAttribute("id"), id_hash) + pbin(ddBinary) + "\n\n")
		return ddBinary

	def __parseCursor(self, cursorDom):
		"""
			/IsBBhh/
			section header (uint32)
			resolution string (byte array)
			bitmap id (byte)
			scale (byte)
			hotspot X (half)
			hotspot Y (half)
		"""

		resolution = self.__parseResolutionToBin(cursorDom.getAttribute("resolution"))
		scale = int(cursorDom.getAttribute("scale"))
		hsX, hsY = cursorDom.getAttribute("hotspot").split(", ")

		cursorBin = struct.pack(self.BYTEORDER + "I%dsBBhh" % len(resolution),
			self.BLOCK_HEADERS['cursor'],
			resolution,
			self.__getBitmap(cursorDom.getAttribute("file")),
			scale,
			int(hsX),
			int(hsY)
		)

#		self.debug("CURSOR:\n%s\n\n" % pbin(cursorBin))
		return cursorBin

	def __parseDialog(self, dialogDom):

		dialog_id = str(dialogDom.getAttribute("name"))
		resolution = self.__parseResolutionToBin(dialogDom.getAttribute("resolution"))

		overlays = str(dialogDom.getAttribute("overlays"))
		overlay_type = 0x0
		overlay_parent = ""

		if overlays == "screen":
			overlay_type = 0x1
		elif overlays == "screen_center":
			overlay_type = 0x2
		else:
			overlay_type = 0x3
			overlay_parent = str(overlays)

		dialog_enabled = 0x1
		if dialogDom.hasAttribute("enabled"):
			dialog_enabled = self.TRUTH_VALUES[dialogDom.getAttribute("enabled")]

		dialog_shading = 0x0
		if dialogDom.hasAttribute("shading"):
			dialog_shading = self.DIALOG_shading[dialogDom.getAttribute("shading")]

		dialog_inset = 0
		if dialogDom.hasAttribute("inset"):
			dialog_inset = int(dialogDom.getAttribute("inset"))

		dialogBin = struct.pack(self.BYTEORDER + \
			"I%ds%dsBBBB%ds" % (len(resolution), len(dialog_id) + 1, len(overlay_parent) + 1),
			self.BLOCK_HEADERS['dialog'],
			resolution,
			dialog_id,
			dialog_enabled,
			dialog_shading,
			dialog_inset,
			overlay_type,
			overlay_parent,
		)

		return dialogBin

	def __parseLayout(self, layoutDom):
		self.debug("GLOBAL SECTION: LAYOUT INFO.")

		dialogBIN = ""

		for dialog in layoutDom.getElementsByTagName("dialog"):
			dialogBIN += self.__parseDialog(dialog)


		printBinaryDump(dialogBIN)

		return dialogBIN

	def __parseRender(self, renderDom):
		self.debug("GLOBAL SECTION: RENDER INFO.")

		bitmapBIN = ""
		fontsBIN = ""
		cursorBIN 	= ""
		drawdataBIN = ""

		# parse color palettes
		paletteDom = renderDom.getElementsByTagName("palette")
		if paletteDom:
			self.__parsePalette(paletteDom[0])

		# parse bitmaps
		bitmapsDom = renderDom.getElementsByTagName("bitmaps")
		if bitmapsDom:
			bitmapBIN = self.__parseBitmaps(bitmapsDom[0])

		# parse fonts
		fontsDom = renderDom.getElementsByTagName("fonts")[0]
		fontsBIN = self.__parseFonts(fontsDom)

		# parse defaults
		defaultsDom = renderDom.getElementsByTagName("defaults")
		if defaultsDom:
			self._globalDefaults = self.__parseDrawStep(defaultsDom[0])
		else:
			self._globalDefaults = {}

		# parse cursors
		for cur in renderDom.getElementsByTagName("cursor"):
			cursorBIN += self.__parseCursor(cur)

		# parse drawdata sets
		for dd in renderDom.getElementsByTagName("drawdata"):
			drawdataBIN += self.__parseDrawData(dd)


		renderInfoBIN = bitmapBIN + fontsBIN + cursorBIN + drawdataBIN
		printBinaryDump(renderInfoBIN)

		return renderInfoBIN

if __name__ == '__main__':
	bin = STXBinaryFile('../gui/themes/scummclassic', True, True)
	bin.parse()
