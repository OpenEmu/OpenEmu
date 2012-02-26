#include <snes/snes.hpp>

#define ST0018_CPP
namespace SNES {

ST0018 st0018;

uint8 ST0018::mmio_read(unsigned addr) {
  addr &= 0xffff;
  if(addr == 0x3800) return regs.r3800;
  if(addr == 0x3804) return regs.r3804;
  return cpu.regs.mdr;
}

void ST0018::mmio_write(unsigned addr, uint8 data) {
  addr &= 0xffff;

  if(addr == 0x3802) {
    switch(regs.mode) {
      case Waiting: {
        switch(data) {
          case 0x01: regs.r3800 = regs.r3800_01; break;
          case 0xaa: op_board_upload(); break;
          case 0xb2: op_b2(); break;
          case 0xb3: op_b3(); break;
          case 0xb4: op_b4(); break;
          case 0xb5: op_b5(); break;
          case 0xf1: op_query_chip(); break;
          case 0xf2: op_query_chip(); break;
          default: fprintf(stdout, "* ST018 w3802::%.2x\n", data); break;
        }
      } return;

      case BoardUpload: {
        op_board_upload(data);
      } return;
    }
  }

  if(addr == 0x3804) {
    regs.w3804 <<= 8;
    regs.w3804  |= data;
    regs.w3804  &= 0xffffff;
    return;
  }
}

void ST0018::init() {
}

void ST0018::load() {
}

void ST0018::unload() {
}

void ST0018::power() {
}

void ST0018::reset() {
  regs.mode = Waiting;
  regs.r3800 = 0x00;
  regs.r3804 = 0x85;
  regs.w3804 = 0;
  for(unsigned i = 0; i < 97; i++) board[i] = 0;
}

//===============
//ST-0018 opcodes
//===============

void ST0018::op_board_upload() {
  regs.mode = BoardUpload;
  regs.counter = 0;
  regs.r3800 = 0xe0;
}

void ST0018::op_board_upload(uint8 data) {
  board[regs.counter] = data;
  regs.r3800 = 96 - regs.counter;
  regs.counter++;
  if(regs.counter >= 97) {
    regs.mode = Waiting;
    #if 0
    for(unsigned y = 0; y < 9; y++) {
      for(unsigned x = 0; x < 9; x++) {
        fprintf(stdout, "%.2x ", board[y * 9 + x]);
      }
      fprintf(stdout, "\n");
    }
    for(unsigned n = 0; n < 16; n++) fprintf(stdout, "%.2x ", board[81 + n]);
    fprintf(stdout, "\n\n");
    #endif
  }
}

void ST0018::op_b2() {
  fprintf(stdout, "* ST018 w3802::b2\n");
  regs.r3800 = 0xe0;
  regs.r3800_01 = 0;  //unknown
}

void ST0018::op_b3() {
  fprintf(stdout, "* ST018 w3802::b3\n");
  regs.r3800 = 0xe0;
  regs.r3800_01 = 1;  //0 = player lost?
}

void ST0018::op_b4() {
  fprintf(stdout, "* ST018 w3802::b4\n");
  regs.r3800 = 0xe0;
  regs.r3800_01 = 1;  //0 = player won?
}

void ST0018::op_b5() {
  fprintf(stdout, "* ST018 w3802::b5\n");
  regs.r3800 = 0xe0;
  regs.r3800_01 = 0;  //1 = move will result in checkmate?
}

void ST0018::op_query_chip() {
  regs.r3800 = 0x00;
}

}
