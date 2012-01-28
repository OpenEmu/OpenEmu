/*
 *  history.c
 *
 *   Generic ROM history list managment
 *
 *   Martin Disibio (6/17/08)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************************/

#ifndef _HISTORY_H
#define _HISTORY_H

#include "filesel.h"

#define NUM_HISTORY_ENTRIES  (10)

/****************************************************************************
 * ROM Play History
 *
 ****************************************************************************/ 
typedef struct 
{
  char filepath[MAXJOLIET];
  char filename[MAXJOLIET];
} t_history_entry;

typedef struct
{
  t_history_entry entries[NUM_HISTORY_ENTRIES];
} t_history;

extern t_history history;
extern void history_add_file(char *filepath, char *filename);
extern void history_save(void);
extern void history_load(void);
extern void history_default(void);

#endif
