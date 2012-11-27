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

#ifndef MADE_REDREADER_H
#define MADE_REDREADER_H

#include "common/scummsys.h"

namespace Common {
class SeekableReadStream;
class File;
}

namespace Made {

class RedReader {
public:
	Common::SeekableReadStream *load(const char *redFilename, const char *filename);
	static Common::SeekableReadStream *loadFromRed(const char *redFilename, const char *filename);
private:
	struct FileEntry {
		uint32 compSize, origSize;
	};
	bool seekFile(Common::File &fd, FileEntry &fileEntry, const char *filename);
};

const uint BITBUFSIZ = 16;
const uint DICBIT = 13;
const uint DICSIZ = 1 << DICBIT;
const uint MATCHBIT = 8;
const uint MAXMATCH = 256;
const uint THRESHOLD = 3;
const uint NC = 255 + MAXMATCH + 2 - THRESHOLD;
const uint CBIT = 9;
const uint CODE_BIT = 16;
const uint NP = DICBIT + 1;
const int NT = CODE_BIT + 3;
const uint PBIT = 4;
const uint TBIT = 5;
const uint NPT = NT;

class LzhDecompressor {
public:
	LzhDecompressor();
	~LzhDecompressor();
	int decompress(Common::SeekableReadStream &source, byte *dest, uint32 compSize, uint32 origSize);
private:
	Common::SeekableReadStream *_source;
	uint32 _compSize, _blockPos;

	uint16 _bitbuf;
	uint _subbitbuf;
	int _bitcount;
	uint16 _left[2 * NC - 1], _right[2 * NC - 1];
	byte _c_len[NC], _pt_len[NPT];
	uint _blocksize;
	uint16 _c_table[4096], _pt_table[256];
	int tree_n, heapsize;
	short heap[NC + 1];
	uint16 *freq, *sortptr, len_cnt[17];
	byte *len_table;

	int decode_i, decode_j;
	int count_len_depth;

	byte readByte();

	void fillbuf(int count);
	uint getbits(int count);
	void init_getbits();
	void decode_start();
	void decode(uint count, byte text[]);
	void huf_decode_start();
	unsigned int decode_c();
	unsigned int decode_p();
	void read_pt_len(int nn, int nbit, int i_special);
	void read_c_len();
	void count_len(int i);
	void make_len(int root);
	void downheap(int i);
	void make_code(int n, byte len[], uint16 code[]);
	void make_table(uint nchar, byte bitlen[], uint tablebits, uint16 table[]);
	int make_tree(int nparm, uint16 freqparm[], byte lenparm[], uint16 codeparm[]);

};

} // End of namespace Made

#endif /* MADE_H */
