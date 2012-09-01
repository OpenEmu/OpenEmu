/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * Inputs.h
 *
 * Header file for CInputs, a class which manages all individual inputs.
 */
 
#ifndef INCLUDED_INPUTS_H
#define INCLUDED_INPUTS_H

#include "Types.h"

#include <vector>
using namespace std;

class CInputSystem;
class CInput;
class CAnalogInput;
class CAxisInput;
class CSwitchInput;
class CGearShift4Input;
class CTriggerInput;
class CINIFile;
struct GameInfo;

/*
 * Represents the collection of Model3 inputs.
 */
class CInputs
{
private:
	// Assigned input system
	CInputSystem *m_system;

	// Vector of all created inputs
	vector<CInput*> m_inputs;

	/*
	 * Adds a switch input (eg button) to this collection.
	 */ 
	CSwitchInput *AddSwitchInput(const char *id, const char *label, unsigned gameFlags, const char *defaultMapping,
		UINT16 offVal = 0x00, UINT16 onVal = 0x01);

	/*
	 * Adds an analog input (eg pedal) to this collection.
	 */
	CAnalogInput *AddAnalogInput(const char *id, const char *label, unsigned gameFlags, const char *defaultMapping, 
		UINT16 minVal = 0x00, UINT16 maxVal = 0xFF);

	/*
	 * Adds an axis input (eg jostick axis, light gun axis or steering wheel) to this collection.
	 */
	CAxisInput *AddAxisInput(const char *id, const char *label, unsigned gameFlags, const char *defaultMapping,
		CAnalogInput *axisNeg, CAnalogInput *axisPos, UINT16 minVal = 0x00, UINT16 offVal = 0x80, UINT16 maxVal = 0xFF);

	/*
	 * Adds a 4-gear shifter input to this collection.
	 */
	CGearShift4Input *AddGearShift4Input(const char *id, const char *label, unsigned gameFlags, 
		CSwitchInput *shift1, CSwitchInput *shift2, CSwitchInput *shift3, CSwitchInput *shift4, CSwitchInput *shiftN, CSwitchInput *shiftUp, CSwitchInput *shiftDown);

	/*
	 * Adds a lightgun trigger input to this collection.
	 */
	CTriggerInput *AddTriggerInput(const char *id, const char *label, unsigned gameFlags, 
		CSwitchInput *trigger, CSwitchInput *offscreen, UINT16 offVal = 0x00, UINT16 onVal = 0x01);

	void PrintHeader(const char *fmt, ...);

	void PrintConfigureInputsHelp();

public:
	// UI controls
	CSwitchInput    *uiExit;
	CSwitchInput    *uiReset;
	CSwitchInput    *uiPause;
	CSwitchInput    *uiSaveState;
	CSwitchInput    *uiChangeSlot;
	CSwitchInput    *uiLoadState;
	CSwitchInput	*uiMusicVolUp;
	CSwitchInput	*uiMusicVolDown;
	CSwitchInput	*uiSoundVolUp;
	CSwitchInput	*uiSoundVolDown;
	CSwitchInput    *uiDumpInpState;
	CSwitchInput    *uiClearNVRAM;
	CSwitchInput    *uiSelectCrosshairs;
	CSwitchInput	*uiToggleFrLimit;
#ifdef SUPERMODEL_DEBUGGER
	CSwitchInput     *uiEnterDebugger;
#endif

	// Common controls between all games
	CSwitchInput    *coin[2];
	CSwitchInput    *start[2];
	CSwitchInput    *test[2];
	CSwitchInput    *service[2];

	// Joysticks (players 1 and 2)
	CSwitchInput    *up[2];
	CSwitchInput    *down[2];
	CSwitchInput    *left[2];
	CSwitchInput    *right[2];

	// Fighting game controls (players 1 and 2)
	CSwitchInput    *punch[2];
	CSwitchInput    *kick[2];
	CSwitchInput    *guard[2];
	CSwitchInput    *escape[2];
	
	// Spikeout controls
	CSwitchInput	*shift;
	CSwitchInput	*beat;
	CSwitchInput	*charge;
	CSwitchInput	*jump;

	// Soccer game controls (players 1 and 2)
	CSwitchInput    *shortPass[2];
	CSwitchInput    *longPass[2];
	CSwitchInput    *shoot[2];

	// Vehicle controls
	CAxisInput      *steering;
	CAnalogInput    *accelerator;
	CAnalogInput    *brake;

	// VR view buttons: VR1 Red, VR2 Blue, VR3 Yellow, VR4 Green
	CSwitchInput    *vr[4];
 
	// 4-speed gear shift
	CGearShift4Input *gearShift4;

	// Rally controls
	CSwitchInput	*viewChange;
	CSwitchInput	*handBrake;

	// Twin joysticks (individually mapped version; 1 = left stick, 2 = right stick)
	CSwitchInput	*twinJoyLeft1;
	CSwitchInput	*twinJoyLeft2;
	CSwitchInput	*twinJoyRight1;
	CSwitchInput	*twinJoyRight2;
	CSwitchInput	*twinJoyUp1;
	CSwitchInput	*twinJoyUp2;
	CSwitchInput	*twinJoyDown1;
	CSwitchInput	*twinJoyDown2;
	CSwitchInput	*twinJoyShot1;
	CSwitchInput    *twinJoyShot2;
	CSwitchInput	*twinJoyTurbo1;
	CSwitchInput    *twinJoyTurbo2;
	
	// Twin joysticks (macro mapping, for users w/out dual joysticks) 
	CSwitchInput	*twinJoyTurnLeft;
	CSwitchInput    *twinJoyTurnRight;
	CSwitchInput	*twinJoyStrafeLeft;
	CSwitchInput    *twinJoyStrafeRight;
	CSwitchInput	*twinJoyForward;
	CSwitchInput    *twinJoyReverse;
	CSwitchInput	*twinJoyJump;
	CSwitchInput    *twinJoyCrouch;
	
	// Analog joystick
	CAxisInput      *analogJoyX;
	CAxisInput      *analogJoyY;
	CSwitchInput	*analogJoyTrigger;
	CSwitchInput	*analogJoyEvent;

	// Gun controls (players 1 and 2)
	CAxisInput      *gunX[2];
	CAxisInput      *gunY[2];
	CTriggerInput   *trigger[2];
	
	/*
	 * Creates a set of inputs with the given input system.
	 */
	CInputs(CInputSystem *system);
	
	/*
	 * CInputs destructor.
	 */
	~CInputs();

	/*
	 * Returns the number of available inputs.
	 */
	unsigned Count();

	/*
	 * Returns the input with the given index.
	 */
	CInput *operator[](const unsigned index);

	/*
	 * Returns the input with the given id or label.
	 */
	CInput *operator[](const char *idOrLabel);

	/*
	 * Returns the assigned input system.
	 */
	CInputSystem *GetInputSystem();

	/*
	 * Initializes the inputs.  Must be called before any other methods are used.
	 */ 
	bool Initialize();

	/*
	 * Reads the input mapping assignments from the given INI file.
	 */
	void ReadFromINIFile(CINIFile *ini, const char *section);

	/*
	 * Writes the current input mapping assignments to the given INI file.
	 */
	void WriteToINIFile(CINIFile *ini, const char *section);

	/*
	 * Configures the current input mapping assignments for the given game, or all inputs if game is NULL, by asking the user for input.
	 * Returns true if the inputs were configured okay or false if the user exited without requesting to save changes.
	 */
	bool ConfigureInputs(const GameInfo *game);
	
	/*
	 * Configures the current input mapping assignments for the given game, or all inputs if game is NULL, by asking the user for input.
	 * Takes display geometry if this has not been set previously by a call to Poll().
	 * Returns true if the inputs were configured okay or false if the user exited without requesting to save changes.
	 */
	bool ConfigureInputs(const GameInfo *game, unsigned dispX, unsigned dispY, unsigned dispW, unsigned dispH);

	void CalibrateJoysticks();

	void CalibrateJoystick(int joyNum);

	/*
	 * Prints to stdout the current input mapping assignments for the given game, or all inputs if game is NULL.
	 */
	void PrintInputs(const GameInfo *game);

	/*
	 * Polls (updates) the inputs for the given game, or all inputs if game is NULL, updating their values from their respective input sources.
	 * First the input system is polled (CInputSystem.Poll()) and then each input is polled (CInput.Poll()).
	 */
	bool Poll(const GameInfo *game, unsigned dispX, unsigned dispY, unsigned dispW, unsigned dispH);

	/*
	 * Prints the current values of the inputs for the given game, or all inputs if game is NULL, to stdout for debugging purposes.
	 */
	void DumpState(const GameInfo *game);
};

#endif	// INCLUDED_INPUTS_H
