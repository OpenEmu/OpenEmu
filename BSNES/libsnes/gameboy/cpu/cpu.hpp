struct CPU : Processor, MMIO {
  #include "core/core.hpp"
  #include "mmio/mmio.hpp"
  #include "timing/timing.hpp"

  bool trace;

  struct Interrupt {
    enum e {
      Vblank,
      Stat,
      Timer,
      Serial,
      Joypad,
    } i;
  };

  struct Status {
    unsigned clock;
    bool halt;
    bool stop;
    bool ei;
    bool ime;

    //$ff00  JOYP
    bool p15;
    bool p14;
    uint8 joyp;
    uint8 mlt_req;

    //$ff01  SB
    uint8 serial_data;
    unsigned serial_bits;

    //$ff02  SC
    bool serial_transfer;
    bool serial_clock;

    //$ff04  DIV
    uint8 div;

    //$ff05  TIMA
    uint8 tima;

    //$ff06  TMA
    uint8 tma;

    //$ff07  TAC
    bool timer_enable;
    unsigned timer_clock;

    //$ff0f  IF
    bool interrupt_request_joypad;
    bool interrupt_request_serial;
    bool interrupt_request_timer;
    bool interrupt_request_stat;
    bool interrupt_request_vblank;

    //$ff4d  KEY1
    bool speed_double;
    bool speed_switch;

    //$ff51,$ff52  HDMA1,HDMA2
    uint16 dma_source;

    //$ff53,$ff54  HDMA3,HDMA4
    uint16 dma_target;

    //$ff55  HDMA5
    bool dma_mode;
    uint16 dma_length;

    //$ff6c  ???
    uint8 ff6c;

    //$ff70  SVBK
    uint3 wram_bank;

    //$ff72-$ff75  ???
    uint8 ff72;
    uint8 ff73;
    uint8 ff74;
    uint8 ff75;

    //$ffff  IE
    bool interrupt_enable_joypad;
    bool interrupt_enable_serial;
    bool interrupt_enable_timer;
    bool interrupt_enable_stat;
    bool interrupt_enable_vblank;
  } status;

  uint8 wram[32768];  //GB=8192, GBC=32768
  uint8 hram[128];

  static void Main();
  void main();
  void interrupt_raise(Interrupt::e id);
  void interrupt_test();
  void interrupt_exec(uint16 pc);
  void power();

  void serialize(serializer&);
  CPU();
};

extern CPU cpu;
