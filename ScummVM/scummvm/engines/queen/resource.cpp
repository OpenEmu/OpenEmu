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

#include "common/debug.h"
#include "common/endian.h"
#include "common/config-manager.h"
#include "common/substream.h"
#include "common/textconsole.h"
#include "queen/resource.h"

namespace Queen {


const char *const Resource::_tableFilename = "queen.tbl";

const RetailGameVersion Resource::_gameVersions[] = {
	{ "PEM10", 1, 0x00000008,  22677657 },
	{ "CEM10", 1, 0x0000584E, 190787021 },
	{ "PFM10", 1, 0x0002CD93,  22157304 },
	{ "CFM10", 1, 0x00032585, 186689095 },
	{ "PGM10", 1, 0x00059ACA,  22240013 },
	{ "CGM10", 1, 0x0005F2A7, 217648975 },
	{ "PIM10", 1, 0x000866B1,  22461366 },
	{ "CIM10", 1, 0x0008BEE2, 190795582 },
	{ "CSM10", 1, 0x000B343C, 190730602 },
	{ "CHM10", 1, 0x000DA981, 190705558 },
	{ "PE100", 1, 0x00101EC6,   3724538 },
	{ "PE100", 1, 0x00102B7F,   3732177 },
	{ "PEint", 1, 0x00103838,   1915913 },
	{ "aEM10", 2, 0x00103F1E,    351775 },
	{ "CE101", 2, 0x00107D8D,    563335 },
	{ "PE100", 2, 0x001086D4,    597032 }
};

static int compareResourceEntry(const void *a, const void *b) {
	const char *filename = (const char *)a;
	const ResourceEntry *entry = (const ResourceEntry *)b;
	return strcmp(filename, entry->filename);
}

Resource::Resource()
	: _resourceEntries(0), _resourceTable(NULL) {
	memset(&_version, 0, sizeof(_version));

	_currentResourceFileNum = 1;
	if (!_resourceFile.open("queen.1c")) {
		if (!_resourceFile.open("queen.1")) {
			error("Could not open resource file 'queen.1[c]'");
		}
	}
	if (!detectVersion(&_version, &_resourceFile)) {
		error("Unable to detect game version");
	}

	if (_version.features & GF_REBUILT) {
		readTableEntries(&_resourceFile);
	} else {
		readTableFile(_version.queenTblVersion, _version.queenTblOffset);
	}

	checkJASVersion();
	debug(5, "Detected game version: %s, which has %d resource entries", _version.str, _resourceEntries);
}

Resource::~Resource() {
	_resourceFile.close();

	if (_resourceTable != _resourceTablePEM10)
		delete[] _resourceTable;
}

ResourceEntry *Resource::resourceEntry(const char *filename) const {
	assert(filename[0] && strlen(filename) < 14);

	Common::String entryName(filename);
	entryName.toUppercase();

	ResourceEntry *re = NULL;
	re = (ResourceEntry *)bsearch(entryName.c_str(), _resourceTable, _resourceEntries, sizeof(ResourceEntry), compareResourceEntry);
	return re;
}

uint8 *Resource::loadFile(const char *filename, uint32 skipBytes, uint32 *size) {
	debug(7, "Resource::loadFile('%s')", filename);
	ResourceEntry *re = resourceEntry(filename);
	assert(re != NULL);
	uint32 sz = re->size - skipBytes;
	if (size != NULL) {
		*size = sz;
	}
	byte *dstBuf = new byte[sz];
	seekResourceFile(re->bundle, re->offset + skipBytes);
	_resourceFile.read(dstBuf, sz);
	return dstBuf;
}

void Resource::loadTextFile(const char *filename, Common::StringArray &stringList) {
	debug(7, "Resource::loadTextFile('%s')", filename);
	ResourceEntry *re = resourceEntry(filename);
	assert(re != NULL);
	seekResourceFile(re->bundle, re->offset);
	Common::SeekableSubReadStream stream(&_resourceFile, re->offset, re->offset + re->size);
	while (true) {
		Common::String tmp = stream.readLine();
		if (stream.eos() || stream.err())
			break;
		stringList.push_back(tmp);
	}
}

bool Resource::detectVersion(DetectedGameVersion *ver, Common::File *f) {
	memset(ver, 0, sizeof(DetectedGameVersion));

	if (f->readUint32BE() == MKTAG('Q','T','B','L')) {
		f->read(ver->str, 6);
		f->skip(2);
		ver->compression = f->readByte();
		ver->features = GF_REBUILT;
		ver->queenTblVersion = 0;
		ver->queenTblOffset = 0;
	} else {
		const RetailGameVersion *gameVersion = detectGameVersionFromSize(f->size());
		if (gameVersion == NULL) {
			warning("Unknown/unsupported FOTAQ version");
			return false;
		}
		strcpy(ver->str, gameVersion->str);
		ver->compression = COMPRESSION_NONE;
		ver->features = 0;
		ver->queenTblVersion = gameVersion->queenTblVersion;
		ver->queenTblOffset = gameVersion->queenTblOffset;
		strcpy(ver->str, gameVersion->str);

		// Handle game versions for which versionStr information is irrevelant
		if (gameVersion == &_gameVersions[VER_AMI_DEMO]) { // CE101
			ver->language = Common::EN_ANY;
			ver->features |= GF_FLOPPY | GF_DEMO;
			ver->platform = Common::kPlatformAmiga;
			return true;
		}
		if (gameVersion == &_gameVersions[VER_AMI_INTERVIEW]) { // PE100
			ver->language = Common::EN_ANY;
			ver->features |= GF_FLOPPY | GF_INTERVIEW;
			ver->platform = Common::kPlatformAmiga;
			return true;
		}
	}

	switch (ver->str[1]) {
	case 'E':
		if (Common::parseLanguage(ConfMan.get("language")) == Common::RU_RUS) {
			ver->language = Common::RU_RUS;
		} else if (Common::parseLanguage(ConfMan.get("language")) == Common::GR_GRE) {
			ver->language = Common::GR_GRE;
		} else {
			ver->language = Common::EN_ANY;
		}
		break;
	case 'F':
		ver->language = Common::FR_FRA;
		break;
	case 'G':
		ver->language = Common::DE_DEU;
		break;
	case 'H':
		ver->language = Common::HE_ISR;
		break;
	case 'I':
		ver->language = Common::IT_ITA;
		break;
	case 'S':
		ver->language = Common::ES_ESP;
		break;
	case 'g':
		ver->language = Common::GR_GRE;
		break;
	case 'R':
		ver->language = Common::RU_RUS;
		break;
	default:
		error("Invalid language id '%c'", ver->str[1]);
		break;
	}

	switch (ver->str[0]) {
	case 'P':
		ver->features |= GF_FLOPPY;
		ver->platform = Common::kPlatformPC;
		break;
	case 'C':
		ver->features |= GF_TALKIE;
		ver->platform = Common::kPlatformPC;
		break;
	case 'a':
		ver->features |= GF_FLOPPY;
		ver->platform = Common::kPlatformAmiga;
		break;
	default:
		error("Invalid platform id '%c'", ver->str[0]);
		break;
	}

	if (strcmp(ver->str + 2, "100") == 0 || strcmp(ver->str + 2, "101") == 0) {
		ver->features |= GF_DEMO;
	} else if (strcmp(ver->str + 2, "int") == 0) {
		ver->features |= GF_INTERVIEW;
	}
	return true;
}

void Resource::checkJASVersion() {
	if (_version.platform == Common::kPlatformAmiga) {
		// don't bother verifying the JAS version string with these versions,
		// it will be done at the end of Logic::readQueenJas, anyway
		return;
	}
	ResourceEntry *re = resourceEntry("QUEEN.JAS");
	assert(re != NULL);
	uint32 offset = re->offset;
	if (isDemo())
		offset += JAS_VERSION_OFFSET_DEMO;
	else if (isInterview())
		offset += JAS_VERSION_OFFSET_INTV;
	else
		offset += JAS_VERSION_OFFSET_PC;
	seekResourceFile(re->bundle, offset);

	char versionStr[6];
	_resourceFile.read(versionStr, 6);
	if (strcmp(_version.str, versionStr))
		error("Verifying game version failed! (expected: '%s', found: '%s')", _version.str, versionStr);
}

void Resource::seekResourceFile(int num, uint32 offset) {
	if (_currentResourceFileNum != num) {
		debug(7, "Opening resource file %d, current %d", num, _currentResourceFileNum);
		_resourceFile.close();
		char name[20];
		sprintf(name, "queen.%d", num);
		if (!_resourceFile.open(name)) {
			error("Could not open resource file '%s'", name);
		}
		_currentResourceFileNum = num;
	}
	_resourceFile.seek(offset);
}

void Resource::readTableFile(uint8 version, uint32 offset) {
	Common::File tableFile;
	tableFile.open(_tableFilename);
	if (tableFile.isOpen() && tableFile.readUint32BE() == MKTAG('Q','T','B','L')) {
		uint32 tableVersion = tableFile.readUint32BE();
		if (version > tableVersion) {
			error("The game you are trying to play requires version %d of queen.tbl, "
			      "you have version %d ; please update it", version, tableVersion);
		}
		tableFile.seek(offset);
		readTableEntries(&tableFile);
	} else {
		// check if it is the english floppy version, for which we have a hardcoded version of the table
		if (strcmp(_version.str, _gameVersions[VER_ENG_FLOPPY].str) == 0) {
			_resourceEntries = 1076;
			_resourceTable = _resourceTablePEM10;
		} else {
			error("Could not find tablefile '%s'", _tableFilename);
		}
	}
}

void Resource::readTableEntries(Common::File *file) {
	_resourceEntries = file->readUint16BE();
	_resourceTable = new ResourceEntry[_resourceEntries];
	for (uint16 i = 0; i < _resourceEntries; ++i) {
		ResourceEntry *re = &_resourceTable[i];
		file->read(re->filename, 12);
		re->filename[12] = '\0';
		re->bundle = file->readByte();
		re->offset = file->readUint32BE();
		re->size = file->readUint32BE();
	}
}

const RetailGameVersion *Resource::detectGameVersionFromSize(uint32 size) {
	for (int i = 0; i < VER_COUNT; ++i) {
		if (_gameVersions[i].dataFileSize == size) {
			return &_gameVersions[i];
		}
	}
	return NULL;
}

Common::File *Resource::findSound(const char *filename, uint32 *size) {
	assert(strstr(filename, ".SB") != NULL || strstr(filename, ".AMR") != NULL || strstr(filename, ".INS") != NULL);
	ResourceEntry *re = resourceEntry(filename);
	if (re) {
		*size = re->size;
		seekResourceFile(re->bundle, re->offset);
		return &_resourceFile;
	}
	return NULL;
}

} // End of namespace Queen
