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



#include "common/file.h"
#include "common/textconsole.h"

#include "agos/agos.h"
#include "agos/intern.h"

namespace AGOS {

// Script opcodes to load into memory
static const char *const opcodeArgTable_elvira1[300] = {
	"I ", "I ", "I ", "I ", "I ", "I ", "I ", "I ",	 "II ",	"II ", "II ", "II ", "F ", "F ", "FN ",
	"FN ", "FN ", "FN ", "FF ", "FF ", "FF ", "FF ", "II ", "II ", "a ", "a ", "n ", "n ", "p ",
	"N ", "I ", "I ", "I ",	 "I ",	"IN ",	"IB ", "IB ", "II ", "IB ", "N ", " ", " ", " ", "I ",
	"I ","I ","I ", "I ","I ","I ",	"II ","II ","II ","II ","IBF ", "FIB ", "FF ", "N ", "NI ",
	"IF ", "F ", "F ", "IB ", "IB ", "FN ",	"FN ", "FN ", "FF ", "FF ", "FN ", "FN ", "FF ", "FF ",
	"FN ", "FF ", "FN ", "F ", "I ", "IN ", "IN ", "IB ", "IB ", "IB ", "IB ", "II ", "I ", "I ",
	"IN ", "T ", "F ", " ", "T ", "T ", "I ", "I ", " ", " ", "T ", " ", " ", " ", " ", " ", "T ",
	" ", "N ", "INN ", "II ", "II ", "ITN ", "ITIN ", "ITIN ", "I3 ", "IN ", "I ",	 "I ", "Ivnn ",
	"vnn ", "Ivnn ", "NN ",	"IT ", "INN ", " ", "N ", "N ", "N ", "T ", "v ", " ", " ", " ", " ",
	"FN ", "I ", "TN ", "IT ", "II ", "I ", " ", "N ", "I ", " ", "I ", "NI ", "I ", "I ", "T ",
	"I ", "I ", "N ", "N ", " ", "N ", "IF ", "IF ", "IF ", "IF ", "IF ", "IF ", "T ", "IB ",
	"IB ", "IB ", "I ", " ", "vnnN ", "Ivnn ", "T ", "T ", "T ", "IF ", " ", " ", " ", "Ivnn ",
	"IF ", "INI ", "INN ", "IN ", "II ", "IFF ", "IIF ", "I ", "II ", "I ", "I ", "IN ", "IN ",
	"II ", "II ", "II ", "II ", "IIN ", "IIN ", "IN ", "II ", "IN ", "IN ", "T ", "vanpan ",
	"vIpI ", "T ", "T ", " ", " ",	"IN ", "IN ", "IN ", "IN ", "N ", "INTTT ", "ITTT ",
	"ITTT ", "I ", "I ", "IN ", "I ", " ", "F ", "NN ", "INN ", "INN ", "INNN ", "TF ", "NN ",
	"N ", "NNNNN ", "N ", " ", "NNNNNNN ", "N ", " ", "N ",	"NN ", "N ", "NNNNNIN ", "N ", "N ",
	"N ", "NNN ", "NNNN ", "INNN ", "IN ", "IN ", "TT ", "I ", "I ", "I ", "TTT ", "IN ", "IN ",
	"FN ", "FN ", "FN ", "N ", "N ", "N ", "NI ", " ", " ",	 "N ", "I ", "INN ", "NN ", "N ",
	"N ", "Nan ", "NN ", " ", " ", " ", " ", " ", " ", " ", "IF ", "N ", " ", " ",	 " ", "II ",
	" ", "NI ","N ",
};

static const char *const opcodeArgTable_elvira2[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "I  ", "I ", " ", "T ", " ", " ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", "T ", "T ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NBNNN ", "N ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "N ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NN ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ", "N ",
	"I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ",	"B ",
	"IBB ", "IBN ", "IB ", "B ", " ", "TB ", "TB ", "I ", "N ", "B ", "INB ", "INB ", "INB ", "INB ",
	"INB ", "INB ", "INB ", "N ", " ", "INBB ", "B ", "B ", "Ian ", "B ", "B ", "B ", "B ", "T ",
	"T ", "B ", " ", "I ", " ", " "
};

static const char *const opcodeArgTable_waxworks[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BT ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", "T ", "T ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NBNNN ", "N ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NN ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ", "N ",
	"I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ",	"B ",
	"IBB ", "IBN ", "IB ", "B ", " ", "TB ", "TB ", "I ", "N ", "B ", "INB ", "INB ", "INB ", "INB ",
	"INB ", "INB ", "INB ", "N ", " ", "INBB ", "B ", "B ", "Ian ", "B ", "B ", "B ", "B ", "T ",
	"T ", "B ", " ", "I ", " ", " "
};

static const char *const opcodeArgTable_simon1talkie[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BTS ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NBNNN ", "N ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NN ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ", "N ",
	"I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ", "B ",
	"IBB ", "IBN ", "IB ", "B ", "BNBN ", "BBTS ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ", "T ",
	"T ", "B ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", " ", " ", "N ", "N ", " ",
	" ",
};

static const char *const opcodeArgTable_simon1dos[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BT ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NBNNN ", "N ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NN ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ", "N ",
	"I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ", "B ",
	"IBB ", "IBN ", "IB ", "B ", "BNBN ", "BBT ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ", "T ",
	"T ", "B ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", " ", " ", "N ", "N ", " ",
	" ",
};

static const char *const opcodeArgTable_simon2talkie[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BTS ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NNBNNN ", "NN ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NNB ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ",
	"N ", "I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ",
	"B ", "IBB ", "IBN ", "IB ", "B ", "BNBN ", "BBTS ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ",
	"T ", "T ", "B ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", " ", " ", "N ", "N ",
	" ", " ", "BT ", " ", "B "
};

static const char *const opcodeArgTable_simon2dos[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BT ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NNBNNN ", "NN ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NNB ", "N ", "N ", "Ban ", "BB ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ",
	"N ", "I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ",
	"B ", "IBB ", "IBN ", "IB ", "B ", "BNBN ", "BBT ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ",
	"T ", "T ", "B ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", " ", " ", "N ", "N ",
	" ", " ", "BT ", " ", "B "
};

static const char *const opcodeArgTable_feeblefiles[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "B ", "B ", "BN ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BB ", "BB ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBB ", "BIB ", "BB ", "B ", "BI ", "IB ", "B ", "B ", "BN ",
	"BN ", "BN ", "BB ", "BB ", "BN ", "BN ", "BB ", "BB ", "BN ", "BB ", "BN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "B ", "T ", "T ", "NNNNNB ", "BT ", "BTS ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NNBNNN ", "NN ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NNB ", "N ", "N ", "Ban ", " ", " ", " ", " ", " ", "IB ", "B ", " ", "II ", " ", "BI ",
	"N ", "I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "B ", "B ", "B ",
	"B ", "IBB ", "IBN ", "IB ", "B ", "BNNN ", "BBTS ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ",
	"T ", "N ", " ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", "T ", " ", "N ", "N ",
	" ", " ", "BT ", " ", "B ", " ", "BBBB ", " ", " ", "BBBB ", "B ", "B ", "B ", "B "
};

static const char *const opcodeArgTable_puzzlepack[256] = {
	" ", "I ", "I ", "I ", "I ", "I ", "I ", "II ", "II ", "II ", "II ", "N ", "N ", "NN ", "NN ",
	"NN ", "NN ", "NN ", "NN ", "NN ", "NN ", "II ", "II ", "N ", "I ", "I ", "I ", "IN ", "IB ",
	"II ", "I ", "I ", "II ", "II ", "IBN ", "NIB ", "NN ", "B ", "BI ", "IN ", "N ", "N ", "NN ",
	"NN ", "NN ", "NN ", "NN ", "NN ", "NN ", "NN ", "NN ", "NN ", "NN ", "NN ", "B ", "I ", "IB ",
	"IB ", "II ", "I ", "I ", "IN ", "N ", "T ", "T ", "NNNNNB ", "BTNN ", "BTS ", "T ", " ", "B ",
	"N ", "IBN ", "I ", "I ", "I ", "NN ", " ", " ", "IT ", "II ", "I ", "B ", " ", "IB ", "IBB ",
	"IIB ", "T ", " ", " ", "IB ", "IB ", "IB ", "B ", "BB ", "IBB ", "NB ", "N ", "NNBNNN ", "NN ",
	" ", "BNNNNNN ", "B ", " ", "B ", "B ", "BB ", "NNNNNIN ", "N ", "N ", "N ", "NNN ", "NBNN ",
	"IBNN ", "IB ", "IB ", "IB ", "IB ", "N ", "N ", "N ", "BI ", " ", " ", "N ", "I ", "IBB ",
	"NNB ", "N ", "N ", "Ban ", " ", " ", " ", " ", " ", "IN ", "B ", " ", "II ", " ", "BI ",
	"N ", "I ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "IB ", "BI ", "BB ", "N ", "N ", "N ",
	"N ", "IBN ", "IBN ", "IN ", "B ", "BNNN ", "BBTS ", "N ", " ", "Ian ", "B ", "B ", "B ", "B ",
	"T ", "N ", " ", " ", "I ", " ", " ", "BBI ", "NNBB ", "BBB ", " ", " ", "T ", " ", "N ", "N ",
	" ", " ", "BT ", " ", "B ", " ", "BBBB ", " ", " ", "BBBB ", "B ", "B ", "B ", "B "
};

Subroutine *AGOSEngine::getSubroutineByID(uint subroutineId) {
	Subroutine *cur;

	for (cur = _subroutineList; cur; cur = cur->next) {
		if (cur->id == subroutineId)
			return cur;
	}

	if (loadXTablesIntoMem(subroutineId)) {
		for (cur = _subroutineList; cur; cur = cur->next) {
			if (cur->id == subroutineId)
				return cur;
		}
	}

	if (loadTablesIntoMem(subroutineId)) {
		for (cur = _subroutineList; cur; cur = cur->next) {
			if (cur->id == subroutineId)
				return cur;
		}
	}

	debug(0,"getSubroutineByID: subroutine %d not found", subroutineId);
	return NULL;
}

void AGOSEngine::alignTableMem() {
	if (!IS_ALIGNED(_tablesHeapPtr, 4)) {
		_tablesHeapPtr += 2;
		_tablesHeapCurPos += 2;
	}
}

void *AGOSEngine::allocateTable(uint size) {
	byte *org = _tablesHeapPtr;

	size = (size + 1) & ~1;

	_tablesHeapPtr += size;
	_tablesHeapCurPos += size;

	if (_tablesHeapCurPos > _tablesHeapSize)
		error("Tablesheap overflow");

	return org;
}

void AGOSEngine::allocTablesHeap() {
	_tablesHeapSize = _tableMemSize;
	_tablesHeapCurPos = 0;
	_tablesHeapPtr = (byte *)calloc(_tableMemSize, 1);
	if (!_tablesHeapPtr)
		error("Out Of Memory - Tables");
}

void AGOSEngine::endCutscene() {
	Subroutine *sub;

	_sound->stopVoice();

	sub = getSubroutineByID(170);
	if (sub != NULL)
		startSubroutineEx(sub);

	_runScriptReturn1 = true;
}

Common::SeekableReadStream *AGOSEngine::openTablesFile(const char *filename) {
	if (getFeatures() & GF_OLD_BUNDLE)
		return openTablesFile_simon1(filename);
	else
		return openTablesFile_gme(filename);
}

Common::SeekableReadStream *AGOSEngine::openTablesFile_simon1(const char *filename) {
	Common::SeekableReadStream *in = _archives.createReadStreamForMember(filename);
	if (!in)
		error("openTablesFile: Can't open '%s'", filename);
	return in;
}

Common::SeekableReadStream *AGOSEngine::openTablesFile_gme(const char *filename) {
	uint res;
	uint32 offs;

	res = atoi(filename + 6) + _tableIndexBase - 1;
	offs = _gameOffsetsPtr[res];

	_gameFile->seek(offs, SEEK_SET);
	return _gameFile;
}

bool AGOSEngine::loadTablesIntoMem(uint16 subrId) {
	byte *p;
	uint16 min_num, max_num, file_num;
	Common::SeekableReadStream *in;
	char filename[30];

	if (_tblList == NULL)
		return 0;

	p = _tblList + 32;

	min_num = READ_BE_UINT16(p);
	max_num = READ_BE_UINT16(p + 2);
	file_num = *(p + 4);
	p += 6;

	while (min_num) {
		if ((subrId >= min_num) && (subrId <= max_num)) {
			_subroutineList = _subroutineListOrg;
			_tablesHeapPtr = _tablesHeapPtrOrg;
			_tablesHeapCurPos = _tablesHeapCurPosOrg;
			_stringIdLocalMin = 1;
			_stringIdLocalMax = 0;

			sprintf(filename, "TABLES%.2d", file_num);
			in = openTablesFile(filename);
			readSubroutineBlock(in);
			closeTablesFile(in);

			alignTableMem();

			_tablesheapPtrNew = _tablesHeapPtr;
			_tablesHeapCurPosNew = _tablesHeapCurPos;

			if (_tablesHeapCurPos > _tablesHeapSize)
				error("loadTablesIntoMem: Out of table memory");
			return 1;
		}

		min_num = READ_BE_UINT16(p);
		max_num = READ_BE_UINT16(p + 2);
		file_num = *(p + 4);
		p += 6;
	}

	debug(1,"loadTablesIntoMem: didn't find %d", subrId);
	return 0;
}

bool AGOSEngine_Waxworks::loadTablesIntoMem(uint16 subrId) {
	byte *p;
	uint min_num, max_num;
	Common::SeekableReadStream *in;

	p = _tblList;
	if (p == NULL)
		return 0;

	while (*p) {
		Common::String filename;
		while (*p)
			filename += *p++;
		p++;

		if (getPlatform() == Common::kPlatformAcorn) {
			filename += ".DAT";
		}

		for (;;) {
			min_num = READ_BE_UINT16(p); p += 2;
			if (min_num == 0)
				break;

			max_num = READ_BE_UINT16(p); p += 2;

			if (subrId >= min_num && subrId <= max_num) {
				_subroutineList = _subroutineListOrg;
				_tablesHeapPtr = _tablesHeapPtrOrg;
				_tablesHeapCurPos = _tablesHeapCurPosOrg;
				_stringIdLocalMin = 1;
				_stringIdLocalMax = 0;

				in = openTablesFile(filename.c_str());
				readSubroutineBlock(in);
				closeTablesFile(in);
				if (getGameType() == GType_SIMON2) {
					_sound->loadSfxTable(getFileName(GAME_GMEFILE), _gameOffsetsPtr[atoi(filename.c_str() + 6) - 1 + _soundIndexBase]);
				} else if (getGameType() == GType_SIMON1 && getPlatform() == Common::kPlatformWindows) {
					filename.setChar('S', 0);
					filename.setChar('F', 1);
					filename.setChar('X', 2);
					filename.setChar('X', 3);
					filename.setChar('X', 4);
					filename.setChar('X', 5);
					if (atoi(filename.c_str() + 6) != 1 && atoi(filename.c_str() + 6) != 30)
						_sound->readSfxFile(filename);
				}

				alignTableMem();

				_tablesheapPtrNew = _tablesHeapPtr;
				_tablesHeapCurPosNew = _tablesHeapCurPos;

				if (_tablesHeapCurPos > _tablesHeapSize)
					error("loadTablesIntoMem: Out of table memory");
				return 1;
			}
		}
	}

	debug(1,"loadTablesIntoMem: didn't find %d", subrId);
	return 0;
}

bool AGOSEngine::loadXTablesIntoMem(uint16 subrId) {
	byte *p;
	int i;
	uint min_num, max_num;
	char filename[30];
	Common::SeekableReadStream *in;

	p = _xtblList;
	if (p == NULL)
		return 0;

	while (*p) {
		for (i = 0; *p; p++, i++)
			filename[i] = *p;
		filename[i] = 0;
		p++;

		for (;;) {
			min_num = READ_BE_UINT16(p);
			p += 2;

			if (min_num == 0)
				break;

			max_num = READ_BE_UINT16(p);
			p += 2;

			if (subrId >= min_num && subrId <= max_num) {
				_subroutineList = _xsubroutineListOrg;
				_tablesHeapPtr = _xtablesHeapPtrOrg;
				_tablesHeapCurPos = _xtablesHeapCurPosOrg;
				_stringIdLocalMin = 1;
				_stringIdLocalMax = 0;

				in = openTablesFile(filename);
				readSubroutineBlock(in);
				closeTablesFile(in);

				alignTableMem();

				_subroutineListOrg = _subroutineList;
				_tablesHeapPtrOrg = _tablesHeapPtr;
				_tablesHeapCurPosOrg = _tablesHeapCurPos;
				_tablesheapPtrNew = _tablesHeapPtr;
				_tablesHeapCurPosNew = _tablesHeapCurPos;

				return 1;
			}
		}
	}

	debug(1,"loadXTablesIntoMem: didn't find %d", subrId);
	return 0;
}

void AGOSEngine::closeTablesFile(Common::SeekableReadStream *in) {
	if (getFeatures() & GF_OLD_BUNDLE) {
		delete in;
	}
}

Subroutine *AGOSEngine::createSubroutine(uint16 id) {
	Subroutine *sub;

	alignTableMem();

	sub = (Subroutine *)allocateTable(sizeof(Subroutine));
	sub->id = id;
	sub->first = 0;
	sub->next = _subroutineList;
	_subroutineList = sub;
	return sub;
}

SubroutineLine *AGOSEngine::createSubroutineLine(Subroutine *sub, int where) {
	SubroutineLine *sl, *cur_sl = NULL, *last_sl = NULL;

	if (sub->id == 0)
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_BIG_SIZE);
	else
		sl = (SubroutineLine *)allocateTable(SUBROUTINE_LINE_SMALL_SIZE);

	// where is what offset to insert the line at, locate the proper beginning line
	if (sub->first != 0) {
		cur_sl = (SubroutineLine *)((byte *)sub + sub->first);
		while (where) {
			last_sl = cur_sl;
			cur_sl = (SubroutineLine *)((byte *)sub + cur_sl->next);
			if ((byte *)cur_sl == (byte *)sub)
				break;
			where--;
		}
	}

	if (last_sl != NULL) {
		// Insert the subroutine line in the middle of the link
		last_sl->next = (byte *)sl - (byte *)sub;
		sl->next = (byte *)cur_sl - (byte *)sub;
	} else {
		// Insert the subroutine line at the head of the link
		sl->next = sub->first;
		sub->first = (byte *)sl - (byte *)sub;
	}

	return sl;
}

void AGOSEngine::runSubroutine101() {
	Subroutine *sub;

	sub = getSubroutineByID(101);
	if (sub != NULL)
		startSubroutineEx(sub);

	permitInput();
}

int AGOSEngine::startSubroutine(Subroutine *sub) {
	int result = -1;
	SubroutineLine *sl = (SubroutineLine *)((byte *)sub + sub->first);

	const byte *old_code_ptr = _codePtr;
	Subroutine *old_currentTable = _currentTable;
	SubroutineLine *old_currentLine = _currentLine;
	SubroutineLine *old_classLine = _classLine;
	int16 old_classMask = _classMask;
	int16 old_classMode1 = _classMode1;
	int16 old_classMode2 = _classMode2;

	_classLine = 0;
	_classMask = 0;
	_classMode1 = 0;
	_classMode2 = 0;

	if (_dumpScripts)
		dumpSubroutine(sub);

	if (++_recursionDepth > 40)
		error("Recursion error");

	// WORKAROUND: If the game is saved, right after Simon is thrown in the dungeon of Sordid's Fortress of Doom,
	// the saved game fails to load correctly. When loading the saved game, the sequence of Simon waking is started,
	// before the scene is actually reloaded, due to a script bug. We manually add the extra script code from the
	// updated DOS CD release, which fixed this particular script bug.
	if (getGameType() == GType_SIMON2 && sub->id == 12101) {
		const byte bit = 228;
		if ((_bitArrayTwo[bit / 16] & (1 << (bit & 15))) != 0 && (int)readVariable(34) == -1) {
			_bitArrayTwo[228 / 16] &= ~(1 << (bit & 15));
			writeVariable(34, 1);
		}
	}

	_currentTable = sub;
restart:

	if (shouldQuit())
		return result;

	while ((byte *)sl != (byte *)sub) {
		_currentLine = sl;
		if (checkIfToRunSubroutineLine(sl, sub)) {
			_codePtr = (byte *)sl;
			if (sub->id)
				_codePtr += 2;
			else
				_codePtr += 8;

			if (_dumpOpcodes)
				debug("; %d", sub->id);
			result = runScript();
			if (result != 0) {
				break;
			}
		}
		sl = (SubroutineLine *)((byte *)sub + sl->next);
	}

	// WORKAROUND: Feeble walks in the incorrect direction, when looking at the Vent in the Research and Testing area of
	// the Company Central Command Compound. We manually add the extra script code from the updated English 2CD release,
	// which fixed this particular script bug.
	if (getGameType() == GType_FF && _language == Common::EN_ANY) {
		if (sub->id == 39125 && readVariable(84) == 2) {
			writeVariable(1, 1136);
			writeVariable(2, 346);
		}
		if (sub->id == 39126 && readVariable(84) == 2) {
			Subroutine *tmpSub = getSubroutineByID(80);
			if (tmpSub != NULL) {
				startSubroutine(tmpSub);
			}
		}
	}

	if (_classMode1) {
		_subjectItem = nextInByClass(_subjectItem, _classMask);
		if (!_subjectItem) {
			_classMode1 = 0;
		} else {
			delay(0);
			sl = _classLine;	/* Rescanner */
			goto restart;
		}
	}
	if (_classMode2) {
		_objectItem = nextInByClass(_objectItem, _classMask);
		if (!_objectItem) {
			_classMode2 = 0;
		} else {
			delay(0);
			sl = _classLine;	/* Rescanner */
			goto restart;
		}
	}

	/* result -10 means restart subroutine */
	if (result == -10) {
		delay(0);
		sl = (SubroutineLine *)((byte *)sub + sub->first);
		goto restart;
	}

	_codePtr = old_code_ptr;
	_currentLine = old_currentLine;
	_currentTable = old_currentTable;
	_classLine = old_classLine;
	_classMask = old_classMask;
	_classMode1 = old_classMode2;
	_classMode2 = old_classMode1;
	_findNextPtr = 0;

	_recursionDepth--;
	return result;
}

int AGOSEngine::startSubroutineEx(Subroutine *sub) {
	return startSubroutine(sub);
}

bool AGOSEngine::checkIfToRunSubroutineLine(SubroutineLine *sl, Subroutine *sub) {
	if (sub->id)
		return true;

	if (sl->verb != -1 && sl->verb != _scriptVerb &&
			(sl->verb != -2 || _scriptVerb != -1))
		return false;

	if (sl->noun1 != -1 && sl->noun1 != _scriptNoun1 &&
			(sl->noun1 != -2 || _scriptNoun1 != -1))
		return false;

	if (sl->noun2 != -1 && sl->noun2 != _scriptNoun2 &&
			(sl->noun2 != -2 || _scriptNoun2 != -1))
		return false;

	return true;
}

void AGOSEngine::readSubroutineBlock(Common::SeekableReadStream *in) {
	while (in->readUint16BE() == 0) {
		readSubroutine(in, createSubroutine(in->readUint16BE()));
	}
}
void AGOSEngine::readSubroutine(Common::SeekableReadStream *in, Subroutine *sub) {
	while (in->readUint16BE() == 0) {
		readSubroutineLine(in, createSubroutineLine(sub, 0xFFFF), sub);
	}
}

void AGOSEngine::readSubroutineLine(Common::SeekableReadStream *in, SubroutineLine *sl, Subroutine *sub) {
	byte line_buffer[2048], *q = line_buffer;
	int size;

	if (sub->id == 0) {
		sl->verb = in->readUint16BE();
		sl->noun1 = in->readUint16BE();
		sl->noun2 = in->readUint16BE();
	} else if (getGameType() == GType_ELVIRA1) {
		in->readUint16BE();
		in->readUint16BE();
		in->readUint16BE();
	}

	if (getGameType() == GType_ELVIRA1) {
		int16 tmp = in->readUint16BE();
		WRITE_BE_UINT16(q, tmp);
		while (tmp != 10000) {
			if (READ_BE_UINT16(q) == 198) {
				in->readUint16BE();
			} else {
				q = readSingleOpcode(in, q);
			}

			tmp = in->readUint16BE();
			WRITE_BE_UINT16(q, tmp);
		}
	} else {
		while ((*q = in->readByte()) != 0xFF) {
			if (*q == 87) {
				in->readUint16BE();
			} else {
				q = readSingleOpcode(in, q);
			}
		}
	}

	size = (q - line_buffer + 2);
	memcpy(allocateTable(size), line_buffer, size);
}

byte *AGOSEngine::readSingleOpcode(Common::SeekableReadStream *in, byte *ptr) {
	int i, l;
	const char *stringPtr;
	uint16 opcode, val;

	const char *const *table;

	if (getGameType() == GType_PP)
		table = opcodeArgTable_puzzlepack;
	else if (getGameType() == GType_FF)
		table = opcodeArgTable_feeblefiles;
	else if (getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE))
		table = opcodeArgTable_simon2talkie;
	else if (getGameType() == GType_SIMON2)
		table = opcodeArgTable_simon2dos;
	else if (getGameType() == GType_SIMON1 && (getFeatures() & GF_TALKIE))
		table = opcodeArgTable_simon1talkie;
	else if (getGameType() == GType_SIMON1)
		table = opcodeArgTable_simon1dos;
	else if (getGameType() == GType_WW)
		table = opcodeArgTable_waxworks;
	else if (getGameType() == GType_ELVIRA2)
		table = opcodeArgTable_elvira2;
	else
		table = opcodeArgTable_elvira1;

	i = 0;
	if (getGameType() == GType_ELVIRA1) {
		opcode = READ_BE_UINT16(ptr);
		ptr += 2;
	} else {
		opcode = *ptr++;
	}

	stringPtr = table[opcode];
	if (!stringPtr)
		error("Unable to locate opcode table. Perhaps you are using the wrong game target?");

	for (;;) {
		if (stringPtr[i] == ' ')
			return ptr;

		l = stringPtr[i++];

		switch (l) {
		case 'F':
		case 'N':
		case 'S':
		case 'a':
		case 'n':
		case 'p':
		case 'v':
		case '3':
			val = in->readUint16BE();
			WRITE_BE_UINT16(ptr, val); ptr += 2;
			break;

		case 'B':
			if (getGameType() == GType_ELVIRA1) {
				val = in->readUint16BE();
				WRITE_BE_UINT16(ptr, val); ptr += 2;
			} else {
				*ptr++ = in->readByte();
				if (ptr[-1] == 0xFF) {
					*ptr++ = in->readByte();
				}
			}
			break;

		case 'I':
			val = in->readUint16BE();
			switch (val) {
			case 1:
				val = 0xFFFF;
				break;
			case 3:
				val = 0xFFFD;
				break;
			case 5:
				val = 0xFFFB;
				break;
			case 7:
				val = 0xFFF9;
				break;
			case 9:
				val = 0xFFF7;
				break;
			default:
				val = fileReadItemID(in);
				break;
			}
			WRITE_BE_UINT16(ptr, val); ptr += 2;
			break;

		case 'T':
			val = in->readUint16BE();
			switch (val) {
			case 0:
				val = 0xFFFF;
				break;
			case 3:
				val = 0xFFFD;
				break;
			default:
				val = (uint16)in->readUint32BE();
				break;
			}
			WRITE_BE_UINT16(ptr, val); ptr += 2;
			break;
		default:
			error("readSingleOpcode: Bad cmd table entry %c", l);
		}
	}
}

} // End of namespace AGOS
