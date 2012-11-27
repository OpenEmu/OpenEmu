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

#ifndef MOHAWK_MYST_H
#define MOHAWK_MYST_H

#include "mohawk/console.h"
#include "mohawk/mohawk.h"
#include "mohawk/resource_cache.h"
#include "mohawk/myst_scripts.h"

#include "common/random.h"

#include "gui/saveload.h"

namespace Mohawk {

class MohawkEngine_Myst;
class VideoManager;
class MystGraphics;
class MystScriptParser;
class MystConsole;
class MystGameState;
class MystOptionsDialog;
class MystResource;
class MystResourceType8;
class MystResourceType13;

// Engine Debug Flags
enum {
	kDebugVariable = (1 << 0),
	kDebugSaveLoad = (1 << 1),
	kDebugView     = (1 << 2),
	kDebugHint     = (1 << 3),
	kDebugResource = (1 << 4),
	kDebugINIT     = (1 << 5),
	kDebugEXIT     = (1 << 6),
	kDebugScript   = (1 << 7),
	kDebugHelp     = (1 << 8),
	kDebugCache    = (1 << 9)
};

// Myst Stacks
enum {
	kChannelwoodStack = 0,	// Channelwood Age
	kCreditsStack,			// Credits
	kDemoStack,				// Demo Main Menu
	kDniStack,				// D'ni
	kIntroStack,			// Intro
	kMakingOfStack,			// Making Of Myst
	kMechanicalStack,		// Mechanical Age
	kMystStack,				// Myst Island
	kSeleniticStack,		// Selenitic Age
	kDemoSlidesStack,		// Demo Slideshow
	kDemoPreviewStack,		// Demo Myst Library Preview
	kStoneshipStack			// Stoneship Age
};

const uint16 kMasterpieceOnly = 0xFFFF;

struct MystCondition {
	uint16 var;
	uint16 numStates;
	uint16 *values;
};

// View Sound Action Type
enum {
	kMystSoundActionConditional  = -4,
	kMystSoundActionContinue     = -1,
	kMystSoundActionChangeVolume = -2,
	kMystSoundActionStop         = -3
	// Other positive values are PlayNewSound of that id
};

// View flags
enum {
	kMystZipDestination = (1 << 0)
};

struct MystView {
	uint16 flags;

	// Image Data
	uint16 conditionalImageCount;
	MystCondition *conditionalImages;
	uint16 mainImage;

	// Sound Data
	int16 sound;
	uint16 soundVolume;
	uint16 soundVar;
	uint16 soundCount;
	int16 *soundList;
	uint16 *soundListVolume;

	// Script Resources
	uint16 scriptResCount;
	struct ScriptResource {
		uint16 type;
		uint16 id; // Not used by type 3
		// TODO: Type 3 has more. Maybe use a union?
		uint16 var; // Used by type 3 only
		uint16 count; // Used by type 3 only
		uint16 u0; // Used by type 3 only
		int16 *resource_list; // Used by type 3 only
	} *scriptResources;

	// Resource ID's
	uint16 rlst;
	uint16 hint;
	uint16 init;
	uint16 exit;
};

struct MystCursorHint {
	uint16 id;
	int16 cursor;

	MystCondition variableHint;
};

class MohawkEngine_Myst : public MohawkEngine {
protected:
	Common::Error run();

public:
	MohawkEngine_Myst(OSystem *syst, const MohawkGameDescription *gamedesc);
	virtual ~MohawkEngine_Myst();

	Common::SeekableReadStream *getResource(uint32 tag, uint16 id);

	Common::String wrapMovieFilename(const Common::String &movieName, uint16 stack);

	void reloadSaveList();

	void changeToStack(uint16 stack, uint16 card, uint16 linkSrcSound, uint16 linkDstSound);
	void changeToCard(uint16 card, bool updateScreen);
	uint16 getCurCard() { return _curCard; }
	uint16 getCurStack() { return _curStack; }
	void setMainCursor(uint16 cursor);
	uint16 getMainCursor() { return _mainCursor; }
	void checkCursorHints();
	MystResource *updateCurrentResource();
	bool skippableWait(uint32 duration);

	bool _tweaksEnabled;
	bool _needsUpdate;
	bool _needsPageDrop;
	bool _needsShowMap;
	bool _needsShowDemoMenu;

	MystView _view;
	MystGraphics *_gfx;
	MystGameState *_gameState;
	MystScriptParser *_scriptParser;
	Common::Array<MystResource *> _resources;
	MystResource *_dragResource;
	Common::RandomSource *_rnd;

	bool _showResourceRects;
	MystResource *loadResource(Common::SeekableReadStream *rlstStream, MystResource *parent);
	void setResourceEnabled(uint16 resourceId, bool enable);
	void redrawArea(uint16 var, bool update = true);
	void redrawResource(MystResourceType8 *resource, bool update = true);
	void drawResourceImages();
	void drawCardBackground();
	uint16 getCardBackgroundId();

	void setCacheState(bool state) { _cache.enabled = state; }
	bool getCacheState() { return _cache.enabled; }

	GUI::Debugger *getDebugger() { return _console; }

	bool canLoadGameStateCurrently();
	bool canSaveGameStateCurrently();
	Common::Error loadGameState(int slot);
	Common::Error saveGameState(int slot, const Common::String &desc);
	bool hasFeature(EngineFeature f) const;

private:
	MystConsole *_console;
	GUI::SaveLoadChooser *_loadDialog;
	MystOptionsDialog *_optionsDialog;
	MystScriptParser *_prevStack;
	ResourceCache _cache;
	void cachePreload(uint32 tag, uint16 id);

	uint16 _curStack;
	uint16 _curCard;

	bool _runExitScript;

	void dropPage();

	void loadCard();
	void unloadCard();
	void runInitScript();
	void runExitScript();

	void loadHelp(uint16 id);

	void loadResources();
	void drawResourceRects();
	void checkCurrentResource();
	int16 _curResource;
	MystResourceType13 *_hoverResource;

	uint16 _cursorHintCount;
	MystCursorHint *_cursorHints;
	void loadCursorHints();
	uint16 _currentCursor;
	uint16 _mainCursor; // Also defines the current page being held (white, blue, red, or none)
};

} // End of namespace Mohawk

#endif
