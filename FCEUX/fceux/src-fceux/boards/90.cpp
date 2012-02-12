/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
 *  Copyright (C) 2005 CaH4e3
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "mapinc.h"
//#define DEBUG90

// Mapper 090 is simpliest mapper hardware and have not extended nametable control and latched chr banks in 4k mode
// Mapper 209 much compicated hardware with decribed above features disabled by default and switchable by command
// Mapper 211 the same mapper 209 but with forced nametable control

static int is209; 
static int is211;

static uint8 IRQMode;        // from $c001
static uint8 IRQPre;         // from $c004
static uint8 IRQPreSize;     // from $c007
static uint8 IRQCount;       // from $c005
static uint8 IRQXOR;         // Loaded from $C006
static uint8 IRQa;           // $c002, $c003, and $c000

static uint8 mul[2];
static uint8 regie;

static uint8 tkcom[4];
static uint8 prgb[4];
static uint8 chrlow[8];
static uint8 chrhigh[8];

static uint8 chr[2];

static uint16 names[4];
static uint8 tekker;

static SFORMAT Tek_StateRegs[]={
  {&IRQMode, 1, "IRQMODE"},
  {&IRQPre, 1, "IRQPRE"},
  {&IRQPreSize, 1, "IRQPRESIZE"},
  {&IRQCount, 1, "IRQC"},
  {&IRQXOR, 1, "IRQXOR"},
  {&IRQa, 1, "IRQa"},
  {mul, 2, "MUL"},
  {&regie, 1, "REGI"},
  {tkcom, 4, "TKCO"},
  {prgb, 4, "PRGB"},
  {chr, 2, "CHRLATCH"},
  {chrlow, 4, "CHRL"},
  {chrhigh, 8, "CHRH"},
  {&names[0], 2|FCEUSTATE_RLSB, "NMS0"},
  {&names[1], 2|FCEUSTATE_RLSB, "NMS1"},
  {&names[2], 2|FCEUSTATE_RLSB, "NMS2"},
  {&names[3], 2|FCEUSTATE_RLSB, "NMS3"},
  {&tekker, 1, "TEKR"},
  {0}
};

static void mira(void)
{
  if((tkcom[0]&0x20&&is209)||is211)
  {
    int x;
    if(tkcom[0]&0x40)        // Name tables are ROM-only
    {
      for(x=0;x<4;x++)
         setntamem(CHRptr[0]+(((names[x])&CHRmask1[0])<<10),0,x);
    }
    else                        // Name tables can be RAM or ROM.
    {
      for(x=0;x<4;x++)
      {
        if((tkcom[1]&0x80)==(names[x]&0x80))        // RAM selected.
          setntamem(NTARAM+((names[x]&0x1)<<10),1,x);
        else
          setntamem(CHRptr[0]+(((names[x])&CHRmask1[0])<<10),0,x);
      }
    }
  }
  else
  {
    switch(tkcom[1]&3)
    {
      case 0: setmirror(MI_V); break;
      case 1: setmirror(MI_H); break;
      case 2: setmirror(MI_0); break;
      case 3: setmirror(MI_1); break;
    }
  }
}

static void tekprom(void)
{
  uint32 bankmode=((tkcom[3]&6)<<5);
  switch(tkcom[0]&7)
  {
    case 00: if(tkcom[0]&0x80)
               setprg8(0x6000,(((prgb[3]<<2)+3)&0x3F)|bankmode);
             setprg32(0x8000,0x0F|((tkcom[3]&6)<<3));
             break;
    case 01: if(tkcom[0]&0x80)
               setprg8(0x6000,(((prgb[3]<<1)+1)&0x3F)|bankmode);
             setprg16(0x8000,(prgb[1]&0x1F)|((tkcom[3]&6)<<4));
             setprg16(0xC000,0x1F|((tkcom[3]&6)<<4));
             break;
    case 03: // bit reversion
    case 02: if(tkcom[0]&0x80)
               setprg8(0x6000,(prgb[3]&0x3F)|bankmode);
             setprg8(0x8000,(prgb[0]&0x3F)|bankmode);
             setprg8(0xa000,(prgb[1]&0x3F)|bankmode);
             setprg8(0xc000,(prgb[2]&0x3F)|bankmode);
             setprg8(0xe000,0x3F|bankmode);
             break;
    case 04: if(tkcom[0]&0x80)
               setprg8(0x6000,(((prgb[3]<<2)+3)&0x3F)|bankmode);
             setprg32(0x8000,(prgb[3]&0x0F)|((tkcom[3]&6)<<3));
             break;
    case 05: if(tkcom[0]&0x80)
               setprg8(0x6000,(((prgb[3]<<1)+1)&0x3F)|bankmode);
             setprg16(0x8000,(prgb[1]&0x1F)|((tkcom[3]&6)<<4));
             setprg16(0xC000,(prgb[3]&0x1F)|((tkcom[3]&6)<<4));
             break;
    case 07: // bit reversion
    case 06: if(tkcom[0]&0x80)
               setprg8(0x6000,(prgb[3]&0x3F)|bankmode);
             setprg8(0x8000,(prgb[0]&0x3F)|bankmode);
             setprg8(0xa000,(prgb[1]&0x3F)|bankmode);
             setprg8(0xc000,(prgb[2]&0x3F)|bankmode);
             setprg8(0xe000,(prgb[3]&0x3F)|bankmode);
             break;
  }
}

static void tekvrom(void)
{
  int x, bank=0, mask=0xFFFF;
  if(!(tkcom[3]&0x20))
  {
    bank=(tkcom[3]&1)|((tkcom[3]&0x18)>>2);
    switch (tkcom[0]&0x18)
    {
      case 0x00: bank<<=5; mask=0x1F; break;
      case 0x08: bank<<=6; mask=0x3F; break;
      case 0x10: bank<<=7; mask=0x7F; break;
      case 0x18: bank<<=8; mask=0xFF; break;
    }
  }
  switch(tkcom[0]&0x18)
  {
    case 0x00:      // 8KB
         setchr8(((chrlow[0]|(chrhigh[0]<<8))&mask)|bank);
         break;
    case 0x08:      // 4KB
//         for(x=0;x<8;x+=4)
//            setchr4(x<<10,((chrlow[x]|(chrhigh[x]<<8))&mask)|bank);
         setchr4(0x0000,((chrlow[chr[0]]|(chrhigh[chr[0]]<<8))&mask)|bank);
         setchr4(0x1000,((chrlow[chr[1]]|(chrhigh[chr[1]]<<8))&mask)|bank);
         break;
    case 0x10:      // 2KB
         for(x=0;x<8;x+=2)
            setchr2(x<<10,((chrlow[x]|(chrhigh[x]<<8))&mask)|bank);
         break;
    case 0x18:      // 1KB
         for(x=0;x<8;x++)
            setchr1(x<<10,((chrlow[x]|(chrhigh[x]<<8))&mask)|bank);
         break;
  }
}

static DECLFW(M90TekWrite)
{
  switch(A&0x5C03)
  {
    case 0x5800: mul[0]=V; break;
    case 0x5801: mul[1]=V; break;
    case 0x5803: regie=V; break;
  }
}

static DECLFR(M90TekRead)
{
  switch(A&0x5C03)
  {
    case 0x5800: return (mul[0]*mul[1]);
    case 0x5801: return((mul[0]*mul[1])>>8);
    case 0x5803: return (regie);
    default: return tekker;
  }
  return(0xff);
}

static DECLFW(M90PRGWrite)
{
//  FCEU_printf("bs %04x %02x\n",A,V);
  prgb[A&3]=V;
  tekprom();
}

static DECLFW(M90CHRlowWrite)
{
//  FCEU_printf("bs %04x %02x\n",A,V);
  chrlow[A&7]=V;
  tekvrom();
}

static DECLFW(M90CHRhiWrite)
{
//  FCEU_printf("bs %04x %02x\n",A,V);
  chrhigh[A&7]=V;
  tekvrom();
}

static DECLFW(M90NTWrite)
{
//  FCEU_printf("bs %04x %02x\n",A,V);
  if(A&4)
  {
    names[A&3]&=0x00FF;
    names[A&3]|=V<<8;
  }
  else
  {
    names[A&3]&=0xFF00;
    names[A&3]|=V;
  }
  mira();
}

static DECLFW(M90IRQWrite)
{
//  FCEU_printf("bs %04x %02x\n",A,V);
  switch(A&7)
  {
    case 00: //FCEU_printf("%s IRQ (C000)\n",V&1?"Enable":"Disable");
             IRQa=V&1;if(!(V&1)) X6502_IRQEnd(FCEU_IQEXT);break;
    case 02: //FCEU_printf("Disable IRQ (C002) scanline=%d\n", scanline);
             IRQa=0;X6502_IRQEnd(FCEU_IQEXT);break;
    case 03: //FCEU_printf("Enable IRQ (C003) scanline=%d\n", scanline);
             IRQa=1;break;
    case 01: IRQMode=V;
             //  FCEU_printf("IRQ Count method: ");
             //  switch (IRQMode&3)
             //  {
             //    case 00: FCEU_printf("M2 cycles\n");break;
             //    case 01: FCEU_printf("PPU A12 toggles\n");break;
             //    case 02: FCEU_printf("PPU reads\n");break;
             //    case 03: FCEU_printf("Writes to CPU space\n");break;
             //  }
             //  FCEU_printf("Counter prescaler size: %s\n",(IRQMode&4)?"3 bits":"8 bits");
             //  FCEU_printf("Counter prescaler size adjust: %s\n",(IRQMode&8)?"Used C007":"Normal Operation");
             //  if((IRQMode>>6)==2) FCEU_printf("Counter Down\n");
             //   else if((IRQMode>>6)==1) FCEU_printf("Counter Up\n");
             //   else FCEU_printf("Counter Stopped\n");
              break;
    case 04: //FCEU_printf("Pre Counter Loaded and Xored wiht C006: %d\n",V^IRQXOR);
             IRQPre=V^IRQXOR;break;
    case 05: //FCEU_printf("Main Counter Loaded and Xored wiht C006: %d\n",V^IRQXOR);
             IRQCount=V^IRQXOR;break;
    case 06: //FCEU_printf("Xor Value: %d\n",V);
             IRQXOR=V;break;
    case 07: //if(!(IRQMode&8)) FCEU_printf("C001 is clear, no effect applied\n");
             //     else if(V==0xFF) FCEU_printf("Prescaler is changed for 12bits\n");
             //     else FCEU_printf("Counter Stopped\n");
             IRQPreSize=V;break;
  }
}

static DECLFW(M90ModeWrite)
{
//    FCEU_printf("bs %04x %02x\n",A,V);
    tkcom[A&3]=V;
    tekprom();
    tekvrom();
    mira();
    
#ifdef DEBUG90
  switch (A&3)
  {
   case 00: FCEU_printf("Main Control Register:\n");
            FCEU_printf("  PGR Banking mode: %d\n",V&7);
            FCEU_printf("  CHR Banking mode: %d\n",(V>>3)&3);
            FCEU_printf("  6000-7FFF addresses mapping: %s\n",(V&0x80)?"Yes":"No");
            FCEU_printf("  Nametable control: %s\n",(V&0x20)?"Enabled":"Disabled");
            if(V&0x20)
               FCEU_printf("  Nametable can be: %s\n",(V&0x40)?"ROM Only":"RAM or ROM");
            break;
   case 01: FCEU_printf("Mirroring mode: ");
            switch (V&3)
            {
             case 0: FCEU_printf("Vertical\n");break;
             case 1: FCEU_printf("Horizontal\n");break;
             case 2: FCEU_printf("Nametable 0 only\n");break;
             case 3: FCEU_printf("Nametable 1 only\n");break;
            }
            FCEU_printf("Mirroring flag: %s\n",(V&0x80)?"On":"Off");
            break;
   case 02: if((((tkcom[0])>>5)&3)==1)
              FCEU_printf("Nametable ROM/RAM select mode: %d\n",V>>7);
            break;
   case 03:
            FCEU_printf("CHR Banking mode: %s\n",(V&0x20)?"Entire CHR ROM":"256Kb Switching mode");
            if(!(V&0x20)) FCEU_printf("256K CHR bank number: %02x\n",(V&1)|((V&0x18)>>2));
            FCEU_printf("512K PRG bank number: %d\n",(V&6)>>1);
            FCEU_printf("CHR Bank mirroring: %s\n",(V&0x80)?"Swapped":"Normal operate");
  }
#endif
}

static DECLFW(M90DummyWrite)
{
//    FCEU_printf("bs %04x %02x\n",A,V);
}

static void CCL(void)
{
  if((IRQMode>>6) == 1) // Count Up
  {
    IRQCount++;
    if((IRQCount == 0) && IRQa)
    {
      X6502_IRQBegin(FCEU_IQEXT);
    }
  }
  else if((IRQMode>>6) == 2) // Count down
  {
    IRQCount--;
    if((IRQCount == 0xFF) && IRQa)
    {
      X6502_IRQBegin(FCEU_IQEXT);
    }
  }
}

static void ClockCounter(void)
{
  uint8 premask;

  if(IRQMode & 0x4)
    premask = 0x7;
  else
    premask = 0xFF;
  if((IRQMode>>6) == 1) // Count up
  {
    IRQPre++;
    if((IRQPre & premask) == 0) CCL();
  }
  else if((IRQMode>>6) == 2) // Count down
  {
    IRQPre--;
    if((IRQPre & premask) == premask) CCL();
  }
}

void CPUWrap(int a)
{
  int x;
  if((IRQMode&3)==0) for(x=0;x<a;x++) ClockCounter();
}

static void SLWrap(void)
{
  int x;
  if((IRQMode&3)==1) for(x=0;x<8;x++) ClockCounter();
}

static uint32 lastread;
static void M90PPU(uint32 A)
{
  if((IRQMode&3)==2)
  {
    if(lastread!=A)
    {
      ClockCounter();
      ClockCounter();
    }
    lastread=A;
  }
  
  if(is209)
  {
    uint8 l,h;
    h=A>>8;
    if(h<0x20&&((h&0x0F)==0xF))
    {
      l=A&0xF0;
      if(l==0xD0)
      {
        chr[(h&0x10)>>4]=((h&0x10)>>2);
        tekvrom();
      }
      else if(l==0xE0)
      {
        chr[(h&0x10)>>4]=((h&0x10)>>2)|2;
        tekvrom();
      }
    }
  }
  else
  {
    chr[0]=0;
    chr[1]=4;
  }
}

static void togglie()
{
  tekker+=0x40;
  tekker&=0xC0;
  FCEU_printf("tekker=%02x\n",tekker);
  memset(tkcom,0x00,sizeof(tkcom));
  memset(prgb,0xff,sizeof(prgb));
  tekprom();
  tekvrom();
}

static void M90Restore(int version)
{
  tekprom();
  tekvrom();
  mira();
}

static void M90Power(void)
{
  SetWriteHandler(0x5000,0x5fff,M90TekWrite);
  SetWriteHandler(0x8000,0x8ff0,M90PRGWrite);
  SetWriteHandler(0x9000,0x9fff,M90CHRlowWrite);
  SetWriteHandler(0xA000,0xAfff,M90CHRhiWrite);
  SetWriteHandler(0xB000,0xBfff,M90NTWrite);
  SetWriteHandler(0xC000,0xCfff,M90IRQWrite);
  SetWriteHandler(0xD000,0xD5ff,M90ModeWrite);
  SetWriteHandler(0xE000,0xFfff,M90DummyWrite);


  SetReadHandler(0x5000,0x5fff,M90TekRead);
  SetReadHandler(0x6000,0xffff,CartBR);

  mul[0]=mul[1]=regie=0xFF;

  memset(tkcom,0x00,sizeof(tkcom));
  memset(prgb,0xff,sizeof(prgb));
  memset(chrlow,0xff,sizeof(chrlow));
  memset(chrhigh,0xff,sizeof(chrhigh));
  memset(names,0x00,sizeof(names));

  if(is211)
    tekker=0xC0;
  else
    tekker=0x00;

  tekprom();
  tekvrom();
}


void Mapper90_Init(CartInfo *info)
{
  is211=0;
  is209=0;
  info->Reset=togglie;
  info->Power=M90Power;
  PPU_hook=M90PPU;
  MapIRQHook=CPUWrap;
  GameHBIRQHook2=SLWrap;
  GameStateRestore=M90Restore;
  AddExState(Tek_StateRegs, ~0, 0, 0);
}

void Mapper209_Init(CartInfo *info)
{
  is211=0;
  is209=1;
  info->Reset=togglie;
  info->Power=M90Power;
  PPU_hook=M90PPU;
  MapIRQHook=CPUWrap;
  GameHBIRQHook2=SLWrap;
  GameStateRestore=M90Restore;
  AddExState(Tek_StateRegs, ~0, 0, 0);
}

void Mapper211_Init(CartInfo *info)
{
  is211=1;
  info->Reset=togglie;
  info->Power=M90Power;
  PPU_hook=M90PPU;
  MapIRQHook=CPUWrap;
  GameHBIRQHook2=SLWrap;
  GameStateRestore=M90Restore;
  AddExState(Tek_StateRegs, ~0, 0, 0);
}
