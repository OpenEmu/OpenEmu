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


#include "common/error.h"
#include "common/events.h"
#include "common/system.h"
#include "queen/journal.h"

#include "queen/bankman.h"
#include "queen/display.h"
#include "queen/graphics.h"
#include "queen/grid.h"
#include "queen/logic.h"
#include "queen/music.h"
#include "queen/queen.h"
#include "queen/resource.h"
#include "queen/sound.h"

namespace Queen {

Journal::Journal(QueenEngine *vm)
	: _vm(vm) {
	_currentSavePage = 0;
	_currentSaveSlot = 0;
}

void Journal::use() {
	BobSlot *joe = _vm->graphics()->bob(0);
	_prevJoeX = joe->x;
	_prevJoeY = joe->y;

	_panelMode = PM_NORMAL;
	_system = g_system;

	_panelTextCount = 0;
	memset(_panelTextY, 0, sizeof(_panelTextY));
	memset(&_textField, 0, sizeof(_textField));

	memset(_saveDescriptions, 0, sizeof(_saveDescriptions));
	_vm->findGameStateDescriptions(_saveDescriptions);

	setup();
	redraw();
	update();
	_vm->display()->palFadeIn(ROOM_JOURNAL);

	_quitMode = QM_LOOP;
	while (_quitMode == QM_LOOP) {
		Common::Event event;
		Common::EventManager *eventMan = _system->getEventManager();
		while (eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				handleKeyDown(event.kbd.ascii, event.kbd.keycode);
				break;
			case Common::EVENT_LBUTTONDOWN:
				handleMouseDown(event.mouse.x, event.mouse.y);
				break;
			case Common::EVENT_WHEELUP:
				handleMouseWheel(-1);
				break;
			case Common::EVENT_WHEELDOWN:
				handleMouseWheel(1);
				break;
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				return;
			default:
				break;
			}
		}
		_system->delayMillis(20);
		_system->updateScreen();
	}

	_vm->writeOptionSettings();

	_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
	_vm->graphics()->putCameraOnBob(0);
	if (_quitMode == QM_CONTINUE) {
		continueGame();
	}
}

void Journal::continueGame() {
	_vm->display()->fullscreen(false);
	_vm->display()->forceFullRefresh();

	_vm->logic()->joePos(_prevJoeX, _prevJoeY);
	_vm->logic()->joeCutFacing(_vm->logic()->joeFacing());

	_vm->logic()->oldRoom(_vm->logic()->currentRoom());
	_vm->logic()->displayRoom(_vm->logic()->currentRoom(), RDM_FADE_JOE, 0, 0, false);
}

void Journal::setup() {
	_vm->display()->palFadeOut(_vm->logic()->currentRoom());
	_vm->display()->horizontalScroll(0);
	_vm->display()->fullscreen(true);
	_vm->graphics()->clearBobs();
	_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
	_vm->bankMan()->eraseFrames(false);
	_vm->display()->textCurrentColor(_vm->display()->getInkColor(INK_JOURNAL));

	_vm->grid()->clear(GS_ROOM);
	for (int i = 0; i < MAX_ZONES; ++i) {
		const Zone *zn = &_zones[i];
		_vm->grid()->setZone(GS_ROOM, zn->num, zn->x1, zn->y1, zn->x2, zn->y2);
	}

	_vm->display()->setupNewRoom("journal", ROOM_JOURNAL);
	_vm->bankMan()->load("journal.BBK", JOURNAL_BANK);
	for (int f = 1; f <= 20; ++f) {
		int frameNum = JOURNAL_FRAMES + f;
		_vm->bankMan()->unpack(f, frameNum, JOURNAL_BANK);
		BobFrame *bf = _vm->bankMan()->fetchFrame(frameNum);
		bf->xhotspot = 0;
		bf->yhotspot = 0;
		if (f == FRAME_INFO_BOX) { // adjust info box hot spot to put it always on top
			bf->yhotspot = 200;
		}
	}
	_vm->bankMan()->close(JOURNAL_BANK);

	_textField.x = 136;
	_textField.y = 9;
	_textField.w = 146;
	_textField.h = 13;
}

void Journal::redraw() {
	drawNormalPanel();
	drawConfigPanel();
	drawSaveDescriptions();
	drawSaveSlot();
}

void Journal::update() {
	_vm->graphics()->sortBobs();
	_vm->display()->prepareUpdate();
	_vm->graphics()->drawBobs();
	if (_textField.enabled) {
		int16 x = _textField.x + _textField.posCursor;
		int16 y = _textField.y + _currentSaveSlot * _textField.h + 8;
		_vm->display()->drawBox(x, y, x + 6, y, _vm->display()->getInkColor(INK_JOURNAL));
	}
	_vm->display()->forceFullRefresh();
	_vm->display()->update();
	_system->updateScreen();
}

void Journal::showBob(int bobNum, int16 x, int16 y, int frameNum) {
	BobSlot *bob = _vm->graphics()->bob(bobNum);
	bob->curPos(x, y);
	bob->frameNum = JOURNAL_FRAMES + frameNum;
}

void Journal::hideBob(int bobNum) {
	_vm->graphics()->bob(bobNum)->active = false;
}

void Journal::drawSaveDescriptions() {
	for (int i = 0; i < NUM_SAVES_PER_PAGE; ++i) {
		int n = _currentSavePage * 10 + i;
		char nb[4];
		sprintf(nb, "%d", n + 1);
		int y = _textField.y + i * _textField.h;
		_vm->display()->setText(_textField.x, y, _saveDescriptions[n], false);
		_vm->display()->setText(_textField.x - 27, y + 1, nb, false);
	}
	// highlight current page
	showBob(BOB_SAVE_PAGE, 300, 3 + _currentSavePage * 15, 6 + _currentSavePage);
}

void Journal::drawSaveSlot() {
	showBob(BOB_SAVE_DESC, 130, 6 + _currentSaveSlot * 13, 17);
}

void Journal::enterYesNoPanelMode(int16 prevZoneNum, int titleNum) {
	_panelMode = PM_YES_NO;
	_prevZoneNum = prevZoneNum;
	drawYesNoPanel(titleNum);
}

void Journal::exitYesNoPanelMode() {
	_panelMode = PM_NORMAL;
	if (_prevZoneNum == ZN_MAKE_ENTRY) {
		closeTextField();
	}
	redraw();
}

void Journal::enterInfoPanelMode() {
	_panelMode = PM_INFO_BOX;
	_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
	drawInfoPanel();
}

void Journal::exitInfoPanelMode() {
	_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
	hideBob(BOB_INFO_BOX);
	redraw();
	_panelMode = PM_NORMAL;
}

void Journal::handleKeyDown(uint16 ascii, int keycode) {
	switch (_panelMode) {
	case PM_INFO_BOX:
		break;
	case PM_YES_NO:
		if (keycode == Common::KEYCODE_ESCAPE) {
			exitYesNoPanelMode();
		} else if (_textField.enabled) {
			updateTextField(ascii, keycode);
		}
		break;
	case PM_NORMAL:
		if (keycode == Common::KEYCODE_ESCAPE) {
			_quitMode = QM_CONTINUE;
		}
		break;
	}
}

void Journal::handleMouseWheel(int inc) {
	if (_panelMode == PM_NORMAL) {
		int curSave = _currentSavePage * NUM_SAVES_PER_PAGE + _currentSaveSlot + inc;
		if (curSave >= 0 && curSave < NUM_SAVES_PER_PAGE * 10) {
			_currentSavePage = curSave / NUM_SAVES_PER_PAGE;
			_currentSaveSlot = curSave % NUM_SAVES_PER_PAGE;
			drawSaveDescriptions();
			drawSaveSlot();
			update();
		}
	}
}

void Journal::handleMouseDown(int x, int y) {
	int val;
	int16 zoneNum = _vm->grid()->findZoneForPos(GS_ROOM, x, y);
	switch (_panelMode) {
	case PM_INFO_BOX:
		exitInfoPanelMode();
		break;
	case PM_YES_NO:
		if (zoneNum == ZN_YES) {
			_panelMode = PM_NORMAL;
			int currentSlot = _currentSavePage * 10 + _currentSaveSlot;
			switch (_prevZoneNum) {
			case ZN_REVIEW_ENTRY:
				if (_saveDescriptions[currentSlot][0]) {
					_vm->graphics()->clearBobs();
					_vm->display()->palFadeOut(ROOM_JOURNAL);
					_vm->sound()->stopSong();
					_vm->loadGameState(currentSlot);
					_vm->display()->clearTexts(0, GAME_SCREEN_HEIGHT - 1);
					_quitMode = QM_RESTORE;
				} else {
					exitYesNoPanelMode();
				}
				break;
			case ZN_MAKE_ENTRY:
				if (_textField.text[0]) {
					closeTextField();
					_vm->saveGameState(currentSlot, _textField.text);
					_quitMode = QM_CONTINUE;
				} else {
					exitYesNoPanelMode();
				}
				break;
			case ZN_GIVEUP:
				_quitMode = QM_CONTINUE;
				_vm->quitGame();
				break;
			}
		} else if (zoneNum == ZN_NO) {
			exitYesNoPanelMode();
		}
		break;
	case PM_NORMAL:
		switch (zoneNum) {
		case ZN_REVIEW_ENTRY:
			enterYesNoPanelMode(zoneNum, TXT_REVIEW_ENTRY);
			break;
		case ZN_MAKE_ENTRY:
			initTextField(_saveDescriptions[_currentSavePage * 10 + _currentSaveSlot]);
			enterYesNoPanelMode(zoneNum, TXT_MAKE_ENTRY);
			break;
		case ZN_CLOSE:
			_quitMode = QM_CONTINUE;
			break;
		case ZN_GIVEUP:
			enterYesNoPanelMode(zoneNum, TXT_GIVE_UP);
			break;
		case ZN_TEXT_SPEED:
			val = (x - 136) * QueenEngine::MAX_TEXT_SPEED / (266 - 136);
			_vm->talkSpeed(val);
			drawConfigPanel();
			break;
		case ZN_SFX_TOGGLE:
			_vm->sound()->toggleSfx();
			drawConfigPanel();
			break;
		case ZN_MUSIC_VOLUME:
			val = (x - 136) * Audio::Mixer::kMaxMixerVolume / (266 - 136);
			_vm->sound()->setVolume(val);
			drawConfigPanel();
			break;
		case ZN_DESC_1:
		case ZN_DESC_2:
		case ZN_DESC_3:
		case ZN_DESC_4:
		case ZN_DESC_5:
		case ZN_DESC_6:
		case ZN_DESC_7:
		case ZN_DESC_8:
		case ZN_DESC_9:
		case ZN_DESC_10:
			_currentSaveSlot = zoneNum - ZN_DESC_1;
			drawSaveSlot();
			break;
		case ZN_PAGE_A:
		case ZN_PAGE_B:
		case ZN_PAGE_C:
		case ZN_PAGE_D:
		case ZN_PAGE_E:
		case ZN_PAGE_F:
		case ZN_PAGE_G:
		case ZN_PAGE_H:
		case ZN_PAGE_I:
		case ZN_PAGE_J:
			_currentSavePage = zoneNum - ZN_PAGE_A;
			drawSaveDescriptions();
			break;
		case ZN_INFO_BOX:
			enterInfoPanelMode();
			break;
		case ZN_MUSIC_TOGGLE:
			_vm->sound()->toggleMusic();
			if (_vm->sound()->musicOn()) {
				_vm->sound()->playLastSong();
			} else {
				_vm->sound()->stopSong();
			}
			drawConfigPanel();
			break;
		case ZN_VOICE_TOGGLE:
			_vm->sound()->toggleSpeech();
			drawConfigPanel();
			break;
		case ZN_TEXT_TOGGLE:
			_vm->subtitles(!_vm->subtitles());
			drawConfigPanel();
			break;
		}
		break;
	}
	update();
}

static void removeLeadingAndTrailingSpaces(char *dst, size_t dstSize, const char* src) {
	assert(dstSize > 0);
	size_t srcLen = strlen(src);
	if (0 == srcLen) {
		dst[0] = '\0';
		return;
	}

	size_t firstNonSpaceIndex;
	for (firstNonSpaceIndex = 0; firstNonSpaceIndex < srcLen; ++firstNonSpaceIndex) {
		if (src[firstNonSpaceIndex] != ' ')
			break;
	}
	if (firstNonSpaceIndex == srcLen) {
		dst[0] = '\0';
		return;
	}

	size_t lastNonSpaceIndex = srcLen - 1;
	while (src[lastNonSpaceIndex] == ' ')
		--lastNonSpaceIndex;

	size_t newLen = lastNonSpaceIndex - firstNonSpaceIndex + 1;
	assert(newLen < dstSize);
	for (size_t i = 0; i < newLen; ++i) {
		dst[i] = src[firstNonSpaceIndex + i];
	}
	dst[newLen] = '\0';
}

void Journal::drawPanelText(int y, const char *text) {
	debug(7, "Journal::drawPanelText(%d, '%s')", y, text);

	char s[128];
	removeLeadingAndTrailingSpaces(s, 128, text); // necessary for spanish version

	// draw the substrings
	char *p = strchr(s, ' ');
	if (!p) {
		int x = (128 - _vm->display()->textWidth(s)) / 2;
		_vm->display()->setText(x, y, s, false);
		assert(_panelTextCount < MAX_PANEL_TEXTS);
		_panelTextY[_panelTextCount++] = y;
	} else {
		*p++ = '\0';
		if (_vm->resource()->getLanguage() == Common::HE_ISR) {
			drawPanelText(y - 5, p);
			drawPanelText(y + 5, s);
		} else {
			drawPanelText(y - 5, s);
			drawPanelText(y + 5, p);
		}
	}
}

void Journal::drawCheckBox(bool active, int bobNum, int16 x, int16 y, int frameNum) {
	if (active) {
		showBob(bobNum, x, y, frameNum);
	} else {
		hideBob(bobNum);
	}
}

void Journal::drawSlideBar(int value, int maxValue, int bobNum, int16 y, int frameNum) {
	showBob(bobNum, 136 + value * (266 - 136) / maxValue, y, frameNum);
}

void Journal::drawPanel(const int *frames, const int *titles, int n) {
	for (int i = 0; i < _panelTextCount; ++i) {
		_vm->display()->clearTexts(_panelTextY[i], _panelTextY[i]);
	}
	_panelTextCount = 0;
	int bobNum = 1;
	int y = 8;
	while (n--) {
		showBob(bobNum++, 32, y, *frames++);
		drawPanelText(y + 12, _vm->logic()->joeResponse(*titles++));
		y += 48;
	}
}

void Journal::drawNormalPanel() {
	static const int frames[] = { FRAME_BLUE_1, FRAME_BLUE_2, FRAME_BLUE_1, FRAME_ORANGE };
	static const int titles[] = { TXT_REVIEW_ENTRY, TXT_MAKE_ENTRY, TXT_CLOSE, TXT_GIVE_UP };
	drawPanel(frames, titles, 4);
}

void Journal::drawYesNoPanel(int titleNum) {
	static const int frames[] = { FRAME_GREY, FRAME_BLUE_1, FRAME_BLUE_2 };
	const int titles[] = { titleNum, TXT_YES, TXT_NO };
	drawPanel(frames, titles, 3);

	hideBob(BOB_LEFT_RECT_4);
	hideBob(BOB_TALK_SPEED);
	hideBob(BOB_SFX_TOGGLE);
	hideBob(BOB_MUSIC_VOLUME);
	hideBob(BOB_SPEECH_TOGGLE);
	hideBob(BOB_TEXT_TOGGLE);
	hideBob(BOB_MUSIC_TOGGLE);
}

void Journal::drawConfigPanel() {
	_vm->checkOptionSettings();

	drawSlideBar(_vm->talkSpeed(), QueenEngine::MAX_TEXT_SPEED, BOB_TALK_SPEED, 164, FRAME_BLUE_PIN);
	drawSlideBar(_vm->sound()->getVolume(), Audio::Mixer::kMaxMixerVolume, BOB_MUSIC_VOLUME, 177, FRAME_GREEN_PIN);

	drawCheckBox(_vm->sound()->sfxOn(), BOB_SFX_TOGGLE, 221, 155, FRAME_CHECK_BOX);
	drawCheckBox(_vm->sound()->speechOn(), BOB_SPEECH_TOGGLE, 158, 155, FRAME_CHECK_BOX);
	drawCheckBox(_vm->subtitles(), BOB_TEXT_TOGGLE, 125, 167, FRAME_CHECK_BOX);
	drawCheckBox(_vm->sound()->musicOn(), BOB_MUSIC_TOGGLE, 125, 181, FRAME_CHECK_BOX);
}

void Journal::drawInfoPanel() {
	showBob(BOB_INFO_BOX, 72, 221, FRAME_INFO_BOX);
	const char *ver = _vm->resource()->getJASVersion();
	switch (ver[0]) {
	case 'P':
		_vm->display()->setTextCentered(132, "PC Hard Drive", false);
		break;
	case 'C':
		_vm->display()->setTextCentered(132, "PC CD-ROM", false);
		break;
	case 'a':
		_vm->display()->setTextCentered(132, "Amiga A500/600", false);
		break;
	}
	switch (ver[1]) {
	case 'E':
		_vm->display()->setTextCentered(144, "English", false);
		break;
	case 'F' :
		_vm->display()->setTextCentered(144, "Fran\x87""ais", false);
		break;
	case 'G':
		_vm->display()->setTextCentered(144, "Deutsch", false);
		break;
	case 'H':
		_vm->display()->setTextCentered(144, "Hebrew", false);
		break;
	case 'I':
		_vm->display()->setTextCentered(144, "Italiano", false);
		break;
	case 'S':
		_vm->display()->setTextCentered(144, "Espa\xA4""ol", false);
		break;
	}
	char versionId[13];
	sprintf(versionId, "Version %c.%c%c", ver[2], ver[3], ver[4]);
	_vm->display()->setTextCentered(156, versionId, false);
}

void Journal::initTextField(const char *desc) {
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);
	_textField.enabled = true;
	_textField.posCursor = _vm->display()->textWidth(desc);
	_textField.textCharsCount = strlen(desc);
	memset(_textField.text, 0, sizeof(_textField.text));
	strcpy(_textField.text, desc);
}

void Journal::updateTextField(uint16 ascii, int keycode) {
	bool dirty = false;
	switch (keycode) {
	case Common::KEYCODE_BACKSPACE:
		if (_textField.textCharsCount > 0) {
			--_textField.textCharsCount;
			_textField.text[_textField.textCharsCount] = '\0';
			dirty = true;
		}
		break;
	case Common::KEYCODE_RETURN:
	case Common::KEYCODE_KP_ENTER:
		if (_textField.text[0]) {
			closeTextField();
			int currentSlot = _currentSavePage * 10 + _currentSaveSlot;
			_vm->saveGameState(currentSlot, _textField.text);
			_quitMode = QM_CONTINUE;
		}
		break;
	default:
		if (isprint((char)ascii) &&
			_textField.textCharsCount < (sizeof(_textField.text) - 1) &&
			_vm->display()->textWidth(_textField.text) < _textField.w) {
			_textField.text[_textField.textCharsCount] = (char)ascii;
			++_textField.textCharsCount;
			dirty = true;
		}
		break;
	}
	if (dirty) {
		_vm->display()->setText(_textField.x, _textField.y + _currentSaveSlot * _textField.h, _textField.text, false);
		_textField.posCursor = _vm->display()->textWidth(_textField.text);
		update();
	}
}

void Journal::closeTextField() {
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
	_textField.enabled = false;
}

const Journal::Zone Journal::_zones[] = {
	{ ZN_REVIEW_ENTRY,  32,   8,  96,  40 },
	{ ZN_MAKE_ENTRY,    32,  56,  96,  88 }, // == ZN_YES
	{ ZN_CLOSE,         32, 104,  96, 136 }, // == ZN_NO
	{ ZN_GIVEUP,        32, 152,  96, 184 },
	{ ZN_TEXT_SPEED,   136, 169, 265, 176 },
	{ ZN_SFX_TOGGLE,   197, 155, 231, 164 },
	{ ZN_MUSIC_VOLUME, 136, 182, 265, 189 },
	{ ZN_DESC_1,       131,   7, 290,  18 },
	{ ZN_DESC_2,       131,  20, 290,  31 },
	{ ZN_DESC_3,       131,  33, 290,  44 },
	{ ZN_DESC_4,       131,  46, 290,  57 },
	{ ZN_DESC_5,       131,  59, 290,  70 },
	{ ZN_DESC_6,       131,  72, 290,  83 },
	{ ZN_DESC_7,       131,  85, 290,  96 },
	{ ZN_DESC_8,       131,  98, 290, 109 },
	{ ZN_DESC_9,       131, 111, 290, 122 },
	{ ZN_DESC_10,      131, 124, 290, 135 },
	{ ZN_PAGE_A,       300,   4, 319,  17 },
	{ ZN_PAGE_B,       300,  19, 319,  32 },
	{ ZN_PAGE_C,       300,  34, 319,  47 },
	{ ZN_PAGE_D,       300,  49, 319,  62 },
	{ ZN_PAGE_E,       300,  64, 319,  77 },
	{ ZN_PAGE_F,       300,  79, 319,  92 },
	{ ZN_PAGE_G,       300,  94, 319, 107 },
	{ ZN_PAGE_H,       300, 109, 319, 122 },
	{ ZN_PAGE_I,       300, 124, 319, 137 },
	{ ZN_PAGE_J,       300, 139, 319, 152 },
	{ ZN_INFO_BOX,     273, 146, 295, 189 },
	{ ZN_MUSIC_TOGGLE, 109, 181, 135, 190 },
	{ ZN_VOICE_TOGGLE, 134, 155, 168, 164 },
	{ ZN_TEXT_TOGGLE,  109, 168, 135, 177 }
};

} // End of namespace Queen
