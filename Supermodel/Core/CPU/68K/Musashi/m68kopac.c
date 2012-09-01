#include "m68kcpu.h"

/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */


void m68k_op_1010(void)
{
	m68ki_exception_1010();
}


void m68k_op_1111(void)
{
	m68ki_exception_1111();
}


void m68k_op_abcd_8_rr(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


void m68k_op_abcd_8_mm_ax7(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_abcd_8_mm_ay7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_abcd_8_mm_axy7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_abcd_8_mm(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = LOW_NIBBLE(src) + LOW_NIBBLE(dst) + XFLAG_AS_1();

	FLAG_V = ~res; /* Undefined V behavior */

	if(res > 9)
		res += 6;
	res += HIGH_NIBBLE(src) + HIGH_NIBBLE(dst);
	FLAG_X = FLAG_C = (res > 0x99) << 8;
	if(FLAG_C)
		res -= 0xa0;

	FLAG_V &= res; /* Undefined V behavior part II */
	FLAG_N = NFLAG_8(res); /* Undefined N behavior */

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_add_8_er_d(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pi7(void)
{
	uint* r_dst = &DX;
	uint src = OPER_A7_PI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pd7(void)
{
	uint* r_dst = &DX;
	uint src = OPER_A7_PD_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_8_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_d(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_a(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(AY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_16_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_add_32_er_d(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_a(void)
{
	uint* r_dst = &DX;
	uint src = AY;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_ai(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_AI_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_pi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PI_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_pd(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_PD_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_di(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_DI_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_ix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AY_IX_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_aw(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AW_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_al(void)
{
	uint* r_dst = &DX;
	uint src = OPER_AL_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_pcdi(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCDI_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_pcix(void)
{
	uint* r_dst = &DX;
	uint src = OPER_PCIX_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_32_er_i(void)
{
	uint* r_dst = &DX;
	uint src = OPER_I_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_add_8_re_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_aw(void)
{
	uint ea = EA_AW_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_8_re_al(void)
{
	uint ea = EA_AL_8();
	uint src = MASK_OUT_ABOVE_8(DX);
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_add_16_re_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_aw(void)
{
	uint ea = EA_AW_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_16_re_al(void)
{
	uint ea = EA_AL_16();
	uint src = MASK_OUT_ABOVE_16(DX);
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_add_32_re_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_di(void)
{
	uint ea = EA_AY_DI_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_aw(void)
{
	uint ea = EA_AW_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_add_32_re_al(void)
{
	uint ea = EA_AL_32();
	uint src = DX;
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_adda_16_d(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(DY));
}


void m68k_op_adda_16_a(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(AY));
}


void m68k_op_adda_16_ai(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_AY_AI_16()));
}


void m68k_op_adda_16_pi(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_AY_PI_16()));
}


void m68k_op_adda_16_pd(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_AY_PD_16()));
}


void m68k_op_adda_16_di(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_AY_DI_16()));
}


void m68k_op_adda_16_ix(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_AY_IX_16()));
}


void m68k_op_adda_16_aw(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_AW_16()));
}


void m68k_op_adda_16_al(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_AL_16()));
}


void m68k_op_adda_16_pcdi(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_PCDI_16()));
}


void m68k_op_adda_16_pcix(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_PCIX_16()));
}


void m68k_op_adda_16_i(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(OPER_I_16()));
}


void m68k_op_adda_32_d(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + DY);
}


void m68k_op_adda_32_a(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + AY);
}


void m68k_op_adda_32_ai(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_AY_AI_32());
}


void m68k_op_adda_32_pi(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_AY_PI_32());
}


void m68k_op_adda_32_pd(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_AY_PD_32());
}


void m68k_op_adda_32_di(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_AY_DI_32());
}


void m68k_op_adda_32_ix(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_AY_IX_32());
}


void m68k_op_adda_32_aw(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_AW_32());
}


void m68k_op_adda_32_al(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_AL_32());
}


void m68k_op_adda_32_pcdi(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_PCDI_32());
}


void m68k_op_adda_32_pcix(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_PCIX_32());
}


void m68k_op_adda_32_i(void)
{
	uint* r_dst = &AX;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + OPER_I_32());
}


void m68k_op_addi_8_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_addi_8_ai(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_AI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_pi(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_pi7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_pd(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_pd7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_di(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_DI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_ix(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_IX_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_aw(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AW_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_8_al(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AL_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addi_16_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_addi_16_ai(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_pi(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_pd(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_di(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_ix(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_aw(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AW_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_16_al(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AL_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addi_32_d(void)
{
	uint* r_dst = &DY;
	uint src = OPER_I_32();
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_addi_32_ai(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_AI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_pi(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_pd(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_di(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_DI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_ix(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_IX_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_aw(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AW_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addi_32_al(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AL_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_8_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


void m68k_op_addq_8_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_pi7(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_A7_PI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_pd7(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_8_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_addq_16_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


void m68k_op_addq_16_a(void)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((REG_IR >> 9) - 1) & 7) + 1);
}


void m68k_op_addq_16_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_16_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst;

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_addq_32_d(void)
{
	uint* r_dst = &DY;
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint dst = *r_dst;
	uint res = src + dst;

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	*r_dst = FLAG_Z;
}


void m68k_op_addq_32_a(void)
{
	uint* r_dst = &AY;

	*r_dst = MASK_OUT_ABOVE_32(*r_dst + (((REG_IR >> 9) - 1) & 7) + 1);
}


void m68k_op_addq_32_ai(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_AI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_pi(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_pd(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_di(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_DI_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_ix(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AY_IX_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_aw(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AW_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addq_32_al(void)
{
	uint src = (((REG_IR >> 9) - 1) & 7) + 1;
	uint ea = EA_AL_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst;


	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);

	m68ki_write_32(ea, FLAG_Z);
}


void m68k_op_addx_8_rr(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


void m68k_op_addx_16_rr(void)
{
	uint* r_dst = &DX;
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


void m68k_op_addx_32_rr(void)
{
	uint* r_dst = &DX;
	uint src = DY;
	uint dst = *r_dst;
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	*r_dst = res;
}


void m68k_op_addx_8_mm_ax7(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_addx_8_mm_ay7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_addx_8_mm_axy7(void)
{
	uint src = OPER_A7_PD_8();
	uint ea  = EA_A7_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_addx_8_mm(void)
{
	uint src = OPER_AY_PD_8();
	uint ea  = EA_AX_PD_8();
	uint dst = m68ki_read_8(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_8(res);
	FLAG_V = VFLAG_ADD_8(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_8(res);

	res = MASK_OUT_ABOVE_8(res);
	FLAG_Z |= res;

	m68ki_write_8(ea, res);
}


void m68k_op_addx_16_mm(void)
{
	uint src = OPER_AY_PD_16();
	uint ea  = EA_AX_PD_16();
	uint dst = m68ki_read_16(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_16(res);
	FLAG_V = VFLAG_ADD_16(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_16(res);

	res = MASK_OUT_ABOVE_16(res);
	FLAG_Z |= res;

	m68ki_write_16(ea, res);
}


void m68k_op_addx_32_mm(void)
{
	uint src = OPER_AY_PD_32();
	uint ea  = EA_AX_PD_32();
	uint dst = m68ki_read_32(ea);
	uint res = src + dst + XFLAG_AS_1();

	FLAG_N = NFLAG_32(res);
	FLAG_V = VFLAG_ADD_32(src, dst, res);
	FLAG_X = FLAG_C = CFLAG_ADD_32(src, dst, res);

	res = MASK_OUT_ABOVE_32(res);
	FLAG_Z |= res;

	m68ki_write_32(ea, res);
}


void m68k_op_and_8_er_d(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (DY | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_ai(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_AI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pi(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_PI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pi7(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_A7_PI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pd(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_PD_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pd7(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_A7_PD_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_di(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_DI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_ix(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_IX_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_aw(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AW_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_al(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AL_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pcdi(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_PCDI_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_pcix(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_PCIX_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_er_i(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_I_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_d(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (DY | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_ai(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_AI_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_pi(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_PI_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_pd(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_PD_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_di(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_DI_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_ix(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_IX_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_aw(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AW_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_al(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AL_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_pcdi(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_PCDI_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_pcix(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_PCIX_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_16_er_i(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_I_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_d(void)
{
	FLAG_Z = DX &= DY;

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_ai(void)
{
	FLAG_Z = DX &= OPER_AY_AI_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_pi(void)
{
	FLAG_Z = DX &= OPER_AY_PI_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_pd(void)
{
	FLAG_Z = DX &= OPER_AY_PD_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_di(void)
{
	FLAG_Z = DX &= OPER_AY_DI_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_ix(void)
{
	FLAG_Z = DX &= OPER_AY_IX_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_aw(void)
{
	FLAG_Z = DX &= OPER_AW_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_al(void)
{
	FLAG_Z = DX &= OPER_AL_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_pcdi(void)
{
	FLAG_Z = DX &= OPER_PCDI_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_pcix(void)
{
	FLAG_Z = DX &= OPER_PCIX_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_32_er_i(void)
{
	FLAG_Z = DX &= OPER_I_32();

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_and_8_re_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_di(void)
{
	uint ea = EA_AY_DI_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_aw(void)
{
	uint ea = EA_AW_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_8_re_al(void)
{
	uint ea = EA_AL_8();
	uint res = DX & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_8(res);

	m68ki_write_8(ea, FLAG_Z);
}


void m68k_op_and_16_re_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_di(void)
{
	uint ea = EA_AY_DI_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_aw(void)
{
	uint ea = EA_AW_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_16_re_al(void)
{
	uint ea = EA_AL_16();
	uint res = DX & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_Z = MASK_OUT_ABOVE_16(res);

	m68ki_write_16(ea, FLAG_Z);
}


void m68k_op_and_32_re_ai(void)
{
	uint ea = EA_AY_AI_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_pi(void)
{
	uint ea = EA_AY_PI_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_pd(void)
{
	uint ea = EA_AY_PD_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_di(void)
{
	uint ea = EA_AY_DI_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_ix(void)
{
	uint ea = EA_AY_IX_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_aw(void)
{
	uint ea = EA_AW_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_and_32_re_al(void)
{
	uint ea = EA_AL_32();
	uint res = DX & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_8_d(void)
{
	FLAG_Z = MASK_OUT_ABOVE_8(DY &= (OPER_I_8() | 0xffffff00));

	FLAG_N = NFLAG_8(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_andi_8_ai(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_AI_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_pi(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PI_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_pi7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PI_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_pd(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_PD_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_pd7(void)
{
	uint src = OPER_I_8();
	uint ea = EA_A7_PD_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_di(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_DI_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_ix(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AY_IX_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_aw(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AW_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_8_al(void)
{
	uint src = OPER_I_8();
	uint ea = EA_AL_8();
	uint res = src & m68ki_read_8(ea);

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_8(ea, res);
}


void m68k_op_andi_16_d(void)
{
	FLAG_Z = MASK_OUT_ABOVE_16(DY &= (OPER_I_16() | 0xffff0000));

	FLAG_N = NFLAG_16(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_andi_16_ai(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_AI_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_pi(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PI_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_pd(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_PD_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_di(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_DI_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_ix(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AY_IX_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_aw(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AW_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_16_al(void)
{
	uint src = OPER_I_16();
	uint ea = EA_AL_16();
	uint res = src & m68ki_read_16(ea);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_16(ea, res);
}


void m68k_op_andi_32_d(void)
{
	FLAG_Z = DY &= (OPER_I_32());

	FLAG_N = NFLAG_32(FLAG_Z);
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_andi_32_ai(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_AI_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_pi(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PI_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_pd(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_PD_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_di(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_DI_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_ix(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AY_IX_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_aw(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AW_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_32_al(void)
{
	uint src = OPER_I_32();
	uint ea = EA_AL_32();
	uint res = src & m68ki_read_32(ea);

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_C = CFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;

	m68ki_write_32(ea, res);
}


void m68k_op_andi_16_toc(void)
{
	m68ki_set_ccr(m68ki_get_ccr() & OPER_I_16());
}


void m68k_op_andi_16_tos(void)
{
	if(FLAG_S)
	{
		uint src = OPER_I_16();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_set_sr(m68ki_get_sr() & src);
		return;
	}
	m68ki_exception_privilege_violation();
}


void m68k_op_asr_8_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_8(src))
		res |= m68ki_shift_8_table[shift];

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


void m68k_op_asr_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_16(src))
		res |= m68ki_shift_16_table[shift];

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


void m68k_op_asr_32_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	if(GET_MSB_32(src))
		res |= m68ki_shift_32_table[shift];

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_X = FLAG_C = src << (9-shift);
}


void m68k_op_asr_8_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 8)
		{
			if(GET_MSB_8(src))
				res |= m68ki_shift_8_table[shift];

			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

			FLAG_X = FLAG_C = src << (9-shift);
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_8(src))
		{
			*r_dst |= 0xff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asr_16_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 16)
		{
			if(GET_MSB_16(src))
				res |= m68ki_shift_16_table[shift];

			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_16(src))
		{
			*r_dst |= 0xffff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asr_32_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = src >> shift;

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			if(GET_MSB_32(src))
				res |= m68ki_shift_32_table[shift];

			*r_dst = res;

			FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
			FLAG_N = NFLAG_32(res);
			FLAG_Z = res;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		if(GET_MSB_32(src))
		{
			*r_dst = 0xffffffff;
			FLAG_C = CFLAG_SET;
			FLAG_X = XFLAG_SET;
			FLAG_N = NFLAG_SET;
			FLAG_Z = ZFLAG_CLEAR;
			FLAG_V = VFLAG_CLEAR;
			return;
		}

		*r_dst = 0;
		FLAG_C = CFLAG_CLEAR;
		FLAG_X = XFLAG_CLEAR;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = VFLAG_CLEAR;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asr_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asr_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = src >> 1;

	if(GET_MSB_16(src))
		res |= 0x8000;

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = FLAG_X = src << 8;
}


void m68k_op_asl_8_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

	FLAG_X = FLAG_C = src << shift;
	FLAG_N = NFLAG_8(res);
	FLAG_Z = res;
	src &= m68ki_shift_8_table[shift + 1];
	FLAG_V = (!(src == 0 || (src == m68ki_shift_8_table[shift + 1] && shift < 8)))<<7;
}


void m68k_op_asl_16_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (8-shift);
	src &= m68ki_shift_16_table[shift + 1];
	FLAG_V = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
}


void m68k_op_asl_32_s(void)
{
	uint* r_dst = &DY;
	uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
		USE_CYCLES(shift<<CYC_SHIFT);

	*r_dst = res;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> (24-shift);
	src &= m68ki_shift_32_table[shift + 1];
	FLAG_V = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
}


void m68k_op_asl_8_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_8(*r_dst);
	uint res = MASK_OUT_ABOVE_8(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 8)
		{
			*r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
			FLAG_X = FLAG_C = src << shift;
			FLAG_N = NFLAG_8(res);
			FLAG_Z = res;
			src &= m68ki_shift_8_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_8_table[shift + 1]))<<7;
			return;
		}

		*r_dst &= 0xffffff00;
		FLAG_X = FLAG_C = ((shift == 8 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_8(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asl_16_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = MASK_OUT_ABOVE_16(*r_dst);
	uint res = MASK_OUT_ABOVE_16(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 16)
		{
			*r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
			FLAG_X = FLAG_C = (src << shift) >> 8;
			FLAG_N = NFLAG_16(res);
			FLAG_Z = res;
			src &= m68ki_shift_16_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
			return;
		}

		*r_dst &= 0xffff0000;
		FLAG_X = FLAG_C = ((shift == 16 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_16(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asl_32_r(void)
{
	uint* r_dst = &DY;
	uint shift = DX & 0x3f;
	uint src = *r_dst;
	uint res = MASK_OUT_ABOVE_32(src << shift);

	if(shift != 0)
	{
		USE_CYCLES(shift<<CYC_SHIFT);

		if(shift < 32)
		{
			*r_dst = res;
			FLAG_X = FLAG_C = (src >> (32 - shift)) << 8;
			FLAG_N = NFLAG_32(res);
			FLAG_Z = res;
			src &= m68ki_shift_32_table[shift + 1];
			FLAG_V = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
			return;
		}

		*r_dst = 0;
		FLAG_X = FLAG_C = ((shift == 32 ? src & 1 : 0))<<8;
		FLAG_N = NFLAG_CLEAR;
		FLAG_Z = ZFLAG_SET;
		FLAG_V = (!(src == 0))<<7;
		return;
	}

	FLAG_C = CFLAG_CLEAR;
	FLAG_N = NFLAG_32(src);
	FLAG_Z = src;
	FLAG_V = VFLAG_CLEAR;
}


void m68k_op_asl_16_ai(void)
{
	uint ea = EA_AY_AI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_pi(void)
{
	uint ea = EA_AY_PI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_pd(void)
{
	uint ea = EA_AY_PD_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_di(void)
{
	uint ea = EA_AY_DI_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_ix(void)
{
	uint ea = EA_AY_IX_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_aw(void)
{
	uint ea = EA_AW_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_asl_16_al(void)
{
	uint ea = EA_AL_16();
	uint src = m68ki_read_16(ea);
	uint res = MASK_OUT_ABOVE_16(src << 1);

	m68ki_write_16(ea, res);

	FLAG_N = NFLAG_16(res);
	FLAG_Z = res;
	FLAG_X = FLAG_C = src >> 7;
	src &= 0xc000;
	FLAG_V = (!(src == 0 || src == 0xc000))<<7;
}


void m68k_op_bhi_8(void)
{
	if(COND_HI())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bls_8(void)
{
	if(COND_LS())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bcc_8(void)
{
	if(COND_CC())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bcs_8(void)
{
	if(COND_CS())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bne_8(void)
{
	if(COND_NE())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_beq_8(void)
{
	if(COND_EQ())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bvc_8(void)
{
	if(COND_VC())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bvs_8(void)
{
	if(COND_VS())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bpl_8(void)
{
	if(COND_PL())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bmi_8(void)
{
	if(COND_MI())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bge_8(void)
{
	if(COND_GE())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_blt_8(void)
{
	if(COND_LT())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bgt_8(void)
{
	if(COND_GT())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_ble_8(void)
{
	if(COND_LE())
	{
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
		return;
	}
	USE_CYCLES(CYC_BCC_NOTAKE_B);
}


void m68k_op_bhi_16(void)
{
	if(COND_HI())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bls_16(void)
{
	if(COND_LS())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bcc_16(void)
{
	if(COND_CC())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bcs_16(void)
{
	if(COND_CS())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bne_16(void)
{
	if(COND_NE())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_beq_16(void)
{
	if(COND_EQ())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bvc_16(void)
{
	if(COND_VC())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bvs_16(void)
{
	if(COND_VS())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bpl_16(void)
{
	if(COND_PL())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bmi_16(void)
{
	if(COND_MI())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bge_16(void)
{
	if(COND_GE())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_blt_16(void)
{
	if(COND_LT())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bgt_16(void)
{
	if(COND_GT())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_ble_16(void)
{
	if(COND_LE())
	{
		uint offset = OPER_I_16();
		REG_PC -= 2;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_16(offset);
		return;
	}
	REG_PC += 2;
	USE_CYCLES(CYC_BCC_NOTAKE_W);
}


void m68k_op_bhi_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_HI())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bls_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_LS())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bcc_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_CC())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bcs_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_CS())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bne_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_NE())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_beq_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_EQ())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bvc_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_VC())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bvs_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_VS())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bpl_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_PL())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bmi_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_MI())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bge_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_GE())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_blt_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_LT())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bgt_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_GT())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_ble_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		if(COND_LE())
		{
			uint offset = OPER_I_32();
			REG_PC -= 4;
			m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
			m68ki_branch_32(offset);
			return;
		}
		REG_PC += 4;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bchg_32_r_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst ^= mask;
}


void m68k_op_bchg_8_r_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_aw(void)
{
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_r_al(void)
{
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_32_s_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst ^= mask;
}


void m68k_op_bchg_8_s_ai(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_pi(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_pi7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_pd(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_pd7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_di(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_ix(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_aw(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bchg_8_s_al(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src ^ mask);
}


void m68k_op_bclr_32_r_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst &= ~mask;
}


void m68k_op_bclr_8_r_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_aw(void)
{
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_r_al(void)
{
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_32_s_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst &= ~mask;
}


void m68k_op_bclr_8_s_ai(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_pi(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_pi7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_pd(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_pd7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_di(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_ix(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_aw(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bclr_8_s_al(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src & ~mask);
}


void m68k_op_bfchg_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint offset = (word2>>6)&31;
		uint width = word2;
		uint* data = &DY;
		uint64 mask;


		if(BIT_B(word2))
			offset = REG_D[offset&7];
		if(BIT_5(word2))
			width = REG_D[width&7];

		offset &= 31;
		width = ((width-1) & 31) + 1;

		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		FLAG_N = NFLAG_32(*data<<offset);
		FLAG_Z = *data & mask;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		*data ^= mask;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfchg_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long ^ mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte ^ mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint offset = (word2>>6)&31;
		uint width = word2;
		uint* data = &DY;
		uint64 mask;


		if(BIT_B(word2))
			offset = REG_D[offset&7];
		if(BIT_5(word2))
			width = REG_D[width&7];


		offset &= 31;
		width = ((width-1) & 31) + 1;


		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		FLAG_N = NFLAG_32(*data<<offset);
		FLAG_Z = *data & mask;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		*data &= ~mask;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfclr_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long & ~mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte & ~mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint offset = (word2>>6)&31;
		uint width = word2;
		uint64 data = DY;


		if(BIT_B(word2))
			offset = REG_D[offset&7];
		if(BIT_5(word2))
			width = REG_D[width&7];

		offset &= 31;
		width = ((width-1) & 31) + 1;

		data = ROL_32(data, offset);
		FLAG_N = NFLAG_32(data);
		data = MAKE_INT_32(data) >> (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2>>12)&7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_PCDI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfexts_32_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_PCIX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);

		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  = MAKE_INT_32(data) >> (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint offset = (word2>>6)&31;
		uint width = word2;
		uint64 data = DY;


		if(BIT_B(word2))
			offset = REG_D[offset&7];
		if(BIT_5(word2))
			width = REG_D[width&7];

		offset &= 31;
		width = ((width-1) & 31) + 1;

		data = ROL_32(data, offset);
		FLAG_N = NFLAG_32(data);
		data >>= 32 - width;

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2>>12)&7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AY_AI_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AY_DI_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AY_IX_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AW_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_AL_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_PCDI_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfextu_32_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint data;
		uint ea = EA_PCIX_8();


		if(BIT_B(word2))
		offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<offset);

		if((offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		REG_D[(word2 >> 12) & 7] = data;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint offset = (word2>>6)&31;
		uint width = word2;
		uint64 data = DY;
		uint bit;


		if(BIT_B(word2))
			offset = REG_D[offset&7];
		if(BIT_5(word2))
			width = REG_D[width&7];

		offset &= 31;
		width = ((width-1) & 31) + 1;

		data = ROL_32(data, offset);
		FLAG_N = NFLAG_32(data);
		data >>= 32 - width;

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		sint local_offset;
		uint width = word2;
		uint data;
		uint bit;
		uint ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		sint local_offset;
		uint width = word2;
		uint data;
		uint bit;
		uint ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		sint local_offset;
		uint width = word2;
		uint data;
		uint bit;
		uint ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		sint local_offset;
		uint width = word2;
		uint data;
		uint bit;
		uint ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		sint local_offset;
		uint width = word2;
		uint data;
		uint bit;
		uint ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		sint local_offset;
		uint width = word2;
		uint data;
		uint bit;
		uint ea = EA_PCDI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfffo_32_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		sint local_offset;
		uint width = word2;
		uint data;
		uint bit;
		uint ea = EA_PCIX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		local_offset = offset % 8;
		if(local_offset < 0)
		{
			local_offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		data = m68ki_read_32(ea);
		data = MASK_OUT_ABOVE_32(data<<local_offset);

		if((local_offset+width) > 32)
			data |= (m68ki_read_8(ea+4) << local_offset) >> 8;

		FLAG_N = NFLAG_32(data);
		data  >>= (32 - width);

		FLAG_Z = data;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		for(bit = 1<<(width-1);bit && !(data & bit);bit>>= 1)
			offset++;

		REG_D[(word2>>12)&7] = offset;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfins_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint offset = (word2>>6)&31;
		uint width = word2;
		uint* data = &DY;
		uint64 mask;
		uint64 insert = REG_D[(word2>>12)&7];


		if(BIT_B(word2))
			offset = REG_D[offset&7];
		if(BIT_5(word2))
			width = REG_D[width&7];


		offset &= 31;
		width = ((width-1) & 31) + 1;


		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		insert = MASK_OUT_ABOVE_32(insert << (32 - width));
		FLAG_N = NFLAG_32(insert);
		FLAG_Z = insert;
		insert = ROR_32(insert, offset);

		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		*data &= ~mask;
		*data |= insert;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfins_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint insert_base = REG_D[(word2>>12)&7];
		uint insert_long;
		uint insert_byte;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		FLAG_N = NFLAG_32(insert_base);
		FLAG_Z = insert_base;
		insert_long = insert_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			insert_byte = MASK_OUT_ABOVE_8(insert_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfins_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint insert_base = REG_D[(word2>>12)&7];
		uint insert_long;
		uint insert_byte;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		FLAG_N = NFLAG_32(insert_base);
		FLAG_Z = insert_base;
		insert_long = insert_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			insert_byte = MASK_OUT_ABOVE_8(insert_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfins_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint insert_base = REG_D[(word2>>12)&7];
		uint insert_long;
		uint insert_byte;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		FLAG_N = NFLAG_32(insert_base);
		FLAG_Z = insert_base;
		insert_long = insert_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			insert_byte = MASK_OUT_ABOVE_8(insert_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfins_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint insert_base = REG_D[(word2>>12)&7];
		uint insert_long;
		uint insert_byte;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		FLAG_N = NFLAG_32(insert_base);
		FLAG_Z = insert_base;
		insert_long = insert_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			insert_byte = MASK_OUT_ABOVE_8(insert_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfins_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint insert_base = REG_D[(word2>>12)&7];
		uint insert_long;
		uint insert_byte;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;

		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		insert_base = MASK_OUT_ABOVE_32(insert_base << (32 - width));
		FLAG_N = NFLAG_32(insert_base);
		FLAG_Z = insert_base;
		insert_long = insert_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, (data_long & ~mask_long) | insert_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			insert_byte = MASK_OUT_ABOVE_8(insert_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, (data_byte & ~mask_byte) | insert_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfset_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint offset = (word2>>6)&31;
		uint width = word2;
		uint* data = &DY;
		uint64 mask;


		if(BIT_B(word2))
			offset = REG_D[offset&7];
		if(BIT_5(word2))
			width = REG_D[width&7];


		offset &= 31;
		width = ((width-1) & 31) + 1;


		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		FLAG_N = NFLAG_32(*data<<offset);
		FLAG_Z = *data & mask;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		*data |= mask;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfset_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_AI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfset_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_DI_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfset_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_IX_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfset_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AW_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bfset_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AL_8();


		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = NFLAG_32(data_long << offset);
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		m68ki_write_32(ea, data_long | mask_long);

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
			m68ki_write_8(ea+4, data_byte | mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bftst_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint offset = (word2>>6)&31;
		uint width = word2;
		uint* data = &DY;
		uint64 mask;


		if(BIT_B(word2))
			offset = REG_D[offset&7];
		if(BIT_5(word2))
			width = REG_D[width&7];


		offset &= 31;
		width = ((width-1) & 31) + 1;


		mask = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask = ROR_32(mask, offset);

		FLAG_N = NFLAG_32(*data<<offset);
		FLAG_Z = *data & mask;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bftst_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_AI_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bftst_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_DI_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bftst_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AY_IX_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bftst_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AW_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bftst_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_AL_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bftst_32_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_PCDI_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bftst_32_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		sint offset = (word2>>6)&31;
		uint width = word2;
		uint mask_base;
		uint data_long;
		uint mask_long;
		uint data_byte = 0;
		uint mask_byte = 0;
		uint ea = EA_PCIX_8();

		if(BIT_B(word2))
			offset = MAKE_INT_32(REG_D[offset&7]);
		if(BIT_5(word2))
			width = REG_D[width&7];

		/* Offset is signed so we have to use ugly math =( */
		ea += offset / 8;
		offset %= 8;
		if(offset < 0)
		{
			offset += 8;
			ea--;
		}
		width = ((width-1) & 31) + 1;


		mask_base = MASK_OUT_ABOVE_32(0xffffffff << (32 - width));
		mask_long = mask_base >> offset;

		data_long = m68ki_read_32(ea);
		FLAG_N = ((data_long & (0x80000000 >> offset))<<offset)>>24;
		FLAG_Z = data_long & mask_long;
		FLAG_V = VFLAG_CLEAR;
		FLAG_C = CFLAG_CLEAR;

		if((width + offset) > 32)
		{
			mask_byte = MASK_OUT_ABOVE_8(mask_base);
			data_byte = m68ki_read_8(ea+4);
			FLAG_Z |= (data_byte & mask_byte);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bkpt(void)
{
	if(CPU_TYPE_IS_010_PLUS(CPU_TYPE))
	{
		m68ki_bkpt_ack(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE) ? REG_IR & 7 : 0);	/* auto-disable (see m68kcpu.h) */
	}
	m68ki_exception_illegal();
}


void m68k_op_bra_8(void)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_bra_16(void)
{
	uint offset = OPER_I_16();
	REG_PC -= 2;
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68ki_branch_16(offset);
	if(REG_PC == REG_PPC)
		USE_ALL_CYCLES();
}


void m68k_op_bra_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint offset = OPER_I_32();
		REG_PC -= 4;
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_branch_32(offset);
		if(REG_PC == REG_PPC)
			USE_ALL_CYCLES();
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_bset_32_r_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (DX & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst |= mask;
}


void m68k_op_bset_8_r_ai(void)
{
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_pi(void)
{
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_pi7(void)
{
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_pd(void)
{
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_pd7(void)
{
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_di(void)
{
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_ix(void)
{
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_aw(void)
{
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_r_al(void)
{
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);
	uint mask = 1 << (DX & 7);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_32_s_d(void)
{
	uint* r_dst = &DY;
	uint mask = 1 << (OPER_I_8() & 0x1f);

	FLAG_Z = *r_dst & mask;
	*r_dst |= mask;
}


void m68k_op_bset_8_s_ai(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_AI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_pi(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_pi7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_pd(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_pd7(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_A7_PD_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_di(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_DI_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_ix(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AY_IX_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_aw(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AW_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bset_8_s_al(void)
{
	uint mask = 1 << (OPER_I_8() & 7);
	uint ea = EA_AL_8();
	uint src = m68ki_read_8(ea);

	FLAG_Z = src & mask;
	m68ki_write_8(ea, src | mask);
}


void m68k_op_bsr_8(void)
{
	m68ki_trace_t0();				   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
}


void m68k_op_bsr_16(void)
{
	uint offset = OPER_I_16();
	m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
	m68ki_push_32(REG_PC);
	REG_PC -= 2;
	m68ki_branch_16(offset);
}


void m68k_op_bsr_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint offset = OPER_I_32();
		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		m68ki_push_32(REG_PC);
		REG_PC -= 4;
		m68ki_branch_32(offset);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_btst_32_r_d(void)
{
	FLAG_Z = DY & (1 << (DX & 0x1f));
}


void m68k_op_btst_8_r_ai(void)
{
	FLAG_Z = OPER_AY_AI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pi(void)
{
	FLAG_Z = OPER_AY_PI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pi7(void)
{
	FLAG_Z = OPER_A7_PI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pd(void)
{
	FLAG_Z = OPER_AY_PD_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pd7(void)
{
	FLAG_Z = OPER_A7_PD_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_di(void)
{
	FLAG_Z = OPER_AY_DI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_ix(void)
{
	FLAG_Z = OPER_AY_IX_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_aw(void)
{
	FLAG_Z = OPER_AW_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_al(void)
{
	FLAG_Z = OPER_AL_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pcdi(void)
{
	FLAG_Z = OPER_PCDI_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_pcix(void)
{
	FLAG_Z = OPER_PCIX_8() & (1 << (DX & 7));
}


void m68k_op_btst_8_r_i(void)
{
	FLAG_Z = OPER_I_8() & (1 << (DX & 7));
}


void m68k_op_btst_32_s_d(void)
{
	FLAG_Z = DY & (1 << (OPER_I_8() & 0x1f));
}


void m68k_op_btst_8_s_ai(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_AI_8() & (1 << bit);
}


void m68k_op_btst_8_s_pi(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_PI_8() & (1 << bit);
}


void m68k_op_btst_8_s_pi7(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_A7_PI_8() & (1 << bit);
}


void m68k_op_btst_8_s_pd(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_PD_8() & (1 << bit);
}


void m68k_op_btst_8_s_pd7(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_A7_PD_8() & (1 << bit);
}


void m68k_op_btst_8_s_di(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_DI_8() & (1 << bit);
}


void m68k_op_btst_8_s_ix(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AY_IX_8() & (1 << bit);
}


void m68k_op_btst_8_s_aw(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AW_8() & (1 << bit);
}


void m68k_op_btst_8_s_al(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_AL_8() & (1 << bit);
}


void m68k_op_btst_8_s_pcdi(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_PCDI_8() & (1 << bit);
}


void m68k_op_btst_8_s_pcix(void)
{
	uint bit = OPER_I_8() & 7;

	FLAG_Z = OPER_PCIX_8() & (1 << bit);
}


void m68k_op_callm_32_ai(void)
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
		uint ea = EA_AY_AI_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_PC += 2;
(void)ea;	/* just to avoid an 'unused variable' warning */
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_callm_32_di(void)
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
		uint ea = EA_AY_DI_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_PC += 2;
(void)ea;	/* just to avoid an 'unused variable' warning */
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_callm_32_ix(void)
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
		uint ea = EA_AY_IX_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_PC += 2;
(void)ea;	/* just to avoid an 'unused variable' warning */
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_callm_32_aw(void)
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
		uint ea = EA_AW_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_PC += 2;
(void)ea;	/* just to avoid an 'unused variable' warning */
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_callm_32_al(void)
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
		uint ea = EA_AL_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_PC += 2;
(void)ea;	/* just to avoid an 'unused variable' warning */
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_callm_32_pcdi(void)
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
		uint ea = EA_PCDI_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_PC += 2;
(void)ea;	/* just to avoid an 'unused variable' warning */
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_callm_32_pcix(void)
{
	/* note: watch out for pcrelative modes */
	if(CPU_TYPE_IS_020_VARIANT(CPU_TYPE))
	{
		uint ea = EA_PCIX_32();

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		REG_PC += 2;
(void)ea;	/* just to avoid an 'unused variable' warning */
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_AI_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_pi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_PI_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_pi7(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_A7_PI_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_pd(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_PD_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_pd7(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_A7_PD_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_DI_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_IX_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AW_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_8_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AL_8();
		uint dest = m68ki_read_8(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_8(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(*compare, dest, res);
		FLAG_C = CFLAG_8(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_8(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_8(ea, MASK_OUT_ABOVE_8(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_16_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_AI_16();
		uint dest = m68ki_read_16(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(*compare, dest, res);
		FLAG_C = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_16_pi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_PI_16();
		uint dest = m68ki_read_16(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(*compare, dest, res);
		FLAG_C = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_16_pd(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_PD_16();
		uint dest = m68ki_read_16(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(*compare, dest, res);
		FLAG_C = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_16_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_DI_16();
		uint dest = m68ki_read_16(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(*compare, dest, res);
		FLAG_C = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_16_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_IX_16();
		uint dest = m68ki_read_16(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(*compare, dest, res);
		FLAG_C = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_16_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AW_16();
		uint dest = m68ki_read_16(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(*compare, dest, res);
		FLAG_C = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_16_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AL_16();
		uint dest = m68ki_read_16(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - MASK_OUT_ABOVE_16(*compare);

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(*compare, dest, res);
		FLAG_C = CFLAG_16(res);

		if(COND_NE())
			*compare = MASK_OUT_BELOW_16(*compare) | dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_D[(word2 >> 6) & 7]));
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_AI_32();
		uint dest = m68ki_read_32(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - *compare;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(*compare, dest, res);
		FLAG_C = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_32(ea, REG_D[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_32_pi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_PI_32();
		uint dest = m68ki_read_32(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - *compare;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(*compare, dest, res);
		FLAG_C = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_32(ea, REG_D[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_32_pd(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_PD_32();
		uint dest = m68ki_read_32(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - *compare;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(*compare, dest, res);
		FLAG_C = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_32(ea, REG_D[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_DI_32();
		uint dest = m68ki_read_32(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - *compare;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(*compare, dest, res);
		FLAG_C = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_32(ea, REG_D[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AY_IX_32();
		uint dest = m68ki_read_32(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - *compare;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(*compare, dest, res);
		FLAG_C = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_32(ea, REG_D[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AW_32();
		uint dest = m68ki_read_32(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - *compare;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(*compare, dest, res);
		FLAG_C = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_32(ea, REG_D[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint ea = EA_AL_32();
		uint dest = m68ki_read_32(ea);
		uint* compare = &REG_D[word2 & 7];
		uint res = dest - *compare;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(*compare, dest, res);
		FLAG_C = CFLAG_SUB_32(*compare, dest, res);

		if(COND_NE())
			*compare = dest;
		else
		{
			USE_CYCLES(3);
			m68ki_write_32(ea, REG_D[(word2 >> 6) & 7]);
		}
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas2_16(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_32();
		uint* compare1 = &REG_D[(word2 >> 16) & 7];
		uint ea1 = REG_DA[(word2 >> 28) & 15];
		uint dest1 = m68ki_read_16(ea1);
		uint res1 = dest1 - MASK_OUT_ABOVE_16(*compare1);
		uint* compare2 = &REG_D[word2 & 7];
		uint ea2 = REG_DA[(word2 >> 12) & 15];
		uint dest2 = m68ki_read_16(ea2);
		uint res2;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_16(res1);
		FLAG_Z = MASK_OUT_ABOVE_16(res1);
		FLAG_V = VFLAG_SUB_16(*compare1, dest1, res1);
		FLAG_C = CFLAG_16(res1);

		if(COND_EQ())
		{
			res2 = dest2 - MASK_OUT_ABOVE_16(*compare2);

			FLAG_N = NFLAG_16(res2);
			FLAG_Z = MASK_OUT_ABOVE_16(res2);
			FLAG_V = VFLAG_SUB_16(*compare2, dest2, res2);
			FLAG_C = CFLAG_16(res2);

			if(COND_EQ())
			{
				USE_CYCLES(3);
				m68ki_write_16(ea1, REG_D[(word2 >> 22) & 7]);
				m68ki_write_16(ea2, REG_D[(word2 >> 6) & 7]);
				return;
			}
		}
		*compare1 = BIT_1F(word2) ? MAKE_INT_16(dest1) : MASK_OUT_BELOW_16(*compare1) | dest1;
		*compare2 = BIT_F(word2) ? MAKE_INT_16(dest2) : MASK_OUT_BELOW_16(*compare2) | dest2;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cas2_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_32();
		uint* compare1 = &REG_D[(word2 >> 16) & 7];
		uint ea1 = REG_DA[(word2 >> 28) & 15];
		uint dest1 = m68ki_read_32(ea1);
		uint res1 = dest1 - *compare1;
		uint* compare2 = &REG_D[word2 & 7];
		uint ea2 = REG_DA[(word2 >> 12) & 15];
		uint dest2 = m68ki_read_32(ea2);
		uint res2;

		m68ki_trace_t0();			   /* auto-disable (see m68kcpu.h) */
		FLAG_N = NFLAG_32(res1);
		FLAG_Z = MASK_OUT_ABOVE_32(res1);
		FLAG_V = VFLAG_SUB_32(*compare1, dest1, res1);
		FLAG_C = CFLAG_SUB_32(*compare1, dest1, res1);

		if(COND_EQ())
		{
			res2 = dest2 - *compare2;

			FLAG_N = NFLAG_32(res2);
			FLAG_Z = MASK_OUT_ABOVE_32(res2);
			FLAG_V = VFLAG_SUB_32(*compare2, dest2, res2);
			FLAG_C = CFLAG_SUB_32(*compare2, dest2, res2);

			if(COND_EQ())
			{
				USE_CYCLES(3);
				m68ki_write_32(ea1, REG_D[(word2 >> 22) & 7]);
				m68ki_write_32(ea2, REG_D[(word2 >> 6) & 7]);
				return;
			}
		}
		*compare1 = dest1;
		*compare2 = dest2;
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_16_d(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(DY);

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_ai(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_AI_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_pi(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_PI_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_pd(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_PD_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_di(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_DI_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_ix(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AY_IX_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_aw(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AW_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_al(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_AL_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_pcdi(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_PCDI_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_pcix(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_PCIX_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_16_i(void)
{
	sint src = MAKE_INT_16(DX);
	sint bound = MAKE_INT_16(OPER_I_16());

	FLAG_Z = ZFLAG_16(src); /* Undocumented */
	FLAG_V = VFLAG_CLEAR;   /* Undocumented */
	FLAG_C = CFLAG_CLEAR;   /* Undocumented */

	if(src >= 0 && src <= bound)
	{
		return;
	}
	FLAG_N = (src < 0)<<7;
	m68ki_exception_trap(EXCEPTION_CHK);
}


void m68k_op_chk_32_d(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(DY);

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_AY_AI_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_pi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_AY_PI_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_pd(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_AY_PD_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_AY_DI_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_AY_IX_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_AW_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_AL_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_PCDI_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_PCIX_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk_32_i(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		sint src = MAKE_INT_32(DX);
		sint bound = MAKE_INT_32(OPER_I_32());

		FLAG_Z = ZFLAG_32(src); /* Undocumented */
		FLAG_V = VFLAG_CLEAR;   /* Undocumented */
		FLAG_C = CFLAG_CLEAR;   /* Undocumented */

		if(src >= 0 && src <= bound)
		{
			return;
		}
		FLAG_N = (src < 0)<<7;
		m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xff;
		uint ea = EA_PCDI_8();
		uint lower_bound = m68ki_read_pcrel_8(ea);
		uint upper_bound = m68ki_read_pcrel_8(ea + 1);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_8(compare) - MAKE_INT_8(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xff;
		uint ea = EA_PCIX_8();
		uint lower_bound = m68ki_read_pcrel_8(ea);
		uint upper_bound = m68ki_read_pcrel_8(ea + 1);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_8(compare) - MAKE_INT_8(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xff;
		uint ea = EA_AY_AI_8();
		uint lower_bound = m68ki_read_8(ea);
		uint upper_bound = m68ki_read_8(ea + 1);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_8(compare) - MAKE_INT_8(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xff;
		uint ea = EA_AY_DI_8();
		uint lower_bound = m68ki_read_8(ea);
		uint upper_bound = m68ki_read_8(ea + 1);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_8(compare) - MAKE_INT_8(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xff;
		uint ea = EA_AY_IX_8();
		uint lower_bound = m68ki_read_8(ea);
		uint upper_bound = m68ki_read_8(ea + 1);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_8(compare) - MAKE_INT_8(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xff;
		uint ea = EA_AW_8();
		uint lower_bound = m68ki_read_8(ea);
		uint upper_bound = m68ki_read_8(ea + 1);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_8(compare) - MAKE_INT_8(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_8_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xff;
		uint ea = EA_AL_8();
		uint lower_bound = m68ki_read_8(ea);
		uint upper_bound = m68ki_read_8(ea + 1);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_8(compare) - MAKE_INT_8(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xffff;
		uint ea = EA_PCDI_16();
		uint lower_bound = m68ki_read_pcrel_16(ea);
		uint upper_bound = m68ki_read_pcrel_16(ea + 2);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(compare) - MAKE_INT_16(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(upper_bound) - MAKE_INT_16(compare);
		else
			FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xffff;
		uint ea = EA_PCIX_16();
		uint lower_bound = m68ki_read_pcrel_16(ea);
		uint upper_bound = m68ki_read_pcrel_16(ea + 2);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(compare) - MAKE_INT_16(lower_bound);
		else
			FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(upper_bound) - MAKE_INT_16(compare);
		else
			FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xffff;
		uint ea = EA_AY_AI_16();
		uint lower_bound = m68ki_read_16(ea);
		uint upper_bound = m68ki_read_16(ea + 2);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(compare) - MAKE_INT_16(lower_bound);
		else
			FLAG_C = compare - lower_bound;

		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}
		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(upper_bound) - MAKE_INT_16(compare);
		else
			FLAG_C = upper_bound - compare;

		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xffff;
		uint ea = EA_AY_DI_16();
		uint lower_bound = m68ki_read_16(ea);
		uint upper_bound = m68ki_read_16(ea + 2);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(compare) - MAKE_INT_16(lower_bound);
		else
			FLAG_C = compare - lower_bound;

		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}
		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(upper_bound) - MAKE_INT_16(compare);
		else
			FLAG_C = upper_bound - compare;

		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xffff;
		uint ea = EA_AY_IX_16();
		uint lower_bound = m68ki_read_16(ea);
		uint upper_bound = m68ki_read_16(ea + 2);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(compare) - MAKE_INT_16(lower_bound);
		else
			FLAG_C = compare - lower_bound;

		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}
		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(upper_bound) - MAKE_INT_16(compare);
		else
			FLAG_C = upper_bound - compare;

		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xffff;
		uint ea = EA_AW_16();
		uint lower_bound = m68ki_read_16(ea);
		uint upper_bound = m68ki_read_16(ea + 2);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(compare) - MAKE_INT_16(lower_bound);
		else
			FLAG_C = compare - lower_bound;

		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}
		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(upper_bound) - MAKE_INT_16(compare);
		else
			FLAG_C = upper_bound - compare;

		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_16_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15]&0xffff;
		uint ea = EA_AL_16();
		uint lower_bound = m68ki_read_16(ea);
		uint upper_bound = m68ki_read_16(ea + 2);

		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(compare) - MAKE_INT_16(lower_bound);
		else
			FLAG_C = compare - lower_bound;

		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}
		if(!BIT_F(word2))
			FLAG_C = MAKE_INT_16(upper_bound) - MAKE_INT_16(compare);
		else
			FLAG_C = upper_bound - compare;

		FLAG_C = CFLAG_16(FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15];
		uint ea = EA_PCDI_32();
		uint lower_bound = m68ki_read_pcrel_32(ea);
		uint upper_bound = m68ki_read_pcrel_32(ea + 4);

		FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_SUB_32(lower_bound, compare, FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_SUB_32(compare, upper_bound, FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15];
		uint ea = EA_PCIX_32();
		uint lower_bound = m68ki_read_pcrel_32(ea);
		uint upper_bound = m68ki_read_pcrel_32(ea + 4);

		FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_SUB_32(lower_bound, compare, FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_SUB_32(compare, upper_bound, FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_ai(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15];
		uint ea = EA_AY_AI_32();
		uint lower_bound = m68ki_read_32(ea);
		uint upper_bound = m68ki_read_32(ea + 4);

		FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_SUB_32(lower_bound, compare, FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_SUB_32(compare, upper_bound, FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_di(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15];
		uint ea = EA_AY_DI_32();
		uint lower_bound = m68ki_read_32(ea);
		uint upper_bound = m68ki_read_32(ea + 4);

		FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_SUB_32(lower_bound, compare, FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_SUB_32(compare, upper_bound, FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_ix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15];
		uint ea = EA_AY_IX_32();
		uint lower_bound = m68ki_read_32(ea);
		uint upper_bound = m68ki_read_32(ea + 4);

		FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_SUB_32(lower_bound, compare, FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_SUB_32(compare, upper_bound, FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_aw(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15];
		uint ea = EA_AW_32();
		uint lower_bound = m68ki_read_32(ea);
		uint upper_bound = m68ki_read_32(ea + 4);

		FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_SUB_32(lower_bound, compare, FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_SUB_32(compare, upper_bound, FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_chk2cmp2_32_al(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint word2 = OPER_I_16();
		uint compare = REG_DA[(word2 >> 12) & 15];
		uint ea = EA_AL_32();
		uint lower_bound = m68ki_read_32(ea);
		uint upper_bound = m68ki_read_32(ea + 4);

		FLAG_C = compare - lower_bound;
		FLAG_Z = !((upper_bound==compare) | (lower_bound==compare));
		FLAG_C = CFLAG_SUB_32(lower_bound, compare, FLAG_C);
		if(COND_CS())
		{
			if(BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
			return;
		}

		FLAG_C = upper_bound - compare;
		FLAG_C = CFLAG_SUB_32(compare, upper_bound, FLAG_C);
		if(COND_CS() && BIT_B(word2))
				m68ki_exception_trap(EXCEPTION_CHK);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_clr_8_d(void)
{
	DY &= 0xffffff00;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_ai(void)
{
	m68ki_write_8(EA_AY_AI_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_pi(void)
{
	m68ki_write_8(EA_AY_PI_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_pi7(void)
{
	m68ki_write_8(EA_A7_PI_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_pd(void)
{
	m68ki_write_8(EA_AY_PD_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_pd7(void)
{
	m68ki_write_8(EA_A7_PD_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_di(void)
{
	m68ki_write_8(EA_AY_DI_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_ix(void)
{
	m68ki_write_8(EA_AY_IX_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_aw(void)
{
	m68ki_write_8(EA_AW_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_8_al(void)
{
	m68ki_write_8(EA_AL_8(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_d(void)
{
	DY &= 0xffff0000;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_ai(void)
{
	m68ki_write_16(EA_AY_AI_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_pi(void)
{
	m68ki_write_16(EA_AY_PI_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_pd(void)
{
	m68ki_write_16(EA_AY_PD_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_di(void)
{
	m68ki_write_16(EA_AY_DI_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_ix(void)
{
	m68ki_write_16(EA_AY_IX_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_aw(void)
{
	m68ki_write_16(EA_AW_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_16_al(void)
{
	m68ki_write_16(EA_AL_16(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_d(void)
{
	DY = 0;

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_ai(void)
{
	m68ki_write_32(EA_AY_AI_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_pi(void)
{
	m68ki_write_32(EA_AY_PI_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_pd(void)
{
	m68ki_write_32(EA_AY_PD_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_di(void)
{
	m68ki_write_32(EA_AY_DI_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_ix(void)
{
	m68ki_write_32(EA_AY_IX_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_aw(void)
{
	m68ki_write_32(EA_AW_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_clr_32_al(void)
{
	m68ki_write_32(EA_AL_32(), 0);

	FLAG_N = NFLAG_CLEAR;
	FLAG_V = VFLAG_CLEAR;
	FLAG_C = CFLAG_CLEAR;
	FLAG_Z = ZFLAG_SET;
}


void m68k_op_cmp_8_d(void)
{
	uint src = MASK_OUT_ABOVE_8(DY);
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_ai(void)
{
	uint src = OPER_AY_AI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pi(void)
{
	uint src = OPER_AY_PI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pi7(void)
{
	uint src = OPER_A7_PI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pd(void)
{
	uint src = OPER_AY_PD_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pd7(void)
{
	uint src = OPER_A7_PD_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_di(void)
{
	uint src = OPER_AY_DI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_ix(void)
{
	uint src = OPER_AY_IX_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_aw(void)
{
	uint src = OPER_AW_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_al(void)
{
	uint src = OPER_AL_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pcdi(void)
{
	uint src = OPER_PCDI_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_pcix(void)
{
	uint src = OPER_PCIX_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_8_i(void)
{
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmp_16_d(void)
{
	uint src = MASK_OUT_ABOVE_16(DY);
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_a(void)
{
	uint src = MASK_OUT_ABOVE_16(AY);
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_ai(void)
{
	uint src = OPER_AY_AI_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_pi(void)
{
	uint src = OPER_AY_PI_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_pd(void)
{
	uint src = OPER_AY_PD_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_di(void)
{
	uint src = OPER_AY_DI_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_ix(void)
{
	uint src = OPER_AY_IX_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_aw(void)
{
	uint src = OPER_AW_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_al(void)
{
	uint src = OPER_AL_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_pcdi(void)
{
	uint src = OPER_PCDI_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_pcix(void)
{
	uint src = OPER_PCIX_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_16_i(void)
{
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(DX);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmp_32_d(void)
{
	uint src = DY;
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_a(void)
{
	uint src = AY;
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_ai(void)
{
	uint src = OPER_AY_AI_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_pi(void)
{
	uint src = OPER_AY_PI_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_pd(void)
{
	uint src = OPER_AY_PD_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_di(void)
{
	uint src = OPER_AY_DI_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_ix(void)
{
	uint src = OPER_AY_IX_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_aw(void)
{
	uint src = OPER_AW_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_al(void)
{
	uint src = OPER_AL_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_pcdi(void)
{
	uint src = OPER_PCDI_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_pcix(void)
{
	uint src = OPER_PCIX_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmp_32_i(void)
{
	uint src = OPER_I_32();
	uint dst = DX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_d(void)
{
	uint src = MAKE_INT_16(DY);
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_a(void)
{
	uint src = MAKE_INT_16(AY);
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_ai(void)
{
	uint src = MAKE_INT_16(OPER_AY_AI_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_pi(void)
{
	uint src = MAKE_INT_16(OPER_AY_PI_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_pd(void)
{
	uint src = MAKE_INT_16(OPER_AY_PD_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_di(void)
{
	uint src = MAKE_INT_16(OPER_AY_DI_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_ix(void)
{
	uint src = MAKE_INT_16(OPER_AY_IX_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_aw(void)
{
	uint src = MAKE_INT_16(OPER_AW_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_al(void)
{
	uint src = MAKE_INT_16(OPER_AL_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_pcdi(void)
{
	uint src = MAKE_INT_16(OPER_PCDI_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_pcix(void)
{
	uint src = MAKE_INT_16(OPER_PCIX_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_16_i(void)
{
	uint src = MAKE_INT_16(OPER_I_16());
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_d(void)
{
	uint src = DY;
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_a(void)
{
	uint src = AY;
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_ai(void)
{
	uint src = OPER_AY_AI_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_pi(void)
{
	uint src = OPER_AY_PI_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_pd(void)
{
	uint src = OPER_AY_PD_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_di(void)
{
	uint src = OPER_AY_DI_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_ix(void)
{
	uint src = OPER_AY_IX_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_aw(void)
{
	uint src = OPER_AW_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_al(void)
{
	uint src = OPER_AL_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_pcdi(void)
{
	uint src = OPER_PCDI_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_pcix(void)
{
	uint src = OPER_PCIX_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpa_32_i(void)
{
	uint src = OPER_I_32();
	uint dst = AX;
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_8_d(void)
{
	uint src = OPER_I_8();
	uint dst = MASK_OUT_ABOVE_8(DY);
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_ai(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_AI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pi(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pi7(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_A7_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pd(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_PD_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pd7(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_A7_PD_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_di(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_DI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_ix(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AY_IX_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_aw(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AW_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_al(void)
{
	uint src = OPER_I_8();
	uint dst = OPER_AL_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpi_8_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint src = OPER_I_8();
		uint dst = OPER_PCDI_8();
		uint res = dst - src;

		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(src, dst, res);
		FLAG_C = CFLAG_8(res);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cmpi_8_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint src = OPER_I_8();
		uint dst = OPER_PCIX_8();
		uint res = dst - src;

		FLAG_N = NFLAG_8(res);
		FLAG_Z = MASK_OUT_ABOVE_8(res);
		FLAG_V = VFLAG_SUB_8(src, dst, res);
		FLAG_C = CFLAG_8(res);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cmpi_16_d(void)
{
	uint src = OPER_I_16();
	uint dst = MASK_OUT_ABOVE_16(DY);
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_ai(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_AI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_pi(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_PI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_pd(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_PD_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_di(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_DI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_ix(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AY_IX_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_aw(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AW_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_al(void)
{
	uint src = OPER_I_16();
	uint dst = OPER_AL_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpi_16_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint src = OPER_I_16();
		uint dst = OPER_PCDI_16();
		uint res = dst - src;

		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(src, dst, res);
		FLAG_C = CFLAG_16(res);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cmpi_16_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint src = OPER_I_16();
		uint dst = OPER_PCIX_16();
		uint res = dst - src;

		FLAG_N = NFLAG_16(res);
		FLAG_Z = MASK_OUT_ABOVE_16(res);
		FLAG_V = VFLAG_SUB_16(src, dst, res);
		FLAG_C = CFLAG_16(res);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cmpi_32_d(void)
{
	uint src = OPER_I_32();
	uint dst = DY;
	uint res = dst - src;

	m68ki_cmpild_callback(src, REG_IR & 7);		   /* auto-disable (see m68kcpu.h) */

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_ai(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_AI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_pi(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_PI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_pd(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_PD_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_di(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_DI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_ix(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AY_IX_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_aw(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AW_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_al(void)
{
	uint src = OPER_I_32();
	uint dst = OPER_AL_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cmpi_32_pcdi(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint src = OPER_I_32();
		uint dst = OPER_PCDI_32();
		uint res = dst - src;

		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(src, dst, res);
		FLAG_C = CFLAG_SUB_32(src, dst, res);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cmpi_32_pcix(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		uint src = OPER_I_32();
		uint dst = OPER_PCIX_32();
		uint res = dst - src;

		FLAG_N = NFLAG_32(res);
		FLAG_Z = MASK_OUT_ABOVE_32(res);
		FLAG_V = VFLAG_SUB_32(src, dst, res);
		FLAG_C = CFLAG_SUB_32(src, dst, res);
		return;
	}
	m68ki_exception_illegal();
}


void m68k_op_cmpm_8_ax7(void)
{
	uint src = OPER_AY_PI_8();
	uint dst = OPER_A7_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpm_8_ay7(void)
{
	uint src = OPER_A7_PI_8();
	uint dst = OPER_AX_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpm_8_axy7(void)
{
	uint src = OPER_A7_PI_8();
	uint dst = OPER_A7_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpm_8(void)
{
	uint src = OPER_AY_PI_8();
	uint dst = OPER_AX_PI_8();
	uint res = dst - src;

	FLAG_N = NFLAG_8(res);
	FLAG_Z = MASK_OUT_ABOVE_8(res);
	FLAG_V = VFLAG_SUB_8(src, dst, res);
	FLAG_C = CFLAG_8(res);
}


void m68k_op_cmpm_16(void)
{
	uint src = OPER_AY_PI_16();
	uint dst = OPER_AX_PI_16();
	uint res = dst - src;

	FLAG_N = NFLAG_16(res);
	FLAG_Z = MASK_OUT_ABOVE_16(res);
	FLAG_V = VFLAG_SUB_16(src, dst, res);
	FLAG_C = CFLAG_16(res);
}


void m68k_op_cmpm_32(void)
{
	uint src = OPER_AY_PI_32();
	uint dst = OPER_AX_PI_32();
	uint res = dst - src;

	FLAG_N = NFLAG_32(res);
	FLAG_Z = MASK_OUT_ABOVE_32(res);
	FLAG_V = VFLAG_SUB_32(src, dst, res);
	FLAG_C = CFLAG_SUB_32(src, dst, res);
}


void m68k_op_cpbcc_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_1111();
}


void m68k_op_cpdbcc_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_1111();
}


void m68k_op_cpgen_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_1111();
}


void m68k_op_cpscc_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_1111();
}


void m68k_op_cptrapcc_32(void)
{
	if(CPU_TYPE_IS_EC020_PLUS(CPU_TYPE))
	{
		M68K_DO_LOG((M68K_LOG_FILEHANDLE "%s at %08x: called unimplemented instruction %04x (%s)\n",
					 m68ki_cpu_names[CPU_TYPE], ADDRESS_68K(REG_PC - 2), REG_IR,
					 m68k_disassemble_quick(ADDRESS_68K(REG_PC - 2))));
		return;
	}
	m68ki_exception_1111();
}


/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */


