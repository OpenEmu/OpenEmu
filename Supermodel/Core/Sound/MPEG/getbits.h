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
 * getbits.h
 * 
 * Amp library internal header file.
 */


/* this file is a part of amp software, (C) tomislav uzelac 1996,1997
*/
 
/* getbits.h
 *
 * Created by: tomislav uzelac  Apr 1996
 */

/* gethdr() error codes
*/
#define GETHDR_ERR 0x1
#define GETHDR_NS  0x2
#define GETHDR_FL1 0x4
#define GETHDR_FL2 0x8
#define GETHDR_FF  0x10
#define GETHDR_SYN 0x20
#define GETHDR_EOF 0x30

/* buffer for the 'bit reservoir'
*/
#define BUFFER_SIZE     4096
#define BUFFER_AUX      2048
extern unsigned char buffer[];
extern int append,data,f_bdirty,bclean_bytes;
  
/* exports
*/
extern int fillbfr(unsigned int advance);
extern unsigned int getbits(int n);
extern int gethdr(struct AUDIO_HEADER *header);
extern void getcrc();
extern void getinfo(struct AUDIO_HEADER *header,struct SIDE_INFO *info);
extern int dummy_getinfo(int n);
extern int rewind_stream(int nbytes);


#ifdef GETBITS

/* buffer, AUX is used in case of input buffer "overflow", and its contents
 * are copied to the beginning of the buffer
*/
unsigned char buffer[BUFFER_SIZE+BUFFER_AUX];

/* buffer pointers: append counts in bytes, data in bits
 */
int append,data;

/* bit reservoir stuff. f_bdirty must be set to TRUE when starting play!
 */
int f_bdirty,bclean_bytes;
 
/* internal buffer, _bptr holds the position in _bits_
 */
static unsigned char _buffer[32];
static int _bptr;


/* buffer and bit manipulation functions
 */
static inline int _fillbfr(unsigned int size);
static inline int readsync();
static inline int get_input(unsigned char* bp, unsigned int size);
static inline unsigned int _getbits(int n);
int fillbfr(unsigned int advance);
unsigned int getbits(int n);
int dummy_getinfo(int n);
int rewind_stream(int nbytes);

/* header and side info parsing stuff 
 */
static inline void parse_header(struct AUDIO_HEADER *header);
static inline int header_sanity_check(struct AUDIO_HEADER *header);

int gethdr(struct AUDIO_HEADER *header);
void getcrc();
void getinfo(struct AUDIO_HEADER *header,struct SIDE_INFO *info);  

#endif /* GETBITS */

