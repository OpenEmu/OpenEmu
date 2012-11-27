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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#ifndef TSAGE_EVENTS_H
#define TSAGE_EVENTS_H

#include "common/events.h"
#include "common/array.h"
#include "common/str.h"
#include "graphics/surface.h"
#include "tsage/saveload.h"

namespace TsAGE {

enum EventType {EVENT_NONE = 0, EVENT_BUTTON_DOWN = 1, EVENT_BUTTON_UP = 2, EVENT_KEYPRESS = 4,
	EVENT_MOUSE_MOVE = 8, EVENT_UNK27 = 27};

enum ButtonShiftFlags {BTNSHIFT_LEFT = 0, BTNSHIFT_RIGHT = 3, BTNSHIFT_MIDDLE = 4};

// Intrinisc game delay between execution frames
#define GAME_FRAME_RATE 50
#define GAME_FRAME_TIME (1000 / GAME_FRAME_RATE)

#define GAME_SCRIPT_RATE 80
#define GAME_SCRIPT_TIME (1000 / GAME_SCRIPT_RATE)

class GfxManager;

class Event {
public:
	EventType eventType;
	Common::Point mousePos;
	int btnState;
	Common::KeyState kbd;
	int ctr;
	GfxManager *gfxMan;
	bool handled;
};

enum CursorType {
	// Ringworld objects
	OBJECT_STUNNER = 0, OBJECT_SCANNER = 1, OBJECT_STASIS_BOX = 2,
	OBJECT_INFODISK = 3, OBJECT_STASIS_NEGATOR = 4, OBJECT_KEY_DEVICE = 5, OBJECT_MEDKIT = 6,
	OBJECT_LADDER = 7, OBJECT_ROPE = 8, OBJECT_KEY = 9, OBJECT_TRANSLATOR = 10, OBJECT_ALE = 11,
	OBJECT_PAPER = 12, OBJECT_WALDOS = 13, OBJECT_STASIS_BOX2 = 14, OBJECT_RING = 15,
	OBJECT_CLOAK = 16, OBJECT_TUNIC = 17, OBJECT_CANDLE = 18, OBJECT_STRAW = 19, OBJECT_SCIMITAR = 20,
	OBJECT_SWORD = 21, OBJECT_HELMET = 22, OBJECT_ITEMS = 23, OBJECT_CONCENTRATOR = 24,
	OBJECT_NULLIFIER = 25, OBJECT_PEG = 26, OBJECT_VIAL = 27, OBJECT_JACKET = 28,
	OBJECT_TUNIC2 = 29, OBJECT_BONE = 30, OBJECT_EMPTY_JAR = 31, OBJECT_JAR = 32,

	// Blue Force objects
	INV_NONE = 0, INV_COLT45 = 1, INV_AMMO_CLIP = 2, INV_SPARE_CLIP = 3, INV_HANDCUFFS = 4,
	INV_GREENS_GUN = 5,	INV_TICKET_BOOK = 6, INV_MIRANDA_CARD = 7, INV_FOREST_RAP = 8,
	INV_GREEN_ID = 9, INV_BASEBALL_CARD = 10, INV_BOOKING_GREEN = 11, INV_FLARE = 12,
	INV_COBB_RAP = 13, INV_22_BULLET = 14, INV_AUTO_RIFLE = 15, INV_WIG = 16, INV_FRANKIE_ID = 17,
	INV_TYRONE_ID = 18, INV_22_SNUB = 19, INV_BOOKING_FRANKIE = 21, INV_BOOKING_GANG = 22,
	INV_FBI_TELETYPE = 23, INV_DA_NOTE = 24, INV_PRINT_OUT = 25, INV_WAREHOUSE_KEYS = 26,
	INV_CENTER_PUNCH = 27, INV_TRANQ_GUN = 28, INV_HOOK = 29, INV_RAGS = 30, INV_JAR = 31,
	INV_SCREWDRIVER = 32, INV_D_FLOPPY = 33, INV_BLANK_DISK = 34, INV_STICK = 35,
	INV_CRATE1 = 36, INV_CRATE2 = 37, INV_SHOEBOX = 38, INV_BADGE = 39, INV_RENTAL_COUPON = 41,
	INV_NICKEL = 42, INV_LYLE_CARD = 43, INV_CARTER_NOTE = 44, INV_MUG_SHOT = 45,
	INV_CLIPPING = 46, INV_MICROFILM  = 47, INV_WAVE_KEYS = 48,	INV_RENTAL_KEYS = 49,
	INV_NAPKIN = 50, INV_DMV_PRINTOUT = 51, INV_FISHING_NET = 52, INV_ID = 53,
	INV_9MM_BULLETS = 54, INV_SCHEDULE = 55, INV_GRENADES = 56, INV_YELLOW_CORD = 57,
	INV_HALF_YELLOW_CORD = 58, INV_BLACK_CORD = 59, INV_HALF_BLACK_CORD = 61, INV_WARRANT = 62,
	INV_JACKET = 63, INV_GREENS_KNIFE = 64, INV_DOG_WHISTLE = 65, INV_AMMO_BELT = 66,
	INV_CARAVAN_KEY = 67, BF_LAST_INVENT = 68,

	// Ringworld 2 objects
	R2_OPTO_DISK = 1, R2_READER = 2, R2_NEGATOR_GUN = 3, R2_STEPPING_DISKS = 4,
	R2_ATTRACTOR_UNIT = 5, R2_SENSOR_PROBE = 6, R2_SONIC_STUNNER = 7,
	R2_CABLE_HARNESS = 8, R2_COM_SCANNER = 9, R2_SPENT_POWER_CAPSULE = 10,
	R2_CHARGED_POWER_CAPSULE = 11, R2_AEROSOL = 12, R2_REMOTE_CONTROL = 13,
	R2_OPTICAL_FIBRE = 14, R2_CLAMP = 15, R2_ATTRACTOR_CABLE_HARNESS = 16,
	R2_FUEL_CELL = 17, R2_GYROSCOPE = 18, R2_AIRBAG = 19, R2_REBREATHER_TANK = 20,
	R2_RESERVE_REBREATHER_TANK = 21, R2_GUIDANCE_MODULE = 22, R2_THRUSTER_VALVE = 23,
	R2_BALLOON_BACKPACK = 24, R2_RADAR_MECHANISM = 25, R2_JOYSTICK = 26,
	R2_IGNITOR = 27, R2_DIAGNOSTICS_DISPLAY = 28, R2_GLASS_DOME = 29, R2_WICK_LAMP = 30,
	R2_SCRITH_KEY = 31, R2_TANNER_MASK = 32, R2_PURE_GRAIN_ALCOHOL = 33, R2_SAPPHIRE_BLUE = 34,
	R2_ANCIENT_SCROLLS = 35, R2_FLUTE = 36, R2_GUNPOWDER = 37, R2_NONAME = 38,
	R2_COM_SCANNER_2 = 39, R2_SUPERCONDUCTOR_WIRE = 40, R2_PILLOW = 41, R2_FOOD_TRAY = 42,
	R2_LASER_HACKSAW = 43, R2_PHOTON_STUNNER = 44, R2_BATTERY = 45, R2_SOAKED_FACEMASK = 46,
	R2_LIGHT_BULB = 47, R2_ALCOHOL_LAMP = 48, R2_ALCOHOL_LAMP_2 = 49, R2_ALCOHOL_LAMP_3 = 50,
	R2_BROKEN_DISPLAY = 51, R2_TOOLBOX = 52, R2_LAST_INVENT = 53,

	// Ringworld 2 cursors
	R2CURSORS_START = 0x8000, EXITCURSOR_N  = 0x8007,  EXITCURSOR_S  = 0x8008, EXITCURSOR_W = 0x8009,
	EXITCURSOR_E = 0x800A, EXITCURSOR_LEFT_HAND = 0x800B, CURSOR_INVALID = 0x800C,
	EXITCURSOR_NE = 0x800D, EXITCURSOR_SE = 0x800E, EXITCURSOR_SW = 0x800F, EXITCURSOR_NW = 0x8010,
	SHADECURSOR_UP = 0x8011, SHADECURSOR_DOWN = 0x8012, SHADECURSOR_HAND = 0x8013,
	R2_CURSOR_20 = 0x8014, R2_CURSOR_21 = 0x8015, R2_CURSOR_22 = 0x8016, R2_CURSOR_23 = 0x8017,

	// Cursors
	CURSOR_WALK = 0x100, CURSOR_LOOK = 0x200, CURSOR_700 = 700, CURSOR_USE = 0x400, CURSOR_TALK = 0x800,
	CURSOR_1000 = 0x1000, CURSOR_PRINTER = 0x4000, CURSOR_EXIT = 0x7004, CURSOR_9999 = 9999,
	CURSOR_NONE = -1, CURSOR_CROSSHAIRS = -2, CURSOR_ARROW = -3
};

class GfxSurface;

class EventsClass : public SaveListener {
private:
	Common::Event _event;
	uint32 _frameNumber;
	uint32 _prevDelayFrame;
	uint32 _priorFrameTime;
public:
	EventsClass();

	Common::Point _mousePos;
	CursorType _currentCursor;
	CursorType _lastCursor;

	void setCursor(CursorType cursorType);
	void pushCursor(CursorType cursorType);
	void popCursor();
	void setCursor(Graphics::Surface &cursor, int transColor, const Common::Point &hotspot, CursorType cursorId);
	void setCursor(GfxSurface &cursor);
	void setCursorFromFlag();
	CursorType getCursor() const { return _currentCursor; }
	void showCursor();
	CursorType hideCursor();
	bool isCursorVisible() const;

	bool pollEvent();
	void waitForPress(int eventMask = EVENT_BUTTON_DOWN | EVENT_KEYPRESS);

	bool getEvent(Event &evt, int eventMask = ~EVENT_MOUSE_MOVE);
	Common::Event event() { return _event; }
	Common::EventType type() { return _event.type; }
	uint32 getFrameNumber() const { return _frameNumber; }
	void delay(int numFrames);
	bool isInventoryIcon() const { return (_currentCursor >= 0) && (_currentCursor < 256); }
	void proc1() { warning("TODO: EventsClass::proc1"); }

	virtual void listenerSynchronize(Serializer &s);
	static void loadNotifierProc(bool postFlag);
};

} // End of namespace TsAGE

#endif
