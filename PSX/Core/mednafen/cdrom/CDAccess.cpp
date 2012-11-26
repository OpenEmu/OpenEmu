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

#include "../mednafen.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


#include "CDAccess.h"
#include "CDAccess_Image.h"

#ifdef HAVE_LIBCDIO
#include "CDAccess_Physical.h"
#endif

using namespace CDUtility;

CDAccess::CDAccess()
{

}

CDAccess::~CDAccess()
{

}

CDAccess *cdaccess_open(const char *path, bool image_memcache)
{
 CDAccess *ret;
 struct stat stat_buf;

 #ifdef HAVE_LIBCDIO
 if(path == NULL || (!stat(path, &stat_buf) && !S_ISREG(stat_buf.st_mode)))
  ret = new CDAccess_Physical(path);
 else
 #endif
  ret = new CDAccess_Image(path, image_memcache);

 return ret;
}

