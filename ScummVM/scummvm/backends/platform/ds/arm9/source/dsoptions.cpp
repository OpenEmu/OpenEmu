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

#include "dsoptions.h"
#include "dsmain.h"
#include "gui/dialog.h"
#include "gui/gui-manager.h"
#include "gui/widgets/list.h"
#include "gui/widgets/tab.h"
#include "osystem_ds.h"
#include "engines/scumm/scumm.h"
#include "touchkeyboard.h"
#include "gui/widgets/popup.h"

#include "common/translation.h"

#define ALLOW_CPU_SCALER

namespace DS {

static bool confGetBool(Common::String key, bool defaultVal) {
	if (ConfMan.hasKey(key, "ds"))
		return ConfMan.getBool(key, "ds");
	return defaultVal;
}

static int confGetInt(Common::String key, int defaultVal) {
	if (ConfMan.hasKey(key, "ds"))
		return ConfMan.getInt(key, "ds");
	return defaultVal;
}



DSOptionsDialog::DSOptionsDialog() : GUI::Dialog(0, 0, 320 - 10, 230 - 40) {

	new GUI::ButtonWidget(this, 10, 170, 72, 16, _("~C~lose"), 0, GUI::kCloseCmd);
	new GUI::ButtonWidget(this, 320 - 10 - 130, 170, 120, 16, _("ScummVM Main Menu"), 0, 0x40000000, 'M');

	_tab = new GUI::TabWidget(this, 10, 5, 300, 230 - 20 - 40 - 20);

	_tab->addTab("Controls");

	_leftHandedCheckbox = new GUI::CheckboxWidget(_tab, 5, 5, 130, 20, _("~L~eft handed mode"));
	_indyFightCheckbox = new GUI::CheckboxWidget(_tab, 5, 20, 140, 20, _("~I~ndy fight controls"));
	_showCursorCheckbox = new GUI::CheckboxWidget(_tab, 150, 5, 130, 20, _("Show mouse cursor"), 0, 0, 'T');
	_snapToBorderCheckbox = new GUI::CheckboxWidget(_tab, 150, 20, 130, 20, _("Snap to edges"), 0, 0, 'T');

	new GUI::StaticTextWidget(_tab, 20, 35, 100, 15, _("Touch X Offset"), Graphics::kTextAlignLeft);
	_touchX = new GUI::SliderWidget(_tab, 130, 35, 130, 12, "TODO: Add tooltip", 1);
	_touchX->setMinValue(-8);
	_touchX->setMaxValue(+8);
	_touchX->setValue(0);
	_touchX->setFlags(GUI::WIDGET_CLEARBG);

	new GUI::StaticTextWidget(_tab, 20, 50, 100, 15, _("Touch Y Offset"), Graphics::kTextAlignLeft);
	_touchY = new GUI::SliderWidget(_tab, 130, 50, 130, 12, "TODO: Add tooltip", 2);
	_touchY->setMinValue(-8);
	_touchY->setMaxValue(+8);
	_touchY->setValue(0);
	_touchY->setFlags(GUI::WIDGET_CLEARBG);

	new GUI::StaticTextWidget(_tab, 130 + 65 - 10, 65, 20, 15, "0", Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_tab, 130 + 130 - 10, 65, 20, 15, "8", Graphics::kTextAlignCenter);
	new GUI::StaticTextWidget(_tab, 130 - 20, 65, 20, 15, "-8", Graphics::kTextAlignCenter);


	_touchPadStyle = new GUI::CheckboxWidget(_tab, 5, 80, 270, 20, _("Use laptop trackpad-style cursor control"), 0, 0x20000001, 'T');
	_screenTaps = new GUI::CheckboxWidget(_tab, 5, 95, 285, 20, _("Tap for left click, double tap right click"), 0, 0x20000002, 'T');

	_sensitivityLabel = new GUI::StaticTextWidget(_tab, 20, 110, 110, 15, _("Sensitivity"), Graphics::kTextAlignLeft);
	_sensitivity = new GUI::SliderWidget(_tab, 130, 110, 130, 12, "TODO: Add tooltip", 1);
	_sensitivity->setMinValue(4);
	_sensitivity->setMaxValue(16);
	_sensitivity->setValue(8);
	_sensitivity->setFlags(GUI::WIDGET_CLEARBG);

	_tab->addTab("Graphics");

	new GUI::StaticTextWidget(_tab, 5, 67, 180, 15, _("Initial top screen scale:"), Graphics::kTextAlignLeft);

	_100PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 82, 80, 20, "100%", "TODO: Add tooltip", 0x30000001, 'T');
	_150PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 97, 80, 20, "150%", "TODO: Add tooltip", 0x30000002, 'T');
	_200PercentCheckbox = new GUI::CheckboxWidget(_tab, 5, 112, 80, 20, "200%", "TODO: Add tooltip", 0x30000003, 'T');

	new GUI::StaticTextWidget(_tab, 5, 5, 180, 15, _("Main screen scaling:"), Graphics::kTextAlignLeft);

	_hardScaler = new GUI::CheckboxWidget(_tab, 5, 20, 270, 20, _("Hardware scale (fast, but low quality)"), 0, 0x10000001, 'T');
	_cpuScaler = new GUI::CheckboxWidget(_tab, 5, 35, 270, 20, _("Software scale (good quality, but slower)"), 0, 0x10000002, 'S');
	_unscaledCheckbox = new GUI::CheckboxWidget(_tab, 5, 50, 270, 20, _("Unscaled (you must scroll left and right)"), 0, 0x10000003, 'S');

	new GUI::StaticTextWidget(_tab, 5, 125, 110, 15, _("Brightness:"), Graphics::kTextAlignLeft);
	_gammaCorrection = new GUI::SliderWidget(_tab, 130, 120, 130, 12, "TODO: Add tooltip", 1);
	_gammaCorrection->setMinValue(0);
	_gammaCorrection->setMaxValue(8);
	_gammaCorrection->setValue(0);



	_tab->addTab("General");

	_highQualityAudioCheckbox = new GUI::CheckboxWidget(_tab, 5, 5, 250, 20, _("High quality audio (slower) (reboot)"), 0, 0, 'T');
	_disablePowerOff = new GUI::CheckboxWidget(_tab, 5, 20, 200, 20, _("Disable power off"), 0, 0, 'T');

	_tab->setActiveTab(0);

	_radioButtonMode = false;

//	new GUI::StaticTextWidget(this, 90, 10, 130, 15, "ScummVM DS Options", Graphics::kTextAlignCenter);


//#ifdef ALLOW_CPU_SCALER
//	_cpuScaler = new GUI::CheckboxWidget(this, 160, 115, 90, 20, "CPU scaler", 0, 0, 'T');
//#endif


#ifdef DS_BUILD_D
	_snapToBorderCheckbox->setState(confGetBool("snaptoborder", true));
#else
	_snapToBorderCheckbox->setState(confGetBool("snaptoborder", false));
#endif

	_showCursorCheckbox->setState(confGetBool("showcursor", true));
	_leftHandedCheckbox->setState(confGetBool("lefthanded", false));
	_unscaledCheckbox->setState(confGetBool("unscaled", false));


	if (ConfMan.hasKey("topscreenzoom", "ds")) {

		_100PercentCheckbox->setState(false);
		_150PercentCheckbox->setState(false);
		_200PercentCheckbox->setState(false);

		switch (ConfMan.getInt("topscreenzoom", "ds")) {
			case 100: {
				_100PercentCheckbox->setState(true);
				break;
			}

			case 150: {
				_150PercentCheckbox->setState(true);
				break;
			}

			case 200: {
				_200PercentCheckbox->setState(true);
				break;
			}
		}

	} else if (ConfMan.hasKey("twohundredpercent", "ds")) {
		_200PercentCheckbox->setState(ConfMan.getBool("twohundredpercent", "ds"));
	} else {
		// No setting
		_150PercentCheckbox->setState(true);
	}

	if (ConfMan.hasKey("gamma", "ds")) {
		_gammaCorrection->setValue(ConfMan.getInt("gamma", "ds"));
	} else {
		_gammaCorrection->setValue(0);
	}

	_highQualityAudioCheckbox->setState(confGetBool("22khzaudio", false));
	_disablePowerOff->setState(confGetBool("disablepoweroff", false));

    #ifdef ALLOW_CPU_SCALER
	_cpuScaler->setState(confGetBool("cpu_scaler", false));
    #endif

	_indyFightCheckbox->setState(DS::getIndyFightState());

	_touchX->setValue(confGetInt("xoffset", 0));
	_touchY->setValue(confGetInt("yoffset", 0));
	_sensitivity->setValue(confGetInt("sensitivity", 8));

	_touchPadStyle->setState(confGetBool("touchpad", false));
	_screenTaps->setState(confGetBool("screentaps", false));

	_screenTaps->setEnabled(!_touchPadStyle->getState());
	_sensitivity->setEnabled(_touchPadStyle->getState());
	_sensitivityLabel->setEnabled(_touchPadStyle->getState());
	_sensitivityLabel->draw();

	if (!_cpuScaler->getState() && !_unscaledCheckbox->getState()) {
		_hardScaler->setState(true);
	}

	_radioButtonMode = true;
}

DSOptionsDialog::~DSOptionsDialog() {
	DS::setIndyFightState(_indyFightCheckbox->getState());
	ConfMan.flushToDisk();
}

void DSOptionsDialog::updateConfigManager() {
	ConfMan.setBool("lefthanded", _leftHandedCheckbox->getState(), "ds");
	ConfMan.setBool("unscaled", _unscaledCheckbox->getState(), "ds");
//	ConfMan.setBool("twohundredpercent", _twoHundredPercentCheckbox->getState(), "ds");
	ConfMan.setBool("22khzaudio", _highQualityAudioCheckbox->getState(), "ds");
	ConfMan.setBool("disablepoweroff", _disablePowerOff->getState(), "ds");
#ifdef ALLOW_CPU_SCALER
	ConfMan.setBool("cpu_scaler", _cpuScaler->getState(), "ds");
#endif
	ConfMan.setInt("xoffset", _touchX->getValue(), "ds");
	ConfMan.setInt("yoffset", _touchY->getValue(), "ds");
	ConfMan.setBool("showcursor", _showCursorCheckbox->getState(), "ds");
	ConfMan.setBool("snaptoborder", _snapToBorderCheckbox->getState(), "ds");
	ConfMan.setBool("touchpad", _touchPadStyle->getState(), "ds");
	ConfMan.setBool("screentaps", _screenTaps->getState(), "ds");
	ConfMan.setInt("sensitivity", _sensitivity->getValue(), "ds");
	ConfMan.setInt("gamma", _gammaCorrection->getValue(), "ds");

	u32 zoomLevel = 150;

	if (_100PercentCheckbox->getState()) {
		zoomLevel = 100;
	} else if (_150PercentCheckbox->getState()) {
		zoomLevel = 150;
	} else if (_200PercentCheckbox->getState()) {
		zoomLevel = 200;
	}

	consolePrintf("Saved zoom: %d\n", zoomLevel);

	ConfMan.setInt("topscreenzoom", zoomLevel, "ds");

	DS::setOptions();
}

void DSOptionsDialog::handleCommand(GUI::CommandSender *sender, uint32 cmd, uint32 data) {

	static bool guard = false;

	if ((!guard) && (_radioButtonMode)) {
		guard = true;

		if ((cmd & 0xFF000000) == 0x10000000) {
			_cpuScaler->setState(false);
			_hardScaler->setState(false);
			_unscaledCheckbox->setState(false);

			if ((sender == _cpuScaler) && (cmd == 0x10000002)) {
				_cpuScaler->setState(true);
			}

			if ((sender == _hardScaler) && (cmd == 0x10000001)) {
				_hardScaler->setState(true);
			}

			if ((sender == _unscaledCheckbox) && (cmd == 0x10000003)) {
				_unscaledCheckbox->setState(true);
			}
		}

		guard = false;

	}


	if ((!guard) && (_radioButtonMode)) {
		guard = true;

		if ((sender == _touchPadStyle) && (cmd == 0x20000001)) {

			if (_touchPadStyle->getState()) {
				// Swap screens when turning on trackpad style, it feels
				// much more natural!
				DS::setGameScreenSwap(true);

				_screenTaps->setState(true);
				_screenTaps->setEnabled(false);
				_screenTaps->draw();
				_sensitivity->setEnabled(true);
				_sensitivityLabel->setEnabled(true);
				_sensitivityLabel->draw();
				_sensitivity->draw();
			} else {
				DS::setGameScreenSwap(false);

				_screenTaps->setEnabled(true);
				_screenTaps->setState(false);
				_screenTaps->draw();
				_sensitivity->setEnabled(false);
				_sensitivityLabel->setEnabled(false);
				_sensitivityLabel->draw();
				_sensitivity->draw();
			}
		}

		guard = false;
	}

	if ((!guard) && (_radioButtonMode)) {

		guard = true;

		if (cmd == 0x30000001) {
			_100PercentCheckbox->setState(true);
			_150PercentCheckbox->setState(false);
			_200PercentCheckbox->setState(false);
			DS::setTopScreenZoom(100);
		}

		if (cmd == 0x30000002) {
			_100PercentCheckbox->setState(false);
			_150PercentCheckbox->setState(true);
			_200PercentCheckbox->setState(false);
			DS::setTopScreenZoom(150);
		}

		if (cmd == 0x30000003) {
			_100PercentCheckbox->setState(false);
			_150PercentCheckbox->setState(false);
			_200PercentCheckbox->setState(true);
			DS::setTopScreenZoom(200);
		}

		guard = false;

	}


	if (cmd == GUI::kCloseCmd) {
		updateConfigManager();
		close();
	}


	if ((!guard) && (cmd == 0x40000000)) {
		close();
		g_engine->openMainMenuDialog();
	}
}


void showOptionsDialog() {


	DS::displayMode16Bit();


	DSOptionsDialog *d = new DSOptionsDialog();
	d->runModal();
	delete d;

	DS::displayMode8Bit();

}

void setOptions() {
	static bool firstLoad = true;

	ConfMan.addGameDomain("ds");

	DS::setLeftHanded(confGetBool("lefthanded", false));
	DS::setMouseCursorVisible(confGetBool("showcursor", true));

#ifdef DS_BUILD_D
	DS::setSnapToBorder(confGetBool("snaptoborder", true));
#else
	DS::setSnapToBorder(confGetBool("snaptoborder", false));
#endif

	DS::setUnscaledMode(confGetBool("unscaled", false));

	if (firstLoad) {
		if (ConfMan.hasKey("topscreenzoom", "ds")) {
			DS::setTopScreenZoom(ConfMan.getInt("topscreenzoom", "ds"));
		} else {
			if (ConfMan.hasKey("twohundredpercent", "ds")) {
				DS::setTopScreenZoom(200);
			} else {
				DS::setTopScreenZoom(150);
			}
		}
	}

	DS::setTouchXOffset(confGetInt("xoffset", 0));
	DS::setTouchYOffset(confGetInt("yoffset", 0));
	DS::setSensitivity(confGetInt("sensitivity", 8));

#ifdef ALLOW_CPU_SCALER
	DS::setCpuScalerEnable(confGetBool("cpu_scaler", false));
#endif

	DS::setTapScreenClicksEnable(confGetBool("screentaps", false));

	DS::setGamma(confGetInt("gamma", 0));


	if (ConfMan.hasKey("touchpad", "ds")) {
		bool enable = ConfMan.getBool("touchpad", "ds");

		DS::setTrackPadStyleEnable(enable);

		if (enable && firstLoad) {
			// If we've just booted up, want to swap screens when trackpad mode is in use
			// but not every time we enter the options dialog.
			DS::setGameScreenSwap(true);
		}

		if (enable) {
			DS::setTapScreenClicksEnable(true);
		}

	} else {
		DS::setTrackPadStyleEnable(false);
	}


	firstLoad = false;
}

}	// End of namespace DS
