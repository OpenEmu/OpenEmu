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

#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

struct BdfBoundingBox {
	int width, height;
	int xOffset, yOffset;
};

struct BdfFont {
	int maxAdvance;
	int height;
	BdfBoundingBox defaultBox;
	int ascent;

	int firstCharacter;
	int defaultCharacter;
	int numCharacters;

	unsigned char **bitmaps;
	unsigned char *advances;
	BdfBoundingBox *boxes;

	BdfFont() : bitmaps(0), advances(0), boxes(0) {
	}

	~BdfFont() {
		if (bitmaps) {
			for (int i = 0; i < numCharacters; ++i)
				delete[] bitmaps[i];
		}
		delete[] bitmaps;
		delete[] advances;
		delete[] boxes;
	}
};

void error(const char *s, ...) {
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, 1024, s, va);
	va_end(va);

	fprintf(stderr, "ERROR: %s!\n", buf);
	exit(1);
}

void warning(const char *s, ...) {
	char buf[1024];
	va_list va;

	va_start(va, s);
	vsnprintf(buf, 1024, s, va);
	va_end(va);

	fprintf(stderr, "WARNING: %s!\n", buf);
}

bool hasPrefix(const std::string &str, const std::string &prefix) {
	return str.compare(0, prefix.size(), prefix) == 0;
}

inline void hexToInt(unsigned char &h) {
	if (h >= '0' && h <= '9')
		h -= '0';
	else if (h >= 'A' && h <= 'F')
		h -= 'A' - 10;
	else if (h >= 'a' && h <= 'f')
		h -= 'a' - 10;
	else
		h = 0;
}

int main(int argc, char *argv[]) {
	if (argc != 3) {
		printf("Usage: convbdf [input] [fontname]\n");
		return 1;
	}

	std::ifstream in(argv[1]);
	std::string line;

	std::getline(in, line);
	if (in.fail() || in.eof())
		error("Premature end of file");

	int verMajor, verMinor;
	if (sscanf(line.c_str(), "STARTFONT %d.%d", &verMajor, &verMinor) != 2)
		error("File '%s' is no BDF file", argv[1]);

	if (verMajor != 2 || (verMinor != 1 && verMinor != 2))
		error("File '%s' does not use a supported BDF version (%d.%d)", argv[1], verMajor, verMinor);

	std::string fontName;
	std::string copyright;
	BdfFont font;
	memset(&font, 0, sizeof(font));
	font.ascent = -1;
	font.defaultCharacter = -1;

	int charsProcessed = 0, charsAvailable = 0, descent = -1;

	while (true) {
		std::getline(in, line);
		if (in.fail() || in.eof())
			error("Premature end of file");

		if (hasPrefix(line, "SIZE ")) {
			// Ignore
		} else if (hasPrefix(line, "FONT ")) {
			fontName = line.substr(5);
		} else if (hasPrefix(line, "COPYRIGHT ")) {
			copyright = line.substr(10);
		} else if (hasPrefix(line, "COMMENT ")) {
			// Ignore
		} else if (hasPrefix(line, "FONTBOUNDINGBOX ")) {
			if (sscanf(line.c_str(), "FONTBOUNDINGBOX %d %d %d %d",
			           &font.defaultBox.width, &font.defaultBox.height,
			           &font.defaultBox.xOffset, &font.defaultBox.yOffset) != 4)
				error("Invalid FONTBOUNDINGBOX");
		} else if (hasPrefix(line, "CHARS ")) {
			if (sscanf(line.c_str(), "CHARS %d", &charsAvailable) != 1)
				error("Invalid CHARS");

			font.numCharacters = 256;
			font.bitmaps = new unsigned char *[font.numCharacters];
			memset(font.bitmaps, 0, sizeof(unsigned char *) * font.numCharacters);
			font.advances = new unsigned char[font.numCharacters];
			font.boxes = new BdfBoundingBox[font.numCharacters];
		} else if (hasPrefix(line, "FONT_ASCENT ")) {
			if (sscanf(line.c_str(), "FONT_ASCENT %d", &font.ascent) != 1)
				error("Invalid FONT_ASCENT");
		} else if (hasPrefix(line, "FONT_DESCENT ")) {
			if (sscanf(line.c_str(), "FONT_DESCENT %d", &descent) != 1)
				error("Invalid FONT_ASCENT");
		} else if (hasPrefix(line, "DEFAULT_CHAR ")) {
			if (sscanf(line.c_str(), "DEFAULT_CHAR %d", &font.defaultCharacter) != 1)
				error("Invalid DEFAULT_CHAR");
		} else if (hasPrefix(line, "STARTCHAR ")) {
			++charsProcessed;
			if (charsProcessed > charsAvailable)
				error("Too many characters defined (only %d specified, but %d existing already)",
				      charsAvailable, charsProcessed);

			bool hasWidth = false, hasBitmap = false;

			int encoding = -1;
			int xAdvance;
			unsigned char *bitmap = 0;
			BdfBoundingBox bbox = font.defaultBox;

			while (true) {
				std::getline(in, line);
				if (in.fail() || in.eof())
					error("Premature end of file");

				if (hasPrefix(line, "ENCODING ")) {
					if (sscanf(line.c_str(), "ENCODING %d", &encoding) != 1)
						error("Invalid ENCODING");
				} else if (hasPrefix(line, "DWIDTH ")) {
					int yAdvance;
					if (sscanf(line.c_str(), "DWIDTH %d %d", &xAdvance, &yAdvance) != 2)
						error("Invalid DWIDTH");
					if (yAdvance != 0)
						error("Character %d uses a DWIDTH y advance of %d", encoding, yAdvance);
					if (xAdvance < 0)
						error("Character %d has a negative x advance", encoding);

					if (xAdvance > font.maxAdvance)
						font.maxAdvance = xAdvance;

					hasWidth = true;
				} else if (hasPrefix(line, "BBX" )) {
					if (sscanf(line.c_str(), "BBX %d %d %d %d",
					           &bbox.width, &bbox.height,
					           &bbox.xOffset, &bbox.yOffset) != 4)
						error("Invalid BBX");
				} else if (line == "BITMAP") {
					hasBitmap = true;

					const size_t bytesPerRow = ((bbox.width + 7) / 8);

					// Since we do not load all characters, we only create a
					// buffer for the ones we actually load.
					if (encoding < font.numCharacters)
						bitmap = new unsigned char[bbox.height * bytesPerRow];

					for (int i = 0; i < bbox.height; ++i) {
						std::getline(in, line);
						if (in.fail() || in.eof())
							error("Premature end of file");
						if (line.size() != bytesPerRow * 2)
							error("Glyph bitmap line too short");

						if (!bitmap)
							continue;

						for (size_t j = 0; j < bytesPerRow; ++j) {
							unsigned char nibble1 = line[j * 2 + 0];
							hexToInt(nibble1);
							unsigned char nibble2 = line[j * 2 + 1];
							hexToInt(nibble2);
							bitmap[i * bytesPerRow + j] = (nibble1 << 4) | nibble2;
						}
					}
				} else if (line == "ENDCHAR") {
					if (encoding == -1 || !hasWidth || !hasBitmap)
						error("Character not completly defined");

					if (encoding < font.numCharacters) {
						font.advances[encoding] = xAdvance;
						font.boxes[encoding] = bbox;
						font.bitmaps[encoding] = bitmap;
					}
					break;
				}
			}
		} else if (line == "ENDFONT") {
			break;
		} else {
			// Silently ignore other declarations
			//warning("Unsupported declaration: \"%s\"", line.c_str());
		}
	}

	if (font.ascent < 0)
		error("No ascent specified");
	if (descent < 0)
		error("No descent specified");

	font.height = font.ascent + descent;

	int firstCharacter = font.numCharacters;
	int lastCharacter = -1;
	bool hasFixedBBox = true;
	bool hasFixedAdvance = true;

	for (int i = 0; i < font.numCharacters; ++i) {
		if (!font.bitmaps[i])
			continue;

		if (i < firstCharacter)
			firstCharacter = i;

		if (i > lastCharacter)
			lastCharacter = i;

		if (font.advances[i] != font.maxAdvance)
			hasFixedAdvance = false;

		const BdfBoundingBox &bbox = font.boxes[i];
		if (bbox.width != font.defaultBox.width
		    || bbox.height != font.defaultBox.height
		    || bbox.xOffset != font.defaultBox.xOffset
		    || bbox.yOffset != font.defaultBox.yOffset)
			hasFixedBBox = false;
	}

	if (lastCharacter == -1)
		error("No glyphs found");

	// Free the advance table, in case all glyphs use the same advance
	if (hasFixedAdvance) {
		delete[] font.advances;
		font.advances = 0;
	}

	// Free the box table, in case all glyphs use the same box
	if (hasFixedBBox) {
		delete[] font.boxes;
		font.boxes = 0;
	}

	// Adapt for the fact that we never use encoding 0.
	if (font.defaultCharacter < firstCharacter
	    || font.defaultCharacter > lastCharacter)
		font.defaultCharacter = -1;

	font.firstCharacter = firstCharacter;

	charsAvailable = lastCharacter - firstCharacter + 1;
	// Try to compact the tables
	if (charsAvailable < font.numCharacters) {
		unsigned char **bitmaps = new unsigned char *[charsAvailable];
		BdfBoundingBox *boxes = 0;
		if (!hasFixedBBox)
			boxes = new BdfBoundingBox[charsAvailable];
		unsigned char *advances = 0;
		if (!hasFixedAdvance)
			advances = new unsigned char[charsAvailable];

		for (int i = 0; i < charsAvailable; ++i) {
			const int encoding = i + firstCharacter;
			if (font.bitmaps[encoding]) {
				bitmaps[i] = font.bitmaps[encoding];

				if (!hasFixedBBox)
					boxes[i] = font.boxes[encoding];
				if (!hasFixedAdvance)
					advances[i] = font.advances[encoding];
			} else {
				bitmaps[i] = 0;
			}
		}

		delete[] font.bitmaps;
		font.bitmaps = bitmaps;
		delete[] font.advances;
		font.advances = advances;
		delete[] font.boxes;
		font.boxes = boxes;

		font.numCharacters = charsAvailable;
	}

	char dateBuffer[256];
	time_t curTime = time(0);
	snprintf(dateBuffer, sizeof(dateBuffer), "%s", ctime(&curTime));

	// Finally output the cpp source file to stdout
	printf("// Generated by convbdf on %s"
	       "#include \"graphics/fonts/bdf.h\"\n"
	       "\n"
	       "// Font information:\n"
	       "//  Name: %s\n"
	       "//  Size: %dx%d\n"
	       "//  Box: %d %d %d %d\n"
	       "//  Ascent: %d\n"
	       "//  First character: %d\n"
	       "//  Default character: %d\n"
	       "//  Characters: %d\n"
	       "//  Copyright: %s\n"
	       "\n",
	       dateBuffer, fontName.c_str(), font.maxAdvance, font.height,
	       font.defaultBox.width, font.defaultBox.height, font.defaultBox.xOffset,
	       font.defaultBox.yOffset, font.ascent, font.firstCharacter,
	       font.defaultCharacter, font.numCharacters, copyright.c_str());

	printf("namespace Graphics {\n"
	       "\n");

	for (int i = 0; i < font.numCharacters; ++i) {
		if (!font.bitmaps[i])
			continue;

		BdfBoundingBox box = hasFixedBBox ? font.defaultBox : font.boxes[i];
		printf("// Character %d (0x%02X)\n"
		       "// Box: %d %d %d %d\n"
		       "// Advance: %d\n"
		       "//\n", i + font.firstCharacter, i + font.firstCharacter,
		       box.width, box.height, box.xOffset, box.yOffset,
		       hasFixedAdvance ? font.maxAdvance : font.advances[i]);

		printf("// +");
		for (int x = 0; x < box.width; ++x)
			printf("-");
		printf("+\n");

		const unsigned char *bitmap = font.bitmaps[i];

		for (int y = 0; y < box.height; ++y) {
			printf("// |");
			unsigned char data;
			for (int x = 0; x < box.width; ++x) {
				if (!(x % 8))
					data = *bitmap++;

				printf("%c", (data & 0x80) ? '*' : ' ');
				data <<= 1;
			}
			printf("|\n");
		}

		printf("// +");
		for (int x = 0; x < box.width; ++x)
			printf("-");
		printf("+\n");

		const int bytesPerRow = (box.width + 7) / 8;
		bitmap = font.bitmaps[i];
		printf("static const byte glyph%d[] = {\n", i);
		for (int y = 0; y < box.height; ++y) {
			printf("\t");

			for (int x = 0; x < bytesPerRow; ++x) {
				if (x)
					printf(", ");
				printf("0x%02X", *bitmap++);
			}

			if (y != box.height - 1)
				printf(",");
			printf("\n");
		}
		printf("};\n"
		       "\n");
	}

	printf("// Bitmap pointer table\n"
	       "const byte *const bitmapTable[] = {\n");
	for (int i = 0; i < font.numCharacters; ++i) {
		if (font.bitmaps[i])
			printf("\tglyph%d", i);
		else
			printf("\t0");

		if (i != font.numCharacters - 1)
			printf(",");
		printf("\n");
	}
	printf("};\n"
	       "\n");

	if (!hasFixedAdvance) {
		printf("// Advance table\n"
		       "static const byte advances[] = {\n");
		for (int i = 0; i < font.numCharacters; ++i) {
			if (font.bitmaps[i])
				printf("\t%d", font.advances[i]);
			else
				printf("\t0");

			if (i != font.numCharacters - 1)
				printf(",");
			printf("\n");
		}
		printf("};\n"
		       "\n");
	}

	if (!hasFixedBBox) {
		printf("// Bounding box table\n"
		       "static const BdfBoundingBox boxes[] = {\n");
		for (int i = 0; i < font.numCharacters; ++i) {
			if (font.bitmaps[i]) {
				const BdfBoundingBox &box = font.boxes[i];
				printf("\t{ %d, %d, %d, %d }", box.width, box.height, box.xOffset, box.yOffset);
			} else {
				printf("\t{ 0, 0, 0, 0 }");
			}

			if (i != font.numCharacters - 1)
				printf(",");
			printf("\n");
		}
		printf("};\n"
		       "\n");
	}

	printf("// Font structure\n"
	       "static const BdfFontData desc = {\n"
	       "\t%d, // Max advance\n"
	       "\t%d, // Height\n"
	       "\t{ %d, %d, %d, %d }, // Bounding box\n"
	       "\t%d, // Ascent\n"
	       "\n"
	       "\t%d, // First character\n"
	       "\t%d, // Default character\n"
	       "\t%d, // Characters\n"
	       "\n"
	       "\tbitmapTable, // Bitmaps\n",
	       font.maxAdvance, font.height, font.defaultBox.width,
	       font.defaultBox.height, font.defaultBox.xOffset, font.defaultBox.yOffset,
	       font.ascent, font.firstCharacter, font.defaultCharacter, font.numCharacters);

	if (hasFixedAdvance)
		printf("\t0, // Advances\n");
	else
		printf("\tadvances, // Advances\n");

	if (hasFixedBBox)
		printf("\t0 // Boxes\n");
	else
		printf("\tboxes // Boxes\n");

	printf("};\n"
	       "\n"
	       "DEFINE_FONT(%s)\n"
	       "\n"
	       "} // End of namespace Graphics\n",
	       argv[2]);
}
