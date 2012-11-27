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

#include "sci/sci.h"
#include "sci/resource.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/graphics/cursor.h"
#include "sci/graphics/menu.h"
#include "sci/graphics/screen.h"

namespace Sci {

reg_t kAddMenu(EngineState *s, int argc, reg_t *argv) {
	Common::String title = s->_segMan->getString(argv[0]);
	Common::String content = s->_segMan->getString(argv[1]);

	g_sci->_gfxMenu->kernelAddEntry(title, content, argv[1]);
	return s->r_acc;
}


reg_t kSetMenu(EngineState *s, int argc, reg_t *argv) {
	uint16 menuId = argv[0].toUint16() >> 8;
	uint16 itemId = argv[0].toUint16() & 0xFF;
	uint16 attributeId;
	int argPos = 1;
	reg_t value;

	while (argPos < argc) {
		attributeId = argv[argPos].toUint16();
		// Happens in the fanmade game Cascade Quest when loading - bug #3038767
		value = (argPos + 1 < argc) ? argv[argPos + 1] : NULL_REG;
		g_sci->_gfxMenu->kernelSetAttribute(menuId, itemId, attributeId, value);
		argPos += 2;
	}
	return s->r_acc;
}

reg_t kGetMenu(EngineState *s, int argc, reg_t *argv) {
	uint16 menuId = argv[0].toUint16() >> 8;
	uint16 itemId = argv[0].toUint16() & 0xFF;
	uint16 attributeId = argv[1].toUint16();

	return g_sci->_gfxMenu->kernelGetAttribute(menuId, itemId, attributeId);
}


reg_t kDrawStatus(EngineState *s, int argc, reg_t *argv) {
	reg_t textReference = argv[0];
	Common::String text;
	int16 colorPen = (argc > 1) ? argv[1].toSint16() : 0;
	int16 colorBack = (argc > 2) ? argv[2].toSint16() : g_sci->_gfxScreen->getColorWhite();

	if (!textReference.isNull()) {
		// Sometimes this is called without giving text, if thats the case dont process it.
		text = s->_segMan->getString(textReference);

		if (text == "Replaying sound") {
			// Happens in the fanmade game Cascade Quest when loading - ignore it
			return s->r_acc;
		}

		g_sci->_gfxMenu->kernelDrawStatus(g_sci->strSplit(text.c_str(), NULL).c_str(), colorPen, colorBack);
	}
	return s->r_acc;
}

reg_t kDrawMenuBar(EngineState *s, int argc, reg_t *argv) {
	bool clear = argv[0].isNull() ? true : false;

	g_sci->_gfxMenu->kernelDrawMenuBar(clear);
	return s->r_acc;
}

reg_t kMenuSelect(EngineState *s, int argc, reg_t *argv) {
	reg_t eventObject = argv[0];
	bool pauseSound = argc <= 1 || !argv[1].isNull();

	return g_sci->_gfxMenu->kernelSelect(eventObject, pauseSound);
}

} // End of namespace Sci
