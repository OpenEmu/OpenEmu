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

#include "kyra/gui_lol.h"
#include "kyra/lol.h"
#include "kyra/screen_lol.h"
#include "kyra/resource.h"
#include "kyra/util.h"
#include "kyra/sound.h"

#include "common/savefile.h"
#include "common/system.h"
#include "common/config-manager.h"

#include "graphics/scaler.h"

#include "backends/keymapper/keymapper.h"

#include "base/version.h"

namespace Kyra {

void LoLEngine::gui_drawPlayField() {
	_screen->loadBitmap("PLAYFLD.CPS", 3, 3, 0);

	if (_flagsTable[31] & 0x40) {
		// copy compass shape
		static const int cx[] = { 112, 152, 224 };
		_screen->copyRegion(cx[_flags.isTalkie ? _lang : 0], 32, 288, 0, 32, 32, 2, 2, Screen::CR_NO_P_CHECK);
		_compassDirection = -1;
	}

	if (_flagsTable[31] & 0x10)
		// draw automap book
		_screen->drawShape(2, _gameShapes[_flags.isTalkie ? 78 : 76], 290, 32, 0, 0);

	int cp = _screen->setCurPage(2);

	if (_flagsTable[31] & 0x20) {
		gui_drawScroll();
	} else {
		_selectedSpell = 0;
	}

	if (_flagsTable[31] & 0x08)
		resetLampStatus();

	updateDrawPage2();
	gui_drawScene(2);

	gui_drawAllCharPortraitsWithStats();
	gui_drawInventory();
	gui_drawMoneyBox(_screen->_curPage);

	_screen->setCurPage(cp);
	_screen->copyPage(2, 0);
	updateDrawPage2();
}

void LoLEngine::gui_drawScene(int pageNum) {
	if (!(_updateFlags & 1) && _weaponsDisabled == false && _partyAwake && _vcnBlocks)
		drawScene(pageNum);
}

void LoLEngine::gui_drawInventory() {
	if (!_currentControlMode || !_needSceneRestore) {
		for (int i = 0; i < 9; i++)
			gui_drawInventoryItem(i);
	}
}

void LoLEngine::gui_drawInventoryItem(int index) {
	static const uint16 inventoryXpos[] = { 0x6A, 0x7F, 0x94, 0xA9, 0xBE, 0xD3, 0xE8, 0xFD, 0x112 };
	int x = inventoryXpos[index];
	int item = _inventoryCurItem + index;
	if (item > 47)
		item -= 48;

	int flag = item & 1 ? 0 : 1;

	_screen->drawShape(_screen->_curPage, _gameShapes[4], x, 179, 0, flag);
	if (_inventory[item])
		_screen->drawShape(_screen->_curPage, getItemIconShapePtr(_inventory[item]), x + 1, 180, 0, 0);
}

void LoLEngine::gui_drawScroll() {
	_screen->copyRegion(112, 0, 12, 0, 87, 15, 2, 2, Screen::CR_NO_P_CHECK);
	Screen::FontId of = _screen->setFont(Screen::FID_9_FNT);
	int h = 0;

	for (int i = 0; i < 7; i++) {
		if (_availableSpells[i] != -1)
			h += 9;
	}

	if (h == 18)
		h = 27;

	if (h) {
		_screen->copyRegion(201, 1, 17, 15, 6, h, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->copyRegion(208, 1, 89, 15, 6, h, 2, 2, Screen::CR_NO_P_CHECK);
		_screen->fillRect(21, 15, 89, h + 15, _flags.use16ColorMode ? 0xbb : 206);
	}

	_screen->copyRegion(112, 16, 12, h + 15, 87, 14, 2, 2, Screen::CR_NO_P_CHECK);

	int y = 15;
	for (int i = 0; i < 7; i++) {
		if (_availableSpells[i] == -1)
			continue;
		uint8 col = (i == _selectedSpell) ? (_flags.use16ColorMode ? 0x88 : 132) : (_flags.use16ColorMode ? 0x44 : 1);
		_screen->fprintString("%s", 24, y, col, 0, 0, getLangString(_spellProperties[_availableSpells[i]].spellNameCode));
		y += 9;
	}
	_screen->setFont(of);
}

void LoLEngine::gui_highlightSelectedSpell(bool mode) {
	int y = 15;
	Screen::FontId of = _screen->setFont(Screen::FID_9_FNT);
	for (int i = 0; i < 7; i++) {
		if (_availableSpells[i] == -1)
			continue;
		uint8 col = (mode && (i == _selectedSpell)) ? (_flags.use16ColorMode ? 0x88 : 132) : (_flags.use16ColorMode ? 0x44 : 1);
		_screen->fprintString("%s", 24, y, col, 0, 0, getLangString(_spellProperties[_availableSpells[i]].spellNameCode));
		y += 9;
	}
	_screen->setFont(of);
}

void LoLEngine::gui_displayCharInventory(int charNum) {
	static const uint8 inventoryTypes[] = { 0, 1, 2, 6, 3, 1, 1, 3, 5, 4 };

	int cp = _screen->setCurPage(2);
	LoLCharacter *l = &_characters[charNum];

	int id = l->id;
	if (id < 0)
		id = -id;

	if (id != _lastCharInventory) {
		char file[13];
		sprintf(file, "invent%d.cps", inventoryTypes[id]);
		_screen->loadBitmap(file, 3, 3, 0);
		_screen->copyRegion(0, 0, 112, 0, 208, 120, 2, 6);
	} else {
		_screen->copyRegion(112, 0, 0, 0, 208, 120, 6, 2);
	}

	_screen->copyRegion(80, 143, 80, 143, 232, 35, 0, 2);
	gui_drawAllCharPortraitsWithStats();

	if (_flags.use16ColorMode)
		_screen->fprintString("%s", 156, 8, 0xe1, 0, 1, l->name);
	else
		_screen->fprintString("%s", 157, 9, 254, 0, 5, l->name);

	gui_printCharInventoryStats(charNum);

	for (int i = 0; i < 11; i++)
		gui_drawCharInventoryItem(i);

	Screen::FontId of = _screen->setFont(Screen::FID_9_FNT);
	_screen->fprintString("%s", 182, 103, _flags.use16ColorMode ? 0xbb : 172, 0, 5, getLangString(0x4033));
	_screen->setFont(of);

	static const uint16 statusFlags[] = { 0x0080, 0x0000, 0x1000, 0x0002, 0x100, 0x0001, 0x0000, 0x0000 };

	memset(_charStatusFlags, 0xFF, sizeof(_charStatusFlags));
	int x = 0;
	int32 c = 0;

	for (int i = 0; i < 3; i++) {
		if (!(l->flags & statusFlags[i << 1]))
			continue;

		uint8 *shp = _gameShapes[statusFlags[(i << 1) + 1]];
		_screen->drawShape(_screen->_curPage, shp, 108 + x, 98, 0, 0);
		x += (shp[3] + 2);
		_charStatusFlags[c] = statusFlags[(i << 1) + 1];
		c++;
	}

	for (int i = 0; i < 3; i++) {
		int32 b = l->experiencePts[i] - _expRequirements[l->skillLevels[i] - 1];
		int32 e = _expRequirements[l->skillLevels[i]] - _expRequirements[l->skillLevels[i] - 1];

		while (e & 0xffff8000) {
			e >>= 1;
			c = b;
			b >>= 1;

			if (c && !b)
				b = 1;
		}

		if (_flags.use16ColorMode)
			gui_drawHorizontalBarGraph(154, 66 + i * 8, 34, 5, b, e, 0x88, 0);
		else
			gui_drawHorizontalBarGraph(154, 64 + i * 10, 34, 5, b, e, 132, 0);
	}

	_screen->drawClippedLine(14, 120, 194, 120, 1);
	_screen->copyRegion(0, 0, 112, 0, 208, 121, 2, 0);
	_screen->copyRegion(80, 143, 80, 143, 232, 35, 2, 0);

	_screen->setCurPage(cp);
}

void LoLEngine::gui_printCharInventoryStats(int charNum) {
	for (int i = 0; i < 5; i++)
		gui_printCharacterStats(i, 1, calculateCharacterStats(charNum, i));

	_charInventoryUnk |= (1 << charNum);
}

void LoLEngine::gui_printCharacterStats(int index, int redraw, int value) {
	uint32 offs = _screen->_curPage ? 0 : 112;
	int y = 0;
	int col = 0;

	if (index < 2) {
		// might
		// protection
		if (_flags.use16ColorMode) {
			y = (index + 2) << 3;
			col = 0xa1;
			if (redraw)
				_screen->fprintString("%s", offs + 108, y, col, 0, 0, getLangString(0x4014 + index));
		} else {
			y = index * 10 + 22;
			col = 158;
			if (redraw)
				_screen->fprintString("%s", offs + 108, y, col, 0, 4, getLangString(0x4014 + index));
		}
	} else {
		//skills
		int s = index - 2;
		y = s * 10 + 62;
		if (_flags.use16ColorMode) {
			y = (s + 8) << 3;
			col = _characters[_selectedCharacter].flags & (0x200 << s) ? 0xe1 : 0x81;
			if (redraw)
				_screen->fprintString("%s", offs + 108, y, col, 0, 0, getLangString(0x4014 + index));
		} else {
			y = s * 10 + 62;
			col = _characters[_selectedCharacter].flags & (0x200 << s) ? 254 : 180;
			if (redraw)
				_screen->fprintString("%s", offs + 108, y, col, 0, 4, getLangString(0x4014 + index));
		}
	}

	if (offs)
		_screen->copyRegion(294, y, 182 + offs, y, 18, 8, 6, _screen->_curPage, Screen::CR_NO_P_CHECK);

	Screen::FontId of = _flags.use16ColorMode ? _screen->setFont(Screen::FID_SJIS_FNT) : _screen->_currentFont;
	_screen->fprintString("%d", 200 + offs, y, col, 0, _flags.use16ColorMode ? 2 : 6, value);
	_screen->setFont(of);
}

void LoLEngine::gui_changeCharacterStats(int charNum) {
	int tmp[5];
	int inc[5];
	bool prc = false;

	for (int i = 0; i < 5; i++) {
		tmp[i] = calculateCharacterStats(charNum, i);
		int diff = tmp[i] - _charStatsTemp[i];
		inc[i] = diff / 15;

		if (diff) {
			prc = true;
			if (!inc[i])
				inc[i] = (diff < 0) ? -1 : 1;
		}
	}

	if (!prc)
		return;

	do {
		prc = false;

		for (int i = 0; i < 5; i++) {
			if (tmp[i] == _charStatsTemp[i])
				continue;

			_charStatsTemp[i] += inc[i];

			if ((inc[i] > 0 && tmp[i] < _charStatsTemp[i]) || (inc[i] < 0 && tmp[i] > _charStatsTemp[i]))
				_charStatsTemp[i] = tmp[i];

			gui_printCharacterStats(i, 0, _charStatsTemp[i]);
			prc = true;
		}

		delay(_tickLength, true);

	} while (prc);
}

void LoLEngine::gui_drawCharInventoryItem(int itemIndex) {
	static const uint8 slotShapes[] = { 0x30, 0x34, 0x30, 0x34, 0x2E, 0x2F, 0x32, 0x33, 0x31, 0x35, 0x35 };

	const uint8 *coords = &_charInvDefs[_charInvIndex[_characters[_selectedCharacter].raceClassSex] * 22 + itemIndex * 2];
	uint8 x = *coords++;
	uint8 y = *coords;

	if (y == 0xff)
		return;

	if (!_screen->_curPage)
		x += 112;

	int i = _characters[_selectedCharacter].items[itemIndex];
	int shapeNum = i ? ((itemIndex < 9) ? 4 : 5) : (_flags.isTalkie ? slotShapes[itemIndex] : slotShapes[itemIndex] - 2);
	_screen->drawShape(_screen->_curPage, _gameShapes[shapeNum], x, y, 0, 0);

	if (itemIndex > 8) {
		x -= 5;
		y -= 5;
	}

	if (i)
		_screen->drawShape(_screen->_curPage, getItemIconShapePtr(i), x + 1, y + 1, 0, 0);
}

void LoLEngine::gui_drawAllCharPortraitsWithStats() {
	int numChars = countActiveCharacters();
	if (!numChars)
		return;

	for (int i = 0; i < numChars; i++)
		gui_drawCharPortraitWithStats(i);
}

void LoLEngine::gui_drawCharPortraitWithStats(int charNum) {
	if (!(_characters[charNum].flags & 1) || _updateFlags & 2)
		return;

	Screen::FontId tmpFid = _screen->setFont(Screen::FID_6_FNT);
	int cp = _screen->setCurPage(6);

	gui_drawBox(0, 0, 66, 34, 1, 1, -1);
	gui_drawCharFaceShape(charNum, 0, 1, _screen->_curPage);

	if (_flags.use16ColorMode) {
		gui_drawLiveMagicBar(33, 32, _characters[charNum].magicPointsCur, 0, _characters[charNum].magicPointsMax, 5, 32, 0xaa, 0x44, 0);
		gui_drawLiveMagicBar(39, 32, _characters[charNum].hitPointsCur, 0, _characters[charNum].hitPointsMax, 5, 32, 0x66, 0x44, 1);
		_screen->printText(getLangString(0x4253), 33, 1, 0x99, 0);
		_screen->printText(getLangString(0x4254), 39, 1, 0x55, 0);
	} else {
		gui_drawLiveMagicBar(33, 32, _characters[charNum].magicPointsCur, 0, _characters[charNum].magicPointsMax, 5, 32, 162, 1, 0);
		gui_drawLiveMagicBar(39, 32, _characters[charNum].hitPointsCur, 0, _characters[charNum].hitPointsMax, 5, 32, 154, 1, 1);
		_screen->printText((_flags.platform == Common::kPlatformPC && !_flags.isTalkie) ? "M" : getLangString(0x4253), 33, 1, 160, 0);
		_screen->printText((_flags.platform == Common::kPlatformPC && !_flags.isTalkie) ? "H" : getLangString(0x4254), 39, 1, 152, 0);
	}

	int spellLevels = 0;
	if (_availableSpells[_selectedSpell] != -1) {
		for (int i = 0; i < 4; i++) {
			if (_spellProperties[_availableSpells[_selectedSpell]].mpRequired[i] <= _characters[charNum].magicPointsCur
			    && _spellProperties[_availableSpells[_selectedSpell]].hpRequired[i] <= _characters[charNum].hitPointsCur)
				spellLevels++;
		}
	}

	if (_characters[charNum].flags & 0x10) {
		// magic submenu open
		_screen->drawShape(_screen->_curPage, _gameShapes[_flags.isTalkie ? 73 : 71], 44, 0, 0, 0);
		if (spellLevels < 4)
			_screen->drawGridBox(44, (spellLevels << 3) + 1, 22, 32 - (spellLevels << 3), 1);
	} else {
		// magic submenu closed
		int handIndex = 0;
		if (_characters[charNum].items[0]) {
			if (_itemProperties[_itemsInPlay[_characters[charNum].items[0]].itemPropertyIndex].might != -1)
				handIndex = _itemsInPlay[_characters[charNum].items[0]].itemPropertyIndex;
		}

		handIndex =  _gameShapeMap[(_itemProperties[handIndex].shpIndex << 1) + 1];
		if (handIndex == _gameShapeMap[1]) { // draw raceClassSex specific hand shape
			handIndex = _characters[charNum].raceClassSex - 1;
			if (handIndex < 0)
				handIndex = 0;
			handIndex += (_flags.isTalkie ? 68 : 66);
		}

		// draw hand/weapon
		_screen->drawShape(_screen->_curPage, _gameShapes[handIndex], 44, 0, 0, 0);
		// draw magic symbol
		_screen->drawShape(_screen->_curPage, _gameShapes[(_flags.isTalkie ? 72 : 70) + _characters[charNum].field_41], 44, 17, 0, 0);

		if (spellLevels == 0)
			_screen->drawGridBox(44, 17, 22, 16, 1);
	}

	uint16 f = _characters[charNum].flags & 0x314C;
	if ((f == 0 && _weaponsDisabled) || (f && (f != 4 || _characters[charNum].weaponHit == 0 || (_characters[charNum].weaponHit && _weaponsDisabled))))
		_screen->drawGridBox(44, 0, 22, 34, 1);

	if (_characters[charNum].weaponHit) {
		_screen->drawShape(_screen->_curPage, _gameShapes[_flags.isTalkie ? 34 : 32], 44, 0, 0, 0);
		_screen->fprintString("%d", 57, 7, _flags.use16ColorMode ? 0x33 : 254, 0, 1, _characters[charNum].weaponHit);
	}
	if (_characters[charNum].damageSuffered)
		_screen->fprintString("%d", 17, 28, _flags.use16ColorMode ? 0x33 : 254, 0, 1, _characters[charNum].damageSuffered);

	uint8 col = (charNum != _selectedCharacter || countActiveCharacters() == 1) ? 1 : 212;
	if (_flags.use16ColorMode)
		col = (charNum != _selectedCharacter || countActiveCharacters() == 1) ? 0x44 : 0x22;
	_screen->drawBox(0, 0, 65, 33, col);

	_screen->copyRegion(0, 0, _activeCharsXpos[charNum], 143, 66, 34, _screen->_curPage, cp, Screen::CR_NO_P_CHECK);

	_screen->setCurPage(cp);
	_screen->setFont(tmpFid);
}

void LoLEngine::gui_drawCharFaceShape(int charNum, int x, int y, int pageNum) {
	if (_characters[charNum].curFaceFrame < 7 && _characters[charNum].tempFaceFrame)
		_characters[charNum].curFaceFrame = _characters[charNum].tempFaceFrame;

	if (_characters[charNum].tempFaceFrame == 0 && _characters[charNum].curFaceFrame > 1 && _characters[charNum].curFaceFrame < 7)
		_characters[charNum].curFaceFrame = _characters[charNum].tempFaceFrame;

	int frm = (_characters[charNum].flags & 0x1108 && _characters[charNum].curFaceFrame < 7) ? 1 : _characters[charNum].curFaceFrame;

	if (_characters[charNum].hitPointsCur <= (_characters[charNum].hitPointsMax >> 1))
		frm += 14;

	if (!pageNum)
		_screen->hideMouse();

	_screen->drawShape(pageNum, _characterFaceShapes[frm][charNum], x, y, 0, 0x100, _screen->_paletteOverlay2, (_characters[charNum].flags & 0x80 ? 1 : 0));

	if (_characters[charNum].flags & 0x40)
		// draw spider web
		_screen->drawShape(pageNum, _gameShapes[21], x, y, 0, 0);

	if (!pageNum)
		_screen->showMouse();
}

void LoLEngine::gui_highlightPortraitFrame(int charNum) {
	if (charNum != _selectedCharacter) {
		int o = _selectedCharacter;
		_selectedCharacter = charNum;
		gui_drawCharPortraitWithStats(o);
	}

	gui_drawCharPortraitWithStats(charNum);
}

void LoLEngine::gui_drawLiveMagicBar(int x, int y, int curPoints, int unk, int maxPoints, int w, int h, int col1, int col2, int flag) {
	w--;
	h--;

	if (maxPoints < 1)
		return;

	int t = (curPoints < 1) ? 0 : curPoints;
	curPoints = (maxPoints < t) ? maxPoints : t;

	int barHeight = (curPoints * h) / maxPoints;

	if (barHeight < 1 && curPoints > 0)
		barHeight = 1;

	_screen->drawClippedLine(x - 1, y - h, x - 1, y, _flags.use16ColorMode ? 0x44 : 1);

	if (flag) {
		t = maxPoints >> 1;
		if (t > curPoints)
			col1 = _flags.use16ColorMode ? 0xbb : 144;
		t = maxPoints >> 2;
		if (t > curPoints)
			col1 = _flags.use16ColorMode ? 0x88 : 132;
	}

	if (barHeight > 0)
		_screen->fillRect(x, y - barHeight, x + w, y, col1);

	if (barHeight < h)
		_screen->fillRect(x, y - h, x + w, y - barHeight, col2);

	if (unk > 0 && unk < maxPoints)
		_screen->drawBox(x, y - barHeight, x + w, y, col1 - 2);
}

void LoLEngine::calcCharPortraitXpos() {
	int nc = countActiveCharacters();

	if (_currentControlMode && !textEnabled()) {
		int t = (280 - (nc * 33)) / (nc + 1);
		for (int i = 0; i < nc; i++)
			_activeCharsXpos[i] = i * 33 + t * (i + 1) + 10;

	} else {
		int t = (235 - (nc * 66)) / (nc + 1);
		for (int i = 0; i < nc; i++)
			_activeCharsXpos[i] = i * 66 + t * (i + 1) + 83;
	}
}

void LoLEngine::gui_drawMoneyBox(int pageNum) {
	static const uint16 moneyX256[] = { 0x128, 0x134, 0x12b, 0x131, 0x12e};
	static const uint16 moneyY256[] = { 0x73, 0x73, 0x74, 0x74, 0x75};
	static const uint16 moneyX16[] = { 0x127, 0x133, 0x12a, 0x130, 0x12d};
	static const uint16 moneyY16[] = { 0x74, 0x74, 0x75, 0x75, 0x76};

	int backupPage = _screen->_curPage;
	_screen->_curPage = pageNum;

	const uint16 *moneyX;
	const uint16 *moneyY;

	if (_flags.use16ColorMode) {
		moneyX = moneyX16;
		moneyY = moneyY16;
		_screen->fillRect(291, 98, 315, 118, 0x11, pageNum);
	} else {
		moneyX = moneyX256;
		moneyY = moneyY256;
		_screen->fillRect(292, 97, 316, 118, 252, pageNum);
	}

	for (int i = 0; i < 5; i++) {
		if (!_moneyColumnHeight[i])
			continue;

		uint8 h = _moneyColumnHeight[i] - 1;
		_screen->drawClippedLine(moneyX[i], moneyY[i], moneyX[i], moneyY[i] - h, _flags.use16ColorMode ? 1 : 0xd2);
		_screen->drawClippedLine(moneyX[i] + 1, moneyY[i], moneyX[i] + 1, moneyY[i] - h, _flags.use16ColorMode ? 2 : 0xd1);
		_screen->drawClippedLine(moneyX[i] + 2, moneyY[i], moneyX[i] + 2, moneyY[i] - h, _flags.use16ColorMode ? 3 : 0xd0);
		_screen->drawClippedLine(moneyX[i] + 3, moneyY[i], moneyX[i] + 3, moneyY[i] - h, _flags.use16ColorMode ? 2 : 0xd1);
		_screen->drawClippedLine(moneyX[i] + 4, moneyY[i], moneyX[i] + 4, moneyY[i] - h, _flags.use16ColorMode ? 1 : 0xd2);
	}

	Screen::FontId backupFont = _screen->setFont(Screen::FID_6_FNT);
	if (_flags.use16ColorMode)
		_screen->fprintString("%d", 304, 99, 0x33, 0, 1, _credits);
	else
		_screen->fprintString("%d", 305, 98, 254, 0, 1, _credits);

	_screen->setFont(backupFont);
	_screen->_curPage = backupPage;

	if (pageNum == 6) {
		if (_flags.use16ColorMode)
			_screen->copyRegion(291, 98, 291, 98, 24, 20, 6, 0);
		else
			_screen->copyRegion(292, 97, 292, 97, 25, 22, 6, 0);
	}
}

void LoLEngine::gui_drawCompass() {
	if (!(_flagsTable[31] & 0x40))
		return;

	if (_compassDirection == -1) {
		_compassDirectionIndex = -1;
		_compassDirection = _currentDirection << 6;
	}

	int t = ((_compassDirection + 4) >> 3) & 0x1f;

	if (t == _compassDirectionIndex)
		return;

	_compassDirectionIndex = t;

	if (!_screen->_curPage)
		_screen->hideMouse();

	const CompassDef *c = &_compassDefs[t];

	int compassShp = 22;
	int compassPtr = 23;
	if (_flags.isTalkie) {
		compassShp += _lang;
		compassPtr = 25;
	}

	_screen->drawShape(_screen->_curPage, _gameShapes[compassShp], 294, 3, 0, 0);
	_screen->drawShape(_screen->_curPage, _gameShapes[compassPtr + c->shapeIndex], 298 + c->x, c->y + 9, 0, c->flags | 0x300, _screen->_paletteOverlay1, 1);
	_screen->drawShape(_screen->_curPage, _gameShapes[compassPtr + c->shapeIndex], 299 + c->x, c->y + 8, 0, c->flags);

	if (!_screen->_curPage)
		_screen->showMouse();
}

int LoLEngine::gui_enableControls() {
	_floatingCursorControl = 0;

	int start = 74;
	int end = 83;

	if (_flags.isTalkie) {
		start = 76;
		end = 85;
	}

	if (!_currentControlMode) {
		for (int i = start; i < end; i++)
			gui_toggleButtonDisplayMode(i, 2);
	}

	gui_toggleFightButtons(false);
	return 1;
}

int LoLEngine::gui_disableControls(int controlMode) {
	if (_currentControlMode)
		return 0;

	_floatingCursorControl = (controlMode & 2) ? 2 : 1;

	gui_toggleFightButtons(true);

	int start = 74;
	int end = 83;
	int swtch = 76;

	if (_flags.isTalkie) {
		start = 76;
		end = 85;
		swtch = 78;
	}

	for (int i = start; i < end; i++)
		gui_toggleButtonDisplayMode(i, ((controlMode & 2) && (i > swtch)) ? 2 : 3);

	return 1;
}

void LoLEngine::gui_toggleButtonDisplayMode(int shapeIndex, int mode) {
	static const int16 buttonX[] = { 0x0056, 0x0128, 0x000C, 0x0021, 0x0122, 0x000C, 0x0021, 0x0036, 0x000C, 0x0021, 0x0036 };
	static const int16 buttonY[] = { 0x00B4, 0x00B4, 0x00B4, 0x00B4, 0x0020, 0x0084, 0x0084, 0x0084, 0x0096, 0x0096, 0x0096 };

	int swtch = 76;
	int subst = 72;

	if (_flags.isTalkie) {
		swtch = 78;
		subst = 74;
	}

	if (shapeIndex == swtch && !(_flagsTable[31] & 0x10))
		return;

	if (_currentControlMode && _needSceneRestore)
		return;

	if (mode == 0)
		shapeIndex = _lastButtonShape;

	int pageNum = 0;

	int16 x1 = buttonX[shapeIndex - subst];
	int16 y1 = buttonY[shapeIndex - subst];
	int16 x2 = 0;
	int16 y2 = 0;
	uint32 t = 0;

	switch (mode) {
	case 1:
		mode = 0x100;
		_lastButtonShape = shapeIndex;
		break;

	case 0:
		if (!_lastButtonShape)
			return;

		t = _system->getMillis();
		if (_buttonPressTimer > t)
			delay(_buttonPressTimer - t);

	case 2:
		mode = 0;
		_lastButtonShape = 0;
		break;

	case 3:
		mode = 0;
		_lastButtonShape = 0;
		pageNum = 6;

		x2 = x1;
		y2 = y1;
		x1 = 0;
		y1 = 0;
		break;

	default:
		break;
	}

	_screen->drawShape(pageNum, _gameShapes[shapeIndex], x1, y1, 0, mode, _screen->_paletteOverlay1, 1);

	if (!pageNum)
		_screen->updateScreen();

	if (pageNum == 6) {
		int cp = _screen->setCurPage(6);

		_screen->drawGridBox(x1, y1, _gameShapes[shapeIndex][3], _gameShapes[shapeIndex][2], 1);
		_screen->copyRegion(x1, y1, x2, y2, _gameShapes[shapeIndex][3], _gameShapes[shapeIndex][2], pageNum, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();

		_screen->setCurPage(cp);
	}

	_buttonPressTimer = _system->getMillis() + 6 * _tickLength;
}

void LoLEngine::gui_toggleFightButtons(bool disable) {
	for (int i = 0; i < 3; i++) {
		if (!(_characters[i].flags & 1))
			continue;

		if (disable)
			_characters[i].flags |= 0x2000;
		else
			_characters[i].flags &= 0xdfff;

		if (disable && !textEnabled()) {
			int u = _selectedCharacter;
			_selectedCharacter = 99;
			int f = _updateFlags;
			_updateFlags &= 0xfffd;

			gui_drawCharPortraitWithStats(i);

			_updateFlags = f;
			_selectedCharacter = u;
		} else {
			gui_drawCharPortraitWithStats(i);
		}
	}
}

void LoLEngine::gui_updateInput() {
	// TODO: We need to catch all cases where loading is not possible and
	// set the "mainLoop" parameter to false for them.
	int inputFlag = checkInput(_activeButtons, !((_updateFlags & 3) || _weaponsDisabled), 0);

	if (_preserveEvents)
		_preserveEvents = false;
	else
		removeInputTop();

	if (inputFlag && _activeMagicMenu != -1 && !(inputFlag & 0x8800)) {
		gui_enableDefaultPlayfieldButtons();
		_characters[_activeMagicMenu].flags &= 0xffef;
		gui_drawCharPortraitWithStats(_activeMagicMenu);
		gui_triggerEvent(inputFlag);
		_preserveEvents = false;
		_activeMagicMenu = -1;
		inputFlag = 0;
	}

	if (inputFlag == _keyMap[Common::KEYCODE_SPACE] || inputFlag == _keyMap[Common::KEYCODE_RETURN]) {
		snd_stopSpeech(true);
	} else if (inputFlag == _keyMap[Common::KEYCODE_SLASH]) {
		if (_weaponsDisabled || _availableSpells[1] == -1)
			return;

		gui_highlightSelectedSpell(false);
		if (_availableSpells[++_selectedSpell] == -1)
			_selectedSpell = 0;
		gui_highlightSelectedSpell(true);
		gui_drawAllCharPortraitsWithStats();
	}
}

void LoLEngine::gui_triggerEvent(int eventType) {
	Common::Event evt;
	memset(&evt, 0, sizeof(Common::Event));
	evt.mouse.x = _mouseX;
	evt.mouse.y = _mouseY;

	if (eventType == 65 || eventType == 199) {
		evt.type = Common::EVENT_LBUTTONDOWN;
	} else if (eventType == 66 || eventType == 201) {
		evt.type = Common::EVENT_RBUTTONDOWN;
	} else {
		evt.type = Common::EVENT_KEYDOWN;

		for (KeyMap::const_iterator c = _keyMap.begin(); c != _keyMap.end(); ++c) {
			if (c->_value == eventType)
				evt.kbd.keycode = (Common::KeyCode)c->_key;
		}
	}

	removeInputTop();
	_eventList.push_back(Event(evt, true));
	_preserveEvents = true;
}

void LoLEngine::gui_enableDefaultPlayfieldButtons() {
	gui_resetButtonList();
	gui_initButtonsFromList(_buttonList1);
	gui_setFaceFramesControlButtons(7, 44);
	gui_setFaceFramesControlButtons(11, 44);
	gui_setFaceFramesControlButtons(17, 0);
	gui_setFaceFramesControlButtons(29, 0);
	gui_setFaceFramesControlButtons(25, 33);

	if (_flagsTable[31] & 0x20)
		gui_initMagicScrollButtons();
}

void LoLEngine::gui_enableSequenceButtons(int x, int y, int w, int h, int enableFlags) {
	gui_resetButtonList();

	_sceneWindowButton.x = x;
	_sceneWindowButton.y = y;
	_sceneWindowButton.w = w;
	_sceneWindowButton.h = h;

	gui_initButtonsFromList(_buttonList3);

	if (enableFlags & 1)
		gui_initButtonsFromList(_buttonList4);

	if (enableFlags & 2)
		gui_initButtonsFromList(_buttonList5);
}

void LoLEngine::gui_specialSceneRestoreButtons() {
	if (!_spsWindowW && !_spsWindowH)
		return;

	gui_enableDefaultPlayfieldButtons();
	_spsWindowX = _spsWindowY = _spsWindowW = _spsWindowH = _seqTrigger = 0;
}

void LoLEngine::gui_enableCharInventoryButtons(int charNum) {
	gui_resetButtonList();
	gui_initButtonsFromList(_buttonList2);
	gui_initCharInventorySpecialButtons(charNum);
	gui_setFaceFramesControlButtons(21, 0);
}

void LoLEngine::gui_setFaceFramesControlButtons(int index, int xOffs) {
	int c = countActiveCharacters();
	for (int i = 0; i < c; i++)
		gui_initButton(index + i, _activeCharsXpos[i] + xOffs);
}

void LoLEngine::gui_initCharInventorySpecialButtons(int charNum) {
	const uint8 *s = &_charInvDefs[_charInvIndex[_characters[charNum].raceClassSex] * 22];

	for (int i = 0; i < 11; i++) {
		if (*s != 0xff)
			gui_initButton(33 + i, s[0], s[1], i);
		s += 2;
	}
}

void LoLEngine::gui_initMagicScrollButtons() {
	for (int i = 0; i < 7; i++) {
		if (_availableSpells[i] == -1)
			continue;
		gui_initButton(71 + i, -1, -1, i);
	}
}

void LoLEngine::gui_initMagicSubmenu(int charNum) {
	gui_resetButtonList();
	_subMenuIndex = charNum;
	gui_initButtonsFromList(_buttonList7);
}

void LoLEngine::gui_initButton(int index, int x, int y, int val) {
	Button *b = 0;
	int cnt = 1;

	if (_activeButtons) {
		Button *n = _activeButtons;
		while (n->nextButton) {
			++cnt;
			n = n->nextButton;
		}

		++cnt;
		b = n->nextButton = &_activeButtonData[cnt];
	} else {
		b = &_activeButtonData[0];
		_activeButtons = b;
	}

	*b = Button();

	b->nextButton = 0;
	b->data0Val2 = b->data1Val2 = b->data2Val2 = 0xfe;
	b->data0Val3 = b->data1Val3 = b->data2Val3 = 0x01;

	b->index = cnt;
	b->keyCode = _buttonData[index].keyCode;
	b->keyCode2 = _buttonData[index].keyCode2;
	b->dimTableIndex = _buttonData[index].screenDim;
	b->flags = _buttonData[index].buttonflags;

	b->arg = (val != -1) ? (uint8)(val & 0xff) : _buttonData[index].index;

	if (index == 15) {
		// magic sub menu
		b->x = _activeCharsXpos[_subMenuIndex] + 44;
		b->arg = _subMenuIndex;
		b->y = _buttonData[index].y;
		b->width = _buttonData[index].w - 1;
		b->height = _buttonData[index].h - 1;
	} else if (index == 64) {
		// scene window button
		b->x = _sceneWindowButton.x;
		b->y = _sceneWindowButton.y;
		b->width = _sceneWindowButton.w - 1;
		b->height = _sceneWindowButton.h - 1;
	} else {
		b->x = x != -1 ? x : _buttonData[index].x;
		b->y = y != -1 ? y : _buttonData[index].y;
		b->width = _buttonData[index].w - 1;
		b->height = _buttonData[index].h - 1;
	}

	b->buttonCallback = _buttonCallbacks[index];
}

int LoLEngine::clickedUpArrow(Button *button) {
	if (button->arg && !_floatingCursorsEnabled)
		return 0;

	moveParty(_currentDirection, ((button->flags2 & 0x1080) == 0x1080) ? 1 : 0, 0, _flags.isTalkie ? 80 : 78);

	return 1;
}

int LoLEngine::clickedDownArrow(Button *button) {
	if (button->arg && !_floatingCursorsEnabled)
		return 0;

	moveParty(_currentDirection ^ 2, 0, 1, _flags.isTalkie ? 83 : 81);

	return 1;
}

int LoLEngine::clickedLeftArrow(Button *button) {
	if (button->arg && !_floatingCursorsEnabled)
		return 0;

	moveParty((_currentDirection - 1) & 3, ((button->flags2 & 0x1080) == 0x1080) ? 1 : 0, 2, _flags.isTalkie ? 82 : 80);

	return 1;
}

int LoLEngine::clickedRightArrow(Button *button) {
	if (button->arg && !_floatingCursorsEnabled)
		return 0;

	moveParty((_currentDirection + 1) & 3, ((button->flags2 & 0x1080) == 0x1080) ? 1 : 0, 3, _flags.isTalkie ? 84 : 82);

	return 1;
}

int LoLEngine::clickedTurnLeftArrow(Button *button) {
	if (button->arg && !_floatingCursorsEnabled)
		return 0;

	gui_toggleButtonDisplayMode(_flags.isTalkie ? 79 : 77, 1);
	_currentDirection = (_currentDirection - 1) & 3;

	_sceneDefaultUpdate = 1;

	runLevelScript(_currentBlock, 0x4000);
	initTextFading(2, 0);

	if (!_sceneDefaultUpdate)
		gui_drawScene(0);
	else
		movePartySmoothScrollTurnLeft(1);

	gui_toggleButtonDisplayMode(_flags.isTalkie ? 79 : 77, 0);
	runLevelScript(_currentBlock, 0x10);
	return 1;
}

int LoLEngine::clickedTurnRightArrow(Button *button) {
	if (button->arg && !_floatingCursorsEnabled)
		return 0;

	gui_toggleButtonDisplayMode(_flags.isTalkie ? 81 : 79, 1);
	_currentDirection = (_currentDirection + 1) & 3;

	_sceneDefaultUpdate = 1;

	runLevelScript(_currentBlock, 0x4000);
	initTextFading(2, 0);

	if (!_sceneDefaultUpdate)
		gui_drawScene(0);
	else
		movePartySmoothScrollTurnRight(1);

	gui_toggleButtonDisplayMode(_flags.isTalkie ? 81 : 79, 0);
	runLevelScript(_currentBlock, 0x10);

	return 1;
}

int LoLEngine::clickedAttackButton(Button *button) {
	int c = button->arg;

	if (_characters[c].flags & 0x314C)
		return 1;

	int bl = calcNewBlockPosition(_currentBlock, _currentDirection);

	if (_levelBlockProperties[bl].flags & 0x10) {
		breakIceWall(0, 0);
		return 1;
	}

	uint16 target = getNearestMonsterFromCharacter(c);
	int s = 0;

	for (int i = 0; i < 4; i++) {
		if (!_characters[c].items[i])
			continue;

		runItemScript(c, _characters[c].items[i], 0x400, target, s);
		runLevelScriptCustom(_currentBlock, 0x400, c, _characters[c].items[i], target, s);
		s -= 10;
	}

	if (!s) {
		runItemScript(c, 0, 0x400, target, s);
		runLevelScriptCustom(_currentBlock, 0x400, c, 0, target, s);
	}

	s = _characters[c].weaponHit ? 4 : calcMonsterSkillLevel(c, 8) + 4;

	// check for Zephyr ring
	if (itemEquipped(c, 230))
		s >>= 1;

	_characters[c].flags |= 4;
	gui_highlightPortraitFrame(c);

	setCharacterUpdateEvent(c, 1, s, 1);

	return 1;
}

int LoLEngine::clickedMagicButton(Button *button) {
	int c = button->arg;

	if (_characters[c].flags & 0x314C)
		return 1;

	if (checkMagic(c, _availableSpells[_selectedSpell], 0))
		return 1;

	_characters[c].flags ^= 0x10;

	gui_drawCharPortraitWithStats(c);
	gui_initMagicSubmenu(c);
	_activeMagicMenu = c;

	return 1;
}

int LoLEngine::clickedMagicSubmenu(Button *button) {
	int spellLevel = (_mouseY - 144) >> 3;
	int c = button->arg;

	gui_enableDefaultPlayfieldButtons();

	if (checkMagic(c, _availableSpells[_selectedSpell], spellLevel)) {
		_characters[c].flags &= 0xffef;
		gui_drawCharPortraitWithStats(c);
	} else {
		_characters[c].flags |= 4;
		_characters[c].flags &= 0xffef;

		if (castSpell(c, _availableSpells[_selectedSpell], spellLevel)) {
			setCharacterUpdateEvent(c, 1, 8, 1);
			increaseExperience(c, 2, spellLevel * spellLevel);
		} else {
			_characters[c].flags &= 0xfffb;
			gui_drawCharPortraitWithStats(c);
		}
	}

	_activeMagicMenu = -1;
	return 1;
}

int LoLEngine::clickedScreen(Button *button) {
	_characters[_activeMagicMenu].flags &= 0xffef;
	gui_drawCharPortraitWithStats(_activeMagicMenu);
	_activeMagicMenu = -1;

	if (!(button->flags2 & 0x80)) {
		if (button->flags2 & 0x100)
			gui_triggerEvent(65);
		else
			gui_triggerEvent(66);
	}

	gui_enableDefaultPlayfieldButtons();

	return 1;
}

int LoLEngine::clickedPortraitLeft(Button *button) {
	disableSysTimer(2);

	if (!_weaponsDisabled) {
		_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer2);
		_screen->copyPage(0, 2);
		_screen->copyRegionToBuffer(2, 0, 0, 320, 200, _pageBuffer1);
		_updateFlags |= 0x0C;
		gui_disableControls(1);
	}

	_selectedCharacter = button->arg;
	_weaponsDisabled = true;

	if (_flags.use16ColorMode)
		_screen->fillRect(112, 0, 288, 120, 0, 2);

	gui_displayCharInventory(_selectedCharacter);
	gui_enableCharInventoryButtons(_selectedCharacter);

	return 1;
}

int LoLEngine::clickedLiveMagicBarsLeft(Button *button) {
	gui_highlightPortraitFrame(button->arg);
	_txt->printMessage(0, getLangString(0x4047), _characters[button->arg].name, _characters[button->arg].hitPointsCur,
	                   _characters[button->arg].hitPointsMax, _characters[button->arg].magicPointsCur, _characters[button->arg].magicPointsMax);
	return 1;
}

int LoLEngine::clickedPortraitEtcRight(Button *button) {
	if (!_itemInHand)
		return 1;

	int flg = _itemProperties[_itemsInPlay[_itemInHand].itemPropertyIndex].flags;
	int c = button->arg;

	if (flg & 1) {
		if (!(_characters[c].flags & 8) || (flg & 0x20)) {
			runItemScript(c, _itemInHand, 0x400, 0, 0);
			runLevelScriptCustom(_currentBlock, 0x400, c, _itemInHand, 0, 0);
		} else {
			_txt->printMessage(2, getLangString(0x402c), _characters[c].name);
		}
		return 1;
	}

	_txt->printMessage(2, "%s", getLangString((flg & 8) ? 0x4029 : ((flg & 0x10) ? 0x402a : 0x402b)));
	return 1;
}

int LoLEngine::clickedCharInventorySlot(Button *button) {
	if (_itemInHand) {
		uint16 sl = 1 << button->arg;
		int type = _itemProperties[_itemsInPlay[_itemInHand].itemPropertyIndex].type;
		if (!(sl & type)) {
			bool f = false;

			for (int i = 0; i < 11; i++) {
				if (!(type & (1 << i)))
					continue;

				_txt->printMessage(0, getLangString(i > 3 ? 0x418A : 0x418B), getLangString(_itemProperties[_itemsInPlay[_itemInHand].itemPropertyIndex].nameStringId), getLangString(_inventorySlotDesc[i]));
				f = true;
			}

			if (!f)
				_txt->printMessage(_itemsInPlay[_itemInHand].itemPropertyIndex == 231 ? 2 : 0, "%s", getLangString(0x418C));

			return 1;
		}
	} else {
		if (!_characters[_selectedCharacter].items[button->arg]) {
			_txt->printMessage(0, "%s", getLangString(_inventorySlotDesc[button->arg] + 8));
			return 1;
		}
	}

	int ih = _itemInHand;

	setHandItem(_characters[_selectedCharacter].items[button->arg]);
	_characters[_selectedCharacter].items[button->arg] = ih;
	gui_drawCharInventoryItem(button->arg);

	recalcCharacterStats(_selectedCharacter);

	if (_itemInHand)
		runItemScript(_selectedCharacter, _itemInHand, 0x100, 0, 0);
	if (ih)
		runItemScript(_selectedCharacter, ih, 0x80, 0, 0);

	gui_drawCharInventoryItem(button->arg);
	gui_drawCharPortraitWithStats(_selectedCharacter);
	gui_changeCharacterStats(_selectedCharacter);

	return 1;
}

int LoLEngine::clickedExitCharInventory(Button *button) {
	_updateFlags &= 0xfff3;
	gui_enableDefaultPlayfieldButtons();
	_weaponsDisabled = false;

	for (int i = 0; i < 4; i++) {
		if (_charInventoryUnk & (1 << i))
			_characters[i].flags &= 0xf1ff;
	}

	_screen->copyBlockToPage(2, 0, 0, 320, 200, _pageBuffer1);

	int cp = _screen->setCurPage(2);
	gui_drawAllCharPortraitsWithStats();
	gui_drawInventory();
	_screen->setCurPage(cp);

	_screen->copyPage(2, 0);
	_screen->updateScreen();
	gui_enableControls();
	_screen->copyBlockToPage(2, 0, 0, 320, 200, _pageBuffer2);

	_lastCharInventory = -1;
	updateDrawPage2();
	enableSysTimer(2);

	return 1;
}

int LoLEngine::clickedSceneDropItem(Button *button) {
	static const uint8 offsX[] = { 0x40, 0xC0, 0x40, 0xC0 };
	static const uint8 offsY[] = { 0x40, 0x40, 0xC0, 0xC0 };

	if ((_updateFlags & 1) || !_itemInHand)
		return 0;

	uint16 block = _currentBlock;
	if (button->arg > 1) {
		block = calcNewBlockPosition(_currentBlock, _currentDirection);
		int f = _wllWallFlags[_levelBlockProperties[block].walls[_currentDirection ^ 2]];
		if (!(f & 0x80) || (f & 2))
			return 1;
	}

	uint16 x = 0;
	uint16 y = 0;
	int i = _dropItemDirIndex[(_currentDirection << 2) + button->arg];

	calcCoordinates(x, y, block, offsX[i], offsY[i]);
	setItemPosition(_itemInHand, x, y, 0, 1);
	setHandItem(0);

	return 1;
}

int LoLEngine::clickedScenePickupItem(Button *button) {
	static const int8 checkX[] = { 0, 0, 1, 0, -1, -1, 1, 1, -1, 0, 2, 0, -2, -1, 1, 2, 2, 1, -1, -2, -2 };
	static const int8 checkY[] = { 0, -1, 0, 1, 0, -1, -1, 1, 1, -2, 0, 2, 0, -2, -2, -1, 1, 2, 2, 1, -1 };
	static const int len = ARRAYSIZE(checkX);

	if ((_updateFlags & 1) || _itemInHand)
		return 0;

	int cp = _screen->setCurPage(_sceneDrawPage1);

	redrawSceneItem();

	int p = 0;
	for (int i = 0; i < len; i++) {
		p = _screen->getPagePixel(_screen->_curPage, CLIP(_mouseX + checkX[i], 0, 320), CLIP(_mouseY + checkY[i], 0, 200));
		if (p)
			break;
	}

	_screen->setCurPage(cp);

	if (!p)
		return 0;

	uint16 block = (p <= 128) ? calcNewBlockPosition(_currentBlock, _currentDirection) : _currentBlock;

	int found = checkSceneForItems(&_levelBlockProperties[block].drawObjects, p & 0x7f);

	if (found != -1) {
		removeLevelItem(found, block);
		setHandItem(found);
	}

	_sceneUpdateRequired = true;

	return 1;
}

int LoLEngine::clickedInventorySlot(Button *button) {
	int slot = _inventoryCurItem + button->arg;
	if (slot > 47)
		slot -= 48;

	uint16 slotItem = _inventory[slot];
	int hItem = _itemInHand;

	if ((_itemsInPlay[hItem].itemPropertyIndex == 281 || _itemsInPlay[slotItem].itemPropertyIndex == 281) &&
	        (_itemsInPlay[hItem].itemPropertyIndex == 220 || _itemsInPlay[slotItem].itemPropertyIndex == 220)) {
		// merge ruby of truth

		WSAMovie_v2 *wsa = new WSAMovie_v2(this);
		wsa->open("truth.wsa", 0, 0);

		_screen->hideMouse();

		_inventory[slot] = 0;
		gui_drawInventoryItem(button->arg);
		_screen->copyRegion(button->x, button->y - 3, button->x, button->y - 3, 25, 27, 0, 2);
		KyraEngine_v1::snd_playSoundEffect(99);

		for (int i = 0; i < 25; i++) {
			uint32 delayTimer = _system->getMillis() + 7 * _tickLength;
			_screen->copyRegion(button->x, button->y - 3, 0, 0, 25, 27, 2, 2);
			wsa->displayFrame(i, 2, 0, 0, 0x4000, 0, 0);
			_screen->copyRegion(0, 0, button->x, button->y - 3, 25, 27, 2, 0);
			_screen->updateScreen();
			delayUntil(delayTimer);
		}

		_screen->showMouse();

		wsa->close();
		delete wsa;

		deleteItem(slotItem);
		deleteItem(hItem);

		setHandItem(0);
		_inventory[slot] = makeItem(280, 0, 0);
	} else {
		setHandItem(slotItem);
		_inventory[slot] = hItem;
	}

	gui_drawInventoryItem(button->arg);

	return 1;
}

int LoLEngine::clickedInventoryScroll(Button *button) {
	int8 inc = (int8)button->arg;
	int shp = (inc == 1) ? 75 : 74;
	if (!_flags.isTalkie)
		shp -= 2;

	if (button->flags2 & 0x1000)
		inc *= 9;

	_inventoryCurItem += inc;

	gui_toggleButtonDisplayMode(shp, 1);

	if (_inventoryCurItem < 0)
		_inventoryCurItem += 48;
	if (_inventoryCurItem > 47)
		_inventoryCurItem -= 48;

	gui_drawInventory();
	gui_toggleButtonDisplayMode(shp, 0);

	return 1;
}

int LoLEngine::clickedWall(Button *button) {
	int block = calcNewBlockPosition(_currentBlock, _currentDirection);
	int dir = _currentDirection ^ 2;
	uint8 type = _specialWallTypes[_levelBlockProperties[block].walls[dir]];

	int res = 0;
	switch (type) {
	case 1:
		res = clickedWallShape(block, dir);
		break;

	case 2:
		res = clickedLeverOn(block, dir);
		break;

	case 3:
		res = clickedLeverOff(block, dir);
		break;

	case 4:
		res = clickedWallOnlyScript(block);
		break;

	case 5:
		res = clickedDoorSwitch(block, dir);
		break;

	case 6:
		res = clickedNiche(block, dir);
		break;

	default:
		break;
	}

	return res;
}

int LoLEngine::clickedSequenceWindow(Button *button) {
	runLevelScript(calcNewBlockPosition(_currentBlock, _currentDirection), 0x40);
	if (!_seqTrigger || !posWithinRect(_mouseX, _mouseY, _seqWindowX1, _seqWindowY1, _seqWindowX2, _seqWindowY2)) {
		_seqTrigger = 0;
		removeInputTop();
	}
	return 1;
}

int LoLEngine::clickedScroll(Button *button) {
	if (_selectedSpell == button->arg)
		return 1;

	gui_highlightSelectedSpell(false);
	_selectedSpell = button->arg;
	gui_highlightSelectedSpell(true);
	gui_drawAllCharPortraitsWithStats();

	return 1;
}

int LoLEngine::clickedSpellTargetCharacter(Button *button) {
	int t = button->arg;
	_txt->printMessage(0, "%s.\r", _characters[t].name);

	if ((_spellProperties[_activeSpell.spell].flags & 0xff) == 1) {
		_activeSpell.target = t;
		castHealOnSingleCharacter(&_activeSpell);
	}

	gui_enableDefaultPlayfieldButtons();
	return 1;
}

int LoLEngine::clickedSpellTargetScene(Button *button) {
	LoLCharacter *c = &_characters[_activeSpell.charNum];
	_txt->printMessage(0, "%s", getLangString(0x4041));

	c->magicPointsCur += _activeSpell.p->mpRequired[_activeSpell.level];
	if (c->magicPointsCur > c->magicPointsMax)
		c->magicPointsCur = c->magicPointsMax;

	c->hitPointsCur += _activeSpell.p->hpRequired[_activeSpell.level];
	if (c->hitPointsCur > c->hitPointsMax)
		c->hitPointsCur = c->hitPointsMax;

	gui_drawCharPortraitWithStats(_activeSpell.charNum);
	gui_enableDefaultPlayfieldButtons();

	return 1;
}

int LoLEngine::clickedSceneThrowItem(Button *button) {
	if (_updateFlags & 1)
		return 0;

	uint16 block = calcNewBlockPosition(_currentBlock, _currentDirection);
	if ((_wllWallFlags[_levelBlockProperties[block].walls[_currentDirection ^ 2]] & 2) || !_itemInHand)
		return 0;

	uint16 x = 0;
	uint16 y = 0;
	calcCoordinates(x, y, _currentBlock, 0x80, 0x80);

	if (launchObject(0, _itemInHand, x, y, 12, _currentDirection << 1, 6, _selectedCharacter, 0x3f)) {
		snd_playSoundEffect(18, -1);
		setHandItem(0);
	}

	_sceneUpdateRequired = true;
	return 1;
}

int LoLEngine::clickedOptions(Button *button) {
	removeInputTop();
	gui_toggleButtonDisplayMode(_flags.isTalkie ? 76 : 74, 1);

	_updateFlags |= 4;

	Button b;
	b.data0Val2 = b.data1Val2 = b.data2Val2 = 0xfe;
	b.data0Val3 = b.data1Val3 = b.data2Val3 = 0x01;

	if (_weaponsDisabled)
		clickedExitCharInventory(&b);

	initTextFading(0, 1);
	stopPortraitSpeechAnim();
	setLampMode(true);
	setMouseCursorToIcon(0);
	disableSysTimer(2);

	gui_toggleButtonDisplayMode(_flags.isTalkie ? 76 : 74, 0);

	bool speechWasEnabled = speechEnabled();
	if (_flags.isTalkie && getVolume(kVolumeSpeech) == 2)
		_configVoice |= (textEnabled() ? 2 : 1);

	_gui->runMenu(_gui->_mainMenu);

	_updateFlags &= 0xfffb;
	setMouseCursorToItemInHand();
	resetLampStatus();
	gui_enableDefaultPlayfieldButtons();
	enableSysTimer(2);
	updateDrawPage2();

	gui_drawPlayField();

	if (getVolume(kVolumeSpeech) == 2)
		_configVoice &= (textEnabled() ? ~2 : ~1);

	if (speechWasEnabled && !textEnabled() && !speechEnabled())
		_configVoice = 0;

	writeSettings();

	return 1;
}

int LoLEngine::clickedRestParty(Button *button) {
	gui_toggleButtonDisplayMode(_flags.isTalkie ? 77 : 75, 1);

	Button b;
	b.data0Val2 = b.data1Val2 = b.data2Val2 = 0xfe;
	b.data0Val3 = b.data1Val3 = b.data2Val3 = 0x01;

	if (_weaponsDisabled)
		clickedExitCharInventory(&b);

	int tHp = -1;
	int tMp = -1;
	int tHa = -1;
	int needPoisoningFlags = 0;
	int needHealingFlags = 0;
	int needMagicGainFlags = 0;

	for (int i = 0; i < 4; i++) {
		LoLCharacter *c = &_characters[i];
		if (!(c->flags & 1) || (c->flags & 8))
			continue;

		if (c->hitPointsMax > tHp)
			tHp = c->hitPointsMax;

		if (c->magicPointsMax > tMp)
			tMp = c->magicPointsMax;

		if (c->flags & 0x80) {
			needPoisoningFlags |= (1 << i);
			if (c->hitPointsCur > tHa)
				tHa = c->hitPointsCur;
		} else {
			if (c->hitPointsCur < c->hitPointsMax)
				needHealingFlags |= (1 << i);
		}

		if (c->magicPointsCur < c->magicPointsMax)
			needMagicGainFlags |= (1 << i);

		c->flags |= 0x1000;
	}

	removeInputTop();

	if (needHealingFlags || needMagicGainFlags) {
		_screen->fillRect(112, 0, 288, 120, _flags.use16ColorMode ? 0x44 : 1);
		gui_drawAllCharPortraitsWithStats();

		_txt->printMessage(0x8000, "%s", getLangString(0x4057));
		gui_toggleButtonDisplayMode(_flags.isTalkie ? 77 : 75, 0);

		int h = 600 / tHp;
		if (h > 30)
			h = 30;

		int m = 600 / tMp;
		if (m > 30)
			m = 30;

		int a = 600 / tHa;
		if (a > 15)
			a = 15;

		uint32 delay1 = _system->getMillis() + h * _tickLength;
		uint32 delay2 = _system->getMillis() + m * _tickLength;
		uint32 delay3 = _system->getMillis() + a * _tickLength;

		_partyAwake = false;
		_updateFlags |= 1;

		for (int i = 0, im = _smoothScrollModeNormal ? 32 : 16; i < im; i++) {
			timerProcessMonsters(0);
			timerProcessMonsters(1);
			timerProcessDoors(0);
			timerProcessFlyingObjects(0);

			if (_partyAwake)
				break;
		}

		resetBlockProperties();

		do {
			for (int i = 0, im = _smoothScrollModeNormal ? 8 : 4; i < im; i++) {
				timerProcessMonsters(0);
				timerProcessMonsters(1);
				timerProcessDoors(0);
				timerProcessFlyingObjects(0);

				if (_partyAwake)
					break;
			}

			int f = checkInput(0);
			removeInputTop();

			if (f & 0x800) {
				gui_notifyButtonListChanged();
			} else if (f) {
				gui_triggerEvent(f);
				break;
			}

			if (!_partyAwake) {
				if (_system->getMillis() > delay3) {
					for (int i = 0; i < 4; i++) {
						if (!(needPoisoningFlags & (1 << i)))
							continue;
						inflictDamage(i, 1, 0x8000, 1, 0x80);
						if (_characters[i].flags & 8)
							needPoisoningFlags &= ~(1 << i);
					}
					delay3 = _system->getMillis() + a * _tickLength;
				}

				if (_system->getMillis() > delay1) {
					for (int i = 0; i < 4; i++) {
						if (!(needHealingFlags & (1 << i)))
							continue;
						increaseCharacterHitpoints(i, 1, false);
						gui_drawCharPortraitWithStats(i);
						if (_characters[i].hitPointsCur == _characters[i].hitPointsMax)
							needHealingFlags &= ~(1 << i);
					}
					delay1 = _system->getMillis() + h * _tickLength;
				}

				if (_system->getMillis() > delay2) {
					for (int i = 0; i < 4; i++) {
						if (!(needMagicGainFlags & (1 << i)))
							continue;
						_characters[i].magicPointsCur++;
						gui_drawCharPortraitWithStats(i);
						if (_characters[i].magicPointsCur == _characters[i].magicPointsMax)
							needMagicGainFlags &= ~(1 << i);
					}
					delay2 = _system->getMillis() + m * _tickLength;
				}
				_screen->updateScreen();
			}

		} while (!_partyAwake && (needHealingFlags || needMagicGainFlags));

		for (int i = 0; i < 4; i++) {
			int frm = 0;
			int upd = 0;
			bool setframe = true;

			if (_characters[i].flags & 0x1000) {
				_characters[i].flags &= 0xefff;

				if (_partyAwake) {
					if (_characters[i].damageSuffered) {
						frm = 5;
						snd_playSoundEffect(_characters[i].screamSfx, -1);
					} else {
						frm = 4;
					}
					upd = 6;
				}

			} else {
				if (_characters[i].damageSuffered)
					setframe = false;
				else
					frm = 4;
			}

			if (setframe)
				setTemporaryFaceFrame(i, frm, upd, 1);
		}

		_updateFlags &= 0xfffe;
		_partyAwake = true;
		updateDrawPage2();
		gui_drawScene(0);
		_txt->printMessage(0x8000, "%s", getLangString(0x4059));
		_screen->fadeToPalette1(40);

	} else {
		for (int i = 0; i < 4; i++)
			_characters[i].flags &= 0xefff;

		if (needPoisoningFlags) {
			setTemporaryFaceFrameForAllCharacters(0, 0, 0);
			for (int i = 0; i < 4; i++) {
				if (needPoisoningFlags & (1 << i))
					setTemporaryFaceFrame(i, 3, 8, 0);
			}
			_txt->printMessage(0x8000, "%s", getLangString(0x405a));
			gui_drawAllCharPortraitsWithStats();

		} else {
			setTemporaryFaceFrameForAllCharacters(2, 4, 1);
			_txt->printMessage(0x8000, "%s", getLangString(0x4058));
		}
		gui_toggleButtonDisplayMode(_flags.isTalkie ? 77 : 75, 0);
	}

	return 1;
}

int LoLEngine::clickedMoneyBox(Button *button) {
	_txt->printMessage(0, getLangString(_credits == 1 ? 0x402D : 0x402E), _credits);
	return 1;
}

int LoLEngine::clickedCompass(Button *button) {
	if (!(_flagsTable[31] & 0x40))
		return 0;

	if (_compassBroken) {
		if (characterSays(0x425b, -1, true))
			_txt->printMessage(4, "%s", getLangString(0x425b));
	} else {
		_txt->printMessage(0, "%s", getLangString(0x402f + _currentDirection));
	}

	return 1;
}

int LoLEngine::clickedAutomap(Button *button) {
	if (!(_flagsTable[31] & 0x10))
		return 0;

	removeInputTop();
	displayAutomap();

	gui_drawPlayField();
	setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);
	return 1;
}

int LoLEngine::clickedLamp(Button *button) {
	if (!(_flagsTable[31] & 0x08))
		return 0;

	if (_itemsInPlay[_itemInHand].itemPropertyIndex == 248) {
		if (_lampOilStatus >= 100) {
			_txt->printMessage(0, "%s", getLangString(0x4061));
			return 1;
		}

		_txt->printMessage(0, "%s", getLangString(0x4062));

		deleteItem(_itemInHand);
		snd_playSoundEffect(181, -1);
		setHandItem(0);

		_lampOilStatus += 100;

	} else {
		uint16 s = (_lampOilStatus >= 100) ? 0x4060 : ((!_lampOilStatus) ? 0x405c : (_lampOilStatus / 33) + 0x405d);
		_txt->printMessage(0, getLangString(0x405b), getLangString(s));
	}

	if (_brightness)
		setPaletteBrightness(_screen->getPalette(0), _brightness, _lampEffect);

	return 1;
}

int LoLEngine::clickedStatusIcon(Button *button) {
	int t = _mouseX - 220;
	if (t < 0)
		t = 0;

	t /= 14;
	if (t > 2)
		t = 2;

	uint16 str = _charStatusFlags[t] + 1;
	if (str == 0 || str > 3)
		return 1;

	_txt->printMessage(0x8002, "%s", getLangString(str == 1 ? 0x424c : (str == 2 ? 0x424e : 0x424d)));
	return 1;
}

GUI_LoL::GUI_LoL(LoLEngine *vm) : GUI_v1(vm), _vm(vm), _screen(vm->_screen) {
	_scrollUpFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::scrollUp);
	_scrollDownFunctor = BUTTON_FUNCTOR(GUI_LoL, this, &GUI_LoL::scrollDown);

	_redrawButtonFunctor = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::redrawButtonCallback);
	_redrawShadedButtonFunctor = BUTTON_FUNCTOR(GUI_v1, this, &GUI_v1::redrawShadedButtonCallback);

	_specialProcessButton = _backUpButtonList = 0;
	_flagsModifier = 0;
	_sliderSfx = 11;
}

void GUI_LoL::processButton(Button *button) {
	if (!button)
		return;

	if (button->flags & 8) {
		if (button->flags & 0x10) {
			// XXX
		}
		return;
	}

	int entry = button->flags2 & 5;

	byte val1 = 0, val2 = 0, val3 = 0;
	const uint8 *dataPtr = 0;
	Button::Callback callback;
	if (entry == 1) {
		val1 = button->data1Val1;
		dataPtr = button->data1ShapePtr;
		callback = button->data1Callback;
		val2 = button->data1Val2;
		val3 = button->data1Val3;
	} else if (entry == 4 || entry == 5) {
		val1 = button->data2Val1;
		dataPtr = button->data2ShapePtr;
		callback = button->data2Callback;
		val2 = button->arg;
		val3 = button->data2Val3;
	} else {
		val1 = button->data0Val1;
		dataPtr = button->data0ShapePtr;
		callback = button->data0Callback;
		val2 = button->data0Val2;
		val3 = button->data0Val3;
	}

	int x = 0, y = 0, x2 = 0, y2 = 0;

	x = button->x;
	if (x < 0)
		x += _screen->getScreenDim(button->dimTableIndex)->w << 3;
	x += _screen->getScreenDim(button->dimTableIndex)->sx << 3;
	x2 = x + button->width - 1;

	y = button->y;
	if (y < 0)
		y += _screen->getScreenDim(button->dimTableIndex)->h << 3;
	y += _screen->getScreenDim(button->dimTableIndex)->sy << 3;
	y2 = y + button->height - 1;

	switch (val1 - 1) {
	case 0:
		_screen->hideMouse();
		_screen->drawShape(_screen->_curPage, dataPtr, x, y, button->dimTableIndex, 0x10);
		_screen->showMouse();
		break;

	case 1:
		_screen->hideMouse();
		_screen->printText((const char *)dataPtr, x, y, val2, val3);
		_screen->showMouse();
		break;

	case 3:
		if (callback)
			(*callback)(button);
		break;

	case 4:
		_screen->hideMouse();
		_screen->drawBox(x, y, x2, y2, val2);
		_screen->showMouse();
		break;

	case 5:
		_screen->hideMouse();
		_screen->fillRect(x, y, x2, y2, val2, -1, true);
		_screen->showMouse();
		break;

	default:
		break;
	}

	_screen->updateScreen();
}

int GUI_LoL::processButtonList(Button *buttonList, uint16 inputFlag, int8 mouseWheel) {
	if (!buttonList)
		return inputFlag & 0x7FFF;

	if (_backUpButtonList != buttonList || _vm->_buttonListChanged) {
		_specialProcessButton = 0;

		_flagsModifier = 0;
		if (_vm->_mouseClick == 1)
			_flagsModifier |= 0x200;
		if (_vm->_mouseClick == 2)
			_flagsModifier |= 0x2000;
		_vm->_mouseClick = 0;

		_backUpButtonList = buttonList;
		_vm->_buttonListChanged = false;

		while (buttonList) {
			processButton(buttonList);
			buttonList = buttonList->nextButton;
		}
	}

	int mouseX = _vm->_mouseX;
	int mouseY = _vm->_mouseY;

	uint16 flags = 0;

	if (1/*!_screen_cursorDisable*/) {
		uint16 inFlags = inputFlag & 0xFF;
		uint16 temp = 0;

		// HACK: inFlags == 200 is our left button (up)
		if (inFlags == 199 || inFlags == 200)
			temp = 0x100;
		if (inFlags == 201 || inFlags == 202)
			temp = 0x1000;

		if (inputFlag & 0x800)
			temp <<= 2;

		flags |= temp;

		_flagsModifier &= ~((temp & 0x4400) >> 1);
		_flagsModifier |= (temp & 0x1100) * 2;
		flags |= _flagsModifier;
		flags |= (_flagsModifier << 2) ^ 0x8800;
	}

	buttonList = _backUpButtonList;
	if (_specialProcessButton) {
		buttonList = _specialProcessButton;
		if (_specialProcessButton->flags & 8)
			_specialProcessButton = 0;
	}

	int returnValue = 0;
	while (buttonList) {
		if (buttonList->flags & 8) {
			buttonList = buttonList->nextButton;
			continue;
		}
		buttonList->flags2 &= ~0x18;
		buttonList->flags2 |= (buttonList->flags2 & 3) << 3;

		int x = buttonList->x;
		if (x < 0)
			x += _screen->getScreenDim(buttonList->dimTableIndex)->w << 3;
		x += _screen->getScreenDim(buttonList->dimTableIndex)->sx << 3;

		int y = buttonList->y;
		if (y < 0)
			y += _screen->getScreenDim(buttonList->dimTableIndex)->h;
		y += _screen->getScreenDim(buttonList->dimTableIndex)->sy;

		bool progress = false;

		if (mouseX >= x && mouseY >= y && mouseX <= x + buttonList->width && mouseY <= y + buttonList->height)
			progress = true;

		buttonList->flags2 &= ~0x80;
		uint16 inFlags = inputFlag & 0x7FFF;
		if (inFlags) {
			if (buttonList->keyCode == inFlags) {
				progress = true;
				flags = buttonList->flags & 0x0F00;
				buttonList->flags2 |= 0x80;
				inputFlag = 0;
				_specialProcessButton = buttonList;
			} else if (buttonList->keyCode2 == inFlags) {
				flags = buttonList->flags & 0xF000;
				if (!flags)
					flags = buttonList->flags & 0x0F00;
				progress = true;
				buttonList->flags2 |= 0x80;
				inputFlag = 0;
				_specialProcessButton = buttonList;
			}
		}

		bool unk1 = false;

		if (mouseWheel && buttonList->mouseWheel == mouseWheel) {
			progress = true;
			unk1 = true;
		}

		if (!progress)
			buttonList->flags2 &= ~6;

		if ((flags & 0x3300) && (buttonList->flags & 4) && progress && (buttonList == _specialProcessButton || !_specialProcessButton)) {
			buttonList->flags |= 6;
			if (!_specialProcessButton)
				_specialProcessButton = buttonList;
		} else if ((flags & 0x8800) && !(buttonList->flags & 4) && progress) {
			buttonList->flags2 |= 6;
		} else {
			buttonList->flags2 &= ~6;
		}

		bool progressSwitch = false;
		if (!_specialProcessButton) {
			progressSwitch = progress;
		} else  {
			if (_specialProcessButton->flags & 0x40)
				progressSwitch = (_specialProcessButton == buttonList);
			else
				progressSwitch = progress;
		}

		if (progressSwitch) {
			if ((flags & 0x1100) && progress && !_specialProcessButton) {
				inputFlag = 0;
				_specialProcessButton = buttonList;
			}

			if ((buttonList->flags & flags) && (progress || !(buttonList->flags & 1))) {
				uint16 combinedFlags = (buttonList->flags & flags);
				combinedFlags = ((combinedFlags & 0xF000) >> 4) | (combinedFlags & 0x0F00);
				combinedFlags >>= 8;

				static const uint16 flagTable[] = {
					0x000, 0x100, 0x200, 0x100, 0x400, 0x100, 0x400, 0x100, 0x800, 0x100,
					0x200, 0x100, 0x400, 0x100, 0x400, 0x100
				};

				assert(combinedFlags < ARRAYSIZE(flagTable));

				switch (flagTable[combinedFlags]) {
				case 0x400:
					if (!(buttonList->flags & 1) || ((buttonList->flags & 1) && _specialProcessButton == buttonList)) {
						buttonList->flags2 ^= 1;
						returnValue = buttonList->index | 0x8000;
						unk1 = true;
					}

					if (!(buttonList->flags & 4)) {
						buttonList->flags2 &= ~4;
						buttonList->flags2 &= ~2;
					}
					break;

				case 0x800:
					if (!(buttonList->flags & 4)) {
						buttonList->flags2 |= 4;
						buttonList->flags2 |= 2;
					}

					if (!(buttonList->flags & 1))
						unk1 = true;
					break;

				case 0x200:
					if (buttonList->flags & 4) {
						buttonList->flags2 |= 4;
						buttonList->flags2 |= 2;
					}

					if (!(buttonList->flags & 1))
						unk1 = true;
					break;

				case 0x100:
				default:
					buttonList->flags2 ^= 1;
					returnValue = buttonList->index | 0x8000;
					unk1 = true;
					if (buttonList->flags & 4) {
						buttonList->flags2 |= 4;
						buttonList->flags2 |= 2;
					}
					_specialProcessButton = buttonList;
				}
			}
		}

		bool unk2 = false;
		if ((flags & 0x2200) && progress) {
			buttonList->flags2 |= 6;
			if (!(buttonList->flags & 4) && !(buttonList->flags2 & 1)) {
				unk2 = true;
				buttonList->flags2 |= 1;
			}
		}

		if ((flags & 0x8800) == 0x8800) {
			_specialProcessButton = 0;
			if (!progress || (buttonList->flags & 4))
				buttonList->flags2 &= ~6;
		}

		if (!progress && buttonList == _specialProcessButton && !(buttonList->flags & 0x40))
			_specialProcessButton = 0;

		if ((buttonList->flags2 & 0x18) != ((buttonList->flags2 & 3) << 3))
			processButton(buttonList);

		if (unk2)
			buttonList->flags2 &= ~1;

		if (unk1) {
			buttonList->flags2 &= 0xFF;
			buttonList->flags2 |= flags;

			if (buttonList->buttonCallback) {
				//_vm->removeInputTop();
				if ((*buttonList->buttonCallback)(buttonList))
					break;
			}

			if (buttonList->flags & 0x20)
				break;
		}

		if (_specialProcessButton == buttonList && (buttonList->flags & 0x40))
			break;

		buttonList = buttonList->nextButton;
	}

	if (!returnValue)
		returnValue = inputFlag & 0x7FFF;
	return returnValue;
}

int GUI_LoL::redrawButtonCallback(Button *button) {
	if (!_displayMenu || _vm->gameFlags().use16ColorMode)
		return 0;

	_screen->drawBox(button->x + 1, button->y + 1, button->x + button->width - 1, button->y + button->height - 1, 225);
	return 0;
}

int GUI_LoL::redrawShadedButtonCallback(Button *button) {
	if (!_displayMenu || _vm->gameFlags().use16ColorMode)
		return 0;

	_screen->drawShadedBox(button->x, button->y, button->x + button->width, button->y + button->height, 223, 227);
	return 0;
}

int GUI_LoL::runMenu(Menu &menu) {
	_currentMenu = &menu;
	_lastMenu = _currentMenu;
	_newMenu = 0;
	_displayMenu = true;
	_menuResult = 1;
	_savegameOffset = 0;
	backupPage0();

	const ScreenDim *d = _screen->getScreenDim(8);
	uint32 textCursorTimer = 0;
	uint8 textCursorStatus = 1;
	Screen::FontId of = _screen->setFont(Screen::FID_9_FNT);
	int wW = _screen->getCharWidth('W');
	_screen->setFont(of);

	int fW = (d->w << 3) - wW;
	int fC = 0;

	// LoL doesnt't have default higlighted items. No item should be
	// highlighted when entering a new menu.
	// Instead, the respevtive struct entry is used to determine whether
	// a menu has scroll buttons or slider bars.
	uint8 hasSpecialButtons = 0;
	_saveSlotsListUpdateNeeded = true;

	while (_displayMenu) {
		_vm->_mouseX = _vm->_mouseY = 0;

		if (_currentMenu == &_loadMenu || _currentMenu == &_saveMenu || _currentMenu == &_deleteMenu) {
			updateSaveSlotsList(_vm->_targetName);
			setupSaveMenuSlots(*_currentMenu, 4);
		}

		hasSpecialButtons = _currentMenu->highlightedItem;
		_currentMenu->highlightedItem = 255;

		if (_currentMenu == &_gameOptions) {
			char *s = (char *)_vm->_tempBuffer5120;
			Common::strlcpy(s, _vm->getLangString(0x406f + _vm->_monsterDifficulty), 30);
			_currentMenu->item[_vm->gameFlags().isTalkie ? 0 : 2].itemString = s;
			s += (strlen(s) + 1);

			Common::strlcpy(s, _vm->getLangString(_vm->_smoothScrollingEnabled ? 0x4068 : 0x4069), 30);
			_currentMenu->item[_vm->gameFlags().isTalkie ? 1 : 3].itemString = s;
			s += (strlen(s) + 1);

			Common::strlcpy(s, _vm->getLangString(_vm->_floatingCursorsEnabled ? 0x4068 : 0x4069), 30);
			_currentMenu->item[_vm->gameFlags().isTalkie ? 2 : 4].itemString = s;
			s += (strlen(s) + 1);

			if (_vm->gameFlags().isTalkie) {
				Common::strlcpy(s, _vm->getLangString(0x42d6 + _vm->_lang), 30);
				_currentMenu->item[3].itemString = s;
				s += (strlen(s) + 1);

				Common::strlcpy(s, _vm->getLangString(_vm->textEnabled() ? 0x4068 : 0x4069), 30);
				_currentMenu->item[4].itemString = s;
				s += (strlen(s) + 1);
			} else {
				Common::strlcpy(s, _vm->getLangString(_vm->_configMusic ? 0x4068 : 0x4069), 30);
				_currentMenu->item[0].itemString = s;
				s += (strlen(s) + 1);

				Common::strlcpy(s, _vm->getLangString(_vm->_configSounds ? 0x4068 : 0x4069), 30);
				_currentMenu->item[1].itemString = s;
				s += (strlen(s) + 1);
			}
		}

		if (hasSpecialButtons == 1) {
			if (_savegameOffset == 0) {
				_scrollUpButton.data0ShapePtr = _scrollUpButton.data1ShapePtr = _scrollUpButton.data2ShapePtr = 0;
			} else {
				_scrollUpButton.data0ShapePtr = _vm->_gameShapes[17];
				_scrollUpButton.data1ShapePtr = _scrollUpButton.data2ShapePtr = _vm->_gameShapes[19];
			}

			int slotOffs = (_currentMenu == &_saveMenu) ? 1 : 0;

			if (((uint)_savegameOffset == _saveSlots.size() - (4 - slotOffs)) || _saveSlots.size() < (uint)(5 - slotOffs)) {
				_scrollDownButton.data0ShapePtr = _scrollDownButton.data1ShapePtr = _scrollDownButton.data2ShapePtr = 0;
			} else {
				_scrollDownButton.data0ShapePtr = _vm->_gameShapes[18];
				_scrollDownButton.data1ShapePtr = _scrollDownButton.data2ShapePtr = _vm->_gameShapes[20];
			}
		}

		for (uint i = 0; i < _currentMenu->numberOfItems; ++i) {
			_menuButtons[i].data0Val1 = _menuButtons[i].data1Val1 = _menuButtons[i].data2Val1 = 4;
			_menuButtons[i].data0Callback = _redrawShadedButtonFunctor;
			_menuButtons[i].data1Callback = _menuButtons[i].data2Callback = _redrawButtonFunctor;
			_menuButtons[i].flags = 0x4487;
			_menuButtons[i].flags2 = 0;
		}

		initMenu(*_currentMenu);

		if (_currentMenu == &_loadMenu || _currentMenu == &_deleteMenu) {
			if (_saveSlots.begin() == _saveSlots.end())
				// "no savegames to load" message
				_screen->fprintString("%s", _currentMenu->x + _currentMenu->width / 2, _currentMenu->y + 42, 204, 0, 9, _vm->getLangString(0x4009));
		}

		if (hasSpecialButtons == 2) {
			static const uint8 oX[] = { 0, 10, 124 };
			static const uint8 oW[] = { 10, 114, 10 };

			for (int i = 1; i < 4; ++i) {
				int tX = _currentMenu->x + _currentMenu->item[i].x;
				int tY = _currentMenu->y + _currentMenu->item[i].y;

				for (int ii = 0; ii < 3; ++ii) {
					Button *b = getButtonListData() + 1 + (i - 1) * 3 + ii;
					b->nextButton = 0;
					b->data0Val2 = b->data1Val2 = b->data2Val2 = 0xfe;
					b->data0Val3 = b->data1Val3 = b->data2Val3 = 0x01;

					b->index = ii;
					b->keyCode = b->keyCode2 = 0;

					b->x = tX + oX[ii];
					b->y = tY;
					b->width = oW[ii];
					b->height = _currentMenu->item[i].height;

					b->data0Val1 = b->data1Val1 = b->data2Val1 = 0;
					b->flags = (ii == 1) ? 0x6606 : 0x4406;

					b->dimTableIndex = 0;

					b->buttonCallback = _currentMenu->item[i].callback;
					b->arg = _currentMenu->item[i].itemId;

					_menuButtonList = addButtonToList(_menuButtonList, b);

					processButton(b);
					updateButton(b);
				}

				_currentMenu->item[i].labelX = _currentMenu->item[i].x - 5;
				_currentMenu->item[i].labelY = _currentMenu->item[i].y + 3;

				printMenuText(getMenuItemLabel(_currentMenu->item[i]), _currentMenu->x + _currentMenu->item[i].labelX, _currentMenu->y + _currentMenu->item[i].labelY, _currentMenu->item[i].textColor, 0, 10);

				int volume = _vm->getVolume((KyraEngine_v1::kVolumeEntry)(i - 1));
				_screen->drawShape(_screen->_curPage, _vm->_gameShapes[85], tX , tY, 0, 0x10);
				_screen->drawShape(_screen->_curPage, _vm->_gameShapes[87], tX + 2 + oX[1], tY, 0, 0x10);
				_screen->drawShape(_screen->_curPage, _vm->_gameShapes[86], tX + oX[1] + volume, tY, 0, 0x10);
			}

			_screen->updateScreen();
		}

		if (_currentMenu == &_mainMenu && !_vm->gameFlags().use16ColorMode) {
			Screen::FontId f = _screen->setFont(Screen::FID_6_FNT);
			_screen->fprintString("%s", menu.x + 8, menu.y + menu.height - 12, 204, 0, 8, gScummVMVersion);
			_screen->setFont(f);
			_screen->updateScreen();
		}

		if (_currentMenu == &_savenameMenu) {
			int mx = (d->sx << 3) - 1;
			int my = d->sy - 1;
			int mw = (d->w << 3) + 1;
			int mh = d->h + 1;
			if (_vm->gameFlags().use16ColorMode) {
				_screen->drawShadedBox(mx, my, mx + mw + 1, my + mh + 1, 0xdd, 0xff);
				_screen->drawLine(true, mx + mw + 1, my, mh + 1, 0xcc);
				_screen->drawLine(false, mx, my + mh + 1, mw + 2, 0xcc);
			} else {
				_screen->drawShadedBox(mx, my, mx + mw, my + mh, 227, 223);
			}

			int pg = _screen->setCurPage(0);
			_vm->_txt->clearDim(8);
			textCursorTimer = 0;
			textCursorStatus = 0;

			Screen::FontId f = _screen->setFont(Screen::FID_9_FNT);
			fC = _screen->getTextWidth(_saveDescription);
			while (fC >= fW) {
				_saveDescription[strlen(_saveDescription) - 1] = 0;
				fC = _screen->getTextWidth(_saveDescription);
			}

			_screen->fprintString("%s", (d->sx << 3), d->sy + 2, d->unk8, d->unkA, 0, _saveDescription);
			f = _screen->setFont(f);
			_screen->fillRect((d->sx << 3) + fC, d->sy, (d->sx << 3) + fC + wW, d->sy + d->h - (_vm->gameFlags().use16ColorMode ? 2 : 1), d->unk8, 0);
			_screen->setCurPage(pg);
		}

		while (!_newMenu && _displayMenu) {
			processHighlights(*_currentMenu);

			if (_currentMenu == &_savenameMenu) {
				if (textCursorTimer <= _vm->_system->getMillis()) {
					Screen::FontId f = _screen->setFont(Screen::FID_9_FNT);
					fC = _screen->getTextWidth(_saveDescription);
					textCursorStatus ^= 1;
					textCursorTimer = _vm->_system->getMillis() + 20 * _vm->_tickLength;
					_screen->fillRect((d->sx << 3) + fC, d->sy, (d->sx << 3) + fC + wW, d->sy + d->h - (_vm->gameFlags().use16ColorMode ? 2 : 1), textCursorStatus ? d->unk8 : d->unkA, 0);
					_screen->updateScreen();
					f = _screen->setFont(f);
				}
			}

			if (getInput()) {
				if (!_newMenu) {
					if (_currentMenu == &_savenameMenu) {
						Screen::FontId f = _screen->setFont(Screen::FID_9_FNT);
						_screen->fillRect((d->sx << 3) + fC, d->sy, (d->sx << 3) + fC + wW, d->sy + d->h - (_vm->gameFlags().use16ColorMode ? 2 : 1), d->unkA, 0);
						fC = _screen->getTextWidth(_saveDescription);
						while (fC >= fW) {
							_saveDescription[strlen(_saveDescription) - 1] = 0;
							fC = _screen->getTextWidth(_saveDescription);
						}
						_screen->fprintString("%s", (d->sx << 3), d->sy + 2, d->unk8, d->unkA, 0, _saveDescription);
						_screen->fillRect((d->sx << 3) + fC, d->sy, (d->sx << 3) + fC + wW, d->sy + d->h - (_vm->gameFlags().use16ColorMode ? 2 : 1), textCursorStatus ? d->unk8 : d->unkA, 0);
						f = _screen->setFont(f);
						textCursorTimer = 0;
						textCursorStatus = 0;
					} else {
						_newMenu = (_currentMenu != &_audioOptions) ? _currentMenu : 0;
					}
				} else {
					_lastMenu = _menuResult == -1 ? _lastMenu : _currentMenu;
				}
			}

			if (!_menuResult)
				_displayMenu = false;
		}

		if (_newMenu != _currentMenu || !_displayMenu)
			restorePage0();

		_currentMenu->highlightedItem = hasSpecialButtons;

		if (_newMenu)
			_currentMenu = _newMenu;

		_newMenu = 0;
	}

	if (_savegameList) {
		for (int i = 0; i < _savegameListSize; i++)
			delete[] _savegameList[i];
		delete[] _savegameList;
		_savegameList = 0;
	}

	return _menuResult;
}

void GUI_LoL::createScreenThumbnail(Graphics::Surface &dst) {
	uint8 *screenPal = new uint8[768];
	_screen->getRealPalette(1, screenPal);

	if (_vm->gameFlags().platform == Common::kPlatformPC98) {
		uint8 *screen = new uint8[Screen::SCREEN_W * Screen::SCREEN_H];
		assert(screen);

		_screen->copyRegionToBuffer(7, 0, 0, 320, 200, screen);
		Screen_LoL::convertPC98Gfx(screen, Screen::SCREEN_W, Screen::SCREEN_H, Screen::SCREEN_W);
		::createThumbnail(&dst, screen, Screen::SCREEN_W, Screen::SCREEN_H, screenPal);
		delete[] screen;
	} else {
		::createThumbnail(&dst, _screen->getCPagePtr(7), Screen::SCREEN_W, Screen::SCREEN_H, screenPal);
	}

	delete[] screenPal;
}

void GUI_LoL::backupPage0() {
	_screen->copyPage(0, 7);
}

void GUI_LoL::restorePage0() {
	_screen->copyPage(7, 0);
	_screen->updateScreen();
}

void GUI_LoL::setupSaveMenuSlots(Menu &menu, int num) {
	char *s = (char *)_vm->_tempBuffer5120;

	for (int i = 0; i < num; ++i) {
		menu.item[i].saveSlot = -1;
		menu.item[i].enabled = false;
	}

	int startSlot = 0;
	int slotOffs = 0;

	if (&menu == &_saveMenu) {
		if (_savegameOffset == 0)
			startSlot = 1;
		slotOffs = 1;
	}

	int saveSlotMaxLen = ((_screen->getScreenDim(8))->w << 3)  - _screen->getCharWidth('W');

	for (int i = startSlot; i < num && _savegameOffset + i - slotOffs < _savegameListSize; ++i) {
		if (_savegameList[i + _savegameOffset - slotOffs]) {
			Common::strlcpy(s, _savegameList[i + _savegameOffset - slotOffs], 80);

			// Trim long GMM save descriptions to fit our save slots
			int fC = _screen->getTextWidth(s);
			while (s[0] && fC >= saveSlotMaxLen) {
				s[strlen(s) - 1]  = 0;
				fC = _screen->getTextWidth(s);
			}

			menu.item[i].itemString = s;
			s += (strlen(s) + 1);
			menu.item[i].saveSlot = _saveSlots[i + _savegameOffset - slotOffs];
			menu.item[i].enabled = true;
		}
	}

	if (_savegameOffset == 0) {
		if (&menu == &_saveMenu) {
			strcpy(s, _vm->getLangString(0x4010));
			menu.item[0].itemString = s;
			menu.item[0].saveSlot = -3;
			menu.item[0].enabled = true;
		}
	}
}

void GUI_LoL::sortSaveSlots() {
	Common::sort(_saveSlots.begin(), _saveSlots.end(), Common::Greater<int>());
}

void GUI_LoL::printMenuText(const char *str, int x, int y, uint8 c0, uint8 c1, uint8 flags) {
	_screen->fprintString("%s", x, y, c0, c1, _vm->gameFlags().use16ColorMode ? (flags & 3) : flags , str);
}

int GUI_LoL::getMenuCenterStringX(const char *str, int x1, int x2) {
	if (!str)
		return 0;

	int strWidth = _screen->getTextWidth(str);
	int w = x2 - x1 + 1;
	return x1 + (w - strWidth) / 2;
}

int GUI_LoL::getInput() {
	if (!_displayMenu)
		return 0;

#ifdef ENABLE_KEYMAPPER
	Common::Keymapper *const keymapper = _vm->getEventManager()->getKeymapper();
	keymapper->pushKeymap(Common::kGlobalKeymapName);
#endif

	Common::Point p = _vm->getMousePos();
	_vm->_mouseX = p.x;
	_vm->_mouseY = p.y;

	if (_currentMenu == &_savenameMenu) {
		_vm->updateInput();

		for (Common::List<LoLEngine::Event>::const_iterator evt = _vm->_eventList.begin(); evt != _vm->_eventList.end(); ++evt) {
			if (evt->event.type == Common::EVENT_KEYDOWN)
				_keyPressed = evt->event.kbd;
		}
	}

	int inputFlag = _vm->checkInput(_menuButtonList);

	if (_currentMenu == &_savenameMenu && _keyPressed.ascii) {
		char inputKey = _keyPressed.ascii;
		Util::convertISOToDOS(inputKey);

		if ((uint8)inputKey > 31 && (uint8)inputKey < (_vm->gameFlags().lang == Common::JA_JPN ? 128 : 226)) {
			_saveDescription[strlen(_saveDescription) + 1] = 0;
			_saveDescription[strlen(_saveDescription)] = inputKey;
			inputFlag |= 0x8000;
		} else if (_keyPressed.keycode == Common::KEYCODE_BACKSPACE && strlen(_saveDescription)) {
			_saveDescription[strlen(_saveDescription) - 1] = 0;
			inputFlag |= 0x8000;
		}
	}

	_vm->removeInputTop();
	_keyPressed.reset();

	if (_vm->shouldQuit())
		_displayMenu = false;

	_vm->delay(8);

#ifdef ENABLE_KEYMAPPER
	keymapper->popKeymap(Common::kGlobalKeymapName);
#endif

	return inputFlag & 0x8000 ? 1 : 0;
}

int GUI_LoL::clickedMainMenu(Button *button) {
	updateMenuButton(button);
	switch (button->arg) {
	case 0x4001:
		_savegameOffset = 0;
		_newMenu = &_loadMenu;
		break;
	case 0x4002:
		_savegameOffset = 0;
		_newMenu = &_saveMenu;
		break;
	case 0x4003:
		_savegameOffset = 0;
		_newMenu = &_deleteMenu;
		break;
	case 0x4004:
		_newMenu = &_gameOptions;
		break;
	case 0x42D9:
		_newMenu = &_audioOptions;
		break;
	case 0x4006:
		_choiceMenu.menuNameId = 0x400a;
		_newMenu = &_choiceMenu;
		break;
	case 0x4005:
		_displayMenu = false;
		break;
	}
	return 1;
}

int GUI_LoL::clickedLoadMenu(Button *button) {
	updateMenuButton(button);

	if (button->arg == 0x4011) {
		if (_currentMenu != _lastMenu)
			_newMenu = _lastMenu;
		else
			_menuResult = 0;
		return 1;
	}

	int16 s = (int16)button->arg;
	_vm->_gameToLoad = _loadMenu.item[-s - 2].saveSlot;
	_displayMenu = false;

	return 1;
}

int GUI_LoL::clickedSaveMenu(Button *button) {
	updateMenuButton(button);

	if (button->arg == 0x4011) {
		_newMenu = &_mainMenu;
		return 1;
	}

	_newMenu = &_savenameMenu;
	int16 s = (int16)button->arg;
	_menuResult = _saveMenu.item[-s - 2].saveSlot + 1;
	_saveDescription = (char *)_vm->_tempBuffer5120 + 1000;
	_saveDescription[0] = 0;
	if (_saveMenu.item[-s - 2].saveSlot != -3)
		strcpy(_saveDescription, _saveMenu.item[-s - 2].itemString);

	return 1;
}

int GUI_LoL::clickedDeleteMenu(Button *button) {
	updateMenuButton(button);

	if (button->arg == 0x4011) {
		_newMenu = &_mainMenu;
		return 1;
	}

	_choiceMenu.menuNameId = 0x400b;
	_newMenu = &_choiceMenu;
	int16 s = (int16)button->arg;
	_menuResult = _deleteMenu.item[-s - 2].saveSlot + 1;

	return 1;
}

int GUI_LoL::clickedOptionsMenu(Button *button) {
	updateMenuButton(button);

	switch (button->arg) {
	case 0xfff9:
		_vm->_configMusic ^= 1;
		_vm->sound()->enableMusic(_vm->_configMusic);

		if (_vm->_configMusic)
			_vm->snd_playTrack(_vm->_curMusicTheme);
		else
			_vm->_sound->beginFadeOut();
		break;
	case 0xfff8:
		_vm->_configSounds ^= true;
		_vm->sound()->enableSFX(_vm->_configSounds);
		break;
	case 0xfff7:
		_vm->_monsterDifficulty = (_vm->_monsterDifficulty + 1) % 3;
		break;
	case 0xfff6:
		_vm->_smoothScrollingEnabled ^= true;
		break;
	case 0xfff5:
		_vm->_floatingCursorsEnabled ^= true;
		break;
	case 0xfff4:
		_vm->_lang = (_vm->_lang + 1) % 3;
		break;
	case 0xfff3:
		_vm->_configVoice ^= 3;
		break;
	case 0x4072: {
		Common::String filename;
		filename = Common::String::format("LEVEL%02d.%s", _vm->_currentLevel, _vm->_languageExt[_vm->_lang]);
		delete[] _vm->_levelLangFile;
		_vm->_levelLangFile = _vm->resource()->fileData(filename.c_str(), 0);
		filename = Common::String::format("LANDS.%s", _vm->_languageExt[_vm->_lang]);
		delete[] _vm->_landsFile;
		_vm->_landsFile = _vm->resource()->fileData(filename.c_str(), 0);
		_newMenu = _lastMenu;
	} break;
	default:
		// TODO: Is there anything we should do if we hit this case?
		break;
	}

	return 1;
}

int GUI_LoL::clickedAudioMenu(Button *button) {
	updateMenuButton(button);

	if (button->arg == 0x4072) {
		_newMenu = _lastMenu;
		return 1;
	}

	int tX = button->x;
	const int oldVolume = _vm->getVolume((KyraEngine_v1::kVolumeEntry)(button->arg - 3));
	int newVolume = oldVolume;

	if (button->index == 0) {
		newVolume -= 10;
		tX += 10;
	} else if (button->index == 1) {
		newVolume = _vm->_mouseX - (tX + 7);
	} else if (button->index == 2) {
		newVolume += 10;
		tX -= 114;
	}

	newVolume = CLIP(newVolume, 2, 102);

	if (newVolume == oldVolume) {
		_screen->updateScreen();
		return 0;
	}

	_screen->drawShape(0, _vm->_gameShapes[87], tX + oldVolume, button->y, 0, 0x10);
	// Temporary HACK
	const int volumeDrawX = _vm->convertVolumeFromMixer(_vm->convertVolumeToMixer(newVolume));
	_screen->drawShape(0, _vm->_gameShapes[86], tX + volumeDrawX, button->y, 0, 0x10);
	_screen->updateScreen();

	_vm->snd_stopSpeech(0);

	_vm->setVolume((KyraEngine_v1::kVolumeEntry)(button->arg - 3), newVolume);

	if (newVolume) {
		if (button->arg == 4) {
			_vm->snd_playSoundEffect(_sliderSfx, -1);
			int16 vocIndex = (int16)READ_LE_UINT16(&_vm->_ingameSoundIndex[_sliderSfx * 2]);
			do {
				++_sliderSfx;
				if (_sliderSfx < 47)
					_sliderSfx++;
				if (vocIndex == 199)
					_sliderSfx = 11;
				vocIndex = (int16)READ_LE_UINT16(&_vm->_ingameSoundIndex[_sliderSfx * 2]);
				if (vocIndex == -1)
					continue;
				if (!scumm_stricmp(_vm->_ingameSoundList[vocIndex], "EMPTY"))
					continue;
				break;
			} while (1);
		} else if (button->arg == 5) {
			_vm->_lastSpeechId = -1;
			_vm->snd_playCharacterSpeech(0x42e0, 0, 0);
		}
	}

	return 1;
}

int GUI_LoL::clickedDeathMenu(Button *button) {
	updateMenuButton(button);
	if (button->arg == _deathMenu.item[0].itemId) {
		_vm->quitGame();
	} else if (button->arg == _deathMenu.item[1].itemId) {
		_newMenu = &_loadMenu;
	}
	return 1;
}

int GUI_LoL::clickedSavenameMenu(Button *button) {
	updateMenuButton(button);
	if (button->arg == _savenameMenu.item[0].itemId) {

		Util::convertDOSToISO(_saveDescription);

		int slot = _menuResult == -2 ? getNextSavegameSlot() : _menuResult - 1;
		Graphics::Surface thumb;
		createScreenThumbnail(thumb);
		_vm->saveGameStateIntern(slot, _saveDescription, &thumb);
		thumb.free();

		_displayMenu = false;

	} else if (button->arg == _savenameMenu.item[1].itemId) {
		_newMenu = &_saveMenu;
	}

	return 1;
}

int GUI_LoL::clickedChoiceMenu(Button *button) {
	updateMenuButton(button);
	if (button->arg == _choiceMenu.item[0].itemId) {
		if (_lastMenu == &_mainMenu) {
			_vm->quitGame();
		} else if (_lastMenu == &_deleteMenu) {
			_vm->_saveFileMan->removeSavefile(_vm->getSavegameFilename(_menuResult - 1));
			Common::Array<int>::iterator i = Common::find(_saveSlots.begin(), _saveSlots.end(), _menuResult - 1);
			while (i != _saveSlots.begin()) {
				--i;
				// not rename quicksave slot filenames
				if (*i >= 990)
					break;
				Common::String oldName = _vm->getSavegameFilename(*i);
				Common::String newName = _vm->getSavegameFilename(*i - 1);
				_vm->_saveFileMan->renameSavefile(oldName, newName);
			}
			_newMenu = &_mainMenu;
			_saveSlotsListUpdateNeeded = true;
		}
	} else if (button->arg == _choiceMenu.item[1].itemId) {
		_newMenu = &_mainMenu;
	}
	return 1;
}

int GUI_LoL::scrollUp(Button *button) {
	if (!_scrollUpButton.data0ShapePtr)
		return 0;

	updateButton(button);
	if (_savegameOffset > 0) {
		_savegameOffset--;
		_newMenu = _currentMenu;
		_menuResult = -1;
	}
	return 1;
}

int GUI_LoL::scrollDown(Button *button) {
	if (!_scrollDownButton.data0ShapePtr)
		return 0;

	updateButton(button);
	if ((uint)_savegameOffset < _saveSlots.size() - (_currentMenu == &_saveMenu ? 3 : 4)) {
		_savegameOffset++;
		_newMenu = _currentMenu;
		_menuResult = -1;
	}
	return 1;
}

const char *GUI_LoL::getMenuTitle(const Menu &menu) {
	if (!menu.menuNameId)
		return 0;
	return _vm->getLangString(menu.menuNameId);
}

const char *GUI_LoL::getMenuItemTitle(const MenuItem &menuItem) {
	if (menuItem.itemId & 0x8000 && menuItem.itemString)
		return menuItem.itemString;
	else if (menuItem.itemId & 0x8000 || !menuItem.itemId)
		return 0;
	return _vm->getLangString(menuItem.itemId);
}

const char *GUI_LoL::getMenuItemLabel(const MenuItem &menuItem) {
	if (menuItem.labelId & 0x8000 && menuItem.labelString)
		return menuItem.labelString;
	else if (menuItem.labelId & 0x8000 || !menuItem.labelId)
		return 0;
	return _vm->getLangString(menuItem.labelId);
}

} // End of namespace Kyra

#endif // ENABLE_LOL
