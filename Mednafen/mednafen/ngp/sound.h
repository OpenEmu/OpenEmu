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

#ifndef __SOUND__
#define __SOUND__

void Write_SoundChipLeft(uint8 data);
void Write_SoundChipRight(uint8 data);

void dac_write_left(uint8);
void dac_write_right(uint8);

int32 MDFNNGPCSOUND_Flush(int16 *SoundBuf, const int32 MaxSoundFrames);
void MDFNNGPCSOUND_Init(void);
bool MDFNNGPC_SetSoundRate(uint32 rate);
int MDFNNGPCSOUND_StateAction(StateMem *sm, int load, int data_only);
void MDFNNGPCSOUND_SetEnable(bool set);

#endif
