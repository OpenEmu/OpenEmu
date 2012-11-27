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

#ifndef TOON_TOON_H
#define TOON_TOON_H

#include "engines/engine.h"
#include "graphics/surface.h"
#include "common/random.h"
#include "common/error.h"
#include "toon/resource.h"
#include "toon/script.h"
#include "toon/script_func.h"
#include "toon/state.h"
#include "toon/picture.h"
#include "toon/anim.h"
#include "toon/movie.h"
#include "toon/font.h"
#include "toon/text.h"
#include "toon/audio.h"
#include "toon/console.h"

namespace Common {
class MemoryWriteStreamDynamic;
}

struct ADGameDescription;

#define TOON_DAT_VER_MAJ 0  // 1 byte
#define TOON_DAT_VER_MIN 3  // 1 byte
#define TOON_SAVEGAME_VERSION 4
#define DATAALIGNMENT 4

#define TOON_SCREEN_WIDTH 640
#define TOON_SCREEN_HEIGHT 400
#define TOON_BACKBUFFER_WIDTH 1280
#define TOON_BACKBUFFER_HEIGHT 400

/**
 * This is the namespace of the Toon engine.
 *
 * Status of this engine: ???
 *
 * Games using this engine:
 * - Toonstruck
 */
namespace Toon {

enum ToonGameType {
	GType_TOON = 1
};

enum ToonDebugChannels {
	kDebugAnim      = 1 <<  0,
	kDebugCharacter = 1 <<  1,
	kDebugAudio     = 1 <<  2,
	kDebugHotspot   = 1 <<  3,
	kDebugFont      = 1 <<  4,
	kDebugPath      = 1 <<  5,
	kDebugMovie     = 1 <<  6,
	kDebugPicture   = 1 <<  7,
	kDebugResource  = 1 <<  8,
	kDebugState     = 1 <<  9,
	kDebugTools     = 1 << 10,
	kDebugText      = 1 << 11
};

class Picture;
class Movie;
class Hotspots;
class Character;
class CharacterDrew;
class CharacterFlux;
class FontRenderer;
class TextResource;
class AudioManager;
class PathFinding;

class ToonEngine : public Engine {
public:
	ToonEngine(OSystem *syst, const ADGameDescription *gameDescription);
	~ToonEngine();

	const ADGameDescription *_gameDescription;
	Common::Language _language;
	byte   _numVariant;
	byte   _gameVariant;
	char **_locationDirNotVisited;
	char **_locationDirVisited;
	char **_specialInfoLine;

	Common::Error run();
	GUI::Debugger *getDebugger() { return _console; }
	bool showMainmenu(bool &loadedGame);
	void init();
	bool loadToonDat();
	char **loadTextsVariants(Common::File &in);
	void unloadTextsVariants(char **texts);
	void unloadToonDat();
	void setPaletteEntries(uint8 *palette, int32 offset, int32 num);
	void fixPaletteEntries(uint8 *palette, int num);
	void flushPalette(bool deferFlushToNextRender = true);
	void parseInput();
	void initChapter();
	void initFonts();
	void loadScene(int32 SceneId, bool forGameLoad = false);
	void exitScene();
	void loadCursor();
	void setCursor(int32 type, bool inventory = false, int32 offsetX = 0, int offsetY = 0);
	void loadAdditionalPalette(const Common::String &fileName, int32 mode);
	void setupGeneralPalette();
	void render();
	void update(int32 timeIncrement);
	void doFrame();
	void updateAnimationSceneScripts(int32 timeElapsed);
	void updateCharacters(int32 timeElapsed);
	void setSceneAnimationScriptUpdate(bool enable);
	bool isUpdatingSceneAnimation();
	int32 getCurrentUpdatingSceneAnimation();
	int32 randRange(int32 minStart, int32 maxStart);
	void selectHotspot();
	void clickEvent();
	int32 runEventScript(int32 x, int32 y, int32 mode, int32 id, int32 scriptId);
	void flipScreens();
	void drawInfoLine();
	void drawConversationLine();
	const char *getLocationString(int32 locationId, bool alreadyVisited);
	int32 getScaleAtPoint(int32 x, int32 y);
	int32 getZAtPoint(int32 x, int32 y);
	int32 getLayerAtPoint(int32 x, int32 y);
	int32 characterTalk(int32 dialogid, bool blocking = true);
	int32 simpleCharacterTalk(int32 dialogid);
	void sayLines(int numLines, int dialogId);
	void haveAConversation(int32 convId);
	void processConversationClick(Conversation *conv, int32 status);
	int32 runConversationCommand(int16 **command);
	void prepareConversations();
	void drawConversationIcons();
	void simpleUpdate(bool waitCharacterToTalk = false);
	int32 waitTicks(int32 numTicks, bool breakOnMouseClick);
	void copyToVirtualScreen(bool updateScreen = true);
	void getMouseEvent();
	int32 showInventory();
	void drawSack();
	void addItemToInventory(int32 item);
	void deleteItemFromInventory(int32 item);
	void replaceItemFromInventory(int32 item, int32 destItem);
	void rearrangeInventory();
	void createMouseItem(int32 item);
	void deleteMouseItem();
	void showCutaway(const Common::String &cutawayPicture);
	void hideCutaway();
	void drawPalette();
	void newGame();
	void playSoundWrong();
	void playSFX(int32 id, int32 volume);
	void storeRifFlags(int32 location);
	void restoreRifFlags(int32 location);
	void getTextPosition(int32 characterId, int32 *retX, int32 *retY);
	int32 getConversationFlag(int32 locationId, int32 param);
	int32 getSpecialInventoryItem(int32 item);
	Character *getCharacterById(int32 charId);
	Common::String getSavegameName(int nr);
	bool loadGame(int32 slot);
	bool saveGame(int32 slot, const Common::String &saveGameDesc);
	void fadeIn(int32 numFrames);
	void fadeOut(int32 numFrames);
	void initCharacter(int32 characterId, int32 animScriptId, int32 animToPlayId, int32 sceneAnimationId);
	int32 handleInventoryOnFlux(int32 itemId);
	int32 handleInventoryOnInventory(int32 itemDest, int32 itemSrc);
	int32 handleInventoryOnDrew(int32 itemId);
	int32 pauseSceneAnimationScript(int32 animScriptId, int32 tickToWait);
	void updateTimer(int32 timeIncrement);
	Common::String createRoomFilename(const Common::String &name);
	void createShadowLUT();
	void playTalkAnimOnCharacter(int32 animID, int32 characterId, bool talker);
	void updateScrolling(bool force, int32 timeIncrement);
	void enableTimer(int32 timerId);
	void setTimer(int32 timerId, int32 timerWait);
	void disableTimer(int32 timerId);
	void updateTimers();
	void makeLineNonWalkable(int32 x, int32 y, int32 x2, int32 y2);
	void makeLineWalkable(int32 x, int32 y, int32 x2, int32 y2);
	void renderInventory();
	void viewInventoryItem(const Common::String &str, int32 lineId, int32 itemDest);
	void storePalette();
	void restorePalette();
	const char *getSpecialConversationMusic(int32 locationId);
	void playRoomMusic();
	void waitForScriptStep();
	void doMagnifierEffect();

	bool canSaveGameStateCurrently();
	bool canLoadGameStateCurrently();
	void pauseEngineIntern(bool pause);

	Resources *resources() {
		return _resources;
	}

	State *state() {
		return _gameState;
	}

	Graphics::Surface &getMainSurface() {
		return *_mainSurface;
	}

	Picture *getMask() {
		return _currentMask;
	}

	Picture *getPicture() {
		return _currentPicture;
	}

	AnimationManager *getAnimationManager() {
		return _animationManager;
	}

	Movie *getMoviePlayer() {
		return _moviePlayer;
	}

	SceneAnimation *getSceneAnimation(int32 id) {
		return &_sceneAnimations[id];
	}

	SceneAnimationScript *getSceneAnimationScript(int32 id) {
		return &_sceneAnimationScripts[id];
	}

	EMCInterpreter *getScript() {
		return _script;
	}

	Hotspots *getHotspots() {
		return _hotspots;
	}

	Character *getCharacter(int32 charId) {
		return _characters[charId];
	}

	uint8 *getShadowLUT() {
		return _shadowLUT;
	}

	int32 getCurrentLineToSay() {
		return _currentTextLineId;
	}

	int32 getCurrentCharacterTalking() {
		return _currentTextLineCharacterId;
	}

	CharacterDrew *getDrew() {
		return (CharacterDrew *)_drew;
	}

	CharacterFlux *getFlux() {
		return (CharacterFlux *)_flux;
	}

	int32 getTickLength() {
		return _tickLength;
	}

	int32 getOldMilli() {
		return _oldTimer2;
	}

	AudioManager *getAudioManager() {
		return _audioManager;
	}

	int32 getScriptRegionNested() {
		return _currentScriptRegion;
	}

	int32 getMouseX() {
		return _mouseX;
	}

	int32 getMouseY() {
		return _mouseY;
	}

	PathFinding *getPathFinding() {
		return _pathFinding;
	}

	Common::WriteStream *getSaveBufferStream();

	bool shouldQuitGame() const {
		return _shouldQuit;
	}

	Common::Error saveGameState(int slot, const Common::String &desc) {

		return (saveGame(slot, desc) ? Common::kWritingFailed : Common::kNoError);
	}

	Common::Error loadGameState(int slot) {
		return (loadGame(slot) ? Common::kReadingFailed : Common::kNoError);
	}

	bool hasFeature(EngineFeature f) const {
		return
			(f == kSupportsRTL) ||
			(f == kSupportsLoadingDuringRuntime) ||
			(f == kSupportsSavingDuringRuntime);
	}

	void dirtyAllScreen();
	void addDirtyRect(int32 left, int32 top, int32 right, int32 bottom);
	void clearDirtyRects();

protected:
	int32 _tickLength;
	Resources *_resources;
	TextResource *_genericTexts;
	TextResource *_roomTexts;
	State *_gameState;
	uint8 *_finalPalette;
	uint8 *_backupPalette;
	uint8 *_additionalPalette1;
	uint8 *_additionalPalette2;
	bool _additionalPalette2Present;
	uint8 *_cutawayPalette;
	uint8 *_universalPalette;
	uint8 *_fluxPalette;
	uint8 *_roomScaleData;
	uint8 *_shadowLUT;

	Picture *_currentPicture;
	Picture *_currentMask;
	Picture *_currentCutaway;
	Picture *_inventoryPicture;
	PathFinding *_pathFinding;

	EMCInterpreter *_script;
	EMCData _scriptData;
	EMCState _scriptState[4];
	int32 _currentScriptRegion; // script region ( nested script run )

	ScriptFunc *_script_func;

	SceneAnimation _sceneAnimations[64];
	SceneAnimationScript _sceneAnimationScripts[64];
	int32 _lastProcessedSceneScript;
	bool _animationSceneScriptRunFlag;
	bool _updatingSceneScriptRunFlag;

	Graphics::Surface *_mainSurface;
	Common::Array<Common::Rect> _dirtyRects;
	Common::Array<Common::Rect> _oldDirtyRects;

	bool _dirtyAll;


	AnimationInstance *_cursorAnimationInstance;
	Animation *_cursorAnimation;
	Animation *_dialogIcons;
	Animation *_inventoryIcons;
	Animation *_inventoryIconSlots;
	int32 _cursorOffsetX;
	int32 _cursorOffsetY;

	char *_currentTextLine;
	int32 _currentTextLineId;
	int32 _currentTextLineX;
	int32 _currentTextLineY;
	int32 _currentTextLineCharacterId;

	int32 _oldScrollValue;

	AnimationManager *_animationManager;

	Character *_characters[32];
	Character *_drew;
	Character *_flux;

	Hotspots *_hotspots;
	int32 _currentHotspotItem;

	bool _shouldQuit;
	int32 _scriptStep;

	int32 _mouseX;
	int32 _mouseY;
	int32 _mouseButton;
	int32 _lastMouseButton;

	int32 _oldTimer;
	int32 _oldTimer2;
	int32 _lastRenderTime;

	Movie *_moviePlayer;

	Common::RandomSource _rnd;

	FontRenderer *_fontRenderer;
	Animation *_fontToon;
	Animation *_fontEZ;

	AudioManager *_audioManager;

	Common::MemoryWriteStreamDynamic *_saveBufferStream;

	int16 *_conversationData;

	bool _firstFrame;
	bool _isDemo;
	bool _showConversationText;
	bool _needPaletteFlush;
private:
	ToonConsole *_console;
};

} // End of namespace Toon

#endif
