
/* ======================================================================== */
/* ============== CYCLE-ACCURATE DIV/MUL EXECUTION ======================== */
/* ======================================================================== */

INLINE void UseDivuCycles(uint32 dst, uint32 src)
{
  int i;

  /* minimum cycle time */
  uint mcycles = 38 * MUL;

  /* 16-bit divisor */
  src <<= 16;

  /* 16-bit dividend */
  for (i=0; i<15; i++)
  {
    /* check if carry bit set  */
    if ((sint32) dst < 0)
    {
      /* shift dividend and apply divisor */
      dst <<= 1;
      dst -= src;
    }
    else
    {
      /* shift dividend and add two cycles */
      dst <<= 1;
      mcycles += (2 * MUL);

      if (dst >= src)
      {
        /* apply divisor and remove one cycle */
        dst -= src;
        mcycles -= 1 * MUL;
      }
    }
  }

  USE_CYCLES(mcycles << 1);
}

INLINE void UseDivsCycles(sint32 dst, sint16 src)
{
  /* minimum cycle time */
  uint mcycles = 6 * MUL;

  /* negative dividend */
  if (dst < 0) mcycles += 1 * MUL;

  if ((abs(dst) >> 16) < abs(src))
  {
    int i;

    /* absolute quotient */
    uint32 quotient = abs(dst) / abs(src);

    /* add default cycle time */
    mcycles += (55 * MUL);

    /* positive divisor */
    if (src >= 0)
    {
      /* check dividend sign */
      if (dst >= 0) mcycles -= 1 * MUL;
      else mcycles += 1 * MUL;
    }

    /* check higher 15-bits of quotient */
    for (i=0; i<15; i++)
    {
      quotient >>= 1;
      if (!(quotient & 1)) mcycles += 1 * MUL;
    }
  }
  else
  {
    /* absolute overflow */
    mcycles += (2 * MUL);
  }

  USE_CYCLES(mcycles << 1);
}

INLINE void UseMuluCycles(uint16 src)
{
  /* minimum cycle time */
  uint mcycles = 38 * MUL;

  /* count number of bits set to 1 */
  while (src)
  {
    if (src & 1) mcycles += (2 * MUL);
    src >>= 1;
  }

  /* 38 + 2*N */
  USE_CYCLES(mcycles);
}

INLINE void UseMulsCycles(sint16 src)
{
  /* minimum cycle time */
  uint mcycles = 38 * MUL;

  /* detect 01 or 10 patterns */
  sint32 tmp = src << 1;
  tmp = (tmp ^ src) & 0xFFFF;

  /* count number of bits set to 1 */
  while (tmp)
  {
    if (tmp & 1) mcycles += (2 * MUL);
    tmp >>= 1;
  }

  /* 38 + 2*N */
  USE_CYCLES(mcycles);
}


/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */


static void m68k_op_1010(void)
{
  m68ki_exception_1010();
}


static void m68k_op_1111(void)
{
  m68ki_exception_1111();
}


static void m68k_op_abcd_8_rr(void)
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


static void m68k_op_abcd_8_mm_ax7(void)
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


static void m68k_op_abcd_8_mm_ay7(void)
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


static void m68k_op_abcd_8_mm_axy7(void)
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


static void m68k_op_abcd_8_mm(void)
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


static void m68k_op_add_8_er_d(void)
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


static void m68k_op_add_8_er_ai(void)
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


static void m68k_op_add_8_er_pi(void)
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


static void m68k_op_add_8_er_pi7(void)
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


static void m68k_op_add_8_er_pd(void)
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


static void m68k_op_add_8_er_pd7(void)
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


static void m68k_op_add_8_er_di(void)
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


static void m68k_op_add_8_er_ix(void)
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


static void m68k_op_add_8_er_aw(void)
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


static void m68k_op_add_8_er_al(void)
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


static void m68k_op_add_8_er_pcdi(void)
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


static void m68k_op_add_8_er_pcix(void)
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


static void m68k_op_add_8_er_i(void)
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


static void m68k_op_add_16_er_d(void)
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


static void m68k_op_add_16_er_a(void)
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


static void m68k_op_add_16_er_ai(void)
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


static void m68k_op_add_16_er_pi(void)
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


static void m68k_op_add_16_er_pd(void)
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


static void m68k_op_add_16_er_di(void)
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


static void m68k_op_add_16_er_ix(void)
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


static void m68k_op_add_16_er_aw(void)
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


static void m68k_op_add_16_er_al(void)
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


static void m68k_op_add_16_er_pcdi(void)
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


static void m68k_op_add_16_er_pcix(void)
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


static void m68k_op_add_16_er_i(void)
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


static void m68k_op_add_32_er_d(void)
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


static void m68k_op_add_32_er_a(void)
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


static void m68k_op_add_32_er_ai(void)
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


static void m68k_op_add_32_er_pi(void)
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


static void m68k_op_add_32_er_pd(void)
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


static void m68k_op_add_32_er_di(void)
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


static void m68k_op_add_32_er_ix(void)
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


static void m68k_op_add_32_er_aw(void)
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


static void m68k_op_add_32_er_al(void)
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


static void m68k_op_add_32_er_pcdi(void)
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


static void m68k_op_add_32_er_pcix(void)
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


static void m68k_op_add_32_er_i(void)
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


static void m68k_op_add_8_re_ai(void)
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


static void m68k_op_add_8_re_pi(void)
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


static void m68k_op_add_8_re_pi7(void)
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


static void m68k_op_add_8_re_pd(void)
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


static void m68k_op_add_8_re_pd7(void)
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


static void m68k_op_add_8_re_di(void)
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


static void m68k_op_add_8_re_ix(void)
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


static void m68k_op_add_8_re_aw(void)
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


static void m68k_op_add_8_re_al(void)
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


static void m68k_op_add_16_re_ai(void)
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


static void m68k_op_add_16_re_pi(void)
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


static void m68k_op_add_16_re_pd(void)
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


static void m68k_op_add_16_re_di(void)
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


static void m68k_op_add_16_re_ix(void)
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


static void m68k_op_add_16_re_aw(void)
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


static void m68k_op_add_16_re_al(void)
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


static void m68k_op_add_32_re_ai(void)
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


static void m68k_op_add_32_re_pi(void)
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


static void m68k_op_add_32_re_pd(void)
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


static void m68k_op_add_32_re_di(void)
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


static void m68k_op_add_32_re_ix(void)
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


static void m68k_op_add_32_re_aw(void)
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


static void m68k_op_add_32_re_al(void)
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


static void m68k_op_adda_16_d(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(DY));
}


static void m68k_op_adda_16_a(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + MAKE_INT_16(AY));
}


static void m68k_op_adda_16_ai(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_AI_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_pi(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_PI_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_pd(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_PD_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_di(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_DI_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_ix(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_IX_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_aw(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AW_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_al(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AL_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_pcdi(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_PCDI_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_pcix(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_PCIX_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_16_i(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_I_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + src);
}


static void m68k_op_adda_32_d(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + DY);
}


static void m68k_op_adda_32_a(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + AY);
}


static void m68k_op_adda_32_ai(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_AY_AI_32() + *r_dst);
}


static void m68k_op_adda_32_pi(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_AY_PI_32() + *r_dst);
}


static void m68k_op_adda_32_pd(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_AY_PD_32() + *r_dst);
}


static void m68k_op_adda_32_di(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_AY_DI_32() + *r_dst);
}


static void m68k_op_adda_32_ix(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_AY_IX_32() + *r_dst);
}


static void m68k_op_adda_32_aw(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_AW_32() + *r_dst);
}


static void m68k_op_adda_32_al(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_AL_32() + *r_dst);
}


static void m68k_op_adda_32_pcdi(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_PCDI_32() + *r_dst);
}


static void m68k_op_adda_32_pcix(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_PCIX_32() + *r_dst);
}


static void m68k_op_adda_32_i(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(OPER_I_32() + *r_dst);
}


static void m68k_op_addi_8_d(void)
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


static void m68k_op_addi_8_ai(void)
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


static void m68k_op_addi_8_pi(void)
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


static void m68k_op_addi_8_pi7(void)
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


static void m68k_op_addi_8_pd(void)
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


static void m68k_op_addi_8_pd7(void)
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


static void m68k_op_addi_8_di(void)
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


static void m68k_op_addi_8_ix(void)
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


static void m68k_op_addi_8_aw(void)
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


static void m68k_op_addi_8_al(void)
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


static void m68k_op_addi_16_d(void)
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


static void m68k_op_addi_16_ai(void)
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


static void m68k_op_addi_16_pi(void)
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


static void m68k_op_addi_16_pd(void)
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


static void m68k_op_addi_16_di(void)
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


static void m68k_op_addi_16_ix(void)
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


static void m68k_op_addi_16_aw(void)
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


static void m68k_op_addi_16_al(void)
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


static void m68k_op_addi_32_d(void)
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


static void m68k_op_addi_32_ai(void)
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


static void m68k_op_addi_32_pi(void)
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


static void m68k_op_addi_32_pd(void)
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


static void m68k_op_addi_32_di(void)
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


static void m68k_op_addi_32_ix(void)
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


static void m68k_op_addi_32_aw(void)
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


static void m68k_op_addi_32_al(void)
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


static void m68k_op_addq_8_d(void)
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


static void m68k_op_addq_8_ai(void)
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


static void m68k_op_addq_8_pi(void)
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


static void m68k_op_addq_8_pi7(void)
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


static void m68k_op_addq_8_pd(void)
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


static void m68k_op_addq_8_pd7(void)
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


static void m68k_op_addq_8_di(void)
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


static void m68k_op_addq_8_ix(void)
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


static void m68k_op_addq_8_aw(void)
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


static void m68k_op_addq_8_al(void)
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


static void m68k_op_addq_16_d(void)
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


static void m68k_op_addq_16_a(void)
{
  uint* r_dst = &AY;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + (((REG_IR >> 9) - 1) & 7) + 1);
}


static void m68k_op_addq_16_ai(void)
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


static void m68k_op_addq_16_pi(void)
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


static void m68k_op_addq_16_pd(void)
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


static void m68k_op_addq_16_di(void)
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


static void m68k_op_addq_16_ix(void)
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


static void m68k_op_addq_16_aw(void)
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


static void m68k_op_addq_16_al(void)
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


static void m68k_op_addq_32_d(void)
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


static void m68k_op_addq_32_a(void)
{
  uint* r_dst = &AY;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst + (((REG_IR >> 9) - 1) & 7) + 1);
}


static void m68k_op_addq_32_ai(void)
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


static void m68k_op_addq_32_pi(void)
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


static void m68k_op_addq_32_pd(void)
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


static void m68k_op_addq_32_di(void)
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


static void m68k_op_addq_32_ix(void)
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


static void m68k_op_addq_32_aw(void)
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


static void m68k_op_addq_32_al(void)
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


static void m68k_op_addx_8_rr(void)
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


static void m68k_op_addx_16_rr(void)
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


static void m68k_op_addx_32_rr(void)
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


static void m68k_op_addx_8_mm_ax7(void)
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


static void m68k_op_addx_8_mm_ay7(void)
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


static void m68k_op_addx_8_mm_axy7(void)
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


static void m68k_op_addx_8_mm(void)
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


static void m68k_op_addx_16_mm(void)
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


static void m68k_op_addx_32_mm(void)
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


static void m68k_op_and_8_er_d(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (DY | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_ai(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_AI_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_pi(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_PI_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_pi7(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_A7_PI_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_pd(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_PD_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_pd7(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_A7_PD_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_di(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_DI_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_ix(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AY_IX_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_aw(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AW_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_al(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_AL_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_pcdi(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_PCDI_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_pcix(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_PCIX_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_er_i(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DX &= (OPER_I_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_d(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (DY | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_ai(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_AI_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_pi(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_PI_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_pd(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_PD_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_di(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_DI_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_ix(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AY_IX_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_aw(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AW_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_al(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_AL_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_pcdi(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_PCDI_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_pcix(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_PCIX_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_16_er_i(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DX &= (OPER_I_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_d(void)
{
  FLAG_Z = DX &= DY;

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_ai(void)
{
  FLAG_Z = DX &= OPER_AY_AI_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_pi(void)
{
  FLAG_Z = DX &= OPER_AY_PI_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_pd(void)
{
  FLAG_Z = DX &= OPER_AY_PD_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_di(void)
{
  FLAG_Z = DX &= OPER_AY_DI_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_ix(void)
{
  FLAG_Z = DX &= OPER_AY_IX_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_aw(void)
{
  FLAG_Z = DX &= OPER_AW_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_al(void)
{
  FLAG_Z = DX &= OPER_AL_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_pcdi(void)
{
  FLAG_Z = DX &= OPER_PCDI_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_pcix(void)
{
  FLAG_Z = DX &= OPER_PCIX_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_32_er_i(void)
{
  FLAG_Z = DX &= OPER_I_32();

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_and_8_re_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_8_re_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_8_re_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_8_re_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_8_re_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_8_re_di(void)
{
  uint ea = EA_AY_DI_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_8_re_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_8_re_aw(void)
{
  uint ea = EA_AW_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_8_re_al(void)
{
  uint ea = EA_AL_8();
  uint res = DX & m68ki_read_8(ea);

  FLAG_N = NFLAG_8(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_and_16_re_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint res = DX & m68ki_read_16(ea);

  FLAG_N = NFLAG_16(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_and_16_re_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint res = DX & m68ki_read_16(ea);

  FLAG_N = NFLAG_16(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_and_16_re_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint res = DX & m68ki_read_16(ea);

  FLAG_N = NFLAG_16(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_and_16_re_di(void)
{
  uint ea = EA_AY_DI_16();
  uint res = DX & m68ki_read_16(ea);

  FLAG_N = NFLAG_16(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_and_16_re_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint res = DX & m68ki_read_16(ea);

  FLAG_N = NFLAG_16(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_and_16_re_aw(void)
{
  uint ea = EA_AW_16();
  uint res = DX & m68ki_read_16(ea);

  FLAG_N = NFLAG_16(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_and_16_re_al(void)
{
  uint ea = EA_AL_16();
  uint res = DX & m68ki_read_16(ea);

  FLAG_N = NFLAG_16(res);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_and_32_re_ai(void)
{
  uint ea = EA_AY_AI_32();
  uint res = DX & m68ki_read_32(ea);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_and_32_re_pi(void)
{
  uint ea = EA_AY_PI_32();
  uint res = DX & m68ki_read_32(ea);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_and_32_re_pd(void)
{
  uint ea = EA_AY_PD_32();
  uint res = DX & m68ki_read_32(ea);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_and_32_re_di(void)
{
  uint ea = EA_AY_DI_32();
  uint res = DX & m68ki_read_32(ea);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_and_32_re_ix(void)
{
  uint ea = EA_AY_IX_32();
  uint res = DX & m68ki_read_32(ea);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_and_32_re_aw(void)
{
  uint ea = EA_AW_32();
  uint res = DX & m68ki_read_32(ea);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_and_32_re_al(void)
{
  uint ea = EA_AL_32();
  uint res = DX & m68ki_read_32(ea);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_andi_8_d(void)
{
  FLAG_Z = MASK_OUT_ABOVE_8(DY &= (OPER_I_8() | 0xffffff00));

  FLAG_N = NFLAG_8(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_andi_8_ai(void)
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


static void m68k_op_andi_8_pi(void)
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


static void m68k_op_andi_8_pi7(void)
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


static void m68k_op_andi_8_pd(void)
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


static void m68k_op_andi_8_pd7(void)
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


static void m68k_op_andi_8_di(void)
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


static void m68k_op_andi_8_ix(void)
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


static void m68k_op_andi_8_aw(void)
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


static void m68k_op_andi_8_al(void)
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


static void m68k_op_andi_16_d(void)
{
  FLAG_Z = MASK_OUT_ABOVE_16(DY &= (OPER_I_16() | 0xffff0000));

  FLAG_N = NFLAG_16(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_andi_16_ai(void)
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


static void m68k_op_andi_16_pi(void)
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


static void m68k_op_andi_16_pd(void)
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


static void m68k_op_andi_16_di(void)
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


static void m68k_op_andi_16_ix(void)
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


static void m68k_op_andi_16_aw(void)
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


static void m68k_op_andi_16_al(void)
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


static void m68k_op_andi_32_d(void)
{
  FLAG_Z = DY &= (OPER_I_32());

  FLAG_N = NFLAG_32(FLAG_Z);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_andi_32_ai(void)
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


static void m68k_op_andi_32_pi(void)
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


static void m68k_op_andi_32_pd(void)
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


static void m68k_op_andi_32_di(void)
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


static void m68k_op_andi_32_ix(void)
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


static void m68k_op_andi_32_aw(void)
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


static void m68k_op_andi_32_al(void)
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


static void m68k_op_andi_16_toc(void)
{
  m68ki_set_ccr(m68ki_get_ccr() & OPER_I_16());
}


static void m68k_op_andi_16_tos(void)
{
  if(FLAG_S)
  {
    uint src = OPER_I_16();
    m68ki_set_sr(m68ki_get_sr() & src);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_asr_8_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = src >> shift;

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  if(GET_MSB_8(src))
    res |= m68ki_shift_8_table[shift];

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_X = FLAG_C = src << (9-shift);
}


static void m68k_op_asr_16_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = src >> shift;

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  if(GET_MSB_16(src))
    res |= m68ki_shift_16_table[shift];

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_X = FLAG_C = src << (9-shift);
}


static void m68k_op_asr_32_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = *r_dst;
  uint res = src >> shift;

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  if(GET_MSB_32(src))
    res |= m68ki_shift_32_table[shift];

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_X = FLAG_C = src << (9-shift);
}


static void m68k_op_asr_8_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = src >> shift;

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

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


static void m68k_op_asr_16_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = src >> shift;

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

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


static void m68k_op_asr_32_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = *r_dst;
  uint res = src >> shift;

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

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


static void m68k_op_asr_16_ai(void)
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


static void m68k_op_asr_16_pi(void)
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


static void m68k_op_asr_16_pd(void)
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


static void m68k_op_asr_16_di(void)
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


static void m68k_op_asr_16_ix(void)
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


static void m68k_op_asr_16_aw(void)
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


static void m68k_op_asr_16_al(void)
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


static void m68k_op_asl_8_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = MASK_OUT_ABOVE_8(src << shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_X = FLAG_C = src << shift;
  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  src &= m68ki_shift_8_table[shift + 1];
  FLAG_V = (!(src == 0 || (src == m68ki_shift_8_table[shift + 1] && shift < 8)))<<7;
}


static void m68k_op_asl_16_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = MASK_OUT_ABOVE_16(src << shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> (8-shift);
  src &= m68ki_shift_16_table[shift + 1];
  FLAG_V = (!(src == 0 || src == m68ki_shift_16_table[shift + 1]))<<7;
}


static void m68k_op_asl_32_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = *r_dst;
  uint res = MASK_OUT_ABOVE_32(src << shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> (24-shift);
  src &= m68ki_shift_32_table[shift + 1];
  FLAG_V = (!(src == 0 || src == m68ki_shift_32_table[shift + 1]))<<7;
}


static void m68k_op_asl_8_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = MASK_OUT_ABOVE_8(src << shift);

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

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


static void m68k_op_asl_16_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = MASK_OUT_ABOVE_16(src << shift);

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

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


static void m68k_op_asl_32_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = *r_dst;
  uint res = MASK_OUT_ABOVE_32(src << shift);

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

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


static void m68k_op_asl_16_ai(void)
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


static void m68k_op_asl_16_pi(void)
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


static void m68k_op_asl_16_pd(void)
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


static void m68k_op_asl_16_di(void)
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


static void m68k_op_asl_16_ix(void)
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


static void m68k_op_asl_16_aw(void)
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


static void m68k_op_asl_16_al(void)
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


static void m68k_op_bhi_8(void)
{
  if(COND_HI())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bls_8(void)
{
  if(COND_LS())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bcc_8(void)
{
  if(COND_CC())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bcs_8(void)
{
  if(COND_CS())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bne_8(void)
{
  if(COND_NE())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_beq_8(void)
{
  if(COND_EQ())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bvc_8(void)
{
  if(COND_VC())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bvs_8(void)
{
  if(COND_VS())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bpl_8(void)
{
  if(COND_PL())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bmi_8(void)
{
  if(COND_MI())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bge_8(void)
{
  if(COND_GE())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_blt_8(void)
{
  if(COND_LT())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bgt_8(void)
{
  if(COND_GT())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_ble_8(void)
{
  if(COND_LE())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bhi_16(void)
{
  if(COND_HI())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bls_16(void)
{
  if(COND_LS())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bcc_16(void)
{
  if(COND_CC())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bcs_16(void)
{
  if(COND_CS())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bne_16(void)
{
  if(COND_NE())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_beq_16(void)
{
  if(COND_EQ())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bvc_16(void)
{
  if(COND_VC())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bvs_16(void)
{
  if(COND_VS())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bpl_16(void)
{
  if(COND_PL())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bmi_16(void)
{
  if(COND_MI())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bge_16(void)
{
  if(COND_GE())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_blt_16(void)
{
  if(COND_LT())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bgt_16(void)
{
  if(COND_GT())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_ble_16(void)
{
  if(COND_LE())
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_BCC_NOTAKE_W);
}


static void m68k_op_bhi_32(void)
{
  if(COND_HI())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bls_32(void)
{
  if(COND_LS())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bcc_32(void)
{
  if(COND_CC())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bcs_32(void)
{
  if(COND_CS())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bne_32(void)
{
  if(COND_NE())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_beq_32(void)
{
  if(COND_EQ())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bvc_32(void)
{
  if(COND_VC())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bvs_32(void)
{
  if(COND_VS())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bpl_32(void)
{
  if(COND_PL())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bmi_32(void)
{
  if(COND_MI())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bge_32(void)
{
  if(COND_GE())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_blt_32(void)
{
  if(COND_LT())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bgt_32(void)
{
  if(COND_GT())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_ble_32(void)
{
  if(COND_LE())
  {
    m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
    return;
  }
  USE_CYCLES(CYC_BCC_NOTAKE_B);
}


static void m68k_op_bchg_32_r_d(void)
{
  uint* r_dst = &DY;
  uint mask = 1 << (DX & 0x1f);

  FLAG_Z = *r_dst & mask;
  *r_dst ^= mask;
}


static void m68k_op_bchg_8_r_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_r_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_r_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_r_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_r_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_r_di(void)
{
  uint ea = EA_AY_DI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_r_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_r_aw(void)
{
  uint ea = EA_AW_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_r_al(void)
{
  uint ea = EA_AL_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_32_s_d(void)
{
  uint* r_dst = &DY;
  uint mask = 1 << (OPER_I_8() & 0x1f);

  FLAG_Z = *r_dst & mask;
  *r_dst ^= mask;
}


static void m68k_op_bchg_8_s_ai(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_AI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_s_pi(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_PI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_s_pi7(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_A7_PI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_s_pd(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_PD_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_s_pd7(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_A7_PD_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_s_di(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_DI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_s_ix(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_IX_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_s_aw(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AW_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bchg_8_s_al(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AL_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src ^ mask);
}


static void m68k_op_bclr_32_r_d(void)
{
  uint* r_dst = &DY;
  uint mask = 1 << (DX & 0x1f);

  FLAG_Z = *r_dst & mask;
  *r_dst &= ~mask;
}


static void m68k_op_bclr_8_r_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_r_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_r_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_r_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_r_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_r_di(void)
{
  uint ea = EA_AY_DI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_r_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_r_aw(void)
{
  uint ea = EA_AW_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_r_al(void)
{
  uint ea = EA_AL_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_32_s_d(void)
{
  uint* r_dst = &DY;
  uint mask = 1 << (OPER_I_8() & 0x1f);

  FLAG_Z = *r_dst & mask;
  *r_dst &= ~mask;
}


static void m68k_op_bclr_8_s_ai(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_AI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_s_pi(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_PI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_s_pi7(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_A7_PI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_s_pd(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_PD_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_s_pd7(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_A7_PD_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_s_di(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_DI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_s_ix(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_IX_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_s_aw(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AW_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bclr_8_s_al(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AL_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src & ~mask);
}


static void m68k_op_bra_8(void)
{
  m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
}


static void m68k_op_bra_16(void)
{
  uint offset = OPER_I_16();
  REG_PC -= 2;
  m68ki_branch_16(offset);
}


static void m68k_op_bra_32(void)
{
  m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
}


static void m68k_op_bset_32_r_d(void)
{
  uint* r_dst = &DY;
  uint mask = 1 << (DX & 0x1f);

  FLAG_Z = *r_dst & mask;
  *r_dst |= mask;
}


static void m68k_op_bset_8_r_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_r_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_r_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_r_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_r_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_r_di(void)
{
  uint ea = EA_AY_DI_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_r_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_r_aw(void)
{
  uint ea = EA_AW_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_r_al(void)
{
  uint ea = EA_AL_8();
  uint src = m68ki_read_8(ea);
  uint mask = 1 << (DX & 7);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_32_s_d(void)
{
  uint* r_dst = &DY;
  uint mask = 1 << (OPER_I_8() & 0x1f);

  FLAG_Z = *r_dst & mask;
  *r_dst |= mask;
}


static void m68k_op_bset_8_s_ai(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_AI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_s_pi(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_PI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_s_pi7(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_A7_PI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_s_pd(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_PD_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_s_pd7(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_A7_PD_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_s_di(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_DI_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_s_ix(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AY_IX_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_s_aw(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AW_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bset_8_s_al(void)
{
  uint mask = 1 << (OPER_I_8() & 7);
  uint ea = EA_AL_8();
  uint src = m68ki_read_8(ea);

  FLAG_Z = src & mask;
  m68ki_write_8(ea, src | mask);
}


static void m68k_op_bsr_8(void)
{
  m68ki_push_32(REG_PC);
  m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
}


static void m68k_op_bsr_16(void)
{
  uint offset = OPER_I_16();
  m68ki_push_32(REG_PC);
  REG_PC -= 2;
  m68ki_branch_16(offset);
}


static void m68k_op_bsr_32(void)
{
  m68ki_push_32(REG_PC);
  m68ki_branch_8(MASK_OUT_ABOVE_8(REG_IR));
}


static void m68k_op_btst_32_r_d(void)
{
  FLAG_Z = DY & (1 << (DX & 0x1f));
}


static void m68k_op_btst_8_r_ai(void)
{
  FLAG_Z = OPER_AY_AI_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_pi(void)
{
  FLAG_Z = OPER_AY_PI_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_pi7(void)
{
  FLAG_Z = OPER_A7_PI_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_pd(void)
{
  FLAG_Z = OPER_AY_PD_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_pd7(void)
{
  FLAG_Z = OPER_A7_PD_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_di(void)
{
  FLAG_Z = OPER_AY_DI_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_ix(void)
{
  FLAG_Z = OPER_AY_IX_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_aw(void)
{
  FLAG_Z = OPER_AW_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_al(void)
{
  FLAG_Z = OPER_AL_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_pcdi(void)
{
  FLAG_Z = OPER_PCDI_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_pcix(void)
{
  FLAG_Z = OPER_PCIX_8() & (1 << (DX & 7));
}


static void m68k_op_btst_8_r_i(void)
{
  FLAG_Z = OPER_I_8() & (1 << (DX & 7));
}


static void m68k_op_btst_32_s_d(void)
{
  FLAG_Z = DY & (1 << (OPER_I_8() & 0x1f));
}


static void m68k_op_btst_8_s_ai(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_AY_AI_8() & (1 << bit);
}


static void m68k_op_btst_8_s_pi(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_AY_PI_8() & (1 << bit);
}


static void m68k_op_btst_8_s_pi7(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_A7_PI_8() & (1 << bit);
}


static void m68k_op_btst_8_s_pd(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_AY_PD_8() & (1 << bit);
}


static void m68k_op_btst_8_s_pd7(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_A7_PD_8() & (1 << bit);
}


static void m68k_op_btst_8_s_di(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_AY_DI_8() & (1 << bit);
}


static void m68k_op_btst_8_s_ix(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_AY_IX_8() & (1 << bit);
}


static void m68k_op_btst_8_s_aw(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_AW_8() & (1 << bit);
}


static void m68k_op_btst_8_s_al(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_AL_8() & (1 << bit);
}


static void m68k_op_btst_8_s_pcdi(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_PCDI_8() & (1 << bit);
}


static void m68k_op_btst_8_s_pcix(void)
{
  uint bit = OPER_I_8() & 7;

  FLAG_Z = OPER_PCIX_8() & (1 << bit);
}


static void m68k_op_chk_16_d(void)
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


static void m68k_op_chk_16_ai(void)
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


static void m68k_op_chk_16_pi(void)
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


static void m68k_op_chk_16_pd(void)
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


static void m68k_op_chk_16_di(void)
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


static void m68k_op_chk_16_ix(void)
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


static void m68k_op_chk_16_aw(void)
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


static void m68k_op_chk_16_al(void)
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


static void m68k_op_chk_16_pcdi(void)
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


static void m68k_op_chk_16_pcix(void)
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


static void m68k_op_chk_16_i(void)
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


static void m68k_op_clr_8_d(void)
{
  DY &= 0xffffff00;

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_8_al(void)
{
  m68ki_write_8(EA_AL_8(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_16_d(void)
{
  DY &= 0xffff0000;

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_16_ai(void)
{
  m68ki_write_16(EA_AY_AI_16(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_16_pi(void)
{
  m68ki_write_16(EA_AY_PI_16(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_16_pd(void)
{
  m68ki_write_16(EA_AY_PD_16(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_16_di(void)
{
  m68ki_write_16(EA_AY_DI_16(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_16_ix(void)
{
  m68ki_write_16(EA_AY_IX_16(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_16_aw(void)
{
  m68ki_write_16(EA_AW_16(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_16_al(void)
{
  m68ki_write_16(EA_AL_16(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_32_d(void)
{
  DY = 0;

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_32_ai(void)
{
  m68ki_write_32(EA_AY_AI_32(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_32_pi(void)
{
  m68ki_write_32(EA_AY_PI_32(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_32_pd(void)
{
  m68ki_write_32(EA_AY_PD_32(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_32_di(void)
{
  m68ki_write_32(EA_AY_DI_32(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_32_ix(void)
{
  m68ki_write_32(EA_AY_IX_32(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_32_aw(void)
{
  m68ki_write_32(EA_AW_32(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_clr_32_al(void)
{
  m68ki_write_32(EA_AL_32(), 0);

  FLAG_N = NFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  FLAG_Z = ZFLAG_SET;
}


static void m68k_op_cmp_8_d(void)
{
  uint src = MASK_OUT_ABOVE_8(DY);
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_ai(void)
{
  uint src = OPER_AY_AI_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_pi(void)
{
  uint src = OPER_AY_PI_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_pi7(void)
{
  uint src = OPER_A7_PI_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_pd(void)
{
  uint src = OPER_AY_PD_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_pd7(void)
{
  uint src = OPER_A7_PD_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_di(void)
{
  uint src = OPER_AY_DI_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_ix(void)
{
  uint src = OPER_AY_IX_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_aw(void)
{
  uint src = OPER_AW_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_al(void)
{
  uint src = OPER_AL_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_pcdi(void)
{
  uint src = OPER_PCDI_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_pcix(void)
{
  uint src = OPER_PCIX_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_8_i(void)
{
  uint src = OPER_I_8();
  uint dst = MASK_OUT_ABOVE_8(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmp_16_d(void)
{
  uint src = MASK_OUT_ABOVE_16(DY);
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_a(void)
{
  uint src = MASK_OUT_ABOVE_16(AY);
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_ai(void)
{
  uint src = OPER_AY_AI_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_pi(void)
{
  uint src = OPER_AY_PI_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_pd(void)
{
  uint src = OPER_AY_PD_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_di(void)
{
  uint src = OPER_AY_DI_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_ix(void)
{
  uint src = OPER_AY_IX_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_aw(void)
{
  uint src = OPER_AW_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_al(void)
{
  uint src = OPER_AL_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_pcdi(void)
{
  uint src = OPER_PCDI_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_pcix(void)
{
  uint src = OPER_PCIX_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_16_i(void)
{
  uint src = OPER_I_16();
  uint dst = MASK_OUT_ABOVE_16(DX);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmp_32_d(void)
{
  uint src = DY;
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_a(void)
{
  uint src = AY;
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_ai(void)
{
  uint src = OPER_AY_AI_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_pi(void)
{
  uint src = OPER_AY_PI_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_pd(void)
{
  uint src = OPER_AY_PD_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_di(void)
{
  uint src = OPER_AY_DI_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_ix(void)
{
  uint src = OPER_AY_IX_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_aw(void)
{
  uint src = OPER_AW_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_al(void)
{
  uint src = OPER_AL_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_pcdi(void)
{
  uint src = OPER_PCDI_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_pcix(void)
{
  uint src = OPER_PCIX_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmp_32_i(void)
{
  uint src = OPER_I_32();
  uint dst = DX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_d(void)
{
  uint src = MAKE_INT_16(DY);
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_a(void)
{
  uint src = MAKE_INT_16(AY);
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_ai(void)
{
  uint src = MAKE_INT_16(OPER_AY_AI_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_pi(void)
{
  uint src = MAKE_INT_16(OPER_AY_PI_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_pd(void)
{
  uint src = MAKE_INT_16(OPER_AY_PD_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_di(void)
{
  uint src = MAKE_INT_16(OPER_AY_DI_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_ix(void)
{
  uint src = MAKE_INT_16(OPER_AY_IX_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_aw(void)
{
  uint src = MAKE_INT_16(OPER_AW_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_al(void)
{
  uint src = MAKE_INT_16(OPER_AL_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_pcdi(void)
{
  uint src = MAKE_INT_16(OPER_PCDI_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_pcix(void)
{
  uint src = MAKE_INT_16(OPER_PCIX_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_16_i(void)
{
  uint src = MAKE_INT_16(OPER_I_16());
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_d(void)
{
  uint src = DY;
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_a(void)
{
  uint src = AY;
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_ai(void)
{
  uint src = OPER_AY_AI_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_pi(void)
{
  uint src = OPER_AY_PI_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_pd(void)
{
  uint src = OPER_AY_PD_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_di(void)
{
  uint src = OPER_AY_DI_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_ix(void)
{
  uint src = OPER_AY_IX_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_aw(void)
{
  uint src = OPER_AW_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_al(void)
{
  uint src = OPER_AL_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_pcdi(void)
{
  uint src = OPER_PCDI_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_pcix(void)
{
  uint src = OPER_PCIX_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpa_32_i(void)
{
  uint src = OPER_I_32();
  uint dst = AX;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpi_8_d(void)
{
  uint src = OPER_I_8();
  uint dst = MASK_OUT_ABOVE_8(DY);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_ai(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_AY_AI_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_pi(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_AY_PI_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_pi7(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_A7_PI_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_pd(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_AY_PD_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_pd7(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_A7_PD_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_di(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_AY_DI_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_ix(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_AY_IX_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_aw(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_AW_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_8_al(void)
{
  uint src = OPER_I_8();
  uint dst = OPER_AL_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpi_16_d(void)
{
  uint src = OPER_I_16();
  uint dst = MASK_OUT_ABOVE_16(DY);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpi_16_ai(void)
{
  uint src = OPER_I_16();
  uint dst = OPER_AY_AI_16();
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpi_16_pi(void)
{
  uint src = OPER_I_16();
  uint dst = OPER_AY_PI_16();
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpi_16_pd(void)
{
  uint src = OPER_I_16();
  uint dst = OPER_AY_PD_16();
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpi_16_di(void)
{
  uint src = OPER_I_16();
  uint dst = OPER_AY_DI_16();
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpi_16_ix(void)
{
  uint src = OPER_I_16();
  uint dst = OPER_AY_IX_16();
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpi_16_aw(void)
{
  uint src = OPER_I_16();
  uint dst = OPER_AW_16();
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpi_16_al(void)
{
  uint src = OPER_I_16();
  uint dst = OPER_AL_16();
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpi_32_d(void)
{
  uint src = OPER_I_32();
  uint dst = DY;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpi_32_ai(void)
{
  uint src = OPER_I_32();
  uint dst = OPER_AY_AI_32();
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpi_32_pi(void)
{
  uint src = OPER_I_32();
  uint dst = OPER_AY_PI_32();
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpi_32_pd(void)
{
  uint src = OPER_I_32();
  uint dst = OPER_AY_PD_32();
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpi_32_di(void)
{
  uint src = OPER_I_32();
  uint dst = OPER_AY_DI_32();
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpi_32_ix(void)
{
  uint src = OPER_I_32();
  uint dst = OPER_AY_IX_32();
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpi_32_aw(void)
{
  uint src = OPER_I_32();
  uint dst = OPER_AW_32();
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpi_32_al(void)
{
  uint src = OPER_I_32();
  uint dst = OPER_AL_32();
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_cmpm_8_ax7(void)
{
  uint src = OPER_AY_PI_8();
  uint dst = OPER_A7_PI_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpm_8_ay7(void)
{
  uint src = OPER_A7_PI_8();
  uint dst = OPER_AX_PI_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpm_8_axy7(void)
{
  uint src = OPER_A7_PI_8();
  uint dst = OPER_A7_PI_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpm_8(void)
{
  uint src = OPER_AY_PI_8();
  uint dst = OPER_AX_PI_8();
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_C = CFLAG_8(res);
}


static void m68k_op_cmpm_16(void)
{
  uint src = OPER_AY_PI_16();
  uint dst = OPER_AX_PI_16();
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_C = CFLAG_16(res);
}


static void m68k_op_cmpm_32(void)
{
  uint src = OPER_AY_PI_32();
  uint dst = OPER_AX_PI_32();
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_C = CFLAG_SUB_32(src, dst, res);
}


static void m68k_op_dbt_16(void)
{
  REG_PC += 2;
}


static void m68k_op_dbf_16(void)
{
  uint* r_dst = &DY;
  uint res = MASK_OUT_ABOVE_16(*r_dst - 1);
  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  /* reset idle loop detection */
  m68ki_cpu.poll.detected = 0;

  if(res != 0xffff)
  {
    uint offset = OPER_I_16();
    REG_PC -= 2;
    m68ki_branch_16(offset);
    USE_CYCLES(CYC_DBCC_F_NOEXP);
    return;
  }
  REG_PC += 2;
  USE_CYCLES(CYC_DBCC_F_EXP);
}


static void m68k_op_dbhi_16(void)
{
  if(COND_NOT_HI())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbls_16(void)
{
  if(COND_NOT_LS())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbcc_16(void)
{
  if(COND_NOT_CC())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbcs_16(void)
{
  if(COND_NOT_CS())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbne_16(void)
{
  if(COND_NOT_NE())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbeq_16(void)
{
  if(COND_NOT_EQ())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbvc_16(void)
{
  if(COND_NOT_VC())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbvs_16(void)
{
  if(COND_NOT_VS())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbpl_16(void)
{
  if(COND_NOT_PL())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbmi_16(void)
{
  if(COND_NOT_MI())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbge_16(void)
{
  if(COND_NOT_GE())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dblt_16(void)
{
  if(COND_NOT_LT())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dbgt_16(void)
{
  if(COND_NOT_GT())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_dble_16(void)
{
  if(COND_NOT_LE())
  {
    uint* r_dst = &DY;
    uint res = MASK_OUT_ABOVE_16(*r_dst - 1);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    if(res != 0xffff)
    {
      uint offset = OPER_I_16();
      REG_PC -= 2;
      m68ki_branch_16(offset);
      USE_CYCLES(CYC_DBCC_F_NOEXP);
      return;
    }
    REG_PC += 2;
    USE_CYCLES(CYC_DBCC_F_EXP);
    return;
  }
  REG_PC += 2;
}


static void m68k_op_divs_16_d(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(DY);
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_ai(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_AI_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_pi(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_PI_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_pd(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_PD_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_di(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_DI_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_ix(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_IX_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_aw(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AW_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_al(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AL_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_pcdi(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_PCDI_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_pcix(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_PCIX_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divs_16_i(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_I_16());
  sint quotient;
  sint remainder;

  if(src != 0)
  {
    UseDivsCycles(*r_dst,src);

    if((uint32)*r_dst == 0x80000000 && src == -1)
    {
      FLAG_Z = 0;
      FLAG_N = NFLAG_CLEAR;
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = 0;
      return;
    }

    quotient = MAKE_INT_32(*r_dst) / src;
    remainder = MAKE_INT_32(*r_dst) % src;

    if(quotient == MAKE_INT_16(quotient))
    {
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_d(void)
{
  uint* r_dst = &DX;
  uint src = MASK_OUT_ABOVE_16(DY);

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_ai(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_AI_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_pi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PI_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 *10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_pd(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PD_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_di(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_DI_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_ix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_IX_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_aw(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AW_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_al(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AL_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_pcdi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCDI_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_pcix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCIX_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_divu_16_i(void)
{
  uint* r_dst = &DX;
  uint src = OPER_I_16();

  if(src != 0)
  {
    uint quotient = *r_dst / src;
    uint remainder = *r_dst % src;

    if(quotient < 0x10000)
    {
      UseDivuCycles(*r_dst,src);
      FLAG_Z = quotient;
      FLAG_N = NFLAG_16(quotient);
      FLAG_V = VFLAG_CLEAR;
      FLAG_C = CFLAG_CLEAR;
      *r_dst = MASK_OUT_ABOVE_32(MASK_OUT_ABOVE_16(quotient) | (remainder << 16));
      return;
    }
    USE_CYCLES(7 * 10);
    FLAG_V = VFLAG_SET;
    FLAG_N = NFLAG_SET; /* undocumented behavior (fixes Blood Shot on Genesis) */
    FLAG_C = CFLAG_CLEAR;
    return;
  }
  FLAG_C = CFLAG_CLEAR;
  m68ki_exception_trap(EXCEPTION_ZERO_DIVIDE);
}


static void m68k_op_eor_8_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY ^= MASK_OUT_ABOVE_8(DX));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_di(void)
{
  uint ea = EA_AY_DI_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_aw(void)
{
  uint ea = EA_AW_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_8_al(void)
{
  uint ea = EA_AL_8();
  uint res = MASK_OUT_ABOVE_8(DX ^ m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_16_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY ^= MASK_OUT_ABOVE_16(DX));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_16_aw(void)
{
  uint ea = EA_AW_16();
  uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_16_al(void)
{
  uint ea = EA_AL_16();
  uint res = MASK_OUT_ABOVE_16(DX ^ m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_32_d(void)
{
  uint res = DY ^= DX;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_32_ai(void)
{
  uint ea = EA_AY_AI_32();
  uint res = DX ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_32_pi(void)
{
  uint ea = EA_AY_PI_32();
  uint res = DX ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_32_pd(void)
{
  uint ea = EA_AY_PD_32();
  uint res = DX ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_32_di(void)
{
  uint ea = EA_AY_DI_32();
  uint res = DX ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_32_ix(void)
{
  uint ea = EA_AY_IX_32();
  uint res = DX ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_32_aw(void)
{
  uint ea = EA_AW_32();
  uint res = DX ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eor_32_al(void)
{
  uint ea = EA_AL_32();
  uint res = DX ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY ^= OPER_I_8());

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_ai(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_AI_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_pi(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_PI_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_pi7(void)
{
  uint src = OPER_I_8();
  uint ea = EA_A7_PI_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_pd(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_PD_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_pd7(void)
{
  uint src = OPER_I_8();
  uint ea = EA_A7_PD_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_di(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_DI_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_ix(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_IX_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_aw(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AW_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_8_al(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AL_8();
  uint res = src ^ m68ki_read_8(ea);

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY ^= OPER_I_16());

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_ai(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_AI_16();
  uint res = src ^ m68ki_read_16(ea);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_pi(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_PI_16();
  uint res = src ^ m68ki_read_16(ea);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_pd(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_PD_16();
  uint res = src ^ m68ki_read_16(ea);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_di(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_DI_16();
  uint res = src ^ m68ki_read_16(ea);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_ix(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_IX_16();
  uint res = src ^ m68ki_read_16(ea);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_aw(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AW_16();
  uint res = src ^ m68ki_read_16(ea);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_al(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AL_16();
  uint res = src ^ m68ki_read_16(ea);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_32_d(void)
{
  uint res = DY ^= OPER_I_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_32_ai(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_AI_32();
  uint res = src ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_32_pi(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_PI_32();
  uint res = src ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_32_pd(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_PD_32();
  uint res = src ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_32_di(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_DI_32();
  uint res = src ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_32_ix(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_IX_32();
  uint res = src ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_32_aw(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AW_32();
  uint res = src ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_32_al(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AL_32();
  uint res = src ^ m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_eori_16_toc(void)
{
  m68ki_set_ccr(m68ki_get_ccr() ^ OPER_I_16());
}


static void m68k_op_eori_16_tos(void)
{
  if(FLAG_S)
  {
    uint src = OPER_I_16();
    m68ki_set_sr(m68ki_get_sr() ^ src);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_exg_32_dd(void)
{
  uint* reg_a = &DX;
  uint* reg_b = &DY;
  uint tmp = *reg_a;
  *reg_a = *reg_b;
  *reg_b = tmp;
}


static void m68k_op_exg_32_aa(void)
{
  uint* reg_a = &AX;
  uint* reg_b = &AY;
  uint tmp = *reg_a;
  *reg_a = *reg_b;
  *reg_b = tmp;
}


static void m68k_op_exg_32_da(void)
{
  uint* reg_a = &DX;
  uint* reg_b = &AY;
  uint tmp = *reg_a;
  *reg_a = *reg_b;
  *reg_b = tmp;
}


static void m68k_op_ext_16(void)
{
  uint* r_dst = &DY;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | MASK_OUT_ABOVE_8(*r_dst) | (GET_MSB_8(*r_dst) ? 0xff00 : 0);

  FLAG_N = NFLAG_16(*r_dst);
  FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_ext_32(void)
{
  uint* r_dst = &DY;

  *r_dst = MASK_OUT_ABOVE_16(*r_dst) | (GET_MSB_16(*r_dst) ? 0xffff0000 : 0);

  FLAG_N = NFLAG_32(*r_dst);
  FLAG_Z = *r_dst;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_illegal(void)
{
  m68ki_exception_illegal();
}


static void m68k_op_jmp_32_ai(void)
{
  m68ki_jump(EA_AY_AI_32());
}


static void m68k_op_jmp_32_di(void)
{
  m68ki_jump(EA_AY_DI_32());
}


static void m68k_op_jmp_32_ix(void)
{
  m68ki_jump(EA_AY_IX_32());
}


static void m68k_op_jmp_32_aw(void)
{
  m68ki_jump(EA_AW_32());
}


static void m68k_op_jmp_32_al(void)
{
  m68ki_jump(EA_AL_32());
}


static void m68k_op_jmp_32_pcdi(void)
{
  m68ki_jump(EA_PCDI_32());
}


static void m68k_op_jmp_32_pcix(void)
{
  m68ki_jump(EA_PCIX_32());
}


static void m68k_op_jsr_32_ai(void)
{
  uint ea = EA_AY_AI_32();
  m68ki_push_32(REG_PC);
  m68ki_jump(ea);
}


static void m68k_op_jsr_32_di(void)
{
  uint ea = EA_AY_DI_32();
  m68ki_push_32(REG_PC);
  m68ki_jump(ea);
}


static void m68k_op_jsr_32_ix(void)
{
  uint ea = EA_AY_IX_32();
  m68ki_push_32(REG_PC);
  m68ki_jump(ea);
}


static void m68k_op_jsr_32_aw(void)
{
  uint ea = EA_AW_32();
  m68ki_push_32(REG_PC);
  m68ki_jump(ea);
}


static void m68k_op_jsr_32_al(void)
{
  uint ea = EA_AL_32();
  m68ki_push_32(REG_PC);
  m68ki_jump(ea);
}


static void m68k_op_jsr_32_pcdi(void)
{
  uint ea = EA_PCDI_32();
  m68ki_push_32(REG_PC);
  m68ki_jump(ea);
}


static void m68k_op_jsr_32_pcix(void)
{
  uint ea = EA_PCIX_32();
  m68ki_push_32(REG_PC);
  m68ki_jump(ea);
}


static void m68k_op_lea_32_ai(void)
{
  AX = EA_AY_AI_32();
}


static void m68k_op_lea_32_di(void)
{
  AX = EA_AY_DI_32();
}


static void m68k_op_lea_32_ix(void)
{
  AX = EA_AY_IX_32();
}


static void m68k_op_lea_32_aw(void)
{
  AX = EA_AW_32();
}


static void m68k_op_lea_32_al(void)
{
  AX = EA_AL_32();
}


static void m68k_op_lea_32_pcdi(void)
{
  AX = EA_PCDI_32();
}


static void m68k_op_lea_32_pcix(void)
{
  AX = EA_PCIX_32();
}


static void m68k_op_link_16_a7(void)
{
  REG_A[7] -= 4;
  m68ki_write_32(REG_A[7], REG_A[7]);
  REG_A[7] = MASK_OUT_ABOVE_32(REG_A[7] + MAKE_INT_16(OPER_I_16()));
}


static void m68k_op_link_16(void)
{
  uint* r_dst = &AY;

  m68ki_push_32(*r_dst);
  *r_dst = REG_A[7];
  REG_A[7] = MASK_OUT_ABOVE_32(REG_A[7] + MAKE_INT_16(OPER_I_16()));
}


static void m68k_op_lsr_8_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = src >> shift;

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_X = FLAG_C = src << (9-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_16_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = src >> shift;

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_X = FLAG_C = src << (9-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_32_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = *r_dst;
  uint res = src >> shift;

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = res;

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_X = FLAG_C = src << (9-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_8_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = src >> shift;

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

    if(shift <= 8)
    {
      *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
      FLAG_X = FLAG_C = src << (9-shift);
      FLAG_N = NFLAG_CLEAR;
      FLAG_Z = res;
      FLAG_V = VFLAG_CLEAR;
      return;
    }

    *r_dst &= 0xffffff00;
    FLAG_X = XFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
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


static void m68k_op_lsr_16_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = src >> shift;

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

    if(shift <= 16)
    {
      *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
      FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
      FLAG_N = NFLAG_CLEAR;
      FLAG_Z = res;
      FLAG_V = VFLAG_CLEAR;
      return;
    }

    *r_dst &= 0xffff0000;
    FLAG_X = XFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
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


static void m68k_op_lsr_32_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = *r_dst;
  uint res = src >> shift;

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

    if(shift < 32)
    {
      *r_dst = res;
      FLAG_C = FLAG_X = (src >> (shift - 1))<<8;
      FLAG_N = NFLAG_CLEAR;
      FLAG_Z = res;
      FLAG_V = VFLAG_CLEAR;
      return;
    }

    *r_dst = 0;
    FLAG_X = FLAG_C = (shift == 32 ? GET_MSB_32(src)>>23 : 0);
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


static void m68k_op_lsr_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint src = m68ki_read_16(ea);
  uint res = src >> 1;

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_C = FLAG_X = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint src = m68ki_read_16(ea);
  uint res = src >> 1;

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_C = FLAG_X = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint src = m68ki_read_16(ea);
  uint res = src >> 1;

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_C = FLAG_X = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint src = m68ki_read_16(ea);
  uint res = src >> 1;

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_C = FLAG_X = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint src = m68ki_read_16(ea);
  uint res = src >> 1;

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_C = FLAG_X = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_16_aw(void)
{
  uint ea = EA_AW_16();
  uint src = m68ki_read_16(ea);
  uint res = src >> 1;

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_C = FLAG_X = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsr_16_al(void)
{
  uint ea = EA_AL_16();
  uint src = m68ki_read_16(ea);
  uint res = src >> 1;

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_CLEAR;
  FLAG_Z = res;
  FLAG_C = FLAG_X = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_8_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = MASK_OUT_ABOVE_8(src << shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src << shift;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_16_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = MASK_OUT_ABOVE_16(src << shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> (8-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_32_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = *r_dst;
  uint res = MASK_OUT_ABOVE_32(src << shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> (24-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_8_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = MASK_OUT_ABOVE_8(src << shift);

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

    if(shift <= 8)
    {
      *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
      FLAG_X = FLAG_C = src << shift;
      FLAG_N = NFLAG_8(res);
      FLAG_Z = res;
      FLAG_V = VFLAG_CLEAR;
      return;
    }

    *r_dst &= 0xffffff00;
    FLAG_X = XFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
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


static void m68k_op_lsl_16_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = MASK_OUT_ABOVE_16(src << shift);

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

    if(shift <= 16)
    {
      *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
      FLAG_X = FLAG_C = (src << shift) >> 8;
      FLAG_N = NFLAG_16(res);
      FLAG_Z = res;
      FLAG_V = VFLAG_CLEAR;
      return;
    }

    *r_dst &= 0xffff0000;
    FLAG_X = XFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
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


static void m68k_op_lsl_32_r(void)
{
  uint* r_dst = &DY;
  uint shift = DX & 0x3f;
  uint src = *r_dst;
  uint res = MASK_OUT_ABOVE_32(src << shift);

  if(shift != 0)
  {
    USE_CYCLES(shift * CYC_SHIFT);

    if(shift < 32)
    {
      *r_dst = res;
      FLAG_X = FLAG_C = (src >> (32 - shift)) << 8;
      FLAG_N = NFLAG_32(res);
      FLAG_Z = res;
      FLAG_V = VFLAG_CLEAR;
      return;
    }

    *r_dst = 0;
    FLAG_X = FLAG_C = ((shift == 32 ? src & 1 : 0))<<8;
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


static void m68k_op_lsl_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(src << 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(src << 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(src << 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(src << 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(src << 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_16_aw(void)
{
  uint ea = EA_AW_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(src << 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_lsl_16_al(void)
{
  uint ea = EA_AL_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(src << 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_X = FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_move_8_d_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_di(void)
{
  uint res = OPER_AY_DI_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_aw(void)
{
  uint res = OPER_AW_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_al(void)
{
  uint res = OPER_AL_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_d_i(void)
{
  uint res = OPER_I_8();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_8_ai_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ai_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_AX_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi7_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pi_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_AX_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd7_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_pd_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_AX_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_di_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_AX_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_ix_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_AX_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_aw_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_ai(void)
{
  uint res = OPER_AY_AI_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_pi(void)
{
  uint res = OPER_AY_PI_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_pi7(void)
{
  uint res = OPER_A7_PI_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_pd(void)
{
  uint res = OPER_AY_PD_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_pd7(void)
{
  uint res = OPER_A7_PD_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_di(void)
{
  uint res = OPER_AY_DI_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_ix(void)
{
  uint res = OPER_AY_IX_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_aw(void)
{
  uint res = OPER_AW_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_al(void)
{
  uint res = OPER_AL_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_pcdi(void)
{
  uint res = OPER_PCDI_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_pcix(void)
{
  uint res = OPER_PCIX_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_8_al_i(void)
{
  uint res = OPER_I_8();
  uint ea = EA_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_8(ea, res);
}


static void m68k_op_move_16_d_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_a(void)
{
  uint res = MASK_OUT_ABOVE_16(AY);
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_ai(void)
{
  uint res = OPER_AY_AI_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_pi(void)
{
  uint res = OPER_AY_PI_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_pd(void)
{
  uint res = OPER_AY_PD_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_di(void)
{
  uint res = OPER_AY_DI_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_ix(void)
{
  uint res = OPER_AY_IX_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_aw(void)
{
  uint res = OPER_AW_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_al(void)
{
  uint res = OPER_AL_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_pcdi(void)
{
  uint res = OPER_PCDI_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_pcix(void)
{
  uint res = OPER_PCIX_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_d_i(void)
{
  uint res = OPER_I_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_16_ai_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_a(void)
{
  uint res = MASK_OUT_ABOVE_16(AY);
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_ai(void)
{
  uint res = OPER_AY_AI_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_pi(void)
{
  uint res = OPER_AY_PI_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_pd(void)
{
  uint res = OPER_AY_PD_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_di(void)
{
  uint res = OPER_AY_DI_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_ix(void)
{
  uint res = OPER_AY_IX_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_aw(void)
{
  uint res = OPER_AW_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_al(void)
{
  uint res = OPER_AL_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_pcdi(void)
{
  uint res = OPER_PCDI_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_pcix(void)
{
  uint res = OPER_PCIX_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ai_i(void)
{
  uint res = OPER_I_16();
  uint ea = EA_AX_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_a(void)
{
  uint res = MASK_OUT_ABOVE_16(AY);
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_ai(void)
{
  uint res = OPER_AY_AI_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_pi(void)
{
  uint res = OPER_AY_PI_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_pd(void)
{
  uint res = OPER_AY_PD_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_di(void)
{
  uint res = OPER_AY_DI_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_ix(void)
{
  uint res = OPER_AY_IX_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_aw(void)
{
  uint res = OPER_AW_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_al(void)
{
  uint res = OPER_AL_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_pcdi(void)
{
  uint res = OPER_PCDI_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_pcix(void)
{
  uint res = OPER_PCIX_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pi_i(void)
{
  uint res = OPER_I_16();
  uint ea = EA_AX_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_a(void)
{
  uint res = MASK_OUT_ABOVE_16(AY);
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_ai(void)
{
  uint res = OPER_AY_AI_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_pi(void)
{
  uint res = OPER_AY_PI_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_pd(void)
{
  uint res = OPER_AY_PD_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_di(void)
{
  uint res = OPER_AY_DI_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_ix(void)
{
  uint res = OPER_AY_IX_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_aw(void)
{
  uint res = OPER_AW_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_al(void)
{
  uint res = OPER_AL_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_pcdi(void)
{
  uint res = OPER_PCDI_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_pcix(void)
{
  uint res = OPER_PCIX_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_pd_i(void)
{
  uint res = OPER_I_16();
  uint ea = EA_AX_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_a(void)
{
  uint res = MASK_OUT_ABOVE_16(AY);
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_ai(void)
{
  uint res = OPER_AY_AI_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_pi(void)
{
  uint res = OPER_AY_PI_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_pd(void)
{
  uint res = OPER_AY_PD_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_di(void)
{
  uint res = OPER_AY_DI_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_ix(void)
{
  uint res = OPER_AY_IX_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_aw(void)
{
  uint res = OPER_AW_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_al(void)
{
  uint res = OPER_AL_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_pcdi(void)
{
  uint res = OPER_PCDI_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_pcix(void)
{
  uint res = OPER_PCIX_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_di_i(void)
{
  uint res = OPER_I_16();
  uint ea = EA_AX_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_a(void)
{
  uint res = MASK_OUT_ABOVE_16(AY);
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_ai(void)
{
  uint res = OPER_AY_AI_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_pi(void)
{
  uint res = OPER_AY_PI_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_pd(void)
{
  uint res = OPER_AY_PD_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_di(void)
{
  uint res = OPER_AY_DI_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_ix(void)
{
  uint res = OPER_AY_IX_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_aw(void)
{
  uint res = OPER_AW_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_al(void)
{
  uint res = OPER_AL_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_pcdi(void)
{
  uint res = OPER_PCDI_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_pcix(void)
{
  uint res = OPER_PCIX_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_ix_i(void)
{
  uint res = OPER_I_16();
  uint ea = EA_AX_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_a(void)
{
  uint res = MASK_OUT_ABOVE_16(AY);
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_ai(void)
{
  uint res = OPER_AY_AI_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_pi(void)
{
  uint res = OPER_AY_PI_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_pd(void)
{
  uint res = OPER_AY_PD_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_di(void)
{
  uint res = OPER_AY_DI_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_ix(void)
{
  uint res = OPER_AY_IX_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_aw(void)
{
  uint res = OPER_AW_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_al(void)
{
  uint res = OPER_AL_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_pcdi(void)
{
  uint res = OPER_PCDI_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_pcix(void)
{
  uint res = OPER_PCIX_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_aw_i(void)
{
  uint res = OPER_I_16();
  uint ea = EA_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_a(void)
{
  uint res = MASK_OUT_ABOVE_16(AY);
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_ai(void)
{
  uint res = OPER_AY_AI_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_pi(void)
{
  uint res = OPER_AY_PI_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_pd(void)
{
  uint res = OPER_AY_PD_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_di(void)
{
  uint res = OPER_AY_DI_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_ix(void)
{
  uint res = OPER_AY_IX_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_aw(void)
{
  uint res = OPER_AW_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_al(void)
{
  uint res = OPER_AL_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_pcdi(void)
{
  uint res = OPER_PCDI_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_pcix(void)
{
  uint res = OPER_PCIX_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_16_al_i(void)
{
  uint res = OPER_I_16();
  uint ea = EA_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea, res);
}


static void m68k_op_move_32_d_d(void)
{
  uint res = DY;
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_a(void)
{
  uint res = AY;
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_ai(void)
{
  uint res = OPER_AY_AI_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_pi(void)
{
  uint res = OPER_AY_PI_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_pd(void)
{
  uint res = OPER_AY_PD_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_di(void)
{
  uint res = OPER_AY_DI_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_ix(void)
{
  uint res = OPER_AY_IX_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_aw(void)
{
  uint res = OPER_AW_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_al(void)
{
  uint res = OPER_AL_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_pcdi(void)
{
  uint res = OPER_PCDI_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_pcix(void)
{
  uint res = OPER_PCIX_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_d_i(void)
{
  uint res = OPER_I_32();
  uint* r_dst = &DX;

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_move_32_ai_d(void)
{
  uint res = DY;
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_a(void)
{
  uint res = AY;
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_ai(void)
{
  uint res = OPER_AY_AI_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_pi(void)
{
  uint res = OPER_AY_PI_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_pd(void)
{
  uint res = OPER_AY_PD_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_di(void)
{
  uint res = OPER_AY_DI_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_ix(void)
{
  uint res = OPER_AY_IX_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_aw(void)
{
  uint res = OPER_AW_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_al(void)
{
  uint res = OPER_AL_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_pcdi(void)
{
  uint res = OPER_PCDI_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_pcix(void)
{
  uint res = OPER_PCIX_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ai_i(void)
{
  uint res = OPER_I_32();
  uint ea = EA_AX_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_d(void)
{
  uint res = DY;
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_a(void)
{
  uint res = AY;
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_ai(void)
{
  uint res = OPER_AY_AI_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_pi(void)
{
  uint res = OPER_AY_PI_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_pd(void)
{
  uint res = OPER_AY_PD_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_di(void)
{
  uint res = OPER_AY_DI_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_ix(void)
{
  uint res = OPER_AY_IX_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_aw(void)
{
  uint res = OPER_AW_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_al(void)
{
  uint res = OPER_AL_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_pcdi(void)
{
  uint res = OPER_PCDI_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_pcix(void)
{
  uint res = OPER_PCIX_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pi_i(void)
{
  uint res = OPER_I_32();
  uint ea = EA_AX_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_pd_d(void)
{
  uint res = DY;
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_a(void)
{
  uint res = AY;
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_ai(void)
{
  uint res = OPER_AY_AI_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_pi(void)
{
  uint res = OPER_AY_PI_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_pd(void)
{
  uint res = OPER_AY_PD_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_di(void)
{
  uint res = OPER_AY_DI_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_ix(void)
{
  uint res = OPER_AY_IX_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_aw(void)
{
  uint res = OPER_AW_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_al(void)
{
  uint res = OPER_AL_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_pcdi(void)
{
  uint res = OPER_PCDI_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_pcix(void)
{
  uint res = OPER_PCIX_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_pd_i(void)
{
  uint res = OPER_I_32();
  uint ea = EA_AX_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_16(ea+2, res & 0xFFFF );
  m68ki_write_16(ea, (res >> 16) & 0xFFFF );
}


static void m68k_op_move_32_di_d(void)
{
  uint res = DY;
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_a(void)
{
  uint res = AY;
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_ai(void)
{
  uint res = OPER_AY_AI_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_pi(void)
{
  uint res = OPER_AY_PI_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_pd(void)
{
  uint res = OPER_AY_PD_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_di(void)
{
  uint res = OPER_AY_DI_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_ix(void)
{
  uint res = OPER_AY_IX_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_aw(void)
{
  uint res = OPER_AW_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_al(void)
{
  uint res = OPER_AL_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_pcdi(void)
{
  uint res = OPER_PCDI_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_pcix(void)
{
  uint res = OPER_PCIX_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_di_i(void)
{
  uint res = OPER_I_32();
  uint ea = EA_AX_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_d(void)
{
  uint res = DY;
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_a(void)
{
  uint res = AY;
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_ai(void)
{
  uint res = OPER_AY_AI_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_pi(void)
{
  uint res = OPER_AY_PI_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_pd(void)
{
  uint res = OPER_AY_PD_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_di(void)
{
  uint res = OPER_AY_DI_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_ix(void)
{
  uint res = OPER_AY_IX_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_aw(void)
{
  uint res = OPER_AW_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_al(void)
{
  uint res = OPER_AL_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_pcdi(void)
{
  uint res = OPER_PCDI_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_pcix(void)
{
  uint res = OPER_PCIX_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_ix_i(void)
{
  uint res = OPER_I_32();
  uint ea = EA_AX_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_d(void)
{
  uint res = DY;
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_a(void)
{
  uint res = AY;
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_ai(void)
{
  uint res = OPER_AY_AI_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_pi(void)
{
  uint res = OPER_AY_PI_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_pd(void)
{
  uint res = OPER_AY_PD_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_di(void)
{
  uint res = OPER_AY_DI_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_ix(void)
{
  uint res = OPER_AY_IX_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_aw(void)
{
  uint res = OPER_AW_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_al(void)
{
  uint res = OPER_AL_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_pcdi(void)
{
  uint res = OPER_PCDI_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_pcix(void)
{
  uint res = OPER_PCIX_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_aw_i(void)
{
  uint res = OPER_I_32();
  uint ea = EA_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_d(void)
{
  uint res = DY;
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_a(void)
{
  uint res = AY;
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_ai(void)
{
  uint res = OPER_AY_AI_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_pi(void)
{
  uint res = OPER_AY_PI_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_pd(void)
{
  uint res = OPER_AY_PD_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_di(void)
{
  uint res = OPER_AY_DI_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_ix(void)
{
  uint res = OPER_AY_IX_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_aw(void)
{
  uint res = OPER_AW_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_al(void)
{
  uint res = OPER_AL_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_pcdi(void)
{
  uint res = OPER_PCDI_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_pcix(void)
{
  uint res = OPER_PCIX_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_move_32_al_i(void)
{
  uint res = OPER_I_32();
  uint ea = EA_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  m68ki_write_32(ea, res);
}


static void m68k_op_movea_16_d(void)
{
  AX = MAKE_INT_16(DY);
}


static void m68k_op_movea_16_a(void)
{
  AX = MAKE_INT_16(AY);
}


static void m68k_op_movea_16_ai(void)
{
  AX = MAKE_INT_16(OPER_AY_AI_16());
}


static void m68k_op_movea_16_pi(void)
{
  AX = MAKE_INT_16(OPER_AY_PI_16());
}


static void m68k_op_movea_16_pd(void)
{
  AX = MAKE_INT_16(OPER_AY_PD_16());
}


static void m68k_op_movea_16_di(void)
{
  AX = MAKE_INT_16(OPER_AY_DI_16());
}


static void m68k_op_movea_16_ix(void)
{
  AX = MAKE_INT_16(OPER_AY_IX_16());
}


static void m68k_op_movea_16_aw(void)
{
  AX = MAKE_INT_16(OPER_AW_16());
}


static void m68k_op_movea_16_al(void)
{
  AX = MAKE_INT_16(OPER_AL_16());
}


static void m68k_op_movea_16_pcdi(void)
{
  AX = MAKE_INT_16(OPER_PCDI_16());
}


static void m68k_op_movea_16_pcix(void)
{
  AX = MAKE_INT_16(OPER_PCIX_16());
}


static void m68k_op_movea_16_i(void)
{
  AX = MAKE_INT_16(OPER_I_16());
}


static void m68k_op_movea_32_d(void)
{
  AX = DY;
}


static void m68k_op_movea_32_a(void)
{
  AX = AY;
}


static void m68k_op_movea_32_ai(void)
{
  AX = OPER_AY_AI_32();
}


static void m68k_op_movea_32_pi(void)
{
  AX = OPER_AY_PI_32();
}


static void m68k_op_movea_32_pd(void)
{
  AX = OPER_AY_PD_32();
}


static void m68k_op_movea_32_di(void)
{
  AX = OPER_AY_DI_32();
}


static void m68k_op_movea_32_ix(void)
{
  AX = OPER_AY_IX_32();
}


static void m68k_op_movea_32_aw(void)
{
  AX = OPER_AW_32();
}


static void m68k_op_movea_32_al(void)
{
  AX = OPER_AL_32();
}


static void m68k_op_movea_32_pcdi(void)
{
  AX = OPER_PCDI_32();
}


static void m68k_op_movea_32_pcix(void)
{
  AX = OPER_PCIX_32();
}


static void m68k_op_movea_32_i(void)
{
  AX = OPER_I_32();
}

static void m68k_op_move_16_toc_d(void)
{
  m68ki_set_ccr(DY);
}


static void m68k_op_move_16_toc_ai(void)
{
  m68ki_set_ccr(OPER_AY_AI_16());
}


static void m68k_op_move_16_toc_pi(void)
{
  m68ki_set_ccr(OPER_AY_PI_16());
}


static void m68k_op_move_16_toc_pd(void)
{
  m68ki_set_ccr(OPER_AY_PD_16());
}


static void m68k_op_move_16_toc_di(void)
{
  m68ki_set_ccr(OPER_AY_DI_16());
}


static void m68k_op_move_16_toc_ix(void)
{
  m68ki_set_ccr(OPER_AY_IX_16());
}


static void m68k_op_move_16_toc_aw(void)
{
  m68ki_set_ccr(OPER_AW_16());
}


static void m68k_op_move_16_toc_al(void)
{
  m68ki_set_ccr(OPER_AL_16());
}


static void m68k_op_move_16_toc_pcdi(void)
{
  m68ki_set_ccr(OPER_PCDI_16());
}


static void m68k_op_move_16_toc_pcix(void)
{
  m68ki_set_ccr(OPER_PCIX_16());
}


static void m68k_op_move_16_toc_i(void)
{
  m68ki_set_ccr(OPER_I_16());
}


static void m68k_op_move_16_frs_d(void)
{
  DY = MASK_OUT_BELOW_16(DY) | m68ki_get_sr();
}


static void m68k_op_move_16_frs_ai(void)
{
  uint ea = EA_AY_AI_16();
  m68ki_write_16(ea, m68ki_get_sr());
}


static void m68k_op_move_16_frs_pi(void)
{
  uint ea = EA_AY_PI_16();
  m68ki_write_16(ea, m68ki_get_sr());
}


static void m68k_op_move_16_frs_pd(void)
{
  uint ea = EA_AY_PD_16();
  m68ki_write_16(ea, m68ki_get_sr());
}


static void m68k_op_move_16_frs_di(void)
{
  uint ea = EA_AY_DI_16();
  m68ki_write_16(ea, m68ki_get_sr());
}


static void m68k_op_move_16_frs_ix(void)
{
  uint ea = EA_AY_IX_16();
  m68ki_write_16(ea, m68ki_get_sr());
}


static void m68k_op_move_16_frs_aw(void)
{
  uint ea = EA_AW_16();
  m68ki_write_16(ea, m68ki_get_sr());
}


static void m68k_op_move_16_frs_al(void)
{
  uint ea = EA_AL_16();
  m68ki_write_16(ea, m68ki_get_sr());
}


static void m68k_op_move_16_tos_d(void)
{
  if(FLAG_S)
  {
    m68ki_set_sr(DY);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_ai(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_AY_AI_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_pi(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_AY_PI_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_pd(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_AY_PD_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_di(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_AY_DI_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_ix(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_AY_IX_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_aw(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_AW_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_al(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_AL_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_pcdi(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_PCDI_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_pcix(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_PCIX_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_16_tos_i(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_I_16();
    m68ki_set_sr(new_sr);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_32_fru(void)
{
  if(FLAG_S)
  {
    AY = REG_USP;
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_move_32_tou(void)
{
  if(FLAG_S)
  {
    REG_USP = AY;
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_movem_16_re_pd(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = AY;
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      ea -= 2;
      m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[15-i]));
      count++;
    }
  AY = ea;

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_re_ai(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_AI_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_re_di(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_DI_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_re_ix(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_IX_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_re_aw(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AW_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_re_al(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AL_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_16(ea, MASK_OUT_ABOVE_16(REG_DA[i]));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_32_re_pd(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = AY;
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      ea -= 4;
      m68ki_write_16(ea+2, REG_DA[15-i] & 0xFFFF );
      m68ki_write_16(ea, (REG_DA[15-i] >> 16) & 0xFFFF );
      count++;
    }
  AY = ea;

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_re_ai(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_AI_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_32(ea, REG_DA[i]);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_re_di(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_DI_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_32(ea, REG_DA[i]);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_re_ix(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_IX_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_32(ea, REG_DA[i]);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_re_aw(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AW_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_32(ea, REG_DA[i]);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_re_al(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AL_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      m68ki_write_32(ea, REG_DA[i]);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_16_er_pi(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = AY;
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
      ea += 2;
      count++;
    }
  AY = ea;

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_er_pcdi(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_PCDI_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_pcrel_16(ea)));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_er_pcix(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_PCIX_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_pcrel_16(ea)));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_er_ai(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_AI_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_er_di(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_DI_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_er_ix(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_IX_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_er_aw(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AW_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_16_er_al(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AL_16();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = MAKE_INT_16(MASK_OUT_ABOVE_16(m68ki_read_16(ea)));
      ea += 2;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_W);
}


static void m68k_op_movem_32_er_pi(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = AY;
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = m68ki_read_32(ea);
      ea += 4;
      count++;
    }
  AY = ea;

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_er_pcdi(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_PCDI_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = m68ki_read_pcrel_32(ea);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_er_pcix(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_PCIX_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = m68ki_read_pcrel_32(ea);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_er_ai(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_AI_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = m68ki_read_32(ea);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_er_di(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_DI_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = m68ki_read_32(ea);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_er_ix(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AY_IX_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = m68ki_read_32(ea);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_er_aw(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AW_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = m68ki_read_32(ea);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movem_32_er_al(void)
{
  uint i = 0;
  uint register_list = OPER_I_16();
  uint ea = EA_AL_32();
  uint count = 0;

  for(; i < 16; i++)
    if(register_list & (1 << i))
    {
      REG_DA[i] = m68ki_read_32(ea);
      ea += 4;
      count++;
    }

  USE_CYCLES(count * CYC_MOVEM_L);
}


static void m68k_op_movep_16_re(void)
{
  uint ea = EA_AY_DI_16();
  uint src = DX;

  m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 8));
  m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));
}


static void m68k_op_movep_32_re(void)
{
  uint ea = EA_AY_DI_32();
  uint src = DX;

  m68ki_write_8(ea, MASK_OUT_ABOVE_8(src >> 24));
  m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 16));
  m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src >> 8));
  m68ki_write_8(ea += 2, MASK_OUT_ABOVE_8(src));
}


static void m68k_op_movep_16_er(void)
{
  uint ea = EA_AY_DI_16();
  uint* r_dst = &DX;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | ((m68ki_read_8(ea) << 8) + m68ki_read_8(ea + 2));
}


static void m68k_op_movep_32_er(void)
{
  uint ea = EA_AY_DI_32();

  DX = (m68ki_read_8(ea) << 24) + (m68ki_read_8(ea + 2) << 16)
    + (m68ki_read_8(ea + 4) << 8) + m68ki_read_8(ea + 6);
}


static void m68k_op_moveq_32(void)
{
  uint res = DX = MAKE_INT_8(MASK_OUT_ABOVE_8(REG_IR));

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_d(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(DY);
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_ai(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_AI_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_pi(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_PI_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_pd(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_PD_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_di(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_DI_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_ix(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AY_IX_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_aw(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AW_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_al(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_AL_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_pcdi(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_PCDI_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_pcix(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_PCIX_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_muls_16_i(void)
{
  uint* r_dst = &DX;
  sint src = MAKE_INT_16(OPER_I_16());
  uint res = MASK_OUT_ABOVE_32( src * MAKE_INT_16(MASK_OUT_ABOVE_16(*r_dst)));

  UseMulsCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_d(void)
{
  uint* r_dst = &DX;
  uint src = MASK_OUT_ABOVE_16(DY);
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_ai(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_AI_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_pi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PI_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_pd(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PD_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_di(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_DI_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_ix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_IX_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_aw(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AW_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_al(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AL_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_pcdi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCDI_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_pcix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCIX_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_mulu_16_i(void)
{
  uint* r_dst = &DX;
  uint src = OPER_I_16();
  uint res = src * MASK_OUT_ABOVE_16(*r_dst);

  UseMuluCycles(src);

  *r_dst = res;

  FLAG_Z = res;
  FLAG_N = NFLAG_32(res);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_nbcd_8_d(void)
{
  uint* r_dst = &DY;
  uint dst = *r_dst;
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_di(void)
{
  uint ea = EA_AY_DI_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_aw(void)
{
  uint ea = EA_AW_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_nbcd_8_al(void)
{
  uint ea = EA_AL_8();
  uint dst = m68ki_read_8(ea);
  uint res = MASK_OUT_ABOVE_8(0x9a - dst - XFLAG_AS_1());

  if(res != 0x9a)
  {
    FLAG_V = ~res; /* Undefined V behavior */

    if((res & 0x0f) == 0xa)
      res = (res & 0xf0) + 0x10;

    res = MASK_OUT_ABOVE_8(res);

    FLAG_V &= res; /* Undefined V behavior part II */

    m68ki_write_8(ea, MASK_OUT_ABOVE_8(res));

    FLAG_Z |= res;
    FLAG_C = CFLAG_SET;
    FLAG_X = XFLAG_SET;
  }
  else
  {
    FLAG_V = VFLAG_CLEAR;
    FLAG_C = CFLAG_CLEAR;
    FLAG_X = XFLAG_CLEAR;
  }
  FLAG_N = NFLAG_8(res);  /* Undefined N behavior */
}


static void m68k_op_neg_8_d(void)
{
  uint* r_dst = &DY;
  uint res = 0 - MASK_OUT_ABOVE_8(*r_dst);

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = *r_dst & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_neg_8_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_8_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_8_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_8_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_8_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_8_di(void)
{
  uint ea = EA_AY_DI_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_8_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_8_aw(void)
{
  uint ea = EA_AW_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_8_al(void)
{
  uint ea = EA_AL_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_8(res);
  FLAG_C = FLAG_X = CFLAG_8(res);
  FLAG_V = src & res;
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_neg_16_d(void)
{
  uint* r_dst = &DY;
  uint res = 0 - MASK_OUT_ABOVE_16(*r_dst);

  FLAG_N = NFLAG_16(res);
  FLAG_C = FLAG_X = CFLAG_16(res);
  FLAG_V = (*r_dst & res)>>8;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_neg_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_16(res);
  FLAG_C = FLAG_X = CFLAG_16(res);
  FLAG_V = (src & res)>>8;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_neg_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_16(res);
  FLAG_C = FLAG_X = CFLAG_16(res);
  FLAG_V = (src & res)>>8;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_neg_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_16(res);
  FLAG_C = FLAG_X = CFLAG_16(res);
  FLAG_V = (src & res)>>8;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_neg_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_16(res);
  FLAG_C = FLAG_X = CFLAG_16(res);
  FLAG_V = (src & res)>>8;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_neg_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_16(res);
  FLAG_C = FLAG_X = CFLAG_16(res);
  FLAG_V = (src & res)>>8;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_neg_16_aw(void)
{
  uint ea = EA_AW_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_16(res);
  FLAG_C = FLAG_X = CFLAG_16(res);
  FLAG_V = (src & res)>>8;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_neg_16_al(void)
{
  uint ea = EA_AL_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_16(res);
  FLAG_C = FLAG_X = CFLAG_16(res);
  FLAG_V = (src & res)>>8;
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_neg_32_d(void)
{
  uint* r_dst = &DY;
  uint res = 0 - *r_dst;

  FLAG_N = NFLAG_32(res);
  FLAG_C = FLAG_X = CFLAG_SUB_32(*r_dst, 0, res);
  FLAG_V = (*r_dst & res)>>24;
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_neg_32_ai(void)
{
  uint ea = EA_AY_AI_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_32(res);
  FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_neg_32_pi(void)
{
  uint ea = EA_AY_PI_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_32(res);
  FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_neg_32_pd(void)
{
  uint ea = EA_AY_PD_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_32(res);
  FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_neg_32_di(void)
{
  uint ea = EA_AY_DI_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_32(res);
  FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_neg_32_ix(void)
{
  uint ea = EA_AY_IX_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_32(res);
  FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_neg_32_aw(void)
{
  uint ea = EA_AW_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_32(res);
  FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_neg_32_al(void)
{
  uint ea = EA_AL_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - src;

  FLAG_N = NFLAG_32(res);
  FLAG_C = FLAG_X = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_negx_8_d(void)
{
  uint* r_dst = &DY;
  uint res = 0 - MASK_OUT_ABOVE_8(*r_dst) - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = *r_dst & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


static void m68k_op_negx_8_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_8_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_8_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_8_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_8_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_8_di(void)
{
  uint ea = EA_AY_DI_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_8_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_8_aw(void)
{
  uint ea = EA_AW_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_8_al(void)
{
  uint ea = EA_AL_8();
  uint src = m68ki_read_8(ea);
  uint res = 0 - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = src & res;

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_negx_16_d(void)
{
  uint* r_dst = &DY;
  uint res = 0 - MASK_OUT_ABOVE_16(*r_dst) - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = (*r_dst & res)>>8;

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


static void m68k_op_negx_16_ai(void)
{
  uint ea  = EA_AY_AI_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = (src & res)>>8;

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  m68ki_write_16(ea, res);
}


static void m68k_op_negx_16_pi(void)
{
  uint ea  = EA_AY_PI_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = (src & res)>>8;

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  m68ki_write_16(ea, res);
}


static void m68k_op_negx_16_pd(void)
{
  uint ea  = EA_AY_PD_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = (src & res)>>8;

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  m68ki_write_16(ea, res);
}


static void m68k_op_negx_16_di(void)
{
  uint ea  = EA_AY_DI_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = (src & res)>>8;

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  m68ki_write_16(ea, res);
}


static void m68k_op_negx_16_ix(void)
{
  uint ea  = EA_AY_IX_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = (src & res)>>8;

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  m68ki_write_16(ea, res);
}


static void m68k_op_negx_16_aw(void)
{
  uint ea  = EA_AW_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = (src & res)>>8;

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  m68ki_write_16(ea, res);
}


static void m68k_op_negx_16_al(void)
{
  uint ea  = EA_AL_16();
  uint src = m68ki_read_16(ea);
  uint res = 0 - MASK_OUT_ABOVE_16(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = (src & res)>>8;

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  m68ki_write_16(ea, res);
}


static void m68k_op_negx_32_d(void)
{
  uint* r_dst = &DY;
  uint res = 0 - MASK_OUT_ABOVE_32(*r_dst) - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(*r_dst, 0, res);
  FLAG_V = (*r_dst & res)>>24;

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  *r_dst = res;
}


static void m68k_op_negx_32_ai(void)
{
  uint ea  = EA_AY_AI_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  m68ki_write_32(ea, res);
}


static void m68k_op_negx_32_pi(void)
{
  uint ea  = EA_AY_PI_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  m68ki_write_32(ea, res);
}


static void m68k_op_negx_32_pd(void)
{
  uint ea  = EA_AY_PD_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  m68ki_write_32(ea, res);
}


static void m68k_op_negx_32_di(void)
{
  uint ea  = EA_AY_DI_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  m68ki_write_32(ea, res);
}


static void m68k_op_negx_32_ix(void)
{
  uint ea  = EA_AY_IX_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  m68ki_write_32(ea, res);
}


static void m68k_op_negx_32_aw(void)
{
  uint ea  = EA_AW_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  m68ki_write_32(ea, res);
}


static void m68k_op_negx_32_al(void)
{
  uint ea  = EA_AL_32();
  uint src = m68ki_read_32(ea);
  uint res = 0 - MASK_OUT_ABOVE_32(src) - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, 0, res);
  FLAG_V = (src & res)>>24;

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  m68ki_write_32(ea, res);
}


static void m68k_op_nop(void)
{
}


static void m68k_op_not_8_d(void)
{
  uint* r_dst = &DY;
  uint res = MASK_OUT_ABOVE_8(~*r_dst);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_di(void)
{
  uint ea = EA_AY_DI_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_aw(void)
{
  uint ea = EA_AW_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_8_al(void)
{
  uint ea = EA_AL_8();
  uint res = MASK_OUT_ABOVE_8(~m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_16_d(void)
{
  uint* r_dst = &DY;
  uint res = MASK_OUT_ABOVE_16(~*r_dst);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_16_aw(void)
{
  uint ea = EA_AW_16();
  uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_16_al(void)
{
  uint ea = EA_AL_16();
  uint res = MASK_OUT_ABOVE_16(~m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_32_d(void)
{
  uint* r_dst = &DY;
  uint res = *r_dst = MASK_OUT_ABOVE_32(~*r_dst);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_32_ai(void)
{
  uint ea = EA_AY_AI_32();
  uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_32_pi(void)
{
  uint ea = EA_AY_PI_32();
  uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_32_pd(void)
{
  uint ea = EA_AY_PD_32();
  uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_32_di(void)
{
  uint ea = EA_AY_DI_32();
  uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_32_ix(void)
{
  uint ea = EA_AY_IX_32();
  uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_32_aw(void)
{
  uint ea = EA_AW_32();
  uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_not_32_al(void)
{
  uint ea = EA_AL_32();
  uint res = MASK_OUT_ABOVE_32(~m68ki_read_32(ea));

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_d(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= MASK_OUT_ABOVE_8(DY)));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_ai(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_AI_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_pi(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_PI_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_pi7(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_A7_PI_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_pd(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_PD_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_pd7(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_A7_PD_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_di(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_DI_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_ix(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_AY_IX_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_aw(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_AW_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_al(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_AL_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_pcdi(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_PCDI_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_pcix(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_PCIX_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_er_i(void)
{
  uint res = MASK_OUT_ABOVE_8((DX |= OPER_I_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_d(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= MASK_OUT_ABOVE_16(DY)));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_ai(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_AI_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_pi(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_PI_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_pd(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_PD_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_di(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_DI_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_ix(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_AY_IX_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_aw(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_AW_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_al(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_AL_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_pcdi(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_PCDI_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_pcix(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_PCIX_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_er_i(void)
{
  uint res = MASK_OUT_ABOVE_16((DX |= OPER_I_16()));

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_d(void)
{
  uint res = DX |= DY;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_ai(void)
{
  uint res = DX |= OPER_AY_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_pi(void)
{
  uint res = DX |= OPER_AY_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_pd(void)
{
  uint res = DX |= OPER_AY_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_di(void)
{
  uint res = DX |= OPER_AY_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_ix(void)
{
  uint res = DX |= OPER_AY_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_aw(void)
{
  uint res = DX |= OPER_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_al(void)
{
  uint res = DX |= OPER_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_pcdi(void)
{
  uint res = DX |= OPER_PCDI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_pcix(void)
{
  uint res = DX |= OPER_PCIX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_er_i(void)
{
  uint res = DX |= OPER_I_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_di(void)
{
  uint ea = EA_AY_DI_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_aw(void)
{
  uint ea = EA_AW_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_8_re_al(void)
{
  uint ea = EA_AL_8();
  uint res = MASK_OUT_ABOVE_8(DX | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_re_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_re_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_re_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_re_di(void)
{
  uint ea = EA_AY_DI_16();
  uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_re_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_re_aw(void)
{
  uint ea = EA_AW_16();
  uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_16_re_al(void)
{
  uint ea = EA_AL_16();
  uint res = MASK_OUT_ABOVE_16(DX | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_re_ai(void)
{
  uint ea = EA_AY_AI_32();
  uint res = DX | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_re_pi(void)
{
  uint ea = EA_AY_PI_32();
  uint res = DX | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_re_pd(void)
{
  uint ea = EA_AY_PD_32();
  uint res = DX | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_re_di(void)
{
  uint ea = EA_AY_DI_32();
  uint res = DX | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_re_ix(void)
{
  uint ea = EA_AY_IX_32();
  uint res = DX | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_re_aw(void)
{
  uint ea = EA_AW_32();
  uint res = DX | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_or_32_re_al(void)
{
  uint ea = EA_AL_32();
  uint res = DX | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_d(void)
{
  uint res = MASK_OUT_ABOVE_8((DY |= OPER_I_8()));

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_ai(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_AI_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_pi(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_PI_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_pi7(void)
{
  uint src = OPER_I_8();
  uint ea = EA_A7_PI_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_pd(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_PD_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_pd7(void)
{
  uint src = OPER_I_8();
  uint ea = EA_A7_PD_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_di(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_DI_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_ix(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_IX_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_aw(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AW_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_8_al(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AL_8();
  uint res = MASK_OUT_ABOVE_8(src | m68ki_read_8(ea));

  m68ki_write_8(ea, res);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY |= OPER_I_16());

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_ai(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_AI_16();
  uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_pi(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_PI_16();
  uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_pd(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_PD_16();
  uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_di(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_DI_16();
  uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_ix(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_IX_16();
  uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_aw(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AW_16();
  uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_al(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AL_16();
  uint res = MASK_OUT_ABOVE_16(src | m68ki_read_16(ea));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_32_d(void)
{
  uint res = DY |= OPER_I_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_32_ai(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_AI_32();
  uint res = src | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_32_pi(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_PI_32();
  uint res = src | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_32_pd(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_PD_32();
  uint res = src | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_32_di(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_DI_32();
  uint res = src | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_32_ix(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_IX_32();
  uint res = src | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_32_aw(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AW_32();
  uint res = src | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_32_al(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AL_32();
  uint res = src | m68ki_read_32(ea);

  m68ki_write_32(ea, res);

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ori_16_toc(void)
{
  m68ki_set_ccr(m68ki_get_ccr() | OPER_I_16());
}


static void m68k_op_ori_16_tos(void)
{
  if(FLAG_S)
  {
    uint src = OPER_I_16();
    m68ki_set_sr(m68ki_get_sr() | src);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_pea_32_ai(void)
{
  uint ea = EA_AY_AI_32();

  m68ki_push_32(ea);
}


static void m68k_op_pea_32_di(void)
{
  uint ea = EA_AY_DI_32();

  m68ki_push_32(ea);
}


static void m68k_op_pea_32_ix(void)
{
  uint ea = EA_AY_IX_32();

  m68ki_push_32(ea);
}


static void m68k_op_pea_32_aw(void)
{
  uint ea = EA_AW_32();

  m68ki_push_32(ea);
}


static void m68k_op_pea_32_al(void)
{
  uint ea = EA_AL_32();

  m68ki_push_32(ea);
}


static void m68k_op_pea_32_pcdi(void)
{
  uint ea = EA_PCDI_32();

  m68ki_push_32(ea);
}


static void m68k_op_pea_32_pcix(void)
{
  uint ea = EA_PCIX_32();

  m68ki_push_32(ea);
}


static void m68k_op_reset(void)
{
  if(FLAG_S)
  {
    m68ki_output_reset()       /* auto-disable (see m68kcpu.h) */
    USE_CYCLES(CYC_RESET);
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_ror_8_s(void)
{
  uint* r_dst = &DY;
  uint orig_shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint shift = orig_shift & 7;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = ROR_8(src, shift);

  if(orig_shift != 0)
    USE_CYCLES(orig_shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = src << (9-orig_shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = ROR_16(src, shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src << (9-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_32_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint64 src = *r_dst;
  uint res = ROR_32(src, shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = src << (9-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_8_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;
  uint shift = orig_shift & 7;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = ROR_8(src, shift);

  if(orig_shift != 0)
  {
    USE_CYCLES(orig_shift * CYC_SHIFT);

    *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
    FLAG_C = src << (8-((shift-1)&7));
    FLAG_N = NFLAG_8(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = CFLAG_CLEAR;
  FLAG_N = NFLAG_8(src);
  FLAG_Z = src;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;
  uint shift = orig_shift & 15;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = ROR_16(src, shift);

  if(orig_shift != 0)
  {
    USE_CYCLES(orig_shift * CYC_SHIFT);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    FLAG_C = (src >> ((shift - 1) & 15)) << 8;
    FLAG_N = NFLAG_16(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = CFLAG_CLEAR;
  FLAG_N = NFLAG_16(src);
  FLAG_Z = src;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_32_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;
  uint shift = orig_shift & 31;
  uint64 src = *r_dst;
  uint res = ROR_32(src, shift);

  if(orig_shift != 0)
  {
    USE_CYCLES(orig_shift * CYC_SHIFT);

    *r_dst = res;
    FLAG_C = (src >> ((shift - 1) & 31)) << 8;
    FLAG_N = NFLAG_32(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = CFLAG_CLEAR;
  FLAG_N = NFLAG_32(src);
  FLAG_Z = src;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_16(src, 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_16(src, 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_16(src, 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_16(src, 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_16(src, 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_aw(void)
{
  uint ea = EA_AW_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_16(src, 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_ror_16_al(void)
{
  uint ea = EA_AL_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_16(src, 1);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src << 8;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_8_s(void)
{
  uint* r_dst = &DY;
  uint orig_shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint shift = orig_shift & 7;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = ROL_8(src, shift);

  if(orig_shift != 0)
    USE_CYCLES(orig_shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_C = src << orig_shift;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = ROL_16(src, shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src >> (8-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_32_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint64 src = *r_dst;
  uint res = ROL_32(src, shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_C = src >> (24-shift);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_8_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;
  uint shift = orig_shift & 7;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = ROL_8(src, shift);

  if(orig_shift != 0)
  {
    USE_CYCLES(orig_shift * CYC_SHIFT);

    if(shift != 0)
    {
      *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
      FLAG_C = src << shift;
      FLAG_N = NFLAG_8(res);
      FLAG_Z = res;
      FLAG_V = VFLAG_CLEAR;
      return;
    }
    FLAG_C = (src & 1)<<8;
    FLAG_N = NFLAG_8(src);
    FLAG_Z = src;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = CFLAG_CLEAR;
  FLAG_N = NFLAG_8(src);
  FLAG_Z = src;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;
  uint shift = orig_shift & 15;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = MASK_OUT_ABOVE_16(ROL_16(src, shift));

  if(orig_shift != 0)
  {
    USE_CYCLES(orig_shift * CYC_SHIFT);

    if(shift != 0)
    {
      *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
      FLAG_C = (src << shift) >> 8;
      FLAG_N = NFLAG_16(res);
      FLAG_Z = res;
      FLAG_V = VFLAG_CLEAR;
      return;
    }
    FLAG_C = (src & 1)<<8;
    FLAG_N = NFLAG_16(src);
    FLAG_Z = src;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = CFLAG_CLEAR;
  FLAG_N = NFLAG_16(src);
  FLAG_Z = src;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_32_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;
  uint shift = orig_shift & 31;
  uint64 src = *r_dst;
  uint res = ROL_32(src, shift);

  if(orig_shift != 0)
  {
    USE_CYCLES(orig_shift * CYC_SHIFT);

    *r_dst = res;

    FLAG_C = (src >> ((32 - shift) & 0x1f)) << 8;
    FLAG_N = NFLAG_32(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = CFLAG_CLEAR;
  FLAG_N = NFLAG_32(src);
  FLAG_Z = src;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_aw(void)
{
  uint ea = EA_AW_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rol_16_al(void)
{
  uint ea = EA_AL_16();
  uint src = m68ki_read_16(ea);
  uint res = MASK_OUT_ABOVE_16(ROL_16(src, 1));

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_C = src >> 7;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_8_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = ROR_9(src | (XFLAG_AS_1() << 8), shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  FLAG_C = FLAG_X = res;
  res = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_16_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = ROR_17(src | (XFLAG_AS_1() << 16), shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_32_s(void)
{
#if M68K_USE_64_BIT

  uint*  r_dst = &DY;
  uint   shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint64 src   = *r_dst;
  uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  res = ROR_33_64(res, shift);

  FLAG_C = FLAG_X = res >> 24;
  res = MASK_OUT_ABOVE_32(res);

  *r_dst =  res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;

#else

  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = *r_dst;
  uint res = MASK_OUT_ABOVE_32((ROR_33(src, shift) & ~(1 << (32 - shift))) | (XFLAG_AS_1() << (32 - shift)));
  uint new_x_flag = src & (1 << (shift - 1));

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = res;

  FLAG_C = FLAG_X = (new_x_flag != 0)<<8;
  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;

#endif
}


static void m68k_op_roxr_8_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;

  if(orig_shift != 0)
  {
    uint shift = orig_shift % 9;
    uint src   = MASK_OUT_ABOVE_8(*r_dst);
    uint res   = ROR_9(src | (XFLAG_AS_1() << 8), shift);

    USE_CYCLES(orig_shift * CYC_SHIFT);

    FLAG_C = FLAG_X = res;
    res = MASK_OUT_ABOVE_8(res);

    *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
    FLAG_N = NFLAG_8(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = FLAG_X;
  FLAG_N = NFLAG_8(*r_dst);
  FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_16_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;

  if(orig_shift != 0)
  {
    uint shift = orig_shift % 17;
    uint src   = MASK_OUT_ABOVE_16(*r_dst);
    uint res   = ROR_17(src | (XFLAG_AS_1() << 16), shift);

    USE_CYCLES(orig_shift * CYC_SHIFT);

    FLAG_C = FLAG_X = res >> 8;
    res = MASK_OUT_ABOVE_16(res);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    FLAG_N = NFLAG_16(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = FLAG_X;
  FLAG_N = NFLAG_16(*r_dst);
  FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_32_r(void)
{
#if M68K_USE_64_BIT

  uint*  r_dst = &DY;
  uint   orig_shift = DX & 0x3f;

  if(orig_shift != 0)
  {
    uint   shift = orig_shift % 33;
    uint64 src   = *r_dst;
    uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

    res = ROR_33_64(res, shift);

    USE_CYCLES(orig_shift * CYC_SHIFT);

    FLAG_C = FLAG_X = res >> 24;
    res = MASK_OUT_ABOVE_32(res);

    *r_dst = res;
    FLAG_N = NFLAG_32(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = FLAG_X;
  FLAG_N = NFLAG_32(*r_dst);
  FLAG_Z = *r_dst;
  FLAG_V = VFLAG_CLEAR;

#else

  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;
  uint shift = orig_shift % 33;
  uint src = *r_dst;
  uint res = MASK_OUT_ABOVE_32((ROR_33(src, shift) & ~(1 << (32 - shift))) | (XFLAG_AS_1() << (32 - shift)));
  uint new_x_flag = src & (1 << (shift - 1));

  if(orig_shift != 0)
    USE_CYCLES(orig_shift * CYC_SHIFT);

  if(shift != 0)
  {
    *r_dst = res;
    FLAG_X = (new_x_flag != 0)<<8;
  }
  else
    res = src;
  FLAG_C = FLAG_X;
  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;

#endif
}


static void m68k_op_roxr_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_16_aw(void)
{
  uint ea = EA_AW_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxr_16_al(void)
{
  uint ea = EA_AL_16();
  uint src = m68ki_read_16(ea);
  uint res = ROR_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_8_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_8(*r_dst);
  uint res = ROL_9(src | (XFLAG_AS_1() << 8), shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  FLAG_C = FLAG_X = res;
  res = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_16_s(void)
{
  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = MASK_OUT_ABOVE_16(*r_dst);
  uint res = ROL_17(src | (XFLAG_AS_1() << 16), shift);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_32_s(void)
{
#if M68K_USE_64_BIT

  uint*  r_dst = &DY;
  uint   shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint64 src   = *r_dst;
  uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  res = ROL_33_64(res, shift);

  FLAG_C = FLAG_X = res >> 24;
  res = MASK_OUT_ABOVE_32(res);

  *r_dst = res;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;

#else

  uint* r_dst = &DY;
  uint shift = (((REG_IR >> 9) - 1) & 7) + 1;
  uint src = *r_dst;
  uint res = MASK_OUT_ABOVE_32((ROL_33(src, shift) & ~(1 << (shift - 1))) | (XFLAG_AS_1() << (shift - 1)));
  uint new_x_flag = src & (1 << (32 - shift));

  if(shift != 0)
    USE_CYCLES(shift * CYC_SHIFT);

  *r_dst = res;

  FLAG_C = FLAG_X = (new_x_flag != 0)<<8;
  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;

#endif
}


static void m68k_op_roxl_8_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;


  if(orig_shift != 0)
  {
    uint shift = orig_shift % 9;
    uint src   = MASK_OUT_ABOVE_8(*r_dst);
    uint res   = ROL_9(src | (XFLAG_AS_1() << 8), shift);

    USE_CYCLES(orig_shift * CYC_SHIFT);

    FLAG_C = FLAG_X = res;
    res = MASK_OUT_ABOVE_8(res);

    *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
    FLAG_N = NFLAG_8(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = FLAG_X;
  FLAG_N = NFLAG_8(*r_dst);
  FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_16_r(void)
{
  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;

  if(orig_shift != 0)
  {
    uint shift = orig_shift % 17;
    uint src   = MASK_OUT_ABOVE_16(*r_dst);
    uint res   = ROL_17(src | (XFLAG_AS_1() << 16), shift);

    USE_CYCLES(orig_shift * CYC_SHIFT);

    FLAG_C = FLAG_X = res >> 8;
    res = MASK_OUT_ABOVE_16(res);

    *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
    FLAG_N = NFLAG_16(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = FLAG_X;
  FLAG_N = NFLAG_16(*r_dst);
  FLAG_Z = MASK_OUT_ABOVE_16(*r_dst);
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_32_r(void)
{
#if M68K_USE_64_BIT

  uint*  r_dst = &DY;
  uint   orig_shift = DX & 0x3f;

  if(orig_shift != 0)
  {
    uint   shift = orig_shift % 33;
    uint64 src   = *r_dst;
    uint64 res   = src | (((uint64)XFLAG_AS_1()) << 32);

    res = ROL_33_64(res, shift);

    USE_CYCLES(orig_shift * CYC_SHIFT);

    FLAG_C = FLAG_X = res >> 24;
    res = MASK_OUT_ABOVE_32(res);

    *r_dst = res;
    FLAG_N = NFLAG_32(res);
    FLAG_Z = res;
    FLAG_V = VFLAG_CLEAR;
    return;
  }

  FLAG_C = FLAG_X;
  FLAG_N = NFLAG_32(*r_dst);
  FLAG_Z = *r_dst;
  FLAG_V = VFLAG_CLEAR;

#else

  uint* r_dst = &DY;
  uint orig_shift = DX & 0x3f;
  uint shift = orig_shift % 33;
  uint src = *r_dst;
  uint res = MASK_OUT_ABOVE_32((ROL_33(src, shift) & ~(1 << (shift - 1))) | (XFLAG_AS_1() << (shift - 1)));
  uint new_x_flag = src & (1 << (32 - shift));

  if(orig_shift != 0)
    USE_CYCLES(orig_shift * CYC_SHIFT);

  if(shift != 0)
  {
    *r_dst = res;
    FLAG_X = (new_x_flag != 0)<<8;
  }
  else
    res = src;
  FLAG_C = FLAG_X;
  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;

#endif
}


static void m68k_op_roxl_16_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_16_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_16_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint src = m68ki_read_16(ea);
  uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_16_di(void)
{
  uint ea = EA_AY_DI_16();
  uint src = m68ki_read_16(ea);
  uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_16_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint src = m68ki_read_16(ea);
  uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_16_aw(void)
{
  uint ea = EA_AW_16();
  uint src = m68ki_read_16(ea);
  uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_roxl_16_al(void)
{
  uint ea = EA_AL_16();
  uint src = m68ki_read_16(ea);
  uint res = ROL_17(src | (XFLAG_AS_1() << 16), 1);

  FLAG_C = FLAG_X = res >> 8;
  res = MASK_OUT_ABOVE_16(res);

  m68ki_write_16(ea, res);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_rte_32(void)
{
  if(FLAG_S)
  {
    uint new_sr;
    uint new_pc;

    new_sr = m68ki_pull_16();
    new_pc = m68ki_pull_32();
    m68ki_jump(new_pc);
    m68ki_set_sr(new_sr);

#if M68K_EMULATE_ADDRESS_ERROR
    CPU_INSTR_MODE = INSTRUCTION_YES;
    CPU_RUN_MODE = RUN_MODE_NORMAL;
#endif

    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_rtr_32(void)
{
  m68ki_set_ccr(m68ki_pull_16());
  m68ki_jump(m68ki_pull_32());
}


static void m68k_op_rts_32(void)
{
  m68ki_jump(m68ki_pull_32());
}


static void m68k_op_sbcd_8_rr(void)
{
  uint* r_dst = &DX;
  uint src = DY;
  uint dst = *r_dst;
  uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

/* FLAG_V = ~res; */ /* Undefined V behavior */
  FLAG_V = VFLAG_CLEAR;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to assume cleared. */

  if(res > 9)
    res -= 6;
  res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
  if(res > 0x99)
  {
    res += 0xa0;
    FLAG_X = FLAG_C = CFLAG_SET;
    FLAG_N = NFLAG_SET;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */
  }
  else
    FLAG_N = FLAG_X = FLAG_C = 0;

  res = MASK_OUT_ABOVE_8(res);

/* FLAG_V &= res; */ /* Undefined V behavior part II */
/* FLAG_N = NFLAG_8(res); */ /* Undefined N behavior */
  FLAG_Z |= res;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


static void m68k_op_sbcd_8_mm_ax7(void)
{
  uint src = OPER_AY_PD_8();
  uint ea  = EA_A7_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

/* FLAG_V = ~res; */ /* Undefined V behavior */
  FLAG_V = VFLAG_CLEAR;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to return zero. */

  if(res > 9)
    res -= 6;
  res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
  if(res > 0x99)
  {
    res += 0xa0;
    FLAG_X = FLAG_C = CFLAG_SET;
    FLAG_N = NFLAG_SET;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */
  }
  else
    FLAG_N = FLAG_X = FLAG_C = 0;

  res = MASK_OUT_ABOVE_8(res);

/* FLAG_V &= res; */ /* Undefined V behavior part II */
/* FLAG_N = NFLAG_8(res); */ /* Undefined N behavior */
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_sbcd_8_mm_ay7(void)
{
  uint src = OPER_A7_PD_8();
  uint ea  = EA_AX_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

/* FLAG_V = ~res; */ /* Undefined V behavior */
  FLAG_V = VFLAG_CLEAR;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to return zero. */

  if(res > 9)
    res -= 6;
  res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
  if(res > 0x99)
  {
    res += 0xa0;
    FLAG_X = FLAG_C = CFLAG_SET;
    FLAG_N = NFLAG_SET;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */
  }
  else
    FLAG_N = FLAG_X = FLAG_C = 0;

  res = MASK_OUT_ABOVE_8(res);

/* FLAG_V &= res; */ /* Undefined V behavior part II */
/* FLAG_N = NFLAG_8(res); */ /* Undefined N behavior */
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_sbcd_8_mm_axy7(void)
{
  uint src = OPER_A7_PD_8();
  uint ea  = EA_A7_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

/* FLAG_V = ~res; */ /* Undefined V behavior */
  FLAG_V = VFLAG_CLEAR;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to return zero. */

  if(res > 9)
    res -= 6;
  res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
  if(res > 0x99)
  {
    res += 0xa0;
    FLAG_X = FLAG_C = CFLAG_SET;
    FLAG_N = NFLAG_SET;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */
  }
  else
    FLAG_N = FLAG_X = FLAG_C = 0;

  res = MASK_OUT_ABOVE_8(res);

/* FLAG_V &= res; */ /* Undefined V behavior part II */
/* FLAG_N = NFLAG_8(res); */ /* Undefined N behavior */
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_sbcd_8_mm(void)
{
  uint src = OPER_AY_PD_8();
  uint ea  = EA_AX_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = LOW_NIBBLE(dst) - LOW_NIBBLE(src) - XFLAG_AS_1();

/* FLAG_V = ~res; */ /* Undefined V behavior */
  FLAG_V = VFLAG_CLEAR;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to return zero. */

  if(res > 9)
    res -= 6;
  res += HIGH_NIBBLE(dst) - HIGH_NIBBLE(src);
  if(res > 0x99)
  {
    res += 0xa0;
    FLAG_X = FLAG_C = CFLAG_SET;
    FLAG_N = NFLAG_SET;  /* Undefined in Motorola's M68000PM/AD rev.1 and safer to follow carry. */
  }
  else
    FLAG_N = FLAG_X = FLAG_C = 0;

  res = MASK_OUT_ABOVE_8(res);

/* FLAG_V &= res; */ /* Undefined V behavior part II */
/* FLAG_N = NFLAG_8(res); */ /* Undefined N behavior */
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_st_8_d(void)
{
  DY |= 0xff;
}


static void m68k_op_st_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), 0xff);
}


static void m68k_op_st_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), 0xff);
}


static void m68k_op_st_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), 0xff);
}


static void m68k_op_st_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), 0xff);
}


static void m68k_op_st_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), 0xff);
}


static void m68k_op_st_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), 0xff);
}


static void m68k_op_st_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), 0xff);
}


static void m68k_op_st_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), 0xff);
}


static void m68k_op_st_8_al(void)
{
  m68ki_write_8(EA_AL_8(), 0xff);
}


static void m68k_op_sf_8_d(void)
{
  DY &= 0xffffff00;
}


static void m68k_op_sf_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), 0);
}


static void m68k_op_sf_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), 0);
}


static void m68k_op_sf_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), 0);
}


static void m68k_op_sf_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), 0);
}


static void m68k_op_sf_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), 0);
}


static void m68k_op_sf_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), 0);
}


static void m68k_op_sf_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), 0);
}


static void m68k_op_sf_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), 0);
}


static void m68k_op_sf_8_al(void)
{
  m68ki_write_8(EA_AL_8(), 0);
}


static void m68k_op_shi_8_d(void)
{
  if(COND_HI())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_sls_8_d(void)
{
  if(COND_LS())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_scc_8_d(void)
{
  if(COND_CC())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_scs_8_d(void)
{
  if(COND_CS())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_sne_8_d(void)
{
  if(COND_NE())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_seq_8_d(void)
{
  if(COND_EQ())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_svc_8_d(void)
{
  if(COND_VC())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_svs_8_d(void)
{
  if(COND_VS())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_spl_8_d(void)
{
  if(COND_PL())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_smi_8_d(void)
{
  if(COND_MI())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_sge_8_d(void)
{
  if(COND_GE())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_slt_8_d(void)
{
  if(COND_LT())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_sgt_8_d(void)
{
  if(COND_GT())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_sle_8_d(void)
{
  if(COND_LE())
  {
    DY |= 0xff;
    USE_CYCLES(CYC_SCC_R_TRUE);
    return;
  }
  DY &= 0xffffff00;
}


static void m68k_op_shi_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_shi_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_shi_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_shi_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_shi_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_shi_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_shi_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_shi_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_shi_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_HI() ? 0xff : 0);
}


static void m68k_op_sls_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_sls_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_sls_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_sls_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_sls_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_sls_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_sls_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_sls_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_sls_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_LS() ? 0xff : 0);
}


static void m68k_op_scc_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scc_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scc_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scc_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scc_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scc_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scc_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scc_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scc_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_CC() ? 0xff : 0);
}


static void m68k_op_scs_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_scs_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_scs_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_scs_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_scs_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_scs_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_scs_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_scs_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_scs_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_CS() ? 0xff : 0);
}


static void m68k_op_sne_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_sne_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_sne_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_sne_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_sne_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_sne_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_sne_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_sne_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_sne_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_NE() ? 0xff : 0);
}


static void m68k_op_seq_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_seq_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_seq_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_seq_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_seq_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_seq_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_seq_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_seq_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_seq_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_EQ() ? 0xff : 0);
}


static void m68k_op_svc_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svc_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svc_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svc_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svc_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svc_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svc_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svc_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svc_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_VC() ? 0xff : 0);
}


static void m68k_op_svs_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_svs_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_svs_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_svs_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_svs_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_svs_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_svs_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_svs_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_svs_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_VS() ? 0xff : 0);
}


static void m68k_op_spl_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_spl_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_spl_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_spl_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_spl_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_spl_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_spl_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_spl_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_spl_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_PL() ? 0xff : 0);
}


static void m68k_op_smi_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_smi_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_smi_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_smi_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_smi_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_smi_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_smi_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_smi_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_smi_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_MI() ? 0xff : 0);
}


static void m68k_op_sge_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_sge_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_sge_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_sge_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_sge_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_sge_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_sge_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_sge_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_sge_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_GE() ? 0xff : 0);
}


static void m68k_op_slt_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_slt_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_slt_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_slt_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_slt_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_slt_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_slt_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_slt_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_slt_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_LT() ? 0xff : 0);
}


static void m68k_op_sgt_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sgt_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sgt_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sgt_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sgt_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sgt_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sgt_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sgt_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sgt_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_GT() ? 0xff : 0);
}


static void m68k_op_sle_8_ai(void)
{
  m68ki_write_8(EA_AY_AI_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_sle_8_pi(void)
{
  m68ki_write_8(EA_AY_PI_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_sle_8_pi7(void)
{
  m68ki_write_8(EA_A7_PI_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_sle_8_pd(void)
{
  m68ki_write_8(EA_AY_PD_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_sle_8_pd7(void)
{
  m68ki_write_8(EA_A7_PD_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_sle_8_di(void)
{
  m68ki_write_8(EA_AY_DI_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_sle_8_ix(void)
{
  m68ki_write_8(EA_AY_IX_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_sle_8_aw(void)
{
  m68ki_write_8(EA_AW_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_sle_8_al(void)
{
  m68ki_write_8(EA_AL_8(), COND_LE() ? 0xff : 0);
}


static void m68k_op_stop(void)
{
  if(FLAG_S)
  {
    uint new_sr = OPER_I_16();
    CPU_STOPPED |= STOP_LEVEL_STOP;
    m68ki_set_sr(new_sr);
    SET_CYCLES(m68ki_cpu.cycle_end - 4*MUL); 
    return;
  }
  m68ki_exception_privilege_violation();
}


static void m68k_op_sub_8_er_d(void)
{
  uint* r_dst = &DX;
  uint src = MASK_OUT_ABOVE_8(DY);
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_ai(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_AI_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_pi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PI_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_pi7(void)
{
  uint* r_dst = &DX;
  uint src = OPER_A7_PI_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_pd(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PD_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_pd7(void)
{
  uint* r_dst = &DX;
  uint src = OPER_A7_PD_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_di(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_DI_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_ix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_IX_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_aw(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AW_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_al(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AL_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_pcdi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCDI_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_pcix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCIX_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_8_er_i(void)
{
  uint* r_dst = &DX;
  uint src = OPER_I_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_d(void)
{
  uint* r_dst = &DX;
  uint src = MASK_OUT_ABOVE_16(DY);
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_a(void)
{
  uint* r_dst = &DX;
  uint src = MASK_OUT_ABOVE_16(AY);
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_ai(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_AI_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_pi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PI_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_pd(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PD_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_di(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_DI_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_ix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_IX_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_aw(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AW_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_al(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AL_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_pcdi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCDI_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_pcix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCIX_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_16_er_i(void)
{
  uint* r_dst = &DX;
  uint src = OPER_I_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_sub_32_er_d(void)
{
  uint* r_dst = &DX;
  uint src = DY;
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_a(void)
{
  uint* r_dst = &DX;
  uint src = AY;
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_ai(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_AI_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_pi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PI_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_pd(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_PD_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_di(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_DI_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_ix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AY_IX_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_aw(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AW_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_al(void)
{
  uint* r_dst = &DX;
  uint src = OPER_AL_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_pcdi(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCDI_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_pcix(void)
{
  uint* r_dst = &DX;
  uint src = OPER_PCIX_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_32_er_i(void)
{
  uint* r_dst = &DX;
  uint src = OPER_I_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);

  *r_dst = FLAG_Z;
}


static void m68k_op_sub_8_re_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_8_re_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_8_re_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_8_re_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_8_re_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_8_re_di(void)
{
  uint ea = EA_AY_DI_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_8_re_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_8_re_aw(void)
{
  uint ea = EA_AW_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_8_re_al(void)
{
  uint ea = EA_AL_8();
  uint src = MASK_OUT_ABOVE_8(DX);
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_sub_16_re_ai(void)
{
  uint ea = EA_AY_AI_16();
  uint src = MASK_OUT_ABOVE_16(DX);
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_sub_16_re_pi(void)
{
  uint ea = EA_AY_PI_16();
  uint src = MASK_OUT_ABOVE_16(DX);
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_sub_16_re_pd(void)
{
  uint ea = EA_AY_PD_16();
  uint src = MASK_OUT_ABOVE_16(DX);
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_sub_16_re_di(void)
{
  uint ea = EA_AY_DI_16();
  uint src = MASK_OUT_ABOVE_16(DX);
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_sub_16_re_ix(void)
{
  uint ea = EA_AY_IX_16();
  uint src = MASK_OUT_ABOVE_16(DX);
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_sub_16_re_aw(void)
{
  uint ea = EA_AW_16();
  uint src = MASK_OUT_ABOVE_16(DX);
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_sub_16_re_al(void)
{
  uint ea = EA_AL_16();
  uint src = MASK_OUT_ABOVE_16(DX);
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_sub_32_re_ai(void)
{
  uint ea = EA_AY_AI_32();
  uint src = DX;
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_sub_32_re_pi(void)
{
  uint ea = EA_AY_PI_32();
  uint src = DX;
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_sub_32_re_pd(void)
{
  uint ea = EA_AY_PD_32();
  uint src = DX;
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_sub_32_re_di(void)
{
  uint ea = EA_AY_DI_32();
  uint src = DX;
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_sub_32_re_ix(void)
{
  uint ea = EA_AY_IX_32();
  uint src = DX;
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_sub_32_re_aw(void)
{
  uint ea = EA_AW_32();
  uint src = DX;
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_sub_32_re_al(void)
{
  uint ea = EA_AL_32();
  uint src = DX;
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_suba_16_d(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(DY));
}


static void m68k_op_suba_16_a(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - MAKE_INT_16(AY));
}


static void m68k_op_suba_16_ai(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_AI_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_pi(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_PI_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_pd(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_PD_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_di(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_DI_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_ix(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AY_IX_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_aw(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AW_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_al(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_AL_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_pcdi(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_PCDI_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_pcix(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_PCIX_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_16_i(void)
{
  uint* r_dst = &AX;
  uint src = MAKE_INT_16(OPER_I_16());

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_d(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - DY);
}


static void m68k_op_suba_32_a(void)
{
  uint* r_dst = &AX;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - AY);
}


static void m68k_op_suba_32_ai(void)
{
  uint* r_dst = &AX;
  uint src = OPER_AY_AI_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_pi(void)
{
  uint* r_dst = &AX;
  uint src = OPER_AY_PI_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_pd(void)
{
  uint* r_dst = &AX;
  uint src = OPER_AY_PD_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_di(void)
{
  uint* r_dst = &AX;
  uint src = OPER_AY_DI_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_ix(void)
{
  uint* r_dst = &AX;
  uint src = OPER_AY_IX_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_aw(void)
{
  uint* r_dst = &AX;
  uint src = OPER_AW_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_al(void)
{
  uint* r_dst = &AX;
  uint src = OPER_AL_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_pcdi(void)
{
  uint* r_dst = &AX;
  uint src = OPER_PCDI_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_pcix(void)
{
  uint* r_dst = &AX;
  uint src = OPER_PCIX_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_suba_32_i(void)
{
  uint* r_dst = &AX;
  uint src = OPER_I_32();

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - src);
}


static void m68k_op_subi_8_d(void)
{
  uint* r_dst = &DY;
  uint src = OPER_I_8();
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_subi_8_ai(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_AI_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_8_pi(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_PI_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_8_pi7(void)
{
  uint src = OPER_I_8();
  uint ea = EA_A7_PI_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_8_pd(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_8_pd7(void)
{
  uint src = OPER_I_8();
  uint ea = EA_A7_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_8_di(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_DI_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_8_ix(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AY_IX_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_8_aw(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AW_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_8_al(void)
{
  uint src = OPER_I_8();
  uint ea = EA_AL_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subi_16_d(void)
{
  uint* r_dst = &DY;
  uint src = OPER_I_16();
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_subi_16_ai(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_AI_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subi_16_pi(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_PI_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subi_16_pd(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_PD_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subi_16_di(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_DI_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subi_16_ix(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AY_IX_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subi_16_aw(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AW_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subi_16_al(void)
{
  uint src = OPER_I_16();
  uint ea = EA_AL_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subi_32_d(void)
{
  uint* r_dst = &DY;
  uint src = OPER_I_32();
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  *r_dst = FLAG_Z;
}


static void m68k_op_subi_32_ai(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_AI_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subi_32_pi(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_PI_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subi_32_pd(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_PD_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subi_32_di(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_DI_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subi_32_ix(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AY_IX_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subi_32_aw(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AW_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subi_32_al(void)
{
  uint src = OPER_I_32();
  uint ea = EA_AL_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subq_8_d(void)
{
  uint* r_dst = &DY;
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | FLAG_Z;
}


static void m68k_op_subq_8_ai(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_AI_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_8_pi(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_PI_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_8_pi7(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_A7_PI_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_8_pd(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_8_pd7(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_A7_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_8_di(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_DI_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_8_ix(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_IX_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_8_aw(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AW_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_8_al(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AL_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_8(res);
  FLAG_Z = MASK_OUT_ABOVE_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  m68ki_write_8(ea, FLAG_Z);
}


static void m68k_op_subq_16_d(void)
{
  uint* r_dst = &DY;
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | FLAG_Z;
}


static void m68k_op_subq_16_a(void)
{
  uint* r_dst = &AY;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((REG_IR >> 9) - 1) & 7) + 1));
}


static void m68k_op_subq_16_ai(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_AI_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subq_16_pi(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_PI_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subq_16_pd(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_PD_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subq_16_di(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_DI_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subq_16_ix(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_IX_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subq_16_aw(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AW_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subq_16_al(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AL_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_16(res);
  FLAG_Z = MASK_OUT_ABOVE_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  m68ki_write_16(ea, FLAG_Z);
}


static void m68k_op_subq_32_d(void)
{
  uint* r_dst = &DY;
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint dst = *r_dst;
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  *r_dst = FLAG_Z;
}


static void m68k_op_subq_32_a(void)
{
  uint* r_dst = &AY;

  *r_dst = MASK_OUT_ABOVE_32(*r_dst - ((((REG_IR >> 9) - 1) & 7) + 1));
}


static void m68k_op_subq_32_ai(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_AI_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subq_32_pi(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_PI_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subq_32_pd(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_PD_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subq_32_di(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_DI_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subq_32_ix(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AY_IX_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subq_32_aw(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AW_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subq_32_al(void)
{
  uint src = (((REG_IR >> 9) - 1) & 7) + 1;
  uint ea = EA_AL_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = MASK_OUT_ABOVE_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  m68ki_write_32(ea, FLAG_Z);
}


static void m68k_op_subx_8_rr(void)
{
  uint* r_dst = &DX;
  uint src = MASK_OUT_ABOVE_8(DY);
  uint dst = MASK_OUT_ABOVE_8(*r_dst);
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  *r_dst = MASK_OUT_BELOW_8(*r_dst) | res;
}


static void m68k_op_subx_16_rr(void)
{
  uint* r_dst = &DX;
  uint src = MASK_OUT_ABOVE_16(DY);
  uint dst = MASK_OUT_ABOVE_16(*r_dst);
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  *r_dst = MASK_OUT_BELOW_16(*r_dst) | res;
}


static void m68k_op_subx_32_rr(void)
{
  uint* r_dst = &DX;
  uint src = DY;
  uint dst = *r_dst;
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  *r_dst = res;
}


static void m68k_op_subx_8_mm_ax7(void)
{
  uint src = OPER_AY_PD_8();
  uint ea  = EA_A7_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_subx_8_mm_ay7(void)
{
  uint src = OPER_A7_PD_8();
  uint ea  = EA_AX_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_subx_8_mm_axy7(void)
{
  uint src = OPER_A7_PD_8();
  uint ea  = EA_A7_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_subx_8_mm(void)
{
  uint src = OPER_AY_PD_8();
  uint ea  = EA_AX_PD_8();
  uint dst = m68ki_read_8(ea);
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_8(res);
  FLAG_X = FLAG_C = CFLAG_8(res);
  FLAG_V = VFLAG_SUB_8(src, dst, res);

  res = MASK_OUT_ABOVE_8(res);
  FLAG_Z |= res;

  m68ki_write_8(ea, res);
}


static void m68k_op_subx_16_mm(void)
{
  uint src = OPER_AY_PD_16();
  uint ea  = EA_AX_PD_16();
  uint dst = m68ki_read_16(ea);
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_16(res);
  FLAG_X = FLAG_C = CFLAG_16(res);
  FLAG_V = VFLAG_SUB_16(src, dst, res);

  res = MASK_OUT_ABOVE_16(res);
  FLAG_Z |= res;

  m68ki_write_16(ea, res);
}


static void m68k_op_subx_32_mm(void)
{
  uint src = OPER_AY_PD_32();
  uint ea  = EA_AX_PD_32();
  uint dst = m68ki_read_32(ea);
  uint res = dst - src - XFLAG_AS_1();

  FLAG_N = NFLAG_32(res);
  FLAG_X = FLAG_C = CFLAG_SUB_32(src, dst, res);
  FLAG_V = VFLAG_SUB_32(src, dst, res);

  res = MASK_OUT_ABOVE_32(res);
  FLAG_Z |= res;

  m68ki_write_32(ea, res);
}


static void m68k_op_swap_32(void)
{
  uint* r_dst = &DY;

  FLAG_Z = MASK_OUT_ABOVE_32(*r_dst<<16);
  *r_dst = (*r_dst>>16) | FLAG_Z;

  FLAG_Z = *r_dst;
  FLAG_N = NFLAG_32(*r_dst);
  FLAG_C = CFLAG_CLEAR;
  FLAG_V = VFLAG_CLEAR;
}


static void m68k_op_tas_8_d(void)
{
  uint* r_dst = &DY;

  FLAG_Z = MASK_OUT_ABOVE_8(*r_dst);
  FLAG_N = NFLAG_8(*r_dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
  *r_dst |= 0x80;
}


static void m68k_op_tas_8_ai(void)
{
  uint ea = EA_AY_AI_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_tas_8_pi(void)
{
  uint ea = EA_AY_PI_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_tas_8_pi7(void)
{
  uint ea = EA_A7_PI_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_tas_8_pd(void)
{
  uint ea = EA_AY_PD_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_tas_8_pd7(void)
{
  uint ea = EA_A7_PD_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_tas_8_di(void)
{
  uint ea = EA_AY_DI_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_tas_8_ix(void)
{
  uint ea = EA_AY_IX_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_tas_8_aw(void)
{
  uint ea = EA_AW_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_tas_8_al(void)
{
  uint ea = EA_AL_8();
  uint dst = m68ki_read_8(ea);

  FLAG_Z = dst;
  FLAG_N = NFLAG_8(dst);
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;

  /* The Genesis/Megadrive games Gargoyles and Ex-Mutants need the TAS writeback
       disabled in order to function properly.  Some Amiga software may also rely
       on this, but only when accessing specific addresses so additional functionality
       will be needed. */
  if (m68ki_tas_callback()) m68ki_write_8(ea, dst | 0x80);
}


static void m68k_op_trap(void)
{
  /* Trap#n stacks exception frame type 0 */
  m68ki_exception_trapN(EXCEPTION_TRAP_BASE + (REG_IR & 0xf));  /* HJB 990403 */
}


static void m68k_op_trapv(void)
{
  if(COND_VC())
  {
    return;
  }
  m68ki_exception_trap(EXCEPTION_TRAPV);  /* HJB 990403 */
}


static void m68k_op_tst_8_d(void)
{
  uint res = MASK_OUT_ABOVE_8(DY);

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_ai(void)
{
  uint res = OPER_AY_AI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_pi(void)
{
  uint res = OPER_AY_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_pi7(void)
{
  uint res = OPER_A7_PI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_pd(void)
{
  uint res = OPER_AY_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_pd7(void)
{
  uint res = OPER_A7_PD_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_di(void)
{
  uint res = OPER_AY_DI_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_ix(void)
{
  uint res = OPER_AY_IX_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_aw(void)
{
  uint res = OPER_AW_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_8_al(void)
{
  uint res = OPER_AL_8();

  FLAG_N = NFLAG_8(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_16_d(void)
{
  uint res = MASK_OUT_ABOVE_16(DY);

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_16_ai(void)
{
  uint res = OPER_AY_AI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_16_pi(void)
{
  uint res = OPER_AY_PI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_16_pd(void)
{
  uint res = OPER_AY_PD_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_16_di(void)
{
  uint res = OPER_AY_DI_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_16_ix(void)
{
  uint res = OPER_AY_IX_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_16_aw(void)
{
  uint res = OPER_AW_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_16_al(void)
{
  uint res = OPER_AL_16();

  FLAG_N = NFLAG_16(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_32_d(void)
{
  uint res = DY;

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_32_ai(void)
{
  uint res = OPER_AY_AI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_32_pi(void)
{
  uint res = OPER_AY_PI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_32_pd(void)
{
  uint res = OPER_AY_PD_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_32_di(void)
{
  uint res = OPER_AY_DI_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_32_ix(void)
{
  uint res = OPER_AY_IX_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_32_aw(void)
{
  uint res = OPER_AW_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_tst_32_al(void)
{
  uint res = OPER_AL_32();

  FLAG_N = NFLAG_32(res);
  FLAG_Z = res;
  FLAG_V = VFLAG_CLEAR;
  FLAG_C = CFLAG_CLEAR;
}


static void m68k_op_unlk_32_a7(void)
{
  REG_A[7] = m68ki_read_32(REG_A[7]);
}


static void m68k_op_unlk_32(void)
{
  uint* r_dst = &AY;

  REG_A[7] = *r_dst;
  *r_dst = m68ki_pull_32();
}


/* ======================================================================== */
/* ========================= OPCODE TABLE BUILDER ========================= */
/* ======================================================================== */

#ifndef BUILD_TABLES

#include "m68ki_instruction_jump_table.h"

#else

/* This is used to generate the opcode handler jump table */
typedef struct
{
  void (*opcode_handler)(void); /* handler function */
  unsigned int  mask;           /* mask on opcode */
  unsigned int  match;          /* what to match after masking */
  unsigned char cycles;         /* cycles each cpu type takes */
} opcode_handler_struct;

/* opcode handler jump table */
static void  (*m68ki_instruction_jump_table[0x10000])(void);

/* Opcode handler table */
static const opcode_handler_struct m68k_opcode_handler_table[] =
{
/*   function                      mask    match  cyc */
  {m68k_op_1010                , 0xf000, 0xa000,  4},
  {m68k_op_1111                , 0xf000, 0xf000,  4},
  {m68k_op_moveq_32            , 0xf100, 0x7000,  4},
  {m68k_op_bra_8               , 0xff00, 0x6000, 10},
  {m68k_op_bsr_8               , 0xff00, 0x6100, 18},
  {m68k_op_bhi_8               , 0xff00, 0x6200, 10},
  {m68k_op_bls_8               , 0xff00, 0x6300, 10},
  {m68k_op_bcc_8               , 0xff00, 0x6400, 10},
  {m68k_op_bcs_8               , 0xff00, 0x6500, 10},
  {m68k_op_bne_8               , 0xff00, 0x6600, 10},
  {m68k_op_beq_8               , 0xff00, 0x6700, 10},
  {m68k_op_bvc_8               , 0xff00, 0x6800, 10},
  {m68k_op_bvs_8               , 0xff00, 0x6900, 10},
  {m68k_op_bpl_8               , 0xff00, 0x6a00, 10},
  {m68k_op_bmi_8               , 0xff00, 0x6b00, 10},
  {m68k_op_bge_8               , 0xff00, 0x6c00, 10},
  {m68k_op_blt_8               , 0xff00, 0x6d00, 10},
  {m68k_op_bgt_8               , 0xff00, 0x6e00, 10},
  {m68k_op_ble_8               , 0xff00, 0x6f00, 10},
  {m68k_op_btst_32_r_d         , 0xf1f8, 0x0100,  6},
  {m68k_op_movep_16_er         , 0xf1f8, 0x0108, 16},
  {m68k_op_btst_8_r_ai         , 0xf1f8, 0x0110,  8},
  {m68k_op_btst_8_r_pi         , 0xf1f8, 0x0118,  8},
  {m68k_op_btst_8_r_pd         , 0xf1f8, 0x0120, 10},
  {m68k_op_btst_8_r_di         , 0xf1f8, 0x0128, 12},
  {m68k_op_btst_8_r_ix         , 0xf1f8, 0x0130, 14},
  {m68k_op_bchg_32_r_d         , 0xf1f8, 0x0140,  8},
  {m68k_op_movep_32_er         , 0xf1f8, 0x0148, 24},
  {m68k_op_bchg_8_r_ai         , 0xf1f8, 0x0150, 12},
  {m68k_op_bchg_8_r_pi         , 0xf1f8, 0x0158, 12},
  {m68k_op_bchg_8_r_pd         , 0xf1f8, 0x0160, 14},
  {m68k_op_bchg_8_r_di         , 0xf1f8, 0x0168, 16},
  {m68k_op_bchg_8_r_ix         , 0xf1f8, 0x0170, 18},
  {m68k_op_bclr_32_r_d         , 0xf1f8, 0x0180, 10},
  {m68k_op_movep_16_re         , 0xf1f8, 0x0188, 16},
  {m68k_op_bclr_8_r_ai         , 0xf1f8, 0x0190, 12},
  {m68k_op_bclr_8_r_pi         , 0xf1f8, 0x0198, 12},
  {m68k_op_bclr_8_r_pd         , 0xf1f8, 0x01a0, 14},
  {m68k_op_bclr_8_r_di         , 0xf1f8, 0x01a8, 16},
  {m68k_op_bclr_8_r_ix         , 0xf1f8, 0x01b0, 18},
  {m68k_op_bset_32_r_d         , 0xf1f8, 0x01c0,  8},
  {m68k_op_movep_32_re         , 0xf1f8, 0x01c8, 24},
  {m68k_op_bset_8_r_ai         , 0xf1f8, 0x01d0, 12},
  {m68k_op_bset_8_r_pi         , 0xf1f8, 0x01d8, 12},
  {m68k_op_bset_8_r_pd         , 0xf1f8, 0x01e0, 14},
  {m68k_op_bset_8_r_di         , 0xf1f8, 0x01e8, 16},
  {m68k_op_bset_8_r_ix         , 0xf1f8, 0x01f0, 18},
  {m68k_op_move_8_d_d          , 0xf1f8, 0x1000,  4},
  {m68k_op_move_8_d_ai         , 0xf1f8, 0x1010,  8},
  {m68k_op_move_8_d_pi         , 0xf1f8, 0x1018,  8},
  {m68k_op_move_8_d_pd         , 0xf1f8, 0x1020, 10},
  {m68k_op_move_8_d_di         , 0xf1f8, 0x1028, 12},
  {m68k_op_move_8_d_ix         , 0xf1f8, 0x1030, 14},
  {m68k_op_move_8_ai_d         , 0xf1f8, 0x1080,  8},
  {m68k_op_move_8_ai_ai        , 0xf1f8, 0x1090, 12},
  {m68k_op_move_8_ai_pi        , 0xf1f8, 0x1098, 12},
  {m68k_op_move_8_ai_pd        , 0xf1f8, 0x10a0, 14},
  {m68k_op_move_8_ai_di        , 0xf1f8, 0x10a8, 16},
  {m68k_op_move_8_ai_ix        , 0xf1f8, 0x10b0, 18},
  {m68k_op_move_8_pi_d         , 0xf1f8, 0x10c0,  8},
  {m68k_op_move_8_pi_ai        , 0xf1f8, 0x10d0, 12},
  {m68k_op_move_8_pi_pi        , 0xf1f8, 0x10d8, 12},
  {m68k_op_move_8_pi_pd        , 0xf1f8, 0x10e0, 14},
  {m68k_op_move_8_pi_di        , 0xf1f8, 0x10e8, 16},
  {m68k_op_move_8_pi_ix        , 0xf1f8, 0x10f0, 18},
  {m68k_op_move_8_pd_d         , 0xf1f8, 0x1100,  8},
  {m68k_op_move_8_pd_ai        , 0xf1f8, 0x1110, 12},
  {m68k_op_move_8_pd_pi        , 0xf1f8, 0x1118, 12},
  {m68k_op_move_8_pd_pd        , 0xf1f8, 0x1120, 14},
  {m68k_op_move_8_pd_di        , 0xf1f8, 0x1128, 16},
  {m68k_op_move_8_pd_ix        , 0xf1f8, 0x1130, 18},
  {m68k_op_move_8_di_d         , 0xf1f8, 0x1140, 12},
  {m68k_op_move_8_di_ai        , 0xf1f8, 0x1150, 16},
  {m68k_op_move_8_di_pi        , 0xf1f8, 0x1158, 16},
  {m68k_op_move_8_di_pd        , 0xf1f8, 0x1160, 18},
  {m68k_op_move_8_di_di        , 0xf1f8, 0x1168, 20},
  {m68k_op_move_8_di_ix        , 0xf1f8, 0x1170, 22},
  {m68k_op_move_8_ix_d         , 0xf1f8, 0x1180, 14},
  {m68k_op_move_8_ix_ai        , 0xf1f8, 0x1190, 18},
  {m68k_op_move_8_ix_pi        , 0xf1f8, 0x1198, 18},
  {m68k_op_move_8_ix_pd        , 0xf1f8, 0x11a0, 20},
  {m68k_op_move_8_ix_di        , 0xf1f8, 0x11a8, 22},
  {m68k_op_move_8_ix_ix        , 0xf1f8, 0x11b0, 24},
  {m68k_op_move_32_d_d         , 0xf1f8, 0x2000,  4},
  {m68k_op_move_32_d_a         , 0xf1f8, 0x2008,  4},
  {m68k_op_move_32_d_ai        , 0xf1f8, 0x2010, 12},
  {m68k_op_move_32_d_pi        , 0xf1f8, 0x2018, 12},
  {m68k_op_move_32_d_pd        , 0xf1f8, 0x2020, 14},
  {m68k_op_move_32_d_di        , 0xf1f8, 0x2028, 16},
  {m68k_op_move_32_d_ix        , 0xf1f8, 0x2030, 18},
  {m68k_op_movea_32_d          , 0xf1f8, 0x2040,  4},
  {m68k_op_movea_32_a          , 0xf1f8, 0x2048,  4},
  {m68k_op_movea_32_ai         , 0xf1f8, 0x2050, 12},
  {m68k_op_movea_32_pi         , 0xf1f8, 0x2058, 12},
  {m68k_op_movea_32_pd         , 0xf1f8, 0x2060, 14},
  {m68k_op_movea_32_di         , 0xf1f8, 0x2068, 16},
  {m68k_op_movea_32_ix         , 0xf1f8, 0x2070, 18},
  {m68k_op_move_32_ai_d        , 0xf1f8, 0x2080, 12},
  {m68k_op_move_32_ai_a        , 0xf1f8, 0x2088, 12},
  {m68k_op_move_32_ai_ai       , 0xf1f8, 0x2090, 20},
  {m68k_op_move_32_ai_pi       , 0xf1f8, 0x2098, 20},
  {m68k_op_move_32_ai_pd       , 0xf1f8, 0x20a0, 22},
  {m68k_op_move_32_ai_di       , 0xf1f8, 0x20a8, 24},
  {m68k_op_move_32_ai_ix       , 0xf1f8, 0x20b0, 26},
  {m68k_op_move_32_pi_d        , 0xf1f8, 0x20c0, 12},
  {m68k_op_move_32_pi_a        , 0xf1f8, 0x20c8, 12},
  {m68k_op_move_32_pi_ai       , 0xf1f8, 0x20d0, 20},
  {m68k_op_move_32_pi_pi       , 0xf1f8, 0x20d8, 20},
  {m68k_op_move_32_pi_pd       , 0xf1f8, 0x20e0, 22},
  {m68k_op_move_32_pi_di       , 0xf1f8, 0x20e8, 24},
  {m68k_op_move_32_pi_ix       , 0xf1f8, 0x20f0, 26},
  {m68k_op_move_32_pd_d        , 0xf1f8, 0x2100, 12},
  {m68k_op_move_32_pd_a        , 0xf1f8, 0x2108, 12},
  {m68k_op_move_32_pd_ai       , 0xf1f8, 0x2110, 20},
  {m68k_op_move_32_pd_pi       , 0xf1f8, 0x2118, 20},
  {m68k_op_move_32_pd_pd       , 0xf1f8, 0x2120, 22},
  {m68k_op_move_32_pd_di       , 0xf1f8, 0x2128, 24},
  {m68k_op_move_32_pd_ix       , 0xf1f8, 0x2130, 26},
  {m68k_op_move_32_di_d        , 0xf1f8, 0x2140, 16},
  {m68k_op_move_32_di_a        , 0xf1f8, 0x2148, 16},
  {m68k_op_move_32_di_ai       , 0xf1f8, 0x2150, 24},
  {m68k_op_move_32_di_pi       , 0xf1f8, 0x2158, 24},
  {m68k_op_move_32_di_pd       , 0xf1f8, 0x2160, 26},
  {m68k_op_move_32_di_di       , 0xf1f8, 0x2168, 28},
  {m68k_op_move_32_di_ix       , 0xf1f8, 0x2170, 30},
  {m68k_op_move_32_ix_d        , 0xf1f8, 0x2180, 18},
  {m68k_op_move_32_ix_a        , 0xf1f8, 0x2188, 18},
  {m68k_op_move_32_ix_ai       , 0xf1f8, 0x2190, 26},
  {m68k_op_move_32_ix_pi       , 0xf1f8, 0x2198, 26},
  {m68k_op_move_32_ix_pd       , 0xf1f8, 0x21a0, 28},
  {m68k_op_move_32_ix_di       , 0xf1f8, 0x21a8, 30},
  {m68k_op_move_32_ix_ix       , 0xf1f8, 0x21b0, 32},
  {m68k_op_move_16_d_d         , 0xf1f8, 0x3000,  4},
  {m68k_op_move_16_d_a         , 0xf1f8, 0x3008,  4},
  {m68k_op_move_16_d_ai        , 0xf1f8, 0x3010,  8},
  {m68k_op_move_16_d_pi        , 0xf1f8, 0x3018,  8},
  {m68k_op_move_16_d_pd        , 0xf1f8, 0x3020, 10},
  {m68k_op_move_16_d_di        , 0xf1f8, 0x3028, 12},
  {m68k_op_move_16_d_ix        , 0xf1f8, 0x3030, 14},
  {m68k_op_movea_16_d          , 0xf1f8, 0x3040,  4},
  {m68k_op_movea_16_a          , 0xf1f8, 0x3048,  4},
  {m68k_op_movea_16_ai         , 0xf1f8, 0x3050,  8},
  {m68k_op_movea_16_pi         , 0xf1f8, 0x3058,  8},
  {m68k_op_movea_16_pd         , 0xf1f8, 0x3060, 10},
  {m68k_op_movea_16_di         , 0xf1f8, 0x3068, 12},
  {m68k_op_movea_16_ix         , 0xf1f8, 0x3070, 14},
  {m68k_op_move_16_ai_d        , 0xf1f8, 0x3080,  8},
  {m68k_op_move_16_ai_a        , 0xf1f8, 0x3088,  8},
  {m68k_op_move_16_ai_ai       , 0xf1f8, 0x3090, 12},
  {m68k_op_move_16_ai_pi       , 0xf1f8, 0x3098, 12},
  {m68k_op_move_16_ai_pd       , 0xf1f8, 0x30a0, 14},
  {m68k_op_move_16_ai_di       , 0xf1f8, 0x30a8, 16},
  {m68k_op_move_16_ai_ix       , 0xf1f8, 0x30b0, 18},
  {m68k_op_move_16_pi_d        , 0xf1f8, 0x30c0,  8},
  {m68k_op_move_16_pi_a        , 0xf1f8, 0x30c8,  8},
  {m68k_op_move_16_pi_ai       , 0xf1f8, 0x30d0, 12},
  {m68k_op_move_16_pi_pi       , 0xf1f8, 0x30d8, 12},
  {m68k_op_move_16_pi_pd       , 0xf1f8, 0x30e0, 14},
  {m68k_op_move_16_pi_di       , 0xf1f8, 0x30e8, 16},
  {m68k_op_move_16_pi_ix       , 0xf1f8, 0x30f0, 18},
  {m68k_op_move_16_pd_d        , 0xf1f8, 0x3100,  8},
  {m68k_op_move_16_pd_a        , 0xf1f8, 0x3108,  8},
  {m68k_op_move_16_pd_ai       , 0xf1f8, 0x3110, 12},
  {m68k_op_move_16_pd_pi       , 0xf1f8, 0x3118, 12},
  {m68k_op_move_16_pd_pd       , 0xf1f8, 0x3120, 14},
  {m68k_op_move_16_pd_di       , 0xf1f8, 0x3128, 16},
  {m68k_op_move_16_pd_ix       , 0xf1f8, 0x3130, 18},
  {m68k_op_move_16_di_d        , 0xf1f8, 0x3140, 12},
  {m68k_op_move_16_di_a        , 0xf1f8, 0x3148, 12},
  {m68k_op_move_16_di_ai       , 0xf1f8, 0x3150, 16},
  {m68k_op_move_16_di_pi       , 0xf1f8, 0x3158, 16},
  {m68k_op_move_16_di_pd       , 0xf1f8, 0x3160, 18},
  {m68k_op_move_16_di_di       , 0xf1f8, 0x3168, 20},
  {m68k_op_move_16_di_ix       , 0xf1f8, 0x3170, 22},
  {m68k_op_move_16_ix_d        , 0xf1f8, 0x3180, 14},
  {m68k_op_move_16_ix_a        , 0xf1f8, 0x3188, 14},
  {m68k_op_move_16_ix_ai       , 0xf1f8, 0x3190, 18},
  {m68k_op_move_16_ix_pi       , 0xf1f8, 0x3198, 18},
  {m68k_op_move_16_ix_pd       , 0xf1f8, 0x31a0, 20},
  {m68k_op_move_16_ix_di       , 0xf1f8, 0x31a8, 22},
  {m68k_op_move_16_ix_ix       , 0xf1f8, 0x31b0, 24},
  {m68k_op_chk_16_d            , 0xf1f8, 0x4180, 10},
  {m68k_op_chk_16_ai           , 0xf1f8, 0x4190, 14},
  {m68k_op_chk_16_pi           , 0xf1f8, 0x4198, 14},
  {m68k_op_chk_16_pd           , 0xf1f8, 0x41a0, 16},
  {m68k_op_chk_16_di           , 0xf1f8, 0x41a8, 18},
  {m68k_op_chk_16_ix           , 0xf1f8, 0x41b0, 20},
  {m68k_op_lea_32_ai           , 0xf1f8, 0x41d0,  4},
  {m68k_op_lea_32_di           , 0xf1f8, 0x41e8,  8},
  {m68k_op_lea_32_ix           , 0xf1f8, 0x41f0, 12},
  {m68k_op_addq_8_d            , 0xf1f8, 0x5000,  4},
  {m68k_op_addq_8_ai           , 0xf1f8, 0x5010, 12},
  {m68k_op_addq_8_pi           , 0xf1f8, 0x5018, 12},
  {m68k_op_addq_8_pd           , 0xf1f8, 0x5020, 14},
  {m68k_op_addq_8_di           , 0xf1f8, 0x5028, 16},
  {m68k_op_addq_8_ix           , 0xf1f8, 0x5030, 18},
  {m68k_op_addq_16_d           , 0xf1f8, 0x5040,  4},
  {m68k_op_addq_16_a           , 0xf1f8, 0x5048,  4},
  {m68k_op_addq_16_ai          , 0xf1f8, 0x5050, 12},
  {m68k_op_addq_16_pi          , 0xf1f8, 0x5058, 12},
  {m68k_op_addq_16_pd          , 0xf1f8, 0x5060, 14},
  {m68k_op_addq_16_di          , 0xf1f8, 0x5068, 16},
  {m68k_op_addq_16_ix          , 0xf1f8, 0x5070, 18},
  {m68k_op_addq_32_d           , 0xf1f8, 0x5080,  8},
  {m68k_op_addq_32_a           , 0xf1f8, 0x5088,  8},
  {m68k_op_addq_32_ai          , 0xf1f8, 0x5090, 20},
  {m68k_op_addq_32_pi          , 0xf1f8, 0x5098, 20},
  {m68k_op_addq_32_pd          , 0xf1f8, 0x50a0, 22},
  {m68k_op_addq_32_di          , 0xf1f8, 0x50a8, 24},
  {m68k_op_addq_32_ix          , 0xf1f8, 0x50b0, 26},
  {m68k_op_subq_8_d            , 0xf1f8, 0x5100,  4},
  {m68k_op_subq_8_ai           , 0xf1f8, 0x5110, 12},
  {m68k_op_subq_8_pi           , 0xf1f8, 0x5118, 12},
  {m68k_op_subq_8_pd           , 0xf1f8, 0x5120, 14},
  {m68k_op_subq_8_di           , 0xf1f8, 0x5128, 16},
  {m68k_op_subq_8_ix           , 0xf1f8, 0x5130, 18},
  {m68k_op_subq_16_d           , 0xf1f8, 0x5140,  4},
  {m68k_op_subq_16_a           , 0xf1f8, 0x5148,  8},
  {m68k_op_subq_16_ai          , 0xf1f8, 0x5150, 12},
  {m68k_op_subq_16_pi          , 0xf1f8, 0x5158, 12},
  {m68k_op_subq_16_pd          , 0xf1f8, 0x5160, 14},
  {m68k_op_subq_16_di          , 0xf1f8, 0x5168, 16},
  {m68k_op_subq_16_ix          , 0xf1f8, 0x5170, 18},
  {m68k_op_subq_32_d           , 0xf1f8, 0x5180,  8},
  {m68k_op_subq_32_a           , 0xf1f8, 0x5188,  8},
  {m68k_op_subq_32_ai          , 0xf1f8, 0x5190, 20},
  {m68k_op_subq_32_pi          , 0xf1f8, 0x5198, 20},
  {m68k_op_subq_32_pd          , 0xf1f8, 0x51a0, 22},
  {m68k_op_subq_32_di          , 0xf1f8, 0x51a8, 24},
  {m68k_op_subq_32_ix          , 0xf1f8, 0x51b0, 26},
  {m68k_op_or_8_er_d           , 0xf1f8, 0x8000,  4},
  {m68k_op_or_8_er_ai          , 0xf1f8, 0x8010,  8},
  {m68k_op_or_8_er_pi          , 0xf1f8, 0x8018,  8},
  {m68k_op_or_8_er_pd          , 0xf1f8, 0x8020, 10},
  {m68k_op_or_8_er_di          , 0xf1f8, 0x8028, 12},
  {m68k_op_or_8_er_ix          , 0xf1f8, 0x8030, 14},
  {m68k_op_or_16_er_d          , 0xf1f8, 0x8040,  4},
  {m68k_op_or_16_er_ai         , 0xf1f8, 0x8050,  8},
  {m68k_op_or_16_er_pi         , 0xf1f8, 0x8058,  8},
  {m68k_op_or_16_er_pd         , 0xf1f8, 0x8060, 10},
  {m68k_op_or_16_er_di         , 0xf1f8, 0x8068, 12},
  {m68k_op_or_16_er_ix         , 0xf1f8, 0x8070, 14},
  {m68k_op_or_32_er_d          , 0xf1f8, 0x8080,  8},
  {m68k_op_or_32_er_ai         , 0xf1f8, 0x8090, 14},
  {m68k_op_or_32_er_pi         , 0xf1f8, 0x8098, 14},
  {m68k_op_or_32_er_pd         , 0xf1f8, 0x80a0, 16},
  {m68k_op_or_32_er_di         , 0xf1f8, 0x80a8, 18},
  {m68k_op_or_32_er_ix         , 0xf1f8, 0x80b0, 20},
  {m68k_op_divu_16_d           , 0xf1f8, 0x80c0,  0},
  {m68k_op_divu_16_ai          , 0xf1f8, 0x80d0,  4},
  {m68k_op_divu_16_pi          , 0xf1f8, 0x80d8,  4},
  {m68k_op_divu_16_pd          , 0xf1f8, 0x80e0,  6},
  {m68k_op_divu_16_di          , 0xf1f8, 0x80e8,  8},
  {m68k_op_divu_16_ix          , 0xf1f8, 0x80f0, 10},
  {m68k_op_sbcd_8_rr           , 0xf1f8, 0x8100,  6},
  {m68k_op_sbcd_8_mm           , 0xf1f8, 0x8108, 18},
  {m68k_op_or_8_re_ai          , 0xf1f8, 0x8110, 12},
  {m68k_op_or_8_re_pi          , 0xf1f8, 0x8118, 12},
  {m68k_op_or_8_re_pd          , 0xf1f8, 0x8120, 14},
  {m68k_op_or_8_re_di          , 0xf1f8, 0x8128, 16},
  {m68k_op_or_8_re_ix          , 0xf1f8, 0x8130, 18},
  {m68k_op_or_16_re_ai         , 0xf1f8, 0x8150, 12},
  {m68k_op_or_16_re_pi         , 0xf1f8, 0x8158, 12},
  {m68k_op_or_16_re_pd         , 0xf1f8, 0x8160, 14},
  {m68k_op_or_16_re_di         , 0xf1f8, 0x8168, 16},
  {m68k_op_or_16_re_ix         , 0xf1f8, 0x8170, 18},
  {m68k_op_or_32_re_ai         , 0xf1f8, 0x8190, 20},
  {m68k_op_or_32_re_pi         , 0xf1f8, 0x8198, 20},
  {m68k_op_or_32_re_pd         , 0xf1f8, 0x81a0, 22},
  {m68k_op_or_32_re_di         , 0xf1f8, 0x81a8, 24},
  {m68k_op_or_32_re_ix         , 0xf1f8, 0x81b0, 26},
  {m68k_op_divs_16_d           , 0xf1f8, 0x81c0,  0},
  {m68k_op_divs_16_ai          , 0xf1f8, 0x81d0,  4},
  {m68k_op_divs_16_pi          , 0xf1f8, 0x81d8,  4},
  {m68k_op_divs_16_pd          , 0xf1f8, 0x81e0,  6},
  {m68k_op_divs_16_di          , 0xf1f8, 0x81e8,  8},
  {m68k_op_divs_16_ix          , 0xf1f8, 0x81f0, 10},
  {m68k_op_sub_8_er_d          , 0xf1f8, 0x9000,  4},
  {m68k_op_sub_8_er_ai         , 0xf1f8, 0x9010,  8},
  {m68k_op_sub_8_er_pi         , 0xf1f8, 0x9018,  8},
  {m68k_op_sub_8_er_pd         , 0xf1f8, 0x9020, 10},
  {m68k_op_sub_8_er_di         , 0xf1f8, 0x9028, 12},
  {m68k_op_sub_8_er_ix         , 0xf1f8, 0x9030, 14},
  {m68k_op_sub_16_er_d         , 0xf1f8, 0x9040,  4},
  {m68k_op_sub_16_er_a         , 0xf1f8, 0x9048,  4},
  {m68k_op_sub_16_er_ai        , 0xf1f8, 0x9050,  8},
  {m68k_op_sub_16_er_pi        , 0xf1f8, 0x9058,  8},
  {m68k_op_sub_16_er_pd        , 0xf1f8, 0x9060, 10},
  {m68k_op_sub_16_er_di        , 0xf1f8, 0x9068, 12},
  {m68k_op_sub_16_er_ix        , 0xf1f8, 0x9070, 14},
  {m68k_op_sub_32_er_d         , 0xf1f8, 0x9080,  8},
  {m68k_op_sub_32_er_a         , 0xf1f8, 0x9088,  8},
  {m68k_op_sub_32_er_ai        , 0xf1f8, 0x9090, 14},
  {m68k_op_sub_32_er_pi        , 0xf1f8, 0x9098, 14},
  {m68k_op_sub_32_er_pd        , 0xf1f8, 0x90a0, 16},
  {m68k_op_sub_32_er_di        , 0xf1f8, 0x90a8, 18},
  {m68k_op_sub_32_er_ix        , 0xf1f8, 0x90b0, 20},
  {m68k_op_suba_16_d           , 0xf1f8, 0x90c0,  8},
  {m68k_op_suba_16_a           , 0xf1f8, 0x90c8,  8},
  {m68k_op_suba_16_ai          , 0xf1f8, 0x90d0, 12},
  {m68k_op_suba_16_pi          , 0xf1f8, 0x90d8, 12},
  {m68k_op_suba_16_pd          , 0xf1f8, 0x90e0, 14},
  {m68k_op_suba_16_di          , 0xf1f8, 0x90e8, 16},
  {m68k_op_suba_16_ix          , 0xf1f8, 0x90f0, 18},
  {m68k_op_subx_8_rr           , 0xf1f8, 0x9100,  4},
  {m68k_op_subx_8_mm           , 0xf1f8, 0x9108, 18},
  {m68k_op_sub_8_re_ai         , 0xf1f8, 0x9110, 12},
  {m68k_op_sub_8_re_pi         , 0xf1f8, 0x9118, 12},
  {m68k_op_sub_8_re_pd         , 0xf1f8, 0x9120, 14},
  {m68k_op_sub_8_re_di         , 0xf1f8, 0x9128, 16},
  {m68k_op_sub_8_re_ix         , 0xf1f8, 0x9130, 18},
  {m68k_op_subx_16_rr          , 0xf1f8, 0x9140,  4},
  {m68k_op_subx_16_mm          , 0xf1f8, 0x9148, 18},
  {m68k_op_sub_16_re_ai        , 0xf1f8, 0x9150, 12},
  {m68k_op_sub_16_re_pi        , 0xf1f8, 0x9158, 12},
  {m68k_op_sub_16_re_pd        , 0xf1f8, 0x9160, 14},
  {m68k_op_sub_16_re_di        , 0xf1f8, 0x9168, 16},
  {m68k_op_sub_16_re_ix        , 0xf1f8, 0x9170, 18},
  {m68k_op_subx_32_rr          , 0xf1f8, 0x9180,  8},
  {m68k_op_subx_32_mm          , 0xf1f8, 0x9188, 30},
  {m68k_op_sub_32_re_ai        , 0xf1f8, 0x9190, 20},
  {m68k_op_sub_32_re_pi        , 0xf1f8, 0x9198, 20},
  {m68k_op_sub_32_re_pd        , 0xf1f8, 0x91a0, 22},
  {m68k_op_sub_32_re_di        , 0xf1f8, 0x91a8, 24},
  {m68k_op_sub_32_re_ix        , 0xf1f8, 0x91b0, 26},
  {m68k_op_suba_32_d           , 0xf1f8, 0x91c0,  8},
  {m68k_op_suba_32_a           , 0xf1f8, 0x91c8,  8},
  {m68k_op_suba_32_ai          , 0xf1f8, 0x91d0, 14},
  {m68k_op_suba_32_pi          , 0xf1f8, 0x91d8, 14},
  {m68k_op_suba_32_pd          , 0xf1f8, 0x91e0, 16},
  {m68k_op_suba_32_di          , 0xf1f8, 0x91e8, 18},
  {m68k_op_suba_32_ix          , 0xf1f8, 0x91f0, 20},
  {m68k_op_cmp_8_d             , 0xf1f8, 0xb000,  4},
  {m68k_op_cmp_8_ai            , 0xf1f8, 0xb010,  8},
  {m68k_op_cmp_8_pi            , 0xf1f8, 0xb018,  8},
  {m68k_op_cmp_8_pd            , 0xf1f8, 0xb020, 10},
  {m68k_op_cmp_8_di            , 0xf1f8, 0xb028, 12},
  {m68k_op_cmp_8_ix            , 0xf1f8, 0xb030, 14},
  {m68k_op_cmp_16_d            , 0xf1f8, 0xb040,  4},
  {m68k_op_cmp_16_a            , 0xf1f8, 0xb048,  4},
  {m68k_op_cmp_16_ai           , 0xf1f8, 0xb050,  8},
  {m68k_op_cmp_16_pi           , 0xf1f8, 0xb058,  8},
  {m68k_op_cmp_16_pd           , 0xf1f8, 0xb060, 10},
  {m68k_op_cmp_16_di           , 0xf1f8, 0xb068, 12},
  {m68k_op_cmp_16_ix           , 0xf1f8, 0xb070, 14},
  {m68k_op_cmp_32_d            , 0xf1f8, 0xb080,  6},
  {m68k_op_cmp_32_a            , 0xf1f8, 0xb088,  6},
  {m68k_op_cmp_32_ai           , 0xf1f8, 0xb090, 14},
  {m68k_op_cmp_32_pi           , 0xf1f8, 0xb098, 14},
  {m68k_op_cmp_32_pd           , 0xf1f8, 0xb0a0, 16},
  {m68k_op_cmp_32_di           , 0xf1f8, 0xb0a8, 18},
  {m68k_op_cmp_32_ix           , 0xf1f8, 0xb0b0, 20},
  {m68k_op_cmpa_16_d           , 0xf1f8, 0xb0c0,  6},
  {m68k_op_cmpa_16_a           , 0xf1f8, 0xb0c8,  6},
  {m68k_op_cmpa_16_ai          , 0xf1f8, 0xb0d0, 10},
  {m68k_op_cmpa_16_pi          , 0xf1f8, 0xb0d8, 10},
  {m68k_op_cmpa_16_pd          , 0xf1f8, 0xb0e0, 12},
  {m68k_op_cmpa_16_di          , 0xf1f8, 0xb0e8, 14},
  {m68k_op_cmpa_16_ix          , 0xf1f8, 0xb0f0, 16},
  {m68k_op_eor_8_d             , 0xf1f8, 0xb100,  4},
  {m68k_op_cmpm_8              , 0xf1f8, 0xb108, 12},
  {m68k_op_eor_8_ai            , 0xf1f8, 0xb110, 12},
  {m68k_op_eor_8_pi            , 0xf1f8, 0xb118, 12},
  {m68k_op_eor_8_pd            , 0xf1f8, 0xb120, 14},
  {m68k_op_eor_8_di            , 0xf1f8, 0xb128, 16},
  {m68k_op_eor_8_ix            , 0xf1f8, 0xb130, 18},
  {m68k_op_eor_16_d            , 0xf1f8, 0xb140,  4},
  {m68k_op_cmpm_16             , 0xf1f8, 0xb148, 12},
  {m68k_op_eor_16_ai           , 0xf1f8, 0xb150, 12},
  {m68k_op_eor_16_pi           , 0xf1f8, 0xb158, 12},
  {m68k_op_eor_16_pd           , 0xf1f8, 0xb160, 14},
  {m68k_op_eor_16_di           , 0xf1f8, 0xb168, 16},
  {m68k_op_eor_16_ix           , 0xf1f8, 0xb170, 18},
  {m68k_op_eor_32_d            , 0xf1f8, 0xb180,  8},
  {m68k_op_cmpm_32             , 0xf1f8, 0xb188, 20},
  {m68k_op_eor_32_ai           , 0xf1f8, 0xb190, 20},
  {m68k_op_eor_32_pi           , 0xf1f8, 0xb198, 20},
  {m68k_op_eor_32_pd           , 0xf1f8, 0xb1a0, 22},
  {m68k_op_eor_32_di           , 0xf1f8, 0xb1a8, 24},
  {m68k_op_eor_32_ix           , 0xf1f8, 0xb1b0, 26},
  {m68k_op_cmpa_32_d           , 0xf1f8, 0xb1c0,  6},
  {m68k_op_cmpa_32_a           , 0xf1f8, 0xb1c8,  6},
  {m68k_op_cmpa_32_ai          , 0xf1f8, 0xb1d0, 14},
  {m68k_op_cmpa_32_pi          , 0xf1f8, 0xb1d8, 14},
  {m68k_op_cmpa_32_pd          , 0xf1f8, 0xb1e0, 16},
  {m68k_op_cmpa_32_di          , 0xf1f8, 0xb1e8, 18},
  {m68k_op_cmpa_32_ix          , 0xf1f8, 0xb1f0, 20},
  {m68k_op_and_8_er_d          , 0xf1f8, 0xc000,  4},
  {m68k_op_and_8_er_ai         , 0xf1f8, 0xc010,  8},
  {m68k_op_and_8_er_pi         , 0xf1f8, 0xc018,  8},
  {m68k_op_and_8_er_pd         , 0xf1f8, 0xc020, 10},
  {m68k_op_and_8_er_di         , 0xf1f8, 0xc028, 12},
  {m68k_op_and_8_er_ix         , 0xf1f8, 0xc030, 14},
  {m68k_op_and_16_er_d         , 0xf1f8, 0xc040,  4},
  {m68k_op_and_16_er_ai        , 0xf1f8, 0xc050,  8},
  {m68k_op_and_16_er_pi        , 0xf1f8, 0xc058,  8},
  {m68k_op_and_16_er_pd        , 0xf1f8, 0xc060, 10},
  {m68k_op_and_16_er_di        , 0xf1f8, 0xc068, 12},
  {m68k_op_and_16_er_ix        , 0xf1f8, 0xc070, 14},
  {m68k_op_and_32_er_d         , 0xf1f8, 0xc080,  8},
  {m68k_op_and_32_er_ai        , 0xf1f8, 0xc090, 14},
  {m68k_op_and_32_er_pi        , 0xf1f8, 0xc098, 14},
  {m68k_op_and_32_er_pd        , 0xf1f8, 0xc0a0, 16},
  {m68k_op_and_32_er_di        , 0xf1f8, 0xc0a8, 18},
  {m68k_op_and_32_er_ix        , 0xf1f8, 0xc0b0, 20},
  {m68k_op_mulu_16_d           , 0xf1f8, 0xc0c0,  0},
  {m68k_op_mulu_16_ai          , 0xf1f8, 0xc0d0,  4},
  {m68k_op_mulu_16_pi          , 0xf1f8, 0xc0d8,  4},
  {m68k_op_mulu_16_pd          , 0xf1f8, 0xc0e0,  6},
  {m68k_op_mulu_16_di          , 0xf1f8, 0xc0e8,  8},
  {m68k_op_mulu_16_ix          , 0xf1f8, 0xc0f0, 10},
  {m68k_op_abcd_8_rr           , 0xf1f8, 0xc100,  6},
  {m68k_op_abcd_8_mm           , 0xf1f8, 0xc108, 18},
  {m68k_op_and_8_re_ai         , 0xf1f8, 0xc110, 12},
  {m68k_op_and_8_re_pi         , 0xf1f8, 0xc118, 12},
  {m68k_op_and_8_re_pd         , 0xf1f8, 0xc120, 14},
  {m68k_op_and_8_re_di         , 0xf1f8, 0xc128, 16},
  {m68k_op_and_8_re_ix         , 0xf1f8, 0xc130, 18},
  {m68k_op_exg_32_dd           , 0xf1f8, 0xc140,  6},
  {m68k_op_exg_32_aa           , 0xf1f8, 0xc148,  6},
  {m68k_op_and_16_re_ai        , 0xf1f8, 0xc150, 12},
  {m68k_op_and_16_re_pi        , 0xf1f8, 0xc158, 12},
  {m68k_op_and_16_re_pd        , 0xf1f8, 0xc160, 14},
  {m68k_op_and_16_re_di        , 0xf1f8, 0xc168, 16},
  {m68k_op_and_16_re_ix        , 0xf1f8, 0xc170, 18},
  {m68k_op_exg_32_da           , 0xf1f8, 0xc188,  6},
  {m68k_op_and_32_re_ai        , 0xf1f8, 0xc190, 20},
  {m68k_op_and_32_re_pi        , 0xf1f8, 0xc198, 20},
  {m68k_op_and_32_re_pd        , 0xf1f8, 0xc1a0, 22},
  {m68k_op_and_32_re_di        , 0xf1f8, 0xc1a8, 24},
  {m68k_op_and_32_re_ix        , 0xf1f8, 0xc1b0, 26},
  {m68k_op_muls_16_d           , 0xf1f8, 0xc1c0,  0},
  {m68k_op_muls_16_ai          , 0xf1f8, 0xc1d0,  4},
  {m68k_op_muls_16_pi          , 0xf1f8, 0xc1d8,  4},
  {m68k_op_muls_16_pd          , 0xf1f8, 0xc1e0,  6},
  {m68k_op_muls_16_di          , 0xf1f8, 0xc1e8,  8},
  {m68k_op_muls_16_ix          , 0xf1f8, 0xc1f0, 10},
  {m68k_op_add_8_er_d          , 0xf1f8, 0xd000,  4},
  {m68k_op_add_8_er_ai         , 0xf1f8, 0xd010,  8},
  {m68k_op_add_8_er_pi         , 0xf1f8, 0xd018,  8},
  {m68k_op_add_8_er_pd         , 0xf1f8, 0xd020, 10},
  {m68k_op_add_8_er_di         , 0xf1f8, 0xd028, 12},
  {m68k_op_add_8_er_ix         , 0xf1f8, 0xd030, 14},
  {m68k_op_add_16_er_d         , 0xf1f8, 0xd040,  4},
  {m68k_op_add_16_er_a         , 0xf1f8, 0xd048,  4},
  {m68k_op_add_16_er_ai        , 0xf1f8, 0xd050,  8},
  {m68k_op_add_16_er_pi        , 0xf1f8, 0xd058,  8},
  {m68k_op_add_16_er_pd        , 0xf1f8, 0xd060, 10},
  {m68k_op_add_16_er_di        , 0xf1f8, 0xd068, 12},
  {m68k_op_add_16_er_ix        , 0xf1f8, 0xd070, 14},
  {m68k_op_add_32_er_d         , 0xf1f8, 0xd080,  8},
  {m68k_op_add_32_er_a         , 0xf1f8, 0xd088,  8},
  {m68k_op_add_32_er_ai        , 0xf1f8, 0xd090, 14},
  {m68k_op_add_32_er_pi        , 0xf1f8, 0xd098, 14},
  {m68k_op_add_32_er_pd        , 0xf1f8, 0xd0a0, 16},
  {m68k_op_add_32_er_di        , 0xf1f8, 0xd0a8, 18},
  {m68k_op_add_32_er_ix        , 0xf1f8, 0xd0b0, 20},
  {m68k_op_adda_16_d           , 0xf1f8, 0xd0c0,  8},
  {m68k_op_adda_16_a           , 0xf1f8, 0xd0c8,  8},
  {m68k_op_adda_16_ai          , 0xf1f8, 0xd0d0, 12},
  {m68k_op_adda_16_pi          , 0xf1f8, 0xd0d8, 12},
  {m68k_op_adda_16_pd          , 0xf1f8, 0xd0e0, 14},
  {m68k_op_adda_16_di          , 0xf1f8, 0xd0e8, 16},
  {m68k_op_adda_16_ix          , 0xf1f8, 0xd0f0, 18},
  {m68k_op_addx_8_rr           , 0xf1f8, 0xd100,  4},
  {m68k_op_addx_8_mm           , 0xf1f8, 0xd108, 18},
  {m68k_op_add_8_re_ai         , 0xf1f8, 0xd110, 12},
  {m68k_op_add_8_re_pi         , 0xf1f8, 0xd118, 12},
  {m68k_op_add_8_re_pd         , 0xf1f8, 0xd120, 14},
  {m68k_op_add_8_re_di         , 0xf1f8, 0xd128, 16},
  {m68k_op_add_8_re_ix         , 0xf1f8, 0xd130, 18},
  {m68k_op_addx_16_rr          , 0xf1f8, 0xd140,  4},
  {m68k_op_addx_16_mm          , 0xf1f8, 0xd148, 18},
  {m68k_op_add_16_re_ai        , 0xf1f8, 0xd150, 12},
  {m68k_op_add_16_re_pi        , 0xf1f8, 0xd158, 12},
  {m68k_op_add_16_re_pd        , 0xf1f8, 0xd160, 14},
  {m68k_op_add_16_re_di        , 0xf1f8, 0xd168, 16},
  {m68k_op_add_16_re_ix        , 0xf1f8, 0xd170, 18},
  {m68k_op_addx_32_rr          , 0xf1f8, 0xd180,  8},
  {m68k_op_addx_32_mm          , 0xf1f8, 0xd188, 30},
  {m68k_op_add_32_re_ai        , 0xf1f8, 0xd190, 20},
  {m68k_op_add_32_re_pi        , 0xf1f8, 0xd198, 20},
  {m68k_op_add_32_re_pd        , 0xf1f8, 0xd1a0, 22},
  {m68k_op_add_32_re_di        , 0xf1f8, 0xd1a8, 24},
  {m68k_op_add_32_re_ix        , 0xf1f8, 0xd1b0, 26},
  {m68k_op_adda_32_d           , 0xf1f8, 0xd1c0,  8},
  {m68k_op_adda_32_a           , 0xf1f8, 0xd1c8,  8},
  {m68k_op_adda_32_ai          , 0xf1f8, 0xd1d0, 14},
  {m68k_op_adda_32_pi          , 0xf1f8, 0xd1d8, 14},
  {m68k_op_adda_32_pd          , 0xf1f8, 0xd1e0, 16},
  {m68k_op_adda_32_di          , 0xf1f8, 0xd1e8, 18},
  {m68k_op_adda_32_ix          , 0xf1f8, 0xd1f0, 20},
  {m68k_op_asr_8_s             , 0xf1f8, 0xe000,  6},
  {m68k_op_lsr_8_s             , 0xf1f8, 0xe008,  6},
  {m68k_op_roxr_8_s            , 0xf1f8, 0xe010,  6},
  {m68k_op_ror_8_s             , 0xf1f8, 0xe018,  6},
  {m68k_op_asr_8_r             , 0xf1f8, 0xe020,  6},
  {m68k_op_lsr_8_r             , 0xf1f8, 0xe028,  6},
  {m68k_op_roxr_8_r            , 0xf1f8, 0xe030,  6},
  {m68k_op_ror_8_r             , 0xf1f8, 0xe038,  6},
  {m68k_op_asr_16_s            , 0xf1f8, 0xe040,  6},
  {m68k_op_lsr_16_s            , 0xf1f8, 0xe048,  6},
  {m68k_op_roxr_16_s           , 0xf1f8, 0xe050,  6},
  {m68k_op_ror_16_s            , 0xf1f8, 0xe058,  6},
  {m68k_op_asr_16_r            , 0xf1f8, 0xe060,  6},
  {m68k_op_lsr_16_r            , 0xf1f8, 0xe068,  6},
  {m68k_op_roxr_16_r           , 0xf1f8, 0xe070,  6},
  {m68k_op_ror_16_r            , 0xf1f8, 0xe078,  6},
  {m68k_op_asr_32_s            , 0xf1f8, 0xe080,  8},
  {m68k_op_lsr_32_s            , 0xf1f8, 0xe088,  8},
  {m68k_op_roxr_32_s           , 0xf1f8, 0xe090,  8},
  {m68k_op_ror_32_s            , 0xf1f8, 0xe098,  8},
  {m68k_op_asr_32_r            , 0xf1f8, 0xe0a0,  8},
  {m68k_op_lsr_32_r            , 0xf1f8, 0xe0a8,  8},
  {m68k_op_roxr_32_r           , 0xf1f8, 0xe0b0,  8},
  {m68k_op_ror_32_r            , 0xf1f8, 0xe0b8,  8},
  {m68k_op_asl_8_s             , 0xf1f8, 0xe100,  6},
  {m68k_op_lsl_8_s             , 0xf1f8, 0xe108,  6},
  {m68k_op_roxl_8_s            , 0xf1f8, 0xe110,  6},
  {m68k_op_rol_8_s             , 0xf1f8, 0xe118,  6},
  {m68k_op_asl_8_r             , 0xf1f8, 0xe120,  6},
  {m68k_op_lsl_8_r             , 0xf1f8, 0xe128,  6},
  {m68k_op_roxl_8_r            , 0xf1f8, 0xe130,  6},
  {m68k_op_rol_8_r             , 0xf1f8, 0xe138,  6},
  {m68k_op_asl_16_s            , 0xf1f8, 0xe140,  6},
  {m68k_op_lsl_16_s            , 0xf1f8, 0xe148,  6},
  {m68k_op_roxl_16_s           , 0xf1f8, 0xe150,  6},
  {m68k_op_rol_16_s            , 0xf1f8, 0xe158,  6},
  {m68k_op_asl_16_r            , 0xf1f8, 0xe160,  6},
  {m68k_op_lsl_16_r            , 0xf1f8, 0xe168,  6},
  {m68k_op_roxl_16_r           , 0xf1f8, 0xe170,  6},
  {m68k_op_rol_16_r            , 0xf1f8, 0xe178,  6},
  {m68k_op_asl_32_s            , 0xf1f8, 0xe180,  8},
  {m68k_op_lsl_32_s            , 0xf1f8, 0xe188,  8},
  {m68k_op_roxl_32_s           , 0xf1f8, 0xe190,  8},
  {m68k_op_rol_32_s            , 0xf1f8, 0xe198,  8},
  {m68k_op_asl_32_r            , 0xf1f8, 0xe1a0,  8},
  {m68k_op_lsl_32_r            , 0xf1f8, 0xe1a8,  8},
  {m68k_op_roxl_32_r           , 0xf1f8, 0xe1b0,  8},
  {m68k_op_rol_32_r            , 0xf1f8, 0xe1b8,  8},
  {m68k_op_trap                , 0xfff0, 0x4e40,  4},
  {m68k_op_btst_8_r_pi7        , 0xf1ff, 0x011f,  8},
  {m68k_op_btst_8_r_pd7        , 0xf1ff, 0x0127, 10},
  {m68k_op_btst_8_r_aw         , 0xf1ff, 0x0138, 12},
  {m68k_op_btst_8_r_al         , 0xf1ff, 0x0139, 16},
  {m68k_op_btst_8_r_pcdi       , 0xf1ff, 0x013a, 12},
  {m68k_op_btst_8_r_pcix       , 0xf1ff, 0x013b, 14},
  {m68k_op_btst_8_r_i          , 0xf1ff, 0x013c,  8},
  {m68k_op_bchg_8_r_pi7        , 0xf1ff, 0x015f, 12},
  {m68k_op_bchg_8_r_pd7        , 0xf1ff, 0x0167, 14},
  {m68k_op_bchg_8_r_aw         , 0xf1ff, 0x0178, 16},
  {m68k_op_bchg_8_r_al         , 0xf1ff, 0x0179, 20},
  {m68k_op_bclr_8_r_pi7        , 0xf1ff, 0x019f, 12},
  {m68k_op_bclr_8_r_pd7        , 0xf1ff, 0x01a7, 14},
  {m68k_op_bclr_8_r_aw         , 0xf1ff, 0x01b8, 16},
  {m68k_op_bclr_8_r_al         , 0xf1ff, 0x01b9, 20},
  {m68k_op_bset_8_r_pi7        , 0xf1ff, 0x01df, 12},
  {m68k_op_bset_8_r_pd7        , 0xf1ff, 0x01e7, 14},
  {m68k_op_bset_8_r_aw         , 0xf1ff, 0x01f8, 16},
  {m68k_op_bset_8_r_al         , 0xf1ff, 0x01f9, 20},
  {m68k_op_move_8_d_pi7        , 0xf1ff, 0x101f,  8},
  {m68k_op_move_8_d_pd7        , 0xf1ff, 0x1027, 10},
  {m68k_op_move_8_d_aw         , 0xf1ff, 0x1038, 12},
  {m68k_op_move_8_d_al         , 0xf1ff, 0x1039, 16},
  {m68k_op_move_8_d_pcdi       , 0xf1ff, 0x103a, 12},
  {m68k_op_move_8_d_pcix       , 0xf1ff, 0x103b, 14},
  {m68k_op_move_8_d_i          , 0xf1ff, 0x103c,  8},
  {m68k_op_move_8_ai_pi7       , 0xf1ff, 0x109f, 12},
  {m68k_op_move_8_ai_pd7       , 0xf1ff, 0x10a7, 14},
  {m68k_op_move_8_ai_aw        , 0xf1ff, 0x10b8, 16},
  {m68k_op_move_8_ai_al        , 0xf1ff, 0x10b9, 20},
  {m68k_op_move_8_ai_pcdi      , 0xf1ff, 0x10ba, 16},
  {m68k_op_move_8_ai_pcix      , 0xf1ff, 0x10bb, 18},
  {m68k_op_move_8_ai_i         , 0xf1ff, 0x10bc, 12},
  {m68k_op_move_8_pi_pi7       , 0xf1ff, 0x10df, 12},
  {m68k_op_move_8_pi_pd7       , 0xf1ff, 0x10e7, 14},
  {m68k_op_move_8_pi_aw        , 0xf1ff, 0x10f8, 16},
  {m68k_op_move_8_pi_al        , 0xf1ff, 0x10f9, 20},
  {m68k_op_move_8_pi_pcdi      , 0xf1ff, 0x10fa, 16},
  {m68k_op_move_8_pi_pcix      , 0xf1ff, 0x10fb, 18},
  {m68k_op_move_8_pi_i         , 0xf1ff, 0x10fc, 12},
  {m68k_op_move_8_pd_pi7       , 0xf1ff, 0x111f, 12},
  {m68k_op_move_8_pd_pd7       , 0xf1ff, 0x1127, 14},
  {m68k_op_move_8_pd_aw        , 0xf1ff, 0x1138, 16},
  {m68k_op_move_8_pd_al        , 0xf1ff, 0x1139, 20},
  {m68k_op_move_8_pd_pcdi      , 0xf1ff, 0x113a, 16},
  {m68k_op_move_8_pd_pcix      , 0xf1ff, 0x113b, 18},
  {m68k_op_move_8_pd_i         , 0xf1ff, 0x113c, 12},
  {m68k_op_move_8_di_pi7       , 0xf1ff, 0x115f, 16},
  {m68k_op_move_8_di_pd7       , 0xf1ff, 0x1167, 18},
  {m68k_op_move_8_di_aw        , 0xf1ff, 0x1178, 20},
  {m68k_op_move_8_di_al        , 0xf1ff, 0x1179, 24},
  {m68k_op_move_8_di_pcdi      , 0xf1ff, 0x117a, 20},
  {m68k_op_move_8_di_pcix      , 0xf1ff, 0x117b, 22},
  {m68k_op_move_8_di_i         , 0xf1ff, 0x117c, 16},
  {m68k_op_move_8_ix_pi7       , 0xf1ff, 0x119f, 18},
  {m68k_op_move_8_ix_pd7       , 0xf1ff, 0x11a7, 20},
  {m68k_op_move_8_ix_aw        , 0xf1ff, 0x11b8, 22},
  {m68k_op_move_8_ix_al        , 0xf1ff, 0x11b9, 26},
  {m68k_op_move_8_ix_pcdi      , 0xf1ff, 0x11ba, 22},
  {m68k_op_move_8_ix_pcix      , 0xf1ff, 0x11bb, 24},
  {m68k_op_move_8_ix_i         , 0xf1ff, 0x11bc, 18},
  {m68k_op_move_32_d_aw        , 0xf1ff, 0x2038, 16},
  {m68k_op_move_32_d_al        , 0xf1ff, 0x2039, 20},
  {m68k_op_move_32_d_pcdi      , 0xf1ff, 0x203a, 16},
  {m68k_op_move_32_d_pcix      , 0xf1ff, 0x203b, 18},
  {m68k_op_move_32_d_i         , 0xf1ff, 0x203c, 12},
  {m68k_op_movea_32_aw         , 0xf1ff, 0x2078, 16},
  {m68k_op_movea_32_al         , 0xf1ff, 0x2079, 20},
  {m68k_op_movea_32_pcdi       , 0xf1ff, 0x207a, 16},
  {m68k_op_movea_32_pcix       , 0xf1ff, 0x207b, 18},
  {m68k_op_movea_32_i          , 0xf1ff, 0x207c, 12},
  {m68k_op_move_32_ai_aw       , 0xf1ff, 0x20b8, 24},
  {m68k_op_move_32_ai_al       , 0xf1ff, 0x20b9, 28},
  {m68k_op_move_32_ai_pcdi     , 0xf1ff, 0x20ba, 24},
  {m68k_op_move_32_ai_pcix     , 0xf1ff, 0x20bb, 26},
  {m68k_op_move_32_ai_i        , 0xf1ff, 0x20bc, 20},
  {m68k_op_move_32_pi_aw       , 0xf1ff, 0x20f8, 24},
  {m68k_op_move_32_pi_al       , 0xf1ff, 0x20f9, 28},
  {m68k_op_move_32_pi_pcdi     , 0xf1ff, 0x20fa, 24},
  {m68k_op_move_32_pi_pcix     , 0xf1ff, 0x20fb, 26},
  {m68k_op_move_32_pi_i        , 0xf1ff, 0x20fc, 20},
  {m68k_op_move_32_pd_aw       , 0xf1ff, 0x2138, 24},
  {m68k_op_move_32_pd_al       , 0xf1ff, 0x2139, 28},
  {m68k_op_move_32_pd_pcdi     , 0xf1ff, 0x213a, 24},
  {m68k_op_move_32_pd_pcix     , 0xf1ff, 0x213b, 26},
  {m68k_op_move_32_pd_i        , 0xf1ff, 0x213c, 20},
  {m68k_op_move_32_di_aw       , 0xf1ff, 0x2178, 28},
  {m68k_op_move_32_di_al       , 0xf1ff, 0x2179, 32},
  {m68k_op_move_32_di_pcdi     , 0xf1ff, 0x217a, 28},
  {m68k_op_move_32_di_pcix     , 0xf1ff, 0x217b, 30},
  {m68k_op_move_32_di_i        , 0xf1ff, 0x217c, 24},
  {m68k_op_move_32_ix_aw       , 0xf1ff, 0x21b8, 30},
  {m68k_op_move_32_ix_al       , 0xf1ff, 0x21b9, 34},
  {m68k_op_move_32_ix_pcdi     , 0xf1ff, 0x21ba, 30},
  {m68k_op_move_32_ix_pcix     , 0xf1ff, 0x21bb, 32},
  {m68k_op_move_32_ix_i        , 0xf1ff, 0x21bc, 26},
  {m68k_op_move_16_d_aw        , 0xf1ff, 0x3038, 12},
  {m68k_op_move_16_d_al        , 0xf1ff, 0x3039, 16},
  {m68k_op_move_16_d_pcdi      , 0xf1ff, 0x303a, 12},
  {m68k_op_move_16_d_pcix      , 0xf1ff, 0x303b, 14},
  {m68k_op_move_16_d_i         , 0xf1ff, 0x303c,  8},
  {m68k_op_movea_16_aw         , 0xf1ff, 0x3078, 12},
  {m68k_op_movea_16_al         , 0xf1ff, 0x3079, 16},
  {m68k_op_movea_16_pcdi       , 0xf1ff, 0x307a, 12},
  {m68k_op_movea_16_pcix       , 0xf1ff, 0x307b, 14},
  {m68k_op_movea_16_i          , 0xf1ff, 0x307c,  8},
  {m68k_op_move_16_ai_aw       , 0xf1ff, 0x30b8, 16},
  {m68k_op_move_16_ai_al       , 0xf1ff, 0x30b9, 20},
  {m68k_op_move_16_ai_pcdi     , 0xf1ff, 0x30ba, 16},
  {m68k_op_move_16_ai_pcix     , 0xf1ff, 0x30bb, 18},
  {m68k_op_move_16_ai_i        , 0xf1ff, 0x30bc, 12},
  {m68k_op_move_16_pi_aw       , 0xf1ff, 0x30f8, 16},
  {m68k_op_move_16_pi_al       , 0xf1ff, 0x30f9, 20},
  {m68k_op_move_16_pi_pcdi     , 0xf1ff, 0x30fa, 16},
  {m68k_op_move_16_pi_pcix     , 0xf1ff, 0x30fb, 18},
  {m68k_op_move_16_pi_i        , 0xf1ff, 0x30fc, 12},
  {m68k_op_move_16_pd_aw       , 0xf1ff, 0x3138, 16},
  {m68k_op_move_16_pd_al       , 0xf1ff, 0x3139, 20},
  {m68k_op_move_16_pd_pcdi     , 0xf1ff, 0x313a, 16},
  {m68k_op_move_16_pd_pcix     , 0xf1ff, 0x313b, 18},
  {m68k_op_move_16_pd_i        , 0xf1ff, 0x313c, 12},
  {m68k_op_move_16_di_aw       , 0xf1ff, 0x3178, 20},
  {m68k_op_move_16_di_al       , 0xf1ff, 0x3179, 24},
  {m68k_op_move_16_di_pcdi     , 0xf1ff, 0x317a, 20},
  {m68k_op_move_16_di_pcix     , 0xf1ff, 0x317b, 22},
  {m68k_op_move_16_di_i        , 0xf1ff, 0x317c, 16},
  {m68k_op_move_16_ix_aw       , 0xf1ff, 0x31b8, 22},
  {m68k_op_move_16_ix_al       , 0xf1ff, 0x31b9, 26},
  {m68k_op_move_16_ix_pcdi     , 0xf1ff, 0x31ba, 22},
  {m68k_op_move_16_ix_pcix     , 0xf1ff, 0x31bb, 24},
  {m68k_op_move_16_ix_i        , 0xf1ff, 0x31bc, 18},
  {m68k_op_chk_16_aw           , 0xf1ff, 0x41b8, 18},
  {m68k_op_chk_16_al           , 0xf1ff, 0x41b9, 22},
  {m68k_op_chk_16_pcdi         , 0xf1ff, 0x41ba, 18},
  {m68k_op_chk_16_pcix         , 0xf1ff, 0x41bb, 20},
  {m68k_op_chk_16_i            , 0xf1ff, 0x41bc, 14},
  {m68k_op_lea_32_aw           , 0xf1ff, 0x41f8,  8},
  {m68k_op_lea_32_al           , 0xf1ff, 0x41f9, 12},
  {m68k_op_lea_32_pcdi         , 0xf1ff, 0x41fa,  8},
  {m68k_op_lea_32_pcix         , 0xf1ff, 0x41fb, 12},
  {m68k_op_addq_8_pi7          , 0xf1ff, 0x501f, 12},
  {m68k_op_addq_8_pd7          , 0xf1ff, 0x5027, 14},
  {m68k_op_addq_8_aw           , 0xf1ff, 0x5038, 16},
  {m68k_op_addq_8_al           , 0xf1ff, 0x5039, 20},
  {m68k_op_addq_16_aw          , 0xf1ff, 0x5078, 16},
  {m68k_op_addq_16_al          , 0xf1ff, 0x5079, 20},
  {m68k_op_addq_32_aw          , 0xf1ff, 0x50b8, 24},
  {m68k_op_addq_32_al          , 0xf1ff, 0x50b9, 28},
  {m68k_op_subq_8_pi7          , 0xf1ff, 0x511f, 12},
  {m68k_op_subq_8_pd7          , 0xf1ff, 0x5127, 14},
  {m68k_op_subq_8_aw           , 0xf1ff, 0x5138, 16},
  {m68k_op_subq_8_al           , 0xf1ff, 0x5139, 20},
  {m68k_op_subq_16_aw          , 0xf1ff, 0x5178, 16},
  {m68k_op_subq_16_al          , 0xf1ff, 0x5179, 20},
  {m68k_op_subq_32_aw          , 0xf1ff, 0x51b8, 24},
  {m68k_op_subq_32_al          , 0xf1ff, 0x51b9, 28},
  {m68k_op_or_8_er_pi7         , 0xf1ff, 0x801f,  8},
  {m68k_op_or_8_er_pd7         , 0xf1ff, 0x8027, 10},
  {m68k_op_or_8_er_aw          , 0xf1ff, 0x8038, 12},
  {m68k_op_or_8_er_al          , 0xf1ff, 0x8039, 16},
  {m68k_op_or_8_er_pcdi        , 0xf1ff, 0x803a, 12},
  {m68k_op_or_8_er_pcix        , 0xf1ff, 0x803b, 14},
  {m68k_op_or_8_er_i           , 0xf1ff, 0x803c,  8},
  {m68k_op_or_16_er_aw         , 0xf1ff, 0x8078, 12},
  {m68k_op_or_16_er_al         , 0xf1ff, 0x8079, 16},
  {m68k_op_or_16_er_pcdi       , 0xf1ff, 0x807a, 12},
  {m68k_op_or_16_er_pcix       , 0xf1ff, 0x807b, 14},
  {m68k_op_or_16_er_i          , 0xf1ff, 0x807c,  8},
  {m68k_op_or_32_er_aw         , 0xf1ff, 0x80b8, 18},
  {m68k_op_or_32_er_al         , 0xf1ff, 0x80b9, 22},
  {m68k_op_or_32_er_pcdi       , 0xf1ff, 0x80ba, 18},
  {m68k_op_or_32_er_pcix       , 0xf1ff, 0x80bb, 20},
  {m68k_op_or_32_er_i          , 0xf1ff, 0x80bc, 16},
  {m68k_op_divu_16_aw          , 0xf1ff, 0x80f8,  8},
  {m68k_op_divu_16_al          , 0xf1ff, 0x80f9, 12},
  {m68k_op_divu_16_pcdi        , 0xf1ff, 0x80fa,  8},
  {m68k_op_divu_16_pcix        , 0xf1ff, 0x80fb, 10},
  {m68k_op_divu_16_i           , 0xf1ff, 0x80fc,  4},
  {m68k_op_sbcd_8_mm_ay7       , 0xf1ff, 0x810f, 18},
  {m68k_op_or_8_re_pi7         , 0xf1ff, 0x811f, 12},
  {m68k_op_or_8_re_pd7         , 0xf1ff, 0x8127, 14},
  {m68k_op_or_8_re_aw          , 0xf1ff, 0x8138, 16},
  {m68k_op_or_8_re_al          , 0xf1ff, 0x8139, 20},
  {m68k_op_or_16_re_aw         , 0xf1ff, 0x8178, 16},
  {m68k_op_or_16_re_al         , 0xf1ff, 0x8179, 20},
  {m68k_op_or_32_re_aw         , 0xf1ff, 0x81b8, 24},
  {m68k_op_or_32_re_al         , 0xf1ff, 0x81b9, 28},
  {m68k_op_divs_16_aw          , 0xf1ff, 0x81f8,  8},
  {m68k_op_divs_16_al          , 0xf1ff, 0x81f9, 12},
  {m68k_op_divs_16_pcdi        , 0xf1ff, 0x81fa,  8},
  {m68k_op_divs_16_pcix        , 0xf1ff, 0x81fb, 10},
  {m68k_op_divs_16_i           , 0xf1ff, 0x81fc,  4},
  {m68k_op_sub_8_er_pi7        , 0xf1ff, 0x901f,  8},
  {m68k_op_sub_8_er_pd7        , 0xf1ff, 0x9027, 10},
  {m68k_op_sub_8_er_aw         , 0xf1ff, 0x9038, 12},
  {m68k_op_sub_8_er_al         , 0xf1ff, 0x9039, 16},
  {m68k_op_sub_8_er_pcdi       , 0xf1ff, 0x903a, 12},
  {m68k_op_sub_8_er_pcix       , 0xf1ff, 0x903b, 14},
  {m68k_op_sub_8_er_i          , 0xf1ff, 0x903c,  8},
  {m68k_op_sub_16_er_aw        , 0xf1ff, 0x9078, 12},
  {m68k_op_sub_16_er_al        , 0xf1ff, 0x9079, 16},
  {m68k_op_sub_16_er_pcdi      , 0xf1ff, 0x907a, 12},
  {m68k_op_sub_16_er_pcix      , 0xf1ff, 0x907b, 14},
  {m68k_op_sub_16_er_i         , 0xf1ff, 0x907c,  8},
  {m68k_op_sub_32_er_aw        , 0xf1ff, 0x90b8, 18},
  {m68k_op_sub_32_er_al        , 0xf1ff, 0x90b9, 22},
  {m68k_op_sub_32_er_pcdi      , 0xf1ff, 0x90ba, 18},
  {m68k_op_sub_32_er_pcix      , 0xf1ff, 0x90bb, 20},
  {m68k_op_sub_32_er_i         , 0xf1ff, 0x90bc, 16},
  {m68k_op_suba_16_aw          , 0xf1ff, 0x90f8, 16},
  {m68k_op_suba_16_al          , 0xf1ff, 0x90f9, 20},
  {m68k_op_suba_16_pcdi        , 0xf1ff, 0x90fa, 16},
  {m68k_op_suba_16_pcix        , 0xf1ff, 0x90fb, 18},
  {m68k_op_suba_16_i           , 0xf1ff, 0x90fc, 12},
  {m68k_op_subx_8_mm_ay7       , 0xf1ff, 0x910f, 18},
  {m68k_op_sub_8_re_pi7        , 0xf1ff, 0x911f, 12},
  {m68k_op_sub_8_re_pd7        , 0xf1ff, 0x9127, 14},
  {m68k_op_sub_8_re_aw         , 0xf1ff, 0x9138, 16},
  {m68k_op_sub_8_re_al         , 0xf1ff, 0x9139, 20},
  {m68k_op_sub_16_re_aw        , 0xf1ff, 0x9178, 16},
  {m68k_op_sub_16_re_al        , 0xf1ff, 0x9179, 20},
  {m68k_op_sub_32_re_aw        , 0xf1ff, 0x91b8, 24},
  {m68k_op_sub_32_re_al        , 0xf1ff, 0x91b9, 28},
  {m68k_op_suba_32_aw          , 0xf1ff, 0x91f8, 18},
  {m68k_op_suba_32_al          , 0xf1ff, 0x91f9, 22},
  {m68k_op_suba_32_pcdi        , 0xf1ff, 0x91fa, 18},
  {m68k_op_suba_32_pcix        , 0xf1ff, 0x91fb, 20},
  {m68k_op_suba_32_i           , 0xf1ff, 0x91fc, 16},
  {m68k_op_cmp_8_pi7           , 0xf1ff, 0xb01f,  8},
  {m68k_op_cmp_8_pd7           , 0xf1ff, 0xb027, 10},
  {m68k_op_cmp_8_aw            , 0xf1ff, 0xb038, 12},
  {m68k_op_cmp_8_al            , 0xf1ff, 0xb039, 16},
  {m68k_op_cmp_8_pcdi          , 0xf1ff, 0xb03a, 12},
  {m68k_op_cmp_8_pcix          , 0xf1ff, 0xb03b, 14},
  {m68k_op_cmp_8_i             , 0xf1ff, 0xb03c,  8},
  {m68k_op_cmp_16_aw           , 0xf1ff, 0xb078, 12},
  {m68k_op_cmp_16_al           , 0xf1ff, 0xb079, 16},
  {m68k_op_cmp_16_pcdi         , 0xf1ff, 0xb07a, 12},
  {m68k_op_cmp_16_pcix         , 0xf1ff, 0xb07b, 14},
  {m68k_op_cmp_16_i            , 0xf1ff, 0xb07c,  8},
  {m68k_op_cmp_32_aw           , 0xf1ff, 0xb0b8, 18},
  {m68k_op_cmp_32_al           , 0xf1ff, 0xb0b9, 22},
  {m68k_op_cmp_32_pcdi         , 0xf1ff, 0xb0ba, 18},
  {m68k_op_cmp_32_pcix         , 0xf1ff, 0xb0bb, 20},
  {m68k_op_cmp_32_i            , 0xf1ff, 0xb0bc, 14},
  {m68k_op_cmpa_16_aw          , 0xf1ff, 0xb0f8, 14},
  {m68k_op_cmpa_16_al          , 0xf1ff, 0xb0f9, 18},
  {m68k_op_cmpa_16_pcdi        , 0xf1ff, 0xb0fa, 14},
  {m68k_op_cmpa_16_pcix        , 0xf1ff, 0xb0fb, 16},
  {m68k_op_cmpa_16_i           , 0xf1ff, 0xb0fc, 10},
  {m68k_op_cmpm_8_ay7          , 0xf1ff, 0xb10f, 12},
  {m68k_op_eor_8_pi7           , 0xf1ff, 0xb11f, 12},
  {m68k_op_eor_8_pd7           , 0xf1ff, 0xb127, 14},
  {m68k_op_eor_8_aw            , 0xf1ff, 0xb138, 16},
  {m68k_op_eor_8_al            , 0xf1ff, 0xb139, 20},
  {m68k_op_eor_16_aw           , 0xf1ff, 0xb178, 16},
  {m68k_op_eor_16_al           , 0xf1ff, 0xb179, 20},
  {m68k_op_eor_32_aw           , 0xf1ff, 0xb1b8, 24},
  {m68k_op_eor_32_al           , 0xf1ff, 0xb1b9, 28},
  {m68k_op_cmpa_32_aw          , 0xf1ff, 0xb1f8, 18},
  {m68k_op_cmpa_32_al          , 0xf1ff, 0xb1f9, 22},
  {m68k_op_cmpa_32_pcdi        , 0xf1ff, 0xb1fa, 18},
  {m68k_op_cmpa_32_pcix        , 0xf1ff, 0xb1fb, 20},
  {m68k_op_cmpa_32_i           , 0xf1ff, 0xb1fc, 14},
  {m68k_op_and_8_er_pi7        , 0xf1ff, 0xc01f,  8},
  {m68k_op_and_8_er_pd7        , 0xf1ff, 0xc027, 10},
  {m68k_op_and_8_er_aw         , 0xf1ff, 0xc038, 12},
  {m68k_op_and_8_er_al         , 0xf1ff, 0xc039, 16},
  {m68k_op_and_8_er_pcdi       , 0xf1ff, 0xc03a, 12},
  {m68k_op_and_8_er_pcix       , 0xf1ff, 0xc03b, 14},
  {m68k_op_and_8_er_i          , 0xf1ff, 0xc03c,  8},
  {m68k_op_and_16_er_aw        , 0xf1ff, 0xc078, 12},
  {m68k_op_and_16_er_al        , 0xf1ff, 0xc079, 16},
  {m68k_op_and_16_er_pcdi      , 0xf1ff, 0xc07a, 12},
  {m68k_op_and_16_er_pcix      , 0xf1ff, 0xc07b, 14},
  {m68k_op_and_16_er_i         , 0xf1ff, 0xc07c,  8},
  {m68k_op_and_32_er_aw        , 0xf1ff, 0xc0b8, 18},
  {m68k_op_and_32_er_al        , 0xf1ff, 0xc0b9, 22},
  {m68k_op_and_32_er_pcdi      , 0xf1ff, 0xc0ba, 18},
  {m68k_op_and_32_er_pcix      , 0xf1ff, 0xc0bb, 20},
  {m68k_op_and_32_er_i         , 0xf1ff, 0xc0bc, 16},
  {m68k_op_mulu_16_aw          , 0xf1ff, 0xc0f8,  8},
  {m68k_op_mulu_16_al          , 0xf1ff, 0xc0f9, 12},
  {m68k_op_mulu_16_pcdi        , 0xf1ff, 0xc0fa,  8},
  {m68k_op_mulu_16_pcix        , 0xf1ff, 0xc0fb, 10},
  {m68k_op_mulu_16_i           , 0xf1ff, 0xc0fc,  4},
  {m68k_op_abcd_8_mm_ay7       , 0xf1ff, 0xc10f, 18},
  {m68k_op_and_8_re_pi7        , 0xf1ff, 0xc11f, 12},
  {m68k_op_and_8_re_pd7        , 0xf1ff, 0xc127, 14},
  {m68k_op_and_8_re_aw         , 0xf1ff, 0xc138, 16},
  {m68k_op_and_8_re_al         , 0xf1ff, 0xc139, 20},
  {m68k_op_and_16_re_aw        , 0xf1ff, 0xc178, 16},
  {m68k_op_and_16_re_al        , 0xf1ff, 0xc179, 20},
  {m68k_op_and_32_re_aw        , 0xf1ff, 0xc1b8, 24},
  {m68k_op_and_32_re_al        , 0xf1ff, 0xc1b9, 28},
  {m68k_op_muls_16_aw          , 0xf1ff, 0xc1f8,  8},
  {m68k_op_muls_16_al          , 0xf1ff, 0xc1f9, 12},
  {m68k_op_muls_16_pcdi        , 0xf1ff, 0xc1fa,  8},
  {m68k_op_muls_16_pcix        , 0xf1ff, 0xc1fb, 10},
  {m68k_op_muls_16_i           , 0xf1ff, 0xc1fc,  4},
  {m68k_op_add_8_er_pi7        , 0xf1ff, 0xd01f,  8},
  {m68k_op_add_8_er_pd7        , 0xf1ff, 0xd027, 10},
  {m68k_op_add_8_er_aw         , 0xf1ff, 0xd038, 12},
  {m68k_op_add_8_er_al         , 0xf1ff, 0xd039, 16},
  {m68k_op_add_8_er_pcdi       , 0xf1ff, 0xd03a, 12},
  {m68k_op_add_8_er_pcix       , 0xf1ff, 0xd03b, 14},
  {m68k_op_add_8_er_i          , 0xf1ff, 0xd03c,  8},
  {m68k_op_add_16_er_aw        , 0xf1ff, 0xd078, 12},
  {m68k_op_add_16_er_al        , 0xf1ff, 0xd079, 16},
  {m68k_op_add_16_er_pcdi      , 0xf1ff, 0xd07a, 12},
  {m68k_op_add_16_er_pcix      , 0xf1ff, 0xd07b, 14},
  {m68k_op_add_16_er_i         , 0xf1ff, 0xd07c,  8},
  {m68k_op_add_32_er_aw        , 0xf1ff, 0xd0b8, 18},
  {m68k_op_add_32_er_al        , 0xf1ff, 0xd0b9, 22},
  {m68k_op_add_32_er_pcdi      , 0xf1ff, 0xd0ba, 18},
  {m68k_op_add_32_er_pcix      , 0xf1ff, 0xd0bb, 20},
  {m68k_op_add_32_er_i         , 0xf1ff, 0xd0bc, 16},
  {m68k_op_adda_16_aw          , 0xf1ff, 0xd0f8, 16},
  {m68k_op_adda_16_al          , 0xf1ff, 0xd0f9, 20},
  {m68k_op_adda_16_pcdi        , 0xf1ff, 0xd0fa, 16},
  {m68k_op_adda_16_pcix        , 0xf1ff, 0xd0fb, 18},
  {m68k_op_adda_16_i           , 0xf1ff, 0xd0fc, 12},
  {m68k_op_addx_8_mm_ay7       , 0xf1ff, 0xd10f, 18},
  {m68k_op_add_8_re_pi7        , 0xf1ff, 0xd11f, 12},
  {m68k_op_add_8_re_pd7        , 0xf1ff, 0xd127, 14},
  {m68k_op_add_8_re_aw         , 0xf1ff, 0xd138, 16},
  {m68k_op_add_8_re_al         , 0xf1ff, 0xd139, 20},
  {m68k_op_add_16_re_aw        , 0xf1ff, 0xd178, 16},
  {m68k_op_add_16_re_al        , 0xf1ff, 0xd179, 20},
  {m68k_op_add_32_re_aw        , 0xf1ff, 0xd1b8, 24},
  {m68k_op_add_32_re_al        , 0xf1ff, 0xd1b9, 28},
  {m68k_op_adda_32_aw          , 0xf1ff, 0xd1f8, 18},
  {m68k_op_adda_32_al          , 0xf1ff, 0xd1f9, 22},
  {m68k_op_adda_32_pcdi        , 0xf1ff, 0xd1fa, 18},
  {m68k_op_adda_32_pcix        , 0xf1ff, 0xd1fb, 20},
  {m68k_op_adda_32_i           , 0xf1ff, 0xd1fc, 16},
  {m68k_op_ori_8_d             , 0xfff8, 0x0000,  8},
  {m68k_op_ori_8_ai            , 0xfff8, 0x0010, 16},
  {m68k_op_ori_8_pi            , 0xfff8, 0x0018, 16},
  {m68k_op_ori_8_pd            , 0xfff8, 0x0020, 18},
  {m68k_op_ori_8_di            , 0xfff8, 0x0028, 20},
  {m68k_op_ori_8_ix            , 0xfff8, 0x0030, 22},
  {m68k_op_ori_16_d            , 0xfff8, 0x0040,  8},
  {m68k_op_ori_16_ai           , 0xfff8, 0x0050, 16},
  {m68k_op_ori_16_pi           , 0xfff8, 0x0058, 16},
  {m68k_op_ori_16_pd           , 0xfff8, 0x0060, 18},
  {m68k_op_ori_16_di           , 0xfff8, 0x0068, 20},
  {m68k_op_ori_16_ix           , 0xfff8, 0x0070, 22},
  {m68k_op_ori_32_d            , 0xfff8, 0x0080, 16},
  {m68k_op_ori_32_ai           , 0xfff8, 0x0090, 28},
  {m68k_op_ori_32_pi           , 0xfff8, 0x0098, 28},
  {m68k_op_ori_32_pd           , 0xfff8, 0x00a0, 30},
  {m68k_op_ori_32_di           , 0xfff8, 0x00a8, 32},
  {m68k_op_ori_32_ix           , 0xfff8, 0x00b0, 34},
  {m68k_op_andi_8_d            , 0xfff8, 0x0200,  8},
  {m68k_op_andi_8_ai           , 0xfff8, 0x0210, 16},
  {m68k_op_andi_8_pi           , 0xfff8, 0x0218, 16},
  {m68k_op_andi_8_pd           , 0xfff8, 0x0220, 18},
  {m68k_op_andi_8_di           , 0xfff8, 0x0228, 20},
  {m68k_op_andi_8_ix           , 0xfff8, 0x0230, 22},
  {m68k_op_andi_16_d           , 0xfff8, 0x0240,  8},
  {m68k_op_andi_16_ai          , 0xfff8, 0x0250, 16},
  {m68k_op_andi_16_pi          , 0xfff8, 0x0258, 16},
  {m68k_op_andi_16_pd          , 0xfff8, 0x0260, 18},
  {m68k_op_andi_16_di          , 0xfff8, 0x0268, 20},
  {m68k_op_andi_16_ix          , 0xfff8, 0x0270, 22},
  {m68k_op_andi_32_d           , 0xfff8, 0x0280, 14},
  {m68k_op_andi_32_ai          , 0xfff8, 0x0290, 28},
  {m68k_op_andi_32_pi          , 0xfff8, 0x0298, 28},
  {m68k_op_andi_32_pd          , 0xfff8, 0x02a0, 30},
  {m68k_op_andi_32_di          , 0xfff8, 0x02a8, 32},
  {m68k_op_andi_32_ix          , 0xfff8, 0x02b0, 34},
  {m68k_op_subi_8_d            , 0xfff8, 0x0400,  8},
  {m68k_op_subi_8_ai           , 0xfff8, 0x0410, 16},
  {m68k_op_subi_8_pi           , 0xfff8, 0x0418, 16},
  {m68k_op_subi_8_pd           , 0xfff8, 0x0420, 18},
  {m68k_op_subi_8_di           , 0xfff8, 0x0428, 20},
  {m68k_op_subi_8_ix           , 0xfff8, 0x0430, 22},
  {m68k_op_subi_16_d           , 0xfff8, 0x0440,  8},
  {m68k_op_subi_16_ai          , 0xfff8, 0x0450, 16},
  {m68k_op_subi_16_pi          , 0xfff8, 0x0458, 16},
  {m68k_op_subi_16_pd          , 0xfff8, 0x0460, 18},
  {m68k_op_subi_16_di          , 0xfff8, 0x0468, 20},
  {m68k_op_subi_16_ix          , 0xfff8, 0x0470, 22},
  {m68k_op_subi_32_d           , 0xfff8, 0x0480, 16},
  {m68k_op_subi_32_ai          , 0xfff8, 0x0490, 28},
  {m68k_op_subi_32_pi          , 0xfff8, 0x0498, 28},
  {m68k_op_subi_32_pd          , 0xfff8, 0x04a0, 30},
  {m68k_op_subi_32_di          , 0xfff8, 0x04a8, 32},
  {m68k_op_subi_32_ix          , 0xfff8, 0x04b0, 34},
  {m68k_op_addi_8_d            , 0xfff8, 0x0600,  8},
  {m68k_op_addi_8_ai           , 0xfff8, 0x0610, 16},
  {m68k_op_addi_8_pi           , 0xfff8, 0x0618, 16},
  {m68k_op_addi_8_pd           , 0xfff8, 0x0620, 18},
  {m68k_op_addi_8_di           , 0xfff8, 0x0628, 20},
  {m68k_op_addi_8_ix           , 0xfff8, 0x0630, 22},
  {m68k_op_addi_16_d           , 0xfff8, 0x0640,  8},
  {m68k_op_addi_16_ai          , 0xfff8, 0x0650, 16},
  {m68k_op_addi_16_pi          , 0xfff8, 0x0658, 16},
  {m68k_op_addi_16_pd          , 0xfff8, 0x0660, 18},
  {m68k_op_addi_16_di          , 0xfff8, 0x0668, 20},
  {m68k_op_addi_16_ix          , 0xfff8, 0x0670, 22},
  {m68k_op_addi_32_d           , 0xfff8, 0x0680, 16},
  {m68k_op_addi_32_ai          , 0xfff8, 0x0690, 28},
  {m68k_op_addi_32_pi          , 0xfff8, 0x0698, 28},
  {m68k_op_addi_32_pd          , 0xfff8, 0x06a0, 30},
  {m68k_op_addi_32_di          , 0xfff8, 0x06a8, 32},
  {m68k_op_addi_32_ix          , 0xfff8, 0x06b0, 34},
  {m68k_op_btst_32_s_d         , 0xfff8, 0x0800, 10},
  {m68k_op_btst_8_s_ai         , 0xfff8, 0x0810, 12},
  {m68k_op_btst_8_s_pi         , 0xfff8, 0x0818, 12},
  {m68k_op_btst_8_s_pd         , 0xfff8, 0x0820, 14},
  {m68k_op_btst_8_s_di         , 0xfff8, 0x0828, 16},
  {m68k_op_btst_8_s_ix         , 0xfff8, 0x0830, 18},
  {m68k_op_bchg_32_s_d         , 0xfff8, 0x0840, 12},
  {m68k_op_bchg_8_s_ai         , 0xfff8, 0x0850, 16},
  {m68k_op_bchg_8_s_pi         , 0xfff8, 0x0858, 16},
  {m68k_op_bchg_8_s_pd         , 0xfff8, 0x0860, 18},
  {m68k_op_bchg_8_s_di         , 0xfff8, 0x0868, 20},
  {m68k_op_bchg_8_s_ix         , 0xfff8, 0x0870, 22},
  {m68k_op_bclr_32_s_d         , 0xfff8, 0x0880, 14},
  {m68k_op_bclr_8_s_ai         , 0xfff8, 0x0890, 16},
  {m68k_op_bclr_8_s_pi         , 0xfff8, 0x0898, 16},
  {m68k_op_bclr_8_s_pd         , 0xfff8, 0x08a0, 18},
  {m68k_op_bclr_8_s_di         , 0xfff8, 0x08a8, 20},
  {m68k_op_bclr_8_s_ix         , 0xfff8, 0x08b0, 22},
  {m68k_op_bset_32_s_d         , 0xfff8, 0x08c0, 12},
  {m68k_op_bset_8_s_ai         , 0xfff8, 0x08d0, 16},
  {m68k_op_bset_8_s_pi         , 0xfff8, 0x08d8, 16},
  {m68k_op_bset_8_s_pd         , 0xfff8, 0x08e0, 18},
  {m68k_op_bset_8_s_di         , 0xfff8, 0x08e8, 20},
  {m68k_op_bset_8_s_ix         , 0xfff8, 0x08f0, 22},
  {m68k_op_eori_8_d            , 0xfff8, 0x0a00,  8},
  {m68k_op_eori_8_ai           , 0xfff8, 0x0a10, 16},
  {m68k_op_eori_8_pi           , 0xfff8, 0x0a18, 16},
  {m68k_op_eori_8_pd           , 0xfff8, 0x0a20, 18},
  {m68k_op_eori_8_di           , 0xfff8, 0x0a28, 20},
  {m68k_op_eori_8_ix           , 0xfff8, 0x0a30, 22},
  {m68k_op_eori_16_d           , 0xfff8, 0x0a40,  8},
  {m68k_op_eori_16_ai          , 0xfff8, 0x0a50, 16},
  {m68k_op_eori_16_pi          , 0xfff8, 0x0a58, 16},
  {m68k_op_eori_16_pd          , 0xfff8, 0x0a60, 18},
  {m68k_op_eori_16_di          , 0xfff8, 0x0a68, 20},
  {m68k_op_eori_16_ix          , 0xfff8, 0x0a70, 22},
  {m68k_op_eori_32_d           , 0xfff8, 0x0a80, 16},
  {m68k_op_eori_32_ai          , 0xfff8, 0x0a90, 28},
  {m68k_op_eori_32_pi          , 0xfff8, 0x0a98, 28},
  {m68k_op_eori_32_pd          , 0xfff8, 0x0aa0, 30},
  {m68k_op_eori_32_di          , 0xfff8, 0x0aa8, 32},
  {m68k_op_eori_32_ix          , 0xfff8, 0x0ab0, 34},
  {m68k_op_cmpi_8_d            , 0xfff8, 0x0c00,  8},
  {m68k_op_cmpi_8_ai           , 0xfff8, 0x0c10, 12},
  {m68k_op_cmpi_8_pi           , 0xfff8, 0x0c18, 12},
  {m68k_op_cmpi_8_pd           , 0xfff8, 0x0c20, 14},
  {m68k_op_cmpi_8_di           , 0xfff8, 0x0c28, 16},
  {m68k_op_cmpi_8_ix           , 0xfff8, 0x0c30, 18},
  {m68k_op_cmpi_16_d           , 0xfff8, 0x0c40,  8},
  {m68k_op_cmpi_16_ai          , 0xfff8, 0x0c50, 12},
  {m68k_op_cmpi_16_pi          , 0xfff8, 0x0c58, 12},
  {m68k_op_cmpi_16_pd          , 0xfff8, 0x0c60, 14},
  {m68k_op_cmpi_16_di          , 0xfff8, 0x0c68, 16},
  {m68k_op_cmpi_16_ix          , 0xfff8, 0x0c70, 18},
  {m68k_op_cmpi_32_d           , 0xfff8, 0x0c80, 14},
  {m68k_op_cmpi_32_ai          , 0xfff8, 0x0c90, 20},
  {m68k_op_cmpi_32_pi          , 0xfff8, 0x0c98, 20},
  {m68k_op_cmpi_32_pd          , 0xfff8, 0x0ca0, 22},
  {m68k_op_cmpi_32_di          , 0xfff8, 0x0ca8, 24},
  {m68k_op_cmpi_32_ix          , 0xfff8, 0x0cb0, 26},
  {m68k_op_move_8_aw_d         , 0xfff8, 0x11c0, 12},
  {m68k_op_move_8_aw_ai        , 0xfff8, 0x11d0, 16},
  {m68k_op_move_8_aw_pi        , 0xfff8, 0x11d8, 16},
  {m68k_op_move_8_aw_pd        , 0xfff8, 0x11e0, 18},
  {m68k_op_move_8_aw_di        , 0xfff8, 0x11e8, 20},
  {m68k_op_move_8_aw_ix        , 0xfff8, 0x11f0, 22},
  {m68k_op_move_8_al_d         , 0xfff8, 0x13c0, 16},
  {m68k_op_move_8_al_ai        , 0xfff8, 0x13d0, 20},
  {m68k_op_move_8_al_pi        , 0xfff8, 0x13d8, 20},
  {m68k_op_move_8_al_pd        , 0xfff8, 0x13e0, 22},
  {m68k_op_move_8_al_di        , 0xfff8, 0x13e8, 24},
  {m68k_op_move_8_al_ix        , 0xfff8, 0x13f0, 26},
  {m68k_op_move_8_pi7_d        , 0xfff8, 0x1ec0,  8},
  {m68k_op_move_8_pi7_ai       , 0xfff8, 0x1ed0, 12},
  {m68k_op_move_8_pi7_pi       , 0xfff8, 0x1ed8, 12},
  {m68k_op_move_8_pi7_pd       , 0xfff8, 0x1ee0, 14},
  {m68k_op_move_8_pi7_di       , 0xfff8, 0x1ee8, 16},
  {m68k_op_move_8_pi7_ix       , 0xfff8, 0x1ef0, 18},
  {m68k_op_move_8_pd7_d        , 0xfff8, 0x1f00,  8},
  {m68k_op_move_8_pd7_ai       , 0xfff8, 0x1f10, 12},
  {m68k_op_move_8_pd7_pi       , 0xfff8, 0x1f18, 12},
  {m68k_op_move_8_pd7_pd       , 0xfff8, 0x1f20, 14},
  {m68k_op_move_8_pd7_di       , 0xfff8, 0x1f28, 16},
  {m68k_op_move_8_pd7_ix       , 0xfff8, 0x1f30, 18},
  {m68k_op_move_32_aw_d        , 0xfff8, 0x21c0, 16},
  {m68k_op_move_32_aw_a        , 0xfff8, 0x21c8, 16},
  {m68k_op_move_32_aw_ai       , 0xfff8, 0x21d0, 24},
  {m68k_op_move_32_aw_pi       , 0xfff8, 0x21d8, 24},
  {m68k_op_move_32_aw_pd       , 0xfff8, 0x21e0, 26},
  {m68k_op_move_32_aw_di       , 0xfff8, 0x21e8, 28},
  {m68k_op_move_32_aw_ix       , 0xfff8, 0x21f0, 30},
  {m68k_op_move_32_al_d        , 0xfff8, 0x23c0, 20},
  {m68k_op_move_32_al_a        , 0xfff8, 0x23c8, 20},
  {m68k_op_move_32_al_ai       , 0xfff8, 0x23d0, 28},
  {m68k_op_move_32_al_pi       , 0xfff8, 0x23d8, 28},
  {m68k_op_move_32_al_pd       , 0xfff8, 0x23e0, 30},
  {m68k_op_move_32_al_di       , 0xfff8, 0x23e8, 32},
  {m68k_op_move_32_al_ix       , 0xfff8, 0x23f0, 34},
  {m68k_op_move_16_aw_d        , 0xfff8, 0x31c0, 12},
  {m68k_op_move_16_aw_a        , 0xfff8, 0x31c8, 12},
  {m68k_op_move_16_aw_ai       , 0xfff8, 0x31d0, 16},
  {m68k_op_move_16_aw_pi       , 0xfff8, 0x31d8, 16},
  {m68k_op_move_16_aw_pd       , 0xfff8, 0x31e0, 18},
  {m68k_op_move_16_aw_di       , 0xfff8, 0x31e8, 20},
  {m68k_op_move_16_aw_ix       , 0xfff8, 0x31f0, 22},
  {m68k_op_move_16_al_d        , 0xfff8, 0x33c0, 16},
  {m68k_op_move_16_al_a        , 0xfff8, 0x33c8, 16},
  {m68k_op_move_16_al_ai       , 0xfff8, 0x33d0, 20},
  {m68k_op_move_16_al_pi       , 0xfff8, 0x33d8, 20},
  {m68k_op_move_16_al_pd       , 0xfff8, 0x33e0, 22},
  {m68k_op_move_16_al_di       , 0xfff8, 0x33e8, 24},
  {m68k_op_move_16_al_ix       , 0xfff8, 0x33f0, 26},
  {m68k_op_negx_8_d            , 0xfff8, 0x4000,  4},
  {m68k_op_negx_8_ai           , 0xfff8, 0x4010, 12},
  {m68k_op_negx_8_pi           , 0xfff8, 0x4018, 12},
  {m68k_op_negx_8_pd           , 0xfff8, 0x4020, 14},
  {m68k_op_negx_8_di           , 0xfff8, 0x4028, 16},
  {m68k_op_negx_8_ix           , 0xfff8, 0x4030, 18},
  {m68k_op_negx_16_d           , 0xfff8, 0x4040,  4},
  {m68k_op_negx_16_ai          , 0xfff8, 0x4050, 12},
  {m68k_op_negx_16_pi          , 0xfff8, 0x4058, 12},
  {m68k_op_negx_16_pd          , 0xfff8, 0x4060, 14},
  {m68k_op_negx_16_di          , 0xfff8, 0x4068, 16},
  {m68k_op_negx_16_ix          , 0xfff8, 0x4070, 18},
  {m68k_op_negx_32_d           , 0xfff8, 0x4080,  6},
  {m68k_op_negx_32_ai          , 0xfff8, 0x4090, 20},
  {m68k_op_negx_32_pi          , 0xfff8, 0x4098, 20},
  {m68k_op_negx_32_pd          , 0xfff8, 0x40a0, 22},
  {m68k_op_negx_32_di          , 0xfff8, 0x40a8, 24},
  {m68k_op_negx_32_ix          , 0xfff8, 0x40b0, 26},
  {m68k_op_move_16_frs_d       , 0xfff8, 0x40c0,  6},
  {m68k_op_move_16_frs_ai      , 0xfff8, 0x40d0, 12},
  {m68k_op_move_16_frs_pi      , 0xfff8, 0x40d8, 12},
  {m68k_op_move_16_frs_pd      , 0xfff8, 0x40e0, 14},
  {m68k_op_move_16_frs_di      , 0xfff8, 0x40e8, 16},
  {m68k_op_move_16_frs_ix      , 0xfff8, 0x40f0, 18},
  {m68k_op_clr_8_d             , 0xfff8, 0x4200,  4},
  {m68k_op_clr_8_ai            , 0xfff8, 0x4210, 12},
  {m68k_op_clr_8_pi            , 0xfff8, 0x4218, 12},
  {m68k_op_clr_8_pd            , 0xfff8, 0x4220, 14},
  {m68k_op_clr_8_di            , 0xfff8, 0x4228, 16},
  {m68k_op_clr_8_ix            , 0xfff8, 0x4230, 18},
  {m68k_op_clr_16_d            , 0xfff8, 0x4240,  4},
  {m68k_op_clr_16_ai           , 0xfff8, 0x4250, 12},
  {m68k_op_clr_16_pi           , 0xfff8, 0x4258, 12},
  {m68k_op_clr_16_pd           , 0xfff8, 0x4260, 14},
  {m68k_op_clr_16_di           , 0xfff8, 0x4268, 16},
  {m68k_op_clr_16_ix           , 0xfff8, 0x4270, 18},
  {m68k_op_clr_32_d            , 0xfff8, 0x4280,  6},
  {m68k_op_clr_32_ai           , 0xfff8, 0x4290, 20},
  {m68k_op_clr_32_pi           , 0xfff8, 0x4298, 20},
  {m68k_op_clr_32_pd           , 0xfff8, 0x42a0, 22},
  {m68k_op_clr_32_di           , 0xfff8, 0x42a8, 24},
  {m68k_op_clr_32_ix           , 0xfff8, 0x42b0, 26},
  {m68k_op_neg_8_d             , 0xfff8, 0x4400,  4},
  {m68k_op_neg_8_ai            , 0xfff8, 0x4410, 12},
  {m68k_op_neg_8_pi            , 0xfff8, 0x4418, 12},
  {m68k_op_neg_8_pd            , 0xfff8, 0x4420, 14},
  {m68k_op_neg_8_di            , 0xfff8, 0x4428, 16},
  {m68k_op_neg_8_ix            , 0xfff8, 0x4430, 18},
  {m68k_op_neg_16_d            , 0xfff8, 0x4440,  4},
  {m68k_op_neg_16_ai           , 0xfff8, 0x4450, 12},
  {m68k_op_neg_16_pi           , 0xfff8, 0x4458, 12},
  {m68k_op_neg_16_pd           , 0xfff8, 0x4460, 14},
  {m68k_op_neg_16_di           , 0xfff8, 0x4468, 16},
  {m68k_op_neg_16_ix           , 0xfff8, 0x4470, 18},
  {m68k_op_neg_32_d            , 0xfff8, 0x4480,  6},
  {m68k_op_neg_32_ai           , 0xfff8, 0x4490, 20},
  {m68k_op_neg_32_pi           , 0xfff8, 0x4498, 20},
  {m68k_op_neg_32_pd           , 0xfff8, 0x44a0, 22},
  {m68k_op_neg_32_di           , 0xfff8, 0x44a8, 24},
  {m68k_op_neg_32_ix           , 0xfff8, 0x44b0, 26},
  {m68k_op_move_16_toc_d       , 0xfff8, 0x44c0, 12},
  {m68k_op_move_16_toc_ai      , 0xfff8, 0x44d0, 16},
  {m68k_op_move_16_toc_pi      , 0xfff8, 0x44d8, 16},
  {m68k_op_move_16_toc_pd      , 0xfff8, 0x44e0, 18},
  {m68k_op_move_16_toc_di      , 0xfff8, 0x44e8, 20},
  {m68k_op_move_16_toc_ix      , 0xfff8, 0x44f0, 22},
  {m68k_op_not_8_d             , 0xfff8, 0x4600,  4},
  {m68k_op_not_8_ai            , 0xfff8, 0x4610, 12},
  {m68k_op_not_8_pi            , 0xfff8, 0x4618, 12},
  {m68k_op_not_8_pd            , 0xfff8, 0x4620, 14},
  {m68k_op_not_8_di            , 0xfff8, 0x4628, 16},
  {m68k_op_not_8_ix            , 0xfff8, 0x4630, 18},
  {m68k_op_not_16_d            , 0xfff8, 0x4640,  4},
  {m68k_op_not_16_ai           , 0xfff8, 0x4650, 12},
  {m68k_op_not_16_pi           , 0xfff8, 0x4658, 12},
  {m68k_op_not_16_pd           , 0xfff8, 0x4660, 14},
  {m68k_op_not_16_di           , 0xfff8, 0x4668, 16},
  {m68k_op_not_16_ix           , 0xfff8, 0x4670, 18},
  {m68k_op_not_32_d            , 0xfff8, 0x4680,  6},
  {m68k_op_not_32_ai           , 0xfff8, 0x4690, 20},
  {m68k_op_not_32_pi           , 0xfff8, 0x4698, 20},
  {m68k_op_not_32_pd           , 0xfff8, 0x46a0, 22},
  {m68k_op_not_32_di           , 0xfff8, 0x46a8, 24},
  {m68k_op_not_32_ix           , 0xfff8, 0x46b0, 26},
  {m68k_op_move_16_tos_d       , 0xfff8, 0x46c0, 12},
  {m68k_op_move_16_tos_ai      , 0xfff8, 0x46d0, 16},
  {m68k_op_move_16_tos_pi      , 0xfff8, 0x46d8, 16},
  {m68k_op_move_16_tos_pd      , 0xfff8, 0x46e0, 18},
  {m68k_op_move_16_tos_di      , 0xfff8, 0x46e8, 20},
  {m68k_op_move_16_tos_ix      , 0xfff8, 0x46f0, 22},
  {m68k_op_nbcd_8_d            , 0xfff8, 0x4800,  6},
  {m68k_op_nbcd_8_ai           , 0xfff8, 0x4810, 12},
  {m68k_op_nbcd_8_pi           , 0xfff8, 0x4818, 12},
  {m68k_op_nbcd_8_pd           , 0xfff8, 0x4820, 14},
  {m68k_op_nbcd_8_di           , 0xfff8, 0x4828, 16},
  {m68k_op_nbcd_8_ix           , 0xfff8, 0x4830, 18},
  {m68k_op_swap_32             , 0xfff8, 0x4840,  4},
  {m68k_op_pea_32_ai           , 0xfff8, 0x4850, 12},
  {m68k_op_pea_32_di           , 0xfff8, 0x4868, 16},
  {m68k_op_pea_32_ix           , 0xfff8, 0x4870, 20},
  {m68k_op_ext_16              , 0xfff8, 0x4880,  4},
  {m68k_op_movem_16_re_ai      , 0xfff8, 0x4890,  8},
  {m68k_op_movem_16_re_pd      , 0xfff8, 0x48a0,  8},
  {m68k_op_movem_16_re_di      , 0xfff8, 0x48a8, 12},
  {m68k_op_movem_16_re_ix      , 0xfff8, 0x48b0, 14},
  {m68k_op_ext_32              , 0xfff8, 0x48c0,  4},
  {m68k_op_movem_32_re_ai      , 0xfff8, 0x48d0,  8},
  {m68k_op_movem_32_re_pd      , 0xfff8, 0x48e0,  8},
  {m68k_op_movem_32_re_di      , 0xfff8, 0x48e8, 12},
  {m68k_op_movem_32_re_ix      , 0xfff8, 0x48f0, 14},
  {m68k_op_tst_8_d             , 0xfff8, 0x4a00,  4},
  {m68k_op_tst_8_ai            , 0xfff8, 0x4a10,  8},
  {m68k_op_tst_8_pi            , 0xfff8, 0x4a18,  8},
  {m68k_op_tst_8_pd            , 0xfff8, 0x4a20, 10},
  {m68k_op_tst_8_di            , 0xfff8, 0x4a28, 12},
  {m68k_op_tst_8_ix            , 0xfff8, 0x4a30, 14},
  {m68k_op_tst_16_d            , 0xfff8, 0x4a40,  4},
  {m68k_op_tst_16_ai           , 0xfff8, 0x4a50,  8},
  {m68k_op_tst_16_pi           , 0xfff8, 0x4a58,  8},
  {m68k_op_tst_16_pd           , 0xfff8, 0x4a60, 10},
  {m68k_op_tst_16_di           , 0xfff8, 0x4a68, 12},
  {m68k_op_tst_16_ix           , 0xfff8, 0x4a70, 14},
  {m68k_op_tst_32_d            , 0xfff8, 0x4a80,  4},
  {m68k_op_tst_32_ai           , 0xfff8, 0x4a90, 12},
  {m68k_op_tst_32_pi           , 0xfff8, 0x4a98, 12},
  {m68k_op_tst_32_pd           , 0xfff8, 0x4aa0, 14},
  {m68k_op_tst_32_di           , 0xfff8, 0x4aa8, 16},
  {m68k_op_tst_32_ix           , 0xfff8, 0x4ab0, 18},
  {m68k_op_tas_8_d             , 0xfff8, 0x4ac0,  4},
  {m68k_op_tas_8_ai            , 0xfff8, 0x4ad0, 18},
  {m68k_op_tas_8_pi            , 0xfff8, 0x4ad8, 18},
  {m68k_op_tas_8_pd            , 0xfff8, 0x4ae0, 20},
  {m68k_op_tas_8_di            , 0xfff8, 0x4ae8, 22},
  {m68k_op_tas_8_ix            , 0xfff8, 0x4af0, 24},
  {m68k_op_movem_16_er_ai      , 0xfff8, 0x4c90, 12},
  {m68k_op_movem_16_er_pi      , 0xfff8, 0x4c98, 12},
  {m68k_op_movem_16_er_di      , 0xfff8, 0x4ca8, 16},
  {m68k_op_movem_16_er_ix      , 0xfff8, 0x4cb0, 18},
  {m68k_op_movem_32_er_ai      , 0xfff8, 0x4cd0, 12},
  {m68k_op_movem_32_er_pi      , 0xfff8, 0x4cd8, 12},
  {m68k_op_movem_32_er_di      , 0xfff8, 0x4ce8, 16},
  {m68k_op_movem_32_er_ix      , 0xfff8, 0x4cf0, 18},
  {m68k_op_link_16             , 0xfff8, 0x4e50, 16},
  {m68k_op_unlk_32             , 0xfff8, 0x4e58, 12},
  {m68k_op_move_32_tou         , 0xfff8, 0x4e60,  4},
  {m68k_op_move_32_fru         , 0xfff8, 0x4e68,  4},
  {m68k_op_jsr_32_ai           , 0xfff8, 0x4e90, 16},
  {m68k_op_jsr_32_di           , 0xfff8, 0x4ea8, 18},
  {m68k_op_jsr_32_ix           , 0xfff8, 0x4eb0, 22},
  {m68k_op_jmp_32_ai           , 0xfff8, 0x4ed0,  8},
  {m68k_op_jmp_32_di           , 0xfff8, 0x4ee8, 10},
  {m68k_op_jmp_32_ix           , 0xfff8, 0x4ef0, 14},
  {m68k_op_st_8_d              , 0xfff8, 0x50c0,  6},
  {m68k_op_dbt_16              , 0xfff8, 0x50c8, 12},
  {m68k_op_st_8_ai             , 0xfff8, 0x50d0, 12},
  {m68k_op_st_8_pi             , 0xfff8, 0x50d8, 12},
  {m68k_op_st_8_pd             , 0xfff8, 0x50e0, 14},
  {m68k_op_st_8_di             , 0xfff8, 0x50e8, 16},
  {m68k_op_st_8_ix             , 0xfff8, 0x50f0, 18},
  {m68k_op_sf_8_d              , 0xfff8, 0x51c0,  4},
  {m68k_op_dbf_16              , 0xfff8, 0x51c8, 12},
  {m68k_op_sf_8_ai             , 0xfff8, 0x51d0, 12},
  {m68k_op_sf_8_pi             , 0xfff8, 0x51d8, 12},
  {m68k_op_sf_8_pd             , 0xfff8, 0x51e0, 14},
  {m68k_op_sf_8_di             , 0xfff8, 0x51e8, 16},
  {m68k_op_sf_8_ix             , 0xfff8, 0x51f0, 18},
  {m68k_op_shi_8_d             , 0xfff8, 0x52c0,  4},
  {m68k_op_dbhi_16             , 0xfff8, 0x52c8, 12},
  {m68k_op_shi_8_ai            , 0xfff8, 0x52d0, 12},
  {m68k_op_shi_8_pi            , 0xfff8, 0x52d8, 12},
  {m68k_op_shi_8_pd            , 0xfff8, 0x52e0, 14},
  {m68k_op_shi_8_di            , 0xfff8, 0x52e8, 16},
  {m68k_op_shi_8_ix            , 0xfff8, 0x52f0, 18},
  {m68k_op_sls_8_d             , 0xfff8, 0x53c0,  4},
  {m68k_op_dbls_16             , 0xfff8, 0x53c8, 12},
  {m68k_op_sls_8_ai            , 0xfff8, 0x53d0, 12},
  {m68k_op_sls_8_pi            , 0xfff8, 0x53d8, 12},
  {m68k_op_sls_8_pd            , 0xfff8, 0x53e0, 14},
  {m68k_op_sls_8_di            , 0xfff8, 0x53e8, 16},
  {m68k_op_sls_8_ix            , 0xfff8, 0x53f0, 18},
  {m68k_op_scc_8_d             , 0xfff8, 0x54c0,  4},
  {m68k_op_dbcc_16             , 0xfff8, 0x54c8, 12},
  {m68k_op_scc_8_ai            , 0xfff8, 0x54d0, 12},
  {m68k_op_scc_8_pi            , 0xfff8, 0x54d8, 12},
  {m68k_op_scc_8_pd            , 0xfff8, 0x54e0, 14},
  {m68k_op_scc_8_di            , 0xfff8, 0x54e8, 16},
  {m68k_op_scc_8_ix            , 0xfff8, 0x54f0, 18},
  {m68k_op_scs_8_d             , 0xfff8, 0x55c0,  4},
  {m68k_op_dbcs_16             , 0xfff8, 0x55c8, 12},
  {m68k_op_scs_8_ai            , 0xfff8, 0x55d0, 12},
  {m68k_op_scs_8_pi            , 0xfff8, 0x55d8, 12},
  {m68k_op_scs_8_pd            , 0xfff8, 0x55e0, 14},
  {m68k_op_scs_8_di            , 0xfff8, 0x55e8, 16},
  {m68k_op_scs_8_ix            , 0xfff8, 0x55f0, 18},
  {m68k_op_sne_8_d             , 0xfff8, 0x56c0,  4},
  {m68k_op_dbne_16             , 0xfff8, 0x56c8, 12},
  {m68k_op_sne_8_ai            , 0xfff8, 0x56d0, 12},
  {m68k_op_sne_8_pi            , 0xfff8, 0x56d8, 12},
  {m68k_op_sne_8_pd            , 0xfff8, 0x56e0, 14},
  {m68k_op_sne_8_di            , 0xfff8, 0x56e8, 16},
  {m68k_op_sne_8_ix            , 0xfff8, 0x56f0, 18},
  {m68k_op_seq_8_d             , 0xfff8, 0x57c0,  4},
  {m68k_op_dbeq_16             , 0xfff8, 0x57c8, 12},
  {m68k_op_seq_8_ai            , 0xfff8, 0x57d0, 12},
  {m68k_op_seq_8_pi            , 0xfff8, 0x57d8, 12},
  {m68k_op_seq_8_pd            , 0xfff8, 0x57e0, 14},
  {m68k_op_seq_8_di            , 0xfff8, 0x57e8, 16},
  {m68k_op_seq_8_ix            , 0xfff8, 0x57f0, 18},
  {m68k_op_svc_8_d             , 0xfff8, 0x58c0,  4},
  {m68k_op_dbvc_16             , 0xfff8, 0x58c8, 12},
  {m68k_op_svc_8_ai            , 0xfff8, 0x58d0, 12},
  {m68k_op_svc_8_pi            , 0xfff8, 0x58d8, 12},
  {m68k_op_svc_8_pd            , 0xfff8, 0x58e0, 14},
  {m68k_op_svc_8_di            , 0xfff8, 0x58e8, 16},
  {m68k_op_svc_8_ix            , 0xfff8, 0x58f0, 18},
  {m68k_op_svs_8_d             , 0xfff8, 0x59c0,  4},
  {m68k_op_dbvs_16             , 0xfff8, 0x59c8, 12},
  {m68k_op_svs_8_ai            , 0xfff8, 0x59d0, 12},
  {m68k_op_svs_8_pi            , 0xfff8, 0x59d8, 12},
  {m68k_op_svs_8_pd            , 0xfff8, 0x59e0, 14},
  {m68k_op_svs_8_di            , 0xfff8, 0x59e8, 16},
  {m68k_op_svs_8_ix            , 0xfff8, 0x59f0, 18},
  {m68k_op_spl_8_d             , 0xfff8, 0x5ac0,  4},
  {m68k_op_dbpl_16             , 0xfff8, 0x5ac8, 12},
  {m68k_op_spl_8_ai            , 0xfff8, 0x5ad0, 12},
  {m68k_op_spl_8_pi            , 0xfff8, 0x5ad8, 12},
  {m68k_op_spl_8_pd            , 0xfff8, 0x5ae0, 14},
  {m68k_op_spl_8_di            , 0xfff8, 0x5ae8, 16},
  {m68k_op_spl_8_ix            , 0xfff8, 0x5af0, 18},
  {m68k_op_smi_8_d             , 0xfff8, 0x5bc0,  4},
  {m68k_op_dbmi_16             , 0xfff8, 0x5bc8, 12},
  {m68k_op_smi_8_ai            , 0xfff8, 0x5bd0, 12},
  {m68k_op_smi_8_pi            , 0xfff8, 0x5bd8, 12},
  {m68k_op_smi_8_pd            , 0xfff8, 0x5be0, 14},
  {m68k_op_smi_8_di            , 0xfff8, 0x5be8, 16},
  {m68k_op_smi_8_ix            , 0xfff8, 0x5bf0, 18},
  {m68k_op_sge_8_d             , 0xfff8, 0x5cc0,  4},
  {m68k_op_dbge_16             , 0xfff8, 0x5cc8, 12},
  {m68k_op_sge_8_ai            , 0xfff8, 0x5cd0, 12},
  {m68k_op_sge_8_pi            , 0xfff8, 0x5cd8, 12},
  {m68k_op_sge_8_pd            , 0xfff8, 0x5ce0, 14},
  {m68k_op_sge_8_di            , 0xfff8, 0x5ce8, 16},
  {m68k_op_sge_8_ix            , 0xfff8, 0x5cf0, 18},
  {m68k_op_slt_8_d             , 0xfff8, 0x5dc0,  4},
  {m68k_op_dblt_16             , 0xfff8, 0x5dc8, 12},
  {m68k_op_slt_8_ai            , 0xfff8, 0x5dd0, 12},
  {m68k_op_slt_8_pi            , 0xfff8, 0x5dd8, 12},
  {m68k_op_slt_8_pd            , 0xfff8, 0x5de0, 14},
  {m68k_op_slt_8_di            , 0xfff8, 0x5de8, 16},
  {m68k_op_slt_8_ix            , 0xfff8, 0x5df0, 18},
  {m68k_op_sgt_8_d             , 0xfff8, 0x5ec0,  4},
  {m68k_op_dbgt_16             , 0xfff8, 0x5ec8, 12},
  {m68k_op_sgt_8_ai            , 0xfff8, 0x5ed0, 12},
  {m68k_op_sgt_8_pi            , 0xfff8, 0x5ed8, 12},
  {m68k_op_sgt_8_pd            , 0xfff8, 0x5ee0, 14},
  {m68k_op_sgt_8_di            , 0xfff8, 0x5ee8, 16},
  {m68k_op_sgt_8_ix            , 0xfff8, 0x5ef0, 18},
  {m68k_op_sle_8_d             , 0xfff8, 0x5fc0,  4},
  {m68k_op_dble_16             , 0xfff8, 0x5fc8, 12},
  {m68k_op_sle_8_ai            , 0xfff8, 0x5fd0, 12},
  {m68k_op_sle_8_pi            , 0xfff8, 0x5fd8, 12},
  {m68k_op_sle_8_pd            , 0xfff8, 0x5fe0, 14},
  {m68k_op_sle_8_di            , 0xfff8, 0x5fe8, 16},
  {m68k_op_sle_8_ix            , 0xfff8, 0x5ff0, 18},
  {m68k_op_sbcd_8_mm_ax7       , 0xfff8, 0x8f08, 18},
  {m68k_op_subx_8_mm_ax7       , 0xfff8, 0x9f08, 18},
  {m68k_op_cmpm_8_ax7          , 0xfff8, 0xbf08, 12},
  {m68k_op_abcd_8_mm_ax7       , 0xfff8, 0xcf08, 18},
  {m68k_op_addx_8_mm_ax7       , 0xfff8, 0xdf08, 18},
  {m68k_op_asr_16_ai           , 0xfff8, 0xe0d0, 12},
  {m68k_op_asr_16_pi           , 0xfff8, 0xe0d8, 12},
  {m68k_op_asr_16_pd           , 0xfff8, 0xe0e0, 14},
  {m68k_op_asr_16_di           , 0xfff8, 0xe0e8, 16},
  {m68k_op_asr_16_ix           , 0xfff8, 0xe0f0, 18},
  {m68k_op_asl_16_ai           , 0xfff8, 0xe1d0, 12},
  {m68k_op_asl_16_pi           , 0xfff8, 0xe1d8, 12},
  {m68k_op_asl_16_pd           , 0xfff8, 0xe1e0, 14},
  {m68k_op_asl_16_di           , 0xfff8, 0xe1e8, 16},
  {m68k_op_asl_16_ix           , 0xfff8, 0xe1f0, 18},
  {m68k_op_lsr_16_ai           , 0xfff8, 0xe2d0, 12},
  {m68k_op_lsr_16_pi           , 0xfff8, 0xe2d8, 12},
  {m68k_op_lsr_16_pd           , 0xfff8, 0xe2e0, 14},
  {m68k_op_lsr_16_di           , 0xfff8, 0xe2e8, 16},
  {m68k_op_lsr_16_ix           , 0xfff8, 0xe2f0, 18},
  {m68k_op_lsl_16_ai           , 0xfff8, 0xe3d0, 12},
  {m68k_op_lsl_16_pi           , 0xfff8, 0xe3d8, 12},
  {m68k_op_lsl_16_pd           , 0xfff8, 0xe3e0, 14},
  {m68k_op_lsl_16_di           , 0xfff8, 0xe3e8, 16},
  {m68k_op_lsl_16_ix           , 0xfff8, 0xe3f0, 18},
  {m68k_op_roxr_16_ai          , 0xfff8, 0xe4d0, 12},
  {m68k_op_roxr_16_pi          , 0xfff8, 0xe4d8, 12},
  {m68k_op_roxr_16_pd          , 0xfff8, 0xe4e0, 14},
  {m68k_op_roxr_16_di          , 0xfff8, 0xe4e8, 16},
  {m68k_op_roxr_16_ix          , 0xfff8, 0xe4f0, 18},
  {m68k_op_roxl_16_ai          , 0xfff8, 0xe5d0, 12},
  {m68k_op_roxl_16_pi          , 0xfff8, 0xe5d8, 12},
  {m68k_op_roxl_16_pd          , 0xfff8, 0xe5e0, 14},
  {m68k_op_roxl_16_di          , 0xfff8, 0xe5e8, 16},
  {m68k_op_roxl_16_ix          , 0xfff8, 0xe5f0, 18},
  {m68k_op_ror_16_ai           , 0xfff8, 0xe6d0, 12},
  {m68k_op_ror_16_pi           , 0xfff8, 0xe6d8, 12},
  {m68k_op_ror_16_pd           , 0xfff8, 0xe6e0, 14},
  {m68k_op_ror_16_di           , 0xfff8, 0xe6e8, 16},
  {m68k_op_ror_16_ix           , 0xfff8, 0xe6f0, 18},
  {m68k_op_rol_16_ai           , 0xfff8, 0xe7d0, 12},
  {m68k_op_rol_16_pi           , 0xfff8, 0xe7d8, 12},
  {m68k_op_rol_16_pd           , 0xfff8, 0xe7e0, 14},
  {m68k_op_rol_16_di           , 0xfff8, 0xe7e8, 16},
  {m68k_op_rol_16_ix           , 0xfff8, 0xe7f0, 18},
  {m68k_op_ori_8_pi7           , 0xffff, 0x001f, 16},
  {m68k_op_ori_8_pd7           , 0xffff, 0x0027, 18},
  {m68k_op_ori_8_aw            , 0xffff, 0x0038, 20},
  {m68k_op_ori_8_al            , 0xffff, 0x0039, 24},
  {m68k_op_ori_16_toc          , 0xffff, 0x003c, 20},
  {m68k_op_ori_16_aw           , 0xffff, 0x0078, 20},
  {m68k_op_ori_16_al           , 0xffff, 0x0079, 24},
  {m68k_op_ori_16_tos          , 0xffff, 0x007c, 20},
  {m68k_op_ori_32_aw           , 0xffff, 0x00b8, 32},
  {m68k_op_ori_32_al           , 0xffff, 0x00b9, 36},
  {m68k_op_andi_8_pi7          , 0xffff, 0x021f, 16},
  {m68k_op_andi_8_pd7          , 0xffff, 0x0227, 18},
  {m68k_op_andi_8_aw           , 0xffff, 0x0238, 20},
  {m68k_op_andi_8_al           , 0xffff, 0x0239, 24},
  {m68k_op_andi_16_toc         , 0xffff, 0x023c, 20},
  {m68k_op_andi_16_aw          , 0xffff, 0x0278, 20},
  {m68k_op_andi_16_al          , 0xffff, 0x0279, 24},
  {m68k_op_andi_16_tos         , 0xffff, 0x027c, 20},
  {m68k_op_andi_32_aw          , 0xffff, 0x02b8, 32},
  {m68k_op_andi_32_al          , 0xffff, 0x02b9, 36},
  {m68k_op_subi_8_pi7          , 0xffff, 0x041f, 16},
  {m68k_op_subi_8_pd7          , 0xffff, 0x0427, 18},
  {m68k_op_subi_8_aw           , 0xffff, 0x0438, 20},
  {m68k_op_subi_8_al           , 0xffff, 0x0439, 24},
  {m68k_op_subi_16_aw          , 0xffff, 0x0478, 20},
  {m68k_op_subi_16_al          , 0xffff, 0x0479, 24},
  {m68k_op_subi_32_aw          , 0xffff, 0x04b8, 32},
  {m68k_op_subi_32_al          , 0xffff, 0x04b9, 36},
  {m68k_op_addi_8_pi7          , 0xffff, 0x061f, 16},
  {m68k_op_addi_8_pd7          , 0xffff, 0x0627, 18},
  {m68k_op_addi_8_aw           , 0xffff, 0x0638, 20},
  {m68k_op_addi_8_al           , 0xffff, 0x0639, 24},
  {m68k_op_addi_16_aw          , 0xffff, 0x0678, 20},
  {m68k_op_addi_16_al          , 0xffff, 0x0679, 24},
  {m68k_op_addi_32_aw          , 0xffff, 0x06b8, 32},
  {m68k_op_addi_32_al          , 0xffff, 0x06b9, 36},
  {m68k_op_btst_8_s_pi7        , 0xffff, 0x081f, 12},
  {m68k_op_btst_8_s_pd7        , 0xffff, 0x0827, 14},
  {m68k_op_btst_8_s_aw         , 0xffff, 0x0838, 16},
  {m68k_op_btst_8_s_al         , 0xffff, 0x0839, 20},
  {m68k_op_btst_8_s_pcdi       , 0xffff, 0x083a, 16},
  {m68k_op_btst_8_s_pcix       , 0xffff, 0x083b, 18},
  {m68k_op_bchg_8_s_pi7        , 0xffff, 0x085f, 16},
  {m68k_op_bchg_8_s_pd7        , 0xffff, 0x0867, 18},
  {m68k_op_bchg_8_s_aw         , 0xffff, 0x0878, 20},
  {m68k_op_bchg_8_s_al         , 0xffff, 0x0879, 24},
  {m68k_op_bclr_8_s_pi7        , 0xffff, 0x089f, 16},
  {m68k_op_bclr_8_s_pd7        , 0xffff, 0x08a7, 18},
  {m68k_op_bclr_8_s_aw         , 0xffff, 0x08b8, 20},
  {m68k_op_bclr_8_s_al         , 0xffff, 0x08b9, 24},
  {m68k_op_bset_8_s_pi7        , 0xffff, 0x08df, 16},
  {m68k_op_bset_8_s_pd7        , 0xffff, 0x08e7, 18},
  {m68k_op_bset_8_s_aw         , 0xffff, 0x08f8, 20},
  {m68k_op_bset_8_s_al         , 0xffff, 0x08f9, 24},
  {m68k_op_eori_8_pi7          , 0xffff, 0x0a1f, 16},
  {m68k_op_eori_8_pd7          , 0xffff, 0x0a27, 18},
  {m68k_op_eori_8_aw           , 0xffff, 0x0a38, 20},
  {m68k_op_eori_8_al           , 0xffff, 0x0a39, 24},
  {m68k_op_eori_16_toc         , 0xffff, 0x0a3c, 20},
  {m68k_op_eori_16_aw          , 0xffff, 0x0a78, 20},
  {m68k_op_eori_16_al          , 0xffff, 0x0a79, 24},
  {m68k_op_eori_16_tos         , 0xffff, 0x0a7c, 20},
  {m68k_op_eori_32_aw          , 0xffff, 0x0ab8, 32},
  {m68k_op_eori_32_al          , 0xffff, 0x0ab9, 36},
  {m68k_op_cmpi_8_pi7          , 0xffff, 0x0c1f, 12},
  {m68k_op_cmpi_8_pd7          , 0xffff, 0x0c27, 14},
  {m68k_op_cmpi_8_aw           , 0xffff, 0x0c38, 16},
  {m68k_op_cmpi_8_al           , 0xffff, 0x0c39, 20},
  {m68k_op_cmpi_16_aw          , 0xffff, 0x0c78, 16},
  {m68k_op_cmpi_16_al          , 0xffff, 0x0c79, 20},
  {m68k_op_cmpi_32_aw          , 0xffff, 0x0cb8, 24},
  {m68k_op_cmpi_32_al          , 0xffff, 0x0cb9, 28},
  {m68k_op_move_8_aw_pi7       , 0xffff, 0x11df, 16},
  {m68k_op_move_8_aw_pd7       , 0xffff, 0x11e7, 18},
  {m68k_op_move_8_aw_aw        , 0xffff, 0x11f8, 20},
  {m68k_op_move_8_aw_al        , 0xffff, 0x11f9, 24},
  {m68k_op_move_8_aw_pcdi      , 0xffff, 0x11fa, 20},
  {m68k_op_move_8_aw_pcix      , 0xffff, 0x11fb, 22},
  {m68k_op_move_8_aw_i         , 0xffff, 0x11fc, 16},
  {m68k_op_move_8_al_pi7       , 0xffff, 0x13df, 20},
  {m68k_op_move_8_al_pd7       , 0xffff, 0x13e7, 22},
  {m68k_op_move_8_al_aw        , 0xffff, 0x13f8, 24},
  {m68k_op_move_8_al_al        , 0xffff, 0x13f9, 28},
  {m68k_op_move_8_al_pcdi      , 0xffff, 0x13fa, 24},
  {m68k_op_move_8_al_pcix      , 0xffff, 0x13fb, 26},
  {m68k_op_move_8_al_i         , 0xffff, 0x13fc, 20},
  {m68k_op_move_8_pi7_pi7      , 0xffff, 0x1edf, 12},
  {m68k_op_move_8_pi7_pd7      , 0xffff, 0x1ee7, 14},
  {m68k_op_move_8_pi7_aw       , 0xffff, 0x1ef8, 16},
  {m68k_op_move_8_pi7_al       , 0xffff, 0x1ef9, 20},
  {m68k_op_move_8_pi7_pcdi     , 0xffff, 0x1efa, 16},
  {m68k_op_move_8_pi7_pcix     , 0xffff, 0x1efb, 18},
  {m68k_op_move_8_pi7_i        , 0xffff, 0x1efc, 12},
  {m68k_op_move_8_pd7_pi7      , 0xffff, 0x1f1f, 12},
  {m68k_op_move_8_pd7_pd7      , 0xffff, 0x1f27, 14},
  {m68k_op_move_8_pd7_aw       , 0xffff, 0x1f38, 16},
  {m68k_op_move_8_pd7_al       , 0xffff, 0x1f39, 20},
  {m68k_op_move_8_pd7_pcdi     , 0xffff, 0x1f3a, 16},
  {m68k_op_move_8_pd7_pcix     , 0xffff, 0x1f3b, 18},
  {m68k_op_move_8_pd7_i        , 0xffff, 0x1f3c, 12},
  {m68k_op_move_32_aw_aw       , 0xffff, 0x21f8, 28},
  {m68k_op_move_32_aw_al       , 0xffff, 0x21f9, 32},
  {m68k_op_move_32_aw_pcdi     , 0xffff, 0x21fa, 28},
  {m68k_op_move_32_aw_pcix     , 0xffff, 0x21fb, 30},
  {m68k_op_move_32_aw_i        , 0xffff, 0x21fc, 24},
  {m68k_op_move_32_al_aw       , 0xffff, 0x23f8, 32},
  {m68k_op_move_32_al_al       , 0xffff, 0x23f9, 36},
  {m68k_op_move_32_al_pcdi     , 0xffff, 0x23fa, 32},
  {m68k_op_move_32_al_pcix     , 0xffff, 0x23fb, 34},
  {m68k_op_move_32_al_i        , 0xffff, 0x23fc, 28},
  {m68k_op_move_16_aw_aw       , 0xffff, 0x31f8, 20},
  {m68k_op_move_16_aw_al       , 0xffff, 0x31f9, 24},
  {m68k_op_move_16_aw_pcdi     , 0xffff, 0x31fa, 20},
  {m68k_op_move_16_aw_pcix     , 0xffff, 0x31fb, 22},
  {m68k_op_move_16_aw_i        , 0xffff, 0x31fc, 16},
  {m68k_op_move_16_al_aw       , 0xffff, 0x33f8, 24},
  {m68k_op_move_16_al_al       , 0xffff, 0x33f9, 28},
  {m68k_op_move_16_al_pcdi     , 0xffff, 0x33fa, 24},
  {m68k_op_move_16_al_pcix     , 0xffff, 0x33fb, 26},
  {m68k_op_move_16_al_i        , 0xffff, 0x33fc, 20},
  {m68k_op_negx_8_pi7          , 0xffff, 0x401f, 12},
  {m68k_op_negx_8_pd7          , 0xffff, 0x4027, 14},
  {m68k_op_negx_8_aw           , 0xffff, 0x4038, 16},
  {m68k_op_negx_8_al           , 0xffff, 0x4039, 20},
  {m68k_op_negx_16_aw          , 0xffff, 0x4078, 16},
  {m68k_op_negx_16_al          , 0xffff, 0x4079, 20},
  {m68k_op_negx_32_aw          , 0xffff, 0x40b8, 24},
  {m68k_op_negx_32_al          , 0xffff, 0x40b9, 28},
  {m68k_op_move_16_frs_aw      , 0xffff, 0x40f8, 16},
  {m68k_op_move_16_frs_al      , 0xffff, 0x40f9, 20},
  {m68k_op_clr_8_pi7           , 0xffff, 0x421f, 12},
  {m68k_op_clr_8_pd7           , 0xffff, 0x4227, 14},
  {m68k_op_clr_8_aw            , 0xffff, 0x4238, 16},
  {m68k_op_clr_8_al            , 0xffff, 0x4239, 20},
  {m68k_op_clr_16_aw           , 0xffff, 0x4278, 16},
  {m68k_op_clr_16_al           , 0xffff, 0x4279, 20},
  {m68k_op_clr_32_aw           , 0xffff, 0x42b8, 24},
  {m68k_op_clr_32_al           , 0xffff, 0x42b9, 28},
  {m68k_op_neg_8_pi7           , 0xffff, 0x441f, 12},
  {m68k_op_neg_8_pd7           , 0xffff, 0x4427, 14},
  {m68k_op_neg_8_aw            , 0xffff, 0x4438, 16},
  {m68k_op_neg_8_al            , 0xffff, 0x4439, 20},
  {m68k_op_neg_16_aw           , 0xffff, 0x4478, 16},
  {m68k_op_neg_16_al           , 0xffff, 0x4479, 20},
  {m68k_op_neg_32_aw           , 0xffff, 0x44b8, 24},
  {m68k_op_neg_32_al           , 0xffff, 0x44b9, 28},
  {m68k_op_move_16_toc_aw      , 0xffff, 0x44f8, 20},
  {m68k_op_move_16_toc_al      , 0xffff, 0x44f9, 24},
  {m68k_op_move_16_toc_pcdi    , 0xffff, 0x44fa, 20},
  {m68k_op_move_16_toc_pcix    , 0xffff, 0x44fb, 22},
  {m68k_op_move_16_toc_i       , 0xffff, 0x44fc, 16},
  {m68k_op_not_8_pi7           , 0xffff, 0x461f, 12},
  {m68k_op_not_8_pd7           , 0xffff, 0x4627, 14},
  {m68k_op_not_8_aw            , 0xffff, 0x4638, 16},
  {m68k_op_not_8_al            , 0xffff, 0x4639, 20},
  {m68k_op_not_16_aw           , 0xffff, 0x4678, 16},
  {m68k_op_not_16_al           , 0xffff, 0x4679, 20},
  {m68k_op_not_32_aw           , 0xffff, 0x46b8, 24},
  {m68k_op_not_32_al           , 0xffff, 0x46b9, 28},
  {m68k_op_move_16_tos_aw      , 0xffff, 0x46f8, 20},
  {m68k_op_move_16_tos_al      , 0xffff, 0x46f9, 24},
  {m68k_op_move_16_tos_pcdi    , 0xffff, 0x46fa, 20},
  {m68k_op_move_16_tos_pcix    , 0xffff, 0x46fb, 22},
  {m68k_op_move_16_tos_i       , 0xffff, 0x46fc, 16},
  {m68k_op_nbcd_8_pi7          , 0xffff, 0x481f, 12},
  {m68k_op_nbcd_8_pd7          , 0xffff, 0x4827, 14},
  {m68k_op_nbcd_8_aw           , 0xffff, 0x4838, 16},
  {m68k_op_nbcd_8_al           , 0xffff, 0x4839, 20},
  {m68k_op_pea_32_aw           , 0xffff, 0x4878, 16},
  {m68k_op_pea_32_al           , 0xffff, 0x4879, 20},
  {m68k_op_pea_32_pcdi         , 0xffff, 0x487a, 16},
  {m68k_op_pea_32_pcix         , 0xffff, 0x487b, 20},
  {m68k_op_movem_16_re_aw      , 0xffff, 0x48b8, 12},
  {m68k_op_movem_16_re_al      , 0xffff, 0x48b9, 16},
  {m68k_op_movem_32_re_aw      , 0xffff, 0x48f8, 12},
  {m68k_op_movem_32_re_al      , 0xffff, 0x48f9, 16},
  {m68k_op_tst_8_pi7           , 0xffff, 0x4a1f,  8},
  {m68k_op_tst_8_pd7           , 0xffff, 0x4a27, 10},
  {m68k_op_tst_8_aw            , 0xffff, 0x4a38, 12},
  {m68k_op_tst_8_al            , 0xffff, 0x4a39, 16},
  {m68k_op_tst_16_aw           , 0xffff, 0x4a78, 12},
  {m68k_op_tst_16_al           , 0xffff, 0x4a79, 16},
  {m68k_op_tst_32_aw           , 0xffff, 0x4ab8, 16},
  {m68k_op_tst_32_al           , 0xffff, 0x4ab9, 20},
  {m68k_op_tas_8_pi7           , 0xffff, 0x4adf, 18},
  {m68k_op_tas_8_pd7           , 0xffff, 0x4ae7, 20},
  {m68k_op_tas_8_aw            , 0xffff, 0x4af8, 22},
  {m68k_op_tas_8_al            , 0xffff, 0x4af9, 26},
  {m68k_op_illegal             , 0xffff, 0x4afc,  4},
  {m68k_op_movem_16_er_aw      , 0xffff, 0x4cb8, 16},
  {m68k_op_movem_16_er_al      , 0xffff, 0x4cb9, 20},
  {m68k_op_movem_16_er_pcdi    , 0xffff, 0x4cba, 16},
  {m68k_op_movem_16_er_pcix    , 0xffff, 0x4cbb, 18},
  {m68k_op_movem_32_er_aw      , 0xffff, 0x4cf8, 16},
  {m68k_op_movem_32_er_al      , 0xffff, 0x4cf9, 20},
  {m68k_op_movem_32_er_pcdi    , 0xffff, 0x4cfa, 16},
  {m68k_op_movem_32_er_pcix    , 0xffff, 0x4cfb, 18},
  {m68k_op_link_16_a7          , 0xffff, 0x4e57, 16},
  {m68k_op_unlk_32_a7          , 0xffff, 0x4e5f, 12},
  {m68k_op_reset               , 0xffff, 0x4e70,  0},
  {m68k_op_nop                 , 0xffff, 0x4e71,  4},
  {m68k_op_stop                , 0xffff, 0x4e72,  4},
  {m68k_op_rte_32              , 0xffff, 0x4e73, 20},
  {m68k_op_rts_32              , 0xffff, 0x4e75, 16},
  {m68k_op_trapv               , 0xffff, 0x4e76,  4},
  {m68k_op_rtr_32              , 0xffff, 0x4e77, 20},
  {m68k_op_jsr_32_aw           , 0xffff, 0x4eb8, 18},
  {m68k_op_jsr_32_al           , 0xffff, 0x4eb9, 20},
  {m68k_op_jsr_32_pcdi         , 0xffff, 0x4eba, 18},
  {m68k_op_jsr_32_pcix         , 0xffff, 0x4ebb, 22},
  {m68k_op_jmp_32_aw           , 0xffff, 0x4ef8, 10},
  {m68k_op_jmp_32_al           , 0xffff, 0x4ef9, 12},
  {m68k_op_jmp_32_pcdi         , 0xffff, 0x4efa, 10},
  {m68k_op_jmp_32_pcix         , 0xffff, 0x4efb, 14},
  {m68k_op_st_8_pi7            , 0xffff, 0x50df, 12},
  {m68k_op_st_8_pd7            , 0xffff, 0x50e7, 14},
  {m68k_op_st_8_aw             , 0xffff, 0x50f8, 16},
  {m68k_op_st_8_al             , 0xffff, 0x50f9, 20},
  {m68k_op_sf_8_pi7            , 0xffff, 0x51df, 12},
  {m68k_op_sf_8_pd7            , 0xffff, 0x51e7, 14},
  {m68k_op_sf_8_aw             , 0xffff, 0x51f8, 16},
  {m68k_op_sf_8_al             , 0xffff, 0x51f9, 20},
  {m68k_op_shi_8_pi7           , 0xffff, 0x52df, 12},
  {m68k_op_shi_8_pd7           , 0xffff, 0x52e7, 14},
  {m68k_op_shi_8_aw            , 0xffff, 0x52f8, 16},
  {m68k_op_shi_8_al            , 0xffff, 0x52f9, 20},
  {m68k_op_sls_8_pi7           , 0xffff, 0x53df, 12},
  {m68k_op_sls_8_pd7           , 0xffff, 0x53e7, 14},
  {m68k_op_sls_8_aw            , 0xffff, 0x53f8, 16},
  {m68k_op_sls_8_al            , 0xffff, 0x53f9, 20},
  {m68k_op_scc_8_pi7           , 0xffff, 0x54df, 12},
  {m68k_op_scc_8_pd7           , 0xffff, 0x54e7, 14},
  {m68k_op_scc_8_aw            , 0xffff, 0x54f8, 16},
  {m68k_op_scc_8_al            , 0xffff, 0x54f9, 20},
  {m68k_op_scs_8_pi7           , 0xffff, 0x55df, 12},
  {m68k_op_scs_8_pd7           , 0xffff, 0x55e7, 14},
  {m68k_op_scs_8_aw            , 0xffff, 0x55f8, 16},
  {m68k_op_scs_8_al            , 0xffff, 0x55f9, 20},
  {m68k_op_sne_8_pi7           , 0xffff, 0x56df, 12},
  {m68k_op_sne_8_pd7           , 0xffff, 0x56e7, 14},
  {m68k_op_sne_8_aw            , 0xffff, 0x56f8, 16},
  {m68k_op_sne_8_al            , 0xffff, 0x56f9, 20},
  {m68k_op_seq_8_pi7           , 0xffff, 0x57df, 12},
  {m68k_op_seq_8_pd7           , 0xffff, 0x57e7, 14},
  {m68k_op_seq_8_aw            , 0xffff, 0x57f8, 16},
  {m68k_op_seq_8_al            , 0xffff, 0x57f9, 20},
  {m68k_op_svc_8_pi7           , 0xffff, 0x58df, 12},
  {m68k_op_svc_8_pd7           , 0xffff, 0x58e7, 14},
  {m68k_op_svc_8_aw            , 0xffff, 0x58f8, 16},
  {m68k_op_svc_8_al            , 0xffff, 0x58f9, 20},
  {m68k_op_svs_8_pi7           , 0xffff, 0x59df, 12},
  {m68k_op_svs_8_pd7           , 0xffff, 0x59e7, 14},
  {m68k_op_svs_8_aw            , 0xffff, 0x59f8, 16},
  {m68k_op_svs_8_al            , 0xffff, 0x59f9, 20},
  {m68k_op_spl_8_pi7           , 0xffff, 0x5adf, 12},
  {m68k_op_spl_8_pd7           , 0xffff, 0x5ae7, 14},
  {m68k_op_spl_8_aw            , 0xffff, 0x5af8, 16},
  {m68k_op_spl_8_al            , 0xffff, 0x5af9, 20},
  {m68k_op_smi_8_pi7           , 0xffff, 0x5bdf, 12},
  {m68k_op_smi_8_pd7           , 0xffff, 0x5be7, 14},
  {m68k_op_smi_8_aw            , 0xffff, 0x5bf8, 16},
  {m68k_op_smi_8_al            , 0xffff, 0x5bf9, 20},
  {m68k_op_sge_8_pi7           , 0xffff, 0x5cdf, 12},
  {m68k_op_sge_8_pd7           , 0xffff, 0x5ce7, 14},
  {m68k_op_sge_8_aw            , 0xffff, 0x5cf8, 16},
  {m68k_op_sge_8_al            , 0xffff, 0x5cf9, 20},
  {m68k_op_slt_8_pi7           , 0xffff, 0x5ddf, 12},
  {m68k_op_slt_8_pd7           , 0xffff, 0x5de7, 14},
  {m68k_op_slt_8_aw            , 0xffff, 0x5df8, 16},
  {m68k_op_slt_8_al            , 0xffff, 0x5df9, 20},
  {m68k_op_sgt_8_pi7           , 0xffff, 0x5edf, 12},
  {m68k_op_sgt_8_pd7           , 0xffff, 0x5ee7, 14},
  {m68k_op_sgt_8_aw            , 0xffff, 0x5ef8, 16},
  {m68k_op_sgt_8_al            , 0xffff, 0x5ef9, 20},
  {m68k_op_sle_8_pi7           , 0xffff, 0x5fdf, 12},
  {m68k_op_sle_8_pd7           , 0xffff, 0x5fe7, 14},
  {m68k_op_sle_8_aw            , 0xffff, 0x5ff8, 16},
  {m68k_op_sle_8_al            , 0xffff, 0x5ff9, 20},
  {m68k_op_bra_16              , 0xffff, 0x6000, 10},
  {m68k_op_bra_32              , 0xffff, 0x60ff, 10},
  {m68k_op_bsr_16              , 0xffff, 0x6100, 18},
  {m68k_op_bsr_32              , 0xffff, 0x61ff, 18},
  {m68k_op_bhi_16              , 0xffff, 0x6200, 10},
  {m68k_op_bhi_32              , 0xffff, 0x62ff, 10},
  {m68k_op_bls_16              , 0xffff, 0x6300, 10},
  {m68k_op_bls_32              , 0xffff, 0x63ff, 10},
  {m68k_op_bcc_16              , 0xffff, 0x6400, 10},
  {m68k_op_bcc_32              , 0xffff, 0x64ff, 10},
  {m68k_op_bcs_16              , 0xffff, 0x6500, 10},
  {m68k_op_bcs_32              , 0xffff, 0x65ff, 10},
  {m68k_op_bne_16              , 0xffff, 0x6600, 10},
  {m68k_op_bne_32              , 0xffff, 0x66ff, 10},
  {m68k_op_beq_16              , 0xffff, 0x6700, 10},
  {m68k_op_beq_32              , 0xffff, 0x67ff, 10},
  {m68k_op_bvc_16              , 0xffff, 0x6800, 10},
  {m68k_op_bvc_32              , 0xffff, 0x68ff, 10},
  {m68k_op_bvs_16              , 0xffff, 0x6900, 10},
  {m68k_op_bvs_32              , 0xffff, 0x69ff, 10},
  {m68k_op_bpl_16              , 0xffff, 0x6a00, 10},
  {m68k_op_bpl_32              , 0xffff, 0x6aff, 10},
  {m68k_op_bmi_16              , 0xffff, 0x6b00, 10},
  {m68k_op_bmi_32              , 0xffff, 0x6bff, 10},
  {m68k_op_bge_16              , 0xffff, 0x6c00, 10},
  {m68k_op_bge_32              , 0xffff, 0x6cff, 10},
  {m68k_op_blt_16              , 0xffff, 0x6d00, 10},
  {m68k_op_blt_32              , 0xffff, 0x6dff, 10},
  {m68k_op_bgt_16              , 0xffff, 0x6e00, 10},
  {m68k_op_bgt_32              , 0xffff, 0x6eff, 10},
  {m68k_op_ble_16              , 0xffff, 0x6f00, 10},
  {m68k_op_ble_32              , 0xffff, 0x6fff, 10},
  {m68k_op_sbcd_8_mm_axy7      , 0xffff, 0x8f0f, 18},
  {m68k_op_subx_8_mm_axy7      , 0xffff, 0x9f0f, 18},
  {m68k_op_cmpm_8_axy7         , 0xffff, 0xbf0f, 12},
  {m68k_op_abcd_8_mm_axy7      , 0xffff, 0xcf0f, 18},
  {m68k_op_addx_8_mm_axy7      , 0xffff, 0xdf0f, 18},
  {m68k_op_asr_16_aw           , 0xffff, 0xe0f8, 16},
  {m68k_op_asr_16_al           , 0xffff, 0xe0f9, 20},
  {m68k_op_asl_16_aw           , 0xffff, 0xe1f8, 16},
  {m68k_op_asl_16_al           , 0xffff, 0xe1f9, 20},
  {m68k_op_lsr_16_aw           , 0xffff, 0xe2f8, 16},
  {m68k_op_lsr_16_al           , 0xffff, 0xe2f9, 20},
  {m68k_op_lsl_16_aw           , 0xffff, 0xe3f8, 16},
  {m68k_op_lsl_16_al           , 0xffff, 0xe3f9, 20},
  {m68k_op_roxr_16_aw          , 0xffff, 0xe4f8, 16},
  {m68k_op_roxr_16_al          , 0xffff, 0xe4f9, 20},
  {m68k_op_roxl_16_aw          , 0xffff, 0xe5f8, 16},
  {m68k_op_roxl_16_al          , 0xffff, 0xe5f9, 20},
  {m68k_op_ror_16_aw           , 0xffff, 0xe6f8, 16},
  {m68k_op_ror_16_al           , 0xffff, 0xe6f9, 20},
  {m68k_op_rol_16_aw           , 0xffff, 0xe7f8, 16},
  {m68k_op_rol_16_al           , 0xffff, 0xe7f9, 20},
  {0, 0, 0, 0}
};


/* Build the opcode handler jump table */
static void m68ki_build_opcode_table(void)
{
  const opcode_handler_struct *ostruct;
  int instr;
  int i;
  int j;

  for(i = 0; i < 0x10000; i++)
  {
    /* default to illegal */
    m68ki_instruction_jump_table[i] = m68k_op_illegal;
    m68ki_cycles[i] = 4;
  }

  ostruct = &m68k_opcode_handler_table[0];
  while(ostruct->mask != 0xff00)
  {
    for(i = 0;i < 0x10000;i++)
    {
      if((i & ostruct->mask) == ostruct->match)
      {
        m68ki_instruction_jump_table[i] = ostruct->opcode_handler;
        m68ki_cycles[i] = ostruct->cycles * MUL;
      }
    }
    ostruct++;
  }
  while(ostruct->mask == 0xff00)
  {
    for(i = 0;i <= 0xff;i++)
    {
      m68ki_instruction_jump_table[ostruct->match | i] = ostruct->opcode_handler;
      m68ki_cycles[ostruct->match | i] = ostruct->cycles * MUL;
    }
    ostruct++;
  }
  while(ostruct->mask == 0xf1f8)
  {
    for(i = 0;i < 8;i++)
    {
      for(j = 0;j < 8;j++)
      {
        instr = ostruct->match | (i << 9) | j;
        m68ki_instruction_jump_table[instr] = ostruct->opcode_handler;
        m68ki_cycles[instr] = ostruct->cycles * MUL;
      }
    }
    ostruct++;
  }
  while(ostruct->mask == 0xfff0)
  {
    for(i = 0;i <= 0x0f;i++)
    {
      m68ki_instruction_jump_table[ostruct->match | i] = ostruct->opcode_handler;
      m68ki_cycles[ostruct->match | i] = ostruct->cycles * MUL;
    }
    ostruct++;
  }
  while(ostruct->mask == 0xf1ff)
  {
    for(i = 0;i <= 0x07;i++)
    {
      m68ki_instruction_jump_table[ostruct->match | (i << 9)] = ostruct->opcode_handler;
      m68ki_cycles[ostruct->match | (i << 9)] = ostruct->cycles * MUL;
    }
    ostruct++;
  }
  while(ostruct->mask == 0xfff8)
  {
    for(i = 0;i <= 0x07;i++)
    {
      m68ki_instruction_jump_table[ostruct->match | i] = ostruct->opcode_handler;
      m68ki_cycles[ostruct->match | i] = ostruct->cycles * MUL;
    }
    ostruct++;
  }
  while(ostruct->mask == 0xffff)
  {
    m68ki_instruction_jump_table[ostruct->match] = ostruct->opcode_handler;
    m68ki_cycles[ostruct->match] = ostruct->cycles * MUL;
    ostruct++;
  }
}

#endif

/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */


