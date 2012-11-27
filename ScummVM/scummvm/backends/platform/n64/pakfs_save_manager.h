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

#ifndef __PAKFS_SAVE_MANAGER__
#define __PAKFS_SAVE_MANAGER__

#include <common/savefile.h>
#include <common/zlib.h>

#include <pakfs.h> // N64 PakFS library

bool pakfs_deleteSaveGame(const char *filename);

class InPAKSave : public Common::InSaveFile {
private:
	PAKFILE *fd;

	uint32 read(void *buf, uint32 cnt);
	bool skip(uint32 offset);
	bool seek(int32 offs, int whence);

public:
	InPAKSave() : fd(NULL) { }

	~InPAKSave() {
		if (fd != NULL)
			pakfs_close(fd);
	}

	bool eos() const {
		return pakfs_eof(fd);
	}
	void clearErr() {
		pakfs_clearerr(fd);
	}
	int32 pos() const {
		return pakfs_tell(fd);
	}
	int32 size() const {
		return fd->size;
	}

	bool readSaveGame(const char *filename) {
		fd = pakfs_open(filename, "r");
		return (fd != NULL);
	}
};

class OutPAKSave : public Common::OutSaveFile {
private:
	PAKFILE *fd;

public:
	uint32 write(const void *buf, uint32 cnt);

	OutPAKSave(const char *_filename) : fd(NULL) {
		fd = pakfs_open(_filename, "w");
	}

	~OutPAKSave() {
		if (fd != NULL) {
			finalize();
			pakfs_close(fd);
			flushCurrentPakData();
		}
	}

	bool err() const {
		if (fd)
			return (pakfs_error(fd) == 1);
		else
			return true;
	}
	void clearErr() {
		pakfs_clearerr(fd);
	}
	void finalize() {
		pakfs_flush(fd);
	}
};

class PAKSaveManager : public Common::SaveFileManager {
public:

	virtual Common::OutSaveFile *openForSaving(const Common::String &filename, bool compress = true) {
		OutPAKSave *s = new OutPAKSave(filename.c_str());
		if (!s->err()) {
			return compress ? Common::wrapCompressedWriteStream(s) : s;
		} else {
			delete s;
			return NULL;
		}
	}

	virtual Common::InSaveFile *openForLoading(const Common::String &filename) {
		InPAKSave *s = new InPAKSave();
		if (s->readSaveGame(filename.c_str())) {
			return Common::wrapCompressedReadStream(s);
		} else {
			delete s;
			return NULL;
		}
	}

	virtual bool removeSavefile(const Common::String &filename) {
		return ::pakfs_deleteSaveGame(filename.c_str());
	}

	virtual Common::StringArray listSavefiles(const Common::String &pattern);
};


#endif
