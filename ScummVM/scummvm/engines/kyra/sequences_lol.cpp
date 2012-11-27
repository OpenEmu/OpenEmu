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

#ifdef ENABLE_LOL

#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "base/version.h"

#include "common/system.h"

namespace Kyra {

#pragma mark - Intro

int LoLEngine::processPrologue() {
	// There are two non-interactive demos (one which plays the intro and another one) which plays a number of specific scenes.
	// We try to identify the latter one by looking for a specific file.
	_res->loadPakFile("GENERAL.PAK");
	if (_flags.isDemo && _res->exists("scene1.cps")) {
		return playDemo();
	} else {
		setupPrologueData(true);
		if (!saveFileLoadable(0) || _flags.isDemo)
			showIntro();
	}

	if (_flags.isDemo) {
		_screen->fadePalette(_screen->getPalette(1), 30, 0);
		_screen->loadBitmap("FINAL.CPS", 2, 2, &_screen->getPalette(0));
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->fadePalette(_screen->getPalette(0), 30, 0);
		delayWithTicks(300);
		_screen->fadePalette(_screen->getPalette(1), 60, 0);

		setupPrologueData(false);
		return -1;
	}

	preInit();

	Common::String versionString(Common::String::format("ScummVM %s", gScummVMVersion));

	int processSelection = -1;
	while (!shouldQuit() && processSelection == -1) {
		_screen->loadBitmap("TITLE.CPS", 2, 2, &_screen->getPalette(0));
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

		_screen->setFont(Screen::FID_6_FNT);
		// Original version: (260|193) "V CD1.02 D"
		const int width = _screen->getTextWidth(versionString.c_str());
		_screen->fprintString("%s", 320 - width, 193, 0x67, 0x00, 0x04, versionString.c_str());
		_screen->setFont(_flags.lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_9_FNT);

		_screen->fadePalette(_screen->getPalette(0), 0x1E);
		_screen->updateScreen();

		_eventList.clear();
		int selection = mainMenu();

		if (selection != 3) {
			_screen->hideMouse();

			// Unlike the original, we add a nice fade to black
			_screen->getPalette(0).clear();
			_screen->fadeToBlack(0x54);
		}

		switch (selection) {
		case -1:
			// This is sent on RTL for example, if we would not have any
			// special case for this the default path would call quitGame
			// and thus make the next game launched from the launcher
			// quit instantly.
			break;

		case 0:     // New game
			processSelection = 0;
			break;

		case 1:     // Show intro
			showIntro();
			break;

		case 2: {   // "Lore of the Lands" (only CD version)
			HistoryPlayer history(this);
			history.play();
		} break;

		case 3:     // Load game
			if (_gui->runMenu(_gui->_loadMenu))
				processSelection = 3;
			break;

		case 4:     // Quit game
		default:
			quitGame();
			updateInput();
		}
	}

	if (processSelection == 0) {
		_sound->loadSoundFile(0);
		_sound->playTrack(6);
		chooseCharacter();
		_sound->playTrack(1);
		_screen->fadeToBlack();
	}

	setupPrologueData(false);

	return processSelection;
}

void LoLEngine::setupPrologueData(bool load) {
	static const char *const fileListCD[] = {
		"GENERAL.PAK", "INTROVOC.PAK", "STARTUP.PAK", "INTRO1.PAK",
		"INTRO2.PAK", "INTRO3.PAK", "INTRO4.PAK", "INTRO5.PAK",
		"INTRO6.PAK", "INTRO7.PAK", "INTRO8.PAK", "INTRO9.PAK",
		"HISTORY.PAK", 0
	};

	static const char *const fileListFloppy[] = {
		"INTRO.PAK", "INTROVOC.PAK", 0
	};
	const char *const *fileList = _flags.isTalkie ? fileListCD : fileListFloppy;

	char filename[32];
	for (uint i = 0; fileList[i]; ++i) {
		filename[0] = '\0';

		if (_flags.isTalkie) {
			strcpy(filename, _languageExt[_lang]);
			strcat(filename, "/");
		}

		strcat(filename, fileList[i]);

		if (load) {
			if (!_res->loadPakFile(filename))
				error("Couldn't load file: '%s'", filename);
		} else {
			_res->unloadPakFile(filename);
		}
	}

	_screen->clearPage(0);
	_screen->clearPage(3);

	if (load) {
		_chargenWSA = new WSAMovie_v2(this);
		assert(_chargenWSA);

		//_charSelection = -1;
		_charSelectionInfoResult = -1;

		_selectionAnimFrames[0] = _selectionAnimFrames[2] = 0;
		_selectionAnimFrames[1] = _selectionAnimFrames[3] = 1;

		memset(_selectionAnimTimers, 0, sizeof(_selectionAnimTimers));
		_screen->getPalette(1).clear();

		_sound->selectAudioResourceSet(kMusicIntro);

		// We have three sound.dat files, one for the intro, one for the
		// end sequence and one for ingame, each contained in a different
		// PAK file. Therefore a new call to loadSoundFile() is required
		// whenever the PAK file configuration changes.
		if (_flags.platform == Common::kPlatformPC98)
			_sound->loadSoundFile("SOUND.DAT");

		if (_flags.isDemo)
			_sound->loadSoundFile("LOREINTR");
	} else {
		delete _chargenWSA; _chargenWSA = 0;

		_screen->getPalette(0).clear();
		_screen->setScreenPalette(_screen->getPalette(0));

		if (shouldQuit())
			return;

		_eventList.clear();
		_sound->selectAudioResourceSet(kMusicIntro);
	}
}

void LoLEngine::showIntro() {
	_tim = new TIMInterpreter(this, _screen, _system);
	assert(_tim);

	if (_flags.platform == Common::kPlatformPC98)
		showStarcraftLogo();

	_screen->getPalette(0).clear();
	_screen->setScreenPalette(_screen->getPalette(0));

	_screen->clearPage(0);
	_screen->clearPage(4);
	_screen->clearPage(8);

	TIM *intro = _tim->load("LOLINTRO.TIM", &_timIntroOpcodes);

	_screen->loadFont(Screen::FID_8_FNT, "NEW8P.FNT");
	_screen->loadFont(Screen::FID_INTRO_FNT, "INTRO.FNT");
	_screen->setFont(_flags.lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);

	_tim->resetFinishedFlag();
	_tim->setLangData("LOLINTRO.DIP");

	_screen->hideMouse();

	uint32 palNextFadeStep = 0;
	while (!_tim->finished() && !shouldQuit() && !skipFlag()) {
		updateInput();
		_tim->exec(intro, false);
		if (!_flags.isDemo && _flags.platform != Common::kPlatformPC98)
			_screen->checkedPageUpdate(8, 4);

		if (_tim->_palDiff) {
			if (palNextFadeStep < _system->getMillis()) {
				_tim->_palDelayAcc += _tim->_palDelayInc;
				palNextFadeStep = _system->getMillis() + ((_tim->_palDelayAcc >> 8) * _tickLength);
				_tim->_palDelayAcc &= 0xFF;

				if (!_screen->fadePalStep(_screen->getPalette(0), _tim->_palDiff)) {
					_screen->setScreenPalette(_screen->getPalette(0));
					_tim->_palDiff = 0;
				}
			}
		}

		_system->delayMillis(10);
		_screen->updateScreen();
	}
	_screen->showMouse();
	_sound->voiceStop();
	_sound->beginFadeOut();

	_eventList.clear();

	_tim->unload(intro);
	_tim->clearLangData();

	for (int i = 0; i < TIM::kWSASlots; i++)
		_tim->freeAnimStruct(i);

	delete _tim;
	_tim = 0;

	_screen->fadePalette(_screen->getPalette(1), 30, 0);
}

int LoLEngine::chooseCharacter() {
	_tim = new TIMInterpreter(this, _screen, _system);
	assert(_tim);

	_tim->setLangData("LOLINTRO.DIP");

	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");

	_screen->loadBitmap("ITEMICN.SHP", 3, 3, 0);
	_screen->setMouseCursor(0, 0, _screen->getPtrToShape(_screen->getCPagePtr(3), 0));

	while (!_screen->isMouseVisible())
		_screen->showMouse();

	_screen->loadBitmap("CHAR.CPS", 2, 2, &_screen->getPalette(0));
	_screen->loadBitmap("BACKGRND.CPS", 4, 4, &_screen->getPalette(0));

	if (!_chargenWSA->open("CHARGEN.WSA", 1, 0))
		error("Couldn't load CHARGEN.WSA");

	_chargenWSA->displayFrame(0, 2, 113, 0, 0, 0, 0);

	_screen->setFont(_flags.lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_9_FNT);
	_screen->_curPage = 2;

	if (_flags.platform == Common::kPlatformPC98) {
		_screen->fillRect(17, 29, 94, 97, 17);
		_screen->fillRect(68, 167, 310, 199, 17);
		_screen->drawClippedLine(68, 166, 311, 166, 238);
		_screen->drawClippedLine(68, 166, 68, 199, 238);
		_screen->drawClippedLine(311, 166, 311, 199, 238);

		_screen->_curPage = 4;
		_screen->fillRect(17, 29, 94, 97, 17);
		_screen->_curPage = 2;

		for (int i = 0; i < 4; ++i) {
			_screen->printText((const char *)_charNamesPC98[i], _charPosXPC98[i], 168, 0xC1, 0x00);

			Screen::FontId old = _screen->setFont(Screen::FID_SJIS_FNT);
			for (int j = 0; j < 3; ++j) {
				Common::String attribString = Common::String::format("%2d", _charPreviews[i].attrib[j]);
				_screen->printText(attribString.c_str(), _charPosXPC98[i] + 16, 176 + j * 8, 0x81, 0x00);
			}
			_screen->setFont(old);
		}

		_screen->printText(_tim->getCTableEntry(51), 72, 176, 0x81, 0x00);
		_screen->printText(_tim->getCTableEntry(53), 72, 184, 0x81, 0x00);
		_screen->printText(_tim->getCTableEntry(55), 72, 192, 0x81, 0x00);
	} else {
		const char *const *previewNames = (_flags.lang == Common::RU_RUS && !_flags.isTalkie) ? _charPreviewNamesRussianFloppy : _charPreviewNamesDefault;
		for (int i = 0; i < 4; ++i) {
			_screen->fprintStringIntro("%s", _charPreviews[i].x + 16, _charPreviews[i].y + 36, 0xC0, 0x00, 0x9C, 0x120, previewNames[i]);
			_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 48, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[0]);
			_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 56, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[1]);
			_screen->fprintStringIntro("%d", _charPreviews[i].x + 21, _charPreviews[i].y + 64, 0x98, 0x00, 0x9C, 0x220, _charPreviews[i].attrib[2]);
		}

		_screen->fprintStringIntro("%s", 36, 173, 0x98, 0x00, 0x9C, 0x20, _tim->getCTableEntry(51));
		_screen->fprintStringIntro("%s", 36, 181, 0x98, 0x00, 0x9C, 0x20, _tim->getCTableEntry(53));
		_screen->fprintStringIntro("%s", 36, 189, 0x98, 0x00, 0x9C, 0x20, _tim->getCTableEntry(55));
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
	_screen->_curPage = 0;

	if (_flags.use16ColorMode)
		_screen->loadPalette("LOL.NOL", _screen->getPalette(0));

	_screen->fadePalette(_screen->getPalette(0), 30, 0);

	bool kingIntro = true;
	while (!shouldQuit()) {
		if (kingIntro)
			kingSelectionIntro();

		if (_charSelection < 0)
			processCharacterSelection();

		if (shouldQuit())
			break;

		if (_charSelection == 100) {
			kingIntro = true;
			_charSelection = -1;
			continue;
		}

		_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		_screen->showMouse();

		if (selectionCharInfo(_charSelection) == -1) {
			_charSelection = -1;
			kingIntro = false;
		} else {
			break;
		}

		delay(10);
	}

	if (shouldQuit())
		return -1;

	uint32 waitTime = _system->getMillis() + 420 * _tickLength;
	while (waitTime > _system->getMillis() && !skipFlag() && !shouldQuit()) {
		updateInput();
		_system->delayMillis(10);
	}

	// HACK: Remove all input events
	_eventList.clear();

	_tim->clearLangData();

	delete _tim;
	_tim = 0;

	return _charSelection;
}

void LoLEngine::kingSelectionIntro() {
	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
	int y = 38;

	if (_flags.platform == Common::kPlatformPC98) {
		for (int i = 0; i < 5; ++i)
			_screen->printText(_tim->getCTableEntry(57 + i), 16, 32 + i * 8, 0xC1, 0x00);
	} else {
		for (int i = 0; i < 5; ++i)
			_screen->fprintStringIntro("%s", 8, y + i * 10, 0x32, 0x00, 0x9C, 0x20, _tim->getCTableEntry(57 + i));
	}

	if (_flags.isTalkie)
		_sound->voicePlay("KING01", &_speechHandle);

	int index = 4;
	while ((!speechEnabled() || (speechEnabled() && _sound->voiceIsPlaying(&_speechHandle))) && _charSelection == -1 && !shouldQuit() && !skipFlag()) {
		index = MAX(index, 4);

		_chargenWSA->displayFrame(_chargenFrameTable[index], 0, 113, 0, 0, 0, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar1IdxTable[index] * 2 + 0], _selectionPosTable[_selectionChar1IdxTable[index] * 2 + 1], _charPreviews[0].x, _charPreviews[0].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar2IdxTable[index] * 2 + 0], _selectionPosTable[_selectionChar2IdxTable[index] * 2 + 1], _charPreviews[1].x, _charPreviews[1].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar3IdxTable[index] * 2 + 0], _selectionPosTable[_selectionChar3IdxTable[index] * 2 + 1], _charPreviews[2].x, _charPreviews[2].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_selectionChar4IdxTable[index] * 2 + 0], _selectionPosTable[_selectionChar4IdxTable[index] * 2 + 1], _charPreviews[3].x, _charPreviews[3].y, 32, 32, 4, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 7 * _tickLength;
		while (waitEnd > _system->getMillis() && _charSelection == -1 && !shouldQuit() && !skipFlag()) {
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		if (speechEnabled())
			index = (index + 1) % 22;
		else if (++index >= 27)
			break;
	}

	resetSkipFlag();

	_chargenWSA->displayFrame(0x10, 0, 113, 0, 0, 0, 0);
	_screen->updateScreen();
	_sound->voiceStop(&_speechHandle);
}

void LoLEngine::kingSelectionReminder() {
	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);
	int y = 48;

	if (_flags.platform == Common::kPlatformPC98) {
		_screen->printText(_tim->getCTableEntry(62), 16, 32, 0xC1, 0x00);
		_screen->printText(_tim->getCTableEntry(63), 16, 40, 0xC1, 0x00);
	} else {
		_screen->fprintStringIntro("%s", 8, y, 0x32, 0x00, 0x9C, 0x20, _tim->getCTableEntry(62));
		_screen->fprintStringIntro("%s", 8, y + 10, 0x32, 0x00, 0x9C, 0x20, _tim->getCTableEntry(63));
	}

	if (_flags.isTalkie)
		_sound->voicePlay("KING02", &_speechHandle);

	int index = 0;
	while ((!speechEnabled() || (speechEnabled() && _sound->voiceIsPlaying(&_speechHandle))) && _charSelection == -1 && !shouldQuit() && index < 15) {
		_chargenWSA->displayFrame(_chargenFrameTable[index + 9], 0, 113, 0, 0, 0, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar1IdxTable[index] * 2 + 0], _selectionPosTable[_reminderChar1IdxTable[index] * 2 + 1], _charPreviews[0].x, _charPreviews[0].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar2IdxTable[index] * 2 + 0], _selectionPosTable[_reminderChar2IdxTable[index] * 2 + 1], _charPreviews[1].x, _charPreviews[1].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar3IdxTable[index] * 2 + 0], _selectionPosTable[_reminderChar3IdxTable[index] * 2 + 1], _charPreviews[2].x, _charPreviews[2].y, 32, 32, 4, 0);
		_screen->copyRegion(_selectionPosTable[_reminderChar4IdxTable[index] * 2 + 0], _selectionPosTable[_reminderChar4IdxTable[index] * 2 + 1], _charPreviews[3].x, _charPreviews[3].y, 32, 32, 4, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 8 * _tickLength;
		while (waitEnd > _system->getMillis() && !shouldQuit()) {
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		if (speechEnabled())
			index = (index + 1) % 22;
		else if (++index >= 27)
			break;
	}

	_sound->voiceStop(&_speechHandle);
}

void LoLEngine::kingSelectionOutro() {
	if (_flags.isTalkie)
		_sound->voicePlay("KING03", &_speechHandle);

	int index = 0;
	while ((!speechEnabled() || (speechEnabled() && _sound->voiceIsPlaying(&_speechHandle))) && !shouldQuit() && !skipFlag()) {
		index = MAX(index, 4);

		_chargenWSA->displayFrame(_chargenFrameTable[index], 0, 113, 0, 0, 0, 0);
		_screen->updateScreen();

		uint32 waitEnd = _system->getMillis() + 8 * _tickLength;
		while (waitEnd > _system->getMillis() && !shouldQuit() && !skipFlag()) {
			updateInput();
			_system->delayMillis(10);
		}

		if (speechEnabled())
			index = (index + 1) % 22;
		else if (++index >= 27)
			break;
	}

	resetSkipFlag();

	_chargenWSA->displayFrame(0x10, 0, 113, 0, 0, 0, 0);
	_screen->updateScreen();
	_sound->voiceStop(&_speechHandle);
}

void LoLEngine::processCharacterSelection() {
	_charSelection = -1;
	while (!shouldQuit() && _charSelection == -1) {
		uint32 nextKingMessage = _system->getMillis() + 900 * _tickLength;

		while (nextKingMessage > _system->getMillis() && _charSelection == -1 && !shouldQuit()) {
			updateSelectionAnims();
			_charSelection = getCharSelection();
			_system->delayMillis(10);
		}

		if (_charSelection == -1)
			kingSelectionReminder();
	}
}

void LoLEngine::updateSelectionAnims() {
	for (int i = 0; i < 4; ++i) {
		if (_system->getMillis() < _selectionAnimTimers[i])
			continue;

		const int index = _selectionAnimIndexTable[_selectionAnimFrames[i] + i * 2];
		_screen->copyRegion(_selectionPosTable[index * 2 + 0], _selectionPosTable[index * 2 + 1], _charPreviews[i].x, _charPreviews[i].y, 32, 32, 4, 0);

		int delayTime = 0;
		if (_selectionAnimFrames[i] == 1)
			delayTime = _rnd.getRandomNumberRng(0, 31) + 80;
		else
			delayTime = _rnd.getRandomNumberRng(0, 3) + 10;

		_selectionAnimTimers[i] = _system->getMillis() + delayTime * _tickLength;
		_selectionAnimFrames[i] = (_selectionAnimFrames[i] + 1) % 2;
	}

	_screen->updateScreen();
}

int LoLEngine::selectionCharInfo(int character) {
	if (character < 0)
		return -1;

	char filename[16];
	char vocFilename[6];
	strcpy(vocFilename, "000X0");

	switch (character) {
	case 0:
		strcpy(filename, "FACE09.SHP");
		vocFilename[3] = 'A';
		break;

	case 1:
		strcpy(filename, "FACE01.SHP");
		vocFilename[3] = 'M';
		break;

	case 2:
		strcpy(filename, "FACE08.SHP");
		vocFilename[3] = 'K';
		break;

	case 3:
		strcpy(filename, "FACE05.SHP");
		vocFilename[3] = 'C';
		break;

	default:
		break;
	}

	_screen->loadBitmap(filename, 9, 9, 0);
	_screen->copyRegion(0, 122, 0, 122, 320, 78, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(_charPreviews[character].x - 3, _charPreviews[character].y - 3, 8, 127, 38, 38, 2, 0);

	static const uint8 charSelectInfoIdx[] = { 0x1D, 0x22, 0x27, 0x2C };
	const int idx = charSelectInfoIdx[character];

	if (_flags.platform == Common::kPlatformPC98) {
		for (int i = 0; i < 5; ++i)
			_screen->printText(_tim->getCTableEntry(idx + i), 60, 128 + i * 8, 0x41, 0x00);

		_screen->printText(_tim->getCTableEntry(69), 112, 168, 0x01, 0x00);
	} else {
		for (int i = 0; i < 5; ++i)
			_screen->fprintStringIntro("%s", 50, 127 + i * 10, 0x53, 0x00, 0xCF, 0x20, _tim->getCTableEntry(idx + i));

		_screen->fprintStringIntro("%s", 100, 168, 0x32, 0x00, 0xCF, 0x20, _tim->getCTableEntry(69));
	}

	selectionCharInfoIntro(vocFilename);
	if (_charSelectionInfoResult == -1) {
		while (_charSelectionInfoResult == -1 && !shouldQuit()) {
			_charSelectionInfoResult = selectionCharAccept();
			_system->delayMillis(10);
		}
	}

	if (_charSelectionInfoResult != 1) {
		_charSelectionInfoResult = -1;
		_screen->copyRegion(0, 122, 0, 122, 320, 78, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
		return -1;
	}

	_screen->copyRegion(48, 127, 48, 127, 272, 60, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->hideMouse();
	_screen->copyRegion(48, 127, 48, 160, 272, 35, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(0, 0, 0, 0, 112, 120, 4, 0, Screen::CR_NO_P_CHECK);

	if (_flags.platform == Common::kPlatformPC98) {
		for (int i = 0; i < 5; ++i)
			_screen->printText(_tim->getCTableEntry(64 + i), 16, 32 + i * 8, 0xC1, 0x00);
	} else {
		for (int i = 0; i < 5; ++i)
			_screen->fprintStringIntro("%s", 3, 28 + i * 10, 0x32, 0x00, 0x9C, 0x20, _tim->getCTableEntry(64 + i));
	}

	resetSkipFlag();
	kingSelectionOutro();
	return character;
}

void LoLEngine::selectionCharInfoIntro(char *file) {
	int index = 0;
	file[4] = '0';
	bool processAnim = true;

	while (_charSelectionInfoResult == -1 && !shouldQuit()) {
		if (speechEnabled() && !_sound->isVoicePresent(file))
			break;

		if (_flags.isTalkie)
			_sound->voicePlay(file, &_speechHandle);

		int i = 0;
		while ((!speechEnabled() || (speechEnabled() && _sound->voiceIsPlaying(&_speechHandle))) && _charSelectionInfoResult == -1 && !shouldQuit()) {
			_screen->drawShape(0, _screen->getPtrToShape(_screen->getCPagePtr(9), _charInfoFrameTable[i]), 11, 130, 0, 0);
			_screen->updateScreen();

			uint32 nextFrame = _system->getMillis() + 8 * _tickLength;
			while (nextFrame > _system->getMillis() && _charSelectionInfoResult == -1 && !shouldQuit()) {
				_charSelectionInfoResult = selectionCharAccept();
				_system->delayMillis(10);
			}

			if (speechEnabled() || processAnim)
				i = (i + 1) % 32;
			if (i == 0)
				processAnim = false;
		}

		_sound->voiceStop(&_speechHandle);
		file[4] = ++index + '0';
	}

	_screen->drawShape(0, _screen->getPtrToShape(_screen->getCPagePtr(9), 0), 11, 130, 0, 0);
	_screen->updateScreen();
}

int LoLEngine::getCharSelection() {
	int inputFlag = checkInput(0, false) & 0xCF;
	removeInputTop();

	if (inputFlag == 200) {
		for (int i = 0; i < 4; ++i) {
			if (_charPreviews[i].x <= _mouseX && _mouseX <= _charPreviews[i].x + 31 &&
			        _charPreviews[i].y <= _mouseY && _mouseY <= _charPreviews[i].y + 31)
				return i;
		}
	}

	return -1;
}

int LoLEngine::selectionCharAccept() {
	int inputFlag = checkInput(0, false) & 0xCF;
	removeInputTop();

	if (inputFlag == 200) {
		if (88 <= _mouseX && _mouseX <= 128 && 180 <= _mouseY && _mouseY <= 194)
			return 1;
		if (196 <= _mouseX && _mouseX <= 236 && 180 <= _mouseY && _mouseY <= 194)
			return 0;
	}

	return -1;
}

void LoLEngine::showStarcraftLogo() {
	WSAMovie_v2 *ci = new WSAMovie_v2(this);
	assert(ci);

	_screen->clearPage(0);
	_screen->clearPage(2);

	int endframe = ci->open("ci01.wsa", 0, &_screen->getPalette(0));
	if (!ci->opened()) {
		delete ci;
		return;
	}
	_screen->hideMouse();
	ci->displayFrame(0, 2, 32, 80, 0, 0, 0);
	_screen->copyPage(2, 0);
	_screen->fadeFromBlack();
	int inputFlag = 0;
	for (int i = 0; i < endframe; i++) {
		inputFlag = checkInput(0) & 0xff;
		if (shouldQuit() || inputFlag)
			break;
		ci->displayFrame(i, 2, 32, 80, 0, 0, 0);
		_screen->copyPage(2, 0);
		_screen->updateScreen();
		delay(4 * _tickLength);
	}

	if (!(shouldQuit() || inputFlag)) {
		_sound->voicePlay("star2", &_speechHandle);
		while (_sound->voiceIsPlaying(&_speechHandle) && !(shouldQuit() || inputFlag)) {
			inputFlag = checkInput(0) & 0xff;
			delay(_tickLength);
		}
	}

	_screen->fadeToBlack();
	_screen->showMouse();

	_eventList.clear();
	delete ci;
}

// history player

HistoryPlayer::HistoryPlayer(LoLEngine *vm) : _system(vm->_system), _vm(vm), _screen(vm->screen()) {
	_x = _y = _width = _height = 0;
	_frame = _fireFrame = 0;
	_nextFireTime = 0;

	_wsa = new WSAMovie_v2(vm);
	assert(_wsa);
	_fireWsa = new WSAMovie_v2(vm);
	assert(_fireWsa);
}

HistoryPlayer::~HistoryPlayer() {
	delete _wsa;
	delete _fireWsa;
}

void HistoryPlayer::play() {
	int dataSize = 0;
	const char *data = (const char *)_vm->staticres()->loadRawData(kLoLHistory, dataSize);

	if (!data)
		error("Could not load history data");

	_screen->loadFont(Screen::FID_9_FNT, "FONT9P.FNT");

	Palette pal(256);
	pal.fill(0, 256, 0);
	_screen->fadePalette(pal, 0x1E);

	_screen->loadBitmap("BACKGND.CPS", 8, 8, &pal);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 8, 0, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 8, 2, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();

	_screen->fadePalette(pal, 0x82);

	_screen->copyRegion(_x, _y, _x, _y, _width, _height, 2, 0);
	_screen->updateScreen();

	pal.fill(0, 256, 0);
	_screen->setFont(Screen::FID_9_FNT);

	char tempWsaFilename[16];
	char voiceFilename[13];
	// the 'a' *has* to be lowercase
	strcpy(voiceFilename, "PS_1a");

	int part = 0;
	Sound *sound = _vm->sound();

	Common::Functor0Mem<void, HistoryPlayer> palFade(this, &HistoryPlayer::updateFire);

	for (; voiceFilename[3] <= '9' && !_vm->shouldQuit() && !_vm->skipFlag(); ++voiceFilename[3], voiceFilename[4] = 'a') {
		while (!_vm->shouldQuit() && !_vm->skipFlag()) {
			if (!sound->isVoicePresent(voiceFilename))
				break;

			if (data[part * 15] == voiceFilename[3] && data[part * 15 + 1] == voiceFilename[4]) {
				switch (part) {
				case 0:
					loadWsa(&data[part * 15 + 2]);
					playWsa(true);
					sound->voicePlay(voiceFilename);
					break;

				case 1: case 2: case 8:
				case 16: case 25:
					sound->voicePlay(voiceFilename);
					playWsa(true);
					break;

				case 3: case 7: case 10:
				case 17: case 23: case 26:
					sound->voicePlay(voiceFilename);
					playWsa(true);
					restoreWsaBkgd();
					loadWsa(&data[part * 15 + 2]);
					playWsa(true);
					break;

				case 6:
					sound->voicePlay(voiceFilename);
					playWsa(false);
					restoreWsaBkgd();
					loadWsa(&data[part * 15 + 2]);
					playWsa(true);
					_vm->delayWithTicks(30);
					playWsa(true);
					break;

				case 9:
					sound->voicePlay(voiceFilename);
					loadWsa(&data[part * 15 + 2]);
					playWsa(true);
					break;

				case 22:
					playWsa(false);
					restoreWsaBkgd();
					loadWsa(&data[part * 15 + 2]);
					_vm->delayWithTicks(30);
					sound->voicePlay(voiceFilename);
					playWsa(true);

					strcpy(tempWsaFilename, &data[part * 15]);

					for (int i = 1; i < 4 && !_vm->shouldQuit(); ++i) {
						uint32 nextTime = _system->getMillis() + 30 * _vm->tickLength();
						tempWsaFilename[8] = 'a' + i;

						loadWsa(&tempWsaFilename[2]);
						_vm->delayUntil(nextTime);

						playWsa(true);
					}

					tempWsaFilename[8] = 'e';
					loadWsa(&tempWsaFilename[2]);
					break;

				case 29:
					sound->voicePlay(voiceFilename);
					playWsa(false);
					restoreWsaBkgd();
					loadWsa(&data[part * 15 + 2]);

					_fireWsa->open("FIRE.WSA", 0, 0);
					playWsa(true);
					_fireFrame = 0;

					for (int i = 0; i < 12 && !_vm->shouldQuit(); ++i, ++_fireFrame) {
						uint32 nextTime = _system->getMillis() + 3 * _vm->tickLength();

						if (_fireFrame > 4)
							_fireFrame = 0;

						_fireWsa->displayFrame(_fireFrame, 0, 75, 51, 0, 0, 0);
						_screen->updateScreen();
						_vm->delayUntil(nextTime);
					}

					_screen->loadPalette("DRACPAL.PAL", pal);
					_screen->fadePalette(pal, 0x78, &palFade);

					while (sound->voiceIsPlaying() && !_vm->shouldQuit()) {
						uint32 nextTime = _system->getMillis() + 3 * _vm->tickLength();

						++_fireFrame;
						if (_fireFrame > 4)
							_fireFrame = 0;

						_fireWsa->displayFrame(_fireFrame, 0, 75, 51, 0, 0, 0);
						_screen->updateScreen();
						_vm->delayUntil(nextTime);
					}

					_fireFrame = 0;
					for (int i = 0; i < 10; ++i, ++_fireFrame) {
						uint32 nextTime = _system->getMillis() + 3 * _vm->tickLength();

						if (_fireFrame > 4)
							_fireFrame = 0;

						_fireWsa->displayFrame(_fireFrame, 0, 75, 51, 0, 0, 0);
						_screen->updateScreen();
						_vm->delayUntil(nextTime);
					}

					break;

				default:
					sound->voicePlay(voiceFilename);
					playWsa(false);
					restoreWsaBkgd();
					loadWsa(&data[part * 15 + 2]);
					playWsa(true);
					break;
				}

				++part;
			} else {
				sound->voicePlay(voiceFilename);
			}

			while (sound->voiceIsPlaying() && !_vm->shouldQuit() && !_vm->skipFlag())
				_vm->delay(10);

			if (_vm->skipFlag())
				sound->voiceStop();

			++voiceFilename[4];
		}
	}

	if (_vm->skipFlag())
		_vm->_eventList.clear();

	pal.fill(0, 256, 63);
	if (_fireWsa->opened())
		_screen->fadePalette(pal, 0x3C, &palFade);
	else
		_screen->fadePalette(pal, 0x3C);

	_screen->clearPage(0);
	pal.fill(0, 256, 0);
	_screen->fadePalette(pal, 0x3C);

	if (_vm->skipFlag())
		_vm->_eventList.clear();
}

void HistoryPlayer::loadWsa(const char *filename) {
	if (_wsa->opened())
		_wsa->close();

	Palette pal(256);
	if (!_wsa->open(filename, 3, &pal))
		error("Could not load WSA file: '%s'", filename);
	_screen->setScreenPalette(pal);

	_x = _wsa->xAdd();
	_y = _wsa->yAdd();
	_width = _wsa->width();
	_height = _wsa->height();
	_frame = 1;
}

void HistoryPlayer::playWsa(bool direction) {
	const int tickLength = _vm->tickLength();

	for (int i = 0; i < 15 && !_vm->shouldQuit(); ++i) {
		uint32 nextTime = _system->getMillis() + 3 * tickLength;

		_wsa->displayFrame(_frame, 2, 0, 0, 0, 0, 0);
		_screen->copyRegion(_x, _y, _x, _y, _width, _height, 2, 0);
		_screen->updateScreen();
		_vm->delayUntil(nextTime);

		if (direction)
			++_frame;
		else
			--_frame;
	}
}

void HistoryPlayer::restoreWsaBkgd() {
	_screen->copyRegion(_x, _y, _x, _y, _width, _height, 8, 0);
	_screen->copyRegion(_x, _y, _x, _y, _width, _height, 8, 2);
	_screen->updateScreen();
}

void HistoryPlayer::updateFire() {
	if (_system->getMillis() > _nextFireTime) {
		_fireWsa->displayFrame(_fireFrame, 0, 75, 51, 0, 0, 0);
		_fireFrame = (_fireFrame + 1) % 5;
		_nextFireTime = _system->getMillis() + 4 * _vm->tickLength();
	}

	_screen->updateScreen();
}

// outro

void LoLEngine::setupEpilogueData(bool load) {
	static const char *const fileListCD[] = {
		"GENERAL.PAK", "INTROVOC.PAK", "STARTUP.PAK",
		"FINALE.PAK", "FINALE1.PAK", "FINALE2.PAK", 0
	};

	static const char *const fileListFloppy[] = {
		"GENERAL.PAK", "INTRO.PAK", "FINALE1.PAK", "FINALE2.PAK", 0
	};

	const char *const *fileList = _flags.isTalkie ? fileListCD : fileListFloppy;
	assert(fileList);

	char filename[32];
	for (uint i = 0; fileList[i]; ++i) {
		filename[0] = '\0';

		if (_flags.isTalkie) {
			strcpy(filename, _languageExt[_lang]);
			strcat(filename, "/");
		}

		strcat(filename, fileList[i]);

		if (load) {
			if (!_res->loadPakFile(filename))
				error("Couldn't load file: '%s'", filename);
		} else {
			_res->unloadPakFile(filename);
		}
	}

	_screen->clearPage(0);
	_screen->clearPage(3);

	if (load) {
		_sound->selectAudioResourceSet(kMusicFinale);

		// We have three sound.dat files, one for the intro, one for the
		// end sequence and one for ingame, each contained in a different
		// PAK file. Therefore a new call to loadSoundFile() is required
		// whenever the PAK file configuration changes.
		if (_flags.platform == Common::kPlatformPC98)
			_sound->loadSoundFile("SOUND.DAT");
	} else {
		_screen->getPalette(0).clear();
		_screen->setScreenPalette(_screen->getPalette(0));

		if (shouldQuit())
			return;

		_eventList.clear();
		_sound->selectAudioResourceSet(kMusicIntro);
	}
}

void LoLEngine::showOutro(int character, bool maxDifficulty) {
	setupEpilogueData(true);
	TIMInterpreter *timBackUp = _tim;
	_tim = new TIMInterpreter(this, _screen, _system);

	_screen->getPalette(0).clear();
	_screen->setScreenPalette(_screen->getPalette(0));

	_screen->clearPage(0);
	_screen->clearPage(4);
	_screen->clearPage(8);

	TIM *outro = _tim->load("LOLFINAL.TIM", &_timOutroOpcodes);
	assert(outro);
	outro->lolCharacter = character;

	_screen->loadFont(Screen::FID_6_FNT, "NEW6P.FNT");
	_screen->loadFont(Screen::FID_INTRO_FNT, "INTRO.FNT");

	_tim->resetFinishedFlag();
	_tim->setLangData("LOLFINAL.DIP");

	_screen->hideMouse();

	uint32 palNextFadeStep = 0;
	while (!_tim->finished() && !shouldQuit() && !skipFlag()) {
		updateInput();
		_tim->exec(outro, false);

		if (_tim->_palDiff) {
			if (palNextFadeStep < _system->getMillis()) {
				_tim->_palDelayAcc += _tim->_palDelayInc;
				palNextFadeStep = _system->getMillis() + ((_tim->_palDelayAcc >> 8) * _tickLength);
				_tim->_palDelayAcc &= 0xFF;

				if (!_screen->fadePalStep(_screen->getPalette(0), _tim->_palDiff)) {
					_screen->setScreenPalette(_screen->getPalette(0));
					_tim->_palDiff = 0;
				}
			}
		}

		_system->delayMillis(10);
		_screen->updateScreen();
	}
	removeInputTop();
	_screen->showMouse();
	_sound->voiceStop();
	_sound->beginFadeOut();

	_eventList.clear();

	_tim->unload(outro);

	for (int i = 0; i < TIM::kWSASlots; i++)
		_tim->freeAnimStruct(i);

	_screen->fadeToBlack(30);

	if (!shouldQuit())
		showCredits();

	_eventList.clear();

	if (!shouldQuit()) {
		switch (character) {
		case 0:
			_screen->loadBitmap("KIERAN.CPS", 3, 3, &_screen->getPalette(0));
			break;

		case 1:
			_screen->loadBitmap("AK'SHEL.CPS", 3, 3, &_screen->getPalette(0));
			break;

		case 2:
			_screen->loadBitmap("MICHAEL.CPS", 3, 3, &_screen->getPalette(0));
			break;

		case 3:
			_screen->loadBitmap("CONRAD.CPS", 3, 3, &_screen->getPalette(0));
			break;

		default:
			_screen->clearPage(3);
			_screen->getPalette(0).clear();
		}

		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
		if (maxDifficulty && !_flags.use16ColorMode)
			_tim->displayText(0x8000, 0, 0xDC);
		_screen->updateScreen();
		_screen->fadePalette(_screen->getPalette(0), 30, 0);

		while (!checkInput(0) && !shouldQuit())
			delay(_tickLength);

		_screen->fadeToBlack(30);
	}

	_tim->clearLangData();
	delete _tim;
	_tim = timBackUp;

	setupEpilogueData(false);
}

void LoLEngine::showCredits() {
	for (int i = 0; i < 255; ++i)
		_outroShapeTable[i] = i;

	if (_flags.use16ColorMode)
		for (int i = 1; i < 16; ++i)
			_outroShapeTable[i] = (i << 4) | i;
	else
		_outroShapeTable[255] = 0;

	_sound->haltTrack();
	_sound->loadSoundFile("LOREFINL");
	_sound->playTrack(4);

	_screen->hideMouse();

	static const uint8 colorMap[] = { 0x00, 0x00, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x72, 0x6F, 0x6F, 0x6D };
	_screen->_charWidth = 0;

	_screen->loadBitmap("ROOM.CPS", 2, 2, &_screen->getPalette(0));

	if (!_flags.use16ColorMode) {
		_screen->setTextColorMap(colorMap);
		_screen->getPalette(0).fill(_screen->getPalette(0).getNumColors() - 1, 1, 0);
	}

	_screen->fadeToBlack(30);

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

	_screen->_charOffset = 0;

	char *credits = 0;

	if (_flags.platform == Common::kPlatformPC98) {
		int size = 0;
		const uint8 *internCredits = _staticres->loadRawData(kLoLCredits, size);
		assert(size > 0);

		credits = new char[size];
		assert(credits);

		memcpy(credits, internCredits, size);
		_staticres->unloadId(kLoLCredits);
	} else {
		credits = (char *)_res->fileData("CREDITS.TXT", 0);
	}

	processCredits(credits, 21, 4, 5);
	delete[] credits;

	uint32 endTime = _system->getMillis() + 120 * _tickLength;
	while (endTime > _system->getMillis() && !shouldQuit()) {
		if (checkInput(0))
			break;
		delay(_tickLength);
	}

	_sound->beginFadeOut();
	_screen->fadeToBlack(30);

	_screen->clearCurPage();
	_screen->updateScreen();
	_screen->showMouse();
}

void LoLEngine::processCredits(char *t, int dimState, int page, int delayTime) {
	if (!t)
		return;

	_screen->setScreenDim(dimState);
	_screen->clearPage(page);
	_screen->clearPage(6);

	_screen->loadBitmap("DOOR.SHP", 5, 5, 0);
	uint8 *doorShape = _screen->makeShapeCopy(_screen->getCPagePtr(5), 0);
	assert(doorShape);

	_screen->drawShape(0, doorShape, 0, 0, 22, 0x10);
	_screen->drawShape(0, doorShape, 0, 0, 23, 0x11);

	int curShapeFile = 0;
	uint8 *shapes[12];
	memset(shapes, 0, sizeof(shapes));

	loadOutroShapes(curShapeFile++, shapes);
	uint8 *monsterPal = 0;

	if (_flags.use16ColorMode) {
		_screen->loadPalette("LOL.NOL", _screen->getPalette(0));
	} else {
		monsterPal = _res->fileData("MONSTERS.PAL", 0);
		assert(monsterPal);
		_screen->getPalette(0).copy(monsterPal, 0, 40, 88);
	}

	_screen->fadePalette(_screen->getPalette(0), 30);

	uint32 waitTimer = _system->getMillis();

	struct CreditsString {
		int16 x, y;
		char *str;
		uint8 code;
		uint8 height;
		uint8 alignment;
	} strings[37];
	memset(strings, 0, sizeof(strings));

	int countStrings = 0;
	char *str = t;

	int frameCounter = 0;
	int monsterAnimFrame = 0;
	bool needNewShape = false;
	bool doorRedraw = true;

	uint8 *animBlock = new uint8[40960];
	assert(animBlock);
	memset(animBlock, 0, 40960);
	int inputFlag = 0;

	do {
		while (_system->getMillis() < waitTimer && !shouldQuit())
			delay(_tickLength);
		waitTimer = _system->getMillis() + delayTime * _tickLength;

		while (countStrings < 35 && str[0]) {
			int y = 0;

			if (!countStrings) {
				y = _screen->_curDim->h;
			} else {
				y = strings[countStrings].y + strings[countStrings].height;
				y += strings[countStrings].height >> 3;
			}

			char *curString = str;
			str = (char *)strpbrk(str, "\x05\x0D");
			if (!str)
				str = strchr(curString, 0);

			CreditsString &s = strings[countStrings + 1];
			s.code = str[0];
			str[0] = 0;

			if (s.code)
				++str;

			s.alignment = 0;
			if (*curString == 3 || *curString == 4)
				s.alignment = *curString++;

			_screen->setFont(Screen::FID_6_FNT);

			if (*curString == 1 || *curString == 2)
				++curString;
			s.height = _screen->getFontHeight();

			if (s.alignment == 3)
				s.x = 0;
			else if (s.alignment == 4)
				s.x = 300 - _screen->getTextWidth(curString);
			else
				s.x = ((_screen->_curDim->w << 3) - _screen->getTextWidth(curString)) / 2;

			if (strings[countStrings].code == 5)
				y -= strings[countStrings].height + (strings[countStrings].height >> 3);

			s.y = y;
			s.str = curString;

			// WORKAROUND: The original did supply some texts, which wouldn't fit on one line.
			// To display them properly, we will break them into two separate entries. The original
			// just did not display these lines at all. (At least not in LordHoto's tests with DOSBox).
			if (s.x + _screen->getTextWidth(s.str) > Screen::SCREEN_W) {
				char *nextLine = 0;
				char *lastSeparator = 0;

				int backupX = s.x;

				while (s.x + _screen->getTextWidth(s.str) > Screen::SCREEN_W) {
					char *sep = strrchr(s.str, ' ');

					if (lastSeparator)
						*lastSeparator = ' ';

					lastSeparator = sep;

					if (lastSeparator) {
						*lastSeparator = 0;
						nextLine = lastSeparator + 1;

						s.x = MAX(((_screen->_curDim->w << 3) - _screen->getTextWidth(s.str)) / 2, 0);
					} else {
						// It seems we ca not find any whitespace, thus we are better safe and
						// do not break up the line into two parts. (This is just paranoia)
						nextLine = 0;
						break;
					}
				}

				s.x = backupX;

				if (nextLine) {
					++countStrings;

					// Center old string
					s.alignment = 0;
					s.x = ((_screen->_curDim->w << 3) - _screen->getTextWidth(s.str)) / 2;

					// Add new string, also centered
					CreditsString &n = strings[countStrings + 1];
					n.y = s.y + s.height + (s.height >> 3);
					n.height = s.height;
					n.alignment = 0;
					n.code = s.code;
					n.str = nextLine;
					n.x = ((_screen->_curDim->w << 3) - _screen->getTextWidth(n.str)) / 2;
				}
			}

			++countStrings;
		}

		++frameCounter;
		if (frameCounter % 3) {
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 6, page, Screen::CR_NO_P_CHECK);
		} else {
			if (!monsterAnimFrame && doorRedraw) {
				_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, page, Screen::CR_NO_P_CHECK);
				_screen->drawShape(page, doorShape, 0, 0, 22, 0x10);
				_screen->drawShape(page, doorShape, 0, 0, 23, 0x11);

				--frameCounter;
				doorRedraw = false;
			} else {
				if (!monsterAnimFrame)
					_screen->setScreenPalette(_screen->getPalette(0));

				_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, page, Screen::CR_NO_P_CHECK);

				uint8 *monsterShape = shapes[_outroFrameTable[monsterAnimFrame]];

				int doorSD = 0;
				int doorX = 0, doorY = 0;
				int monsterX = 0, monsterY = 0;

				bool isRightMonster = ((curShapeFile - 1) & 1) != 0;

				if (isRightMonster) {
					doorSD = 23;
					doorX = _outroRightDoorPos[monsterAnimFrame * 2 + 0];
					doorY = _outroRightDoorPos[monsterAnimFrame * 2 + 1];

					monsterX = _outroRightMonsterPos[monsterAnimFrame * 2 + 0];
					monsterY = _outroRightMonsterPos[monsterAnimFrame * 2 + 1];

					_screen->drawShape(page, doorShape, 0, 0, 22, 0x10);
				} else {
					doorSD = 22;
					doorX = _outroLeftDoorPos[monsterAnimFrame * 2 + 0];
					doorY = _outroLeftDoorPos[monsterAnimFrame * 2 + 1];

					monsterX = _outroLeftMonsterPos[monsterAnimFrame * 2 + 0];
					monsterY = _outroLeftMonsterPos[monsterAnimFrame * 2 + 1];

					_screen->drawShape(page, doorShape, 0, 0, 23, 0x11);
				}

				if (monsterAnimFrame >= 8)
					_screen->drawShape(page, doorShape, doorX, doorY, doorSD, (doorSD == 22) ? 0 : 1);

				_screen->drawShape(page, monsterShape, monsterX, monsterY, 0, 0x104 | ((!isRightMonster || monsterAnimFrame < 20) ? 0 : 1), _outroShapeTable, 1, _outroMonsterScaleTableX[monsterAnimFrame], _outroMonsterScaleTableY[monsterAnimFrame]);

				if (monsterAnimFrame < 8)
					_screen->drawShape(page, doorShape, doorX, doorY, doorSD, (doorSD == 22) ? 0 : 1);

				_screen->copyRegion(0, 0, 0, 0, 320, 200, page, 6, Screen::CR_NO_P_CHECK);
				doorRedraw = true;

				monsterAnimFrame = (monsterAnimFrame + 1) % 24;
				needNewShape = !monsterAnimFrame;
			}
		}

		for (int i = 0; i < countStrings; ++i) {
			CreditsString &s = strings[i + 1];
			int x = s.x, y = s.y;

			if (y < _screen->_curDim->h) {
				_screen->_curPage = page;
				_screen->setFont(Screen::FID_6_FNT);
				if (_flags.use16ColorMode) {
					_screen->printText(s.str, (_screen->_curDim->sx << 3) + x + 1, _screen->_curDim->sy + y + 1, 0x44, 0x00);
					_screen->printText(s.str, (_screen->_curDim->sx << 3) + x, _screen->_curDim->sy + y, 0x33, 0x00);
				} else {
					_screen->printText(s.str, (_screen->_curDim->sx << 3) + x, _screen->_curDim->sy + y, 0xDC, 0x00);
				}
				_screen->_curPage = 0;
			}

			--s.y;
		}

		_screen->copyToPage0(_screen->_curDim->sy, _screen->_curDim->h, page, animBlock);

		if (strings[1].y < -10) {
			strings[1].str += strlen(strings[1].str);
			strings[1].str[0] = strings[1].code;
			--countStrings;
			memmove(&strings[1], &strings[2], countStrings * sizeof(CreditsString));
		}

		if (needNewShape) {
			++curShapeFile;
			if (curShapeFile == 16)
				curShapeFile += 2;
			if (curShapeFile == 6)
				curShapeFile += 2;
			curShapeFile = curShapeFile % 28;

			loadOutroShapes(curShapeFile, shapes);

			if (!_flags.use16ColorMode) {
				_screen->getPalette(0).copy(monsterPal, curShapeFile * 40, 40, 88);
				_screen->setScreenPalette(_screen->getPalette(0));
			}

			needNewShape = false;
		}

		_screen->updateScreen();
		inputFlag = checkInput(0);
		removeInputTop();
	} while (countStrings && !(inputFlag && !(inputFlag & 0x800)) && !shouldQuit());
	removeInputTop();

	delete[] animBlock;
	delete[] doorShape;
	delete[] monsterPal;
	for (int i = 0; i < 12; ++i)
		delete[] shapes[i];
}

void LoLEngine::loadOutroShapes(int file, uint8 **storage) {
	_screen->loadBitmap(_outroShapeFileTable[file], 5, 5, 0);

	for (int i = 0; i < 12; ++i) {
		delete[] storage[i];
		if (i < 8)
			storage[i] = _screen->makeShapeCopy(_screen->getCPagePtr(5), i);
		else
			storage[i] = _screen->makeShapeCopy(_screen->getCPagePtr(5), i + 4);
	}
}

} // End of namespace Kyra

#endif // ENABLE_LOL
