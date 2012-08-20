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
#include "error.h"
#include <string.h>
#include <stdarg.h>
#include <trio/trio.h>

MDFN_Error::MDFN_Error() throw()
{
 abort();
}

MDFN_Error::MDFN_Error(int errno_code_new, const char *format, ...) throw()
{
 errno_code = errno_code_new;

 va_list ap;
 va_start(ap, format);
 error_message = trio_vaprintf(format, ap);
 va_end(ap);
}


MDFN_Error::MDFN_Error(const ErrnoHolder &enh)
{
 errno_code = enh.Errno();

 error_message = trio_aprintf("%s", enh.StrError());
}


MDFN_Error::~MDFN_Error() throw()
{
 if(error_message)
 {
  free(error_message);
  error_message = NULL;
 }
}

MDFN_Error::MDFN_Error(const MDFN_Error &ze_error) throw()
{
 if(ze_error.error_message)
  error_message = strdup(ze_error.error_message);
 else
  error_message = NULL;

 errno_code = ze_error.errno_code;
}

MDFN_Error& MDFN_Error::operator=(const MDFN_Error &ze_error) throw()
{
 char *new_error_message = ze_error.error_message ? strdup(ze_error.error_message) : NULL;
 int new_errno_code = ze_error.errno_code;

 if(error_message)
  free(error_message);

 error_message = new_error_message;
 errno_code = new_errno_code;

 return(*this);
}


const char * MDFN_Error::what(void) const throw()
{
 if(!error_message)
  return("Error allocating memory for the error message!");

 return(error_message);
}

int MDFN_Error::GetErrno(void) const throw()
{
 return(errno_code);
}

static const char *srr_wrap(int ret, const char *local_strerror)
{
 if(ret == -1)
  return("ERROR IN strerror_r()!!!");

 return(local_strerror);
}

static const char *srr_wrap(const char *ret, const char *local_strerror)
{
 if(ret == NULL)
  return("ERROR IN strerror_r()!!!");

 return(ret);
}

void ErrnoHolder::SetErrno(int the_errno)
{
 local_errno = the_errno;

 if(the_errno == 0)
  local_strerror[0] = 0;
 else
 {
  #ifdef HAVE_STRERROR_R
   const char *retv;

   retv = srr_wrap(strerror_r(the_errno, local_strerror, 256), local_strerror);

   if(retv != local_strerror)
    strncpy(local_strerror, retv, 255);

  #else	// No strerror_r :(

   strncpy(local_strerror, strerror(the_errno), 255);

  #endif

  local_strerror[255] = 0;
 }
}

