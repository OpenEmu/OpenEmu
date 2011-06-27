#include <../base.hpp>

#define ADSP_CPP
namespace SNES {

aDSP dsp;

#include "adsp_tables.cpp"

void aDSP::enter() { loop:
  run();
  goto loop;
}

uint8 aDSP::readb(uint16 addr) {
  return spcram[addr];
}

void aDSP::writeb(uint16 addr, uint8 data) {
  spcram[addr] = data;
}

uint16 aDSP::readw(uint16 addr) {
  return (readb(addr + 0)) | (readb(addr + 1) << 8);
}

void aDSP::writew(uint16 addr, uint16 data) {
  writeb(addr + 0, data);
  writeb(addr + 1, data >> 8);
}

uint8 aDSP::read(uint8 addr) {
  addr &= 127;
int v = addr >> 4;
int n = addr & 15;

  switch(addr) {
  case 0x00: case 0x10: case 0x20: case 0x30:
  case 0x40: case 0x50: case 0x60: case 0x70:
    return voice[v].VOLL;
  case 0x01: case 0x11: case 0x21: case 0x31:
  case 0x41: case 0x51: case 0x61: case 0x71:
    return voice[v].VOLR;
  case 0x02: case 0x12: case 0x22: case 0x32:
  case 0x42: case 0x52: case 0x62: case 0x72:
    return voice[v].PITCH;
  case 0x03: case 0x13: case 0x23: case 0x33:
  case 0x43: case 0x53: case 0x63: case 0x73:
    return voice[v].PITCH >> 8;
  case 0x04: case 0x14: case 0x24: case 0x34:
  case 0x44: case 0x54: case 0x64: case 0x74:
    return voice[v].SRCN;
  case 0x05: case 0x15: case 0x25: case 0x35:
  case 0x45: case 0x55: case 0x65: case 0x75:
    return voice[v].ADSR1;
  case 0x06: case 0x16: case 0x26: case 0x36:
  case 0x46: case 0x56: case 0x66: case 0x76:
    return voice[v].ADSR2;
  case 0x07: case 0x17: case 0x27: case 0x37:
  case 0x47: case 0x57: case 0x67: case 0x77:
    return voice[v].GAIN;
  case 0x08: case 0x18: case 0x28: case 0x38:
  case 0x48: case 0x58: case 0x68: case 0x78:
    return voice[v].ENVX;
  case 0x09: case 0x19: case 0x29: case 0x39:
  case 0x49: case 0x59: case 0x69: case 0x79:
    return voice[v].OUTX;

  case 0x0f: case 0x1f: case 0x2f: case 0x3f:
  case 0x4f: case 0x5f: case 0x6f: case 0x7f:
    return status.FIR[v];

  case 0x0c: return status.MVOLL;
  case 0x1c: return status.MVOLR;
  case 0x2c: return status.EVOLL;
  case 0x3c: return status.EVOLR;
  case 0x4c: return status.KON;
  case 0x5c: return status.KOFF;
  case 0x6c: return status.FLG;
  case 0x7c: return status.ENDX;

  case 0x0d: return status.EFB;
  case 0x2d: return status.PMON;
  case 0x3d: return status.NON;
  case 0x4d: return status.EON;
  case 0x5d: return status.DIR;
  case 0x6d: return status.ESA;
  case 0x7d: return status.EDL;
  }

  return dspram[addr];
}

void aDSP::write(uint8 addr, uint8 data) {
//0x80-0xff is a read-only mirror of 0x00-0x7f
  if(addr & 0x80)return;

int v = addr >> 4;
int n = addr & 15;

  switch(addr) {
  case 0x00: case 0x10: case 0x20: case 0x30:
  case 0x40: case 0x50: case 0x60: case 0x70:
    voice[v].VOLL = data;
    break;
  case 0x01: case 0x11: case 0x21: case 0x31:
  case 0x41: case 0x51: case 0x61: case 0x71:
    voice[v].VOLR = data;
    break;
  case 0x02: case 0x12: case 0x22: case 0x32:
  case 0x42: case 0x52: case 0x62: case 0x72:
    voice[v].PITCH &= 0xff00;
    voice[v].PITCH |= data;
    break;
  case 0x03: case 0x13: case 0x23: case 0x33:
  case 0x43: case 0x53: case 0x63: case 0x73:
    voice[v].PITCH &= 0x00ff;
    voice[v].PITCH |= data << 8;
    break;
  case 0x04: case 0x14: case 0x24: case 0x34:
  case 0x44: case 0x54: case 0x64: case 0x74:
    voice[v].SRCN = data;
    break;
  case 0x05: case 0x15: case 0x25: case 0x35:
  case 0x45: case 0x55: case 0x65: case 0x75:
    voice[v].ADSR1 = data;
    voice[v].AdjustEnvelope();
    break;
  case 0x06: case 0x16: case 0x26: case 0x36:
  case 0x46: case 0x56: case 0x66: case 0x76:
    voice[v].ADSR2 = data;
  //sustain_level = 0-7, 7 is a special case handled by ATTACK envx mode
    voice[v].env_sustain = (voice[v].ADSR_sus_level() + 1) << 8;
    voice[v].AdjustEnvelope();
    break;
  case 0x07: case 0x17: case 0x27: case 0x37:
  case 0x47: case 0x57: case 0x67: case 0x77:
    voice[v].GAIN = data;
    voice[v].AdjustEnvelope();
    break;
  case 0x08: case 0x18: case 0x28: case 0x38:
  case 0x48: case 0x58: case 0x68: case 0x78:
    voice[v].ENVX = data;
    break;
  case 0x09: case 0x19: case 0x29: case 0x39:
  case 0x49: case 0x59: case 0x69: case 0x79:
    voice[v].OUTX = data;
    break;

  case 0x0f: case 0x1f: case 0x2f: case 0x3f:
  case 0x4f: case 0x5f: case 0x6f: case 0x7f:
    status.FIR[v] = data;
    break;

  case 0x0c: status.MVOLL = data; break;
  case 0x1c: status.MVOLR = data; break;
  case 0x2c: status.EVOLL = data; break;
  case 0x3c: status.EVOLR = data; break;

  case 0x4c:
    status.KON = data;
    status.kon = data;
    break;
  case 0x5c:
    status.KOFF = data;
    break;
  case 0x6c:
    status.FLG = data;
    status.noise_rate = rate_table[data & 0x1f];
    break;

  case 0x7c:
  //read-only register, writes clear all bits of ENDX
    status.ENDX = 0;
    break;

  case 0x0d: status.EFB  = data; break;
  case 0x2d: status.PMON = data; break;
  case 0x3d: status.NON  = data; break;
  case 0x4d: status.EON  = data; break;
  case 0x5d: status.DIR  = data; break;
  case 0x6d: status.ESA  = data; break;
  case 0x7d: status.EDL  = data; break;
  }

  dspram[addr] = data;
}

void aDSP::power() {
  spcram = r_smp->get_spcram_handle();
  memset(dspram, 0x00, 128);

  for(int v = 0; v < 8; v++) {
    voice[v].VOLL  = 0;
    voice[v].VOLR  = 0;
    voice[v].PITCH = 0;
    voice[v].SRCN  = 0;
    voice[v].ADSR1 = 0;
    voice[v].ADSR2 = 0;
    voice[v].GAIN  = 0;

    status.FIR[v]  = 0;
  }

  status.FLG   = 0xe0;
  status.MVOLL = status.MVOLR = 0;
  status.EVOLL = status.EVOLR = 0;
  status.ENDX  = 0;
  status.EFB   = 0;
  status.PMON  = 0;
  status.NON   = 0;
  status.EON   = 0;
  status.DIR   = 0;
  status.ESA   = 0;
  status.EDL   = 0;

  status.echo_length = 0;

  reset();
}

void aDSP::reset() {
  status.KON   = 0x00;
  status.KOFF  = 0x00;
  status.FLG  |= 0xe0;

  status.kon = 0x00;
  status.esa = 0x00;

  status.noise_ctr    = 0;
  status.noise_rate   = 0;
  status.noise_sample = 0x4000;

  status.echo_index       = 0;
  status.fir_buffer_index = 0;

  for(int v = 0; v < 8; v++) {
    voice[v].ENVX = 0;
    voice[v].OUTX = 0;

    voice[v].pitch_ctr = 0;

    voice[v].brr_index      = 0;
    voice[v].brr_ptr        = readw((status.DIR << 8) + (voice[v].SRCN << 2));
    voice[v].brr_looped     = false;
    voice[v].brr_data[0]    = 0;
    voice[v].brr_data[1]    = 0;
    voice[v].brr_data[2]    = 0;
    voice[v].brr_data[3]    = 0;
    voice[v].brr_data_index = 0;

    voice[v].envx      = 0;
    voice[v].env_ctr   = 0;
    voice[v].env_rate  = 0;
    voice[v].env_state = SILENCE;
    voice[v].env_mode  = DIRECT;

    status.fir_buffer[0][v] = 0;
    status.fir_buffer[1][v] = 0;
  }

  dsp_counter = 0;
}

void aDSP::run() {
uint8 pmon;
int32 sample;
int32 msamplel, msampler;
int32 esamplel, esampler;
int32 fir_samplel, fir_sampler;
  pmon = status.PMON & ~status.NON & ~1;

  if((dsp_counter++ & 1) == 0) {
    for(uint v = 0; v < 8; v++) {
      if(status.soft_reset()) {
        if(voice[v].env_state != SILENCE) {
          voice[v].env_state = SILENCE;
          voice[v].AdjustEnvelope();
        }
      }
      if(status.KOFF & (1 << v)) {
        if(voice[v].env_state != SILENCE && voice[v].env_state != RELEASE) {
          voice[v].env_state = RELEASE;
          voice[v].AdjustEnvelope();
        }
      }
      if(status.kon & (1 << v)) {
        voice[v].brr_ptr     = readw((status.DIR << 8) + (voice[v].SRCN << 2));
        voice[v].brr_index   = -9;
        voice[v].brr_looped  = false;
        voice[v].brr_data[0] = 0;
        voice[v].brr_data[1] = 0;
        voice[v].brr_data[2] = 0;
        voice[v].brr_data[3] = 0;
        voice[v].envx        = 0;
        voice[v].env_state   = ATTACK;
        voice[v].AdjustEnvelope();
      }
    }
    status.ENDX &= ~status.kon;
    status.kon = 0;
  }

/*****
 * update noise
 *****/
  status.noise_ctr += status.noise_rate;
  if(status.noise_ctr >= 0x7800) {
    status.noise_ctr   -= 0x7800;
    status.noise_sample = (status.noise_sample >> 1) | (((status.noise_sample << 14) ^ (status.noise_sample << 13)) & 0x4000);
  }

  msamplel = msampler = 0;
  esamplel = esampler = 0;

/*****
 * process voice channels
 *****/
  for(int v = 0; v < 8; v++) {
    if(voice[v].brr_index < -1) {
      voice[v].brr_index++;
      voice[v].OUTX = voice[v].outx = 0;
      voice[v].ENVX = 0;
      continue;
    }

    if(voice[v].brr_index >= 0) {
      if(pmon & (1 << v)) {
        voice[v].pitch_ctr += (voice[v].pitch_rate() * (voice[v - 1].outx + 0x8000)) >> 15;
      } else {
        voice[v].pitch_ctr += voice[v].pitch_rate();
      }
    } else {
      voice[v].pitch_ctr = 0x3000;
      voice[v].brr_index = 0;
    }

  /*****
   * decode BRR samples
   *****/
    while(voice[v].pitch_ctr >= 0) {
      voice[v].pitch_ctr -= 0x1000;

      voice[v].brr_data_index++;
      voice[v].brr_data_index &= 3;

      if(voice[v].brr_index == 0) {
        voice[v].brr_header = readb(voice[v].brr_ptr);

        if(voice[v].brr_header_flags() == BRR_END) {
          status.ENDX |= (1 << v);
          voice[v].env_state = SILENCE;
          voice[v].AdjustEnvelope();
        }
      }

#define S(x) voice[v].brr_data[(voice[v].brr_data_index + (x)) & 3]
      if(voice[v].env_state != SILENCE) {
        sample = readb(voice[v].brr_ptr + 1 + (voice[v].brr_index >> 1));
        if(voice[v].brr_index & 1) {
          sample = sclip<4>(sample);
        } else {
          sample = sclip<4>(sample >> 4);
        }

        if(voice[v].brr_header_shift() <= 12) {
          sample = (sample << voice[v].brr_header_shift() >> 1);
        } else {
          sample &= ~0x7ff;
        }

        switch(voice[v].brr_header_filter()) {
        case 0: //direct
          break;
        case 1: //15/16
          sample += S(-1) + ((-S(-1)) >> 4);
          break;
        case 2: //61/32 - 15/16
          sample += (S(-1) << 1) + ((-((S(-1) << 1) + S(-1))) >> 5)
                  - S(-2) + (S(-2) >> 4);
          break;
        case 3: //115/64 - 13/16
          sample += (S(-1) << 1) + ((-(S(-1) + (S(-1) << 2) + (S(-1) << 3))) >> 6)
                  - S(-2) + (((S(-2) << 1) + S(-2)) >> 4);
          break;
        }

        S(0) = sample = sclip<15>(sclamp<16>(sample));
      } else {
        S(0) = sample = 0;
      }

      if(++voice[v].brr_index > 15) {
        voice[v].brr_index = 0;
        if(voice[v].brr_header_flags() & BRR_END) {
          if(voice[v].brr_header_flags() & BRR_LOOP) {
            status.ENDX |= (1 << v);
          }
          voice[v].brr_ptr    = readw((status.DIR << 8) + (voice[v].SRCN << 2) + 2);
          voice[v].brr_looped = true;
        } else {
          voice[v].brr_ptr   += 9;
        }
      }
    }

  /*****
   * volume envelope adjust
   *****/
    voice[v].env_ctr += voice[v].env_rate;

    if(voice[v].env_ctr >= 0x7800) {
      voice[v].env_ctr -= 0x7800;
      switch(voice[v].env_mode) {
      case DIRECT:
        voice[v].env_rate = 0;
        break;
      case LINEAR_DEC:
        voice[v].envx -= 32;
        if(voice[v].envx <= 0) {
          voice[v].envx     = 0;
          voice[v].env_rate = 0;
          voice[v].env_mode = DIRECT;
        }
        break;
      case LINEAR_INC:
        voice[v].envx += 32;
        if(voice[v].envx >= 0x7ff) {
          voice[v].envx     = 0x7ff;
          voice[v].env_rate = 0;
          voice[v].env_mode = DIRECT;
          if(voice[v].ADSR_enabled() && voice[v].env_state == ATTACK) {
            voice[v].env_state = ((voice[v].env_sustain == 0x800) ? SUSTAIN : DECAY);
            voice[v].AdjustEnvelope();
          }
        }
        break;
      case EXP_DEC:
      //multiply by 255/256ths
        voice[v].envx -= ((voice[v].envx - 1) >> 8) + 1;
        if(voice[v].ADSR_enabled() && voice[v].env_state == DECAY && voice[v].envx <= voice[v].env_sustain) {
          voice[v].env_state = SUSTAIN;
          voice[v].AdjustEnvelope();
        } else if(voice[v].envx <= 0) {
          voice[v].envx     = 0;
          voice[v].env_rate = 0;
          voice[v].env_mode = DIRECT;
        }
        break;
      case BENT_INC:
        if(voice[v].envx < 0x600) {
          voice[v].envx += 32;
        } else {
          voice[v].envx += 8;

          if(voice[v].envx >= 0x7ff) {
            voice[v].envx     = 0x7ff;
            voice[v].env_rate = 0;
            voice[v].env_mode = DIRECT;
          }
        }
        break;
      case FAST_ATTACK:
        voice[v].envx += 0x400;
        if(voice[v].envx >= 0x7ff) {
          voice[v].envx = 0x7ff;

        //attack raises to max envx. if sustain is also set to max envx, skip decay phase
          voice[v].env_state = ((voice[v].env_sustain == 0x800) ? SUSTAIN : DECAY);
          voice[v].AdjustEnvelope();
        }
        break;
      case RELEASE_DEC:
        voice[v].envx -= 8;
        if(voice[v].envx <= 0) {
          voice[v].env_state = SILENCE;
          voice[v].AdjustEnvelope();
        }
        break;
      }
    }

    voice[v].ENVX = voice[v].envx >> 4;

  /*****
   * gaussian interpolation / noise
   *****/
    if(status.NON & (1 << v)) {
      sample  = sclip<15>(status.noise_sample);
    } else {
    int32 d = voice[v].pitch_ctr >> 4; //-256 <= sample <= -1
      sample  = ((gaussian_table[ -1 - d] * S(-3)) >> 11);
      sample += ((gaussian_table[255 - d] * S(-2)) >> 11);
      sample += ((gaussian_table[512 + d] * S(-1)) >> 11);
      sample  = sclip <15>(sample);
      sample += ((gaussian_table[256 + d] * S( 0)) >> 11);
      sample  = sclamp<15>(sample);
    }
#undef S

  /*****
   * envelope / volume adjust
   *****/
    sample = (sample * voice[v].envx) >> 11;
    voice[v].outx = sample << 1;
    voice[v].OUTX = sample >> 7;

    if(!status.mute()) {
      msamplel += ((sample * voice[v].VOLL) >> 7) << 1;
      msampler += ((sample * voice[v].VOLR) >> 7) << 1;
    }

    if((status.EON & (1 << v)) && status.echo_write()) {
      esamplel += ((sample * voice[v].VOLL) >> 7) << 1;
      esampler += ((sample * voice[v].VOLR) >> 7) << 1;
    }
  }

/*****
 * echo (FIR) adjust
 *****/
#define F(c,x) status.fir_buffer[c][(status.fir_buffer_index + (x)) & 7]
  status.fir_buffer_index++;
  F(0,0) = readw((status.esa << 8) + status.echo_index + 0);
  F(1,0) = readw((status.esa << 8) + status.echo_index + 2);

  fir_samplel = (F(0,-0) * status.FIR[7] +
                 F(0,-1) * status.FIR[6] +
                 F(0,-2) * status.FIR[5] +
                 F(0,-3) * status.FIR[4] +
                 F(0,-4) * status.FIR[3] +
                 F(0,-5) * status.FIR[2] +
                 F(0,-6) * status.FIR[1] +
                 F(0,-7) * status.FIR[0]);

  fir_sampler = (F(1,-0) * status.FIR[7] +
                 F(1,-1) * status.FIR[6] +
                 F(1,-2) * status.FIR[5] +
                 F(1,-3) * status.FIR[4] +
                 F(1,-4) * status.FIR[3] +
                 F(1,-5) * status.FIR[2] +
                 F(1,-6) * status.FIR[1] +
                 F(1,-7) * status.FIR[0]);
#undef F

/*****
 * update echo buffer
 *****/
  if(status.echo_write()) {
    esamplel += (fir_samplel * status.EFB) >> 14;
    esampler += (fir_sampler * status.EFB) >> 14;

    esamplel = sclamp<16>(esamplel);
    esampler = sclamp<16>(esampler);

    writew((status.esa << 8) + status.echo_index + 0, esamplel);
    writew((status.esa << 8) + status.echo_index + 2, esampler);
  }

  status.echo_index += 4;
  if(status.echo_index >= status.echo_length) {
    status.echo_index  = 0;
    status.echo_length = (status.EDL & 0x0f) << 11;
  }

//ESA read occurs at roughly 22/32th sample
//ESA fetch occurs at roughly 29/32th sample
//as this is not a subsample-level S-DSP emulator,
//simulate ~25/32th delay by caching ESA for one
//complete sample ...
  status.esa = status.ESA;

/*****
 * main output adjust
 *****/
  if(!status.mute()) {
    msamplel  = (msamplel * status.MVOLL) >> 7;
    msampler  = (msampler * status.MVOLR) >> 7;

    msamplel += (fir_samplel * status.EVOLL) >> 14;
    msampler += (fir_sampler * status.EVOLR) >> 14;

    msamplel  = sclamp<16>(msamplel);
    msampler  = sclamp<16>(msampler);
  }

  audio.sample(msamplel, msampler);
  scheduler.addclocks_dsp(32 * 3 * 8);
  scheduler.sync_dspsmp();
}

aDSP::aDSP()  {}
aDSP::~aDSP() {}

};
