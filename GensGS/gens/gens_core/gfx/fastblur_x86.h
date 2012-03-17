/***************************************************************************
 * Gens: Fast Blur function. (x86 asm function prototypes)                 *
 *                                                                         *
 * Copyright (c) 1999-2002 by Stéphane Dallongeville                       *
 * Copyright (c) 2003-2004 by Stéphane Akhoun                              *
 * Copyright (c) 2008 by David Korth                                       *
 *                                                                         *
 * This program is free software; you can redistribute it and/or modify it *
 * under the terms of the GNU General Public License as published by the   *
 * Free Software Foundation; either version 2 of the License, or (at your  *
 * option) any later version.                                              *
 *                                                                         *
 * This program is distributed in the hope that it will be useful, but     *
 * WITHOUT ANY WARRANTY; without even the implied warranty of              *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the           *
 * GNU General Public License for more details.                            *
 *                                                                         *
 * You should have received a copy of the GNU General Public License along *
 * with this program; if not, write to the Free Software Foundation, Inc., *
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.           *
 ***************************************************************************/

#ifndef GENS_FASTBLUR_X86_H
#define GENS_FASTBLUR_X86_H

#ifdef __cplusplus
extern "C" {
#endif

void Fast_Blur_16_x86(void);
void Fast_Blur_16_x86_mmx(void);

void Fast_Blur_32_x86(void);
void Fast_Blur_32_x86_mmx(void);

#ifdef __cplusplus
}
#endif

#endif
