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

#ifndef __FRAMFS_SAVE_MANAGER__
#define __FRAMFS_SAVE_MANAGER__

#include <common/savefile.h>
#include <common/zlib.h>

#include <framfs.h> // N64 FramFS library

bool fram_deleteSaveGame(const char *filename);

class InFRAMSave : public Common::InSaveFile {
private:
	FRAMFILE *fd;

	uint32 read(void *buf, uint32 cnt);
	bool skip(uint32 offset);
	bool seek(int32 offs, int whence);

public:
	InFRAMSave() : fd(NULL) { }

	~InFRAMSave() {
		if (fd != NULL)
			framfs_close(fd);
	}

	bool eos() const {
		return framfs_eof(fd);
	}
	void clearErr() {
		framfs_clearerr(fd);
	}
	int32 pos() const {
		return framfs_tell(fd);
	}
	int32 size() const {
		return fd->size;
	}

	bool readSaveGame(const char *filename) {
		fd = framfs_open(filename, "r");
		return (fd != NULL);
	}
};

class OutFRAMSave : public Common::OutSaveFile {
private:
	FRAMFILE *fd;

public:
	uint32 write(const void *buf, uint32 cnt);

	OutFRAMSave(const char *_filename) : fd(NULL) {
		fd = framfs_open(_filename, "w");
	}

	~OutFRAMSave() {
		if (fd != NULL) {
			finalize();
			framfs_close(fd);
		}
	}

	bool err() const {
		if (fd)
			return (framfs_error(fd) == 1);
		else
			return true;
	}
	void clearErr() {
		framfs_clearerr(fd);
	}
	void finalize() {
		framfs_flush(fd);
	}
};

class FRAMSaveManager : public Common::SaveFileManager {
public:

	virtual Common::OutSaveFile *openForSaving(const Common::String &filename, bool compress = true) {
		OutFRAMSave *s = new OutFRAMSave(filename.c_str());
		if (!s->err()) {
			return compress ? Common::wrapCompressedWriteStream(s) : s;
		} else {
			delete s;
			return 0;
		}
	}

	virtual Common::InSaveFile *openForLoading(const Common::String &filename) {
		InFRAMSave *s = new InFRAMSave();
		if (s->readSaveGame(filename.c_str())) {
			return Common::wrapCompressedReadStream(s);
		} else {
			delete s;
			return 0;
		}
	}

	virtual bool removeSavefile(const Common::String &filename) {
		return ::fram_deleteSaveGame(filename.c_str());
	}

	virtual Common::StringArray listSavefiles(const Common::String &pattern);
};


#endif
