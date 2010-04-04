//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

/*
//---------------------------------------------------------------------------
//=========================================================================

	sound.h

//=========================================================================
//---------------------------------------------------------------------------

  History of changes:
  ===================

20 JUL 2002 - neopop_uk
=======================================
- Cleaned and tidied up for the source release

03 AUG 2002 - neopop_uk
=======================================
- Added dac_update function
- Made the DAC buffer frequency a global define.
- Converted DAC to mono, it never gets used in stereo.

//---------------------------------------------------------------------------
*/

#ifndef __SOUND__
#define __SOUND__
//=============================================================================

typedef struct
{
	int LastRegister;	/* last register written */
	int Register[8];	/* registers */
	int Volume[4];		
	int Period[4];		
	int Count[4];		
	int Output[4];		

	unsigned int RNG;	/* noise generator      */
	int NoiseFB;		/* noise feedback mask */

} SoundChip; 

//=============================================================================

extern SoundChip toneChip;
extern SoundChip noiseChip;

void WriteSoundChip(SoundChip* chip, _u8 data);

#define Write_SoundChipTone(VALUE)		(WriteSoundChip(&toneChip, VALUE))
#define Write_SoundChipNoise(VALUE)		(WriteSoundChip(&noiseChip, VALUE))

//=============================================================================

void dac_write(void);

//=============================================================================
#endif
