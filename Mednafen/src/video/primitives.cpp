#include "video-common.h"

void MDFN_DrawRectangle(uint32 *XBuf, int pitch, int xpos, int ypos, uint32 color, uint32 width, uint32 height)
{
 uint32 x;
 uint32 y;

 XBuf += pitch * ypos + xpos;
 /* Draw top and bottom horiz */
 for(x=0;x<width;x++)
 {
  XBuf[x] = color;
  XBuf[x + (height-1) * pitch] = color;
 }

 for(y=0;y<height;y++)
 {
  XBuf[y * pitch] = color;
  XBuf[y * pitch + width-1] = color;
 }
}

void MDFN_DrawRectangleAlpha(uint32 *XBuf, int pitch, int xpos, int ypos, uint32 color, uint32 alpha_color, uint32 width, uint32 height)
{
 uint32 x;
 uint32 y;

 XBuf += pitch * ypos + xpos;
 /* Draw top and bottom horiz */
 for(x=0;x<width;x++)
 {
  XBuf[x] = color;
  XBuf[x + (height-1) * pitch] = color;
 }

 for(y=0;y<height;y++)
 {
  XBuf[y * pitch] = color;
  XBuf[y * pitch + width-1] = color;
 }

 /* Now draw inner transparency area. */
 XBuf += 1 + 1 * pitch;

 for(y=0;y<height-2;y++)
  for(x=0;x<width-2;x++)
  {
   XBuf[y*pitch+x] = alpha_color; //MK_COLORA(0x00, 0x00, 0x00, 0x80);
  }
}

void MDFN_DrawRectangleFill(uint32 *XBuf, int pitch, int xpos, int ypos, uint32 color, uint32 fillcolor, uint8 width, uint8 height)
{
 int x;
 int y;

 XBuf += pitch * ypos + xpos;
 /* Draw top and bottom horiz */
 for(x=0;x<width;x++)
 {
  XBuf[x] = color;
  XBuf[x + (height-1) * pitch] = color;
 }

 for(y=0;y<height;y++)
 {
  XBuf[y * pitch] = color;
  XBuf[y * pitch + width-1] = color;
 }

 /* Now draw innerarea. */
 XBuf += 1 + 1 * pitch;

 for(y=0;y<height-2;y++)
  for(x=0;x<width-2;x++)
  {
   XBuf[y*pitch+x] = fillcolor;
  }
}
