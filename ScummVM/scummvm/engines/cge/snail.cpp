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

/*
 * This code is based on original Soltys source code
 * Copyright (c) 1994-1995 Janus B. Wisniewski and L.K. Avalon
 */

#include "cge/general.h"
#include "cge/sound.h"
#include "cge/snail.h"
#include "cge/vga13h.h"
#include "cge/text.h"
#include "cge/cge_main.h"
#include "cge/events.h"
#include "cge/walk.h"

namespace CGE {

const char *CommandHandler::_commandText[] = {
	"LABEL",  "PAUSE",  "WAIT",    "LEVEL",   "HIDE",
	"SAY",    "INF",    "TIME",    "CAVE",    "KILL",
	"RSEQ",   "SEQ",    "SEND",    "SWAP",    "KEEP",
	"GIVE",   "IF",     "GAME",    "SETX0",   "SETY0",
	"SLAVE",  "SETXY",  "RELX",    "RELY",    "RELZ",
	"SETX",   "SETY",   "SETZ",    "TRANS",   "PORT",
	"NEXT",   "NNEXT",  "TNEXT",   "RNNEXT",  "RTNEXT",
	"RMNEAR", "RMTAKE", "FLAG",    "SETREF",  "BACKPT",
	"FLASH",  "LIGHT",  "SETHB",   "SETVB",   "WALK",
	"REACH",  "COVER",  "UNCOVER", "CLEAR",   "TALK",
	"MOUSE",  "SOUND",  "COUNT",   NULL
};

CommandHandler::CommandHandler(CGEEngine *vm, bool turbo)
	: _turbo(turbo), _busy(false), _textDelay(false),
	  _timerExpiry(0), _talkEnable(true),
	  _head(0), _tail(0), _commandList((Command *)malloc(sizeof(Command) * 256)), _vm(vm) {
}

CommandHandler::~CommandHandler() {
	free(_commandList);
}

/**
 * Add a Command on the head of _commandList
 * @param com			Command
 * @param ref			Reference
 * @param val			Value
 * @param ptr			Sprite pointer
 */
void CommandHandler::addCommand(CommandType com, int ref, int val, void *ptr) {
	Command *headCmd = &_commandList[_head++];
	headCmd->_commandType = com;
	headCmd->_ref = ref;
	headCmd->_val = val;
	headCmd->_spritePtr = ptr;
	headCmd->_cbType = kNullCB;
	if (headCmd->_commandType == kCmdClear) {
		_tail = _head;
		_vm->killText();
		_timerExpiry = 0;
	}
}

/**
 * Add a Callback on the head of _commandList
 * @param com			Command
 * @param ref			Reference
 * @param val			Value
 * @param CallbackType	Callback type
 */
void CommandHandler::addCallback(CommandType com, int ref, int val, CallbackType cbType) {
	Command *headCmd = &_commandList[_head++];
	headCmd->_commandType = com;
	headCmd->_ref = ref;
	headCmd->_val = val;
	headCmd->_spritePtr = NULL;
	headCmd->_cbType = cbType;
	if (headCmd->_commandType == kCmdClear) {
		_tail = _head;
		_vm->killText();
		_timerExpiry = 0;
	}
}

/**
 * Add a Command on the tail of _commandList
 * @param com			Command
 * @param ref			Reference
 * @param val			Value
 * @param ptr			Sprite pointer
 */
void CommandHandler::insertCommand(CommandType com, int ref, int val, void *ptr) {
	Command *tailCmd;

	if (_busy) {
		_commandList[(_tail - 1) & 0xFF] = _commandList[_tail];
		tailCmd = &_commandList[_tail];
	} else
		tailCmd = &_commandList[(_tail - 1) & 0xFF];
	_tail--;
	tailCmd->_commandType = com;
	tailCmd->_ref = ref;
	tailCmd->_val = val;
	tailCmd->_spritePtr = ptr;
	tailCmd->_cbType = kNullCB;
	if (tailCmd->_commandType == kCmdClear) {
		_tail = _head;
		_vm->killText();
		_timerExpiry = 0;
	}
}

void CommandHandler::runCommand() {
	if (_busy)
		return;

	_busy = true;
	uint8 tmpHead = _head;
	while (_tail != tmpHead) {
		Command *tailCmd = &_commandList[_tail];

		if (!_turbo) { // only for the slower one
			if (_timerExpiry) {
				// Delay in progress
				if (_timerExpiry > g_system->getMillis())
					// Delay not yet ended
					break;

				// Delay is finished
				_timerExpiry = 0;
			} else {
				if (_textDelay) {
					_vm->killText();
					_textDelay = false;
				}
			}
			if (_vm->_talk && tailCmd->_commandType != kCmdPause)
				break;
		}

		Sprite *spr = ((tailCmd->_ref >= 0) ? _vm->locate(tailCmd->_ref) : ((Sprite *) tailCmd->_spritePtr));
		switch (tailCmd->_commandType) {
		case kCmdLabel:
			break;
		case kCmdPause:
			_timerExpiry = g_system->getMillis() + tailCmd->_val * kCommandFrameDelay;
			if (_vm->_talk)
				_textDelay = true;
			break;
		case kCmdWait:
			if (spr) {
				if (spr->seqTest(tailCmd->_val) &&
					(tailCmd->_val >= 0 || spr != _vm->_hero || _vm->_hero->_tracePtr < 0)) {
					_timerExpiry = g_system->getMillis() + spr->_time * kCommandFrameDelay;
				} else {
					_busy = false;
					return;
				}
			}
			break;
		case kCmdLevel:
			_vm->snLevel(spr, tailCmd->_val);
			break;
		case kCmdHide:
			_vm->snHide(spr, tailCmd->_val);
			break;
		case kCmdSay:
			if (spr && _talkEnable) {
				if (spr == _vm->_hero && spr->seqTest(-1))
					spr->step(kSeqHTalk);
				_vm->_text->say(_vm->_text->getText(tailCmd->_val), spr);
				_vm->_sys->_funDel = kHeroFun0;
			}
			break;
		case kCmdInf:
			if (_talkEnable) {
				_vm->inf(_vm->_text->getText(tailCmd->_val), true);
				_vm->_sys->_funDel = kHeroFun0;
			}
			break;
		case kCmdTime:
			if (spr && _talkEnable) {
				if (spr == _vm->_hero && spr->seqTest(-1))
					spr->step(kSeqHTalk);
				_vm->_text->sayTime(spr);
			}
			break;
		case kCmdCave:
			_vm->switchScene(tailCmd->_val);
			break;
		case kCmdKill:
			_vm->snKill(spr);
			break;
		case kCmdSeq:
			_vm->snSeq(spr, tailCmd->_val);
			break;
		case kCmdRSeq:
			_vm->snRSeq(spr, tailCmd->_val);
			break;
		case kCmdSend:
			_vm->snSend(spr, tailCmd->_val);
			break;
		case kCmdSwap:
			_vm->snSwap(spr, tailCmd->_val);
			break;
		case kCmdCover:
			_vm->snCover(spr, tailCmd->_val);
			break;
		case kCmdUncover:
			_vm->snUncover(spr, (tailCmd->_val >= 0) ? _vm->locate(tailCmd->_val) : ((Sprite *) tailCmd->_spritePtr));
			break;
		case kCmdKeep:
			_vm->snKeep(spr, tailCmd->_val);
			break;
		case kCmdGive:
			_vm->snGive(spr, tailCmd->_val);
			break;
		case kCmdGame:
			_vm->snGame(spr, tailCmd->_val);
			break;
		case kCmdSetX0:
			_vm->snSetX0(tailCmd->_ref, tailCmd->_val);
			break;
		case kCmdSetY0:
			_vm->snSetY0(tailCmd->_ref, tailCmd->_val);
			break;
		case kCmdSetXY:
			_vm->snSetXY(spr, tailCmd->_val);
			break;
		case kCmdRelX:
			_vm->snRelX(spr, tailCmd->_val);
			break;
		case kCmdRelY:
			_vm->snRelY(spr, tailCmd->_val);
			break;
		case kCmdRelZ:
			_vm->snRelZ(spr, tailCmd->_val);
			break;
		case kCmdSetX:
			_vm->snSetX(spr, tailCmd->_val);
			break;
		case kCmdSetY:
			_vm->snSetY(spr, tailCmd->_val);
			break;
		case kCmdSetZ:
			_vm->snSetZ(spr, tailCmd->_val);
			break;
		case kCmdSlave:
			_vm->snSlave(spr, tailCmd->_val);
			break;
		case kCmdTrans:
			_vm->snTrans(spr, tailCmd->_val);
			break;
		case kCmdPort:
			_vm->snPort(spr, tailCmd->_val);
			break;
		case kCmdNext:
		case kCmdIf:
		case kCmdTalk:
			break;
		case kCmdMouse:
			_vm->snMouse(tailCmd->_val != 0);
			break;
		case kCmdNNext:
			_vm->snNNext(spr, tailCmd->_val);
			break;
		case kCmdTNext:
			_vm->snTNext(spr, tailCmd->_val);
			break;
		case kCmdRNNext:
			_vm->snRNNext(spr, tailCmd->_val);
			break;
		case kCmdRTNext:
			_vm->snRTNext(spr, tailCmd->_val);
			break;
		case kCmdRMNear:
			_vm->snRmNear(spr);
			break;
		case kCmdRmTake:
			_vm->snRmTake(spr);
			break;
		case kCmdFlag:
			_vm->snFlag(tailCmd->_ref & 3, tailCmd->_val != 0);
			break;
		case kCmdSetRef:
			_vm->snSetRef(spr, tailCmd->_val);
			break;
		case kCmdBackPt:
			_vm->snBackPt(spr, tailCmd->_val);
			break;
		case kCmdFlash:
			_vm->snFlash(tailCmd->_val != 0);
			break;
		case kCmdLight:
			_vm->snLight(tailCmd->_val != 0);
			break;
		case kCmdSetHBarrier:
			_vm->snHBarrier(tailCmd->_ref, tailCmd->_val);
			break;
		case kCmdSetVBarrier:
			_vm->snVBarrier(tailCmd->_ref, tailCmd->_val);
			break;
		case kCmdWalk:
			_vm->snWalk(spr, tailCmd->_ref, tailCmd->_val);
			break;
		case kCmdReach:
			_vm->snReach(spr, tailCmd->_val);
			break;
		case kCmdSound:
			_vm->snSound(spr, tailCmd->_val);
			break;
		case kCmdCount:
			_vm->_sound->setRepeat(tailCmd->_val);
			break;
		case kCmdExec:
			switch (tailCmd->_cbType) {
			case kQGame:
				_vm->qGame();
				break;
			case kMiniStep:
				_vm->miniStep(tailCmd->_val);
				break;
			case kXScene:
				_vm->xScene();
				break;
			case kSoundSetVolume:
				_vm->sndSetVolume();
				break;
			default:
				error("Unknown Callback Type in SNEXEC");
			}
			break;
		case kCmdStep:
			spr->step();
			break;
		case kCmdZTrim:
			_vm->snZTrim(spr);
			break;
		case kCmdGhost:
			_vm->snGhost((Bitmap *) tailCmd->_spritePtr);
			break;
		default:
			warning("Unhandled snc->_com in SNMouse(bool)");
			break;
		}
		_tail++;
		if (!_turbo)
			break;
	}

	_busy = false;
}

bool CommandHandler::idle() {
	return (_head == _tail);
}

void CommandHandler::reset() {
	_tail = _head;
}

/**
 * Handles mini-Games logic
 * @param com			Command
 * @param num			mini game number
 */
void CGEEngine::snGame(Sprite *spr, int num) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snGame(spr, %d)", num);

	switch (num) {
	case 1: {
		static Sprite *dup[3] = { NULL, NULL, NULL };
		int buref = 0;
		int Stage = 0;

		for (dup[0] = _vga->_showQ->first(); dup[0]; dup[0] = dup[0]->_next) {
			buref = dup[0]->_ref;
			if (buref / 1000 == 16 && buref % 100 == 6) {
				Stage = (buref / 100) % 10;
				break;
			}
		}
		if (dup[1] == NULL) {
			dup[1] = _vga->_showQ->locate(16003);    // pan
			dup[2] = _vga->_showQ->locate(16004);    // pani
		}

		if (_game) { // continue game
			int i = newRandom(3), hand = (dup[0]->_shpCnt == 6);
			Stage++;
			if (hand && Stage > kDressed)
				++hand;
			if (i >= 0 && (dup[i] == spr && newRandom(3) == 0)) {
				_commandHandler->addCommand(kCmdSeq, -1, 3, dup[0]);               // Yes
				_commandHandler->addCommand(kCmdSeq, -1, 3, dup[1]);               // Yes
				_commandHandler->addCommand(kCmdSeq, -1, 3, dup[2]);               // Yes
				_commandHandler->addCommand(kCmdTNext, -1, 0, dup[0]);             // Reset Take
				_commandHandler->addCommand(kCmdTNext, -1, 0, dup[1]);             // Reset Take
				_commandHandler->addCommand(kCmdTNext, -1, 0, dup[2]);             // Reset Take
				_commandHandler->addCommand(kCmdNNext, -1, 0, dup[0]);             // Reset Near
				_commandHandler->addCommand(kCmdPause, -1, 72, NULL);              // Pause the game for 72/80 second
				_commandHandler->addCommand(kCmdSay, 1, 16009, NULL);              // Say "I win.."
				_commandHandler->addCommand(kCmdSay, buref, 16010, NULL);          // Say "Go Sit..."
				_commandHandler->addCommand(kCmdSay, 1, 16011, NULL);              // Say "I prefer not"

				if (hand) {
					_commandHandler->addCommand(kCmdSend, 16060 + hand, 16, NULL);   // Give hand
					_commandHandler->addCommand(kCmdSeq, buref, 4, NULL);            // Take off
					_commandHandler->addCommand(kCmdSeq, 16060 + hand, 1, NULL);     // start one of the Bartender animations
					_commandHandler->addCommand(kCmdSound, 16060 + hand, 16002, NULL); // Play tear sound
					_commandHandler->addCommand(kCmdWait, 16060 + hand, 3, NULL);    // Take up
					_commandHandler->addCommand(kCmdSwap, buref, buref + 100, NULL); // Open hand
					_commandHandler->addCommand(kCmdSeq, 16016, Stage, NULL);        // Start Belongings animation
					_commandHandler->addCommand(kCmdSend, 16060 + hand, -1, NULL);   // Hide hand
					_commandHandler->addCommand(kCmdWait, 16060 + hand, -1, NULL);   // Stop moving hand
				} else {
					_commandHandler->addCommand(kCmdSeq, buref, 4, NULL);            // Take off
					_commandHandler->addCommand(kCmdSound, 16060 + hand, 16002, NULL); // Play tear sound
					_commandHandler->addCommand(kCmdWait, buref, -1, NULL);          // Will take off
					_commandHandler->addCommand(kCmdSwap, buref, buref + 100, NULL); // Open hand
					_commandHandler->addCommand(kCmdSeq, 16016, Stage, NULL);        // Start Belongings animation
				}
				_commandHandler->addCommand(kCmdPause, -1, 72, NULL);              // Pause the game for 72/80 second
				_commandHandler->addCommand(kCmdSeq, -1, 0, dup[1]);               // Get away (Him)
				_commandHandler->addCommand(kCmdSetXY, -1, 203 + kScrWidth * 49, dup[1]);
				_commandHandler->addCommand(kCmdSetZ, -1, 7, dup[1]);
				_commandHandler->addCommand(kCmdSeq, -1, 0, dup[2]);               // Get Away (Her)
				_commandHandler->addCommand(kCmdSetXY, -1, 182 + kScrWidth * 62, dup[2]);
				_commandHandler->addCommand(kCmdSetZ, -1, 9, dup[2]);
				_game = false;
				return;
			} else {
				_commandHandler->addCommand(kCmdSeq, -1, 2, dup[0]);               // reset animation sequence
				_commandHandler->addCommand(kCmdSeq, -1, 2, dup[1]);               // reset animation sequence
				_commandHandler->addCommand(kCmdSeq, -1, 2, dup[2]);               // reset animation sequence
				_commandHandler->addCommand(kCmdPause, -1, 72, NULL);              // Pause the game for 72/80 second
			}
		}
		_commandHandler->addCommand(kCmdWalk, 198, 134, NULL);                 // Go to place
		_commandHandler->addCommand(kCmdWait, 1, -1, NULL);                    // Stop moving
		_commandHandler->addCommand(kCmdCover, 1, 16101, NULL);                // Man to beat
		_commandHandler->addCommand(kCmdSeq, 16101, 1, NULL);                  // Start Chief animation (16dupnia)
		_commandHandler->addCommand(kCmdWait, 16101, 5, NULL);                 // wait
		_commandHandler->addCommand(kCmdPause, 16101, 24, NULL);               // Pause the game for 24/80 second
		_commandHandler->addCommand(kCmdSeq, 16040, 1, NULL);                  // Start Slap animation (16plask)
		_commandHandler->addCommand(kCmdSound, 16101, 16001, NULL);            // Play "Slap" sound
		_commandHandler->addCommand(kCmdPause, 16101, 24, NULL);               // Pause the game for 24/80 second
		_commandHandler->addCommand(kCmdSeq, 16040, 0, NULL);                  // Reset animation sequence
		_commandHandler->addCommand(kCmdWait, 16101, -1, NULL);                // stay
		_commandHandler->addCommand(kCmdUncover, 1, 16101, NULL);              // SDS
		if (!_game) {
			_commandHandler->addCommand(kCmdSay, buref, 16008, NULL);            // say "Guess!"
			_game = true;
		}
		}
		break;
	case 2:
		if (_sprTv == NULL) {
			_sprTv = _vga->_showQ->locate(20700);
			_sprK1 = _vga->_showQ->locate(20701);
			_sprK2 = _vga->_showQ->locate(20702);
			_sprK3 = _vga->_showQ->locate(20703);
		}

		if (!_game) { // init
			_commandHandler->addCommand(kCmdGame, 20002, 2, NULL);
			_game = true;
			break;
		}

		// cont
		_sprK1->step(newRandom(6));
		_sprK2->step(newRandom(6));
		_sprK3->step(newRandom(6));

		if (spr->_ref == 1 && _keyboard->_keyAlt) {
			_sprK1->step(5);
			_sprK2->step(5);
			_sprK3->step(5);
		}

		_commandHandler->addCommand(kCmdSetZ, 20700, 0, NULL);
		bool hit = (_sprK1->_seqPtr + _sprK2->_seqPtr + _sprK3->_seqPtr == 15);
		if (hit) {
			if (spr->_ref == 1) {
				_commandHandler->addCommand(kCmdSay,       1, 20003, NULL);       // hurray!
				_commandHandler->addCommand(kCmdSeq,   20011,     2, NULL);       // Camera away
				_commandHandler->addCommand(kCmdSend,  20701,    -1, NULL);       // move dice1 to scene -1
				_commandHandler->addCommand(kCmdSend,  20702,    -1, NULL);       // move dice2 to scene -1
				_commandHandler->addCommand(kCmdSend,  20703,    -1, NULL);       // move dice3 to scene -1
				_commandHandler->addCommand(kCmdSend,  20700,    -1, NULL);       // move TV to scene -1
				_commandHandler->addCommand(kCmdKeep,  20007,     0, NULL);       // to pocket
				_commandHandler->addCommand(kCmdSend,  20006,    20, NULL);       // Move Coin to scene 20
				_commandHandler->addCommand(kCmdSound, 20006, 20002, NULL);       // Play Coin sound
				_commandHandler->addCommand(kCmdSay,   20002, 20004, NULL);	      // Say "Luck guy..."
				_commandHandler->addCommand(kCmdSend,  20010,    20, NULL);       // Move Paper to scene 20
				_commandHandler->addCommand(kCmdSound, 20010, 20003, NULL);       // Play "ksh" sound! (fx20003.wav)
				_commandHandler->addCommand(kCmdSay,   20001, 20005, NULL);       // Say "Congratulations"
				_game = false;
				return;
			} else
				_sprK3->step(newRandom(5));
		}

		if (_gameCase2Cpt < 100) {
			switch (_gameCase2Cpt) {
			case 15:
				// Give hint about ALTered dice
				_commandHandler->addCommand(kCmdSay, 20003, 20021, NULL);
				break;
			case 30:
			case 45:
			case 60:
			case 75:
				// Tell to use ALT key
				_commandHandler->addCommand(kCmdSay, 20003, 20022, NULL);
				break;
			}
			_gameCase2Cpt++;
		}

		switch (spr->_ref) {
		case 1:
			_commandHandler->addCommand(kCmdSay,   20001, 20011, NULL);        // Say "It'a my turn"
			_commandHandler->addCommand(kCmdSeq,   20001,     1, NULL);        // Throw dice
			_commandHandler->addCommand(kCmdWait,  20001,     1, NULL);        // wait
			_commandHandler->addCommand(kCmdSetZ,  20700,     2, NULL);        // hide dice
			_commandHandler->addCommand(kCmdHide,  20007,     1, NULL);        // hide dice
			_commandHandler->addCommand(kCmdWait,  20001,    16, NULL);        // wait
			_commandHandler->addCommand(kCmdSeq,   20007,     1, NULL);        // Start dice animation (20kosci)
			_commandHandler->addCommand(kCmdHide,  20007,     0, NULL);        // unhide
			_commandHandler->addCommand(kCmdSound, 20007, 20001, NULL);        // Play Dice sound
			_commandHandler->addCommand(kCmdWait,  20007,    -1, NULL);        // the end
			_commandHandler->addCommand(kCmdGame,  20001,     2, NULL);        // again!
			break;

		case 20001:
			_commandHandler->addCommand(kCmdSay,   20002, 20012, NULL);        // Say "Now it's mine"
			_commandHandler->addCommand(kCmdSeq,   20002,     1, NULL);        // Throw dice
			_commandHandler->addCommand(kCmdWait,  20002,     3, NULL);        // wait
			_commandHandler->addCommand(kCmdSetZ,  20700,     2, NULL);        // hide dice
			_commandHandler->addCommand(kCmdHide,  20007,     1, NULL);        // hide dice
			_commandHandler->addCommand(kCmdWait,  20002,    10, NULL);        // wait
			_commandHandler->addCommand(kCmdSeq,   20007,     2, NULL);        // Start dice animation (20kosci)
			_commandHandler->addCommand(kCmdHide,  20007,     0, NULL);        // unhide
			_commandHandler->addCommand(kCmdSound, 20007, 20001, NULL);        // Play Dice sound
			_commandHandler->addCommand(kCmdWait,  20007,    -1, NULL);        // the end
			_commandHandler->addCommand(kCmdGame,  20002,     2, NULL);        // again!
			break;

		case 20002:
			_commandHandler->addCommand(kCmdSay,   20002, 20010, NULL);        // "Roll the bones!"
			_commandHandler->addCommand(kCmdWalk,  20005,    -1, NULL);        // Walk to table
			_commandHandler->addCommand(kCmdWait,      1,    -1, NULL);        // Wait
			_commandHandler->addCommand(kCmdCover,     1, 20101, NULL);        // grasol ??
			_commandHandler->addCommand(kCmdSeq,   20101,     1, NULL);        // Start Chief animation (20solgra)
			_commandHandler->addCommand(kCmdWait,  20101,     5, NULL);        // Wait
			_commandHandler->addCommand(kCmdSetZ,  20700,     2, NULL);        // Hide dice
			_commandHandler->addCommand(kCmdHide,  20007,     1, NULL);        // Hide dice
			_commandHandler->addCommand(kCmdWait,  20101,    15, NULL);        // wait
			_commandHandler->addCommand(kCmdSeq,   20007,     1, NULL);        // Start dice animation (20kosci)
			_commandHandler->addCommand(kCmdHide,  20007,     0, NULL);        // Unhide
			_commandHandler->addCommand(kCmdSound, 20007, 20001, NULL);        // Play Dice sound
			_commandHandler->addCommand(kCmdWait,  20101,    -1, NULL);        // the end
			_commandHandler->addCommand(kCmdUncover,   1, 20101, NULL);        // SDS ??
			_commandHandler->addCommand(kCmdGame,      1,     2, NULL);        // again!
			break;
		}
	}
}

void CGEEngine::expandSprite(Sprite *spr) {
	debugC(5, kCGEDebugEngine, "CGEEngine::expandSprite(spr)");

	if (spr)
		_vga->_showQ->insert(_vga->_spareQ->remove(spr));
}

void CGEEngine::contractSprite(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::contractSprite(spr)");

	if (spr)
		_vga->_spareQ->append(_vga->_showQ->remove(spr));
}

/**
 * Check if an item is in the inventory, and returns its position
 * @param spr			Sprite pointer
 * @return -1			if not found, else index.
 */
int CGEEngine::findPocket(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::findPocket(spr)");

	for (int i = 0; i < kPocketNX; i++)
		if (_pocket[i] == spr)
			return i;
	return -1;
}

/**
 * Check if an item is in the inventory, and returns its position
 * @param Inventory slot number		Sprite pointer
 */
void CGEEngine::selectPocket(int n) {
	debugC(1, kCGEDebugEngine, "CGEEngine::selectPocket(%d)", n);

	if (n < 0 || (_pocLight->_seqPtr && _pocPtr == n)) {
		// If no slot specified, or another slot already selected
		// stop the blinking animation
		_pocLight->step(0);
		n = findPocket(NULL);
		if (n >= 0)
			_pocPtr = n;
	} else {
		// If slot specified, check if the slot if used.
		// Is so, start the blinking animation
		if (_pocket[n] != NULL) {
			_pocPtr = n;
			_pocLight->step(1);
		}
	}
	_pocLight->gotoxy(kPocketX + _pocPtr * kPocketDX + kPocketSX, kPocketY + kPocketSY);
}

/**
 * Logic used when all the inventory slots are full and the user tries to pick
 * another object.
 * @param Inventory slot number		Sprite pointer
 */
void CGEEngine::pocFul() {
	debugC(1, kCGEDebugEngine, "CGEEngine::pocFul()");

	_hero->park();
	_commandHandler->addCommand(kCmdWait, -1, -1, _hero);
	_commandHandler->addCommand(kCmdSeq, -1, kSeqPocketFull, _hero);
	_commandHandler->addCommand(kCmdSound, -1, 2, _hero); // Play the 'hum-hum" sound (fx00002)
	_commandHandler->addCommand(kCmdWait, -1, -1, _hero);
	_commandHandler->addCommand(kCmdSay,  1, kPocketFull, _hero);
}

void CGEEngine::hide1(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::hide1(spr)");

	_commandHandlerTurbo->addCommand(kCmdGhost, -1, 0, spr->ghost());
}

void CGEEngine::snGhost(Bitmap *bmp) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snGhost(bmp)");

	bmp->hide(bmp->_map & 0xFFFF, bmp->_map >> 16);
	bmp->_m = NULL;
	bmp->_map = 0;
	delete bmp;
}

void CGEEngine::feedSnail(Sprite *spr, SnList snq) {
	debugC(1, kCGEDebugEngine, "CGEEngine::feedSnail(spr, snq)");

	if (!spr || !spr->active())
		return;

	uint8 ptr = (snq == kTake) ? spr->_takePtr : spr->_nearPtr;

	if (ptr == kNoPtr)
		return;

	CommandHandler::Command *comtab = spr->snList(snq);
	CommandHandler::Command *c = comtab + ptr;

	if (findPocket(NULL) < 0) {                 // no empty pockets?
		CommandHandler::Command *p;
		for (p = c; p->_commandType != kCmdNext; p++) {     // find KEEP command
			if (p->_commandType == kCmdKeep) {
				pocFul();
				return;
			}
			if (p->_spritePtr)
				break;
		}
	}
	while (true) {
		if (c->_commandType == kCmdTalk) {
			if ((_commandHandler->_talkEnable = (c->_val != 0)) == false)
				killText();
		}
		if (c->_commandType == kCmdNext) {
			Sprite *s = (c->_ref < 0) ? spr : locate(c->_ref);
			if (s) {
				uint8 *idx = (snq == kTake) ? &s->_takePtr : &s->_nearPtr;
				if (*idx != kNoPtr) {
					int v;
					switch (c->_val) {
					case -1 :
						v = c - comtab + 1;
						break;
					case -2 :
						v = c - comtab;
						break;
					case -3 :
						v = -1;
						break;
					default :
						v = c->_val;
						break;
					}
					if (v >= 0)
						*idx = v;
				}
			}
			if (s == spr)
				break;
		}
		if (c->_commandType == kCmdIf) {
			Sprite *s = (c->_ref < 0) ? spr : locate(c->_ref);
			if (s) { // sprite extsts
				if (! s->seqTest(-1))
					c = comtab + c->_val;                // not parked
				else
					++c;
			} else
				++c;
		} else {
			_commandHandler->addCommand(c->_commandType, c->_ref, c->_val, spr);
			if (c->_spritePtr)
				break;
			else
				c++;
		}
	}
}

void CGEEngine::snNNext(Sprite *spr, int p) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snNNext(spr, %d)", p);

	if (spr)
		if (spr->_nearPtr != kNoPtr)
			spr->_nearPtr = p;
}

void CGEEngine::snTNext(Sprite *spr, int p) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snTNext(spr, %d)", p);

	if (spr)
		if (spr->_takePtr != kNoPtr)
			spr->_takePtr = p;
}

void CGEEngine::snRNNext(Sprite *spr, int p) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRNNext(spr, %d)", p);

	if (spr)
		if (spr->_nearPtr != kNoPtr)
			spr->_nearPtr += p;
}


void CGEEngine::snRTNext(Sprite *spr, int p) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRTNext(spr, %d)", p);

	if (spr)
		if (spr->_takePtr != kNoPtr)
			spr->_takePtr += p;
}

void CGEEngine::snZTrim(Sprite *spr) {
	debugC(4, kCGEDebugEngine, "CGEEngine::snZTrim(spr)");

	if (!spr || !spr->active())
		return;

	Sprite *s = (spr->_flags._shad) ? spr->_prev : NULL;
	_vga->_showQ->insert(_vga->_showQ->remove(spr));
	if (s) {
		s->_z = spr->_z;
		_vga->_showQ->insert(_vga->_showQ->remove(s), spr);
	}
}

void CGEEngine::snHide(Sprite *spr, int val) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snHide(spr, %d)", val);

	if (spr) {
		spr->_flags._hide = (val >= 0) ? (val != 0) : (!spr->_flags._hide);
		if (spr->_flags._shad)
			spr->_prev->_flags._hide = spr->_flags._hide;
	}
}

void CGEEngine::snRmNear(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRmNear(spr)");

	if (spr)
		spr->_nearPtr = kNoPtr;
}

void CGEEngine::snRmTake(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRmTake(spr)");

	if (spr)
		spr->_takePtr = kNoPtr;
}

void CGEEngine::snSeq(Sprite *spr, int val) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSeq(spr, %d)", val);

	if (spr) {
		if (spr == _hero && val == 0)
			_hero->park();
		else
			spr->step(val);
	}
}

void CGEEngine::snRSeq(Sprite *spr, int val) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRSeq(spr, %d)", val);

	if (spr)
		snSeq(spr, spr->_seqPtr + val);
}

void CGEEngine::snSend(Sprite *spr, int val) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSend(spr, %d)", val);

	if (!spr)
		return;

	int was = spr->_scene;
	bool was1 = (was == 0 || was == _now);
	bool val1 = (val == 0 || val == _now);
	spr->_scene = val;
	if (val1 != was1) {
		if (was1) {
			if (spr->_flags._kept) {
				int n = findPocket(spr);
				if (n >= 0)
					_pocket[n] = NULL;
			}
			hide1(spr);
			contractSprite(spr);
			spr->_flags._slav = false;
		} else {
			if (spr->_ref % 1000 == 0)
				_bitmapPalette = _vga->_sysPal;
			if (spr->_flags._back)
				spr->backShow(true);
			else
				expandSprite(spr);
			_bitmapPalette = NULL;
		}
	}
}

void CGEEngine::snSwap(Sprite *spr, int xref) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSwap(spr, %d)", xref);

	Sprite *xspr = locate(xref);
	if (!spr || !xspr)
		return;

	int was = spr->_scene;
	int xwas = xspr->_scene;
	bool was1 = (was == 0 || was == _now);
	bool xwas1 = (xwas == 0 || xwas == _now);

	SWAP(spr->_scene, xspr->_scene);
	SWAP(spr->_x, xspr->_x);
	SWAP(spr->_y, xspr->_y);
	SWAP(spr->_z, xspr->_z);
	if (spr->_flags._kept) {
		int n = findPocket(spr);
		if (n >= 0)
			_pocket[n] = xspr;
		xspr->_flags._kept = true;
		xspr->_flags._port = false;
	}
	if (xwas1 != was1) {
		if (was1) {
			hide1(spr);
			contractSprite(spr);
		} else
			expandSprite(spr);
		if (xwas1) {
			hide1(xspr);
			contractSprite(xspr);
		} else
			expandSprite(xspr);
	}
}

void CGEEngine::snCover(Sprite *spr, int xref) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snCover(spr, %d)", xref);

	Sprite *xspr = locate(xref);
	if (!spr || !xspr)
		return;

	spr->_flags._hide = true;
	xspr->_z = spr->_z;
	xspr->_scene = spr->_scene;
	xspr->gotoxy(spr->_x, spr->_y);
	expandSprite(xspr);
	if ((xspr->_flags._shad = spr->_flags._shad) == 1) {
		_vga->_showQ->insert(_vga->_showQ->remove(spr->_prev), xspr);
		spr->_flags._shad = false;
	}
	feedSnail(xspr, kNear);
}

void CGEEngine::snUncover(Sprite *spr, Sprite *xspr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snUncover(spr, xspr)");

	if (!spr || !xspr)
		return;

	spr->_flags._hide = false;
	spr->_scene = xspr->_scene;
	spr->gotoxy(xspr->_x, xspr->_y);
	if ((spr->_flags._shad = xspr->_flags._shad) == 1) {
		_vga->_showQ->insert(_vga->_showQ->remove(xspr->_prev), spr);
		xspr->_flags._shad = false;
	}
	spr->_z = xspr->_z;
	snSend(xspr, -1);
	if (spr->_time == 0)
		spr->_time++;
}

void CGEEngine::snSetX0(int scene, int x0) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetX0(%d, %d)", scene, x0);

	_heroXY[scene - 1].x = x0;
}

void CGEEngine::snSetY0(int scene, int y0) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetY0(%d, %d)", scene, y0);

	_heroXY[scene - 1].y = y0;
}

void CGEEngine::snSetXY(Sprite *spr, uint16 xy) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetXY(spr, %d)", xy);

	if (spr)
		spr->gotoxy(xy % kScrWidth, xy / kScrWidth);
}

void CGEEngine::snRelX(Sprite *spr, int x) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRelX(spr, %d)", x);

	if (spr && _hero)
		spr->gotoxy(_hero->_x + x, spr->_y);
}

void CGEEngine::snRelY(Sprite *spr, int y) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRelY(spr, %d)", y);

	if (spr && _hero)
		spr->gotoxy(spr->_x, _hero->_y + y);
}

void CGEEngine::snRelZ(Sprite *spr, int z) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snRelZ(spr, %d)", z);

	if (spr && _hero) {
		spr->_z = _hero->_z + z;
		snZTrim(spr);
	}
}

void CGEEngine::snSetX(Sprite *spr, int x) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetX(spr, %d)", x);

	if (spr)
		spr->gotoxy(x, spr->_y);
}

void CGEEngine::snSetY(Sprite *spr, int y) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetY(spr, %d)", y);

	if (spr)
		spr->gotoxy(spr->_x, y);
}

void CGEEngine::snSetZ(Sprite *spr, int z) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetZ(spr, %d)", z);

	if (spr) {
		spr->_z = z;
		snZTrim(spr);
	}
}

void CGEEngine::snSlave(Sprite *spr, int ref) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSlave(spr, %d)", ref);

	Sprite *slv = locate(ref);
	if (spr && slv) {
		if (spr->active()) {
			snSend(slv, spr->_scene);
			slv->_flags._slav = true;
			slv->_z = spr->_z;
			_vga->_showQ->insert(_vga->_showQ->remove(slv), spr->_next);
		}
	}
}

void CGEEngine::snTrans(Sprite *spr, int trans) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snTrans(spr, %d)", trans);

	if (spr)
		spr->_flags._tran = (trans < 0) ? !spr->_flags._tran : (trans != 0);
}

void CGEEngine::snPort(Sprite *spr, int port) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snPort(spr, %d)", port);

	if (spr)
		spr->_flags._port = (port < 0) ? !spr->_flags._port : (port != 0);
}

void CGEEngine::snKill(Sprite *spr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snKill(spr)");

	if (!spr)
		return;

	if (spr->_flags._kept) {
		int n = findPocket(spr);
		if (n >= 0)
			_pocket[n] = NULL;
	}
	Sprite *nx = spr->_next;
	hide1(spr);
	_vga->_showQ->remove(spr);
	_eventManager->clearEvent(spr);
	if (spr->_flags._kill) {
		delete spr;
	} else {
		spr->_scene = -1;
		_vga->_spareQ->append(spr);
	}
	if (nx) {
		if (nx->_flags._slav)
			snKill(nx);
	}
}

/**
 * Play a FX sound
 * @param spr			Sprite pointer
 * @param wav			FX index
 */
void CGEEngine::snSound(Sprite *spr, int wav) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSound(spr, %d)", wav);

	if (wav == -1)
		_sound->stop();
	else
		_sound->play((*_fx)[wav], (spr) ? ((spr->_x + spr->_w / 2) / (kScrWidth / 16)) : 8);

	_sound->setRepeat(1);
}

void CGEEngine::snKeep(Sprite *spr, int stp) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snKeep(spr, %d)", stp);

	selectPocket(-1);
	if (spr && ! spr->_flags._kept && _pocket[_pocPtr] == NULL) {
		int16 oldRepeat = _sound->getRepeat();
		_sound->setRepeat(1);
		snSound(spr, 3);
		_sound->setRepeat(oldRepeat);
		_pocket[_pocPtr] = spr;
		spr->_scene = 0;
		spr->_flags._kept = true;
		spr->gotoxy(kPocketX + kPocketDX * _pocPtr + kPocketDX / 2 - spr->_w / 2,
		          kPocketY + kPocketDY / 2 - spr->_h / 2);
		if (stp >= 0)
			spr->step(stp);
	}
	selectPocket(-1);
}

/**
 * Remove an object from the inventory and (if specified) trigger an animation
 * @param spr			Inventory item
 * @param stp			Animation
 */
void CGEEngine::snGive(Sprite *spr, int stp) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snGive(spr, %d)", stp);

	if (spr) {
		int p = findPocket(spr);
		if (p >= 0) {
			_pocket[p] = NULL;
			spr->_scene = _now;
			spr->_flags._kept = false;
			if (stp >= 0)
				spr->step(stp);
		}
	}
	selectPocket(-1);
}

void CGEEngine::snBackPt(Sprite *spr, int stp) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snBackPt(spr, %d)", stp);

	if (spr) {
		if (stp >= 0)
			spr->step(stp);
		spr->backShow(true);
	}
}

void CGEEngine::snLevel(Sprite *spr, int lev) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snLevel(spr, %d)", lev);

	assert((lev >= 0) && (lev < 5));

	for (int i = 0; i < 5; i++) {
		spr = _vga->_spareQ->locate(100 + i);
		if (spr) {
			if (i <= lev) {
				spr->backShow(true);
				spr->_scene = 0;
				spr->_flags._hide = false;
			} else {
				spr->_flags._hide = true;
				spr->_scene = -1;
			}
		} else {
			warning("SPR not found! ref: %d", 100 + i);
		}
	}

	_lev = lev;
	_maxScene = _maxSceneArr[_lev];
}

/**
 * Set a flag to a value
 * @param indx			Flag index
 * @param val			Flag value
 */
void CGEEngine::snFlag(int indx, bool val) {
	_flag[indx] = val;
}

void CGEEngine::snSetRef(Sprite *spr, int nr) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snSetRef(spr, %d)", nr);

	if (spr)
		spr->_ref = nr;
}

void CGEEngine::snFlash(bool on) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snFlash(%s)", on ? "true" : "false");

	if (on) {
		Dac *pal = (Dac *)malloc(sizeof(Dac) * kPalCount);
		if (pal) {
			memcpy(pal, _vga->_sysPal, kPalSize);
			for (int i = 0; i < kPalCount; i++) {
				register int c;
				c = pal[i]._r << 1;
				pal[i]._r = (c < 64) ? c : 63;
				c = pal[i]._g << 1;
				pal[i]._g = (c < 64) ? c : 63;
				c = pal[i]._b << 1;
				pal[i]._b = (c < 64) ? c : 63;
			}
			_vga->setColors(pal, 64);
		}
	} else
		_vga->setColors(_vga->_sysPal, 64);
	_dark = false;
}

void CGEEngine::snLight(bool in) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snLight(%s)", in ? "true" : "false");

	if (in)
		_vga->sunrise(_vga->_sysPal);
	else
		_vga->sunset();
	_dark = !in;
}

/**
 * Set an horizontal boundary
 * @param scene			Scene number
 * @param barX			Horizontal boundary value
 */
void CGEEngine::snHBarrier(const int scene, const int barX) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snHBarrier(%d, %d)", scene, barX);

	_barriers[(scene > 0) ? scene : _now]._horz = barX;
}

/**
 * Set a vertical boundary
 * @param scene			Scene number
 * @param barY			Vertical boundary value
 */
void CGEEngine::snVBarrier(const int scene, const int barY) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snVBarrier(%d, %d)", scene, barY);

	_barriers[(scene > 0) ? scene : _now]._vert = barY;
}

void CGEEngine::snWalk(Sprite *spr, int x, int y) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snWalk(spr, %d, %d)", x, y);

	if (_hero) {
		if (spr && y < 0)
			_hero->findWay(spr);
		else
			_hero->findWay(XZ(x, y));
	}
}

void CGEEngine::snReach(Sprite *spr, int mode) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snReach(spr, %d)", mode);

	if (_hero)
		_hero->reach(spr, mode);
}

void CGEEngine::snMouse(bool on) {
	debugC(1, kCGEDebugEngine, "CGEEngine::snMouse(%s)", on ? "true" : "false");

	if (on)
		_mouse->on();
	else
		_mouse->off();
}

} // End of namespace CGE
