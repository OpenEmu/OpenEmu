#include <stdio.h>
#include "snes.h"

int spcoutput;

int spctotal,dsptotal;

double spccycles;
double spctotal2;
double spctotal3;

struct SPC_Struct
{
	union
	{
		unsigned short w;
		struct
		{
			unsigned char a,y;
		} b;
	} ya;
	unsigned char x;
	unsigned char s;
	unsigned short pc;
	struct SPC_P
	{
		int n,v,p,b,h,i,z,c;
	} p;
} spc;


unsigned char *spcram;
unsigned char spcrom[64]=
{
	0xCD,0xEF,0xBD,0xE8,0x00,0xC6,0x1D,0xD0,
	0xFC,0x8F,0xAA,0xF4,0x8F,0xBB,0xF5,0x78,
	0xCC,0xF4,0xD0,0xFB,0x2F,0x19,0xEB,0xF4,
	0xD0,0xFC,0x7E,0xF4,0xD0,0x0B,0xE4,0xF5,
	0xCB,0xF4,0xD7,0x00,0xFC,0xD0,0xF3,0xAB,
	0x01,0x10,0xEF,0x7E,0xF4,0x10,0xEB,0xBA,
	0xF6,0xDA,0x00,0xBA,0xF4,0xC4,0xF4,0xDD,
	0x5D,0xD0,0xDB,0x1F,0x00,0x00,0xC0,0xFF
};

unsigned char *spcreadhigh;
unsigned char spctocpu[4];
unsigned char dspaddr;
unsigned char voiceon;
int spctimer[3],spctimer2[3],spclimit[3];

unsigned char readfromspc(unsigned short addr)
{
	return spctocpu[addr&3];
}

void writetospc(unsigned short addr, unsigned char val)
{
	spcram[(addr&3)+0xF4]=val;
}



void writespcregs(unsigned short a, unsigned char v)
{
	switch (a)
	{
	case 0xF1:
		if (v&0x10) spcram[0xF4]=spcram[0xF5]=0;
		if (v&0x20) spcram[0xF6]=spcram[0xF7]=0;
		spcram[0xF1]=v;
		if (v&0x80) spcreadhigh=spcrom;
		else        spcreadhigh=spcram+0xFFC0;
		//                printf("Write F1 %02X %04X\n",v,spc.pc);
		break;
	case 0xF4: case 0xF5: case 0xF6: case 0xF7:
		spctocpu[a&3]=v;
		//                printf("SPC writes %02X to %02X\n",v,a);
		break;
	case 0xFA: case 0xFB: case 0xFC:
		spclimit[a-0xFA]=v;
		//                printf("SPC limit %i = %02X\n",a-0xFA,v);
		break;
	case 0xFD: case 0xFE: case 0xFF:
		spcram[a]=v;
		break;
	}
}

unsigned short getspcpc() 
{ 
	return spc.pc; 
}
unsigned char readspcregs(unsigned short a)
{
	unsigned char v;
	switch (a)
	{
	case 0xFD: case 0xFE: case 0xFF:
		//              printf("Read timer %04X\n",spc.pc);
		v=spcram[a];
		spcram[a]=0;
		return v;
	}
	return spcram[a];
}

#define readspc(a)  ((((a)&0xFFC0)==0xFFC0)?spcreadhigh[(a)&63]:((((a)&0xFFF0)==0xF0)?readspcregs(a):spcram[a]))
#define writespc(a,v) /*if ((a)==0x17 || (a)==0x221) printf("Write %04X %02X %04X\n",a,v,spc.pc); */if (((a)&0xFFF0)==0xF0) writespcregs(a,v); else spcram[a]=v

void initspc()
{
	spcram=(unsigned char*)malloc(65536);
	memset(spcram,0,65536);
}

void resetspc()
{
	memset(&spc,0,sizeof(spc));
	spc.pc=0xFFC0;
	spcreadhigh=spcrom;
	spc.x=spc.ya.w=0;
	spccycles=0;
	spc.p.p=0;
	spctimer[0]=spctimer[1]=spctimer[2]=0;
	spctimer2[0]=spctimer2[1]=spctimer2[2]=0;
	spcram[0xF1]=0x80;
}

#define getdp()  addr=readspc(spc.pc)|spc.p.p; spc.pc++
#define getdpx() addr=((readspc(spc.pc)+spc.x)&0xFF)|spc.p.p; spc.pc++
#define getdpy() addr=((readspc(spc.pc)+spc.ya.b.y)&0xFF)|spc.p.p; spc.pc++
#define getabs()  addr=readspc(spc.pc)|(readspc(spc.pc+1)<<8); spc.pc+=2

#define setspczn(v)   spc.p.z=!(v); spc.p.n=(v)&0x80
#define setspczn16(v) spc.p.z=!(v); spc.p.n=(v)&0x8000

#define spccycles(c) spccount+=c

#define ADC(ac,temp) tempw=ac+temp+((spc.p.c)?1:0);                  \
	spc.p.v=(!((ac^temp)&0x80)&&((ac^tempw)&0x80)); \
	ac=tempw&0xFF;                                  \
	setspczn(ac);                                   \
	spc.p.c=tempw&0x100;

#define SBC(ac,temp) tempw=ac-temp-((spc.p.c)?0:1);                 \
	spc.p.v=(((ac^temp)&0x80)&&((ac^tempw)&0x80)); \
	ac=tempw&0xFF;                                 \
	setspczn(ac);                                  \
	spc.p.c=tempw<=0xFF;

unsigned short spc2,spc3;
void execspc()
{
	unsigned char opcode,temp,temp2;
	unsigned short addr,addr2,tempw;
	unsigned long templ;
	int spccount;
	//        snemlog("ExecSPC %i\n",spccycles);
	while (spccycles>0)
	{
		spc3=spc2;
		spc2=spc.pc;
		spccount=0;
		opcode=readspc(spc.pc); spc.pc++;
		switch (opcode)
		{
		case 0x00: /*NOP*/
			spccycles(2);
			break;

		case 0x02: /*SET1 d.0*/
			getdp();
			temp=readspc(addr)|0x01;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0x22: /*SET1 d.1*/
			getdp();
			temp=readspc(addr)|0x02;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0x42: /*SET1 d.2*/
			getdp();
			temp=readspc(addr)|0x04;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0x62: /*SET1 d.3*/
			getdp();
			temp=readspc(addr)|0x08;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0x82: /*SET1 d.4*/
			getdp();
			temp=readspc(addr)|0x10;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0xA2: /*SET1 d.5*/
			getdp();
			temp=readspc(addr)|0x20;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0xC2: /*SET1 d.6*/
			getdp();
			temp=readspc(addr)|0x40;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0xE2: /*SET1 d.7*/
			getdp();
			temp=readspc(addr)|0x80;
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x12: /*CLR1 d.0*/
			getdp();
			temp=readspc(addr)&~0x01;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0x32: /*CLR1 d.1*/
			getdp();
			temp=readspc(addr)&~0x02;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0x52: /*CLR1 d.2*/
			getdp();
			temp=readspc(addr)&~0x04;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0x72: /*CLR1 d.3*/
			getdp();
			temp=readspc(addr)&~0x08;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0x92: /*CLR1 d.4*/
			getdp();
			temp=readspc(addr)&~0x10;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0xB2: /*CLR1 d.5*/
			getdp();
			temp=readspc(addr)&~0x20;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0xD2: /*CLR1 d.6*/
			getdp();
			temp=readspc(addr)&~0x40;
			writespc(addr,temp);
			spccycles(4);
			break;
		case 0xF2: /*CLR1 d.7*/
			getdp();
			temp=readspc(addr)&~0x80;
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x04: /*OR A,(xx)*/
			getdp();
			spc.ya.b.a|=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(3);
			break;

		case 0x05: /*OR A,(xxxx)*/
			getabs();
			spc.ya.b.a|=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(4);
			break;

		case 0x06: /*OR A,(X)*/
			addr=spc.x+spc.p.p;
			spc.ya.b.a|=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(3);
			break;

		case 0x07: /*OR A,(xx+X)*/
			getdpx();
			addr2=readspc(addr)+(readspc(addr+1)<<8);
			spc.ya.b.a|=readspc(addr2);
			setspczn(spc.ya.b.a);
			spccycles(6);
			break;

		case 0x08: /*OR A,#*/
			spc.ya.b.a|=readspc(spc.pc); spc.pc++;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x09: /*OR (xx),(xx)*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(spc.pc)+spc.p.p; spc.pc++;
			temp=readspc(addr)|readspc(addr2);
			setspczn(temp);
			writespc(addr2,temp);
			spccycles(6);
			break;

		case 0x0B: /*ASL (xx)*/
			getdp();
			temp=readspc(addr);
			spc.p.c=temp&0x80;
			temp<<=1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x0C: /*ASL (xxxx)*/
			getabs();
			temp=readspc(addr);
			spc.p.c=temp&0x80;
			temp<<=1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x0D: /*PUSH PSW*/
			temp=(spc.p.c)?1:0;
			if (spc.p.z) temp|=0x02;
			if (spc.p.i) temp|=0x04;
			if (spc.p.h) temp|=0x08;
			if (spc.p.b) temp|=0x10;
			if (spc.p.p) temp|=0x20;
			if (spc.p.v) temp|=0x40;
			if (spc.p.n) temp|=0x80;
			writespc(spc.s+0x100,temp); spc.s--;
			spccycles(4);
			break;

		case 0x0E: /*TSET1 xxxx*/
			getabs();
			temp=readspc(addr);
			setspczn(spc.ya.b.a&temp);
			temp|=spc.ya.b.a;
			writespc(addr,temp);
			spccycles(6);
			break;

		case 0x10: /*BPL*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!spc.p.n)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(2);
			break;

		case 0x14: /*OR A,(xx+X)*/
			getdpx();
			spc.ya.b.a|=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(4);
			break;

		case 0x15: /*OR A,(xxxx+X)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			spc.ya.b.a|=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0x16: /*OR A,(xxxx+Y)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.ya.b.y; spc.pc+=2;
			spc.ya.b.a|=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0x17: /*OR A,(xx)+Y*/
			getdp();
			addr2=readspc(addr)+(readspc(addr+1)<<8)+spc.ya.b.y;
			spc.ya.b.a|=readspc(addr2);
			setspczn(spc.ya.b.a);
			spccycles(6);
			break;

		case 0x18: /*OR (xx),#*/
			temp2=readspc(spc.pc); spc.pc++;
			getdp();
			temp2|=readspc(addr);
			setspczn(temp2);
			writespc(addr,temp2);
			spccycles(5);
			break;

		case 0x1A: /*DECW xx*/
			getdp();
			tempw=(readspc(addr)+(readspc(addr+1)<<8))-1;
			writespc(addr,tempw&0xFF);
			writespc(addr+1,tempw>>8);
			spc.p.z=!tempw; spc.p.n=tempw&0x8000;
			spccycles(6);
			break;

		case 0x1B: /*ASL (xx+X)*/
			getdpx();
			temp=readspc(addr);
			spc.p.c=temp&0x80;
			temp<<=1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x1C: /*ASL A*/
			spc.p.c=spc.ya.b.a&0x80;
			spc.ya.b.a<<=1;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x1D: /*DEC X*/
			spc.x--;
			setspczn(spc.x);
			spccycles(2);
			break;

		case 0x1E: /*CMP X,(xxxx)*/
			getabs();
			temp=readspc(addr);
			setspczn(spc.x-temp);
			spc.p.c=(spc.x>=temp);
			spccycles(4);
			break;

		case 0x1F: /*JMP (xxxx+X)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			spc.pc=readspc(addr)|(readspc(addr+1)<<8);
			//                        printf("JMPx %06X\n",spc.pc);
			spccycles(6);
			break;

		case 0x20: /*CLRP*/
			spc.p.p=0;
			spccycles(2);
			break;

		case 0x24: /*AND A,(xx)*/
			getdp();
			spc.ya.b.a&=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(3);
			break;

		case 0x25: /*AND A,(xxxx)*/
			getabs();
			spc.ya.b.a&=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(4);
			break;

		case 0x26: /*AND A,(X)*/
			addr=spc.x+spc.p.p;
			spc.ya.b.a&=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(3);
			break;

		case 0x27: /*AND A,(xx+X)*/
			getdpx();
			addr2=readspc(addr)+(readspc(addr+1)<<8);
			spc.ya.b.a&=readspc(addr2);
			setspczn(spc.ya.b.a);
			spccycles(6);
			break;

		case 0x28: /*AND A,#*/
			spc.ya.b.a&=readspc(spc.pc); spc.pc++;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x29: /*AND (xx),(xx)*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(spc.pc)+spc.p.p; spc.pc++;
			temp=readspc(addr)&readspc(addr2);
			setspczn(temp);
			writespc(addr2,temp);
			spccycles(6);
			break;

		case 0x2B: /*ROL (xx)*/
			getdp();
			temp=readspc(addr);
			templ=spc.p.c;
			spc.p.c=temp&0x80;
			temp<<=1;
			if (templ) temp|=1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x2D: /*PUSH A*/
			writespc(spc.s+0x100,spc.ya.b.a); spc.s--;
			spccycles(4);
			break;

		case 0x2E: /*CBNE A,(xx)*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (spc.ya.b.a!=temp)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(5);
			break;

		case 0x2F: /*BRA*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			spc.pc+=addr;
			spccycles(4);
			break;

		case 0x30: /*BMI*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (spc.p.n)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(2);
			break;

		case 0x35: /*AND A,(xxxx+X)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			spc.ya.b.a&=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0x36: /*AND A,(xxxx+Y)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.ya.b.y; spc.pc+=2;
			spc.ya.b.a&=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0x38: /*AND (xx),#*/
			temp2=readspc(spc.pc); spc.pc++;
			getdp();
			temp2&=readspc(addr);
			setspczn(temp2);
			writespc(addr,temp2);
			spccycles(5);
			break;

		case 0x3A: /*INCW xx*/
			getdp();
			tempw=readspc(addr)+(readspc(addr+1)<<8)+1;
			writespc(addr,tempw&0xFF);
			writespc(addr+1,tempw>>8);
			spc.p.z=!tempw; spc.p.n=tempw&0x8000;
			spccycles(6);
			break;

		case 0x3C: /*ROL A*/
			templ=spc.p.c;
			spc.p.c=spc.ya.b.a&0x80;
			spc.ya.b.a<<=1;
			if (templ) spc.ya.b.a|=1;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x3D: /*INC X*/
			spc.x++;
			setspczn(spc.x);
			spccycles(2);
			break;

		case 0x3E: /*CMP X,(xx)*/
			getdp();
			temp=readspc(addr);
			setspczn(spc.x-temp);
			spc.p.c=(spc.x>=temp);
			spccycles(3);
			break;

		case 0x3F: /*CALL*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8); spc.pc+=2;
			//                        printf("CALL %04X->",spc.pc);
			writespc(spc.s+0x100,spc.pc>>8);   spc.s--;
			writespc(spc.s+0x100,spc.pc&0xFF); spc.s--;
			spc.pc=addr;
			//                        printf("%04X\n",spc.pc);
			spccycles(8);
			break;

		case 0x40: /*SETP*/
			//                        printf("SETP!!\n");
			spc.p.p=0x100;
			spccycles(2);
			break;

		case 0x44: /*EOR A,(xx)*/
			getdp();
			spc.ya.b.a^=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(3);
			break;

		case 0x45: /*EOR A,(xxxx)*/
			getabs();
			spc.ya.b.a^=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(4);
			break;

		case 0x46: /*EOR A,(X)*/
			addr=spc.x+spc.p.p;
			spc.ya.b.a^=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(3);
			break;

		case 0x47: /*EOR A,(xx+X)*/
			getdpx();
			addr2=readspc(addr)+(readspc(addr+1)<<8);
			spc.ya.b.a^=readspc(addr2);
			setspczn(spc.ya.b.a);
			spccycles(6);
			break;

		case 0x48: /*EOR A,#*/
			spc.ya.b.a^=readspc(spc.pc); spc.pc++;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x49: /*EOR (xx),(xx)*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(spc.pc)+spc.p.p; spc.pc++;
			temp=readspc(addr)^readspc(addr2);
			setspczn(temp);
			writespc(addr2,temp);
			spccycles(6);
			break;

		case 0x4B: /*LSR (xx)*/
			getdp();
			temp=readspc(addr);
			spc.p.c=temp&1;
			temp>>=1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x4C: /*LSR (xxxx)*/
			getabs();
			temp=readspc(addr);
			spc.p.c=temp&1;
			temp>>=1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0x4D: /*PUSH X*/
			writespc(spc.s+0x100,spc.x); spc.s--;
			spccycles(4);
			break;

		case 0x4E: /*TCLR1 xxxx*/
			getabs();
			temp=readspc(addr);
			setspczn(spc.ya.b.a&temp);
			temp&=~spc.ya.b.a;
			writespc(addr,temp);
			spccycles(6);
			break;

		case 0x4F: /*PCALL*/
			temp=readspc(spc.pc); spc.pc++;
			writespc(spc.s+0x100,spc.pc>>8);   spc.s--;
			writespc(spc.s+0x100,spc.pc&0xFF); spc.s--;
			spc.pc=0xFF00|temp;
			spccycles(6);
			break;

		case 0x50: /*BVC*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!spc.p.v)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(2);
			break;

		case 0x55: /*EOR A,(xxxx+X)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			spc.ya.b.a^=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0x56: /*EOR A,(xxxx+Y)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.ya.b.y; spc.pc+=2;
			spc.ya.b.a^=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0x58: /*EOR (xx),#*/
			temp2=readspc(spc.pc); spc.pc++;
			getdp();
			temp2^=readspc(addr);
			setspczn(temp2);
			writespc(addr,temp2);
			spccycles(5);
			break;

		case 0x5A: /*CMPW YA,xx*/
			getdp();
			tempw=readspc(addr)|(readspc(addr+1)<<8);
			setspczn16(spc.ya.w-tempw);
			spccycles(4);
			break;

		case 0x5B: /*LSR (xx+X)*/
			getdpx();
			temp=readspc(addr);
			spc.p.c=temp&1;
			temp>>=1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0x5C: /*LSR A*/
			spc.p.c=spc.ya.b.a&1;
			spc.ya.b.a>>=1;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x5D: /*MOV X,A*/
			spc.x=spc.ya.b.a;
			setspczn(spc.x);
			spccycles(2);
			break;

		case 0x5E: /*CMP Y,(xxxx)*/
			getabs();
			temp=readspc(addr);
			setspczn(spc.ya.b.y-temp);
			spc.p.c=(spc.ya.b.y>=temp);
			spccycles(4);
			break;

		case 0x5F: /*JMP xxxx*/
			getabs();
			spc.pc=addr;
			//                        printf("JMP %04X\n",spc.pc);
			spccycles(3);
			break;

		case 0x60: /*CLRC*/
			spc.p.c=0;
			spccycles(2);
			break;

		case 0x64: /*CMP A,(xx)*/
			getdp();
			temp=readspc(addr);
			setspczn(spc.ya.b.a-temp);
			spc.p.c=(spc.ya.b.a>=temp);
			spccycles(3);
			break;

		case 0x65: /*CMP A,(xxxx)*/
			getabs();
			temp=readspc(addr);
			setspczn(spc.ya.b.a-temp);
			spc.p.c=(spc.ya.b.a>=temp);
			spccycles(4);
			break;

		case 0x66: /*CMP A,(X)*/
			addr=spc.x+spc.p.p;
			temp=readspc(addr);
			setspczn(spc.ya.b.a-temp);
			spc.p.c=(spc.ya.b.a>=temp);
			spccycles(3);
			break;

		case 0x68: /*CMP A,#*/
			temp=readspc(spc.pc); spc.pc++;
			setspczn(spc.ya.b.a-temp);
			spc.p.c=(spc.ya.b.a>=temp);
			spccycles(2);
			break;

		case 0x69: /*CMP (xx),(xx)*/
			getdp(); addr2=addr;
			getdp();
			temp=readspc(addr2);
			temp2=readspc(addr);
			spc.p.c=(temp>=temp2);
			setspczn(temp-temp2);
			spccycles(6);
			break;

		case 0x6B: /*ROR (xx)*/
			getdp();
			temp=readspc(addr);
			templ=spc.p.c;
			spc.p.c=temp&1;
			temp>>=1;
			if (templ) temp|=0x80;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x6D: /*PUSH Y*/
			writespc(spc.s+0x100,spc.ya.b.y); spc.s--;
			spccycles(4);
			break;

		case 0x6E: /*DBNZ (xx)*/
			getdp();
			temp=readspc(addr)-1;
			writespc(addr,temp);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (temp)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(5);
			break;

		case 0x6F: /*RET*/
			spc.s++; spc.pc=readspc(spc.s+0x100);
			spc.s++; spc.pc|=(readspc(spc.s+0x100)<<8);
			//                        printf("RET %04X\n",spc.pc);
			spccycles(5);
			break;

		case 0x70: /*BVS*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (spc.p.v)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(2);
			break;

		case 0x74: /*CMP A,(xx+X)*/
			getdpx();
			temp=readspc(addr);
			setspczn(spc.ya.b.a-temp);
			spc.p.c=(spc.ya.b.a>=temp);
			spccycles(5);
			break;

		case 0x75: /*CMP A,(xxxx+X)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			temp=readspc(addr);
			setspczn(spc.ya.b.a-temp);
			spc.p.c=(spc.ya.b.a>=temp);
			spccycles(5);
			break;

		case 0x76: /*CMP A,(xxxx+Y)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.ya.b.y; spc.pc+=2;
			temp=readspc(addr);
			setspczn(spc.ya.b.a-temp);
			spc.p.c=(spc.ya.b.a>=temp);
			spccycles(5);
			break;

		case 0x77: /*CMP A,(xx)+Y*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(addr)+(readspc(addr+1)<<8)+spc.ya.b.y;
			temp=readspc(addr2);
			setspczn(spc.ya.b.a-temp);
			spc.p.c=(spc.ya.b.a>=temp);
			spccycles(6);
			break;

		case 0x78: /*CMP (xx),#*/
			temp=readspc(spc.pc);         spc.pc++;
			getdp();
			temp2=readspc(addr);
			setspczn(temp2-temp);
			spc.p.c=(temp2>=temp);
			spccycles(5);
			break;

		case 0x7A: /*ADDW YA,xx*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			//                        printf("ADDW %04X ",addr);
			tempw=readspc(addr)|(readspc(addr+1)<<8);
			//                        printf("%04X+%04X",tempw,spc.ya.w);
			templ=spc.ya.w+tempw;
			spc.p.v=(!((spc.ya.w^tempw)&0x8000)&&((spc.ya.w^templ)&0x8000));
			spc.ya.w=templ&0xFFFF;
			setspczn16(spc.ya.w);
			spc.p.c=templ&0x10000;
			//                        printf("=%04X\n",spc.ya.w);
			/*                        templ=spc.ya.w+tempw;
			spc.p.c=templ&0x10000;
			spc.p.v=(!((spc.ya.w^tempw)&0x8000)&&((spc.ya.w^templ)&0x8000));
			spc.ya.w=templ&0xFFFF;
			spc.p.z=!spc.ya.w;
			spc.p.n=spc.ya.w&0x8000;*/
			spccycles(5);
			break;

		case 0x7B: /*ROR (xx+X)*/
			getdpx();
			temp=readspc(addr);
			templ=spc.p.c;
			spc.p.c=temp&1;
			temp>>=1;
			if (templ) temp|=0x80;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x7C: /*ROR A*/
			templ=spc.p.c;
			spc.p.c=spc.ya.b.a&1;
			spc.ya.b.a>>=1;
			if (templ) spc.ya.b.a|=0x80;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x7D: /*MOV A,X*/
			spc.ya.b.a=spc.x;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x7E: /*CMP Y,(xx)*/
			getdp();
			temp=readspc(addr);
			setspczn(spc.ya.b.y-temp);
			spc.p.c=(spc.ya.b.y>=temp);
			spccycles(3);
			break;

		case 0x80: /*SETC*/
			spc.p.c=1;
			spccycles(2);
			break;

		case 0x84: /*ADC A,(xx)*/
			getdp();
			temp=readspc(addr);
			ADC(spc.ya.b.a,temp);
			spccycles(3);
			break;

		case 0x85: /*ADC A,(xxxx)*/
			getabs();
			temp=readspc(addr);
			ADC(spc.ya.b.a,temp);
			spccycles(4);
			break;

		case 0x88: /*ADC A,#*/
			temp=readspc(spc.pc); spc.pc++;
			ADC(spc.ya.b.a,temp);
			spccycles(2);
			break;

		case 0x89: /*ADC (xx),(xx)*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(spc.pc)+spc.p.p; spc.pc++;
			temp=readspc(addr);
			temp2=readspc(addr2);
			ADC(temp2,temp);
			writespc(addr2,temp2);
			spccycles(6);
			break;

		case 0x8A: /*EOR1 m.b*/
			addr=readspc(spc.pc)|(readspc(spc.pc+1)<<8); spc.pc+=2;
			tempw=addr>>13;
			addr&=0x1FFF;
			temp=readspc(addr);
			spc.p.c=(spc.p.c?1:0)^((temp&(1<<tempw))?1:0);
			spccycles(5);
			break;

		case 0x8B: /*DEC (xx)*/
			getdp();
			temp=readspc(addr)-1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0x8C: /*DEC (xxxx)*/
			getabs();
			temp=readspc(addr)-1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0x8D: /*MOV Y,#*/
			spc.ya.b.y=readspc(spc.pc); spc.pc++;
			setspczn(spc.ya.b.y);
			spccycles(2);
			break;

		case 0x8E: /*POP PSW*/
			spc.s++; temp=readspc(spc.s+0x100);
			spc.p.c=temp&0x01;
			spc.p.z=temp&0x02;
			spc.p.i=temp&0x04;
			spc.p.h=temp&0x08;
			spc.p.b=temp&0x10;
			spc.p.p=(temp&0x20)?0x100:0;
			spc.p.v=temp&0x40;
			spc.p.n=temp&0x80;
			spccycles(4);
			break;

		case 0x8F: /*MOV (xx),#*/
			temp=readspc(spc.pc);         spc.pc++;
			getdp();
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0x90: /*BCC*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!spc.p.c)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(2);
			break;

		case 0x94: /*ADC A,(xx,X)*/
			getdpx();
			temp=readspc(addr);
			ADC(spc.ya.b.a,temp);
			spccycles(4);
			break;

		case 0x95: /*ADC A,(xxxx+X)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			temp=readspc(addr);
			ADC(spc.ya.b.a,temp);
			spccycles(5);
			break;

		case 0x96: /*ADC A,(xxxx+Y)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.ya.b.y; spc.pc+=2;
			temp=readspc(addr);
			ADC(spc.ya.b.a,temp);
			spccycles(5);
			break;

		case 0x97: /*ADC A,(xx)+Y*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(addr)+(readspc(addr+1)<<8)+spc.ya.b.y;
			temp=readspc(addr2);
			ADC(spc.ya.b.a,temp);
			spccycles(6);
			break;

		case 0x98: /*ADC (xx),#*/
			temp2=readspc(spc.pc); spc.pc++;
			getdp();
			temp=readspc(addr);
			ADC(temp,temp2);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0x9A: /*SUBW YA,xx*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			tempw=readspc(addr)|(readspc(addr+1)<<8);
			templ=spc.ya.w-tempw;
			spc.p.v=(((spc.ya.w^tempw)&0x8000)&&((spc.ya.w^templ)&0x8000));
			spc.ya.w=templ&0xFFFF;
			spc.p.c=(spc.ya.w>=tempw);
			setspczn16(spc.ya.w);
			/*                        templ=spc.ya.w-tempw;
			spc.p.c=(spc.ya.w>=tempw);
			spc.p.v=(((spc.ya.w^tempw)&0x8000)&&((spc.ya.w^templ)&0x8000));
			spc.ya.w=templ&0xFFFF;
			spc.p.z=!spc.ya.w;
			spc.p.n=spc.ya.w&0x8000;*/
			spccycles(5);
			break;

		case 0x9B: /*DEC (xx+X)*/
			getdpx();
			temp=readspc(addr)-1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0x9C: /*DEC A*/
			spc.ya.b.a--;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0x9D: /*MOV X,SP*/
			spc.x=spc.s;
			setspczn(spc.x);
			spccycles(2);
			break;

		case 0x9E: /*DIV YA,X*/
			if (spc.x)
			{
				temp=spc.ya.w/spc.x;
				spc.ya.b.y=spc.ya.w%spc.x;
				spc.ya.b.a=temp;
				setspczn(temp);
				spc.p.v=0;
			}
			else
			{
				spc.ya.w=0xFFFF;
				spc.p.v=1;
			}
			spccycles(12);
			break;

		case 0x9F: /*XCN*/
			spc.ya.b.a=(spc.ya.b.a>>4)|(spc.ya.b.a<<4);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0xA4: /*SBC A,(xx)*/
			getdp();
			temp=readspc(addr);
			SBC(spc.ya.b.a,temp);
			spccycles(3);
			break;

		case 0xA5: /*SBC A,(xxxx)*/
			getabs();
			temp=readspc(addr);
			SBC(spc.ya.b.a,temp);
			spccycles(4);
			break;

		case 0xA8: /*SBC A,#*/
			temp=readspc(spc.pc); spc.pc++;
			SBC(spc.ya.b.a,temp);
			spccycles(2);
			break;

		case 0xA9: /*SBC (xx),(xx)*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(spc.pc)+spc.p.p; spc.pc++;
			temp=readspc(addr);
			temp2=readspc(addr2);
			SBC(temp2,temp);
			writespc(addr2,temp2);
			spccycles(6);
			break;

		case 0xAA: /*MOV1 C,m.b*/
			getabs();
			tempw=addr>>13;
			addr&=0x1FFF;
			temp=readspc(addr);
			spc.p.c=temp&(1<<tempw);
			spccycles(4);
			break;

		case 0xAB: /*INC (xx)*/
			getdp();
			temp=readspc(addr)+1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(4);
			break;

		case 0xAC: /*INC (xxxx)*/
			getabs();
			temp=readspc(addr)+1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0xAD: /*CMP Y,#*/
			temp=readspc(spc.pc); spc.pc++;
			setspczn(spc.ya.b.y-temp);
			spc.p.c=(spc.ya.b.y>=temp);
			spccycles(2);
			break;

		case 0xAE: /*POP A*/
			spc.s++; spc.ya.b.a=readspc(spc.s+0x100);
			//                        setspczn(spc.ya.b.a);
			spccycles(4);
			break;

		case 0xAF: /*MOV (X+),A*/
			writespc(spc.x+spc.p.p,spc.ya.b.a);
			spc.x++;
			spccycles(4);
			break;

		case 0xB0: /*BCS*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (spc.p.c)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(2);
			break;

		case 0xB4: /*SBC A,(xx+X)*/
			getdpx();
			temp=readspc(addr);
			SBC(spc.ya.b.a,temp);
			spccycles(4);
			break;

		case 0xB5: /*SBC A,(xxxx+X)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			temp=readspc(addr);
			SBC(spc.ya.b.a,temp);
			spccycles(5);
			break;

		case 0xB6: /*SBC A,(xxxx+Y)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.ya.b.y; spc.pc+=2;
			temp=readspc(addr);
			SBC(spc.ya.b.a,temp);
			spccycles(5);
			break;

		case 0xB7: /*SBC A,(xx)+Y*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(addr)+(readspc(addr+1)<<8)+spc.ya.b.y;
			temp=readspc(addr2);
			SBC(spc.ya.b.a,temp);
			spccycles(6);
			break;

		case 0xB8: /*SBC (xx),#*/
			temp2=readspc(spc.pc); spc.pc++;
			getdp();
			temp=readspc(addr);
			SBC(temp,temp2);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0xBA: /*MOVW YA,(xx)*/
			getdp();
			spc.ya.b.a=readspc(addr);
			spc.ya.b.y=readspc(addr+1);
			setspczn16(spc.ya.w);
			spccycles(5);
			break;

		case 0xBB: /*INC (xx+X)*/
			getdpx();
			temp=readspc(addr)+1;
			setspczn(temp);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0xBC: /*INC A*/
			spc.ya.b.a++;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0xBD: /*MOV SP,X*/
			spc.s=spc.x;
			spccycles(2);
			break;

		case 0xC0: /*DI*/
			spc.p.i=0;
			spccycles(3);
			break;

		case 0xC4: /*MOV (xx),A*/
			getdp();
			writespc(addr,spc.ya.b.a);
			spccycles(4);
			break;

		case 0xC5: /*MOV (xxxx),A*/
			getabs();
			writespc(addr,spc.ya.b.a);
			spccycles(5);
			break;

		case 0xC6: /*MOV (X),A*/
			writespc(spc.x+spc.p.p,spc.ya.b.a);
			spccycles(4);
			break;

		case 0xC7: /*MOV (xx+X),A*/
			getdpx();
			addr2=readspc(addr)+(readspc(addr+1)<<8);
			writespc(addr2,spc.ya.b.a);
			spccycles(7);
			break;

		case 0xC8: /*CMP X,#*/
			temp=readspc(spc.pc); spc.pc++;
			setspczn(spc.x-temp);
			spc.p.c=(spc.x>=temp);
			spccycles(2);
			break;

		case 0xC9: /*MOV (xxxx),X*/
			getabs();
			writespc(addr,spc.x);
			spccycles(5);
			break;

		case 0xCA: /*MOV1 m.b,C*/
			addr=readspc(spc.pc)|(readspc(spc.pc+1)<<8); spc.pc+=2;
			tempw=addr>>13;
			addr&=0x1FFF;
			temp=readspc(addr);
			if (spc.p.c) temp|= (1<<tempw);
			else         temp&=~(1<<tempw);
			writespc(addr,temp);
			spccycles(6);
			break;

		case 0xCB: /*MOV (xx),Y*/
			getdp();
			writespc(addr,spc.ya.b.y);
			spccycles(4);
			break;

		case 0xCC: /*MOV (xxxx),Y*/
			getabs();
			writespc(addr,spc.ya.b.y);
			spccycles(5);
			break;

		case 0xCD: /*MOV X,#*/
			spc.x=readspc(spc.pc); spc.pc++;
			setspczn(spc.x);
			spccycles(2);
			break;

		case 0xCE: /*POP X*/
			spc.s++; spc.x=readspc(spc.s+0x100);
			//                        setspczn(spc.x);
			spccycles(4);
			break;

		case 0xCF: /*MUL YA*/
			//                        printf("MUL %02X*%02X=",spc.ya.b.a,spc.ya.b.y);
			spc.ya.w=spc.ya.b.a*spc.ya.b.y;
			//                        printf("%04X\n",spc.ya.w);
			setspczn(spc.ya.b.y);
			spccycles(9);
			break;

		case 0xD0: /*BNE*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!spc.p.z)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(2);
			break;

		case 0xD4: /*MOV (xx+X),A*/
			getdpx();
			writespc(addr,spc.ya.b.a);
			spccycles(5);
			break;

		case 0xD5: /*MOV (xxxx+X),A*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			writespc(addr,spc.ya.b.a);
			spccycles(6);
			break;

		case 0xD6: /*MOV (xxxx+Y),A*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.ya.b.y; spc.pc+=2;
			writespc(addr,spc.ya.b.a);
			spccycles(6);
			break;

		case 0xD7: /*MOV (xx)+Y,A*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(addr)+(readspc(addr+1)<<8)+spc.ya.b.y;
			writespc(addr2,spc.ya.b.a);
			spccycles(7);
			break;

		case 0xD8: /*MOV (xx),X*/
			getdp();
			writespc(addr,spc.x);
			spccycles(4);
			break;

		case 0xD9: /*MOV (xx+Y),X*/
			addr=((readspc(spc.pc)+spc.ya.b.y)&0xFF)+spc.p.p; spc.pc++;
			writespc(addr,spc.x);
			spccycles(5);
			break;

		case 0xDA: /*MOVW (xx),YA*/
			getdp();
			writespc(addr,spc.ya.b.a);
			writespc(addr+1,spc.ya.b.y);
			spccycles(5);
			break;

		case 0xDB: /*MOV (xx+X),Y*/
			getdpx();
			writespc(addr,spc.ya.b.y);
			spccycles(5);
			break;

		case 0xDC: /*DEC Y*/
			spc.ya.b.y--;
			setspczn(spc.ya.b.y);
			spccycles(2);
			break;

		case 0xDD: /*MOV A,Y*/
			spc.ya.b.a=spc.ya.b.y;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0xDE: /*CBNE A,(xx+X)*/
			getdpx();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (spc.ya.b.a!=temp)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(6);
			break;

		case 0xE0: /*CLRV*/
			spc.p.v=spc.p.h=0;
			spccycles(2);
			break;

		case 0xE4: /*MOV A,(xx)*/
			getdp();
			spc.ya.b.a=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(3);
			break;

		case 0xE5: /*MOV A,(xxxx)*/
			getabs();
			spc.ya.b.a=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(4);
			break;

		case 0xE6: /*MOV A,(X)*/
			spc.ya.b.a=readspc(spc.x+spc.p.p);
			setspczn(spc.ya.b.a);
			spccycles(3);
			break;

		case 0xE7: /*MOV A,(xx+X)*/
			getdpx();
			addr2=readspc(addr)|(readspc(addr+1)<<8);
			spc.ya.b.a=readspc(addr2);
			setspczn(spc.ya.b.a);
			spccycles(6);
			break;

		case 0xE8: /*MOV A,#*/
			spc.ya.b.a=readspc(spc.pc); spc.pc++;
			setspczn(spc.ya.b.a);
			spccycles(2);
			break;

		case 0xE9: /*MOV X,(xxxx)*/
			getabs();
			spc.x=readspc(addr);
			setspczn(spc.x);
			spccycles(4);
			break;

		case 0xEA: /*NOT1 m.b*/
			addr=readspc(spc.pc)|(readspc(spc.pc+1)<<8); spc.pc+=2;
			tempw=addr>>13;
			addr&=0x1FFF;
			temp=readspc(addr);
			temp^=(1<<tempw);
			writespc(addr,temp);
			spccycles(5);
			break;

		case 0xEB: /*MOV Y,(xx)*/
			getdp();
			spc.ya.b.y=readspc(addr);
			setspczn(spc.ya.b.y);
			spccycles(3);
			break;

		case 0xEC: /*MOV Y,(xxxx)*/
			getabs();
			spc.ya.b.y=readspc(addr);
			setspczn(spc.ya.b.y);
			spccycles(4);
			break;

		case 0xED: /*NOTC*/
			spc.p.c=!spc.p.c;
			spccycles(3);
			break;

		case 0xEE: /*POP Y*/
			spc.s++; spc.ya.b.y=readspc(spc.s+0x100);
			//                        setspczn(spc.ya.b.y);
			spccycles(4);
			break;

		case 0xF0: /*BEQ*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (spc.p.z)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(2);
			break;

		case 0xF4: /*MOV A,(xx+X)*/
			getdpx();
			spc.ya.b.a=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(4);
			break;

		case 0xF5: /*MOV A,(xxxx+X)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.x; spc.pc+=2;
			//                        printf("MOVA %04X",addr);
			spc.ya.b.a=readspc(addr);
			//                        printf(" %02X\n",spc.ya.b.a);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0xF6: /*MOV A,(xxxx+Y)*/
			addr=readspc(spc.pc)+(readspc(spc.pc+1)<<8)+spc.ya.b.y; spc.pc+=2;
			spc.ya.b.a=readspc(addr);
			setspczn(spc.ya.b.a);
			spccycles(5);
			break;

		case 0xF7: /*MOV A,(xx)+Y*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(addr)+(readspc(addr+1)<<8)+spc.ya.b.y;
			spc.ya.b.a=readspc(addr2);
			setspczn(spc.ya.b.a);
			spccycles(6);
			break;

		case 0xF8: /*MOV X,(xx)*/
			getdp();
			spc.x=readspc(addr);
			setspczn(spc.x);
			spccycles(3);
			break;
		case 0xF9: /*MOV X,(xx+Y)*/
			getdpy();
			spc.x=readspc(addr);
			setspczn(spc.x);
			spccycles(3);
			break;

		case 0xFA: /*MOV (xx),(xx)*/
			addr=readspc(spc.pc)+spc.p.p; spc.pc++;
			addr2=readspc(spc.pc)+spc.p.p; spc.pc++;
			temp=readspc(addr);
			writespc(addr2,temp);
			spccycles(5);
			break;

		case 0xFB: /*MOV Y,(xx+X)*/
			getdpx();
			spc.ya.b.y=readspc(addr);
			setspczn(spc.ya.b.y);
			spccycles(4);
			break;

		case 0xFC: /*INC Y*/
			spc.ya.b.y++;
			setspczn(spc.ya.b.y);
			spccycles(2);
			break;

		case 0xFD: /*MOV Y,A*/
			spc.ya.b.y=spc.ya.b.a;
			setspczn(spc.ya.b.y);
			spccycles(2);
			break;

		case 0xFE: /*DBNZ Y*/
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			spc.ya.b.y--;
			if (spc.ya.b.y)
			{
				spc.pc+=addr;
				spccycles(2);
			}
			spccycles(4);
			break;

		case 0x13: /*BBC 0.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!(temp&0x01))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0x33: /*BBC 1.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!(temp&0x02))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0x53: /*BBC 2.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!(temp&0x04))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0x73: /*BBC 3.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!(temp&0x08))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0x93: /*BBC 4.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!(temp&0x10))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0xB3: /*BBC 5.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!(temp&0x20))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0xD3: /*BBC 6.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!(temp&0x40))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0xF3: /*BBC 7.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if (!(temp&0x80))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;

		case 0x03: /*BBS 0.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if ((temp&0x01))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0x23: /*BBS 1.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if ((temp&0x02))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0x43: /*BBS 2.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if ((temp&0x04))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0x63: /*BBS 3.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if ((temp&0x08))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0x83: /*BBS 4.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if ((temp&0x10))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0xA3: /*BBS 5.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if ((temp&0x20))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0xC3: /*BBS 6.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if ((temp&0x40))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;
		case 0xE3: /*BBS 7.xx*/
			getdp();
			temp=readspc(addr);
			addr=readspc(spc.pc); spc.pc++;
			if (addr&0x80) addr|=0xFF00;
			if ((temp&0x80))
			{
				spc.pc+=addr;
				spccycles(5);
			}
			spccycles(2);
			break;

		default:
			spc.pc--;
			//snemlog(L"Bad SPC opcode %02X at %04X\n",opcode,spc.pc);

		}
		if (global_pal) spccycles-=(spccount*20.7796875f);//20.36383f);
		else     spccycles-=(spccount*20.9395313f);
		//                spctotal2+=(spccount*20.78f);//20.36383f);
		//                spctotal3+=spccount;
		spctimer[0]-=spccount;
		if (spctimer[0]<=0)
		{
			spctimer[0]+=128;
			spctimer2[0]++;
			if (spctimer2[0]==spclimit[0])
			{
				spctimer2[0]=0;
				spcram[0xFD]++;
			}
			spctimer2[0]&=255;
		}
		spctimer[1]-=spccount;
		if (spctimer[1]<=0)
		{
			spctimer[1]+=128;
			spctimer2[1]++;
			if (spctimer2[1]==spclimit[1])
			{
				spctimer2[1]=0;
				spcram[0xFE]++;
			}
			spctimer2[1]&=255;
		}
		spctimer[2]-=spccount;
		if (spctimer[2]<=0)
		{
			spctimer[2]+=16;
			spctimer2[2]++;
			if (spctimer2[2]==spclimit[2])
			{
				spctimer2[2]=0;
				spcram[0xFF]++;
			}
			spctimer2[2]&=255;
		}
		dsptotal-=spccount;
		if (dsptotal<=0)
		{
			dsptotal+=32;
			//                        snemlog("PollDSP\n");
		//	polldsp();
		}
		//                spctotal+=spccount;
		//                if (spcoutput) printf("%04X : %04X %02X %02X %02X\n",spc.pc,spc.ya.w,spc.x,spc.s,opcode);
		//                if (spc.pc==0x12F7) printf("12F7 from %04X %04X\n",spc2,spc3);
	}
	//        snemlog("End of execSPC\n");
}
