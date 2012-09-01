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
 * InputTypes.cpp
 *
 * The different input subclasses.  They are grouped together here as they are 
 * all very simple classes.
 */

#include "Supermodel.h"

/*
 * CSwitchInput
 */
CSwitchInput::CSwitchInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, UINT16 offVal, UINT16 onVal) : 
	CInput(inputId, inputLabel, INPUT_FLAGS_SWITCH, inputGameFlags, defaultMapping), m_offVal(offVal), m_onVal(onVal)
{
	//
}

void CSwitchInput::Poll()
{
	prevValue = value;

	bool boolValue = !!value;
	if (m_source != NULL && m_source->GetValueAsSwitch(boolValue))
		value = (boolValue ? m_onVal : m_offVal);
	else
		value = m_offVal;
}

bool CSwitchInput::Pressed()
{
	return prevValue == m_offVal && value == m_onVal;
}

bool CSwitchInput::Released()
{
	return prevValue == m_onVal && value == m_offVal;
}

/*
 * CAnalogInput
 */
CAnalogInput::CAnalogInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, UINT16 minVal, UINT16 maxVal) : 
	CInput(inputId, inputLabel, INPUT_FLAGS_ANALOG, inputGameFlags, defaultMapping, minVal), m_minVal(minVal), m_maxVal(maxVal) 
{
	//
}

void CAnalogInput::Poll()
{
	prevValue = value;

	if (m_source == NULL)
	{
		value = m_minVal;
		return;
	}
	int intValue = value;
	if (m_source->GetValueAsAnalog(intValue, m_minVal, m_minVal, m_maxVal))
		value = intValue;
	else
		value = m_minVal;
}

bool CAnalogInput::HasValue()
{
	return value > m_minVal;
}

double CAnalogInput::ValueAsFraction()
{
	double frac = (double)(value - m_minVal)/(double)(m_maxVal - m_minVal);
	return (frac >= 0.0 ? frac : -frac);
}

/*
 * CAxisInput
 */
CAxisInput::CAxisInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags, const char *defaultMapping, 
	CAnalogInput *negInput, CAnalogInput *posInput,	UINT16 minVal, UINT16 offVal, UINT16 maxVal) : 
	CInput(inputId, inputLabel, INPUT_FLAGS_AXIS, inputGameFlags, defaultMapping, offVal), m_negInput(negInput), m_posInput(posInput), 
		m_minVal(minVal), m_offVal(offVal), m_maxVal(maxVal)
{
	//
}

void CAxisInput::Poll()
{
	prevValue = value;

	// Try getting value from analog inputs that represent negative and positive range of the axis first and then try the default input source
	int intValue = value;
	if ((m_negInput != NULL && m_negInput->HasValue()) || (m_posInput != NULL && m_posInput->HasValue()))
	{
		if (m_maxVal > m_minVal)
		{
			value = m_offVal;
			if (m_posInput != NULL) value += (int)(m_posInput->ValueAsFraction() * (double)(m_maxVal - m_offVal));
			if (m_negInput != NULL) value -= (int)(m_negInput->ValueAsFraction() * (double)(m_offVal - m_minVal));
		}
		else
		{ 
			value = m_offVal;
			if (m_posInput != NULL) value += (int)(m_posInput->ValueAsFraction() * (double)(m_offVal - m_maxVal));
			if (m_negInput != NULL) value -= (int)(m_negInput->ValueAsFraction() * (double)(m_minVal - m_offVal));
		}
	}
	else if (m_source != NULL && m_source->GetValueAsAnalog(intValue, m_minVal, m_offVal, m_maxVal))
		value = intValue;
	else 
		value = m_offVal;
}

bool CAxisInput::HasValue()
{
	return value != m_offVal;
}

double CAxisInput::ValueAsFraction()
{
	double frac = (double)(value - m_minVal)/(double)(m_maxVal - m_minVal);
	return (frac >= 0.0 ? frac : -frac);
}

/*
 * CGearShift4Input
 */
CGearShift4Input::CGearShift4Input(const char *inputId, const char *inputLabel, unsigned inputGameFlags,
		CSwitchInput *shift1Input, CSwitchInput *shift2Input, CSwitchInput *shift3Input, CSwitchInput *shift4Input, CSwitchInput *shiftNInput,
		CSwitchInput *shiftUpInput, CSwitchInput *shiftDownInput) : 
	CInput(inputId, inputLabel, INPUT_FLAGS_VIRTUAL, inputGameFlags),
		m_shift1Input(shift1Input), m_shift2Input(shift2Input), m_shift3Input(shift3Input), m_shift4Input(shift4Input), m_shiftNInput(shiftNInput),
		m_shiftUpInput(shiftUpInput), m_shiftDownInput(shiftDownInput)
{
	// Initialize to gear 1
	prevValue = value = 1;
}

void CGearShift4Input::Poll()
{
	prevValue = value;

	// Gears (values 1-4) are set by pressing a button (lower gears have priority) and "stick" until a shift to another gear is made.
	// Neutral is selected by pressing the neutral gear button. It means all gears are released (value 0).
	if		(m_shiftNInput->Pressed()) value = 0;
	else if (m_shift1Input->Pressed()) value = 1;
	else if (m_shift2Input->Pressed()) value = 2;
	else if (m_shift3Input->Pressed()) value = 3;
	else if (m_shift4Input->Pressed()) value = 4;

	// Also the shift up/down controls can increase/decrease the gears too
	if      (m_shiftUpInput->Pressed())   value = CInputSource::Clamp(value + 1, 0, 4);
	else if (m_shiftDownInput->Pressed()) value = CInputSource::Clamp(value - 1, 0, 4);
}

CTriggerInput::CTriggerInput(const char *inputId, const char *inputLabel, unsigned inputGameFlags,
	CSwitchInput *triggerInput, CSwitchInput *offscreenInput, UINT16 offVal, UINT16 onVal) :
	CInput(inputId, inputLabel, INPUT_FLAGS_VIRTUAL, inputGameFlags),
	m_triggerInput(triggerInput), m_offscreenInput(offscreenInput), m_autoTrigger(false), m_offscreenCount(0), m_offVal(offVal), m_onVal(onVal)
{
	//
}

void CTriggerInput::ReadFromINIFile(CINIFile *ini, const char *section)
{
	CInput::ReadFromINIFile(ini, section);

	string key("Input");
	key.append(id);
	unsigned autoTrigger;
	if (ini->Get(section, key, autoTrigger) == OKAY)
		m_autoTrigger = !!autoTrigger;
}

void CTriggerInput::WriteToINIFile(CINIFile *ini, const char *section)
{
	CInput::WriteToINIFile(ini, section);

	string key("Input");
	key.append(id);
	ini->Set(section, key, (unsigned)m_autoTrigger);
}

void CTriggerInput::Poll()
{
	prevValue = value;

	// See if auto-trigger on reload is enabled
	if (m_autoTrigger)
	{
		// If so, when offscreen activated simulate triggered being pressed a short while afterwards
		if (m_offscreenCount > 0)
		{
			value = m_offscreenCount < 5;
			offscreenValue = m_onVal;
			m_offscreenCount--;
		}
		else
		{
			value = m_triggerInput->value;
			if (m_offscreenInput->Pressed())
			{
				offscreenValue = m_onVal;
				m_offscreenCount = 10;
			}
			else
				offscreenValue = m_offVal;
		}
	}
	else
	{
		// Otherwise if disabled, just take raw values from inputs
		value = m_triggerInput->value;
		offscreenValue = m_offscreenInput->value;
	}
}
