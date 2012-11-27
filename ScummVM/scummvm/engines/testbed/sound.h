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

#ifndef TESTBED_SOUND_H
#define TESTBED_SOUND_H

#include "gui/dialog.h"
#include "audio/mixer.h"
#include "testbed/config.h"
#include "testbed/testsuite.h"

namespace Testbed {

class SoundSubsystemDialog : public TestbedInteractionDialog {
public:
	SoundSubsystemDialog();
	~SoundSubsystemDialog() {}
	void handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data);
	Audio::Mixer *_mixer;
	Audio::SoundHandle _h1, _h2, _h3;
};

namespace SoundSubsystem {

// Helper functions for SoundSubsystem tests

// will contain function declarations for SoundSubsystem tests
TestExitStatus playBeeps();
TestExitStatus mixSounds();
TestExitStatus audiocdOutput();
TestExitStatus sampleRates();
}

class SoundSubsystemTestSuite : public Testsuite {
public:
	/**
	 * The constructor for the SoundSubsystemTestSuite
	 * For every test to be executed one must:
	 * 1) Create a function that would invoke the test
	 * 2) Add that test to list by executing addTest()
	 *
	 * @see addTest()
	 */
	SoundSubsystemTestSuite();
	~SoundSubsystemTestSuite() {}

	const char *getName() const {
		return "SoundSubsystem";
	}

	const char *getDescription() const {
		return "Sound Subsystem";
	}
};

} // End of namespace Testbed

#endif // TESTBED_SOUND_H
