/////////////////////////////////////////////////////////////////////////
// $Id: syntax.cc,v 1.10 2006/04/27 15:11:45 sshwarts Exp $
/////////////////////////////////////////////////////////////////////////

#include "disasm.h"
#include <stdio.h>

//////////////////
// Intel STYLE
//////////////////

static const char *intel_general_16bit_regname[8] = {
    "ax",  "cx",  "dx",  "bx",  "sp",  "bp",  "si",  "di"
};

static const char *intel_general_8bit_regname[8] = {
    "al",  "cl",  "dl",  "bl",  "ah",  "ch",  "dh",  "bh"
};

static const char *intel_segment_name[8] = {
    "es",  "cs",  "ss",  "ds",  "fs",  "gs",  "??",  "??"
};

static const char *intel_index16[8] = {
    "bx+si", 
    "bx+di", 
    "bp+si", 
    "bp+di", 
    "si", 
    "di", 
    "bp", 
    "bx"
};


//////////////////
// AT&T STYLE
//////////////////

static const char *att_general_16bit_regname[8] = {
    "%ax",  "%cx",  "%dx",  "%bx",  "%sp",  "%bp",  "%si",  "%di"
};

static const char *att_general_8bit_regname[8] = {
    "%al",  "%cl",  "%dl",  "%bl",  "%ah",  "%ch",  "%dh",  "%bh"
};

static const char *att_segment_name[8] = {
    "%es",  "%cs",  "%ss",  "%ds",  "%fs",  "%gs",  "%??",  "%??"
};

static const char *att_index16[8] = {
    "%bx, %si", 
    "%bx, %di", 
    "%bp, %si", 
    "%bp, %di", 
    "%si", 
    "%di", 
    "%bp", 
    "%bx"
};

#define NULL_SEGMENT_REGISTER 7

void disassembler::initialize_modrm_segregs()
{
  sreg_mod00_rm16[0] = segment_name[DS_REG];
  sreg_mod00_rm16[1] = segment_name[DS_REG];
  sreg_mod00_rm16[2] = segment_name[SS_REG];
  sreg_mod00_rm16[3] = segment_name[SS_REG];
  sreg_mod00_rm16[4] = segment_name[DS_REG];
  sreg_mod00_rm16[5] = segment_name[DS_REG];
  sreg_mod00_rm16[6] = segment_name[DS_REG];
  sreg_mod00_rm16[7] = segment_name[DS_REG];

  sreg_mod01or10_rm16[0] = segment_name[DS_REG];
  sreg_mod01or10_rm16[1] = segment_name[DS_REG];
  sreg_mod01or10_rm16[2] = segment_name[SS_REG];
  sreg_mod01or10_rm16[3] = segment_name[SS_REG];
  sreg_mod01or10_rm16[4] = segment_name[DS_REG];
  sreg_mod01or10_rm16[5] = segment_name[DS_REG];
  sreg_mod01or10_rm16[6] = segment_name[SS_REG];
  sreg_mod01or10_rm16[7] = segment_name[DS_REG];

  sreg_mod01or10_rm32[0] = segment_name[DS_REG];
  sreg_mod01or10_rm32[1] = segment_name[DS_REG];
  sreg_mod01or10_rm32[2] = segment_name[DS_REG];
  sreg_mod01or10_rm32[3] = segment_name[DS_REG];
  sreg_mod01or10_rm32[4] = segment_name[NULL_SEGMENT_REGISTER];
  sreg_mod01or10_rm32[5] = segment_name[SS_REG];
  sreg_mod01or10_rm32[6] = segment_name[DS_REG];
  sreg_mod01or10_rm32[7] = segment_name[DS_REG];

  sreg_mod00_base32[0] = segment_name[DS_REG];
  sreg_mod00_base32[1] = segment_name[DS_REG];
  sreg_mod00_base32[2] = segment_name[DS_REG];
  sreg_mod00_base32[3] = segment_name[DS_REG];
  sreg_mod00_base32[4] = segment_name[SS_REG];
  sreg_mod00_base32[5] = segment_name[DS_REG];
  sreg_mod00_base32[6] = segment_name[DS_REG];
  sreg_mod00_base32[7] = segment_name[DS_REG];

  sreg_mod01or10_base32[0] = segment_name[DS_REG];
  sreg_mod01or10_base32[1] = segment_name[DS_REG];
  sreg_mod01or10_base32[2] = segment_name[DS_REG];
  sreg_mod01or10_base32[3] = segment_name[DS_REG];
  sreg_mod01or10_base32[4] = segment_name[SS_REG];
  sreg_mod01or10_base32[5] = segment_name[SS_REG];
  sreg_mod01or10_base32[6] = segment_name[DS_REG];
  sreg_mod01or10_base32[7] = segment_name[DS_REG];
}

//////////////////
// Intel STYLE
//////////////////

void disassembler::set_syntax_intel()
{
  intel_mode = 1;

  general_16bit_regname = intel_general_16bit_regname;
  general_8bit_regname = intel_general_8bit_regname;

  segment_name = intel_segment_name;
  index16 = intel_index16;

  initialize_modrm_segregs();
}

void disassembler::print_disassembly_intel(const x86_insn *insn, const BxDisasmOpcodeInfo_t *entry)
{
  // print opcode
  dis_sprintf("%s ", entry->IntelOpcode);

  if (entry->Operand1) {
    (this->*entry->Operand1)(insn);
  }
  if (entry->Operand2) {
    dis_sprintf(", ");
    (this->*entry->Operand2)(insn);
  }
  if (entry->Operand3) {
    dis_sprintf(", ");
    (this->*entry->Operand3)(insn);
  }
}

//////////////////
// AT&T STYLE
//////////////////
 
void disassembler::set_syntax_att()
{
  intel_mode = 0;

  general_16bit_regname = att_general_16bit_regname;
  general_8bit_regname = att_general_8bit_regname;

  segment_name = att_segment_name;
  index16 = att_index16;

  initialize_modrm_segregs();
}

void disassembler::toggle_syntax_mode()
{
  if (intel_mode) set_syntax_att();
  else set_syntax_intel();
}

void disassembler::print_disassembly_att(const x86_insn *insn, const BxDisasmOpcodeInfo_t *entry)
{
  // print opcode
  dis_sprintf("%s ", entry->AttOpcode);

  if (entry->Operand3) {                                         
    (this->*entry->Operand3)(insn);
    dis_sprintf(", ");
  }
  if (entry->Operand2) {
    (this->*entry->Operand2)(insn);
    dis_sprintf(", ");
  }
  if (entry->Operand1) {
    (this->*entry->Operand1)(insn);
  }
}
