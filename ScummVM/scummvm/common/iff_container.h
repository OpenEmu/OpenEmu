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
 */

#ifndef COMMON_IFF_CONTAINER_H
#define COMMON_IFF_CONTAINER_H

#include "common/scummsys.h"
#include "common/endian.h"
#include "common/func.h"
#include "common/stream.h"
#include "common/textconsole.h"

namespace Common {

typedef uint32 IFF_ID;

#define ID_FORM     MKTAG('F','O','R','M')
/* EA IFF 85 group identifier */
#define ID_CAT      MKTAG('C','A','T',' ')
/* EA IFF 85 group identifier */
#define ID_LIST     MKTAG('L','I','S','T')
/* EA IFF 85 group identifier */
#define ID_PROP     MKTAG('P','R','O','P')
/* EA IFF 85 group identifier */
#define ID_END      MKTAG('E','N','D',' ')
/* unofficial END-of-FORM identifier (see Amiga RKM Devices Ed.3
page 376) */
#define ID_ILBM     MKTAG('I','L','B','M')
/* EA IFF 85 raster bitmap form */
#define ID_DEEP     MKTAG('D','E','E','P')
/* Chunky pixel image files (Used in TV Paint) */
#define ID_RGB8     MKTAG('R','G','B','8')
/* RGB image forms, Turbo Silver (Impulse) */
#define ID_RGBN     MKTAG('R','G','B','N')
/* RGB image forms, Turbo Silver (Impulse) */
#define ID_PBM      MKTAG('P','B','M',' ')
/* 256-color chunky format (DPaint 2 ?) */
#define ID_ACBM     MKTAG('A','C','B','M')
/* Amiga Contiguous Bitmap (AmigaBasic) */
#define ID_8SVX     MKTAG('8','S','V','X')
/* Amiga 8 bits voice */

/* generic */

#define ID_FVER     MKTAG('F','V','E','R')
/* AmigaOS version string */
#define ID_JUNK     MKTAG('J','U','N','K')
/* always ignore this chunk */
#define ID_ANNO     MKTAG('A','N','N','O')
/* EA IFF 85 Generic Annotation chunk */
#define ID_AUTH     MKTAG('A','U','T','H')
/* EA IFF 85 Generic Author chunk */
#define ID_CHRS     MKTAG('C','H','R','S')
/* EA IFF 85 Generic character string chunk */
#define ID_NAME     MKTAG('N','A','M','E')
/* EA IFF 85 Generic Name of art, music, etc. chunk */
#define ID_TEXT     MKTAG('T','E','X','T')
/* EA IFF 85 Generic unformatted ASCII text chunk */
#define ID_copy     MKTAG('(','c',')',' ')
/* EA IFF 85 Generic Copyright text chunk */

/* ILBM chunks */

#define ID_BMHD     MKTAG('B','M','H','D')
/* ILBM BitmapHeader */
#define ID_CMAP     MKTAG('C','M','A','P')
/* ILBM 8bit RGB colormap */
#define ID_GRAB     MKTAG('G','R','A','B')
/* ILBM "hotspot" coordiantes */
#define ID_DEST     MKTAG('D','E','S','T')
/* ILBM destination image info */
#define ID_SPRT     MKTAG('S','P','R','T')
/* ILBM sprite identifier */
#define ID_CAMG     MKTAG('C','A','M','G')
/* Amiga viewportmodes */
#define ID_BODY     MKTAG('B','O','D','Y')
/* ILBM image data */
#define ID_CRNG     MKTAG('C','R','N','G')
/* color cycling */
#define ID_CCRT     MKTAG('C','C','R','T')
/* color cycling */
#define ID_CLUT     MKTAG('C','L','U','T')
/* Color Lookup Table chunk */
#define ID_DPI      MKTAG('D','P','I',' ')
/* Dots per inch chunk */
#define ID_DPPV     MKTAG('D','P','P','V')
/* DPaint perspective chunk (EA) */
#define ID_DRNG     MKTAG('D','R','N','G')
/* DPaint IV enhanced color cycle chunk (EA) */
#define ID_EPSF     MKTAG('E','P','S','F')
/* Encapsulated Postscript chunk */
#define ID_CMYK     MKTAG('C','M','Y','K')
/* Cyan, Magenta, Yellow, & Black color map (Soft-Logik) */
#define ID_CNAM     MKTAG('C','N','A','M')
/* Color naming chunk (Soft-Logik) */
#define ID_PCHG     MKTAG('P','C','H','G')
/* Line by line palette control information (Sebastiano Vigna) */
#define ID_PRVW     MKTAG('P','R','V','W')
/* A mini duplicate ILBM used for preview (Gary Bonham) */
#define ID_XBMI     MKTAG('X','B','M','I')
/* eXtended BitMap Information (Soft-Logik) */
#define ID_CTBL     MKTAG('C','T','B','L')
/* Newtek Dynamic Ham color chunk */
#define ID_DYCP     MKTAG('D','Y','C','P')
/* Newtek Dynamic Ham chunk */
#define ID_SHAM     MKTAG('S','H','A','M')
/* Sliced HAM color chunk */
#define ID_ABIT     MKTAG('A','B','I','T')
/* ACBM body chunk */
#define ID_DCOL     MKTAG('D','C','O','L')
/* unofficial direct color */
#define ID_DPPS     MKTAG('D','P','P','S')
/* ? */
#define ID_TINY     MKTAG('T','I','N','Y')
/* ? */
#define ID_DPPV     MKTAG('D','P','P','V')
/* ? */

/* 8SVX chunks */

#define ID_VHDR     MKTAG('V','H','D','R')
/* 8SVX Voice8Header */


/**
 *  Represents a IFF chunk available to client code.
 *
 *  Client code must *not* deallocate _stream when done.
 */
struct IFFChunk {
	IFF_ID      _type;
	uint32      _size;
	ReadStream *_stream;

	IFFChunk(IFF_ID type, uint32 size, ReadStream *stream) : _type(type), _size(size), _stream(stream) {
		assert(_stream);
	}
};

/**
 *  Parser for IFF containers.
 */
class IFFParser {

	/**
	 *  This private class implements IFF chunk navigation.
	 */
	class IFFChunkNav : public ReadStream {
	protected:
		ReadStream *_input;
		uint32 _bytesRead;
	public:
		IFF_ID id;
		uint32 size;

		IFFChunkNav() : _input(0) {
		}
		void setInputStream(ReadStream *input) {
			_input = input;
			size = _bytesRead = 0;
		}
		void incBytesRead(uint32 inc) {
			_bytesRead += inc;
			if (_bytesRead > size) {
				error("Chunk overread");
			}
		}
		void readHeader() {
			id = _input->readUint32BE();
			size = _input->readUint32BE();
			_bytesRead = 0;
		}
		bool hasReadAll() const {
			return (size - _bytesRead) == 0;
		}
		void feed() {
			if (size % 2) {
				size++;
			}
			while (!hasReadAll()) {
				readByte();
			}
		}
		// ReadStream implementation
		bool eos() const { return _input->eos(); }
		bool err() const { return _input->err(); }
		void clearErr() { _input->clearErr(); }

		uint32 read(void *dataPtr, uint32 dataSize) {
			incBytesRead(dataSize);
			return _input->read(dataPtr, dataSize);
		}
	};

protected:
	IFFChunkNav _formChunk;	///< The root chunk of the file.
	IFFChunkNav _chunk; 	///< The current chunk.

	uint32 _formSize;
	IFF_ID _formType;

	ReadStream *_stream;
	bool _disposeStream;

	void setInputStream(ReadStream *stream);

public:
	IFFParser(ReadStream *stream, bool disposeStream = false);
	~IFFParser();

	/**
	 * Callback type for the parser.
	 */
	typedef Functor1< IFFChunk&, bool > IFFCallback;

	/**
	 * Parse the IFF container, invoking the callback on each chunk encountered.
	 * The callback can interrupt the parsing by returning 'true'.
	 */
	void parse(IFFCallback &callback);
};


} // namespace Common

#endif
