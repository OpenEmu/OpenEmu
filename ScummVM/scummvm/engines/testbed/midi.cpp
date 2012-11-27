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

#include "common/archive.h"
#include "common/events.h"
#include "common/memstream.h"

#include "graphics/cursorman.h"

#include "audio/mididrv.h"

#include "testbed/midi.h"
#include "testbed/testbed.h"

namespace Testbed {

bool MidiTests::loadMusicInMemory(Common::WriteStream *ws) {
	Common::SeekableReadStream *midiFile = SearchMan.createReadStreamForMember("music.mid");
	if (!midiFile) {
		Testsuite::logPrintf("Error! Can't open Midi music file, check game data directory for file music.mid\n");
		return false;
	}

	while (!midiFile->eos()) {
		byte data = midiFile->readByte();
		ws->writeByte(data);
	}
	return true;
}

void MidiTests::waitForMusicToPlay(MidiParser *parser) {
	Common::EventManager *eventMan = g_system->getEventManager();
	bool quitLoop = false;
	Common::Event event;

	CursorMan.showMouse(true);
	while (!quitLoop) {
		while (eventMan->pollEvent(event)) {
			// Quit if explicitly requested!
			if (Engine::shouldQuit()) {
				return;
			}

			if (event.type == Common::EVENT_LBUTTONDOWN || event.type == Common::EVENT_RBUTTONDOWN) {
				quitLoop = true;
			} else {
				Testsuite::writeOnScreen("Playing Midi Music, Click to end", Common::Point(0, 100));
				if (!parser->isPlaying()) {
					quitLoop = true;
				}
			}
		}
	}
	CursorMan.showMouse(false);
	return;
}

TestExitStatus MidiTests::playMidiMusic() {
	Testsuite::clearScreen();
	Common::String info = "Testing Midi Sound output.\n"
						  "Here, We generate some Music by using the Midi Driver selected in the GUI.\n"
						  "You should expect to hear that. The initialization may take some time.\n";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Play Midi Music\n");
		return kTestSkipped;
	}

	MidiDriver::DeviceHandle dev = MidiDriver::detectDevice(MDT_MIDI | MDT_ADLIB);
	// Create a driver instance
	MidiDriver *driver = MidiDriver::createMidi(dev);
	// Create a SMF parser
	MidiParser *smfParser = MidiParser::createParser_SMF();
	// Open the driver
	int errCode = driver->open();

	if (errCode) {
		Common::String errMsg = MidiDriver::getErrorName(errCode);
		Testsuite::writeOnScreen(errMsg, Common::Point(0, 100));
		Testsuite::logPrintf("Error! %s", errMsg.c_str());
		return kTestFailed;
	}

	Testsuite::logDetailedPrintf("Info! Midi: Succesfully opened the driver\n");

	Common::MemoryWriteStreamDynamic ws(DisposeAfterUse::YES);
	loadMusicInMemory(&ws);

	// start playing
	if (smfParser->loadMusic(ws.getData(), ws.size())) {
		smfParser->setTrack(0);
		smfParser->setMidiDriver(driver);
		smfParser->setTimerRate(driver->getBaseTempo());
		driver->setTimerCallback(smfParser, MidiParser::timerCallback);
		Testsuite::logDetailedPrintf("Info! Midi: Parser Successfully loaded Music data.\n");
		if (smfParser->isPlaying()) {
			Testsuite::writeOnScreen("Playing Midi Music, Click to end.", Common::Point(0, 100));
			Testsuite::logDetailedPrintf("Info! Midi: Playing music!\n");
		}
	}


	// Play until track ends or an exit is requested.
	waitForMusicToPlay(smfParser);

	// Done. Clean up.
	smfParser->unloadMusic();
	driver->setTimerCallback(NULL, NULL);
	driver->close();
	delete smfParser;
	delete driver;

	if (Testsuite::handleInteractiveInput("Were you able to hear the music as described?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Midi: Can't play Music\n");
		return kTestFailed;
	}
	return kTestPassed;
}

MidiTestSuite::MidiTestSuite() {
	addTest("MidiTests", &MidiTests::playMidiMusic);
	_isMidiDataFound = true;
	if (!SearchMan.hasFile("music.mid")) {
		// add some fallback test if filesystem loading failed
		Testsuite::logPrintf("Warning! Midi: Sound data file music.mid not found\n");
		_isMidiDataFound = false;
		MidiTestSuite::enable(false);
	}
}

void MidiTestSuite::enable(bool flag) {
	Testsuite::enable(_isMidiDataFound ? flag : false);
}

}
