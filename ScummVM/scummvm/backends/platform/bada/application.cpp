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
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#include "engines/engine.h"

#include "backends/platform/bada/form.h"
#include "backends/platform/bada/system.h"
#include "backends/platform/bada/application.h"

using namespace Osp::System;
using namespace Osp::Ui::Controls;

Application *BadaScummVM::createInstance() {
	return new BadaScummVM();
}

BadaScummVM::BadaScummVM() : _appForm(0) {
}

BadaScummVM::~BadaScummVM() {
	logEntered();
	if (g_system) {
		BadaSystem *system = (BadaSystem *)g_system;
		system->destroyBackend();
		delete system;
		g_system = 0;
	}
}

bool BadaScummVM::OnAppInitializing(AppRegistry &appRegistry) {
	_appForm = systemStart(this);
	return (_appForm != NULL);
}

bool BadaScummVM::OnAppTerminating(AppRegistry &appRegistry,
																	 bool forcedTermination) {
	logEntered();
	return true;
}

void BadaScummVM::OnUserEventReceivedN(RequestId requestId,
																			 Osp::Base::Collection::IList *args) {
	logEntered();

	if (requestId == USER_MESSAGE_EXIT) {
		// normal program termination
		Terminate();
	} else if (requestId == USER_MESSAGE_EXIT_ERR) {
		// assertion failure termination
		String *message = NULL;
		if (args) {
			message = (String *)args->GetAt(0);
		}
		if (!message) {
			message = new String("Unknown error");
		}

		MessageBox messageBox;
		messageBox.Construct(L"Oops...", *message, MSGBOX_STYLE_OK);
		int modalResult;
		messageBox.ShowAndWait(modalResult);
		Terminate();
	}
}

void BadaScummVM::OnForeground(void) {
	logEntered();
	pauseGame(false);
}

void BadaScummVM::OnBackground(void) {
	logEntered();
	pauseGame(true);
}

void BadaScummVM::OnBatteryLevelChanged(BatteryLevel batteryLevel) {
}

void BadaScummVM::OnLowMemory(void) {
}

void BadaScummVM::pauseGame(bool pause) {
	if (_appForm) {
		if (pause && g_engine && !g_engine->isPaused()) {
			_appForm->pushKey(Common::KEYCODE_SPACE);
		}

		if (g_system) {
			((BadaSystem *)g_system)->setMute(pause);
		}
	}
}
