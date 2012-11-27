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

#include "made/redreader.h"

#include "common/file.h"
#include "common/memstream.h"

namespace Made {

Common::SeekableReadStream *RedReader::load(const char *redFilename, const char *filename) {

	Common::File fd;
	FileEntry fileEntry;

	if (!fd.open(redFilename))
		error("RedReader::RedReader() Could not open %s", redFilename);

	if (!seekFile(fd, fileEntry, filename))
		error("RedReader::RedReader() Could not find %s in archive %s", filename, redFilename);

	byte *fileBuf = (byte *)malloc(fileEntry.origSize);

	LzhDecompressor* lzhDec = new LzhDecompressor();
	lzhDec->decompress(fd, fileBuf, fileEntry.compSize, fileEntry.origSize);
	delete lzhDec;

	return new Common::MemoryReadStream(fileBuf, fileEntry.origSize, DisposeAfterUse::YES);

}

Common::SeekableReadStream *RedReader::loadFromRed(const char *redFilename, const char *filename) {
	RedReader* red = new RedReader();
	Common::SeekableReadStream *stream = red->load(redFilename, filename);
	delete red;
	return stream;
}

bool RedReader::seekFile(Common::File &fd, FileEntry &fileEntry, const char *filename) {
	char arcFilename[13];
	while (true) {
		fd.skip(8); // skip unknown
		fileEntry.compSize = fd.readUint32LE();
		if (fd.eos()) break;

		fileEntry.origSize = fd.readUint32LE();
		fd.skip(10); // skip unknown
		fd.read(arcFilename, 13);
		fd.skip(2); // skip unknown
		// Check if we have found the file
		if (!scumm_stricmp(arcFilename, filename))
			return true;
		// Skip compressed data
		fd.skip(fileEntry.compSize);
	}
	return false;
}

LzhDecompressor::LzhDecompressor() {
}

LzhDecompressor::~LzhDecompressor() {
}

int LzhDecompressor::decompress(Common::SeekableReadStream &source, byte *dest, uint32 sourceLen, uint32 destLen) {

	int bufsize;
	byte* buffer;

	buffer = (byte *) malloc(DICSIZ);

	_source = &source;
	_compSize = sourceLen;

	count_len_depth = 0;

	_blockPos = 0;

	decode_start();
	while (destLen > 0) {
		bufsize = ((destLen > DICSIZ) ? DICSIZ : destLen);
		decode(bufsize, buffer);
		memcpy(dest, buffer, bufsize);
		dest += bufsize;
		destLen -= bufsize;
	}

	free(buffer);

	return 0;
}

byte LzhDecompressor::readByte() {
	if (_blockPos == 0xFFE) {
		_blockPos = 0;
		_source->skip(2); // skip unknown value
	}
	byte temp = _source->readByte();
	_blockPos++;
	return temp;
}

void LzhDecompressor::fillbuf(int count) {
	_bitbuf <<= count;
	while (count > _bitcount) {
		_bitbuf |= _subbitbuf << (count -= _bitcount);
		if (_compSize != 0) {
			_compSize--;
			_subbitbuf = readByte();
		} else _subbitbuf = 0;
		_bitcount = 8;
	}
	_bitbuf |= _subbitbuf >> (_bitcount -= count);
}

uint LzhDecompressor::getbits(int count) {
	uint x;
	x = _bitbuf >> (BITBUFSIZ - count);
	fillbuf(count);
	return x;
}

void LzhDecompressor::init_getbits() {
	_bitbuf = 0;
	_subbitbuf = 0;
	_bitcount = 0;
	fillbuf(BITBUFSIZ);
}

void LzhDecompressor::decode_start() {
	huf_decode_start();
	decode_j = 0;
}

void LzhDecompressor::decode(uint count, byte buffer[]) {
	uint r, c;
	r = 0;
	while (--decode_j >= 0) {
		buffer[r] = buffer[decode_i];
		decode_i = (decode_i + 1) & (DICSIZ - 1);
		if (++r == count) return;
	}
	for ( ; ; ) {
		c = decode_c();
		if (c <= 255) {
			buffer[r] = c;
			if (++r == count) return;
		} else {
			decode_j = c - (255 + 1 - THRESHOLD);
			decode_i = (r - decode_p() - 1) & (DICSIZ - 1);
			while (--decode_j >= 0) {
				buffer[r] = buffer[decode_i];
				decode_i = (decode_i + 1) & (DICSIZ - 1);
				if (++r == count) return;
			}
		}
	}
}

void LzhDecompressor::read_pt_len(int nn, int nbit, int i_special) {
	int i, c, v;
	unsigned int mask;
	v = getbits(nbit);
	if (v == 0) {
		c = getbits(nbit);
		for (i = 0; i < nn; i++) _pt_len[i] = 0;
		for (i = 0; i < 256; i++) _pt_table[i] = c;
	} else {
		i = 0;
		while (i < v) {
			c = _bitbuf >> (BITBUFSIZ - 3);
			if (c == 7) {
				mask = 1U << (BITBUFSIZ - 1 - 3);
				while (mask & _bitbuf) {  mask >>= 1;  c++;  }
			}
			fillbuf((c < 7) ? 3 : c - 3);
			_pt_len[i++] = c;
			if (i == i_special) {
				c = getbits(2);
				while (--c >= 0) _pt_len[i++] = 0;
			}
		}
		while (i < nn) _pt_len[i++] = 0;
		make_table(nn, _pt_len, 8, _pt_table);
	}
}

void LzhDecompressor::read_c_len() {
	uint i, v;
	int c;
	unsigned int mask;
	v = getbits(CBIT);
	if (v == 0) {
		c = getbits(CBIT);
		for (i = 0; i < NC; i++) _c_len[i] = 0;
		for (i = 0; i < 4096; i++) _c_table[i] = c;
	} else {
		i = 0;
		while (i < v) {
			c = _pt_table[_bitbuf >> (BITBUFSIZ - 8)];
			if (c >= NT) {
				mask = 1U << (BITBUFSIZ - 1 - 8);
				do {
					if (_bitbuf & mask) c = _right[c];
					else			   c = _left [c];
					mask >>= 1;
				} while (c >= NT);
			}
			fillbuf(_pt_len[c]);
			if (c <= 2) {
				if	  (c == 0) c = 1;
				else if (c == 1) c = getbits(4) + 3;
				else			 c = getbits(CBIT) + 20;
				while (--c >= 0) _c_len[i++] = 0;
			} else _c_len[i++] = c - 2;
		}
		while (i < NC) _c_len[i++] = 0;
		make_table(NC, _c_len, 12, _c_table);
	}
}

unsigned int LzhDecompressor::decode_c() {
	uint j, mask;
	if (_blocksize == 0) {
		_blocksize = getbits(16);
		read_pt_len(NT, TBIT, 3);
		read_c_len();
		read_pt_len(NP, PBIT, -1);
	}
	_blocksize--;
	j = _c_table[_bitbuf >> (BITBUFSIZ - 12)];
	if (j >= NC) {
		mask = 1U << (BITBUFSIZ - 1 - 12);
		do {
			if (_bitbuf & mask) j = _right[j];
			else			   j = _left [j];
			mask >>= 1;
		} while (j >= NC);
	}
	fillbuf(_c_len[j]);
	return j;
}

unsigned int LzhDecompressor::decode_p() {
	unsigned int j, mask;
	j = _pt_table[_bitbuf >> (BITBUFSIZ - 8)];
	if (j >= NP) {
		mask = 1U << (BITBUFSIZ - 1 - 8);
		do {
			if (_bitbuf & mask) j = _right[j];
			else			   j = _left [j];
			mask >>= 1;
		} while (j >= NP);
	}
	fillbuf(_pt_len[j]);
	if (j != 0) j = (1U << (j - 1)) + getbits(j - 1);
	return j;
}

void LzhDecompressor::huf_decode_start() {
	init_getbits();
	_blocksize = 0;
}

void LzhDecompressor::make_table(uint nchar, byte bitlen[], uint tablebits, uint16 table[]) {
	uint16 count[17], weight[17], start[18], *p;
	uint i, k, len, ch, jutbits, avail, nextcode, mask;
	for (i = 1; i <= 16; i++) count[i] = 0;
	for (i = 0; i < nchar; i++) count[bitlen[i]]++;
	start[1] = 0;
	for (i = 1; i <= 16; i++)
		start[i + 1] = start[i] + (count[i] << (16 - i));
	if (start[17] != (uint16)(1U << 16))
		error("LzhDecompressor::make_table() Bad table");
	jutbits = 16 - tablebits;
	for (i = 1; i <= tablebits; i++) {
		start[i] >>= jutbits;
		weight[i] = 1U << (tablebits - i);
	}
	for (; i <= 16; i++) {
		weight[i] = 1U << (16 - i);
	}
	i = start[tablebits + 1] >> jutbits;
	if (i != (uint16)(1U << 16)) {
		k = 1U << tablebits;
		while (i != k) table[i++] = 0;
	}
	avail = nchar;
	mask = 1U << (15 - tablebits);
	for (ch = 0; ch < nchar; ch++) {
		if ((len = bitlen[ch]) == 0) continue;
		nextcode = start[len] + weight[len];
		if (len <= tablebits) {
			for (i = start[len]; i < nextcode; i++) table[i] = ch;
		} else {
			k = start[len];
			p = &table[k >> jutbits];
			i = len - tablebits;
			while (i != 0) {
				if (*p == 0) {
					_right[avail] = _left[avail] = 0;
					*p = avail++;
				}
				if (k & mask) p = &_right[*p];
				else		  p = &_left[*p];
				k <<= 1;  i--;
			}
			*p = ch;
		}
		start[len] = nextcode;
	}
}

/* call with i = root */
void LzhDecompressor::count_len(int i) {
	if (i < tree_n)
		len_cnt[(count_len_depth < 16) ? count_len_depth : 16]++;
	else {
		count_len_depth++;
		count_len(_left [i]);
		count_len(_right[i]);
		count_len_depth--;
	}
}

void LzhDecompressor::make_len(int root) {
	int i, k;
	uint cum;
	for (i = 0; i <= 16; i++) len_cnt[i] = 0;
	count_len(root);
	cum = 0;
	for (i = 16; i > 0; i--)
		cum += len_cnt[i] << (16 - i);
	while (cum != (1U << 16)) {
		len_cnt[16]--;
		for (i = 15; i > 0; i--) {
			if (len_cnt[i] != 0) {
				len_cnt[i]--;
				len_cnt[i+1] += 2;
				break;
			}
		}
		cum--;
	}
	for (i = 16; i > 0; i--) {
		k = len_cnt[i];
		while (--k >= 0) len_table[*sortptr++] = i;
	}
}

void LzhDecompressor::downheap(int i) {
	int j, k;
	k = heap[i];
	while ((j = 2 * i) <= heapsize) {
		if (j < heapsize && freq[heap[j]] > freq[heap[j + 1]])
			j++;
		if (freq[k] <= freq[heap[j]]) break;
		heap[i] = heap[j];  i = j;
	}
	heap[i] = k;
}

void LzhDecompressor::make_code(int n, byte len[], uint16 code[]) {
	int	i;
	uint16 start[18];
	start[1] = 0;
	for (i = 1; i <= 16; i++)
		start[i + 1] = (start[i] + len_cnt[i]) << 1;
	for (i = 0; i < n; i++) code[i] = start[len[i]]++;
}

/* make tree, calculate len[], return root */
int LzhDecompressor::make_tree(int nparm, uint16 freqparm[], byte lenparm[], uint16 codeparm[]) {
	int i, j, k, avail;

	tree_n = nparm;
	freq = freqparm;
	len_table = lenparm;
	avail = tree_n;
	heapsize = 0;
	heap[1] = 0;
	for (i = 0; i < tree_n; i++) {
		len_table[i] = 0;
		if (freq[i]) heap[++heapsize] = i;
	}
	if (heapsize < 2) {
		codeparm[heap[1]] = 0;
		return heap[1];
	}
	for (i = heapsize / 2; i >= 1; i--)
		downheap(i);  /* make priority queue */
	sortptr = codeparm;
	do {  /* while queue has at least two entries */
		i = heap[1];  /* take out least-freq entry */
		if (i < tree_n) *sortptr++ = i;
		heap[1] = heap[heapsize--];
		downheap(1);
		j = heap[1];  /* next least-freq entry */
		if (j < tree_n) *sortptr++ = j;
		k = avail++;  /* generate new node */
		freq[k] = freq[i] + freq[j];
		heap[1] = k;
		downheap(1);  /* put into queue */
		_left[k] = i;
		_right[k] = j;
	} while (heapsize > 1);
	sortptr = codeparm;
	make_len(k);
	make_code(nparm, lenparm, codeparm);
	return k;  /* return root */
}

}
