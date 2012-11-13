#ifndef FC_HPP
#define FC_HPP

#include <emulator/emulator.hpp>
#include <processor/r6502/r6502.hpp>

namespace Famicom {
  namespace Info {
    static const char Name[] = "bnes";
    static const unsigned SerializerVersion = 2;
  }
}

/*
  bnes - Famicom emulator
  authors: byuu, Ryphecha
  license: GPLv3
  project started: 2011-09-05
*/

#include <libco/libco.h>

namespace Famicom {
  struct Thread {
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

    inline Thread() : thread(nullptr) {
    }

    inline ~Thread() {
      if(thread) co_delete(thread);
    }
  };

  #include <fc/system/system.hpp>
  #include <fc/scheduler/scheduler.hpp>
  #include <fc/input/input.hpp>
  #include <fc/memory/memory.hpp>
  #include <fc/cartridge/cartridge.hpp>
  #include <fc/cpu/cpu.hpp>
  #include <fc/apu/apu.hpp>
  #include <fc/ppu/ppu.hpp>
  #include <fc/cheat/cheat.hpp>
  #include <fc/video/video.hpp>
  #include <fc/interface/interface.hpp>
}

#endif
