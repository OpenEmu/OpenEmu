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

#ifndef COMPOSER_H
#define COMPOSER_H

#include "common/config-file.h"
#include "common/random.h"
#include "common/system.h"
#include "common/debug.h"
#include "common/debug-channels.h"
#include "common/textconsole.h"
#include "common/rect.h"

#include "engines/engine.h"
#include "engines/util.h"

#include "graphics/surface.h"

#include "audio/mixer.h"

#include "composer/resource.h"

namespace Audio {
	class QueuingAudioStream;
}

namespace Composer {

struct ComposerGameDescription;

enum GameType {
	GType_ComposerV1,
	GType_ComposerV2
};

class Archive;
struct Animation;
class ComposerEngine;
class Pipe;
struct Sprite;

enum {
	kButtonRect = 0,
	kButtonEllipse = 1,
	kButtonSprites = 4
};

class Button {
public:
	Button() { }
	Button(Common::SeekableReadStream *stream, uint16 id, uint gameType);
	Button(Common::SeekableReadStream *stream);

	bool contains(const Common::Point &pos) const;

	uint16 _id;
	uint16 _type;
	uint16 _zorder;
	uint16 _scriptId;
	uint16 _scriptIdRollOn;
	uint16 _scriptIdRollOff;
	bool _active;

	Common::Rect _rect;
	Common::Array<uint16> _spriteIds;
};

enum {
	kEventAnimStarted = 1,
	kEventAnimDone = 2,
	kEventLoad = 3,
	kEventUnload = 4,
	kEventKeyDown = 5,
	kEventChar = 6,
	kEventKeyUp = 7
};

struct KeyboardHandler {
	uint16 keyId;
	uint16 modifierId;
	uint16 scriptId;
};

struct RandomEvent {
	uint16 weight;
	uint16 scriptId;
};

struct Library {
	uint _id;
	Archive *_archive;

	Common::List<Button> _buttons;
	Common::List<KeyboardHandler> _keyboardHandlers;
};

struct QueuedScript {
	uint32 _baseTime;
	uint32 _duration;
	uint32 _count;
	uint16 _scriptId;
};

struct PendingPageChange {
	PendingPageChange() { }
	PendingPageChange(uint16 id, bool remove) : _pageId(id), _remove(remove) { }

	uint16 _pageId;
	bool _remove;
};

struct OldScript {
	OldScript(uint16 id, Common::SeekableReadStream *stream);
	~OldScript();

	uint16 _id;

	uint32 _size;
	Common::SeekableReadStream *_stream;

	uint16 _zorder;
	uint32 _currDelay;
};

class ComposerEngine : public Engine {
protected:
	Common::Error run();

public:
	ComposerEngine(OSystem *syst, const ComposerGameDescription *gameDesc);
	virtual ~ComposerEngine();

	virtual bool hasFeature(EngineFeature f) const;

	int getGameType() const;
	const char *getGameId() const;
	uint32 getFeatures() const;
	Common::Language getLanguage() const;

	const ComposerGameDescription *_gameDescription;

private:
	Common::RandomSource *_rnd;

	Audio::SoundHandle _soundHandle;
	Audio::QueuingAudioStream *_audioStream;
	uint16 _currSoundPriority;

	uint32 _currentTime, _lastTime;

	bool _needsUpdate;
	Common::Array<Common::Rect> _dirtyRects;
	Graphics::Surface _screen;
	Common::List<Sprite> _sprites;

	uint _directoriesToStrip;
	Common::ConfigFile _bookIni;
	Common::String _bookGroup;
	Common::List<Library> _libraries;
	Common::Array<PendingPageChange> _pendingPageChanges;

	Common::Array<uint16> _stack;
	Common::Array<uint16> _vars;

	Common::List<OldScript *> _oldScripts;
	Common::Array<QueuedScript> _queuedScripts;
	Common::List<Animation *> _anims;
	Common::List<Pipe *> _pipes;
	Common::Array<Common::SeekableReadStream *> _pipeStreams;

	Common::HashMap<uint16, Common::Array<RandomEvent> > _randomEvents;

	void onMouseDown(const Common::Point &pos);
	void onMouseMove(const Common::Point &pos);
	void onKeyDown(uint16 keyCode);
	void setCursor(uint16 id, const Common::Point &offset);
	void setCursorVisible(bool visible);

	bool _mouseEnabled;
	bool _mouseVisible;
	Common::Point _lastMousePos;
	const Button *_lastButton;
	uint16 _mouseSpriteId;
	Common::Point _mouseOffset;

	Common::String getStringFromConfig(const Common::String &section, const Common::String &key);
	Common::String getFilename(const Common::String &section, uint id);
	Common::String mangleFilename(Common::String filename);
	void loadLibrary(uint id);
	void unloadLibrary(uint id);

	bool hasResource(uint32 tag, uint16 id);
	Common::SeekableReadStream *getResource(uint32 tag, uint16 id);

	void runEvent(uint16 id, int16 param1, int16 param2, int16 param3);
	int16 runScript(uint16 id, int16 param1, int16 param2, int16 param3);

	int16 getArg(uint16 arg, uint16 type);
	void setArg(uint16 arg, uint16 type, uint16 val);
	void runScript(uint16 id);
	int16 scriptFuncCall(uint16 id, int16 param1, int16 param2, int16 param3);
	void runOldScript(uint16 id, uint16 wait);
	void stopOldScript(uint16 id);
	void tickOldScripts();
	bool tickOldScript(OldScript *script);

	void playAnimation(uint16 animId, int16 param1, int16 param2, int16 param3);
	void stopAnimation(Animation *anim, bool localOnly = false, bool pipesOnly = false);
	void playWaveForAnim(uint16 id, uint16 priority, bool bufferingOnly);
	void processAnimFrame();

	void playPipe(uint16 id);
	void stopPipes();

	bool spriteVisible(uint16 id, uint16 animId);
	Sprite *addSprite(uint16 id, uint16 animId, uint16 zorder, const Common::Point &pos);
	void removeSprite(uint16 id, uint16 animId);
	const Sprite *getSpriteAtPos(const Common::Point &pos);
	const Button *getButtonFor(const Sprite *sprite, const Common::Point &pos);
	void setButtonActive(uint16 id, bool active);

	void dirtySprite(const Sprite &sprite);
	void redraw();
	void loadCTBL(uint16 id, uint fadePercent);
	void setBackground(uint16 id);
	void decompressBitmap(uint16 type, Common::SeekableReadStream *stream, byte *buffer, uint32 size, uint width, uint height);
	bool initSprite(Sprite &sprite);
	Common::SeekableReadStream *getStreamForSprite(uint16 id);
	void drawSprite(const Sprite &sprite);
};

} // End of namespace Composer

#endif
