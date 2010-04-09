/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

/* MJPEG-ish decoder based on the algorithm and huffman data tables provided by David Michel of MagicEngine and MagicEngine-FX */

#include "pcfx.h"
#include "rainbow.h"
#include "king.h"
#include "interrupt.h"
#include "jrevdct.h"
#include "../clamp.h"

static const uint8 zigzag[63] =
{
 0x01, 0x08, 0x10, 0x09, 0x02, 0x03, 0x0A, 0x11,
 0x18, 0x20, 0x19, 0x12, 0x0B, 0x04, 0x05, 0x0C,
 0x13, 0x1A, 0x21, 0x28, 0x30, 0x29, 0x22, 0x1B,
 0x14, 0x0D, 0x06, 0x07, 0x0E, 0x15, 0x1C, 0x23,
 0x2A, 0x31, 0x38, 0x39, 0x32, 0x2B, 0x24, 0x1D,
 0x16, 0x0F, 0x17, 0x1E, 0x25, 0x2C, 0x33, 0x3A,
 0x3B, 0x34, 0x2D, 0x26, 0x1F, 0x27, 0x2E, 0x35,
 0x3C, 0x3D, 0x36, 0x2F, 0x37, 0x3E, 0x3F
};

/* Y = luminance/luma, UV = chrominance/chroma */

typedef struct
{
	const uint8 *base;
        const uint8 *codes;
	const uint32 *minimum;
	const uint32 *maximum;
} HuffmanTable;

/* Luma DC Huffman tables */
static const uint8 dc_y_base[17] =
{
        0x00, 0x00, 0x00, 0x00, 0x07, 0x00, 0x08, 0x09,
        0x00, 0x0B, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00
};

static const uint8 dc_y_codes[27] =
{
        0x00, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x01,
        0x08, 0x09, 0x0F, 0x10, 0x11, 0x12, 0x13, 0x14,
        0x15, 0x16, 0x17, 0x18, 0x19, 0x1A, 0x1B, 0x1C,
        0x1D, 0x1E, 0x1F
};

static const uint32 dc_y_minimum[17] =
{
        0x0000, 0x0000, 0x0000, 0x0000, 0x000E, 0x0000, 0x003C, 0x007A,
        0x0000, 0x01F0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000
};

static const uint32 dc_y_maximum[17] = 
{
        0xFFFF, 0xFFFF, 0xFFFF, 0x0006, 0x000E, 0xFFFF, 0x003C, 0x007B,
        0xFFFF, 0x01FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
        0xFFFF
};

static const HuffmanTable dc_y_table =
{
	dc_y_base,
	dc_y_codes,
	dc_y_minimum,
	dc_y_maximum
};

/* Chroma DC Huffman tables */

static const uint8 dc_uv_base[17] =
{
        0x00, 0x00, 0x00, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00
};

static const uint8 dc_uv_codes[10] =
{
        0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
        0x08, 0x09
};

static const uint32 dc_uv_minimum[17] =
{
        0x0000, 0x0000, 0x0000, 0x0006, 0x000E, 0x001E, 0x003E, 0x007E,
        0x00FE, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
        0x0000
};

static const uint32 dc_uv_maximum[17] =
{
        0xFFFF, 0xFFFF, 0x0002, 0x0006, 0x000E, 0x001E, 0x003E, 0x007E,
        0x00FF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF, 0xFFFF,
        0xFFFF
};

static const HuffmanTable dc_uv_table =
{
	dc_uv_base,
	dc_uv_codes,
	dc_uv_minimum,
	dc_uv_maximum
};

/* Luma AC Huffman tables */
static const uint8 ac_y_base[17] =
{
        0x00, 0x00, 0x00, 0x02, 0x03, 0x05, 0x09, 0x0D,
        0x00, 0x10, 0x00, 0x12, 0x22, 0x00, 0x00, 0x00,
        0x00
};

static const uint8 ac_y_codes[146] =
{
        0x01, 0x02, 0x03, 0x04, 0x11, 0x05, 0x12, 0x21,
        0x00, 0x06, 0x31, 0x41, 0x51, 0x13, 0x22, 0x61,
        0x07, 0x71, 0x09, 0x19, 0x29, 0x39, 0x49, 0x59,
        0x69, 0x79, 0x89, 0x99, 0xA9, 0xB9, 0xC9, 0xD9,
        0xE9, 0xF9, 0x08, 0x14, 0x15, 0x16, 0x17, 0x18,
        0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x32, 0x33,
        0x34, 0x35, 0x36, 0x37, 0x38, 0x42, 0x43, 0x44,
        0x45, 0x46, 0x47, 0x48, 0x52, 0x53, 0x54, 0x55,
        0x56, 0x57, 0x58, 0x62, 0x63, 0x64, 0x65, 0x66,
        0x67, 0x68, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77,
        0x78, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
        0x98, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
        0xA8, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
        0xB8, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
        0xC8, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
        0xD8, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
        0xE8, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
        0xF8, 0x10
};

static const uint32 ac_y_minimum[17] =
{
        0x0000, 0x0000, 0x0000, 0x0004, 0x000A, 0x0018, 0x0036, 0x0074,
        0x0000, 0x01DC, 0x0000, 0x0778, 0x0F10, 0x0000, 0x0000, 0x0000,
        0x0000
};

static const uint32 ac_y_maximum[17] =
{
        0xFFFF, 0xFFFF, 0x0001, 0x0004, 0x000B, 0x001A, 0x0039, 0x0076,
        0xFFFF, 0x01DD, 0xFFFF, 0x0787, 0x0F7F, 0xFFFF, 0xFFFF, 0xFFFF,
        0xFFFF
};

static const HuffmanTable ac_y_table =
{
	ac_y_base,
	ac_y_codes,
	ac_y_minimum,
	ac_y_maximum
};

/* Chroma AC Huffman tables */
static const uint8 ac_uv_base[17] =
{
        0x00, 0x00, 0x00, 0x02, 0x03, 0x05, 0x0A, 0x0B,
        0x00, 0x10, 0x00, 0x12, 0x22, 0x00, 0x00, 0x00,
        0x00
};

static const uint8 ac_uv_codes[146] =
{
        0x01, 0x02, 0x11, 0x03, 0x21, 0x04, 0x12, 0x31,
        0x41, 0x00, 0x51, 0x05, 0x13, 0x22, 0x61, 0x71,
        0x32, 0x81, 0x09, 0x19, 0x29, 0x39, 0x49, 0x59,
        0x69, 0x79, 0x89, 0x99, 0xA9, 0xB9, 0xC9, 0xD9,
        0xE9, 0xF9, 0x06, 0x07, 0x08, 0x14, 0x15, 0x16,
        0x17, 0x18, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28,
        0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x42, 0x43,
        0x44, 0x45, 0x46, 0x47, 0x48, 0x52, 0x53, 0x54,
        0x55, 0x56, 0x57, 0x58, 0x62, 0x63, 0x64, 0x65,
        0x66, 0x67, 0x68, 0x72, 0x73, 0x74, 0x75, 0x76,
        0x77, 0x78, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87,
        0x88, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97,
        0x98, 0xA1, 0xA2, 0xA3, 0xA4, 0xA5, 0xA6, 0xA7,
        0xA8, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7,
        0xB8, 0xC1, 0xC2, 0xC3, 0xC4, 0xC5, 0xC6, 0xC7,
        0xC8, 0xD1, 0xD2, 0xD3, 0xD4, 0xD5, 0xD6, 0xD7,
        0xD8, 0xE1, 0xE2, 0xE3, 0xE4, 0xE5, 0xE6, 0xE7,
        0xE8, 0xF1, 0xF2, 0xF3, 0xF4, 0xF5, 0xF6, 0xF7,
        0xF8, 0x10
};

static const uint32 ac_uv_minimum[17] =
{
        0x0000, 0x0000, 0x0000, 0x0004, 0x000A, 0x0018, 0x0038, 0x0072,
        0x0000, 0x01DC, 0x0000, 0x0778, 0x0F10, 0x0000, 0x0000, 0x0000,
        0x0000
};

static const uint32 ac_uv_maximum[17] =
{
        0xFFFF, 0xFFFF, 0x0001, 0x0004, 0x000B, 0x001B, 0x0038, 0x0076,
        0xFFFF, 0x01DD, 0xFFFF, 0x0787, 0x0F7F, 0xFFFF, 0xFFFF, 0xFFFF,
        0xFFFF
};

static const HuffmanTable ac_uv_table =
{
	ac_uv_base,
	ac_uv_codes,
	ac_uv_minimum,
	ac_uv_maximum
};

static uint8 *DecodeBuffer[2] = { NULL, NULL };
static int DecodeFormat[2]; // The format each buffer is in(0 = palettized 8-bit, 1 = YUV)
static uint32 QuantTables[2][64], QuantTablesBase[2][64];	// 0 = Y, 1 = UV

static uint32 DecodeBufferWhichRead;

static int32 RasterReadPos;
static uint16 Control;
static uint16 NullRunY, NullRunU, NullRunV, HSync;
static uint16 HScroll;

static uint32 bits_buffer;
static uint32 bits_buffered_bits;
static int32 bits_bytes_left;

static void InitBits(int32 bcount)
{
 bits_bytes_left = bcount;
 bits_buffer = 0;
 bits_buffered_bits = 0;
}

static ALWAYS_INLINE uint8 FetchWidgywabbit(void)
{
 if(bits_bytes_left <= 0)
  return(0);

 uint8 ret = KING_RB_Fetch();
 if(ret == 0xFF) 
  KING_RB_Fetch();

 bits_bytes_left--;

 return(ret);
}

enum
{
 MDFNBITS_PEEK = 1,
 MDFNBITS_FUNNYSIGN = 2,
};

static ALWAYS_INLINE uint32 GetBits(const unsigned int count, const unsigned int how = 0)
{
 uint32 ret;

 while(bits_buffered_bits < count)
 {
  bits_buffer <<= 8;
  bits_buffer |= FetchWidgywabbit();
  bits_buffered_bits += 8;
 }

 ret = (bits_buffer >> (bits_buffered_bits - count)) & ((1 << count) - 1);

 if(!(how & MDFNBITS_PEEK))
  bits_buffered_bits -= count;

 if((how & MDFNBITS_FUNNYSIGN) && count)
 {
  if(ret < (1 << (count - 1)))
   ret += 1 - (1 << count);
 }

 return(ret);
}

static ALWAYS_INLINE void SkipBits(const unsigned int count)
{
 while(bits_buffered_bits < count)
 {
  bits_buffer <<= 8;
  bits_buffer |= FetchWidgywabbit();
  bits_buffered_bits += 8;
 }

 bits_buffered_bits -= count;
}


static uint32 HappyColor; // Cached, calculated from null run yuv registers;
static void CalcHappyColor(void)
{
 uint8 y_c = (int16)NullRunY + 0x80;
 uint8 u_c = (int16)NullRunU + 0x80;
 uint8 v_c = (int16)NullRunV + 0x80;

 HappyColor = (y_c << 16) | (u_c << 8) | (v_c << 0);
}

static uint32 get_ac_coeff(const HuffmanTable *table, int32 *zeroes)
{
 unsigned int numbits;
 uint32 code;

 for(numbits = 2; ; numbits++)
 {
  code = GetBits(numbits, MDFNBITS_PEEK);
  
  if((numbits == 5) && (code == 0x1F))
  {
   SkipBits(5);
   *zeroes = 0;
   return(0);
  }
  if((code <= table->maximum[numbits]) && (table->maximum[numbits] != 0xFFFF))
  {
   SkipBits(numbits);
   code = table->codes[table->base[numbits] + (code - table->minimum[numbits])];
   break;
  }
 }

 numbits = code & 0xF;
 *zeroes = code >> 4;

 return(GetBits(numbits, MDFNBITS_FUNNYSIGN));
}

static uint32 get_dc_coeff(const HuffmanTable *table, int32 *zeroes)
{
 unsigned int numbits;
 uint32 code;

 for(;;)
 {
  for(numbits = 2; ; numbits++)
  {
   code = GetBits(numbits, MDFNBITS_PEEK);
   if((code <= table->maximum[numbits]) && (table->maximum[numbits] != 0xFFFF))
   {
    SkipBits(numbits);
    code = table->codes[table->base[numbits] + (code - table->minimum[numbits])];
    break;
   }
  }

  if(code < 0xF)
  {
   *zeroes = 0;
   return(GetBits(code, MDFNBITS_FUNNYSIGN));
  }
  else if(code == 0xF)
  {
   get_ac_coeff(&ac_y_table, zeroes);
   (*zeroes)++;
   return(0);
  }
  else if(code >= 0x10)
  {
   code -= 0x10;

   for(int i = 0; i < 64; i++)
   {
    // Y
    uint32 coeff = (QuantTablesBase[0][i] * code) >> 2;

    if(coeff < 1)
     coeff = 1;
    else if(coeff > 0xFE)
     coeff = 0xFE;

    QuantTables[0][i] = coeff;

    // UV
    if(i)
     coeff = (QuantTablesBase[1][i] * code) >> 2;
    else
     coeff = (QuantTablesBase[1][i]) >> 2;

    if(coeff < 1)
     coeff = 1;
    else if(coeff > 0xFE)
     coeff = 0xFE;

    QuantTables[1][i] = coeff;
   }

  }
 }

}

static void decode(int32 *dct, const uint32 *QuantTable, const int32 dc, const HuffmanTable *table)
{
 int32 coeff;
 int zeroes;
 int count;
 int index;

 dct[0] = (int16)(QuantTable[0] * dc);
 count = 0;

 do
 {
  coeff = get_ac_coeff(table, &zeroes);
  if(!coeff)
  {
   if(!zeroes)
   {
    while(count < 63)
    {
     dct[zigzag[count++]] = 0;
    }
    break;
   }
   else if(zeroes == 1)
    zeroes = 0xF;
  }
  
  while(zeroes-- && count < 63)
  {
   dct[zigzag[count++]] = 0;
  }
  zeroes = 0;
  if(count < 63)
  {
   index = zigzag[count++];
   dct[index] = (int16)(QuantTable[index] * coeff);
  }
 } while(count < 63);

}

#ifdef WANT_DEBUGGER
static RegType RainbowRegs[] =
{
 { "RSCRLL", "Rainbow Horizontal Scroll", 2 },
 { "RCTRL", "Rainbow Control", 2 },
 { "RNRY", "Rainbow Null Run Y", 2 },
 { "RNRU", "Rainbow Null Run U", 2 },
 { "RNRV", "Rainbow Null Run V", 2 },
 { "------", "", 0xFFFF },
        { "BGMODE", "Background Mode", 2 },
        { "BGPRIO", "Background Priority", 2 },
        { "BGSCRM", "Background Scroll Mode", 2 },
        { "BGSIZ0", "Background 0 Size", 2 },
        { "BGSIZ1", "Background 1 Size", 2 },
        { "BGSIZ2", "Background 2 Size", 2 },
        { "BGSIZ3", "Background 3 Size", 2 },
        { "BGXSC0", "Background 0 X Scroll", 2 },
        { "BGXSC1", "Background 1 X Scroll", 2 },
        { "BGXSC2", "Background 2 X Scroll", 2 },
        { "BGXSC3", "Background 3 X Scroll", 2 },
        { "BGYSC0", "Background 0 Y Scroll", 2 },
        { "BGYSC1", "Background 1 Y Scroll", 2 },
        { "BGYSC2", "Background 2 Y Scroll", 2 },
        { "BGYSC3", "Background 3 Y Scroll", 2 },

        { "BGBAT0", "Background 0 BAT Address", 2 },
        { "BGBATS", "Background SUB BAT Address", 2 },
        { "BGBAT1", "Background 1 BAT Address", 2 },
        { "BGBAT2", "Background 2 BAT Address", 2 },
        { "BGBAT3", "Background 3 BAT Address", 2 },
        { "BGCG0", "Background 0 CG Address", 2 },
        { "BGCGS", "Background SUB CG Address", 2 },
        { "BGCG1", "Background 1 CG Address", 2 },
        { "BGCG2", "Background 2 CG Address", 2 },
        { "BGCG3", "Background 3 CG Address", 2 },

 { "BGAFFINA", "Background Affin Coefficient A", 2 },
 { "BGAFFINB", "Background Affin Coefficient B", 2 },
 { "BGAFFINC", "Background Affin Coefficient C", 2 },
 { "BGAFFIND", "Background Affin Coefficient D", 2 },
 { "BGAFFINX", "Background Affin Center X", 2 },
 { "BGAFFINY", "Background Affin Center Y", 2 },
 { "MPROG0", "Micro-program", 2},
 { "MPROG1", "Micro-program", 2},
 { "MPROG2", "Micro-program", 2},
 { "MPROG3", "Micro-program", 2},
 { "MPROG4", "Micro-program", 2},
 { "MPROG5", "Micro-program", 2},
 { "MPROG6", "Micro-program", 2},
 { "MPROG7", "Micro-program", 2},
 { "MPROG8", "Micro-program", 2},
 { "MPROG9", "Micro-program", 2},
 { "MPROGA", "Micro-program", 2},
 { "MPROGB", "Micro-program", 2},
 { "MPROGC", "Micro-program", 2},
 { "MPROGD", "Micro-program", 2},
 { "MPROGE", "Micro-program", 2},
 { "MPROGF", "Micro-program", 2},

 // EVIL EVIL:

 { "", "", 0 },
};

static uint32 RainbowDBG_GetRegister(const std::string &name, std::string *special)
{
 uint32 value = 0xDEADBEEF;

 if(name ==  "RSCRLL")
  value = HScroll;
 else if(name == "RCTRL")
  value = Control;
 else if(name == "RNRY")
  value = NullRunY;
 else if(name == "RNRU")
  value = NullRunU;
 else if(name == "RNRV")
  value = NullRunV;
 else
  value = KING_GetRegister(name, special); // EVIL EVIL
 return(value);
}

static void RainbowDBG_SetRegister(const std::string &name, uint32 value)
{
 if(name == "RSCRLL")
  HScroll = value & 0x1FF;
 else if(name == "RCTRL")
  Control = value;
 else if(name == "RNRY")
  NullRunY = value;
 else if(name == "RNRU")
  NullRunU = value;
 else if(name == "RNRV")
  NullRunV = value;
 else
  KING_SetRegister(name, value);
}

static RegGroupType RainbowRegsGroup =
{
 RainbowRegs,
 RainbowDBG_GetRegister,
 RainbowDBG_SetRegister
};

#endif

bool RAINBOW_Init(void)
{
 #ifdef WANT_DEBUGGER
 MDFNDBG_AddRegGroup(&RainbowRegsGroup);
 #endif

 for(int i = 0; i < 2; i++)
 {
  if(!(DecodeBuffer[i] = (uint8*)MDFN_malloc(0x2000 * 4, _("RAINBOW buffer RAM"))))
   return(0);
  memset(DecodeBuffer[i], 0, 0x2000 * 4);
 }

 return(1);
}

void RAINBOW_Close(void)
{
 for(int i = 0; i < 2; i++)
  if(DecodeBuffer[i])
  {
   free(DecodeBuffer[i]);
   DecodeBuffer[i] = NULL;
  }
}

// RAINBOW base I/O port address: 0x200

#define REGSETBOFW(_reg, _data, _wb) { (_reg) &= ~(0xFF << ((_wb) * 8)); (_reg) |= (_data) << ((_wb) * 8); }
#define REGSETBOFH REGSETBOFW

// The horizontal scroll register is set up kind of weird...D0-D7 of writes to $200 set the lower byte, D0-D7 of writes to $202 set the upper byte

void RAINBOW_Write8(uint32 A, uint8 V)
{
 //printf("RAINBOW Wr8: %08x %02x\n", A, V);
 switch(A & 0x1C)
 {
  case 0x00: REGSETBOFH(HScroll, V, (A & 0x2) >> 1); HScroll &= 0x1FF; break;
  case 0x04: REGSETBOFH(Control, V, A & 0x3); break;
  case 0x08: REGSETBOFH(NullRunY, V, A & 0x3); CalcHappyColor(); break;
  case 0x0C: REGSETBOFH(NullRunU, V, A & 0x3); CalcHappyColor(); break;
  case 0x10: REGSETBOFH(NullRunV, V, A & 0x3); CalcHappyColor(); break;
  case 0x14: REGSETBOFH(HSync, V, A & 0x3); break;
 }
}

void RAINBOW_Write16(uint32 A, uint16 V)
{
 int msh = A & 0x2;

 //printf("RAINBOW Wr16: %08x %04x\n", A, V);
 switch(A & 0x1C)
 {
  case 0x00: REGSETBOFH(HScroll, V & 0xFF, (A & 0x2) >> 1); HScroll &= 0x1FF; break;
  case 0x04: REGSETHW(Control, V, msh); break;
  case 0x08: REGSETHW(NullRunY, V, msh); CalcHappyColor(); break;
  case 0x0C: REGSETHW(NullRunU, V, msh); CalcHappyColor(); break;
  case 0x10: REGSETHW(NullRunV, V, msh); CalcHappyColor(); break;
  case 0x14: REGSETHW(HSync, V, msh); break;
 }
}

static uint32 LastLine[256];
static bool FirstDecode;
static bool GarbageData;

void RAINBOW_TransferStart(void)
{
 GarbageData = FALSE;
 FirstDecode = TRUE;
 //DecodeBufferWhichRead = 0;
 RasterReadPos = 0;
 //MDFN_FastU32MemsetM8((uint32*)DecodeBuffer[DecodeBufferWhichRead], 0x008080, 256 * 16);
 //memset(DecodeBuffer[DecodeBufferWhichRead], 0, 256 * 4 * 16);
 //printf("TransferStart, bufferread: %d\n", DecodeBufferWhichRead);
}

int RAINBOW_FetchRaster(uint8 *ptr, bool WantDecode, bool Skip)
{
 if(RasterReadPos == 0 && WantDecode)
 {
   uint8 block_type;
   int32 block_size;
   int icount;

   if(GarbageData)
    icount = 0;
   else
    icount = 0x200;

   do
   {
    do
    {
     while(KING_RB_Fetch() != 0xFF && icount > 0)
      icount--;

     block_type = KING_RB_Fetch();
     //if(icount > 0 && block_type != 0xF0 && block_type != 0xF1 && block_type != 0xF2 && block_type != 0xF3 && block_type != 0xF8 && block_type != 0xFF)
     //if(icount > 0 && block_type == 0x11)
     // printf("%02x\n", block_type);
     icount--;
    } while(block_type != 0xF0 && block_type != 0xF1 && block_type != 0xF2 && block_type != 0xF3 && block_type != 0xF8 && block_type != 0xFF && icount > 0);

    block_size = (int16)((KING_RB_Fetch() << 8) | (KING_RB_Fetch()));
    block_size -= 2;
    if(block_type == 0xFF && block_size <= 0)
     for(int i = 0; i < 128; i++,icount--) KING_RB_Fetch();
   } while(block_size <= 0 && icount > 0);

   //printf("%d\n", icount);
   if(icount <= 0)
   {
    GarbageData = TRUE;
    //printf("Dooom: %d\n");
    DecodeFormat[DecodeBufferWhichRead ^ 1] = 0;
    memset(DecodeBuffer[DecodeBufferWhichRead ^ 1], 0, 0x2000);
    goto BufferNoDecode;
   }

   if(block_type == 0xf8 || block_type == 0xff)
    DecodeFormat[DecodeBufferWhichRead ^ 1] = 1;
   else
    DecodeFormat[DecodeBufferWhichRead ^ 1] = 0;

   if(block_type == 0xF8 || block_type == 0xFF)
   {
    if(block_type == 0xFF)
    {
     for(int q = 0; q < 2; q++)
      for(int i = 0; i < 64; i++)
      {
       uint8 meow = KING_RB_Fetch();

       QuantTables[q][i] = meow; 
       QuantTablesBase[q][i] = meow;
      }
     block_size -= 128;
    }

    InitBits(block_size);

    int32 dc_y = 0, dc_u = 0, dc_v = 0;
    uint32 *dest_base = (uint32 *)DecodeBuffer[DecodeBufferWhichRead ^ 1];
    for(int column = 0; column < 16; column++)
    {
     uint32 *dest_base_column = &dest_base[column * 16];
     int zeroes;

     dc_y += get_dc_coeff(&dc_y_table, &zeroes);

     if(zeroes) // If set, clear the number of columns
     {
      do
      {
       if(column < 16)
       {
	dest_base_column = &dest_base[column * 16];

        for(int y = 0; y < 16; y++)
         for(int x = 0; x < 16; x++)
          dest_base_column[y * 256 + x] = HappyColor;
       }
       column++;
       zeroes--;
      } while(zeroes);
      column--; // Fix for the column autoincrement in the while(zeroes) loop
      dc_y = dc_u = dc_v = 0;
     }
     else
     {
      int32 dct_y[256];
      int32 dct_u[64];
      int32 dct_v[64];

      if(Skip) continue;

      // Y/Luma, 16x16 components
      decode(&dct_y[0x00], QuantTables[0], dc_y, &ac_y_table);
      dc_y += get_dc_coeff(&dc_y_table, &zeroes);
      decode(&dct_y[0x40], QuantTables[0], dc_y, &ac_y_table);
      dc_y += get_dc_coeff(&dc_y_table, &zeroes);
      decode(&dct_y[0x80], QuantTables[0], dc_y, &ac_y_table);
      dc_y += get_dc_coeff(&dc_y_table, &zeroes);
      decode(&dct_y[0xC0], QuantTables[0], dc_y, &ac_y_table);

      // U, 8x8 components
      dc_u += get_dc_coeff(&dc_uv_table, &zeroes);
      decode(&dct_u[0x00], QuantTables[1], dc_u, &ac_uv_table);

      // V, 8x8 components
      dc_v += get_dc_coeff(&dc_uv_table, &zeroes);
      decode(&dct_v[0x00], QuantTables[1], dc_v, &ac_uv_table);

      j_rev_dct(&dct_y[0x00]);
      j_rev_dct(&dct_y[0x40]);
      j_rev_dct(&dct_y[0x80]);
      j_rev_dct(&dct_y[0xC0]);
      j_rev_dct(&dct_u[0x00]);
      j_rev_dct(&dct_v[0x00]);

      for(int y = 0; y < 16; y++)
       for(int x = 0; x < 16; x++)
        dest_base_column[y * 256 + x] = clamp_to_u8(dct_y[y * 8 + (x & 0x7) + ((x & 0x8) << 4)] + 0x80) << 16;

      for(int y = 0; y < 8; y++)
      {
       for(int x = 0; x < 8; x++)
       {
        uint32 component_uv = (clamp_to_u8(dct_u[y * 8 + x] + 0x80) << 8) | clamp_to_u8(dct_v[y * 8 + x] + 0x80);
	dest_base_column[y * 512 + (256 * 1) + x * 2 + 0] |= component_uv;
       }
      }
     }
    }

    // Do bilinear interpolation on the chroma channels:
    if(!Skip)
    for(int y = 0; y < 16; y+= 2)
    {
     uint32 *linebase = &dest_base[y * 256];
     uint32 *linebase1 = &dest_base[(y + 1) * 256];

     for(int x = 0; x < 254; x += 2)
     {
      unsigned int u, v;

      u = (((linebase1[x] >> 8) & 0xFF) + ((linebase1[x + 2] >> 8) & 0xFF)) >> 1;
      v = (((linebase1[x] >> 0) & 0xFF) + ((linebase1[x + 2] >> 0) & 0xFF)) >> 1;

      linebase1[x + 1] = (linebase1[x + 1] & ~ 0xFFFF) | (u << 8) | v;
     }

     linebase1[0xFF] = (linebase1[0xFF] & ~ 0xFFFF) | (linebase1[0xFE] & 0xFFFF);

     if(FirstDecode)
     {
      for(int x = 0; x < 256; x++) linebase[x] = (linebase[x] & ~ 0xFFFF) | (linebase1[x] & 0xFFFF);
      FirstDecode = 0;
     }
     else
      for(int x = 0; x < 256; x++)
      {
       unsigned int u, v;         
 
       u = (((LastLine[x] >> 8) & 0xFF) + ((linebase1[x] >> 8) & 0xFF)) >> 1;
       v = (((LastLine[x] >> 0) & 0xFF) + ((linebase1[x] >> 0) & 0xFF)) >> 1;

       linebase[x] = (linebase[x] & ~ 0xFFFF) | (u << 8) | v;
      }

     memcpy(LastLine, linebase1, 256 * 4);
    }
   } // end jpeg-like decoding
   else 
   {
    unsigned int plt_shift;
    unsigned int crl_mask;
    int x = 0;
    
    switch(block_type)
    {
     default: // Not reached, but shuts up the compiler.
     case 0xF0: plt_shift = 4; crl_mask = 0xF; break; // 16 color
     case 0xF1: plt_shift = 3; crl_mask = 0x7; break; // 32 color
     case 0xF2: plt_shift = 2; crl_mask = 0x3; break; // 64 color
     case 0xF3: plt_shift = 1; crl_mask = 0x1; break; // 128 color
    }

    while(block_size > 0)
    {
     uint8 boot = KING_RB_Fetch();
     block_size--;
     unsigned int rle_count;

     if(boot == 0xFF)
     {
      KING_RB_Fetch();
      block_size--;
     }

     if(!(boot & crl_mask)) // Expand mode?
     {
      rle_count = KING_RB_Fetch();
      block_size--;
      if(rle_count == 0xFF) 
      {
       KING_RB_Fetch();
       block_size--;
      }
      rle_count++;
     }
     else
      rle_count = boot & crl_mask;

     for(unsigned int i = 0; i < rle_count; i++)
     {
      if(x >= 0x2000) 
      {
       //puts("Oops");
       break; // Don't overflow our decode buffer!
      }
      DecodeBuffer[DecodeBufferWhichRead ^ 1][x] = (boot >> plt_shift);
      x++;
     }
    }
   } // end RLE decoding

   //for(int i = 0; i < 8 + block_size; i++)
   // KING_RB_Fetch();

  BufferNoDecode: ;
 }
 int ret = DecodeFormat[DecodeBufferWhichRead];

 if(ptr)
 {
  uint16 tmpss = HScroll & 0x1FF;

  if(DecodeFormat[DecodeBufferWhichRead] == 1)
  {
   uint32 *out_ptr = (uint32*)ptr;
   uint32 *in_ptr = (uint32*)&DecodeBuffer[DecodeBufferWhichRead][RasterReadPos * 256 * 4];

   for(int x = 0; x < 256; x++)
   {
    if((Control & 0x2) || tmpss < 256)
     out_ptr[x] = in_ptr[tmpss & 0xFF];
    else
     out_ptr[x] = 0;
    tmpss = (tmpss + 1) & 0x1FF;
   }

   MDFN_FastU32MemsetM8((uint32*)&DecodeBuffer[DecodeBufferWhichRead][RasterReadPos * 256 * 4], 0x008080, 256);
  }
  else
  {
   uint8 *out_ptr = ptr;
   uint8 *in_ptr = &DecodeBuffer[DecodeBufferWhichRead][RasterReadPos * 256];

   for(int x = 0; x < 256; x++)
   {
    if((Control & 0x2) || tmpss < 256)
     out_ptr[x] = in_ptr[tmpss & 0xFF];
    else
     out_ptr[x] = 0;
    tmpss = (tmpss + 1) & 0x1FF;
   }
   memset(&DecodeBuffer[DecodeBufferWhichRead][RasterReadPos * 256], 0, 256);
  }
 }

 //printf("Fetch: %d, buffer: %d\n", RasterReadPos, DecodeBufferWhichRead);
 RasterReadPos = (RasterReadPos + 1) % 16;
 if(!RasterReadPos) DecodeBufferWhichRead ^= 1;

 return(ret);
}

void RAINBOW_Reset(void)
{
 Control = 0;
 NullRunY = NullRunU = NullRunV = 0;
 HScroll = 0;
 RasterReadPos = 0;
 DecodeBufferWhichRead = 0;

 memset(QuantTables, 0, sizeof(QuantTables));
 memset(QuantTablesBase, 0, sizeof(QuantTablesBase));
 DecodeFormat[0] = DecodeFormat[1] = 0;

 CalcHappyColor();
}


int RAINBOW_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
   SFVAR(HScroll),
   SFVAR(Control),
   SFVAR(RasterReadPos),
   SFVAR(DecodeBufferWhichRead),
   SFVAR(NullRunY),
   SFVAR(NullRunU),
   SFVAR(NullRunV),
   SFEND
 };

 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "RBOW");

 if(load)
 {
  RasterReadPos &= 0xF;
  CalcHappyColor();
 }
 return(ret);
}

