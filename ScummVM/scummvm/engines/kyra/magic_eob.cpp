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

#include "kyra/eobcommon.h"
#include "kyra/resource.h"
#include "common/system.h"

namespace Kyra {

void EoBCoreEngine::useMagicBookOrSymbol(int charIndex, int type) {
	EoBCharacter *c = &_characters[charIndex];
	_openBookSpellLevel = c->slotStatus[3];
	_openBookSpellSelectedItem = c->slotStatus[2];
	_openBookSpellListOffset = c->slotStatus[4];
	_openBookChar = charIndex;
	_openBookType = type;
	_openBookSpellList = (type == 1) ? _clericSpellList : _mageSpellList;
	_openBookAvailableSpells = (type == 1) ? c->clericSpells : c->mageSpells;
	int8 *tmp = _openBookAvailableSpells + _openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem;

	if (*tmp <= 0) {
		for (bool loop = true; loop && _openBookSpellSelectedItem < 10;) {
			tmp = _openBookAvailableSpells + _openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem;
			if (*tmp > 0) {
				if (_openBookSpellSelectedItem > 5) {
					_openBookSpellListOffset = 6;
					_openBookSpellSelectedItem -= 6;
				}
				loop = false;
			} else {
				_openBookSpellSelectedItem++;
			}
		}

		if (_openBookSpellSelectedItem == 10) {
			_openBookSpellListOffset = 0;
			_openBookSpellSelectedItem = 6;
		}
	}

	if (!_updateFlags)
		_screen->copyRegion(64, 121, 0, 0, 112, 56, 0, 10, Screen::CR_NO_P_CHECK);
	_updateFlags = 1;
	gui_setPlayFieldButtons();
	gui_drawSpellbook();
}

void EoBCoreEngine::useMagicScroll(int charIndex, int type, int weaponSlot) {
	_openBookCharBackup = _openBookChar;
	_openBookTypeBackup = _openBookType;
	_castScrollSlot = weaponSlot + 1;
	_openBookChar = charIndex;
	_openBookType = type <= _clericSpellOffset ? 0 : 1;
	castSpell(type, weaponSlot);
}

void EoBCoreEngine::usePotion(int charIndex, int weaponSlot) {
	EoBCharacter *c = &_characters[charIndex];

	int val = deleteInventoryItem(charIndex, weaponSlot);
	snd_playSoundEffect(10);

	if (_flags.gameID == GI_EOB1)
		val--;

	switch (val) {
	case 0:
		sparkEffectDefensive(charIndex);
		c->strengthCur = 22;
		c->strengthExtCur = 0;
		setCharEventTimer(charIndex, 546 * rollDice(1, 4, 4), 7, 1);
		break;

	case 1:
		sparkEffectDefensive(charIndex);
		modifyCharacterHitpoints(charIndex, rollDice(2, 4, 2));
		break;

	case 2:
		sparkEffectDefensive(charIndex);
		modifyCharacterHitpoints(charIndex, rollDice(3, 8, 3));
		break;

	case 3:
		statusAttack(charIndex, 2, _potionStrings[0], 0, 1, 8, 1);
		c->effectFlags &= ~0x2000;
		if (c->flags & 2)
			return;
		break;

	case 4:
		sparkEffectDefensive(charIndex);
		c->food = 100;
		if (_currentControlMode)
			gui_drawCharPortraitWithStats(charIndex);
		break;

	case 5:
		sparkEffectDefensive(charIndex);
		c->effectFlags |= 0x10000;
		setCharEventTimer(charIndex, 546 * rollDice(1, 4, 4), 12, 1);
		snd_playSoundEffect(100);
		gui_drawCharPortraitWithStats(charIndex);
		break;

	case 6:
		sparkEffectDefensive(charIndex);
		c->effectFlags |= 0x40;
		gui_drawCharPortraitWithStats(charIndex);
		break;

	case 7:
		sparkEffectDefensive(charIndex);
		neutralizePoison(charIndex);
		break;

	default:
		break;
	}

	_txt->printMessage(_potionStrings[1], -1, c->name, _potionEffectStrings[val]);
}

void EoBCoreEngine::useWand(int charIndex, int weaponSlot) {
	int v = _items[_characters[charIndex].inventory[weaponSlot]].value;
	if (!v) {
		_txt->printMessage(_wandStrings[0]);
		return;
	}

	if (v != 5)
		useMagicScroll(charIndex, _wandTypes[v], weaponSlot);
	else if (_flags.gameID == GI_EOB2)
		useMagicScroll(charIndex, 64, weaponSlot);
	else {
		uint16 bl1 = calcNewBlockPosition(_currentBlock, _currentDirection);
		uint16 bl2 = calcNewBlockPosition(bl1, _currentDirection);
		snd_playSoundEffect(98);
		sparkEffectOffensive();

		if ((_wllWallFlags[_levelBlockProperties[bl2].walls[_currentDirection ^ 2]] & 4) && !(_levelBlockProperties[bl2].flags & 7) && (_levelBlockProperties[bl1].flags & 7)) {
			for (int i = 0; i < 30; i++) {
				if (_monsters[i].block != bl1)
					continue;
				placeMonster(&_monsters[i], bl2, -1);
				_sceneUpdateRequired = true;
			}
		} else {
			_txt->printMessage(_wandStrings[1]);
		}
	}
}

void EoBCoreEngine::castSpell(int spell, int weaponSlot) {
	EoBSpell *s = &_spells[spell];
	EoBCharacter *c = &_characters[_openBookChar];
	_activeSpell = spell;

	if ((s->flags & 0x100) && (c->effectFlags & 0x40))
		// remove invisibility effect
		removeCharacterEffect(10, _openBookChar, 1);

	int ci = _openBookChar;
	if (ci > 3)
		ci -= 2;

	_activeSpellCharacterPos = _dropItemDirIndex[(_currentDirection << 2) + ci];

	if (s->flags & 0x400) {
		if (c->inventory[0] && c->inventory[1]) {
			printWarning(_magicStrings1[2]);
			return;
		}

		if (isMagicEffectItem(c->inventory[0]) || isMagicEffectItem(c->inventory[1])) {
			printWarning(_magicStrings1[3]);
			return;
		}
	}

	if (!(_flags.gameID == GI_EOB2 && _activeSpell == 62)) {
		if (!_castScrollSlot) {
			int8 tmp = _openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem];
			if (_openBookSpellListOffset + _openBookSpellSelectedItem < 8)
				memmove(&_openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem], &_openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem + 1], 8 - (_openBookSpellListOffset + _openBookSpellSelectedItem));
			_openBookAvailableSpells[_openBookSpellLevel * 10 + 8] = -tmp;
			if (_openBookAvailableSpells[_openBookSpellLevel * 10 + _openBookSpellListOffset + _openBookSpellSelectedItem] < 0) {
				if (--_openBookSpellSelectedItem == -1) {
					if (_openBookSpellListOffset) {
						_openBookSpellListOffset = 0;
						_openBookSpellSelectedItem = 5;
					} else {
						_openBookSpellSelectedItem = 6;
					}
				}
			}
		} else if (weaponSlot != -1) {
			updateUsedCharacterHandItem(_openBookChar, weaponSlot);
		}
	}

	_txt->printMessage(_magicStrings1[4], -1, c->name, s->name);

	if (s->flags & 0x20) {
		castOnWhomDialogue();
		return;
	}

	_activeSpellCharId = _openBookChar;
	startSpell(spell);
}

void EoBCoreEngine::removeCharacterEffect(int spell, int charIndex, int showWarning) {
	assert(spell >= 0);
	EoBCharacter *c = &_characters[charIndex];
	EoBSpell *s = &_spells[spell];

	if (showWarning) {
		int od = _screen->curDimIndex();
		Screen::FontId of = _screen->setFont(Screen::FID_6_FNT);
		_screen->setScreenDim(7);
		printWarning(Common::String::format(_magicStrings3[_flags.gameID == GI_EOB1 ? 3 : 2], c->name, s->name).c_str());
		_screen->setScreenDim(od);
		_screen->setFont(of);
	}

	if (s->endCallback)
		(this->*s->endCallback)(c);

	if (s->flags & 1)
		c->effectFlags &= ~s->effectFlags;

	if (s->flags & 4)
		_partyEffectFlags &= ~s->effectFlags;

	if (s->flags & 0x200) {
		for (int i = 0; i < 6; i++) {
			if (!testCharacter(i, 1))
				continue;
			if (!testCharacter(i, 2) && !(s->flags & 0x800))
				continue;
			_characters[i].effectFlags &= ~s->effectFlags;
		}
	}

	if (s->flags & 0x2)
		recalcArmorClass(_activeSpellCharId);

	if (showWarning) {
		if (s->flags & 0x20A0)
			gui_drawCharPortraitWithStats(charIndex);
		else if (s->flags & 0x40)
			gui_drawAllCharPortraitsWithStats();
	}
}

void EoBCoreEngine::removeAllCharacterEffects(int charIndex) {
	EoBCharacter *c = &_characters[charIndex];
	c->effectFlags = 0;
	memset(c->effectsRemainder, 0, 4);

	for (int i = 0; i < 10; i++) {
		if (c->events[i] < 0) {
			removeCharacterEffect(-c->events[i], charIndex, 0);
			c->timers[i] = 0;
			c->events[i] = 0;
		}
	}

	setupCharacterTimers();
	recalcArmorClass(charIndex);
	c->disabledSlots = 0;
	c->slotStatus[0] = c->slotStatus[1] = 0;
	c->damageTaken = 0;
	c->strengthCur = c->strengthMax;
	c->strengthExtCur = c->strengthExtMax;
	gui_drawAllCharPortraitsWithStats();
}

void EoBCoreEngine::castOnWhomDialogue() {
	printWarning(_magicStrings3[0]);
	gui_setCastOnWhomButtons();
}

void EoBCoreEngine::startSpell(int spell) {
	EoBSpell *s = &_spells[spell];
	EoBCharacter *c = &_characters[_activeSpellCharId];
	snd_playSoundEffect(s->sound);

	if (s->flags & 0xa0)
		sparkEffectDefensive(_activeSpellCharId);
	else if (s->flags & 0x40)
		sparkEffectDefensive(-1);
	else if (s->flags & 0x1000)
		sparkEffectOffensive();

	if (s->flags & 0x20) {
		_txt->printMessage(c->name);
		_txt->printMessage(_flags.gameID == GI_EOB1 ? _magicStrings3[1] : _magicStrings1[5]);
	}

	if ((s->flags & 0x30) && (s->effectFlags & c->effectFlags)) {
		if (_flags.gameID == GI_EOB2)
			printWarning(Common::String::format(_magicStrings7[0], c->name, s->name).c_str());
	} else if ((s->flags & 0x50) && (s->effectFlags & _partyEffectFlags)) {
		if (_flags.gameID == GI_EOB1 && s->effectFlags == 0x400)
			// EOB 1 only warns in case of a bless spell
			printWarning(_magicStrings8[1]);
		else
			printWarning(Common::String::format(_magicStrings7[1], s->name).c_str());
	} else {
		if (s->flags & 8)
			setSpellEventTimer(spell, s->timingPara[0], s->timingPara[1], s->timingPara[2], s->timingPara[3]);

		_returnAfterSpellCallback = false;
		if (s->startCallback)
			(this->*s->startCallback)();
		if (_returnAfterSpellCallback)
			return;

		if (s->flags & 1)
			c->effectFlags |= s->effectFlags;
		if (s->flags & 4)
			_partyEffectFlags |= s->effectFlags;

		if (s->flags & 0x200) {
			for (int i = 0; i < 6; i++) {
				if (!testCharacter(i, 1))
					continue;
				if (!testCharacter(i, 2) && !(s->flags & 0x800))
					continue;
				_characters[i].effectFlags |= s->effectFlags;
			}
		}

		if (s->flags & 2)
			recalcArmorClass(_activeSpellCharId);

		if (s->flags & 0x20A0)
			gui_drawCharPortraitWithStats(_activeSpellCharId);
		if (s->flags & 0x40)
			gui_drawAllCharPortraitsWithStats();
	}

	if (_castScrollSlot) {
		gui_updateSlotAfterScrollUse();
	} else {
		_characters[_openBookChar].disabledSlots |= 4;
		setCharEventTimer(_openBookChar, 72, 11, 1);
		gui_toggleButtons();
		gui_drawSpellbook();
	}

	if (_flags.gameID == GI_EOB2) {
		//_castSpellWd1 = spell;
		runLevelScript(_currentBlock, 0x800);
		//_castSpellWd1 = 0;
	}
}

void EoBCoreEngine::sparkEffectDefensive(int charIndex) {
	int first = charIndex;
	int last = charIndex;
	if (charIndex == -1) {
		first = 0;
		last = 5;
	}

	for (int i = 0; i < 8; i++) {
		for (int ii = first; ii <= last; ii++) {
			if (!testCharacter(ii, 1) || (_currentControlMode && ii != _updateCharNum))
				continue;

			gui_drawCharPortraitWithStats(ii);

			for (int iii = 0; iii < 4; iii++) {
				int shpIndex = ((_sparkEffectDefSteps[i] & _sparkEffectDefSubSteps[iii]) >> _sparkEffectDefShift[iii]);
				if (!shpIndex)
					continue;
				int x = _sparkEffectDefAdd[iii * 2] - 8;
				int y = _sparkEffectDefAdd[iii * 2 + 1];
				if (_currentControlMode) {
					x += 181;
					y += 3;
				} else {
					x += (_sparkEffectDefX[ii] << 3);
					y += _sparkEffectDefY[ii];
				}
				_screen->drawShape(0, _sparkShapes[shpIndex - 1], x, y, 0);
				_screen->updateScreen();
			}
		}
		delay(2 * _tickLength);
	}

	for (int i = first; i < last; i++)
		gui_drawCharPortraitWithStats(i);
}

void EoBCoreEngine::sparkEffectOffensive() {
	disableSysTimer(2);
	_screen->copyRegion(0, 0, 0, 0, 176, 120, 0, 2, Screen::CR_NO_P_CHECK);

	for (int i = 0; i < 16; i++)
		_screen->copyRegionToBuffer(0, _sparkEffectOfX[i], _sparkEffectOfY[i], 16, 16, &_spellAnimBuffer[i << 8]);
	_screen->updateScreen();

	for (int i = 0; i < 11; i++) {
		for (int ii = 0; ii < 16; ii++)
			_screen->copyBlockToPage(2, _sparkEffectOfX[ii], _sparkEffectOfY[ii], 16, 16, &_spellAnimBuffer[ii << 8]);

		for (int ii = 0; ii < 16; ii++) {
			int shpIndex = (_sparkEffectOfFlags1[i] & _sparkEffectOfFlags2[ii]) >> _sparkEffectOfShift[ii];
			if (shpIndex)
				_screen->drawShape(2, _sparkShapes[shpIndex - 1], _sparkEffectOfX[ii], _sparkEffectOfY[ii], 0);
		}
		delay(2 * _tickLength);
		_screen->copyRegion(0, 0, 0, 0, 176, 120, 2, 0, Screen::CR_NO_P_CHECK);
		_screen->updateScreen();
	}

	for (int i = 0; i < 16; i++)
		_screen->copyBlockToPage(0, _sparkEffectOfX[i], _sparkEffectOfY[i], 16, 16, &_spellAnimBuffer[i << 8]);

	_screen->updateScreen();
	enableSysTimer(2);
}

void EoBCoreEngine::setSpellEventTimer(int spell, int timerBaseFactor, int timerLength, int timerLevelFactor, int updateExistingTimer) {
	assert(spell >= 0);
	int l = _openBookType == 1 ? getClericPaladinLevel(_openBookChar) : getMageLevel(_openBookChar);
	uint32 countdown = timerLength * timerBaseFactor + timerLength * l * timerLevelFactor;
	setCharEventTimer(_activeSpellCharId, countdown, -spell, updateExistingTimer);
}

void EoBCoreEngine::sortCharacterSpellList(int charIndex) {
	int8 *list = _characters[charIndex].mageSpells;

	for (int i = 0; i < 16;) {
		bool p = false;
		for (int ii = 0; ii < 9; ii++) {
			int8 *pos = &list[ii];

			int s1 = pos[0];
			int s2 = pos[1];

			if (s1 == 0)
				s1 = 80;
			else if (s1 < 0)
				s1 = s1 * -1 + 40;

			if (s2 == 0)
				s2 = 80;
			else if (s2 < 0)
				s2 = s2 * -1 + 40;

			if (s1 > s2) {
				SWAP(pos[0], pos[1]);
				p = true;
			}
		}

		if (p)
			continue;

		list += 10;
		if (++i == 8)
			list = _characters[charIndex].clericSpells;
	}
}

bool EoBCoreEngine::magicObjectDamageHit(EoBFlyingObject *fo, int dcTimes, int dcPips, int dcOffs, int level) {
	int ignoreAttackerId = fo->flags & 0x10;
	int singleTargetCheckAdjacent = fo->flags & 1;
	int blockDamage = fo->flags & 2;
	int hitTest = fo->flags & 4;

	int savingThrowType = 5;
	int savingThrowEffect = 3;
	if (fo->flags & 8) {
		savingThrowType = 4;
		savingThrowEffect = 0;
	}

	int dmgFlag = _spells[fo->callBackIndex].damageFlags;
	if (fo->attackerId >= 0)
		dmgFlag |= 0x800;

	bool res = false;
	if (!level)
		level = 1;

	if ((_levelBlockProperties[fo->curBlock].flags & 7) && (fo->attackerId >= 0 || ignoreAttackerId)) {
		_preventMonsterFlash = true;

		for (const int16 *m = findBlockMonsters(fo->curBlock, fo->curPos, fo->direction, blockDamage, singleTargetCheckAdjacent); *m != -1; m++) {
			int dmg = rollDice(dcTimes, dcPips, dcOffs) * level;

			if (hitTest) {
				if (!characterAttackHitTest(fo->attackerId, *m, 0, 0))
					continue;
			}

			calcAndInflictMonsterDamage(&_monsters[*m], 0, 0, dmg, dmgFlag, savingThrowType, savingThrowEffect);
			res = true;
		}
		updateAllMonsterShapes();

	} else if (fo->curBlock == _currentBlock && (fo->attackerId < 0 || ignoreAttackerId)) {
		if (blockDamage) {
			for (int i = 0; i < 6; i++) {
				if (!testCharacter(i, 1))
					continue;
				if (hitTest && !monsterAttackHitTest(&_monsters[0], i))
					continue;

				int dmg = rollDice(dcTimes, dcPips, dcOffs) * level;
				res = true;

				calcAndInflictCharacterDamage(i, 0, 0, dmg, dmgFlag, savingThrowType, savingThrowEffect);
			}
		} else {
			int c = _dscItemPosIndex[(_currentDirection << 2) + (fo->curPos & 3)];
			if ((c > 2) && (testCharacter(4, 1) || testCharacter(5, 1)) && rollDice(1, 2, -1))
				c += 2;

			if (!fo->item && (_characters[c].effectFlags & 8)) {
				res = true;
			} else {
				if ((_characters[c].flags & 1) && (!hitTest || monsterAttackHitTest(&_monsters[0], c))) {
					int dmg = rollDice(dcTimes, dcPips, dcOffs) * level;
					res = true;
					calcAndInflictCharacterDamage(c, 0, 0, dmg, dmgFlag, savingThrowType, savingThrowEffect);
				}
			}
		}
	}

	if (res && (fo->flags & 0x40))
		explodeObject(fo, fo->curBlock, fo->item);
	else if ((_flags.gameID == GI_EOB1 && fo->item == 5) || (_flags.gameID == GI_EOB2 && fo->item == 4))
		res = false;

	return res;
}

bool EoBCoreEngine::magicObjectStatusHit(EoBMonsterInPlay *m, int type, bool tryEvade, int mod) {
	EoBMonsterProperty *p = &_monsterProps[m->type];
	if (tryEvade) {
		if (tryMonsterAttackEvasion(m) || (p->immunityFlags & 0x10))
			return true;
	}

	if (trySavingThrow(m, 0, p->level, mod, 6))
		return false;

	int para = 0;

	switch (type) {
	case 0:
	case 1:
	case 2:
		para = (type == 0) ? ((p->typeFlags & 1) ? 1 : 0) : ((type == 1) ? ((p->typeFlags & 2) ? 1 : 0) : 1);
		if (para && !(p->immunityFlags & 2)) {
			m->mode = 10;
			m->spellStatusLeft = 15;
		}

		break;

	case 3:
		if (!(p->immunityFlags & 8))
			inflictMonsterDamage(m, 1000, true);
		break;

	case 4:
		inflictMonsterDamage(m, 1000, true);
		break;

	case 5:
		m->flags |= 0x20;
		_sceneUpdateRequired = true;
		break;

	case 6:
		if (!(_flags.gameID == GI_EOB1 && !(p->typeFlags & 3)) && !(p->immunityFlags & 4) && m->mode != 7 && m->mode != 8 && m->mode != 10) {
			m->mode = 0;
			m->spellStatusLeft = 20;
			m->flags |= 8;
			walkMonsterNextStep(m, -1, (getNextMonsterDirection(m->block, _currentBlock) ^ 4) >> 1);
		}
		break;

	default:
		break;
	}

	return true;
}

bool EoBCoreEngine::turnUndeadHit(EoBMonsterInPlay *m, int hitChance, int casterLevel) {
	assert(_monsterProps[m->type].tuResist > 0);
	uint8 e = _turnUndeadEffect[_monsterProps[m->type].tuResist * 14 + MIN(casterLevel, 14)];

	if (e == 0xff) {
		calcAndInflictMonsterDamage(m, 0, 0, 500, 0x200, 5, 3);
	} else if (hitChance < e) {
		return false;
	} else {
		m->mode = 0;
		m->flags |= 8;
		m->spellStatusLeft = 40;
		m->dir = (getNextMonsterDirection(m->block, _currentBlock) ^ 4) >> 1;
	}

	return true;
}

int EoBCoreEngine::getMagicWeaponSlot(int charIndex) {
	return _characters[charIndex].inventory[1] ? 0 : 1;
}

void EoBCoreEngine::causeWounds(int dcTimes, int dcPips, int dcOffs) {
	if (_openBookChar == 0 || _openBookChar == 1) {
		int d = getClosestMonster(_openBookChar, calcNewBlockPosition(_currentBlock, _currentDirection));
		if (d != -1) {
			if (!characterAttackHitTest(_openBookChar, d, 0, 1))
				return;

			if (dcTimes == -1) {
				dcOffs = _monsters[d].hitPointsMax - rollDice(1, 4);
				dcTimes = dcPips = 0;
			}
			calcAndInflictMonsterDamage(&_monsters[d], dcTimes, dcPips, dcOffs, 0x801, 4, 2);
		} else {
			printWarning(Common::String::format(_magicStrings3[_flags.gameID == GI_EOB1 ? 4 : 3], _characters[_openBookChar].name).c_str());
		}
	} else {
		printWarning(Common::String::format(_magicStrings3[_flags.gameID == GI_EOB1 ? 5 : 4], _characters[_openBookChar].name).c_str());
	}
}

int EoBCoreEngine::createMagicWeaponType(int invFlags, int handFlags, int armorClass, int allowedClasses, int dmgNumDice, int dmgPips, int dmgInc, int extraProps) {
	int i = 51;
	for (; i < 57; i++) {
		if (_itemTypes[i].armorClass == -30)
			break;
	}

	if (i == 57)
		return -1;

	EoBItemType *tp = &_itemTypes[i];
	tp->invFlags = invFlags;
	tp->requiredHands = 0;
	tp->handFlags = handFlags;
	tp->armorClass = armorClass;
	tp->allowedClasses = allowedClasses;
	tp->dmgNumDiceL = tp->dmgNumDiceS = dmgNumDice;
	tp->dmgNumPipsL = tp->dmgNumPipsS = dmgPips;
	tp->dmgIncL = tp->dmgIncS = dmgInc;
	tp->extraProperties = extraProps;

	return i;
}

Item EoBCoreEngine::createMagicWeaponItem(int flags, int icon, int value, int type) {
	Item i = 11;
	for (; i < 17; i++) {
		if (_items[i].block == -2)
			break;
	}

	if (i == 17)
		return -1;

	EoBItem *itm = &_items[i];
	itm->flags = 0x20 | flags;
	itm->icon = icon;
	itm->value = value;
	itm->type = type;
	itm->pos = 0;
	itm->block = 0;
	itm->nameId = itm->nameUnid = 0;
	itm->prev = itm->next = 0;

	return i;
}

void EoBCoreEngine::removeMagicWeaponItem(Item item) {
	_itemTypes[_items[item].type].armorClass = -30;
	_items[item].block = -2;
	_items[item].level = 0xff;
}

void EoBCoreEngine::updateWallOfForceTimers() {
	uint32 ct = _system->getMillis();
	for (int i = 0; i < 5; i++) {
		if (!_wallsOfForce[i].block)
			continue;
		if (_wallsOfForce[i].duration < ct)
			destroyWallOfForce(i);
	}
}

void EoBCoreEngine::destroyWallOfForce(int index) {
	memset(_levelBlockProperties[_wallsOfForce[index].block].walls, 0, 4);
	_wallsOfForce[index].block = 0;
	_sceneUpdateRequired = true;
}

int EoBCoreEngine::findSingleSpellTarget(int dist) {
	uint16 bl = _currentBlock;
	int res = -1;

	for (int i = 0; i < dist && res == -1; i++) {
		bl = calcNewBlockPosition(bl, _currentDirection);
		res = getClosestMonster(_openBookChar, bl);
		if (!(_wllWallFlags[_levelBlockProperties[bl].walls[_sceneDrawVarDown]] & 1)) {
			i = dist;
			res = -1;
		}
	}

	return res;
}

int EoBCoreEngine::findFirstCharacterSpellTarget() {
	int curCharIndex = rollDice(1, 6, -1);
	for (_characterSpellTarget = 0; _characterSpellTarget < 6; _characterSpellTarget++) {
		if (testCharacter(curCharIndex, 3))
			return curCharIndex;
		if (++curCharIndex == 6)
			curCharIndex = 0;
	}
	return -1;
}

int EoBCoreEngine::findNextCharacterSpellTarget(int curCharIndex) {
	for (_characterSpellTarget++; _characterSpellTarget < 6;) {
		if (++curCharIndex == 6)
			curCharIndex = 0;
		if (testCharacter(curCharIndex, 3))
			return curCharIndex;
	}
	return -1;
}

int EoBCoreEngine::charDeathSavingThrow(int charIndex, int div) {
	bool _beholderOrgBhv = true;
	// Due to a bug in the original code the saving throw result is completely ignored
	// here. The Beholders' disintegrate spell will alway succeed while their flesh to
	// stone spell will always fail.
	if (_beholderOrgBhv)
		div >>= 1;
	else
		div = specialAttackSavingThrow(charIndex, 4) ? 1 : 0;
	return div;
}

void EoBCoreEngine::printWarning(const char *str) {
	_txt->printMessage(str);
	snd_playSoundEffect(79);
}

void EoBCoreEngine::printNoEffectWarning() {
	printWarning(_magicStrings4[0]);
}

void EoBCoreEngine::spellCallback_start_armor() {
	_characters[_activeSpellCharId].effectsRemainder[0] = getMageLevel(_openBookChar) + 8;
	if ((getDexterityArmorClassModifier(_characters[_activeSpellCharId].dexterityCur) + 6) >= _characters[_activeSpellCharId].armorClass)
		printWarning(Common::String::format(_magicStrings6[0], _characters[_activeSpellCharId].name).c_str());
}

void EoBCoreEngine::spellCallback_start_burningHands() {
	static const int16 bX[] = { 0, 152, 24, 120, 56, 88 };
	static const int8 bY[] = { 64, 64, 56, 56, 56, 56 };

	for (int i = 0; i < 6; i++)
		drawBlockObject(i & 1, 0, _firebeamShapes[(5 - i) >> 1], bX[i], bY[i], 0);
	_screen->updateScreen();
	delay(2 * _tickLength);

	int cl = getMageLevel(_openBookChar);
	int bl = calcNewBlockPosition(_currentBlock, _currentDirection);

	const int8 *pos = getMonstersOnBlockPositions(bl);
	_preventMonsterFlash = true;

	int numDest = (_flags.gameID == GI_EOB1) ? 2 : 6;
	const uint8 *d = &_burningHandsDest[_currentDirection * (_flags.gameID == GI_EOB1 ? 2 : 8)];

	for (int i = 0; i < numDest; i++, d++) {
		if (pos[*d] == -1)
			continue;
		calcAndInflictMonsterDamage(&_monsters[pos[*d]], 1, 3, cl << 1, 0x21, 4, 0);
	}

	updateAllMonsterShapes();
	_sceneUpdateRequired = true;
}

void EoBCoreEngine::spellCallback_start_detectMagic() {
	setHandItem(_itemInHand);
}

bool EoBCoreEngine::spellCallback_end_detectMagic(void *) {
	setHandItem(_itemInHand);
	return true;
}

void EoBCoreEngine::spellCallback_start_magicMissile() {
	launchMagicObject(_openBookChar, 0, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_magicMissile(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	return magicObjectDamageHit(fo, 1, 4, 1, (getMageLevel(fo->attackerId) - 1) >> 1);
}

void EoBCoreEngine::spellCallback_start_shockingGrasp() {
	int t = createMagicWeaponType(0, 0, 0, 0x0f, 1, 8, getMageLevel(_openBookChar), 1);
	Item i = (t != -1) ? createMagicWeaponItem(0x10, 82, 0, t) : -1;
	if (t == -1 || i == -1) {
		if (_flags.gameID == GI_EOB2)
			printWarning(_magicStrings8[0]);
		removeCharacterEffect(_activeSpell, _activeSpellCharId, 0);
		deleteCharEventTimer(_activeSpellCharId, -_activeSpell);
		_returnAfterSpellCallback = true;
	} else {
		_characters[_activeSpellCharId].inventory[getMagicWeaponSlot(_activeSpellCharId)] = i;
	}
}

bool EoBCoreEngine::spellCallback_end_shockingGraspFlameBlade(void *obj) {
	EoBCharacter *c = (EoBCharacter *)obj;
	for (int i = 0; i < 2; i++) {
		if (isMagicEffectItem(c->inventory[i])) {
			removeMagicWeaponItem(c->inventory[i]);
			c->inventory[i] = 0;
		}
	}
	return true;
}

void EoBCoreEngine::spellCallback_start_improvedIdentify() {
	for (int i = 0; i < 2; i++) {
		Item itm = _characters[_activeSpellCharId].inventory[i];
		if (itm)
			_items[itm].flags |= 0x40;
	}
}

void EoBCoreEngine::spellCallback_start_melfsAcidArrow() {
	launchMagicObject(_openBookChar, 1, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_melfsAcidArrow(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	assert(fo);
	return magicObjectDamageHit(fo, 2, 4, 0, getMageLevel(fo->attackerId) / 3);
}

void EoBCoreEngine::spellCallback_start_dispelMagic() {
	for (int i = 0; i < 6; i++) {
		if (testCharacter(i, 1))
			removeAllCharacterEffects(i);
	}
}

void EoBCoreEngine::spellCallback_start_fireball() {
	launchMagicObject(_openBookChar, 2, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_fireball(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	return magicObjectDamageHit(fo, 1, 6, 0, getMageLevel(fo->attackerId));
}

void EoBCoreEngine::spellCallback_start_flameArrow() {
	launchMagicObject(_openBookChar, 3, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_flameArrow(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	return magicObjectDamageHit(fo, 5, 6, 0, getMageLevel(fo->attackerId));
}

void EoBCoreEngine::spellCallback_start_holdPerson() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 4 : 3, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_holdPerson(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	bool res = false;

	if (_flags.gameID == GI_EOB2 && fo->curBlock == _currentBlock) {
		// party hit
		int numChar = rollDice(1, 4, 0);
		int charIndex = rollDice(1, 6, -1);
		for (int i = 0; i < 6 && numChar; i++) {
			if (testCharacter(charIndex, 3)) {
				statusAttack(charIndex, 4, _magicStrings8[1], 4, 5, 9, 1);
				numChar--;
			}
			charIndex = (charIndex + 1) % 6;
		}
		res = true;

	} else {
		// monster hit
		for (const int16 *m = findBlockMonsters(fo->curBlock, fo->curPos, fo->direction, 1, 1); *m != -1; m++)
			res |= magicObjectStatusHit(&_monsters[*m], 0, true, 4);
	}

	return res;
}

void EoBCoreEngine::spellCallback_start_lightningBolt() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 5 : 4, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_lightningBolt(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	return magicObjectDamageHit(fo, 1, 6, 0, getMageLevel(fo->attackerId));
}

void EoBCoreEngine::spellCallback_start_vampiricTouch() {
	int t = createMagicWeaponType(0, 0, 0, 0x0f, getMageLevel(_openBookChar) >> 1, 6, 0, 1);
	Item i = (t != -1) ? createMagicWeaponItem(0x18, 83, 0, t) : -1;
	if (t == -1 || i == -1) {
		if (_flags.gameID == GI_EOB2)
			printWarning(_magicStrings8[2]);
		removeCharacterEffect(_activeSpell, _activeSpellCharId, 0);
		deleteCharEventTimer(_activeSpellCharId, -_activeSpell);
		_returnAfterSpellCallback = true;
	} else {
		_characters[_activeSpellCharId].inventory[getMagicWeaponSlot(_activeSpellCharId)] = i;
	}
}

bool EoBCoreEngine::spellCallback_end_vampiricTouch(void *obj) {
	EoBCharacter *c = (EoBCharacter *)obj;
	if (c->hitPointsCur > c->hitPointsMax)
		c->hitPointsCur = c->hitPointsMax;
	spellCallback_end_shockingGraspFlameBlade(obj);
	return true;
}

void EoBCoreEngine::spellCallback_start_fear() {
	sparkEffectOffensive();
	uint16 bl = calcNewBlockPosition(_currentBlock, _currentDirection);
	for (int i = 0; i < 30; i++) {
		if (_monsters[i].block == bl)
			magicObjectStatusHit(&_monsters[i], 6, true, 4);
	}
}

void EoBCoreEngine::spellCallback_start_iceStorm() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 6 : 5, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_iceStorm(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	static int8 blockAdv[] = { -32, 32, 1, -1 };
	bool res = magicObjectDamageHit(fo, 1, 6, 0, getMageLevel(fo->attackerId));
	if (res) {
		for (int i = 0; i < 4; i++) {
			uint16 bl = fo->curBlock;
			fo->curBlock = (fo->curBlock + blockAdv[i]) & 0x3ff;
			magicObjectDamageHit(fo, 1, 6, 0, getMageLevel(fo->attackerId));
			fo->curBlock = bl;
		}
	}
	return res;
}

void EoBCoreEngine::spellCallback_start_stoneSkin() {
	_characters[_activeSpellCharId].effectsRemainder[1] = (getMageLevel(_openBookChar) >> 1) + rollDice(1, 4);
}

void EoBCoreEngine::spellCallback_start_removeCurse() {
	for (int i = 0; i < 27; i++) {
		Item itm = _characters[_activeSpellCharId].inventory[i];
		if (itm && (_items[itm].flags & 0x20) && !isMagicEffectItem(itm))
			_items[itm].flags = (_items[itm].flags & ~0x20) | 0x40;
	}
}

void EoBCoreEngine::spellCallback_start_coneOfCold() {
	const int8 *dirTables[] = { _coneOfColdDest1, _coneOfColdDest2, _coneOfColdDest3, _coneOfColdDest4 };

	int cl = getMageLevel(_openBookChar);

	_screen->setCurPage(2);
	_screen->fillRect(0, 0, 176, 120, 0);
	_screen->setGfxParameters(0, 0, _screen->getPagePixel(2, 0, 0));
	drawSceneShapes(7);
	_screen->setCurPage(0);
	disableSysTimer(2);
	_screen->drawVortex(150, 50, 10, 1, 100, _coneOfColdGfxTbl, _coneOfColdGfxTblSize);
	enableSysTimer(2);

	const int8 *tbl = dirTables[_currentDirection];
	_preventMonsterFlash = true;

	for (int i = 0; i < 7; i++) {
		for (const int16 *m = findBlockMonsters((_currentBlock + tbl[i]) & 0x3ff, 4, _currentDirection, 1, 1); *m != -1; m++)
			calcAndInflictMonsterDamage(&_monsters[*m], cl, 4, cl, 0x41, 5, 0);
	}

	updateAllMonsterShapes();
}

void EoBCoreEngine::spellCallback_start_holdMonster() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 7 : 6, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_holdMonster(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	bool res = false;
	for (const int16 *m = findBlockMonsters(fo->curBlock, fo->curPos, fo->direction, 1, 1); *m != -1; m++)
		res |= magicObjectStatusHit(&_monsters[*m], 1, true, 4);
	return res;
}

void EoBCoreEngine::spellCallback_start_wallOfForce() {
	uint16 bl = calcNewBlockPosition(_currentBlock, _currentDirection);
	LevelBlockProperty *l = &_levelBlockProperties[bl];
	if (l->walls[0] || l->walls[1] || l->walls[2] || l->walls[3] || (l->flags & 7)) {
		printWarning(_magicStrings8[3]);
		return;
	}

	uint32 dur = 0xffffffff;
	int s = 0;
	int i = 0;

	for (; i < 5; i++) {
		if (!_wallsOfForce[i].block)
			break;
		if (_wallsOfForce[i].duration < dur) {
			dur = _wallsOfForce[i].duration;
			s = i;
		}
	}

	if (i == 5)
		destroyWallOfForce(s);

	memset(_levelBlockProperties[bl].walls, 74, 4);
	_wallsOfForce[s].block = bl;
	_wallsOfForce[s].duration = _system->getMillis() + (((getMageLevel(_openBookChar) * 546) >> 1) + 546) * _tickLength;
	_sceneUpdateRequired = true;
}

void EoBCoreEngine::spellCallback_start_disintegrate() {
	int d = findSingleSpellTarget(1);
	if (d != -1)
		magicObjectStatusHit(&_monsters[d], 4, true, 4);
	memset(_visibleBlocks[13]->walls, 0, 4);
	_sceneUpdateRequired = true;
}

void EoBCoreEngine::spellCallback_start_fleshToStone() {
	sparkEffectOffensive();
	int t = getClosestMonster(_openBookChar, calcNewBlockPosition(_currentBlock, _currentDirection));
	if (t != -1)
		magicObjectStatusHit(&_monsters[t], 5, true, 4);
	else
		printWarning(_magicStrings8[4]);
}

void EoBCoreEngine::spellCallback_start_stoneToFlesh() {
	if (_characters[_activeSpellCharId].flags & 8)
		_characters[_activeSpellCharId].flags &= ~8;
	else
		printNoEffectWarning();
}

void EoBCoreEngine::spellCallback_start_trueSeeing() {
	_wllVmpMap[46] = 0;
}

bool EoBCoreEngine::spellCallback_end_trueSeeing(void *) {
	_wllVmpMap[46] = 1;
	return true;
}

void EoBCoreEngine::spellCallback_start_slayLiving() {
	int d = findSingleSpellTarget(2);
	if (d != -1) {
		if (!magicObjectStatusHit(&_monsters[d], 3, true, 4))
			inflictMonsterDamage(&_monsters[d], rollDice(2, 8, 1), true);
	}
}

void EoBCoreEngine::spellCallback_start_powerWordStun() {
	int d = findSingleSpellTarget(2);
	if (d != -1) {
		if (_monsters[d].hitPointsCur < 90)
			magicObjectStatusHit(&_monsters[d], 5, true, 4);
	}
}

void EoBCoreEngine::spellCallback_start_causeLightWounds() {
	causeWounds(1, 8, 0);
}

void EoBCoreEngine::spellCallback_start_cureLightWounds() {
	modifyCharacterHitpoints(_activeSpellCharId, rollDice(1, 8));
}

void EoBCoreEngine::spellCallback_start_aid() {
	if (!testCharacter(_activeSpellCharId, 3)) {
		printNoEffectWarning();
	} else if (_characters[_activeSpellCharId].effectsRemainder[3]) {
		printWarning(Common::String::format(_magicStrings8[(_flags.gameID == GI_EOB1) ? 2 : 5], _characters[_activeSpellCharId].name).c_str());
	} else {
		_characters[_activeSpellCharId].effectsRemainder[3] = rollDice(1, 8);
		_characters[_activeSpellCharId].hitPointsCur += _characters[_activeSpellCharId].effectsRemainder[3];
		_characters[_activeSpellCharId].effectFlags |= 0x1000;
		return;
	}

	removeCharacterEffect(_activeSpell, _activeSpellCharId, 0);
	deleteCharEventTimer(_activeSpellCharId, -_activeSpell);
}

bool EoBCoreEngine::spellCallback_end_aid(void *obj) {
	EoBCharacter *c = (EoBCharacter *)obj;
	c->hitPointsCur -= c->effectsRemainder[3];
	c->effectsRemainder[3] = 0;
	c->effectFlags &= ~0x1000;
	return true;
}

void EoBCoreEngine::spellCallback_start_flameBlade() {
	int t = createMagicWeaponType(0, 0, 0, 0x0f, 1, 4, 4, 1);
	Item i = (t != -1) ? createMagicWeaponItem(0, 84, 0, t) : -1;
	if (t == -1 || i == -1) {
		if (_flags.gameID == GI_EOB2)
			printWarning(_magicStrings8[0]);
		removeCharacterEffect(_activeSpell, _activeSpellCharId, 0);
		deleteCharEventTimer(_activeSpellCharId, -_activeSpell);
		_returnAfterSpellCallback = true;
	} else {
		_characters[_activeSpellCharId].inventory[getMagicWeaponSlot(_activeSpellCharId)] = i;
	}
}

void EoBCoreEngine::spellCallback_start_slowPoison() {
	if (_characters[_activeSpellCharId].flags & 2) {
		_characters[_activeSpellCharId].effectFlags |= 0x2000;
		setSpellEventTimer(_activeSpell, 1, 32760, 1, 1);
	} else {
		printNoEffectWarning();
	}
}

bool EoBCoreEngine::spellCallback_end_slowPoison(void *obj) {
	EoBCharacter *c = (EoBCharacter *)obj;
	c->effectFlags &= ~0x2000;
	return true;
}

void EoBCoreEngine::spellCallback_start_createFood() {
	for (int i = 0; i < 6; i++) {
		if (!testCharacter(i, 3))
			continue;
		_characters[i].food = 100;
	}
}

void EoBCoreEngine::spellCallback_start_removeParalysis() {
	int numChar = 4;
	for (int i = 0; i < 6; i++) {
		if (!(_characters[i].flags & 4) || !numChar)
			continue;
		_characters[i].flags &= ~4;
		numChar--;
	}
}

void EoBCoreEngine::spellCallback_start_causeSeriousWounds() {
	causeWounds(2, 8, 1);
}

void EoBCoreEngine::spellCallback_start_cureSeriousWounds() {
	modifyCharacterHitpoints(_activeSpellCharId, rollDice(2, 8, 1));
}

void EoBCoreEngine::spellCallback_start_neutralizePoison() {
	if (_characters[_activeSpellCharId].flags & 2)
		neutralizePoison(_activeSpellCharId);
	else
		printNoEffectWarning();
}

void EoBCoreEngine::spellCallback_start_causeCriticalWounds() {
	causeWounds(3, 8, 3);
}

void EoBCoreEngine::spellCallback_start_cureCriticalWounds() {
	modifyCharacterHitpoints(_activeSpellCharId, rollDice(3, 8, 3));
}

void EoBCoreEngine::spellCallback_start_flameStrike() {
	launchMagicObject(_openBookChar, _flags.gameID == GI_EOB1 ? 8 : 7, _currentBlock, _activeSpellCharacterPos, _currentDirection);
}

bool EoBCoreEngine::spellCallback_end_flameStrike(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	return magicObjectDamageHit(fo, 6, 8, 0, 0);
}

void EoBCoreEngine::spellCallback_start_raiseDead() {
	if (_characters[_activeSpellCharId].hitPointsCur == -10 && ((_characters[_activeSpellCharId].raceSex >> 1) != 1)) {
		_characters[_activeSpellCharId].hitPointsCur = 1;
		gui_drawCharPortraitWithStats(_activeSpellCharId);
	} else {
		printNoEffectWarning();
	}
}

void EoBCoreEngine::spellCallback_start_harm() {
	causeWounds(-1, -1, -1);
}

void EoBCoreEngine::spellCallback_start_heal() {
	EoBCharacter *c = &_characters[_activeSpellCharId];
	if (c->hitPointsMax <= c->hitPointsCur)
		printWarning(_magicStrings4[0]);
	else
		modifyCharacterHitpoints(_activeSpellCharId, c->hitPointsMax - c->hitPointsCur);
}

void EoBCoreEngine::spellCallback_start_layOnHands() {
	modifyCharacterHitpoints(_activeSpellCharId, _characters[_openBookChar].level[0] << 1);
}

void EoBCoreEngine::spellCallback_start_turnUndead() {
	uint16 bl = calcNewBlockPosition(_currentBlock, _currentDirection);
	if (!(_levelBlockProperties[bl].flags & 7))
		return;

	int cl = _openBookCasterLevel ? _openBookCasterLevel : getClericPaladinLevel(_openBookChar);
	int r = rollDice(1, 20);
	bool hit = false;

	for (const int16 *m = findBlockMonsters(bl, 4, 4, 1, 1); *m != -1; m++) {
		if ((_monsterProps[_monsters[*m].type].typeFlags & 4) && !(_monsters[*m].flags & 0x10)) {
			_preventMonsterFlash = true;
			_monsters[*m].flags |= 0x10;
			hit |= turnUndeadHit(&_monsters[*m], r, cl);
		}
	}

	if (hit) {
		turnUndeadAutoHit();
		snd_playSoundEffect(95);
		updateAllMonsterShapes();
	}

	_preventMonsterFlash = false;
}

bool EoBCoreEngine::spellCallback_end_monster_lightningBolt(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	return magicObjectDamageHit(fo, 0, 0, 12, 1);
}

bool EoBCoreEngine::spellCallback_end_monster_fireball1(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	bool res = false;
	if (_partyEffectFlags & 0x20000) {
		res = magicObjectDamageHit(fo, 4, 10, 6, 0);
		if (res) {
			gui_drawAllCharPortraitsWithStats();
			_partyEffectFlags &= ~0x20000;
		}
	} else {
		res = magicObjectDamageHit(fo, 12, 10, 6, 0);
	}
	return res;
}

bool EoBCoreEngine::spellCallback_end_monster_fireball2(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	return magicObjectDamageHit(fo, 0, 0, 18, 0);
}

bool EoBCoreEngine::spellCallback_end_monster_deathSpell(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	if (fo->curBlock != _currentBlock)
		return false;

	int numDest = rollDice(1, 4);
	_txt->printMessage(_magicStrings2[2]);
	for (int d = findFirstCharacterSpellTarget(); d != -1 && numDest; d = findNextCharacterSpellTarget(d)) {
		if (_characters[d].level[0] < 8) {
			inflictCharacterDamage(d, 300);
			numDest--;
		}
	}

	return true;
}

bool EoBCoreEngine::spellCallback_end_monster_disintegrate(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	if (fo->curBlock != _currentBlock)
		return false;

	int d = findFirstCharacterSpellTarget();
	if (d != -1) {
		if (!charDeathSavingThrow(d, 1)) {
			inflictCharacterDamage(d, 300);
			_txt->printMessage(_magicStrings2[1], -1, _characters[d].name);
		}
	}

	return true;
}

bool EoBCoreEngine::spellCallback_end_monster_causeCriticalWounds(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	if (fo->curBlock != _currentBlock)
		return false;

	int d = findFirstCharacterSpellTarget();
	if (d != -1) {
		_txt->printMessage(_magicStrings2[3], -1, _characters[d].name);
		inflictCharacterDamage(d, rollDice(3, 8, 3));
	}

	return true;
}

bool EoBCoreEngine::spellCallback_end_monster_fleshToStone(void *obj) {
	EoBFlyingObject *fo = (EoBFlyingObject *)obj;
	if (fo->curBlock != _currentBlock)
		return false;

	int d = findFirstCharacterSpellTarget();
	while (d != -1) {
		if (!charDeathSavingThrow(d, 2)) {
			statusAttack(d, 8, _magicStrings2[4], 5, 0, 0, 1);
			d = -1;
		} else {
			d = findNextCharacterSpellTarget(d);
		}
	}

	return true;
}

} // End of namespace Kyra

#endif // ENABLE_EOB
