#ifndef SNES_HPP
#define SNES_HPP

namespace SNES {
  namespace Info {
    static const char Name[] = "bsnes";
    static const unsigned SerializerVersion = 23;
  }
}

/*
  bsnes - SNES emulator
  author: byuu
  license: GPLv3
  project started: 2004-10-14
*/

#include <libco/libco.h>

#include <nall/string.hpp>
#include <nall/platform.hpp>
#include <nall/algorithm.hpp>
#include <nall/array.hpp>
#include <nall/dl.hpp>
#include <nall/dsp.hpp>
#include <nall/endian.hpp>
#include <nall/file.hpp>
#include <nall/foreach.hpp>
#include <nall/function.hpp>
#include <nall/moduloarray.hpp>
#include <nall/priorityqueue.hpp>
#include <nall/property.hpp>
#include <nall/serializer.hpp>
#include <nall/stdint.hpp>
#include <nall/utility.hpp>
#include <nall/varint.hpp>
#include <nall/vector.hpp>
#include <nall/gameboy/cartridge.hpp>
using namespace nall;

#include <gameboy/gameboy.hpp>

#ifdef DEBUGGER
  #define debugvirtual virtual
#else
  #define debugvirtual
#endif

namespace SNES {
  typedef int8_t  int8;
  typedef int16_t int16;
  typedef int32_t int32;
  typedef int64_t int64;

  typedef int_t<24> int24;

  typedef uint8_t  uint8;
  typedef uint16_t uint16;
  typedef uint32_t uint32;
  typedef uint64_t uint64;

  typedef uint_t< 1> uint1;
  typedef uint_t< 2> uint2;
  typedef uint_t< 3> uint3;
  typedef uint_t< 4> uint4;
  typedef uint_t< 5> uint5;
  typedef uint_t< 6> uint6;
  typedef uint_t< 7> uint7;

  typedef uint_t< 9> uint9;
  typedef uint_t<10> uint10;
  typedef uint_t<11> uint11;
  typedef uint_t<12> uint12;
  typedef uint_t<13> uint13;
  typedef uint_t<14> uint14;
  typedef uint_t<15> uint15;

  typedef uint_t<17> uint17;
  typedef uint_t<18> uint18;
  typedef uint_t<19> uint19;
  typedef uint_t<20> uint20;
  typedef uint_t<21> uint21;
  typedef uint_t<22> uint22;
  typedef uint_t<23> uint23;
  typedef uint_t<24> uint24;
  typedef uint_t<25> uint25;
  typedef uint_t<26> uint26;
  typedef uint_t<27> uint27;
  typedef uint_t<28> uint28;
  typedef uint_t<29> uint29;
  typedef uint_t<30> uint30;
  typedef uint_t<31> uint31;

  typedef varuint_t varuint;

  template<uint8 banklo, uint8 bankhi, uint16 addrlo, uint16 addrhi>
  alwaysinline bool within(unsigned addr) {
    static const unsigned lo = (banklo << 16) | addrlo;
    static const unsigned hi = (bankhi << 16) | addrhi;
    static const unsigned mask = ~(hi ^ lo);
    return (addr & mask) == lo;
  }

  struct Processor {
    cothread_t thread;
    unsigned frequency;
    int64 clock;

    inline void create(void (*entrypoint)(), unsigned frequency) {
      if(thread) co_delete(thread);
      thread = co_create(65536 * sizeof(void*), entrypoint);
      this->frequency = frequency;
      clock = 0;
    }

    inline void serialize(serializer &s) {
      s.integer(frequency);
      s.integer(clock);
    }

    inline Processor() : thread(0) {}
    inline ~Processor() {
      if (thread) co_delete(thread);
    }
  };

  struct ChipDebugger {
    virtual bool property(unsigned id, string &name, string &value) = 0;
  };

  #include <snes/memory/memory.hpp>
  #include <snes/cpu/core/core.hpp>
  #include <snes/smp/core/core.hpp>
  #include <snes/ppu/counter/counter.hpp>

  #if defined(PROFILE_ACCURACY)
  #include "profile-accuracy.hpp"
  #elif defined(PROFILE_COMPATIBILITY)
  #include "profile-compatibility.hpp"
  #elif defined(PROFILE_PERFORMANCE)
  #include "profile-performance.hpp"
  #endif

  #include <snes/controller/controller.hpp>
  #include <snes/system/system.hpp>
  #include <snes/chip/chip.hpp>
  #include <snes/cartridge/cartridge.hpp>
  #include <snes/cheat/cheat.hpp>
  #include <snes/interface/interface.hpp>

  #include <snes/memory/memory-inline.hpp>
  #include <snes/ppu/counter/counter-inline.hpp>
}

#undef debugvirtual

#endif
