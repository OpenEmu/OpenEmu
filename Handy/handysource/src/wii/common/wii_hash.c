/*
Copyright (C) 2010
raz0red (www.twitchasylum.com)

This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1.	The origin of this software must not be misrepresented; you
must not claim that you wrote the original software. If you use
this software in a product, an acknowledgment in the product
documentation would be appreciated but is not required.

2.	Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3.	This notice may not be removed or altered from any source
distribution.
*/

#include <stdio.h>

#include "wii_hash.h"

/*
 * Puts an unsigned 32-bit value into the specified byte array
 *
 * data		The unsigned 32-bit value
 * addr		The array to place the bytes of the value into
 */
static void putu32( unsigned int data, unsigned char *addr )
{
  addr[0] = (unsigned char)data;
  addr[1] = (unsigned char)(data >> 8);
  addr[2] = (unsigned char)(data >> 16);
  addr[3] = (unsigned char)(data >> 24);
}

/*
 * Returns a 32-bit value from the specified byte array
 *
 * addr		The byte array
 * return	A 32-bit value from the specified byte array
 */
static unsigned int getu32( const unsigned char *addr )     
{
  return 
    (((((unsigned long)
    addr[3] << 8 ) | 
    addr[2] ) << 8 ) | 
    addr[1] ) << 8 | 
    addr[0];
}

// ----------------------------------------------------------------------------
// Step1
// ----------------------------------------------------------------------------
static uint hash_Step1( uint w, uint x, uint y, uint z, uint data, uint s ) 
{
  w += (z ^ (x & (y ^ z))) + data;
  w = w << s | w >> (32 - s);
  w += x;
  return w;
}

// ----------------------------------------------------------------------------
// Step2
// ----------------------------------------------------------------------------
static uint hash_Step2( uint w, uint x, uint y, uint z, uint data, uint s ) 
{
  w += (y ^ (z & (x ^ y))) + data;
  w = w << s | w >> (32 - s);
  w += x;
  return w;
}

// ----------------------------------------------------------------------------
// Step3
// ----------------------------------------------------------------------------
static uint hash_Step3( uint w, uint x, uint y, uint z, uint data, uint s ) 
{
  w += (x ^ y ^ z) + data;  
  w = w << s | w >> (32 - s);
  w += x;
  return w;
}

// ----------------------------------------------------------------------------
// Step4
// ----------------------------------------------------------------------------
static uint hash_Step4( uint w, uint x, uint y, uint z, uint data, uint s ) 
{
  w += (y ^ (x | ~z)) + data;  
  w = w << s | w >> (32 - s);
  w += x;
  return w;
}    

// ----------------------------------------------------------------------------
// Transform
// ----------------------------------------------------------------------------
static void hash_Transform( uint out[4], uint inraw[16] ) 
{
  uint a, b, c, d;

  unsigned int in[16];
  int i;
  for (i = 0; i < 16; ++i)
  {
    in[i] = getu32 ( (const unsigned char*)&(inraw[i]));
  }

  a = out[0];
  b = out[1];
  c = out[2];
  d = out[3];

  a = hash_Step1(a, b, c, d, in[0] + 0xd76aa478, 7);
  d = hash_Step1(d, a, b, c, in[1] + 0xe8c7b756, 12);
  c = hash_Step1(c, d, a, b, in[2] + 0x242070db, 17);
  b = hash_Step1(b, c, d, a, in[3] + 0xc1bdceee, 22);
  a = hash_Step1(a, b, c, d, in[4] + 0xf57c0faf, 7);
  d = hash_Step1(d, a, b, c, in[5] + 0x4787c62a, 12);
  c = hash_Step1(c, d, a, b, in[6] + 0xa8304613, 17);
  b = hash_Step1(b, c, d, a, in[7] + 0xfd469501, 22);
  a = hash_Step1(a, b, c, d, in[8] + 0x698098d8, 7);
  d = hash_Step1(d, a, b, c, in[9] + 0x8b44f7af, 12);
  c = hash_Step1(c, d, a, b, in[10] + 0xffff5bb1, 17);
  b = hash_Step1(b, c, d, a, in[11] + 0x895cd7be, 22);
  a = hash_Step1(a, b, c, d, in[12] + 0x6b901122, 7);
  d = hash_Step1(d, a, b, c, in[13] + 0xfd987193, 12);
  c = hash_Step1(c, d, a, b, in[14] + 0xa679438e, 17);
  b = hash_Step1(b, c, d, a, in[15] + 0x49b40821, 22);

  a = hash_Step2(a, b, c, d, in[1] + 0xf61e2562, 5);
  d = hash_Step2(d, a, b, c, in[6] + 0xc040b340, 9);
  c = hash_Step2(c, d, a, b, in[11] + 0x265e5a51, 14);
  b = hash_Step2(b, c, d, a, in[0] + 0xe9b6c7aa, 20);
  a = hash_Step2(a, b, c, d, in[5] + 0xd62f105d, 5);
  d = hash_Step2(d, a, b, c, in[10] + 0x02441453, 9);
  c = hash_Step2(c, d, a, b, in[15] + 0xd8a1e681, 14);
  b = hash_Step2(b, c, d, a, in[4] + 0xe7d3fbc8, 20);
  a = hash_Step2(a, b, c, d, in[9] + 0x21e1cde6, 5);
  d = hash_Step2(d, a, b, c, in[14] + 0xc33707d6, 9);
  c = hash_Step2(c, d, a, b, in[3] + 0xf4d50d87, 14);
  b = hash_Step2(b, c, d, a, in[8] + 0x455a14ed, 20);
  a = hash_Step2(a, b, c, d, in[13] + 0xa9e3e905, 5);
  d = hash_Step2(d, a, b, c, in[2] + 0xfcefa3f8, 9);
  c = hash_Step2(c, d, a, b, in[7] + 0x676f02d9, 14);
  b = hash_Step2(b, c, d, a, in[12] + 0x8d2a4c8a, 20);

  a = hash_Step3(a, b, c, d, in[5] + 0xfffa3942, 4);
  d = hash_Step3(d, a, b, c, in[8] + 0x8771f681, 11);
  c = hash_Step3(c, d, a, b, in[11] + 0x6d9d6122, 16);
  b = hash_Step3(b, c, d, a, in[14] + 0xfde5380c, 23);
  a = hash_Step3(a, b, c, d, in[1] + 0xa4beea44, 4);
  d = hash_Step3(d, a, b, c, in[4] + 0x4bdecfa9, 11);
  c = hash_Step3(c, d, a, b, in[7] + 0xf6bb4b60, 16);
  b = hash_Step3(b, c, d, a, in[10] + 0xbebfbc70, 23);
  a = hash_Step3(a, b, c, d, in[13] + 0x289b7ec6, 4);
  d = hash_Step3(d, a, b, c, in[0] + 0xeaa127fa, 11);
  c = hash_Step3(c, d, a, b, in[3] + 0xd4ef3085, 16);
  b = hash_Step3(b, c, d, a, in[6] + 0x04881d05, 23);
  a = hash_Step3(a, b, c, d, in[9] + 0xd9d4d039, 4);
  d = hash_Step3(d, a, b, c, in[12] + 0xe6db99e5, 11);
  c = hash_Step3(c, d, a, b, in[15] + 0x1fa27cf8, 16);
  b = hash_Step3(b, c, d, a, in[2] + 0xc4ac5665, 23);

  a = hash_Step4(a, b, c, d, in[0] + 0xf4292244, 6);
  d = hash_Step4(d, a, b, c, in[7] + 0x432aff97, 10);
  c = hash_Step4(c, d, a, b, in[14] + 0xab9423a7, 15);
  b = hash_Step4(b, c, d, a, in[5] + 0xfc93a039, 21);
  a = hash_Step4(a, b, c, d, in[12] + 0x655b59c3, 6);
  d = hash_Step4(d, a, b, c, in[3] + 0x8f0ccc92, 10);
  c = hash_Step4(c, d, a, b, in[10] + 0xffeff47d, 15);
  b = hash_Step4(b, c, d, a, in[1] + 0x85845dd1, 21);
  a = hash_Step4(a, b, c, d, in[8] + 0x6fa87e4f, 6);
  d = hash_Step4(d, a, b, c, in[15] + 0xfe2ce6e0, 10);
  c = hash_Step4(c, d, a, b, in[6] + 0xa3014314, 15);
  b = hash_Step4(b, c, d, a, in[13] + 0x4e0811a1, 21);
  a = hash_Step4(a, b, c, d, in[4] + 0xf7537e82, 6);
  d = hash_Step4(d, a, b, c, in[11] + 0xbd3af235, 10);
  c = hash_Step4(c, d, a, b, in[2] + 0x2ad7d2bb, 15);
  b = hash_Step4(b, c, d, a, in[9] + 0xeb86d391, 21);

  out[0] += a;
  out[1] += b;
  out[2] += c;
  out[3] += d;
}

/*
 * Computes the hash of the specified source
 *
 * source	The source to calculate the hash for
 * length	The length of the source
 * result	The string to receive the result of the hash computation
 */
void wii_hash_compute( const u8* source, u32 length, char result[33] ) 
{
  u8* ptr;
  u8 digest[16];
  u32 index, count;
  u32 buffer1[4] = {0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476};
  u32 buffer2[2] = {0};
  //byte __attribute__((aligned(64))) buffer3[64] = { 0 };
  u8 buffer3[64] = { 0 };

  u32 temp = buffer2[0];
  if((buffer2[0] = temp + ((uint)length << 3)) < temp) {
    buffer2[1]++;
  }
  buffer2[1] += length >> 29;

  temp = (temp >> 3) & 0x3f;
  if(temp) {
    u8* ptr = (u8*)buffer3 + temp;
    temp = 64 - temp;
    if(length < temp) {
      for(index = 0; index < length; index++) {
        ptr[index] = source[index];
      }
    }

    for(index = 0; index < temp; index++) {
      ptr[index] = source[index];
    }

    hash_Transform(buffer1, (uint*)buffer3);
    source += temp;
    length -= temp;
  }

  while(length >= 64) {
    for(index = 0; index < 64; index++) {
      buffer3[index] = source[index];
    }
    hash_Transform(buffer1, (uint*)buffer3);
    source += 64;
    length -= 64;
  }

  for(index = 0; index < length; index++) {
    buffer3[index] = source[index];
  }

  count = (buffer2[0] >> 3) & 0x3f;
  ptr = buffer3 + count;
  *ptr++ = 0x80;

  count = 63 - count;

  if(count < 8) {
    for(index = 0; index < count; index++) {
      ptr[index] = 0;
    }
    hash_Transform(buffer1, (uint*)buffer3);

    for(index = 0; index < 56; index++) {
      buffer3[index] = 0;
    }
  } 
  else {
    for(index = 0; index < count - 8; index++) {
      ptr[index] = 0;
    }
  }

  putu32( buffer2[0], (unsigned char*)&(((uint*)buffer3)[14]) );
  putu32( buffer2[1], (unsigned char*)&(((uint*)buffer3)[15]) );

  hash_Transform(buffer1, (uint*)buffer3);  

  putu32( buffer1[0], (unsigned char*)&(digest[0]) );
  putu32( buffer1[1], (unsigned char*)&(digest[4]) );
  putu32( buffer1[2], (unsigned char*)&(digest[8]) );
  putu32( buffer1[3], (unsigned char*)&(digest[12]) );

  sprintf(
    result, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
    digest[0], digest[1], digest[2], digest[3], digest[4], digest[5], 
    digest[6], digest[7], digest[8], digest[9], digest[10], digest[11], 
    digest[12], digest[13], digest[14], digest[15] );
}