struct Registers {
  bool halt;

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
} regs;

uint24 stack[8];
uint16 opcode;

uint24 reg_read(uint8 addr) const;
void reg_write(uint8 addr, uint24 data);
