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
 */

#ifndef GRAPHICS_FONT_H
#define GRAPHICS_FONT_H

#include "common/str.h"

namespace Common {
template<class T> class Array;
}

namespace Graphics {

struct Surface;

/** Text alignment modes */
enum TextAlign {
	kTextAlignInvalid,
	kTextAlignLeft,     ///< Text should be aligned to the left
	kTextAlignCenter,   ///< Text should be centered
	kTextAlignRight     ///< Text should be aligned to the right
};

/**
 * Instances of this class represent a distinct font, with a built-in renderer.
 * @todo Maybe move the high-level methods (drawString etc.) to a separate
 *       FontRenderer class? That way, we could have different variants... ?
 */
class Font {
public:
	Font() {}
	virtual ~Font() {}

	/**
	 * Query the height of the font.
	 *
	 * @return font height.
	 */
	virtual int getFontHeight() const = 0;

	/**
	 * Query the maximum width of the font.
	 *
	 * @return maximum font width.
	 */
	virtual int getMaxCharWidth() const = 0;

	/**
	 * Query the width of a specific character.
	 *
	 * @param chr The character to query the width of.
	 * @return The character's width.
	 */
	virtual int getCharWidth(byte chr) const = 0;

	/**
	 * Query the kerning offset between two characters.
	 *
	 * @param left  The left character. May be 0.
	 * @param right The right character. May be 0.
	 * @return The horizontal displacement.
	 */
	virtual int getKerningOffset(byte left, byte right) const;

	/**
	 * Draw a character at a specific point on a surface.
	 *
	 * Note that the point describes the top left edge point of the
	 * character's bounding box.
	 *
	 * The Font implemenation should take care of not drawing outside of the
	 * specified surface.
	 *
	 * @param dst The surface to drawn on.
	 * @param chr The character to draw.
	 * @param x   The x coordinate where to draw the character.
	 * @param y   The y coordinate where to draw the character.
	 * @param color The color of the character.
	 */
	virtual void drawChar(Surface *dst, byte chr, int x, int y, uint32 color) const = 0;

	// TODO: Add doxygen comments to this
	void drawString(Surface *dst, const Common::String &str, int x, int y, int w, uint32 color, TextAlign align = kTextAlignLeft, int deltax = 0, bool useEllipsis = true) const;

	/**
	 * Compute and return the width the string str has when rendered using this font.
	 */
	int getStringWidth(const Common::String &str) const;

	/**
	 * Take a text (which may contain newline characters) and word wrap it so that
	 * no text line is wider than maxWidth pixels. If necessary, additional line breaks
	 * are generated, preferably between words (i.e. where whitespaces are).
	 * The resulting lines are appended to the lines string list.
	 * It returns the maximal width of any of the new lines (i.e. a value which is less
	 * or equal to maxWidth).
	 *
	 * @param str      the string to word wrap
	 * @param maxWidth the maximum width a line may have
	 * @param lines    the string list to which the text lines from str are appended
	 * @return the maximal width of any of the lines added to lines
	 */
	int wordWrapText(const Common::String &str, int maxWidth, Common::Array<Common::String> &lines) const;
};

} // End of namespace Graphics

#endif
