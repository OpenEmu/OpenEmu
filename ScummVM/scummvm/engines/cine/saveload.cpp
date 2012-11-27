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
#include "common/savefile.h"
#include "common/textconsole.h"

#include "cine/cine.h"
#include "cine/bg_list.h"
#include "cine/saveload.h"
#include "cine/sound.h"
#include "cine/various.h"

namespace Cine {

int16 currentDisk;
int16 saveVar2;


bool writeChunkHeader(Common::OutSaveFile &out, const ChunkHeader &header) {
	out.writeUint32BE(header.id);
	out.writeUint32BE(header.version);
	out.writeUint32BE(header.size);
	return !out.err();
}

bool loadChunkHeader(Common::SeekableReadStream &in, ChunkHeader &header) {
	header.id      = in.readUint32BE();
	header.version = in.readUint32BE();
	header.size    = in.readUint32BE();
	return !(in.eos() || in.err());
}

/**
 * Savegame format detector
 * @param fHandle Savefile to check
 * @return Savegame format on success, ANIMSIZE_UNKNOWN on failure
 *
 * This function seeks through the savefile and tries to determine the
 * savegame format it uses. There's a miniscule chance that the detection
 * algorithm could get confused and think that the file uses both the older
 * and the newer format but that is such a remote possibility that I wouldn't
 * worry about it at all.
 *
 * Also detects the temporary Operation Stealth savegame format now.
 */
enum CineSaveGameFormat detectSaveGameFormat(Common::SeekableReadStream &fHandle) {
	const uint32 prevStreamPos = fHandle.pos();

	// First check for the temporary Operation Stealth savegame format.
	fHandle.seek(0);
	ChunkHeader hdr;
	loadChunkHeader(fHandle, hdr);
	fHandle.seek(prevStreamPos);
	if (hdr.id == TEMP_OS_FORMAT_ID) {
		return TEMP_OS_FORMAT;
	}

	// Ok, so the savegame isn't using the temporary Operation Stealth savegame format.
	// Let's check for the plain Future Wars savegame format and its different versions then.
	// The animDataTable begins at savefile position 0x2315.
	// Each animDataTable entry takes 23 bytes in older saves (Revisions 21772-31443)
	// and 30 bytes in the save format after that (Revision 31444 and onwards).
	// There are 255 entries in the animDataTable in both of the savefile formats.
	static const uint animDataTableStart = 0x2315;
	static const uint animEntriesCount = 255;
	static const uint oldAnimEntrySize = 23;
	static const uint newAnimEntrySize = 30;
	static const uint animEntrySizeChoices[] = {oldAnimEntrySize, newAnimEntrySize};
	Common::Array<uint> animEntrySizeMatches;

	// Try to walk through the savefile using different animDataTable entry sizes
	// and make a list of all the successful entry sizes.
	for (uint i = 0; i < ARRAYSIZE(animEntrySizeChoices); i++) {
		// 206 = 2 * 50 * 2 + 2 * 3 (Size of global and object script entries)
		// 20 = 4 * 2 + 2 * 6 (Size of overlay and background incrust entries)
		static const uint sizeofScreenParams = 2 * 6;
		static const uint globalScriptEntrySize = 206;
		static const uint objectScriptEntrySize = 206;
		static const uint overlayEntrySize = 20;
		static const uint bgIncrustEntrySize = 20;
		static const uint chainEntrySizes[] = {
			globalScriptEntrySize,
			objectScriptEntrySize,
			overlayEntrySize,
			bgIncrustEntrySize
		};

		uint animEntrySize = animEntrySizeChoices[i];
		// Jump over the animDataTable entries and the screen parameters
		int32 newPos = animDataTableStart + animEntrySize * animEntriesCount + sizeofScreenParams;
		// Check that there's data left after the point we're going to jump to
		if (newPos >= fHandle.size()) {
			continue;
		}
		fHandle.seek(newPos);

		// Jump over the remaining items in the savegame file
		// (i.e. the global scripts, object scripts, overlays and background incrusts).
		bool chainWalkSuccess = true;
		for (uint chainIndex = 0; chainIndex < ARRAYSIZE(chainEntrySizes); chainIndex++) {
			// Read entry count and jump over the entries
			int entryCount = fHandle.readSint16BE();
			newPos = fHandle.pos() + chainEntrySizes[chainIndex] * entryCount;
			// Check that we didn't go past the end of file.
			// Note that getting exactly to the end of file is acceptable.
			if (newPos > fHandle.size()) {
				chainWalkSuccess = false;
				break;
			}
			fHandle.seek(newPos);
		}

		// If we could walk the chain successfully and
		// got exactly to the end of file then we've got a match.
		if (chainWalkSuccess && fHandle.pos() == fHandle.size()) {
			// We found a match, let's save it
			animEntrySizeMatches.push_back(animEntrySize);
		}
	}

	// Check that we got only one entry size match.
	// If we didn't, then return an error.
	enum CineSaveGameFormat result = ANIMSIZE_UNKNOWN;
	if (animEntrySizeMatches.size() == 1) {
		const uint animEntrySize = animEntrySizeMatches[0];
		assert(animEntrySize == oldAnimEntrySize || animEntrySize == newAnimEntrySize);
		if (animEntrySize == oldAnimEntrySize) {
			result = ANIMSIZE_23;
		} else { // animEntrySize == newAnimEntrySize
			// Check data and mask pointers in all of the animDataTable entries
			// to see whether we've got the version with the broken data and mask pointers or not.
			// In the broken format all data and mask pointers were always zero.
			static const uint relativeDataPos = 2 * 4;
			bool pointersIntact = false;
			for (uint i = 0; i < animEntriesCount; i++) {
				fHandle.seek(animDataTableStart + i * animEntrySize + relativeDataPos);
				uint32 data = fHandle.readUint32BE();
				uint32 mask = fHandle.readUint32BE();
				if ((data != 0) || (mask != 0)) {
					pointersIntact = true;
					break;
				}
			}
			result = (pointersIntact ? ANIMSIZE_30_PTRS_INTACT : ANIMSIZE_30_PTRS_BROKEN);
		}
	} else if (animEntrySizeMatches.size() > 1) {
		warning("Savegame format detector got confused by input data. Detecting savegame to be using an unknown format");
	} else { // animEtrySizeMatches.size() == 0
		debug(3, "Savegame format detector was unable to detect savegame's format");
	}

	fHandle.seek(prevStreamPos);
	return result;
}

/**
 * Restore script list item from savefile
 * @param fHandle Savefile handle open for reading
 * @param isGlobal Restore object or global script?
 */
void loadScriptFromSave(Common::SeekableReadStream &fHandle, bool isGlobal) {
	ScriptVars localVars, labels;
	uint16 compare, pos;
	int16 idx;

	labels.load(fHandle);
	localVars.load(fHandle);

	compare = fHandle.readUint16BE();
	pos = fHandle.readUint16BE();
	idx = fHandle.readUint16BE();

	// no way to reinitialize these
	if (idx < 0) {
		return;
	}

	// original code loaded everything into globalScripts, this should be
	// the correct behavior
	if (isGlobal) {
		ScriptPtr tmp(scriptInfo->create(*g_cine->_scriptTable[idx], idx, labels, localVars, compare, pos));
		assert(tmp);
		g_cine->_globalScripts.push_back(tmp);
	} else {
		ScriptPtr tmp(scriptInfo->create(*g_cine->_relTable[idx], idx, labels, localVars, compare, pos));
		assert(tmp);
		g_cine->_objectScripts.push_back(tmp);
	}
}

/**
 * Restore overlay sprites from savefile
 * @param fHandle Savefile open for reading
 */
void loadOverlayFromSave(Common::SeekableReadStream &fHandle) {
	overlay tmp;

	fHandle.readUint32BE();
	fHandle.readUint32BE();

	tmp.objIdx = fHandle.readUint16BE();
	tmp.type = fHandle.readUint16BE();
	tmp.x = fHandle.readSint16BE();
	tmp.y = fHandle.readSint16BE();
	tmp.width = fHandle.readSint16BE();
	tmp.color = fHandle.readSint16BE();

	g_cine->_overlayList.push_back(tmp);
}

bool loadObjectTable(Common::SeekableReadStream &in) {
	in.readUint16BE(); // Entry count
	in.readUint16BE(); // Entry size

	for (int i = 0; i < NUM_MAX_OBJECT; i++) {
		g_cine->_objectTable[i].x = in.readSint16BE();
		g_cine->_objectTable[i].y = in.readSint16BE();
		g_cine->_objectTable[i].mask = in.readUint16BE();
		g_cine->_objectTable[i].frame = in.readSint16BE();
		g_cine->_objectTable[i].costume = in.readSint16BE();
		in.read(g_cine->_objectTable[i].name, 20);
		g_cine->_objectTable[i].part = in.readUint16BE();
	}
	return !(in.eos() || in.err());
}

bool loadZoneData(Common::SeekableReadStream &in) {
	for (int i = 0; i < 16; i++) {
		g_cine->_zoneData[i] = in.readUint16BE();
	}
	return !(in.eos() || in.err());
}

bool loadCommandVariables(Common::SeekableReadStream &in) {
	for (int i = 0; i < 4; i++) {
		commandVar3[i] = in.readUint16BE();
	}
	return !(in.eos() || in.err());
}

bool loadScreenParams(Common::SeekableReadStream &in) {
	// TODO: handle screen params (really required ?)
	in.readUint16BE();
	in.readUint16BE();
	in.readUint16BE();
	in.readUint16BE();
	in.readUint16BE();
	in.readUint16BE();
	return !(in.eos() || in.err());
}

bool loadGlobalScripts(Common::SeekableReadStream &in) {
	int size = in.readSint16BE();
	for (int i = 0; i < size; i++) {
		loadScriptFromSave(in, true);
	}
	return !(in.eos() || in.err());
}

bool loadObjectScripts(Common::SeekableReadStream &in) {
	int size = in.readSint16BE();
	for (int i = 0; i < size; i++) {
		loadScriptFromSave(in, false);
	}
	return !(in.eos() || in.err());
}

bool loadOverlayList(Common::SeekableReadStream &in) {
	int size = in.readSint16BE();
	for (int i = 0; i < size; i++) {
		loadOverlayFromSave(in);
	}
	return !(in.eos() || in.err());
}

bool loadSeqList(Common::SeekableReadStream &in) {
	uint size = in.readUint16BE();
	SeqListElement tmp;
	for (uint i = 0; i < size; i++) {
		tmp.var4   = in.readSint16BE();
		tmp.objIdx = in.readUint16BE();
		tmp.var8   = in.readSint16BE();
		tmp.frame  = in.readSint16BE();
		tmp.varC   = in.readSint16BE();
		tmp.varE   = in.readSint16BE();
		tmp.var10  = in.readSint16BE();
		tmp.var12  = in.readSint16BE();
		tmp.var14  = in.readSint16BE();
		tmp.var16  = in.readSint16BE();
		tmp.var18  = in.readSint16BE();
		tmp.var1A  = in.readSint16BE();
		tmp.var1C  = in.readSint16BE();
		tmp.var1E  = in.readSint16BE();
		g_cine->_seqList.push_back(tmp);
	}
	return !(in.eos() || in.err());
}

bool loadZoneQuery(Common::SeekableReadStream &in) {
	for (int i = 0; i < 16; i++) {
		g_cine->_zoneQuery[i] = in.readUint16BE();
	}
	return !(in.eos() || in.err());
}

void saveObjectTable(Common::OutSaveFile &out) {
	out.writeUint16BE(NUM_MAX_OBJECT); // Entry count
	out.writeUint16BE(0x20); // Entry size

	for (int i = 0; i < NUM_MAX_OBJECT; i++) {
		out.writeUint16BE(g_cine->_objectTable[i].x);
		out.writeUint16BE(g_cine->_objectTable[i].y);
		out.writeUint16BE(g_cine->_objectTable[i].mask);
		out.writeUint16BE(g_cine->_objectTable[i].frame);
		out.writeUint16BE(g_cine->_objectTable[i].costume);
		out.write(g_cine->_objectTable[i].name, 20);
		out.writeUint16BE(g_cine->_objectTable[i].part);
	}
}

void saveZoneData(Common::OutSaveFile &out) {
	for (int i = 0; i < 16; i++) {
		out.writeUint16BE(g_cine->_zoneData[i]);
	}
}

void saveCommandVariables(Common::OutSaveFile &out) {
	for (int i = 0; i < 4; i++) {
		out.writeUint16BE(commandVar3[i]);
	}
}

/** Save the 80 bytes long command buffer padded to that length with zeroes. */
void saveCommandBuffer(Common::OutSaveFile &out) {
	// Let's make sure there's space for the trailing zero
	// (That's why we subtract one from the maximum command buffer size here).
	uint32 size = MIN<uint32>(g_cine->_commandBuffer.size(), kMaxCommandBufferSize - 1);
	out.write(g_cine->_commandBuffer.c_str(), size);
	// Write the rest as zeroes (Here we also write the string's trailing zero)
	for (uint i = 0; i < kMaxCommandBufferSize - size; i++) {
		out.writeByte(0);
	}
}

void saveAnimDataTable(Common::OutSaveFile &out) {
	out.writeUint16BE(NUM_MAX_ANIMDATA); // Entry count
	out.writeUint16BE(0x1E); // Entry size

	for (int i = 0; i < NUM_MAX_ANIMDATA; i++) {
		g_cine->_animDataTable[i].save(out);
	}
}

void saveScreenParams(Common::OutSaveFile &out) {
	// Screen parameters, unhandled
	out.writeUint16BE(0);
	out.writeUint16BE(0);
	out.writeUint16BE(0);
	out.writeUint16BE(0);
	out.writeUint16BE(0);
	out.writeUint16BE(0);
}

void saveGlobalScripts(Common::OutSaveFile &out) {
	ScriptList::const_iterator it;
	out.writeUint16BE(g_cine->_globalScripts.size());
	for (it = g_cine->_globalScripts.begin(); it != g_cine->_globalScripts.end(); ++it) {
		(*it)->save(out);
	}
}

void saveObjectScripts(Common::OutSaveFile &out) {
	ScriptList::const_iterator it;
	out.writeUint16BE(g_cine->_objectScripts.size());
	for (it = g_cine->_objectScripts.begin(); it != g_cine->_objectScripts.end(); ++it) {
		(*it)->save(out);
	}
}

void saveOverlayList(Common::OutSaveFile &out) {
	Common::List<overlay>::const_iterator it;

	out.writeUint16BE(g_cine->_overlayList.size());

	for (it = g_cine->_overlayList.begin(); it != g_cine->_overlayList.end(); ++it) {
		out.writeUint32BE(0); // next
		out.writeUint32BE(0); // previous?
		out.writeUint16BE(it->objIdx);
		out.writeUint16BE(it->type);
		out.writeSint16BE(it->x);
		out.writeSint16BE(it->y);
		out.writeSint16BE(it->width);
		out.writeSint16BE(it->color);
	}
}

void saveBgIncrustList(Common::OutSaveFile &out) {
	Common::List<BGIncrust>::const_iterator it;
	out.writeUint16BE(g_cine->_bgIncrustList.size());

	for (it = g_cine->_bgIncrustList.begin(); it != g_cine->_bgIncrustList.end(); ++it) {
		out.writeUint32BE(0); // next
		out.writeUint32BE(0); // previous?
		out.writeUint16BE(it->objIdx);
		out.writeUint16BE(it->param);
		out.writeUint16BE(it->x);
		out.writeUint16BE(it->y);
		out.writeUint16BE(it->frame);
		out.writeUint16BE(it->part);
	}
}

void saveZoneQuery(Common::OutSaveFile &out) {
	for (int i = 0; i < 16; i++) {
		out.writeUint16BE(g_cine->_zoneQuery[i]);
	}
}

void saveSeqList(Common::OutSaveFile &out) {
	Common::List<SeqListElement>::const_iterator it;
	out.writeUint16BE(g_cine->_seqList.size());

	for (it = g_cine->_seqList.begin(); it != g_cine->_seqList.end(); ++it) {
		out.writeSint16BE(it->var4);
		out.writeUint16BE(it->objIdx);
		out.writeSint16BE(it->var8);
		out.writeSint16BE(it->frame);
		out.writeSint16BE(it->varC);
		out.writeSint16BE(it->varE);
		out.writeSint16BE(it->var10);
		out.writeSint16BE(it->var12);
		out.writeSint16BE(it->var14);
		out.writeSint16BE(it->var16);
		out.writeSint16BE(it->var18);
		out.writeSint16BE(it->var1A);
		out.writeSint16BE(it->var1C);
		out.writeSint16BE(it->var1E);
	}
}

bool CineEngine::loadSaveDirectory() {
	Common::InSaveFile *fHandle;
	fHandle = _saveFileMan->openForLoading(Common::String::format("%s.dir", _targetName.c_str()));

	if (!fHandle) {
		return false;
	}

	// Initialize all savegames' descriptions to empty strings
	// so that if the savegames' descriptions can only be partially read from file
	// then the missing ones are correctly set to empty strings.
	memset(currentSaveName, 0, sizeof(currentSaveName));

	fHandle->read(currentSaveName, 10 * 20);
	delete fHandle;

	// Make sure all savegames' descriptions end with a trailing zero.
	for (int i = 0; i < ARRAYSIZE(currentSaveName); i++)
		currentSaveName[i][sizeof(CommandeType) - 1] = 0;

	return true;
}

bool CineEngine::loadTempSaveOS(Common::SeekableReadStream &in) {
	char musicName[13];
	char bgNames[8][13];

	// First check the temporary Operation Stealth savegame format header.
	ChunkHeader hdr;
	loadChunkHeader(in, hdr);
	if (hdr.id != TEMP_OS_FORMAT_ID) {
		warning("loadTempSaveOS: File has incorrect identifier. Not loading savegame");
		return false;
	} else if (hdr.version > CURRENT_OS_SAVE_VER) {
		warning("loadTempSaveOS: Detected newer format version. Not loading savegame");
		return false;
	} else if ((int)hdr.version < (int)CURRENT_OS_SAVE_VER) {
		warning("loadTempSaveOS: Detected older format version. Trying to load nonetheless. Things may break");
	} else { // hdr.id == TEMP_OS_FORMAT_ID && hdr.version == CURRENT_OS_SAVE_VER
		debug(3, "loadTempSaveOS: Found correct header (Both the identifier and version number match).");
	}

	// There shouldn't be any data in the header's chunk currently so it's an error if there is.
	if (hdr.size > 0) {
		warning("loadTempSaveOS: Format header's chunk seems to contain data so format is incorrect. Not loading savegame");
		return false;
	}

	// Ok, so we've got a correct header for a temporary Operation Stealth savegame.
	// Let's start loading the plain savegame data then.
	currentDisk = in.readUint16BE();
	in.read(currentPartName, 13);
	in.read(currentPrcName, 13);
	in.read(currentRelName, 13);
	in.read(currentMsgName, 13);

	// Load the 8 background names.
	for (uint i = 0; i < 8; i++) {
		in.read(bgNames[i], 13);
	}

	in.read(currentCtName, 13);

	// Moved the loading of current procedure, relation,
	// backgrounds and Ct here because if they were at the
	// end of this function then the global scripts loading
	// made an array out of bounds access. In the original
	// game's disassembly these aren't here but at the end.
	// The difference is probably in how we handle loading
	// the global scripts and some other things (i.e. the
	// loading routines aren't exactly the same and subtle
	// semantic differences result in having to do things
	// in a different order).
	{
		// Not sure if this is needed with Operation Stealth...
		checkDataDisk(currentDisk);

		if (strlen(currentPrcName)) {
			loadPrc(currentPrcName);
		}

		if (strlen(currentRelName)) {
			loadRel(currentRelName);
		}

		// Load first background (Uses loadBg)
		if (strlen(bgNames[0])) {
			loadBg(bgNames[0]);
		}

		// Add backgrounds 1-7 (Uses addBackground)
		for (int i = 1; i < 8; i++) {
			if (strlen(bgNames[i])) {
				addBackground(bgNames[i], i);
			}
		}

		if (strlen(currentCtName)) {
			loadCtOS(currentCtName);
		}
	}

	loadObjectTable(in);
	renderer->restorePalette(in, hdr.version);
	g_cine->_globalVars.load(in, NUM_MAX_VAR);
	loadZoneData(in);
	loadCommandVariables(in);
	char tempCommandBuffer[kMaxCommandBufferSize];
	in.read(tempCommandBuffer, kMaxCommandBufferSize);
	g_cine->_commandBuffer = tempCommandBuffer;
	renderer->setCommand(g_cine->_commandBuffer);
	loadZoneQuery(in);

	// TODO: Use the loaded string (Current music name (String, 13 bytes)).
	in.read(musicName, 13);

	// TODO: Use the loaded value (Is music loaded? (Uint16BE, Boolean)).
	in.readUint16BE();

	// TODO: Use the loaded value (Is music playing? (Uint16BE, Boolean)).
	in.readUint16BE();

	renderer->_cmdY      = in.readUint16BE();
	in.readUint16BE(); // Some unknown variable that seems to always be zero
	allowPlayerInput     = in.readUint16BE();
	playerCommand        = in.readUint16BE();
	commandVar1          = in.readUint16BE();
	isDrawCommandEnabled = in.readUint16BE();
	var5                 = in.readUint16BE();
	var4                 = in.readUint16BE();
	var3                 = in.readUint16BE();
	var2                 = in.readUint16BE();
	commandVar2          = in.readUint16BE();
	renderer->_messageBg = in.readUint16BE();

	// TODO: Use the loaded value (adBgVar1 (Uint16BE)).
	in.readUint16BE();

	currentAdditionalBgIdx = in.readSint16BE();
	currentAdditionalBgIdx2 = in.readSint16BE();

	// TODO: Check whether the scroll value really gets used correctly after this.
	// Note that the backgrounds are loaded only later than this value is set.
	renderer->setScroll(in.readUint16BE());

	// TODO: Use the loaded value (adBgVar0 (Uint16BE). Maybe this means bgVar0?).
	in.readUint16BE();

	disableSystemMenu = in.readUint16BE();

	// TODO: adBgVar1 = 1 here

	// Load the animDataTable entries
	in.readUint16BE(); // Entry count (255 in the PC version of Operation Stealth).
	in.readUint16BE(); // Entry size (36 in the PC version of Operation Stealth).
	loadResourcesFromSave(in, ANIMSIZE_30_PTRS_INTACT);

	loadScreenParams(in);
	loadGlobalScripts(in);
	loadObjectScripts(in);
	loadSeqList(in);
	loadOverlayList(in);
	loadBgIncrustFromSave(in);

	// Left this here instead of moving it earlier in this function with
	// the other current value loadings (e.g. loading of current procedure,
	// current backgrounds etc). Mostly emulating the way we've handled
	// Future Wars savegames and hoping that things work out.
	if (strlen(currentMsgName)) {
		loadMsg(currentMsgName);
	}

	// TODO: Add current music loading and playing here
	// TODO: Palette handling?

	if (in.pos() == in.size()) {
		debug(3, "loadTempSaveOS: Loaded the whole savefile.");
	} else {
		warning("loadTempSaveOS: Loaded the savefile but didn't exhaust it completely. Something was left over");
	}

	return !(in.eos() || in.err());
}

bool CineEngine::loadPlainSaveFW(Common::SeekableReadStream &in, CineSaveGameFormat saveGameFormat) {
	char bgName[13];

	// At savefile position 0x0000:
	currentDisk = in.readUint16BE();

	// At 0x0002:
	in.read(currentPartName, 13);
	// At 0x000F:
	in.read(currentDatName, 13);

	// At 0x001C:
	musicIsPlaying = in.readSint16BE();

	// At 0x001E:
	in.read(currentPrcName, 13);
	// At 0x002B:
	in.read(currentRelName, 13);
	// At 0x0038:
	in.read(currentMsgName, 13);
	// At 0x0045:
	in.read(bgName, 13);
	// At 0x0052:
	in.read(currentCtName, 13);

	checkDataDisk(currentDisk);

	if (strlen(currentPartName)) {
		loadPart(currentPartName);
	}

	if (strlen(currentPrcName)) {
		loadPrc(currentPrcName);
	}

	if (strlen(currentRelName)) {
		loadRel(currentRelName);
	}

	if (strlen(bgName)) {
		loadBg(bgName);
	}

	if (strlen(currentCtName)) {
		loadCtFW(currentCtName);
	}

	// At 0x005F:
	loadObjectTable(in);

	// At 0x2043 (i.e. 0x005F + 2 * 2 + 255 * 32):
	renderer->restorePalette(in, 0);

	// At 0x2083 (i.e. 0x2043 + 16 * 2 * 2):
	g_cine->_globalVars.load(in, NUM_MAX_VAR);

	// At 0x2281 (i.e. 0x2083 + 255 * 2):
	loadZoneData(in);

	// At 0x22A1 (i.e. 0x2281 + 16 * 2):
	loadCommandVariables(in);

	// At 0x22A9 (i.e. 0x22A1 + 4 * 2):
	char tempCommandBuffer[kMaxCommandBufferSize];
	in.read(tempCommandBuffer, kMaxCommandBufferSize);
	g_cine->_commandBuffer = tempCommandBuffer;
	renderer->setCommand(g_cine->_commandBuffer);

	// At 0x22F9 (i.e. 0x22A9 + 0x50):
	renderer->_cmdY = in.readUint16BE();

	// At 0x22FB:
	bgVar0 = in.readUint16BE();
	// At 0x22FD:
	allowPlayerInput = in.readUint16BE();
	// At 0x22FF:
	playerCommand = in.readSint16BE();
	// At 0x2301:
	commandVar1 = in.readSint16BE();
	// At 0x2303:
	isDrawCommandEnabled = in.readUint16BE();
	// At 0x2305:
	var5 = in.readUint16BE();
	// At 0x2307:
	var4 = in.readUint16BE();
	// At 0x2309:
	var3 = in.readUint16BE();
	// At 0x230B:
	var2 = in.readUint16BE();
	// At 0x230D:
	commandVar2 = in.readSint16BE();

	// At 0x230F:
	renderer->_messageBg = in.readUint16BE();

	// At 0x2311:
	in.readUint16BE();
	// At 0x2313:
	in.readUint16BE();

	// At 0x2315:
	loadResourcesFromSave(in, saveGameFormat);

	loadScreenParams(in);
	loadGlobalScripts(in);
	loadObjectScripts(in);
	loadOverlayList(in);
	loadBgIncrustFromSave(in);

	if (strlen(currentMsgName)) {
		loadMsg(currentMsgName);
	}

	if (strlen(currentDatName)) {
		g_sound->loadMusic(currentDatName);
		if (musicIsPlaying) {
			g_sound->playMusic();
		}
	}

	return !(in.eos() || in.err());
}

bool CineEngine::makeLoad(const Common::String &saveName) {
	Common::SharedPtr<Common::InSaveFile> saveFile(_saveFileMan->openForLoading(saveName));

	if (!saveFile) {
		renderer->drawString(otherMessages[0], 0);
		waitPlayerInput();
		// restoreScreen();
		checkDataDisk(-1);
		return false;
	}

	setMouseCursor(MOUSE_CURSOR_DISK);

	uint32 saveSize = saveFile->size();
	// TODO: Evaluate the maximum savegame size for the temporary Operation Stealth savegame format.
	if (saveSize == 0) { // Savefile's compressed using zlib format can't tell their unpacked size, test for it
		// Can't get information about the savefile's size so let's try
		// reading as much as we can from the file up to a predefined upper limit.
		//
		// Some estimates for maximum savefile sizes (All with 255 animDataTable entries of 30 bytes each):
		// With 256 global scripts, object scripts, overlays and background incrusts:
		// 0x2315 + (255 * 30) + (2 * 6) + (206 + 206 + 20 + 20) * 256 = ~129kB
		// With 512 global scripts, object scripts, overlays and background incrusts:
		// 0x2315 + (255 * 30) + (2 * 6) + (206 + 206 + 20 + 20) * 512 = ~242kB
		//
		// I think it extremely unlikely that there would be over 512 global scripts, object scripts,
		// overlays and background incrusts so 256kB seems like quite a safe upper limit.
		// NOTE: If the savegame format is changed then this value might have to be re-evaluated!
		// Hopefully devices with more limited memory can also cope with this memory allocation.
		saveSize = 256 * 1024;
	}
	Common::SharedPtr<Common::SeekableReadStream> in(saveFile->readStream(saveSize));

	// Try to detect the used savegame format
	enum CineSaveGameFormat saveGameFormat = detectSaveGameFormat(*in);

	// Handle problematic savegame formats
	bool load = true; // Should we try to load the savegame?
	bool result = false;
	if (saveGameFormat == ANIMSIZE_30_PTRS_BROKEN) {
		// One might be able to load the ANIMSIZE_30_PTRS_BROKEN format but
		// that's not implemented here because it was never used in a stable
		// release of ScummVM but only during development (From revision 31453,
		// which introduced the problem, until revision 32073, which fixed it).
		// Therefore we bail out if we detect this particular savegame format.
		warning("Detected a known broken savegame format, not loading savegame");
		load = false; // Don't load the savegame
	} else if (saveGameFormat == ANIMSIZE_UNKNOWN) {
		// If we can't detect the savegame format
		// then let's try the default format and hope for the best.
		warning("Couldn't detect the used savegame format, trying default savegame format. Things may break");
		saveGameFormat = ANIMSIZE_30_PTRS_INTACT;
	}

	if (load) {
		// Reset the engine's state
		resetEngine();

		if (saveGameFormat == TEMP_OS_FORMAT) {
			// Load the temporary Operation Stealth savegame format
			result = loadTempSaveOS(*in);
		} else {
			// Load the plain Future Wars savegame format
			result = loadPlainSaveFW(*in, saveGameFormat);
		}
	}

	setMouseCursor(MOUSE_CURSOR_NORMAL);

	return result;
}

void CineEngine::makeSaveFW(Common::OutSaveFile &out) {
	out.writeUint16BE(currentDisk);
	out.write(currentPartName, 13);
	out.write(currentDatName, 13);
	out.writeUint16BE(musicIsPlaying);
	out.write(currentPrcName, 13);
	out.write(currentRelName, 13);
	out.write(currentMsgName, 13);
	renderer->saveBgNames(out);
	out.write(currentCtName, 13);

	saveObjectTable(out);
	renderer->savePalette(out);
	g_cine->_globalVars.save(out, NUM_MAX_VAR);
	saveZoneData(out);
	saveCommandVariables(out);
	saveCommandBuffer(out);

	out.writeUint16BE(renderer->_cmdY);
	out.writeUint16BE(bgVar0);
	out.writeUint16BE(allowPlayerInput);
	out.writeUint16BE(playerCommand);
	out.writeUint16BE(commandVar1);
	out.writeUint16BE(isDrawCommandEnabled);
	out.writeUint16BE(var5);
	out.writeUint16BE(var4);
	out.writeUint16BE(var3);
	out.writeUint16BE(var2);
	out.writeUint16BE(commandVar2);
	out.writeUint16BE(renderer->_messageBg);

	saveAnimDataTable(out);
	saveScreenParams(out);

	saveGlobalScripts(out);
	saveObjectScripts(out);
	saveOverlayList(out);
	saveBgIncrustList(out);
}

/**
 * Save an Operation Stealth type savegame. WIP!
 *
 * NOTE: This is going to be very much a work in progress so the Operation Stealth's
 *       savegame formats that are going to be tried are extremely probably not going
 *       to be supported at all after Operation Stealth becomes officially supported.
 *       This means that the savegame format will hopefully change to something nicer
 *       when official support for Operation Stealth begins.
 */
void CineEngine::makeSaveOS(Common::OutSaveFile &out) {
	int i;

	// Make a temporary Operation Stealth savegame format chunk header and save it.
	ChunkHeader header;
	header.id = TEMP_OS_FORMAT_ID;
	header.version = CURRENT_OS_SAVE_VER;
	header.size = 0; // No data is currently put inside the chunk, all the plain data comes right after it.
	writeChunkHeader(out, header);

	// Start outputting the plain savegame data right after the chunk header.
	out.writeUint16BE(currentDisk);
	out.write(currentPartName, 13);
	out.write(currentPrcName, 13);
	out.write(currentRelName, 13);
	out.write(currentMsgName, 13);
	renderer->saveBgNames(out);
	out.write(currentCtName, 13);

	saveObjectTable(out);
	renderer->savePalette(out);
	g_cine->_globalVars.save(out, NUM_MAX_VAR);
	saveZoneData(out);
	saveCommandVariables(out);
	saveCommandBuffer(out);
	saveZoneQuery(out);

	// FIXME: Save a proper name here, saving an empty string currently.
	// 0x2925: Current music name (String, 13 bytes).
	for (i = 0; i < 13; i++) {
		out.writeByte(0);
	}
	// FIXME: Save proper value for this variable, currently writing zero
	// 0x2932: Is music loaded? (Uint16BE, Boolean).
	out.writeUint16BE(0);
	// FIXME: Save proper value for this variable, currently writing zero
	// 0x2934: Is music playing? (Uint16BE, Boolean).
	out.writeUint16BE(0);

	out.writeUint16BE(renderer->_cmdY);
	out.writeUint16BE(0); // Some unknown variable that seems to always be zero
	out.writeUint16BE(allowPlayerInput);
	out.writeUint16BE(playerCommand);
	out.writeUint16BE(commandVar1);
	out.writeUint16BE(isDrawCommandEnabled);
	out.writeUint16BE(var5);
	out.writeUint16BE(var4);
	out.writeUint16BE(var3);
	out.writeUint16BE(var2);
	out.writeUint16BE(commandVar2);
	out.writeUint16BE(renderer->_messageBg);

	// FIXME: Save proper value for this variable, currently writing zero.
	// An unknown variable at 0x295E: adBgVar1 (Uint16BE).
	out.writeUint16BE(0);
	out.writeSint16BE(currentAdditionalBgIdx);
	out.writeSint16BE(currentAdditionalBgIdx2);
	// FIXME: Save proper value for this variable, currently writing zero.
	// 0x2954: additionalBgVScroll (Uint16BE). This probably means renderer->_bgShift.
	out.writeUint16BE(0);
	// FIXME: Save proper value for this variable, currently writing zero.
	// An unknown variable at 0x2956: adBgVar0 (Uint16BE). Maybe this means bgVar0?
	out.writeUint16BE(0);
	out.writeUint16BE(disableSystemMenu);

	saveAnimDataTable(out);
	saveScreenParams(out);
	saveGlobalScripts(out);
	saveObjectScripts(out);
	saveSeqList(out);
	saveOverlayList(out);
	saveBgIncrustList(out);
}

void CineEngine::makeSave(char *saveFileName) {
	Common::SharedPtr<Common::OutSaveFile> fHandle(_saveFileMan->openForSaving(saveFileName));

	setMouseCursor(MOUSE_CURSOR_DISK);

	if (!fHandle) {
		renderer->drawString(otherMessages[1], 0);
		waitPlayerInput();
		// restoreScreen();
		checkDataDisk(-1);
	} else {
		if (getGameType() == GType_FW) {
			makeSaveFW(*fHandle);
		} else {
			makeSaveOS(*fHandle);
		}
	}

	setMouseCursor(MOUSE_CURSOR_NORMAL);
}

/**
 * Load animDataTable from save
 * @param fHandle Savefile open for reading
 * @param saveGameFormat The used savegame format
 * @todo Add Operation Stealth savefile support
 *
 * Unlike the old code, this one actually rebuilds the table one frame
 * at a time.
 */
void loadResourcesFromSave(Common::SeekableReadStream &fHandle, enum CineSaveGameFormat saveGameFormat) {
	int16 foundFileIdx;
	char *animName, part[256], name[10];

	strcpy(part, currentPartName);

	// We only support these variations of the savegame format at the moment.
	assert(saveGameFormat == ANIMSIZE_23 || saveGameFormat == ANIMSIZE_30_PTRS_INTACT);

	const int entrySize = ((saveGameFormat == ANIMSIZE_23) ? 23 : 30);
	const int fileStartPos = fHandle.pos();

	for (int resourceIndex = 0; resourceIndex < NUM_MAX_ANIMDATA; resourceIndex++) {
		// Seek to the start of the current animation's entry
		fHandle.seek(fileStartPos + resourceIndex * entrySize);
		// Read in the current animation entry
		fHandle.readUint16BE(); // width
		fHandle.readUint16BE();
		fHandle.readUint16BE(); // bpp
		fHandle.readUint16BE(); // height

		bool validPtr = false;
		// Handle variables only present in animation entries of size 30
		if (entrySize == 30) {
			validPtr = (fHandle.readUint32BE() != 0); // Read data pointer
			fHandle.readUint32BE(); // Discard mask pointer
		}

		foundFileIdx = fHandle.readSint16BE();
		int16 frameIndex = fHandle.readSint16BE(); // frame
		fHandle.read(name, 10);

		// Handle variables only present in animation entries of size 23
		if (entrySize == 23) {
			validPtr = (fHandle.readByte() != 0);
		}

		// Don't try to load invalid entries.
		if (foundFileIdx < 0 || !validPtr) {
			//resourceIndex++; // Jump over the invalid entry
			continue;
		}

		// Alright, the animation entry looks to be valid so let's start handling it...
		if (strcmp(currentPartName, name) != 0) {
			closePart();
			loadPart(name);
		}

		animName = g_cine->_partBuffer[foundFileIdx].partName;
		loadRelatedPalette(animName); // Is this for Future Wars only?
		loadResource(animName, resourceIndex, frameIndex);
	}

	loadPart(part);

	// Make sure we jump over all the animation entries
	fHandle.seek(fileStartPos + NUM_MAX_ANIMDATA * entrySize);
}

} // End of namespace Cine
