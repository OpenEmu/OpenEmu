/****************************************************************************
 *  gx_input.c
 *
 *  Genesis Plus GX input support
 *
 *  Eke-Eke (2008,2009)
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

#ifndef _GC_INPUT_H_
#define _GC_INPUT_H_

/* max. supported inputs */
#ifdef HW_DOL
#define MAX_INPUTS 4
#else
#define MAX_INPUTS 8
#endif

/* Configurable keys */
#define MAX_KEYS 8

/* Key configuration structure */
typedef struct 
{
  s8 device;
  u8 port;
  u8 padtype;
} t_input_config;


extern void gx_input_Init(void);
extern int gx_input_FindDevices(void);
extern void gx_input_SetDefault(void);
extern void gx_input_Config(u8 chan, u8 device, u8 type);
extern void gx_input_UpdateEmu(void);
extern void gx_input_UpdateMenu(u32 cnt);

#endif
