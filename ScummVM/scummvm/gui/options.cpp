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

#include "gui/browser.h"
#include "gui/themebrowser.h"
#include "gui/message.h"
#include "gui/gui-manager.h"
#include "gui/options.h"
#include "gui/widgets/popup.h"
#include "gui/widgets/tab.h"
#include "gui/ThemeEval.h"

#include "common/fs.h"
#include "common/config-manager.h"
#include "common/gui_options.h"
#include "common/rendermode.h"
#include "common/system.h"
#include "common/textconsole.h"
#include "common/translation.h"

#include "audio/mididrv.h"
#include "audio/musicplugin.h"
#include "audio/mixer.h"
#include "audio/fmopl.h"

namespace GUI {

enum {
	kMidiGainChanged		= 'mgch',
	kMusicVolumeChanged		= 'muvc',
	kSfxVolumeChanged		= 'sfvc',
	kMuteAllChanged			= 'mute',
	kSubtitleToggle			= 'sttg',
	kSubtitleSpeedChanged	= 'stsc',
	kSpeechVolumeChanged	= 'vcvc',
	kChooseSoundFontCmd		= 'chsf',
	kClearSoundFontCmd      = 'clsf',
	kChooseSaveDirCmd		= 'chos',
	kSavePathClearCmd		= 'clsp',
	kChooseThemeDirCmd		= 'chth',
	kThemePathClearCmd		= 'clth',
	kChooseExtraDirCmd		= 'chex',
	kExtraPathClearCmd		= 'clex',
	kChoosePluginsDirCmd	= 'chpl',
	kChooseThemeCmd			= 'chtf'
};

enum {
	kSubtitlesSpeech,
	kSubtitlesSubs,
	kSubtitlesBoth
};

#ifdef SMALL_SCREEN_DEVICE
enum {
	kChooseKeyMappingCmd    = 'chma'
};
#endif

static const char *savePeriodLabels[] = { _s("Never"), _s("every 5 mins"), _s("every 10 mins"), _s("every 15 mins"), _s("every 30 mins"), 0 };
static const int savePeriodValues[] = { 0, 5 * 60, 10 * 60, 15 * 60, 30 * 60, -1 };
static const char *outputRateLabels[] = { _s("<default>"), _s("8 kHz"), _s("11kHz"), _s("22 kHz"), _s("44 kHz"), _s("48 kHz"), 0 };
static const int outputRateValues[] = { 0, 8000, 11025, 22050, 44100, 48000, -1 };

OptionsDialog::OptionsDialog(const Common::String &domain, int x, int y, int w, int h)
	: Dialog(x, y, w, h), _domain(domain), _graphicsTabId(-1), _midiTabId(-1), _pathsTabId(-1), _tabWidget(0) {
	init();
}

OptionsDialog::OptionsDialog(const Common::String &domain, const Common::String &name)
	: Dialog(name), _domain(domain), _graphicsTabId(-1), _tabWidget(0) {
	init();
}

OptionsDialog::~OptionsDialog() {
	delete _subToggleGroup;
}

void OptionsDialog::init() {
	_enableGraphicSettings = false;
	_gfxPopUp = 0;
	_gfxPopUpDesc = 0;
	_renderModePopUp = 0;
	_renderModePopUpDesc = 0;
	_fullscreenCheckbox = 0;
	_aspectCheckbox = 0;
	_enableAudioSettings = false;
	_midiPopUp = 0;
	_midiPopUpDesc = 0;
	_oplPopUp = 0;
	_oplPopUpDesc = 0;
	_outputRatePopUp = 0;
	_outputRatePopUpDesc = 0;
	_enableMIDISettings = false;
	_gmDevicePopUp = 0;
	_gmDevicePopUpDesc = 0;
	_soundFont = 0;
	_soundFontButton = 0;
	_soundFontClearButton = 0;
	_multiMidiCheckbox = 0;
	_midiGainDesc = 0;
	_midiGainSlider = 0;
	_midiGainLabel = 0;
	_enableMT32Settings = false;
	_mt32Checkbox = 0;
	_mt32DevicePopUp = 0;
	_mt32DevicePopUpDesc = 0;
	_enableGSCheckbox = 0;
	_enableVolumeSettings = false;
	_musicVolumeDesc = 0;
	_musicVolumeSlider = 0;
	_musicVolumeLabel = 0;
	_sfxVolumeDesc = 0;
	_sfxVolumeSlider = 0;
	_sfxVolumeLabel = 0;
	_speechVolumeDesc = 0;
	_speechVolumeSlider = 0;
	_speechVolumeLabel = 0;
	_muteCheckbox = 0;
	_subToggleDesc = 0;
	_subToggleGroup = 0;
	_subToggleSubOnly = 0;
	_subToggleSpeechOnly = 0;
	_subToggleSubBoth = 0;
	_subSpeedDesc = 0;
	_subSpeedSlider = 0;
	_subSpeedLabel = 0;
	_oldTheme = g_gui.theme()->getThemeId();

	// Retrieve game GUI options
	_guioptions.clear();
	if (ConfMan.hasKey("guioptions", _domain)) {
		_guioptionsString = ConfMan.get("guioptions", _domain);
		_guioptions = parseGameGUIOptions(_guioptionsString);
	}
}

void OptionsDialog::open() {
	Dialog::open();

	// Reset result value
	setResult(0);

	// Retrieve game GUI options
	_guioptions.clear();
	if (ConfMan.hasKey("guioptions", _domain)) {
		_guioptionsString = ConfMan.get("guioptions", _domain);
		_guioptions = parseGameGUIOptions(_guioptionsString);
	}

	// Graphic options
	if (_fullscreenCheckbox) {
		_gfxPopUp->setSelected(0);

		if (ConfMan.hasKey("gfx_mode", _domain)) {
			const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
			Common::String gfxMode(ConfMan.get("gfx_mode", _domain));
			int gfxCount = 1;
			while (gm->name) {
				gfxCount++;

				if (scumm_stricmp(gm->name, gfxMode.c_str()) == 0)
					_gfxPopUp->setSelected(gfxCount);

				gm++;
			}
		}

		_renderModePopUp->setSelected(0);

		if (ConfMan.hasKey("render_mode", _domain)) {
			const Common::RenderModeDescription *p = Common::g_renderModes;
			const Common::RenderMode renderMode = Common::parseRenderMode(ConfMan.get("render_mode", _domain));
			int sel = 0;
			for (int i = 0; p->code; ++p, ++i) {
				if (renderMode == p->id)
					sel = p->id;
			}
			_renderModePopUp->setSelectedTag(sel);
		}

#ifdef SMALL_SCREEN_DEVICE
		_fullscreenCheckbox->setState(true);
		_fullscreenCheckbox->setEnabled(false);
		_aspectCheckbox->setState(false);
		_aspectCheckbox->setEnabled(false);
#else // !SMALL_SCREEN_DEVICE
		// Fullscreen setting
		_fullscreenCheckbox->setState(ConfMan.getBool("fullscreen", _domain));

		// Aspect ratio setting
		if (_guioptions.contains(GUIO_NOASPECT)) {
			_aspectCheckbox->setState(false);
			_aspectCheckbox->setEnabled(false);
		} else {
			_aspectCheckbox->setEnabled(true);
			_aspectCheckbox->setState(ConfMan.getBool("aspect_ratio", _domain));
		}
#endif // SMALL_SCREEN_DEVICE

	}

	// Audio options
	if (!loadMusicDeviceSetting(_midiPopUp, "music_driver"))
		_midiPopUp->setSelected(0);

	if (_oplPopUp) {
		OPL::Config::DriverId id = MAX<OPL::Config::DriverId>(OPL::Config::parse(ConfMan.get("opl_driver", _domain)), 0);
		_oplPopUp->setSelectedTag(id);
	}

	if (_outputRatePopUp) {
		_outputRatePopUp->setSelected(1);
		int value = ConfMan.getInt("output_rate", _domain);
		for	(int i = 0; outputRateLabels[i]; i++) {
			if (value == outputRateValues[i])
				_outputRatePopUp->setSelected(i);
		}
	}

	if (_multiMidiCheckbox) {
		if (!loadMusicDeviceSetting(_gmDevicePopUp, "gm_device"))
			_gmDevicePopUp->setSelected(0);

		// Multi midi setting
		_multiMidiCheckbox->setState(ConfMan.getBool("multi_midi", _domain));

		Common::String soundFont(ConfMan.get("soundfont", _domain));
		if (soundFont.empty() || !ConfMan.hasKey("soundfont", _domain)) {
			_soundFont->setLabel(_c("None", "soundfont"));
			_soundFontClearButton->setEnabled(false);
		} else {
			_soundFont->setLabel(soundFont);
			_soundFontClearButton->setEnabled(true);
		}

		// MIDI gain setting
		_midiGainSlider->setValue(ConfMan.getInt("midi_gain", _domain));
		_midiGainLabel->setLabel(Common::String::format("%.2f", (double)_midiGainSlider->getValue() / 100.0));
	}

	// MT-32 options
	if (_mt32DevicePopUp) {
		if (!loadMusicDeviceSetting(_mt32DevicePopUp, "mt32_device"))
			_mt32DevicePopUp->setSelected(0);

		// Native mt32 setting
		_mt32Checkbox->setState(ConfMan.getBool("native_mt32", _domain));

		// GS extensions setting
		_enableGSCheckbox->setState(ConfMan.getBool("enable_gs", _domain));
	}

	// Volume options
	if (_musicVolumeSlider) {
		int vol;

		vol = ConfMan.getInt("music_volume", _domain);
		_musicVolumeSlider->setValue(vol);
		_musicVolumeLabel->setValue(vol);

		vol = ConfMan.getInt("sfx_volume", _domain);
		_sfxVolumeSlider->setValue(vol);
		_sfxVolumeLabel->setValue(vol);

		vol = ConfMan.getInt("speech_volume", _domain);
		_speechVolumeSlider->setValue(vol);
		_speechVolumeLabel->setValue(vol);

		bool val = false;
		if (ConfMan.hasKey("mute", _domain)) {
			val = ConfMan.getBool("mute", _domain);
		} else {
			ConfMan.setBool("mute", false);
		}
		_muteCheckbox->setState(val);
	}

	// Subtitle options
	if (_subToggleGroup) {
		int speed;
		int sliderMaxValue = _subSpeedSlider->getMaxValue();

		int subMode = getSubtitleMode(ConfMan.getBool("subtitles", _domain), ConfMan.getBool("speech_mute", _domain));
		_subToggleGroup->setValue(subMode);

		// Engines that reuse the subtitle speed widget set their own max value.
		// Scale the config value accordingly (see addSubtitleControls)
		speed = (ConfMan.getInt("talkspeed", _domain) * sliderMaxValue + 255 / 2) / 255;
		_subSpeedSlider->setValue(speed);
		_subSpeedLabel->setValue(speed);
	}
}

void OptionsDialog::close() {
	if (getResult()) {

		// Graphic options
		bool graphicsModeChanged = false;
		if (_fullscreenCheckbox) {
			if (_enableGraphicSettings) {
				if (ConfMan.getBool("fullscreen", _domain) != _fullscreenCheckbox->getState())
					graphicsModeChanged = true;
				if (ConfMan.getBool("aspect_ratio", _domain) != _aspectCheckbox->getState())
					graphicsModeChanged = true;

				ConfMan.setBool("fullscreen", _fullscreenCheckbox->getState(), _domain);
				ConfMan.setBool("aspect_ratio", _aspectCheckbox->getState(), _domain);

				bool isSet = false;

				if ((int32)_gfxPopUp->getSelectedTag() >= 0) {
					const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();

					while (gm->name) {
						if (gm->id == (int)_gfxPopUp->getSelectedTag()) {
							if (ConfMan.get("gfx_mode", _domain) != gm->name)
								graphicsModeChanged = true;
							ConfMan.set("gfx_mode", gm->name, _domain);
							isSet = true;
							break;
						}
						gm++;
					}
				}
				if (!isSet)
					ConfMan.removeKey("gfx_mode", _domain);

				if ((int32)_renderModePopUp->getSelectedTag() >= 0)
					ConfMan.set("render_mode", Common::getRenderModeCode((Common::RenderMode)_renderModePopUp->getSelectedTag()), _domain);
			} else {
				ConfMan.removeKey("fullscreen", _domain);
				ConfMan.removeKey("aspect_ratio", _domain);
				ConfMan.removeKey("gfx_mode", _domain);
				ConfMan.removeKey("render_mode", _domain);
			}
		}

		// Setup graphics again if needed
		if (_domain == Common::ConfigManager::kApplicationDomain && graphicsModeChanged) {
			g_system->beginGFXTransaction();
			g_system->setGraphicsMode(ConfMan.get("gfx_mode", _domain).c_str());

			if (ConfMan.hasKey("aspect_ratio"))
				g_system->setFeatureState(OSystem::kFeatureAspectRatioCorrection, ConfMan.getBool("aspect_ratio", _domain));
			if (ConfMan.hasKey("fullscreen"))
				g_system->setFeatureState(OSystem::kFeatureFullscreenMode, ConfMan.getBool("fullscreen", _domain));
			OSystem::TransactionError gfxError = g_system->endGFXTransaction();

			// Since this might change the screen resolution we need to give
			// the GUI a chance to update it's internal state. Otherwise we might
			// get a crash when the GUI tries to grab the overlay.
			//
			// This fixes bug #3303501 "Switching from HQ2x->HQ3x crashes ScummVM"
			//
			// It is important that this is called *before* any of the current
			// dialog's widgets are destroyed (for example before
			// Dialog::close) is called, to prevent crashes caused by invalid
			// widgets being referenced or similar errors.
			g_gui.checkScreenChange();

			if (gfxError != OSystem::kTransactionSuccess) {
				// Revert ConfMan to what OSystem is using.
				Common::String message = _("Failed to apply some of the graphic options changes:");

				if (gfxError & OSystem::kTransactionModeSwitchFailed) {
					const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
					while (gm->name) {
						if (gm->id == g_system->getGraphicsMode()) {
							ConfMan.set("gfx_mode", gm->name, _domain);
							break;
						}
						gm++;
					}
					message += "\n";
					message += _("the video mode could not be changed.");
				}

				if (gfxError & OSystem::kTransactionAspectRatioFailed) {
					ConfMan.setBool("aspect_ratio", g_system->getFeatureState(OSystem::kFeatureAspectRatioCorrection), _domain);
					message += "\n";
					message += _("the fullscreen setting could not be changed");
				}

				if (gfxError & OSystem::kTransactionFullscreenFailed) {
					ConfMan.setBool("fullscreen", g_system->getFeatureState(OSystem::kFeatureFullscreenMode), _domain);
					message += "\n";
					message += _("the aspect ratio setting could not be changed");
				}

				// And display the error
				GUI::MessageDialog dialog(message);
				dialog.runModal();
			}
		}

		// Volume options
		if (_musicVolumeSlider) {
			if (_enableVolumeSettings) {
				ConfMan.setInt("music_volume", _musicVolumeSlider->getValue(), _domain);
				ConfMan.setInt("sfx_volume", _sfxVolumeSlider->getValue(), _domain);
				ConfMan.setInt("speech_volume", _speechVolumeSlider->getValue(), _domain);
				ConfMan.setBool("mute", _muteCheckbox->getState(), _domain);
			} else {
				ConfMan.removeKey("music_volume", _domain);
				ConfMan.removeKey("sfx_volume", _domain);
				ConfMan.removeKey("speech_volume", _domain);
				ConfMan.removeKey("mute", _domain);
			}
		}

		// Audio options
		if (_midiPopUp) {
			if (_enableAudioSettings) {
				saveMusicDeviceSetting(_midiPopUp, "music_driver");
			} else {
				ConfMan.removeKey("music_driver", _domain);
			}
		}

		if (_oplPopUp) {
			if (_enableAudioSettings) {
				const OPL::Config::EmulatorDescription *ed = OPL::Config::getAvailable();
				while (ed->name && ed->id != (int)_oplPopUp->getSelectedTag())
					++ed;

				if (ed->name)
					ConfMan.set("opl_driver", ed->name, _domain);
				else
					ConfMan.removeKey("opl_driver", _domain);
			} else {
				ConfMan.removeKey("opl_driver", _domain);
			}
		}

		if (_outputRatePopUp) {
			if (_enableAudioSettings) {
				if (_outputRatePopUp->getSelectedTag() != 0)
					ConfMan.setInt("output_rate", _outputRatePopUp->getSelectedTag(), _domain);
				else
					ConfMan.removeKey("output_rate", _domain);
			} else {
				ConfMan.removeKey("output_rate", _domain);
			}
		}

		// MIDI options
		if (_multiMidiCheckbox) {
			if (_enableMIDISettings) {
				saveMusicDeviceSetting(_gmDevicePopUp, "gm_device");

				ConfMan.setBool("multi_midi", _multiMidiCheckbox->getState(), _domain);
				ConfMan.setInt("midi_gain", _midiGainSlider->getValue(), _domain);

				Common::String soundFont(_soundFont->getLabel());
				if (!soundFont.empty() && (soundFont != _c("None", "soundfont")))
					ConfMan.set("soundfont", soundFont, _domain);
				else
					ConfMan.removeKey("soundfont", _domain);
			} else {
				ConfMan.removeKey("gm_device", _domain);
				ConfMan.removeKey("multi_midi", _domain);
				ConfMan.removeKey("midi_gain", _domain);
				ConfMan.removeKey("soundfont", _domain);
			}
		}

		// MT-32 options
		if (_mt32DevicePopUp) {
			if (_enableMT32Settings) {
				saveMusicDeviceSetting(_mt32DevicePopUp, "mt32_device");
				ConfMan.setBool("native_mt32", _mt32Checkbox->getState(), _domain);
				ConfMan.setBool("enable_gs", _enableGSCheckbox->getState(), _domain);
			} else {
				ConfMan.removeKey("mt32_device", _domain);
				ConfMan.removeKey("native_mt32", _domain);
				ConfMan.removeKey("enable_gs", _domain);
			}
		}

		// Subtitle options
		if (_subToggleGroup) {
			if (_enableSubtitleSettings) {
				bool subtitles, speech_mute;
				int talkspeed;
				int sliderMaxValue = _subSpeedSlider->getMaxValue();

				switch (_subToggleGroup->getValue()) {
				case kSubtitlesSpeech:
					subtitles = speech_mute = false;
					break;
				case kSubtitlesBoth:
					subtitles = true;
					speech_mute = false;
					break;
				case kSubtitlesSubs:
				default:
					subtitles = speech_mute = true;
					break;
				}

				ConfMan.setBool("subtitles", subtitles, _domain);
				ConfMan.setBool("speech_mute", speech_mute, _domain);

				// Engines that reuse the subtitle speed widget set their own max value.
				// Scale the config value accordingly (see addSubtitleControls)
				talkspeed = (_subSpeedSlider->getValue() * 255 + sliderMaxValue / 2) / sliderMaxValue;
				ConfMan.setInt("talkspeed", talkspeed, _domain);

			} else {
				ConfMan.removeKey("subtitles", _domain);
				ConfMan.removeKey("talkspeed", _domain);
				ConfMan.removeKey("speech_mute", _domain);
			}
		}

		// Save config file
		ConfMan.flushToDisk();
	}

	Dialog::close();
}

void OptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kMidiGainChanged:
		_midiGainLabel->setLabel(Common::String::format("%.2f", (double)_midiGainSlider->getValue() / 100.0));
		_midiGainLabel->draw();
		break;
	case kMusicVolumeChanged:
		_musicVolumeLabel->setValue(_musicVolumeSlider->getValue());
		_musicVolumeLabel->draw();
		break;
	case kSfxVolumeChanged:
		_sfxVolumeLabel->setValue(_sfxVolumeSlider->getValue());
		_sfxVolumeLabel->draw();
		break;
	case kSpeechVolumeChanged:
		_speechVolumeLabel->setValue(_speechVolumeSlider->getValue());
		_speechVolumeLabel->draw();
		break;
	case kMuteAllChanged:
		// 'true' because if control is disabled then event do not pass
		setVolumeSettingsState(true);
		break;
	case kSubtitleToggle:
		// We update the slider settings here, when there are sliders, to
		// disable the speech volume in case we are in subtitle only mode.
		if (_musicVolumeSlider)
			setVolumeSettingsState(true);
		break;
	case kSubtitleSpeedChanged:
		_subSpeedLabel->setValue(_subSpeedSlider->getValue());
		_subSpeedLabel->draw();
		break;
	case kClearSoundFontCmd:
		_soundFont->setLabel(_c("None", "soundfont"));
		_soundFontClearButton->setEnabled(false);
		draw();
		break;
	case kOKCmd:
		setResult(1);
		close();
		break;
	case kCloseCmd:
		if (g_gui.theme()->getThemeId() != _oldTheme) {
			g_gui.loadNewTheme(_oldTheme);
			ConfMan.set("gui_theme", _oldTheme);
		}
		close();
		break;
	default:
		Dialog::handleCommand(sender, cmd, data);
	}
}

void OptionsDialog::setGraphicSettingsState(bool enabled) {
	_enableGraphicSettings = enabled;

	_gfxPopUpDesc->setEnabled(enabled);
	_gfxPopUp->setEnabled(enabled);
	_renderModePopUpDesc->setEnabled(enabled);
	_renderModePopUp->setEnabled(enabled);
#ifndef SMALL_SCREEN_DEVICE
	_fullscreenCheckbox->setEnabled(enabled);
	if (_guioptions.contains(GUIO_NOASPECT))
		_aspectCheckbox->setEnabled(false);
	else
		_aspectCheckbox->setEnabled(enabled);
#endif
}

void OptionsDialog::setAudioSettingsState(bool enabled) {
	_enableAudioSettings = enabled;
	_midiPopUpDesc->setEnabled(enabled);
	_midiPopUp->setEnabled(enabled);

	const Common::String allFlags = MidiDriver::musicType2GUIO((uint32)-1);
	bool hasMidiDefined = (strpbrk(_guioptions.c_str(), allFlags.c_str()) != NULL);

	if (_domain != Common::ConfigManager::kApplicationDomain && // global dialog
		hasMidiDefined && // No flags are specified
		!(_guioptions.contains(GUIO_MIDIADLIB))) {
		_oplPopUpDesc->setEnabled(false);
		_oplPopUp->setEnabled(false);
	} else {
		_oplPopUpDesc->setEnabled(enabled);
		_oplPopUp->setEnabled(enabled);
	}
	_outputRatePopUpDesc->setEnabled(enabled);
	_outputRatePopUp->setEnabled(enabled);
}

void OptionsDialog::setMIDISettingsState(bool enabled) {
	if (_guioptions.contains(GUIO_NOMIDI))
		enabled = false;

	_gmDevicePopUpDesc->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);
	_gmDevicePopUp->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);

	_enableMIDISettings = enabled;

	_soundFontButton->setEnabled(enabled);
	_soundFont->setEnabled(enabled);

	if (enabled && !_soundFont->getLabel().empty() && (_soundFont->getLabel() != _c("None", "soundfont")))
		_soundFontClearButton->setEnabled(enabled);
	else
		_soundFontClearButton->setEnabled(false);

	_multiMidiCheckbox->setEnabled(enabled);
	_midiGainDesc->setEnabled(enabled);
	_midiGainSlider->setEnabled(enabled);
	_midiGainLabel->setEnabled(enabled);
}

void OptionsDialog::setMT32SettingsState(bool enabled) {
	_enableMT32Settings = enabled;

	_mt32DevicePopUpDesc->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);
	_mt32DevicePopUp->setEnabled(_domain.equals(Common::ConfigManager::kApplicationDomain) ? enabled : false);

	_mt32Checkbox->setEnabled(enabled);
	_enableGSCheckbox->setEnabled(enabled);
}

void OptionsDialog::setVolumeSettingsState(bool enabled) {
	bool ena;

	_enableVolumeSettings = enabled;

	ena = enabled && !_muteCheckbox->getState();
	if (_guioptions.contains(GUIO_NOMUSIC))
		ena = false;

	_musicVolumeDesc->setEnabled(ena);
	_musicVolumeSlider->setEnabled(ena);
	_musicVolumeLabel->setEnabled(ena);

	ena = enabled && !_muteCheckbox->getState();
	if (_guioptions.contains(GUIO_NOSFX))
		ena = false;

	_sfxVolumeDesc->setEnabled(ena);
	_sfxVolumeSlider->setEnabled(ena);
	_sfxVolumeLabel->setEnabled(ena);

	ena = enabled && !_muteCheckbox->getState();
	// Disable speech volume slider, when we are in subtitle only mode.
	if (_subToggleGroup)
		ena = ena && _subToggleGroup->getValue() != kSubtitlesSubs;
	if (_guioptions.contains(GUIO_NOSPEECH))
		ena = false;

	_speechVolumeDesc->setEnabled(ena);
	_speechVolumeSlider->setEnabled(ena);
	_speechVolumeLabel->setEnabled(ena);

	_muteCheckbox->setEnabled(enabled);
}

void OptionsDialog::setSubtitleSettingsState(bool enabled) {
	bool ena;
	_enableSubtitleSettings = enabled;

	ena = enabled;
	if ((_guioptions.contains(GUIO_NOSUBTITLES)) || (_guioptions.contains(GUIO_NOSPEECH)))
		ena = false;

	_subToggleGroup->setEnabled(ena);
	_subToggleDesc->setEnabled(ena);

	ena = enabled;
	if (_guioptions.contains(GUIO_NOSUBTITLES))
		ena = false;

	_subSpeedDesc->setEnabled(ena);
	_subSpeedSlider->setEnabled(ena);
	_subSpeedLabel->setEnabled(ena);
}

void OptionsDialog::addGraphicControls(GuiObject *boss, const Common::String &prefix) {
	const OSystem::GraphicsMode *gm = g_system->getSupportedGraphicsModes();
	Common::String context;
	if (g_system->getOverlayWidth() <= 320)
		context = "lowres";

	// The GFX mode popup
	_gfxPopUpDesc = new StaticTextWidget(boss, prefix + "grModePopupDesc", _("Graphics mode:"));
	_gfxPopUp = new PopUpWidget(boss, prefix + "grModePopup");

	_gfxPopUp->appendEntry(_("<default>"));
	_gfxPopUp->appendEntry("");
	while (gm->name) {
		_gfxPopUp->appendEntry(_c(gm->description, context), gm->id);
		gm++;
	}

	// RenderMode popup
	const Common::String allFlags = Common::allRenderModesGUIOs();
	bool renderingTypeDefined = (strpbrk(_guioptions.c_str(), allFlags.c_str()) != NULL);

	_renderModePopUpDesc = new StaticTextWidget(boss, prefix + "grRenderPopupDesc", _("Render mode:"), _("Special dithering modes supported by some games"));
	_renderModePopUp = new PopUpWidget(boss, prefix + "grRenderPopup", _("Special dithering modes supported by some games"));
	_renderModePopUp->appendEntry(_("<default>"), Common::kRenderDefault);
	_renderModePopUp->appendEntry("");
	const Common::RenderModeDescription *rm = Common::g_renderModes;
	for (; rm->code; ++rm) {
		Common::String renderGuiOption = Common::renderMode2GUIO(rm->id);
		if ((_domain == Common::ConfigManager::kApplicationDomain) || (_domain != Common::ConfigManager::kApplicationDomain && !renderingTypeDefined) || (_guioptions.contains(renderGuiOption)))
			_renderModePopUp->appendEntry(_c(rm->description, context), rm->id);
	}

	// Fullscreen checkbox
	_fullscreenCheckbox = new CheckboxWidget(boss, prefix + "grFullscreenCheckbox", _("Fullscreen mode"));

	// Aspect ratio checkbox
	_aspectCheckbox = new CheckboxWidget(boss, prefix + "grAspectCheckbox", _("Aspect ratio correction"), _("Correct aspect ratio for 320x200 games"));

	_enableGraphicSettings = true;
}

void OptionsDialog::addAudioControls(GuiObject *boss, const Common::String &prefix) {
	// The MIDI mode popup & a label
	if (g_system->getOverlayWidth() > 320)
		_midiPopUpDesc = new StaticTextWidget(boss, prefix + "auMidiPopupDesc", _domain == Common::ConfigManager::kApplicationDomain ? _("Preferred Device:") : _("Music Device:"), _domain == Common::ConfigManager::kApplicationDomain ? _("Specifies preferred sound device or sound card emulator") : _("Specifies output sound device or sound card emulator"));
	else
		_midiPopUpDesc = new StaticTextWidget(boss, prefix + "auMidiPopupDesc", _domain == Common::ConfigManager::kApplicationDomain ? _c("Preferred Dev.:", "lowres") : _c("Music Device:", "lowres"), _domain == Common::ConfigManager::kApplicationDomain ? _("Specifies preferred sound device or sound card emulator") : _("Specifies output sound device or sound card emulator"));
	_midiPopUp = new PopUpWidget(boss, prefix + "auMidiPopup", _("Specifies output sound device or sound card emulator"));

	// Populate it
	const Common::String allFlags = MidiDriver::musicType2GUIO((uint32)-1);
	bool hasMidiDefined = (strpbrk(_guioptions.c_str(), allFlags.c_str()) != NULL);

	const MusicPlugin::List p = MusicMan.getPlugins();
	for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (**m)->getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			Common::String deviceGuiOption = MidiDriver::musicType2GUIO(d->getMusicType());

			if ((_domain == Common::ConfigManager::kApplicationDomain && d->getMusicType() != MT_TOWNS  // global dialog - skip useless FM-Towns, C64, Amiga, AppleIIGS options there
				 && d->getMusicType() != MT_C64 && d->getMusicType() != MT_AMIGA && d->getMusicType() != MT_APPLEIIGS && d->getMusicType() != MT_PC98)
				|| (_domain != Common::ConfigManager::kApplicationDomain && !hasMidiDefined) // No flags are specified
				|| (_guioptions.contains(deviceGuiOption)) // flag is present
				// HACK/FIXME: For now we have to show GM devices, even when the game only has GUIO_MIDIMT32 set,
				// else we would not show for example external devices connected via ALSA, since they are always
				// marked as General MIDI device.
				|| (deviceGuiOption.contains(GUIO_MIDIGM) && (_guioptions.contains(GUIO_MIDIMT32)))
				|| d->getMusicDriverId() == "auto" || d->getMusicDriverId() == "null") // always add default and null device
				_midiPopUp->appendEntry(d->getCompleteName(), d->getHandle());
		}
	}

	// The OPL emulator popup & a label
	_oplPopUpDesc = new StaticTextWidget(boss, prefix + "auOPLPopupDesc", _("AdLib emulator:"), _("AdLib is used for music in many games"));
	_oplPopUp = new PopUpWidget(boss, prefix + "auOPLPopup", _("AdLib is used for music in many games"));

	// Populate it
	const OPL::Config::EmulatorDescription *ed = OPL::Config::getAvailable();
	while (ed->name) {
		_oplPopUp->appendEntry(_(ed->description), ed->id);
		++ed;
	}

	// Sample rate settings
	_outputRatePopUpDesc = new StaticTextWidget(boss, prefix + "auSampleRatePopupDesc", _("Output rate:"), _("Higher value specifies better sound quality but may be not supported by your soundcard"));
	_outputRatePopUp = new PopUpWidget(boss, prefix + "auSampleRatePopup", _("Higher value specifies better sound quality but may be not supported by your soundcard"));

	for (int i = 0; outputRateLabels[i]; i++) {
		_outputRatePopUp->appendEntry(_(outputRateLabels[i]), outputRateValues[i]);
	}

	_enableAudioSettings = true;
}

void OptionsDialog::addMIDIControls(GuiObject *boss, const Common::String &prefix) {
	_gmDevicePopUpDesc = new StaticTextWidget(boss, prefix + "auPrefGmPopupDesc", _("GM Device:"), _("Specifies default sound device for General MIDI output"));
	_gmDevicePopUp = new PopUpWidget(boss, prefix + "auPrefGmPopup");

	// Populate
	const MusicPlugin::List p = MusicMan.getPlugins();
	// Make sure the null device is the first one in the list to avoid undesired
	// auto detection for users who don't have a saved setting yet.
	for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (**m)->getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getMusicDriverId() == "null")
				_gmDevicePopUp->appendEntry(_("Don't use General MIDI music"), d->getHandle());
		}
	}
	// Now we add the other devices.
	for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (**m)->getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getMusicType() >= MT_GM) {
				if (d->getMusicType() != MT_MT32)
					_gmDevicePopUp->appendEntry(d->getCompleteName(), d->getHandle());
			} else if (d->getMusicDriverId() == "auto") {
				_gmDevicePopUp->appendEntry(_("Use first available device"), d->getHandle());
			}
		}
	}

	if (!_domain.equals(Common::ConfigManager::kApplicationDomain)) {
		_gmDevicePopUpDesc->setEnabled(false);
		_gmDevicePopUp->setEnabled(false);
	}

	// SoundFont
	if (g_system->getOverlayWidth() > 320)
		_soundFontButton = new ButtonWidget(boss, prefix + "mcFontButton", _("SoundFont:"), _("SoundFont is supported by some audio cards, Fluidsynth and Timidity"), kChooseSoundFontCmd);
	else
		_soundFontButton = new ButtonWidget(boss, prefix + "mcFontButton", _c("SoundFont:", "lowres"), _("SoundFont is supported by some audio cards, Fluidsynth and Timidity"), kChooseSoundFontCmd);
	_soundFont = new StaticTextWidget(boss, prefix + "mcFontPath", _c("None", "soundfont"), _("SoundFont is supported by some audio cards, Fluidsynth and Timidity"));

	_soundFontClearButton = addClearButton(boss, prefix + "mcFontClearButton", kClearSoundFontCmd);

	// Multi midi setting
	_multiMidiCheckbox = new CheckboxWidget(boss, prefix + "mcMixedCheckbox", _("Mixed AdLib/MIDI mode"), _("Use both MIDI and AdLib sound generation"));

	// MIDI gain setting (FluidSynth uses this)
	_midiGainDesc = new StaticTextWidget(boss, prefix + "mcMidiGainText", _("MIDI gain:"));
	_midiGainSlider = new SliderWidget(boss, prefix + "mcMidiGainSlider", 0, kMidiGainChanged);
	_midiGainSlider->setMinValue(0);
	_midiGainSlider->setMaxValue(1000);
	_midiGainLabel = new StaticTextWidget(boss, prefix + "mcMidiGainLabel", "1.00");

	_enableMIDISettings = true;
}

void OptionsDialog::addMT32Controls(GuiObject *boss, const Common::String &prefix) {
	_mt32DevicePopUpDesc = new StaticTextWidget(boss, prefix + "auPrefMt32PopupDesc", _("MT-32 Device:"), _("Specifies default sound device for Roland MT-32/LAPC1/CM32l/CM64 output"));
	_mt32DevicePopUp = new PopUpWidget(boss, prefix + "auPrefMt32Popup");

	// Native mt32 setting
	if (g_system->getOverlayWidth() > 320)
		_mt32Checkbox = new CheckboxWidget(boss, prefix + "mcMt32Checkbox", _("True Roland MT-32 (disable GM emulation)"), _("Check if you want to use your real hardware Roland-compatible sound device connected to your computer"));
	else
		_mt32Checkbox = new CheckboxWidget(boss, prefix + "mcMt32Checkbox", _c("True Roland MT-32 (no GM emulation)", "lowres"), _("Check if you want to use your real hardware Roland-compatible sound device connected to your computer"));

	// GS Extensions setting
	_enableGSCheckbox = new CheckboxWidget(boss, prefix + "mcGSCheckbox", _("Enable Roland GS Mode"), _("Turns off General MIDI mapping for games with Roland MT-32 soundtrack"));

	const MusicPlugin::List p = MusicMan.getPlugins();
	// Make sure the null device is the first one in the list to avoid undesired
	// auto detection for users who don't have a saved setting yet.
	for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (**m)->getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getMusicDriverId() == "null")
				_mt32DevicePopUp->appendEntry(_("Don't use Roland MT-32 music"), d->getHandle());
		}
	}
	// Now we add the other devices.
	for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end(); ++m) {
		MusicDevices i = (**m)->getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getMusicType() >= MT_GM)
				_mt32DevicePopUp->appendEntry(d->getCompleteName(), d->getHandle());
			else if (d->getMusicDriverId() == "auto")
				_mt32DevicePopUp->appendEntry(_("Use first available device"), d->getHandle());
		}
	}

	if (!_domain.equals(Common::ConfigManager::kApplicationDomain)) {
		_mt32DevicePopUpDesc->setEnabled(false);
		_mt32DevicePopUp->setEnabled(false);
	}

	_enableMT32Settings = true;
}

// The function has an extra slider range parameter, since both the launcher and SCUMM engine
// make use of the widgets. The launcher range is 0-255. SCUMM's 0-9
void OptionsDialog::addSubtitleControls(GuiObject *boss, const Common::String &prefix, int maxSliderVal) {

	if (g_system->getOverlayWidth() > 320) {
		_subToggleDesc = new StaticTextWidget(boss, prefix + "subToggleDesc", _("Text and Speech:"));

		_subToggleGroup = new RadiobuttonGroup(boss, kSubtitleToggle);

		_subToggleSpeechOnly = new RadiobuttonWidget(boss, prefix + "subToggleSpeechOnly", _subToggleGroup, kSubtitlesSpeech, _("Speech"));
		_subToggleSubOnly = new RadiobuttonWidget(boss, prefix + "subToggleSubOnly", _subToggleGroup, kSubtitlesSubs, _("Subtitles"));
		_subToggleSubBoth = new RadiobuttonWidget(boss, prefix + "subToggleSubBoth", _subToggleGroup, kSubtitlesBoth, _("Both"));

		_subSpeedDesc = new StaticTextWidget(boss, prefix + "subSubtitleSpeedDesc", _("Subtitle speed:"));
	} else {
		_subToggleDesc = new StaticTextWidget(boss, prefix + "subToggleDesc", _c("Text and Speech:", "lowres"));

		_subToggleGroup = new RadiobuttonGroup(boss, kSubtitleToggle);

		_subToggleSpeechOnly = new RadiobuttonWidget(boss, prefix + "subToggleSpeechOnly", _subToggleGroup, kSubtitlesSpeech, _("Spch"), _("Speech"));
		_subToggleSubOnly = new RadiobuttonWidget(boss, prefix + "subToggleSubOnly", _subToggleGroup, kSubtitlesSubs, _("Subs"), _("Subtitles"));
		_subToggleSubBoth = new RadiobuttonWidget(boss, prefix + "subToggleSubBoth", _subToggleGroup, kSubtitlesBoth, _c("Both", "lowres"), _("Show subtitles and play speech"));

		_subSpeedDesc = new StaticTextWidget(boss, prefix + "subSubtitleSpeedDesc", _c("Subtitle speed:", "lowres"));
	}

	// Subtitle speed
	_subSpeedSlider = new SliderWidget(boss, prefix + "subSubtitleSpeedSlider", 0, kSubtitleSpeedChanged);
	_subSpeedLabel = new StaticTextWidget(boss, prefix + "subSubtitleSpeedLabel", "100%");
	_subSpeedSlider->setMinValue(0); _subSpeedSlider->setMaxValue(maxSliderVal);
	_subSpeedLabel->setFlags(WIDGET_CLEARBG);

	_enableSubtitleSettings = true;
}

void OptionsDialog::addVolumeControls(GuiObject *boss, const Common::String &prefix) {

	// Volume controllers
	if (g_system->getOverlayWidth() > 320)
		_musicVolumeDesc = new StaticTextWidget(boss, prefix + "vcMusicText", _("Music volume:"));
	else
		_musicVolumeDesc = new StaticTextWidget(boss, prefix + "vcMusicText", _c("Music volume:", "lowres"));
	_musicVolumeSlider = new SliderWidget(boss, prefix + "vcMusicSlider", 0, kMusicVolumeChanged);
	_musicVolumeLabel = new StaticTextWidget(boss, prefix + "vcMusicLabel", "100%");
	_musicVolumeSlider->setMinValue(0);
	_musicVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_musicVolumeLabel->setFlags(WIDGET_CLEARBG);

	_muteCheckbox = new CheckboxWidget(boss, prefix + "vcMuteCheckbox", _("Mute All"), 0, kMuteAllChanged);

	if (g_system->getOverlayWidth() > 320)
		_sfxVolumeDesc = new StaticTextWidget(boss, prefix + "vcSfxText", _("SFX volume:"), _("Special sound effects volume"));
	else
		_sfxVolumeDesc = new StaticTextWidget(boss, prefix + "vcSfxText", _c("SFX volume:", "lowres"), _("Special sound effects volume"));
	_sfxVolumeSlider = new SliderWidget(boss, prefix + "vcSfxSlider", _("Special sound effects volume"), kSfxVolumeChanged);
	_sfxVolumeLabel = new StaticTextWidget(boss, prefix + "vcSfxLabel", "100%");
	_sfxVolumeSlider->setMinValue(0);
	_sfxVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_sfxVolumeLabel->setFlags(WIDGET_CLEARBG);

	if (g_system->getOverlayWidth() > 320)
		_speechVolumeDesc = new StaticTextWidget(boss, prefix + "vcSpeechText" , _("Speech volume:"));
	else
		_speechVolumeDesc = new StaticTextWidget(boss, prefix + "vcSpeechText" , _c("Speech volume:", "lowres"));
	_speechVolumeSlider = new SliderWidget(boss, prefix + "vcSpeechSlider", 0, kSpeechVolumeChanged);
	_speechVolumeLabel = new StaticTextWidget(boss, prefix + "vcSpeechLabel", "100%");
	_speechVolumeSlider->setMinValue(0);
	_speechVolumeSlider->setMaxValue(Audio::Mixer::kMaxMixerVolume);
	_speechVolumeLabel->setFlags(WIDGET_CLEARBG);

	_enableVolumeSettings = true;
}

void OptionsDialog::addEngineControls(GuiObject *boss, const Common::String &prefix, const ExtraGuiOptions &engineOptions) {
	// Note: up to 7 engine options can currently fit on screen (the most that
	// can fit in a 320x200 screen with the classic theme).
	// TODO: Increase this number by including the checkboxes inside a scroll
	// widget. The appropriate number of checkboxes will need to be added to
	// the theme files.

	uint i = 1;
	ExtraGuiOptions::const_iterator iter;
	for (iter = engineOptions.begin(); iter != engineOptions.end(); ++iter, ++i) {
		Common::String id = Common::String::format("%d", i);
		_engineCheckboxes.push_back(new CheckboxWidget(boss,
			prefix + "customOption" + id + "Checkbox", _(iter->label), _(iter->tooltip)));
	}
}

bool OptionsDialog::loadMusicDeviceSetting(PopUpWidget *popup, Common::String setting, MusicType preferredType) {
	if (!popup || !popup->isEnabled())
		return true;

	if (_domain != Common::ConfigManager::kApplicationDomain || ConfMan.hasKey(setting, _domain) || preferredType) {
		const Common::String drv = ConfMan.get(setting, (_domain != Common::ConfigManager::kApplicationDomain && !ConfMan.hasKey(setting, _domain)) ? Common::ConfigManager::kApplicationDomain : _domain);
		const MusicPlugin::List p = MusicMan.getPlugins();

		for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end(); ++m) {
			MusicDevices i = (**m)->getDevices();
			for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
				if (setting.empty() ? (preferredType == d->getMusicType()) : (drv == d->getCompleteId())) {
					popup->setSelectedTag(d->getHandle());
					return popup->getSelected() != -1;
				}
			}
		}
	}

	return false;
}

void OptionsDialog::saveMusicDeviceSetting(PopUpWidget *popup, Common::String setting) {
	if (!popup || !_enableAudioSettings)
		return;

	const MusicPlugin::List p = MusicMan.getPlugins();
	bool found = false;
	for (MusicPlugin::List::const_iterator m = p.begin(); m != p.end() && !found; ++m) {
		MusicDevices i = (**m)->getDevices();
		for (MusicDevices::iterator d = i.begin(); d != i.end(); ++d) {
			if (d->getHandle() == popup->getSelectedTag()) {
				ConfMan.set(setting, d->getCompleteId(), _domain);
				found = true;
				break;
			}
		}
	}

	if (!found)
		ConfMan.removeKey(setting, _domain);
}

int OptionsDialog::getSubtitleMode(bool subtitles, bool speech_mute) {
	if (_guioptions.contains(GUIO_NOSUBTITLES))
		return kSubtitlesSpeech; // Speech only
	if (_guioptions.contains(GUIO_NOSPEECH))
		return kSubtitlesSubs; // Subtitles only

	if (!subtitles && !speech_mute) // Speech only
		return kSubtitlesSpeech;
	else if (subtitles && !speech_mute) // Speech and subtitles
		return kSubtitlesBoth;
	else if (subtitles && speech_mute) // Subtitles only
		return kSubtitlesSubs;
	else
		warning("Wrong configuration: Both subtitles and speech are off. Assuming subtitles only");
	return kSubtitlesSubs;
}

void OptionsDialog::reflowLayout() {
	if (_graphicsTabId != -1 && _tabWidget)
		_tabWidget->setTabTitle(_graphicsTabId, g_system->getOverlayWidth() > 320 ? _("Graphics") : _("GFX"));

	Dialog::reflowLayout();
}

#pragma mark -


GlobalOptionsDialog::GlobalOptionsDialog()
	: OptionsDialog(Common::ConfigManager::kApplicationDomain, "GlobalOptions") {

	// The tab widget
	TabWidget *tab = new TabWidget(this, "GlobalOptions.TabWidget");

	//
	// 1) The graphics tab
	//
	_graphicsTabId = tab->addTab(g_system->getOverlayWidth() > 320 ? _("Graphics") : _("GFX"));
	addGraphicControls(tab, "GlobalOptions_Graphics.");

	//
	// 2) The audio tab
	//
	tab->addTab(_("Audio"));
	addAudioControls(tab, "GlobalOptions_Audio.");
	addSubtitleControls(tab, "GlobalOptions_Audio.");

	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Volume"));
	else
		tab->addTab(_c("Volume", "lowres"));
	addVolumeControls(tab, "GlobalOptions_Volume.");

	// TODO: cd drive setting

	//
	// 3) The MIDI tab
	//
	_midiTabId = tab->addTab(_("MIDI"));
	addMIDIControls(tab, "GlobalOptions_MIDI.");

	//
	// 4) The MT-32 tab
	//
	tab->addTab(_("MT-32"));
	addMT32Controls(tab, "GlobalOptions_MT32.");

	//
	// 5) The Paths tab
	//
	if (g_system->getOverlayWidth() > 320)
		_pathsTabId = tab->addTab(_("Paths"));
	else
		_pathsTabId = tab->addTab(_c("Paths", "lowres"));

#if !( defined(__DC__) || defined(__GP32__) )
	// These two buttons have to be extra wide, or the text will be
	// truncated in the small version of the GUI.

	// Save game path
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GlobalOptions_Paths.SaveButton", _("Save Path:"), _("Specifies where your savegames are put"), kChooseSaveDirCmd);
	else
		new ButtonWidget(tab, "GlobalOptions_Paths.SaveButton", _c("Save Path:", "lowres"), _("Specifies where your savegames are put"), kChooseSaveDirCmd);
	_savePath = new StaticTextWidget(tab, "GlobalOptions_Paths.SavePath", "/foo/bar", _("Specifies where your savegames are put"));

	_savePathClearButton = addClearButton(tab, "GlobalOptions_Paths.SavePathClearButton", kSavePathClearCmd);

	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GlobalOptions_Paths.ThemeButton", _("Theme Path:"), 0, kChooseThemeDirCmd);
	else
		new ButtonWidget(tab, "GlobalOptions_Paths.ThemeButton", _c("Theme Path:", "lowres"), 0, kChooseThemeDirCmd);
	_themePath = new StaticTextWidget(tab, "GlobalOptions_Paths.ThemePath", _c("None", "path"));

	_themePathClearButton = addClearButton(tab, "GlobalOptions_Paths.ThemePathClearButton", kThemePathClearCmd);

	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GlobalOptions_Paths.ExtraButton", _("Extra Path:"), _("Specifies path to additional data used by all games or ScummVM"), kChooseExtraDirCmd);
	else
		new ButtonWidget(tab, "GlobalOptions_Paths.ExtraButton", _c("Extra Path:", "lowres"), _("Specifies path to additional data used by all games or ScummVM"), kChooseExtraDirCmd);
	_extraPath = new StaticTextWidget(tab, "GlobalOptions_Paths.ExtraPath", _c("None", "path"), _("Specifies path to additional data used by all games or ScummVM"));

	_extraPathClearButton = addClearButton(tab, "GlobalOptions_Paths.ExtraPathClearButton", kExtraPathClearCmd);

#ifdef DYNAMIC_MODULES
	if (g_system->getOverlayWidth() > 320)
		new ButtonWidget(tab, "GlobalOptions_Paths.PluginsButton", _("Plugins Path:"), 0, kChoosePluginsDirCmd);
	else
		new ButtonWidget(tab, "GlobalOptions_Paths.PluginsButton", _c("Plugins Path:", "lowres"), 0, kChoosePluginsDirCmd);
	_pluginsPath = new StaticTextWidget(tab, "GlobalOptions_Paths.PluginsPath", _c("None", "path"));
#endif
#endif

	//
	// 6) The miscellaneous tab
	//
	if (g_system->getOverlayWidth() > 320)
		tab->addTab(_("Misc"));
	else
		tab->addTab(_c("Misc", "lowres"));

	new ButtonWidget(tab, "GlobalOptions_Misc.ThemeButton", _("Theme:"), 0, kChooseThemeCmd);
	_curTheme = new StaticTextWidget(tab, "GlobalOptions_Misc.CurTheme", g_gui.theme()->getThemeName());


	_rendererPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.RendererPopupDesc", _("GUI Renderer:"));
	_rendererPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.RendererPopup");

	if (g_system->getOverlayWidth() > 320) {
		for (uint i = 1; i < GUI::ThemeEngine::_rendererModesSize; ++i)
			_rendererPopUp->appendEntry(_(GUI::ThemeEngine::_rendererModes[i].name), GUI::ThemeEngine::_rendererModes[i].mode);
	} else {
		for (uint i = 1; i < GUI::ThemeEngine::_rendererModesSize; ++i)
			_rendererPopUp->appendEntry(_(GUI::ThemeEngine::_rendererModes[i].shortname), GUI::ThemeEngine::_rendererModes[i].mode);
	}

	if (g_system->getOverlayWidth() > 320)
		_autosavePeriodPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopupDesc", _("Autosave:"));
	else
		_autosavePeriodPopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopupDesc", _c("Autosave:", "lowres"));
	_autosavePeriodPopUp = new PopUpWidget(tab, "GlobalOptions_Misc.AutosavePeriodPopup");

	for (int i = 0; savePeriodLabels[i]; i++) {
		_autosavePeriodPopUp->appendEntry(_(savePeriodLabels[i]), savePeriodValues[i]);
	}

#ifdef SMALL_SCREEN_DEVICE
	new ButtonWidget(tab, "GlobalOptions_Misc.KeysButton", _("Keys"), 0, kChooseKeyMappingCmd);
#endif

	// TODO: joystick setting


#ifdef USE_TRANSLATION
	_guiLanguagePopUpDesc = new StaticTextWidget(tab, "GlobalOptions_Misc.GuiLanguagePopupDesc", _("GUI Language:"), _("Language of ScummVM GUI"));
	_guiLanguagePopUp = new PopUpWidget(tab, "GlobalOptions_Misc.GuiLanguagePopup");
#ifdef USE_DETECTLANG
	_guiLanguagePopUp->appendEntry(_("<default>"), Common::kTranslationAutodetectId);
#endif // USE_DETECTLANG
	_guiLanguagePopUp->appendEntry("English", Common::kTranslationBuiltinId);
	_guiLanguagePopUp->appendEntry("", 0);
	Common::TLangArray languages = TransMan.getSupportedLanguageNames();
	Common::TLangArray::iterator lang = languages.begin();
	while (lang != languages.end()) {
		_guiLanguagePopUp->appendEntry(lang->name, lang->id);
		lang++;
	}

	// Select the currently configured language or default/English if
	// nothing is specified.
	if (ConfMan.hasKey("gui_language"))
		_guiLanguagePopUp->setSelectedTag(TransMan.parseLanguage(ConfMan.get("gui_language")));
	else
#ifdef USE_DETECTLANG
		_guiLanguagePopUp->setSelectedTag(Common::kTranslationAutodetectId);
#else // !USE_DETECTLANG
		_guiLanguagePopUp->setSelectedTag(Common::kTranslationBuiltinId);
#endif // USE_DETECTLANG

#endif // USE_TRANSLATION

	// Activate the first tab
	tab->setActiveTab(0);
	_tabWidget = tab;

	// Add OK & Cancel buttons
	new ButtonWidget(this, "GlobalOptions.Cancel", _("Cancel"), 0, kCloseCmd);
	new ButtonWidget(this, "GlobalOptions.Ok", _("OK"), 0, kOKCmd);

#ifdef SMALL_SCREEN_DEVICE
	_keysDialog = new KeysDialog();
#endif
}

GlobalOptionsDialog::~GlobalOptionsDialog() {
#ifdef SMALL_SCREEN_DEVICE
	delete _keysDialog;
#endif
}

void GlobalOptionsDialog::open() {
	OptionsDialog::open();

#if !( defined(__DC__) || defined(__GP32__) )
	// Set _savePath to the current save path
	Common::String savePath(ConfMan.get("savepath", _domain));
	Common::String themePath(ConfMan.get("themepath", _domain));
	Common::String extraPath(ConfMan.get("extrapath", _domain));

	if (savePath.empty() || !ConfMan.hasKey("savepath", _domain)) {
		_savePath->setLabel(_("Default"));
	} else {
		_savePath->setLabel(savePath);
	}

	if (themePath.empty() || !ConfMan.hasKey("themepath", _domain)) {
		_themePath->setLabel(_c("None", "path"));
	} else {
		_themePath->setLabel(themePath);
	}

	if (extraPath.empty() || !ConfMan.hasKey("extrapath", _domain)) {
		_extraPath->setLabel(_c("None", "path"));
	} else {
		_extraPath->setLabel(extraPath);
	}

#ifdef DYNAMIC_MODULES
	Common::String pluginsPath(ConfMan.get("pluginspath", _domain));
	if (pluginsPath.empty() || !ConfMan.hasKey("pluginspath", _domain)) {
		_pluginsPath->setLabel(_c("None", "path"));
	} else {
		_pluginsPath->setLabel(pluginsPath);
	}
#endif
#endif

	// Misc Tab
	_autosavePeriodPopUp->setSelected(1);
	int value = ConfMan.getInt("autosave_period");
	for (int i = 0; savePeriodLabels[i]; i++) {
		if (value == savePeriodValues[i])
			_autosavePeriodPopUp->setSelected(i);
	}

	ThemeEngine::GraphicsMode mode = ThemeEngine::findMode(ConfMan.get("gui_renderer"));
	if (mode == ThemeEngine::kGfxDisabled)
		mode = ThemeEngine::_defaultRendererMode;
	_rendererPopUp->setSelectedTag(mode);
}

void GlobalOptionsDialog::close() {
	if (getResult()) {
		Common::String savePath(_savePath->getLabel());
		if (!savePath.empty() && (savePath != _("Default")))
			ConfMan.set("savepath", savePath, _domain);
		else
			ConfMan.removeKey("savepath", _domain);

		Common::String themePath(_themePath->getLabel());
		if (!themePath.empty() && (themePath != _c("None", "path")))
			ConfMan.set("themepath", themePath, _domain);
		else
			ConfMan.removeKey("themepath", _domain);

		Common::String extraPath(_extraPath->getLabel());
		if (!extraPath.empty() && (extraPath != _c("None", "path")))
			ConfMan.set("extrapath", extraPath, _domain);
		else
			ConfMan.removeKey("extrapath", _domain);

#ifdef DYNAMIC_MODULES
		Common::String pluginsPath(_pluginsPath->getLabel());
		if (!pluginsPath.empty() && (pluginsPath != _c("None", "path")))
			ConfMan.set("pluginspath", pluginsPath, _domain);
		else
			ConfMan.removeKey("pluginspath", _domain);
#endif

		ConfMan.setInt("autosave_period", _autosavePeriodPopUp->getSelectedTag(), _domain);

		GUI::ThemeEngine::GraphicsMode selected = (GUI::ThemeEngine::GraphicsMode)_rendererPopUp->getSelectedTag();
		const char *cfg = GUI::ThemeEngine::findModeConfigName(selected);
		if (!ConfMan.get("gui_renderer").equalsIgnoreCase(cfg)) {
			// FIXME: Actually, any changes (including the theme change) should
			// only become active *after* the options dialog has closed.
			g_gui.loadNewTheme(g_gui.theme()->getThemeId(), selected);
			ConfMan.set("gui_renderer", cfg, _domain);
		}
#ifdef USE_TRANSLATION
		Common::String oldLang = ConfMan.get("gui_language");
		int selLang = _guiLanguagePopUp->getSelectedTag();

		ConfMan.set("gui_language", TransMan.getLangById(selLang));

		Common::String newLang = ConfMan.get("gui_language").c_str();
		if (newLang != oldLang) {
#if 0
			// Activate the selected language
			TransMan.setLanguage(selLang);

			// FIXME: Actually, any changes (including the theme change) should
			// only become active *after* the options dialog has closed.
			g_gui.loadNewTheme(g_gui.theme()->getThemeId(), ThemeEngine::kGfxDisabled, true);
#else
			MessageDialog error(_("You have to restart ScummVM before your changes will take effect."));
			error.runModal();
#endif
		}
#endif // USE_TRANSLATION

	}
	OptionsDialog::close();
}

void GlobalOptionsDialog::handleCommand(CommandSender *sender, uint32 cmd, uint32 data) {
	switch (cmd) {
	case kChooseSaveDirCmd: {
		BrowserDialog browser(_("Select directory for savegames"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			if (dir.isWritable()) {
				_savePath->setLabel(dir.getPath());
			} else {
				MessageDialog error(_("The chosen directory cannot be written to. Please select another one."));
				error.runModal();
				return;
			}
			draw();
		}
		break;
	}
	case kChooseThemeDirCmd: {
		BrowserDialog browser(_("Select directory for GUI themes"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_themePath->setLabel(dir.getPath());
			draw();
		}
		break;
	}
	case kChooseExtraDirCmd: {
		BrowserDialog browser(_("Select directory for extra files"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_extraPath->setLabel(dir.getPath());
			draw();
		}
		break;
	}
#ifdef DYNAMIC_MODULES
	case kChoosePluginsDirCmd: {
		BrowserDialog browser(_("Select directory for plugins"), true);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode dir(browser.getResult());
			_pluginsPath->setLabel(dir.getPath());
			draw();
		}
		break;
	}
#endif
	case kThemePathClearCmd:
		_themePath->setLabel(_c("None", "path"));
		break;
	case kExtraPathClearCmd:
		_extraPath->setLabel(_c("None", "path"));
		break;
	case kSavePathClearCmd:
		_savePath->setLabel(_("Default"));
		break;
	case kChooseSoundFontCmd: {
		BrowserDialog browser(_("Select SoundFont"), false);
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::FSNode file(browser.getResult());
			_soundFont->setLabel(file.getPath());

			if (!file.getPath().empty() && (file.getPath() != _c("None", "path")))
				_soundFontClearButton->setEnabled(true);
			else
				_soundFontClearButton->setEnabled(false);

			draw();
		}
		break;
	}
	case kChooseThemeCmd: {
		ThemeBrowser browser;
		if (browser.runModal() > 0) {
			// User made his choice...
			Common::String theme = browser.getSelected();
			// FIXME: Actually, any changes (including the theme change) should
			// only become active *after* the options dialog has closed.
#ifdef USE_TRANSLATION
			Common::String lang = TransMan.getCurrentLanguage();
#endif
			if (g_gui.loadNewTheme(theme)) {
#ifdef USE_TRANSLATION
				// If the charset has changed, it means the font were not found for the
				// new theme. Since for the moment we do not support change of translation
				// language without restarting, we let the user know about this.
				if (lang != TransMan.getCurrentLanguage()) {
					TransMan.setLanguage(lang.c_str());
					g_gui.loadNewTheme(_oldTheme);
					MessageDialog error(_("The theme you selected does not support your current language. If you want to use this theme you need to switch to another language first."));
					error.runModal();
				} else {
#endif
					_curTheme->setLabel(g_gui.theme()->getThemeName());
					ConfMan.set("gui_theme", theme);
#ifdef USE_TRANSLATION
				}
#endif
			}
			draw();
		}
		break;
	}
#ifdef SMALL_SCREEN_DEVICE
	case kChooseKeyMappingCmd:
		_keysDialog->runModal();
		break;
#endif
	default:
		OptionsDialog::handleCommand(sender, cmd, data);
	}
}

void GlobalOptionsDialog::reflowLayout() {
	int activeTab = _tabWidget->getActiveTab();

	if (_midiTabId != -1) {
		_tabWidget->setActiveTab(_midiTabId);

		_tabWidget->removeWidget(_soundFontClearButton);
		_soundFontClearButton->setNext(0);
		delete _soundFontClearButton;
		_soundFontClearButton = addClearButton(_tabWidget, "GlobalOptions_MIDI.mcFontClearButton", kClearSoundFontCmd);
	}

	if (_pathsTabId != -1) {
		_tabWidget->setActiveTab(_pathsTabId);

		_tabWidget->removeWidget(_savePathClearButton);
		_savePathClearButton->setNext(0);
		delete _savePathClearButton;
		_savePathClearButton = addClearButton(_tabWidget, "GlobalOptions_Paths.SavePathClearButton", kSavePathClearCmd);

		_tabWidget->removeWidget(_themePathClearButton);
		_themePathClearButton->setNext(0);
		delete _themePathClearButton;
		_themePathClearButton = addClearButton(_tabWidget, "GlobalOptions_Paths.ThemePathClearButton", kThemePathClearCmd);

		_tabWidget->removeWidget(_extraPathClearButton);
		_extraPathClearButton->setNext(0);
		delete _extraPathClearButton;
		_extraPathClearButton = addClearButton(_tabWidget, "GlobalOptions_Paths.ExtraPathClearButton", kExtraPathClearCmd);
	}

	_tabWidget->setActiveTab(activeTab);
	OptionsDialog::reflowLayout();
}

} // End of namespace GUI
