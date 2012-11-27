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

#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/utils/path_util.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/gfx/base_surface.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_frame.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/base/base_file_manager.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseSprite, false)

//////////////////////////////////////////////////////////////////////
BaseSprite::BaseSprite(BaseGame *inGame, BaseObject *Owner) : BaseScriptHolder(inGame) {
	_editorAllFrames = false;
	_owner = Owner;
	setDefaults();
}


//////////////////////////////////////////////////////////////////////
BaseSprite::~BaseSprite() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
void BaseSprite::setDefaults() {
	_currentFrame = -1;
	_looping = false;
	_lastFrameTime = 0;
	setFilename(NULL);
	_finished = false;
	_changed = false;
	_paused = false;
	_continuous = false;
	_moveX = _moveY = 0;

	_editorMuted = false;
	_editorBgFile = NULL;
	_editorBgOffsetX = _editorBgOffsetY = 0;
	_editorBgAlpha = 0xFF;
	_streamed = false;
	_streamedKeepLoaded = false;

	setName("");

	_precise = true;
}


//////////////////////////////////////////////////////////////////////////
void BaseSprite::cleanup() {
	BaseScriptHolder::cleanup();

	for (uint32 i = 0; i < _frames.size(); i++) {
		delete _frames[i];
	}
	_frames.clear();

	delete[] _editorBgFile;
	_editorBgFile = NULL;

	setDefaults();
}


//////////////////////////////////////////////////////////////////////////
bool BaseSprite::draw(int x, int y, BaseObject *registerOwner, float zoomX, float zoomY, uint32 alpha) {
	getCurrentFrame(zoomX, zoomY);
	if (_currentFrame < 0 || _currentFrame >= (int32)_frames.size()) {
		return STATUS_OK;
	}

	// move owner if allowed to
	if (_changed && _owner && _owner->_movable) {
		_owner->_posX += _moveX;
		_owner->_posY += _moveY;
		_owner->afterMove();

		x = _owner->_posX;
		y = _owner->_posY;
	}

	// draw frame
	return display(x, y, registerOwner, zoomX, zoomY, alpha);
}

bool BaseSprite::isChanged() {
	return _changed;
}

bool BaseSprite::isFinished() {
	return _finished;
}

//////////////////////////////////////////////////////////////////////
bool BaseSprite::loadFile(const Common::String &filename, int lifeTime, TSpriteCacheType cacheType) {
	Common::SeekableReadStream *file = BaseFileManager::getEngineInstance()->openFile(filename);
	if (!file) {
		_gameRef->LOG(0, "BaseSprite::LoadFile failed for file '%s'", filename.c_str());
		if (_gameRef->_debugDebugMode) {
			return loadFile("invalid_debug.bmp", lifeTime, cacheType);
		} else {
			return loadFile("invalid.bmp", lifeTime, cacheType);
		}
	} else {
		BaseFileManager::getEngineInstance()->closeFile(file);
		file = NULL;
	}

	bool ret = STATUS_FAILED;

	AnsiString filePrefix = filename;
	AnsiString ext = PathUtil::getExtension(filename);
	ext.toLowercase();
	filePrefix.toLowercase();
	if (filePrefix.hasPrefix("savegame:") || (ext == "bmp") || (ext == "tga") || (ext == "png") || (ext == "jpg")) {
		BaseFrame *frame = new BaseFrame(_gameRef);
		BaseSubFrame *subframe = new BaseSubFrame(_gameRef);
		subframe->setSurface(filename, true, 0, 0, 0, lifeTime, true);
		if (subframe->_surface == NULL) {
			_gameRef->LOG(0, "Error loading simple sprite '%s'", filename.c_str());
			ret = STATUS_FAILED;
			delete frame;
			delete subframe;
		} else {
			subframe->setDefaultRect();
			frame->_subframes.add(subframe);
			_frames.add(frame);
			_currentFrame = 0;
			ret = STATUS_OK;
		}
	} else {
		byte *buffer = BaseFileManager::getEngineInstance()->readWholeFile(filename);
		if (buffer) {
			if (DID_FAIL(ret = loadBuffer(buffer, true, lifeTime, cacheType))) {
				_gameRef->LOG(0, "Error parsing SPRITE file '%s'", filename.c_str());
			} else {
				ret = STATUS_OK;
			}
			delete[] buffer;
		}
	}

	setFilename(filename.c_str());

	return ret;
}



TOKEN_DEF_START
TOKEN_DEF(CONTINUOUS)
TOKEN_DEF(SPRITE)
TOKEN_DEF(LOOPING)
TOKEN_DEF(FRAME)
TOKEN_DEF(NAME)
TOKEN_DEF(PRECISE)
TOKEN_DEF(EDITOR_MUTED)
TOKEN_DEF(STREAMED_KEEP_LOADED)
TOKEN_DEF(STREAMED)
TOKEN_DEF(SCRIPT)
TOKEN_DEF(EDITOR_BG_FILE)
TOKEN_DEF(EDITOR_BG_OFFSET_X)
TOKEN_DEF(EDITOR_BG_OFFSET_Y)
TOKEN_DEF(EDITOR_BG_ALPHA)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
bool BaseSprite::loadBuffer(byte *buffer, bool complete, int lifeTime, TSpriteCacheType cacheType) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(CONTINUOUS)
	TOKEN_TABLE(SPRITE)
	TOKEN_TABLE(LOOPING)
	TOKEN_TABLE(FRAME)
	TOKEN_TABLE(NAME)
	TOKEN_TABLE(PRECISE)
	TOKEN_TABLE(EDITOR_MUTED)
	TOKEN_TABLE(STREAMED_KEEP_LOADED)
	TOKEN_TABLE(STREAMED)
	TOKEN_TABLE(SCRIPT)
	TOKEN_TABLE(EDITOR_BG_FILE)
	TOKEN_TABLE(EDITOR_BG_OFFSET_X)
	TOKEN_TABLE(EDITOR_BG_OFFSET_Y)
	TOKEN_TABLE(EDITOR_BG_ALPHA)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE_END

	byte *params;
	int cmd;
	BaseParser parser;

	cleanup();


	if (complete) {
		if (parser.getCommand((char **)&buffer, commands, (char **)&params) != TOKEN_SPRITE) {
			_gameRef->LOG(0, "'SPRITE' keyword expected.");
			return STATUS_FAILED;
		}
		buffer = params;
	}

	int frameCount = 1;
	BaseFrame *frame;
	while ((cmd = parser.getCommand((char **)&buffer, commands, (char **)&params)) > 0) {
		switch (cmd) {
		case TOKEN_CONTINUOUS:
			parser.scanStr((char *)params, "%b", &_continuous);
			break;

		case TOKEN_EDITOR_MUTED:
			parser.scanStr((char *)params, "%b", &_editorMuted);
			break;

		case TOKEN_SCRIPT:
			addScript((char *)params);
			break;

		case TOKEN_LOOPING:
			parser.scanStr((char *)params, "%b", &_looping);
			break;

		case TOKEN_PRECISE:
			parser.scanStr((char *)params, "%b", &_precise);
			break;

		case TOKEN_STREAMED:
			parser.scanStr((char *)params, "%b", &_streamed);
			if (_streamed && lifeTime == -1) {
				lifeTime = 500;
				cacheType = CACHE_ALL;
			}
			break;

		case TOKEN_STREAMED_KEEP_LOADED:
			parser.scanStr((char *)params, "%b", &_streamedKeepLoaded);
			break;

		case TOKEN_NAME:
			setName((char *)params);
			break;

		case TOKEN_EDITOR_BG_FILE:
			if (_gameRef->_editorMode) {
				delete[] _editorBgFile;
				_editorBgFile = new char[strlen((char *)params) + 1];
				if (_editorBgFile) {
					strcpy(_editorBgFile, (char *)params);
				}
			}
			break;

		case TOKEN_EDITOR_BG_OFFSET_X:
			parser.scanStr((char *)params, "%d", &_editorBgOffsetX);
			break;

		case TOKEN_EDITOR_BG_OFFSET_Y:
			parser.scanStr((char *)params, "%d", &_editorBgOffsetY);
			break;

		case TOKEN_EDITOR_BG_ALPHA:
			parser.scanStr((char *)params, "%d", &_editorBgAlpha);
			_editorBgAlpha = MIN(_editorBgAlpha, 255);
			_editorBgAlpha = MAX(_editorBgAlpha, 0);
			break;

		case TOKEN_FRAME: {
			int frameLifeTime = lifeTime;
			if (cacheType == CACHE_HALF && frameCount % 2 != 1) {
				frameLifeTime = -1;
			}

			frame = new BaseFrame(_gameRef);

			if (DID_FAIL(frame->loadBuffer(params, frameLifeTime, _streamedKeepLoaded))) {
				delete frame;
				_gameRef->LOG(0, "Error parsing frame %d", frameCount);
				return STATUS_FAILED;
			}

			_frames.add(frame);
			frameCount++;
			if (_currentFrame == -1) {
				_currentFrame = 0;
			}
		}
		break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty(params, false);
			break;
		}
	}

	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in SPRITE definition");
		return STATUS_FAILED;
	}
	_canBreak = !_continuous;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////
void BaseSprite::reset() {
	if (_frames.size() > 0) {
		_currentFrame = 0;
	} else {
		_currentFrame = -1;
	}

	killAllSounds();

	_lastFrameTime = 0;
	_finished = false;
	_moveX = _moveY = 0;
}


//////////////////////////////////////////////////////////////////////
bool BaseSprite::getCurrentFrame(float zoomX, float zoomY) {
	//if (_owner && _owner->_freezable && _gameRef->_state == GAME_FROZEN) return true;

	if (_currentFrame == -1) {
		return false;
	}

	uint32 timer;
	if (_owner && _owner->_freezable) {
		timer = _gameRef->_timer;
	} else {
		timer = _gameRef->_liveTimer;
	}

	int lastFrame = _currentFrame;

	// get current frame
	if (!_paused && !_finished && timer >= _lastFrameTime + _frames[_currentFrame]->_delay && _lastFrameTime != 0) {
		if (_currentFrame < (int32)_frames.size() - 1) {
			_currentFrame++;
			if (_continuous) {
				_canBreak = (_currentFrame == (int32)_frames.size() - 1);
			}
		} else {
			if (_looping) {
				_currentFrame = 0;
				_canBreak = true;
			} else {
				_finished = true;
				_canBreak = true;
			}
		}

		_lastFrameTime = timer;
	}

	_changed = (lastFrame != _currentFrame || (_looping && (int32)_frames.size() == 1));

	if (_lastFrameTime == 0) {
		_lastFrameTime = timer;
		_changed = true;
		if (_continuous) {
			_canBreak = (_currentFrame == (int32)_frames.size() - 1);
		}
	}

	if (_changed) {
		_moveX = _frames[_currentFrame]->_moveX;
		_moveY = _frames[_currentFrame]->_moveY;

		if (zoomX != 100 || zoomY != 100) {
			_moveX = (int)((float)_moveX * (float)(zoomX / 100.0f));
			_moveY = (int)((float)_moveY * (float)(zoomY / 100.0f));
		}
	}

	return _changed;
}


//////////////////////////////////////////////////////////////////////
bool BaseSprite::display(int x, int y, BaseObject *registerVal, float zoomX, float zoomY, uint32 alpha, float rotate, TSpriteBlendMode blendMode) {
	if (_currentFrame < 0 || _currentFrame >= (int32)_frames.size()) {
		return STATUS_OK;
	}

	// on change...
	if (_changed) {
		if (_frames[_currentFrame]->_killSound) {
			killAllSounds();
		}
		applyEvent("FrameChanged");
		_frames[_currentFrame]->oneTimeDisplay(_owner, _gameRef->_editorMode && _editorMuted);
	}

	// draw frame
	return _frames[_currentFrame]->draw(x - _gameRef->_offsetX, y - _gameRef->_offsetY, registerVal, zoomX, zoomY, _precise, alpha, _editorAllFrames, rotate, blendMode);
}


//////////////////////////////////////////////////////////////////////////
BaseSurface *BaseSprite::getSurface() {
	// only used for animated textures for 3D models
	if (_currentFrame < 0 || _currentFrame >= (int32)_frames.size()) {
		return NULL;
	}
	BaseFrame *frame = _frames[_currentFrame];
	if (frame && frame->_subframes.size() > 0) {
		BaseSubFrame *subframe = frame->_subframes[0];
		if (subframe) {
			return subframe->_surface;
		} else {
			return NULL;
		}
	} else {
		return NULL;
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseSprite::getBoundingRect(Rect32 *rect, int x, int y, float scaleX, float scaleY) {
	if (!rect) {
		return false;
	}

	BasePlatform::setRectEmpty(rect);
	for (uint32 i = 0; i < _frames.size(); i++) {
		Rect32 frame;
		Rect32 temp;
		BasePlatform::copyRect(&temp, rect);
		_frames[i]->getBoundingRect(&frame, x, y, scaleX, scaleY);
		BasePlatform::unionRect(rect, &temp, &frame);
	}
	return true;
}

//////////////////////////////////////////////////////////////////////////
bool BaseSprite::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "SPRITE {\n");
	buffer->putTextIndent(indent + 2, "NAME=\"%s\"\n", getName());
	buffer->putTextIndent(indent + 2, "LOOPING=%s\n", _looping ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "CONTINUOUS=%s\n", _continuous ? "TRUE" : "FALSE");
	buffer->putTextIndent(indent + 2, "PRECISE=%s\n", _precise ? "TRUE" : "FALSE");
	if (_streamed) {
		buffer->putTextIndent(indent + 2, "STREAMED=%s\n", _streamed ? "TRUE" : "FALSE");

		if (_streamedKeepLoaded) {
			buffer->putTextIndent(indent + 2, "STREAMED_KEEP_LOADED=%s\n", _streamedKeepLoaded ? "TRUE" : "FALSE");
		}
	}

	if (_editorMuted) {
		buffer->putTextIndent(indent + 2, "EDITOR_MUTED=%s\n", _editorMuted ? "TRUE" : "FALSE");
	}

	if (_editorBgFile) {
		buffer->putTextIndent(indent + 2, "EDITOR_BG_FILE=\"%s\"\n", _editorBgFile);
		buffer->putTextIndent(indent + 2, "EDITOR_BG_OFFSET_X=%d\n", _editorBgOffsetX);
		buffer->putTextIndent(indent + 2, "EDITOR_BG_OFFSET_Y=%d\n", _editorBgOffsetY);
		buffer->putTextIndent(indent + 2, "EDITOR_BG_ALPHA=%d\n", _editorBgAlpha);
	}

	BaseScriptHolder::saveAsText(buffer, indent + 2);

	// scripts
	for (uint32 i = 0; i < _scripts.size(); i++) {
		buffer->putTextIndent(indent + 2, "SCRIPT=\"%s\"\n", _scripts[i]->_filename);
	}

	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->saveAsText(buffer, indent + 2);
	}

	buffer->putTextIndent(indent, "}\n\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseSprite::persist(BasePersistenceManager *persistMgr) {
	BaseScriptHolder::persist(persistMgr);

	persistMgr->transfer(TMEMBER(_canBreak));
	persistMgr->transfer(TMEMBER(_changed));
	persistMgr->transfer(TMEMBER(_paused));
	persistMgr->transfer(TMEMBER(_continuous));
	persistMgr->transfer(TMEMBER(_currentFrame));
	persistMgr->transfer(TMEMBER(_editorAllFrames));
	persistMgr->transfer(TMEMBER(_editorBgAlpha));
	persistMgr->transfer(TMEMBER(_editorBgFile));
	persistMgr->transfer(TMEMBER(_editorBgOffsetX));
	persistMgr->transfer(TMEMBER(_editorBgOffsetY));
	persistMgr->transfer(TMEMBER(_editorMuted));
	persistMgr->transfer(TMEMBER(_finished));

	_frames.persist(persistMgr);

	persistMgr->transfer(TMEMBER(_lastFrameTime));
	persistMgr->transfer(TMEMBER(_looping));
	persistMgr->transfer(TMEMBER(_moveX));
	persistMgr->transfer(TMEMBER(_moveY));
	persistMgr->transfer(TMEMBER(_owner));
	persistMgr->transfer(TMEMBER(_precise));
	persistMgr->transfer(TMEMBER(_streamed));
	persistMgr->transfer(TMEMBER(_streamedKeepLoaded));


	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseSprite::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {
	//////////////////////////////////////////////////////////////////////////
	// GetFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetFrame") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt(-1);
		if (index < 0 || index >= (int32)_frames.size()) {
			script->runtimeError("Sprite.GetFrame: Frame index %d is out of range.", index);
			stack->pushNULL();
		} else {
			stack->pushNative(_frames[index], true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteFrame") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		if (val->isInt()) {
			int index = val->getInt(-1);
			if (index < 0 || index >= (int32)_frames.size()) {
				script->runtimeError("Sprite.DeleteFrame: Frame index %d is out of range.", index);
			}
		} else {
			BaseFrame *frame = (BaseFrame *)val->getNative();
			for (uint32 i = 0; i < _frames.size(); i++) {
				if (_frames[i] == frame) {
					if (i == (uint32)_currentFrame) {
						_lastFrameTime = 0;
					}
					delete _frames[i];
					_frames.remove_at(i);
					break;
				}
			}
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Reset
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Reset") == 0) {
		stack->correctParams(0);
		reset();
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddFrame") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		const char *filename = NULL;
		if (!val->isNULL()) {
			filename = val->getString();
		}

		BaseFrame *frame = new BaseFrame(_gameRef);
		if (filename != NULL) {
			BaseSubFrame *sub = new BaseSubFrame(_gameRef);
			if (DID_SUCCEED(sub->setSurface(filename))) {
				sub->setDefaultRect();
				frame->_subframes.add(sub);
			} else {
				delete sub;
			}
		}
		_frames.add(frame);

		stack->pushNative(frame, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertFrame
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertFrame") == 0) {
		stack->correctParams(2);
		int index = stack->pop()->getInt();
		if (index < 0) {
			index = 0;
		}

		ScValue *val = stack->pop();
		const char *filename = NULL;
		if (!val->isNULL()) {
			filename = val->getString();
		}

		BaseFrame *frame = new BaseFrame(_gameRef);
		if (filename != NULL) {
			BaseSubFrame *sub = new BaseSubFrame(_gameRef);
			if (DID_SUCCEED(sub->setSurface(filename))) {
				frame->_subframes.add(sub);
			} else {
				delete sub;
			}
		}

		if (index >= (int32)_frames.size()) {
			_frames.add(frame);
		} else {
			_frames.insert_at(index, frame);
		}

		stack->pushNative(frame, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Pause
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Pause") == 0) {
		stack->correctParams(0);
		_paused = true;
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Play
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Play") == 0) {
		stack->correctParams(0);
		_paused = false;
		stack->pushNULL();
		return STATUS_OK;
	} else {
		return BaseScriptHolder::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseSprite::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("sprite");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumFrames (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NumFrames") {
		_scValue->setInt(_frames.size());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	else if (name == "CurrentFrame") {
		_scValue->setInt(_currentFrame);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (name == "PixelPerfect") {
		_scValue->setBool(_precise);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Looping") {
		_scValue->setBool(_looping);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Owner (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Owner") {
		if (_owner == NULL) {
			_scValue->setNULL();
		} else {
			_scValue->setNative(_owner, true);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Finished (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Finished") {
		_scValue->setBool(_finished);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Paused (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Paused") {
		_scValue->setBool(_paused);
		return _scValue;
	} else {
		return BaseScriptHolder::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseSprite::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// CurrentFrame
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "CurrentFrame") == 0) {
		_currentFrame = value->getInt(0);
		if (_currentFrame >= (int32)_frames.size() || _currentFrame < 0) {
			_currentFrame = -1;
		}
		_lastFrameTime = 0;
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PixelPerfect
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PixelPerfect") == 0) {
		_precise = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Looping
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Looping") == 0) {
		_looping = value->getBool();
		return STATUS_OK;
	} else {
		return BaseScriptHolder::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseSprite::scToString() {
	return "[sprite]";
}


//////////////////////////////////////////////////////////////////////////
bool BaseSprite::killAllSounds() {
	for (uint32 i = 0; i < _frames.size(); i++) {
		_frames[i]->stopSound();
	}
	return STATUS_OK;
}

} // end of namespace Wintermute
