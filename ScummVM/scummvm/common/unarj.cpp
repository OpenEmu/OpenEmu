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
 */

//
// This file is heavily based on the arj code available under the GPL
// from http://arj.sourceforge.net/ , version 3.10.22 .

#include "common/scummsys.h"
#include "common/archive.h"
#include "common/debug.h"
#include "common/unarj.h"
#include "common/file.h"
#include "common/hash-str.h"
#include "common/memstream.h"
#include "common/bufferedstream.h"
#include "common/textconsole.h"

namespace Common {

#define ARJ_UCHAR_MAX 255
#define ARJ_CHAR_BIT 8

#define ARJ_COMMENT_MAX 2048
#define ARJ_FILENAME_MAX 512

#define ARJ_CODE_BIT 16
#define ARJ_THRESHOLD 3
#define ARJ_DICSIZ 26624
#define ARJ_FDICSIZ ARJ_DICSIZ
#define ARJ_MAXDICBIT   16
#define ARJ_MAXMATCH   256
#define ARJ_NC (ARJ_UCHAR_MAX + ARJ_MAXMATCH + 2 - ARJ_THRESHOLD)
#define ARJ_NP (ARJ_MAXDICBIT + 1)
#define ARJ_NT (ARJ_CODE_BIT + 3)

#if ARJ_NT > ARJ_NP
#define ARJ_NPT ARJ_NT
#else
#define ARJ_NPT ARJ_NP
#endif

#define ARJ_CTABLESIZE 4096
#define ARJ_PTABLESIZE 256

// these struct represents a file inside an Arj archive
struct ArjHeader {
	int32 pos;
	uint16 id;
	uint16 headerSize;
	//
	byte firstHdrSize;
	byte nbr;
	byte xNbr;
	byte hostOs;
	byte flags;
	byte method;
	byte fileType;
	byte pad;
	uint32 timeStamp;
	int32 compSize;
	int32 origSize;
	uint32 fileCRC;
	uint16 entryPos;
	uint16 fileMode;
	uint16 hostData;
	char   filename[ARJ_FILENAME_MAX];
	char   comment[ARJ_COMMENT_MAX];

	uint32 headerCrc;
};

static int32 findHeader(SeekableReadStream &stream);
static ArjHeader *readHeader(SeekableReadStream &stream);

class ArjDecoder {
public:
	ArjDecoder(const ArjHeader *hdr) {
		_compsize = hdr->compSize;
	}

	~ArjDecoder() {
		delete _compressed;
		delete _outstream;
	}

	void decode(int32 origsize);
	void decode_f(int32 origsize);

	ReadStream *_compressed;
	MemoryWriteStream *_outstream;

//protected:
	uint16 _bitbuf;
	uint16 _bytebuf;
	int32 _compsize;
	byte _subbitbuf;
	int _bitcount;

	void init_getbits();
	void fillbuf(int n);
	uint16 getbits(int n);


	void make_table(int nchar, byte *bitlen, int tablebits, uint16 *table, int tablesize);
	void read_pt_len(int nn, int nbit, int i_special);
	void read_c_len(void);
	uint16 decode_c(void);
	uint16 decode_p(void);
	void decode_start(void);
	int16 decode_ptr(void);
	int16 decode_len(void);

private:
	byte  _ntext[ARJ_FDICSIZ];

	int16  _getlen;
	int16  _getbuf;

	uint16 _left[2 * ARJ_NC - 1];
	uint16 _right[2 * ARJ_NC - 1];
	byte  _c_len[ARJ_NC];
	byte  _pt_len[ARJ_NPT];

	uint16 _c_table[ARJ_CTABLESIZE];
	uint16 _pt_table[ARJ_PTABLESIZE];
	uint16 _blocksize;
};

#define HEADER_ID     0xEA60
#define HEADER_ID_HI    0xEA
#define HEADER_ID_LO    0x60

#define FIRST_HDR_SIZE    30
#define HEADERSIZE_MAX   (FIRST_HDR_SIZE + 10 + ARJ_FILENAME_MAX + ARJ_COMMENT_MAX)
#define CRC_MASK        0xFFFFFFFFL
#define HSLIMIT_ARJ		524288L

#define CBIT		 9
#define PBIT		 5
#define TBIT		 5

// Source for CRC32::init, CRC32::checksum : crc32.c
class CRC32 {
	static uint32 	_table[256];
	static bool _initialized;

private:
	static void init() {
		const uint32 poly = 0xEDB88320;
		int i, j;
		uint32 r;

		for (i = 0; i < 256; i++) {
			r = i;
			for (j = 0; j < 8; j++)
				if (r & 1)
					r = (r >> 1) ^ poly;
				else
					r >>= 1;
			_table[i] = r;
		}

		_initialized = true;
	}

public:
	static uint32 checksum(byte *data, int len) {
		if (!_initialized) {
			init();
		}

		uint32 CRC = 0xFFFFFFFF;
		int i;
		for (i = 0; i < len; i++)
			CRC = (CRC >> 8) ^ _table[(CRC ^ data[i]) & 0xFF];
		return CRC ^ 0xFFFFFFFF;
	}
};

bool CRC32::_initialized = false;
uint32 CRC32::_table[256];

// Source for findHeader and readHeader: arj_arcv.c
int32 findHeader(SeekableReadStream &stream) {
	long end_pos, tmp_pos;
	int id;
	byte header[HEADERSIZE_MAX];
	uint32 crc;
	uint16 basic_hdr_size;

	tmp_pos = stream.pos();
	stream.seek(0L, SEEK_END);
	end_pos = stream.pos() - 2;
	if (end_pos >= tmp_pos + HSLIMIT_ARJ)
		end_pos = tmp_pos + HSLIMIT_ARJ;

	while (tmp_pos < end_pos) {
		stream.seek(tmp_pos, SEEK_SET);
		id = stream.readByte();
		while (tmp_pos < end_pos) {
			if (id == HEADER_ID_LO) {
				if ((id = stream.readByte()) == HEADER_ID_HI)
					break;
			} else
				id = stream.readByte();
			tmp_pos++;
		}
		if (tmp_pos >= end_pos)
			return -1;
		if ((basic_hdr_size = stream.readUint16LE()) <= HEADERSIZE_MAX) {
			stream.read(header, basic_hdr_size);
			crc = CRC32::checksum(header, basic_hdr_size);
			if (crc == stream.readUint32LE()) {
				stream.seek(tmp_pos, SEEK_SET);
				return tmp_pos;
			}
		}
		tmp_pos++;
	}
	return -1;
}

ArjHeader *readHeader(SeekableReadStream &stream) {
	ArjHeader header;
	ArjHeader *head;
	byte headData[HEADERSIZE_MAX];

	// Strictly check the header ID
	header.id = stream.readUint16LE();
	if (header.id != HEADER_ID) {
		warning("ArjFile::readHeader(): Bad header ID (%x)", header.id);

		return NULL;
	}

	header.headerSize = stream.readUint16LE();
	if (header.headerSize == 0)
		return NULL;			// end of archive
	if (header.headerSize > HEADERSIZE_MAX) {
		warning("ArjFile::readHeader(): Bad header");

		return NULL;
	}

	int rSize = stream.read(headData, header.headerSize);

	MemoryReadStream readS(headData, rSize);

	header.headerCrc = stream.readUint32LE();
	if (CRC32::checksum(headData, header.headerSize) != header.headerCrc) {
		warning("ArjFile::readHeader(): Bad header CRC");
		return NULL;
	}

	header.firstHdrSize = readS.readByte();
	header.nbr = readS.readByte();
	header.xNbr = readS.readByte();
	header.hostOs = readS.readByte();
	header.flags = readS.readByte();
	header.method = readS.readByte();
	header.fileType = readS.readByte();
	(void)readS.readByte(); // password_modifier
	header.timeStamp = readS.readUint32LE();
	header.compSize = readS.readSint32LE();
	header.origSize = readS.readSint32LE();
	header.fileCRC = readS.readUint32LE();
	header.entryPos = readS.readUint16LE();
	header.fileMode = readS.readUint16LE();
	header.hostData = readS.readUint16LE();

	// static int check_file_size()
	if (header.origSize < 0 || header.compSize < 0) {
		warning("ArjFile::readHeader(): Wrong file size");
		return NULL;
	}

	strlcpy(header.filename, (const char *)&headData[header.firstHdrSize], ARJ_FILENAME_MAX);
	strlcpy(header.comment, (const char *)&headData[header.firstHdrSize + strlen(header.filename) + 1], ARJ_COMMENT_MAX);

	// Process extended headers, if any
	uint16 extHeaderSize;
	while ((extHeaderSize = stream.readUint16LE()) != 0)
		stream.seek((long)(extHeaderSize + 4), SEEK_CUR);

	header.pos = stream.pos();

	head = new ArjHeader(header);

	return head;
}

// Source for init_getbits: arj_file.c (decode_start_stub)
void ArjDecoder::init_getbits() {
	_bitbuf = 0;
	_bytebuf = 0;
	_bitcount = 0;
	fillbuf(ARJ_CHAR_BIT * 2);
}

// Source for fillbuf, getbits: decode.c
void ArjDecoder::fillbuf(int n) {
	while (_bitcount < n) {
		_bitbuf = (_bitbuf << _bitcount) | (_bytebuf >> (8 - _bitcount));
		n -= _bitcount;
		if (_compsize > 0) {
			_compsize--;
			_bytebuf = _compressed->readByte();
		} else {
			_bytebuf = 0;
		}
		_bitcount = 8;
	}
	_bitcount -= n;
	_bitbuf = ( _bitbuf << n) | (_bytebuf >> (8-n));
	_bytebuf <<= n;
}

// Reads a series of bits into the input buffer */
uint16 ArjDecoder::getbits(int n) {
	uint16 rc;

	rc = _bitbuf >> (ARJ_CODE_BIT - n);
	fillbuf(n);
	return rc;
}

// Huffman decode routines
// Source: decode.c

// Creates a table for decoding
void ArjDecoder::make_table(int nchar, byte *bitlen, int tablebits, uint16 *table, int tablesize) {
	uint16 count[17], weight[17], start[18];
	uint16 *p;
	uint i, k, len, ch, jutbits, avail, nextcode, mask;

	for (i = 1; i <= 16; i++)
		count[i] = 0;
	for (i = 0; (int)i < nchar; i++)
		count[bitlen[i]]++;

	start[1] = 0;
	for (i = 1; i <= 16; i++)
		start[i + 1] = start[i] + (count[i] << (16 - i));
	if (start[17] != (uint16) (1 << 16))
		error("ArjDecoder::make_table(): bad file data");

	jutbits = 16 - tablebits;
	for (i = 1; (int)i <= tablebits; i++) {
		start[i] >>= jutbits;
		weight[i] = 1 << (tablebits - i);
	}
	while (i <= 16) {
		weight[i] = 1 << (16 - i);
		i++;
	}

	i = start[tablebits + 1] >> jutbits;
	if (i != (uint16) (1 << 16)) {
		k = 1 << tablebits;
		while (i != k)
			table[i++] = 0;
	}

	avail = nchar;
	mask = 1 << (15 - tablebits);
	for (ch = 0; (int)ch < nchar; ch++) {
		if ((len = bitlen[ch]) == 0)
			continue;
		k = start[len];
		nextcode = k + weight[len];
		if ((int)len <= tablebits) {
			if (nextcode > (uint)tablesize)
				error("ArjDecoder::make_table(): bad file data");
			for (i = start[len]; i < nextcode; i++)
				table[i] = ch;
		} else {
			p = &table[k >> jutbits];
			i = len - tablebits;
			while (i != 0) {
				if (*p == 0) {
					_right[avail] = _left[avail] = 0;
					*p = avail;
					avail++;
				}
				if (k & mask)
					p = &_right[*p];
				else
					p = &_left[*p];
				k <<= 1;
				i--;
			}
			*p = ch;
		}
		start[len] = nextcode;
	}
}

// Reads length of data pending
void ArjDecoder::read_pt_len(int nn, int nbit, int i_special) {
	int i, n;
	int16 c;
	uint16 mask;

	n = getbits(nbit);
	if (n == 0) {
		c = getbits(nbit);
		for (i = 0; i < nn; i++)
			_pt_len[i] = 0;
		for (i = 0; i < 256; i++)
			_pt_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = _bitbuf >> 13;
			if (c == 7) {
				mask = 1 << 12;
				while (mask & _bitbuf) {
					mask >>= 1;
					c++;
				}
			}
			fillbuf((c < 7) ? 3 : (int)(c - 3));
			_pt_len[i++] = (byte)c;
			if (i == i_special) {
				c = getbits(2);
				while (--c >= 0)
					_pt_len[i++] = 0;
			}
		}
		while (i < nn)
			_pt_len[i++] = 0;
		make_table(nn, _pt_len, 8, _pt_table, ARJ_PTABLESIZE);
	}
}

// Reads a character table
void ArjDecoder::read_c_len() {
	int16 i, c, n;
	uint16 mask;

	n = getbits(CBIT);
	if (n == 0) {
		c = getbits(CBIT);
		for (i = 0; i < ARJ_NC; i++)
			_c_len[i] = 0;
		for (i = 0; i < ARJ_CTABLESIZE; i++)
			_c_table[i] = c;
	} else {
		i = 0;
		while (i < n) {
			c = _pt_table[_bitbuf >> (8)];
			if (c >= ARJ_NT) {
				mask = 1 << 7;
				do {
					if (_bitbuf & mask)
						c = _right[c];
					else
						c = _left[c];
					mask >>= 1;
				} while (c >= ARJ_NT);
			}
			fillbuf((int)(_pt_len[c]));
			if (c <= 2) {
				if (c == 0)
					c = 1;
				else if (c == 1) {
					c = getbits(4);
					c += 3;
				} else {
					c = getbits(CBIT);
					c += 20;
				}
				while (--c >= 0)
					_c_len[i++] = 0;
			}
			else
				_c_len[i++] = (byte)(c - 2);
		}
		while (i < ARJ_NC)
			_c_len[i++] = 0;
		make_table(ARJ_NC, _c_len, 12, _c_table, ARJ_CTABLESIZE);
	}
}

// Decodes a single character
uint16 ArjDecoder::decode_c() {
	uint16 j, mask;

	if (_blocksize == 0) {
		_blocksize = getbits(ARJ_CODE_BIT);
		read_pt_len(ARJ_NT, TBIT, 3);
		read_c_len();
		read_pt_len(ARJ_NP, PBIT, -1);
	}
	_blocksize--;
	j = _c_table[_bitbuf >> 4];
	if (j >= ARJ_NC) {
		mask = 1 << 3;
		do {
			if (_bitbuf & mask)
				j = _right[j];
			else
				j = _left[j];
			mask >>= 1;
		} while (j >= ARJ_NC);
	}
	fillbuf((int)(_c_len[j]));
	return j;
}

// Decodes a control character
uint16 ArjDecoder::decode_p() {
	uint16 j, mask;

	j = _pt_table[_bitbuf >> 8];
	if (j >= ARJ_NP) {
		mask = 1 << 7;
		do {
			if (_bitbuf & mask)
				j = _right[j];
			else
				j = _left[j];
			mask >>= 1;
		} while (j >= ARJ_NP);
	}
	fillbuf((int)(_pt_len[j]));
	if (j != 0) {
		j--;
		j = (1 << j) + getbits((int)j);
	}
	return j;
}

// Initializes memory for decoding
void ArjDecoder::decode_start() {
	_blocksize = 0;
	init_getbits();
}

// Decodes the entire file
void ArjDecoder::decode(int32 origsize) {
	int16 i;
	int16 r;
	int16 c;
	int16 j;
	int32 count;

	decode_start();
	count = origsize;
	r = 0;

	while (count > 0) {
		if ((c = decode_c()) <= ARJ_UCHAR_MAX) {
			_ntext[r] = (byte) c;
			count--;
			if (++r >= ARJ_DICSIZ) {
				r = 0;
				_outstream->write(_ntext, ARJ_DICSIZ);
			}
		} else {
			j = c - (ARJ_UCHAR_MAX + 1 - ARJ_THRESHOLD);
			count -= j;
			i = r - decode_p() - 1;
			if (i < 0)
				i += ARJ_DICSIZ;
			if (r > i && r < ARJ_DICSIZ - ARJ_MAXMATCH - 1) {
				while (--j >= 0)
					_ntext[r++] = _ntext[i++];
			} else {
				while (--j >= 0) {
					_ntext[r] = _ntext[i];
					if (++r >= ARJ_DICSIZ) {
						r = 0;
						_outstream->write(_ntext, ARJ_DICSIZ);
					}
					if (++i >= ARJ_DICSIZ)
						i = 0;
				}
			}
		}
	}
	if (r > 0)
		_outstream->write(_ntext, r);
}

// Backward pointer decoding
int16 ArjDecoder::decode_ptr() {
	int16 c = 0;
	int16 width;
	int16 plus;
	int16 pwr;

	plus = 0;
	pwr = 1 << 9;
	for (width = 9; width < 13; width++) {
		c = getbits(1);
		if (c == 0)
			break;
		plus += pwr;
		pwr <<= 1;
	}
	if (width != 0)
		c = getbits(width);
	c += plus;
	return c;
}

// Reference length decoding
int16 ArjDecoder::decode_len() {
	int16 c = 0;
	int16 width;
	int16 plus;
	int16 pwr;

	plus = 0;
	pwr = 1;
	for (width = 0; width < 7; width++) {
		c = getbits(1);
		if (c == 0)
			break;
		plus += pwr;
		pwr <<= 1;
	}
	if (width != 0)
		c = getbits(width);
	c += plus;
	return c;
}

// Decodes the entire file, using method 4
void ArjDecoder::decode_f(int32 origsize) {
	int16 i;
	int16 j;
	int16 c;
	int16 r;
	uint32 ncount;

	init_getbits();
	ncount = 0;
	_getlen = _getbuf = 0;
	r = 0;

	while (ncount < (uint32)origsize) {
		c = decode_len();
		if (c == 0) {
			ncount++;
			_ntext[r] = (byte)getbits(8);
			if (++r >= ARJ_FDICSIZ) {
				r = 0;
				_outstream->write(_ntext, ARJ_FDICSIZ);
			}
		} else {
			j = c - 1 + ARJ_THRESHOLD;
			ncount += j;
			if ((i = r - decode_ptr() - 1) < 0)
				i += ARJ_FDICSIZ;
			while (j-- > 0) {
				_ntext[r] = _ntext[i];
				if (++r >= ARJ_FDICSIZ) {
					r = 0;
					_outstream->write(_ntext, ARJ_FDICSIZ);
				}
				if (++i >= ARJ_FDICSIZ)
					i = 0;
			}
		}
	}
	if (r != 0)
		_outstream->write(_ntext, r);
}

#pragma mark ArjArchive implementation

typedef HashMap<String, ArjHeader*, IgnoreCase_Hash, IgnoreCase_EqualTo> ArjHeadersMap;

class ArjArchive : public Archive {
	ArjHeadersMap _headers;
	String _arjFilename;

public:
	ArjArchive(const String &name);
	virtual ~ArjArchive();

	// Archive implementation
	virtual bool hasFile(const String &name) const;
	virtual int listMembers(ArchiveMemberList &list) const;
	virtual const ArchiveMemberPtr getMember(const String &name) const;
	virtual SeekableReadStream *createReadStreamForMember(const String &name) const;
};

ArjArchive::ArjArchive(const String &filename) : _arjFilename(filename) {
	File arjFile;

	if (!arjFile.open(_arjFilename)) {
		warning("ArjArchive::ArjArchive(): Could not find the archive file");
		return;
	}

	int32 firstHeaderOffset = findHeader(arjFile);

	if (firstHeaderOffset < 0) {
		warning("ArjArchive::ArjArchive(): Could not find a valid header");
		return;
	}

	ArjHeader *header = NULL;

	arjFile.seek(firstHeaderOffset, SEEK_SET);
	if ((header = readHeader(arjFile)) == NULL)
		return;
	delete header;

	while ((header = readHeader(arjFile)) != NULL) {
		_headers[header->filename] = header;
		arjFile.seek(header->compSize, SEEK_CUR);
	}

	debug(0, "ArjArchive::ArjArchive(%s): Located %d files", filename.c_str(), _headers.size());
}

ArjArchive::~ArjArchive() {
	debug(0, "ArjArchive Destructor Called");
	ArjHeadersMap::iterator it = _headers.begin();
	for ( ; it != _headers.end(); ++it) {
		delete it->_value;
	}
}

bool ArjArchive::hasFile(const String &name) const {
	return _headers.contains(name);
}

int ArjArchive::listMembers(ArchiveMemberList &list) const {
	int matches = 0;

	ArjHeadersMap::const_iterator it = _headers.begin();
	for ( ; it != _headers.end(); ++it) {
		list.push_back(ArchiveMemberList::value_type(new GenericArchiveMember(it->_value->filename, this)));
		matches++;
	}

	return matches;
}

const ArchiveMemberPtr ArjArchive::getMember(const String &name) const {
	if (!hasFile(name))
		return ArchiveMemberPtr();

	return ArchiveMemberPtr(new GenericArchiveMember(name, this));
}

SeekableReadStream *ArjArchive::createReadStreamForMember(const String &name) const {
	if (!_headers.contains(name)) {
		return 0;
	}

	ArjHeader *hdr = _headers[name];

	File archiveFile;
	archiveFile.open(_arjFilename);
	archiveFile.seek(hdr->pos, SEEK_SET);

	// TODO: It would be good if ArjFile could decompress files in a streaming
	// mode, so it would not need to pre-allocate the entire output.
	byte *uncompressedData = (byte *)malloc(hdr->origSize);
	assert(uncompressedData);

	if (hdr->method == 0) { // store
		int32 len = archiveFile.read(uncompressedData, hdr->origSize);
		assert(len == hdr->origSize);
	} else {
		ArjDecoder *decoder = new ArjDecoder(hdr);

		// TODO: It might not be appropriate to use this wrapper inside ArjFile.
		// If reading from archiveFile directly is too slow to be usable,
		// maybe the filesystem code should instead wrap its files
		// in a BufferedReadStream.
		decoder->_compressed = wrapBufferedReadStream(&archiveFile, 4096, DisposeAfterUse::NO);
		decoder->_outstream = new MemoryWriteStream(uncompressedData, hdr->origSize);

		if (hdr->method == 1 || hdr->method == 2 || hdr->method == 3)
			decoder->decode(hdr->origSize);
		else if (hdr->method == 4)
			decoder->decode_f(hdr->origSize);

		delete decoder;
	}

	return new MemoryReadStream(uncompressedData, hdr->origSize, DisposeAfterUse::YES);
}

Archive *makeArjArchive(const String &name) {
	return new ArjArchive(name);
}

} // End of namespace Common
