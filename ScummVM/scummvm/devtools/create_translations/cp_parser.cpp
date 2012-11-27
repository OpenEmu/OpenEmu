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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * This is a utility for create the translations.dat file from all the po files.
 * The generated files is used by ScummVM to propose translation of its GUI.
 */

#include "cp_parser.h"

#include <fstream>
#include <stdio.h>

Codepage::Codepage(const std::string &name, const uint32 *mapping) : _name(name) {
	if (!mapping) {
		// Default to a ISO-8859-1 mapping
		for (unsigned int i = 0; i < 256; ++i)
			_mapping[i] = i;
	} else {
		for (unsigned int i = 0; i < 256; ++i)
			_mapping[i] = *mapping++;
	}
}

Codepage *parseCodepageMapping(const std::string &filename) {
	size_t start = filename.find_last_of("/\\");
	if (start == std::string::npos)
		start = 0;
	else
		++start;
	// Strip off the filename extension
	const size_t pos = filename.find_last_of('.');
	const std::string charset(filename.substr(start, pos != std::string::npos ? (pos - start) : std::string::npos));

	std::ifstream in(filename.c_str());
	if (!in) {
		fprintf(stderr, "ERROR: Could not open file \"%s\"!", filename.c_str());
		return 0;
	}

	uint32 mapping[256];

	int processedMappings = 0;
	std::string line;
	while (processedMappings < 256) {
		std::getline(in, line);
		if (in.eof())
			break;
		if (in.fail()) {
			fprintf(stderr, "ERROR: Reading from file \"%s\" failed!", filename.c_str());
			return 0;
		}

		// In case the line starts with a "#" it is a comment. We also ignore
		// empty lines.
		if (line.empty() || line[0] == '#')
			continue;

		// Read the unicode number, we thereby ignore everything else on the line
		int unicode, required;
		const int parsed = sscanf(line.c_str(), "%d %d", &unicode, &required);
		if (parsed < 1 || parsed > 2) {
			fprintf(stderr, "ERROR: Line \"%s\" is malformed!", filename.c_str());
			return 0;
		}
		// In case no required integer was given, we assume the character is
		// required
		if (parsed == 1)
			required = 1;

		// -1 means default mapping
		if (unicode == -1)
			unicode = processedMappings;

		uint32 unicodeValue = unicode;
		if (required)
			unicodeValue |= 0x80000000;

		mapping[processedMappings++] = (uint32)unicodeValue;
	}

	// Check whether all character encodings are mapped, if not error out
	if (processedMappings != 256) {
		fprintf(stderr, "ERROR: File \"%s\" does not contain mappings for exactly 256 characters!", filename.c_str());
		return 0;
	}

	// Return the codepage
	return new Codepage(charset, mapping);
}
