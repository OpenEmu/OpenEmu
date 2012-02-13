/* FCE Ultra - NES/Famicom Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __LIBSNES__
void FCEUSS_Load(void);
void FCEUSS_Save(void);
#else
void FCEUSS_Save(const char * fname);
int FCEUSS_Load(const char * fname);
#endif

typedef struct {
	void *v;
	uint32 s;
	const char *desc;
} SFORMAT;

void ResetExState(void (*PreSave)(void),void (*PostSave)(void));
void AddExState(void *v, uint32 s, int type, const char *desc);

#define FCEUSTATE_RLSB      0x80000000

#ifdef __cplusplus
}
#endif
