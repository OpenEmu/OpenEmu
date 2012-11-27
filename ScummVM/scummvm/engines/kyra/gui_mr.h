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

#ifndef KYRA_GUI_MR_H
#define KYRA_GUI_MR_H

#include "kyra/gui_v2.h"

namespace Kyra {

class KyraEngine_MR;
class Screen_MR;

class GUI_MR : public GUI_v2 {
friend class KyraEngine_MR;
public:
	GUI_MR(KyraEngine_MR *engine);

	void initStaticData();

	void flagButtonEnable(Button *button);
	void flagButtonDisable(Button *button);

	int redrawShadedButtonCallback(Button *button);
	int redrawButtonCallback(Button *button);

	int optionsButton(Button *button);

	void createScreenThumbnail(Graphics::Surface &dst);
private:
	const char *getMenuTitle(const Menu &menu);
	const char *getMenuItemTitle(const MenuItem &menuItem);
	const char *getMenuItemLabel(const MenuItem &menuItem);
	char *getTableString(int id);

	uint8 textFieldColor1() const { return 0xFF; }
	uint8 textFieldColor2() const { return 0xCF; }
	uint8 textFieldColor3() const { return 0xBA; }

	uint8 defaultColor1() const { return 0xF0; }
	uint8 defaultColor2() const { return 0xD0; }

	void resetState(int item);

	int quitGame(Button *button);
	int loadMenu(Button *button);
	int loadSecondChance(Button *button);

	int gameOptions(Button *button);
	void setupOptionsButtons();

	int audioOptions(Button *button);

	int sliderHandler(Button *caller);
	void drawSliderBar(int slider, const uint8 *shape);

	int changeLanguage(Button *caller);
	int toggleStudioSFX(Button *caller);
	int toggleSkipSupport(Button *caller);
	int toggleHeliumMode(Button *caller);

	KyraEngine_MR *_vm;
	Screen_MR *_screen;
};

} // End of namespace Kyra

#endif
