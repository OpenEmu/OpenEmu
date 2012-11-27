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
#include "agos/animation.h"
#include "agos/debugger.h"
#include "agos/intern.h"

#include "common/events.h"
#include "common/system.h"
#include "common/textconsole.h"

#include "backends/audiocd/audiocd.h"

#include "gui/about.h"

#include "graphics/surface.h"


namespace AGOS {

void AGOSEngine::addTimeEvent(uint16 timeout, uint16 subroutine_id) {
	TimeEvent *te = (TimeEvent *)malloc(sizeof(TimeEvent)), *first, *last = NULL;
	uint32 cur_time = getTime();

	if (getGameId() == GID_DIMP) {
		timeout /= 2;
	}

	te->time = cur_time + timeout - _gameStoppedClock;
	if (getGameType() == GType_FF && _clockStopped)
		te->time -= (getTime() - _clockStopped);
	te->subroutine_id = subroutine_id;

	first = _firstTimeStruct;
	while (first) {
		if (te->time <= first->time) {
			if (last) {
				last->next = te;
				te->next = first;
				return;
			}
			te->next = _firstTimeStruct;
			_firstTimeStruct = te;
			return;
		}

		last = first;
		first = first->next;
	}

	if (last) {
		last->next = te;
		te->next = NULL;
	} else {
		_firstTimeStruct = te;
		te->next = NULL;
	}
}

void AGOSEngine::delTimeEvent(TimeEvent *te) {
	TimeEvent *cur;

	if (te == _pendingDeleteTimeEvent)
		_pendingDeleteTimeEvent = NULL;

	if (te == _firstTimeStruct) {
		_firstTimeStruct = te->next;
		free(te);
		return;
	}

	cur = _firstTimeStruct;
	if (cur == NULL)
		error("delTimeEvent: none available");

	for (;;) {
		if (cur->next == NULL)
			error("delTimeEvent: no such te");
		if (te == cur->next) {
			cur->next = te->next;
			free(te);
			return;
		}
		cur = cur->next;
	}
}

void AGOSEngine::invokeTimeEvent(TimeEvent *te) {
	Subroutine *sub;

	_scriptVerb = 0;

	if (_runScriptReturn1)
		return;

	sub = getSubroutineByID(te->subroutine_id);
	if (sub != NULL)
		startSubroutineEx(sub);

	_runScriptReturn1 = false;
}

void AGOSEngine::killAllTimers() {
	TimeEvent *cur, *next;

	for (cur = _firstTimeStruct; cur; cur = next) {
		next = cur->next;
		delTimeEvent(cur);
	}
	_clickOnly = false;
}

bool AGOSEngine::kickoffTimeEvents() {
	uint32 cur_time;
	TimeEvent *te;
	bool result = false;

	if (getGameType() == GType_FF && _clockStopped)
		return result;

	cur_time = getTime() - _gameStoppedClock;

	while ((te = _firstTimeStruct) != NULL && te->time <= cur_time && !shouldQuit()) {
		result = true;
		_pendingDeleteTimeEvent = te;
		invokeTimeEvent(te);
		if (_pendingDeleteTimeEvent) {
			_pendingDeleteTimeEvent = NULL;
			delTimeEvent(te);
		}
	}

	return result;
}

bool AGOSEngine::isVgaQueueEmpty() {
	VgaTimerEntry *vte = _vgaTimerList;
	bool result = false;

	while (vte->delay) {
		if (vte->zoneNum == _variableArray[999] && vte->id >= 100) {
			result = true;
			break;
		}
		vte++;
	}
	return result;
}

void AGOSEngine::haltAnimation() {
	if (_videoLockOut & 0x10)
		return;

	_videoLockOut |= 0x10;

	if (_displayFlag) {
		displayScreen();
		_displayFlag = 0;
	}
}

void AGOSEngine::restartAnimation() {
	if (!(_videoLockOut & 0x10))
		return;

	if (getGameType() != GType_PN) {
		_window4Flag = 2;
		setMoveRect(0, 0, 224, 127);
		displayScreen();
	}

	_videoLockOut &= ~0x10;
}

void AGOSEngine::addVgaEvent(uint16 num, uint8 type, const byte *codePtr, uint16 curSprite, uint16 curZoneNum) {
	VgaTimerEntry *vte;

	_videoLockOut |= 1;

	for (vte = _vgaTimerList; vte->delay; vte++) {
	}

	vte->delay = num;
	vte->codePtr = codePtr;
	vte->id = curSprite;
	vte->zoneNum = curZoneNum;
	vte->type = type;

	_videoLockOut &= ~1;
}

void AGOSEngine::deleteVgaEvent(VgaTimerEntry * vte) {
	_videoLockOut |= 1;

	if (vte + 1 <= _nextVgaTimerToProcess) {
		_nextVgaTimerToProcess--;
	}

	do {
		memcpy(vte, vte + 1, sizeof(VgaTimerEntry));
		vte++;
	} while (vte->delay);

	_videoLockOut &= ~1;
}

void AGOSEngine::processVgaEvents() {
	VgaTimerEntry *vte = _vgaTimerList;

	_vgaTickCounter++;

	while (vte->delay) {
		vte->delay -= _vgaBaseDelay;
		if (vte->delay <= 0) {
			uint16 curZoneNum = vte->zoneNum;
			uint16 curSprite = vte->id;
			const byte *script_ptr = vte->codePtr;

			switch (vte->type) {
			case ANIMATE_INT:
				vte->delay = (getGameType() == GType_SIMON2) ? 5 : _frameCount;
				animateSprites();
				vte++;
				break;
			case ANIMATE_EVENT:
				_nextVgaTimerToProcess = vte + 1;
				deleteVgaEvent(vte);
				animateEvent(script_ptr, curZoneNum, curSprite);
				vte = _nextVgaTimerToProcess;
				break;
			case SCROLL_EVENT:
				_nextVgaTimerToProcess = vte + 1;
				deleteVgaEvent(vte);
				scrollEvent();
				vte = _nextVgaTimerToProcess;
				break;
			case PLAYER_DAMAGE_EVENT:
				playerDamageEvent(vte, curZoneNum);
				vte = _nextVgaTimerToProcess;
				break;
			case MONSTER_DAMAGE_EVENT:
				monsterDamageEvent(vte, curZoneNum);
				vte = _nextVgaTimerToProcess;
				break;
			default:
				error("processVgaEvents: Unknown event type %d", vte->type);
			}
		} else {
			vte++;
		}
	}
}

void AGOSEngine::animateEvent(const byte *codePtr, uint16 curZoneNum, uint16 curSprite) {
	VgaPointersEntry *vpe;

	_vgaCurSpriteId = curSprite;

	_vgaCurZoneNum = curZoneNum;
	_zoneNumber = curZoneNum;
	vpe = &_vgaBufferPointers[curZoneNum];

	_curVgaFile1 = vpe->vgaFile1;
	_curVgaFile2 = vpe->vgaFile2;
	_curSfxFile = vpe->sfxFile;
	_curSfxFileSize = vpe->sfxFileEnd - vpe->sfxFile;

	_vcPtr = codePtr;

	runVgaScript();
}

void AGOSEngine::scrollEvent() {
	if (_scrollCount == 0)
		return;

	if (getGameType() == GType_FF) {
		if (_scrollCount < 0) {
			if (_scrollFlag != -8) {
				_scrollFlag = -8;
				_scrollCount += 8;
			}
		} else {
			if (_scrollFlag != 8) {
				_scrollFlag = 8;
				_scrollCount -= 8;
			}
		}
	} else {
		if (_scrollCount < 0) {
			if (_scrollFlag != -1) {
				_scrollFlag = -1;
				if (++_scrollCount == 0)
					return;
			}
		} else {
			if (_scrollFlag != 1) {
				_scrollFlag = 1;
				if (--_scrollCount == 0)
					return;
			}
		}

		addVgaEvent(6, SCROLL_EVENT, NULL, 0, 0);
	}
}

static const byte _image1[32] = {
	0x3A, 0x37, 0x3B, 0x37,
	0x3A, 0x3E, 0x3F, 0x3E,
	0x37, 0x3F, 0x31, 0x3F,
	0x37, 0x3F, 0x31, 0x3F,
	0x3A, 0x3E, 0x3F, 0x3E,
	0x3A, 0x37, 0x3B, 0x37,
};

static const byte _image2[32] = {
	0x3A, 0x3A, 0x3B, 0x3A,
	0x3A, 0x37, 0x3E, 0x37,
	0x3A, 0x37, 0x3E, 0x37,
	0x3A, 0x37, 0x3E, 0x37,
	0x3A, 0x37, 0x3E, 0x37,
	0x3A, 0x3A, 0x3B, 0x3A,
};

static const byte _image3[32] = {
	0x3A, 0x32, 0x3B, 0x32,
	0x3A, 0x39, 0x3F, 0x39,
	0x32, 0x3F, 0x31, 0x3F,
	0x32, 0x3F, 0x31, 0x3F,
	0x3A, 0x39, 0x3F, 0x39,
	0x3A, 0x32, 0x3B, 0x32,
};

static const byte _image4[32] = {
	0x3A, 0x3A, 0x3B, 0x3A,
	0x3A, 0x32, 0x39, 0x32,
	0x3A, 0x32, 0x38, 0x32,
	0x3A, 0x32, 0x38, 0x32,
	0x3A, 0x32, 0x39, 0x32,
	0x3A, 0x3A, 0x3B, 0x3A,
};

void AGOSEngine::drawStuff(const byte *src, uint xoffs) {
	const uint8 y = (getPlatform() == Common::kPlatformAtariST) ? 132 : 135;

	Graphics::Surface *screen = _system->lockScreen();
	byte *dst = (byte *)screen->pixels + y * screen->pitch + xoffs;

	for (uint h = 0; h < 6; h++) {
		memcpy(dst, src, 4);
		src += 4;
		dst += screen->pitch;
	}

	_system->unlockScreen();
}

void AGOSEngine::playerDamageEvent(VgaTimerEntry * vte, uint dx) {
	// Draws damage indicator gauge when player hit
	_nextVgaTimerToProcess = vte + 1;

	if (!_opcode177Var1) {
		drawStuff(_image1, 4 + _opcode177Var2 * 4);
		_opcode177Var2++;
		if (_opcode177Var2 == dx) {
			_opcode177Var1 = 1;
			vte->delay = 16 - dx;
		} else {
			vte->delay = 1;
		}
	} else if (_opcode177Var2) {
		_opcode177Var2--;
		drawStuff(_image2, 4 + _opcode177Var2 * 4);
		vte->delay = 3;
	} else {
		deleteVgaEvent(vte);
	}
}

void AGOSEngine::monsterDamageEvent(VgaTimerEntry * vte, uint dx) {
	// Draws damage indicator gauge when monster hit
	_nextVgaTimerToProcess = vte + 1;

	if (!_opcode178Var1) {
		drawStuff(_image3, 275 + _opcode178Var2 * 4);
		_opcode178Var2++;
		if (_opcode178Var2 >= 10 || _opcode178Var2 == dx) {
			_opcode178Var1 = 1;
			vte->delay = 16 - dx;
		} else {
			vte->delay = 1;
		}
	} else if (_opcode178Var2) {
		_opcode178Var2--;
		drawStuff(_image4, 275 + _opcode178Var2 * 4);
		vte->delay = 3;
	} else {
		deleteVgaEvent(vte);
	}
}

void AGOSEngine::delay(uint amount) {
	Common::Event event;

	uint32 start = _system->getMillis();
	uint32 cur = start;
	uint this_delay, vgaPeriod;

	_system->getAudioCDManager()->updateCD();

	_debugger->onFrame();

	vgaPeriod = (_fastMode) ? 10 : _vgaPeriod;
	if (getGameType() == GType_PP && getGameId() != GID_DIMP) {
		if (vgaPeriod == 15 && _variableArray[999] == 0)
			vgaPeriod = 30;
	}

	_rnd.getRandomNumber(2);

	do {
		while (!_inCallBack && cur >= _lastVgaTick + vgaPeriod && !_pause) {
			_lastVgaTick += vgaPeriod;

			// don't get too many frames behind
			if (cur >= _lastVgaTick + vgaPeriod * 2)
				_lastVgaTick = cur;

			_inCallBack = true;
			timerProc();
			_inCallBack = false;
		}

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_KEYDOWN:
				if (event.kbd.keycode >= Common::KEYCODE_0 && event.kbd.keycode <= Common::KEYCODE_9
					&& (event.kbd.hasFlags(Common::KBD_ALT) ||
						event.kbd.hasFlags(Common::KBD_CTRL))) {
					_saveLoadSlot = event.kbd.keycode - Common::KEYCODE_0;

					// There is no save slot 0
					if (_saveLoadSlot == 0)
						_saveLoadSlot = 10;

					memset(_saveLoadName, 0, sizeof(_saveLoadName));
					sprintf(_saveLoadName, "Quick %d", _saveLoadSlot);
					_saveLoadType = (event.kbd.hasFlags(Common::KBD_ALT)) ? 1 : 2;
					quickLoadOrSave();
				} else if (event.kbd.hasFlags(Common::KBD_CTRL)) {
					if (event.kbd.keycode == Common::KEYCODE_a) {
						GUI::Dialog *_aboutDialog;
						_aboutDialog = new GUI::AboutDialog();
						_aboutDialog->runModal();
					} else if (event.kbd.keycode == Common::KEYCODE_f) {
						_fastMode = !_fastMode;
					} else if (event.kbd.keycode == Common::KEYCODE_d) {
						_debugger->attach();
					} else if (event.kbd.keycode == Common::KEYCODE_s) {
						dumpAllSubroutines();
					} else if (event.kbd.keycode == Common::KEYCODE_i) {
						dumpAllVgaImageFiles();
					}
				}

				if (getGameType() == GType_PP) {
					if (event.kbd.hasFlags(Common::KBD_SHIFT))
						_variableArray[41] = 0;
					else
						_variableArray[41] = 1;
				}

				_keyPressed = event.kbd;
				break;
			case Common::EVENT_MOUSEMOVE:
				break;
			case Common::EVENT_LBUTTONDOWN:
				if (getGameType() == GType_FF)
					setBitFlag(89, true);
				_leftButtonDown = true;
				_leftButton = 1;
				break;
			case Common::EVENT_LBUTTONUP:
				if (getGameType() == GType_FF)
					setBitFlag(89, false);

				_leftButton = 0;
				_leftButtonCount = 0;
				_leftClick = true;
				break;
			case Common::EVENT_RBUTTONDOWN:
				if (getGameType() == GType_FF)
					setBitFlag(92, false);
				_rightButtonDown = true;
				break;
			case Common::EVENT_RBUTTONUP:
				_rightClick = true;
				break;
			case Common::EVENT_RTL:
			case Common::EVENT_QUIT:
				return;
			default:
				break;
			}
		}

		if (_leftButton == 1)
			_leftButtonCount++;

		_system->getAudioCDManager()->updateCD();

		_system->updateScreen();

		if (amount == 0)
			break;

		this_delay = _fastMode ? 1 : 20;
		if (this_delay > amount)
			this_delay = amount;
		_system->delayMillis(this_delay);

		cur = _system->getMillis();
	} while (cur < start + amount && !shouldQuit());
}

#ifdef ENABLE_AGOS2
void AGOSEngine_DIMP::timerProc() {
	_lastTickCount = _system->getMillis();

	AGOSEngine_Feeble::timerProc();
	dimpIdle();
}

void AGOSEngine_Feeble::timerProc() {
	if (_videoLockOut & 0x80E9 || _videoLockOut & 2)
		return;

	_syncCount++;

	_videoLockOut |= 2;

	if (!(_videoLockOut & 0x10)) {
		_syncFlag2 = !_syncFlag2;
		if (!_syncFlag2) {
			processVgaEvents();
		} else {
			// Double speed on Oracle
			if (getGameType() == GType_FF && getBitFlag(99)) {
				processVgaEvents();
			} else if (_scrollCount == 0) {
				_videoLockOut &= ~2;
				return;
			}
		}

		if (getGameType() == GType_FF && _interactiveVideo) {
			// Controls Omni TV videos
			if (getBitFlag(42)) {
				stopInteractiveVideo();
			} else {
				_moviePlayer->nextFrame();
			}
		}

		animateSprites();
	}

	if (_displayFlag) {
		if (getGameType() == GType_FF && !(getFeatures() & GF_DEMO)) {
			if (!getBitFlag(78)) {
				oracleLogo();
			}
			if (getBitFlag(76)) {
				swapCharacterLogo();
			}
		}
		handleMouseMoved();
		displayScreen();
		_displayFlag = 0;
	}

	_videoLockOut &= ~2;
}
#endif

void AGOSEngine_PN::timerProc() {
	if (_videoLockOut & 0x80E9 || _videoLockOut & 2)
		return;

	_syncCount++;

	_videoLockOut |= 2;

	_sound->handleSoundQueue();
	handleMouseMoved();
	handleKeyboard();

	if (!(_videoLockOut & 0x10)) {
		if (_sampleWait) {
			_vgaCurSpriteId = 0xFFFF;
			vc15_sync();
			_sampleWait = false;
		}
		if (_sampleEnd) {
			_vgaCurSpriteId = 0xFFFE;
			vc15_sync();
			_sampleEnd = false;
		}

		processVgaEvents();
		processVgaEvents();
		_cepeFlag = !_cepeFlag;
		if (!_cepeFlag)
			processVgaEvents();
	}

	if (_displayFlag) {
		displayScreen();
		_displayFlag = 0;
	}

	_videoLockOut &= ~2;
}

void AGOSEngine::timerProc() {
	if (_videoLockOut & 0x80E9 || _videoLockOut & 2)
		return;

	_syncCount++;

	_videoLockOut |= 2;

	handleMouseMoved();

	if (!(_videoLockOut & 0x10)) {
		processVgaEvents();
		processVgaEvents();
		_cepeFlag = !_cepeFlag;
		if (!_cepeFlag)
			processVgaEvents();
	}

	if (_displayFlag) {
		displayScreen();
		_displayFlag = 0;
	}

	_videoLockOut &= ~2;
}

#ifdef ENABLE_AGOS2
void AGOSEngine_DIMP::dimpIdle() {
	int z, n;

	_iconToggleCount++;
	if (_iconToggleCount == 30) {
		if ((_variableArray[110] < 3) || (_variableArray[111] < 3) || (_variableArray[112] < 3)) {
			_voiceCount++;
			if (_voiceCount == 50) {
				if (!getBitFlag(14) && !getBitFlag(11) && !getBitFlag(13)) {
					loadSoundFile("Whistle.WAV");
					z = 0;
					while (z == 0) {
						n = _rnd.getRandomNumber(2);
						switch (n) {
							case(0):
								if (_variableArray[110] > 2)
									break;
								n = _rnd.getRandomNumber(6);
								switch (n) {
									case(0): loadSoundFile("And01.wav");break;
									case(1): loadSoundFile("And02.wav");break;
									case(2): loadSoundFile("And03.wav");break;
									case(3): loadSoundFile("And04.wav");break;
									case(4): loadSoundFile("And05.wav");break;
									case(5): loadSoundFile("And06.wav");break;
									case(6): loadSoundFile("And07.wav");break;
								}
								z = 1;
								break;
							case(1):
								if (_variableArray[111] > 2)
									break;
								n = _rnd.getRandomNumber(6);
								switch (n) {
									case(0): loadSoundFile("And08.wav");break;
									case(1): loadSoundFile("And09.wav");break;
									case(2): loadSoundFile("And0a.wav");break;
									case(3): loadSoundFile("And0b.wav");break;
									case(4): loadSoundFile("And0c.wav");break;
									case(5): loadSoundFile("And0d.wav");break;
									case(6): loadSoundFile("And0e.wav");break;
								}
								z = 1;
								break;
							case(2):
								if (_variableArray[112] > 2)
									break;
								n = _rnd.getRandomNumber(4);
								switch (n) {
									case(0): loadSoundFile("And0f.wav");break;
									case(1): loadSoundFile("And0g.wav");break;
									case(2): loadSoundFile("And0h.wav");break;
									case(3): loadSoundFile("And0i.wav");break;
									case(4): loadSoundFile("And0j.wav");break;
								}
								z = 1;
								break;
						}
					}
				}
				_voiceCount = 0;
			}
		} else {
			_voiceCount = 48;
		}
		_iconToggleCount = 0;
	}

	if (_variableArray[121] == 0) {
		_variableArray[121]++;
		_startSecondCount = _lastTickCount;
	}
	if (((_lastTickCount - _startSecondCount) / 1000) != _tSecondCount) {
		if (_startSecondCount != 0) {
			uint32 x = (_variableArray[123] * 65536) + _variableArray[122] + ((_lastTickCount - _startSecondCount) / 1000) - _tSecondCount;
			_variableArray[122] = (uint16)(x % 65536);
			_variableArray[123] = (uint16)(x / 65536);
			_tSecondCount = (_lastTickCount - _startSecondCount) / 1000;
		}
	}
}
#endif

} // End of namespace AGOS
