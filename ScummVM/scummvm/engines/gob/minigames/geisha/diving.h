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

#ifndef GOB_MINIGAMES_GEISHA_DIVING_H
#define GOB_MINIGAMES_GEISHA_DIVING_H

#include "common/system.h"

#include "gob/util.h"

#include "gob/sound/sounddesc.h"

namespace Gob {

class GobEngine;
class Surface;
class DECFile;
class ANIFile;
class ANIObject;

namespace Geisha {

class EvilFish;
class Oko;
class Meter;

/** Geisha's "Diving" minigame. */
class Diving {
public:
	Diving(GobEngine *vm);
	~Diving();

	bool play(uint16 playerCount, bool hasPearlLocation);

	bool isPlaying() const;
	void cheatWin();

private:
	static const uint kEvilFishCount  =  3;
	static const uint kDecorFishCount =  3;
	static const uint kMaxShotCount   = 10;

	static const uint   kEvilFishTypeCount = 3;
	static const uint16 kEvilFishTypes[kEvilFishTypeCount][5];

	struct PlantLevel {
		int16 y;
		uint plantCount;
		const uint16 *plants;
	};

	static const uint kPlantLevelCount    = 3;
	static const uint kPlantPerLevelCount = 5;

	static const uint16 kPlantLevel1[];
	static const uint16 kPlantLevel2[];
	static const uint16 kPlantLevel3[];

	static const PlantLevel kPlantLevels[kPlantLevelCount];

	static const uint kPlantCount = kPlantLevelCount * kPlantPerLevelCount;

	struct ManagedEvilFish {
		EvilFish *evilFish;

		uint32 enterAt;
		uint32 leaveAt;
	};

	struct ManagedDecorFish {
		ANIObject *decorFish;

		uint32 enterAt;
		int8 deltaX;
	};

	struct ManagedPlant {
		ANIObject *plant;

		uint level;
		int8 deltaX;
		int16 x, y;
	};

	struct ManagedPearl {
		ANIObject *pearl;

		bool picked;
		bool black;
	};

	GobEngine *_vm;

	DECFile *_background;
	ANIFile *_objects;
	ANIFile *_gui;
	ANIFile *_okoAnim;

	ANIObject *_water;
	ANIObject *_lungs;
	ANIObject *_heart;

	ManagedEvilFish  _evilFish[kEvilFishCount];
	ManagedDecorFish _decorFish[kDecorFishCount];
	ManagedPlant     _plant[kPlantCount];
	ManagedPearl     _pearl;

	Oko *_oko;

	ANIObject *_shot[kMaxShotCount];

	Common::List<int> _activeShots;

	Common::List<ANIObject *> _anims;

	Surface *_blackPearl;

	uint8 _whitePearlCount;
	uint8 _blackPearlCount;

	Meter *_airMeter;
	Meter *_healthMeter;

	uint8 _airCycle;
	uint8 _hurtGracePeriod;

	uint8 _currentShot;

	SoundDesc _soundShoot;
	SoundDesc _soundBreathe;
	SoundDesc _soundWhitePearl;
	SoundDesc _soundBlackPearl;

	bool _hasPearlLocation;
	bool _isPlaying;


	void init();
	void deinit();

	void initScreen();
	void initCursor();
	void initPlants();

	void enterPlant(ManagedPlant &plant, int16 prevPlantX);
	void enterPearl(int16 x);

	void getPearl();

	void foundBlackPearl();
	void foundWhitePearl();

	void updateAirMeter();
	void updateEvilFish();
	void updateDecorFish();
	void updatePlants();
	void updatePearl();
	void updateAnims();

	int16 checkInput(int16 &mouseX, int16 &mouseY, MouseButtons &mouseButtons);

	void shoot(int16 mouseX, int16 mouseY);
	void checkShots();

	void handleOko(int16 key);

	void checkOkoHurt();
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_DIVING_H
