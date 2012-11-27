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

#ifndef KYRA_DEBUGGER_H
#define KYRA_DEBUGGER_H

#include "gui/debugger.h"

namespace Kyra {

class KyraEngine_v1;
class KyraEngine_LoK;
class KyraEngine_v2;
class KyraEngine_HoF;

class Debugger : public ::GUI::Debugger {
public:
	Debugger(KyraEngine_v1 *vm);
	virtual ~Debugger() {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

	virtual void initialize();

protected:
	KyraEngine_v1 *_vm;

	bool cmd_setScreenDebug(int argc, const char **argv);
	bool cmd_loadPalette(int argc, const char **argv);
	bool cmd_showFacings(int argc, const char **argv);
	bool cmd_gameSpeed(int argc, const char **argv);
	bool cmd_listFlags(int argc, const char **argv);
	bool cmd_toggleFlag(int argc, const char **argv);
	bool cmd_queryFlag(int argc, const char **argv);
	bool cmd_listTimers(int argc, const char **argv);
	bool cmd_setTimerCountdown(int argc, const char **argv);
};

class Debugger_LoK : public Debugger {
public:
	Debugger_LoK(KyraEngine_LoK *vm);
	virtual ~Debugger_LoK() {}  // we need this for __SYMBIAN32__ archaic gcc/UIQ

	virtual void initialize();
protected:
	KyraEngine_LoK *_vm;

	bool cmd_enterRoom(int argc, const char **argv);
	bool cmd_listScenes(int argc, const char **argv);
	bool cmd_giveItem(int argc, const char **argv);
	bool cmd_listBirthstones(int argc, const char **argv);
};

class Debugger_v2 : public Debugger {
public:
	Debugger_v2(KyraEngine_v2 *vm);
	virtual ~Debugger_v2() {}

	virtual void initialize();
protected:
	KyraEngine_v2 *_vm;

	bool cmd_enterScene(int argc, const char **argv);
	bool cmd_listScenes(int argc, const char **argv);
	bool cmd_sceneInfo(int argc, const char **argv);
	bool cmd_characterInfo(int argc, const char **argv);
	bool cmd_sceneToFacing(int argc, const char **argv);
	bool cmd_giveItem(int argc, const char **argv);
};

class Debugger_HoF : public Debugger_v2 {
public:
	Debugger_HoF(KyraEngine_HoF *vm);

	virtual void initialize();
protected:
	KyraEngine_HoF *_vm;

	bool cmd_passcodes(int argc, const char **argv);
};

#ifdef ENABLE_LOL
class LoLEngine;

class Debugger_LoL : public Debugger {
public:
	Debugger_LoL(LoLEngine *vm);

protected:
	LoLEngine *_vm;
};
#endif // ENABLE_LOL

#ifdef ENABLE_EOB
class EoBCoreEngine;

class Debugger_EoB : public Debugger {
public:
	Debugger_EoB(EoBCoreEngine *vm);

	virtual void initialize();
protected:
	EoBCoreEngine *_vm;

	bool cmd_importSaveFile(int argc, const char **argv);
};
#endif // ENABLE_EOB

} // End of namespace Kyra

#endif
