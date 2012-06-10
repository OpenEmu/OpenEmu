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

#include "video-common.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>

#include <trio/trio.h>

#include "png.h"

void MDFNI_SaveSnapshot(const MDFN_Surface *src, const MDFN_Rect *rect, const MDFN_Rect *LineWidths)
{
 FileWrapper *pp = NULL;

 try
 {
  std::string fn;
  int u = 0;

  try
  {
   pp = new FileWrapper(MDFN_MakeFName(MDFNMKF_SNAP_DAT, 0, NULL).c_str(), FileWrapper::MODE_READ);
  }
  catch(std::exception &e)
  {

  }

  if(pp)
  {
   if(pp->scanf("%d", &u) != 1)
    u = 0;

   delete pp;
   pp = NULL;
  }

  pp = new FileWrapper(MDFN_MakeFName(MDFNMKF_SNAP_DAT, 0, NULL).c_str(), FileWrapper::MODE_WRITE);

  pp->seek(0, SEEK_SET);
  pp->printf("%d\n", u + 1);

  delete pp;
  pp = NULL;

  fn = MDFN_MakeFName(MDFNMKF_SNAP, u, "png");

  PNGWrite(fn.c_str(), src, *rect, LineWidths);

  MDFN_DispMessage(_("Screen snapshot %d saved."), u);
 }
 catch(std::exception &e)
 {
  if(pp)
  {
   delete pp;
   pp = NULL;
  }

  MDFN_PrintError(_("Error saving screen snapshot: %s"), e.what());
  MDFN_DispMessage(_("Error saving screen snapshot: %s"), e.what());
 }
}

void MDFN_DispMessage(const char *format, ...) throw()
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


