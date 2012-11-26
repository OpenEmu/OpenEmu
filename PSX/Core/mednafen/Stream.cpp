// TODO/WIP

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
#include "Stream.h"

#include <trio/trio.h>

Stream::Stream()
{

}

Stream::~Stream()
{

}

void Stream::printf(const char *format, ...)
{
 char *str = NULL;
 int rc;

 va_list ap;

 va_start(ap, format);

 rc = trio_vasprintf(&str, format, ap);

 va_end(ap);

 if(rc < 0)
  throw MDFN_Error(0, "Error in trio_vasprintf()");
 else
 {
  try	// Bleck
  {
   write(str, rc);
  }
  catch(...)
  {
   free(str);
   throw;
  }
  free(str);
 }
}

#if 0
int Stream::scanf(const char *format, ...)
{


}

void Stream::printf(const char *format, ...)
{

}

void Stream::put_string(const char *str)
{
 write(str, strlen(str));
}

void Stream::put_string(const std::string &str)
{
 write(str.data(), str.size());
}
#endif

int Stream::get_line(std::string &str)
{
 uint8 c;

 str.clear();	// or str.resize(0)??

 while(read(&c, sizeof(c), false) > 0)
 {
  if(c == '\r' || c == '\n' || c == 0)
   return(c);

  str.push_back(c);
 }

 return(-1);
}

StreamFilter::StreamFilter()
{
 target_stream = NULL;
}

StreamFilter::StreamFilter(Stream *target_arg)
{
 target_stream = target_arg;
}

StreamFilter::~StreamFilter()
{
 if(target_stream)
  delete target_stream;
}

Stream* StreamFilter::steal(void)
{
 Stream *ret = target_stream;
 target_stream = NULL;
 return ret;
}
