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

#include "common/system.h"

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/kernel.h"
#include "sci/engine/seg_manager.h"
#include "sci/graphics/cache.h"
#include "sci/graphics/compare.h"
#include "sci/graphics/controls32.h"
#include "sci/graphics/font.h"
#include "sci/graphics/screen.h"
#include "sci/graphics/text32.h"

namespace Sci {

GfxControls32::GfxControls32(SegManager *segMan, GfxCache *cache, GfxScreen *screen, GfxText32 *text)
	: _segMan(segMan), _cache(cache), _screen(screen), _text(text) {
}

GfxControls32::~GfxControls32() {
}

void GfxControls32::kernelTexteditChange(reg_t controlObject) {
	SciEvent curEvent;
	uint16 maxChars = 40;	//readSelectorValue(_segMan, controlObject, SELECTOR(max));	// TODO
	reg_t textReference = readSelector(_segMan, controlObject, SELECTOR(text));
	GfxFont *font = _cache->getFont(readSelectorValue(_segMan, controlObject, SELECTOR(font)));
	Common::String text;
	uint16 textSize;
	bool textChanged = false;
	bool textAddChar = false;
	Common::Rect rect;

	if (textReference.isNull())
		error("kEditControl called on object that doesnt have a text reference");
	text = _segMan->getString(textReference);

	// TODO: Finish this
	warning("kEditText ('%s')", text.c_str());
	return;

	uint16 cursorPos = 0;
	//uint16 oldCursorPos = cursorPos;
	bool captureEvents = true;
	EventManager* eventMan = g_sci->getEventManager();

	while (captureEvents) {
		curEvent = g_sci->getEventManager()->getSciEvent(SCI_EVENT_KEYBOARD | SCI_EVENT_PEEK);

		if (curEvent.type == SCI_EVENT_NONE) {
			eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
		} else {
			textSize = text.size();

			switch (curEvent.type) {
			case SCI_EVENT_MOUSE_PRESS:
				// TODO: Implement mouse support for cursor change
				break;
			case SCI_EVENT_KEYBOARD:
				switch (curEvent.data) {
				case SCI_KEY_BACKSPACE:
					if (cursorPos > 0) {
						cursorPos--; text.deleteChar(cursorPos);
						textChanged = true;
					}
					eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
					break;
				case SCI_KEY_DELETE:
					if (cursorPos < textSize) {
						text.deleteChar(cursorPos);
						textChanged = true;
					}
					eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
					break;
				case SCI_KEY_HOME: // HOME
					cursorPos = 0; textChanged = true;
					eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
					break;
				case SCI_KEY_END: // END
					cursorPos = textSize; textChanged = true;
					eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
					break;
				case SCI_KEY_LEFT: // LEFT
					if (cursorPos > 0) {
						cursorPos--; textChanged = true;
					}
					eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
					break;
				case SCI_KEY_RIGHT: // RIGHT
					if (cursorPos + 1 <= textSize) {
						cursorPos++; textChanged = true;
					}
					eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
					break;
				case 3:	// returned in SCI1 late and newer when Control - C is pressed
					if (curEvent.modifiers & SCI_KEYMOD_CTRL) {
						// Control-C erases the whole line
						cursorPos = 0; text.clear();
						textChanged = true;
					}
					eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
					break;
				case SCI_KEY_UP:
				case SCI_KEY_DOWN:
				case SCI_KEY_ENTER:
				case SCI_KEY_ESC:
				case SCI_KEY_TAB:
				case SCI_KEY_SHIFT_TAB:
					captureEvents = false;
					break;
				default:
					if ((curEvent.modifiers & SCI_KEYMOD_CTRL) && curEvent.data == 99) {
						// Control-C in earlier SCI games (SCI0 - SCI1 middle)
						// Control-C erases the whole line
						cursorPos = 0; text.clear();
						textChanged = true;
					} else if (curEvent.data > 31 && curEvent.data < 256 && textSize < maxChars) {
						// insert pressed character
						textAddChar = true;
						textChanged = true;
					}
					eventMan->getSciEvent(SCI_EVENT_KEYBOARD);	// consume the event
					break;
				}
				break;
			}
		}

		if (textChanged) {
			rect = g_sci->_gfxCompare->getNSRect(controlObject);

			if (textAddChar) {
				const char *textPtr = text.c_str();

				// We check if we are really able to add the new char
				uint16 textWidth = 0;
				while (*textPtr)
					textWidth += font->getCharWidth((byte)*textPtr++);
				textWidth += font->getCharWidth(curEvent.data);

				// Does it fit?
				if (textWidth >= rect.width()) {
					return;
				}

				text.insertChar(curEvent.data, cursorPos++);

				// Note: the following checkAltInput call might make the text
				// too wide to fit, but SSCI fails to check that too.
			}

			reg_t hunkId = readSelector(_segMan, controlObject, SELECTOR(bitmap));
			Common::Rect nsRect = g_sci->_gfxCompare->getNSRect(controlObject);
			//texteditCursorErase();	// TODO: Cursor

			// Write back string
			_segMan->strcpy(textReference, text.c_str());
			// Modify the buffer and show it
			_text->createTextBitmap(controlObject, 0, 0, hunkId);

			_text->drawTextBitmap(0, 0, nsRect, controlObject);
			//texteditCursorDraw(rect, text.c_str(), cursorPos);	// TODO: Cursor
			g_system->updateScreen();
		} else {
			// TODO: Cursor
			/*
			if (g_system->getMillis() >= _texteditBlinkTime) {
				_paint16->invertRect(_texteditCursorRect);
				_paint16->bitsShow(_texteditCursorRect);
				_texteditCursorVisible = !_texteditCursorVisible;
				texteditSetBlinkTime();
			}
			*/
		}

		textAddChar = false;
		textChanged = false;
		g_sci->sleep(10);
	}	// while
}

} // End of namespace Sci
