#include <stdio.h>
#include <string.h>

#include "../System.h"
#include "gbGlobals.h"

typedef struct {
  u8 mask;
  u8 value;
  const char *mnen;
} GBOPCODE;

#define GB_READ(x) gbMemoryMap[(x)>>12][(x)&0xfff]

static const char *registers[] =
  { "B", "C", "D", "E", "H", "L", "(HL)", "A" };

static const char *registers16[] =
  { "BC", "DE", "HL", "SP", // for some operations
    "BC", "DE", "HL", "AF" }; // for push/pop

static const char *cond[] =
  { "NZ", "Z", "NC", "C" };

static char hexDigits[16] = {
  '0', '1', '2', '3', '4', '5', '6', '7',
  '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
};

static GBOPCODE opcodes[] = {
  { 0xff, 0x00, "NOP" },
  { 0xcf, 0x01, "LD %R4,%W" },
  { 0xff, 0x02, "LD (BC),A" },
  { 0xcf, 0x03, "INC %R4" },
  { 0xc7, 0x04, "INC %r3" },
  { 0xc7, 0x05, "DEC %r3" },
  { 0xc7, 0x06, "LD %r3,%B" },
  { 0xff, 0x07, "RLCA" },
  { 0xff, 0x08, "LD (%W),SP" },
  { 0xcf, 0x09, "ADD HL,%R4" },
  { 0xff, 0x0a, "LD A,(BC)" },
  { 0xcf, 0x0b, "DEC %R4" },
  { 0xff, 0x0f, "RRCA" },
  { 0xff, 0x10, "STOP" },
  { 0xff, 0x12, "LD (DE),A" },
  { 0xff, 0x17, "RLA" },
  { 0xff, 0x18, "JR %d" },
  { 0xff, 0x1a, "LD A,(DE)" },
  { 0xff, 0x1f, "RRA" },
  { 0xe7, 0x20, "JR %c3,%d" },
  { 0xff, 0x22, "LDI (HL),A" },
  { 0xff, 0x27, "DAA" },
  { 0xff, 0x2a, "LDI A,(HL)" },
  { 0xff, 0x2f, "CPL" },
  { 0xff, 0x32, "LDD (HL),A" },
  { 0xff, 0x37, "SCF" },
  { 0xff, 0x3a, "LDD A,(HL)" },
  { 0xff, 0x3f, "CCF" },
  { 0xff, 0x76, "HALT" },
  { 0xc0, 0x40, "LD %r3,%r0" },
  { 0xf8, 0x80, "ADD A,%r0" },
  { 0xf8, 0x88, "ADC A,%r0" },
  { 0xf8, 0x90, "SUB %r0" },
  { 0xf8, 0x98, "SBC A,%r0" },
  { 0xf8, 0xa0, "AND %r0" },
  { 0xf8, 0xa8, "XOR %r0" },
  { 0xf8, 0xb0, "OR %r0" },
  { 0xf8, 0xb8, "CP %r0" },
  { 0xe7, 0xc0, "RET %c3" },
  { 0xcf, 0xc1, "POP %t4" },
  { 0xe7, 0xc2, "JP %c3,%W" },
  { 0xff, 0xc3, "JP %W" },
  { 0xe7, 0xc4, "CALL %c3,%W" },
  { 0xcf, 0xc5, "PUSH %t4" },
  { 0xff, 0xc6, "ADD A,%B" },
  { 0xc7, 0xc7, "RST %P" },
  { 0xff, 0xc9, "RET" },
  { 0xff, 0xcd, "CALL %W" },
  { 0xff, 0xce, "ADC %B" },
  { 0xff, 0xd6, "SUB %B" },
  { 0xff, 0xd9, "RETI" },
  { 0xff, 0xde, "SBC %B" },
  { 0xff, 0xe0, "LD (FF%B),A" },
  { 0xff, 0xe2, "LD (FF00h+C),A" },
  { 0xff, 0xe6, "AND %B" },
  { 0xff, 0xe8, "ADD SP,%D" },
  { 0xff, 0xe9, "LD PC,HL" },
  { 0xff, 0xea, "LD (%W),A" },
  { 0xff, 0xee, "XOR %B" },
  { 0xff, 0xf0, "LD A,(FF%B)" },
  { 0xff, 0xf2, "LD A,(FF00h+C)" },
  { 0xff, 0xf3, "DI" },
  { 0xff, 0xf6, "OR %B" },
  { 0xff, 0xf8, "LD HL,SP%D" },
  { 0xff, 0xf9, "LD SP,HL" },
  { 0xff, 0xfa, "LD A,(%W)" },
  { 0xff, 0xfb, "EI" },
  { 0xff, 0xfe, "CP %B" },
  { 0x00, 0x00, "DB %B" }
};

static GBOPCODE cbOpcodes[] = {
  { 0xf8, 0x00, "RLC %r0" },
  { 0xf8, 0x08, "RRC %r0" },
  { 0xf8, 0x10, "RL %r0" },
  { 0xf8, 0x18, "RR %r0" },
  { 0xf8, 0x20, "SLA %r0" },
  { 0xf8, 0x28, "SRA %r0" },
  { 0xf8, 0x30, "SWAP %r0" },
  { 0xf8, 0x38, "SRL %r0" },
  { 0xc0, 0x40, "BIT %b,%r0" },
  { 0xc0, 0x80, "RES %b,%r0" },
  { 0xc0, 0xc0, "SET %b,%r0" },
  { 0x00, 0x00, "DB CBh,%B" }
};

static char *addHex(char *p, u8 value)
{
  *p++ = hexDigits[value >> 4];
  *p++ = hexDigits[value & 15];
  return p;
}

static char *addHex16(char *p, u16 value)
{
  p = addHex(p, value>>8);
  return addHex(p, value & 255);
}

static char *addStr(char *p, const char *s)
{
  while(*s) {
    *p++ = *s++;
  }
  return p;
}

int gbDis(char *buffer, u16 address)
{
  char *p = buffer;
  int instr = 1;
  u16 addr = address;
  sprintf(p, "%04x        ", address);
  p += 12;

  u8 opcode = GB_READ(address);
  address++;
  const char *mnen;
  GBOPCODE *op;
  if(opcode == 0xcb) {
    opcode = GB_READ(address);
    address++;
    instr++;
    op = cbOpcodes;
  } else {
    op = opcodes;
  }
  while(op->value != (opcode & op->mask)) op++;
  mnen = op->mnen;

  u8 b0, b1;
  s8 disp;
  int shift;

  while(*mnen) {
    if(*mnen == '%') {
      mnen++;
      switch(*mnen++) {
      case 'W':
        b0 = GB_READ(address);
        address++;
        b1 = GB_READ(address);
        address++;
        p = addHex16(p, b0|b1<<8);
        instr += 2;
        *p++ = 'h';
        break;
      case 'B':
        p = addHex(p, GB_READ(address));
        *p++ = 'h';
        address++;
        instr++;
        break;
      case 'D':
        disp = GB_READ(address);
        if(disp >= 0)
          *p++ = '+';
        p += sprintf(p, "%d", disp);
        instr++;
        break;
      case 'd':
        disp = GB_READ(address);
        address++;
        p = addHex16(p, address+disp);
        *p++ = 'h';
        instr++;
        break;
      case 'b':
        // kind of a hack, but it works :-)
        *p++ = hexDigits[(opcode >> 3) & 7];
        break;
      case 'r':
        shift = *mnen++ - '0';
        p = addStr(p, registers[(opcode >> shift) & 7]);
        break;
      case 'R':
        shift = *mnen++ - '0';
        p = addStr(p, registers16[(opcode >> shift) & 3]);
        break;
      case 't':
        shift = *mnen++ - '0';
        p = addStr(p, registers16[4+((opcode >> shift) & 3)]);
        break;
      case 'P':
        p = addHex(p, ((opcode >> 3) & 7) * 8);
        break;
      case 'c':
        shift = *mnen++ - '0';
        p = addStr(p, cond[(opcode >> shift) & 3]);
        break;
      }
    } else
      *p++ = *mnen++;
  }
  for(int i = 0; i < instr; i++) {
    u16 a = addr + i;
    addHex(buffer+5+i*2, GB_READ(a));
  }
  *p = 0;
  return instr;
}
