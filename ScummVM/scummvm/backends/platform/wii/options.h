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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef _WII_OPTIONS_H_
#define _WII_OPTIONS_H_

#include "common/str.h"
#include "gui/dialog.h"
#include "gui/widgets/tab.h"
#include "gui/widgets/edittext.h"

using namespace GUI;

class WiiOptionsDialog: public GUI::Dialog {
	typedef Common::String String;

public:
	WiiOptionsDialog(bool doubleStrike);
	virtual ~WiiOptionsDialog();

protected:
	virtual void handleTickle();
	virtual void handleCommand(CommandSender *sender, uint32 cmd, uint32 data);

private:
	bool _doubleStrike;
	String _strUnderscanX;
	String _strUnderscanY;

	TabWidget *_tab;

	int _tabVideo;
	SliderWidget *_sliderUnderscanX;
	SliderWidget *_sliderUnderscanY;

	int _tabInput;
	SliderWidget *_sliderPadSensitivity;
	SliderWidget *_sliderPadAcceleration;

#ifdef USE_WII_DI
	int _tabDVD;
	StaticTextWidget *_textDVDStatus;
#endif

#ifdef USE_WII_SMB
	int _tabSMB;
	StaticTextWidget *_textNetworkStatus;
	StaticTextWidget *_textSMBStatus;
	EditTextWidget *_editSMBServer;
	EditTextWidget *_editSMBShare;
	EditTextWidget *_editSMBUsername;
	EditTextWidget *_editSMBPassword;
#endif

	void revert();
	void load();
	void save();
};

#endif
