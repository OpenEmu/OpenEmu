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
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "common/scummsys.h"

#ifdef ENABLE_VKEYBD

#include "backends/vkeybd/virtual-keyboard-parser.h"
#include "backends/vkeybd/polygon.h"

#include "common/keyboard.h"
#include "common/util.h"
#include "common/system.h"
#include "common/archive.h"
#include "common/tokenizer.h"
#include "common/stream.h"

#include "graphics/decoders/bmp.h"

namespace Common {

VirtualKeyboardParser::VirtualKeyboardParser(VirtualKeyboard *kbd)
	: XMLParser(), _keyboard(kbd) {
}

void VirtualKeyboardParser::cleanup() {
	_mode = 0;
	_kbdParsed = false;
	_initialModeName.clear();
	if (_parseMode == kParseFull) {
		// reset keyboard to remove existing config
		_keyboard->reset();
	}
}

bool VirtualKeyboardParser::closedKeyCallback(ParserNode *node) {
	if (node->name.equalsIgnoreCase("keyboard")) {
		_kbdParsed = true;
		if (!_keyboard->_initialMode)
			return parserError("Initial mode of keyboard pack not defined");
	} else if (node->name.equalsIgnoreCase("mode")) {
		if (!_layoutParsed) {
			return parserError("'" + _mode->resolution + "' layout missing from '" + _mode->name + "' mode");
		}
	}
	return true;
}

bool VirtualKeyboardParser::parserCallback_keyboard(ParserNode *node) {
	if (_kbdParsed)
		return parserError("Only a single keyboard element is allowed");

	// if not full parse then we're done
	if (_parseMode == kParseCheckResolutions)
		return true;

	_initialModeName = node->values["initial_mode"];

	if (node->values.contains("h_align")) {
		String h = node->values["h_align"];
		if (h.equalsIgnoreCase("left"))
			_keyboard->_hAlignment = VirtualKeyboard::kAlignLeft;
		else if (h.equalsIgnoreCase("centre") || h.equalsIgnoreCase("center"))
			_keyboard->_hAlignment = VirtualKeyboard::kAlignCenter;
		else if (h.equalsIgnoreCase("right"))
			_keyboard->_hAlignment = VirtualKeyboard::kAlignRight;
	}

	if (node->values.contains("v_align")) {
		String v = node->values["v_align"];
		if (v.equalsIgnoreCase("top"))
			_keyboard->_vAlignment = VirtualKeyboard::kAlignTop;
		else if (v.equalsIgnoreCase("middle") || v.equalsIgnoreCase("center"))
			_keyboard->_vAlignment = VirtualKeyboard::kAlignMiddle;
		else if (v.equalsIgnoreCase("bottom"))
			_keyboard->_vAlignment = VirtualKeyboard::kAlignBottom;
	}

	return true;
}

bool VirtualKeyboardParser::parserCallback_mode(ParserNode *node) {
	String name = node->values["name"];

	if (_parseMode == kParseFull) {
		// if full parse then add new mode to keyboard
		if (_keyboard->_modes.contains(name))
			return parserError("Mode '" + name + "' has already been defined");

		VirtualKeyboard::Mode mode;
		mode.name = name;
		_keyboard->_modes[name] = mode;
	}

	_mode = &(_keyboard->_modes[name]);
	if (name == _initialModeName)
		_keyboard->_initialMode = _mode;

	String resolutions = node->values["resolutions"];
	StringTokenizer tok(resolutions, " ,");

	// select best resolution simply by minimising the difference between the
	// overlay size and the resolution dimensions.
	// TODO: improve this by giving preference to a resolution that is smaller
	// than the overlay res (so the keyboard can't be too big for the screen)
	uint16 scrW = g_system->getOverlayWidth(), scrH = g_system->getOverlayHeight();
	uint32 diff = 0xFFFFFFFF;
	String newResolution;
	for (String res = tok.nextToken(); res.size() > 0; res = tok.nextToken()) {
		int resW, resH;
		if (sscanf(res.c_str(), "%dx%d", &resW, &resH) != 2) {
			return parserError("Invalid resolution specification");
		} else {
			if (resW == scrW && resH == scrH) {
				newResolution = res;
				break;
			} else {
				uint32 newDiff = ABS(scrW - resW) + ABS(scrH - resH);
				if (newDiff < diff) {
					diff = newDiff;
					newResolution = res;
				}
			}
		}
	}

	if (newResolution.empty())
		return parserError("No acceptable resolution was found");

	if (_parseMode == kParseCheckResolutions) {
		if (_mode->resolution == newResolution) {
			node->ignore = true;
			return true;
		} else {
			// remove data relating to old resolution
			_mode->bitmapName.clear();
			if (_mode->image) {
				_mode->image->free();
				delete _mode->image;
				_mode->image = 0;
			}
			_mode->imageMap.removeAllAreas();
			_mode->displayArea = Rect();
		}
	}

	_mode->resolution = newResolution;
	_layoutParsed = false;

	return true;
}

bool VirtualKeyboardParser::parserCallback_event(ParserNode *node) {
	// if just checking resolutions we're done
	if (_parseMode == kParseCheckResolutions)
		return true;

	String name = node->values["name"];
	if (_mode->events.contains(name))
		return parserError("Event '" + name + "' has already been defined");

	VirtualKeyboard::VKEvent *evt = new VirtualKeyboard::VKEvent();
	evt->name = name;

	String type = node->values["type"];
	if (type.equalsIgnoreCase("key")) {
		if (!node->values.contains("code") || !node->values.contains("ascii")) {
			delete evt;
			return parserError("Key event element must contain code and ascii attributes");
		}
		evt->type = VirtualKeyboard::kVKEventKey;

		KeyState *ks = (KeyState *)malloc(sizeof(KeyState));
		ks->keycode = (KeyCode)atoi(node->values["code"].c_str());
		ks->ascii = atoi(node->values["ascii"].c_str());
		ks->flags = 0;
		if (node->values.contains("modifiers"))
			ks->flags = parseFlags(node->values["modifiers"]);
		evt->data = ks;

	} else if (type.equalsIgnoreCase("modifier")) {
		if (!node->values.contains("modifiers")) {
			delete evt;
			return parserError("Key modifier element must contain modifier attributes");
		}

		evt->type = VirtualKeyboard::kVKEventModifier;
		byte *flags = (byte *)malloc(sizeof(byte));
		if (!flags)
			error("[VirtualKeyboardParser::parserCallback_event] Cannot allocate memory");

		*(flags) = parseFlags(node->values["modifiers"]);
		evt->data = flags;

	} else if (type.equalsIgnoreCase("switch_mode")) {
		if (!node->values.contains("mode")) {
			delete evt;
			return parserError("Switch mode event element must contain mode attribute");
		}

		evt->type = VirtualKeyboard::kVKEventSwitchMode;
		String &mode = node->values["mode"];
		char *str = (char *)malloc(sizeof(char) * mode.size() + 1);
		if (!str)
			error("[VirtualKeyboardParser::parserCallback_event] Cannot allocate memory");

		memcpy(str, mode.c_str(), sizeof(char) * mode.size());
		str[mode.size()] = 0;
		evt->data = str;
	} else if (type.equalsIgnoreCase("submit")) {
		evt->type = VirtualKeyboard::kVKEventSubmit;
	} else if (type.equalsIgnoreCase("cancel")) {
		evt->type = VirtualKeyboard::kVKEventCancel;
	} else if (type.equalsIgnoreCase("clear")) {
		evt->type = VirtualKeyboard::kVKEventClear;
	} else if (type.equalsIgnoreCase("delete")) {
		evt->type = VirtualKeyboard::kVKEventDelete;
	} else if (type.equalsIgnoreCase("move_left")) {
		evt->type = VirtualKeyboard::kVKEventMoveLeft;
	} else if (type.equalsIgnoreCase("move_right")) {
		evt->type = VirtualKeyboard::kVKEventMoveRight;
	} else {
		delete evt;
		return parserError("Event type '" + type + "' not known");
	}

	_mode->events[name] = evt;

	return true;
}

bool VirtualKeyboardParser::parserCallback_layout(ParserNode *node) {
	assert(!_mode->resolution.empty());

	String res = node->values["resolution"];

	if (res != _mode->resolution) {
		node->ignore = true;
		return true;
	}

	_mode->bitmapName = node->values["bitmap"];

	SeekableReadStream *file = _keyboard->_fileArchive->createReadStreamForMember(_mode->bitmapName);
	if (!file)
		return parserError("Bitmap '" + _mode->bitmapName + "' not found");

	const Graphics::PixelFormat format = g_system->getOverlayFormat();

	{
		Graphics::BitmapDecoder bmp;
		if (!bmp.loadStream(*file))
			return parserError("Error loading bitmap '" + _mode->bitmapName + "'");

		_mode->image = bmp.getSurface()->convertTo(format);
	}

	delete file;

	int r, g, b;
	if (node->values.contains("transparent_color")) {
		if (!parseIntegerKey(node->values["transparent_color"], 3, &r, &g, &b))
			return parserError("Could not parse color value");
	} else {
		// default to purple
		r = 255;
		g = 0;
		b = 255;
	}
	_mode->transparentColor = format.RGBToColor(r, g, b);

	if (node->values.contains("display_font_color")) {
		if (!parseIntegerKey(node->values["display_font_color"], 3, &r, &g, &b))
			return parserError("Could not parse color value");
	} else {
		r = g = b = 0; // default to black
	}
	_mode->displayFontColor = format.RGBToColor(r, g, b);

	_layoutParsed = true;

	return true;
}

bool VirtualKeyboardParser::parserCallback_map(ParserNode *node) {
	return true;
}

bool VirtualKeyboardParser::parserCallback_area(ParserNode *node) {
	String &shape = node->values["shape"];
	String &target = node->values["target"];
	String &coords = node->values["coords"];

	if (target.equalsIgnoreCase("display_area")) {
		if (!shape.equalsIgnoreCase("rect"))
			return parserError("display_area must be a rect area");
		_mode->displayArea = Rect();
		return parseRect(_mode->displayArea, coords);
	} else if (shape.equalsIgnoreCase("rect")) {
		Polygon *poly = _mode->imageMap.createArea(target);
		if (!poly)
			return parserError(Common::String::format("Cannot define area '%s' again", target.c_str()));
		else
			return parseRectAsPolygon(*poly, coords);
	} else if (shape.equalsIgnoreCase("poly")) {
		Polygon *poly = _mode->imageMap.createArea(target);
		if (!poly)
			return parserError(Common::String::format("Cannot define area '%s' again", target.c_str()));
		else
			return parsePolygon(*poly, coords);
	}
	return parserError("Area shape '" + shape + "' not known");
}

byte VirtualKeyboardParser::parseFlags(const String &flags) {
	if (flags.empty())
		return 0;

	StringTokenizer tok(flags, ", ");
	byte val = 0;
	for (String fl = tok.nextToken(); !fl.empty(); fl = tok.nextToken()) {
		if (fl == "ctrl" || fl == "control")
			val |= KBD_CTRL;
		else if (fl == "alt")
			val |= KBD_ALT;
		else if (fl == "shift")
			val |= KBD_SHIFT;
	}
	return val;
}

bool VirtualKeyboardParser::parseRect(Rect &rect, const String &coords) {
	int x1, y1, x2, y2;
	if (!parseIntegerKey(coords, 4, &x1, &y1, &x2, &y2))
		return parserError("Invalid coords for rect area");
	rect.left = x1;
	rect.top = y1;
	rect.right = x2;
	rect.bottom = y2;
	if (!rect.isValidRect())
		return parserError("Rect area is not a valid rectangle");
	return true;
}

bool VirtualKeyboardParser::parsePolygon(Polygon &poly, const String &coords) {
	StringTokenizer tok(coords, ", ");
	for (String st = tok.nextToken(); !st.empty(); st = tok.nextToken()) {
		int x, y;
		if (sscanf(st.c_str(), "%d", &x) != 1)
			return parserError("Invalid coords for polygon area");
		st = tok.nextToken();
		if (sscanf(st.c_str(), "%d", &y) != 1)
			return parserError("Invalid coords for polygon area");
		poly.addPoint(x, y);
	}
	if (poly.getPointCount() < 3)
		return parserError("Invalid coords for polygon area");

	return true;
}

bool VirtualKeyboardParser::parseRectAsPolygon(Polygon &poly, const String &coords) {
	Rect rect;
	if (!parseRect(rect, coords))
		return false;
	poly.addPoint(rect.left, rect.top);
	poly.addPoint(rect.right, rect.top);
	poly.addPoint(rect.right, rect.bottom);
	poly.addPoint(rect.left, rect.bottom);
	return true;
}

} // End of namespace GUI

#endif // #ifdef ENABLE_VKEYBD
