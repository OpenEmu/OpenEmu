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

#ifdef ENABLE_EOB

#include "kyra/darkmoon.h"
#include "kyra/screen_eob.h"
#include "kyra/resource.h"
#include "kyra/sound.h"

#include "common/system.h"

#include "base/version.h"

namespace Kyra {

class DarkmoonSequenceHelper {
friend class DarkMoonEngine;
public:
	enum Mode {
		kIntro,
		kFinale
	};

	struct Config {
		Config(Mode m, const char *const *str, const char *const *cps, const char *const *pal, const DarkMoonShapeDef **shp, const DarkMoonAnimCommand **anim, bool paletteFading) : mode(m), strings(str), cpsFiles(cps), palFiles(pal), shapeDefs(shp), animData(anim), palFading(paletteFading) {}
		Mode mode;
		const char *const *strings;
		const char *const *cpsFiles;
		const char *const *palFiles;
		const DarkMoonShapeDef **shapeDefs;
		const DarkMoonAnimCommand **animData;
		bool palFading;
	};

	DarkmoonSequenceHelper(OSystem *system, DarkMoonEngine *vm, Screen_EoB *screen, const Config *config);
	~DarkmoonSequenceHelper();

	void loadScene(int index, int pageNum);
	void animCommand(int index, int del = -1);

	void printText(int index, int color);
	void fadeText();

	void update(int srcPage);

	void setPalette(int index);
	void fadePalette(int index, int del);
	void copyPalette(int srcIndex, int destIndex);

	void initDelayedPaletteFade(int palIndex, int rate);
	bool processDelayedPaletteFade();

	void delay(uint32 ticks);
	void waitForSongNotifier(int index, bool introUpdateAnim = false);

private:
	void setPaletteWithoutTextColor(int index);

	OSystem *_system;
	DarkMoonEngine *_vm;
	Screen_EoB *_screen;
	const Config *_config;

	Palette *_palettes[12];

	const uint8 **_shapes;

	uint32 _fadePalTimer;
	int _fadePalRate;
	int _fadePalIndex;
};

int DarkMoonEngine::mainMenu() {
	int menuChoice = _menuChoiceInit;
	_menuChoiceInit = 0;

	_sound->loadSoundFile("INTRO");
	Screen::FontId of = _screen->_currentFont;
	int op = 0;
	Common::SeekableReadStream *s = 0;

	while (menuChoice >= 0 && !shouldQuit()) {
		switch (menuChoice) {
		case 0: {
			s = _res->createReadStream("XENU.CPS");
			if (s) {
				s->read(_screen->getPalette(0).getData(), 768);
				_screen->loadFileDataToPage(s, 3, 64000);
				delete s;
			} else {
				_screen->loadBitmap("MENU.CPS", 3, 3, &_screen->getPalette(0));
			}

			if (_configRenderMode == Common::kRenderEGA)
				_screen->loadPalette("MENU.EGA", _screen->getPalette(0));

			_screen->setScreenPalette(_screen->getPalette(0));
			_screen->convertPage(3, 2, 0);

			of = _screen->setFont(Screen::FID_6_FNT);
			op = _screen->setCurPage(2);
			Common::String versionString(Common::String::format("ScummVM %s", gScummVMVersion));
			_screen->printText(versionString.c_str(), 267 - versionString.size() * 6, 160, 13, 0);
			_screen->setFont(of);
			_screen->_curPage = op;
			_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			_allowImport = true;
			menuChoice = mainMenuLoop();
			_allowImport = false;
		} break;

		case 1:
			// load game in progress
			menuChoice = -1;
			break;

		case 2:
			// create new party
			menuChoice = -2;
			break;

		case 3:
			// transfer party
			//seq_playFinale();
			menuChoice = -3;
			break;

		case 4:
			// play intro
			seq_playIntro();
			menuChoice = 0;
			break;

		case 5:
			// quit
			menuChoice = -5;
			break;
		}
	}

	return shouldQuit() ? -5 : menuChoice;
}

int DarkMoonEngine::mainMenuLoop() {
	int sel = -1;
	do {
		_screen->setScreenDim(6);
		_gui->simpleMenu_setup(6, 0, _mainMenuStrings, -1, 0, 0);

		while (sel == -1 && !shouldQuit())
			sel = _gui->simpleMenu_process(6, _mainMenuStrings, 0, -1, 0);
	} while ((sel < 0 || sel > 5) && !shouldQuit());

	return sel + 1;
}

void DarkMoonEngine::seq_playIntro() {
	DarkmoonSequenceHelper::Config config(DarkmoonSequenceHelper::kIntro, _introStrings, _cpsFilesIntro, _configRenderMode == Common::kRenderEGA ? _palFilesIntroEGA : _palFilesIntroVGA, _shapesIntro,	_animIntro,	false);
	DarkmoonSequenceHelper sq(_system, this, _screen, &config);

	_screen->setCurPage(0);
	_screen->clearCurPage();

	snd_stopSound();

	sq.loadScene(4, 2);
	sq.loadScene(0, 2);
	sq.delay(1);

	if (!skipFlag() && !shouldQuit())
		snd_playSong(12);

	_screen->copyRegion(0, 0, 8, 8, 304, 128, 2, 0, Screen::CR_NO_P_CHECK);
	sq.setPalette(9);
	sq.fadePalette(0, 3);

	_screen->setCurPage(2);
	_screen->setClearScreenDim(17);
	_screen->setCurPage(0);

	removeInputTop();
	sq.delay(18);

	sq.animCommand(3, 18);
	sq.animCommand(6, 18);
	sq.animCommand(0);

	sq.waitForSongNotifier(1);

	sq.animCommand(_configRenderMode == Common::kRenderEGA ? 12 : 11);
	sq.animCommand(7, 6);
	sq.animCommand(2, 6);

	sq.waitForSongNotifier(2);

	sq.animCommand(_configRenderMode == Common::kRenderEGA ? 39 : 38);
	sq.animCommand(3);
	sq.animCommand(8);
	sq.animCommand(1, 10);
	sq.animCommand(0, 6);
	sq.animCommand(2);

	sq.waitForSongNotifier(3);

	_screen->setClearScreenDim(17);
	_screen->setCurPage(2);
	_screen->setClearScreenDim(17);
	_screen->setCurPage(0);

	sq.animCommand(_configRenderMode == Common::kRenderEGA ? 41 : 40);
	sq.animCommand(7, 18);

	sq.printText(0, 16);    // You were settling...
	sq.animCommand(7, 90);
	sq.fadeText();

	sq.printText(1, 16);    // Then a note was slipped to you
	sq.animCommand(8);
	sq.animCommand(2, 72);
	sq.fadeText();

	sq.printText(2, 16);    // It was from your friend Khelben Blackstaff...
	sq.animCommand(2);
	sq.animCommand(6, 36);
	sq.animCommand(3);
	sq.fadeText();

	sq.printText(3, 16);    // The message was urgent.

	sq.loadScene(1, 2);
	sq.waitForSongNotifier(4);

	// intro scroll
	if (!skipFlag() && !shouldQuit()) {
		if (_configRenderMode == Common::kRenderEGA) {
			for (int i = 0; i < 35; i++) {
				uint32 endtime = _system->getMillis() + 2 * _tickLength;
				_screen->copyRegion(16, 8, 8, 8, 296, 128, 0, 0, Screen::CR_NO_P_CHECK);
				_screen->copyRegion(i << 3, 0, 304, 8, 8, 128, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->updateScreen();
				if (i == 12)
					sq.animCommand(42);
				else if (i == 25)
					snd_playSoundEffect(11);
				delayUntil(endtime);
			}
		} else {
			for (int i = 0; i < 280; i += 3) {
				uint32 endtime = _system->getMillis() + _tickLength;
				_screen->copyRegion(11, 8, 8, 8, 301, 128, 0, 0, Screen::CR_NO_P_CHECK);
				_screen->copyRegion(i, 0, 309, 8, 3, 128, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->updateScreen();
				if (i == 96)
					sq.animCommand(42);
				delayUntil(endtime);
			}
		}
	}

	_screen->copyRegion(8, 8, 0, 0, 304, 128, 0, 2, Screen::CR_NO_P_CHECK);
	sq.animCommand(4);
	sq.fadeText();
	sq.delay(10);

	sq.loadScene(2, 2);
	sq.update(2);
	sq.delay(10);

	sq.printText(4, 16);    // What could Khelben want?
	sq.delay(25);

	sq.loadScene(3, 2);
	sq.delay(54);
	sq.animCommand(13);
	_screen->copyRegion(104, 16, 96, 8, 120, 100, 0, 2, Screen::CR_NO_P_CHECK);
	sq.fadeText();

	sq.printText(5, 15);    // Welcome, please come in
	sq.animCommand(10);
	sq.animCommand(10);
	sq.animCommand(9);
	sq.animCommand(9);
	sq.fadeText();

	sq.printText(6, 15);    // Khelben awaits you in his study
	for (int i = 0; i < 3; i++)
		sq.animCommand(10);
	sq.animCommand(9);
	sq.animCommand(14);
	sq.loadScene(5, 2);

	sq.waitForSongNotifier(5);

	sq.fadeText();
	_screen->clearCurPage();
	_screen->updateScreen();

	for (int i = 0; i < 6; i++)
		sq.animCommand(15);

	if (_configRenderMode == Common::kRenderEGA && !skipFlag() && !shouldQuit()) {
		_screen->loadPalette("INTRO.EGA", _screen->getPalette(0));
		_screen->setScreenPalette(_screen->getPalette(0));
	}

	sq.loadScene(6, 2);
	sq.loadScene(7, 2);
	_screen->clearCurPage();
	sq.update(2);

	sq.animCommand(16);
	sq.printText(7, 15);    // Thank you for coming so quickly
	sq.animCommand(16);
	sq.animCommand(17);
	for (int i = 0; i < 3; i++)
		sq.animCommand(16);
	sq.fadeText();
	sq.animCommand(16);

	sq.loadScene(8, 2);
	sq.update(2);
	sq.animCommand(32);
	sq.printText(8, 15);    // I am troubled my friend
	sq.animCommand(33);
	sq.animCommand(33);
	for (int i = 0; i < 4; i++)
		sq.animCommand(32);
	sq.fadeText();

	sq.printText(9, 15);    // Ancient evil stirs in the Temple Darkmoon
	sq.animCommand(33);
	sq.animCommand(43);
	sq.animCommand(33);
	for (int i = 0; i < 3; i++)
		sq.animCommand(32);
	sq.fadeText();

	sq.printText(10, 15);   // I fear for the safety of our city
	for (int i = 0; i < 4; i++)
		sq.animCommand(33);
	sq.animCommand(32);
	sq.animCommand(32);

	sq.loadScene(9, 2);
	sq.fadeText();

	sq.waitForSongNotifier(6);

	sq.update(2);
	sq.animCommand(34);

	sq.printText(11, 15);   // I need your help
	for (int i = 0; i < 3; i++)
		sq.animCommand(34);
	sq.animCommand(35);
	for (int i = 0; i < 4; i++)
		sq.animCommand(34);
	sq.fadeText();

	sq.loadScene(12, 2);
	sq.update(2);
	sq.loadScene(6, 2);
	sq.animCommand(18);

	sq.printText(12, 15);   // Three nights ago I sent forth a scout
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(22);
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.fadeText();

	sq.printText(13, 15);   // She has not yet returned
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(23);
	sq.animCommand(24);
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(17);
	sq.animCommand(18);
	sq.fadeText();

	sq.printText(14, 15);   // I fear for her safety
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.animCommand(25);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();
	sq.animCommand(18);
	sq.animCommand(18);

	sq.printText(15, 15);   // Take this coin
	sq.animCommand(28);
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();

	sq.loadScene(10, 2);
	_screen->clearCurPage();
	_screen->updateScreen();

	sq.animCommand(37, 18);
	sq.animCommand(36, 36);

	sq.loadScene(12, 2);
	_screen->clearCurPage();
	sq.update(2);

	sq.loadScene(11, 2);
	sq.printText(16, 15);   // I will use it to contact you
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();

	sq.printText(17, 15);   // You must act quickly
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();
	sq.animCommand(18);

	sq.printText(18, 15);   // I will teleport you near Darkmoon
	sq.animCommand(20);
	sq.animCommand(27);
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(18);
	sq.animCommand(18);
	sq.fadeText();
	sq.animCommand(18);

	sq.printText(19, 15);   // May luck be with you my friend
	sq.animCommand(19);
	sq.animCommand(19);
	sq.animCommand(20);
	sq.animCommand(18);
	sq.fadeText();
	sq.animCommand(29);

	sq.waitForSongNotifier(7);

	sq.animCommand(30);
	sq.animCommand(31);

	sq.waitForSongNotifier(8, true);

	if (skipFlag() || shouldQuit()) {
		snd_fadeOut();
	} else {
		_screen->setScreenDim(17);
		_screen->clearCurDim();
		snd_playSoundEffect(14);

		if (_configRenderMode != Common::kRenderEGA)
			sq.fadePalette(10, 1);
		_screen->setClearScreenDim(18);
		sq.delay(6);
		if (_configRenderMode != Common::kRenderEGA)
			sq.fadePalette(9, 1);
		_screen->clearCurPage();
	}
	sq.fadePalette(9, 10);
}

void DarkMoonEngine::seq_playFinale() {
	DarkmoonSequenceHelper::Config config(DarkmoonSequenceHelper::kFinale, _finaleStrings, _cpsFilesFinale, _configRenderMode == Common::kRenderEGA ? _palFilesFinaleEGA : _palFilesFinaleVGA, _shapesFinale, _animFinale, true);
	DarkmoonSequenceHelper sq(_system, this, _screen, &config);

	_screen->setCurPage(0);
	_screen->setFont(Screen::FID_8_FNT);

	_sound->loadSoundFile("FINALE1");
	snd_stopSound();
	sq.delay(3);

	_screen->clearCurPage();
	_screen->updateScreen();

	sq.loadScene(0, 2);
	sq.delay(18);

	if (!skipFlag() && !shouldQuit())
		snd_playSong(1);
	sq.update(2);

	sq.loadScene(1, 2);

	sq.animCommand(0);
	sq.animCommand(0);
	for (int i = 0; i < 3; i++)
		sq.animCommand(2);
	sq.animCommand(1);
	sq.animCommand(2);
	sq.animCommand(2);

	sq.printText(0, 10);            // Finally, Dran has been defeated
	for (int i = 0; i < 7; i++)
		sq.animCommand(2);
	sq.fadeText();
	sq.animCommand(2);

	sq.waitForSongNotifier(1);

	sq.printText(1, 10);            // Suddenly, your friend Khelben appears
	sq.animCommand(4);
	for (int i = 0; i < 3; i++)
		sq.animCommand(2);
	sq.fadeText();

	sq.printText(2, 15);            // Greetings, my victorious friends
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();
	sq.animCommand(6);

	sq.printText(3, 15);            // You have defeated Dran
	for (int i = 0; i < 5; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(4, 15);            // I did not know Dran was a dragon
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(5, 15);            // He must have been over 300 years old
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(6, 15);            // His power is gone
	for (int i = 0; i < 3; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(7, 15);            // But Darkmoon is still a source of great evil
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.printText(8, 15);            // And many of his minions remain
	for (int i = 0; i < 4; i++)
		sq.animCommand(5);
	sq.animCommand(2);
	sq.animCommand(2);
	sq.fadeText();

	sq.loadScene(2, 2);
	sq.update(2);
	sq.loadScene(3, 2);
	_screen->copyRegion(8, 8, 0, 0, 304, 128, 0, 2, Screen::CR_NO_P_CHECK);

	sq.printText(9, 15);            // Now we must leave this place
	sq.animCommand(7);
	sq.animCommand(8);
	sq.animCommand(7);
	sq.animCommand(7, 36);
	sq.fadeText();

	sq.printText(10, 15);           // So my forces can destroy it..
	for (int i = 0; i < 3; i++)
		sq.animCommand(7);
	sq.animCommand(8);
	sq.animCommand(7);
	sq.animCommand(7, 36);
	sq.animCommand(8, 18);
	sq.fadeText();

	sq.printText(11, 15);           // Follow me
	sq.animCommand(7, 18);
	sq.animCommand(9, 18);
	sq.animCommand(8, 18);
	sq.fadeText();

	sq.loadScene(7, 2);

	if (_configRenderMode != Common::kRenderEGA)
		sq.copyPalette(3, 0);

	sq.loadScene(4, 2);

	sq.waitForSongNotifier(2);

	_screen->clearCurPage();
	sq.update(2);

	sq.loadScene(8, 2);
	sq.loadScene(6, 6);
	sq.delay(10);

	sq.printText(12, 10);           // Powerful mages stand ready for the final assault...
	sq.delay(90);
	sq.fadeText();

	sq.waitForSongNotifier(3);

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(7);
	sq.delay(8);

	sq.animCommand(10);
	sq.animCommand(13);
	sq.initDelayedPaletteFade(4, 1);

	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(14);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.initDelayedPaletteFade(2, 1);

	sq.animCommand(15);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(15);
	sq.animCommand(15);
	sq.animCommand(11);

	sq.printText(13, 10);           // The temple's evil is very strong
	sq.delay(72);
	sq.fadeText();

	sq.printText(14, 10);           // It must not be allowed...
	sq.delay(72);
	sq.fadeText();

	sq.waitForSongNotifier(4);

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(7);
	sq.delay(8);

	sq.animCommand(10);
	sq.initDelayedPaletteFade(5, 1);
	sq.animCommand(13);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(13);
	sq.animCommand(14);
	sq.animCommand(14);
	sq.animCommand(13);
	sq.animCommand(12);
	for (int i = 0; i < 4; i++)
		sq.animCommand(16);
	sq.animCommand(17);
	sq.animCommand(18);

	sq.printText(15, 10);           // The temple ceases to exist
	sq.initDelayedPaletteFade(6, 1);
	sq.delay(36);

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11);

	sq.delay(54);
	sq.fadeText();
	sq.loadScene(12, 2);

	sq.waitForSongNotifier(5);

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(6);

	if (!skipFlag() && !shouldQuit()) {
		if (_configRenderMode != Common::kRenderEGA)
			sq.setPaletteWithoutTextColor(0);
		_screen->crossFadeRegion(0, 0, 8, 8, 304, 128, 2, 0);
	}
	sq.delay(18);

	sq.printText(16, 15);           // My friends, our work is done
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(19, 36);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12);
	sq.fadeText();

	sq.printText(17, 15);           // Thank you
	sq.animCommand(19);
	sq.animCommand(20, 36);
	sq.fadeText();

	sq.printText(18, 15);           // You have earned my deepest respect
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11);
	sq.animCommand(20);
	sq.animCommand(19);
	sq.animCommand(19);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11);
	sq.delay(36);
	sq.fadeText();

	sq.printText(19, 15);           // We will remember you always
	sq.animCommand(19);
	sq.animCommand(19, 18);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11);
	sq.animCommand(20, 18);
	sq.fadeText();

	sq.delay(28);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12);
	sq.delay(3);

	sq.loadScene(5, 2);
	if (skipFlag() || shouldQuit()) {
		_screen->copyRegion(0, 0, 8, 8, 304, 128, 2, 0, Screen::CR_NO_P_CHECK);
	} else {
		snd_playSoundEffect(6);
		if (_configRenderMode != Common::kRenderEGA)
			sq.setPaletteWithoutTextColor(0);
		_screen->crossFadeRegion(0, 0, 8, 8, 304, 128, 2, 0);
	}

	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12);
	sq.delay(5);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11);
	sq.delay(11);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12);
	sq.delay(7);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(11);
	sq.delay(12);
	if (!skipFlag() && !shouldQuit())
		snd_playSoundEffect(12);

	removeInputTop();
	resetSkipFlag(true);

	sq.loadScene(10, 2);
	sq.loadScene(9, 2);
	snd_stopSound();
	sq.delay(3);

	_sound->loadSoundFile("FINALE2");

	sq.delay(18);
	if (!skipFlag() && !shouldQuit())
		snd_playSong(1);

	seq_playCredits(&sq, _creditsData, 18, 2, 6, 2);

	sq.delay(90);

	resetSkipFlag(true);

	if (_configRenderMode != Common::kRenderEGA) {
		sq.setPalette(11);
		sq.fadePalette(9, 10);
	}

	_screen->clearCurPage();
	sq.loadScene(11, 2);

	static const uint8 finPortraitPos[] = { 0x50, 0x50, 0xD0, 0x50, 0x50, 0x90, 0xD0, 0x90, 0x90, 0x50, 0x90, 0x90 };

	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 1))
			continue;
		if (i > 3)
			_screen->drawShape(2, sq._shapes[6 + i], finPortraitPos[i << 1] - 16, finPortraitPos[(i << 1) + 1] - 16, 0);
		_screen->drawShape(2, _characters[i].faceShape, finPortraitPos[i << 1], finPortraitPos[(i << 1) + 1], 0);
	}

	_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0, Screen::CR_NO_P_CHECK);

	sq.setPalette(9);
	sq.fadePalette(0, 18);

	while (!skipFlag() && !shouldQuit())
		delay(_tickLength);

	snd_stopSound();
	sq.fadePalette(9, 10);
}

void DarkMoonEngine::seq_playCredits(DarkmoonSequenceHelper *sq, const uint8 *data, int sd, int backupPage, int tempPage, int speed) {
	if (!data)
		return;

	_screen->setScreenDim(sd);
	const ScreenDim *dm = _screen->_curDim;

	_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, 0, backupPage, Screen::CR_NO_P_CHECK);

	struct CreditsDataItem {
		int16 x;
		int16 y;
		const void *data;
		char *str;
		uint8 crlf;
		uint8 size;
		uint8 dataType;
	} items[36];
	memset(items, 0, sizeof(items));

	const char *pos = (const char *)data;
	uint32 end = _system->getMillis();
	uint32 cur = 0;
	int i = 0;

	do {
		for (bool loop = true; loop;) {
			sq->processDelayedPaletteFade();
			cur = _system->getMillis();
			if (end <= cur)
				break;
			delay(MIN<uint32>(_tickLength, end - cur));
		}

		end = _system->getMillis() + speed * _tickLength;

		for (; i < 35 && *pos; i++) {
			int16 nextY = i ? items[i].y + items[i].size + (items[i].size >> 2) : dm->h;

			const char *posOld = pos;
			pos = strchr(pos, 0x0d);
			if (!pos)
				pos = strchr(posOld, 0x00);

			items[i + 1].crlf = *pos++;

			if (*posOld == 2) {
				const uint8 *shp = sq->_shapes[(*++posOld) - 1];
				items[i + 1].data = shp;
				items[i + 1].size = shp[1];
				items[i + 1].x = (dm->w - shp[2]) << 2 ;
				items[i + 1].dataType = 1;
				delete[] items[i + 1].str;
				items[i + 1].str = 0;

			} else {
				if (*posOld == 1) {
					posOld++;
					items[i + 1].size = 6;
				} else {
					items[i + 1].size = _screen->getFontWidth();
				}

				items[i + 1].dataType = 0;

				int l = pos - posOld;
				if (items[i + 1].crlf != 0x0d)
					l++;

				delete[] items[i + 1].str;
				items[i + 1].str = new char[l];
				memcpy(items[i + 1].str, posOld, l);
				items[i + 1].str[l - 1] = 0;
				items[i + 1].data = 0;
				items[i + 1].x = (((dm->w << 3) - (strlen(items[i + 1].str) * items[i + 1].size)) >> 1) + 1;
			}

			items[i + 1].y = nextY;
		}

		_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, backupPage, tempPage, Screen::CR_NO_P_CHECK);

		for (int h = 0; h < i; h++) {
			if (items[h + 1].y < dm->h) {
				if (items[h + 1].dataType == 1) {
					_screen->drawShape(tempPage, (const uint8 *)items[h + 1].data, items[h + 1].x, items[h + 1].y, sd);
				} else {
					_screen->setCurPage(tempPage);

					if (items[h + 1].size == 6)
						_screen->setFont(Screen::FID_6_FNT);

					_screen->printText(items[h + 1].str, (dm->sx << 3) + items[h + 1].x - 1, dm->sy + items[h + 1].y + 1, 12, 0);
					_screen->printText(items[h + 1].str, (dm->sx << 3) + items[h + 1].x, dm->sy + items[h + 1].y, 240, 0);

					if (items[h + 1].size == 6)
						_screen->setFont(Screen::FID_8_FNT);

					_screen->setCurPage(0);
				}
			}

			items[h + 1].y -= 2;
		}

		_screen->copyRegion(dm->sx << 3, dm->sy, dm->sx << 3, dm->sy, dm->w << 3, dm->h, tempPage, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		if (-items[1].size > items[1].y) {
			delete[] items[1].str;
			--i;
			for (int t = 1; t <= i; t++)
				memcpy(&items[t], &items[t + 1], sizeof(CreditsDataItem));
			items[i + 1].str = 0;
		}

		if (i < 35 && ((items[i].y + items[i].size) < (dm->sy + dm->h))) {
			resetSkipFlag(true);
			break;
		}

		sq->processDelayedPaletteFade();
	} while (!skipFlag() && i && !shouldQuit());

	for (i = 0; i < 35; i++)
		delete[] items[i].str;
}

DarkmoonSequenceHelper::DarkmoonSequenceHelper(OSystem *system, DarkMoonEngine *vm, Screen_EoB *screen, const Config *config) :
	_system(system), _vm(vm), _screen(screen), _config(config) {

	for (int i = 0; _config->palFiles[i]; i++) {
		if (i < 4)
			_palettes[i] = &_screen->getPalette(i);
		else
			_palettes[i] = new Palette(256);
		_screen->loadPalette(_config->palFiles[i], *_palettes[i]);
	}

	_palettes[9] = new Palette(256);
	_palettes[9]->fill(0, 256, 0);
	_palettes[10] = new Palette(256);
	_palettes[10]->fill(0, 256, 63);
	_palettes[11] = new Palette(256);
	_palettes[11]->fill(0, 256, 0);

	_shapes = new const uint8*[30];
	memset(_shapes, 0, 30 * sizeof(uint8 *));

	_fadePalTimer = 0;
	_fadePalRate = 0;

	_screen->setScreenPalette(*_palettes[0]);
	_screen->setFont(Screen::FID_8_FNT);
	_screen->hideMouse();

	_vm->delay(150);
	_vm->_eventList.clear();
	_vm->_allowSkip = true;
}

DarkmoonSequenceHelper::~DarkmoonSequenceHelper() {
	for (int i = 4; _config->palFiles[i]; i++)
		delete _palettes[i];
	delete _palettes[9];
	delete _palettes[10];
	delete _palettes[11];

	for (int i = 0; i < 30; i++)
		delete[] _shapes[i];
	delete[] _shapes;

	_screen->clearCurPage();
	_screen->showMouse();
	_screen->updateScreen();

	_system->delayMillis(150);
	_vm->resetSkipFlag(true);
	_vm->_allowSkip = false;
}

void DarkmoonSequenceHelper::loadScene(int index, int pageNum) {
	char file[13];
	strcpy(file, _config->cpsFiles[index]);

	Common::SeekableReadStream *s = _vm->resource()->createReadStream(file);
	uint32 chunkID = 0;
	if (s) {
		chunkID = s->readUint32LE();
		s->seek(0);
	}

	if (s && chunkID == MKTAG('F', 'O', 'R', 'M')) {
		// The original code also handles files with FORM chunks and ILBM and PBM sub chunks. This will probably be necessary for Amiga versions.
		// The DOS versions do not need this, but still have the code for it. We error out for now.
		error("DarkmoonSequenceHelper::loadScene(): CPS file loading failure in scene %d - unhandled FORM chunk encountered", index);
	} else if (s && file[0] != 'X') {
		delete s;
		_screen->loadBitmap(_config->cpsFiles[index], pageNum | 1, pageNum | 1, _palettes[0]);
	} else {
		if (!s) {
			file[0] = 'X';
			s = _vm->resource()->createReadStream(file);
		}

		if (!s)
			error("DarkmoonSequenceHelper::loadScene(): CPS file loading failure in scene %d", index);

		if (_config->mode == kFinale)
			s->read(_palettes[0]->getData(), 768);
		else
			s->seek(768);
		_screen->loadFileDataToPage(s, 3, 64000);
		delete s;
	}

	int cp = _screen->setCurPage(pageNum);

	if (_config->shapeDefs[index]) {
		for (const DarkMoonShapeDef *df = _config->shapeDefs[index]; df->w; df++) {
			uint16 shapeIndex = (df->index < 0) ? df->index * -1 : df->index;
			if (_shapes[shapeIndex])
				delete[] _shapes[shapeIndex];
			_shapes[shapeIndex] = _screen->encodeShape(df->x, df->y, df->w, df->h, (df->index >> 8) != 0);
		}
	}

	_screen->setCurPage(cp);

	if (_vm->_configRenderMode == Common::kRenderEGA)
		setPalette(0);

	_screen->convertPage(pageNum | 1, pageNum, 0);

	if ((pageNum == 0 || pageNum == 1) && !_vm->skipFlag() && !_vm->shouldQuit())
		_screen->updateScreen();
}

void DarkmoonSequenceHelper::animCommand(int index, int del) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;

	uint32 end = 0;

	for (const DarkMoonAnimCommand *s = _config->animData[index]; s->command != 0xff && !_vm->skipFlag() && !_vm->shouldQuit(); s++) {
		int palIndex = _config->mode == kFinale ? (s->pal + 1) : s->pal;
		int x = s->x1;
		int y = s->y1;
		int x2 = 0;
		uint16 shapeW = 0;
		uint16 shapeH = 0;

		switch (s->command) {
		case 0:
			// flash palette
			if (_vm->_configRenderMode != Common::kRenderEGA && s->pal)
				setPaletteWithoutTextColor(palIndex);
			delay(s->delay);
			if (_vm->_configRenderMode != Common::kRenderEGA && _config->mode == kIntro && s->pal)
				setPaletteWithoutTextColor(0);
			break;

		case 1:
			// draw shape, then restore background
			shapeW = _shapes[s->obj][2];
			shapeH = _shapes[s->obj][3];

			if (_config->mode == kFinale) {
				_screen->setScreenDim(18);
				x -= (_screen->_curDim->sx << 3);
				y -= _screen->_curDim->sy;
				if (x < 0)
					shapeW -= ((-x >> 3) + 1);
				else
					x2 = x;
			}

			_screen->drawShape(0, _shapes[s->obj], x, y, _config->mode == kIntro ? 0 : 18);

			if (_vm->_configRenderMode != Common::kRenderEGA && s->pal)
				setPaletteWithoutTextColor(palIndex);
			else
				_screen->updateScreen();

			delay(s->delay);

			if (_config->mode == kIntro) {
				if (_vm->_configRenderMode != Common::kRenderEGA && s->pal)
					setPaletteWithoutTextColor(0);
				_screen->copyRegion(x - 8, y - 8, x, y, (shapeW + 1) << 3, shapeH, 2, 0, Screen::CR_NO_P_CHECK);
			} else {
				_screen->copyRegion(x2, y, x2 + (_screen->_curDim->sx << 3), y + _screen->_curDim->sy, (shapeW + 1) << 3, shapeH, 2, 0, Screen::CR_NO_P_CHECK);
			}

			_screen->updateScreen();
			break;

		case 2:
			// draw shape
			_screen->drawShape(_screen->_curPage, _shapes[s->obj], x, y, 0);

			if (_vm->_configRenderMode != Common::kRenderEGA && s->pal)
				setPaletteWithoutTextColor(palIndex);
			else if (!_screen->_curPage)
				_screen->updateScreen();

			delay(s->delay);

			if (_vm->_configRenderMode != Common::kRenderEGA && _config->mode == kIntro && s->pal)
				setPaletteWithoutTextColor(0);
			break;

		case 3:
		case 4:
			// fade shape in or out or restore background
			if (_config->mode == kFinale)
				break;

			if (_vm->_configRenderMode == Common::kRenderEGA) {
				if (palIndex)
					_screen->drawShape(0, _shapes[s->obj], s->x1, y, 0);
				else
					_screen->copyRegion(s->x1 - 8, s->y1 - 8, s->x1, s->y1, (_shapes[s->obj][2] + 1) << 3, _shapes[s->obj][3], 2, 0, Screen::CR_NO_P_CHECK);
				_screen->updateScreen();
				delay(s->delay /** 7*/);
			} else {
				_screen->setShapeFadeMode(0, true);
				_screen->setShapeFadeMode(1, true);

				end = _system->getMillis() + s->delay * _vm->tickLength();

				if (palIndex) {
					_screen->setFadeTableIndex(palIndex - 1);

					_screen->copyRegion(s->x1 - 8, s->y1 - 8, 0, 0, (_shapes[s->obj][2] + 1) << 3, _shapes[s->obj][3], 2, 4, Screen::CR_NO_P_CHECK);
					_screen->drawShape(4, _shapes[s->obj], s->x1 & 7, 0, 0);
					_screen->copyRegion(0, 0, s->x1, s->y1, (_shapes[s->obj][2] + 1) << 3, _shapes[s->obj][3], 4, 0, Screen::CR_NO_P_CHECK);
				} else {
					_screen->copyRegion(s->x1 - 8, s->y1 - 8, s->x1, s->y1, (_shapes[s->obj][2] + 1) << 3, _shapes[s->obj][3], 2, 0, Screen::CR_NO_P_CHECK);
				}
				_screen->updateScreen();

				_vm->delayUntil(end);
				_screen->setShapeFadeMode(0, false);
				_screen->setShapeFadeMode(1, false);
			}
			break;

		case 5:
			// copy region
			if (_config->mode == kFinale && s->pal)
				setPaletteWithoutTextColor(palIndex);

			_screen->copyRegion(s->x2 << 3, s->y2, s->x1, s->y1, s->w << 3, s->h, (s->obj && _config->mode == kFinale) ? 6 : 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			delay(s->delay);
			break;

		case 6:
			// play sound effect
			if (s->obj != 0xff)
				_vm->snd_playSoundEffect(s->obj);
			break;

		case 7:
			// restore background (only used in EGA mode)
			delay(s->delay);
			_screen->copyRegion(s->x1 - 8, s->y1 - 8, s->x1, s->y1, (_shapes[s->obj][2] + 1) << 3, _shapes[s->obj][3], 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
			break;

		default:
			error("DarkmoonSequenceHelper::animCommand(): Unknown animation opcode encountered.");
			break;
		}
	}

	if (del > 0)
		delay(del);
}

void DarkmoonSequenceHelper::printText(int index, int color) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;

	_screen->setClearScreenDim(17);
	uint8 col1 = 15;

	if (_vm->_configRenderMode != Common::kRenderEGA) {
		_palettes[0]->copy(*_palettes[0], color, 1, 255);
		setPalette(0);
		col1 = 255;
	}

	char *temp = new char[strlen(_config->strings[index]) + 1];
	char *str = temp;
	strcpy(str, _config->strings[index]);

	const ScreenDim *dm = _screen->_curDim;

	for (int yOffs = 0; *str; yOffs += 9) {
		char *cr = strchr(str, 13);

		if (cr)
			*cr = 0;

		uint32 len = strlen(str);
		_screen->printText(str, (dm->sx + ((dm->w - len) >> 1)) << 3, dm->sy + yOffs, col1, dm->unkA);

		if (cr) {
			*cr = 13;
			str = cr + 1;
		} else {
			str += len;
		}
	}

	delete[] temp;
	_screen->updateScreen();
}

void DarkmoonSequenceHelper::fadeText() {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;
	if (_vm->_configRenderMode != Common::kRenderEGA)
		_screen->fadeTextColor(_palettes[0], 255, 8);
	_screen->clearCurDim();
}

void DarkmoonSequenceHelper::update(int srcPage) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;

	_screen->copyRegion(0, 0, 8, 8, 304, 128, srcPage, 0, Screen::CR_NO_P_CHECK);

	if (_vm->_configRenderMode != Common::kRenderEGA)
		setPaletteWithoutTextColor(0);
}

void DarkmoonSequenceHelper::setPaletteWithoutTextColor(int index) {
	if (_vm->_configRenderMode == Common::kRenderEGA || _vm->skipFlag() || _vm->shouldQuit())
		return;

	if (!memcmp(_palettes[11]->getData(), _palettes[index]->getData(), 765))
		return;

	_palettes[11]->copy(*_palettes[index], 0, 255);
	_palettes[11]->copy(*_palettes[0], 255, 1, 255);
	setPalette(11);

	_screen->updateScreen();
	_system->delayMillis(10);
}

void DarkmoonSequenceHelper::setPalette(int index) {
	_screen->setScreenPalette(*_palettes[index]);
}

void DarkmoonSequenceHelper::fadePalette(int index, int del) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;
	if (_vm->_configRenderMode == Common::kRenderEGA) {
		setPalette(index);
		_screen->updateScreen();
	} else {
		_screen->fadePalette(*_palettes[index], del * _vm->tickLength());
	}
}

void DarkmoonSequenceHelper::copyPalette(int srcIndex, int destIndex) {
	_palettes[destIndex]->copy(*_palettes[srcIndex]);
}

void DarkmoonSequenceHelper::initDelayedPaletteFade(int palIndex, int rate) {
	_palettes[11]->copy(*_palettes[0]);

	_fadePalIndex = palIndex;
	_fadePalRate = rate;
	_fadePalTimer = _system->getMillis() + 2 * _vm->_tickLength;
}

bool DarkmoonSequenceHelper::processDelayedPaletteFade() {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return true;

	if (_vm->_configRenderMode == Common::kRenderEGA || !_fadePalRate || (_system->getMillis() <= _fadePalTimer))
		return false;

	if (_screen->delayedFadePalStep(_palettes[_fadePalIndex], _palettes[0], _fadePalRate)) {
		setPaletteWithoutTextColor(0);
		_fadePalTimer = _system->getMillis() + 3 * _vm->_tickLength;
	} else {
		_fadePalRate = 0;
	}

	return false;
}

void DarkmoonSequenceHelper::delay(uint32 ticks) {
	if (_vm->skipFlag() || _vm->shouldQuit())
		return;

	uint32 end = _system->getMillis() + ticks * _vm->_tickLength;

	if (_config->palFading) {
		do {
			if (processDelayedPaletteFade())
				break;
			_vm->updateInput();
		} while (end > _system->getMillis());
		processDelayedPaletteFade();

	} else {
		_vm->delayUntil(end);
	}
}

void DarkmoonSequenceHelper::waitForSongNotifier(int index, bool introUpdateAnim) {
	int seq = 0;
	while (_vm->sound()->checkTrigger() < index && !(_vm->skipFlag() || _vm->shouldQuit())) {
		if (introUpdateAnim) {
			animCommand(30 | seq);
			seq ^= 1;
		}

		if (_config->palFading)
			processDelayedPaletteFade();

		_vm->updateInput();
	}
}

void DarkMoonEngine::seq_nightmare() {
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);
	_screen->copyRegion(0, 0, 0, 120, 176, 24, 12, 2, Screen::CR_NO_P_CHECK);

	initDialogueSequence();
	gui_drawDialogueBox();

	_txt->printDialogueText(99, 0);
	snd_playSoundEffect(54);

	static const uint8 seqX[] = { 0, 20, 0, 20 };
	static const uint8 seqY[] = { 0, 0, 96, 96 };
	static const uint8 seqDelay[] = { 12, 7, 7, 12 };

	for (const int8 *i = _dreamSteps; *i != -1; ++i) {
		drawSequenceBitmap("DREAM", 0, seqX[*i], seqY[*i], 0);
		delay(seqDelay[*i] * _tickLength);
	}

	_txt->printDialogueText(20, _okStrings[0]);

	restoreAfterDialogueSequence();

	_screen->setFont(of);
}

void DarkMoonEngine::seq_kheldran() {
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);

	initDialogueSequence();
	gui_drawDialogueBox();

	static const char file[] = "KHELDRAN";
	_txt->printDialogueText(_kheldranStrings[0]);
	drawSequenceBitmap(file, 0, 0, 0, 0);
	_txt->printDialogueText(20, _moreStrings[0]);
	snd_playSoundEffect(56);
	drawSequenceBitmap(file, 0, 20, 0, 0);
	delay(10 * _tickLength);
	drawSequenceBitmap(file, 0, 0, 96, 0);
	delay(10 * _tickLength);
	drawSequenceBitmap(file, 0, 20, 96, 0);
	delay(7 * _tickLength);
	_txt->printDialogueText(76, _okStrings[0]);

	restoreAfterDialogueSequence();

	_screen->setFont(of);
}

void DarkMoonEngine::seq_dranDragonTransformation() {
	Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);

	initDialogueSequence();
	gui_drawDialogueBox();

	static const char file[] = "DRANX";
	drawSequenceBitmap(file, 0, 0, 0, 0);
	_txt->printDialogueText(120, _moreStrings[0]);
	snd_playSoundEffect(56);
	drawSequenceBitmap(file, 0, 20, 0, 0);
	delay(7 * _tickLength);
	drawSequenceBitmap(file, 0, 0, 96, 0);
	delay(7 * _tickLength);
	drawSequenceBitmap(file, 0, 20, 96, 0);
	delay(18 * _tickLength);

	restoreAfterDialogueSequence();

	_screen->setFont(of);
}

} // End of namespace Kyra

#endif // ENABLE_EOB
