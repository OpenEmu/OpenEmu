#ifndef EMULATOR_HPP
#define EMULATOR_HPP

namespace Emulator {
  static const char Name[] = "bsnes";
  static const char Version[] = "089";
  static const char Author[] = "byuu";
  static const char License[] = "GPLv3";
}

#include <nall/platform.hpp>
#include <nall/algorithm.hpp>
#include <nall/dl.hpp>
#include <nall/dsp.hpp>
#include <nall/endian.hpp>
#include <nall/file.hpp>
#include <nall/function.hpp>
#include <nall/priorityqueue.hpp>
#include <nall/property.hpp>
#include <nall/random.hpp>
#include <nall/serializer.hpp>
#include <nall/sha256.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
#include <nall/utility.hpp>
#include <nall/varint.hpp>
#include <nall/vector.hpp>
#include <nall/stream/memory.hpp>
#include <nall/stream/vector.hpp>
using namespace nall;

#include "interface.hpp"

//debugging function hook:
//no overhead (and no debugger invocation) if not compiled with -DDEBUGGER
//wraps testing of function to allow invocation without a defined callback
template<typename T> struct hook;
template<typename R, typename... P> struct hook<R (P...)> {
  function<R (P...)> callback;

  R operator()(P... p) const {
    #if defined(DEBUGGER)
    if(callback) return callback(std::forward<P>(p)...);
    #endif
    return R();
  }

  hook() {}
  hook(const hook &hook) { callback = hook.callback; }
  hook(void *function) { callback = function; }
  hook(R (*function)(P...)) { callback = function; }
  template<typename C> hook(R (C::*function)(P...), C *object) { callback = {function, object}; }
  template<typename C> hook(R (C::*function)(P...) const, C *object) { callback = {function, object}; }
  template<typename L> hook(const L& function) { callback = function; }

  hook& operator=(const hook& hook) { callback = hook.callback; return *this; }
};

#if defined(DEBUGGER)
  #define privileged public
#else
  #define privileged private
#endif

typedef  int1_t  int1;
typedef  int2_t  int2;
typedef  int3_t  int3;
typedef  int4_t  int4;
typedef  int5_t  int5;
typedef  int6_t  int6;
typedef  int7_t  int7;
typedef  int8_t  int8;
typedef  int9_t  int9;
typedef int10_t int10;
typedef int11_t int11;
typedef int12_t int12;
typedef int13_t int13;
typedef int14_t int14;
typedef int15_t int15;
typedef int16_t int16;
typedef int17_t int17;
typedef int18_t int18;
typedef int19_t int19;
typedef int20_t int20;
typedef int21_t int21;
typedef int22_t int22;
typedef int23_t int23;
typedef int24_t int24;
typedef int25_t int25;
typedef int26_t int26;
typedef int27_t int27;
typedef int28_t int28;
typedef int29_t int29;
typedef int30_t int30;
typedef int31_t int31;
typedef int32_t int32;
typedef int64_t int64;

typedef  uint1_t  uint1;
typedef  uint2_t  uint2;
typedef  uint3_t  uint3;
typedef  uint4_t  uint4;
typedef  uint5_t  uint5;
typedef  uint6_t  uint6;
typedef  uint7_t  uint7;
typedef  uint8_t  uint8;
typedef  uint9_t  uint9;
typedef uint10_t uint10;
typedef uint11_t uint11;
typedef uint12_t uint12;
typedef uint13_t uint13;
typedef uint14_t uint14;
typedef uint15_t uint15;
typedef uint16_t uint16;
typedef uint17_t uint17;
typedef uint18_t uint18;
typedef uint19_t uint19;
typedef uint20_t uint20;
typedef uint21_t uint21;
typedef uint22_t uint22;
typedef uint23_t uint23;
typedef uint24_t uint24;
typedef uint25_t uint25;
typedef uint26_t uint26;
typedef uint27_t uint27;
typedef uint28_t uint28;
typedef uint29_t uint29;
typedef uint30_t uint30;
typedef uint31_t uint31;
typedef uint32_t uint32;
typedef uint_t<33> uint33;
typedef uint64_t uint64;

typedef varuint_t<unsigned> varuint;

#endif
