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
 * RTC72421.h
 * 
 * Header file defining the CRTC72421 class: RTC-72421 real-time clock.
 */

#ifndef INCLUDED_RTC72421_H
#define INCLUDED_RTC72421_H


/*
 * CRTC72421:
 *
 * Epson RTC-72421 real-time clock.
 */
class CRTC72421
{
public:
	/*
	 * ReadRegister(unsigned reg):
	 *
	 * Reads data from one of the RTC registers.
	 *
	 * Parameters:
	 *		reg		Register number (0-15).
	 *
	 * Returns:
	 *		Data in the lower nibble.
	 */
	UINT8 ReadRegister(unsigned reg);
	
	/*
	 * WriteRegister(unsigned reg, UINT8 data):
	 *
	 * Writes data to one of the RTC registers.
	 *
	 * Parameters:
	 *		reg		Register number (0-15).
	 *		data	Data to write.
	 */
	void WriteRegister(unsigned reg, UINT8 data);
	 
	/*
	 * Reset(void):
	 *
	 * Does nothing. Provided for compability with other device objects. The
	 * RTC is battery-backed and always available.
	 */
	void Reset(void);
	
	/*
	 * Init(void):
	 *
	 * One-time initialization of the context. Must be called prior to all
	 * other members.
	 */
	void Init(void);
	 
	/*
	 * CRTC72421(void):
	 * ~CRTC72421(void):
	 *
	 * Constructor and destructor.
	 */
	CRTC72421(void);
	~CRTC72421(void);
};


#endif	// INCLUDED_RTC72421_H
