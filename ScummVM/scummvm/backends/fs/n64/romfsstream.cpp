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

#ifdef __N64__

#include <romfs.h>
#include "backends/fs/n64/romfsstream.h"

RomfsStream::RomfsStream(void *handle) : _handle(handle) {
	assert(handle);
}

RomfsStream::~RomfsStream() {
	romfs_close((ROMFILE *)_handle);
}

bool RomfsStream::err() const {
	return romfs_error((ROMFILE *)_handle) != 0;
}

void RomfsStream::clearErr() {
	romfs_clearerr((ROMFILE *)_handle);
}

bool RomfsStream::eos() const {
	return romfs_eof((ROMFILE *)_handle) != 0;
}

int32 RomfsStream::pos() const {
	return romfs_tell((ROMFILE *)_handle);
}

int32 RomfsStream::size() const {
	int32 oldPos = romfs_tell((ROMFILE *)_handle);
	romfs_seek((ROMFILE *)_handle, 0, SEEK_END);
	int32 length = romfs_tell((ROMFILE *)_handle);
	romfs_seek((ROMFILE *)_handle, oldPos, SEEK_SET);

	return length;
}

bool RomfsStream::seek(int32 offs, int whence) {
	return romfs_seek((ROMFILE *)_handle, offs, whence) >= 0;
}

uint32 RomfsStream::read(void *ptr, uint32 len) {
	return romfs_read((byte *)ptr, 1, len, (ROMFILE *)_handle);
}

uint32 RomfsStream::write(const void *ptr, uint32 len) {
	return romfs_write(ptr, 1, len, (ROMFILE *)_handle);
}

bool RomfsStream::flush() {
	return romfs_flush((ROMFILE *)_handle) == 0;
}

RomfsStream *RomfsStream::makeFromPath(const Common::String &path, bool writeMode) {
	ROMFILE *handle = romfs_open(path.c_str(), writeMode ? "wb" : "rb");

	if (handle)
		return new RomfsStream(handle);
	return 0;
}

#endif /* __N64__ */
