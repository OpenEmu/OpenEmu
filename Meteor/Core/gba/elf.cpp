#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "GBA.h"
#include "../common/Port.h"
#include "elf.h"
#include "../NLS.h"

#define elfReadMemory(addr) \
  READ32LE((&map[(addr)>>24].address[(addr) & map[(addr)>>24].mask]))

#define DW_TAG_array_type             0x01
#define DW_TAG_enumeration_type       0x04
#define DW_TAG_formal_parameter       0x05
#define DW_TAG_label                  0x0a
#define DW_TAG_lexical_block          0x0b
#define DW_TAG_member                 0x0d
#define DW_TAG_pointer_type           0x0f
#define DW_TAG_reference_type         0x10
#define DW_TAG_compile_unit           0x11
#define DW_TAG_structure_type         0x13
#define DW_TAG_subroutine_type        0x15
#define DW_TAG_typedef                0x16
#define DW_TAG_union_type             0x17
#define DW_TAG_unspecified_parameters 0x18
#define DW_TAG_inheritance            0x1c
#define DW_TAG_inlined_subroutine     0x1d
#define DW_TAG_subrange_type          0x21
#define DW_TAG_base_type              0x24
#define DW_TAG_const_type             0x26
#define DW_TAG_enumerator             0x28
#define DW_TAG_subprogram             0x2e
#define DW_TAG_variable               0x34
#define DW_TAG_volatile_type          0x35

#define DW_AT_sibling              0x01
#define DW_AT_location             0x02
#define DW_AT_name                 0x03
#define DW_AT_byte_size            0x0b
#define DW_AT_bit_offset           0x0c
#define DW_AT_bit_size             0x0d
#define DW_AT_stmt_list            0x10
#define DW_AT_low_pc               0x11
#define DW_AT_high_pc              0x12
#define DW_AT_language             0x13
#define DW_AT_compdir              0x1b
#define DW_AT_const_value          0x1c
#define DW_AT_containing_type      0x1d
#define DW_AT_inline               0x20
#define DW_AT_producer             0x25
#define DW_AT_prototyped           0x27
#define DW_AT_upper_bound          0x2f
#define DW_AT_abstract_origin      0x31
#define DW_AT_accessibility        0x32
#define DW_AT_artificial           0x34
#define DW_AT_data_member_location 0x38
#define DW_AT_decl_file            0x3a
#define DW_AT_decl_line            0x3b
#define DW_AT_declaration          0x3c
#define DW_AT_encoding             0x3e
#define DW_AT_external             0x3f
#define DW_AT_frame_base           0x40
#define DW_AT_macro_info           0x43
#define DW_AT_specification        0x47
#define DW_AT_type                 0x49
#define DW_AT_virtuality           0x4c
#define DW_AT_vtable_elem_location 0x4d
// DWARF 2.1/3.0 extensions
#define DW_AT_entry_pc             0x52
#define DW_AT_ranges               0x55
// ARM Compiler extensions
#define DW_AT_proc_body            0x2000
#define DW_AT_save_offset          0x2001
#define DW_AT_user_2002            0x2002
// MIPS extensions
#define DW_AT_MIPS_linkage_name    0x2007

#define DW_FORM_addr      0x01
#define DW_FORM_data2     0x05
#define DW_FORM_data4     0x06
#define DW_FORM_string    0x08
#define DW_FORM_block     0x09
#define DW_FORM_block1    0x0a
#define DW_FORM_data1     0x0b
#define DW_FORM_flag      0x0c
#define DW_FORM_sdata     0x0d
#define DW_FORM_strp      0x0e
#define DW_FORM_udata     0x0f
#define DW_FORM_ref_addr  0x10
#define DW_FORM_ref4      0x13
#define DW_FORM_ref_udata 0x15
#define DW_FORM_indirect  0x16

#define DW_OP_addr        0x03
#define DW_OP_plus_uconst 0x23
#define DW_OP_reg0        0x50
#define DW_OP_reg1        0x51
#define DW_OP_reg2        0x52
#define DW_OP_reg3        0x53
#define DW_OP_reg4        0x54
#define DW_OP_reg5        0x55
#define DW_OP_reg6        0x56
#define DW_OP_reg7        0x57
#define DW_OP_reg8        0x58
#define DW_OP_reg9        0x59
#define DW_OP_reg10       0x5a
#define DW_OP_reg11       0x5b
#define DW_OP_reg12       0x5c
#define DW_OP_reg13       0x5d
#define DW_OP_reg14       0x5e
#define DW_OP_reg15       0x5f
#define DW_OP_fbreg       0x91

#define DW_LNS_extended_op      0x00
#define DW_LNS_copy             0x01
#define DW_LNS_advance_pc       0x02
#define DW_LNS_advance_line     0x03
#define DW_LNS_set_file         0x04
#define DW_LNS_set_column       0x05
#define DW_LNS_negate_stmt      0x06
#define DW_LNS_set_basic_block  0x07
#define DW_LNS_const_add_pc     0x08
#define DW_LNS_fixed_advance_pc 0x09

#define DW_LNE_end_sequence 0x01
#define DW_LNE_set_address  0x02
#define DW_LNE_define_file  0x03

#define DW_CFA_advance_loc      0x01
#define DW_CFA_offset           0x02
#define DW_CFA_restore          0x03
#define DW_CFA_set_loc          0x01
#define DW_CFA_advance_loc1     0x02
#define DW_CFA_advance_loc2     0x03
#define DW_CFA_advance_loc4     0x04
#define DW_CFA_offset_extended  0x05
#define DW_CFA_restore_extended 0x06
#define DW_CFA_undefined        0x07
#define DW_CFA_same_value       0x08
#define DW_CFA_register         0x09
#define DW_CFA_remember_state   0x0a
#define DW_CFA_restore_state    0x0b
#define DW_CFA_def_cfa          0x0c
#define DW_CFA_def_cfa_register 0x0d
#define DW_CFA_def_cfa_offset   0x0e
#define DW_CFA_nop              0x00

#define CASE_TYPE_TAG \
    case DW_TAG_const_type:\
    case DW_TAG_volatile_type:\
    case DW_TAG_pointer_type:\
    case DW_TAG_base_type:\
    case DW_TAG_array_type:\
    case DW_TAG_structure_type:\
    case DW_TAG_union_type:\
    case DW_TAG_typedef:\
    case DW_TAG_subroutine_type:\
    case DW_TAG_enumeration_type:\
    case DW_TAG_enumerator:\
    case DW_TAG_reference_type

struct ELFcie {
  ELFcie *next;
  u32 offset;
  u8 *augmentation;
  u32 codeAlign;
  s32 dataAlign;
  int returnAddress;
  u8 *data;
  u32 dataLen;
};

struct ELFfde {
  ELFcie *cie;
  u32 address;
  u32 end;
  u8 *data;
  u32 dataLen;
};

enum ELFRegMode {
  REG_NOT_SET,
  REG_OFFSET,
  REG_REGISTER
};


struct ELFFrameStateRegister {
  ELFRegMode mode;
  int reg;
  s32 offset;
};

struct ELFFrameStateRegisters {
  ELFFrameStateRegister regs[16];
  ELFFrameStateRegisters *previous;
};

enum ELFCfaMode {
  CFA_NOT_SET,
  CFA_REG_OFFSET
};

struct ELFFrameState {
  ELFFrameStateRegisters registers;

  ELFCfaMode cfaMode;
  int cfaRegister;
  s32 cfaOffset;

  u32 pc;

  int dataAlign;
  int codeAlign;
  int returnAddress;
};

extern bool cpuIsMultiBoot;

Symbol *elfSymbols = NULL;
char *elfSymbolsStrTab = NULL;
int elfSymbolsCount = 0;

ELFSectionHeader **elfSectionHeaders = NULL;
char *elfSectionHeadersStringTable = NULL;
int elfSectionHeadersCount = 0;
u8 *elfFileData = NULL;

CompileUnit *elfCompileUnits = NULL;
DebugInfo *elfDebugInfo = NULL;
char *elfDebugStrings = NULL;

ELFcie *elfCies = NULL;
ELFfde **elfFdes = NULL;
int elfFdeCount = 0;

CompileUnit *elfCurrentUnit = NULL;

u32 elfRead4Bytes(u8 *);
u16 elfRead2Bytes(u8 *);

CompileUnit *elfGetCompileUnit(u32 addr)
{
  if(elfCompileUnits) {
    CompileUnit *unit = elfCompileUnits;
    while(unit) {
      if(unit->lowPC) {
        if(addr >= unit->lowPC && addr < unit->highPC)
          return unit;
      } else {
        ARanges *r = unit->ranges;
        if(r) {
          int count = r->count;
          for(int j = 0; j < count; j++) {
            if(addr >= r->ranges[j].lowPC && addr < r->ranges[j].highPC)
              return unit;
          }
        }
      }
      unit = unit->next;
    }
  }
  return NULL;
}

const char *elfGetAddressSymbol(u32 addr)
{
  static char buffer[256];

  CompileUnit *unit = elfGetCompileUnit(addr);
  // found unit, need to find function
  if(unit) {
    Function *func = unit->functions;
    while(func) {
      if(addr >= func->lowPC && addr < func->highPC) {
        int offset = addr - func->lowPC;
        const char *name = func->name;
        if(!name)
          name = "";
        if(offset)
          sprintf(buffer, "%s+%d", name, offset);
        else
          strcpy(buffer, name);
        return buffer;
      }
      func = func->next;
    }
  }

  if(elfSymbolsCount) {
    for(int i = 0; i < elfSymbolsCount; i++) {
      Symbol *s = &elfSymbols[i];
      if((addr >= s->value)  && addr < (s->value+s->size)) {
        int offset = addr-s->value;
        const char *name = s->name;
        if(name == NULL)
          name = "";
        if(offset)
          sprintf(buffer, "%s+%d", name, addr-s->value);
        else
          strcpy(buffer, name);
        return buffer;
      } else if(addr == s->value) {
        if(s->name)
          strcpy(buffer, s->name);
        else
          strcpy(buffer, "");
        return buffer;
      }
    }
  }

  return "";
}

bool elfFindLineInModule(u32 *addr, const char *name, int line)
{
  CompileUnit *unit = elfCompileUnits;

  while(unit) {
    if(unit->lineInfoTable) {
      int i;
      int count = unit->lineInfoTable->fileCount;
      char *found = NULL;
      for(i = 0; i < count; i++) {
        if(strcmp(name, unit->lineInfoTable->files[i]) == 0) {
          found = unit->lineInfoTable->files[i];
          break;
        }
      }
      // found a matching filename... try to find line now
      if(found) {
        LineInfoItem *table = unit->lineInfoTable->lines;
        count = unit->lineInfoTable->number;
        for(i = 0; i < count; i++) {
          if(table[i].file == found && table[i].line == line) {
            *addr = table[i].address;
            return true;
          }
        }
        // we can only find a single match
        return false;
      }
    }
    unit = unit->next;
  }
  return false;
}

int elfFindLine(CompileUnit *unit, Function * /* func */, u32 addr, const char **f)
{
  int currentLine = -1;
  if(unit->hasLineInfo) {
    int count = unit->lineInfoTable->number;
    LineInfoItem *table = unit->lineInfoTable->lines;
    int i;
    for(i = 0; i < count; i++) {
      if(addr <= table[i].address)
        break;
    }
    if(i == count)
      i--;
    *f = table[i].file;
    currentLine = table[i].line;
  }
  return currentLine;
}

bool elfFindLineInUnit(u32 *addr, CompileUnit *unit, int line)
{
  if(unit->hasLineInfo) {
    int count = unit->lineInfoTable->number;
    LineInfoItem *table = unit->lineInfoTable->lines;
    int i;
    for(i = 0; i < count; i++) {
      if(line == table[i].line) {
        *addr = table[i].address;
        return true;
      }
    }
  }
  return false;
}

bool elfGetCurrentFunction(u32 addr, Function **f, CompileUnit **u)
{
  CompileUnit *unit = elfGetCompileUnit(addr);
  // found unit, need to find function
  if(unit) {
    Function *func = unit->functions;
    while(func) {
      if(addr >= func->lowPC && addr < func->highPC) {
        *f = func;
        *u = unit;
        return true;
      }
      func = func->next;
    }
  }
  return false;
}

bool elfGetObject(const char *name, Function *f, CompileUnit *u, Object **o)
{
  if(f && u) {
    Object *v = f->variables;

    while(v) {
      if(strcmp(name, v->name) == 0) {
        *o = v;
        return true;
      }
      v = v->next;
    }
    v = f->parameters;
    while(v) {
      if(strcmp(name, v->name) == 0) {
        *o = v;
        return true;
      }
      v = v->next;
    }
    v = u->variables;
    while(v) {
      if(strcmp(name, v->name) == 0) {
        *o = v;
        return true;
      }
      v = v->next;
    }
  }

  CompileUnit *c = elfCompileUnits;

  while(c) {
    if(c != u) {
      Object *v = c->variables;
      while(v) {
        if(strcmp(name, v->name) == 0) {
          *o = v;
          return true;
        }
        v = v->next;
      }
    }
    c = c->next;
  }

  return false;
}

const char *elfGetSymbol(int i, u32 *value, u32 *size, int *type)
{
  if(i < elfSymbolsCount) {
    Symbol *s = &elfSymbols[i];
    *value = s->value;
    *size = s->size;
    *type = s->type;
    return s->name;
  }
  return NULL;
}

bool elfGetSymbolAddress(const char *sym, u32 *addr, u32 *size, int *type)
{
  if(elfSymbolsCount) {
    for(int i = 0; i < elfSymbolsCount; i++) {
      Symbol *s = &elfSymbols[i];
      if(strcmp(sym, s->name) == 0) {
        *addr = s->value;
        *size = s->size;
        *type = s->type;
        return true;
      }
    }
  }
  return false;
}

ELFfde *elfGetFde(u32 address)
{
  if(elfFdes) {
    int i;
    for(i = 0; i < elfFdeCount; i++) {
      if(address >= elfFdes[i]->address &&
         address < elfFdes[i]->end) {
        return elfFdes[i];
      }
    }
  }

  return NULL;
}

void elfExecuteCFAInstructions(ELFFrameState *state, u8 *data, u32 len,
                               u32 pc)
{
  u8 *end = data + len;
  int bytes;
  int reg;
  ELFFrameStateRegisters *fs;

  while(data < end && state->pc < pc) {
    u8 op = *data++;

    switch(op >> 6) {
    case DW_CFA_advance_loc:
      state->pc += (op & 0x3f) * state->codeAlign;
      break;
    case DW_CFA_offset:
      reg = op & 0x3f;
      state->registers.regs[reg].mode = REG_OFFSET;
      state->registers.regs[reg].offset = state->dataAlign *
        (s32)elfReadLEB128(data, &bytes);
      data += bytes;
      break;
    case DW_CFA_restore:
      // we don't care much about the other possible settings,
      // so just setting to unset is enough for now
      state->registers.regs[op & 0x3f].mode = REG_NOT_SET;
      break;
    case 0:
      switch(op & 0x3f) {
      case DW_CFA_nop:
          break;
      case DW_CFA_advance_loc1:
        state->pc += state->codeAlign * (*data++);
        break;
      case DW_CFA_advance_loc2:
        state->pc += state->codeAlign * elfRead2Bytes(data);
        data += 2;
        break;
      case DW_CFA_advance_loc4:
        state->pc += state->codeAlign * elfRead4Bytes(data);
        data += 4;
        break;
      case DW_CFA_offset_extended:
        reg = elfReadLEB128(data, &bytes);
        data += bytes;
        state->registers.regs[reg].mode = REG_OFFSET;
        state->registers.regs[reg].offset = state->dataAlign *
          (s32)elfReadLEB128(data, &bytes);
        data += bytes;
        break;
      case DW_CFA_restore_extended:
      case DW_CFA_undefined:
      case DW_CFA_same_value:
        reg = elfReadLEB128(data, &bytes);
        data += bytes;
        state->registers.regs[reg].mode = REG_NOT_SET;
        break;
      case DW_CFA_register:
        reg = elfReadLEB128(data, &bytes);
        data += bytes;
        state->registers.regs[reg].mode = REG_REGISTER;
        state->registers.regs[reg].reg = elfReadLEB128(data, &bytes);
        data += bytes;
        break;
      case DW_CFA_remember_state:
        fs = (ELFFrameStateRegisters *)calloc(1,
                                              sizeof(ELFFrameStateRegisters));
        memcpy(fs, &state->registers, sizeof(ELFFrameStateRegisters));
        state->registers.previous = fs;
        break;
      case DW_CFA_restore_state:
        if(state->registers.previous == NULL) {
          printf("Error: previous frame state is NULL.\n");
          return;
        }
        fs = state->registers.previous;
        memcpy(&state->registers, fs, sizeof(ELFFrameStateRegisters));
        free(fs);
        break;
      case DW_CFA_def_cfa:
        state->cfaRegister = elfReadLEB128(data, &bytes);
        data += bytes;
        state->cfaOffset = (s32)elfReadLEB128(data, &bytes);
        data += bytes;
        state->cfaMode = CFA_REG_OFFSET;
        break;
      case DW_CFA_def_cfa_register:
        state->cfaRegister = elfReadLEB128(data, &bytes);
        data += bytes;
        state->cfaMode = CFA_REG_OFFSET;
        break;
      case DW_CFA_def_cfa_offset:
        state->cfaOffset = (s32)elfReadLEB128(data, &bytes);
        data += bytes;
        state->cfaMode = CFA_REG_OFFSET;
        break;
      default:
        printf("Unknown CFA opcode %08x\n", op);
        return;
      }
      break;
    default:
      printf("Unknown CFA opcode %08x\n", op);
      return;
    }
  }
}

ELFFrameState *elfGetFrameState(ELFfde *fde, u32 address)
{
  ELFFrameState *state = (ELFFrameState *)calloc(1, sizeof(ELFFrameState));
  state->pc = fde->address;
  state->dataAlign = fde->cie->dataAlign;
  state->codeAlign = fde->cie->codeAlign;
  state->returnAddress = fde->cie->returnAddress;

  elfExecuteCFAInstructions(state,
                            fde->cie->data,
                            fde->cie->dataLen,
                            0xffffffff);
  elfExecuteCFAInstructions(state,
                            fde->data,
                            fde->dataLen,
                            address);

  return state;
}

void elfPrintCallChain(u32 address)
{
  int count = 1;

  reg_pair regs[15];
  reg_pair newRegs[15];

  memcpy(&regs[0], &reg[0], sizeof(reg_pair) * 15);

  while(count < 20) {
    const char *addr = elfGetAddressSymbol(address);
    if(*addr == 0)
      addr = "???";

    printf("%08x %s\n", address, addr);

    ELFfde *fde = elfGetFde(address);

    if(fde == NULL) {
      break;
    }

    ELFFrameState *state = elfGetFrameState(fde, address);

    if(!state) {
      break;
    }

    if(state->cfaMode == CFA_REG_OFFSET) {
      memcpy(&newRegs[0], &regs[0], sizeof(reg_pair) * 15);
      u32 addr = 0;
      for(int i = 0; i < 15; i++) {
        ELFFrameStateRegister *r = &state->registers.
          regs[i];

        switch(r->mode) {
        case REG_NOT_SET:
          newRegs[i].I = regs[i].I;
          break;
        case REG_OFFSET:
          newRegs[i].I = elfReadMemory(regs[state->cfaRegister].I +
                                       state->cfaOffset +
                                       r->offset);
          break;
        case REG_REGISTER:
          newRegs[i].I = regs[r->reg].I;
          break;
        default:
          printf("Unknown register mode: %d\n", r->mode);
          break;
        }
      }
      memcpy(regs, newRegs, sizeof(reg_pair)*15);
      addr = newRegs[14].I;
      addr &= 0xfffffffe;
      address = addr;
      count++;
    } else {
      printf("CFA not set\n");
      break;
    }
    if(state->registers.previous) {
      ELFFrameStateRegisters *prev = state->registers.previous;

      while(prev) {
        ELFFrameStateRegisters *p = prev->previous;
        free(prev);
        prev = p;
      }
    }
    free(state);
  }
}

u32 elfDecodeLocation(Function *f, ELFBlock *o, LocationType *type, u32 base)
{
  u32 framebase = 0;
  if(f && f->frameBase) {
    ELFBlock *b = f->frameBase;
    switch(*b->data) {
    case DW_OP_reg0:
    case DW_OP_reg1:
    case DW_OP_reg2:
    case DW_OP_reg3:
    case DW_OP_reg4:
    case DW_OP_reg5:
    case DW_OP_reg6:
    case DW_OP_reg7:
    case DW_OP_reg8:
    case DW_OP_reg9:
    case DW_OP_reg10:
    case DW_OP_reg11:
    case DW_OP_reg12:
    case DW_OP_reg13:
    case DW_OP_reg14:
    case DW_OP_reg15:
      framebase = reg[*b->data-0x50].I;
      break;
    default:
      fprintf(stderr, "Unknown frameBase %02x\n", *b->data);
      break;
    }
  }

  ELFBlock *loc = o;
  u32 location = 0;
  int bytes = 0;
  if(loc) {
    switch(*loc->data) {
    case DW_OP_addr:
      location = elfRead4Bytes(loc->data+1);
      *type = LOCATION_memory;
      break;
    case DW_OP_plus_uconst:
      location = base + elfReadLEB128(loc->data+1, &bytes);
      *type = LOCATION_memory;
      break;
    case DW_OP_reg0:
    case DW_OP_reg1:
    case DW_OP_reg2:
    case DW_OP_reg3:
    case DW_OP_reg4:
    case DW_OP_reg5:
    case DW_OP_reg6:
    case DW_OP_reg7:
    case DW_OP_reg8:
    case DW_OP_reg9:
    case DW_OP_reg10:
    case DW_OP_reg11:
    case DW_OP_reg12:
    case DW_OP_reg13:
    case DW_OP_reg14:
    case DW_OP_reg15:
      location = *loc->data - 0x50;
      *type = LOCATION_register;
      break;
    case DW_OP_fbreg:
      {
        int bytes;
        s32 off = elfReadSignedLEB128(loc->data+1, &bytes);
        location = framebase + off;
        *type = LOCATION_memory;
      }
      break;
    default:
      fprintf(stderr, "Unknown location %02x\n", *loc->data);
      break;
    }
  }
  return location;
}

u32 elfDecodeLocation(Function *f, ELFBlock *o, LocationType *type)
{
  return elfDecodeLocation(f, o, type, 0);
}

// reading function

u32 elfRead4Bytes(u8 *data)
{
  u32 value = *data++;
  value |= (*data++ << 8);
  value |= (*data++ << 16);
  value |= (*data << 24);
  return value;
}

u16 elfRead2Bytes(u8 *data)
{
  u16 value = *data++;
  value |= (*data << 8);
  return value;
}

char *elfReadString(u8 *data, int *bytesRead)
{
  if(*data == 0) {
    *bytesRead = 1;
    return NULL;
  }
  *bytesRead = (int)strlen((char *)data) + 1;
  return (char *)data;
}

s32 elfReadSignedLEB128(u8 *data, int *bytesRead)
{
  s32 result = 0;
  int shift = 0;
  int count = 0;

  u8 byte;
  do {
    byte = *data++;
    count++;
    result |= (byte & 0x7f) << shift;
    shift += 7;
  } while(byte & 0x80);
  if((shift < 32) && (byte & 0x40))
    result |= -(1 << shift);
  *bytesRead = count;
  return result;
}

u32 elfReadLEB128(u8 *data, int *bytesRead)
{
  u32 result = 0;
  int shift = 0;
  int count = 0;
  u8 byte;
  do {
    byte = *data++;
    count++;
    result |= (byte & 0x7f) << shift;
    shift += 7;
  } while(byte & 0x80);
  *bytesRead = count;
  return result;
}

u8 *elfReadSection(u8 *data, ELFSectionHeader *sh)
{
  return data + READ32LE(&sh->offset);
}

ELFSectionHeader *elfGetSectionByName(const char *name)
{
  for(int i = 0; i < elfSectionHeadersCount; i++) {
    if(strcmp(name,
              &elfSectionHeadersStringTable[READ32LE(&elfSectionHeaders[i]->
                                                     name)]) == 0) {
      return elfSectionHeaders[i];
    }
  }
  return NULL;
}

ELFSectionHeader *elfGetSectionByNumber(int number)
{
  if(number < elfSectionHeadersCount) {
    return elfSectionHeaders[number];
  }
  return NULL;
}

CompileUnit *elfGetCompileUnitForData(u8 *data)
{
  u8 *end = elfCurrentUnit->top + 4 + elfCurrentUnit->length;

  if(data >= elfCurrentUnit->top && data < end)
    return elfCurrentUnit;

  CompileUnit *unit = elfCompileUnits;

  while(unit) {
    end = unit->top + 4 + unit->length;

    if(data >= unit->top && data < end)
      return unit;

    unit = unit->next;
  }

  printf("Error: cannot find reference to compile unit at offset %08x\n",
         (int)(data - elfDebugInfo->infodata));
  exit(-1);
}

u8 *elfReadAttribute(u8 *data, ELFAttr *attr)
{
  int bytes;
  int form = attr->form;
 start:
  switch(form) {
  case DW_FORM_addr:
    attr->value = elfRead4Bytes(data);
    data += 4;
    break;
  case DW_FORM_data2:
    attr->value = elfRead2Bytes(data);
    data += 2;
    break;
  case DW_FORM_data4:
    attr->value = elfRead4Bytes(data);
    data += 4;
    break;
  case DW_FORM_string:
    attr->string = (char *)data;
    data += strlen(attr->string)+1;
    break;
  case DW_FORM_strp:
    attr->string = elfDebugStrings + elfRead4Bytes(data);
    data += 4;
    break;
  case DW_FORM_block:
    attr->block = (ELFBlock *)malloc(sizeof(ELFBlock));
    attr->block->length = elfReadLEB128(data, &bytes);
    data += bytes;
    attr->block->data = data;
    data += attr->block->length;
    break;
  case DW_FORM_block1:
    attr->block = (ELFBlock *)malloc(sizeof(ELFBlock));
    attr->block->length = *data++;
    attr->block->data = data;
    data += attr->block->length;
    break;
  case DW_FORM_data1:
    attr->value = *data++;
    break;
  case DW_FORM_flag:
    attr->flag = (*data++) ? true : false;
    break;
  case DW_FORM_sdata:
    attr->value = elfReadSignedLEB128(data, &bytes);
    data += bytes;
    break;
  case DW_FORM_udata:
    attr->value = elfReadLEB128(data, &bytes);
    data += bytes;
    break;
  case DW_FORM_ref_addr:
    attr->value = (u32)((elfDebugInfo->infodata + elfRead4Bytes(data)) - elfGetCompileUnitForData(data)->top);
    data += 4;
    break;
  case DW_FORM_ref4:
    attr->value = elfRead4Bytes(data);
    data += 4;
    break;
  case DW_FORM_ref_udata:
    attr->value = (u32)((elfDebugInfo->infodata + (elfGetCompileUnitForData(data)->top - elfDebugInfo->infodata) + elfReadLEB128(data, &bytes)) - elfCurrentUnit->top);
    data += bytes;
    break;
  case DW_FORM_indirect:
    form = elfReadLEB128(data, &bytes);
    data += bytes;
    goto start;
  default:
    fprintf(stderr, "Unsupported FORM %02x\n", form);
    exit(-1);
  }
  return data;
}

ELFAbbrev *elfGetAbbrev(ELFAbbrev **table, u32 number)
{
  int hash = number % 121;

  ELFAbbrev *abbrev = table[hash];

  while(abbrev) {
    if(abbrev->number == number)
      return abbrev;
    abbrev = abbrev->next;
  }
  return NULL;
}

ELFAbbrev **elfReadAbbrevs(u8 *data, u32 offset)
{
  data += offset;
  ELFAbbrev **abbrevs = (ELFAbbrev **)calloc(sizeof(ELFAbbrev *)*121,1);
  int bytes = 0;
  u32 number = elfReadLEB128(data, &bytes);
  data += bytes;
  while(number) {
    ELFAbbrev *abbrev = (ELFAbbrev *)calloc(sizeof(ELFAbbrev),1);

    // read tag information
    abbrev->number = number;
    abbrev->tag = elfReadLEB128(data, &bytes);
    data += bytes;
    abbrev->hasChildren = *data++ ? true: false;

    // read attributes
    int name = elfReadLEB128(data, &bytes);
    data += bytes;
    int form = elfReadLEB128(data, &bytes);
    data += bytes;

    while(name) {
      if((abbrev->numAttrs % 4) == 0) {
        abbrev->attrs = (ELFAttr *)realloc(abbrev->attrs,
                                           (abbrev->numAttrs + 4) *
                                           sizeof(ELFAttr));
      }
      abbrev->attrs[abbrev->numAttrs].name = name;
      abbrev->attrs[abbrev->numAttrs++].form = form;

      name = elfReadLEB128(data, &bytes);
      data += bytes;
      form = elfReadLEB128(data, &bytes);
      data += bytes;
    }

    int hash = number % 121;
    abbrev->next = abbrevs[hash];
    abbrevs[hash] = abbrev;

    number = elfReadLEB128(data, &bytes);
    data += bytes;

    if(elfGetAbbrev(abbrevs, number) != NULL)
      break;
  }

  return abbrevs;
}

void elfParseCFA(u8 *top)
{
  ELFSectionHeader *h = elfGetSectionByName(".debug_frame");

  if(h == NULL) {
    return;
  }

  u8 *data = elfReadSection(top, h);

  u8 *topOffset = data;

  u8 *end = data + READ32LE(&h->size);

  ELFcie *cies = NULL;

  while(data < end) {
    u32 offset = (u32)(data - topOffset);
    u32 len = elfRead4Bytes(data);
    data += 4;

    u8 *dataEnd = data + len;

    u32 id = elfRead4Bytes(data);
    data += 4;

    if(id == 0xffffffff) {
      // skip version
      *data++;

      ELFcie *cie = (ELFcie *)calloc(1, sizeof(ELFcie));

      cie->next = cies;
      cies = cie;

      cie->offset = offset;

      cie->augmentation = data;
      while(*data)
        data++;
      data++;

      if(*cie->augmentation) {
        fprintf(stderr, "Error: augmentation not supported\n");
        exit(-1);
      }

      int bytes;
      cie->codeAlign = elfReadLEB128(data, &bytes);
      data += bytes;

      cie->dataAlign = elfReadSignedLEB128(data, &bytes);
      data += bytes;

      cie->returnAddress = *data++;

      cie->data = data;
      cie->dataLen = (u32)(dataEnd - data);
    } else {
      ELFfde *fde = (ELFfde *)calloc(1, sizeof(ELFfde));

      ELFcie *cie = cies;

      while(cie != NULL) {
        if(cie->offset == id)
          break;
        cie = cie->next;
      }

      if(!cie) {
        fprintf(stderr, "Cannot find CIE %08x\n", id);
        exit(-1);
      }

      fde->cie = cie;

      fde->address = elfRead4Bytes(data);
      data += 4;

      fde->end = fde->address + elfRead4Bytes(data);
      data += 4;

      fde->data = data;
      fde->dataLen = (u32)(dataEnd - data);

      if((elfFdeCount %10) == 0) {
        elfFdes = (ELFfde **)realloc(elfFdes, (elfFdeCount+10) *
                                    sizeof(ELFfde *));
      }
      elfFdes[elfFdeCount++] = fde;
    }
    data = dataEnd;
  }

  elfCies = cies;
}

void elfAddLine(LineInfo *l, u32 a, int file, int line, int *max)
{
  if(l->number == *max) {
    *max += 1000;
    l->lines = (LineInfoItem *)realloc(l->lines, *max*sizeof(LineInfoItem));
  }
  LineInfoItem *li = &l->lines[l->number];
  li->file = l->files[file-1];
  li->address = a;
  li->line = line;
  l->number++;
}

void elfParseLineInfo(CompileUnit *unit, u8 *top)
{
  ELFSectionHeader *h = elfGetSectionByName(".debug_line");
  if(h == NULL) {
    fprintf(stderr, "No line information found\n");
    return;
  }
  LineInfo *l = unit->lineInfoTable = (LineInfo *)calloc(1, sizeof(LineInfo));
  l->number = 0;
  int max = 1000;
  l->lines = (LineInfoItem *)malloc(1000*sizeof(LineInfoItem));

  u8 *data = elfReadSection(top, h);
  data += unit->lineInfo;
  u32 totalLen = elfRead4Bytes(data);
  data += 4;
  u8 *end = data + totalLen;
  //  u16 version = elfRead2Bytes(data);
  data += 2;
  //  u32 offset = elfRead4Bytes(data);
  data += 4;
  int minInstrSize = *data++;
  int defaultIsStmt = *data++;
  int lineBase = (s8)*data++;
  int lineRange = *data++;
  int opcodeBase = *data++;
  u8 *stdOpLen = (u8 *)malloc(opcodeBase * sizeof(u8));
  stdOpLen[0] = 1;
  int i;
  for(i = 1; i < opcodeBase; i++)
    stdOpLen[i] = *data++;

  free(stdOpLen);// todo
  int bytes = 0;

  char *s;
  while((s = elfReadString(data, &bytes)) != NULL) {
    data += bytes;
    //    fprintf(stderr, "Directory is %s\n", s);
  }
  data += bytes;
  int count = 4;
  int index = 0;
  l->files = (char **)malloc(sizeof(char *)*count);

  while((s = elfReadString(data, &bytes)) != NULL) {
    l->files[index++] = s;

    data += bytes;
    // directory
    elfReadLEB128(data, &bytes);
    data += bytes;
    // time
    elfReadLEB128(data, &bytes);
    data += bytes;
    // size
    elfReadLEB128(data, &bytes);
    data += bytes;
    //    fprintf(stderr, "File is %s\n", s);
    if(index == count) {
      count += 4;
      l->files = (char **)realloc(l->files, sizeof(char *)*count);
    }
  }
  l->fileCount = index;
  data += bytes;

  while(data < end) {
    u32 address = 0;
    int file = 1;
    int line = 1;
    int col = 0;
    int isStmt = defaultIsStmt;
    int basicBlock = 0;
    int endSeq = 0;

    while(!endSeq) {
      int op = *data++;
      switch(op) {
      case DW_LNS_extended_op:
        {
          data++;
          op = *data++;
          switch(op) {
          case DW_LNE_end_sequence:
            endSeq = 1;
            break;
          case DW_LNE_set_address:
            address = elfRead4Bytes(data);
            data += 4;
            break;
          default:
            fprintf(stderr, "Unknown extended LINE opcode %02x\n", op);
            exit(-1);
          }
        }
        break;
      case DW_LNS_copy:
        //      fprintf(stderr, "Address %08x line %d (%d)\n", address, line, file);
        elfAddLine(l, address, file, line, &max);
        basicBlock = 0;
        break;
      case DW_LNS_advance_pc:
        address += minInstrSize * elfReadLEB128(data, &bytes);
        data += bytes;
        break;
      case DW_LNS_advance_line:
        line += elfReadSignedLEB128(data, &bytes);
        data += bytes;
        break;
      case DW_LNS_set_file:
        file = elfReadLEB128(data, &bytes);
        data += bytes;
        break;
      case DW_LNS_set_column:
        col = elfReadLEB128(data, &bytes);
        data += bytes;
        break;
      case DW_LNS_negate_stmt:
        isStmt = !isStmt;
        break;
      case DW_LNS_set_basic_block:
        basicBlock = 1;
        break;
      case DW_LNS_const_add_pc:
        address += (minInstrSize *((255 - opcodeBase)/lineRange));
        break;
      case DW_LNS_fixed_advance_pc:
        address += elfRead2Bytes(data);
        data += 2;
        break;
      default:
        op = op - opcodeBase;
        address += (op / lineRange) * minInstrSize;
        line += lineBase + (op % lineRange);
        elfAddLine(l, address, file, line, &max);
        //        fprintf(stderr, "Address %08x line %d (%d)\n", address, line,file);
        basicBlock = 1;
        break;
      }
    }
  }
  l->lines = (LineInfoItem *)realloc(l->lines, l->number*sizeof(LineInfoItem));
}

u8 *elfSkipData(u8 *data, ELFAbbrev *abbrev, ELFAbbrev **abbrevs)
{
  int i;
  int bytes;

  for(i = 0; i < abbrev->numAttrs; i++) {
    data = elfReadAttribute(data,  &abbrev->attrs[i]);
    if(abbrev->attrs[i].form == DW_FORM_block1)
      free(abbrev->attrs[i].block);
  }

  if(abbrev->hasChildren) {
    int nesting = 1;
    while(nesting) {
      u32 abbrevNum = elfReadLEB128(data, &bytes);
      data += bytes;

      if(!abbrevNum) {
        nesting--;
        continue;
      }

      abbrev = elfGetAbbrev(abbrevs, abbrevNum);

      for(i = 0; i < abbrev->numAttrs; i++) {
        data = elfReadAttribute(data,  &abbrev->attrs[i]);
        if(abbrev->attrs[i].form == DW_FORM_block1)
          free(abbrev->attrs[i].block);
      }

      if(abbrev->hasChildren) {
        nesting++;
      }
    }
  }
  return data;
}

Type *elfParseType(CompileUnit *unit, u32);
u8 *elfParseObject(u8 *data, ELFAbbrev *abbrev, CompileUnit *unit,
                   Object **object);
u8 *elfParseFunction(u8 *data, ELFAbbrev *abbrev, CompileUnit *unit,
                     Function **function);
void elfCleanUp(Function *);

void elfAddType(Type *type, CompileUnit *unit, u32 offset)
{
  if(type->next == NULL) {
    if(unit->types != type && type->offset == 0) {
      type->offset = offset;
      type->next = unit->types;
      unit->types = type;
    }
  }
}

void elfParseType(u8 *data, u32 offset, ELFAbbrev *abbrev, CompileUnit *unit,
                  Type **type)
{
  switch(abbrev->tag) {
  case DW_TAG_typedef:
    {
      u32 typeref = 0;
      char *name = NULL;
      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data = elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_name:
          name = attr->string;
          break;
        case DW_AT_type:
          typeref = attr->value;
          break;
        case DW_AT_decl_file:
        case DW_AT_decl_line:
          break;
        default:
          fprintf(stderr, "Unknown attribute for typedef %02x\n", attr->name);
          break;
        }
      }
      if(abbrev->hasChildren)
        fprintf(stderr, "Unexpected children for typedef\n");
      *type = elfParseType(unit, typeref);
      if(name)
        (*type)->name = name;
      return;
    }
    break;
  case DW_TAG_union_type:
  case DW_TAG_structure_type:
    {
      Type *t = (Type *)calloc(sizeof(Type), 1);
      if(abbrev->tag == DW_TAG_structure_type)
        t->type = TYPE_struct;
      else
        t->type = TYPE_union;

      Struct *s = (Struct *)calloc(sizeof(Struct), 1);
      t->structure = s;
      elfAddType(t, unit, offset);

      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data = elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_name:
          t->name = attr->string;
          break;
        case DW_AT_byte_size:
          t->size = attr->value;
          break;
        case DW_AT_decl_file:
        case DW_AT_decl_line:
        case DW_AT_sibling:
        case DW_AT_containing_type: // todo?
        case DW_AT_declaration:
  case DW_AT_specification: // TODO:
          break;
        default:
          fprintf(stderr, "Unknown attribute for struct %02x\n", attr->name);
          break;
        }
      }
      if(abbrev->hasChildren) {
        int bytes;
        u32 num = elfReadLEB128(data, &bytes);
        data += bytes;
        int index = 0;
        while(num) {
          ELFAbbrev *abbr = elfGetAbbrev(unit->abbrevs, num);

          switch(abbr->tag) {
          case DW_TAG_member:
            {
              if((index % 4) == 0)
                s->members = (Member *)realloc(s->members,
                                               sizeof(Member)*(index+4));
              Member *m = &s->members[index];
              m->location = NULL;
              m->bitOffset = 0;
              m->bitSize = 0;
              m->byteSize = 0;
              for(int i = 0; i < abbr->numAttrs; i++) {
                ELFAttr *attr = &abbr->attrs[i];
                data = elfReadAttribute(data, attr);
                switch(attr->name) {
                case DW_AT_name:
                  m->name = attr->string;
                  break;
                case DW_AT_type:
                  m->type = elfParseType(unit, attr->value);
                  break;
                case DW_AT_data_member_location:
                  m->location = attr->block;
                  break;
                case DW_AT_byte_size:
                  m->byteSize = attr->value;
                  break;
                case DW_AT_bit_offset:
                  m->bitOffset = attr->value;
                  break;
                case DW_AT_bit_size:
                  m->bitSize = attr->value;
                  break;
                case DW_AT_decl_file:
                case DW_AT_decl_line:
                case DW_AT_accessibility:
                case DW_AT_artificial: // todo?
                  break;
                default:
                  fprintf(stderr, "Unknown member attribute %02x\n",
                          attr->name);
                }
              }
              index++;
            }
            break;
          case DW_TAG_subprogram:
            {
              Function *fnc = NULL;
              data = elfParseFunction(data, abbr, unit, &fnc);
              if(fnc != NULL) {
                if(unit->lastFunction)
                  unit->lastFunction->next = fnc;
                else
                  unit->functions = fnc;
                unit->lastFunction = fnc;
              }
            }
            break;
          case DW_TAG_inheritance:
            // TODO: add support
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          CASE_TYPE_TAG:
            // skip types... parsed only when used
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          case DW_TAG_variable:
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          default:
            fprintf(stderr, "Unknown struct tag %02x %s\n", abbr->tag, t->name);
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          }
          num = elfReadLEB128(data, &bytes);
          data += bytes;
        }
        s->memberCount = index;
      }
      *type = t;
      return;
    }
    break;
  case DW_TAG_base_type:
    {
      Type *t = (Type *)calloc(sizeof(Type), 1);

      t->type = TYPE_base;
      elfAddType(t, unit, offset);
      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data = elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_name:
          t->name = attr->string;
          break;
        case DW_AT_encoding:
          t->encoding = attr->value;
          break;
        case DW_AT_byte_size:
          t->size = attr->value;
          break;
        case DW_AT_bit_size:
          t->bitSize = attr->value;
          break;
        default:
          fprintf(stderr, "Unknown attribute for base type %02x\n",
                  attr->name);
          break;
        }
      }
      if(abbrev->hasChildren)
        fprintf(stderr, "Unexpected children for base type\n");
      *type = t;
      return;
    }
    break;
  case DW_TAG_pointer_type:
    {
      Type *t = (Type *)calloc(sizeof(Type), 1);

      t->type = TYPE_pointer;

      elfAddType(t, unit, offset);

      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data =elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_type:
          t->pointer = elfParseType(unit, attr->value);
          break;
        case DW_AT_byte_size:
          t->size = attr->value;
          break;
        default:
          fprintf(stderr, "Unknown pointer type attribute %02x\n", attr->name);
          break;
        }
      }
      if(abbrev->hasChildren)
        fprintf(stderr, "Unexpected children for pointer type\n");
      *type = t;
      return;
    }
    break;
  case DW_TAG_reference_type:
    {
      Type *t = (Type *)calloc(sizeof(Type), 1);

      t->type = TYPE_reference;

      elfAddType(t, unit, offset);

      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data =elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_type:
          t->pointer = elfParseType(unit, attr->value);
          break;
        case DW_AT_byte_size:
          t->size = attr->value;
          break;
        default:
          fprintf(stderr, "Unknown ref type attribute %02x\n", attr->name);
          break;
        }
      }
      if(abbrev->hasChildren)
        fprintf(stderr, "Unexpected children for ref type\n");
      *type = t;
      return;
    }
    break;
  case DW_TAG_volatile_type:
    {
      u32 typeref = 0;

      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data = elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_type:
          typeref = attr->value;
          break;
        default:
          fprintf(stderr, "Unknown volatile attribute for type %02x\n",
                  attr->name);
          break;
        }
      }
      if(abbrev->hasChildren)
        fprintf(stderr, "Unexpected children for volatile type\n");
      *type = elfParseType(unit, typeref);
      return;
    }
    break;
  case DW_TAG_const_type:
    {
      u32 typeref = 0;

      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data = elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_type:
          typeref = attr->value;
          break;
        default:
          fprintf(stderr, "Unknown const attribute for type %02x\n",
                  attr->name);
          break;
        }
      }
      if(abbrev->hasChildren)
        fprintf(stderr, "Unexpected children for const type\n");
      *type = elfParseType(unit, typeref);
      return;
    }
    break;
  case DW_TAG_enumeration_type:
    {
      Type *t = (Type *)calloc(sizeof(Type), 1);
      t->type = TYPE_enum;
      Enum *e = (Enum *)calloc(sizeof(Enum), 1);
      t->enumeration = e;
      elfAddType(t, unit, offset);
      int count = 0;
      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data = elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_name:
          t->name = attr->string;
          break;
        case DW_AT_byte_size:
          t->size = attr->value;
          break;
        case DW_AT_sibling:
        case DW_AT_decl_file:
        case DW_AT_decl_line:
          break;
        default:
          fprintf(stderr, "Unknown enum attribute %02x\n", attr->name);
        }
      }
      if(abbrev->hasChildren) {
        int bytes;
        u32 num = elfReadLEB128(data, &bytes);
        data += bytes;
        while(num) {
          ELFAbbrev *abbr = elfGetAbbrev(unit->abbrevs, num);

          switch(abbr->tag) {
          case DW_TAG_enumerator:
            {
              count++;
              e->members = (EnumMember *)realloc(e->members,
                                                 count*sizeof(EnumMember));
              EnumMember *m = &e->members[count-1];
              for(int i = 0; i < abbr->numAttrs; i++) {
                ELFAttr *attr = &abbr->attrs[i];
                data = elfReadAttribute(data, attr);
                switch(attr->name) {
                case DW_AT_name:
                  m->name = attr->string;
                  break;
                case DW_AT_const_value:
                  m->value = attr->value;
                  break;
                default:
                  fprintf(stderr, "Unknown sub param attribute %02x\n",
                          attr->name);
                }
              }
            }
            break;
          default:
            fprintf(stderr, "Unknown enum tag %02x\n", abbr->tag);
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          }
          num = elfReadLEB128(data, &bytes);
          data += bytes;
        }
      }
      e->count = count;
      *type = t;
      return;
    }
    break;
  case DW_TAG_subroutine_type:
    {
      Type *t = (Type *)calloc(sizeof(Type), 1);
      t->type = TYPE_function;
      FunctionType *f = (FunctionType *)calloc(sizeof(FunctionType), 1);
      t->function = f;
      elfAddType(t, unit, offset);
      for(int i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data = elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_prototyped:
        case DW_AT_sibling:
          break;
        case DW_AT_type:
          f->returnType = elfParseType(unit, attr->value);
          break;
        default:
          fprintf(stderr, "Unknown subroutine attribute %02x\n", attr->name);
        }
      }
      if(abbrev->hasChildren) {
        int bytes;
        u32 num = elfReadLEB128(data, &bytes);
        data += bytes;
        Object *lastVar = NULL;
        while(num) {
          ELFAbbrev *abbr = elfGetAbbrev(unit->abbrevs, num);

          switch(abbr->tag) {
          case DW_TAG_formal_parameter:
            {
              Object *o;
              data = elfParseObject(data, abbr, unit, &o);
              if(f->args)
                lastVar->next = o;
              else
                f->args = o;
              lastVar = o;
            }
            break;
          case DW_TAG_unspecified_parameters:
            // no use in the debugger yet
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          CASE_TYPE_TAG:
            // skip types... parsed only when used
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          default:
            fprintf(stderr, "Unknown subroutine tag %02x\n", abbr->tag);
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          }
          num = elfReadLEB128(data, &bytes);
          data += bytes;
        }
      }
      *type = t;
      return;
    }
    break;
  case DW_TAG_array_type:
    {
      u32 typeref = 0;
      int i;
      Array *array = (Array *)calloc(sizeof(Array), 1);
      Type *t = (Type *)calloc(sizeof(Type), 1);
      t->type = TYPE_array;
      elfAddType(t, unit, offset);

      for(i = 0; i < abbrev->numAttrs; i++) {
        ELFAttr *attr = &abbrev->attrs[i];
        data = elfReadAttribute(data, attr);
        switch(attr->name) {
        case DW_AT_sibling:
          break;
        case DW_AT_type:
          typeref = attr->value;
          array->type = elfParseType(unit, typeref);
          break;
        default:
          fprintf(stderr, "Unknown array attribute %02x\n", attr->name);
        }
      }
      if(abbrev->hasChildren) {
        int bytes;
        u32 num = elfReadLEB128(data, &bytes);
        data += bytes;
        int index = 0;
        int maxBounds = 0;
        while(num) {
          ELFAbbrev *abbr = elfGetAbbrev(unit->abbrevs, num);

          switch(abbr->tag) {
          case DW_TAG_subrange_type:
            {
              if(maxBounds == index) {
                maxBounds += 4;
                array->bounds = (int *)realloc(array->bounds,
                                               sizeof(int)*maxBounds);
              }
              for(int i = 0; i < abbr->numAttrs; i++) {
                ELFAttr *attr = &abbr->attrs[i];
                data = elfReadAttribute(data, attr);
                switch(attr->name) {
                case DW_AT_upper_bound:
                  array->bounds[index] = attr->value+1;
                  break;
                case DW_AT_type: // ignore
                  break;
                default:
                  fprintf(stderr, "Unknown subrange attribute %02x\n",
                          attr->name);
                }
              }
              index++;
            }
            break;
          default:
            fprintf(stderr, "Unknown array tag %02x\n", abbr->tag);
            data = elfSkipData(data, abbr, unit->abbrevs);
            break;
          }
          num = elfReadLEB128(data, &bytes);
          data += bytes;
        }
        array->maxBounds = index;
      }
      t->size = array->type->size;
      for(i = 0; i < array->maxBounds; i++)
        t->size *= array->bounds[i];
      t->array = array;
      *type = t;
      return;
    }
    break;
  default:
    fprintf(stderr, "Unknown type TAG %02x\n", abbrev->tag);
    exit(-1);
  }
}

Type *elfParseType(CompileUnit *unit, u32 offset)
{
  Type *t = unit->types;

  while(t) {
    if(t->offset == offset)
      return t;
    t = t->next;
  }
  if(offset == 0) {
    Type *t = (Type *)calloc(sizeof(Type), 1);
    t->type = TYPE_void;
    t->offset = 0;
    elfAddType(t, unit, 0);
    return t;
  }
  u8 *data = unit->top + offset;
  int bytes;
  int abbrevNum = elfReadLEB128(data, &bytes);
  data += bytes;
  Type *type = NULL;

  ELFAbbrev *abbrev = elfGetAbbrev(unit->abbrevs, abbrevNum);

  elfParseType(data, offset, abbrev, unit, &type);
  return type;
}

void elfGetObjectAttributes(CompileUnit *unit, u32 offset, Object *o)
{
  u8 *data = unit->top + offset;
  int bytes;
  u32 abbrevNum = elfReadLEB128(data, &bytes);
  data += bytes;

  if(!abbrevNum) {
    return;
  }

  ELFAbbrev *abbrev = elfGetAbbrev(unit->abbrevs, abbrevNum);

  for(int i = 0; i < abbrev->numAttrs; i++) {
    ELFAttr *attr = &abbrev->attrs[i];
    data = elfReadAttribute(data, attr);
    switch(attr->name) {
    case DW_AT_location:
      o->location = attr->block;
      break;
    case DW_AT_name:
      if(o->name == NULL)
        o->name = attr->string;
      break;
    case DW_AT_MIPS_linkage_name:
      o->name = attr->string;
      break;
    case DW_AT_decl_file:
      o->file = attr->value;
      break;
    case DW_AT_decl_line:
      o->line = attr->value;
      break;
    case DW_AT_type:
      o->type = elfParseType(unit, attr->value);
      break;
    case DW_AT_external:
      o->external = attr->flag;
      break;
    case DW_AT_const_value:
    case DW_AT_abstract_origin:
    case DW_AT_declaration:
    case DW_AT_artificial:
      // todo
      break;
    case DW_AT_specification:
      // TODO:
      break;
    default:
      fprintf(stderr, "Unknown object attribute %02x\n", attr->name);
      break;
    }
  }
}

u8 *elfParseObject(u8 *data, ELFAbbrev *abbrev, CompileUnit *unit,
                   Object **object)
{
  Object *o = (Object *)calloc(sizeof(Object), 1);

  o->next = NULL;

  for(int i = 0; i < abbrev->numAttrs; i++) {
    ELFAttr *attr = &abbrev->attrs[i];
    data = elfReadAttribute(data, attr);
    switch(attr->name) {
    case DW_AT_location:
      o->location = attr->block;
      break;
    case DW_AT_name:
      if(o->name == NULL)
        o->name = attr->string;
      break;
    case DW_AT_MIPS_linkage_name:
      o->name = attr->string;
      break;
    case DW_AT_decl_file:
      o->file = attr->value;
      break;
    case DW_AT_decl_line:
      o->line = attr->value;
      break;
    case DW_AT_type:
      o->type = elfParseType(unit, attr->value);
      break;
    case DW_AT_external:
      o->external = attr->flag;
      break;
    case DW_AT_abstract_origin:
      elfGetObjectAttributes(unit, attr->value, o);
      break;
    case DW_AT_const_value:
    case DW_AT_declaration:
    case DW_AT_artificial:
      break;
    case DW_AT_specification:
      // TODO:
      break;
    default:
      fprintf(stderr, "Unknown object attribute %02x\n", attr->name);
      break;
    }
  }
  *object = o;
  return data;
}

u8 *elfParseBlock(u8 *data, ELFAbbrev *abbrev, CompileUnit *unit,
                  Function *func, Object **lastVar)
{
  int bytes;
  u32 start = func->lowPC;
  u32 end = func->highPC;

  for(int i = 0; i < abbrev->numAttrs; i++) {
    ELFAttr *attr = &abbrev->attrs[i];
    data = elfReadAttribute(data, attr);
    switch(attr->name) {
    case DW_AT_sibling:
      break;
    case DW_AT_low_pc:
      start = attr->value;
      break;
    case DW_AT_high_pc:
      end = attr->value;
      break;
    case DW_AT_ranges: // ignore for now
      break;
    default:
      fprintf(stderr, "Unknown block attribute %02x\n", attr->name);
      break;
    }
  }

  if(abbrev->hasChildren) {
    int nesting = 1;

    while(nesting) {
      u32 abbrevNum = elfReadLEB128(data, &bytes);
      data += bytes;

      if(!abbrevNum) {
        nesting--;
        continue;
      }

      abbrev = elfGetAbbrev(unit->abbrevs, abbrevNum);

      switch(abbrev->tag) {
      CASE_TYPE_TAG: // types only parsed when used
      case DW_TAG_label: // not needed
        data = elfSkipData(data, abbrev, unit->abbrevs);
        break;
      case DW_TAG_lexical_block:
        data = elfParseBlock(data, abbrev, unit, func, lastVar);
        break;
      case DW_TAG_subprogram:
        {
          Function *f = NULL;
          data = elfParseFunction(data, abbrev, unit, &f);
          if(f != NULL) {
            if(unit->lastFunction)
              unit->lastFunction->next = f;
            else
              unit->functions = f;
            unit->lastFunction = f;
          }
        }
        break;
      case DW_TAG_variable:
        {
          Object *o;
          data = elfParseObject(data, abbrev, unit, &o);
          if(o->startScope == 0)
            o->startScope = start;
          if(o->endScope == 0)
            o->endScope = 0;
          if(func->variables)
            (*lastVar)->next = o;
          else
            func->variables = o;
          *lastVar = o;
        }
        break;
      case DW_TAG_inlined_subroutine:
        // TODO:
        data = elfSkipData(data, abbrev, unit->abbrevs);
        break;
      default:
        {
          fprintf(stderr, "Unknown block TAG %02x\n", abbrev->tag);
          data = elfSkipData(data, abbrev, unit->abbrevs);
        }
        break;
      }
    }
  }
  return data;
}

void elfGetFunctionAttributes(CompileUnit *unit, u32 offset, Function *func)
{
  u8 *data = unit->top + offset;
  int bytes;
  u32 abbrevNum = elfReadLEB128(data, &bytes);
  data += bytes;

  if(!abbrevNum) {
    return;
  }

  ELFAbbrev *abbrev = elfGetAbbrev(unit->abbrevs, abbrevNum);

  for(int i = 0; i < abbrev->numAttrs; i++) {
    ELFAttr *attr = &abbrev->attrs[i];
    data = elfReadAttribute(data, attr);

    switch(attr->name) {
    case DW_AT_sibling:
      break;
    case DW_AT_name:
      if(func->name == NULL)
        func->name = attr->string;
      break;
    case DW_AT_MIPS_linkage_name:
      func->name = attr->string;
      break;
    case DW_AT_low_pc:
      func->lowPC = attr->value;
      break;
    case DW_AT_high_pc:
      func->highPC = attr->value;
      break;
    case DW_AT_decl_file:
      func->file = attr->value;
      break;
    case DW_AT_decl_line:
      func->line = attr->value;
      break;
    case DW_AT_external:
      func->external = attr->flag;
      break;
    case DW_AT_frame_base:
      func->frameBase = attr->block;
      break;
    case DW_AT_type:
      func->returnType = elfParseType(unit, attr->value);
      break;
    case DW_AT_inline:
    case DW_AT_specification:
    case DW_AT_declaration:
    case DW_AT_artificial:
    case DW_AT_prototyped:
    case DW_AT_proc_body:
    case DW_AT_save_offset:
    case DW_AT_user_2002:
    case DW_AT_virtuality:
    case DW_AT_containing_type:
    case DW_AT_accessibility:
      // todo;
      break;
    case DW_AT_vtable_elem_location:
      free(attr->block);
      break;
    default:
      fprintf(stderr, "Unknown function attribute %02x\n", attr->name);
      break;
    }
  }

  return;
}

u8 *elfParseFunction(u8 *data, ELFAbbrev *abbrev, CompileUnit *unit,
                     Function **f)
{
  Function *func = (Function *)calloc(sizeof(Function), 1);
  *f = func;

  int bytes;
  bool mangled = false;
  bool declaration = false;
  for(int i = 0; i < abbrev->numAttrs; i++) {
    ELFAttr *attr = &abbrev->attrs[i];
    data = elfReadAttribute(data, attr);
    switch(attr->name) {
    case DW_AT_sibling:
      break;
    case DW_AT_name:
      if(func->name == NULL)
        func->name = attr->string;
      break;
    case DW_AT_MIPS_linkage_name:
      func->name = attr->string;
      mangled = true;
      break;
    case DW_AT_low_pc:
      func->lowPC = attr->value;
      break;
    case DW_AT_high_pc:
      func->highPC = attr->value;
      break;
    case DW_AT_prototyped:
      break;
    case DW_AT_decl_file:
      func->file = attr->value;
      break;
    case DW_AT_decl_line:
      func->line = attr->value;
      break;
    case DW_AT_external:
      func->external = attr->flag;
      break;
    case DW_AT_frame_base:
      func->frameBase = attr->block;
      break;
    case DW_AT_type:
      func->returnType = elfParseType(unit, attr->value);
      break;
    case DW_AT_abstract_origin:
      elfGetFunctionAttributes(unit, attr->value, func);
      break;
    case DW_AT_declaration:
      declaration = attr->flag;
      break;
    case DW_AT_inline:
    case DW_AT_specification:
    case DW_AT_artificial:
    case DW_AT_proc_body:
    case DW_AT_save_offset:
    case DW_AT_user_2002:
    case DW_AT_virtuality:
    case DW_AT_containing_type:
    case DW_AT_accessibility:
      // todo;
      break;
    case DW_AT_vtable_elem_location:
      free(attr->block);
      break;
    default:
      fprintf(stderr, "Unknown function attribute %02x\n", attr->name);
      break;
    }
  }

  if(declaration) {
    elfCleanUp(func);
    free(func);
    *f = NULL;

    while(1) {
      u32 abbrevNum = elfReadLEB128(data, &bytes);
      data += bytes;

      if(!abbrevNum) {
        return data;
      }

      abbrev = elfGetAbbrev(unit->abbrevs, abbrevNum);

      data = elfSkipData(data, abbrev, unit->abbrevs);
    }
  }

  if(abbrev->hasChildren) {
    int nesting = 1;
    Object *lastParam = NULL;
    Object *lastVar = NULL;

    while(nesting) {
      u32 abbrevNum = elfReadLEB128(data, &bytes);
      data += bytes;

      if(!abbrevNum) {
        nesting--;
        continue;
      }

      abbrev = elfGetAbbrev(unit->abbrevs, abbrevNum);

      switch(abbrev->tag) {
      CASE_TYPE_TAG: // no need to parse types. only parsed when used
      case DW_TAG_label: // not needed
        data = elfSkipData(data, abbrev, unit->abbrevs);
        break;
      case DW_TAG_subprogram:
        {
          Function *fnc=NULL;
          data = elfParseFunction(data, abbrev, unit, &fnc);
          if(fnc != NULL) {
            if(unit->lastFunction == NULL)
              unit->functions = fnc;
            else
              unit->lastFunction->next = fnc;
            unit->lastFunction = fnc;
          }
        }
        break;
      case DW_TAG_lexical_block:
        {
          data = elfParseBlock(data, abbrev, unit, func, &lastVar);
        }
        break;
      case DW_TAG_formal_parameter:
        {
          Object *o;
          data = elfParseObject(data, abbrev, unit, &o);
          if(func->parameters)
            lastParam->next = o;
          else
            func->parameters = o;
          lastParam = o;
        }
        break;
      case DW_TAG_variable:
        {
          Object *o;
          data = elfParseObject(data, abbrev, unit, &o);
          if(func->variables)
            lastVar->next = o;
          else
            func->variables = o;
          lastVar = o;
        }
        break;
      case DW_TAG_unspecified_parameters:
      case DW_TAG_inlined_subroutine:
        {
          // todo
          for(int i = 0; i < abbrev->numAttrs; i++) {
            data = elfReadAttribute(data,  &abbrev->attrs[i]);
            if(abbrev->attrs[i].form == DW_FORM_block1)
              free(abbrev->attrs[i].block);
          }

          if(abbrev->hasChildren)
            nesting++;
        }
        break;
      default:
        {
          fprintf(stderr, "Unknown function TAG %02x\n", abbrev->tag);
          data = elfSkipData(data, abbrev, unit->abbrevs);
        }
        break;
      }
    }
  }
  return data;
}

u8 *elfParseUnknownData(u8 *data, ELFAbbrev *abbrev, ELFAbbrev **abbrevs)
{
  int i;
  int bytes;
  //  switch(abbrev->tag) {
  //  default:
    fprintf(stderr, "Unknown TAG %02x\n", abbrev->tag);

    for(i = 0; i < abbrev->numAttrs; i++) {
      data = elfReadAttribute(data,  &abbrev->attrs[i]);
      if(abbrev->attrs[i].form == DW_FORM_block1)
        free(abbrev->attrs[i].block);
    }

    if(abbrev->hasChildren) {
      int nesting = 1;
      while(nesting) {
        u32 abbrevNum = elfReadLEB128(data, &bytes);
        data += bytes;

        if(!abbrevNum) {
          nesting--;
          continue;
        }

        abbrev = elfGetAbbrev(abbrevs, abbrevNum);

        fprintf(stderr, "Unknown TAG %02x\n", abbrev->tag);

        for(i = 0; i < abbrev->numAttrs; i++) {
          data = elfReadAttribute(data,  &abbrev->attrs[i]);
          if(abbrev->attrs[i].form == DW_FORM_block1)
            free(abbrev->attrs[i].block);
        }

        if(abbrev->hasChildren) {
          nesting++;
        }
      }
    }
    //  }
  return data;
}

u8 *elfParseCompileUnitChildren(u8 *data, CompileUnit *unit)
{
  int bytes;
  u32 abbrevNum = elfReadLEB128(data, &bytes);
  data += bytes;
  Object *lastObj = NULL;
  while(abbrevNum) {
    ELFAbbrev *abbrev = elfGetAbbrev(unit->abbrevs, abbrevNum);
    switch(abbrev->tag) {
    case DW_TAG_subprogram:
      {
        Function *func = NULL;
        data = elfParseFunction(data, abbrev, unit, &func);
        if(func != NULL) {
          if(unit->lastFunction)
            unit->lastFunction->next = func;
          else
            unit->functions = func;
          unit->lastFunction = func;
        }
      }
      break;
    CASE_TYPE_TAG:
      data = elfSkipData(data, abbrev, unit->abbrevs);
      break;
    case DW_TAG_variable:
      {
        Object *var = NULL;
        data = elfParseObject(data, abbrev, unit, &var);
        if(lastObj)
          lastObj->next = var;
        else
          unit->variables = var;
        lastObj = var;
      }
      break;
    default:
      data = elfParseUnknownData(data, abbrev, unit->abbrevs);
      break;
    }

    abbrevNum = elfReadLEB128(data, &bytes);
    data += bytes;
  }
  return data;
}


CompileUnit *elfParseCompUnit(u8 *data, u8 *abbrevData)
{
  int bytes;
  u8 *top = data;

  u32 length = elfRead4Bytes(data);
  data += 4;

  u16 version = elfRead2Bytes(data);
  data += 2;

  u32 offset = elfRead4Bytes(data);
  data += 4;

  u8 addrSize = *data++;

  if(version != 2) {
    fprintf(stderr, "Unsupported debugging information version %d\n", version);
    return NULL;
  }

  if(addrSize != 4) {
    fprintf(stderr, "Unsupported address size %d\n", addrSize);
    return NULL;
  }

  ELFAbbrev **abbrevs = elfReadAbbrevs(abbrevData, offset);

  u32 abbrevNum = elfReadLEB128(data, &bytes);
  data += bytes;

  ELFAbbrev *abbrev = elfGetAbbrev(abbrevs, abbrevNum);

  CompileUnit *unit = (CompileUnit *)calloc(sizeof(CompileUnit), 1);
  unit->top = top;
  unit->length = length;
  unit->abbrevs = abbrevs;
  unit->next = NULL;

  elfCurrentUnit = unit;

  int i;

  for(i = 0; i < abbrev->numAttrs; i++) {
    ELFAttr *attr = &abbrev->attrs[i];
    data = elfReadAttribute(data, attr);

    switch(attr->name) {
    case DW_AT_name:
      unit->name = attr->string;
      break;
    case DW_AT_stmt_list:
      unit->hasLineInfo = true;
      unit->lineInfo = attr->value;
      break;
    case DW_AT_low_pc:
      unit->lowPC = attr->value;
      break;
    case DW_AT_high_pc:
      unit->highPC = attr->value;
      break;
    case DW_AT_compdir:
      unit->compdir = attr->string;
      break;
      // ignore
    case DW_AT_language:
    case DW_AT_producer:
    case DW_AT_macro_info:
    case DW_AT_entry_pc:
      break;
    default:
      fprintf(stderr, "Unknown attribute %02x\n", attr->name);
      break;
    }
  }

  if(abbrev->hasChildren)
    elfParseCompileUnitChildren(data, unit);

  return unit;
}

void elfParseAranges(u8 *data)
{
  ELFSectionHeader *sh = elfGetSectionByName(".debug_aranges");
  if(sh == NULL) {
    fprintf(stderr, "No aranges found\n");
    return;
  }

  data = elfReadSection(data, sh);
  u8 *end = data + READ32LE(&sh->size);

  int max = 4;
  ARanges *ranges = (ARanges *)calloc(sizeof(ARanges), 4);

  int index = 0;

  while(data < end) {
    u32 len = elfRead4Bytes(data);
    data += 4;
    //    u16 version = elfRead2Bytes(data);
    data += 2;
    u32 offset = elfRead4Bytes(data);
    data += 4;
    //    u8 addrSize = *data++;
    //    u8 segSize = *data++;
    data += 2; // remove if uncommenting above
    data += 4;
    ranges[index].count = (len-20)/8;
    ranges[index].offset = offset;
    ranges[index].ranges = (ARange *)calloc(sizeof(ARange), (len-20)/8);
    int i = 0;
    while(true) {
      u32 addr = elfRead4Bytes(data);
      data += 4;
      u32 len = elfRead4Bytes(data);
      data += 4;
      if(addr == 0 && len == 0)
        break;
      ranges[index].ranges[i].lowPC = addr;
      ranges[index].ranges[i].highPC = addr+len;
      i++;
    }
    index++;
    if(index == max) {
      max += 4;
      ranges = (ARanges *)realloc(ranges, max*sizeof(ARanges));
    }
  }
  elfDebugInfo->numRanges = index;
  elfDebugInfo->ranges = ranges;
}

void elfReadSymtab(u8 *data)
{
  ELFSectionHeader *sh = elfGetSectionByName(".symtab");
  int table = READ32LE(&sh->link);

  char *strtable = (char *)elfReadSection(data, elfGetSectionByNumber(table));

  ELFSymbol *symtab = (ELFSymbol *)elfReadSection(data, sh);

  int count = READ32LE(&sh->size) / sizeof(ELFSymbol);
  elfSymbolsCount = 0;

  elfSymbols = (Symbol *)malloc(sizeof(Symbol)*count);

  int i;

  for(i = 0; i < count; i++) {
    ELFSymbol *s = &symtab[i];
    int type = s->info & 15;
    int binding = s->info >> 4;

    if(binding) {
      Symbol *sym = &elfSymbols[elfSymbolsCount];
      sym->name = &strtable[READ32LE(&s->name)];
      sym->binding = binding;
      sym->type = type;
      sym->value = READ32LE(&s->value);
      sym->size = READ32LE(&s->size);
      elfSymbolsCount++;
    }
  }
  for(i = 0; i < count; i++) {
    ELFSymbol *s = &symtab[i];
    int bind = s->info>>4;
    int type = s->info & 15;

    if(!bind) {
      Symbol *sym = &elfSymbols[elfSymbolsCount];
      sym->name = &strtable[READ32LE(&s->name)];
      sym->binding = (s->info >> 4);
      sym->type = type;
      sym->value = READ32LE(&s->value);
      sym->size = READ32LE(&s->size);
      elfSymbolsCount++;
    }
  }
  elfSymbolsStrTab = strtable;
  //  free(symtab);
}

bool elfReadProgram(ELFHeader *eh, u8 *data, int& size, bool parseDebug)
{
  int count = READ16LE(&eh->e_phnum);
  int i;

  if(READ32LE(&eh->e_entry) == 0x2000000)
    cpuIsMultiBoot = true;

  // read program headers... should probably move this code down
  u8 *p = data + READ32LE(&eh->e_phoff);
  size = 0;
  for(i = 0; i < count; i++) {
    ELFProgramHeader *ph = (ELFProgramHeader *)p;
    p += sizeof(ELFProgramHeader);
    if(READ16LE(&eh->e_phentsize) != sizeof(ELFProgramHeader)) {
      p += READ16LE(&eh->e_phentsize) - sizeof(ELFProgramHeader);
    }

    //    printf("PH %d %08x %08x %08x %08x %08x %08x %08x %08x\n",
    //     i, ph->type, ph->offset, ph->vaddr, ph->paddr,
    //     ph->filesz, ph->memsz, ph->flags, ph->align);
    if(cpuIsMultiBoot) {
      if(READ32LE(&ph->paddr) >= 0x2000000 &&
         READ32LE(&ph->paddr) <= 0x203ffff) {
        memcpy(&workRAM[READ32LE(&ph->paddr) & 0x3ffff],
               data + READ32LE(&ph->offset),
               READ32LE(&ph->filesz));
        size += READ32LE(&ph->filesz);
      }
    } else {
      if(READ32LE(&ph->paddr) >= 0x8000000 &&
         READ32LE(&ph->paddr) <= 0x9ffffff) {
        memcpy(&rom[READ32LE(&ph->paddr) & 0x1ffffff],
               data + READ32LE(&ph->offset),
               READ32LE(&ph->filesz));
        size += READ32LE(&ph->filesz);
      }
    }
  }

  char *stringTable = NULL;

  // read section headers
  p = data + READ32LE(&eh->e_shoff);
  count = READ16LE(&eh->e_shnum);

  ELFSectionHeader **sh = (ELFSectionHeader **)
    malloc(sizeof(ELFSectionHeader *) * count);

  for(i = 0; i < count; i++) {
    sh[i] = (ELFSectionHeader *)p;
    p += sizeof(ELFSectionHeader);
    if(READ16LE(&eh->e_shentsize) != sizeof(ELFSectionHeader))
      p += READ16LE(&eh->e_shentsize) - sizeof(ELFSectionHeader);
  }

  if(READ16LE(&eh->e_shstrndx) != 0) {
    stringTable = (char *)elfReadSection(data,
                                         sh[READ16LE(&eh->e_shstrndx)]);
  }

  elfSectionHeaders = sh;
  elfSectionHeadersStringTable = stringTable;
  elfSectionHeadersCount = count;

  for(i = 0; i < count; i++) {
    //    printf("SH %d %-20s %08x %08x %08x %08x %08x %08x %08x %08x\n",
    //   i, &stringTable[sh[i]->name], sh[i]->name, sh[i]->type,
    //   sh[i]->flags, sh[i]->addr, sh[i]->offset, sh[i]->size,
    //   sh[i]->link, sh[i]->info);
    if(READ32LE(&sh[i]->flags) & 2) { // load section
      if(cpuIsMultiBoot) {
        if(READ32LE(&sh[i]->addr) >= 0x2000000 &&
           READ32LE(&sh[i]->addr) <= 0x203ffff) {
          memcpy(&workRAM[READ32LE(&sh[i]->addr) & 0x3ffff], data +
                 READ32LE(&sh[i]->offset),
                 READ32LE(&sh[i]->size));
                   size += READ32LE(&sh[i]->size);
        }
      } else {
        if(READ32LE(&sh[i]->addr) >= 0x8000000 &&
           READ32LE(&sh[i]->addr) <= 0x9ffffff) {
          memcpy(&rom[READ32LE(&sh[i]->addr) & 0x1ffffff],
                 data + READ32LE(&sh[i]->offset),
                 READ32LE(&sh[i]->size));
          size += READ32LE(&sh[i]->size);
        }
      }
    }
  }

  if(parseDebug) {
    fprintf(stderr, "Parsing debug info\n");

    ELFSectionHeader *dbgHeader = elfGetSectionByName(".debug_info");
    if(dbgHeader == NULL) {
      fprintf(stderr, "Cannot find debug information\n");
      goto end;
    }

    ELFSectionHeader *h = elfGetSectionByName(".debug_abbrev");
    if(h == NULL) {
      fprintf(stderr, "Cannot find abbreviation table\n");
      goto end;
    }

    elfDebugInfo = (DebugInfo *)calloc(sizeof(DebugInfo), 1);
    u8 *abbrevdata = elfReadSection(data, h);

    h = elfGetSectionByName(".debug_str");

    if(h == NULL)
      elfDebugStrings = NULL;
    else
      elfDebugStrings = (char *)elfReadSection(data, h);

    u8 *debugdata = elfReadSection(data, dbgHeader);

    elfDebugInfo->debugdata = data;
    elfDebugInfo->infodata = debugdata;

    u32 total = READ32LE(&dbgHeader->size);
    u8 *end = debugdata + total;
    u8 *ddata = debugdata;

    CompileUnit *last = NULL;
    CompileUnit *unit = NULL;

    while(ddata < end) {
      unit = elfParseCompUnit(ddata, abbrevdata);
      unit->offset = (u32)(ddata-debugdata);
      elfParseLineInfo(unit, data);
      if(last == NULL)
        elfCompileUnits = unit;
      else
        last->next = unit;
      last = unit;
      ddata += 4 + unit->length;
    }
    elfParseAranges(data);
    CompileUnit *comp = elfCompileUnits;
    while(comp) {
      ARanges *r = elfDebugInfo->ranges;
      for(int i = 0; i < elfDebugInfo->numRanges; i++)
        if(r[i].offset == comp->offset) {
          comp->ranges = &r[i];
          break;
        }
      comp = comp->next;
    }
    elfParseCFA(data);
    elfReadSymtab(data);
  }
 end:
  if(sh) {
    free(sh);
  }

  elfSectionHeaders = NULL;
  elfSectionHeadersStringTable = NULL;
  elfSectionHeadersCount = 0;

  return true;
}

extern bool parseDebug;

bool elfRead(const char *name, int& siz, FILE *f)
{
  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  elfFileData = (u8 *)malloc(size);
  fseek(f, 0, SEEK_SET);
  int res = fread(elfFileData, 1, size, f);
  fclose(f);

  if (res < 0)
  {
    free(elfFileData);
    elfFileData = NULL;
    return false;
  }

  ELFHeader *header = (ELFHeader *)elfFileData;

  if(READ32LE(&header->magic) != 0x464C457F ||
     READ16LE(&header->e_machine) != 40 ||
     header->clazz != 1) {
    systemMessage(0, N_("Not a valid ELF file %s"), name);
    free(elfFileData);
    elfFileData = NULL;
    return false;
  }

  if(!elfReadProgram(header, elfFileData, siz, parseDebug)) {
    free(elfFileData);
    elfFileData = NULL;
    return false;
  }

  return true;
}

void elfCleanUp(Object *o)
{
  free(o->location);
}

void elfCleanUp(Function *func)
{
  Object *o = func->parameters;
  while(o) {
    elfCleanUp(o);
    Object *next = o->next;
    free(o);
    o = next;
  }

  o = func->variables;
  while(o) {
    elfCleanUp(o);
    Object *next = o->next;
    free(o);
    o = next;
  }
  free(func->frameBase);
}

void elfCleanUp(ELFAbbrev **abbrevs)
{
  for(int i = 0; i < 121; i++) {
    ELFAbbrev *abbrev = abbrevs[i];

    while(abbrev) {
      free(abbrev->attrs);
      ELFAbbrev *next = abbrev->next;
      free(abbrev);

      abbrev = next;
    }
  }
}

void elfCleanUp(Type *t)
{
  switch(t->type) {
  case TYPE_function:
    if(t->function) {
      Object *o = t->function->args;
      while(o) {
        elfCleanUp(o);
        Object *next = o->next;
        free(o);
        o = next;
      }
      free(t->function);
    }
    break;
  case TYPE_array:
    if(t->array) {
      free(t->array->bounds);
      free(t->array);
    }
    break;
  case TYPE_struct:
  case TYPE_union:
    if(t->structure) {
      for(int i = 0; i < t->structure->memberCount; i++) {
        free(t->structure->members[i].location);
      }
      free(t->structure->members);
      free(t->structure);
    }
    break;
  case TYPE_enum:
    if(t->enumeration) {
      free(t->enumeration->members);
      free(t->enumeration);
    }
    break;
  case TYPE_base:
  case TYPE_pointer:
  case TYPE_void:
  case TYPE_reference:
    break; // nothing to do
  }
}

void elfCleanUp(CompileUnit *comp)
{
  elfCleanUp(comp->abbrevs);
  free(comp->abbrevs);
  Function *func = comp->functions;
  while(func) {
    elfCleanUp(func);
    Function *next = func->next;
    free(func);
    func = next;
  }
  Type *t = comp->types;
  while(t) {
    elfCleanUp(t);
    Type *next = t->next;
    free(t);
    t = next;
  }
  Object *o = comp->variables;
  while(o) {
    elfCleanUp(o);
    Object *next = o->next;
    free(o);
    o = next;
  }
  if(comp->lineInfoTable) {
    free(comp->lineInfoTable->lines);
    free(comp->lineInfoTable->files);
    free(comp->lineInfoTable);
  }
}

void elfCleanUp()
{
  CompileUnit *comp = elfCompileUnits;

  while(comp) {
    elfCleanUp(comp);
    CompileUnit *next = comp->next;
    free(comp);
    comp = next;
  }
  elfCompileUnits = NULL;
  free(elfSymbols);
  elfSymbols = NULL;
  //  free(elfSymbolsStrTab);
  elfSymbolsStrTab = NULL;

  elfDebugStrings = NULL;
  if(elfDebugInfo) {
    int num = elfDebugInfo->numRanges;
    int i;
    for(i = 0; i < num; i++) {
      free(elfDebugInfo->ranges[i].ranges);
    }
    free(elfDebugInfo->ranges);
    free(elfDebugInfo);
    elfDebugInfo = NULL;
  }

  if(elfFdes) {
    if(elfFdeCount) {
      for(int i = 0; i < elfFdeCount; i++)
        free(elfFdes[i]);
    }
    free(elfFdes);

    elfFdes = NULL;
    elfFdeCount = 0;
  }

  ELFcie *cie = elfCies;
  while(cie) {
    ELFcie *next = cie->next;
    free(cie);
    cie = next;
  }
  elfCies = NULL;

  if(elfFileData) {
    free(elfFileData);
    elfFileData = NULL;
  }
}
