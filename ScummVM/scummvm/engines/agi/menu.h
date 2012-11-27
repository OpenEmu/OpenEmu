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

#ifndef AGI_MENU_H
#define AGI_MENU_H

namespace Agi {

#define MENU_BG		0x0f	// White
#define MENU_DISABLED	0x07	// Grey

#define MENU_FG		0x00	// Black
#define MENU_LINE	0x00	// Black

struct AgiMenu;
struct AgiMenuOption;
typedef Common::List<AgiMenu *> MenuList;
typedef Common::List<AgiMenuOption *> MenuOptionList;

class GfxMgr;
class PictureMgr;

class Menu {
private:
	AgiEngine *_vm;
	GfxMgr *_gfx;
	PictureMgr *_picture;

public:
	Menu(AgiEngine *vm, GfxMgr *gfx, PictureMgr *picture);
	~Menu();

	void add(const char *s);
	void addItem(const char *s, int code);
	void submit();
	void setItem(int event, int state);
	bool keyhandler(int key);
	void enableAll();

private:
	MenuList _menubar;

	int _hCurMenu;
	int _vCurMenu;

	int _hIndex;
	int _vIndex;
	int _hCol;
	int _hMaxMenu;
	int _vMaxMenu[10];

	AgiMenu* getMenu(int i);
	AgiMenuOption *getMenuOption(int i, int j);
	void drawMenuBar();
	void drawMenuHilite(int curMenu);
	void drawMenuOption(int hMenu);
	void drawMenuOptionHilite(int hMenu, int vMenu);
	void newMenuSelected(int i);
	bool mouseOverText(int line, int col, char *s);

};

} // End of namespace Agi

#endif /* AGI_MENU_H */
