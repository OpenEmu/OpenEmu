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


#include "common/textconsole.h"
#include "common/savefile.h"

#include "touche/graphics.h"
#include "touche/touche.h"

namespace Touche {

enum {
	kGameStateDescriptionLen = 32,
	kCurrentGameStateVersion = 6
};

static void saveOrLoad(Common::WriteStream &stream, uint16 &i) {
	stream.writeUint16LE(i);
}

static void saveOrLoad(Common::ReadStream &stream, uint16 &i) {
	i = stream.readUint16LE();
}

static void saveOrLoad(Common::WriteStream &stream, int16 &i) {
	stream.writeSint16LE(i);
}

static void saveOrLoad(Common::ReadStream &stream, int16 &i) {
	i = stream.readSint16LE();
}

static void saveOrLoadPtr(Common::WriteStream &stream, int16 *&p, int16 *base) {
	int32 offset = (int32)(p - base);
	stream.writeSint32LE(offset);
}

static void saveOrLoadPtr(Common::ReadStream &stream, int16 *&p, int16 *base) {
	int32 offset = stream.readSint32LE();
	p = base + offset;
}

template<class S>
static void saveOrLoad(S &s, Common::Rect &r) {
	saveOrLoad(s, r.left);
	saveOrLoad(s, r.top);
	saveOrLoad(s, r.right);
	saveOrLoad(s, r.bottom);
}

template<class S>
static void saveOrLoad(S &s, SequenceEntry &seq) {
	saveOrLoad(s, seq.sprNum);
	saveOrLoad(s, seq.seqNum);
}

template<class S>
static void saveOrLoad(S &s, KeyChar &key) {
	saveOrLoad(s, key.num);
	saveOrLoad(s, key.flags);
	saveOrLoad(s, key.currentAnimCounter);
	saveOrLoad(s, key.strNum);
	saveOrLoad(s, key.walkDataNum);
	saveOrLoad(s, key.spriteNum);
	saveOrLoad(s, key.prevBoundingRect);
	saveOrLoad(s, key.boundingRect);
	saveOrLoad(s, key.xPos);
	saveOrLoad(s, key.yPos);
	saveOrLoad(s, key.zPos);
	saveOrLoad(s, key.xPosPrev);
	saveOrLoad(s, key.yPosPrev);
	saveOrLoad(s, key.zPosPrev);
	saveOrLoad(s, key.prevWalkDataNum);
	saveOrLoad(s, key.textColor);
	for (uint i = 0; i < 4; ++i) {
		saveOrLoad(s, key.inventoryItems[i]);
	}
	saveOrLoad(s, key.money);
	saveOrLoad(s, key.pointsDataNum);
	saveOrLoad(s, key.currentWalkBox);
	saveOrLoad(s, key.prevPointsDataNum);
	saveOrLoad(s, key.currentAnim);
	saveOrLoad(s, key.facingDirection);
	saveOrLoad(s, key.currentAnimSpeed);
	for (uint i = 0; i < 16; ++i) {
		saveOrLoad(s, key.framesList[i]);
	}
	saveOrLoad(s, key.framesListCount);
	saveOrLoad(s, key.currentFrame);
	saveOrLoad(s, key.anim1Start);
	saveOrLoad(s, key.anim1Count);
	saveOrLoad(s, key.anim2Start);
	saveOrLoad(s, key.anim2Count);
	saveOrLoad(s, key.anim3Start);
	saveOrLoad(s, key.anim3Count);
	saveOrLoad(s, key.followingKeyCharNum);
	saveOrLoad(s, key.followingKeyCharPos);
	saveOrLoad(s, key.sequenceDataIndex);
	saveOrLoad(s, key.sequenceDataOffset);
	saveOrLoad(s, key.walkPointsListIndex);
	for (uint i = 0; i < 40; ++i) {
		saveOrLoad(s, key.walkPointsList[i]);
	}
	saveOrLoad(s, key.scriptDataStartOffset);
	saveOrLoad(s, key.scriptDataOffset);
	saveOrLoadPtr(s, key.scriptStackPtr, &key.scriptStackTable[39]);
	saveOrLoad(s, key.delay);
	saveOrLoad(s, key.waitingKeyChar);
	for (uint i = 0; i < 3; ++i) {
		saveOrLoad(s, key.waitingKeyCharPosTable[i]);
	}
	for (uint i = 0; i < 40; ++i) {
		saveOrLoad(s, key.scriptStackTable[i]);
	}
}

template<class S>
static void saveOrLoad(S &s, TalkEntry &entry) {
	saveOrLoad(s, entry.otherKeyChar);
	saveOrLoad(s, entry.talkingKeyChar);
	saveOrLoad(s, entry.num);
}

template<class S>
static void saveOrLoad(S &s, ProgramHitBoxData &data) {
	saveOrLoad(s, data.item);
	saveOrLoad(s, data.talk);
	saveOrLoad(s, data.state);
	saveOrLoad(s, data.str);
	saveOrLoad(s, data.defaultStr);
	for (uint i = 0; i < 8; ++i) {
		saveOrLoad(s, data.actions[i]);
	}
	for (uint i = 0; i < 2; ++i) {
		saveOrLoad(s, data.hitBoxes[i]);
	}
}

template<class S>
static void saveOrLoad(S &s, Area &area) {
	saveOrLoad(s, area.r);
	saveOrLoad(s, area.srcX);
	saveOrLoad(s, area.srcY);
}

template<class S>
static void saveOrLoad(S &s, ProgramBackgroundData &data) {
	saveOrLoad(s, data.area);
	saveOrLoad(s, data.type);
	saveOrLoad(s, data.offset);
	saveOrLoad(s, data.scaleMul);
	saveOrLoad(s, data.scaleDiv);
}

template<class S>
static void saveOrLoad(S &s, ProgramAreaData &data) {
	saveOrLoad(s, data.area);
	saveOrLoad(s, data.id);
	saveOrLoad(s, data.state);
	saveOrLoad(s, data.animCount);
	saveOrLoad(s, data.animNext);
}

template<class S>
static void saveOrLoad(S &s, ProgramWalkData &data) {
	saveOrLoad(s, data.point1);
	saveOrLoad(s, data.point2);
	saveOrLoad(s, data.clippingRect);
	saveOrLoad(s, data.area1);
	saveOrLoad(s, data.area2);
}

template<class S>
static void saveOrLoad(S &s, ProgramPointData &data) {
	saveOrLoad(s, data.x);
	saveOrLoad(s, data.y);
	saveOrLoad(s, data.z);
	saveOrLoad(s, data.order);
}

template<class A>
static void saveOrLoadCommonArray(Common::WriteStream &stream, A &array) {
	uint count = array.size();
	assert(count < 0xFFFF);
	stream.writeUint16LE(count);
	for (uint i = 0; i < count; ++i) {
		saveOrLoad(stream, array[i]);
	}
}

template<class A>
static void saveOrLoadCommonArray(Common::ReadStream &stream, A &array) {
	uint count = stream.readUint16LE();
	if (count == array.size()) {
		for (uint i = 0; i < count; ++i) {
			saveOrLoad(stream, array[i]);
		}
	}
}

template<class S, class A>
static void saveOrLoadStaticArray(S &s, A &array, uint count) {
	for (uint i = 0; i < count; ++i) {
		saveOrLoad(s, array[i]);
	}
}

static const uint32 saveLoadEndMarker = 0x55AA55AA;

void ToucheEngine::saveGameStateData(Common::WriteStream *stream) {
	setKeyCharMoney();
	stream->writeUint16LE(_currentEpisodeNum);
	stream->writeUint16LE(_currentMusicNum);
	stream->writeUint16LE(_currentRoomNum);
	stream->writeUint16LE(_flagsTable[614]);
	stream->writeUint16LE(_flagsTable[615]);
	stream->writeUint16LE(_disabledInputCounter);
	saveOrLoadCommonArray(*stream, _programHitBoxTable);
	saveOrLoadCommonArray(*stream, _programBackgroundTable);
	saveOrLoadCommonArray(*stream, _programAreaTable);
	saveOrLoadCommonArray(*stream, _programWalkTable);
	saveOrLoadCommonArray(*stream, _programPointsTable);
	stream->write(_updatedRoomAreasTable, 200);
	saveOrLoadStaticArray(*stream, _sequenceEntryTable, NUM_SEQUENCES);
	saveOrLoadStaticArray(*stream, _flagsTable, 1024);
	saveOrLoadStaticArray(*stream, _inventoryList1, 100);
	saveOrLoadStaticArray(*stream, _inventoryList2, 100);
	saveOrLoadStaticArray(*stream, _inventoryList3, 6);
	saveOrLoadStaticArray(*stream, _keyCharsTable, NUM_KEYCHARS);
	saveOrLoadStaticArray(*stream, _inventoryItemsInfoTable, NUM_INVENTORY_ITEMS);
	saveOrLoadStaticArray(*stream, _talkTable, NUM_TALK_ENTRIES);
	stream->writeUint16LE(_talkListEnd);
	stream->writeUint16LE(_talkListCurrent);
	stream->writeUint32LE(saveLoadEndMarker);
}

void ToucheEngine::loadGameStateData(Common::ReadStream *stream) {
	setKeyCharMoney();
	clearDirtyRects();
	_flagsTable[115] = 0;
	clearRoomArea();
	_currentEpisodeNum = stream->readUint16LE();
	_newMusicNum = stream->readUint16LE();
	_currentRoomNum = stream->readUint16LE();
	res_loadRoom(_currentRoomNum);
	int16 roomOffsX = _flagsTable[614] = stream->readUint16LE();
	int16 roomOffsY = _flagsTable[615] = stream->readUint16LE();
	_disabledInputCounter = stream->readUint16LE();
	res_loadProgram(_currentEpisodeNum);
	setupEpisode(-1);
	saveOrLoadCommonArray(*stream, _programHitBoxTable);
	saveOrLoadCommonArray(*stream, _programBackgroundTable);
	saveOrLoadCommonArray(*stream, _programAreaTable);
	saveOrLoadCommonArray(*stream, _programWalkTable);
	saveOrLoadCommonArray(*stream, _programPointsTable);
	stream->read(_updatedRoomAreasTable, 200);
	for (uint i = 1; i < _updatedRoomAreasTable[0]; ++i) {
		updateRoomAreas(_updatedRoomAreasTable[i], -1);
	}
	saveOrLoadStaticArray(*stream, _sequenceEntryTable, NUM_SEQUENCES);
	saveOrLoadStaticArray(*stream, _flagsTable, 1024);
	saveOrLoadStaticArray(*stream, _inventoryList1, 100);
	saveOrLoadStaticArray(*stream, _inventoryList2, 100);
	saveOrLoadStaticArray(*stream, _inventoryList3, 6);
	saveOrLoadStaticArray(*stream, _keyCharsTable, NUM_KEYCHARS);
	saveOrLoadStaticArray(*stream, _inventoryItemsInfoTable, NUM_INVENTORY_ITEMS);
	saveOrLoadStaticArray(*stream, _talkTable, NUM_TALK_ENTRIES);
	_talkListEnd = stream->readUint16LE();
	_talkListCurrent = stream->readUint16LE();
	if (stream->readUint32LE() != saveLoadEndMarker) {
		warning("Corrupted gamestate data");
		// if that ever happens, exit the game
		quitGame();
	}
	_flagsTable[614] = roomOffsX;
	_flagsTable[615] = roomOffsY;
	for (uint i = 0; i < NUM_SEQUENCES; ++i) {
		if (_sequenceEntryTable[i].seqNum != -1) {
			res_loadSequence(_sequenceEntryTable[i].seqNum, i);
		}
		if (_sequenceEntryTable[i].sprNum != -1) {
			res_loadSprite(_sequenceEntryTable[i].sprNum, i);
		}
	}
	_currentKeyCharNum = _flagsTable[104];
	_inventoryStateTable[0].displayOffset = 0;
	_inventoryStateTable[1].displayOffset = 0;
	_inventoryStateTable[2].displayOffset = 0;
	drawInventory(_currentKeyCharNum, 1);
	Graphics::copyRect(_offscreenBuffer, kScreenWidth, 0, 0,
	  _backdropBuffer, _currentBitmapWidth, _flagsTable[614], _flagsTable[615],
	  kScreenWidth, kRoomHeight);
	updateRoomRegions();
	_fullRedrawCounter = 1;
	_roomNeedRedraw = false;
	if (_flagsTable[617] != 0) {
		res_loadSpeech(_flagsTable[617]);
	}
	debug(0, "Loaded state, current episode %d", _currentEpisodeNum);
}

Common::Error ToucheEngine::saveGameState(int num, const Common::String &description) {
	bool saveOk = false;
	Common::String gameStateFileName = generateGameStateFileName(_targetName.c_str(), num);
	Common::OutSaveFile *f = _saveFileMan->openForSaving(gameStateFileName);
	if (f) {
		f->writeUint16LE(kCurrentGameStateVersion);
		f->writeUint16LE(0);
		char headerDescription[kGameStateDescriptionLen];
		memset(headerDescription, 0, kGameStateDescriptionLen);
		strncpy(headerDescription, description.c_str(), kGameStateDescriptionLen - 1);
		f->write(headerDescription, kGameStateDescriptionLen);
		saveGameStateData(f);
		f->finalize();
		if (!f->err()) {
			saveOk = true;
		} else {
			warning("Can't write file '%s'", gameStateFileName.c_str());
		}
		delete f;
	}
	return saveOk ? Common::kNoError : Common::kUnknownError;
}

Common::Error ToucheEngine::loadGameState(int num) {
	bool loadOk = false;
	Common::String gameStateFileName = generateGameStateFileName(_targetName.c_str(), num);
	Common::InSaveFile *f = _saveFileMan->openForLoading(gameStateFileName);
	if (f) {
		uint16 version = f->readUint16LE();
		if (version < kCurrentGameStateVersion) {
			warning("Unsupported gamestate version %d (index %d)", version, num);
		} else {
			f->skip(2 + kGameStateDescriptionLen);
			loadGameStateData(f);
			if (f->err() || f->eos()) {
				warning("Can't read file '%s'", gameStateFileName.c_str());
			} else {
				loadOk = true;
			}
		}
		delete f;
	}
	return loadOk ? Common::kNoError : Common::kUnknownError;
}

void readGameStateDescription(Common::ReadStream *f, char *description, int len) {
	uint16 version = f->readUint16LE();
	if (version >= kCurrentGameStateVersion) {
		f->readUint16LE();
		f->read(description, MIN<int>(len, kGameStateDescriptionLen));
		description[len] = 0;
	} else {
		description[0] = 0;
	}
}

Common::String generateGameStateFileName(const char *target, int slot, bool prefixOnly) {
	Common::String name(target);
	if (prefixOnly) {
		name += ".*";
	} else {
		name += Common::String::format(".%d", slot);
	}
	return name;
}

int getGameStateFileSlot(const char *filename) {
	int i = -1;
	const char *slot = strrchr(filename, '.');
	if (slot) {
		i = atoi(slot + 1);
	}
	return i;
}

} // namespace Touche
