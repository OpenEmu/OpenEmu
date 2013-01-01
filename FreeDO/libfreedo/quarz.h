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

#ifndef QUARZ_3DO_HEADER_DEFINTION
#define QUARZ_3DO_HEADER_DEFINTION

void __fastcall _qrz_Init();

int __fastcall _qrz_VDCurrLine();
int __fastcall _qrz_VDHalfFrame();
int __fastcall _qrz_VDCurrOverline();

bool __fastcall _qrz_QueueVDL();
bool __fastcall _qrz_QueueDSP();
bool __fastcall _qrz_QueueTimer();

void __fastcall _qrz_PushARMCycles(unsigned int clks);

unsigned int _qrz_SaveSize();
void _qrz_Save(void *buff);
void _qrz_Load(void *buff);

#endif
 
