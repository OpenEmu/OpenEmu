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

#ifndef MYST_SCRIPTS_CHANNELWOOD_H
#define MYST_SCRIPTS_CHANNELWOOD_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class Channelwood : public MystScriptParser {
public:
	Channelwood(MohawkEngine_Myst *vm);
	~Channelwood();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	virtual uint16 getMap() { return 9932; }

	DECLARE_OPCODE(o_bridgeToggle);
	DECLARE_OPCODE(o_pipeExtend);
	DECLARE_OPCODE(o_drawImageChangeCardAndVolume);
	DECLARE_OPCODE(o_waterTankValveOpen);
	DECLARE_OPCODE(o_leverStartMove);
	DECLARE_OPCODE(o_leverMove);
	DECLARE_OPCODE(o_leverMoveFail);
	DECLARE_OPCODE(o_leverEndMove);
	DECLARE_OPCODE(o_leverEndMoveResumeBackground);
	DECLARE_OPCODE(o_leverEndMoveWithSound);
	DECLARE_OPCODE(o_leverElev3StartMove);
	DECLARE_OPCODE(o_leverElev3EndMove);
	DECLARE_OPCODE(o_pumpLeverMove);
	DECLARE_OPCODE(o_pumpLeverEndMove);
	DECLARE_OPCODE(o_stairsDoorToggle);
	DECLARE_OPCODE(o_valveHandleMove1);
	DECLARE_OPCODE(o_valveHandleMoveStart1);
	DECLARE_OPCODE(o_valveHandleMoveStop);
	DECLARE_OPCODE(o_valveHandleMove2);
	DECLARE_OPCODE(o_valveHandleMoveStart2);
	DECLARE_OPCODE(o_valveHandleMove3);
	DECLARE_OPCODE(o_valveHandleMoveStart3);
	DECLARE_OPCODE(o_hologramMonitor);
	DECLARE_OPCODE(o_drawerOpen);
	DECLARE_OPCODE(o_hologramTemple);
	DECLARE_OPCODE(o_executeMouseUp);
	DECLARE_OPCODE(o_waterTankValveClose);
	DECLARE_OPCODE(o_elevatorMovies);
	DECLARE_OPCODE(o_soundReplace);

	DECLARE_OPCODE(o_lever_init);
	DECLARE_OPCODE(o_pipeValve_init);
	DECLARE_OPCODE(o_drawer_init);

	MystGameState::Channelwood &_state;

	uint16 _valveVar; // 64

	uint16 _siriusDrawerState; // 66

	uint16 _doorOpened; // 68

	bool _leverPulled;
	MystResourceType5 *_leverAction; // 72

	bool pipeChangeValve(bool open, uint16 mask);
};

} // End of namespace MystStacks
} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
