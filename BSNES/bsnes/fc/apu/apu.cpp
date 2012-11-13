#include <fc/fc.hpp>

namespace Famicom {

#include "envelope.cpp"
#include "sweep.cpp"
#include "pulse.cpp"
#include "triangle.cpp"
#include "noise.cpp"
#include "dmc.cpp"
#include "serialization.cpp"
APU apu;

const uint8 APU::length_counter_table[32] = {
  0x0a, 0xfe, 0x14, 0x02, 0x28, 0x04, 0x50, 0x06, 0xa0, 0x08, 0x3c, 0x0a, 0x0e, 0x0c, 0x1a, 0x0e,
  0x0c, 0x10, 0x18, 0x12, 0x30, 0x14, 0x60, 0x16, 0xc0, 0x18, 0x48, 0x1a, 0x10, 0x1c, 0x20, 0x1e,
};

const uint16 APU::ntsc_noise_period_table[16] = {
  4, 8, 16, 32, 64, 96, 128, 160, 202, 254, 380, 508, 762, 1016, 2034, 4068,
};

const uint16 APU::pal_noise_period_table[16] = {
  4, 7, 14, 30, 60, 88, 118, 148, 188, 236, 354, 472, 708,  944, 1890, 3778,
};

const uint16 APU::ntsc_dmc_period_table[16] = {
  428, 380, 340, 320, 286, 254, 226, 214, 190, 160, 142, 128, 106, 84, 72, 54,
};

const uint16 APU::pal_dmc_period_table[16] = {
  398, 354, 316, 298, 276, 236, 210, 198, 176, 148, 132, 118,  98, 78, 66, 50,
};

void APU::Main() {
  apu.main();
}

void APU::main() {
  while(true) {
    if(scheduler.sync == Scheduler::SynchronizeMode::All) {
      scheduler.exit(Scheduler::ExitReason::SynchronizeEvent);
    }

    unsigned pulse_output, triangle_output, noise_output, dmc_output;

    pulse_output  = pulse[0].clock();
    pulse_output += pulse[1].clock();
    triangle_output = triangle.clock();
    noise_output = noise.clock();
    dmc_output = dmc.clock();

    clock_frame_counter_divider();

    signed output = pulse_dac[pulse_output] + dmc_triangle_noise_dac[dmc_output][triangle_output][noise_output];

    output  = filter.run_hipass_strong(output);
    output += cartridge_sample;
    output  = filter.run_hipass_weak(output);
  //output  = filter.run_lopass(output);
    output  = sclamp<16>(output);

    interface->audioSample(output, output);

    tick();
  }
}

void APU::tick() {
  clock += 12;
  if(clock >= 0 && scheduler.sync != Scheduler::SynchronizeMode::All) co_switch(cpu.thread);
}

void APU::set_irq_line() {
  cpu.set_irq_apu_line(frame.irq_pending || dmc.irq_pending);
}

void APU::set_sample(int16 sample) {
  cartridge_sample = sample;
}

void APU::power() {
  filter.hipass_strong = 0;
  filter.hipass_weak = 0;
  filter.lopass = 0;

  pulse[0].power();
  pulse[1].power();
  triangle.power();
  noise.power();
  dmc.power();
}

void APU::reset() {
  create(APU::Main, 21477272);

  pulse[0].reset();
  pulse[1].reset();
  triangle.reset();
  noise.reset();
  dmc.reset();

  frame.irq_pending = 0;

  frame.mode = 0;
  frame.counter = 0;
  frame.divider = 1;

  enabled_channels = 0;
  cartridge_sample = 0;

  set_irq_line();
}

uint8 APU::read(uint16 addr) {
  if(addr == 0x4015) {
    uint8 result = 0x00;
    result |= pulse[0].length_counter ? 0x01 : 0;
    result |= pulse[1].length_counter ? 0x02 : 0;
    result |= triangle.length_counter ? 0x04 : 0;
    result |=    noise.length_counter ? 0x08 : 0;
    result |=      dmc.length_counter ? 0x10 : 0;
    result |=       frame.irq_pending ? 0x40 : 0;
    result |=         dmc.irq_pending ? 0x80 : 0;

    frame.irq_pending = false;
    set_irq_line();

    return result;
  }

  return cpu.mdr();
}

void APU::write(uint16 addr, uint8 data) {
  const unsigned n = (addr >> 2) & 1;  //pulse#

  switch(addr) {
  case 0x4000: case 0x4004:
    pulse[n].duty = data >> 6;
    pulse[n].envelope.loop_mode = data & 0x20;
    pulse[n].envelope.use_speed_as_volume = data & 0x10;
    pulse[n].envelope.speed = data & 0x0f;
    break;

  case 0x4001: case 0x4005:
    pulse[n].sweep.enable = data & 0x80;
    pulse[n].sweep.period = (data & 0x70) >> 4;
    pulse[n].sweep.decrement = data & 0x08;
    pulse[n].sweep.shift = data & 0x07;
    pulse[n].sweep.reload = true;
    break;

  case 0x4002: case 0x4006:
    pulse[n].period = (pulse[n].period & 0x0700) | (data << 0);
    pulse[n].sweep.pulse_period = (pulse[n].sweep.pulse_period & 0x0700) | (data << 0);
    break;

  case 0x4003: case 0x4007:
    pulse[n].period = (pulse[n].period & 0x00ff) | (data << 8);
    pulse[n].sweep.pulse_period = (pulse[n].sweep.pulse_period & 0x00ff) | (data << 8);

    pulse[n].duty_counter = 7;
    pulse[n].envelope.reload_decay = true;

    if(enabled_channels & (1 << n)) {
      pulse[n].length_counter = length_counter_table[(data >> 3) & 0x1f];
    }
    break;

  case 0x4008:
    triangle.halt_length_counter = data & 0x80;
    triangle.linear_length = data & 0x7f;
    break;

  case 0x400a:
    triangle.period = (triangle.period & 0x0700) | (data << 0);
    break;

  case 0x400b:
    triangle.period = (triangle.period & 0x00ff) | (data << 8);

    triangle.reload_linear = true;

    if(enabled_channels & (1 << 2)) {
      triangle.length_counter = length_counter_table[(data >> 3) & 0x1f];
    }
    break;

  case 0x400c:
    noise.envelope.loop_mode = data & 0x20;
    noise.envelope.use_speed_as_volume = data & 0x10;
    noise.envelope.speed = data & 0x0f;
    break;

  case 0x400e:
    noise.short_mode = data & 0x80;
    noise.period = data & 0x0f;
    break;

  case 0x400f:
    noise.envelope.reload_decay = true;

    if(enabled_channels & (1 << 3)) {
      noise.length_counter = length_counter_table[(data >> 3) & 0x1f];
    }
    break;

  case 0x4010:
    dmc.irq_enable = data & 0x80;
    dmc.loop_mode = data & 0x40;
    dmc.period = data & 0x0f;

    dmc.irq_pending = dmc.irq_pending && dmc.irq_enable && !dmc.loop_mode;
    set_irq_line();
    break;

  case 0x4011:
    dmc.dac_latch = data & 0x7f;
    break;

  case 0x4012:
    dmc.addr_latch = data;
    break;

  case 0x4013:
    dmc.length_latch = data;
    break;

  case 0x4015:
    if((data & 0x01) == 0) pulse[0].length_counter = 0;
    if((data & 0x02) == 0) pulse[1].length_counter = 0;
    if((data & 0x04) == 0) triangle.length_counter = 0;
    if((data & 0x08) == 0)    noise.length_counter = 0;

    (data & 0x10) ? dmc.start() : dmc.stop();
    dmc.irq_pending = false;

    set_irq_line();
    enabled_channels = data & 0x1f;
    break;

  case 0x4017:
    frame.mode = data >> 6;

    frame.counter = 0;
    if(frame.mode & 2) clock_frame_counter();
    if(frame.mode & 1) {
      frame.irq_pending = false;
      set_irq_line();
    }
    frame.divider = FrameCounter::NtscPeriod;
    break;
  }
}

signed APU::Filter::run_hipass_strong(signed sample) {
  hipass_strong += ((((int64)sample << 16) - (hipass_strong >> 16)) * HiPassStrong) >> 16;
  return sample - (hipass_strong >> 32);
}

signed APU::Filter::run_hipass_weak(signed sample) {
  hipass_weak += ((((int64)sample << 16) - (hipass_weak >> 16)) * HiPassWeak) >> 16;
  return sample - (hipass_weak >> 32);
}

signed APU::Filter::run_lopass(signed sample) {
  lopass += ((((int64)sample << 16) - (lopass >> 16)) * LoPass) >> 16;
  return (lopass >> 32);
}

void APU::clock_frame_counter() {
  frame.counter++;

  if(frame.counter & 1) {
    pulse[0].clock_length();
    pulse[0].sweep.clock(0);
    pulse[1].clock_length();
    pulse[1].sweep.clock(1);
    triangle.clock_length();
    noise.clock_length();
  }

  pulse[0].envelope.clock();
  pulse[1].envelope.clock();
  triangle.clock_linear_length();
  noise.envelope.clock();

  if(frame.counter == 0) {
    if(frame.mode & 2) frame.divider += FrameCounter::NtscPeriod;
    if(frame.mode == 0) {
      frame.irq_pending = true;
      set_irq_line();
    }
  }
}

void APU::clock_frame_counter_divider() {
  frame.divider -= 2;
  if(frame.divider <= 0) {
    clock_frame_counter();
    frame.divider += FrameCounter::NtscPeriod;
  }
}

APU::APU() {
  for(unsigned amp = 0; amp < 32; amp++) {
    if(amp == 0) {
      pulse_dac[amp] = 0;
    } else {
      pulse_dac[amp] = 16384.0 * 95.88 / (8128.0 / amp + 100.0);
    }
  }

  for(unsigned dmc_amp = 0; dmc_amp < 128; dmc_amp++) {
    for(unsigned triangle_amp = 0; triangle_amp < 16; triangle_amp++) {
      for(unsigned noise_amp = 0; noise_amp < 16; noise_amp++) {
        if(dmc_amp == 0 && triangle_amp == 0 && noise_amp == 0) {
          dmc_triangle_noise_dac[dmc_amp][triangle_amp][noise_amp] = 0;
        } else {
          dmc_triangle_noise_dac[dmc_amp][triangle_amp][noise_amp]
          = 16384.0 * 159.79 / (100.0 + 1.0 / (triangle_amp / 8227.0 + noise_amp / 12241.0 + dmc_amp / 22638.0));
        }
      }
    }
  }
}

}
