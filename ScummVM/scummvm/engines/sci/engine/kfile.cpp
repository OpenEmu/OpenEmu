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
#include "common/config-manager.h"
#include "common/debug-channels.h"
#include "common/file.h"
#include "common/macresman.h"
#include "common/str.h"
#include "common/savefile.h"
#include "common/system.h"
#include "common/translation.h"

#include "gui/saveload.h"

#include "sci/sci.h"
#include "sci/engine/file.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/savegame.h"
#include "sci/console.h"

namespace Sci {

extern reg_t file_open(EngineState *s, const Common::String &filename, int mode, bool unwrapFilename);
extern FileHandle *getFileFromHandle(EngineState *s, uint handle);
extern int fgets_wrapper(EngineState *s, char *dest, int maxsize, int handle);
extern void listSavegames(Common::Array<SavegameDesc> &saves);
extern int findSavegame(Common::Array<SavegameDesc> &saves, int16 savegameId);

/**
 * Writes the cwd to the supplied address and returns the address in acc.
 */
reg_t kGetCWD(EngineState *s, int argc, reg_t *argv) {
	// We do not let the scripts see the file system, instead pretending
	// we are always in the same directory.
	// TODO/FIXME: Is "/" a good value? Maybe "" or "." or "C:\" are better?
	s->_segMan->strcpy(argv[0], "/");

	debugC(kDebugLevelFile, "kGetCWD() -> %s", "/");

	return argv[0];
}

enum {
	K_DEVICE_INFO_GET_DEVICE = 0,
	K_DEVICE_INFO_GET_CURRENT_DEVICE = 1,
	K_DEVICE_INFO_PATHS_EQUAL = 2,
	K_DEVICE_INFO_IS_FLOPPY = 3,
	K_DEVICE_INFO_GET_CONFIG_PATH = 5,
	K_DEVICE_INFO_GET_SAVECAT_NAME = 7,
	K_DEVICE_INFO_GET_SAVEFILE_NAME = 8
};

reg_t kDeviceInfo(EngineState *s, int argc, reg_t *argv) {
	if (g_sci->getGameId() == GID_FANMADE && argc == 1) {
		// WORKAROUND: The fan game script library calls kDeviceInfo with one parameter.
		// According to the scripts, it wants to call CurDevice. However, it fails to
		// provide the subop to the function.
		s->_segMan->strcpy(argv[0], "/");
		return s->r_acc;
	}

	int mode = argv[0].toUint16();

	switch (mode) {
	case K_DEVICE_INFO_GET_DEVICE: {
		Common::String input_str = s->_segMan->getString(argv[1]);

		s->_segMan->strcpy(argv[2], "/");
		debug(3, "K_DEVICE_INFO_GET_DEVICE(%s) -> %s", input_str.c_str(), "/");
		break;
	}
	case K_DEVICE_INFO_GET_CURRENT_DEVICE:
		s->_segMan->strcpy(argv[1], "/");
		debug(3, "K_DEVICE_INFO_GET_CURRENT_DEVICE() -> %s", "/");
		break;

	case K_DEVICE_INFO_PATHS_EQUAL: {
		Common::String path1_s = s->_segMan->getString(argv[1]);
		Common::String path2_s = s->_segMan->getString(argv[2]);
		debug(3, "K_DEVICE_INFO_PATHS_EQUAL(%s,%s)", path1_s.c_str(), path2_s.c_str());

		return make_reg(0, Common::matchString(path2_s.c_str(), path1_s.c_str(), false, true));
		}
		break;

	case K_DEVICE_INFO_IS_FLOPPY: {
		Common::String input_str = s->_segMan->getString(argv[1]);
		debug(3, "K_DEVICE_INFO_IS_FLOPPY(%s)", input_str.c_str());
		return NULL_REG; /* Never */
	}
	case K_DEVICE_INFO_GET_CONFIG_PATH: {
		// Early versions return drive letter, later versions a path string
		// FIXME: Implement if needed, for now return NULL_REG
		return NULL_REG;
	}
	/* SCI uses these in a less-than-portable way to delete savegames.
	** Read http://www-plan.cs.colorado.edu/creichen/freesci-logs/2005.10/log20051019.html
	** for more information on our workaround for this.
	*/
	case K_DEVICE_INFO_GET_SAVECAT_NAME: {
		Common::String game_prefix = s->_segMan->getString(argv[2]);
		s->_segMan->strcpy(argv[1], "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVECAT_NAME(%s) -> %s", game_prefix.c_str(), "__throwaway");
		}

	break;
	case K_DEVICE_INFO_GET_SAVEFILE_NAME: {
		Common::String game_prefix = s->_segMan->getString(argv[2]);
		uint virtualId = argv[3].toUint16();
		s->_segMan->strcpy(argv[1], "__throwaway");
		debug(3, "K_DEVICE_INFO_GET_SAVEFILE_NAME(%s,%d) -> %s", game_prefix.c_str(), virtualId, "__throwaway");
		if ((virtualId < SAVEGAMEID_OFFICIALRANGE_START) || (virtualId > SAVEGAMEID_OFFICIALRANGE_END))
			error("kDeviceInfo(deleteSave): invalid savegame ID specified");
		uint savegameId = virtualId - SAVEGAMEID_OFFICIALRANGE_START;
		Common::Array<SavegameDesc> saves;
		listSavegames(saves);
		if (findSavegame(saves, savegameId) != -1) {
			// Confirmed that this id still lives...
			Common::String filename = g_sci->getSavegameName(savegameId);
			Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
			saveFileMan->removeSavefile(filename);
		}
		break;
	}

	default:
		error("Unknown DeviceInfo() sub-command: %d", mode);
		break;
	}

	return s->r_acc;
}

reg_t kCheckFreeSpace(EngineState *s, int argc, reg_t *argv) {
	if (argc > 1) {
		// SCI1.1/SCI32
		// TODO: don't know if those are right for SCI32 as well
		// Please note that sierra sci supported both calls either w/ or w/o opcode in SCI1.1
		switch (argv[1].toUint16()) {
		case 0: // return saved game size
			return make_reg(0, 0); // we return 0

		case 1: // return free harddisc space (shifted right somehow)
			return make_reg(0, 0x7fff); // we return maximum

		case 2: // same as call w/o opcode
			break;
			return make_reg(0, 1);

		default:
			error("kCheckFreeSpace: called with unknown sub-op %d", argv[1].toUint16());
		}
	}

	Common::String path = s->_segMan->getString(argv[0]);

	debug(3, "kCheckFreeSpace(%s)", path.c_str());
	// We simply always pretend that there is enough space. The alternative
	// would be to write a big test file, which is not nice on systems where
	// doing so is very slow.
	return make_reg(0, 1);
}

reg_t kValidPath(EngineState *s, int argc, reg_t *argv) {
	Common::String path = s->_segMan->getString(argv[0]);

	debug(3, "kValidPath(%s) -> %d", path.c_str(), s->r_acc.getOffset());

	// Always return true
	return make_reg(0, 1);
}

#ifdef ENABLE_SCI32

reg_t kCD(EngineState *s, int argc, reg_t *argv) {
	// TODO: Stub
	switch (argv[0].toUint16()) {
	case 0:
		if (argc == 1) {
			// Check if a disc is in the drive
			return TRUE_REG;
		} else {
			// Check if the specified disc is in the drive
			// and return the current disc number. We just
			// return the requested disc number.
			return argv[1];
		}
	case 1:
		// Return the current CD number
		return make_reg(0, 1);
	default:
		warning("CD(%d)", argv[0].toUint16());
	}

	return NULL_REG;
}

#endif

// ---- FileIO operations -----------------------------------------------------

reg_t kFileIO(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kFileIOOpen(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);

	// SCI32 can call K_FILEIO_OPEN with only one argument. It seems to
	// just be checking if it exists.
	int mode = (argc < 2) ? (int)_K_FILE_MODE_OPEN_OR_FAIL : argv[1].toUint16();
	bool unwrapFilename = true;

	// SQ4 floppy prepends /\ to the filenames
	if (name.hasPrefix("/\\")) {
		name.deleteChar(0);
		name.deleteChar(0);
	}

	// SQ4 floppy attempts to update the savegame index file sq4sg.dir when
	// deleting saved games. We don't use an index file for saving or loading,
	// so just stop the game from modifying the file here in order to avoid
	// having it saved in the ScummVM save directory.
	if (name == "sq4sg.dir") {
		debugC(kDebugLevelFile, "Not opening unused file sq4sg.dir");
		return SIGNAL_REG;
	}

	if (name.empty()) {
		// Happens many times during KQ1 (e.g. when typing something)
		debugC(kDebugLevelFile, "Attempted to open a file with an empty filename");
		return SIGNAL_REG;
	}
	debugC(kDebugLevelFile, "kFileIO(open): %s, 0x%x", name.c_str(), mode);

#ifdef ENABLE_SCI32
	if (name == PHANTASMAGORIA_SAVEGAME_INDEX) {
		if (s->_virtualIndexFile) {
			return make_reg(0, VIRTUALFILE_HANDLE);
		} else {
			Common::String englishName = g_sci->getSciLanguageString(name, K_LANG_ENGLISH);
			Common::String wrappedName = g_sci->wrapFilename(englishName);
			if (!g_sci->getSaveFileManager()->listSavefiles(wrappedName).empty()) {
				s->_virtualIndexFile = new VirtualIndexFile(wrappedName);
				return make_reg(0, VIRTUALFILE_HANDLE);
			}
		}
	}

	// Shivers is trying to store savegame descriptions and current spots in
	// separate .SG files, which are hardcoded in the scripts.
	// Essentially, there is a normal save file, created by the executable
	// and an extra hardcoded save file, created by the game scripts, probably
	// because they didn't want to modify the save/load code to add the extra
	// information.
	// Each slot in the book then has two strings, the save description and a
	// description of the current spot that the player is at. Currently, the
	// spot strings are always empty (probably related to the unimplemented
	// kString subop 14, which gets called right before this call).
	// For now, we don't allow the creation of these files, which means that
	// all the spot descriptions next to each slot description will be empty
	// (they are empty anyway). Until a viable solution is found to handle these
	// extra files and until the spot description strings are initialized
	// correctly, we resort to virtual files in order to make the load screen
	// useable. Without this code it is unusable, as the extra information is
	// always saved to 0.SG for some reason, but on restore the correct file is
	// used. Perhaps the virtual ID is not taken into account when saving.
	//
	// Future TODO: maintain spot descriptions and show them too, ideally without
	// having to return to this logic of extra hardcoded files.
	if (g_sci->getGameId() == GID_SHIVERS && name.hasSuffix(".SG")) {
		if (mode == _K_FILE_MODE_OPEN_OR_CREATE || mode == _K_FILE_MODE_CREATE) {
			// Game scripts are trying to create a file with the save
			// description, stop them here
			debugC(kDebugLevelFile, "Not creating unused file %s", name.c_str());
			return SIGNAL_REG;
		} else if (mode == _K_FILE_MODE_OPEN_OR_FAIL) {
			// Create a virtual file containing the save game description
			// and slot number, as the game scripts expect.
			int slotNumber;
			sscanf(name.c_str(), "%d.SG", &slotNumber);

			Common::Array<SavegameDesc> saves;
			listSavegames(saves);
			int savegameNr = findSavegame(saves, slotNumber - SAVEGAMEID_OFFICIALRANGE_START);

			if (!s->_virtualIndexFile) {
				// Make the virtual file buffer big enough to avoid having it grow dynamically.
				// 50 bytes should be more than enough.
				s->_virtualIndexFile = new VirtualIndexFile(50);
			}

			s->_virtualIndexFile->seek(0, SEEK_SET);
			s->_virtualIndexFile->write(saves[savegameNr].name, strlen(saves[savegameNr].name));
			s->_virtualIndexFile->write("\0", 1);
			s->_virtualIndexFile->write("\0", 1);	// Spot description (empty)
			s->_virtualIndexFile->seek(0, SEEK_SET);
			return make_reg(0, VIRTUALFILE_HANDLE);
		}
	}
#endif

	// QFG import rooms get a virtual filelisting instead of an actual one
	if (g_sci->inQfGImportRoom()) {
		// We need to find out what the user actually selected, "savedHeroes" is
		// already destroyed when we get here. That's why we need to remember
		// selection via kDrawControl.
		name = s->_dirseeker.getVirtualFilename(s->_chosenQfGImportItem);
		unwrapFilename = false;
	}

	return file_open(s, name, mode, unwrapFilename);
}

reg_t kFileIOClose(EngineState *s, int argc, reg_t *argv) {
	debugC(kDebugLevelFile, "kFileIO(close): %d", argv[0].toUint16());

	if (argv[0] == SIGNAL_REG)
		return s->r_acc;

	uint16 handle = argv[0].toUint16();

#ifdef ENABLE_SCI32
	if (handle == VIRTUALFILE_HANDLE) {
		s->_virtualIndexFile->close();
		return SIGNAL_REG;
	}
#endif

	FileHandle *f = getFileFromHandle(s, handle);
	if (f) {
		f->close();
		if (getSciVersion() <= SCI_VERSION_0_LATE)
			return s->r_acc;	// SCI0 semantics: no value returned
		return SIGNAL_REG;
	}

	if (getSciVersion() <= SCI_VERSION_0_LATE)
		return s->r_acc;	// SCI0 semantics: no value returned
	return NULL_REG;
}

reg_t kFileIOReadRaw(EngineState *s, int argc, reg_t *argv) {
	uint16 handle = argv[0].toUint16();
	uint16 size = argv[2].toUint16();
	int bytesRead = 0;
	char *buf = new char[size];
	debugC(kDebugLevelFile, "kFileIO(readRaw): %d, %d", handle, size);

#ifdef ENABLE_SCI32
	if (handle == VIRTUALFILE_HANDLE) {
		bytesRead = s->_virtualIndexFile->read(buf, size);
	} else {
#endif
		FileHandle *f = getFileFromHandle(s, handle);
		if (f)
			bytesRead = f->_in->read(buf, size);
#ifdef ENABLE_SCI32
	}
#endif

	// TODO: What happens if less bytes are read than what has
	// been requested? (i.e. if bytesRead is non-zero, but still
	// less than size)
	if (bytesRead > 0)
		s->_segMan->memcpy(argv[1], (const byte*)buf, size);

	delete[] buf;
	return make_reg(0, bytesRead);
}

reg_t kFileIOWriteRaw(EngineState *s, int argc, reg_t *argv) {
	uint16 handle = argv[0].toUint16();
	uint16 size = argv[2].toUint16();
	char *buf = new char[size];
	bool success = false;
	s->_segMan->memcpy((byte *)buf, argv[1], size);
	debugC(kDebugLevelFile, "kFileIO(writeRaw): %d, %d", handle, size);

#ifdef ENABLE_SCI32
	if (handle == VIRTUALFILE_HANDLE) {
		s->_virtualIndexFile->write(buf, size);
		success = true;
	} else {
#endif
		FileHandle *f = getFileFromHandle(s, handle);
		if (f) {
			f->_out->write(buf, size);
			success = true;
		}
#ifdef ENABLE_SCI32
	}
#endif

	delete[] buf;
	if (success)
		return NULL_REG;
	return make_reg(0, 6); // DOS - invalid handle
}

reg_t kFileIOUnlink(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	bool result;

	// SQ4 floppy prepends /\ to the filenames
	if (name.hasPrefix("/\\")) {
		name.deleteChar(0);
		name.deleteChar(0);
	}

	// Special case for SQ4 floppy: This game has hardcoded names for all of
	// its savegames, and they are all named "sq4sg.xxx", where xxx is the
	// slot. We just take the slot number here, and delete the appropriate
	// save game.
	if (name.hasPrefix("sq4sg.")) {
		// Special handling for SQ4... get the slot number and construct the
		// save game name.
		int slotNum = atoi(name.c_str() + name.size() - 3);
		Common::Array<SavegameDesc> saves;
		listSavegames(saves);
		int savedir_nr = saves[slotNum].id;
		name = g_sci->getSavegameName(savedir_nr);
		result = saveFileMan->removeSavefile(name);
	} else if (getSciVersion() >= SCI_VERSION_2) {
		// The file name may be already wrapped, so check both cases
		result = saveFileMan->removeSavefile(name);
		if (!result) {
			const Common::String wrappedName = g_sci->wrapFilename(name);
			result = saveFileMan->removeSavefile(wrappedName);
		}

#ifdef ENABLE_SCI32
		if (name == PHANTASMAGORIA_SAVEGAME_INDEX) {
			delete s->_virtualIndexFile;
			s->_virtualIndexFile = 0;
		}
#endif
	} else {
		const Common::String wrappedName = g_sci->wrapFilename(name);
		result = saveFileMan->removeSavefile(wrappedName);
	}

	debugC(kDebugLevelFile, "kFileIO(unlink): %s", name.c_str());
	if (result)
		return NULL_REG;
	return make_reg(0, 2); // DOS - file not found error code
}

reg_t kFileIOReadString(EngineState *s, int argc, reg_t *argv) {
	uint16 maxsize = argv[1].toUint16();
	char *buf = new char[maxsize];
	uint16 handle = argv[2].toUint16();
	debugC(kDebugLevelFile, "kFileIO(readString): %d, %d", handle, maxsize);
	uint32 bytesRead;

#ifdef ENABLE_SCI32
	if (handle == VIRTUALFILE_HANDLE)
		bytesRead = s->_virtualIndexFile->readLine(buf, maxsize);
	else
#endif
		bytesRead = fgets_wrapper(s, buf, maxsize, handle);

	s->_segMan->memcpy(argv[0], (const byte*)buf, maxsize);
	delete[] buf;
	return bytesRead ? argv[0] : NULL_REG;
}

reg_t kFileIOWriteString(EngineState *s, int argc, reg_t *argv) {
	int handle = argv[0].toUint16();
	Common::String str = s->_segMan->getString(argv[1]);
	debugC(kDebugLevelFile, "kFileIO(writeString): %d", handle);

#ifdef ENABLE_SCI32
	if (handle == VIRTUALFILE_HANDLE) {
		s->_virtualIndexFile->write(str.c_str(), str.size());
		return NULL_REG;
	}
#endif

	FileHandle *f = getFileFromHandle(s, handle);

	if (f) {
		f->_out->write(str.c_str(), str.size());
		if (getSciVersion() <= SCI_VERSION_0_LATE)
			return s->r_acc;	// SCI0 semantics: no value returned
		return NULL_REG;
	}

	if (getSciVersion() <= SCI_VERSION_0_LATE)
		return s->r_acc;	// SCI0 semantics: no value returned
	return make_reg(0, 6); // DOS - invalid handle
}

reg_t kFileIOSeek(EngineState *s, int argc, reg_t *argv) {
	uint16 handle = argv[0].toUint16();
	uint16 offset = ABS<int16>(argv[1].toSint16());	// can be negative
	uint16 whence = argv[2].toUint16();
	debugC(kDebugLevelFile, "kFileIO(seek): %d, %d, %d", handle, offset, whence);

#ifdef ENABLE_SCI32
	if (handle == VIRTUALFILE_HANDLE)
		return make_reg(0, s->_virtualIndexFile->seek(offset, whence));
#endif

	FileHandle *f = getFileFromHandle(s, handle);

	if (f && f->_in) {
		// Backward seeking isn't supported in zip file streams, thus adapt the
		// parameters accordingly if games ask for such a seek mode. A known
		// case where this is requested is the save file manager in Phantasmagoria
		if (whence == SEEK_END) {
			whence = SEEK_SET;
			offset = f->_in->size() - offset;
		}

		return make_reg(0, f->_in->seek(offset, whence));
	} else if (f && f->_out) {
		error("kFileIOSeek: Unsupported seek operation on a writeable stream (offset: %d, whence: %d)", offset, whence);
	}

	return SIGNAL_REG;
}

reg_t kFileIOFindFirst(EngineState *s, int argc, reg_t *argv) {
	Common::String mask = s->_segMan->getString(argv[0]);
	reg_t buf = argv[1];
	int attr = argv[2].toUint16(); // We won't use this, Win32 might, though...
	debugC(kDebugLevelFile, "kFileIO(findFirst): %s, 0x%x", mask.c_str(), attr);

	// We remove ".*". mask will get prefixed, so we will return all additional files for that gameid
	if (mask == "*.*")
		mask = "*";
	return s->_dirseeker.firstFile(mask, buf, s->_segMan);
}

reg_t kFileIOFindNext(EngineState *s, int argc, reg_t *argv) {
	debugC(kDebugLevelFile, "kFileIO(findNext)");
	return s->_dirseeker.nextFile(s->_segMan);
}

reg_t kFileIOExists(EngineState *s, int argc, reg_t *argv) {
	Common::String name = s->_segMan->getString(argv[0]);

#ifdef ENABLE_SCI32
	// Cache the file existence result for the Phantasmagoria
	// save index file, as the game scripts keep checking for
	// its existence.
	if (name == PHANTASMAGORIA_SAVEGAME_INDEX && s->_virtualIndexFile)
		return TRUE_REG;
#endif

	bool exists = false;

	// Check for regular file
	exists = Common::File::exists(name);

	// Check for a savegame with the name
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	if (!exists)
		exists = !saveFileMan->listSavefiles(name).empty();

	// Try searching for the file prepending "target-"
	const Common::String wrappedName = g_sci->wrapFilename(name);
	if (!exists) {
		exists = !saveFileMan->listSavefiles(wrappedName).empty();
	}

	// SCI2+ debug mode
	if (DebugMan.isDebugChannelEnabled(kDebugLevelDebugMode)) {
		if (!exists && name == "1.scr")		// PQ4
			exists = true;
		if (!exists && name == "18.scr")	// QFG4
			exists = true;
		if (!exists && name == "99.scr")	// GK1, KQ7
			exists = true;
		if (!exists && name == "classes")	// GK2, SQ6, LSL7
			exists = true;
	}

	// Special case for non-English versions of LSL5: The English version of
	// LSL5 calls kFileIO(), case K_FILEIO_OPEN for reading to check if
	// memory.drv exists (which is where the game's password is stored). If
	// it's not found, it calls kFileIO() again, case K_FILEIO_OPEN for
	// writing and creates a new file. Non-English versions call kFileIO(),
	// case K_FILEIO_FILE_EXISTS instead, and fail if memory.drv can't be
	// found. We create a default memory.drv file with no password, so that
	// the game can continue.
	if (!exists && name == "memory.drv") {
		// Create a new file, and write the bytes for the empty password
		// string inside
		byte defaultContent[] = { 0xE9, 0xE9, 0xEB, 0xE1, 0x0D, 0x0A, 0x31, 0x30, 0x30, 0x30 };
		Common::WriteStream *outFile = saveFileMan->openForSaving(wrappedName);
		for (int i = 0; i < 10; i++)
			outFile->writeByte(defaultContent[i]);
		outFile->finalize();
		exists = !outFile->err();	// check whether we managed to create the file.
		delete outFile;
	}

	// Special case for KQ6 Mac: The game checks for two video files to see
	// if they exist before it plays them. Since we support multiple naming
	// schemes for resource fork files, we also need to support that here in
	// case someone has a "HalfDome.bin" file, etc.
	if (!exists && g_sci->getGameId() == GID_KQ6 && g_sci->getPlatform() == Common::kPlatformMacintosh &&
			(name == "HalfDome" || name == "Kq6Movie"))
		exists = Common::MacResManager::exists(name);

	debugC(kDebugLevelFile, "kFileIO(fileExists) %s -> %d", name.c_str(), exists);
	return make_reg(0, exists);
}

reg_t kFileIORename(EngineState *s, int argc, reg_t *argv) {
	Common::String oldName = s->_segMan->getString(argv[0]);
	Common::String newName = s->_segMan->getString(argv[1]);

	// SCI1.1 returns 0 on success and a DOS error code on fail. SCI32
	// returns -1 on fail. We just return -1 for all versions.
	if (g_sci->getSaveFileManager()->renameSavefile(oldName, newName))
		return NULL_REG;
	else
		return SIGNAL_REG;
}

#ifdef ENABLE_SCI32
reg_t kFileIOReadByte(EngineState *s, int argc, reg_t *argv) {
	// Read the byte into the low byte of the accumulator
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (!f)
		return NULL_REG;
	return make_reg(0, (s->r_acc.toUint16() & 0xff00) | f->_in->readByte());
}

reg_t kFileIOWriteByte(EngineState *s, int argc, reg_t *argv) {
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (f)
		f->_out->writeByte(argv[1].toUint16() & 0xff);
	return s->r_acc; // FIXME: does this really not return anything?
}

reg_t kFileIOReadWord(EngineState *s, int argc, reg_t *argv) {
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (!f)
		return NULL_REG;
	return make_reg(0, f->_in->readUint16LE());
}

reg_t kFileIOWriteWord(EngineState *s, int argc, reg_t *argv) {
	FileHandle *f = getFileFromHandle(s, argv[0].toUint16());
	if (f)
		f->_out->writeUint16LE(argv[1].toUint16());
	return s->r_acc; // FIXME: does this really not return anything?
}

reg_t kFileIOCreateSaveSlot(EngineState *s, int argc, reg_t *argv) {
	// Used in Shivers when the user enters his name on the guest book
	// in the beginning to start the game.

	// Creates a new save slot, and returns if the operation was successful

	// Argument 0 denotes the save slot as a negative integer, 2 means "0"
	// Argument 1 is a string, with the file name, obtained from kSave(5).
	// The interpreter checks if it can be written to (by checking for free
	// disk space and write permissions)

	// We don't really use or need any of this...

	uint16 saveSlot = argv[0].toUint16();
	char* fileName = s->_segMan->lookupString(argv[1])->getRawData();
	warning("kFileIOCreateSaveSlot(%d, '%s')", saveSlot, fileName);

	return TRUE_REG;	// slot creation was successful
}

reg_t kFileIOIsValidDirectory(EngineState *s, int argc, reg_t *argv) {
	// Used in Torin's Passage and LSL7 to determine if the directory passed as
	// a parameter (usually the save directory) is valid. We always return true
	// here.
	return TRUE_REG;
}

#endif

// ---- Save operations -------------------------------------------------------

#ifdef ENABLE_SCI32

reg_t kSave(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

#endif

reg_t kSaveGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id;
 	int16 virtualId = argv[1].toSint16();
	int16 savegameId = -1;
	Common::String game_description;
	Common::String version;

	if (argc > 3)
		version = s->_segMan->getString(argv[3]);

	// We check here, we don't want to delete a users save in case we are within a kernel function
	if (s->executionStackBase) {
		warning("kSaveGame - won't save from within kernel function");
		return NULL_REG;
	}

	if (argv[0].isNull()) {
		// Direct call, from a patched Game::save
		if ((argv[1] != SIGNAL_REG) || (!argv[2].isNull()))
			error("kSaveGame: assumed patched call isn't accurate");

		// we are supposed to show a dialog for the user and let him choose where to save
		g_sci->_soundCmd->pauseAll(true); // pause music
		GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
		savegameId = dialog->runModalWithCurrentTarget();
		game_description = dialog->getResultString();
		if (game_description.empty()) {
			// create our own description for the saved game, the user didnt enter it
			game_description = dialog->createDefaultSaveDescription(savegameId);
		}
		delete dialog;
		g_sci->_soundCmd->pauseAll(false); // unpause music ( we can't have it paused during save)
		if (savegameId < 0)
			return NULL_REG;

	} else {
		// Real call from script
		game_id = s->_segMan->getString(argv[0]);
		if (argv[2].isNull())
			error("kSaveGame: called with description being NULL");
		game_description = s->_segMan->getString(argv[2]);

		debug(3, "kSaveGame(%s,%d,%s,%s)", game_id.c_str(), virtualId, game_description.c_str(), version.c_str());

		Common::Array<SavegameDesc> saves;
		listSavegames(saves);

		if ((virtualId >= SAVEGAMEID_OFFICIALRANGE_START) && (virtualId <= SAVEGAMEID_OFFICIALRANGE_END)) {
			// savegameId is an actual Id, so search for it just to make sure
			savegameId = virtualId - SAVEGAMEID_OFFICIALRANGE_START;
			if (findSavegame(saves, savegameId) == -1)
				return NULL_REG;
		} else if (virtualId < SAVEGAMEID_OFFICIALRANGE_START) {
			// virtualId is low, we assume that scripts expect us to create new slot
			if (virtualId == s->_lastSaveVirtualId) {
				// if last virtual id is the same as this one, we assume that caller wants to overwrite last save
				savegameId = s->_lastSaveNewId;
			} else {
				uint savegameNr;
				// savegameId is in lower range, scripts expect us to create a new slot
				for (savegameId = 0; savegameId < SAVEGAMEID_OFFICIALRANGE_START; savegameId++) {
					for (savegameNr = 0; savegameNr < saves.size(); savegameNr++) {
						if (savegameId == saves[savegameNr].id)
							break;
					}
					if (savegameNr == saves.size())
						break;
				}
				if (savegameId == SAVEGAMEID_OFFICIALRANGE_START)
					error("kSavegame: no more savegame slots available");
			}
		} else {
			error("kSaveGame: invalid savegameId used");
		}

		// Save in case caller wants to overwrite last newly created save
		s->_lastSaveVirtualId = virtualId;
		s->_lastSaveNewId = savegameId;
	}

	s->r_acc = NULL_REG;

	Common::String filename = g_sci->getSavegameName(savegameId);
	Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
	Common::OutSaveFile *out;

	out = saveFileMan->openForSaving(filename);
	if (!out) {
		warning("Error opening savegame \"%s\" for writing", filename.c_str());
	} else {
		if (!gamestate_save(s, out, game_description, version)) {
			warning("Saving the game failed");
		} else {
			s->r_acc = TRUE_REG; // save successful
		}

		out->finalize();
		if (out->err()) {
			warning("Writing the savegame failed");
			s->r_acc = NULL_REG; // write failure
		}
		delete out;
	}

	return s->r_acc;
}

reg_t kRestoreGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = !argv[0].isNull() ? s->_segMan->getString(argv[0]) : "";
	int16 savegameId = argv[1].toSint16();
	bool pausedMusic = false;

	debug(3, "kRestoreGame(%s,%d)", game_id.c_str(), savegameId);

	if (argv[0].isNull()) {
		// Direct call, either from launcher or from a patched Game::restore
		if (savegameId == -1) {
			// we are supposed to show a dialog for the user and let him choose a saved game
			g_sci->_soundCmd->pauseAll(true); // pause music
			GUI::SaveLoadChooser *dialog = new GUI::SaveLoadChooser(_("Restore game:"), _("Restore"), false);
			savegameId = dialog->runModalWithCurrentTarget();
			delete dialog;
			if (savegameId < 0) {
				g_sci->_soundCmd->pauseAll(false); // unpause music
				return s->r_acc;
			}
			pausedMusic = true;
		}
		// don't adjust ID of the saved game, it's already correct
	} else {
		if (argv[2].isNull())
			error("kRestoreGame: called with parameter 2 being NULL");
		// Real call from script, we need to adjust ID
		if ((savegameId < SAVEGAMEID_OFFICIALRANGE_START) || (savegameId > SAVEGAMEID_OFFICIALRANGE_END)) {
			warning("Savegame ID %d is not allowed", savegameId);
			return TRUE_REG;
		}
		savegameId -= SAVEGAMEID_OFFICIALRANGE_START;
	}

	s->r_acc = NULL_REG; // signals success

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);
	if (findSavegame(saves, savegameId) == -1) {
		s->r_acc = TRUE_REG;
		warning("Savegame ID %d not found", savegameId);
	} else {
		Common::SaveFileManager *saveFileMan = g_sci->getSaveFileManager();
		Common::String filename = g_sci->getSavegameName(savegameId);
		Common::SeekableReadStream *in;

		in = saveFileMan->openForLoading(filename);
		if (in) {
			// found a savegame file

			gamestate_restore(s, in);
			delete in;

			if (g_sci->getGameId() == GID_MOTHERGOOSE256) {
				// WORKAROUND: Mother Goose SCI1/SCI1.1 does some weird things for
				//  saving a previously restored game.
				// We set the current savedgame-id directly and remove the script
				//  code concerning this via script patch.
				s->variables[VAR_GLOBAL][0xB3].setOffset(SAVEGAMEID_OFFICIALRANGE_START + savegameId);
			}
		} else {
			s->r_acc = TRUE_REG;
			warning("Savegame #%d not found", savegameId);
		}
	}

	if (!s->r_acc.isNull()) {
		// no success?
		if (pausedMusic)
			g_sci->_soundCmd->pauseAll(false); // unpause music
	}

	return s->r_acc;
}

reg_t kGetSaveDir(EngineState *s, int argc, reg_t *argv) {
#ifdef ENABLE_SCI32
	// SCI32 uses a parameter here. It is used to modify a string, stored in a
	// global variable, so that game scripts store the save directory. We
	// don't really set a save game directory, thus not setting the string to
	// anything is the correct thing to do here.
	//if (argc > 0)
	//	warning("kGetSaveDir called with %d parameter(s): %04x:%04x", argc, PRINT_REG(argv[0]));
#endif
		return s->_segMan->getSaveDirPtr();
}

reg_t kCheckSaveGame(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = s->_segMan->getString(argv[0]);
	uint16 virtualId = argv[1].toUint16();

	debug(3, "kCheckSaveGame(%s, %d)", game_id.c_str(), virtualId);

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	// we allow 0 (happens in QfG2 when trying to restore from an empty saved game list) and return false in that case
	if (virtualId == 0)
		return NULL_REG;

	// Find saved-game
	if ((virtualId < SAVEGAMEID_OFFICIALRANGE_START) || (virtualId > SAVEGAMEID_OFFICIALRANGE_END))
		error("kCheckSaveGame: called with invalid savegameId");
	uint savegameId = virtualId - SAVEGAMEID_OFFICIALRANGE_START;
	int savegameNr = findSavegame(saves, savegameId);
	if (savegameNr == -1)
		return NULL_REG;

	// Check for compatible savegame version
	int ver = saves[savegameNr].version;
	if (ver < MINIMUM_SAVEGAME_VERSION || ver > CURRENT_SAVEGAME_VERSION)
		return NULL_REG;

	// Otherwise we assume the savegame is OK
	return TRUE_REG;
}

reg_t kGetSaveFiles(EngineState *s, int argc, reg_t *argv) {
	Common::String game_id = s->_segMan->getString(argv[0]);

	debug(3, "kGetSaveFiles(%s)", game_id.c_str());

	// Scripts ask for current save files, we can assume that if afterwards they ask us to create a new slot they really
	//  mean new slot instead of overwriting the old one
	s->_lastSaveVirtualId = SAVEGAMEID_OFFICIALRANGE_START;

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);
	uint totalSaves = MIN<uint>(saves.size(), MAX_SAVEGAME_NR);

	reg_t *slot = s->_segMan->derefRegPtr(argv[2], totalSaves);

	if (!slot) {
		warning("kGetSaveFiles: %04X:%04X invalid or too small to hold slot data", PRINT_REG(argv[2]));
		totalSaves = 0;
	}

	const uint bufSize = (totalSaves * SCI_MAX_SAVENAME_LENGTH) + 1;
	char *saveNames = new char[bufSize];
	char *saveNamePtr = saveNames;

	for (uint i = 0; i < totalSaves; i++) {
		*slot++ = make_reg(0, saves[i].id + SAVEGAMEID_OFFICIALRANGE_START); // Store the virtual savegame ID ffs. see above
		strcpy(saveNamePtr, saves[i].name);
		saveNamePtr += SCI_MAX_SAVENAME_LENGTH;
	}

	*saveNamePtr = 0; // Terminate list

	s->_segMan->memcpy(argv[1], (byte *)saveNames, bufSize);
	delete[] saveNames;

	return make_reg(0, totalSaves);
}

#ifdef ENABLE_SCI32

reg_t kMakeSaveCatName(EngineState *s, int argc, reg_t *argv) {
	// Normally, this creates the name of the save catalogue/directory to save into.
	// First parameter is the string to save the result into. Second is a string
	// with game parameters. We don't have a use for this at all, as we have our own
	// savegame directory management, thus we always return an empty string.
	return argv[0];
}

reg_t kMakeSaveFileName(EngineState *s, int argc, reg_t *argv) {
	// Creates a savegame name from a slot number. Used when deleting saved games.
	// Param 0: the output buffer (same as in kMakeSaveCatName)
	// Param 1: a string with game parameters, ignored
	// Param 2: the selected slot

	SciString *resultString = s->_segMan->lookupString(argv[0]);
	uint16 virtualId = argv[2].toUint16();
	if ((virtualId < SAVEGAMEID_OFFICIALRANGE_START) || (virtualId > SAVEGAMEID_OFFICIALRANGE_END))
		error("kMakeSaveFileName: invalid savegame ID specified");
	uint saveSlot = virtualId - SAVEGAMEID_OFFICIALRANGE_START;

	Common::Array<SavegameDesc> saves;
	listSavegames(saves);

	Common::String filename = g_sci->getSavegameName(saveSlot);
	resultString->fromString(filename);

	return argv[0];
}

reg_t kAutoSave(EngineState *s, int argc, reg_t *argv) {
	// TODO
	// This is a timer callback, with 1 parameter: the timer object
	// (e.g. "timers").
	// It's used for auto-saving (i.e. save every X minutes, by checking
	// the elapsed time from the timer object)

	// This function has to return something other than 0 to proceed
	return TRUE_REG;
}

#endif

} // End of namespace Sci
