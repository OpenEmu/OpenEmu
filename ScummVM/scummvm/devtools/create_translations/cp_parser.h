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

#ifndef CP_PARSER_H
#define CP_PARSER_H

#include "create_translations.h"

#include <string>

/**
 * Codepage description.
 *
 * This includes a name, and the codepage -> unicode mapping.
 */
class Codepage {
public:
	Codepage(const std::string &name, const uint32 *mapping);

	const std::string &getName() const { return _name; }

	uint32 getMapping(unsigned char src) const { return _mapping[src]; }
private:
	std::string _name;
	uint32 _mapping[256];
};

/**
 * Parse the codepage file and create a codepage.
 */
Codepage *parseCodepageMapping(const std::string &filename);

#endif
