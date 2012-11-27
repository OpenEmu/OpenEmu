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

// Based on eos' Huffman code

#ifndef COMMON_HUFFMAN_H
#define COMMON_HUFFMAN_H

#include "common/array.h"
#include "common/list.h"
#include "common/types.h"

namespace Common {

class BitStream;

/**
 * Huffman bitstream decoding
 *
 * Used in engines:
 *  - scumm
 */
class Huffman {
public:
	/** Construct a Huffman decoder.
	 *
	 *  @param maxLength Maximal code length. If 0, it's searched for.
	 *  @param codeCount Number of codes.
	 *  @param codes The actual codes.
	 *  @param lengths Lengths of the individual codes.
	 *  @param symbols The symbols. If 0, assume they are identical to the code indices.
	 */
	Huffman(uint8 maxLength, uint32 codeCount, const uint32 *codes, const uint8 *lengths, const uint32 *symbols = 0);
	~Huffman();

	/** Modify the codes' symbols. */
	void setSymbols(const uint32 *symbols = 0);

	/** Return the next symbol in the bitstream. */
	uint32 getSymbol(BitStream &bits) const;

private:
	struct Symbol {
		uint32 code;
		uint32 symbol;

		Symbol(uint32 c, uint32 s);
	};

	typedef List<Symbol> CodeList;
	typedef Array<CodeList> CodeLists;
	typedef Array<Symbol *> SymbolList;

	/** Lists of codes and their symbols, sorted by code length. */
	CodeLists _codes;

	/** Sorted list of pointers to the symbols. */
	SymbolList _symbols;
};

} // End of namespace Common

#endif // COMMON_HUFFMAN_H
