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

#ifndef TSAGE_H
#define TSAGE_H

#include "engines/engine.h"
#include "common/rect.h"
#include "audio/mixer.h"
#include "common/file.h"

#include "tsage/core.h"
#include "tsage/resources.h"
#include "tsage/debugger.h"
#include "tsage/events.h"
#include "tsage/graphics.h"
#include "tsage/resources.h"


namespace TsAGE {

enum {
	GType_Ringworld = 0,
	GType_BlueForce = 1,
	GType_Ringworld2 = 2
};

enum {
	GF_DEMO = 1 << 0,
	GF_CD = 1 << 1,
	GF_FLOPPY = 1 << 2,
	GF_ALT_REGIONS = 1 << 3
};

enum {
	kRingDebugScripts = 1 << 0,
	ktSageSound = 1 << 1,
	ktSageCore = 1 << 2,
	ktSageDebugGraphics = 1 << 3
};

struct tSageGameDescription;

#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 200
#define SCREEN_CENTER_X 160
#define SCREEN_CENTER_Y 100
#define UI_INTERFACE_Y 168

class TSageEngine : public Engine {
private:
	const tSageGameDescription *_gameDescription;
public:
	TSageEngine(OSystem *system, const tSageGameDescription *gameDesc);
	~TSageEngine();
	virtual bool hasFeature(EngineFeature f) const;

	MemoryManager _memoryManager;
	Debugger *_debugger;

	const char *getGameId() const;
	uint32 getGameID() const;
	uint32 getFeatures() const;
	Common::String getPrimaryFilename() const;
	bool shouldQuit();

	virtual Common::Error init();
	virtual Common::Error run();
	virtual bool canLoadGameStateCurrently();
	virtual bool canSaveGameStateCurrently();
	virtual Common::Error loadGameState(int slot);
	virtual Common::Error saveGameState(int slot, const Common::String &desc);
	virtual void syncSoundSettings();
	Common::String generateSaveName(int slot);

	void initialize();
	void deinitialize();
};

extern TSageEngine *g_vm;

#define ALLOCATE_HANDLE(x) g_vm->_memoryManager.allocate(x)
#define ALLOCATE(x) g_vm->_memoryManager.allocate2(x)
#define DEALLOCATE(x) g_vm->_memoryManager.deallocate(x)

} // End of namespace TsAGE

#endif
