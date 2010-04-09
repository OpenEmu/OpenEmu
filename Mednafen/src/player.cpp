/* Mednafen - Multi-system Emulator
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

#include "mednafen.h"

#include <math.h>

#include "mednafen.h"
#include "video.h"
#include "player.h"
#include "memory.h"

static UTF8 *AlbumName, *Artist, *Copyright, **SongNames;
static int TotalSongs;

static ALWAYS_INLINE void DrawLine(uint32 *buf, uint32 color, uint32 bmatch, uint32 breplace, int x1, int y1, int x2, int y2)
{
 float dy_dx = (float)(y2 - y1) / (x2 - x1);
 int x;

 float r_ys = 0; //x1 * dy_dx;
 float r_ye = (x2 - x1) * dy_dx;

 for(x = x1; x <= x2; x++)
 {
  float ys = dy_dx * (x - 1 - x1) + dy_dx / 2;
  float ye = dy_dx * (x + 1 - x1) - dy_dx / 2;

  if(dy_dx > 0)
  {
   ys = round(ys);
   ye = round(ye);

   if(ys < r_ys) ys = r_ys;
   if(ye > r_ye) ye = r_ye;
   if(bmatch != ~0U)
    for(unsigned int y = (unsigned int) ys; y <= (unsigned int)ye; y++)
    {
     uint32 tmpcolor = color;
     if(buf[x + (y + y1) * (MDFNGameInfo->pitch >> 2)] == bmatch) tmpcolor = breplace;
     if(buf[x + (y + y1) * (MDFNGameInfo->pitch >> 2)] != breplace)
      buf[x + (y + y1) * (MDFNGameInfo->pitch >> 2)] = tmpcolor;
    }
   else
    for(unsigned int y = (unsigned int)ys; y <= (unsigned int)ye; y++)
     buf[x + (y + y1) * (MDFNGameInfo->pitch >> 2)] = color;
  }
  else
  {
   ys = round(ys);
   ye = round(ye);

   if(ys > r_ys) ys = r_ys;
   if(ye < r_ye) ye = r_ye;

   if(bmatch != ~0U)
    for(int y = (int)ys; y >= (int)ye; y--)
    {
     uint32 tmpcolor = color;

     if(buf[x + (y + y1) * (MDFNGameInfo->pitch >> 2)] == bmatch)
      tmpcolor = breplace;

     if(buf[x + (y + y1) * (MDFNGameInfo->pitch >> 2)] != breplace)
      buf[x + (y + y1) * (MDFNGameInfo->pitch >> 2)] = tmpcolor;
    }
   else
    for(int y = (int)ys; y >= (int)ye; y--)
    {
     buf[x + (y + y1) * (MDFNGameInfo->pitch >> 2)] = color;
    }
  }
 }
}

int Player_Init(int tsongs, UTF8 *album, UTF8 *artist, UTF8 *copyright, UTF8 **snames)
{
 AlbumName = album;
 Artist = artist;
 Copyright = copyright;
 SongNames = snames;

 TotalSongs = tsongs;

 MDFNGameInfo->width = 384;
 MDFNGameInfo->height = 240;
 MDFNGameInfo->pitch = 384 * sizeof(uint32);

 MDFNGameInfo->DisplayRect.x = 0;
 MDFNGameInfo->DisplayRect.y = 0;
 MDFNGameInfo->DisplayRect.w = 384;
 MDFNGameInfo->DisplayRect.h = 240;

 MDFNGameInfo->GameType = GMT_PLAYER;

 return(1);
}

void Player_Draw(uint32 *XBuf, int CurrentSong, int16 *samples, int32 sampcount)
{
 MDFN_Rect *dr = &MDFNGameInfo->DisplayRect;
 int x,y;
 uint32 *backXB;
 uint32 text_color = MK_COLOR(0xE8, 0xE8, 0xE8);

 XBuf += dr->x + (dr->y) * (MDFNGameInfo->pitch >> 2);
 backXB = XBuf;

 // Draw the background color
 for(y = 0; y < dr->h; y++)
  MDFN_FastU32MemsetM8(&XBuf[y * (MDFNGameInfo->pitch >> 2)], MK_COLOR(0x20, 0x00, 0x08), dr->w);

 // Now we draw the waveform data.  It should be centered vertically, and extend the screen horizontally from left to right.
 int32 x_scale;
 float y_scale;
 int lastX, lastY;


 x_scale = (sampcount << 8) / dr->w;

 y_scale = (float)dr->h;

 if(sampcount)
 {
  for(int wc = 0; wc < MDFNGameInfo->soundchan; wc++)
  {
   uint32 color =  wc ? MK_COLOR(0x80, 0xff, 0x80) : MK_COLOR(0x80, 0x80, 0xFF);
   if(MDFNGameInfo->soundchan == 1) color = MK_COLOR(0x80, 0xc0, 0xc0);

   lastX = -1;
   lastY = 0;

   for(x = 0; x < dr->w; x++)
   {
    float samp = ((float)-samples[wc + (x * x_scale >> 8) * MDFNGameInfo->soundchan]) / 32768;
    int ypos;

    ypos = (dr->h / 2) + (int)(y_scale * samp);
    if(ypos < 0 || ypos >= dr->h) ypos = dr->h / 2;

    if(lastX >= 0) 
     DrawLine(backXB, color, wc ? MK_COLOR(0x80, 0x80, 0xFF) : ~0, MK_COLOR(0x80, 0xc0, 0xc0), lastX, lastY, x, ypos);
    lastX = x;
    lastY = ypos;
   }
  }
 }

 // Quick warning:  DrawTextTransShadow() has the possibility of drawing past the visible display area by 1 pixel on each axis.  This should only be a cosmetic issue
 // if 1-pixel line overflowing occurs onto the next line(most likely with NES, where width == pitch).  Fixme in the future?

 XBuf += 2 * (MDFNGameInfo->pitch >> 2);
 if(AlbumName)
  DrawTextTransShadow(XBuf, MDFNGameInfo->pitch, dr->w, AlbumName, text_color, MK_COLOR(0x00, 0x18, 0x10), 1);

 XBuf += (13 + 2) * (MDFNGameInfo->pitch >> 2);
 if(Artist)
  DrawTextTransShadow(XBuf, MDFNGameInfo->pitch, dr->w, Artist, text_color, MK_COLOR(0x00, 0x18, 0x10), 1);

 XBuf += (13 + 2) * (MDFNGameInfo->pitch >> 2);
 if(Copyright)
  DrawTextTransShadow(XBuf, MDFNGameInfo->pitch, dr->w, Copyright, text_color, MK_COLOR(0x00, 0x18, 0x10), 1);

 XBuf += (13 * 2) * (MDFNGameInfo->pitch >> 2);
 
 // If each song has an individual name, show this song's name.
 UTF8 *tmpsong = SongNames?SongNames[CurrentSong] : 0;

 if(!tmpsong && TotalSongs > 1)
  tmpsong = (UTF8 *)_("Song:");

 if(tmpsong)
  DrawTextTransShadow(XBuf, MDFNGameInfo->pitch, dr->w, tmpsong, text_color, MK_COLOR(0x00, 0x18, 0x10), 1);
 
 XBuf += (13 + 2) * (MDFNGameInfo->pitch >> 2);
 if(TotalSongs > 1)
 {
  char snbuf[32];
  snprintf(snbuf, 32, "<%d/%d>", CurrentSong + 1, TotalSongs);
  DrawTextTransShadow(XBuf, MDFNGameInfo->pitch, dr->w, (uint8*)snbuf, text_color, MK_COLOR(0x00, 0x18, 0x10), 1);
 }
}
