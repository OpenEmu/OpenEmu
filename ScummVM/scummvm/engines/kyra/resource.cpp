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

#include "kyra/resource.h"
#include "kyra/resource_intern.h"

#include "common/config-manager.h"
#include "common/fs.h"

namespace Kyra {

Resource::Resource(KyraEngine_v1 *vm) : _archiveCache(), _files(), _archiveFiles(), _protectedFiles(), _loaders(), _vm(vm) {
	initializeLoaders();

	// Initialize directories for playing from CD or with original
	// directory structure
	if (_vm->game() == GI_KYRA3)
		SearchMan.addSubDirectoryMatching(Common::FSNode(ConfMan.get("path")), "malcolm");

	_files.add("global_search", &Common::SearchManager::instance(), 3, false);
	// compressed installer archives are added at level '2',
	// but that's done in Resource::reset not here
	_files.add("protected", &_protectedFiles, 1, false);
	_files.add("archives", &_archiveFiles, 0, false);
}

Resource::~Resource() {
	_loaders.clear();

	for (ArchiveMap::iterator i = _archiveCache.begin(); i != _archiveCache.end(); ++i)
		delete i->_value;
	_archiveCache.clear();
}

bool Resource::reset() {
	unloadAllPakFiles();

	Common::FSNode dir(ConfMan.get("path"));

	if (!dir.exists() || !dir.isDirectory())
		error("invalid game path '%s'", dir.getPath().c_str());

	if (_vm->game() == GI_KYRA1 || _vm->game() == GI_EOB1) {
		// We only need kyra.dat for the demo.
		if (_vm->gameFlags().isDemo && !_vm->gameFlags().isTalkie)
			return true;

		if (!_vm->gameFlags().isDemo && _vm->gameFlags().isTalkie) {
			// List of files in the talkie version, which can never be unload.
			static const char *const list[] = {
				"ADL.PAK", "CHAPTER1.VRM", "COL.PAK", "FINALE.PAK", "INTRO1.PAK", "INTRO2.PAK",
				"INTRO3.PAK", "INTRO4.PAK", "MISC.PAK", "SND.PAK", "STARTUP.PAK", "XMI.PAK",
				"CAVE.APK", "DRAGON1.APK", "DRAGON2.APK", "LAGOON.APK", 0
			};

			loadProtectedFiles(list);
		} else {
			Common::ArchiveMemberList files;

			_files.listMatchingMembers(files, "*.PAK");
			_files.listMatchingMembers(files, "*.APK");

			for (Common::ArchiveMemberList::const_iterator i = files.begin(); i != files.end(); ++i) {
				Common::String name = (*i)->getName();
				name.toUppercase();

				// No PAK file
				if (name == "TWMUSIC.PAK" || name == "EYE.PAK")
					continue;

				// We need to only load the script archive for the language the user specified
				if (name == ((_vm->gameFlags().lang == Common::EN_ANY) ? "JMC.PAK" : "EMC.PAK"))
					continue;

				Common::Archive *archive = loadArchive(name, *i);
				if (archive)
					_files.add(name, archive, 0, false);
				else
					error("Couldn't load PAK file '%s'", name.c_str());
			}
		}
	} else if (_vm->game() == GI_KYRA2) {
		if (_vm->gameFlags().useInstallerPackage)
			_files.add("installer", loadInstallerArchive("WESTWOOD", "%03d", 6), 2, false);

		// mouse pointer, fonts, etc. required for initialization
		if (_vm->gameFlags().isDemo && !_vm->gameFlags().isTalkie) {
			loadPakFile("GENERAL.PAK");
		} else {
			loadPakFile("INTROGEN.PAK");
			loadPakFile("OTHER.PAK");
		}
	} else if (_vm->game() == GI_KYRA3) {
		if (_vm->gameFlags().useInstallerPackage) {
			if (!loadPakFile("WESTWOOD.001"))
				error("Couldn't load file: 'WESTWOOD.001'");
		}

		if (!loadFileList("FILEDATA.FDT"))
			error("Couldn't load file: 'FILEDATA.FDT'");
	} else if (_vm->game() == GI_LOL) {
		if (_vm->gameFlags().useInstallerPackage)
			_files.add("installer", loadInstallerArchive("WESTWOOD", "%d", 0), 2, false);

		if (!_vm->gameFlags().isTalkie && !_vm->gameFlags().isDemo) {
			static const char *const list[] = {
				"GENERAL.PAK", 0
			};

			loadProtectedFiles(list);
		}
	} else if (_vm->game() != GI_EOB2) {
		error("Unknown game id: %d", _vm->game());
		return false;   // for compilers that don't support NORETURN
	}

	return true;
}

bool Resource::loadPakFile(Common::String filename) {
	filename.toUppercase();

	Common::ArchiveMemberPtr file = _files.getMember(filename);
	if (!file)
		return false;

	return loadPakFile(filename, file);
}

bool Resource::loadPakFile(Common::String name, Common::ArchiveMemberPtr file) {
	name.toUppercase();

	if (_archiveFiles.hasArchive(name) || _protectedFiles.hasArchive(name))
		return true;

	Common::Archive *archive = loadArchive(name, file);
	if (!archive)
		return false;

	_archiveFiles.add(name, archive, 0, false);

	return true;
}

bool Resource::loadFileList(const Common::String &filedata) {
	Common::SeekableReadStream *f = createReadStream(filedata);

	if (!f)
		return false;

	uint32 filenameOffset = 0;
	while ((filenameOffset = f->readUint32LE()) != 0) {
		uint32 offset = f->pos();
		f->seek(filenameOffset, SEEK_SET);

		uint8 buffer[13];
		f->read(buffer, sizeof(buffer) - 1);
		buffer[12] = 0;
		f->seek(offset + 16, SEEK_SET);

		Common::String filename = Common::String((char *)buffer);
		filename.toUppercase();

		if (filename.hasSuffix(".PAK")) {
			if (!exists(filename.c_str()) && _vm->gameFlags().isDemo) {
				// the demo version supplied with Kyra3 does not
				// contain all pak files listed in filedata.fdt
				// so we don't do anything here if they are non
				// existant.
			} else if (!loadPakFile(filename)) {
				delete f;
				error("couldn't load file '%s'", filename.c_str());
				return false;   // for compilers that don't support NORETURN
			}
		}
	}

	delete f;
	return true;
}

bool Resource::loadFileList(const char *const *filelist, uint32 numFiles) {
	if (!filelist)
		return false;

	while (numFiles--) {
		if (!loadPakFile(filelist[numFiles])) {
			error("couldn't load file '%s'", filelist[numFiles]);
			return false;   // for compilers that don't support NORETURN
		}
	}

	return true;
}

bool Resource::loadProtectedFiles(const char *const *list) {
	for (uint i = 0; list[i]; ++i) {
		Common::ArchiveMemberPtr file = _files.getMember(list[i]);
		if (!file)
			error("Couldn't find PAK file '%s'", list[i]);

		Common::Archive *archive = loadArchive(list[i], file);
		if (archive)
			_protectedFiles.add(list[i], archive, 0, false);
		else
			error("Couldn't load PAK file '%s'", list[i]);
	}

	return true;
}

void Resource::unloadPakFile(Common::String filename, bool remFromCache) {
	filename.toUppercase();

	// We do not remove files from '_protectedFiles' here, since
	// those are protected against unloading.
	if (_archiveFiles.hasArchive(filename)) {
		_archiveFiles.remove(filename);
		if (remFromCache) {
			ArchiveMap::iterator iter = _archiveCache.find(filename);
			if (iter != _archiveCache.end()) {
				delete iter->_value;
				_archiveCache.erase(filename);
			}
		}
	}
}

bool Resource::isInPakList(Common::String filename) {
	filename.toUppercase();
	return (_archiveFiles.hasArchive(filename) || _protectedFiles.hasArchive(filename));
}

bool Resource::isInCacheList(Common::String name) {
	name.toUppercase();
	return (_archiveCache.find(name) != _archiveCache.end());
}

void Resource::unloadAllPakFiles() {
	_archiveFiles.clear();
	_protectedFiles.clear();
}

void Resource::listFiles(const Common::String &pattern, Common::ArchiveMemberList &list) {
	_files.listMatchingMembers(list, pattern);
}

uint8 *Resource::fileData(const char *file, uint32 *size) {
	Common::SeekableReadStream *stream = createReadStream(file);
	if (!stream)
		return 0;

	uint32 bufferSize = stream->size();
	uint8 *buffer = new uint8[bufferSize];
	assert(buffer);
	if (size)
		*size = bufferSize;
	stream->read(buffer, bufferSize);
	delete stream;
	return buffer;
}

bool Resource::exists(const char *file, bool errorOutOnFail) {
	if (_files.hasFile(file))
		return true;
	else if (errorOutOnFail)
		error("File '%s' can't be found", file);
	return false;
}

uint32 Resource::getFileSize(const char *file) {
	Common::SeekableReadStream *stream = createReadStream(file);
	if (!stream)
		return 0;

	uint32 size = stream->size();
	delete stream;
	return size;
}

bool Resource::loadFileToBuf(const char *file, void *buf, uint32 maxSize) {
	Common::SeekableReadStream *stream = createReadStream(file);
	if (!stream)
		return false;

	memset(buf, 0, maxSize);
	stream->read(buf, ((int32)maxSize <= stream->size()) ? maxSize : stream->size());
	delete stream;
	return true;
}

Common::SeekableReadStream *Resource::createReadStream(const Common::String &file) {
	return _files.createReadStreamForMember(file);
}

Common::Archive *Resource::loadArchive(const Common::String &name, Common::ArchiveMemberPtr member) {
	ArchiveMap::iterator cachedArchive = _archiveCache.find(name);
	if (cachedArchive != _archiveCache.end())
		return cachedArchive->_value;

	Common::SeekableReadStream *stream = member->createReadStream();

	if (!stream)
		return 0;

	Common::Archive *archive = 0;
	for (LoaderList::const_iterator i = _loaders.begin(); i != _loaders.end(); ++i) {
		if ((*i)->checkFilename(name)) {
			if ((*i)->isLoadable(name, *stream)) {
				stream->seek(0, SEEK_SET);
				archive = (*i)->load(member, *stream);
				break;
			} else {
				stream->seek(0, SEEK_SET);
			}
		}
	}

	delete stream;

	if (!archive)
		return 0;

	_archiveCache[name] = archive;
	return archive;
}

Common::Archive *Resource::loadInstallerArchive(const Common::String &file, const Common::String &ext, const uint8 offset) {
	ArchiveMap::iterator cachedArchive = _archiveCache.find(file);
	if (cachedArchive != _archiveCache.end())
		return cachedArchive->_value;

	Common::Archive *archive = InstallerLoader::load(this, file, ext, offset);
	if (!archive)
		return 0;

	_archiveCache[file] = archive;
	return archive;
}

#pragma mark -

void Resource::initializeLoaders() {
	_loaders.push_back(LoaderList::value_type(new ResLoaderPak()));
	_loaders.push_back(LoaderList::value_type(new ResLoaderInsMalcolm()));
	_loaders.push_back(LoaderList::value_type(new ResLoaderTlk()));
}

} // End of namespace Kyra
