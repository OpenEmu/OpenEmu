/* z80.h: z80 emulation core
   Copyright (c) 1999-2003 Philip Kendall

   $Id: z80.h,v 1.11 2004/07/04 18:48:40 pak21 Exp $

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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

   Author contact information:

   E-mail: pak21-fuse@srcf.ucam.org
   Postal address: 15 Crescent Road, Wokingham, Berks, RG40 2DB, England

*/

#ifndef FUSE_Z80_H
#define FUSE_Z80_H

/* Union allowing a register pair to be accessed as bytes or as a word */
typedef union {
#ifdef MSB_FIRST
  struct { uint8 h,l; } b;
#else
  struct { uint8 l,h; } b;
#endif
  uint16 w;
} regpair;

/* What's stored in the main processor */
typedef struct {
  regpair af,bc,de,hl;
  regpair af_,bc_,de_,hl_;
  regpair ix,iy;
  uint8 i;
  uint16 r;	/* The low seven bits of the R register. 16 bits long
			   so it can also act as an RZX instruction counter */
  uint8 r7;	/* The high bit of the R register */
  regpair sp,pc;
  uint8 iff1, iff2, im;
  int halted;

  /* Interrupts were enabled at this time; do not accept any interrupts
     until z80_tstates > this value */
  int32 interrupts_enabled_at;

} processor;

uint16 z80_getpc(void);

void z80_init(void);
void z80_reset(void);

int z80_interrupt( void );
void z80_nmi( void );

int z80_do_opcode(void);

void z80_enable_interrupts( void );

extern processor z80;
extern const uint8 halfcarry_add_table[];
extern const uint8 halfcarry_sub_table[];
extern const uint8 overflow_add_table[];
extern const uint8 overflow_sub_table[];
extern uint8 sz53_table[];
extern uint8 sz53p_table[];
extern uint8 parity_table[];

extern uint64 z80_tstates, last_z80_tstates;

extern void (*z80_writebyte)(uint16, uint8);
extern uint8 (*z80_readbyte)(uint16);
extern void (*z80_writeport)(uint16, uint8);
extern uint8 (*z80_readport)(uint16);

// Ok, I lied, not a macro!

//Write mem
static ALWAYS_INLINE void Z80_WB_MACRO(uint16 A, uint8 V)
{ 
 z80_tstates += 3; 
 z80_writebyte(A, V); 
}

// Write port
static ALWAYS_INLINE void Z80_WP_MACRO(uint16 A, uint8 V)
{ 
 z80_tstates += 4; 
 z80_writeport(A, V); 
}

// Read mem
static ALWAYS_INLINE uint8 Z80_RB_MACRO(uint16 A)
{
 z80_tstates += 3;
 return(z80_readbyte(A));
}

// Read port
static ALWAYS_INLINE uint8 Z80_RP_MACRO(uint16 A)
{
 z80_tstates += 4;
 return(z80_readport(A));
}


void z80_set_interrupt(int set);

#endif			/* #ifndef FUSE_Z80_H */
