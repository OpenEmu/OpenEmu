/****************************************************************************
 *  aram.c
 *
 *  ARAM wrapper for libogc
 *
 *  Softdev (2006)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ***************************************************************************/

#include "shared.h"

#define ARAMSTART 0x8000
#define ARAM_READ        1
#define ARAM_WRITE       0

/**
 * StartARAM
 * This simply sets up the call to internal libOGC.
 * Passing NULL for array list, and 0 items to allocate.
 * Required so libOGC knows to handle any interrupts etc.
 */
void StartARAM ()
{
  AR_Init (NULL, 0);
}

/**
 * ARAMPut
 *
 * Move data from MAIN memory to ARAM
 */
void ARAMPut (char *src, char *dst, int len)
{
  DCFlushRange (src, len);
  AR_StartDMA( ARAM_WRITE, (u32)src, (u32)dst, len);
  while (AR_GetDMAStatus());
}

/**
 * ARAMFetch
 *
 * This function will move data from ARAM to MAIN memory
 */
void
ARAMFetch (char *dst, char *src, int len)
{
  DCInvalidateRange(dst, len);
  AR_StartDMA( ARAM_READ, (u32) dst, (u32) src, len);
  while (AR_GetDMAStatus());
}
