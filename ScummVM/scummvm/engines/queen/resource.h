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

#ifndef QUEEN_RESOURCE_H
#define QUEEN_RESOURCE_H

#include "common/file.h"
#include "common/str-array.h"
#include "common/language.h"
#include "common/platform.h"
#include "queen/defs.h"

namespace Queen {

enum GameFeatures {
	GF_DEMO      = 1 << 0, // demo
	GF_TALKIE    = 1 << 1, // equivalent to cdrom version check
	GF_FLOPPY    = 1 << 2, // floppy, ie. non-talkie version
	GF_INTERVIEW = 1 << 3, // interview demo
	GF_REBUILT   = 1 << 4  // version rebuilt with the 'compression_queen' tool
};

struct RetailGameVersion {
	char str[6];
	uint8 queenTblVersion;
	uint32 queenTblOffset;
	uint32 dataFileSize;
};

struct DetectedGameVersion {
	Common::Platform platform;
	Common::Language language;
	uint8 features;
	uint8 compression;
	char str[6];
	uint8 queenTblVersion;
	uint32 queenTblOffset;
};

struct ResourceEntry {
	char filename[13];
	uint8 bundle;
	uint32 offset;
	uint32 size;
};

class Resource {
public:

	Resource();
	~Resource();

	//! loads a binary file
	uint8 *loadFile(const char *filename, uint32 skipBytes = 0, uint32 *size = NULL);

	//! loads a text file
	void loadTextFile(const char *filename, Common::StringArray &stringList);

	//! returns true if the file is present in the resource
	bool fileExists(const char *filename) const { return resourceEntry(filename) != NULL; }

	//! returns a reference to a sound file
	Common::File *findSound(const char *filename, uint32 *size);

	bool isDemo() const { return (_version.features & GF_DEMO) != 0; }
	bool isInterview() const { return (_version.features & GF_INTERVIEW) != 0; }
	bool isFloppy() const { return (_version.features & GF_FLOPPY) != 0; }
	bool isCD() const { return (_version.features & GF_TALKIE) != 0; }

	//! returns compression type for audio files
	uint8 getCompression() const { return _version.compression; }

	//! returns JAS version string (contains language, platform and version information)
	const char *getJASVersion() const { return _version.str; }

	//! returns the language of the game
	Common::Language getLanguage() const { return _version.language; }

	Common::Platform getPlatform() const { return _version.platform; }

	//! detect game version
	static bool detectVersion(DetectedGameVersion *ver, Common::File *f);

	enum Version {
		VER_ENG_FLOPPY     = 0,
		VER_ENG_TALKIE     = 1,
		VER_FRE_FLOPPY     = 2,
		VER_FRE_TALKIE     = 3,
		VER_GER_FLOPPY     = 4,
		VER_GER_TALKIE     = 5,
		VER_ITA_FLOPPY     = 6,
		VER_ITA_TALKIE     = 7,
		VER_SPA_TALKIE     = 8,
		VER_HEB_TALKIE	   = 9,
		VER_DEMO_PCGAMES   = 10,
		VER_DEMO           = 11,
		VER_INTERVIEW      = 12,
		VER_AMI_ENG_FLOPPY = 13,
		VER_AMI_DEMO       = 14,
		VER_AMI_INTERVIEW  = 15,

		VER_COUNT          = 16
	};

	enum {
		JAS_VERSION_OFFSET_DEMO = 0x119A8,
		JAS_VERSION_OFFSET_INTV	= 0xCF8,
		JAS_VERSION_OFFSET_PC	= 0x12484
	};

protected:

	Common::File _resourceFile;

	int _currentResourceFileNum;

	DetectedGameVersion _version;

	//! number of entries in resource table
	uint32 _resourceEntries;

	ResourceEntry *_resourceTable;

	//! verify the version of the selected game
	void checkJASVersion();

	//! returns a reference to the ReseourceEntry for the specified filename
	ResourceEntry *resourceEntry(const char *filename) const;

	//! seeks resource file to specific bundle and file offset
	void seekResourceFile(int num, uint32 offset);

	//! extract the resource table for the specified game version
	void readTableFile(uint8 version, uint32 offset);

	//! read the resource table from the specified file
	void readTableEntries(Common::File *file);

	//! detect game version based on queen.1 datafile size
	static const RetailGameVersion *detectGameVersionFromSize(uint32 size);

	//! resource table filename (queen.tbl)
	static const char *const _tableFilename;

	//! known FOTAQ versions
	static const RetailGameVersion _gameVersions[];

	//! resource table for english floppy version
	static ResourceEntry _resourceTablePEM10[];
};

} // End of namespace Queen

#endif
