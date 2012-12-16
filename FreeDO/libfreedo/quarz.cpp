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

#include "freedoconfig.h"
#include "quarz.h"
#include "types.h"
#include "Clio.h"
#include "Madam.h"


int ARM_CLOCK=12500000;
#define SND_CLOCK       44100
//#define NTSC_CLOCK      12270000        //818*500(lines)  //15 KHz
//#define PAL_CLOCK       14750000        //944*625(lines)  //15625 Hz


#pragma pack(push,1)
struct QDatum
{
        uint32 qrz_AccARM;
        uint32 qrz_AccDSP;
        uint32 qrz_AccVDL;
        uint32 qrz_TCount;
        uint32 VDL_CLOCK, qrz_vdlline, VDL_HS,VDL_FS;
};
#pragma pack(pop)

static QDatum quarz;

#include <memory.h>
unsigned int _qrz_SaveSize()
{
        return sizeof(QDatum);
}
void _qrz_Save(void *buff)
{
        memcpy(buff,&quarz,sizeof(QDatum));
}
void _qrz_Load(void *buff)
{
        memcpy(&quarz,buff,sizeof(QDatum));
}

#define qrz_AccARM quarz.qrz_AccARM
#define qrz_AccDSP quarz.qrz_AccDSP
#define qrz_AccVDL quarz.qrz_AccVDL
#define qrz_TCount quarz.qrz_TCount
#define VDL_CLOCK quarz.VDL_CLOCK
#define qrz_vdlline quarz.qrz_vdlline
#define VDL_HS quarz.VDL_HS
#define VDL_FS quarz.VDL_FS

void __fastcall _qrz_Init()
{
        qrz_AccVDL=qrz_AccDSP=0;
        qrz_AccARM=0;

        VDL_FS=526;
        VDL_CLOCK=VDL_FS*30;
        VDL_HS=VDL_FS/2;

        qrz_TCount=0;
        qrz_vdlline=0;
}

int __fastcall _qrz_VDCurrLine()
{
        return qrz_vdlline%(VDL_HS);
}
int __fastcall _qrz_VDHalfFrame()
{
        return qrz_vdlline/(VDL_HS);
}

int __fastcall _qrz_VDCurrOverline()
{
        return qrz_vdlline;
}

bool __fastcall _qrz_QueueVDL()
{
        if(qrz_AccVDL>>24)
        {
                qrz_AccVDL-=0x1000000;
                qrz_vdlline++;
                qrz_vdlline%=VDL_FS;
                return true;
        }
        return false;
}
bool __fastcall _qrz_QueueDSP()
{
        if(qrz_AccDSP>>24)
        {
                qrz_AccDSP-=0x1000000;
                return true;
        }
        return false;
}

bool __fastcall _qrz_QueueTimer()
{
 //uint32 cnt=_clio_GetTimerDelay();
        if(qrz_TCount>>24)//=cnt)
        {
                qrz_TCount-=0x1000000;//cnt;
                return true;
        }
        return false;
}

void __fastcall _qrz_PushARMCycles(unsigned int clks)
{
 uint32 arm;
        arm=(clks<<24)/ARM_CLOCK;
        qrz_AccARM+=arm*ARM_CLOCK;
        if( (qrz_AccARM>>24) != clks )
        {
                arm++;
                qrz_AccARM+=ARM_CLOCK;
                qrz_AccARM&=0xffffff;
        }
        qrz_AccDSP+=arm*SND_CLOCK;
        qrz_AccVDL+=arm*(VDL_CLOCK);

        //if(Get_madam_FSM()!=FSM_INPROCESS)
        if(_clio_GetTimerDelay())qrz_TCount+=arm*((__temporalfixes?12500000:25000000)/(_clio_GetTimerDelay()));//clks<<1;
}
