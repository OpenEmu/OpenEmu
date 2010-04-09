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

MDFN_Error::MDFN_Error(int errno_code_new, const char *format, ...)
{
 errno_code = errno_code_new;

 va_list ap;
 va_start(ap, format);
 error_message = trio_vaprintf(format, ap);
 va_end(ap);
}

MDFN_Error::~MDFN_Error()
{
 if(error_message)
 {
  free(error_message);
  error_message = NULL;
 }
}

MDFN_Error::MDFN_Error(const MDFN_Error &ze_error)
{
 if(ze_error.error_message)
  error_message = strdup(ze_error.error_message);
}

const char * MDFN_Error::GetErrorMessage(void)
{
 return(error_message);
}

int MDFN_Error::GetErrno(void)
{
 return(errno_code);
}

