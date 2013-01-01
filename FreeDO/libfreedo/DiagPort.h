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

// DiagPort.h: interface for the DiagPort class.
//
//////////////////////////////////////////////////////////////////////

#ifndef _DIAG_PORT_HEADER_DEFINITION_
#define _DIAG_PORT_HEADER_DEFINITION_

	void _diag_Init(int testcode);

	unsigned int __fastcall _diag_Get();
	void __fastcall _diag_Send(unsigned int val);


#endif 
