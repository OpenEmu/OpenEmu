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

#ifndef TESTBED_MIDI_H
#define TESTBED_MIDI_H

#include "common/stream.h"
#include "audio/midiparser.h"
#include "testbed/testsuite.h"

// This file can be used as template for header files of other newer testsuites.

namespace Testbed {

namespace MidiTests {

// Helper functions for MIDI tests
bool loadMusicInMemory(Common::WriteStream *ws);
void waitForMusicToPlay(MidiParser *parser);

// will contain function declarations for MIDI tests
// add more here
TestExitStatus playMidiMusic();

} // End of namespace MIDItests

class MidiTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the XXXTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	MidiTestSuite();
	~MidiTestSuite() {}
	const char *getName() const {
		return "MIDI";
	}

	const char *getDescription() const {
		return "Midi Music";
	}

	void enable(bool flag);

private:
	bool _isMidiDataFound;

};

} // End of namespace Testbed

#endif // TESTBED_MIDI_H
