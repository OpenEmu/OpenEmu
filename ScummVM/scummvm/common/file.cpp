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

#include "common/archive.h"
#include "common/debug.h"
#include "common/file.h"
#include "common/fs.h"
#include "common/textconsole.h"

namespace Common {

File::File()
	: _handle(0) {
}

File::~File() {
	close();
}

bool File::open(const String &filename) {
	return open(filename, SearchMan);
}

bool File::open(const String &filename, Archive &archive) {
	assert(!filename.empty());
	assert(!_handle);

	SeekableReadStream *stream = 0;

	if ((stream = archive.createReadStreamForMember(filename))) {
		debug(8, "Opening hashed: %s", filename.c_str());
	} else if ((stream = archive.createReadStreamForMember(filename + "."))) {
		// WORKAROUND: Bug #1458388: "SIMON1: Game Detection fails"
		// sometimes instead of "GAMEPC" we get "GAMEPC." (note trailing dot)
		debug(8, "Opening hashed: %s.", filename.c_str());
	}

	return open(stream, filename);
}

bool File::open(const FSNode &node) {
	assert(!_handle);

	if (!node.exists()) {
		warning("File::open: '%s' does not exist", node.getPath().c_str());
		return false;
	} else if (node.isDirectory()) {
		warning("File::open: '%s' is a directory", node.getPath().c_str());
		return false;
	}

	SeekableReadStream *stream = node.createReadStream();
	return open(stream, node.getPath());
}

bool File::open(SeekableReadStream *stream, const String &name) {
	assert(!_handle);

	if (stream) {
		_handle = stream;
		_name = name;
	} else {
		debug(2, "File::open: opening '%s' failed", name.c_str());
	}
	return _handle != NULL;
}


bool File::exists(const String &filename) {
	if (SearchMan.hasFile(filename)) {
		return true;
	} else if (SearchMan.hasFile(filename + ".")) {
		// WORKAROUND: Bug #1458388: "SIMON1: Game Detection fails"
		// sometimes instead of "GAMEPC" we get "GAMEPC." (note trailing dot)
		return true;
	}

	return false;
}

void File::close() {
	delete _handle;
	_handle = NULL;
}

bool File::isOpen() const {
	return _handle != NULL;
}

bool File::err() const {
	assert(_handle);
	return _handle->err();
}

void File::clearErr() {
	assert(_handle);
	_handle->clearErr();
}

bool File::eos() const {
	assert(_handle);
	return _handle->eos();
}

int32 File::pos() const {
	assert(_handle);
	return _handle->pos();
}

int32 File::size() const {
	assert(_handle);
	return _handle->size();
}

bool File::seek(int32 offs, int whence) {
	assert(_handle);
	return _handle->seek(offs, whence);
}

uint32 File::read(void *ptr, uint32 len) {
	assert(_handle);
	return _handle->read(ptr, len);
}


DumpFile::DumpFile() : _handle(0) {
}

DumpFile::~DumpFile() {
	close();
}

bool DumpFile::open(const String &filename) {
	assert(!filename.empty());
	assert(!_handle);

	FSNode node(filename);
	return open(node);
}

bool DumpFile::open(const FSNode &node) {
	assert(!_handle);

	if (node.isDirectory()) {
		warning("DumpFile::open: FSNode is a directory");
		return false;
	}

	_handle = node.createWriteStream();

	if (_handle == NULL)
		debug(2, "File %s not found", node.getName().c_str());

	return _handle != NULL;
}

void DumpFile::close() {
	delete _handle;
	_handle = NULL;
}

bool DumpFile::isOpen() const {
	return _handle != NULL;
}

bool DumpFile::err() const {
	assert(_handle);
	return _handle->err();
}

void DumpFile::clearErr() {
	assert(_handle);
	_handle->clearErr();
}

uint32 DumpFile::write(const void *ptr, uint32 len) {
	assert(_handle);
	return _handle->write(ptr, len);
}

bool DumpFile::flush() {
	assert(_handle);
	return _handle->flush();
}

}	// End of namespace Common
