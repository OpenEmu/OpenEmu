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

#include "cruise/cruise.h"
#include "cruise/cruise_main.h"
#include "cruise/staticres.h"

#include "engines/metaengine.h"
#include "gui/saveload.h"
#include "common/system.h"
#include "common/translation.h"

namespace Cruise {

extern int currentMouseButton;

menuStruct *menuTable[8];

menuStruct *createMenu(int X, int Y, const char *menuName) {
	menuStruct *entry;

	entry = (menuStruct *) MemAlloc(sizeof(menuStruct));
	ASSERT(entry);

	entry->x = X - 160 / 2;
	entry->y = Y;
	entry->stringPtr = menuName;
	entry->numElements = 0;
	entry->ptrNextElement = NULL;
	entry->gfx = renderText(160, menuName);

	return entry;
}

// TODO: rewrite to remove the goto
void addSelectableMenuEntry(int ovlIdx, int headerIdx, menuStruct *pMenu, int param2, int color, const char *menuText) {
	menuElementStruct *di;
	menuElementStruct *var_6;
	menuElementStruct *pNewElement;
	menuElementSubStruct *pSubStruct;
	menuElementSubStruct *pSubStructCurrent;

	if (pMenu->numElements <= 48) {
		var_6 = pMenu->ptrNextElement;

		if (var_6) {
			do {
				di = var_6;
				if (param2) {
					if (!strcmp(var_6->string, menuText)) {
						pNewElement = var_6;
						pSubStruct = (menuElementSubStruct *)allocAndZero(sizeof(menuElementSubStruct));
						ASSERT(pSubStruct);

						pSubStruct->pNext = NULL;
						pSubStruct->ovlIdx = ovlIdx;
						pSubStruct->header = headerIdx;

						pSubStructCurrent = pNewElement->ptrSub;

						if (!pSubStructCurrent) {
							pNewElement->ptrSub = pSubStruct;
							return;
						}

						if (pSubStructCurrent->pNext) {
							do {
								pSubStructCurrent = pSubStructCurrent->pNext;
							} while (pSubStructCurrent->pNext);
						}

						pSubStructCurrent->pNext = pSubStruct;
						return;
					}
				}
				var_6 = var_6->next;
			} while (var_6);

			var_6 = di;
		}

		pNewElement = (menuElementStruct *)allocAndZero(sizeof(menuElementStruct));
		ASSERT(pNewElement);
		pSubStruct = (menuElementSubStruct *)allocAndZero(sizeof(menuElementSubStruct));
		ASSERT(pSubStruct);

		pNewElement->string = menuText;
		pNewElement->next = NULL;
		pNewElement->selected = false;
		pNewElement->color = color;
		pNewElement->gfx = renderText(160, menuText);

		if (var_6 == NULL) {
			pMenu->ptrNextElement = pNewElement;
		} else {
			var_6->next = pNewElement;
		}

		pNewElement->ptrSub = pSubStruct;

		pSubStruct->pNext = NULL;
		pSubStruct->ovlIdx = ovlIdx;
		pSubStruct->header = headerIdx;

		pMenu->numElements++;
	}
}

void updateMenuMouse(int mouseX, int mouseY, menuStruct *pMenu) {
	if (pMenu) {
		if (pMenu->gfx) {
			int height = pMenu->gfx->height;	// rustine
			int var_2 = 0;
			menuElementStruct *pCurrentEntry = pMenu->ptrNextElement;

			while (pCurrentEntry) {
				pCurrentEntry->selected = false;

				if (var_2 == 0) {
					if ((mouseX > pCurrentEntry->x) && ((pCurrentEntry->x + 160) >= mouseX)) {
						if ((mouseY > pCurrentEntry->y) && ((pCurrentEntry->y + height) >= mouseY)) {
							var_2 = 1;
							pCurrentEntry->selected = true;
						}
					}
				}

				pCurrentEntry = pCurrentEntry->next;
			}
		}
	}
}

bool manageEvents();

int processMenu(menuStruct *pMenu) {
	int16 mouseX;
	int16 mouseY;
	int16 mouseButton;
	int di;
	int si;
	currentActiveMenu = 0;

	mainDraw(1);
	flipScreen();

	di = 0;
	si = 0;

	do {
		getMouseStatus(&main10, &mouseX, &mouseButton, &mouseY);

		updateMenuMouse(mouseX, mouseY, pMenu);

		if (mouseButton) {
			if (di) {
				si = 1;
			}
		} else {
			di = 1;
		}

		mainDraw(1);
		flipScreen();

		manageEvents();
		g_system->delayMillis(10);

//    readKeyboard();
	} while (!si);

	currentActiveMenu = -1;

	mainDraw(1);
	flipScreen();

	if (mouseButton & 1) {
		menuElementSubStruct* pSelectedEntry = getSelectedEntryInMenu(pMenu);

		if (pSelectedEntry) {
			return pSelectedEntry->header;
		} else {
			return -1;
		}
	}

	return -1;
}

static void handleSaveLoad(bool saveFlag) {
	GUI::SaveLoadChooser *dialog;
	if (saveFlag)
		dialog = new GUI::SaveLoadChooser(_("Save game:"), _("Save"), true);
	else
		dialog = new GUI::SaveLoadChooser(_("Load game:"), _("Load"), false);

	int slot = dialog->runModalWithCurrentTarget();

	if (slot >= 0) {
		if (!saveFlag)
			_vm->loadGameState(slot);
		else {
			Common::String result(dialog->getResultString());
			if (result.empty()) {
				// If the user was lazy and entered no save name, come up with a default name.
				result = Common::String::format("Save %d", slot + 1);
			}

			_vm->saveGameState(slot, result);
		}
	}

	delete dialog;
}

int playerMenu(int menuX, int menuY) {
	int retourMenu;
	//int restartGame = 0;

	if (playerMenuEnabled && displayOn) {
		if (remdo) {
			_vm->sound().stopMusic();
			freeStuff2();
		}
		/*
		    if (currentMenu) {
		      freeMenu(currentMenu);
		      currentMenu = 0;
		      selectDown = 0;
		      menuDown = 0;
		      main9 = -1;
		    }

		    if (inventoryMenu) {
		      freeMenu(inventoryMenu);
		      inventoryMenu = 0;
		      selectDown = 0;
		      menuDown = 0;
		      main9 = -1;
		    }*/

		/*    if (mouseVar2) {
		      free3(mouseVar2);
		    } */

		/*    mouseVar2 = 0;
		    linkedRelation = 0; */
		freeDisk();

		menuTable[0] = createMenu(menuX, menuY, _vm->langString(ID_PLAYER_MENU));
		ASSERT(menuTable[0]);

		//addSelectableMenuEntry(0, 3, menuTable[0], 1, -1, "Save game disk");
		if (userEnabled) {
			addSelectableMenuEntry(0, 4, menuTable[0], 1, -1, _vm->langString(ID_SAVE));
		}
		addSelectableMenuEntry(0, 5, menuTable[0], 1, -1, _vm->langString(ID_LOAD));
		addSelectableMenuEntry(0, 6, menuTable[0], 1, -1, _vm->langString(ID_RESTART));
		addSelectableMenuEntry(0, 7, menuTable[0], 1, -1, _vm->langString(ID_QUIT));

		retourMenu = processMenu(menuTable[0]);

		freeMenu(menuTable[0]);
		menuTable[0] = NULL;
		currentMouseButton = 0;

		switch (retourMenu) {
		case 3: // select save drive
			break;
		case 4: // save
		case 5: // load
			handleSaveLoad(retourMenu == 4);
			break;
		case 6: // restart
			_vm->sound().fadeOutMusic();
			Op_FadeOut();
			memset(globalScreen, 0, 320 * 200);
			initVars();
			_vm->initAllData();
			changeCursor(CURSOR_NORMAL);
			userEnabled = 0;
			break;
		case 7: // exit
			return 1;
		}
	}

	return 0;
}

void freeMenu(menuStruct *pMenu) {
	menuElementStruct *pElement = pMenu->ptrNextElement;

	while (pElement) {
		menuElementStruct *next;
		menuElementSubStruct *pSub = pElement->ptrSub;

		next = pElement->next;

		while (pSub) {
			menuElementSubStruct *nextSub;

			nextSub = pSub->pNext;

			MemFree(pSub);

			pSub = nextSub;
		}

		if (pElement->gfx) {
			freeGfx(pElement->gfx);
		}

		MemFree(pElement);

		pElement = next;
	}

	freeGfx(pMenu->gfx);
	MemFree(pMenu);
}

} // End of namespace Cruise
