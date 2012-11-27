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

#include "audio/softsynth/pcspk.h"

#include "backends/audiocd/audiocd.h"

#include "common/config-manager.h"

#include "testbed/sound.h"

namespace Testbed {

enum {
	kPlayChannel1 = 'pch1',
	kPlayChannel2 = 'pch2',
	kPlayChannel3 = 'pch3',
	kPauseChannel1 = 'pac1',
	kPauseChannel2 = 'pac2',
	kPauseChannel3 = 'pac3'
};

SoundSubsystemDialog::SoundSubsystemDialog() : TestbedInteractionDialog(80, 60, 400, 170) {
	_xOffset = 25;
	_yOffset = 0;
	Common::String text = "Sound Subsystem Tests: Test Mixing of Audio Streams.";
	addText(350, 20, text, Graphics::kTextAlignCenter, _xOffset, 15);
	addButton(200, 20, "Play Channel #1", kPlayChannel1);
	addButton(200, 20, "Play Channel #2", kPlayChannel2);
	addButton(200, 20, "Play Channel #3", kPlayChannel3);
	addButton(50, 20, "Close", GUI::kCloseCmd, 160, 15);

	_mixer = g_system->getMixer();

	// the three streams to be mixed
	Audio::PCSpeaker *s1 = new Audio::PCSpeaker();
	Audio::PCSpeaker *s2 = new Audio::PCSpeaker();
	Audio::PCSpeaker *s3 = new Audio::PCSpeaker();

	s1->play(Audio::PCSpeaker::kWaveFormSine, 1000, -1);
	s2->play(Audio::PCSpeaker::kWaveFormSine, 1200, -1);
	s3->play(Audio::PCSpeaker::kWaveFormSine, 1400, -1);

	_mixer->playStream(Audio::Mixer::kPlainSoundType, &_h1, s1);
	_mixer->pauseHandle(_h1, true);

	_mixer->playStream(Audio::Mixer::kSpeechSoundType, &_h2, s2);
	_mixer->pauseHandle(_h2, true);

	_mixer->playStream(Audio::Mixer::kSFXSoundType, &_h3, s3);
	_mixer->pauseHandle(_h3, true);

}


void SoundSubsystemDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	switch (cmd) {
		case kPlayChannel1:
			_buttonArray[0]->setLabel("Pause Channel #1");
			_buttonArray[0]->setCmd(kPauseChannel1);
			_mixer->pauseHandle(_h1, false);
			break;
		case kPlayChannel2:
			_buttonArray[1]->setLabel("Pause Channel #2");
			_buttonArray[1]->setCmd(kPauseChannel2);
			_mixer->pauseHandle(_h2, false);
			break;
		case kPlayChannel3:
			_buttonArray[2]->setLabel("Pause Channel #3");
			_buttonArray[2]->setCmd(kPauseChannel3);
			_mixer->pauseHandle(_h3, false);
			break;
		case kPauseChannel1:
			_buttonArray[0]->setLabel("Play Channel #1");
			_buttonArray[0]->setCmd(kPlayChannel1);
			_mixer->pauseHandle(_h1, true);
			break;
		case kPauseChannel2:
			_buttonArray[1]->setLabel("Play Channel #2");
			_buttonArray[1]->setCmd(kPlayChannel2);
			_mixer->pauseHandle(_h2, true);
			break;
		case kPauseChannel3:
			_buttonArray[2]->setLabel("Play Channel #3");
			_buttonArray[2]->setCmd(kPlayChannel3);
			_mixer->pauseHandle(_h3, true);
			break;
		default:
			_mixer->stopAll();
			GUI::Dialog::handleCommand(sender, cmd, data);
	}
}

TestExitStatus SoundSubsystem::playBeeps() {
	Testsuite::clearScreen();
	TestExitStatus passed = kTestPassed;
	Common::String info = "Testing Sound Output by generating beeps\n"
	"You should hear a left beep followed by a right beep\n";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Play Beeps\n");
		return kTestSkipped;
	}

	Audio::PCSpeaker *speaker = new Audio::PCSpeaker();
	Audio::Mixer *mixer = g_system->getMixer();
	Audio::SoundHandle handle;
	mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, speaker);

	// Left Beep
	Testsuite::writeOnScreen("Left Beep", Common::Point(0, 100));
	mixer->setChannelBalance(handle, -127);
	speaker->play(Audio::PCSpeaker::kWaveFormSine, 1000, -1);
	g_system->delayMillis(500);
	mixer->pauseHandle(handle, true);

	if (Testsuite::handleInteractiveInput("  Were you able to hear the left beep?  ", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Left Beep couldn't be detected : Error with Mixer::setChannelBalance()\n");
		passed = kTestFailed;
	}

	// Right Beep
	Testsuite::writeOnScreen("Right Beep", Common::Point(0, 100));
	mixer->setChannelBalance(handle, 127);
	mixer->pauseHandle(handle, false);
	g_system->delayMillis(500);
	mixer->stopAll();

	if (Testsuite::handleInteractiveInput("Were you able to hear the right beep?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Right Beep couldn't be detected : Error with Mixer::setChannelBalance()\n");
		passed = kTestFailed;
	}
	return passed;
}

TestExitStatus SoundSubsystem::mixSounds() {
	Testsuite::clearScreen();
	TestExitStatus passed = kTestPassed;
	Common::String info = "Testing Mixer Output by generating multichannel sound output using PC speaker emulator.\n"
	"The mixer should be able to play them simultaneously\n";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Mix Sounds\n");
		return kTestSkipped;
	}

	SoundSubsystemDialog sDialog;
	sDialog.runModal();
	if (Testsuite::handleInteractiveInput("Was the mixer able to simultaneously play multiple channels?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Multiple channels couldn't be played : Error with Mixer Class\n");
		passed = kTestFailed;
	}
	return passed;
}

TestExitStatus SoundSubsystem::audiocdOutput() {
	Testsuite::clearScreen();
	TestExitStatus passed = kTestPassed;
	Common::String info = "Testing AudioCD API implementation.\n"
	"Here we have four tracks, we play them in order i.e 1-2-3-last.\n"
	"The user should verify if the tracks were run in correct order or not.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : AudioCD API\n");
		return kTestSkipped;
	}

	Common::Point pt(0, 100);
	Testsuite::writeOnScreen("Playing the tracks of testCD in order i.e 1-2-3-last", pt);


	// Play all tracks
	for (int i = 1; i < 5; i++) {
		g_system->getAudioCDManager()->play(i, 1, 0, 0);
		while (g_system->getAudioCDManager()->isPlaying()) {
			g_system->delayMillis(500);
			Testsuite::writeOnScreen(Common::String::format("Playing Now: track%02d", i), pt);
		}
		g_system->delayMillis(500);
	}

	Testsuite::clearScreen();
	if (Testsuite::handleInteractiveInput("Were all the tracks played in order i.e 1-2-3-last ?", "Yes", "No", kOptionRight)) {
		Testsuite::logPrintf("Error! Error in _system->getAudioCDManager()->play() or probably sound files were not detected, try -d1 (debuglevel 1)\n");
		passed = kTestFailed;
	}

	return passed;
}

TestExitStatus SoundSubsystem::sampleRates() {

	Common::String info = "Testing Multiple Sample Rates.\n"
						  "Here we try to play sounds at three different sample rates.";

	if (Testsuite::handleInteractiveInput(info, "OK", "Skip", kOptionRight)) {
		Testsuite::logPrintf("Info! Skipping test : Sample Rates\n");
		return kTestSkipped;
	}

	TestExitStatus passed = kTestPassed;
	Audio::Mixer *mixer = g_system->getMixer();

	Audio::PCSpeaker *s1 = new Audio::PCSpeaker();
	// Stream at half sampling rate
	Audio::PCSpeaker *s2 = new Audio::PCSpeaker(s1->getRate() - 10000);
	// Stream at twice sampling rate
	Audio::PCSpeaker *s3 = new Audio::PCSpeaker(s1->getRate() + 10000);

	s1->play(Audio::PCSpeaker::kWaveFormSine, 1000, -1);
	s2->play(Audio::PCSpeaker::kWaveFormSine, 1000, -1);
	s3->play(Audio::PCSpeaker::kWaveFormSine, 1000, -1);

	Audio::SoundHandle handle;
	Common::Point pt(0, 100);

	mixer->playStream(Audio::Mixer::kPlainSoundType, &handle, s1);
	Testsuite::writeOnScreen(Common::String::format("Playing at smaple rate: %d", s1->getRate()), pt);
	g_system->delayMillis(1000);
	mixer->stopHandle(handle);
	g_system->delayMillis(1000);

	mixer->playStream(Audio::Mixer::kSpeechSoundType, &handle, s2);
	Testsuite::writeOnScreen(Common::String::format("Playing at sample rate : %d", s2->getRate()), pt);
	g_system->delayMillis(1000);
	mixer->stopHandle(handle);
	g_system->delayMillis(1000);

	mixer->playStream(Audio::Mixer::kSFXSoundType, &handle, s3);
	Testsuite::writeOnScreen(Common::String::format("Playing at sample rate : %d", s3->getRate()), pt);
	g_system->delayMillis(1000);
	mixer->stopHandle(handle);
	g_system->delayMillis(1000);

	Testsuite::clearScreen();
	if (Testsuite::handleInteractiveInput("Was the mixer able to play beeps with variable sample rates?", "Yes", "No", kOptionRight)) {
		Testsuite::logDetailedPrintf("Error! Error with variable sample rates\n");
		passed = kTestFailed;
	}

	return passed;
}

SoundSubsystemTestSuite::SoundSubsystemTestSuite() {
	addTest("SimpleBeeps", &SoundSubsystem::playBeeps, true);
	addTest("MixSounds", &SoundSubsystem::mixSounds, true);

	// Make audio-files discoverable
	Common::FSNode gameRoot(ConfMan.get("path"));
	if (gameRoot.exists()) {
		SearchMan.addSubDirectoryMatching(gameRoot, "audiocd-files");
		if (SearchMan.hasFile("track01.mp3") && SearchMan.hasFile("track02.mp3") && SearchMan.hasFile("track03.mp3") && SearchMan.hasFile("track04.mp3")) {
			addTest("AudiocdOutput", &SoundSubsystem::audiocdOutput, true);
		} else {
			Testsuite::logPrintf("Warning! Skipping test AudioCD: Required data files missing, check game-dir/audiocd-files\n");
		}
	}
	addTest("SampleRates", &SoundSubsystem::sampleRates, true);
}

}	// End of namespace Testbed
