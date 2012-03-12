#include <stdio.h>
#include "snes.h"

extern int pal;
UINT16 srammask=0;

UINT8 *sram;
UINT8 *ram;
UINT8 *rom;
UINT8 *memlookup[2048];
UINT8 memread[2048];
UINT8 memwrite[2048];
UINT8 accessspeed[2048];
INT32 lorom;

void allocmem()
{
	ram=(UINT8*)BurnMalloc(128*1024);
	memset(ram,0x55,128*1024);
	sram=(UINT8*)BurnMalloc(8192);
	memset(sram,0,8192);
}

void freemem()
{
	for (int i=0;i<2048;i++)
	{
		memlookup[i]=NULL;
		memread[i]=0;
		memwrite[i]=0;
		accessspeed[i]=0;

	}
	if (ram !=NULL)
	{
		BurnFree(ram);
	}
	if (sram!=NULL)
	{
		BurnFree(sram);
	}
	if (rom!=NULL)
	{
		BurnFree(rom);
	}
}


void initmem()
{

	int c,d;
	for (c=0;c<256;c++)
	{
		for (d=0;d<8;d++)
		{
			memread[(c<<3)|d]=memwrite[(c<<3)|d]=0;
		}
	}
	if (lorom)
	{
		for (c=0;c<96;c++)
		{
			for (d=0;d<4;d++)
			{
				memread[(c<<3)|(d+4)]=1;
				memlookup[(c<<3)|(d+4)]=&rom[((d*0x2000)+(c*0x8000))&0x3FFFFF];
				memread[(c<<3)|(d+4)|0x400]=1;
				memlookup[(c<<3)|(d+4)|0x400]=&rom[((d*0x2000)+(c*0x8000))&0x3FFFFF];
			}
		}
		for (c=0;c<64;c++)
		{
			memread[(c<<3)|0]=memwrite[(c<<3)|0]=1;
			memlookup[(c<<3)|0]=ram;
		}
		for (c=0;c<64;c++)
		{
			memread[(c<<3)|0x400]=memwrite[(c<<3)|0x400]=1;
			memlookup[(c<<3)|0x400]=ram;
		}
		for (c=0;c<8;c++)
		{
			memread[(0x7E<<3)|c]=memwrite[(0x7E<<3)|c]=1;
			memlookup[(0x7E<<3)|c]=&ram[c*0x2000];
			memread[(0x7F<<3)|c]=memwrite[(0x7F<<3)|c]=1;
			memlookup[(0x7F<<3)|c]=&ram[(c*0x2000)+0x10000];
		}
	
	}
	else
	{
		for (c=0;c<2048;c++)
		{
			memread[c]=1;
			memwrite[c]=0;
			memlookup[c]=&rom[(c*0x2000)&0x3FFFFF];
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
			memlookup[(c<<3)|0]=ram;
			memread[(c<<3)|1024]=memwrite[(c<<3)|1024]=1;
			memlookup[(c<<3)|1024]=ram;
		}
		for (c=0;c<8;c++)
		{
			memread[(0x7E<<3)|c]=memwrite[(0x7E<<3)|c]=1;
			memlookup[(0x7E<<3)|c]=&ram[c*0x2000];
			memread[(0x7F<<3)|c]=memwrite[(0x7F<<3)|c]=1;
			memlookup[(0x7F<<3)|c]=&ram[(c*0x2000)+0x10000];
		}
		for (c=0;c<16;c++)
		{
			memread[(0x70<<3)+c]=memwrite[(0x70<<3)+c]=1;
			memlookup[(0x70<<3)+c]=sram;
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

unsigned char readmeml(unsigned long addr)
{
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
					return sram[addr&srammask];
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
			return sram[addr&srammask];
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

void writememl(unsigned long addr, unsigned char val)
{
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
			if (!lorom) sram[addr&srammask]=val;
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
		sram[addr&srammask]=val;
		return;
	}

	//snemlog(L"Bad write %06X %02X\n",addr,val);

}

unsigned char snes_readmem(unsigned long adress)
{
	cycles-=accessspeed[(adress>>13)&0x7FF];
	clockspc(accessspeed[(adress>>13)&0x7FF]);
	if (memread[(adress>>13)&0x7FF])
	{
		return memlookup[(adress>>13)&0x7FF][adress&0x1FFF];
	}
	return readmeml(adress);
}

void snes_writemem(unsigned long ad, unsigned char v)
{
	cycles-=accessspeed[(ad>>13)&0x7FF];
	clockspc(accessspeed[(ad>>13)&0x7FF]);
	if (memwrite[(ad>>13)&0x7FF])
		memlookup[(ad>>13)&0x7FF][(ad)&0x1FFF]=v;
	else
		writememl(ad,v);
}
