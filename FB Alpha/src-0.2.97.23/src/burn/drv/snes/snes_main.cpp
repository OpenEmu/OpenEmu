#include <stdio.h>
#include "snes.h"

static INT32 frames;
INT32 intthisline;
INT32 framenum;
INT32 oldnmi=0;
INT32 spcclck;
double spcclck2,spcclck3;

extern int pal;
UINT16 srammask=0;


static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;

UINT8 *SNES_sram;
UINT8 *SNES_ram;
UINT8 *SNES_rom;
UINT8 *memlookup[2048];
UINT8 *memread;
UINT8 *memwrite;
UINT8 *accessspeed;
INT32 lorom;


static INT32 MemIndex(UINT32 cart_size)
{
	UINT8 *Next; Next = AllMem;

	SNES_rom	= Next; Next += cart_size;
	// Technically these are not ram but need to be saved with machine state but not reset with machine
	memread = Next; Next += 0x800;
	memwrite = Next; Next += 0x800;
	accessspeed =  Next; Next += 0x800;
	AllRam		= Next;
	// but these really are ram
	SNES_ram	= Next; Next += 0x20000;
	SNES_sram	= Next; Next += 0x2000; 
	RamEnd		= Next;
	MemEnd		= Next;

	return 0;
}




void __cdecl snemlog(TCHAR *format,...)
{
	bprintf(0,format);
}

void snes_mapmem()
{

	int c,d;
	for (c=0;c<0x800;c++)
	{
		memread[c]=0;
		memwrite[c]=0;
	}
	if (lorom)
	{
		for (c=0;c<96;c++)
		{
			for (d=0;d<4;d++)
			{
				memread[(c<<3)|(d+4)]=1;
				memlookup[(c<<3)|(d+4)]=&SNES_rom[((d*0x2000)+(c*0x8000))&0x3FFFFF];
				memread[(c<<3)|(d+4)|0x400]=1;
				memlookup[(c<<3)|(d+4)|0x400]=&SNES_rom[((d*0x2000)+(c*0x8000))&0x3FFFFF];

			}
			
		}
		
		for (c=0;c<64;c++)
		{
			memread[(c<<3)|0]=memwrite[(c<<3)|0]=1;
			memlookup[(c<<3)|0]=SNES_ram;
		}
		
		for (c=0;c<64;c++)
		{
			memread[(c<<3)|0x400]=memwrite[(c<<3)|0x400]=1;
			memlookup[(c<<3)|0x400]=SNES_ram;
		}
		
		for (c=0;c<8;c++)
		{
			memread[(0x7E<<3)|c]=memwrite[(0x7E<<3)|c]=1;
			memlookup[(0x7E<<3)|c]=&SNES_ram[c*0x2000];
			memread[(0x7F<<3)|c]=memwrite[(0x7F<<3)|c]=1;
			memlookup[(0x7F<<3)|c]=&SNES_ram[(c*0x2000)+0x10000];
		}
	
	}
	else
	{
		for (c=0;c<2048;c++)
		{
			memread[c]=1;
			memwrite[c]=0;
			memlookup[c]=&SNES_rom[(c*0x2000)&0x3FFFFF];
		}
		for (c=0;c<64;c++)
		{
			for (d=1;d<4;d++)
			{
				memread[(c<<3)+d]=memwrite[(c<<3)+d]=0;
				memread[(c<<3)+d+1024]=memwrite[(c<<3)+d+1024]=0;
			}
		}
		for (c=0;c<64;c++)
		{
			memread[(c<<3)|0]=memwrite[(c<<3)|0]=1;
			memlookup[(c<<3)|0]=SNES_ram;
			memread[(c<<3)|1024]=memwrite[(c<<3)|1024]=1;
			memlookup[(c<<3)|1024]=SNES_ram;
		}
		for (c=0;c<8;c++)
		{
			memread[(0x7E<<3)|c]=memwrite[(0x7E<<3)|c]=1;
			memlookup[(0x7E<<3)|c]=&SNES_ram[c*0x2000];
			memread[(0x7F<<3)|c]=memwrite[(0x7F<<3)|c]=1;
			memlookup[(0x7F<<3)|c]=&SNES_ram[(c*0x2000)+0x10000];
		}
		for (c=0;c<16;c++)
		{
			memread[(0x70<<3)+c]=memwrite[(0x70<<3)+c]=1;
			memlookup[(0x70<<3)+c]=SNES_sram;
		}
	}
	/*Set up access speed table*/
	for (c=0;c<64;c++)
	{
		accessspeed[(c<<3)|0]=8;
		accessspeed[(c<<3)|1]=6;
		accessspeed[(c<<3)|2]=6;
		accessspeed[(c<<3)|3]=6;
		accessspeed[(c<<3)|4]=accessspeed[(c<<3)|5]=8;
		accessspeed[(c<<3)|6]=accessspeed[(c<<3)|7]=8;
	}
	for (c=64;c<128;c++)
	{
		for (d=0;d<8;d++)
		{
			accessspeed[(c<<3)|d]=8;
		}
	}
	for (c=128;c<192;c++)
	{
		accessspeed[(c<<3)|0]=8;
		accessspeed[(c<<3)|1]=6;
		accessspeed[(c<<3)|2]=6;
		accessspeed[(c<<3)|3]=6;
		accessspeed[(c<<3)|4]=accessspeed[(c<<3)|5]=8;
		accessspeed[(c<<3)|6]=accessspeed[(c<<3)|7]=8;
	}
	for (c=192;c<256;c++)
	{
		for (d=0;d<8;d++)
		{
			accessspeed[(c<<3)|d]=8;
		}
	}
}

unsigned char snes_readmem(unsigned long addr)
{
	cycles-=accessspeed[(addr>>13)&0x7FF];
	clockspc(accessspeed[(addr>>13)&0x7FF]);
	if (memread[(addr>>13)&0x7FF])
	{
		return memlookup[(addr>>13)&0x7FF][addr&0x1FFF];
	}
	
	addr&=~0xFF000000;

	if (((addr>>16)&0x7F)<0x40)
	{
		switch (addr&0xF000)
		{
			case 0x2000:
				return readppu(addr);
			case 0x4000:
				if ((addr&0xE00)==0x200)
				{
					return readio(addr);
				}
				if ((addr&0xFFFE)==0x4016)
				{
					return readjoyold(addr);
				}
				//snemlog(L"Bad Read %06X\n",addr);
				return 0;
			case 0x6000: 
			case 0x7000:
				if (!lorom) 
				{
					return SNES_sram[addr&srammask];
				}
			default:
				//snemlog(L"Bad read %06X\n",addr);
				return 0xFF;
		}
	}

	if ((addr>>16)>=0xD0 && (addr>>16)<=0xFE) 
	{
		return 0;
	}

	if ((addr>>16)==0x70)
	{
		if (srammask)
		{
			return SNES_sram[addr&srammask];
		}
		return 0;
	}

	if ((addr>>16)==0x60) 
	{
		return 0;
	}

	//snemlog(L"Bad read %06X\n",addr);

	return 0xff;
}

void snes_writemem(unsigned long addr, unsigned char val)
{
	cycles-=accessspeed[(addr>>13)&0x7FF];
	clockspc(accessspeed[(addr>>13)&0x7FF]);
	if (memwrite[(addr>>13)&0x7FF])
	{
		memlookup[(addr>>13)&0x7FF][(addr)&0x1FFF]=val;
		return;
	}
	
	addr&=~0xFF000000;
	if (((addr>>16)&0x7F)<0x40)
	{
		switch (addr&0xF000)
		{

		case 0x2000:
			writeppu(addr,val);
			return;

		case 0x3000:
			return;

		case 0x4000:
			if ((addr&0xE00)==0x200)
				writeio(addr,val);
			if ((addr&0xFFFE)==0x4016)
				writejoyold(addr,val);
			return;

		case 0x5000: 
			return;

		case 0x6000: 
		case 0x7000:
			if (!lorom)
			{			
				SNES_sram[addr&srammask]=val;
			}
			return;
		case 0x8000: 
		case 0x9000: 
		case 0xA000: 
		case 0xB000:
		case 0xC000: 
		case 0xD000: 
		case 0xE000: 
		case 0xF000:
			return;
		default:
			break;
			//snemlog(L"Bad write %06X %02X\n",addr,val);

		}
	}
	
	if ((addr>>16)>=0xD0 && (addr>>16)<=0xFE) 
	{
		return;
	}

	if ((addr>>16)==0x70)
	{
		SNES_sram[addr&srammask]=val;
		return;
	}
}


void SnesReset()
{
	memset (AllRam, 0, RamEnd - AllRam);
	resetppu();
	resetspc();
	reset65816();
	frames=0;
}

INT32 SnesInit()
{
	INT32 nret = 0;

	UINT16 temp,temp2;
	
	struct BurnRomInfo ri;
	BurnDrvGetRomInfo(&ri, 0);
	UINT32 length = ri.nLen;
	
	AllMem = NULL;
	MemIndex(length);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex(length);
	
	initppu();
	initspc();
	makeopcodetable();

	SnesReset();

	spccycles=-10000;


	BurnLoadRom(SNES_rom,0,0);


	temp=SNES_rom[0x7FDC]|(SNES_rom[0x7FDD]<<8);
	temp2=SNES_rom[0x7FDE]|(SNES_rom[0x7FDF]<<8);
	if ((temp|temp2)==0xFFFF)
	{
		lorom=1;
	}
	else
	{
		lorom=0;
	}
	
	snes_mapmem();
	
	if (((snes_readmem(0xFFFD)<<8)|snes_readmem(0xFFFC))==0xFFFF) 
	{ 
		lorom^=1; 
		snes_mapmem(); 
	}

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
		memset(SNES_sram,0,srammask+1);
	}

	memset(SNES_ram,0x55,128*1024);

	SnesReset();
	
	return nret;
}

INT32 SnesExit()
{
	for (int i=0;i<2048;i++)
	{
		memlookup[i]=NULL;
	}
	BurnFree (AllMem);
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
