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

// XBUS.h: interface for the CXBUS class.
//
//////////////////////////////////////////////////////////////////////

#ifndef	XBUS_3DO_HEADER
#define XBUS_3DO_HEADER
#include "IsoXBUS.h"
//interface functions
typedef void* (*_xbus_device)(int, void*);
	int _xbus_Attach(_xbus_device dev);

	void _xbus_DevLoad(int dev, const char * name);	//!!!
	void _xbus_DevEject(int dev);				//!!!

	void _xbus_Init(_xbus_device zero_dev);
	void _xbus_Destroy();
	
	void _xbus_SetSEL(unsigned int val);
	void _xbus_SetPoll(unsigned int val);
	void _xbus_SetCommandFIFO(unsigned int val);
	void _xbus_SetDataFIFO(unsigned int val);

	unsigned int _xbus_GetStatusFIFO();
	unsigned int _xbus_GetRes();
	unsigned int _xbus_GetPoll();
	unsigned int _xbus_GetDataFIFO();

unsigned int _xbus_SaveSize();
void _xbus_Save(void *buff);
void _xbus_Load(void *buff);

#endif 
