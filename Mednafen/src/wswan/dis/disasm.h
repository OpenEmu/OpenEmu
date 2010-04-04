#ifndef _BX_DISASM_H_
#define _BX_DISASM_H_

#include "../../types.h"

typedef uint16 bx_address;

#define BX_DECODE_MODRM(modrm_byte, mod, opcode, rm) { \
  mod    = (modrm_byte >> 6) & 0x03; \
  opcode = (modrm_byte >> 3) & 0x07; \
  rm     =  modrm_byte & 0x07;       \
}

#define BX_DECODE_SIB(sib_byte, scale, index, base) { \
  scale =  sib_byte >> 6;          \
  index = (sib_byte >> 3) & 0x07;  \
  base  =  sib_byte & 0x07;        \
}

/* general purpose bit register */
enum {
	rAX_REG,
	rCX_REG,
	rDX_REG,
	rBX_REG,
	rSP_REG,
	rBP_REG,
	rSI_REG,
	rDI_REG
};

/* segment register */
enum {
	ES_REG,
	CS_REG,
	SS_REG,
	DS_REG,
	FS_REG,
	GS_REG,
        INVALID_SEG1,
        INVALID_SEG2
};

class disassembler;
struct x86_insn;

typedef void (disassembler::*BxDisasmPtr_t)(const x86_insn *insn);
typedef void (disassembler::*BxDisasmResolveModrmPtr_t)(const x86_insn *insn, unsigned attr);

struct BxDisasmOpcodeInfo_t
{
    const char *IntelOpcode;
    const char *AttOpcode;
    BxDisasmPtr_t Operand1;
    BxDisasmPtr_t Operand2;
    BxDisasmPtr_t Operand3;
};

struct BxDisasmOpcodeTable_t
{
    uint32 Attr;
    const void *OpcodeInfo;
};

// segment override not used
#define NO_SEG_OVERRIDE 0xFF

// datasize attributes
#define X_SIZE      0x0000
#define B_SIZE      0x0100
#define W_SIZE      0x0200
#define D_SIZE      0x0300
#define Q_SIZE      0x0400
#define Z_SIZE      0x0500
#define V_SIZE      0x0600
#define O_SIZE      0x0700
#define T_SIZE      0x0800
#define P_SIZE      0x0900

// branch hint attribute
#define BRANCH_HINT 0x1000

struct x86_insn 
{
public:
  x86_insn();

  bool is_seg_override() const { 
     return (seg_override != NO_SEG_OVERRIDE);
 }

public:
  uint8 extend8b;
  uint8 seg_override;
  unsigned b1, prefixes;
  unsigned ilen;

  uint8 modrm, mod, nnn, rm;
  uint8 sib, scale, index, base;
  union {
     uint16 displ16;
     uint32 displ32;
  } displacement;
};

class disassembler {
public:
  disassembler() { set_syntax_intel(); }

  unsigned disasm(bx_address base, bx_address ip, const uint8 *instr, char *disbuf);

  unsigned disasm16(bx_address base, bx_address ip, const uint8 *instr, char *disbuf)
    { return disasm(base, ip, instr, disbuf); }

  x86_insn decode(bx_address base, bx_address ip, const uint8 *instr, char *disbuf);

  x86_insn decode16(bx_address base, bx_address ip, const uint8 *instr, char *disbuf)
    { return decode(base, ip, instr, disbuf); }

  void set_syntax_intel();
  void set_syntax_att  ();

  void toggle_syntax_mode();

private:
  bool intel_mode;

  const char **general_16bit_regname;
  const char **general_8bit_regname;

  const char **segment_name;
  const char **index16;

  const char *sreg_mod01or10_rm32[8];
  const char *sreg_mod00_base32[8];
  const char *sreg_mod01or10_base32[8];
  const char *sreg_mod00_rm16[8];
  const char *sreg_mod01or10_rm16[8];

private:

  bx_address db_eip, db_base;

  const uint8 *instruction;        // for fetching of next byte of instruction

  char *disbufptr;

  BxDisasmResolveModrmPtr_t resolve_modrm;

  uint8  fetch_byte() {
    db_eip++;
    return(*instruction++);
  };

  uint8  peek_byte() {
    return(*instruction);
  };

  uint16 fetch_word() {
    uint8 b0 = * (uint8 *) instruction++;
    uint8 b1 = * (uint8 *) instruction++;
    uint16 ret16 = (b1<<8) | b0;
    db_eip += 2;
    return(ret16);
  };

  uint32 fetch_dword() {
    uint8 b0 = * (uint8 *) instruction++;
    uint8 b1 = * (uint8 *) instruction++;
    uint8 b2 = * (uint8 *) instruction++;
    uint8 b3 = * (uint8 *) instruction++;
    uint32 ret32 = (b3<<24) | (b2<<16) | (b1<<8) | b0;
    db_eip += 4;
    return(ret32);
  };

  void dis_putc(char symbol);
  void dis_sprintf(const char *fmt, ...);
  void decode_modrm(x86_insn *insn);

  void resolve16_mod0   (const x86_insn *insn, unsigned mode);
  void resolve16_mod1or2(const x86_insn *insn, unsigned mode);

  void initialize_modrm_segregs();

  void print_datasize (unsigned mode);

  void print_memory_access16(int datasize,
          const char *seg, const char *index, uint16 disp);
  void print_memory_access  (int datasize,
          const char *seg, const char *base, const char *index, int scale, uint32 disp);

  void print_disassembly_intel(const x86_insn *insn, const BxDisasmOpcodeInfo_t *entry);
  void print_disassembly_att  (const x86_insn *insn, const BxDisasmOpcodeInfo_t *entry);

public:

/* 
 * Codes for Addressing Method:
 * ---------------------------
 * A  - Direct address. The instruction has no ModR/M byte; the address 
 *      of the operand is encoded in the instruction; and no base register, 
 *      index register, or scaling factor can be applied.
 * C  - The reg field of the ModR/M byte selects a control register.
 * D  - The reg field of the ModR/M byte selects a debug register.
 * E  - A ModR/M byte follows the opcode and specifies the operand. The 
 *      operand is either a general-purpose register or a memory address. 
 *      If it is a memory address, the address is computed from a segment 
 *      register and any of the following values: a base register, an
 *      index register, a scaling factor, a displacement.
 * F  - Flags Register.
 * G  - The reg field of the ModR/M byte selects a general register.
 * I  - Immediate data. The operand value is encoded in subsequent bytes of 
 *      the instruction.
 * J  - The instruction contains a relative offset to be added to the 
 *      instruction pointer register.
 * M  - The ModR/M byte may refer only to memory.
 * N  - The R/M field of the ModR/M byte selects a packed-quadword  MMX 
        technology register.
 * O  - The instruction has no ModR/M byte; the offset of the operand is 
 *      coded as a word or double word (depending on address size attribute) 
 *      in the instruction. No base register, index register, or scaling 
 *      factor can be applied.
 * P  - The reg field of the ModR/M byte selects a packed quadword MMX 
 *      technology register.
 * Q  - A ModR/M byte follows the opcode and specifies the operand. The 
 *      operand is either an MMX technology register or a memory address. 
 *      If it is a memory address, the address is computed from a segment 
 *      register and any of the following values: a base register, an 
 *      index register, a scaling factor, and a displacement.
 * R  - The mod field of the ModR/M byte may refer only to a general register.
 * S  - The reg field of the ModR/M byte selects a segment register.
 * U  - The R/M field of the ModR/M byte selects a 128-bit XMM register.
 * T  - The reg field of the ModR/M byte selects a test register.
 * V  - The reg field of the ModR/M byte selects a 128-bit XMM register.
 * W  - A ModR/M byte follows the opcode and specifies the operand. The 
 *      operand is either a 128-bit XMM register or a memory address. If 
 *      it is a memory address, the address is computed from a segment 
 *      register and any of the following values: a base register, an
 *      index register, a scaling factor, and a displacement.
 * X  - Memory addressed by the DS:rSI register pair.
 * Y  - Memory addressed by the ES:rDI register pair.
 */   

/* 
 * Codes for Operand Type:
 * ----------------------
 * a  - Two one-word operands in memory or two double-word operands in 
 *      memory, depending on operand-size attribute (used only by the BOUND
 *      instruction).
 * b  - Byte, regardless of operand-size attribute.
 * d  - Doubleword, regardless of operand-size attribute.
 * dq - Double-quadword, regardless of operand-size attribute.
 * p  - 32-bit or 48-bit pointer, depending on operand-size attribute.
 * pd - 128-bit packed double-precision floating-point data.
 * pi - Quadword MMX technology register (packed integer)
 * ps - 128-bit packed single-precision floating-point data.
 * q  - Quadword, regardless of operand-size attribute.
 * s  - 6-byte or 10-byte pseudo-descriptor.
 * si - Doubleword integer register (scalar integer)
 * ss - Scalar element of a 128-bit packed single-precision floating data.
 * sd - Scalar element of a 128-bit packed double-precision floating data.
 * v  - Word, doubleword or quadword, depending on operand-size attribute.
 * w  - Word, regardless of operand-size attr.
 */

  // far call/jmp
  void Apw(const x86_insn *insn);
  void Apd(const x86_insn *insn);

  // 8-bit general purpose registers
  void AL(const x86_insn *insn);
  void CL(const x86_insn *insn);

  // 16-bit general purpose registers
  void AX(const x86_insn *insn);
  void DX(const x86_insn *insn);

  // segment registers
  void CS(const x86_insn *insn);
  void DS(const x86_insn *insn);
  void ES(const x86_insn *insn);
  void SS(const x86_insn *insn);

  // segment registers
  void Sw(const x86_insn *insn);

  // test registers
  void Td(const x86_insn *insn);

  // control register
  void Cd(const x86_insn *insn);
  void Cq(const x86_insn *insn);

  // debug register
  void Dd(const x86_insn *insn);
  void Dq(const x86_insn *insn);

  //  8-bit general purpose register
  void R8(const x86_insn *insn);

  // 16-bit general purpose register
  void RX(const x86_insn *insn);

  // general purpose register or memory operand
  void Eb(const x86_insn *insn);
  void Ew(const x86_insn *insn);
  void Ed(const x86_insn *insn);
  void Eq(const x86_insn *insn);

  // general purpose register
  void Gb(const x86_insn *insn);
  void Gw(const x86_insn *insn);
  void Gd(const x86_insn *insn);
  void Gq(const x86_insn *insn);

  // immediate
  void I1(const x86_insn *insn);
  void Ib(const x86_insn *insn);
  void Iw(const x86_insn *insn);
  void Id(const x86_insn *insn);
  void Iq(const x86_insn *insn);

  // two immediates Iw/Ib
  void IwIb(const x86_insn *insn);

  // sign extended immediate
  void sIbw(const x86_insn *insn);
  void sIbd(const x86_insn *insn);
  void sIbq(const x86_insn *insn);
  void sIdq(const x86_insn *insn);

  // general purpose register
  void Rw(const x86_insn *insn);
  void Rd(const x86_insn *insn);
  void Rq(const x86_insn *insn);

  // direct memory access
  void OP_O(const x86_insn *insn, unsigned size);
  void Ob(const x86_insn *insn);
  void Ow(const x86_insn *insn);
  void Od(const x86_insn *insn);
  void Oq(const x86_insn *insn);

  // memory operand
  void OP_M(const x86_insn *insn, unsigned size);
  void Ma(const x86_insn *insn);
  void Mp(const x86_insn *insn);
  void Ms(const x86_insn *insn);
  void Mx(const x86_insn *insn);
  void Mb(const x86_insn *insn);
  void Mw(const x86_insn *insn);
  void Md(const x86_insn *insn);
  void Mq(const x86_insn *insn);
  void Mt(const x86_insn *insn);
  void Mdq(const x86_insn *insn);
  void Mps(const x86_insn *insn);
  void Mpd(const x86_insn *insn);

  // string instructions
  void OP_X(const x86_insn *insn, unsigned size);
  void Xb(const x86_insn *insn);
  void Xw(const x86_insn *insn);
  void Xd(const x86_insn *insn);
  void Xq(const x86_insn *insn);

  // string instructions
  void OP_Y(const x86_insn *insn, unsigned size);
  void Yb(const x86_insn *insn);
  void Yw(const x86_insn *insn);
  void Yd(const x86_insn *insn);
  void Yq(const x86_insn *insn);

  // jump offset
  void Jb(const x86_insn *insn);
  void Jw(const x86_insn *insn);
  void Jd(const x86_insn *insn);
};

#endif
