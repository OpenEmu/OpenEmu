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

#ifndef SCI_DECOMPRESSOR_H
#define SCI_DECOMPRESSOR_H

#include "common/scummsys.h"

namespace Common {
class ReadStream;
}

namespace Sci {

enum ResourceCompression {
	kCompUnknown = -1,
	kCompNone = 0,
	kCompLZW,
	kCompHuffman,
	kCompLZW1,			// LZW-like compression used in SCI01 and SCI1
	kCompLZW1View,		// Comp3 + view Post-processing
	kCompLZW1Pic,		// Comp3 + pic Post-processing
#ifdef ENABLE_SCI32
	kCompSTACpack,	// ? Used in SCI32
#endif
	kCompDCL
};

/**
 * Base class for decompressors.
 * Simply copies nPacked bytes from src to dest.
 */
class Decompressor {
public:
	Decompressor() {}
	virtual ~Decompressor() {}


	virtual int unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);

protected:
	/**
	 * Initialize decompressor.
	 * @param src		source stream to read from
	 * @param dest		destination stream to write to
	 * @param nPacked	size of packed data
	 * @param nUnpacket	size of unpacked data
	 * @return 0 on success, non-zero on error
	 */
	virtual void init(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);

	/**
	 * Get a number of bits from _src stream, starting with the most
	 * significant unread bit of the current four byte block.
	 * @param n		number of bits to get
	 * @return n-bits number
	 */
	uint32 getBitsMSB(int n);

	/**
	 * Get a number of bits from _src stream, starting with the least
	 * significant unread bit of the current four byte block.
	 * @param n		number of bits to get
	 * @return n-bits number
	 */
	uint32 getBitsLSB(int n);

	/**
	 * Get one byte from _src stream.
	 * @return byte
	 */
	byte getByteMSB();
	byte getByteLSB();

	void fetchBitsMSB();
	void fetchBitsLSB();

	/**
	 * Write one byte into _dest stream
	 * @param b byte to put
	 */

	virtual void putByte(byte b);

	/**
	 * Returns true if all expected data has been unpacked to _dest
	 * and there is no more data in _src.
	 */
	bool isFinished() {
		return (_dwWrote == _szUnpacked) && (_dwRead >= _szPacked);
	}

	uint32 _dwBits;		///< bits buffer
	byte _nBits;		///< number of unread bits in _dwBits
	uint32 _szPacked;	///< size of the compressed data
	uint32 _szUnpacked;	///< size of the decompressed data
	uint32 _dwRead;		///< number of bytes read from _src
	uint32 _dwWrote;	///< number of bytes written to _dest
	Common::ReadStream *_src;
	byte *_dest;
};

/**
 * Huffman decompressor
 */
class DecompressorHuffman : public Decompressor {
public:
	int unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);

protected:
	int16 getc2();

	byte *_nodes;
};

/**
 * LZW-like decompressor for SCI01/SCI1.
 * TODO: Needs clean-up of post-processing fncs
 */
class DecompressorLZW : public Decompressor {
public:
	DecompressorLZW(int nCompression) {
		_compression = nCompression;
	}
	void init(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);
	int unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);

protected:
	enum {
		PIC_OPX_EMBEDDED_VIEW = 1,
		PIC_OPX_SET_PALETTE = 2,
		PIC_OP_OPX = 0xfe
	};
	// unpacking procedures
	// TODO: unpackLZW and unpackLZW1 are similar and should be merged
	int unpackLZW1(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);
	int unpackLZW(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);

	// functions to post-process view and pic resources
	void reorderPic(byte *src, byte *dest, int dsize);
	void reorderView(byte *src, byte *dest);
	void decodeRLE(byte **rledata, byte **pixeldata, byte *outbuffer, int size);
	int getRLEsize(byte *rledata, int dsize);
	void buildCelHeaders(byte **seeker, byte **writer, int celindex, int *cc_lengths, int max);

	// decompressor data
	struct Tokenlist {
		byte data;
		uint16 next;
	};
	uint16 _numbits;
	uint16 _curtoken, _endtoken;
	int _compression;
};

/**
 * DCL decompressor for SCI1.1
 */
class DecompressorDCL : public Decompressor {
public:
	int unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);
};

#ifdef ENABLE_SCI32
/**
 * STACpack decompressor for SCI32
 */
class DecompressorLZS : public Decompressor {
public:
	int unpack(Common::ReadStream *src, byte *dest, uint32 nPacked, uint32 nUnpacked);
protected:
	int unpackLZS();
	uint32 getCompLen();
	void copyComp(int offs, uint32 clen);
};
#endif

} // End of namespace Sci

#endif // SCI_SCICORE_DECOMPRESSOR_H
