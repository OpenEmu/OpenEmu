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
 * DSB.h
 * 
 * Header file for the Sega Digital Sound Board (Type 1 and 2) devices. CDSB1
 * is an implementation of the Z80-based DSB Type 1, and CDSB2 is the 68K-based
 * Type 2 board. Only one may be active at a time because they rely on non-
 * reentrant MPEG playback code.
 */

#ifndef INCLUDED_DSB_H
#define INCLUDED_DSB_H

#include "Types.h"
#include "CPU/Bus.h"


/******************************************************************************
 Configuration
 
 Because DSB code mixes both the sound board SCSP and MPEG audio together, both
 volume settings are stored here (for now). 
******************************************************************************/

/*
 * CDSBConfig:
 *
 * Settings used by CDSB.
 */
class CDSBConfig
{
public:
	bool	emulateDSB;	// DSB emulation (enabled if true)
	
	// Sound (SCSP) volume (0-200, 100 being full amplitude)
	inline void SetSoundVolume(unsigned vol)
	{
		if (vol > 200)
		{
			ErrorLog("Sound volume cannot exceed 200%%; setting to 100%%.\n");
			vol = 100;
		}
		
		soundVol = vol;
	}
	
	inline unsigned GetSoundVolume(void)
	{
		return soundVol;
	}
	
	// Music (DSB MPEG) volume (0-200)
	inline void SetMusicVolume(unsigned vol)
	{
		if (vol > 200)
		{
			ErrorLog("Music volume cannot exceed 200%%; setting to 100%%.\n");
			vol = 100;
		}
		
		musicVol = vol;
	}
	
	inline unsigned GetMusicVolume(void)
	{
		return musicVol;
	}
	
	// Defaults
	CDSBConfig(void)
	{
		emulateDSB = true;
		soundVol = 100;
		musicVol = 100;
	}
	
private:
	unsigned soundVol;
	unsigned musicVol;
};


/******************************************************************************
 Resampling
 
 Used internally by the DSB's MPEG code. If this becomes sufficiently generic,
 it can be moved to Sound/. Not intended for general use for now.
******************************************************************************/

/*
 * CDSBResampler:
 *
 * Frame-by-frame resampler. Resamples one single frame of audio and maintains
 * continuity between frames by copying unprocessed input samples to the 
 * beginning of the buffer and retaining the internal interpolation state. 
 *
 * See DSB.cpp for a detailed description of how this works.
 *
 * NOTE: If the sampling frequencies change, it is probably best to call
 * Reset(). Whether the resampler will otherwise behave correctly and stay
 * within array bounds has not been verified.
 *
 * Designed for use at 60 Hz, for input frequencies of 11.025, 22.05, 16, and 
 * 32 KHz and 44.1 KHz output frequencies. Theoretically, it should be able to
 * operate on most output frequencies and input frequencies that are simply 
 * lower, but it has not been extensively verified.
 */
class CDSBResampler
{
public:
	int		UpSampleAndMix(INT16 *outL, INT16 *outR, INT16 *inL, INT16 *inR, UINT8 volumeL, UINT8 volumeR, int sizeOut, int sizeIn, int outRate, int inRate);
	void	Reset(void);
private:
	int	nFrac;
	int	pFrac;
};


/******************************************************************************
 DSB Base Class
******************************************************************************/

/*
 * CDSB:
 *
 * Abstract base class defining the common interface for both DSB board types.
 */
 
class CDSB: public CBus
{
public:
	/*
	 * SendCommand(data):
	 *
	 * Send a MIDI command to the DSB board. 
	 */
	virtual void SendCommand(UINT8 data) = 0;
	
	/*
	 * RunFrame(audioL, audioR):
	 *
	 * Runs one frame and updates the MPEG audio. Audio is mixed into the
	 * supplied buffers (they are assumed to already contain audio data).
	 *
	 * Parameters:
	 *		audioL	Left audio channel, one frame (44 KHz, 1/60th second).
	 *		audioR	Right audio channel.
	 */
	virtual void RunFrame(INT16 *audioL, INT16 *audioR) = 0;
	
	/*
	 * Reset(void):
	 *
	 * Resets the DSB. Must be called prior to RunFrame().
	 */
	virtual void Reset(void) = 0;
	
	/*
	 * SaveState(SaveState):
	 *
	 * Saves an image of the current device state.
	 *
	 * Parameters:
	 *		SaveState	Block file to save state information to.
	 */
	virtual void SaveState(CBlockFile *SaveState) = 0;

	/*
	 * LoadState(SaveState):
	 *
	 * Loads and a state image.
	 *
	 * Parameters:
	 *		SaveState	Block file to load state information from.
	 */
	virtual void LoadState(CBlockFile *SaveState) = 0;
	
	/*
	 * Init(progROMPtr, mpegROMPtr):
	 *
	 * Initializes the DSB board. This member must be called first.
	 *
	 * Parameters:
	 *		progROMPtr	Program (68K or Z80) ROM.
	 *		mpegROMPtr	MPEG data ROM.
	 *
	 * Returns:
	 *		OKAY if successful, otherwise FAIL. 
	 */
	virtual bool	Init(const UINT8 *progROMPtr, const UINT8 *mpegROMPtr) = 0;
};


/******************************************************************************
 DSB Classes 
 
 DSB1 and DSB2 hardware. The base class, CDSB, should ideally be dynamically
 allocated using one of these. See CDSB for descriptions of member functions.
******************************************************************************/

/*
 * CDSB1:
 *
 * Sega Digital Sound Board Type 1: Z80 plus custom gate array for MPEG 
 * decoding.
 */
class CDSB1: public CDSB
{
public:
	// Read and write handlers for the Z80 (required by CBus)
	UINT8 	IORead8(UINT32 addr);
	void 	IOWrite8(UINT32 addr, UINT8 data);
	UINT8	Read8(UINT32 addr);
	void 	Write8(UINT32 addr, UINT8 data);
	
	// DSB interface (see CDSB definition)
	void 	SendCommand(UINT8 data);
	void 	RunFrame(INT16 *audioL, INT16 *audioR);
	void 	Reset(void);
	void	SaveState(CBlockFile *StateFile);
	void	LoadState(CBlockFile *StateFile);
	bool 	Init(const UINT8 *progROMPtr, const UINT8 *mpegROMPtr);
	
	// Returns a reference to the Z80 CPU
	CZ80 *GetZ80(void);

	// Constructor and destructor
	CDSB1(void);
	~CDSB1(void);
	
private:
	// Resampler
	CDSBResampler	Resampler;
	int				retainedSamples;	// how many MPEG samples carried over from previous frame
	
	// MPEG decode buffers (48KHz, 1/60th second + 2 extra padding samples)
	INT16	*mpegL, *mpegR;
	
	// DSB memory
	const UINT8	*progROM;		// Z80 program ROM (passed in from parent object)
	const UINT8	*mpegROM;		// MPEG music ROM
	UINT8		*memoryPool;	// all memory allocated here
	UINT8		*ram;			// Z80 RAM
	
	// Command FIFO
	UINT8	fifo[128];
	int		fifoIdxR;		// read position
	int		fifoIdxW;		// write position
	
	// MPEG playback variables
	int		mpegStart;
	int		mpegEnd;
	int		mpegState;
	int		loopStart;
	int		loopEnd;
	
	// Settings of currently playing stream (may not match the playback register variables above)
	UINT32	usingLoopStart;	// what was last set by MPEG_SetLoop() or MPEG_PlayMemory()
	UINT32	usingLoopEnd;
	UINT32	usingMPEGStart;	// what was last set by MPEG_PlayMemory()
	UINT32	usingMPEGEnd;
	
	// Registers
	UINT32	startLatch;	// MPEG start address latch
	UINT32	endLatch;	// MPEG end address latch
	UINT8	status;
	UINT8	cmdLatch;
	UINT8	volume;		// 0x00-0x7F
	UINT8	stereo;
	
	// Z80 CPU
	CZ80	Z80;
};

/*
 * CDSB2:
 *
 * Sega Digital Sound Board Type 2: 68K CPU.
 */
class CDSB2: public CDSB
{
public:
	// Read and write handlers for the 68K (required by CBus)
	UINT8	Read8(UINT32 addr);
	UINT16	Read16(UINT32 addr);
	UINT32	Read32(UINT32 addr);
	void	Write8(UINT32 addr, UINT8 data);
	void	Write16(UINT32 addr, UINT16 data);
	void	Write32(UINT32 addr, UINT32 data);
	
	// DSB interface (see definition of CDSB)
	void 	SendCommand(UINT8 data);
	void 	RunFrame(INT16 *audioL, INT16 *audioR);
	void 	Reset(void);
	void	SaveState(CBlockFile *StateFile);
	void	LoadState(CBlockFile *StateFile);
	bool 	Init(const UINT8 *progROMPtr, const UINT8 *mpegROMPtr);

	// Returns a reference to the 68K CPU context
	M68KCtx *GetM68K(void);

	// Constructor and destructor
	CDSB2(void);
	~CDSB2(void);
	
private:
	// Private helper functions
	void	WriteMPEGFIFO(UINT8 byte);
	
	// Resampler
	CDSBResampler	Resampler;
	int				retainedSamples;	// how many MPEG samples carried over from previous frame
	
	// MPEG decode buffers (48KHz, 1/60th second + 2 extra padding samples)
	INT16	*mpegL, *mpegR;
	
	// DSB memory
	const UINT8	*progROM;		// 68K program ROM (passed in from parent object)
	const UINT8	*mpegROM;		// MPEG music ROM
	UINT8		*memoryPool;	// all memory allocated here
	UINT8		*ram;			// 68K RAM

	// Command FIFO
	UINT8	fifo[128];
	int		fifoIdxR;		// read position
	int		fifoIdxW;		// write position
	
	// Registers
	int 	cmdLatch;
	int 	mpegState;
	int 	mpegStart, mpegEnd, playing;
	UINT8	volume[2];		// left, right volume (0x00-0xFF)
	
	// Settings of currently playing stream (may not match the playback register variables above)
	UINT32	usingLoopStart;	// what was last set by MPEG_SetLoop() or MPEG_PlayMemory()
	UINT32	usingLoopEnd;
	UINT32	usingMPEGStart;	// what was last set by MPEG_PlayMemory()
	UINT32	usingMPEGEnd;
	
	// M68K CPU
	M68KCtx	M68K;
};


#endif	// INCLUDED_DSB_H
