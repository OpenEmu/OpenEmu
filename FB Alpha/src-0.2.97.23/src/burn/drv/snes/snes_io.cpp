/*Snem 0.1 by Tom Walker
I/O, or $42xx*/

#include "snes.h"

extern int intthisline;
int dmaops=0;
extern int framenum;
extern int oldnmi;

int padpos,padstat;
unsigned short pad[4];

void readjoy()
{
	pad[0]=0;
	if (SnesJoy1[6])      
		pad[0]|=0x8000;
	if (SnesJoy1[4])
		pad[0]|=0x4000;
	if (SnesJoy1[11]) 
		pad[0]|=0x2000;
	if (SnesJoy1[7]) 
		pad[0]|=0x1000;
	if (SnesJoy1[0]) 
		pad[0]|=0x0800;
	if (SnesJoy1[1]) 
		pad[0]|=0x0400;
	if (SnesJoy1[2]) 
		pad[0]|=0x0200;
	if (SnesJoy1[3]) 
		pad[0]|=0x0100;
	if (SnesJoy1[5])
		pad[0]|=0x0080;
	if (SnesJoy1[10])
		pad[0]|=0x0040;
	if (SnesJoy1[9]) 
		pad[0]|=0x0020;
	if (SnesJoy1[8])  
		pad[0]|=0x0010;
	padpos=16;
	//        printf("Read joy\n");
}

unsigned char readjoyold(unsigned short addr)
{
	int temp;
	if (addr==0x4016)
	{
		temp=pad[0]>>(padpos^15);
		if (!(padstat&1)) padpos++;
		if (padpos>15) temp=1;
		//                printf("Read 4016 %i %i %02X %06X\n",temp&1,padpos,padstat,pbr|pc);
		return temp&1;
	}
	return 0xFF;
}

void writejoyold(unsigned short addr, unsigned char val)
{
	if (addr==0x4016)
	{
		//                printf("Write 4016 %02X\n",val);
		if ((val&1) && !(padstat&1)) padpos=0;
		padstat=val;
	}
}

unsigned short mulr,divc,divr;
unsigned char mula,mulb,divb;

void writeio(unsigned short addr, unsigned char val)
{
	int c,d=0,offset=0,speed;
	unsigned char temp;
	switch (addr&0x1FF)
	{
	case 0x00:
		nmienable=val&0x80;
		irqenable=(val>>4)&3;
		if (!irqenable) 
		{
			irq=0;
		}
		break;
	case 0x02: /*Multiplicand A*/
		mula=val;
		return;
	case 0x03: /*Multiplier B*/
		mulb=val;
		mulr=mula*mulb;
		return;
	case 0x04: /*Dividend C Low*/
		divc&=0xFF00;
		divc|=val;
		return;
	case 0x05: /*Dividend C High*/
		divc&=0xFF;
		divc|=(val<<8);
		return;
	case 0x06: /*Divisor B*/
		divb=val;
		if (divb)
		{
			divr=divc/divb;
			mulr=divc%divb;
		}
		else
		{
			divr=0xFFFF;
			mulr=divc;
		}
		return;
	case 0x07: /*X low*/
		xirq=(xirq&0x100)|val;
		intthisline=0;
		break;
	case 0x08: /*X high*/
		xirq=(xirq&0xFF)|(val&0x100);
		intthisline=0;
		break;
	case 0x09: /*Y low*/
		yirq=(yirq&0x100)|val;
		break;
	case 0x0A: /*Y high*/
		yirq=(yirq&0xFF)|(val&0x100);
		break;
	case 0x0B: /*DMA enable*/
		for (c=1;c<0x100;c<<=1)
		{
			if (val&c)
			{
				/*                                if (getvramaddr()==0xB000)
				{
				dmaops++;
				if (dmaops==3) return;
				}*/
				//                                if (!dmalen[d]) printf("DMA %i ctrl %02X src %06X dest %02X size %04X %i\n",d,dmactrl[d],(dmabank[d]<<16)|dmasrc[d],dmadest[d],dmalen[d],dmaops);
				do
				{
					if (dmactrl[d]&0x80)
					{
						//                                                printf("Dest %04X+%04X SRC %06X %04X\n",dmadest[d],offset,dmabank[d],dmasrc[d]);
						temp=readppu(dmadest[d]+offset);
						snes_writemem((dmabank[d]<<16)|dmasrc[d],temp);
					}
					else
					{
						temp=snes_readmem((dmabank[d]<<16)|dmasrc[d]);
						//                                                if (dmabank[d]==0 && dmasrc[d]<0x2000) printf("%02X %06X %04X\n",temp,dmabank[d]|dmasrc[d],dmalen[d]);
						writeppu(dmadest[d]+offset,temp);
					}
					if (!(dmactrl[d]&8))
					{
						if (dmactrl[d]&0x10) dmasrc[d]--;
						else                 dmasrc[d]++;
					}
					switch (dmactrl[d]&7)
					{
					case 0: /*1 address write twice*/
					case 2: /*1 address write once*/
						break;
					case 1: /*2 addresses*/
						offset++;
						offset&=1;
						break;

					default:
						break;
					}
					dmalen[d]--;

				} while (dmalen[d]!=0);
			}
			d++;
		}
		break;
	case 0x0C: /*HDMA enable*/
		hdmaena=val;
		//                printf("HDMA ena : %02X %06X %i %i\n",val,pbr|pc,framenum,lines);
		break;
	case 0x0D: /*ROM speed select*/
		if (val&1) speed=6;
		else       speed=8;
		for (c=192;c<256;c++)
		{
			for (d=0;d<8;d++)
			{
				accessspeed[(c<<3)|d]=speed;
			}
		}
		for (c=128;c<192;c++)
		{
			accessspeed[(c<<3)|4]=accessspeed[(c<<3)|5]=speed;
			accessspeed[(c<<3)|6]=accessspeed[(c<<3)|7]=speed;
		}
		break;
	case 0x100: case 0x110: case 0x120: case 0x130: /*DMA control*/
	case 0x140: case 0x150: case 0x160: case 0x170:
		dmactrl[(addr>>4)&7]=val;
		//                printf("Write ctrl %i %02X %06X\n",(addr>>4)&7,val,pbr|pc);
		break;
	case 0x101: case 0x111: case 0x121: case 0x131: /*DMA dest*/
	case 0x141: case 0x151: case 0x161: case 0x171:
		dmadest[(addr>>4)&7]=val|0x2100;
		break;
	case 0x102: case 0x112: case 0x122: case 0x132: /*DMA src low*/
	case 0x142: case 0x152: case 0x162: case 0x172:
		dmasrc[(addr>>4)&7]=(dmasrc[(addr>>4)&7]&0xFF00)|val;
		break;
	case 0x103: case 0x113: case 0x123: case 0x133: /*DMA src high*/
	case 0x143: case 0x153: case 0x163: case 0x173:
		dmasrc[(addr>>4)&7]=(dmasrc[(addr>>4)&7]&0xFF)|(val<<8);
		break;
	case 0x104: case 0x114: case 0x124: case 0x134: /*DMA src bank*/
	case 0x144: case 0x154: case 0x164: case 0x174:
		dmabank[(addr>>4)&7]=val;
		break;
	case 0x105: case 0x115: case 0x125: case 0x135: /*DMA size low*/
	case 0x145: case 0x155: case 0x165: case 0x175:
		dmalen[(addr>>4)&7]=(dmalen[(addr>>4)&7]&0xFF00)|val;
		break;
	case 0x106: case 0x116: case 0x126: case 0x136: /*DMA size high*/
	case 0x146: case 0x156: case 0x166: case 0x176:
		dmalen[(addr>>4)&7]=(dmalen[(addr>>4)&7]&0xFF)|(val<<8);
		break;
	case 0x107: case 0x117: case 0x127: case 0x137: /*HDMA ibank*/
	case 0x147: case 0x157: case 0x167: case 0x177:
		dmaibank[(addr>>4)&7]=val;
		break;
	case 0x108: case 0x118: case 0x128: case 0x138:
	case 0x148: case 0x158: case 0x168: case 0x178:
		hdmaaddr[(addr>>4)&7]=(hdmaaddr[(addr>>4)&7]&0xFF00)|val;
		//                printf("HDMA addr %i now %04X %06X\n",(addr>>4)&7,hdmaaddr[(addr>>4)&7],pbr|pc);
		break;
	case 0x109: case 0x119: case 0x129: case 0x139:
	case 0x149: case 0x159: case 0x169: case 0x179:
		hdmaaddr[(addr>>4)&7]=(hdmaaddr[(addr>>4)&7]&0xFF)|(val<<8);
		//                printf("HDMA addr %i now %04X %06X\n",(addr>>4)&7,hdmaaddr[(addr>>4)&7],pbr|pc);
		break;
	case 0x10A: case 0x11A: case 0x12A: case 0x13A: /*HDMA lines left*/
	case 0x14A: case 0x15A: case 0x16A: case 0x17A:
		hdmacount[(addr>>4)&7]=val;
		break;
	}
}

unsigned char readio(unsigned short addr)
{
	int temp=0;
	if (addr==0x4016 || addr==0x4017)
	{
		//                printf("Read oldstyle joypad\n");
		//                dumpregs();
		//                exit(-1);
	}
	//        if (addr!=0x4218 && addr!=0x4219) snemlog("Read IO %04X %02X:%04X %04X\n",addr,pbr>>16,pc,dp);
	switch (addr&0x1FF)
	{
	case 0:
		return 0;

	case 0xB: /*DMA enable*/
		return 0;
	case 0x0C: /*HDMA enable*/
		return hdmaena;

	case 0x10: /*NMI*/
		if (nmi) temp=0x80;
		nmi=oldnmi=0;
		return temp;

	case 0x11: /*IRQ*/
		if (irq) temp=0x80;
		irq=0;
		return temp;

	case 0x12: /*VBL*/
		if (vbl) temp|=0x80;
		if (joyscan) temp|=1;
		if (cycles<340) temp|=0x40; /*340=1364-(256 pixels * 4)*/
		return temp;

	case 0x13: /*I/O port*/
		return 0;

	case 0x14: /*Division Result Low*/
		//                printf("Read div low\n");
		return divr;
	case 0x15: /*Division Result High*/
		//                printf("Read div high\n");
		return divr>>8;
	case 0x16: /*Multiplication Result Low*/
		//                printf("Read mul low\n");
		return mulr;
	case 0x17: /*Multiplication Result High*/
		//                printf("Read mul high\n");
		return mulr>>8;

	case 0x18: /*Joypad #1*/
		//                printf("Read joy low %02X %06X %i\n",pad[0]>>8,pbr|pc,ins);
		return pad[0]&0xFF;
	case 0x19:
		//                printf("Read joy high %02X %06X\n",pad[0]&0xFF,pbr|pc);
		return pad[0]>>8;

	case 0x1A: case 0x1B: /*Joypad #2*/
	case 0x1C: case 0x1D: /*Joypad #3*/
	case 0x1E: case 0x1F: /*Joypad #4*/
		return 0;

	case 0x100: case 0x110: case 0x120: case 0x130: /*DMA control*/
	case 0x140: case 0x150: case 0x160: case 0x170:
		return dmactrl[(addr>>4)&7];
		break;
	case 0x101: case 0x111: case 0x121: case 0x131: /*DMA dest*/
	case 0x141: case 0x151: case 0x161: case 0x171:
		return dmadest[(addr>>4)&7]&0xFF;
		break;
	case 0x102: case 0x112: case 0x122: case 0x132: /*DMA src low*/
	case 0x142: case 0x152: case 0x162: case 0x172:
		return dmasrc[(addr>>4)&7]&0xFF;
		break;
	case 0x103: case 0x113: case 0x123: case 0x133: /*DMA src high*/
	case 0x143: case 0x153: case 0x163: case 0x173:
		return dmasrc[(addr>>4)&7]>>8;
		break;
	case 0x104: case 0x114: case 0x124: case 0x134: /*DMA src bank*/
	case 0x144: case 0x154: case 0x164: case 0x174:
		return dmabank[(addr>>4)&7];
		break;
	case 0x105: case 0x115: case 0x125: case 0x135: /*DMA size low*/
	case 0x145: case 0x155: case 0x165: case 0x175:
		return dmalen[(addr>>4)&7]&0xFF;
		break;
	case 0x106: case 0x116: case 0x126: case 0x136: /*DMA size high*/
	case 0x146: case 0x156: case 0x166: case 0x176:
		return dmalen[(addr>>4)&7]>>8;
		break;
	case 0x107: case 0x117: case 0x127: case 0x137: /*HDMA indirect bank*/
	case 0x147: case 0x157: case 0x167: case 0x177:
		return dmaibank[(addr>>4)&7];//>>8;
		break;

	default:
		return 0;
	}
}

