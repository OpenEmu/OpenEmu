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

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "sci/sci.h"
#include "sci/engine/features.h"
#include "sci/engine/state.h"
#include "sci/engine/kernel.h"
#include "sci/engine/vm.h"		// for Object
#include "sci/sound/audio.h"
#include "sci/sound/soundcmd.h"

#include "audio/mixer.h"
#include "common/system.h"

namespace Sci {

/**
 * Used for synthesized music playback
 */
reg_t kDoSound(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, g_sci->_features->detectDoSoundType());
	error("not supposed to call this");
}

#define CREATE_DOSOUND_FORWARD(_name_) reg_t k##_name_(EngineState *s, int argc, reg_t *argv) { return g_sci->_soundCmd->k##_name_(argc, argv, s->r_acc); }

CREATE_DOSOUND_FORWARD(DoSoundInit)
CREATE_DOSOUND_FORWARD(DoSoundPlay)
CREATE_DOSOUND_FORWARD(DoSoundRestore)
CREATE_DOSOUND_FORWARD(DoSoundDispose)
CREATE_DOSOUND_FORWARD(DoSoundMute)
CREATE_DOSOUND_FORWARD(DoSoundStop)
CREATE_DOSOUND_FORWARD(DoSoundStopAll)
CREATE_DOSOUND_FORWARD(DoSoundPause)
CREATE_DOSOUND_FORWARD(DoSoundResumeAfterRestore)
CREATE_DOSOUND_FORWARD(DoSoundMasterVolume)
CREATE_DOSOUND_FORWARD(DoSoundUpdate)
CREATE_DOSOUND_FORWARD(DoSoundFade)
CREATE_DOSOUND_FORWARD(DoSoundGetPolyphony)
CREATE_DOSOUND_FORWARD(DoSoundUpdateCues)
CREATE_DOSOUND_FORWARD(DoSoundSendMidi)
CREATE_DOSOUND_FORWARD(DoSoundGlobalReverb)
CREATE_DOSOUND_FORWARD(DoSoundSetHold)
CREATE_DOSOUND_FORWARD(DoSoundDummy)
CREATE_DOSOUND_FORWARD(DoSoundGetAudioCapability)
CREATE_DOSOUND_FORWARD(DoSoundSuspend)
CREATE_DOSOUND_FORWARD(DoSoundSetVolume)
CREATE_DOSOUND_FORWARD(DoSoundSetPriority)
CREATE_DOSOUND_FORWARD(DoSoundSetLoop)

reg_t kDoCdAudio(EngineState *s, int argc, reg_t *argv) {
	switch (argv[0].toUint16()) {
	case kSciAudioPlay: {
		if (argc < 2)
			return NULL_REG;

		uint16 track = argv[1].toUint16();
		uint32 startFrame = (argc > 2) ? argv[2].toUint16() * 75 : 0;
		uint32 totalFrames = (argc > 3) ? argv[3].toUint16() * 75 : 0;

		return make_reg(0, g_sci->_audio->audioCdPlay(track, startFrame, totalFrames));
	}
	case kSciAudioStop:
		g_sci->_audio->audioCdStop();

		if (getSciVersion() == SCI_VERSION_1_1)
			return make_reg(0, 1);

		break;
	case kSciAudioPause:
		warning("Can't pause CD Audio");
		break;
	case kSciAudioResume:
		// This seems to be hacked up to update the CD instead of resuming
		// audio like kDoAudio does.
		g_sci->_audio->audioCdUpdate();
		break;
	case kSciAudioPosition:
		return make_reg(0, g_sci->_audio->audioCdPosition());
	case kSciAudioWPlay: // CD Audio can't be preloaded
	case kSciAudioRate: // No need to set the audio rate
	case kSciAudioVolume: // The speech setting isn't used by CD Audio
	case kSciAudioLanguage: // No need to set the language
		break;
	case kSciAudioCD:
		// Init
		return make_reg(0, 1);
	default:
		error("kCdDoAudio: Unhandled case %d", argv[0].toUint16());
	}

	return s->r_acc;
}

/**
 * Used for speech playback and digital soundtracks in CD games
 */
reg_t kDoAudio(EngineState *s, int argc, reg_t *argv) {
	// JonesCD uses different functions based on the cdaudio.map file
	// to use red book tracks.
	if (g_sci->_features->usesCdTrack())
		return kDoCdAudio(s, argc, argv);

	Audio::Mixer *mixer = g_system->getMixer();

	switch (argv[0].toUint16()) {
	case kSciAudioWPlay:
	case kSciAudioPlay: {
		uint16 module;
		uint32 number;

		g_sci->_audio->stopAudio();

		if (argc == 2) {
			module = 65535;
			number = argv[1].toUint16();
		} else if (argc == 6 || argc == 8) {
			module = argv[1].toUint16();
			number = ((argv[2].toUint16() & 0xff) << 24) |
			         ((argv[3].toUint16() & 0xff) << 16) |
			         ((argv[4].toUint16() & 0xff) <<  8) |
			          (argv[5].toUint16() & 0xff);
			// Removed warning because of the high amount of console spam
			/*if (argc == 8) {
				// TODO: Handle the extra 2 SCI21 params
				// argv[6] is always 1
				// argv[7] is the contents of global 229 (0xE5)
				warning("kDoAudio: Play called with SCI2.1 extra parameters: %04x:%04x and %04x:%04x",
						PRINT_REG(argv[6]), PRINT_REG(argv[7]));
			}*/
		} else {
			warning("kDoAudio: Play called with an unknown number of parameters (%d)", argc);
			return NULL_REG;
		}

		debugC(kDebugLevelSound, "kDoAudio: play sample %d, module %d", number, module);

		// return sample length in ticks
		if (argv[0].toUint16() == kSciAudioWPlay)
			return make_reg(0, g_sci->_audio->wPlayAudio(module, number));
		else
			return make_reg(0, g_sci->_audio->startAudio(module, number));
	}
	case kSciAudioStop:
		debugC(kDebugLevelSound, "kDoAudio: stop");
		g_sci->_audio->stopAudio();
		break;
	case kSciAudioPause:
		debugC(kDebugLevelSound, "kDoAudio: pause");
		g_sci->_audio->pauseAudio();
		break;
	case kSciAudioResume:
		debugC(kDebugLevelSound, "kDoAudio: resume");
		g_sci->_audio->resumeAudio();
		break;
	case kSciAudioPosition:
		//debugC(kDebugLevelSound, "kDoAudio: get position");	// too verbose
		return make_reg(0, g_sci->_audio->getAudioPosition());
	case kSciAudioRate:
		debugC(kDebugLevelSound, "kDoAudio: set audio rate to %d", argv[1].toUint16());
		g_sci->_audio->setAudioRate(argv[1].toUint16());
		break;
	case kSciAudioVolume: {
		int16 volume = argv[1].toUint16();
		volume = CLIP<int16>(volume, 0, AUDIO_VOLUME_MAX);
		debugC(kDebugLevelSound, "kDoAudio: set volume to %d", volume);
#ifdef ENABLE_SCI32
		if (getSciVersion() >= SCI_VERSION_2_1) {
			int16 volumePrev = mixer->getVolumeForSoundType(Audio::Mixer::kSpeechSoundType) / 2;
			volumePrev = CLIP<int16>(volumePrev, 0, AUDIO_VOLUME_MAX);
			mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume * 2);
			return make_reg(0, volumePrev);
		} else
#endif
		mixer->setVolumeForSoundType(Audio::Mixer::kSpeechSoundType, volume * 2);
	}
	case kSciAudioLanguage:
		// In SCI1.1: tests for digital audio support
		if (getSciVersion() == SCI_VERSION_1_1) {
			debugC(kDebugLevelSound, "kDoAudio: audio capability test");
			return make_reg(0, 1);
		} else {
			int16 language = argv[1].toSint16();

			// athrxx: It seems from disasm that the original KQ5 FM-Towns loads a default language (Japanese) audio map at the beginning
			// right after loading the video and audio drivers. The -1 language argument in here simply means that the original will stick
			// with Japanese. Instead of doing that we switch to the language selected in the launcher.
			if (g_sci->getPlatform() == Common::kPlatformFMTowns && language == -1)
				language = (g_sci->getLanguage() == Common::JA_JPN) ? K_LANG_JAPANESE : K_LANG_ENGLISH;

			debugC(kDebugLevelSound, "kDoAudio: set language to %d", language);

			if (language != -1)
				g_sci->getResMan()->setAudioLanguage(language);

			kLanguage kLang = g_sci->getSciLanguage();
			g_sci->setSciLanguage(kLang);

			return make_reg(0, kLang);
		}
		break;
	case kSciAudioCD:

		if (getSciVersion() <= SCI_VERSION_1_1) {
			debugC(kDebugLevelSound, "kDoAudio: CD audio subop");
			return kDoCdAudio(s, argc - 1, argv + 1);
#ifdef ENABLE_SCI32
		} else {
			// TODO: This isn't CD Audio in SCI32 anymore
			warning("kDoAudio: Unhandled case 10, %d extra arguments passed", argc - 1);
			break;
#endif
		}

		// 3 new subops in Pharkas. kDoAudio in Pharkas sits at seg026:038C
	case 11:
		// Not sure where this is used yet
		warning("kDoAudio: Unhandled case 11, %d extra arguments passed", argc - 1);
		break;
	case 12:
		// Seems to be some sort of audio sync, used in Pharkas. Silenced the
		// warning due to the high level of spam it produces. (takes no params)
		//warning("kDoAudio: Unhandled case 12, %d extra arguments passed", argc - 1);
		break;
	case 13:
		// Used in Pharkas whenever a speech sample starts (takes no params)
		//warning("kDoAudio: Unhandled case 13, %d extra arguments passed", argc - 1);
		break;
	case 17:
		// Seems to be some sort of audio sync, used in SQ6. Silenced the
		// warning due to the high level of spam it produces. (takes no params)
		//warning("kDoAudio: Unhandled case 17, %d extra arguments passed", argc - 1);
		break;
	default:
		warning("kDoAudio: Unhandled case %d, %d extra arguments passed", argv[0].toUint16(), argc - 1);
	}

	return s->r_acc;
}

reg_t kDoSync(EngineState *s, int argc, reg_t *argv) {
	SegManager *segMan = s->_segMan;
	switch (argv[0].toUint16()) {
	case kSciAudioSyncStart: {
		ResourceId id;

		g_sci->_audio->stopSoundSync();

		// Load sound sync resource and lock it
		if (argc == 3) {
			id = ResourceId(kResourceTypeSync, argv[2].toUint16());
		} else if (argc == 7) {
			id = ResourceId(kResourceTypeSync36, argv[2].toUint16(), argv[3].toUint16(), argv[4].toUint16(),
							argv[5].toUint16(), argv[6].toUint16());
		} else {
			warning("kDoSync: Start called with an unknown number of parameters (%d)", argc);
			return s->r_acc;
		}

		g_sci->_audio->setSoundSync(id, argv[1], segMan);
		break;
	}
	case kSciAudioSyncNext:
		g_sci->_audio->doSoundSync(argv[1], segMan);
		break;
	case kSciAudioSyncStop:
		g_sci->_audio->stopSoundSync();
		break;
	default:
		error("DoSync: Unhandled subfunction %d", argv[0].toUint16());
	}

	return s->r_acc;
}

#ifdef ENABLE_SCI32

reg_t kSetLanguage(EngineState *s, int argc, reg_t *argv) {
	// This is used by script 90 of MUMG Deluxe from the main menu to toggle
	// the audio language between English and Spanish.
	// Basically, it instructs the interpreter to switch the audio resources
	// (resource.aud and associated map files) and load them from the "Spanish"
	// subdirectory instead.
	Common::String audioDirectory = s->_segMan->getString(argv[0]);
	//warning("SetLanguage: set audio resource directory to '%s'", audioDirectory.c_str());
	g_sci->getResMan()->changeAudioDirectory(audioDirectory);

	return s->r_acc;
}

#endif

} // End of namespace Sci
