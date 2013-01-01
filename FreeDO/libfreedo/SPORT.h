/*
  www.freedo.org
The first and only working 3DO multiplayer emulator.

The FreeDO licensed under modified GNU LGPL, with following notes:

*   The owners and original authors of the FreeDO have full right to develop closed source derivative work.
*   Any non-commercial uses of the FreeDO sources or any knowledge obtained by studying or reverse engineering
    of the sources, or any other material published by FreeDO have to be accompanied with full credits.
*   Any commercial uses of FreeDO sources or any knowledge obtained by studying or reverse engineering of the sources,
    or any other material published by FreeDO is strictly forbidden without owners approval.

The above notes are taking precedence over GNU LGPL in conflicting situations.

Project authors:

Alexander Troosh
Maxim Grishin
Allen Wright
John Sammons
Felix Lazarev
*/

// SPORT.h: interface for the SPORT class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	SPORT_3DO_HEADER
#define SPORT_3DO_HEADER


	void _sport_Init(unsigned char *vmem);

	 //take source for SPORT
	int __fastcall _sport_SetSource(unsigned int index);

	void __fastcall _sport_WriteAccess(unsigned int index, unsigned int mask);

unsigned int _sport_SaveSize();
void _sport_Save(void *buff);
void _sport_Load(void *buff);

#endif 
