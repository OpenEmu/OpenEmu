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

#ifndef MYST_SCRIPTS_STONESHIP_H
#define MYST_SCRIPTS_STONESHIP_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class Stoneship : public MystScriptParser {
public:
	Stoneship(MohawkEngine_Myst *vm);
	~Stoneship();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	virtual uint16 getMap() { return 9933; }

	DECLARE_OPCODE(o_pumpTurnOff);
	DECLARE_OPCODE(o_brotherDoorOpen);
	DECLARE_OPCODE(o_cabinBookMovie);
	DECLARE_OPCODE(o_drawerOpenSirius);
	DECLARE_OPCODE(o_drawerClose);
	DECLARE_OPCODE(o_telescopeStart);
	DECLARE_OPCODE(o_telescopeMove);
	DECLARE_OPCODE(o_telescopeStop);
	DECLARE_OPCODE(o_generatorStart);
	DECLARE_OPCODE(o_generatorStop);
	DECLARE_OPCODE(o_drawerOpenAchenar);
	DECLARE_OPCODE(o_hologramPlayback);
	DECLARE_OPCODE(o_hologramSelectionStart);
	DECLARE_OPCODE(o_hologramSelectionMove);
	DECLARE_OPCODE(o_hologramSelectionStop);
	DECLARE_OPCODE(o_compassButton);
	DECLARE_OPCODE(o_chestValveVideos);
	DECLARE_OPCODE(o_chestDropKey);
	DECLARE_OPCODE(o_trapLockOpen);
	DECLARE_OPCODE(o_sideDoorsMovies);
	DECLARE_OPCODE(o_cloudOrbEnter);
	DECLARE_OPCODE(o_cloudOrbLeave);
	DECLARE_OPCODE(o_drawerCloseOpened);

	DECLARE_OPCODE(o_hologramDisplay_init);
	DECLARE_OPCODE(o_hologramSelection_init);
	DECLARE_OPCODE(o_battery_init);
	DECLARE_OPCODE(o_tunnelEnter_init);
	DECLARE_OPCODE(o_batteryGauge_init);
	DECLARE_OPCODE(o_tunnel_init);
	DECLARE_OPCODE(o_tunnelLeave_init);
	DECLARE_OPCODE(o_chest_init);
	DECLARE_OPCODE(o_telescope_init);
	DECLARE_OPCODE(o_achenarDrawers_init);
	DECLARE_OPCODE(o_cloudOrb_init);

	void chargeBattery_run();
	void batteryDeplete_run();

	MystGameState::Stoneship &_state;

	bool _batteryCharging;
	bool _batteryDepleting;
	uint32 _batteryNextTime;

	bool _batteryGaugeRunning;
	uint16 _batteryLastCharge; // 92
	MystResourceType8 *_batteryGauge; // 96
	void batteryGaugeUpdate();
	void batteryGauge_run();

	uint16 _cabinMystBookPresent; // 64

	uint16 _siriusDrawerDrugsOpen; // 72

	uint16 _brotherDoorOpen; // 76

	uint16 _chestDrawersOpen; // 78
	uint16 _chestAchenarBottomDrawerClosed; // 144
	void drawerClose(uint16 drawer);

	uint16 _hologramTurnedOn; // 80
	MystResourceType6 *_hologramDisplay; // 84
	MystResourceType6 *_hologramSelection; // 88
	uint16 _hologramDisplayPos;

	bool _tunnelRunning;
	uint32 _tunnelNextTime;
	uint16 _tunnelAlarmSound; // 100
	uint16 _tunnelImagesCount; // 22
	uint16 _tunnelImages[2]; // 32
	void tunnel_run();

	uint16 _telescopePosition; // 112
	uint16 _telescopePanorama;
	uint16 _telescopeOldMouse;
	uint16 _telescopeLighthouseOff; // 130
	uint16 _telescopeLighthouseOn; // 128
	bool _telescopeLighthouseState; // 124
	bool _telescopeRunning;
	uint32 _telescopeNexTime;
	void telescope_run();
	void telescopeLighthouseDraw();

	MystResourceType6 *_cloudOrbMovie; // 136
	uint16 _cloudOrbSound; // 140
	uint16 _cloudOrbStopSound; // 142

	uint16 batteryRemainingCharge();
};

} // End of namespace MystStacks
}

#undef DECLARE_OPCODE

#endif
