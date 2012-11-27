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

#ifndef LURE_FIGHT_H
#define LURE_FIGHT_H

#include "lure/luredefs.h"
#include "lure/hotspots.h"
#include "lure/palette.h"

#include "common/singleton.h"
#include "common/endian.h"
#include "common/random.h"

namespace Lure {

struct FighterRecord {
	uint16 fwheader_list;
	uint16 fwweapon;
	uint16 fwdie_seq;
	uint16 fwhit_value;
	uint16 fwhit_rate;
	int16 fwtrue_x;
	int16 fwtrue_y;
	uint16 fwblocking;
	uint16 fwattack_table;
	uint16 fwdef_len;
	uint16 fwdefend_table;
	uint16 fwnot_near;
	uint16 fwdefend_adds;
	uint16 fwseq_no;
	uint16 fwdist;
	uint16 fwwalk_roll;
	uint16 fwmove_number;
	uint16 fwhits;
	uint16 fwseq_ad;
	uint16 fwenemy_ad;
};

// Constant references into the fight data
#define FIGHT_TBL_1 0x8b8
#define FIGHT_PLAYER_MOVE_TABLE 0xDAA
#define FIGHT_PLAYER_INIT 0xDC8
#define FIGHT_PLAYER_DIES 0xF46

#define FIGHT_DISTANCE 32

enum KeyStatus {KS_UP, KS_KEYDOWN_1, KS_KEYDOWN_2};

class FightsManager {
private:
	MemoryBlock *_fightData;
	Common::RandomSource &_rnd;
	uint8 _mouseFlags;
	KeyStatus _keyDown;
	FighterRecord _fighterList[3];

	FighterRecord &getDetails(uint16 hotspotId);
	uint16 fetchFighterDistance(FighterRecord &f1, FighterRecord &f2);
	void removeWeapon(uint16 weaponId);
	void enemyKilled();
	uint16 getFighterMove(FighterRecord &rec, uint16 baseOffset);
	void checkEvents();
	void fightHandler(Hotspot &h, uint16 moveOffset);

	inline uint16 getWord(uint16 offset) {
		if (!_fightData)
			_fightData = Disk::getReference().getEntry(FIGHT_DATA_RESOURCE_ID);
		if (offset >= _fightData->size() - 1) error("Invalid fight data index");
		return READ_LE_UINT16(_fightData->data() + offset);
	}
	inline uint8 getByte(uint16 offset) {
		if (!_fightData)
			_fightData = Disk::getReference().getEntry(FIGHT_DATA_RESOURCE_ID);
		if (offset >= _fightData->size()) error("Invalid fight data index");
		return _fightData->data()[offset];
	}
public:
	FightsManager();
	~FightsManager();
	static FightsManager &getReference();

	void setupPigFight();
	void setupSkorlFight();
	bool isFighting();
	void fightLoop();
	void saveToStream(Common::WriteStream *stream);
	void loadFromStream(Common::ReadStream *stream);
	void reset();

	void fighterAnimHandler(Hotspot &h);
	void playerAnimHandler(Hotspot &h);
};

#define Fights FightsManager::getReference()

} // End of namespace Lure

#endif
