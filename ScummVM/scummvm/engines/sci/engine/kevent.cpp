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

#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/selector.h"
#include "sci/engine/kernel.h"
#include "sci/console.h"
#include "sci/debug.h"	// for g_debug_simulated_key
#include "sci/event.h"
#include "sci/graphics/coordadjuster.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/maciconbar.h"

namespace Sci {

reg_t kGetEvent(EngineState *s, int argc, reg_t *argv) {
	int mask = argv[0].toUint16();
	reg_t obj = argv[1];
	SciEvent curEvent;
	int modifier_mask = getSciVersion() <= SCI_VERSION_01 ? SCI_KEYMOD_ALL : SCI_KEYMOD_NO_FOOLOCK;
	SegManager *segMan = s->_segMan;
	Common::Point mousePos;

	// For Mac games with an icon bar, handle possible icon bar events first
	if (g_sci->hasMacIconBar()) {
		reg_t iconObj = g_sci->_gfxMacIconBar->handleEvents();
		if (!iconObj.isNull())
			invokeSelector(s, iconObj, SELECTOR(select), argc, argv, 0, NULL);
	}

	// If there's a simkey pending, and the game wants a keyboard event, use the
	// simkey instead of a normal event
	if (g_debug_simulated_key && (mask & SCI_EVENT_KEYBOARD)) {
		// In case we use a simulated event we query the current mouse position
		mousePos = g_sci->_gfxCursor->getPosition();
#ifdef ENABLE_SCI32
		if (getSciVersion() >= SCI_VERSION_2_1)
			g_sci->_gfxCoordAdjuster->fromDisplayToScript(mousePos.y, mousePos.x);
#endif
		// Limit the mouse cursor position, if necessary
		g_sci->_gfxCursor->refreshPosition();

		writeSelectorValue(segMan, obj, SELECTOR(type), SCI_EVENT_KEYBOARD); // Keyboard event
		writeSelectorValue(segMan, obj, SELECTOR(message), g_debug_simulated_key);
		writeSelectorValue(segMan, obj, SELECTOR(modifiers), SCI_KEYMOD_NUMLOCK); // Numlock on
		writeSelectorValue(segMan, obj, SELECTOR(x), mousePos.x);
		writeSelectorValue(segMan, obj, SELECTOR(y), mousePos.y);
		g_debug_simulated_key = 0;
		return make_reg(0, 1);
	}

	curEvent = g_sci->getEventManager()->getSciEvent(mask);

	// For a real event we use its associated mouse position
	mousePos = curEvent.mousePos;
#ifdef ENABLE_SCI32
	if (getSciVersion() >= SCI_VERSION_2_1)
		g_sci->_gfxCoordAdjuster->fromDisplayToScript(mousePos.y, mousePos.x);
#endif
	// Limit the mouse cursor position, if necessary
	g_sci->_gfxCursor->refreshPosition();

	if (g_sci->getVocabulary())
		g_sci->getVocabulary()->parser_event = NULL_REG; // Invalidate parser event

	if (s->_cursorWorkaroundActive) {
		// ffs: GfxCursor::setPosition()
		// we check, if actual cursor position is inside given rect
		//  if that's the case, we switch ourself off. Otherwise
		//  we simulate the original set position to the scripts
		if (s->_cursorWorkaroundRect.contains(mousePos.x, mousePos.y)) {
			s->_cursorWorkaroundActive = false;
		} else {
			mousePos.x = s->_cursorWorkaroundPoint.x;
			mousePos.y = s->_cursorWorkaroundPoint.y;
		}
	}

	writeSelectorValue(segMan, obj, SELECTOR(x), mousePos.x);
	writeSelectorValue(segMan, obj, SELECTOR(y), mousePos.y);

	//s->_gui->moveCursor(s->gfx_state->pointer_pos.x, s->gfx_state->pointer_pos.y);

	switch (curEvent.type) {
	case SCI_EVENT_QUIT:
		s->abortScriptProcessing = kAbortQuitGame; // Terminate VM
		g_sci->_debugState.seeking = kDebugSeekNothing;
		g_sci->_debugState.runningStep = 0;
		break;

	case SCI_EVENT_KEYBOARD:
		writeSelectorValue(segMan, obj, SELECTOR(type), SCI_EVENT_KEYBOARD); // Keyboard event
		s->r_acc = make_reg(0, 1);

		writeSelectorValue(segMan, obj, SELECTOR(message), curEvent.character);
		// We only care about the translated character
		writeSelectorValue(segMan, obj, SELECTOR(modifiers), curEvent.modifiers & modifier_mask);
		break;

	case SCI_EVENT_MOUSE_RELEASE:
	case SCI_EVENT_MOUSE_PRESS:

		// track left buttton clicks, if requested
		if (curEvent.type == SCI_EVENT_MOUSE_PRESS && curEvent.data == 1 && g_debug_track_mouse_clicks) {
			g_sci->getSciDebugger()->DebugPrintf("Mouse clicked at %d, %d\n",
						mousePos.x, mousePos.y);
		}

		if (mask & curEvent.type) {
			int extra_bits = 0;

			switch (curEvent.data) {
			case 2:
				extra_bits = SCI_KEYMOD_LSHIFT | SCI_KEYMOD_RSHIFT;
				break;
			case 3:
				extra_bits = SCI_KEYMOD_CTRL;
			default:
				break;
			}

			writeSelectorValue(segMan, obj, SELECTOR(type), curEvent.type);
			writeSelectorValue(segMan, obj, SELECTOR(message), 0);
			writeSelectorValue(segMan, obj, SELECTOR(modifiers), (curEvent.modifiers | extra_bits) & modifier_mask);
			s->r_acc = make_reg(0, 1);
		}
		break;

	default:
		// Return a null event
		writeSelectorValue(segMan, obj, SELECTOR(type), SCI_EVENT_NONE);
		writeSelectorValue(segMan, obj, SELECTOR(message), 0);
		writeSelectorValue(segMan, obj, SELECTOR(modifiers), curEvent.modifiers & modifier_mask);
		s->r_acc = NULL_REG;
	}

	if ((s->r_acc.getOffset()) && (g_sci->_debugState.stopOnEvent)) {
		g_sci->_debugState.stopOnEvent = false;

		// A SCI event occurred, and we have been asked to stop, so open the debug console
		Console *con = g_sci->getSciDebugger();
		con->DebugPrintf("SCI event occurred: ");
		switch (curEvent.type) {
		case SCI_EVENT_QUIT:
			con->DebugPrintf("quit event\n");
			break;
		case SCI_EVENT_KEYBOARD:
			con->DebugPrintf("keyboard event\n");
			break;
		case SCI_EVENT_MOUSE_RELEASE:
		case SCI_EVENT_MOUSE_PRESS:
			con->DebugPrintf("mouse click event\n");
			break;
		default:
			con->DebugPrintf("unknown or no event (event type %d)\n", curEvent.type);
		}

		con->attach();
		con->onFrame();
	}

	if (g_sci->_features->detectDoSoundType() <= SCI_VERSION_0_LATE) {
		// If we're running a sound-SCI0 game, update the sound cues, to
		// compensate for the fact that sound-SCI0 does not poll to update
		// the sound cues itself, like sound-SCI1 and later do with
		// cmdUpdateSoundCues. kGetEvent is called quite often, so emulate
		// the sound-SCI1 behavior of cmdUpdateSoundCues with this call
		g_sci->_soundCmd->updateSci0Cues();
	}

	// Wait a bit here, so that the CPU isn't maxed out when the game
	// is waiting for user input (e.g. when showing text boxes) - bug
	// #3037874. Make sure that we're not delaying while the game is
	// benchmarking, as that will affect the final benchmarked result -
	// check bugs #3058865 and #3127824
	if (s->_gameIsBenchmarking) {
		// Game is benchmarking, don't add a delay
	} else {
		g_system->delayMillis(10);
	}

	return s->r_acc;
}

struct KeyDirMapping {
	uint16 key;
	uint16 direction;
};

const KeyDirMapping keyToDirMap[] = {
	{ SCI_KEY_HOME,   8 }, { SCI_KEY_UP,     1 }, { SCI_KEY_PGUP,   2 },
	{ SCI_KEY_LEFT,   7 }, { SCI_KEY_CENTER, 0 }, { SCI_KEY_RIGHT,  3 },
	{ SCI_KEY_END,    6 }, { SCI_KEY_DOWN,   5 }, { SCI_KEY_PGDOWN, 4 },
};

reg_t kMapKeyToDir(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	SegManager *segMan = s->_segMan;

	if (readSelectorValue(segMan, obj, SELECTOR(type)) == SCI_EVENT_KEYBOARD) { // Keyboard
		uint16 message = readSelectorValue(segMan, obj, SELECTOR(message));
		uint16 eventType = SCI_EVENT_DIRECTION;
		// Check if the game is using cursor views. These games allowed control
		// of the mouse cursor via the keyboard controls (the so called
		// "PseudoMouse" functionality in script 933).
		if (g_sci->_features->detectSetCursorType() == SCI_VERSION_1_1)
			eventType |= SCI_EVENT_KEYBOARD;

		for (int i = 0; i < 9; i++) {
			if (keyToDirMap[i].key == message) {
				writeSelectorValue(segMan, obj, SELECTOR(type), eventType);
				writeSelectorValue(segMan, obj, SELECTOR(message), keyToDirMap[i].direction);
				return TRUE_REG;	// direction mapped
			}
		}

		return NULL_REG;	// unknown direction
	}

	return s->r_acc;	// no keyboard event to map, leave accumulator unchanged
}

reg_t kGlobalToLocal(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	reg_t planeObject = argc > 1 ? argv[1] : NULL_REG; // SCI32
	SegManager *segMan = s->_segMan;

	if (obj.getSegment()) {
		int16 x = readSelectorValue(segMan, obj, SELECTOR(x));
		int16 y = readSelectorValue(segMan, obj, SELECTOR(y));

		g_sci->_gfxCoordAdjuster->kernelGlobalToLocal(x, y, planeObject);

		writeSelectorValue(segMan, obj, SELECTOR(x), x);
		writeSelectorValue(segMan, obj, SELECTOR(y), y);
	}

	return s->r_acc;

}

reg_t kLocalToGlobal(EngineState *s, int argc, reg_t *argv) {
	reg_t obj = argv[0];
	reg_t planeObject = argc > 1 ? argv[1] : NULL_REG; // SCI32
	SegManager *segMan = s->_segMan;

	if (obj.getSegment()) {
		int16 x = readSelectorValue(segMan, obj, SELECTOR(x));
		int16 y = readSelectorValue(segMan, obj, SELECTOR(y));

		g_sci->_gfxCoordAdjuster->kernelLocalToGlobal(x, y, planeObject);

		writeSelectorValue(segMan, obj, SELECTOR(x), x);
		writeSelectorValue(segMan, obj, SELECTOR(y), y);
	}

	return s->r_acc;
}

reg_t kJoystick(EngineState *s, int argc, reg_t *argv) {
	// Subfunction 12 sets/gets joystick repeat rate
	debug(5, "Unimplemented syscall 'Joystick()'");
	return NULL_REG;
}

} // End of namespace Sci
