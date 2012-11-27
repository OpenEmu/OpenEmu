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

#ifndef KYRA_GUI_H
#define KYRA_GUI_H

#include "kyra/kyra_v1.h"
#include "kyra/screen.h"

#include "common/ptr.h"
#include "common/array.h"
#include "common/func.h"

#include "graphics/surface.h"

namespace Kyra {

#define BUTTON_FUNCTOR(type, x, y) Button::Callback(new Common::Functor1Mem<Button *, int, type>(x, y))

struct Button {
	typedef Common::Functor1<Button *, int> CallbackFunctor;
	typedef Common::SharedPtr<CallbackFunctor> Callback;

	Button() : nextButton(0), index(0), keyCode(0), keyCode2(0), data0Val1(0), data1Val1(0), data2Val1(0), data3Val1(0), flags(0),
	    data0ShapePtr(0), data1ShapePtr(0), data2ShapePtr(0), data0Callback(), data1Callback(), data2Callback(),
	    dimTableIndex(0), x(0), y(0), width(0), height(0), data0Val2(0), data0Val3(0), data1Val2(0), data1Val3(0),
	    data2Val2(0), data2Val3(0), data3Val2(0), data3Val3(0), flags2(0), mouseWheel(0), buttonCallback(), extButtonDef(0), arg(0) {}

	Button *nextButton;
	uint16 index;

	uint16 keyCode;
	uint16 keyCode2;

	byte data0Val1;
	byte data1Val1;
	byte data2Val1;
	byte data3Val1;

	uint16 flags;

	const uint8 *data0ShapePtr;
	const uint8 *data1ShapePtr;
	const uint8 *data2ShapePtr;
	Callback data0Callback;
	Callback data1Callback;
	Callback data2Callback;

	uint16 dimTableIndex;

	int16 x, y;
	uint16 width, height;

	uint8 data0Val2;
	uint8 data0Val3;

	uint8 data1Val2;
	uint8 data1Val3;

	uint8 data2Val2;
	uint8 data2Val3;

	uint8 data3Val2;
	uint8 data3Val3;

	uint16 flags2;

	int8 mouseWheel;

	Callback buttonCallback;

	const void *extButtonDef;

	uint16 arg;
};

class Screen;
class TextDisplayer;

class GUI {
public:
	GUI(KyraEngine_v1 *vm);
	virtual ~GUI();

	// button specific
	virtual void processButton(Button *button) = 0;
	virtual int processButtonList(Button *buttonList, uint16 inputFlags, int8 mouseWheel) = 0;

	// utilities for thumbnail creation
	virtual void createScreenThumbnail(Graphics::Surface &dst) = 0;

	void notifyUpdateSaveSlotsList() { _saveSlotsListUpdateNeeded = true; }

protected:
	KyraEngine_v1 *_vm;
	Screen *_screen;

	// The engine expects a list of contiguous savegame indices.
	// Since ScummVM's savegame indices aren't, we re-index them.
	// The integers stored in _saveSlots are ScummVM savegame indices.
	Common::Array<int> _saveSlots;
	void updateSaveFileList(Common::String targetName, bool excludeQuickSaves = false);
	int getNextSavegameSlot();
	void updateSaveSlotsList(Common::String targetName, bool force = false);

	virtual void sortSaveSlots();

	uint32 _lastScreenUpdate;
	char **_savegameList;
	int _savegameListSize;
	bool _saveSlotsListUpdateNeeded;

	Common::KeyState _keyPressed;
};

} // End of namespace Kyra

#endif
