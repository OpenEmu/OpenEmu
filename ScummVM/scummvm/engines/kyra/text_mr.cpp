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

#include "kyra/text_mr.h"
#include "kyra/resource.h"

#include "common/system.h"

namespace Kyra {

TextDisplayer_MR::TextDisplayer_MR(KyraEngine_MR *vm, Screen_MR *screen)
	: TextDisplayer(vm, screen), _vm(vm), _screen(screen) {
}

char *TextDisplayer_MR::preprocessString(const char *str) {
	if (_talkBuffer != str) {
		assert(strlen(str) < sizeof(_talkBuffer) - 1);
		strcpy(_talkBuffer, str);
	}

	char *p = _talkBuffer;
	while (*p) {
		if (*p++ == '\r')
			return _talkBuffer;
	}

	p = _talkBuffer;
	Screen::FontId curFont = _screen->setFont(Screen::FID_8_FNT);
	_screen->_charWidth = -2;

	const int maxTextWidth = (_vm->language() == 0) ? 176 : 240;
	int textWidth = _screen->getTextWidth(p);

	if (textWidth > maxTextWidth) {
		int count = 0, offs = 0;
		if (textWidth > (3*maxTextWidth)) {
			count = getCharLength(p, textWidth/4);
			offs = dropCRIntoString(p, count, getCharLength(p, maxTextWidth));
			p += count + offs;
			// No update of textWidth here
		}

		if (textWidth > (2*maxTextWidth)) {
			count = getCharLength(p, textWidth/3);
			offs = dropCRIntoString(p, count, getCharLength(p, maxTextWidth));
			p += count + offs;
			textWidth = _screen->getTextWidth(p);
		}

		count = getCharLength(p, textWidth/2);
		offs = dropCRIntoString(p, count, getCharLength(p, maxTextWidth));
		p += count + offs;
		textWidth = _screen->getTextWidth(p);

		if (textWidth > maxTextWidth) {
			count = getCharLength(p, textWidth/2);
			offs = dropCRIntoString(p, count, getCharLength(p, maxTextWidth));
		}
	}

	_screen->setFont(curFont);
	return _talkBuffer;
}

int TextDisplayer_MR::dropCRIntoString(char *str, int minOffs, int maxOffs) {
	int offset = 0;
	char *proc = str + minOffs;

	for (int i = minOffs; i < maxOffs; ++i) {
		if (*proc == ' ') {
			*proc = '\r';
			return offset;
		} else if (*proc == '-') {
			memmove(proc+1, proc, strlen(proc)+1);
			*(++proc) = '\r';
			++offset;
			return offset;
		}

		++offset;
		++proc;

		if (!*proc)
			return 0;
	}

	offset = 0;
	proc = str + minOffs;
	for (int i = minOffs; i >= 0; --i) {
		if (*proc == ' ') {
			*proc = '\r';
			return offset;
		} else if (*proc == '-') {
			memmove(proc+1, proc, strlen(proc)+1);
			*(++proc) = '\r';
			++offset;
			return offset;
		}

		--offset;
		--proc;

		if (!*proc)
			return 0;
	}

	*(str + minOffs) = '\r';
	return 0;
}

void TextDisplayer_MR::printText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 c2) {
	if (_vm->_albumChatActive) {
		c0 = 0xEE;
		c1 = 0xE3;
		c2 = 0x00;
	}

	uint8 colorMap[] = { 0, 255, 240, 240 };
	colorMap[3] = c1;
	_screen->setTextColor(colorMap, 0, 3);
	_screen->_charWidth = -2;
	_screen->printText(str, x, y, c0, c2);
	_screen->_charWidth = 0;
}

void TextDisplayer_MR::restoreScreen() {
	_vm->restorePage3();
	_vm->drawAnimObjects();
	_screen->copyRegion(_talkCoords.x, _talkMessageY, _talkCoords.x, _talkMessageY, _talkCoords.w, _talkMessageH, 2, 0, Screen::CR_NO_P_CHECK);
	_vm->flagAnimObjsForRefresh();
	_vm->refreshAnimObjects(0);
}

void TextDisplayer_MR::calcWidestLineBounds(int &x1, int &x2, int w, int x) {
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

int KyraEngine_MR::chatGetType(const char *str) {
	while (*str)
		++str;
	--str;
	switch (*str) {
	case '!':
		return 2;

	case ')':
		return 3;

	case '?':
		return 1;

	case '.':
	default:
		return 0;
	}
}

int KyraEngine_MR::chatCalcDuration(const char *str) {
	return MAX<int>(120, strlen(str)*6);
}

void KyraEngine_MR::objectChat(const char *str, int object, int vocHigh, int vocLow) {
	if (_mainCharacter.animFrame == 87 || _mainCharacter.animFrame == 0xFFFF || _mainCharacter.x1 <= 0 || _mainCharacter.y1 <= 0)
		return;

	_chatVocLow = _chatVocHigh = -1;
	objectChatInit(str, object, vocHigh, vocLow);
	_chatText = str;
	_chatObject = object;
	int chatType = chatGetType(str);

	if (_mainCharacter.facing > 7)
		_mainCharacter.facing = 5;

	static const uint8 talkScriptTable[] = {
		0x10, 0x11, 0x12, 0x13,
		0x0C, 0x0D, 0x0E, 0x0F,
		0x0C, 0x0D, 0x0E, 0x0F,
		0x04, 0x05, 0x06, 0x07,
		0x00, 0x01, 0x02, 0x03,
		0x00, 0x01, 0x02, 0x03,
		0x08, 0x09, 0x0A, 0x0B,
		0x08, 0x09, 0x0A, 0x0B
	};

	static const char *const talkFilenameTable[] = {
		"MTFL00S.EMC", "MTFL00Q.EMC", "MTFL00E.EMC", "MTFL00T.EMC",
		"MTFR00S.EMC", "MTFR00Q.EMC", "MTFR00E.EMC", "MTFR00T.EMC",
		 "MTL00S.EMC",  "MTL00Q.EMC",  "MTL00E.EMC",  "MTL00T.EMC",
		 "MTR00S.EMC",  "MTR00Q.EMC",  "MTR00E.EMC",  "MTR00T.EMC",
		 "MTA00S.EMC",  "MTA00Q.EMC",  "MTA00E.EMC",  "MTA00T.EMC"
	};

	int chat = talkScriptTable[chatType + _mainCharacter.facing * 4];
	objectChatProcess(talkFilenameTable[chat]);
	_text->restoreScreen();
	_mainCharacter.animFrame = _characterFrameTable[_mainCharacter.facing];
	updateCharacterAnim(0);
	_chatText = 0;
	_chatObject = -1;
	setNextIdleAnimTimer();
}

void KyraEngine_MR::objectChatInit(const char *str, int object, int vocHigh, int vocLow) {
	str = _text->preprocessString(str);
	int lineNum = _text->buildMessageSubstrings(str);

	int xPos = 0, yPos = 0;

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

void KyraEngine_MR::objectChatPrintText(const char *str, int object) {
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

		_text->printText(str, x, y, c1, 0xF0, 0);
	}
}

void KyraEngine_MR::objectChatProcess(const char *script) {
	memset(&_chatScriptData, 0, sizeof(_chatScriptData));
	memset(&_chatScriptState, 0, sizeof(_chatScriptState));

	_emc->load(script, &_chatScriptData, &_opcodesAnimation);
	_emc->init(&_chatScriptState, &_chatScriptData);
	_emc->start(&_chatScriptState, 0);
	while (_emc->isValid(&_chatScriptState))
		_emc->run(&_chatScriptState);

	if (_chatVocHigh >= 0) {
		playVoice(_chatVocHigh, _chatVocLow);
		_chatVocHigh = _chatVocLow = -1;
	}

	_useFrameTable = true;
	objectChatWaitToFinish();
	_useFrameTable = false;

	_emc->unload(&_chatScriptData);
}

void KyraEngine_MR::objectChatWaitToFinish() {
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
		while (!_animNeedUpdate && _emc->isValid(&_chatScriptState) && !shouldQuit())
			_emc->run(&_chatScriptState);

		int curFrame = _animNewFrame;
		uint32 delayTime = _animDelayTime;

		_mainCharacter.animFrame = curFrame;
		updateCharacterAnim(0);

		uint32 nextFrame = _system->getMillis() + delayTime * _tickLength;

		while (_system->getMillis() < nextFrame && !shouldQuit()) {
			updateWithText();

			const uint32 curTime = _system->getMillis();
			if ((textEnabled() && !speechEnabled() && curTime > endTime) || (speechEnabled() && !snd_voiceIsPlaying()) || skipFlag()) {
				snd_stopVoice();
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

void KyraEngine_MR::badConscienceChat(const char *str, int vocHigh, int vocLow) {
	if (!_badConscienceShown)
		return;

	setNextIdleAnimTimer();
	_chatVocHigh = _chatVocLow = -1;
	objectChatInit(str, 1, vocHigh, vocLow);
	_chatText = str;
	_chatObject = 1;
	badConscienceChatWaitToFinish();
	updateSceneAnim(0x0E, _badConscienceFrameTable[_badConscienceAnim+16]);
	_text->restoreScreen();
	update();
	_chatText = 0;
	_chatObject = -1;
}

void KyraEngine_MR::badConscienceChatWaitToFinish() {
	if (_chatVocHigh) {
		playVoice(_chatVocHigh, _chatVocLow);
		_chatVocHigh = _chatVocLow = -1;
	}

	bool running = true;
	const uint32 endTime = _chatEndTime;
	resetSkipFlag();

	uint32 nextFrame = _system->getMillis() + _rnd.getRandomNumberRng(4, 8) * _tickLength;

	int frame = _badConscienceFrameTable[_badConscienceAnim+24];
	while (running && !shouldQuit()) {
		if (nextFrame < _system->getMillis()) {
			++frame;
			if (_badConscienceFrameTable[_badConscienceAnim+32] < frame)
				frame = _badConscienceFrameTable[_badConscienceAnim+24];

			updateSceneAnim(0x0E, frame);
			updateWithText();

			nextFrame = _system->getMillis() + _rnd.getRandomNumberRng(4, 8) * _tickLength;
		}

		updateWithText();

		const uint32 curTime = _system->getMillis();
		if ((textEnabled() && !speechEnabled() && curTime > endTime) || (speechEnabled() && !snd_voiceIsPlaying()) || skipFlag()) {
			snd_stopVoice();
			resetSkipFlag();
			nextFrame = curTime;
			running = false;
		}

		delay(10);
	}
}

void KyraEngine_MR::goodConscienceChat(const char *str, int vocHigh, int vocLow) {
	if (!_goodConscienceShown)
		return;

	setNextIdleAnimTimer();
	_chatVocHigh = _chatVocLow = -1;
	objectChatInit(str, 87, vocHigh, vocLow);
	_chatText = str;
	_chatObject = 87;
	goodConscienceChatWaitToFinish();
	updateSceneAnim(0x0F, _goodConscienceFrameTable[_goodConscienceAnim+10]);
	_text->restoreScreen();
	update();
	_chatText = 0;
	_chatObject = -1;
}

void KyraEngine_MR::goodConscienceChatWaitToFinish() {
	if (_chatVocHigh) {
		playVoice(_chatVocHigh, _chatVocLow);
		_chatVocHigh = _chatVocLow = -1;
	}

	bool running = true;
	const uint32 endTime = _chatEndTime;
	resetSkipFlag();

	uint32 nextFrame = _system->getMillis() + _rnd.getRandomNumberRng(3, 6) * _tickLength;

	int frame = _goodConscienceFrameTable[_goodConscienceAnim+15];
	while (running && !shouldQuit()) {
		if (nextFrame < _system->getMillis()) {
			++frame;
			if (_goodConscienceFrameTable[_goodConscienceAnim+20] < frame)
				frame = _goodConscienceFrameTable[_goodConscienceAnim+15];

			updateSceneAnim(0x0F, frame);
			updateWithText();

			nextFrame = _system->getMillis() + _rnd.getRandomNumberRng(3, 6) * _tickLength;
		}

		updateWithText();

		const uint32 curTime = _system->getMillis();
		if ((textEnabled() && !speechEnabled() && curTime > endTime) || (speechEnabled() && !snd_voiceIsPlaying()) || skipFlag()) {
			snd_stopVoice();
			resetSkipFlag();
			nextFrame = curTime;
			running = false;
		}

		delay(10);
	}
}

void KyraEngine_MR::albumChat(const char *str, int vocHigh, int vocLow) {
	_talkObjectList[1].x = 190;
	_talkObjectList[1].y = 188;

	_chatVocHigh = _chatVocLow = -1;
	_albumChatActive = true;
	albumChatInit(str, 1, vocHigh, vocLow);
	_albumChatActive = false;

	_chatText = str;
	_chatObject = 1;
	_screen->hideMouse();
	albumChatWaitToFinish();
	_screen->showMouse();

	_chatText = 0;
	_chatObject = -1;
}

void KyraEngine_MR::albumChatInit(const char *str, int object, int vocHigh, int vocLow) {
	Common::String realString;

	while (*str) {
		if (str[0] == '\\' && str[1] == 'r') {
			realString += '\r';
			++str;
		} else {
			realString += *str;
		}

		++str;
	}

	str = realString.c_str();

	str = _text->preprocessString(str);
	int lineNum = _text->buildMessageSubstrings(str);

	int xPos = 0, yPos = 0;

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

	_screen->hideMouse();

	if (textEnabled()) {
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

	_screen->showMouse();
}

void KyraEngine_MR::albumChatWaitToFinish() {
	if (_chatVocHigh) {
		playVoice(_chatVocHigh, _chatVocLow);
		_chatVocHigh = _chatVocLow = -1;
	}

	bool running = true;
	const uint32 endTime = _chatEndTime;
	resetSkipFlag();

	uint32 nextFrame = 0;
	int frame = 12;
	while (running && !shouldQuit()) {
		if (nextFrame < _system->getMillis()) {
			++frame;
			if (frame > 22)
				frame = 13;

			albumRestoreRect();
			_album.wsa->displayFrame(frame, 2, -100, 90, 0x4000, 0, 0);
			albumUpdateRect();

			nextFrame = _system->getMillis() + _rnd.getRandomNumberRng(4, 8) * _tickLength;
		}

		if (_album.nextPage != 14)
			albumUpdateAnims();
		else
			_screen->updateScreen();

		const uint32 curTime = _system->getMillis();
		if ((textEnabled() && !speechEnabled() && curTime > endTime) || (speechEnabled() && !snd_voiceIsPlaying()) || skipFlag()) {
			snd_stopVoice();
			resetSkipFlag();
			nextFrame = curTime;
			running = false;
		}

		delay(10);
	}
}

void KyraEngine_MR::malcolmSceneStartupChat() {
	if (_noStartupChat)
		return;

	int index = _mainCharacter.sceneId - _chapterLowestScene[_currentChapter];
	if (_newSceneDlgState[index])
		return;

	updateDlgBuffer();
	int vocHighBase = 0, vocHighIndex = 0, index1 = 0, index2 = 0;
	loadDlgHeader(vocHighBase, vocHighIndex, index1, index2);

	_cnvFile->seek(index1*6, SEEK_CUR);
	_cnvFile->seek(index2*4, SEEK_CUR);
	_cnvFile->seek(index*2, SEEK_CUR);
	_cnvFile->seek(_cnvFile->readUint16LE(), SEEK_SET);

	_isStartupDialog = true;
	processDialog(vocHighIndex, vocHighBase, 0);
	_isStartupDialog = false;
	_newSceneDlgState[index] = true;
}

void KyraEngine_MR::updateDlgBuffer() {
	if (_cnvFile)
		_cnvFile->seek(0, SEEK_SET);

	if (_curDlgIndex == _mainCharacter.dlgIndex && _curDlgChapter == _currentChapter && _curDlgLang == _lang)
		return;

	Common::String dlgFile = Common::String::format("CH%.02d-S%.02d.%s", _currentChapter, _mainCharacter.dlgIndex, _languageExtension[_lang]);
	Common::String cnvFile = Common::String::format("CH%.02d-S%.02d.CNV", _currentChapter, _mainCharacter.dlgIndex);

	delete _cnvFile;
	delete _dlgBuffer;

	_res->exists(cnvFile.c_str(), true);
	_res->exists(dlgFile.c_str(), true);
	_cnvFile = _res->createReadStream(cnvFile);
	_dlgBuffer = _res->createReadStream(dlgFile);
	assert(_cnvFile);
	assert(_dlgBuffer);
}

void KyraEngine_MR::loadDlgHeader(int &vocHighBase, int &vocHighIndex, int &index1, int &index2) {
	assert(_cnvFile);
	vocHighIndex = _cnvFile->readSint16LE();
	vocHighBase = _cnvFile->readSint16LE();
	index1 = _cnvFile->readSint16LE();
	index2 = _cnvFile->readSint16LE();
}

void KyraEngine_MR::setDlgIndex(int index) {
	if (_mainCharacter.dlgIndex != index) {
		memset(_newSceneDlgState, 0, sizeof(_newSceneDlgState));
		memset(_conversationState, -1, sizeof(_conversationState));
		_chatAltFlag = false;
		_mainCharacter.dlgIndex = index;
	}
}

void KyraEngine_MR::updateDlgIndex() {
	uint16 dlgIndex = _mainCharacter.dlgIndex;

	if (_currentChapter == 1) {
		static const uint8 dlgIndexMoodNice[] = { 0x0C, 0x0E, 0x10, 0x0F, 0x11 };
		static const uint8 dlgIndexMoodNormal[] = { 0x00, 0x02, 0x04, 0x03, 0x05 };
		static const uint8 dlgIndexMoodEvil[] = { 0x06, 0x08, 0x0A, 0x09, 0x0B };

		if (_malcolmsMood == 0)
			dlgIndex = dlgIndexMoodNice[_characterShapeFile];
		else if (_malcolmsMood == 1)
			dlgIndex = dlgIndexMoodNormal[_characterShapeFile];
		else if (_malcolmsMood == 2)
			dlgIndex = dlgIndexMoodEvil[_characterShapeFile];
	} else if (_currentChapter == 2) {
		if (dlgIndex >= 8)
			dlgIndex -= 4;
		if (dlgIndex >= 4)
			dlgIndex -= 4;

		if (_malcolmsMood == 0)
			dlgIndex += 8;
		else if (_malcolmsMood == 2)
			dlgIndex += 4;
	} else if (_currentChapter == 4) {
		if (dlgIndex >= 10)
			dlgIndex -= 5;
		if (dlgIndex >= 5)
			dlgIndex -= 5;

		if (_malcolmsMood == 0)
			dlgIndex += 10;
		else if (_malcolmsMood == 2)
			dlgIndex += 5;
	}

	_mainCharacter.dlgIndex = dlgIndex;
}

void KyraEngine_MR::processDialog(int vocHighIndex, int vocHighBase, int funcNum) {
	bool running = true;
	int script = -1;
	int vocHigh = -1, vocLow = -1;

	while (running) {
		uint16 cmd = _cnvFile->readUint16LE();
		int object = cmd - 12;

		if (cmd == 10) {
			break;
		} else if (cmd == 4) {
			vocHighBase = _cnvFile->readUint16LE();
			setDlgIndex(vocHighBase);
		} else if (cmd == 11) {
			int strSize = _cnvFile->readUint16LE();
			vocLow = _cnvFile->readUint16LE();
			_cnvFile->read(_stringBuffer, strSize);
			_stringBuffer[strSize] = 0;
		} else {
			vocHigh = _vocHighTable[vocHighIndex-1] + vocHighBase;
			vocLow = _cnvFile->readUint16LE();
			getTableEntry(_dlgBuffer, vocLow, _stringBuffer);

			if (_isStartupDialog) {
				delay(60*_tickLength, true);
				_isStartupDialog = false;
			}

			if (*_stringBuffer == 0)
				continue;

			if (cmd != 12) {
				if (object != script) {
					if (script >= 0) {
						dialogEndScript(script);
						script = -1;
					}

					dialogStartScript(object, funcNum);
					script = object;
				}

				npcChatSequence(_stringBuffer, object, vocHigh, vocLow);
			} else {
				if (script >= 0) {
					dialogEndScript(script);
					script = -1;
				}

				objectChat(_stringBuffer, 0, vocHigh, vocLow);
				playStudioSFX(_stringBuffer);
			}
		}
	}

	if (script != -1)
		dialogEndScript(script);
}

void KyraEngine_MR::dialogStartScript(int object, int funcNum) {
	_dialogSceneAnim = _talkObjectList[object].sceneAnim;
	_dialogSceneScript = _talkObjectList[object].sceneScript;
	if (_dialogSceneAnim >= 0 && _dialogSceneScript >= 0) {
		_specialSceneScriptStateBackup[_dialogSceneScript] = _specialSceneScriptState[_dialogSceneScript];
		_specialSceneScriptState[_dialogSceneScript] = true;
	}

	_emc->init(&_dialogScriptState, &_dialogScriptData);
	_emc->load(_talkObjectList[object].filename, &_dialogScriptData, &_opcodesDialog);

	_dialogScriptFuncStart = funcNum * 3 + 0;
	_dialogScriptFuncProc = funcNum * 3 + 1;
	_dialogScriptFuncEnd = funcNum * 3 + 2;

	_emc->start(&_dialogScriptState, _dialogScriptFuncStart);
	while (_emc->isValid(&_dialogScriptState))
		_emc->run(&_dialogScriptState);
}

void KyraEngine_MR::dialogEndScript(int object) {
	_emc->init(&_dialogScriptState, &_dialogScriptData);
	_emc->start(&_dialogScriptState, _dialogScriptFuncEnd);

	while (_emc->isValid(&_dialogScriptState))
		_emc->run(&_dialogScriptState);

	if (_dialogSceneAnim >= 0 && _dialogSceneScript >= 0) {
		_specialSceneScriptState[_dialogSceneScript] = _specialSceneScriptStateBackup[_dialogSceneScript];
		_dialogSceneScript = _dialogSceneAnim = -1;
	}

	_emc->unload(&_dialogScriptData);
}

void KyraEngine_MR::npcChatSequence(const char *str, int object, int vocHigh, int vocLow) {
	_chatText = str;
	_chatObject = object;
	_chatVocHigh = _chatVocLow = -1;

	objectChatInit(str, object, vocHigh, vocLow);

	if (_chatVocHigh >= 0 && _chatVocLow >= 0) {
		playVoice(_chatVocHigh, _chatVocLow);
		_chatVocHigh = _chatVocLow = -1;
	}

	_emc->init(&_dialogScriptState, &_dialogScriptData);
	_emc->start(&_dialogScriptState, _dialogScriptFuncProc);

	resetSkipFlag();

	uint32 endTime = _chatEndTime;
	bool running = true;
	while (running && !shouldQuit()) {
		if (!_emc->run(&_dialogScriptState)) {
			_emc->init(&_dialogScriptState, &_dialogScriptData);
			_emc->start(&_dialogScriptState, _dialogScriptFuncProc);
		}

		const uint32 curTime = _system->getMillis();
		if ((textEnabled() && !speechEnabled() && curTime > endTime) || (speechEnabled() && !snd_voiceIsPlaying()) || skipFlag()) {
			snd_stopVoice();
			resetSkipFlag();
			running = false;
		}
	}
	_text->restoreScreen();
	_chatText = 0;
	_chatObject= - 1;
}

void KyraEngine_MR::randomSceneChat() {
	updateDlgBuffer();

	int index = (_mainCharacter.sceneId - _chapterLowestScene[_currentChapter]) * 2;

	int vocHighBase = 0, vocHighIndex = 0, index1 = 0, index2 = 0;
	loadDlgHeader(vocHighBase, vocHighIndex, index1, index2);

	if (_chatAltFlag)
		index++;
	_chatAltFlag = !_chatAltFlag;

	_cnvFile->seek(index1*6, SEEK_CUR);
	_cnvFile->seek(index*2, SEEK_CUR);
	_cnvFile->seek(_cnvFile->readUint16LE(), SEEK_SET);

	processDialog(vocHighIndex, vocHighBase, 0);
}

void KyraEngine_MR::doDialog(int dlgIndex, int funcNum) {
	switch (_currentChapter-2) {
	case 0:
		dlgIndex -= 34;
		break;

	case 1:
		dlgIndex -= 54;
		break;

	case 2:
		dlgIndex -= 55;
		break;

	case 3:
		dlgIndex -= 70;
		break;

	default:
		break;
	}

	updateDlgBuffer();

	int vocHighBase = 0, vocHighIndex = 0, index1 = 0, index2 = 0;
	loadDlgHeader(vocHighBase, vocHighIndex, index1, index2);

	int convState = _conversationState[dlgIndex][vocHighBase];
	uint32 offset = ((vocHighIndex == 1) ? dlgIndex - 1 : dlgIndex) * 6;
	if (convState == -1) {
		_cnvFile->seek(offset, SEEK_CUR);
		_conversationState[dlgIndex][vocHighBase] = 0;
	} else if (convState == 0 || convState == 2) {
		_cnvFile->seek(offset+2, SEEK_CUR);
		_conversationState[dlgIndex][vocHighBase] = 1;
	} else {
		_cnvFile->seek(offset+4, SEEK_CUR);
		_conversationState[dlgIndex][vocHighBase] = 2;
	}

	_cnvFile->seek(_cnvFile->readUint16LE(), SEEK_SET);

	processDialog(vocHighIndex, vocHighBase, funcNum);
}

} // End of namespace Kyra
