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
 * InputSystem.h
 *
 * Header file for CInputSystem. Defines the base class for an input system, 
 * the interface between the OS-independent and OS-dependent code.
 */
 
#ifndef INCLUDED_INPUTSYSTEM_H
#define INCLUDED_INPUTSYSTEM_H

#include <cstdio>
#include <string>
#include <vector>
using namespace std;

#include "MultiInputSource.h"

class CInput;
class CInputSource;
class CINIFile;

#define MAX_NAME_LENGTH 255

// Read flags for ReadMapping
#define READ_KEYBOARD 1
#define READ_MOUSE 2
#define READ_JOYSTICK 4
#define READ_MERGE_KEYBOARD 8
#define READ_MERGE_MOUSE 16
#define READ_MERGE_JOYSTICK 32
#define READ_ALL (READ_KEYBOARD|READ_MOUSE|READ_JOYSTICK)
#define READ_MERGE (READ_MERGE_KEYBOARD|READ_MERGE_MOUSE|READ_MERGE_JOYSTICK)

// Used to specify any keyboard, mouse or joystick when a number is required
#define ANY_KEYBOARD -1
#define ANY_MOUSE -1
#define ANY_JOYSTICK -1

// Default keyboard, mouse and joystick settings
#define DEFAULT_KEY_SENSITIVITY 25
#define DEFAULT_KEY_DECAYSPEED 50
#define DEFAULT_MSE_DEADZONE 0
#define DEFAULT_JOY_AXISMINVAL -32768
#define DEFAULT_JOY_AXISOFFVAL 0
#define DEFAULT_JOY_AXISMAXVAL 32767
#define DEFAULT_JOY_DEADZONE 3
#define DEFAULT_JOY_SATURATION 100

// Number of valid keys and mouse and joystick parts
#define NUM_VALID_KEYS (sizeof(s_validKeyNames) / sizeof(const char*))
#define NUM_MOUSE_PARTS ((int)MouseButtonX2 + 1)
#define NUM_MOUSE_AXES 3
#define NUM_MOUSE_BUTTONS 5
#define NUM_JOY_PARTS ((int)JoyButton31 + 1)
#define NUM_JOY_AXES 6
#define NUM_JOY_POVS 4
#define NUM_JOY_BUTTONS 32

// Axis numbers
#define AXIS_X 0
#define AXIS_Y 1
#define AXIS_Z 2
#define AXIS_RX 3
#define AXIS_RY 4
#define AXIS_RZ 5

// Axis directions
#define AXIS_FULL 0
#define AXIS_INVERTED 1
#define AXIS_POS 2
#define AXIS_NEG 3

// POV directions
#define POV_UP 0
#define POV_DOWN 1
#define POV_LEFT 2
#define POV_RIGHT 3

/*
 * Enumeration of all recognised mouse parts
 */ 
enum EMousePart
{
	MouseUnknown = -1,
	MouseXAxis = 0,
	MouseXAxisInv,
	MouseXAxisPos,
	MouseXAxisNeg,
	MouseYAxis,
	MouseYAxisInv,
	MouseYAxisPos,
	MouseYAxisNeg,
	MouseZAxis,
	MouseZAxisInv,
	MouseZAxisPos,
	MouseZAxisNeg,
	MouseButtonLeft,
	MouseButtonMiddle,
	MouseButtonRight,
	MouseButtonX1,
	MouseButtonX2,
};

/*
 * Enumeration of all supported joystick parts
 */
enum EJoyPart
{
	JoyUnknown = -1,
	JoyXAxis = 0,
	JoyXAxisInv,
	JoyXAxisPos,
	JoyXAxisNeg,
	JoyYAxis,
	JoyYAxisInv,
	JoyYAxisPos,
	JoyYAxisNeg,
	JoyZAxis,
	JoyZAxisInv,
	JoyZAxisPos,
	JoyZAxisNeg,
	JoyRXAxis,
	JoyRXAxisInv,
	JoyRXAxisPos,
	JoyRXAxisNeg,
	JoyRYAxis,
	JoyRYAxisInv,
	JoyRYAxisPos,
	JoyRYAxisNeg,
	JoyRZAxis,
	JoyRZAxisInv,
	JoyRZAxisPos,
	JoyRZAxisNeg,
	JoyPOV0Up,
	JoyPOV0Down,
	JoyPOV0Left,
	JoyPOV0Right,
	JoyPOV1Up,
	JoyPOV1Down,
	JoyPOV1Left,
	JoyPOV1Right,
	JoyPOV2Up,
	JoyPOV2Down,
	JoyPOV2Left,
	JoyPOV2Right,
	JoyPOV3Up,
	JoyPOV3Down,
	JoyPOV3Left,
	JoyPOV3Right,
	JoyButton0,
	JoyButton1,
	JoyButton2,
	JoyButton3,
	JoyButton4,
	JoyButton5,
	JoyButton6,
	JoyButton7,
	JoyButton8,
	JoyButton9,
	JoyButton10,
	JoyButton11,
	JoyButton12,
	JoyButton13,
	JoyButton14,
	JoyButton15,
	JoyButton16,
	JoyButton17,
	JoyButton18,
	JoyButton19,
	JoyButton20,
	JoyButton21,
	JoyButton22,
	JoyButton23,
	JoyButton24,
	JoyButton25,
	JoyButton26,
	JoyButton27,
	JoyButton28,
	JoyButton29,
	JoyButton30,
	JoyButton31
};

struct MousePartsStruct
{
	const char* id;
	EMousePart msePart;
};

struct JoyPartsStruct
{
	const char* id;
	EJoyPart joyPart;
};

/*
 * Struct that holds settings for a keyboard
 */
struct KeySettings
{
	int kbdNum;             // Keyboard number (or ANY_KEYBOARD for settings that apply to all keyboards)
	unsigned sensitivity;   // Key sensitivity for analog controls as percentage 1-100, where 100 is extremely responsive 
    unsigned decaySpeed;    // Decay speed as percentage 1-200 of on speed
	
	/*
	 * Creates a KeySettings with default settings
	 */
	KeySettings()
	{
		kbdNum = ANY_KEYBOARD;
		sensitivity = DEFAULT_KEY_SENSITIVITY;
		decaySpeed = DEFAULT_KEY_DECAYSPEED;
	}
};

/*
 * Struct that holds settings for a mouse
 */
struct MouseSettings
{
	int mseNum;                         // Mouse number (or ANY_MOUSE for settings that apply to all mice)
	unsigned deadZones[NUM_MOUSE_AXES]; // Axis dead zone as a percentage 0-99 of display width (X)/height (Y) or axis range (Z)

	/*
	 * Creates a MouseSettings with default settings
	 */
	MouseSettings()
	{
		mseNum = ANY_MOUSE;
		deadZones[AXIS_X] = DEFAULT_MSE_DEADZONE;
		deadZones[AXIS_Y] = DEFAULT_MSE_DEADZONE;
		deadZones[AXIS_Z] = 0;
	}
};

/*
 * Struct that holds settings for a joystick
 */
struct JoySettings
{
	int joyNum;                         // Joystick number (or ANY_JOYSTICK for settings that apply to all joysticks)
	int axisMinVals[NUM_JOY_AXES];		// Axis min raw value (default -32768)
	int axisOffVals[NUM_JOY_AXES];		// Axis center/off value (default 0)
	int axisMaxVals[NUM_JOY_AXES];		// Axis max raw value (default 32767)
	unsigned deadZones[NUM_JOY_AXES];   // Axis dead zone as a percentage 0-99 of axis positive/negative ranges
	unsigned saturations[NUM_JOY_AXES]; // Axis saturation as a percentage 1-200 of axis positive/negative ranges

	/*
	 * Creates a JoySettings with default settings
	 */
	JoySettings()
	{
		joyNum = ANY_JOYSTICK;
		for (int axisNum = 0; axisNum < NUM_JOY_AXES; axisNum++)
		{
			axisMinVals[axisNum] = DEFAULT_JOY_AXISMINVAL;
			axisOffVals[axisNum] = DEFAULT_JOY_AXISOFFVAL;
			axisMaxVals[axisNum] = DEFAULT_JOY_AXISMAXVAL;
			deadZones[axisNum] = DEFAULT_JOY_DEADZONE;
			saturations[axisNum] = DEFAULT_JOY_SATURATION;
		}
	}
};

struct KeyDetails
{
	char name[MAX_NAME_LENGTH + 1]; // Keyboard name (if available)
};

struct MouseDetails
{
	char name[MAX_NAME_LENGTH + 1]; // Mouse name (if available)
	bool isAbsolute;                // True if uses absolute positions (ie lightgun)
};

struct JoyDetails
{
	char name[MAX_NAME_LENGTH + 1];                   // Joystick name (if available)
	int numAxes;                                      // Total number of axes on joystick
	int numPOVs;                                      // Total number of POV hat controllers on joystick
	int numButtons;                                   // Total number of buttons on joystick
	bool hasFFeedback;	                              // True if joystick supports force feedback
	bool hasAxis[NUM_JOY_AXES];                       // Flags to indicate which axes available on joystick
	char axisName[NUM_JOY_AXES][MAX_NAME_LENGTH + 1]; // Axis names (if available)
	bool axisHasFF[NUM_JOY_AXES];                     // Flags to indicate which axes are force feedback enabled
};

/*
 * Abstract base class that represents an input system.  An input system encapsulates all the O/S dependent code to read keyboards, 
 * mice and joysticks.
 */
class CInputSystem
{
private:
	// Array of valid key names
	static const char *s_validKeyNames[];

	// Lookup table for translating mouse mapping strings to their respective mouse parts
	static MousePartsStruct s_mseParts[];

	// Lookup table for translating joystick mapping strings to their respective joystick parts
	static JoyPartsStruct s_joyParts[];

	// Names of axes
	static const char *s_axisNames[];

	// Number of keyboards, mice and joysticks
	int m_numKbds;
	int m_numMice;
	int m_numJoys;

	// Cached input sources
	CInputSource **m_anyKeySources;
	CInputSource **m_anyMseSources;
	CInputSource **m_anyJoySources;
	CInputSource ***m_keySources; 
	CInputSource ***m_mseSources;
	CInputSource ***m_joySources;
	
	// Default key, mouse and joystick settings
	KeySettings m_defKeySettings;
	MouseSettings m_defMseSettings;
	JoySettings m_defJoySettings;

	// Current key, mouse and joystick settings for attached keyboards, mice and joysticks
	vector<KeySettings*> m_keySettings;
	vector<MouseSettings*> m_mseSettings;
	vector<JoySettings*> m_joySettings;

	// Empty input source
	CMultiInputSource *m_emptySource;

	//
	// Helper methods
	//

	/*
	 * Creates source cache.
	 */
	void CreateSourceCache();

	/*
	 * Clears cache of all sources and optionally deletes cache itself.
	 */
	void ClearSourceCache(bool deleteCache = false);

	/* 
	 * Releases a source from the cache.
	 */ 
	void ReleaseSource(CInputSource *&source);
	
	/*
	 * Returns a key source for the given keyboard number (or all keyboards if ANY_KEYBOARD supplied) and key index.
	 * Will check the source cache first and if not found will create the source with CreateAnyKeySource or CreateKeySource.
	 */
	CInputSource *GetKeySource(int kbdNum, int keyIndex);

	/*
	 * Returns a mouse source for the given mouse number (or all mice if ANY_MOUSE supplied) and mouse index.
	 * Will check the source cache first and if not found will create the source with CreateAnyMouseSource or CreateMouseSource.
	 */
	CInputSource *GetMouseSource(int mseNum, EMousePart msePart);

	/*
	 * Returns a joystick source for the given joystick number (or all joysticks if ANY_JOYSTICK supplied) and joystick index.
	 * Will check the source cache first and if not found will create the source with CreateAnyJoySource or CreateJoySource.
	 */
	CInputSource *GetJoySource(int joyNum, EJoyPart joyPart);

	void CheckAllSources(unsigned readFlags, bool fullAxisOnly, bool &mseCentered, vector<CInputSource*> &sources, string &mapping, vector<CInputSource*> &badSources);

	/*
	 * Finds any currently activated key sources for the given keyboard number (or all keyboards if ANY_KEYBOARD supplied)
	 * and adds them to the sources vector, aswell as constructing the corresponding mapping(s) in the given string.
	 * If fullAxisOnly is true, then only sources that represent a full axis range (eg MouseXAxis) are considered.
	 */
	void CheckKeySources(int kbdNum, bool fullAxisOnly, vector<CInputSource*> &sources, string &mapping, vector<CInputSource*> &badSources);

	/*
	 * Finds any currently activated mouse sources for the given mouse number (or all mice if ANY_MOUSE supplied)
	 * and adds them to the sources vector, aswell as constructing the corresponding mapping(s) in the given string.
	 * If fullAxisOnly is true, then only sources that represent a full axis range (eg MouseXAxis) are considered.
	 */
	void CheckMouseSources(int mseNum, bool fullAxisOnly, bool mseCentered, vector<CInputSource*> &sources, string &mapping, vector<CInputSource*> &badSources);

	/*
	 * Finds any currently activated joystick sources for the given joystick number (or all joysticks if ANY_JOYSTICK supplied)
	 * and adds them to the sources vector, aswell as constructing the corresponding mapping(s) in the given string.
	 * If fullAxisOnly is true, then only sources that represent a full axis range (eg MouseXAxis) are considered.
	 */
	void CheckJoySources(int joyNum, bool fullAxisOnly, vector<CInputSource*> &sources, string &mapping, vector<CInputSource*> &badSources);

	bool ParseInt(string str, int &num);

	string IntToString(int num);

	bool EqualsIgnoreCase(string str1, const char *str2);

	bool StartsWithIgnoreCase(string str1, const char *str2);

	/*
	 * Returns true if the given string represents a valid key name.
	 */
	bool IsValidKeyName(string str);

	/*
	 * Returns the EMousePart with the given mapping name or MouseUnknown if not found.
	 */
	EMousePart LookupMousePart(string str);

	/*
	 * Returns the mapping name for the given EMousePart.
	 */
	const char *LookupName(EMousePart msePart);

	/*
	 * Returns the EJoyPart with the given mapping name or JoyUnknown if not found.
	 */
	EJoyPart LookupJoyPart(string str);

	/*
	 * Returns the mapping name for the given EJoyPart.
	 */
	const char *LookupName(EJoyPart joyPart);

	size_t ParseDevMapping(string str, const char *devType, int &devNum);

	/*
	 * Parses the given mapping string, possibly representing more than one mapping, and returns an input source for it or NULL if the 
	 * mapping is invalid.
	 * If fullAxisOnly is true, then only mappings that represent a full axis range (eg MouseXAxis) are parsed.
	 */
	CInputSource* ParseMultiSource(string str, bool fullAxisOnly, bool isOr);

	/*
	 * Parses the given single mapping string and returns an input source for it, or NULL if non exists.
	 */
	CInputSource* ParseSingleSource(string str);

	/*
	 * Prints the given key settings to stdout.
	 */
	void PrintKeySettings(int kbdNum, KeySettings *settings);

	/*
	 * Reads key settings from an INI file for the given keyboard number, or common settings if ANY_KEYBOARD specified.
	 * Returns NULL if no relevant settings were found in the INI file.
	 */
	KeySettings *ReadKeySettings(CINIFile *ini, const char *section, int kbdNum);

	/*
	 * Writes the given key settings to an INI file, only writing out settings that are different to their defaults.
	 */
	void WriteKeySettings(CINIFile *ini, const char *section, KeySettings *settings);

	/*
	 * Prints the given mouse settings to stdout.
	 */
	void PrintMouseSettings(int mseNum, MouseSettings *settings);

	/*
	 * Reads mouse settings from an INI file for the given mouse number, or common settings if ANY_MOUSE specified.
	 * Returns NULL if no relevant settings were found in the INI file.
	 */
	MouseSettings *ReadMouseSettings(CINIFile *ini, const char *section, int mseNum);

	/*
	 * Writes the given mouse settings to an INI file, only writing out settings that are different to their defaults.
	 */
	void WriteMouseSettings(CINIFile *ini, const char *section, MouseSettings *settings);
	
	/*
	 * Prints the given joystick settings to stdout.
	 */
	void PrintJoySettings(int joyNum, JoySettings *settings);

	/*
	 * Reads joystick settings from an INI file for the given joystick number, or common settings if ANY_JOYSTICK specified.
	 * Returns NULL if no relevant settings were found in the INI file.
	 */
	JoySettings *ReadJoySettings(CINIFile *ini, const char *section, int joyNum);

	/*
	 * Writes the given joystick settings to an INI file, only writing out settings that are different to their defaults.
	 */
	void WriteJoySettings(CINIFile *ini, const char *section, JoySettings *settings);

protected:
	// Current display geometry
	unsigned m_dispX;
	unsigned m_dispY;
	unsigned m_dispW;
	unsigned m_dispH;	

	// Flag to indicate if system has grabbed mouse
	bool m_grabMouse;

	/*
	 * Constructs an input system with the given name.
	 */
	CInputSystem(const char *systemName);

	/*
	 * Returns true if the given EMousePart is an axis.
	 */
	bool IsAxis(EMousePart msePart);

	/*
	 * Returns true if the given EMousePart represents a full axis, eg MouseXAxis or MouseXAxisInv.
	 */
	bool IsFullAxis(EMousePart msePart);

	/*
	 * Returns true if the EMousePart represents an axis and sets axisPart and axisDir as follows:
	 * axisNum will be the axis number (AXIS_X, AXIS_Y or AXIS_Z)
	 * axisDir will be AXIS_FULL, AXIS_INVERTED, AXIS_POS or AXIS_POS depending on whether the axis has the full range, has
	 * the full range but inverted, is negative only, or is positive only.
	 */
	bool GetAxisDetails(EMousePart msePart, int &axisNum, int &axisDir);

	/*
	 * Returns true if the given EMousePart represets a button, eg MouseButtonLeft.
	 */
	bool IsButton(EMousePart msePart);

	/*
	 * Returns the button number (indexed from 0) of the given EMousePart if it is a button, or -1 otherwise.
	 */
	int GetButtonNumber(EMousePart msePart);

	/* 
	 * Returns the EMousePart that represents the axis component for the given axis number (AXIS_X, AXIS_Y or AXIS_Z) and direction 
	 * (AXIS_FULL, AXIS_INVERTED, AXIS_POS or AXIS_POS), or MouseUnknown otherwise.
	 */
	EMousePart GetMouseAxis(int axisNum, int axisDir);

	/*
	 * Returns the EMousePart that represents the mouse button with the given number (0-4), or MouseUnknown otherwise.
	 */
	EMousePart GetMouseButton(int butNum);

	/*
	 * Returns true if the given EJoyPart is an axis.
	 */
	bool IsAxis(EJoyPart joyPart);

	/*
	 * Returns true if the given EJoyPart represents a full axis, eg JoyXAxis.
	 */
	bool IsFullAxis(EJoyPart joyPart);

	/*
	 * Returns true if joystick part represents an axis and sets axisPart and axisDir as follows:
	 * axisNum will be the axis number (AXIS_X, AXIS_Y, AXIS_Z, AXIS_RX, AXIS_RY or AXIS_RZ)
	 * axisDir is AXIS_FULL, AXIS_INVERTED, AXIS_POS or AXIS_POS depending on whether the axis has the full range, has
	 * the full range but inverted, is negative only, or is positive only.
	 */
	bool GetAxisDetails(EJoyPart joyPart, int &axisNum, int &axisDir);

	/*
	 * Returns true if the given EJoyPart represents a POV hat direction, eg JoyPOV0Left.
	 */ 
	bool IsPOV(EJoyPart joyPart);

	/*
	 * Returns true if the EJoyPart represents a POV hat direction and sets povNum and povDir as follows:
	 * povNum will be the POV hat number 0-4,
	 * povDir will be POV_UP, POV_DOWN, POV_LEFT or POV_RIGHT.
	 */
	bool GetPOVDetails(EJoyPart joyPart, int &povNum, int &povDir);

	/*
	 * Returns true if the given EJoyPart is a button
	 */
	bool IsButton(EJoyPart joyPart);

	/*
	 * Returns the button number (indexed from 0) of the given EJoyPart if it is a button, or -1 otherwise.
	 */
	int GetButtonNumber(EJoyPart joyPart);

	/* 
	 * Returns the EJoyPart that represents the axis component for the given axis number (AXIS_X, AXIS_Y, AXIS_Z, AXIS_RX, AXIS_RY or AXIS_RZ) and
	 * direction (AXIS_FULL, AXIS_INVERTED, AXIS_POS or AXIS_POS), or JoyUnknown otherwise.
	 */
	EJoyPart GetJoyAxis(int axisNum, int axisDir);

	/*
	 * Returns the EJoyPart that represents the POV hot direction for the given POV number (0-4) and direction (POV_UP, POV_DOWN, 
	 * POV_LEFT or POV_RIGHT), JoyUnknown otherwise.
	 */
	EJoyPart GetJoyPOV(int povNum, int povDir);

	/*
	 * Returns the EJoyPart that represents the joystick button with the given number (0-31), or JoyUnknown otherwise.
	 */
	EJoyPart GetJoyButton(int butNum);

	//
	// Abstract methods subclass must implement (ie system-specific code)
	//

	virtual bool InitializeSystem() = 0;

	/*
	 * Returns the system-specific key index that represents the given key name.
	 */
	virtual int GetKeyIndex(const char *keyName) = 0;

	/*
	 * Returns the key name of the given system-specific integer key index.
	 */
	virtual const char *GetKeyName(int keyIndex) = 0;

	/* 
	 * Returns true if for the given keyboard the key with the system-specific key index is currently pressed.
	 */
	virtual bool IsKeyPressed(int kbdNum, int keyIndex) = 0;

	/*
	 * Returns the current axis value for the given mouse and axis number (AXIS_X, AXIS_Y or AXIS_Z).
	 */
	virtual int GetMouseAxisValue(int mseNum, int axisNum) = 0;

	/*
	 * Returns the current direction (-1, 0 or 1) the Z-axis (wheel) is moving in for the given mouse.
	 */
	virtual int GetMouseWheelDir(int mseNum) = 0;

	/*
	 * Returns true if for the given mouse the button with the given number is currently pressed.
	 */
	virtual bool IsMouseButPressed(int mseNum, int butNum) = 0;

	/*
	 * Returns the current axis value for the given joystick and axis number (AXIS_X, AXIS_Y, AXIS_Z, AXIS_RX, AXIS_RY or AXIS_RZ).
	 */
	virtual int GetJoyAxisValue(int joyNum, int axisNum) = 0;

	/*
	 * Returns true if for the given joystick the POV-hat controller with the given number is pointing in a particular direction (POV_UP, POV_DOWN,
	 * POV_LEFT or POV_RIGHT)
	 */
	virtual bool IsJoyPOVInDir(int joyNum, int povNum, int povDir) = 0;

	/*
	 * Returns true if for the given joystick the button with the given number is currently pressed.
	 */
	virtual bool IsJoyButPressed(int joyNum, int butNum) = 0;

	/*
	 * Processes the given force feedback command for the given joystick and axis number.
	 */
	virtual bool ProcessForceFeedbackCmd(int joyNum, int axisNum, ForceFeedbackCmd ffCmd) = 0;

	/*
	 * Waits for the given time in milliseconds
	 */
	virtual void Wait(int ms) = 0;

	//
	// Virtual methods subclass can override if required
	//

	/*
	 * Returns true if the mouse is currently centered in the display during configuration.
	 */
	virtual bool ConfigMouseCentered();

	/*
	 * Creates an input source combining all keyboards for the given key index.
	 */
	virtual CInputSource *CreateAnyKeySource(int keyIndex);

	/*
	 * Creates an input source combining all mice for the given EMousePart.
	 */
	virtual CInputSource *CreateAnyMouseSource(EMousePart msePart);

	/*
	 * Creates an input source combining all joysticks for the given EJoyPart.
	 */
	virtual CInputSource *CreateAnyJoySource(EJoyPart joyPart);
	
	/*
	 * Creates an input source for the given keyboard number and key index.
	 */
	virtual CInputSource *CreateKeySource(int kbdNum, int keyIndex);

	/*
	 * Creates an input source for the given mouse number and EMousePart.
	 */
	virtual CInputSource *CreateMouseSource(int mseNum, EMousePart msePart);

	/*
	 * Creates an input source for the given joystick number and EJoyPart.
	 */
	virtual CInputSource *CreateJoySource(int joyNum, EJoyPart joyPart);

public:
#ifdef DEBUG
	static unsigned totalSrcsAcquired;
	static unsigned totalSrcsReleased;
#endif
	static const char *GetDefaultAxisName(int axisNum);

	// Name of this input system
	const char *name;

	virtual ~CInputSystem();

	/*
	 * Initializes the input system.  Must be called before any other methods are used.
	 * Returns false if unable to initialize the system.
	 */
	bool Initialize();

	/*
	 * Sets the current display geometry so that mouse movements can be scaled properly.
	 */ 
	void SetDisplayGeom(unsigned dispX, unsigned dispY, unsigned dispW, unsigned dispH);

	/*
	 * Returns the number of attached keyboards (or 0 if the system cannot handle keyboards at all or ANY_KEYBOARD if the system cannot 
	 * handle multiple keyboards).
	 */
	virtual int GetNumKeyboards() = 0;
	
	/*
	 * Returns the number of attached mice (or 0 if the system cannot handle mice at all or ANY_MOUSE if the system cannot handle 
	 * multiple mice).
	 */
	virtual int GetNumMice() = 0;
	
	/*
	 * Returns number of attached joysticks (or 0 if the system cannot handle joysticks at all or ANY_JOYSTICK if the system cannot 
	 * handle multiple joysticks).
	 */
	virtual int GetNumJoysticks() = 0;

	/*
	 * Returns details about the keyboard with the given number, or NULL if it does not exist.
	 */
	virtual const KeyDetails *GetKeyDetails(int kbdNum) = 0;

	/*
	 * Returns details about the mouse with the given number, or NULL if it does not exist.
	 */
	virtual const MouseDetails *GetMouseDetails(int mseNum) = 0;

	/*
	 * Returns details about the joystick with the given number, or NULL if it does not exist.
	 */
	virtual const JoyDetails *GetJoyDetails(int joyNum) = 0;

	/*
	 * Clears all keyboard, mouse and joystick settings.
     */
	void ClearSettings();

	/*
	 * Prints current keyboard, mouse and joystick settings to stdout.
	 */
	void PrintSettings();

	/*
	 * Reads all keyboard, mouse and joystick settings (and any additional system-specific additional settings) from the given INI file.
	 */
	virtual void ReadFromINIFile(CINIFile *ini, const char *section);

	/*
	 * Writes all keyboard, mouse and joystick settings (and any additional system-specific settings) to the given INI file.
	 */
	virtual void WriteToINIFile(CINIFile *ini, const char *section);

	/*
	 * Returns the current key settings for given keyboard number, or common settings if ANY_KEYBOARD specified.
	 * If no settings are found and useDefault is false, NULL is returned.  If useDefault is true then default settings are returned.
	 */
	KeySettings *GetKeySettings(int kbdNum, bool useDefault);

	/*
	 * Returns the current mouse settings for given mouse number, or common settings if ANY_MOUSE specified.
	 * If no settings are found and useDefault is false, NULL is returned.  If useDefault is true then default settings are returned.
	 */
	MouseSettings *GetMouseSettings(int mseNum, bool useDefault);

	/*
	 * Returns the current joystick settings for given joystick number, or common settings if ANY_JOYSTICK specified.
	 * If no settings are found and useDefault is false, NULL is returned.  If useDefault is true then default settings are returned.
	 */
	JoySettings *GetJoySettings(int joyNum, bool useDefault);
	
	/*
	 * Returns the input source for the given mapping, or NULL if mapping is not valid.
	 */
	CInputSource* ParseSource(const char *mapping, bool fullAxisOnly = false);

	/*
	 * Waits for any input from the user and once received copies a mapping configuration representing the input (eg KEY_A or JOY1_AXIS_POS)
	 * into the given buffer.
	 * Returns true if input was successfully received or false if the user activated the given escape mapping or closed the window.
	 * readFlags specifies which types of inputs (keyboards, mice, joysticks) are to be read and whether to merge the inputs to a common
	 * mapping, eg return MOUSE_XAXIS rather than MOUSE3_XAXIS.
	 * If fullAxisOnly is true, then only mappings representing a full axis are returned, eg JOY1_XAXIS is allowed but not JOY1_XAXIS_POS.
	 */	
	bool ReadMapping(char *buffer, unsigned bufSize, bool fullAxisOnly = false, unsigned readFlags = READ_ALL, const char *escapeMapping = "KEY_ESCAPE");

	/*
	 * Updates the current state of the input system (called by CInputs.Poll).
	 */
	virtual bool Poll() = 0;

	virtual void GrabMouse();

	virtual void UngrabMouse();

	/*
	 * Sets the mouse visibility (some systems may choose to ignore this).
	 */
	virtual void SetMouseVisibility(bool visible) = 0;

	virtual bool SendForceFeedbackCmd(int joyNum, int axisNum, ForceFeedbackCmd ffCmd);

	bool DetectJoystickAxis(unsigned joyNum, unsigned &axisNum, const char *escapeMapping = "KEY_ESCAPE", const char *confirmMapping = "KEY_RETURN");

	bool CalibrateJoystickAxis(unsigned joyNum, unsigned axisNum, const char *escapeMapping = "KEY_ESCAPE", const char *confirmMapping = "KEY_RETURN");

	void PrintDevices();

	//
	// Nested Classes
	//

	/*
	 * Input source for a key on a keyboard.
	 */
	class CKeyInputSource : public CInputSource
	{
	private:
		CInputSystem *m_system; // Parent input system
		int m_kbdNum;           // Keyboard number
		int m_keyIndex;         // Key index
		int m_incr;             // Key increment for analog values 
		int m_decr;             // Key decrement for analog values
		int m_val;              // Current analog key value
		int m_maxVal;           // Maximum analog key value

	public:
		CKeyInputSource(CInputSystem *system, int kbdNum, int keyIndex, unsigned sensitivity, unsigned decaySpeed);

		bool GetValueAsSwitch(bool &val);

		bool GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal);
	};

	/*
	 * Input source for the X- or Y-axis of a mouse.
	 */ 
	class CMseAxisInputSource : public CInputSource
	{
	private:
		CInputSystem *m_system; // Parent input system
		int m_mseNum;           // Mouse number
		int m_axisNum;			// Axis number (AXIS_X, AXIS_Y or AXIS_Z)
		int m_axisDir;          // Axis direction (AXIS_FULL, AXIS_INVERTED, AXIS_POSITIVE or AXIS_NEGATIVE)
		int m_deadPixels;       // Size in pixels of dead zone in centre of axis

		/*
		 * Scales the mouse axis value to the given range.
		 */
		int ScaleAxisValue(int minVal, int offVal, int maxVal);

	public:
		CMseAxisInputSource(CInputSystem *system, int mseNum, int axisNum, int axisDir, unsigned deadZone);

		bool GetValueAsSwitch(bool &val);

		bool GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal);
	};

	/*
	 * Input source for the button of a mouse.
	 */
	class CMseButInputSource : public CInputSource
	{
	private:
		CInputSystem *m_system; // Parent input system
		int m_mseNum;           // Mouse number	
		int m_butNum;           // Button number

	public:
		CMseButInputSource(CInputSystem *system, int mseNum, int butNum);

		bool GetValueAsSwitch(bool &val);

		bool GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal);
	};

	/*
	 * Input source for the axis of a joystick.
	 */ 
	class CJoyAxisInputSource : public CInputSource
	{
	private:
		CInputSystem *m_system; // Parent input system
		int m_joyNum;           // Joystick number
		int m_axisNum;          // Axis number (AXIS_X, AXIS_Y, AXIS_Z, AXIS_RX, AXIS_RY or AXIS_RZ)
		int m_axisDir;          // Axis direction (AXIS_FULL, AXIS_INVERTED, AXIS_POSITIVE or AXIS_NEGATIVE)
		int m_axisMinVal;	    // Axis min raw value (default -32768)
		int m_axisOffVal;		// Axis center/off raw value (default 0)
		int m_axisMaxVal;		// Axis max raw value (default 32767)
		bool m_axisInverted;	// True if axis max raw value less than axis min raw value
		int m_posDZone;         // Dead zone for positive range (0-99%)
		int m_negDZone;         // Dead zone for negative range (0-99%)
		int m_posSat;           // Saturation for positive range (1-100%)
		int m_negSat;           // Saturation for negative range (1-100%)

		/*
		 * Scales the joystick axis value to the given range.
		 */
		int ScaleAxisValue(int minVal, int offVal, int maxVal);

	public:
		CJoyAxisInputSource(CInputSystem *system, int joyNum, int axisNum, int axisDir, int axisMinVal, int axisOffVal, int axisMaxVal,
			unsigned deadZone, unsigned saturation);

		bool GetValueAsSwitch(bool &val);

		bool GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal);

		bool SendForceFeedbackCmd(ForceFeedbackCmd ffCmd);
	};

	/*
	 * Input source for a particular direction of a POV hat controller of a joystick.
	 */
	class CJoyPOVInputSource : public CInputSource
	{
	private:
		CInputSystem *m_system; // Parent input system
		int m_joyNum;           // Joystick number
		int m_povNum;           // POV hat number
		int m_povDir;           // POV hat direction (POV_UP, POV_LEFT, POV_RIGHT, POV_DOWN)

	public:
		CJoyPOVInputSource(CInputSystem *system, int joyNum, int povNum, int povDir);

		bool GetValueAsSwitch(bool &val);

		bool GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal);
	};

	/*
	 * Input source for the button of a joystick.
	 */
	class CJoyButInputSource : public CInputSource
	{
	private:
		CInputSystem *m_system; // Parent input system
		int m_joyNum;           // Joystick number
		int m_butNum;           // Button number

	public:
		CJoyButInputSource(CInputSystem *system, int joyNum, int butNum);

		bool GetValueAsSwitch(bool &val);

		bool GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal);
	};
};

#endif	// INCLUDED_INPUTSYSTEM_H
