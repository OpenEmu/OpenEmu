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

#ifndef SCI_GRAPHICS_PORTS_H
#define SCI_GRAPHICS_PORTS_H

#include "common/serializer.h"
#include "common/list.h"
#include "common/array.h"

namespace Sci {

class GfxPaint16;
class GfxScreen;
class GfxText16;
struct WorklistManager;

// window styles
enum {
	SCI_WINDOWMGR_STYLE_TRANSPARENT = (1 << 0),
	SCI_WINDOWMGR_STYLE_NOFRAME     = (1 << 1),
	SCI_WINDOWMGR_STYLE_TITLE       = (1 << 2),
	SCI_WINDOWMGR_STYLE_TOPMOST     = (1 << 3),
	SCI_WINDOWMGR_STYLE_USER        = (1 << 7)
};

typedef Common::List<Port *> PortList;
typedef Common::Array<Port *> PortArray;

/**
 * Ports class, includes all port managment for SCI0->SCI1.1 games. Ports are some sort of windows in SCI
 *  this class also handles adjusting coordinates to a specific port
 */
class GfxPorts : public Common::Serializable {
public:
	GfxPorts(SegManager *segMan, GfxScreen *screen);
	~GfxPorts();

	void init(bool usesOldGfxFunctions, GfxPaint16 *paint16, GfxText16 *text16);
	void reset();

	void kernelSetActive(uint16 portId);
	Common::Rect kernelGetPicWindow(int16 &picTop, int16 &picLeft);
	void kernelSetPicWindow(Common::Rect rect, int16 picTop, int16 picLeft, bool initPriorityBandsFlag);
	reg_t kernelGetActive();
	reg_t kernelNewWindow(Common::Rect dims, Common::Rect restoreRect, uint16 style, int16 priority, int16 colorPen, int16 colorBack, const char *title);
	void kernelDisposeWindow(uint16 windowId, bool reanimate);

	int16 isFrontWindow(Window *wnd);
	void beginUpdate(Window *wnd);
	void endUpdate(Window *wnd);
	Window *addWindow(const Common::Rect &dims, const Common::Rect *restoreRect, const char *title, uint16 style, int16 priority, bool draw);
	void drawWindow(Window *wnd);
	void removeWindow(Window *pWnd, bool reanimate);
	void freeWindow(Window *pWnd);
	void updateWindow(Window *wnd);

	Port *getPortById(uint16 id);

	Port *setPort(Port *newPort);
	Port *getPort();
	void setOrigin(int16 left, int16 top);
	void moveTo(int16 left, int16 top);
	void move(int16 left, int16 top);
	void openPort(Port *port);
	void penColor(int16 color);
	void backColor(int16 color);
	void penMode(int16 mode);
	void textGreyedOutput(bool state);
	int16 getPointSize();

	void offsetRect(Common::Rect &r);
	void offsetLine(Common::Point &start, Common::Point &end);
	void clipLine(Common::Point &start, Common::Point &end);

	void priorityBandsInit(int16 bandCount, int16 top, int16 bottom);
	void priorityBandsInit(byte *data);
	void priorityBandsInitSci11(byte *data);

	void kernelInitPriorityBands();
	void kernelGraphAdjustPriority(int top, int bottom);
	byte kernelCoordinateToPriority(int16 y);
	int16 kernelPriorityToCoordinate(byte priority);
	void processEngineHunkList(WorklistManager &wm);
	void printWindowList(Console *con);

	Port *_wmgrPort;
	Window *_picWind;

	Port *_menuPort;
	Common::Rect _menuBarRect;
	Common::Rect _menuRect;
	Common::Rect _menuLine;
	Port *_curPort;

	virtual void saveLoadWithSerializer(Common::Serializer &ser);

private:
	/** The list of open 'windows' (and ports), in visual order. */
	PortList _windowList;

	/** The list of all open 'windows' (and ports), ordered by their id. */
	PortArray _windowsById;

	SegManager *_segMan;
	GfxPaint16 *_paint16;
	GfxScreen *_screen;
	GfxText16 *_text16;

	bool _usesOldGfxFunctions;

	uint16 _styleUser;

	// counts windows that got disposed but are not freed yet
	uint16 _freeCounter;

	Common::Rect _bounds;

	// Priority Bands related variables
	int16 _priorityTop, _priorityBottom, _priorityBandCount;
	byte _priorityBands[200];
};

} // End of namespace Sci

#endif
