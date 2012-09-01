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
 * proto.h
 * 
 * Amp library internal header file.
 */


/* From: util.c */
void die(char *, ...);
void warn(char *, ...);
void msg(char *, ...);
void debugSetup(char *);
void debugOptions();

/* From: audioIO_<OSTYPE>.c */
void audioOpen(int frequency, int stereo, int volume);
void audioSetVolume(int);
void audioFlush();
void audioClose();
int  audioWrite(char *, int);
int  getAudioFd();
void audioBufferOn(int);


/* From: buffer.c */
void printout(void);
int  audioBufferOpen(int, int, int);
void audioBufferClose();
void audioBufferWrite(char *, int);
void audioBufferFlush();

/* From: audio.c */
void displayUsage();
