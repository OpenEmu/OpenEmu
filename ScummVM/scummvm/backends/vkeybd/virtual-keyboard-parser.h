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

#ifndef COMMON_VIRTUAL_KEYBOARD_PARSER_H
#define COMMON_VIRTUAL_KEYBOARD_PARSER_H

#include "common/scummsys.h"

#ifdef ENABLE_VKEYBD

#include "common/xmlparser.h"
#include "backends/vkeybd/virtual-keyboard.h"

/**

                   ***************************************
                   ** Virtual Keyboard Pack File Format **
                   ***************************************

The new virtual keyboard for ScummVM is implemented in the same way as a HTML
ImageMap. It uses a single bitmap of the entire keyboard layout and then a
image map description allows certain areas of the bitmap to be given special
actions. Most of these actions will be a virtual key press event, but there
will also be special keys that will change the keyboard layout or close the
keyboard. The HTML image map description is contained in a larger XML file that
can describe all the different modes of the keyboard, and also different
keyboard layouts for different screen resolutions.

                ********************************************
                ** Example keyboard pack description file **
                ********************************************

<keyboard modes="normal,caps" initial_mode="normal" v_align="bottom" h_align="center">
  <mode name="normal" resolutions="640x400,320x200">
    <layout resolution="640x400" bitmap="normal_640x400.bmp" transparent_color="255,0,255">
      <map>
        <area shape="poly" coords="65,50,67,48,94,48,96,50,96,77,94,79,67,79,65,77" target="q" />
        <area shape="poly" coords="105,50,107,48,134,48,136,50,136,77,134,79,107,79,105,77" target="w" />
        <area shape="poly" coords="146,50,148,48,174,48,176,50,176,77,174,79,148,79,146,77" target="e" />
        ...
        <area shape="poly" coords="11,89,12,88,69,88,70,89,70,116,69,117,12,117,11,116" target="caps" />
      </map>
    </layout>
    <layout resolution="320x200" bitmap="normal_320x200.bmp" transparent_color="255,0,255">
      ...
    </layout>
    <event name="a" type="key" code="97" ascii="97" modifiers="" />
    <event name="b" type="key" code="98" ascii="98" modifiers="" />
    <event name="c" type="key" code="99" ascii="99" modifiers="" />
    ...
    <event name="caps" type="switch_mode" mode="caps" />
  </mode>

  <mode name="caps" resolutions="640x400">
    <layout resolution="640x400" bitmap="caps_640x480.bmp" transparent_color="255,0,255">
      <map>
        <area shape="poly" coords="65,50,67,48,94,48,96,50,96,77,94,79,67,79,65,77" target="Q" />
        ...
      </map>
    </layout>
    <event name="A" type="key" code="97" ascii="65" modifiers="shift" />
    <event name="B" type="key" code="98" ascii="66" modifiers="shift" />
    <event name="C" type="key" code="99" ascii="67" modifiers="shift" />
    ...
  </mode>
</keyboard>

*************************
** Description of tags **
*************************

<keyboard>

This is the required, root element of the file format.

required attributes:
 - initial_mode: name of the mode the keyboard will show initially

optional attributes:
 - v_align/h_align: where on the screen should the keyboard appear initially
   (defaults to bottom/center).

child tags:
 - mode

-------------------------------------------------------------------------------

<mode>

This tag encapsulates a single mode of the keyboard. Within are a number of
layouts, which provide the specific implementation at different resolutions.

required attributes:
 - name: the name of the mode
 - resolutions: list of the different layout resolutions

child tags:
 - layout
 - event

-------------------------------------------------------------------------------

<event>

These tags describe a particular event that will be triggered by a mouse click
on a particular area. The target attribute of each image map area should be the
same as an event's name.

required attributes:
 - name: name of the event
 - type: key | modifier | switch_mode | submit | cancel | clear | delete |
   move_left | move_right - see VirtualKeyboard::EventType for explanation
for key events
 - code / ascii: describe a key press in ScummVM KeyState format
for key and modifier events
 - modifiers: modifier keystate as comma-separated list of shift, ctrl and/or
   alt.
for switch_mode events
 - mode: name of the mode that should be switched to
-------------------------------------------------------------------------------

<layout>

These tags encapsulate an implementation of a mode at a particular resolution.

required attributes:
 - resolution: the screen resolution that this layout is designed for
 - bitmap: filename of the 24-bit bitmap that will be used for this layout

optional attributes:
 - transparent_color: color in r,g,b format that will be used for keycolor
   transparency (defaults to (255,0,255).
 - display_font_color: color in r,g,b format that will be used for the text of
   the keyboard display (defaults to (0,0,0).

child nodes:
 - map: this describes the image map using the same format as html image maps

-------------------------------------------------------------------------------

<map>

These tags describe the image map for a particular layout. It uses the same
format as HTML image maps. The only area shapes that are supported are
rectangles and polygons. The target attribute of each area should be the name
of an event for this mode (see <event> tag). They will usually be generated by
an external tool such as GIMP's Image Map plugin, and so will not be written
by hand, but for more information on HTML image map format see
 - http://www.w3schools.com/TAGS/tag_map.asp
 - http://www.w3schools.com/TAGS/tag_area.asp

*/

namespace Common {

/**
 * Subclass of Common::XMLParser that parses the virtual keyboard pack
 * description file
 */
class VirtualKeyboardParser : public XMLParser {

public:

	/**
	 * Enum dictating how extensive a parse will be
	 */
	enum ParseMode {
		/**
		 * Full parse - when loading keyboard pack for first time
		 */
		kParseFull,

		/**
		 * Just check resolutions and reload layouts if needed - following a
		 * change in screen size
		 */
		kParseCheckResolutions
	};

	VirtualKeyboardParser(VirtualKeyboard *kbd);
	void setParseMode(ParseMode m) {
		_parseMode = m;
	}

protected:
	CUSTOM_XML_PARSER(VirtualKeyboardParser) {
		XML_KEY(keyboard)
			XML_PROP(initial_mode, true)
			XML_PROP(v_align, false)
			XML_PROP(h_align, false)
			XML_KEY(mode)
				XML_PROP(name, true)
				XML_PROP(resolutions, true)
				XML_KEY(layout)
					XML_PROP(resolution, true)
					XML_PROP(bitmap, true)
					XML_PROP(transparent_color, false)
					XML_PROP(display_font_color, false)
					XML_KEY(map)
						XML_KEY(area)
							XML_PROP(shape, true)
							XML_PROP(coords, true)
							XML_PROP(target, true)
						KEY_END()
					KEY_END()
				KEY_END()
				XML_KEY(event)
					XML_PROP(name, true)
					XML_PROP(type, true)
					XML_PROP(code, false)
					XML_PROP(ascii, false)
					XML_PROP(modifiers, false)
					XML_PROP(mode, false)
				KEY_END()
			KEY_END()
		KEY_END()
	} PARSER_END()

protected:
	VirtualKeyboard *_keyboard;

	/** internal state variables of parser */
	ParseMode _parseMode;
	VirtualKeyboard::Mode *_mode;
	String _initialModeName;
	bool _kbdParsed;
	bool _layoutParsed;

	/** Cleanup internal state before parse */
	virtual void cleanup();

	/** Parser callback function */
	bool parserCallback_keyboard(ParserNode *node);
	bool parserCallback_mode(ParserNode *node);
	bool parserCallback_event(ParserNode *node);
	bool parserCallback_layout(ParserNode *node);
	bool parserCallback_map(ParserNode *node);
	bool parserCallback_area(ParserNode *node);
	virtual bool closedKeyCallback(ParserNode *node);

	/** Parse helper functions */
	byte parseFlags(const String &flags);
	bool parseRect(Rect &rect, const String &coords);
	bool parsePolygon(Polygon &poly, const String &coords);
	bool parseRectAsPolygon(Polygon &poly, const String &coords);
};

} // End of namespace GUI

#endif // #ifdef ENABLE_VKEYBD

#endif // #ifndef COMMON_VIRTUAL_KEYBOARD_PARSER_H
