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

// SPORT.cpp: implementation of the SPORT class.
//
//////////////////////////////////////////////////////////////////////

#include "freedoconfig.h"
#include "SPORT.h"
#include "arm.h"
#include <memory.h>

#include "freedocore.h"

extern _ext_Interface  io_interface;

static unsigned int gSPORTCOLOR;
static unsigned int gSPORTSOURCE=0;
static unsigned int gSPORTDESTINATION=0;
static unsigned char* VRAM;

unsigned int _sport_SaveSize()
{
        return 12;
}
void _sport_Save(void *buff)
{
        ((unsigned int*)buff)[0]=gSPORTCOLOR;
        ((unsigned int*)buff)[1]=gSPORTSOURCE;
        ((unsigned int*)buff)[2]=gSPORTDESTINATION;
}
void _sport_Load(void *buff)
{
        gSPORTCOLOR=((unsigned int*)buff)[0];
        gSPORTSOURCE=((unsigned int*)buff)[1];
        gSPORTDESTINATION=((unsigned int*)buff)[2];
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//static AString str;

void _sport_Init(unsigned char *vmem)
{
       	VRAM=vmem;
}

int __fastcall _sport_SetSource(unsigned int index) //take source for SPORT
{
	gSPORTSOURCE=(index<<7);
	return 0;
}


void __fastcall _sport_WriteAccess(unsigned int index, unsigned int mask)
{
 int i;
 unsigned int tmp,ctmp;



	if((index & ~0x1FFF)==0x4000) //SPORT flash write
	{
                index&=0x7ff;
                index<<=7;
                if(mask == 0xFFFFffff)
		{
			for(i=0;i<512;i++)
			{
				((unsigned int*)VRAM)[index+i]=gSPORTCOLOR;
			}
		}
		else  // mask is not 0xFFFFffff
		{
			for(i=0;i<512;i++)
			{
                                tmp=((unsigned int*)VRAM)[index+i];
				tmp=((tmp^gSPORTCOLOR)&mask)^gSPORTCOLOR;
				((unsigned int*)VRAM)[index+i]=tmp;
			}
		}
                if(!RESSCALE)return;
                memcpy(&((unsigned int*)VRAM)[index+1024*256], &((unsigned int*)VRAM)[index], 2048);
                memcpy(&((unsigned int*)VRAM)[index+2*1024*256], &((unsigned int*)VRAM)[index], 2048);
                memcpy(&((unsigned int*)VRAM)[index+3*1024*256], &((unsigned int*)VRAM)[index], 2048);
                return;
	}


	if(!(index & ~0x1FFF)) //SPORT copy page
	{
                gSPORTDESTINATION=(index &0x7ff)<<7;
                if(mask == 0xFFFFffff)
		{
			memcpy(&((unsigned int*)VRAM)[gSPORTDESTINATION],&((unsigned int*)VRAM)[gSPORTSOURCE],512*4);
                        if(!RESSCALE)return;
                        memcpy(&((unsigned int*)VRAM)[gSPORTDESTINATION+1024*256], &((unsigned int*)VRAM)[gSPORTSOURCE+1024*256], 2048);
                        memcpy(&((unsigned int*)VRAM)[gSPORTDESTINATION+2*1024*256], &((unsigned int*)VRAM)[gSPORTSOURCE+2*1024*256], 2048);
                        memcpy(&((unsigned int*)VRAM)[gSPORTDESTINATION+3*1024*256], &((unsigned int*)VRAM)[gSPORTSOURCE+3*1024*256], 2048);
                }
		else  // mask != 0xFFFFffff
		{
			for(i=0;i<512;i++)
			{
				tmp=((unsigned int*)VRAM)[gSPORTDESTINATION+i];
				ctmp=((unsigned int*)VRAM)[gSPORTSOURCE+i];
				tmp=((tmp^ctmp)&mask)^ctmp;
				((unsigned int*)VRAM)[gSPORTDESTINATION+i]=tmp;
			}
                        if(!RESSCALE)return;
                        memcpy(&((unsigned int*)VRAM)[gSPORTDESTINATION+1024*256], &((unsigned int*)VRAM)[gSPORTDESTINATION], 2048);
                        memcpy(&((unsigned int*)VRAM)[gSPORTDESTINATION+2*1024*256], &((unsigned int*)VRAM)[gSPORTDESTINATION], 2048);
                        memcpy(&((unsigned int*)VRAM)[gSPORTDESTINATION+3*1024*256], &((unsigned int*)VRAM)[gSPORTDESTINATION], 2048);
                }
		return;
	}

	if((index & ~0x1FFF)==0x2000) //SPORT set color!!!
	{
		gSPORTCOLOR=mask;
		return;
	}

       // io_interface(EXT_DEBUG_PRINT,(void*)str.print("::::SPORT:::: Unknown write, Index=0x%8.8X, Mask=0x%8.8X",index, mask).CStr());
}







