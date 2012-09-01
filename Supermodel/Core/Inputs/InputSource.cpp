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
 * InputSource.cpp
 *
 * Implementation of CInputSource. Obtains input values from the platform-
 * dependent input system and returns them as bools for switches and integer
 * values for analog inputs.
 */

#include "Supermodel.h"

#include <vector>
using namespace std;

CInputSource::CInputSource(ESourceType sourceType) : type(sourceType), m_acquired(0)
{
	//
}

void CInputSource::Acquire()
{
	m_acquired++;

#ifdef DEBUG
	CInputSystem::totalSrcsAcquired++;
#endif
}

void CInputSource::Release()
{
	if (--m_acquired == 0)
		delete this;

#ifdef DEBUG
	CInputSystem::totalSrcsReleased++;
#endif
}

int CInputSource::Clamp(int val, int minVal, int maxVal)
{
	if      (val > maxVal) return maxVal;
	else if (val < minVal) return minVal;
	else                   return val;
}

int CInputSource::Scale(int val, int fromMinVal, int fromMaxVal, int toMinVal, int toMaxVal)
{
	return Scale(val, fromMinVal, fromMinVal, fromMaxVal, toMinVal, toMinVal, toMaxVal);
}

int CInputSource::Scale(int val, int fromMinVal, int fromOffVal, int fromMaxVal, int toMinVal, int toOffVal, int toMaxVal)
{
	double fromRange;
	double frac;
	if (fromMaxVal > fromMinVal)
	{
		val = Clamp(val, fromMinVal, fromMaxVal);
		if (val > fromOffVal)
		{
			fromRange = (double)(fromMaxVal - fromOffVal);
			frac = (double)(val - fromOffVal) / fromRange;
		}
		else if (val < fromOffVal)
		{
			fromRange = (double)(fromOffVal - fromMinVal);
			frac = (double)(val - fromOffVal) / fromRange;
		}
		else
			return toOffVal;
	}
	else if (fromMinVal > fromMaxVal)
	{
		val = Clamp(val, fromMaxVal, fromMinVal);
		if (val > fromOffVal)
		{
			fromRange = (double)(fromMinVal - fromOffVal);
			frac = (double)(fromOffVal - val) / fromRange;
		}
		else if (val < fromOffVal)
		{
			fromRange = (double)(fromOffVal - fromMaxVal);
			frac = (double)(fromOffVal - val) / fromRange;
		}
		else
			return toOffVal;
	}
	else
		return toOffVal;
	double toRange;
	if (toMaxVal > toMinVal)
	{
		if (frac >= 0)
			toRange = (double)(toMaxVal - toOffVal);
		else
			toRange = (double)(toOffVal - toMinVal);
		return toOffVal + (int)(toRange * frac); 
	}
	else
	{
		if (frac >= 0)
			toRange = (double)(toOffVal - toMaxVal);
		else
			toRange = (double)(toMinVal - toOffVal);
		return toOffVal - (int)(toRange * frac); 
	}
}

bool CInputSource::IsActive()
{
	bool boolVal;
	return GetValueAsSwitch(boolVal);
}

bool CInputSource::SendForceFeedbackCmd(ForceFeedbackCmd ffCmd)
{
	return false;
}