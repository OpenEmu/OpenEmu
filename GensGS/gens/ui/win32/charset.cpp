/***************************************************************************
 * Gens: Character set conversion functions.                               *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "charset.hpp"

// C includes
#include <cstring>
#include <cstdlib>

// C++ includes
#include <string>

#include "macros/hashtable.hpp"
#include <utility>

using std::string;

// Character set arrays.

// CP1252 to UTF-8 mapping.
// Only used for characters 0x80 - 0x9F.
typedef std::pair<unsigned int, unsigned char> charConversion;
typedef GENS_HASHTABLE<unsigned int, unsigned char> charMap;
charMap charset_map_utf8_to_cp1252;

static const unsigned short charset_map_utf8_to_cp1252_array[][2] =
{
	{0x20AC, 0x80},
	{0x201A, 0x82},
	{0x0192, 0x83},
	{0x201E, 0x84},
	{0x2026, 0x85},
	{0x2020, 0x86},
	{0x2021, 0x87},
	{0x02C6, 0x88},
	{0x2030, 0x89},
	{0x0160, 0x8A},
	{0x2039, 0x8B},
	{0x0152, 0x8C},
	{0x017D, 0x8E},
	{0x2018, 0x91},
	{0x2019, 0x92},
	{0x201C, 0x93},
	{0x201D, 0x94},
	{0x2022, 0x95},
	{0x2013, 0x96},
	{0x2014, 0x97},
	{0x02DC, 0x98},
	{0x2122, 0x99},
	{0x0161, 0x9A},
	{0x203A, 0x9B},
	{0x0153, 0x9C},
	{0x017E, 0x9E},
	{0x0178, 0x9F},
	{NULL, NULL},
};

/**
 * charset_utf8_to_cp1252(): Convert a UTF-8 string to cp1252.
 * @param s_utf8 UTF-8 string.
 * @param replacementChar Replacement character for symbols not available in cp1252.
 * @return cp1252 string.
 */
string charset_utf8_to_cp1252(const char* s_utf8, const char replacementChar)
{
	int sLen = strlen(s_utf8);
	unsigned int utf8char;
	unsigned char cp1252char;
	
	// Output buffer.
	unsigned char *outBuf = static_cast<unsigned char*>(malloc(sLen + 1));
	int outBufPos = 0;
	outBuf[0] = 0;
	
	for (int i = 0; i < sLen; i++)
	{
		if (!(s_utf8[i] & 0x80))
		{
			// 0x00 - 0x7F - regular ASCII character, same as cp1252.
			utf8char = s_utf8[i];
		}
		else
		{
			// Multi-byte character.
			
			// Check for 2-byte character.
			if ((i + 1 < sLen) &&
			    ((s_utf8[i] & 0xE0) == 0xC0) &&
			    ((s_utf8[i + 1] & 0xC0) == 0x80))
			{
				// 2-byte character.
				utf8char = ((s_utf8[i] & 0x1F) << 6) |
					   ((s_utf8[i + 1] & 0x3F));
				i++;
			}
			else if ((i + 2 < sLen) &&
				 ((s_utf8[i] & 0xF0) == 0xE0) &&
				 ((s_utf8[i + 1] & 0xC0) == 0x80) &&
				 ((s_utf8[i + 2] & 0xC0) == 0x80))
			{
				// 3-byte character.
				utf8char = ((s_utf8[i] & 0x0F) << 12) |
					   ((s_utf8[i + 1] & 0x3F) << 6) |
					   ((s_utf8[i + 2] & 0x3F));
				i += 2;
			}
			else if ((i + 3 < sLen) &&
				 ((s_utf8[i] & 0xF8) == 0xF0) &&
				 ((s_utf8[i + 1] & 0xC0) == 0x80) &&
				 ((s_utf8[i + 2] & 0xC0) == 0x80) &&
				 ((s_utf8[i + 3] & 0xC0) == 0x80))
			{
				// 4-byte character.
				utf8char = ((s_utf8[i] & 0x07) << 18) |
					   ((s_utf8[i + 1] & 0x3F) << 12) |
					   ((s_utf8[i + 2] & 0x3F) << 6) |
					   ((s_utf8[i + 3] & 0x3F));
				i += 3;
			}
			else
			{
				// Unknown character.
				utf8char = replacementChar;
			}
		}
		
		// Determine a replacement character.
		if (utf8char <= 0x7F)
		{
			// 7-bit ASCII character. Use as-is.
			cp1252char = (unsigned char)utf8char;
		}
		else if (utf8char >= 0xA0 && utf8char <= 0xFF)
		{
			// Unicode characters in the range 0xA0 - 0xFF match cp1252.
			cp1252char = (unsigned char)utf8char;
		}
		else
		{
			// Check the conversion table.
			if (charset_map_utf8_to_cp1252.empty())
			{
				// Fill the conversion table.
				int j = 0;
				
				while (charset_map_utf8_to_cp1252_array[j][0])
				{
					charset_map_utf8_to_cp1252.insert(charConversion(
							charset_map_utf8_to_cp1252_array[j][0],
							charset_map_utf8_to_cp1252_array[j][1]
										));
					j++;
				}
			}
			
			// Check the map for the character.
			charMap::iterator charIter;
			charIter = charset_map_utf8_to_cp1252.find(utf8char);
			
			if (charIter == charset_map_utf8_to_cp1252.end())
			{
				// Character not found.
				cp1252char = replacementChar;
			}
			else
			{
				// Character found.
				cp1252char = (*charIter).second;
			}
		}
		
		outBuf[outBufPos] = cp1252char;
		outBufPos++;
	}
	
	// Set the last character in the output buffer to 0.
	outBuf[outBufPos] = 0x00;
	
	// Return the string.
	string retString = reinterpret_cast<char*>(outBuf);
	free(outBuf);
	return retString;
}
