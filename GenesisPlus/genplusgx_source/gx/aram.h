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

#ifndef _ARAM_H
#define _ARAM_H

extern void StartARAM ();
extern void ARAMFetch (char *src, char *dst, int len);
extern void ARAMPut (char *src, char *dst, int len);

#endif
