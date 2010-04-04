/* z80_ops.c: Process the next opcode
   Copyright (c) 1999-2004 Philip Kendall, Witold Filipczyk

   $Id: z80_ops.c,v 1.56 2004/06/09 10:55:09 pak21 Exp $

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

/* Modified somewhat for usage in Mednafen */


#include "../mednafen.h"

#include "z80.h"

#include "z80_macros.h"

int iline = 0;

void z80_set_interrupt(int set)
{
 iline = set;
}

int z80_do_opcode( void )
{
 if(iline)
 {
  if(z80_interrupt())
  {
   int ret = z80_tstates - last_z80_tstates;
   last_z80_tstates = z80_tstates;
   return(ret);
  }
 }
 uint8 opcode;

    /* Check to see if M1 cycles happen on even z80_tstates */
    //if( z80_tstates & 1 )
    // z80_tstates++;
    //uint16 lastpc = PC;

    opcode = Z80_RB_MACRO( PC ); 
    //printf("Z80-op: %04x, %02x\n", PC, opcode);
    z80_tstates++;

    PC++; 
    R++;

    switch(opcode) 
    {
     #include "opcodes_base.c"
    }

   int ret = z80_tstates - last_z80_tstates;
   last_z80_tstates = z80_tstates;

   //printf("PC: %04x, %02x, time=%d\n", lastpc, opcode, ret);

   return(ret);
}
