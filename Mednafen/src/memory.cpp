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

#include <stdlib.h>
#include <errno.h>

#include "memory.h"

void *MDFN_calloc_real(uint32 nmemb, uint32 size, const char *purpose, const char *_file, const int _line)
{
 void *ret;

 ret = calloc(nmemb, size);

 if(!ret)
 {
  MDFN_PrintError(_("Error allocating(calloc) %u bytes for \"%s\" in %s(%d)!"), size, purpose, _file, _line);
  return(0);
 }
 return ret;
}

void *MDFN_malloc_real(uint32 size, const char *purpose, const char *_file, const int _line)
{
 void *ret;

 ret = malloc(size);

 if(!ret)
 {
  MDFN_PrintError(_("Error allocating(malloc) %u bytes for \"%s\" in %s(%d)!"), size, purpose, _file, _line);
  return(0);
 }
 return ret;
}

void *MDFN_realloc_real(void *ptr, uint32 size, const char *purpose, const char *_file, const int _line)
{
 void *ret;

 ret = realloc(ptr, size);

 if(!ret)
 {
  MDFN_PrintError(_("Error allocating(realloc) %u bytes for \"%s\" in %s(%d)!"), size, purpose, _file, _line);
  return(0);
 }
 return ret;
}

void MDFN_free(void *ptr)
{
 free(ptr);
}

