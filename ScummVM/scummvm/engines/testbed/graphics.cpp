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

#include "common/events.h"
#include "common/list.h"
#include "common/random.h"

#include "engines/engine.h"

#include "testbed/graphics.h"
#include "testbed/testsuite.h"

#include "graphics/cursorman.h"
#include "graphics/fontman.h"
#include "graphics/palette.h"
#include "graphics/surface.h"
#include "graphics/VectorRendererSpec.h"

namespace Testbed {

byte GFXTestSuite::_palette[256 * 3] = {0, 0, 0, 255, 255, 255, 255, 255, 255};

GFXTestSuite::GFXTestSuite() {
	// Initialize color palettes
	// The fourth field is for alpha channel which is unused
	// Assuming 8bpp as of now
	g_system->getPaletteManager()->setPalette(_palette, 0, 3);

	// Init Mouse Palette (White-black-yellow)
	GFXtests::initMousePalette();

	// Add tests here

	// Blitting buffer on screen
	addTest("BlitBitmaps", &GFXtests::copyRectToScreen);

	// GFX Transcations
	addTest("FullScreenMode", &GFXtests::fullScreenMode);
	addTest("AspectRatio", &GFXtests::aspectRatio);
	addTest("IconifyingWindow", &GFXtests::iconifyWindow);

	// Mouse Layer tests (Palettes and movements)
	addTest("PalettizedCursors", &GFXtests::palettizedCursors);
	addTest("MouseMovements", &GFXtests::mouseMovements);
	// FIXME: Scaled cursor crash with odd dimmensions
	addTest("ScaledCursors", &GFXtests::scaledCursors);

	// Effects
	addTest("shakingEffect", &GFXtests::shakingEffect);
	// addTest("focusRectangle", &GFXtests::focusRectangle);

	// Overlay
	addTest("Overlays", &GFXtests::overlayGraphics);

	// Specific Tests:
	addTest("PaletteRotation", &GFXtests::paletteRotation);
	addTest("cursorTrailsInGUI", &GFXtests::cursorTrails);
	//addTest("Pixel Formats", &GFXtests::pixelFormats);
}

void GFXTestSuite::setCustomColor(uint r, uint g, uint b) {
	_palette[6] = r;
	_palette[7] = g;
	_palette[8] = b;

	// Set colorNum kColorSpecial with a special color.
	int absIndx = kColorSpecial * 3;
	_palette[absIndx + 1] = 173;
	_palette[absIndx + 2] = 255;
	_palette[absIndx + 3] = 47;
	g_system->getPaletteManager()->setPalette(_palette, 0, 256);
}

// Helper functions used by GFX tests

void GFXtests::initMousePalette() {
	byte palette[3 * 3]; // Black, white and yellow

	palette[0] = palette[1] = palette[2] = 0;
	palette[3] = palette[4] = palette[5] = 255;
	palette[6] = palette[7] = 255;
	palette[8] = 0;

	CursorMan.replaceCursorPalette(palette, 0, 3);
}

Common::Rect GFXtests::computeSize(const Common::Rect &cursorRect, int scalingFactor, int cursorTargetScale) {
	if (cursorTargetScale == 1 || scalingFactor == 1) {
		// Game data and cursor would be scaled equally.
		// so dimensions would be same.
		return Common::Rect(cursorRect.width(), cursorRect.height());
	}

	if (scalingFactor == 2) {
		// Game data is scaled by 2, cursor is said to be scaled by 2 or 3. so it wud not be scaled any further
		// So a w/2 x h/2 rectangle when scaled would match the cursor
		return Common::Rect(cursorRect.width() / 2, cursorRect.height() / 2);
	}

	if (scalingFactor == 3) {
		// Cursor traget scale is 2 or 3.
		return Common::Rect((cursorRect.width() / cursorTargetScale), (cursorRect.height() / cursorTargetScale));
	} else {
		Testsuite::logPrintf("Unsupported scaler %dx\n", scalingFactor);
		return Common::Rect();
	}
}

void GFXtests::HSVtoRGB(int &rComp, int &gComp, int &bComp, int hue, int sat, int val) {
	float r = rComp;
	float g = gComp;
	float b = bComp;

	float h = hue * (360 / 256.0); // All colors are tried
	float s = sat;
	float v = val;

	int i;
	float f, p, q, t;

	if (s == 0) {
		rComp = gComp = bComp = (int)(v * 255);
		return;
	}

	h /= 60;
	i = (int)h;
	f = h - i;
	p = v * (1 - s);
	q = v * (1 - s * f);
	t = v * (1 - s * (1 - f));

	switch (i) {
	case 0:
		r = v;
		g = t;
		b = p;
		break;
	case 1:
		r = q;
		g = v;
		b = p;
		break;
	case 2:
		r = p;
		g = v;
		b = t;
		break;
	case 3:
		r = p;
		g = q;
		b = v;
		break;
	case 4:
		r = t;
		g = p;
		b = v;
		break;
	default:
		r = v;
		g = p;
		b = q;
		break;
	}

	rComp = (int)(r * 255);
	gComp = (int)(g * 255);
	bComp = (int)(b * 255);
}

Common::Rect GFXtests::drawCursor(bool cursorPaletteDisabled, int cursorTargetScale) {
	// Buffer initialized with yellow color
	byte buffer[500];
	memset(buffer, 2, sizeof(buffer));

	int cursorWidth = 12;
	int cursorHeight = 12;

	/* Disable HotSpot highlighting as of now

	// Paint the cursor with yellow, except the hotspot
	for (int i = 0; i < 16; i++) {
		for (int j = 0; j < 16; j++) {
			if (i != j && i != 15 - j) {
				buffer[i * 16 + j] = 2;
			}
		}
	}

	*/

	// Uncommenting the next line and commenting the line after that would reproduce the crash
	// CursorMan.replaceCursor(buffer, 11, 11, 0, 0, 255, cursorTargetScale);
	CursorMan.replaceCursor(buffer, 12, 12, 0, 0, 255, cursorTargetScale);
	CursorMan.showMouse(true);

	if (cursorPaletteDisabled) {
		CursorMan.disableCursorPalette(true);
	} else {
		initMousePalette();
		CursorMan.disableCursorPalette(false);
	}

	g_system->updateScreen();
	return Common::Rect(0, 0, cursorWidth, cursorHeight);
}

void rotatePalette(byte *palette, int size) {
	// Rotate the colors starting from address palette "size" times

	// take a temporary palette color
	byte tColor[3] = {0};
	// save first color in it.
	memcpy(tColor, &palette[0], 3 * sizeof(byte));

	// Move each color upward by 1
	for (int i = 0; i < size - 1; i++) {
		memcpy(&palette[i * 3], &palette[(i + 1) * 3], 3 * sizeof(byte));
	}
	// Assign last color to tcolor
	memcpy(&palette[(size - 1) * 3], tColor, 3 * sizeof(byte));
}

/**
 * Sets up mouse loop, exits when user clicks any of the mouse button
 */
void GFXtests::setupMouseLoop(bool disableCursorPalette, const char *gfxModeName, int cursorTargetScale) {
	bool isFeaturePresent;
	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureCursorPalette);
	Common::Rect cursorRect;

	if (isFeaturePresent) {

		cursorRect = GFXtests::drawCursor(disableCursorPalette, cursorTargetScale);

		Common::EventManager *eventMan = g_system->getEventManager();
		Common::Event event;
		Common::Point pt(0, 100);

		bool quitLoop = false;
		uint32 lastRedraw = 0;
		const uint32 waitTime = 1000 / 45;

		Testsuite::clearScreen();
		Common::String info = disableCursorPalette ? "Using Game Palette" : "Using cursor palette";
		info += " to render the cursor, Click to finish";

		Common::String gfxScalarMode(gfxModeName);

		if (!gfxScalarMode.equals("")) {
			info = "The cursor size (yellow) should match the red rectangle.";
		}

		Testsuite::writeOnScreen(info, pt);

		info = "GFX Mode";
		info += gfxModeName;
		info += " ";

		char cScale = cursorTargetScale + '0';
		info += "Cursor scale: ";
		info += cScale;

		Common::Rect estimatedCursorRect;

		if (!gfxScalarMode.equals("")) {

			if (gfxScalarMode.contains("1x")) {
				estimatedCursorRect = computeSize(cursorRect, 1, cursorTargetScale);
			} else if (gfxScalarMode.contains("2x")) {
				estimatedCursorRect = computeSize(cursorRect, 2, cursorTargetScale);
			} else if (gfxScalarMode.contains("3x")) {
				estimatedCursorRect = computeSize(cursorRect, 3, cursorTargetScale);
			} else {
				// If unable to detect scaler, default to 2
				Testsuite::writeOnScreen("Unable to detect scaling factor, assuming 2x", Common::Point(0, 5));
				estimatedCursorRect = computeSize(cursorRect, 2, cursorTargetScale);
			}
			Testsuite::writeOnScreen(info, Common::Point(0, 120));

			// Move cursor to (20, 20)
			g_system->warpMouse(20, 20);
			// Move estimated rect to (20, 20)
			estimatedCursorRect.moveTo(20, 20);

			Graphics::Surface *screen = g_system->lockScreen();
			GFXTestSuite::setCustomColor(255, 0, 0);
			screen->fillRect(estimatedCursorRect, 2);
			g_system->unlockScreen();
			g_system->updateScreen();
		}

		while (!quitLoop) {
			while (eventMan->pollEvent(event)) {
				if (Engine::shouldQuit()) {
					// Quit directly
					return;
				}
				if (lastRedraw + waitTime < g_system->getMillis()) {
					g_system->updateScreen();
					lastRedraw = g_system->getMillis();
				}

				switch (event.type) {
				case Common::EVENT_MOUSEMOVE:
					break;
				case Common::EVENT_LBUTTONDOWN:
				case Common::EVENT_RBUTTONDOWN:
					quitLoop = true;
					Testsuite::clearScreen();
					Testsuite::writeOnScreen("Mouse clicked", pt);
					g_system->delayMillis(1000);
					break;
				default:
					break;// Ignore handling any other event

				}
			}
		}
	} else {
		Testsuite::displayMessage("feature not supported");
	}
}

/**
 * Used by aspectRatio()
 */
void GFXtests::drawEllipse(int cx, int cy, int a, int b) {

	// Take a buffer of screen size
	int width = g_system->getWidth();
	int height = Testsuite::getDisplayRegionCoordinates().y;
	byte *buffer = new byte[height * width];
	double theta;
	int x, y, x1, y1;

	memset(buffer, 0, sizeof(byte) * width * height);
	// Illuminate the center
	buffer[cx * width + cy] = 1;

	// Illuminate the points lying on ellipse

	for (theta = 0; theta <= M_PI / 2; theta += M_PI / 360) {
		x = (int)(b * sin(theta) + 0.5);
		y = (int)(a * cos(theta) + 0.5);

		// This gives us four points

		x1 = x + cx;
		y1 = y + cy;

		buffer[x1 * width + y1] = 1;

		x1 = (-1) * x + cx;
		y1 = y + cy;

		buffer[x1 * width + y1] = 1;

		x1 = x + cx;
		y1 = (-1) * y + cy;

		buffer[x1 * width + y1] = 1;

		x1 = (-1) * x + cx;
		y1 = (-1) * y + cy;

		buffer[x1 * width + y1] = 1;
	}

	g_system->copyRectToScreen(buffer, width, 0, 0, width, height);
	g_system->updateScreen();
	delete[] buffer;
}

// GFXtests go here

/**
 * Tests the fullscreen mode by: toggling between fullscreen and windowed mode
 */
TestExitStatus GFXtests::fullScreenMode() {
	Testsuite::clearScreen();
	Common::String info = "Fullscreen test. Here you should expect a toggle between windowed and fullscreen states depending "
	"upon your initial state.";

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing fullscreen mode", pt);

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : FullScreenMode\n");
		return kTestSkipped;
	}

	bool isFeaturePresent;
	bool isFeatureEnabled;
	TestExitStatus passed = kTestPassed;
	Common::String prompt;
	OptionSelected shouldSelect;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureFullscreenMode);

	if (isFeaturePresent) {
		// Toggle
		isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;

		g_system->delayMillis(1000);

		if (isFeatureEnabled) {
			Testsuite::logDetailedPrintf("Current Mode is Fullsecreen\n");
		} else {
			Testsuite::logDetailedPrintf("Current Mode is Windowed\n");
		}

		prompt = " Which mode do you see currently ?  ";

		if (!Testsuite::handleInteractiveInput(prompt, "Fullscreen", "Windowed", shouldSelect)) {
			// User selected incorrect current state
			passed = kTestFailed;
			Testsuite::logDetailedPrintf("g_system->getFeatureState() failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		// Current state should be now !isFeatureEnabled
		isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureFullscreenMode);
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;

		g_system->delayMillis(1000);

		prompt = "  Which screen mode do you see now ?   ";

		if (!Testsuite::handleInteractiveInput(prompt, "Fullscreen", "Windowed", shouldSelect)) {
			// User selected incorrect mode
			passed = kTestFailed;
			Testsuite::logDetailedPrintf("g_system->setFeatureState() failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureFullscreenMode, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);

		prompt = "This should be your initial state. Is it?";

		if (!Testsuite::handleInteractiveInput(prompt, "Yes, it is", "Nopes", shouldSelect)) {
			// User selected incorrect mode
			Testsuite::logDetailedPrintf("switching back to initial state failed\n");
			passed = kTestFailed;
		}

	} else {
		Testsuite::displayMessage("feature not supported");
	}

	return passed;
}

/**
 * Tests the aspect ratio correction by: drawing an ellipse, when corrected the ellipse should render to a circle
 */
TestExitStatus GFXtests::aspectRatio() {

	Testsuite::clearScreen();
	Common::String info = "Aspect Ratio Correction test. If aspect ratio correction is enabled you should expect a circle on screen,"
	" an ellipse otherwise.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Aspect Ratio\n");
		return kTestSkipped;
	}
	// Draw an ellipse on the screen
	drawEllipse(80, 160, 72, 60);

	bool isFeaturePresent;
	bool isFeatureEnabled;
	TestExitStatus passed = kTestPassed;
	Common::String prompt;
	OptionSelected shouldSelect;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureAspectRatioCorrection);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		// Toggle
		shouldSelect = isFeatureEnabled ? kOptionLeft : kOptionRight;
		prompt = " What does the curve on screen appears to you ?";
		if (!Testsuite::handleInteractiveInput(prompt, "Circle", "Ellipse", shouldSelect)) {
			// User selected incorrect option
			passed = kTestFailed;
			Testsuite::logDetailedPrintf("Aspect Ratio Correction failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);

		shouldSelect = !isFeatureEnabled ? kOptionLeft : kOptionRight;
		prompt = " What does the curve on screen appears to you ?";
		if (!Testsuite::handleInteractiveInput(prompt, "Circle", "Ellipse", shouldSelect)) {
			// User selected incorrect option
			passed = kTestFailed;
			Testsuite::logDetailedPrintf("Aspect Ratio Correction failed\n");
		}

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, isFeatureEnabled);
		g_system->endGFXTransaction();
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	g_system->delayMillis(500);

	if (Testsuite::handleInteractiveInput("This should definetely be your initial state?", "Yes, it is", "Nopes", kOptionRight)) {
		// User selected incorrect mode
		Testsuite::logDetailedPrintf("Switching back to initial state failed\n");
		passed = kTestFailed;
	}

	return passed;
}

/**
 * Tests Palettized cursors.
 * Method: Create a yellow colored cursor, should be able to move it. Once you click test terminates
 */
TestExitStatus GFXtests::palettizedCursors() {

	Testsuite::clearScreen();
	Common::String info = "Palettized Cursors test.\n "
		"Here you should expect to see a yellow mouse cursor rendered with mouse graphics.\n"
		"You would be able to move the cursor. Later we use game graphics to render the cursor.\n"
		"For cursor palette it should be yellow and will be red if rendered by the game palette.\n"
		"The test finishes when mouse (L/R) is clicked.";


	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Palettized Cursors\n");
		return kTestSkipped;
	}

	TestExitStatus passed = kTestPassed;

	// Testing with cursor Palette
	setupMouseLoop();

	if (Testsuite::handleInteractiveInput("Which color did the cursor appeared to you?", "Yellow", "Any other", kOptionRight)) {
		Testsuite::logDetailedPrintf("Couldn't use cursor palette for rendering cursor\n");
		passed = kTestFailed;
	}

	// Testing with game Palette
	GFXTestSuite::setCustomColor(255, 0, 0);
	setupMouseLoop(true);

	if (Testsuite::handleInteractiveInput("Which color did the cursor appeared to you?", "Red", "Any other", kOptionRight)) {
		Testsuite::logDetailedPrintf("Couldn't use Game palette for rendering cursor\n");
		passed = kTestFailed;
	}

	if (!Testsuite::handleInteractiveInput("     Did test run as was described?     ")) {
		passed = kTestFailed;
	}

	// re-enable cursor palette
	CursorMan.disableCursorPalette(false);
	// Done with cursors, make them invisible, any other test the could simply make it visible
	CursorMan.showMouse(false);
	return passed;
}

/**
 * Tests automated mouse movements. "Warp" functionality provided by the backend.
 */

TestExitStatus GFXtests::mouseMovements() {
	Testsuite::clearScreen();
	// Ensure that the cursor is visible
	CursorMan.showMouse(true);

	Common::String info = "Testing Automated Mouse movements.\n"
						"You should expect cursor hotspot(top-left corner) to automatically move from (0, 0) to (100, 100).\n"
						"There we have a rectangle drawn, finally the cursor would lie centered in that rectangle.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Mouse Movements\n");
		return kTestSkipped;
	}

	// Draw Rectangle
	Graphics::Surface *screen = g_system->lockScreen();
	// Ensure that 2 represents red in current palette
	GFXTestSuite::setCustomColor(255, 0, 0);
	screen->fillRect(Common::Rect::center(106, 106, 14, 14), 2);
	g_system->unlockScreen();

	// Testing Mouse Movements now!
	Common::Point pt(0, 10);
	Testsuite::writeOnScreen("Moving mouse hotspot automatically from (0, 0) to (100, 100)", pt);
	g_system->warpMouse(0, 0);
	g_system->updateScreen();
	g_system->delayMillis(1000);

	for (int i = 0; i <= 100; i++) {
		g_system->delayMillis(20);
		g_system->warpMouse(i, i);
		g_system->updateScreen();
	}

	Testsuite::writeOnScreen("Mouse hotspot Moved to (100, 100)", pt);
	g_system->delayMillis(1500);
	CursorMan.showMouse(false);

	if (Testsuite::handleInteractiveInput("Was the cursor centered in the rectangle at (100, 100)?", "Yes", "No", kOptionRight)) {
		return kTestFailed;
	}

	return kTestPassed;
}



/**
 * This basically blits the screen by the contents of its buffer.
 *
 */
TestExitStatus GFXtests::copyRectToScreen() {

	Testsuite::clearScreen();
	Common::String info = "Testing Blitting a Bitmap to screen.\n"
		"You should expect to see a 20x40 yellow horizontal rectangle centered at the screen.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Blitting Bitmap\n");
		return kTestSkipped;
	}

	GFXTestSuite::setCustomColor(255, 255, 0);
	byte buffer[20 * 40];
	memset(buffer, 2, 20 * 40);

	uint x = g_system->getWidth() / 2 - 20;
	uint y = g_system->getHeight() / 2 - 10;

	g_system->copyRectToScreen(buffer, 40, x, y, 40, 20);
	g_system->updateScreen();
	g_system->delayMillis(1000);

	if (Testsuite::handleInteractiveInput("      Did you see yellow rectangle ?       ", "Yes", "No", kOptionRight)) {
		return kTestFailed;
	}

	return kTestPassed;
}

/**
 * Testing feature : Iconifying window
 * It is expected the screen minimizes when this feature is enabled
 */
TestExitStatus GFXtests::iconifyWindow() {

	Testsuite::clearScreen();
	Common::String info = "Testing Iconify Window mode.\n If the feature is supported by the backend, "
		"you should expect the window to be minimized.\n However you would manually need to de-iconify.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Iconifying window\n");
		return kTestSkipped;
	}

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Testing Iconifying window", pt);

	bool isFeaturePresent;
	bool isFeatureEnabled;

	isFeaturePresent = g_system->hasFeature(OSystem::kFeatureIconifyWindow);
	isFeatureEnabled = g_system->getFeatureState(OSystem::kFeatureIconifyWindow);
	g_system->delayMillis(1000);

	if (isFeaturePresent) {
		// Toggle

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureIconifyWindow, !isFeatureEnabled);
		g_system->endGFXTransaction();

		g_system->delayMillis(1000);

		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureIconifyWindow, isFeatureEnabled);
		g_system->endGFXTransaction();
	} else {
		Testsuite::displayMessage("feature not supported");
	}

	if (Testsuite::handleInteractiveInput("  Did you see the window minimized?  ", "Yes", "No", kOptionRight)) {
		return kTestFailed;
	}

	return kTestPassed;
}

/**
 * Testing feature: Scaled cursors
 */
TestExitStatus GFXtests::scaledCursors() {

	Testsuite::clearScreen();
	Common::String info = "Testing : Scaled cursors\n"
		"Here every graphics mode is tried with a cursorTargetScale of 1, 2 and 3.\n"
		"The expected cursor size is drawn as a rectangle.\n The cursor should approximately match that rectangle.\n"
		"This may take time, You may skip the later scalers and just examine the first three i.e 1x, 2x and 3x";

	bool isAspectRatioCorrected = g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection);

	if (isAspectRatioCorrected) {
		info += "\nDisabling Aspect ratio correction, for letting cusors match exactly, will be restored after this test.";
	}

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Scaled Cursors\n");
		return kTestSkipped;
	}

	int maxLimit = 1000;
	if (!Testsuite::handleInteractiveInput("Do You want to restrict scalers to 1x, 2x and 3x only?", "Yes", "No", kOptionRight)) {
		maxLimit = 3;
	}


	if (isAspectRatioCorrected) {
		g_system->beginGFXTransaction();
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, false);
		g_system->endGFXTransaction();
	}

	const int currGFXMode = g_system->getGraphicsMode();
	const OSystem::GraphicsMode *gfxMode = g_system->getSupportedGraphicsModes();

	while (gfxMode->name && maxLimit > 0) {
		// for every graphics mode display cursors for cursorTargetScale 1, 2 and 3
		// Switch Graphics mode
		// FIXME: Crashes with "3x" mode now.:

		info = Common::String::format("Testing : Scaled cursors with GFX Mode %s\n", gfxMode->name);
		if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
			Testsuite::logPrintf("\tInfo! Skipping sub-test : Scaled Cursors :: GFX Mode %s\n", gfxMode->name);
			gfxMode++;
			maxLimit--;
			continue;
		}
		if (Engine::shouldQuit()) {
			// Explicit exit requested
			Testsuite::logPrintf("Info! Explicit exit requested during scaling test, this test may be incomplete\n");
			return kTestSkipped;
		}

		g_system->beginGFXTransaction();

		bool isGFXModeSet = g_system->setGraphicsMode(gfxMode->id);
		g_system->initSize(320, 200);

		OSystem::TransactionError gfxError = g_system->endGFXTransaction();

		if (gfxError == OSystem::kTransactionSuccess && isGFXModeSet) {
			setupMouseLoop(false, gfxMode->name, 1);
			Testsuite::clearScreen();

			setupMouseLoop(false, gfxMode->name, 2);
			Testsuite::clearScreen();

			setupMouseLoop(false, gfxMode->name, 3);
			Testsuite::clearScreen();
		} else {
			Testsuite::logDetailedPrintf("Switching to graphics mode %s failed\n", gfxMode->name);
			return kTestFailed;
		}
		gfxMode++;
		maxLimit--;

		info = "Did the expected cursor size and the actual cursor size matched?";
		if (Testsuite::handleInteractiveInput(info, "Yes", "No", kOptionRight)) {
			Testsuite::logPrintf("\tInfo! Failed sub-test : Scaled Cursors :: GFX Mode %s\n", gfxMode->name);
		}

		if (Engine::shouldQuit()) {
			// Explicit exit requested
			Testsuite::logPrintf("Info! Explicit exit requested during scaling test, this test may be incomplete\n");
			return kTestSkipped;
		}

	}

	// Restore Original State
	g_system->beginGFXTransaction();
	bool isGFXModeSet = g_system->setGraphicsMode(currGFXMode);
	g_system->initSize(320, 200);

	if (isAspectRatioCorrected) {
		g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, true);
	}

	OSystem::TransactionError gfxError = g_system->endGFXTransaction();

	if (gfxError != OSystem::kTransactionSuccess || !isGFXModeSet) {
		Testsuite::logDetailedPrintf("Switcing to initial state failed\n");
		return kTestFailed;
	}

	// Done with cursors, Make them invisible, any other test may enable and use it.
	CursorMan.showMouse(false);
	return kTestPassed;
}

TestExitStatus GFXtests::shakingEffect() {

	Testsuite::clearScreen();
	Common::String info = "Shaking test. You should expect the graphics(text/bars etc) drawn on the screen to shake!";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Shaking Effect\n");
		return kTestSkipped;
	}

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("If Shaking Effect works, this should shake!", pt);
	int times = 15;
	while (times--) {
		g_system->setShakePos(25);
		g_system->delayMillis(50);
		g_system->updateScreen();
		g_system->setShakePos(0);
		g_system->delayMillis(50);
		g_system->updateScreen();
	}
	g_system->delayMillis(1500);

	if (Testsuite::handleInteractiveInput("Did the Shaking test worked as you were expecting?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Shaking Effect didn't worked");
		return kTestFailed;
	}
	return kTestPassed;
}

TestExitStatus GFXtests::focusRectangle() {

	Testsuite::clearScreen();
	Common::String info = "Testing : Setting and hiding Focus \n"
		"If this feature is implemented, the focus should be toggled between the two rectangles on the corners";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : focus Rectangle\n");
		return kTestSkipped;
	}

	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));

	Graphics::Surface *screen = g_system->lockScreen();
	int screenHeight = g_system->getHeight();
	int screenWidth = g_system->getWidth();

	int height = font.getFontHeight();
	int width = screenWidth / 2;

	Common::Rect rectLeft(0, 0, width, height * 2);
	screen->fillRect(rectLeft, kColorWhite);
	font.drawString(screen, "Focus 1", rectLeft.left, rectLeft.top, width, kColorBlack, Graphics::kTextAlignLeft);

	Common::Rect rectRight(screenWidth - width, screenHeight - height * 2 , screenWidth, screenHeight);
	screen->fillRect(rectRight, kColorWhite);
	font.drawString(screen, "Focus 2", rectRight.left, rectRight.top, width, kColorBlack, Graphics::kTextAlignRight);
	g_system->unlockScreen();
	g_system->updateScreen();

	g_system->clearFocusRectangle();

	g_system->setFocusRectangle(rectLeft);
	g_system->updateScreen();

	g_system->delayMillis(1000);

	g_system->setFocusRectangle(rectRight);
	g_system->updateScreen();

	g_system->clearFocusRectangle();

	if (Testsuite::handleInteractiveInput("Did you noticed a variation in focus?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Focus Rectangle feature doesn't works. Check platform.\n");
	}

	return kTestPassed;
}

TestExitStatus GFXtests::overlayGraphics() {
	Testsuite::clearScreen();
	Common::String info = "Overlay Graphics. You should expect to see a green colored rectangle on the screen";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Overlay Graphics\n");
		return kTestSkipped;
	}

	Graphics::PixelFormat pf = g_system->getOverlayFormat();

	OverlayColor buffer[50 * 100];
	OverlayColor value = pf.RGBToColor(0, 255, 0);

	for (int i = 0; i < 50 * 100; i++) {
		buffer[i] = value;
	}

	g_system->showOverlay();
	g_system->copyRectToOverlay(buffer, 200, 270, 175, 100, 50);
	g_system->updateScreen();

	g_system->delayMillis(1000);

	g_system->hideOverlay();
	g_system->updateScreen();

	if (Testsuite::handleInteractiveInput("Did you see a green overlayed rectangle?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Overlay Rectangle feature doesn't works\n");
		return kTestFailed;
	}

	return kTestPassed;
}

TestExitStatus GFXtests::paletteRotation() {

	Common::String info = "Palette rotation. Here we draw a full 256 colored rainbow and then rotate it.\n"
						"Note that the screen graphics change without having to draw anything.\n"
						"The palette should appear to rotate, as a result, the background will change its color too.\n"
						"Click the mouse button to exit.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : palette Rotation\n");
		return kTestSkipped;
	}

	Testsuite::clearEntireScreen();

	// Use 256 colors
	byte palette[256 * 3] = {0};

	int r, g, b;
	r = g = b = 0;
	int colIndx;

	for (int i = 0; i < 256; i++) {
		HSVtoRGB(r, g, b, i, 1, 1);
		colIndx = i * 3;
		palette[colIndx] = r;
		palette[colIndx + 1] = g;
		palette[colIndx + 2] = b;
	}

	// Initialize this palette.
	g_system->getPaletteManager()->setPalette(palette, 0, 256);

	// Draw 256 Rectangles, each 1 pixel wide and 10 pixels long
	// one for 0-255 color range other for 0-127-255 range
	byte buffer[256 * 30] = {0};

	for (int i = 0; i < 30; i++) {
		for (int j = 0; j < 256; j++) {
			if (i < 10) {
				buffer[i * 256 + j] = j + 2;
			} else if (i < 20) {
				buffer[i * 256 + j] = 0;
			} else {
				buffer[i * 256 + j] = ((j + 127) % 256) + 2;
			}
		}
	}

	g_system->copyRectToScreen(buffer, 256, 22, 50, 256, 30);

	// Show mouse
	CursorMan.showMouse(true);
	g_system->updateScreen();


	bool toRotate = true;
	Common::Event event;

	while (toRotate) {
		while (g_system->getEventManager()->pollEvent(event)) {
			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				toRotate = false;
			}
		}

		rotatePalette(palette, 256);

		g_system->delayMillis(10);
		g_system->getPaletteManager()->setPalette(palette, 0, 256);
		g_system->updateScreen();
	}

	CursorMan.showMouse(false);
	// Reset initial palettes
	GFXTestSuite::setCustomColor(255, 0, 0);
	Testsuite::clearScreen();

	if (Testsuite::handleInteractiveInput("Did you see a rotation in colors of rectangles displayed on screen?", "Yes", "No", kOptionRight)) {
		return kTestFailed;
	}

	return kTestPassed;
}

TestExitStatus GFXtests::cursorTrails() {
	Common::String info = "With some shake offset the cursor was known to leave trails in the GUI\n"
						"Here we set some offset and ask user to check for mouse trails, \n"
						"the test is passed when there are no trails";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Cursor Trails\n");
		return kTestSkipped;
	}
	TestExitStatus passed = kTestFailed;
	g_system->setShakePos(25);
	g_system->updateScreen();
	if (Testsuite::handleInteractiveInput("Does the cursor leaves trails while moving?", "Yes", "No", kOptionRight)) {
		passed = kTestPassed;
	}
	g_system->setShakePos(0);
	g_system->updateScreen();
	return passed;
}

TestExitStatus GFXtests::pixelFormats() {
	Testsuite::clearScreen();
	Common::String info = "Testing pixel formats. Here we iterate over all the supported pixel formats and display some colors using them\n"
		"This may take long, especially if the backend supports many pixel formats";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Pixel Formats\n");
		return kTestSkipped;
	}

	Common::List<Graphics::PixelFormat> pfList = g_system->getSupportedFormats();

	int numFormatsTested = 0;
	int numPassed = 0;
	int numFailed = 0;

	Testsuite::logDetailedPrintf("Testing Pixel Formats. Size of list : %d\n", pfList.size());

	for (Common::List<Graphics::PixelFormat>::const_iterator iter = pfList.begin(); iter != pfList.end(); iter++) {
		numFormatsTested++;
		if (iter->bytesPerPixel == 1) {
			// Palettes already tested
			continue;
		} else if (iter->bytesPerPixel > 2) {
			Testsuite::logDetailedPrintf("Can't test pixels with bpp > 2\n");
			continue;
		}

		// Switch to that pixel Format
		g_system->beginGFXTransaction();
		g_system->initSize(320, 200, &(*iter));
		g_system->endGFXTransaction();
		Testsuite::clearScreen(true);

		// Draw some nice gradients
		// Pick up some colors
		uint colors[6];

		colors[0] = iter->RGBToColor(255, 255, 255);
		colors[1] = iter->RGBToColor(135, 48, 21);
		colors[2] = iter->RGBToColor(205, 190, 87);
		colors[3] = iter->RGBToColor(0, 32, 64);
		colors[4] = iter->RGBToColor(181, 126, 145);
		colors[5] = iter->RGBToColor(47, 78, 36);

		Common::Point pt(0, 170);
		Common::String msg;
		msg = Common::String::format("Testing Pixel Formats, %d of %d", numFormatsTested, pfList.size());
		Testsuite::writeOnScreen(msg, pt, true);

		// CopyRectToScreen could have been used, but that may involve writing code which
		// already resides in graphics/surface.h
		// So using Graphics::Surface

		Graphics::Surface *screen = g_system->lockScreen();

		// Draw 6 rectangles centered at (50, 160), piled over one another
		// each with color in colors[]
		for (int i = 0; i < 6; i++) {
			screen->fillRect(Common::Rect::center(160, 20 + i * 10, 100, 10), colors[i]);
		}

		g_system->unlockScreen();
		g_system->updateScreen();
		g_system->delayMillis(500);

		if (Testsuite::handleInteractiveInput("Were you able to notice the colored rectangles on the screen for this format?", "Yes", "No", kOptionLeft)) {
			numPassed++;
		} else {
			numFailed++;
			Testsuite::logDetailedPrintf("Testing pixel format failed for format #%d on the list\n", numFormatsTested);
		}
	}

	// Revert back to 8bpp
	g_system->beginGFXTransaction();
	g_system->initSize(320, 200);
	g_system->endGFXTransaction();
	GFXTestSuite::setCustomColor(255, 0, 0);
	initMousePalette();
	Testsuite::clearScreen();

	if (numFailed) {
		Testsuite::logDetailedPrintf("Pixel Format test: Failed : %d, Passed : %d, Ignored %d\n",numFailed, numPassed, numFormatsTested - (numPassed + numFailed));
		return kTestFailed;
	}

	return kTestPassed;
}

} // End of namespace Testbed
