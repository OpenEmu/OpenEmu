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

#include "video.h"
#include "player.h"

static std::string AlbumName, Artist, Copyright;
static std::vector<std::string> SongNames;
static int TotalSongs;

static INLINE void DrawLine(MDFN_Surface *surface, uint32 color, uint32 bmatch, uint32 breplace, int x1, int y1, int x2, int y2)
{
 uint32 *buf = surface->pixels;
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
     if(buf[x + (y + y1) * surface->pitch32] == bmatch) tmpcolor = breplace;
     if(buf[x + (y + y1) * surface->pitch32] != breplace)
      buf[x + (y + y1) * surface->pitch32] = tmpcolor;
    }
   else
    for(unsigned int y = (unsigned int)ys; y <= (unsigned int)ye; y++)
     buf[x + (y + y1) * surface->pitch32] = color;
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

     if(buf[x + (y + y1) * surface->pitch32] == bmatch)
      tmpcolor = breplace;

     if(buf[x + (y + y1) * surface->pitch32] != breplace)
      buf[x + (y + y1) * surface->pitch32] = tmpcolor;
    }
   else
    for(int y = (int)ys; y >= (int)ye; y--)
    {
     buf[x + (y + y1) * surface->pitch32] = color;
    }
  }
 }
}

int Player_Init(int tsongs, const std::string &album, const std::string &artist, const std::string &copyright, const std::vector<std::string> &snames)
{
 AlbumName = album;
 Artist = artist;
 Copyright = copyright;
 SongNames = snames;

 TotalSongs = tsongs;

 MDFNGameInfo->nominal_width = 384;
 MDFNGameInfo->nominal_height = 240;

 MDFNGameInfo->fb_width = 384;
 MDFNGameInfo->fb_height = 240;

 MDFNGameInfo->lcm_width = 384;
 MDFNGameInfo->lcm_height = 240;

 MDFNGameInfo->GameType = GMT_PLAYER;

 return(1);
}

int Player_Init(int tsongs, const std::string &album, const std::string &artist, const std::string &copyright, char **snames)
{
 std::vector<std::string> tmpvec;

 if(snames)
 {
  for(int i = 0; i < tsongs; i++)
   tmpvec.push_back(snames[i] ? snames[i] : "");
 }

 return Player_Init(tsongs, album, artist, copyright, tmpvec);
}

void Player_Draw(MDFN_Surface *surface, MDFN_Rect *dr, int CurrentSong, int16 *samples, int32 sampcount)
{
 uint32 *XBuf = surface->pixels;
 //MDFN_Rect *dr = &MDFNGameInfo->DisplayRect;
 int x,y;
 const uint32 text_color = surface->MakeColor(0xE8, 0xE8, 0xE8);
 const uint32 text_shadow_color = surface->MakeColor(0x00, 0x18, 0x10);
 const uint32 bg_color = surface->MakeColor(0x20, 0x00, 0x08);
 const uint32 left_color = surface->MakeColor(0x80, 0x80, 0xFF);
 const uint32 right_color = surface->MakeColor(0x80, 0xff, 0x80);
 const uint32 center_color = surface->MakeColor(0x80, 0xCC, 0xCC);

 dr->x = 0;
 dr->y = 0;
 dr->w = 384;
 dr->h = 240;

 // Draw the background color
 for(y = 0; y < dr->h; y++)
  MDFN_FastU32MemsetM8(&XBuf[y * surface->pitch32], bg_color, dr->w);

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
   uint32 color =  wc ? right_color : left_color; //MK_COLOR(0x80, 0xff, 0x80) : MK_COLOR(0x80, 0x80, 0xFF);

   if(MDFNGameInfo->soundchan == 1) 
    color = center_color; //MK_COLOR(0x80, 0xc0, 0xc0);

   lastX = -1;
   lastY = 0;

   for(x = 0; x < dr->w; x++)
   {
    float samp = ((float)-samples[wc + (x * x_scale >> 8) * MDFNGameInfo->soundchan]) / 32768;
    int ypos;

    ypos = (dr->h / 2) + (int)(y_scale * samp);

    if(ypos >= dr->h)
     ypos = dr->h - 1;

    if(ypos < 0)
     ypos = 0;

    if(lastX >= 0) 
     DrawLine(surface, color, wc ? left_color : ~0, center_color, lastX, lastY, x, ypos);
    lastX = x;
    lastY = ypos;
   }
  }
 }

 // Quick warning:  DrawTextTransShadow() has the possibility of drawing past the visible display area by 1 pixel on each axis.  This should only be a cosmetic issue
 // if 1-pixel line overflowing occurs onto the next line.  Fixme in the future?

 XBuf += 2 * surface->pitch32;
 DrawTextTransShadow(XBuf, surface->pitch32 * sizeof(uint32), dr->w, AlbumName, text_color, text_shadow_color, 1);

 XBuf += (13 + 2) * surface->pitch32;
 DrawTextTransShadow(XBuf, surface->pitch32 * sizeof(uint32), dr->w, Artist, text_color, text_shadow_color, 1);

 XBuf += (13 + 2) * surface->pitch32;
 DrawTextTransShadow(XBuf, surface->pitch32 * sizeof(uint32), dr->w, Copyright, text_color, text_shadow_color, 1);

 XBuf += (13 * 2) * surface->pitch32;

 // If each song has an individual name, show this song's name.
 {
  std::string tmpsong = "";

  if((unsigned int)CurrentSong < SongNames.size())
   tmpsong = SongNames[CurrentSong];

  if(tmpsong == "" && TotalSongs > 1)
   tmpsong = std::string(_("Song:"));

  DrawTextTransShadow(XBuf, surface->pitch32 * sizeof(uint32), dr->w, tmpsong, text_color, text_shadow_color, 1);
 }

 XBuf += (13 + 2) * surface->pitch32;
 if(TotalSongs > 1)
 {
  char snbuf[32];
  snprintf(snbuf, 32, "<%d/%d>", CurrentSong + 1, TotalSongs);
  DrawTextTransShadow(XBuf, surface->pitch32 * sizeof(uint32), dr->w, (uint8*)snbuf, text_color, text_shadow_color, 1);
 }
}
