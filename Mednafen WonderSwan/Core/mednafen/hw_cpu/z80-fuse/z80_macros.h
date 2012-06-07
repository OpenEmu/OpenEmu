/* z80_macros.h: Some commonly used z80 things as macros
   Copyright (c) 1999-2011 Philip Kendall

   $Id: z80_macros.h 4624 2012-01-09 20:59:35Z pak21 $

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

#ifndef FUSE_Z80_MACROS_H
#define FUSE_Z80_MACROS_H

/* Macros used for accessing the registers */
#define A   z80.af.b.h
#define F   z80.af.b.l
#define AF  z80.af.w

#define B   z80.bc.b.h
#define C   z80.bc.b.l
#define BC  z80.bc.w

#define D   z80.de.b.h
#define E   z80.de.b.l
#define DE  z80.de.w

#define H   z80.hl.b.h
#define L   z80.hl.b.l
#define HL  z80.hl.w

#define A_  z80.af_.b.h
#define F_  z80.af_.b.l
#define AF_ z80.af_.w

#define B_  z80.bc_.b.h
#define C_  z80.bc_.b.l
#define BC_ z80.bc_.w

#define D_  z80.de_.b.h
#define E_  z80.de_.b.l
#define DE_ z80.de_.w

#define H_  z80.hl_.b.h
#define L_  z80.hl_.b.l
#define HL_ z80.hl_.w

#define IXH z80.ix.b.h
#define IXL z80.ix.b.l
#define IX  z80.ix.w

#define IYH z80.iy.b.h
#define IYL z80.iy.b.l
#define IY  z80.iy.w

#define SPH z80.sp.b.h
#define SPL z80.sp.b.l
#define SP  z80.sp.w

#define PCH z80.pc.b.h
#define PCL z80.pc.b.l
#define PC  z80.pc.w

#define I  z80.i
#define R  z80.r
#define R7 z80.r7

#define IFF1 z80.iff1
#define IFF2 z80.iff2
#define IM   z80.im

#define IR ( ( z80.i ) << 8 | ( z80.r7 & 0x80 ) | ( z80.r & 0x7f ) )

/* The flags */

#define FLAG_C	0x01
#define FLAG_N	0x02
#define FLAG_P	0x04
#define FLAG_V	FLAG_P
#define FLAG_3	0x08
#define FLAG_H	0x10
#define FLAG_5	0x20
#define FLAG_Z	0x40
#define FLAG_S	0x80

/* Get the appropriate contended memory delay. Use a macro for performance
   reasons in the main core, but a function for flexibility when building
   the core tester */

#ifndef CORETEST

#if 0
#define contend_read(address,time) \
  if( memory_map_read[ (address) >> MEMORY_PAGE_SIZE_LOGARITHM ].contended ) \
    z80_tstates += ula_contention[ z80_tstates ]; \
  z80_tstates += (time);

#define contend_read_no_mreq(address,time) \
  if( memory_map_read[ (address) >> MEMORY_PAGE_SIZE_LOGARITHM ].contended ) \
    z80_tstates += ula_contention_no_mreq[ z80_tstates ]; \
  z80_tstates += (time);

#define contend_write_no_mreq(address,time) \
  if( memory_map_write[ (address) >> MEMORY_PAGE_SIZE_LOGARITHM ].contended ) \
    z80_tstates += ula_contention_no_mreq[ z80_tstates ]; \
  z80_tstates += (time);
#endif

#define contend_read(address,time) \
  z80_tstates += (time);

#define contend_read_no_mreq(address,time) \
  z80_tstates += (time);

#define contend_write_no_mreq(address,time) \
  z80_tstates += (time);


#else				/* #ifndef CORETEST */

void contend_read( uint16 address, uint32 time );
void contend_read_no_mreq( uint16 address, uint32 time );
void contend_write_no_mreq( uint16 address, uint32 time );

#endif				/* #ifndef CORETEST */

/* Some commonly used instructions */
#define AND(value)\
{\
  A &= (value);\
  F = FLAG_H | sz53p_table[A];\
}

#define ADC(value)\
{\
  uint16 adctemp = A + (value) + ( F & FLAG_C ); \
  uint8 lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    ( ( adctemp & 0x88 ) >> 1 );  \
  A=adctemp;\
  F = ( adctemp & 0x100 ? FLAG_C : 0 ) |\
    halfcarry_add_table[lookup & 0x07] | overflow_add_table[lookup >> 4] |\
    sz53_table[A];\
}

#define ADC16(value)\
{\
  uint32 add16temp= HL + (value) + ( F & FLAG_C ); \
  uint8 lookup = ( (        HL & 0x8800 ) >> 11 ) | \
			    ( (   (value) & 0x8800 ) >> 10 ) | \
			    ( ( add16temp & 0x8800 ) >>  9 );  \
  HL = add16temp;\
  F = ( add16temp & 0x10000 ? FLAG_C : 0 )|\
    overflow_add_table[lookup >> 4] |\
    ( H & ( FLAG_3 | FLAG_5 | FLAG_S ) ) |\
    halfcarry_add_table[lookup&0x07]|\
    ( HL ? 0 : FLAG_Z );\
}

#define ADD(value)\
{\
  uint16 addtemp = A + (value); \
  uint8 lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    ( ( addtemp & 0x88 ) >> 1 );  \
  A=addtemp;\
  F = ( addtemp & 0x100 ? FLAG_C : 0 ) |\
    halfcarry_add_table[lookup & 0x07] | overflow_add_table[lookup >> 4] |\
    sz53_table[A];\
}

#define ADD16(value1,value2)\
{\
  uint32 add16temp = (value1) + (value2); \
  uint8 lookup = ( (  (value1) & 0x0800 ) >> 11 ) | \
			    ( (  (value2) & 0x0800 ) >> 10 ) | \
			    ( ( add16temp & 0x0800 ) >>  9 );  \
  (value1) = add16temp;\
  F = ( F & ( FLAG_V | FLAG_Z | FLAG_S ) ) |\
    ( add16temp & 0x10000 ? FLAG_C : 0 )|\
    ( ( add16temp >> 8 ) & ( FLAG_3 | FLAG_5 ) ) |\
    halfcarry_add_table[lookup];\
}

/* This may look fairly inefficient, but the (gcc) optimiser does the
   right thing assuming it's given a constant for 'bit' */
#define BIT( bit, value ) \
{ \
  F = ( F & FLAG_C ) | FLAG_H | ( value & ( FLAG_3 | FLAG_5 ) ); \
  if( ! ( (value) & ( 0x01 << (bit) ) ) ) F |= FLAG_P | FLAG_Z; \
  if( (bit) == 7 && (value) & 0x80 ) F |= FLAG_S; \
}

#define BIT_I( bit, value, address ) \
{ \
  F = ( F & FLAG_C ) | FLAG_H | ( ( address >> 8 ) & ( FLAG_3 | FLAG_5 ) ); \
  if( ! ( (value) & ( 0x01 << (bit) ) ) ) F |= FLAG_P | FLAG_Z; \
  if( (bit) == 7 && (value) & 0x80 ) F |= FLAG_S; \
}  

#define CALL()\
{\
  uint8 calltempl, calltemph; \
  calltempl=Z80_RB_MACRO(PC++);\
  calltemph=Z80_RB_MACRO( PC ); \
  contend_read_no_mreq( PC, 1 ); PC++;\
  PUSH16(PCL,PCH);\
  PCL=calltempl; PCH=calltemph;\
}

#define CP(value)\
{\
  uint16 cptemp = A - value; \
  uint8 lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    ( (  cptemp & 0x88 ) >> 1 );  \
  F = ( cptemp & 0x100 ? FLAG_C : ( cptemp ? 0 : FLAG_Z ) ) | FLAG_N |\
    halfcarry_sub_table[lookup & 0x07] |\
    overflow_sub_table[lookup >> 4] |\
    ( value & ( FLAG_3 | FLAG_5 ) ) |\
    ( cptemp & FLAG_S );\
}

/* Macro for the {DD,FD} CB dd xx rotate/shift instructions */
#define DDFDCB_ROTATESHIFT(time, target, instruction)\
z80_tstates+=(time);\
{\
  (target) = Z80_RB_MACRO( tempaddr );\
  instruction( (target) );\
  Z80_WB_MACRO( tempaddr, (target) );\
}\
break

#define DEC(value)\
{\
  F = ( F & FLAG_C ) | ( (value)&0x0f ? 0 : FLAG_H ) | FLAG_N;\
  (value)--;\
  F |= ( (value)==0x7f ? FLAG_V : 0 ) | sz53_table[value];\
}

#define Z80_IN( reg, port )\
{\
  (reg)=Z80_RP_MACRO((port));\
  F = ( F & FLAG_C) | sz53p_table[(reg)];\
}

#define INC(value)\
{\
  (value)++;\
  F = ( F & FLAG_C ) | ( (value)==0x80 ? FLAG_V : 0 ) |\
  ( (value)&0x0f ? 0 : FLAG_H ) | sz53_table[(value)];\
}

#define LD16_NNRR(regl,regh)\
{\
  uint16 ldtemp; \
  ldtemp=Z80_RB_MACRO(PC++);\
  ldtemp|=Z80_RB_MACRO(PC++) << 8;\
  Z80_WB_MACRO(ldtemp++,(regl));\
  Z80_WB_MACRO(ldtemp,(regh));\
  break;\
}

#define LD16_RRNN(regl,regh)\
{\
  uint16 ldtemp; \
  ldtemp=Z80_RB_MACRO(PC++);\
  ldtemp|=Z80_RB_MACRO(PC++) << 8;\
  (regl)=Z80_RB_MACRO(ldtemp++);\
  (regh)=Z80_RB_MACRO(ldtemp);\
  break;\
}

#define JP()\
{\
  uint16 jptemp=PC; \
  PCL=Z80_RB_MACRO(jptemp++);\
  PCH=Z80_RB_MACRO(jptemp);\
}

#define JR()\
{\
  int8 jrtemp = Z80_RB_MACRO( PC ); \
  contend_read_no_mreq( PC, 1 ); contend_read_no_mreq( PC, 1 ); \
  contend_read_no_mreq( PC, 1 ); contend_read_no_mreq( PC, 1 ); \
  contend_read_no_mreq( PC, 1 ); \
  PC += jrtemp; \
}

#define OR(value)\
{\
  A |= (value);\
  F = sz53p_table[A];\
}

#define POP16(regl,regh)\
{\
  (regl)=Z80_RB_MACRO(SP++);\
  (regh)=Z80_RB_MACRO(SP++);\
}

#define PUSH16(regl,regh)\
{\
  Z80_WB_MACRO( --SP, (regh) );\
  Z80_WB_MACRO( --SP, (regl) );\
}

#define RET()\
{\
  POP16(PCL,PCH);\
}

#define RL(value)\
{\
  uint8 rltemp = (value); \
  (value) = ( (value)<<1 ) | ( F & FLAG_C );\
  F = ( rltemp >> 7 ) | sz53p_table[(value)];\
}

#define RLC(value)\
{\
  (value) = ( (value)<<1 ) | ( (value)>>7 );\
  F = ( (value) & FLAG_C ) | sz53p_table[(value)];\
}

#define RR(value)\
{\
  uint8 rrtemp = (value); \
  (value) = ( (value)>>1 ) | ( F << 7 );\
  F = ( rrtemp & FLAG_C ) | sz53p_table[(value)];\
}

#define RRC(value)\
{\
  F = (value) & FLAG_C;\
  (value) = ( (value)>>1 ) | ( (value)<<7 );\
  F |= sz53p_table[(value)];\
}

#define RST(value)\
{\
  PUSH16(PCL,PCH);\
  PC=(value);\
}

#define SBC(value)\
{\
  uint16 sbctemp = A - (value) - ( F & FLAG_C ); \
  uint8 lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    ( ( sbctemp & 0x88 ) >> 1 );  \
  A=sbctemp;\
  F = ( sbctemp & 0x100 ? FLAG_C : 0 ) | FLAG_N |\
    halfcarry_sub_table[lookup & 0x07] | overflow_sub_table[lookup >> 4] |\
    sz53_table[A];\
}

#define SBC16(value)\
{\
  uint32 sub16temp = HL - (value) - (F & FLAG_C); \
  uint8 lookup = ( (        HL & 0x8800 ) >> 11 ) | \
			    ( (   (value) & 0x8800 ) >> 10 ) | \
			    ( ( sub16temp & 0x8800 ) >>  9 );  \
  HL = sub16temp;\
  F = ( sub16temp & 0x10000 ? FLAG_C : 0 ) |\
    FLAG_N | overflow_sub_table[lookup >> 4] |\
    ( H & ( FLAG_3 | FLAG_5 | FLAG_S ) ) |\
    halfcarry_sub_table[lookup&0x07] |\
    ( HL ? 0 : FLAG_Z) ;\
}

#define SLA(value)\
{\
  F = (value) >> 7;\
  (value) <<= 1;\
  F |= sz53p_table[(value)];\
}

#define SLL(value)\
{\
  F = (value) >> 7;\
  (value) = ( (value) << 1 ) | 0x01;\
  F |= sz53p_table[(value)];\
}

#define SRA(value)\
{\
  F = (value) & FLAG_C;\
  (value) = ( (value) & 0x80 ) | ( (value) >> 1 );\
  F |= sz53p_table[(value)];\
}

#define SRL(value)\
{\
  F = (value) & FLAG_C;\
  (value) >>= 1;\
  F |= sz53p_table[(value)];\
}

#define SUB(value)\
{\
  uint16 subtemp = A - (value); \
  uint8 lookup = ( (       A & 0x88 ) >> 3 ) | \
			    ( ( (value) & 0x88 ) >> 2 ) | \
			    (  (subtemp & 0x88 ) >> 1 );  \
  A=subtemp;\
  F = ( subtemp & 0x100 ? FLAG_C : 0 ) | FLAG_N |\
    halfcarry_sub_table[lookup & 0x07] | overflow_sub_table[lookup >> 4] |\
    sz53_table[A];\
}

#define XOR(value)\
{\
  A ^= (value);\
  F = sz53p_table[A];\
}

#endif		/* #ifndef FUSE_Z80_MACROS_H */
