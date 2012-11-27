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
#include "common/localization.h"

#include "graphics/palette.h"

#include "agos/agos.h"

#ifdef _WIN32_WCE
extern bool isSmartphone();
#endif

namespace AGOS {

#define OPCODE(x)	_OPCODE(AGOSEngine_Simon1, x)

void AGOSEngine_Simon1::setupOpcodes() {
	static const OpcodeEntrySimon1 opcodes[] = {
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
		OPCODE(oww_printLongText),
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
		OPCODE(oe1_rescan),
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
		OPCODE(os1_animate),
		OPCODE(oe1_stopAnimate),
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
		OPCODE(o_playTune),
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
		/* 156 */
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
		OPCODE(os1_screenTextPObj),
		OPCODE(os1_getPathPosn),
		OPCODE(os1_scnTxtLongText),
		/* 180 */
		OPCODE(os1_mouseOn),
		OPCODE(os1_mouseOff),
		OPCODE(os1_loadBeard),
		OPCODE(os1_unloadBeard),
		/* 184 */
		OPCODE(os1_unloadZone),
		OPCODE(os1_loadStrings),
		OPCODE(os1_unfreezeZones),
		OPCODE(os1_specialFade),
	};

	_opcodesSimon1 = opcodes;
	_numOpcodes = 188;
}

void AGOSEngine_Simon1::executeOpcode(int opcode) {
	OpcodeProcSimon1 op = _opcodesSimon1[opcode].proc;
	(this->*op) ();
}

// -----------------------------------------------------------------------
// Simon 1 Opcodes
// -----------------------------------------------------------------------

void AGOSEngine_Simon1::os1_animate() {
	// 98: animate
	uint16 vgaSpriteId = getVarOrWord();
	uint16 windowNum = getVarOrByte();
	int16 x = getVarOrWord();
	int16 y = getVarOrWord();
	uint16 palette = (getVarOrWord() & 15);

	if (getFeatures() & GF_TALKIE && vgaSpriteId >= 400) {
		_lastVgaWaitFor = 0;
	}

	_videoLockOut |= 0x40;
	animate(windowNum, vgaSpriteId / 100, vgaSpriteId, x, y, palette);
	_videoLockOut &= ~0x40;
}

void AGOSEngine_Simon1::os1_pauseGame() {
	// 135: pause game
	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	Common::KeyCode keyYes, keyNo;

	Common::getLanguageYesNo(_language, keyYes, keyNo);

	while (!shouldQuit()) {
		delay(1);
		if (_keyPressed.keycode == keyYes)
			quitGame();
		else if (_keyPressed.keycode == keyNo)
			break;
	}

	_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);
}

void AGOSEngine_Simon1::os1_screenTextBox() {
	// 161: setup text
	TextLocation *tl = getTextLocation(getVarOrByte());

	tl->x = getVarOrWord();
	tl->y = getVarOrByte();
	tl->width = getVarOrWord();
}

void AGOSEngine_Simon1::os1_screenTextMsg() {
	// 162: print string
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	uint stringId = getNextStringID();
	const byte *stringPtr = NULL;
	uint speechId = 0;
	TextLocation *tl;

	if (stringId != 0xFFFF)
		stringPtr = getStringPtrByID(stringId);

	if (getFeatures() & GF_TALKIE) {
		if (getGameType() == GType_FF || getGameType() == GType_PP)
			speechId = (uint16)getVarOrWord();
		else
			speechId = (uint16)getNextWord();
	}

	if (getGameType() == GType_FF || getGameType() == GType_PP)
		vgaSpriteId = 1;

	tl = getTextLocation(vgaSpriteId);
	if (_speech && speechId != 0)
		playSpeech(speechId, vgaSpriteId);
	if (((getGameType() == GType_SIMON2 && (getFeatures() & GF_TALKIE)) || getGameType() == GType_FF) &&
		speechId == 0) {
		stopAnimateSimon2(2, vgaSpriteId + 2);
	}

	// WORKAROUND: Several strings in the French version of Simon the Sorcerer 1 set the incorrect width,
	// causing crashes, or glitches in subtitles. See bug #3512776 for example.
	if (getGameType() == GType_SIMON1 && _language == Common::FR_FRA) {
		if ((getFeatures() & GF_TALKIE) && stringId == 33219)
			tl->width = 96;
		if (!(getFeatures() & GF_TALKIE) && stringId == 33245)
			tl->width = 96;
	}

	if (stringPtr != NULL && stringPtr[0] != 0 && (speechId == 0 || _subtitles))
		printScreenText(vgaSpriteId, color, (const char *)stringPtr, tl->x, tl->y, tl->width);

}

void AGOSEngine_Simon1::os1_playEffect() {
	// 163: play sound
	uint16 soundId = getVarOrWord();

	if (getGameId() == GID_SIMON1DOS)
		playSting(soundId);
	else
		_sound->playEffects(soundId);
}

void AGOSEngine_Simon1::os1_screenTextPObj() {
	// 177: inventory descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();

	SubObject *subObject = (SubObject *)findChildOfType(getNextItemPtr(), kObjectType);
	if (getFeatures() & GF_TALKIE) {
		if (subObject != NULL && subObject->objectFlags & kOFVoice) {
			uint offs = getOffsetOfChild2Param(subObject, kOFVoice);
			playSpeech(subObject->objectFlagValue[offs], vgaSpriteId);
		} else if (subObject != NULL && subObject->objectFlags & kOFNumber) {
			uint offs = getOffsetOfChild2Param(subObject, kOFNumber);
			playSpeech(subObject->objectFlagValue[offs] + 3550, vgaSpriteId);
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
		if (stringPtr != NULL && stringPtr[0] != 0)
			printScreenText(vgaSpriteId, color, stringPtr, tl->x, tl->y, tl->width);
	}
}

void AGOSEngine_Simon1::os1_getPathPosn() {
	// 178: path find
	uint x = getVarOrWord();
	uint y = getVarOrWord();
	uint var_1 = getVarOrByte();
	uint var_2 = getVarOrByte();

	const uint16 *p;
	uint i, j;
	uint prev_i;
	uint x_diff, y_diff;
	uint best_i = 0, best_j = 0, best_dist = 0xFFFFFFFF;
	uint maxPath = (getGameType() == GType_FF || getGameType() == GType_PP) ? 100 : 20;

	if (getGameType() == GType_FF || getGameType() == GType_PP) {
		x += _scrollX;
		y += _scrollY;
	} else if (getGameType() == GType_SIMON2) {
		x += _scrollX * 8;
	}

	int end = (getGameType() == GType_FF) ? 9999 : 999;
	prev_i = maxPath + 1 - readVariable(12);
	for (i = maxPath; i != 0; --i) {
		p = (const uint16 *)_pathFindArray[maxPath - i];
		if (!p)
			continue;
		for (j = 0; readUint16Wrapper(&p[0]) != end; j++, p += 2) {
			x_diff = ABS((int16)(readUint16Wrapper(&p[0]) - x));
			y_diff = ABS((int16)(readUint16Wrapper(&p[1]) - 12 - y));

			if (x_diff < y_diff) {
				x_diff /= 4;
				y_diff *= 4;
			}
			x_diff += y_diff /= 4;

			if ((x_diff < best_dist) || ((x_diff == best_dist) && (prev_i == i))) {
				best_dist = x_diff;
				best_i = maxPath + 1 - i;
				best_j = j;
			}
		}
	}

	writeVariable(var_1, best_i);
	writeVariable(var_2, best_j);
}

void AGOSEngine_Simon1::os1_scnTxtLongText() {
	// 179: conversation responses and room descriptions
	uint vgaSpriteId = getVarOrByte();
	uint color = getVarOrByte();
	uint stringId = getVarOrByte();
	uint speechId = 0;
	TextLocation *tl;

	const char *stringPtr = (const char *)getStringPtrByID(_longText[stringId]);
	if (getFeatures() & GF_TALKIE)
		speechId = _longSound[stringId];

	if (getGameType() == GType_FF || getGameType() == GType_PP)
		vgaSpriteId = 1;
	tl = getTextLocation(vgaSpriteId);

	if (_speech && speechId != 0)
		playSpeech(speechId, vgaSpriteId);
	if (stringPtr != NULL && stringPtr[0] != 0 && _subtitles)
		printScreenText(vgaSpriteId, color, stringPtr, tl->x, tl->y, tl->width);
}

void AGOSEngine_Simon1::os1_mouseOn() {
	// 180: force mouseOn
	_mouseHideCount = 0;
}

void AGOSEngine_Simon1::os1_mouseOff() {
	// 181: force mouseOff
	scriptMouseOff();
}

void AGOSEngine_Simon1::os1_loadBeard() {
	// 182: load beard
	if (_beardLoaded == false) {
		_beardLoaded = true;
		_videoLockOut |= 0x8000;
		loadVGABeardFile(328);
		_videoLockOut &= ~0x8000;
	}
}

void AGOSEngine_Simon1::os1_unloadBeard() {
	// 183: unload beard
	if (_beardLoaded == true) {
		_beardLoaded = false;
		_videoLockOut |= 0x8000;
		loadVGABeardFile(23);
		_videoLockOut &= ~0x8000;
	}
}

void AGOSEngine_Simon1::os1_unloadZone() {
	// 184: unload zone
	uint a = getVarOrWord();
	VgaPointersEntry *vpe = &_vgaBufferPointers[a];

	vpe->sfxFile = NULL;
	vpe->vgaFile1 = NULL;
	vpe->vgaFile2 = NULL;
}

void AGOSEngine_Simon1::os1_loadStrings() {
	// 185: load sound files
	_soundFileId = getVarOrWord();
	if (getPlatform() == Common::kPlatformAmiga && (getFeatures() & GF_TALKIE)) {
		char buf[10];
		sprintf(buf, "%d%s", _soundFileId, "Effects");
		_sound->readSfxFile(buf);
		sprintf(buf, "%d%s", _soundFileId, "simon");
		_sound->readVoiceFile(buf);
	}
}

void AGOSEngine_Simon1::os1_unfreezeZones() {
	// 186: freeze zone
	unfreezeBottom();
}

void AGOSEngine_Simon1::os1_specialFade() {
	// 187: fade to black
	uint i;

	for (i = 32; i != 0; --i) {
		paletteFadeOut(_currentPalette, 32, 8);
		paletteFadeOut(_currentPalette + 3 * 48, 144, 8);
		paletteFadeOut(_currentPalette + 3 * 208, 48, 8);
		_system->getPaletteManager()->setPalette(_currentPalette, 0, 256);
		delay(5);
	}

	memcpy(_displayPalette, _currentPalette, sizeof(_currentPalette));
}

void AGOSEngine::scriptMouseOff() {
	_videoLockOut |= 0x8000;
	vc34_setMouseOff();
	_videoLockOut &= ~0x8000;
}

} // End of namespace AGOS
