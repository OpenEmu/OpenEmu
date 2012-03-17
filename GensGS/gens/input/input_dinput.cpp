/***************************************************************************
 * Gens: Input Class - DirectInput 5                                       *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#include "input_dinput.hpp"
#include "input_win32_keys.h"

#include "emulator/g_main.hpp"
#include "emulator/g_input.hpp"
#include "ui/gens_ui.hpp"
#include "gens/gens_window.hpp"

#include "controller_config/controller_config_window.hpp"

// Input_DInput being handled by InitJoystick at the moment.
// TODO: This is a REALLY bad hack.
static Input_DInput *DInput_Callback_Handler_Object;

const struct KeyMap keyDefault[8] =
{
	// Player 1
	{DIK_RETURN, DIK_RSHIFT,
	DIK_A, DIK_S, DIK_D,
	DIK_Z, DIK_X, DIK_C,
	DIK_UP, DIK_DOWN, DIK_LEFT, DIK_RIGHT},
	
	// Players 1B, 1C, 1D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	
	// Player 2
	{DIK_U, DIK_T,
	DIK_K, DIK_L, DIK_M,
	DIK_I, DIK_O, DIK_P,
	DIK_Y, DIK_H, DIK_H, DIK_J},
	
	// Players 2B, 2C, 2D
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
	{0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}
};

// Axis values.
static const unsigned char JoyAxisValues[2][6] =
{
	// axis value < -10,000
	{0x03, 0x01, 0x07, 0x05, 0x0B, 0x09},
	
	// axis value > 10,000
	{0x04, 0x02, 0x08, 0x06, 0x0C, 0x0A},
};

#define MOD_NONE 0
#define VK_NONE 0
#define ID_NONE 0


Input_DInput::Input_DInput()
{
	int i;
	HRESULT rval;
	
	// Attempt to initialize DirectInput 5.
	m_diVersion = 0;
	rval = DirectInputCreate(ghInstance, DIRECTINPUT_VERSION_5, &lpDI, NULL);
	if (rval == DI_OK)
	{
		// DirectInput 5 initialized.
		fprintf(stderr, "Input_DInput(): Initialized DirectInput 5.\n");
		m_diVersion = DIRECTINPUT_VERSION_5;
	}
	else
	{
		// Attempt to initialize DirectInput 3.
		rval = DirectInputCreate(ghInstance, DIRECTINPUT_VERSION_3, &lpDI, NULL);
		if (rval == DI_OK)
		{
			// DirectInput 3 initialized.
			fprintf(stderr, "Input_DInput(): Initialized DirectInput 3.\n");
			m_diVersion = DIRECTINPUT_VERSION_3;
		}
		else
		{
			// DirectInput could not be initialized.
			fprintf(stderr, "Input_DInput(): Could not initialize DirectInput 3 or DirectInput 5.\n");
			GensUI::msgBox("Input_DInput(): DirectInputCreate() failed.\n\nYou must have DirectX 3 or later.",
				       "DirectInput Error", GensUI::MSGBOX_ICON_ERROR);
			// TODO: Use cross-platform error numbers, not just DirectInput return values.
			throw rval;
		}
	}
	
	joysticksInitialized = false;
	joystickError = false;
	m_numJoysticks = 0;
	memset(m_joyID, 0x00, sizeof(m_joyID));
	
	//rval = lpDI->CreateDevice(GUID_SysMouse, &lpDIDMouse, NULL);
	lpDIDMouse = NULL;
	rval = lpDI->CreateDevice(GUID_SysKeyboard, &lpDIDKeyboard, NULL);
	if (rval != DI_OK)
	{
		GensUI::msgBox("Input_DInput(): CreateDevice() failed.", "DirectInput Error", GensUI::MSGBOX_ICON_ERROR);
		// TODO: Use cross-platform error numbers, not just DirectInput return values.
		throw rval;
	}
	
	setCooperativeLevel();
	
	//rval = lpDIDMouse->SetDataFormat(&c_dfDIMouse);
	rval = lpDIDKeyboard->SetDataFormat(&c_dfDIKeyboard);
	if (rval != DI_OK)
	{
		GensUI::msgBox("Input_DInput(): SetDataFormat() failed.", "DirectInput Error", GensUI::MSGBOX_ICON_ERROR);
		// TODO: Use cross-platform error numbers, not just DirectInput return values.
		throw rval;
	}
	
	//rval = lpDIDMouse->Acquire();
	for(i = 0; i < 10; i++)
	{
		rval = lpDIDKeyboard->Acquire();
		if (rval == DI_OK)
			break;
		GensUI::sleep(10);
	}
	
	// Clear the DirectInput arrays.
	memset(m_DIKeys, 0x00, sizeof(m_DIKeys));
	memset(m_joyID, 0x00, sizeof(m_joyID));
	memset(m_joyState, 0x00, sizeof(m_joyState));
	
	return;
}


Input_DInput::~Input_DInput()
{
	// If any joysticks were opened, close them.
	int i;
	if (!lpDI)
		return;
	
	if (lpDIDMouse)
	{
		lpDIDMouse->Release();
		lpDIDMouse = NULL;
	}
	
	if (lpDIDKeyboard)
	{
		lpDIDKeyboard->Release();
		lpDIDKeyboard = NULL;
	}
	
	for (i = 0; i < MAX_JOYS; i++)
	{
		if (m_joyID[i])
		{
			m_joyID[i]->Unacquire();
			m_joyID[i]->Release();
			m_joyID[i] = NULL;
		}
	}
	
	m_numJoysticks = 0;
	lpDI->Release();
	lpDI = NULL;
}


/**
 * initJoysticks(): Initialize joysticks.
 * @param hWnd Window handle where the joysticks should be acquired initially.
 */
void Input_DInput::initJoysticks(HWND hWnd)
{
	// TODO: Check if joysticks work with DirectInput 3.
	if (joystickError)
		return;
	
	if (joysticksInitialized)
	{
		// Joysticks are already initialized.
		// Set the cooperative level.
		// TODO: If set to DISCL_FOREGROUND, run setCooperativeLevel_Joysticks().
		// TODO: If set to DISCL_BACKGROUND, don't run setCooperativeLevel_Joysticks().
		// Currently hard-coded for DISCL_BACKGROUND.
		//setCooperativeLevel_Joysticks(hWnd);
		return;
	}
	
	// Joysticks are being initialized.
	joysticksInitialized = true;
	DInput_Callback_Handler_Object = this;
	
	HRESULT rval;
	rval = lpDI->EnumDevices(DIDEVTYPE_JOYSTICK, &EnumDevices_Joysticks, hWnd, DIEDFL_ATTACHEDONLY);
	if (rval != DI_OK)
	{
		joystickError = true;
		GensUI::msgBox("Input_DInput::initJoysticks(): EnumDevices() failed.", "DirectInput Error", GensUI::MSGBOX_ICON_ERROR);
		// TODO: Error handling.
		return;
	}
}


BOOL CALLBACK Input_DInput::EnumDevices_Joysticks(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef)
{
	return DInput_Callback_Handler_Object->EnumDevices_Joysticks_int(lpDIIJoy, pvRef);
}


BOOL Input_DInput::EnumDevices_Joysticks_int(LPCDIDEVICEINSTANCE lpDIIJoy, LPVOID pvRef)
{
	HRESULT rval;
	LPDIRECTINPUTDEVICE	lpDIJoy;
	DIPROPRANGE diprg;
	int i;
 
	if (m_numJoysticks >= MAX_JOYS)
		return DIENUM_STOP;
		
	m_joyID[m_numJoysticks] = NULL;
	
	rval = lpDI->CreateDevice(lpDIIJoy->guidInstance, &lpDIJoy, NULL);
	if (rval != DI_OK)
	{
		GensUI::msgBox("IDirectInput::CreateDevice() FAILED", "Joystick Error", GensUI::MSGBOX_ICON_ERROR);
		return(DIENUM_CONTINUE);
	}
	
	rval = lpDIJoy->QueryInterface(IID_IDirectInputDevice2, (void **)&m_joyID[m_numJoysticks]);
	lpDIJoy->Release();
	if (rval != DI_OK)
	{
		GensUI::msgBox("IDirectInputDevice2::QueryInterface() FAILED", "Joystick Error", GensUI::MSGBOX_ICON_ERROR);
		m_joyID[m_numJoysticks] = NULL;
		return(DIENUM_CONTINUE);
	}
	
	rval = m_joyID[m_numJoysticks]->SetDataFormat(&c_dfDIJoystick);
	if (rval != DI_OK)
	{
		GensUI::msgBox("IDirectInputDevice::SetDataFormat() FAILED", "Joystick Error", GensUI::MSGBOX_ICON_ERROR);
		m_joyID[m_numJoysticks]->Release();
		m_joyID[m_numJoysticks] = NULL;
		return(DIENUM_CONTINUE);
	}
	
	// TODO: Add an option to specify DISCL_FOREGROUND so the joysticks only work when the Gens window is active.
	rval = m_joyID[m_numJoysticks]->SetCooperativeLevel((HWND)pvRef, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
	
	if (rval != DI_OK)
	{
		GensUI::msgBox("IDirectInputDevice::SetCooperativeLevel() FAILED", "Joystick Error", GensUI::MSGBOX_ICON_ERROR);
		m_joyID[m_numJoysticks]->Release();
		m_joyID[m_numJoysticks] = NULL;
		return(DIENUM_CONTINUE);
	}
	
	diprg.diph.dwSize = sizeof(diprg); 
	diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
	diprg.diph.dwObj = DIJOFS_X;
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.lMin = -1000; 
	diprg.lMax = +1000;
 
	rval = m_joyID[m_numJoysticks]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if ((rval != DI_OK) && (rval != DI_PROPNOEFFECT))
	{
		GensUI::msgBox("IDirectInputDevice::SetProperty() (X-Axis) FAILED", "Joystick Error", GensUI::MSGBOX_ICON_ERROR);
	}
	
	diprg.diph.dwSize = sizeof(diprg); 
	diprg.diph.dwHeaderSize = sizeof(diprg.diph); 
	diprg.diph.dwObj = DIJOFS_Y;
	diprg.diph.dwHow = DIPH_BYOFFSET;
	diprg.lMin = -1000; 
	diprg.lMax = +1000;
 
	rval = m_joyID[m_numJoysticks]->SetProperty(DIPROP_RANGE, &diprg.diph);
	if ((rval != DI_OK) && (rval != DI_PROPNOEFFECT))
	{
		GensUI::msgBox("IDirectInputDevice::SetProperty() (Y-Axis) FAILED", "Joystick Error", GensUI::MSGBOX_ICON_ERROR);
	}
	
	for(i = 0; i < 10; i++)
	{
		rval = m_joyID[m_numJoysticks]->Acquire();
		if (rval == DI_OK)
			break;
		GensUI::sleep(10);
	}
	
	m_numJoysticks++;
	
	return(DIENUM_CONTINUE);
}


void Input_DInput::restoreInput(void)
{
	//lpDIDMouse->Acquire();
	lpDIDKeyboard->Acquire();
}


/**
 * joyExists(): Check if the specified joystick exists.
 * @param joyNum Joystick number.
 * @return true if the joystick exists; false if it does not exist.
 */
bool Input_DInput::joyExists(int joyNum)
{
	if (joyNum < 0 || joyNum >= MAX_JOYS)
		return false;
	
	if (m_joyID[joyNum])
		return true;
	
	// Joystick does not exist.
	return false;
}


/**
 * getKey(): Get a key. (Used for controller configuration.)
 * @return Key value.
 */
unsigned int Input_DInput::getKey(void)
{
	int i, j, joyIndex;
	
	bool prevReady = false;
	
	BOOL prevDiKeys[256];
	BOOL prevJoyKeys[256];
	
	BOOL curDiKeys[256];
	BOOL curJoyKeys[256];
	
	while (true)
	{
		// Compute the current state of all buttons.
		update();
		
		// Current state of DirectInput keys
		for (i = 0; i < 256; i++)
			curDiKeys[i] = (m_DIKeys[i] & 0x80);
		
		// Current state of recognized buttons on joypad
		joyIndex = 0;
		for (i = 0; i < m_numJoysticks; i++)
		{
			if (!joyExists(i))
				continue;
			
			curJoyKeys[joyIndex++] = (m_joyState[i].lY < -500);
			curJoyKeys[joyIndex++] = (m_joyState[i].lY > +500);
			curJoyKeys[joyIndex++] = (m_joyState[i].lX < -500);
			curJoyKeys[joyIndex++] = (m_joyState[i].lX > +500);
			
			for (j = 0; j < 4; j++)
			{
				curJoyKeys[joyIndex++] = (m_joyState[i].rgdwPOV[j] == 0);
				curJoyKeys[joyIndex++] = (m_joyState[i].rgdwPOV[j] == 9000);
				curJoyKeys[joyIndex++] = (m_joyState[i].rgdwPOV[j] == 18000);
				curJoyKeys[joyIndex++] = (m_joyState[i].rgdwPOV[j] == 27000);
			}
			
			for (j = 0; j < 32; j++)
			{
				curJoyKeys[joyIndex++] = (m_joyState[i].rgbButtons[j]);
			}
			
			curJoyKeys[joyIndex++] = (m_joyState[i].lRx < 0x3FFF);
			curJoyKeys[joyIndex++] = (m_joyState[i].lRx > 0xBFFF);
			curJoyKeys[joyIndex++] = (m_joyState[i].lRy < 0x3FFF);
			curJoyKeys[joyIndex++] = (m_joyState[i].lRy > 0xBFFF);
			curJoyKeys[joyIndex++] = (m_joyState[i].lZ < 0x3FFF);
			curJoyKeys[joyIndex++] = (m_joyState[i].lZ > 0xBFFF);
		}
		
		// Compare buttons against the previous state to determine
		// what is pressed now that wasn't already pressed before.
		
		if (prevReady)
		{
			// Check for new DirectInput key presses.
			for (i = 1; i < 255; i++)
			{
				if (curDiKeys[i] && !prevDiKeys[i])
					return i;
			}
			
			// Check for new recognized joypad button presses.
			for (int index = 0; index < joyIndex; index++)
			{
				if (curJoyKeys[index] && !prevJoyKeys[index])
				{
					int joyIndex2 = 0;
					for (i = 0; i < m_numJoysticks; i++)
					{
						if (!joyExists(i))
							continue;
						
						if (index == joyIndex2++)
						{
							return (0x1000 + (0x100 * i) + 0x1);
						}
						if (index == joyIndex2++)
						{
							return (0x1000 + (0x100 * i) + 0x2);
						}
						if (index == joyIndex2++)
						{
							return (0x1000 + (0x100 * i) + 0x3);
						}
						if (index == joyIndex2++)
						{
							return (0x1000 + (0x100 * i) + 0x4);
						}
						
						for (j = 0; j < 4; j++)
						{
							if (index == joyIndex2++)
							{
								return (0x1080 + (0x100 * i) + (0x10 * j) + 0x1);
							}
							if (index == joyIndex2++)
							{
								return (0x1080 + (0x100 * i) + (0x10 * j) + 0x2);
							}
							if (index == joyIndex2++)
							{
								return (0x1080 + (0x100 * i) + (0x10 * j) + 0x3);
							}
							if (index == joyIndex2++)
							{
								return (0x1080 + (0x100 * i) + (0x10 * j) + 0x4);
							}
						}
						
						for (j = 0; j < 32; j++)
						{
							if (index == joyIndex2++)
							{
								return (0x1010 + (0x100 * i) + j);
							}
						}
						
						if (index == joyIndex2++)
						{
							return (0x1000 + (0x100 * i) + 0x5);
						}
						if (index == joyIndex2++)
						{
							return (0x1000 + (0x100 * i) + 0x6);
						}
						if (index == joyIndex2++)
						{
							return (0x1000 + (0x100 * i) + 0x7);
						}
						if (index == joyIndex2++)
						{
							return (0x1000 + (0x100 * i) + 0x8);
						}
					}
				}
			}
		}
		
		// Update previous state.
		memcpy(prevDiKeys, curDiKeys, sizeof(prevDiKeys));
		memcpy(prevJoyKeys, curJoyKeys, sizeof(prevJoyKeys));
		prevReady = true;
	}
}


/**
 * update(): Update the input subsystem.
 */
void Input_DInput::update(void)
{
	//DIMOUSESTATE MouseState;
	HRESULT rval;
	int i;
	
	rval = lpDIDKeyboard->GetDeviceState(256, &m_DIKeys);
	
	// HACK because DirectInput is totally wacky about recognizing the PAUSE/BREAK key
	// still not perfect with this, but at least it goes above a 25% success rate
	if(GetAsyncKeyState(VK_PAUSE)) // normally this should have & 0x8000, but apparently this key is too special for that to work
		m_DIKeys[0xC5] |= 0x80;
	
	if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
		restoreInput();
	
	for (i = 0; i < m_numJoysticks; i++)
	{
		if (m_joyID[i])
		{
			m_joyID[i]->Poll();
			rval = m_joyID[i]->GetDeviceState(sizeof(m_joyState[i]), &m_joyState[i]);
			if (rval != DI_OK) m_joyID[i]->Acquire();
		}
	}
	
	//rval = lpDIDMouse->GetDeviceState(sizeof(MouseState), &MouseState);
	
	//if ((rval == DIERR_INPUTLOST) | (rval == DIERR_NOTACQUIRED))
	//	Restore_Input();

	//MouseX = MouseState.lX;
	//MouseY = MouseState.lY;
	
	// TODO: This is from Gens/Rerecording's hotkey remapping system.
#if 0
	int numInputButtons = GetNumHotkeys();
	for(int i = 0; i < numInputButtons; i++)
	{
		InputButton& button = s_inputButtons[i];

		BOOL pressed = button.diKey ? Check_Key_Pressed(button.diKey) : FALSE;

		if(button.virtKey || button.modifiers)
		{
			bool pressed2 = button.virtKey ? !!(GetAsyncKeyState(button.virtKey) & 0x8000) : true;

			pressed2 &= !(button.modifiers & MOD_CONTROL) == !(GetAsyncKeyState(VK_CONTROL) & 0x8000);
			pressed2 &= !(button.modifiers & MOD_SHIFT) == !(GetAsyncKeyState(VK_SHIFT) & 0x8000);
			pressed2 &= !(button.modifiers & MOD_ALT) == !(GetAsyncKeyState(VK_MENU) & 0x8000);
			pressed2 &= !(button.modifiers & MOD_WIN) == !((GetAsyncKeyState(VK_LWIN)|GetAsyncKeyState(VK_RWIN)) & 0x8000);

			if(!button.diKey)
				pressed = TRUE;

			if(!pressed2)
				pressed = FALSE;
		}

		if (button.alias)
			*button.alias = pressed;

		BOOL oldPressed = button.heldNow;
		button.heldNow = pressed;

		if (pressed && !oldPressed && button.eventID && !button.ShouldUseAccelerator())
			SendMessage(Gens_hWnd, WM_COMMAND, button.eventID, 0);
	}
#endif
}


/**
 * checkKeyPressed(): Checks if the specified key is pressed.
 * @param key Key to check.
 * @return True if the key is pressed.
 */
bool Input_DInput::checkKeyPressed(unsigned int key)
{
	// If the key value is <256, it's a keyboard key.
	if (key < 0x100)
		return (m_DIKeys[key] & 0x80);
	
	// Joystick "key" check.
	
	// Determine which joystick we're looking for.
	int joyNum = ((key >> 8) & 0xF);
	
	// Check that this joystick exists.
	if (!joyExists(joyNum))
		return false;
	
	// Joystick exists. Check the state.
	if (key & 0x80)
	{
		// Joystick POV
		int value = m_joyState[joyNum].rgdwPOV[(key >> 4) & 3];
		if (value == -1)
			return false;
		
		switch (key & 0xF)
		{
			case 1:
				if (value >= 29250 || value <= 6750)
					return true;
				break;
			case 2:
				if (value >= 2250 && value <= 15750)
					return true;
				break;
			case 3:
				if (value >= 11250 && value <= 24750)
					return true;
				break;
			case 4:
				if (value >= 20250 && value <= 33750)
					return true;
				break;
		}
	}
	else if (key & 0x70)
	{
		// Joystick buttons
		if (m_joyState[joyNum].rgbButtons[(key & 0xFF) - 0x10])
			return true;
	}
	else
	{
		// Joystick axes
		switch (key & 0xF)
		{
			case 1:
				if (m_joyState[joyNum].lY < -500)
					return true;
				break;
			case 2:
				if (m_joyState[joyNum].lY > +500)
					return true;
				break;
			case 3:
				if (m_joyState[joyNum].lX < -500)
					return true;
				break;
			case 4:
				if (m_joyState[joyNum].lX > +500)
					return true;
				break;
			case 5:
				if (m_joyState[joyNum].lRx < 0x3FFF)
					return true;
				break;
			case 6:
				if (m_joyState[joyNum].lRx > 0xBFFF)
					return true;
				break;
			case 7:
				if (m_joyState[joyNum].lRy < 0x3FFF)
					return true;
				break;
			case 8:
				if (m_joyState[joyNum].lRy > 0xBFFF)
					return true;
				break;
			case 9:
				if (m_joyState[joyNum].lZ < 0x3FFF)
					return true;
				break;
			case 10:
				if (m_joyState[joyNum].lZ > 0xBFFF)
					return true;
				break;
		}
	}
	
	// Key is not pressed.
	return false;
}


/**
 * setCooperativeLevel(): Sets the cooperative level.
 * @param hWnd Window to set the cooperative level on.
 */
void Input_DInput::setCooperativeLevel(HWND hWnd)
{
	// If no hWnd was specified, use the Gens window.
	if (!hWnd)
		hWnd = Gens_hWnd;
	
	if (!hWnd || !lpDIDKeyboard /*|| lpDIDMouse*/)
		return;
	
	HRESULT rval;
	//rval = lpDIDMouse->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
	rval = lpDIDKeyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
	if (rval != DI_OK)
	{
		fprintf(stderr, "%s(): lpDIDKeyboard->SetCooperativeLevel() failed.\n", __func__);
		// TODO: Error handling code.
	}
	else
	{
		fprintf(stderr, "%s(): lpDIDKeyboard->SetCooperativeLevel() succeeded.\n", __func__);
	}
}


/**
 * setCooperativeLevel_Joysticks(): Sets the cooperative level on joysticks.
 * @param hWnd Window to set the cooperative level on.
 */
void Input_DInput::setCooperativeLevel_Joysticks(HWND hWnd)
{
	// If no hWnd was specified, use the Gens window.
	if (!hWnd)
		hWnd = Gens_hWnd;
	
	HRESULT rval;
	for (int i = 0; i < MAX_JOYS; i++)
	{
		if (!m_joyID[i])
			continue;
		
		rval = m_joyID[i]->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND);
		
		if (rval != DI_OK)
		{
			fprintf(stderr, "%s(): SetCooperativeLevel() failed on joystick %d.\n", __func__, i);
			m_joyID[m_numJoysticks]->Release();
			m_joyID[m_numJoysticks] = NULL;
		}
		else
		{
			fprintf(stderr, "%s(): SetCooperativeLevel() succeeded on joystick %d.\n", __func__, i);
		}
	}
}
