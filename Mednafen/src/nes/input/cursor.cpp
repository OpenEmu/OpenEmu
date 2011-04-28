#include "share.h"

static const uint8 GunSight[]={
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
static const uint8 MDFNcursor[11*19]=
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

void MDFN_DrawGunSight(MDFN_Surface *surface, int xc, int yc)
{
 uint32 *buf = surface->pixels;
 int x,y;
 int c,d;
 const int ctransform[2] = { surface->MakeColor(0xff, 0xff, 0xff), surface->MakeColor(0, 0, 0) };
 MDFN_Rect DisplayRect;

 NESPPU_GetDisplayRect(&DisplayRect);

  for(y=0;y<13;y++)
   for(x=0;x<13;x++)
   {
    uint8 a;
    a=GunSight[y*13+x];
    if(a)
    {
     c = (yc+y-7);
     d = (x-7) + xc * (DisplayRect.w + DisplayRect.x) / 256;
     if(c>=0 && d>=0 && d<DisplayRect.w && c<240)
     {
      if(a==3)
       buf[c * surface->pitch32 + d] = ~(buf[c * surface->pitch32 + d]);
      else
       buf[c * surface->pitch32 + d] = ctransform[(a - 1) & 1];
     }
    }
   }
}


void MDFN_DrawCursor(MDFN_Surface *surface, int xc, int yc)
{
 uint32 *buf = surface->pixels;
 int x,y;
 int c,d;
 const int ctransform[4] = { surface->MakeColor(0, 0, 0), surface->MakeColor(0, 0, 0),
			     surface->MakeColor(0xff, 0xff, 0xff), surface->MakeColor(0, 0, 0)
			   };
 MDFN_Rect DisplayRect;

 NESPPU_GetDisplayRect(&DisplayRect);

 for(y=0;y<19;y++)
  for(x=0;x<11;x++)
  {
   uint8 a;
   a=MDFNcursor[y*11+x];
   if(a)
   {
    c =(yc+y);
    d =x + xc * (DisplayRect.w + DisplayRect.x) / 256;

    if(c>=0 && d>=0 && d<DisplayRect.w && c<240)
     buf[c * surface->pitch32 + d] = ctransform[a & 0x3]; // + 0xFF; //+ 127;

   }
  }
}
