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
 * DirectInputSystem.cpp
 * 
 * Implementation of the DirectInput-based input system. Also provides support
 * for XInput and Raw Input.
 */

#include "DirectInputSystem.h"
#include "Supermodel.h"

#include <wbemidl.h>
#include <oleauto.h>

#include <SDL.h>
#include <SDL_syswm.h>

// TODO - need to double check these all correct and see if can fill in any missing codes (although most just don't exist)
DIKeyMapStruct CDirectInputSystem::s_keyMap[] = 
{
	// General keys
	{ "BACKSPACE",			DIK_BACK },
	{ "TAB",				DIK_TAB },
	//{ "CLEAR",			?? },
	{ "RETURN",				DIK_RETURN },
	{ "PAUSE",				DIK_PAUSE },
	{ "ESCAPE",				DIK_ESCAPE },
	{ "SPACE",				DIK_SPACE },
	//{ "EXCLAIM",			?? },
	//{ "DBLQUOTE",			?? },
	//{ "HASH",				?? }, 
	//{ "DOLLAR",			?? },
	//{ "AMPERSAND",		?? },
	{ "QUOTE",				DIK_APOSTROPHE },
	{ "LEFTPAREN",			DIK_LBRACKET },
	{ "RIGHTPAREN",			DIK_RBRACKET },
	//{ "ASTERISK",			?? },
	//{ "PLUS",				?? },
	{ "COMMA",				DIK_COMMA },
	{ "MINUS",				DIK_MINUS },
	{ "PERIOD",				DIK_PERIOD },
	{ "SLASH",				DIK_SLASH },
	{ "0",					DIK_0 },
	{ "1",					DIK_1 },
	{ "2",					DIK_2 },
	{ "3",					DIK_3 },
	{ "4",					DIK_4 },
	{ "5",					DIK_5 },
	{ "6",					DIK_6 },
	{ "7",					DIK_7 },
	{ "8",					DIK_8 },
	{ "9",					DIK_9 },
	//{ "COLON",			?? },
	{ "SEMICOLON",			DIK_SEMICOLON },
	{ "LESS",				DIK_OEM_102 },
	{ "EQUALS",				DIK_EQUALS },
	//{ "GREATER",			?? },
	//{ "QUESTION",			?? },
	//{ "AT",				?? },
	//{ "LEFTBRACKET",		?? },
	//{ "BACKSLASH",		?? },
	//{ "RIGHTBRACKET",		?? },
	//{ "CARET",			?? },
	//{ "UNDERSCORE",		?? },
	{ "BACKQUOTE",			DIK_GRAVE },
	{ "A",					DIK_A },
	{ "B",					DIK_B },
	{ "C",					DIK_C },
	{ "D",					DIK_D },
	{ "E",					DIK_E },
	{ "F",					DIK_F },
	{ "G",					DIK_G },
	{ "H",					DIK_H },
	{ "I",					DIK_I },
	{ "J",					DIK_J },
	{ "K",					DIK_K },
	{ "L",					DIK_L },
	{ "M",					DIK_M },
	{ "N",					DIK_N },
	{ "O",					DIK_O },
	{ "P",					DIK_P },
	{ "Q",					DIK_Q },
	{ "R",					DIK_R },
	{ "S",					DIK_S },
	{ "T",					DIK_T },
	{ "U",					DIK_U },
	{ "V",					DIK_V },
	{ "W",					DIK_W },
	{ "X",					DIK_X },
	{ "Y",					DIK_Y },
	{ "Z",					DIK_Z },
	{ "DEL",				DIK_DELETE },
	
	// Keypad
	{ "KEYPAD0",			DIK_NUMPAD0 },
	{ "KEYPAD1",			DIK_NUMPAD1 },
	{ "KEYPAD2",			DIK_NUMPAD2 },
	{ "KEYPAD3",			DIK_NUMPAD3 },
	{ "KEYPAD4",			DIK_NUMPAD4 },
	{ "KEYPAD5",			DIK_NUMPAD5 },
	{ "KEYPAD6",			DIK_NUMPAD6 },
	{ "KEYPAD7",			DIK_NUMPAD7 },
	{ "KEYPAD8",			DIK_NUMPAD8 },
	{ "KEYPAD9",			DIK_NUMPAD9 },
	{ "KEYPADPERIOD",		DIK_DECIMAL },
	{ "KEYPADDIVIDE",		DIK_DIVIDE },
	{ "KEYPADMULTIPLY",		DIK_MULTIPLY },
	{ "KEYPADMINUS",		DIK_SUBTRACT },
	{ "KEYPADPLUS",			DIK_ADD },
	{ "KEYPADENTER",		DIK_NUMPADENTER },
	{ "KEYPADEQUALS",		DIK_NUMPADEQUALS },
	
	// Arrows + Home/End Pad
	{ "UP",					DIK_UP },
	{ "DOWN",				DIK_DOWN },
	{ "RIGHT",				DIK_RIGHT },
	{ "LEFT",				DIK_LEFT },
	{ "INSERT",				DIK_INSERT },
	{ "HOME",				DIK_HOME },
	{ "END",				DIK_END },
	{ "PGUP",				DIK_PRIOR },
	{ "PGDN",				DIK_NEXT },

	// Function Key
	{ "F1",					DIK_F1 },
	{ "F2",					DIK_F2 },
	{ "F3",					DIK_F3 },
	{ "F4",					DIK_F4 },
	{ "F5",					DIK_F5 },
	{ "F6",					DIK_F6 },
	{ "F7",					DIK_F7 },
	{ "F8",					DIK_F8 },
	{ "F9",					DIK_F9 },
	{ "F10",				DIK_F10 },
	{ "F11",				DIK_F11 },
	{ "F12",				DIK_F12 },
	{ "F13",				DIK_F13 },
	{ "F14",				DIK_F14 },
	{ "F15",				DIK_F15 },
    
	// Modifier Keys  
	{ "NUMLOCK",			DIK_NUMLOCK },
	{ "CAPSLOCK",			DIK_CAPITAL },
	{ "SCROLLLOCK",			DIK_SCROLL },
	{ "RIGHTSHIFT",			DIK_RSHIFT },
	{ "LEFTSHIFT",			DIK_LSHIFT },
	{ "RIGHTCTRL",			DIK_RCONTROL },
	{ "LEFTCTRL",			DIK_LCONTROL },
	{ "RIGHTALT",			DIK_RMENU },
	{ "LEFTALT",			DIK_LMENU },
	//{ "RIGHTMETA",		?? },
	//{ "LEFTMETA",			?? },
	{ "RIGHTWINDOWS",		DIK_RWIN },
	{ "LEFTWINDOWS",		DIK_LWIN },
	//{ "ALTGR",			?? },
	//{ "COMPOSE",			?? },
    
	// Other
	//{ "HELP",				?? },
	{ "PRINT",				DIK_SYSRQ },
	//{ "SYSREQ",			?? },
	//{ "BREAK",			?? },								
	//{ "MENU",				?? },
	//{ "POWER",			?? },
	//{ "EURO",				?? },
	//{ "UNDO",				?? },
};

static bool IsXInputDevice(const GUID &devProdGUID)
{
	// Following code taken from MSDN
	IWbemLocator* pIWbemLocator  = NULL;
    IEnumWbemClassObject* pEnumDevices = NULL;
    IWbemClassObject* pDevices[20] = {0};
    IWbemServices* pIWbemServices = NULL;
    BSTR bstrNamespace = NULL;
    BSTR bstrDeviceID = NULL;
    BSTR bstrClassName = NULL;
    
    // Create WMI
    bool isXInpDev = false;
    HRESULT hr = CoCreateInstance(__uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*)&pIWbemLocator);
    if (FAILED(hr) || pIWbemLocator == NULL)
        goto Finish;

    if ((bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2")) == NULL) goto Finish;        
    if ((bstrClassName = SysAllocString(L"Win32_PNPEntity")) == NULL)    goto Finish;        
    if ((bstrDeviceID  = SysAllocString(L"DeviceID")) == NULL)           goto Finish;        
    
    // Connect to WMI 
    hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices);
    if (FAILED(hr) || pIWbemServices == NULL)
        goto Finish;

    // Switch security level to IMPERSONATE 
    CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);
    
	hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices); 
    if (FAILED(hr) || pEnumDevices == NULL)
        goto Finish;

    // Loop over all devices
    for (;;)
    {
        // Get 20 at a time
		DWORD uReturned;
        hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
        if (FAILED(hr) || uReturned == 0)
            goto Finish;

        for (unsigned devNum = 0; devNum < uReturned; devNum++)
        {
            // For each device, get its device ID
			VARIANT var;
            hr = pDevices[devNum]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
            if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL)
            {
                // Check if the device ID contains "IG_", which means it's an XInput device (this can't be determined via DirectInput on its own)
                if (wcsstr(var.bstrVal, L"IG_"))
                {
                    // If so, then get VID/PID from var.bstrVal
                    DWORD dwPid = 0, dwVid = 0;
                    WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
                    if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
                        dwVid = 0;
                    WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
                    if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
                        dwPid = 0;

                    // Compare VID/PID to values held in DirectInput device's product GUID
                    DWORD dwVidPid = MAKELONG(dwVid, dwPid);
                    if (dwVidPid == devProdGUID.Data1)
                    {
                        isXInpDev = true;
                        goto Finish;
                    }
                }
            }   
			if (pDevices[devNum] != NULL)
			{
				pDevices[devNum]->Release();	
				pDevices[devNum] = NULL;
			}
        }
    }

Finish:
    if (bstrNamespace)
        SysFreeString(bstrNamespace);
    if (bstrDeviceID)
        SysFreeString(bstrDeviceID);
    if (bstrClassName)
        SysFreeString(bstrClassName);
    for (unsigned devNum = 0; devNum < 20; devNum++)
	{
		if (pDevices[devNum] != NULL)
			pDevices[devNum]->Release();
	}
	if (pEnumDevices != NULL)
		pEnumDevices->Release();
	if (pIWbemLocator != NULL)
		pIWbemLocator->Release();
	if (pIWbemServices != NULL)
		pIWbemServices->Release();
    return isXInpDev;
}

struct DIEnumDevsContext
{
	vector<DIJoyInfo> *infos;
	bool useXInput;
};

static BOOL CALLBACK DI8EnumDevicesCallback(LPCDIDEVICEINSTANCE instance, LPVOID context)
{
	// Keep track of all joystick device GUIDs
	DIEnumDevsContext *diDevsContext = (DIEnumDevsContext*)context;
	DIJoyInfo info;
	memset(&info, 0, sizeof(DIJoyInfo));
	info.guid = instance->guidInstance;
	// If XInput is enabled, see if device is an XInput device
	info.isXInput = diDevsContext->useXInput && IsXInputDevice(instance->guidProduct);
	diDevsContext->infos->push_back(info);
	return DIENUM_CONTINUE;
}

struct DIEnumAxesContext
{
	JoyDetails *joyDetails;
	bool enumError;
};

static BOOL CALLBACK DI8EnumAxesCallback(LPCDIDEVICEOBJECTINSTANCE instance, LPVOID context)
{
	// Work out which axis is currently being enumerated from the GUID
	DIEnumAxesContext *diAxesContext = (DIEnumAxesContext*)context;
	int axisNum;
	if      (instance->guidType == GUID_XAxis)  axisNum = AXIS_X;
	else if (instance->guidType == GUID_YAxis)  axisNum = AXIS_Y;
	else if (instance->guidType == GUID_ZAxis)  axisNum = AXIS_Z;
	else if (instance->guidType == GUID_RxAxis) axisNum = AXIS_RX;
	else if (instance->guidType == GUID_RyAxis) axisNum = AXIS_RY;
	else if (instance->guidType == GUID_RzAxis) axisNum = AXIS_RZ;
	else
	{
		// If couldn't match GUID (which, according to MSDN, is an optional attribute), then flag error and try matching using a different method
		diAxesContext->enumError = true;
		int objNum = DIDFT_GETINSTANCE(instance->dwType);
		DIOBJECTDATAFORMAT fmt = c_dfDIJoystick2.rgodf[objNum];
		switch (fmt.dwOfs)
		{
			case DIJOFS_X:  axisNum = AXIS_X; break;
			case DIJOFS_Y:  axisNum = AXIS_Y; break;
			case DIJOFS_Z:  axisNum = AXIS_Z; break;
			case DIJOFS_RX: axisNum = AXIS_RX; break;
			case DIJOFS_RY: axisNum = AXIS_RY; break;
			case DIJOFS_RZ: axisNum = AXIS_RZ; break;
			default:        
				// If still couldn't match then it is not an axis
				return DIENUM_CONTINUE;  
		}
	}
	
	// If axis overlaps with a previous ones, flag error
	JoyDetails *joyDetails = diAxesContext->joyDetails;
	if (joyDetails->hasAxis[axisNum])
		diAxesContext->enumError = true;

	// Record fact that axis is present and also whether it has force feedback available
	joyDetails->hasAxis[axisNum] = true; 
	joyDetails->axisHasFF[axisNum] = !!(instance->dwFlags & DIDOI_FFACTUATOR);

	// Get axis name from DirectInput and store that too
	char *axisName = joyDetails->axisName[axisNum];
	strcpy(axisName, CInputSystem::GetDefaultAxisName(axisNum));
	strcat(axisName, "-Axis (");
	strncat(axisName, instance->tszName, MAX_NAME_LENGTH - strlen(axisName) - 1);
	strcat(axisName, ")");

	return DIENUM_CONTINUE;
}

static BOOL CALLBACK DI8EnumEffectsCallback(LPCDIEFFECTINFO effectInfo, LPVOID context)
{
	// Check joystick has at least one of required types of effects
	JoyDetails *joyDetails = (JoyDetails*)context;
	if (!!(effectInfo->dwEffType & (DIEFT_CONSTANTFORCE | DIEFT_PERIODIC | DIEFT_CONDITION)))
		joyDetails->hasFFeedback = true;
	return DIENUM_CONTINUE;
}

const char *CDirectInputSystem::ConstructName(bool useRawInput, bool useXInput)
{
	if (useRawInput)
		return (useXInput ? "RawInput/XInput" : "RawInput/DirectInput");
	else
		return (useXInput ? "Xinput" : "DirectInput");
}

CDirectInputSystem::CDirectInputSystem(bool useRawInput, bool useXInput) : 
	CInputSystem(ConstructName(useRawInput, useXInput)),
	m_useRawInput(useRawInput), m_useXInput(useXInput), m_enableFFeedback(true),
	m_initializedCOM(false), m_activated(false), m_hwnd(NULL), m_screenW(0), m_screenH(0), 
	m_getRIDevListPtr(NULL), m_getRIDevInfoPtr(NULL), m_regRIDevsPtr(NULL), m_getRIDataPtr(NULL),
	m_xiGetCapabilitiesPtr(NULL), m_xiGetStatePtr(NULL), m_xiSetStatePtr(NULL), m_di8(NULL), m_di8Keyboard(NULL), m_di8Mouse(NULL)
{
	// Reset initial states
	memset(&m_combRawMseState, 0, sizeof(RawMseState));
	memset(&m_diKeyState, 0, sizeof(LPDIRECTINPUTDEVICE8));
	memset(&m_diMseState, 0, sizeof(LPDIRECTINPUTDEVICE8));
}

CDirectInputSystem::~CDirectInputSystem()
{
	CloseKeyboardsAndMice();
	CloseJoysticks();

	if (m_di8)
	{
		m_di8->Release();
		m_di8 = NULL;
		if (m_initializedCOM)
			CoUninitialize();
	}
}

bool CDirectInputSystem::GetRegString(HKEY regKey, const char *regPath, string &str)
{
	// Query to get the length
	DWORD dataLen;
	LONG result = RegQueryValueEx(regKey, regPath, NULL, NULL, NULL, &dataLen);
	if (result != ERROR_SUCCESS)
		return false;
	
	// Retrieve the actual data
	char data[MAX_PATH];
	dataLen = min<DWORD>(MAX_PATH - 1, dataLen);
	result = RegQueryValueEx(regKey, regPath, NULL, NULL, (LPBYTE)data, &dataLen);
	if (result != ERROR_SUCCESS)
		return false;
	data[MAX_PATH - 1] = '\0';
	str.assign(data);
	return true;
}

bool CDirectInputSystem::GetRegDeviceName(const char *rawDevName, char *name)
{
	// Check raw device string is in form that can be handled and remove initial 4-char sequence
	// For XP this is: \??\TypeID#HardwareID#InstanceID#{DevicesClasses-id}
	// For Vista/Win7 64bit this is: \\?\TypeID#HardwareID#InstanceID#{DevicesClasses-id}
	string devNameStr(rawDevName);
	if (devNameStr.find("\\??\\") != string::npos || devNameStr.find("\\\\?\\") != string::npos)
		devNameStr.erase(0, 4);
	else 
		return false;

	// Append raw device string to base registry path and convert all #'s to \ in the process
	string regPath = "SYSTEM\\CurrentControlSet\\Enum\\" + devNameStr;
	for (size_t i = 0; i < regPath.size(); i++)
	{
		if (regPath[i] == '#')
			regPath[i] = '\\';
	}

	// Remove part after last \ in path
	size_t last = regPath.rfind('\\');
	if (last != string::npos)
		regPath = regPath.erase(last);

	// Try and open registry key with this path
	HKEY regKey;
	LONG result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &regKey);
	if (result != ERROR_SUCCESS)
		return false;

	string parentIdStr;

	// Fetch device description from registry, if it exists, and use that for name
	string regStr;
	if (GetRegString(regKey, "DeviceDesc", regStr))
		goto Found;

	// If above failed, then try looking at USB key for HID devices
	RegCloseKey(regKey);

	// Check it is HID device
	if (devNameStr.find("HID") == string::npos)
		return false;

	// Get parent id, from after last \ in name
	last = regPath.rfind('\\');
	if (last == regPath.size() - 1 || last == string::npos)
		return false;
	parentIdStr = regPath.substr(last + 1);

	// Open USB base key
	result = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SYSTEM\\CurrentControlSet\\Enum\\USB", 0, KEY_READ, &regKey);
	if (result != ERROR_SUCCESS)
		return false;

	// Loop through all USB devices
	for (int usbIndex = 0; result == ERROR_SUCCESS; usbIndex++)
	{
		// Get sub-key name
		char keyName[MAX_PATH];
		DWORD nameLen = MAX_PATH - 1;
		result = RegEnumKeyEx(regKey, usbIndex, keyName, &nameLen, NULL, NULL, NULL, NULL);
		if (result == ERROR_SUCCESS)
		{
			// Open sub-key
			HKEY subRegKey;
			LONG subResult = RegOpenKeyEx(regKey, keyName, 0, KEY_READ, &subRegKey);
			if (subResult != ERROR_SUCCESS)
				continue;

			// Loop through all sub-keys
			for (int subIndex = 0; subResult == ERROR_SUCCESS; subIndex++)
			{
				//  the next enumerated subkey and scan it
				nameLen = MAX_PATH - 1;
				subResult = RegEnumKeyEx(subRegKey, subIndex, keyName, &nameLen, NULL, NULL, NULL, NULL);
				if (subResult == ERROR_SUCCESS)
				{
					// Open final key
					HKEY finalRegKey;
					LONG finalResult = RegOpenKeyEx(subRegKey, keyName, 0, KEY_READ, &finalRegKey);
					if (finalResult != ERROR_SUCCESS)
						continue;

					// Get parent id prefix and see if it matches
					string finalParentIdStr;
					if (GetRegString(finalRegKey, "ParentIdPrefix", finalParentIdStr) && parentIdStr.compare(0, finalParentIdStr.size(), finalParentIdStr) == 0)
					{
						// Get device description, if it exists, and use that for name
						if (GetRegString(finalRegKey, "DeviceDesc", regStr))
						{
							RegCloseKey(finalRegKey);
							RegCloseKey(subRegKey);
							goto Found;
						}
					}

					// Close final key
					RegCloseKey(finalRegKey);
				}
			}

			// Close sub-key
			RegCloseKey(subRegKey);
		}
	}

	RegCloseKey(regKey);
	return false;

Found:
	// If found device description, name will be from final colon
	last = regStr.rfind(';');
	if (last == regStr.size() - 1 || last == string::npos)
		last = 0;
	else 
		last++;
	strncpy(name, regStr.c_str() + last, MAX_NAME_LENGTH - 1);
	name[MAX_NAME_LENGTH - 1] = '\0';

	RegCloseKey(regKey);
	return true;
}

void CDirectInputSystem::OpenKeyboardsAndMice()
{
	if (m_useRawInput)
	{
		// If RawInput enabled, get list of available devices
		UINT nDevices;
		if (m_getRIDevListPtr(NULL, &nDevices, sizeof(RAWINPUTDEVICELIST)) == 0 && nDevices > 0)
		{
			PRAWINPUTDEVICELIST pDeviceList = new RAWINPUTDEVICELIST[nDevices];
			if (pDeviceList != NULL && m_getRIDevListPtr(pDeviceList, &nDevices, sizeof(RAWINPUTDEVICELIST)) != (UINT)-1)
			{
				// Loop through devices backwards (since new devices are usually added at beginning)
				for (int devNum = nDevices - 1; devNum >= 0; devNum--)
				{
					RAWINPUTDEVICELIST device = pDeviceList[devNum];
					
					// Get device name
					UINT nLength;
					if (m_getRIDevInfoPtr(device.hDevice, RIDI_DEVICENAME, NULL, &nLength) != 0)
						continue;
					nLength = min<int>(MAX_NAME_LENGTH, nLength);
					char name[MAX_NAME_LENGTH];
					if (m_getRIDevInfoPtr(device.hDevice, RIDI_DEVICENAME, name, &nLength) == -1)
						continue;

					// Ignore any RDP devices
					if (strstr(name, "Root#RDP_") != NULL)
						continue;

					// Store details and device handles for attached keyboards and mice
					if (device.dwType == RIM_TYPEKEYBOARD)
					{
						m_rawKeyboards.push_back(device.hDevice);

						KeyDetails keyDetails;
						if (!GetRegDeviceName(name, keyDetails.name))
							strcpy(keyDetails.name, "Unknown Keyboard");
						m_keyDetails.push_back(keyDetails);

						bool *pKeyState = new bool[255];
						memset(pKeyState, 0, sizeof(bool) * 255);
						m_rawKeyStates.push_back(pKeyState);
					}
					else if (device.dwType == RIM_TYPEMOUSE)
					{
						m_rawMice.push_back(device.hDevice);

						MouseDetails mseDetails;
						if (!GetRegDeviceName(name, mseDetails.name))
							strcpy(mseDetails.name, "Unknown Mouse");
						// TODO mseDetails.isAbsolute = ???
						m_mseDetails.push_back(mseDetails);

						RawMseState mseState;
						memset(&mseState, 0, sizeof(RawMseState));
						m_rawMseStates.push_back(mseState);
					}
				}

				DebugLog("RawInput - found %d keyboards and %d mice", m_rawKeyboards.size(), m_rawMice.size());

				// Check some devices were actually found
				m_useRawInput = m_rawKeyboards.size() > 0 && m_rawMice.size() > 0;
			}
			else
			{
				ErrorLog("Unable to query RawInput API for attached devices (error %d) - switching to DirectInput.\n", GetLastError());

				m_useRawInput = false;
			}

			if (pDeviceList != NULL)
				delete[] pDeviceList;
		}
		else
		{
			ErrorLog("Unable to query RawInput API for attached devices (error %d) - switching to DirectInput.\n", GetLastError());

			m_useRawInput = false;
		}

		if (m_useRawInput)
			return;
	}

	// If get here then either RawInput disabled or getting its devices failed so default to DirectInput.
	// Open DirectInput system keyboard and set its data format
	HRESULT hr;
	if (FAILED(hr = m_di8->CreateDevice(GUID_SysKeyboard, &m_di8Keyboard, NULL)))
	{
		ErrorLog("Unable to create DirectInput keyboard device (error %d) - key input will be unavailable.\n", hr);

		m_di8Keyboard = NULL;	
	}
	else if (FAILED(hr = m_di8Keyboard->SetDataFormat(&c_dfDIKeyboard)))
	{
		ErrorLog("Unable to set data format for DirectInput keyboard (error %d) - key input will be unavailable.\n", hr);

		m_di8Keyboard->Release();
		m_di8Keyboard = NULL;
	}
	
	// Open DirectInput system mouse and set its data format
	if (FAILED(hr = m_di8->CreateDevice(GUID_SysMouse, &m_di8Mouse, NULL)))
	{
		ErrorLog("Unable to create DirectInput mouse device (error %d) - mouse input will be unavailable.\n", hr);

		m_di8Mouse = NULL;	
		return;
	}
	if (FAILED(hr = m_di8Mouse->SetDataFormat(&c_dfDIMouse2)))
	{
		ErrorLog("Unable to set data format for DirectInput mouse (error %d) - mouse input will be unavailable.\n", hr);

		m_di8Mouse->Release();
		m_di8Mouse = NULL;
		return;
	}

	// Set mouse axis mode to relative
	DIPROPDWORD dipdw;
	dipdw.diph.dwSize = sizeof(DIPROPDWORD); 
	dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
	dipdw.diph.dwHow = DIPH_DEVICE;
	dipdw.diph.dwObj = 0;
	dipdw.dwData = DIPROPAXISMODE_REL;
	if (FAILED(hr = m_di8Mouse->SetProperty(DIPROP_AXISMODE, &dipdw.diph)))
	{
		ErrorLog("Unable to set axis mode for DirectInput mouse (error %d) - mouse input will be unavailable.\n", hr);

		m_di8Mouse->Release();
		m_di8Mouse = NULL;
	}
}

void CDirectInputSystem::ActivateKeyboardsAndMice()
{
	// Sync up all mice with current cursor position
	ResetMice();

	if (m_useRawInput)
	{
		// Register for RawInput
		RAWINPUTDEVICE rid[2];
		
		// Register for keyboard input
		rid[0].usUsagePage = 0x01;
		rid[0].usUsage = 0x06;
		rid[0].dwFlags = (m_grabMouse ? RIDEV_CAPTUREMOUSE : RIDEV_INPUTSINK) | RIDEV_NOLEGACY;
		rid[0].hwndTarget = m_hwnd;

		// Register for mouse input
		rid[1].usUsagePage = 0x01;
		rid[1].usUsage = 0x02;
		rid[1].dwFlags = (m_grabMouse ? RIDEV_CAPTUREMOUSE : RIDEV_INPUTSINK) | RIDEV_NOLEGACY;
		rid[1].hwndTarget = m_hwnd;

		if (!m_regRIDevsPtr(rid, 2, sizeof(RAWINPUTDEVICE)))
			ErrorLog("Unable to register for keyboard and mouse input with RawInput API (error %d) - keyboard and mouse input will be unavailable.\n", GetLastError());
		return;
	}
	
	// Set DirectInput cooperative level of keyboard and mouse
	if (m_di8Keyboard != NULL)
	{
		m_di8Keyboard->Unacquire();
		m_di8Keyboard->SetCooperativeLevel(m_hwnd, (m_grabMouse ? DISCL_FOREGROUND : DISCL_BACKGROUND) | DISCL_NONEXCLUSIVE);
		m_di8Keyboard->Acquire();
	}
	if (m_di8Mouse != NULL)
	{
		m_di8Mouse->Unacquire();
		m_di8Mouse->SetCooperativeLevel(m_hwnd, (m_grabMouse ? DISCL_FOREGROUND : DISCL_BACKGROUND) | DISCL_NONEXCLUSIVE);
		m_di8Mouse->Acquire();
	}
}

void CDirectInputSystem::PollKeyboardsAndMice()
{
	if (m_useRawInput)
	{
		// For RawInput, only thing to do is update wheelDir from wheelData for each mouse state.  Everything else is updated via WM events.
		for (vector<RawMseState>::iterator it = m_rawMseStates.begin(); it != m_rawMseStates.end(); it++)
		{
			if (it->wheelDelta != 0)
			{
				it->wheelDir = (it->wheelDelta > 0 ? 1 : -1);
				it->wheelDelta = 0;
			}
			else
				it->wheelDir = 0;
		}
		if (m_combRawMseState.wheelDelta != 0)
		{
			m_combRawMseState.wheelDir = (m_combRawMseState.wheelDelta > 0 ? 1 : -1);
			m_combRawMseState.wheelDelta = 0;
		}
		else
			m_combRawMseState.wheelDir = 0;
		return;
	}

	// Get current keyboard state from DirectInput
	HRESULT hr;
	if (m_di8Keyboard != NULL)
	{
		if (FAILED(hr = m_di8Keyboard->Poll()))
		{
			hr = m_di8Keyboard->Acquire();
			while (hr == DIERR_INPUTLOST)
				hr = m_di8Keyboard->Acquire();

			if (hr == DIERR_OTHERAPPHASPRIO || hr == DIERR_INVALIDPARAM || hr == DIERR_NOTINITIALIZED)
				return;
		}

		// Keep track of keyboard state
		m_di8Keyboard->GetDeviceState(sizeof(m_diKeyState), m_diKeyState);
	}

	// Get current mouse state from DirectInput
	if (m_di8Mouse != NULL)
	{
		if (FAILED(hr = m_di8Mouse->Poll()))
		{
			hr = m_di8Mouse->Acquire();
			while (hr == DIERR_INPUTLOST)
				hr = m_di8Mouse->Acquire();

			if (hr == DIERR_OTHERAPPHASPRIO || hr == DIERR_INVALIDPARAM || hr == DIERR_NOTINITIALIZED)
				return;
		}

		// Keep track of mouse absolute axis values, clamping them at display edges, aswell as wheel direction and buttons
		DIMOUSESTATE2 mseState;
		m_di8Mouse->GetDeviceState(sizeof(mseState), &mseState);
		m_diMseState.x = CInputSource::Clamp(m_diMseState.x + mseState.lX, m_dispX, m_dispX + m_dispW);
		m_diMseState.y = CInputSource::Clamp(m_diMseState.y + mseState.lY, m_dispY, m_dispY + m_dispH);
		if (mseState.lZ != 0)
		{
			// Z-axis is clamped to range -100 to 100 (DirectInput returns +120 & -120 for wheel delta which are scaled to +5 & -5)
			LONG wheelDelta = 5 * mseState.lZ / 120;
			m_diMseState.z = CInputSource::Clamp(m_diMseState.z + wheelDelta, -100, 100);
			m_diMseState.wheelDir = (wheelDelta > 0 ? 1 : -1);
		}
		else
			m_diMseState.wheelDir = 0;
		memcpy(&m_diMseState.buttons, mseState.rgbButtons, sizeof(m_diMseState.buttons));
	}
}

void CDirectInputSystem::CloseKeyboardsAndMice()
{
	if (m_useRawInput)
	{
		if (m_activated)
		{
			// If RawInput was registered, then unregister now
			RAWINPUTDEVICE rid[2];
			
			// Unregister from keyboard input
			rid[0].usUsagePage = 0x01;
			rid[0].usUsage = 0x06;
			rid[0].dwFlags = RIDEV_REMOVE;
			rid[0].hwndTarget = m_hwnd;

			// Unregister from mouse input
			rid[1].usUsagePage = 0x01;
			rid[1].usUsage = 0x02;
			rid[1].dwFlags = RIDEV_REMOVE;
			rid[1].hwndTarget = m_hwnd;

			m_regRIDevsPtr(rid, 2, sizeof(RAWINPUTDEVICE));
		}

		// Delete storage for keyboards
		for (vector<bool*>::iterator it = m_rawKeyStates.begin(); it != m_rawKeyStates.end(); it++)
			delete[] *it;
		m_keyDetails.clear();
		m_rawKeyboards.clear();
		m_rawKeyStates.clear();

		// Delete storage for mice
		m_mseDetails.clear();
		m_rawMice.clear();
		m_rawMseStates.clear();
	}

	// If DirectInput keyboard and mouse were created, then release them too
	if (m_di8Keyboard != NULL)
	{
		m_di8Keyboard->Unacquire();
		m_di8Keyboard->Release();
		m_di8Keyboard = NULL;
	}
	if (m_di8Mouse != NULL)
	{
		m_di8Mouse->Unacquire();
		m_di8Mouse->Release();
		m_di8Mouse = NULL;
	}
}

void CDirectInputSystem::ResetMice()
{
	// Get current mouse cursor position in window
	POINT p;
	if (!GetCursorPos(&p) || !ScreenToClient(m_hwnd, &p))
		return;

	// Set all mice coords to current cursor position
	if (m_useRawInput)
	{
		m_combRawMseState.x = p.x;
		m_combRawMseState.y = p.y;
		m_combRawMseState.z = 0;
		for (vector<RawMseState>::iterator it = m_rawMseStates.begin(); it != m_rawMseStates.end(); it++)
		{
			it->x = p.x;
			it->y = p.y;
			it->z = 0;
		}
	}

	m_diMseState.x = p.x;
	m_diMseState.y = p.y;
	m_diMseState.z = 0;
}

void CDirectInputSystem::ProcessRawInput(HRAWINPUT hInput)
{
	// RawInput data event
	BYTE buffer[4096];
	LPBYTE pBuf = buffer;

	// Get size of data structure to receive
	UINT dwSize;
	if (m_getRIDataPtr(hInput, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER)) != 0)
		return;
	if (dwSize > sizeof(buffer))
	{
		pBuf = new BYTE[dwSize];
		if (pBuf == NULL)
			return;
	}

	// Get data
	if (m_getRIDataPtr(hInput, RID_INPUT, pBuf, &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
	{
		RAWINPUT *pData = (RAWINPUT*)pBuf;
		if (pData->header.dwType == RIM_TYPEKEYBOARD)
		{
			// Keyboard event, so identify which keyboard produced event
			bool *pKeyState = NULL;
			size_t kbdNum;
			for (kbdNum = 0; kbdNum < m_rawKeyboards.size(); kbdNum++)
			{
				if (m_rawKeyboards[kbdNum] == pData->header.hDevice)
				{
					pKeyState = m_rawKeyStates[kbdNum];
					break;
				}
			}
			
			// Check is a valid keyboard
			if (pKeyState != NULL)
			{
				// Get scancode of key and whether key was pressed or released
				int isRight = (pData->data.keyboard.Flags & RI_KEY_E0);
				UINT8 scanCode = (pData->data.keyboard.MakeCode & 0x7f) | (isRight ? 0x80 : 0x00);
				bool pressed = !(pData->data.keyboard.Flags & RI_KEY_BREAK);

				// Store current state for key 
				if (scanCode != 0xAA)
					pKeyState[scanCode] = pressed;
			}
		}
		else if (pData->header.dwType == RIM_TYPEMOUSE)
		{
			// Mouse event, so identify which mouse produced event
			RawMseState *pMseState = NULL;
			size_t mseNum;
			for (mseNum = 0; mseNum < m_rawMice.size(); mseNum++)
			{
				if (m_rawMice[mseNum] == pData->header.hDevice)
				{
					pMseState = &m_rawMseStates[mseNum];
					break;
				}
			}

			// Check is a valid mouse
			if (pMseState != NULL)
			{
				// Get X- & Y-axis data
				LONG lx = pData->data.mouse.lLastX;
				LONG ly = pData->data.mouse.lLastY;
				if (pData->data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)
				{
					// If data is absolute, then scale source values (which range 0 to 65535) to screen coordinates and convert
					// to be relative to game window origin
					POINT p;
					p.x = CInputSource::Scale(lx, 0, 0xFFFF, 0, m_screenW);
					p.y = CInputSource::Scale(ly, 0, 0xFFFF, 0, m_screenH);
					if (ScreenToClient(m_hwnd, &p))
					{
						pMseState->x = p.x;
						pMseState->y = p.y;
					}

					// Also update combined state
					m_combRawMseState.x = pMseState->x;
					m_combRawMseState.y = pMseState->y;
				}
				else
				{
					// If data is relative, then keep track of absolute position, clamping it at display edges
					pMseState->x = CInputSource::Clamp(pMseState->x + lx, m_dispX, m_dispX + m_dispW);
					pMseState->y = CInputSource::Clamp(pMseState->y + ly, m_dispY, m_dispY + m_dispH);

					// Also update combined state
					m_combRawMseState.x = CInputSource::Clamp(m_combRawMseState.x + lx, m_dispX, m_dispX + m_dispW);
					m_combRawMseState.y = CInputSource::Clamp(m_combRawMseState.y + ly, m_dispY, m_dispY + m_dispH);
				}

				// Get button flags and wheel delta (RawInput returns +120 & -120 for the latter which are scaled to +5 & -5)
				USHORT butFlags = pData->data.mouse.usButtonFlags;
				LONG wheelDelta = 5 * (SHORT)pData->data.mouse.usButtonData / 120;
				
				// Update Z-axis (wheel) value
				if (butFlags & RI_MOUSE_WHEEL)
				{
					// Z-axis is clamped to range -100 to 100
					pMseState->z = CInputSource::Clamp(pMseState->z + wheelDelta, -100, 100);
					pMseState->wheelDelta += wheelDelta;
				}

				// Keep track of buttons pressed/released
				if      (butFlags & RI_MOUSE_LEFT_BUTTON_DOWN)   pMseState->buttons |= 1;
				else if (butFlags & RI_MOUSE_LEFT_BUTTON_UP)     pMseState->buttons &= ~1;
				if      (butFlags & RI_MOUSE_MIDDLE_BUTTON_DOWN) pMseState->buttons |= 2;
				else if (butFlags & RI_MOUSE_MIDDLE_BUTTON_UP)   pMseState->buttons &= ~2;
				if      (butFlags & RI_MOUSE_RIGHT_BUTTON_DOWN)  pMseState->buttons |= 4;
				else if (butFlags & RI_MOUSE_RIGHT_BUTTON_UP)    pMseState->buttons &= ~4;
				if      (butFlags & RI_MOUSE_BUTTON_4_DOWN)      pMseState->buttons |= 8;
				else if (butFlags & RI_MOUSE_BUTTON_4_UP)        pMseState->buttons &= ~8;
				if      (butFlags & RI_MOUSE_BUTTON_5_DOWN)      pMseState->buttons |= 16;
				else if (butFlags & RI_MOUSE_BUTTON_5_UP)        pMseState->buttons &= ~16;

				// Also update combined state for wheel axis and buttons
				if (butFlags & RI_MOUSE_WHEEL)
				{
					// Z-axis is clamped to range -100 to 100
					m_combRawMseState.z = CInputSource::Clamp(m_combRawMseState.z + wheelDelta, -100, 100);
					m_combRawMseState.wheelDelta += wheelDelta;
				}

				m_combRawMseState.buttons = 0;
				for (vector<RawMseState>::iterator it = m_rawMseStates.begin(); it != m_rawMseStates.end(); it++)
					m_combRawMseState.buttons |= it->buttons;
			}
		}
	}

	if (pBuf != buffer)
		delete[] pBuf;
}

void CDirectInputSystem::OpenJoysticks()
{
	// Get the info about all attached joystick devices
	DIEnumDevsContext diDevsContext;
	diDevsContext.infos = &m_diJoyInfos;
	diDevsContext.useXInput = m_useXInput;
	HRESULT hr;
	if (FAILED(hr = m_di8->EnumDevices(DI8DEVCLASS_GAMECTRL, DI8EnumDevicesCallback, &diDevsContext, DIEDFL_ATTACHEDONLY)))
		return;

	// Loop through those found
	int joyNum = 0;
	int xNum = 0;
	for (vector<DIJoyInfo>::iterator it = m_diJoyInfos.begin(); it != m_diJoyInfos.end(); it++)
	{
		joyNum++;

		JoyDetails joyDetails;
		memset(&joyDetails, 0, sizeof(joyDetails));

		// See if can use XInput for device
		if (it->isXInput)
		{
			// If so, set joystick details (currently XBox controller is only gamepad handled by XInput and so its capabilities are fixed)
			sprintf(joyDetails.name, "Xbox 360 Controller %d (via XInput)", (xNum + 1));
			joyDetails.numAxes = 6;  // Left & right triggers are mapped to axes in addition to the two analog sticks, giving a total of 6 axes
			joyDetails.numPOVs = 1;  // Digital D-pad
			joyDetails.numButtons = 10;
			joyDetails.hasFFeedback = m_enableFFeedback;
			joyDetails.hasAxis[AXIS_X] = true;
			joyDetails.hasAxis[AXIS_Y] = true;
			joyDetails.hasAxis[AXIS_Z] = true;
			joyDetails.hasAxis[AXIS_RX] = true;
			joyDetails.hasAxis[AXIS_RY] = true;
			joyDetails.hasAxis[AXIS_RZ] = true;
			joyDetails.axisHasFF[AXIS_X] = true;  // Force feedback simulated on left and right sticks
			joyDetails.axisHasFF[AXIS_Y] = true;
			joyDetails.axisHasFF[AXIS_Z] = false;
			joyDetails.axisHasFF[AXIS_RX] = true;
			joyDetails.axisHasFF[AXIS_RY] = true;
			joyDetails.axisHasFF[AXIS_RZ] = false;
			
			// Keep track of XInput device number
			it->xInputNum = xNum++;		
		}
		else 
		{
			// Otherwise, open joystick with DirectInput for given GUID and set its data format
			LPDIRECTINPUTDEVICE8 joystick;
			if (FAILED(hr = m_di8->CreateDevice(it->guid, &joystick, NULL)))
			{
				ErrorLog("Unable to create DirectInput joystick device %d (error %d) - skipping joystick.\n", joyNum, hr);
				continue;
			}
			if (FAILED(hr = joystick->SetDataFormat(&c_dfDIJoystick2)))
			{
				ErrorLog("Unable to set data format for DirectInput joystick %d (error %d) - skipping joystick.\n", joyNum, hr);
				
				joystick->Release();
				continue;
			}

			// Get joystick's capabilities
			DIDEVCAPS devCaps;
			devCaps.dwSize = sizeof(DIDEVCAPS);
			if (FAILED(hr = joystick->GetCapabilities(&devCaps)))
			{
				ErrorLog("Unable to query capabilities of DirectInput joystick %d (error %d) - skipping joystick.\n", joyNum, hr);
				
				joystick->Release();
				continue;
			}

			// Gather joystick details (name, num POVs & buttons, which axes are available and whether force feedback is available)
			DIPROPSTRING didps;
			didps.diph.dwSize = sizeof(DIPROPSTRING); 
			didps.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
			didps.diph.dwHow = DIPH_DEVICE;
			didps.diph.dwObj = 0;
			if (FAILED(hr = joystick->GetProperty(DIPROP_INSTANCENAME, &didps.diph)))
			{
				ErrorLog("Unable to get name of DirectInput joystick %d (error %d) - skipping joystick.\n", joyNum, hr);

				joystick->Release();
				continue;
			}
			// DInput returns name as Unicode, convert to ASCII
			int len = min<int>(MAX_NAME_LENGTH, (int)wcslen(didps.wsz) + 1);
			WideCharToMultiByte(CP_ACP, 0, didps.wsz, len, joyDetails.name, len, NULL, NULL);
			joyDetails.name[MAX_NAME_LENGTH - 1] = '\0';
			joyDetails.numPOVs = devCaps.dwPOVs;
			joyDetails.numButtons = devCaps.dwButtons;
			
			// Enumerate axes
			DIEnumAxesContext diAxesContext;
			diAxesContext.joyDetails = &joyDetails;
			diAxesContext.enumError = false;
			if (FAILED(hr = joystick->EnumObjects(DI8EnumAxesCallback, &diAxesContext, DIDFT_AXIS)))
			{
				ErrorLog("Unable to enumerate axes of DirectInput joystick %d (error %d) - skipping joystick.\n", joyNum, hr);

				joystick->Release();
				continue;
			}

			// If enumeration failed for some reason then include all possible joystick axes so that no axis is left off due to error
			if (diAxesContext.enumError)
			{
				for (int axisNum = 0; axisNum < NUM_JOY_AXES; axisNum++)
				{
					if (!joyDetails.hasAxis[axisNum])
					{
						joyDetails.hasAxis[axisNum] = true;
						joyDetails.axisHasFF[axisNum] = false;
						char *axisName = joyDetails.axisName[axisNum];
						strcpy(axisName, CInputSystem::GetDefaultAxisName(axisNum));
						strcat(axisName, "-Axis");
					}
				}
			}

			// Count number of axes
			joyDetails.numAxes = 0;
			for (int axisNum = 0; axisNum < NUM_JOY_AXES; axisNum++)
				joyDetails.numAxes += joyDetails.hasAxis[axisNum];
			
			// See if force feedback enabled and is available for joystick 
			if (m_enableFFeedback && (devCaps.dwFlags & DIDC_FORCEFEEDBACK))
			{
				// If so, see what types of effects are available and for which axes
				if (FAILED(hr = joystick->EnumEffects(DI8EnumEffectsCallback, &joyDetails, DIEFT_ALL)))
					ErrorLog("Unable to enumerate effects of DirectInput joystick %d (error %d) - force feedback will be unavailable for joystick.\n", joyNum, hr);
			}

			// Configure axes, if any
			if (joyDetails.numAxes > 0)
			{
				// Set axis range to be from -32768 to 32767
				DIPROPRANGE didpr;
				didpr.diph.dwSize = sizeof(DIPROPRANGE); 
				didpr.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
				didpr.diph.dwHow = DIPH_DEVICE;
				didpr.diph.dwObj = 0;
				didpr.lMin = -32768;
				didpr.lMax = 32767;
				if (FAILED(hr = joystick->SetProperty(DIPROP_RANGE, &didpr.diph)))
				{
					ErrorLog("Unable to set axis range of DirectInput joystick %d (error %d) - skipping joystick.\n", joyNum, hr);

					joystick->Release();
					continue;
				}

				// Set axis mode to absolute
				DIPROPDWORD dipdw;
				dipdw.diph.dwSize = sizeof(DIPROPDWORD); 
				dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER); 
				dipdw.diph.dwHow = DIPH_DEVICE;
				dipdw.diph.dwObj = 0;
				dipdw.dwData = DIPROPAXISMODE_ABS;
				if (FAILED(hr = joystick->SetProperty(DIPROP_AXISMODE, &dipdw.diph)))
				{
					ErrorLog("Unable to set axis mode of DirectInput joystick %d (error %d) - skipping joystick.\n", joyNum, hr);

					joystick->Release();
					continue;
				}

				// Turn off deadzone as handled by this class
				dipdw.dwData = 0;
				if (FAILED(hr = joystick->SetProperty(DIPROP_DEADZONE, &dipdw.diph)))
				{
					ErrorLog("Unable to set deadzone of DirectInput joystick %d (error %d) - skipping joystick.\n", joyNum, hr);

					joystick->Release();
					continue;
				}

				// Turn off saturation as handle by this class
				dipdw.dwData = 10000;
				if (FAILED(hr = joystick->SetProperty(DIPROP_SATURATION, &dipdw.diph)))
				{
					ErrorLog("Unable to set saturation of DirectInput joystick %d (error %d) - skipping joystick.\n", joyNum, hr);

					joystick->Release();
					continue;
				}
			
				// If joystick has force feedback capabilities then disable auto-center
				if (joyDetails.hasFFeedback)
				{
					dipdw.dwData = false;

					if (FAILED(hr = joystick->SetProperty(DIPROP_AUTOCENTER, &dipdw.diph)))
					{
						ErrorLog("Unable to unset auto-center of DirectInput joystick %d (error %d) - force feedback will be unavailable for joystick.\n", joyNum, hr);

						joyDetails.hasFFeedback = false;
					}
				}
			}
		
			// Keep track of DirectInput device number
			it->dInputNum = m_di8Joysticks.size();

			m_di8Joysticks.push_back(joystick);
		}

		// Create initial blank joystick state
		DIJOYSTATE2 joyState;
		memset(&joyState, 0, sizeof(DIJOYSTATE2));
		for (int povNum = 0; povNum < 4; povNum++)
			joyState.rgdwPOV[povNum] = -1;
		
		m_joyDetails.push_back(joyDetails);
		m_diJoyStates.push_back(joyState);
	}
}

void CDirectInputSystem::ActivateJoysticks()
{
	// Set DirectInput cooperative level of joysticks
	unsigned joyNum = 0;
	for (vector<DIJoyInfo>::iterator it = m_diJoyInfos.begin(); it != m_diJoyInfos.end(); it++)
	{
		if (!it->isXInput)
		{	
			LPDIRECTINPUTDEVICE8 joystick = m_di8Joysticks[it->dInputNum];
			joystick->Unacquire();
			if (m_grabMouse)
				joystick->SetCooperativeLevel(m_hwnd, DISCL_EXCLUSIVE | DISCL_FOREGROUND);
			else
				joystick->SetCooperativeLevel(m_hwnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
			joystick->Acquire();
		}
		joyNum++;
	}
}

void CDirectInputSystem::PollJoysticks()
{
	// Get current joystick states from XInput and DirectInput
	int i = 0;
	for (vector<DIJoyInfo>::iterator it = m_diJoyInfos.begin(); it != m_diJoyInfos.end(); it++)
	{
		LPDIJOYSTATE2 pJoyState = &m_diJoyStates[i++];

		HRESULT hr;
		if (it->isXInput)
		{
			// Use XInput to query joystick
			XINPUT_STATE xState;
			memset(&xState, 0, sizeof(XINPUT_STATE));
			if (FAILED(hr = m_xiGetStatePtr(it->xInputNum, &xState)))
			{
				memset(pJoyState, 0, sizeof(DIJOYSTATE2));
				continue;
			}

			// Map XInput state onto joystick's DirectInput state object
			XINPUT_GAMEPAD gamepad = xState.Gamepad;
			pJoyState->lX = (LONG)gamepad.sThumbLX, 
			pJoyState->lY = (LONG)-gamepad.sThumbLY;
			pJoyState->lZ = (LONG)CInputSource::Scale(gamepad.bLeftTrigger, 0, 255, 0, 32767);
			pJoyState->lRx = (LONG)gamepad.sThumbRX;
			pJoyState->lRy = (LONG)-gamepad.sThumbRY;
			pJoyState->lRz = (LONG)CInputSource::Scale(gamepad.bRightTrigger, 0, 255, 0, 32767);
			WORD buttons = gamepad.wButtons;
			int dUp = (buttons & XINPUT_GAMEPAD_DPAD_UP);
			int dDown = (buttons & XINPUT_GAMEPAD_DPAD_DOWN);
			int dLeft = (buttons & XINPUT_GAMEPAD_DPAD_LEFT);
			int dRight = (buttons & XINPUT_GAMEPAD_DPAD_RIGHT);
			if (dUp)
			{
				if      (dLeft)  pJoyState->rgdwPOV[0] = 31500;
				else if (dRight) pJoyState->rgdwPOV[0] = 4500;
				else             pJoyState->rgdwPOV[0] = 0;
			}
			else if (dDown)
			{
				if      (dLeft)  pJoyState->rgdwPOV[0] = 22500;
				else if (dRight) pJoyState->rgdwPOV[0] = 13500;
				else             pJoyState->rgdwPOV[0] = 18000;
			}
			else if (dLeft)  pJoyState->rgdwPOV[0] = 27000;
			else if (dRight) pJoyState->rgdwPOV[0] = 9000;
			else             pJoyState->rgdwPOV[0] = -1;
			pJoyState->rgbButtons[0] = !!(buttons & XINPUT_GAMEPAD_A);
			pJoyState->rgbButtons[1] = !!(buttons & XINPUT_GAMEPAD_B);
			pJoyState->rgbButtons[2] = !!(buttons & XINPUT_GAMEPAD_X);
			pJoyState->rgbButtons[3] = !!(buttons & XINPUT_GAMEPAD_Y);
			pJoyState->rgbButtons[4] = !!(buttons & XINPUT_GAMEPAD_LEFT_SHOULDER);
			pJoyState->rgbButtons[5] = !!(buttons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
			pJoyState->rgbButtons[6] = !!(buttons & XINPUT_GAMEPAD_BACK);
			pJoyState->rgbButtons[7] = !!(buttons & XINPUT_GAMEPAD_START);
			pJoyState->rgbButtons[8] = !!(buttons & XINPUT_GAMEPAD_LEFT_THUMB);
			pJoyState->rgbButtons[9] = !!(buttons & XINPUT_GAMEPAD_RIGHT_THUMB);
		}
		else
		{
			// Use DirectInput to query joystick
			LPDIRECTINPUTDEVICE8 joystick = m_di8Joysticks[it->dInputNum];
			if (FAILED(hr = joystick->Poll()))
			{
				hr = joystick->Acquire();
				while (hr == DIERR_INPUTLOST)
					hr = joystick->Acquire();

				if (hr == DIERR_OTHERAPPHASPRIO || hr == DIERR_INVALIDPARAM || hr == DIERR_NOTINITIALIZED)
				{
					memset(pJoyState, 0, sizeof(DIJOYSTATE2));
					continue;
				}
			}
		
			// Update joystick's DirectInput state
			joystick->GetDeviceState(sizeof(DIJOYSTATE2), pJoyState);
		}
	}
}

void CDirectInputSystem::CloseJoysticks()
{
	// Release any DirectInput force feedback effects that were created
	for (vector<DIJoyInfo>::iterator it = m_diJoyInfos.begin(); it != m_diJoyInfos.end(); it++)
	{
		for (unsigned axisNum = 0; axisNum < NUM_JOY_AXES; axisNum++)
		{
			for (unsigned effNum = 0; effNum < NUM_FF_EFFECTS; effNum++)
			{
				if (it->dInputEffects[axisNum][effNum] != NULL)
				{
					it->dInputEffects[axisNum][effNum]->Release();
					it->dInputEffects[axisNum][effNum] = NULL;
				}
			}
		}
	}

	// Release each DirectInput joystick
	for (vector<LPDIRECTINPUTDEVICE8>::iterator it = m_di8Joysticks.begin(); it != m_di8Joysticks.end(); it++)
	{
		(*it)->Unacquire();
		(*it)->Release();
	}

	m_joyDetails.clear();
	m_diJoyInfos.clear();
	m_diJoyStates.clear();
	m_di8Joysticks.clear();
}

HRESULT CDirectInputSystem::CreateJoystickEffect(LPDIRECTINPUTDEVICE8 joystick, int axisNum, ForceFeedbackCmd ffCmd, LPDIRECTINPUTEFFECT *pEffect)
{
	// Map axis number to DI object offset
	DWORD axisOfs;
	switch (axisNum)
	{
		case AXIS_X:  axisOfs = DIJOFS_X; break;
		case AXIS_Y:  axisOfs = DIJOFS_Y; break;
		case AXIS_Z:  axisOfs = DIJOFS_Z; break;
		case AXIS_RX: axisOfs = DIJOFS_RX; break;
		case AXIS_RY: axisOfs = DIJOFS_RY; break;
		case AXIS_RZ: axisOfs = DIJOFS_RZ; break;
		default:      return E_FAIL;
	}

	DWORD dwAxis = axisOfs;
	LONG lDirection = 0;
	DICONSTANTFORCE dicf;
	DICONDITION dic;
	DIPERIODIC dip;
	DIENVELOPE die;
	GUID guid;

	// Set common effects parameters
	DIEFFECT eff;
	memset(&eff, 0, sizeof(eff));
	eff.dwSize = sizeof(DIEFFECT);
	eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
	eff.dwTriggerButton = DIEB_NOTRIGGER;
	eff.dwTriggerRepeatInterval = 0;
	eff.dwGain = DI_FFNOMINALMAX;
	eff.cAxes = 1;
	eff.rgdwAxes = &dwAxis;
	eff.rglDirection = &lDirection;
	eff.dwDuration = INFINITE;
	eff.dwStartDelay = 0;
	eff.lpEnvelope = NULL;
	
	// Set specific effects parameters
	switch (ffCmd.id)
	{
		case FFStop:
			return E_FAIL;

		case FFConstantForce:
			guid = GUID_ConstantForce;

			dicf.lMagnitude = 0;
			
			eff.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
			eff.lpvTypeSpecificParams = &dicf;
			break;

		case FFSelfCenter:
			guid = GUID_Spring;

			dic.lOffset = 0; // offset is +ve/-ve bias, 0 = evenly spread in both directions
			dic.lPositiveCoefficient = 0;
			dic.lNegativeCoefficient = 0;
			dic.dwPositiveSaturation = DI_FFNOMINALMAX;
			dic.dwNegativeSaturation = DI_FFNOMINALMAX;
			dic.lDeadBand = (LONG)(0.05 * DI_FFNOMINALMAX);  // 5% deadband
			
			eff.cbTypeSpecificParams = sizeof(DICONDITION);
			eff.lpvTypeSpecificParams = &dic;
			break;

		case FFFriction:
			guid = GUID_Friction;
	
			dic.lOffset = 0;
			dic.lPositiveCoefficient = 0;
			dic.lNegativeCoefficient = 0;
			dic.dwPositiveSaturation = DI_FFNOMINALMAX;
			dic.dwNegativeSaturation = DI_FFNOMINALMAX;
			dic.lDeadBand = 0; // 0% deadband
			
			eff.cbTypeSpecificParams = sizeof(DICONDITION);
			eff.lpvTypeSpecificParams = &dic;
			break;

		case FFVibrate:
			guid = GUID_Sine;

			dip.dwMagnitude = 0;
			dip.lOffset = 0;
            dip.dwPhase = 0;
            dip.dwPeriod = (DWORD)(0.05 * DI_SECONDS); // 1/20th second
				
			eff.cbTypeSpecificParams = sizeof(DIPERIODIC);
			eff.lpvTypeSpecificParams = &dip;
			break;
	}

	joystick->Acquire();

	HRESULT hr;
	if (FAILED(hr = joystick->CreateEffect(guid, &eff, pEffect, NULL)))
		return hr;
	if (*pEffect == NULL)
		return E_FAIL;
	(*pEffect)->Start(1, 0);
	return S_OK;
}

bool CDirectInputSystem::InitializeSystem()
{
	if (m_useRawInput)
	{
		// Dynamically load RawInput API
		HMODULE user32 = LoadLibrary(TEXT("user32.dll"));
		if (user32 != NULL)
		{
			m_getRIDevListPtr = (GetRawInputDeviceListPtr)GetProcAddress(user32, "GetRawInputDeviceList");
			m_getRIDevInfoPtr = (GetRawInputDeviceInfoPtr)GetProcAddress(user32, "GetRawInputDeviceInfoA");
			m_regRIDevsPtr = (RegisterRawInputDevicesPtr)GetProcAddress(user32, "RegisterRawInputDevices");
			m_getRIDataPtr = (GetRawInputDataPtr)GetProcAddress(user32, "GetRawInputData");
			m_useRawInput = m_getRIDevListPtr != NULL && m_getRIDevInfoPtr != NULL && m_regRIDevsPtr != NULL && m_getRIDataPtr != NULL;
		}
		else
			m_useRawInput = false;

		if (m_useRawInput)
		{
			// Get screen resolution (needed for absolute mouse devices)
			DEVMODEA settings;
			if (!EnumDisplaySettings(NULL, ENUM_CURRENT_SETTINGS, &settings))
			{
				ErrorLog("Unable to read current display settings\n");
				return false;
			}
			m_screenW = settings.dmPelsWidth;
			m_screenH = settings.dmPelsHeight;
		}
		else
			ErrorLog("Unable to initialize RawInput API (library hooks are not available) - switching to DirectInput.\n");
	}

	if (m_useXInput)
	{
		// Dynamically load XInput API
		HMODULE xInput = LoadLibrary(TEXT(XINPUT_DLL_A));
		if (xInput != NULL)
		{
			m_xiGetCapabilitiesPtr = (XInputGetCapabilitiesPtr)GetProcAddress(xInput, "XInputGetCapabilities");
			m_xiGetStatePtr = (XInputGetStatePtr)GetProcAddress(xInput, "XInputGetState");
			m_xiSetStatePtr = (XInputSetStatePtr)GetProcAddress(xInput, "XInputSetState");
			m_useXInput = m_xiGetCapabilitiesPtr != NULL && m_xiGetStatePtr != NULL && m_xiSetStatePtr != NULL;
		}
		else
			m_useXInput = false;

		if (!m_useXInput)
			ErrorLog("Unable to initialize XInput API (library hooks are not available) - switching to DirectInput.\n");
	}

	// Dynamically create DirectInput8 via COM, rather than statically linking to dinput8.dll
	// TODO - if fails, try older versions of DirectInput
	HRESULT hr;
	if (SUCCEEDED(hr = CoInitialize(NULL)))
		m_initializedCOM = true;
	else
	{
		// CoInitialize fails if called from managed context (ie .NET debugger) so check for this and ignore this error
		if (hr != RPC_E_CHANGED_MODE)
		{
			ErrorLog("Unable to initialize COM (error %d).\n", hr);

			return false;
		}
	}
	if (FAILED(hr = CoCreateInstance(CLSID_DirectInput8, NULL, CLSCTX_INPROC_SERVER, IID_IDirectInput8A, (LPVOID*)&m_di8)))
	{
		ErrorLog("Unable to initialize DirectInput API (error %d) - is DirectX 8 or later installed?\n", hr);
		
		if (m_initializedCOM)
			CoUninitialize();
		return false;
	}
	if (FAILED(hr = m_di8->Initialize(GetModuleHandle(NULL), DIRECTINPUT_VERSION)))
	{
		ErrorLog("Unable to initialize DirectInput API (error %d) - is DirectX 8 or later installed?\n", hr);

		m_di8->Release();
		m_di8 = NULL;
		if (m_initializedCOM)
			CoUninitialize();
		return false;
	}

	// Open all devices
	OpenKeyboardsAndMice();
	OpenJoysticks();

	return true;
}

int CDirectInputSystem::GetKeyIndex(const char *keyName)
{
	for (int i = 0; i < NUM_DI_KEYS; i++)
	{
		if (stricmp(keyName, s_keyMap[i].keyName) == 0)
			return i;
	}
	return -1;
}

const char *CDirectInputSystem::GetKeyName(int keyIndex)
{
	if (keyIndex < 0 || keyIndex >= NUM_DI_KEYS)
		return NULL;
	return s_keyMap[keyIndex].keyName;
}

bool CDirectInputSystem::IsKeyPressed(int kbdNum, int keyIndex)
{
	// Get DI key code (scancode) for given key index
	int diKey = s_keyMap[keyIndex].diKey;
	
	if (m_useRawInput)
	{
		// For RawInput, check if key is currently pressed for given keyboard number
		bool *keyState = m_rawKeyStates[kbdNum];
		return !!keyState[diKey];
	}

	// For DirectInput, just check common keyboard state
	return !!(m_diKeyState[diKey] & 0x80);
}

int CDirectInputSystem::GetMouseAxisValue(int mseNum, int axisNum)
{
	if (m_useRawInput)
	{
		// For RawInput, get combined or individual mouse state and return value for given axis
		// The cursor is always hidden when using RawInput, so it does not matter if these values don't match with the cursor (with multiple
		// mice the cursor is irrelevant anyway)
		RawMseState *pMseState = (mseNum == ANY_MOUSE ? &m_combRawMseState : &m_rawMseStates[mseNum]);
		switch (axisNum)
		{
			case AXIS_X: return pMseState->x;
			case AXIS_Y: return pMseState->y;
			case AXIS_Z: return pMseState->z;
			default:     return 0;
		}
	}
	
	// For DirectInput, for X- and Y-axes just use cursor position within window if available (so that mouse movements sync with the cursor)
	if (axisNum == AXIS_X || axisNum == AXIS_Y)
	{
		POINT p;
		if (GetCursorPos(&p) && ScreenToClient(m_hwnd, &p))
			return (axisNum == AXIS_X ? p.x : p.y);
	}

	// Otherwise, return the raw DirectInput axis values
	switch (axisNum)
	{
		case AXIS_X: return m_diMseState.x;
		case AXIS_Y: return m_diMseState.y;
		case AXIS_Z: return m_diMseState.z;
		default:     return 0;
	}
}

int CDirectInputSystem::GetMouseWheelDir(int mseNum)
{
	if (m_useRawInput)
	{
		// For RawInput, return the wheel value for combined or individual mouse state
		return (mseNum == ANY_MOUSE ? m_combRawMseState.wheelDir : m_rawMseStates[mseNum].wheelDir);
	}

	// For DirectInput just return the common wheel value
	return m_diMseState.wheelDir;
}

bool CDirectInputSystem::IsMouseButPressed(int mseNum, int butNum)
{
	if (m_useRawInput)
	{
		// For RawInput, return the button state for combined or individual mouse state
		return !!((mseNum == ANY_MOUSE ? m_combRawMseState.buttons : m_rawMseStates[mseNum].buttons) & (1<<butNum));
	}
	
	// For DirectInput just return the common button state (taking care with the middle and right mouse buttons
	// which DirectInput numbers 2 and 1 respectively, rather than 1 and 2)
	if      (butNum == 1) butNum = 2;
	else if (butNum == 2) butNum = 1;
	return (butNum < 5 ? !!(m_diMseState.buttons[butNum] & 0x80) : false);
}

int CDirectInputSystem::GetJoyAxisValue(int joyNum, int axisNum)
{
	// Return raw value for given joystick number and axis (values range from -32768 to 32767)
	switch (axisNum)
	{
		case AXIS_X:  return (int)m_diJoyStates[joyNum].lX;
		case AXIS_Y:  return (int)m_diJoyStates[joyNum].lY;
		case AXIS_Z:  return (int)m_diJoyStates[joyNum].lZ;
		case AXIS_RX: return (int)m_diJoyStates[joyNum].lRx;
		case AXIS_RY: return (int)m_diJoyStates[joyNum].lRy;
		case AXIS_RZ: return (int)m_diJoyStates[joyNum].lRz;
		default:      return 0;
	}
}

bool CDirectInputSystem::IsJoyPOVInDir(int joyNum, int povNum, int povDir)
{
	// Check if POV-hat value for given joystick number and POV is pointing in required direction
	int povVal = m_diJoyStates[joyNum].rgdwPOV[povNum] / 100;   // DirectInput value is angle of POV-hat in 100ths of a degree
	switch (povDir)
	{
		case POV_UP:    return povVal == 315 || povVal == 0 || povVal == 45;
		case POV_DOWN:  return povVal == 135 || povVal == 180 || povVal == 225;
		case POV_RIGHT: return povVal == 45 || povVal == 90 || povVal == 135;
		case POV_LEFT:  return povVal == 225 || povVal == 270 || povVal == 315;
		default:        return false;
	}
}

bool CDirectInputSystem::IsJoyButPressed(int joyNum, int butNum)
{
	// Get joystick state for given joystick and return current button value for given button number
	return !!m_diJoyStates[joyNum].rgbButtons[butNum];
}

bool CDirectInputSystem::ProcessForceFeedbackCmd(int joyNum, int axisNum, ForceFeedbackCmd ffCmd)
{
	DIJoyInfo *pInfo = &m_diJoyInfos[joyNum];

	HRESULT hr;
	if (pInfo->isXInput)
	{	
		if (axisNum != AXIS_X && axisNum != AXIS_Y && axisNum != AXIS_RX && axisNum != AXIS_RY)
			return false;
		XINPUT_VIBRATION vibration;
		bool negForce;
		float absForce;
		float threshold;
		switch (ffCmd.id)
		{
			case FFStop:
				// Stop command halts all vibration
				pInfo->xiConstForceLeft = 0;
				pInfo->xiConstForceRight = 0;
				pInfo->xiVibrateBoth = 0;
				break;

			case FFConstantForce:
				// Check if constant force effect is disabled
				if (g_Config.xInputConstForceMax == 0)
					return false;
				// Constant force effect is mapped to either left or right vibration motor depending on its direction
				negForce = ffCmd.force < 0.0f;
				absForce = (negForce ? -ffCmd.force : ffCmd.force);
				threshold = (float)g_Config.xInputConstForceThreshold / 100.0f;
				// Check if constant force effect is being stopped or is below threshold 
				if (absForce == 0.0f || absForce < threshold)
				{
					// If so, stop vibration due to force effect
					pInfo->xiConstForceLeft = 0;
					pInfo->xiConstForceRight = 0;
				}
				else if (negForce)
				{
					// If force is negative (to left), set left motor vibrating
					pInfo->xiConstForceLeft = (WORD)(absForce * (float)(g_Config.xInputConstForceMax * XI_VIBRATE_SCALE));
					pInfo->xiConstForceRight = 0;
				}
				else
				{
					// If force positive (to right), set right motor vibrating
					pInfo->xiConstForceLeft = 0;
					pInfo->xiConstForceRight = (WORD)(absForce * (float)(g_Config.xInputConstForceMax * XI_VIBRATE_SCALE));
				}
				break;

			case FFSelfCenter:
			case FFFriction:
				// Self center and friction effects are not mapped
				return false;

			case FFVibrate:
				// Check if vibration effect is disabled
				if (g_Config.xInputVibrateMax == 0)
					return false;
				// Check if vibration effect is being stopped
				if (ffCmd.force == 0.0f)
				{
					// If so, stop vibration due to vibration effect
					pInfo->xiVibrateBoth = 0;
				}
				else
				{
					// Otherwise, set both motors vibrating
					pInfo->xiVibrateBoth = (WORD)(ffCmd.force * (float)(g_Config.xInputVibrateMax * XI_VIBRATE_SCALE));
				}
				break;

			default:
				// Unknown feedback command
				return false;
		}

		// Combine vibration speeds from both constant force effect and vibration effect and set motors in action
		vibration.wLeftMotorSpeed = min<WORD>(pInfo->xiConstForceLeft + pInfo->xiVibrateBoth, XI_VIBRATE_MAX);
		vibration.wRightMotorSpeed = min<WORD>(pInfo->xiConstForceRight + pInfo->xiVibrateBoth, XI_VIBRATE_MAX);
		return SUCCEEDED(hr = m_xiSetStatePtr(pInfo->xInputNum, &vibration));
	}
	else
	{
		LPDIRECTINPUTDEVICE8 joystick = m_di8Joysticks[pInfo->dInputNum];
		
		// See if command is to stop all force feedback, if so send appropriate command
		if (ffCmd.id == FFStop)
			return SUCCEEDED(hr = joystick->SendForceFeedbackCommand(DISFFC_STOPALL));

		// Create effect for given axis if has not already been created
		int effNum = (int)ffCmd.id;
		LPDIRECTINPUTEFFECT *pEffect = &pInfo->dInputEffects[axisNum][effNum];
		if ((*pEffect) == NULL)
		{
			if (FAILED(hr = CreateJoystickEffect(joystick, axisNum, ffCmd, pEffect)))
				return false;
		}
		
		LONG lDirection = 0;
		DICONSTANTFORCE dicf;
		DICONDITION dic;
		DIPERIODIC dip;
		DIENVELOPE die;
				
		// Set common parameters
		DIEFFECT eff;
		memset(&eff, 0, sizeof(eff));
		eff.dwSize = sizeof(DIEFFECT);
		eff.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
		eff.cAxes = 1;
		eff.rglDirection = &lDirection;
		eff.dwStartDelay = 0;
		eff.lpEnvelope = NULL;
				
		// Set command specific parameters
		switch (ffCmd.id)	
		{
			case FFConstantForce:
				//printf("FFConstantForce %0.2f\n", 100.0f * ffCmd.force);
				if (g_Config.dInputConstForceMax == 0)
					return false;
				dicf.lMagnitude = min<LONG>(-ffCmd.force * (float)(g_Config.dInputConstForceMax * DI_EFFECTS_SCALE), DI_EFFECTS_MAX); // Invert sign for DirectInput effect
				
				eff.cbTypeSpecificParams = sizeof(DICONSTANTFORCE);
				eff.lpvTypeSpecificParams = &dicf;
				break;

			case FFSelfCenter:
				//printf("FFSelfCenter %0.2f\n", 100.0f * ffCmd.force);
				if (g_Config.dInputSelfCenterMax == 0)
					return false;
				dic.lOffset = 0;
				dic.lPositiveCoefficient = min<LONG>(ffCmd.force * (float)(g_Config.dInputSelfCenterMax * DI_EFFECTS_SCALE), DI_EFFECTS_MAX);
				dic.lNegativeCoefficient = min<LONG>(ffCmd.force * (float)(g_Config.dInputSelfCenterMax * DI_EFFECTS_SCALE), DI_EFFECTS_MAX);
				dic.dwPositiveSaturation = DI_FFNOMINALMAX;
				dic.dwNegativeSaturation = DI_FFNOMINALMAX;
				dic.lDeadBand = (LONG)(0.05 * DI_FFNOMINALMAX);

				eff.cbTypeSpecificParams = sizeof(DICONDITION);
				eff.lpvTypeSpecificParams = &dic;
				break;

			case FFFriction:
				//printf("FFFriction %0.2f\n", 100.0f * ffCmd.force);
				if (g_Config.dInputFrictionMax == 0)
					return false;
				dic.lOffset = 0;
				dic.lPositiveCoefficient = min<LONG>(ffCmd.force * (float)(g_Config.dInputFrictionMax * DI_EFFECTS_SCALE), DI_EFFECTS_MAX);
				dic.lNegativeCoefficient = min<LONG>(ffCmd.force * (float)(g_Config.dInputFrictionMax * DI_EFFECTS_SCALE), DI_EFFECTS_MAX);
				dic.dwPositiveSaturation = DI_FFNOMINALMAX;
				dic.dwNegativeSaturation = DI_FFNOMINALMAX;
				dic.lDeadBand = 0;

				eff.cbTypeSpecificParams = sizeof(DICONDITION);
				eff.lpvTypeSpecificParams = &dic;
				break;

			case FFVibrate:
				//printf("FFVibrate %0.2f\n", 100.0f * ffCmd.force);
				if (g_Config.dInputVibrateMax == 0)
					return false;
				dip.dwMagnitude = min<DWORD>(ffCmd.force * (float)(g_Config.dInputVibrateMax * DI_EFFECTS_SCALE), DI_EFFECTS_MAX);
				dip.lOffset = 0;
                dip.dwPhase = 0;
                dip.dwPeriod = (DWORD)(0.05 * DI_SECONDS); // 1/20th second

				eff.cbTypeSpecificParams = sizeof(DIPERIODIC);
				eff.lpvTypeSpecificParams = &dip;
				break;

			default:
				// Unknown feedback command
				return false;
		}

		// Set the new parameters and start effect immediately
		return SUCCEEDED(hr = (*pEffect)->SetParameters(&eff, DIEP_DIRECTION | DIEP_TYPESPECIFICPARAMS | DIEP_START));
	}
}

void CDirectInputSystem::Wait(int ms)
{
	Sleep(ms);
}

bool CDirectInputSystem::ConfigMouseCentered()
{
	// When checking if mouse centered, use system cursor rather than raw values (otherwise user's mouse movements won't match up
	// with onscreen cursor during configuration)
	POINT p;
	if (!GetCursorPos(&p) || !ScreenToClient(m_hwnd, &p))
		return false;
	
	// See if mouse in center of display
	unsigned lx = m_dispX + m_dispW / 4;
	unsigned ly = m_dispY + m_dispH / 4;
	if (p.x < (LONG)lx || p.x > (LONG)(lx + m_dispW / 2) || p.y < (LONG)ly || p.y > (LONG)(ly + m_dispH / 2))
		return false;

	// Once mouse has been centered, sync up mice raw values with current cursor position so that movements are detected correctly
	ResetMice();
	return true;
}

CInputSource *CDirectInputSystem::CreateAnyMouseSource(EMousePart msePart)
{
	// If using RawInput, create a mouse source that uses the combined mouse state m_combRawState, rather than combining all the individual mouse
	// sources in the default manner
	if (m_useRawInput)
		return CreateMouseSource(ANY_MOUSE, msePart);

	return CInputSystem::CreateAnyMouseSource(msePart);
}

int CDirectInputSystem::GetNumKeyboards()
{
	// If RawInput enabled, then return number of keyboards found.  Otherwise, return ANY_KEYBOARD as DirectInput cannot handle multiple keyboards
	return (m_useRawInput ? m_rawKeyboards.size() : ANY_KEYBOARD);
}
	
int CDirectInputSystem::GetNumMice()
{
	// If RawInput enabled, then return number of mice found.  Otherwise, return ANY_MOUSE as DirectInput cannot handle multiple keyboards
	return (m_useRawInput ? m_rawMice.size() : ANY_MOUSE);
}
	
int CDirectInputSystem::GetNumJoysticks()
{
	// Return number of joysticks found
	return m_diJoyInfos.size();
}

const KeyDetails *CDirectInputSystem::GetKeyDetails(int kbdNum)
{
	// If RawInput enabled, then return details for given keyboard.  Otherwise, return NULL as DirectInput cannot handle multiple keyboards
	return (m_useRawInput ? &m_keyDetails[kbdNum] : NULL);
}

const MouseDetails *CDirectInputSystem::GetMouseDetails(int mseNum)
{
	// If RawInput enabled, then return details of given mouse.  Otherwise, return NULL as DirectInput cannot handle multiple keyboards
	return (m_useRawInput ? &m_mseDetails[mseNum] : NULL);
}

const JoyDetails *CDirectInputSystem::GetJoyDetails(int joyNum)
{
	return &m_joyDetails[joyNum];
}

bool CDirectInputSystem::Poll()
{
	// See if keyboard, mice and joysticks have been activated yet
	if (!m_activated)
	{
		// If not, then get Window handle of SDL window
		SDL_SysWMinfo info;
        SDL_VERSION(&info.version);
        if (SDL_GetWMInfo(&info)) 
			m_hwnd = info.window;
		
		// Tell SDL to pass on all Windows events
		// Removed - see below
		//SDL_EventState(SDL_SYSWMEVENT, SDL_ENABLE);

		// Activate the devices now that a Window handle is available
		ActivateKeyboardsAndMice();
		ActivateJoysticks();

		m_activated = true;
	}

	// Wait or poll for event from SDL
	// Removed - see below
	/*
	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		if (e.type == SDL_QUIT)
			return false; 	
		else if (e.type == SDL_SYSWMEVENT)
		{
			SDL_SysWMmsg *wmMsg = e.syswm.msg;
			ProcessWMEvent(wmMsg->hwnd, wmMsg->msg, wmMsg->wParam, wmMsg->lParam);
		}
	}*/

	// Wait or poll for event on Windows message queue (done this way instead of using SDL_PollEvent as above because
	// for some reason this causes RawInput HRAWINPUT handles to arrive stale.  Not sure what SDL_PollEvent is doing to cause this
	// but the following code can replace it without any problems as it is effectively what SDL_PollEvent does anyway)
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
	{
		int ret = GetMessage(&msg, NULL, 0, 0);
		if (ret == 0)
			return false;
		else if (ret > 0)
		{
			TranslateMessage(&msg);

			// Handle RawInput messages here
			if (m_useRawInput && msg.message == WM_INPUT)
				ProcessRawInput((HRAWINPUT)msg.lParam);

			// Propagate all messages to default (SDL) handlers
			DispatchMessage(&msg);
		}
	}

	// Poll keyboards, mice and joysticks
	PollKeyboardsAndMice();
	PollJoysticks();

	return true;
}

void CDirectInputSystem::SetMouseVisibility(bool visible)
{
	if (m_useRawInput)
		ShowCursor(!m_grabMouse && visible ? TRUE : FALSE);
	else
		ShowCursor(visible ? TRUE : FALSE);
}

void CDirectInputSystem::GrabMouse()
{
	CInputSystem::GrabMouse();

	if (m_useRawInput)
		SetMouseVisibility(false);

	// When grabbing mouse, make sure devices get re-activated
	if (m_activated)
	{
		ActivateKeyboardsAndMice();
		ActivateJoysticks();
	}
}

void CDirectInputSystem::UngrabMouse()
{
	CInputSystem::UngrabMouse();

	// When ungrabbing mouse, make sure devices get re-activated
	if (m_activated)
	{
		ActivateKeyboardsAndMice();
		ActivateJoysticks();
	}
}
