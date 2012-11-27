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

#include "kyra/kyra_lok.h"
#include "kyra/seqplayer.h"
#include "kyra/resource.h"
#include "kyra/sprites.h"
#include "kyra/wsamovie.h"
#include "kyra/animator_lok.h"
#include "kyra/timer.h"
#include "kyra/sound.h"

#include "common/system.h"
#include "common/savefile.h"
#include "common/list.h"

namespace Kyra {

void KyraEngine_LoK::seq_demo() {
	snd_playTheme(0, 2);

	_screen->loadBitmap("START.CPS", 7, 7, &_screen->getPalette(0));
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 6, 0, Screen::CR_NO_P_CHECK);
	_screen->updateScreen();
	_screen->fadeFromBlack();
	delay(60 * _tickLength);
	_screen->fadeToBlack();

	_screen->clearPage(0);
	_screen->loadBitmap("TOP.CPS", 7, 7, 0);
	_screen->loadBitmap("BOTTOM.CPS", 5, 5, &_screen->getPalette(0));
	_screen->copyRegion(0, 91, 0, 8, 320, 103, 6, 0);
	_screen->copyRegion(0, 0, 0, 111, 320, 64, 6, 0);
	_screen->updateScreen();
	_screen->fadeFromBlack();

	_seq->playSequence(_seq_WestwoodLogo, true);
	delay(60 * _tickLength);
	_seq->playSequence(_seq_KyrandiaLogo, true);

	_screen->fadeToBlack();
	_screen->clearPage(2);
	_screen->clearPage(0);

	_seq->playSequence(_seq_Demo1, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo2, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo3, true);

	_screen->clearPage(0);
	_seq->playSequence(_seq_Demo4, true);

	_screen->clearPage(0);
	_screen->loadBitmap("FINAL.CPS", 7, 7, &_screen->getPalette(0));
	_screen->_curPage = 0;
	_screen->copyRegion(0, 0, 0, 0, 320, 200, 6, 0);
	_screen->updateScreen();
	_screen->fadeFromBlack();
	delay(60 * _tickLength);
	_screen->fadeToBlack();
	_sound->haltTrack();
}

void KyraEngine_LoK::seq_intro() {
	if (_flags.isTalkie)
		_res->loadPakFile("INTRO.VRM");

	static const IntroProc introProcTable[] = {
		&KyraEngine_LoK::seq_introPublisherLogos,
		&KyraEngine_LoK::seq_introLogos,
		&KyraEngine_LoK::seq_introStory,
		&KyraEngine_LoK::seq_introMalcolmTree,
		&KyraEngine_LoK::seq_introKallakWriting,
		&KyraEngine_LoK::seq_introKallakMalcolm
	};

	Common::InSaveFile *in;
	if ((in = _saveFileMan->openForLoading(getSavegameFilename(0)))) {
		delete in;
		_skipIntroFlag = true;
	} else {
		_skipIntroFlag = !_flags.isDemo;
	}

	_seq->setCopyViewOffs(true);
	_screen->setFont(_flags.lang == Common::JA_JPN ? Screen::FID_SJIS_FNT : Screen::FID_8_FNT);
	if (_flags.platform != Common::kPlatformFMTowns && _flags.platform != Common::kPlatformPC98 && _flags.platform != Common::kPlatformAmiga)
		snd_playTheme(0, 2);
	_text->setTalkCoords(144);

	for (int i = 0; i < ARRAYSIZE(introProcTable) && !seq_skipSequence(); ++i) {
		if ((this->*introProcTable[i])() && !shouldQuit()) {
			resetSkipFlag();
			_screen->fadeToBlack();
			_screen->clearPage(0);
		}
	}

	_text->setTalkCoords(136);
	delay(30 * _tickLength);
	_seq->setCopyViewOffs(false);
	_sound->haltTrack();
	_sound->voiceStop();

	if (_flags.isTalkie)
		_res->unloadPakFile("INTRO.VRM");
}

bool KyraEngine_LoK::seq_introPublisherLogos() {
	if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) {
		_screen->loadBitmap("LOGO.CPS", 3, 3, &_screen->getPalette(0));
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();
		_screen->fadeFromBlack();
		delay(90 * _tickLength);
		if (!_abortIntroFlag) {
			_screen->fadeToBlack();
			snd_playWanderScoreViaMap(_flags.platform == Common::kPlatformFMTowns ? 57 : 2, 0);
		}
	} else if (_flags.platform == Common::kPlatformMacintosh && _res->exists("MP_GOLD.CPS")) {
		_screen->loadPalette("MP_GOLD.COL", _screen->getPalette(0));
		_screen->loadBitmap("MP_GOLD.CPS", 3, 3, 0);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 2, 0);
		_screen->updateScreen();
		_screen->fadeFromBlack();
		delay(120 * _tickLength);
		if (!_abortIntroFlag)
			_screen->fadeToBlack();
	}

	return _abortIntroFlag;
}

bool KyraEngine_LoK::seq_introLogos() {
	_screen->clearPage(0);

	if (_flags.platform == Common::kPlatformAmiga) {
		_screen->loadPaletteTable("INTRO.PAL", 0);
		_screen->loadBitmap("BOTTOM.CPS", 3, 5, 0);
		_screen->loadBitmap("TOP.CPS", 3, 3, 0);
		_screen->copyRegion(0, 0, 0, 111, 320, 64, 2, 0);
		_screen->copyRegion(0, 91, 0, 8, 320, 109, 2, 0);
		_screen->copyRegion(0, 0, 0, 0, 320, 190, 0, 2);
	} else {
		_screen->loadBitmap("TOP.CPS", 7, 7, 0);
		_screen->loadBitmap("BOTTOM.CPS", 5, 5, &_screen->getPalette(0));
		_screen->copyRegion(0, 91, 0, 8, 320, 103, 6, 0);
		_screen->copyRegion(0, 0, 0, 111, 320, 64, 6, 0);
	}

	_screen->_curPage = 0;
	_screen->updateScreen();
	_screen->fadeFromBlack();

	if (_seq->playSequence(_seq_WestwoodLogo, skipFlag()) || shouldQuit())
		return true;

	delay(60 * _tickLength);

	if (_flags.platform == Common::kPlatformAmiga) {
		_screen->copyPalette(0, 1);
		_screen->setScreenPalette(_screen->getPalette(0));
	}

	Screen::FontId of = _screen->setFont(Screen::FID_8_FNT);

	if (_seq->playSequence(_seq_KyrandiaLogo, skipFlag()) || shouldQuit())
		return true;

	_screen->setFont(of);
	_screen->fillRect(0, 179, 319, 199, 0);

	if (shouldQuit())
		return false;

	if (_flags.platform == Common::kPlatformAmiga) {
		_screen->copyPalette(0, 2);
		_screen->fadeToBlack();
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 4, 0);
		_screen->fadeFromBlack();
	} else {
		_screen->copyRegion(0, 91, 0, 8, 320, 104, 6, 2);
		_screen->copyRegion(0, 0, 0, 112, 320, 64, 6, 2);

		uint32 start = _system->getMillis();
		bool doneFlag = false;
		int oldDistance = 0;

		do {
			uint32 now = _system->getMillis();

			// The smallest y2 we ever draw the screen for is 65.
			int distance = (now - start) / _tickLength;
			if (distance > 112) {
				distance = 112;
				doneFlag = true;
			}

			if (distance > oldDistance) {
				int y1 = 8 + distance;
				int h1 = 168 - distance;
				int y2 = 176 - distance;
				int h2 = distance;

				_screen->copyRegion(0, y1, 0, 8, 320, h1, 2, 0);
				if (h2 > 0)
					_screen->copyRegion(0, 64, 0, y2, 320, h2, 4, 0);
				_screen->updateScreen();
			}

			oldDistance = distance;
			delay(10);
		} while (!doneFlag && !shouldQuit() && !_abortIntroFlag);
	}

	if (_abortIntroFlag || shouldQuit())
		return true;

	return _seq->playSequence(_seq_Forest, true);
}

bool KyraEngine_LoK::seq_introStory() {
	_screen->clearPage(3);
	_screen->clearPage(0);

	// HACK: The Italian fan translation uses an special text screen here
	// so we show it even when text is disabled
	if (!textEnabled() && speechEnabled() && _flags.lang != Common::IT_ITA)
		return false;

	if (((_flags.lang == Common::EN_ANY || _flags.lang == Common::RU_RUS) && !_flags.isTalkie && _flags.platform == Common::kPlatformPC) || _flags.platform == Common::kPlatformAmiga)
		_screen->loadBitmap("TEXT.CPS", 3, 3, &_screen->getPalette(0));
	else if (_flags.lang == Common::EN_ANY || _flags.lang == Common::JA_JPN)
		_screen->loadBitmap("TEXT_ENG.CPS", 3, 3, &_screen->getPalette(0));
	else if (_flags.lang == Common::DE_DEU)
		_screen->loadBitmap("TEXT_GER.CPS", 3, 3, &_screen->getPalette(0));
	else if (_flags.lang == Common::FR_FRA)
		_screen->loadBitmap("TEXT_FRE.CPS", 3, 3, &_screen->getPalette(0));
	else if (_flags.lang == Common::ES_ESP)
		_screen->loadBitmap("TEXT_SPA.CPS", 3, 3, &_screen->getPalette(0));
	else if (_flags.lang == Common::IT_ITA && !_flags.isTalkie)
		_screen->loadBitmap("TEXT_ITA.CPS", 3, 3, &_screen->getPalette(0));
	else if (_flags.lang == Common::IT_ITA && _flags.isTalkie)
		_screen->loadBitmap("TEXT_ENG.CPS", 3, 3, &_screen->getPalette(0));
	else
		warning("no story graphics file found");

	if (_flags.platform == Common::kPlatformAmiga)
		_screen->setScreenPalette(_screen->getPalette(4));
	else
		_screen->setScreenPalette(_screen->getPalette(0));
	_screen->copyPage(3, 0);

	if (_flags.lang == Common::JA_JPN) {
		const int y1 = 175;
		int x1, x2, y2, col1;
		const char *s1, *s2;

		if (_flags.platform == Common::kPlatformFMTowns) {
			s1 = _seq_textsTable[18];
			s2 = _seq_textsTable[19];
			x1 = (Screen::SCREEN_W - _screen->getTextWidth(s1)) / 2;
			x2 = (Screen::SCREEN_W - _screen->getTextWidth(s2)) / 2;
			uint8 colorMap[] = { 0, 15, 12, 12 };
			_screen->setTextColor(colorMap, 0, 3);
			y2 = 184;
			col1 = 5;

		} else {
			s1 = _storyStrings[0];
			s2 = _storyStrings[1];
			x1 = x2 = 54;
			y2 = 185;
			col1 = 15;
		}

		_screen->printText(s1, x1, y1, col1, 8);
		_screen->printText(s2, x2, y2, col1, 8);
	}

	_screen->updateScreen();
	delay(360 * _tickLength);

	return _abortIntroFlag;
}

bool KyraEngine_LoK::seq_introMalcolmTree() {
	_screen->_curPage = 0;
	_screen->clearPage(3);
	return _seq->playSequence(_seq_MalcolmTree, true);
}

bool KyraEngine_LoK::seq_introKallakWriting() {
	_seq->makeHandShapes();
	_screen->setAnimBlockPtr(5060);
	_screen->_charWidth = -2;
	_screen->clearPage(3);
	const bool skipped = _seq->playSequence(_seq_KallakWriting, true);
	_seq->freeHandShapes();

	return skipped;
}

bool KyraEngine_LoK::seq_introKallakMalcolm() {
	_screen->clearPage(3);
	return _seq->playSequence(_seq_KallakMalcolm, true);
}

void KyraEngine_LoK::seq_createAmuletJewel(int jewel, int page, int noSound, int drawOnly) {
	static const uint16 specialJewelTable[] = {
		0x167, 0x162, 0x15D, 0x158, 0x153, 0xFFFF
	};
	static const uint16 specialJewelTable1[] = {
		0x14F, 0x154, 0x159, 0x15E, 0x163, 0xFFFF
	};
	static const uint16 specialJewelTable2[] = {
		0x150, 0x155, 0x15A, 0x15F, 0x164, 0xFFFF
	};
	static const uint16 specialJewelTable3[] = {
		0x151, 0x156, 0x15B, 0x160, 0x165, 0xFFFF
	};
	static const uint16 specialJewelTable4[] = {
		0x152, 0x157, 0x15C, 0x161, 0x166, 0xFFFF
	};
	if (!noSound)
		snd_playSoundEffect(0x5F);
	_screen->hideMouse();
	if (!drawOnly) {
		for (int i = 0; specialJewelTable[i] != 0xFFFF; ++i) {
			_screen->drawShape(page, _shapes[specialJewelTable[i]], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
			_screen->updateScreen();
			delayWithTicks(3);
		}

		const uint16 *opcodes = 0;
		switch (jewel - 1) {
		case 0:
			opcodes = specialJewelTable1;
			break;

		case 1:
			opcodes = specialJewelTable2;
			break;

		case 2:
			opcodes = specialJewelTable3;
			break;

		case 3:
			opcodes = specialJewelTable4;
			break;
		}

		if (opcodes) {
			for (int i = 0; opcodes[i] != 0xFFFF; ++i) {
				_screen->drawShape(page, _shapes[opcodes[i]], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
				_screen->updateScreen();
				delayWithTicks(3);
			}
		}
	}
	_screen->drawShape(page, _shapes[323 + jewel], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	_screen->showMouse();
	setGameFlag(0x55 + jewel);
}

void KyraEngine_LoK::seq_brandonHealing() {
	if (!(_deathHandler & 8))
		return;
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_healingShapeTable);
	setupShapes123(_healingShapeTable, 22, 0);
	_animator->setBrandonAnimSeqSize(3, 48);
	snd_playSoundEffect(0x53);
	for (int i = 123; i <= 144; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	for (int i = 125; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_brandonHealing2() {
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_healingShape2Table);
	setupShapes123(_healingShape2Table, 30, 0);
	resetBrandonPoisonFlags();
	_animator->setBrandonAnimSeqSize(3, 48);
	snd_playSoundEffect(0x50);
	for (int i = 123; i <= 152; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
	assert(_poisonGone);
	characterSays(2010, _poisonGone[0], 0, -2);
	characterSays(2011, _poisonGone[1], 0, -2);
}

void KyraEngine_LoK::seq_poisonDeathNow(int now) {
	if (!(_brandonStatusBit & 1))
		return;
	++_poisonDeathCounter;
	if (now)
		_poisonDeathCounter = 2;
	if (_poisonDeathCounter >= 2) {
		snd_playWanderScoreViaMap(1, 1);
		assert(_thePoison);
		characterSays(7000, _thePoison[0], 0, -2);
		characterSays(7001, _thePoison[1], 0, -2);
		seq_poisonDeathNowAnim();
		_deathHandler = 3;
	} else {
		assert(_thePoison);
		characterSays(7002, _thePoison[2], 0, -2);
		characterSays(7004, _thePoison[3], 0, -2);
	}
}

void KyraEngine_LoK::seq_poisonDeathNowAnim() {
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_posionDeathShapeTable);
	setupShapes123(_posionDeathShapeTable, 20, 0);
	_animator->setBrandonAnimSeqSize(8, 48);

	_currentCharacter->currentAnimFrame = 124;
	_animator->animRefreshNPC(0);
	delayWithTicks(30);

	_currentCharacter->currentAnimFrame = 123;
	_animator->animRefreshNPC(0);
	delayWithTicks(30);

	for (int i = 125; i <= 139; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	delayWithTicks(60);

	for (int i = 140; i <= 142; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	delayWithTicks(60);

	_animator->resetBrandonAnimSeqSize();
	freeShapes123();
	_animator->restoreAllObjectBackgrounds();
	_currentCharacter->x1 = _currentCharacter->x2 = -1;
	_currentCharacter->y1 = _currentCharacter->y2 = -1;
	_animator->preserveAllBackgrounds();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_playFluteAnimation() {
	_screen->hideMouse();
	checkAmuletAnimFlags();
	setupShapes123(_fluteAnimShapeTable, 36, 0);
	_animator->setBrandonAnimSeqSize(3, 75);
	for (int i = 123; i <= 130; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(2);
	}

	int delayTime = 0, soundType = 0;
	if (queryGameFlag(0x85)) {
		snd_playSoundEffect(0x63);
		delayTime = 9;
		soundType = 3;
	} else if (!queryGameFlag(0x86)) {
		snd_playSoundEffect(0x61);
		delayTime = 2;
		soundType = 1;
		setGameFlag(0x86);
	} else {
		snd_playSoundEffect(0x62);
		delayTime = 2;
		soundType = 2;
	}

	for (int i = 131; i <= 158; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(delayTime);
	}

	for (int i = 126; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(delayTime);
	}
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();

	if (soundType == 1) {
		assert(_fluteString);
		characterSays(1000, _fluteString[0], 0, -2);
	} else if (soundType == 2) {
		assert(_fluteString);
		characterSays(1001, _fluteString[1], 0, -2);
	}
}

void KyraEngine_LoK::seq_winterScroll1() {
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_winterScrollTable);
	assert(_winterScroll1Table);
	assert(_winterScroll2Table);
	setupShapes123(_winterScrollTable, 7, 0);
	_animator->setBrandonAnimSeqSize(5, 66);

	for (int i = 123; i <= 129; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	freeShapes123();
	snd_playSoundEffect(0x20);

	uint8 numFrames, midpoint;
	if (_flags.isTalkie) {
		numFrames = 18;
		midpoint = 136;
	} else {
		numFrames = 35;
		midpoint = 147;
	}
	setupShapes123(_winterScroll1Table, numFrames, 0);
	for (int i = 123; i < midpoint; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	if (_currentCharacter->sceneId == 41 && !queryGameFlag(0xA2)) {
		snd_playSoundEffect(0x20);
		_sprites->_anims[0].play = false;
		_animator->sprites()[0].active = 0;
		_sprites->_anims[1].play = true;
		_animator->sprites()[1].active = 1;

		if (_flags.platform != Common::kPlatformAmiga)
			setGameFlag(0xA2);
	}

	for (int i = midpoint; i < 123 + numFrames; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	if (_currentCharacter->sceneId == 117 && !queryGameFlag(0xB3)) {
		for (int i = 0; i <= 7; ++i) {
			_sprites->_anims[i].play = false;
			_animator->sprites()[i].active = 0;
		}

		if (_flags.platform == Common::kPlatformAmiga) {
			_screen->copyPalette(0, 11);
		} else {
			_screen->getPalette(0).copy(palTable2()[0], 0, 20, 228);
			_screen->fadePalette(_screen->getPalette(0), 72);
			_screen->setScreenPalette(_screen->getPalette(0));
			setGameFlag(0xB3);
		}
	} else {
		delayWithTicks(120);
	}

	freeShapes123();
	setupShapes123(_winterScroll2Table, 4, 0);

	for (int i = 123; i <= 126; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_winterScroll2() {
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_winterScrollTable);
	setupShapes123(_winterScrollTable, 7, 0);
	_animator->setBrandonAnimSeqSize(5, 66);

	for (int i = 123; i <= 128; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	delayWithTicks(120);

	for (int i = 127; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_makeBrandonInv() {
	if (_deathHandler == 8)
		return;

	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}

	_screen->hideMouse();
	checkAmuletAnimFlags();
	_brandonStatusBit |= 0x20;
	_timer->setCountdown(18, 2700);
	_brandonStatusBit |= 0x40;
	snd_playSoundEffect(0x77);
	_brandonInvFlag = 0;
	while (_brandonInvFlag <= 0x100) {
		_animator->animRefreshNPC(0);
		delayWithTicks(10);
		_brandonInvFlag += 0x10;
	}
	_brandonStatusBit &= 0xFFBF;
	_screen->showMouse();
}

void KyraEngine_LoK::seq_makeBrandonNormal() {
	_screen->hideMouse();
	_brandonStatusBit |= 0x40;
	snd_playSoundEffect(0x77);
	_brandonInvFlag = 0x100;
	while (_brandonInvFlag >= 0) {
		_animator->animRefreshNPC(0);
		delayWithTicks(10);
		_brandonInvFlag -= 0x10;
	}
	_brandonInvFlag = 0;
	_brandonStatusBit &= 0xFF9F;
	_screen->showMouse();
}

void KyraEngine_LoK::seq_makeBrandonNormal2() {
	_screen->hideMouse();
	assert(_brandonToWispTable);
	setupShapes123(_brandonToWispTable, 26, 0);
	_animator->setBrandonAnimSeqSize(5, 48);
	_brandonStatusBit &= 0xFFFD;
	snd_playSoundEffect(0x6C);
	for (int i = 138; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_animator->setBrandonAnimSeqSize(3, 48);
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);

	if (_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245)
		_screen->fadeSpecialPalette(31, 234, 13, 4);
	else if (_currentCharacter->sceneId >= 118 && _currentCharacter->sceneId <= 186)
		_screen->fadeSpecialPalette(14, 228, 15, 4);

	freeShapes123();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_makeBrandonWisp() {
	if (_deathHandler == 8)
		return;

	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	_screen->hideMouse();
	checkAmuletAnimFlags();
	assert(_brandonToWispTable);
	setupShapes123(_brandonToWispTable, 26, 0);
	_animator->setBrandonAnimSeqSize(5, 48);
	snd_playSoundEffect(0x6C);
	for (int i = 123; i <= 138; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}
	_brandonStatusBit |= 2;

	if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198)
		_timer->setCountdown(14, 18000);
	else
		_timer->setCountdown(14, 7200);

	_animator->_brandonDrawFrame = 113;
	_brandonStatusBit0x02Flag = 1;
	_currentCharacter->currentAnimFrame = 113;
	_animator->animRefreshNPC(0);
	_animator->updateAllObjectShapes();

	if (_flags.platform == Common::kPlatformAmiga) {
		if ((_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245) ||
		        (_currentCharacter->sceneId >= 118 && _currentCharacter->sceneId <= 186))
			_screen->fadePalette(_screen->getPalette(10), 0x54);
	} else {
		if (_currentCharacter->sceneId >= 229 && _currentCharacter->sceneId <= 245)
			_screen->fadeSpecialPalette(30, 234, 13, 4);
		else if (_currentCharacter->sceneId >= 118 && _currentCharacter->sceneId <= 186)
			_screen->fadeSpecialPalette(14, 228, 15, 4);
	}

	freeShapes123();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_dispelMagicAnimation() {
	if (_deathHandler == 8)
		return;
	if (_currentCharacter->sceneId == 210) {
		if (_beadStateVar == 4 || _beadStateVar == 6)
			return;
	}
	_screen->hideMouse();
	if (_currentCharacter->sceneId == 210 && _currentCharacter->sceneId < 160)
		_currentCharacter->facing = 3;
	if (_malcolmFlag == 7 && _beadStateVar == 3) {
		_beadStateVar = 6;
		_unkEndSeqVar5 = 2;
		_malcolmFlag = 10;
	}
	checkAmuletAnimFlags();
	setGameFlag(0xEE);
	assert(_magicAnimationTable);
	setupShapes123(_magicAnimationTable, 5, 0);
	_animator->setBrandonAnimSeqSize(8, 49);
	snd_playSoundEffect(0x15);
	for (int i = 123; i <= 127; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	delayWithTicks(120);

	for (int i = 127; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(10);
	}
	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_fillFlaskWithWater(int item, int type) {
	int newItem = -1;

	static const uint8 flaskTable1[] = { 0x46, 0x48, 0x4A, 0x4C };
	static const uint8 flaskTable2[] = { 0x47, 0x49, 0x4B, 0x4D };

	if (item >= 60 && item <= 77) {
		assert(_flaskFull);
		characterSays(8006, _flaskFull[0], 0, -2);
	} else if (item == 78) {
		assert(type >= 0 && type < ARRAYSIZE(flaskTable1));
		newItem = flaskTable1[type];
	} else if (item == 79) {
		assert(type >= 0 && type < ARRAYSIZE(flaskTable2));
		newItem = flaskTable2[type];
	}

	if (newItem == -1)
		return;

	setMouseItem(newItem);
	_itemInHand = newItem;

	assert(_fullFlask);
	assert(type < _fullFlask_Size && type >= 0);

	static const uint16 voiceEntries[] = {
		0x1F40, 0x1F41, 0x1F42, 0x1F45
	};
	assert(type < ARRAYSIZE(voiceEntries));

	characterSays(voiceEntries[type], _fullFlask[type], 0, -2);
}

void KyraEngine_LoK::seq_playDrinkPotionAnim(int item, int unk2, int flags) {
	if (_flags.platform == Common::kPlatformAmiga) {
		uint8 r, g, b;

		switch (item) {
		case 60: case 61:
			// 0xC22
			r = 50;
			g = 8;
			b = 8;
			break;

		case 62: case 63: case 76:
		case 77:
			// 0x00E
			r = 0;
			g = 0;
			b = 58;
			break;

		case 64: case 65:
			// 0xFF5
			r = 63;
			g = 63;
			b = 21;
			break;

		case 66:
			// 0x090
			r = 0;
			g = 37;
			b = 0;
			break;

		case 67:
			// 0xC61
			r = 50;
			g = 25;
			b = 4;
			break;

		case 68:
			// 0xE2E
			r = 58;
			g = 8;
			b = 58;
			break;

		case 69:
			// 0xBBB
			r = 46;
			g = 46;
			b = 46;
			break;

		default:
			// 0xFFF
			r = 63;
			g = 63;
			b = 63;
		}

		_screen->setPaletteIndex(16, r, g, b);
	} else {
		uint8 red, green, blue;

		switch (item) {
		case 60: case 61:
			red = 63;
			green = blue = 6;
			break;

		case 62: case 63:
			red = green = 0;
			blue = 67;
			break;

		case 64: case 65:
			red = 84;
			green = 78;
			blue = 14;
			break;

		case 66:
			red = blue = 0;
			green = 48;
			break;

		case 67:
			red = 100;
			green = 48;
			blue = 23;
			break;

		case 68:
			red = 73;
			green = 0;
			blue = 89;
			break;

		case 69:
			red = green = 73;
			blue = 86;
			break;

		default:
			red = 33;
			green = 66;
			blue = 100;
		}

		red   = red * 0x3F / 100;
		green = green * 0x3F / 100;
		blue  = blue * 0x3F / 100;

		_screen->setPaletteIndex(0xFE, red, green, blue);
	}

	_screen->hideMouse();
	checkAmuletAnimFlags();
	_currentCharacter->facing = 5;
	_animator->animRefreshNPC(0);
	assert(_drinkAnimationTable);
	setupShapes123(_drinkAnimationTable, 9, flags);
	_animator->setBrandonAnimSeqSize(5, 54);

	for (int i = 123; i <= 131; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(5);
	}

	snd_playSoundEffect(0x34);

	for (int i = 0; i < 2; ++i) {
		_currentCharacter->currentAnimFrame = 130;
		_animator->animRefreshNPC(0);
		delayWithTicks(7);
		_currentCharacter->currentAnimFrame = 131;
		_animator->animRefreshNPC(0);
		delayWithTicks(7);
	}

	if (unk2) {
		// XXX
	}

	for (int i = 131; i >= 123; --i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(5);
	}

	_animator->resetBrandonAnimSeqSize();
	_currentCharacter->currentAnimFrame = 7;
	_animator->animRefreshNPC(0);
	freeShapes123();

	if (_flags.platform != Common::kPlatformAmiga)
		_screen->setPaletteIndex(0xFE, 30, 30, 30);

	_screen->showMouse();
}

int KyraEngine_LoK::seq_playEnd() {
	if (_endSequenceSkipFlag)
		return 0;

	if (_deathHandler == 8)
		return 0;

	_screen->_curPage = 2;
	if (_endSequenceNeedLoading) {
		snd_playWanderScoreViaMap(50, 1);
		setupPanPages();

		if (_flags.platform == Common::kPlatformAmiga) {
			_sound->loadSoundFile(kMusicFinale);

			// The original started song 0 directly here. Since our player
			// uses 0, 1 for stop and fade we start song 0 with 2
			_sound->playTrack(2);
		}

		_finalA = createWSAMovie();
		assert(_finalA);
		_finalA->open("finala.wsa", 1, 0);

		_finalB = createWSAMovie();
		assert(_finalB);
		_finalB->open("finalb.wsa", 1, 0);

		_finalC = createWSAMovie();
		assert(_finalC);
		_endSequenceNeedLoading = 0;
		_finalC->open("finalc.wsa", 1, 0);

		_screen->_curPage = 0;
		_beadStateVar = 0;
		_malcolmFlag = 0;
		_unkEndSeqVar2 = _system->getMillis() + 600 * _tickLength;

		_screen->copyRegion(312, 0, 312, 0, 8, 136, 0, 2);
	}

	// TODO: better handling. This timer shouldn't count when the menu is open or something.
	if (_unkEndSeqVar2 != -1) {
		if (_system->getMillis() > (uint32)_unkEndSeqVar2) {
			_unkEndSeqVar2 = -1;
			if (!_malcolmFlag)
				_malcolmFlag = 1;
		}
	}

	if (handleMalcolmFlag()) {
		_beadStateVar = 0;
		_malcolmFlag = 12;
		handleMalcolmFlag();
		handleBeadState();
		closeFinalWsa();
		if (_deathHandler == 8) {
			_screen->_curPage = 0;
			checkAmuletAnimFlags();
			seq_brandonToStone();
			delay(60 * _tickLength);
			return 1;
		} else {
			_endSequenceSkipFlag = 1;
			if (_text->printed())
				_text->restoreTalkTextMessageBkgd(2, 0);

			_screen->_curPage = 0;
			_screen->hideMouse();

			if (_flags.platform != Common::kPlatformAmiga)
				_screen->fadeSpecialPalette(32, 228, 20, 60);

			delay(60 * _tickLength);

			_screen->loadBitmap("GEMHEAL.CPS", 3, 3, &_screen->getPalette(0));
			_screen->setScreenPalette(_screen->getPalette(0));
			_screen->shuffleScreen(8, 8, 304, 128, 2, 0, 1, 0);

			uint32 nextTime = _system->getMillis() + 120 * _tickLength;

			_finalA = createWSAMovie();
			assert(_finalA);
			_finalA->open("finald.wsa", 1, 0);

			delayUntil(nextTime);
			snd_playSoundEffect(0x40);
			for (int i = 0; i < 22; ++i) {
				delayUntil(nextTime);
				if (i == 4)
					snd_playSoundEffect(0x3E);
				else if (i == 20)
					snd_playSoundEffect(_flags.platform == Common::kPlatformPC98 ? 0x13 : 0x0E);
				nextTime = _system->getMillis() + 8 * _tickLength;
				_finalA->displayFrame(i, 0, 8, 8, 0, 0, 0);
				_screen->updateScreen();
			}
			delete _finalA;

			_finalA = 0;
			seq_playEnding();
			return 1;
		}
	} else {
		handleBeadState();
		_screen->bitBlitRects();
		_screen->updateScreen();
		_screen->_curPage = 0;
	}
	return 0;
}

void KyraEngine_LoK::seq_brandonToStone() {
	_screen->hideMouse();
	assert(_brandonStoneTable);
	setupShapes123(_brandonStoneTable, 14, 0);
	_animator->setBrandonAnimSeqSize(5, 51);

	for (int i = 123; i <= 136; ++i) {
		_currentCharacter->currentAnimFrame = i;
		_animator->animRefreshNPC(0);
		delayWithTicks(8);
	}

	_animator->resetBrandonAnimSeqSize();
	freeShapes123();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_playEnding() {
	if (shouldQuit())
		return;
	_screen->hideMouse();
	_screen->_curPage = 0;
	_screen->fadeToBlack();

	if (_flags.platform == Common::kPlatformAmiga) {
		_screen->loadBitmap("GEMCUT.CPS", 3, 3, &_screen->getPalette(0));
		_screen->copyRegion(232, 136, 176, 56, 56, 56, 2, 2);
		_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
		_screen->copyRegion(0, 0, 0, 0, 320, 200, 0, 2, Screen::CR_NO_P_CHECK);
	} else {
		_screen->loadBitmap("REUNION.CPS", 3, 3, &_screen->getPalette(0));
		_screen->copyRegion(8, 8, 8, 8, 304, 128, 2, 0);
	}

	_screen->_curPage = 0;
	// XXX
	assert(_homeString);
	drawSentenceCommand(_homeString[0], 179);

	_screen->getPalette(2).clear();
	_screen->setScreenPalette(_screen->getPalette(2));

	_seqPlayerFlag = true;
	_seq->playSequence(_seq_Reunion, false);
	_screen->fadeToBlack();
	_seqPlayerFlag = false;

	_screen->showMouse();

	// To avoid any remaining input events, we remove the queue
	// over here.
	_eventList.clear();

	if (_flags.platform == Common::kPlatformAmiga) {
		_screen->_charWidth = -2;
		_screen->setCurPage(2);

		_screen->getPalette(2).clear();
		_screen->setScreenPalette(_screen->getPalette(2));

		while (!shouldQuit()) {
			seq_playCreditsAmiga();
			delayUntil(_system->getMillis() + 300 * _tickLength);
		}
	} else {
		seq_playCredits();
	}
}

namespace {
struct CreditsLine {
	int16 x, y;
	Screen::FontId font;
	uint8 *str;
};
} // end of anonymous namespace

void KyraEngine_LoK::seq_playCredits() {
	static const uint8 colorMap[] = { 0, 0, 0xC, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
	static const char stringTerms[] = { 0x5, 0xd, 0x0};

	typedef Common::List<CreditsLine> CreditsLineList;
	CreditsLineList lines;

	_screen->enableInterfacePalette(false);

	_screen->hideMouse();
	if (!_flags.isTalkie) {
		_screen->loadFont(Screen::FID_CRED6_FNT, "CREDIT6.FNT");
		_screen->loadFont(Screen::FID_CRED8_FNT, "CREDIT8.FNT");

		_screen->setFont(Screen::FID_CRED8_FNT);
	} else
		_screen->setFont(Screen::FID_8_FNT);

	_screen->loadBitmap("CHALET.CPS", 4, 4, &_screen->getPalette(0));

	_screen->setCurPage(0);
	_screen->clearCurPage();
	_screen->setTextColorMap(colorMap);
	_screen->_charWidth = -1;

	// we only need this for the FM-TOWNS version
	if (_flags.platform == Common::kPlatformFMTowns && _configMusic == 1)
		snd_playWanderScoreViaMap(53, 1);

	uint8 *buffer = 0;
	uint32 size = 0;

	if (_flags.platform == Common::kPlatformFMTowns || _flags.platform == Common::kPlatformPC98) {
		int sizeTmp = 0;
		const uint8 *bufferTmp = _staticres->loadRawData(k1CreditsStrings, sizeTmp);
		buffer = new uint8[sizeTmp];
		assert(buffer);
		memcpy(buffer, bufferTmp, sizeTmp);
		size = sizeTmp;
		_staticres->unloadId(k1CreditsStrings);
	} else {
		buffer = _res->fileData("CREDITS.TXT", &size);
		assert(buffer);
	}

	uint8 *nextString = buffer;
	uint8 *currentString = buffer;
	int currentY = 200;

	do {
		currentString = nextString;
		nextString = (uint8 *)strpbrk((char *)currentString, stringTerms);
		if (!nextString)
			nextString = (uint8 *)strchr((char *)currentString, 0);

		CreditsLine line;

		int lineEndCode = nextString[0];
		*nextString = 0;
		if (lineEndCode != 0)
			nextString++;

		int alignment = 0;
		if (*currentString == 3 || *currentString == 4) {
			alignment = *currentString;
			currentString++;
		}

		if (*currentString == 1) {
			currentString++;

			if (!_flags.isTalkie)
				_screen->setFont(Screen::FID_CRED6_FNT);
		} else if (*currentString == 2) {
			currentString++;

			if (!_flags.isTalkie)
				_screen->setFont(Screen::FID_CRED8_FNT);
		}

		line.font = _screen->_currentFont;

		if (alignment == 3)
			line.x = 157 - _screen->getTextWidth((const char *)currentString);
		else if (alignment == 4)
			line.x = 161;
		else
			line.x = (320  - _screen->getTextWidth((const char *)currentString)) / 2 + 1;

		line.y = currentY;
		if (lineEndCode != 5)
			currentY += 10;

		line.str = currentString;

		lines.push_back(line);
	} while (*nextString);

	_screen->setCurPage(2);

	_screen->getPalette(2).clear();
	_screen->setScreenPalette(_screen->getPalette(2));

	_screen->copyRegion(0, 32, 0, 32, 320, 128, 4, 0, Screen::CR_NO_P_CHECK);
	_screen->fadePalette(_screen->getPalette(0), 0x5A);

	bool finished = false;
	int bottom = 201;
	while (!finished && !shouldQuit()) {
		uint32 startLoop = _system->getMillis();

		if (bottom > 175) {
			_screen->copyRegion(0, 32, 0, 32, 320, 128, 4, 2, Screen::CR_NO_P_CHECK);
			bottom = 0;

			for (CreditsLineList::iterator it = lines.begin(); it != lines.end();) {
				if (it->y < 0) {
					it = lines.erase(it);
					continue;
				}

				if (it->y < 200) {
					if (it->font != _screen->_currentFont)
						_screen->setFont(it->font);

					_screen->printText((const char *)it->str, it->x, it->y, 15, 0);
				}

				it->y--;
				if (it->y > bottom)
					bottom = it->y;

				++it;
			}

			_screen->copyRegion(0, 32, 0, 32, 320, 128, 2, 0, Screen::CR_NO_P_CHECK);
			_screen->updateScreen();
		}

		if (checkInput(0, false)) {
			removeInputTop();
			finished = true;
		}

		uint32 now = _system->getMillis();
		uint32 nextLoop = startLoop + _tickLength * 5;

		if (nextLoop > now)
			_system->delayMillis(nextLoop - now);
	}

	delete[] buffer;

	_screen->fadeToBlack();
	_screen->clearCurPage();
	_screen->showMouse();
}

void KyraEngine_LoK::seq_playCreditsAmiga() {
	_screen->setFont(Screen::FID_8_FNT);

	_screen->loadBitmap("CHALET.CPS", 4, 2, &_screen->getPalette(0));
	_screen->copyPage(2, 0);

	_screen->getPalette(0).fill(16, 1, 63);
	_screen->fadePalette(_screen->getPalette(0), 0x5A);
	_screen->updateScreen();

	const char *theEnd = "THE END";

	const int width = _screen->getTextWidth(theEnd) + 1;
	int x = (320 - width) / 2 + 1;

	_screen->copyRegion(x, 8, x, 8, width, 56, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->copyRegion(x, 8, 0, 8, width, 11, 0, 2, Screen::CR_NO_P_CHECK);
	_screen->printText(theEnd, 0, 10, 31, 0);

	for (int y = 18, h = 1; y >= 10 && !shouldQuit(); --y, ++h) {
		uint32 endTime = _system->getMillis() + 3 * _tickLength;

		_screen->copyRegion(0, y, x, 8, width, h, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		delayUntil(endTime);
	}

	for (int y = 8; y <= 62 && !shouldQuit(); ++y) {
		uint32 endTime = _system->getMillis() + 3 * _tickLength;

		_screen->copyRegion(x, y, 0, 8, width, 11, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->printText(theEnd, 0, 9, 31, 0);
		_screen->copyRegion(0, 8, x, y, width, 11, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		delayUntil(endTime);
	}

	int size = 0;
	const char *creditsData = (const char *)_staticres->loadRawData(k1CreditsStrings, size);

	char stringBuffer[81];
	memset(stringBuffer, 0, sizeof(stringBuffer));

	const char *cur = creditsData;
	char *specialString = stringBuffer;
	bool fillRectFlag = false, subWidth = false, centerFlag = false;
	x = 0;
	int specialX = 0;

	const int fontHeight = _screen->getFontHeight();

	do {
		char code = *cur;

		if (code == 3) {
			fillRectFlag = subWidth = true;
		} else if (code == 5) {
			centerFlag = true;
		} else if (code == 4) {
			if (fillRectFlag) {
				_screen->fillRect(0, 0, 319, 20, 0);

				if (subWidth)
					specialX = 157 - _screen->getTextWidth(stringBuffer);

				_screen->printText(stringBuffer, specialX + 8, 0, 31, 0);
			}

			specialString = stringBuffer;
			*specialString = 0;

			x = 161;
		} else if (code == 13) {
			if (!fillRectFlag)
				_screen->fillRect(0, 0, 319, 20, 0);

			uint32 nextTime = _system->getMillis() + 8 * _tickLength;

			if (centerFlag)
				x = (320 - _screen->getTextWidth(stringBuffer)) / 2 - 8;

			_screen->printText(stringBuffer, x + 8, 0, 31, 0);

			for (int i = 0; i < fontHeight && !shouldQuit(); ++i) {
				_screen->copyRegion(0, 141, 0, 140, 320, 59, 0, 0, Screen::CR_NO_P_CHECK);
				_screen->copyRegion(0, i, 0, 198, 320, 3, 2, 0, Screen::CR_NO_P_CHECK);
				_screen->updateScreen();

				delayUntil(nextTime);
				nextTime = _system->getMillis() + 8 * _tickLength;
			}

			specialString = stringBuffer;
			*specialString = 0;

			centerFlag = fillRectFlag = false;
		} else {
			*specialString++ = code;
			*specialString = 0;
		}

		if (checkInput(0, false)) {
			removeInputTop();
			break;
		}
	} while (++cur != (creditsData + size) && !shouldQuit());
}

bool KyraEngine_LoK::seq_skipSequence() const {
	return shouldQuit() || _abortIntroFlag;
}

int KyraEngine_LoK::handleMalcolmFlag() {
	switch (_malcolmFlag) {
	case 1:
		_malcolmFrame = 0;
		_malcolmFlag = 2;
		_malcolmTimer2 = 0;

		// Fall through to the next case

	case 2:
		if (_system->getMillis() >= _malcolmTimer2) {
			_finalA->displayFrame(_malcolmFrame, 0, 8, 46, 0, 0, 0);
			_screen->updateScreen();
			_malcolmTimer2 = _system->getMillis() + 8 * _tickLength;
			++_malcolmFrame;
			if (_malcolmFrame > 13) {
				_malcolmFlag = 3;
				_malcolmTimer1 = _system->getMillis() + 180 * _tickLength;
			}
		}
		break;

	case 3:
		if (_system->getMillis() < _malcolmTimer1) {
			if (_system->getMillis() >= _malcolmTimer2) {
				_malcolmFrame = _rnd.getRandomNumberRng(14, 17);
				_finalA->displayFrame(_malcolmFrame, 0, 8, 46, 0, 0, 0);
				_screen->updateScreen();
				_malcolmTimer2 = _system->getMillis() + 8 * _tickLength;
			}
		} else {
			_malcolmFlag = 4;
			_malcolmFrame = 18;
		}
		break;

	case 4:
		if (_system->getMillis() >= _malcolmTimer2) {
			_finalA->displayFrame(_malcolmFrame, 0, 8, 46, 0, 0, 0);
			_screen->updateScreen();
			_malcolmTimer2 = _system->getMillis() + 8 * _tickLength;
			++_malcolmFrame;
			if (_malcolmFrame > 25) {
				_malcolmFrame = 26;
				_malcolmFlag = 5;
				_beadStateVar = 1;
			}
		}
		break;

	case 5:
		if (_system->getMillis() >= _malcolmTimer2) {
			_finalA->displayFrame(_malcolmFrame, 0, 8, 46, 0, 0, 0);
			_screen->updateScreen();
			_malcolmTimer2 = _system->getMillis() + 8 * _tickLength;
			++_malcolmFrame;
			if (_malcolmFrame > 31) {
				_malcolmFrame = 32;
				_malcolmFlag = 6;
			}
		}
		break;

	case 6:
		if (_unkEndSeqVar4) {
			if (_malcolmFrame <= 33 && _system->getMillis() >= _malcolmTimer2) {
				_finalA->displayFrame(_malcolmFrame, 0, 8, 46, 0, 0, 0);
				_screen->updateScreen();
				_malcolmTimer2 = _system->getMillis() + 8 * _tickLength;
				++_malcolmFrame;
				if (_malcolmFrame > 33) {
					_malcolmFlag = 7;
					_malcolmFrame = 32;
					_unkEndSeqVar5 = 0;
				}
			}
		}
		break;

	case 7:
		if (_unkEndSeqVar5 == 1) {
			_malcolmFlag = 8;
			_malcolmFrame = 34;
		} else if (_unkEndSeqVar5 == 2) {
			_malcolmFlag = 3;
			_malcolmTimer1 = _system->getMillis() + 180 * _tickLength;
		}
		break;

	case 8:
		if (_system->getMillis() >= _malcolmTimer2) {
			_finalA->displayFrame(_malcolmFrame, 0, 8, 46, 0, 0, 0);
			_screen->updateScreen();
			_malcolmTimer2 = _system->getMillis() + 8 * _tickLength;
			++_malcolmFrame;
			if (_malcolmFrame > 37) {
				_malcolmFlag = 0;
				_deathHandler = 8;
				return 1;
			}
		}
		break;

	case 9:
		snd_playSoundEffect(12);
		snd_playSoundEffect(12);
		for (int i = 0; i < 18; ++i) {
			_malcolmTimer2 = _system->getMillis() + 4 * _tickLength;
			_finalC->displayFrame(i, 0, 16, 50, 0, 0, 0);
			_screen->updateScreen();
			delayUntil(_malcolmTimer2);
		}
		snd_playWanderScoreViaMap(51, 1);
		delay(60 * _tickLength);
		_malcolmFlag = 0;
		return 1;

	case 10:
		if (!_beadStateVar) {
			handleBeadState();
			_screen->bitBlitRects();
			assert(_veryClever);
			_text->printTalkTextMessage(_veryClever[0], 60, 31, 5, 0, 2);
			_malcolmTimer2 = _system->getMillis() + 180 * _tickLength;
			_malcolmFlag = 11;
		}
		break;

	case 11:
		if (_system->getMillis() >= _malcolmTimer2) {
			_text->restoreTalkTextMessageBkgd(2, 0);
			_malcolmFlag = 3;
			_malcolmTimer1 = _system->getMillis() + 180 * _tickLength;
		}
		break;

	default:
		break;
	}

	return 0;
}

int KyraEngine_LoK::handleBeadState() {
	static const int table1[] = {
		-1, -2, -4, -5, -6, -7, -6, -5,
		-4, -2, -1,  0,  1,  2,  4,  5,
		 6,  7,  6,  5,  4,  2,  1,  0, 0
	};

	static const int table2[] = {
		0, 0, 1, 1, 2, 2, 3, 3,
		4, 4, 5, 5, 5, 5, 4, 4,
		3, 3, 2, 2, 1, 1, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0,
		0, 0, 0, 0, 0, 0, 0, 0, 0
	};

	switch (_beadStateVar) {
	case 0:
		if (_beadState1.x != -1 && _endSequenceBackUpRect) {
			_screen->copyBlockToPage(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
			_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
		}

		_beadState1.x = -1;
		_beadState1.tableIndex = 0;
		_beadStateTimer1 = 0;
		_beadStateTimer2 = 0;
		_lastDisplayedPanPage = 0;
		return 1;

	case 1:
		if (_beadState1.x != -1) {
			if (_endSequenceBackUpRect) {
				_screen->copyBlockToPage(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
				_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
			}
			_beadState1.x = -1;
			_beadState1.tableIndex = 0;
		}
		_beadStateVar = 2;
		break;

	case 2:
		if (_system->getMillis() >= _beadStateTimer1) {
			int x = 0, y = 0;
			_beadStateTimer1 = _system->getMillis() + 4 * _tickLength;
			if (_beadState1.x == -1) {
				assert(_panPagesTable);
				_beadState1.width2 = _animator->fetchAnimWidth(_panPagesTable[19], 256);
				_beadState1.width = ((_beadState1.width2 + 7) >> 3) + 1;
				_beadState1.height = _animator->fetchAnimHeight(_panPagesTable[19], 256);
				if (!_endSequenceBackUpRect) {
					_endSequenceBackUpRect = new uint8[(_beadState1.width * _beadState1.height) << 3];
					assert(_endSequenceBackUpRect);
					memset(_endSequenceBackUpRect, 0, ((_beadState1.width * _beadState1.height) << 3) * sizeof(uint8));
				}
				x = _beadState1.x = 60;
				y = _beadState1.y = 40;
				initBeadState(x, y, x, 25, 8, &_beadState2);
			} else {
				if (processBead(_beadState1.x, _beadState1.y, x, y, &_beadState2)) {
					_beadStateVar = 3;
					_beadStateTimer2 = _system->getMillis() + 240 * _tickLength;
					_unkEndSeqVar4 = 0;
					_beadState1.dstX = _beadState1.x;
					_beadState1.dstY = _beadState1.y;
					return 0;
				} else {
					_screen->copyBlockToPage(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
					_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
					_beadState1.x = x;
					_beadState1.y = y;
				}
			}

			_screen->copyRegionToBuffer(_screen->_curPage, x, y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
			_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);

			if (_lastDisplayedPanPage > 17)
				_lastDisplayedPanPage = 0;

			_screen->addBitBlitRect(x, y, _beadState1.width2, _beadState1.height);
		}
		break;

	case 3:
		if (_system->getMillis() >= _beadStateTimer1) {
			_beadStateTimer1 = _system->getMillis() + 4 * _tickLength;
			_screen->copyBlockToPage(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
			_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);

			_beadState1.x = _beadState1.dstX + table1[_beadState1.tableIndex];
			_beadState1.y = _beadState1.dstY + table2[_beadState1.tableIndex];
			_screen->copyRegionToBuffer(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);

			_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], _beadState1.x, _beadState1.y, 0, 0);
			if (_lastDisplayedPanPage >= 17)
				_lastDisplayedPanPage = 0;

			_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);

			++_beadState1.tableIndex;
			if (_beadState1.tableIndex > 24)
				_beadState1.tableIndex = 0;
				_unkEndSeqVar4 = 1;
			if (_system->getMillis() > _beadStateTimer2 && _malcolmFlag == 7 && !_unkAmuletVar && !_text->printed()) {
				snd_playSoundEffect(0x0B);
				if (_currentCharacter->x1 > 233 && _currentCharacter->x1 < 305 && _currentCharacter->y1 > 85 && _currentCharacter->y1 < 105 &&
				        (_brandonStatusBit & 0x20)) {
					_beadState1.unk8 = 290;
					_beadState1.unk9 = 40;
					_beadStateVar = 5;
				} else {
					_beadStateVar = 4;
					_beadState1.unk8 = _currentCharacter->x1 - 4;
					_beadState1.unk9 = _currentCharacter->y1 - 30;
				}

				if (_text->printed())
					_text->restoreTalkTextMessageBkgd(2, 0);

				initBeadState(_beadState1.x, _beadState1.y, _beadState1.unk8, _beadState1.unk9, 12, &_beadState2);
				_lastDisplayedPanPage = 18;
			}
		}
		break;

	case 4:
		if (_system->getMillis() >= _beadStateTimer1) {
			int x = 0, y = 0;
			_beadStateTimer1 = _system->getMillis() + _tickLength;
			if (processBead(_beadState1.x, _beadState1.y, x, y, &_beadState2)) {
				if (_brandonStatusBit & 20) {
					_unkEndSeqVar5 = 2;
					_beadStateVar = 6;
				} else {
					snd_playWanderScoreViaMap(52, 1);
					snd_playSoundEffect(0x0C);
					_unkEndSeqVar5 = 1;
					_beadStateVar = 0;
				}
			} else {
				_screen->copyBlockToPage(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
				_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
				_beadState1.x = x;
				_beadState1.y = y;
				_screen->copyRegionToBuffer(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
				_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);
				if (_lastDisplayedPanPage > 17) {
					_lastDisplayedPanPage = 0;
				}
				_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
			}
		}
		break;

	case 5:
		if (_system->getMillis() >= _beadStateTimer1) {
			_beadStateTimer1 = _system->getMillis() + _tickLength;
			int x = 0, y = 0;
			if (processBead(_beadState1.x, _beadState1.y, x, y, &_beadState2)) {
				if (_beadState2.dstX == 290) {
					_screen->copyBlockToPage(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
					uint32 nextRun = 0;
					for (int i = 0; i < 8; ++i) {
						nextRun = _system->getMillis() + _tickLength;
						_finalB->displayFrame(i, 0, 224, 8, 0, 0, 0);
						_screen->updateScreen();
						delayUntil(nextRun);
					}
					snd_playSoundEffect(0x0D);
					for (int i = 7; i >= 0; --i) {
						nextRun = _system->getMillis() + _tickLength;
						_finalB->displayFrame(i, 0, 224, 8, 0, 0, 0);
						_screen->updateScreen();
						delayUntil(nextRun);
					}
					initBeadState(_beadState1.x, _beadState1.y, 63, 60, 12, &_beadState2);
				} else {
					_screen->copyBlockToPage(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
					_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
					_beadState1.x = -1;
					_beadState1.tableIndex = 0;
					_beadStateVar = 0;
					_malcolmFlag = 9;
				}
			} else {
				_screen->copyBlockToPage(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
				_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
				_beadState1.x = x;
				_beadState1.y = y;
				_screen->copyRegionToBuffer(_screen->_curPage, _beadState1.x, _beadState1.y, _beadState1.width << 3, _beadState1.height, _endSequenceBackUpRect);
				_screen->drawShape(2, _panPagesTable[_lastDisplayedPanPage++], x, y, 0, 0);
				if (_lastDisplayedPanPage > 17)
					_lastDisplayedPanPage = 0;
				_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
			}
		}
		break;

	case 6:
		_screen->drawShape(2, _panPagesTable[19], _beadState1.x, _beadState1.y, 0, 0);
		_screen->addBitBlitRect(_beadState1.x, _beadState1.y, _beadState1.width2, _beadState1.height);
		_beadStateVar = 0;
		break;

	default:
		break;
	}

	return 0;
}

void KyraEngine_LoK::initBeadState(int x, int y, int x2, int y2, int unk, BeadState *ptr) {
	ptr->unk9 = unk;
	int xDiff = x2 - x;
	int yDiff = y2 - y;
	int unk1 = 0, unk2 = 0;
	if (xDiff > 0)
		unk1 = 1;
	else if (xDiff == 0)
		unk1 = 0;
	else
		unk1 = -1;


	if (yDiff > 0)
		unk2 = 1;
	else if (yDiff == 0)
		unk2 = 0;
	else
		unk2 = -1;

	xDiff = ABS(xDiff);
	yDiff = ABS(yDiff);

	ptr->y = 0;
	ptr->x = 0;
	ptr->width = xDiff;
	ptr->height = yDiff;
	ptr->dstX = x2;
	ptr->dstY = y2;
	ptr->width2 = unk1;
	ptr->unk8 = unk2;
}

int KyraEngine_LoK::processBead(int x, int y, int &x2, int &y2, BeadState *ptr) {
	if (x == ptr->dstX && y == ptr->dstY)
		return 1;

	int xPos = x, yPos = y;
	if (ptr->width >= ptr->height) {
		for (int i = 0; i < ptr->unk9; ++i) {
			ptr->y += ptr->height;
			if (ptr->y >= ptr->width) {
				ptr->y -= ptr->width;
				yPos += ptr->unk8;
			}
			xPos += ptr->width2;
		}
	} else {
		for (int i = 0; i < ptr->unk9; ++i) {
			ptr->x += ptr->width;
			if (ptr->x >= ptr->height) {
				ptr->x -= ptr->height;
				xPos += ptr->width2;
			}
			yPos += ptr->unk8;
		}
	}

	int temp = ABS(x - ptr->dstX);
	if (ptr->unk9 > temp)
		xPos = ptr->dstX;
	temp = ABS(y - ptr->dstY);
	if (ptr->unk9 > temp)
		yPos = ptr->dstY;
	x2 = xPos;
	y2 = yPos;
	return 0;
}

void KyraEngine_LoK::setupPanPages() {
	_screen->savePageToDisk("BKGD.PG", 2);
	_screen->loadBitmap("BEAD.CPS", 3, 3, 0);
	if (_flags.platform == Common::kPlatformMacintosh || _flags.platform == Common::kPlatformAmiga) {
		int pageBackUp = _screen->_curPage;
		_screen->_curPage = 2;

		delete[] _panPagesTable[19];
		_panPagesTable[19] = _screen->encodeShape(0, 0, 16, 9, 0);
		assert(_panPagesTable[19]);

		int curX = 16;
		for (int i = 0; i < 19; ++i) {
			delete[] _panPagesTable[i];
			_panPagesTable[i] = _screen->encodeShape(curX, 0, 8, 5, 0);
			assert(_panPagesTable[i]);
			curX += 8;
		}

		_screen->_curPage = pageBackUp;
	} else {
		for (int i = 0; i <= 19; ++i) {
			delete[] _panPagesTable[i];
			_panPagesTable[i] = _seq->setPanPages(3, i);
			assert(_panPagesTable[i]);
		}
	}
	_screen->loadPageFromDisk("BKGD.PG", 2);
}

void KyraEngine_LoK::freePanPages() {
	delete[] _endSequenceBackUpRect;
	_endSequenceBackUpRect = 0;
	for (int i = 0; i <= 19; ++i) {
		delete[] _panPagesTable[i];
		_panPagesTable[i] = 0;
	}
}

void KyraEngine_LoK::closeFinalWsa() {
	delete _finalA;
	_finalA = 0;
	delete _finalB;
	_finalB = 0;
	delete _finalC;
	_finalC = 0;
	freePanPages();
	_endSequenceNeedLoading = 1;
}

void KyraEngine_LoK::updateKyragemFading() {
	if (_flags.platform == Common::kPlatformAmiga) {
		// The AMIGA version seems to have no fading for the Kyragem. The code does not
		// alter the screen palette.
		//
		// TODO: Check this in the original.
		return;
	}

	static const uint8 kyraGemPalette[0x28] = {
		0x3F, 0x3B, 0x38, 0x34, 0x32, 0x2F, 0x2C, 0x29, 0x25, 0x22,
		0x1F, 0x1C, 0x19, 0x16, 0x12, 0x0F, 0x0C, 0x0A, 0x06, 0x03,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};

	if (_system->getMillis() < _kyragemFadingState.timerCount)
		return;

	_kyragemFadingState.timerCount = _system->getMillis() + 4 * _tickLength;

	int palPos = 684;
	for (int i = 0; i < 20; ++i) {
		_screen->getPalette(0)[palPos++] = kyraGemPalette[i + _kyragemFadingState.rOffset];
		_screen->getPalette(0)[palPos++] = kyraGemPalette[i + _kyragemFadingState.gOffset];
		_screen->getPalette(0)[palPos++] = kyraGemPalette[i + _kyragemFadingState.bOffset];
	}

	_screen->setScreenPalette(_screen->getPalette(0));

	switch (_kyragemFadingState.nextOperation) {
	case 0:
		--_kyragemFadingState.bOffset;
		if (_kyragemFadingState.bOffset >= 1)
			return;
		_kyragemFadingState.nextOperation = 1;
		break;

	case 1:
		++_kyragemFadingState.rOffset;
		if (_kyragemFadingState.rOffset < 19)
			return;
		_kyragemFadingState.nextOperation = 2;
		break;

	case 2:
		--_kyragemFadingState.gOffset;
		if (_kyragemFadingState.gOffset >= 1)
			return;
		_kyragemFadingState.nextOperation = 3;
		break;

	case 3:
		++_kyragemFadingState.bOffset;
		if (_kyragemFadingState.bOffset < 19)
			return;
		_kyragemFadingState.nextOperation = 4;
		break;

	case 4:
		--_kyragemFadingState.rOffset;
		if (_kyragemFadingState.rOffset >= 1)
			return;
		_kyragemFadingState.nextOperation = 5;
		break;

	case 5:
		++_kyragemFadingState.gOffset;
		if (_kyragemFadingState.gOffset < 19)
			return;
		_kyragemFadingState.nextOperation = 0;
		break;

	default:
		break;
	}

	_kyragemFadingState.timerCount = _system->getMillis() + 120 * _tickLength;
}

void KyraEngine_LoK::drawJewelPress(int jewel, int drawSpecial) {
	_screen->hideMouse();
	int shape = 0;

	if (drawSpecial)
		shape = 0x14E;
	else
		shape = jewel + 0x149;

	snd_playSoundEffect(0x45);
	_screen->drawShape(0, _shapes[shape], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	delayWithTicks(2);

	if (drawSpecial)
		shape = 0x148;
	else
		shape = jewel + 0x143;

	_screen->drawShape(0, _shapes[shape], _amuletX2[jewel], _amuletY2[jewel], 0, 0);
	_screen->updateScreen();
	_screen->showMouse();
}

void KyraEngine_LoK::drawJewelsFadeOutStart() {
	static const uint16 jewelTable1[] = { 0x164, 0x15F, 0x15A, 0x155, 0x150, 0xFFFF };
	static const uint16 jewelTable2[] = { 0x163, 0x15E, 0x159, 0x154, 0x14F, 0xFFFF };
	static const uint16 jewelTable3[] = { 0x166, 0x160, 0x15C, 0x157, 0x152, 0xFFFF };
	static const uint16 jewelTable4[] = { 0x165, 0x161, 0x15B, 0x156, 0x151, 0xFFFF };
	for (int i = 0; jewelTable1[i] != 0xFFFF; ++i) {
		if (queryGameFlag(0x57))
			_screen->drawShape(0, _shapes[jewelTable1[i]], _amuletX2[2], _amuletY2[2], 0, 0);
		if (queryGameFlag(0x59))
			_screen->drawShape(0, _shapes[jewelTable3[i]], _amuletX2[4], _amuletY2[4], 0, 0);
		if (queryGameFlag(0x56))
			_screen->drawShape(0, _shapes[jewelTable2[i]], _amuletX2[1], _amuletY2[1], 0, 0);
		if (queryGameFlag(0x58))
			_screen->drawShape(0, _shapes[jewelTable4[i]], _amuletX2[3], _amuletY2[3], 0, 0);
		_screen->updateScreen();
		delayWithTicks(3);
	}
}

void KyraEngine_LoK::drawJewelsFadeOutEnd(int jewel) {
	static const uint16 jewelTable[] = { 0x153, 0x158, 0x15D, 0x162, 0x148, 0xFFFF };
	int newDelay = 0;

	switch (jewel - 1) {
	case 2:
		if (_currentCharacter->sceneId >= 109 && _currentCharacter->sceneId <= 198)
			newDelay = 18900;
		else
			newDelay = 8100;
		break;

	default:
		newDelay = 3600;
	}

	setGameFlag(0xF1);
	_timer->setCountdown(19, newDelay);
	_screen->hideMouse();
	for (int i = 0; jewelTable[i] != 0xFFFF; ++i) {
		uint16 shape = jewelTable[i];
		if (queryGameFlag(0x57))
			_screen->drawShape(0, _shapes[shape], _amuletX2[2], _amuletY2[2], 0, 0);
		if (queryGameFlag(0x59))
			_screen->drawShape(0, _shapes[shape], _amuletX2[4], _amuletY2[4], 0, 0);
		if (queryGameFlag(0x56))
			_screen->drawShape(0, _shapes[shape], _amuletX2[1], _amuletY2[1], 0, 0);
		if (queryGameFlag(0x58))
			_screen->drawShape(0, _shapes[shape], _amuletX2[3], _amuletY2[3], 0, 0);

		_screen->updateScreen();
		delayWithTicks(3);
	}
	_screen->showMouse();
}

} // End of namespace Kyra
