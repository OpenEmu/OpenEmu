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

#ifndef TSAGE_BLUEFORCE_DIALOGS_H
#define TSAGE_BLUEFORCE_DIALOGS_H

#include "gui/options.h"
#include "tsage/dialogs.h"
#include "tsage/events.h"
#include "tsage/graphics.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/system.h"

namespace TsAGE {

namespace BlueForce {

using namespace TsAGE;

class RightClickDialog : public GfxDialog {
private:
	GfxSurface _surface;
	Visage _btnImages;
	Rect _rectList1[5];
	Rect _rectList2[5];
	Rect _rectList3[5];
	Rect _rectList4[5];

	int _highlightedAction;
	int _selectedAction;
public:
	RightClickDialog();
	~RightClickDialog();

	virtual void draw();
	virtual bool process(Event &event);
	void execute();
};

class AmmoBeltDialog : public GfxDialog {
private:
	GfxSurface _surface;
	Visage _cursorImages;
	Rect _dialogRect, _loadedRect, _gunRect, _clip1Rect, _clip2Rect;
	CursorType _cursorNum;
	int _inDialog;
	bool _closeFlag;
public:
	AmmoBeltDialog();
	~AmmoBeltDialog();

	virtual void draw();
	virtual bool process(Event &event);
	void execute();
};

class RadioConvDialog : public GfxDialog {
private:
	GfxButton _buttons[8];
public:
	RadioConvDialog();
	virtual ~RadioConvDialog();
	int execute();

	static int show();
};

class OptionsDialog: public GfxDialog {
private:
	GfxButton _btnSave, _btnRestore, _btnRestart;
	GfxButton _btnQuit, _btnResume;
	GfxButton _btnSound;
	GfxMessage _gfxMessage;
public:
	OptionsDialog();
	virtual ~OptionsDialog() {}

	static void show();
};


} // End of namespace BlueForce

} // End of namespace TsAGE

#endif
