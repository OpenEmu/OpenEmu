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



#include "agos/agos.h"
#include "agos/midi.h"

#include "common/textconsole.h"

namespace AGOS {

#define OPCODE(x)	_OPCODE(AGOSEngine_Simon2, x)

void AGOSEngine_Simon2::setupOpcodes() {
	static const OpcodeEntrySimon2 opcodes[] = {
		/* 00 */
		OPCODE(o_invalid),
		OPCODE(o_at),
		OPCODE(o_notAt),
		OPCODE(o_invalid),
		/* 04 */
		OPCODE(o_invalid),
		OPCODE(o_carried),
		OPCODE(o_notCarried),
		OPCODE(o_isAt),
		/* 08 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_zero),
		/* 12 */
		OPCODE(o_notZero),
		OPCODE(o_eq),
		OPCODE(o_notEq),
		OPCODE(o_gt),
		/* 16 */
		OPCODE(o_lt),
		OPCODE(o_eqf),
		OPCODE(o_notEqf),
		OPCODE(o_ltf),
		/* 20 */
		OPCODE(o_gtf),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_chance),
		/* 24 */
		OPCODE(o_invalid),
		OPCODE(o_isRoom),
		OPCODE(o_isObject),
		OPCODE(o_state),
		/* 28 */
		OPCODE(o_oflag),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_destroy),
		/* 32 */
		OPCODE(o_invalid),
		OPCODE(o_place),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 36 */
		OPCODE(o_copyff),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 40 */
		OPCODE(o_invalid),
		OPCODE(o_clear),
		OPCODE(o_let),
		OPCODE(o_add),
		/* 44 */
		OPCODE(o_sub),
		OPCODE(o_addf),
		OPCODE(o_subf),
		OPCODE(o_mul),
		/* 48 */
		OPCODE(o_div),
		OPCODE(o_mulf),
		OPCODE(o_divf),
		OPCODE(o_mod),
		/* 52 */
		OPCODE(o_modf),
		OPCODE(o_random),
		OPCODE(o_invalid),
		OPCODE(o_goto),
		/* 56 */
		OPCODE(o_oset),
		OPCODE(o_oclear),
		OPCODE(o_putBy),
		OPCODE(o_inc),
		/* 60 */
		OPCODE(o_dec),
		OPCODE(o_setState),
		OPCODE(o_print),
		OPCODE(o_message),
		/* 64 */
		OPCODE(o_msg),
		OPCODE(oww_addTextBox),
		OPCODE(oww_setShortText),
		OPCODE(oww_setLongText),
		/* 68 */
		OPCODE(o_end),
		OPCODE(o_done),
		OPCODE(os2_printLongText),
		OPCODE(o_process),
		/* 72 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 76 */
		OPCODE(o_when),
		OPCODE(o_if1),
		OPCODE(o_if2),
		OPCODE(o_isCalled),
		/* 80 */
		OPCODE(o_is),
		OPCODE(o_invalid),
		OPCODE(o_debug),
		OPCODE(os2_rescan),
		/* 84 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_comment),
		/* 88 */
		OPCODE(o_haltAnimation),
		OPCODE(o_restartAnimation),
		OPCODE(o_getParent),
		OPCODE(o_getNext),
		/* 92 */
		OPCODE(o_getChildren),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 96 */
		OPCODE(o_picture),
		OPCODE(o_loadZone),
		OPCODE(os2_animate),
		OPCODE(os2_stopAnimate),
		/* 100 */
		OPCODE(o_killAnimate),
		OPCODE(o_defWindow),
		OPCODE(o_window),
		OPCODE(o_cls),
		/* 104 */
		OPCODE(o_closeWindow),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_addBox),
		/* 108 */
		OPCODE(o_delBox),
		OPCODE(o_enableBox),
		OPCODE(o_disableBox),
		OPCODE(o_moveBox),
		/* 112 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_doIcons),
		OPCODE(o_isClass),
		/* 116 */
		OPCODE(o_setClass),
		OPCODE(o_unsetClass),
		OPCODE(o_invalid),
		OPCODE(o_waitSync),
		/* 120 */
		OPCODE(o_sync),
		OPCODE(o_defObj),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 124 */
		OPCODE(o_invalid),
		OPCODE(o_here),
		OPCODE(o_doClassIcons),
		OPCODE(os2_playTune),
		/* 128 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_setAdjNoun),
		OPCODE(o_invalid),
		/* 132 */
		OPCODE(o_saveUserGame),
		OPCODE(o_loadUserGame),
		OPCODE(o_invalid),
		OPCODE(os1_pauseGame),
		/* 136 */
		OPCODE(o_copysf),
		OPCODE(o_restoreIcons),
		OPCODE(o_freezeZones),
		OPCODE(o_placeNoIcons),
		/* 140 */
		OPCODE(o_clearTimers),
		OPCODE(o_setDollar),
		OPCODE(o_isBox),
		OPCODE(oe2_doTable),
		/* 144 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 148 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oe2_storeItem),
		/* 152 */
		OPCODE(oe2_getItem),
		OPCODE(oe2_bSet),
		OPCODE(oe2_bClear),
		OPCODE(oe2_bZero),
		/* 156C */
		OPCODE(oe2_bNotZero),
		OPCODE(oe2_getOValue),
		OPCODE(oe2_setOValue),
		OPCODE(o_invalid),
		/* 160 */
		OPCODE(oe2_ink),
		OPCODE(os1_screenTextBox),
		OPCODE(os1_screenTextMsg),
		OPCODE(os1_playEffect),
		/* 164 */
		OPCODE(oe2_getDollar2),
		OPCODE(oe2_isAdjNoun),
		OPCODE(oe2_b2Set),
		OPCODE(oe2_b2Clear),
		/* 168 */
		OPCODE(oe2_b2Zero),
		OPCODE(oe2_b2NotZero),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 172 */
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		OPCODE(oww_lockZones),
		/* 176 */
		OPCODE(oww_unlockZones),
		OPCODE(os2_screenTextPObj),
		OPCODE(os1_getPathPosn),
		OPCODE(os1_scnTxtLongText),
		/* 180 */
		OPCODE(os2_mouseOn),
		OPCODE(os2_mouseOff),
		OPCODE(o_invalid),
		OPCODE(o_invalid),
		/* 184 */
		OPCODE(os1_unloadZone),
		OPCODE(o_invalid),
		OPCODE(os1_unfreezeZones),
		OPCODE(o_invalid),
		/* 188 */
		OPCODE(os2_isShortText),
		OPCODE(os2_clearMarks),
		OPCODE(os2_waitMark),
	};

	_opcodesSimon2 = opcodes;
	_numOpcodes = 191;
}

void AGOSEngine_Simon2::executeOpcode(int opcode) {
	OpcodeProcSimon2 op = _opcodesSimon2[opcode].proc;
	(this->*op) ();
}

// -----------------------------------------------------------------------
// Simon 2 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine_Simon2::os2_printLongText() {
	// 70: show string from array
	const char *str = (const char *)getStringPtrByID(_longText[getVarOrByte()]);
	writeVariable(51, strlen(str) / 53 * 8 + 8);
	showMessageFormat("%s\n", str);
}

void AGOSEngine_Simon2::os2_rescan() {
	// 83: restart subroutine
	if (_exitCutscene) {
		if (getBitFlag(9)) {
			endCutscene();
		}
	} else {
		processSpecialKeys();
	}

	setScriptReturn(-10);
}

void AGOSEngine_Simon2::os2_animate() {
	// 98: start vga
	uint16 zoneNum = getVarOrWord();
	uint16 vgaSpriteId = getVarOrWord();
	uint16 windowNum = getVarOrByte();
	int16 x = getVarOrWord();
	int16 y = getVarOrWord();
	uint16 palette = (getVarOrWord() & 15);

	_videoLockOut |= 0x40;
	animate(windowNum, zoneNum, vgaSpriteId, x, y, palette);
	_videoLockOut &= ~0x40;
}

void AGOSEngine_Simon2::os2_stopAnimate() {
	// 99: kill sprite
	uint16 a = getVarOrWord();
	uint16 b = getVarOrWord();
	stopAnimateSimon2(a, b);
}

void AGOSEngine_Simon2::os2_playTune() {
	// 127: deals with music
	int music = getVarOrWord();
	int track = getVarOrWord();
	int loop = getVarOrByte();

	// Jamieson630:
	// This appears to be a "load or play music" command.
	// The music resource is specified, and optionally
	// a track as well. Normally we see two calls being
	// made, one to load the resource and another to
	// actually start a track (so the resource is
	// effectively preloaded so there's no latency when
	// starting playback).

	_midi->setLoop(loop != 0);
	if (_lastMusicPlayed != music)
		_nextMusicToPlay = music;
	else
		_midi->startTrack(track);
}

void AGOSEngine_Simon2::os2_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);
	if (getFeatures() & GF_TALKIE) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint speechId = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFVoice)];

			if (subObject->objectFlags & kOFNumber) {
				uint speechIdOffs = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];

				if (speechId == 116)
					speechId = speechIdOffs + 115;
				if (speechId == 92)
					speechId = speechIdOffs + 98;
				if (speechId == 99)
					speechId = 9;
				if (speechId == 97) {
					switch (speechIdOffs) {
					case 12:
						speechId = 109;
						break;
					case 14:
						speechId = 108;
						break;
					case 18:
						speechId = 107;
						break;
					case 20:
						speechId = 106;
						break;
					case 22:
						speechId = 105;
						break;
					case 28:
						speechId = 104;
						break;
					case 90:
						speechId = 103;
						break;
					case 92:
						speechId = 102;
						break;
					case 100:
						speechId = 51;
						break;
					default:
						error("os2_screenTextPObj: invalid case %d", speechIdOffs);
					}
				}
			}

			if (_speech)
				playSpeech(speechId, vgaSpriteId);
		}

	}

	if (subObject != NULL && subObject->objectFlags & kOFText && _subtitles) {
		const char *stringPtr = (const char *)getStringPtrByID(subObject->objectFlagValue[0]);
		TextLocation *tl = getTextLocation(vgaSpriteId);
		char buf[256];
		int j, k;

		if (subObject->objectFlags & kOFNumber) {
			if (_language == Common::HE_ISR) {
				j = subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)];
				k = (j % 10) * 10;
				k += j / 10;
				if (!(j % 10))
					sprintf(buf,"0%d%s", k, stringPtr);
				else
					sprintf(buf,"%d%s", k, stringPtr);
			} else {
				sprintf(buf,"%d%s", subObject->objectFlagValue[getOffsetOfChild2Param(subObject, kOFNumber)], stringPtr);
			}
			stringPtr = buf;
		}
		if (stringPtr != NULL)
			printScreenText(vgaSpriteId, color, stringPtr, tl->x, tl->y, tl->width);
	}
}

void AGOSEngine_Simon2::os2_mouseOn() {
	// 180: force mouseOn
	if (getGameType() == GType_SIMON2 && getBitFlag(79)) {
		_mouseCursor = 0;
	}
	_mouseHideCount = 0;
}

void AGOSEngine_Simon2::os2_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
	changeWindow(1);
	showMessageFormat("\xC");
}

void AGOSEngine_Simon2::os2_isShortText() {
	// 188: string2 is
	uint i = getVarOrByte();
	uint str = getNextStringID();
	setScriptCondition(str < _numTextBoxes && _shortText[i] == str);
}

void AGOSEngine_Simon2::os2_clearMarks() {
	// 189: clear_op189_flag
	_marks = 0;
}

void AGOSEngine_Simon2::os2_waitMark() {
	// 190
	uint i = getVarOrByte();
	if (!(_marks & (1 << i)))
		waitForMark(i);
}

void AGOSEngine::stopAnimateSimon2(uint16 a, uint16 b) {
	uint16 items[2];

	items[0] = to16Wrapper(a);
	items[1] = to16Wrapper(b);

	_videoLockOut |= 0x8000;
	_vcPtr = (byte *)&items;
	vc60_stopAnimation();
	_videoLockOut &= ~0x8000;
}

void AGOSEngine::waitForMark(uint i) {
	_exitCutscene = false;
	while (!(_marks & (1 << i))) {
		if (_exitCutscene) {
			if (getGameType() == GType_PP) {
				if (_picture8600)
					break;
			} else {
				if (getBitFlag(9)) {
					endCutscene();
					break;
				}
			}
		} else {
			processSpecialKeys();
		}

		delay(10);
	}
}

} // End of namespace AGOS
