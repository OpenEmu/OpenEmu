/////////////////////////////////////////////////////////////////////////
// $Id: dis_decode.cc,v 1.32 2006/05/12 17:04:19 sshwarts Exp $
/////////////////////////////////////////////////////////////////////////

#include "disasm.h"
#include "dis_tables.h"

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

x86_insn::x86_insn()
{
  extend8b = 0;
  seg_override = NO_SEG_OVERRIDE;
  prefixes = 0;
  ilen = 0;
  b1 = 0;

  modrm = mod = nnn = rm = 0;
  sib = scale = index = base = 0;
  displacement.displ32 = 0;
}

#define OPCODE(entry) ((BxDisasmOpcodeInfo_t*) entry->OpcodeInfo)
#define OPCODE_TABLE(entry) ((BxDisasmOpcodeTable_t*) entry->OpcodeInfo)

static const unsigned char instruction_has_modrm[512] = {
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f          */
  /*       -------------------------------          */
  /* 00 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, 
  /* 10 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, 
  /* 20 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, 
  /* 30 */ 1,1,1,1,0,0,0,0,1,1,1,1,0,0,0,0, 
  /* 40 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
  /* 50 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
  /* 60 */ 0,0,1,1,0,0,0,0,0,1,0,1,0,0,0,0, 
  /* 70 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
  /* 80 */ 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, 
  /* 90 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
  /* A0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
  /* B0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
  /* C0 */ 1,1,0,0,1,1,1,1,0,0,0,0,0,0,0,0, 
  /* D0 */ 1,1,1,1,0,0,0,0,1,1,1,1,1,1,1,1, 
  /* E0 */ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, 
  /* F0 */ 0,0,0,0,0,0,1,1,0,0,0,0,0,0,1,1,
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f           */
  /*       -------------------------------           */
           1,1,1,1,0,0,0,0,0,0,0,0,0,1,0,1, /* 0F 00 */
           1,1,1,1,1,1,1,1,1,0,0,0,0,0,0,1, /* 0F 10 */
           1,1,1,1,1,0,1,0,1,1,1,1,1,1,1,1, /* 0F 20 */
           0,0,0,0,0,0,0,0,1,0,1,0,0,0,0,0, /* 0F 30 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 40 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 50 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 60 */
           1,1,1,1,1,1,1,0,0,0,0,0,1,1,1,1, /* 0F 70 */
           0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, /* 0F 80 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F 90 */
           0,0,0,1,1,1,0,0,0,0,0,1,1,1,1,1, /* 0F A0 */
           1,1,1,1,1,1,1,1,0,0,1,1,1,1,1,1, /* 0F B0 */
           1,1,1,1,1,1,1,1,0,0,0,0,0,0,0,0, /* 0F C0 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F D0 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1, /* 0F E0 */
           1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0  /* 0F F0 */
  /*       -------------------------------           */
  /*       0 1 2 3 4 5 6 7 8 9 a b c d e f           */
};

unsigned disassembler::disasm(bx_address base, bx_address ip, const uint8 *instr, char *disbuf)
{
  x86_insn insn = decode(base, ip, instr, disbuf);
  return insn.ilen;
}

x86_insn disassembler::decode(bx_address base, bx_address ip, const uint8 *instr, char *disbuf)
{
  x86_insn insn;
  const uint8 *instruction_begin = instruction = instr;
  resolve_modrm = NULL;
  unsigned b3 = 0;

  db_eip = ip;
  db_base = base; // cs linear base (base for PM & cs<<4 for RM & VM)

  disbufptr = disbuf; // start sprintf()'ing into beginning of buffer

  for(;;)
  {
    if(insn.prefixes == 7) // V30MZ only supports 7 prefixes reliably
     break;
    insn.b1 = fetch_byte();
    insn.prefixes++;

    switch(insn.b1) {
      case 0x26:     // ES:
        insn.seg_override = ES_REG;
        continue;

      case 0x2e:     // CS:
        insn.seg_override = CS_REG;
        continue;

      case 0x36:     // SS:
        insn.seg_override = SS_REG;
        continue;

      case 0x3e:     // DS:
        insn.seg_override = DS_REG;
        continue;

      case 0x64:     // FS:
        insn.seg_override = FS_REG;
        continue;

      case 0x65:     // GS:
        insn.seg_override = GS_REG;
        continue;

      case 0xf0:     // lock
        continue;

      case 0xf2:     // repne
        continue;

      case 0xf3:     // rep
        continue;

      // no more prefixes
      default:
        break;
    }

    insn.prefixes--;
    break;
  }

  if (insn.b1 == 0x0f)
  {
    insn.b1 = 0x100 | fetch_byte();
  }

  const BxDisasmOpcodeTable_t *opcode_table, *entry;

  opcode_table = BxDisasmOpcodes16;

  entry = opcode_table + insn.b1;

  // will require 3rd byte for 3-byte opcode
  if (entry->Attr & _GRP3BTAB) b3 = fetch_byte();

  if (instruction_has_modrm[insn.b1])
  {
    decode_modrm(&insn);
  }

  int attr = entry->Attr;
  while(attr) 
  {
    switch(attr) {
       case _GROUPN:
         entry = &(OPCODE_TABLE(entry)[insn.nnn]);
         break;

       case _SPLIT11B:
         entry = &(OPCODE_TABLE(entry)[insn.mod != 3]); /* REG/MEM */
         break;

       case _GRPRM:
         entry = &(OPCODE_TABLE(entry)[insn.rm]);
         break;

       case _GRP3BTAB:
         entry = &(OPCODE_TABLE(entry)[b3 >> 4]);
         break;

       case _GRP3BOP:
         entry = &(OPCODE_TABLE(entry)[b3 & 15]);
         break;

       default:
         printf("Internal disassembler error - unknown attribute !\n");
         return x86_insn();
    }

    /* get additional attributes from group table */
    attr = entry->Attr;
  }

#define BRANCH_NOT_TAKEN 0x2E
#define BRANCH_TAKEN     0x3E

  unsigned branch_hint = 0;

  // print prefixes
  for(unsigned i=0;i<insn.prefixes;i++)
  {
    uint8 prefix_byte = *(instr+i);

    if (prefix_byte == 0xF3 || prefix_byte == 0xF2 || prefix_byte == 0xF0) 
    {
      const BxDisasmOpcodeTable_t *prefix = &(opcode_table[prefix_byte]);
      dis_sprintf("%s ", OPCODE(prefix)->IntelOpcode);
    }

    // branch hint for jcc instructions
    if ((insn.b1 >= 0x070 && insn.b1 <= 0x07F) ||
        (insn.b1 >= 0x180 && insn.b1 <= 0x18F))
    {
      if (prefix_byte == BRANCH_NOT_TAKEN || prefix_byte == BRANCH_TAKEN) 
        branch_hint = prefix_byte;
    }
  }

  const BxDisasmOpcodeInfo_t *opcode = OPCODE(entry);

  // fix nop opcode
  if (insn.b1 == 0x90) {
    opcode = &Ia_nop;
  }

  // print instruction disassembly
  if (intel_mode)
    print_disassembly_intel(&insn, opcode);
  else
    print_disassembly_att  (&insn, opcode);

  if (branch_hint == BRANCH_NOT_TAKEN)
  {
    dis_sprintf(", not taken");
  }
  else if (branch_hint == BRANCH_TAKEN)
  {
    dis_sprintf(", taken");
  }
 
  insn.ilen = (unsigned)(instruction - instruction_begin);

  return insn;
}

void disassembler::dis_sprintf(const char *fmt, ...)
{
  va_list ap;

  va_start(ap, fmt);
  vsprintf(disbufptr, fmt, ap);
  va_end(ap);

  disbufptr += strlen(disbufptr);
}

void disassembler::dis_putc(char symbol)
{
  *disbufptr++ = symbol;
  *disbufptr = 0;
}
