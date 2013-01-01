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

// Madam.h: interface for the CMadam class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	MADAM_3DO_HEADER
#define MADAM_3DO_HEADER

#define FSM_IDLE 1
#define FSM_INPROCESS 2
#define FSM_SUSPENDED 3

        unsigned int Get_madam_FSM();
        void Set_madam_FSM(unsigned int val);


	 void _madam_SetMapping(unsigned int flag);
	 void _madam_Reset();
	 unsigned int _madam_GetCELCycles();
	 unsigned int * _madam_GetRegs();
	 int _madam_HandleCEL();      //!!!~!
	 void _madam_Init(unsigned char* memory);
	 void _madam_KeyPressed(unsigned char* data, unsigned int num);
	 void __fastcall _madam_Poke(unsigned int addr, unsigned int val);
	 unsigned int __fastcall _madam_Peek(unsigned int addr);
	 
unsigned int _madam_SaveSize();
void _madam_Save(void *buff);
void _madam_Load(void *buff);

        //void _madam_PrepToRendering();
        //bool __fastcall _madam_VRAMBusy(unsigned int addr, int size);


#endif 
