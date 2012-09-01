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
 * InputSystem.cpp
 *
 * Implementation of CInputSystem, the base input system class.
 */

// TODO
// - switch to using more C++ strings
// - simplify handling of configuration settings because it is a mess
// - think more about where config, calibrate and debug methods should go - OSD classes, CInputs or here?
// - open up API to allow direct access to keyboard, mouse and joystick values
// - add GetKey method that is easier to use than reading keyboard with ReadMapping

#include "Supermodel.h"

#include <cmath>
#include <string>
#include <algorithm>
#include <vector>
#include <sstream>
using namespace std;

#ifdef DEBUG
unsigned CInputSystem::totalSrcsAcquired = 0;
unsigned CInputSystem::totalSrcsReleased = 0;
#endif

const char *CInputSystem::s_validKeyNames[] = 
{
	// General keys
	"BACKSPACE",
	"TAB",
	"CLEAR",
	"RETURN",
	"PAUSE",
	"ESCAPE",
	"SPACE",
	"EXCLAIM",
	"DBLQUOTE",
	"HASH",
	"DOLLAR",
	"AMPERSAND",
	"QUOTE",
	"LEFTPAREN",
	"RIGHTPAREN",
	"ASTERISK",
	"PLUS",
	"COMMA",
	"MINUS",
	"PERIOD",
	"SLASH",
	"0",
	"1",
	"2",
	"3",
	"4",
	"5",
	"6",
	"7",
	"8",
	"9",
	"COLON",
	"SEMICOLON",
	"LESS",
	"EQUALS",
	"GREATER",
	"QUESTION",
	"AT",
	"LEFTBRACKET",
	"BACKSLASH",
	"RIGHTBRACKET",
	"CARET",
	"UNDERSCORE",
	"BACKQUOTE",
	"A",
	"B",
	"C",
	"D",
	"E",
	"F",
	"G",
	"H",
	"I",
	"J",
	"K",
	"L",
	"M",
	"N",
	"O",
	"P",
	"Q",
	"R",
	"S",
	"T",
	"U",
	"V",
	"W",
	"X",
	"Y",
	"Z",
	"DEL",
	
	// Keypad
	"KEYPAD0",
	"KEYPAD1",
	"KEYPAD2",
	"KEYPAD3",
	"KEYPAD4",
	"KEYPAD5",
	"KEYPAD6",
	"KEYPAD7",
	"KEYPAD8",
	"KEYPAD9",
	"KEYPADPERIOD",
	"KEYPADDIVIDE",
	"KEYPADMULTIPLY",
	"KEYPADMINUS",
	"KEYPADPLUS",
	"KEYPADENTER",
	"KEYPADEQUALS",
	
	// Arrows + Home/End Pad
	"UP",
	"DOWN",
	"RIGHT",
	"LEFT",
	"INSERT",
	"HOME",
	"END",
	"PGUP",
	"PGDN",

	// Function Key
	"F1",
	"F2",
	"F3",
	"F4",
	"F5",
	"F6",
	"F7",
	"F8",
	"F9",
	"F10",
	"F11",
	"F12",
	"F13",
	"F14",
	"F15",
    
	// Modifier Keys  
	"NUMLOCK",
	"CAPSLOCK",
	"SCROLLLOCK",
	"SHIFT",
	"RIGHTSHIFT",
	"LEFTSHIFT",
	"CTRL",
	"RIGHTCTRL",
	"LEFTCTRL",
	"ALT",
	"RIGHTALT",
	"LEFTALT",
	"RIGHTMETA",
	"LEFTMETA",
	"RIGHTWINDOWS",
	"LEFTWINDOWS",
	"ALTGR",
	"COMPOSE",
    
	// Other
	"HELP",
	"PRINT",
	"SYSREQ",
	"BREAK",
	"MENU",
	"POWER",
	"EURO",
	"UNDO"
};

MousePartsStruct CInputSystem::s_mseParts[] = 
{
	// X Axis (Axis 0)
	{ "XAXIS",         MouseXAxis },
	{ "XAXIS_INV",     MouseXAxisInv },
	{ "XAXIS_POS",     MouseXAxisPos },
	{ "XAXIS_NEG",     MouseXAxisNeg },
	{ "RIGHT",         MouseXAxisPos },
	{ "LEFT",          MouseXAxisNeg },
	{ "AXIS1",         MouseXAxis },
	{ "AXIS1_INV",     MouseXAxisInv },
	{ "AXIS1_POS",     MouseXAxisPos },
	{ "AXIS1_NEG",     MouseXAxisNeg },
	
	// Y Axis (Axis 1)
	{ "YAXIS",         MouseYAxis },
	{ "YAXIS_INV",     MouseYAxisInv },
	{ "YAXIS_POS",     MouseYAxisPos },
	{ "YAXIS_NEG",     MouseYAxisNeg },
	{ "DOWN",          MouseYAxisPos },
	{ "UP",            MouseYAxisNeg },
	{ "AXIS2",         MouseYAxis },
	{ "AXIS2_INV",     MouseYAxisInv },
	{ "AXIS2_POS",     MouseYAxisPos },
	{ "AXIS2_NEG",     MouseYAxisNeg },

	// Z/Wheel Axis (Axis 2)
	{ "ZAXIS",         MouseZAxis },
	{ "ZAXIS_INV",     MouseZAxisInv },
	{ "ZAXIS_POS",     MouseZAxisPos },
	{ "ZAXIS_NEG",     MouseZAxisNeg },
	{ "WHEEL_UP",      MouseZAxisPos },
	{ "WHEEL_DOWN",    MouseZAxisNeg },
	{ "AXIS3",         MouseZAxis },
	{ "AXIS3_INV",     MouseZAxisInv },
	{ "AXIS3_POS",     MouseZAxisPos },
	{ "AXIS3_NEG",     MouseZAxisNeg },

	// Left/Middle/Right Buttons (Buttons 0-2)
	{ "LEFT_BUTTON",   MouseButtonLeft },
	{ "BUTTON1",       MouseButtonLeft },
	{ "MIDDLE_BUTTON", MouseButtonMiddle },
	{ "BUTTON2",       MouseButtonMiddle },
	{ "RIGHT_BUTTON",  MouseButtonRight },
	{ "BUTTON3",       MouseButtonRight },

	// Extra Buttons (Buttons 3 & 4)
	{ "BUTTONX1",      MouseButtonX1 },
	{ "BUTTON4",       MouseButtonX1 },
	{ "BUTTONX2",      MouseButtonX2 },
	{ "BUTTON5",       MouseButtonX2 },

	// List terminator
	{ NULL,            MouseUnknown }
};

JoyPartsStruct CInputSystem::s_joyParts[] = 
{
	// X-Axis (Axis 0)
	{ "XAXIS",         JoyXAxis },
	{ "XAXIS_INV",     JoyXAxisInv },
	{ "XAXIS_POS",     JoyXAxisPos },
	{ "XAXIS_NEG",     JoyXAxisNeg },
	{ "RIGHT",         JoyXAxisPos },
	{ "LEFT",          JoyXAxisNeg },
	{ "AXIS1",         JoyXAxis },
	{ "AXIS1_INV",     JoyXAxisInv },
	{ "AXIS1_POS",     JoyXAxisPos },
	{ "AXIS1_NEG",     JoyXAxisNeg },
	
	// Y-Axis (Axis 1)
	{ "YAXIS",         JoyYAxis },
	{ "YAXIS_INV",     JoyYAxisInv },
	{ "YAXIS_POS",     JoyYAxisPos },
	{ "YAXIS_NEG",     JoyYAxisNeg },
	{ "DOWN",          JoyYAxisPos },
	{ "UP",            JoyYAxisNeg },
	{ "AXIS2",         JoyYAxis },
	{ "AXIS2_INV",     JoyYAxisInv },
	{ "AXIS2_POS",     JoyYAxisPos },
	{ "AXIS2_NEG",     JoyYAxisNeg },
	
	// Z-Axis (Axis 2)
	{ "ZAXIS",         JoyZAxis },
	{ "ZAXIS_INV",     JoyZAxisInv },
	{ "ZAXIS_POS",     JoyZAxisPos },
	{ "ZAXIS_NEG",     JoyZAxisNeg },
	{ "AXIS3",         JoyZAxis },
	{ "AXIS3_INV",     JoyZAxisInv },
	{ "AXIS3_POS",     JoyZAxisPos },
	{ "AXIS3_NEG",     JoyZAxisNeg },

	// RX-Axis (Axis 3)
	{ "RXAXIS",        JoyRXAxis },
	{ "RXAXIS_INV",    JoyRXAxisInv },
	{ "RXAXIS_POS",    JoyRXAxisPos },
	{ "RXAXIS_NEG",    JoyRXAxisNeg },
	{ "AXIS4",         JoyRXAxis },
	{ "AXIS4_INV",     JoyRXAxisInv },
	{ "AXIS4_POS",     JoyRXAxisPos },
	{ "AXIS4_NEG",     JoyRXAxisNeg },

	// RY-Axis (Axis 4)
	{ "RYAXIS",        JoyRYAxis },
	{ "RYAXIS_INV",    JoyRYAxisInv },
	{ "RYAXIS_POS",    JoyRYAxisPos },
	{ "RYAXIS_NEG",    JoyRYAxisNeg },
	{ "AXIS5",         JoyRYAxis },
	{ "AXIS5_INV",     JoyRYAxisInv },
	{ "AXIS5_POS",     JoyRYAxisPos },
	{ "AXIS5_NEG",     JoyRYAxisNeg },
	
	// RZ-Axis (Axis 5)
	{ "RZAXIS",        JoyRZAxis },
	{ "RZAXIS_INV",    JoyRZAxisInv },
	{ "RZAXIS_POS",    JoyRZAxisPos },
	{ "RZAXIS_NEG",    JoyRZAxisNeg },
	{ "AXIS6",         JoyRZAxis },
	{ "AXIS6_INV",     JoyRZAxisInv },
	{ "AXIS6_POS",     JoyRZAxisPos },
	{ "AXIS6_NEG",     JoyRZAxisNeg },

	// POV Hat 0
	{ "POV1_UP",       JoyPOV0Up },
	{ "POV1_DOWN",     JoyPOV0Down },
	{ "POV1_LEFT",     JoyPOV0Left },
	{ "POV1_RIGHT",    JoyPOV0Right },

	// POV Hat 1
	{ "POV2_UP",       JoyPOV1Up },
	{ "POV2_DOWN",     JoyPOV1Down },
	{ "POV2_LEFT",     JoyPOV1Left },
	{ "POV2_RIGHT",    JoyPOV1Right },

	// POV Hat 2
	{ "POV3_UP",       JoyPOV2Up },
	{ "POV3_DOWN",     JoyPOV2Down },
	{ "POV3_LEFT",     JoyPOV2Left },
	{ "POV3_RIGHT",    JoyPOV2Right },

	// POV Hat 3
	{ "POV4_UP",       JoyPOV3Up },
	{ "POV4_DOWN",     JoyPOV3Down },
	{ "POV4_LEFT",     JoyPOV3Left },
	{ "POV4_RIGHT",    JoyPOV3Right },

	// Buttons 0-31
	{ "BUTTON1",       JoyButton0 },
	{ "BUTTON2",       JoyButton1 },
	{ "BUTTON3",       JoyButton2 },
	{ "BUTTON4",       JoyButton3 },
	{ "BUTTON5",       JoyButton4 },
	{ "BUTTON6",       JoyButton5 },
	{ "BUTTON7",       JoyButton6 },
	{ "BUTTON8",       JoyButton7 },
	{ "BUTTON9",       JoyButton8 },
	{ "BUTTON10",      JoyButton9 },
	{ "BUTTON11",      JoyButton10 },
	{ "BUTTON12",      JoyButton11 },
	{ "BUTTON13",      JoyButton12 },
	{ "BUTTON14",      JoyButton13 },
	{ "BUTTON15",      JoyButton14 },
	{ "BUTTON16",      JoyButton15 },
	{ "BUTTON17",      JoyButton16 },
	{ "BUTTON18",      JoyButton17 },
	{ "BUTTON19",      JoyButton18 },
	{ "BUTTON20",      JoyButton19 },
	{ "BUTTON21",      JoyButton20 },
	{ "BUTTON22",      JoyButton21 },
	{ "BUTTON23",      JoyButton22 },
	{ "BUTTON24",      JoyButton23 },
	{ "BUTTON25",      JoyButton24 },
	{ "BUTTON26",      JoyButton25 },
	{ "BUTTON27",      JoyButton26 },
	{ "BUTTON28",      JoyButton27 },
	{ "BUTTON29",      JoyButton28 },
	{ "BUTTON30",      JoyButton29 },
	{ "BUTTON31",      JoyButton30 },
	{ "BUTTON32",      JoyButton31 },

	// List terminator
	{ NULL,            JoyUnknown }
};

const char *CInputSystem::s_axisNames[] = { "X", "Y", "Z", "RX", "RY", "RZ" };

const char *CInputSystem::GetDefaultAxisName(int axisNum)
{
	return (axisNum >= 0 && axisNum < 6 ? s_axisNames[axisNum] : "");
}

CInputSystem::CInputSystem(const char *systemName) : 
	name(systemName), m_dispX(0), m_dispY(0), m_dispW(0), m_dispH(0), m_grabMouse(false)
{
	m_emptySource = new CMultiInputSource();
	m_emptySource->Acquire();
}

CInputSystem::~CInputSystem()
{
	m_emptySource->Release();

	ClearSettings();
	ClearSourceCache(true);

#ifdef DEBUG
	if (totalSrcsAcquired != totalSrcsReleased)
		printf("WARNING - number of input source acquisitions (%u) does not equal number of releases (%u)\n", totalSrcsAcquired, totalSrcsReleased);
#endif
}

void CInputSystem::CreateSourceCache()
{
	// Create cache for key sources
	m_anyKeySources = new CInputSource*[NUM_VALID_KEYS];
	memset(m_anyKeySources, NULL, sizeof(CInputSource*) * NUM_VALID_KEYS);
	if (m_numKbds != ANY_KEYBOARD)
	{
		m_keySources = new CInputSource**[m_numKbds];
		for (int kbdNum = 0; kbdNum < m_numKbds; kbdNum++)
		{
			m_keySources[kbdNum] = new CInputSource*[NUM_VALID_KEYS];
			memset(m_keySources[kbdNum], NULL, sizeof(CInputSource*) * NUM_VALID_KEYS);
		}
	}

	// Create cache for mouse sources
	m_anyMseSources = new CInputSource*[NUM_MOUSE_PARTS];
	memset(m_anyMseSources, NULL, sizeof(CInputSource*) * NUM_MOUSE_PARTS);
	if (m_numMice != ANY_MOUSE)
	{
		m_mseSources = new CInputSource**[m_numMice];
		for (int mseNum = 0; mseNum < m_numMice; mseNum++)
		{
			m_mseSources[mseNum] = new CInputSource*[NUM_MOUSE_PARTS];
			memset(m_mseSources[mseNum], NULL, sizeof(CInputSource*) * NUM_MOUSE_PARTS);
		}
	}

	// Create cache for joystick sources
	m_anyJoySources = new CInputSource*[NUM_JOY_PARTS];
	memset(m_anyJoySources, NULL, sizeof(CInputSource*) * NUM_JOY_PARTS);
	if (m_numJoys != ANY_JOYSTICK)
	{
		m_joySources = new CInputSource**[m_numJoys];
		for (int joyNum = 0; joyNum < m_numJoys; joyNum++)
		{
			m_joySources[joyNum] = new CInputSource*[NUM_JOY_PARTS];
			memset(m_joySources[joyNum], NULL, sizeof(CInputSource*) * NUM_JOY_PARTS);
		}
	}
}	

void CInputSystem::ClearSourceCache(bool deleteCache)
{
	// Clear cache of keyboard sources
	if (m_anyKeySources != NULL)
	{
		for (int keyIndex = 0; keyIndex < NUM_VALID_KEYS; keyIndex++)
			ReleaseSource(m_anyKeySources[keyIndex]);
		if (deleteCache)
		{
			delete[] m_anyKeySources;
			m_anyKeySources = NULL;
		}
		if (m_numKbds != ANY_KEYBOARD)
		{
			for (int kbdNum = 0; kbdNum < m_numKbds; kbdNum++)
			{
				for (int keyIndex = 0; keyIndex < NUM_VALID_KEYS; keyIndex++)
					ReleaseSource(m_keySources[kbdNum][keyIndex]);
				if (deleteCache)
					delete[] m_keySources[kbdNum];
			}
			if (deleteCache)
			{
				delete[] m_keySources;
				m_keySources = NULL;
			}
		}
	}

	// Clear cache of mouse sources
	if (m_anyMseSources != NULL)
	{
		for (int mseIndex = 0; mseIndex < NUM_MOUSE_PARTS; mseIndex++)
			ReleaseSource(m_anyMseSources[mseIndex]);
		if (deleteCache)
		{
			delete[] m_anyMseSources;
			m_anyMseSources = NULL;
		}
		if (m_numMice != ANY_MOUSE)
		{
			for (int mseNum = 0; mseNum < m_numMice; mseNum++)
			{
				for (int mseIndex = 0; mseIndex < NUM_MOUSE_PARTS; mseIndex++)
					ReleaseSource(m_mseSources[mseNum][mseIndex]);
				if (deleteCache)
					delete[] m_mseSources[mseNum];
			}
			if (deleteCache)
			{
				delete[] m_mseSources;
				m_mseSources = NULL;
			}
		}
	}

	// Clear cache of joystick sources
	if (m_anyJoySources != NULL)
	{
		for (int joyIndex = 0; joyIndex < NUM_JOY_PARTS; joyIndex++)
			ReleaseSource(m_anyJoySources[joyIndex]);
		if (deleteCache)
		{
			delete[] m_anyJoySources;
			m_anyJoySources = NULL;
		}
		if (m_numJoys != ANY_JOYSTICK)
		{
			for (int joyNum = 0; joyNum < m_numJoys; joyNum++)
			{
				for (int joyIndex = 0; joyIndex < NUM_JOY_PARTS; joyIndex++)
					ReleaseSource(m_joySources[joyNum][joyIndex]);
				if (deleteCache)
					delete[] m_joySources[joyNum];
			}
			if (deleteCache)
			{
				delete[] m_joySources;
				m_joySources = NULL;
			}
		}
	}
}

void CInputSystem::ReleaseSource(CInputSource *&source)
{
	if (source != NULL)
		source->Release();
	source = NULL;
}
	
CInputSource *CInputSystem::GetKeySource(int kbdNum, int keyIndex)
{
	if (kbdNum == ANY_KEYBOARD)
	{
		// Check keyboard source cache first
		if (m_anyKeySources[keyIndex] == NULL)
		{
			if (m_numKbds == ANY_KEYBOARD)
				m_anyKeySources[keyIndex] = CreateKeySource(ANY_KEYBOARD, keyIndex);
			else
				m_anyKeySources[keyIndex] = CreateAnyKeySource(keyIndex);
			m_anyKeySources[keyIndex]->Acquire();
		}
		return m_anyKeySources[keyIndex];
	}
	else if (kbdNum < m_numKbds)
	{
		// Check keyboard source cache first
		if (m_keySources[kbdNum][keyIndex] == NULL)
		{
			m_keySources[kbdNum][keyIndex] = CreateKeySource(kbdNum, keyIndex);
			m_keySources[kbdNum][keyIndex]->Acquire();
		}
		return m_keySources[kbdNum][keyIndex];
	}
	else
		return m_emptySource;
}

CInputSource *CInputSystem::GetMouseSource(int mseNum, EMousePart msePart)
{
	int mseIndex = (int)msePart;
	if (mseNum == ANY_MOUSE)
	{
		// Check mouse source cache first
		if (m_anyMseSources[mseIndex] == NULL)
		{
			if (m_numMice == ANY_MOUSE)
				m_anyMseSources[mseIndex] = CreateMouseSource(ANY_MOUSE, msePart);
			else
				m_anyMseSources[mseIndex] = CreateAnyMouseSource(msePart);
			m_anyMseSources[mseIndex]->Acquire();
		}
		return m_anyMseSources[mseIndex];
	}
	else if (mseNum < m_numMice)
	{
		// Check mouse source cache first
		if (m_mseSources[mseNum][mseIndex] == NULL)
		{
			m_mseSources[mseNum][mseIndex] = CreateMouseSource(mseNum, msePart);
			m_mseSources[mseNum][mseIndex]->Acquire();
		}
		return m_mseSources[mseNum][mseIndex];
	}
	else
		return m_emptySource;
}

CInputSource *CInputSystem::GetJoySource(int joyNum, EJoyPart joyPart)
{
	int joyIndex = (int)joyPart;
	if (joyNum == ANY_JOYSTICK)
	{
		// Check joystick source cache first
		if (m_anyJoySources[joyIndex] == NULL)
		{
			if (m_numJoys == ANY_JOYSTICK)
				m_anyJoySources[joyIndex] = CreateJoySource(ANY_JOYSTICK, joyPart);
			else
				m_anyJoySources[joyIndex] = CreateAnyJoySource(joyPart);
			m_anyJoySources[joyIndex]->Acquire();
		}
		return m_anyJoySources[joyIndex];
	}
	else if (joyNum < m_numJoys)
	{
		// Check joystick source cache first
		if (m_joySources[joyNum][joyIndex] == NULL)
		{
			m_joySources[joyNum][joyIndex] = CreateJoySource(joyNum, joyPart);
			m_joySources[joyNum][joyIndex]->Acquire();
		}
		return m_joySources[joyNum][joyIndex];
	}
	else
		return m_emptySource;
}

void CInputSystem::CheckAllSources(unsigned readFlags, bool fullAxisOnly, bool &mseCentered, vector<CInputSource*> &sources, string &mapping, vector<CInputSource*> &badSources)
{
	// See if should read keyboards
	if (readFlags & READ_KEYBOARD)
	{
		// Check all keyboard sources for inputs, merging devices if required
		if ((readFlags & READ_MERGE_KEYBOARD) || m_numKbds == ANY_KEYBOARD)
			CheckKeySources(ANY_KEYBOARD, fullAxisOnly, sources, mapping, badSources);
		else
		{
			for (int kbdNum = 0; kbdNum < m_numKbds; kbdNum++)
				CheckKeySources(kbdNum, fullAxisOnly, sources, mapping, badSources);
		}
	}

	// See if should read mice
	if (readFlags & READ_MOUSE)
	{
		// For mouse input, wait until mouse is in centre of display before parsing X- and Y-axis movements
		if (!mseCentered)
			mseCentered = ConfigMouseCentered();
	
		// Check all mouse sources for input, merging devices if required
		if ((readFlags & READ_MERGE_MOUSE) || m_numMice == ANY_MOUSE)
			CheckMouseSources(ANY_MOUSE, fullAxisOnly, mseCentered, sources, mapping, badSources);
		else
		{
			for (int mseNum = 0; mseNum < m_numMice; mseNum++)
				CheckMouseSources(mseNum, fullAxisOnly, mseCentered, sources, mapping, badSources);
		}
	}
	
	// See if should read joysticks
	if (readFlags & READ_JOYSTICK)
	{
		// Check all joystick sources, merging devices if required
		if ((readFlags & READ_MERGE_JOYSTICK) || m_numJoys == ANY_JOYSTICK)
			CheckJoySources(ANY_JOYSTICK, fullAxisOnly, sources, mapping, badSources);
		else
		{
			for (int joyNum = 0; joyNum < m_numJoys; joyNum++)
				CheckJoySources(joyNum, fullAxisOnly, sources, mapping, badSources);
		}
	}
}

void CInputSystem::CheckKeySources(int kbdNum, bool fullAxisOnly, vector<CInputSource*> &sources, string &mapping, vector<CInputSource*> &badSources)
{
	// Loop through all valid keys
	for (int i = 0; i < NUM_VALID_KEYS; i++)
	{
		const char *keyName = s_validKeyNames[i];
		int keyIndex = GetKeyIndex(keyName);
		if (keyIndex < 0)
			continue;
		// Get key source for keyboard number and key and test to see if it is active (but was not previously) and that it is not a "bad" source
		CInputSource *source = GetKeySource(kbdNum, keyIndex);
		if (source != NULL && source->IsActive() && find(sources.begin(), sources.end(), source) == sources.end() &&
			find(badSources.begin(), badSources.end(), source) == badSources.end())
		{
			// Update mapping string and add source to list
			if (sources.size() == 0)
				mapping.assign("KEY");
			else
				mapping.append("+KEY");
			if (kbdNum >= 0)
				mapping.append(IntToString(kbdNum + 1));
			mapping.append("_");
			mapping.append(keyName);
			sources.push_back(source);
		}
	}
}

void CInputSystem::CheckMouseSources(int mseNum, bool fullAxisOnly, bool mseCentered, vector<CInputSource*> &sources, string &mapping, vector<CInputSource*> &badSources)
{
	// Loop through all mouse parts
	for (int mseIndex = 0; mseIndex < NUM_MOUSE_PARTS; mseIndex++)
	{
		EMousePart msePart = (EMousePart)mseIndex;
		// Get axis details
		int axisNum;
		int axisDir;
		bool isAxis = GetAxisDetails(msePart, axisNum, axisDir);
		bool isXYAxis = isAxis && (axisNum == AXIS_X || axisNum == AXIS_Y);
		// Ignore X- & Y-axes if mouse hasn't been centered yet and filter axes according to fullAxisOnly
		if (isXYAxis && !mseCentered || isAxis && (IsFullAxis(msePart) && !fullAxisOnly || !IsFullAxis(msePart) && fullAxisOnly))
			continue;
		// Get mouse source for mouse number and part and test to see if it is active (but was not previously) and that it is not a "bad" source
		CInputSource *source = GetMouseSource(mseNum, msePart);
		if (source != NULL && source->IsActive() && find(sources.begin(), sources.end(), source) == sources.end() &&
			find(badSources.begin(), badSources.end(), source) == badSources.end())
		{
			// Otherwise, update mapping string and add source to list
			const char *partName = LookupName(msePart);
			if (sources.size() == 0)
				mapping.assign("MOUSE");
			else
				mapping.append("+MOUSE");
			if (mseNum >= 0)
				mapping.append(IntToString(mseNum + 1));
			mapping.append("_");
			mapping.append(partName);
			sources.push_back(source);
		}
	}
}

void CInputSystem::CheckJoySources(int joyNum, bool fullAxisOnly, vector<CInputSource*> &sources, string &mapping, vector<CInputSource*> &badSources)
{
	// Loop through all joystick parts
	for (int joyIndex = 0; joyIndex < NUM_JOY_PARTS; joyIndex++)
	{
		EJoyPart joyPart = (EJoyPart)joyIndex;
		// Filter axes according to fullAxisOnly
		if (IsAxis(joyPart) && (IsFullAxis(joyPart) && !fullAxisOnly || !IsFullAxis(joyPart) && fullAxisOnly))
			continue;
		// Get joystick source for joystick number and part and test to see if it is active (but was not previously) and that it is not a "bad" source
		CInputSource *source = GetJoySource(joyNum, joyPart);
		if (source != NULL && source->IsActive() && find(sources.begin(), sources.end(), source) == sources.end() &&
			find(badSources.begin(), badSources.end(), source) == badSources.end())
		{
			// Otherwise, update mapping string and add source to list
			const char *partName = LookupName(joyPart);
			if (sources.size() == 0)
				mapping.assign("JOY");
			else
				mapping.append("+JOY");
			if (joyNum >= 0)
				mapping.append(IntToString(joyNum + 1));
			mapping.append("_");
			mapping.append(partName);
			sources.push_back(source);
		}
	}
}

bool CInputSystem::ParseInt(string str, int &num)
{
	stringstream ss(str);
	return !(ss >> num).fail();
}

string CInputSystem::IntToString(int num)
{
	stringstream ss;
	ss << num;
	return ss.str();
}

bool CInputSystem::EqualsIgnoreCase(string str1, const char *str2)
{
	for (string::const_iterator ci = str1.begin(); ci < str1.end(); ci++) 
	{
		if (*str2 == '\0' || tolower(*ci) != tolower(*str2))
			return false;
		str2++;
	}
	return *str2 == '\0';
}

bool CInputSystem::StartsWithIgnoreCase(string str1, const char *str2)
{
	for (string::const_iterator ci = str1.begin(); ci < str1.end(); ci++) 
	{
		if (*str2 == '\0')
			return true;
		if (tolower(*ci) != tolower(*str2))
			return false;
		str2++;
	}
	return *str2 == '\0'; 
}

bool CInputSystem::IsValidKeyName(string str)
{
	for (int i = 0; i < NUM_VALID_KEYS; i++)
	{
		if (EqualsIgnoreCase(str, s_validKeyNames[i]))
			return true;
	}
	return false;
}

EMousePart CInputSystem::LookupMousePart(string str)
{
	for (int i = 0; s_mseParts[i].id != NULL; i++)
	{
		if (EqualsIgnoreCase(str, s_mseParts[i].id))
			return s_mseParts[i].msePart;
	}
	return MouseUnknown;
}

const char *CInputSystem::LookupName(EMousePart msePart)
{
	for (int i = 0; s_mseParts[i].id != NULL; i++)
	{
		if (msePart == s_mseParts[i].msePart)
			return s_mseParts[i].id;
	}
	return NULL;
}

EJoyPart CInputSystem::LookupJoyPart(string str)
{
	for (int i = 0; s_joyParts[i].id != NULL; i++)
	{
		if (EqualsIgnoreCase(str, s_joyParts[i].id))
			return s_joyParts[i].joyPart;
	}
	return JoyUnknown;
}

const char *CInputSystem::LookupName(EJoyPart joyPart)
{
	for (int i = 0; s_joyParts[i].id != NULL; i++)
	{
		if (joyPart == s_joyParts[i].joyPart)
			return s_joyParts[i].id;
	}
	return NULL;
}

size_t CInputSystem::ParseDevMapping(string str, const char *devType, int &devNum)
{
	if (!StartsWithIgnoreCase(str, devType))
		return -1;
	size_t size = str.size();
	size_t devLen = strlen(devType);

	// Parse optional device number
	devNum = -1;
	size_t i = devLen;
	while (i < size && isdigit(str[i]))
		i++;
	if (i > devLen && i < size)
	{
		if (!ParseInt(str.substr(devLen, i), devNum))
			return -1;
		devNum--;
	}

	// Check hyphen preceeds device part
	if (i < size - 1 && str[i] == '_')
		return i + 1;
	else
		return -1;
}

CInputSource* CInputSystem::ParseMultiSource(string str, bool fullAxisOnly, bool isOr)
{
	// Check for empty or NONE mapping
	size_t size = str.size();
	if (size == 0 || EqualsIgnoreCase(str, "NONE"))
		return NULL;

	CInputSource *source = NULL;
	bool isMulti = false;
	vector<CInputSource*> sources;

	size_t start = 0;
	size_t end;
	do
	{
		// Remove leading whitespace/quotes and see if have any plusses/commas in mapping (to specify and/or multiple assignments)
		while ((isspace(str[start]) || str[start] == '"') && start < size - 1)
			start++;
		char delim = (isOr ? ',' : '+');
		end = str.find(delim, start);
		if (end == string::npos)
			end = size;

		// Remove tailing whitespace/quotes
		size_t subEnd = end;
		while (subEnd > 0 && (isspace(str[subEnd - 1]) || str[start] == '"'))
			subEnd--;
		string subStr = str.substr(start, subEnd - start);
		start = end + 1;

		// Parse the multi/single source in substring
		CInputSource *parsed;
		if (isOr)
			parsed = ParseMultiSource(subStr, fullAxisOnly, false);
		else
			parsed = ParseSingleSource(subStr);

		// Check the result is valid
		if (parsed != NULL && parsed->type != SourceInvalid &&
			(parsed->type == SourceEmpty || parsed->type == SourceFullAxis && fullAxisOnly || parsed->type != SourceFullAxis && !fullAxisOnly))
		{
			// Keep track of all sources parsed
			if (isMulti)
				sources.push_back(parsed);
			else if (source != NULL)
			{
				sources.push_back(source);
				sources.push_back(parsed);
				isMulti = true;
			}
			else
				source = parsed;
		}
	}
	while (start < size);

	// If only parsed a single source, return that, otherwise return a CMultiInputSource combining all the sources
	return (isMulti ? new CMultiInputSource(isOr, sources) : source);
}

CInputSource *CInputSystem::ParseSingleSource(string str)
{
	// First, check for ! at beginning of string, which means input source must not be activated
	if (str[0] == '!')
	{
		// If found, skip any whitespace after that and get remaining string and parse it again
		size_t i = 1;
		while (i < str.size() && str[i] == ' ')
			i++;
		str.erase(0, i);

		CInputSource *source = ParseSingleSource(str);
		if (source != NULL && source != m_emptySource)
			return new CNegInputSource(source);
		else
			return source;
	}	

	// Try parsing a key mapping
	int kbdNum;
	int keyNameIndex = ParseDevMapping(str, "KEY", kbdNum);
	if (keyNameIndex >= 0)
	{
		string keyName = str.substr(keyNameIndex);
		if (IsValidKeyName(keyName))
		{
			// Lookup key index and map to key source
			int keyIndex = GetKeyIndex(keyName.c_str());
			if (keyIndex >= 0)
				return GetKeySource(kbdNum, keyIndex);
			else if (EqualsIgnoreCase(keyName, "SHIFT") || EqualsIgnoreCase(keyName, "CTRL") || EqualsIgnoreCase(keyName, "ALT"))
			{
				// Handle special cases if not handled by subclass: SHIFT, CTRL and ALT
				string leftName = "LEFT" + keyName;
				string rightName = "RIGHT" + keyName;
				int leftIndex = GetKeyIndex(leftName.c_str());
				int rightIndex = GetKeyIndex(rightName.c_str());
				vector<CInputSource*> sources;
				if (leftIndex >= 0)
				{
					CInputSource *leftSource = GetKeySource(kbdNum, leftIndex);
					if (leftSource != NULL)
						sources.push_back(leftSource);
				}
				if (rightIndex >= 0)
				{
					CInputSource *rightSource = GetKeySource(kbdNum, rightIndex);
					if (rightSource != NULL)
						sources.push_back(rightSource);
				}
				if (sources.size() > 0)
					return new CMultiInputSource(true, sources);
			}
			return m_emptySource;
		}
	}
	
	// Try parsing a mouse mapping
	int mseNum;
	int msePartIndex = ParseDevMapping(str, "MOUSE", mseNum);
	if (msePartIndex >= 0)
	{
		// Lookup mouse part and map to mouse source
		EMousePart msePart = LookupMousePart(str.substr(msePartIndex));
		if (msePart != MouseUnknown)
			return GetMouseSource(mseNum, msePart);
	}

	// Try parsing a joystick mapping
	int joyNum;
	int joyPartIndex = ParseDevMapping(str, "JOY", joyNum);
	if (joyPartIndex >= 0)
	{
		// Lookup joystick part and map to joystick source
		EJoyPart joyPart = LookupJoyPart(str.substr(joyPartIndex));
		if (joyPart != JoyUnknown)
			return GetJoySource(joyNum, joyPart);
	}

	// As last option, assume mapping is just a key name and try creating keyboard source from it (to retain compatibility with previous 
	// versions of Supermodel)
	if (IsValidKeyName(str))
	{
		// Lookup key index and map to key source
		int keyIndex = GetKeyIndex(str.c_str());
		if (keyIndex >= 0)
			return GetKeySource(ANY_KEYBOARD, keyIndex);
		else
			return m_emptySource;
	}

	// If got here, it was not possible to parse mapping string so return NULL
	return NULL;
}

void CInputSystem::PrintKeySettings(int kbdNum, KeySettings *settings)
{
	printf(" Sensitivity = %d %%\n", settings->sensitivity);
	printf(" Decay Speed = %d %%\n", settings->decaySpeed);
}

KeySettings *CInputSystem::ReadKeySettings(CINIFile *ini, const char *section, int kbdNum)
{
	// Get common key settings and create new key settings based on that
	KeySettings *common = (kbdNum != ANY_KEYBOARD ? GetKeySettings(ANY_KEYBOARD, true) : &m_defKeySettings);
	KeySettings *settings = new KeySettings(*common);
	settings->kbdNum = kbdNum;

	// Read settings from ini file
	string baseKey("InputKey");
	if (kbdNum != ANY_KEYBOARD)
		baseKey.append(IntToString(kbdNum + 1));
	bool read = false;
	read |= ini->Get(section, baseKey + "Sensitivity", settings->sensitivity) == OKAY;
	read |= ini->Get(section, baseKey + "DecaySpeed", settings->decaySpeed) == OKAY;
	if (read)
		return settings;
	delete settings;
	return NULL;
}

void CInputSystem::WriteKeySettings(CINIFile *ini, const char *section, KeySettings *settings)
{
	// Get common key settings
	KeySettings *common = (settings->kbdNum != ANY_KEYBOARD ? GetKeySettings(ANY_KEYBOARD, true) : &m_defKeySettings);

	// Write to ini file any settings that are different to common settings
	string baseKey("InputKey");
	if (settings->kbdNum != ANY_KEYBOARD)
		baseKey.append(IntToString(settings->kbdNum + 1));
	if (settings->sensitivity != common->sensitivity) ini->Set(section, baseKey + "Sensitivity", settings->sensitivity);
	if (settings->decaySpeed != common->decaySpeed) ini->Set(section, baseKey + "DecaySpeed", settings->decaySpeed);
}

void CInputSystem::PrintMouseSettings(int mseNum, MouseSettings *settings)
{
	for (int axisNum = 0; axisNum < NUM_MOUSE_AXES; axisNum++)
	{
		const char *axisName = s_axisNames[axisNum];
		printf(" %s-Axis Dead Zone = %d %%\n", axisName, settings->deadZones[axisNum]);
	}
}

MouseSettings *CInputSystem::ReadMouseSettings(CINIFile *ini, const char *section, int mseNum)
{
	// Get common mouse settings and create new mouse settings based on that
	MouseSettings *common = (mseNum != ANY_MOUSE ? GetMouseSettings(ANY_MOUSE, true) : &m_defMseSettings);
	MouseSettings *settings = new MouseSettings(*common);
	settings->mseNum = mseNum;

	// Read settings from ini file
	string baseKey("InputMouse");
	if (mseNum != ANY_MOUSE)
		baseKey.append(IntToString(mseNum + 1));
	bool read = false;
	for (int axisNum = 0; axisNum < NUM_MOUSE_AXES; axisNum++)
	{
		const char *axisName = s_axisNames[axisNum];
		read |= ini->Get(section, baseKey + axisName + "DeadZone", settings->deadZones[axisNum]) == OKAY; 
	}
	if (read)
		return settings;
	delete settings;
	return NULL;
}

void CInputSystem::WriteMouseSettings(CINIFile *ini, const char *section, MouseSettings *settings)
{
	// Get common mouse settings
	MouseSettings *common = (settings->mseNum != ANY_MOUSE ? GetMouseSettings(ANY_MOUSE, true) : &m_defMseSettings);
	
	// Write to ini file any settings that are different to common/default settings
	string baseKey("InputMouse");
	if (settings->mseNum != ANY_MOUSE)
		baseKey.append(IntToString(settings->mseNum + 1));
	for (int axisNum = 0; axisNum < NUM_MOUSE_AXES; axisNum++)
	{
		const char *axisName = s_axisNames[axisNum];
		if (settings->deadZones[axisNum] != common->deadZones[axisNum]) 
			ini->Set(section, baseKey + axisName + "DeadZone", settings->deadZones[axisNum]);
	}
}

void CInputSystem::PrintJoySettings(int joyNum, JoySettings *settings)
{
	const JoyDetails *joyDetails = (joyNum != ANY_JOYSTICK ? GetJoyDetails(joyNum) : NULL);
	for (int axisNum = 0; axisNum < NUM_JOY_AXES; axisNum++)
	{	
		if (joyDetails && !joyDetails->hasAxis[axisNum])
			continue;
		const char *axisName = s_axisNames[axisNum];
		printf(" %-2s-Axis Min Value        = %d\n", axisName, settings->axisMinVals[axisNum]);
		printf(" %-2s-Axis Center/Off Value = %d\n", axisName, settings->axisOffVals[axisNum]);
		printf(" %-2s-Axis Max Value        = %d\n", axisName, settings->axisMaxVals[axisNum]);
		printf(" %-2s-Axis Dead Zone        = %d %%\n", axisName, settings->deadZones[axisNum]);
		printf(" %-2s-Axis Saturation       = %d %%\n", axisName, settings->saturations[axisNum]);
	}
}

JoySettings *CInputSystem::ReadJoySettings(CINIFile *ini, const char *section, int joyNum)
{
	// Get common/default joystick settings and create new joystick settings based on that
	JoySettings *common = (joyNum != ANY_JOYSTICK ? GetJoySettings(ANY_JOYSTICK, true) : &m_defJoySettings);
	JoySettings *settings = new JoySettings(*common);
	settings->joyNum = joyNum;

	// Read settings from ini file
	string baseKey("InputJoy");
	if (joyNum != ANY_JOYSTICK)
		baseKey.append(IntToString(joyNum + 1));
	bool read = false;
	for (int axisNum = 0; axisNum < NUM_JOY_AXES; axisNum++)
	{
		const char *axisName = s_axisNames[axisNum];
		read |= ini->Get(section, baseKey + axisName + "MinVal", settings->axisMinVals[axisNum]) == OKAY; 
		read |= ini->Get(section, baseKey + axisName + "OffVal", settings->axisOffVals[axisNum]) == OKAY; 
		read |= ini->Get(section, baseKey + axisName + "MaxVal", settings->axisMaxVals[axisNum]) == OKAY; 
		read |= ini->Get(section, baseKey + axisName + "DeadZone", settings->deadZones[axisNum]) == OKAY; 
		read |= ini->Get(section, baseKey + axisName + "Saturation", settings->saturations[axisNum]) == OKAY; 
	}
	if (read)
		return settings;
	delete settings;
	return NULL;
}

void CInputSystem::WriteJoySettings(CINIFile *ini, const char *section, JoySettings *settings)
{
	// Get common/default joystick settings
	JoySettings *common = (settings->joyNum != ANY_JOYSTICK ? GetJoySettings(ANY_JOYSTICK, true) : &m_defJoySettings);

	// Write to ini file any settings that are different to common/default settings
	string baseKey("InputJoy");
	if (settings->joyNum != ANY_JOYSTICK)
		baseKey.append(IntToString(settings->joyNum + 1));
	for (int axisNum = 0; axisNum < NUM_JOY_AXES; axisNum++)
	{
		const char *axisName = s_axisNames[axisNum];
		if (settings->axisMinVals[axisNum] != common->axisMinVals[axisNum]) 
			ini->Set(section, baseKey + axisName + "MinVal", settings->axisMinVals[axisNum]);
		if (settings->axisOffVals[axisNum] != common->axisOffVals[axisNum]) 
			ini->Set(section, baseKey + axisName + "OffVal", settings->axisOffVals[axisNum]);
		if (settings->axisMaxVals[axisNum] != common->axisMaxVals[axisNum])
			ini->Set(section, baseKey + axisName + "MaxVal", settings->axisMaxVals[axisNum]);
		if (settings->deadZones[axisNum] != common->deadZones[axisNum]) 
			ini->Set(section, baseKey + axisName + "DeadZone", settings->deadZones[axisNum]);
		if (settings->saturations[axisNum] != common->saturations[axisNum]) 
			ini->Set(section, baseKey + axisName + "Saturation", settings->saturations[axisNum]);
	}
}

KeySettings *CInputSystem::GetKeySettings(int kbdNum, bool useDefault)
{
	KeySettings *common = NULL;
	for (vector<KeySettings*>::iterator it = m_keySettings.begin(); it != m_keySettings.end(); it++)
	{
		if ((*it)->kbdNum == kbdNum)
			return *it;
		else if ((*it)->kbdNum == ANY_KEYBOARD)
			common = *it;
	}
	if (!useDefault)
		return NULL;
	return (common != NULL ? common : &m_defKeySettings);
}

MouseSettings *CInputSystem::GetMouseSettings(int mseNum, bool useDefault)
{
	MouseSettings *common = NULL;
	for (vector<MouseSettings*>::iterator it = m_mseSettings.begin(); it != m_mseSettings.end(); it++)
	{
		if ((*it)->mseNum == mseNum)
			return *it;
		else if ((*it)->mseNum == ANY_MOUSE)
			common = *it;
	}
	if (!useDefault)
		return NULL;
	return (common != NULL ? common : &m_defMseSettings);
}

JoySettings *CInputSystem::GetJoySettings(int joyNum, bool useDefault)
{
	JoySettings *common = NULL;
	for (vector<JoySettings*>::iterator it = m_joySettings.begin(); it != m_joySettings.end(); it++)
	{
		if ((*it)->joyNum == joyNum)
			return *it;
		else if ((*it)->joyNum == ANY_JOYSTICK)
			common = *it;
	}
	if (!useDefault)
		return NULL;
	return (common != NULL ? common : &m_defJoySettings);
}

bool CInputSystem::IsAxis(EMousePart msePart)
{
	return msePart >= MouseXAxis && msePart <= MouseZAxisNeg;
}

bool CInputSystem::IsFullAxis(EMousePart msePart)
{
	return IsAxis(msePart) && (((msePart - MouseXAxis) % 4) == AXIS_FULL || ((msePart - MouseXAxis) % 4) == AXIS_INVERTED);
}

bool CInputSystem::GetAxisDetails(EMousePart msePart, int &axisNum, int &axisDir)
{
	if (!IsAxis(msePart))
		return false;
	axisNum = (msePart - MouseXAxis) / 4;
	axisDir = (msePart - MouseXAxis) % 4;
	return true;
}

bool CInputSystem::IsButton(EMousePart msePart)
{
	return msePart >= MouseButtonLeft && msePart <= MouseButtonX2;
}

int CInputSystem::GetButtonNumber(EMousePart msePart)
{
	if (!IsButton(msePart))
		return -1;
	return msePart - MouseButtonLeft;
}

EMousePart CInputSystem::GetMouseAxis(int axisNum, int axisDir)
{
	if (axisNum > 0 || axisNum >= NUM_MOUSE_AXES || axisDir < 0 || axisDir > 3)
		return MouseUnknown;
	return (EMousePart)(MouseXAxis + 4 * axisNum + axisDir);
}

EMousePart CInputSystem::GetMouseButton(int butNum)
{
	if (butNum < 0 || butNum >= NUM_MOUSE_BUTTONS)
		return MouseUnknown;
	return (EMousePart)(MouseButtonLeft + butNum);	
}

bool CInputSystem::IsAxis(EJoyPart joyPart)
{
	return joyPart >= JoyXAxis && joyPart <= JoyRZAxisNeg;
}

bool CInputSystem::IsFullAxis(EJoyPart joyPart)
{
	return IsAxis(joyPart) && (((joyPart - JoyXAxis) % 4) == AXIS_FULL || ((joyPart - JoyXAxis) % 4) == AXIS_INVERTED);
}

bool CInputSystem::GetAxisDetails(EJoyPart joyPart, int &axisNum, int &axisDir)
{
	if (!IsAxis(joyPart))
		return false;
	axisNum = (joyPart - JoyXAxis) / 4;
	axisDir = (joyPart - JoyXAxis) % 4;
	return true;
}

bool CInputSystem::IsPOV(EJoyPart joyPart)
{
	return joyPart >= JoyPOV0Up && joyPart <= JoyPOV3Right;
}

bool CInputSystem::GetPOVDetails(EJoyPart joyPart, int &povNum, int &povDir)
{
	if (!IsPOV(joyPart))
		return false;
	povNum = (joyPart - JoyPOV0Up) / 4;
	povDir = (joyPart - JoyPOV0Up) % 4;
	return true;
}

bool CInputSystem::IsButton(EJoyPart joyPart)
{
	return joyPart >= JoyButton0 && joyPart <= JoyButton31;
}

int CInputSystem::GetButtonNumber(EJoyPart joyPart)
{
	if (!IsButton(joyPart))
		return -1;
	return joyPart - JoyButton0;
}

EJoyPart CInputSystem::GetJoyAxis(int axisNum, int axisDir)
{
	if (axisNum < 0 || axisNum >= NUM_JOY_AXES || axisDir < 0 || axisDir > 3)
		return JoyUnknown;
	return (EJoyPart)(JoyXAxis + 4 * axisNum + axisDir);
}

EJoyPart CInputSystem::GetJoyPOV(int povNum, int povDir)
{
	if (povNum < 0 || povNum >= NUM_JOY_POVS)
		return JoyUnknown;
	return (EJoyPart)(JoyPOV0Up + 4 * povNum + povDir);
}

EJoyPart CInputSystem::GetJoyButton(int butNum)
{
	if (butNum < 0 || butNum >= NUM_JOY_BUTTONS)
		return JoyUnknown;
	return (EJoyPart)(JoyButton0 + butNum);	
}

bool CInputSystem::ConfigMouseCentered()
{
	// Get mouse X & Y
	int mx = GetMouseAxisValue(ANY_MOUSE, AXIS_X);
	int my = GetMouseAxisValue(ANY_MOUSE, AXIS_Y);
	
	// See if mouse in center of display
	unsigned lx = m_dispX + m_dispW / 4;
	unsigned ly = m_dispY + m_dispH / 4;
	return mx >= (int)lx && mx <= (int)(lx + m_dispW / 2) && my >= (int)ly && my <= (int)(ly + m_dispH / 2);
}	

CInputSource *CInputSystem::CreateAnyKeySource(int keyIndex)
{
	// Default ANY_KEYBOARD source is to use CMultiInputSource to combine all individual sources
	vector<CInputSource*> keySrcs;
	for (int kbdNum = 0; kbdNum < m_numKbds; kbdNum++)
	{
		CInputSource *keySrc = CreateKeySource(kbdNum, keyIndex);
		if (keySrc != NULL)
			keySrcs.push_back(keySrc);
	}
	return new CMultiInputSource(true, keySrcs);
}

CInputSource *CInputSystem::CreateAnyMouseSource(EMousePart msePart)
{
	// Default ANY_MOUSE source is to use CMultiInputSource to combine all individual sources
	vector<CInputSource*> mseSrcs;
	for (int mseNum = 0; mseNum < m_numMice; mseNum++)
	{
		CInputSource *mseSrc = CreateMouseSource(mseNum, msePart);
		if (mseSrc != NULL)
			mseSrcs.push_back(mseSrc);
	}
	return new CMultiInputSource(true, mseSrcs);
}

CInputSource *CInputSystem::CreateAnyJoySource(EJoyPart joyPart)
{
	// Default ANY_JOYSTICK source is to use CMultiInputSource to combine all individual sources
	vector<CInputSource*> joySrcs;
	for (int joyNum = 0; joyNum < m_numJoys; joyNum++)
	{
		CInputSource *joySrc = CreateJoySource(joyNum, joyPart);
		if (joySrc != NULL)
			joySrcs.push_back(joySrc);
	}
	return new CMultiInputSource(true, joySrcs);
}

CInputSource *CInputSystem::CreateKeySource(int kbdNum, int keyIndex)
{
	// Get key settings
	KeySettings *settings = GetKeySettings(kbdNum, true);

	// Create source for given key index
	return new CKeyInputSource(this, kbdNum, keyIndex, settings->sensitivity, settings->decaySpeed);
}

CInputSource *CInputSystem::CreateMouseSource(int mseNum, EMousePart msePart)
{
	// Get mouse settings
	MouseSettings *settings = GetMouseSettings(mseNum, true);

	// Create source according to given mouse part
	int axisNum;
	int axisDir;
	if (GetAxisDetails(msePart, axisNum, axisDir))
	{
		// Part is mouse axis so create axis source with appropriate deadzone setting
		return new CMseAxisInputSource(this, mseNum, axisNum, axisDir, settings->deadZones[axisNum]);
	}
	else if (IsButton(msePart))
	{
		// Part is mouse button so map it to button number
		int butNum = GetButtonNumber(msePart);
		if (butNum < 0)
			return NULL;  // Buttons out of range are invalid
		return new CMseButInputSource(this, mseNum, butNum);
	}

	// If got here, then mouse part is invalid
	return NULL;
}

CInputSource *CInputSystem::CreateJoySource(int joyNum, EJoyPart joyPart)
{
	// Get joystick details and settings
	const JoyDetails *joyDetails = GetJoyDetails(joyNum);
	JoySettings *settings = GetJoySettings(joyNum, true);
	
	// Create source according to given joystick part
	int axisNum;
	int axisDir;
	int povNum;
	int povDir;
	if (GetAxisDetails(joyPart, axisNum, axisDir))
	{
		// Part is joystick axis, so see whether joystick actually has this axis
		if (!joyDetails->hasAxis[axisNum])
			return m_emptySource;  // If joystick doesn't have axis, then return empty source rather than NULL as not really an error
		// Otherwise, create axis source with appropriate axis range, deadzone and saturation settings
		return new CJoyAxisInputSource(this, joyNum, axisNum, axisDir, 
			settings->axisMinVals[axisNum], settings->axisOffVals[axisNum], settings->axisMaxVals[axisNum],
			settings->deadZones[axisNum], settings->saturations[axisNum]);
	}
	else if (GetPOVDetails(joyPart, povNum, povDir))
	{
		// Part is joystick POV hat controller so see whether joystick has this POV
		if (povNum >= joyDetails->numPOVs)
			return m_emptySource;  // If joystick doesn't have POV, then return empty source rather than NULL as not really an error
		return new CJoyPOVInputSource(this, joyNum, povNum, povDir);
	}
	else if (IsButton(joyPart))
	{	
		// Part is joystick button so map it to a button number
		int butNum = GetButtonNumber(joyPart);
		if (butNum < 0 || butNum >= NUM_JOY_BUTTONS)
			return NULL;  // Buttons out of range are invalid
		if (butNum >= joyDetails->numButtons)
			return m_emptySource;  // If joystick doesn't have button, then return empty source rather than NULL as not really an error
		return new CJoyButInputSource(this, joyNum, butNum);
	}
	
	// If got here, then joystick part is invalid
	return NULL;
}

bool CInputSystem::Initialize()
{
	// Initialize subclass
	if (!InitializeSystem())
		return false;

	// Get number of keyboard, mice and joysticks (they are stored here as need to access the values in the destructor)
	m_numKbds = GetNumKeyboards();
	m_numMice = GetNumMice();
	m_numJoys = GetNumJoysticks();

	// Create cache to hold input sources
	CreateSourceCache();

	GrabMouse();
	return true;
}

void CInputSystem::SetDisplayGeom(unsigned dispX, unsigned dispY, unsigned dispW, unsigned dispH)
{
	// Remember display geometry
	m_dispX = dispX;
	m_dispY = dispY;
	m_dispW = dispW;
	m_dispH = dispH;
}

CInputSource* CInputSystem::ParseSource(const char *mapping, bool fullAxisOnly)
{
	return ParseMultiSource(mapping, fullAxisOnly, true);
}

void CInputSystem::ClearSettings()
{
	// Delete all key settings
	for (vector<KeySettings*>::iterator it = m_keySettings.begin(); it != m_keySettings.end(); it++)
		delete *it;
	m_keySettings.clear();

	// Delete all mouse settings
	for (vector<MouseSettings*>::iterator it = m_mseSettings.begin(); it != m_mseSettings.end(); it++)
		delete *it;
	m_mseSettings.clear();

	// Delete all joystick settings
	for (vector<JoySettings*>::iterator it = m_joySettings.begin(); it != m_joySettings.end(); it++)
		delete *it;
	m_joySettings.clear();
}

void CInputSystem::PrintSettings()
{
	puts("Input System Settings");
	puts("---------------------");
	puts("");

	printf("Input System: %s\n", name);

	puts("");

	PrintDevices();

	puts("");

	// Print all key settings for attached keyboards
	KeySettings *keySettings;
	if (m_numKbds == ANY_KEYBOARD)
	{
		puts("Common Keyboard Settings:");
		keySettings = GetKeySettings(ANY_KEYBOARD, true);
		PrintKeySettings(ANY_KEYBOARD, keySettings);
	}
	else
	{
		for (int kbdNum = 0; kbdNum < m_numKbds; kbdNum++)
		{
			printf("Keyboard %d Settings:\n", kbdNum + 1);
			keySettings = GetKeySettings(kbdNum, true);
			PrintKeySettings(kbdNum, keySettings);
		}
	}

	// Print all mouse settings for attached mice
	MouseSettings *mseSettings;
	if (m_numMice == ANY_MOUSE)
	{
		puts("Common Mouse Settings:");
		mseSettings = GetMouseSettings(ANY_MOUSE, true);
		PrintMouseSettings(ANY_MOUSE, mseSettings);
	}
	else
	{
		for (int mseNum = 0; mseNum < m_numMice; mseNum++)
		{
			printf("Mouse %d Settings:\n", mseNum + 1);
			mseSettings = GetMouseSettings(mseNum, true);
			PrintMouseSettings(mseNum, mseSettings);
		}
	}

	// Print all joystick settings for attached joysticks
	JoySettings *joySettings;
	if (m_numJoys == ANY_JOYSTICK)
	{
		puts("Common Joystick Settings:");
		joySettings = GetJoySettings(ANY_JOYSTICK, true);
		PrintJoySettings(ANY_JOYSTICK, joySettings);
	}
	else
	{
		for (int joyNum = 0; joyNum < m_numJoys; joyNum++)
		{
			printf("Joystick %d Settings:\n", joyNum + 1);
			joySettings = GetJoySettings(joyNum, true);
			PrintJoySettings(joyNum, joySettings);
		}
	}

	puts("");
}

void CInputSystem::ReadFromINIFile(CINIFile *ini, const char *section)
{
	ClearSettings();
	ClearSourceCache();

	// Read all key settings for attached keyboards
	KeySettings *keySettings = ReadKeySettings(ini, section, ANY_KEYBOARD);
	if (keySettings != NULL)
		m_keySettings.push_back(keySettings);
	for (int kbdNum = 0; kbdNum < m_numKbds; kbdNum++)
	{
		keySettings = ReadKeySettings(ini, section, kbdNum);
		if (keySettings != NULL)
			m_keySettings.push_back(keySettings);
	}

	// Read all mouse settings for attached mice
	MouseSettings *mseSettings = ReadMouseSettings(ini, section, ANY_MOUSE);
	if (mseSettings != NULL)
		m_mseSettings.push_back(mseSettings);
	for (int mseNum = 0; mseNum < m_numMice; mseNum++)
	{
		mseSettings = ReadMouseSettings(ini, section, mseNum);
		if (mseSettings != NULL)
			m_mseSettings.push_back(mseSettings);
	}

	// Read all joystick settings for attached joysticks
	JoySettings *joySettings = ReadJoySettings(ini, section, ANY_JOYSTICK);
	if (joySettings != NULL)
		m_joySettings.push_back(joySettings);
	for (int joyNum = 0; joyNum < m_numJoys; joyNum++)
	{
		joySettings = ReadJoySettings(ini, section, joyNum);
		if (joySettings != NULL)
			m_joySettings.push_back(joySettings);
	}
}

void CInputSystem::WriteToINIFile(CINIFile *ini, const char *section)
{
	// Write all key settings
	for (vector<KeySettings*>::iterator it = m_keySettings.begin(); it != m_keySettings.end(); it++)
		WriteKeySettings(ini, section, *it);

	// Write all mouse settings
	for (vector<MouseSettings*>::iterator it = m_mseSettings.begin(); it != m_mseSettings.end(); it++)
		WriteMouseSettings(ini, section, *it);

	// Write all joystick settings
	for (vector<JoySettings*>::iterator it = m_joySettings.begin(); it != m_joySettings.end(); it++)
		WriteJoySettings(ini, section, *it);
}

bool CInputSystem::ReadMapping(char *buffer, unsigned bufSize, bool fullAxisOnly, unsigned readFlags, const char *escapeMapping)
{
	// Map given escape mapping to an input source
	bool cancelled = false;
	CInputSource *escape = ParseSource(escapeMapping);
	if (escape)
		escape->Acquire();
	
	string badMapping;
	string mapping;
	vector<CInputSource*> badSources;
	vector<CInputSource*> sources;
	bool mseCentered = false;
	
	// See which sources activated to begin with and from here on ignore these (this stops badly calibrated axes that are constantly "active"
	// from preventing the user from exiting read loop)
	if (!Poll())
		goto Cancelled;

	CheckAllSources(readFlags, fullAxisOnly, mseCentered, badSources, badMapping, sources);

	// Loop until have received meaningful inputs
	for (;;)
	{
		// Poll inputs
		if (!Poll())
			goto Cancelled;

		// Check if escape source was triggered
		if (escape && escape->IsActive())
		{
			// If so, wait until source no longer active and then exit
			while (escape->IsActive())
			{
				if (!Poll())
					goto Cancelled;
				Wait(1000/60);
			}
			goto Cancelled;
		}

		// Check all active sources
		CheckAllSources(readFlags, fullAxisOnly, mseCentered, sources, mapping, badSources);

		// When some inputs have been activated, keep looping until they have all been released again.
		if (sources.size() > 0)
		{
			// Check each source is no longer active
			bool active = false;
			for (vector<CInputSource*>::iterator it = sources.begin(); it != sources.end(); it++)
			{
				if ((*it)->IsActive())
				{
					active = true;
					break;
				}
			}
			if (!active)
			{
				// If so, get combined type of sources and if is valid then return
				ESourceType type = CMultiInputSource::GetCombinedType(sources);
				if (type != SourceInvalid && (type == SourceFullAxis && fullAxisOnly || type != SourceFullAxis && !fullAxisOnly))
					break;

				mapping.clear();
				sources.clear();
				mseCentered = false;
			}	
		}

		// Don't poll continuously
		Wait(1000/60);
	}

	// Copy mapping to buffer and return
	strncpy(buffer, mapping.c_str(), bufSize - 1);
	buffer[bufSize - 1] = '\0';
	goto Finish;

Cancelled:
	cancelled = true;

Finish:
	if (escape)
		escape->Release();
	return !cancelled;
}

void CInputSystem::GrabMouse()
{
	m_grabMouse = true;
}

void CInputSystem::UngrabMouse()
{
	m_grabMouse = false;

	// Make sure mouse is visible
	SetMouseVisibility(true);
}

bool CInputSystem::SendForceFeedbackCmd(int joyNum, int axisNum, ForceFeedbackCmd ffCmd)
{
	const JoyDetails *joyDetails = GetJoyDetails(joyNum);
	if (!joyDetails->hasFFeedback || !joyDetails->axisHasFF[axisNum])
		return false;
	return ProcessForceFeedbackCmd(joyNum, axisNum, ffCmd);
}

bool CInputSystem::DetectJoystickAxis(unsigned joyNum, unsigned &axisNum, const char *escapeMapping, const char *confirmMapping)
{
	const JoyDetails *joyDetails = GetJoyDetails(joyNum);
	if (joyDetails == NULL)
	{
		puts("No such joystick!");
		return false;
	}

	// Map given escape & confirm mappings to input sources
	bool cancelled = false;
	CInputSource *escape = ParseSource(escapeMapping);
	CInputSource *confirm = ParseSource(confirmMapping);
	if (escape)
		escape->Acquire();
	if (confirm)
		confirm->Acquire();

	printf("Move axis around and then press Return (or press Esc to cancel): ");
	fflush(stdout);	// required on terminals that use buffering

	unsigned maxRange;
	unsigned maxAxisNum;

	int minVals[NUM_JOY_AXES];
	int maxVals[NUM_JOY_AXES];
	for (unsigned loopAxisNum = 0; loopAxisNum < NUM_JOY_AXES; loopAxisNum++)
	{
		if (!joyDetails->hasAxis[loopAxisNum])
			continue;
		int joyVal = GetJoyAxisValue(joyNum, loopAxisNum);
		minVals[loopAxisNum] = joyVal;
		maxVals[loopAxisNum] = joyVal;
	}
	for (;;)
	{	
		if (!Poll())
			goto Cancelled;
		
		// Check if escape source was triggered
		if (escape && escape->IsActive())
		{
			// If so, wait until source no longer active and then exit
			while (escape->IsActive())
			{
				if (!Poll())
					goto Cancelled;
				Wait(1000/60);
			}
			goto Cancelled;
		}

		// Check if confirm source was triggered
		if (confirm && confirm->IsActive())
		{
			// If so, wait until source no longer active and then exit
			while (confirm->IsActive())
			{
				if (!Poll())
					goto Cancelled;
				Wait(1000/60);
			}
			break;
		}

		for (unsigned loopAxisNum = 0; loopAxisNum < NUM_JOY_AXES; loopAxisNum++)
		{
			if (!joyDetails->hasAxis[loopAxisNum])
				continue;
			int joyVal = GetJoyAxisValue(joyNum, loopAxisNum);
			minVals[loopAxisNum] = min<int>(joyVal, minVals[loopAxisNum]);
			maxVals[loopAxisNum] = max<int>(joyVal, maxVals[loopAxisNum]);
		}

		// Don't poll continuously
		Wait(1000/60);
	}

	maxRange = 0;
	maxAxisNum = 0;
	for (unsigned loopAxisNum = 0; loopAxisNum < NUM_JOY_AXES; loopAxisNum++)
	{
		if (!joyDetails->hasAxis[loopAxisNum])
			continue;
		unsigned range = maxVals[loopAxisNum] - minVals[loopAxisNum];
		if (range > maxRange)
		{
			maxRange = range;
			axisNum = loopAxisNum;
		}
	}

	if (maxRange > 3000)
		printf("Detected\n", joyDetails->axisName[axisNum]);
	else
	{
		cancelled = true;
		puts("Not Detected");
	}
	goto Finish;	
	
Cancelled:
	puts("[Cancelled]");
	cancelled = true;

Finish:
	if (escape)
		escape->Release();
	if (confirm)
		confirm->Release();
	return !cancelled;
}

bool CInputSystem::CalibrateJoystickAxis(unsigned joyNum, unsigned axisNum, const char *escapeMapping, const char *confirmMapping)
{
	const JoyDetails *joyDetails = GetJoyDetails(joyNum);
	if (joyDetails == NULL || axisNum >= NUM_JOY_AXES || !joyDetails->hasAxis[axisNum])
	{
		puts("No such axis or joystick!");
		return false;
	}

	// Map given escape mapping to input source
	bool cancelled = false;
	CInputSource *escape = ParseSource(escapeMapping);
	CInputSource *output = ParseSource("KEY_SHIFT");
	if (escape)
		escape->Acquire();
	if (output)
		output->Acquire();

Repeat:
	printf("Calibrating %s of joystick '%s'.\n\n", joyDetails->axisName[axisNum], joyDetails->name);
	
	unsigned totalRange;
	unsigned posDeadZone;
	unsigned negDeadZone;
	unsigned deadZone;
			
	int posVal;
	int negVal;
	int offVal;
	unsigned posRange;
	unsigned negRange;
	unsigned posOffRange;
	unsigned negOffRange;
	char mapping[50];
	for (unsigned step = 0; step < 3; step++)
	{
		switch (step)
		{
			case 0: 
				puts("Step 1:");
				puts(" Move axis now to its furthest positive/'on' position and hold, ie:");
				if (axisNum == AXIS_X || axisNum == AXIS_RX || axisNum == AXIS_Z || axisNum == AXIS_RZ)
					puts(" - for a horizontal joystick axis, push it all the way to the right.");
				if (axisNum == AXIS_Y || axisNum == AXIS_RY || axisNum == AXIS_Z || axisNum == AXIS_RZ)
					puts(" - for a vertical joystick axis, push it all the way downwards.");
				puts(" - for a steering wheel, turn it all the way to the right.");
				puts(" - for a pedal, press it all the way to the floor.");
				break;
			case 1: 
				puts("Step 2:");
				puts(" Move axis the other way to its furthest negative position and hold, ie:");
				if (axisNum == AXIS_X || axisNum == AXIS_RX || axisNum == AXIS_Z || axisNum == AXIS_RZ)
					puts(" - for a horizontal joystick axis, push it all the way to the left.");
				if (axisNum == AXIS_Y || axisNum == AXIS_RY || axisNum == AXIS_Z || axisNum == AXIS_RZ)
					puts(" - for a vertical joystick axis, push it all the way updwards.");
				puts(" - for a steering wheel, turn it all the way to the left.");
				puts(" - for a pedal, let go of the pedal completely.  If there is another pedal");
				puts("   that shares the same axis then press that one all the way to the floor.");
				break;
			case 2: 
				puts("Step 3:");
				puts(" Return axis to its central/'off' position and hold, ie:");
				puts(" - for a joystick axis, let it return to the middle.");
				puts(" - for a steering weel, turn it back to the center.");
				puts(" - for a pedal, let go of pedal completely.  Likewise for any other pedal");
				puts("   that shares the same axis.");
				break;
		}
		printf("\nPress Return when done (or press Esc to cancel): ");
		fflush(stdout);	// required on terminals that use buffering

		// Loop until user confirms or aborts
		for (;;)
		{
			if (!ReadMapping(mapping, 50, false, READ_KEYBOARD|READ_MERGE, escapeMapping))
				goto Cancelled;
			if (stricmp(mapping, confirmMapping) == 0)
				break;
		}

		printf("Calibrating... ");
		fflush(stdout);	// required on terminals that use buffering

		// Loop until at least three seconds have elapsed or user aborts
		int joyVal = GetJoyAxisValue(joyNum, axisNum);
		int minVal = joyVal;
		int maxVal = joyVal;
		bool firstOut = true;
		for (unsigned frames = 0; frames < 3 * 60; frames++)
		{	
			if (!Poll())
				goto Cancelled;

			// Check if escape source was triggered
			if (escape && escape->IsActive())
			{
				// If so, wait until source no longer active and then exit
				while (escape->IsActive())
				{
					if (!Poll())
						goto Cancelled;
					Wait(1000/60);
				}
				goto Cancelled;
			}

			joyVal = GetJoyAxisValue(joyNum, axisNum);
			minVal = min<int>(minVal, joyVal);
			maxVal = max<int>(maxVal, joyVal);

			// Check if output source is triggered, and if so output value for debugging
			if (output && output->IsActive())
			{
				if (firstOut)
					puts("");
				printf(" [value: %d, min: %d, %max: %d]\n", joyVal, minVal, maxVal);
				firstOut = false;
			}

			// Don't poll continuously
			Wait(1000/60);
		}

		printf("Done\n\n");

		switch (step)
		{
			case 0:	posVal = (abs(maxVal) >= abs(minVal) ? maxVal : minVal); break;
			case 1: negVal = (abs(minVal) >= abs(maxVal) ? minVal : maxVal); break;
			case 2: 
				if (minVal <= 0 && maxVal >= 0)
					offVal = 0;
				else if (minVal == DEFAULT_JOY_AXISMINVAL)
					offVal = DEFAULT_JOY_AXISMINVAL;
				else if (maxVal == DEFAULT_JOY_AXISMAXVAL)
					offVal = DEFAULT_JOY_AXISMAXVAL;
				else
					offVal = (minVal + maxVal) / 2;
				posRange = abs(posVal - offVal);
				negRange = abs(negVal - offVal);
				posOffRange = (unsigned)(posVal > offVal ? maxVal - offVal : offVal - minVal);
				negOffRange = (unsigned)(posVal > offVal ? offVal - minVal : maxVal - offVal);
				break;
		}
	}
	
	totalRange = posRange + negRange;
	posDeadZone = (unsigned)ceil(100.0 * (double)posOffRange / (double)posRange);
	negDeadZone = (unsigned)ceil(100.0 * (double)negOffRange / (double)negRange);
	deadZone = max<unsigned>(1, max<unsigned>(negDeadZone, posDeadZone));
		
	bool okay;
	if (posVal > negVal)
		okay = negVal <= offVal && offVal <= posVal && totalRange > 3000 && deadZone < 90;
	else
		okay = posVal <= offVal && offVal <= negVal && totalRange > 3000 && deadZone < 90;
	if (okay)
	{
		JoySettings *commonSettings = GetJoySettings(ANY_JOYSTICK, true);
		JoySettings *joySettings = GetJoySettings(joyNum, false);
		if (joySettings == NULL)
		{
			joySettings = new JoySettings(*commonSettings);
			m_joySettings.push_back(joySettings);
			joySettings->joyNum = joyNum;
		}

		printf("Calibrated Axis Settings:\n\n");
		printf(" Min Value        = %d\n", negVal);
		printf(" Center/Off Value = %d\n", offVal);
		printf(" Max Value        = %d\n", posVal);
		printf(" Dead Zone        = %d %%\n", deadZone);
		printf("\nAccept these settings: y/n? ");
		fflush(stdout);	// required on terminals that use buffering
		
		// Loop until user confirms or declines
		while (ReadMapping(mapping, 50, false, READ_KEYBOARD|READ_MERGE, escapeMapping))
		{
			if (stricmp(mapping, "KEY_N") == 0)
				break;
			else if (stricmp(mapping, "KEY_Y") == 0)
			{
				joySettings->axisMinVals[axisNum] = negVal;
				joySettings->axisMaxVals[axisNum] = posVal;
				joySettings->axisOffVals[axisNum] = offVal;
				joySettings->deadZones[axisNum] = deadZone;
				
				ClearSourceCache();

				puts("Accepted");
				goto Finish;		
			}
		}
		goto Cancelled;
	}
	else
	{
		puts("There was a problem calibrating the axis.  This may be because the steps");
		puts("were not followed correctly or the joystick is sending invalid data.");
		printf("\nTry calibrating again: y/n? ");
		fflush(stdout);	// required on terminals that use buffering
		
		// Loop until user confirms or declines
		while (ReadMapping(mapping, 50, false, READ_KEYBOARD|READ_MERGE, escapeMapping))
		{
			if (stricmp(mapping, "KEY_N") == 0)
				break;
			else if (stricmp(mapping, "KEY_Y") == 0)
			{
				puts("[Cancelled]");
				goto Repeat;
			}
		}
		goto Cancelled;
	}
	
Cancelled:
	puts("[Cancelled]");
	cancelled = true;

Finish:
	if (escape)
		escape->Release();
	if (output)
		output->Release();
	return !cancelled;
}

void CInputSystem::PrintDevices()
{
	puts("Keyboards:");
	if (m_numKbds == 0)
		puts(" None");
	else if (m_numKbds == ANY_KEYBOARD)
		puts(" System Keyboard");
	else
	{
		for (int kbdNum = 0; kbdNum < m_numKbds; kbdNum++)
		{
			const KeyDetails *keyDetails = GetKeyDetails(kbdNum);
			printf(" %d: %s\n", kbdNum + 1, keyDetails->name);
		}
	}

	puts("Mice:");
	if (m_numMice == 0)
		puts(" None");
	else if (m_numMice == ANY_MOUSE)
		puts(" System Mouse");
	else
	{
		for (int mseNum = 0; mseNum < m_numMice; mseNum++)
		{
			const MouseDetails *mseDetails = GetMouseDetails(mseNum);
			printf(" %d: %s\n", mseNum + 1, mseDetails->name);
		}
	}

	puts("Joysticks:");
	if (m_numJoys == 0)
		puts(" None");
	else if (m_numJoys == ANY_JOYSTICK)
		puts(" System Joystick");
	else
	{
		for (int joyNum = 0; joyNum < m_numJoys; joyNum++)
		{
			const JoyDetails *joyDetails = GetJoyDetails(joyNum);
			if (joyDetails->hasFFeedback)
				printf(" %d: %s [Force Feedback Available]\n", joyNum + 1, joyDetails->name);
			else
				printf(" %d: %s\n", joyNum + 1, joyDetails->name);
		}
	}
}

/*
 * CInputSystem::CKeyInputSource
 */
CInputSystem::CKeyInputSource::CKeyInputSource(CInputSystem *system, int kbdNum, int keyIndex, unsigned sensitivity, unsigned decaySpeed) : 
	CInputSource(SourceSwitch), m_system(system), m_kbdNum(kbdNum), m_keyIndex(keyIndex), m_val(0) 
{
	// Calculate max value and incr and decr values (sensitivity is given as percentage 1-100, with 100 being most sensitive, and
	// decay speed given as percentage 1-200 of attack speed)
	int s = Clamp((int)sensitivity, 1, 100);
	int d = Clamp((int)decaySpeed, 1, 200);
	m_incr = 100 * s;
	m_decr = d * s;
	m_maxVal = 10000;
}

bool CInputSystem::CKeyInputSource::GetValueAsSwitch(bool &val)
{
	if (!m_system->IsKeyPressed(m_kbdNum, m_keyIndex))
		return false;
	val = true;
	return true;
}

bool CInputSystem::CKeyInputSource::GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal)
{
	if (m_system->IsKeyPressed(m_kbdNum, m_keyIndex))
		m_val = min<int>(m_maxVal, m_val + m_incr);
	else
		m_val = max<int>(0, m_val - m_decr);
	if (m_val == 0)
		return false;
	val = Scale(m_val, 0, 0, m_maxVal, minVal, offVal, maxVal);
	return true;
}

/*
 * CInputSystem::CMseAxisInputSource
 */
CInputSystem::CMseAxisInputSource::CMseAxisInputSource(CInputSystem *system, int mseNum, int axisNum, int axisDir, unsigned deadZone) : 
	CInputSource(axisDir == AXIS_FULL || axisDir == AXIS_INVERTED ? SourceFullAxis : SourceHalfAxis), 
	m_system(system), m_mseNum(mseNum), m_axisNum(axisNum), m_axisDir(axisDir)
{
	// If X- or Y-axis then calculate size of dead pixels region in centre of display (deadzone is given as a percentage 0-99)
	if (m_axisNum == AXIS_X || m_axisNum == AXIS_Y)
	{
		double dDeadZone = (double)Clamp((int)deadZone, 0, 99) / 100.0;
		m_deadPixels = (int)(dDeadZone * (double)(m_axisNum == AXIS_X ? m_system->m_dispW : m_system->m_dispH));
	}
	else
		m_deadPixels = Clamp((int)deadZone, 0, 99);
}

int CInputSystem::CMseAxisInputSource::ScaleAxisValue(int minVal, int offVal, int maxVal)
{
	int mseVal = m_system->GetMouseAxisValue(m_mseNum, m_axisNum);
	// If X- or Y-axis then convert to value centered around zero (ie relative to centre of display)
	int mseMin, mseMax;
	if (m_axisNum == AXIS_X || m_axisNum == AXIS_Y)
	{
		int dispExtent = (int)(m_axisNum == AXIS_X ? m_system->m_dispW : m_system->m_dispH);
		if (dispExtent == 0)
			return offVal;
		mseMin = -dispExtent / 2;
		mseMax = mseMin + dispExtent - 1;
		mseVal = (m_axisNum == AXIS_X ? mseVal - m_system->m_dispX : mseVal - m_system->m_dispY) - dispExtent / 2;
	}
	else
	{
		// Z-axis (wheel) is always between -100 and 100
		mseMin = -100;
		mseMax = 100;
	}
	// Check value is not zero
	if (mseVal == 0)
		return offVal;
	// Scale values from dead zone to display edge, taking positive or negative values only or using the whole axis range as required
	int dZone = (mseVal > 0 ? m_deadPixels / 2 : m_deadPixels - m_deadPixels / 2);
	if      (m_axisDir == AXIS_POS)  return Scale(mseVal, dZone, dZone, mseMax, minVal, offVal, maxVal);
	else if (m_axisDir == AXIS_NEG)  return Scale(mseVal, -dZone, -dZone, mseMin, minVal, offVal, maxVal);
	else if (m_axisDir == AXIS_FULL)
	{
		// Full axis range
		if (mseVal > 0) return Scale(mseVal, dZone, dZone, mseMax, minVal, offVal, maxVal);
		else            return Scale(mseVal, mseMin, -dZone, -dZone, minVal, offVal, maxVal);
	}
	else
	{
		// Full axis range, but inverted
		if (mseVal > 0) return Scale(mseVal, dZone, dZone, mseMax, maxVal, offVal, minVal);
		else            return Scale(mseVal, mseMin, -dZone, -dZone, maxVal, offVal, minVal);
	}
}

bool CInputSystem::CMseAxisInputSource::GetValueAsSwitch(bool &val)
{
	// For Z-axis (wheel), switch value is handled slightly differently
	if (m_axisNum == AXIS_Z)
	{
		int wheelDir = m_system->GetMouseWheelDir(m_mseNum);
		if ((m_axisDir == AXIS_POS || m_axisDir == AXIS_FULL)     && wheelDir <= 0 ||
			(m_axisDir == AXIS_NEG || m_axisDir == AXIS_INVERTED) && wheelDir >= 0)
			return false;
	}
	else
	{
		if (ScaleAxisValue(0, 0, 3) < 2)
			return false;
	}
	val = true;
	return true;
}

bool CInputSystem::CMseAxisInputSource::GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal)
{
	int axisVal = ScaleAxisValue(minVal, offVal, maxVal);
	if (axisVal == offVal)
		return false;
	val = axisVal;
	return true;
}

/*
 * CInputSystem::CMseButInputSource
 */
CInputSystem::CMseButInputSource::CMseButInputSource(CInputSystem *system, int mseNum, int butNum) :
	CInputSource(SourceSwitch), m_system(system), m_mseNum(mseNum), m_butNum(butNum)
{
	//
}

bool CInputSystem::CMseButInputSource::GetValueAsSwitch(bool &val)
{
	if (!m_system->IsMouseButPressed(m_mseNum, m_butNum))
		return false;
	val = true;
	return true;
}

bool CInputSystem::CMseButInputSource::GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal)
{
	if (!m_system->IsMouseButPressed(m_mseNum, m_butNum))
		return false;
	val = maxVal;
	return true;
}

/*
 * CInputSystem::CJoyAxisInputSource
 */
CInputSystem::CJoyAxisInputSource::CJoyAxisInputSource(CInputSystem *system, int joyNum, int axisNum, int axisDir,
	int axisMinVal, int axisOffVal, int axisMaxVal, unsigned deadZone, unsigned saturation) : 
	CInputSource(axisDir == AXIS_FULL || axisDir == AXIS_INVERTED ? SourceFullAxis : SourceHalfAxis),
	m_system(system), m_joyNum(joyNum), m_axisNum(axisNum), m_axisDir(axisDir), m_axisMinVal(axisMinVal), m_axisOffVal(axisOffVal), m_axisMaxVal(axisMaxVal)
{
	m_axisInverted = m_axisMaxVal < m_axisMinVal;
	// Calculate pos/neg deadzone and saturation points (joystick raw values range from axisMinVal to axisMasVal (centered/off at axisOffVal),
	// deadzone given as percentage 0-99 and saturation given as percentage 1 - 200)
	double dDeadZone = (double)Clamp((int)deadZone, 0, 99) / 100.0;
	double dSaturation = (double)Clamp((int)saturation, (int)deadZone + 1, 200) / 100.0;
	m_posDZone = m_axisOffVal + (int)(dDeadZone * (m_axisMaxVal - m_axisOffVal));
	m_negDZone = m_axisOffVal + (int)(dDeadZone * (m_axisMinVal - m_axisOffVal));
	m_posSat = m_axisOffVal + (int)(dSaturation * (m_axisMaxVal - m_axisOffVal));
	m_negSat = m_axisOffVal + (int)(dSaturation * (m_axisMinVal - m_axisOffVal));
}

int CInputSystem::CJoyAxisInputSource::ScaleAxisValue(int minVal, int offVal, int maxVal)
{
	// Get raw axis value from input system
	int joyVal = m_system->GetJoyAxisValue(m_joyNum, m_axisNum);
	// Check if value is at axis off value
	if (joyVal == m_axisOffVal)
		return offVal;
	// Scale value between deadzone and saturation points, taking positive or negative values only or using the whole axis range as required
	if      (m_axisDir == AXIS_POS)  return Scale(joyVal, m_posDZone, m_posDZone, m_posSat, minVal, offVal, maxVal);
	else if (m_axisDir == AXIS_NEG)  return Scale(joyVal, m_negDZone, m_negDZone, m_negSat, minVal, offVal, maxVal);
	else if (m_axisDir == AXIS_FULL)
	{
		// Full axis range
		if (!m_axisInverted && joyVal > m_axisOffVal || m_axisInverted && joyVal < m_axisOffVal) 
			return Scale(joyVal, m_posDZone, m_posDZone, m_posSat, minVal, offVal, maxVal);
		else
			return Scale(joyVal, m_negSat, m_negDZone, m_negDZone, minVal, offVal, maxVal);
	}
	else
	{
		// Full axis range, but inverted
		if (!m_axisInverted && joyVal > m_axisOffVal || m_axisInverted && joyVal < m_axisOffVal)
			return Scale(joyVal, m_posDZone, m_posDZone, m_posSat, maxVal, offVal, minVal);
		else
			return Scale(joyVal, m_negSat, m_negDZone, m_negDZone, maxVal, offVal, minVal);
	}
}

bool CInputSystem::CJoyAxisInputSource::GetValueAsSwitch(bool &val)
{
	if (ScaleAxisValue(0, 0, 3) < 2)
		return false;
	val = true;
	return true;
}

bool CInputSystem::CJoyAxisInputSource::GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal)
{
	int axisVal = ScaleAxisValue(minVal, offVal, maxVal);
	if (axisVal == offVal)
		return false;
	val = axisVal;
	return true;
}

bool CInputSystem::CJoyAxisInputSource::SendForceFeedbackCmd(ForceFeedbackCmd ffCmd)
{
	return m_system->SendForceFeedbackCmd(m_joyNum, m_axisNum, ffCmd);
}

/*
 * CInputSystem::CJoyPOVInputSource
 */
CInputSystem::CJoyPOVInputSource::CJoyPOVInputSource(CInputSystem *system, int joyNum, int povNum, int povDir) :
	CInputSource(SourceSwitch), m_system(system), m_joyNum(joyNum), m_povNum(povNum), m_povDir(povDir)
{
	//
}

bool CInputSystem::CJoyPOVInputSource::GetValueAsSwitch(bool &val)
{
	if (!m_system->IsJoyPOVInDir(m_joyNum, m_povNum, m_povDir))
		return false;
	val = true;
	return true;
}

bool CInputSystem::CJoyPOVInputSource::GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal)
{
	if (!m_system->IsJoyPOVInDir(m_joyNum, m_povNum, m_povDir))
		return false;
	val = maxVal;
	return true;
}
	
/*
 * CInputSystem::CJoyButInputSource
 */
CInputSystem::CJoyButInputSource::CJoyButInputSource(CInputSystem *system, int joyNum, int butNum) :
	CInputSource(SourceSwitch), m_system(system), m_joyNum(joyNum), m_butNum(butNum)
{
	//
}

bool CInputSystem::CJoyButInputSource::GetValueAsSwitch(bool &val)
{
	if (!m_system->IsJoyButPressed(m_joyNum, m_butNum))
		return false;
	val = true;
	return true;
}

bool CInputSystem::CJoyButInputSource::GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal)
{
	if (!m_system->IsJoyButPressed(m_joyNum, m_butNum))
		return false;
	val = maxVal;
	return true;
}
