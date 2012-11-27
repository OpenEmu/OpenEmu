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

#ifndef TSAGE_RINGWORLD2_DIALOGS_H
#define TSAGE_RINGWORLD2_DIALOGS_H

#include "gui/options.h"
#include "tsage/dialogs.h"
#include "tsage/events.h"
#include "tsage/graphics.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/system.h"

namespace TsAGE {

namespace Ringworld2 {

using namespace TsAGE;

class RightClickDialog : public GfxDialog {
private:
	GfxSurface _surface;
	Visage _btnImages;
	Common::Point _btnList[6];

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

class CharacterDialog: public GfxDialog {
private:
	GfxMessage _msgTitle;
	GfxButton _btnQuinn, _btnMiranda, _btnSeeker;
	GfxButton _btnCancel;
public:
	CharacterDialog();
	virtual ~CharacterDialog() {}

	static void show();
};

class HelpDialog: public GfxDialog {
private:
	GfxMessage _msgTitle, _msgVersion;
	GfxButton _btnList[7];
	GfxMessage _btnDescription[7];
	GfxButton _btnResume;
public:
	HelpDialog();
	virtual ~HelpDialog() {}

	static void show();
};

} // End of namespace Ringworld2

} // End of namespace TsAGE

#endif
