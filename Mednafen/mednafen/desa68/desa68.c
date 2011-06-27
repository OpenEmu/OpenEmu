/*
 *                        sc68 - 68000 disassembler
 *         Copyright (C) 2001 Benjamin Gerard <ben@sashipa.com>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 2 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but
 *  WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 */

/* $Id: desa68.c,v 2.3 2003/09/25 01:00:05 benjihan Exp $ */

/* Modified for usage in Mednafen */

/* #include <config68.h> */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef _TEST_DESA68_
#	include <stdio.h>
#	include <string.h>
#	include <stdlib.h>
#endif

#include "desa68/desa68.h"

/**********************************************************
 * Defines my types :                                      *
 * Should be OK without configure, since 32-bit and 16-bit *
 * are not really needed.                                  *
 **********************************************************/
typedef signed char    s8;
typedef unsigned char  u8;
typedef int            s32;
typedef unsigned int   u32;
typedef unsigned short u16;
typedef short          s16;

#define REG0(W)		(((W))&7)
#define REG9(W)		(((W)>>9)&7)
#define OPSZ(W)		(((W)>>6)&3)
#define LINE(W)		(((W)>>12)&15)
#define MODE3(W)	(((W)>>3)&7)
#define MODE6(W)	(((W)>>6)&7)

/*****************************
 * Disassembler string tables *
 *****************************/

enum adressingword
  {
    MODE_DN=0,
    MODE_AN,
    MODE_iAN,
    MODE_ANp,
    MODE_pAN,
    MODE_dAN,
    MODE_dANXI,
    MODE_ABSW,
    MODE_ABSL,
    MODE_dPC,
    MODE_dPCXI,
    MODE_IMM
  };

static char Thex[16] = {
  '0','1','2','3','4','5','6','7',
  '8','9','A','B','C','D','E','F'  };

/* Condition Code String Tables */
static u16 bcc_ascii[] =
  {
    'RA','SR','HI','LS',
    'CC','CS','NE','EQ',
    'VC','VS','PL','MI',
    'GE','LT','GT','LE'
  };

static u16 scc_ascii[] =
  {
    'T','F','HI','LS',
    'CC','CS','NE','EQ',
    'VC','VS','PL','MI',
    'GE','LT','GT','LE'
  };

static u16 dbcc_ascii[] =
  {
    'RA','F','HI','LS',
    'CC','CS','NE','EQ',
    'VC','VS','PL','MI',
    'GE','LT','GT','LE'
  };

DESA68parm_t d;

/* instruction operand size */
/*static char sz_str[4][4] = { ".B", ".W", ".L", ".?" };*/

/*******************
 * String functions *
 *******************/

/* Add a char to disassembly string */
#if 0
static void desa_char(const unsigned char c)
{
  *d.s++ = c;
}
#else
#	define desa_char(C) (*d.s++ = (C))
#endif

/* Add a string to disassembly string */
static void desa_str(char *str)
{
  char c;
  while(c=*str++, c)
    desa_char(c);
}

/* Add a string to disassembly string */
static void desa_ascii(unsigned int n)
{
  int shift;
  for(shift=(sizeof(int)-1)*8; shift>=0; shift-=8)
    {
      u8 c;
      c = (u8)(n>>shift);
      if(c) desa_char(c);
    }
}

#if 0
/* Count signifiant hexa digit */
static int nuhexadigit(unsigned int n)
{
  int ct;
  for(ct=1, n>>=4; n; ct++,  n>>=4);
  return ct;
}

/* Count significatif hexa digit */
static int nhexadigit(int n)
{
  return nuhexadigit(n<0?-n:n);
}
#endif

/* Add a N-ndigit digit unsigned hexa number with header char to disassembly string */
static void desa_uhexacat(unsigned int n, int ndigit, int header_char)
{
  int shf;
  desa_char(header_char);
  for(shf=(ndigit-1)*4; shf>=0; shf-=4) {
    desa_char(Thex[(n>>shf)&15] );
  }
}

#if 0
/* Add a N-ndigit digit signed hexa number with heading '$' to disassembly string */
void desa_hexacat(int n, int ndigit)
{
  if(n<0)
    {
      desa_char('-' );
      n = -n;
    }
  desa_uhexacat(n, ndigit, '$');
}
#endif

/* Add a signifiant digit only unsigned hexa number
 * with heading '$' to disassembly string
 */
static void desa_usignifiant(unsigned int n)
{
  int shf;
  desa_char('$');
  for(shf=(sizeof(int)*2-1)*4; shf>=0 && !(n>>shf); shf-=4);
  if(shf<0) shf=0;
  for(; shf>=0; shf-=4)
    desa_char(Thex[(n>>shf)&15] );
}

/* idem desa_usignifiant, except it is signed */
static void desa_signifiant(int n)
{
  if(n<0)
    {
      desa_char('-');
      n = -n;
    }
  desa_usignifiant(n);
}

static int desa_isascii(u8 c)
{
  return c=='_' || c==' '
    || (c>='a' && c<='z')
    || (c>='A' && c<='Z')
    || (c>='0' && c<='9');
}

/*******************************
 * General disassembly function *
 *******************************/

/* Get addressing mode number at bit #6 [0..14], only [0..11] are valid */
/*static u32 getmode_6(u32 w)
  {
  u32 mode;
  mode  = ((w>>6)&7);
  if(mode==7)
  mode += (w>>9)&7;
  return mode;
  }*/

/* Read next word , increment pc */
static int read_pc(void)
{
  unsigned int pc = d.pc;

/*
  d.w = (s8)d.mem[pc++&d.memmsk]<<8;
  d.w  += d.mem[pc++&d.memmsk];
*/

  d.w = d.mem_callb(pc & d.memmsk, d.private_data);
  pc += 2;

  d.pc = pc;
  return d.w;
}

static s32 immB(void)
{
  return (s32)(s8)read_pc();
}

#if 0
static s32 immW(void)
{
  read_pc();
  return d.w;
}
#else
# define immW read_pc
#endif

static s32 immL(void)
{
  return (read_pc()<<16) | (read_pc()&0xffff );
}

static s32 adrL(void)
{
  return immL()/* & d.memmsk*/ ;  /* Problem with hardware reg ! */
}

#if 0
static s32 adrW(void)
{
  read_pc();
  return d.w;
}
#else
#	define adrW immW
#endif

static s32 relPC(void)
{
  read_pc();
  return (d.pc + d.w - 2) & d.memmsk;
}

/* return [AD][0-7][d][WL] : SZ='W'/'L' XI = hexa value for reg ($D0) */
static s32 indAnXi()
{
  s32 v;
  read_pc();
  v  = (d.w&0x8000)? ('A'<<24) : ('D'<<24);
  v |= ('0'+((d.w>>12)&7)) << 16;
  v |= (u8)d.w<<8;
  v |= ((d.w&(1<<11))? 'L' : 'W');
  return v;
}

static void desa_dcw(void)
{
  desa_str("DC.W");
  desa_char (' ');
  desa_uhexacat((u16)d.w, 4, '$');
  d.status = 0;
}

static void update_ea(unsigned int v)
{
  v &= 0xFFFFFF;
  if (d.ea_src == -1) {
    d.ea_src = v;
  } else if (d.ea_dst == -1) {
    d.ea_dst = v;
  }
}

static void desa_label(unsigned int v)
{
  desa_uhexacat(v, 6, 'L');
}

static void desa_immL(int v)
{
  unsigned int v2 = v;
  desa_char('#');
  if ((d.flags&DESA68_SYMBOL_FLAG) && v2>=d.immsym_min && v2<d.immsym_max) {
    desa_label(v2);
    update_ea(v2);
  } else {
    if(desa_isascii(v2) && desa_isascii(v2>>8) && desa_isascii(v2>>16) && desa_isascii(v2>>24)) {
      desa_char ('\'');
      desa_ascii((u32)v2);
      desa_char ('\'');
    } else {
      desa_signifiant(v);
    }
  }
}

static void desa_absL(int v)
{
  unsigned int v2 = v & 0xFFFFFF;

  if ((d.flags&DESA68_SYMBOL_FLAG) && v2>=d.immsym_min && v2<d.immsym_max) {
    desa_uhexacat(v2, 6, 'L');
  } else {
    desa_usignifiant(v);
    desa_str(".L");
  }
}

static void get_ea_2(u32 mode, u32 reg, u8 sz)
{
  s32 v;

  if(mode==MODE_ABSW) {
    mode += reg;
  }

  switch(mode )
    {
    case MODE_DN:
      desa_ascii('D0'+reg);
      break;
    case MODE_AN:
      desa_ascii('A0'+reg);
      break;
    case MODE_iAN:
      desa_ascii('(A0)'+(reg<<8));
      break;
    case MODE_pAN:
      desa_char('-');
      desa_ascii('(A0)'+(reg<<8));
      break;
    case MODE_ANp:
      desa_ascii('(A0)'+(reg<<8));
      desa_char('+');
      break;
    case MODE_dAN:
      read_pc();
      desa_signifiant(d.w);
      desa_ascii('(A0)'+(reg<<8));
      break;
    case MODE_dANXI:
      v = indAnXi();
      d.ea = (s8)(v>>8);
      desa_signifiant((s8)(v>>8));
      desa_ascii('(A0,'+(reg<<8));
      v = (v&0xFFFF00FF)+('.'<<8);
      desa_ascii(v);
      desa_char(')');
      break;
    case MODE_ABSW:
      d.ea = v = adrW();
      desa_usignifiant(v);
      desa_ascii('.W');
      break;
    case MODE_ABSL:
      d.ea = v = adrL();
      update_ea(v);
      desa_absL(v);
      break;
    case MODE_dPC:
      d.ea = v = relPC();
      update_ea(v);
      if (d.flags & DESA68_SYMBOL_FLAG) {
	desa_label(v);
      } else {
	desa_usignifiant(v);
      }
      desa_str("(PC)");
      break;
    case MODE_dPCXI:
      v = indAnXi();
      d.ea = d.pc-2+(s8)(v>>8);
      update_ea(d.ea);
      if (d.flags & DESA68_SYMBOL_FLAG) {
	desa_label(d.ea);
      } else {
	desa_usignifiant(d.ea);
      }
      desa_str("(PC,");
      v = (v&0xFFFF00FF)+('.'<<8);
      desa_ascii(v);
      desa_char(')');
      break;
    case MODE_IMM:
      switch(sz )
	{
	case 1:
	case 'B':
	  v = (s8)immB();
	  desa_immL(v);
	  break;
	case 2:
	case 'W':
	  v = (s16)immW();
	  desa_immL(v);
	  break;
	case 4:
	case 'L':
	  v = (u32)immL();
	  desa_immL(v);
	  return;
	default:
	  desa_str("#");
	  /*BREAKPOINT68;*/
	  break;
	}
      break;
    default:
      desa_char('?');
      desa_usignifiant(mode);
      desa_char('?');
      break;
    }
}

/* Get move operand
 * bit : 0/6 -> src/dest
 * w   : current opcode
 */
static void get_ea_move(int bit, s32 w, u32 easz)
{
  u32 ea    = (w>>bit)&63;

  if(bit)
    get_ea_2(ea&7,ea>>3, easz);
  else if(bit==0)
    get_ea_2(ea>>3,ea&7, easz);
}

static void desa_reg(int reg)
{
  desa_ascii(((reg&8)? 'A0':'D0') + (reg&7));
}

/* Used with ABCD, SBCD, ADDX, SUBX */
static void desa_ry_rx(int inst, int size)
{
  desa_ascii(inst);
  desa_ascii(size);
  desa_char(' ');
  if(d.mode3&1)	/* -(Ay),-(Ax) */
    {
      desa_ascii('-(A0'+d.reg0);
      desa_ascii('),-(');
      desa_ascii('A0)' + (d.reg9<<8));
    }
  else				/* Dy,Dx */
    {
      desa_reg(d.reg0);
      desa_char(',');
      desa_reg(d.reg9);
    }
}

static void desa_dn_ae(int name)
{
  desa_ascii(name);
  desa_ascii(d.szchar);
  desa_char (' ');
  /*  dn,<ae> */
  if(d.w&0400)
    {
      desa_ascii('D0,' + (d.reg9<<8));
      get_ea_2(d.mode3, d.reg0, d.szchar);
    }

  /*  <ae>,dn */
  else
    {
      get_ea_2(d.mode3, d.reg0, d.szchar);
      desa_ascii(',D0'+d.reg9);
    }
}

/**************
 *
 *   LINE 0 :
 *   -Immediat operations
 *   -SR & CCR operations
 *   -Bit operations
 *   -Movep
 *
 ***************/

static int check_desa_bitop(void)
{
  static u32 fn[] = { 'BTST', 'BCHG', 'BCLR', 'BSET'};
  int modemsk = 00775;
  unsigned int inst;

  if(!(modemsk&(1<<d.adrmode0)))
    return 0;

  inst = fn[d.opsz];
  /* B... #x,<AE>*/
  if(!(d.w&0400))
    {
      if((d.w&0xF00)!=0x800)
	return 0;
      desa_ascii(inst);
      desa_ascii(' #');
      read_pc();
      desa_usignifiant((u8)d.w);
    }
  else
    /* B... dn,<AE>*/
    {
      desa_ascii(inst);
      desa_ascii(' D0'+d.reg9);
    }
  desa_char(',');
  get_ea_2(d.mode3, d.reg0, 'B');

  return 1;
}

static int check_desa_IMM_SR(void)
{
  u32 modemsk = 1 + (1<<2) + (1<<10);
  int mode = (d.w>>8)&15, inst='ORI';

  if((d.w&0677)!=0074 || !(modemsk&(1<<mode)))
    return 0;

  switch(mode)
    {
    case 0xA: /* EORI */
      inst = 'EORI';
      break;
#if 0
    case 0x0: /* ORI */
      inst = 'ORI';
      break;
#endif
    case 0x2: /* ANDI */
      inst = 'ANDI';
      break;
    }
  desa_ascii(inst);
  desa_char(' ');
  get_ea_2(MODE_ABSW, MODE_IMM-MODE_ABSW, 'W');
  desa_ascii((d.mode6&1) ? ',SR' : ',CCR');
  return 1;
}

static int check_desa_movep(void)
{
  if((d.w & 0470) != 0410 )
    return 0;

  desa_str("MOVEP.");
  desa_ascii((d.opsz&1)?'L ' : 'W ');

  /* MOVEP.? d(Ax),Dy */
  if(!(d.w&(1<<7)))
    {
      get_ea_2(MODE_dAN, d.reg0, 0 );
      desa_ascii(',D0' + d.reg9 );
    }
  /* MOVEP.? Dx,d(Ay) */
  else
    {
      desa_ascii('D0,' + (d.reg9<<8) );
      get_ea_2(MODE_dAN, d.reg0, 0 );
    }
  return 1;
}

static int check_desa_imm_op(void)
{
  static u32 fn[8] =
    {
      'ORI' , 'ANDI', 'SUBI', 'ADDI',
      '???I', 'EORI', 'CMPI', '???I'
    };
  int modemsk=0x6F;

  if((d.w&0x100) || !(modemsk&(1<<d.reg9)) || d.opsz==3)
    return 0;

  desa_ascii(fn[d.reg9] );
  desa_ascii((d.szchar<<8) + ' ');
  get_ea_2(MODE_ABSW, MODE_IMM-MODE_ABSW, d.szchar);
  desa_char(',');
  get_ea_2(d.mode3, d.reg0, d.szchar);
  return 1;
}

static void desa_line0(void)
{
  if(check_desa_movep() ) 	return;
  if(check_desa_bitop() ) 	return;
  if(check_desa_IMM_SR() ) 	return;
  if(check_desa_imm_op() )	return;
  else desa_dcw();
}

/* General move(a) disassemble
 */
static void desa_move()
{
  s32 w=d.w;
  int movsz  = (u8)('WLB?' >> ((w&(3<<12))>>(12-3)));
  desa_ascii('MOVE');
  desa_ascii(((d.adrmode6==MODE_AN)? ('A'<<24) : 0) + ('.'<<16) + ' ' + (movsz<<8) );
  get_ea_move(0,w,movsz);
  desa_char(',');
  get_ea_move(6,w,movsz);
}

/* Check and disassemble a valid move
 * return TRUE if valid move
 */
static int check_desa_move()
{
  u32 srcmsk   = 0xFFF;
  u32 destmsk  = 0x1FF;
  /* Remove An source & destination addressing mode for byte access */
  if(d.line==0x1)
    {
      srcmsk  &= ~(1<<MODE_AN);
      destmsk &= ~(1<<MODE_AN);
    }
  if((srcmsk&(1<<d.adrmode0 )) && (destmsk&(1<<d.adrmode6)) );
  {
    desa_move();
    return 1;
  }
  return 0;

}

/**************
 *
 *   LINE 1 :
 *   -MOVE.B
 *   LINE 2 :
 *   -MOVE.L
 *   LINE 3 :
 *   -MOVE.W
 *
 ***************/

static void desa_li123(void)
{
  if(!check_desa_move())
    desa_dcw();
}

/**************
 *
 *   LINE 4 :
 *   -Other instructions
 *
 ***************/

static void get_movemsub(s32 i, s32 j)
{
  desa_reg(i);
  if(i!=j)
    {
      desa_char('-');
      desa_reg(j);
    }
}

static void get_movemreg(u32 v, u32 rev)
{
  s32 i,j,p=0;
  for(i=0 ; i<16; i++)
    {
      for(; i<16 && (v&(1<<(i^rev)))==0; i++);
      if(i==16) break;
      j = i;
      for(; i<16 && (v&(1<<(i^rev))); i++);
      if(p) desa_char('/');
      get_movemsub(j,(i-1));
      p = 1;
    }
}

static int desa_check_imp(unsigned int name, int mskmode)
{
  if((d.w&0400) || !(mskmode&(1<<d.adrmode0)))
    return 0;
  desa_ascii(name);
  desa_ascii(d.szchar);
  desa_char (' ');
  get_ea_2(d.mode3, d.reg0, d.szchar);
  return 1;
}

static int check_desa_lea()
{
  int modemsk = 03744;
  if((d.w&0700) != 0700 || !(modemsk&(1<<d.adrmode0)))
    return 0;
  desa_ascii('LEA ');
  get_ea_2(d.mode3, d.reg0, 0);
  desa_ascii(',A0'+d.reg9);
  return 1;
}

static int check_desa_chk()
{
  int modemsk = 07775;
  if((d.w&0700) != 0600 || !(modemsk&(1<<d.adrmode0)))
    return 0;
  desa_ascii('CHK ');
  get_ea_2(d.mode3, d.reg0, 0);
  desa_ascii(',D0'+d.reg9);
  d.status = DESA68_INST | DESA68_INT;
  d.branch = 0x18;
  return 1;
}

static int check_desa_ext()
{
  if((d.w&07670)!=04200)
    return 0;
  desa_ascii('EXT.' );
  desa_ascii((!(d.w&0100)? 'W D0':'L D0') + d.reg0);
  return 1;
}

static int check_desa_movem()
{
  int modemsk, regmsk;
  int mode3, reg0, w;
  if((d.w&05600)!=04200)
    return 0;
  modemsk = !(d.w&02000) ? 00764 : 03754;
  if(!(modemsk&(1<<d.adrmode0)))
    return 0;

  desa_ascii('MOVE');
  desa_ascii('M.');
  desa_char (!(d.w&0100)?'W':'L');
  desa_char (' ');

  w = d.w;
  mode3 = d.mode3;
  reg0 = d.reg0;
  regmsk = immW();

  /*  -> reg */
  if(w&02000)
    {
      get_ea_2(mode3, reg0, 0);
      desa_char(',');
      get_movemreg(regmsk,0);
    }
  /* -> mem */
  else
    {
      get_movemreg(regmsk, (mode3==MODE_pAN) ? 15 : 0);
      desa_char(',');
      get_ea_2(mode3, reg0, 0);
    }
  return 1;
}

static int check_desa_jmp()
{
  int modemsk = 03744;
  if((d.w&07600) != 07200) {
    return 0;
  }
  /* JMP / JSR */
  if(modemsk & (1<<d.adrmode0)) {
    int s = 'MP ', stat = DESA68_INST | DESA68_BRA;
    desa_char('J');
    if (d.opsz==2) {
      stat = DESA68_INST | DESA68_BRA | DESA68_BSR;
      s = 'SR ';
    }
    d.status = stat;
    desa_ascii(s);
    get_ea_2(d.mode3, d.reg0, d.szchar);
    d.branch = d.ea;

    return 1;
  }
  /* invalid JSR / JMP address mode */
  /* $$$ Not sure for a DC.W here */
  desa_dcw();
  return 1;
  /*return 0;*/
}

static int check_desa_line4_mode3(void)
{
  if (d.mode6 != 3) {
    return 0;
  }

  switch(d.reg9)
    {

    case 0:				/* MOVE FROM SR */
      {
	int modemsk = 00775;
	if(!(modemsk&(1<<d.adrmode0)))
	  break;
	desa_ascii('MOVE');
	desa_ascii(' SR,');
	get_ea_2(d.mode3, d.reg0, 'W');
	return 1;
      }

    case 1:
      break;

    case 2:			/* MOVE TO CCR */
      {
	int modemsk = 07775;
	if(!(modemsk&(1<<d.adrmode0)))
	  break;
	desa_ascii('MOVE');
	desa_char (' ');
	get_ea_2(d.mode3, d.reg0, 'B');
	desa_ascii(',CCR');
	return 1;
      }

    case 3:			/* MOVE TO SR */
      {
	int modemsk = 07775;
	if(!(modemsk&(1<<d.adrmode0)))
	  break;
	desa_ascii('MOVE');
	desa_char (' ');
	get_ea_2(d.mode3, d.reg0, 'W');
	desa_ascii(',SR');
	return 1;
      }

    case 4:
      break;

    case 5:			/* TAS */
      {
	d.szchar = 0;
	if(desa_check_imp('TAS', 00775)) {
	  return 1;
	} else if(d.w == 0x4AFC) {
	  desa_ascii('ILLE');
	  desa_ascii('GAL');
	  d.status = DESA68_INST | DESA68_INT;
	  d.branch = 0x10;
	  return 1;
	}
      }
      break;

    case 6:
      break;

    case 7:
      break;
    }
  return 0;
}


static void desa_line4(void)
{
  switch (d.mode6) {

  case 7:
    if (check_desa_lea()) return;
    break;
  case 5:
    if (check_desa_chk()) return;
    break;
  case 2:
  case 3:
    if (check_desa_ext()) return;
    else if (check_desa_movem()) return;
    else if (check_desa_jmp()) return;
    if (check_desa_line4_mode3()) return;

  default:

    switch(d.reg9)
      {
      case 0:				/* NEGX */
	if(desa_check_imp('NEGX', 00775))
	  return;
	break;

      case 1:				/* CLR */
	if(desa_check_imp('CLR', 00775))
	  return;
	break;

      case 2:				/* NEG */
	if(desa_check_imp('NEG', 00775))
	  return;
	break;

      case 3:				/* NOT */
	if(desa_check_imp('NOT', 00775))
	  return;
	break;

      case 4:
	if(d.mode6==0)				/* NBCD */
	  {
	    d.szchar = 0;
	    if(desa_check_imp('NBCD', 00775))
	      return;
	  }
	else if(d.mode6==1)
	  {
	    if(d.mode3 == MODE_DN)	/* SWAP */
	      {
		desa_ascii('SWAP');
		desa_ascii(' D0'+d.reg0);
		return;
	      }
	    else					/* PEA */
	      {
		d.szchar = 0;
		if(desa_check_imp('PEA', 0x7E4))
		  return;
	      }
	  }
	break;

      case 5:				/* TST */
	if(desa_check_imp('TST', 00775))
	  return;
	break;

      case 6:
	break;

      case 7:
	if(d.mode6 == 1)	/* FUNKY LINE 4 */
	  {
	    switch(d.mode3)
	      {
	      case 0:
	      case 1:
		{
		  int num = d.w&0xF;
		  desa_ascii('TRAP');
		  desa_ascii(' #');
		  desa_char (Thex[num]);
		  d.status = DESA68_INST | DESA68_INT;
		  d.branch = 0x80 + (num<<2);
		} return;
	      case 2:
		desa_ascii('LINK');
		desa_ascii(' A0,'+(d.reg0<<8));
		get_ea_2(MODE_ABSW, MODE_IMM-MODE_ABSW, 'W');
		return;
	      case 3:
		desa_ascii('UNLK');
		desa_ascii(' A0'+d.reg0);
		return;
	      case 4:
		desa_ascii('MOVE');
		desa_ascii(' A0'+d.reg0);
		desa_ascii(',USP');
		return;
	      case 5:
		desa_ascii('MOVE');
		desa_ascii(' USP');
		desa_ascii(',A0'+d.reg0);
		return;
	      case 6:	/* FUNKY LINE 4 MODE 6 (4E */
		{
		  static char *str[8] =	/* $4E70 - $4E77 */
		    {
		      /* 0      1      2      3     4    5      6      7 */
		      "RESET","NOP","STOP ","RTE", "?","RTS","TRAPV","RTR"
		    };

		  if (d.reg0 == 4) {
		    break;
		  }
		  if ((d.reg0 & 1)){
		    d.status = DESA68_INST | ( (d.reg0 == 1) ? DESA68_NOP : DESA68_RTS);
		  }

		  desa_str(str[d.reg0]);
		  if (d.reg0 == 2) {
		    get_ea_2(MODE_IMM,0,'W');
		  }
		} return;
	      }
	  }
      }
  }

  /*	desa_ascii('r9:');
	desa_ascii(' 0 ' + d.reg9*256);
	desa_ascii('m6:');
	desa_ascii(' 0 ' + d.mode6*256);
	desa_ascii('m3:');
	desa_ascii(' 0 ' + d.mode3*256);
	desa_ascii('r0:');
	desa_ascii(' 0 ' + d.reg0*256);
  */
  desa_dcw();
}


/**************
 *
 *   LINE 5 :
 *   -ADDQ
 *   -SUBQ
 *   -Scc
 *   -Dcc
 *
 ***************/

static void desa_addq_subq()
{
  int v;
  v = d.reg9 * 256;
  if(v==0) v=8*256;
  desa_ascii((d.w&0400) ? 'SUBQ':'ADDQ');
  desa_ascii(d.szchar);
  desa_ascii(' #0,' + v);
  get_ea_2(d.mode3, d.reg0, d.szchar);
}

static void desa_Dcc()
{
  desa_ascii(('DB'<<16) + dbcc_ascii[(d.w>>8)&15]);
  desa_ascii(' D0,' + (d.reg0<<8));
  get_ea_2(MODE_ABSW, MODE_dPC-MODE_ABSW, 0);
  /* $$$ hack : remove (PC) at the end */
  d.s[-4] = 0;
  d.branch = d.ea;
  d.status = DESA68_INST | DESA68_BRA | DESA68_BSR;
}

static void desa_Scc()
{
  desa_ascii(('S'<<24) + ' ' + scc_ascii[(d.w>>8)&15]*256);
  get_ea_2(d.mode3, d.reg0, d.szchar);
}

static void desa_line5(void)
{
  if((d.w&0370)==0310) {
    desa_Dcc();
  } else if(d.opsz==3) {
    desa_Scc();
  } else {
    desa_addq_subq();
  }
}

/**************
 *
 *   LINE 6 :
 *   -Bcc
 *
 * Format 0110 COND OFFSET08 [OFFSET16 if OFFSET08==0]  
 * !!! COND=0001(false) => BSR
 ***************/

static void desa_line6(void)
{
  s32 a;
  int cond = (d.w>>8) & 0xF;
  desa_ascii(('B'<<16) + bcc_ascii[cond]);
  /* Bcc.S */
  if(d.w&255)
    {
      desa_ascii('.S');
      a = (s32)(s8)d.w;
      a += (s32)d.pc;
    }
  /* Bcc.W */
  else a = relPC();
  desa_char(' ');

  desa_absL(a&d.memmsk);

  d.status = DESA68_INST | DESA68_BRA | (-(cond!=0) & DESA68_BSR );
  d.branch = a;
}

/**************
 *
 *   LINE 7 :
 *   -MOVEQ
 *
 * Format : 01111 REG 0 XXXXXXXX
 *
 ***************/
static void desa_line7(void)
{
  if(d.w&0400) {
    desa_dcw();
  } else {
    desa_str("MOVEQ #");
    desa_signifiant((int)(s8)d.w);
    desa_ascii(',D0'+d.reg9);
  }
}


/**************
 *
 *   LINE B :
 *   -CMP
 *   -CMPM
 *   -EOR
 *
 ***************/

static int check_desa_cmpa()
{
  int modemsk = 07777;
  if(d.opsz!=3 || !(modemsk&(1<<d.adrmode0)))
    return 0;

  d.szchar=(d.w&0400) ? '.L' : '.W';
  desa_ascii('CMPA');
  desa_ascii(d.szchar);
  desa_char (' ');
  get_ea_2  (d.mode3, d.reg0, d.szchar);
  desa_ascii(',A0'+d.reg9);
  return 1;
}

static int check_desa_eor_cmp()
{
  int modemsk, inst;

  if(d.opsz==3)
    return 0;

  /* EOR */
  else if(d.w&0400)
    {
      modemsk = 00775;
      inst = 'EOR';
    }

  /* CMP */
  else
    {
      modemsk = (!d.opsz) ? 07775 : 07777;
      inst = 'CMP';
    }

  if(!(modemsk&(1<<d.adrmode0)))
    return 0;

  desa_dn_ae(inst);
  return 1;
}

static int check_desa_cmpm()
{
  if((d.w&0470)!=0410/* || (d.w&0300)==0300 */)
    return 0;
  desa_ascii('CMPM');
  desa_ascii(' (A0' + d.reg0);
  desa_ascii(')+,(');
  desa_ascii('A0)+' + (d.reg9<<16));
  return 1;
}

static void desa_lineB(void)
{
  if(check_desa_cmpa())		return;
  else if(check_desa_eor_cmp())	return;
  else if(check_desa_cmpm())		return;
  else desa_dcw();
  return;
}


/**************
 *
 *   LINE 8 :
 *   -OR
 *   -SBCD
 *   -DIVU
 *
 *
 *   LINE C :
 *   -EXG
 *   -MULS,MULU
 *   -ABCD
 *   -AND
 *
 ***************/

static int check_desa_exg()
{
  unsigned int reg;
  switch(d.w&0770)
    {
    case 0500:
      reg = 0x0000;
      break;
    case 0510:
      reg = 0x0808;
      break;
    case 0610:
      reg = 0x0008;
      break;
    default:
      return 0;
    }
  desa_ascii('EXG ');
  desa_reg(d.reg9 + (reg>>8));
  desa_char (',');
  desa_reg(d.reg0 + (reg&8));
  return 1;
}

static int check_desa_mul_div(void)
{
  int modemsk = 0xFFD;
  if((d.w&0300)!=0300 || !(modemsk&(1<<d.adrmode0)))
    return 0;
  desa_ascii((d.line==0xC) ? 'MUL' : 'DIV');
  desa_ascii((d.w&0x100) ? 'S ' : 'U ');
  get_ea_2  (d.mode3, d.reg0, 'W');
  desa_ascii(',D0'+d.reg9);
  return 1;
}

static int check_desa_abcd_sbcd(void)
{
  if((d.w&0x1f0)!=0x100)
    return 0;
  desa_ry_rx((d.line==0xC) ? 'ABCD' : 'SBCD', 0) ;
  return 1;
}


static int check_desa_and_or(void)
{
  int modemsk = !(d.w&0400)? 0xFFD : 0x1FC;
  if(!(modemsk&(1<<d.adrmode0)))
    return 0;
  desa_dn_ae((d.line==0xC)? 'AND' : 'OR');
  return 1;
}

static void desa_lin8C(void)
{
  if(check_desa_abcd_sbcd())	return;
  else if(check_desa_mul_div())		return;
  else if(check_desa_exg())			return;
  else if(check_desa_and_or())		return;
  else desa_dcw();
  return;
}

/**************
 *
 *   LINE 9 :
 *   -SUB, SUBX, SUBA
 *
 *   LINE D :
 *   -ADD, ADDX, ADDA
 *
 **************/

static int check_desa_addx_subx()
{
  if((d.w&0460)!=0400/* || (d.w&300==0300)*/)
    return 0;
  desa_ry_rx((d.line==0xD) ? 'ADDX' : 'SUBX', d.szchar);
  return 1;
}

static int check_desa_adda_suba(void)
{
  if(d.opsz!=3)
    return 0;

  d.szchar = (d.w&0400) ? '.L' : '.W';
  desa_ascii((d.line==0xD) ? 'ADDA' : 'SUBA');
  desa_ascii(d.szchar);
  desa_char (' ');
  get_ea_2(d.mode3, d.reg0, d.szchar);
  desa_ascii(',A0'+d.reg9);
  return 1;
}

static int check_desa_add_sub(void)
{
  int modemsk = !(d.w&0400) ? 07777 : 00774;
  if(d.opsz==0)	/* Fordib An for byte access size */
    modemsk &= ~(1<<MODE_AN);
  if(!(modemsk & (1<<d.adrmode0)))
    return 0;
  desa_dn_ae (d.line==0xD ? 'ADD' : 'SUB');
  return 1;
}

static void desa_lin9D(void)
{
  if(check_desa_adda_suba())	return;
  else if(check_desa_addx_subx())	return;
  else if(check_desa_add_sub())		return;
  else desa_dcw();
  return;
}

/**************
 *
 *   LINE E :
 *   -Shifting
 *
 * Format Reg: 1110 VAL D SZ I TY RG0
 * Format Mem: 1110 0TY D 11 MODRG0
 ***************/

static void desa_lineE(void)
{
  static u16 shf_ascii[4] = { 'AS', 'LS', 'RO', 'RO' };

  /* Memory */
  if(d.opsz==3)
    {
      const int modemsk = 00774;
      int type = d.reg9;
      if (!(modemsk & (1<<d.adrmode0)) || (type&4)) {
	desa_dcw();
	return;
      }
      desa_ascii(shf_ascii[type]);
      desa_char ((d.w&0400) ? 'L' :'R');
      if(type==2) desa_char('X');
      desa_char (' ');
      get_ea_2(d.mode3, d.reg0, 0);
    }

  /* Register */
  else
    {
      int type = (d.w>>3)&3;
      desa_ascii (shf_ascii[type]);
      desa_char ((d.w&0400) ? 'L' :'R');
      if(type==2) desa_char('X');
      desa_ascii(d.szchar);
      /* dn,dn */
      if(d.w&(1<<5))
	desa_ascii(' D0'+d.reg9);
      /* #x,dn */
      else
	desa_ascii(' #1'+((d.reg9-1)&7));
      desa_ascii(',D0'+d.reg0);
    }
}

/**************
 *
 *   LINE A :
 *   -LineA
 *   LINE F :
 *   -LineF
 *
 * Format : LINE XXXX XXXX XXXX
 *
 ***************/

static void desa_linAF(void)
{
  desa_ascii('LINE');
  desa_ascii('A '+((d.line-0xA)<<8));
  desa_uhexacat(d.w, 3, '$');
}

static void (*desa_line[16])(void) =
{
  desa_line0, desa_li123, desa_li123, desa_li123,
  desa_line4, desa_line5, desa_line6, desa_line7,
  desa_lin8C, desa_lin9D, desa_linAF, desa_lineB,
  desa_lin8C, desa_lin9D, desa_lineE, desa_linAF,
};

void desa68(DESA68parm_t *dparm)
{
  char tmp[64];
  dparm->ea = dparm->branch = d.memmsk+1;
  dparm->status = DESA68_INST; /* Assume valid instruction */
  d = *dparm;
  d.s = d.str;
  if (!d.s) {
    d.s = tmp;
    d.strmax = sizeof(tmp);
  }
  d.s[0]=0;
  read_pc();

  d.reg0 = REG0(d.w);
  d.reg9 = REG9(d.w);
  d.mode3 = MODE3(d.w);
  d.mode6 = MODE6(d.w);
  d.line  = LINE(d.w);
  d.opsz  = OPSZ(d.w);
  d.adrmode0 = d.mode3 + ((d.mode3==MODE_ABSW)?d.reg0 : 0);
  d.adrmode6 = d.mode6 + ((d.mode6==MODE_ABSW)?d.reg9 : 0);
  d.szchar = ('.'<<8)|(u8)('?LWB'>>(d.opsz*8));

  d.ea_src = d.ea_dst = -1;

  (desa_line[d.line])();
  desa_char(0);
  /* Restore current status to caller struct */
  dparm->pc = d.pc;
  dparm->status = d.status;
  dparm->branch = d.branch;
  dparm->ea_src = d.ea_src;
  dparm->ea_dst = d.ea_dst;
}

#ifdef _TEST_DESA68_

static void message(void )
{
  printf("Desa68 -- Built-in test\n(c) BeN / SasHipA - Dev\n");
}


int main(int na, char **a)
{
  FILE *f;
  u8 *b;
  int l,i,nline=50;
  static char s[256];

  DESA68parm_t d;

  message();

  if(na<2)
    {
      perror("No input file\n");
      return 1;
    }

  if(f=fopen(a[1],"rb"), f==NULL)
    {
      perror("Can't open input file\n");
      return 2;
    }

  fseek(f,0L,SEEK_END);
  l = (int)ftell(f);
  fseek(f,0L,SEEK_SET);
  if(b=malloc(l), b==NULL)
    {
      fclose(f);
      perror("Can't alloc memory\n");
      return 3;
    }

  if(fread(b,l,1,f)!=1)
    {
      fclose(f);
      free(b);
      perror("Read error\n");
      return 4;
    }

  if (na > 2) {
    int n = atoi (a[2]);
    if (n > 0) {
      nline = n;
    }
  }

  fclose(f);



  {
#if 0
    char b[1024];
    /*
      b[0] = 0x4e;
      b[1] = 0xf6;
      b[2] = 0x12;
      b[3] = 0x32;
    */
    /*
      b[0] = 0x4a;
      b[1] = 0x87;
      b[2] = 0x12;
      b[3] = 0x32;
    */
    b[0] = 0x08;
    b[1] = 0xb9;
    b[2] = 0x00;
    b[3] = 0x05;
    b[4] = 0x00;
    b[5] = 0xff;
    b[6] = 0xfa;
    b[7] = 0x11;
#endif

    d.mem = b;
    d.memmsk = 0xFFFFFF;
    d.pc = 0;
    d.str = s;

    for(i=0; i<nline; i++)
      {
	printf("%06X  ",d.pc);
	desa68(&d);
	printf("%s\n",d.str);
      }

    //free(b);
  }
  return 0;
}

#endif
