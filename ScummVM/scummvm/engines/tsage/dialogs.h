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

#ifndef TSAGE_DIALOGS_H
#define TSAGE_DIALOGS_H

#include "gui/options.h"
#include "tsage/events.h"
#include "tsage/graphics.h"
#include "common/list.h"
#include "common/rect.h"
#include "common/system.h"

namespace TsAGE {

class MessageDialog : public GfxDialog {
public:
	GfxButton _btn1, _btn2;
	GfxDialog _dialog;
	GfxMessage _msg;
public:
	MessageDialog(const Common::String &message, const Common::String &btn1Message, const Common::String &btn2Message = Common::String());

	static int show(const Common::String &message, const Common::String &btn1Message, const Common::String &btn2Message = Common::String());
	static int show2(const Common::String &message, const Common::String &btn1Message, const Common::String &btn2Message = Common::String());
};

class ConfigDialog : public GUI::OptionsDialog {
public:
	ConfigDialog();
};

/*--------------------------------------------------------------------------*/

class ModalDialog : public GfxDialog {
protected:
	void drawFrame();
public:
	virtual void draw();
};

/*--------------------------------------------------------------------------*/

class SoundDialog {
public:
	static void execute();
};

} // End of namespace TsAGE

#endif
