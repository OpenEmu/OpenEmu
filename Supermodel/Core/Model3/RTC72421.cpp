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
 * RTC72421.cpp
 * 
 * Epson RTC-72421 implementation. 
 *
 * TO-DO List:
 * -----------
 * - Rather than returning the host system time, should compute the "virtual"
 *   time inside the Model 3, which could differ.
 * - Writes do nothing yet.
 */

#include <time.h>
#include "Supermodel.h"


/******************************************************************************
 Emulation Functions
******************************************************************************/

UINT8 CRTC72421::ReadRegister(unsigned reg)
{
	time_t 		currentTime;
	struct tm	*Time;

	time(&currentTime);
	Time = localtime(&currentTime);

	switch (reg&0xF)
	{
		case 0:		// 1-second digit
			return (Time->tm_sec % 10) & 0xF;
		case 1:		// 10-seconds digit
			return (Time->tm_sec / 10) & 0x7;
		case 2:		// 1-minute digit
			return (Time->tm_min % 10) & 0xF;
		case 3:		// 10-minute digit
			return (Time->tm_min / 10) & 0x7;
		case 4:		// 1-hour digit
			return (Time->tm_hour % 10) & 0xF;
		case 5:		// 10-hours digit
			return (Time->tm_hour / 10) & 0x7;
		case 6:		// 1-day digit (days in month)
			return (Time->tm_mday % 10) & 0xF;
		case 7:		// 10-days digit
			return (Time->tm_mday / 10) & 0x3;
		case 8:		// 1-month digit
			return ((Time->tm_mon + 1) % 10) & 0xF;
		case 9:		// 10-months digit
			return ((Time->tm_mon + 1) / 10) & 0x1;
		case 10:	// 1-year digit
			return (Time->tm_year % 10) & 0xF;
		case 11:	// 10-years digit
			return ((Time->tm_year % 100) / 10) & 0xF;
		case 12:	// day of the week
			return Time->tm_wday & 0x7;
		case 13:
			break;
		case 14:
			break;
		case 15:
			break;
	}
	
	return 0;
}

void CRTC72421::WriteRegister(unsigned reg, UINT8 data)
{
	// TO-DO: emulate me!
}

void CRTC72421::Reset(void)
{
	// nothing to do
}


/******************************************************************************
 Initialization and Shutdown
******************************************************************************/

void CRTC72421::Init(void)
{
	// this function really only exists for consistency with other device classes
}

CRTC72421::CRTC72421(void)
{	
	DebugLog("Built RTC-72421\n");
}

CRTC72421::~CRTC72421(void)
{	
	DebugLog("Destroyed RTC-72421\n");
}
