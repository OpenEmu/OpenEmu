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

#include "tsage/blue_force/blueforce_speakers.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/graphics.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace BlueForce {

VisualSpeaker::VisualSpeaker(): Speaker() {
	_textWidth = 312;
	_color1 = 19;
	_hideObjects = false;
	_removeObject1 = false;
	_removeObject2 = false;
	_xp = 160;
	_fontNumber = 4;
	_color2 = 82;
	_offsetPos = Common::Point(4, 170);
	_numFrames = 0;
}

void VisualSpeaker::remove() {
	if (_removeObject2)
		_object2.remove();
	if (_removeObject1)
		_object1.remove();

	Speaker::remove();
}

void VisualSpeaker::synchronize(Serializer &s) {
	Speaker::synchronize(s);

	s.syncAsByte(_removeObject1);
	s.syncAsByte(_removeObject2);
	s.syncAsSint16LE(_xp);
	s.syncAsSint16LE(_numFrames);
	s.syncAsSint16LE(_offsetPos.x);
	s.syncAsSint16LE(_offsetPos.y);
}

void VisualSpeaker::proc12(Action *action) {
	Speaker::proc12(action);
	_textPos = Common::Point(_offsetPos.x + BF_GLOBALS._sceneManager._scene->_sceneBounds.left,
		_offsetPos.y + BF_GLOBALS._sceneManager._scene->_sceneBounds.top);
	_numFrames = 0;
}

void VisualSpeaker::setText(const Common::String &msg) {
	_objectList.draw();
	BF_GLOBALS._sceneObjects->draw();

	_sceneText._color1 = _color1;
	_sceneText._color2 = _color2;
	_sceneText._color3 = _color3;
	_sceneText._width = _textWidth;
	_sceneText._fontNumber = _fontNumber;
	_sceneText._textMode = _textMode;
	_sceneText.setup(msg);

	// Get the string bounds
	GfxFont f;
	f.setFontNumber(_fontNumber);
	Rect bounds;
	f.getStringBounds(msg.c_str(), bounds, _textWidth);

	// Set the position for the text
	switch (_textMode) {
	case ALIGN_LEFT:
	case ALIGN_JUSTIFIED:
		_sceneText.setPosition(_textPos);
		break;
	case ALIGN_CENTER:
		_sceneText.setPosition(Common::Point(_textPos.x + (_textWidth - bounds.width()) / 2, _textPos.y));
		break;
	case ALIGN_RIGHT:
		_sceneText.setPosition(Common::Point(_textPos.x + _textWidth - bounds.width(), _textPos.y));
		break;
	default:
		break;
	}

	// Ensure the text is in the foreground
	_sceneText.fixPriority(256);

	// Count the number of words (by spaces) in the string
	const char *s = msg.c_str();
	int spaceCount = 0;
	while (*s) {
		if (*s++ == ' ')
			++spaceCount;
	}

	_numFrames = spaceCount * 3 + 2;
}

/*--------------------------------------------------------------------------*/

SpeakerGameText::SpeakerGameText(): VisualSpeaker() {
	_speakerName = "GAMETEXT";
	_color1 = 8;
	_color2 = 19;
}

/*--------------------------------------------------------------------------*/

SpeakerSutter::SpeakerSutter() {
	_speakerName = "SUTTER";
	_color1 = 20;
	_color2 = 22;
	_textMode = ALIGN_CENTER;
}

void SpeakerSutter::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(329);
	_object1.setStrip2(2);
	_object1.fixPriority(254);
	_object1.changeZoom(100);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 45,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(329);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 45,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerDoug::SpeakerDoug(): VisualSpeaker() {
	_color1 = 32;
	_speakerName = "DOUG";
}

/*--------------------------------------------------------------------------*/

SpeakerJakeNoHead::SpeakerJakeNoHead(): VisualSpeaker() {
	_color1 = 13;
	_speakerName = "JAKE_NO_HEAD";
}

/*--------------------------------------------------------------------------*/

SpeakerHarrison::SpeakerHarrison(): VisualSpeaker() {
	_color1 = 32;
	_speakerName = "HARRISON";
}

void SpeakerHarrison::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(325);
	_object1.setStrip2(7);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 277,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.fixPriority(255);
	_object2.setVisage(325);
	_object2.setStrip2(6);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 277,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerJake::SpeakerJake(): VisualSpeaker() {
	_color1 = 13;
	_color2 = 7;
	_speakerName = "JAKE";
}

void SpeakerJake::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(1001);
	_object1.setStrip2(2);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 38,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(1001);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 38,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerJakeJacket::SpeakerJakeJacket(): VisualSpeaker() {
	_color1 = 13;
	_color2 = 7;
	_speakerName = "JAKEJACKET";
}

void SpeakerJakeJacket::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(1001);
	_object1.setStrip2(4);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + _xp,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(1001);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + _xp,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerJakeUniform::SpeakerJakeUniform(): VisualSpeaker() {
	_color1 = 13;
	_color2 = 7;
	_speakerName = "JAKEUNIFORM";
}

void SpeakerJakeUniform::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(1001);
	_object1.setStrip2(3);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 45,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(1001);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 45,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerJailer::SpeakerJailer(): VisualSpeaker() {
	_color1 = 13;
	_color2 = 7;
	_speakerName = "JAILER";
}

void SpeakerJailer::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(395);
	_object1.setStrip(6);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(395);
	_object2.setStrip(5);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerGreen::SpeakerGreen(): VisualSpeaker() {
	_color1 = 96;
	_color2 = 99;
	_speakerName = "GREEN";
}

void SpeakerGreen::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(373);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerPSutter::SpeakerPSutter(): VisualSpeaker() {
	_color1 = 20;
	_color2 = 22;
	_speakerName = "PSUTTER";
}

void SpeakerPSutter::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(335);
	_object2.setStrip2(3);
	_object2.fixPriority(200);
	_object2.setPosition(Common::Point(202, 48));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerJakeRadio::SpeakerJakeRadio(): VisualSpeaker() {
	_color1 = 13;
	_color2 = 7;
	_speakerName = "JAKE_RADIO";
}

/*--------------------------------------------------------------------------*/

SpeakerLyleHat::SpeakerLyleHat(): VisualSpeaker() {
	_color1 = 29;
	_color2 = 89;
	_xp = 75;
	_speakerName = "LYLEHAT";
}

void SpeakerLyleHat::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(278);
	_object1.setStrip2(4);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + _xp,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(278);
	_object2.setStrip2(2);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + _xp,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerJordan::SpeakerJordan(): VisualSpeaker() {
	_color1 = 96;
	_color2 = 93;
	_speakerName = "JORDAN";
}

void SpeakerJordan::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(344);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 78));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerSkip::SpeakerSkip(): VisualSpeaker() {
	_color1 = 80;
	_color2 = 82;
	_speakerName = "SKIP";
}

void SpeakerSkip::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(345);
	_object1.setStrip2(4);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 282,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(345);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 282,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerSkipB::SpeakerSkipB(): VisualSpeaker() {
	_color1 = 80;
	_color2 = 82;
	_speakerName = "SKIPB";
}

void SpeakerSkipB::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(347);
	_object1.setStrip2(2);
	_object1.setFrame2(3);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 37,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(347);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 37,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerCarter::SpeakerCarter(): VisualSpeaker() {
	_color1 = 89;
	_color2 = 95;
	_speakerName = "CARTER";
}

void SpeakerCarter::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(844);
	_object1.setStrip2(8);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(78, 166));

	_object2.postInit();
	_object2.setVisage(844);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(78, 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerDriver::SpeakerDriver(): VisualSpeaker() {
	_color1 = 43;
	_color2 = 70;
	_speakerName = "DRIVER";
}

void SpeakerDriver::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(416);
	_object1.setStrip2(8);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 263,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(416);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 263,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerShooter::SpeakerShooter(): VisualSpeaker() {
	_color1 = 43;
	_color2 = 70;
	_speakerName = "SHOOTER";
}

void SpeakerShooter::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(418);
	_object1.setStrip2(7);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 268,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(418);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 268,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerJim::SpeakerJim(): VisualSpeaker() {
	_color1 = 32;
	_speakerName = "JIM";
}

void SpeakerJim::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(385);
	_object1.setStrip(7);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(262, 166));

	_object2.postInit();
	_object2.setVisage(385);
	_object2.setStrip2(6);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(262, 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerDezi::SpeakerDezi(): VisualSpeaker() {
	_color1 = 21;
	_speakerName = "DEZI";
}

void SpeakerDezi::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(385);
	_object2.setStrip2(5);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(252, 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerJake385::SpeakerJake385(): VisualSpeaker() {
	_color1 = 13;
	_speakerName = "JAKE_385";
}

void SpeakerJake385::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(1001);
	_object1.setStrip2(BF_GLOBALS.getFlag(onDuty) ? 3 : 4);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(42, 166));

	_object2.postInit();
	_object2.setVisage(1001);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(42, 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerLaura::SpeakerLaura(): VisualSpeaker() {
	_color1 = 97;
	_color2 = 105;
	_speakerName = "LAURA";
}

void SpeakerLaura::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(345);
	_object2.setStrip2(2);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 38,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerLauraHeld::SpeakerLauraHeld(): VisualSpeaker() {
	_color1 = 97;
	_color2 = 105;
	_speakerName = "LAURAHELD";
}

void SpeakerLauraHeld::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(374);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 120,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 120));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerEugene::SpeakerEugene(): VisualSpeaker() {
	_color1 = 20;
	_color2 = 22;
	_speakerName = "EUGENE";
}

void SpeakerEugene::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(465);
	_object1.setStrip2(3);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(465);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerWeasel::SpeakerWeasel(): VisualSpeaker() {
	_color1 = 8;
	_color2 = 5;
	_speakerName = "WEASEL";
}

void SpeakerWeasel::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(464);
	_object1.setStrip2(3);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(464);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerBilly::SpeakerBilly(): VisualSpeaker() {
	_color1 = 20;
	_color2 = 22;
	_speakerName = "BILLY";
}

void SpeakerBilly::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(463);
	_object1.setStrip2(7);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(463);
	_object2.setStrip2(6);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 40,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 167));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerDrunk::SpeakerDrunk(): VisualSpeaker() {
	_color1 = 21;
	_speakerName = "DRUNK";
}

void SpeakerDrunk::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(556);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 52,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerGiggles::SpeakerGiggles(): VisualSpeaker() {
	_color1 = 32;

	_speakerName = "GIGGLES";
}

void SpeakerGiggles::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(557);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 262,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerFBI::SpeakerFBI(): VisualSpeaker() {
	_color1 = 27;
	_color2 = 89;

	_speakerName = "FBI";
}

void SpeakerFBI::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(837);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 267,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerNico::SpeakerNico(): VisualSpeaker() {
	_color1 = 105;
	_color2 = 102;

	_speakerName = "NICO";
}

void SpeakerNico::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(905);
	_object1.setStrip2(2);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 262,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(905);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 262,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerDA::SpeakerDA(): VisualSpeaker() {
	_color1 = 82;
	_color2 = 80;

	_speakerName = "DA";
}

void SpeakerDA::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(915);
	_object1.setStrip2(8);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 84,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(915);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 84,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerGrandma::SpeakerGrandma(): VisualSpeaker() {
	_color1 = 20;
	_color2 = 23;

	_speakerName = "GRANDMA";
}

void SpeakerGrandma::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(274);
	_object1.setStrip2(4);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 43,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(274);
	_object2.setStrip2(3);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 43,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerLyle::SpeakerLyle(): VisualSpeaker() {
	_color1 = 29;
	_color2 = 89;

	_speakerName = "LYLE";
}

void SpeakerLyle::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(278);
	_object1.setStrip2(3);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 75,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(278);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 75,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerGranText::SpeakerGranText(): VisualSpeaker() {
	_color1 = 20;
	_color2 = 23;

	_speakerName = "GRANTEXT";
}

/*--------------------------------------------------------------------------*/

SpeakerLyleText::SpeakerLyleText(): VisualSpeaker() {
	_color1 = 29;
	_color2 = 89;

	_speakerName = "LYLETEXT";
}

/*--------------------------------------------------------------------------*/

SpeakerKate::SpeakerKate(): VisualSpeaker() {
	_color1 = 108;

	_speakerName = "KATE";
}

void SpeakerKate::setText(const Common::String &msg) {
	_removeObject1 = _removeObject2 = true;

	_object1.postInit();
	_object1.setVisage(122);
	_object1.setStrip2(8);
	_object1.fixPriority(254);
	_object1.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 270,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2.postInit();
	_object2.setVisage(122);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 270,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	_object2._numFrames = 7;
	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

/*--------------------------------------------------------------------------*/

SpeakerTony::SpeakerTony(): VisualSpeaker() {
	_color1 = 108;
	_color2 = 8;

	_speakerName = "TONY";
}

void SpeakerTony::setText(const Common::String &msg) {
	_removeObject2 = true;

	_object2.postInit();
	_object2.setVisage(132);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(BF_GLOBALS._sceneManager._scene->_sceneBounds.left + 62,
		BF_GLOBALS._sceneManager._scene->_sceneBounds.top + 166));

	VisualSpeaker::setText(msg);
	_object2.fixCountdown(8, _numFrames);
}

} // End of namespace BlueForce

} // End of namespace TsAGE
