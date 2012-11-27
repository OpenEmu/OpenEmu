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

#include "lure/room.h"
#include "lure/luredefs.h"
#include "lure/res.h"
#include "lure/screen.h"
#include "lure/game.h"
#include "lure/lure.h"
#include "lure/events.h"
#include "lure/strings.h"
#include "lure/scripts.h"
#include "lure/sound.h"

namespace Lure {

static Room *int_room;

RoomLayer::RoomLayer(uint16 screenId, bool backgroundLayer):
		Surface(FULL_SCREEN_WIDTH, FULL_SCREEN_HEIGHT) {
	Disk &disk = Disk::getReference();
	byte *screenData = data().data();
	int cellY;
	int cellIndex = 0;

	// Reset all the cells to unused
	Common::fill((uint8 *) _cells, (uint8 *) _cells + GRID_SIZE, 0xff);

	// Load up the screen data
	MemoryBlock *rawData = disk.getEntry(screenId);
	loadScreen(rawData);

	uint16 v = READ_BE_UINT16(rawData->data());
	bool is5Bit = (v & 0xfffe) == 0x140;
	delete rawData;

	_paletteId = (screenId & 0xffe0) - 1;
	if (is5Bit) {
		uint16 roomNumber = Room::getReference().roomNumber();

		if (roomNumber == 6)
			_paletteId = 0x45ff;
		else if (roomNumber == 49)
			_paletteId = 0xf1ff;
		else {
			_paletteId = 0x40ff;
		}
	}

	// Loop through each cell of the screen
	for (cellY = 0; cellY < NUM_VERT_RECTS; ++cellY) {
		for (int cellX = 0; cellX < NUM_HORIZ_RECTS; ++cellX) {
			bool hasPixels = false;

			if (backgroundLayer) {
				hasPixels = true;
			} else {
				// Check the cell
				for (int yP = 0; yP < RECT_SIZE; ++yP) {
					if (hasPixels) break;
					byte *linePos = screenData + (cellY * RECT_SIZE + yP + MENUBAR_Y_SIZE)
						* FULL_SCREEN_WIDTH + (cellX * RECT_SIZE);

					for (int xP = 0; xP < RECT_SIZE; ++xP) {
						hasPixels = *linePos++ != 0;
						if (hasPixels) break;
					}
				}
			}

			_cells[cellY + NUM_EDGE_RECTS][cellX + NUM_EDGE_RECTS] =
				hasPixels ? cellIndex++ : 0xff;
		}
	}
}

/*--------------------------------------------------------------------------*/

Room::Room(): _screen(Screen::getReference()) {
	int_room = this;

	_roomData = NULL;
	_talkDialog = NULL;
	_hotspotId = 0;
	_hotspotName[0] = '\0';
	_statusLine[0] = '\0';
	for (int ctr = 0; ctr < MAX_NUM_LAYERS; ++ctr) _layers[ctr] = NULL;
	_numLayers = 0;
	_showInfo = false;
	_isExit = false;
	_roomNumber = 0;
	_destRoomNumber = 0;
	_cursorState = CS_NONE;

//****DEBUG****
	memset(tempLayer, 0, DECODED_PATHS_WIDTH * DECODED_PATHS_HEIGHT * 2);
}

Room::~Room() {
	for (int layerNum = 0; layerNum < _numLayers; ++layerNum)
		delete _layers[layerNum];

	delete _talkDialog;
	int_room = NULL;
}

Room &Room::getReference() {
	return *int_room;
}

// leaveRoom
// Handles leaving the current room

void Room::leaveRoom() {
	Resources &r = Resources::getReference();

	// Scan through the hotspot list and remove any uneeded entries

	HotspotList &list = r.activeHotspots();
	HotspotList::iterator i = list.begin();
	while (i != list.end()) {
		Hotspot const &h = **i;
		if (!h.persistant()) {
			i = list.erase(i);
		} else {
			++i;
		}
	}
}

void Room::loadRoomHotspots() {
	Resources &r = Resources::getReference();
	HotspotDataList &list = r.hotspotData();

	HotspotDataList::iterator i;
	for (i = list.begin(); i != list.end(); ++i) {
		HotspotData const &rec = **i;

		if ((rec.hotspotId < 0x7530) && (rec.roomNumber == _roomNumber) &&
			(rec.layer != 0))
			r.activateHotspot(rec.hotspotId);
	}
}

void Room::checkRoomHotspots() {
	uint16 rangeStart[4] = {0x408, 0x3e8, 0x7530, 0x2710};
	uint16 rangeEnd[4] = {0x270f, 0x407, 0xffff, 0x752f};

	Mouse &m = Mouse::getReference();
	Resources &res = Resources::getReference();
	HotspotDataList &list = res.hotspotData();
	HotspotData *entry = NULL;
	int16 currentX = m.x();
	int16 currentY = m.y();
	HotspotDataList::iterator i;

	_destRoomNumber = 0;

	// Loop for each range of hotspot Ids
	for (int ctr = 0; ctr < 4; ++ctr) {
		for (i = list.begin(); i != list.end(); ++i) {
			entry = (*i).get();
			if ((entry->hotspotId < rangeStart[ctr]) || (entry->hotspotId > rangeEnd[ctr]))
				// Hotspot outside range, so skip it
				continue;

			bool skipFlag = (entry->roomNumber != _roomNumber);
			if (!skipFlag) {
				skipFlag = (((entry->flags & HOTSPOTFLAG_FOUND) == 0) &&
							((entry->flags & HOTSPOTFLAG_SKIP) != 0)) ||
						    ((entry->flags & HOTSPOTFLAG_MENU_EXCLUSION) != 0);
			}

			if ((!skipFlag) && (entry->hotspotId < 0x409))
				// For character hotspots, validate they're in clipping range
				skipFlag = !res.checkHotspotExtent(entry);

			if (!skipFlag && (entry->hotspotId >= 0x2710) && (entry->hotspotId <= 0x27ff)) {
				RoomExitJoinData *rec = res.getExitJoin(entry->hotspotId);
				if ((rec) && (!rec->blocked))
					// Hotspot is over a room exit, and it's not blocked, so don't
					// register it as an active hotspot
					skipFlag = true;
			}

			if (!skipFlag) {
				// Check for a hotspot override
				HotspotOverrideData *hsEntry = res.getHotspotOverride(entry->hotspotId);

				if (hsEntry) {
					// Check whether cursor is in override hotspot area
					if ((currentX >= hsEntry->xs) && (currentX <= hsEntry->xe) &&
						(currentY >= hsEntry->ys) && (currentY <= hsEntry->ye))
						// Found to be in hotspot entry
						break;
				} else {
					// Check whether cursor is in default hospot area
					if ((currentX >= entry->startX) && (currentY >= entry->startY) &&
						(currentX < entry->startX + entry->widthCopy) &&
						(currentY < entry->startY + entry->height))
						// Found hotspot entry
						break;
				}
			}
		}

		if (i != list.end())
			break;
	}

	if (i == list.end()) {
		_hotspotId = 0;
		_hotspotNameId = 0;
		_hotspot = NULL;
	} else {
		_hotspotNameId = entry->nameId;
		_hotspot = entry;
		_hotspotId = entry->hotspotId;
		_isExit = false;
		entry->flags |= HOTSPOTFLAG_FOUND;
	}
}

CursorType Room::checkRoomExits() {
	Mouse &m = Mouse::getReference();
	Resources &res = Resources::getReference();
	_destRoomNumber = 0;

	RoomExitHotspotList &exits = _roomData->exitHotspots;
	if (exits.empty()) return CURSOR_ARROW;
	RoomExitJoinData *join;
	bool skipFlag;

	RoomExitHotspotList::iterator i;
	for (i = exits.begin(); i != exits.end(); ++i) {
		RoomExitHotspotData const &rec = **i;
		skipFlag = false;

		if (rec.hotspotId != 0) {
			join = res.getExitJoin(rec.hotspotId);
			if ((join) && (join->blocked != 0))
				skipFlag = true;
		}

		if (!skipFlag && (m.x() >= rec.xs) && (m.x() <= rec.xe) &&
			(m.y() >= rec.ys) && (m.y() <= rec.ye)) {
			// Cursor is within exit area
			CursorType cursorNum = (CursorType)rec.cursorNum;
			_destRoomNumber = rec.destRoomNumber;

			// If it's a hotspotted exit, change arrow to the + arrow
			if (rec.hotspotId != 0) {
				_hotspotId = rec.hotspotId;
				_hotspot = res.getHotspot(_hotspotId);
				_hotspotNameId = _hotspot->nameId;
				_isExit = true;
				cursorNum = (CursorType)((int)cursorNum + 7);
			}

			return cursorNum;
		}
	}

	// No room exits found
	return CURSOR_ARROW;
}

void Room::addAnimation(Hotspot &h) {
	Surface &s = _screen.screen();
	char buffer[10];
	h.copyTo(&s);

	if (_showInfo) {
		int16 x = h.x();
		int16 y = h.y();
		if ((x >= 0) && (x < FULL_SCREEN_WIDTH) && (y >= 0) && (y < FULL_SCREEN_HEIGHT))
			sprintf(buffer, "%xh", h.hotspotId());

	}
}

void Room::addLayers(Hotspot &h) {
	int16 hsX = h.x() + (NUM_EDGE_RECTS * RECT_SIZE);
	int16 hsY = h.y() + (NUM_EDGE_RECTS * RECT_SIZE) - MENUBAR_Y_SIZE;

	int16 xStart = hsX / RECT_SIZE;
	int16 xEnd = (hsX + h.width()) / RECT_SIZE;
	int16 numX = xEnd - xStart + 1;
	int16 yStart = hsY / RECT_SIZE;
	int16 yEnd = (hsY + h.heightCopy() - 1) / RECT_SIZE;
	int16 numY = yEnd - yStart + 1;

	if ((xStart < 0) || (yEnd < 0))
		return;

	for (int16 xCtr = 0; xCtr < numX; ++xCtr, ++xStart) {
		int16 xs = xStart - NUM_EDGE_RECTS;
		if (xs < 0) continue;

		// Check foreground layers for an occupied one

		int layerNum = 1;
		while ((layerNum < 4) && (_layers[layerNum] != NULL) &&
				(_layers[layerNum]->getCell(xStart, yEnd) == 0xff))
			++layerNum;
		if ((layerNum == 4) || (_layers[layerNum] == NULL)) continue;

		int16 ye = yEnd - NUM_EDGE_RECTS;
		for (int16 yCtr = 0; yCtr < numY; ++yCtr, --ye) {
			if (ye < 0) break;
			addCell(xs, ye, layerNum);
		}
	}
}

void Room::addCell(int16 xp, int16 yp, int layerNum) {
	Surface &s = _screen.screen();

	while ((layerNum < 4) && (_layers[layerNum] != NULL) &&
			(_layers[layerNum]->getCell(xp + NUM_EDGE_RECTS, yp + NUM_EDGE_RECTS) >= 0xfe))
		++layerNum;
	if ((layerNum == 4) || (_layers[layerNum] == NULL)) return;

	RoomLayer *layer = _layers[layerNum];

	int index = ((yp * RECT_SIZE + MENUBAR_Y_SIZE) * FULL_SCREEN_WIDTH) + (xp * RECT_SIZE);
	byte *srcPos = layer->data().data() + index;
	byte *destPos = s.data().data() + index;

	for (int yCtr = 0; yCtr < RECT_SIZE; ++yCtr) {
		for (int xCtr = 0; xCtr < RECT_SIZE; ++xCtr, ++destPos) {
			byte pixel = *srcPos++;
			if (pixel) *destPos = pixel;
		}

		// Move to start of next cell line
		srcPos += FULL_SCREEN_WIDTH - RECT_SIZE;
		destPos += FULL_SCREEN_WIDTH - RECT_SIZE;
	}
}

void Room::blockMerge() {
	for (int layerNum1 = 0; layerNum1 < 3; ++layerNum1) {
		if (_layers[layerNum1] == NULL) break;

		for (int layerNum2 = layerNum1 + 1; layerNum2 < 4; ++layerNum2) {
			if (_layers[layerNum2] == NULL) break;

			for (int yp = 0; yp < NUM_VERT_RECTS; ++yp) {
				for (int xp = 0; xp < NUM_HORIZ_RECTS; ++xp) {
					if (_layers[layerNum1]->isOccupied(xp + NUM_EDGE_RECTS, yp + NUM_EDGE_RECTS) &&
						_layers[layerNum2]->isOccupied(xp + NUM_EDGE_RECTS, yp + NUM_EDGE_RECTS)) {
						// Copy the rect from the later layer onto the earlier layer
						int offset = (yp * RECT_SIZE + MENUBAR_Y_SIZE) * FULL_SCREEN_WIDTH + (xp * RECT_SIZE);
						byte *src = _layers[layerNum2]->data().data() + offset;
						byte *dest = _layers[layerNum1]->data().data() + offset;

						for (int y = 0; y < RECT_SIZE; ++y) {
							for (int x = 0; x < RECT_SIZE; ++x, ++src, ++dest) {
								if (*src != 0) *dest = *src;
							}
							src += FULL_SCREEN_WIDTH - RECT_SIZE;
							dest += FULL_SCREEN_WIDTH - RECT_SIZE;
						}
					}
				}
			}
		}
	}
}

void Room::layersPostProcess() {
	for (int layerNum = 1; layerNum < 4; ++layerNum) {
		if (_layers[layerNum] == NULL)
			continue;

		// Layer optimisation
		for (int xp = NUM_EDGE_RECTS; xp < NUM_HORIZ_RECTS + NUM_EDGE_RECTS; ++xp) {
			bool priorFlag = false, nextFlag = false;

			for (int yp = NUM_EDGE_RECTS; yp < NUM_VERT_RECTS + NUM_EDGE_RECTS; ++yp) {
				if (_layers[layerNum]->getCell(xp, yp) == 0xff) {
					priorFlag = false;
					nextFlag = false;
					continue;
				}

				if (priorFlag && (_layers[layerNum]->getCell(xp - 1, yp) == 0xff))
					_layers[layerNum]->setCell(xp - 1, yp, 0xfe);
				if (nextFlag && (_layers[layerNum]->getCell(xp + 1, yp) == 0xff))
					_layers[layerNum]->setCell(xp + 1, yp, 0xfe);

				priorFlag = _layers[layerNum]->getCell(xp - 1, yp) != 0xff;
				nextFlag = _layers[layerNum]->getCell(xp + 1, yp) != 0xff;
			}
		}

		// Layer extension of final row to off-screen edge rows below

		for (int xp = NUM_EDGE_RECTS + NUM_HORIZ_RECTS - 1; xp >= NUM_EDGE_RECTS; --xp) {
			if (_layers[layerNum]->getCell(xp, NUM_EDGE_RECTS + NUM_VERT_RECTS - 1) != 0xff) {
				for (int yp = NUM_VERT_RECTS + NUM_EDGE_RECTS; yp < FULL_VERT_RECTS; ++yp)
					_layers[layerNum]->setCell(xp, yp, 0xfe);
			}
		}
	}
}

void Room::update() {
	Surface &s = _screen.screen();
	Resources &res = Resources::getReference();
	HotspotList &hotspots = res.activeHotspots();
	byte white = LureEngine::getReference().isEGA() ?  EGA_DIALOG_WHITE_COLOR : VGA_DIALOG_WHITE_COLOR;
	HotspotList::iterator i;

	// Copy the background to the temporary screen surface
	_layers[0]->copyTo(&s);

	// Handle first layer (layer 3)
	for (i = hotspots.begin(); i != hotspots.end(); ++i) {
		Hotspot &h = **i;

		if ((h.roomNumber() == _roomNumber) && h.isActiveAnimation() && (h.layer() == 3)) {
			addAnimation(h);
			addLayers(h);
		}
	}

	// Handle second layer (layer 1) - do in order of Y axis
	Common::List<Hotspot *> tempList;
	Common::List<Hotspot *>::iterator iTemp;
	for (i = hotspots.begin(); i != hotspots.end(); ++i) {
		Hotspot *h = i->get();
		if ((h->layer() != 1) || (h->roomNumber() != _roomNumber) ||
			h->skipFlag() || !h->isActiveAnimation())
			continue;
		int16 endY = h->y() + h->heightCopy();

		for (iTemp = tempList.begin(); iTemp != tempList.end(); ++iTemp) {
			Hotspot *hTemp = *iTemp;
			int16 tempY = hTemp->y() + hTemp->heightCopy();
			if (endY < tempY) break;
		}
		tempList.insert(iTemp, h);
	}
	for (iTemp = tempList.begin(); iTemp != tempList.end(); ++iTemp) {
		Hotspot &h = **iTemp;
		addAnimation(h);
		addLayers(h);
	}

	// Handle third layer (layer 2)
	for (i = hotspots.begin(); i != hotspots.end(); ++i) {
		Hotspot &h = **i;

		if ((h.roomNumber() == _roomNumber) && h.isActiveAnimation() && (h.layer() == 2)) {
			addAnimation(h);
		}
	}

	// Show any active talk dialog
	if (_talkDialog)  {
		// Make sure the character is still active and in the viewing room
		Hotspot *talkCharacter = res.getActiveHotspot(res.getTalkingCharacter());
		if ((talkCharacter != NULL) && (talkCharacter->roomNumber() == _roomNumber))
			_talkDialog->copyTo(&s, _talkDialogX, _talkDialogY);
	}

	// Handle showing the status line
	if (!*_statusLine) {
		// No current status action being display
		if (_hotspotId != 0)
			s.writeString(0, 0, _hotspotName, false);
	} else {
		// Word wrap (if necessary) the status line and dispaly it
		char *statusLineCopy = strdup(_statusLine);
		char **lines;
		uint8 numLines;
		int16 yPos = 0;
		s.wordWrap(statusLineCopy, s.width(), lines, numLines);
		for (int lineNum = 0; lineNum < numLines; ++lineNum) {
			s.writeString(0, yPos, lines[lineNum], false, white);
			yPos += FONT_HEIGHT;
		}
		Memory::dealloc(lines);
		Memory::dealloc(statusLineCopy);
	}

	// Debug - if the bottle object is on layer 0FEh, then display it's surface
	Hotspot *displayHotspot = res.getActiveHotspot(BOTTLE_HOTSPOT_ID);
	if ((displayHotspot != NULL) && (displayHotspot->layer() == 0xfe))
		displayHotspot->frames().copyTo(&s);

	// If show information is turned on, show extra debugging information
	if (_showInfo) {
		char buffer[64];

		// Temporary display of pathfinding data
		for (int yctr = 0; yctr < ROOM_PATHS_HEIGHT; ++yctr) {
			for (int xctr = 0; xctr < ROOM_PATHS_WIDTH; ++xctr) {
/*
				if (_roomData->paths.isOccupied(xctr, yctr))
					s.fillRect(Rect(xctr * 8, yctr * 8 + 8, xctr * 8 + 7, yctr * 8 + 15), 255);
*/
				uint16 v = tempLayer[(yctr + 1) * DECODED_PATHS_WIDTH + xctr + 1];
				if ((v != 0) && (v < 100)) {
					sprintf(buffer, "%d", v % 10);
					s.writeString(xctr * 8, yctr * 8 + 8, buffer, true);
//				} else if (v == 0xffff) {
				} else if (_roomData->paths.isOccupied(xctr, yctr)) {
					s.fillRect(Common::Rect(xctr * 8, yctr * 8 + 8, xctr * 8 + 7, yctr * 8 + 15), 255);
				}
			}
		}

		Mouse &m = Mouse::getReference();
		sprintf(buffer, "Room %d Pos (%d,%d) @ (%d,%d)", _roomNumber, m.x(), m.y(),
			m.x() / RECT_SIZE, (m.y() - MENUBAR_Y_SIZE) / RECT_SIZE);
		s.writeString(FULL_SCREEN_WIDTH / 2, 0, buffer, false, white);
	}
}

void Room::setRoomNumber(uint16 newRoomNumber, bool showOverlay) {
	Resources &res = Resources::getReference();
	Game &game = Game::getReference();
	Mouse &mouse = Mouse::getReference();
	bool isEGA = LureEngine::getReference().isEGA();

	mouse.pushCursorNum(CURSOR_DISK);

	_roomData = res.getRoom(newRoomNumber);
	if (!_roomData)
		error("Tried to change to non-existant room: %d", newRoomNumber);

	bool fadeFlag = (newRoomNumber != _roomNumber) && (_roomNumber != 0);
	bool leaveFlag = _roomNumber != 999;

	_roomNumber = _roomData->roomNumber;
	_descId = _roomData->descId;

	if (fadeFlag) {
		if (isEGA)
			_screen.setPaletteEmpty();
		else
			// Fade out all the colors except the high index 0FFh, which is used to show the
			// disk cursor as a room changes
			_screen.paletteFadeOut(GAME_COLORS - 1);

		// Deallocate graphical layers from the room
		for (int layerNum = 0; layerNum < _numLayers; ++layerNum) {
			if (_layers[layerNum]) {
				delete _layers[layerNum];
				_layers[layerNum] = NULL;
			}
		}

		if (leaveFlag) {
			leaveRoom();
			Sound.removeSounds();
		}
	}

	_screen.empty();
	_screen.setPaletteEmpty(RES_PALETTE_ENTRIES);

	_numLayers = _roomData->numLayers;
	if (showOverlay) ++_numLayers;

	for (uint8 layerNum = 0; layerNum < _numLayers; ++layerNum)
		_layers[layerNum] = new RoomLayer(_roomData->layers[layerNum],
			layerNum == 0);

	blockMerge();
	layersPostProcess();

	// Generate the palette for the room that will be faded in
	Palette *p;
	if (isEGA) {
		p = new Palette(_layers[0]->paletteId());
	} else {
		p = new Palette(GAME_PALETTE_RESOURCE_ID);
		Palette tempPalette(_layers[0]->paletteId());
		p->copyFrom(&tempPalette);
		res.insertPaletteSubset(*p);
	}

	// Set the new room number
	res.fieldList().setField(ROOM_NUMBER, newRoomNumber);

	if (_roomData->sequenceOffset != 0xffff)
		Script::execute(_roomData->sequenceOffset);

	loadRoomHotspots();

	if (leaveFlag) {
		// A previous room has been left - check if there are any seconds worth
		// of animations that need to be done in 'fast forward'
		if ((_roomData->exitTime != 0xffff) && (_roomData->exitTime != 0)) {
			// If time has passed, animation ticks needed before room is displayed
			int numSeconds = (g_system->getMillis() - _roomData->exitTime) / 1000;
			if (numSeconds > 300) numSeconds = 300;

			game.preloadFlag() = true;
			while (numSeconds-- > 0)
				game.tick();
			game.preloadFlag() = false;
		}
	}

	game.tick();
	update();
	_screen.update();

	if (fadeFlag && !isEGA)
		_screen.paletteFadeIn(p);
	else
		_screen.setPalette(p);

	mouse.popCursor();
	delete p;
}

// checkCursor
// Called repeatedly to check if any changes to the cursor are required

void Room::checkCursor() {
	Mouse &mouse = Mouse::getReference();
	Resources &res = Resources::getReference();
	uint16 oldHotspotId = _hotspotId;
	CursorType currentCursor = mouse.getCursorNum();
	CursorType newCursor = currentCursor;
	CurrentAction playerAction = res.getActiveHotspot(PLAYER_ID)->currentActions().action();
	uint16 oldRoomNumber = res.fieldList().getField(OLD_ROOM_NUMBER);

	if ((currentCursor >= CURSOR_TIME_START) && (currentCursor <= CURSOR_TIME_END) &&
		((playerAction == START_WALKING) || (playerAction == PROCESSING_PATH))) {
		// Animate the clock when processing the player path
		newCursor = (CursorType)((int)newCursor + 1);
		if (newCursor == CURSOR_CROSS) newCursor = CURSOR_TIME_START;
	} else if (checkInTalkDialog() && (oldRoomNumber == 0)) {
		newCursor = CURSOR_TALK;
	} else if (res.getTalkData()) {
		newCursor = CURSOR_ARROW;
	} else if (_cursorState == CS_BUMPED) {
		newCursor = CURSOR_CAMERA;
	} else if (_cursorState == CS_TALKING) {
		newCursor = CURSOR_ARROW;
	} else if (mouse.y() < MENUBAR_Y_SIZE) {
		// If viewing a room remotely, then don't change to the menu cursor
		if (oldRoomNumber != 0) return;

		newCursor = CURSOR_MENUBAR;
	} else if (_cursorState != CS_NONE) {
		// Currently in a special mode
		checkRoomHotspots();
		newCursor = CURSOR_CAMERA;
	} else {
		// Check for a highlighted hotspot
		checkRoomHotspots();

		if (_hotspotId != 0) {
			newCursor = CURSOR_CROSS;
		} else {
			newCursor = checkRoomExits();
		}

		if (oldHotspotId != _hotspotId)
			StringData::getReference().getString(_hotspotNameId, _hotspotName);
	}

	if (mouse.getCursorNum() != newCursor)
		mouse.setCursorNum(newCursor);
}

void Room::setTalkDialog(uint16 srcCharacterId, uint16 destCharacterId, uint16 usedId, uint16 stringId) {
	Resources &res = Resources::getReference();
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Room::setTalkDialog - char=%xh string=%d",
		srcCharacterId, stringId);

	if (_talkDialog) {
		delete _talkDialog;
		_talkDialog = NULL;
	}
/*
	if (res.getTalkingCharacter() != 0) {
		// Signal to any talked to character that they're no longer being talked to
		HotspotData *talkingChar = res.getHotspot(res.getTalkingCharacter());
		if ((talkingChar->talkDestCharacterId != 0) &&
			(talkingChar->talkDestCharacterId != NOONE_ID)) {
			HotspotData *destChar = res.getHotspot(talkingChar->talkDestCharacterId);
			destChar->talkerId = 0;
		}
	}
*/
	res.setTalkingCharacter(srcCharacterId);

	if (srcCharacterId == 0)
		return;

	HotspotData *character = res.getHotspot(srcCharacterId);
	if (character->roomNumber != _roomNumber)
		return;

	_talkDialog = new TalkDialog(srcCharacterId, destCharacterId, usedId, stringId);
	_talkDialogX = character->startX + (character->width / 2) - (TALK_DIALOG_WIDTH / 2);

	if (_talkDialogX < 0) _talkDialogX = 0;
	if (_talkDialogX + TALK_DIALOG_WIDTH >= FULL_SCREEN_WIDTH - 10)
		_talkDialogX = FULL_SCREEN_WIDTH - 10 - TALK_DIALOG_WIDTH;

	_talkDialogY = TALK_DIALOG_Y;
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Room::setTalkDialog end");
}

// Checks to see if a talk dialog is active, and if so if the mouse is in
// within it

bool Room::checkInTalkDialog() {
	// Make sure there is a talk dialog active
	if (!_talkDialog) return false;

	// Don't allow dialog close if it's still in progress
	if (_talkDialog->isBuilding()) return false;

	// Only allow the dialog to be closable if it's the player talking, or
	// someone talking to the player
	Resources &res = Resources::getReference();
	uint16 talkerId = res.getTalkingCharacter();
	if ((talkerId == NOONE_ID) || (talkerId == 0))
		return false;

	if (talkerId != PLAYER_ID) {
		HotspotData *charHotspot = res.getHotspot(talkerId);
		assert(charHotspot);
		if (charHotspot->talkDestCharacterId != PLAYER_ID)
			return false;
	}

	// Check boundaries
	Mouse &mouse = Mouse::getReference();
	return ((mouse.x() >= _talkDialogX) && (mouse.y() >= _talkDialogY) &&
		(mouse.x() < _talkDialogX + _talkDialog->surface().width()) &&
		(mouse.y() < _talkDialogY + _talkDialog->surface().height()));
}

void Room::saveToStream(Common::WriteStream *stream) {
	if (_talkDialog == NULL)
		stream->writeUint16LE(0);
	else
		_talkDialog->saveToStream(stream);

	stream->writeUint16LE(_roomNumber);
	stream->writeUint16LE(_destRoomNumber);
	stream->writeByte(_showInfo);
	stream->writeUint16LE(_cursorState);
}

void Room::loadFromStream(Common::ReadStream *stream) {
	uint8 saveVersion = LureEngine::getReference().saveVersion();

	if (_talkDialog) {
		delete _talkDialog;
		_talkDialog = NULL;
	}

	if (saveVersion >= 26)
		_talkDialog = TalkDialog::loadFromStream(stream);

	// Clear any active hotspot
	_hotspotId = 0;
	_hotspotName[0] = '\0';
	_statusLine[0] = '\0';

	uint16 roomNum = stream->readUint16LE();
	_roomNumber = 999; // Dummy room number so current room is faded out
	setRoomNumber(roomNum, false);

	_destRoomNumber = stream->readUint16LE();
	_showInfo = stream->readByte() != 0;
	_cursorState = (CursorState) stream->readUint16LE();
}

void Room::reset() {
	_roomNumber = 999;
	setTalkDialog(0, 0, 0, 0);

	_hotspotId = 0;
	_hotspotName[0] = '\0';
	_statusLine[0] = '\0';
}

} // End of namespace Lure
