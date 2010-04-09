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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <trio/trio.h>

#include "video-common.h"

void MDFNI_SaveSnapshot(void)
{
 try
 {
  FILE *pp=NULL;
  std::string fn;
  int u;

  if(!(pp = fopen(MDFN_MakeFName(MDFNMKF_SNAP_DAT, 0, NULL).c_str(), "rb")))
   u = 0;
  else
  {
   if(fscanf(pp, "%d", &u) == 0)
    u = 0;
   fclose(pp);
  }

  if(!(pp = fopen(MDFN_MakeFName(MDFNMKF_SNAP_DAT, 0, NULL).c_str(), "wb")))
   throw(0);

  fseek(pp, 0, SEEK_SET);
  fprintf(pp, "%d\n", u + 1);
  fclose(pp);

  fn = MDFN_MakeFName(MDFNMKF_SNAP, u, "png");

  if(!MDFN_SavePNGSnapshot(fn.c_str(), (uint32*)MDFNGameInfo->fb, &MDFNGameInfo->DisplayRect, MDFNGameInfo->pitch))
   throw(0);

  MDFN_DispMessage(_("Screen snapshot %d saved."), u);
 }
 catch(int x)
 {
  MDFN_DispMessage(_("Error saving screen snapshot."));
 }
}

void MDFN_DispMessage(const char *format, ...)
{
 va_list ap;
 va_start(ap,format);
 char *msg = NULL;

 trio_vasprintf(&msg, format,ap);
 va_end(ap);

 MDFND_DispMessage((UTF8*)msg);
}

void MDFN_ResetMessages(void)
{
 MDFND_DispMessage(NULL);
}

