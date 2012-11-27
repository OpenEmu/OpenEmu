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

#ifndef MYST_SCRIPTS_MECHANICAL_H
#define MYST_SCRIPTS_MECHANICAL_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class Mechanical : public MystScriptParser {
public:
	Mechanical(MohawkEngine_Myst *vm);
	~Mechanical();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	virtual uint16 getMap() { return 9931; }

	void birdSing_run();
	void elevatorRotation_run();
	void elevatorGoMiddle_run();
	void fortressRotation_run();
	void fortressSimulation_run();

	DECLARE_OPCODE(o_throneEnablePassage);
	DECLARE_OPCODE(o_birdCrankStart);
	DECLARE_OPCODE(o_birdCrankStop);
	DECLARE_OPCODE(o_snakeBoxTrigger);
	DECLARE_OPCODE(o_fortressStaircaseMovie);
	DECLARE_OPCODE(o_elevatorRotationStart);
	DECLARE_OPCODE(o_elevatorRotationMove);
	DECLARE_OPCODE(o_elevatorRotationStop);
	DECLARE_OPCODE(o_fortressRotationSpeedStart);
	DECLARE_OPCODE(o_fortressRotationSpeedMove);
	DECLARE_OPCODE(o_fortressRotationSpeedStop);
	DECLARE_OPCODE(o_fortressRotationBrakeStart);
	DECLARE_OPCODE(o_fortressRotationBrakeMove);
	DECLARE_OPCODE(o_fortressRotationBrakeStop);
	DECLARE_OPCODE(o_fortressSimulationSpeedStart);
	DECLARE_OPCODE(o_fortressSimulationSpeedMove);
	DECLARE_OPCODE(o_fortressSimulationSpeedStop);
	DECLARE_OPCODE(o_fortressSimulationBrakeStart);
	DECLARE_OPCODE(o_fortressSimulationBrakeMove);
	DECLARE_OPCODE(o_fortressSimulationBrakeStop);
	DECLARE_OPCODE(o_elevatorWindowMovie);
	DECLARE_OPCODE(o_elevatorGoMiddle);
	DECLARE_OPCODE(o_elevatorTopMovie);
	DECLARE_OPCODE(opcode_124);
	DECLARE_OPCODE(o_mystStaircaseMovie);
	DECLARE_OPCODE(opcode_126);
	DECLARE_OPCODE(o_crystalEnterYellow);
	DECLARE_OPCODE(o_crystalEnterGreen);
	DECLARE_OPCODE(o_crystalEnterRed);
	DECLARE_OPCODE(o_crystalLeaveYellow);
	DECLARE_OPCODE(o_crystalLeaveGreen);
	DECLARE_OPCODE(o_crystalLeaveRed);

	DECLARE_OPCODE(o_throne_init);
	DECLARE_OPCODE(o_fortressStaircase_init);
	DECLARE_OPCODE(o_bird_init);
	DECLARE_OPCODE(o_snakeBox_init);
	DECLARE_OPCODE(o_elevatorRotation_init);
	DECLARE_OPCODE(o_fortressRotation_init);
	DECLARE_OPCODE(o_fortressSimulation_init);
	DECLARE_OPCODE(o_fortressSimulationStartup_init);

	MystGameState::Mechanical &_state;

	bool _mystStaircaseState; // 76

	bool _fortressRotationRunning;
	uint16 _fortressRotationSpeed; // 78
	uint16 _fortressRotationBrake; // 80
	uint16 _fortressPosition; // 82
	uint16 _fortressRotationSounds[4]; // 86 to 92
	MystResourceType6 *_fortressRotationGears; // 172

	bool _fortressSimulationRunning;
	uint16 _fortressSimulationSpeed; // 96
	uint16 _fortressSimulationBrake; // 98
	uint16 _fortressSimulationStartSound1; // 102
	uint16 _fortressSimulationStartSound2; // 100
	MystResourceType6 *_fortressSimulationHolo; // 160
	MystResourceType6 *_fortressSimulationStartup; // 164

	uint16 _elevatorGoingDown; // 112

	float _elevatorRotationSpeed; // 120
	float _elevatorRotationGearPosition; // 124
	uint16 _elevatorRotationSoundId; // 128
	bool _elevatorRotationLeverMoving; // 184

	bool _elevatorGoingMiddle; // 148
	bool _elevatorTooLate;
	uint16 _elevatorPosition; // 104
	bool _elevatorInCabin; // 108
	uint16 _elevatorTopCounter;
	uint32 _elevatorNextTime;

	uint16 _crystalLit; // 130

	bool _birdSinging; // 144
	uint32 _birdCrankStartTime; // 136
	uint32 _birdSingEndTime; // 140
	MystResourceType6 *_bird; // 152


	MystResourceType6 *_snakeBox; // 156
};

} // End of namespace MystStacks
} // End of namespace Mohawk

#undef DECLARE_OPCODE

#endif
