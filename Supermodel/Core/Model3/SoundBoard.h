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
 * SoundBoard.h
 * 
 * Header file defining the CSoundBoard class: Model 3 audio subsystem.
 */

#ifndef INCLUDED_SOUNDBOARD_H
#define INCLUDED_SOUNDBOARD_H

#include "Types.h"
#include "CPU/Bus.h"
#include "Model3/DSB.h"
#include "OSD/Thread.h"

/*
 * CSoundBoardConfig:
 *
 * Settings used by CSoundBoard.
 */
class CSoundBoardConfig
{
public:
	bool	emulateSound;	// sound board emulation (enabled if true)
	
	// Defaults
	CSoundBoardConfig(void)
	{
		emulateSound = true;
	}
};

/*
 * CSoundBoard:
 *
 * Model 3 sound board (68K CPU + 2 x SCSP).
 */
class CSoundBoard: public CBus
{
public:
	/*
	 * Read8(addr):
	 * Read16(addr):
	 * Read32(addr):
	 * Read64(addr):
	 *
	 * Read a byte, 16-bit word, or 32-bit long word from the 68K address 
	 * space. 
	 *
	 * Parameters:
	 *		addr	Address to read.
	 *
	 * Returns:
	 *		Data at the address.
	 */
	UINT8 Read8(UINT32 addr);
	UINT16 Read16(UINT32 addr);
	UINT32 Read32(UINT32 addr);
	
	/*
	 * Write8(addr, data):
	 * Write16(addr, data):
	 * Write32(addr, data):
	 *
	 * Write a byte, word, or long word to the 68K address space.
	 *
	 * Parameters:
	 *		addr	Address to write.
	 *		data	Data to write.
	 */
	void Write8(UINT32 addr, UINT8 data);
	void Write16(UINT32 addr, UINT16 data);
	void Write32(UINT32 addr, UINT32 data);

	/*
	 * WriteMIDIPort(data):
	 *
	 * Writes to the sound board MIDI port.
	 *
	 * Parameters:
	 *		data	Byte to write to MIDI port.
	 */
	void WriteMIDIPort(UINT8 data);
	
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
	 * RunFrame(void):
	 *
	 * Runs the sound board for one frame, updating sound in the process.
	 */
	bool RunFrame(void);
	
	/*
	 * Reset(void):
	 *
	 * Resets the sound board.
	 */
	void Reset(void);
	
	/*
	 * AttachDSB(CDSB *DSBPtr):
	 *
	 * Connects a Digital Sound Board. The sound board passes MIDI commands,
	 * resets the board, and runs it each frame to generate audio. If there is
	 * no DSB, this function does not need to be called.
	 *
	 * Parameters:
	 *		DSBPtr	Pointer to DSB object.
	 */
	void AttachDSB(CDSB *DSBPtr);
	
	/*
	 * GetMS68K(void):
	 *
	 * Returns a reference to the 68K CPU of the sound board.
	 *
	 * Returns:
	 *		A pointer to the M68K context.
	 */
	M68KCtx *GetM68K(void);

	/*
	 * GetDSB(void):
	 * 
	 * Returns a reference the Digital Sound Board (if attached).
	 *
	 * Returns:
	 *		A pointer to the DSB object or NULL if not attached.
	 */
	CDSB *GetDSB(void);

	/*
	 * Init(soundROMPtr, sampleROMPtr):
	 *
	 * One-time initialization. Must be called prior to all other members.
	 *
	 * Parameters:
	 *		soundROMPtr		Pointer to sound ROM (68K program).
	 *		sampleROMPtr	Pointer to sample ROM.
	 *
	 * Returns:
	 *		OKAY if successful, FAIL if unable to allocate memory. Prints own
	 *		error messages.
	 */
	bool Init(const UINT8 *soundROMPtr, const UINT8 *sampleROMPtr);

	/*
	 * CSoundBoard(void):
	 * ~CSoundBoard(void):
	 *
	 * Constructor and destructor.
	 */
	CSoundBoard(void);
	~CSoundBoard(void);
	
private:
	// Private helper functions
	void		UpdateROMBanks(void);
	
	// Digital Sound Board
	CDSB		*DSB;
	
	// 68K context
	M68KCtx		M68K;
	
	// Sound board memory
	const UINT8	*soundROM;		// 68K program ROM (passed in from parent object)
	const UINT8	*sampleROM;		// 68K sample ROM (passed in from parent object)
	const UINT8	*sampleBankLo;	// sample ROM bank switching
	const UINT8	*sampleBankHi;
	UINT8		*memoryPool;	// single allocated region for all sound board RAM
	UINT8		*ram1, *ram2;	// SCSP1 and SCSP2 RAM
	
	// Registers
	UINT8	ctrlReg;			// control register: ROM banking
	
	// Audio
	INT16	*audioL, *audioR;	// left and right audio channels (1/60th second, 44.1 KHz)
};


#endif	// INCLUDED_SOUNDBOARD_H
