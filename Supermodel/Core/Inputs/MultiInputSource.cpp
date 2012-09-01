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
 * MultiInputSource.cpp
 *
 * Implementation of CMultiInputSource.
 */

#include "Supermodel.h"

#include <vector>
using namespace std;

ESourceType CMultiInputSource::GetCombinedType(vector<CInputSource*> &sources)
{
	// Check if vector is empty
	if (sources.size() == 0)
		return SourceEmpty;
	// Otherwise, see whether all sources are switches, or if have a full- or half-axis present
	bool allSwitches = true;
	bool hasFullAxis = false;
	bool hasHalfAxis = false;
	for (vector<CInputSource*>::iterator it = sources.begin(); it != sources.end(); it++)
	{
		if ((*it)->type == SourceInvalid)
			return SourceInvalid;  // An invalid source makes the whole lot invalid
		else if ((*it)->type == SourceSwitch)
			continue;
		allSwitches = false;
		if ((*it)->type == SourceFullAxis)
		{
			if (hasHalfAxis)
				return SourceInvalid;  // A half-axis and full-axis combined makes the whole lot invalid
			hasFullAxis = true;
		}
		else if ((*it)->type == SourceHalfAxis)
		{
			if (hasFullAxis)
				return SourceInvalid;  // A half-axis and full-axis combined makes the whole lot invalid
			hasHalfAxis = true;
		}
	}
	// Return resulting combined type
	if      (allSwitches) return SourceSwitch;
	else if (hasFullAxis) return SourceFullAxis;
	else if (hasHalfAxis) return SourceHalfAxis;
	else                  return SourceEmpty;
}

CMultiInputSource::CMultiInputSource() : CInputSource(SourceEmpty), m_isOr(true), m_numSrcs(0), m_srcArray(NULL) 
{
	//
}

CMultiInputSource::CMultiInputSource(bool isOr, vector<CInputSource*> &sources) : 
	CInputSource(GetCombinedType(sources)), m_isOr(isOr), m_numSrcs(sources.size())
{
	m_srcArray = new CInputSource*[m_numSrcs];
	copy(sources.begin(), sources.end(), m_srcArray);
}

CMultiInputSource::~CMultiInputSource()
{
	if (m_srcArray != NULL)
		delete m_srcArray;
}

void CMultiInputSource::Acquire()
{
	CInputSource::Acquire();

	if (m_acquired == 1)
	{
		// Acquire all sources
		for (int i = 0; i < m_numSrcs; i++)
			m_srcArray[i]->Acquire();
	}
}

void CMultiInputSource::Release()
{
	if (m_acquired == 1)
	{
		// Release all sources
		for (int i = 0; i < m_numSrcs; i++)
			m_srcArray[i]->Release();
	}

	CInputSource::Release();
}

bool CMultiInputSource::GetValueAsSwitch(bool &val)
{
	if (m_isOr)
	{
		// Return value for first input that is active
		for (int i = 0; i < m_numSrcs; i++)
		{
			if (m_srcArray[i]->GetValueAsSwitch(val))
				return true;
		}
		return false;
	}
	else
	{
		// Check all inputs are active
		for (int i = 0; i < m_numSrcs; i++)
		{
			if (!m_srcArray[i]->GetValueAsSwitch(val))
				return false;
		}
		return m_numSrcs > 0;
	}
}

bool CMultiInputSource::GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal)
{
	if (m_isOr)
	{
		// Return value for first input that is active
		for (int i = 0; i < m_numSrcs; i++)
		{
			if (m_srcArray[i]->GetValueAsAnalog(val, minVal, offVal, maxVal))
				return true;
		}
		return false;
	}
	else
	{
		// Check all switch inputs are active
		for (int i = 0; i < m_numSrcs; i++)
		{
			if (m_srcArray[i]->type == SourceSwitch && !m_srcArray[i]->GetValueAsAnalog(val, minVal, offVal, maxVal))
				return false;
		}
		// If so, then return value for first non-switch input that is active
		for (int i = 0; i < m_numSrcs; i++)
		{
			if (m_srcArray[i]->type != SourceSwitch && m_srcArray[i]->GetValueAsAnalog(val, minVal, offVal, maxVal))
				return true;
		}
		// If non found, then value is only valid if not empty and all inputs are switches 
		return m_numSrcs > 0 && type == SourceSwitch;
	}
}

bool CMultiInputSource::SendForceFeedbackCmd(ForceFeedbackCmd ffCmd)
{
	bool result = false;
	for (int i = 0; i < m_numSrcs; i++)
		result |= m_srcArray[i]->SendForceFeedbackCmd(ffCmd);
	return result;
}

CNegInputSource::CNegInputSource(CInputSource *source) : CInputSource(source->type), m_source(source) 
{
	// Acquire source
	m_source->Acquire();
}

void CNegInputSource::Acquire()
{
	CInputSource::Acquire();

	// Acquire source
	if (m_acquired == 1)
		m_source->Acquire();
}

void CNegInputSource::Release()
{
	// Release source
	if (m_acquired == 1)
		m_source->Release();

	CInputSource::Release();
}

bool CNegInputSource::GetValueAsSwitch(bool &val)
{
	bool oldVal = val;
	if (m_source->GetValueAsSwitch(val))
	{
		val = oldVal;
		return false;
	}
	val = true;
	return true;
}

bool CNegInputSource::GetValueAsAnalog(int &val, int minVal, int offVal, int maxVal)
{
	int oldVal = val;
	if (m_source->GetValueAsAnalog(val, minVal, offVal, maxVal))
	{
		val = oldVal;
		return false;
	}
	val = maxVal;
	return true;
}