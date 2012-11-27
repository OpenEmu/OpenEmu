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

#ifndef GOB_MINIGAMES_GEISHA_PENETRATION_H
#define GOB_MINIGAMES_GEISHA_PENETRATION_H

#include "common/system.h"
#include "common/list.h"

#include "gob/sound/sounddesc.h"

#include "gob/minigames/geisha/submarine.h"

namespace Gob {

class GobEngine;
class Surface;
class CMPFile;
class ANIFile;

namespace Geisha {

class Meter;
class Mouth;

/** Geisha's "Penetration" minigame. */
class Penetration {
public:
	Penetration(GobEngine *vm);
	~Penetration();

	bool play(bool hasAccessPass, bool hasMaxEnergy, bool testMode);

	bool isPlaying() const;
	void cheatWin();

private:
	static const int kModeCount  = 2;
	static const int kFloorCount = 3;

	static const int kMapWidth  = 17;
	static const int kMapHeight = 13;

	static const int kPaletteSize = 16;

	static const byte kPalettes[kFloorCount][3 * kPaletteSize];
	static const byte kMaps[kModeCount][kFloorCount][kMapWidth * kMapHeight];

	static const int kEnemyCount     =  9;
	static const int kMaxBulletCount = 10;

	struct MapObject {
		uint16 tileX;
		uint16 tileY;

		uint16 mapX;
		uint16 mapY;

		uint16 width;
		uint16 height;

		bool isBlocking;

		MapObject(uint16 tX, uint16 tY, uint16 mX, uint16 mY, uint16 w, uint16 h);
		MapObject(uint16 tX, uint16 tY, uint16 w, uint16 h);

		void setTileFromMapPosition();
		void setMapFromTilePosition();

		bool isIn(uint16 mX, uint16 mY) const;
		bool isIn(uint16 mX, uint16 mY, uint16 w, uint16 h) const;
		bool isIn(const MapObject &obj) const;
	};

	enum MouthType {
		kMouthTypeBite,
		kMouthTypeKiss
	};

	struct ManagedMouth : public MapObject {
		Mouth *mouth;

		MouthType type;

		ManagedMouth(uint16 tX, uint16 tY, MouthType t);
		~ManagedMouth();
	};

	struct ManagedSub : public MapObject {
		Submarine *sub;

		ManagedSub(uint16 tX, uint16 tY);
		~ManagedSub();
	};

	struct ManagedEnemy : public MapObject {
		ANIObject *enemy;

		bool dead;

		ManagedEnemy();
		~ManagedEnemy();

		void clear();
	};

	struct ManagedBullet : public MapObject {
		ANIObject *bullet;

		int16 deltaX;
		int16 deltaY;

		ManagedBullet();
		~ManagedBullet();

		void clear();
	};

	enum Keys {
		kKeyUp = 0,
		kKeyDown,
		kKeyLeft,
		kKeyRight,
		kKeySpace,
		kKeyCount
	};

	GobEngine *_vm;

	bool _hasAccessPass;
	bool _hasMaxEnergy;
	bool _testMode;

	bool _needFadeIn;

	bool _quit;
	bool _keys[kKeyCount];

	Surface *_background;
	CMPFile *_sprites;
	ANIFile *_objects;

	Common::List<ANIObject *> _anims;
	Common::List<ANIObject *> _mapAnims;

	Meter *_shieldMeter;
	Meter *_healthMeter;

	uint8 _floor;

	Surface *_map;

	ManagedSub *_sub;

	Common::List<MapObject>    _walls;
	Common::List<MapObject>    _exits;
	Common::List<MapObject>    _shields;
	Common::List<ManagedMouth> _mouths;

	ManagedEnemy  _enemies[kEnemyCount];
	ManagedBullet _bullets[kMaxBulletCount];

	Common::List<MapObject *> _blockingObjects;

	uint8 _shotCoolDown;

	SoundDesc _soundShield;
	SoundDesc _soundBite;
	SoundDesc _soundKiss;
	SoundDesc _soundShoot;
	SoundDesc _soundExit;
	SoundDesc _soundExplode;

	bool _isPlaying;


	void init();
	void deinit();

	void clearMap();
	void createMap();

	void initScreen();

	void setPalette();
	void fadeIn();

	void drawFloorText();
	void drawEndText();

	bool isBlocked(const MapObject &self, int16 x, int16 y, MapObject **blockedBy = 0);
	void findPath(MapObject &obj, int x, int y, MapObject **blockedBy = 0);

	void updateAnims();

	void checkInput();

	Submarine::Direction getDirection(int &x, int &y) const;

	void handleSub();
	void subMove(int x, int y, Submarine::Direction direction);
	void subShoot();

	int findEmptyBulletSlot() const;
	uint16 directionToBullet(Submarine::Direction direction) const;
	void setBulletPosition(const ManagedSub &sub, ManagedBullet &bullet) const;

	void bulletsMove();
	void bulletMove(ManagedBullet &bullet);
	void checkShotEnemy(MapObject &shotObject);

	void checkExits();
	void checkShields();
	void checkMouths();

	void healthGain(int amount);
	void healthLose(int amount);

	void checkExited();

	void enemiesCreate();
	void enemiesMove();
	void enemyMove(ManagedEnemy &enemy, int x, int y);
	void enemyAttack(ManagedEnemy &enemy);
	void enemyExplode(ManagedEnemy &enemy);

	bool isDead() const;
	bool hasWon() const;

	int getLanguage() const;
};

} // End of namespace Geisha

} // End of namespace Gob

#endif // GOB_MINIGAMES_GEISHA_PENETRATION_H
