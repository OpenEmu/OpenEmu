#include "mapinc.h"

/* I might want to add some code to the mapper 96 PPU hook function
   to not change CHR banks if the attribute table is being accessed,
   if I make emulation a little more accurate in the future.
*/
static uint8 latche;
static uint8 M96LA;
static uint8 *M96_CHR = NULL;

static DECLFW(Mapper96_write)
{
 latche=V;
 setprg32(0x8000,V&3);
 setchr4r(0x10,0x0000,(latche&4)|M96LA);
 setchr4r(0x10,0x1000,(latche&4)|3);
}

static void M96Hook(uint32 A)
{
 if((A&0x3000)!=0x2000) return;
 //if((A&0x3ff)>=0x3c0) return;
 M96LA=(A>>8)&3;
 setchr4r(0x10,0x0000,(latche&4)|M96LA);
}

static void M96Sync(void)
{
 setprg32(0x8000,latche&3);
 setchr4r(0x10,0x0000,(latche&4)|M96LA);
 setchr4r(0x10,0x1000,(latche&4)|3);
}

static int M96_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[]=
 {
  SFVARN(latche, "LATC"),
  SFVARN(M96LA, "LAVA"),
  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "MAPR");

 if(load)
  M96Sync();

 return(ret);
}

static void Mapper96_Close(void)
{
 if(M96_CHR)
  free(M96_CHR);
 M96_CHR = NULL;
}

static void Mapper96_Reset(CartInfo *info)
{
 latche = 0;
 M96LA = 0;
 M96Sync();
}

int Mapper96_Init(CartInfo *info)
{
 if(!(M96_CHR = (uint8 *)malloc(32768)))
  return(0);

 SetWriteHandler(0x8000, 0xffff, Mapper96_write);
 SetReadHandler(0x8000, 0xFFFF, CartBR);
 PPU_hook = M96Hook;
 info->StateAction = M96_StateAction;
 info->Close = Mapper96_Close;
 info->Power = Mapper96_Reset;
 SetupCartCHRMapping(0x10, M96_CHR, 32768, 1);
 latche=M96LA=0;
 setmirror(MI_0);

 return(1);
}

