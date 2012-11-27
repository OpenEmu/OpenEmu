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

#include "sci/sci.h"
#include "sci/engine/script.h"
#include "sci/engine/state.h"

#include "common/util.h"

namespace Sci {

#define PATCH_END             0xFFFF
#define PATCH_COMMANDMASK     0xF000
#define PATCH_VALUEMASK       0x0FFF
#define PATCH_ADDTOOFFSET     0xE000
#define PATCH_GETORIGINALBYTE 0xD000
#define PATCH_ADJUSTWORD      0xC000
#define PATCH_ADJUSTWORD_NEG  0xB000
#define PATCH_MAGICDWORD(a, b, c, d) CONSTANT_LE_32(a | (b << 8) | (c << 16) | (d << 24))
#define PATCH_VALUELIMIT      4096

struct SciScriptSignature {
	uint16 scriptNr;
	const char *description;
	int16 applyCount;
	uint32 magicDWord;
	int magicOffset;
	const byte *data;
	const uint16 *patch;
};

#define SCI_SIGNATUREENTRY_TERMINATOR { 0, NULL, 0, 0, 0, NULL, NULL }

// signatures are built like this:
//  - first a counter of the bytes that follow
//  - then the actual bytes that need to get matched
//  - then another counter of bytes (0 for EOS)
//  - if not EOS, an adjust offset and the actual bytes
//  - rinse and repeat

// ===========================================================================
// stayAndHelp::changeState (0) is called when ego swims to the left or right
//  boundaries of room 660. Normally a textbox is supposed to get on screen
//  but the call is wrong, so not only do we get an error message the script
//  is also hanging because the cue won't get sent out
//  This also happens in sierra sci - ffs. bug #3038387
const byte ecoquest1SignatureStayAndHelp[] = {
	40,
	0x3f, 0x01,        // link 01
	0x87, 0x01,        // lap param[1]
	0x65, 0x14,        // aTop state
	0x36,              // push
	0x3c,              // dup
	0x35, 0x00,        // ldi 00
	0x1a,              // eq?
	0x31, 0x1c,        // bnt [next state]
	0x76,              // push0
	0x45, 0x01, 0x00,  // callb export1 from script 0 (switching control off)
	0x38, 0x22, 0x01,  // pushi 0122
	0x78,              // push1
	0x76,              // push0
	0x81, 0x00,        // lag global[0]
	0x4a, 0x06,        // send 06 - call ego::setMotion(0)
	0x39, 0x6e,        // pushi 6e (selector init)
	0x39, 0x04,        // pushi 04
	0x76,              // push0
	0x76,              // push0
	0x39, 0x17,        // pushi 17
	0x7c,              // pushSelf
	0x51, 0x82,        // class EcoNarrator
	0x4a, 0x0c,        // send 0c - call EcoNarrator::init(0, 0, 23, self) (BADLY BROKEN!)
	0x33,              // jmp [end]
	0
};

const uint16 ecoquest1PatchStayAndHelp[] = {
	0x87, 0x01,        // lap param[1]
	0x65, 0x14,        // aTop state
	0x36,              // push
	0x2f, 0x22,        // bt [next state] (this optimization saves 6 bytes)
	0x39, 0x00,        // pushi 0 (wasting 1 byte here)
	0x45, 0x01, 0x00,  // callb export1 from script 0 (switching control off)
	0x38, 0x22, 0x01,  // pushi 0122
	0x78,              // push1
	0x76,              // push0
	0x81, 0x00,        // lag global[0]
	0x4a, 0x06,        // send 06 - call ego::setMotion(0)
	0x39, 0x6e,        // pushi 6e (selector init)
	0x39, 0x06,        // pushi 06
	0x39, 0x02,        // pushi 02 (additional 2 bytes)
	0x76,              // push0
	0x76,              // push0
	0x39, 0x17,        // pushi 17
	0x7c,              // pushSelf
	0x38, 0x80, 0x02,  // pushi 280 (additional 3 bytes)
	0x51, 0x82,        // class EcoNarrator
	0x4a, 0x10,        // send 10 - call EcoNarrator::init(2, 0, 0, 23, self, 640)
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature ecoquest1Signatures[] = {
	{    660, "CD: bad messagebox and freeze",               1, PATCH_MAGICDWORD(0x38, 0x22, 0x01, 0x78),   -17, ecoquest1SignatureStayAndHelp, ecoquest1PatchStayAndHelp },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// doMyThing::changeState (2) is supposed to remove the initial text on the
//  ecorder. This is done by reusing temp-space, that was filled on state 1.
//  this worked in sierra sci just by accident. In our sci, the temp space
//  is resetted every time, which means the previous text isn't available
//  anymore. We have to patch the code because of that ffs. bug #3035386
const byte ecoquest2SignatureEcorder[] = {
	35,
	0x31, 0x22,        // bnt [next state]
	0x39, 0x0a,        // pushi 0a
	0x5b, 0x04, 0x1e,  // lea temp[1e]
	0x36,              // push
	0x39, 0x64,        // pushi 64
	0x39, 0x7d,        // pushi 7d
	0x39, 0x32,        // pushi 32
	0x39, 0x66,        // pushi 66
	0x39, 0x17,        // pushi 17
	0x39, 0x69,        // pushi 69
	0x38, 0x31, 0x26,  // pushi 2631
	0x39, 0x6a,        // pushi 6a
	0x39, 0x64,        // pushi 64
	0x43, 0x1b, 0x14,  // call kDisplay
	0x35, 0x0a,        // ldi 0a
	0x65, 0x20,        // aTop ticks
	0x33,              // jmp [end]
	+1, 5,             // [skip 1 byte]
	0x3c,              // dup
	0x35, 0x03,        // ldi 03
	0x1a,              // eq?
	0x31,              // bnt [end]
	0
};

const uint16 ecoquest2PatchEcorder[] = {
	0x2f, 0x02,        // bt [to pushi 07]
	0x3a,              // toss
	0x48,              // ret
	0x38, 0x07, 0x00,  // pushi 07 (parameter count) (waste 1 byte)
	0x39, 0x0b,        // push (FillBoxAny)
	0x39, 0x1d,        // pushi 29d
	0x39, 0x73,        // pushi 115d
	0x39, 0x5e,        // pushi 94d
	0x38, 0xd7, 0x00,  // pushi 215d
	0x78,              // push1 (visual screen)
	0x38, 0x17, 0x00,  // pushi 17 (color) (waste 1 byte)
	0x43, 0x6c, 0x0e,  // call kGraph
	0x38, 0x05, 0x00,  // pushi 05 (parameter count) (waste 1 byte)
	0x39, 0x0c,        // pushi 12d (UpdateBox)
	0x39, 0x1d,        // pushi 29d
	0x39, 0x73,        // pushi 115d
	0x39, 0x5e,        // pushi 94d
	0x38, 0xd7, 0x00,  // pushi 215d
	0x43, 0x6c, 0x0a,  // call kGraph
	PATCH_END
};

// ===========================================================================
// Same patch as above for the ecorder introduction. Fixes bug #3092115.
// Two workarounds are needed for this patch in workarounds.cpp (when calling
// kGraphFillBoxAny and kGraphUpdateBox), as there isn't enough space to patch
// the function otherwise.
const byte ecoquest2SignatureEcorderTutorial[] = {
	36,
	0x30, 0x23, 0x00,  // bnt [next state]
	0x39, 0x0a,        // pushi 0a
	0x5b, 0x04, 0x1f,  // lea temp[1f]
	0x36,              // push
	0x39, 0x64,        // pushi 64
	0x39, 0x7d,        // pushi 7d
	0x39, 0x32,        // pushi 32
	0x39, 0x66,        // pushi 66
	0x39, 0x17,        // pushi 17
	0x39, 0x69,        // pushi 69
	0x38, 0x31, 0x26,  // pushi 2631
	0x39, 0x6a,        // pushi 6a
	0x39, 0x64,        // pushi 64
	0x43, 0x1b, 0x14,  // call kDisplay
	0x35, 0x1e,        // ldi 1e
	0x65, 0x20,        // aTop ticks
	0x32,              // jmp [end]
	// 2 extra bytes, jmp offset
	0
};

const uint16 ecoquest2PatchEcorderTutorial[] = {
	0x31, 0x23,        // bnt [next state] (save 1 byte)
	// The parameter count below should be 7, but we're out of bytes
	// to patch! A workaround has been added because of this
	0x78,              // push1 (parameter count)
	//0x39, 0x07,        // pushi 07 (parameter count)
	0x39, 0x0b,        // push (FillBoxAny)
	0x39, 0x1d,        // pushi 29d
	0x39, 0x73,        // pushi 115d
	0x39, 0x5e,        // pushi 94d
	0x38, 0xd7, 0x00,  // pushi 215d
	0x78,              // push1 (visual screen)
	0x39, 0x17,        // pushi 17 (color)
	0x43, 0x6c, 0x0e,  // call kGraph
	// The parameter count below should be 5, but we're out of bytes
	// to patch! A workaround has been added because of this
	0x78,              // push1 (parameter count)
	//0x39, 0x05,        // pushi 05 (parameter count)
	0x39, 0x0c,        // pushi 12d (UpdateBox)
	0x39, 0x1d,        // pushi 29d
	0x39, 0x73,        // pushi 115d
	0x39, 0x5e,        // pushi 94d
	0x38, 0xd7, 0x00,  // pushi 215d
	0x43, 0x6c, 0x0a,  // call kGraph
	// We are out of bytes to patch at this point,
	// so we skip 494 (0x1EE) bytes to reuse this code:
	// ldi 1e
	// aTop 20
	// jmp 030e (jump to end)
	0x32, 0xee, 0x01,  // skip 494 (0x1EE) bytes
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature ecoquest2Signatures[] = {
	{     50, "initial text not removed on ecorder",         1, PATCH_MAGICDWORD(0x39, 0x64, 0x39, 0x7d),    -8, ecoquest2SignatureEcorder, ecoquest2PatchEcorder },
	{    333, "initial text not removed on ecorder tutorial",1, PATCH_MAGICDWORD(0x39, 0x64, 0x39, 0x7d),    -9, ecoquest2SignatureEcorderTutorial, ecoquest2PatchEcorderTutorial },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// EventHandler::handleEvent in Demo Quest has a bug, and it jumps to the
// wrong address when an incorrect word is typed, therefore leading to an
// infinite loop. This script bug was not apparent in SSCI, probably because
// event handling was slightly different there, so it was never discovered.
// Fixes bug #3038870.
const byte fanmadeSignatureInfiniteLoop[] = {
	13,
	0x38, 0x4c, 0x00,  // pushi 004c
	0x39, 0x00,        // pushi 00
	0x87, 0x01,        // lap 01
	0x4b, 0x04,        // send 04
	0x18,              // not
	0x30, 0x2f, 0x00,  // bnt 002f  [06a5]	--> jmp ffbc  [0664] --> BUG! infinite loop
	0
};

const uint16 fanmadePatchInfiniteLoop[] = {
	PATCH_ADDTOOFFSET | +10,
	0x30, 0x32, 0x00,  // bnt 0032  [06a8] --> pushi 004c
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature fanmadeSignatures[] = {
	{    999, "infinite loop on typo",                       1, PATCH_MAGICDWORD(0x18, 0x30, 0x2f, 0x00),    -9, fanmadeSignatureInfiniteLoop, fanmadePatchInfiniteLoop },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
//  script 0 of freddy pharkas/CD PointsSound::check waits for a signal and if
//   no signal received will call kDoSound(0xD) which is a dummy in sierra sci
//   and ScummVM and will use acc (which is not set by the dummy) to trigger
//   sound disposal. This somewhat worked in sierra sci, because the sample
//   was already playing in the sound driver. In our case we would also stop
//   the sample from playing, so we patch it out
//   The "score" code is already buggy and sets volume to 0 when playing
const byte freddypharkasSignatureScoreDisposal[] = {
	10,
	0x67, 0x32,       // pTos 32 (selector theAudCount)
	0x78,             // push1
	0x39, 0x0d,       // pushi 0d
	0x43, 0x75, 0x02, // call kDoAudio
	0x1c,             // ne?
	0x31,             // bnt (-> to skip disposal)
	0
};

const uint16 freddypharkasPatchScoreDisposal[] = {
	0x34, 0x00, 0x00, // ldi 0000
	0x34, 0x00, 0x00, // ldi 0000
	0x34, 0x00, 0x00, // ldi 0000
	PATCH_END
};

//  script 235 of freddy pharkas rm235::init and sEnterFrom500::changeState
//   disable icon 7+8 of iconbar (CD only). When picking up the canister after
//   placing it down, the scripts will disable all the other icons. This results
//   in IconBar::disable doing endless loops even in sierra sci, because there
//   is no enabled icon left. We remove disabling of icon 8 (which is help),
//   this fixes the issue.
const byte freddypharkasSignatureCanisterHang[] = {
	12,
	0x38, 0xf1, 0x00, // pushi f1 (selector disable)
	0x7a,             // push2
	0x39, 0x07,       // pushi 07
	0x39, 0x08,       // pushi 08
	0x81, 0x45,       // lag 45
	0x4a, 0x08,       // send 08 - call IconBar::disable(7, 8)
	0
};

const uint16 freddypharkasPatchCanisterHang[] = {
	PATCH_ADDTOOFFSET | +3,
	0x78,             // push1
	PATCH_ADDTOOFFSET | +2,
	0x33, 0x00,       // ldi 00 (waste 2 bytes)
	PATCH_ADDTOOFFSET | +3,
	0x06,             // send 06 - call IconBar::disable(7)
	PATCH_END
};

//  script 215 of freddy pharkas lowerLadder::doit and highLadder::doit actually
//   process keyboard-presses when the ladder is on the screen in that room.
//   They strangely also call kGetEvent. Because the main User::doit also calls
//   kGetEvent, it's pure luck, where the event will hit. It's the same issue
//   as in QfG1VGA and if you turn dos-box to max cycles, and click around for
//   ego, sometimes clicks also won't get registered. Strangely it's not nearly
//   as bad as in our sci, but these differences may be caused by timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
const byte freddypharkasSignatureLadderEvent[] = {
	21,
	0x39, 0x6d,       // pushi 6d (selector new)
	0x76,             // push0
	0x38, 0xf5, 0x00, // pushi f5 (selector curEvent)
	0x76,             // push0
	0x81, 0x50,       // lag global[50]
	0x4a, 0x04,       // send 04 - read User::curEvent
	0x4a, 0x04,       // send 04 - call curEvent::new
	0xa5, 0x00,       // sat temp[0]
	0x38, 0x94, 0x00, // pushi 94 (selector localize)
	0x76,             // push0
	0x4a, 0x04,       // send 04 - call curEvent::localize
	0
};

const uint16 freddypharkasPatchLadderEvent[] = {
	0x34, 0x00, 0x00, // ldi 0000 (waste 3 bytes, overwrites first 2 pushes)
	PATCH_ADDTOOFFSET | +8,
	0xa5, 0x00,       // sat temp[0] (waste 2 bytes, overwrites 2nd send)
	PATCH_ADDTOOFFSET | +2,
	0x34, 0x00, 0x00, // ldi 0000
	0x34, 0x00, 0x00, // ldi 0000 (waste 6 bytes, overwrites last 3 opcodes)
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature freddypharkasSignatures[] = {
	{      0, "CD: score early disposal",                    1, PATCH_MAGICDWORD(0x39, 0x0d, 0x43, 0x75),    -3, freddypharkasSignatureScoreDisposal, freddypharkasPatchScoreDisposal },
	{    235, "CD: canister pickup hang",                   3, PATCH_MAGICDWORD(0x39, 0x07, 0x39, 0x08),     -4, freddypharkasSignatureCanisterHang,  freddypharkasPatchCanisterHang },
	{    320, "ladder event issue",                          2, PATCH_MAGICDWORD(0x6d, 0x76, 0x38, 0xf5),    -1, freddypharkasSignatureLadderEvent,   freddypharkasPatchLadderEvent },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// daySixBeignet::changeState (4) is called when the cop goes out and sets cycles to 220.
//  this is not enough time to get to the door, so we patch that to 23 seconds
const byte gk1SignatureDay6PoliceBeignet[] = {
	4,
	0x35, 0x04,        // ldi 04
	0x1a,              // eq?
	0x30,              // bnt [next state check]
	+2, 5,             // [skip 2 bytes, offset of bnt]
	0x38, 0x93, 0x00,  // pushi 93 (selector dispose)
	0x76,              // push0
	0x72,              // lofsa deskSarg
	+2, 9,             // [skip 2 bytes, offset of lofsa]
	0x4a, 0x04, 0x00,  // send 04
	0x34, 0xdc, 0x00,  // ldi 220
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	0
};

const uint16 gk1PatchDay6PoliceBeignet[] = {
	PATCH_ADDTOOFFSET | +16,
	0x34, 0x17, 0x00,  // ldi 23
	0x65, 0x1c,        // aTop seconds
	PATCH_END
};

// sargSleeping::changeState (8) is called when the cop falls asleep and sets cycles to 220.
//  this is not enough time to get to the door, so we patch it to 42 seconds
const byte gk1SignatureDay6PoliceSleep[] = {
	4,
	0x35, 0x08,        // ldi 08
	0x1a,              // eq?
	0x31,              // bnt [next state check]
	+1, 6,             // [skip 1 byte, offset of bnt]
	0x34, 0xdc, 0x00,  // ldi 220
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	0
};

const uint16 gk1PatchDay6PoliceSleep[] = {
	PATCH_ADDTOOFFSET | +5,
	0x34, 0x2a, 0x00,  // ldi 42
	0x65, 0x1c,        // aTop seconds
	PATCH_END
};

// startOfDay5::changeState (20h) - when gabriel goes to the phone the script will hang
const byte gk1SignatureDay5PhoneFreeze[] = {
	5,
	0x35, 0x03,        // ldi 03
	0x65, 0x1a,        // aTop cycles
	0x32,              // jmp [end]
	+2, 3,             // [skip 2 bytes, offset of jmp]
	0x3c,              // dup
	0x35, 0x21,        // ldi 21
	0
};

const uint16 gk1PatchDay5PhoneFreeze[] = {
	0x35, 0x06,        // ldi 06
	0x65, 0x20,        // aTop ticks
	PATCH_END
};

// Floppy version: Interrogation::dispose() compares an object reference
// (stored in the view selector) with a number, leading to a crash (this kind
// of comparison was not used in SCI32). The view selector is used to store
// both a view number (in some cases), and a view reference (in other cases).
// In the floppy version, the checks are in the wrong order, so there is a
// comparison between a number an an object. In the CD version, the checks are
// in the correct order, thus the comparison is correct, thus we use the code
// from the CD version in the floppy one.
const byte gk1SignatureInterrogationBug[] = {
	43,
	0x65, 0x4c,        // aTop 4c
	0x67, 0x50,        // pTos 50
	0x34, 0x10, 0x27,  // ldi 2710
	0x1e,              // gt?
	0x31, 0x08,        // bnt 08  [05a0]
	0x67, 0x50,        // pTos 50
	0x34, 0x10, 0x27,  // ldi 2710
	0x04,              // sub
	0x65, 0x50,        // aTop 50
	0x63, 0x50,        // pToa 50
	0x31, 0x15,        // bnt 15  [05b9]
	0x39, 0x0e,        // pushi 0e
	0x76,              // push0
	0x4a, 0x04, 0x00,  // send 0004
	0xa5, 0x00,        // sat 00
	0x38, 0x93, 0x00,  // pushi 0093
	0x76,              // push0
	0x63, 0x50,        // pToa 50
	0x4a, 0x04, 0x00,  // send 0004
	0x85, 0x00,        // lat 00
	0x65, 0x50,        // aTop 50
	0
};

const uint16 gk1PatchInterrogationBug[] = {
	0x65, 0x4c,        // aTop 4c
	0x63, 0x50,        // pToa 50
	0x31, 0x15,        // bnt 15  [05b9]
	0x39, 0x0e,        // pushi 0e
	0x76,              // push0
	0x4a, 0x04, 0x00,  // send 0004
	0xa5, 0x00,        // sat 00
	0x38, 0x93, 0x00,  // pushi 0093
	0x76,              // push0
	0x63, 0x50,        // pToa 50
	0x4a, 0x04, 0x00,  // send 0004
	0x85, 0x00,        // lat 00
	0x65, 0x50,        // aTop 50
	0x67, 0x50,        // pTos 50
	0x34, 0x10, 0x27,  // ldi 2710
	0x1e,              // gt?
	0x31, 0x08,        // bnt 08  [05b9]
	0x67, 0x50,        // pTos 50
	0x34, 0x10, 0x27,  // ldi 2710
	0x04,              // sub
	0x65, 0x50,        // aTop 50
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature gk1Signatures[] = {
	{     51, "interrogation bug",                           1, PATCH_MAGICDWORD(0x65, 0x4c, 0x67, 0x50),     0, gk1SignatureInterrogationBug, gk1PatchInterrogationBug },
	{    212, "day 5 phone freeze",                          1, PATCH_MAGICDWORD(0x35, 0x03, 0x65, 0x1a),     0, gk1SignatureDay5PhoneFreeze, gk1PatchDay5PhoneFreeze },
	{    230, "day 6 police beignet timer issue",            1, PATCH_MAGICDWORD(0x34, 0xdc, 0x00, 0x65),   -16, gk1SignatureDay6PoliceBeignet, gk1PatchDay6PoliceBeignet },
	{    230, "day 6 police sleep timer issue",              1, PATCH_MAGICDWORD(0x34, 0xdc, 0x00, 0x65),    -5, gk1SignatureDay6PoliceSleep, gk1PatchDay6PoliceSleep },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// at least during harpy scene export 29 of script 0 is called in kq5cd and
//  has an issue for those calls, where temp 3 won't get inititialized, but
//  is later used to set master volume. This issue makes sierra sci set
//  the volume to max. We fix the export, so volume won't get modified in
//  those cases.
const byte kq5SignatureCdHarpyVolume[] = {
	34,
	0x80, 0x91, 0x01,  // lag global[191h]
	0x18,              // not
	0x30, 0x2c, 0x00,  // bnt [jump further] (jumping, if global 191h is 1)
	0x35, 0x01,        // ldi 01
	0xa0, 0x91, 0x01,  // sag global[191h] (setting global 191h to 1)
	0x38, 0x7b, 0x01,  // pushi 017b
	0x76,              // push0
	0x81, 0x01,        // lag global[1]
	0x4a, 0x04,        // send 04 - read KQ5::masterVolume
	0xa5, 0x03,        // sat temp[3] (store volume in temp 3)
	0x38, 0x7b, 0x01,  // pushi 017b
	0x76,              // push0
	0x81, 0x01,        // lag global[1]
	0x4a, 0x04,        // send 04 - read KQ5::masterVolume
	0x36,              // push
	0x35, 0x04,        // ldi 04
	0x20,              // ge? (followed by bnt)
	0
};

const uint16 kq5PatchCdHarpyVolume[] = {
	0x38, 0x2f, 0x02,  // pushi 022f (selector theVol) (3 new bytes)
	0x76,              // push0 (1 new byte)
	0x51, 0x88,        // class SpeakTimer (2 new bytes)
	0x4a, 0x04,        // send 04 (2 new bytes) -> read SpeakTimer::theVol
	0xa5, 0x03,        // sat temp[3] (2 new bytes) -> write to temp 3
	0x80, 0x91, 0x01,  // lag global[191h]
	// saving 1 byte due optimization
	0x2e, 0x23, 0x00,  // bt [jump further] (jumping, if global 191h is 1)
	0x35, 0x01,        // ldi 01
	0xa0, 0x91, 0x01,  // sag global[191h] (setting global 191h to 1)
	0x38, 0x7b, 0x01,  // pushi 017b
	0x76,              // push0
	0x81, 0x01,        // lag global[1]
	0x4a, 0x04,        // send 04 - read KQ5::masterVolume
	0xa5, 0x03,        // sat temp[3] (store volume in temp 3)
	// saving 8 bytes due removing of duplicate code
	0x39, 0x04,        // pushi 04 (saving 1 byte due swapping)
	0x22,              // lt? (because we switched values)
	PATCH_END
};

// This is a heap patch, and it modifies the properties of an object, instead
// of patching script code.
//
// The witchCage object in script 200 is broken and claims to have 12
// variables instead of the 8 it should have because it is a Cage.
// Additionally its top,left,bottom,right properties are set to 0 rather
// than the right values. We fix the object by setting the right values.
// If they are all zero, this causes an impossible position check in
// witch::cantBeHere and an infinite loop when entering room 22 (bug #3034714).
//
// This bug is accidentally not triggered in SSCI because the invalid number
// of variables effectively hides witchCage::doit, causing this position check
// to be bypassed entirely.
// See also the warning+comment in Object::initBaseObject
const byte kq5SignatureWitchCageInit[] = {
	16,
	0x00, 0x00,	// top
	0x00, 0x00,	// left
	0x00, 0x00,	// bottom
	0x00, 0x00,	// right
	0x00, 0x00,	// extra property #1
	0x7a, 0x00,	// extra property #2
	0xc8, 0x00,	// extra property #3
	0xa3, 0x00,	// extra property #4
	0
};

const uint16 kq5PatchWitchCageInit[] = {
	0x00, 0x00,	// top
	0x7a, 0x00,	// left
	0xc8, 0x00,	// bottom
	0xa3, 0x00,	// right
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature kq5Signatures[] = {
	{      0, "CD: harpy volume change",                     1, PATCH_MAGICDWORD(0x80, 0x91, 0x01, 0x18),     0, kq5SignatureCdHarpyVolume, kq5PatchCdHarpyVolume },
	{    200, "CD: witch cage init",                         1, PATCH_MAGICDWORD(0x7a, 0x00, 0xc8, 0x00),   -10, kq5SignatureWitchCageInit, kq5PatchWitchCageInit },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// When giving the milk bottle to one of the babies in the garden in KQ6 (room
// 480), script 481 starts a looping baby cry sound. However, that particular
// script also has an overriden check method (cryMusic::check). This method
// explicitly restarts the sound, even if it's set to be looped, thus the same
// sound is played twice, squelching all other sounds. We just rip the
// unnecessary cryMusic::check method out, thereby stopping the sound from
// constantly restarting (since it's being looped anyway), thus the normal
// game speech can work while the baby cry sound is heard. Fixes bug #3034579.
const byte kq6SignatureDuplicateBabyCry[] = {
	10,
	0x83, 0x00,         // lal 00
    0x31, 0x1e,         // bnt 1e  [07f4]
    0x78,               // push1
    0x39, 0x04,         // pushi 04
    0x43, 0x75, 0x02,   // callk DoAudio[75] 02
	0
};

const uint16 kq6PatchDuplicateBabyCry[] = {
	0x48,              // ret
	PATCH_END
};

//    script, description,                                      magic DWORD,                                 adjust
const SciScriptSignature kq6Signatures[] = {
	{    481, "duplicate baby cry",                          1, PATCH_MAGICDWORD(0x83, 0x00, 0x31, 0x1e),     0, kq6SignatureDuplicateBabyCry, kq6PatchDuplicateBabyCry },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Script 210 in the German version of Longbow handles the case where Robin
// hands out the scroll to Marion and then types his name using the hand code.
// The German version script contains a typo (probably a copy/paste error),
// and the function that is used to show each letter is called twice. The
// second time that the function is called, the second parameter passed to
// the function is undefined, thus kStrCat() that is called inside the function
// reads a random pointer and crashes. We patch all of the 5 function calls
// (one for each letter typed from "R", "O", "B", "I", "N") so that they are
// the same as the English version. Fixes bug #3048054.
const byte longbowSignatureShowHandCode[] = {
	3,
	0x78,                   // push1
	0x78,                   // push1
	0x72,                   // lofsa
	+2, 2,                  // skip 2 bytes, offset of lofsa (the letter typed)
	0x36,                   // push
	0x40,                   // call
	+2, 3,                  // skip 2 bytes, offset of call
	0x02,                   // perform the call above with 2 parameters
	0x36,                   // push
	0x40,                   // call
	+2, 8,                  // skip 2 bytes, offset of call
	0x02,                   // perform the call above with 2 parameters
	0x38, 0x1c, 0x01,       // pushi 011c (setMotion)
	0x39, 0x04,             // pushi 04 (x)
	0x51, 0x1e,             // class MoveTo
	0
};

const uint16 longbowPatchShowHandCode[] = {
	0x39, 0x01,             // pushi 1 (combine the two push1's in one, like in the English version)
	PATCH_ADDTOOFFSET | +3, // leave the lofsa call untouched
	// The following will remove the duplicate call
	0x32, 0x02, 0x00,       // jmp 02 - skip 2 bytes (the remainder of the first call)
	0x48,                   // ret (dummy, should never be reached)
	0x48,                   // ret (dummy, should never be reached)
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature longbowSignatures[] = {
	{    210, "hand code crash",                             5, PATCH_MAGICDWORD(0x02, 0x38, 0x1c, 0x01),   -14, longbowSignatureShowHandCode, longbowPatchShowHandCode },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// this is called on every death dialog. Problem is at least the german
//  version of lsl6 gets title text that is far too long for the
//  available temp space resulting in temp space corruption
//  This patch moves the title text around, so this overflow
//  doesn't happen anymore. We would otherwise get a crash
//  calling for invalid views (this happens of course also
//  in sierra sci)
const byte larry6SignatureDeathDialog[] = {
	7,
	0x3e, 0x33, 0x01,             // link 0133 (offset 0x20)
	0x35, 0xff,                   // ldi ff
	0xa3, 0x00,                   // sal 00
	+255, 0,
	+255, 0,
	+170, 12,                     // [skip 680 bytes]
	0x8f, 0x01,                   // lsp 01 (offset 0x2cf)
	0x7a,                         // push2
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,                         // push
	0x43, 0x7c, 0x0e,             // kMessage[7c] 0e
	+90, 10,                      // [skip 90 bytes]
	0x38, 0xd6, 0x00,             // pushi 00d6 (offset 0x335)
	0x78,                         // push1
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,                         // push
	+76, 11,                      // [skip 76 bytes]
	0x38, 0xcd, 0x00,             // pushi 00cd (offset 0x38b)
	0x39, 0x03,                   // pushi 03
	0x5a, 0x04, 0x00, 0x0e, 0x01, // lea 0004 010e
	0x36,
	0
};

const uint16 larry6PatchDeathDialog[] = {
	0x3e, 0x00, 0x02,             // link 0200
	PATCH_ADDTOOFFSET | +687,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_ADDTOOFFSET | +98,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_ADDTOOFFSET | +82,
	0x5a, 0x04, 0x00, 0x40, 0x01, // lea 0004 0140
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature larry6Signatures[] = {
	{     82, "death dialog memory corruption",              1, PATCH_MAGICDWORD(0x3e, 0x33, 0x01, 0x35),     0, larry6SignatureDeathDialog, larry6PatchDeathDialog },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// rm560::doit was supposed to close the painting, when Heimlich enters the
//  room. The code is buggy, so it actually closes the painting, when heimlich
//  is not in the room. We fix that.
const byte laurabow2SignaturePaintingClosing[] = {
	17,
	0x4a, 0x04,       // send 04 - read aHeimlich::room
	0x36,             // push
	0x81, 0x0b,       // lag global[11d] -> current room
	0x1c,             // ne?
	0x31, 0x0e,       // bnt [don't close]
	0x35, 0x00,       // ldi 00
	0xa3, 0x00,       // sal local[0]
	0x38, 0x92, 0x00, // pushi 0092
	0x78,             // push1
	0x72,             // lofsa sDumpSafe
	0
};

const uint16 laurabow2PatchPaintingClosing[] = {
	PATCH_ADDTOOFFSET | +6,
	0x2f, 0x0e,       // bt [don't close]
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature laurabow2Signatures[] = {
	{    560, "painting closing immediately",                1, PATCH_MAGICDWORD(0x36, 0x81, 0x0b, 0x1c),    -2, laurabow2SignaturePaintingClosing, laurabow2PatchPaintingClosing },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Mother Goose SCI1/SCI1.1
// MG::replay somewhat calculates the savedgame-id used when saving again
//  this doesn't work right and we remove the code completely.
//  We set the savedgame-id directly right after restoring in kRestoreGame.
const byte mothergoose256SignatureReplay[] = {
	6,
	0x36,             // push
	0x35, 0x20,       // ldi 20
	0x04,             // sub
	0xa1, 0xb3,       // sag global[b3]
	0
};

const uint16 mothergoose256PatchReplay[] = {
	0x34, 0x00, 0x00, // ldi 0000 (dummy)
	0x34, 0x00, 0x00, // ldi 0000 (dummy)
	PATCH_END
};

// when saving, it also checks if the savegame ID is below 13.
//  we change this to check if below 113 instead
const byte mothergoose256SignatureSaveLimit[] = {
	5,
	0x89, 0xb3,       // lsg global[b3]
	0x35, 0x0d,       // ldi 0d
	0x20,             // ge?
	0
};

const uint16 mothergoose256PatchSaveLimit[] = {
	PATCH_ADDTOOFFSET | +2,
	0x35, 0x0d + SAVEGAMEID_OFFICIALRANGE_START, // ldi 113d
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature mothergoose256Signatures[] = {
	{      0, "replay save issue",                           1, PATCH_MAGICDWORD(0x20, 0x04, 0xa1, 0xb3),    -2, mothergoose256SignatureReplay,    mothergoose256PatchReplay },
	{      0, "save limit dialog (SCI1.1)",                  1, PATCH_MAGICDWORD(0xb3, 0x35, 0x0d, 0x20),    -1, mothergoose256SignatureSaveLimit, mothergoose256PatchSaveLimit },
	{    994, "save limit dialog (SCI1)",                    1, PATCH_MAGICDWORD(0xb3, 0x35, 0x0d, 0x20),    -1, mothergoose256SignatureSaveLimit, mothergoose256PatchSaveLimit },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
//  script 215 of qfg1vga pointBox::doit actually processes button-presses
//   during fighting with monsters. It strangely also calls kGetEvent. Because
//   the main User::doit also calls kGetEvent it's pure luck, where the event
//   will hit. It's the same issue as in freddy pharkas and if you turn dos-box
//   to max cycles, sometimes clicks also won't get registered. Strangely it's
//   not nearly as bad as in our sci, but these differences may be caused by
//   timing.
//   We just reuse the active event, thus removing the duplicate kGetEvent call.
const byte qfg1vgaSignatureFightEvents[] = {
	25,
	0x39, 0x6d,       // pushi 6d (selector new)
	0x76,             // push0
	0x51, 0x07,       // class Event
	0x4a, 0x04,       // send 04 - call Event::new
	0xa5, 0x00,       // sat temp[0]
	0x78,             // push1
	0x76,             // push0
	0x4a, 0x04,       // send 04 - read Event::x
	0xa5, 0x03,       // sat temp[3]
	0x76,             // push0 (selector y)
	0x76,             // push0
	0x85, 0x00,       // lat temp[0]
	0x4a, 0x04,       // send 04 - read Event::y
	0x36,             // push
	0x35, 0x0a,       // ldi 0a
	0x04,             // sub (poor mans localization) ;-)
	0
};

const uint16 qfg1vgaPatchFightEvents[] = {
	0x38, 0x5a, 0x01, // pushi 15a (selector curEvent)
	0x76,             // push0
	0x81, 0x50,       // lag global[50]
	0x4a, 0x04,       // send 04 - read User::curEvent -> needs one byte more than previous code
	0xa5, 0x00,       // sat temp[0]
	0x78,             // push1
	0x76,             // push0
	0x4a, 0x04,       // send 04 - read Event::x
	0xa5, 0x03,       // sat temp[3]
	0x76,             // push0 (selector y)
	0x76,             // push0
	0x85, 0x00,       // lat temp[0]
	0x4a, 0x04,       // send 04 - read Event::y
	0x39, 0x00,       // pushi 00
	0x02,             // add (waste 3 bytes) - we don't need localization, User::doit has already done it
	PATCH_END
};

// Script 814 of QFG1VGA is responsible for showing dialogs. However, the death
// screen message shown when the hero dies in room 64 (ghost room) is too large
// (254 chars long). Since the window header and main text are both stored in
// temp space, this is an issue, as the scripts read the window header, then the
// window text, which erases the window header text because of its length. To
// fix that, we allocate more temp space and move the pointer used for the
// window header a little bit, wherever it's used in script 814.
// Fixes bug #3568431.

// Patch 1: Increase temp space
const byte qfg1vgaSignatureTempSpace[] = {
	4,
	0x3f, 0xba,       // link 0xba
	0x87, 0x00,       // lap 0
	0
};

const uint16 qfg1vgaPatchTempSpace[] = {
	0x3f, 0xca,       // link 0xca
	PATCH_END
};

// Patch 2: Move the pointer used for the window header a little bit
const byte qfg1vgaSignatureDialogHeader[] = {
	4,
	0x5b, 0x04, 0x80,  // lea temp[0x80]
	0x36,              // push
	0
};

const uint16 qfg1vgaPatchDialogHeader[] = {
	0x5b, 0x04, 0x90,  // lea temp[0x90]
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature qfg1vgaSignatures[] = {
	{    215, "fight event issue",                           1, PATCH_MAGICDWORD(0x6d, 0x76, 0x51, 0x07),    -1, qfg1vgaSignatureFightEvents,       qfg1vgaPatchFightEvents },
	{    216, "weapon master event issue",                   1, PATCH_MAGICDWORD(0x6d, 0x76, 0x51, 0x07),    -1, qfg1vgaSignatureFightEvents,       qfg1vgaPatchFightEvents },
	{    814, "window text temp space",                      1, PATCH_MAGICDWORD(0x3f, 0xba, 0x87, 0x00),     0, qfg1vgaSignatureTempSpace,         qfg1vgaPatchTempSpace },
	{    814, "dialog header offset",                        3, PATCH_MAGICDWORD(0x5b, 0x04, 0x80, 0x36),     0, qfg1vgaSignatureDialogHeader,      qfg1vgaPatchDialogHeader },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Script 944 in QFG2 contains the FileSelector system class, used in the
// character import screen. This gets incorrectly called constantly, whenever
// the user clicks on a button in order to refresh the file list. This was
// probably done because it would be easier to refresh the list whenever the
// user inserted a new floppy disk, or changed directory. The problem is that
// the script has a bug, and it invalidates the text of the entries in the
// list. This has a high probability of breaking, as the user could change the
// list very quickly, or the garbage collector could kick in and remove the
// deleted entries. We don't allow the user to change the directory, thus the
// contents of the file list are constant, so we can avoid the constant file
// and text entry refreshes whenever a button is pressed, and prevent possible
// crashes because of these constant quick object reallocations. Fixes bug
// #3037996.
const byte qfg2SignatureImportDialog[] = {
	16,
	0x63, 0x20,       // pToa text
	0x30, 0x0b, 0x00, // bnt [next state]
	0x7a,             // push2
	0x39, 0x03,       // pushi 03
	0x36,             // push
	0x43, 0x72, 0x04, // callk Memory 4
	0x35, 0x00,       // ldi 00
	0x65, 0x20,       // aTop text
	0
};

const uint16 qfg2PatchImportDialog[] = {
	PATCH_ADDTOOFFSET | +5,
	0x48,             // ret
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature qfg2Signatures[] = {
	{    944, "import dialog continuous calls",                 1, PATCH_MAGICDWORD(0x20, 0x30, 0x0b, 0x00),  -1, qfg2SignatureImportDialog, qfg2PatchImportDialog },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
// Patch for the import screen in QFG3, same as the one for QFG2 above
const byte qfg3SignatureImportDialog[] = {
	15,
	0x63, 0x2a,       // pToa text
	0x31, 0x0b,       // bnt [next state]
	0x7a,             // push2
	0x39, 0x03,       // pushi 03
	0x36,             // push
	0x43, 0x72, 0x04, // callk Memory 4
	0x35, 0x00,       // ldi 00
	0x65, 0x2a,       // aTop text
	0
};

const uint16 qfg3PatchImportDialog[] = {
	PATCH_ADDTOOFFSET | +4,
	0x48,             // ret
	PATCH_END
};



// ===========================================================================
// Patch for the Woo dialog option in Uhura's conversation. Bug #3040722
// Problem: The Woo dialog option (0xffb5) is negative, and therefore
// treated as an option opening a submenu. This leads to uhuraTell::doChild
// being called, which calls hero::solvePuzzle and then proceeds with
// Teller::doChild to open the submenu. However, there is no actual submenu
// defined for option -75 since -75 does not show up in uhuraTell::keys.
// This will cause Teller::doChild to run out of bounds while scanning through
// uhuraTell::keys.
// Strategy: there is another conversation option in uhuraTell::doChild calling
// hero::solvePuzzle (0xfffc) which does a ret afterwards without going to
// Teller::doChild. We jump to this call of hero::solvePuzzle to get that same
// behaviour.

const byte qfg3SignatureWooDialog[] = {
	30,
	0x67, 0x12,       // pTos 12 (query)
	0x35, 0xb6,       // ldi b6
	0x1a,             // eq?
	0x2f, 0x05,       // bt 05
	0x67, 0x12,       // pTos 12 (query)
	0x35, 0x9b,       // ldi 9b
	0x1a,             // eq?
	0x31, 0x0c,       // bnt 0c
	0x38, 0x97, 0x02, // pushi 0297
	0x7a,             // push2
	0x38, 0x0c, 0x01, // pushi 010c
	0x7a,             // push2
	0x81, 0x00,       // lag 00
	0x4a, 0x08,       // send 08
	0x67, 0x12,       // pTos 12 (query)
	0x35, 0xb5,       // ldi b5
	0
};

const uint16 qfg3PatchWooDialog[] = {
	PATCH_ADDTOOFFSET | +0x29,
	0x33, 0x11, // jmp to 0x6a2, the call to hero::solvePuzzle for 0xFFFC
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature qfg3Signatures[] = {
	{    944, "import dialog continuous calls",                 1, PATCH_MAGICDWORD(0x2a, 0x31, 0x0b, 0x7a),  -1, qfg3SignatureImportDialog,         qfg3PatchImportDialog },
	{    440, "dialog crash when asking about Woo",             1, PATCH_MAGICDWORD(0x67, 0x12, 0x35, 0xb5),  -26, qfg3SignatureWooDialog,         qfg3PatchWooDialog },
	SCI_SIGNATUREENTRY_TERMINATOR
};

// ===========================================================================
//  script 298 of sq4/floppy has an issue. object "nest" uses another property
//   which isn't included in property count. We return 0 in that case, the game
//   adds it to nest::x. The problem is that the script also checks if x exceeds
//   we never reach that of course, so the pterodactyl-flight will go endlessly
//   we could either calculate property count differently somehow fixing this
//   but I think just patching it out is cleaner (ffs. bug #3037938)
const byte sq4FloppySignatureEndlessFlight[] = {
	8,
	0x39, 0x04,       // pushi 04 (selector x)
	0x78,             // push1
	0x67, 0x08,       // pTos 08 (property x)
	0x63, 0x44,       // pToa 44 (invalid property)
	0x02,             // add
	0
};

// Similar to the above, for the German version (ffs. bug #3110215)
const byte sq4FloppySignatureEndlessFlightGerman[] = {
	8,
	0x39, 0x04,       // pushi 04 (selector x)
	0x78,             // push1
	0x67, 0x08,       // pTos 08 (property x)
	0x63, 0x4c,       // pToa 4c (invalid property)
	0x02,             // add
	0
};

const uint16 sq4FloppyPatchEndlessFlight[] = {
	PATCH_ADDTOOFFSET | +5,
	0x35, 0x03,       // ldi 03 (which would be the content of the property)
	PATCH_END
};

// The scripts in SQ4CD support simultaneous playing of speech and subtitles,
// but this was not available as an option. The following two patches enable
// this functionality in the game's GUI options dialog.
// Patch 1: iconTextSwitch::show, called when the text options button is shown.
// This is patched to add the "Both" text resource (i.e. we end up with
// "Speech", "Text" and "Both")
const byte sq4CdSignatureTextOptionsButton[] = {
	11,
	0x35, 0x01,      // ldi 0x01
	0xa1, 0x53,      // sag 0x53
	0x39, 0x03,      // pushi 0x03
	0x78,            // push1
	0x39, 0x09,      // pushi 0x09
	0x54, 0x06,      // self 0x06
	0
};

const uint16 sq4CdPatchTextOptionsButton[] = {
	PATCH_ADDTOOFFSET | +7,
	0x39, 0x0b,      // pushi 0x0b
	PATCH_END
};

// Patch 2: Adjust a check in babbleIcon::init, which handles the babble icon
// (e.g. the two guys from Andromeda) shown when dying/quitting.
// Fixes bug #3538418.
const byte sq4CdSignatureBabbleIcon[] = {
	7,
	0x89, 0x5a,      // lsg 5a
	0x35, 0x02,      // ldi 02
	0x1a,            // eq?
	0x31, 0x26,      // bnt 26  [02a7]
	0
};

const uint16 sq4CdPatchBabbleIcon[] = {
	0x89, 0x5a,      // lsg 5a
	0x35, 0x01,      // ldi 01
	0x1a,            // eq?
	0x2f, 0x26,      // bt 26  [02a7]
	PATCH_END
};

// Patch 3: Add the ability to toggle among the three available options,
// when the text options button is clicked: "Speech", "Text" and "Both".
// Refer to the patch above for additional details.
// iconTextSwitch::doit (called when the text options button is clicked)
const byte sq4CdSignatureTextOptions[] = {
	32,
	0x89, 0x5a,       // lsg 0x5a (load global 90 to stack)
	0x3c,             // dup
	0x35, 0x01,       // ldi 0x01
	0x1a,             // eq? (global 90 == 1)
	0x31, 0x06,       // bnt 0x06 (0x0691)
	0x35, 0x02,       // ldi 0x02
	0xa1, 0x5a,       // sag 0x5a (save acc to global 90)
	0x33, 0x0a,       // jmp 0x0a (0x69b)
	0x3c,             // dup
	0x35, 0x02,       // ldi 0x02
	0x1a,             // eq? (global 90 == 2)
	0x31, 0x04,       // bnt 0x04 (0x069b)
	0x35, 0x01,       // ldi 0x01
	0xa1, 0x5a,       // sag 0x5a (save acc to global 90)
	0x3a,             // toss
	0x38, 0xd9, 0x00, // pushi 0x00d9
	0x76,             // push0
	0x54, 0x04,       // self 0x04
	0x48,             // ret
	0
};

const uint16 sq4CdPatchTextOptions[] = {
	0x89, 0x5a,       // lsg 0x5a (load global 90 to stack)
	0x3c,             // dup
	0x35, 0x03,       // ldi 0x03 (acc = 3)
	0x1a,             // eq? (global 90 == 3)
	0x2f, 0x07,       // bt 0x07
	0x89, 0x5a,       // lsg 0x5a (load global 90 to stack again)
	0x35, 0x01,       // ldi 0x01 (acc = 1)
	0x02,             // add: acc = global 90 (on stack) + 1 (previous acc value)
	0x33, 0x02,       // jmp 0x02
	0x35, 0x01,       // ldi 0x01 (reset acc to 1)
	0xa1, 0x5a,       // sag 0x5a (save acc to global 90)
	0x33, 0x03,       // jmp 0x03 (jump over the wasted bytes below)
	0x34, 0x00, 0x00, // ldi 0x0000 (waste 3 bytes)
	0x3a,             // toss
	// (the rest of the code is the same)
	PATCH_END
};

//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature sq4Signatures[] = {
	{    298, "Floppy: endless flight",                      1, PATCH_MAGICDWORD(0x67, 0x08, 0x63, 0x44),    -3,       sq4FloppySignatureEndlessFlight, sq4FloppyPatchEndlessFlight },
	{    298, "Floppy (German): endless flight",             1, PATCH_MAGICDWORD(0x67, 0x08, 0x63, 0x4c),    -3, sq4FloppySignatureEndlessFlightGerman, sq4FloppyPatchEndlessFlight },
	{    818, "CD: Speech and subtitles option",             1, PATCH_MAGICDWORD(0x89, 0x5a, 0x3c, 0x35),     0,             sq4CdSignatureTextOptions,       sq4CdPatchTextOptions },
	{      0, "CD: Babble icon speech and subtitles fix",    1, PATCH_MAGICDWORD(0x89, 0x5a, 0x35, 0x02),     0,              sq4CdSignatureBabbleIcon,        sq4CdPatchBabbleIcon },
	{    818, "CD: Speech and subtitles option button",      1, PATCH_MAGICDWORD(0x35, 0x01, 0xa1, 0x53),     0,       sq4CdSignatureTextOptionsButton, sq4CdPatchTextOptionsButton },
	SCI_SIGNATUREENTRY_TERMINATOR
};

const byte sq1vgaSignatureEgoShowsCard[] = {
	25,
	0x38, 0x46, 0x02, // push 0x246 (set up send frame to set timesShownID)
	0x78,             // push1
	0x38, 0x46, 0x02, // push 0x246 (set up send frame to get timesShownID)
	0x76,             // push0
	0x51, 0x7c,       // class DeltaurRegion
	0x4a, 0x04,       // send 0x04 (get timesShownID)
	0x36,             // push
	0x35, 0x01,       // ldi 1
	0x02,             // add
	0x36,             // push
	0x51, 0x7c,       // class DeltaurRegion
	0x4a, 0x06,       // send 0x06 (set timesShownID)
	0x36,             // push      (wrong, acc clobbered by class, above)
	0x35, 0x03,       // ldi 0x03
	0x22,             // lt?
	0};

// Note that this script patch is merely a reordering of the
// instructions in the original script.
const uint16 sq1vgaPatchEgoShowsCard[] = {
	0x38, 0x46, 0x02, // push 0x246 (set up send frame to get timesShownID)
	0x76,             // push0
	0x51, 0x7c,       // class DeltaurRegion
	0x4a, 0x04,       // send 0x04 (get timesShownID)
	0x36,             // push
	0x35, 0x01,       // ldi 1
	0x02,             // add
	0x36,             // push (this push corresponds to the wrong one above)
	0x38, 0x46, 0x02, // push 0x246 (set up send frame to set timesShownID)
	0x78,             // push1
	0x36,             // push
	0x51, 0x7c,       // class DeltaurRegion
	0x4a, 0x06,       // send 0x06 (set timesShownID)
	0x35, 0x03,       // ldi 0x03
	0x22,             // lt?
	PATCH_END};


//    script, description,                                      magic DWORD,                                  adjust
const SciScriptSignature sq1vgaSignatures[] = {
	{   58, "Sarien armory droid zapping ego first time", 1, PATCH_MAGICDWORD( 0x72, 0x88, 0x15, 0x36 ), -70,
		sq1vgaSignatureEgoShowsCard, sq1vgaPatchEgoShowsCard },

	SCI_SIGNATUREENTRY_TERMINATOR};

// will actually patch previously found signature area
void Script::applyPatch(const uint16 *patch, byte *scriptData, const uint32 scriptSize, int32 signatureOffset) {
	byte orgData[PATCH_VALUELIMIT];
	int32 offset = signatureOffset;
	uint16 patchWord = *patch;

	// Copy over original bytes from script
	uint32 orgDataSize = scriptSize - offset;
	if (orgDataSize > PATCH_VALUELIMIT)
		orgDataSize = PATCH_VALUELIMIT;
	memcpy(&orgData, &scriptData[offset], orgDataSize);

	while (patchWord != PATCH_END) {
		uint16 patchValue = patchWord & PATCH_VALUEMASK;
		switch (patchWord & PATCH_COMMANDMASK) {
		case PATCH_ADDTOOFFSET:
			// add value to offset
			offset += patchValue & ~PATCH_ADDTOOFFSET;
			break;
		case PATCH_GETORIGINALBYTE:
			// get original byte from script
			if (patchValue >= orgDataSize)
				error("patching: can not get requested original byte from script");
			scriptData[offset] = orgData[patchValue];
			offset++;
			break;
		case PATCH_ADJUSTWORD: {
			// Adjust word right before current position
			byte *adjustPtr = &scriptData[offset - 2];
			uint16 adjustWord = READ_LE_UINT16(adjustPtr);
			adjustWord += patchValue;
			WRITE_LE_UINT16(adjustPtr, adjustWord);
			break;
		}
		case PATCH_ADJUSTWORD_NEG: {
			// Adjust word right before current position (negative way)
			byte *adjustPtr = &scriptData[offset - 2];
			uint16 adjustWord = READ_LE_UINT16(adjustPtr);
			adjustWord -= patchValue;
			WRITE_LE_UINT16(adjustPtr, adjustWord);
			break;
		}
		default:
			scriptData[offset] = patchValue & 0xFF;
			offset++;
		}
		patch++;
		patchWord = *patch;
	}
}

// will return -1 if no match was found, otherwise an offset to the start of the signature match
int32 Script::findSignature(const SciScriptSignature *signature, const byte *scriptData, const uint32 scriptSize) {
	if (scriptSize < 4) // we need to find a DWORD, so less than 4 bytes is not okay
		return -1;

	const uint32 magicDWord = signature->magicDWord; // is platform-specific BE/LE form, so that the later match will work
	const uint32 searchLimit = scriptSize - 3;
	uint32 DWordOffset = 0;
	// first search for the magic DWORD
	while (DWordOffset < searchLimit) {
		if (magicDWord == READ_UINT32(scriptData + DWordOffset)) {
			// magic DWORD found, check if actual signature matches
			uint32 offset = DWordOffset + signature->magicOffset;
			uint32 byteOffset = offset;
			const byte *signatureData = signature->data;
			byte matchAdjust = 1;
			while (matchAdjust) {
				byte matchBytesCount = *signatureData++;
				if ((byteOffset + matchBytesCount) > scriptSize) // Out-Of-Bounds?
					break;
				if (memcmp(signatureData, &scriptData[byteOffset], matchBytesCount)) // Byte-Mismatch?
					break;
				// those bytes matched, adjust offsets accordingly
				signatureData += matchBytesCount;
				byteOffset += matchBytesCount;
				// get offset...
				matchAdjust = *signatureData++;
				byteOffset += matchAdjust;
			}
			if (!matchAdjust) // all matches worked?
				return offset;
		}
		DWordOffset++;
	}
	// nothing found
	return -1;
}

void Script::matchSignatureAndPatch(uint16 scriptNr, byte *scriptData, const uint32 scriptSize) {
	const SciScriptSignature *signatureTable = NULL;
	switch (g_sci->getGameId()) {
	case GID_ECOQUEST:
		signatureTable = ecoquest1Signatures;
		break;
	case GID_ECOQUEST2:
		signatureTable = ecoquest2Signatures;
		break;
	case GID_FANMADE:
		signatureTable = fanmadeSignatures;
		break;
	case GID_FREDDYPHARKAS:
		signatureTable = freddypharkasSignatures;
		break;
	case GID_GK1:
		signatureTable = gk1Signatures;
		break;
	case GID_KQ5:
		signatureTable = kq5Signatures;
		break;
	case GID_KQ6:
		signatureTable = kq6Signatures;
		break;
	case GID_LAURABOW2:
		signatureTable = laurabow2Signatures;
		break;
	case GID_LONGBOW:
		signatureTable = longbowSignatures;
		break;
	case GID_LSL6:
		signatureTable = larry6Signatures;
		break;
	case GID_MOTHERGOOSE256:
		signatureTable = mothergoose256Signatures;
		break;
	case GID_QFG1VGA:
		signatureTable = qfg1vgaSignatures;
		break;
	case GID_QFG2:
		signatureTable = qfg2Signatures;
		break;
	case GID_QFG3:
		signatureTable = qfg3Signatures;
		break;
	case GID_SQ1:
		signatureTable = sq1vgaSignatures;
		break;
	case GID_SQ4:
		signatureTable = sq4Signatures;
		break;
	default:
		break;
	}

	if (signatureTable) {
		while (signatureTable->data) {
			if (scriptNr == signatureTable->scriptNr) {
				int32 foundOffset = 0;
				int16 applyCount = signatureTable->applyCount;
				do {
					foundOffset = findSignature(signatureTable, scriptData, scriptSize);
					if (foundOffset != -1) {
						// found, so apply the patch
						debugC(kDebugLevelScripts, "matched and patched %s on script %d offset %d", signatureTable->description, scriptNr, foundOffset);
						applyPatch(signatureTable->patch, scriptData, scriptSize, foundOffset);
					}
					applyCount--;
				} while ((foundOffset != -1) && (applyCount));
			}
			signatureTable++;
		}
	}
}

} // End of namespace Sci
