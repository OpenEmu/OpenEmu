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


// CPU.cpp: implementation of the CCPU class.
//
//////////////////////////////////////////////////////////////////////
#include "freedoconfig.h"
#include "arm.h"
#include "XBUS.h"
#include "vdlp.h"
#include "Madam.h"
#include "Clio.h"
#include "DiagPort.h"
#include "SPORT.h"

#ifdef _WIN32
#include <mem.h>
#else
#include <memory.h>
#endif


#include "types.h"
#include "freedocore.h"

extern _ext_Interface  io_interface;

#define ARM_MUL_MASK    0x0fc000f0
#define ARM_MUL_SIGN    0x00000090
#define ARM_SDS_MASK    0x0fb00ff0
#define ARM_SDS_SIGN    0x01000090
#define ARM_UND_MASK    0x0e000010
#define ARM_UND_SIGN    0x06000010
#define ARM_MRS_MASK    0x0fbf0fff
#define ARM_MRS_SIGN    0x010f0000
#define ARM_MSR_MASK    0x0fbffff0
#define ARM_MSR_SIGN    0x0129f000
#define ARM_MSRF_MASK   0x0dbff000
#define ARM_MSRF_SIGN   0x0128f000

//режимы процессора----------------------------------------------------------
#define ARM_MODE_USER   0
#define ARM_MODE_FIQ    1
#define ARM_MODE_IRQ    2
#define ARM_MODE_SVC    3
#define ARM_MODE_ABT    4
#define ARM_MODE_UND    5
#define ARM_MODE_UNK    0xff

//static AString str;

const static uint8 arm_mode_table[]=
{
     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,
     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,
     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,
     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,
     ARM_MODE_USER,    ARM_MODE_FIQ,     ARM_MODE_IRQ,     ARM_MODE_SVC,
     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_ABT,
     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UND,
     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK,     ARM_MODE_UNK
};

//для посчета тактов------------------------------------------------------------
#define NCYCLE 4
#define SCYCLE 1
#define ICYCLE 1

//--------------------------Conditions-------------------------------------------
    //flags - N Z C V  -  31...28
const static uint16 cond_flags_cross[]={   //((cond_flags_cross[cond_feald]>>flags)&1)  -- пример проверки
    0xf0f0, //EQ - Z set (equal)
    0x0f0f, //NE - Z clear (not equal)
    0xcccc, //CS - C set (unsigned higher or same)
    0x3333, //CC - C clear (unsigned lower)
    0xff00, //N set (negative)
    0x00ff, //N clear (positive or zero)
    0xaaaa, //V set (overflow)
    0x5555, //V clear (no overflow)
    0x0c0c, //C set and Z clear (unsigned higher)
    0xf3f3, //C clear or Z set (unsigned lower or same)
    0xaa55, //N set and V set, or N clear and V clear (greater or equal)
    0x55aa, //N set and V clear, or N clear and V set (less than)
    0x0a05, //Z clear, and either N set and V set, or N clear and V clear (greater than)
    0xf5fa, //Z set, or N set and V clear, or N clear and V set (less than or equal)
    0xffff, //always
    0x0000  //never
};


///////////////////////////////////////////////////////////////
// Global variables;
///////////////////////////////////////////////////////////////
#define RAMSIZE     3*1024*1024 //dram1+dram2+vram
#define ROMSIZE     1*1024*1024 //rom
#define NVRAMSIZE   (65536>>1)		//nvram at 0x03140000...0x317FFFF
#define REG_PC	RON_USER[15]
#define UNDEFVAL 0xBAD12345

#pragma pack(push,1)

uint32 *profiling;
uint32 *profiling2;
uint32 *profiling3;



struct ARM_CoreState
{
//console memories------------------------
    uint8 *Ram;//[RAMSIZE];
    uint8 *Rom;//[ROMSIZE*2];
	uint8 *NVRam;//[NVRAMSIZE];

//ARM60 registers
	uint32 USER[16];
	uint32 CASH[7];
    uint32 SVC[2];
    uint32 ABT[2];
    uint32 FIQ[7];
    uint32 IRQ[2];
    uint32 UND[2];
    uint32 SPSR[6];
	uint32 CPSR;

        bool nFIQ; //external interrupt
        bool SecondROM;	//ROM selector
        bool MAS_Access_Exept;	//memory exceptions
};
#pragma pack(pop)

static ARM_CoreState arm;
static int CYCLES;	//cycle counter

unsigned int __fastcall rreadusr(unsigned int rn);
void __fastcall loadusr(unsigned int rn, unsigned int val);
unsigned int __fastcall mreadb(unsigned int addr);
void __fastcall mwriteb(unsigned int addr, unsigned int val);
unsigned int __fastcall mreadw(unsigned int addr);
void __fastcall mwritew(unsigned int addr,unsigned int val);

#define MAS_Access_Exept	arm.MAS_Access_Exept
#define pRam	arm.Ram
#define pRom	arm.Rom
#define pNVRam	arm.NVRam
#define RON_USER	arm.USER
#define RON_CASH	arm.CASH
#define RON_SVC	arm.SVC
#define RON_ABT	arm.ABT
#define RON_FIQ	arm.FIQ
#define RON_IRQ	arm.IRQ
#define RON_UND	arm.UND
#define SPSR	arm.SPSR
#define CPSR	arm.CPSR
#define gFIQ	arm.nFIQ
#define gSecondROM	arm.SecondROM

void* Getp_NVRAM(){return pNVRam;};
void* Getp_ROMS(){return pRom;};
void* Getp_RAMS(){return pRam;};

unsigned int _arm_SaveSize()
{
        return sizeof(ARM_CoreState)+RAMSIZE+ROMSIZE*2+NVRAMSIZE;
}
void _arm_Save(void *buff)
{
        memcpy(buff,&arm,sizeof(ARM_CoreState));
        memcpy(((uint8*)buff)+sizeof(ARM_CoreState),pRam,RAMSIZE);
        memcpy(((uint8*)buff)+sizeof(ARM_CoreState)+RAMSIZE,pRom,ROMSIZE*2);
        memcpy(((uint8*)buff)+sizeof(ARM_CoreState)+RAMSIZE+ROMSIZE*2,pNVRam,NVRAMSIZE);
}

void _arm_Load(void *buff)
{
 uint8 *tRam=pRam;
 uint8 *tRom=pRom;
 uint8 *tNVRam=pNVRam;
        memcpy(&arm,buff,sizeof(ARM_CoreState));
        memcpy(tRam,((uint8*)buff)+sizeof(ARM_CoreState),RAMSIZE);
        memcpy(tRom,((uint8*)buff)+sizeof(ARM_CoreState)+RAMSIZE,ROMSIZE*2);
        memcpy(tNVRam,((uint8*)buff)+sizeof(ARM_CoreState)+RAMSIZE+ROMSIZE*2,NVRAMSIZE);

        memcpy(tRam+3*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+4*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+5*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+6*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+7*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+8*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+9*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+10*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+11*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+12*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+13*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+14*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+15*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+16*1024*1024,tRam+2*1024*1024, 1024*1024);
        memcpy(tRam+17*1024*1024,tRam+2*1024*1024, 1024*1024);

        pRom=tRom;
        pRam=tRam;
        pNVRam=tNVRam;
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

__inline void load(unsigned int rn, unsigned int val)
{
    RON_USER[rn]=val;
}


void ARM_RestUserRONS()
{
   switch(arm_mode_table[(CPSR&0x1f)])
   {
     case ARM_MODE_USER:
             break;
     case ARM_MODE_FIQ:
             memcpy(RON_FIQ,&RON_USER[8],7<<2);
             memcpy(&RON_USER[8],RON_CASH,7<<2);
             break;
     case ARM_MODE_IRQ:
			 RON_IRQ[0]=RON_USER[13];
			 RON_IRQ[1]=RON_USER[14];
			 RON_USER[13]=RON_CASH[5];
			 RON_USER[14]=RON_CASH[6];
             break;
     case ARM_MODE_SVC:
		     RON_SVC[0]=RON_USER[13];
			 RON_SVC[1]=RON_USER[14];
			 RON_USER[13]=RON_CASH[5];
			 RON_USER[14]=RON_CASH[6];
             break;
     case ARM_MODE_ABT:
			 RON_ABT[0]=RON_USER[13];
			 RON_ABT[1]=RON_USER[14];
			 RON_USER[13]=RON_CASH[5];
			 RON_USER[14]=RON_CASH[6];
             break;
     case ARM_MODE_UND:
			 RON_UND[0]=RON_USER[13];
			 RON_UND[1]=RON_USER[14];
			 RON_USER[13]=RON_CASH[5];
			 RON_USER[14]=RON_CASH[6];
             break;
   }
}

void ARM_RestFiqRONS()
{
   switch(arm_mode_table[(CPSR&0x1f)])
   {
     case ARM_MODE_USER:
             memcpy(RON_CASH,&RON_USER[8],7<<2);
             memcpy(&RON_USER[8],RON_FIQ,7<<2);
             break;
     case ARM_MODE_FIQ:
             break;
     case ARM_MODE_IRQ:
             memcpy(RON_CASH,&RON_USER[8],5<<2);
			 RON_IRQ[0]=RON_USER[13];
			 RON_IRQ[1]=RON_USER[14];
             memcpy(&RON_USER[8],RON_FIQ,7<<2);
             break;
     case ARM_MODE_SVC:
             memcpy(RON_CASH,&RON_USER[8],5<<2);
             RON_SVC[0]=RON_USER[13];
			 RON_SVC[1]=RON_USER[14];
             memcpy(&RON_USER[8],RON_FIQ,7<<2);
             break;
     case ARM_MODE_ABT:
             memcpy(RON_CASH,&RON_USER[8],5<<2);
             RON_ABT[0]=RON_USER[13];
			 RON_ABT[1]=RON_USER[14];
             memcpy(&RON_USER[8],RON_FIQ,7<<2);
             break;
     case ARM_MODE_UND:
             memcpy(RON_CASH,&RON_USER[8],5<<2);
             RON_UND[0]=RON_USER[13];
			 RON_UND[1]=RON_USER[14];
             memcpy(&RON_USER[8],RON_FIQ,7<<2);
             break;
   }
}

void ARM_RestIrqRONS()
{
   switch(arm_mode_table[(CPSR&0x1f)])
   {
     case ARM_MODE_USER:
			 RON_CASH[5]=RON_USER[13];
			 RON_CASH[6]=RON_USER[14];
             RON_USER[13]=RON_IRQ[0];
			 RON_USER[14]=RON_IRQ[1];
             break;
     case ARM_MODE_FIQ:
             memcpy(RON_FIQ,&RON_USER[8],7<<2);
             memcpy(&RON_USER[8],RON_CASH,5<<2);
			 RON_USER[13]=RON_IRQ[0];
			 RON_USER[14]=RON_IRQ[1];
             break;
     case ARM_MODE_IRQ:
             break;
     case ARM_MODE_SVC:
			 RON_SVC[0]=RON_USER[13];
			 RON_SVC[1]=RON_USER[14];
			 RON_USER[13]=RON_IRQ[0];
			 RON_USER[14]=RON_IRQ[1];
             break;
     case ARM_MODE_ABT:
		     RON_ABT[0]=RON_USER[13];
			 RON_ABT[1]=RON_USER[14];
			 RON_USER[13]=RON_IRQ[0];
			 RON_USER[14]=RON_IRQ[1];
             break;
     case ARM_MODE_UND:
			 RON_UND[0]=RON_USER[13];
			 RON_UND[1]=RON_USER[14];
			 RON_USER[13]=RON_IRQ[0];
			 RON_USER[14]=RON_IRQ[1];
             break;
   }
}

void ARM_RestSvcRONS()
{
   switch(arm_mode_table[(CPSR&0x1f)])
   {
     case ARM_MODE_USER:
			 RON_CASH[5]=RON_USER[13];
			 RON_CASH[6]=RON_USER[14];
			 RON_USER[13]=RON_SVC[0];
			 RON_USER[14]=RON_SVC[1];
             break;
     case ARM_MODE_FIQ:
             memcpy(RON_FIQ,&RON_USER[8],7<<2);
             memcpy(&RON_USER[8],RON_CASH,5<<2);
			 RON_USER[13]=RON_SVC[0];
			 RON_USER[14]=RON_SVC[1];
             break;
     case ARM_MODE_IRQ:
			 RON_IRQ[0]=RON_USER[13];
			 RON_IRQ[1]=RON_USER[14];
			 RON_USER[13]=RON_SVC[0];
			 RON_USER[14]=RON_SVC[1];
             break;
     case ARM_MODE_SVC:
             break;
     case ARM_MODE_ABT:
			 RON_ABT[0]=RON_USER[13];
			 RON_ABT[1]=RON_USER[14];
			 RON_USER[13]=RON_SVC[0];
			 RON_USER[14]=RON_SVC[1];
             break;
     case ARM_MODE_UND:
			 RON_UND[0]=RON_USER[13];
			 RON_UND[1]=RON_USER[14];
			 RON_USER[13]=RON_SVC[0];
			 RON_USER[14]=RON_SVC[1];
             break;
   }
}

void ARM_RestAbtRONS()
{
   switch(arm_mode_table[(CPSR&0x1f)])
   {
     case ARM_MODE_USER:
			 RON_CASH[5]=RON_USER[13];
			 RON_CASH[6]=RON_USER[14];
			 RON_USER[13]=RON_ABT[0];
			 RON_USER[14]=RON_ABT[1];
             break;
     case ARM_MODE_FIQ:
             memcpy(RON_FIQ,&RON_USER[8],7<<2);
             memcpy(&RON_USER[8],RON_CASH,5<<2);
			 RON_USER[13]=RON_ABT[0];
			 RON_USER[14]=RON_ABT[1];
             break;
     case ARM_MODE_IRQ:
			 RON_IRQ[0]=RON_USER[13];
			 RON_IRQ[1]=RON_USER[14];
			 RON_USER[13]=RON_ABT[0];
			 RON_USER[14]=RON_ABT[1];
             break;
     case ARM_MODE_SVC:
			 RON_SVC[0]=RON_USER[13];
			 RON_SVC[1]=RON_USER[14];
			 RON_USER[13]=RON_ABT[0];
			 RON_USER[14]=RON_ABT[1];
             break;
     case ARM_MODE_ABT:
             break;
     case ARM_MODE_UND:
			 RON_UND[0]=RON_USER[13];
			 RON_UND[1]=RON_USER[14];
			 RON_USER[13]=RON_ABT[0];
			 RON_USER[14]=RON_ABT[1];
             break;
   }
}

void ARM_RestUndRONS()
{
   switch(arm_mode_table[(CPSR&0x1f)])
   {
     case ARM_MODE_USER:
			 RON_CASH[5]=RON_USER[13];
			 RON_CASH[6]=RON_USER[14];
			 RON_USER[13]=RON_UND[0];
			 RON_USER[14]=RON_UND[1];
             break;
     case ARM_MODE_FIQ:
             memcpy(RON_FIQ,&RON_USER[8],7<<2);
             memcpy(&RON_USER[8],RON_CASH,5<<2);
			 RON_USER[13]=RON_UND[0];
			 RON_USER[14]=RON_UND[1];
             break;
     case ARM_MODE_IRQ:
			 RON_IRQ[0]=RON_USER[13];
			 RON_IRQ[1]=RON_USER[14];
			 RON_USER[13]=RON_UND[0];
			 RON_USER[14]=RON_UND[1];
             break;
     case ARM_MODE_SVC:
			 RON_SVC[0]=RON_USER[13];
			 RON_SVC[1]=RON_USER[14];
			 RON_USER[13]=RON_UND[0];
			 RON_USER[14]=RON_UND[1];
             break;
     case ARM_MODE_ABT:
			 RON_ABT[0]=RON_USER[13];
			 RON_ABT[1]=RON_USER[14];
			 RON_USER[13]=RON_UND[0];
			 RON_USER[14]=RON_UND[1];
             break;
     case ARM_MODE_UND:
             break;
   }
}

void __fastcall ARM_Change_ModeSafe(uint32 mode)
{
    switch(arm_mode_table[mode&0x1f])
    {
     case ARM_MODE_USER:
             ARM_RestUserRONS();
             break;
     case ARM_MODE_FIQ:
             ARM_RestFiqRONS();
             break;
     case ARM_MODE_IRQ:
             ARM_RestIrqRONS();
             break;
     case ARM_MODE_SVC:
             ARM_RestSvcRONS();
             break;
     case ARM_MODE_ABT:
             ARM_RestAbtRONS();
             break;
     case ARM_MODE_UND:
             ARM_RestUndRONS();
             break;
    }
}

void __fastcall SelectROM(int n)
{
    gSecondROM = (n>0)? true:false;
}

void _arm_SetCPSR(unsigned int a)
{
	if(arm_mode_table[a&0x1f]==ARM_MODE_UNK)
	{
		//!!Exeption!!
	}
	a|=0x10;
	ARM_Change_ModeSafe(a);
	CPSR=a&0xf00000df;
}


__inline void SETM(unsigned int a)
{
	if(arm_mode_table[a&0x1f]==ARM_MODE_UNK)
	{
		//!!Exeption!!
	}
	a|=0x10;
	ARM_Change_ModeSafe(a);
	CPSR=(CPSR&0xffffffe0)|(a & 0x1F);
}

// This functions d'nt change mode bits, then need no update regcur
__inline void SETN(bool a) { CPSR=(CPSR&0x7fffffff)|((a?1<<31:0)); }
__inline void SETZ(bool a) { CPSR=(CPSR&0xbfffffff)|((a?1<<30:0)); }
__inline void SETC(bool a) { CPSR=(CPSR&0xdfffffff)|((a?1<<29:0)); }
__inline void SETV(bool a) { CPSR=(CPSR&0xefffffff)|((a?1<<28:0)); }
__inline void SETI(bool a) { CPSR=(CPSR&0xffffff7f)|((a?1<<7:0)); }
__inline void SETF(bool a) { CPSR=(CPSR&0xffffffbf)|((a?1<<6:0)); }


///////////////////////////////////////////////////////////////
// Macros
///////////////////////////////////////////////////////////////
#define ISN  ((CPSR>>31)&1)
#define ISZ  ((CPSR>>30)&1)
#define ISC  ((CPSR>>29)&1)
#define ISV  ((CPSR>>28)&1)

#define MODE ((CPSR&0x1f))
#define ISI	 ((CPSR>>7)&1)
#define ISF  ((CPSR>>6)&1)


__inline uint32 _bswap(uint32 x)
{
	return (x>>24) | ((x>>8)&0x0000FF00L) | ((x&0x0000FF00L)<<8) | (x<<24);
}

__inline uint32 _rotr(uint32 val, uint32 shift)
{
        if(!shift)return val;
        return (val>>shift)|(val<<(32-shift));
}

unsigned char * _arm_Init()
{
    int i;

	MAS_Access_Exept=false;

        profiling=new uint32[(1024*1024*3)>>2];
        memset(profiling,0,RAMSIZE);

		profiling2=new uint32[(1024*1024*3)>>2];
        memset(profiling2,0,RAMSIZE);

		profiling3=new uint32[(1024*1024*3)>>2];
        memset(profiling3,0,RAMSIZE);

	CYCLES=0;
    for(i=0;i<16;i++)
        RON_USER[i]=0;
    for(i=0;i<2;i++)
    {
        RON_SVC[i]=0;
        RON_ABT[i]=0;
        RON_IRQ[i]=0;
        RON_UND[i]=0;
    }
    for(i=0;i<7;i++)
        RON_CASH[i]=RON_FIQ[i]=0;

	gSecondROM=0;
	pRam=new uint8[RAMSIZE+1024*1024*16];
	pRom=new uint8[ROMSIZE*2];
	pNVRam=new uint8[NVRAMSIZE];

    memset( pRam, 0, RAMSIZE+1024*1024*16);
    memset( pRom, 0, ROMSIZE*2);
    memset( pNVRam,0, NVRAMSIZE);
    gFIQ=false;

	io_interface(EXT_READ_NVRAM,pNVRam);//_3do_LoadNVRAM(pNVRam);

    // Endian swap for loaded ROM image


	REG_PC=0x03000000;
    _arm_SetCPSR(0x13); //set svc mode

    return (unsigned char *)pRam;
}

void _arm_Destroy()
{
	io_interface(EXT_WRITE_NVRAM,pNVRam);//_3do_SaveNVRAM(pNVRam);

        delete []profiling;
		delete []profiling2;
		delete []profiling3;
	delete []pNVRam;
	delete []pRom;
	delete []pRam;
}

void _arm_Reset()
{
    int i;
	gSecondROM=0;
	CYCLES=0;
    for(i=0;i<16;i++)
        RON_USER[i]=0;
    for(i=0;i<2;i++)
    {
        RON_SVC[i]=0;
        RON_ABT[i]=0;
        RON_IRQ[i]=0;
        RON_UND[i]=0;
    }
    for(i=0;i<7;i++)
        RON_CASH[i]=RON_FIQ[i]=0;

	MAS_Access_Exept=false;

    REG_PC=0x03000000;
    _arm_SetCPSR(0x13); //set svc mode
    gFIQ=false;		//no FIQ!!!
    gSecondROM=0;

	_clio_Reset();
	_madam_Reset();
}


void __fastcall ldm_accur(unsigned int opc, unsigned int base, unsigned int rn_ind)
{
 unsigned short x=opc&0xffff;
 unsigned short list=opc&0xffff;
 unsigned int base_comp,	//по ней шагаем
				i=0,tmp;

	x = (x & 0x5555) + ((x >> 1) & 0x5555);
	x = (x & 0x3333) + ((x >> 2) & 0x3333);
	x = (x & 0xff) + (x >> 8);
	x = (x & 0xf) + (x >> 4);

	switch((opc>>23)&3)
	{
	 case 0:
		base-=(x<<2);
		base_comp=base+4;
		break;
	 case 1:
		base_comp=base;
		base+=(x<<2);
		break;
	 case 2:
		base_comp=base=base-(x<<2);
		break;
	 case 3:
		base_comp=base+4;
		base+=(x<<2);
		break;
	}

	//base_comp&=~3;

	//if(opc&(1<<21))RON_USER[rn_ind]=base;

	if((opc&(1<<22)) && !(opc&0x8000))
	{
	    if(opc&(1<<21))loadusr(rn_ind,base);
		while(list)
		{
			if(list&1)
			{
				tmp=mreadw(base_comp);
				/*if(MAS_Access_Exept)
				{
					if(opc&(1<<21))RON_USER[rn_ind]=base;
					break;
				} */
				loadusr(i,tmp);
				base_comp+=4;
			}
			i++;
			list>>=1;
		}
	}
	else
	{
	    if(opc&(1<<21))RON_USER[rn_ind]=base;
		while(list)
		{
			if(list&1)
			{
				tmp=mreadw(base_comp);
				/*if(MAS_Access_Exept)
				{
					if(opc&(1<<21))RON_USER[rn_ind]=base;
					break;
				} */
				RON_USER[i]=tmp;
				base_comp+=4;
			}
			i++;
			list>>=1;
		}
		if((opc&(1<<22)) && arm_mode_table[MODE] /*&& !MAS_Access_Exept*/)
            _arm_SetCPSR(SPSR[arm_mode_table[MODE]]);
	}

	CYCLES-=(x-1)*SCYCLE+NCYCLE+ICYCLE;

}


void __fastcall stm_accur(unsigned int opc, unsigned int base, unsigned int rn_ind)
{
 unsigned short x=opc&0xffff;
 unsigned short list=opc&0x7fff;
 unsigned int base_comp,	//по ней шагаем
                                i=0;

	x = (x & 0x5555) + ((x >> 1) & 0x5555);
	x = (x & 0x3333) + ((x >> 2) & 0x3333);
	x = (x & 0xff) + (x >> 8);
	x = (x & 0xf) + (x >> 4);

	switch((opc>>23)&3)
	{
	 case 0:
		base-=(x<<2);
		base_comp=base+4;
		break;
	 case 1:
		base_comp=base;
		base+=(x<<2);
		break;
	 case 2:
		base_comp=base=base-(x<<2);
		break;
	 case 3:
		base_comp=base+4;
		base+=(x<<2);
		break;
	}

	//base_comp&=~3;


	if((opc&(1<<22)))
	{
	    if((opc&(1<<21)) && (opc&((1<<rn_ind)-1)) )loadusr(rn_ind,base);
		while(list)
		{
			if(list&1)
			{
				mwritew(base_comp,rreadusr(i));
				//if(MAS_Access_Exept)break;
				base_comp+=4;
			}
			i++;
			list>>=1;
		}
		if(opc&(1<<21))loadusr(rn_ind,base);
	}
	else
	{
	    if((opc&(1<<21)) && (opc&((1<<rn_ind)-1)) )RON_USER[rn_ind]=base;
		while(list)
		{
			if(list&1)
			{
				mwritew(base_comp,RON_USER[i]);
				//if(MAS_Access_Exept)break;
				base_comp+=4;
			}
			i++;
			list>>=1;
		}
		if(opc&(1<<21))RON_USER[rn_ind]=base;
	}

    if((opc&0x8000) /*&& !MAS_Access_Exept*/)mwritew(base_comp,RON_USER[15]+8);

	CYCLES-=(x-2)*SCYCLE+NCYCLE+NCYCLE;

}



void __fastcall  bdt_core(unsigned int opc)
{
 unsigned int base,rn_ind;

 //unsigned short list=opc&0xffff;

	//decode_mrt(opc);
	//return;

	rn_ind=(opc>>16)&0xf;

	if(rn_ind==0xf)base=RON_USER[rn_ind]+8;
	else base=RON_USER[rn_ind];


        if(opc&(1<<20))	//memory or register?
	{
		if(opc&0x8000)CYCLES-=SCYCLE+NCYCLE;

		ldm_accur(opc,base,rn_ind);

	}
	else //из регистра в память
	{
		stm_accur(opc,base,rn_ind);
	}

}

//------------------------------math SWI------------------------------------------------
typedef struct TagArg
			{
				unsigned int Type;
				unsigned int Arg;
			} TagItem;
void __fastcall decode_swi(unsigned int i)
{

    (void) i;
    SPSR[arm_mode_table[0x13]]=CPSR;

    SETI(1);
    SETM(0x13);

	load(14,REG_PC);

    REG_PC=0x00000008;
	CYCLES-=SCYCLE+NCYCLE; // +2S+1N
}


uint32 carry_out=0;

void ARM_SET_C(uint32 x)
{
    //old_C=(CPSR>>29)&1;

    CPSR=((CPSR&0xdfffffff)|(((x)&1)<<29));
}

#define ARM_SET_Z(x)    (CPSR=((CPSR&0xbfffffff)|((x)==0?0x40000000:0)))
#define ARM_SET_N(x)    (CPSR=((CPSR&0x7fffffff)|((x)&0x80000000)))
#define ARM_GET_C       ((CPSR>>29)&1)

__inline void ARM_SET_ZN(uint32 val)
{
        if(val)
                CPSR=((CPSR&0x3fffffff)|(val&0x80000000));
        else
                CPSR=((CPSR&0x3fffffff)|0x40000000);
}

__inline void ARM_SET_CV(uint32 rd, uint32 op1, uint32 op2)
{
    //old_C=(CPSR>>29)&1;

  	CPSR=(CPSR&0xcfffffff)|
		((((op1 & op2) | ((~rd) & (op1 | op2)))&0x80000000)>>2) |
		(((((op1&(op2&(~rd))) | ((~op1)&(~op2)&rd)))&0x80000000)>>3);

}

__inline void ARM_SET_CV_sub(uint32 rd, uint32 op1, uint32 op2)
{
    //old_C=(CPSR>>29)&1;

	CPSR=(CPSR&0xcfffffff)|
		//(( ( ~( ((~op1) & op2) | (rd&((~op1)|op2))) )&0x80000000)>>2) |
		((((op1 & (~op2)) | ((~rd) & (op1 | (~op2))))&0x80000000)>>2) |
		(((((op1&((~op2)&(~rd))) | ((~op1)&op2&rd)))&0x80000000)>>3);
}


bool __fastcall ARM_ALU_Exec(uint32 inst, uint8 opc, uint32 op1, uint32 op2, uint32 *Rd)
{
 switch(opc)
 {
   case 0:
        *Rd=op1&op2;
        break;
   case 2:
        *Rd=op1^op2;
        break;
   case 4:
        *Rd=op1-op2;
        break;
   case 6:
        *Rd=op2-op1;
        break;
   case 8:
        *Rd=op1+op2;
        break;
   case 10:
        *Rd=op1+op2+ARM_GET_C;
        break;
   case 12:
        *Rd=op1-op2-(ARM_GET_C^1);
        break;
   case 14:
        *Rd=op2-op1-(ARM_GET_C^1);
        break;
   case 16:
   case 20:
		if((inst>>22)&1)
			RON_USER[(inst>>12)&0xf]=SPSR[arm_mode_table[CPSR&0x1f]];
		else
			RON_USER[(inst>>12)&0xf]=CPSR;

		return true;
   case 18:
   case 22:
		if(!((inst>>16)&0x1) || !(arm_mode_table[MODE]))
		{
			if((inst>>22)&1)
				SPSR[arm_mode_table[MODE]]=(SPSR[arm_mode_table[MODE]]&0x0fffffff)|(op2&0xf0000000);
			else
				CPSR=(CPSR&0x0fffffff)|(op2&0xf0000000);
		}
		else
		{
			if((inst>>22)&1)
				SPSR[arm_mode_table[MODE]]=op2&0xf00000df;
			else
				_arm_SetCPSR(op2);
		}
	    return true;
   case 24:
        *Rd=op1|op2;
        break;
   case 26:
        *Rd=op2;
        break;
   case 28:
        *Rd=op1&(~op2);
        break;
   case 30:
        *Rd=~op2;
        break;
   case 1:
        *Rd=op1&op2;
        ARM_SET_ZN(*Rd);
        break;
   case 3:
        *Rd=op1^op2;
        ARM_SET_ZN(*Rd);
        break;
   case 5:
        *Rd=op1-op2;
        ARM_SET_ZN(*Rd);
        ARM_SET_CV_sub(*Rd,op1,op2);
        break;
   case 7:
        *Rd=op2-op1;
        ARM_SET_ZN(*Rd);
        ARM_SET_CV_sub(*Rd,op2,op1);
        break;
   case 9:
        *Rd=op1+op2;
        ARM_SET_ZN(*Rd);
        ARM_SET_CV(*Rd,op1,op2);
        break;

   case 11:
		*Rd=op1+op2+ARM_GET_C;
        ARM_SET_ZN(*Rd);
        ARM_SET_CV(*Rd,op1,op2);
        break;
   case 13:
        *Rd=op1-op2-(ARM_GET_C^1);
        ARM_SET_ZN(*Rd);
        ARM_SET_CV_sub(*Rd,op1,op2);
        break;
   case 15:
        *Rd=op2-op1-(ARM_GET_C^1);
        ARM_SET_ZN(*Rd);
        ARM_SET_CV_sub(*Rd,op2,op1);
        break;//*/
   case 17:
        op1&=op2;
        ARM_SET_ZN(op1);
	return true;
   case 19:
        op1^=op2;
        ARM_SET_ZN(op1);
        return true;
   case 21:
        ARM_SET_CV_sub(op1-op2,op1,op2);
        ARM_SET_ZN(op1-op2);
        return true;
   case 23:
        ARM_SET_CV(op1+op2,op1,op2);
        ARM_SET_ZN(op1+op2);
        return true;
   case 25:
        *Rd=op1|op2;
        ARM_SET_ZN(*Rd);
        break;
   case 27:
        *Rd=op2;
        ARM_SET_ZN(*Rd);
        break;
   case 29:
        *Rd=op1&(~op2);
        ARM_SET_ZN(*Rd);
        break;
   case 31:
        *Rd=~op2;
        ARM_SET_ZN(*Rd);
        break;
 };
 return false;
}


uint32 __fastcall ARM_SHIFT_NSC(uint32 value, uint8 shift, uint8 type)
{
  switch(type)
  {
   case 0:
        if(shift)
        {
			if(shift>32) carry_out=(0);
			else carry_out=(((value<<(shift-1))&0x80000000)>>31);
        }
        else carry_out=ARM_GET_C;

	    if(shift==0)return value;
		if(shift>31)return 0;
        return value<<shift;
   case 1:

        if(shift)
		{
			if(shift>32) carry_out=(0);
			else carry_out=((value>>(shift-1))&1);
		}
        else carry_out=ARM_GET_C;

	    if(shift==0)return value;
		if(shift>31)return 0;
        return value>>shift;
   case 2:

        if(shift)
		{
			if(shift>32) carry_out=((((signed int)value)>>31)&1);
			else carry_out=((((signed int)value)>>(shift-1))&1);
		}
        else carry_out=ARM_GET_C;

	    if(shift==0)return value;
		if(shift>31)return (((signed int)value)>>31);
        return (((signed int)value)>>shift);
   case 3:

        if(shift)
        {
				if(shift&31)carry_out=((value>>(shift-1))&1);
				else carry_out=((value>>31)&1);
        }
        else carry_out=ARM_GET_C;

        shift&=31;
		if(shift==0)return value;
        return _rotr(value, shift);
   case 4:
        carry_out=value&1;
        return (value>>1)|(ARM_GET_C<<31);
  }
  return 0;
}

uint32 __fastcall ARM_SHIFT_SC(uint32 value, uint8 shift, uint8 type)
{
uint32 tmp;
  switch(type)
  {
   case 0:
        if(shift)
        {
			if(shift>32) ARM_SET_C(0);
			else ARM_SET_C(((value<<(shift-1))&0x80000000)>>31);
        }
        else return value;
        if(shift>31)return 0;
        return value<<shift;
   case 1:
        if(shift)
		{
			if(shift>32) ARM_SET_C(0);
			else ARM_SET_C((value>>(shift-1))&1);
		}
        else return value;
		if(shift>31)return 0;
        return value>>shift;
   case 2:
        if(shift)
		{
			if(shift>32) ARM_SET_C((((signed int)value)>>31)&1);
			else ARM_SET_C((((signed int)value)>>(shift-1))&1);
		}
        else return value;
		if(shift>31)return (((signed int)value)>>31);
		return ((signed int)value)>>shift;
   case 3:
        if(shift)
        {
                shift=((shift)&31);
				if(shift)
				{
					ARM_SET_C((value>>(shift-1))&1);
				}
				else
				{
					ARM_SET_C((value>>31)&1);
				}
        }
        else return value;
        return _rotr(value, shift);
   case 4:
 	    tmp=ARM_GET_C<<31;
	    ARM_SET_C(value&1);
        return (value>>1)|(tmp);
  }

  return 0;
}



void __fastcall ARM_SWAP(uint32 cmd)
{

    unsigned int tmp, addr;

    REG_PC+=4;
    addr=RON_USER[(cmd>>16)&0xf];
    REG_PC+=4;

    if(cmd&(1<<22))
    {
        tmp=mreadb(addr);
	     //	if(MAS_Access_Exept)return true;
        mwriteb(addr, RON_USER[cmd&0xf]);
        REG_PC-=8;
	     //	if(MAS_Access_Exept)return true;
        RON_USER[(cmd>>12)&0xf]=tmp;
    }
    else
    {
        tmp=mreadw(addr);
        //if(MAS_Access_Exept)return true;
        mwritew(addr, RON_USER[cmd&0xf]);
        REG_PC-=8;
	  //	if(MAS_Access_Exept)return true;
		if(addr&3)tmp=(tmp>>((addr&3)<<3))|(tmp<<(32-((addr&3)<<3)));
        RON_USER[(cmd>>12)&0xf]=tmp;
    }
}

unsigned int calcbits(unsigned int num)
{
 unsigned int retval;

 if(!num)return 1;

 if(num>>16){num>>=16;retval=16;}
 else retval=0;

 if(num>>8){num>>=8;retval+=8;}
 if(num>>4){num>>=4;retval+=4;}
 if(num>>2){num>>=2;retval+=2;}
 if(num>>1){num>>=1;retval+=2;}
 else if(num)retval++;

 return retval;
}

unsigned int curr_pc;

const bool is_logic[]={
    true,true,false,false,
    false,false,false,false,
    true,true,false,false,
    true,true,true,true
    };

int __fastcall _arm_Execute()
{
    uint32 cmd,pc_tmp;

	//for(; CYCLES>0; CYCLES-=SCYCLE)
	{
 		cmd=mreadw(REG_PC);

                #ifdef DEBUG_CORE
                if(REG_PC<0x00300000)
                {
                        profiling[REG_PC>>2]++;
                }
                #endif

		curr_pc=REG_PC;


		REG_PC+=4;

                CYCLES=-SCYCLE;
		//if(MAS_Access_Exept)

		if(((cond_flags_cross[(((uint32)cmd)>>28)]>>((CPSR)>>28))&1))
		{

			switch((cmd>>24)&0xf)  //разбор типа команды
			{
			case 0x0:	//Multiply

					if ((cmd & ARM_MUL_MASK) == ARM_MUL_SIGN)
					{
						unsigned int res;

						res=((calcbits(RON_USER[(cmd>>8)&0xf])+5)>>1)-1;
						if(res>16)CYCLES-=16;
						else CYCLES-=res;

						if(((cmd>>16)&0xf)==(cmd&0xf))
						{
							if (cmd&(1<<21))
                                                        {
                                                                REG_PC+=8;
                                                                res=RON_USER[(cmd>>12)&0xf];
                                                                REG_PC-=8;
                                                        }
                                                        else
                                                                res=0;
						}
						else
						{
							if (cmd&(1<<21))
                                                        {
                                                                res=RON_USER[cmd&0xf]*RON_USER[(cmd>>8)&0xf];
                                                                REG_PC+=8;
                                                                res+=RON_USER[(cmd>>12)&0xf];
                                                                REG_PC-=8;
                                                        }
                                                        else
                                                                res=RON_USER[cmd&0xf]*RON_USER[(cmd>>8)&0xf];
						}
						if(cmd&(1<<20))
						{
							ARM_SET_ZN(res);
						}

						RON_USER[(cmd>>16)&0xf]=res;
						break;
					}
			case 0x1:	//Single Data Swap
					if ((cmd & ARM_SDS_MASK) == ARM_SDS_SIGN)
					{
						ARM_SWAP(cmd);
						//if(MAS_Access_Exept)
						CYCLES-=2*NCYCLE+ICYCLE;
						break;
					}
			case 0x2:	//ALU
			case 0x3:
                    uint32 op2,op1;
                                        //uint8 tmp;
					uint8 shift,shtype;

                    if((cmd&0x2000090)!=0x90)
                    {
							  /////////////////////////////////////////////SHIFT
                                pc_tmp=REG_PC;
                                REG_PC+=4;
								if (cmd&(1<<25))
								{
									op2=cmd&0xff;
									if(((cmd>>7)&0x1e))
									{
										op2=_rotr(op2, (cmd>>7)&0x1e);
										//if((cmd&(1<<20))) SETC(((cmd&0xff)>>(((cmd>>7)&0x1e)-1))&1);
									}
									op1=RON_USER[(cmd>>16)&0xf];
								}
								else
								{
									shtype=(cmd>>5)&0x3;
									if(cmd&(1<<4))
									{
										shift=((cmd>>8)&0xf);
										shift=(RON_USER[shift])&0xff;
                                                                                REG_PC+=4;
										op2=RON_USER[cmd&0xf];
										op1=RON_USER[(cmd>>16)&0xf];
										CYCLES-=ICYCLE;
									}
									else
									{
										shift=(cmd>>7)&0x1f;

										if(!shift)
										{
											if(shtype)
											{
												if(shtype==3)shtype++;
												else shift=32;
											}
										}
										op2=RON_USER[cmd&0xf];
										op1=RON_USER[(cmd>>16)&0xf];
									}


									//if((cmd&(1<<20)) && is_logic[((cmd>>21)&0xf)] ) op2=ARM_SHIFT_SC(op2, shift, shtype);
									//else
                                        op2=ARM_SHIFT_NSC(op2, shift, shtype);

								}

                              REG_PC=pc_tmp;

                              if((cmd&(1<<20)) && is_logic[((cmd>>21)&0xf)] ) ARM_SET_C(carry_out);


							  if(ARM_ALU_Exec(cmd, (cmd>>20)&0x1f ,op1,op2,&RON_USER[(cmd>>12)&0xf]))
							  {
										break;
							  }


							  if(((cmd>>12)&0xf)==0xf) //destination = pc, take care of cpsr
							  {
								if(cmd&(1<<20))
								{
									_arm_SetCPSR(SPSR[arm_mode_table[MODE]]);
								}

								CYCLES-=ICYCLE+NCYCLE;

							  }
							  break;
                    }
			case 0x6:	//Undefined
			case 0x7:

    Undefine:
				if((cmd&ARM_UND_MASK)==ARM_UND_SIGN)
				{
					//!!Exeption!!
                               //         io_interface(EXT_DEBUG_PRINT,(void*)str.print("*PC: 0x%8.8X undefined\n",REG_PC).CStr());

					SPSR[arm_mode_table[0x1b]]=CPSR;
					SETI(1);
					SETM(0x1b);
					load(14,REG_PC);
					REG_PC=0x00000004;  // (-4) fetch!!!
					CYCLES-=SCYCLE+NCYCLE; // +2S+1N
					break;
				}
			case 0x4:	//Single Data Transfer
			case 0x5:

                if((cmd&0x2000090)!=0x2000090)
                {
					  unsigned int base,tbas;
					  unsigned int oper2;
					  unsigned int val, rora;
					  //uint8	delta;

                      pc_tmp=REG_PC;
                      REG_PC+=4;
					  if(cmd&(1<<25))
					  {
                            shtype=(cmd>>5)&0x3;
                            if(cmd&(1<<4))
                            {
                                shift=((cmd>>8)&0xf);
                                shift=(RON_USER[shift])&0xff;
                                REG_PC+=4;
                            }
                            else
                            {
                                shift=(cmd>>7)&0x1f;
                                if(!shift)
                                {
                                    if(shtype)
                                    {
                                        if(shtype==3)shtype++;
                                        else shift=32;
                                    }
                                }
                            }
							oper2=ARM_SHIFT_NSC(RON_USER[cmd&0xf], shift, shtype);

					  }
					  else
					  {
							oper2=(cmd&0xfff);
					  }


					  tbas=base=RON_USER[((cmd>>16)&0xf)];


					  if(!(cmd&(1<<23))) oper2=0-oper2;

					  if(cmd&(1<<24)) tbas=base=base+oper2;
					  else base=base+oper2;


					  if(cmd&(1<<20)) //load
					  {
						  if(cmd&(1<<22))//bytes
						  {
							val=mreadb(tbas)&0xff;
						  }
						  else //words/halfwords
						  {
							val=mreadw(tbas);
                            rora=tbas&3;
							if((rora)) val=_rotr(val,rora*8);
						  }

						  if(((cmd>>12)&0xf)==0xf)
						  {
							  CYCLES-=SCYCLE+NCYCLE;   // +1S+1N if R15 load
						  }

						  CYCLES-=NCYCLE+ICYCLE;  // +1N+1I
                          REG_PC=pc_tmp;

						  if ((cmd&(1<<21)) || (!(cmd&(1<<24)))) load((cmd>>16)&0xf,base);

						  if((cmd&(1<<21)) && !(cmd&(1<<24)))
							  loadusr((cmd>>12)&0xf,val);//privil mode
						  else
							  load((cmd>>12)&0xf,val);

					  }
					  else
					  { // store

						  if((cmd&(1<<21)) && !(cmd&(1<<24)))
						  	  val=rreadusr((cmd>>12)&0xf);// privil mode
						  else
							  val=RON_USER[(cmd>>12)&0xf];

						  //if(((cmd>>12)&0xf)==0xf)val+=delta;
                          REG_PC=pc_tmp;
						  CYCLES-=-SCYCLE+2*NCYCLE;  // 2N

						  if(cmd&(1<<22))//bytes/words
						  	mwriteb(tbas,val);
						  else //words/halfwords
						  	mwritew(tbas,val);

						  if ( (cmd&(1<<21)) || !(cmd&(1<<24)) ) load((cmd>>16)&0xf,base);

					  }

					  //if(MAS_Access_Exept)


					break;
                }
                else goto Undefine;

			case 0x8:	//Block Data Transfer
			case 0x9:

				  bdt_core(cmd);
				  /*if(MAS_Access_Exept)
				  {
							//sprintf(str,"*PC: 0x%8.8X DataAbort!!!\n",REG_PC);
							//CDebug::DPrint(str);
							//!!Exeption!!

							SPSR[arm_mode_table[0x17]]=CPSR;
							SETI(1);
							SETM(0x17);
							load(14,REG_PC+4);
							REG_PC=0x00000010;
							CYCLES-=SCYCLE+NCYCLE;
							MAS_Access_Exept=false;
							break;
				  } */


				break;

			case 0xa:	//BRANCH
			case 0xb:
					if(cmd&(1<<24))
					{
						RON_USER[14]=REG_PC;
					}
					REG_PC+=(((cmd&0xffffff)|((cmd&0x800000)?0xff000000:0))<<2)+4;



					CYCLES-=SCYCLE+NCYCLE; //2S+1N

					break;

			case 0xf:	//SWI
					decode_swi(cmd);
					break;
			//---------
                        default:	//coprocessor
					//!!Exeption!!
                                        //io_interface(EXT_DEBUG_PRINT,(void*)str.print("*PC: 0x%8.8X undefined\n",REG_PC).CStr());

					SPSR[arm_mode_table[0x1b]]=CPSR;
					SETI(1);
					SETM(0x1b);
					load(14,REG_PC);
					REG_PC=0x00000004;
					CYCLES-=SCYCLE+NCYCLE;
					break;

			};


		}	//condition



            if(!ISF && _clio_NeedFIQ()/*gFIQ*/)
			{

					//Set_madam_FSM(FSM_SUSPENDED);
					gFIQ=0;

					SPSR[arm_mode_table[0x11]]=CPSR;
					SETF(1);
					SETI(1);
					SETM(0x11);
					load(14,REG_PC+4);
					REG_PC=0x0000001c;//1c
			}

	} // for(CYCLES)


	return -CYCLES;
}


void __fastcall _mem_write8(unsigned int addr, unsigned char val)
{
	    pRam[addr]=val;
	    if(addr<0x200000 || !RESSCALE) return;
        pRam[addr+1024*1024]=val;
        pRam[addr+2*1024*1024]=val;
        pRam[addr+3*1024*1024]=val;
}
void __fastcall _mem_write16(unsigned int addr, unsigned short val)
{
        *((unsigned short*)&pRam[addr])=val;
        if(addr<0x200000 || !RESSCALE) return;
        *((unsigned short*)&pRam[addr+1024*1024])=val;
        *((unsigned short*)&pRam[addr+2*1024*1024])=val;
        *((unsigned short*)&pRam[addr+3*1024*1024])=val;
}
void __fastcall _mem_write32(unsigned int addr, unsigned int val)
{
	    *((unsigned int*)&pRam[addr])=val;
        if(addr<0x200000 || !RESSCALE) return;
        *((unsigned int*)&pRam[addr+1024*1024])=val;
        *((unsigned int*)&pRam[addr+2*1024*1024])=val;
        *((unsigned int*)&pRam[addr+3*1024*1024])=val;
}

unsigned short __fastcall _mem_read16(unsigned int addr)
{
        return *((unsigned short*)&pRam[addr]);
}
unsigned int __fastcall _mem_read32(unsigned int addr)
{
        return *((unsigned int*)&pRam[addr]);
}
unsigned char __fastcall _mem_read8(unsigned int addr)
{
        return pRam[addr];
}


void __fastcall mwritew(unsigned int addr, unsigned int val)
{
    //to do -- wipe out all HW part
    //to do -- add proper loging
    unsigned int index;

	addr&=~3;


    if (addr<0x00300000) //dram1&dram2&vram
    {
        _mem_write32(addr,val);
        return;
    }

    if (!((index=(addr^0x03300000)) & ~0x7FF)) //madam
//  if((addr & ~0xFFFFF)==0x03300000) //madam
    {
        _madam_Poke(index,val);

        return;
    }


    if (!((index=(addr^0x03400000)) & ~0xFFFF)) //clio
//  if((addr & ~0xFFFFF)==0x03400000) //clio
    {
        if(_clio_Poke(index,val))
            REG_PC+=4;  // ???
        return;
    }

    if(!((index=(addr^0x03200000)) & ~0xFFFFF)) //SPORT
    {
        _sport_WriteAccess(index,val);
		return;
    }


    if (!((index=(addr^0x03100000)) & ~0xFFFFF)) // NVRAM & DiagPort
    {
        if(index & 0x80000) //if (addr>=0x03180000)
        {
            _diag_Send(val);
            return;
        }
        else if(index & 0x40000)       //else if ((addr>=0x03140000) && (addr<0x03180000))
		{
			//  sprintf(str,":NVRAM Write [0x%X] = 0x%8.8X\n",addr,val);
			//  CDebug::DPrint(str);
			pNVRam[(index>>2) & 32767]=(unsigned char)val;
			//CConfig::SetNVRAMData(pNVRam);
			io_interface(EXT_WRITE_NVRAM,pNVRam);//_3do_SaveNVRAM(pNVRam);
		}
        return;
    }

	/*
    if ((addr>=0x03000000) && (addr<0x03100000)) //rom
    {
        return;
    }*/
    //io_interface(EXT_DEBUG_PRINT,(void*)str.print("0x%8.8X:  WriteWord???  0x%8.8X=0x%8.8X\n",REG_PC,addr,val).CStr());


}

unsigned int __fastcall mreadw(unsigned int addr)
{
    //to do -- wipe out all HW
    //to do -- add abort (may be in HW)
    //to do -- proper loging
    unsigned int val;
    int index;

	addr&=~3;



    if (addr<0x00300000) //dram1&dram2&vram
    {
		return _mem_read32(addr);
    }

    if (!((index=(addr^0x03300000)) & ~0xFFFFF)) //madam
    {
        return _madam_Peek(index);
    }


    if (!((index=(addr^0x03400000)) & ~0xFFFFF)) //clio
    {
        return _clio_Peek(index);
    }

	if (!((index=(addr^0x03200000)) & ~0xFFFFF)) // read acces to SPORT
	{
		if (!((index=(addr^0x03200000)) & ~0x1FFF))
		{
			return _sport_SetSource(index);
		}
		else
		{
              //          io_interface(EXT_DEBUG_PRINT,(void*)str.print("0x%8.8X:  Unknow read access to SPORT  0x%8.8X=0x%8.8X\n",REG_PC,addr,0xBADACCE5).CStr());
			//!!Exeption!!
			return 0xBADACCE5;
		}
	}

    if (!((index=(addr^0x03000000)) & ~0xFFFFF)) //rom
    {
        if(!gSecondROM) // 2nd rom
		{
			return *(unsigned int*)(pRom+index);
		}
        else
            return *(unsigned int*)(pRom+index+1024*1024);
    }


    if (!((index=(addr^0x03100000)) & ~0xFFFFF)) // NVRAM & DiagPort
    {
        if(index & 0x80000) //if (addr>=0x03180000)
        {
            return _diag_Get();
        }
        else if(index & 0x40000)       //else if ((addr>=0x03140000) && (addr<0x03180000))
		{
			val=(unsigned int)pNVRam[(index>>2)&32767];
	        return val;
		}
		/*else
		{
			return 0xBADACCE5;
		}*/
    }

 //   io_interface(EXT_DEBUG_PRINT,(void*)str.print("0x%8.8X:  ReadWord???  0x%8.8X=0x%8.8X\n",REG_PC,addr,0xBADACCE5).CStr());

    //MAS_Access_Exept=true;
    return 0xBADACCE5;///data abort
}


void __fastcall mwriteb(unsigned int addr, unsigned int val)
{
  int index; // for avoid bad compiler optimization

  val&=0xff;


    if (addr<0x00300000) //dram1&dram2&vram
    {
                _mem_write8(addr^3,val);
		return;
    }

    else if (!((index=(addr^0x03100003)) & ~0xFFFFF)) //NVRAM
      {
		  if((index & 0x40000)==0x40000)
		  {
				//if((addr&3)==3)
				{
					pNVRam[(index>>2)&32767]=val;
					io_interface(EXT_WRITE_NVRAM,pNVRam);//_3do_SaveNVRAM(pNVRam);
				}
				return;
		  }
      }
    else if (!((index=(addr^0x03000003)) & ~0xFFFFF)) //rom
      {
        return;
      }

 //io_interface(EXT_DEBUG_PRINT,(void*)str.print("0x%8.8X:  WritetByte???  0x%8.8X=0x%8.8X\n",REG_PC,addr,val).CStr());

}



unsigned int __fastcall mreadb(unsigned int addr)
{

  int index; // for avoid bad compiler optimization

    if (addr<0x00300000) //dram1&dram2&vram
    {
        return _mem_read8(addr^3);
    }
    else if (!((index=(addr^0x03000003)) & ~0xFFFFF)) //rom
    {
		  if(!gSecondROM) // 2nd rom
		  {
			    return pRom[index];
		  }
		  else
				return pRom[index+1024*1024];
    }
    else if (!((index=(addr^0x03100003)) & ~0xFFFFF)) //NVRAM
    {
	    if((index & 0x40000)==0x40000)
		{
			//if((addr&3)!=3)return 0;
			//else
				return pNVRam[(index>>2)&32767];
		}
    }

	//MAS_Access_Exept=true;
//    io_interface(EXT_DEBUG_PRINT,(void*)str.print("0x%8.8X:  ReadByte???  0x%8.8X=0x%8.8X\n",REG_PC,addr,0xBADACCE5).CStr());

    return 0xBADACCE5;///data abort
}


void  __fastcall loadusr(unsigned int n, unsigned int val)
{
 if(n==15)
 {
	 RON_USER[15]=val;
	 return;
 }

 switch(arm_mode_table[(CPSR&0x1f)|0x10])
 {
  case ARM_MODE_USER:
    RON_USER[n]=val;
	break;
  case ARM_MODE_FIQ:
    if(n>7) RON_CASH[n-8]=val;
    else RON_USER[n]=val;
	break;
  case ARM_MODE_IRQ:
  case ARM_MODE_ABT:
  case ARM_MODE_UND:
  case ARM_MODE_SVC:
    if(n>12)RON_CASH[n-8]=val;
    else RON_USER[n]=val;
	break;
 }

}


unsigned int __fastcall rreadusr(unsigned int n)
{
 if(n==15)return RON_USER[15];
 switch(arm_mode_table[(CPSR&0x1f)])
 {
  case ARM_MODE_USER:
    return RON_USER[n];
  case ARM_MODE_FIQ:
    if(n>7)return RON_CASH[n-8];
    else return RON_USER[n];
  case ARM_MODE_IRQ:
  case ARM_MODE_ABT:
  case ARM_MODE_UND:
  case ARM_MODE_SVC:
    if(n>12)return RON_CASH[n-8];
    else return RON_USER[n];
 }
 return 0;
}




unsigned int __fastcall ReadIO(unsigned int addr)
{
    return mreadw(addr);
}

void __fastcall WriteIO(unsigned int addr, unsigned int val)
{
    mwritew(addr,val);
}


