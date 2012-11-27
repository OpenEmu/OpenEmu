/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * Additional copyright for this file:
 * Copyright (C) 1994-1998 Revolution Software Ltd.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "common/file.h"
#include "common/textconsole.h"

#include "sword2/sword2.h"
#include "sword2/defs.h"
#include "sword2/header.h"
#include "sword2/console.h"
#include "sword2/logic.h"
#include "sword2/maketext.h"
#include "sword2/object.h"
#include "sword2/resman.h"
#include "sword2/screen.h"

namespace Sword2 {

// To request the status of a target, we run its 4th script, get-speech-state.
// This will cause RESULT to be set to either 1 (target is waiting) or 0
// (target is busy).

// Distance kept above talking sprite
#define GAP_ABOVE_HEAD 20

enum {
	S_OB_GRAPHIC	= 0,
	S_OB_SPEECH	= 1,
	S_OB_LOGIC	= 2,
	S_OB_MEGA	= 3,

	S_TEXT		= 4,
	S_WAV		= 5,
	S_ANIM		= 6,
	S_DIR_TABLE	= 7,
	S_ANIM_MODE	= 8
};

/**
 * Sets _textX and _textY for position of text sprite. Note that _textX is
 * also used to calculate speech pan.
 */

void Logic::locateTalker(int32 *params) {
	// params:	0 pointer to ob_graphic
	//		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 encoded text number
	//		5 wav res id
	//		6 anim res id
	//		7 pointer to anim table
	//		8 animation mode	0 lip synced,
	//					1 just straight animation

	if (!_animId) {
		// There is no animation. Assume it's voice-over text, and put
		// it at the bottom of the screen.

		_textX = 320;
		_textY = 400;
		return;
	}

	byte *file = _vm->_resman->openResource(_animId);

	// '0' means 1st frame

	CdtEntry cdt_entry;
	FrameHeader frame_head;

	cdt_entry.read(_vm->fetchCdtEntry(file, 0));
	frame_head.read(_vm->fetchFrameHeader(file, 0));

	// Note: This part of the code is quite similar to registerFrame().

	if (cdt_entry.frameType & FRAME_OFFSET) {
		// The frame has offsets, i.e. it's a scalable mega frame
		ObjectMega obMega(decodePtr(params[S_OB_MEGA]));

		uint16 scale = obMega.calcScale();

		// Calc suitable center point above the head, based on scaled
		// height

		// just use 'feet_x' as center
		_textX = obMega.getFeetX();

		// Add scaled y-offset to feet_y coord to get top of sprite
		_textY = obMega.getFeetY() + (cdt_entry.y * scale) / 256;
	} else {
		// It's a non-scaling anim - calc suitable center point above
		// the head, based on scaled width

		// x-coord + half of width
		_textX = cdt_entry.x + frame_head.width / 2;
		_textY = cdt_entry.y;
	}

	_vm->_resman->closeResource(_animId);

	// Leave space above their head
	_textY -= GAP_ABOVE_HEAD;

	// Adjust the text coords for RDSPR_DISPLAYALIGN

	ScreenInfo *screenInfo = _vm->_screen->getScreenInfo();

	_textX -= screenInfo->scroll_offset_x;
	_textY -= screenInfo->scroll_offset_y;
}

/**
 * This function is called the first time to build the text, if we need one. If
 * If necessary it also brings in the wav and sets up the animation.
 *
 * If there is an animation it can be repeating lip-sync or run-once.
 *
 * If there is no wav, then the text comes up instead. There can be any
 * combination of text/wav playing.
 */

void Logic::formText(int32 *params) {
	// params	0 pointer to ob_graphic
	//		1 pointer to ob_speech
	//		2 pointer to ob_logic
	//		3 pointer to ob_mega
	//		4 encoded text number
	//		5 wav res id
	//		6 anim res id
	//		7 pointer to anim table
	//		8 animation mode	0 lip synced,
	//					1 just straight animation

	// There should always be a text line, as all text is derived from it.
	// If there is none, that's bad...

	if (!params[S_TEXT]) {
		warning("No text line for speech wav %d", params[S_WAV]);
		return;
	}

	ObjectSpeech obSpeech(decodePtr(params[S_OB_SPEECH]));

	// Establish the max width allowed for this text sprite.
	uint32 textWidth = obSpeech.getWidth();

	if (!textWidth)
		textWidth = 400;

	// Pull out the text line, and make the sprite and text block

	uint32 text_res = params[S_TEXT] / SIZE;
	uint32 local_text = params[S_TEXT] & 0xffff;
	byte *text = _vm->fetchTextLine(_vm->_resman->openResource(text_res), local_text);

	// 'text + 2' to skip the first 2 bytes which form the line reference
	// number

	_speechTextBlocNo = _vm->_fontRenderer->buildNewBloc(
		text + 2, _textX, _textY,
		textWidth, obSpeech.getPen(),
		RDSPR_TRANS | RDSPR_DISPLAYALIGN,
		_vm->_speechFontId, POSITION_AT_CENTER_OF_BASE);

	_vm->_resman->closeResource(text_res);

	// Set speech duration, in case not using a wav.
	_speechTime = strlen((char *)text) + 30;
}

/**
 * There are some hard-coded cases where speech is used to illustrate a sound
 * effect. In this case there is no sound associated with the speech itself.
 */

bool Logic::wantSpeechForLine(uint32 wavId) {
	switch (wavId) {
	case 1328:	// AttendantSpeech
			//	SFX(Phone71);
			//	FX <Telephone rings>
	case 2059:	// PabloSpeech
			//	SFX (2059);
			//	FX <Sound of sporadic gunfire from below>
	case 4082:	// DuaneSpeech
			//	SFX (4082);
			//	FX <Pffffffffffft! Frp. (Unimpressive, flatulent noise.)>
	case 4214:	// cat_52
			//	SFX (4214);
			//	4214FXMeow!
	case 4568:	// trapdoor_13
			//	SFX (4568);
			//	4568fx<door slamming>
	case 4913:	// LobineauSpeech
			//	SFX (tone2);
			//	FX <Lobineau hangs up>
	case 5120:	// bush_66
			//	SFX (5120);
			//	5120FX<loud buzzing>
	case 528:	// PresidentaSpeech
			//	SFX (528);
			//	FX <Nearby Crash of Collapsing Masonry>
	case 920:	// Zombie Island forest maze (bird)
	case 923:	// Zombie Island forest maze (monkey)
	case 926:	// Zombie Island forest maze (zombie)
		// Don't want speech for these lines!
		return false;
	default:
		// Ok for all other lines
		return true;
	}
}

} // End of namespace Sword2
