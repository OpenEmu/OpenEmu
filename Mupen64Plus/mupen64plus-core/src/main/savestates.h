/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - savestates.h                                            *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Olejl Tillin9                                      *
 *   Copyright (C) 2008 Richard42 Tillin9                                  *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef __SAVESTAVES_H__
#define __SAVESTAVES_H__

#define SAVESTATE 1
#define LOADSTATE 2
#define SAVEPJ64STATE 4

extern int savestates_job;

void savestates_save(void);
void savestates_load(void);
int savestates_save_pj64(void);
void savestates_load_pj64(void);
char* savestates_get_pj64_filename(void);

void savestates_select_slot(unsigned int s);
unsigned int savestates_get_slot(void);
void savestates_set_autoinc_slot(int b);
int savestates_get_autoinc_slot(void);
void savestates_inc_slot(void);
void savestates_select_filename(const char* fn);
char* savestates_get_filename(void);

typedef struct _TLB_pj64
    {
    unsigned int _EntryDefined;

    struct _BreakDownPageMask
        {
        unsigned int zero : 13;
        unsigned int Mask : 12;
        unsigned int zero2 : 7;
        } BreakDownPageMask;

    struct _BreakDownEntryHi
        {
        unsigned int ASID : 8;
        unsigned int Zero : 4;
        unsigned int G : 1;
        unsigned int VPN2 : 19;
        } BreakDownEntryHi;

    struct _BreakDownEntryLo0 
        {
        unsigned int GLOBAL: 1;
        unsigned int V : 1;
        unsigned int D : 1;
        unsigned int C : 3;
        unsigned int PFN : 20;
        unsigned int ZERO: 6;
        } BreakDownEntryLo0;

    struct _BreakDownEntryLo1 
        {
        unsigned int GLOBAL: 1;
        unsigned int V : 1;
        unsigned int D : 1;
        unsigned int C : 3;
        unsigned int PFN : 20;
        unsigned int ZERO: 6;
        } BreakDownEntryLo1;
    } TLB_pj64;

#endif /* __SAVESTAVES_H__ */

