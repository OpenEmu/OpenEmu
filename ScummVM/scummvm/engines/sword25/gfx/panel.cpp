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

/*
 * This code is based on Broken Sword 2.5 engine
 *
 * Copyright (c) Malte Thiesen, Daniel Queteschiner and Michael Elsdoerfer
 *
 * Licensed under GNU GPL v2
 *
 */

#include "sword25/gfx/panel.h"

#include "sword25/kernel/inputpersistenceblock.h"
#include "sword25/kernel/outputpersistenceblock.h"
#include "sword25/gfx/graphicengine.h"
#include "sword25/gfx/image/image.h"

namespace Sword25 {

Panel::Panel(RenderObjectPtr<RenderObject> parentPtr, int width, int height, uint color) :
	RenderObject(parentPtr, RenderObject::TYPE_PANEL),
	_color(color) {
	_initSuccess = false;

	_width = width;
	_height = height;

	if (_width < 0) {
		error("Tried to initialize a panel with an invalid width (%d).", _width);
		return;
	}

	if (_height < 0) {
		error("Tried to initialize a panel with an invalid height (%d).", _height);
		return;
	}

	_initSuccess = true;
}

Panel::Panel(InputPersistenceBlock &reader, RenderObjectPtr<RenderObject> parentPtr, uint handle) :
	RenderObject(parentPtr, RenderObject::TYPE_PANEL, handle) {
	_initSuccess = unpersist(reader);
}

Panel::~Panel() {
}

bool Panel::doRender() {
	// Falls der Alphawert 0 ist, ist das Panel komplett durchsichtig und es muss nichts gezeichnet werden.
	if (_color >> 24 == 0)
		return true;

	GraphicEngine *gfxPtr = Kernel::getInstance()->getGfx();
	assert(gfxPtr);

	return gfxPtr->fill(&_bbox, _color);
}

bool Panel::persist(OutputPersistenceBlock &writer) {
	bool result = true;

	result &= RenderObject::persist(writer);
	writer.write(_color);

	result &= RenderObject::persistChildren(writer);

	return result;
}

bool Panel::unpersist(InputPersistenceBlock &reader) {
	bool result = true;

	result &= RenderObject::unpersist(reader);

	uint color;
	reader.read(color);
	setColor(color);

	result &= RenderObject::unpersistChildren(reader);

	return reader.isGood() && result;
}

} // End of namespace Sword25
