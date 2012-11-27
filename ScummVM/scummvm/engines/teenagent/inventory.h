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
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef TEENAGENT_INVENTORY_H
#define TEENAGENT_INVENTORY_H

#include "teenagent/surface.h"
#include "teenagent/animation.h"
#include "common/events.h"
#include "common/array.h"
#include "teenagent/objects.h"

namespace TeenAgent {

struct InventoryObject;
class TeenAgentEngine;

// Maximum number of items found within game
const uint8 kNumInventoryItems = 92;

// Inventory Item Ids
enum {
	kInvItemNoItem = 0, // No item i.e. empty inventory slot
	kInvItemFeather = 1,
	kInvItemShotgun = 2,
	kInvItemToolboxFull = 3, // Contains Car Jack and Spanner
	kInvItemToolboxHalfEmpty = 4, // Contains Spanner
	kInvItemSpanner = 5,
	kInvItemComb = 6,
	kInvItemFan = 7,
	kInvItemBrokenPaddle = 8,
	kInvItemPaddle = 9, // Repaired - BrokenPaddle combined with Branch (with Glue)
	kInvItemFirstFlower = 10, // Smells nice
	kInvItemSecondFlower = 11, // Really beautiful
	kInvItemFeatherDusterClean = 12,
	kInvItemChainsaw = 13, // Unfueled
	kInvItemDrunkenChainsaw = 14, // Fueled with Whisky (Chainsaw combined with Whiskey)
	kInvItemBranch = 15,
	kInvItemWhisky = 16,
	kInvItemNeedle = 17,
	kInvItemWrapper = 18,
	kInvItemChocCandy = 19,
	kInvItemPotato = 20,
	kInvItemRakeBroken = 21,
	kInvItemHeartShapedCandy = 22,
	kInvItemWrappedCandy = 23, // HeartShapedCandy combined with Wrapper
	kInvItemRibbon = 24,
	kInvItemRakeFixed = 25, // Rake combined with Ribbon
	kInvItemNut = 26,
	kInvItemPlasticApple = 27,
	kInvItemCone = 28,
	kInvItemSuperGlue = 29,
	kInvItemConeAndNeedle = 30, // Cone combined with Needle
	kInvItemConeAndFeather = 31, // Cone combined with Feather
	kInvItemDart = 32, // Needle combined with ConeAndFeather or Feather combined with ConeAndNeedle
	kInvItemFeatherDusterDirty = 33,
	kInvItemPaintedPotato = 34, // Potato combined with Dirty Feather Duster (Soot)
	kInvItemCarJack = 35,
	kInvItemBone = 36,
	kInvItemShovelAct2 = 37,
	kInvItemRopeAct2 = 38,
	kInvItemMask = 39,
	kInvItemFins = 40,
	kInvItemDiveEquipment = 41, // Mask combined with Fins
	kInvItemAnchor = 42,
	kInvItemGrapplingHook = 43,
	kInvItemSickleBlunt = 44,
	kInvItemCheese = 45,
	kInvItemSickleSharp = 46,
	kInvItemHandkerchief = 47,
	kInvItemMouse = 48,
	kInvItemRock = 49,
	kInvItemNugget = 50,
	kInvItemBanknote = 51,
	kInvItemDictaphoneNoBatteries = 52,
	kInvItemPolaroidCamera = 53,
	kInvItemVideoTape = 54,
	kInvItemSheetOfPaper = 55,
	kInvItemCognac = 56,
	kInvItemRemoteControl = 57,
	kInvItemIceTongs = 58,
	kInvItemCork = 59,
	kInvItemWrappedCork = 60, // Cork combined with Sheet Of Paper
	kInvItemPhoto = 61,
	kInvItemChilliWithLabel = 62,
	kInvItemPastryRoller = 63,
	kInvItemFakeChilli = 64,
	kInvItemLabel = 65,
	kInvItemBatteries = 66,
	kInvItemDictaphoneWithBatteries = 67, // Dictaphone combined with Batteries
	kInvItemBurningPaper = 68,
	kInvItemMeat = 69,
	kInvItemPlasticBag = 70,
	kInvItemSocks = 71,
	kInvItemTimePills = 72,
	kInvItemHandle = 73,
	kInvItemChilliNoLabel = 74,
	kInvItemPass = 75,
	kInvItemBulb = 76,
	kInvItemJailKey = 77,
	kInvItemDelicatePlant = 78,
	kInvItemSwissArmyKnife = 79,
	kInvItemSpring = 80,
	kInvItemShovelAct1 = 81,
	kInvItemKaleidoscope = 82,
	kInvItemSoldierNews = 83,
	kInvItemGrenade = 84,
	kInvItemMug = 85, // Empty
	kInvItemMugOfMud = 86, // Full of mud
	kInvItemCrumbs = 87,
	kInvItemRopeAct1 = 88,
	kInvItemRopeAndGrenade = 89, // Rope combined with Grenade
	kInvItemMedicine = 90,
	kInvItemDruggedFood = 91, // Crumbs combined with Medicine
	kInvItemBird = 92
};

// Maximum number of inventory items held by Ego (Mark)
const uint8 kInventorySize = 24;

class Inventory {
public:
	Inventory(TeenAgentEngine *vm);
	~Inventory();

	void render(Graphics::Surface *surface, int delta);

	void clear();
	void reload();
	void add(byte item);
	bool has(byte item) const;
	void remove(byte item);

	void activate(bool a) { _active = a; }
	bool active() const { return _active; }

	bool processEvent(const Common::Event &event);

	InventoryObject *selectedObject() { return _selectedObj; }
	void resetSelectedObject() { _selectedObj = NULL; }

private:
	TeenAgentEngine *_vm;
	Surface _background;
	byte *_items;
	uint _offset[kNumInventoryItems+1];

	Common::Array<InventoryObject> _objects;
	byte *_inventory;

	struct Item {
		Animation _animation;
		Surface _surface;
		Rect _rect;
		bool _hovered;

		Item() : _hovered(false) {}
		void free();
		void load(Inventory *inventory, uint itemId);
		void backgroundEffect(Graphics::Surface *s);
		void render(Inventory *inventory, uint itemId, Graphics::Surface *surface, int delta);
	};

	Item _graphics[kInventorySize];

	bool _active;
	Common::Point _mouse;

	bool tryObjectCallback(InventoryObject *obj);

	InventoryObject *_hoveredObj;
	InventoryObject *_selectedObj;
};

} // End of namespace TeenAgent

#endif
