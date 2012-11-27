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
#include "common/macresman.h"
#include "common/substream.h"
#include "common/textconsole.h"

#include "groovie/resource.h"
#include "groovie/groovie.h"

namespace Groovie {

// ResMan

Common::SeekableReadStream *ResMan::open(uint32 fileRef) {
	// Get the information about the resource
	ResInfo resInfo;
	if (!getResInfo(fileRef, resInfo)) {
		return NULL;
	}

	// Do we know the name of the required GJD?
	if (resInfo.gjd >= _gjds.size()) {
		error("Groovie::Resource: Unknown GJD %d", resInfo.gjd);
		return NULL;
	}

	debugC(1, kGroovieDebugResource | kGroovieDebugAll, "Groovie::Resource: Opening resource 0x%04X (%s, %d, %d)", fileRef, _gjds[resInfo.gjd].c_str(), resInfo.offset, resInfo.size);

	// Does it exist?
	if (!Common::File::exists(_gjds[resInfo.gjd])) {
		error("Groovie::Resource: %s not found", _gjds[resInfo.gjd].c_str());
		return NULL;
	}

	// Open the pack file
	Common::File *gjdFile = new Common::File();
	if (!gjdFile->open(_gjds[resInfo.gjd].c_str())) {
		delete gjdFile;
		error("Groovie::Resource: Couldn't open %s", _gjds[resInfo.gjd].c_str());
		return NULL;
	}

	// Save the used gjd file (except xmi and gamwav)
	if (resInfo.gjd < 19) {
		_lastGjd = resInfo.gjd;
	}

	// Returning the resource substream
	return new Common::SeekableSubReadStream(gjdFile, resInfo.offset, resInfo.offset + resInfo.size, DisposeAfterUse::YES);
}


// ResMan_t7g

static const char t7g_gjds[][0x15] = {"at", "b", "ch", "d", "dr", "fh", "ga", "hdisk", "htbd", "intro", "jhek", "k", "la", "li", "mb", "mc", "mu", "n", "p", "xmi", "gamwav"};

ResMan_t7g::ResMan_t7g(Common::MacResManager *macResFork) : _macResFork(macResFork) {
	for (int i = 0; i < 0x15; i++) {
		// Prepare the filename
		Common::String filename = t7g_gjds[i];
		filename += ".gjd";

		// Handle the special case of Mac's hdisk.gjd
		if (_macResFork && i == 7)
			filename = "T7GData";

		// Append it to the list of GJD files
		_gjds.push_back(filename);
	}
}

uint32 ResMan_t7g::getRef(Common::String name, Common::String scriptname) {
	// Get the name of the RL file
	Common::String rlFileName(t7g_gjds[_lastGjd]);
	rlFileName += ".rl";

	Common::SeekableReadStream *rlFile = 0;

	if (_macResFork) {
		// Open the RL file from the resource fork
		rlFile = _macResFork->getResource(rlFileName);
	} else {
		// Open the RL file
		rlFile = SearchMan.createReadStreamForMember(rlFileName);
	}

	if (!rlFile)
		error("Groovie::Resource: Couldn't open %s", rlFileName.c_str());

	uint32 resNum;
	bool found = false;
	for (resNum = 0; !found && !rlFile->err() && !rlFile->eos(); resNum++) {
		// Read the resource name
		char readname[12];
		rlFile->read(readname, 12);

		// Test whether it's the resource we're searching
		Common::String resname(readname, 12);
		if (resname.hasPrefix(name.c_str())) {
			debugC(2, kGroovieDebugResource | kGroovieDebugAll, "Groovie::Resource: Resource %12s matches %s", readname, name.c_str());
			found = true;
		}

		// Skip the rest of resource information
		rlFile->read(readname, 8);
	}

	// Close the RL file
	delete rlFile;

	// Verify we really found the resource
	if (!found) {
		error("Groovie::Resource: Couldn't find resource %s in %s", name.c_str(), rlFileName.c_str());
		return (uint32)-1;
	}

	return (_lastGjd << 10) | (resNum - 1);
}

bool ResMan_t7g::getResInfo(uint32 fileRef, ResInfo &resInfo) {
	// Calculate the GJD and the resource number
	resInfo.gjd = fileRef >> 10;
	uint16 resNum = fileRef & 0x3FF;

	// Get the name of the RL file
	Common::String rlFileName(t7g_gjds[resInfo.gjd]);
	rlFileName += ".rl";

	Common::SeekableReadStream *rlFile = 0;

	if (_macResFork) {
		// Open the RL file from the resource fork
		rlFile = _macResFork->getResource(rlFileName);
	} else {
		// Open the RL file
		rlFile = SearchMan.createReadStreamForMember(rlFileName);
	}

	if (!rlFile)
		error("Groovie::Resource: Couldn't open %s", rlFileName.c_str());

	// Seek to the position of the desired resource
	rlFile->seek(resNum * 20);
	if (rlFile->eos()) {
		delete rlFile;
		error("Groovie::Resource: Invalid resource number: 0x%04X (%s)", resNum, rlFileName.c_str());
	}

	// Read the resource name
	char resname[13];
	rlFile->read(resname, 12);
	resname[12] = 0;
	debugC(2, kGroovieDebugResource | kGroovieDebugAll, "Groovie::Resource: Resource name: %12s", resname);
	resInfo.filename = resname;

	// Read the resource information
	resInfo.offset = rlFile->readUint32LE();
	resInfo.size = rlFile->readUint32LE();

	// Close the resource RL file
	delete rlFile;

	return true;
}


// ResMan_v2

ResMan_v2::ResMan_v2() {
	Common::File indexfile;

	// Open the GJD index file
	if (!indexfile.open("gjd.gjd")) {
		error("Groovie::Resource: Couldn't open gjd.gjd");
		return;
	}

	Common::String line = indexfile.readLine();
	while (!indexfile.eos() && !line.empty()) {
		// Get the name before the space
		Common::String filename;
		for (const char *cur = line.c_str(); *cur != ' '; cur++) {
			filename += *cur;
		}

		// Append it to the list of GJD files
		if (!filename.empty()) {
			_gjds.push_back(filename);
		}

		// Read the next line
		line = indexfile.readLine();
	}

	// Close the GJD index file
	indexfile.close();
}

uint32 ResMan_v2::getRef(Common::String name, Common::String scriptname) {
	// Open the RL file
	Common::File rlFile;
	if (!rlFile.open("dir.rl")) {
		error("Groovie::Resource: Couldn't open dir.rl");
		return false;
	}

	uint32 resNum;
	bool found = false;
	for (resNum = 0; !found && !rlFile.err() && !rlFile.eos(); resNum++) {
		// Seek past metadata
		rlFile.seek(14, SEEK_CUR);

		// Read the resource name
		char readname[18];
		rlFile.read(readname, 18);

		// Test whether it's the resource we're searching
		Common::String resname(readname, 18);
		if (resname.hasPrefix(name.c_str())) {
			debugC(2, kGroovieDebugResource | kGroovieDebugAll, "Groovie::Resource: Resource %18s matches %s", readname, name.c_str());
			found = true;
			break;
		}
	}

	// Close the RL file
	rlFile.close();

	// Verify we really found the resource
	if (!found) {
		error("Groovie::Resource: Couldn't find resource %s", name.c_str());
		return (uint32)-1;
	}

	return resNum;
}

bool ResMan_v2::getResInfo(uint32 fileRef, ResInfo &resInfo) {
	// Open the RL file
	Common::File rlFile;
	if (!rlFile.open("dir.rl")) {
		error("Groovie::Resource: Couldn't open dir.rl");
		return false;
	}

	// Seek to the position of the desired resource
	rlFile.seek(fileRef * 32);
	if (rlFile.eos()) {
		rlFile.close();
		error("Groovie::Resource: Invalid resource number: 0x%04X", fileRef);
		return false;
	}

	// Read the resource information
	rlFile.readUint32LE(); // Unknown
	resInfo.offset = rlFile.readUint32LE();
	resInfo.size = rlFile.readUint32LE();
	resInfo.gjd = rlFile.readUint16LE();

	// Read the resource name
	char resname[19];
	resname[18] = 0;
	rlFile.read(resname, 18);
	debugC(2, kGroovieDebugResource | kGroovieDebugAll, "Groovie::Resource: Resource name: %18s", resname);
	resInfo.filename = resname;

	// 6 padding bytes? (it looks like they're always 0)

	// Close the resource RL file
	rlFile.close();

	return true;
}

} // End of Groovie namespace
