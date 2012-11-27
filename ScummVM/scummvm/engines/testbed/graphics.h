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

#ifndef TESTBED_GRAPHICS_H
#define TESTBED_GRAPHICS_H

#include "testbed/testsuite.h"

namespace Testbed {

namespace GFXtests {

// Helper functions for GFX tests
void drawEllipse(int x, int y, int a, int b);
void setupMouseLoop(bool disableCursorPalette = false, const char *gfxModeName = "", int cursorTargetScale = 1);
void initMousePalette();
Common::Rect computeSize(const Common::Rect &cursorRect, int scalingFactor, int cursorTargetScale);
void HSVtoRGB(int &rComp, int &gComp, int &bComp, int hue, int sat, int val);
Common::Rect drawCursor(bool cursorPaletteDisabled = false, int cursorTargetScale = 1);

// will contain function declarations for GFX tests
TestExitStatus cursorTrails();
TestExitStatus fullScreenMode();
TestExitStatus aspectRatio();
TestExitStatus palettizedCursors();
TestExitStatus mouseMovements();
TestExitStatus copyRectToScreen();
TestExitStatus iconifyWindow();
TestExitStatus scaledCursors();
TestExitStatus shakingEffect();
TestExitStatus focusRectangle();
TestExitStatus overlayGraphics();
TestExitStatus paletteRotation();
TestExitStatus pixelFormats();
// add more here

} // End of namespace GFXtests

class GFXTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the GFXTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	GFXTestSuite();
	~GFXTestSuite() {}
	const char *getName() const {
		return "GFX";
	}
	const char *getDescription() const {
		return "Graphics Subsystem";
	}
	static void setCustomColor(uint r, uint g, uint b);

private:
	/**
	 * A Palette consists of 3 components RGB.
	 * As of now we only take 3 colors
	 * 0 (R:0, G:0, B:0) Black (kColorBlack)
	 * 1 (R:255, G:255, B:255) White (kColorWhite)
	 * 2 (R:255, G:255, B:255) your customized color (by default white) (kColorCustom)
	 * The remaining values are zero
	 */
	static byte _palette[256 * 3];
};

} // End of namespace Testbed

#endif // TESTBED_GRAPHICS_H
