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

#include "tsage/ringworld/ringworld_speakers.h"
#include "tsage/scenes.h"
#include "tsage/tsage.h"
#include "tsage/graphics.h"
#include "tsage/staticres.h"

namespace TsAGE {

namespace Ringworld {

SpeakerGText::SpeakerGText() {
	_speakerName = "GTEXT";
	_textWidth = 160;
	_textPos = Common::Point(130, 10);
	_color1 = 42;
	_hideObjects = false;
}

void SpeakerGText::setText(const Common::String &msg) {
	// Set the animation properties
	_sceneObject.postInit();
	_sceneObject.setVisage(9405);
	_sceneObject.setStrip2(3);
	_sceneObject.fixPriority(255);
	_sceneObject.changeZoom(100);
	_sceneObject._frame = 1;
	_sceneObject.setPosition(Common::Point(183, 71));
	_sceneObject.animate(ANIM_MODE_7, 0, NULL);

	// Set the text
	Rect textRect;
	g_globals->gfxManager()._font.getStringBounds(msg.c_str(), textRect, _textWidth);
	textRect.center(_sceneObject._position.x, _sceneObject._position.y);
	_textPos.x = textRect.left;
	Speaker::setText(msg);
}

void SpeakerGText::removeText() {
	_sceneObject.remove();
	Speaker::removeText();
}

/*--------------------------------------------------------------------------*/

SpeakerPOR::SpeakerPOR() {
	_speakerName = "POR";
	_newSceneNumber = 7221;
	_textPos = Common::Point(10, 30);
	_color1 = 41;
}

void SpeakerPOR::SpeakerAction1::signal(){
	switch (_actionIndex++) {
	case 0:
		setDelay(g_globals->_randomSource.getRandomNumber(60) + 60);
		break;
	case 1:
		static_cast<SceneObject *>(_owner)->animate(ANIM_MODE_5, this, NULL);
		break;
	case 2:
		setDelay(g_globals->_randomSource.getRandomNumber(10));
		_actionIndex = 0;
		break;
	default:
		break;
	}
}

void SpeakerPOR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(7223);
	_object1.setStrip2(2);
	_object1.setPosition(Common::Point(191, 166));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(7223);
	_object2.setPosition(Common::Point(159, 86));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(7223);
	_object3.setStrip(3);
	_object3.setPosition(Common::Point(119, 107));
	_object3.fixPriority(199);
	_object3.setAction(&_action2);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerOR::SpeakerOR() {
	_speakerName = "OR";
	_newSceneNumber = 9430;
	_textPos = Common::Point(8, 36);
	_color1 = 42;
	_textWidth = 136;
}

void SpeakerOR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(9431);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(202, 147));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(9431);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(199, 85));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerOText::SpeakerOText() : SpeakerGText() {
	_speakerName = "OTEXT";
	_textWidth = 240;
	_textPos = Common::Point(130, 10);
	_color1 = 42;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerQText::SpeakerQText() : ScreenSpeaker() {
	_speakerName = "QTEXT";
	_textPos = Common::Point(160, 40);
	_color1 = 35;
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerSText::SpeakerSText() : ScreenSpeaker() {
	_speakerName = "STEXT";
	_color1 = 13;
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerPOText::SpeakerPOText() : ScreenSpeaker() {
	_speakerName = "POTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 41;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerMText::SpeakerMText() {
	_speakerName = "MTEXT";
	_color1 = 22;
	_textWidth = 230;
	_textMode = ALIGN_CENTER;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCText::SpeakerCText() {
	_speakerName = "CTEXT";
	_color1 = 4;
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerEText::SpeakerEText() {
	_speakerName = "ETEXT";
	_textPos = Common::Point(20, 20);
	_color1 = 22;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerGR::SpeakerGR() : AnimatedSpeaker() {
	_speakerName = "GR";
	_newSceneNumber = 9220;
	_textWidth = 136;
	_textPos = Common::Point(168, 36);
	_color1 = 14;
}

void SpeakerGR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(9221);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(101, 70));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerHText::SpeakerHText() {
	_speakerName = "HTEXT";
	_textPos = Common::Point(160, 40);
	_color1 = 52;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerSKText::SpeakerSKText() : ScreenSpeaker() {
	_speakerName = "SKTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 9;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerPText::SpeakerPText() {
	_speakerName = "PTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 5;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCHFText::SpeakerCHFText() {
	_speakerName = "CHFTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 56;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerCDRText::SpeakerCDRText() {
	_speakerName = "CDRTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 52;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerFLText::SpeakerFLText() {
	_speakerName = "FLTEXT";
	_textPos = Common::Point(10, 40);
	_color1 = 17;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerBatText::SpeakerBatText() {
	_speakerName = "BATTEXT";
	_textWidth = 240;
	_textMode = ALIGN_CENTER;
	_color1 = 3;
	_hideObjects = false;
}

/*--------------------------------------------------------------------------*/

SpeakerSKL::SpeakerSKL() : AnimatedSpeaker() {
	_speakerName = "SKL";
	_newSceneNumber = 7011;
	_textPos = Common::Point(10, 30);
	_color1 = 9;
}

void SpeakerSKL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(7013);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(203, 120));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(7013);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(197, 80));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerQL::SpeakerQL() : AnimatedSpeaker() {
	_speakerName = "QL";
	_newSceneNumber = 2610;
	_textPos = Common::Point(160, 30);
	_color1 = 35;
	_textMode = ALIGN_CENTER;
}

void SpeakerQL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2612);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(128, 146));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2612);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(122, 84));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerSR::SpeakerSR() {
	_speakerName = "SR";
	_newSceneNumber = 2811;
	_textPos = Common::Point(10, 30);
	_color1 = 13;
	_textMode = ALIGN_CENTER;
}

void SpeakerSR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2813);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(224, 198));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2813);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(203, 96));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(2813);
	_object3.setStrip(3);
	_object3.setPosition(Common::Point(204, 91));
	_object3.fixPriority(199);
	_object3._numFrames = 3;
	_object3.animate(ANIM_MODE_7, 0, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerSL::SpeakerSL() {
	_speakerName = "SL";
	_newSceneNumber = 2810;
	_textPos = Common::Point(140, 30);
	_textWidth = 160;
	_color1 = 13;
	_textMode = ALIGN_CENTER;
}

void SpeakerSL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2812);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(95, 198));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2812);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(116, 96));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerQR::SpeakerQR() {
	_speakerName = "QR";
	_newSceneNumber = 2611;
	_textPos = Common::Point(10, 30);
	_color1 = 35;
	_textMode = ALIGN_CENTER;
}

void SpeakerQR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2613);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(191, 146));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2613);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(197, 84));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerQU::SpeakerQU() {
	_speakerName = "QU";
	_newSceneNumber = 7020;
	_textPos = Common::Point(160, 30);
	_color1 = 35;
	_textMode = ALIGN_CENTER;
}

void SpeakerQU::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(7021);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(116, 120));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(7021);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(111, 84));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCR::SpeakerCR() {
	_speakerName = "CR";
	_newSceneNumber = 9010;
	_textPos = Common::Point(20, 40);
	_color1 = 4;
}

void SpeakerCR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(9011);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.setPosition(Common::Point(219, 168));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(9011);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.setPosition(Common::Point(232, 81));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerMR::SpeakerMR() {
	_speakerName = "MR";
	_newSceneNumber = 2711;
	_textPos = Common::Point(10, 40);
	_color1 = 22;
}

void SpeakerMR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2713);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(220, 143));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2713);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(215, 99));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerSAL::SpeakerSAL() {
	_speakerName = "SAL";
	_newSceneNumber = 2851;
	_textPos = Common::Point(10, 30);
	_color1 = 13;
	_textMode = ALIGN_CENTER;
}

void SpeakerSAL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2853);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(185, 200));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2853);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(170, 92));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerML::SpeakerML() {
	_speakerName = "ML";
	_newSceneNumber = 2710;
	_textPos = Common::Point(160, 40);
	_color1 = 22;
}

void SpeakerML::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(2712);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(99, 143));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(2712);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(105, 99));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCHFL::SpeakerCHFL() {
	_speakerName = "CHFL";
	_newSceneNumber = 4111;
	_textPos = Common::Point(10, 40);
	_color1 = 56;
}

void SpeakerCHFL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4113);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(205, 116));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4113);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(202, 71));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCHFR::SpeakerCHFR() {
	_speakerName = "CHFR";
	_newSceneNumber = 4110;
	_textPos = Common::Point(160, 40);
	_color1 = 56;
}

void SpeakerCHFR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4112);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(103, 116));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4112);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(106, 71));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerPL::SpeakerPL() {
	_speakerName = "PL";
	_newSceneNumber = 4060;
	_textPos = Common::Point(160, 40);
	_color1 = 5;
}

void SpeakerPL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4062);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(107, 117));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4062);
	_object2.setStrip2(1);
	_object2.fixPriority(200);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(105, 62));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(4062);
	_object3.setStrip2(3);
	_object3.fixPriority(255);
	_object3.changeZoom(100);
	_object3._frame = 1;
	_object3.setPosition(Common::Point(105, 59));
	_object3.setAction(&_speakerAction2, NULL);

	Speaker::setText(msg);
}

void SpeakerPL::removeText() {
	_object3.remove();
	AnimatedSpeaker::removeText();
}

/*--------------------------------------------------------------------------*/

SpeakerPR::SpeakerPR() {
	_speakerName = "PR";
	_newSceneNumber = 4061;
	_textPos = Common::Point(10, 40);
	_color1 = 5;
}

void SpeakerPR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4063);
	_object1.setStrip2(1);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(212, 117));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4063);
	_object2.setStrip2(2);
	_object2.fixPriority(200);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(214, 62));
	_object2.setAction(&_speakerAction, NULL);

	_object3.postInit(&_objectList);
	_object3.setVisage(4063);
	_object3.setStrip2(3);
	_object3.fixPriority(255);
	_object3.changeZoom(100);
	_object3._frame = 1;
	_object3.setPosition(Common::Point(214, 59));
	_object3.setAction(&_speakerAction2, NULL);

	Speaker::setText(msg);
}

void SpeakerPR::removeText() {
	_object3.remove();
	AnimatedSpeaker::removeText();
}

/*--------------------------------------------------------------------------*/

SpeakerCDR::SpeakerCDR() {
	_speakerName = "CDR";
	_newSceneNumber = 4161;
	_textPos = Common::Point(10, 40);
	_color1 = 52;
}

void SpeakerCDR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4163);
	_object1.setStrip2(1);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(208, 97));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4163);
	_object2.setStrip2(2);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(200, 57));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerCDL::SpeakerCDL() {
	_speakerName = "CDL";
	_newSceneNumber = 4160;
	_textPos = Common::Point(160, 40);
	_color1 = 52;
}

void SpeakerCDL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(4162);
	_object1.setStrip2(1);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(112, 97));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(4162);
	_object2.setStrip2(2);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(115, 57));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerFLL::SpeakerFLL() {
	_speakerName = "FLL";
	_newSceneNumber = 5221;
	_textPos = Common::Point(10, 40);
	_color1 = 17;
}

void SpeakerFLL::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(5223);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(216, 129));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(5223);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(210, 67));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

/*--------------------------------------------------------------------------*/

SpeakerBatR::SpeakerBatR() {
	_speakerName = "BATR";
	_newSceneNumber = 5360;
	_textPos = Common::Point(140, 40);
	_color1 = 3;
}

void SpeakerBatR::setText(const Common::String &msg) {
	_object1.postInit(&_objectList);
	_object1.setVisage(5361);
	_object1.setStrip2(2);
	_object1.fixPriority(255);
	_object1.changeZoom(100);
	_object1._frame = 1;
	_object1.setPosition(Common::Point(137, 122));
	_object1.animate(ANIM_MODE_7, 0, NULL);

	_object2.postInit(&_objectList);
	_object2.setVisage(5361);
	_object2.setStrip2(1);
	_object2.fixPriority(255);
	_object2.changeZoom(100);
	_object2._frame = 1;
	_object2.setPosition(Common::Point(137, 104));
	_object2.setAction(&_speakerAction, NULL);

	Speaker::setText(msg);
}

} // End of namespace Ringworld

} // End of namespace TsAGE
