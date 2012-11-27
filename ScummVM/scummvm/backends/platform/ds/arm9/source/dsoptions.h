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

#ifndef _DSOPTIONS_H_
#define _DSOPTIONS_H_


#include "common/scummsys.h"
#include "common/str.h"
#include "common/config-manager.h"

#include "gui/object.h"
#include "gui/widget.h"
#include "gui/dialog.h"
#include "gui/widgets/tab.h"
#include "scumm/dialogs.h"

namespace DS {

class DSOptionsDialog : public GUI::Dialog {

public:
	DSOptionsDialog();
	~DSOptionsDialog();

protected:
	virtual void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	void updateConfigManager();

	GUI::TabWidget *_tab;

	GUI::StaticTextWidget *_sensitivityLabel;

	GUI::SliderWidget *_touchX;
	GUI::SliderWidget *_touchY;
	GUI::SliderWidget *_sensitivity;
	GUI::SliderWidget *_gammaCorrection;
	GUI::CheckboxWidget *_leftHandedCheckbox;
	GUI::CheckboxWidget *_unscaledCheckbox;
	GUI::CheckboxWidget *_100PercentCheckbox;
	GUI::CheckboxWidget *_150PercentCheckbox;
	GUI::CheckboxWidget *_200PercentCheckbox;
	GUI::CheckboxWidget *_indyFightCheckbox;
	GUI::CheckboxWidget *_highQualityAudioCheckbox;
	GUI::CheckboxWidget *_disablePowerOff;
	GUI::CheckboxWidget *_showCursorCheckbox;
	GUI::CheckboxWidget *_snapToBorderCheckbox;

	GUI::CheckboxWidget *_hardScaler;
	GUI::CheckboxWidget *_cpuScaler;

	GUI::CheckboxWidget *_touchPadStyle;
	GUI::CheckboxWidget *_screenTaps;

	bool	_radioButtonMode;

};

extern void showOptionsDialog();
extern void setOptions();

}	// End of namespace DS

#endif
