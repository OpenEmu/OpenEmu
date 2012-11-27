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

#include "lastexpress/game/action.h"

#include "lastexpress/data/animation.h"
#include "lastexpress/data/cursor.h"
#include "lastexpress/data/snd.h"
#include "lastexpress/data/scene.h"

#include "lastexpress/entities/abbot.h"
#include "lastexpress/entities/anna.h"

#include "lastexpress/game/beetle.h"
#include "lastexpress/game/entities.h"
#include "lastexpress/game/inventory.h"
#include "lastexpress/game/logic.h"
#include "lastexpress/game/object.h"
#include "lastexpress/game/savegame.h"
#include "lastexpress/game/savepoint.h"
#include "lastexpress/game/scenes.h"
#include "lastexpress/game/state.h"

#include "lastexpress/sound/queue.h"

#include "lastexpress/lastexpress.h"
#include "lastexpress/resource.h"

namespace LastExpress {

static const int _animationListSize = 273;

// List of animations
static const struct {
	const char *filename;
	uint16 time;
} _animationList[_animationListSize] = {
	{"", 0},
	{"1002",    255},
	{"1002D",   255},
	{"1003",    0},
	{"1005",    195},
	{"1006",    750},   // 5
	{"1006A",   750},
	{"1008",    765},
	{"1008N",   765},
	{"1008A",   750},
	{"1008AN",  750},   // 10
	{"1009",    0},
	{"1011",    1005},
	{"1011A",   780},
	{"1012",    300},
	{"1013",    285},
	{"1017",    870},   // 15
	{"1017A",   0},     // Not in the data files?
	{"1019",    120},
	{"1019D",   120},
	{"1020",    120},   // 20
	{"1022",    525},
	{"1022A",   180},
	{"1022AD",  210},
	{"1022B",   210},
	{"1022C",   210},   // 25
	{"1023",    135},
	{"1025",    945},
	{"1028",    300},
	{"1030",    390},
	{"1031",    375},   // 30
	{"1032",    1050},
	{"1033",    945},
	{"1034",    495},
	{"1035",    1230},
	{"1037",    1425},  // 35
	{"1038",    195},
	{"1038A",   405},
	{"1039",    600},
	{"1040",    945},
	{"1041",    510},   // 40
	{"1042",    540},
	{"1043",    855},
	{"1044",    645},
	{"1046",    0},
	{"1047",    0},     // 45
	{"1047A",   0},
	{"1059",    1005},
	{"1060",    255},
	{"1063",    0},
	{"1101",    255},   // 50
	{"1102",    1320},
	{"1103",    210},
	{"1104",    120},
	{"1105",    1350},
	{"1106",    315},   // 55
	{"1106A",   315},
	{"1106D",   315},
	{"1107",    1},
	{"1107A",   660},
	{"1108",    300},   // 60
	{"1109",    1305},
	{"1110",    300},
	{"1112",    0},
	{"1115",    0},
	{"1115A",   0},     // 65
	{"1115B",   0},
	{"1115C",   0},
	{"1115D",   0},
	{"1115E",   0},
	{"1115F",   0},     // 70
	{"1115G",   0},
	{"1115H",   0},
	{"1116",    0},
	{"1117",    0},
	{"1118",    105},   // 75
	{"1202",    510},
	{"1202A",   510},
	{"1203",    720},
	{"1204",    120},
	{"1205",    465},   // 80
	{"1206",    690},
	{"1206A",   450},
	{"1208",    465},
	{"1210",    1020},
	{"1211",    600},   // 85
	{"1212",    435},
	{"1213",    525},
	{"1213A",   150},
	{"1215",    390},
	{"1216",    0},     // 90
	{"1219",    240},
	{"1222",    1095},
	{"1223",    0},
	{"1224",    720},
	{"1225",    1005},  // 95
	{"1227",    840},
	{"1227A",   840},
	{"1303",    450},
	{"1303N",   450},
	{"1304",    450},   // 100
	{"1304N",   450},
	{"1305",    630},
	{"1309",    0},
	{"1311",    1710},
	{"1312",    240},   // 105
	{"1312D",   240},
	{"1313",    930},
	{"1315",    1035},
	{"1315A",   1035},
	{"1401",    540},   // 110
	{"1402",    150},
	{"1402B",   150},
	{"1403",    90},
	{"1404",    885},
	{"1404A",   0},     // 115
	{"1405",    135},
	{"1406",    1665},
	{"1501",    285},
	{"1501A",   285},
	{"1502",    165},   // 120
	{"1502A",   165},
	{"1502D",   165},
	{"1503",    0},
	{"1504",    0},
	{"1505",    0},     // 125
	{"1505A",   0},
	{"1506",    300},
	{"1506A",   180},
	{"1508",    0},
	{"1509",    450},   // 130
	{"1509S",   450},
	{"1509A",   450},
	{"1509AS",  450},
	{"1509N",   450},
	{"1509SN",  450},   // 135
	{"1509AN",  450},
	{"1509BN",  450},
	{"1511",    150},
	{"1511A",   150},
	{"1511B",   90},    // 140
	{"1511BA",  90},
	{"1511C",   135},
	{"1511D",   105},
	{"1930",    0},
	{"1511E",   150},   // 145
	{"1512",    165},
	{"1513",    180},
	{"1517",    0},
	{"1517A",   165},
	{"1518",    165},   // 150
	{"1518A",   165},
	{"1518B",   165},
	{"1591",    450},
	{"1592",    450},
	{"1593",    450},   // 155
	{"1594",    450},
	{"1595",    450},
	{"1596",    450},
	{"1601",    0},
	{"1603",    0},     // 160
	{"1606B",   315},
	{"1607A",   0},
	{"1610",    0},
	{"1611",    0},
	{"1612",    0},     // 165
	{"1615",    0},
	{"1619",    0},
	{"1620",    120},
	{"1621",    105},
	{"1622",    105},   // 170
	{"1629",    450},
	{"1630",    450},
	{"1631",    525},
	{"1632",    0},
	{"1633",    615},   // 175
	{"1634",    180},
	{"1702",    180},
	{"1702DD",  180},
	{"1702NU",  180},
	{"1702ND",  180},   // 180
	{"1704",    300},
	{"1704D",   300},
	{"1705",    195},
	{"1705D",   195},
	{"1706",    195},   // 185
	{"1706DD",  195},
	{"1706ND",  195},
	{"1706NU",  195},
	{"1901",    135},
	{"1902",    1410},  // 190
	{"1903",    0},
	{"1904",    1920},
	{"1908",    600},
	{"1908A",   195},
	{"1908B",   105},   // 195
	{"1908C",   165},
	{"1908CD",  0},
	{"1909A",   150},
	{"1909B",   150},
	{"1909C",   150},   // 200
	{"1910A",   180},
	{"1910B",   180},
	{"1910C",   180},
	{"1911A",   90},
	{"1911B",   90},    // 205
	{"1911C",   90},
	{"1912",    0},
	{"1913",    0},
	{"1917",    0},
	{"1918",    390},   // 210
	{"1919",    360},
	{"1919A",   105},
	{"1920",    75},
	{"1922",    75},
	{"1923",    150},   // 215
	{"8001",    120},
	{"8001A",   120},
	{"8002",    120},
	{"8002A",   120},
	{"8002B",   120},   // 220
	{"8003",    105},
	{"8003A",   105},
	{"8004",    105},
	{"8004A",   105},
	{"8005",    270},   // 225
	{"8005B",   270},
	{"8010",    270},
	{"8013",    120},
	{"8013A",   120},
	{"8014",    165},   // 230
	{"8014A",   165},
	{"8014R",   165},
	{"8014AR",  165},
	{"8015",    150},
	{"8015A",   150},   // 235
	{"8015R",   150},
	{"8015AR",  150},
	{"8017",    120},
	{"8017A",   120},
	{"8017R",   120},   // 240
	{"8017AR",  120},
	{"8017N",   90},
	{"8023",    135},
	{"8023A",   135},
	{"8023M",   135},   // 245
	{"8024",    150},
	{"8024A",   180},
	{"8024M",   180},
	{"8025",    150},
	{"8025A",   150},   // 250
	{"8025M",   150},
	{"8027",    75},
	{"8028",    75},
	{"8029",    120},
	{"8029A",   120},   // 255
	{"8031",    375},
	{"8032",    0},
	{"8032A",   0},
	{"8033",    105},
	{"8035",    195},   // 260
	{"8035A",   120},
	{"8035B",   180},
	{"8035C",   135},
	{"8036",    105},
	{"8037",    195},   // 265
	{"8037A",   195},
	{"8040",    240},
	{"8040A",   240},
	{"8041",    195},
	{"8041A",   195},   // 270
	{"8042",    600},
	{"8042A",   600}
};

template<class Arg, class Res, class T>
class Functor1MemConst : public Common::Functor1<Arg, Res> {
public:
	typedef Res (T::*FuncType)(Arg) const;

	Functor1MemConst(T *t, const FuncType &func) : _t(t), _func(func) {}

	bool isValid() const { return _func != 0 && _t != 0; }
	Res operator()(Arg v1) const {
		return (_t->*_func)(v1);
	}
private:
	mutable T *_t;
	const FuncType _func;
};

Action::Action(LastExpressEngine *engine) : _engine(engine) {
	ADD_ACTION(dummy);
	ADD_ACTION(inventory);
	ADD_ACTION(savePoint);
	ADD_ACTION(playSound);
	ADD_ACTION(playMusic);
	ADD_ACTION(knock);
	ADD_ACTION(compartment);
	ADD_ACTION(playSounds);
	ADD_ACTION(playAnimation);
	ADD_ACTION(openCloseObject);
	ADD_ACTION(updateObjetLocation2);
	ADD_ACTION(setItemLocation);
	ADD_ACTION(knockNoSound);
	ADD_ACTION(pickItem);
	ADD_ACTION(dropItem);
	ADD_ACTION(dummy);
	ADD_ACTION(enterCompartment);
	ADD_ACTION(dummy);
	ADD_ACTION(getOutsideTrain);
	ADD_ACTION(slip);
	ADD_ACTION(getInsideTrain);
	ADD_ACTION(climbUpTrain);
	ADD_ACTION(climbDownTrain);
	ADD_ACTION(jumpUpDownTrain);
	ADD_ACTION(unbound);
	ADD_ACTION(25);
	ADD_ACTION(26);
	ADD_ACTION(27);
	ADD_ACTION(concertSitCough);
	ADD_ACTION(29);
	ADD_ACTION(catchBeetle);
	ADD_ACTION(exitCompartment);
	ADD_ACTION(32);
	ADD_ACTION(useWhistle);
	ADD_ACTION(openMatchBox);
	ADD_ACTION(openBed);
	ADD_ACTION(dummy);
	ADD_ACTION(dialog);
	ADD_ACTION(eggBox);
	ADD_ACTION(39);
	ADD_ACTION(bed);
	ADD_ACTION(playMusicChapter);
	ADD_ACTION(playMusicChapterSetupTrain);
	ADD_ACTION(switchChapter);
	ADD_ACTION(44);
}

Action::~Action() {
	for (uint i = 0; i < _actions.size(); i++)
		SAFE_DELETE(_actions[i]);

	_actions.clear();

	// Zero-out passed pointers
	_engine = NULL;
}

//////////////////////////////////////////////////////////////////////////
// Processing hotspot
//////////////////////////////////////////////////////////////////////////
SceneIndex Action::processHotspot(const SceneHotspot &hotspot) {
	if (!hotspot.action || hotspot.action >= (int)_actions.size())
		return kSceneInvalid;

	return (*_actions[hotspot.action])(hotspot);
}

//////////////////////////////////////////////////////////////////////////
// Actions
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
// Action 0
IMPLEMENT_ACTION(dummy)
	error("[Action::action_dummy] Dummy action function called (hotspot action: %d)", hotspot.action);
}

//////////////////////////////////////////////////////////////////////////
// Action 1
IMPLEMENT_ACTION(inventory)
	if (!getState()->sceneUseBackup)
		return kSceneInvalid;

	SceneIndex index = kSceneNone;
	if (getState()->sceneBackup2) {
		index = getState()->sceneBackup2;
		getState()->sceneBackup2 = kSceneNone;
	} else {
		getState()->sceneUseBackup = false;
		index = getState()->sceneBackup;

		Scene *backup = getScenes()->get(getState()->sceneBackup);
		if (getEntities()->getPosition(backup->car, backup->position))
			index = getScenes()->processIndex(getState()->sceneBackup);
	}

	getScenes()->loadScene(index);

	if (!getInventory()->getSelectedItem())
		return kSceneInvalid;

	if (!getInventory()->getSelectedEntry()->isSelectable || (!getState()->sceneBackup2 && getInventory()->getFirstExaminableItem()))
		getInventory()->selectItem(getInventory()->getFirstExaminableItem());

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 2
IMPLEMENT_ACTION(savePoint)
	getSavePoints()->push(kEntityPlayer, (EntityIndex)hotspot.param1, (ActionIndex)hotspot.param2);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 3
IMPLEMENT_ACTION(playSound)

	// Check that the file is not already buffered
	if (hotspot.param2 || !getSoundQueue()->isBuffered(Common::String::format("LIB%03d", hotspot.param1), true))
		getSound()->playSoundEvent(kEntityPlayer, hotspot.param1, hotspot.param2);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 4
IMPLEMENT_ACTION(playMusic)
	// Check that the file is not already buffered
	Common::String filename = Common::String::format("MUS%03d", hotspot.param1);

	if (!getSoundQueue()->isBuffered(filename) && (hotspot.param1 != 50 || getProgress().chapter == kChapter5))
		getSound()->playSound(kEntityPlayer, filename, kFlagDefault, hotspot.param2);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 5
IMPLEMENT_ACTION(knock)
	ObjectIndex object = (ObjectIndex)hotspot.param1;
	if (object >= kObjectMax)
		return kSceneInvalid;

	if (getObjects()->get(object).entity) {
		getSavePoints()->push(kEntityPlayer, getObjects()->get(object).entity, kActionKnock, object);
	} else {
		if (!getSoundQueue()->isBuffered("LIB012", true))
			getSound()->playSoundEvent(kEntityPlayer, 12);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 6
IMPLEMENT_ACTION(compartment)
	ObjectIndex compartment = (ObjectIndex)hotspot.param1;

	if (compartment >= kObjectMax)
		return kSceneInvalid;

	if (getObjects()->get(compartment).entity) {
		getSavePoints()->push(kEntityPlayer, getObjects()->get(compartment).entity, kActionOpenDoor, compartment);

		// Stop processing further
		return kSceneNone;
	}

	if (handleOtherCompartment(compartment, true, true)) {
		// Stop processing further
		return kSceneNone;
	}

	ObjectLocation location = getObjects()->get(compartment).location;
	if (location == kObjectLocation1 || location == kObjectLocation3 || getEntities()->checkFields2(compartment)) {

		if (location != kObjectLocation1 || getEntities()->checkFields2(compartment)
		 || (getInventory()->getSelectedItem() != kItemKey
		 && (compartment != kObjectCompartment1
		  || !getInventory()->hasItem(kItemKey)
		  || (getInventory()->getSelectedItem() != kItemFirebird && getInventory()->getSelectedItem() != kItemBriefcase)))) {
			if (!getSoundQueue()->isBuffered("LIB13"))
				getSound()->playSoundEvent(kEntityPlayer, 13);

			// Stop processing further
			return kSceneNone;
		}

		getSound()->playSoundEvent(kEntityPlayer, 32);

		if ((compartment >= kObjectCompartment1 && compartment <= kObjectCompartment3) || (compartment >= kObjectCompartmentA && compartment <= kObjectCompartmentF))
			getObjects()->update(compartment, kEntityPlayer, kObjectLocationNone, kCursorHandKnock, kCursorHand);

		getSound()->playSoundEvent(kEntityPlayer, 15, 22);
		getInventory()->unselectItem();

		return kSceneInvalid;
	}

	if (hotspot.action != SceneHotspot::kActionEnterCompartment || getInventory()->getSelectedItem() != kItemKey) {
		if (compartment == kObjectCageMax) {
			getSound()->playSoundEvent(kEntityPlayer, 26);
		} else {
			getSound()->playSoundEvent(kEntityPlayer, 14);
			getSound()->playSoundEvent(kEntityPlayer, 15, 22);
		}
		return kSceneInvalid;
	}

	getObjects()->update(kObjectCompartment1, kEntityPlayer, kObjectLocation1, kCursorHandKnock, kCursorHand);
	getSound()->playSoundEvent(kEntityPlayer, 16);
	getInventory()->unselectItem();

	// Stop processing further
	return kSceneNone;
}

//////////////////////////////////////////////////////////////////////////
// Action 7
IMPLEMENT_ACTION(playSounds)
	getSound()->playSoundEvent(kEntityPlayer, hotspot.param1);
	getSound()->playSoundEvent(kEntityPlayer, hotspot.param3, hotspot.param2);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 8
IMPLEMENT_ACTION(playAnimation)
	if (getEvent(hotspot.param1))
		return kSceneInvalid;

	playAnimation((EventIndex)hotspot.param1);

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 9
IMPLEMENT_ACTION(openCloseObject)
	ObjectIndex object = (ObjectIndex)hotspot.param1;
	ObjectLocation location = (ObjectLocation)hotspot.param2;

	if (object >= kObjectMax)
		return kSceneInvalid;

	getObjects()->update(object, getObjects()->get(object).entity, location, kCursorKeepValue, kCursorKeepValue);

	bool isNotWindow = ((object <= kObjectCompartment8  || object >= kObjectHandleBathroom) && (object <= kObjectCompartmentH || object >= kObject48));

	switch (location) {
	default:
		break;

	case kObjectLocation1:
		if (isNotWindow)
			getSound()->playSoundEvent(kEntityPlayer, 24);
		else
			getSound()->playSoundEvent(kEntityPlayer, 21);
		break;

	case kObjectLocation2:
		if (isNotWindow)
			getSound()->playSoundEvent(kEntityPlayer, 36);
		else
			getSound()->playSoundEvent(kEntityPlayer, 20);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 10
IMPLEMENT_ACTION(updateObjetLocation2)
	ObjectIndex object = (ObjectIndex)hotspot.param1;
	ObjectLocation location = (ObjectLocation)hotspot.param2;

	if (object >= kObjectMax)
		return kSceneInvalid;

	getObjects()->updateLocation2(object, location);

	if (object != kObject112 || getSoundQueue()->isBuffered("LIB096")) {
		if (object == 1)
			getSound()->playSoundEvent(kEntityPlayer, 73);
	} else {
		getSound()->playSoundEvent(kEntityPlayer, 96);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 11
IMPLEMENT_ACTION(setItemLocation)
	InventoryItem item = (InventoryItem)hotspot.param1;
	if (item >= kPortraitOriginal)
		return kSceneInvalid;

	Inventory::InventoryEntry *entry = getInventory()->get(item);
	if (entry->isPresent)
		return kSceneInvalid;

	entry->location = (ObjectLocation)hotspot.param2;

	if (item == kItemCorpse) {
		ObjectLocation corpseLocation = getInventory()->get(kItemCorpse)->location;
		getProgress().eventCorpseMovedFromFloor = (corpseLocation == kObjectLocation3 || corpseLocation == kObjectLocation4);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 12
IMPLEMENT_ACTION(knockNoSound)
	ObjectIndex object = (ObjectIndex)hotspot.param1;
	if (object >= kObjectMax)
		return kSceneInvalid;

	if (getObjects()->get(object).entity)
		getSavePoints()->push(kEntityPlayer, getObjects()->get(object).entity, kActionKnock, object);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 13
IMPLEMENT_ACTION(pickItem)
	InventoryItem item = (InventoryItem)hotspot.param1;
	ObjectLocation location = (ObjectLocation)hotspot.param2;
	bool process = (hotspot.scene == 0);
	SceneIndex sceneIndex = kSceneInvalid;

	if (item >= kPortraitOriginal)
		return kSceneInvalid;

	Inventory::InventoryEntry *entry = getInventory()->get(item);
	if (!entry->location)
		return kSceneInvalid;

	// Special case for corpse
	if (item == kItemCorpse) {
		pickCorpse(location, process);
		return kSceneInvalid;
	}

	// Add and process items
	getInventory()->addItem(item);

	switch (item) {
	default:
		break;

	case kItemGreenJacket:
		pickGreenJacket(process);
		break;

	case kItemScarf:
		pickScarf(process);

		// stop processing
		return kSceneInvalid;

	case kItemParchemin:
		if (location != kObjectLocation2)
			break;

		getInventory()->addItem(kItemParchemin);
		getInventory()->get(kItem11)->location = kObjectLocation1;
		getSound()->playSoundEvent(kEntityPlayer, 9);
		break;

	case kItemBomb:
		RESET_ENTITY_STATE(kEntityAbbot, Abbot, setup_pickBomb);
		break;

	case kItemBriefcase:
		getSound()->playSoundEvent(kEntityPlayer, 83);
		break;
	}

	// Load item scene
	if (getInventory()->get(item)->scene) {
		if (!getState()->sceneUseBackup) {
			getState()->sceneUseBackup = true;
			getState()->sceneBackup = (hotspot.scene ? hotspot.scene : getState()->scene);
		}

		getScenes()->loadScene(getInventory()->get(item)->scene);

		// do not process further
		sceneIndex = kSceneNone;
	}

	// Select item
	if (getInventory()->get(item)->isSelectable) {
		getInventory()->selectItem(item);
		_engine->getCursor()->setStyle(getInventory()->get(item)->cursor);
	}

	return sceneIndex;
}

//////////////////////////////////////////////////////////////////////////
// Action 14
IMPLEMENT_ACTION(dropItem)
	InventoryItem item = (InventoryItem)hotspot.param1;
	ObjectLocation location = (ObjectLocation)hotspot.param2;
	bool process = (hotspot.scene == kSceneNone);

	if (item >= kPortraitOriginal)
		return kSceneInvalid;

	if (!getInventory()->hasItem(item))
		return kSceneInvalid;

	if (location < kObjectLocation1)
		return kSceneInvalid;

	// Handle actions
	if (item == kItemBriefcase) {
		getSound()->playSoundEvent(kEntityPlayer, 82);

		if (location == kObjectLocation2) {
			if (!getProgress().field_58) {
				getSaveLoad()->saveGame(kSavegameTypeTime, kEntityPlayer, kTimeNone);
				getProgress().field_58 = 1;
			}

			if (getInventory()->get(kItemParchemin)->location == kObjectLocation2) {
				getInventory()->addItem(kItemParchemin);
				getInventory()->get(kItem11)->location = kObjectLocation1;
				getSound()->playSoundEvent(kEntityPlayer, 9);
			}
		}
	}

	// Update item location
	getInventory()->removeItem(item, location);

	if (item == kItemCorpse)
		dropCorpse(process);

	// Unselect item
	getInventory()->unselectItem();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 15: Dummy action

//////////////////////////////////////////////////////////////////////////
// Action 16
IMPLEMENT_ACTION(enterCompartment)
	if (getObjects()->get(kObjectCompartment1).location == kObjectLocation1 || getObjects()->get(kObjectCompartment1).location == kObjectLocation3 || getInventory()->getSelectedItem() == kItemKey)
		return action_compartment(hotspot);

	if (getProgress().eventCorpseFound) {
		if (hotspot.action != SceneHotspot::kActionEnterCompartment || getInventory()->get(kItemBriefcase)->location != kObjectLocation2)
			return action_compartment(hotspot);

		getSound()->playSoundEvent(kEntityPlayer, 14);
		getSound()->playSoundEvent(kEntityPlayer, 15, 22);

		if (getProgress().field_78 && !getSoundQueue()->isBuffered("MUS003")) {
			getSound()->playSound(kEntityPlayer, "MUS003", kFlagDefault);
			getProgress().field_78 = 0;
		}

		getScenes()->loadSceneFromPosition(kCarGreenSleeping, 77);

		return kSceneNone;
	}

	getSaveLoad()->saveGame(kSavegameTypeTime, kEntityPlayer, kTimeNone);
	getSound()->playSound(kEntityPlayer, "LIB014");
	playAnimation(kEventCathFindCorpse);
	getSound()->playSound(kEntityPlayer, "LIB015");
	getProgress().eventCorpseFound = true;

	return kSceneCompartmentCorpse;
}

//////////////////////////////////////////////////////////////////////////
// Action 17: Dummy action

//////////////////////////////////////////////////////////////////////////
// Action 18
IMPLEMENT_ACTION(getOutsideTrain)
	ObjectIndex object = (ObjectIndex)hotspot.param1;

	if ((getEvent(kEventCathLookOutsideWindowDay) || getEvent(kEventCathLookOutsideWindowNight) || getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1)
	  && getProgress().isTrainRunning
	  && (object != kObjectOutsideAnnaCompartment || (!getEntities()->isInsideCompartment(kEntityRebecca, kCarRedSleeping, kPosition_4840) && getObjects()->get(kObjectOutsideBetweenCompartments).location == kObjectLocation2))
	  && getInventory()->getSelectedItem() != kItemFirebird
	  && getInventory()->getSelectedItem() != kItemBriefcase) {

		switch (object) {
		default:
			return kSceneInvalid;

		case kObjectOutsideTylerCompartment:
			getEvent(kEventCathLookOutsideWindowDay) = 1;
			playAnimation(isNight() ? kEventCathGoOutsideTylerCompartmentNight : kEventCathGoOutsideTylerCompartmentDay);
			getProgress().field_C8 = 1;
			break;

		case kObjectOutsideBetweenCompartments:
			getEvent(kEventCathLookOutsideWindowDay) = 1;
			playAnimation(isNight() ? kEventCathGoOutsideNight : kEventCathGoOutsideDay);
			getProgress().field_C8 = 1;
			break;

		case kObjectOutsideAnnaCompartment:
			getEvent(kEventCathLookOutsideWindowDay) = 1;
			playAnimation(isNight() ? kEventCathGetInsideNight : kEventCathGetInsideDay);
			if (!hotspot.scene)
				getScenes()->processScene();
			break;
		}
	} else {
		if (object == kObjectOutsideTylerCompartment || object == kObjectOutsideBetweenCompartments || object == kObjectOutsideAnnaCompartment) {
			playAnimation(isNight() ? kEventCathLookOutsideWindowNight : kEventCathLookOutsideWindowDay);
			getScenes()->processScene();
			return kSceneNone;
		}
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 19
IMPLEMENT_ACTION(slip)
	switch((ObjectIndex)hotspot.param1) {
	default:
		return kSceneInvalid;

	case kObjectOutsideTylerCompartment:
		playAnimation(isNight() ? kEventCathSlipTylerCompartmentNight : kEventCathSlipTylerCompartmentDay);
		break;

	case kObjectOutsideBetweenCompartments:
		playAnimation(isNight() ? kEventCathSlipNight : kEventCathSlipDay);
		break;
	}

	getProgress().field_C8 = 0;

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 20
IMPLEMENT_ACTION(getInsideTrain)
	switch ((ObjectIndex)hotspot.param1) {
	default:
		return kSceneInvalid;

	case kObjectOutsideTylerCompartment:
		playAnimation(isNight() ? kEventCathGetInsideTylerCompartmentNight : kEventCathGetInsideTylerCompartmentDay);
		break;

	case kObjectOutsideBetweenCompartments:
		playAnimation(isNight() ? kEventCathGetInsideNight : kEventCathGetInsideDay);
		break;

	case kObjectOutsideAnnaCompartment:
		playAnimation(kEventCathGettingInsideAnnaCompartment);
		break;
	}

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 21
IMPLEMENT_ACTION(climbUpTrain)
	byte action = hotspot.param1;

	if (action != 1 && action != 2)
		return kSceneInvalid;

	switch (getProgress().chapter) {
	default:
		break;

	case kChapter2:
	case kChapter3:
		if (action == 2)
			playAnimation(kEventCathClimbUpTrainGreenJacket);

		playAnimation(kEventCathTopTrainGreenJacket);
		break;

	case kChapter5:
		if (action == 2)
			playAnimation(getProgress().isNightTime ? kEventCathClimbUpTrainNoJacketNight : kEventCathClimbUpTrainNoJacketDay);

		playAnimation(getProgress().isNightTime ? kEventCathTopTrainNoJacketNight : kEventCathTopTrainNoJacketDay);
		break;
	}

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 22
IMPLEMENT_ACTION(climbDownTrain)
	EventIndex evt = kEventNone;
	switch (getProgress().chapter) {
	default:
		return kSceneInvalid;

	case kChapter2:
	case kChapter3:
		evt = kEventCathClimbDownTrainGreenJacket;
		break;

	case kChapter5:
		evt = (getProgress().isNightTime ? kEventCathClimbDownTrainNoJacketNight : kEventCathClimbDownTrainNoJacketDay);
		break;
	}

	playAnimation(evt);
	if (evt == kEventCathClimbDownTrainNoJacketDay)
		getSound()->playSoundEvent(kEntityPlayer, 37);

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 23
IMPLEMENT_ACTION(jumpUpDownTrain)
	switch (hotspot.param1) {
	default:
		break;

	case 1:
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kActionBreakCeiling);
		break;

	case 2:
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kActionJumpDownCeiling);
		break;

	case 3:
		if (getInventory()->getSelectedItem() == kItemBriefcase) {
			getInventory()->removeItem(kItemBriefcase, kObjectLocation3);
			getSound()->playSoundEvent(kEntityPlayer, 82);
			getInventory()->unselectItem();
		}

		// Show animation with or without briefcase
		playAnimation((getInventory()->get(kItemBriefcase)->location - 3) ? kEventCathJumpUpCeilingBriefcase : kEventCathJumpUpCeiling);

		if (!hotspot.scene)
			getScenes()->processScene();

		break;

	case 4:
		if (getProgress().chapter == kChapter1)
			getSavePoints()->push(kEntityPlayer, kEntityKronos, kAction202621266);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 24
IMPLEMENT_ACTION(unbound)
	byte action = hotspot.param1;

	switch (action) {
	default:
		break;

	case 1:
		playAnimation(kEventCathStruggleWithBonds);
		if (hotspot.scene)
			getScenes()->processScene();
		break;

	case 2:
		playAnimation(kEventCathBurnRope);
		if (hotspot.scene)
			getScenes()->processScene();
		break;

	case 3:
		if (getEvent(kEventCathBurnRope)) {
			playAnimation(kEventCathRemoveBonds);
			getProgress().field_84 = 0;
			getScenes()->loadSceneFromPosition(kCarBaggageRear, 89);
			return kSceneNone;
		}
		break;

	case 4:
		if (!getEvent(kEventCathStruggleWithBonds2)) {
			playAnimation(kEventCathStruggleWithBonds2);
			getSound()->playSoundEvent(kEntityPlayer, 101);
			getInventory()->setLocationAndProcess(kItemMatch, kObjectLocation2);
			if (!hotspot.scene)
				getScenes()->processScene();
		}
		break;

	case 5:
		getSavePoints()->push(kEntityPlayer, kEntityIvo, kAction192637492);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 25
IMPLEMENT_ACTION(25)
	switch(hotspot.param1) {
	default:
		break;

	case 1:
		getSavePoints()->push(kEntityPlayer, kEntityAnna, kAction272177921);
		break;

	case 2:
		if (!getSoundQueue()->isBuffered("MUS021"))
			getSound()->playSound(kEntityPlayer, "MUS021", kFlagDefault);
		break;

	case 3:
		getSound()->playSoundEvent(kEntityPlayer, 43);
		if (!getInventory()->hasItem(kItemKey) && !getEvent(kEventAnnaBaggageArgument)) {
			RESET_ENTITY_STATE(kEntityAnna, Anna, setup_baggage);
			return kSceneNone;
		}
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 26
IMPLEMENT_ACTION(26)
	switch(hotspot.param1) {
	default:
		return kSceneInvalid;

	case 1:
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kAction158610240);
		break;

	case 2:
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kAction225367984);
		getInventory()->unselectItem();
		return kSceneNone;

	case 3:
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kAction191001984);
		return kSceneNone;

	case 4:
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kAction201959744);
		return kSceneNone;

	case 5:
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kAction169300225);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 27
IMPLEMENT_ACTION(27)
	if (!getSoundQueue()->isBuffered("LIB031", true))
		getSound()->playSoundEvent(kEntityPlayer, 31);

	switch (getEntityData(kEntityPlayer)->car) {
	default:
		break;

	case kCarGreenSleeping:
		getSavePoints()->push(kEntityPlayer, kEntityMertens, kAction225358684, hotspot.param1);
		break;

	case kCarRedSleeping:
		getSavePoints()->push(kEntityPlayer, kEntityCoudert, kAction225358684, hotspot.param1);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 28
IMPLEMENT_ACTION(concertSitCough)
	switch(hotspot.param1) {
	default:
		return kSceneInvalid;

	case 1:
		playAnimation(kEventConcertSit);
		break;

	case 2:
		playAnimation(kEventConcertCough);
		break;
	}

	if (!hotspot.scene)
		getScenes()->processScene();

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 29
IMPLEMENT_ACTION(29)
	getProgress().field_C = 1;
	getSound()->playSoundEvent(kEntityPlayer, hotspot.param1, hotspot.param2);

	Common::String filename = Common::String::format("MUS%03d", hotspot.param3);
	if (!getSoundQueue()->isBuffered(filename))
		getSound()->playSound(kEntityPlayer, filename, kFlagDefault);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 30
IMPLEMENT_ACTION(catchBeetle)
	if (!getBeetle()->isLoaded())
		return kSceneInvalid;

	if (getBeetle()->catchBeetle()) {
		getBeetle()->unload();
		getInventory()->get(kItemBeetle)->location = kObjectLocation1;
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kActionCatchBeetle);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 31
IMPLEMENT_ACTION(exitCompartment)
	if (!getProgress().field_30 && getProgress().jacket != kJacketOriginal) {
		getSaveLoad()->saveGame(kSavegameTypeTime, kEntityPlayer, kTimeNone);
		getProgress().field_30 = 1;
	}

	getObjects()->updateLocation2(kObjectCompartment1, (ObjectLocation)hotspot.param2);

	// fall to case enterCompartment action
	return action_enterCompartment(hotspot);
}

//////////////////////////////////////////////////////////////////////////
// Action 32
IMPLEMENT_ACTION(32)
	switch(hotspot.param1) {
	default:
		break;

	case 1:
		getSavePoints()->push(kEntityPlayer, kEntitySalko, kAction167992577);
		break;

	case 2:
		getSavePoints()->push(kEntityPlayer, kEntityVesna, kAction202884544);
		break;

	case 3:
		if (getProgress().chapter == kChapter5) {
			getSavePoints()->push(kEntityPlayer, kEntityAbbot, kAction168646401);
			getSavePoints()->push(kEntityPlayer, kEntityMilos, kAction168646401);
		} else {
			getSavePoints()->push(kEntityPlayer, kEntityTrain, kAction203339360);
		}
		// Stop processing further scenes
		return kSceneNone;

	case 4:
		getSavePoints()->push(kEntityPlayer, kEntityMilos, kAction169773228);
		break;

	case 5:
		getSavePoints()->push(kEntityPlayer, kEntityVesna, kAction167992577);
		break;

	case 6:
		getSavePoints()->push(kEntityPlayer, kEntityAugust, kAction203078272);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 33
IMPLEMENT_ACTION(useWhistle)
	EventIndex evt = kEventNone;
	SceneIndex sceneIndex = kSceneInvalid;

	switch (hotspot.param1) {
	default:
		break;

	case 1:
		if (getEvent(kEventKronosBringFirebird)) {
			getSavePoints()->push(kEntityPlayer, kEntityAnna, kAction205294778);
			break;
		}

		if (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_8200)) {
			evt = kEventCathOpenEgg;

			Scene *scene = getScenes()->get(hotspot.scene);
			if (scene->getHotspot())
				sceneIndex = scene->getHotspot()->scene;

		} else {
			evt = kEventCathOpenEggNoBackground;
		}
		getProgress().isEggOpen = true;
		break;

	case 2:
		if (getEvent(kEventKronosBringFirebird)) {
			getSavePoints()->push(kEntityPlayer, kEntityAnna, kAction224309120);
			break;
		}

		evt = (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_8200)) ? kEventCathCloseEgg : kEventCathCloseEggNoBackground;
		getProgress().isEggOpen = false;
		break;

	case 3:
		if (getEvent(kEventKronosBringFirebird)) {
			getSavePoints()->push(kEntityPlayer, kEntityAnna, kActionUseWhistle);
			break;
		}

		evt = (getEntities()->isInsideCompartment(kEntityPlayer, kCarGreenSleeping, kPosition_8200)) ? kEventCathUseWhistleOpenEgg : kEventCathUseWhistleOpenEggNoBackground;
		break;

	}

	if (evt != kEventNone) {
		playAnimation(evt);
		if (sceneIndex == kSceneNone || !hotspot.scene)
			getScenes()->processScene();
	}

	return sceneIndex;
}

//////////////////////////////////////////////////////////////////////////
// Action 34
IMPLEMENT_ACTION(openMatchBox)
	// If the match is already in the inventory, do nothing
	if (!getInventory()->get(kItemMatch)->location
	 || getInventory()->get(kItemMatch)->isPresent)
		return kSceneInvalid;

	getInventory()->addItem(kItemMatch);
	getSound()->playSoundEvent(kEntityPlayer, 102);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 35
IMPLEMENT_ACTION(openBed)
	getSound()->playSoundEvent(kEntityPlayer, 59);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 36: Dummy action

//////////////////////////////////////////////////////////////////////////
// Action 37
IMPLEMENT_ACTION(dialog)
	getSound()->playDialog(kEntityTables4, (EntityIndex)hotspot.param1, kFlagDefault, 0);

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 38
IMPLEMENT_ACTION(eggBox)
	getSound()->playSoundEvent(kEntityPlayer, 43);
	if (getProgress().field_7C && !getSoundQueue()->isBuffered("MUS003")) {
		getSound()->playSound(kEntityPlayer, "MUS003", kFlagDefault);
		getProgress().field_7C = 0;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 39
IMPLEMENT_ACTION(39)
	getSound()->playSoundEvent(kEntityPlayer, 24);
	if (getProgress().field_80 && !getSoundQueue()->isBuffered("MUS003")) {
		getSound()->playSound(kEntityPlayer, "MUS003", kFlagDefault);
		getProgress().field_80 = 0;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 40
IMPLEMENT_ACTION(bed)
	getSound()->playSoundEvent(kEntityPlayer, 85);
	// falls to case knockNoSound
	return action_knockNoSound(hotspot);
}

//////////////////////////////////////////////////////////////////////////
// Action 41
IMPLEMENT_ACTION(playMusicChapter)
	byte id = 0;
	switch (getProgress().chapter) {
	default:
		break;

	case kChapter1:
		id = hotspot.param1;
		break;

	case kChapter2:
	case kChapter3:
		id = hotspot.param2;
		break;

	case kChapter4:
	case kChapter5:
		id = hotspot.param3;
		break;
	}

	if (id) {
		Common::String filename = Common::String::format("MUS%03d", id);

		if (!getSoundQueue()->isBuffered(filename))
			getSound()->playSound(kEntityPlayer, filename, kFlagDefault);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 42
IMPLEMENT_ACTION(playMusicChapterSetupTrain)
	int id = 0;
	switch (getProgress().chapter) {
	default:
		break;

	case kChapter1:
		id = 1;
		break;

	case kChapter2:
	case kChapter3:
		id = 2;
		break;

	case kChapter4:
	case kChapter5:
		id = 4;
		break;
	}

	Common::String filename = Common::String::format("MUS%03d", hotspot.param1);

	if (!getSoundQueue()->isBuffered(filename) && hotspot.param3 & id) {
		getSound()->playSound(kEntityPlayer, filename, kFlagDefault);

		getSavePoints()->call(kEntityPlayer, kEntityTrain, kAction203863200, filename.c_str());
		getSavePoints()->push(kEntityPlayer, kEntityTrain, kAction222746496, hotspot.param2);
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// // Action 43
IMPLEMENT_ACTION(switchChapter)
	// Nothing to do here as an hotspot action
	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Action 44
IMPLEMENT_ACTION(44)
	switch (hotspot.param1) {
	default:
		break;

	case 1:
		getSavePoints()->push(kEntityPlayer, kEntityRebecca, kAction205034665);
		break;

	case 2:
		getSavePoints()->push(kEntityPlayer, kEntityChapters, kAction225358684);
		break;
	}

	return kSceneInvalid;
}

//////////////////////////////////////////////////////////////////////////
// Helper functions
//////////////////////////////////////////////////////////////////////////
void Action::pickGreenJacket(bool process) const {
	getProgress().jacket = kJacketGreen;
	getInventory()->addItem(kItemMatchBox);

	getObjects()->update(kObjectOutsideTylerCompartment, kEntityPlayer, kObjectLocation2, kCursorKeepValue, kCursorKeepValue);
	playAnimation(kEventPickGreenJacket);

	getInventory()->setPortrait(kPortraitGreen);

	if (process)
		getScenes()->processScene();
}

void Action::pickScarf(bool process) const {
	playAnimation(getProgress().jacket == kJacketGreen ? kEventPickScarfGreen : kEventPickScarfOriginal);

	if (process)
		getScenes()->processScene();
}

void Action::pickCorpse(ObjectLocation bedPosition, bool process) const {

	if (getProgress().jacket == kJacketOriginal)
		getProgress().jacket = kJacketBlood;

	switch(getInventory()->get(kItemCorpse)->location) {
	// No way to pick the corpse
	default:
		break;

	// Floor
	case kObjectLocation1:
		// Bed is fully opened, move corpse directly there
		if (bedPosition == 4) {
			playAnimation(kEventCorpsePickFloorOpenedBedOriginal);

			getInventory()->get(kItemCorpse)->location = kObjectLocation5;
			break;
		}

		playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpsePickFloorGreen : kEventCorpsePickFloorOriginal);
		break;

	// Bed
	case kObjectLocation2:
		playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpsePickBedGreen : kEventCorpsePickBedOriginal);
		break;
	}

	if (process)
		getScenes()->processScene();

	// Add corpse to inventory
	if (bedPosition != 4) { // bed is not fully opened
		getInventory()->addItem(kItemCorpse);
		getInventory()->selectItem(kItemCorpse);
		_engine->getCursor()->setStyle(kCursorCorpse);
	}
}

void Action::dropCorpse(bool process) const {
	switch(getInventory()->get(kItemCorpse)->location) {
	default:
		break;

	case kObjectLocation1:	// Floor
		playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpseDropFloorGreen : kEventCorpseDropFloorOriginal);
		break;

	case kObjectLocation2:	// Bed
		playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpseDropBedGreen : kEventCorpseDropBedOriginal);
		break;

	case kObjectLocation4: // Window
		// Say goodbye to an old friend
		getInventory()->get(kItemCorpse)->location = kObjectLocationNone;
		getProgress().eventCorpseThrown = true;

		if (getState()->time <= kTime1138500) {
			playAnimation(getProgress().jacket == kJacketGreen ? kEventCorpseDropWindowGreen : kEventCorpseDropWindowOriginal);

			getProgress().field_24 = true;
		} else {
			playAnimation(kEventCorpseDropBridge);
		}

		getProgress().eventCorpseMovedFromFloor = true;
		break;
	}

	if (process)
		getScenes()->processScene();
}

bool Action::handleOtherCompartment(ObjectIndex object, bool doPlaySound, bool doLoadScene) const {
#define ENTITY_PARAMS(entity, index, id) \
	((EntityData::EntityParametersIIII*)getEntities()->get(entity)->getParamData()->getParameters(8, index))->param##id

	// Only handle compartments
	if (getEntityData(kEntityPlayer)->location != kLocationOutsideCompartment
	|| ((object < kObjectCompartment2 || object > kObjectCompartment8) && (object < kObjectCompartmentA || object > kObjectCompartmentH)))
		return false;

	//////////////////////////////////////////////////////////////////////////
	// Gendarmes
	if (getEntityData(kEntityPlayer)->car == getEntityData(kEntityGendarmes)->car
	&& getEntityData(kEntityGendarmes)->location == kLocationOutsideCompartment
	&& !getEntities()->compare(kEntityPlayer, kEntityGendarmes)) {
		if (doPlaySound)
			playCompartmentSoundEvents(object);

		if (doLoadScene)
			getScenes()->loadSceneFromObject(object);

		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	// Mertens
	if (getEntityData(kEntityPlayer)->car == kCarGreenSleeping
	 && getEntityData(kEntityMertens)->car == kCarGreenSleeping
	 && !getEntityData(kEntityMertens)->location
	 && !ENTITY_PARAMS(kEntityMertens, 0, 1)) {

		if (!getEntities()->compare(kEntityPlayer, kEntityMertens)) {

			if (getEntityData(kEntityMertens)->entityPosition < kPosition_2740
			 && getEntityData(kEntityMertens)->entityPosition > kPosition_850
			 && (getEntityData(kEntityCoudert)->car != kCarGreenSleeping || getEntityData(kEntityCoudert)->entityPosition > kPosition_2740)
			 && (getEntityData(kEntityVerges)->car != kCarGreenSleeping || getEntityData(kEntityVerges)->entityPosition > kPosition_2740)) {
				if (doPlaySound)
					playCompartmentSoundEvents(object);

				if (!getSoundQueue()->isBuffered(kEntityMertens))
					getSound()->playWarningCompartment(kEntityMertens, object);

				getSavePoints()->push(kEntityPlayer, kEntityMertens, kAction305159806);

				if (doLoadScene)
					getScenes()->loadSceneFromObject(object);

				return true;
			}

			if (getEntityData(kEntityMertens)->direction == kDirectionUp
			 && getEntityData(kEntityMertens)->entityPosition < getEntityData(kEntityPlayer)->entityPosition) {
				if (doPlaySound)
					playCompartmentSoundEvents(object);

				if (!getSoundQueue()->isBuffered(kEntityMertens))
					getSound()->playSound(kEntityMertens, (rnd(2)) ? "JAC1000" : "JAC1000A");

				if (doLoadScene)
					getScenes()->loadSceneFromObject(object);
			}

			if (getEntityData(kEntityMertens)->direction == kDirectionDown
			 && getEntityData(kEntityMertens)->entityPosition > getEntityData(kEntityPlayer)->entityPosition) {
				if (doPlaySound)
					playCompartmentSoundEvents(object);

				if (!getSoundQueue()->isBuffered(kEntityMertens))
					getSound()->playSound(kEntityMertens, (rnd(2)) ? "JAC1000" : "JAC1000A");

				if (doLoadScene)
					getScenes()->loadSceneFromObject(object, true);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// Coudert
	if (getEntityData(kEntityPlayer)->car != kCarRedSleeping
	 || !getEntityData(kEntityCoudert)->car
	 || getEntityData(kEntityCoudert)->location != kLocationOutsideCompartment
	 || ENTITY_PARAMS(kEntityCoudert, 0, 1))
	 return false;

	if (!getEntities()->compare(kEntityPlayer, kEntityCoudert)) {

		if (getEntityData(kEntityCoudert)->entityPosition < kPosition_2740
		 && getEntityData(kEntityCoudert)->entityPosition > kPosition_850
		 && (getEntityData(kEntityMertens)->car != kCarRedSleeping || getEntityData(kEntityMertens)->entityPosition > kPosition_2740)
		 && (getEntityData(kEntityVerges)->car != kCarRedSleeping || getEntityData(kEntityVerges)->entityPosition > kPosition_2740)
		 && (getEntityData(kEntityMmeBoutarel)->car != kCarRedSleeping || getEntityData(kEntityMmeBoutarel)->entityPosition > kPosition_2740)) {
			if (doPlaySound)
				playCompartmentSoundEvents(object);

			if (!getSoundQueue()->isBuffered(kEntityCoudert))
				getSound()->playWarningCompartment(kEntityCoudert, object);

			getSavePoints()->push(kEntityPlayer, kEntityCoudert, kAction305159806);

			if (doLoadScene)
				getScenes()->loadSceneFromObject(object);

			return true;
		}

		// Direction = Up
		if (getEntityData(kEntityCoudert)->direction == kDirectionUp
		 && getEntityData(kEntityCoudert)->entityPosition < getEntityData(kEntityPlayer)->entityPosition) {
			if (doPlaySound)
				playCompartmentSoundEvents(object);

			if (!getSoundQueue()->isBuffered(kEntityCoudert))
				getSound()->playSound(kEntityCoudert, (rnd(2)) ? "JAC1000" : "JAC1000A");

			if (doLoadScene)
				getScenes()->loadSceneFromObject(object);

			return true;
		}

		// Direction = down
		if (getEntityData(kEntityCoudert)->direction == kDirectionDown
		 && getEntityData(kEntityCoudert)->entityPosition > getEntityData(kEntityPlayer)->entityPosition) {
			if (doPlaySound)
				playCompartmentSoundEvents(object);

			if (!getSoundQueue()->isBuffered(kEntityCoudert))
				getSound()->playSound(kEntityCoudert, (rnd(2)) ? "JAC1000" : "JAC1000A");

			if (doLoadScene)
				getScenes()->loadSceneFromObject(object, true);
		}
	}

	return false;
}

void Action::playCompartmentSoundEvents(ObjectIndex object) const {
	if (getObjects()->get(object).location == kObjectLocation1 || getObjects()->get(object).location == kObjectLocation3 || getEntities()->checkFields2(object)) {
		getSound()->playSoundEvent(kEntityPlayer, 13);
	} else {
		getSound()->playSoundEvent(kEntityPlayer, 14);
		getSound()->playSoundEvent(kEntityPlayer, 15, 3);
	}
}

//////////////////////////////////////////////////////////////////////////
// Cursors
//////////////////////////////////////////////////////////////////////////
CursorStyle Action::getCursor(const SceneHotspot &hotspot) const {
	// Simple cursor style
	if (hotspot.cursor != kCursorProcess)
		return (CursorStyle)hotspot.cursor;

	ObjectIndex object = (ObjectIndex)hotspot.param1;

	switch (hotspot.action) {
	default:
		return kCursorNormal;

	case SceneHotspot::kActionInventory:
		if (!getState()->sceneBackup2 && (getEvent(kEventKronosBringFirebird) || getProgress().isEggOpen))
			return kCursorNormal;
		else
			return kCursorBackward;

	case SceneHotspot::kActionKnockOnDoor:
		debugC(2, kLastExpressDebugScenes, "================================= DOOR %03d =================================", object);
		if (object >= kObjectMax)
			return kCursorNormal;
		else
			return (CursorStyle)getObjects()->get(object).cursor;

	case SceneHotspot::kAction12:
		debugC(2, kLastExpressDebugScenes, "================================= OBJECT %03d =================================", object);
		if (object >= kObjectMax)
			return kCursorNormal;

		if (getObjects()->get(object).entity)
			return (CursorStyle)getObjects()->get(object).cursor;
		else
			return kCursorNormal;

	case SceneHotspot::kActionPickItem:
		debugC(2, kLastExpressDebugScenes, "================================= ITEM %03d =================================", object);
		if (object >= kObjectCompartmentA)
			return kCursorNormal;

		if ((!getInventory()->getSelectedItem() || getInventory()->getSelectedEntry()->manualSelect)
		 && (object != kObject21 || getProgress().eventCorpseMovedFromFloor))
			return kCursorHand;
		else
			return kCursorNormal;

	case SceneHotspot::kActionDropItem:
		debugC(2, kLastExpressDebugScenes, "================================= ITEM %03d =================================", object);
		if (object >= kObjectCompartmentA)
			return kCursorNormal;

		if (getInventory()->getSelectedItem() != (InventoryItem)object)
			return kCursorNormal;

		if (object == kObject20 && hotspot.param2 == 4 && !getProgress().isTrainRunning)
			return kCursorNormal;

		if (object == kObjectHandleInsideBathroom  && hotspot.param2 == 1 && getProgress().field_5C)
			return kCursorNormal;

		return (CursorStyle)getInventory()->getSelectedEntry()->cursor;

	case SceneHotspot::kAction15:
		if (object >= kObjectMax)
			return kCursorNormal;

		if (getProgress().isEqual(object, hotspot.param2))
			return (CursorStyle)hotspot.param3;

		return kCursorNormal;

	case SceneHotspot::kActionEnterCompartment:
		if ((getInventory()->getSelectedItem() != kItemKey || getObjects()->get(kObjectCompartment1).location)
		&& (getObjects()->get(kObjectCompartment1).location != 1 || !getInventory()->hasItem(kItemKey)
		 ||	(getInventory()->getSelectedItem() != kItemFirebird && getInventory()->getSelectedItem() != kItemBriefcase)))
			goto LABEL_KEY;

		return (CursorStyle)getInventory()->get(kItemKey)->cursor; // TODO is that always the same as kCursorKey ?

	case SceneHotspot::kActionGetOutsideTrain:
		if (getProgress().jacket != kJacketGreen)
			return kCursorNormal;

		if ((getEvent(kEventCathLookOutsideWindowDay) || getEvent(kEventCathLookOutsideWindowNight) || getObjects()->get(kObjectCompartment1).location2 == kObjectLocation1)
			&& getProgress().isTrainRunning
			&& (object != kObjectOutsideAnnaCompartment || (getEntities()->isInsideCompartment(kEntityRebecca, kCarRedSleeping, kPosition_4840) && getObjects()->get(kObjectOutsideBetweenCompartments).location == 2))
			&& getInventory()->getSelectedItem() != kItemBriefcase && getInventory()->getSelectedItem() != kItemFirebird)
			return kCursorForward;

		return (getObjects()->get(kObjectCompartment1).location2 < kObjectLocation2) ? kCursorNormal : kCursorMagnifier;

	case SceneHotspot::kActionSlip:
		return (getProgress().field_C8 < 1) ? kCursorNormal : kCursorLeft;

	case SceneHotspot::kActionClimbUpTrain:
		if (getProgress().isTrainRunning
			&& (getProgress().chapter == kChapter2 || getProgress().chapter == kChapter3 || getProgress().chapter == kChapter5)
			&& getInventory()->getSelectedItem() != kItemFirebird
			&& getInventory()->getSelectedItem() != kItemBriefcase)
			return kCursorUp;

		return kCursorNormal;

	case SceneHotspot::kActionJumpUpDownTrain:
		if (object != kObjectCompartment1)
			return kCursorNormal;

		return (getObjects()->get(kObjectCeiling).location < kObjectLocation1) ? kCursorHand : kCursorNormal;

	case SceneHotspot::kActionUnbound:
		if (hotspot.param2 != 2)
			return kCursorNormal;

		if (getEvent(kEventCathBurnRope) || !getEvent(kEventCathStruggleWithBonds2))
			return kCursorNormal;

		return kCursorHand;

	case SceneHotspot::kActionCatchBeetle:
		if (!getBeetle()->isLoaded())
			return kCursorNormal;

		if (!getBeetle()->isCatchable())
			return kCursorNormal;

		if (getInventory()->getSelectedItem() == kItemMatchBox && getInventory()->hasItem(kItemMatch))
			return (CursorStyle)getInventory()->get(kItemMatchBox)->cursor;

		return kCursorHandPointer;

	case SceneHotspot::KActionUseWhistle:
		if (object != kObjectCompartment3)
			return kCursorNormal;

		if (getInventory()->getSelectedItem() == kItemWhistle)
			return kCursorWhistle;
		else
			return kCursorNormal;

	case SceneHotspot::kActionOpenBed:
		if (getProgress().chapter < kChapter2)
			return kCursorHand;

		return kCursorNormal;

	case SceneHotspot::kActionDialog:
		if (getSound()->getDialogName((EntityIndex)object))
			return kCursorHandPointer;

		return kCursorNormal;

	case SceneHotspot::kActionBed:
		if (getProgress().field_18 == 2 && !getProgress().field_E4
			&& (getState()->time > kTimeBedTime
			|| (getProgress().eventMetAugust && getProgress().field_CC
			&& (!getProgress().field_24 || getProgress().field_3C))))
			return kCursorSleep;

		return kCursorNormal;

LABEL_KEY:
	// Handle compartment action
	case SceneHotspot::kActionCompartment:
	case SceneHotspot::kActionExitCompartment:
		debugC(2, kLastExpressDebugScenes, "================================= DOOR %03d =================================", object);
		if (object >= kObjectMax)
			return kCursorNormal;

		if (getInventory()->getSelectedItem() != kItemKey
		|| getObjects()->get(object).entity
		|| getObjects()->get(object).location != 1
		|| !getObjects()->get(object).cursor2
		|| getEntities()->isInsideCompartments(kEntityPlayer)
		|| getEntities()->checkFields2(object))
			return (CursorStyle)getObjects()->get(object).cursor2;
		else
			return (CursorStyle)getInventory()->get(kItemKey)->cursor;
	}
}

//////////////////////////////////////////////////////////////////////////
// Animation
//////////////////////////////////////////////////////////////////////////

// Play an animation and add delta time to global game time
void Action::playAnimation(EventIndex index, bool debugMode) const {
	if (index >= _animationListSize)
		error("[Action::playAnimation] Invalid event index (value=%i, max=%i)", index, _animationListSize);

	// In debug mode, just show the animation
	if (debugMode) {
		Animation animation;
		if (animation.load(getArchive(Common::String(_animationList[index].filename) + ".nis")))
			animation.play();
		return;
	}

	getFlags()->flag_3 = true;

	// Hide cursor
	_engine->getCursor()->show(false);

	// Show inventory & hourglass
	getInventory()->show();
	getInventory()->showHourGlass();

	if (!getFlags()->mouseRightClick) {

		if (getGlobalTimer()) {
			if (getSoundQueue()->isBuffered("TIMER")) {
				getSoundQueue()->processEntry("TIMER");
				setGlobalTimer(105);
			}
		}

		bool processSound = false;
		if (index >= kEventCorpseDropFloorOriginal
		 || index == kEventCathWakingUp
		 || index == kEventConcertCough
		 || index == kEventConcertSit
		 || index == kEventConcertLeaveWithBriefcase)
			processSound = true;

		Animation animation;
		if (animation.load(getArchive(Common::String(_animationList[index].filename) + ".nis") , processSound ? Animation::kFlagDefault : Animation::kFlagProcess))
			animation.play();

		if (getSoundQueue()->isBuffered("TIMER"))
			getSoundQueue()->removeFromQueue("TIMER");
	}

	// Show cursor
	_engine->getCursor()->show(true);

	getEvent(index) = 1;

	// Adjust game time
	getState()->timeTicks += _animationList[index].time;
	getState()->time = (TimeValue)(getState()->time + (TimeValue)(_animationList[index].time * getState()->timeDelta));
}

} // End of namespace LastExpress
