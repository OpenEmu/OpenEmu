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

#include "mednafen.h"

#define FATALME	 { printf("Math test failed: %s:%d\n", __FILE__, __LINE__); fprintf(stderr, "Math test failed: %s:%d\n", __FILE__, __LINE__); return(0); }

// Don't define this static, and don't define it const.  We want these tests to be done at run time, not compile time(although maybe we should do both...).
typedef struct
{
 int bits;
 uint32 negative_one;
 uint32 mostneg;
 int32 mostnegresult;
} MathTestEntry;

#define ADD_MTE(_bits) { _bits, ((uint32)1 << _bits) - 1, (uint32)1 << (_bits - 1), 0 - ((uint32)1 << (_bits - 1)) }

MathTestEntry math_test_vals[] =
{
 {  9, 0x01FF, 0x0100, -256 },
 { 10, 0x03FF, 0x0200, -512 },
 { 11, 0x07FF, 0x0400, -1024 },
 { 12, 0x0FFF, 0x0800, -2048 },
 { 13, 0x1FFF, 0x1000, -4096 },
 { 14, 0x3FFF, 0x2000, -8192 },
 { 15, 0x7FFF, 0x4000, -16384 },

 ADD_MTE(17),
 ADD_MTE(18),
 ADD_MTE(19),
 ADD_MTE(20),
 ADD_MTE(21),
 ADD_MTE(22),
 ADD_MTE(23),
 ADD_MTE(24),
 ADD_MTE(25),
 ADD_MTE(26),
 ADD_MTE(27),
 ADD_MTE(28),
 ADD_MTE(29),
 ADD_MTE(30),
 ADD_MTE(31),

 { 0, 0, 0, 0 },
};

bool MDFN_RunMathTests(void)
{
 MathTestEntry *itoo = math_test_vals;

 if(sign_9_to_s16(itoo->negative_one) != -1 || sign_9_to_s16(itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_10_to_s16(itoo->negative_one) != -1 || sign_10_to_s16(itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_11_to_s16(itoo->negative_one) != -1 || sign_11_to_s16(itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_12_to_s16(itoo->negative_one) != -1 || sign_12_to_s16(itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_13_to_s16(itoo->negative_one) != -1 || sign_13_to_s16(itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_14_to_s16(itoo->negative_one) != -1 || sign_14_to_s16(itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_15_to_s16(itoo->negative_one) != -1 || sign_15_to_s16(itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(17, itoo->negative_one) != -1 || sign_x_to_s32(17, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(18, itoo->negative_one) != -1 || sign_x_to_s32(18, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(19, itoo->negative_one) != -1 || sign_x_to_s32(19, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(20, itoo->negative_one) != -1 || sign_x_to_s32(20, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(21, itoo->negative_one) != -1 || sign_x_to_s32(21, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(22, itoo->negative_one) != -1 || sign_x_to_s32(22, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(23, itoo->negative_one) != -1 || sign_x_to_s32(23, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(24, itoo->negative_one) != -1 || sign_x_to_s32(24, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(25, itoo->negative_one) != -1 || sign_x_to_s32(25, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(26, itoo->negative_one) != -1 || sign_x_to_s32(26, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(27, itoo->negative_one) != -1 || sign_x_to_s32(27, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(28, itoo->negative_one) != -1 || sign_x_to_s32(28, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(29, itoo->negative_one) != -1 || sign_x_to_s32(29, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(30, itoo->negative_one) != -1 || sign_x_to_s32(30, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;

 if(sign_x_to_s32(31, itoo->negative_one) != -1 || sign_x_to_s32(31, itoo->mostneg) != itoo->mostnegresult)
  FATALME;
 itoo++;


 return(1);
}
