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

#include <string.h>

#include "main.h"

static bool IsActive;

#define MK_COLOR_A(r,g,b,a) ( ((a)<<surface->format->Ashift) | ((r)<<surface->format->Rshift) | ((g) << surface->format->Gshift) | ((b) << surface->format->Bshift))

void Help_Draw(SDL_Surface *surface, const SDL_Rect *rect)
{
 if(!IsActive) return;

 uint32 * pixels = (uint32 *)surface->pixels;
 uint32 pitch32 = surface->pitch >> 2;

 for(unsigned int y = 0; y < rect->h; y++)
 {
  uint32 *row = pixels + y * pitch32;
  for(unsigned int x = 0; x < rect->w; x++)
   row[x] = MK_COLOR_A(0, 0, 0, 0xc0);
 }
 static const char *HelpStrings[] = { 
gettext_noop("Default key assignments:"),
gettext_noop(" F1 - Toggle Help"),
gettext_noop(" F2 - Configure Command Key"),
gettext_noop(" ALT + SHIFT + [n] - Configure buttons on port n(1-5)"),
gettext_noop(" CTRL + SHIFT + [n] - Select device on port n"),
gettext_noop(" ALT + S - Enable state rewinding"),
gettext_noop(" BACKSPACE - Rewind"),
gettext_noop(" F5 - Save State"), 
gettext_noop(" F6 - Select Disk"),
gettext_noop(" F7 - Load State"),
gettext_noop(" F8 - Insert coin; Insert/Eject disk"),
gettext_noop(" F9 - Take Screen Snapshot"),
gettext_noop(" F10 - (Soft) Reset, if available on emulated system."),
gettext_noop(" F11 - Power Toggle/Hard Reset"),
gettext_noop(" F12 - Exit"),
};

 DrawTextTrans(pixels, surface->pitch, rect->w, (UTF8*)_("Mednafen Quickie Help"), MK_COLOR_A(0x00,0xFF,0x00,0xFF), TRUE, FALSE);
 DrawTextTrans(pixels + pitch32 * 18, surface->pitch, rect->w, (UTF8*)_("(now with 30% more moaning)"), MK_COLOR_A(0xa0,0x40,0x40,0xFF), TRUE, TRUE);

 for(unsigned int x = 0; x < sizeof(HelpStrings) / sizeof(char *); x++)
   DrawTextTrans(pixels + pitch32 * (x * 18 + 30), surface->pitch, rect->w, (UTF8*)_(HelpStrings[x]), MK_COLOR_A(0x00,0xFF,0x00,0xFF), FALSE, FALSE);
}

bool Help_IsActive(void)
{

 return(IsActive);
}

bool Help_Toggle(void)
{
 IsActive = !IsActive;
 return(IsActive);
}

void Help_Init(void)
{
 IsActive = MDFN_GetSettingB("helpenabled");
}

void Help_Close(void)
{
 MDFNI_SetSettingB("helpenable", IsActive);
}
