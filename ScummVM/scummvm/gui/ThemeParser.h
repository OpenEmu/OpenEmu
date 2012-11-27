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

#ifndef THEME_PARSER_H
#define THEME_PARSER_H

#include "common/scummsys.h"
#include "common/xmlparser.h"

namespace GUI {

class ThemeEngine;

class ThemeParser : public Common::XMLParser {
public:
	ThemeParser(ThemeEngine *parent);

	virtual ~ThemeParser();

	bool getPaletteColor(const Common::String &name, int &r, int &g, int &b) {
		if (!_palette.contains(name))
			return false;

		r = _palette[name].r;
		g = _palette[name].g;
		b = _palette[name].b;

		return true;
	}

protected:
	ThemeEngine *_theme;

	CUSTOM_XML_PARSER(ThemeParser) {
		XML_KEY(render_info)
			XML_PROP(resolution, false)
			XML_KEY(palette)
				XML_KEY(color)
					XML_PROP(name, true)
					XML_PROP(rgb, true)
				KEY_END()
			KEY_END()

			XML_KEY(fonts)
				XML_KEY(font)
					XML_PROP(id, true)
					XML_PROP(file, true)
					XML_PROP(resolution, false)
					XML_PROP(scalable_file, false)
					XML_PROP(point_size, false)
				KEY_END()

				XML_KEY(text_color)
					XML_PROP(id, true);
					XML_PROP(color, true);
				KEY_END()
			KEY_END()

			XML_KEY(bitmaps)
				XML_KEY(bitmap)
					XML_PROP(filename, true)
					XML_PROP(resolution, false)
				KEY_END()
			KEY_END()

			XML_KEY(cursor)
				XML_PROP(file, true)
				XML_PROP(hotspot, true)
				XML_PROP(resolution, false)
			KEY_END()

			XML_KEY(defaults)
				XML_PROP(stroke, false)
				XML_PROP(shadow, false)
				XML_PROP(bevel, false)
				XML_PROP(factor, false)
				XML_PROP(fg_color, false)
				XML_PROP(bg_color, false)
				XML_PROP(gradient_start, false)
				XML_PROP(gradient_end, false)
				XML_PROP(bevel_color, false)
				XML_PROP(gradient_factor, false)
				XML_PROP(fill, false)
			KEY_END()

			XML_KEY(drawdata)
				XML_PROP(id, true)
				XML_PROP(cache, false)
				XML_PROP(resolution, false)

				XML_KEY(defaults)
					XML_PROP(stroke, false)
					XML_PROP(shadow, false)
					XML_PROP(bevel, false)
					XML_PROP(factor, false)
					XML_PROP(fg_color, false)
					XML_PROP(bg_color, false)
					XML_PROP(gradient_start, false)
					XML_PROP(gradient_end, false)
					XML_PROP(bevel_color, false)
					XML_PROP(gradient_factor, false)
					XML_PROP(fill, false)
				KEY_END()

				XML_KEY(drawstep)
					XML_PROP(func, true)
					XML_PROP(stroke, false)
					XML_PROP(shadow, false)
					XML_PROP(bevel, false)
					XML_PROP(factor, false)
					XML_PROP(fg_color, false)
					XML_PROP(bg_color, false)
					XML_PROP(gradient_start, false)
					XML_PROP(gradient_end, false)
					XML_PROP(gradient_factor, false)
					XML_PROP(bevel_color, false)
					XML_PROP(fill, false)
					XML_PROP(radius, false)
					XML_PROP(width, false)
					XML_PROP(height, false)
					XML_PROP(xpos, false)
					XML_PROP(ypos, false)
					XML_PROP(padding, false)
					XML_PROP(orientation, false)
					XML_PROP(file, false)
				KEY_END()

				XML_KEY(text)
					XML_PROP(font, true)
					XML_PROP(text_color, true)
					XML_PROP(vertical_align, true)
					XML_PROP(horizontal_align, true)
				KEY_END()
			KEY_END()

		KEY_END() // render_info end

		XML_KEY(layout_info)
			XML_PROP(resolution, false)
			XML_KEY(globals)
				XML_PROP(resolution, false)
				XML_KEY(def)
					XML_PROP(var, true)
					XML_PROP(value, true)
					XML_PROP(resolution, false)
				KEY_END()

				XML_KEY(widget)
					XML_PROP(name, true)
					XML_PROP(size, false)
					XML_PROP(pos, false)
					XML_PROP(padding, false)
					XML_PROP(resolution, false)
					XML_PROP(textalign, false)
				KEY_END()
			KEY_END()

			XML_KEY(dialog)
				XML_PROP(name, true)
				XML_PROP(overlays, true)
				XML_PROP(shading, false)
				XML_PROP(enabled, false)
				XML_PROP(resolution, false)
				XML_PROP(inset, false)
				XML_KEY(layout)
					XML_PROP(type, true)
					XML_PROP(center, false)
					XML_PROP(padding, false)
					XML_PROP(spacing, false)

					XML_KEY(import)
						XML_PROP(layout, true)
					KEY_END()

					XML_KEY(widget)
						XML_PROP(name, true)
						XML_PROP(width, false)
						XML_PROP(height, false)
						XML_PROP(type, false)
						XML_PROP(enabled, false)
						XML_PROP(textalign, false)
					KEY_END()

					XML_KEY(space)
						XML_PROP(size, false)
					KEY_END()

					XML_KEY_RECURSIVE(layout)
				KEY_END()
			KEY_END()
		KEY_END()

	} PARSER_END()

	/** Render info callbacks */
	bool parserCallback_render_info(ParserNode *node);
	bool parserCallback_defaults(ParserNode *node);
	bool parserCallback_font(ParserNode *node);
	bool parserCallback_text_color(ParserNode *node);
	bool parserCallback_fonts(ParserNode *node);
	bool parserCallback_text(ParserNode *node);
	bool parserCallback_palette(ParserNode *node);
	bool parserCallback_color(ParserNode *node);
	bool parserCallback_drawstep(ParserNode *node);
	bool parserCallback_drawdata(ParserNode *node);
	bool parserCallback_bitmaps(ParserNode *node) { return true; }
	bool parserCallback_bitmap(ParserNode *node);
	bool parserCallback_cursor(ParserNode *node);


	/** Layout info callbacks */
	bool parserCallback_layout_info(ParserNode *node);
	bool parserCallback_globals(ParserNode *node) { return true; }
	bool parserCallback_def(ParserNode *node);
	bool parserCallback_widget(ParserNode *node);
	bool parserCallback_dialog(ParserNode *node);
	bool parserCallback_layout(ParserNode *node);
	bool parserCallback_space(ParserNode *node);
	bool parserCallback_import(ParserNode *node);

	bool closedKeyCallback(ParserNode *node);

	bool resolutionCheck(const Common::String &resolution);

	void cleanup();

	Graphics::DrawStep *newDrawStep();
	Graphics::DrawStep *defaultDrawStep();
	bool parseDrawStep(ParserNode *stepNode, Graphics::DrawStep *drawstep, bool functionSpecific);
	bool parseCommonLayoutProps(ParserNode *node, const Common::String &var);

	Graphics::DrawStep *_defaultStepGlobal;
	Graphics::DrawStep *_defaultStepLocal;

	struct PaletteColor {
		uint8 r, g, b;
	};

	Common::HashMap<Common::String, PaletteColor, Common::IgnoreCase_Hash, Common::IgnoreCase_EqualTo> _palette;
};

} // End of namespace GUI

#endif
