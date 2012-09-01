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
  * Input.cpp
  *
  * Implementation of CInput, the base input class. Input types are derived
  * from this.
  */

#include "Supermodel.h"

CInput::CInput(const char *inputId, const char *inputLabel, unsigned inputFlags, unsigned inputGameFlags, const char *defaultMapping, UINT16 initValue) : 
	id(inputId), label(inputLabel), flags(inputFlags), gameFlags(inputGameFlags), m_defaultMapping(defaultMapping), value(initValue), prevValue(initValue),
	m_system(NULL), m_source(NULL)
{
	ResetToDefaultMapping();
}

CInput::~CInput()
{
	// Release source, if any
	if (m_source != NULL)
		m_source->Release();
}

void CInput::CreateSource()
{
	// If already have a source, then release it now
	if (m_source != NULL)
		m_source->Release();

	// If no system set yet or mapping is empty or NONE, then set source to NULL
	if (m_system == NULL || m_mapping[0] == '\0' || stricmp(m_mapping, "NONE") == 0)
		m_source = NULL;
	else
	{
		// Otherwise, ask system to parse mapping into appropriate input source
		m_source = m_system->ParseSource(m_mapping, !!(flags & INPUT_FLAGS_AXIS));

		// Check that mapping was parsed okay and if so acquire it
		if (m_source != NULL)
			m_source->Acquire();
		else
		{
			// Otherwise, fall back to default mapping
			if (stricmp(m_mapping, m_defaultMapping) != 0)
			{
				ErrorLog("Unable to map input %s to [%s] - switching to default [%s].\n", id, m_mapping, m_defaultMapping);

				ResetToDefaultMapping();
			}
		}
	}
}

void CInput::Initialize(CInputSystem *system)
{
	m_system = system;

	CreateSource();
}

const char* CInput::GetInputGroup()
{
	switch (gameFlags)
	{
		case GAME_INPUT_UI:              return "User Interface Controls";
		case GAME_INPUT_COMMON:          return "Common Controls";
		case GAME_INPUT_JOYSTICK1:       // Fall through to below
		case GAME_INPUT_JOYSTICK2:       return "4-Way Joysticks";
		case GAME_INPUT_FIGHTING:        return "Fighting Game Buttons";
		case GAME_INPUT_SPIKEOUT:		 return "Spikeout Buttons";
		case GAME_INPUT_SOCCER:          return "Virtua Striker Buttons";
		case GAME_INPUT_VEHICLE:         return "Racing Game Steering Controls";
		case GAME_INPUT_SHIFT4:          return "Racing Game Gear Shift";
		case GAME_INPUT_VR:              return "Racing Game VR View Buttons";
		case GAME_INPUT_RALLY:           return "Miscellaneous Driving Game Buttons";
		case GAME_INPUT_TWIN_JOYSTICKS:  return "Virtual On Controls";
		case GAME_INPUT_ANALOG_JOYSTICK: return "Analog Joystick";
		case GAME_INPUT_GUN1:            // Fall through to below
		case GAME_INPUT_GUN2:            return "Lightguns";
		default:                         return "Misc";
	}
}

const char *CInput::GetMapping()
{
	return m_mapping;
}

void CInput::ClearMapping()
{
	SetMapping("NONE");
}

void CInput::SetMapping(const char *mapping)
{
	strncpy(m_mapping, mapping, MAX_MAPPING_LENGTH);
	m_mapping[MAX_MAPPING_LENGTH] = '\0';
	CreateSource();
}

void CInput::AppendMapping(const char *mapping)
{
	// If mapping is empty or NONE, then simply set mapping
	if (m_mapping[0] == '\0' || stricmp(m_mapping, "NONE") == 0)
		SetMapping(mapping);
	else
	{
		// Otherwise, append to mapping string and recreate source from new mapping string
		int size = MAX_MAPPING_LENGTH - strlen(m_mapping);
		strncat(m_mapping, ",", size--);
		strncat(m_mapping, mapping, size);
		CreateSource();
	}
}

void CInput::ResetToDefaultMapping()
{
	SetMapping(m_defaultMapping);
}

void CInput::ReadFromINIFile(CINIFile *ini, const char *section)
{
	// See if input is configurable
	if (IsConfigurable())
	{
		// If so, check INI file for mapping string
		string key("Input");
		key.append(id);
		string mapping;
		if (ini->Get(section, key, mapping) == OKAY)
		{
			// If found, then set mapping string
			SetMapping(mapping.c_str());
			return;
		}
	}

	// If input has not been configured, then force recreation of source anyway since input system settings may have changed 
	CreateSource();
}

void CInput::WriteToINIFile(CINIFile *ini, const char *section)
{
	if (!IsConfigurable())
		return;
	string key("Input");
	key.append(id);
	ini->Set(section, key, m_mapping);
}

void CInput::InputSystemChanged()
{
	// If input system or its settings have changed, then force recreation of source
	CreateSource();
}

bool CInput::Configure(bool append, const char *escapeMapping)
{
	char mapping[MAX_MAPPING_LENGTH];
	if (!m_system->ReadMapping(mapping, MAX_MAPPING_LENGTH, !!(flags & INPUT_FLAGS_AXIS), READ_ALL, escapeMapping))
		return false;
	if (append)
		AppendMapping(mapping);
	else
		SetMapping(mapping);
	return true;
}

bool CInput::Changed()
{
	return value != prevValue;
}

bool CInput::SendForceFeedbackCmd(ForceFeedbackCmd ffCmd)
{
	if (m_source == NULL)
		return false;
	return m_source->SendForceFeedbackCmd(ffCmd);
}