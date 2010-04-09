#include <../base.hpp>

#define DSP2_CPP
namespace SNES {

DSP2 dsp2;

#include "serialization.cpp"
#include "opcodes.cpp"

void DSP2::init() {
}

void DSP2::enable() {
  bus.map(Bus::MapDirect, 0x20, 0x3f, 0x6000, 0x6fff, *this);
  bus.map(Bus::MapDirect, 0x20, 0x3f, 0x8000, 0xbfff, *this);
  bus.map(Bus::MapDirect, 0xa0, 0xbf, 0x6000, 0x6fff, *this);
  bus.map(Bus::MapDirect, 0xa0, 0xbf, 0x8000, 0xbfff, *this);
}

void DSP2::power() {
  reset();
}

void DSP2::reset() {
  status.waiting_for_command = true;
  status.in_count  = 0;
  status.in_index  = 0;
  status.out_count = 0;
  status.out_index = 0;

  status.op05transparent = 0;
  status.op05haslen      = false;
  status.op05len         = 0;
  status.op06haslen      = false;
  status.op06len         = 0;
  status.op09word1       = 0;
  status.op09word2       = 0;
  status.op0dhaslen      = false;
  status.op0doutlen      = 0;
  status.op0dinlen       = 0;
}

uint8 DSP2::read(unsigned addr) {
  uint8 r = 0xff;
  if(status.out_count) {
    r = status.output[status.out_index++];
    status.out_index &= 511;
    if(status.out_count == status.out_index) {
      status.out_count = 0;
    }
  }
  return r;
}

void DSP2::write(unsigned addr, uint8 data) {
  if(status.waiting_for_command) {
    status.command  = data;
    status.in_index = 0;
    status.waiting_for_command = false;

    switch(data) {
    case 0x01: status.in_count = 32; break;
    case 0x03: status.in_count =  1; break;
    case 0x05: status.in_count =  1; break;
    case 0x06: status.in_count =  1; break;
    case 0x07: break;
    case 0x08: break;
    case 0x09: status.in_count =  4; break;
    case 0x0d: status.in_count =  2; break;
    case 0x0f: status.in_count =  0; break;
    }
  } else {
    status.parameters[status.in_index++] = data;
    status.in_index &= 511;
  }

  if(status.in_count == status.in_index) {
    status.waiting_for_command = true;
    status.out_index = 0;
    switch(status.command) {
    case 0x01: {
      status.out_count = 32;
      op01();
    } break;

    case 0x03: {
      op03();
    } break;

    case 0x05: {
      if(status.op05haslen) {
        status.op05haslen = false;
        status.out_count  = status.op05len;
        op05();
      } else {
        status.op05len    = status.parameters[0];
        status.in_index   = 0;
        status.in_count   = status.op05len * 2;
        status.op05haslen = true;
        if(data)status.waiting_for_command = false;
      }
    } break;

    case 0x06: {
      if(status.op06haslen) {
        status.op06haslen = false;
        status.out_count  = status.op06len;
        op06();
      } else {
        status.op06len    = status.parameters[0];
        status.in_index   = 0;
        status.in_count   = status.op06len;
        status.op06haslen = true;
        if(data)status.waiting_for_command = false;
      }
    } break;

    case 0x07: break;
    case 0x08: break;

    case 0x09: {
      op09();
    } break;

    case 0x0d: {
      if(status.op0dhaslen) {
        status.op0dhaslen = false;
        status.out_count  = status.op0doutlen;
        op0d();
      } else {
        status.op0dinlen  = status.parameters[0];
        status.op0doutlen = status.parameters[1];
        status.in_index   = 0;
        status.in_count   = (status.op0dinlen + 1) >> 1;
        status.op0dhaslen = true;
        if(data)status.waiting_for_command = false;
      }
    } break;

    case 0x0f: break;
    }
  }
}

DSP2::DSP2() {}
DSP2::~DSP2() {}
};

