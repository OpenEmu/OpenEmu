#ifndef __MDFN_MATH_OPS_H
#define __MDFN_MATH_OPS_H

// Source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
// Rounds up to the nearest power of 2.
static INLINE uint64 round_up_pow2(uint64 v)
{
 v--;
 v |= v >> 1;
 v |= v >> 2;
 v |= v >> 4;
 v |= v >> 8;
 v |= v >> 16;
 v |= v >> 32;
 v++;

 v += (v == 0);

 return(v);
}

static INLINE uint32 uilog2(uint32 v)
{
 // http://graphics.stanford.edu/~seander/bithacks.html#IntegerLogDeBruijn

 static const uint32 MultiplyDeBruijnBitPosition[32] = 
 {
   0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30,
   8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
 };

 v |= v >> 1; // first round down to one less than a power of 2 
 v |= v >> 2;
 v |= v >> 4;
 v |= v >> 8;
 v |= v >> 16;

 return MultiplyDeBruijnBitPosition[(uint32_t)(v * 0x07C4ACDDU) >> 27];
}

// Some compilers' optimizers and some platforms might fubar the generated code from these macros,
// so some tests are run in...tests.cpp
#define sign_8_to_s16(_value) ((int16)(int8)(_value))
#define sign_9_to_s16(_value)  (((int16)((unsigned int)(_value) << 7)) >> 7)
#define sign_10_to_s16(_value)  (((int16)((uint32)(_value) << 6)) >> 6)
#define sign_11_to_s16(_value)  (((int16)((uint32)(_value) << 5)) >> 5)
#define sign_12_to_s16(_value)  (((int16)((uint32)(_value) << 4)) >> 4)
#define sign_13_to_s16(_value)  (((int16)((uint32)(_value) << 3)) >> 3)
#define sign_14_to_s16(_value)  (((int16)((uint32)(_value) << 2)) >> 2)
#define sign_15_to_s16(_value)  (((int16)((uint32)(_value) << 1)) >> 1)

// This obviously won't convert higher-than-32 bit numbers to signed 32-bit ;)
// Also, this shouldn't be used for 8-bit and 16-bit signed numbers, since you can
// convert those faster with typecasts...
#define sign_x_to_s32(_bits, _value) (((int32)((uint32)(_value) << (32 - _bits))) >> (32 - _bits))

#endif
