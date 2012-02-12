/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 1998 BERO
 *  Copyright (C) 2002 Xodnizel
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

static void BandaiIRQHook(int a)
{
  if(IRQa)
  {
   IRQCount-=a;
   if(IRQCount<0)
   {
    X6502_IRQBegin(FCEU_IQEXT);
    //printf("IRQ: %d, %d\n",scanline,timestamp);
    IRQa=0;
    IRQCount=0xFFFF;
   }
  }
}

static DECLFW(Mapper16_write)
{
        A&=0xF;

        if(A<=0x7)
         VROM_BANK1(A<<10,V);
        else if(A==0x8)
         ROM_BANK16(0x8000,V);
        else switch(A) {
         case 0x9: switch(V&3) {
                    case 0x00:MIRROR_SET2(1);break;
                    case 0x01:MIRROR_SET2(0);break;
                    case 0x02:onemir(0);break;
                    case 0x03:onemir(1);break;
                   }
                   break;
         case 0xA:X6502_IRQEnd(FCEU_IQEXT);
                  IRQa=V&1;
                  IRQCount=IRQLatch;
                  break;
         case 0xB:IRQLatch&=0xFF00; IRQLatch|=V;
                  break;
         case 0xC:IRQLatch&=0xFF; IRQLatch|=V<<8;
                  break;
         case 0xD: break;/* Serial EEPROM control port */
 }
}

// Famicom jump 2:
// 0-7: Lower bit of data selects which 256KB PRG block is in use.
// This seems to be a hack on the developers' part, so I'll make emulation
// of it a hack(I think the current PRG block would depend on whatever the
// lowest bit of the CHR bank switching register that corresponds to the
// last CHR address read).
/*
static void PRGO(void)
{
 uint32 base=(mapbyte1[0]&1)<<4;
 ROM_BANK16(0x8000,(mapbyte2[0]&0xF)|base);
 ROM_BANK16(0xC000,base|0xF);
}

static DECLFW(Mapper153_write)
{
        A&=0xF;
        if(A<=0x7)
        {
         mapbyte1[A&7]=V;
         PRGO();
        }
        else if(A==0x8)
        {
         mapbyte2[0]=V;
         PRGO();
        }
        else switch(A) {
         case 0x9: switch(V&3) {
                           case 0x00:MIRROR_SET2(1);break;
                    case 0x01:MIRROR_SET2(0);break;
                    case 0x02:onemir(0);break;
                    case 0x03:onemir(1);break;
                   }
                   break;
         case 0xA:X6502_IRQEnd(FCEU_IQEXT);
                    IRQa=V&1;
                  IRQCount=IRQLatch;
                  break;
         case 0xB:IRQLatch&=0xFF00;
                  IRQLatch|=V;
                   break;
         case 0xC:IRQLatch&=0xFF;
                   IRQLatch|=V<<8;
                  break;
        }
}*/

//void Mapper16_init(void)
//{
// MapIRQHook=BandaiIRQHook;
//SetWriteHandler(0x6000,0xFFFF,Mapper16_write);
//}

//void Mapper153_init(void)
//{
// MapIRQHook=BandaiIRQHook;
// SetWriteHandler(0x8000,0xFFFF,Mapper153_write);
 /* This mapper/board seems to have WRAM at $6000-$7FFF, so I'll let the
    main ines code take care of that memory region. */
//}


static uint8 BarcodeData[256];
static int BarcodeReadPos;
static int BarcodeCycleCount;
static uint32 BarcodeOut;

int FCEUI_DatachSet(const uint8 *rcode)
{
        int    prefix_parity_type[10][6] = {
                {0,0,0,0,0,0}, {0,0,1,0,1,1}, {0,0,1,1,0,1}, {0,0,1,1,1,0},
                {0,1,0,0,1,1}, {0,1,1,0,0,1}, {0,1,1,1,0,0}, {0,1,0,1,0,1},
                {0,1,0,1,1,0}, {0,1,1,0,1,0}
        };
        int    data_left_odd[10][7] = {
                {0,0,0,1,1,0,1}, {0,0,1,1,0,0,1}, {0,0,1,0,0,1,1}, {0,1,1,1,1,0,1},
                {0,1,0,0,0,1,1}, {0,1,1,0,0,0,1}, {0,1,0,1,1,1,1}, {0,1,1,1,0,1,1},
                {0,1,1,0,1,1,1}, {0,0,0,1,0,1,1}
        };
        int    data_left_even[10][7] = {
                {0,1,0,0,1,1,1}, {0,1,1,0,0,1,1}, {0,0,1,1,0,1,1}, {0,1,0,0,0,0,1},
                {0,0,1,1,1,0,1}, {0,1,1,1,0,0,1}, {0,0,0,0,1,0,1}, {0,0,1,0,0,0,1},
                {0,0,0,1,0,0,1}, {0,0,1,0,1,1,1}
        };
        int    data_right[10][7] = {
                {1,1,1,0,0,1,0}, {1,1,0,0,1,1,0}, {1,1,0,1,1,0,0}, {1,0,0,0,0,1,0},
                {1,0,1,1,1,0,0}, {1,0,0,1,1,1,0}, {1,0,1,0,0,0,0}, {1,0,0,0,1,0,0},
                {1,0,0,1,0,0,0}, {1,1,1,0,1,0,0}
        };
        uint8 code[13+1];
        uint32 tmp_p=0;
        int i, j;
        int len;

        for(i=len=0;i<13;i++)
        {
         if(!rcode[i]) break;

         if((code[i]=rcode[i]-'0') > 9)
          return(0);
         len++;
        }
        if(len!=13 && len!=12 && len!=8 && len!=7) return(0);

        #define BS(x) BarcodeData[tmp_p]=x;tmp_p++

        for(j=0;j<32;j++)
        {
         BS(0x00);
        }

        /* Left guard bars */
        BS(1);        BS(0); BS(1);

        if(len==13 || len==12)
        {
         uint32 csum;

          for(i=0;i<6;i++)
          if(prefix_parity_type[code[0]][i])
          {
           for(j=0;j<7;j++)
           {
            BS(data_left_even[code[i+1]][j]);
           }
          }
          else
           for(j=0;j<7;j++)
           {
            BS(data_left_odd[code[i+1]][j]);
           }

         /* Center guard bars */
         BS(0); BS(1); BS(0); BS(1); BS(0);

         for(i=7;i<12;i++)
          for(j=0;j<7;j++)
          {
           BS(data_right[code[i]][j]);
          }
         csum=0;
         for(i=0;i<12;i++) csum+=code[i]*((i&1)?3:1);
         csum=(10-(csum%10))%10;
         //printf("%d\n",csum);
         for(j=0;j<7;j++)
         {
          BS(data_right[csum][j]);
         }

        }
        else if(len==8 || len==7)
        {
         uint32 csum=0;

         for(i=0;i<7;i++) csum+=(i&1)?code[i]:(code[i]*3);

         csum=(10-(csum%10))%10;

         for(i=0;i<4;i++)
          for(j=0;j<7;j++)
          {
           BS(data_left_odd[code[i]][j]);
          }


         /* Center guard bars */
         BS(0); BS(1); BS(0); BS(1); BS(0);

         for(i=4;i<7;i++)
          for(j=0;j<7;j++)
          {
           BS(data_right[code[i]][j]);
          }

         for(j=0;j<7;j++)
          { BS(data_right[csum][j]);}

        }

        /* Right guard bars */
        BS(1); BS(0); BS(1);

        for(j=0;j<32;j++)
        {
         BS(0x00);
        }

        BS(0xFF);
        #undef BS
        BarcodeReadPos=0;
        BarcodeOut=0x8;
        BarcodeCycleCount=0;
        return(1);
}

static void BarcodeIRQHook(int a)
{
 BandaiIRQHook(a);

 BarcodeCycleCount+=a;

 if(BarcodeCycleCount >= 1000)
 {
  BarcodeCycleCount -= 1000;
  if(BarcodeData[BarcodeReadPos]==0xFF)
  {
   BarcodeOut=0;
  }
  else
  {
   BarcodeOut=(BarcodeData[BarcodeReadPos]^1)<<3;
   BarcodeReadPos++;
  }
 }
}

static DECLFR(Mapper157_read)
{
 uint8 ret;

 ret=BarcodeOut;
 return(ret);
}

void Mapper157_init(void)
{
 GameInfo->cspecial = SIS_DATACH;
 MapIRQHook=BarcodeIRQHook;
 SetWriteHandler(0x6000,0xFFFF,Mapper16_write);
 SetReadHandler(0x6000,0x7FFF,Mapper157_read);

 BarcodeData[0]=0xFF;
 BarcodeReadPos=0;
 BarcodeOut=0;
 BarcodeCycleCount=0;
}
