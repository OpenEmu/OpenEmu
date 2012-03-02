#ifndef GAMEBOY_HPP
#define GAMEBOY_HPP

namespace GameBoy {
  namespace Info {
    static const char Name[] = "bgameboy";
    static const unsigned SerializerVersion = 3;
  }
}

/*
  bgameboy - Game Boy, Super Game Boy, and Game Boy Color emulator
  author: byuu
  license: GPLv3
  project started: 2010-12-27
*/

#include <libco/libco.h>

#include <nall/platform.hpp>
#include <nall/property.hpp>
#include <nall/random.hpp>
#include <nall/serializer.hpp>
#include <nall/stdint.hpp>
#include <nall/string.hpp>
#include <nall/varint.hpp>
using namespace nall;

namespace GameBoy {
  typedef int8_t  int8;
  typedef int16_t int16;
  typedef int32_t int32;
  typedef int64_t int64;

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

  #include <gameboy/memory/memory.hpp>
  #include <gameboy/system/system.hpp>
  #include <gameboy/scheduler/scheduler.hpp>
  #include <gameboy/cartridge/cartridge.hpp>
  #include <gameboy/cpu/cpu.hpp>
  #include <gameboy/apu/apu.hpp>
  #include <gameboy/lcd/lcd.hpp>
  #include <gameboy/cheat/cheat.hpp>
  #include <gameboy/video/video.hpp>
};

#endif
