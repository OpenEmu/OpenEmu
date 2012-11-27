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

/*
 * This file is based on WME Lite.
 * http://dead-code.org/redir.php?target=wmelite
 * Copyright (c) 2011 Jan Nedoma
 */

#ifndef WINTERMUTE_BASE_GAME_H
#define WINTERMUTE_BASE_GAME_H

#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/persistent.h"
#include "engines/wintermute/coll_templ.h"
#include "engines/wintermute/math/rect32.h"
#include "common/events.h"

namespace Wintermute {

typedef void (*ENGINE_LOG_CALLBACK)(char *text, bool result, void *data);

class BaseSoundMgr;
class BaseFader;
class BaseFont;
class BaseFileManager;
class BaseTransitionMgr;
class ScEngine;
class BaseFontStorage;
class BaseStringTable;
class BaseQuickMsg;
class UIWindow;
class BaseViewport;
class BaseRenderer;
class BaseRegistry;
class BaseSaveThumbHelper;
class BaseSurfaceStorage;
class SXMath;
class BaseKeyboardState;
class VideoPlayer;
class VideoTheoraPlayer;

#define NUM_MUSIC_CHANNELS 5

class BaseGame: public BaseObject {
public:
	DECLARE_PERSISTENT(BaseGame, BaseObject)

	virtual bool onScriptShutdown(ScScript *script);

	virtual bool onActivate(bool activate, bool refreshMouse);
	virtual bool onMouseLeftDown();
	virtual bool onMouseLeftUp();
	virtual bool onMouseLeftDblClick();
	virtual bool onMouseRightDblClick();
	virtual bool onMouseRightDown();
	virtual bool onMouseRightUp();
	virtual bool onMouseMiddleDown();
	virtual bool onMouseMiddleUp();
	virtual bool onPaint();
	virtual bool onWindowClose();

	bool isLeftDoubleClick();
	bool isRightDoubleClick();

	bool _autorunDisabled;

	uint32 _lastMiniUpdate;
	bool _miniUpdateEnabled;

	virtual void miniUpdate();

	void getMousePos(Point32 *Pos);
	Rect32 _mouseLockRect;

	bool _shuttingDown;

	virtual bool displayDebugInfo();
	bool _debugShowFPS;

	bool _suspendedRendering;
	int _soundBufferSizeSec;

	TTextEncoding _textEncoding;
	bool _textRTL;

	virtual bool resetContent();

	void DEBUG_DumpClassRegistry();
	bool setWaitCursor(const char *filename);

	int _thumbnailWidth;
	int _thumbnailHeight;

	bool _editorMode;
	void getOffset(int *offsetX, int *offsetY);
	void setOffset(int offsetX, int offsetY);
	int getSequence();

	int _offsetY;
	int _offsetX;
	float _offsetPercentX;
	float _offsetPercentY;
	BaseObject *_mainObject;

	bool initInput();
	bool initLoop();
	uint32 _currentTime;
	uint32 _deltaTime;
	BaseFont *_systemFont;
	BaseFont *_videoFont;
	bool initialize1();
	bool initialize2();
	bool initialize3();
	BaseTransitionMgr *_transMgr;

	void LOG(bool res, const char *fmt, ...);

	BaseRenderer *_renderer;
	BaseSoundMgr *_soundMgr;
	ScEngine *_scEngine;
	SXMath *_mathClass;
	BaseSurfaceStorage *_surfaceStorage;
	BaseFontStorage *_fontStorage;
	BaseGame(const Common::String &gameId);
	virtual ~BaseGame();

	void DEBUG_DebugDisable();
	void DEBUG_DebugEnable(const char *filename = NULL);
	bool _debugDebugMode;

	void *_debugLogFile;
	int _sequence;
	virtual bool loadFile(const char *filename);
	virtual bool loadBuffer(byte *buffer, bool complete = true);

	int _viewportSP;

	BaseStringTable *_stringTable;
	int _settingsResWidth;
	int _settingsResHeight;
	char *_settingsGameFile;
	bool _suppressScriptErrors;
	bool _mouseLeftDown; // TODO: Hide

	virtual bool externalCall(ScScript *script, ScStack *stack, ScStack *thisStack, char *name);
	// scripting interface
	virtual ScValue *scGetProperty(const Common::String &name);
	virtual bool scSetProperty(const char *name, ScValue *value);
	virtual bool scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name);
	virtual const char *scToString();
	// compatibility bits
	bool _compatKillMethodThreads;

	const char* getGameId() { return _gameId.c_str(); }
	void setGameId(const Common::String& gameId) { _gameId = gameId; }
	uint32 _surfaceGCCycleTime;
	bool _smartCache; // RO
	bool _subtitles; // RO

	int _scheduledLoadSlot;
	bool _loading;

	virtual bool handleMouseWheel(int delta);
	bool _quitting;
	virtual bool getVersion(byte *verMajor, byte *verMinor, byte *extMajor, byte *extMinor);

	virtual bool handleKeypress(Common::Event *event, bool printable = false);
	virtual void handleKeyRelease(Common::Event *event);

	bool unfreeze();
	bool freeze(bool includingMusic = true);
	bool focusWindow(UIWindow *window);
	bool _loadInProgress;
	UIWindow *_focusedWindow;
	bool _editorForceScripts;

	static void invalidateValues(void *value, void *data);

	bool loadSettings(const char *filename);

	bool displayWindows(bool inGame = false);
	bool _useD3D;
	virtual bool cleanup();
	bool loadGame(int slot);
	bool loadGame(const char *filename);
	bool saveGame(int slot, const char *desc, bool quickSave = false);
	virtual bool showCursor();

	BaseObject *_activeObject;

	bool _interactive;
	TGameState _state;
	TGameState _origState;
	bool _origInteractive;
	uint32 _timer;
	uint32 _timerDelta;
	uint32 _timerLast;

	uint32 _liveTimer;
	uint32 _liveTimerDelta;
	uint32 _liveTimerLast;

	BaseObject *_capturedObject;
	Point32 _mousePos;
	bool validObject(BaseObject *object);
	bool unregisterObject(BaseObject *object);
	bool registerObject(BaseObject *object);
	void quickMessage(const char *text);
	void quickMessageForm(char *fmt, ...);
	bool displayQuickMsg();

	virtual bool displayContent(bool update = true, bool displayAll = false);
	virtual bool displayContentSimple();
	bool _forceNonStreamedSounds;
	void resetMousePos();
	int _subtitlesSpeed;
	void setInteractive(bool state);
	virtual bool windowLoadHook(UIWindow *win, char **buf, char **params);
	virtual bool windowScriptMethodHook(UIWindow *win, ScScript *script, ScStack *stack, const char *name);
	bool getCurrentViewportOffset(int *offsetX = NULL, int *offsetY = NULL);
	bool getCurrentViewportRect(Rect32 *rect, bool *custom = NULL);
	bool popViewport();
	bool pushViewport(BaseViewport *Viewport);
	bool setActiveObject(BaseObject *Obj);
	BaseSprite *_lastCursor;
	bool drawCursor(BaseSprite *Cursor);

	BaseSaveThumbHelper *_cachedThumbnail;
	void addMem(int bytes);
	bool _touchInterface;
	bool _constrainedMemory;
protected:
	BaseSprite *_loadingIcon;
	int _loadingIconX;
	int _loadingIconY;
	int _loadingIconPersistent;

	BaseFader *_fader;

	int _freezeLevel;
	VideoPlayer *_videoPlayer;
	VideoTheoraPlayer *_theoraPlayer;
private:
	bool _mouseRightDown;
	bool _mouseMidlleDown;
	bool _settingsRequireAcceleration;
	bool _settingsAllowWindowed;
	bool _settingsAllowAdvanced;
	bool _settingsAllowAccessTab;
	bool _settingsAllowAboutTab;
	bool _settingsRequireSound;
	bool _settingsAllowDesktopRes;
	int _settingsTLMode;
	virtual bool invalidateDeviceObjects();
	virtual bool restoreDeviceObjects();

	char *_localSaveDir;
	bool _saveDirChecked;
	bool _richSavedGames;
	char *_savedGameExt;

	bool _reportTextureFormat;

	// FPS stuff
	uint32 _lastTime;
	uint32 _fpsTime;
	uint32 _framesRendered;
	Common::String _gameId;

	void setEngineLogCallback(ENGINE_LOG_CALLBACK callback = NULL, void *data = NULL);
	ENGINE_LOG_CALLBACK _engineLogCallback;
	void *_engineLogCallbackData;

	bool _videoSubtitles;
	uint32 _musicStartTime[NUM_MUSIC_CHANNELS];
	bool _compressedSavegames;

	bool _personalizedSave;

	void setWindowTitle();

	bool resumeMusic(int channel);
	bool setMusicStartTime(int channel, uint32 time);
	bool pauseMusic(int channel);
	bool stopMusic(int channel);
	bool playMusic(int channel, const char *filename, bool looping = true, uint32 loopStart = 0);
	BaseSound *_music[NUM_MUSIC_CHANNELS];
	bool _musicCrossfadeRunning;
	bool _musicCrossfadeSwap;
	uint32 _musicCrossfadeStartTime;
	uint32 _musicCrossfadeLength;
	int _musicCrossfadeChannel1;
	int _musicCrossfadeChannel2;

	BaseSprite *_cursorNoninteractive;
	BaseKeyboardState *_keyboardState;

	uint32 _fps;
	bool updateMusicCrossfade();

	bool isVideoPlaying();
	bool stopVideo();

	BaseArray<BaseQuickMsg *> _quickMessages;
	BaseArray<UIWindow *> _windows;
	BaseArray<BaseViewport *> _viewportStack;
	BaseArray<BaseObject *> _regObjects;

	AnsiString getDeviceType() const;

	struct LastClickInfo {
		LastClickInfo() {
			posX = posY = 0;
			time = 0;
		}

		int posX;
		int posY;
		uint32 time;
	};

	LastClickInfo _lastClick[2];
	bool isDoubleClick(int buttonIndex);
	uint32 _usedMem;



protected:
	// WME Lite specific
	bool _autoSaveOnExit;
	int _autoSaveSlot;
	bool _cursorHidden;

public:
	void autoSaveOnExit();

};

} // end of namespace Wintermute

#endif
