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

#include "tsage/debugger.h"
#include "tsage/globals.h"
#include "tsage/graphics.h"
#include "tsage/ringworld/ringworld_logic.h"
#include "tsage/blue_force/blueforce_logic.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

Debugger::Debugger() : GUI::Debugger() {
	DCmd_Register("continue",		WRAP_METHOD(Debugger, Cmd_Exit));
	DCmd_Register("scene",			WRAP_METHOD(Debugger, Cmd_Scene));
	DCmd_Register("walk_regions",	WRAP_METHOD(Debugger, Cmd_WalkRegions));
	DCmd_Register("priority_regions",	WRAP_METHOD(Debugger, Cmd_PriorityRegions));
	DCmd_Register("scene_regions",	WRAP_METHOD(Debugger, Cmd_SceneRegions));
	DCmd_Register("setflag",		WRAP_METHOD(Debugger, Cmd_SetFlag));
	DCmd_Register("getflag",		WRAP_METHOD(Debugger, Cmd_GetFlag));
	DCmd_Register("clearflag",		WRAP_METHOD(Debugger, Cmd_ClearFlag));
	DCmd_Register("listobjects",	WRAP_METHOD(Debugger, Cmd_ListObjects));
	DCmd_Register("moveobject",		WRAP_METHOD(Debugger, Cmd_MoveObject));
	DCmd_Register("hotspots",		WRAP_METHOD(Debugger, Cmd_Hotspots));
	DCmd_Register("sound",			WRAP_METHOD(Debugger, Cmd_Sound));
}

static int strToInt(const char *s) {
	if (!*s)
		// No string at all
		return 0;
	else if (toupper(s[strlen(s) - 1]) != 'H')
		// Standard decimal string
		return atoi(s);

	// Hexadecimal string
	uint tmp = 0;
	int read = sscanf(s, "%xh", &tmp);
	if (read < 1)
		error("strToInt failed on string \"%s\"", s);
	return (int)tmp;
}

/**
 * This command loads up the specified new scene number
 */
bool Debugger::Cmd_Scene(int argc, const char **argv) {
	if (argc < 2) {
		DebugPrintf("Usage: %s <scene number> [prior scene #]\n", argv[0]);
		return true;
	}

	if (argc == 3)
		g_globals->_sceneManager._sceneNumber = strToInt(argv[2]);

	g_globals->_sceneManager.changeScene(strToInt(argv[1]));
	return false;
}

/**
 * This command draws the walk regions onto the screen
 */
bool Debugger::Cmd_WalkRegions(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	// Color index to use for the first walk region
	int color = 16;

	// Lock the background surface for access
	Graphics::Surface destSurface = g_globals->_sceneManager._scene->_backSurface.lockSurface();

	// Loop through drawing each walk region in a different color to the background surface
	Common::String regionsDesc;

	for (uint regionIndex = 0; regionIndex < g_globals->_walkRegions._regionList.size(); ++regionIndex, ++color) {
		WalkRegion &wr = g_globals->_walkRegions._regionList[regionIndex];

		// Skip the region if it's in the list of explicitly disabled regions
		if (contains(g_globals->_walkRegions._disabledRegions, (int)regionIndex + 1))
			continue;

		for (int yp = wr._bounds.top; yp < wr._bounds.bottom; ++yp) {
			LineSliceSet sliceSet = wr.getLineSlices(yp);

			for (uint idx = 0; idx < sliceSet.items.size(); ++idx)
				destSurface.hLine(sliceSet.items[idx].xs - g_globals->_sceneOffset.x, yp,
				sliceSet.items[idx].xe - g_globals->_sceneOffset.x, color);
		}

		regionsDesc += Common::String::format("Region #%d d bounds=%d,%d,%d,%d\n",
					regionIndex, wr._bounds.left, wr._bounds.top, wr._bounds.right, wr._bounds.bottom);
	}

	// Release the surface
	g_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	g_globals->_paneRefreshFlag[0] = 2;

	DebugPrintf("Total regions = %d\n", g_globals->_walkRegions._regionList.size());
	DebugPrintf("%s\n", regionsDesc.c_str());

	return false;
}

/*
 * This command draws the priority regions onto the screen
 */
bool Debugger::Cmd_PriorityRegions(int argc, const char **argv) {
	int regionNum = 0;

	// Check for an optional specific region to display
	if (argc == 2)
		regionNum = strToInt(argv[1]);

	// Color index to use for the first priority region
	int color = 16;
	int count = 0;

	// Lock the background surface for access
	Graphics::Surface destSurface = g_globals->_sceneManager._scene->_backSurface.lockSurface();

	Common::List<Region>::iterator i = g_globals->_sceneManager._scene->_priorities.begin();
	Common::String regionsDesc;

	for (; i != g_globals->_sceneManager._scene->_priorities.end(); ++i, ++color, ++count) {
		Region &r = *i;

		if ((regionNum == 0) || (regionNum == (count + 1))) {
			for (int y = 0; y < destSurface.h; ++y) {
				byte *destP = (byte *)destSurface.getBasePtr(0, y);

				for (int x = 0; x < destSurface.w; ++x) {
					if (r.contains(Common::Point(g_globals->_sceneManager._scene->_sceneBounds.left + x,
							g_globals->_sceneManager._scene->_sceneBounds.top + y)))
						*destP = color;
					++destP;
				}
			}
		}

		regionsDesc += Common::String::format("Region Priority = %d bounds=%d,%d,%d,%d\n",
			r._regionId, r._bounds.left, r._bounds.top, r._bounds.right, r._bounds.bottom);
	}

	// Release the surface
	g_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	g_globals->_paneRefreshFlag[0] = 2;

	DebugPrintf("Total regions = %d\n", count);
	DebugPrintf("%s", regionsDesc.c_str());

	return true;
}

/*
 * This command draws the scene regions onto the screen. These are the regions
 * used by hotspots that have non-rectangular areas.
 */
bool Debugger::Cmd_SceneRegions(int argc, const char **argv) {
	int regionNum = 0;

	// Check for an optional specific region to display
	if (argc == 2)
		regionNum = strToInt(argv[1]);

	// Color index to use for the first priority region
	int color = 16;
	int count = 0;

	// Lock the background surface for access
	Graphics::Surface destSurface = g_globals->_sceneManager._scene->_backSurface.lockSurface();

	Common::List<Region>::iterator i = g_globals->_sceneRegions.begin();
	Common::String regionsDesc;

	for (; i != g_globals->_sceneRegions.end(); ++i, ++color, ++count) {
		Region &r = *i;

		if ((regionNum == 0) || (regionNum == (count + 1))) {
			for (int y = 0; y < destSurface.h; ++y) {
				byte *destP = (byte *)destSurface.getBasePtr(0, y);

				for (int x = 0; x < destSurface.w; ++x) {
					if (r.contains(Common::Point(g_globals->_sceneManager._scene->_sceneBounds.left + x,
							g_globals->_sceneManager._scene->_sceneBounds.top + y)))
						*destP = color;
					++destP;
				}
			}
		}

		regionsDesc += Common::String::format("Region id = %d bounds=%d,%d,%d,%d\n",
			r._regionId, r._bounds.left, r._bounds.top, r._bounds.right, r._bounds.bottom);
	}

	// Release the surface
	g_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	g_globals->_paneRefreshFlag[0] = 2;

	DebugPrintf("Total regions = %d\n", count);
	DebugPrintf("%s", regionsDesc.c_str());

	return true;
}

/*
 * This command sets a flag
 */
bool Debugger::Cmd_SetFlag(int argc, const char **argv) {
	// Check for a flag to set
	if (argc != 2) {
		DebugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	g_globals->setFlag(flagNum);
	return true;
}

/*
 * This command gets the value of a flag
 */
bool Debugger::Cmd_GetFlag(int argc, const char **argv) {
	// Check for an flag to display
	if (argc != 2) {
		DebugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	DebugPrintf("Value: %d\n", g_globals->getFlag(flagNum));
	return true;
}

/*
 * This command clears a flag
 */
bool Debugger::Cmd_ClearFlag(int argc, const char **argv) {
	// Check for a flag to clear
	if (argc != 2) {
		DebugPrintf("Usage: %s <flag number>\n", argv[0]);
		return true;
	}

	int flagNum = strToInt(argv[1]);
	g_globals->clearFlag(flagNum);
	return true;
}

/**
 * Show any active hotspot areas in the scene
 */
bool Debugger::Cmd_Hotspots(int argc, const char **argv) {
	int colIndex = 16;
	const Rect &sceneBounds = g_globals->_sceneManager._scene->_sceneBounds;

	// Lock the background surface for access
	Graphics::Surface destSurface = g_globals->_sceneManager._scene->_backSurface.lockSurface();

	// Iterate through the scene items
	SynchronizedList<SceneItem *>::iterator i;
	for (i = g_globals->_sceneItems.reverse_begin(); i != g_globals->_sceneItems.end(); --i, ++colIndex) {
		SceneItem *o = *i;

		// Draw the contents of the hotspot area
		if (o->_sceneRegionId == 0) {
			// Scene item doesn't use a region, so fill in the entire area
			if ((o->_bounds.right > o->_bounds.left) && (o->_bounds.bottom > o->_bounds.top))
				destSurface.fillRect(Rect(o->_bounds.left - sceneBounds.left, o->_bounds.top - sceneBounds.top,
					o->_bounds.right - sceneBounds.left - 1, o->_bounds.bottom - sceneBounds.top - 1), colIndex);
		} else {
			// Scene uses a region, so get it and use it to fill out only the correct parts
			SceneRegions::iterator ri = g_globals->_sceneRegions.begin();
			while ((ri != g_globals->_sceneRegions.end()) && ((*ri)._regionId != o->_sceneRegionId))
				++ri;

			if (ri != g_globals->_sceneRegions.end()) {
				// Fill out the areas defined by the region
				Region &r = *ri;

				for (int y = r._bounds.top; y < r._bounds.bottom; ++y) {
					LineSliceSet set = r.getLineSlices(y);

					for (uint p = 0; p < set.items.size(); ++p)
						destSurface.hLine(set.items[p].xs - sceneBounds.left, y - sceneBounds.top,
							set.items[p].xe - sceneBounds.left - 1, colIndex);
				}
			}
		}
	}

	// Release the surface
	g_globals->_sceneManager._scene->_backSurface.unlockSurface();

	// Mark the scene as requiring a full redraw
	g_globals->_paneRefreshFlag[0] = 2;

	return false;
}

/**
 * Play the specified sound
 */
bool Debugger::Cmd_Sound(int argc, const char **argv) {
	if (argc != 2) {
		DebugPrintf("Usage: %s <sound number>\n", argv[0]);
		return true;
	}

	int soundNum = strToInt(argv[1]);
	g_globals->_soundHandler.play(soundNum);
	return false;
}

/*
 * This command lists the objects available, and their ID
 */
bool DemoDebugger::Cmd_ListObjects(int argc, const char **argv) {
	DebugPrintf("Not available in Demo\n");
	return true;
}

bool DemoDebugger::Cmd_MoveObject(int argc, const char **argv) {
	DebugPrintf("Not available in Demo\n");
	return true;
}

/*
 * This command lists the objects available, and their ID
 */
bool RingworldDebugger::Cmd_ListObjects(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	DebugPrintf("Available objects for this game are:\n");
	DebugPrintf("0 - Stunner\n");
	DebugPrintf("1 - Scanner\n");
	DebugPrintf("2 - Stasis Box\n");
	DebugPrintf("3 - Info Disk\n");
	DebugPrintf("4 - Stasis Negator\n");
	DebugPrintf("5 - Key Device\n");
	DebugPrintf("6 - Medkit\n");
	DebugPrintf("7 - Ladder\n");
	DebugPrintf("8 - Rope\n");
	DebugPrintf("9 - Key\n");
	DebugPrintf("10 - Translator\n");
	DebugPrintf("11 - Ale\n");
	DebugPrintf("12 - Paper\n");
	DebugPrintf("13 - Waldos\n");
	DebugPrintf("14 - Stasis Box 2\n");
	DebugPrintf("15 - Ring\n");
	DebugPrintf("16 - Cloak\n");
	DebugPrintf("17 - Tunic\n");
	DebugPrintf("18 - Candle\n");
	DebugPrintf("19 - Straw\n");
	DebugPrintf("20 - Scimitar\n");
	DebugPrintf("21 - Sword\n");
	DebugPrintf("22 - Helmet\n");
	DebugPrintf("23 - Items\n");
	DebugPrintf("24 - Concentrator\n");
	DebugPrintf("25 - Nullifier\n");
	DebugPrintf("26 - Peg\n");
	DebugPrintf("27 - Vial\n");
	DebugPrintf("28 - Jacket\n");
	DebugPrintf("29 - Tunic 2\n");
	DebugPrintf("30 - Bone\n");
	DebugPrintf("31 - Empty Jar\n");
	DebugPrintf("32 - Jar\n");
	return true;
}

/*
 * This command gets an item, or move it to a room
 */
bool RingworldDebugger::Cmd_MoveObject(int argc, const char **argv) {
	// Check for a flag to clear
	if ((argc < 2) || (argc > 3)){
		DebugPrintf("Usage: %s <object number> [<scene number>]\n", argv[0]);
		DebugPrintf("If no scene is specified, the object will be added to inventory\n");
		return true;
	}

	int objNum = strToInt(argv[1]);
	int sceneNum = 1;
	if (argc == 3)
		sceneNum = strToInt(argv[2]);

	switch (objNum) {
	case OBJECT_STUNNER:
		RING_INVENTORY._stunner._sceneNumber = sceneNum;
		break;
	case OBJECT_SCANNER:
		RING_INVENTORY._scanner._sceneNumber = sceneNum;
		break;
	case OBJECT_STASIS_BOX:
		RING_INVENTORY._stasisBox._sceneNumber = sceneNum;
		break;
	case OBJECT_INFODISK:
		RING_INVENTORY._infoDisk._sceneNumber = sceneNum;
		break;
	case OBJECT_STASIS_NEGATOR:
		RING_INVENTORY._stasisNegator._sceneNumber = sceneNum;
		break;
	case OBJECT_KEY_DEVICE:
		RING_INVENTORY._keyDevice._sceneNumber = sceneNum;
		break;
	case OBJECT_MEDKIT:
		RING_INVENTORY._medkit._sceneNumber = sceneNum;
		break;
	case OBJECT_LADDER:
		RING_INVENTORY._ladder._sceneNumber = sceneNum;
		break;
	case OBJECT_ROPE:
		RING_INVENTORY._rope._sceneNumber = sceneNum;
		break;
	case OBJECT_KEY:
		RING_INVENTORY._key._sceneNumber = sceneNum;
		break;
	case OBJECT_TRANSLATOR:
		RING_INVENTORY._translator._sceneNumber = sceneNum;
		break;
	case OBJECT_ALE:
		RING_INVENTORY._ale._sceneNumber = sceneNum;
		break;
	case OBJECT_PAPER:
		RING_INVENTORY._paper._sceneNumber = sceneNum;
		break;
	case OBJECT_WALDOS:
		RING_INVENTORY._waldos._sceneNumber = sceneNum;
		break;
	case OBJECT_STASIS_BOX2:
		RING_INVENTORY._stasisBox2._sceneNumber = sceneNum;
		break;
	case OBJECT_RING:
		RING_INVENTORY._ring._sceneNumber = sceneNum;
		break;
	case OBJECT_CLOAK:
		RING_INVENTORY._cloak._sceneNumber = sceneNum;
		break;
	case OBJECT_TUNIC:
		RING_INVENTORY._tunic._sceneNumber = sceneNum;
		break;
	case OBJECT_CANDLE:
		RING_INVENTORY._candle._sceneNumber = sceneNum;
		break;
	case OBJECT_STRAW:
		RING_INVENTORY._straw._sceneNumber = sceneNum;
		break;
	case OBJECT_SCIMITAR:
		RING_INVENTORY._scimitar._sceneNumber = sceneNum;
		break;
	case OBJECT_SWORD:
		RING_INVENTORY._sword._sceneNumber = sceneNum;
		break;
	case OBJECT_HELMET:
		RING_INVENTORY._helmet._sceneNumber = sceneNum;
		break;
	case OBJECT_ITEMS:
		RING_INVENTORY._items._sceneNumber = sceneNum;
		break;
	case OBJECT_CONCENTRATOR:
		RING_INVENTORY._concentrator._sceneNumber = sceneNum;
		break;
	case OBJECT_NULLIFIER:
		RING_INVENTORY._nullifier._sceneNumber = sceneNum;
		break;
	case OBJECT_PEG:
		RING_INVENTORY._peg._sceneNumber = sceneNum;
		break;
	case OBJECT_VIAL:
		RING_INVENTORY._vial._sceneNumber = sceneNum;
		break;
	case OBJECT_JACKET:
		RING_INVENTORY._jacket._sceneNumber = sceneNum;
		break;
	case OBJECT_TUNIC2:
		RING_INVENTORY._tunic2._sceneNumber = sceneNum;
		break;
	case OBJECT_BONE:
		RING_INVENTORY._bone._sceneNumber = sceneNum;
		break;
	case OBJECT_EMPTY_JAR:
		RING_INVENTORY._emptyJar._sceneNumber = sceneNum;
		break;
	case OBJECT_JAR:
		RING_INVENTORY._jar._sceneNumber = sceneNum;
		break;
	default:
		DebugPrintf("Invalid object Id %s\n", argv[1]);
		break;
	}

	return true;
}

/*
 * This command lists the objects available, and their ID
 */
bool BlueForceDebugger::Cmd_ListObjects(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	DebugPrintf("Available objects for this game are:\n");
	DebugPrintf("1  - INV_COLT45\n");
	DebugPrintf("2  - INV_AMMO_CLIP\n");
	DebugPrintf("3  - INV_SPARE_CLIP\n");
	DebugPrintf("4  - INV_HANDCUFFS\n");
	DebugPrintf("5  - INV_GREENS_GUN\n");
	DebugPrintf("6  - INV_TICKET_BOOK\n");
	DebugPrintf("7  - INV_MIRANDA_CARD\n");
	DebugPrintf("8  - INV_FOREST_RAP\n");
	DebugPrintf("9  - INV_GREEN_ID\n");
	DebugPrintf("10 - INV_BASEBALL_CARD\n");
	DebugPrintf("11 - INV_BOOKING_GREEN\n");
	DebugPrintf("12 - INV_FLARE\n");
	DebugPrintf("13 - INV_COBB_RAP\n");
	DebugPrintf("14 - INV_22_BULLET\n");
	DebugPrintf("15 - INV_AUTO_RIFLE\n");
	DebugPrintf("16 - INV_WIG\n");
	DebugPrintf("17 - INV_FRANKIE_ID\n");
	DebugPrintf("18 - INV_TYRONE_ID\n");
	DebugPrintf("19 - INV_22_SNUB\n");
	DebugPrintf("20 - INV_BOOKING_FRANKIE\n");
	DebugPrintf("21 - INV_BOOKING_GANG\n");
	DebugPrintf("22 - INV_FBI_TELETYPE\n");
	DebugPrintf("23 - INV_DA_NOTE\n");
	DebugPrintf("24 - INV_PRINT_OUT\n");
	DebugPrintf("25 - INV_WAREHOUSE_KEYS\n");
	DebugPrintf("26 - INV_CENTER_PUNCH\n");
	DebugPrintf("27 - INV_TRANQ_GUN\n");
	DebugPrintf("28 - INV_HOOK\n");
	DebugPrintf("29 - INV_RAGS\n");
	DebugPrintf("30 - INV_JAR\n");
	DebugPrintf("31 - INV_SCREWDRIVER\n");
	DebugPrintf("32 - INV_D_FLOPPY\n");
	DebugPrintf("33 - INV_BLANK_DISK\n");
	DebugPrintf("34 - INV_STICK\n");
	DebugPrintf("35 - INV_CRATE1\n");
	DebugPrintf("36 - INV_CRATE2\n");
	DebugPrintf("37 - INV_SHOEBOX\n");
	DebugPrintf("38 - INV_BADGE\n");
	DebugPrintf("39 - INV_RENTAL_COUPON\n");
	DebugPrintf("40 - INV_NICKEL\n");
	DebugPrintf("41 - INV_LYLE_CARD\n");
	DebugPrintf("42 - INV_CARTER_NOTE\n");
	DebugPrintf("43 - INV_MUG_SHOT\n");
	DebugPrintf("44 - INV_CLIPPING\n");
	DebugPrintf("45 - INV_MICROFILM \n");
	DebugPrintf("46 - INV_WAVE_KEYS\n");
	DebugPrintf("47 - INV_RENTAL_KEYS\n");
	DebugPrintf("48 - INV_NAPKIN\n");
	DebugPrintf("49 - INV_DMV_PRINTOUT\n");
	DebugPrintf("50 - INV_FISHING_NET\n");
	DebugPrintf("51 - INV_ID\n");
	DebugPrintf("52 - INV_9MM_BULLETS\n");
	DebugPrintf("53 - INV_SCHEDULE\n");
	DebugPrintf("54 - INV_GRENADES\n");
	DebugPrintf("55 - INV_YELLOW_CORD\n");
	DebugPrintf("56 - INV_HALF_YELLOW_CORD\n");
	DebugPrintf("57 - INV_BLACK_CORD\n");
	DebugPrintf("58 - INV_HALF_BLACK_CORD\n");
	DebugPrintf("59 - INV_WARRANT\n");
	DebugPrintf("60 - INV_JACKET\n");
	DebugPrintf("61 - INV_GREENS_KNIFE\n");
	DebugPrintf("62 - INV_DOG_WHISTLE\n");
	DebugPrintf("63 - INV_AMMO_BELT\n");
	DebugPrintf("64 - INV_CARAVAN_KEY\n");
	return true;
}

bool BlueForceDebugger::Cmd_MoveObject(int argc, const char **argv) {
	// Check for a flag to clear
	if ((argc < 2) || (argc > 3)){
		DebugPrintf("Usage: %s <object number> [<scene number>]\n", argv[0]);
		DebugPrintf("If no scene is specified, the object will be added to inventory\n");
		return true;
	}

	int objNum = strToInt(argv[1]);
	int sceneNum = 1;
	if (argc == 3)
		sceneNum = strToInt(argv[2]);

	if ((objNum > 0) && (objNum < 65))
		BF_INVENTORY.setObjectScene(objNum, sceneNum);
	else
		DebugPrintf("Invalid object Id %s\n", argv[1]);

	return true;
}

/*
 * This command lists the objects available, and their ID
 */
bool Ringworld2Debugger::Cmd_ListObjects(int argc, const char **argv) {
	if (argc != 1) {
		DebugPrintf("Usage: %s\n", argv[0]);
		return true;
	}

	DebugPrintf("Available objects for this game are:\n");
	DebugPrintf("1  - R2_OPTO_DISK\n");
	DebugPrintf("2  - R2_2\n");
	DebugPrintf("3  - R2_NEGATOR_GUN\n");
	DebugPrintf("4  - R2_STEPPING_DISKS\n");
	DebugPrintf("5  - R2_5\n");
	DebugPrintf("6  - R2_6\n");
	DebugPrintf("7  - R2_7\n");
	DebugPrintf("8  - R2_8\n");
	DebugPrintf("9  - R2_9\n");
	DebugPrintf("10 - R2_10\n");
	DebugPrintf("11 - R2_11\n");
	DebugPrintf("12 - R2_12\n");
	DebugPrintf("13 - R2_13\n");
	DebugPrintf("14 - R2_14\n");
	DebugPrintf("15 - R2_15\n");
	DebugPrintf("16 - R2_16\n");
	DebugPrintf("17 - R2_17\n");
	DebugPrintf("18 - R2_18\n");
	DebugPrintf("19 - R2_19\n");
	DebugPrintf("20 - R2_20\n");
	DebugPrintf("21 - R2_21\n");
	DebugPrintf("22 - R2_22\n");
	DebugPrintf("23 - R2_23\n");
	DebugPrintf("24 - R2_24\n");
	DebugPrintf("25 - R2_25\n");
	DebugPrintf("26 - R2_26\n");
	DebugPrintf("27 - R2_27\n");
	DebugPrintf("28 - R2_28\n");
	DebugPrintf("29 - R2_29\n");
	DebugPrintf("30 - R2_30\n");
	DebugPrintf("31 - R2_31\n");
	DebugPrintf("32 - R2_32\n");
	DebugPrintf("33 - R2_33\n");
	DebugPrintf("34 - R2_34\n");
	DebugPrintf("35 - R2_35\n");
	DebugPrintf("36 - R2_36\n");
	DebugPrintf("37 - R2_37\n");
	DebugPrintf("38 - R2_38\n");
	DebugPrintf("39 - R2_39\n");
	DebugPrintf("40 - R2_40\n");
	DebugPrintf("41 - R2_41\n");
	DebugPrintf("42 - R2_42\n");
	DebugPrintf("43 - R2_43\n");
	DebugPrintf("44 - R2_44\n");
	DebugPrintf("45 - R2_45\n");
	DebugPrintf("46 - R2_46\n");
	DebugPrintf("47 - R2_47\n");
	DebugPrintf("48 - R2_48\n");
	DebugPrintf("49 - R2_49\n");
	DebugPrintf("50 - R2_50\n");
	DebugPrintf("51 - R2_51\n");
	DebugPrintf("52 - R2_52\n");

	return true;
}

bool Ringworld2Debugger::Cmd_MoveObject(int argc, const char **argv) {
	// Check for a flag to clear
	if ((argc < 2) || (argc > 3)){
		DebugPrintf("Usage: %s <object number> [<scene number>]\n", argv[0]);
		DebugPrintf("If no scene is specified, the object will be added to inventory\n");
		return true;
	}

	int objNum = strToInt(argv[1]);
	int sceneNum = 1;
	if (argc == 3)
		sceneNum = strToInt(argv[2]);

	if ((objNum > 0) && (objNum < 53))
		R2_INVENTORY.setObjectScene(objNum, sceneNum);
	else
		DebugPrintf("Invalid object Id %s\n", argv[1]);

	return true;
}
} // End of namespace TsAGE
