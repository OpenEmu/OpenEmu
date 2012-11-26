// DO NOT REMOVE/DISABLE THESE MATH AND COMPILER SANITY TESTS.  THEY EXIST FOR A REASON.

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

// We really don't want NDEBUG defined ;)
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#undef NDEBUG

#include "mednafen.h"
#include "lepacker.h"

#undef NDEBUG
#include <assert.h>

#define FATALME	 { printf("Math test failed: %s:%d\n", __FILE__, __LINE__); fprintf(stderr, "Math test failed: %s:%d\n", __FILE__, __LINE__); return(0); }

// Don't define this static, and don't define it const.  We want these tests to be done at run time, not compile time(although maybe we should do both...).
typedef struct
{
 int bits;
 uint32 negative_one;
 uint32 mostneg;
 int32 mostnegresult;
} MathTestEntry;

#define ADD_MTE(_bits) { _bits, ((uint32)1 << _bits) - 1, (uint32)1 << (_bits - 1), (int32)(0 - ((uint32)1 << (_bits - 1))) }

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

static bool DoSizeofTests(void)
{
 const int SizePairs[][2] =
 {
  { sizeof(uint8), 1 },
  { sizeof(int8), 1 },

  { sizeof(uint16), 2 },
  { sizeof(int16), 2 },

  { sizeof(uint32), 4 },
  { sizeof(int32), 4 },

  { sizeof(uint64), 8 },
  { sizeof(int64), 8 },

  { 0, 0 },
 };

 int i = -1;

 while(SizePairs[++i][0])
 {
  if(SizePairs[i][0] != SizePairs[i][1])
   FATALME;
 }

 return(1);
}

static void AntiNSOBugTest_Sub1_a(int *array) NO_INLINE;
static void AntiNSOBugTest_Sub1_a(int *array)
{
 for(int value = 0; value < 127; value++)
  array[value] += (int8)value * 15;
}

static void AntiNSOBugTest_Sub1_b(int *array) NO_INLINE;
static void AntiNSOBugTest_Sub1_b(int *array)
{
 for(int value = 127; value < 256; value++)
  array[value] += (int8)value * 15;
}

static void AntiNSOBugTest_Sub2(int *array) NO_INLINE;
static void AntiNSOBugTest_Sub2(int *array)
{
 for(int value = 0; value < 256; value++)
  array[value] += (int8)value * 15;
}

static void AntiNSOBugTest_Sub3(int *array) NO_INLINE;
static void AntiNSOBugTest_Sub3(int *array)
{
 for(int value = 0; value < 256; value++)
 {
  if(value >= 128)
   array[value] = (value - 256) * 15;
  else
   array[value] = value * 15;
 }
}

static bool DoAntiNSOBugTest(void)
{
 int array1[256], array2[256], array3[256];
 
 memset(array1, 0, sizeof(array1));
 memset(array2, 0, sizeof(array2));
 memset(array3, 0, sizeof(array3));

 AntiNSOBugTest_Sub1_a(array1);
 AntiNSOBugTest_Sub1_b(array1);
 AntiNSOBugTest_Sub2(array2);
 AntiNSOBugTest_Sub3(array3);

 for(int i = 0; i < 256; i++)
 {
  if((array1[i] != array2[i]) || (array2[i] != array3[i]))
  {
   printf("%d %d %d %d\n", i, array1[i], array2[i], array3[i]);
   FATALME;
  }
 }
 //for(int value = 0; value < 256; value++)
 // printf("%d, %d\n", (int8)value, ((int8)value) * 15);

 return(1);
}

bool DoLEPackerTest(void)
{
 MDFN::LEPacker mizer;
 static const uint8 correct_result[24] = { 0xed, 0xfe, 0xed, 0xde, 0xaa, 0xca, 0xef, 0xbe, 0xbe, 0xba, 0xfe, 0xca, 0xad, 0xde, 0x01, 0x9a, 0x0c, 0xa7, 0xff, 0x00, 0xff, 0xff, 0x55, 0x7f };

 uint64 u64_test = 0xDEADCAFEBABEBEEFULL;
 uint32 u32_test = 0xDEEDFEED;
 uint16 u16_test = 0xCAAA;
 uint8 u8_test = 0x55;
 int32 s32_test = -5829478;
 int16 s16_test = -1;
 int8 s8_test = 127;

 bool bool_test0 = 0;
 bool bool_test1 = 1;

 mizer ^ u32_test;
 mizer ^ u16_test;
 mizer ^ u64_test;
 mizer ^ bool_test1;
 mizer ^ s32_test;
 mizer ^ bool_test0;
 mizer ^ s16_test;
 mizer ^ u8_test;
 mizer ^ s8_test;

 if(mizer.size() != 24)
 {
  printf("Test failed: LEPacker data incorrect size.\n");
  return(FALSE);
 }

 for(unsigned int i = 0; i < mizer.size(); i++)
  if(mizer[i] != correct_result[i])
  {
   printf("Test failed: LEPacker packed data incorrect.\n");
   return(FALSE);
  }

 u64_test = 0;
 u32_test = 0;
 u16_test = 0;
 u8_test = 0;
 s32_test = 0;
 s16_test = 0;
 s8_test = 0;

 bool_test0 = 1;
 bool_test1 = 0;

 mizer.set_read_mode(TRUE);

 mizer ^ u32_test;
 mizer ^ u16_test;
 mizer ^ u64_test;
 mizer ^ bool_test1;
 mizer ^ s32_test;
 mizer ^ bool_test0;
 mizer ^ s16_test;
 mizer ^ u8_test;
 mizer ^ s8_test;


 if(u32_test != 0xDEEDFEED)
 {
  printf("Test failed: LEPacker u32 unpacking incorrect.\n");
  return(FALSE);
 }

 if(u16_test != 0xCAAA)
 {
  printf("Test failed: LEPacker u16 unpacking incorrect.\n");
  return(FALSE);
 }

 if(u64_test != 0xDEADCAFEBABEBEEFULL)
 {
  printf("%16llx\n", (unsigned long long)u64_test);
  printf("Test failed: LEPacker u64 unpacking incorrect.\n");
  return(FALSE);
 }

 if(u8_test != 0x55)
 {
  printf("Test failed: LEPacker u8 unpacking incorrect.\n");
  return(FALSE);
 }

 if(s32_test != -5829478)
 {
  printf("Test failed: LEPacker s32 unpacking incorrect.\n");
  return(FALSE);
 }

 if(s16_test != -1)
 {
  printf("Test failed: LEPacker s16 unpacking incorrect.\n");
  return(FALSE);
 }

 if(s8_test != 127)
 {
  printf("Test failed: LEPacker s8 unpacking incorrect.\n");
  return(FALSE);
 }

 if(bool_test0 != 0)
 {
  printf("Test failed: LEPacker bool unpacking incorrect.\n");
  return(FALSE);
 }

 if(bool_test1 != 1)
 {
  printf("Test failed: LEPacker bool unpacking incorrect.\n");
  return(FALSE);
 }

 return(TRUE);
}

struct MathTestTSOEntry
{
 int32 a;
 int32 b;
};

// Don't declare as static(though whopr might mess it up anyway)
MathTestTSOEntry MathTestTSOTests[] =
{
 { 0x7FFFFFFF, 2 },
 { 0x7FFFFFFE, 0x7FFFFFFF },
 { 0x7FFFFFFF, 0x7FFFFFFF },
 { 0x7FFFFFFE, 0x7FFFFFFE },
};

static void TestSignedOverflow(void)
{
 for(unsigned int i = 0; i < sizeof(MathTestTSOTests) / sizeof(MathTestTSOEntry); i++)
 {
  int32 a = MathTestTSOTests[i].a;
  int32 b = MathTestTSOTests[i].b;

  assert((a + b) < a && (a + b) < b);

  assert((a + 0x7FFFFFFE) < a);
  assert((b + 0x7FFFFFFE) < b);

  assert((a + 0x7FFFFFFF) < a);
  assert((b + 0x7FFFFFFF) < b);

  assert((int32)(a + 0x80000000) < a);
  assert((int32)(b + 0x80000000) < b);

  assert((int32)(a ^ 0x80000000) < a);
  assert((int32)(b ^ 0x80000000) < b);
 }
}


static void DoAlignmentChecks(void)
{
 uint8 padding0[3];
 MDFN_ALIGN(16) uint8 aligned0[7];
 MDFN_ALIGN(4) uint8 aligned1[2];
 MDFN_ALIGN(16) uint32 aligned2[2];
 uint8 padding1[3];

 static uint8 g_padding0[3];
 static MDFN_ALIGN(16) uint8 g_aligned0[7];
 static MDFN_ALIGN(4) uint8 g_aligned1[2];
 static MDFN_ALIGN(16) uint32 g_aligned2[2];
 static uint8 g_padding1[3];

 // Make sure compiler doesn't removing padding vars
 assert((&padding0[1] - &padding0[0]) == 1);
 assert((&padding1[1] - &padding1[0]) == 1);
 assert((&g_padding0[1] - &g_padding0[0]) == 1);
 assert((&g_padding1[1] - &g_padding1[0]) == 1);


 assert( (((unsigned long long)&aligned0[0]) & 0xF) == 0);
 assert( (((unsigned long long)&aligned1[0]) & 0x3) == 0);
 assert( (((unsigned long long)&aligned2[0]) & 0xF) == 0);

 assert(((uint8 *)&aligned0[1] - (uint8 *)&aligned0[0]) == 1);
 assert(((uint8 *)&aligned1[1] - (uint8 *)&aligned1[0]) == 1);
 assert(((uint8 *)&aligned2[1] - (uint8 *)&aligned2[0]) == 4);


 assert( (((unsigned long long)&g_aligned0[0]) & 0xF) == 0);
 assert( (((unsigned long long)&g_aligned1[0]) & 0x3) == 0);
 assert( (((unsigned long long)&g_aligned2[0]) & 0xF) == 0);

 assert(((uint8 *)&g_aligned0[1] - (uint8 *)&g_aligned0[0]) == 1);
 assert(((uint8 *)&g_aligned1[1] - (uint8 *)&g_aligned1[0]) == 1);
 assert(((uint8 *)&g_aligned2[1] - (uint8 *)&g_aligned2[0]) == 4);
}

#include "masmem.h"

static void QuickEndianRBOTest(void)
{
 uint32 test[2] = { 0xDEADBEEF, 0xCAFEBABE };
 uint32 test2 = { 0xD00FD00F };

 assert(LoadU32_RBO(&test[0]) == 0xEFBEADDE);
 StoreU32_RBO(&test[1], 0x12341235);
 assert(test[1] == 0x35123412);
 assert(LoadU32_RBO(&test[1]) == 0x12341235);
 assert(LoadU32_RBO(&test2) == 0x0FD00FD0);
}

// don't make this static, and don't make it local scope.  Whole-program optimization might defeat the purpose of this, though...
unsigned int mdfn_shifty_test[4] =
{
 0, 8, 16, 32
};


#include "general.h"

bool MDFN_RunMathTests(void)
{
 MathTestEntry *itoo = math_test_vals;

 if(!DoSizeofTests())
  return(0);

 // Make sure the "char" type is signed(pass -fsigned-char to gcc).  New code in Mednafen shouldn't be written with the
 // assumption that "char" is signed, but there likely is at least some code that does.
 {
  char tmp = 255;
  assert(tmp < 0);
 }

 #if 0
 // TODO(except for 32-bit >> 32 test)
 {
  uint8 test_cow8 = (uint8)0xFF >> mdfn_shifty_test[1];
  uint16 test_cow16 = (uint16)0xFFFF >> mdfn_shifty_test[2];
  uint32 test_cow32 = (uint32)0xFFFFFFFF >> mdfn_shifty_test[3];
  uint32 test_cow32_2 = (uint32)0xFFFFFFFF >> mdfn_shifty_test[0];

 printf("%08x\n", test_cow32);

  assert(test_cow8 == 0);
  assert(test_cow16 == 0);
  assert(test_cow32 == 0);
  assert(test_cow32_2 == 0xFFFFFFFF);
 }
 #endif

 {
  int32 meow;

  meow = 1;
  meow >>= 1;
  assert(meow == 0);

  meow = 5;
  meow >>= 1;
  assert(meow == 2);

  meow = -1;
  meow >>= 1;
  assert(meow == -1);

  meow = -5;
  meow >>= 1;
  assert(meow == -3);

  meow = 1;
  meow /= 2;
  assert(meow == 0);

  meow = 5;
  meow /= 2;
  assert(meow == 2);

  meow = -1;
  meow /= 2;
  assert(meow == 0);

  meow = -5;
  meow /= 2;
  assert(meow == -2);

  meow = -5;
  meow = (int32)(meow + ((uint32)meow >> 31)) >> 1;
  assert(meow == -2);

  #if 0
  meow = 1 << 30;
  meow <<= 1;
  assert(meow == -2147483648);

  meow = 1 << 31;
  meow <<= 1;
  assert(meow == 0);
  #endif
 }


 // New tests added May 22, 2010 to detect MSVC compiler(and possibly other compilers) bad code generation.
 {
  uint32 test_tab[4] = { 0x2000 | 0x1000, 0x2000, 0x1000, 0x0000 };
  const uint32 result_tab[4][2] = { { 0xE, 0x7 }, { 0xE, 0x0 }, { 0x0, 0x7 }, { 0x0, 0x0 } };

  for(int i = 0; i < 4; i++)
  {
   uint32 hflip_xor;
   uint32 vflip_xor;
   uint32 bgsc;

   bgsc = test_tab[i];

   hflip_xor = ((int32)(bgsc << 18) >> 30) & 0xE;
   vflip_xor = ((int32)(bgsc << 19) >> 31) & 0x7;

   assert(hflip_xor == result_tab[i][0]);
   assert(vflip_xor == result_tab[i][1]);

   //printf("%d %d\n", hflip_xor, result_tab[i][0]);
   //printf("%d %d\n", vflip_xor, result_tab[i][1]);
  }

  uint32 lfsr = 1;

  // quick and dirty RNG(to also test non-constant-expression evaluation, at least until compilers are extremely advanced :b)
  for(int i = 0; i < 256; i++)
  {
   int feedback = ((lfsr >> 7) & 1) ^ ((lfsr >> 14) & 1);
   lfsr = ((lfsr << 1) & 0x7FFF) | feedback;
	
   uint32 hflip_xor;
   uint32 vflip_xor;
   uint32 hflip_xor_alt;
   uint32 vflip_xor_alt;
   uint32 bgsc;

   bgsc = lfsr;

   hflip_xor = ((int32)(bgsc << 18) >> 30) & 0xE;
   vflip_xor = ((int32)(bgsc << 19) >> 31) & 0x7;

   hflip_xor_alt = bgsc & 0x2000 ? 0xE : 0;
   vflip_xor_alt = bgsc & 0x1000 ? 0x7 : 0;

   assert(hflip_xor == hflip_xor_alt);
   assert(vflip_xor == vflip_xor_alt);
  }

 }

 DoAlignmentChecks();
 TestSignedOverflow();

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

 if(sizeof(int8) != 1 || sizeof(uint8) != 1)
  FATALME;


 if(!DoAntiNSOBugTest())
  return(0);

 if(!DoLEPackerTest())
  return(0);

 assert(uilog2(0) == 0);
 assert(uilog2(1) == 0);
 assert(uilog2(3) == 1);
 assert(uilog2(4095) == 11);
 assert(uilog2(0xFFFFFFFF) == 31);

 QuickEndianRBOTest();

#if 0
// Not really a math test.
 const char *test_paths[] = { "/meow", "/meow/cow", "\\meow", "\\meow\\cow", "\\\\meow", "\\\\meow\\cow",
			      "/meow.", "/me.ow/cow.", "\\meow.", "\\me.ow\\cow.", "\\\\meow.", "\\\\meow\\cow.",
			      "/meow.txt", "/me.ow/cow.txt", "\\meow.txt", "\\me.ow\\cow.txt", "\\\\meow.txt", "\\\\meow\\cow.txt"

			      "/meow", "/meow\\cow", "\\meow", "\\meow/cow", "\\\\meow", "\\\\meow/cow",
			      "/meow.", "\\me.ow/cow.", "\\meow.", "/me.ow\\cow.", "\\\\meow.", "\\\\meow/cow.",
			      "/meow.txt", "/me.ow\\cow.txt", "\\meow.txt", "\\me.ow/cow.txt", "\\\\meow.txt", "\\\\meow/cow.txt",
			      "/bark///dog", "\\bark\\\\\\dog" };

 for(unsigned i = 0; i < sizeof(test_paths) / sizeof(const char *); i++)
 {
  std::string file_path = std::string(test_paths[i]);
  std::string dir_path;
  std::string file_base;
  std::string file_ext;

  MDFN_GetFilePathComponents(file_path, &dir_path, &file_base, &file_ext);

  printf("%s ------ dir=%s --- base=%s --- ext=%s\n", file_path.c_str(), dir_path.c_str(), file_base.c_str(), file_ext.c_str());

 }
#endif


 return(1);
}
