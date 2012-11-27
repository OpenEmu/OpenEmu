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

#ifndef MOHAWK_CSTIME_VIEW_H
#define MOHAWK_CSTIME_VIEW_H

#include "mohawk/cstime.h"
#include "mohawk/view.h"

namespace Mohawk {

class CSTimeModule : public Module {
public:
	CSTimeModule(MohawkEngine_CSTime *vm);

	// TODO: these don't really belong here
	void init() { }
	void shutdown() { }
	void update() { }

	void defaultMoveProc(Feature *feature);
	void defaultDrawProc(Feature *feature);
	bool defaultTimeProc(Feature *feature);

	void defaultPickupProc(NewFeature *feature, Common::Point pos, uint32 flags, Common::Rect *rect);
	void defaultDropProc(NewFeature *feature);
	void defaultDragMoveProc(NewFeature *feature);

	void cursorMoveProc(Feature *feature);
	void cursorDrawProc(Feature *feature);

	void dialogTextMoveProc(Feature *feature);
	void dialogTextDrawProc(Feature *feature);
	void rolloverTextMoveProc(Feature *feature);
	void rolloverTextDrawProc(Feature *feature);
	void bubbleTextMoveProc(Feature *feature);
	void bubbleTextDrawProc(Feature *feature);

protected:
	MohawkEngine_CSTime *_vm;
};

class CSTimeView : public View {
public:
	CSTimeView(MohawkEngine_CSTime *vm);

	uint32 getTime();
	void setupView();
	Feature *installViewFeature(uint16 scrbId, uint32 flags, Common::Point *pos);

	void installGroup(uint16 resourceId, uint size, uint count, bool regs, uint16 baseId);
	void removeGroup(uint16 resourceId);

	void loadBitmapCursors(uint16 baseId);
	void setBitmapCursor(uint16 id);
	uint16 getBitmapCursor() { return _bitmapCursorId; }

	void dragFeature(NewFeature *feature, Common::Point pos, uint mode, uint32 flags, Common::Rect *rect);

protected:
	MohawkEngine_CSTime *_timeVm;

	uint16 _bitmapCursorId;

	uint16 _SCRBGroupResources[14];
	void removeObjectsUsingBaseId(uint16 baseId);
	void freeShapesUsingResourceId(uint16 resourceId);
	void freeScriptsUsingResourceId(uint16 resourceId);
	void groupFreeScript(uint index);
	void groupAdjustView(uint index, uint count);

	void finishDraw();
};

} // End of namespace Mohawk

#endif
