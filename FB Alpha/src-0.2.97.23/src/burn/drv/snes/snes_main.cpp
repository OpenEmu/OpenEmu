#include <stdio.h>
#include "snes.h"

static INT32 frames;
INT32 intthisline;
INT32 framenum;
INT32 oldnmi=0;
INT32 spcclck;
double spcclck2,spcclck3;

void __cdecl snemlog(TCHAR *format,...)
{
	bprintf(0,format);
}


void SnesReset()
{
	resetppu();
	resetspc();
	reset65816();
	frames=0;
}

INT32 SnesInit()
{
	INT32 nret = 0;
	INT32 counter=0;
	char name[22];
	INT32 len;
	UINT16 temp,temp2;
		
	allocmem();
	initppu();
	initspc();
	makeopcodetable();
//	initdsp();

	SnesReset();


	spccycles=-10000;

	rom=(UINT8*)BurnMalloc(4096*1024);
	BurnLoadRom(rom,0,0);


	temp=rom[0x7FDC]|(rom[0x7FDD]<<8);
	temp2=rom[0x7FDE]|(rom[0x7FDF]<<8);
	if ((temp|temp2)==0xFFFF)
	{
		lorom=1;
	}
	else
	{
		lorom=0;
	}
	
	initmem();
	
	if (((snes_readmem(0xFFFD)<<8)|snes_readmem(0xFFFC))==0xFFFF) 
	{ 
		lorom^=1; 
		initmem(); 
	}

	len=counter;//-0x10000;
	for (counter=0;counter<21;counter++)
	{
		name[counter]=snes_readmem(0xFFC0+counter);
	}
	
	name[21]=0;
	srammask=(1<<(snes_readmem(0xFFD8)+10))-1;
	
	if (!snes_readmem(0xFFD8)) 
	{
		srammask=0;
	}

	if (snes_readmem(0xFFD9)>1) 
	{
		global_pal=1;
	}
	else
	{
		global_pal=0;
	}

	if (srammask)
	{
		memset(sram,0,srammask+1);
	}

	memset(ram,0x55,128*1024);

	SnesReset();
	
	return nret;
}

INT32 SnesExit()
{
	freemem();
	return 0;
}

INT32 SnesFrame()
{
	if (DoSnesReset)
	{
		SnesReset();
	}

	nmi=vbl=0;
	framenum++;
	if (framenum==50)
	{
		spcclck=spctotal;
		spcclck2=spctotal2;
		spcclck3=spctotal3;
		spctotal=0;
		spctotal2=0.0f;
		spctotal3=0.0f;
	}
	for (lines=0;lines<((global_pal)?312:262);lines++)
	{
		//                snemlog("%i %02X:%04X %i %i %i\n",lines,pbr>>16,pc,irqenable,xirq,yirq);
		if ((irqenable==2/* || irqenable==1*/) && (lines==yirq))
		{ 
			irq=1; 
			/*snemlog("Raise IRQ line %i %02X\n",lines,lines);*/ 
		}
		if (lines<225) 
		{
			drawline(lines);
		}
		cycles+=1364;
		intthisline=0;
		while (cycles>0)
		{
			global_opcode=snes_readmem(pbr|pc); 
			pc++;
			opcodes[global_opcode ][cpumode]();
			if ((((irqenable==3) && (lines==yirq)) || (irqenable==1)) && !intthisline)
			{
				if (((1364-cycles)>>2)>=xirq)
				{
					irq=1;
					intthisline=1;
					//                                        snemlog("Raise IRQ horiz %i %i\n",lines,irqenable);
				}
			}
			ins++;
			if (oldnmi!=nmi && nmienable && nmi)
			{
				nmi65816();
			}
			else if (irq && (!p.i || inwai))
			{
				irq65816();
			}
			oldnmi=nmi;
		}
		if (lines==0xE0) 
		{
			nmi=1;
			vbl=joyscan=1;
			readjoy();
		}
		if (lines==0xE3) 
		{
			joyscan=0;
		}

	}
	frames++;

	return 0;
}

INT32 SnesScan(INT32,INT32 *)
{
	return 0;
}
