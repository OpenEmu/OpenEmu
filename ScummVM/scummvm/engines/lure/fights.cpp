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

#include "lure/fights.h"
#include "lure/luredefs.h"
#include "lure/game.h"
#include "lure/lure.h"
#include "lure/res.h"
#include "lure/room.h"
#include "lure/sound.h"

namespace Lure {

// Three records containing initial states for player, pig, and Skorl
const FighterRecord initialFighterList[3] = {
	{0x23C, 0x440, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0},
	{0, 0x441, 0x1092, 0, 3, 0, 0, 0, 0xB94, 8, 0xA34, 0x8D4, 0xD06, 0,
	0, 0, 0, 0, 0xDDC, PLAYER_ID},
	{0, 0x446, 0x1092, 0, 3, 0, 0, 0, 0xB94, 8, 0xA34, 0x8D4, 0xD06, 0,
	0, 0, 0, 0, 0xDDC, PLAYER_ID}
};

FightsManager *int_fights = NULL;

FightsManager::FightsManager() : _rnd(LureEngine::getReference().rnd()) {
	int_fights = this;
	_fightData = NULL;
	_mouseFlags = 0;
	_keyDown = KS_UP;
	reset();
}

FightsManager::~FightsManager() {
	// Release the fight data
	delete _fightData;
}

FightsManager &FightsManager::getReference() {
	return *int_fights;
}

FighterRecord &FightsManager::getDetails(uint16 hotspotId) {
	if (hotspotId == PLAYER_ID) return _fighterList[0];
	else if (hotspotId == PIG_ID) return _fighterList[1];
	else if (hotspotId == SKORL_FIGHTER_ID) return _fighterList[2];
	error("Unknown NPC %d attempted to fight", hotspotId);
}

// Sets up the data for the pig fight in the cave

void FightsManager::setupPigFight() {
	Resources &res = Resources::getReference();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	player->setSkipFlag(false);
	player->resource()->colorOffset = 16;
	player->setTickProc(PLAYER_FIGHT_TICK_PROC_ID);
	player->setSize(48, 53);
	player->setAnimationIndex(PLAYER_FIGHT_ANIM_INDEX);
	player->resource()->width = 48;
	player->resource()->height = 53;

	player->setOccupied(false);
	player->setPosition(262, 94);
	FighterRecord &rec = getDetails(PLAYER_ID);
	rec.fwhits = 0;
	rec.fwtrue_x = 262;
	rec.fwtrue_y = 53;
	rec.fwseq_ad = FIGHT_PLAYER_INIT;
	rec.fwenemy_ad = PIG_ID;
}

void FightsManager::setupSkorlFight() {
	Resources &res = Resources::getReference();
	Hotspot *player = res.getActiveHotspot(PLAYER_ID);
	FighterRecord &rec = getDetails(PLAYER_ID);

	setupPigFight();

	rec.fwenemy_ad = SKORL_FIGHTER_ID;
	rec.fwweapon = 0x445;
	rec.fwtrue_x = 282;
	rec.fwtrue_y = 136;
	player->setPosition(282, 136);
	player->resource()->colorOffset = 96;
}

bool FightsManager::isFighting() {
	FighterRecord &rec = getDetails(PLAYER_ID);
	return rec.fwhits == 0;
}

void FightsManager::fightLoop() {
	LureEngine &engine = LureEngine::getReference();
	Resources &res = Resources::getReference();
	Game &game = Game::getReference();
	Room &room = Room::getReference();
	FighterRecord &playerFight = getDetails(PLAYER_ID);
	uint32 timerVal = g_system->getMillis();

	// Loop for the duration of the battle
	while (!engine.shouldQuit() && (playerFight.fwhits != GENERAL_MAGIC_ID)) {
		checkEvents();

		if (g_system->getMillis() > timerVal + GAME_FRAME_DELAY) {
			timerVal = g_system->getMillis();

			game.tick();
			room.update();
			res.delayList().tick();
		}

		Screen::getReference().update();
		game.debugger().onFrame();

		g_system->delayMillis(10);
	}
}

void FightsManager::saveToStream(Common::WriteStream *stream) {
	for (int fighterCtr = 0; fighterCtr < 3; ++fighterCtr) {
		FighterRecord &rec = _fighterList[fighterCtr];

		stream->writeUint16LE(rec.fwseq_no);
		stream->writeUint16LE(rec.fwseq_ad);
		stream->writeUint16LE(rec.fwdist);
		stream->writeUint16LE(rec.fwwalk_roll);
		stream->writeUint16LE(rec.fwmove_number);
		stream->writeUint16LE(rec.fwhits);
	}
}

void FightsManager::loadFromStream(Common::ReadStream *stream) {
	reset();

	for (int fighterCtr = 0; fighterCtr < 3; ++fighterCtr) {
		FighterRecord &rec = _fighterList[fighterCtr];

		rec.fwseq_no = stream->readUint16LE();
		rec.fwseq_ad = stream->readUint16LE();
		rec.fwdist = stream->readUint16LE();
		rec.fwwalk_roll = stream->readUint16LE();
		rec.fwmove_number = stream->readUint16LE();
		rec.fwhits = stream->readUint16LE();
	}
}

void FightsManager::reset() {
	_fighterList[0] = initialFighterList[0];
	_fighterList[1] = initialFighterList[1];
	_fighterList[2] = initialFighterList[2];
}

const CursorType moveList[] = {CURSOR_LEFT_ARROW, CURSOR_FIGHT_UPPER,
	CURSOR_FIGHT_MIDDLE, CURSOR_FIGHT_LOWER, CURSOR_RIGHT_ARROW};

struct KeyMapping {
	Common::KeyCode keycode;
	uint8 moveNumber;
};

const KeyMapping keyList[] = {
	{Common::KEYCODE_LEFT, 10}, {Common::KEYCODE_RIGHT, 14},
	{Common::KEYCODE_KP7, 11}, {Common::KEYCODE_KP4, 12}, {Common::KEYCODE_KP1, 13},
	{Common::KEYCODE_KP9, 6},  {Common::KEYCODE_KP6, 7},  {Common::KEYCODE_KP3, 8},
	{Common::KEYCODE_INVALID, 0}};

void FightsManager::checkEvents() {
	LureEngine &engine = LureEngine::getReference();
	Game &game = Game::getReference();
	Events &events = Events::getReference();
	Mouse &mouse = Mouse::getReference();
	FighterRecord &rec = getDetails(PLAYER_ID);
	Hotspot *player = Resources::getReference().getActiveHotspot(PLAYER_ID);
	int moveNumber = 0;

	while ((moveNumber == 0) && events.pollEvent()) {

		if (events.type() == Common::EVENT_KEYDOWN) {
			switch (events.event().kbd.keycode) {
			case Common::KEYCODE_ESCAPE:
				engine.quitGame();
				return;

			case Common::KEYCODE_d:
				if (events.event().kbd.hasFlags(Common::KBD_CTRL)) {
					// Activate the debugger
					game.debugger().attach();
					return;
				}
				break;

			default:
				// Scan through the mapping list for a move for the keypress
				const KeyMapping *keyPtr = &keyList[0];
				while ((keyPtr->keycode != Common::KEYCODE_INVALID) &&
					(keyPtr->keycode != events.event().kbd.keycode))
					++keyPtr;
				if (keyPtr->keycode != Common::KEYCODE_INVALID) {
					moveNumber = keyPtr->moveNumber;
					_keyDown = KS_KEYDOWN_1;
				}
			}

		} else if (events.type() == Common::EVENT_KEYUP) {
			_keyDown = KS_UP;

		} else if (events.type() == Common::EVENT_MOUSEMOVE) {
			Common::Point mPos = events.event().mouse;
			if (mPos.x < rec.fwtrue_x - 12)
				mouse.setCursorNum(CURSOR_LEFT_ARROW);
			else if (mPos.x > rec.fwtrue_x + player->width())
				mouse.setCursorNum(CURSOR_RIGHT_ARROW);
			else if (mPos.y < player->y() + 4)
				mouse.setCursorNum(CURSOR_FIGHT_UPPER);
			else if (mPos.y < player->y() + 38)
				mouse.setCursorNum(CURSOR_FIGHT_MIDDLE);
			else
				mouse.setCursorNum(CURSOR_FIGHT_LOWER);

		} else if ((events.type() == Common::EVENT_LBUTTONDOWN) ||
				(events.type() == Common::EVENT_RBUTTONDOWN) ||
				(events.type() == Common::EVENT_LBUTTONUP) ||
				(events.type() == Common::EVENT_RBUTTONUP)) {
			_mouseFlags = 0;
			if (events.type() == Common::EVENT_LBUTTONDOWN) ++_mouseFlags;
			if (events.type() == Common::EVENT_RBUTTONDOWN) _mouseFlags += 2;
		}
	}

	if (_keyDown == KS_KEYDOWN_2) return;

	// Get the correct base index for the move
	while ((moveNumber < 5) && (moveList[moveNumber] != mouse.getCursorNum()))
		++moveNumber;
	if (moveNumber < 5) {
		if (_mouseFlags == 1)
			// Left mouse
			moveNumber += 10;
		else if (_mouseFlags == 2)
			// Right mouse
			moveNumber += 5;
	}

	rec.fwmove_number = moveNumber;

	if (_keyDown == KS_KEYDOWN_1)
		_keyDown = KS_KEYDOWN_2;

	if (rec.fwmove_number >= 5)
		debugC(ERROR_INTERMEDIATE, kLureDebugFights,
			"Player fight move number=%d", rec.fwmove_number);
}

void FightsManager::fighterAnimHandler(Hotspot &h) {
	FighterRecord &fighter = getDetails(h.hotspotId());
	FighterRecord &opponent = getDetails(fighter.fwenemy_ad);
	FighterRecord &player = getDetails(PLAYER_ID);

	fetchFighterDistance(fighter, opponent);

	if (fighter.fwseq_ad) {
		fightHandler(h, fighter.fwseq_ad);
		return;
	}

	uint16 seqNum = 0;
	if (fighter.fwdist != FIGHT_DISTANCE) {
		seqNum = getFighterMove(fighter, fighter.fwnot_near);
	} else {
		uint16 offset = (fighter.fwhits * fighter.fwdef_len) + fighter.fwdefend_adds + 4;

		// Scan for the given sequence
		uint16 v = getWord(offset);
		while ((v != 0) && (v != player.fwseq_no)) {
			offset += 4;
			v = getWord(offset);
		}

		if (v == 0) {
			// No sequence match found
			seqNum = getFighterMove(fighter, fighter.fwattack_table);
		} else {
			v = getWord(offset + 2);
			seqNum  = getFighterMove(fighter, fighter.fwdefend_table);

			if (seqNum == 0)
				seqNum = getFighterMove(fighter, fighter.fwattack_table);
			else if (seqNum == 0xff)
				seqNum = v;
		}
	}

	// Set the sequence and pass onto the fight handler
	fighter.fwseq_no = seqNum;
	fighter.fwseq_ad = getWord(FIGHT_TBL_1 + (seqNum << 1));
}

void FightsManager::playerAnimHandler(Hotspot &h) {
	FighterRecord &fighter = getDetails(h.hotspotId());
	fightHandler(h, fighter.fwseq_ad);
}

void FightsManager::fightHandler(Hotspot &h, uint16 moveOffset) {
	Resources &res = Resources::getReference();
	FighterRecord &fighter = getDetails(h.hotspotId());
	FighterRecord &opponent = getDetails(fighter.fwenemy_ad);

	uint16 v1, v2;
	bool breakFlag = false;

	while (!breakFlag) {
		if (moveOffset == 0) {
			// Player is doing nothing, so check the move number
			moveOffset = getWord(FIGHT_PLAYER_MOVE_TABLE + (fighter.fwmove_number << 1));

			debugC(ERROR_DETAILED, kLureDebugFights,
				"Hotspot %xh fight move=%d, new offset=%xh",
				h.hotspotId(), fighter.fwmove_number, moveOffset);

			if (moveOffset == 0)
				return;

			fighter.fwseq_no = fighter.fwmove_number;
			fighter.fwseq_ad = moveOffset;
		}

		uint16 moveValue = getWord(moveOffset);
		debugC(ERROR_DETAILED, kLureDebugFights,
			"Hotspot %xh script offset=%xh value=%xh",
			h.hotspotId(), moveOffset, moveValue);
		moveOffset += sizeof(uint16);

		if ((moveValue & 0x8000) == 0) {
			// Set frame to specified number
			h.setFrameNumber(moveValue);

			// Set the new fighter position
			int16 newX, newY;
			newX = h.x() + (int16)getWord(moveOffset);
			if (newX < 32) newX = 32;
			if (newX > 240) newX = 240;
			newY = h.y() + (int16)getWord(moveOffset + 2);
			h.setPosition(newX, newY);

			if (fighter.fwweapon != 0) {
				Hotspot *weaponHotspot = res.getActiveHotspot(fighter.fwweapon);
				assert(weaponHotspot);

				uint16 newFrameNumber = getWord(moveOffset + 4);
				int16 xChange = (int16)getWord(moveOffset + 6);
				int16 yChange = (int16)getWord(moveOffset + 8);

				weaponHotspot->setFrameNumber(newFrameNumber);
				weaponHotspot->setPosition(h.x() + xChange, h.y() + yChange);
			}

			moveOffset += 5 * sizeof(uint16);
			fighter.fwseq_ad = moveOffset;
			return;
		}

		switch (moveValue) {
		case 0xFFFA:
			// Walk left
			if ((fighter.fwmove_number == 5) || (fighter.fwmove_number == 10)) {
				if (h.x() < 32) {
					breakFlag = true;
				} else {
					h.setPosition(h.x() - 4, h.y());
					fighter.fwtrue_x = h.x();
					if (fetchFighterDistance(fighter, opponent) < FIGHT_DISTANCE) {
						h.setPosition(h.x() + 4, h.y());
						fighter.fwtrue_x += 4;
						breakFlag = true;
					} else {
						removeWeapon(fighter.fwweapon);
						fighter.fwtrue_x = h.x();
						fighter.fwtrue_y = h.y();

						uint16 frameNum = (fighter.fwwalk_roll == 7) ? 0 :
							fighter.fwwalk_roll + 1;
						fighter.fwwalk_roll = frameNum;
						fighter.fwseq_ad = moveOffset;
						h.setFrameNumber(frameNum);
						return;
					}
				}
			} else {
				// Signal to start a new action
				moveOffset = 0;
			}
			break;

		case 0xFFF9:
			// Walk right
			if ((fighter.fwmove_number == 9) || (fighter.fwmove_number == 14)) {
				if (h.x() >= 240) {
					breakFlag = true;
				} else {
					removeWeapon(fighter.fwweapon);
					h.setPosition(h.x() + 4, h.y());
					fighter.fwtrue_x = h.x();
					fighter.fwtrue_y = h.y();

					fighter.fwwalk_roll = (fighter.fwwalk_roll == 0) ? 7 :
						fighter.fwwalk_roll - 1;
					fighter.fwseq_ad = moveOffset;
					h.setFrameNumber(fighter.fwwalk_roll);
					return;
				}

			} else {
				// Signal to start a new action
				moveOffset = 0;
			}
			break;

		case 0xFFEB:
			// Enemy right
			removeWeapon(fighter.fwweapon);
			h.setPosition(h.x() + 4, h.y());
			fighter.fwtrue_x = h.x();

			if (fetchFighterDistance(fighter, opponent) < FIGHT_DISTANCE) {
				h.setPosition(h.x() - 4, h.y());
				fighter.fwtrue_x -= 4;
				fighter.fwseq_ad = 0;
				h.setFrameNumber(8);
			} else {
				h.setFrameNumber(getWord(moveOffset));
				moveOffset += sizeof(uint16);
				fighter.fwseq_ad = moveOffset;
			}
			return;

		case 0xFFFB:
			// End of sequence
			breakFlag = true;
			break;

		case 0xFFF8:
			// Set fight address
			moveOffset = getWord(moveOffset);
			break;

		case 0xFFFF:
		case 0xFFFE:
			if (moveValue == 0xffff)
				// Set the animation record
				h.setAnimation(getWord(moveOffset));
			else
				// New set animation record
				h.setAnimation(getWord(fighter.fwheader_list + (getWord(moveOffset) << 1)));
			h.setFrameNumber(0);
			moveOffset += sizeof(uint16);
			break;

		case 0xFFF7:
			// On hold
			if (getWord(moveOffset) == fighter.fwmove_number)
				moveOffset = getWord(moveOffset + 2);
			else
				moveOffset += 2 * sizeof(uint16);
			break;

		case 0xFFF6:
			// Not hold
			if (getWord(moveOffset) == fighter.fwmove_number)
				moveOffset += 2 * sizeof(uint16);
			else
				moveOffset = getWord(moveOffset + 2);
			break;

		case 0xFFF4:
			// End sequence
			fighter.fwseq_no = 0;
			break;

		case 0xFFF2:
			// Set defend
			fighter.fwblocking = getWord(moveOffset);
			moveOffset += sizeof(uint16);
			break;

		case 0xFFF1:
			// If blocking
			v1 = getWord(moveOffset);
			v2 = getWord(moveOffset + 2);
			moveOffset += 2 * sizeof(uint16);

			if (v1 == opponent.fwblocking) {
				Sound.addSound(42);
				moveOffset = v2;
			}
			break;

		case 0xFFF0:
			// Check hit
			v1 = getWord(moveOffset);
			moveOffset += sizeof(uint16);
			if (fighter.fwdist <= FIGHT_DISTANCE) {
				if (h.hotspotId() == PLAYER_ID) {
					// Player hits opponent
					Sound.addSound(52);

					if (opponent.fwhits != 5) {
						opponent.fwseq_ad = v1;
						if (++opponent.fwhit_value != opponent.fwhit_rate) {
							opponent.fwhit_value = 0;
							if (++opponent.fwhits == 5)
								opponent.fwseq_ad = opponent.fwdie_seq;
						}
					}
				} else {
					// Opponent hit player
					Sound.addSound(37);
					opponent.fwseq_ad = v1;
					if (++opponent.fwhits == 10) {
						// Player has been killed
						fighter.fwhits = 10;
						opponent.fwseq_ad = FIGHT_PLAYER_DIES;
						Sound.addSound(36);
					}
				}
			}
			break;

		case 0xFFEF:
			// Save co-ordinates
			fighter.fwtrue_x = h.x();
			fighter.fwtrue_y = h.y();
			break;

		case 0xFFEE:
			// Restore co-ordinates
			h.setPosition(fighter.fwtrue_x, fighter.fwtrue_y);
			break;

		case 0xFFED:
			// End of game
			getDetails(PLAYER_ID).fwhits = GENERAL_MAGIC_ID;
			Game::getReference().setState(GS_RESTORE_RESTART);
			return;

		case 0xFFEC:
			// Enemy has been killed
			enemyKilled();
			break;

		case 0xFFEA:
			// Fight sound
			Sound.addSound(getWord(moveOffset) & 0xff);
			moveOffset += sizeof(uint16);
			break;

		default:
			error("Unknown fight command %xh", moveValue);
		}
	}

	fighter.fwseq_no = 0;
	fighter.fwseq_ad = 0;
	if (h.hotspotId() == PLAYER_ID)
		_mouseFlags = 0;
}

uint16 FightsManager::fetchFighterDistance(FighterRecord &f1, FighterRecord &f2) {
	f1.fwdist = ABS(f1.fwtrue_x - f2.fwtrue_x) & 0xFFF8;
	return f1.fwdist;
}

void FightsManager::enemyKilled() {
	Resources &res = Resources::getReference();
	Hotspot *playerHotspot = res.getActiveHotspot(PLAYER_ID);
	FighterRecord &playerRec = getDetails(PLAYER_ID);

	playerHotspot->setTickProc(PLAYER_TICK_PROC_ID);
	playerRec.fwhits = GENERAL_MAGIC_ID;
	playerHotspot->resource()->colorOffset = 128;
	playerHotspot->setSize(32, 48);
	playerHotspot->resource()->width = 32;
	playerHotspot->resource()->height = 48;
	playerHotspot->setAnimationIndex(PLAYER_ANIM_INDEX);
	playerHotspot->setPosition(playerHotspot->x(), playerHotspot->y() + 5);
	playerHotspot->setDirection(LEFT);

	if (playerHotspot->roomNumber() == 6) {
		Dialog::show(0xc9f);
		HotspotData *axeHotspot = res.getHotspot(0x2738);
		axeHotspot->roomNumber = PLAYER_ID;
		axeHotspot->flags |= HOTSPOTFLAG_FOUND;

		// Prevent the weapon animation being drawn
		axeHotspot = res.getHotspot(0x440);
		axeHotspot->layer = 0;
	}
}

// Makes sure the given weapon is off-screen
void FightsManager::removeWeapon(uint16 weaponId) {
	Hotspot *weaponHotspot = Resources::getReference().getActiveHotspot(weaponId);
	weaponHotspot->setPosition(-32, -32);
}

uint16 FightsManager::getFighterMove(FighterRecord &rec, uint16 baseOffset) {
	int actionIndex = _rnd.getRandomNumber(31);
	return getByte(baseOffset + (rec.fwhits << 5) + actionIndex);
}

} // End of namespace Lure
