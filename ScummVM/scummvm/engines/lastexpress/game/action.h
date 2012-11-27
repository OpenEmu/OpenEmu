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

#ifndef LASTEXPRESS_ACTION_H
#define LASTEXPRESS_ACTION_H

#include "lastexpress/shared.h"

#include "common/array.h"
#include "common/func.h"
#include "common/system.h"

namespace LastExpress {

#define DECLARE_ACTION(name) \
	SceneIndex action_##name(const SceneHotspot &hotspot) const

#define ADD_ACTION(name) \
	_actions.push_back(new Functor1MemConst<const SceneHotspot &, SceneIndex, Action>(this, &Action::action_##name));

#define IMPLEMENT_ACTION(name) \
	SceneIndex Action::action_##name(const SceneHotspot &hotspot) const { \
	debugC(6, kLastExpressDebugLogic, "Hotspot action: " #name "%s", hotspot.toString().c_str());

class LastExpressEngine;
class SceneHotspot;

class Action {
public:
	Action(LastExpressEngine *engine);
	~Action();

	// Hotspot action
	SceneIndex processHotspot(const SceneHotspot &hotspot);

	// Cursor
	CursorStyle getCursor(const SceneHotspot &hotspot) const;

	// Animation
	void playAnimation(EventIndex index, bool debugMode = false) const;

	// Compartment action
	bool handleOtherCompartment(ObjectIndex object, bool doPlaySound, bool doLoadScene) const;

private:
	typedef Common::Functor1<const SceneHotspot &, SceneIndex> ActionFunctor;

	LastExpressEngine *_engine;
	Common::Array<ActionFunctor *> _actions;

	// Each action is of the form action_<name>(SceneHotspot *hotspot)
	//   - a pointer to each action is added to the _actions array
	//   - processHotspot simply calls the proper function given by the hotspot->action value
	//
	// Note: even though there are 44 actions, only 41 are used in processHotspot

	DECLARE_ACTION(inventory);
	DECLARE_ACTION(savePoint);
	DECLARE_ACTION(playSound);
	DECLARE_ACTION(playMusic);
	DECLARE_ACTION(knock);
	DECLARE_ACTION(compartment);
	DECLARE_ACTION(playSounds);
	DECLARE_ACTION(playAnimation);
	DECLARE_ACTION(openCloseObject);
	DECLARE_ACTION(updateObjetLocation2);
	DECLARE_ACTION(setItemLocation);
	DECLARE_ACTION(knockNoSound);
	DECLARE_ACTION(pickItem);
	DECLARE_ACTION(dropItem);
	DECLARE_ACTION(enterCompartment);
	DECLARE_ACTION(getOutsideTrain);
	DECLARE_ACTION(slip);
	DECLARE_ACTION(getInsideTrain);
	DECLARE_ACTION(climbUpTrain);
	DECLARE_ACTION(climbDownTrain);
	DECLARE_ACTION(jumpUpDownTrain);
	DECLARE_ACTION(unbound);
	DECLARE_ACTION(25);
	DECLARE_ACTION(26);
	DECLARE_ACTION(27);
	DECLARE_ACTION(concertSitCough);
	DECLARE_ACTION(29);
	DECLARE_ACTION(catchBeetle);
	DECLARE_ACTION(exitCompartment);
	DECLARE_ACTION(32);
	DECLARE_ACTION(useWhistle);
	DECLARE_ACTION(openMatchBox);
	DECLARE_ACTION(openBed);
	DECLARE_ACTION(dialog);
	DECLARE_ACTION(eggBox);
	DECLARE_ACTION(39);
	DECLARE_ACTION(bed);
	DECLARE_ACTION(playMusicChapter);
	DECLARE_ACTION(playMusicChapterSetupTrain);
	DECLARE_ACTION(switchChapter);
	DECLARE_ACTION(44);

	// Special dummy function
	DECLARE_ACTION(dummy);

	// Helpers
	void pickGreenJacket(bool process) const;
	void pickScarf(bool process) const;
	void pickCorpse(ObjectLocation bedPosition, bool process) const;
	void dropCorpse(bool process) const;

	void playCompartmentSoundEvents(ObjectIndex object) const;
};

} // End of namespace LastExpress

#endif // LASTEXPRESS_ACTION_H
