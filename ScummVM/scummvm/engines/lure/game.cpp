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

#include "lure/game.h"
#include "lure/animseq.h"
#include "lure/fights.h"
#include "lure/lure.h"
#include "lure/res_struct.h"
#include "lure/room.h"
#include "lure/scripts.h"
#include "lure/sound.h"
#include "lure/strings.h"

#include "common/config-manager.h"
#include "common/system.h"

namespace Lure {

static Game *int_game = NULL;

bool Game::isCreated() {
	return int_game != NULL;
}

Game &Game::getReference() {
	return *int_game;
}

Game::Game() {
	int_game = this;
	_debugger = new Debugger();
	_fastTextFlag = false;
	_preloadFlag = false;
	_debugFlag = gDebugLevel >= ERROR_BASIC;

	_soundFlag = true;
}

Game::~Game() {
	delete _debugger;
}

void Game::tick() {
	// Call the tick method for each hotspot - this is somewaht complicated
	// by the fact that a tick proc can unload both itself and/or others,
	// so we first get a list of the Ids, and call the tick proc for each
	// id in sequence if it's still active
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	HotspotList::iterator i;

	uint16 *idList = new uint16[res.activeHotspots().size()];
	int idSize = 0;
	for (i = res.activeHotspots().begin(); i != res.activeHotspots().end(); ++i) {
		Hotspot const &hotspot = **i;

		if (!_preloadFlag || ((hotspot.layer() != 0xff) &&
			(hotspot.hotspotId() < FIRST_NONCHARACTER_ID)))
			// Add hotspot to list to execute
			idList[idSize++] = hotspot.hotspotId();
	}

	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot ticks begin");
	for (int idCtr = 0; idCtr < idSize; ++idCtr) {
		Hotspot *hotspot = res.getActiveHotspot(idList[idCtr]);
		if (hotspot) {
			fields.setField(CHARACTER_HOTSPOT_ID, hotspot->hotspotId());
			hotspot->tick();
		}
	}
	debugC(ERROR_DETAILED, kLureDebugAnimations, "Hotspot ticks end");

	delete[] idList;
}

void Game::tickCheck() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	bool remoteFlag = res.fieldList().getField(OLD_ROOM_NUMBER) != 0;

	_state |= GS_TICK;
	if ((room.roomNumber() == ROOMNUM_VILLAGE_SHOP) && !remoteFlag && ((_state & GS_TICK) != 0)) {
		// In the village shop,
		bool tockFlag = (_state & GS_TOCK) != 0;
		Sound.addSound(tockFlag ? 16 : 50);

		_state = _state ^ (GS_TICK | GS_TOCK);
	}
}

void Game::nextFrame() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();

	if (Fights.isFighting())
		Fights.fightLoop();

	res.pausedList().countdown();
	room.update();
	room.checkCursor();
	tick();

	Screen::getReference().update();
}

void Game::execute() {
	OSystem &system = *g_system;
	LureEngine &engine = LureEngine::getReference();
	Room &room = Room::getReference();
	Resources &res = Resources::getReference();
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	Screen &screen = Screen::getReference();
	ValueTableData &fields = res.fieldList();

	uint32 timerVal = system.getMillis();
	uint32 timerVal2 = system.getMillis();

	screen.empty();
	screen.setPaletteEmpty();

	bool _loadSavegame = false;

	if (engine.gameToLoad() != -1)
		_loadSavegame = engine.loadGame(engine.gameToLoad());

	if (!_loadSavegame) {
		// Flag for starting game
		setState(GS_RESTART);
	}

	bool initialRestart = true;

	while (!engine.shouldQuit()) {

		if ((_state & GS_RESTART) != 0) {
			res.reset();
			Fights.reset();
			if (!initialRestart) room.reset();

			setState(0);
			Script::execute(STARTUP_SCRIPT);

			int bootParam = initialRestart ? ConfMan.getInt("boot_param") : 0;
			handleBootParam(bootParam);
			initialRestart = false;
		}

		room.update();
		mouse.setCursorNum(CURSOR_ARROW);
		mouse.cursorOn();

		// Main game loop
		while (!engine.shouldQuit() && ((_state & GS_RESTART) == 0)) {
			// If time for next frame, allow everything to update
			if (system.getMillis() > timerVal + GAME_FRAME_DELAY) {
				timerVal = system.getMillis();
				nextFrame();
				res.delayList().tick();

				Sound.musicInterface_ContinuePlaying();
			}

			// Also check if time to do another village shop tick check
			if (system.getMillis() > timerVal2 + GAME_TICK_DELAY) {
				timerVal2 = system.getMillis();
				tickCheck();
			}

			while (events.pollEvent()) {
				if (events.type() == Common::EVENT_KEYDOWN) {
					uint16 roomNum = room.roomNumber();

					if ((events.event().kbd.hasFlags(Common::KBD_CTRL)) &&
						(events.event().kbd.keycode == Common::KEYCODE_d)) {
						// Activate the debugger
						_debugger->attach();
						break;
					}

					// Handle special keys
					bool handled = true;
					switch (events.event().kbd.keycode) {
					case Common::KEYCODE_F5:
						if (isMenuAvailable())
							SaveRestoreDialog::show(true);
						break;

					case Common::KEYCODE_F7:
						SaveRestoreDialog::show(false);
						break;

					case Common::KEYCODE_F9:
						doRestart();
						break;

					case Common::KEYCODE_KP_PLUS:
						if (_debugFlag) {
							while (++roomNum <= 51)
								if (res.getRoom(roomNum) != NULL) break;
							if (roomNum == 52) roomNum = 1;
							room.setRoomNumber(roomNum);
						}
						break;

					case Common::KEYCODE_KP_MINUS:
						if (_debugFlag) {
							if (roomNum == 1) roomNum = 55;
							while (res.getRoom(--roomNum) == NULL)
								;
							room.setRoomNumber(roomNum);
						}
						break;

					case Common::KEYCODE_KP_MULTIPLY:
						if (_debugFlag)
							res.getActiveHotspot(PLAYER_ID)->setRoomNumber(
								room.roomNumber());
						break;

					case Common::KEYCODE_KP_DIVIDE:
					case Common::KEYCODE_SLASH:
						if (_debugFlag)
							room.setShowInfo(!room.showInfo());
						break;

					case Common::KEYCODE_ESCAPE:
						doQuit();
						break;

					default:
						handled = false;
					}
					if (handled)
						continue;
				}

				if ((events.type() == Common::EVENT_LBUTTONDOWN) ||
					(events.type() == Common::EVENT_RBUTTONDOWN))
					handleClick();
			}

			uint16 destRoom;
			destRoom = fields.getField(NEW_ROOM_NUMBER);
			if (destRoom != 0) {
				// Need to change the current room
				strcpy(room.statusLine(), "");
				bool remoteFlag = fields.getField(OLD_ROOM_NUMBER) != 0;
				room.setRoomNumber(destRoom, remoteFlag);
				fields.setField(NEW_ROOM_NUMBER, 0);
			}

			destRoom = fields.playerNewPos().roomNumber;
			if (destRoom != 0) {
				playerChangeRoom();
			}

			system.updateScreen();
			system.delayMillis(10);

			_debugger->onFrame();
		}

		room.leaveRoom();

		// If Skorl catches player, show the catching animation
		if ((_state & GS_CAUGHT) != 0) {
			Palette palette(SKORL_CATCH_PALETTE_ID);
			AnimationSequence *anim = new AnimationSequence(SKORL_CATCH_ANIM_ID, palette, false);
			mouse.cursorOff();
			Sound.addSound(0x33);
			anim->show();
			delete anim;
		}

		// If the Restart/Restore dialog is needed, show it
		if ((_state & GS_RESTORE) != 0) {
			// Show the Restore/Restart dialog
			bool restartFlag = RestartRestoreDialog::show();

			if (restartFlag)
				setState(GS_RESTART);
		}
	}
}

void Game::handleMenuResponse(uint8 selection) {
	Common::String filename;

	switch (selection) {
	case MENUITEM_CREDITS:
		doShowCredits();
		break;

	case MENUITEM_RESTART_GAME:
		doRestart();
		break;

	case MENUITEM_SAVE_GAME:
		SaveRestoreDialog::show(true);
		break;

	case MENUITEM_RESTORE_GAME:
		SaveRestoreDialog::show(false);
		break;

	case MENUITEM_QUIT:
		doQuit();
		break;

	case MENUITEM_TEXT_SPEED:
		doTextSpeed();
		break;

	case MENUITEM_SOUND:
		doSound();
	}
}

void Game::playerChangeRoom() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();
	SequenceDelayList &delayList = Resources::getReference().delayList();

	uint16 roomNum = fields.playerNewPos().roomNumber;
	fields.playerNewPos().roomNumber = 0;
	Common::Point &newPos = fields.playerNewPos().position;

	delayList.clear();

	RoomData *roomData = res.getRoom(roomNum);
	assert(roomData);
	roomData->flags |= HOTSPOTFLAG_FOUND;

	// Check for any room change animation

	int animFlag = fields.getField(ROOM_EXIT_ANIMATION);
	if (animFlag == 1)
		displayChuteAnimation();
	else if (animFlag != 0)
		displayBarrelAnimation();

	fields.setField(ROOM_EXIT_ANIMATION, 0);
	roomData->exitTime = g_system->getMillis();

	// Change to the new room
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	player->currentActions().clear();
	player->setRoomNumber(roomNum);
	player->setPosition((newPos.x & 0xfff8) | 5, newPos.y & 0xfff8);
	player->setOccupied(true);
	room.setRoomNumber(roomNum, false);

	// Special check for change back from Selena
	if ((roomNum != 31) && (roomNum != 14) && (fields.getField(74) != 0)) {
		uint16 v = fields.getField(29);
		if (v != 0) {
			--v;
			fields.setField(29, v);
			if (v == 0)
				res.delayList().add(2, 0xCB7, false);
		}
	}
}

void Game::displayChuteAnimation() {
	Resources &res = Resources::getReference();
	Mouse &mouse = Mouse::getReference();

	ValueTableData &fields = res.fieldList();
	Palette palette(CHUTE_PALETTE_ID);

	debugC(ERROR_INTERMEDIATE, kLureDebugAnimations, "Starting chute animation");
	mouse.cursorOff();

	Sound.killSounds();
	Sound.musicInterface_Play(0x40, 0);

	AnimationSequence *anim = new AnimationSequence(CHUTE_ANIM_ID, palette, false);
	anim->show();
	delete anim;

	anim = new AnimationSequence(CHUTE2_ANIM_ID, palette, false);
	anim->show();
	delete anim;

	anim = new AnimationSequence(CHUTE3_ANIM_ID, palette, false);
	anim->show();
	delete anim;

	Sound.killSounds();
	mouse.cursorOn();
	fields.setField(AREA_FLAG, 1);
}

void Game::displayBarrelAnimation() {
	Mouse &mouse = Mouse::getReference();
	Resources &res = Resources::getReference();

	debugC(ERROR_INTERMEDIATE, kLureDebugAnimations, "Starting barrel animation");
	Palette palette(BARREL_PALETTE_ID);
	AnimationSequence *anim = new AnimationSequence(BARREL_ANIM_ID, palette, false);
	mouse.cursorOff();

	Sound.killSounds();
	Sound.musicInterface_Play(0x3B, 0);

	anim->show();

	delete anim;

	// Disable town NPCs that are no longer needed
	res.deactivateHotspot(SKORL_ID);
	res.deactivateHotspot(BLACKSMITH_ID);
	res.deactivateHotspot(GWEN_ID);
	res.deactivateHotspot(MALLIN_ID);
	res.deactivateHotspot(MONK1_ID);
	res.deactivateHotspot(GOEWIN_ID);
	res.deactivateHotspot(MONK2_ID);
	res.deactivateHotspot(WAYNE_ID);

	Sound.killSounds();
	mouse.cursorOn();
}

void Game::handleClick() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();
	Mouse &mouse = Mouse::getReference();
	uint16 oldRoomNumber = fields.getField(OLD_ROOM_NUMBER);

	if (room.checkInTalkDialog()) {
		// Close the active talk dialog
		room.setTalkDialog(0, 0, 0, 0);
	} else if (oldRoomNumber != 0) {
		// Viewing a room remotely - handle returning to prior room
		if ((room.roomNumber() != 35) || (fields.getField(87) == 0)) {
			// Reset player tick proc and signal to change back to the old room
			res.getActiveHotspot(PLAYER_ID)->setTickProc(PLAYER_TICK_PROC_ID);
			fields.setField(NEW_ROOM_NUMBER, oldRoomNumber);
			fields.setField(OLD_ROOM_NUMBER, 0);
		}
	} else if ((room.cursorState() == CS_TALKING) ||
			   (res.getTalkState() != TALK_NONE)) {
		// Currently talking, so let its tick proc handle it
	} else if (mouse.y() < MENUBAR_Y_SIZE) {
		uint8 response = Menu::getReference().execute();
		if (response != MENUITEM_NONE)
			handleMenuResponse(response);
	} else if ((room.cursorState() == CS_SEQUENCE) ||
			   (room.cursorState() == CS_BUMPED)) {
		// No action necessary
	} else {
		if (mouse.lButton())
			handleLeftClick();
		else
			handleRightClickMenu();
	}
}

void Game::handleRightClickMenu() {
	Room &room = Room::getReference();
	Resources &res = Resources::getReference();
	Screen &screen = Screen::getReference();
	ValueTableData &fields = res.fieldList();
	StringList &stringList = res.stringList();
	StringData &strings = StringData::getReference();
	Mouse &mouse = Mouse::getReference();
	char *statusLine = room.statusLine();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	HotspotData *hotspot, *useHotspot;
	Action action;
	uint32 actions;
	uint16 itemId = 0xffff;
	bool hasItems;

	if (room.hotspotId() != 0) {
		// Get hotspot actions
		actions = room.hotspotActions();
	} else {
		// Standard actions - drink, examine, look, status
		actions = 0x1184000;
	}

	// If no inventory items remove entries that require them
	if (res.numInventoryItems() == 0)
		actions &= 0xFEF3F9FD;

	// If the player hasn't any money, remove any bribe entry
	if (res.fieldList().numGroats() == 0)
		actions &= 0xFF7FFFFF;

	action = NONE;
	hotspot = NULL;

	bool breakFlag = false;
	while (!breakFlag) {
		statusLine = room.statusLine();
		strcpy(statusLine, "");
		room.update();
		screen.update();

		action = PopupMenu::Show(actions);

		if (action != NONE) {
			sprintf(statusLine, "%s ", stringList.getString(action));
			statusLine += strlen(statusLine);
		}

		switch (action) {
		case LOOK:
		case STATUS:
			breakFlag = true;
			break;

		case ASK:
			hotspot = res.getHotspot(room.hotspotId());
			assert(hotspot);
			strings.getString(hotspot->nameId, statusLine);
			strcat(statusLine, stringList.getString(S_FOR));
			statusLine += strlen(statusLine);

			itemId = PopupMenu::ShowItems(GET, player->roomNumber());
			breakFlag = ((itemId != 0xffff) && (itemId != 0xfffe));
			break;

		case TELL:
			hotspot = res.getHotspot(room.hotspotId());
			assert(hotspot);
			strings.getString(hotspot->nameId, statusLine);
			strcat(statusLine, stringList.getString(S_TO));
			breakFlag = GetTellActions();
			break;

		case GIVE:
		case USE:
		case EXAMINE:
		case DRINK:
			hasItems = (res.numInventoryItems() != 0);
			if (!hasItems)
				strcat(statusLine, stringList.getString(S_ACTION_NOTHING));
			statusLine += strlen(statusLine);

			room.update();
			screen.update();
			mouse.waitForRelease();

			if (hasItems) {
				if (action != DRINK)
					hotspot = res.getHotspot(room.hotspotId());
				itemId = PopupMenu::ShowInventory();
				breakFlag = (itemId != 0xffff);
				if (breakFlag) {
					fields.setField(USE_HOTSPOT_ID, itemId);
					if ((action == GIVE) || (action == USE)) {
						// Add in the "X to " or "X on " section of give/use action
						useHotspot = res.getHotspot(itemId);
						assert(useHotspot);
						strings.getString(useHotspot->nameId, statusLine);
						if (action == GIVE)
							strcat(statusLine, stringList.getString(S_TO));
						else
							strcat(statusLine, stringList.getString(S_ON));
						statusLine += strlen(statusLine);
					}
					else if ((action == DRINK) || (action == EXAMINE))
						hotspot = res.getHotspot(itemId);
				}
			}
			break;

		default:
			hotspot = res.getHotspot(room.hotspotId());
			breakFlag = true;
			break;
		}
	}

	if (action != NONE) {
		player->stopWalking();

		if (hotspot == NULL) {
			doAction(action, 0, itemId);
		} else {
			if (action != TELL) {
				// Add the hotspot name to the status line and then go do the action
				if ((itemId != 0xffff) && (action != GIVE) && (action != USE)) {
					HotspotData *itemHotspot = res.getHotspot(itemId);
					if (itemHotspot != NULL)
						strings.getString(itemHotspot->nameId, statusLine);
				}
				else
					strings.getString(hotspot->nameId, statusLine);
			}

			doAction(action, hotspot->hotspotId, itemId);
		}
	} else {
		// Clear the status line
		strcpy(room.statusLine(), "");
	}
}

void Game::handleLeftClick() {
	Room &room = Room::getReference();
	Mouse &mouse = Mouse::getReference();
	Resources &res = Resources::getReference();
	StringData &strings = StringData::getReference();
	StringList &stringList = res.stringList();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);

	room.setCursorState(CS_NONE);
	player->stopWalking();
	player->setDestHotspot(0);
	player->setActionCtr(0);
	strcpy(room.statusLine(), "");

	if ((room.destRoomNumber() == 0) && (room.hotspotId() != 0)) {
		// Handle look at hotspot
		sprintf(room.statusLine(), "%s ", stringList.getString(LOOK_AT));
		HotspotData *hotspot = res.getHotspot(room.hotspotId());
		assert(hotspot);
		strings.getString(hotspot->nameId, room.statusLine() + strlen(room.statusLine()));
		doAction(LOOK_AT, room.hotspotId(), 0xffff);

	} else if (room.destRoomNumber() != 0) {
		// Walk to another room
		RoomExitCoordinateData &exitData =
			res.coordinateList().getEntry(room.roomNumber()).getData(room.destRoomNumber());

		player->walkTo((exitData.x & 0xfff8) | 5, (exitData.y & 0xfff8),
			room.hotspotId() == 0 ? 0xffff : room.hotspotId());
	} else {
		// Walking within room
		player->walkTo(mouse.x(), mouse.y(), 0);
	}
}

bool Game::GetTellActions() {
	Resources &res = Resources::getReference();
	Screen &screen = Screen::getReference();
	Room &room = Room::getReference();
	StringData &strings = StringData::getReference();
	StringList &stringList = res.stringList();
	char *statusLine = room.statusLine();
	uint16 *commands = &_tellCommands[1];
	char *statusLinePos[MAX_TELL_COMMANDS][4];
	int paramIndex = 0;
	uint16 selectionId;
	char selectionName[MAX_DESC_SIZE];
	HotspotData *hotspot;
	Action action;
	const char *continueStrsList[2] = {stringList.getString(S_AND_THEN), stringList.getString(S_FINISH)};

	// First word will be the destination character
	_tellCommands[0] = room.hotspotId();
	_numTellCommands = 0;

	// Set up a room transfer list
	Common::List<uint16> roomList;
	roomList.push_front(room.roomNumber());

	// Loop for getting tell commands

	while ((_numTellCommands >= 0) && (_numTellCommands < MAX_TELL_COMMANDS)) {

		// Loop for each sub-part of commands: Action, up to two params, and
		// a "and then" selection to allow for more commands

		while ((paramIndex >= 0) && (paramIndex <= 4)) {
			// Update status line
			statusLine += strlen(statusLine);
			statusLinePos[_numTellCommands][paramIndex] = statusLine;
			room.update();
			screen.update();

			switch (paramIndex) {
			case 0:
				// Prompt for selection of action to perform
				action = PopupMenu::Show(0x6A07FD);
				if (action == NONE) {
					// Move backwards to prior specified action
					--_numTellCommands;
					if (_numTellCommands < 0)
						paramIndex = -1;
					else {
						paramIndex = 3;
						statusLine = statusLinePos[_numTellCommands][paramIndex];
						*statusLine = '\0';
					}
					break;
				}

				// Add the action to the status line
				sprintf(statusLine + strlen(statusLine), "%s ", stringList.getString(action));

				// Handle any processing for the action
				commands[_numTellCommands * 3] = (uint16) action;
				commands[_numTellCommands * 3 + 1] = 0;
				commands[_numTellCommands * 3 + 2] = 0;
				++paramIndex;
				break;

			case 1:
				// First parameter
				action = (Action) commands[_numTellCommands * 3];
				if (action != RETURN) {
					// Prompt for selection
					if ((action != USE) && (action != DRINK) && (action != GIVE))
						selectionId = PopupMenu::ShowItems(action, *roomList.begin());
					else
						selectionId = PopupMenu::ShowItems(GET, *roomList.begin());

					if ((selectionId == 0xffff) || (selectionId == 0xfffe)) {
						// Move back to prompting for action
						--paramIndex;
						statusLine = statusLinePos[_numTellCommands][paramIndex];
						*statusLine = '\0';
						break;
					}

					if (selectionId < NOONE_ID) {
						// Must be a room selection
						strings.getString(selectionId, selectionName);
						roomList.push_front(selectionId);
					} else {
						hotspot = res.getHotspot(selectionId);
						assert(hotspot);
						strings.getString(hotspot->nameId, selectionName);
					}

					// Store selected entry
					commands[_numTellCommands * 3 + 1] = selectionId;
					strcat(statusLine, selectionName);
				}

				++paramIndex;
				break;

			case 2:
				// Second parameter
				action = (Action) commands[_numTellCommands * 3];
				if (action == ASK)
					strcat(statusLine, stringList.getString(S_FOR));
				else if (action == GIVE)
					strcat(statusLine, stringList.getString(S_TO));
				else if (action == USE)
					strcat(statusLine, stringList.getString(S_ON));
				else {
					// All other commads don't need a second parameter
					++paramIndex;
					break;
				}

				// Get the second parameter
				selectionId = PopupMenu::ShowItems(GET, *roomList.begin());
				if ((selectionId == 0xfffe) || (selectionId == 0xffff)) {
					--paramIndex;
					statusLine = statusLinePos[_numTellCommands][paramIndex];
					*statusLine = '\0';
				} else {
					// Display the second parameter
					hotspot = res.getHotspot(selectionId);
					assert(hotspot);
					strings.getString(hotspot->nameId, selectionName);
					strcat(statusLine, selectionName);

					commands[_numTellCommands * 3 + 2] = selectionId;
					++paramIndex;
				}
				break;

			case 3:
				// Prompting for "and then" for more commands
				if (_numTellCommands == MAX_TELL_COMMANDS - 1) {
					// No more commands allowed
					++_numTellCommands;
					paramIndex = -1;
				} else {
					// Only prompt if less than 8 commands entered
					selectionId = PopupMenu::Show(2, continueStrsList);

					switch (selectionId) {
					case 0:
						// Get ready for next command
						sprintf(statusLine + strlen(statusLine), " %s ", continueStrsList[0]);
						++_numTellCommands;
						paramIndex = 0;
						break;

					case 1:
						// Increment for just selected command, and add a large amount
						// to signal that the command sequence is complete
						_numTellCommands += 1 + 0x100;
						paramIndex = -1;
						break;

					default:
						// Move to end of just completed command
						action = (Action) commands[_numTellCommands * 3];
						if (action == RETURN)
							paramIndex = 0;
						else if ((action == ASK) || (action == GIVE) || (action == USE))
							paramIndex = 2;
						else {
							paramIndex = 1;
							if (action == GO_TO)
								// Remove top of the cached room change list
								roomList.erase(roomList.begin());
						}

						statusLine = statusLinePos[_numTellCommands][paramIndex];
						*statusLine = '\0';
					}
				}
			}
		}
	}

	bool result = (_numTellCommands != -1);
	if (result) {
		_numTellCommands &= 0xff;
		assert((_numTellCommands > 0) && (_numTellCommands <= MAX_TELL_COMMANDS));
		strcpy(statusLinePos[0][0], "..");
		room.update();
		screen.update();
	}

	return result;
}

void Game::doAction(Action action, uint16 hotspotId, uint16 usedId) {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	ValueTableData &fields = res.fieldList();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);

	fields.setField(CHARACTER_HOTSPOT_ID, PLAYER_ID);
	fields.setField(ACTIVE_HOTSPOT_ID, hotspotId);

	res.setCurrentAction(action);
	room.setCursorState(CS_ACTION);

	// Set the action
	if (action == TELL) {
		// Tell action needs special handling because of the variable length parameter list - add in a
		// placeholder entry, and then replace it's details with the TELL command data
		player->currentActions().addFront(NONE, player->roomNumber(), 0, 0);
		player->currentActions().top().supportData().setDetails2(TELL, _numTellCommands * 3 + 1, &_tellCommands[0]);
	} else if (action == USE)
		// Use action parameters are, for some reason, in reverse order from other 2 item actions
		player->currentActions().addFront(action, player->roomNumber(), usedId, hotspotId);
	else
		// All other action types
		player->currentActions().addFront(action, player->roomNumber(), hotspotId, usedId);
}

void Game::doShowCredits() {
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	Screen &screen = Screen::getReference();
	Room &room = Room::getReference();
	bool isEGA = LureEngine::getReference().isEGA();

	Sound.pause();
	mouse.cursorOff();

	Surface *s = Surface::getScreen(CREDITS_RESOURCE_ID);

	if (isEGA) {
		s->copyToScreen(0, 0);
	} else {
		Palette p(CREDITS_RESOURCE_ID - 1);
		screen.setPaletteEmpty();
		s->copyToScreen(0, 0);
		screen.setPalette(&p);
	}

	delete s;
	events.waitForPress();

	room.setRoomNumber(room.roomNumber());
	mouse.cursorOn();
	Sound.resume();
}

void Game::doQuit() {
	Sound.pause();
	if (getYN())
		LureEngine::getReference().quitGame();
	Sound.resume();
}

void Game::doRestart() {
	Sound.pause();
	if (getYN())
		setState(GS_RESTART);
	Sound.resume();
}

void Game::doTextSpeed() {
	Menu &menu = Menu::getReference();
	StringList &sl = Resources::getReference().stringList();

	_fastTextFlag = !_fastTextFlag;
	menu.getMenu(2).entries()[1] = sl.getString(_fastTextFlag ? S_FAST_TEXT : S_SLOW_TEXT);
}

void Game::doSound() {
	Menu &menu = Menu::getReference();
	StringList &sl = Resources::getReference().stringList();

	_soundFlag = !_soundFlag;
	menu.getMenu(2).entries()[2] = sl.getString(_soundFlag ? S_SOUND_ON : S_SOUND_OFF);

	if (!_soundFlag)
		// Stop all currently playing sounds
		Sound.killSounds();
}

void Game::handleBootParam(int value) {
	Resources &res = Resources::getReference();
	ValueTableData &fields = res.fieldList();
	Room &room = Room::getReference();
	Hotspot *h;

	switch (value) {
	case 0:
		// No parameter - load the first room
		room.setRoomNumber(1);
		break;

	case 1:
		// Set player to be in rack room with a few items
		// Setup Skorl in cell room
		h = res.getActiveHotspot(SKORL_ID);
		h->setRoomNumber(1);
		h->setPosition(140, 120);
		h->currentActions().top().setSupportData(0x1400);
		fields.setField(11, 1);

		// Set up player
		h = res.getActiveHotspot(PLAYER_ID);
		h->setRoomNumber(4);
		h->setPosition(150, 110);
		res.getHotspot(0x2710)->roomNumber = PLAYER_ID;  // Bottle
		res.getHotspot(0x2713)->roomNumber = PLAYER_ID;  // Knife

		room.setRoomNumber(4);
		break;

	case 2:
		// Set the player up in the outer cell with a full bottle & knife
		h = res.getActiveHotspot(PLAYER_ID);
		h->setRoomNumber(2);
		h->setPosition(100, 110);
		res.getHotspot(0x2710)->roomNumber = PLAYER_ID;  // Bottle
		fields.setField(BOTTLE_FILLED, 1);
		res.getHotspot(0x2713)->roomNumber = PLAYER_ID;  // Knife

		room.setRoomNumber(2);
		break;

	default:
		room.setRoomNumber(value);
		break;
	}
}

bool Game::getYN() {
	Mouse &mouse = Mouse::getReference();
	Events &events = Events::getReference();
	Screen &screen = Screen::getReference();
	Resources &res = Resources::getReference();
	LureEngine &engine = LureEngine::getReference();

	Common::Language l = LureEngine::getReference().getLanguage();
	Common::KeyCode y = Common::KEYCODE_y;
	if (l == Common::FR_FRA) y = Common::KEYCODE_o;
	else if ((l == Common::DE_DEU) || (l == Common::NL_NLD)) y = Common::KEYCODE_j;
	else if ((l == Common::ES_ESP) || (l == Common::IT_ITA)) y = Common::KEYCODE_s;

	bool vKbdFlag = g_system->hasFeature(OSystem::kFeatureVirtualKeyboard);
	if (!vKbdFlag)
		mouse.cursorOff();
	else
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, true);

	Surface *s = Surface::newDialog(190, res.stringList().getString(S_CONFIRM_YN));
	s->centerOnScreen();
	delete s;

	bool breakFlag = false;
	bool result = false;

	do {
		while (events.pollEvent()) {
			if (events.event().type == Common::EVENT_KEYDOWN) {
				Common::KeyCode key = events.event().kbd.keycode;
				if ((key == y) || (key == Common::KEYCODE_n) ||
					(key == Common::KEYCODE_ESCAPE)) {
					breakFlag = true;
					result = key == y;
				}
			}
			if (events.event().type == Common::EVENT_LBUTTONUP) {
				breakFlag = true;
				result = true;
			}
			if (events.event().type == Common::EVENT_RBUTTONUP) {
				breakFlag = true;
				result = false;
			}
		}

		g_system->delayMillis(10);
	} while (!engine.shouldQuit() && !breakFlag);

	screen.update();
	if (!vKbdFlag)
		mouse.cursorOn();
	else
		g_system->setFeatureState(OSystem::kFeatureVirtualKeyboard, false);

	return result;
}

bool Game::isMenuAvailable() {
	Resources &res = Resources::getReference();
	Room &room = Room::getReference();
	uint16 oldRoomNumber = res.fieldList().getField(OLD_ROOM_NUMBER);

	if (oldRoomNumber != 0)
		// Viewing a room remotely - so the menu isn't available
		return false;

	else if ((room.cursorState() == CS_TALKING) || (res.getTalkState() != TALK_NONE))
		return false;

	return true;
}

void Game::saveToStream(Common::WriteStream *stream) {
	stream->writeByte(_fastTextFlag);
	stream->writeByte(_soundFlag);
}

void Game::loadFromStream(Common::ReadStream *stream) {
	Menu &menu = Menu::getReference();
	StringList &sl = Resources::getReference().stringList();

	_fastTextFlag = stream->readByte() != 0;
	menu.getMenu(2).entries()[1] = sl.getString(_fastTextFlag ? S_FAST_TEXT : S_SLOW_TEXT);

	_soundFlag = stream->readByte() != 0;
	menu.getMenu(2).entries()[2] = sl.getString(_soundFlag ? S_SOUND_ON : S_SOUND_OFF);

	// Reset game state flags
	setState(0);
}


} // End of namespace Lure
