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

#include "engines/wintermute/base/base_object.h"
#include "engines/wintermute/base/base_parser.h"
#include "engines/wintermute/base/scriptables/script_value.h"
#include "engines/wintermute/base/scriptables/script_stack.h"
#include "engines/wintermute/base/sound/base_sound.h"
#include "engines/wintermute/base/sound/base_sound_manager.h"
#include "engines/wintermute/base/base_game.h"
#include "engines/wintermute/base/base_string_table.h"
#include "engines/wintermute/base/base_sprite.h"
#include "engines/wintermute/platform_osystem.h"

namespace Wintermute {

IMPLEMENT_PERSISTENT(BaseObject, false)

//////////////////////////////////////////////////////////////////////
BaseObject::BaseObject(BaseGame *inGame) : BaseScriptHolder(inGame) {
	_posX = _posY = 0;
	_movable = true;
	_zoomable = true;
	_registrable = true;
	_shadowable = true;
	_rotatable = false;
	_is3D = false;

	_alphaColor = 0;
	_scale = -1;
	_relativeScale = 0;

	_scaleX = -1;
	_scaleY = -1;

	_ready = true;

	_soundEvent = NULL;

	_iD = _gameRef->getSequence();

	BasePlatform::setRectEmpty(&_rect);
	_rectSet = false;

	_cursor = NULL;
	_activeCursor = NULL;
	_sharedCursors = false;

	_sFX = NULL;
	_sFXStart = 0;
	_sFXVolume = 100;
	_autoSoundPanning = true;

	_editorAlwaysRegister = false;
	_editorSelected = false;

	_editorOnly = false;

	_rotate = 0.0f;
	_rotateValid = false;
	_relativeRotate = 0.0f;

	for (int i = 0; i < 7; i++) {
		_caption[i] = NULL;
	}
	_saveState = true;

	_nonIntMouseEvents = false;

	// sound FX
	_sFXType = SFX_NONE;
	_sFXParam1 = _sFXParam2 = _sFXParam3 = _sFXParam4 = 0;

	_blendMode = BLEND_NORMAL;
}


//////////////////////////////////////////////////////////////////////
BaseObject::~BaseObject() {
	cleanup();
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::cleanup() {
	if (_gameRef && _gameRef->_activeObject == this) {
		_gameRef->_activeObject = NULL;
	}

	BaseScriptHolder::cleanup();
	delete[] _soundEvent;
	_soundEvent = NULL;

	if (!_sharedCursors) {
		delete _cursor;
		delete _activeCursor;
		_cursor = NULL;
		_activeCursor = NULL;
	}
	delete _sFX;
	_sFX = NULL;

	for (int i = 0; i < 7; i++) {
		delete[] _caption[i];
		_caption[i] = NULL;
	}

	_sFXType = SFX_NONE;
	_sFXParam1 = _sFXParam2 = _sFXParam3 = _sFXParam4 = 0;

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
void BaseObject::setCaption(const char *caption, int caseVal) {
	if (caseVal == 0) {
		caseVal = 1;
	}
	if (caseVal < 1 || caseVal > 7) {
		return;
	}

	delete[] _caption[caseVal - 1];
	_caption[caseVal - 1] = new char[strlen(caption) + 1];
	if (_caption[caseVal - 1]) {
		strcpy(_caption[caseVal - 1], caption);
		_gameRef->_stringTable->expand(&_caption[caseVal - 1]);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseObject::getCaption(int caseVal) {
	if (caseVal == 0) {
		caseVal = 1;
	}
	if (caseVal < 1 || caseVal > 7 || _caption[caseVal - 1] == NULL) {
		return "";
	} else {
		return _caption[caseVal - 1];
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::listen(BaseScriptHolder *param1, uint32 param2) {
	return STATUS_FAILED;
}


//////////////////////////////////////////////////////////////////////////
// high level scripting interface
//////////////////////////////////////////////////////////////////////////
bool BaseObject::scCallMethod(ScScript *script, ScStack *stack, ScStack *thisStack, const char *name) {

	//////////////////////////////////////////////////////////////////////////
	// SkipTo
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "SkipTo") == 0) {
		stack->correctParams(2);
		_posX = stack->pop()->getInt();
		_posY = stack->pop()->getInt();
		afterMove();
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Caption") == 0) {
		stack->correctParams(1);
		stack->pushString(getCaption(stack->pop()->getInt()));

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetCursor") == 0) {
		stack->correctParams(1);
		if (DID_SUCCEED(setCursor(stack->pop()->getString()))) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RemoveCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RemoveCursor") == 0) {
		stack->correctParams(0);
		if (!_sharedCursors) {
			delete _cursor;
			_cursor = NULL;
		} else {
			_cursor = NULL;

		}
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetCursor") == 0) {
		stack->correctParams(0);
		if (!_cursor || !_cursor->getFilename()) {
			stack->pushNULL();
		} else {
			stack->pushString(_cursor->getFilename());
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetCursorObject
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetCursorObject") == 0) {
		stack->correctParams(0);
		if (!_cursor) {
			stack->pushNULL();
		} else {
			stack->pushNative(_cursor, true);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// HasCursor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "HasCursor") == 0) {
		stack->correctParams(0);

		if (_cursor) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetCaption") == 0) {
		stack->correctParams(2);
		setCaption(stack->pop()->getString(), stack->pop()->getInt());
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// LoadSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "LoadSound") == 0) {
		stack->correctParams(1);
		const char *filename = stack->pop()->getString();
		if (DID_SUCCEED(playSFX(filename, false, false))) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlaySound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlaySound") == 0) {
		stack->correctParams(3);

		const char *filename;
		bool looping;
		uint32 loopStart;

		ScValue *val1 = stack->pop();
		ScValue *val2 = stack->pop();
		ScValue *val3 = stack->pop();

		if (val1->_type == VAL_BOOL) {
			filename = NULL;
			looping = val1->getBool();
			loopStart = val2->getInt();
		} else {
			if (val1->isNULL()) {
				filename = NULL;
			} else {
				filename = val1->getString();
			}
			looping = val2->isNULL() ? false : val2->getBool();
			loopStart = val3->getInt();
		}

		if (DID_FAIL(playSFX(filename, looping, true, NULL, loopStart))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PlaySoundEvent
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PlaySoundEvent") == 0) {
		stack->correctParams(2);

		const char *filename;
		const char *eventName;

		ScValue *val1 = stack->pop();
		ScValue *val2 = stack->pop();

		if (val2->isNULL()) {
			filename = NULL;
			eventName = val1->getString();
		} else {
			filename = val1->getString();
			eventName = val2->getString();
		}

		if (DID_FAIL(playSFX(filename, false, true, eventName))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// StopSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "StopSound") == 0) {
		stack->correctParams(0);

		if (DID_FAIL(stopSFX())) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// PauseSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "PauseSound") == 0) {
		stack->correctParams(0);

		if (DID_FAIL(pauseSFX())) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ResumeSound
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ResumeSound") == 0) {
		stack->correctParams(0);

		if (DID_FAIL(resumeSFX())) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// IsSoundPlaying
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "IsSoundPlaying") == 0) {
		stack->correctParams(0);

		if (_sFX && _sFX->isPlaying()) {
			stack->pushBool(true);
		} else {
			stack->pushBool(false);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSoundPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSoundPosition") == 0) {
		stack->correctParams(1);

		uint32 time = stack->pop()->getInt();
		if (DID_FAIL(setSFXTime(time))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSoundPosition
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSoundPosition") == 0) {
		stack->correctParams(0);

		if (!_sFX) {
			stack->pushInt(0);
		} else {
			stack->pushInt(_sFX->getPositionTime());
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SetSoundVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SetSoundVolume") == 0) {
		stack->correctParams(1);

		int volume = stack->pop()->getInt();
		if (DID_FAIL(setSFXVolume(volume))) {
			stack->pushBool(false);
		} else {
			stack->pushBool(true);
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// GetSoundVolume
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "GetSoundVolume") == 0) {
		stack->correctParams(0);

		if (!_sFX) {
			stack->pushInt(_sFXVolume);
		} else {
			stack->pushInt(_sFX->getVolumePercent());
		}
		return STATUS_OK;
	}


	//////////////////////////////////////////////////////////////////////////
	// SoundFXNone
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundFXNone") == 0) {
		stack->correctParams(0);
		_sFXType = SFX_NONE;
		_sFXParam1 = 0;
		_sFXParam2 = 0;
		_sFXParam3 = 0;
		_sFXParam4 = 0;
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundFXEcho
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundFXEcho") == 0) {
		stack->correctParams(4);
		_sFXType = SFX_ECHO;
		_sFXParam1 = (float)stack->pop()->getFloat(0); // Wet/Dry Mix [%] (0-100)
		_sFXParam2 = (float)stack->pop()->getFloat(0); // Feedback [%] (0-100)
		_sFXParam3 = (float)stack->pop()->getFloat(333.0f); // Left Delay [ms] (1-2000)
		_sFXParam4 = (float)stack->pop()->getFloat(333.0f); // Right Delay [ms] (1-2000)
		stack->pushNULL();

		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundFXReverb
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundFXReverb") == 0) {
		stack->correctParams(4);
		_sFXType = SFX_REVERB;
		_sFXParam1 = (float)stack->pop()->getFloat(0); // In Gain [dB] (-96 - 0)
		_sFXParam2 = (float)stack->pop()->getFloat(0); // Reverb Mix [dB] (-96 - 0)
		_sFXParam3 = (float)stack->pop()->getFloat(1000.0f); // Reverb Time [ms] (0.001 - 3000)
		_sFXParam4 = (float)stack->pop()->getFloat(0.001f); // HighFreq RT Ratio (0.001 - 0.999)
		stack->pushNULL();

		return STATUS_OK;
	} else {
		return BaseScriptHolder::scCallMethod(script, stack, thisStack, name);
	}
}


//////////////////////////////////////////////////////////////////////////
ScValue *BaseObject::scGetProperty(const Common::String &name) {
	_scValue->setNULL();

	//////////////////////////////////////////////////////////////////////////
	// Type
	//////////////////////////////////////////////////////////////////////////
	if (name == "Type") {
		_scValue->setString("object");
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Caption") {
		_scValue->setString(getCaption(1));
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (name == "X") {
		_scValue->setInt(_posX);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Y") {
		_scValue->setInt(_posY);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Height (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Height") {
		_scValue->setInt(getHeight());
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Ready (RO)
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Ready") {
		_scValue->setBool(_ready);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Movable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Movable") {
		_scValue->setBool(_movable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Registrable/Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Registrable" || name == "Interactive") {
		_scValue->setBool(_registrable);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Zoomable/Scalable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Zoomable" || name == "Scalable") {
		_scValue->setBool(_zoomable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// Rotatable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Rotatable") {
		_scValue->setBool(_rotatable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AlphaColor") {
		_scValue->setInt((int)_alphaColor);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlendMode
	//////////////////////////////////////////////////////////////////////////
	else if (name == "BlendMode") {
		_scValue->setInt((int)_blendMode);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Scale") {
		if (_scale < 0) {
			_scValue->setNULL();
		} else {
			_scValue->setFloat((double)_scale);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleX
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ScaleX") {
		if (_scaleX < 0) {
			_scValue->setNULL();
		} else {
			_scValue->setFloat((double)_scaleX);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleY
	//////////////////////////////////////////////////////////////////////////
	else if (name == "ScaleY") {
		if (_scaleY < 0) {
			_scValue->setNULL();
		} else {
			_scValue->setFloat((double)_scaleY);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeScale
	//////////////////////////////////////////////////////////////////////////
	else if (name == "RelativeScale") {
		_scValue->setFloat((double)_relativeScale);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotate
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Rotate") {
		if (!_rotateValid) {
			_scValue->setNULL();
		} else {
			_scValue->setFloat((double)_rotate);
		}
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeRotate
	//////////////////////////////////////////////////////////////////////////
	else if (name == "RelativeRotate") {
		_scValue->setFloat((double)_relativeRotate);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// Colorable
	//////////////////////////////////////////////////////////////////////////
	else if (name == "Colorable") {
		_scValue->setBool(_shadowable);
		return _scValue;
	}
	//////////////////////////////////////////////////////////////////////////
	// SoundPanning
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SoundPanning") {
		_scValue->setBool(_autoSoundPanning);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveState
	//////////////////////////////////////////////////////////////////////////
	else if (name == "SaveState") {
		_scValue->setBool(_saveState);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// NonIntMouseEvents
	//////////////////////////////////////////////////////////////////////////
	else if (name == "NonIntMouseEvents") {
		_scValue->setBool(_nonIntMouseEvents);
		return _scValue;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccCaption
	//////////////////////////////////////////////////////////////////////////
	else if (name == "AccCaption") {
		_scValue->setNULL();
		return _scValue;
	} else {
		return BaseScriptHolder::scGetProperty(name);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::scSetProperty(const char *name, ScValue *value) {
	//////////////////////////////////////////////////////////////////////////
	// Caption
	//////////////////////////////////////////////////////////////////////////
	if (strcmp(name, "Caption") == 0) {
		setCaption(value->getString());
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// X
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "X") == 0) {
		_posX = value->getInt();
		afterMove();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Y
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Y") == 0) {
		_posY = value->getInt();
		afterMove();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Movable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Movable") == 0) {
		_movable = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Registrable/Interactive
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Registrable") == 0 || strcmp(name, "Interactive") == 0) {
		_registrable = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Zoomable/Scalable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Zoomable") == 0 || strcmp(name, "Scalable") == 0) {
		_zoomable = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotatable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotatable") == 0) {
		_rotatable = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AlphaColor
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AlphaColor") == 0) {
		_alphaColor = (uint32)value->getInt();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// BlendMode
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "BlendMode") == 0) {
		int i = value->getInt();
		if (i < BLEND_NORMAL || i >= NUM_BLEND_MODES) {
			i = BLEND_NORMAL;
		}
		_blendMode = (TSpriteBlendMode)i;
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Scale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Scale") == 0) {
		if (value->isNULL()) {
			_scale = -1;
		} else {
			_scale = (float)value->getFloat();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleX
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleX") == 0) {
		if (value->isNULL()) {
			_scaleX = -1;
		} else {
			_scaleX = (float)value->getFloat();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// ScaleY
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "ScaleY") == 0) {
		if (value->isNULL()) {
			_scaleY = -1;
		} else {
			_scaleY = (float)value->getFloat();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeScale
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RelativeScale") == 0) {
		_relativeScale = (float)value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Rotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Rotate") == 0) {
		if (value->isNULL()) {
			_rotate = 0.0f;
			_rotateValid = false;
		} else {
			_rotate = (float)value->getFloat();
			_rotateValid = true;
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// RelativeRotate
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "RelativeRotate") == 0) {
		_relativeRotate = (float)value->getFloat();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// Colorable
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "Colorable") == 0) {
		_shadowable = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SoundPanning
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SoundPanning") == 0) {
		_autoSoundPanning = value->getBool();
		if (!_autoSoundPanning) {
			resetSoundPan();
		}
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// SaveState
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "SaveState") == 0) {
		_saveState = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// NonIntMouseEvents
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "NonIntMouseEvents") == 0) {
		_nonIntMouseEvents = value->getBool();
		return STATUS_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// AccCaption
	//////////////////////////////////////////////////////////////////////////
	else if (strcmp(name, "AccCaption") == 0) {
		return STATUS_OK;
	} else {
		return BaseScriptHolder::scSetProperty(name, value);
	}
}


//////////////////////////////////////////////////////////////////////////
const char *BaseObject::scToString() {
	return "[object]";
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::showCursor() {
	if (_cursor) {
		return _gameRef->drawCursor(_cursor);
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::saveAsText(BaseDynamicBuffer *buffer, int indent) {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::persist(BasePersistenceManager *persistMgr) {
	BaseScriptHolder::persist(persistMgr);

	for (int i = 0; i < 7; i++) {
		persistMgr->transfer(TMEMBER(_caption[i]));
	}
	persistMgr->transfer(TMEMBER(_activeCursor));
	persistMgr->transfer(TMEMBER(_alphaColor));
	persistMgr->transfer(TMEMBER(_autoSoundPanning));
	persistMgr->transfer(TMEMBER(_cursor));
	persistMgr->transfer(TMEMBER(_sharedCursors));
	persistMgr->transfer(TMEMBER(_editorAlwaysRegister));
	persistMgr->transfer(TMEMBER(_editorOnly));
	persistMgr->transfer(TMEMBER(_editorSelected));
	persistMgr->transfer(TMEMBER(_iD));
	persistMgr->transfer(TMEMBER(_is3D));
	persistMgr->transfer(TMEMBER(_movable));
	persistMgr->transfer(TMEMBER(_posX));
	persistMgr->transfer(TMEMBER(_posY));
	persistMgr->transfer(TMEMBER(_relativeScale));
	persistMgr->transfer(TMEMBER(_rotatable));
	persistMgr->transfer(TMEMBER(_scale));
	persistMgr->transfer(TMEMBER(_sFX));
	persistMgr->transfer(TMEMBER(_sFXStart));
	persistMgr->transfer(TMEMBER(_sFXVolume));
	persistMgr->transfer(TMEMBER(_ready));
	persistMgr->transfer(TMEMBER(_rect));
	persistMgr->transfer(TMEMBER(_rectSet));
	persistMgr->transfer(TMEMBER(_registrable));
	persistMgr->transfer(TMEMBER(_shadowable));
	persistMgr->transfer(TMEMBER(_soundEvent));
	persistMgr->transfer(TMEMBER(_zoomable));

	persistMgr->transfer(TMEMBER(_scaleX));
	persistMgr->transfer(TMEMBER(_scaleY));

	persistMgr->transfer(TMEMBER(_rotate));
	persistMgr->transfer(TMEMBER(_rotateValid));
	persistMgr->transfer(TMEMBER(_relativeRotate));

	persistMgr->transfer(TMEMBER(_saveState));
	persistMgr->transfer(TMEMBER(_nonIntMouseEvents));

	persistMgr->transfer(TMEMBER_INT(_sFXType));
	persistMgr->transfer(TMEMBER(_sFXParam1));
	persistMgr->transfer(TMEMBER(_sFXParam2));
	persistMgr->transfer(TMEMBER(_sFXParam3));
	persistMgr->transfer(TMEMBER(_sFXParam4));


	persistMgr->transfer(TMEMBER_INT(_blendMode));

	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::setCursor(const char *filename) {
	if (!_sharedCursors) {
		delete _cursor;
		_cursor = NULL;
	}

	_sharedCursors = false;
	_cursor = new BaseSprite(_gameRef);
	if (!_cursor || DID_FAIL(_cursor->loadFile(filename))) {
		delete _cursor;
		_cursor = NULL;
		return STATUS_FAILED;
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::setActiveCursor(const char *filename) {
	delete _activeCursor;
	_activeCursor = new BaseSprite(_gameRef);
	if (!_activeCursor || DID_FAIL(_activeCursor->loadFile(filename))) {
		delete _activeCursor;
		_activeCursor = NULL;
		return STATUS_FAILED;
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
int BaseObject::getHeight() {
	return 0;
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::handleMouse(TMouseEvent event, TMouseButton button) {
	return STATUS_OK;
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::handleKeypress(Common::Event *event, bool printable) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::handleMouseWheel(int delta) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::playSFX(const char *filename, bool looping, bool playNow, const char *eventName, uint32 loopStart) {
	// just play loaded sound
	if (filename == NULL && _sFX) {
		if (_gameRef->_editorMode || _sFXStart) {
			_sFX->setVolumePercent(_sFXVolume);
			_sFX->setPositionTime(_sFXStart);
			if (!_gameRef->_editorMode) {
				_sFXStart = 0;
			}
		}
		if (playNow) {
			setSoundEvent(eventName);
			if (loopStart) {
				_sFX->setLoopStart(loopStart);
			}
			return _sFX->play(looping);
		} else {
			return STATUS_OK;
		}
	}

	if (filename == NULL) {
		return STATUS_FAILED;
	}

	// create new sound
	delete _sFX;

	_sFX = new BaseSound(_gameRef);
	if (_sFX && DID_SUCCEED(_sFX->setSound(filename, Audio::Mixer::kSFXSoundType, true))) {
		_sFX->setVolumePercent(_sFXVolume);
		if (_sFXStart) {
			_sFX->setPositionTime(_sFXStart);
			_sFXStart = 0;
		}
		_sFX->applyFX(_sFXType, _sFXParam1, _sFXParam2, _sFXParam3, _sFXParam4);
		if (playNow) {
			setSoundEvent(eventName);
			if (loopStart) {
				_sFX->setLoopStart(loopStart);
			}
			return _sFX->play(looping);
		} else {
			return STATUS_OK;
		}
	} else {
		delete _sFX;
		_sFX = NULL;
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::stopSFX(bool deleteSound) {
	if (_sFX) {
		_sFX->stop();
		if (deleteSound) {
			delete _sFX;
			_sFX = NULL;
		}
		return STATUS_OK;
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::pauseSFX() {
	if (_sFX) {
		return _sFX->pause();
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::resumeSFX() {
	if (_sFX) {
		return _sFX->resume();
	} else {
		return STATUS_FAILED;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::setSFXTime(uint32 time) {
	_sFXStart = time;
	if (_sFX && _sFX->isPlaying()) {
		return _sFX->setPositionTime(time);
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::setSFXVolume(int volume) {
	_sFXVolume = volume;
	if (_sFX) {
		return _sFX->setVolumePercent(volume);
	} else {
		return STATUS_OK;
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::updateSounds() {
	if (_soundEvent) {
		if (_sFX && !_sFX->isPlaying()) {
			applyEvent(_soundEvent);
			setSoundEvent(NULL);
		}
	}

	if (_sFX) {
		updateOneSound(_sFX);
	}

	return STATUS_OK;
}

//////////////////////////////////////////////////////////////////////////
bool BaseObject::updateOneSound(BaseSound *sound) {
	bool ret = STATUS_OK;

	if (sound) {
		if (_autoSoundPanning) {
			ret = sound->setPan(_gameRef->_soundMgr->posToPan(_posX  - _gameRef->_offsetX, _posY - _gameRef->_offsetY));
		}

		ret = sound->applyFX(_sFXType, _sFXParam1, _sFXParam2, _sFXParam3, _sFXParam4);
	}
	return ret;
}

//////////////////////////////////////////////////////////////////////////
bool BaseObject::resetSoundPan() {
	if (!_sFX) {
		return STATUS_OK;
	} else {
		return _sFX->setPan(0.0f);
	}
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::getExtendedFlag(const char *flagName) {
	return false;
}


//////////////////////////////////////////////////////////////////////////
bool BaseObject::isReady() {
	return _ready;
}


//////////////////////////////////////////////////////////////////////////
void BaseObject::setSoundEvent(const char *eventName) {
	delete[] _soundEvent;
	_soundEvent = NULL;
	if (eventName) {
		_soundEvent = new char[strlen(eventName) + 1];
		if (_soundEvent) {
			strcpy(_soundEvent, eventName);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
bool BaseObject::afterMove() {
	return STATUS_OK;
}

} // end of namespace Wintermute
