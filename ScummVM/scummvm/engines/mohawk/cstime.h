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

#ifndef MOHAWK_CSTIME_H
#define MOHAWK_CSTIME_H

#include "mohawk/mohawk.h"
#include "mohawk/console.h"
#include "mohawk/cstime_graphics.h"

#include "common/random.h"
#include "common/list.h"

namespace Mohawk {

class CSTimeCase;
class CSTimeInterface;
class CSTimeView;

enum {
	kCSTimeEventNothing = 0xffff,
	kCSTimeEventCondition = 1,
	kCSTimeEventCharPlayNIS = 2,
	kCSTimeEventStartConversation = 3,
	kCSTimeEventNewScene = 4,
	kCSTimeEventCharStartFlapping = 5,
	kCSTimeEventSetCaseVariable = 6,
	kCSTimeEventSetupAmbientAnims = 7,
	kCSTimeEventUnused8 = 8,
	kCSTimeEventDropItemInInventory = 9,
	kCSTimeEventRemoveItemFromInventory = 10,
	kCSTimeEventAddNotePiece = 11,
	kCSTimeEventDisableHotspot = 12,
	kCSTimeEventDisableFeature = 13,
	kCSTimeEventAddFeature = 14,
	kCSTimeEventStartMusic = 15,
	kCSTimeEventStopMusic = 16,
	kCSTimeEventEnableHotspot = 17,
	kCSTimeEventSetAsked = 18,
	kCSTimeEventStartHelp = 19,
	kCSTimeEventPlaySound = 20,
	kCSTimeEventUnused21 = 21,
	kCSTimeEventShowBigNote = 22,
	kCSTimeEventActivateCuffs = 23,
	kCSTimeEventHelperSetupRestPos = 24,
	kCSTimeEventUnknown25 = 25,
	kCSTimeEventUnknown26 = 26,
	kCSTimeEventRemoveChar = 27,
	kCSTimeEventUnknown28 = 28,
	kCSTimeEventUnknown29 = 29,
	kCSTimeEventUnknown30 = 30,
	kCSTimeEventUnknown31 = 31,
	kCSTimeEventCharSomeNIS32 = 32,
	kCSTimeEventCharResetNIS = 33,
	kCSTimeEventUnknown34 = 34,
	kCSTimeEventCharPauseAmbients = 35,
	kCSTimeEventCharUnauseAmbients = 36,
	kCSTimeEventCharDisableAmbients = 37,
	kCSTimeEventStopAmbientAnims = 38,
	kCSTimeEventUnknown39 = 39,
	kCSTimeEventWait = 40,
	kCSTimeEventSpeech = 41,
	kCSTimeEventCharSetState = 42,
	kCSTimeEventUnknown43 = 43,
	kCSTimeEventCharSetupRestPos = 44,
	kCSTimeEventCharStopAmbients = 45,
	kCSTimeEventCharRestartAmbients = 46,
	kCSTimeEventStopEnvironmentSound = 47,
	kCSTimeEventWaitForClick = 48,
	kCSTimeEventSetMusic = 49,
	kCSTimeEventStartEnvironmentSound = 50,
	kCSTimeEventPreloadSound = 51,
	kCSTimeEventPlayPreloadedSound = 52,
	kCSTimeEventSetInsertBefore = 53,
	kCSTimeEventSetEnvironmentSound = 54,
	kCSTimeEventCharSomeNIS55 = 55,
	kCSTimeEventUnknown56 = 56,
	kCSTimeEventUpdateBubble = 57,
	kCSTimeEventCharSurfAndFlap = 58,
	kCSTimeEventInitScene = 59,
	kCSTimeEventFadeDown = 60,
	kCSTimeEventEndOfCase = 61,
	kCSTimeEventCharPlaySimultaneousAnim = 62,
	kCSTimeEventUnused63 = 63,
	kCSTimeEventUnknown64 = 64,
	kCSTimeEventPrepareSave = 65,
	kCSTimeEventSave = 66,
	kCSTimeEventQuit = 67,
	kCSTimeEventPlayMovie = 68,
	kCSTimeEventUnknown69 = 69, // queues Unknown48
	kCSTimeEventUnknown70 = 70 // conv/QaR cleanup
};

struct CSTimeEvent {
	CSTimeEvent() { }
	CSTimeEvent(uint16 t, uint16 p1, uint16 p2) : type(t), param1(p1), param2(p2) { }

	uint16 type;
	uint16 param1;
	uint16 param2;
};

enum CSTimeState {
	kCSTStateStartup,
	kCSTStateNewCase,
	kCSTStateNewScene,
	kCSTStateNormal
};

class MohawkEngine_CSTime : public MohawkEngine {
protected:
	Common::Error run();

public:
	MohawkEngine_CSTime(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_CSTime();

	Common::RandomSource *_rnd;

	CSTimeGraphics *_gfx;
	bool _needsUpdate;

	GUI::Debugger *getDebugger() { return _console; }
	CSTimeView *getView() { return _view; }
	CSTimeCase *getCase() { return _case; }
	CSTimeInterface *getInterface() { return _interface; }

	void loadResourceFile(Common::String name);

	void addEvent(const CSTimeEvent &event);
	void addEventList(const Common::Array<CSTimeEvent> &list);
	void insertEventAtFront(const CSTimeEvent &event);
	uint16 getCurrentEventType();
	void eventIdle();
	void resetTimeout();
	void mouseClicked();
	bool NISIsRunning();

	uint16 _haveInvItem[19];
	uint16 _caseVariable[20];

private:
	CSTimeCase *_case;
	CSTimeConsole *_console;
	CSTimeInterface *_interface;
	CSTimeView *_view;

	CSTimeState _state;

	void initCase();
	void nextScene();
	void update();

	uint16 _nextSceneId;

	bool _processingEvent;
	bool _processingNIS55;
	bool _NISRunning;
	uint32 _lastTimeout;
	void reset();

	Common::List<CSTimeEvent> _events;
	void triggerEvent(CSTimeEvent &event);
};

} // End of namespace Mohawk

#endif
