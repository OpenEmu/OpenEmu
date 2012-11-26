/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "psx.h"

namespace MDFN_IEN_PSX
{

struct OpEntry
{
 uint32 mask;
 uint32 value;
 const char *mnemonic;
 const char *format;
};

#define MASK_OP (0x3FU << 26)
#define MASK_FUNC (0x3FU)
#define MASK_RS (0x1FU << 21)
#define MASK_RT (0x1FU << 16)
#define MASK_RD (0x1FU << 11)
#define MASK_SA (0x1FU << 6)

#define MK_OP(mnemonic, format, op, func, extra_mask)	{ MASK_OP | (op ? 0 : MASK_FUNC) | extra_mask, ((unsigned)op << 26) | func, mnemonic, format }

#define MK_OP_REGIMM(mnemonic, regop)	{ MASK_OP | MASK_RT, (0x01U << 26) | (regop << 16), mnemonic, "s, p" }


#define MK_COPZ(z) { MASK_OP | (0x1U << 25), (0x1U << 25) | ((0x10U | z) << 26), "cop" #z, "F" }
#define MK_COP0_FUNC(mnemonic, func) { MASK_OP | (0x1U << 25) | MASK_FUNC, (0x10U << 26) | (0x1U << 25) | func, mnemonic, "" }

#define MK_COPZ_XFER(z, mnemonic, format, xf) { MASK_OP | (0x1FU << 21), ((0x10U | z) << 26) | (xf << 21), mnemonic, format }

#define MK_GTE(mnemonic, format, func) { MASK_OP | (0x1U << 25) | MASK_FUNC, (0x1U << 25) | (0x12U << 26) | func, mnemonic, format }

static OpEntry ops[] =
{
 MK_OP("nop",	"",	0, 0, MASK_RT | MASK_RD | MASK_SA),

 //
 //
 //
 MK_OP("sll",	"d, t, a", 0, 0, 0),
 MK_OP("srl",   "d, t, a", 0, 2, 0),
 MK_OP("sra",   "d, t, a", 0, 3, 0),

 MK_OP("sllv",   "d, t, s", 0, 4, 0),
 MK_OP("srlv",   "d, t, s", 0, 6, 0),
 MK_OP("srav",   "d, t, s", 0, 7, 0),

 MK_OP("jr",   	 "s", 0, 8, 0),
 MK_OP("jalr",   "d, s", 0, 9, 0),

 MK_OP("syscall", "", 0, 12, 0),	// TODO
 MK_OP("break",   "", 0, 13, 0),	// TODO

 MK_OP("mfhi",  "d", 0, 16, 0),
 MK_OP("mthi",  "s", 0, 17, 0),
 MK_OP("mflo",  "d", 0, 18, 0),
 MK_OP("mtlo",  "s", 0, 19, 0),

 MK_OP("mult",  "s, t", 0, 24, 0),
 MK_OP("multu", "s, t", 0, 25, 0),
 MK_OP("div",   "s, t", 0, 26, 0),
 MK_OP("divu",  "s, t", 0, 27, 0),

 MK_OP("add",   "d, s, t", 0, 32, 0),
 MK_OP("addu",  "d, s, t", 0, 33, 0),
 MK_OP("sub",	"d, s, t", 0, 34, 0),
 MK_OP("subu",	"d, s, t", 0, 35, 0),
 MK_OP("and",   "d, s, t", 0, 36, 0),
 MK_OP("or",    "d, s, t", 0, 37, 0),
 MK_OP("xor",   "d, s, t", 0, 38, 0),
 MK_OP("nor",   "d, s, t", 0, 39, 0),
 MK_OP("slt",   "d, s, t", 0, 42, 0),
 MK_OP("sltu",  "d, s, t", 0, 43, 0),

 MK_OP_REGIMM("bgez",	0x01),
 MK_OP_REGIMM("bgezal", 0x11),
 MK_OP_REGIMM("bltz",	0x00),
 MK_OP_REGIMM("bltzal",	0x10),

 
 MK_OP("j",	"P", 2, 0, 0),
 MK_OP("jal",	"P", 3, 0, 0),

 MK_OP("beq",	"s, t, p", 4, 0, 0),
 MK_OP("bne",   "s, t, p", 5, 0, 0),
 MK_OP("blez",  "s, p", 6, 0, 0),
 MK_OP("bgtz",  "s, p", 7, 0, 0),

 MK_OP("addi",  "t, s, i", 8, 0, 0),
 MK_OP("addiu", "t, s, i", 9, 0, 0),
 MK_OP("slti",  "t, s, i", 10, 0, 0),
 MK_OP("sltiu", "t, s, i", 11, 0, 0),

 MK_OP("andi",  "t, s, z", 12, 0, 0),

 MK_OP("ori",  	"t, s, z", 13, 0, 0),
 MK_OP("xori",  "t, s, z", 14, 0, 0),
 MK_OP("lui",	"t, z", 15, 0, 0),

 MK_COPZ_XFER(0, "mfc0", "t, 0", 0x00),
 MK_COPZ_XFER(1, "mfc1", "t, ?", 0x00),
 MK_COPZ_XFER(2, "mfc2", "t, g", 0x00),
 MK_COPZ_XFER(3, "mfc3", "t, ?", 0x00),

 MK_COPZ_XFER(0, "mtc0", "t, 0", 0x04),
 MK_COPZ_XFER(1, "mtc1", "t, ?", 0x04),
 MK_COPZ_XFER(2, "mtc2", "t, g", 0x04),
 MK_COPZ_XFER(3, "mtc3", "t, ?", 0x04),

 MK_COPZ_XFER(0, "cfc0", "t, ?", 0x02),
 MK_COPZ_XFER(1, "cfc1", "t, ?", 0x02),
 MK_COPZ_XFER(2, "cfc2", "t, G", 0x02),
 MK_COPZ_XFER(3, "cfc3", "t, ?", 0x02),

 MK_COPZ_XFER(0, "ctc0", "t, ?", 0x06),
 MK_COPZ_XFER(1, "ctc1", "t, ?", 0x06),
 MK_COPZ_XFER(2, "ctc2", "t, G", 0x06),
 MK_COPZ_XFER(3, "ctc3", "t, ?", 0x06),

 // COP0 stuff here
 MK_COP0_FUNC("rfe", 0x10),

 MK_OP("lwc0", "?, i(s)", 0x30, 0, 0),
 MK_OP("lwc1", "?, i(s)", 0x31, 0, 0),
 MK_OP("lwc2", "h, i(s)", 0x32, 0, 0),
 MK_OP("lwc3", "?, i(s)", 0x33, 0, 0),

 MK_OP("swc0", "?, i(s)", 0x38, 0, 0),
 MK_OP("swc1", "?, i(s)", 0x39, 0, 0),
 MK_OP("swc2", "h, i(s)", 0x3A, 0, 0),
 MK_OP("swc3", "?, i(s)", 0x3B, 0, 0),

 MK_OP("lb",    "t, i(s)", 0x20, 0, 0),
 MK_OP("lh",    "t, i(s)", 0x21, 0, 0),
 MK_OP("lwl",   "t, i(s)", 0x22, 0, 0),
 MK_OP("lw",    "t, i(s)", 0x23, 0, 0),
 MK_OP("lbu",   "t, i(s)", 0x24, 0, 0),
 MK_OP("lhu",   "t, i(s)", 0x25, 0, 0),
 MK_OP("lwr",   "t, i(s)", 0x26, 0, 0),

 MK_OP("sb",    "t, i(s)", 0x28, 0, 0),
 MK_OP("sh",    "t, i(s)", 0x29, 0, 0),
 MK_OP("swl",   "t, i(s)", 0x2A, 0, 0),
 MK_OP("sw",	"t, i(s)", 0x2B, 0, 0),
 MK_OP("swr",   "t, i(s)", 0x2E, 0, 0),

 //
 // GTE specific instructions
 //
 // sf mx v cv lm
 //
 MK_GTE("rtps", "#sf# #lm#", 0x00),
 MK_GTE("rtps", "#sf# #lm#", 0x01),
 MK_GTE("nclip", "", 0x06),
 MK_GTE("op", "#sf# #lm#", 0x0C),

 MK_GTE("dpcs", "#sf# #lm#", 0x10),
 MK_GTE("intpl", "#sf# #lm#", 0x11),
 MK_GTE("mvmva", "#sf# #mx# #v# #cv# #lm#", 0x12),
 MK_GTE("ncds",	"#sf# #lm#", 0x13),
 MK_GTE("cdp", "#sf# #lm#", 0x14),
 MK_GTE("ncdt", "#sf# #lm#", 0x16),
 MK_GTE("dcpl", "#sf# #lm#", 0x1A),
 MK_GTE("nccs", "#sf# #lm#", 0x1B),
 MK_GTE("cc", "#sf# #lm#", 0x1C),
 MK_GTE("ncs", "#sf# #lm#", 0x1E),
 MK_GTE("nct", "#sf# #lm#", 0x20),
 MK_GTE("sqr", "#sf# #lm#", 0x28),
 MK_GTE("dcpl", "#sf# #lm#", 0x29),
 MK_GTE("dpct", "#sf# #lm#", 0x2A),
 MK_GTE("avsz3", "", 0x2D),
 MK_GTE("avsz4", "", 0x2E),
 MK_GTE("rtpt", "#sf# #lm#", 0x30),
 MK_GTE("gpf", "#sf# #lm#", 0x3D),
 MK_GTE("gpl", "#sf# #lm#", 0x3E),
 MK_GTE("ncct", "#sf# #lm#", 0x3F),


 //
 //
 //
 MK_COPZ(0),
 MK_COPZ(1),
 MK_COPZ(2),
 MK_COPZ(3),

 { 0, 0, NULL, NULL }
};

std::string DisassembleMIPS(uint32 PC, uint32 instr)
{
 std::string ret = "UNKNOWN";
 unsigned int rs = (instr >> 21) & 0x1F;
 unsigned int rt = (instr >> 16) & 0x1F;
 unsigned int rd = (instr >> 11) & 0x1F;
 unsigned int shamt = (instr >> 6) & 0x1F;
 unsigned int immediate = (int32)(int16)(instr & 0xFFFF);
 unsigned int immediate_ze = (instr & 0xFFFF);
 unsigned int jt = instr & ((1 << 26) - 1);

 static const char *gpr_names[32] =
 {
  "r0", "at", "v0", "v1", "a0", "a1", "a2", "a3", "t0", "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0", "s1", "s2", "s3", "s4", "s5", "s6", "s7", "t8", "t9", "k0", "k1", "gp", "sp", "fp", "ra"
 };

 static const char *cop0_names[32] =
 {
  "CPR0", "CPR1", "CPR2", "BPC", "CPR4", "BDA", "TAR", "DCIC", "CPR8", "BDAM", "CPR10", "BPCM", "SR", "CAUSE", "EPC", "PRID",
  "ERREG", "CPR17", "CPR18", "CPR19", "CPR20", "CPR21", "CPR22", "CPR23", "CPR24", "CPR25", "CPR26", "CPR27", "CPR28", "CPR29", "CPR30", "CPR31"
 };

 static const char *gte_cr_names[32] =
 {
  "R11R12", "R13R21", "R22R23", "R31R32", "R33", "TRX", "TRY", "TRZ", "L11L12", "L13L21", "L22L23", "L31L32", "L33", "RBK", "GBK", "BBK",
  "LR1LR2", "LR3LG1", "LG2LG3", "LB1LB2", "LB3", "RFC", "GFC", "BFC", "OFX", "OFY", "H", "DQA", "DQB", "ZSF3", "ZSF4", "FLAG"
 };

 static const char *gte_dr_names[32] = 
 {
  "VXY0", "VZ0", "VXY1", "VZ1", "VXY2", "VZ2", "RGB", "OTZ", "IR0", "IR1", "IR2", "IR3", "SXY0", "SXY1", "SXY2", "SXYP",
  "SZ0", "SZ1", "SZ2", "SZ3", "RGB0", "RGB1", "RGB2", "RES1", "MAC0", "MAC1", "MAC2", "MAC3", "IRGB", "ORGB", "LZCS", "LZCR"
 };

 OpEntry *op = ops;

 while(op->mnemonic)
 {
  if((instr & op->mask) == op->value)
  {
   // a = shift amount
   // s = rs
   // t = rt
   // d = rd
   // i = immediate
   // z = immediate, zero-extended
   // p = PC + 4 + immediate
   // P = ((PC + 4) & 0xF0000000) | (26bitval << 2)
   //
   // 0 = rd(cop0 registers)
   // c = rd(copz data registers)
   // C = rd(copz control registers)
   // g = rd(GTE data registers)
   // G = rd(GTE control registers)
   // h = rt(GTE data registers)

   char s_a[16];
   char s_i[16];
   char s_z[16];
   char s_p[16];
   char s_P[16];
   char s_c[16];
   char s_C[16];

   snprintf(s_a, sizeof(s_a), "%d", shamt);

   if(immediate < 0)
    snprintf(s_i, sizeof(s_i), "%d", immediate);
   else
    snprintf(s_i, sizeof(s_i), "0x%04x", (uint32)immediate);

   snprintf(s_z, sizeof(s_z), "0x%04x", immediate_ze);

   snprintf(s_p, sizeof(s_p), "0x%08x", PC + 4 + (immediate << 2));

   snprintf(s_P, sizeof(s_P), "0x%08x", ((PC + 4) & 0xF0000000) | (jt << 2));

   snprintf(s_c, sizeof(s_c), "CPR%d", rd);
   snprintf(s_C, sizeof(s_C), "CCR%d", rd);

   ret = std::string(op->mnemonic);
   ret.append(10 - ret.size(), ' ');

   for(unsigned int i = 0; i < strlen(op->format); i++)
   {
    switch(op->format[i])
    {
     case '#':
 // sf mx v cv lm
        {
	 char as[16];

	 as[0] = 0;
	 if(!strncmp(&op->format[i], "#sf#", 4))
	 {
	  i += 3;
	  snprintf(as, 16, "sf=%d", (int)(bool)(instr & (1 << 19)));
	 }
         else if(!strncmp(&op->format[i], "#mx#", 4))
	 {
	  i += 3;
          snprintf(as, 16, "mx=%d", (instr >> 17) & 0x3);
	 }
         else if(!strncmp(&op->format[i], "#v#", 3))
         {
          i += 2;
	  snprintf(as, 16, "v=%d", (instr >> 15) & 0x3);
         }
         else if(!strncmp(&op->format[i], "#cv#", 4))
         {
          i += 3;
	  snprintf(as, 16, "cv=%d", (instr >> 13) & 0x3);
         }
         else if(!strncmp(&op->format[i], "#lm#", 4))
         {
          i += 3;
	  snprintf(as, 16, "lm=%d", (int)(bool)(instr & (1 << 10)));
         }
	 ret.append(as);
	}
	break;
     case 'F':
	{
	 char s_F[16];

	 snprintf(s_F, 16, "0x%07x", instr & 0x1FFFFFF);
	 ret.append(s_F);
	}
	break;

     case 'h':
        ret.append(gte_dr_names[rt]);
	break;

     case 'g':
	ret.append(gte_dr_names[rd]);
	break;

     case 'G':
	ret.append(gte_cr_names[rd]);
	break;

     case '0':
	ret.append(cop0_names[rd]);
	break;

     case 'c':
	ret.append(s_c);
	break;

     case 'C':
	ret.append(s_C);
	break;

     case 'a':
	ret.append(s_a);
	break;

     case 'i':
	ret.append(s_i);
	break;

     case 'z':
	ret.append(s_z);
	break;

     case 'p':
	ret.append(s_p);
	break;

     case 'P':
	ret.append(s_P);
	break;

     case 's':
	ret.append(gpr_names[rs]);
	break;

     case 't':
	ret.append(gpr_names[rt]);
	break;

    case 'd':
	ret.append(gpr_names[rd]);
	break;

     default:
	ret.append(1, op->format[i]);
	break;
    }
   }
   break;
  }
  op++;
 }

 return(ret);
}

}

