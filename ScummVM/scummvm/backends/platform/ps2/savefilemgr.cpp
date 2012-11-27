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

#define FORBIDDEN_SYMBOL_EXCEPTION_printf
#define FORBIDDEN_SYMBOL_EXCEPTION_mkdir
#define FORBIDDEN_SYMBOL_EXCEPTION_unistd_h

#include "common/config-manager.h"
#include "common/zlib.h"

// #include "backends/saves/compressed/compressed-saves.h"

#ifdef __USE_LIBMC__
	#include <libmc.h>
#endif

#include "asyncfio.h"
#include "savefilemgr.h"
#include "Gs2dScreen.h"
#include "ps2temp.h"

extern AsyncFio fio;

Ps2SaveFileManager::Ps2SaveFileManager(OSystem_PS2 *system, Gs2dScreen *screen) {
	// _system = system;
	_screen = screen;
}

Ps2SaveFileManager::~Ps2SaveFileManager() {

}


bool Ps2SaveFileManager::mcCheck(const char *path) {
	// Common::FSNode dir(Common::String(path), 1); // FIXED in gcc 3.4.x
	const Common::String str(path);
	Common::FSNode dir(str);

	// int res;

	printf("mcCheck\n");

	if (!dir.exists()) {
		printf("! exist -> create : ");
#ifdef __USE_LIBMC__
		printf("%s\n", path+4);
		// WaitSema(_sema);
		mcSync(0, NULL, NULL);
		mcMkDir(0 /*port*/, 0 /*slot*/,	path+4);
		mcSync(0, NULL, &res);
		printf("sync : %d\n", res);
		// SignalSema(_sema);
#else
		printf("%s\n", path);
		fio.mkdir(path);
#endif
	}

	// TODO: res;
	return true;
}

void Ps2SaveFileManager::mcSplit(char *full, char *game, char *ext) {
	// TODO
}

Common::InSaveFile *Ps2SaveFileManager::openForLoading(const Common::String &filename) {
	Common::FSNode savePath(ConfMan.get("savepath")); // TODO: is this fast?
	Common::SeekableReadStream *sf;

	if (!savePath.exists() || !savePath.isDirectory())
		return NULL;

	// _screen->wantAnim(true);

	if (_getDev(savePath) == MC_DEV) {
	// if (strncmp(savePath.getPath().c_str(), "mc0:", 4) == 0) {
		char path[32];

		// FIXME : hack for indy4 iq-points
		if (filename == "iq-points") {
			mcCheck("mc0:ScummVM/indy4");
			sprintf(path, "mc0:ScummVM/indy4/iq-points");
		}
		// FIXME : hack for bs1 saved games
		else if (filename == "SAVEGAME.INF") {
			mcCheck("mc0:ScummVM/sword1");
			sprintf(path, "mc0:ScummVM/sword1/SAVEGAME.INF");
		}
		else {
			char temp[32];
			printf("MC : filename = %s\n", filename.c_str());
			strcpy(temp, filename.c_str());

			// mcSplit(temp, game, ext);
			char *game = strdup(strtok(temp, "."));
			char *ext = strdup(strtok(NULL, "*"));
			sprintf(path, "mc0:ScummVM/%s", game); // per game path

			// mcCheck(path); // needed on load ?
			sprintf(path, "mc0:ScummVM/%s/%s.sav", game, ext);

			free(game);
			free(ext);
		}


		Common::FSNode file(path);

		if (!file.exists())
			return NULL;

		sf = file.createReadStream();

	} else {
		Common::FSNode file = savePath.getChild(filename);

		if (!file.exists())
			return NULL;

		sf = file.createReadStream();
	}

	// _screen->wantAnim(false);

	return Common::wrapCompressedReadStream(sf);
}

Common::OutSaveFile *Ps2SaveFileManager::openForSaving(const Common::String &filename, bool compress) {
	Common::FSNode savePath(ConfMan.get("savepath")); // TODO: is this fast?
	Common::WriteStream *sf;

	printf("openForSaving : %s\n", filename.c_str());

	if (!savePath.exists() || !savePath.isDirectory())
		return NULL;

	_screen->wantAnim(true);

	if (_getDev(savePath) == MC_DEV) {
	// if (strncmp(savePath.getPath().c_str(), "mc0:", 4) == 0) {
		char path[32];

		// FIXME : hack for indy4 iq-points
		if (filename == "iq-points") {
			mcCheck("mc0:ScummVM/indy4");
			sprintf(path, "mc0:ScummVM/indy4/iq-points");
		}
		// FIXME : hack for bs1 saved games
        else if (filename == "SAVEGAME.INF") {
            mcCheck("mc0:ScummVM/sword1");
            sprintf(path, "mc0:ScummVM/sword1/SAVEGAME.INF");
        }
		else {
			char temp[32];
			strcpy(temp, filename.c_str());

			// mcSplit(temp, game, ext);
			char *game = strdup(strtok(temp, "."));
			char *ext = strdup(strtok(NULL, "*"));
			sprintf(path, "mc0:ScummVM/%s", game); // per game path
			mcCheck(path);
			sprintf(path, "mc0:ScummVM/%s/%s.sav", game, ext);

			free(game);
			free(ext);
		}

		Common::FSNode file(path);
		sf = file.createWriteStream();
	} else {
		Common::FSNode file = savePath.getChild(filename);
		sf = file.createWriteStream();
	}

	_screen->wantAnim(false);
	return compress ? Common::wrapCompressedWriteStream(sf) : sf;
}

bool Ps2SaveFileManager::removeSavefile(const Common::String &filename) {
	Common::FSNode savePath(ConfMan.get("savepath")); // TODO: is this fast?
	Common::FSNode file;

	if (!savePath.exists() || !savePath.isDirectory())
		return false;

	if (_getDev(savePath) == MC_DEV) {
	// if (strncmp(savePath.getPath().c_str(), "mc0:", 4) == 0) {
		char path[32], temp[32];
		strcpy(temp, filename.c_str());

		// mcSplit(temp, game, ext);
		char *game = strdup(strtok(temp, "."));
		char *ext = strdup(strtok(NULL, "*"));
		sprintf(path, "mc0:ScummVM/%s", game); // per game path
		mcCheck(path);
		sprintf(path, "mc0:ScummVM/%s/%s.sav", game, ext);
		file = Common::FSNode(path);
		free(game);
		free(ext);
	} else {
		file = savePath.getChild(filename);
	}

	if (!file.exists() || file.isDirectory())
		return false;

	fio.remove(file.getPath().c_str());

	return true;
}

Common::StringArray Ps2SaveFileManager::listSavefiles(const Common::String &pattern) {
	Common::FSNode savePath(ConfMan.get("savepath")); // TODO: is this fast?
	Common::String _dir;
	Common::String search;
	bool _mc = (_getDev(savePath) == MC_DEV);
 		// (strncmp(savePath.getPath().c_str(), "mc0:", 4) == 0);
	char *game=0, path[32], temp[32];

	if (!savePath.exists() || !savePath.isDirectory())
		return Common::StringArray();

	printf("listSavefiles = %s\n", pattern.c_str());

	if (_mc) {
		strcpy(temp, pattern.c_str());

		// mcSplit(temp, game, ext);
		game = strdup(strtok(temp, "."));
		sprintf(path, "mc0:ScummVM/%s", game); // per game path
		mcCheck(path);

		sprintf(path, "mc0:ScummVM/%s/", game);
		_dir = Common::String(path);
		search = Common::String("*.sav");
	}
	else {
		_dir = Common::String(savePath.getPath());
		search = pattern;
	}

	Common::FSDirectory dir(_dir);
	Common::ArchiveMemberList savefiles;
	Common::StringArray results;

	printf("dir = %s --- reg = %s\n", _dir.c_str(), search.c_str());

	if (dir.listMatchingMembers(savefiles, search) > 0) {
		for (Common::ArchiveMemberList::const_iterator file = savefiles.begin(); file != savefiles.end(); ++file) {
			if (_mc) { // convert them back in ScummVM names
				strncpy(temp, (*file)->getName().c_str(), 3);
				temp[3] = '\0';
				sprintf(path, "%s.%s", game, temp);
				results.push_back(path);
				printf(" --> found = %s -> %s\n", (*file)->getName().c_str(), path);
			}
			else {
				results.push_back((*file)->getName());
				printf(" --> found = %s\n", (*file)->getName().c_str());
			}
		}
	}

	free(game);

	return results;
}
