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

#include "tsage/ringworld2/ringworld2_speakers.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/graphics.h"
#include "tsage/staticres.h"
#include "tsage/ringworld2/ringworld2_scenes0.h"
#include "tsage/ringworld2/ringworld2_scenes1.h"
#include "tsage/ringworld2/ringworld2_scenes2.h"
#include "tsage/ringworld2/ringworld2_scenes3.h"

namespace TsAGE {

namespace Ringworld2 {

VisualSpeaker::VisualSpeaker(): Speaker() {
	_delayAmount = 0;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
	_color1 = 8;
	_color2 = 0;
	_displayMode = 0;
	_fieldF6 = 0;
}

void VisualSpeaker::remove() {
	if (_object2) {
		if (_fieldF8) {
			_fieldF8 = 0;
			_object1.setStrip(_object1._strip - 1);
			_object1.setFrame(_object1.getFrameCount());
			_object1.animate(ANIM_MODE_6, (_fieldF6 == 0xff) ? this : NULL);
		} else {
			_object1.animate(ANIM_MODE_6, (_fieldF6 == 0xff) ? this : NULL);
		}
	}

	Speaker::remove();
}

void VisualSpeaker::synchronize(Serializer &s) {
	Speaker::synchronize(s);

	SYNC_POINTER(_object2);
	s.syncAsSint16LE(_fieldF6);
	s.syncAsSint16LE(_fieldF8);
	s.syncAsSint16LE(_displayMode);
	s.syncAsSint16LE(_soundId);
	s.syncAsSint16LE(_delayAmount);
	s.syncAsByte(_removeObject);
	s.syncAsSint32LE(_frameNumber);
	s.syncAsSint16LE(_numFrames);
}

void VisualSpeaker::setText(const Common::String &msg) {
	_sceneText.remove();

	// Position the text depending on the specified display mode
	switch (_displayMode) {
	case 2:
		_textPos = Common::Point(60, 20);
		break;
	case 3:
		_textPos = Common::Point(110, 20);
		break;
	case 4:
		_textPos = Common::Point(10, 100);
		break;
	case 5:
		_textPos = Common::Point(60, 100);
		break;
	case 6:
		_textPos = Common::Point(110, 100);
		break;
	case 7:
		_textPos = Common::Point(170, 20);
		break;
	case 8:
		_textPos = Common::Point(170, 100);
		break;
	case 9:
		_textPos = Common::Point(330, 20);
		break;
	default:
		_textPos = Common::Point(10, 20);
		break;
	}

	// Check if the message starts with a '!'. If so, it indicates a speech resource Id to be played,
	// in which case extract the resource number from the message.
	_soundId = 0;
	Common::String s = msg;
	if (s.hasPrefix("!")) {
		s.deleteChar(0);
		_soundId = atoi(s.c_str());

		while (!s.empty() && (*s.c_str() >= '0' && *s.c_str() <= '9'))
			s.deleteChar(0);
	}

	// Set up the text details
	_sceneText._color1 = _color1;
	_sceneText._color2 = _color2;
	_sceneText._color3 = _color3;
	_sceneText._width = _textWidth;
	_sceneText._fontNumber = _fontNumber;
	_sceneText._textMode = _textMode;
	_sceneText.setup(s);

	//_sceneText.clone();

	_sceneText.setPosition(_textPos);
	_sceneText.fixPriority(256);

	// If subtitles are turned off, don't show the text
	if (!(R2_GLOBALS._speechSubtitles & SPEECH_TEXT)) {
		_sceneText.hide();
	}

	// Figure out the text delay if subtitles are turned on, or there's no speech resource specified
	if ((R2_GLOBALS._speechSubtitles & SPEECH_TEXT) || !_soundId) {
		const char *msgP = s.c_str();
		int numWords = 0;
		while (*msgP != '\0') {
			if (*msgP++ == ' ')
				++numWords;
		}

		if (!numWords && !s.empty())
			++numWords;

		_numFrames = numWords * 30 + 120;
		setDelay(_numFrames);
	} else {
		_numFrames = 1;
	}

	// If the text is empty, no delay is needed
	if (s.empty())
		_numFrames = 0;


	if (_fieldF6) {
		if ((R2_GLOBALS._speechSubtitles & SPEECH_TEXT) || !_soundId)
			_sceneText.hide();
	} else {
		if ((R2_GLOBALS._speechSubtitles & SPEECH_VOICE) && _soundId) {
			if (!R2_GLOBALS._playStream.play(_soundId, NULL))
				_sceneText.show();
		}
	}
}

void VisualSpeaker::proc16() {
	R2_GLOBALS._playStream.stop();
	_fieldF6 = 0;
	_object1.remove();

	assert(_object2);
	_object2->show();
	_object2 = NULL;
	_fieldF8 = 0;
}

void VisualSpeaker::setFrame(int numFrames) {
	_delayAmount = numFrames;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
}

void VisualSpeaker::setDelay(int delay) {
	_delayAmount = delay;
	_frameNumber = R2_GLOBALS._events.getFrameNumber();
}

//----------------------------------------------------------------------------

SpeakerGameText::SpeakerGameText(): VisualSpeaker() {
	_speakerName = "GAMETEXT";
	_color1 = 8;
	_color2 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
}

//----------------------------------------------------------------------------
// Classes related to CAPTAIN
//----------------------------------------------------------------------------

SpeakerCaptain3210::SpeakerCaptain3210() {
	_speakerName = "Captain";
	_color1 = 5;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerCaptain3210::proc15() {
	int v = _fieldF6;
	Scene3210 *scene = (Scene3210 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to CARETAKER
//----------------------------------------------------------------------------

SpeakerCaretaker2450::SpeakerCaretaker2450() {
	_speakerName = "CARETAKER";
	_color1 = 43;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

//----------------------------------------------------------------------------
// Classes related to CHIEF
//----------------------------------------------------------------------------

SpeakerChief1100::SpeakerChief1100() {
	_speakerName = "CHIEF";
	_color1 = 8;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerChief1100::proc15() {
	int v = _fieldF6;
	Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor18;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4080, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4080, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 100:
		_numFrames = 0;
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to GUARD
//----------------------------------------------------------------------------

SpeakerGuard::SpeakerGuard() {
	_speakerName = "GUARD";
	_color1 = 5;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerGuard2800::proc15() {
	int v = _fieldF6;
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setZoom(75);
		_object1.setup(4060, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to JOCKO
//----------------------------------------------------------------------------

SpeakerJocko::SpeakerJocko() {
	_speakerName = "Jocko";
	_color1 = 45;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerJocko3200::proc15() {
	int v = _fieldF6;
	Scene3200 *scene = (Scene3200 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerJocko3220::proc15() {
	int v = _fieldF6;
	Scene3220 *scene = (Scene3220 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerJocko3230::proc15() {
	int v = _fieldF6;
	Scene3230 *scene = (Scene3230 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to MIRANDA
//----------------------------------------------------------------------------

SpeakerMiranda::SpeakerMiranda(): VisualSpeaker() {
	_speakerName = "MIRANDA";
	_color1 = 154;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerMiranda300::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_miranda;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else if (v == 100) {
		_numFrames = 0;
		((SceneItem *)_action)->_sceneRegionId = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;

		if (v == 4) {
			_object1.setup(304, 5, 1);
		} else {
			_object1.setup(305, v * 2 - 1, 1);
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerMiranda1625::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		Scene1625 *scene = (Scene1625 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_actor3;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(Common::Point(196, 65));

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1627, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerMiranda3255::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		_object2 = &R2_GLOBALS._player;
		_object2->hide();
		_object1.postInit();
		_object1._effect = _object2->_effect;
		_object1._shade = _object2->_shade;
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(3257, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerMiranda3375::proc15() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (scene->_actor1._position.y != 163)
			R2_GLOBALS._player.setStrip(8);
		else
			R2_GLOBALS._player.setStrip(2);

		R2_GLOBALS._player.disableControl();

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4051, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3385::proc15() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4051, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3395::proc15() {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4051, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3400::proc15() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4051, 5, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4050, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3600::proc15() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor12;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);

		_object1.setPosition(_object2->_position);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4051, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4050, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerMiranda3700::proc15() {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor3;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	_object1.setPosition(_object2->_position);

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		scene->_actor1.setup(10, 6, 1);
		scene->_actor2.setup(20, 5, 1);
		_object2->setup(30, 1, 1);
		scene->_actor4.setup(40, 1, 1);
		_object1.setup(4050, 5, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		scene->_actor3.setup(30, 8, 1);
		_object1.setup(4052, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		scene->_actor2.setup(20, 1, 1);
		scene->_actor3.setup(30, 1, 1);
		_object1.setup(4051, 7, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to NEJ
//----------------------------------------------------------------------------

SpeakerNej::SpeakerNej() {
	_speakerName = "NEJ";
	_color1 = 171;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerNej2700::proc15() {
	int v = _fieldF6;
	Scene2700 *scene = (Scene2700 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		switch (_object2->_visage) {
		case 2701:
			_object1.setup(4022, 3, 1);
			_object1.setPosition(Common::Point(164, 163));
			_object2->setPosition(Common::Point(-10, -10));
			break;
		case 2705:
			_object1.setup(4022, 7, 1);
			_object1.fixPriority(162);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerNej2750::proc15() {
	int v = _fieldF6;
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		switch (_object2->_visage) {
		case 2705:
			_object1.setup(4022, 7, 1);
			break;
		case 2752:
			_object1.setup(2752, 1, 1);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerNej2800::proc15() {
	int v = _fieldF6;
	Scene2750 *scene = (Scene2750 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4023, 3, 1);
		if (_object2->_visage == 2801)
			_object1.setPosition(Common::Point(R2_GLOBALS._player._position.x - 12, R2_GLOBALS._player._position.y));
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to PHARISHA
//----------------------------------------------------------------------------

SpeakerPharisha::SpeakerPharisha(): VisualSpeaker() {
	_speakerName = "PHARISHA";
	_color1 = 151;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerPharisha2435::proc15() {
	int v = _fieldF6;
	Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4098, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to PRIVATE
//----------------------------------------------------------------------------

SpeakerPrivate3210::SpeakerPrivate3210() {
	_speakerName = "Private";
	_color1 = 45;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerPrivate3210::proc15() {
	int v = _fieldF6;
	Scene3210 *scene = (Scene3210 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to PROTECTOR
//----------------------------------------------------------------------------

SpeakerProtector3600::SpeakerProtector3600() {
	_speakerName = "Protector";
	_color1 = 170;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 7;
	_numFrames = 0;
}

void SpeakerProtector3600::proc15() {
	int v = _fieldF6;
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor13;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	_object1.setPosition(_object2->_position);

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		if (scene->_sceneMode != 3324) {
			_object1.setup(4125, 3, 1);
			_object1.animate(ANIM_MODE_5, this);
		} else {
			_object1.setup(3258, 6, 1);
			_object1.animate(ANIM_MODE_2, NULL);
			_object1.hide();
			_object2->setup(3258, 6, 1);
			_object2->show();
		}
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to QUINN
//----------------------------------------------------------------------------

SpeakerQuinn::SpeakerQuinn(): VisualSpeaker() {
	_speakerName = "QUINN";
	_color1 = 60;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerQuinn300::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_quinn;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else if (v == 100) {
		_numFrames = 0;
		((SceneItem *)_action)->_sceneRegionId = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;

		switch (_object2->_visage) {
		case 10:
			_object1.setup((v - 1) / 4 + 4010, ((v - ((v - 1) / 4 * 4) - 1) % 8) * 2 + 1, 1);
			break;
		case 302:
			_object1.setup(308, (v - 1) % 8 + 1, 1);
			break;
		case 308:
			_object1.setup(308, 5, 1);
			break;
		}

		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn1100::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (v == 0)
			return;

		if (R2_GLOBALS._player._characterIndex == 1) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_actor16;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1108, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1109, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1109, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn2435::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 1) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_actor1;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object2->setStrip(7);
		_object1.setup(2020, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn2450::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 1) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_actor1;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		if (R2_GLOBALS.getFlag(61))
			_object1.setup(2020, 3, 1);
		else
			_object1.setup(2020, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn2700::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		_object2 = &R2_GLOBALS._player;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		switch (_object2->_visage) {
		case 19:
			_object1.setup(4022, 5, 1);
			break;
		case 2701:
			_object1.setup(4022, 1, 1);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn2750::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		_object2 = &R2_GLOBALS._player;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		switch (_object2->_visage) {
		case 19:
			_object1.setup(4022, 5, 1);
			break;
		case 2752:
			_object1.setup(2752, 1, 1);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn2800::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		_object2 = &R2_GLOBALS._player;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		switch (_object2->_visage) {
		case 16:
			_object1.setZoom(75);
			_object1.setup(4023, 5, 1);
			break;
		case 19:
			_object1.setup(4023, 1, 1);
			break;
		case 3110:
			_object1.setZoom(75);
			if (_object2->_strip == 1)
				_object1.setup(4061	, 1, 1);
			else
				_object1.setup(4061	, 3, 1);
			break;
		default:
			break;
		}
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn3255::proc15() {
	Scene3255 *scene = (Scene3255 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor4;
		_object2->hide();
		_object1.postInit();
		_object1._effect = _object2->_effect;
		_object1._shade = _object2->_shade;
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(3257, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerQuinn3375::proc15() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 1)
			_object2 = &R2_GLOBALS._player;
		else if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &scene->_actor1;
		else
			_object2 = &scene->_actor2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (scene->_actor1._position.y != 163)
			R2_GLOBALS._player.setStrip(8);
		else
			R2_GLOBALS._player.setStrip(2);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4010, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3385::proc15() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 1)
			_object2 = &R2_GLOBALS._player;
		else if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &scene->_actor1;
		else
			_object2 = &scene->_actor2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		if (R2_GLOBALS._player._characterIndex == 2)
			_object1.setup(4010, 3, 1);
		else
			_object1.setup(4010, 5, 1);

		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3395::proc15() {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 1)
			_object2 = &R2_GLOBALS._player;
		else if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &scene->_actor1;
		else
			_object2 = &scene->_actor2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		if (R2_GLOBALS._player._characterIndex == 2)
			_object1.setup(4010, 3, 1);
		else
			_object1.setup(4010, 5, 1);

		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3400::proc15() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 1)
			_object2 = &R2_GLOBALS._player;
		else if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &scene->_actor1;
		else
			_object2 = &scene->_actor2;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4010, 5, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4010, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4012, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3600::proc15() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 1)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor10;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4021, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4010, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4012, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerQuinn3700::setText(const Common::String &msg) {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	switch (_fieldF6) {
	case 2:
		scene->_actor3.setup(30, 1, 1);
		R2_GLOBALS._sound2.play(44);
		break;
	case 3:
		scene->_actor3.setup(30, 1, 1);
		break;
	default:
		scene->_actor3.setup(30, 7, 1);
		break;
	}
	VisualSpeaker::setText(msg);
}

void SpeakerQuinn3700::proc15() {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	_object1.setPosition(_object2->_position);

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		R2_GLOBALS._sound2.stop();
		scene->_actor1.setup(10, 4, 1);
		scene->_actor3.setup(30, 7, 1);
		_object1.setup(3701, 1, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		scene->_actor2.setup(20, 1, 1);
		scene->_actor3.setup(30, 1, 1);
		_object1.setup(3701, 2, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		scene->_actor1.setup(10, 2, 1);
		scene->_actor3.setup(30, 1, 1);
		_object1.setup(4011, 1, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to QUINNL
//----------------------------------------------------------------------------

SpeakerQuinnL::SpeakerQuinnL(): VisualSpeaker() {
	_speakerName = "QUINNL";
	_color1 = 35;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
	_fontNumber = 10;
}

//----------------------------------------------------------------------------
// Classes related to RALF
//----------------------------------------------------------------------------

SpeakerRalf3245::SpeakerRalf3245() {
	_speakerName = "Ralf";
	_color1 = 5;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerRalf3245::proc15() {
	int v = _fieldF6;
	Scene3245 *scene = (Scene3245 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		switch (_object2->_visage) {
		case 3100:
			_object1.setup(4105, (_object2->_strip * 2) - 1, 1);
			break;
		case 3101:
			_object1.setup(4108, (_object2->_strip * 2) - 1, 1);
			break;
		case 3102:
			_object1.setup(4109, (_object2->_strip * 2) - 1, 1);
			break;
		default:
			break;
		}

		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to ROCKO
//----------------------------------------------------------------------------

SpeakerRocko::SpeakerRocko() {
	_speakerName = "Rocko";
	_color1 = 5;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerRocko3200::proc15() {
	int v = _fieldF6;
	Scene3200 *scene = (Scene3200 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerRocko3220::proc15() {
	int v = _fieldF6;
	Scene3220 *scene = (Scene3220 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerRocko3230::proc15() {
	int v = _fieldF6;
	Scene3230 *scene = (Scene3230 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4111, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to SEEKER
//----------------------------------------------------------------------------

SpeakerSeeker::SpeakerSeeker(): VisualSpeaker() {
	_speakerName = "SEEKER";
	_color1 = 35;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerSeeker300::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 3) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_seeker;
		}

		_object2->hide();
		_object1.postInit();
		_object1.fixPriority(140);
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else if (v == 100) {
		_numFrames = 0;
		((SceneItem *)_action)->_sceneRegionId = 0;

		_object1.setStrip(_object1._strip - 1);
		_object1.setFrame(_object1.getFrameCount());
		_object1.animate(ANIM_MODE_6, this);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(306, v * 2 - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerSeeker1100::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (v == 0)
			return;

		if (R2_GLOBALS._player._characterIndex == 2) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene1100 *scene = (Scene1100 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_actor16;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1108, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1108, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		_object1.setPosition(Common::Point(197, 134));
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1108, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 4:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1109, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 5:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1109, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker1900::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 2) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene1900 *scene = (Scene1900 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_actor1;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else if (v == 1) {
		_object1.setup(4032, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	} else {
		signal();
	}
}

void SpeakerSeeker2435::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 2) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene2435 *scene = (Scene2435 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_actor1;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object2->setStrip(7);
		_object1.setup(4099, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerSeeker2450::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 2) {
			_object2 = &R2_GLOBALS._player;
		} else {
			Scene2450 *scene = (Scene2450 *)R2_GLOBALS._sceneManager._scene;
			_object2 = &scene->_actor1;
		}

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4099, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerSeeker3375::proc15() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor1;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (scene->_actor1._position.y != 163)
			R2_GLOBALS._player.setStrip(8);
		else
			R2_GLOBALS._player.setStrip(2);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4031, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3385::proc15() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor1;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4031, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3395::proc15() {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor1;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}


	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4031, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3400::proc15() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor1;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4031, 1, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4031, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4030, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 4:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4031, 7, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 5:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4033, 1, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3600::proc15() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		if (R2_GLOBALS._player._characterIndex == 2)
			_object2 = &R2_GLOBALS._player;
		else
			_object2 = &scene->_actor11;

		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);

		_object1.setPosition(_object2->_position);

	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4031, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4030, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerSeeker3700::setText(const Common::String &msg) {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	if (_fieldF6 == 1) {
		R2_GLOBALS._sound2.play(44);
		scene->_actor3.setup(30, 8, 1);
	} else {
		scene->_actor3.setup(30, 2, 1);
	}
	VisualSpeaker::setText(msg);
}

void SpeakerSeeker3700::proc15() {
	Scene3700 *scene = (Scene3700 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	_object1.setPosition(_object2->_position);

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		R2_GLOBALS._sound2.stop();
		scene->_actor1.setup(10, 8, 1);
		scene->_actor2.setup(20, 7, 1);
		scene->_actor3.setup(30, 8, 1);
		_object1.setup(3701, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		scene->_actor1.setup(10, 2, 1);
		scene->_actor2.setup(20, 1, 1);
		scene->_actor3.setup(30, 1, 1);
		_object1.setup(4031, 1, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to SEEKERL
//----------------------------------------------------------------------------

SpeakerSeekerL::SpeakerSeekerL(): VisualSpeaker() {
	_speakerName = "SEEKERL";
	_color1 = 35;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
	_fontNumber = 10;
}

//----------------------------------------------------------------------------
// Classes related to SOCKO
//----------------------------------------------------------------------------

SpeakerSocko3200::SpeakerSocko3200() {
	_speakerName = "Socko";
	_color1 = 10;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerSocko3200::proc15() {
	int v = _fieldF6;
	Scene3200 *scene = (Scene3200 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor3;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4060, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to SOLDIER
//----------------------------------------------------------------------------

SpeakerSoldier::SpeakerSoldier(int colour) {
	_speakerName = "SOLDIER";
	_color1 = colour;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerSoldier300::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_teal;
		_object2->hide();

		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(303, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to TEAL
//----------------------------------------------------------------------------

SpeakerTeal::SpeakerTeal(): VisualSpeaker() {
	_speakerName = "TEAL";
	_color1 = 22;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

SpeakerTealMode7::SpeakerTealMode7(): SpeakerTeal() {
	_displayMode = 7;
}

void SpeakerTeal300::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		Scene300 *scene = (Scene300 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_teal;
		_object2->hide();

		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(303, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerTeal1625::proc15() {
	int v = _fieldF6;

	if (!_object2) {
		Scene1625 *scene = (Scene1625 *)R2_GLOBALS._sceneManager._scene;
		_object2 = &scene->_actor2;
		_object2->hide();

		_object1.postInit();
		_object1.setPosition(Common::Point(68, 68));

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(1627, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerTeal3240::proc15() {
	int v = _fieldF6;
	Scene3240 *scene = (Scene3240 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor1;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4070, (_object2->_strip * 2) - 1, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerTeal3400::proc15() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor4;
		_object2->hide();
		_object1.postInit();
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}
	_object1.setPosition(_object2->_position);
	_object1.show();

	if (scene ->_sceneMode == 3305) {
		R2_GLOBALS._player.setStrip(6);
		scene->_actor1.setStrip(6);
		scene->_actor2.setStrip(6);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4107, 5, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4107, 1, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4107, 7, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 4:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4107, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerTeal3600::proc15() {
	Scene3600 *scene = (Scene3600 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor5;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}
	_object1.setPosition(_object2->_position);

	if (scene ->_sceneMode == 3323) {
		_object1.hide();
		_object2->show();
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4107, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4107, 1, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4107, 7, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	case 4:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4107, 3, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------
// Classes related to TOMKO
//----------------------------------------------------------------------------

SpeakerTomko3245::SpeakerTomko3245() {
	_speakerName = "Tomko";
	_color1 = 10;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerTomko3245::proc15() {
	int v = _fieldF6;
	Scene3245 *scene = (Scene3245 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		switch (_object2->_visage) {
		case 3100:
			_object1.setup(4105, (_object2->_strip * 2) - 1, 1);
			break;
		case 3101:
			_object1.setup(4108, (_object2->_strip * 2) - 1, 1);
			break;
		case 3102:
			_object1.setup(4109, (_object2->_strip * 2) - 1, 1);
			break;
		default:
			break;
		}

		_object1.animate(ANIM_MODE_5, this);
	}
}

//----------------------------------------------------------------------------
// Classes related to WEBBSTER
//----------------------------------------------------------------------------

SpeakerWebbster::SpeakerWebbster(int colour) {
	_speakerName = "WEBBSTER";
	_color1 = colour;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerWebbster3240::proc15() {
	int v = _fieldF6;
	Scene3240 *scene = (Scene3240 *)R2_GLOBALS._sceneManager._scene;

	if (!_object2) {
		_object2 = &scene->_actor2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	if (v == 0) {
		_object1.animate(ANIM_MODE_2, NULL);
	} else {
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
	}
}

void SpeakerWebbster3375::proc15() {
	Scene3375 *scene = (Scene3375 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor3;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (scene->_actor1._position.y != 163)
			R2_GLOBALS._player.setStrip(8);
		else
			R2_GLOBALS._player.setStrip(2);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerWebbster3385::proc15() {
	Scene3385 *scene = (Scene3385 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor3;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3375)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerWebbster3395::proc15() {
	Scene3395 *scene = (Scene3395 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor3;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);

		if (R2_GLOBALS._sceneManager._previousScene == 3385)
			R2_GLOBALS._player.setStrip(4);
		else
			R2_GLOBALS._player.setStrip(3);

		if (R2_GLOBALS._player._mover)
			R2_GLOBALS._player.addMover(NULL);

		R2_GLOBALS._player.disableControl(CURSOR_TALK);
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}

void SpeakerWebbster3400::proc15() {
	Scene3400 *scene = (Scene3400 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_actor3;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);
		_object1._numFrames = 7;
		_object1._effect = 1;
		_object1.changeZoom(-1);
		R2_GLOBALS._player.disableControl();
		R2_GLOBALS._events.setCursor(CURSOR_CROSSHAIRS);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4110, 5, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 2:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4110, 7, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	case 3:
		((SceneItem *)_action)->_sceneRegionId = 0;
		_object1.setup(4110, 3, 1);
		_object1.animate(ANIM_MODE_5, NULL);
		break;
	default:
		signal();
		break;
	}
}

//----------------------------------------------------------------------------

SpeakerDutyOfficer::SpeakerDutyOfficer(): VisualSpeaker() {
	_speakerName = "DUTYOFFICER";
	_color1 = 5;
	_color2 = 0;
	_fieldF6 = 0;
	_textWidth = 300;
	_hideObjects = false;
	_object2 = NULL;
	_displayMode = 1;
	_numFrames = 0;
}

void SpeakerDutyOfficer::proc15() {
	Scene180 *scene = (Scene180 *)R2_GLOBALS._sceneManager._scene;

	int v = _fieldF6;

	if (!_object2) {
		_object2 = &scene->_object2;
		_object2->hide();
		_object1.postInit();
		_object1.setPosition(_object2->_position);

		if (_object2->_mover)
			_object2->addMover(NULL);
	}

	switch (v) {
	case 0:
		_object1.animate(ANIM_MODE_2, NULL);
		break;
	case 1:
		_action = NULL;
		_object1.setup(76, 2, 1);
		_object1.animate(ANIM_MODE_5, this);
		break;
	default:
		signal();
		break;
	}
}


} // End of namespace Ringworld2
} // End of namespace TsAGE
