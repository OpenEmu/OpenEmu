/* Cygne
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Dox dox@space.pl
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

#include "wswan.h"
#include "gfx.h"
#include "memory.h"

namespace MDFN_IEN_WSWAN
{


uint8	tiles[256][256][2][8];
uint8	wsTCache[512*64];			
uint8	wsTCache2[512*64];			
uint8	wsTCacheFlipped[512*64];
uint8	wsTCacheFlipped2[512*64];
uint8	wsTCacheUpdate[512];		
uint8	wsTCacheUpdate2[512];		  
uint8	wsTileRow[8];
int	wsVMode;				

void WSWan_TCacheInvalidByAddr(uint32 ws_offset)
{
  if(wsVMode  && (ws_offset>=0x4000)&&(ws_offset<0x8000))
  {
   wsTCacheUpdate[(ws_offset-0x4000)>>5]=FALSE; /*invalidate tile*/
   return;
  }
  else if((ws_offset>=0x2000)&&(ws_offset<0x4000))
  {
   wsTCacheUpdate[(ws_offset-0x2000)>>4]=FALSE; /*invalidate tile*/
   return;
  }

  if(wsVMode  && (ws_offset>=0x8000)&&(ws_offset<0xc000))
  {
   wsTCacheUpdate2[(ws_offset-0x8000)>>5]=FALSE; /*invalidate tile*/
   return;
  }
  else if((ws_offset>=0x4000)&&(ws_offset<0x6000))
  {
   wsTCacheUpdate2[(ws_offset-0x4000)>>4]=FALSE; /*invalidate tile*/
   return;
  }
}

void wsSetVideo(int number,bool force)
{
 if((number!=wsVMode)||(force))
 { 
  wsVMode=number;
  memset(wsTCacheUpdate,0,512);
  memset(wsTCacheUpdate2,0,512);
 }
}

void wsMakeTiles(void)
{
 int	x,y,b0,b1,b2,b3,b4,b5,b6,b7;
 for(x=0;x<256;x++)
	 for(y=0;y<256;y++)
	 {
		b0=(x&128)>>7;b1=(x&64)>>6;b2=(x&32)>>5;b3=(x&16)>>4;b4=(x&8)>>3;b5=(x&4)>>2;b6=(x&2)>>1;b7=(x&1);
		b0|=(y&128)>>6;b1|=(y&64)>>5;b2|=(y&32)>>4;b3|=(y&16)>>3;b4|=(y&8)>>2;b5|=(y&4)>>1;b6|=(y&2);b7|=(y&1)<<1;
		tiles[x][y][0][0]=b0;
		tiles[x][y][0][1]=b1;
		tiles[x][y][0][2]=b2;
		tiles[x][y][0][3]=b3;
		tiles[x][y][0][4]=b4;
		tiles[x][y][0][5]=b5;
		tiles[x][y][0][6]=b6;
		tiles[x][y][0][7]=b7;
		tiles[x][y][1][0]=b7;
		tiles[x][y][1][1]=b6;
		tiles[x][y][1][2]=b5;
		tiles[x][y][1][3]=b4;
		tiles[x][y][1][4]=b3;
		tiles[x][y][1][5]=b2;
		tiles[x][y][1][6]=b1;
		tiles[x][y][1][7]=b0;
	 }
}

void wsGetTile(uint32 number,uint32 line,int flipv,int fliph,int bank)
{
 uint32		t_adr,t_index,i;
 uint8		byte0,byte1,byte2,byte3;

 if((!bank)||(!(wsVMode &0x07)))
 {

#ifdef TCACHE_OFF
 wsTCacheUpdate[number]=false;
#endif
	 
  if(!wsTCacheUpdate[number])
  {
   wsTCacheUpdate[number]=true;
   switch(wsVMode)
   {
	case 7:
	 t_adr=0x4000+(number<<5);
	 t_index=number<<6;
	 for(i=0;i<8;i++)
	 {
	  byte0=wsRAM[t_adr++];
	  byte1=wsRAM[t_adr++];
	  byte2=wsRAM[t_adr++];
	  byte3=wsRAM[t_adr++];
	  wsTCache[t_index]=byte0>>4;
	  wsTCacheFlipped[t_index++]=byte3&15;
	  wsTCache[t_index]=byte0&15;
	  wsTCacheFlipped[t_index++]=byte3>>4;
	  wsTCache[t_index]=byte1>>4;
	  wsTCacheFlipped[t_index++]=byte2&15;
      wsTCache[t_index]=byte1&15;
	  wsTCacheFlipped[t_index++]=byte2>>4;
	  wsTCache[t_index]=byte2>>4;
	  wsTCacheFlipped[t_index++]=byte1&15;
	  wsTCache[t_index]=byte2&15;
	  wsTCacheFlipped[t_index++]=byte1>>4;
	  wsTCache[t_index]=byte3>>4;
	  wsTCacheFlipped[t_index++]=byte0&15;
	  wsTCache[t_index]=byte3&15;
	  wsTCacheFlipped[t_index++]=byte0>>4;
	 }
	break;

	case 6:
     t_adr=0x4000+(number<<5);
	 t_index=number<<6;
	 for(i=0;i<8;i++)
	 {
	  byte0=wsRAM[t_adr++];
	  byte1=wsRAM[t_adr++];
	  byte2=wsRAM[t_adr++];
	  byte3=wsRAM[t_adr++];
	  wsTCache[t_index]=((byte0>>7)&1)|(((byte1>>7)&1)<<1)|(((byte2>>7)&1)<<2)|(((byte3>>7)&1)<<3);
	  wsTCacheFlipped[t_index++]=((byte0)&1)|(((byte1)&1)<<1)|(((byte2)&1)<<2)|(((byte3)&1)<<3);
	  wsTCache[t_index]=((byte0>>6)&1)|(((byte1>>6)&1)<<1)|(((byte2>>6)&1)<<2)|(((byte3>>6)&1)<<3);
	  wsTCacheFlipped[t_index++]=((byte0>>1)&1)|(((byte1>>1)&1)<<1)|(((byte2>>1)&1)<<2)|(((byte3>>1)&1)<<3);
	  wsTCache[t_index]=((byte0>>5)&1)|(((byte1>>5)&1)<<1)|(((byte2>>5)&1)<<2)|(((byte3>>5)&1)<<3);
	  wsTCacheFlipped[t_index++]=((byte0>>2)&1)|(((byte1>>2)&1)<<1)|(((byte2>>2)&1)<<2)|(((byte3>>2)&1)<<3);
	  wsTCache[t_index]=((byte0>>4)&1)|(((byte1>>4)&1)<<1)|(((byte2>>4)&1)<<2)|(((byte3>>4)&1)<<3);
	  wsTCacheFlipped[t_index++]=((byte0>>3)&1)|(((byte1>>3)&1)<<1)|(((byte2>>3)&1)<<2)|(((byte3>>3)&1)<<3);
	  wsTCache[t_index]=((byte0>>3)&1)|(((byte1>>3)&1)<<1)|(((byte2>>3)&1)<<2)|(((byte3>>3)&1)<<3);
	  wsTCacheFlipped[t_index++]=((byte0>>4)&1)|(((byte1>>4)&1)<<1)|(((byte2>>4)&1)<<2)|(((byte3>>4)&1)<<3);
	  wsTCache[t_index]=((byte0>>2)&1)|(((byte1>>2)&1)<<1)|(((byte2>>2)&1)<<2)|(((byte3>>2)&1)<<3);
	  wsTCacheFlipped[t_index++]=((byte0>>5)&1)|(((byte1>>5)&1)<<1)|(((byte2>>5)&1)<<2)|(((byte3>>5)&1)<<3);
	  wsTCache[t_index]=((byte0>>1)&1)|(((byte1>>1)&1)<<1)|(((byte2>>1)&1)<<2)|(((byte3>>1)&1)<<3);
	  wsTCacheFlipped[t_index++]=((byte0>>6)&1)|(((byte1>>6)&1)<<1)|(((byte2>>6)&1)<<2)|(((byte3>>6)&1)<<3);
	  wsTCache[t_index]=((byte0)&1)|(((byte1)&1)<<1)|(((byte2)&1)<<2)|(((byte3)&1)<<3);
	  wsTCacheFlipped[t_index++]=((byte0>>7)&1)|(((byte1>>7)&1)<<1)|(((byte2>>7)&1)<<2)|(((byte3>>7)&1)<<3);
	 }
    break;

	default:
	 t_adr=0x2000+(number<<4);
	 t_index=number<<6;
	 for(i=0;i<8;i++)
	 {
	  byte0=wsRAM[t_adr++];
	  byte1=wsRAM[t_adr++];
	  wsTCache[t_index]=tiles[byte0][byte1][0][0];
	  wsTCacheFlipped[t_index++]=tiles[byte0][byte1][1][0];
          wsTCache[t_index]=tiles[byte0][byte1][0][1];
	  wsTCacheFlipped[t_index++]=tiles[byte0][byte1][1][1];
          wsTCache[t_index]=tiles[byte0][byte1][0][2];
 	  wsTCacheFlipped[t_index++]=tiles[byte0][byte1][1][2];
          wsTCache[t_index]=tiles[byte0][byte1][0][3];
	  wsTCacheFlipped[t_index++]=tiles[byte0][byte1][1][3];
          wsTCache[t_index]=tiles[byte0][byte1][0][4];
	  wsTCacheFlipped[t_index++]=tiles[byte0][byte1][1][4];
          wsTCache[t_index]=tiles[byte0][byte1][0][5];
	  wsTCacheFlipped[t_index++]=tiles[byte0][byte1][1][5];
          wsTCache[t_index]=tiles[byte0][byte1][0][6];
	  wsTCacheFlipped[t_index++]=tiles[byte0][byte1][1][6];
          wsTCache[t_index]=tiles[byte0][byte1][0][7];
	  wsTCacheFlipped[t_index++]=tiles[byte0][byte1][1][7];
	 }
	}
  }
  if(flipv)
   line=7-line;
  if(fliph)
   memcpy(&wsTileRow[0],&wsTCacheFlipped[(number<<6)|(line<<3)],8);
  else
   memcpy(&wsTileRow[0],&wsTCache[(number<<6)|(line<<3)],8);
 }
 else
 {

#ifdef TCACHE_OFF
 wsTCacheUpdate2[number]=FALSE;
#endif
	 
  if(!wsTCacheUpdate2[number])
  {
   wsTCacheUpdate2[number]=TRUE;
   switch(wsVMode)
   {
	case 7:
     t_adr=0x8000+(number<<5);
	 t_index=number<<6;
	 for(i=0;i<8;i++)
	 {
	  byte0=wsRAM[t_adr++];
	  byte1=wsRAM[t_adr++];
	  byte2=wsRAM[t_adr++];
	  byte3=wsRAM[t_adr++];
	  wsTCache2[t_index]=byte0>>4;
	  wsTCacheFlipped2[t_index++]=byte3&15;
	  wsTCache2[t_index]=byte0&15;
	  wsTCacheFlipped2[t_index++]=byte3>>4;
	  wsTCache2[t_index]=byte1>>4;
	  wsTCacheFlipped2[t_index++]=byte2&15;
	  wsTCache2[t_index]=byte1&15;
	  wsTCacheFlipped2[t_index++]=byte2>>4;
	  wsTCache2[t_index]=byte2>>4;
	  wsTCacheFlipped2[t_index++]=byte1&15;
	  wsTCache2[t_index]=byte2&15;
	  wsTCacheFlipped2[t_index++]=byte1>>4;
	  wsTCache2[t_index]=byte3>>4;
	  wsTCacheFlipped2[t_index++]=byte0&15;
	  wsTCache2[t_index]=byte3&15;
	  wsTCacheFlipped2[t_index++]=byte0>>4;
	 }
	break;
    case 6:
	 t_adr=0x8000+(number<<5);
	 t_index=number<<6;
	 for(i=0;i<8;i++)
	 {
	  byte0=wsRAM[t_adr++];
	  byte1=wsRAM[t_adr++];
	  byte2=wsRAM[t_adr++];
	  byte3=wsRAM[t_adr++];
	  wsTCache2[t_index]=((byte0>>7)&1)|(((byte1>>7)&1)<<1)|(((byte2>>7)&1)<<2)|(((byte3>>7)&1)<<3);
	  wsTCacheFlipped2[t_index++]=((byte0)&1)|(((byte1)&1)<<1)|(((byte2)&1)<<2)|(((byte3)&1)<<3);
	  wsTCache2[t_index]=((byte0>>6)&1)|(((byte1>>6)&1)<<1)|(((byte2>>6)&1)<<2)|(((byte3>>6)&1)<<3);
	  wsTCacheFlipped2[t_index++]=((byte0>>1)&1)|(((byte1>>1)&1)<<1)|(((byte2>>1)&1)<<2)|(((byte3>>1)&1)<<3);
	  wsTCache2[t_index]=((byte0>>5)&1)|(((byte1>>5)&1)<<1)|(((byte2>>5)&1)<<2)|(((byte3>>5)&1)<<3);
	  wsTCacheFlipped2[t_index++]=((byte0>>2)&1)|(((byte1>>2)&1)<<1)|(((byte2>>2)&1)<<2)|(((byte3>>2)&1)<<3);
	  wsTCache2[t_index]=((byte0>>4)&1)|(((byte1>>4)&1)<<1)|(((byte2>>4)&1)<<2)|(((byte3>>4)&1)<<3);
	  wsTCacheFlipped2[t_index++]=((byte0>>3)&1)|(((byte1>>3)&1)<<1)|(((byte2>>3)&1)<<2)|(((byte3>>3)&1)<<3);
	  wsTCache2[t_index]=((byte0>>3)&1)|(((byte1>>3)&1)<<1)|(((byte2>>3)&1)<<2)|(((byte3>>3)&1)<<3);
	  wsTCacheFlipped2[t_index++]=((byte0>>4)&1)|(((byte1>>4)&1)<<1)|(((byte2>>4)&1)<<2)|(((byte3>>4)&1)<<3);
	  wsTCache2[t_index]=((byte0>>2)&1)|(((byte1>>2)&1)<<1)|(((byte2>>2)&1)<<2)|(((byte3>>2)&1)<<3);
	  wsTCacheFlipped2[t_index++]=((byte0>>5)&1)|(((byte1>>5)&1)<<1)|(((byte2>>5)&1)<<2)|(((byte3>>5)&1)<<3);
	  wsTCache2[t_index]=((byte0>>1)&1)|(((byte1>>1)&1)<<1)|(((byte2>>1)&1)<<2)|(((byte3>>1)&1)<<3);
	  wsTCacheFlipped2[t_index++]=((byte0>>6)&1)|(((byte1>>6)&1)<<1)|(((byte2>>6)&1)<<2)|(((byte3>>6)&1)<<3);
	  wsTCache2[t_index]=((byte0)&1)|(((byte1)&1)<<1)|(((byte2)&1)<<2)|(((byte3)&1)<<3);
	  wsTCacheFlipped2[t_index++]=((byte0>>7)&1)|(((byte1>>7)&1)<<1)|(((byte2>>7)&1)<<2)|(((byte3>>7)&1)<<3);
	 }
	break;
   default:
	t_adr=0x4000+(number<<4);
	t_index=number<<6;
 	for(i=0;i<8;i++)
	{
	 byte0=wsRAM[t_adr++];
	 byte1=wsRAM[t_adr++];
	 wsTCache2[t_index]=tiles[byte0][byte1][0][0];
	 wsTCacheFlipped2[t_index++]=tiles[byte0][byte1][1][0];
	 wsTCache2[t_index]=tiles[byte0][byte1][0][1];
	 wsTCacheFlipped2[t_index++]=tiles[byte0][byte1][1][1];
	 wsTCache2[t_index]=tiles[byte0][byte1][0][2];
	 wsTCacheFlipped2[t_index++]=tiles[byte0][byte1][1][2];
	 wsTCache2[t_index]=tiles[byte0][byte1][0][3];
	 wsTCacheFlipped2[t_index++]=tiles[byte0][byte1][1][3];
	 wsTCache2[t_index]=tiles[byte0][byte1][0][4];
	 wsTCacheFlipped2[t_index++]=tiles[byte0][byte1][1][4];
	 wsTCache2[t_index]=tiles[byte0][byte1][0][5];
	 wsTCacheFlipped2[t_index++]=tiles[byte0][byte1][1][5];
	 wsTCache2[t_index]=tiles[byte0][byte1][0][6];
	 wsTCacheFlipped2[t_index++]=tiles[byte0][byte1][1][6];
	 wsTCache2[t_index]=tiles[byte0][byte1][0][7];
	 wsTCacheFlipped2[t_index++]=tiles[byte0][byte1][1][7];
	}
   }	
  }
  if(flipv)
   line=7-line;
  if(fliph)
   memcpy(&wsTileRow[0],&wsTCacheFlipped2[(number<<6)|(line<<3)],8);
  else
   memcpy(&wsTileRow[0],&wsTCache2[(number<<6)|(line<<3)],8);
 }
}

}
