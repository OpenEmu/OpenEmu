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
 * ppc_ops.c
 *
 * PowerPC common opcodes. Included from ppc.cpp; do not compile compile 
 * separately.
 * 
 * Changes to opcode handlers since inclusion in new Supermodel:
 *		- Feb. 13 2011: Changed stwcx. to always set the EQ flag.
 */
 
/* PowerPC common opcodes */

// it really seems like this should be elsewhere - like maybe the floating point checks can hang out someplace else
#include <math.h>

static void ppc_unimplemented(UINT32 op)
{
	ErrorLog("PowerPC hit an unimplemented instruction. Halting emulation until reset.");
	DebugLog("PowerPC encountered an unimplemented opcode %08X at %08X\n", op, ppc.pc);
	ppc.fatalError = true;
}

static void ppc_addx(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 rb = REG(RB);

	REG(RT) = ra + rb;

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, rb);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_addcx(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 rb = REG(RB);

	REG(RT) = ra + rb;

	SET_ADD_CA(REG(RT), ra, rb);

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, rb);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_addex(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 rb = REG(RB);
	UINT32 carry = (XER >> 29) & 0x1;
	UINT32 tmp;

	tmp = rb + carry;
	REG(RT) = ra + tmp;

	if( ADD_CA(tmp, rb, carry) || ADD_CA(REG(RT), ra, tmp) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, rb);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_addi(UINT32 op)
{
	UINT32 i = SIMM16;
	UINT32 a = RA;

	if( a )
		i += REG(a);

	REG(RT) = i;
}

static void ppc_addic(UINT32 op)
{
	UINT32 i = SIMM16;
	UINT32 ra = REG(RA);

	REG(RT) = ra + i;

	if( ADD_CA(REG(RT), ra, i) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;
}

static void ppc_addic_rc(UINT32 op)
{
	UINT32 i = SIMM16;
	UINT32 ra = REG(RA);

	REG(RT) = ra + i;

	if( ADD_CA(REG(RT), ra, i) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;

	SET_CR0(REG(RT));
}

static void ppc_addis(UINT32 op)
{
	UINT32 i = UIMM16 << 16;
	UINT32 a = RA;

	if( a )
		i += REG(a);

	REG(RT) = i;
}

static void ppc_addmex(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 carry = (XER >> 29) & 0x1;
	UINT32 tmp;

	tmp = ra + carry;
	REG(RT) = tmp + -1;

	if( ADD_CA(tmp, ra, carry) || ADD_CA(REG(RT), tmp, -1) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, carry - 1);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_addzex(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 carry = (XER >> 29) & 0x1;

	REG(RT) = ra + carry;

	if( ADD_CA(REG(RT), ra, carry) )
		XER |= XER_CA;
	else
		XER &= ~XER_CA;

	if( OEBIT ) {
		SET_ADD_OV(REG(RT), ra, carry);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_andx(UINT32 op)
{
	REG(RA) = REG(RS) & REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_andcx(UINT32 op)
{
	REG(RA) = REG(RS) & ~REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_andi_rc(UINT32 op)
{
	UINT32 i = UIMM16;

	REG(RA) = REG(RS) & i;

	SET_CR0(REG(RA));
}

static void ppc_andis_rc(UINT32 op)
{
	UINT32 i = UIMM16 << 16;

	REG(RA) = REG(RS) & i;

	SET_CR0(REG(RA));
}

static void ppc_bx(UINT32 op)
{
	INT32 li = op & 0x3fffffc;
	if( li & 0x2000000 )
		li |= 0xfc000000;

	if( AABIT ) {
		ppc.npc = li;
	} else {
		ppc.npc = ppc.pc + li;
	}

	if( LKBIT ) {
		LR = ppc.pc + 4;
	}
	
	ppc_change_pc(ppc.npc);
}

static void ppc_bcx(UINT32 op)
{
	int condition = check_condition_code(BO, BI);

	if( condition ) {
		if( AABIT ) {
			ppc.npc = SIMM16 & ~0x3;
		} else {
			ppc.npc = ppc.pc + (SIMM16 & ~0x3);
		}

		ppc_change_pc(ppc.npc);
	}

	if( LKBIT ) {
		LR = ppc.pc + 4;
	}
}

static void ppc_bcctrx(UINT32 op)
{
	int condition = check_condition_code(BO, BI);

	if( condition ) {
		ppc.npc = CTR & ~0x3;
		ppc_change_pc(ppc.npc);
	}

	if( LKBIT ) {
		LR = ppc.pc + 4;
	}
}

static void ppc_bclrx(UINT32 op)
{
	int condition = check_condition_code(BO, BI);

	if( condition ) {
		ppc.npc = LR & ~0x3;
		ppc_change_pc(ppc.npc);
	}

	if( LKBIT ) {
		LR = ppc.pc + 4;
	}
}

static void ppc_cmp(UINT32 op)
{
	INT32 ra = REG(RA);
	INT32 rb = REG(RB);
	int d = CRFD;

	if( ra < rb )
		CR(d) = 0x8;
	else if( ra > rb )
		CR(d) = 0x4;
	else
		CR(d) = 0x2;

	if( XER & XER_SO )
		CR(d) |= 0x1;
}

static void ppc_cmpi(UINT32 op)
{
	INT32 ra = REG(RA);
	INT32 i = SIMM16;
	int d = CRFD;

	if( ra < i )
		CR(d) = 0x8;
	else if( ra > i )
		CR(d) = 0x4;
	else
		CR(d) = 0x2;

	if( XER & XER_SO )
		CR(d) |= 0x1;
}

static void ppc_cmpl(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 rb = REG(RB);
	int d = CRFD;

	if( ra < rb )
		CR(d) = 0x8;
	else if( ra > rb )
		CR(d) = 0x4;
	else
		CR(d) = 0x2;

	if( XER & XER_SO )
		CR(d) |= 0x1;
}

static void ppc_cmpli(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 i = UIMM16;
	int d = CRFD;

	if( ra < i )
		CR(d) = 0x8;
	else if( ra > i )
		CR(d) = 0x4;
	else
		CR(d) = 0x2;

	if( XER & XER_SO )
		CR(d) |= 0x1;
}

static void ppc_cntlzw(UINT32 op)
{
	int n = 0;
	int t = RT;
	UINT32 m = 0x80000000;

	while(n < 32)
	{
		if( REG(t) & m )
			break;
		m >>= 1;
		n++;
	}

	REG(RA) = n;

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_crand(UINT32 op)
{
	int bit = RT;
	int b = CRBIT(RA) & CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crandc(UINT32 op)
{
	int bit = RT;
	int b = CRBIT(RA) & ~CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_creqv(UINT32 op)
{
	int bit = RT;
	int b = ~(CRBIT(RA) ^ CRBIT(RB));
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crnand(UINT32 op)
{
	int bit = RT;
	int b = ~(CRBIT(RA) & CRBIT(RB));
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crnor(UINT32 op)
{
	int bit = RT;
	int b = ~(CRBIT(RA) | CRBIT(RB));
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_cror(UINT32 op)
{
	int bit = RT;
	int b = CRBIT(RA) | CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crorc(UINT32 op)
{
	int bit = RT;
	int b = CRBIT(RA) | ~CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_crxor(UINT32 op)
{
	int bit = RT;
	int b = CRBIT(RA) ^ CRBIT(RB);
	if( b & 0x1 )
		CR(bit / 4) |= _BIT(3-(bit % 4));
	else
		CR(bit / 4) &= ~_BIT(3-(bit % 4));
}

static void ppc_dcbf(UINT32 op)
{

}

static void ppc_dcbi(UINT32 op)
{

}

static void ppc_dcbst(UINT32 op)
{

}

static void ppc_dcbt(UINT32 op)
{

}

static void ppc_dcbtst(UINT32 op)
{

}

static void ppc_dcbz(UINT32 op)
{

}

static void ppc_divwx(UINT32 op)
{
	if( REG(RB) == 0 && REG(RA) < 0x80000000 )
	{
		REG(RT) = 0;
		if( OEBIT ) {
			XER |= XER_SO | XER_OV;
		}
	}
	else if( REG(RB) == 0 || (REG(RB) == 0xffffffff && REG(RA) == 0x80000000) )
	{
		REG(RT) = 0xffffffff;
		if( OEBIT ) {
			XER |= XER_SO | XER_OV;
		}
	}
	else
	{
		REG(RT) = (INT32)REG(RA) / (INT32)REG(RB);
		if( OEBIT ) {
			XER &= ~XER_OV;
		}
	}

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_divwux(UINT32 op)
{
	if( REG(RB) == 0 )
	{
		REG(RT) = 0;
		if( OEBIT ) {
			XER |= XER_SO | XER_OV;
		}
	}
	else
	{
		REG(RT) = (UINT32)REG(RA) / (UINT32)REG(RB);
		if( OEBIT ) {
			XER &= ~XER_OV;
		}
	}

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_eieio(UINT32 op)
{

}

static void ppc_eqvx(UINT32 op)
{
	REG(RA) = ~(REG(RS) ^ REG(RB));

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_extsbx(UINT32 op)
{
	REG(RA) = (INT32)(INT8)REG(RS);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_extshx(UINT32 op)
{
	REG(RA) = (INT32)(INT16)REG(RS);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_icbi(UINT32 op)
{

}

static void ppc_isync(UINT32 op)
{

}

static void ppc_lbz(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	REG(RT) = (UINT32)READ8(ea);
}

static void ppc_lbzu(UINT32 op)
{
	UINT32 ea = REG(RA) + SIMM16;

	REG(RT) = (UINT32)READ8(ea);
	REG(RA) = ea;
}

static void ppc_lbzux(UINT32 op)
{
	UINT32 ea = REG(RA) + REG(RB);

	REG(RT) = (UINT32)READ8(ea);
	REG(RA) = ea;
}

static void ppc_lbzx(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	REG(RT) = (UINT32)READ8(ea);
}

static void ppc_lha(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	REG(RT) = (INT32)(INT16)READ16(ea);
}

static void ppc_lhau(UINT32 op)
{
	UINT32 ea = REG(RA) + SIMM16;

	REG(RT) = (INT32)(INT16)READ16(ea);
	REG(RA) = ea;
}

static void ppc_lhaux(UINT32 op)
{
	UINT32 ea = REG(RA) + REG(RB);

	REG(RT) = (INT32)(INT16)READ16(ea);
	REG(RA) = ea;
}

static void ppc_lhax(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	REG(RT) = (INT32)(INT16)READ16(ea);
}

static void ppc_lhbrx(UINT32 op)
{
	UINT32 ea;
	UINT16 w;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	w = READ16(ea);
	REG(RT) = (UINT32)BYTE_REVERSE16(w);
}

static void ppc_lhz(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	REG(RT) = (UINT32)READ16(ea);
}

static void ppc_lhzu(UINT32 op)
{
	UINT32 ea = REG(RA) + SIMM16;

	REG(RT) = (UINT32)READ16(ea);
	REG(RA) = ea;
}

static void ppc_lhzux(UINT32 op)
{
	UINT32 ea = REG(RA) + REG(RB);

	REG(RT) = (UINT32)READ16(ea);
	REG(RA) = ea;
}

static void ppc_lhzx(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	REG(RT) = (UINT32)READ16(ea);
}

static void ppc_lmw(UINT32 op)
{
	int r = RT;
	UINT32 ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	while( r <= 31 )
	{
		REG(r) = READ32(ea);
		ea += 4;
		r++;
	}
}

static void ppc_lswi(UINT32 op)
{
	int n, r, i;
	UINT32 ea = 0;
	if( RA != 0 )
		ea = REG(RA);

	if( RB == 0 )
		n = 32;
	else
		n = RB;

	r = RT - 1;
	i = 0;

	while(n > 0)
	{
		if (i == 0) {
			r = (r + 1) % 32;
			REG(r) = 0;
		}
		REG(r) |= ((READ8(ea) & 0xff) << (24 - i));
		i += 8;
		if (i == 32) {
			i = 0;
		}
		ea++;
		n--;
	}
}

static void ppc_lswx(UINT32 op)
{
	ErrorLog("PowerPC hit an unimplemented instruction. Halting emulation until reset.");
	DebugLog("ppc: lswx unimplemented at %08X\n", ppc.pc);
	ppc.fatalError = true;
}

static void ppc_lwarx(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	ppc.reserved_address = ea;
	ppc.reserved = 1;

	REG(RT) = READ32(ea);
}

static void ppc_lwbrx(UINT32 op)
{
	UINT32 ea;
	UINT32 w;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	w = READ32(ea);
	REG(RT) = BYTE_REVERSE32(w);
}

static void ppc_lwz(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	REG(RT) = READ32(ea);
}

static void ppc_lwzu(UINT32 op)
{
	UINT32 ea = REG(RA) + SIMM16;

	REG(RT) = READ32(ea);
	REG(RA) = ea;
}

static void ppc_lwzux(UINT32 op)
{
	UINT32 ea = REG(RA) + REG(RB);

	REG(RT) = READ32(ea);
	REG(RA) = ea;
}

static void ppc_lwzx(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	REG(RT) = READ32(ea);
}

static void ppc_mcrf(UINT32 op)
{
	CR(RT >> 2) = CR(RA >> 2);
}

static void ppc_mcrxr(UINT32 op)
{
	CR(RT >> 2) = (XER >> 28) & 0x0F;
	XER &= ~0xf0000000;
}

static void ppc_mfcr(UINT32 op)
{
	REG(RT) = ppc_get_cr();
}

static void ppc_mfmsr(UINT32 op)
{
	REG(RT) = ppc_get_msr();
}

static void ppc_mfspr(UINT32 op)
{
	REG(RT) = ppc_get_spr(SPR);
}

static void ppc_mtcrf(UINT32 op)
{
	int fxm = FXM;
	int t = RT;

	if( fxm & 0x80 )	CR(0) = (REG(t) >> 28) & 0xf;
	if( fxm & 0x40 )	CR(1) = (REG(t) >> 24) & 0xf;
	if( fxm & 0x20 )	CR(2) = (REG(t) >> 20) & 0xf;
	if( fxm & 0x10 )	CR(3) = (REG(t) >> 16) & 0xf;
	if( fxm & 0x08 )	CR(4) = (REG(t) >> 12) & 0xf;
	if( fxm & 0x04 )	CR(5) = (REG(t) >> 8) & 0xf;
	if( fxm & 0x02 )	CR(6) = (REG(t) >> 4) & 0xf;
	if( fxm & 0x01 )	CR(7) = (REG(t) >> 0) & 0xf;
}

static void ppc_mtmsr(UINT32 op)
{
	ppc_set_msr(REG(RS));
}

static void ppc_mtspr(UINT32 op)
{
	ppc_set_spr(SPR, REG(RS));
}

static void ppc_mulhwx(UINT32 op)
{
	INT64 ra = (INT64)(INT32)REG(RA);
	INT64 rb = (INT64)(INT32)REG(RB);

	REG(RT) = (UINT32)((ra * rb) >> 32);

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_mulhwux(UINT32 op)
{
	UINT64 ra = (UINT64)REG(RA);
	UINT64 rb = (UINT64)REG(RB);

	REG(RT) = (UINT32)((ra * rb) >> 32);

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_mulli(UINT32 op)
{
	INT32 ra = (INT32)REG(RA);
	INT32 i = SIMM16;

	REG(RT) = ra * i;
}

static void ppc_mullwx(UINT32 op)
{
	INT64 ra = (INT64)(INT32)REG(RA);
	INT64 rb = (INT64)(INT32)REG(RB);
	INT64 r;

	r = ra * rb;
	REG(RT) = (UINT32)r;

	if( OEBIT ) {
		XER &= ~XER_OV;

		if( r != (INT64)(INT32)r )
			XER |= XER_OV | XER_SO;
	}

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_nandx(UINT32 op)
{
	REG(RA) = ~(REG(RS) & REG(RB));

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_negx(UINT32 op)
{
	REG(RT) = -(INT32)(REG(RA));

	if( OEBIT ) {
		if( REG(RT) == 0x80000000 )
			XER |= XER_OV | XER_SO;
		else
			XER &= ~XER_OV;
	}

	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_norx(UINT32 op)
{
	REG(RA) = ~(REG(RS) | REG(RB));

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_orx(UINT32 op)
{
	REG(RA) = REG(RS) | REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_orcx(UINT32 op)
{
	REG(RA) = REG(RS) | ~REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_ori(UINT32 op)
{
	REG(RA) = REG(RS) | UIMM16;
}

static void ppc_oris(UINT32 op)
{
	REG(RA) = REG(RS) | (UIMM16 << 16);
}

static void ppc_rfi(UINT32 op)
{
	UINT32 msr;
	ppc.npc = ppc_get_spr(SPR_SRR0);
	msr = ppc_get_spr(SPR_SRR1);
	ppc_set_msr( msr );

	ppc_change_pc(ppc.npc);
}

static void ppc_rlwimix(UINT32 op)
{
	UINT32 r;
	UINT32 mask = GET_ROTATE_MASK(MB, ME);
	UINT32 rs = REG(RS);
	int sh = SH;

	r = (rs << sh) | (rs >> (32-sh));
	REG(RA) = (REG(RA) & ~mask) | (r & mask);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_rlwinmx(UINT32 op)
{
	UINT32 r;
	UINT32 mask = GET_ROTATE_MASK(MB, ME);
	UINT32 rs = REG(RS);
	int sh = SH;

	r = (rs << sh) | (rs >> (32-sh));
	REG(RA) = r & mask;

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_rlwnmx(UINT32 op)
{
	UINT32 r;
	UINT32 mask = GET_ROTATE_MASK(MB, ME);
	UINT32 rs = REG(RS);
	int sh = REG(RB) & 0x1f;

	r = (rs << sh) | (rs >> (32-sh));
	REG(RA) = r & mask;

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_sc(UINT32 op)
{
	ppc603_exception(EXCEPTION_SYSTEM_CALL);
}

static void ppc_slwx(UINT32 op)
{
	int sh = REG(RB) & 0x3f;

	if( sh > 31 ) {
		REG(RA) = 0;
	}
	else {
		REG(RA) = REG(RS) << sh;
	}

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_srawx(UINT32 op)
{
	int sh = REG(RB) & 0x3f;

	XER &= ~XER_CA;

	if( sh > 31 ) {
		if (REG(RS) & 0x80000000)
			REG(RA) = 0xffffffff;
		else
			REG(RA) = 0;
		if( REG(RA) )
			XER |= XER_CA;
	}
	else {
		REG(RA) = (INT32)(REG(RS)) >> sh;
		if( ((INT32)(REG(RS)) < 0) && (REG(RS) & BITMASK_0(sh)) )
			XER |= XER_CA;
	}

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_srawix(UINT32 op)
{
	int sh = SH;

	XER &= ~XER_CA;
	if( ((INT32)(REG(RS)) < 0) && (REG(RS) & BITMASK_0(sh)) )
		XER |= XER_CA;

	REG(RA) = (INT32)(REG(RS)) >> sh;

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_srwx(UINT32 op)
{
	int sh = REG(RB) & 0x3f;

	if( sh > 31 ) {
		REG(RA) = 0;
	}
	else {
		REG(RA) = REG(RS) >> sh;
	}

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_stb(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	WRITE8(ea, (UINT8)REG(RS));
}

static void ppc_stbu(UINT32 op)
{
	UINT32 ea = REG(RA) + SIMM16;

	WRITE8(ea, (UINT8)REG(RS));
	REG(RA) = ea;
}

static void ppc_stbux(UINT32 op)
{
	UINT32 ea = REG(RA) + REG(RB);

	WRITE8(ea, (UINT8)REG(RS));
	REG(RA) = ea;
}

static void ppc_stbx(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	WRITE8(ea, (UINT8)REG(RS));
}

static void ppc_sth(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	WRITE16(ea, (UINT16)REG(RS));
}

static void ppc_sthbrx(UINT32 op)
{
	UINT32 ea;
	UINT16 w;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	w = REG(RS);
	WRITE16(ea, (UINT16)BYTE_REVERSE16(w));
}

static void ppc_sthu(UINT32 op)
{
	UINT32 ea = REG(RA) + SIMM16;

	WRITE16(ea, (UINT16)REG(RS));
	REG(RA) = ea;
}

static void ppc_sthux(UINT32 op)
{
	UINT32 ea = REG(RA) + REG(RB);

	WRITE16(ea, (UINT16)REG(RS));
	REG(RA) = ea;
}

static void ppc_sthx(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	WRITE16(ea, (UINT16)REG(RS));
}

static void ppc_stmw(UINT32 op)
{
	UINT32 ea;
	int r = RS;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	while( r <= 31 )
	{
		WRITE32(ea, REG(r));
		ea += 4;
		r++;
	}
}

static void ppc_stswi(UINT32 op)
{
	int n, r, i;
	UINT32 ea = 0;
	if( RA != 0 )
		ea = REG(RA);

	if( RB == 0 )
		n = 32;
	else
		n = RB;

	r = RT - 1;
	i = 0;

	while(n > 0)
	{
		if (i == 0) {
			r = (r + 1) % 32;
		}
		WRITE8(ea, (REG(r) >> (24-i)) & 0xff);
		i += 8;
		if (i == 32) {
			i = 0;
		}
		ea++;
		n--;
	}
}

static void ppc_stswx(UINT32 op)
{
	ErrorLog("PowerPC hit an unimplemented instruction. Halting emulation until reset.");
	DebugLog("ppc: stswx unimplemented\n");
	ppc.fatalError = true;
}

static void ppc_stw(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = SIMM16;
	else
		ea = REG(RA) + SIMM16;

	WRITE32(ea, REG(RS));
}

static void ppc_stwbrx(UINT32 op)
{
	UINT32 ea;
	UINT32 w;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	w = REG(RS);
	WRITE32(ea, BYTE_REVERSE32(w));
}

static void ppc_stwcx_rc(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	if( ppc.reserved ) {
		WRITE32(ea, REG(RS));

		ppc.reserved = 0;
		ppc.reserved_address = 0;

		CR(0) = 0x2;	// set EQ to indicate success
		if( XER & XER_SO )
			CR(0) |= 0x1;
	} else {
		CR(0) = 0;
		if( XER & XER_SO )
			CR(0) |= 0x1;
	}
}

static void ppc_stwu(UINT32 op)
{
	UINT32 ea = REG(RA) + SIMM16;

	WRITE32(ea, REG(RS));
	REG(RA) = ea;
}

static void ppc_stwux(UINT32 op)
{
	UINT32 ea = REG(RA) + REG(RB);

	WRITE32(ea, REG(RS));
	REG(RA) = ea;
}

static void ppc_stwx(UINT32 op)
{
	UINT32 ea;

	if( RA == 0 )
		ea = REG(RB);
	else
		ea = REG(RA) + REG(RB);

	WRITE32(ea, REG(RS));
}

static void ppc_subfx(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 rb = REG(RB);
	REG(RT) = rb - ra;

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), rb, ra);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_subfcx(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 rb = REG(RB);
	REG(RT) = rb - ra;

	SET_SUB_CA(REG(RT), rb, ra);

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), rb, ra);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_subfex(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 rb = REG(RB);
	UINT32 carry = (XER >> 29) & 0x1;
	UINT32 r;

	r = ~ra + carry;
	REG(RT) = rb + r;

	SET_ADD_CA(r, ~ra, carry);		/* step 1 carry */
	if( REG(RT) < r )				/* step 2 carry */
		XER |= XER_CA;

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), rb, ra);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_subfic(UINT32 op)
{
	UINT32 i = SIMM16;
	UINT32 ra = REG(RA);

	REG(RT) = i - ra;

	SET_SUB_CA(REG(RT), i, ra);
}

static void ppc_subfmex(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 carry = (XER >> 29) & 0x1;
	UINT32 r;

	r = ~ra + carry;
	REG(RT) = r - 1;

	SET_SUB_CA(r, ~ra, carry);		/* step 1 carry */
	if( REG(RT) < r )
		XER |= XER_CA;				/* step 2 carry */

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), -1, ra);
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_subfzex(UINT32 op)
{
	UINT32 ra = REG(RA);
	UINT32 carry = (XER >> 29) & 0x1;

	REG(RT) = ~ra + carry;

	SET_ADD_CA(REG(RT), ~ra, carry);

	if( OEBIT ) {
		SET_SUB_OV(REG(RT), 0, REG(RA));
	}
	if( RCBIT ) {
		SET_CR0(REG(RT));
	}
}

static void ppc_sync(UINT32 op)
{

}

static void ppc_tw(UINT32 op)
{
	int exception = 0;
	INT32 a = REG(RA);
	INT32 b = REG(RB);
	int to = RT;

	if( (a < b) && (to & 0x10) ) {
		exception = 1;
	}
	if( (a > b) && (to & 0x08) ) {
		exception = 1;
	}
	if( (a == b) && (to & 0x04) ) {
		exception = 1;
	}
	if( ((UINT32)a < (UINT32)b) && (to & 0x02) ) {
		exception = 1;
	}
	if( ((UINT32)a > (UINT32)b) && (to & 0x01) ) {
		exception = 1;
	}

	if (exception) {
		ppc603_exception(EXCEPTION_TRAP);
	}
}

static void ppc_twi(UINT32 op)
{
	int exception = 0;
	INT32 a = REG(RA);
	INT32 i = SIMM16;
	int to = RT;

	if( (a < i) && (to & 0x10) ) {
		exception = 1;
	}
	if( (a > i) && (to & 0x08) ) {
		exception = 1;
	}
	if( (a == i) && (to & 0x04) ) {
		exception = 1;
	}
	if( ((UINT32)a < (UINT32)i) && (to & 0x02) ) {
		exception = 1;
	}
	if( ((UINT32)a > (UINT32)i) && (to & 0x01) ) {
		exception = 1;
	}

	if (exception) {
		ppc603_exception(EXCEPTION_TRAP);
	}
}

static void ppc_xorx(UINT32 op)
{
	REG(RA) = REG(RS) ^ REG(RB);

	if( RCBIT ) {
		SET_CR0(REG(RA));
	}
}

static void ppc_xori(UINT32 op)
{
	REG(RA) = REG(RS) ^ UIMM16;
}

static void ppc_xoris(UINT32 op)
{
	REG(RA) = REG(RS) ^ (UIMM16 << 16);
}



static void ppc_invalid(UINT32 op)
{
	ErrorLog("PowerPC hit an invalid instruction. Halting emulation until reset.");
	DebugLog("ppc: Invalid opcode %08X PC : %X, %08X\n", op, ppc.pc, ppc.npc);
	ppc.fatalError = true;
}



#define DOUBLE_SIGN		(0x8000000000000000ULL)
#define DOUBLE_EXP		(0x7ff0000000000000ULL)
#define DOUBLE_FRAC		(0x000fffffffffffffULL)
#define DOUBLE_ZERO		(0ULL)

/*
  Floating point operations.
*/

/*************************OLD
INLINE int is_nan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) );
}

INLINE int is_qnan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & 0x0007fffffffffff) == 0x000000000000000) &&
			((x.id & 0x000800000000000) == 0x000800000000000) );
}

INLINE int is_snan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) &&
			((x.id & 0x0008000000000000) == DOUBLE_ZERO) );
}

INLINE int is_infinity_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) == DOUBLE_ZERO) );
}

INLINE int is_normalized_double(FPR x)
{
	UINT64 exp;

	exp = (x.id & DOUBLE_EXP) >> 52;

	return (exp >= 1) && (exp <= 2046);
}

INLINE int is_denormalized_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == 0) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) );
}

INLINE int sign_double(FPR x)
{
	return ((x.id & DOUBLE_SIGN) != 0);
}

INLINE INT64 round_to_nearest(FPR f)
{
	//return (INT64)(f.fd + 0.5);
	if (f.fd >= 0)
	{
		return (INT64)(f.fd + 0.5);
	}
	else
	{
		return -(INT64)(-f.fd + 0.5);
	}
}

INLINE INT64 round_toward_zero(FPR f)
{
	return (INT64)(f.fd);
}

INLINE INT64 round_toward_positive_infinity(FPR f)
{
	double r = ceil(f.fd);
	return (INT64)(r);
}

INLINE INT64 round_toward_negative_infinity(FPR f)
{
	double r = floor(f.fd);
	return (INT64)(r);
}
*/


// New below, based on changes in MAME
INLINE int is_nan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) );
}

INLINE int is_qnan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & 0x0007fffffffffffULL) == 0x000000000000000ULL) &&
			((x.id & 0x000800000000000ULL) == 0x000800000000000ULL) );
}

INLINE int is_snan_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) &&
			((x.id & (0x0008000000000000ULL)) == DOUBLE_ZERO) );
}

INLINE int is_infinity_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == DOUBLE_EXP) &&
			((x.id & DOUBLE_FRAC) == DOUBLE_ZERO) );
}

INLINE int is_normalized_double(FPR x)
{
	UINT64 exp;

	exp = (x.id & DOUBLE_EXP) >> 52;

	return (exp >= 1) && (exp <= 2046);
}

INLINE int is_denormalized_double(FPR x)
{
	return( ((x.id & DOUBLE_EXP) == 0) &&
			((x.id & DOUBLE_FRAC) != DOUBLE_ZERO) );
}

INLINE int sign_double(FPR x)
{
	return ((x.id & DOUBLE_SIGN) != 0);
}

INLINE INT64 smround_to_nearest(FPR f)
{
	if (f.fd >= 0)
	{
		return (INT64)(f.fd + 0.5);
	}
	else
	{
		return -(INT64)(-f.fd + 0.5);
	}
}

INLINE INT64 smround_toward_zero(FPR f)
{
	return (INT64)(f.fd);
}

INLINE INT64 round_toward_positive_infinity(FPR f)
{
	double r = ceil(f.fd);
	return (INT64)(r);
}

INLINE INT64 round_toward_negative_infinity(FPR f)
{
	double r = floor(f.fd);
	return (INT64)(r);
}

#define SET_VXSNAN(a, b)    if (is_snan_double(a) || is_snan_double(b)) ppc.fpscr |= 0x80000000
#define SET_VXSNAN_1(c)     if (is_snan_double(c)) ppc.fpscr |= 0x80000000

INLINE void set_fprf(FPR f)
{
	UINT32 fprf;

	// see page 3-30, 3-31

	if (is_qnan_double(f))
	{
		fprf = 0x11;
	}
	else if (is_infinity_double(f))
	{
		if (sign_double(f))		// -INF
			fprf = 0x09;
		else					// +INF
			fprf = 0x05;
	}
	else if (is_normalized_double(f))
	{
		if (sign_double(f))		// -Normalized
			fprf = 0x08;
		else					// +Normalized
			fprf = 0x04;
	}
	else if (is_denormalized_double(f))
	{
		if (sign_double(f))		// -Denormalized
			fprf = 0x18;
		else					// +Denormalized
			fprf = 0x14;
	}
	else    // Zero
	{
		if (sign_double(f))		// -Zero
			fprf = 0x12;
		else					// +Zero
			fprf = 0x02;
	}

	ppc.fpscr &= ~0x0001f000;
	ppc.fpscr |= (fprf << 12);
}




static void ppc_lfs(UINT32 op)
{
	UINT32 ea = SIMM16;
	UINT32 a = RA;
	UINT32 t = RT;
	FPR32 f;

	if(a)
		ea += REG(a);

	f.i = READ32(ea);
	FPR(t).fd = (double)(f.f);
}

static void ppc_lfsu(UINT32 op)
{
	UINT32 ea = SIMM16;
	UINT32 a = RA;
	UINT32 t = RT;
	FPR32 f;

	ea += REG(a);

	f.i = READ32(ea);
	FPR(t).fd = (double)(f.f);

	REG(a) = ea;
}

static void ppc_lfd(UINT32 op)
{
	UINT32 ea = SIMM16;
	UINT32 a = RA;
	UINT32 t = RT;

	if(a)
		ea += REG(a);

	FPR(t).id = READ64(ea);
}

static void ppc_lfdu(UINT32 op)
{
	UINT32 ea = SIMM16;
	UINT32 a = RA;
	UINT32 d = RD;

	ea += REG(a);

	FPR(d).id = READ64(ea);

	REG(a) = ea;
}

static void ppc_stfs(UINT32 op)
{
	UINT32 ea = SIMM16;
	UINT32 a = RA;
	UINT32 t = RT;
	FPR32 f;

	if(a)
		ea += REG(a);

	f.f = (float)(FPR(t).fd);
	WRITE32(ea, f.i);
}

static void ppc_stfsu(UINT32 op)
{
	UINT32 ea = SIMM16;
	UINT32 a = RA;
	UINT32 t = RT;
	FPR32 f;

	ea += REG(a);

	f.f = (float)(FPR(t).fd);
	WRITE32(ea, f.i);

	REG(a) = ea;
}

static void ppc_stfd(UINT32 op)
{
	UINT32 ea = SIMM16;
	UINT32 a = RA;
	UINT32 t = RT;

	if(a)
		ea += REG(a);

	WRITE64(ea, FPR(t).id);
}

static void ppc_stfdu(UINT32 op)
{
	UINT32 ea = SIMM16;
	UINT32 a = RA;
	UINT32 t = RT;

	ea += REG(a);

	WRITE64(ea, FPR(t).id);

	REG(a) = ea;
}

static void ppc_lfdux(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 d = RD;

	ea += REG(a);

	FPR(d).id = READ64(ea);

	REG(a) = ea;
}

static void ppc_lfdx(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 d = RD;

	if(a)
		ea += REG(a);

	FPR(d).id = READ64(ea);
}

static void ppc_lfsux(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 t = RT;
	FPR32 f;

	ea += REG(a);

	f.i = READ32(ea);
	FPR(t).fd = (double)(f.f);

	REG(a) = ea;
}

static void ppc_lfsx(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 t = RT;
	FPR32 f;

	if(a)
		ea += REG(a);

	f.i = READ32(ea);
	FPR(t).fd = (double)(f.f);
}

static void ppc_mfsr(UINT32 op)
{
	UINT32 sr = (op >> 16) & 15;
	UINT32 t = RT;

	CHECK_SUPERVISOR();

	REG(t) = ppc.sr[sr];
}

static void ppc_mfsrin(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_SUPERVISOR();

	REG(t) = ppc.sr[REG(b) >> 28];
}

static void ppc_mftb(UINT32 op)
{
	UINT32 x = SPRF;

	switch(x)
	{
		case 268:	REG(RT) = (UINT32)(ppc_read_timebase()); break;
		case 269:	REG(RT) = (UINT32)(ppc_read_timebase() >> 32); break;
		default:	
			ErrorLog("PowerPC read from an invalid register. Halting emulation until reset.");
			DebugLog("ppc: Invalid timebase register %d at %08X\n", x, ppc.pc);
			ppc.fatalError = true;
			break;
	}
}

static void ppc_mtsr(UINT32 op)
{
	UINT32 sr = (op >> 16) & 15;
	UINT32 t = RT;

	CHECK_SUPERVISOR();

	ppc.sr[sr] = REG(t);
}

static void ppc_mtsrin(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_SUPERVISOR();

	ppc.sr[REG(b) >> 28] = REG(t);
}

static void ppc_dcba(UINT32 op)
{
	/* TODO: Cache not emulated so this opcode doesn't need to be implemented */
}

static void ppc_stfdux(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 t = RT;

	ea += REG(a);

	WRITE64(ea, FPR(t).id);

	REG(a) = ea;
}

static void ppc_stfdx(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 t = RT;

	if(a)
		ea += REG(a);

	WRITE64(ea, FPR(t).id);
}

static void ppc_stfiwx(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 t = RT;

	if(a)
		ea += REG(a);

	WRITE32(ea, (UINT32)FPR(t).id);
}

static void ppc_stfsux(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 t = RT;
	FPR32 f;

	ea += REG(a);

	f.f = (float)(FPR(t).fd);
	WRITE32(ea, f.i);

	REG(a) = ea;
}

static void ppc_stfsx(UINT32 op)
{
	UINT32 ea = REG(RB);
	UINT32 a = RA;
	UINT32 t = RT;
	FPR32 f;

	if(a)
		ea += REG(a);

	f.f = (float)(FPR(t).fd);

	WRITE32(ea, f.i);
}

static void ppc_tlbia(UINT32 op)
{
	/* TODO: TLB not emulated so this opcode doesn't need to implemented */
}

static void ppc_tlbie(UINT32 op)
{
	/* TODO: TLB not emulated so this opcode doesn't need to implemented */
}

static void ppc_tlbsync(UINT32 op)
{
	/* TODO: TLB not emulated so this opcode doesn't need to implemented */
}

static void ppc_eciwx(UINT32 op)
{
	ppc_unimplemented(op);
}

static void ppc_ecowx(UINT32 op)
{
	ppc_unimplemented(op);
}

static void ppc_fabsx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).id = FPR(b).id & ~DOUBLE_SIGN;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_faddx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = FPR(a).fd + FPR(b).fd;

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fcmpo(UINT32 op)
{
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = (RT >> 2);
	UINT32 c;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));

	if(is_nan_double(FPR(a)) || is_nan_double(FPR(b)))
	{
		c = 1; /* OX */
		if(is_snan_double(FPR(a)) || is_snan_double(FPR(b))) {
			ppc.fpscr |= 0x01000000; /* VXSNAN */

			if(!(ppc.fpscr & 0x40000000) || is_qnan_double(FPR(a)) || is_qnan_double(FPR(b)))
				ppc.fpscr |= 0x00080000; /* VXVC */
		}
	}
	else if(FPR(a).fd < FPR(b).fd){
		c = 8; /* FX */
	}
	else if(FPR(a).fd > FPR(b).fd){
		c = 4; /* FEX */
	}
	else {
		c = 2; /* VX */
	}

	CR(t) = c;

	// TODO
	// Enabled by Bart
	ppc.fpscr &= ~0x0001F000;
	ppc.fpscr |= (c << 12);
}

static void ppc_fcmpu(UINT32 op)
{
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = (RT >> 2);
	UINT32 c;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

    if(is_nan_double(FPR(a)) || is_nan_double(FPR(b)))
	{
		c = 1; /* OX */
		if(is_snan_double(FPR(a)) || is_snan_double(FPR(b))) {
			ppc.fpscr |= 0x01000000; /* VXSNAN */
		}
	}
	else if(FPR(a).fd < FPR(b).fd){
		c = 8; /* FX */
	}
	else if(FPR(a).fd > FPR(b).fd){
		c = 4; /* FEX */
	}
	else {
		c = 2; /* VX */
	}

	CR(t) = c;

	// TODO
	ppc.fpscr &= ~0x0001F000;
	ppc.fpscr |= (c << 12);
}

static void ppc_fctiwx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;
	INT64 r = 0;
	
	// TODO: fix FPSCR flags FX,VXSNAN,VXCVI

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	switch(ppc.fpscr & 3)
	{
		case 0: r = (INT64)smround_to_nearest(FPR(b)); break;
		case 1: r = (INT64)smround_toward_zero(FPR(b)); break;
		case 2: r = (INT64)round_toward_positive_infinity(FPR(b)); break;
		case 3: r = (INT64)round_toward_negative_infinity(FPR(b)); break;
	}

    if(r > (INT64)((INT32)0x7FFFFFFF))
	{
		FPR(t).id = 0x7FFFFFFF;
		// FPSCR[FR] = 0
		// FPSCR[FI] = 1
		// FPSCR[XX] = 1
	}
	else if(FPR(b).fd < (INT64)((INT32)0x80000000))
	{
		FPR(t).id = 0x80000000;
		// FPSCR[FR] = 1
		// FPSCR[FI] = 1
		// FPSCR[XX] = 1
	}
	else
	{
		FPR(t).id = (UINT32)(r);
		// FPSCR[FR] = t.iw > t.fd
		// FPSCR[FI] = t.iw == t.fd
		// FPSCR[XX] = ?
	}

	// FPSCR[FPRF] = undefined (leave it as is)
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fctiwzx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;
	INT64 r;

	// TODO: fix FPSCR flags FX,VXSNAN,VXCVI

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));
	r = smround_toward_zero(FPR(b));

    if(r > (INT64)((INT32)0x7fffffff))
	{
		FPR(t).id = 0x7fffffff;
		// FPSCR[FR] = 0
		// FPSCR[FI] = 1
		// FPSCR[XX] = 1

	}
	else if(r < (INT64)((INT32)0x80000000))
	{
		FPR(t).id = 0x80000000;
		// FPSCR[FR] = 1
		// FPSCR[FI] = 1
		// FPSCR[XX] = 1
	}
	else
	{
		FPR(t).id = (UINT32)r;
		// FPSCR[FR] = t.iw > t.fd
		// FPSCR[FI] = t.iw == t.fd
		// FPSCR[XX] = ?
	}

	// FPSCR[FPRF] = undefined (leave it as is)
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fdivx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

    FPR(t).fd = FPR(a).fd / FPR(b).fd;

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmrx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).fd = FPR(b).fd;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnabsx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).id = FPR(b).id | DOUBLE_SIGN;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnegx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).id = FPR(b).id ^ DOUBLE_SIGN;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_frspx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	FPR(t).fd = (float)FPR(b).fd;

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_frsqrtex(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	FPR(t).fd = 1.0 / sqrt(FPR(b).fd);	/* verify this */

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fsqrtx(UINT32 op)
{
	/* NOTE: PPC603e doesn't support this opcode */
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	FPR(t).fd = (double)(sqrt(FPR(b).fd));

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fsubx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = FPR(a).fd - FPR(b).fd;

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mffsx(UINT32 op)
{
	FPR(RT).id = (UINT32)ppc.fpscr;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mtfsb0x(UINT32 op)
{
    UINT32 crbD;

    crbD = (op >> 21) & 0x1F;

    if (crbD != 1 && crbD != 2) // these bits cannot be explicitly cleared
        ppc.fpscr &= ~(1 << (31 - crbD));

    if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mtfsb1x(UINT32 op)
{
    UINT32 crbD;

    crbD = (op >> 21) & 0x1F;

    if (crbD != 1 && crbD != 2) // these bits cannot be explicitly cleared
        ppc.fpscr |= (1 << (31 - crbD));

    if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mtfsfx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 f = FM;

	f = ppc_field_xlat[FM];

	ppc.fpscr &= (~f) | ~(FPSCR_FEX | FPSCR_VX);
	ppc.fpscr |= (UINT32)(FPR(b).id) & ~(FPSCR_FEX | FPSCR_VX);

	// FEX, VX

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mtfsfix(UINT32 op)
{
    UINT32 crfd = CRFD;
    UINT32 imm = (op >> 12) & 0xF;

    /*
     * According to the manual:
     *
     * If bits 0 and 3 of FPSCR are to be modified, they take the immediate
     * value specified. Bits 1 and 2 (FEX and VX) are set according to the
     * "usual rule" and not from IMM[1-2].
     *
     * The "usual rule" is not emulated, so these bits simply aren't modified
     * at all here.
     */

    crfd = (7 - crfd) * 4;  // calculate LSB position of field

    if (crfd == 28)         // field containing FEX and VX is special...
    {                       // bits 1 and 2 of FPSCR must not be altered
        ppc.fpscr &= 0x9fffffff;
        ppc.fpscr |= (imm & 0x9fffffff);
    }

    ppc.fpscr &= ~(0xf << crfd);    // clear field
    ppc.fpscr |= (imm << crfd);     // insert new data

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_mcrfs(UINT32 op)
{
	UINT32 crfs, f;
	crfs = CRFA;

	f = ppc.fpscr >> ((7 - crfs) * 4);	// get crfS field from FPSCR
	f &= 0xf;

	switch(crfs)	// determine which exception bits to clear in FPSCR
	{
		case 0:		// FX, OX
			ppc.fpscr &= ~0x90000000;
			break;
		case 1:		// UX, ZX, XX, VXSNAN
			ppc.fpscr &= ~0x0f000000;
			break;
		case 2:		// VXISI, VXIDI, VXZDZ, VXIMZ
			ppc.fpscr &= ~0x00F00000;
			break;
		case 3:		// VXVC
			ppc.fpscr &= ~0x00080000;
			break;
		case 5:		// VXSOFT, VXSQRT, VXCVI
			ppc.fpscr &= ~0x00000e00;
			break;
		default:
			break;
	}

	CR(CRFD) = f;
}

static void ppc_faddsx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = (float)(FPR(a).fd + FPR(b).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fdivsx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = (float)(FPR(a).fd / FPR(b).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fresx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN_1(FPR(b));

	FPR(t).fd = 1.0 / FPR(b).fd; /* ??? */

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fsqrtsx(UINT32 op)
{
	/* NOTE: This opcode is not supported in PPC603e */
	UINT32 b = RB;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN_1(FPR(b));

	FPR(t).fd = (float)(sqrt(FPR(b).fd));

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fsubsx(UINT32 op)
{
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));

	FPR(t).fd = (float)(FPR(a).fd - FPR(b).fd);

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmaddx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));
    SET_VXSNAN_1(FPR(c));

	FPR(t).fd = ((FPR(a).fd * FPR(c).fd) + FPR(b).fd);

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmsubx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

    SET_VXSNAN(FPR(a), FPR(b));
    SET_VXSNAN_1(FPR(c));

    FPR(t).fd = ((FPR(a).fd * FPR(c).fd) - FPR(b).fd);

    set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmulx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(c));

	FPR(t).fd = (FPR(a).fd * FPR(c).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnmaddx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (-((FPR(a).fd * FPR(c).fd) + FPR(b).fd));

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnmsubx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (-((FPR(a).fd * FPR(c).fd) - FPR(b).fd));

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fselx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	FPR(t).fd = (FPR(a).fd >= 0.0) ? FPR(c).fd : FPR(b).fd;

	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmaddsx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (float)((FPR(a).fd * FPR(c).fd) + FPR(b).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmsubsx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (float)((FPR(a).fd * FPR(c).fd) - FPR(b).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fmulsx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();
	SET_VXSNAN(FPR(a), FPR(c));

	FPR(t).fd = (float)(FPR(a).fd * FPR(c).fd);

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnmaddsx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

    FPR(t).fd = (float)(-((FPR(a).fd * FPR(c).fd) + FPR(b).fd));

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}

static void ppc_fnmsubsx(UINT32 op)
{
	UINT32 c = RC;
	UINT32 b = RB;
	UINT32 a = RA;
	UINT32 t = RT;

	CHECK_FPU_AVAILABLE();

	SET_VXSNAN(FPR(a), FPR(b));
	SET_VXSNAN_1(FPR(c));

	FPR(t).fd = (float)(-((FPR(a).fd * FPR(c).fd) - FPR(b).fd));

	set_fprf(FPR(t));
	if( RCBIT ) {
		SET_CR1();
	}
}
