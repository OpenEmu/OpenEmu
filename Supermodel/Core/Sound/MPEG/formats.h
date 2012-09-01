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
 * formats.h
 * 
 * Amp library internal header file.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
 
/* formats.h
 *
 * Created by: tomislav uzelac Dec 22 1996
 */
extern void wav_end(struct AUDIO_HEADER *header);
extern void wav_begin(void);

#ifdef FORMATS
void wav_end(struct AUDIO_HEADER *header);
void wav_begin(void);
#endif /* FORMATS */
