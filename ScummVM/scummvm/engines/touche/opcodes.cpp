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


#include "common/system.h"

#include "touche/touche.h"

namespace Touche {

void ToucheEngine::setupOpcodes() {
	static const OpcodeProc opcodesTable[] = {
		/* 0x00 */
		&ToucheEngine::op_nop,
		&ToucheEngine::op_jnz,
		&ToucheEngine::op_jz,
		&ToucheEngine::op_jmp,
		/* 0x04 */
		&ToucheEngine::op_true,
		&ToucheEngine::op_false,
		&ToucheEngine::op_push,
		&ToucheEngine::op_not,
		/* 0x08 */
		&ToucheEngine::op_add,
		&ToucheEngine::op_sub,
		&ToucheEngine::op_mul,
		&ToucheEngine::op_div,
		/* 0x0C */
		&ToucheEngine::op_mod,
		&ToucheEngine::op_and,
		&ToucheEngine::op_or,
		&ToucheEngine::op_neg,
		/* 0x10 */
		&ToucheEngine::op_testGreater,
		&ToucheEngine::op_testEquals,
		&ToucheEngine::op_testLower,
		&ToucheEngine::op_fetchScriptWord,
		/* 0x14 */
		0,
		0,
		0,
		0,
		/* 0x18 */
		&ToucheEngine::op_testGreaterOrEquals,
		&ToucheEngine::op_testLowerOrEquals,
		&ToucheEngine::op_testNotEquals,
		&ToucheEngine::op_endConversation,
		/* 0x1C */
		&ToucheEngine::op_stopScript,
		&ToucheEngine::op_getFlag,
		&ToucheEngine::op_setFlag,
		0,
		/* 0x20 */
		0,
		0,
		0,
		&ToucheEngine::op_fetchScriptByte,
		/* 0x24 */
		0,
		0,
		0,
		0,
		/* 0x28 */
		0,
		0,
		0,
		0,
		/* 0x2C */
		0,
		0,
		&ToucheEngine::op_getKeyCharWalkBox,
		&ToucheEngine::op_startSound,
		/* 0x30 */
		&ToucheEngine::op_moveKeyCharToPos,
		0,
		0,
		0,
		/* 0x34 */
		&ToucheEngine::op_loadRoom,
		&ToucheEngine::op_updateRoom,
		&ToucheEngine::op_startTalk,
		&ToucheEngine::op_setKeyCharBox,
		/* 0x38 */
		&ToucheEngine::op_initKeyCharScript,
		&ToucheEngine::op_loadSprite,
		&ToucheEngine::op_loadSequence,
		&ToucheEngine::op_setKeyCharFrame,
		/* 0x3C */
		&ToucheEngine::op_setKeyCharDirection,
		&ToucheEngine::op_clearConversationChoices,
		&ToucheEngine::op_addConversationChoice,
		&ToucheEngine::op_removeConversationChoice,
		/* 0x40 */
		&ToucheEngine::op_getInventoryItem,
		&ToucheEngine::op_setInventoryItem,
		&ToucheEngine::op_startEpisode,
		&ToucheEngine::op_setConversationNum,
		/* 0x44 */
		0,
		&ToucheEngine::op_enableInput,
		&ToucheEngine::op_disableInput,
		&ToucheEngine::op_faceKeyChar,
		/* 0x48 */
		&ToucheEngine::op_getKeyCharCurrentAnim,
		&ToucheEngine::op_getCurrentKeyChar,
		&ToucheEngine::op_isKeyCharActive,
		&ToucheEngine::op_setPalette,
		/* 0x4C */
		&ToucheEngine::op_changeWalkPath,
		&ToucheEngine::op_lockWalkPath,
		&ToucheEngine::op_initializeKeyChar,
		&ToucheEngine::op_setupWaitingKeyChars,
		/* 0x50 */
		&ToucheEngine::op_updateRoomAreas,
		&ToucheEngine::op_unlockWalkPath,
		0,
		&ToucheEngine::op_addItemToInventoryAndRedraw,
		/* 0x54 */
		&ToucheEngine::op_giveItemTo,
		&ToucheEngine::op_setHitBoxText,
		&ToucheEngine::op_fadePalette,
		0,
		/* 0x58 */
		0,
		0,
		0,
		0,
		/* 0x5C */
		0,
		0,
		0,
		0,
		/* 0x60 */
		0,
		&ToucheEngine::op_getInventoryItemFlags,
		&ToucheEngine::op_drawInventory,
		&ToucheEngine::op_stopKeyCharScript,
		/* 0x64 */
		&ToucheEngine::op_restartKeyCharScript,
		&ToucheEngine::op_getKeyCharCurrentWalkBox,
		&ToucheEngine::op_getKeyCharPointsDataNum,
		&ToucheEngine::op_setupFollowingKeyChar,
		/* 0x68 */
		&ToucheEngine::op_startAnimation,
		&ToucheEngine::op_setKeyCharTextColor,
		0,
		0,
		/* 0x6C */
		0,
		0,
		0,
		0,
		/* 0x70 */
		&ToucheEngine::op_startMusic,
		0,
		&ToucheEngine::op_sleep,
		0,
		/* 0x74 */
		&ToucheEngine::op_setKeyCharDelay,
		&ToucheEngine::op_lockHitBox,
		&ToucheEngine::op_removeItemFromInventory,
		&ToucheEngine::op_unlockHitBox,
		/* 0x78 */
		&ToucheEngine::op_addRoomArea,
		&ToucheEngine::op_setKeyCharFlags,
		0,
		0,
		/* 0x7C */
		0,
		0,
		0,
		0,
		/* 0x80 */
		&ToucheEngine::op_unsetKeyCharFlags,
		&ToucheEngine::op_drawSpriteOnBackdrop,
		&ToucheEngine::op_loadSpeechSegment,
		0,
		/* 0x84 */
		&ToucheEngine::op_startPaletteFadeIn,
		&ToucheEngine::op_startPaletteFadeOut,
		&ToucheEngine::op_setRoomAreaState
	};

	_opcodesTable = opcodesTable;
	_numOpcodes = ARRAYSIZE(opcodesTable);
}

void ToucheEngine::op_nop() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_nop()");
}

void ToucheEngine::op_jnz() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_jnz()");
	if (*_script.stackDataPtr != 0) {
		_script.dataOffset = _script.readNextWord();
	} else {
		_script.dataOffset += 2;
	}
}

void ToucheEngine::op_jz() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_jz()");
	if (*_script.stackDataPtr == 0) {
		_script.dataOffset = _script.readNextWord();
	} else {
		_script.dataOffset += 2;
	}
}

void ToucheEngine::op_jmp() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_jmp()");
	_script.dataOffset = _script.readNextWord();
}

void ToucheEngine::op_true() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_true()");
	*_script.stackDataPtr = -1;
}

void ToucheEngine::op_false() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_false()");
	*_script.stackDataPtr = 0;
}

void ToucheEngine::op_push() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_push()");
	--_script.stackDataPtr;
	*_script.stackDataPtr = 0;
}

void ToucheEngine::op_not() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_not()");
	if (*_script.stackDataPtr == 0) {
		*_script.stackDataPtr = -1;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_add() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_add()");
	int16 val = *_script.stackDataPtr++;
	*_script.stackDataPtr += val;
}

void ToucheEngine::op_sub() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_sub()");
	int16 val = *_script.stackDataPtr++;
	*_script.stackDataPtr -= val;
}

void ToucheEngine::op_mul() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_mul()");
	int16 val = *_script.stackDataPtr++;
	*_script.stackDataPtr *= val;
}

void ToucheEngine::op_div() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_div()");
	int16 val = *_script.stackDataPtr++;
	if (val != 0) {
		*_script.stackDataPtr /= val;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_mod() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_mod()");
	int16 val = *_script.stackDataPtr++;
	if (val != 0) {
		*_script.stackDataPtr %= val;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_and() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_and()");
	uint16 val = *_script.stackDataPtr++;
	*_script.stackDataPtr &= val;
}

void ToucheEngine::op_or() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_or()");
	uint16 val = *_script.stackDataPtr++;
	*_script.stackDataPtr |= val;
}

void ToucheEngine::op_neg() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_neg()");
	uint16 val = *_script.stackDataPtr;
	*_script.stackDataPtr = ~val;
}

void ToucheEngine::op_testGreater() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_testGreater()");
	int16 val = *_script.stackDataPtr++;
	if (val > *_script.stackDataPtr) {
		*_script.stackDataPtr = -1;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_testEquals() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_testEquals()");
	int16 val = *_script.stackDataPtr++;
	if (val == *_script.stackDataPtr) {
		*_script.stackDataPtr = -1;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_testLower() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_testLower()");
	int16 val = *_script.stackDataPtr++;
	if (val < *_script.stackDataPtr) {
		*_script.stackDataPtr = -1;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_fetchScriptWord() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_fetchScriptWord()");
	int16 val = _script.readNextWord();
	*_script.stackDataPtr = val;
}

void ToucheEngine::op_testGreaterOrEquals() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_testGreaterOrEquals()");
	int16 val = *_script.stackDataPtr++;
	if (val >= *_script.stackDataPtr) {
		*_script.stackDataPtr = -1;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_testLowerOrEquals() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_testLowerOrEquals()");
	int16 val = *_script.stackDataPtr++;
	if (val <= *_script.stackDataPtr) {
		*_script.stackDataPtr = -1;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_testNotEquals() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_testNotEquals()");
	int16 val = *_script.stackDataPtr++;
	if (val != *_script.stackDataPtr) {
		*_script.stackDataPtr = -1;
	} else {
		*_script.stackDataPtr = 0;
	}
}

void ToucheEngine::op_endConversation() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_endConversation()");
	_script.quitFlag = 1;
	_conversationEnded = true;
	_disabledInputCounter = 0;
}

void ToucheEngine::op_stopScript() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_stopScript()");
	_script.quitFlag = 1;
}

void ToucheEngine::op_getFlag() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_getFlag()");
	uint16 fl = _script.readNextWord();
	*_script.stackDataPtr = _flagsTable[fl];
}

void ToucheEngine::op_setFlag() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setFlag()");
	uint16 flag = _script.readNextWord();
	int16 val = *_script.stackDataPtr;
	_flagsTable[flag] = val;
	switch (flag) {
	case 104:
		_currentKeyCharNum = val;
		break;
	case 611:
		if (val != 0)
			quitGame();
		break;
	case 612:
		_flagsTable[613] = getRandomNumber(val);
		break;
	case 614:
	case 615:
		_fullRedrawCounter = 1;
		break;
	case 618:
		showCursor(val == 0);
		break;
	case 619:
		debug(0, "Unknown music flag %d", val);
		break;
	}
}

void ToucheEngine::op_fetchScriptByte() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_fetchScriptByte()");
	int16 val = _script.readNextByte();
	*_script.stackDataPtr = val;
}

void ToucheEngine::op_getKeyCharWalkBox() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_getKeyCharWalkBox()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	*_script.stackDataPtr = _keyCharsTable[keyChar].walkDataNum;
}

void ToucheEngine::op_startSound() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_startSound()");
	_newSoundNum = _script.readNextWord();
	_newSoundDelay = _script.readNextWord();
	_newSoundPriority = 1;
}

void ToucheEngine::op_moveKeyCharToPos() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_moveKeyCharToPos()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	int16 num = _script.readNextWord();
	if (num == -1) {
		num = _script.readNextWord();
		num = _keyCharsTable[num].pointsDataNum;
	}
	sortPointsData(-1, num);
	buildWalkPointsList(keyChar);
	_keyCharsTable[keyChar].flags &= ~0x10;
	if (_script.keyCharNum == keyChar) {
		removeFromTalkTable(_script.keyCharNum);
		_keyCharsTable[keyChar].waitingKeyCharPosTable[0] = -1;
		_keyCharsTable[keyChar].waitingKeyCharPosTable[2] = -1;
		_keyCharsTable[keyChar].waitingKeyChar = _script.keyCharNum;
		_keyCharsTable[keyChar].waitingKeyCharPosTable[1] = num;
		_script.quitFlag = 3;
	}
}

void ToucheEngine::op_loadRoom() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_loadRoom()");
	int16 num = _script.readNextWord();
	res_loadRoom(num);
}

void ToucheEngine::op_updateRoom() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_updateRoom()");
	int16 area = _script.readNextWord();
	updateRoomAreas(area, 0);

	// Workaround for bug #1618700. Beggar sign (area 25) should be displayed even
	// if Henri isn't present in the room.
	//
	//  [00B3] (1D) ST[0] = FLAGS[2]
	//  [00B6] (06) PUSH
	//  [00B7] (13) ST[0] = 0
	//  [00BA] (11) ST[0] = ST[1] == ST[0]
	//  [00BB] (02) JZ 0xF6
	//  [xxxx] ...
	//  [0192] (35) UPDATE_ROOM(16, 0)
	//  [0195] (35) UPDATE_ROOM(19, 0)

	if (_currentEpisodeNum == 91 && area == 19 && _flagsTable[2] != 0) {
		debug(0, "Workaround beggar sign disappearing bug");
		updateRoomAreas(25, 0);
	}
}

void ToucheEngine::op_startTalk() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_startTalk()");
	int16 keyChar = _script.readNextWord();
	int16 num = _script.readNextWord();
	if (num == 750) {
		return;
	}
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
		num += _currentKeyCharNum & 1;
	}
	addToTalkTable(keyChar, num, _script.keyCharNum);
	_script.quitFlag = 3;
}

void ToucheEngine::op_loadSprite() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_loadSprite()");
	int16 index = _script.readNextWord();
	int16 num = _script.readNextWord();
	res_loadSprite(num, index);
}

void ToucheEngine::op_loadSequence() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_loadSequence()");
	int16 index = _script.readNextWord();
	int16 num = _script.readNextWord();
	res_loadSequence(num, index);
}

void ToucheEngine::op_setKeyCharBox() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setKeyCharBox()");
	int16 keyChar = _script.readNextWord();
	int16 num = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	setKeyCharBox(keyChar, num);
}

void ToucheEngine::op_initKeyCharScript() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_initKeyCharScript()");
	int16 keyChar = _script.readNextWord();
	int16 color = _script.readNextWord();
	int16 f1 = _script.readNextWord();
	int16 f2 = _script.readNextWord();
	int16 f3 = _script.readNextWord();
	setKeyCharTextColor(keyChar, color);
	initKeyCharScript(keyChar, f1, f2, f3);

	// Workaround for bug #1622114. KeyChar 3 script must be running in order to complete the
	// rope+torch puzzle.
	//
	// FLAG[500] : 1 if Cardinal cutscene has already been played
	// FLAG[501] : 1 if cathedral is lightened (by the two torches)
	//
	// [00D3] (38) INIT_KEY_CHAR_SCRIPT(keychar=1, 254, 1, 1, 0)

	if (_currentEpisodeNum == 109 && keyChar == 1 && _flagsTable[500] == 1 && _flagsTable[501] == 1 && _keyCharsTable[3].scriptDataOffset == 0) {
		debug(0, "Workaround disappearing rope bug");
		initKeyCharScript(3, 3, 3, 0);
	}
}

void ToucheEngine::op_setKeyCharFrame() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setKeyCharFrame()");
	int16 keyChar = _script.readNextWord();
	int16 val1 = _script.readNextWord();
	int16 val2 = _script.readNextWord();
	int16 val3 = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	setKeyCharFrame(keyChar, val1, val2, val3);
}

void ToucheEngine::op_setKeyCharDirection() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setKeyCharDirection()");
	int16 keyChar = _script.readNextWord();
	int16 dir = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	setKeyCharFacingDirection(keyChar, dir);
}

void ToucheEngine::op_clearConversationChoices() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_clearConversationChoices()");
	clearConversationChoices();
}

void ToucheEngine::op_addConversationChoice() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_addConversationChoice()");
	int16 num = _script.readNextWord();
	addConversationChoice(num);
}

void ToucheEngine::op_removeConversationChoice() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_removeConversationChoice()");
	int16 num = _script.readNextWord();
	removeConversationChoice(num);
}

void ToucheEngine::op_getInventoryItem() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_getInventoryItem()");
	int16 keyChar = _script.readNextWord();
	uint16 item = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	assert(item < sizeof(_keyCharsTable[keyChar].inventoryItems));
	*_script.stackDataPtr = _keyCharsTable[keyChar].inventoryItems[item];
}

void ToucheEngine::op_setInventoryItem() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setInventoryItem()");
	int16 keyChar = _script.readNextWord();
	uint16 item = _script.readNextWord();
	if (item == 4) {
		setKeyCharMoney();
	}
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	assert(item < sizeof(_keyCharsTable[keyChar].inventoryItems));
	_keyCharsTable[keyChar].inventoryItems[item] = *_script.stackDataPtr;
	if (item == 4 && !_hideInventoryTexts) {
		drawAmountOfMoneyInInventory();
	}
}

void ToucheEngine::op_startEpisode() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_startEpisode()");
	_newEpisodeNum = _script.readNextWord();
	_flagsTable[0] = _script.readNextWord();
	_disabledInputCounter = 1;
	_script.quitFlag = 1;
}

void ToucheEngine::op_setConversationNum() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setConversationNum()");
	_conversationNum = _script.readNextWord();
}

void ToucheEngine::op_enableInput() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_enableInput()");
	++_disabledInputCounter;
}

void ToucheEngine::op_disableInput() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_disableInput()");
	if (_disabledInputCounter != 0) {
		--_disabledInputCounter;
	}
}

void ToucheEngine::op_faceKeyChar() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_faceKeyChar()");
	int16 keyChar1 = _script.readNextWord();
	int16 keyChar2 = _script.readNextWord();
	if (keyChar1 == 256) {
		keyChar1 = _currentKeyCharNum;
	}
	if (_keyCharsTable[keyChar1].xPos <= _keyCharsTable[keyChar2].xPos) {
		_keyCharsTable[keyChar2].facingDirection = 3;
	} else {
		_keyCharsTable[keyChar2].facingDirection = 0;
	}
}

void ToucheEngine::op_getKeyCharCurrentAnim() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_getKeyCharCurrentAnim()");
	int16 keyChar = _script.readNextWord();
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	*_script.stackDataPtr = _keyCharsTable[keyChar].currentAnim;
}

void ToucheEngine::op_getCurrentKeyChar() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_getCurrentKeyChar()");
	*_script.stackDataPtr = _currentKeyCharNum;
}

void ToucheEngine::op_isKeyCharActive() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_isKeyCharActive()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	*_script.stackDataPtr = _keyCharsTable[keyChar].num != 0 ? 1 : 0;
}

void ToucheEngine::op_setPalette() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setPalette()");
	int16 r = _script.readNextWord();
	int16 g = _script.readNextWord();
	int16 b = _script.readNextWord();
	setPalette(0, 240, r, g, b);
}

void ToucheEngine::op_changeWalkPath() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_changeWalkPath()");
	int16 num1 = _script.readNextWord();
	int16 num2 = _script.readNextWord();
	int16 val = _script.readNextWord();
	changeWalkPath(num1, num2, val);
}

void ToucheEngine::op_lockWalkPath() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_lockWalkPath()");
	int16 num1 = _script.readNextWord();
	int16 num2 = _script.readNextWord();
	lockWalkPath(num1, num2);
}

void ToucheEngine::op_initializeKeyChar() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_initializeKeyChar()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	initKeyChars(keyChar);
}

void ToucheEngine::op_setupWaitingKeyChars() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setupWaitingKeyChars()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	int16 val1 = _script.readNextWord();
	int16 val2 = _script.readNextWord();
	if (val1 == -1) {
		_waitingSetKeyCharNum2 = keyChar;
		_waitingSetKeyCharNum1 = val2;
		_waitingSetKeyCharNum3 = _script.keyCharNum;
		_script.quitFlag = 3;
	} else {
		_keyCharsTable[_script.keyCharNum].waitingKeyCharPosTable[0] = -1;
		_keyCharsTable[_script.keyCharNum].waitingKeyCharPosTable[1] = -1;
		_keyCharsTable[_script.keyCharNum].waitingKeyCharPosTable[2] = -1;
		_keyCharsTable[_script.keyCharNum].waitingKeyChar = keyChar;
		assert(val1 >= 0 && val1 < 3);
		_keyCharsTable[_script.keyCharNum].waitingKeyCharPosTable[val1] = val2;
		_script.quitFlag = 3;
	}
}

void ToucheEngine::op_updateRoomAreas() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_updateRoomAreas()");
	int16 area = _script.readNextWord();
	updateRoomAreas(area, 1);
}

void ToucheEngine::op_unlockWalkPath() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_unlockWalkPath()");
	int16 num1 = _script.readNextWord();
	int16 num2 = _script.readNextWord();
	unlockWalkPath(num1, num2);
}

void ToucheEngine::op_addItemToInventoryAndRedraw() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_addItemToInventoryAndRedraw()");
	int16 keyChar = _script.readNextWord();
	int16 item = *_script.stackDataPtr;
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}

	// Workaround for bug #1623356. The original script allows you to either use the
	// "waxy knife" (object 72) or the dagger (object 7) on the rope. But in both
	// situations, only the dagger is put back in the inventory.
	//
	//  [1A35] (1D) ST[0] = FLAGS[119]
	//  [1A38] (06) PUSH
	//  [1A39] (13) ST[0] = 7
	//  [1A3C] (11) ST[0] = ST[1] == ST[0]
	//  [1A3D] (06) PUSH
	//  [1A3E] (1D) ST[0] = FLAGS[119]
	//  [1A41] (06) PUSH
	//  [1A42] (13) ST[0] = 72
	//  [1A45] (11) ST[0] = ST[1] == ST[0]
	//  [1A46] (0E) OR
	//  [1A47] (02) JZ 0x1B1B
	//  [xxxx] ...
	//  [1B05] (13) ST[0] = 7
	//  [1B08] (53) ADD_ITEM_TO_INVENTORY_AND_REDRAW(keychar=1)

	if (_currentEpisodeNum == 92 && keyChar == 1 && item == 7) {
		if (_flagsTable[119] == 72) {
			debug(0, "Workaround waxy knife not re-appearing in the inventory");
			item = 72;
		}
	}

	addItemToInventory(keyChar, item);
	if (_currentKeyCharNum == keyChar && !_hideInventoryTexts) {
		drawInventory(_currentKeyCharNum, 1);
	}
}

void ToucheEngine::op_giveItemTo() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_giveItemTo()");
	_giveItemToCounter = 1;
	_giveItemToObjectNum = _script.readNextWord();
	_giveItemToKeyCharNum = _script.keyCharNum;
	_script.quitFlag = 3;
}

void ToucheEngine::op_setHitBoxText() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setHitBoxText()");
	int16 num = _script.readNextWord();
	if (num & 0x4000) {
		num &= 0xFF;
		_keyCharsTable[num].strNum = 1;
	} else {
		for (uint i = 0; i < _programHitBoxTable.size(); ++i) {
			if (_programHitBoxTable[i].item == num) {
				_programHitBoxTable[i].str = _programHitBoxTable[i].defaultStr;
				break;
			}
		}
	}
}

void ToucheEngine::op_fadePalette() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_fadePalette()");
	int16 fadeOut = _script.readNextWord();
	int colorsCount = 240;
	// Workaround for bug #1751149. Script triggers a palette fading, but some
	// of the room graphics use palette colors >= 240.
	if (_currentEpisodeNum == 104 && _currentRoomNum == 68) {
		colorsCount = 256;
	}
	if (fadeOut) {
		fadePalette(0, colorsCount, 255, -2, 128);
	} else {
		fadePalette(0, colorsCount, 0, 2, 128);
	}
}

void ToucheEngine::op_getInventoryItemFlags() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_getInventoryItemFlags()");
	int16 item = _script.readNextWord();
	int16 flags = _inventoryItemsInfoTable[item];
	if (flags & 0x10) {
		flags &= 0xF;
	} else {
		flags &= ~0xF;
	}
	*_script.stackDataPtr = flags;
}

void ToucheEngine::op_drawInventory() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_drawInventory()");
	int16 num = _script.readNextWord();
	drawInventory(num, 1);
}

void ToucheEngine::op_stopKeyCharScript() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_stopKeyCharScript()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	_keyCharsTable[keyChar].flags |= kScriptStopped;
}

void ToucheEngine::op_restartKeyCharScript() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_restartKeyCharScript()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	KeyChar *key = &_keyCharsTable[keyChar];
	key->flags &= ~(kScriptStopped | kScriptPaused);
	key->scriptDataOffset = key->scriptDataStartOffset;
	key->scriptStackPtr = &key->scriptStackTable[39];
}

void ToucheEngine::op_getKeyCharCurrentWalkBox() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_getKeyCharCurrentWalkBox()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	*_script.stackDataPtr = _keyCharsTable[keyChar].currentWalkBox;
}

void ToucheEngine::op_getKeyCharPointsDataNum() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_getKeyCharPointsDataNum()");
	int16 keyChar = _script.readNextWord();
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	*_script.stackDataPtr = _keyCharsTable[keyChar].pointsDataNum;
}

void ToucheEngine::op_setupFollowingKeyChar() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setupFollowingKeyChar()");
	int16 val = _script.readNextWord();
	int16 keyChar = _script.readNextWord();
	assert(keyChar >= 0 && keyChar < NUM_KEYCHARS);
	_keyCharsTable[keyChar].followingKeyCharNum = val;
	_keyCharsTable[keyChar].flags |= 0x10;
	_keyCharsTable[keyChar].followingKeyCharPos = -1;
}

void ToucheEngine::op_startAnimation() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_startAnimation()");
	int16 keyChar = _script.readNextWord();
	int16 pos = _script.readNextWord();
	int16 num = *_script.stackDataPtr;
	addToAnimationTable(num, pos, keyChar, 3);
}

void ToucheEngine::op_setKeyCharTextColor() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setKeyCharTextColor()");
	int16 keyChar = _script.readNextWord();
	uint16 color = _script.readNextWord();
	setKeyCharTextColor(keyChar, color);
}

void ToucheEngine::op_startMusic() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_startMusic()");
	_newMusicNum = _script.readNextWord();
}

void ToucheEngine::op_sleep() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_sleep()");
	// this should probably be turned into a no-op/debug-op...
	int cycles = _script.readNextWord() * 2;
	if (!_fastMode) {
		for (; cycles > 0; --cycles) {
			_system->delayMillis(kCycleDelay);
			_system->updateScreen();
		}
	}
}

void ToucheEngine::op_setKeyCharDelay() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setKeyCharDelay()");
	int16 delay = _script.readNextWord();
	_keyCharsTable[_script.keyCharNum].delay = delay;
	_script.quitFlag = 3;
}

void ToucheEngine::op_lockHitBox() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_lockHitBox()");
	int16 num = _script.readNextWord();
	lockUnlockHitBox(num, 1);
}

void ToucheEngine::op_removeItemFromInventory() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_removeItemFromInventory()");
	int16 keyChar = _script.readNextWord();
	int16 item = *_script.stackDataPtr;
	if (keyChar == 256) {
		keyChar = _currentKeyCharNum;
	}
	removeItemFromInventory(keyChar, item);
	if (keyChar == _currentKeyCharNum && !_hideInventoryTexts) {
		drawInventory(_currentKeyCharNum, 1);
	}
}

void ToucheEngine::op_unlockHitBox() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_unlockHitBox()");
	int16 num = _script.readNextWord();
	lockUnlockHitBox(num, 0);
}

void ToucheEngine::op_addRoomArea() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_addRoomArea()");
	int16 num = _script.readNextWord();
	uint16 flag = _script.readNextWord();
	addRoomArea(num, flag);
}

void ToucheEngine::op_setKeyCharFlags() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setKeyCharFlags()");
	int16 keyChar = _script.readNextWord();
	uint16 flags = _script.readNextWord();
	flags &= 0xFF00;
	_keyCharsTable[keyChar].flags |= flags;
}

void ToucheEngine::op_unsetKeyCharFlags() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_unsetKeyCharFlags()");
	int16 keyChar = _script.readNextWord();
	uint16 flags = _script.readNextWord();
	flags &= 0xFF00;
	_keyCharsTable[keyChar].flags &= ~flags;
}

void ToucheEngine::op_loadSpeechSegment() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_loadSpeechSegment()");
	int16 num = _script.readNextWord();
	res_loadSpeech(num);
}

void ToucheEngine::op_drawSpriteOnBackdrop() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_drawSpriteOnBackdrop()");
	int16 num = _script.readNextWord();
	int16 x = _script.readNextWord();
	int16 y = _script.readNextWord();
	drawSpriteOnBackdrop(num, x, y);
}

void ToucheEngine::op_startPaletteFadeIn() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_startPaletteFadeIn()");
	_flagsTable[290] = 0;
	_flagsTable[605] = 0;
	_flagsTable[607] = 0;
	_flagsTable[608] = 0xFF;
	_flagsTable[609] = 0xFF;
	_flagsTable[610] = 0;
	_flagsTable[603] = _script.readNextWord();
}

void ToucheEngine::op_startPaletteFadeOut() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_startPaletteFadeOut()");
	_flagsTable[290] = 0;
	_flagsTable[605] = 0xFF;
	_flagsTable[607] = 0;
	_flagsTable[608] = 0xFF;
	_flagsTable[609] = 0xFF;
	_flagsTable[610] = 0;
	_flagsTable[603] = -_script.readNextWord();
}

void ToucheEngine::op_setRoomAreaState() {
	debugC(9, kDebugOpcodes, "ToucheEngine::op_setRoomAreaState()");
	int16 num = _script.readNextWord();
	int16 val = _script.readNextWord();
	setRoomAreaState(num, val);
}

} // namespace Touche
