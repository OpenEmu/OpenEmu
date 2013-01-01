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

// Clio.h: interface for the CClio class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	CLIO_3DO_HEADER
#define CLIO_3DO_HEADER

        int _clio_v0line();
        int _clio_v1line();
        bool __fastcall _clio_NeedFIQ();

	unsigned int _clio_FIFOStruct(unsigned int addr);
	void _clio_Reset();
	void _clio_SetFIFO(unsigned int adr, unsigned int val);
	unsigned short  __fastcall _clio_GetEOFIFOStat(unsigned char channel);
	unsigned short  __fastcall _clio_GetEIFIFOStat(unsigned char channel);
	unsigned short  __fastcall _clio_EIFIFONI(unsigned short channel);
	void  __fastcall _clio_EOFIFO(unsigned short channel, unsigned short val);
	unsigned short  __fastcall _clio_EIFIFO(unsigned short channel);
	
	void _clio_Init(int ResetReson);
	
	void __fastcall _clio_DoTimers();
	unsigned int __fastcall _clio_Peek(unsigned int addr);
	int __fastcall _clio_Poke(unsigned int addr, unsigned int val);
	void __fastcall _clio_UpdateVCNT(int line, int halfframe);
	void __fastcall _clio_GenerateFiq(unsigned int reason1, unsigned int reason2);

	unsigned int _clio_GetTimerDelay();

        unsigned int _clio_SaveSize();
        void _clio_Save(void *buff);
        void _clio_Load(void *buff);

#endif 
