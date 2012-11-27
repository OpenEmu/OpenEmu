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

#include "sword2/header.h"
#include "sword2/object.h"
#include "sword2/screen.h"
#include "sword2/sword2.h"

#include "common/memstream.h"
#include "common/endian.h"

namespace Sword2 {

void ResHeader::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	fileType = readS.readByte();
	compType = readS.readByte();
	compSize = readS.readUint32LE();
	decompSize = readS.readUint32LE();
	readS.read(name, NAME_LEN);
}

void ResHeader::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeByte(fileType);
	writeS.writeByte(compType);
	writeS.writeUint32LE(compSize);
	writeS.writeUint32LE(decompSize);
	writeS.write(name, NAME_LEN);
}

void AnimHeader::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	if (Sword2Engine::isPsx()) {
		noAnimFrames = readS.readUint16LE();
		feetStartX = readS.readUint16LE();
		feetStartY = readS.readUint16LE();
		feetEndX = readS.readUint16LE();
		feetEndY = readS.readUint16LE();
		blend = readS.readUint16LE();
		runTimeComp = readS.readByte();
		feetStartDir = readS.readByte();
		feetEndDir = readS.readByte();
	} else {
		runTimeComp = readS.readByte();
		noAnimFrames = readS.readUint16LE();
		feetStartX = readS.readUint16LE();
		feetStartY = readS.readUint16LE();
		feetStartDir = readS.readByte();
		feetEndX = readS.readUint16LE();
		feetEndY = readS.readUint16LE();
		feetEndDir = readS.readByte();
		blend = readS.readUint16LE();
	}
}

void AnimHeader::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeByte(runTimeComp);
	writeS.writeUint16LE(noAnimFrames);
	writeS.writeUint16LE(feetStartX);
	writeS.writeUint16LE(feetStartY);
	writeS.writeByte(feetStartDir);
	writeS.writeUint16LE(feetEndX);
	writeS.writeUint16LE(feetEndY);
	writeS.writeByte(feetEndDir);
	writeS.writeUint16LE(blend);
}

int CdtEntry::size() {
	if (Sword2Engine::isPsx())
		return 12;
	else
		return 9;
}

void CdtEntry::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	if (Sword2Engine::isPsx()) {
		readS.readByte(); // Skip a byte in psx version
		x = readS.readUint16LE();
		y = readS.readUint16LE();
		frameOffset = readS.readUint32LE();
		frameType = readS.readByte();
	} else {
		x = readS.readUint16LE();
		y = readS.readUint16LE();
		frameOffset = readS.readUint32LE();
		frameType = readS.readByte();
	}
}

void CdtEntry::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint16LE(x);
	writeS.writeUint16LE(y);
	writeS.writeUint32LE(frameOffset);
	writeS.writeByte(frameType);
}

void FrameHeader::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	compSize = readS.readUint32LE();
	width = readS.readUint16LE();
	height = readS.readUint16LE();

	if (Sword2Engine::isPsx()) { // In PSX version, frames are half height
		height *= 2;
		width = (width % 2) ? width + 1 : width;
	}
}

void FrameHeader::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint32LE(compSize);
	writeS.writeUint16LE(width);
	writeS.writeUint16LE(height);
}

void MultiScreenHeader::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	palette = readS.readUint32LE();
	bg_parallax[0] = readS.readUint32LE();
	bg_parallax[1] = readS.readUint32LE();
	screen = readS.readUint32LE();
	fg_parallax[0] = readS.readUint32LE();
	fg_parallax[1] = readS.readUint32LE();
	layers = readS.readUint32LE();
	paletteTable = readS.readUint32LE();
	maskOffset = readS.readUint32LE();
}

void MultiScreenHeader::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint32LE(palette);
	writeS.writeUint32LE(bg_parallax[0]);
	writeS.writeUint32LE(bg_parallax[1]);
	writeS.writeUint32LE(screen);
	writeS.writeUint32LE(fg_parallax[0]);
	writeS.writeUint32LE(fg_parallax[1]);
	writeS.writeUint32LE(layers);
	writeS.writeUint32LE(paletteTable);
	writeS.writeUint32LE(maskOffset);
}

void ScreenHeader::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	width = readS.readUint16LE();
	height = readS.readUint16LE();
	noLayers = readS.readUint16LE();
}

void ScreenHeader::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint16LE(width);
	writeS.writeUint16LE(height);
	writeS.writeUint16LE(noLayers);
}

void LayerHeader::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	x = readS.readUint16LE();
	y = readS.readUint16LE();
	width = readS.readUint16LE();
	height = readS.readUint16LE();
	maskSize = readS.readUint32LE();
	offset = readS.readUint32LE();
}

void LayerHeader::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint16LE(x);
	writeS.writeUint16LE(y);
	writeS.writeUint16LE(width);
	writeS.writeUint16LE(height);
	writeS.writeUint32LE(maskSize);
	writeS.writeUint32LE(offset);
}

void TextHeader::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	noOfLines = readS.readUint32LE();
}

void TextHeader::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint32LE(noOfLines);
}

void PSXScreensEntry::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	bgPlxXres = readS.readUint16LE();
	bgPlxYres = readS.readUint16LE();
	bgPlxOffset = readS.readUint32LE();
	bgPlxSize = readS.readUint32LE();
	bgXres = readS.readUint16LE();
	bgYres = readS.readUint16LE();
	bgOffset = readS.readUint32LE();
	bgSize = readS.readUint32LE();
	fgPlxXres = readS.readUint16LE();
	fgPlxYres = readS.readUint16LE();
	fgPlxOffset = readS.readUint32LE();
	fgPlxSize = readS.readUint32LE();
}

void PSXScreensEntry::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint16LE(bgPlxXres);
	writeS.writeUint16LE(bgPlxYres);
	writeS.writeUint32LE(bgPlxOffset);
	writeS.writeUint32LE(bgPlxSize);
	writeS.writeUint16LE(bgXres);
	writeS.writeUint16LE(bgYres);
	writeS.writeUint32LE(bgOffset);
	writeS.writeUint32LE(bgSize);
	writeS.writeUint16LE(fgPlxXres);
	writeS.writeUint16LE(fgPlxYres);
	writeS.writeUint32LE(fgPlxOffset);
	writeS.writeUint32LE(fgPlxSize);
}

void PSXFontEntry::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	offset = readS.readUint16LE() / 2;
	skipLines = readS.readUint16LE();
	charWidth = readS.readUint16LE() / 2;
	charHeight = readS.readUint16LE();
}

void PSXFontEntry::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint16LE(offset);
	writeS.writeUint16LE(skipLines);
	writeS.writeUint16LE(charWidth);
	writeS.writeUint16LE(charHeight);
}

void Parallax::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	w = readS.readUint16LE();
	h = readS.readUint16LE();
}

void Parallax::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint16LE(w);
	writeS.writeUint16LE(h);
}

void ObjectMouse::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	x1 = readS.readSint32LE();
	y1 = readS.readSint32LE();
	x2 = readS.readSint32LE();
	y2 = readS.readSint32LE();
	priority = readS.readSint32LE();
	pointer = readS.readSint32LE();
}

void ObjectMouse::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeSint32LE(x1);
	writeS.writeSint32LE(y1);
	writeS.writeSint32LE(x2);
	writeS.writeSint32LE(y2);
	writeS.writeSint32LE(priority);
	writeS.writeSint32LE(pointer);
}

void ObjectWalkdata::read(byte *addr) {
	Common::MemoryReadStream readS(addr, size());

	nWalkFrames = readS.readUint32LE();
	usingStandingTurnFrames = readS.readUint32LE();
	usingWalkingTurnFrames = readS.readUint32LE();
	usingSlowInFrames = readS.readUint32LE();
	usingSlowOutFrames = readS.readUint32LE();

	int i;

	for (i = 0; i < ARRAYSIZE(nSlowInFrames); i++)
		nSlowInFrames[i] = readS.readUint32LE();

	for (i = 0; i < ARRAYSIZE(leadingLeg); i++)
		leadingLeg[i] = readS.readUint32LE();

	for (i = 0; i < ARRAYSIZE(dx); i++)
		dx[i] = readS.readUint32LE();

	for (i = 0; i < ARRAYSIZE(dy); i++)
		dy[i] = readS.readUint32LE();
}

void ObjectWalkdata::write(byte *addr) {
	Common::MemoryWriteStream writeS(addr, size());

	writeS.writeUint32LE(nWalkFrames);
	writeS.writeUint32LE(usingStandingTurnFrames);
	writeS.writeUint32LE(usingWalkingTurnFrames);
	writeS.writeUint32LE(usingSlowInFrames);
	writeS.writeUint32LE(usingSlowOutFrames);

	int i;

	for (i = 0; i < ARRAYSIZE(nSlowInFrames); i++)
		writeS.writeUint32LE(nSlowInFrames[i]);

	for (i = 0; i < ARRAYSIZE(leadingLeg); i++)
		writeS.writeUint32LE(leadingLeg[i]);

	for (i = 0; i < ARRAYSIZE(dx); i++)
		writeS.writeUint32LE(dx[i]);

	for (i = 0; i < ARRAYSIZE(dy); i++)
		writeS.writeUint32LE(dy[i]);
}

} // End of namespace Sword2
