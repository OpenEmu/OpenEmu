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
#include "common/scummsys.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/random.h"
#include "common/fs.h"
#include "common/keyboard.h"
#include "common/substream.h"

#include "graphics/cursorman.h"
#include "graphics/surface.h"
#include "graphics/pixelformat.h"
#include "graphics/wincursor.h"

#include "engines/util.h"
#include "engines/advancedDetector.h"

#include "audio/audiostream.h"

#include "composer/composer.h"
#include "composer/graphics.h"
#include "composer/resource.h"

namespace Composer {

ComposerEngine::ComposerEngine(OSystem *syst, const ComposerGameDescription *gameDesc) : Engine(syst), _gameDescription(gameDesc) {
	_rnd = new Common::RandomSource("composer");
	_audioStream = NULL;
}

ComposerEngine::~ComposerEngine() {
	DebugMan.clearAllDebugChannels();

	stopPipes();
	for (Common::List<OldScript *>::iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++)
		delete *i;
	for (Common::List<Animation *>::iterator i = _anims.begin(); i != _anims.end(); i++)
		delete *i;
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
		delete i->_archive;
	for (Common::List<Sprite>::iterator i = _sprites.begin(); i != _sprites.end(); i++)
		i->_surface.free();

	delete _rnd;
}

Common::Error ComposerEngine::run() {
	Common::Event event;

	_vars.resize(1000);
	for (uint i = 0; i < _vars.size(); i++)
		_vars[i] = 0;

	_queuedScripts.resize(10);
	for (uint i = 0; i < _queuedScripts.size(); i++) {
		_queuedScripts[i]._count = 0;
		_queuedScripts[i]._scriptId = 0;
	}

	_mouseVisible = true;
	_mouseEnabled = false;
	_mouseSpriteId = 0;
	_lastButton = NULL;

	_directoriesToStrip = 1;
	if (!_bookIni.loadFromFile("book.ini")) {
		_directoriesToStrip = 0;
		if (!_bookIni.loadFromFile("programs/book.ini")) {
			// mac version?
			if (!_bookIni.loadFromFile("Darby the Dragon.ini"))
				if (!_bookIni.loadFromFile("Gregory.ini"))
					error("failed to find book.ini");
		}
	}

	uint width = 640;
	if (_bookIni.hasKey("Width", "Common"))
		width = atoi(getStringFromConfig("Common", "Width").c_str());
	uint height = 480;
	if (_bookIni.hasKey("Height", "Common"))
		height = atoi(getStringFromConfig("Common", "Height").c_str());
	initGraphics(width, height, true);
	_screen.create(width, height, Graphics::PixelFormat::createFormatCLUT8());
	_needsUpdate = true;

	Graphics::Cursor *cursor = Graphics::makeDefaultWinCursor();
	CursorMan.replaceCursor(cursor->getSurface(), cursor->getWidth(), cursor->getHeight(), cursor->getHotspotX(),
		cursor->getHotspotY(), cursor->getKeyColor());
	CursorMan.replaceCursorPalette(cursor->getPalette(), cursor->getPaletteStartIndex(), cursor->getPaletteCount());
	delete cursor;

	loadLibrary(0);

	_currentTime = 0;
	_lastTime = 0;

	uint fps = atoi(getStringFromConfig("Common", "FPS").c_str());
	uint frameTime = 1000 / fps;
	uint32 lastDrawTime = 0;

	while (!shouldQuit()) {
		for (uint i = 0; i < _pendingPageChanges.size(); i++) {
			if (_pendingPageChanges[i]._remove)
				unloadLibrary(_pendingPageChanges[i]._pageId);
			else
				loadLibrary(_pendingPageChanges[i]._pageId);

			lastDrawTime = _system->getMillis();
		}
		_pendingPageChanges.clear();

		uint32 thisTime = _system->getMillis();
		// maintain our own internal timing, since otherwise we get
		// confused when starved of CPU (for example when the user
		// is dragging the scummvm window around)
		if (thisTime > _lastTime + frameTime)
			_currentTime += frameTime;
		else
			_currentTime += thisTime - _lastTime;
		_lastTime = thisTime;

		for (uint i = 0; i < _queuedScripts.size(); i++) {
			QueuedScript &script = _queuedScripts[i];
			if (!script._count)
				continue;
			if (script._baseTime + script._duration > _currentTime)
				continue;
			if (script._count != 0xffffffff)
				script._count--;
			script._baseTime = _currentTime;
			runScript(script._scriptId, i, 0, 0);
		}

		if (lastDrawTime + frameTime <= thisTime) {
			// catch up if we're more than 2 frames behind
			if (lastDrawTime + (frameTime * 2) <= thisTime)
				lastDrawTime = thisTime;
			else
				lastDrawTime += frameTime;

			redraw();

			tickOldScripts();
			processAnimFrame();
		} else if (_needsUpdate) {
			redraw();
		}

		while (_eventMan->pollEvent(event)) {
			switch (event.type) {
			case Common::EVENT_LBUTTONDOWN:
				onMouseDown(event.mouse);
				break;

			case Common::EVENT_LBUTTONUP:
				break;

			case Common::EVENT_RBUTTONDOWN:
				break;

			case Common::EVENT_MOUSEMOVE:
				onMouseMove(event.mouse);
				break;

			case Common::EVENT_KEYDOWN:
				switch (event.kbd.keycode) {
				case Common::KEYCODE_d:
					/*if (event.kbd.hasFlags(Common::KBD_CTRL)) {
						// Start the debugger
						getDebugger()->attach();
						getDebugger()->onFrame();
					}*/
					break;

				case Common::KEYCODE_q:
					if (event.kbd.hasFlags(Common::KBD_CTRL))
						quitGame();
					break;

				default:
					break;
				}

				onKeyDown(event.kbd.keycode);
				break;

			default:
				break;
			}
		}

		_system->delayMillis(20);
	}

	_screen.free();

	return Common::kNoError;
}

void ComposerEngine::onMouseDown(const Common::Point &pos) {
	if (!_mouseEnabled || !_mouseVisible)
		return;

	const Sprite *sprite = getSpriteAtPos(pos);
	const Button *button = getButtonFor(sprite, pos);
	if (!button)
		return;

	debug(3, "mouseDown on button id %d", button->_id);

	// TODO: other buttons?
	uint16 buttonsDown = 1; // MK_LBUTTON

	uint16 spriteId = sprite ? sprite->_id : 0;
	runScript(button->_scriptId, (getGameType() == GType_ComposerV1) ? 0 : button->_id, buttonsDown, spriteId);
}

void ComposerEngine::onMouseMove(const Common::Point &pos) {
	_lastMousePos = pos;

	if (!_mouseEnabled || !_mouseVisible)
		return;

	// TODO: do we need to keep track of this?
	uint buttonsDown = 0;

	const Sprite *sprite = getSpriteAtPos(pos);
	const Button *button = getButtonFor(sprite, pos);
	if (_lastButton != button) {
		if (_lastButton && _lastButton->_scriptIdRollOff)
			runScript(_lastButton->_scriptIdRollOff, (getGameType() == GType_ComposerV1) ? 0 : _lastButton->_id, buttonsDown, 0);
		_lastButton = button;
		if (_lastButton && _lastButton->_scriptIdRollOn)
			runScript(_lastButton->_scriptIdRollOn, (getGameType() == GType_ComposerV1) ? 0 : _lastButton->_id, buttonsDown, 0);
	}

	if (_mouseSpriteId) {
		addSprite(_mouseSpriteId, 0, 0, _lastMousePos - _mouseOffset);
	}
	_needsUpdate = true;
}

void ComposerEngine::onKeyDown(uint16 keyCode) {
	runEvent(kEventKeyDown, keyCode, 0, 0);
	runEvent(kEventChar, keyCode, 0, 0);

	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++) {
		for (Common::List<KeyboardHandler>::iterator j = i->_keyboardHandlers.begin(); j != i->_keyboardHandlers.end(); j++) {
			const KeyboardHandler &handler = *j;
			if (keyCode != handler.keyId)
				continue;

			int modifiers = g_system->getEventManager()->getModifierState();
			switch (handler.modifierId) {
			case 0x10: // shift
				if (!(modifiers & Common::KBD_SHIFT))
					continue;
				break;
			case 0x11: // control
				if (!(modifiers & Common::KBD_CTRL))
					continue;
				break;
			case 0:
				break;
			default:
				warning("unknown keyb modifier %d", handler.modifierId);
				continue;
			}

			runScript(handler.scriptId);
		}
	}
}

void ComposerEngine::setCursor(uint16 id, const Common::Point &offset) {
	_mouseOffset = offset;
	if (_mouseSpriteId == id)
		return;

	if (_mouseSpriteId && _mouseVisible) {
		removeSprite(_mouseSpriteId, 0);
	}
	_mouseSpriteId = id;
	if (_mouseSpriteId && _mouseVisible) {
		addSprite(_mouseSpriteId, 0, 0, _lastMousePos - _mouseOffset);
	}
}

void ComposerEngine::setCursorVisible(bool visible) {
	if (visible && !_mouseVisible) {
		_mouseVisible = true;
		if (_mouseSpriteId)
			addSprite(_mouseSpriteId, 0, 0, _lastMousePos - _mouseOffset);
		else
			CursorMan.showMouse(true);
		onMouseMove(_lastMousePos);
	} else if (!visible && _mouseVisible) {
		_mouseVisible = false;
		if (_mouseSpriteId)
			removeSprite(_mouseSpriteId, 0);
		else
			CursorMan.showMouse(false);
	}
}

Common::String ComposerEngine::getStringFromConfig(const Common::String &section, const Common::String &key) {
	Common::String value;
	if (!_bookIni.getKey(key, section, value))
		error("failed to find key '%s' in section '%s' of book config", key.c_str(), section.c_str());
	return value;
}

Common::String ComposerEngine::getFilename(const Common::String &section, uint id) {
	Common::String key = Common::String::format("%d", id);
	Common::String filename = getStringFromConfig(section, key);

	return mangleFilename(filename);
}

Common::String ComposerEngine::mangleFilename(Common::String filename) {
	while (filename.size() && (filename[0] == '~' || filename[0] == ':' || filename[0] == '\\'))
		filename = filename.c_str() + 1;

	uint slashesToStrip = _directoriesToStrip;
	while (slashesToStrip--) {
		for (uint i = 0; i < filename.size(); i++) {
			if (filename[i] != '\\' && filename[i] != ':')
				continue;
			filename = filename.c_str() + i + 1;
			break;
		}
	}

	Common::String outFilename;
	for (uint i = 0; i < filename.size(); i++) {
		if (filename[i] == '\\' || filename[i] == ':')
			outFilename += '/';
		else
			outFilename += filename[i];
	}
	return outFilename;
}

void ComposerEngine::loadLibrary(uint id) {
	if (getGameType() == GType_ComposerV1 && !_libraries.empty()) {
		// kill the previous page, starting with any scripts running on it

		for (Common::List<OldScript *>::iterator i = _oldScripts.begin(); i != _oldScripts.end(); i++)
			delete *i;
		_oldScripts.clear();

		Library *library = &_libraries.front();
		unloadLibrary(library->_id);
	}

	Common::String filename;

	if (getGameType() == GType_ComposerV1) {
		if (!id || _bookGroup.empty())
			filename = getStringFromConfig("Common", "StartPage");
		else
			filename = getStringFromConfig(_bookGroup, Common::String::format("%d", id));
		filename = mangleFilename(filename);

		// bookGroup is the basename of the path.
		// TODO: tidy this up.
		_bookGroup.clear();
		for (uint i = 0; i < filename.size(); i++) {
			if (filename[i] == '~' || filename[i] == '/' || filename[i] == ':')
				continue;
			for (uint j = 0; j < filename.size(); j++) {
				if (filename[j] == '/') {
					_bookGroup.clear();
					continue;
				}
				if (filename[j] == '.')
					break;
				_bookGroup += filename[j];
			}
			break;
		}
	} else {
		if (!id)
			id = atoi(getStringFromConfig("Common", "StartUp").c_str());
		filename = getFilename("Libs", id);
	}

	Library library;

	library._id = id;
	library._archive = new ComposerArchive();
	if (!library._archive->openFile(filename))
		error("failed to open '%s'", filename.c_str());
	_libraries.push_front(library);

	Library &newLib = _libraries.front();

	Common::Array<uint16> buttonResources = library._archive->getResourceIDList(ID_BUTN);
	for (uint i = 0; i < buttonResources.size(); i++) {
		uint16 buttonId = buttonResources[i];
		Common::SeekableReadStream *stream = library._archive->getResource(ID_BUTN, buttonId);
		Button button(stream, buttonId, getGameType());

		bool inserted = false;
		for (Common::List<Button>::iterator b = newLib._buttons.begin(); b != newLib._buttons.end(); b++) {
			if (button._zorder < b->_zorder)
				continue;
			newLib._buttons.insert(b, button);
			inserted = true;
			break;
		}
		if (!inserted)
			newLib._buttons.push_back(button);
	}

	Common::Array<uint16> ambientResources = library._archive->getResourceIDList(ID_AMBI);
	for (uint i = 0; i < ambientResources.size(); i++) {
		Common::SeekableReadStream *stream = library._archive->getResource(ID_AMBI, ambientResources[i]);
		Button button(stream);
		newLib._buttons.insert(newLib._buttons.begin(), button);
	}

	Common::Array<uint16> accelResources = library._archive->getResourceIDList(ID_ACEL);
	for (uint i = 0; i < accelResources.size(); i++) {
		Common::SeekableReadStream *stream = library._archive->getResource(ID_ACEL, accelResources[i]);
		KeyboardHandler handler;
		handler.keyId = stream->readUint16LE();
		handler.modifierId = stream->readUint16LE();
		handler.scriptId = stream->readUint16LE();
		newLib._keyboardHandlers.push_back(handler);
	}

	Common::Array<uint16> randResources = library._archive->getResourceIDList(ID_RAND);
	for (uint i = 0; i < randResources.size(); i++) {
		Common::SeekableReadStream *stream = library._archive->getResource(ID_RAND, randResources[i]);
		Common::Array<RandomEvent> &events = _randomEvents[randResources[i]];
		uint16 count = stream->readUint16LE();
		for (uint j = 0; j < count; j++) {
			RandomEvent random;
			random.scriptId = stream->readUint16LE();
			random.weight = stream->readUint16LE();
			events.push_back(random);
		}
		delete stream;
	}

	// add background sprite, if it exists
	if (hasResource(ID_BMAP, 1000))
		setBackground(1000);

	// TODO: better CTBL logic
	loadCTBL(1000, 100);

	// Run the startup script.
	runScript(1000, 0, 0, 0);

	_mouseEnabled = true;
	onMouseMove(_lastMousePos);

	runEvent(kEventLoad, id, 0, 0);
}

void ComposerEngine::unloadLibrary(uint id) {
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++) {
		if (i->_id != id)
			continue;

		for (Common::List<Animation *>::iterator j = _anims.begin(); j != _anims.end(); j++) {
			delete *j;
		}
		_anims.clear();
		stopPipes();

		_randomEvents.clear();

		for (Common::List<Sprite>::iterator j = _sprites.begin(); j != _sprites.end(); j++) {
			j->_surface.free();
		}
		_sprites.clear();
		i->_buttons.clear();

		_lastButton = NULL;

		_mixer->stopAll();
		_audioStream = NULL;

		for (uint j = 0; j < _queuedScripts.size(); j++) {
			_queuedScripts[j]._count = 0;
			_queuedScripts[j]._scriptId = 0;
		}

		delete i->_archive;
		_libraries.erase(i);

		runEvent(kEventUnload, id, 0, 0);

		return;
	}

	error("tried to unload library %d, which isn't loaded", id);
}

bool ComposerEngine::hasResource(uint32 tag, uint16 id) {
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
		if (i->_archive->hasResource(tag, id))
			return true;

	return false;
}

Common::SeekableReadStream *ComposerEngine::getResource(uint32 tag, uint16 id) {
	for (Common::List<Library>::iterator i = _libraries.begin(); i != _libraries.end(); i++)
		if (i->_archive->hasResource(tag, id))
			return i->_archive->getResource(tag, id);

	error("No loaded library contains '%s' %04x", tag2str(tag), id);
}

Button::Button(Common::SeekableReadStream *stream, uint16 id, uint gameType) {
	_id = id;

	_type = stream->readUint16LE();
	_active = (_type & 0x8000) ? true : false;
	bool hasRollover = (gameType == GType_ComposerV1) && (_type & 0x4000);
	_type &= 0xfff;
	debug(9, "button %d: type %d, active %d", id, _type, _active);

	uint16 size = 4;
	if (gameType == GType_ComposerV1) {
		stream->skip(2);

		_zorder = 0;
		_scriptId = stream->readUint16LE();
		_scriptIdRollOn = 0;
		_scriptIdRollOff = 0;
	} else {
		_zorder = stream->readUint16LE();
		_scriptId = stream->readUint16LE();
		_scriptIdRollOn = stream->readUint16LE();
		_scriptIdRollOff = stream->readUint16LE();

		stream->skip(4);

		size = stream->readUint16LE();
	}

	switch (_type) {
	case kButtonRect:
	case kButtonEllipse:
		if (size != 4)
			error("button %d of type %d had %d points, not 4", id, _type, size);
		_rect.left = stream->readSint16LE();
		_rect.top = stream->readSint16LE();
		_rect.right = stream->readSint16LE();
		_rect.bottom = stream->readSint16LE();
		break;
	case kButtonSprites:
		if (gameType == GType_ComposerV1)
			error("encountered kButtonSprites in V1 data");
		for (uint i = 0; i < size; i++) {
			_spriteIds.push_back(stream->readUint16LE());
		}
		break;
	default:
		error("unknown button type %d", _type);
	}

	if (hasRollover) {
		_scriptIdRollOn = stream->readUint16LE();
		_scriptIdRollOff = stream->readUint16LE();
	}

	delete stream;
}

// AMBI-style button
Button::Button(Common::SeekableReadStream *stream) {
	_id = 0;
	_zorder = 0;
	_active = true;
	_type = kButtonSprites;
	_scriptIdRollOn = 0;
	_scriptIdRollOff = 0;

	_scriptId = stream->readUint16LE();

	uint16 count = stream->readUint16LE();
	for (uint j = 0; j < count; j++) {
		uint16 spriteId = stream->readUint16LE();
		_spriteIds.push_back(spriteId);
	}

	delete stream;
}

bool Button::contains(const Common::Point &pos) const {
	switch (_type) {
	case kButtonRect:
		return _rect.contains(pos);
	case kButtonEllipse:
		if (!_rect.contains(pos))
			return false;
		{
		int16 a = _rect.width() / 2;
		int16 b = _rect.height() / 2;
		if (!a || !b)
			return false;
		Common::Point adjustedPos = pos - Common::Point(_rect.left + a, _rect.top + b);
		return ((adjustedPos.x*adjustedPos.x)/(a*a) + (adjustedPos.y*adjustedPos.y)/(b*b) < 1);
		}
	case kButtonSprites:
		return false;
	default:
		error("internal error (button type %d)", _type);
	}
}

const Button *ComposerEngine::getButtonFor(const Sprite *sprite, const Common::Point &pos) {
	for (Common::List<Library>::iterator l = _libraries.begin(); l != _libraries.end(); l++) {
		for (Common::List<Button>::iterator i = l->_buttons.reverse_begin(); i != l->_buttons.end(); --i) {
			if (!i->_active)
				continue;

			if (i->_spriteIds.empty()) {
				if (i->contains(pos))
					return &(*i);
				continue;
			}

			if (!sprite)
				continue;

			for (uint j = 0; j < i->_spriteIds.size(); j++) {
				if (i->_spriteIds[j] == sprite->_id)
					return &(*i);
			}
		}
	}

	return NULL;
}

void ComposerEngine::setButtonActive(uint16 id, bool active) {
	for (Common::List<Library>::iterator l = _libraries.begin(); l != _libraries.end(); l++) {
		for (Common::List<Button>::iterator i = l->_buttons.begin(); i != l->_buttons.end(); i++) {
			if (i->_id != id)
				continue;
			i->_active = active;
		}
	}

	onMouseMove(_lastMousePos);
}

} // End of namespace Composer
