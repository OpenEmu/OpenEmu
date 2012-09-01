#ifndef INCLUDED_INPUTTYPES_H
#define INCLUDED_INPUTTYPES_H

#include "Types.h"
#include "Input.h"

// All the input subclasses have been grouped together here as they are very simple classes

/*
 * Represents a switch input (such as a button or 8-way joystick switch) whose value can be either on (usually 1) or off (usually 0).
 */
class CSwitchInput : public CInput
{
private:
	// On and off values
	UINT16 m_offVal;
	UINT16 m_onVal;

public:
	CSwitchInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, 
		UINT16 offVal = 0x00, UINT16 onVal = 0x01);

	/*
	 * Polls (updates) the input, updating its value from the input source
	 */
	void Poll();

	/*
	 * Returns true if the input was pressed during last update (ie currently on but previously off)
	 */
	bool Pressed();

	/*
	 * Returns true if the input was released duing last update (ie currently off but previously on)
	 */
	bool Released();
};

/*
 * Represents an analog input (such as a pedal) whose value ranges from a minimum 'off' value (usually 0) to a maximum 'on' value (usually 0xFF).
 */
class CAnalogInput : public CInput
{
private:
	// Min and max values
	UINT16 m_minVal;
	UINT16 m_maxVal;

public:
	CAnalogInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, 
		UINT16 minVal = 0x00, UINT16 maxVal = 0xFF);

	/*
	 * Polls (updates) the input, updating its value from the input source
	 */
	void Poll();

	/*
	 * Returns true if the input is currently activated
	 */
	bool HasValue();

	/*
	 * Returns current value as a fraction 0.0 to 1.0 of the full range
	 */
	double ValueAsFraction();
};

/*
 * Represents an axis input (such as a joystick axis, lightgun axis or steering wheel) whose value ranges from a minimum value (usually 0) to a maximum 
 * value (usually 0xFF) with an off value (usually 0x80) somewhere in-between.
 * As well as having its own input source it takes two analog inputs that can represent the negative and positive ranges of the axis. 
 */
class CAxisInput : public CInput
{
private:
	// Analog inputs that represent the negative and positive range of the axis
	CAnalogInput *m_negInput;
	CAnalogInput *m_posInput;

	// Min, off and max values
	UINT16 m_minVal;
	UINT16 m_offVal;
	UINT16 m_maxVal;
	
public:
	CAxisInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, CAnalogInput *negInput, CAnalogInput *posInput,
		UINT16 minVal = 0x00, UINT16 offVal = 0x80, UINT16 maxVal = 0xFF);

	/*
	 * Polls (updates) the input, updating its value from the switch inputs and/or input source
	 */
	void Poll();

	/*
	 * Returns true if the input is currently activated (ie axis is not centered)
	 */
	bool HasValue();

	/*
	 * Returns current value as a fraction -1.0 to 1.0
	 */ 
	double ValueAsFraction();
};

/*
 * Represents a 4-gear shift input whose value ranges from 0 to 4, where neutral is 0 and 1-4 represents a gear selection.
 * It takes six switch inputs that combine together to control the gear shifting.  The first four inputs can set the gear 
 * directly, while the last two allow shifting up or down through the gears.
 */
class CGearShift4Input : public CInput
{
private:
	// Five switch inputs for gears 1-4 and N
	CSwitchInput *m_shift1Input;
	CSwitchInput *m_shift2Input;
	CSwitchInput *m_shift3Input;
	CSwitchInput *m_shift4Input;
	CSwitchInput *m_shiftNInput;

	// Two switch inputs for up/down gear
	CSwitchInput *m_shiftUpInput;
	CSwitchInput *m_shiftDownInput;

public:
	CGearShift4Input(const char *inputId, const char *inputLabel, unsigned inputGameFlags,
		CSwitchInput *shift1Input, CSwitchInput *shift2Input, CSwitchInput *shift3Input, CSwitchInput *shift4Input, CSwitchInput *shiftNInput, 
		CSwitchInput *shiftUpInput, CSwitchInput *shiftDownInput);

	/*
	 * Polls (updates) the input, updating its value from the switch inputs
	 */
	void Poll();
};

/*
 * Represents a trigger input, with both a trigger value and an offscreen value.  If required, it can simulate pointing offscreen and pulling
 * the trigger (in that order, which lightgun games require to reload properly) just when the offscreen input is activated.  This makes
 * reloading the gun easier when playing with just the mouse for example.
 */
class CTriggerInput : public CInput
{
private:
	// Real trigger and offscreen inputs
	CSwitchInput *m_triggerInput;
	CSwitchInput *m_offscreenInput;
	
	// Offscreen on and off values
	UINT16 m_offVal;
	UINT16 m_onVal;

	bool m_autoTrigger;
	int m_offscreenCount;

public:
	// Offscreen value
	UINT16 offscreenValue;

	CTriggerInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, CSwitchInput *triggerInput, CSwitchInput *offscreenInput,
		UINT16 offVal = 0x00, UINT16 onVal = 0x01);

	void ReadFromINIFile(CINIFile *ini, const char *section);

	void WriteToINIFile(CINIFile *ini, const char *section);

	/*
	 * Polls (updates) the input, updating its trigger value and offscreen value from the switch inputs
	 */
	void Poll();
};

#endif	// INCLUDED_INPUTTYPES_H