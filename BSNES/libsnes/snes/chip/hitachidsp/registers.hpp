struct Registers {
  uint24 pc;
  uint16 p;
  bool n;
  bool z;
  bool c;

  uint24 a;
  uint24 acch;
  uint24 accl;
  uint24 busdata;
  uint24 romdata;
  uint24 ramdata;
  uint24 busaddr;
  uint24 ramaddr;
  uint24 gpr[16];

  //MMIO
  uint24 dma_source;       //$1f40-$1f42
  uint24 dma_length;       //$1f43-$1f44
  uint24 dma_target;       //$1f45-$1f47
  uint8  r1f48;            //$1f48
  uint24 program_offset;   //$1f49-$1f4b
  uint8  r1f4c;            //$1f4c
  uint16 page_number;      //$1f4d-$1f4e
  uint8  program_counter;  //$1f4f
  uint8  r1f50;            //$1f50
  uint8  r1f51;            //$1f51
  uint8  r1f52;            //$1f52
  uint8  vector[32];       //$1f60-$1f7f
} regs;
