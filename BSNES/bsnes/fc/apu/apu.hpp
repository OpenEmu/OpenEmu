struct APU : Thread {
  static void Main();
  void main();
  void tick();
  void set_irq_line();
  void set_sample(int16 sample);

  void power();
  void reset();

  uint8 read(uint16 addr);
  void write(uint16 addr, uint8 data);

  void serialize(serializer&);
  APU();

  struct Filter {
    enum : signed { HiPassStrong = 225574, HiPassWeak = 57593, LoPass = 86322413 };

    int64 hipass_strong;
    int64 hipass_weak;
    int64 lopass;

    signed run_hipass_strong(signed sample);
    signed run_hipass_weak(signed sample);
    signed run_lopass(signed sample);
    void serialize(serializer&);
  } filter;

  #include "envelope.hpp"
  #include "sweep.hpp"
  #include "pulse.hpp"
  #include "triangle.hpp"
  #include "noise.hpp"
  #include "dmc.hpp"

  struct FrameCounter {
    enum : unsigned { NtscPeriod = 14915 };  //~(21.477MHz / 6 / 240hz)

    bool irq_pending;

    uint2 mode;
    uint2 counter;
    signed divider;

    void serialize(serializer&);
  } frame;

  void clock_frame_counter();
  void clock_frame_counter_divider();

  uint8 enabled_channels;
  int16 cartridge_sample;

  int16 pulse_dac[32];
  int16 dmc_triangle_noise_dac[128][16][16];

  static const uint8 length_counter_table[32];
  static const uint16 ntsc_dmc_period_table[16];
  static const uint16 pal_dmc_period_table[16];
  static const uint16 ntsc_noise_period_table[16];
  static const uint16 pal_noise_period_table[16];
};

extern APU apu;
