/////////////////////////////////////////////////////////////////////////
// $Id: dis_groups.cc,v 1.33 2006/08/13 09:40:07 sshwarts Exp $
/////////////////////////////////////////////////////////////////////////

#include "disasm.h"
#include <stdio.h>
#include <assert.h>

/*
#if BX_DEBUGGER
#include "../bx_debug/debug.h"
#endif
*/

void disassembler::Apw(const x86_insn *insn)
{
  uint16 imm16 = fetch_word();
  uint16 cs_selector = fetch_word();
  dis_sprintf("%04x:%04x", (unsigned) cs_selector, (unsigned) imm16);
}

void disassembler::Apd(const x86_insn *insn)
{
  uint32 imm32 = fetch_dword();
  uint16 cs_selector = fetch_word();
  dis_sprintf("%04x:%08x", (unsigned) cs_selector, (unsigned) imm32);
}

// 8-bit general purpose registers
void disassembler::AL(const x86_insn *insn) { dis_sprintf("%s", general_8bit_regname[rAX_REG]); }
void disassembler::CL(const x86_insn *insn) { dis_sprintf("%s", general_8bit_regname[rCX_REG]); }

// 16-bit general purpose registers
void disassembler::AX(const x86_insn *insn) {
  dis_sprintf("%s", general_16bit_regname[rAX_REG]);
}

void disassembler::DX(const x86_insn *insn) {
  dis_sprintf("%s", general_16bit_regname[rDX_REG]);
}

// segment registers
void disassembler::CS(const x86_insn *insn) { dis_sprintf("%s", segment_name[CS_REG]); }
void disassembler::DS(const x86_insn *insn) { dis_sprintf("%s", segment_name[DS_REG]); }
void disassembler::ES(const x86_insn *insn) { dis_sprintf("%s", segment_name[ES_REG]); }
void disassembler::SS(const x86_insn *insn) { dis_sprintf("%s", segment_name[SS_REG]); }

void disassembler::Sw(const x86_insn *insn) { dis_sprintf("%s", segment_name[insn->nnn]); }

// test registers
void disassembler::Td(const x86_insn *insn)
{
  if (intel_mode)
    dis_sprintf  ("tr%d", insn->nnn);
  else
    dis_sprintf("%%tr%d", insn->nnn);
}

// control register
void disassembler::Cd(const x86_insn *insn) 
{ 
  if (intel_mode)
    dis_sprintf  ("cr%d", insn->nnn);
  else
    dis_sprintf("%%cr%d", insn->nnn);
}

void disassembler::Cq(const x86_insn *insn) { Cd(insn); }

// debug register
void disassembler::Dd(const x86_insn *insn) 
{
  if (intel_mode)
    dis_sprintf  ("db%d", insn->nnn);
  else
    dis_sprintf("%%db%d", insn->nnn);
}

void disassembler::Dq(const x86_insn *insn) { Dd(insn); }

// 8-bit general purpose register
void disassembler::R8(const x86_insn *insn)
{ 
  unsigned reg = (insn->b1 & 7);
 
  dis_sprintf("%s", general_8bit_regname[reg]);
}

// 16-bit general purpose register
void disassembler::RX(const x86_insn *insn)
{ 
  dis_sprintf("%s", general_16bit_regname[(insn->b1 & 7)]);
}

// general purpose register or memory operand
void disassembler::Eb(const x86_insn *insn) 
{
  if (insn->mod == 3) {
   dis_sprintf("%s", general_8bit_regname[insn->rm]);
  }
  else
    (this->*resolve_modrm)(insn, B_SIZE);
}

void disassembler::Ew(const x86_insn *insn) 
{
  if (insn->mod == 3)
    dis_sprintf("%s", general_16bit_regname[insn->rm]);
  else
    (this->*resolve_modrm)(insn, W_SIZE);
}

// general purpose register
void disassembler::Gb(const x86_insn *insn) 
{
  dis_sprintf("%s", general_8bit_regname[insn->nnn]);
}

void disassembler::Gw(const x86_insn *insn) 
{
  dis_sprintf("%s", general_16bit_regname[insn->nnn]);
}

// immediate
void disassembler::I1(const x86_insn *insn) 
{ 
  if (! intel_mode) dis_putc('$');
  dis_putc ('1');
}

void disassembler::Ib(const x86_insn *insn) 
{
  if (! intel_mode) dis_putc('$');
  dis_sprintf("0x%02x", (unsigned) fetch_byte());
}

void disassembler::Iw(const x86_insn *insn) 
{
  if (! intel_mode) dis_putc('$');
  dis_sprintf("0x%04x", (unsigned) fetch_word());
}

void disassembler::IwIb(const x86_insn *insn) 
{
  uint16 iw = fetch_word();
  uint8  ib = fetch_byte();

  if (intel_mode) {
     dis_sprintf("0x%04x, 0x%02x", iw, ib);
  }
  else {
     dis_sprintf("$0x%02x, $0x%04x", ib, iw);
  }
}

// sign extended immediate
void disassembler::sIbw(const x86_insn *insn) 
{
  if (! intel_mode) dis_putc('$');
  uint16 imm16 = (int8) fetch_byte();
  dis_sprintf("0x%04x", (unsigned) imm16);
}

// sign extended immediate
void disassembler::sIbd(const x86_insn *insn) 
{
  if (! intel_mode) dis_putc('$');
  uint32 imm32 = (int8) fetch_byte();
  dis_sprintf ("0x%08x", (unsigned) imm32);
}

// 16-bit general purpose register
void disassembler::Rw(const x86_insn *insn)
{
  dis_sprintf("%s", general_16bit_regname[insn->rm]);
}

// direct memory access
void disassembler::OP_O(const x86_insn *insn, unsigned size)
{
  const char *seg;

  if (insn->is_seg_override())
    seg = segment_name[insn->seg_override];
  else
    seg = segment_name[DS_REG];

  print_datasize(size);

  uint16 imm16 = fetch_word();
  dis_sprintf("%s:0x%x", seg, (unsigned) imm16);
}

void disassembler::Ob(const x86_insn *insn) { OP_O(insn, B_SIZE); }
void disassembler::Ow(const x86_insn *insn) { OP_O(insn, W_SIZE); }
void disassembler::Od(const x86_insn *insn) { OP_O(insn, D_SIZE); }
void disassembler::Oq(const x86_insn *insn) { OP_O(insn, Q_SIZE); }

// memory operand
void disassembler::OP_M(const x86_insn *insn, unsigned size)
{
  if(insn->mod == 3)
    dis_sprintf("(bad)");
  else
    (this->*resolve_modrm)(insn, size);
}

void disassembler::Ma(const x86_insn *insn) { OP_M(insn, X_SIZE); }
void disassembler::Mp(const x86_insn *insn) { OP_M(insn, X_SIZE); }
void disassembler::Ms(const x86_insn *insn) { OP_M(insn, X_SIZE); }
void disassembler::Mx(const x86_insn *insn) { OP_M(insn, X_SIZE); }

void disassembler::Mb(const x86_insn *insn) { OP_M(insn, B_SIZE); }
void disassembler::Mw(const x86_insn *insn) { OP_M(insn, W_SIZE); }
void disassembler::Md(const x86_insn *insn) { OP_M(insn, D_SIZE); }
void disassembler::Mq(const x86_insn *insn) { OP_M(insn, Q_SIZE); }
void disassembler::Mt(const x86_insn *insn) { OP_M(insn, T_SIZE); }

void disassembler::Mdq(const x86_insn *insn) { OP_M(insn, O_SIZE); }
void disassembler::Mps(const x86_insn *insn) { OP_M(insn, O_SIZE); }
void disassembler::Mpd(const x86_insn *insn) { OP_M(insn, O_SIZE); }

// string instructions
void disassembler::OP_X(const x86_insn *insn, unsigned size)
{
  const char *rsi, *seg;

  rsi = general_16bit_regname[rSI_REG];
  
  if (insn->is_seg_override())
    seg = segment_name[insn->seg_override];
  else
    seg = segment_name[DS_REG];

  print_datasize(size);

  if (intel_mode)
    dis_sprintf("%s:[%s]", seg, rsi);
  else
    dis_sprintf("%s:(%s)", seg, rsi);
}

void disassembler::Xb(const x86_insn *insn) { OP_X(insn, B_SIZE); }
void disassembler::Xw(const x86_insn *insn) { OP_X(insn, W_SIZE); }
void disassembler::Xd(const x86_insn *insn) { OP_X(insn, D_SIZE); }
void disassembler::Xq(const x86_insn *insn) { OP_X(insn, Q_SIZE); }

void disassembler::OP_Y(const x86_insn *insn, unsigned size)
{
  const char *rdi;

  rdi = general_16bit_regname[rDI_REG];
  
  print_datasize(size);

  if (intel_mode)
    dis_sprintf("%s:[%s]", segment_name[ES_REG], rdi);
  else
    dis_sprintf("%s:(%s)", segment_name[ES_REG], rdi);
}

void disassembler::Yb(const x86_insn *insn) { OP_Y(insn, B_SIZE); }
void disassembler::Yw(const x86_insn *insn) { OP_Y(insn, W_SIZE); }
void disassembler::Yd(const x86_insn *insn) { OP_Y(insn, D_SIZE); }
void disassembler::Yq(const x86_insn *insn) { OP_Y(insn, Q_SIZE); }

#define BX_JUMP_TARGET_NOT_REQ ((bx_address)(-1))

// jump offset
void disassembler::Jb(const x86_insn *insn)
{
  int8 imm8 = (int8) fetch_byte();

  uint16 imm16 = (int16) imm8;
  dis_sprintf(".+0x%04x", (unsigned) imm16);

  if (db_base != BX_JUMP_TARGET_NOT_REQ) {
    uint16 target = (db_eip + (int16) imm16) & 0xffff;
    dis_sprintf(" (0x%08x)", target + db_base);
  }
}

void disassembler::Jw(const x86_insn *insn)
{
  uint16 imm16 = (int16) fetch_word();
  dis_sprintf(".+0x%04x", (unsigned) imm16);

  if (db_base != BX_JUMP_TARGET_NOT_REQ) {
    uint16 target = (db_eip + (int16) imm16) & 0xffff;
    dis_sprintf(" (0x%08x)", target + db_base);
  }
}

void disassembler::Jd(const x86_insn *insn)
{
  int32 imm32 = (int32) fetch_dword();

  dis_sprintf(".+0x%08x", (unsigned) imm32);

  if (db_base != BX_JUMP_TARGET_NOT_REQ) {
    uint32 target = db_eip + (int32) imm32; target += db_base;
    dis_sprintf(" (0x%08x)", target);
  }
}
