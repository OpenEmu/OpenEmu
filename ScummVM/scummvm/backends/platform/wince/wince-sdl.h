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

#ifndef WINCE_SDL_H
#define WINCE_SDL_H

#include "common/scummsys.h"
#include "common/system.h"
#include "graphics/scaler.h"
#include "backends/platform/sdl/sdl.h"

#include "backends/platform/wince/CEgui/CEGUI.h"
#include "backends/platform/wince/CEkeys/CEKeys.h"
#include "backends/platform/wince/CEDevice.h"

#include "backends/graphics/wincesdl/wincesdl-graphics.h"
#include "backends/events/wincesdl/wincesdl-events.h"
#include "backends/timer/default/default-timer.h"
#include "backends/fs/windows/windows-fs-factory.h"

// defines used for implementing the raw frame buffer access method (2003+)
#define GETRAWFRAMEBUFFER   0x00020001
#define FORMAT_565 1
#define FORMAT_555 2
#define FORMAT_OTHER 3

class OSystem_WINCE3 : public OSystem_SDL {
public:
	OSystem_WINCE3();
	virtual ~OSystem_WINCE3();

	void setGraphicsModeIntern();
	void initBackend();

	// Overloaded from SDL backend
	void init();
	void quit();
	virtual Common::String getSystemLanguage() const;

	// Overloaded from OSystem
	void engineInit();
	void getTimeAndDate(TimeDate &t) const;

	virtual Common::String getDefaultConfigFileName();

	void swap_sound_master();

	static int getScreenWidth();
	static int getScreenHeight();
	static void initScreenInfos();
	static bool isOzone();

	static bool _soundMaster;   // turn off sound after all calculations
	// static since needed by the SDL callback

protected:
	void initSDL();
	Audio::MixerImpl *_mixer;

private:
	void check_mappings();

	bool _forcePanelInvisible;  // force panel visibility for some cases

	static int _platformScreenWidth;
	static int _platformScreenHeight;
	static bool _isOzone;       // true if running on Windows 2003 SE

};

#endif
