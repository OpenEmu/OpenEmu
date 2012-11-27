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

#include "kyra/text_hof.h"
#include "kyra/resource.h"

#include "common/system.h"

namespace Kyra {

TextDisplayer_HoF::TextDisplayer_HoF(KyraEngine_HoF *vm, Screen_v2 *screen)
	: TextDisplayer(vm, screen), _vm(vm) {
}

void TextDisplayer_HoF::backupTalkTextMessageBkgd(int srcPage, int dstPage) {
	_screen->copyRegion(_talkCoords.x, _talkMessageY, 0, 144, _talkCoords.w, _talkMessageH, srcPage, dstPage);
}

void TextDisplayer_HoF::restoreTalkTextMessageBkgd(int srcPage, int dstPage) {
	_screen->copyRegion(0, 144, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, srcPage, dstPage);
}

void TextDisplayer_HoF::restoreScreen() {
	_vm->restorePage3();
	_vm->drawAnimObjects();
	_screen->copyRegion(_talkCoords.x, _talkMessageY, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, 2, 0, Screen::CR_NO_P_CHECK);
	_vm->flagAnimObjsForRefresh();
	_vm->refreshAnimObjects(0);
}

void TextDisplayer_HoF::printCustomCharacterText(const char *text, int x, int y, uint8 c1, int srcPage, int dstPage) {
	text = preprocessString(text);
	int lineCount = buildMessageSubstrings(text);
	int w = getWidestLineWidth(lineCount);
	int h = lineCount * 10;
	y = MAX(0, y - (lineCount * 10));
	int x1 = 0, x2 = 0;
	calcWidestLineBounds(x1, x2, w, x);

	_talkCoords.x = x1;
	_talkCoords.w = w+2;
	_talkCoords.y = y;
	_talkMessageY = y;
	_talkMessageH = h;

	backupTalkTextMessageBkgd(srcPage, dstPage);
	int curPageBackUp = _screen->_curPage;
	_screen->_curPage = srcPage;

	if (_vm->textEnabled()) {
		for (int i = 0; i < lineCount; ++i) {
			const char *msg = &_talkSubstrings[i * TALK_SUBSTRING_LEN];
			printText(msg, getCenterStringX(msg, x1, x2), i * 10 + _talkMessageY, c1, 0xCF, 0);
		}
	}

	_screen->_curPage = curPageBackUp;
}

char *TextDisplayer_HoF::preprocessString(const char *str) {
	if (str != _talkBuffer) {
		assert(strlen(str) < sizeof(_talkBuffer) - 1);
		strcpy(_talkBuffer, str);
	}

	char *p = _talkBuffer;
	while (*p) {
		if (*p == '\r')
			return _talkBuffer;
		++p;
	}

	p = _talkBuffer;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;
	int textWidth = _screen->getTextWidth(p);
	_screen->_charWidth = 0;

	int maxTextWidth = (_vm->language() == 0) ? 176 : 240;

	if (textWidth > maxTextWidth) {
		if (textWidth > (maxTextWidth*2)) {
			int count = getCharLength(p, textWidth / 3);
			int offs = dropCRIntoString(p, count);
			p += count + offs;
			_screen->_charWidth = -2;
			textWidth = _screen->getTextWidth(p);
			_screen->_charWidth = 0;
			count = getCharLength(p, textWidth / 2);
			dropCRIntoString(p, count);
		} else {
			int count = getCharLength(p, textWidth / 2);
			dropCRIntoString(p, count);
		}
	}
	_screen->setFont(curFont);
	return _talkBuffer;
}

void TextDisplayer_HoF::calcWidestLineBounds(int &x1, int &x2, int w, int x) {
	x1 = x;
	x1 -= (w >> 1);
	x2 = x1 + w + 1;

	if (x1 + w >= 311)
		x1 = 311 - w - 1;

	if (x1 < 8)
		x1 = 8;

	x2 = x1 + w + 1;
}

#pragma mark -

int KyraEngine_HoF::chatGetType(const char *str) {
	str += strlen(str);
	--str;
	switch (*str) {
	case '!':
		return 2;

	case ')':
		return -1;

	case '?':
		return 1;

	default:
		return 0;
	}
}

int KyraEngine_HoF::chatCalcDuration(const char *str) {
	static const uint8 durationMultiplicator[] = { 16, 14, 12, 10, 8, 8, 7, 6, 5, 4 };

	int duration = strlen(str);
	duration *= _flags.isTalkie ? 8 : durationMultiplicator[(_configTextspeed / 10)];
	return MAX<int>(duration, 120);
}

void KyraEngine_HoF::objectChat(const char *str, int object, int vocHigh, int vocLow) {
	setNextIdleAnimTimer();

	_chatVocHigh = _chatVocLow = -1;

	objectChatInit(str, object, vocHigh, vocLow);
	_chatText = str;
	_chatObject = object;
	int chatType = chatGetType(str);
	if (chatType == -1) {
		_chatIsNote = true;
		chatType = 0;
	}

	if (_mainCharacter.facing > 7)
		_mainCharacter.facing = 5;

	static const uint8 talkScriptTable[] = {
		6, 7, 8,
		3, 4, 5,
		3, 4, 5,
		0, 1, 2,
		0, 1, 2,
		0, 1, 2,
		3, 4, 5,
		3, 4, 5
	};

	assert(_mainCharacter.facing * 3 + chatType < ARRAYSIZE(talkScriptTable));
	int script = talkScriptTable[_mainCharacter.facing * 3 + chatType];

	static const char *const chatScriptFilenames[] = {
		"_Z1FSTMT.EMC",
		"_Z1FQUES.EMC",
		"_Z1FEXCL.EMC",
		"_Z1SSTMT.EMC",
		"_Z1SQUES.EMC",
		"_Z1SEXCL.EMC",
		"_Z1BSTMT.EMC",
		"_Z1BQUES.EMC",
		"_Z1BEXCL.EMC"
	};

	objectChatProcess(chatScriptFilenames[script]);
	_chatIsNote = false;

	_text->restoreScreen();

	_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);

	_chatText = 0;
	_chatObject = -1;

	setNextIdleAnimTimer();
}

void KyraEngine_HoF::objectChatInit(const char *str, int object, int vocHigh, int vocLow) {
	str = _text->preprocessString(str);
	int lineNum = _text->buildMessageSubstrings(str);

	int yPos = 0, xPos = 0;

	if (!object) {
		int scale = getScale(_mainCharacter.x1, _mainCharacter.y1);
		yPos = _mainCharacter.y1 - ((_mainCharacter.height * scale) >> 8) - 8;
		xPos = _mainCharacter.x1;
	} else {
		yPos = _talkObjectList[object].y;
		xPos = _talkObjectList[object].x;
	}

	yPos -= lineNum * 10;
	yPos = MAX(yPos, 0);
	_text->_talkMessageY = yPos;
	_text->_talkMessageH = lineNum*10;

	int width = _text->getWidestLineWidth(lineNum);
	_text->calcWidestLineBounds(xPos, yPos, width, xPos);
	_text->_talkCoords.x = xPos;
	_text->_talkCoords.w = width + 2;

	restorePage3();
	_text->backupTalkTextMessageBkgd(2, 2);

	_chatTextEnabled = textEnabled();
	if (_chatTextEnabled) {
		objectChatPrintText(str, object);
		_chatEndTime = _system->getMillis() + chatCalcDuration(str) * _tickLength;
	} else {
		_chatEndTime = _system->getMillis();
	}

	if (speechEnabled()) {
		_chatVocHigh = vocHigh;
		_chatVocLow = vocLow;
	} else {
		_chatVocHigh = _chatVocLow = -1;
	}
}

void KyraEngine_HoF::objectChatPrintText(const char *str, int object) {
	int c1 = _talkObjectList[object].color;
	str = _text->preprocessString(str);
	int lineNum = _text->buildMessageSubstrings(str);
	int maxWidth = _text->getWidestLineWidth(lineNum);
	int x = (object == 0) ? _mainCharacter.x1 : _talkObjectList[object].x;
	int cX1 = 0, cX2 = 0;
	_text->calcWidestLineBounds(cX1, cX2, maxWidth, x);

	for (int i = 0; i < lineNum; ++i) {
		str = &_text->_talkSubstrings[i*_text->maxSubstringLen()];

		int y = _text->_talkMessageY + i * 10;
		x = _text->getCenterStringX(str, cX1, cX2);

		_text->printText(str, x, y, c1, 0xCF, 0);
	}
}

void KyraEngine_HoF::objectChatProcess(const char *script) {
	memset(&_chatScriptData, 0, sizeof(_chatScriptData));
	memset(&_chatScriptState, 0, sizeof(_chatScriptState));

	_emc->load(script, &_chatScriptData, &_opcodesAnimation);
	_emc->init(&_chatScriptState, &_chatScriptData);
	_emc->start(&_chatScriptState, 0);
	while (_emc->isValid(&_chatScriptState))
		_emc->run(&_chatScriptState);

	_animShapeFilename[2] = _characterShapeFile + '0';
	uint8 *shapeBuffer = _res->fileData(_animShapeFilename, 0);
	if (shapeBuffer) {
		int shapeCount = initAnimationShapes(shapeBuffer);

		if (_chatVocHigh >= 0) {
			playVoice(_chatVocHigh, _chatVocLow);
			_chatVocHigh = _chatVocLow = -1;
		}

		objectChatWaitToFinish();

		uninitAnimationShapes(shapeCount, shapeBuffer);
	} else {
		warning("couldn't load file '%s'", _animShapeFilename);
	}

	_emc->unload(&_chatScriptData);
}

void KyraEngine_HoF::objectChatWaitToFinish() {
	int charAnimFrame = _mainCharacter.animFrame;
	setCharacterAnimDim(_animShapeWidth, _animShapeHeight);

	_emc->init(&_chatScriptState, &_chatScriptData);
	_emc->start(&_chatScriptState, 1);

	bool running = true;
	const uint32 endTime = _chatEndTime;
	resetSkipFlag();

	while (running && !shouldQuit()) {
		if (!_emc->isValid(&_chatScriptState))
			_emc->start(&_chatScriptState, 1);

		_animNeedUpdate = false;
		while (!_animNeedUpdate && _emc->isValid(&_chatScriptState))
			_emc->run(&_chatScriptState);

		int curFrame = _animNewFrame;
		uint32 delayTime = _animDelayTime;

		if (!_chatIsNote)
			_mainCharacter.animFrame = 33 + curFrame;

		updateCharacterAnim(0);

		uint32 nextFrame = _system->getMillis() + delayTime * _tickLength;

		while (_system->getMillis() < nextFrame && !shouldQuit()) {
			updateWithText();

			const uint32 curTime = _system->getMillis();
			if ((textEnabled() && curTime > endTime) || (speechEnabled() && !textEnabled() && !snd_voiceIsPlaying()) || skipFlag()) {
				resetSkipFlag();
				nextFrame = curTime;
				running = false;
			}

			delay(10);
		}
	}

	_mainCharacter.animFrame = charAnimFrame;
	updateCharacterAnim(0);
	resetCharacterAnimDim();
}

void KyraEngine_HoF::startDialogue(int dlgIndex) {
	updateDlgBuffer();
	int csEntry, vocH, unused1, unused2;
	loadDlgHeader(csEntry, vocH, unused1, unused2);
	int s = _conversationState[dlgIndex][csEntry];
	uint8 bufferIndex = 8;

	if (s == -1) {
		bufferIndex += (dlgIndex * 6);
		_conversationState[dlgIndex][csEntry] = 0;
	} else if (!s || s == 2) {
		bufferIndex += (dlgIndex * 6 + 2);
		_conversationState[dlgIndex][csEntry] = 1;
	} else {
		bufferIndex += (dlgIndex * 6 + 4);
		_conversationState[dlgIndex][csEntry] = 2;
	}

	int offs = READ_LE_UINT16(_dlgBuffer + bufferIndex);
	processDialogue(offs, vocH, csEntry);
}

void KyraEngine_HoF::zanthSceneStartupChat() {
	int lowest = _flags.isTalkie ? 6 : 5;
	int tableIndex = _mainCharacter.sceneId - READ_LE_UINT16(&_ingameTalkObjIndex[lowest + _newChapterFile]);
	if (queryGameFlag(0x159) || _newSceneDlgState[tableIndex])
		return;

	int csEntry, vocH, scIndex1, scIndex2;
	updateDlgBuffer();
	loadDlgHeader(csEntry, vocH, scIndex1, scIndex2);

	uint8 bufferIndex = 8 + scIndex1 * 6 + scIndex2 * 4 + tableIndex * 2;
	int offs = READ_LE_UINT16(_dlgBuffer + bufferIndex);
	processDialogue(offs, vocH, csEntry);

	_newSceneDlgState[tableIndex] = 1;
}

void KyraEngine_HoF::randomSceneChat() {
	int lowest = _flags.isTalkie ? 6 : 5;
	int tableIndex = (_mainCharacter.sceneId - READ_LE_UINT16(&_ingameTalkObjIndex[lowest + _newChapterFile])) << 2;
	if (queryGameFlag(0x164))
		return;

	int csEntry, vocH, scIndex1, unused;
	updateDlgBuffer();
	loadDlgHeader(csEntry, vocH, scIndex1, unused);

	if (_chatAltFlag) {
		_chatAltFlag = 0;
		tableIndex += 2;
	} else {
		_chatAltFlag = 1;
	}

	uint8 bufferIndex = 8 + scIndex1 * 6 + tableIndex;
	int offs = READ_LE_UINT16(_dlgBuffer + bufferIndex);
	processDialogue(offs, vocH, csEntry);
}

void KyraEngine_HoF::updateDlgBuffer() {
	static const char suffixTalkie[] = "EFG";
	static const char suffixTowns[] = "G  J";

	if (_currentChapter == _npcTalkChpIndex && _mainCharacter.dlgIndex == _npcTalkDlgIndex)
		return;

	_npcTalkChpIndex = _currentChapter;
	_npcTalkDlgIndex = _mainCharacter.dlgIndex;

	Common::String filename = Common::String::format("CH%.02d-S%.02d.DL", _currentChapter, _npcTalkDlgIndex);

	const char *suffix = _flags.isTalkie ? suffixTalkie : suffixTowns;
	if (_flags.platform != Common::kPlatformPC || _flags.isTalkie)
		filename += suffix[_lang];
	else
		filename += 'G';

	delete[] _dlgBuffer;
	_dlgBuffer = _res->fileData(filename.c_str(), 0);
}

void KyraEngine_HoF::loadDlgHeader(int &csEntry, int &vocH, int &scIndex1, int &scIndex2) {
	csEntry = READ_LE_UINT16(_dlgBuffer);
	vocH = READ_LE_UINT16(_dlgBuffer + 2);
	scIndex1 = READ_LE_UINT16(_dlgBuffer + 4);
	scIndex2 = READ_LE_UINT16(_dlgBuffer + 6);
}

void KyraEngine_HoF::processDialogue(int dlgOffset, int vocH, int csEntry) {
	int activeTimSequence = -1;
	int nextTimSequence = -1;
	int cmd = 0;
	int vocHi = -1;
	int vocLo = -1;
	bool loop = true;
	int offs = dlgOffset;

	_screen->hideMouse();

	while (loop) {
		cmd = READ_LE_UINT16(_dlgBuffer + offs);
		offs += 2;

		nextTimSequence = READ_LE_UINT16(&_ingameTalkObjIndex[cmd]);

		if (nextTimSequence == 10) {
			if (queryGameFlag(0x3e))
				nextTimSequence = 14;
			if (queryGameFlag(0x3f))
				nextTimSequence = 15;
			if (queryGameFlag(0x40))
				nextTimSequence = 16;
		}

		if (nextTimSequence == 27 && _mainCharacter.sceneId == 34)
			nextTimSequence = 41;

		if (queryGameFlag(0x72)) {
			if (nextTimSequence == 18)
				nextTimSequence = 43;
			else if (nextTimSequence == 19)
				nextTimSequence = 44;
		}

		if (_mainCharacter.x1 > 160) {
			if (nextTimSequence == 4)
				nextTimSequence = 46;
			else if (nextTimSequence == 5)
				nextTimSequence = 47;
		}

		if (cmd == 10) {
			loop = false;

		} else if (cmd == 4) {
			csEntry = READ_LE_UINT16(_dlgBuffer + offs);
			setDlgIndex(csEntry);
			offs += 2;

		} else {
			if (!_flags.isTalkie || cmd == 11) {
				int len = READ_LE_UINT16(_dlgBuffer + offs);
				offs += 2;
				if (_flags.isTalkie) {
					vocLo = READ_LE_UINT16(_dlgBuffer + offs);
					offs += 2;
				}
				memcpy(_unkBuf500Bytes, _dlgBuffer + offs, len);
				_unkBuf500Bytes[len] = 0;
				offs += len;
				if (_flags.isTalkie)
					continue;

			} else if (_flags.isTalkie) {
				int len = READ_LE_UINT16(_dlgBuffer + offs);
				offs += 2;
				static const int irnv[] = { 91, 105, 110, 114, 118 };
				vocHi = irnv[vocH - 1] + csEntry;
				vocLo = READ_LE_UINT16(_dlgBuffer + offs);
				offs += 2;
				memcpy(_unkBuf500Bytes, _dlgBuffer + offs, len);
				_unkBuf500Bytes[len] = 0;
				offs += len;
			}

			if (_unkBuf500Bytes[0]) {
				if ((!_flags.isTalkie && cmd == 11) || (_flags.isTalkie && cmd == 12)) {
					if (activeTimSequence > -1) {
						deinitTalkObject(activeTimSequence);
						activeTimSequence = -1;
					}
					objectChat((const char *)_unkBuf500Bytes, 0, vocHi, vocLo);
				} else {
					if (activeTimSequence != nextTimSequence) {
						if (activeTimSequence > -1) {
							deinitTalkObject(activeTimSequence);
							activeTimSequence = -1;
						}
						initTalkObject(nextTimSequence);
						activeTimSequence = nextTimSequence;
					}
					npcChatSequence((const char *)_unkBuf500Bytes, nextTimSequence, vocHi, vocLo);
				}
			}
		}
	}

	if (activeTimSequence > -1)
		deinitTalkObject(activeTimSequence);

	_screen->showMouse();
}

void KyraEngine_HoF::initTalkObject(int index) {
	TalkObject &object = _talkObjectList[index];

	char STAFilename[13];
	char ENDFilename[13];

	strcpy(STAFilename, object.filename);
	strcpy(_TLKFilename, object.filename);
	strcpy(ENDFilename, object.filename);

	strcat(STAFilename + 4, "_STA.TIM");
	strcat(_TLKFilename + 4, "_TLK.TIM");
	strcat(ENDFilename + 4, "_END.TIM");

	_currentTalkSections.STATim = _tim->load(STAFilename, &_timOpcodes);
	_currentTalkSections.TLKTim = _tim->load(_TLKFilename, &_timOpcodes);
	_currentTalkSections.ENDTim = _tim->load(ENDFilename, &_timOpcodes);

	if (object.scriptId != -1) {
		_specialSceneScriptStateBackup[object.scriptId] = _specialSceneScriptState[object.scriptId];
		_specialSceneScriptState[object.scriptId] = 1;
	}

	if (_currentTalkSections.STATim) {
		_tim->resetFinishedFlag();
		while (!shouldQuit() && !_tim->finished()) {
			_tim->exec(_currentTalkSections.STATim, false);
			if (_chatText)
				updateWithText();
			else
				update();
			delay(10);
		}
	}
}

void KyraEngine_HoF::deinitTalkObject(int index) {
	TalkObject &object = _talkObjectList[index];

	if (_currentTalkSections.ENDTim) {
		_tim->resetFinishedFlag();
		while (!shouldQuit() && !_tim->finished()) {
			_tim->exec(_currentTalkSections.ENDTim, false);
			if (_chatText)
				updateWithText();
			else
				update();
			delay(10);
		}
	}

	if (object.scriptId != -1)
		_specialSceneScriptState[object.scriptId] = _specialSceneScriptStateBackup[object.scriptId];

	_tim->unload(_currentTalkSections.STATim);
	_tim->unload(_currentTalkSections.TLKTim);
	_tim->unload(_currentTalkSections.ENDTim);
}

void KyraEngine_HoF::npcChatSequence(const char *str, int objectId, int vocHigh, int vocLow) {
	_chatText = str;
	_chatObject = objectId;
	objectChatInit(str, objectId, vocHigh, vocLow);

	if (!_currentTalkSections.TLKTim)
		_currentTalkSections.TLKTim = _tim->load(_TLKFilename, &_timOpcodes);

	setNextIdleAnimTimer();

	uint32 ct = chatCalcDuration(str);
	uint32 time = _system->getMillis();
	_chatEndTime =  time + (3 + ct) * _tickLength;
	uint32 chatAnimEndTime = time + (3 + (ct >> 1)) * _tickLength;

	if (_chatVocHigh >= 0) {
		playVoice(_chatVocHigh, _chatVocLow);
		_chatVocHigh = _chatVocLow = -1;
	}

	while (((textEnabled() && _chatEndTime > _system->getMillis()) || (speechEnabled() && snd_voiceIsPlaying())) && !(shouldQuit() || skipFlag())) {
		if ((!speechEnabled() && chatAnimEndTime > _system->getMillis()) || (speechEnabled() && snd_voiceIsPlaying())) {
			_tim->resetFinishedFlag();
			while (!_tim->finished() && !skipFlag() && !shouldQuit()) {
				if (_currentTalkSections.TLKTim)
					_tim->exec(_currentTalkSections.TLKTim, false);
				else
					_tim->resetFinishedFlag();

				updateWithText();
				delay(10);
			}

			if (_currentTalkSections.TLKTim)
				_tim->stopCurFunc();
		}
		updateWithText();
	}

	resetSkipFlag();

	_tim->unload(_currentTalkSections.TLKTim);

	_text->restoreScreen();
	_chatText = 0;
	_chatObject = -1;
	setNextIdleAnimTimer();
}

void KyraEngine_HoF::setDlgIndex(int dlgIndex) {
	if (dlgIndex == _mainCharacter.dlgIndex)
		return;
	memset(_newSceneDlgState, 0, 32);
	for (int i = 0; i < 19; i++)
		memset(_conversationState[i], -1, 14);
	_chatAltFlag = false;
	_mainCharacter.dlgIndex = dlgIndex;
}

} // End of namespace Kyra
