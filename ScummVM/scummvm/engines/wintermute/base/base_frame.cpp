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

#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/base_frame.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_dynamic_buffer.h"
#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/base_sub_frame.h"
#include "engines/wintermute/platform_osystem.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "common/str.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseFrame, false)

//////////////////////////////////////////////////////////////////////
BaseFrame::BaseFrame(BaseGame *inGame) : BaseScriptable(inGame, true) {
	_delay = 0;
	_moveX = _moveY = 0;

	_sound = NULL;
	_killSound = false;

	_editorExpanded = false;
	_keyframe = false;
}


//////////////////////////////////////////////////////////////////////
BaseFrame::~BaseFrame() {
	delete _sound;
	_sound = NULL;

	for (uint32 i = 0; i < _subframes.size(); i++) {
		delete _subframes[i];
	}
	_subframes.clear();

	for (uint32 i = 0; i < _applyEvent.size(); i++) {
		delete[] _applyEvent[i];
		_applyEvent[i] = NULL;
	}
	_applyEvent.clear();
}


//////////////////////////////////////////////////////////////////////
bool BaseFrame::draw(int x, int y, BaseObject *registerOwner, float zoomX, float zoomY, bool precise, uint32 alpha, bool allFrames, float rotate, TSpriteBlendMode blendMode) {
	bool res;

	for (uint32 i = 0; i < _subframes.size(); i++) {
		res = _subframes[i]->draw(x, y, registerOwner, zoomX, zoomY, precise, alpha, rotate, blendMode);
		if (DID_FAIL(res)) {
			return res;
		}
	}
	return STATUS_OK;
}

void BaseFrame::stopSound() {
	if (_sound) {
		_sound->stop();
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseFrame::oneTimeDisplay(BaseObject *owner, bool muted) {
	if (_sound && !muted) {
		if (owner) {
			owner->updateOneSound(_sound);
		}
		_sound->play();
		/*
		if (_gameRef->_state == GAME_FROZEN) {
		    _sound->Pause(true);
		}
		*/
	}
	if (owner) {
		for (uint32 i = 0; i < _applyEvent.size(); i++) {
			owner->applyEvent(_applyEvent[i]);
		}
	}
	return STATUS_OK;
}



TOKEN_DEF_START
TOKEN_DEF(DELAY)
TOKEN_DEF(IMAGE)
TOKEN_DEF(TRANSPARENT)
TOKEN_DEF(RECT)
TOKEN_DEF(HOTSPOT)
TOKEN_DEF(2D_ONLY)
TOKEN_DEF(3D_ONLY)
TOKEN_DEF(MIRROR_X)
TOKEN_DEF(MIRROR_Y)
TOKEN_DEF(MOVE)
TOKEN_DEF(ALPHA_COLOR)
TOKEN_DEF(ALPHA)
TOKEN_DEF(SUBFRAME)
TOKEN_DEF(SOUND)
TOKEN_DEF(KEYFRAME)
TOKEN_DEF(DECORATION)
TOKEN_DEF(APPLY_EVENT)
TOKEN_DEF(EDITOR_SELECTED)
TOKEN_DEF(EDITOR_EXPANDED)
TOKEN_DEF(EDITOR_PROPERTY)
TOKEN_DEF(KILL_SOUND)
TOKEN_DEF_END
//////////////////////////////////////////////////////////////////////
bool BaseFrame::loadBuffer(byte *buffer, int lifeTime, bool keepLoaded) {
	TOKEN_TABLE_START(commands)
	TOKEN_TABLE(DELAY)
	TOKEN_TABLE(IMAGE)
	TOKEN_TABLE(TRANSPARENT)
	TOKEN_TABLE(RECT)
	TOKEN_TABLE(HOTSPOT)
	TOKEN_TABLE(2D_ONLY)
	TOKEN_TABLE(3D_ONLY)
	TOKEN_TABLE(MIRROR_X)
	TOKEN_TABLE(MIRROR_Y)
	TOKEN_TABLE(MOVE)
	TOKEN_TABLE(ALPHA_COLOR)
	TOKEN_TABLE(ALPHA)
	TOKEN_TABLE(SUBFRAME)
	TOKEN_TABLE(SOUND)
	TOKEN_TABLE(KEYFRAME)
	TOKEN_TABLE(DECORATION)
	TOKEN_TABLE(APPLY_EVENT)
	TOKEN_TABLE(EDITOR_SELECTED)
	TOKEN_TABLE(EDITOR_EXPANDED)
	TOKEN_TABLE(EDITOR_PROPERTY)
	TOKEN_TABLE(KILL_SOUND)
	TOKEN_TABLE_END

	char *params;
	int cmd;
	BaseParser parser;
	Rect32 rect;
	int r = 255, g = 255, b = 255;
	int ar = 255, ag = 255, ab = 255, alpha = 255;
	int hotspotX = 0, hotspotY = 0;
	bool custoTrans = false;
	bool editorSelected = false;
	bool is2DOnly = false;
	bool is3DOnly = false;
	bool decoration = false;
	bool mirrorX = false;
	bool mirrorY = false;
	BasePlatform::setRectEmpty(&rect);
	char *surface_file = NULL;

	while ((cmd = parser.getCommand((char **)&buffer, commands, &params)) > 0) {
		switch (cmd) {
		case TOKEN_DELAY:
			parser.scanStr(params, "%d", &_delay);
			break;

		case TOKEN_IMAGE:
			surface_file = params;
			break;

		case TOKEN_TRANSPARENT:
			parser.scanStr(params, "%d,%d,%d", &r, &g, &b);
			custoTrans = true;
			break;

		case TOKEN_RECT:
			parser.scanStr(params, "%d,%d,%d,%d", &rect.left, &rect.top, &rect.right, &rect.bottom);
			break;

		case TOKEN_HOTSPOT:
			parser.scanStr(params, "%d,%d", &hotspotX, &hotspotY);
			break;

		case TOKEN_MOVE:
			parser.scanStr(params, "%d,%d", &_moveX, &_moveY);
			break;

		case TOKEN_2D_ONLY:
			parser.scanStr(params, "%b", &is2DOnly);
			break;

		case TOKEN_3D_ONLY:
			parser.scanStr(params, "%b", &is3DOnly);
			break;

		case TOKEN_MIRROR_X:
			parser.scanStr(params, "%b", &mirrorX);
			break;

		case TOKEN_MIRROR_Y:
			parser.scanStr(params, "%b", &mirrorY);
			break;

		case TOKEN_ALPHA_COLOR:
			parser.scanStr(params, "%d,%d,%d", &ar, &ag, &ab);
			break;

		case TOKEN_ALPHA:
			parser.scanStr(params, "%d", &alpha);
			break;

		case TOKEN_EDITOR_SELECTED:
			parser.scanStr(params, "%b", &editorSelected);
			break;

		case TOKEN_EDITOR_EXPANDED:
			parser.scanStr(params, "%b", &_editorExpanded);
			break;

		case TOKEN_KILL_SOUND:
			parser.scanStr(params, "%b", &_killSound);
			break;

		case TOKEN_SUBFRAME: {
			BaseSubFrame *subframe = new BaseSubFrame(_gameRef);
			if (!subframe || DID_FAIL(subframe->loadBuffer((byte *)params, lifeTime, keepLoaded))) {
				delete subframe;
				cmd = PARSERR_GENERIC;
			} else {
				_subframes.add(subframe);
			}
		}
		break;

		case TOKEN_SOUND: {
			if (_sound) {
				delete _sound;
				_sound = NULL;
			}
			_sound = new BaseSound(_gameRef);
			if (!_sound || DID_FAIL(_sound->setSound(params, Audio::Mixer::kSFXSoundType, false))) {
				if (_gameRef->_soundMgr->_soundAvailable) {
					_gameRef->LOG(0, "Error loading sound '%s'.", params);
				}
				delete _sound;
				_sound = NULL;
			}
		}
		break;

		case TOKEN_APPLY_EVENT: {
			char *event = new char[strlen(params) + 1];
			strcpy(event, params);
			_applyEvent.add(event);
		}
		break;

		case TOKEN_KEYFRAME:
			parser.scanStr(params, "%b", &_keyframe);
			break;

		case TOKEN_DECORATION:
			parser.scanStr(params, "%b", &decoration);
			break;

		case TOKEN_EDITOR_PROPERTY:
			parseEditorProperty((byte *)params, false);
			break;
		}
	}
	if (cmd == PARSERR_TOKENNOTFOUND) {
		_gameRef->LOG(0, "Syntax error in FRAME definition");
		return STATUS_FAILED;
	}

	if (cmd == PARSERR_GENERIC) {
		_gameRef->LOG(0, "Error loading FRAME definition");
		return STATUS_FAILED;
	}


	BaseSubFrame *sub = new BaseSubFrame(_gameRef);
	if (surface_file != NULL) {
		if (custoTrans) {
			sub->setSurface(surface_file, false, r, g, b, lifeTime, keepLoaded);
		} else {
			sub->setSurface(surface_file, true, 0, 0, 0, lifeTime, keepLoaded);
		}

		if (!sub->_surface) {
			delete sub;
			_gameRef->LOG(0, "Error loading SUBFRAME");
			return STATUS_FAILED;
		}

		sub->_alpha = BYTETORGBA(ar, ag, ab, alpha);
		if (custoTrans) {
			sub->_transparent = BYTETORGBA(r, g, b, 0xFF);
		}
	}

	if (BasePlatform::isRectEmpty(&rect)) {
		sub->setDefaultRect();
	} else {
		sub->setRect(rect);
	}

	sub->_hotspotX = hotspotX;
	sub->_hotspotY = hotspotY;
	sub->_2DOnly = is2DOnly;
	sub->_3DOnly = is3DOnly;
	sub->_decoration = decoration;
	sub->_mirrorX = mirrorX;
	sub->_mirrorY = mirrorY;


	sub->_editorSelected = editorSelected;
	_subframes.insert_at(0, sub);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFrame::getBoundingRect(Rect32 *rect, int x, int y, float scaleX, float scaleY) {
	if (!rect) {
		return false;
	}
	BasePlatform::setRectEmpty(rect);

	Rect32 subRect;

	for (uint32 i = 0; i < _subframes.size(); i++) {
		_subframes[i]->getBoundingRect(&subRect, x, y, scaleX, scaleY);
		BasePlatform::unionRect(rect, rect, &subRect);
	}
	return true;
}



//////////////////////////////////////////////////////////////////////////
bool BaseFrame::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	buffer->putTextIndent(indent, "FRAME {\n");
	buffer->putTextIndent(indent + 2, "DELAY = %d\n", _delay);

	if (_moveX != 0 || _moveY != 0) {
		buffer->putTextIndent(indent + 2, "MOVE {%d, %d}\n", _moveX, _moveY);
	}

	if (_sound && _sound->getFilename()) {
		buffer->putTextIndent(indent + 2, "SOUND=\"%s\"\n", _sound->getFilename());
	}

	buffer->putTextIndent(indent + 2, "KEYFRAME=%s\n", _keyframe ? "TRUE" : "FALSE");

	if (_killSound) {
		buffer->putTextIndent(indent + 2, "KILL_SOUND=%s\n", _killSound ? "TRUE" : "FALSE");
	}

	if (_editorExpanded) {
		buffer->putTextIndent(indent + 2, "EDITOR_EXPANDED=%s\n", _editorExpanded ? "TRUE" : "FALSE");
	}

	if (_subframes.size() > 0) {
		_subframes[0]->saveAsText(buffer, indent, false);
	}

	for (uint32 i = 1; i < _subframes.size(); i++) {
		_subframes[i]->saveAsText(buffer, indent + 2);
	}

	for (uint32 i = 0; i < _applyEvent.size(); i++) {
		buffer->putTextIndent(indent + 2, "APPLY_EVENT=\"%s\"\n", _applyEvent[i]);
	}

	BaseClass::saveAsText(buffer, indent + 2);


	buffer->putTextIndent(indent, "}\n\n");

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseFrame::persist(BasePersistenceManager *persistMgr) {
	BaseScriptable::persist(persistMgr);

	_applyEvent.persist(persistMgr);
	persistMgr->transfer(TMEMBER(_delay));
	persistMgr->transfer(TMEMBER(_editorExpanded));
	persistMgr->transfer(TMEMBER(_keyframe));
	persistMgr->transfer(TMEMBER(_killSound));
	persistMgr->transfer(TMEMBER(_moveX));
	persistMgr->transfer(TMEMBER(_moveY));
	persistMgr->transfer(TMEMBER(_sound));
	_subframes.persist(persistMgr);

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseFrame::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {

	//////////////////////////////////////////////////////////////////////////
	// GetSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetSound") == 0) {
		stack->correctParams(0);

		if (_sound && _sound->getFilename()) {
			stack->pushString(_sound->getFilename());
		} else {
			stack->pushNULL();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSound
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SetSound") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		delete _sound;
		_sound = NULL;

		if (!val->isNULL()) {
			_sound = new BaseSound(_gameRef);
			if (!_sound || DID_FAIL(_sound->setSound(val->getString(), Audio::Mixer::kSFXSoundType, false))) {
				stack->pushBool(false);
				delete _sound;
				_sound = NULL;
			} else {
				stack->pushBool(true);
			}
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSubframe
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "GetSubframe") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt(-1);
		if (index < 0 || index >= (int32)_subframes.size()) {
			script->runtimeError("Frame.GetSubframe: Subframe index %d is out of range.", index);
			stack->pushNULL();
		} else {
			stack->pushNative(_subframes[index], true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteSubframe") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		if (val->isInt()) {
			int index = val->getInt(-1);
			if (index < 0 || index >= (int32)_subframes.size()) {
				script->runtimeError("Frame.DeleteSubframe: Subframe index %d is out of range.", index);
			}
		} else {
			BaseSubFrame *sub = (BaseSubFrame *)val->getNative();
			for (uint32 i = 0; i < _subframes.size(); i++) {
				if (_subframes[i] == sub) {
					delete _subframes[i];
					_subframes.remove_at(i);
					break;
				}
			}
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddSubframe") == 0) {
		stack->correctParams(1);
		ScValue *val = stack->pop();
		const char *filename = NULL;
		if (!val->isNULL()) {
			filename = val->getString();
		}

		BaseSubFrame *sub = new BaseSubFrame(_gameRef);
		if (filename != NULL) {
			sub->setSurface(filename);
			sub->setDefaultRect();
		}
		_subframes.add(sub);

		stack->pushNative(sub, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// InsertSubframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "InsertSubframe") == 0) {
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

		BaseSubFrame *sub = new BaseSubFrame(_gameRef);
		if (filename != NULL) {
			sub->setSurface(filename);
		}

		if (index >= (int32)_subframes.size()) {
			_subframes.add(sub);
		} else {
			_subframes.insert_at(index, sub);
		}

		stack->pushNative(sub, true);
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSubframe") == 0) {
		stack->correctParams(1);
		int index = stack->pop()->getInt(-1);
		if (index < 0 || index >= (int32)_applyEvent.size()) {
			script->runtimeError("Frame.GetEvent: Event index %d is out of range.", index);
			stack->pushNULL();
		} else {
			stack->pushString(_applyEvent[index]);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AddEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AddEvent") == 0) {
		stack->correctParams(1);
		const char *event = stack->pop()->getString();
		for (uint32 i = 0; i < _applyEvent.size(); i++) {
			if (scumm_stricmp(_applyEvent[i], event) == 0) {
				stack->pushNULL();
				return STATUS_OK;
			}
		}
		_applyEvent.add(event);
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// DeleteEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "DeleteEvent") == 0) {
		stack->correctParams(1);
		const char *event = stack->pop()->getString();
		for (uint32 i = 0; i < _applyEvent.size(); i++) {
			if (scumm_stricmp(_applyEvent[i], event) == 0) {
				delete[] _applyEvent[i];
				_applyEvent.remove_at(i);
				break;
			}
		}
		stack->pushNULL();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.size() == 1) {
			return _subframes[0]->scCallMethod(script, stack, thisStack, name);
		} else {
			return BaseScriptable::scCallMethod(script, stack, thisStack, name);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseFrame::scGetProperty(const Common::String &name) {
	if (!_scValue) {
		_scValue = new ScValue(_gameRef);
	}
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type (RO)
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("frame");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Delay") {
		_scValue->setInt(_delay);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyframe
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Keyframe") {
		_scValue->setBool(_keyframe);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// KillSounds
	//////////////////////////////////////////////////////////////////////////
	else if (name == "KillSounds") {
		_scValue->setBool(_killSound);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MoveX") {
		_scValue->setInt(_moveX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "MoveY") {
		_scValue->setInt(_moveY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumSubframes (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NumSubframes") {
		_scValue->setInt(_subframes.size());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NumEvents (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NumEvents") {
		_scValue->setInt(_applyEvent.size());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.size() == 1) {
			return _subframes[0]->scGetProperty(name);
		} else {
			return BaseScriptable::scGetProperty(name);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseFrame::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Delay
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Delay") == 0) {
		_delay = MAX(0, value->getInt());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Keyframe
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Keyframe") == 0) {
		_keyframe = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// KillSounds
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "KillSounds") == 0) {
		_killSound = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveX") == 0) {
		_moveX = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// MoveY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "MoveY") == 0) {
		_moveY = value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	else {
		if (_subframes.size() == 1) {
			return _subframes[0]->scSetProperty(name, value);
		} else {
			return BaseScriptable::scSetProperty(name, value);
		}
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseFrame::scToString() {
	return "[frame]";
}

} // end of namespace Wintermute
