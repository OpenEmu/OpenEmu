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

// Clio.cpp: implementation of the CClio class.
//
//////////////////////////////////////////////////////////////////////

#include "freedoconfig.h"
#include "Clio.h"
#include "Madam.h"
#include "XBUS.h"
#include "arm.h"
#include "DSP.h"

#define DECREMENT	0x1
#define RELOAD		0x2
#define CASCADE		0x4
#define FLABLODE	0x8


void __fastcall HandleDMA(unsigned int val);

//#define FIFODBG

#define RELOAD_VAL 0x10

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

#pragma pack(push,1)
struct FIFOt{

	unsigned int StartAdr;
	int StartLen;
	unsigned int NextAdr;
	int NextLen;
};
struct CLIODatum
{
        unsigned int cregs[65536];
        int DSPW1;
        int DSPW2;
        int DSPA;
        int PTRI[13];
        int PTRO[4];
        FIFOt FIFOI[13];
        FIFOt FIFOO[4];
};
#pragma pack(pop)

static unsigned int * Mregs;

static CLIODatum clio;

#define cregs clio.cregs
#define DSPW1 clio.DSPW1
#define DSPW2 clio.DSPW2
#define DSPA clio.DSPA
#define PTRI clio.PTRI
#define PTRO clio.PTRO
#define FIFOI clio.FIFOI
#define FIFOO clio.FIFOO

#include <memory.h>
unsigned int _clio_SaveSize()
{
        return sizeof(CLIODatum);
}
void _clio_Save(void *buff)
{
        memcpy(buff,&clio,sizeof(CLIODatum));
}
void _clio_Load(void *buff)
{
        memcpy(&clio,buff,sizeof(CLIODatum));
}

#define CURADR Mregs[base]
#define CURLEN Mregs[base+4]
#define RLDADR Mregs[base+8]
#define RLDLEN Mregs[base+0xc]

extern int fastrand();

int _clio_v0line()
{
        return cregs[8]&0x7ff;
}

int _clio_v1line()
{
        return cregs[12]&0x7ff;
}

bool __fastcall _clio_NeedFIQ()
{
        if( (cregs[0x40]&cregs[0x48]) || (cregs[0x60]&cregs[0x68]) ) return true;
        return false;
}

void __fastcall _clio_GenerateFiq(unsigned int reason1, unsigned int reason2)
{

	cregs[0x40]|=reason1;
	cregs[0x60]|=reason2;
	if(cregs[0x60])	cregs[0x40]|=0x80000000;	// irq31 if exist irq32 and high

	//if( (cregs[0x40]&cregs[0x48]) || (cregs[0x60]&cregs[0x68]) ) _arm_SetFIQ();
/////////////
#ifdef DBGFIQ
    sprintf(str,"FIQ accepted 0x%8.8X 0x%8.8X 0x%8.8X 0x%8.8X\n",reason1,reason2,CClio::Peek(0x48),CClio::Peek(0x68));
    CDebug::DPrint(str);
    if( (cregs[0x40]&cregs[0x48]) || (cregs[0x60]&cregs[0x68]) )
    {
        sprintf(str,"!!!FIQ decision taken\n");
        CDebug::DPrint(str);
    }
#endif
/////////////
}

#include "freedocore.h"
extern _ext_Interface  io_interface;
//extern AString str;
void __fastcall _clio_SetTimers(uint32 v200, uint32 v208);
void __fastcall _clio_ClearTimers(uint32 v204, uint32 v20c);
int __fastcall _clio_Poke(unsigned int addr, unsigned int val)
{
	int base;
	int i;

        //if(addr==0x200 || addr==0x204 || addr==0x208 || addr==0x20c || (addr>=0x100 && addr<=0x17c) || addr==0x220)io_interface(EXT_DEBUG_PRINT,(void*)str.print("CLIO Write[0x%X] = 0x%8.8X",addr,val).CStr());
        //if(addr==0x34 || addr==0x30)io_interface(EXT_DEBUG_PRINT,(void*)str.print("CLIO Write[0x%X] = 0x%8.8X",addr,val).CStr());
	if( (addr& ~0x2C) == 0x40 ) // 0x40..0x4C, 0x60..0x6C case
	{
		if(addr==0x40)
		{
			cregs[0x40]|=val;
			if(cregs[0x60]) cregs[0x40]|=0x80000000;
			//if(cregs[0x40]&cregs[0x48]) _arm_SetFIQ();
			return 0;
		}
		else if(addr==0x44)
		{
			cregs[0x40]&=~val;
			if(!cregs[0x60]) cregs[0x40]&=~0x80000000;
			return 0;
		}
		else if(addr==0x48)
		{
			cregs[0x48]|=val;
			//if(cregs[0x40]&cregs[0x48]) _arm_SetFIQ();
			return 0;
		}
		else if(addr==0x4c)
		{
			cregs[0x48]&=~val;
			cregs[0x48]|=0x80000000; // always one for irq31
			return 0;
		}
                /*else if(addr==0x50)
		{
			cregs[0x50]|=val&0x3fff0000;
			return 0;
		}
		else if(addr==0x54)
		{
			cregs[0x50]&=~val;
			return 0;
		} */
		else if(addr==0x60)
		{
			cregs[0x60]|=val;
			if(cregs[0x60]) cregs[0x40]|=0x80000000;
			//if(cregs[0x60]&cregs[0x68])	_arm_SetFIQ();
			return 0;
		}
		else if(addr==0x64)
		{
			cregs[0x60]&=~val;
			if(!cregs[0x60]) cregs[0x40]&=~0x80000000;
			return 0;
		}
		else if(addr==0x68)
		{
			cregs[0x68]|=val;
			//if(cregs[0x60]&cregs[0x68]) _arm_SetFIQ();
			return 0;
		}
		else if(addr==0x6c)
		{
			cregs[0x68]&=~val;
			return 0;
		}
	}
	else if(addr==0x84)
	{
		//temp=val&0xf0;
		//temp=temp>>4;
		//val&=0x0f;

		//cregs[0x84]&=~temp;
		//cregs[0x84]|=val;
		cregs[0x84]=val&0xf;
		SelectROM((val&4)? 1:0 );
		return 0;
	}else if(addr==0x300)
	{	//clear down the fifos and stop them
		base=0;
		cregs[0x304]&=~val;

		for(i=0;i<13;i++)
		{
			if(val&(1<<i))
			{
				base=0x400+(i<<4);
				RLDADR=CURADR=0;
				RLDLEN=CURLEN=0;
				_clio_SetFIFO(base,0);
				_clio_SetFIFO(base+4,0);
				_clio_SetFIFO(base+8,0);
				_clio_SetFIFO(base+0xc,0);
				val&=~(1<<i);
				PTRI[i]=0;
			}

		}
		{
			for(i=0;i<4;i++)
			{
				if(val&(1<<(i+16)))
				{
					base=0x500+(i<<4);
					RLDADR=CURADR=0;
					RLDLEN=CURLEN=0;
					_clio_SetFIFO(base,0);
					_clio_SetFIFO(base+4,0);
					_clio_SetFIFO(base+8,0);
					_clio_SetFIFO(base+0xc,0);

					val&=~(1<<(i+16));
					PTRO[i]=0;

				}

			}

		}

		return 0;

	}
	else if(addr==0x304) // Dma Starter!!!!! P/A !!!! need to create Handler.
	{

		//if(val&0x00100000)
		//{
			HandleDMA(val);
		//	cregs[0x304]&=~0x00100000;
		//}
		return 0;
	}
	else if(addr==0x308) //Dma Stopper!!!!
	{
		cregs[0x304]&=~val;
		return 0;
	}
	else if(addr==0x400) //XBUS direction
	{
		if(val&0x800)
			return 0;
		else
		{
			cregs[0x400]=val;
			return 0;
		}
	}
	else if((addr>=0x500) && (addr<0x540))
	{
		_xbus_SetSEL(val);

		return 0;
	}
	else if((addr>=0x540) && (addr<0x580))
	{
#ifdef DBGXBUS
		sprintf(str,"XBPC : 0x%8.8x :",RegRead(15));
		CDebug::DPrint(str);
#endif
		_xbus_SetPoll(val);
		return 0;
	}
	else if((addr>=0x580) && (addr<0x5c0))
	{
		_xbus_SetCommandFIFO(val); // on FIFO Filled execute the command
		return 0;
	}
        else if((addr>=0x5c0) && (addr<0x600))
	{
		_xbus_SetDataFIFO(val); // on FIFO Filled execute the command
		return 0;
	}
	else if(addr==0x28)
	{
		cregs[addr]=val;
		if(val==0x30)
			return 1;
		else
			return 0;
	}else if((addr>=0x1800)&&(addr<=0x1fff))//0x0340 1800 … 0x0340 1BFF && 0x0340 1C00 … 0x0340 1FFF
	{
		addr&=~0x400; //mirrors
		DSPW1=val>>16;
		DSPW2=val&0xffff;
		DSPA=(addr-0x1800)>>1;
		//sprintf(str,"0x%8.8X : 2x1NWRITE 0x%3.3X 0x%4.4X\n",GetPC(),DSPA,DSPW1);
		//CDebug::DPrint(str);
		//sprintf(str,"0x%8.8X : 2x2NWRITE 0x%3.3X 0x%4.4X\n",GetPC(),DSPA+1,DSPW2);
		//CDebug::DPrint(str);


		_dsp_WriteMemory(DSPA,DSPW1);
		_dsp_WriteMemory(DSPA+1,DSPW2);
		return 0;
		//DSPNRAMWrite 2 DSPW per 1ARMW
	}else if((addr>=0x2000)&&(addr<=0x2fff))
	{
		addr&=~0x800;//mirrors
		DSPW1=val&0xffff;
		DSPA=(addr-0x2000)>>2;
		//sprintf(str,"0x%8.8X : 1xNWRITE 0x%3.3X 0x%4.4X\n",GetPC(),DSPA,DSPW1);
		//CDebug::DPrint(str);

		_dsp_WriteMemory(DSPA,DSPW1);
		return 0;
		//DSPNRAMWrite 1 DSPW per 1ARMW
	}else if((addr>=0x3000)&&(addr<=0x33ff)) //0x0340 3000 … 0x0340 33FF
	{
		DSPA=(addr-0x3000)>>1;
		DSPA&=0xff;
		DSPW1=val>>16;
		DSPW2=val&0xffff;
		_dsp_WriteIMem(DSPA,DSPW1);
		_dsp_WriteIMem(DSPA+1,DSPW2);
	/*	printf("#ARM 2IWRITE1 0x%3.3X<=0x%4.4X\n",DSPA,DSPW1);
		printf("#ARM 2IWRITE2 0x%3.3X<=0x%4.4X\n",DSPA+1,DSPW2);
		printf("#ARM word %8.8x\n-----------------------\n",val);*/
		return 0;
		//DSPEIRam 2 DSPW per 1ARMW
	}else if((addr>=0x3400)&&(addr<=0x37ff))//0x0340 3400 … 0x0340 37FF
	{
		DSPA=(addr-0x3400)>>2;
		DSPA&=0xff;
		//if(DSPA>0x6f)
			//printf("##Strange coeff write -- 0x%3.3X<=0x%4.4X\n",DSPA,DSPW1);
		DSPW1=val&0xffff;
		_dsp_WriteIMem(DSPA,DSPW1);
	/*	printf("#ARM IWRITE 0x%3.3X<=0x%4.4X\n",DSPA,DSPW1);
		printf("#ARM word %8.8x\n-----------------------\n",val);*/
		return 0;
		//DSPEIRam 1 DSPW per 1 ARMW
	}
	else if(addr==0x17E8)//Reset
	{
		_dsp_Reset();
		return 0;
	}
	else if(addr==0x17D0)//Write DSP/ARM Semaphore
	{
		_dsp_ARMwrite2sema4(val);
		return 0;
	}
	else if(addr==0x17FC)//start/stop
	{
		_dsp_SetRunning(val>0);
		return 0;
	}
	else if(addr==0x200)
	{
		cregs[0x200]|=val;
                _clio_SetTimers(val, 0);
		return 0;
	}
	else if(addr==0x204)
	{
		cregs[0x200]&=~val;
                _clio_ClearTimers(val, 0);
		return 0;
	}
	else if(addr==0x208)
	{
		cregs[0x208]|=val;
                _clio_SetTimers(0, val);
		return 0;
	}
	else if(addr==0x20c)
	{
		cregs[0x208]&=~val;
                _clio_ClearTimers(0, val);
		return 0;
	}
	else if(addr==0x220)
	{
                //if(val<64)val=64;
		cregs[addr]=val&0x3ff;
		return 0;
	}
        else if(addr>=0x100 && addr<=0x7c)
        {
                cregs[addr]=val&0xffff;
		return 0;
        }

	cregs[addr]=val;
	return 0;
}



unsigned int __fastcall _clio_Peek(unsigned int addr)
{

#ifdef DBGCLIO
		val=CClio::Peek(index);
		if(index!=0x34)
		{
			sprintf(str,"0x%8.8X : CLIO Read clio[0x%X] = 0x%8.8X\n",RegRead(15),index,val);
			CDebug::DPrint(str);
		}
		return val;
#endif

	//if(addr>0x600) // ???????  DSP debug
	//{
	//	printf("#CLIO:PEEK (0x%4.4X)\n",addr);
	//}
        //if(addr==0x200 || addr==0x204 || addr==0x208 || addr==0x20c || (addr>=0x100 && addr<=0x17c) || addr==0x220)io_interface(EXT_DEBUG_PRINT,(void*)str.print("CLIO Read[0x%X] = 0x%8.8X",addr,cregs[addr]).CStr());
        //if(addr==0x34 || addr==0x30)io_interface(EXT_DEBUG_PRINT,(void*)str.print("CLIO Read[0x%X] = 0x%8.8X",addr,cregs[addr]).CStr());

	if( (addr& ~0x2C) == 0x40 ) // 0x40..0x4C, 0x60..0x6C case
	{
		addr&=~4;	// By read 40 and 44, 48 and 4c, 60 and 64, 68 and 6c same
		if(addr==0x40)
		{
			return cregs[0x40];
		}
		else if(addr==0x48)
		{
			return cregs[0x48]|0x80000000;
		}
		else if(addr==0x60)
		{
			return cregs[0x60];
		}
		else if(addr==0x68)
		{
			return cregs[0x68];
		}
		return 0; // for skip warning C4715
	}
	else if(addr==0x204)
	{
		return cregs[0x200];
	}
	else if(addr==0x20c)
	{
		return cregs[0x208];
	}
	else if(addr==0x308)
	{
		return cregs[0x304];
	}
	else if (addr==0x414)
	{
		return 0x4000; //TO CHECK!!! requested by CDROMDIPIR
	}
	else if((addr>=0x500) && (addr<0x540))
	{
		return _xbus_GetRes();
	}
	else if((addr>=0x540) && (addr<0x580))
	{
#ifdef DBGXBUS
		sprintf(str,"XBPC : 0x%8.8x :",RegRead(15));
		CDebug::DPrint(str);
#endif
		return _xbus_GetPoll();
	}
	else if((addr>=0x580) && (addr<0x5c0))
	{
		return _xbus_GetStatusFIFO();
	}
	else if((addr>=0x5c0) && (addr<0x600))
	{
		return _xbus_GetDataFIFO();
	}
	else if(addr==0x0)
	{
		return 0x02020000;
	}
	else if((addr>=0x3800)&&(addr<=0x3bff))//0x0340 3800 … 0x0340 3BFF
	{
		//2DSPW per 1ARMW
		DSPA=(addr-0x3800)>>1;
		DSPA&=0xff;
		DSPA+=0x300;
		DSPW1=_dsp_ReadIMem(DSPA);
		DSPW2=_dsp_ReadIMem(DSPA+1);
		return ((DSPW1<<16)|DSPW2);
	}
	else if((addr>=0x3c00)&&(addr<=0x3fff))//0x0340 3C00 … 0x0340 3FFF
	{
		DSPA=(addr-0x3c00)>>2;
		DSPA&=0xff;
		DSPA+=0x300;
		return (_dsp_ReadIMem(DSPA));
	}
	else if(addr==0x17F0)
	{
		return fastrand();
	}
	else if(addr==0x17D0)//Read DSP/ARM Semaphore
	{
		return _dsp_ARMread2sema4();
	}
        else if(addr>=0x100 && addr<=0x7c)
        {
		return cregs[addr]&0xffff;
        }
	else
		return cregs[addr];
}

void __fastcall _clio_UpdateVCNT(int line, int halfframe)
{
//	Poke(0x34,Peek(0x34)+1);
	cregs[0x34]=(halfframe<<11)+line;
}


void __fastcall _clio_SetTimers(uint32 v200, uint32 v208)
{
    (void) v200;
    (void) v208;
}
void __fastcall _clio_ClearTimers(uint32 v204, uint32 v20c)
{
    (void) v204;
    (void) v20c;
}
void __fastcall _clio_DoTimers()
{
    unsigned int timer;
    unsigned short counter;
    unsigned int flag;
    bool NeedDecrementNextTimer=true;   // Need decrement for next timer
    //bool prevdec=false;
        for (timer=0;timer<16;timer++)
        {
            flag=cregs[(timer<8)?0x200:0x208]>>((timer*4)&31);
            if( !(flag&CASCADE) ) NeedDecrementNextTimer=true;

            if( NeedDecrementNextTimer && (flag&DECREMENT) )
            {
				counter=cregs[0x100+timer*8];
                                if((NeedDecrementNextTimer=(counter--==0)))
                                {
                                                if((timer&1))   // Only odd timers can generate
						{  // generate the interrupts because be overflow
						   _clio_GenerateFiq(1<<(10-timer/2),0);
						}
						if(flag&RELOAD)
						{  // reload timer by reload value
						   counter=cregs[0x100+timer*8+4];
                                                   //return;
						}
						else
						{  // timer stopped -> reset it's flag DECREMENT
						   cregs[(timer<8)?0x200:0x208] &= ~( DECREMENT<<((timer*4)&31) );
						}
				}
				cregs[0x100+timer*8]=counter;
            }
            //else if( !prevdec && (flag&CASCADE) ) return;
            else NeedDecrementNextTimer=false;
            //if( !(flag&CASCADE) ) NeedDecrementNextTimer=true;
            //prevdec=(flag&DECREMENT);
        }
}

unsigned int _clio_GetTimerDelay()
{
	return cregs[0x220];
}


void __fastcall HandleDMA(unsigned int val)
{
	unsigned int src;
	unsigned int trg;
        int len;
	unsigned int ptr;
	unsigned char b0,b1,b2,b3;


	cregs[0x304]|=val;
	if(val&0x00100000)
	{
		cregs[0x304]&=~0x00100000;
		src=_madam_Peek(0x540);
                //if(src&3)_3do_DPrint("Align Err!!! - see CLIO XBUS DMA");
		trg=src;
		len=_madam_Peek(0x544);
                //if(len&3)_3do_DPrint("Overflow Err!!! - see CLIO XBUS DMA");

		cregs[0x400]&=~0x80;

	  if((cregs[0x404])&0x200)
	  {
		ptr=0;
		while(len>=0)
		  {
			  b3=_xbus_GetDataFIFO();
			  //st=CXBUS::GetPoll();
			  //ptr++;

  			  b2=_xbus_GetDataFIFO();
			  //st=CXBUS::GetPoll();

			  b1=_xbus_GetDataFIFO();
			  //st=CXBUS::GetPoll();

			  b0=_xbus_GetDataFIFO();
			  //st=CXBUS::GetPoll();

			  _mem_write8(trg,b0);
                          _mem_write8(trg+1,b1);
                          _mem_write8(trg+2,b2);
                          _mem_write8(trg+3,b3);

			  trg+=4;
			  len-=4;
			  ptr+=4;

		  }
		//CCdRom::ClearDataPoll(CMadam::Peek(0x544));
		//CCdRom::ClearDataPoll();
		cregs[0x400]|=0x80;

	  }
	  else
	  {
		ptr=0;

		while(len>=0)
		  {
			  b3=_xbus_GetDataFIFO();
			  //st=CXBUS::GetPoll();
			  //ptr++;

  			  b2=_xbus_GetDataFIFO();
			  //st=CXBUS::GetPoll();

			  b1=_xbus_GetDataFIFO();
			  //st=CXBUS::GetPoll();

			  b0=_xbus_GetDataFIFO();
			  //st=CXBUS::GetPoll();

                          _mem_write8(trg,b0);
                          _mem_write8(trg+1,b1);
                          _mem_write8(trg+2,b2);
                          _mem_write8(trg+3,b3);

			  trg+=4;
			  len-=4;
			  ptr+=4;

		  }
		//CCdRom::ClearDataPoll(CMadam::Peek(0x544));
		//CCdRom::ClearDataPoll();
		cregs[0x400]|=0x80;

	  }
	  len=0xFFFFFFFC;
	  _madam_Poke(0x544,len);
	  	//event.type = SDL_USEREVENT;
		//event.user.code = CREATEFIQ|29;
	//SDL_PushEvent(&event);
		//CFE::FE_PushEvent(&event);
		_clio_GenerateFiq(1<<29,0);

	  return;
	}//XBDMA transfer
#ifdef FIFODBG
	else
		{

				sprintf(str,"_Fifo304 inChan=");
				CDebug::DPrint(str);

				for(i=0;i<13;i++)
				{
					if(val&(1<<i))
					{
						sprintf(str,"%d ",i);
						CDebug::DPrint(str);
					}

				}
				sprintf(str,"outChan=");
				CDebug::DPrint(str);
				for(i=0;i<4;i++)
					{
						if(val&(1<<(i+16)))
						{
							sprintf(str,"%d ",i);
							CDebug::DPrint(str);
						}
					}
				sprintf(str,"\n");
				CDebug::DPrint(str);
		}
#endif
}

void _clio_Init(int ResetReson)
{
	for(int i=0;i<32768;i++)
		cregs[i]=0;

        //cregs[8]=240;

    cregs[0x0028]=ResetReson;
    cregs[0x0400]=0x80;
	cregs[0x220]=64;
	Mregs=_madam_GetRegs();

}
unsigned short  __fastcall _clio_EIFIFO(unsigned short channel)
{
	unsigned int val,base,mask;



	base=0x400+(channel*16);
	mask=1<<channel;

	if(FIFOI[channel].StartAdr!=0)//channel enabled
	{

		if( (FIFOI[channel].StartLen-PTRI[channel])>0 )
		{
			val=_mem_read16( ((FIFOI[channel].StartAdr+PTRI[channel])^2) );
			PTRI[channel]+=2;
		}
		else
		{
			PTRI[channel]=0;
			_clio_GenerateFiq(1<<(channel+16),0);//generate fiq
			if(FIFOI[channel].NextAdr!=0)// reload enabled see patent WO09410641A1, 49.16
			{
				FIFOI[channel].StartAdr=FIFOI[channel].NextAdr;
				FIFOI[channel].StartLen=FIFOI[channel].NextLen;
				val=_mem_read16(((FIFOI[channel].StartAdr+PTRI[channel])^2)); //get the value!!!
				PTRI[channel]+=2;
			}
			else
			{
				 FIFOI[channel].StartAdr=0;
				 val=0;
			}
		}
	}

	/*else
	{
		val=0;
		_clio_GenerateFiq(1<<(channel+16),0);
	} */

	return val;
}

void  __fastcall _clio_EOFIFO(unsigned short channel, unsigned short val)
{

	unsigned int base;
	unsigned int mask;
	base=0x500+(channel*16);
	mask=1<<(channel+16);

	if(FIFOO[channel].StartAdr!=0)//channel enabled
	{

		if( (FIFOO[channel].StartLen-PTRO[channel])>0 )
		{
			_mem_write16(((FIFOO[channel].StartAdr+PTRO[channel])^2),val);
			PTRO[channel]+=2;
		}
		else
		{
			PTRO[channel]=0;
			_clio_GenerateFiq(1<<(channel+12),0);//generate fiq
			if(FIFOO[channel].NextAdr!=0) //reload enabled?
			{
				FIFOO[channel].StartAdr=FIFOO[channel].NextAdr;
				FIFOO[channel].StartLen=FIFOO[channel].NextLen;
			}
			else
			{
				 FIFOO[channel].StartAdr=0;
			}
		}
	}


}

unsigned short  __fastcall _clio_EIFIFONI(unsigned short channel)
{
	unsigned int base;
	base=0x400+(channel*16);
	return _mem_read16(((FIFOI[channel].StartAdr+PTRI[channel])^2));
}

unsigned short   __fastcall _clio_GetEIFIFOStat(unsigned char channel)
{
	unsigned int mask;

	mask=1<<channel;
	//if(cregs[0x304]&mask)//channel enabled
	if( FIFOI[channel].StartAdr!=0 )
	{
		//return 1;
		//return (FIFOI[channel].StartLen-PTRI[channel]);
		return 2;// 2fixme
	}
	return 0;
}

unsigned short   __fastcall _clio_GetEOFIFOStat(unsigned char channel)
{
	unsigned int mask;
	mask=1<<(channel+16);
	//if(cregs[0x304]&mask)//channel enabled
	if( FIFOO[channel].StartAdr!=0 )
		return 1;
	return 0;

}

void _clio_SetFIFO(unsigned int adr, unsigned int val)
{
	if((adr&0x500)==0x400)
	{

		switch (adr&0xf)
		{
			case 0: FIFOI[(adr>>4)&0xf].StartAdr=val;
				FIFOI[(adr>>4)&0xf].NextAdr=0;//see patent WO09410641A1, 46.25

#ifdef FIFODBG
				sprintf(str,"SetInFIFO chan=%x StartAdr=%x\n",(adr>>4)&0xf,val);CDebug::DPrint(str);
#endif
				return;
			case 4: FIFOI[(adr>>4)&0xf].StartLen=val+4;
				if(val==0)
					FIFOI[(adr>>4)&0xf].StartLen=0;

				FIFOI[(adr>>4)&0xf].NextLen=0;//see patent WO09410641A1, 46.25
#ifdef FIFODBG
				sprintf(str,"SetInFIFO chan=%x StartLen=%x\n",(adr>>4)&0xf,val+4);CDebug::DPrint(str);
#endif
				return;
			case 8: FIFOI[(adr>>4)&0xf].NextAdr=val;
				#ifdef FIFODBG
				sprintf(str,"SetInFIFO chan=%x NextAdr=%x\n",(adr>>4)&0xf,val);CDebug::DPrint(str);
				#endif
				return;
			case 0xc:
				if(val!=0)
					FIFOI[(adr>>4)&0xf].NextLen=val+4;
				else
					FIFOI[(adr>>4)&0xf].NextLen=0;
				#ifdef FIFODBG
				sprintf(str,"SetInFIFO chan=%x NextLen=%x\n",(adr>>4)&0xf,val+4);CDebug::DPrint(str);
				#endif
				return;
		}
	}
	else
	{
	#ifdef FIFODBG
		sprintf(str,"SetOutFIFO chan=%x addr=%x var=%x\n",(adr>>4)&0xf,val,adr&0xf);
		 CDebug::DPrint(str);
#endif
		switch (adr&0xf)
		{
			case 0: FIFOO[(adr>>4)&0xf].StartAdr=val;return;
			case 4: FIFOO[(adr>>4)&0xf].StartLen=val+4;return;
			case 8: FIFOO[(adr>>4)&0xf].NextAdr=val;return;
			case 0xc:FIFOO[(adr>>4)&0xf].NextLen=val+4;return;
		}

	}
}



void _clio_Reset()
{
	int i;
	for(i=0;i<65536;i++)
		cregs[i]=0;

}

unsigned int _clio_FIFOStruct(unsigned int addr)
{
	if((addr&0x500)==0x400)
	{
		//printf("SetOutFIFO chan=%x addr=%x var=%x\n",(adr>>4)&0xf,val,adr&0xf);
		switch (addr&0xf)
		{
			case 0:
				#ifdef FIFODBG
				sprintf(str,"_GetInFIFO chan=%x StartAdr=%x\n",(addr>>4)&0xf,FIFOI[(addr>>4)&0xf].StartAdr+PTRI[(addr>>4)&0xf]);CDebug::DPrint(str);
#endif
				return FIFOI[(addr>>4)&0xf].StartAdr+PTRI[(addr>>4)&0xf];
			case 4:
				#ifdef FIFODBG
				sprintf(str,"_GetInFIFO chan=%x StartLen=%x\n",(addr>>4)&0xf,FIFOI[(addr>>4)&0xf].StartLen-PTRI[(addr>>4)&0xf]);CDebug::DPrint(str);
			#endif
				return FIFOI[(addr>>4)&0xf].StartLen-PTRI[(addr>>4)&0xf];
			case 8:
				#ifdef FIFODBG
				sprintf(str,"_GetInFIFO chan=%x NextAdr=%x\n",(addr>>4)&0xf,FIFOI[(addr>>4)&0xf].NextAdr);CDebug::DPrint(str);
				#endif
				return FIFOI[(addr>>4)&0xf].NextAdr;
			case 0xc:
				#ifdef FIFODBG
				sprintf(str,"_GetInFIFO chan=%x NextLen=%x\n",(addr>>4)&0xf,FIFOI[(addr>>4)&0xf].NextLen);CDebug::DPrint(str);
				#endif
				return FIFOI[(addr>>4)&0xf].NextLen;
		}
	}
	else
	{
		//printf("SetInFIFO chan=%x addr=%x var=%x\n",(adr>>4)&0xf,val,adr&0xf);
		switch (addr&0xf)
		{
			case 0: return FIFOO[(addr>>4)&0xf].StartAdr+PTRO[(addr>>4)&0xf];
			case 4: return FIFOO[(addr>>4)&0xf].StartLen-PTRO[(addr>>4)&0xf];
			case 8: return FIFOO[(addr>>4)&0xf].NextAdr;
			case 0xc:return FIFOO[(addr>>4)&0xf].NextLen;
		}

	}

	return 0; // ??? it's possible?

}
