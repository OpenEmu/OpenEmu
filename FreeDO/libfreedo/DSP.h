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

// DSP.h: interface for the CDSP class.
//
//////////////////////////////////////////////////////////////////////

#ifndef DSP_3DO_HEADER
#define DSP_3DO_HEADER


	unsigned int _dsp_Loop();

	unsigned short __fastcall _dsp_ReadIMem(unsigned short addr);
	void __fastcall _dsp_WriteIMem(unsigned short addr, unsigned short val);
	void __fastcall _dsp_WriteMemory(unsigned short addr,unsigned short val);
	void __fastcall _dsp_SetRunning(bool val);
	void __fastcall _dsp_ARMwrite2sema4(unsigned int val);
	unsigned int _dsp_ARMread2sema4(void);

	void _dsp_Init();
	void _dsp_Reset();
	

        unsigned int _dsp_SaveSize();
        void _dsp_Save(void *buff);
        void _dsp_Load(void *buff);


#endif 
