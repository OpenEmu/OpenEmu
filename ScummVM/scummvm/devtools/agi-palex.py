#!/usr/bin/python
# Amiga AGI game palette extractor.
# Extracts palette from an Amiga AGI game's executable file.
# Initial version written during summer of 2007 by Buddha^.
# Somewhat optimized. Adding the preliminary palette test helped speed a lot.
# FIXME: Doesn't report anything about not found files when some files are found.
#        An example: palex.py SQ2 PQ1 (When file SQ2 exists but PQ1 doesn't)
import struct, sys, os.path, glob

# Constants
maxComponentValue     = 0xF
colorsPerPalette      = 16
componentsPerColor    = 3
bytesPerComponent     = 2
bytesPerColor         = componentsPerColor   * bytesPerComponent
componentsPerPalette  = colorsPerPalette     * componentsPerColor
bytesPerPalette       = componentsPerPalette * bytesPerComponent
encodedBlack          = '\x00' * bytesPerColor
encodedWhite          = (('\x00' * (bytesPerComponent - 1)) + ("%c" % maxComponentValue)) * componentsPerColor
decodedBlack          = tuple([0 for x in range(componentsPerColor)])
decodedWhite          = tuple([maxComponentValue for x in range(componentsPerColor)])
blackColorNum         = 0
whiteColorNum         = colorsPerPalette - 1
encodedBlackStart     = blackColorNum * bytesPerColor
encodedBlackEnd       = encodedBlackStart + bytesPerColor
encodedWhiteStart     = whiteColorNum * bytesPerColor
encodedWhiteEnd       = encodedWhiteStart + bytesPerColor
componentPrintFormat  = "0x%1X"
arraynamePrefix       = "amigaPalette"

def isColor12Bit(color):
	"""Is the color 12-bit (i.e. 4 bits per color component)?"""
	for component in color:
		if not (0 <= component <= maxComponentValue):
			return False
	return True

def printCommentLineList(lines):
	"""Prints list of lines inside a comment"""
	if len(lines) > 0:
		if len(lines) == 1:
			print "// " + lines[0]
		else:
			print "/**"
			for line in lines:
				print " * " + line
			print " */"

def printColor(color, tabulate = True, printLastComma = True, newLine = True):
	"""Prints color with optional start tabulation, comma in the end and a newline"""
	result = ""
	if tabulate:
		result += "\t"
	for component in color[:-1]:
		result += ((componentPrintFormat + ", ") % component)
	result += (componentPrintFormat % color[-1])
	if printLastComma:
		result += ","
	if newLine:
		print result
	else:
		print result,

def printPalette(palette, filename, arrayname):
	"""Prints out palette as a C-style array"""
	# Print comments about the palette
	comments = ["A 16-color, 12-bit RGB palette from an Amiga AGI game."]
	comments.append("Extracted from file " + os.path.basename(filename))
	printCommentLineList(comments)

	# Print the palette as a C-style array
	print "static const unsigned char " + arrayname + "[] = {"
	for color in palette[:-1]:
		printColor(color)
	printColor(palette[-1], printLastComma = False)
	print("};")

def isAmigaPalette(palette):
	"""Test if the given palette is an Amiga-style palette"""
	# Palette must be of correct size
	if len(palette) != colorsPerPalette:
		return False

	# First palette color must be black and last palette color must be black
	if palette[whiteColorNum] != decodedWhite or palette[blackColorNum] != decodedBlack:
		return False

	# All colors must be 12-bit (i.e. 4 bits per color component)
	for color in palette:
		if not isColor12Bit(color):
			return False

	# All colors must be unique
	if len(set(palette)) != colorsPerPalette:
		return False

	return True

def preliminaryPaletteTest(data, pos):
	"""Preliminary test for a palette (For speed's sake)."""
	# Test that palette's last color is white
	if data[pos + encodedWhiteStart : pos + encodedWhiteEnd] != encodedWhite:
		return False
	# Test that palette's first color is black
	if data[pos + encodedBlackStart : pos + encodedBlackEnd] != encodedBlack:
		return False
	return True

def searchForAmigaPalettes(filename):
	"""Search file for Amiga AGI game palettes and return any found unique palettes"""
	try:
		file = None
		foundPalettes = []
		# Open file and read it into memory
		file = open(filename, 'rb')
		data = file.read()
		palette = [decodedBlack for x in range(colorsPerPalette)]
		# Search through the whole file
		for searchPosition in range(len(data) - bytesPerPalette + 1):
			if preliminaryPaletteTest(data, searchPosition):
				# Parse possible palette from byte data
				for colorNum in range(colorsPerPalette):
					colorStart = searchPosition + colorNum * bytesPerColor
					colorEnd   = colorStart + bytesPerColor
					# Parse color components as unsigned 16-bit big endian integers
					color = struct.unpack('>' + 'H' * componentsPerColor, data[colorStart:colorEnd])
					palette[colorNum] = color
				# Save good candidates to a list
				if isAmigaPalette(palette):
					foundPalettes.append(tuple(palette))
		# Close source file and return unique found palettes
		file.close()
		return set(foundPalettes)
	except IOError:
		if file != None:
			file.close()
		return None

# The main program starts here
if len(sys.argv) < 2 or sys.argv[1] == "-h" or sys.argv[1] == "--help":
	quit("Usage: " + os.path.basename(sys.argv[0]) + " FILE [[FILE] ... [FILE]]\n" \
		"  Searches all FILE parameters for Amiga AGI game palettes\n" \
		"  and prints out any found candidates as C-style arrays\n" \
		"  with sequentially numbered names (" + arraynamePrefix + "1, " + arraynamePrefix + "2 etc).\n" \
		"  Supports wildcards.")

# Get the list of filenames (Works with wildcards too)
filenameList = []
for parameter in sys.argv[1:]:
	filenameList.extend(glob.glob(parameter))

# Go through all the files and search for palettes
totalPalettesCount = 0
if len(filenameList) > 0:
	negativeFiles = []
	errorFiles = []
	for filename in filenameList:
		foundPalettes = searchForAmigaPalettes(filename)
		if foundPalettes == None:
			errorFiles.append(filename)
		elif len(foundPalettes) == 0:
			negativeFiles.append(filename)
		else:
			# Print out the found palettes
			for palette in foundPalettes:
				# Print palettes with sequentially numbered array names
				totalPalettesCount = totalPalettesCount + 1
				printPalette(palette, filename, arraynamePrefix + str(totalPalettesCount))
				print "" # Print an extra newline to separate things
	# Print comment about files we couldn't find any palettes in
	if len(negativeFiles) > 0:
		comments = []
		comments.append("Couldn't find any palettes in the following files:")
		comments.extend(negativeFiles)
		printCommentLineList(comments)
		print "" # Print an extra newline to separate things
	# Print comment about errors handling files
	if len(errorFiles) > 0:
		comments = []
		comments.append("Error handling the following files:")
		comments.extend(errorFiles)
		printCommentLineList(comments)
		print "" # Print an extra newline to separate things
else:
	printCommentLineList(["No files found"])
