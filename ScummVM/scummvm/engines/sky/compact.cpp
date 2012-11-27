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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */


#include "common/debug.h"
#include "common/endian.h"
#include "common/file.h"
#include "common/textconsole.h"
#include "common/translation.h"
#include "sky/compact.h"
#include "gui/message.h"
#include <stddef.h>	// for ptrdiff_t

namespace Sky {

#define	SKY_CPT_SIZE	419427

#define OFFS(type,item) (((ptrdiff_t)(&((type *)42)->item))-42)
#define MK32(type,item) OFFS(type, item),0,0,0
#define MK16(type,item) OFFS(type, item),0
#define MK32_A5(type, item) MK32(type, item[0]), MK32(type, item[1]), \
	MK32(type, item[2]), MK32(type, item[3]), MK32(type, item[4])

static const uint32 compactOffsets[] = {
	MK16(Compact, logic),
	MK16(Compact, status),
	MK16(Compact, sync),
	MK16(Compact, screen),
	MK16(Compact, place),
	MK32(Compact, getToTableId),
	MK16(Compact, xcood),
	MK16(Compact, ycood),
	MK16(Compact, frame),
	MK16(Compact, cursorText),
	MK16(Compact, mouseOn),
	MK16(Compact, mouseOff),
	MK16(Compact, mouseClick),
	MK16(Compact, mouseRelX),
	MK16(Compact, mouseRelY),
	MK16(Compact, mouseSizeX),
	MK16(Compact, mouseSizeY),
	MK16(Compact, actionScript),
	MK16(Compact, upFlag),
	MK16(Compact, downFlag),
	MK16(Compact, getToFlag),
	MK16(Compact, flag),
	MK16(Compact, mood),
	MK32(Compact, grafixProgId),
	MK16(Compact, offset),
	MK16(Compact, mode),
	MK16(Compact, baseSub),
	MK16(Compact, baseSub_off),
	MK16(Compact, actionSub),
	MK16(Compact, actionSub_off),
	MK16(Compact, getToSub),
	MK16(Compact, getToSub_off),
	MK16(Compact, extraSub),
	MK16(Compact, extraSub_off),
	MK16(Compact, dir),
	MK16(Compact, stopScript),
	MK16(Compact, miniBump),
	MK16(Compact, leaving),
	MK16(Compact, atWatch),
	MK16(Compact, atWas),
	MK16(Compact, alt),
	MK16(Compact, request),
	MK16(Compact, spWidth_xx),
	MK16(Compact, spColor),
	MK16(Compact, spTextId),
	MK16(Compact, spTime),
	MK16(Compact, arAnimIndex),
	MK32(Compact, turnProgId),
	MK16(Compact, waitingFor),
	MK16(Compact, arTargetX),
	MK16(Compact, arTargetY),
	MK32(Compact, animScratchId),
	MK16(Compact, megaSet),
};

static const uint32 megaSetOffsets[] = {
	MK16(MegaSet, gridWidth),
	MK16(MegaSet, colOffset),
	MK16(MegaSet, colWidth),
	MK16(MegaSet, lastChr),
	MK32(MegaSet, animUpId),
	MK32(MegaSet, animDownId),
	MK32(MegaSet, animLeftId),
	MK32(MegaSet, animRightId),
	MK32(MegaSet, standUpId),
	MK32(MegaSet, standDownId),
	MK32(MegaSet, standLeftId),
	MK32(MegaSet, standRightId),
	MK32(MegaSet, standTalkId),
};

static const uint32 turnTableOffsets[] = {
	MK32_A5(TurnTable, turnTableUp),
	MK32_A5(TurnTable, turnTableDown),
	MK32_A5(TurnTable, turnTableLeft),
	MK32_A5(TurnTable, turnTableRight),
	MK32_A5(TurnTable, turnTableTalk),
};

#define COMPACT_SIZE (sizeof(compactOffsets)/sizeof(uint32))
#define MEGASET_SIZE (sizeof(megaSetOffsets)/sizeof(uint32))
#define TURNTABLE_SIZE (sizeof(turnTableOffsets)/sizeof(uint32))

SkyCompact::SkyCompact() {
	_cptFile = new Common::File();
	if (!_cptFile->open("sky.cpt")) {
		GUI::MessageDialog dialog(_("Unable to find \"sky.cpt\" file!\n"
								  "Please download it from www.scummvm.org"), _("OK"), NULL);
		dialog.runModal();
		error("Unable to find \"sky.cpt\" file\nPlease download it from www.scummvm.org");
	}

	uint16 fileVersion = _cptFile->readUint16LE();
	if (fileVersion != 0)
		error("unknown \"sky.cpt\" version");

	if (SKY_CPT_SIZE != _cptFile->size()) {
		GUI::MessageDialog dialog(_("The \"sky.cpt\" file has an incorrect size.\nPlease (re)download it from www.scummvm.org"), _("OK"), NULL);
		dialog.runModal();
		error("Incorrect sky.cpt size (%d, expected: %d)", _cptFile->size(), SKY_CPT_SIZE);
	}

	// set the necessary data structs up...
	_numDataLists = _cptFile->readUint16LE();
	_cptNames	  = (char***)malloc(_numDataLists * sizeof(char**));
	_dataListLen  = (uint16 *)malloc(_numDataLists * sizeof(uint16));
	_cptSizes	  = (uint16 **)malloc(_numDataLists * sizeof(uint16 *));
	_cptTypes	  = (uint16 **)malloc(_numDataLists * sizeof(uint16 *));
	_compacts	  = (Compact***)malloc(_numDataLists * sizeof(Compact**));

	for (int i = 0; i < _numDataLists; i++) {
		_dataListLen[i] = _cptFile->readUint16LE();
		_cptNames[i] = (char**)malloc(_dataListLen[i] * sizeof(char *));
		_cptSizes[i] = (uint16 *)malloc(_dataListLen[i] * sizeof(uint16));
		_cptTypes[i] = (uint16 *)malloc(_dataListLen[i] * sizeof(uint16));
		_compacts[i] = (Compact**)malloc(_dataListLen[i] * sizeof(Compact *));
	}

	uint32 rawSize = _cptFile->readUint32LE() * sizeof(uint16);
	uint16 *rawPos = _rawBuf = (uint16 *)malloc(rawSize);

	uint32 srcSize = _cptFile->readUint32LE() * sizeof(uint16);
	uint16 *srcBuf = (uint16 *)malloc(srcSize);
	uint16 *srcPos = srcBuf;
	_cptFile->read(srcBuf, srcSize);

	uint32 asciiSize = _cptFile->readUint32LE();
	char *asciiPos = _asciiBuf = (char *)malloc(asciiSize);
	_cptFile->read(_asciiBuf, asciiSize);

	// and fill them with the compact data
	for (uint32 lcnt = 0; lcnt < _numDataLists; lcnt++) {
		for (uint32 ecnt = 0; ecnt < _dataListLen[lcnt]; ecnt++) {
			_cptSizes[lcnt][ecnt] = READ_LE_UINT16(srcPos++);
			if (_cptSizes[lcnt][ecnt]) {
				_cptTypes[lcnt][ecnt] = READ_LE_UINT16(srcPos++);
				_compacts[lcnt][ecnt] = (Compact *)rawPos;
				_cptNames[lcnt][ecnt] = asciiPos;
				asciiPos += strlen(asciiPos) + 1;

				for (uint16 elemCnt = 0; elemCnt < _cptSizes[lcnt][ecnt]; elemCnt++)
					*rawPos++ = READ_LE_UINT16(srcPos++);
			} else {
				_cptTypes[lcnt][ecnt] = 0;
				_compacts[lcnt][ecnt] = NULL;
				_cptNames[lcnt][ecnt] = NULL;
			}
		}
	}
	free(srcBuf);

	uint16 numDlincs = _cptFile->readUint16LE();
	uint16 *dlincBuf = (uint16 *)malloc(numDlincs * 2 * sizeof(uint16));
	uint16 *dlincPos = dlincBuf;
	_cptFile->read(dlincBuf, numDlincs * 2 * sizeof(uint16));
	// these compacts don't actually exist but only point to other ones...
	uint16 cnt;
	for (cnt = 0; cnt < numDlincs; cnt++) {
		uint16 dlincId = READ_LE_UINT16(dlincPos++);
		uint16 destId = READ_LE_UINT16(dlincPos++);
		assert(((dlincId >> 12) < _numDataLists) && ((dlincId & 0xFFF) < _dataListLen[dlincId >> 12]) && (_compacts[dlincId >> 12][dlincId & 0xFFF] == NULL));
		_compacts[dlincId >> 12][dlincId & 0xFFF] = _compacts[destId >> 12][destId & 0xFFF];

		assert(_cptNames[dlincId >> 12][dlincId & 0xFFF] == NULL);
		_cptNames[dlincId >> 12][dlincId & 0xFFF] = asciiPos;
		asciiPos += strlen(asciiPos) + 1;
	}
	free(dlincBuf);

	// if this is v0.0288, parse this diff data
	uint16 numDiffs = _cptFile->readUint16LE();
	uint16 diffSize = _cptFile->readUint16LE();
	uint16 *diffBuf = (uint16 *)malloc(diffSize * sizeof(uint16));
	_cptFile->read(diffBuf, diffSize * sizeof(uint16));
	if (SkyEngine::_systemVars.gameVersion == 288) {
		uint16 *diffPos = diffBuf;
		for (cnt = 0; cnt < numDiffs; cnt++) {
			uint16 cptId = READ_LE_UINT16(diffPos++);
			uint16 *rawCpt = (uint16 *)fetchCpt(cptId);
			rawCpt += READ_LE_UINT16(diffPos++);
			uint16 len = READ_LE_UINT16(diffPos++);
			for (uint16 elemCnt = 0; elemCnt < len; elemCnt++)
				rawCpt[elemCnt] = READ_LE_UINT16(diffPos++);
		}
		assert(diffPos == (diffBuf + diffSize));
	}
	free(diffBuf);

	// these are the IDs that have to be saved into savegame files.
	_numSaveIds = _cptFile->readUint16LE();
	_saveIds = (uint16 *)malloc(_numSaveIds * sizeof(uint16));
	_cptFile->read(_saveIds, _numSaveIds * sizeof(uint16));
	for (cnt = 0; cnt < _numSaveIds; cnt++)
		_saveIds[cnt] = FROM_LE_16(_saveIds[cnt]);
	_resetDataPos = _cptFile->pos();
}

SkyCompact::~SkyCompact() {
	free(_rawBuf);
	free(_asciiBuf);
	free(_saveIds);
	for (int i = 0; i < _numDataLists; i++) {
		free(_cptNames[i]);
		free(_cptSizes[i]);
		free(_cptTypes[i]);
		free(_compacts[i]);
	}
	free(_cptNames);
	free(_dataListLen);
	free(_cptSizes);
	free(_cptTypes);
	free(_compacts);
	_cptFile->close();
	delete _cptFile;
}

// needed for some workaround where the engine has to check if it's currently processing joey, for example
bool SkyCompact::cptIsId(Compact *cpt, uint16 id) {
	return (cpt == fetchCpt(id));
}

Compact *SkyCompact::fetchCpt(uint16 cptId) {
	if (cptId == 0xFFFF) // is this really still necessary?
		return NULL;
	assert(((cptId >> 12) < _numDataLists) && ((cptId & 0xFFF) < _dataListLen[cptId >> 12]));

	debug(8, "Loading Compact %s [%s] (%04X=%d,%d)", _cptNames[cptId >> 12][cptId & 0xFFF], nameForType(_cptTypes[cptId >> 12][cptId & 0xFFF]), cptId, cptId >> 12, cptId & 0xFFF);

	return _compacts[cptId >> 12][cptId & 0xFFF];
}

Compact *SkyCompact::fetchCptInfo(uint16 cptId, uint16 *elems, uint16 *type, char *name) {
	assert(((cptId >> 12) < _numDataLists) && ((cptId & 0xFFF) < _dataListLen[cptId >> 12]));
	if (elems)
		*elems = _cptSizes[cptId >> 12][cptId & 0xFFF];
	if (type)
		*type  = _cptTypes[cptId >> 12][cptId & 0xFFF];
	if (name) {
		if (_cptNames[cptId >> 12][cptId & 0xFFF] != NULL)
			strcpy(name, _cptNames[cptId >> 12][cptId & 0xFFF]);
		else
			strcpy(name, "(null)");
	}
	return fetchCpt(cptId);
}

const char *SkyCompact::nameForType(uint16 type) {
	if (type >= NUM_CPT_TYPES)
		return "unknown";
	else
		return _typeNames[type];
}

uint16 *SkyCompact::getSub(Compact *cpt, uint16 mode) {
	switch (mode) {
	case 0:
		return &(cpt->baseSub);
	case 2:
		return &(cpt->baseSub_off);
	case 4:
		return &(cpt->actionSub);
	case 6:
		return &(cpt->actionSub_off);
	case 8:
		return &(cpt->getToSub);
	case 10:
		return &(cpt->getToSub_off);
	case 12:
		return &(cpt->extraSub);
	case 14:
		return &(cpt->extraSub_off);
	default:
		error("Invalid Mode (%d)", mode);
	}
}

uint16 *SkyCompact::getGrafixPtr(Compact *cpt) {
	uint16 *gfxBase = (uint16 *)fetchCpt(cpt->grafixProgId);
	if (gfxBase == NULL)
		return NULL;

	return gfxBase + cpt->grafixProgPos;
}

/**
 * Returns the n'th mega set specified by \a megaSet from Compact \a cpt.
 */
MegaSet *SkyCompact::getMegaSet(Compact *cpt) {
	switch (cpt->megaSet) {
	case 0:
		return &cpt->megaSet0;
	case NEXT_MEGA_SET:
		return &cpt->megaSet1;
	case NEXT_MEGA_SET*2:
		return &cpt->megaSet2;
	case NEXT_MEGA_SET*3:
		return &cpt->megaSet3;
	default:
		error("Invalid MegaSet (%d)", cpt->megaSet);
	}
}

/**
 \brief Returns the turn table for direction \a dir
	from Compact \a cpt in \a megaSet.

 Functionally equivalent to:
 \verbatim
 clear eax
 mov al,20
 mul (cpt[esi]).c_dir
 add ax,(cpt[esi]).c_mega_set
 lea eax,(cpt[esi+eax]).c_turn_table_up
 \endverbatim
*/
uint16 *SkyCompact::getTurnTable(Compact *cpt, uint16 dir) {
	MegaSet *m = getMegaSet(cpt);
	TurnTable *turnTable = (TurnTable *)fetchCpt(m->turnTableId);
	switch (dir) {
	case 0:
		return turnTable->turnTableUp;
	case 1:
		return turnTable->turnTableDown;
	case 2:
		return turnTable->turnTableLeft;
	case 3:
		return turnTable->turnTableRight;
	case 4:
		return turnTable->turnTableTalk;
	default:
		error("No TurnTable (%d) in MegaSet (%d)", dir, cpt->megaSet);
	}
}

void *SkyCompact::getCompactElem(Compact *cpt, uint16 off) {
	if (off < COMPACT_SIZE)
		return((uint8 *)cpt + compactOffsets[off]);
	off -= COMPACT_SIZE;

	if (off < MEGASET_SIZE)
		return((uint8 *)&(cpt->megaSet0) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)fetchCpt(cpt->megaSet0.turnTableId) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)&(cpt->megaSet1) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)fetchCpt(cpt->megaSet1.turnTableId) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)&(cpt->megaSet2) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)fetchCpt(cpt->megaSet2.turnTableId) + turnTableOffsets[off]);

	off -= TURNTABLE_SIZE;
	if (off < MEGASET_SIZE)
		return((uint8 *)&(cpt->megaSet3) + megaSetOffsets[off]);

	off -= MEGASET_SIZE;
	if (off < TURNTABLE_SIZE)
		return ((uint8 *)fetchCpt(cpt->megaSet3.turnTableId) + turnTableOffsets[off]);
	off -= TURNTABLE_SIZE;

	error("Offset %X out of bounds of compact", (int)(off + COMPACT_SIZE + 4 * MEGASET_SIZE + 4 * TURNTABLE_SIZE));
}

uint8 *SkyCompact::createResetData(uint16 gameVersion) {
	_cptFile->seek(_resetDataPos);
	uint32 dataSize = _cptFile->readUint16LE() * sizeof(uint16);
	uint16 *resetBuf = (uint16 *)malloc(dataSize);
	_cptFile->read(resetBuf, dataSize);
	uint16 numDiffs = _cptFile->readUint16LE();
	for (uint16 cnt = 0; cnt < numDiffs; cnt++) {
		uint16 version = _cptFile->readUint16LE();
		uint16 diffFields = _cptFile->readUint16LE();
		if (version == gameVersion) {
			for (uint16 diffCnt = 0; diffCnt < diffFields; diffCnt++) {
				uint16 pos = _cptFile->readUint16LE();
				resetBuf[pos] = TO_LE_16(_cptFile->readUint16LE());
			}
			return (uint8 *)resetBuf;
		} else
			_cptFile->seek(diffFields * 2 * sizeof(uint16), SEEK_CUR);
	}
	free(resetBuf);
	error("Unable to find reset data for Beneath a Steel Sky Version 0.0%03d", gameVersion);
}

// - debugging functions

uint16 SkyCompact::findCptId(void *cpt) {
	for (uint16 listCnt = 0; listCnt < _numDataLists; listCnt++)
		for (uint16 elemCnt = 0; elemCnt < _dataListLen[listCnt]; elemCnt++)
			if (_compacts[listCnt][elemCnt] == cpt)
				return (listCnt << 12) | elemCnt;
	// not found
	debug(1, "Id for Compact %p wasn't found", cpt);
	return 0;
}

uint16 SkyCompact::findCptId(const char *cptName) {
	for (uint16 listCnt = 0; listCnt < _numDataLists; listCnt++)
		for (uint16 elemCnt = 0; elemCnt < _dataListLen[listCnt]; elemCnt++)
			if (_cptNames[listCnt][elemCnt] != 0)
				if (scumm_stricmp(cptName, _cptNames[listCnt][elemCnt]) == 0)
					return (listCnt << 12) | elemCnt;
	// not found
	debug(1, "Id for Compact %s wasn't found", cptName);
	return 0;
}

uint16 SkyCompact::giveNumDataLists() {
	return _numDataLists;
}

uint16 SkyCompact::giveDataListLen(uint16 listNum) {
	if (listNum >= _numDataLists) // list doesn't exist
		return 0;
	else
		return _dataListLen[listNum];
}

const char *const SkyCompact::_typeNames[NUM_CPT_TYPES] = {
	"null",
	"COMPACT",
	"TURNTABLE",
	"ANIM SEQ",
	"UNKNOWN",
	"GETTOTABLE",
	"AR BUFFER",
	"MAIN LIST"
};

} // End of namespace Sky
