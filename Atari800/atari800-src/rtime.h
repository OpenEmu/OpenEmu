#ifndef RTIME_H_
#define RTIME_H_
/* Emulate ICD R-Time 8 cartridge
   Copyright 2000 Jason Duerstock <jason@cluephone.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */

#include "atari.h"

extern int RTIME_enabled;

int RTIME_ReadConfig(char *string, char *ptr);
void RTIME_WriteConfig(FILE *fp);
int RTIME_Initialise(int *argc, char *argv[]);
UBYTE RTIME_GetByte(void);
void RTIME_PutByte(UBYTE byte);
#endif /* RTIME_H_ */
