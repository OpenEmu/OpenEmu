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

#ifndef LASTEXPRESS_SEQUENCE_H
#define LASTEXPRESS_SEQUENCE_H

/*
	Sequence format (.SEQ / .NIS (frame header & data only))

	uint32 {4}    - Number of frames in sequence
	uint32 {4}    - Unknown

	frames headers (68 bytes):
	// for each frame
	    uint32 {4}    - Data offset (from beginning of file)
	    uint32 {4}    - Unknown
	    uint32 {4}    - Palette offset (from beginning of file)
	    uint32 {4}    - Top-left X coordinate
	    uint32 {4}    - Top-left Y coordinate
	    uint32 {4}    - Bottom-right X coordinate
	    uint32 {4}    - Bottom-right Y coordinate
	    uint32 {4}    - Initial offset of decompressed data (doubled, since each pixel occupies one color word)
	    uint32 {4}    - End of data after decompression

	    (for SEQ files only)
	    uint16 {2}    - Hotspot left
	    uint16 {2}    - Hotspot right
	    uint16 {2}    - Hotspot top
	    uint16 {2}    - Hotspot bottom
	    byte {1}      - Compression type
	    byte {1}      - Subtype (determines which set of decompression functions will be called) => 0, 1, 2, 3
	    byte {1}      - Unknown
	    byte {1}      - Keep previous frame while drawing
	    byte {1}      - Unknown
	    byte {1}      - Unknown
	    byte {1}      - Sound action
	    byte {1}      - Unknown
	    uint32 {4}    - positionId
	    uint32 {4}    - Unknown
	    uint16 {2}    - Entity Position
	    uint16 {2}    - Location (~z-order)
	    uint32 {4}    - Next sequence in the linked list

	    (for NIS files: found at 0x124)
	    byte {1}      - Compression type

	palette data:
	    uint16 {x}    - palette data (max size: 256)

	data
	    byte {x}      - compressed image data
*/

#include "lastexpress/drawable.h"

#include "lastexpress/shared.h"

#include "common/array.h"
#include "common/rect.h"
#include "common/str.h"

namespace Common {
class SeekableReadStream;
}

namespace LastExpress {

enum FrameSubType {
	kFrameTypeNone = 0,
	kFrameType1 = 1,
	kFrameType2 = 2,
	kFrameType3 = 3
};

struct FrameInfo {
	void read(Common::SeekableReadStream *in, bool isSequence);

	uint32 dataOffset;            ///< Data offset (from beginning of file)
	uint32 unknown;               ///< FIXME: unknown data
	uint32 paletteOffset;         ///< Palette offset (from beginning of file)
	uint32 xPos1;                 ///< Top-left X coordinate
	uint32 yPos1;                 ///< Top-left Y coordinate
	uint32 xPos2;                 ///< Bottom-right X coordinate
	uint32 yPos2;                 ///< Bottom-right Y coordinate
	uint32 initialSkip;           ///< Initial on-screen offset of decompressed data (doubled, since each pixel occupies one color word)
	uint32 decompressedEndOffset; ///< End of data after decompression

	// NIS frame headers end here. SEQ frame headers have additional 32 bytes of
	// data, notably the compression type at the position outlined above in
	// CompPos_SEQ

	Common::Rect hotspot;

	byte compressionType;         ///< Type of frame compression (0x03, 0x04, 0x05, 0x07, 0xFF)
	FrameSubType subType;         ///< Subtype (byte)

	byte field_2E;
	byte keepPreviousFrame;
	byte field_30;
	byte field_31;
	byte soundAction;
	byte field_33;
	Position position;
	byte field_35;
	int16 field_36;
	uint32 field_38;
	EntityPosition entityPosition;
	uint16 location;
	uint32 next;
};

class AnimFrame : public Drawable {
public:
	AnimFrame(Common::SeekableReadStream *in, const FrameInfo &f, bool ignoreSubtype = false);
	~AnimFrame();
	Common::Rect draw(Graphics::Surface *s);

private:
	void decomp3(Common::SeekableReadStream *in, const FrameInfo &f);
	void decomp4(Common::SeekableReadStream *in, const FrameInfo &f);
	void decomp34(Common::SeekableReadStream *in, const FrameInfo &f, byte mask, byte shift);
	void decomp5(Common::SeekableReadStream *in, const FrameInfo &f);
	void decomp7(Common::SeekableReadStream *in, const FrameInfo &f);
	void decompFF(Common::SeekableReadStream *in, const FrameInfo &f);
	void readPalette(Common::SeekableReadStream *in, const FrameInfo &f);

	Graphics::Surface _image;
	uint16 _palSize;
	uint16 *_palette;
	Common::Rect _rect;
	bool _ignoreSubtype;
};

class Sequence {
public:
	Sequence(Common::String name) : _stream(NULL), _isLoaded(false), _name(name), _field30(15) {}
	~Sequence();

	static Sequence *load(Common::String name, Common::SeekableReadStream *stream = NULL, byte field30 = 15);

	bool load(Common::SeekableReadStream *stream, byte field30 = 15);

	uint16 count() const { return (uint16)_frames.size(); }
	AnimFrame *getFrame(uint16 index = 0);
	FrameInfo *getFrameInfo(uint16 index = 0);

	Common::String getName() { return _name; }
	byte getField30() { return _field30; }

	bool isLoaded() { return _isLoaded; }

private:
	static const uint32 _sequenceHeaderSize = 8;
	static const uint32 _sequenceFrameSize = 68;

	void reset();

	Common::Array<FrameInfo> _frames;
	Common::SeekableReadStream *_stream;
	bool _isLoaded;

	Common::String _name;
	byte _field30; // used when copying sequences
};

class SequenceFrame : public Drawable {
public:
	SequenceFrame(Sequence *sequence, uint16 frame = 0, bool dispose = false) : _sequence(sequence), _frame(frame), _dispose(dispose) {}
	~SequenceFrame();

	Common::Rect draw(Graphics::Surface *surface);

	bool setFrame(uint16 frame);
	uint32 getFrame() { return _frame; }
	bool nextFrame();

	Common::String getName();
	FrameInfo *getInfo();

	bool equal(const SequenceFrame *other) const;

private:
	Sequence *_sequence;
	uint16 _frame;
	bool _dispose;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_SEQUENCE_H
