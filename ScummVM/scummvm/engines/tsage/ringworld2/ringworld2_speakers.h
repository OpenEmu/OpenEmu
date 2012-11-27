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

#ifndef TSAGE_RINGWORLD2_SPEAKERS_H
#define TSAGE_RINGWORLD2_SPEAKERS_H

#include "common/scummsys.h"
#include "tsage/converse.h"
#include "tsage/events.h"
#include "tsage/core.h"
#include "tsage/scenes.h"
#include "tsage/globals.h"
#include "tsage/ringworld2/ringworld2_logic.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class VisualSpeaker : public Speaker {
public:
	SceneActor _object1;
	SceneObject *_object2;
	int _fieldF6, _fieldF8;
	int _displayMode;
	int _soundId;
	int _delayAmount;
	bool _removeObject;
	int _frameNumber;
	int _numFrames;
private:
	void setFrame(int numFrames);
public:
	VisualSpeaker();

	virtual Common::String getClassName() { return "VisualSpeaker"; }
	virtual void synchronize(Serializer &s);
	virtual void remove();
	virtual void setText(const Common::String &msg);
	virtual void proc15() {}
	virtual void proc16();

	void setDelay(int delay);
};

class SpeakerGameText : public VisualSpeaker {
public:
	SpeakerGameText();

	virtual Common::String getClassName() { return "SpeakerGameText"; }
};

// Classes related to Captain

class SpeakerCaptain3210 : public VisualSpeaker {
public:
	SpeakerCaptain3210();

	virtual Common::String getClassName() { return "SpeakerCaptain3210"; }
	virtual void proc15();
};

// Classes related to Caretaker

class SpeakerCaretaker2450 : public VisualSpeaker {
public:
	SpeakerCaretaker2450();

	virtual Common::String getClassName() { return "SpeakerCaretaker2450"; }
};

// Classes related to Chief

class SpeakerChief1100 : public VisualSpeaker {
public:
	SpeakerChief1100();

	virtual Common::String getClassName() { return "SpeakerChief1100"; }
	virtual void proc15();
};

// Classes related to Guard

class SpeakerGuard : public VisualSpeaker {
public:
	SpeakerGuard();
	virtual Common::String getClassName() { return "SpeakerGuard"; }
};

class SpeakerGuard2800 : public SpeakerGuard {
public:
	virtual Common::String getClassName() { return "SpeakerGuard2800"; }
	virtual void proc15();
};

// Classes related to Jocko

class SpeakerJocko : public VisualSpeaker {
public:
	SpeakerJocko();
	virtual Common::String getClassName() { return "SpeakerJocko"; }
};

class SpeakerJocko3200 : public SpeakerJocko {
public:
	virtual Common::String getClassName() { return "SpeakerJocko3200"; }
	virtual void proc15();
};

class SpeakerJocko3220 : public SpeakerJocko {
public:
	virtual Common::String getClassName() { return "SpeakerJocko3220"; }
	virtual void proc15();
};

class SpeakerJocko3230 : public SpeakerJocko {
public:
	virtual Common::String getClassName() { return "SpeakerJocko3230"; }
	virtual void proc15();
};

// Classes related to Miranda

class SpeakerMiranda : public VisualSpeaker {
public:
	SpeakerMiranda();
	virtual Common::String getClassName() { return "SpeakerMiranda"; }
};

class SpeakerMiranda300 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda300"; }
	virtual void proc15();
};

class SpeakerMiranda1625 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda1625"; }
	virtual void proc15();
};

class SpeakerMiranda3255 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda3255"; }
	virtual void proc15();
};

class SpeakerMiranda3375 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda3375"; }
	virtual void proc15();
};

class SpeakerMiranda3385 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda3385"; }
	virtual void proc15();
};

class SpeakerMiranda3395 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda3395"; }
	virtual void proc15();
};

class SpeakerMiranda3400 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda3400"; }
	virtual void proc15();
};

class SpeakerMiranda3600 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda3600"; }
	virtual void proc15();
};

class SpeakerMiranda3700 : public SpeakerMiranda {
public:
	virtual Common::String getClassName() { return "SpeakerMiranda3700"; }
	virtual void proc15();
};

// Classes related to Nej

class SpeakerNej : public VisualSpeaker {
public:
	SpeakerNej();
	virtual Common::String getClassName() { return "SpeakerNej"; }
};

class SpeakerNej2700 : public SpeakerNej {
public:
	virtual Common::String getClassName() { return "SpeakerNej2700"; }
	virtual void proc15();
};

class SpeakerNej2750 : public SpeakerNej {
public:
	virtual Common::String getClassName() { return "SpeakerNej2750"; }
	virtual void proc15();
};

class SpeakerNej2800 : public SpeakerNej {
public:
	virtual Common::String getClassName() { return "SpeakerNej2800"; }
	virtual void proc15();
};

// Classes related to Pharisha

class SpeakerPharisha : public VisualSpeaker {
public:
	SpeakerPharisha();

	virtual Common::String getClassName() { return "SpeakerPharisha"; }
};

class SpeakerPharisha2435 : public SpeakerPharisha {
public:
	virtual Common::String getClassName() { return "SpeakerPharisha2435"; }
	virtual void proc15();
};

// Classes related to Private

class SpeakerPrivate3210 : public VisualSpeaker {
public:
	SpeakerPrivate3210();

	virtual Common::String getClassName() { return "SpeakerPrivate3210"; }
	virtual void proc15();
};

// Classes related to Protector

class SpeakerProtector3600 : public VisualSpeaker {
public:
	SpeakerProtector3600();

	virtual Common::String getClassName() { return "SpeakerProtector3600"; }
	virtual void proc15();
};

// Classes related to Quinn

class SpeakerQuinn : public VisualSpeaker {
public:
	SpeakerQuinn();
	virtual Common::String getClassName() { return "SpeakerQuinn"; }
};

class SpeakerQuinn300 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn300"; }
	virtual void proc15();
};

class SpeakerQuinn1100 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn1100"; }
	virtual void proc15();
};

class SpeakerQuinn2435 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn2435"; }
	virtual void proc15();
};

class SpeakerQuinn2450 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn2450"; }
	virtual void proc15();
};

class SpeakerQuinn2700 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn2700"; }
	virtual void proc15();
};

class SpeakerQuinn2750 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn2750"; }
	virtual void proc15();
};

class SpeakerQuinn2800 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn2800"; }
	virtual void proc15();
};

class SpeakerQuinn3255 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn3255"; }
	virtual void proc15();
};

class SpeakerQuinn3375 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn3375"; }
	virtual void proc15();
};

class SpeakerQuinn3385 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn3385"; }
	virtual void proc15();
};

class SpeakerQuinn3395 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn3395"; }
	virtual void proc15();
};

class SpeakerQuinn3400 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn3400"; }
	virtual void proc15();
};

class SpeakerQuinn3600 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn3600"; }
	virtual void proc15();
};

class SpeakerQuinn3700 : public SpeakerQuinn {
public:
	virtual Common::String getClassName() { return "SpeakerQuinn3700"; }
	virtual void setText(const Common::String &msg);
	virtual void proc15();
};

// Classes related to QuinnL

class SpeakerQuinnL : public VisualSpeaker {
public:
	SpeakerQuinnL();

	virtual Common::String getClassName() { return "SpeakerQuinnL"; }
};

// Classes related to Ralf

class SpeakerRalf3245 : public VisualSpeaker {
public:
	SpeakerRalf3245();

	virtual Common::String getClassName() { return "SpeakerRalf3245"; }
	virtual void proc15();
};

// Classes related to Rocko

class SpeakerRocko : public VisualSpeaker {
public:
	SpeakerRocko();
	virtual Common::String getClassName() { return "SpeakerRocko"; }
};

class SpeakerRocko3200 : public SpeakerRocko {
public:
	virtual Common::String getClassName() { return "SpeakerRocko3200"; }
	virtual void proc15();
};

class SpeakerRocko3220 : public SpeakerRocko {
public:
	virtual Common::String getClassName() { return "SpeakerRocko3220"; }
	virtual void proc15();
};

class SpeakerRocko3230 : public SpeakerRocko {
public:
	virtual Common::String getClassName() { return "SpeakerRocko3230"; }
	virtual void proc15();
};

// Classes related to Seeker

class SpeakerSeeker : public VisualSpeaker {
public:
	SpeakerSeeker();
	virtual Common::String getClassName() { return "SpeakerSeeker"; }
};

class SpeakerSeeker300 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker300"; }
	virtual void proc15();
};

class SpeakerSeeker1100 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker1100"; }
	virtual void proc15();
};

class SpeakerSeeker1900 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker1900"; }
	virtual void proc15();
};

class SpeakerSeeker2435 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker2435"; }
	virtual void proc15();
};

class SpeakerSeeker2450 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker2450"; }
	virtual void proc15();
};

class SpeakerSeeker3375 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker3375"; }
	virtual void proc15();
};

class SpeakerSeeker3385 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker3385"; }
	virtual void proc15();
};

class SpeakerSeeker3395 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker3395"; }
	virtual void proc15();
};

class SpeakerSeeker3400 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker3400"; }
	virtual void proc15();
};

class SpeakerSeeker3600 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker3600"; }
	virtual void proc15();
};

class SpeakerSeeker3700 : public SpeakerSeeker {
public:
	virtual Common::String getClassName() { return "SpeakerSeeker3700"; }
	virtual void setText(const Common::String &msg);
	virtual void proc15();
};

// Classes related to SeekerL

class SpeakerSeekerL : public VisualSpeaker {
public:
	SpeakerSeekerL();

	virtual Common::String getClassName() { return "SpeakerSeekerL"; }
};

// Classes related to Socko

class SpeakerSocko3200 : public VisualSpeaker {
public:
	SpeakerSocko3200();

	virtual Common::String getClassName() { return "SpeakerSocko3200"; }
	virtual void proc15();
};

// Classes related to Soldier

class SpeakerSoldier : public VisualSpeaker {
public:
	SpeakerSoldier(int colour);
	virtual Common::String getClassName() { return "SpeakerSoldier"; }
};

class SpeakerSoldier300 : public SpeakerSoldier {
public:
	SpeakerSoldier300() : SpeakerSoldier(60) {}
	virtual Common::String getClassName() { return "SpeakerSoldier300"; }
	virtual void proc15();
};

class SpeakerSoldier1625 : public SpeakerSoldier {
public:
	SpeakerSoldier1625() : SpeakerSoldier(5) {}
	virtual Common::String getClassName() { return "SpeakerSoldier1625"; }
};

// Classes related to Teal

class SpeakerTeal : public VisualSpeaker {
public:
	SpeakerTeal();
	virtual Common::String getClassName() { return "SpeakerTeal"; }
};

class SpeakerTealMode7 : public SpeakerTeal {
public:
	SpeakerTealMode7();
	virtual Common::String getClassName() { return "SpeakerTealMode7"; }
};

class SpeakerTeal300 : public SpeakerTeal {
public:
	virtual Common::String getClassName() { return "SpeakerTeal300"; }
	virtual void proc15();
};

class SpeakerTeal1625 : public SpeakerTeal {
public:
	virtual Common::String getClassName() { return "SpeakerTeal1625"; }
	virtual void proc15();
};

class SpeakerTeal3240 : public SpeakerTeal {
public:
	virtual Common::String getClassName() { return "SpeakerTeal3240"; }
	virtual void proc15();
};

class SpeakerTeal3400 : public SpeakerTeal {
public:
	virtual Common::String getClassName() { return "SpeakerTeal3400"; }
	virtual void proc15();
};

class SpeakerTeal3600 : public SpeakerTealMode7 {
public:
	virtual Common::String getClassName() { return "SpeakerTeal3600"; }
	virtual void proc15();
};

// Classes related to Tomko

class SpeakerTomko3245 : public VisualSpeaker {
public:
	SpeakerTomko3245();

	virtual Common::String getClassName() { return "SpeakerTomko3245"; }
	virtual void proc15();
};

// Classes related to Webbster

class SpeakerWebbster : public VisualSpeaker {
public:
	SpeakerWebbster(int colour);
	virtual Common::String getClassName() { return "SpeakerWebbster"; }
};

class SpeakerWebbster2500 : public SpeakerWebbster {
public:
	SpeakerWebbster2500() : SpeakerWebbster(27) {}
	virtual Common::String getClassName() { return "SpeakerWebbster2500"; }
};

class SpeakerWebbster3240 : public SpeakerWebbster {
public:
	SpeakerWebbster3240() : SpeakerWebbster(10) {}

	virtual Common::String getClassName() { return "SpeakerWebbster3240"; }
	virtual void proc15();
};

class SpeakerWebbster3375 : public SpeakerWebbster {
public:
	SpeakerWebbster3375() : SpeakerWebbster(60) {}

	virtual Common::String getClassName() { return "SpeakerWebbster3375"; }
	virtual void proc15();
};

class SpeakerWebbster3385 : public SpeakerWebbster {
public:
	SpeakerWebbster3385() : SpeakerWebbster(60) {}

	virtual Common::String getClassName() { return "SpeakerWebbster3385"; }
	virtual void proc15();
};

class SpeakerWebbster3395 : public SpeakerWebbster {
public:
	SpeakerWebbster3395() : SpeakerWebbster(60) {}

	virtual Common::String getClassName() { return "SpeakerWebbster3395"; }
	virtual void proc15();
};

class SpeakerWebbster3400 : public SpeakerWebbster {
public:
	SpeakerWebbster3400() : SpeakerWebbster(27) {}

	virtual Common::String getClassName() { return "SpeakerWebbster3400"; }
	virtual void proc15();
};

class SpeakerDutyOfficer: public VisualSpeaker {
public:
	SpeakerDutyOfficer();

	virtual Common::String getClassName() { return "SpeakerDutyOfficer"; }
	virtual void proc15();
};

} // End of namespace Ringworld2
} // End of namespace TsAGE

#endif
