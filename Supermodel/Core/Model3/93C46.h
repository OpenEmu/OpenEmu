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
 * 93C46.h
 * 
 * Header file defining the C93C46 class: 93C46 EEPROM.
 */

#ifndef INCLUDED_93C46_H
#define INCLUDED_93C46_H


/*
 * C93C46:
 *
 * 93C46 serial EEPROM.
 */
class C93C46
{
public:
	/*
	 * SaveState(SaveState):
	 *
	 * Saves an image of the current device state.
	 *
	 * Parameters:
	 *		SaveState	Block file to save state information to.
	 */
	void SaveState(CBlockFile *SaveState);

	/*
	 * LoadState(SaveState):
	 *
	 * Loads and a state image.
	 *
	 * Parameters:
	 *		SaveState	Block file to load state information from.
	 */
	void LoadState(CBlockFile *SaveState);
	
	/*
	 * Clear(void):
	 *
	 * Clears the EEPROM contents by writing all 1's.
	 */
	void Clear(void);
	
	/*
	 * Write(pinCS, pinCLK, pinDI):
	 *
	 * Write to the EEPROM. All inputs must be either 0 or 1 only!
	 *
	 * Parameters:
	 *		pinCS	Chip select.
	 *		pinCLK	Serial data clock.
	 *		pinDI	Serial data input.
	 */
	void Write(unsigned pinCS, unsigned pinCLK, unsigned pinDI);
	
	/*
	 * Read(void):
	 *
	 * Read from the EEPROM.
	 *
	 * Returns:
	 *		The serial data output bit (either a 1 or 0).
	 */
	unsigned Read(void);
	
	/*
	 * Reset(void):
	 *
	 * Resets the device, putting it into a locked state. Does not modify the
	 * memory.
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
	 * C93C46(void):
	 * ~C93C46(void):
	 *
	 * Constructor and destructor.
	 */
	C93C46(void);
	~C93C46(void);
	
private:
	UINT16		regs[64];	// memory: 64 16-bit registers
	unsigned	CS, CLK, DI, DO;	// pins
	UINT32		bitBufferOut;	// bits to be shifted out
	UINT32		bitBufferIn;	// stores bits as they are shifted in
	int			bitsOut;		// how many bits have been shifted out
	bool		receiving;		// if true, accepting data, if false, sending data out (read commands)
	unsigned	addr;			// latched address
	int			busyCycles;		// when > 0, counts down delay cycles and indicates busy
	bool		locked;			// whether the EEPROM is in a locked state
};


#endif	// INCLUDED_93C46_H
