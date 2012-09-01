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
 * Input.h
 *
 * Header file for base input class, CInput, defining an input source. Also
 * defines GAME_INPUT_UI.
 */
 
#ifndef INCLUDED_INPUT_H
#define INCLUDED_INPUT_H

#include "Types.h"

class CInputSource;
class CInputSystem;
class CINIFile;

// Special game input flag for UI controls
#define GAME_INPUT_UI 0

// Flags for inputs
#define INPUT_FLAGS_SWITCH  0x0001
#define INPUT_FLAGS_ANALOG  0x0002
#define INPUT_FLAGS_AXIS    0x0004
#define INPUT_FLAGS_VIRTUAL 0x0008

#define MAX_MAPPING_LENGTH 255

#define NUM_FF_EFFECTS 3

enum EForceFeedback
{
	FFStop = -1,
	FFConstantForce = 0,
	FFSelfCenter = 1,
	FFFriction = 2,
	FFVibrate = 3
};

struct ForceFeedbackCmd
{
	EForceFeedback id;   // Type of effect to apply
	float force;         // Intensity (and, for FFConstantForce, direction) of effect with 0.0f = off and +/1.0f = maximum
};

/*
 * Base class for any type of Model3 input control.
 */
class CInput
{
private:
	// Current mapping(s) for input, eg JOY1_XAXIS_POS
	char m_mapping[MAX_MAPPING_LENGTH + 1];
	
	// Default mapping for input
	const char *m_defaultMapping;
	
	// Assigned input system
	CInputSystem *m_system;

	/*
	 * Creates an input source using the current input system and assigns it to this input.
	 */
	void CreateSource();

protected:
	// Current input source
	CInputSource *m_source;
	
	/*
	 * Constructs an input with the given identifier, label, flags, game flags, default mapping and initial value.
	 */ 
	CInput(const char *inputId, const char *inputLabel, unsigned inputFlags, unsigned inputGameFlags, 
		const char *defaultMapping = "NONE", UINT16 initValue = 0);

public:
	virtual ~CInput();

	// Input identifier
	const char *id;

	// Input label
	const char *label;

	// Input flags
	const unsigned flags;

	// Input game flags
	const unsigned gameFlags;

	// Current input value
	UINT16 value;

	// Previous input value
	UINT16 prevValue;
	
	/*
	 * Initializes this input with the given input system.  Must be called before any other methods are used.
	 */
	void Initialize(CInputSystem *system);

	/*
	 * Returns the name of the group of controls that this input belongs to.
	 */
	const char* GetInputGroup();

	/*
	 * Returns the current mapping(s) assigned to this input.
	 */
	const char* GetMapping();

	/*
	 * Clears the current mapping(s) assigned to this input.
	 */
	void ClearMapping();

	/*
	 * Sets the current mapping(s) assigned to this input. 
	 * Multiple mapping assignments are comma-separated, eg KEY_RIGHT,JOY1_XAXIS_POS.
	 * Simultaneous assignments are joined with a plus, eg KEY_ALT+KEY_P.
	 */
	void SetMapping(const char *mapping);

	/*
	 * Appends a mapping to this input to create a multiple mapping assignment.
	 */
	void AppendMapping(const char *mapping);

	/*
	 * Resets the mapping(s) assigned to this input to the input's default.
	 */
	void ResetToDefaultMapping();
	
	/*
	 * Reads the input's mapping(s) from the given INI file, as well as any other settings.
	 */
	virtual void ReadFromINIFile(CINIFile *ini, const char *section);

	/*
	 * Writes the current input mapping(s) to the given INI file, as well as any other settings.
	 */
	virtual void WriteToINIFile(CINIFile *ini, const char *section);

	void InputSystemChanged();

	/*
	 * Returns true if the input is a UI input.
	 */
	bool IsUIInput();

	/*
	 * Returns true if the input is configurable and can be set by the user.
	 */
	bool IsConfigurable();

	/*
	 * Returns true if the input is a virtual input, ie one which generates its value based on other inputs and so has no mapping.
	 */
	bool IsVirtual();

	/*
	 * Configures the current mapping(s) assigned to this input by asking the user for input.
	 * If append is true, then the user's selected mapping is appended.  Otherwise, it overwrites the existing mapping(s).
	 * escapeMapping holds the input mapping used to exit the configuration without applying the changes.
	 */
	bool Configure(bool append, const char *escapeMapping = "KEY_ESCAPE");

	/*
	 * Polls (updates) this input, updating its value from the input source
	 */
	virtual void Poll() = 0;

	/*
	 * Returns true if the value of this input changed during the last poll.
	 */
	bool Changed();

	/*
	 * Sends a force feedback command to the input source of this input.
	 */
	bool SendForceFeedbackCmd(ForceFeedbackCmd ffCmd);
};

//
// Inlined methods
//

inline bool CInput::IsUIInput()
{
	return gameFlags == GAME_INPUT_UI;
}

inline bool CInput::IsConfigurable()
{
	// All inputs except UI and virtual ones can be configured by the user
	return (gameFlags != GAME_INPUT_UI) && !(flags & INPUT_FLAGS_VIRTUAL);
}

inline bool CInput::IsVirtual()
{
	return !!(flags & INPUT_FLAGS_VIRTUAL);
}

#endif	// INCLUDED_INPUT_H
