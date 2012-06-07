/* z80.c: z80 supplementary functions
   Copyright (c) 1999-2003 Philip Kendall

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

   Author contact information:

   E-mail: philip-fuse@shadowmagic.org.uk

*/

#include "z80.h"
#include "z80_macros.h"

void (*z80_writebyte)(uint16, uint8);
uint8 (*z80_readbyte)(uint16);
void (*z80_writeport)(uint16, uint8);
uint8 (*z80_readport)(uint16);

/* Whether a half carry occurred or not can be determined by looking at
   the 3rd bit of the two arguments and the result; these are hashed
   into this table in the form r12, where r is the 3rd bit of the
   result, 1 is the 3rd bit of the 1st argument and 2 is the
   third bit of the 2nd argument; the tables differ for add and subtract
   operations */
const uint8 halfcarry_add_table[] =
  { 0, FLAG_H, FLAG_H, FLAG_H, 0, 0, 0, FLAG_H };
const uint8 halfcarry_sub_table[] =
  { 0, 0, FLAG_H, 0, FLAG_H, 0, FLAG_H, FLAG_H };

/* Similarly, overflow can be determined by looking at the 7th bits; again
   the hash into this table is r12 */
const uint8 overflow_add_table[] = { 0, 0, 0, FLAG_V, FLAG_V, 0, 0, 0 };
const uint8 overflow_sub_table[] = { 0, FLAG_V, 0, 0, 0, 0, FLAG_V, 0 };

/* Some more tables; initialised in z80_init_tables() */

uint8 sz53_table[0x100]; /* The S, Z, 5 and 3 bits of the index */
uint8 parity_table[0x100]; /* The parity of the lookup value */
uint8 sz53p_table[0x100]; /* OR the above two tables together */

/* This is what everything acts on! */
processor z80;
uint64 z80_tstates;
uint64 last_z80_tstates;

static void z80_init_tables(void);

/* Set up the z80 emulation */
void z80_init( void )
{
  z80_init_tables();
}

/* Initalise the tables used to set flags */
static void z80_init_tables(void)
{
  int i,j,k;
  uint8 parity;

  for(i=0;i<0x100;i++) {
    sz53_table[i]= i & ( FLAG_3 | FLAG_5 | FLAG_S );
    j=i; parity=0;
    for(k=0;k<8;k++) { parity ^= j & 1; j >>=1; }
    parity_table[i]= ( parity ? 0 : FLAG_P );
    sz53p_table[i] = sz53_table[i] | parity_table[i];
  }

  sz53_table[0]  |= FLAG_Z;
  sz53p_table[0] |= FLAG_Z;

}

/* Reset the z80 */
void z80_reset( void )
{
  AF =BC =DE =HL =0;
  AF_=BC_=DE_=HL_=0;
  IX=IY=0;
  I=R=R7=0;
  SP=PC=0;
  IFF1=IFF2=IM=0;
  z80.halted=0;

  z80.interrupts_enabled_at = -1;
  z80_tstates = last_z80_tstates = 0;
}

/* Process a z80 maskable interrupt */
int z80_interrupt( void )
{

  if( IFF1 ) {

    /* If interrupts have just been enabled, don't accept the interrupt now,
       but check after the next instruction has been executed */
    if( z80_tstates == z80.interrupts_enabled_at ) {
      return 0;
    }

    if( z80.halted ) { PC++; z80.halted = 0; }
    
    IFF1=IFF2=0;

    Z80_WB_MACRO( --SP, PCH ); Z80_WB_MACRO( --SP, PCL );

    R++;

    switch(IM) {
      case 0: PC = 0x0038; z80_tstates += 7; break;
      case 1: PC = 0x0038; z80_tstates += 7; break;
      case 2: 
	{
	  uint16 inttemp=(0x100*I)+0xff;
	  PCL = Z80_RB_MACRO(inttemp++); PCH = Z80_RB_MACRO(inttemp);
	  z80_tstates += 7;
	  break;
	}
      //default:
      //	ui_error( UI_ERROR_ERROR, "Unknown interrupt mode %d", IM );
      //	fuse_abort();
    }

    return 1;			/* Accepted an interrupt */

  } else {

    return 0;			/* Did not accept an interrupt */

  }
}

/* Process a z80 non-maskable interrupt */
void z80_nmi( void )
{
  if( z80.halted ) { PC++; z80.halted = 0; }

  IFF1 = 0;

  Z80_WB_MACRO( --SP, PCH ); Z80_WB_MACRO( --SP, PCL );

  /* FIXME: how is R affected? */

  /* FIXME: how does contention apply here? */
  z80_tstates += 11; PC = 0x0066;
}

#if 0
/* Routines for transferring the Z80 contents to and from snapshots */
static void
z80_from_snapshot( libspectrum_snap *snap )
{
  A  = libspectrum_snap_a ( snap ); F  = libspectrum_snap_f ( snap );
  A_ = libspectrum_snap_a_( snap ); F_ = libspectrum_snap_f_( snap );

  BC  = libspectrum_snap_bc ( snap ); DE  = libspectrum_snap_de ( snap );
  HL  = libspectrum_snap_hl ( snap ); BC_ = libspectrum_snap_bc_( snap );
  DE_ = libspectrum_snap_de_( snap ); HL_ = libspectrum_snap_hl_( snap );

  IX = libspectrum_snap_ix( snap ); IY = libspectrum_snap_iy( snap );
  I  = libspectrum_snap_i ( snap ); R = R7 = libspectrum_snap_r( snap );
  SP = libspectrum_snap_sp( snap ); PC = libspectrum_snap_pc( snap );

  IFF1 = libspectrum_snap_iff1( snap ); IFF2 = libspectrum_snap_iff2( snap );
  IM = libspectrum_snap_im( snap );

  z80.halted = libspectrum_snap_halted( snap );

  z80.interrupts_enabled_at =
    libspectrum_snap_last_instruction_ei( snap ) ? z80_tstates : -1;
}
  
static void
z80_to_snapshot( libspectrum_snap *snap )
{
  uint8 r_register;

  r_register = ( R7 & 0x80 ) | ( R & 0x7f );

  libspectrum_snap_set_a  ( snap, A   ); libspectrum_snap_set_f  ( snap, F   );
  libspectrum_snap_set_a_ ( snap, A_  ); libspectrum_snap_set_f_ ( snap, F_  );

  libspectrum_snap_set_bc ( snap, BC  ); libspectrum_snap_set_de ( snap, DE  );
  libspectrum_snap_set_hl ( snap, HL  ); libspectrum_snap_set_bc_( snap, BC_ );
  libspectrum_snap_set_de_( snap, DE_ ); libspectrum_snap_set_hl_( snap, HL_ );

  libspectrum_snap_set_ix ( snap, IX  ); libspectrum_snap_set_iy ( snap, IY  );
  libspectrum_snap_set_i  ( snap, I   );
  libspectrum_snap_set_r  ( snap, r_register );
  libspectrum_snap_set_sp ( snap, SP  ); libspectrum_snap_set_pc ( snap, PC  );

  libspectrum_snap_set_iff1( snap, IFF1 );
  libspectrum_snap_set_iff2( snap, IFF2 );
  libspectrum_snap_set_im( snap, IM );

  libspectrum_snap_set_halted( snap, z80.halted );
  libspectrum_snap_set_last_instruction_ei(
    snap, z80.interrupts_enabled_at == z80_tstates
  );
}
#endif



int z80_state_action(StateMem *sm, int load, int data_only, const char *section_name)
{
 uint8 r_register;

 SFORMAT StateRegs[] =
 {
  SFVARN(z80.af.w, "AF"),
  SFVARN(z80.bc.w, "BC"),
  SFVARN(z80.de.w, "DE"),
  SFVARN(z80.hl.w, "HL"),
  SFVARN(z80.af_.w, "AF_"),
  SFVARN(z80.bc_.w, "BC_"),
  SFVARN(z80.de_.w, "DE_"),
  SFVARN(z80.hl_.w, "HL_"),
  SFVARN(z80.ix.w, "IX"),
  SFVARN(z80.iy.w, "IY"),
  SFVARN(z80.i, "I"),
  SFVARN(z80.sp.w, "SP"),
  SFVARN(z80.pc.w, "PC"),
  SFVARN(z80.iff1, "IFF1"),
  SFVARN(z80.iff2, "IFF2"),
  SFVARN(z80.im, "IM"),
  SFVARN(r_register, "R"),

  SFVARN(z80.interrupts_enabled_at, "interrupts_enabled_at"),
  SFVARN(z80.halted, "halted"),

  SFVAR(z80_tstates),
  SFVAR(last_z80_tstates),

  SFEND
 };

 if(!load)
  r_register = (z80.r7 & 0x80) | (z80.r & 0x7f);

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, section_name))
 {
  return(0);
 }

 if(load)
 {
  z80.r7 = r_register & 0x80;
  z80.r = r_register & 0x7F;
 }

 return(1);
}

