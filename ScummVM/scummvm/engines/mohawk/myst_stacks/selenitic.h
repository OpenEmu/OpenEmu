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

#ifndef MYST_SCRIPTS_SELENITIC_H
#define MYST_SCRIPTS_SELENITIC_H

#include "common/scummsys.h"
#include "common/util.h"
#include "mohawk/myst_scripts.h"

namespace Mohawk {

class MystResourceType8;
struct MystScriptEntry;

namespace MystStacks {

#define DECLARE_OPCODE(x) void x(uint16 op, uint16 var, uint16 argc, uint16 *argv)

class Selenitic : public MystScriptParser {
public:
	Selenitic(MohawkEngine_Myst *vm);
	~Selenitic();

	void disablePersistentScripts();
	void runPersistentScripts();

private:
	void setupOpcodes();
	uint16 getVar(uint16 var);
	void toggleVar(uint16 var);
	bool setVarValue(uint16 var, uint16 value);

	virtual uint16 getMap() { return 9930; }

	DECLARE_OPCODE(o_mazeRunnerMove);
	DECLARE_OPCODE(o_mazeRunnerSoundRepeat);
	DECLARE_OPCODE(o_soundReceiverSigma);
	DECLARE_OPCODE(o_soundReceiverRight);
	DECLARE_OPCODE(o_soundReceiverLeft);
	DECLARE_OPCODE(o_soundReceiverSource);
	DECLARE_OPCODE(o_mazeRunnerDoorButton);
	DECLARE_OPCODE(o_soundReceiverUpdateSound);
	DECLARE_OPCODE(o_soundLockMove);
	DECLARE_OPCODE(o_soundLockStartMove);
	DECLARE_OPCODE(o_soundLockEndMove);
	DECLARE_OPCODE(o_soundLockButton);
	DECLARE_OPCODE(o_soundReceiverEndMove);

	DECLARE_OPCODE(o_mazeRunnerCompass_init);
	DECLARE_OPCODE(o_mazeRunnerWindow_init);
	DECLARE_OPCODE(o_mazeRunnerLight_init);
	DECLARE_OPCODE(o_soundReceiver_init);
	DECLARE_OPCODE(o_soundLock_init);
	DECLARE_OPCODE(o_mazeRunnerRight_init);
	DECLARE_OPCODE(o_mazeRunnerLeft_init);

	void soundReceiver_run();

	MystGameState::Selenitic &_state;

	bool _soundReceiverRunning;
	bool _soundReceiverSigmaPressed; // 6
	MystResourceType8 *_soundReceiverSources[5]; // 92 -> 108
	MystResourceType8 *_soundReceiverCurrentSource; // 112
	uint16 *_soundReceiverPosition; // 116
	uint16 _soundReceiverDirection; // 120
	uint16 _soundReceiverSpeed; // 122
	uint32 _soundReceiverStartTime; //124
	MystResourceType8 *_soundReceiverViewer; // 128
	MystResourceType8 *_soundReceiverRightButton; // 132
	MystResourceType8 *_soundReceiverLeftButton; // 136
	MystResourceType8 *_soundReceiverAngle1; // 140
	MystResourceType8 *_soundReceiverAngle2; // 144
	MystResourceType8 *_soundReceiverAngle3; // 148
	MystResourceType8 *_soundReceiverAngle4; // 152
	MystResourceType8 *_soundReceiverSigmaButton; // 156

	static const uint16 _mazeRunnerMap[300][4];
	static const uint8 _mazeRunnerVideos[300][4];

	uint16 _mazeRunnerPosition; // 56
	uint16 _mazeRunnerDirection; // 58
	MystResourceType8 *_mazeRunnerWindow; // 68
	MystResourceType8 *_mazeRunnerCompass; // 72
	MystResourceType8 *_mazeRunnerLight; // 76
	MystResourceType8 *_mazeRunnerRightButton; // 80
	MystResourceType8 *_mazeRunnerLeftButton; // 84
	bool _mazeRunnerDoorOpened; // 160

	uint16 _soundLockSoundId;
	MystResourceType10 *_soundLockSlider1; // 164
	MystResourceType10 *_soundLockSlider2; // 168
	MystResourceType10 *_soundLockSlider3; // 172
	MystResourceType10 *_soundLockSlider4; // 176
	MystResourceType10 *_soundLockSlider5; // 180
	MystResourceType8 *_soundLockButton; // 184

	void soundReceiverLeftRight(uint direction);
	void soundReceiverUpdate();
	void soundReceiverDrawView();
	void soundReceiverDrawAngle();
	void soundReceiverIncreaseSpeed();
	void soundReceiverUpdateSound();
	uint16 soundReceiverCurrentSound(uint16 source, uint16 position);
	void soundReceiverSolution(uint16 source, uint16 &solution, bool &enabled);

	uint16 soundLockCurrentSound(uint16 position, bool pixels);
	MystResourceType10 *soundLockSliderFromVar(uint16 var);
	void soundLockCheckSolution(MystResourceType10 *slider, uint16 value, uint16 solution, bool &solved);

	bool mazeRunnerForwardAllowed(uint16 position);
	void mazeRunnerUpdateCompass();
	void mazeRunnerPlaySoundHelp();
	void mazeRunnerPlayVideo(uint16 video, uint16 pos);
	void mazeRunnerBacktrack(uint16 &oldPosition);
};

} // End of namespace MystStacks
}

#undef DECLARE_OPCODE

#endif
