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
#include "kyra/kyra_lok.h"
#include "kyra/kyra_hof.h"
#include "kyra/kyra_mr.h"
#include "kyra/screen.h"
#include "kyra/screen_lok.h"
#include "kyra/screen_hof.h"
#include "kyra/screen_mr.h"
#include "kyra/gui_lok.h"
#include "kyra/gui_hof.h"
#include "kyra/gui_mr.h"
#include "kyra/sequences_hof.h"
#include "kyra/sound_intern.h"

#include "common/endian.h"
#include "common/md5.h"

namespace Kyra {

#define RESFILE_VERSION 83

namespace {
bool checkKyraDat(Common::SeekableReadStream *file) {
	if (!file)
		return false;

	uint32 size = file->size() - 16;
	uint8 digest[16];
	file->seek(size, SEEK_SET);
	if (file->read(digest, 16) != 16)
		return false;

	uint8 digestCalc[16];
	file->seek(0, SEEK_SET);
	if (!Common::computeStreamMD5(*file, digestCalc, size))
		return false;

	for (int i = 0; i < 16; ++i)
		if (digest[i] != digestCalc[i])
			return false;
	return true;
}

struct IndexTable {
	int type;
	int value;

	bool operator==(int t) const {
		return (type == t);
	}
};

const IndexTable iGameTable[] = {
	{ GI_KYRA1, 0 },
	{ GI_KYRA2, 1 },
	{ GI_KYRA3, 2 },
	{ GI_EOB1, 3 },
	{ GI_EOB2, 4 },
	{ GI_LOL, 5 },
	{ -1, -1 }
};

byte getGameID(const GameFlags &flags) {
	return Common::find(iGameTable, ARRAYEND(iGameTable) - 1, flags.gameID)->value;
}

const IndexTable iLanguageTable[] = {
	{ Common::EN_ANY, 1 },
	{ Common::FR_FRA, 2 },
	{ Common::DE_DEU, 3 },
	{ Common::ES_ESP, 4 },
	{ Common::IT_ITA, 5 },
	{ Common::JA_JPN, 6 },
	{ Common::RU_RUS, 7 },
	{ -1, -1 }
};

byte getLanguageID(const GameFlags &flags) {
	return Common::find(iLanguageTable, ARRAYEND(iLanguageTable) - 1, flags.lang)->value;
}

const IndexTable iPlatformTable[] = {
	{ Common::kPlatformPC, 0 },
	{ Common::kPlatformAmiga, 1 },
	{ Common::kPlatformFMTowns, 2 },
	{ Common::kPlatformPC98, 3 },
	{ Common::kPlatformMacintosh, 0 }, // HACK: Should be type "4", but as long as we can't extract Macintosh data, we need to use DOS data.
	{ -1, -1 }
};

byte getPlatformID(const GameFlags &flags) {
	return Common::find(iPlatformTable, ARRAYEND(iPlatformTable) - 1, flags.platform)->value;
}

byte getSpecialID(const GameFlags &flags) {
	if (flags.isOldFloppy)
		return 4;
	else if (flags.isDemo && flags.isTalkie)
		return 3;
	else if (flags.isDemo)
		return 2;
	else if (flags.isTalkie)
		return 1;
	else
		return 0;
}

} // end of anonymous namespace

bool StaticResource::loadStaticResourceFile() {
	Resource *res = _vm->resource();

	if (res->isInCacheList(staticDataFilename()))
		return true;

	Common::ArchiveMemberList kyraDatFiles;
	res->listFiles(staticDataFilename(), kyraDatFiles);

	bool foundWorkingKyraDat = false;
	for (Common::ArchiveMemberList::iterator i = kyraDatFiles.begin(); i != kyraDatFiles.end(); ++i) {
		Common::SeekableReadStream *file = (*i)->createReadStream();
		if (!checkKyraDat(file)) {
			delete file;
			continue;
		}

		delete file; file = 0;

		if (!res->loadPakFile(staticDataFilename(), *i))
			continue;

		if (tryKyraDatLoad()) {
			foundWorkingKyraDat = true;
			break;
		}

		res->unloadPakFile(staticDataFilename(), true);
		unloadId(-1);
	}

	if (!foundWorkingKyraDat) {
		Common::String errorMessage = "You're missing the '" + StaticResource::staticDataFilename() + "' file or it got corrupted, (re)get it from the ScummVM website";
		GUIErrorMessage(errorMessage);
		error("%s", errorMessage.c_str());
	}

	return true;
}

bool StaticResource::tryKyraDatLoad() {
	Common::SeekableReadStream *index = _vm->resource()->createReadStream("INDEX");
	if (!index)
		return false;

	const uint32 version = index->readUint32BE();

	if (version != RESFILE_VERSION) {
		delete index;
		return false;
	}

	const uint32 includedGames = index->readUint32BE();

	if (includedGames * 2 + 8 != (uint32)index->size()) {
		delete index;
		return false;
	}

	const GameFlags &flags = _vm->gameFlags();
	const byte game = getGameID(flags) & 0xF;
	const byte platform = getPlatformID(flags) & 0xF;
	const byte special = getSpecialID(flags) & 0xF;
	const byte lang = getLanguageID(flags) & 0xF;

	const uint16 gameDef = (game << 12) | (platform << 8) | (special << 4) | (lang << 0);

	bool found = false;
	for (uint32 i = 0; i < includedGames; ++i) {
		if (index->readUint16BE() == gameDef) {
			found = true;
			break;
		}
	}

	delete index;
	index = 0;

	if (!found)
		return false;

	// load the ID map for our game
	const Common::String filenamePattern = Common::String::format("0%01X%01X%01X000%01X", game, platform, special, lang);
	Common::SeekableReadStream *idMap = _vm->resource()->createReadStream(filenamePattern);
	if (!idMap)
		return false;

	uint16 numIDs = idMap->readUint16BE();
	while (numIDs--) {
		uint16 id = idMap->readUint16BE();
		uint8 type = idMap->readByte();
		uint32 filename = idMap->readUint32BE();

		_dataTable[id] = DataDescriptor(filename, type);
	}

	const bool fileError = idMap->err();
	delete idMap;
	if (fileError)
		return false;

	// load all tables for now
	if (!prefetchId(-1))
		return false;

	return true;
}

bool StaticResource::init() {
#define proc(x) &StaticResource::x
	static const FileType fileTypeTable[] = {
		{ kStringList, proc(loadStringTable), proc(freeStringTable) },
		{ StaticResource::kRoomList, proc(loadRoomTable), proc(freeRoomTable) },
		{ kShapeList, proc(loadShapeTable), proc(freeShapeTable) },
		{ kAmigaSfxTable, proc(loadAmigaSfxTable), proc(freeAmigaSfxTable) },
		{ kRawData, proc(loadRawData), proc(freeRawData) },

		{ k2SeqData, proc(loadHoFSequenceData), proc(freeHoFSequenceData) },
		{ k2SeqItemAnimData, proc(loadHoFSeqItemAnimData), proc(freeHoFSeqItemAnimData) },
		{ k2ItemAnimDefinition, proc(loadItemAnimDefinition), proc(freeItemAnimDefinition) },

#ifdef ENABLE_LOL
		{ kLoLCharData, proc(loadCharData), proc(freeCharData) },
		{ kLoLSpellData, proc(loadSpellData), proc(freeSpellData) },
		{ kLoLCompassData, proc(loadCompassData), proc(freeCompassData) },
		{ kLoLFlightShpData, proc(loadFlyingObjectData), proc(freeFlyingObjectData) },
#else
		{ kLoLCharData, proc(loadDummy), proc(freeDummy) },
		{ kLoLSpellData, proc(loadDummy), proc(freeDummy) },
		{ kLoLCompassData, proc(loadDummy), proc(freeDummy) },
		{ kLoLFlightShpData, proc(loadDummy), proc(freeDummy) },
#endif
#if defined(ENABLE_EOB) || defined(ENABLE_LOL)
		{ kRawDataBe16, proc(loadRawDataBe16), proc(freeRawDataBe16) },
		{ kRawDataBe32, proc(loadRawDataBe32), proc(freeRawDataBe32) },
#endif
#ifdef ENABLE_LOL
		{ kLoLButtonData, proc(loadButtonDefs), proc(freeButtonDefs) },
#else
		{ kLoLButtonData, proc(loadDummy), proc(freeDummy) },
#endif

#ifdef ENABLE_EOB
		{ kEoB2SequenceData, proc(loadEoB2SeqData), proc(freeEoB2SeqData) },
		{ kEoB2ShapeData, proc(loadEoB2ShapeData), proc(freeEoB2ShapeData) },
		{ kEoBNpcData, proc(loadEoBNpcData), proc(freeEoBNpcData) },
#endif

		{ 0, 0, 0 }
	};
#undef proc
	_fileLoader = fileTypeTable;

	return loadStaticResourceFile();
}

void StaticResource::deinit() {
	unloadId(-1);
}

const char *const *StaticResource::loadStrings(int id, int &strings) {
	return (const char *const *)getData(id, kStringList, strings);
}

const uint8 *StaticResource::loadRawData(int id, int &size) {
	return (const uint8 *)getData(id, kRawData, size);
}

const Shape *StaticResource::loadShapeTable(int id, int &entries) {
	return (const Shape *)getData(id, kShapeList, entries);
}

const AmigaSfxTable *StaticResource::loadAmigaSfxTable(int id, int &entries) {
	return (const AmigaSfxTable *)getData(id, kAmigaSfxTable, entries);
}

const Room *StaticResource::loadRoomTable(int id, int &entries) {
	return (const Room *)getData(id, StaticResource::kRoomList, entries);
}

const HoFSeqData *StaticResource::loadHoFSequenceData(int id, int &entries) {
	return (const HoFSeqData *)getData(id, k2SeqData, entries);
}

const HoFSeqItemAnimData *StaticResource::loadHoFSeqItemAnimData(int id, int &entries) {
	return (const HoFSeqItemAnimData *)getData(id, k2SeqItemAnimData, entries);
}

const ItemAnimDefinition *StaticResource::loadItemAnimDefinition(int id, int &entries) {
	return (const ItemAnimDefinition *)getData(id, k2ItemAnimDefinition, entries);
}

bool StaticResource::prefetchId(int id) {
	if (id == -1) {
		for (DataMap::const_iterator i = _dataTable.begin(); i != _dataTable.end(); ++i) {
			if (!prefetchId(i->_key))
				return false;
		}
		return true;
	}

	const void *ptr = 0;
	int type = -1, size = -1;

	if (checkResList(id, type, ptr, size))
		return true;

	DataMap::const_iterator dDesc = _dataTable.find(id);
	if (dDesc == _dataTable.end())
		return false;

	const FileType *filetype = getFiletype(dDesc->_value.type);
	if (!filetype)
		return false;

	ResData data;
	data.id = id;
	data.type = dDesc->_value.type;
	Common::SeekableReadStream *fileStream = _vm->resource()->createReadStream(Common::String::format("%08X", dDesc->_value.filename));
	if (!fileStream)
		return false;

	if (!(this->*(filetype->load))(*fileStream, data.data, data.size)) {
		delete fileStream;
		return false;
	}
	delete fileStream;
	_resList.push_back(data);

	return true;
}

void StaticResource::unloadId(int id) {
	Common::List<ResData>::iterator pos = _resList.begin();
	for (; pos != _resList.end();) {
		if (pos->id == id || id == -1) {
			const FileType *filetype = getFiletype(pos->type);
			(this->*(filetype->free))(pos->data, pos->size);
			pos = _resList.erase(pos);
			if (id != -1)
				break;
		} else {
			++pos;
		}
	}
}

bool StaticResource::checkResList(int id, int &type, const void *&ptr, int &size) {
	Common::List<ResData>::iterator pos = _resList.begin();
	for (; pos != _resList.end(); ++pos) {
		if (pos->id == id) {
			size = pos->size;
			type = pos->type;
			ptr = pos->data;
			return true;
		}
	}
	return false;
}

const StaticResource::FileType *StaticResource::getFiletype(int type) {
	if (!_fileLoader)
		return 0;

	for (int i = 0; _fileLoader[i].load; ++i) {
		if (_fileLoader[i].type == type)
			return &_fileLoader[i];
	}

	return 0;
}

const void *StaticResource::getData(int id, int requesttype, int &size) {
	const void *ptr = 0;
	int type = -1;
	size = 0;

	if (checkResList(id, type, ptr, size)) {
		if (type == requesttype)
			return ptr;
		return 0;
	}

	if (!prefetchId(id))
		return 0;

	if (checkResList(id, type, ptr, size)) {
		if (type == requesttype)
			return ptr;
	}

	return 0;
}

bool StaticResource::loadDummy(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	return true;
}

bool StaticResource::loadStringTable(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	uint32 count = stream.readUint32BE();
	size = count;
	char **output = new char *[count];
	assert(output);

	for (uint32 i = 0; i < count; ++i) {
		Common::String string;
		char c = 0;
		while ((c = (char)stream.readByte()) != 0)
			string += c;

		output[i] = new char[string.size() + 1];
		strcpy(output[i], string.c_str());
	}

	ptr = output;
	return true;
}

bool StaticResource::loadRawData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	ptr = new uint8[stream.size()];
	stream.read(ptr, stream.size());
	size = stream.size();
	return true;
}

bool StaticResource::loadShapeTable(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	uint32 count = stream.readUint32BE();
	size = count;
	Shape *loadTo = new Shape[count];
	assert(loadTo);

	for (uint32 i = 0; i < count; ++i) {
		loadTo[i].imageIndex = stream.readByte();
		loadTo[i].x = stream.readByte();
		loadTo[i].y = stream.readByte();
		loadTo[i].w = stream.readByte();
		loadTo[i].h = stream.readByte();
		loadTo[i].xOffset = stream.readSByte();
		loadTo[i].yOffset = stream.readSByte();
	}

	ptr = loadTo;
	return true;
}

bool StaticResource::loadAmigaSfxTable(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.readUint32BE();
	AmigaSfxTable *loadTo = new AmigaSfxTable[size];
	assert(loadTo);

	for (int i = 0; i < size; ++i) {
		loadTo[i].note = stream.readByte();
		loadTo[i].patch = stream.readByte();
		loadTo[i].duration = stream.readUint16BE();
		loadTo[i].volume = stream.readByte();
		loadTo[i].pan = stream.readByte();
	}

	ptr = loadTo;
	return true;
}

bool StaticResource::loadRoomTable(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	uint32 count = stream.readUint32BE();
	size = count;
	Room *loadTo = new Room[count];
	assert(loadTo);

	for (uint32 i = 0; i < count; ++i) {
		loadTo[i].nameIndex = stream.readByte();
		loadTo[i].northExit = stream.readUint16BE();
		loadTo[i].eastExit = stream.readUint16BE();
		loadTo[i].southExit = stream.readUint16BE();
		loadTo[i].westExit = stream.readUint16BE();
		memset(&loadTo[i].itemsTable[0], 0xFF, sizeof(byte) * 6);
		memset(&loadTo[i].itemsTable[6], 0, sizeof(byte) * 6);
		memset(loadTo[i].itemsXPos, 0, sizeof(uint16) * 12);
		memset(loadTo[i].itemsYPos, 0, sizeof(uint8) * 12);
		memset(loadTo[i].needInit, 0, sizeof(loadTo[i].needInit));
	}

	ptr = loadTo;
	return true;
}

bool StaticResource::loadHoFSequenceData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	int numSeq = stream.readUint16BE();
	uint32 offset = 2;
	HoFSequence *tmp_s = new HoFSequence[numSeq];

	size = sizeof(HoFSeqData) + numSeq * (sizeof(HoFSequence) + 28);

	for (int i = 0; i < numSeq; i++) {
		stream.seek(offset, SEEK_SET); offset += 2;
		stream.seek(stream.readUint16BE(), SEEK_SET);

		tmp_s[i].flags = stream.readUint16BE();
		tmp_s[i].wsaFile = new char[14];
		stream.read(const_cast<char *>(tmp_s[i].wsaFile), 14);
		tmp_s[i].cpsFile = new char[14];
		stream.read(const_cast<char *>(tmp_s[i].cpsFile), 14);
		tmp_s[i].fadeInTransitionType = stream.readByte();
		tmp_s[i].fadeOutTransitionType = stream.readByte();
		tmp_s[i].stringIndex1 = stream.readUint16BE();
		tmp_s[i].stringIndex2 = stream.readUint16BE();
		tmp_s[i].startFrame = stream.readUint16BE();
		tmp_s[i].numFrames = stream.readUint16BE();
		tmp_s[i].duration = stream.readUint16BE();
		tmp_s[i].xPos = stream.readUint16BE();
		tmp_s[i].yPos = stream.readUint16BE();
		tmp_s[i].timeout = stream.readUint16BE();
	}

	stream.seek(offset, SEEK_SET); offset += 2;
	int numSeqN = stream.readUint16BE();
	HoFNestedSequence *tmp_n = new HoFNestedSequence[numSeqN];
	size += (numSeqN * (sizeof(HoFNestedSequence) + 14));

	for (int i = 0; i < numSeqN; i++) {
		stream.seek(offset, SEEK_SET); offset += 2;
		stream.seek(stream.readUint16BE(), SEEK_SET);

		tmp_n[i].flags = stream.readUint16BE();
		tmp_n[i].wsaFile = new char[14];
		stream.read(const_cast<char *>(tmp_n[i].wsaFile), 14);
		tmp_n[i].startframe = stream.readUint16BE();
		tmp_n[i].endFrame = stream.readUint16BE();
		tmp_n[i].frameDelay = stream.readUint16BE();
		tmp_n[i].x = stream.readUint16BE();
		tmp_n[i].y = stream.readUint16BE();
		uint16 ctrlOffs = stream.readUint16BE();
		tmp_n[i].fadeInTransitionType = stream.readUint16BE();
		tmp_n[i].fadeOutTransitionType = stream.readUint16BE();

		if (ctrlOffs) {
			stream.seek(ctrlOffs, SEEK_SET);
			int num_c = stream.readByte();
			FrameControl *tmp_f = new FrameControl[num_c];

			for (int ii = 0; ii < num_c; ii++) {
				tmp_f[ii].index = stream.readUint16BE();
				tmp_f[ii].delay = stream.readUint16BE();
			}

			tmp_n[i].wsaControl = (const FrameControl *)tmp_f;
			size += (num_c * sizeof(FrameControl));

		} else {
			tmp_n[i].wsaControl = 0;
		}
	}

	HoFSeqData *loadTo = new HoFSeqData;
	assert(loadTo);

	loadTo->seq = tmp_s;
	loadTo->nestedSeq = tmp_n;
	loadTo->numSeq = numSeq;
	loadTo->numNestedSeq = numSeqN;

	ptr = loadTo;
	return true;
}

bool StaticResource::loadHoFSeqItemAnimData(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.readByte();
	HoFSeqItemAnimData *loadTo = new HoFSeqItemAnimData[size];
	assert(loadTo);

	for (int i = 0; i < size; i++) {
		loadTo[i].itemIndex = stream.readSint16BE();
		loadTo[i].y = stream.readUint16BE();
		uint16 *tmp_f = new uint16[20];
		for (int ii = 0; ii < 20; ii++)
			tmp_f[ii] = stream.readUint16BE();
		loadTo[i].frames = tmp_f;
	}

	ptr = loadTo;
	return true;
}

bool StaticResource::loadItemAnimDefinition(Common::SeekableReadStream &stream, void *&ptr, int &size) {
	size = stream.readByte();
	ItemAnimDefinition *loadTo = new ItemAnimDefinition[size];
	assert(loadTo);

	for (int i = 0; i < size; i++) {
		loadTo[i].itemIndex = stream.readSint16BE();
		loadTo[i].numFrames = stream.readByte();
		FrameControl *tmp_f = new FrameControl[loadTo[i].numFrames];
		for (int ii = 0; ii < loadTo[i].numFrames; ii++) {
			tmp_f[ii].index = stream.readUint16BE();
			tmp_f[ii].delay = stream.readUint16BE();
		}
		loadTo[i].frames = tmp_f;
	}

	ptr = loadTo;
	return true;
}

void StaticResource::freeDummy(void *&ptr, int &size) {
}

void StaticResource::freeRawData(void *&ptr, int &size) {
	uint8 *data = (uint8 *)ptr;
	delete[] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeStringTable(void *&ptr, int &size) {
	char **data = (char **)ptr;
	while (size--)
		delete[] data[size];
	delete[] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeShapeTable(void *&ptr, int &size) {
	Shape *data = (Shape *)ptr;
	delete[] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeAmigaSfxTable(void *&ptr, int &size) {
	AmigaSfxTable *data = (AmigaSfxTable *)ptr;
	delete[] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeRoomTable(void *&ptr, int &size) {
	Room *data = (Room *)ptr;
	delete[] data;
	ptr = 0;
	size = 0;
}

void StaticResource::freeHoFSequenceData(void *&ptr, int &size) {
	HoFSeqData *h = (HoFSeqData *)ptr;

	for (int i = 0; i < h->numSeq; i++) {
		delete[] h->seq[i].wsaFile;
		delete[] h->seq[i].cpsFile;
	}
	delete[] h->seq;

	for (int i = 0; i < h->numNestedSeq; i++) {
		delete[] h->nestedSeq[i].wsaFile;
		delete[] h->nestedSeq[i].wsaControl;
	}
	delete[] h->nestedSeq;

	delete h;
	ptr = 0;
	size = 0;
}

void StaticResource::freeHoFSeqItemAnimData(void *&ptr, int &size) {
	HoFSeqItemAnimData *d = (HoFSeqItemAnimData *)ptr;
	for (int i = 0; i < size; i++)
		delete[] d[i].frames;
	delete[] d;
	ptr = 0;
	size = 0;
}

void StaticResource::freeItemAnimDefinition(void *&ptr, int &size) {
	ItemAnimDefinition *d = (ItemAnimDefinition *)ptr;
	for (int i = 0; i < size; i++)
		delete[] d[i].frames;
	delete[] d;
	ptr = 0;
	size = 0;
}

#pragma mark -

void KyraEngine_LoK::initStaticResource() {
	int temp = 0;
	_seq_Forest = _staticres->loadRawData(k1ForestSeq, temp);
	_seq_KallakWriting = _staticres->loadRawData(k1KallakWritingSeq, temp);
	_seq_KyrandiaLogo = _staticres->loadRawData(k1KyrandiaLogoSeq, temp);
	_seq_KallakMalcolm = _staticres->loadRawData(k1KallakMalcolmSeq, temp);
	_seq_MalcolmTree = _staticres->loadRawData(k1MalcolmTreeSeq, temp);
	_seq_WestwoodLogo = _staticres->loadRawData(k1WestwoodLogoSeq, temp);
	_seq_Demo1 = _staticres->loadRawData(k1Demo1Seq, temp);
	_seq_Demo2 = _staticres->loadRawData(k1Demo2Seq, temp);
	_seq_Demo3 = _staticres->loadRawData(k1Demo3Seq, temp);
	_seq_Demo4 = _staticres->loadRawData(k1Demo4Seq, temp);
	_seq_Reunion = _staticres->loadRawData(k1OutroReunionSeq, temp);

	_seq_WSATable = _staticres->loadStrings(k1IntroWSAStrings, _seq_WSATable_Size);
	_seq_CPSTable = _staticres->loadStrings(k1IntroCPSStrings, _seq_CPSTable_Size);
	_seq_COLTable = _staticres->loadStrings(k1IntroCOLStrings, _seq_COLTable_Size);
	_seq_textsTable = _staticres->loadStrings(k1IntroStrings, _seq_textsTable_Size);

	_itemList = _staticres->loadStrings(k1ItemNames, _itemList_Size);
	_takenList = _staticres->loadStrings(k1TakenStrings, _takenList_Size);
	_placedList = _staticres->loadStrings(k1PlacedStrings, _placedList_Size);
	_droppedList = _staticres->loadStrings(k1DroppedStrings, _droppedList_Size);
	_noDropList = _staticres->loadStrings(k1NoDropStrings, _noDropList_Size);
	_putDownFirst = _staticres->loadStrings(k1PutDownString, _putDownFirst_Size);
	_waitForAmulet = _staticres->loadStrings(k1WaitAmuletString, _waitForAmulet_Size);
	_blackJewel = _staticres->loadStrings(k1BlackJewelString, _blackJewel_Size);
	_poisonGone = _staticres->loadStrings(k1PoisonGoneString, _poisonGone_Size);
	_healingTip = _staticres->loadStrings(k1HealingTipString, _healingTip_Size);
	_thePoison = _staticres->loadStrings(k1ThePoisonStrings, _thePoison_Size);
	_fluteString = _staticres->loadStrings(k1FluteStrings, _fluteString_Size);
	_wispJewelStrings = _staticres->loadStrings(k1WispJewelStrings, _wispJewelStrings_Size);
	_magicJewelString = _staticres->loadStrings(k1MagicJewelStrings, _magicJewelString_Size);
	_flaskFull = _staticres->loadStrings(k1FlaskFullString, _flaskFull_Size);
	_fullFlask = _staticres->loadStrings(k1FullFlaskString, _fullFlask_Size);
	_veryClever = _staticres->loadStrings(k1VeryCleverString, _veryClever_Size);
	_homeString = _staticres->loadStrings(k1OutroHomeString, _homeString_Size);
	_newGameString = _staticres->loadStrings(k1NewGameString, _newGameString_Size);

	_healingShapeTable = _staticres->loadShapeTable(k1Healing1Shapes, _healingShapeTableSize);
	_healingShape2Table = _staticres->loadShapeTable(k1Healing2Shapes, _healingShape2TableSize);
	_posionDeathShapeTable = _staticres->loadShapeTable(k1PoisonDeathShapes, _posionDeathShapeTableSize);
	_fluteAnimShapeTable = _staticres->loadShapeTable(k1FluteShapes, _fluteAnimShapeTableSize);
	_winterScrollTable = _staticres->loadShapeTable(k1Winter1Shapes, _winterScrollTableSize);
	_winterScroll1Table = _staticres->loadShapeTable(k1Winter2Shapes, _winterScroll1TableSize);
	_winterScroll2Table = _staticres->loadShapeTable(k1Winter3Shapes, _winterScroll2TableSize);
	_drinkAnimationTable = _staticres->loadShapeTable(k1DrinkShapes, _drinkAnimationTableSize);
	_brandonToWispTable = _staticres->loadShapeTable(k1WispShapes, _brandonToWispTableSize);
	_magicAnimationTable = _staticres->loadShapeTable(k1MagicAnimShapes, _magicAnimationTableSize);
	_brandonStoneTable = _staticres->loadShapeTable(k1BranStoneShapes, _brandonStoneTableSize);

	_characterImageTable = _staticres->loadStrings(k1CharacterImageFilenames, _characterImageTableSize);

	_roomFilenameTable = _staticres->loadStrings(k1RoomFilenames, _roomFilenameTableSize);

	_amuleteAnim = _staticres->loadRawData(k1AmuleteAnimSeq, temp);

	const uint8 **palTable = new const uint8 *[k1SpecialPalette33 - k1SpecialPalette1 + 1];
	for (int i = k1SpecialPalette1; i <= k1SpecialPalette33; ++i)
		palTable[i - k1SpecialPalette1] = _staticres->loadRawData(i, temp);
	_specialPalettes = palTable;

	_guiStrings = _staticres->loadStrings(k1GUIStrings, _guiStringsSize);
	_configStrings = _staticres->loadStrings(k1ConfigStrings, _configStringsSize);

	_storyStrings = _staticres->loadStrings(k1PC98StoryStrings, _storyStringsSize);

	// room list
	const Room *tempRoomList = _staticres->loadRoomTable(k1RoomList, _roomTableSize);

	if (_roomTableSize > 0) {
		_roomTable = new Room[_roomTableSize];
		assert(_roomTable);

		memcpy(_roomTable, tempRoomList, _roomTableSize * sizeof(Room));
		tempRoomList = 0;

		_staticres->unloadId(k1RoomList);
	}

	// default shape table
	const Shape *tempShapeTable = _staticres->loadShapeTable(k1DefaultShapes, _defaultShapeTableSize);

	if (_defaultShapeTableSize > 0) {
		_defaultShapeTable = new Shape[_defaultShapeTableSize];
		assert(_defaultShapeTable);

		memcpy(_defaultShapeTable, tempShapeTable, _defaultShapeTableSize * sizeof(Shape));
		tempShapeTable = 0;

		_staticres->unloadId(k1DefaultShapes);
	}

	// audio resource assignment
	int size1, size2;
	const char *const *soundfiles1 = _staticres->loadStrings(k1AudioTracks, size1);
	const char *const *soundfiles2 = _staticres->loadStrings(k1AudioTracks2, size2);
	int soundFilesSize = size1 + size2;
	int soundFilesIntroSize = 0;
	int cdaTableSize = 0;
	const char **soundFiles = 0;

	if (soundFilesSize) {
		soundFiles = new const char*[soundFilesSize];
		for (int i = 0; i < soundFilesSize; i++)
			soundFiles[i] = (i < size1) ? soundfiles1[i] : soundfiles2[i - size1];
	}
	const char *const *soundFilesIntro = _staticres->loadStrings(k1AudioTracksIntro, temp);
	const int32 *cdaTable = (const int32 *)_staticres->loadRawData(k1TownsCDATable, cdaTableSize);

	// FIXME: It seems Kyra1 MAC CD includes AdLib and MIDI music and sfx, thus we enable
	// support for those for now. (Based on patch #2767489 "Support for Mac Kyrandia 1 CD" by satz).
	if (_flags.platform == Common::kPlatformPC || _flags.platform == Common::kPlatformMacintosh) {
		SoundResourceInfo_PC resInfoIntro(soundFilesIntro, soundFilesIntroSize);
		SoundResourceInfo_PC resInfoIngame(soundFiles, soundFilesSize);
		_sound->initAudioResourceInfo(kMusicIntro, &resInfoIntro);
		_sound->initAudioResourceInfo(kMusicIngame, &resInfoIngame);
	} else if (_flags.platform == Common::kPlatformFMTowns) {
		SoundResourceInfo_Towns resInfoIntro(soundFiles, soundFilesSize, cdaTable, cdaTableSize);
		SoundResourceInfo_Towns resInfoIngame(soundFiles, soundFilesSize, cdaTable, cdaTableSize);
		_sound->initAudioResourceInfo(kMusicIntro, &resInfoIntro);
		_sound->initAudioResourceInfo(kMusicIngame, &resInfoIngame);
	} else if (_flags.platform == Common::kPlatformPC98) {
		SoundResourceInfo_PC98 resInfoIntro("INTRO%d.DAT");
		SoundResourceInfo_PC98 resInfoIngame("KYRAM%d.DAT");
		_sound->initAudioResourceInfo(kMusicIntro, &resInfoIntro);
		_sound->initAudioResourceInfo(kMusicIngame, &resInfoIngame);
	}
}

void KyraEngine_LoK::loadMouseShapes() {
	_screen->loadBitmap("MOUSE.CPS", 3, 3, 0);
	_screen->_curPage = 2;
	_shapes[0] = _screen->encodeShape(0, 0, 8, 10, 0);
	_shapes[1] = _screen->encodeShape(0, 0x17, 0x20, 7, 0);
	_shapes[2] = _screen->encodeShape(0x50, 0x12, 0x10, 9, 0);
	_shapes[3] = _screen->encodeShape(0x60, 0x12, 0x10, 11, 0);
	_shapes[4] = _screen->encodeShape(0x70, 0x12, 0x10, 9, 0);
	_shapes[5] = _screen->encodeShape(0x80, 0x12, 0x10, 11, 0);
	_shapes[6] = _screen->encodeShape(0x90, 0x12, 0x10, 10, 0);
	_shapes[360] = _screen->encodeShape(0x28, 0, 0x10, 13, 0);
	_screen->setMouseCursor(1, 1, 0);
	_screen->setMouseCursor(1, 1, _shapes[0]);
	_screen->setShapePages(5, 3);
}

void KyraEngine_LoK::loadCharacterShapes() {
	int curImage = 0xFF;
	int videoPage = _screen->_curPage;
	_screen->_curPage = 2;
	for (int i = 0; i < 115; ++i) {
		assert(i < _defaultShapeTableSize);
		Shape *shape = &_defaultShapeTable[i];
		if (shape->imageIndex == 0xFF) {
			_shapes[i + 7] = 0;
			continue;
		}
		if (shape->imageIndex != curImage) {
			assert(shape->imageIndex < _characterImageTableSize);
			_screen->loadBitmap(_characterImageTable[shape->imageIndex], 3, 3, 0);
			curImage = shape->imageIndex;
		}
		_shapes[i + 7] = _screen->encodeShape(shape->x << 3, shape->y, shape->w << 3, shape->h, 1);
	}
	_screen->_curPage = videoPage;
}

void KyraEngine_LoK::loadSpecialEffectShapes() {
	_screen->loadBitmap("EFFECTS.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	int currShape;
	for (currShape = 173; currShape < 183; currShape++)
		_shapes[currShape] = _screen->encodeShape((currShape - 173) * 24, 0, 24, 24, 1);

	for (currShape = 183; currShape < 190; currShape++)
		_shapes[currShape] = _screen->encodeShape((currShape - 183) * 24, 24, 24, 24, 1);

	for (currShape = 190; currShape < 201; currShape++)
		_shapes[currShape] = _screen->encodeShape((currShape - 190) * 24, 48, 24, 24, 1);

	for (currShape = 201; currShape < 206; currShape++)
		_shapes[currShape] = _screen->encodeShape((currShape - 201) * 16, 106, 16, 16, 1);
}

void KyraEngine_LoK::loadItems() {
	int shape;

	_screen->loadBitmap("JEWELS3.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	_shapes[323] = 0;

	for (shape = 1; shape < 6; shape++)
		_shapes[323 + shape] = _screen->encodeShape((shape - 1) * 32, 0, 32, 17, 0);

	for (shape = 330; shape <= 334; shape++)
		_shapes[shape] = _screen->encodeShape((shape - 330) * 32, 102, 32, 17, 0);

	for (shape = 335; shape <= 339; shape++)
		_shapes[shape] = _screen->encodeShape((shape - 335) * 32, 17,  32, 17, 0);

	for (shape = 340; shape <= 344; shape++)
		_shapes[shape] = _screen->encodeShape((shape - 340) * 32, 34,  32, 17, 0);

	for (shape = 345; shape <= 349; shape++)
		_shapes[shape] = _screen->encodeShape((shape - 345) * 32, 51,  32, 17, 0);

	for (shape = 350; shape <= 354; shape++)
		_shapes[shape] = _screen->encodeShape((shape - 350) * 32, 68,  32, 17, 0);

	for (shape = 355; shape <= 359; shape++)
		_shapes[shape] = _screen->encodeShape((shape - 355) * 32, 85,  32, 17, 0);


	_screen->loadBitmap("ITEMS.CPS", 3, 3, 0);
	_screen->_curPage = 2;

	for (int i = 0; i < 107; i++) {
		shape = findDuplicateItemShape(i);

		if (shape != -1)
			_shapes[216 + i] = _shapes[216 + shape];
		else
			_shapes[216 + i] = _screen->encodeShape((i % 20) * 16, i / 20 * 16, 16, 16, 0);
	}

	_res->loadFileToBuf("_ITEM_HT.DAT", &_itemHtDat, sizeof(_itemHtDat));
}

void KyraEngine_LoK::loadButtonShapes() {
	_screen->loadBitmap("BUTTONS2.CPS", 3, 3, 0);
	_screen->_curPage = 2;
	_gui->_scrollUpButton.data0ShapePtr = _screen->encodeShape(0, 0, 24, 14, 1);
	_gui->_scrollUpButton.data1ShapePtr = _screen->encodeShape(24, 0, 24, 14, 1);
	_gui->_scrollUpButton.data2ShapePtr = _screen->encodeShape(48, 0, 24, 14, 1);
	_gui->_scrollDownButton.data0ShapePtr = _screen->encodeShape(0, 15, 24, 14, 1);
	_gui->_scrollDownButton.data1ShapePtr = _screen->encodeShape(24, 15, 24, 14, 1);
	_gui->_scrollDownButton.data2ShapePtr = _screen->encodeShape(48, 15, 24, 14, 1);
	_screen->_curPage = 0;
}

void KyraEngine_LoK::loadMainScreen(int page) {
	_screen->clearPage(page);

	if (((_flags.lang == Common::EN_ANY || _flags.lang == Common::RU_RUS) && !_flags.isTalkie && _flags.platform == Common::kPlatformPC) || _flags.platform == Common::kPlatformAmiga)
		_screen->loadBitmap("MAIN15.CPS", page, page, &_screen->getPalette(0));
	else if (_flags.lang == Common::EN_ANY || _flags.lang == Common::JA_JPN || (_flags.isTalkie && _flags.lang == Common::IT_ITA))
		_screen->loadBitmap("MAIN_ENG.CPS", page, page, 0);
	else if (_flags.lang == Common::FR_FRA)
		_screen->loadBitmap("MAIN_FRE.CPS", page, page, 0);
	else if (_flags.lang == Common::DE_DEU)
		_screen->loadBitmap("MAIN_GER.CPS", page, page, 0);
	else if (_flags.lang == Common::ES_ESP)
		_screen->loadBitmap("MAIN_SPA.CPS", page, page, 0);
	else if (_flags.lang == Common::IT_ITA)
		_screen->loadBitmap("MAIN_ITA.CPS", page, page, 0);
	else
		warning("no main graphics file found");

	_screen->copyRegion(0, 0, 0, 0, 320, 200, page, 0, Screen::CR_NO_P_CHECK);

	if (_flags.platform == Common::kPlatformAmiga) {
		_screen->copyPalette(1, 0);
		_screen->setInterfacePalette(_screen->getPalette(1), 0x3F, 0x3F, 0x3F);

		// TODO: Move this to a better place
		_screen->enableInterfacePalette(true);
	}
}

void KyraEngine_HoF::initStaticResource() {
	_ingamePakList = _staticres->loadStrings(k2IngamePakFiles, _ingamePakListSize);
	_ingameSoundList = _staticres->loadStrings(k2IngameSfxFiles, _ingameSoundListSize);
	_ingameSoundIndex = (const uint16 *)_staticres->loadRawData(k2IngameSfxIndex, _ingameSoundIndexSize);
	_musicFileListIntro = _staticres->loadStrings(k2SeqplayIntroTracks, _musicFileListIntroSize);
	_musicFileListIngame = _staticres->loadStrings(k2IngameTracks, _musicFileListIngameSize);
	_musicFileListFinale = _staticres->loadStrings(k2SeqplayFinaleTracks, _musicFileListFinaleSize);
	_cdaTrackTableIntro = _staticres->loadRawData(k2SeqplayIntroCDA, _cdaTrackTableIntroSize);
	_cdaTrackTableIngame = _staticres->loadRawData(k2IngameCDA, _cdaTrackTableIngameSize);
	_cdaTrackTableFinale = _staticres->loadRawData(k2SeqplayFinaleCDA, _cdaTrackTableFinaleSize);
	_ingameTalkObjIndex = (const uint16 *)_staticres->loadRawData(k2IngameTalkObjIndex, _ingameTalkObjIndexSize);
	_ingameTimJpStr = _staticres->loadStrings(k2IngameTimJpStrings, _ingameTimJpStrSize);
	_itemAnimDefinition = _staticres->loadItemAnimDefinition(k2IngameShapeAnimData, _itemAnimDefinitionSize);

	// assign music data
	if (_flags.platform == Common::kPlatformPC) {
		SoundResourceInfo_PC resInfoIntro(_musicFileListIntro, _musicFileListIntroSize);
		SoundResourceInfo_PC resInfoIngame(_musicFileListIngame, _musicFileListIngameSize);
		SoundResourceInfo_PC resInfoFinale(_musicFileListFinale, _musicFileListFinaleSize);
		_sound->initAudioResourceInfo(kMusicIntro, &resInfoIntro);
		_sound->initAudioResourceInfo(kMusicIngame, &resInfoIngame);
		_sound->initAudioResourceInfo(kMusicFinale, &resInfoFinale);
	} else if (_flags.platform == Common::kPlatformFMTowns) {
		SoundResourceInfo_TownsPC98V2 resInfoIntro(0, 0, "intro%d.twn", (const uint16*)_cdaTrackTableIntro, _cdaTrackTableIntroSize >> 1);
		SoundResourceInfo_TownsPC98V2 resInfoIngame(0, 0, "km%02d.twn", (const uint16*)_cdaTrackTableIngame, _cdaTrackTableIngameSize >> 1);
		SoundResourceInfo_TownsPC98V2 resInfoFinale(0, 0, "finale%d.twn", (const uint16*)_cdaTrackTableFinale, _cdaTrackTableFinaleSize >> 1);
		_sound->initAudioResourceInfo(kMusicIntro, &resInfoIntro);
		_sound->initAudioResourceInfo(kMusicIngame, &resInfoIngame);
		_sound->initAudioResourceInfo(kMusicFinale, &resInfoFinale);
	} else if (_flags.platform == Common::kPlatformPC98) {
		SoundResourceInfo_TownsPC98V2 resInfoIntro(0, 0, "intro%d.86", 0, 0);
		SoundResourceInfo_TownsPC98V2 resInfoIngame(0, 0, "km%02d.86", 0, 0);
		SoundResourceInfo_TownsPC98V2 resInfoFinale(0, 0, "finale%d.86", 0, 0);
		_sound->initAudioResourceInfo(kMusicIntro, &resInfoIntro);
		_sound->initAudioResourceInfo(kMusicIngame, &resInfoIngame);
		_sound->initAudioResourceInfo(kMusicFinale, &resInfoFinale);
	}
}

void KyraEngine_MR::initStaticResource() {
	int tmp = 0;
	_mainMenuStrings = _staticres->loadStrings(k3MainMenuStrings, _mainMenuStringsSize);
	_soundList = _staticres->loadStrings(k3MusicFiles, _soundListSize);
	_scoreTable = _staticres->loadRawData(k3ScoreTable, _scoreTableSize);
	_sfxFileList = _staticres->loadStrings(k3SfxFiles, _sfxFileListSize);
	_sfxFileMap = _staticres->loadRawData(k3SfxMap, _sfxFileMapSize);
	_itemAnimDefinition = _staticres->loadItemAnimDefinition(k3ItemAnimData, tmp);
	_itemMagicTable = _staticres->loadRawData(k3ItemMagicTable, tmp);
	_itemStringMap = _staticres->loadRawData(k3ItemStringMap, _itemStringMapSize);
}

const uint8 Screen_LoK_16::_palette16[48] = {
	0x00, 0x00, 0x00, 0x02, 0x07, 0x0B, 0x0C, 0x06, 0x04,
	0x0E, 0x09, 0x07, 0x00, 0x06, 0x03, 0x00, 0x0C, 0x07,
	0x0A, 0x0A, 0x0A, 0x08, 0x03, 0x03, 0x02, 0x02, 0x02,
	0x08, 0x0B, 0x0E, 0x0D, 0x00, 0x00, 0x00, 0x00, 0x0A,
	0x05, 0x05, 0x05, 0x00, 0x0F, 0x0F, 0x0F, 0x0D, 0x00,
	0x0F, 0x0F, 0x0F
};

const ScreenDim Screen_LoK::_screenDimTable[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x08, 0x26, 0x80, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x00, 0xC2, 0x28, 0x06, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0x04, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x94, 0x26, 0x30, 0x04, 0x1B, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0x0F, 0x0D, 0x00, 0x00 },
	{ 0x01, 0x96, 0x26, 0x32, 0x0F, 0x0D, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0x88, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x01, 0x20, 0x26, 0x80, 0x0F, 0x0C, 0x00, 0x00 },
	{ 0x03, 0x28, 0x22, 0x46, 0x0F, 0x0D, 0x00, 0x00 }
};

const int Screen_LoK::_screenDimTableCount = ARRAYSIZE(Screen_LoK::_screenDimTable);

const ScreenDim Screen_HoF::_screenDimTable[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0x90, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x00, 0xC2, 0x28, 0x06, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0x96, 0xCF, 0x00, 0x00 },
	{ 0x01, 0x94, 0x26, 0x30, 0x96, 0x1B, 0x00, 0x00 },
	{ 0x00, 0x90, 0x28, 0x38, 0xC7, 0xCC, 0x00, 0x00 },
	{ 0x01, 0x96, 0x26, 0x32, 0xC7, 0xCC, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0x88, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x00, 0x08, 0x28, 0xB8, 0xC7, 0xCF, 0x00, 0x00 },
	{ 0x01, 0x28, 0x26, 0x46, 0xC7, 0xCC, 0x00, 0x00 },
	{ 0x0A, 0x96, 0x14, 0x30, 0x19, 0xF0, 0x00, 0x00 }  // menu, just present for current menu code
};

const int Screen_HoF::_screenDimTableCount = ARRAYSIZE(Screen_HoF::_screenDimTable);

const ScreenDim Screen_MR::_screenDimTable[] = {
	{ 0x00, 0x00, 0x28, 0xC8, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x08, 0x48, 0x18, 0x38, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x00, 0x00, 0x28, 0xBC, 0xFF, 0xF0, 0x00, 0x00 },
	{ 0x0A, 0x96, 0x14, 0x30, 0x19, 0xF0, 0x00, 0x00 }
};

const int Screen_MR::_screenDimTableCount = ARRAYSIZE(Screen_MR::_screenDimTable);

const int8 KyraEngine_v1::_addXPosTable[] = {
	 4,  4,  0, -4, -4, -4,  0,  4
};

const int8 KyraEngine_v1::_addYPosTable[] = {
	 0, -2, -2, -2,  0,  2,  2,  2
};

const int8 KyraEngine_v1::_charAddXPosTable[] = {
	 0,  4,  4,  4,  0, -4, -4, -4
};

const int8 KyraEngine_v1::_charAddYPosTable[] = {
	-2, -2,  0,  2,  2,  2,  0, -2
};

const uint16 KyraEngine_LoK::_itemPosX[] = {
	95, 115, 135, 155, 175, 95, 115, 135, 155, 175
};

const uint8 KyraEngine_LoK::_itemPosY[] = {
	160, 160, 160, 160, 160, 181, 181, 181, 181, 181
};

void GUI_LoK::initStaticResource() {
	GUI_V1_BUTTON(_scrollUpButton, 0x12, 1, 1, 1, 0x483, 0, 0, 0, 0x18, 0x0f, 0);
	GUI_V1_BUTTON(_scrollDownButton, 0x13, 1, 1, 1, 0x483, 0, 0, 0, 0x18, 0x0f, 0);

	GUI_V1_BUTTON(_menuButtonData[0], 0x0c, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[1], 0x0d, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[2], 0x0e, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[3], 0x0f, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[4], 0x10, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);
	GUI_V1_BUTTON(_menuButtonData[5], 0x11, 1, 1, 1, 0x487, 0, 0, 0, 0, 0, 0);

	delete[] _menu;
	_menu = new Menu[6];
	assert(_menu);

	Button::Callback quitPlayingFunctor = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::quitPlaying);
	Button::Callback loadGameMenuFunctor = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::loadGameMenu);
	Button::Callback cancelSubMenuFunctor = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::cancelSubMenu);

	GUI_V1_MENU(_menu[0], -1, -1, 0x100, 0x8B, 248, 249, 250, 0, 251, -1, 8, 0, 5, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[0].item[0], 1, 0, 0, 0, -1, -1, 0x1E, 0xDC, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[0].item[1], 1, 0, 0, 0, -1, -1, 0x2F, 0xDC, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[0].item[2], 1, 0, 0, 0, -1, -1, 0x40, 0xDC, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[0].item[3], 1, 0, 0, 0, -1, -1, 0x51, 0xDC, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[0].item[4], 1, 0, 0, 0, -1,  0, 0x6E, 0xDC, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[0].item[0].callback = loadGameMenuFunctor;
	_menu[0].item[1].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::saveGameMenu);
	_menu[0].item[2].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::gameControlsMenu);
	_menu[0].item[3].callback = quitPlayingFunctor;
	_menu[0].item[4].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::resumeGame);

	GUI_V1_MENU(_menu[1], -1, -1, 0x140, 0x38, 248, 249, 250, 0, 254, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[1].item[0], 1, 0, 0, 0, 0x18, 0, 0x1E, 0x48, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[1].item[1], 1, 0, 0, 0, 0xD8, 0, 0x1E, 0x48, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[1].item[0].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::quitConfirmYes);
	_menu[1].item[1].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::quitConfirmNo);

	GUI_V1_MENU(_menu[2], -1, -1, 0x120, 0xA0, 248, 249, 250, 0, 251, -1, 8, 0, 6, 132, 22, 132, 124);
	GUI_V1_MENU_ITEM(_menu[2].item[0], 1, 0, 0, 0, -1, 255, 0x27, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[1], 1, 0, 0, 0, -1, 255, 0x38, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[2], 1, 0, 0, 0, -1, 255, 0x49, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[3], 1, 0, 0, 0, -1, 255, 0x5A, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[4], 1, 0, 0, 0, -1, 255, 0x6B, 0x100, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[2].item[5], 1, 0, 0, 0, 0xB8, 0, 0x86, 0x58, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[2].item[5].callback = cancelSubMenuFunctor;

	GUI_V1_MENU(_menu[3], -1, -1, 288, 67, 248, 249, 250, 0, 251, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[3].item[0], 1, 0, 0, 0, 24, 0, 44, 85, 15, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[3].item[1], 1, 0, 0, 0, 179, 0, 44, 85, 15, 252, 253, -1, 255, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[3].item[0].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::savegameConfirm);
	_menu[3].item[1].callback = cancelSubMenuFunctor;

	GUI_V1_MENU(_menu[4], -1, -1, 0xD0, 0x4C, 248, 249, 250, 0, 251, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[4].item[0], 1, 0, 0, 0, -1, -1, 0x1E, 0xB4, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	GUI_V1_MENU_ITEM(_menu[4].item[1], 1, 0, 0, 0, -1, -1, 0x2F, 0xB4, 0x0F, 252, 253, -1, 0, 248, 249, 250, -1, 0, 0, 0, 0, 0);
	_menu[4].item[0].callback = loadGameMenuFunctor;
	_menu[4].item[1].callback = quitPlayingFunctor;

	GUI_V1_MENU(_menu[5], -1, -1, 0x130, 0x99, 248, 249, 250, 0, 251, -1, 8, 0, 6, -1, -1, -1, -1);
	GUI_V1_MENU_ITEM(_menu[5].item[0], 1, 0, 0, 0, 0xA5, 0, 0x1E, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x20, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[1], 1, 0, 0, 0, 0xA5, 0, 0x2F, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x31, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[2], 1, 0, 0, 0, 0xA5, 0, 0x40, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x42, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[3], 1, 0, 0, 0, 0xA5, 0, 0x51, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x53, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[4], 1, 0, 0, 0, 0xA5, 0, 0x62, 0x80, 0x0F, 252, 253, 5, 0, 248, 249, 250, -1, 0, 0x10, 0x65, 0, 0);
	GUI_V1_MENU_ITEM(_menu[5].item[5], 1, 0, 0, 0,   -1, 0, 0x7F, 0x6C, 0x0F, 252, 253, -1, 255, 248, 249, 250, -1, -0, 0, 0, 0, 0);
	_menu[5].item[0].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::controlsChangeMusic);
	_menu[5].item[1].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::controlsChangeSounds);
	_menu[5].item[2].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::controlsChangeWalk);
	_menu[5].item[4].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::controlsChangeText);
	_menu[5].item[5].callback = BUTTON_FUNCTOR(GUI_LoK, this, &GUI_LoK::controlsApply);

	// The AMIGA version uses different colors, due to its 32 color nature. We did setup the 256 color version
	// colors above, so we need to overwrite those with the correct values over here.
	if (_vm->gameFlags().platform == Common::kPlatformAmiga) {
		for (int i = 0; i < 6; ++i) {
			_menu[i].bkgdColor = 17;
			_menu[i].color1 = 31;
			_menu[i].color2 = 18;

			for (int j = 0; j < _menu[i].numberOfItems; ++j) {
				_menu[i].item[j].bkgdColor = 17;
				_menu[i].item[j].color1 = 31;
				_menu[i].item[j].color2 = 18;
			}
		}
	}
}

void KyraEngine_LoK::setupButtonData() {
	delete[] _buttonData;
	delete[] _buttonDataListPtr;

	_buttonData = new Button[15];
	assert(_buttonData);
	_buttonDataListPtr = new Button *[15];
	assert(_buttonDataListPtr);

	GUI_V1_BUTTON(_buttonData[1], 0x01, 1, 1, 1, 0x0487, 0, 0x009, 0xA4, 0x36, 0x1E, 0);
	_buttonData[1].buttonCallback = BUTTON_FUNCTOR(GUI_LoK, _gui, &GUI_LoK::buttonMenuCallback);

	Button::Callback inventoryFunctor = BUTTON_FUNCTOR(KyraEngine_LoK, this, &KyraEngine_LoK::buttonInventoryCallback);
	for (int i = 2; i <= 10; ++i)
		_buttonData[i].buttonCallback = inventoryFunctor;
	_buttonData[0].buttonCallback = inventoryFunctor;
	GUI_V1_BUTTON(_buttonData[0], 0x02, 0, 0, 0, 0x0400, 0, 0x05D, 0x9E, 0x13, 0x13, 0);
	GUI_V1_BUTTON(_buttonData[2], 0x03, 0, 0, 0, 0x0400, 0, 0x071, 0x9E, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[3], 0x04, 0, 0, 0, 0x0400, 0, 0x085, 0x9E, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[4], 0x05, 0, 0, 0, 0x0400, 0, 0x099, 0x9E, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[5], 0x06, 0, 0, 0, 0x0400, 0, 0x0AD, 0x9E, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[6], 0x07, 0, 0, 0, 0x0400, 0, 0x05D, 0xB3, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[7], 0x08, 0, 0, 0, 0x0400, 0, 0x071, 0xB3, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[8], 0x09, 0, 0, 0, 0x0400, 0, 0x085, 0xB3, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[9], 0x0A, 0, 0, 0, 0x0400, 0, 0x099, 0xB3, 0x13, 0x14, 0);
	GUI_V1_BUTTON(_buttonData[10], 0x0B, 0, 0, 0, 0x0400, 0, 0x0AD, 0xB3, 0x13, 0x14, 0);

	Button::Callback amuletFunctor = BUTTON_FUNCTOR(KyraEngine_LoK, this, &KyraEngine_LoK::buttonAmuletCallback);
	GUI_V1_BUTTON(_buttonData[11], 0x15, 1, 1, 1, 0x0487, 0, 0x0FD, 0x9C, 0x1A, 0x12, 0);
	GUI_V1_BUTTON(_buttonData[12], 0x16, 1, 1, 1, 0x0487, 0, 0x0E7, 0xAA, 0x1A, 0x12, 0);
	GUI_V1_BUTTON(_buttonData[13], 0x17, 1, 1, 1, 0x0487, 0, 0x0FD, 0xB5, 0x1A, 0x12, 0);
	GUI_V1_BUTTON(_buttonData[14], 0x18, 1, 1, 1, 0x0487, 0, 0x113, 0xAA, 0x1A, 0x12, 0);
	for (int i = 11; i <= 14; ++i)
		_buttonData[i].buttonCallback = amuletFunctor;

	for (int i = 1; i < 15; ++i)
		_buttonDataListPtr[i - 1] = &_buttonData[i];
	_buttonDataListPtr[14] = 0;
}

const uint8 KyraEngine_LoK::_magicMouseItemStartFrame[] = {
	0xAD, 0xB7, 0xBE, 0x00
};

const uint8 KyraEngine_LoK::_magicMouseItemEndFrame[] = {
	0xB1, 0xB9, 0xC2, 0x00
};

const uint8 KyraEngine_LoK::_magicMouseItemStartFrame2[] = {
	0xB2, 0xBA, 0xC3, 0x00
};

const uint8 KyraEngine_LoK::_magicMouseItemEndFrame2[] = {
	0xB6, 0xBD, 0xC8, 0x00
};

const uint16 KyraEngine_LoK::_amuletX[] = { 231, 275, 253, 253 };
const uint16 KyraEngine_LoK::_amuletY[] = { 170, 170, 159, 181 };

const uint16 KyraEngine_LoK::_amuletX2[] = { 0x000, 0x0FD, 0x0E7, 0x0FD, 0x113, 0x000 };
const uint16 KyraEngine_LoK::_amuletY2[] = { 0x000, 0x09F, 0x0AA, 0x0B5, 0x0AA, 0x000 };

const int8 KyraEngine_LoK::_dosTrackMap[] = {
	-1,   0,  -1,   1,   0,   3,   0,   2,
	 0,   4,   1,   2,   1,   3,   1,   4,
	 1,  92,   1,   6,   1,   7,   2,   2,
	 2,   3,   2,   4,   2,   5,   2,   6,
	 2,   7,   3,   3,   3,   4,   1,   8,
	 1,   9,   4,   2,   4,   3,   4,   4,
	 4,   5,   4,   6,   4,   7,   4,   8,
	 1,  11,   1,  12,   1,  14,   1,  13,
	 4,   9,   5,  12,   6,   2,   6,   6,
	 6,   7,   6,   8,   6,   9,   6,   3,
	 6,   4,   6,   5,   7,   2,   7,   3,
	 7,   4,   7,   5,   7,   6,   7,   7,
	 7,   8,   7,   9,   8,   2,   8,   3,
	 8,   4,   8,   5,   6,  11,   5,  11
};

const int KyraEngine_LoK::_dosTrackMapSize = ARRAYSIZE(KyraEngine_LoK::_dosTrackMap);

const int8 KyraEngine_LoK::_amigaTrackMap[] = {
	 0,  1, 32, 26, 31, 30, 33, 33,
	32, 17, 27, 32, 25, 29, 25, 24,
	23, 26, 26, 30, 28, 21, 21, 15,
	 3, 15, 23, 25, 33, 21, 30, 22,
	15,  3, 33, 11, 12, 13, 14, 22,
	22, 22,  3,  3,  3, 23,  3,  3,
	23,  3,  3,  3,  3,  3,  3, 33
};

const int KyraEngine_LoK::_amigaTrackMapSize = ARRAYSIZE(KyraEngine_LoK::_amigaTrackMap);

// kyra engine v2 static data

const int GUI_v2::_sliderBarsPosition[] = {
	0x92, 0x1F, 0x92, 0x30, 0x92, 0x41, 0x92, 0x52
};

// kyra 2 static res

const char *const KyraEngine_HoF::_languageExtension[] = {
	"ENG",
	"FRE",
	"GER",/*,
	"ITA",      Italian and Spanish were never included
	"SPA"*/
	"JPN",
};

const char *const KyraEngine_HoF::_scriptLangExt[] = {
	"EMC",
	"FMC",
	"GMC",/*,
	"IMC",      Italian and Spanish were never included
	"SMC"*/
	"JMC"
};

const uint8 KyraEngine_HoF::_characterFrameTable[] = {
	0x19, 0x09, 0x09, 0x12, 0x12, 0x12, 0x09, 0x09
};

const int KyraEngine_HoF::_inventoryX[] = {
	0x4F, 0x63, 0x77, 0x8B, 0x9F, 0x4F, 0x63, 0x77, 0x8B, 0x9F
};

const int KyraEngine_HoF::_inventoryY[] = {
	0x95, 0x95, 0x95, 0x95, 0x95, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA
};

const byte KyraEngine_HoF::_itemStringMap[] = {
	2,    2,    0,    0,    2,    2,    2,    0,
	2,    2,    0,    0,    0,    2,    0,    0,
	0,    0,    0,    0,    2,    0,    0,    0,
	0,    1,    0,    2,    2,    2,    2,    0,
	3,    0,    3,    2,    2,    2,    3,    2,
	2,    2,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    2,    0,    0,    0,
	0,    0,    0,    0,    0,    2,    0,    0,
	2,    0,    0,    0,    0,    0,    0,    2,
	2,    0,    0,    0,    2,    2,    2,    2,
	2,    2,    2,    2,    2,    2,    2,    2,
	2,    2,    2,    2,    2,    2,    2,    0,
	2,    2,    2,    0,    0,    1,    3,    2,
	2,    2,    2,    2,    2,    0,    0,    0,
	0,    2,    2,    1,    0,    1,    2,    0,
	0,    0,    0,    0,    0,    2,    2,    2,
	2,    2,    2,    2,    0,    2,    2,    2,
	2,    3,    2,    0,    0,    0,    0,    1,
	2,    0,    0,    0,    0,    0,    0,    0,
	0,    0,    0,    0,    0,    0,    0,    0,
	2,    2,    0,    0,    0,    0,    0,    2,
	0,    2,    0,    0,    0,    0,    0,    0
};

const int KyraEngine_HoF::_itemStringMapSize = ARRAYSIZE(KyraEngine_HoF::_itemStringMap);

const int8 KyraEngine_HoF::_dosTrackMap[] = {
	-1,    0,   -1,    1,    9,    6,    5,    4,
	 8,    3,   -2,    0,   -2,    0,    2,    3,
	-2,    0,   -2,    0,   -2,    0,   -2,    0,
	 0,    2,    0,    3,    1,    2,    1,    3,
	 2,    2,    2,    0,    3,    2,    3,    3,
	 3,    4,    4,    2,    5,    2,    5,    3,
	 5,    4,    6,    2,    6,    3,    6,    4,
	 6,    5,    6,    6,    6,    7,    6,    8,
	 6,    0,    6,    9,    7,    2,    7,    3,
	 7,    4,    7,    5,    8,    6,    7,    6,
	 7,    7,    7,    8,    7,    9,    8,    2,
	14,    2,    8,    4,    8,    7,    8,    8,
	 8,    9,    9,    2,    9,    3,    9,    4,
	 9,    5,    9,    7,    9,    8,    9,    9,
	10,    2,   10,    3,   10,    4,   10,    5,
	10,    6,   10,    7,   11,    2,   11,    3,
	11,    4,   11,    5,   11,    6,   11,    7,
	11,    8,   11,    9,   12,    2,   12,    3,
	12,    4,   12,    5,   12,    6,   12,    7,
	12,    8,   12,    9,   13,    2,    4,    7,
	14,    3,   14,    4,   14,    5,    4,    2,
	 4,    3,    4,    4,    4,    5,    4,    6
};

const int KyraEngine_HoF::_dosTrackMapSize = ARRAYSIZE(KyraEngine_HoF::_dosTrackMap);

const int8 KyraEngine_HoF::_mt32SfxMap[] = {
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   49,   27,    5,   36,   13,
	-1,   -1,   68,   55,   37,   73,   43,   61,
	49,   -1,   56,   -1,   62,   38,   -1,   -1,
	61,   -1,   -1,   31,   70,    2,   45,   -1,
	45,   -1,   -1,   -1,   10,   14,   24,   25,
	-1,   -1,   59,    9,   26,   -1,   71,   79,
	12,    9,   -1,   -1,   61,   -1,   -1,   65,
	66,   50,   27,   24,   29,   29,   15,   16,
	17,   18,   19,   20,   21,   57,   -1,   -1,
	34,    3,   -1,   56,   56,   -1,   -1,   50,
	43,   68,   32,   33,   67,   25,   60,   40,
	39,   11,   24,    2,   60,    3,   46,   54,
	 1,    8,   -1,   -1,   41,   42,   37,   74,
	69,   62,   58,   27,   -1,   -1,   -1,   -1,
	48,    4,   -1,   25,   39,   40,   24,   58,
	35,    4,    4,    4,   -1,   50,   -1,    6,
	 8,   -1,   -1,   -1,   -1,   -1,   53,   52,
	-1,   63,   47,   -1,   -1,   -1,   53,   -1,
	29,   -1,   -1,   79,   -1,   41,   12,   -1,
	-1,   -1,   26,   -1,    7,   27,   72,   51,
	23,   51,   64,   -1,   -1,   -1,   27,   76,
	77,   78,   28,   47,   -1,   -1,   53,   -1,
	-1,   -1,   -1,   -1,    2,   22,   -1,   51,
	58,   -1,   -1,   30,   -1,   79,   -1,   -1,
	22,   36,    1,   -1,   12,    1,   -1,   -1,
	41,   -1,   76,   77,   47
};

const int KyraEngine_HoF::_mt32SfxMapSize = ARRAYSIZE(KyraEngine_HoF::_mt32SfxMap);

const int8 KyraEngine_HoF::_gmSfxMap[] = {
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   31,   25,   19,   12,    4,
	-1,   -1,   46,   18,   -1,   21,   15,   -1,
	31,   -1,   -1,   -1,   -1,   -1,   47,   -1,
	33,   -1,   36,   -1,   -1,   23,   48,   -1,
	48,   -1,   -1,   49,   -1,   50,   22,   24,
	51,   -1,   52,   20,   -1,   -1,   22,   53,
	 3,   20,   47,   54,   33,   -1,   55,   56,
	57,   33,   -1,   51,   58,   -1,    5,    6,
	 7,    8,    9,   10,   11,   22,   -1,   -1,
	-1,   24,   -1,   26,   17,   -1,   -1,   33,
	15,   -1,   23,   23,   -1,   22,   -1,   23,
	24,   21,   22,   -1,   -1,   24,   16,   -1,
	 1,   48,   -1,   -1,   13,   14,   -1,   29,
	64,   -1,   -1,   25,   -1,   -1,   -1,   -1,
	-1,    2,   13,   24,   23,   23,   22,   -1,
	60,    2,    2,    2,   -1,   33,   -1,   61,
	48,   62,   -1,   39,   -1,   -1,   28,   63,
	33,   -1,   17,   -1,   45,   45,   28,   55,
	34,   -1,   -1,   34,   55,   13,   -1,   47,
	54,   -1,   -1,   33,   44,   25,   -1,   -1,
	-1,   32,   -1,   -1,   -1,   -1,   25,   37,
	37,   37,   26,   43,   -1,   42,   24,   -1,
	-1,   -1,   -1,   -1,   23,   32,   -1,   32,
	-1,   -1,   -1,   27,   41,   34,   -1,   40,
	32,   -1,   16,   40,   -1,   16,   38,   39,
	13,   -1,   37,   28,   33
};

const int KyraEngine_HoF::_gmSfxMapSize = ARRAYSIZE(KyraEngine_HoF::_gmSfxMap);

const int8 KyraEngine_HoF::_pcSpkSfxMap[] = {
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,    0,    1,    2,    3,    4,
	 5,    6,   -1,    7,    8,    9,   10,   -1,
	 6,   -1,   -1,   11,   -1,   12,   -1,   -1,
	-1,   -1,   -1,   13,   -1,   39,   14,   15,
	 3,   16,   16,   -1,   -1,   -1,   17,   18,
	 5,   -1,   -1,   -1,   -1,   -1,   19,   20,
	21,   -1,   22,   23,   -1,   -1,   -1,   -1,
	-1,   -1,   39,   -1,   24,   24,   25,   26,
	27,   28,   29,   30,   31,   32,   -1,   -1,
	-1,    2,   -1,   -1,   -1,   -1,   -1,   21,
	10,   -1,   -1,   -1,   -1,   17,   -1,   17,
	40,   -1,   18,   38,   -1,   40,   33,   -1,
	34,   35,   36,   37,   38,   39,   40,   41,
	-1,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   42,   43,   44,   45,   -1,   -1,
	-1,   -1,   -1,   -1,   46,   -1,    5,   47,
	48,   -1,   -1,   -1,   -1,   -1,   49,   50,
	-1,   40,   -1,   24,   -1,   -1,   43,   -1,
	-1,   38,   -1,   -1,   -1,   51,   -1,   -1,
	-1,   -1,   -1,   -1,   -1,    9,   -1,   52,
	53,   40,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   50,   -1,   -1,   -1,   11,   54,
	 5,   -1,   -1,   -1,   -1,   11,    7,   55,
	 8,   36,   -1,   -1,   -1,   -1,   -1,   -1,
	11,   -1,   -1,   -1,   -1,   -1,   -1,   -1,
	-1,   -1,   -1,   -1,   24
};

const int KyraEngine_HoF::_pcSpkSfxMapSize = ARRAYSIZE(KyraEngine_HoF::_pcSpkSfxMap);

void KyraEngine_HoF::initInventoryButtonList() {
	delete[] _inventoryButtons;

	_inventoryButtons = new Button[15];
	assert(_inventoryButtons);

	GUI_V2_BUTTON(_inventoryButtons[0], 0x1, 0x4F, 0, 1, 1, 1, 0x4487, 0, 0x00A, 0x95, 0x39, 0x1D, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[0].buttonCallback = BUTTON_FUNCTOR(GUI_HoF, _gui, &GUI_HoF::optionsButton);

	GUI_V2_BUTTON(_inventoryButtons[1], 0x2, 0x00, 0, 1, 1, 1, 0x4487, 0, 0x104, 0x90, 0x3C, 0x2C, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[1].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::cauldronButton);

	GUI_V2_BUTTON(_inventoryButtons[2], 0x5, 0x00, 0, 1, 1, 1, 0x4487, 0, 0x0FA, 0x90, 0x0A, 0x2C, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[2].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::cauldronClearButton);

	GUI_V2_BUTTON(_inventoryButtons[3], 0x3, 0x00, 0, 1, 1, 1, 0x4487, 0, 0x0CE, 0x90, 0x2C, 0x2C, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[3].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::bookButton);

	GUI_V2_BUTTON(_inventoryButtons[4], 0x4, 0x00, 0, 1, 1, 1, 0x4487, 0, 0x0B6, 0x9D, 0x18, 0x1E, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	_inventoryButtons[4].buttonCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::scrollInventory);

	Button::Callback inventoryCallback = BUTTON_FUNCTOR(KyraEngine_HoF, this, &KyraEngine_HoF::buttonInventory);
	GUI_V2_BUTTON(_inventoryButtons[5], 0x6, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x04D, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[6], 0x7, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x061, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[7], 0x8, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x075, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[8], 0x9, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x089, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[9], 0xA, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x09D, 0x92, 0x13, 0x15, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[10], 0xB, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x04D, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[11], 0xC, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x061, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[12], 0xD, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x075, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[13], 0xE, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x089, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_inventoryButtons[14], 0xF, 0x00, 0, 0, 0, 0, 0x1100, 0, 0x09D, 0xA8, 0x13, 0x14, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);

	for (int i = 5; i <= 14; ++i)
		_inventoryButtons[i].buttonCallback = inventoryCallback;

	_buttonList = &_inventoryButtons[0];
	for (size_t i = 1; i < 15; ++i)
		_buttonList = _gui->addButtonToList(_buttonList, &_inventoryButtons[i]);
}

void GUI_HoF::initStaticData() {
	GUI_V2_BUTTON(_scrollUpButton, 0x17, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x18, 0x0F, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	GUI_V2_BUTTON(_scrollDownButton, 0x18, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x18, 0x0F, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);

	for (int i = 0; i < 4; ++i)
		GUI_V2_BUTTON(_sliderButtons[0][i], 0x18 + i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x0A, 0x0E, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	for (int i = 0; i < 4; ++i)
		GUI_V2_BUTTON(_sliderButtons[1][i], 0x1C + i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x0A, 0x0E, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);
	for (int i = 0; i < 4; ++i)
		GUI_V2_BUTTON(_sliderButtons[2][i], 0x20 + i, 0, 0, 0, 0, 0, 0x2200, 0, 0, 0, 0x6E, 0x0E, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);

	for (uint i = 0; i < ARRAYSIZE(_menuButtons); ++i)
		GUI_V2_BUTTON(_menuButtons[i], 0x10 + i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0, 0, 0xC7, 0xCF, 0xC7, 0xCF, 0xC7, 0xCF, 0);

	Button::Callback clickLoadSlotFunctor = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::clickLoadSlot);
	Button::Callback clickSaveSlotFunctor = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::clickSaveSlot);
	Button::Callback clickLoadMenuFunctor = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::loadMenu);
	Button::Callback clickQuitGameFunctor = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::quitGame);
	Button::Callback clickQuitOptionsFunctor = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::quitOptionsMenu);

	const uint16 *menuStr = _vm->gameFlags().isTalkie ? _menuStringsTalkie : _menuStringsOther;

	GUI_V2_MENU(_mainMenu, -1, -1, 0x100, 0xAC, 0xF8, 0xF9, 0xFA, menuStr[0 * 8], 0xFB, -1, 8, 0, 7, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_mainMenu.item[0], 1, 0x02, -1, 0x1E, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[0].callback = clickLoadMenuFunctor;
	GUI_V2_MENU_ITEM(_mainMenu.item[1], 1, 0x03, -1, 0x2F, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[1].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::saveMenu);
	GUI_V2_MENU_ITEM(_mainMenu.item[2], 1, 0x23, -1, 0x40, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[2].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::deleteMenu);
	GUI_V2_MENU_ITEM(_mainMenu.item[3], 1, 0x04, -1, 0x51, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[3].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::gameOptionsTalkie);
	GUI_V2_MENU_ITEM(_mainMenu.item[4], 1, 0x25, -1, 0x62, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[4].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::audioOptions);
	GUI_V2_MENU_ITEM(_mainMenu.item[5], 1, 0x05, -1, 0x73, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[5].callback = clickQuitGameFunctor;
	GUI_V2_MENU_ITEM(_mainMenu.item[6], 1, 0x06, -1, 0x90, 0xDC, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_mainMenu.item[6].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::resumeGame);
	for (int i = 0; i < 7; ++i)
		_mainMenu.item[i].itemId = menuStr[0 * 8 + i + 1];

	if (!_vm->gameFlags().isTalkie) {
		_mainMenu.height = 0x9C;
		_mainMenu.numberOfItems = 6;
		_mainMenu.item[6].enabled = false;
		for (int i = 4; i < 6; ++i)
			_mainMenu.item[i].callback = _mainMenu.item[i + 1].callback;
		_mainMenu.item[3].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::gameOptions);
		_mainMenu.item[6].callback = Button::Callback();
		_mainMenu.item[5].y = 0x7F;
	}

	GUI_V2_MENU(_gameOptions, -1, -1, 0x120, 0x88, 0xF8, 0xF9, 0xFA, menuStr[1 * 8], 0xFB, -1, 8, 4, 4, -1, -1, -1, -1);
	if (_vm->gameFlags().isTalkie) {
		GUI_V2_MENU_ITEM(_gameOptions.item[0], 1, 0, 0xA0, 0x1E, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x15, 8, 0x20, 0);
		_gameOptions.item[0].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::toggleWalkspeed);
		GUI_V2_MENU_ITEM(_gameOptions.item[1], 1, 0, 0xA0, 0x2F, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x26, 8, 0x31, 0);
		_gameOptions.item[1].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::changeLanguage);
		GUI_V2_MENU_ITEM(_gameOptions.item[2], 1, 0, 0xA0, 0x40, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x16, 8, 0x42, 0);
		_gameOptions.item[2].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::toggleText);
		GUI_V2_MENU_ITEM(_gameOptions.item[3], 1, 0x10, -1, 0x6E, 0x6C, 0x0F, 0xFD, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
		_gameOptions.item[3].callback = clickQuitOptionsFunctor;
	} else {
		_gameOptions.numberOfItems = 5;
		GUI_V2_MENU_ITEM(_gameOptions.item[0], 0, 0x2B, 0xA0, 0x1E, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x1F, 0x10, 0x20, 0);
		GUI_V2_MENU_ITEM(_gameOptions.item[1], 0, 0x2C, 0xA0, 0x2F, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x20, 0x10, 0x31, 0);
		GUI_V2_MENU_ITEM(_gameOptions.item[2], 0, 0x2D, 0xA0, 0x40, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x1D, 0x10, 0x42, 0);
		GUI_V2_MENU_ITEM(_gameOptions.item[3], 0, 0x2E, 0xA0, 0x51, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x1E, 0x10, 0x53, 0);
		GUI_V2_MENU_ITEM(_gameOptions.item[4], 1, 0x18, -1, 0x6E, 0x6C, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
		_gameOptions.item[4].callback = clickQuitOptionsFunctor;
	}

	for (int i = _gameOptions.numberOfItems; i <= 6; ++i)
		_gameOptions.item[i].enabled = false;
	for (int i = 0; i < 7; ++i)
		_gameOptions.item[i].itemId = menuStr[1 * 8 + i + 1];

	GUI_V2_MENU(_audioOptions, -1, -1, 0x120, 0x88, 0xF8, 0xF9, 0xFA, menuStr[2 * 8], 0xFB, -1, 8, 3, 4, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_audioOptions.item[0], 0, 0, 0xA0, 0x1E, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x17, 8, 0x20, 0);
	GUI_V2_MENU_ITEM(_audioOptions.item[1], 0, 0, 0xA0, 0x2F, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x18, 8, 0x31, 0);
	GUI_V2_MENU_ITEM(_audioOptions.item[2], 0, 0, 0xA0, 0x40, 0x74, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0x27, 8, 0x42, 0);
	GUI_V2_MENU_ITEM(_audioOptions.item[3], 1, 0x10, -1, 0x6E, 0x5C, 0x0F, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_audioOptions.item[3].callback = clickQuitOptionsFunctor;
	for (int i = 4; i <= 6; ++i)
		_audioOptions.item[i].enabled = false;
	for (int i = 0; i < 7; ++i)
		_audioOptions.item[i].itemId = menuStr[2 * 8 + i + 1];

	GUI_V2_MENU(_choiceMenu, -1, -1, 0x140, 0x38, 0xF8, 0xF9, 0xFA, 0, 0xFE, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_choiceMenu.item[0], 1, 0x14, 0x18, 0x1E, 0x48, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_choiceMenu.item[0].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::choiceYes);
	GUI_V2_MENU_ITEM(_choiceMenu.item[1], 1, 0x13, 0xD8, 0x1E, 0x48, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_choiceMenu.item[1].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::choiceNo);
	for (int i = 2; i <= 6; ++i)
		_choiceMenu.item[i].enabled = false;
	for (int i = 0; i < 7; ++i)
		_choiceMenu.item[i].itemId = menuStr[3 * 8 + i + 1];

	GUI_V2_MENU(_loadMenu, -1, -1, 0x120, 0xA0, 0xF8, 0xF9, 0xFA, menuStr[4 * 8], 0xFB, -1, 8, 0, 6, 0x84, 0x16, 0x84, 0x7C);
	GUI_V2_MENU_ITEM(_loadMenu.item[0], 1, 0x29, -1, 0x27, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_loadMenu.item[1], 1, 0x2A, -1, 0x38, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_loadMenu.item[2], 1, 0x2B, -1, 0x49, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_loadMenu.item[3], 1, 0x2C, -1, 0x5A, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_loadMenu.item[4], 1, 0x2D, -1, 0x6B, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	for (int i = 0; i <= 4; ++i)
		_loadMenu.item[i].callback = clickLoadSlotFunctor;
	GUI_V2_MENU_ITEM(_loadMenu.item[5], 1, 0x0B, 0xB8, 0x86, 0x58, 0xF, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_loadMenu.item[5].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::cancelLoadMenu);
	_loadMenu.item[6].enabled = false;
	for (int i = 0; i < 7; ++i)
		_loadMenu.item[i].itemId = menuStr[4 * 8 + i + 1];

	GUI_V2_MENU(_saveMenu, -1, -1, 0x120, 0xA0, 0xF8, 0xF9, 0xFA, menuStr[5 * 8], 0xFB, -1, 8, 0, 6, 0x84, 0x16, 0x84, 0x7C);
	GUI_V2_MENU_ITEM(_saveMenu.item[0], 1, 0x29, -1, 0x27, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_saveMenu.item[1], 1, 0x2A, -1, 0x38, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_saveMenu.item[2], 1, 0x2B, -1, 0x49, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_saveMenu.item[3], 1, 0x2C, -1, 0x5A, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	GUI_V2_MENU_ITEM(_saveMenu.item[4], 1, 0x2D, -1, 0x6B, 0x100, 0xF, 0xFC, 0xFD, 5, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	for (int i = 0; i <= 4; ++i)
		_saveMenu.item[i].callback = clickSaveSlotFunctor;
	GUI_V2_MENU_ITEM(_saveMenu.item[5], 1, 0x0B, 0xB8, 0x86, 0x58, 0xF, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_saveMenu.item[5].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::cancelSaveMenu);
	_saveMenu.item[6].enabled = false;
	for (int i = 0; i < 7; ++i)
		_saveMenu.item[i].itemId = menuStr[5 * 8 + i + 1];

	GUI_V2_MENU(_savenameMenu, -1, -1, 0x140, 0x43, 0xF8, 0xF9, 0xFA, menuStr[6 * 8], 0xFB, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_savenameMenu.item[0], 1, 0xD, 0x18, 0x2C, 0x58, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_savenameMenu.item[0].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::finishSavename);
	GUI_V2_MENU_ITEM(_savenameMenu.item[1], 1, 0xB, 0xD0, 0x2C, 0x58, 0x0F, 0xFC, 0xFD, -1, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_savenameMenu.item[1].callback = BUTTON_FUNCTOR(GUI_HoF, this, &GUI_HoF::cancelSavename);
	for (int i = 2; i <= 6; ++i)
		_savenameMenu.item[i].enabled = false;
	for (int i = 0; i < 7; ++i)
		_savenameMenu.item[i].itemId = menuStr[6 * 8 + i + 1];

	GUI_V2_MENU(_deathMenu, -1, -1, 0xD0, 0x4C, 0xF8, 0xF9, 0xFA, menuStr[7 * 8], 0xFB, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_deathMenu.item[0], 1, 2, -1, 0x1E, 0xB4, 0x0F, 0xFC, 0xFD, 8, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_deathMenu.item[0].callback = clickLoadMenuFunctor;
	GUI_V2_MENU_ITEM(_deathMenu.item[1], 1, 5, -1, 0x2F, 0xB4, 0x0F, 0xFC, 0xFD, 8, 0xF8, 0xF9, 0xFA, -1, 0, 0, 0, 0);
	_deathMenu.item[1].callback = clickQuitGameFunctor;
	for (int i = 2; i <= 6; ++i)
		_deathMenu.item[i].enabled = false;
	for (int i = 0; i < 2; ++i)
		_deathMenu.item[i].itemId = menuStr[7 * 8 + i + 1];
}

const uint16 GUI_HoF::_menuStringsTalkie[] = {
	0x001, 0x002, 0x003, 0x023, 0x004, 0x025, 0x005, 0x006, // Main Menu String IDs
	0x025, 0x000, 0x000, 0x000, 0x010, 0x000, 0x000, 0x000, // Options Menu String IDs
	0x007, 0x000, 0x000, 0x000, 0x010, 0x000, 0x000, 0x000, // Audio Menu String IDs
	0x000, 0x014, 0x013, 0x000, 0x000, 0x000, 0x000, 0x000, // Menu3 Menu String IDs
	0x008, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x00B, 0x000, // Load Menu String IDs
	0x009, 0x029, 0x02A, 0x02B, 0x02C, 0x02D, 0x00B, 0x000, // Save Menu String IDs
	0x00C, 0x00D, 0x00B, 0x000, 0x000, 0x000, 0x000, 0x000, // Menu6 Menu String IDs
	0x00E, 0x002, 0x005, 0x000, 0x000, 0x000, 0x000, 0x000  // Death Menu String IDs
};

const uint16 GUI_HoF::_menuStringsOther[] = {
	0x009, 0x00A, 0x00B, 0x001, 0x00C, 0x00D, 0x00E, 0x000, // Main Menu String IDs
	0x00F, 0x02B, 0x02C, 0x02D, 0x02E, 0x018, 0x000, 0x000, // Options Menu String IDs
	0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, 0x000, // Dummy
	0x000, 0x01C, 0x01B, 0x000, 0x000, 0x000, 0x000, 0x000, // Menu3 Menu String IDs
	0x010, 0x02F, 0x030, 0x031, 0x032, 0x033, 0x013, 0x000, // Load Menu String IDs
	0x011, 0x02F, 0x030, 0x031, 0x032, 0x033, 0x013, 0x000, // Save Menu String IDs
	0x014, 0x015, 0x013, 0x3E8, 0x000, 0x000, 0x000, 0x000, // Menu6 String IDs
	0x016, 0x00A, 0x00D, 0x000, 0x000, 0x000, 0x000, 0x000  // Death Menu String IDs
};

const uint16 KyraEngine_HoF::_itemMagicTable[] = {
	0x0D,  0x0A,  0x0B,    0,
	0x0D,  0x0B,  0x0A,    0,
	0x0D,  0x38,  0x37,    0,
	0x0D,  0x37,  0x38,    0,
	0x0D,  0x35,  0x36,    0,
	0x0D,  0x36,  0x35,    0,
	0x34,  0x27,  0x33,    0,
	0x41,  0x29,  0x49,    0,
	0x45,  0x29,  0x4A,    1,
	0x46,  0x29,  0x4A,    1,
	0x3C,  0x29,  0x4B,    1,
	0x34,  0x29,  0x4C,    0,
	0x3C,  0x49,  0x3B,    1,
	0x41,  0x4B,  0x3B,    0,
	0x3C,  0x4A,  0x3B,    1,
	0x34,  0x49,  0x3B,    0,
	0x41,  0x4C,  0x3B,    0,
	0x45,  0x4C,  0x3B,    1,
	0x46,  0x4C,  0x3B,    1,
	0x34,  0x4A,  0x3B,    0,
	0x0D,  0x67,  0x68,    0,
	0x0D,  0x68,  0x67,    0,
	0x0D,  0x69,  0x6A,    0,
	0x0D,  0x6A,  0x69,    0,
	0x0D,  0x6B,  0x6C,    0,
	0x0D,  0x6C,  0x6B,    0,
	0x0D,  0x88,  0x87,    0,
	0x0D,  0x87,  0x88,    0,
	0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF
};

const int KyraEngine_HoF::_bookPageYOffset[] = {
	0, 0, 2, 2,
	0, 0, 2, 2,
	0, 0, 2, 2
};

const byte KyraEngine_HoF::_bookTextColorMap[] = {
	0x00, 0xC7, 0xCF, 0x00
};

const int16 KyraEngine_HoF::_cauldronProtectedItems[] = {
	0x07, 0x0D, 0x47, 0x48,
	0x29, 0x1A, 0x1C, 0x6D,
	0x4D, 0x3A, 0x0E, 0x0F,
	0x10, 0x11, 0x26, 0x3E,
	0x35, 0x40, 0x42, 0xA6,
	0xA4, 0xA5, 0x91, 0x95,
	0x99, 0xAC, 0xAE, 0xAF,
	0x8A, 0x79, 0x61, -1
};

const int16 KyraEngine_HoF::_cauldronBowlTable[] = {
	0x0027, 0x0029,
	0x0028, 0x0029,
	0x0033, 0x0029,
	0x0049, 0x0029,
	0x004A, 0x0029,
	0x004B, 0x0029,
	0x004C, 0x0029,
	0x003B, 0x0029,
	0x0034, 0x0034,
	-1, -1
};

const int16 KyraEngine_HoF::_cauldronMagicTable[] = {
	0x0, 0x16, 0x2, 0x1A,
	0x7, 0xA4, 0x5, 0x4D,
	0x1, 0xA5, 0x3, 0xA6,
	0x6, 0x6D, 0x4, 0x91,
	0xA, 0x99, 0xC, 0x95,
	0x9, 0xAC, -1, -1
};

const int16 KyraEngine_HoF::_cauldronMagicTableScene77[] = {
	0x0, 0x16, 0x2, 0x1A,
	0x7, 0xAB, 0x5, 0x4D,
	0x1, 0xAE, 0x3, 0xAF,
	0x6, 0x6D, 0x4, 0x91,
	0xA, 0x99, 0xC, 0x95,
	0x9, 0xAC, -1, -1
};

const uint8 KyraEngine_HoF::_cauldronStateTable[] = {
	3, 1, 3, 1, 1, 4, 4, 2,
	3, 1, 1, 3, 1, 3, 3, 3,
	3, 3, 3, 3, 3, 3, 3
};

const Item KyraEngine_HoF::_flaskTable[] = {
	0x19, 0x14, 0x15, 0x16, 0x17, 0x18, 0x34,
	0x1B, 0x39, 0x1A, 0x3A, 0x4D, 0x72, kItemNone
};

const uint8 KyraEngine_HoF::_rainbowRoomData[] = {
	0x02, 0xA9, 0x9E, 0x75, 0x73, 0x17, 0x00, 0xA0,
	0x08, 0x01, 0x19, 0x9F, 0x66, 0x05, 0x22, 0x7D,
	0x20, 0x25, 0x1D, 0x64, 0xA0, 0x78, 0x85, 0x3B,
	0x3C, 0x5E, 0x38, 0x45, 0x8F, 0x61, 0xA1, 0x71,
	0x47, 0x77, 0x86, 0x41, 0xA2, 0x5F, 0x03, 0x72,
	0x83, 0x9E, 0x84, 0x8E, 0xAD, 0xA8, 0x04, 0x79,
	0xAA, 0xA3, 0x06, 0x27, 0x8F, 0x9F, 0x0A, 0x76,
	0x46, 0x1E, 0x24, 0x63, 0x18, 0x69, 0x39, 0x1F,
	0x7E, 0xAD, 0x28, 0x60, 0x67, 0x21, 0x84, 0x34
};

// kyra 3 static res

const char *const KyraEngine_MR::_languageExtension[] = {
	"TRE",
	"TRF",
	"TRG"/*,
	"TRI",      Italian and Spanish were never included, the supported fan translations are using
	"TRS"       English/French extensions thus overwriting these languages */
};

const int KyraEngine_MR::_languageExtensionSize = ARRAYSIZE(KyraEngine_MR::_languageExtension);

const char *const KyraEngine_MR::_mainMenuSpanishFan[] = {
	"Nueva Partida",
	"Ver Intro",
	"Restaurar",
	"Finalizar"
};

const char *const KyraEngine_MR::_mainMenuItalianFan[] = {
	"Nuova Partita",
	"Introduzione",
	"Carica una partita",
	"Esci dal gioco"
};

const KyraEngine_MR::ShapeDesc KyraEngine_MR::_shapeDescs[] = {
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 82, 96, -43, -86 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 69, 91, -31, -82 },
	{ 57, 91, -31, -82 },
	{ 57, 91, -31, -82 }
};

const int KyraEngine_MR::_shapeDescsSize = ARRAYSIZE(KyraEngine_MR::_shapeDescs);

const uint8 KyraEngine_MR::_characterFrameTable[] = {
	0x36, 0x35, 0x35, 0x33, 0x32, 0x32, 0x34, 0x34
};

const uint8 KyraEngine_MR::_badConscienceFrameTable[] = {
	0x13, 0x13, 0x13, 0x18, 0x13, 0x13, 0x13, 0x13,
	0x13, 0x13, 0x13, 0x10, 0x13, 0x13, 0x13, 0x13,
	0x13, 0x13, 0x13, 0x18, 0x13, 0x13, 0x13, 0x13,
	0x15, 0x15, 0x14, 0x18, 0x14, 0x14, 0x14, 0x14,
	0x24, 0x24, 0x24, 0x24, 0x24, 0x1D, 0x1D, 0x1D
};

const uint8 KyraEngine_MR::_goodConscienceFrameTable[] = {
	0x13, 0x13, 0x13, 0x13, 0x13,
	0x13, 0x13, 0x13, 0x13, 0x13,
	0x13, 0x13, 0x13, 0x13, 0x13,
	0x15, 0x15, 0x15, 0x15, 0x15,
	0x1E, 0x1E, 0x1E, 0x1E, 0x1E
};

const uint8 KyraEngine_MR::_chapterLowestScene[] = {
	0x00, 0x00, 0x19, 0x2B, 0x33, 0x3B
};

const uint8 KyraEngine_MR::_vocHighTable[] = {
	0x64, 0x76, 0x82, 0x83, 0x92
};

const uint8 KyraEngine_MR::_inventoryX[] = {
	0x45, 0x61, 0x7D, 0x99, 0xB5,
	0x45, 0x61, 0x7D, 0x99, 0xB5
};

const uint8 KyraEngine_MR::_inventoryY[] = {
	0x9C, 0x9C, 0x9C, 0x9C, 0x9C,
	0xB2, 0xB2, 0xB2, 0xB2, 0xB2
};

const Item KyraEngine_MR::_trashItemList[] = {
	0x1E, 0x1D, 0x1C, 0x1F, 0x0F, 0x05, 0x04, 0x00,
	0x03, 0x22, 0x0B, 0x20, 0x21, 0x10, 0x11, 0x3A,
	0x39, 0x40, 0x3E, 0x3D, 0x3C, 0x3F, kItemNone
};

const uint8 KyraEngine_MR::_itemStringPickUp[] = {
	0x4, 0x7, 0x0, 0xA
};

const uint8 KyraEngine_MR::_itemStringDrop[] = {
	0x5, 0x8, 0x1, 0xB
};

const uint8 KyraEngine_MR::_itemStringInv[] = {
	0x6, 0x9, 0x2, 0xC
};

void KyraEngine_MR::initMainButtonList(bool disable) {
	if (!_mainButtonListInitialized) {
		_mainButtonData = new Button[14];
		assert(_mainButtonData);

		GUI_V2_BUTTON(_mainButtonData[0], 1, 0, 0, 4, 4, 4, 0x4487, 0,   5, 162, 50, 25, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
		_mainButtonData[0].buttonCallback = BUTTON_FUNCTOR(GUI_MR, _gui, &GUI_MR::optionsButton);
		GUI_V2_BUTTON(_mainButtonData[1], 2, 0, 0, 1, 1, 1, 0x4487, 0, 245, 156, 69, 33, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
		_mainButtonData[1].buttonCallback = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::buttonMoodChange);
		GUI_V2_BUTTON(_mainButtonData[2], 3, 0, 0, 1, 1, 1, 0x4487, 0, 215, 191, 24,  9, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
		_mainButtonData[2].buttonCallback = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::buttonShowScore);
		GUI_V2_BUTTON(_mainButtonData[3], 4, 0, 0, 1, 1, 1, 0x4487, 0, 215, 155, 25, 36, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
		_mainButtonData[3].buttonCallback = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::buttonJesterStaff);

		Button::Callback buttonInventoryFunctor = BUTTON_FUNCTOR(KyraEngine_MR, this, &KyraEngine_MR::buttonInventory);
		for (int i = 0; i < 5; ++i) {
			GUI_V2_BUTTON(_mainButtonData[i + 4], i + 5, 0, 0, 0, 0, 0, 0x1100, 0, 67 + i * 28, 155, 27, 21, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
			_mainButtonData[i + 4].buttonCallback = buttonInventoryFunctor;
		}

		for (int i = 0; i < 5; ++i) {
			GUI_V2_BUTTON(_mainButtonData[i + 9], i + 10, 0, 0, 0, 0, 0, 0x1100, 0, 67 + i * 28, 177, 27, 21, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
			_mainButtonData[i + 9].buttonCallback = buttonInventoryFunctor;
		}

		for (int i = 0; i < 14; ++i)
			_mainButtonList = _gui->addButtonToList(_mainButtonList, &_mainButtonData[i]);

		_mainButtonListInitialized = true;
	}

	for (int i = 0; i < 14; ++i) {
		if (disable)
			_gui->flagButtonDisable(&_mainButtonData[i]);
		else
			_gui->flagButtonEnable(&_mainButtonData[i]);
	}
}

void GUI_MR::initStaticData() {
	GUI_V2_BUTTON(_scrollUpButton, 22, 0, 0, 4, 4, 4, 0x4487, 0, 0, 0, 0x18, 0x0F, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
	GUI_V2_BUTTON(_scrollDownButton, 23, 0, 0, 4, 4, 4, 0x4487, 0, 0, 0, 0x18, 0x0F, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);

	for (int i = 0; i < 4; ++i)
		GUI_V2_BUTTON(_sliderButtons[0][i], 0x18 + i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x0A, 0x0E, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
	for (int i = 0; i < 4; ++i)
		GUI_V2_BUTTON(_sliderButtons[1][i], 0x1C + i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0x0A, 0x0E, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);
	for (int i = 0; i < 4; ++i)
		GUI_V2_BUTTON(_sliderButtons[2][i], 0x20 + i, 0, 0, 0, 0, 0, 0x2200, 0, 0, 0, 0x6E, 0x0E, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);

	for (uint i = 0; i < ARRAYSIZE(_menuButtons); ++i)
		GUI_V2_BUTTON(_menuButtons[i], 0x0F + i, 0, 0, 1, 1, 1, 0x4487, 0, 0, 0, 0, 0, 0xFF, 0xF0, 0xFF, 0xF0, 0xFF, 0xF0, 0);

	Button::Callback clickLoadSlotFunctor = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::clickLoadSlot);
	Button::Callback clickSaveSlotFunctor = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::clickSaveSlot);
	Button::Callback clickLoadMenuFunctor = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::loadMenu);
	Button::Callback clickQuitOptionsFunctor = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::quitOptionsMenu);

	GUI_V2_MENU(_mainMenu, -1, -1, 256, 172, 0xD0, 0xD1, 0xCF, 1, 0xBD, -1, 8, 0, 7, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_mainMenu.item[0], 1, 2, -1, 30, 220, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_mainMenu.item[0].callback = clickLoadMenuFunctor;
	GUI_V2_MENU_ITEM(_mainMenu.item[1], 1, 3, -1, 47, 220, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_mainMenu.item[1].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::saveMenu);
	GUI_V2_MENU_ITEM(_mainMenu.item[2], 1, 35, -1, 64, 220, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_mainMenu.item[2].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::deleteMenu);
	GUI_V2_MENU_ITEM(_mainMenu.item[3], 1, 4, -1, 81, 220, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_mainMenu.item[3].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::gameOptions);
	GUI_V2_MENU_ITEM(_mainMenu.item[4], 1, 37, -1, 98, 220, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_mainMenu.item[4].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::audioOptions);
	GUI_V2_MENU_ITEM(_mainMenu.item[5], 1, 5, -1, 115, 220, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_mainMenu.item[5].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::quitGame);
	GUI_V2_MENU_ITEM(_mainMenu.item[6], 1, 6, -1, 144, 220, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_mainMenu.item[6].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::resumeGame);

	GUI_V2_MENU(_audioOptions, -1, -1, 288, 136, 0xD0, 0xD1, 0xCF, 37, 0xBD, -1, 8, 4, 5, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_audioOptions.item[0], 0, 0, 160, 30, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 23, 8, 32, 0x0000);
	GUI_V2_MENU_ITEM(_audioOptions.item[1], 0, 0, 160, 47, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 24, 8, 49, 0x0000);
	GUI_V2_MENU_ITEM(_audioOptions.item[2], 0, 0, 160, 64, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 39, 8, 66, 0x0000);
	GUI_V2_MENU_ITEM(_audioOptions.item[3], 1, 0, 152, 81, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 47, 8, 83, 0x0000);
	_audioOptions.item[3].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::toggleHeliumMode);
	GUI_V2_MENU_ITEM(_audioOptions.item[4], 1, 16, -1, 110, 92, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_audioOptions.item[4].callback = clickQuitOptionsFunctor;
	for (int i = 5; i < 7; ++i)
		_audioOptions.item[i].enabled = false;

	GUI_V2_MENU(_gameOptions, -1, -1, 288, 154, 0xD0, 0xD1, 0xCF, 7, 0xBD, -1, 8, 0, 6, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_gameOptions.item[0], 1, 0, 160, 30, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 21, 8, 32, 0x0000);
	_gameOptions.item[0].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::toggleWalkspeed);
	GUI_V2_MENU_ITEM(_gameOptions.item[1], 1, 0, 160, 47, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 26, 8, 49, 0x0000);
	_gameOptions.item[1].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::changeLanguage);
	GUI_V2_MENU_ITEM(_gameOptions.item[2], 1, 0, 160, 64, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 40, 8, 66, 0x0000);
	_gameOptions.item[2].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::toggleStudioSFX);
	GUI_V2_MENU_ITEM(_gameOptions.item[3], 1, 0, 160, 81, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 46, 8, 83, 0x0000);
	_gameOptions.item[3].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::toggleSkipSupport);
	GUI_V2_MENU_ITEM(_gameOptions.item[4], 1, 0, 160, 98, 116, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 22, 8, 100, 0x0000);
	_gameOptions.item[4].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::toggleText);
	GUI_V2_MENU_ITEM(_gameOptions.item[5], 1, 16, -1, 127, 125, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_gameOptions.item[5].callback = clickQuitOptionsFunctor;
	_gameOptions.item[6].enabled = false;

	GUI_V2_MENU(_choiceMenu, -1, -1, 320, 56, 0xD0, 0xD1, 0xCF, 0, 0xBA, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_choiceMenu.item[0], 1, 20, 24, 30, 72, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_choiceMenu.item[0].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::choiceYes);
	GUI_V2_MENU_ITEM(_choiceMenu.item[1], 1, 19, 216, 30, 72, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_choiceMenu.item[1].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::choiceNo);
	for (int i = 2; i < 7; ++i)
		_choiceMenu.item[i].enabled = false;

	GUI_V2_MENU(_loadMenu, -1, -1, 288, 160, 0xD0, 0xD1, 0xCF, 8, 0xBD, -1, 8, 0, 6, 132, 22, 132, 124);
	GUI_V2_MENU_ITEM(_loadMenu.item[0], 1, 41, -1, 39, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	GUI_V2_MENU_ITEM(_loadMenu.item[1], 1, 42, -1, 56, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	GUI_V2_MENU_ITEM(_loadMenu.item[2], 1, 43, -1, 73, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	GUI_V2_MENU_ITEM(_loadMenu.item[3], 1, 44, -1, 90, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	GUI_V2_MENU_ITEM(_loadMenu.item[4], 1, 45, -1, 107, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	for (int i = 0; i <= 4; ++i)
		_loadMenu.item[i].callback = clickLoadSlotFunctor;
	GUI_V2_MENU_ITEM(_loadMenu.item[5], 1, 11, 184, 134, 88, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_loadMenu.item[5].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::cancelLoadMenu);
	_loadMenu.item[6].enabled = false;

	GUI_V2_MENU(_saveMenu, -1, -1, 288, 160, 0xD0, 0xD1, 0xCF, 9, 0xBD, -1, 8, 0, 6, 132, 22, 132, 124);
	GUI_V2_MENU_ITEM(_saveMenu.item[0], 1, 41, -1, 39, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	GUI_V2_MENU_ITEM(_saveMenu.item[1], 1, 42, -1, 56, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	GUI_V2_MENU_ITEM(_saveMenu.item[2], 1, 43, -1, 73, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	GUI_V2_MENU_ITEM(_saveMenu.item[3], 1, 44, -1, 90, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	GUI_V2_MENU_ITEM(_saveMenu.item[4], 1, 45, -1, 107, 256, 15, 0xFA, 0xFF, 5, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	for (int i = 0; i <= 4; ++i)
		_saveMenu.item[i].callback = clickSaveSlotFunctor;
	GUI_V2_MENU_ITEM(_saveMenu.item[5], 1, 11, 184, 134, 88, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_saveMenu.item[5].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::cancelSaveMenu);
	_saveMenu.item[6].enabled = false;

	GUI_V2_MENU(_savenameMenu, -1, -1, 320, 67, 0xD0, 0xD1, 0xCF, 12, 0xBD, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_savenameMenu.item[0], 1, 13, 24, 44, 88, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_savenameMenu.item[0].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::finishSavename);
	GUI_V2_MENU_ITEM(_savenameMenu.item[1], 1, 11, 208, 44, 88, 15, 0xFA, 0xFF, -1, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_savenameMenu.item[1].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::cancelSavename);
	for (int i = 2; i < 7; ++i)
		_savenameMenu.item[i].enabled = false;

	GUI_V2_MENU(_deathMenu, -1, -1, 208, 76, 0xD0, 0xD1, 0xCF, 14, 0xBD, -1, 8, 0, 2, -1, -1, -1, -1);
	GUI_V2_MENU_ITEM(_deathMenu.item[0], 1, 2, -1, 30, 180, 15, 0xFA, 0xFF, 8, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_deathMenu.item[0].callback = clickLoadMenuFunctor;
	GUI_V2_MENU_ITEM(_deathMenu.item[1], 1, 38, -1, 47, 180, 15, 0xFA, 0xFF, 8, 0xD0, 0xD1, 0xCF, -1, 0, 0, 0, 0x0000);
	_deathMenu.item[1].callback = BUTTON_FUNCTOR(GUI_MR, this, &GUI_MR::loadSecondChance);
	for (int i = 2; i < 7; ++i)
		_deathMenu.item[i].enabled = false;
}

const int8 KyraEngine_MR::_albumWSAX[] = {
	 0, 77, -50, 99, -61, 82, -58, 85,
	-64, 80, -63, 88, -63, 88, -64,  0
};

const int8 KyraEngine_MR::_albumWSAY[] = {
	 0, -1, 3, 0, -1,  0, -2, 0,
	-1, -2, 2, 2, -6, -6, -6, 0
};

} // End of namespace Kyra
