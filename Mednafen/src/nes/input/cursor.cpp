#include "share.h"

static uint8 GunSight[]={
        0,0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,2,0,0,0,0,0,0,
        0,0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,2,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,3,0,0,0,0,0,0,
        1,2,1,2,0,3,3,3,0,2,1,2,1,
        0,0,0,0,0,0,3,0,0,0,0,0,0,
        0,0,0,0,0,0,0,0,0,0,0,0,0,
        0,0,0,0,0,0,2,0,0,0,0,0,0,
        0,0,0,0,0,0,1,0,0,0,0,0,0,
        0,0,0,0,0,0,2,0,0,0,0,0,0,
        0,0,0,0,0,0,1,0,0,0,0,0,0,
};
static uint8 MDFNcursor[11*19]=
{
 1,0,0,0,0,0,0,0,0,0,0,
 1,1,0,0,0,0,0,0,0,0,0,
 1,2,1,0,0,0,0,0,0,0,0,
 1,2,2,1,0,0,0,0,0,0,0,
 1,2,2,2,1,0,0,0,0,0,0,
 1,2,2,2,2,1,0,0,0,0,0,
 1,2,2,2,2,2,1,0,0,0,0,
 1,2,2,2,2,2,2,1,0,0,0,
 1,2,2,2,2,2,2,2,1,0,0,
 1,2,2,2,2,2,2,2,2,1,0,
 1,2,2,2,2,2,1,1,1,1,1,
 1,2,2,1,2,2,1,0,0,0,0,
 1,2,1,0,1,2,2,1,0,0,0,
 1,1,0,0,1,2,2,1,0,0,0,
 1,0,0,0,0,1,2,2,1,0,0,
 0,0,0,0,0,1,2,2,1,0,0,
 0,0,0,0,0,0,1,2,2,1,0,
 0,0,0,0,0,0,1,2,2,1,0,
 0,0,0,0,0,0,0,1,1,0,0,
};

void MDFN_DrawGunSight(uint32 *buf, int xc, int yc)
{
 int x,y;
 int c,d;
 const int ctransform[2] = { MK_COLOR(0xff,0xff,0xff), MK_COLOR(0,0,0) };

  for(y=0;y<13;y++)
   for(x=0;x<13;x++)
   {
    uint8 a;
    a=GunSight[y*13+x];
    if(a)
    {
     c = (yc+y-7);
     d = (x-7) + xc * (MDFNGameInfo->DisplayRect.w + MDFNGameInfo->DisplayRect.x) / 256;
     if(c>=0 && d>=0 && d<MDFNGameInfo->DisplayRect.w && c<240)
     {
      if(a==3)
       buf[c*(MDFNGameInfo->pitch >> 2)+d]=~(buf[c*(MDFNGameInfo->pitch >> 2)+d]);
      else
       buf[c*(MDFNGameInfo->pitch >> 2)+d]=ctransform[(a-1)&1];
     }
    }
   }
}


void MDFN_DrawCursor(uint32 *buf, int xc, int yc)
{
 int x,y;
 int c,d;
 const int ctransform[4] = { MK_COLOR(0,0,0), MK_COLOR(0,0,0), MK_COLOR(0xff,0xff,0xff), MK_COLOR(0,0,0) };

 for(y=0;y<19;y++)
  for(x=0;x<11;x++)
  {
   uint8 a;
   a=MDFNcursor[y*11+x];
   if(a)
   {
    c =(yc+y);
    d =x + xc * (MDFNGameInfo->DisplayRect.w + MDFNGameInfo->DisplayRect.x) / 256;

    if(c>=0 && d>=0 && d<MDFNGameInfo->DisplayRect.w && c<240)
     buf[c*(MDFNGameInfo->pitch >> 2)+d]=ctransform[a & 0x3]; // + 0xFF; //+ 127;

   }
  }
}
