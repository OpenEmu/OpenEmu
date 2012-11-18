#ifndef NALL_ENDIAN_HPP
#define NALL_ENDIAN_HPP

#include <nall/intrinsics.hpp>

#if defined(ENDIAN_LSB)
  //little-endian: uint8_t[] { 0x01, 0x02, 0x03, 0x04 } == 0x04030201
  #define order_lsb2(a,b)             a,b
  #define order_lsb3(a,b,c)           a,b,c
  #define order_lsb4(a,b,c,d)         a,b,c,d
  #define order_lsb5(a,b,c,d,e)       a,b,c,d,e
  #define order_lsb6(a,b,c,d,e,f)     a,b,c,d,e,f
  #define order_lsb7(a,b,c,d,e,f,g)   a,b,c,d,e,f,g
  #define order_lsb8(a,b,c,d,e,f,g,h) a,b,c,d,e,f,g,h
  #define order_msb2(a,b)             b,a
  #define order_msb3(a,b,c)           c,b,a
  #define order_msb4(a,b,c,d)         d,c,b,a
  #define order_msb5(a,b,c,d,e)       e,d,c,b,a
  #define order_msb6(a,b,c,d,e,f)     f,e,d,c,b,a
  #define order_msb7(a,b,c,d,e,f,g)   g,f,e,d,c,b,a
  #define order_msb8(a,b,c,d,e,f,g,h) h,g,f,e,d,c,b,a
#elif defined(ENDIAN_MSB)
  //big-endian:    uint8_t[] { 0x01, 0x02, 0x03, 0x04 } == 0x01020304
  #define order_lsb2(a,b)             b,a
  #define order_lsb3(a,b,c)           c,b,a
  #define order_lsb4(a,b,c,d)         d,c,b,a
  #define order_lsb5(a,b,c,d,e)       e,d,c,b,a
  #define order_lsb6(a,b,c,d,e,f)     f,e,d,c,b,a
  #define order_lsb7(a,b,c,d,e,f,g)   g,f,e,d,c,b,a
  #define order_lsb8(a,b,c,d,e,f,g,h) h,g,f,e,d,c,b,a
  #define order_msb2(a,b)             a,b
  #define order_msb3(a,b,c)           a,b,c
  #define order_msb4(a,b,c,d)         a,b,c,d
  #define order_msb5(a,b,c,d,e)       a,b,c,d,e
  #define order_msb6(a,b,c,d,e,f)     a,b,c,d,e,f
  #define order_msb7(a,b,c,d,e,f,g)   a,b,c,d,e,f,g
  #define order_msb8(a,b,c,d,e,f,g,h) a,b,c,d,e,f,g,h
#else
  #error "Unknown endian. Please specify in nall/intrinsics.hpp"
#endif

#endif
