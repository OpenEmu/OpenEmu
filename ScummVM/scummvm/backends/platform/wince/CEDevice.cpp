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

#include "CEDevice.h"

#include "backends/platform/sdl/sdl-sys.h"

#include "backends/platform/wince/wince-sdl.h"

static void (WINAPI *_SHIdleTimerReset)(void) = NULL;
static HANDLE(WINAPI *_SetPowerRequirement)(PVOID, int, ULONG, PVOID, ULONG) = NULL;
static DWORD (WINAPI *_ReleasePowerRequirement)(HANDLE) = NULL;
static HANDLE _hPowerManagement = NULL;
static DWORD _lastTime = 0;
static DWORD REG_bat = 0, REG_ac = 0, REG_disp = 0, bat_timeout = 0;
static bool REG_tampered = false;
#ifdef __GNUC__
extern "C" void WINAPI SystemIdleTimerReset(void);
#define SPI_SETBATTERYIDLETIMEOUT   251
#define SPI_GETBATTERYIDLETIMEOUT   252
#endif

#define TIMER_TRIGGER 9000

DWORD CEDevice::reg_access(const TCHAR *key, const TCHAR *val, DWORD data) {
	HKEY regkey;
	DWORD tmpval, cbdata;

	if (RegOpenKeyEx(HKEY_CURRENT_USER, key, 0, 0, &regkey) != ERROR_SUCCESS)
		return data;

	cbdata = sizeof(DWORD);
	if (RegQueryValueEx(regkey, val, NULL, NULL, (LPBYTE) &tmpval, &cbdata) != ERROR_SUCCESS) {
		RegCloseKey(regkey);
		return data;
	}

	cbdata = sizeof(DWORD);
	if (RegSetValueEx(regkey, val, 0, REG_DWORD, (LPBYTE) &data, cbdata) != ERROR_SUCCESS) {
		RegCloseKey(regkey);
		return data;
	}

	RegCloseKey(regkey);
	return tmpval;
}

void CEDevice::backlight_xchg() {
	HANDLE h;

	REG_bat = reg_access(TEXT("ControlPanel\\BackLight"), (const TCHAR *)TEXT("BatteryTimeout"), REG_bat);
	REG_ac = reg_access(TEXT("ControlPanel\\BackLight"), TEXT("ACTimeout"), REG_ac);
	REG_disp = reg_access(TEXT("ControlPanel\\Power"), TEXT("Display"), REG_disp);

	h = CreateEvent(NULL, FALSE, FALSE, TEXT("BackLightChangeEvent"));
	if (h) {
		SetEvent(h);
		CloseHandle(h);
	}
}

void CEDevice::init() {
	// 2003+ power management code borrowed from MoDaCo & Betaplayer. Thanks !
	HINSTANCE dll = LoadLibrary(TEXT("aygshell.dll"));
	if (dll) {
		_SHIdleTimerReset = (void (*)())GetProcAddress(dll, MAKEINTRESOURCE(2006));
	}
	dll = LoadLibrary(TEXT("coredll.dll"));
	if (dll) {
		_SetPowerRequirement = (HANDLE (*)(PVOID, int, ULONG, PVOID, ULONG))GetProcAddress(dll, TEXT("SetPowerRequirement"));
		_ReleasePowerRequirement = (DWORD (*)(HANDLE))GetProcAddress(dll, TEXT("ReleasePowerRequirement"));
	}
	if (_SetPowerRequirement)
		_hPowerManagement = _SetPowerRequirement((PVOID) TEXT("BKL1:"), 0, 1, (PVOID) NULL, 0);
	_lastTime = GetTickCount();

	// older devices
	REG_bat = REG_ac = REG_disp = 2 * 60 * 60 * 1000;   // 2hrs should do it
	backlight_xchg();
	REG_tampered = true;
	SystemParametersInfo(SPI_GETBATTERYIDLETIMEOUT, 0, (void *) &bat_timeout, 0);
	SystemParametersInfo(SPI_SETBATTERYIDLETIMEOUT, 60 * 60 * 2, NULL, SPIF_SENDCHANGE);
}

void CEDevice::end() {
	if (_ReleasePowerRequirement && _hPowerManagement)
		_ReleasePowerRequirement(_hPowerManagement);
	if (REG_tampered)
		backlight_xchg();
	SystemParametersInfo(SPI_SETBATTERYIDLETIMEOUT, bat_timeout, NULL, SPIF_SENDCHANGE);
}

void CEDevice::wakeUp() {
	DWORD currentTime = GetTickCount();
	if (currentTime > _lastTime + TIMER_TRIGGER) {
		_lastTime = currentTime;
		SystemIdleTimerReset();
		if (_SHIdleTimerReset)
			_SHIdleTimerReset();
	}
}

bool CEDevice::hasSquareQVGAResolution() {
	return (OSystem_WINCE3::getScreenWidth() == 240 && OSystem_WINCE3::getScreenHeight() == 240);
}

bool CEDevice::hasWideResolution() {
	return (OSystem_WINCE3::getScreenWidth() >= 640 || OSystem_WINCE3::getScreenHeight() >= 640);
}

bool CEDevice::hasPocketPCResolution() {
	if (OSystem_WINCE3::isOzone() && hasWideResolution())
		return true;
	return (OSystem_WINCE3::getScreenWidth() <= 320 && OSystem_WINCE3::getScreenWidth() >= 240);
}

bool CEDevice::hasDesktopResolution() {
	if (OSystem_WINCE3::isOzone() && hasWideResolution())
		return true;
	return (OSystem_WINCE3::getScreenWidth() > 320);
}

bool CEDevice::hasSmartphoneResolution() {
	return (OSystem_WINCE3::getScreenWidth() < 240);
}

bool CEDevice::isSmartphone() {
	TCHAR platformType[100];
	BOOL result = SystemParametersInfo(SPI_GETPLATFORMTYPE, sizeof(platformType), platformType, 0);
	if (!result && GetLastError() == ERROR_ACCESS_DENIED)
		return true;
	return (_wcsnicmp(platformType, TEXT("SmartPhone"), 10) == 0);
}
