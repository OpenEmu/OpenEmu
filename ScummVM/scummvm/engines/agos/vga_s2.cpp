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
#include "agos/intern.h"
#include "agos/midi.h"

#include "graphics/surface.h"

namespace AGOS {

void AGOSEngine_Simon2::setupVideoOpcodes(VgaOpcodeProc *op) {
	AGOSEngine_Simon1::setupVideoOpcodes(op);

	op[56] = &AGOSEngine::vc56_delayLong;
	op[58] = &AGOSEngine::vc58_changePriority;
	op[59] = &AGOSEngine::vc59_stopAnimations;
	op[64] = &AGOSEngine::vc64_ifSpeech;
	op[65] = &AGOSEngine::vc65_slowFadeIn;
	op[66] = &AGOSEngine::vc66_ifEqual;
	op[67] = &AGOSEngine::vc67_ifLE;
	op[68] = &AGOSEngine::vc68_ifGE;
	op[69] = &AGOSEngine::vc69_playSeq;
	op[70] = &AGOSEngine::vc70_joinSeq;
	op[71] = &AGOSEngine::vc71_ifSeqWaiting;
	op[72] = &AGOSEngine::vc72_segue;
	op[73] = &AGOSEngine::vc73_setMark;
	op[74] = &AGOSEngine::vc74_clearMark;
}

void AGOSEngine::vc56_delayLong() {
	uint16 num = vcReadVarOrWord() * _frameCount;

	addVgaEvent(num + _vgaBaseDelay, ANIMATE_EVENT, _vcPtr, _vgaCurSpriteId, _vgaCurZoneNum);
	_vcPtr = (byte *)&_vcGetOutOfCode;
}

void AGOSEngine::vc58_changePriority() {
	uint16 sprite = _vgaCurSpriteId;
	uint16 file = _vgaCurZoneNum;
	const byte *vcPtrOrg;
	uint16 tmp;

	_vgaCurZoneNum = vcReadNextWord();
	_vgaCurSpriteId = vcReadNextWord();

	tmp = to16Wrapper(vcReadNextWord());

	vcPtrOrg = _vcPtr;
	_vcPtr = (byte *)&tmp;
	vc23_setPriority();

	_vcPtr = vcPtrOrg;
	_vgaCurSpriteId = sprite;
	_vgaCurZoneNum = file;
}

void AGOSEngine::vc59_stopAnimations() {
	uint16 file = vcReadNextWord();
	uint16 start = vcReadNextWord();
	uint16 end = vcReadNextWord() + 1;

	do {
		vcStopAnimation(file, start);
	} while (++start != end);
}

void AGOSEngine::vc64_ifSpeech() {
	if ((getGameType() == GType_SIMON2 && _subtitles && _language != Common::HE_ISR) ||
		!_sound->isVoiceActive()) {
		vcSkipNextInstruction();
	}
}

void AGOSEngine::vc65_slowFadeIn() {
	_fastFadeInFlag = 624;
	_fastFadeCount = 208;
	if (_windowNum != 4) {
		_fastFadeInFlag = 768;
		_fastFadeCount = 256;
	}
	_fastFadeInFlag |= 0x8000;
	_fastFadeOutFlag = false;
}

void AGOSEngine::vc66_ifEqual() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();

	if (vcReadVar(a) != vcReadVar(b))
		vcSkipNextInstruction();
}

void AGOSEngine::vc67_ifLE() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();

	if (vcReadVar(a) >= vcReadVar(b))
		vcSkipNextInstruction();
}

void AGOSEngine::vc68_ifGE() {
	uint16 a = vcReadNextWord();
	uint16 b = vcReadNextWord();

	if (vcReadVar(a) <= vcReadVar(b))
		vcSkipNextInstruction();
}

void AGOSEngine::vc69_playSeq() {
	int16 track = vcReadNextWord();
	int16 loop = vcReadNextWord();

	// Jamieson630:
	// This is a "play track". The original
	// design stored the track to play if one was
	// already in progress, so that the next time a
	// "fill MIDI stream" event occurred, the MIDI
	// player would find the change and switch
	// tracks. We use a different architecture that
	// allows for an immediate response here, but
	// we'll simulate the variable changes so other
	// scripts don't get thrown off.
	// NOTE: This opcode looks very similar in function
	// to vc72(), except that vc72() may allow for
	// specifying a non-valid track number (999 or -1)
	// as a means of stopping what music is currently
	// playing.
	_midi->setLoop(loop != 0);
	_midi->startTrack(track);
}

void AGOSEngine::vc70_joinSeq() {
	// Simon2
	uint16 track = vcReadNextWord();
	uint16 loop = vcReadNextWord();

	// Jamieson630:
	// This sets the "on end of track" action.
	// It specifies whether to loop the current
	// track and, if not, whether to switch to
	// a different track upon completion.
	if (track != 0xFFFF && track != 999)
		_midi->queueTrack(track, loop != 0);
	else
		_midi->setLoop(loop != 0);
}

void AGOSEngine::vc71_ifSeqWaiting() {
	// Jamieson630:
	// This command skips the next instruction
	// unless (1) there is a track playing, AND
	// (2) there is a track queued to play after it.
	if (!_midi->isPlaying(true))
		vcSkipNextInstruction();
}

void AGOSEngine::vc72_segue() {
	// Jamieson630:
	// This is a "play or stop track". Note that
	// this opcode looks very similar in function
	// to vc69(), except that this opcode may allow
	// for specifying a track of 999 or -1 in order to
	// stop the music. We'll code it that way for now.

	// NOTE: It's possible that when "stopping" a track,
	// we're supposed to just go on to the next queued
	// track, if any. Must find out if there is ANY
	// case where this is used to stop a track in the
	// first place.

	int16 track = vcReadNextWord();
	int16 loop = vcReadNextWord();

	if (track == -1 || track == 999) {
		stopMusic();
	} else {
		_midi->setLoop(loop != 0);
		_midi->startTrack(track);
	}
}

void AGOSEngine::vc73_setMark() {
	_marks |= (1 << vcReadNextWord());
}

void AGOSEngine::vc74_clearMark() {
	_marks &= ~(1 << vcReadNextWord());
}

void AGOSEngine_Simon2::clearVideoWindow(uint16 num, uint16 color) {
	const uint16 *vlut = &_videoWindows[num * 4];

	uint16 xoffs = vlut[0] * 16;
	uint16 yoffs = vlut[1];
	uint16 dstWidth = _videoWindows[18] * 16;
	byte *dst = (byte *)_window4BackScn->pixels + xoffs + yoffs * dstWidth;

	setMoveRect(0, 0, vlut[2] * 16, vlut[3]);

	for (uint h = 0; h < vlut[3]; h++) {
		memset(dst, color, vlut[2] * 16);
		dst += dstWidth;
	}

	_window4Flag = 1;
}

} // End of namespace AGOS
