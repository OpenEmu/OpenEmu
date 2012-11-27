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

#include "audio/decoders/flac.h"
#include "audio/mixer.h"
#include "audio/decoders/mp3.h"
#include "audio/decoders/voc.h"
#include "audio/decoders/vorbis.h"
#include "audio/decoders/raw.h"
#include "audio/audiostream.h"

#include "touche/midi.h"
#include "touche/touche.h"
#include "touche/graphics.h"

namespace Touche {

enum {
	kCurrentSpeechDataVersion = 1,
	kSpeechDataFileHeaderSize = 4
};

struct CompressedSpeechFile {
	const char *filename;
	Audio::SeekableAudioStream *(*makeStream)(
			Common::SeekableReadStream *stream,
			DisposeAfterUse::Flag disposeAfterUse);
};

static const CompressedSpeechFile compressedSpeechFilesTable[] = {
#ifdef USE_FLAC
	{ "TOUCHE.SOF", Audio::makeFLACStream },
#endif
#ifdef USE_VORBIS
	{ "TOUCHE.SOG", Audio::makeVorbisStream },
#endif
#ifdef USE_MAD
	{ "TOUCHE.SO3", Audio::makeMP3Stream },
#endif
	{ 0, 0 }
};

void ToucheEngine::res_openDataFile() {
	if (!_fData.open("TOUCHE.DAT")) {
		error("Unable to open 'TOUCHE.DAT' for reading");
	}
	for (int i = 0; compressedSpeechFilesTable[i].filename; ++i) {
		if (_fSpeech[0].open(compressedSpeechFilesTable[i].filename)) {
			int version = _fSpeech[0].readUint16LE();
			if (version == kCurrentSpeechDataVersion) {
				_compressedSpeechData = i;
				return;
			}
			warning("Unhandled version %d for compressed sound file '%s'", version, compressedSpeechFilesTable[i].filename);
			return;
		}
	}
	// _fSpeech[0] opening/closing is driven by the scripts
	_fSpeech[1].open("OBJ");
	_compressedSpeechData = -1;
}

void ToucheEngine::res_closeDataFile() {
	_fData.close();
	_fSpeech[0].close();
	_fSpeech[1].close();
}

void ToucheEngine::res_allocateTables() {
	_fData.seek(64);
	uint32 textDataOffs = _fData.readUint32LE();
	uint32 textDataSize = _fData.readUint32LE();
	_textData = (uint8 *)malloc(textDataSize);
	if (!_textData) {
		error("Unable to allocate memory for text data");
	}
	_fData.seek(textDataOffs);
	_fData.read(_textData, textDataSize);

	_fData.seek(2);
	const int bw = _fData.readUint16LE();
	const int bh = _fData.readUint16LE();
	uint32 size = bw * bh;
	_backdropBuffer = (uint8 *)malloc(size);
	if (!_backdropBuffer) {
		error("Unable to allocate memory for backdrop buffer");
	}

	_menuKitData = (uint8 *)malloc(42 * 120);
	if (!_menuKitData) {
		error("Unable to allocate memory for menu kit data");
	}

	_convKitData = (uint8 *)malloc(152 * 80);
	if (!_convKitData) {
		error("Unable to allocate memory for conv kit data");
	}

	for (int i = 0; i < NUM_SEQUENCES; ++i) {
		_sequenceDataTable[i] = (uint8 *)malloc(16384);
		if (!_sequenceDataTable[i]) {
			error("Unable to allocate memory for sequence data %d", i);
		}
	}

	_programData = (uint8 *)malloc(kMaxProgramDataSize);
	if (!_programData) {
		error("Unable to allocate memory for program data");
	}

	_mouseData = (uint8 *)malloc(kCursorWidth * kCursorHeight);
	if (!_mouseData) {
		error("Unable to allocate memory for mouse data");
	}

	_iconData = (uint8 *)malloc(kIconWidth * kIconHeight);
	if (!_iconData) {
		error("Unable to allocate memory for object data");
	}

	memset(_spritesTable, 0, sizeof(_spritesTable));

	_offscreenBuffer = (uint8 *)malloc(kScreenWidth * kScreenHeight);
	if (!_offscreenBuffer) {
		error("Unable to allocate memory for offscreen buffer");
	}
}

void ToucheEngine::res_deallocateTables() {
	free(_textData);
	_textData = 0;

	free(_backdropBuffer);
	_backdropBuffer = 0;

	free(_menuKitData);
	_menuKitData = 0;

	free(_convKitData);
	_convKitData = 0;

	for (int i = 0; i < NUM_SEQUENCES; ++i) {
		free(_sequenceDataTable[i]);
		_sequenceDataTable[i] = 0;
	}

	free(_programData);
	_programData = 0;

	free(_mouseData);
	_mouseData = 0;

	free(_iconData);
	_iconData = 0;

	for (int i = 0; i < NUM_SPRITES; ++i) {
		free(_spritesTable[i].ptr);
		_spritesTable[i].ptr = 0;
	}

	free(_offscreenBuffer);
	_offscreenBuffer = 0;
}

uint32 ToucheEngine::res_getDataOffset(ResourceType type, int num, uint32 *size) {
	debugC(9, kDebugResource, "ToucheEngine::res_getDataOffset() type=%d num=%d", type, num);
	static const struct ResourceData {
		int offs;
		int count;
		int type;
	} dataTypesTable[] = {
		{ 0x048, 100, kResourceTypeRoomImage   },
		{ 0x228,  30, kResourceTypeSequence    },
		{ 0x2A0,  50, kResourceTypeSpriteImage },
		{ 0x390, 100, kResourceTypeIconImage   },
		{ 0x6B0,  80, kResourceTypeRoomInfo    },
		{ 0x908, 150, kResourceTypeProgram     },
		{ 0xB60,  50, kResourceTypeMusic       },
		{ 0xC28, 120, kResourceTypeSound       }
	};

	const ResourceData *rd = NULL;
	for (unsigned int i = 0; i < ARRAYSIZE(dataTypesTable); ++i) {
		if (dataTypesTable[i].type == type) {
			rd = &dataTypesTable[i];
			break;
		}
	}
	if (rd == NULL) {
		error("Invalid resource type %d", type);
	}
	if (num < 0 || num > rd->count) {
		error("Invalid resource number %d (type %d)", num, type);
	}
	_fData.seek(rd->offs + num * 4);
	uint32 offs = _fData.readUint32LE();
	assert(offs != 0);
	if (size) {
		uint32 nextOffs = _fData.readUint32LE();
		*size = nextOffs - offs;
	}
	return offs;
}

void ToucheEngine::res_loadSpriteImage(int num, uint8 *dst) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadSpriteImage() num=%d", num);
	const uint32 offs = res_getDataOffset(kResourceTypeSpriteImage, num);
	_fData.seek(offs);
	_currentImageWidth = _fData.readUint16LE();
	_currentImageHeight = _fData.readUint16LE();
	for (int i = 0; i < _currentImageHeight; ++i) {
		res_decodeScanLineImageRLE(dst + _currentImageWidth * i, _currentImageWidth);
	}
}

void ToucheEngine::res_loadProgram(int num) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadProgram() num=%d", num);
	const uint32 offs = res_getDataOffset(kResourceTypeProgram, num, &_programDataSize);
	_fData.seek(offs);
	assert(_programDataSize <= kMaxProgramDataSize);
	_fData.read(_programData, _programDataSize);
}

void ToucheEngine::res_decodeProgramData() {
	debugC(9, kDebugResource, "ToucheEngine::res_decodeProgramData()");

	uint8 *p;
	uint8 *programDataEnd = _programData + _programDataSize;

	p = _programData + READ_LE_UINT32(_programData + 32);
	_script.init(p);

	p = _programData + READ_LE_UINT32(_programData + 4);
	_programTextDataPtr = p;

	_programRectsTable.clear();
	p = _programData + READ_LE_UINT32(_programData + 20);
	while (p < programDataEnd) {
		int16 x = READ_LE_UINT16(p); p += 2;
		int16 y = READ_LE_UINT16(p); p += 2;
		int16 w = READ_LE_UINT16(p); p += 2;
		int16 h = READ_LE_UINT16(p); p += 2;
		_programRectsTable.push_back(Common::Rect(x, y, x + w, y + h));
		if (x == -1) {
			break;
		}
	}

	_programPointsTable.clear();
	p = _programData + READ_LE_UINT32(_programData + 24);
	while (p < programDataEnd) {
		ProgramPointData ppd;
		ppd.x = READ_LE_UINT16(p); p += 2;
		ppd.y = READ_LE_UINT16(p); p += 2;
		ppd.z = READ_LE_UINT16(p); p += 2;
		ppd.order = READ_LE_UINT16(p); p += 2;
		_programPointsTable.push_back(ppd);
		if (ppd.x == -1) {
			break;
		}
	}

	_programWalkTable.clear();
	p = _programData + READ_LE_UINT32(_programData + 28);
	while (p < programDataEnd) {
		ProgramWalkData pwd;
		pwd.point1 = READ_LE_UINT16(p); p += 2;
		if (pwd.point1 == -1) {
			break;
		}
		assert((uint16)pwd.point1 < _programPointsTable.size());
		pwd.point2 = READ_LE_UINT16(p); p += 2;
		assert((uint16)pwd.point2 < _programPointsTable.size());
		pwd.clippingRect = READ_LE_UINT16(p); p += 2;
		pwd.area1 = READ_LE_UINT16(p); p += 2;
		pwd.area2 = READ_LE_UINT16(p); p += 2;
		p += 12; // unused
		_programWalkTable.push_back(pwd);
	}

	_programAreaTable.clear();
	p = _programData + READ_LE_UINT32(_programData + 8);
	while (p < programDataEnd) {
		ProgramAreaData pad;
		int16 x = READ_LE_UINT16(p); p += 2;
		if (x == -1) {
			break;
		}
		int16 y = READ_LE_UINT16(p); p += 2;
		int16 w = READ_LE_UINT16(p); p += 2;
		int16 h = READ_LE_UINT16(p); p += 2;
		pad.area.r = Common::Rect(x, y, x + w, y + h);
		pad.area.srcX = READ_LE_UINT16(p); p += 2;
		pad.area.srcY = READ_LE_UINT16(p); p += 2;
		pad.id = READ_LE_UINT16(p); p += 2;
		pad.state = READ_LE_UINT16(p); p += 2;
		pad.animCount = READ_LE_UINT16(p); p += 2;
		pad.animNext = READ_LE_UINT16(p); p += 2;
		_programAreaTable.push_back(pad);
	}

	_programBackgroundTable.clear();
	p = _programData + READ_LE_UINT32(_programData + 12);
	while (p < programDataEnd) {
		ProgramBackgroundData pbd;
		int16 x = READ_LE_UINT16(p); p += 2;
		if (x == -1) {
			break;
		}
		int16 y = READ_LE_UINT16(p); p += 2;
		int16 w = READ_LE_UINT16(p); p += 2;
		int16 h = READ_LE_UINT16(p); p += 2;
		pbd.area.r = Common::Rect(x, y, x + w, y + h);
		pbd.area.srcX = READ_LE_UINT16(p); p += 2;
		pbd.area.srcY = READ_LE_UINT16(p); p += 2;
		pbd.type = READ_LE_UINT16(p); p += 2;
		pbd.offset = READ_LE_UINT16(p); p += 2;
		pbd.scaleMul = READ_LE_UINT16(p); p += 2;
		pbd.scaleDiv = READ_LE_UINT16(p); p += 2;
		_programBackgroundTable.push_back(pbd);
	}

	_programHitBoxTable.clear();
	p = _programData + READ_LE_UINT32(_programData + 16);
	while (p < programDataEnd) {
		ProgramHitBoxData phbd;
		phbd.item = READ_LE_UINT16(p); p += 2;
		if (phbd.item == 0) {
			break;
		}
		phbd.talk = READ_LE_UINT16(p); p += 2;
		phbd.state = READ_LE_UINT16(p); p += 2;
		phbd.str = READ_LE_UINT16(p); p += 2;
		phbd.defaultStr = READ_LE_UINT16(p); p += 2;
		for (int i = 0; i < 8; ++i) {
			phbd.actions[i] = READ_LE_UINT16(p); p += 2;
		}
		for (int i = 0; i < 2; ++i) {
			int16 x = READ_LE_UINT16(p); p += 2;
			int16 y = READ_LE_UINT16(p); p += 2;
			int16 w = READ_LE_UINT16(p); p += 2;
			int16 h = READ_LE_UINT16(p); p += 2;
			phbd.hitBoxes[i].left = x;
			phbd.hitBoxes[i].top = y;
			phbd.hitBoxes[i].right = x + w;
			phbd.hitBoxes[i].bottom = y + h;
		}
		p += 8; // unused
		_programHitBoxTable.push_back(phbd);
	}

	_programActionScriptOffsetTable.clear();
	p = _programData + READ_LE_UINT32(_programData + 36);
	while (p < programDataEnd) {
		ProgramActionScriptOffsetData pasod;
		pasod.object1 = READ_LE_UINT16(p); p += 2;
		if (pasod.object1 == 0) {
			break;
		}
		pasod.action = READ_LE_UINT16(p); p += 2;
		pasod.object2 = READ_LE_UINT16(p); p += 2;
		pasod.offset = READ_LE_UINT16(p); p += 2;
		_programActionScriptOffsetTable.push_back(pasod);
	}

	_programConversationTable.clear();
	int count = (READ_LE_UINT32(_programData + 44) - READ_LE_UINT32(_programData + 40)) / 6;
	assert(count >= 0);
	p = _programData + READ_LE_UINT32(_programData + 40);
	while (p < programDataEnd && count != 0) {
		ProgramConversationData pcd;
		pcd.num = READ_LE_UINT16(p); p += 2;
		pcd.offset = READ_LE_UINT16(p); p += 2;
		pcd.msg = READ_LE_UINT16(p); p += 2;
		_programConversationTable.push_back(pcd);
		--count;
	}

	_programKeyCharScriptOffsetTable.clear();
	p = _programData + READ_LE_UINT32(_programData + 44);
	while (p < programDataEnd) {
		ProgramKeyCharScriptOffsetData pksod;
		pksod.keyChar = READ_LE_UINT16(p); p += 2;
		if (pksod.keyChar == 0) {
			break;
		}
		pksod.offset = READ_LE_UINT16(p); p += 2;
		_programKeyCharScriptOffsetTable.push_back(pksod);
	}
}

void ToucheEngine::res_loadRoom(int num) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadRoom() num=%d flag115=%d", num, _flagsTable[115]);

	debug(0, "Setting up room %d", num);

	const uint32 offsInfo = res_getDataOffset(kResourceTypeRoomInfo, num);
	_fData.seek(offsInfo);
	_fData.skip(2);
	const int roomImageNum = _fData.readUint16LE();
	_fData.skip(2);
	_fData.read(_paletteBuffer, 3 * 256);

	const uint32 offsImage = res_getDataOffset(kResourceTypeRoomImage, roomImageNum);
	_fData.seek(offsImage);
	res_loadBackdrop();

	bool updateScreenPalette = _flagsTable[115] == 0;

	// Workaround to what appears to be a scripting bug. Scripts 27 and 100 triggers
	// a palette fading just after loading a room. Catch this, so that only *one*
	// palette refresh occurs.
	if ((_currentEpisodeNum == 27 && _currentRoomNum == 56 && num == 34) ||
	    (_currentEpisodeNum == 100 && _currentRoomNum == 2 && num == 1)) {
		updateScreenPalette = false;
	}

	if (updateScreenPalette) {
		updatePalette();
	} else {
		setPalette(0, 255, 0, 0, 0);
	}

	_currentRoomNum = num;
	_updatedRoomAreasTable[0] = 1;

	_fullRedrawCounter = 1;
	_roomNeedRedraw = true;

	_sequenceEntryTable[5].sprNum = -1;
	_sequenceEntryTable[5].seqNum = -1;
	_sequenceEntryTable[6].sprNum = -1;
	_sequenceEntryTable[6].seqNum = -1;
}

void ToucheEngine::res_loadSprite(int num, int index) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadSprite() num=%d index=%d", num, index);
	assert(index >= 0 && index < NUM_SEQUENCES);
	_sequenceEntryTable[index].sprNum = num;
	SpriteData *spr = &_spritesTable[index];
	const uint32 offs = res_getDataOffset(kResourceTypeSpriteImage, num);
	_fData.seek(offs);
	_currentImageWidth = _fData.readUint16LE();
	_currentImageHeight = _fData.readUint16LE();
	const uint32 size = _currentImageWidth * _currentImageHeight;
	if (size > spr->size) {
		debug(8, "Reallocating memory for sprite %d (index %d), %d bytes needed", num, index, size - spr->size);
		spr->size = size;

		uint8 *buffer = NULL;
		if (spr->ptr)
			buffer = (uint8 *)realloc(spr->ptr, size);

		if (!buffer) {
			// Free previously allocated sprite (when realloc failed)
			free(spr->ptr);

			buffer = (uint8 *)malloc(size);
		}

		if (!buffer)
			error("[ToucheEngine::res_loadSprite] Unable to reallocate memory for sprite %d (%d bytes)", num, size);

		spr->ptr = buffer;
	}
	for (int i = 0; i < _currentImageHeight; ++i) {
		res_decodeScanLineImageRLE(spr->ptr + _currentImageWidth * i, _currentImageWidth);
	}
	spr->bitmapWidth = _currentImageWidth;
	spr->bitmapHeight = _currentImageHeight;
	if (_flagsTable[268] == 0) {
		res_loadImageHelper(spr->ptr, _currentImageWidth, _currentImageHeight);
	}
	spr->w = _currentImageWidth;
	spr->h = _currentImageHeight;
}

void ToucheEngine::res_loadSequence(int num, int index) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadSequence() num=%d index=%d", num, index);
	assert(index < NUM_SEQUENCES);
	_sequenceEntryTable[index].seqNum = num;
	const uint32 offs = res_getDataOffset(kResourceTypeSequence, num);
	_fData.seek(offs);
	_fData.read(_sequenceDataTable[index], 16000);
}

void ToucheEngine::res_decodeScanLineImageRLE(uint8 *dst, int lineWidth) {
	int w = 0;
	while (w < lineWidth) {
		uint8 code = _fData.readByte();
		if ((code & 0xC0) == 0xC0) {
			int len = code & 0x3F;
			uint8 color = _fData.readByte();
			memset(dst, color, len);
			dst += len;
			w += len;
		} else {
			*dst = code;
			++dst;
			++w;
		}
	}
}

void ToucheEngine::res_loadBackdrop() {
	debugC(9, kDebugResource, "ToucheEngine::res_loadBackdrop()");
	_currentBitmapWidth = _fData.readUint16LE();
	_currentBitmapHeight = _fData.readUint16LE();
	for (int i = 0; i < _currentBitmapHeight; ++i) {
		res_decodeScanLineImageRLE(_backdropBuffer + _currentBitmapWidth * i, _currentBitmapWidth);
	}
	_roomWidth = _currentBitmapWidth;
	for (int i = 0; i < _currentBitmapWidth; ++i) {
		if (_backdropBuffer[i] == 255) {
			_roomWidth = i;
			_backdropBuffer[i] = 0;
			break;
		}
	}
	// Workaround for bug #1751149 (original bitmap has a white pixel in its transparent area).
	if (_currentRoomNum == 8 && _currentBitmapWidth == 860) {
		_backdropBuffer[120 * _currentBitmapWidth + 734] = 0;
	}
}

void ToucheEngine::res_loadImage(int num, uint8 *dst) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadImage() num=%d", num);
	const uint32 offsInfo = res_getDataOffset(kResourceTypeIconImage, num);
	_fData.seek(offsInfo);
	_currentImageWidth = _fData.readUint16LE();
	_currentImageHeight = _fData.readUint16LE();
	for (int i = 0; i < _currentImageHeight; ++i) {
		res_decodeScanLineImageRLE(dst + _currentImageWidth * i, _currentImageWidth);
	}
	res_loadImageHelper(dst, _currentImageWidth, _currentImageHeight);
}

void ToucheEngine::res_loadImageHelper(uint8 *imgData, int imgWidth, int imgHeight) {
	uint8 *p = imgData;
	for (_currentImageHeight = 0; _currentImageHeight < imgHeight; ++_currentImageHeight, p += imgWidth) {
		if (*p == 64 || *p == 255) {
			break;
		}
	}
	p = imgData;
	for (_currentImageWidth = 0; _currentImageWidth < imgWidth; ++_currentImageWidth, ++p) {
		if (*p == 64 || *p == 255) {
			break;
		}
	}
	if (_flagsTable[267] == 0) {
		for (int i = 0; i < imgWidth * imgHeight; ++i) {
			uint8 color = imgData[i];
			if (color != 0) {
				if (color < 64) {
					color += 192;
				} else {
					color = 0;
				}
			}
			imgData[i] = color;
		}
	}
}

void ToucheEngine::res_loadSound(int priority, int num) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadSound() num=%d", num);
	if (priority >= 0) {
		uint32 size;
		const uint32 offs = res_getDataOffset(kResourceTypeSound, num, &size);
		Common::SeekableReadStream *datastream = SearchMan.createReadStreamForMember("TOUCHE.DAT");
		if (!datastream) {
			warning("res_loadSound: Could not open TOUCHE.DAT");
			return;
		}

		datastream->seek(offs);
		Audio::AudioStream *stream = Audio::makeVOCStream(datastream, Audio::FLAG_UNSIGNED, DisposeAfterUse::YES);
		if (stream) {
			_mixer->playStream(Audio::Mixer::kSFXSoundType, &_sfxHandle, stream);
		}
	}
}

void ToucheEngine::res_stopSound() {
	_mixer->stopHandle(_sfxHandle);
}

void ToucheEngine::res_loadMusic(int num) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadMusic() num=%d", num);
	uint32 size;
	const uint32 offs = res_getDataOffset(kResourceTypeMusic, num, &size);
	_fData.seek(offs);
	_midiPlayer->play(_fData, size, true);
}

void ToucheEngine::res_loadSpeech(int num) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadSpeech() num=%d", num);
	if (num == -1) {
		res_stopSpeech();
	} else {
		if (_compressedSpeechData < 0) { // uncompressed speech data
			if (_fSpeech[0].isOpen()) {
				_fSpeech[0].close();
			}
			char filename[10];
			sprintf(filename, "V%d", num);
			_fSpeech[0].open(filename);
		}
		if (_fSpeech[0].isOpen()) {
			_flagsTable[617] = num;
		}
	}
}

void ToucheEngine::res_loadSpeechSegment(int num) {
	debugC(9, kDebugResource, "ToucheEngine::res_loadSpeechSegment() num=%d", num);
	if (_talkTextMode != kTalkModeTextOnly && _flagsTable[617] != 0) {
		Audio::AudioStream *stream = 0;
		if (_compressedSpeechData < 0) { // uncompressed speech data
			int i = 0;
			if (num >= 750) {
				num -= 750;
				i = 1;
			}
			if (!_fSpeech[i].isOpen()) {
				return;
			}
			_fSpeech[i].seek(num * 8);
			uint32 offs = _fSpeech[i].readUint32LE();
			uint32 size = _fSpeech[i].readUint32LE();
			if (size == 0) {
				return;
			}
			_fSpeech[i].seek(offs);
			stream = Audio::makeVOCStream(&_fSpeech[i], Audio::FLAG_UNSIGNED);
		} else {
			if (num >= 750) {
				num -= 750;
				_fSpeech[0].seek(kSpeechDataFileHeaderSize);
			} else {
				assert(_flagsTable[617] > 0 && _flagsTable[617] < 140);
				_fSpeech[0].seek(kSpeechDataFileHeaderSize + _flagsTable[617] * 4);
			}
			uint32 dataOffs = _fSpeech[0].readUint32LE();
			if (dataOffs == 0) {
				return;
			}
			_fSpeech[0].seek(dataOffs + num * 8);
			uint32 offs = _fSpeech[0].readUint32LE();
			uint32 size = _fSpeech[0].readUint32LE();
			if (size == 0) {
				return;
			}
			_fSpeech[0].seek(offs);
			Common::SeekableReadStream *tmp = _fSpeech[0].readStream(size);
			if (tmp)
				stream = (compressedSpeechFilesTable[_compressedSpeechData].makeStream)(tmp, DisposeAfterUse::YES);
		}
		if (stream) {
			_speechPlaying = true;
			_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_speechHandle, stream);
		}
	}
}

void ToucheEngine::res_stopSpeech() {
	debugC(9, kDebugResource, "ToucheEngine::res_stopSpeech()");
	_mixer->stopHandle(_speechHandle);
	_speechPlaying = false;
}

} // namespace Touche
