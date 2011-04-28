/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
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

#include "share.h"
#include "fkb.h"
#define AK2(x,y)	( (FKB_##x) | (FKB_##y <<8) )
#define AK(x) 		FKB_##x

static uint8 bufit[0x49];
static uint8 ksmode;
static uint8 ksindex;


static const uint16 matrix[9][2][4]=
{
{{AK(F8),AK(RETURN),AK(BRACKETLEFT),AK(BRACKETRIGHT)},   
	{AK(KANA),AK(RIGHTSHIFT),AK(BACKSLASH),AK(STOP)}},
{{AK(F7),AK(AT),AK(COLON),AK(SEMICOLON)}, 		
	{AK(UNDERSCORE),AK(SLASH),AK(MINUS),AK(CARET)}},
{{AK(F6),AK(O),AK(L),AK(K)},   
	{AK(PERIOD),AK(COMMA),AK(P),AK(0)}},
{{AK(F5),AK(I),AK(U),AK(J)},   	
	{AK(M),AK(N),AK(9),AK(8)}},
{{AK(F4),AK(Y),AK(G),AK(H)},  
	{AK(B),AK(V),AK(7),AK(6)}},
{{AK(F3),AK(T),AK(R),AK(D)},  
	{AK(F),AK(C),AK(5),AK(4)}},
{{AK(F2),AK(W),AK(S),AK(A)},  
	{AK(X),AK(Z),AK(E),AK(3)}},
{{AK(F1),AK(ESCAPE),AK(Q),AK(CONTROL)},    
	{AK(LEFTSHIFT),AK(GRAPH),AK(1),AK(2)}},
{{AK(CLEAR),AK(UP),AK(RIGHT),AK(LEFT)},    
	{AK(DOWN),AK(SPACE),AK(DELETE),AK(INSERT)}},
};

static void FKB_Write(uint8 v)
{
 v>>=1;
 if(v&2)
 {
  if((ksmode&1) && !(v&1))
   ksindex=(ksindex+1)%9;
 }
 ksmode=v;
}

static uint8 FKB_Read(int w, uint8 ret)
{
 //printf("$%04x, %d, %d\n",w+0x4016,ksindex,ksmode&1);
 if(w)
 {
  int x;

  ret&=~0x1E;
  for(x=0;x<4;x++)
    if(bufit[ matrix[ksindex][ksmode&1][x]&0xFF ] || bufit[ matrix[ksindex][ksmode&1][x]>>8])
    {
     ret|=1<<(x+1);
    }
  ret^=0x1E;
 }
 return(ret);
}

static void FKB_Strobe(void)
{
 ksmode=0;
 ksindex=0;
 //printf("strobe\n");
}

static void FKB_Update(void *data)
{
 memcpy(bufit+1,data,0x48);
}

static int StateActionFC(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
   SFARRAY(bufit, 0x49),
   SFVAR(ksmode),
   SFVAR(ksindex),
   SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "INPF");
 if(load)
 {

 }
 return(ret);
}


static INPUTCFC FKB={FKB_Read,FKB_Write,FKB_Strobe,FKB_Update,0,0, StateActionFC, 0x48, sizeof(uint8) };

INPUTCFC *MDFN_InitFKB(void)
{
 memset(bufit,0,sizeof(bufit));
 ksmode=ksindex=0;
 return(&FKB);
}
