#include <../base.hpp>

//B-bus interface

//$21f0  command port (r/w)
//-------------------------
//$00  set data port address (sr[3-0] = address)
//$01  set audio track number (sr[1-0] = track number)
//$02  set volume (sr[1] = left, sr[0] = right)
//$03  set audio state (sr[0].d1 = pause, sr[0].d0 = repeat)
//
//d7 = data port busy
//d6 = audio port busy
//d5 = audio playing
//d4 = reserved (0)
//d3-d0 = version (0)
//
//
//$21f1  parameter port (w)
//-------------------------
//(shift register)
//
//
//$21f2  data port (r)
//--------------------
//(auto-increment read port)

//A-bus interface

//$2200  command port (r/w)
//-------------------------
//$00  set data port address (sr[3-0] = address)
//$01  set audio track number (sr[1-0] = track number)
//$02  set volume (sr[1] = left, sr[0] = right)
//$03  set audio state (sr[0].d1 = pause, sr[0].d0 = repeat)
//
//d7 = data port busy
//d6 = audio port busy
//d5 = audio playing
//d4 = reserved (0)
//d3-d0 = version (0)
//
//$2201  data port (r/w)
//----------------------
//(shift register)
//
//(auto-increment read port)

#define S21FX_CPP
namespace SNES {

S21fx s21fx;

#include "serialization.cpp"

void S21fx::enter() {
  scheduler.clock.cop_freq = 44100;

  while(true) {
    if(scheduler.sync == Scheduler::SyncAll) {
      scheduler.exit(Scheduler::SynchronizeEvent);
    }

    int16 left = 0, right = 0;

    if((mmio.status & AudioPlaying) && !mmio.audio_pause) {
      if(audiofile.open()) {
        if(audiofile.end()) {
          if(!mmio.audio_repeat) mmio.status &= ~AudioPlaying;
          audiofile.seek(mmio.audio_offset = 58);
        } else {
          mmio.audio_offset += 4;
          left = audiofile.readl(2);
          right = audiofile.readl(2);
        }
      } else {
        mmio.status &= ~AudioPlaying;
      }
    }

    left  = sclamp<16>((double)left  * (double)mmio.audio_volume_left  / 255.0);
    right = sclamp<16>((double)right * (double)mmio.audio_volume_right / 255.0);

    audio.coprocessor_sample(left, right);
    scheduler.addclocks_cop(1);
    scheduler.sync_copcpu();
  }
}

void S21fx::init() {
}

void S21fx::enable() {
  audio.coprocessor_enable(true);
  audio.coprocessor_frequency(44100.0);

  for(unsigned i = 0x21f0; i <= 0x21f7; i++) {
    memory::mmio.map(i, *this);
  }

  memory::mmio.map(0x2200, *this);
  memory::mmio.map(0x2201, *this);

  if(datafile.open()) datafile.close();
  datafile.open(string() << basepath << "21fx.bin", file::mode_read);
}

void S21fx::power() {
  reset();
}

void S21fx::reset() {
  mmio.status = DataPortBusy | AudioBusy;
  mmio.shift_register = 0;

  mmio.data_offset  = 0;
  mmio.audio_offset = 0;
  mmio.audio_track  = 0;
  mmio.audio_volume_left  = 255;
  mmio.audio_volume_right = 255;
  mmio.audio_repeat = false;
  mmio.audio_pause  = false;
}

uint8 S21fx::mmio_read(unsigned addr) {
  addr &= 0xffff;

  if((addr == 0x21f0) || (addr == 0x2200)) {
    return mmio.status | 0x00;
  }

  if((addr == 0x21f2) || (addr == 0x2201)) {
    if(mmio.status & DataPortBusy) return 0x00;
    mmio.data_offset++;
    if(datafile.open()) return datafile.read();
    return 0x00;
  }

  return 0x00;
}

void S21fx::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;

  if((addr == 0x21f0) || (addr == 0x2200)) {
    if(data == 0x00) {
      mmio.data_offset = mmio.shift_register & 0xffffffff;
      if(datafile.open()) {
        datafile.seek(mmio.data_offset);
      }
      mmio.status &= ~DataPortBusy;
    }

    if(data == 0x01) {
      mmio.audio_track = mmio.shift_register & 0xffff;
      if(audiofile.open()) audiofile.close();
      char track[16];
      sprintf(track, "%.5u", mmio.audio_track);
      if(audiofile.open(string() << basepath << "audio" << track << ".wav", file::mode_read)) {
        audiofile.seek(mmio.audio_offset = 58);  //skip WAV header
      }
      mmio.status &= ~(AudioBusy | AudioPlaying);
    }

    if(data == 0x02) {
      mmio.audio_volume_left  = mmio.shift_register >> 8;
      mmio.audio_volume_right = mmio.shift_register >> 0;
    }

    if(data == 0x03) {
      mmio.status |= AudioPlaying;
      mmio.audio_repeat = mmio.shift_register & 1;
      mmio.audio_pause  = mmio.shift_register & 2;
    }

    mmio.shift_register = 0;
  }

  if((addr == 0x21f1) || (addr == 0x2201)) {
    mmio.shift_register = (mmio.shift_register << 8) | data;
  }
}

void S21fx::base(const string& path) {
  basepath = path;
}

bool S21fx::exists() {
  return file::exists(string() << basepath << "21fx.bin");
}

S21fx::S21fx() {
}

}
