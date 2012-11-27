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

#include "video/codecs/cdtoons.h"
#include "common/rect.h"
#include "common/stream.h"
#include "common/textconsole.h"
#include "common/array.h"

namespace Video {

struct CDToonsAction {
	uint16 blockId;
	Common::Rect rect;
};

struct CDToonsDiff {
	byte *data;
	uint32 size;
	Common::Rect rect;
};

static Common::Rect readRect(Common::SeekableReadStream *stream) {
	Common::Rect rect;
	rect.top = stream->readUint16BE();
	rect.left = stream->readUint16BE();
	rect.bottom = stream->readUint16BE();
	rect.right = stream->readUint16BE();
	return rect;
}

CDToonsDecoder::CDToonsDecoder(uint16 width, uint16 height) {
	debugN(5, "CDToons: width %d, height %d\n", width, height);

	_surface = new Graphics::Surface();
	_surface->create(width, height, Graphics::PixelFormat::createFormatCLUT8());

	_currentPaletteId = 0;
	memset(_palette, 0, 256 * 3);
	_dirtyPalette = false;
}

CDToonsDecoder::~CDToonsDecoder() {
	_surface->free();
	delete _surface;

	for (Common::HashMap<uint16, CDToonsBlock>::iterator i = _blocks.begin(); i != _blocks.end(); i++)
		delete[] i->_value.data;
}

Graphics::Surface *CDToonsDecoder::decodeImage(Common::SeekableReadStream *stream) {
	uint16 u0 = stream->readUint16BE(); // always 9?
	uint16 frameId = stream->readUint16BE();
	uint16 blocksValidUntil = stream->readUint16BE();
	byte u6 = stream->readByte();
	byte backgroundColor = stream->readByte();
	debugN(5, "CDToons frame %d, size %d, unknown %04x (at 0), blocks valid until %d, unknown 6 is %02x, bkg color is %02x\n",
		frameId, stream->size(), u0, blocksValidUntil, u6, backgroundColor);

	Common::Rect clipRect = readRect(stream);
	debugN(9, "CDToons clipRect: (%d, %d) to (%d, %d)\n",
		clipRect.left, clipRect.top, clipRect.right, clipRect.bottom);

	Common::Rect dirtyRect = readRect(stream);
	debugN(9, "CDToons dirtyRect: (%d, %d) to (%d, %d)\n",
		dirtyRect.left, dirtyRect.top, dirtyRect.right, dirtyRect.bottom);

	uint32 flags = stream->readUint32BE();
	if (flags & 0x80)
		error("CDToons: frame already processed?");
	debugN(5, "CDToons flags: %08x\n", flags);

	uint16 blockCount = stream->readUint16BE();
	uint16 blockOffset = stream->readUint16BE();
	debugN(9, "CDToons: %d blocks at 0x%04x\n",
		blockCount, blockOffset);

	// max block id?
	uint16 u32 = stream->readUint16BE();
	debugN(5, "CDToons unknown at 32: %04x\n", u32);

	byte actionCount = stream->readByte();
	byte u35 = stream->readByte();

	uint16 paletteId = stream->readUint16BE();
	byte paletteSet = stream->readByte();
	debugN(9, "CDToons palette id %04x, palette byte %02x\n",
		paletteId, paletteSet);

	byte u39 = stream->readByte();
	uint16 u40 = stream->readUint16BE();
	uint16 u42 = stream->readUint16BE();
	debugN(5, "CDToons: unknown at 35 is %02x, unknowns at 39: %02x, %04x, %04x\n",
		u35, u39, u40, u42);

	Common::Array<CDToonsAction> actions;

	for (uint i = 0; i < actionCount; i++) {
		CDToonsAction action;
		action.blockId = stream->readUint16BE();
		action.rect = readRect(stream);
		debugN(9, "CDToons action: render block %d at (%d, %d) to (%d, %d)\n",
			action.blockId, action.rect.left, action.rect.top, action.rect.right, action.rect.bottom);
		actions.push_back(action);
	}

	if (stream->pos() > blockOffset)
		error("CDToons header ended at 0x%08x, but blocks should have started at 0x%08x",
			stream->pos(), blockOffset);

	if (stream->pos() != blockOffset)
		error("CDToons had %d unknown bytes after header", blockOffset - stream->pos());

	for (uint i = 0; i < blockCount; i++) {
		uint16 blockId = stream->readUint16BE();
		if (blockId >= 1200)
			error("CDToons: block id %d was too high", blockId);
		if (_blocks.contains(blockId))
			error("CDToons: new block %d was already seen", blockId);

		CDToonsBlock block;
		block.flags = stream->readUint16BE();
		// flag 1 = palette, flag 2 = data?
		if (block.flags & 0x8000)
			error("CDToons: block already processed?");
		block.size = stream->readUint32BE();
		if (block.size < 14)
			error("CDToons: block size was %d, too small", block.size);
		block.size -= 14;
		block.startFrame = stream->readUint16BE();
		block.endFrame = stream->readUint16BE();
		block.unknown12 = stream->readUint16BE();
		block.data = new byte[block.size];
		stream->read(block.data, block.size);

		debugN(9, "CDToons block id 0x%04x of size 0x%08x, flags %04x, from frame %d to %d, unknown at 12 is %04x\n",
			blockId, block.size, block.flags, block.startFrame, block.endFrame, block.unknown12);

		_blocks[blockId] = block;
	}

	byte xFrmBegin = 0, xFrmCount;
	Common::Array<CDToonsDiff> diffs;

	while (true) {
		int32 nextPos = stream->pos();
		uint32 tag = stream->readUint32BE();
		uint32 size = stream->readUint32BE();
		nextPos += size;

		switch (tag) {
		case MKTAG('D','i','f','f'):
			{
			debugN(5, "CDToons: Diff\n");
			uint16 count = stream->readUint16BE();

			Common::Rect diffClipRect = readRect(stream);
			debugN(9, "CDToons diffClipRect: (%d, %d) to (%d, %d)\n",
				diffClipRect.left, diffClipRect.top, diffClipRect.right, diffClipRect.bottom);

			debugN(5, "CDToons Diff: %d subentries\n", count);
			for (uint i = 0; i < count; i++) {
				CDToonsDiff diff;

				diff.rect = readRect(stream);
				diff.size = stream->readUint32BE();
				if (diff.size < 20)
					error("CDToons: Diff block size was %d, too small", diff.size);

				uint16 diffWidth = stream->readUint16BE();
				uint16 diffHeight = stream->readUint16BE();
				uint16 unknown16 = stream->readUint16BE();
				uint16 unknown18 = stream->readUint16BE();
				diff.size -= 8;

				if (diffWidth != diff.rect.width() || diffHeight != diff.rect.height())
					error("CDToons: Diff sizes didn't match");
				debugN(5, "CDToons Diff: size %d, frame from (%d, %d) to (%d, %d), unknowns %04x, %04x\n",
					diff.size, diff.rect.left, diff.rect.top, diff.rect.right, diff.rect.bottom,
					unknown16, unknown18);

				diff.data = new byte[diff.size];
				stream->read(diff.data, diff.size);
				diffs.push_back(diff);
			}
			}
			break;
		case MKTAG('X','F','r','m'):
			{
			debugN(5, "CDToons: XFrm\n");
			if (!(flags & 0x10))
				error("CDToons: useless XFrm?");

			if (xFrmBegin)
				error("CDToons: duplicate XFrm");
			xFrmBegin = stream->readByte();
			xFrmCount = stream->readByte();
			debugN(9, "CDToons XFrm: run %d actions from %d\n", xFrmCount, xFrmBegin - 1);

			// TODO: don't ignore (if xFrmCount is non-zero)
			Common::Rect dirtyRectXFrm = readRect(stream);
			debugN(9, "CDToons XFrm dirtyRect: (%d, %d) to (%d, %d)\n",
				dirtyRectXFrm.left, dirtyRectXFrm.top, dirtyRectXFrm.right, dirtyRectXFrm.bottom);

			// always zero?
			Common::Rect dirtyRect2XFrm = readRect(stream);
			debugN(9, "CDToons XFrm dirtyRect2: (%d, %d) to (%d, %d)\n",
				dirtyRect2XFrm.left, dirtyRect2XFrm.top, dirtyRect2XFrm.right, dirtyRect2XFrm.bottom);
			}
			break;
		case MKTAG('M','r','k','s'):
			debugN(5, "CDToons: Mrks\n");
			if (!(flags & 0x2))
				error("CDToons: useless Mrks?");

			// TODO
			warning("CDToons: encountered Mrks, not implemented yet");
			break;
		case MKTAG('S','c','a','l'):
			// TODO
			warning("CDToons: encountered Scal, not implemented yet");
			break;
		case MKTAG('W','r','M','p'):
			warning("CDToons: encountered WrMp, ignoring");
			break;
		case MKTAG('F','r','t','R'):
			{
			debugN(5, "CDToons: FrtR\n");
			if (!(flags & 0x40))
				error("CDToons: useless FrtR?");

			uint16 count = stream->readUint16BE();
			debugN(9, "CDToons FrtR: %d dirty rectangles\n", count);
			for (uint i = 0; i < count; i++) {
				Common::Rect dirtyRectFrtR = readRect(stream);
				debugN(9, "CDToons FrtR dirtyRect: (%d, %d) to (%d, %d)\n",
					dirtyRectFrtR.left, dirtyRectFrtR.top, dirtyRectFrtR.right, dirtyRectFrtR.bottom);
			}
			}
			break;
		case MKTAG('B','c','k','R'):
			{
			debugN(5, "CDToons: BckR\n");
			if (!(flags & 0x20))
				error("CDToons: useless BckR?");

			uint16 count = stream->readUint16BE();
			debugN(9, "CDToons BckR: %d subentries\n", count);
			for (uint i = 0; i < count; i++) {
				Common::Rect dirtyRectBckR = readRect(stream);
				debugN(9, "CDToons BckR dirtyRect: (%d, %d) to (%d, %d)\n",
					dirtyRectBckR.left, dirtyRectBckR.top, dirtyRectBckR.right, dirtyRectBckR.bottom);
			}
			}
			break;
		default:
			warning("Unknown CDToons tag '%s'", tag2str(tag));
		}

		if (stream->pos() > nextPos)
			error("CDToons ran off the end of a block while reading it (at %d, next block at %d)",
				stream->pos(), nextPos);
		if (stream->pos() != nextPos) {
			warning("CDToons had %d unknown bytes after block", nextPos - stream->pos());
			stream->seek(nextPos);
		}

		if (stream->pos() == stream->size())
			break;
	}

	for (uint i = 0; i < diffs.size(); i++) {
		renderBlock(diffs[i].data, diffs[i].size, diffs[i].rect.left, diffs[i].rect.top, diffs[i].rect.width(), diffs[i].rect.height());
		delete[] diffs[i].data;
	}
	if (!diffs.empty())
		return _surface;

	for (uint i = 0; i < actions.size(); i++) {
		CDToonsAction &action = actions[i];
		if (i == 0 && action.blockId == 0)
			memset(_surface->pixels, backgroundColor, _surface->w * _surface->h);
		if (!_blocks.contains(action.blockId))
			continue;
		if (!action.rect.right)
			continue;
		if (i == 0 && !diffs.empty())
			continue;

		CDToonsBlock &block = _blocks[action.blockId];
		uint16 width = READ_BE_UINT16(block.data + 2);
		uint16 height = READ_BE_UINT16(block.data);

		renderBlock(block.data + 14, block.size - 14, action.rect.left, action.rect.top, width, height);
	}

	if (paletteId && _currentPaletteId != paletteId) {
		if (!_blocks.contains(paletteId))
			error("CDToons: no block for palette %04x", paletteId);
		if (_blocks[paletteId].size != 2 * 3 * 256)
			error("CDToons: palette %04x is wrong size (%d)", paletteId, _blocks[paletteId].size);

		_currentPaletteId = paletteId;
		if (!paletteSet)
			setPalette(_blocks[paletteId].data);
	}

	return _surface;
}

void CDToonsDecoder::renderBlock(byte *data, uint dataSize, int destX, int destY, uint width, uint height) {
	byte *currData = data;
	byte *dataEnd = data + dataSize;

	debugN(9, "CDToons renderBlock at (%d, %d), width %d, height %d\n",
		destX, destY, width, height);

	if (destX + width > _surface->w)
		width = _surface->w - destX;
	if (destY + height > _surface->h)
		height = _surface->h - destY;

	uint skip = 0;
	if (destX < 0) {
		skip = -destX;
		if (width <= skip)
			return;
		width -= skip;
		destX = 0;
	}

	for (uint y = 0; y < height; y++) {
		if (destY + (int)y >= _surface->h)
			break;

		if (currData + 2 > dataEnd)
			error("CDToons renderBlock overran whole data by %d bytes", (uint32)(currData - dataEnd));

		uint16 lineSize = READ_BE_UINT16(currData);
		currData += 2;
		byte *nextLine = currData + lineSize;

		if (nextLine > dataEnd)
			error("CDToons renderBlock was going to overrun data by %d bytes (line size %d)",
				(uint32)(nextLine - dataEnd), (uint32)(nextLine - currData));

		if (destY + (int)y < 0) {
			currData = nextLine;
			continue;
		}

		byte *pixels = (byte *)_surface->getBasePtr(destX, destY + y);

		int leftToSkip = skip;
		uint x = 0;
		bool done = false;
		while (x < width && !done) {
			int size = (uint)*currData;
			currData++;
			bool raw = !(size & 0x80);
			size = (size & 0x7f) + 1;

			if (leftToSkip) {
				if (leftToSkip >= size) {
					leftToSkip -= size;
					if (raw)
						currData += size;
					else
						currData++;
					continue;
				} else {
					size -= leftToSkip;
					if (raw)
						currData += leftToSkip;
					leftToSkip = 0;
				}
			}

			if (x + size >= width) {
				size = width - x;
				done = true;
			}
			if (destX + (int)x + size >= (int)_surface->w) {
				size = MIN<int>((int)_surface->w - destX - (int)x, width - x);
				done = true;
			}
			if (size <= 0) {
				size = 0;
				done = true;
			}

			if (raw) {
				memcpy(pixels + x, currData, size);
				currData += size;
				x += size;
			} else {
				byte color = *currData;
				currData++;
				if (color) {
					memset(pixels + x, color, size);
				}
				x += size;
			}

			if (currData > nextLine) {
				warning("CDToons renderBlock overran line by %d bytes", (uint32)(currData - nextLine));
				return;
			}
		}

		currData = nextLine;
	}
}

void CDToonsDecoder::setPalette(byte *data) {
	_dirtyPalette = true;

	// A lovely QuickTime palette
	for (uint i = 0; i < 256; i++) {
		_palette[i * 3]     = *data;
		_palette[i * 3 + 1] = *(data + 2);
		_palette[i * 3 + 2] = *(data + 4);
		data += 6;
	}

	_palette[0] = _palette[1] = _palette[2] = 0;
}

} // End of namespace Video
