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

#include "common/config-manager.h"
#include "common/events.h"
#include "common/stream.h"

#include "graphics/fontman.h"
#include "graphics/surface.h"

#include "gui/message.h"

#include "testbed/graphics.h"
#include "testbed/testbed.h"
#include "testbed/testsuite.h"

namespace Testbed {

void Testsuite::logPrintf(const char *fmt, ...) {
	// Assuming log message size to be not greater than STRINGBUFLEN i.e 256
	char buffer[STRINGBUFLEN];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buffer, STRINGBUFLEN, fmt, vl);
	va_end(vl);
	Common::WriteStream *ws = ConfigParams::instance().getLogWriteStream();

	if (ws) {
		ws->writeString(buffer);
		ws->flush();
		debugCN(kTestbedLogOutput, "%s", buffer);
	} else {
		debugCN(kTestbedLogOutput, "%s", buffer);
	}
}

void Testsuite::logDetailedPrintf(const char *fmt, ...) {
	// Assuming log message size to be not greater than STRINGBUFLEN i.e 256
	// Messages with this function would only be displayed if -d1 is specified on command line
	char buffer[STRINGBUFLEN];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf(buffer, STRINGBUFLEN, fmt, vl);
	va_end(vl);
	Common::WriteStream *ws = ConfigParams::instance().getLogWriteStream();

	if (ws) {
		ws->writeString(buffer);
		ws->flush();
		debugCN(1, kTestbedLogOutput, "%s", buffer);
	} else {
		debugCN(1, kTestbedLogOutput, "%s", buffer);
	}
}

Testsuite::Testsuite() {
	_numTestsPassed = 0;
	_numTestsExecuted = 0;
	_numTestsSkipped = 0;
	_toQuit = kLoopNormal;
	// Initially all testsuites are enabled, disable them by calling enableTestSuite(name, false)
	_isTsEnabled = true;
	// Set custom color for progress bar
	GFXTestSuite::setCustomColor(0, 0, 0);
}

Testsuite::~Testsuite() {
	for (Common::Array<Test *>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		delete (*i);
	}
}

void Testsuite::reset() {
	_numTestsPassed = 0;
	_numTestsExecuted = 0;
	_toQuit = kLoopNormal;
	for (Common::Array<Test *>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		(*i)->passed = false;
	}
}

void Testsuite::genReport() const {
	logPrintf("\n");
	logPrintf("Consolidating results...\n");
	logPrintf("Subsystem: %s ", getName());
	logPrintf("(Tests Executed: %d)\n", _numTestsExecuted);
	logPrintf("Passed: %d ", _numTestsPassed);
	logPrintf("Skipped: %d ", _numTestsSkipped);
	logPrintf("Failed: %d\n", getNumTestsFailed());
	logPrintf("\n");
}

bool Testsuite::handleInteractiveInput(const Common::String &textToDisplay, const char *opt1, const char *opt2, OptionSelected result) {
	GUI::MessageDialog prompt(textToDisplay, opt1, opt2);
	return prompt.runModal() == result ? true : false;
}

void Testsuite::displayMessage(const Common::String &textToDisplay, const char *defaultButton) {
	GUI::MessageDialog prompt(textToDisplay, defaultButton);
	prompt.runModal();
}

Common::Rect Testsuite::writeOnScreen(const Common::String &textToDisplay, const Common::Point &pt, bool flag) {
	const Graphics::Font &font(*FontMan.getFontByUsage(Graphics::FontManager::kConsoleFont));
	uint fillColor = kColorBlack;
	uint textColor = kColorWhite;

	Graphics::Surface *screen = g_system->lockScreen();

	int height = font.getFontHeight();
	int width = screen->w;

	Common::Rect rect(pt.x, pt.y, pt.x + width, pt.y + height);

	if (flag) {
		Graphics::PixelFormat pf = g_system->getScreenFormat();
		fillColor = pf.RGBToColor(0, 0, 0);
		textColor = pf.RGBToColor(255, 255, 255);
	}

	screen->fillRect(rect, fillColor);
	font.drawString(screen, textToDisplay, rect.left, rect.top, screen->w, textColor, Graphics::kTextAlignCenter);

	g_system->unlockScreen();
	g_system->updateScreen();

	return rect;
}

void Testsuite::clearScreen(const Common::Rect &rect) {
	Graphics::Surface *screen = g_system->lockScreen();

	screen->fillRect(rect, kColorBlack);

	g_system->unlockScreen();
	g_system->updateScreen();
}

void Testsuite::clearScreen() {
	int numBytesPerLine = g_system->getWidth() * g_system->getScreenFormat().bytesPerPixel;
	int height = getDisplayRegionCoordinates().y;

	// Don't clear test info display region
	int size =  height * numBytesPerLine;
	byte *buffer = new byte[size];
	memset(buffer, 0, size);
	g_system->copyRectToScreen(buffer, numBytesPerLine, 0, 0, g_system->getWidth(), height);
	g_system->updateScreen();
	delete[] buffer;
}

void Testsuite::clearScreen(bool flag) {
	Graphics::Surface *screen = g_system->lockScreen();
	uint fillColor = kColorBlack;

	if (flag) {
		fillColor = g_system->getScreenFormat().RGBToColor(0, 0, 0);
	}

	screen->fillRect(Common::Rect(0, 0, g_system->getWidth(), g_system->getHeight()), fillColor);

	g_system->unlockScreen();
	g_system->updateScreen();
}

void Testsuite::addTest(const Common::String &name, InvokingFunction f, bool isInteractive) {
	Test *featureTest = new Test(name, f, isInteractive);
	_testsToExecute.push_back(featureTest);
}

int Testsuite::getNumTestsEnabled() {
	int count = 0;
	Common::Array<Test *>::const_iterator iter;

	if (!isEnabled()) {
		return 0;
	}

	for (iter = _testsToExecute.begin(); iter != _testsToExecute.end(); iter++) {
		if ((*iter)->enabled) {
			count++;
		}
	}
	return count;
}

uint Testsuite::parseEvents() {
	uint startTime = g_system->getMillis();
	uint end = startTime + kEventHandlingTime;
	do {
		Common::Event ev;
		while (g_system->getEventManager()->pollEvent(ev)) {
			switch (ev.type) {
			case Common::EVENT_KEYDOWN:
				if (ev.kbd.keycode == Common::KEYCODE_ESCAPE) {
					return kSkipNext;
				}
				break;

			case Common::EVENT_QUIT:
			case Common::EVENT_RTL:
				return kEngineQuit;

			default:
				break;
			}
		}
		g_system->delayMillis(10);
		startTime = g_system->getMillis();
	} while (startTime <= end);

	return kLoopNormal;
}

void Testsuite::updateStats(const char *prefix, const char *info, uint testNum, uint numTests, Common::Point pt) {
	Common::String text = Common::String::format(" Running %s: %s (%d of %d) ", prefix, info, testNum, numTests);
	writeOnScreen(text, pt);
	uint barColor = kColorSpecial;
	// below the text a rectangle denoting the progress in the testsuite can be drawn.
	int separation = getLineSeparation();
	pt.y += separation;
	int wRect = 200;
	int lRect = 7;
	pt.x = g_system->getWidth() / 2 - 100;
	byte *buffer = new byte[lRect * wRect];
	memset(buffer, 0, sizeof(byte) * lRect * wRect);

	int wShaded = (int) (wRect * (((float)testNum) / numTests));

	// draw the boundary
	memset(buffer, barColor, sizeof(byte) * wRect);
	memset(buffer + (wRect * (lRect - 1)) , barColor, sizeof(byte) * wRect);

	for (int i = 0; i < lRect; i++) {
		for (int j = 0; j < wRect; j++) {
			if (j < wShaded) {
				buffer[i * wRect + j] = barColor;
			}
		}
		buffer[i * wRect + 0] = barColor;
		buffer[i * wRect + wRect - 1] = barColor;
	}
	g_system->copyRectToScreen(buffer, wRect, pt.x, pt.y, wRect, lRect);
	g_system->updateScreen();
	delete[] buffer;
}

bool Testsuite::enableTest(const Common::String &testName, bool toEnable) {
	for (uint i = 0; i < _testsToExecute.size(); i++) {
		if (_testsToExecute[i]->featureName.equalsIgnoreCase(testName)) {
			_testsToExecute[i]->enabled = toEnable;
			return true;
		}
	}
	return false;
}


void Testsuite::execute() {
	// Main Loop for a testsuite

	uint count = 0;
	Common::Point pt = getDisplayRegionCoordinates();
	pt.y += getLineSeparation();
	int numEnabledTests = getNumTestsEnabled();

	for (Common::Array<Test *>::iterator i = _testsToExecute.begin(); i != _testsToExecute.end(); ++i) {
		if (!(*i)->enabled) {
			logPrintf("Info! Skipping Test: %s, Skipped by configuration.\n", ((*i)->featureName).c_str());
			_numTestsSkipped++;
			continue;
		}

		if ((*i)->isInteractive && !ConfParams.isSessionInteractive()) {
			logPrintf("Info! Skipping Test: %s, non-interactive environment is selected\n", ((*i)->featureName).c_str());
			_numTestsSkipped++;
			continue;
		}

		logPrintf("Info! Executing Test: %s\n", ((*i)->featureName).c_str());
		updateStats("Test", ((*i)->featureName).c_str(), count++, numEnabledTests, pt);

		// Run the test and capture exit status.
		TestExitStatus eStatus = (*i)->driver();
		if (kTestPassed == eStatus) {
			logPrintf("Result: Passed\n");
			_numTestsExecuted++;
			_numTestsPassed++;
		} else if (kTestSkipped == eStatus){
			logPrintf("Result: Skipped\n");
			_numTestsSkipped++;
		} else {
			_numTestsExecuted++;
			logPrintf("Result: Failed\n");
		}

		updateStats("Test", ((*i)->featureName).c_str(), count, numEnabledTests, pt);
		// TODO: Display a screen here to user with details of upcoming test, he can skip it or Quit or RTL
		// Check if user wants to quit/RTL/Skip next test by parsing events.
		// Quit directly if explicitly requested

		if (Engine::shouldQuit()) {
			_toQuit = kEngineQuit;
			genReport();
			return;
		}

		_toQuit = parseEvents();
	}
	genReport();
}

} // End of namespace Testebed
