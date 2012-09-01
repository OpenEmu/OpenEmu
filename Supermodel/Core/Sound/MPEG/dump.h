/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/
 
/*
 * dump.h
 * 
 * Amp library internal header file.
 */
 

/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
/* dump.h
 * 
 * Last modified by: tomislav uzelac May 31 1997
 */

extern void dump(int *length);
extern void show_header(struct AUDIO_HEADER *header);

#ifdef DUMP
void dump(int *length);
void show_header(struct AUDIO_HEADER *header);
/*
static char *t_modes[] = {
        "stereo","joint_stereo","dual_channel","single_channel"};
*/
#endif /* DUMP */
