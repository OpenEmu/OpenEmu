static  void thumbUnknownInsn(u32 opcode)
{
	u32 PC = bus.reg[15].I;
	bool savedArmState = armState;
	CPUSwitchMode(0x1b, true, false);
	bus.reg[14].I = PC - (savedArmState ? 4 : 2);
	bus.reg[15].I = 0x04;
	armState = true;
	armIrqEnable = false;
	bus.armNextPC = 0x04;
	ARM_PREFETCH;
	bus.reg[15].I += 4;
}

#define NEG(i) ((i) >> 31)
#define POS(i) ((~(i)) >> 31)

// C core
#ifndef ADDCARRY
 #define ADDCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & NEG(b)) |\
            (NEG(a) & POS(c)) |\
            (NEG(b) & POS(c))) ? true : false;
#endif
#ifndef ADDOVERFLOW
 #define ADDOVERFLOW(a, b, c) \
  V_FLAG = ((NEG(a) & NEG(b) & POS(c)) |\
            (POS(a) & POS(b) & NEG(c))) ? true : false;
#endif
#ifndef SUBCARRY
 #define SUBCARRY(a, b, c) \
  C_FLAG = ((NEG(a) & POS(b)) |\
            (NEG(a) & POS(c)) |\
            (POS(b) & POS(c))) ? true : false;
#endif
#ifndef SUBOVERFLOW
 #define SUBOVERFLOW(a, b, c)\
  V_FLAG = ((NEG(a) & POS(b) & POS(c)) |\
            (POS(a) & NEG(b) & NEG(c))) ? true : false;
#endif
#ifndef ADD_RD_RS_RN
 #define ADD_RD_RS_RN(N) \
   {\
     u32 lhs = bus.reg[source].I;\
     u32 rhs = bus.reg[N].I;\
     u32 res = lhs + rhs;\
     bus.reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef ADD_RD_RS_O3
 #define ADD_RD_RS_O3(N) \
   {\
     u32 lhs = bus.reg[source].I;\
     u32 rhs = N;\
     u32 res = lhs + rhs;\
     bus.reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef ADD_RD_RS_O3_0
# define ADD_RD_RS_O3_0 ADD_RD_RS_O3
#endif
#ifndef ADD_RN_O8
 #define ADD_RN_O8(d) \
   {\
     u32 lhs = bus.reg[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs + rhs;\
     bus.reg[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef CMN_RD_RS
 #define CMN_RD_RS \
   {\
     u32 lhs = bus.reg[dest].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef ADC_RD_RS
 #define ADC_RD_RS \
   {\
     u32 lhs = bus.reg[dest].I;\
     u32 rhs = value;\
     u32 res = lhs + rhs + (u32)C_FLAG;\
     bus.reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     ADDCARRY(lhs, rhs, res);\
     ADDOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef SUB_RD_RS_RN
 #define SUB_RD_RS_RN(N) \
   {\
     u32 lhs = bus.reg[source].I;\
     u32 rhs = bus.reg[N].I;\
     u32 res = lhs - rhs;\
     bus.reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef SUB_RD_RS_O3
 #define SUB_RD_RS_O3(N) \
   {\
     u32 lhs = bus.reg[source].I;\
     u32 rhs = N;\
     u32 res = lhs - rhs;\
     bus.reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef SUB_RD_RS_O3_0
# define SUB_RD_RS_O3_0 SUB_RD_RS_O3
#endif
#ifndef SUB_RN_O8
 #define SUB_RN_O8(d) \
   {\
     u32 lhs = bus.reg[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs - rhs;\
     bus.reg[(d)].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef MOV_RN_O8
 #define MOV_RN_O8(d) \
   {\
     u32 val;\
	 val = (opcode & 255);\
     bus.reg[d].I = val;\
     N_FLAG = false;\
     Z_FLAG = (val ? false : true);\
   }
#endif
#ifndef CMP_RN_O8
 #define CMP_RN_O8(d) \
   {\
     u32 lhs = bus.reg[(d)].I;\
     u32 rhs = (opcode & 255);\
     u32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef SBC_RD_RS
 #define SBC_RD_RS \
   {\
     u32 lhs = bus.reg[dest].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs - !((u32)C_FLAG);\
     bus.reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef LSL_RD_RM_I5
 #define LSL_RD_RM_I5 \
   {\
     C_FLAG = (bus.reg[source].I >> (32 - shift)) & 1 ? true : false;\
     value = bus.reg[source].I << shift;\
   }
#endif
#ifndef LSL_RD_RS
 #define LSL_RD_RS \
   {\
     C_FLAG = (bus.reg[dest].I >> (32 - value)) & 1 ? true : false;\
     value = bus.reg[dest].I << value;\
   }
#endif
#ifndef LSR_RD_RM_I5
 #define LSR_RD_RM_I5 \
   {\
     C_FLAG = (bus.reg[source].I >> (shift - 1)) & 1 ? true : false;\
     value = bus.reg[source].I >> shift;\
   }
#endif
#ifndef LSR_RD_RS
 #define LSR_RD_RS \
   {\
     C_FLAG = (bus.reg[dest].I >> (value - 1)) & 1 ? true : false;\
     value = bus.reg[dest].I >> value;\
   }
#endif
#ifndef ASR_RD_RM_I5
 #define ASR_RD_RM_I5 \
   {\
     C_FLAG = ((s32)bus.reg[source].I >> (int)(shift - 1)) & 1 ? true : false;\
     value = (s32)bus.reg[source].I >> (int)shift;\
   }
#endif
#ifndef ASR_RD_RS
 #define ASR_RD_RS \
   {\
     C_FLAG = ((s32)bus.reg[dest].I >> (int)(value - 1)) & 1 ? true : false;\
     value = (s32)bus.reg[dest].I >> (int)value;\
   }
#endif
#ifndef ROR_RD_RS
 #define ROR_RD_RS \
   {\
     C_FLAG = (bus.reg[dest].I >> (value - 1)) & 1 ? true : false;\
     value = ((bus.reg[dest].I << (32 - value)) |\
              (bus.reg[dest].I >> value));\
   }
#endif
#ifndef NEG_RD_RS
 #define NEG_RD_RS \
   {\
     u32 lhs = bus.reg[source].I;\
     u32 rhs = 0;\
     u32 res = rhs - lhs;\
     bus.reg[dest].I = res;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(rhs, lhs, res);\
     SUBOVERFLOW(rhs, lhs, res);\
   }
#endif
#ifndef CMP_RD_RS
 #define CMP_RD_RS \
   {\
     u32 lhs = bus.reg[dest].I;\
     u32 rhs = value;\
     u32 res = lhs - rhs;\
     Z_FLAG = (res == 0) ? true : false;\
     N_FLAG = NEG(res) ? true : false;\
     SUBCARRY(lhs, rhs, res);\
     SUBOVERFLOW(lhs, rhs, res);\
   }
#endif
#ifndef IMM5_INSN
 #define IMM5_INSN(OP,N) \
  int dest = opcode & 0x07;\
  int source = (opcode >> 3) & 0x07;\
  u32 value;\
  OP(N);\
  bus.reg[dest].I = value;\
  N_FLAG = (value & 0x80000000 ? true : false);\
  Z_FLAG = (value ? false : true);
 #define IMM5_INSN_0(OP) \
  int dest = opcode & 0x07;\
  int source = (opcode >> 3) & 0x07;\
  u32 value;\
  OP;\
  bus.reg[dest].I = value;\
  N_FLAG = (value & 0x80000000 ? true : false);\
  Z_FLAG = (value ? false : true);
 #define IMM5_LSL(N) \
  int shift = N;\
  LSL_RD_RM_I5;
 #define IMM5_LSL_0 \
  value = bus.reg[source].I;
 #define IMM5_LSR(N) \
  int shift = N;\
  LSR_RD_RM_I5;
 #define IMM5_LSR_0 \
  C_FLAG = bus.reg[source].I & 0x80000000 ? true : false;\
  value = 0;
 #define IMM5_ASR(N) \
  int shift = N;\
  ASR_RD_RM_I5;
 #define IMM5_ASR_0 \
  if(bus.reg[source].I & 0x80000000) {\
    value = 0xFFFFFFFF;\
    C_FLAG = true;\
  } else {\
    value = 0;\
    C_FLAG = false;\
  }
#endif
#ifndef THREEARG_INSN
 #define THREEARG_INSN(OP,N) \
  int dest = opcode & 0x07;          \
  int source = (opcode >> 3) & 0x07; \
  OP(N);
#endif

// Shift instructions /////////////////////////////////////////////////////

#define DEFINE_IMM5_INSN(OP,BASE) \
  static  void thumb##BASE##_00(u32 opcode) { IMM5_INSN_0(OP##_0); } \
  static  void thumb##BASE##_01(u32 opcode) { IMM5_INSN(OP, 1); } \
  static  void thumb##BASE##_02(u32 opcode) { IMM5_INSN(OP, 2); } \
  static  void thumb##BASE##_03(u32 opcode) { IMM5_INSN(OP, 3); } \
  static  void thumb##BASE##_04(u32 opcode) { IMM5_INSN(OP, 4); } \
  static  void thumb##BASE##_05(u32 opcode) { IMM5_INSN(OP, 5); } \
  static  void thumb##BASE##_06(u32 opcode) { IMM5_INSN(OP, 6); } \
  static  void thumb##BASE##_07(u32 opcode) { IMM5_INSN(OP, 7); } \
  static  void thumb##BASE##_08(u32 opcode) { IMM5_INSN(OP, 8); } \
  static  void thumb##BASE##_09(u32 opcode) { IMM5_INSN(OP, 9); } \
  static  void thumb##BASE##_0A(u32 opcode) { IMM5_INSN(OP,10); } \
  static  void thumb##BASE##_0B(u32 opcode) { IMM5_INSN(OP,11); } \
  static  void thumb##BASE##_0C(u32 opcode) { IMM5_INSN(OP,12); } \
  static  void thumb##BASE##_0D(u32 opcode) { IMM5_INSN(OP,13); } \
  static  void thumb##BASE##_0E(u32 opcode) { IMM5_INSN(OP,14); } \
  static  void thumb##BASE##_0F(u32 opcode) { IMM5_INSN(OP,15); } \
  static  void thumb##BASE##_10(u32 opcode) { IMM5_INSN(OP,16); } \
  static  void thumb##BASE##_11(u32 opcode) { IMM5_INSN(OP,17); } \
  static  void thumb##BASE##_12(u32 opcode) { IMM5_INSN(OP,18); } \
  static  void thumb##BASE##_13(u32 opcode) { IMM5_INSN(OP,19); } \
  static  void thumb##BASE##_14(u32 opcode) { IMM5_INSN(OP,20); } \
  static  void thumb##BASE##_15(u32 opcode) { IMM5_INSN(OP,21); } \
  static  void thumb##BASE##_16(u32 opcode) { IMM5_INSN(OP,22); } \
  static  void thumb##BASE##_17(u32 opcode) { IMM5_INSN(OP,23); } \
  static  void thumb##BASE##_18(u32 opcode) { IMM5_INSN(OP,24); } \
  static  void thumb##BASE##_19(u32 opcode) { IMM5_INSN(OP,25); } \
  static  void thumb##BASE##_1A(u32 opcode) { IMM5_INSN(OP,26); } \
  static  void thumb##BASE##_1B(u32 opcode) { IMM5_INSN(OP,27); } \
  static  void thumb##BASE##_1C(u32 opcode) { IMM5_INSN(OP,28); } \
  static  void thumb##BASE##_1D(u32 opcode) { IMM5_INSN(OP,29); } \
  static  void thumb##BASE##_1E(u32 opcode) { IMM5_INSN(OP,30); } \
  static  void thumb##BASE##_1F(u32 opcode) { IMM5_INSN(OP,31); }

// LSL Rd, Rm, #Imm 5
DEFINE_IMM5_INSN(IMM5_LSL,00)
// LSR Rd, Rm, #Imm 5
DEFINE_IMM5_INSN(IMM5_LSR,08)
// ASR Rd, Rm, #Imm 5
DEFINE_IMM5_INSN(IMM5_ASR,10)

// 3-argument ADD/SUB /////////////////////////////////////////////////////

#define DEFINE_REG3_INSN(OP,BASE) \
  static  void thumb##BASE##_0(u32 opcode) { THREEARG_INSN(OP,0); } \
  static  void thumb##BASE##_1(u32 opcode) { THREEARG_INSN(OP,1); } \
  static  void thumb##BASE##_2(u32 opcode) { THREEARG_INSN(OP,2); } \
  static  void thumb##BASE##_3(u32 opcode) { THREEARG_INSN(OP,3); } \
  static  void thumb##BASE##_4(u32 opcode) { THREEARG_INSN(OP,4); } \
  static  void thumb##BASE##_5(u32 opcode) { THREEARG_INSN(OP,5); } \
  static  void thumb##BASE##_6(u32 opcode) { THREEARG_INSN(OP,6); } \
  static  void thumb##BASE##_7(u32 opcode) { THREEARG_INSN(OP,7); }

#define DEFINE_IMM3_INSN(OP,BASE) \
  static  void thumb##BASE##_0(u32 opcode) { THREEARG_INSN(OP##_0,0); } \
  static  void thumb##BASE##_1(u32 opcode) { THREEARG_INSN(OP,1); } \
  static  void thumb##BASE##_2(u32 opcode) { THREEARG_INSN(OP,2); } \
  static  void thumb##BASE##_3(u32 opcode) { THREEARG_INSN(OP,3); } \
  static  void thumb##BASE##_4(u32 opcode) { THREEARG_INSN(OP,4); } \
  static  void thumb##BASE##_5(u32 opcode) { THREEARG_INSN(OP,5); } \
  static  void thumb##BASE##_6(u32 opcode) { THREEARG_INSN(OP,6); } \
  static  void thumb##BASE##_7(u32 opcode) { THREEARG_INSN(OP,7); }

// ADD Rd, Rs, Rn
DEFINE_REG3_INSN(ADD_RD_RS_RN,18)
// SUB Rd, Rs, Rn
DEFINE_REG3_INSN(SUB_RD_RS_RN,1A)
// ADD Rd, Rs, #Offset3
DEFINE_IMM3_INSN(ADD_RD_RS_O3,1C)
// SUB Rd, Rs, #Offset3
DEFINE_IMM3_INSN(SUB_RD_RS_O3,1E)

// MOV/CMP/ADD/SUB immediate //////////////////////////////////////////////

// MOV R0, #Offset8
static  void thumb20(u32 opcode) { MOV_RN_O8(0); }
// MOV R1, #Offset8
static  void thumb21(u32 opcode) { MOV_RN_O8(1); }
// MOV R2, #Offset8
static  void thumb22(u32 opcode) { MOV_RN_O8(2); }
// MOV R3, #Offset8
static  void thumb23(u32 opcode) { MOV_RN_O8(3); }
// MOV R4, #Offset8
static  void thumb24(u32 opcode) { MOV_RN_O8(4); }
// MOV R5, #Offset8
static  void thumb25(u32 opcode) { MOV_RN_O8(5); }
// MOV R6, #Offset8
static  void thumb26(u32 opcode) { MOV_RN_O8(6); }
// MOV R7, #Offset8
static  void thumb27(u32 opcode) { MOV_RN_O8(7); }

// CMP R0, #Offset8
static  void thumb28(u32 opcode) { CMP_RN_O8(0); }
// CMP R1, #Offset8
static  void thumb29(u32 opcode) { CMP_RN_O8(1); }
// CMP R2, #Offset8
static  void thumb2A(u32 opcode) { CMP_RN_O8(2); }
// CMP R3, #Offset8
static  void thumb2B(u32 opcode) { CMP_RN_O8(3); }
// CMP R4, #Offset8
static  void thumb2C(u32 opcode) { CMP_RN_O8(4); }
// CMP R5, #Offset8
static  void thumb2D(u32 opcode) { CMP_RN_O8(5); }
// CMP R6, #Offset8
static  void thumb2E(u32 opcode) { CMP_RN_O8(6); }
// CMP R7, #Offset8
static  void thumb2F(u32 opcode) { CMP_RN_O8(7); }

// ADD R0,#Offset8
static  void thumb30(u32 opcode) { ADD_RN_O8(0); }
// ADD R1,#Offset8
static  void thumb31(u32 opcode) { ADD_RN_O8(1); }
// ADD R2,#Offset8
static  void thumb32(u32 opcode) { ADD_RN_O8(2); }
// ADD R3,#Offset8
static  void thumb33(u32 opcode) { ADD_RN_O8(3); }
// ADD R4,#Offset8
static  void thumb34(u32 opcode) { ADD_RN_O8(4); }
// ADD R5,#Offset8
static  void thumb35(u32 opcode) { ADD_RN_O8(5); }
// ADD R6,#Offset8
static  void thumb36(u32 opcode) { ADD_RN_O8(6); }
// ADD R7,#Offset8
static  void thumb37(u32 opcode) { ADD_RN_O8(7); }

// SUB R0,#Offset8
static  void thumb38(u32 opcode) { SUB_RN_O8(0); }
// SUB R1,#Offset8
static  void thumb39(u32 opcode) { SUB_RN_O8(1); }
// SUB R2,#Offset8
static  void thumb3A(u32 opcode) { SUB_RN_O8(2); }
// SUB R3,#Offset8
static  void thumb3B(u32 opcode) { SUB_RN_O8(3); }
// SUB R4,#Offset8
static  void thumb3C(u32 opcode) { SUB_RN_O8(4); }
// SUB R5,#Offset8
static  void thumb3D(u32 opcode) { SUB_RN_O8(5); }
// SUB R6,#Offset8
static  void thumb3E(u32 opcode) { SUB_RN_O8(6); }
// SUB R7,#Offset8
static  void thumb3F(u32 opcode) { SUB_RN_O8(7); }

// ALU operations /////////////////////////////////////////////////////////

// AND Rd, Rs
static  void thumb40_0(u32 opcode)
{
  int dest = opcode & 7;
  u32 val = (bus.reg[dest].I & bus.reg[(opcode >> 3)&7].I);
  
  //bus.reg[dest].I &= bus.reg[(opcode >> 3)&7].I;
  N_FLAG = val & 0x80000000 ? true : false;
  Z_FLAG = val ? false : true;

  bus.reg[dest].I = val;

}

// EOR Rd, Rs
static  void thumb40_1(u32 opcode)
{
  int dest = opcode & 7;
  bus.reg[dest].I ^= bus.reg[(opcode >> 3)&7].I;
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
  Z_FLAG = bus.reg[dest].I ? false : true;
}

// LSL Rd, Rs
static  void thumb40_2(u32 opcode)
{
  int dest = opcode & 7;
  u32 value = bus.reg[(opcode >> 3)&7].B.B0;
  u32 val = value;
  if(val) {
    if(val == 32) {
      value = 0;
      C_FLAG = (bus.reg[dest].I & 1 ? true : false);
    } else if(val < 32) {
      LSL_RD_RS;
    } else {
      value = 0;
      C_FLAG = false;
    }
    bus.reg[dest].I = value;
  }
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
  Z_FLAG = bus.reg[dest].I ? false : true;
  clockTicks = codeTicksAccess(bus.armNextPC, BITS_16)+2;
}

// LSR Rd, Rs
static  void thumb40_3(u32 opcode)
{
  int dest = opcode & 7;
  u32 value = bus.reg[(opcode >> 3)&7].B.B0;
  u32 val = value;
  if(val) {
    if(val == 32) {
      value = 0;
      C_FLAG = (bus.reg[dest].I & 0x80000000 ? true : false);
    } else if(val < 32) {
      LSR_RD_RS;
    } else {
      value = 0;
      C_FLAG = false;
    }
    bus.reg[dest].I = value;
  }
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
  Z_FLAG = bus.reg[dest].I ? false : true;
  clockTicks = codeTicksAccess(bus.armNextPC, BITS_16)+2;
}

// ASR Rd, Rs
static  void thumb41_0(u32 opcode)
{
  int dest = opcode & 7;
  u32 value = bus.reg[(opcode >> 3)&7].B.B0;
  u32 val = value;
  
  if(value) {
    if(value < 32) {
      ASR_RD_RS;
      bus.reg[dest].I = value;
    } else {
      if(bus.reg[dest].I & 0x80000000){
        bus.reg[dest].I = 0xFFFFFFFF;
        C_FLAG = true;
      } else {
        bus.reg[dest].I = 0x00000000;
        C_FLAG = false;
      }
    }
  }
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
  Z_FLAG = bus.reg[dest].I ? false : true;
  clockTicks = codeTicksAccess(bus.armNextPC, BITS_16)+2;
}

// ADC Rd, Rs
static  void thumb41_1(u32 opcode)
{
  int dest = opcode & 0x07;
  u32 value = bus.reg[(opcode >> 3)&7].I;
  ADC_RD_RS;
}

// SBC Rd, Rs
static  void thumb41_2(u32 opcode)
{
  int dest = opcode & 0x07;
  u32 value = bus.reg[(opcode >> 3)&7].I;
  SBC_RD_RS;
}

// ROR Rd, Rs
static  void thumb41_3(u32 opcode)
{
  int dest = opcode & 7;
  u32 value = bus.reg[(opcode >> 3)&7].B.B0;
  u32 val = value;
  if(val) {
    value = value & 0x1f;
    if(val == 0) {
      C_FLAG = (bus.reg[dest].I & 0x80000000 ? true : false);
    } else {
      ROR_RD_RS;
      bus.reg[dest].I = value;
    }
  }
  clockTicks = codeTicksAccess(bus.armNextPC, BITS_16)+2;
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
  Z_FLAG = bus.reg[dest].I ? false : true;
}

// TST Rd, Rs
static  void thumb42_0(u32 opcode)
{
  u32 value = bus.reg[opcode & 7].I & bus.reg[(opcode >> 3) & 7].I;
  N_FLAG = value & 0x80000000 ? true : false;
  Z_FLAG = value ? false : true;
}

// NEG Rd, Rs
static  void thumb42_1(u32 opcode)
{
  int dest = opcode & 7;
  int source = (opcode >> 3) & 7;
  NEG_RD_RS;
}

// CMP Rd, Rs
static  void thumb42_2(u32 opcode)
{
  int dest = opcode & 7;
  u32 value = bus.reg[(opcode >> 3)&7].I;
  CMP_RD_RS;
}

// CMN Rd, Rs
static  void thumb42_3(u32 opcode)
{
  int dest = opcode & 7;
  u32 value = bus.reg[(opcode >> 3)&7].I;
  CMN_RD_RS;
}

// ORR Rd, Rs
static  void thumb43_0(u32 opcode)
{
  int dest = opcode & 7;
  bus.reg[dest].I |= bus.reg[(opcode >> 3) & 7].I;
  Z_FLAG = bus.reg[dest].I ? false : true;
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
}

// MUL Rd, Rs
static  void thumb43_1(u32 opcode)
{
  clockTicks = 1;
  int dest = opcode & 7;
  u32 rm = bus.reg[dest].I;
  bus.reg[dest].I = bus.reg[(opcode >> 3) & 7].I * rm;
  if (((s32)rm) < 0)
    rm = ~rm;
  if ((rm & 0xFFFF0000) == 0)
    clockTicks += 1;
  else if ((rm & 0xFF000000) == 0)
    clockTicks += 2;
  else
    clockTicks += 3;
  bus.busPrefetchCount = (bus.busPrefetchCount<<clockTicks) | (0xFF>>(8-clockTicks));
  clockTicks += codeTicksAccess(bus.armNextPC, BITS_16) + 1;
  Z_FLAG = bus.reg[dest].I ? false : true;
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
}

// BIC Rd, Rs
static  void thumb43_2(u32 opcode)
{
  int dest = opcode & 7;
  bus.reg[dest].I &= (~bus.reg[(opcode >> 3) & 7].I);
  Z_FLAG = bus.reg[dest].I ? false : true;
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
}

// MVN Rd, Rs
static  void thumb43_3(u32 opcode)
{
  int dest = opcode & 7;
  bus.reg[dest].I = ~bus.reg[(opcode >> 3) & 7].I;
  Z_FLAG = bus.reg[dest].I ? false : true;
  N_FLAG = bus.reg[dest].I & 0x80000000 ? true : false;
}

// High-register instructions and BX //////////////////////////////////////

// ADD Rd, Hs
static  void thumb44_1(u32 opcode)
{
  bus.reg[opcode&7].I += bus.reg[((opcode>>3)&7)+8].I;
}

// ADD Hd, Rs
static  void thumb44_2(u32 opcode)
{
  bus.reg[(opcode&7)+8].I += bus.reg[(opcode>>3)&7].I;
  if((opcode&7) == 7) {
    bus.reg[15].I &= 0xFFFFFFFE;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = codeTicksAccessSeq16(bus.armNextPC)<<1
        + codeTicksAccess(bus.armNextPC, BITS_16) + 3;
  }
}

// ADD Hd, Hs
static  void thumb44_3(u32 opcode)
{
  bus.reg[(opcode&7)+8].I += bus.reg[((opcode>>3)&7)+8].I;
  if((opcode&7) == 7) {
    bus.reg[15].I &= 0xFFFFFFFE;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = codeTicksAccessSeq16(bus.armNextPC)<<1
        + codeTicksAccess(bus.armNextPC, BITS_16) + 3;
  }
}

// CMP Rd, Hs
static  void thumb45_1(u32 opcode)
{
  int dest = opcode & 7;
  u32 value = bus.reg[((opcode>>3)&7)+8].I;
  CMP_RD_RS;
}

// CMP Hd, Rs
static  void thumb45_2(u32 opcode)
{
  int dest = (opcode & 7) + 8;
  u32 value = bus.reg[(opcode>>3)&7].I;
  CMP_RD_RS;
}

// CMP Hd, Hs
static  void thumb45_3(u32 opcode)
{
  int dest = (opcode & 7) + 8;
  u32 value = bus.reg[((opcode>>3)&7)+8].I;
  CMP_RD_RS;
}

// MOV Rd, Hs
static  void thumb46_1(u32 opcode)
{
  bus.reg[opcode&7].I = bus.reg[((opcode>>3)&7)+8].I;
}

// MOV Hd, Rs
static  void thumb46_2(u32 opcode)
{
  bus.reg[(opcode&7)+8].I = bus.reg[(opcode>>3)&7].I;
  if((opcode&7) == 7) {
    bus.reg[15].I &= 0xFFFFFFFE;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = codeTicksAccessSeq16(bus.armNextPC)<<1
        + codeTicksAccess(bus.armNextPC, BITS_16) + 3;
  }
}

// MOV Hd, Hs
static  void thumb46_3(u32 opcode)
{
  bus.reg[(opcode&7)+8].I = bus.reg[((opcode>>3)&7)+8].I;
  if((opcode&7) == 7) {
    bus.reg[15].I &= 0xFFFFFFFE;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = codeTicksAccessSeq16(bus.armNextPC)<<1
        + codeTicksAccess(bus.armNextPC, BITS_16) + 3;
  }
}


// BX Rs
static  void thumb47(u32 opcode)
{
  int base = (opcode >> 3) & 15;
  bus.busPrefetchCount=0;
  bus.reg[15].I = bus.reg[base].I;
  if(bus.reg[base].I & 1) {
    armState = false;
    bus.reg[15].I &= 0xFFFFFFFE;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1)
    + codeTicksAccess(bus.armNextPC, BITS_16) + 3;
  } else {
    armState = true;
    bus.reg[15].I &= 0xFFFFFFFC;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 4;
    ARM_PREFETCH;
    clockTicks = ((codeTicksAccessSeq32(bus.armNextPC)) << 1) 
    + codeTicksAccess(bus.armNextPC, BITS_32) + 3;
  }
}

// Load/store instructions ////////////////////////////////////////////////

// LDR R0~R7,[PC, #Imm]
static  void thumb48(u32 opcode)
{
  u8 regist = (opcode >> 8) & 7;
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = (bus.reg[15].I & 0xFFFFFFFC) + ((opcode & 0xFF) << 2);
  bus.reg[regist].I = CPUReadMemoryQuick(address);
  bus.busPrefetchCount=0;
  clockTicks = 3 + dataTicksAccess(address, BITS_32) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// STR Rd, [Rs, Rn]
static  void thumb50(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + bus.reg[(opcode>>6)&7].I;
  CPUWriteMemory(address, bus.reg[opcode & 7].I);
  clockTicks = dataTicksAccess(address, BITS_32) + codeTicksAccess(bus.armNextPC, BITS_16) + 2;
}

// STRH Rd, [Rs, Rn]
static  void thumb52(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + bus.reg[(opcode>>6)&7].I;
  CPUWriteHalfWord(address, bus.reg[opcode&7].W.W0);
  clockTicks = dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16) + 2;
}

// STRB Rd, [Rs, Rn]
static  void thumb54(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + bus.reg[(opcode >>6)&7].I;
  CPUWriteByte(address, bus.reg[opcode & 7].B.B0);
  clockTicks = dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16) + 2;
}

// LDSB Rd, [Rs, Rn]
static  void thumb56(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + bus.reg[(opcode>>6)&7].I;
  bus.reg[opcode&7].I = (s8)CPUReadByte(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// LDR Rd, [Rs, Rn]
static  void thumb58(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + bus.reg[(opcode>>6)&7].I;
  bus.reg[opcode&7].I = CPUReadMemory(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_32) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// LDRH Rd, [Rs, Rn]
static  void thumb5A(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + bus.reg[(opcode>>6)&7].I;
  bus.reg[opcode&7].I = CPUReadHalfWord(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_32) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// LDRB Rd, [Rs, Rn]
static  void thumb5C(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + bus.reg[(opcode>>6)&7].I;
  bus.reg[opcode&7].I = CPUReadByte(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// LDSH Rd, [Rs, Rn]
static  void thumb5E(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + bus.reg[(opcode>>6)&7].I;
  bus.reg[opcode&7].I = (s16)CPUReadHalfWordSigned(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// STR Rd, [Rs, #Imm]
static  void thumb60(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
  CPUWriteMemory(address, bus.reg[opcode&7].I);
  clockTicks = dataTicksAccess(address, BITS_32) + codeTicksAccess(bus.armNextPC, BITS_16) + 2;
}

// LDR Rd, [Rs, #Imm]
static  void thumb68(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<2);
  bus.reg[opcode&7].I = CPUReadMemory(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_32) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// STRB Rd, [Rs, #Imm]
static  void thumb70(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + (((opcode>>6)&31));
  CPUWriteByte(address, bus.reg[opcode&7].B.B0);
  clockTicks = dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16) + 2;
}

// LDRB Rd, [Rs, #Imm]
static  void thumb78(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + (((opcode>>6)&31));
  bus.reg[opcode&7].I = CPUReadByte(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// STRH Rd, [Rs, #Imm]
static  void thumb80(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
  CPUWriteHalfWord(address, bus.reg[opcode&7].W.W0);
  clockTicks = dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16) + 2;
}

// LDRH Rd, [Rs, #Imm]
static  void thumb88(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[(opcode>>3)&7].I + (((opcode>>6)&31)<<1);
  bus.reg[opcode&7].I = CPUReadHalfWord(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_16) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// STR R0~R7, [SP, #Imm]
static  void thumb90(u32 opcode)
{
  u8 regist = (opcode >> 8) & 7;
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[13].I + ((opcode&255)<<2);
  CPUWriteMemory(address, bus.reg[regist].I);
  clockTicks = dataTicksAccess(address, BITS_32) + codeTicksAccess(bus.armNextPC, BITS_16) + 2;
}

// LDR R0~R7, [SP, #Imm]
static  void thumb98(u32 opcode)
{
  u8 regist = (opcode >> 8) & 7;
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[13].I + ((opcode&255)<<2);
  bus.reg[regist].I = CPUReadMemoryQuick(address);
  clockTicks = 3 + dataTicksAccess(address, BITS_32) + codeTicksAccess(bus.armNextPC, BITS_16);
}

// PC/stack-related ///////////////////////////////////////////////////////

// ADD R0~R7, PC, Imm
static  void thumbA0(u32 opcode)
{
  u8 regist = (opcode >> 8) & 7;
  bus.reg[regist].I = (bus.reg[15].I & 0xFFFFFFFC) + ((opcode&255)<<2);
}

// ADD R0~R7, SP, Imm
static  void thumbA8(u32 opcode)
{
  u8 regist = (opcode >> 8) & 7;
  bus.reg[regist].I = bus.reg[13].I + ((opcode&255)<<2);
}

// ADD SP, Imm
static  void thumbB0(u32 opcode)
{
  int offset = (opcode & 127) << 2;
  if(opcode & 0x80)
    offset = -offset;
  bus.reg[13].I += offset;
}

// Push and pop ///////////////////////////////////////////////////////////

#define PUSH_REG(val, r)                                    \
  if (opcode & (val)) {                                     \
    CPUWriteMemory(address, bus.reg[(r)].I);                    \
    if (!count) {                                           \
        clockTicks += 1 + dataTicksAccess(address, BITS_32);       \
    } else {                                                \
        clockTicks += 1 + dataTicksAccessSeq(address, BITS_32);    \
    }                                                       \
    count++;                                                \
    address += 4;                                           \
  }

#define POP_REG(val, r)                                     \
  if (opcode & (val)) {                                     \
    bus.reg[(r)].I = CPUReadMemory(address);                    \
    if (!count) {                                           \
        clockTicks += 1 + dataTicksAccess(address, BITS_32);       \
    } else {                                                \
        clockTicks += 1 + dataTicksAccessSeq(address, BITS_32);    \
    }                                                       \
    count++;                                                \
    address += 4;                                           \
  }

// PUSH {Rlist}
static  void thumbB4(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  int count = 0;
  u32 temp = bus.reg[13].I - 4 * cpuBitsSet[opcode & 0xff];
  u32 address = temp & 0xFFFFFFFC;
  PUSH_REG(1, 0);
  PUSH_REG(2, 1);
  PUSH_REG(4, 2);
  PUSH_REG(8, 3);
  PUSH_REG(16, 4);
  PUSH_REG(32, 5);
  PUSH_REG(64, 6);
  PUSH_REG(128, 7);
  clockTicks += 1 + codeTicksAccess(bus.armNextPC, BITS_16);
  bus.reg[13].I = temp;
}

// PUSH {Rlist, LR}
static  void thumbB5(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  int count = 0;
  u32 temp = bus.reg[13].I - 4 - 4 * cpuBitsSet[opcode & 0xff];
  u32 address = temp & 0xFFFFFFFC;
  PUSH_REG(1, 0);
  PUSH_REG(2, 1);
  PUSH_REG(4, 2);
  PUSH_REG(8, 3);
  PUSH_REG(16, 4);
  PUSH_REG(32, 5);
  PUSH_REG(64, 6);
  PUSH_REG(128, 7);
  PUSH_REG(256, 14);
  clockTicks += 1 + codeTicksAccess(bus.armNextPC, BITS_16);
  bus.reg[13].I = temp;
}

// POP {Rlist}
static  void thumbBC(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  int count = 0;
  u32 address = bus.reg[13].I & 0xFFFFFFFC;
  u32 temp = bus.reg[13].I + 4*cpuBitsSet[opcode & 0xFF];
  POP_REG(1, 0);
  POP_REG(2, 1);
  POP_REG(4, 2);
  POP_REG(8, 3);
  POP_REG(16, 4);
  POP_REG(32, 5);
  POP_REG(64, 6);
  POP_REG(128, 7);
  bus.reg[13].I = temp;
  clockTicks = 2 + codeTicksAccess(bus.armNextPC, BITS_16);
}

// POP {Rlist, PC}
static  void thumbBD(u32 opcode)
{
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  int count = 0;
  u32 address = bus.reg[13].I & 0xFFFFFFFC;
  u32 temp = bus.reg[13].I + 4 + 4*cpuBitsSet[opcode & 0xFF];
  POP_REG(1, 0);
  POP_REG(2, 1);
  POP_REG(4, 2);
  POP_REG(8, 3);
  POP_REG(16, 4);
  POP_REG(32, 5);
  POP_REG(64, 6);
  POP_REG(128, 7);
  bus.reg[15].I = (CPUReadMemory(address) & 0xFFFFFFFE);
  if (!count) {
    clockTicks += 1 + dataTicksAccess(address, BITS_32);
  } else {
    clockTicks += 1 + dataTicksAccessSeq(address, BITS_32);
  }
  count++;
  bus.armNextPC = bus.reg[15].I;
  bus.reg[15].I += 2;
  bus.reg[13].I = temp;
  THUMB_PREFETCH;
  bus.busPrefetchCount = 0;
  clockTicks += 3 + ((codeTicksAccess(bus.armNextPC, BITS_16)) << 1);
}

// Load/store multiple ////////////////////////////////////////////////////

#define THUMB_STM_REG(val,r,b)                              \
  if(opcode & (val)) {                                      \
    CPUWriteMemory(address, bus.reg[(r)].I);                    \
    bus.reg[(b)].I = temp;                                      \
    if (!count) {                                           \
        clockTicks += 1 + dataTicksAccess(address, BITS_32);       \
    } else {                                                \
        clockTicks += 1 + dataTicksAccessSeq(address, BITS_32);    \
    }                                                       \
    count++;                                                \
    address += 4;                                           \
  }

#define THUMB_LDM_REG(val,r)                                \
  if(opcode & (val)) {                                      \
    bus.reg[(r)].I = CPUReadMemory(address);                    \
    if (!count) {                                           \
        clockTicks += 1 + dataTicksAccess(address, BITS_32);       \
    } else {                                                \
        clockTicks += 1 + dataTicksAccessSeq(address, BITS_32);    \
    }                                                       \
    count++;                                                \
    address += 4;                                           \
  }

// STM R0~7!, {Rlist}
static  void thumbC0(u32 opcode)
{
  u8 regist = (opcode >> 8) & 7;
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[regist].I & 0xFFFFFFFC;
  u32 temp = bus.reg[regist].I + 4*cpuBitsSet[opcode & 0xff];
  int count = 0;
  // store
  THUMB_STM_REG(1, 0, regist);
  THUMB_STM_REG(2, 1, regist);
  THUMB_STM_REG(4, 2, regist);
  THUMB_STM_REG(8, 3, regist);
  THUMB_STM_REG(16, 4, regist);
  THUMB_STM_REG(32, 5, regist);
  THUMB_STM_REG(64, 6, regist);
  THUMB_STM_REG(128, 7, regist);
  clockTicks = 1 + codeTicksAccess(bus.armNextPC, BITS_16);
}

// LDM R0~R7!, {Rlist}
static  void thumbC8(u32 opcode)
{
  u8 regist = (opcode >> 8) & 7;
  if (bus.busPrefetchCount == 0)
    bus.busPrefetch = bus.busPrefetchEnable;
  u32 address = bus.reg[regist].I & 0xFFFFFFFC;
  u32 temp = bus.reg[regist].I + 4*cpuBitsSet[opcode & 0xFF];
  int count = 0;
  // load
  THUMB_LDM_REG(1, 0);
  THUMB_LDM_REG(2, 1);
  THUMB_LDM_REG(4, 2);
  THUMB_LDM_REG(8, 3);
  THUMB_LDM_REG(16, 4);
  THUMB_LDM_REG(32, 5);
  THUMB_LDM_REG(64, 6);
  THUMB_LDM_REG(128, 7);
  clockTicks = 2 + codeTicksAccess(bus.armNextPC, BITS_16);
  if(!(opcode & (1<<regist)))
    bus.reg[regist].I = temp;
}

// Conditional branches ///////////////////////////////////////////////////

// BEQ offset
static  void thumbD0(u32 opcode)
{
  int val;
  if(Z_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
#if defined (SPEEDHAX)
	clockTicks = 30;
#else
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
#endif
    bus.busPrefetchCount=0;
  }
}

// BNE offset
static  void thumbD1(u32 opcode)
{
  if(!Z_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BCS offset
static  void thumbD2(u32 opcode)
{
  if(C_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BCC offset
static  void thumbD3(u32 opcode)
{
  if(!C_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BMI offset
static  void thumbD4(u32 opcode)
{
  if(N_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BPL offset
static  void thumbD5(u32 opcode)
{
  if(!N_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BVS offset
static  void thumbD6(u32 opcode)
{
  if(V_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BVC offset
static  void thumbD7(u32 opcode)
{
  if(!V_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BHI offset
static  void thumbD8(u32 opcode)
{
  if(C_FLAG && !Z_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BLS offset
static  void thumbD9(u32 opcode)
{
  if(!C_FLAG || Z_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BGE offset
static  void thumbDA(u32 opcode)
{
  if(N_FLAG == V_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BLT offset
static  void thumbDB(u32 opcode)
{
  if(N_FLAG != V_FLAG) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BGT offset
static  void thumbDC(u32 opcode)
{
  if(!Z_FLAG && (N_FLAG == V_FLAG)) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = codeTicksAccessSeq16(bus.armNextPC) + codeTicksAccessSeq16(bus.armNextPC) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// BLE offset
static  void thumbDD(u32 opcode)
{
  if(Z_FLAG || (N_FLAG != V_FLAG)) {
    bus.reg[15].I += ((s8)(opcode & 0xFF)) << 1;
    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH;
    clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
        codeTicksAccess(bus.armNextPC, BITS_16)+3;
    bus.busPrefetchCount=0;
  }
}

// SWI, B, BL /////////////////////////////////////////////////////////////

// SWI #comment
static  void thumbDF(u32 opcode)
{
  u32 address = 0;
  clockTicks = ((codeTicksAccessSeq16(address)) << 1) +
      codeTicksAccess(address, BITS_16)+3;
  bus.busPrefetchCount=0;
  CPUSoftwareInterrupt(opcode & 0xFF);
}

// B offset
static  void thumbE0(u32 opcode)
{
  int offset = (opcode & 0x3FF) << 1;
  if(opcode & 0x0400)
    offset |= 0xFFFFF800;
  bus.reg[15].I += offset;
  bus.armNextPC = bus.reg[15].I;
  bus.reg[15].I += 2;
  THUMB_PREFETCH;
  clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
      codeTicksAccess(bus.armNextPC, BITS_16) + 3;
  bus.busPrefetchCount=0;
}

// BLL #offset (forward)
static  void thumbF0(u32 opcode)
{
  int offset = (opcode & 0x7FF);
  bus.reg[14].I = bus.reg[15].I + (offset << 12);
  clockTicks = codeTicksAccessSeq16(bus.armNextPC) + 1;
}

// BLL #offset (backward)
static  void thumbF4(u32 opcode)
{
  int offset = (opcode & 0x7FF);
  bus.reg[14].I = bus.reg[15].I + ((offset << 12) | 0xFF800000);
  clockTicks = codeTicksAccessSeq16(bus.armNextPC) + 1;
}

// BLH #offset
static  void thumbF8(u32 opcode)
{
  int offset = (opcode & 0x7FF);
  u32 temp = bus.reg[15].I-2;
  bus.reg[15].I = (bus.reg[14].I + (offset<<1))&0xFFFFFFFE;
  bus.armNextPC = bus.reg[15].I;
  bus.reg[15].I += 2;
  bus.reg[14].I = temp|1;
  THUMB_PREFETCH;
  clockTicks = ((codeTicksAccessSeq16(bus.armNextPC)) << 1) +
      codeTicksAccess(bus.armNextPC, BITS_16) + 3;
  bus.busPrefetchCount = 0;
}

// Instruction table //////////////////////////////////////////////////////

typedef  void (*insnfunc_t)(u32 opcode);
#define thumbUI thumbUnknownInsn
#define thumbBP thumbUnknownInsn
static insnfunc_t thumbInsnTable[1024] = {
  thumb00_00,thumb00_01,thumb00_02,thumb00_03,thumb00_04,thumb00_05,thumb00_06,thumb00_07,  // 00
  thumb00_08,thumb00_09,thumb00_0A,thumb00_0B,thumb00_0C,thumb00_0D,thumb00_0E,thumb00_0F,
  thumb00_10,thumb00_11,thumb00_12,thumb00_13,thumb00_14,thumb00_15,thumb00_16,thumb00_17,
  thumb00_18,thumb00_19,thumb00_1A,thumb00_1B,thumb00_1C,thumb00_1D,thumb00_1E,thumb00_1F,
  thumb08_00,thumb08_01,thumb08_02,thumb08_03,thumb08_04,thumb08_05,thumb08_06,thumb08_07,  // 08
  thumb08_08,thumb08_09,thumb08_0A,thumb08_0B,thumb08_0C,thumb08_0D,thumb08_0E,thumb08_0F,
  thumb08_10,thumb08_11,thumb08_12,thumb08_13,thumb08_14,thumb08_15,thumb08_16,thumb08_17,
  thumb08_18,thumb08_19,thumb08_1A,thumb08_1B,thumb08_1C,thumb08_1D,thumb08_1E,thumb08_1F,
  thumb10_00,thumb10_01,thumb10_02,thumb10_03,thumb10_04,thumb10_05,thumb10_06,thumb10_07,  // 10
  thumb10_08,thumb10_09,thumb10_0A,thumb10_0B,thumb10_0C,thumb10_0D,thumb10_0E,thumb10_0F,
  thumb10_10,thumb10_11,thumb10_12,thumb10_13,thumb10_14,thumb10_15,thumb10_16,thumb10_17,
  thumb10_18,thumb10_19,thumb10_1A,thumb10_1B,thumb10_1C,thumb10_1D,thumb10_1E,thumb10_1F,
  thumb18_0,thumb18_1,thumb18_2,thumb18_3,thumb18_4,thumb18_5,thumb18_6,thumb18_7,          // 18
  thumb1A_0,thumb1A_1,thumb1A_2,thumb1A_3,thumb1A_4,thumb1A_5,thumb1A_6,thumb1A_7,
  thumb1C_0,thumb1C_1,thumb1C_2,thumb1C_3,thumb1C_4,thumb1C_5,thumb1C_6,thumb1C_7,
  thumb1E_0,thumb1E_1,thumb1E_2,thumb1E_3,thumb1E_4,thumb1E_5,thumb1E_6,thumb1E_7,
  thumb20,thumb20,thumb20,thumb20,thumb21,thumb21,thumb21,thumb21,  // 20
  thumb22,thumb22,thumb22,thumb22,thumb23,thumb23,thumb23,thumb23,
  thumb24,thumb24,thumb24,thumb24,thumb25,thumb25,thumb25,thumb25,
  thumb26,thumb26,thumb26,thumb26,thumb27,thumb27,thumb27,thumb27,
  thumb28,thumb28,thumb28,thumb28,thumb29,thumb29,thumb29,thumb29,  // 28
  thumb2A,thumb2A,thumb2A,thumb2A,thumb2B,thumb2B,thumb2B,thumb2B,
  thumb2C,thumb2C,thumb2C,thumb2C,thumb2D,thumb2D,thumb2D,thumb2D,
  thumb2E,thumb2E,thumb2E,thumb2E,thumb2F,thumb2F,thumb2F,thumb2F,
  thumb30,thumb30,thumb30,thumb30,thumb31,thumb31,thumb31,thumb31,  // 30
  thumb32,thumb32,thumb32,thumb32,thumb33,thumb33,thumb33,thumb33,
  thumb34,thumb34,thumb34,thumb34,thumb35,thumb35,thumb35,thumb35,
  thumb36,thumb36,thumb36,thumb36,thumb37,thumb37,thumb37,thumb37,
  thumb38,thumb38,thumb38,thumb38,thumb39,thumb39,thumb39,thumb39,  // 38
  thumb3A,thumb3A,thumb3A,thumb3A,thumb3B,thumb3B,thumb3B,thumb3B,
  thumb3C,thumb3C,thumb3C,thumb3C,thumb3D,thumb3D,thumb3D,thumb3D,
  thumb3E,thumb3E,thumb3E,thumb3E,thumb3F,thumb3F,thumb3F,thumb3F,
  thumb40_0,thumb40_1,thumb40_2,thumb40_3,thumb41_0,thumb41_1,thumb41_2,thumb41_3,  // 40
  thumb42_0,thumb42_1,thumb42_2,thumb42_3,thumb43_0,thumb43_1,thumb43_2,thumb43_3,
  thumbUI,thumb44_1,thumb44_2,thumb44_3,thumbUI,thumb45_1,thumb45_2,thumb45_3,
  thumbUI,thumb46_1,thumb46_2,thumb46_3,thumb47,thumb47,thumbUI,thumbUI,
  thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,  // 48
  thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,
  thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,
  thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,thumb48,
  thumb50,thumb50,thumb50,thumb50,thumb50,thumb50,thumb50,thumb50,  // 50
  thumb52,thumb52,thumb52,thumb52,thumb52,thumb52,thumb52,thumb52,
  thumb54,thumb54,thumb54,thumb54,thumb54,thumb54,thumb54,thumb54,
  thumb56,thumb56,thumb56,thumb56,thumb56,thumb56,thumb56,thumb56,
  thumb58,thumb58,thumb58,thumb58,thumb58,thumb58,thumb58,thumb58,  // 58
  thumb5A,thumb5A,thumb5A,thumb5A,thumb5A,thumb5A,thumb5A,thumb5A,
  thumb5C,thumb5C,thumb5C,thumb5C,thumb5C,thumb5C,thumb5C,thumb5C,
  thumb5E,thumb5E,thumb5E,thumb5E,thumb5E,thumb5E,thumb5E,thumb5E,
  thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,  // 60
  thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,
  thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,
  thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,thumb60,
  thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,  // 68
  thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,
  thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,
  thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,thumb68,
  thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,  // 70
  thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,
  thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,
  thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,thumb70,
  thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,  // 78
  thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,
  thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,
  thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,thumb78,
  thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,  // 80
  thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,
  thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,
  thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,thumb80,
  thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,  // 88
  thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,
  thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,
  thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,thumb88,
  thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,  // 90
  thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,
  thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,
  thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,thumb90,
  thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,  // 98
  thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,
  thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,
  thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,thumb98,
  thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,  // A0
  thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,
  thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,
  thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,thumbA0,
  thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,  // A8
  thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,
  thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,
  thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,thumbA8,
  thumbB0,thumbB0,thumbB0,thumbB0,thumbUI,thumbUI,thumbUI,thumbUI,  // B0
  thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,
  thumbB4,thumbB4,thumbB4,thumbB4,thumbB5,thumbB5,thumbB5,thumbB5,
  thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,
  thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,  // B8
  thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,
  thumbBC,thumbBC,thumbBC,thumbBC,thumbBD,thumbBD,thumbBD,thumbBD,
  thumbBP,thumbBP,thumbBP,thumbBP,thumbUI,thumbUI,thumbUI,thumbUI,
  thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,  // C0
  thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,
  thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,
  thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,thumbC0,
  thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,  // C8
  thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,
  thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,
  thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,thumbC8,
  thumbD0,thumbD0,thumbD0,thumbD0,thumbD1,thumbD1,thumbD1,thumbD1,  // D0
  thumbD2,thumbD2,thumbD2,thumbD2,thumbD3,thumbD3,thumbD3,thumbD3,
  thumbD4,thumbD4,thumbD4,thumbD4,thumbD5,thumbD5,thumbD5,thumbD5,
  thumbD6,thumbD6,thumbD6,thumbD6,thumbD7,thumbD7,thumbD7,thumbD7,
  thumbD8,thumbD8,thumbD8,thumbD8,thumbD9,thumbD9,thumbD9,thumbD9,  // D8
  thumbDA,thumbDA,thumbDA,thumbDA,thumbDB,thumbDB,thumbDB,thumbDB,
  thumbDC,thumbDC,thumbDC,thumbDC,thumbDD,thumbDD,thumbDD,thumbDD,
  thumbUI,thumbUI,thumbUI,thumbUI,thumbDF,thumbDF,thumbDF,thumbDF,
  thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,  // E0
  thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,
  thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,
  thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,thumbE0,
  thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,  // E8
  thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,
  thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,
  thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,thumbUI,
  thumbF0,thumbF0,thumbF0,thumbF0,thumbF0,thumbF0,thumbF0,thumbF0,  // F0
  thumbF0,thumbF0,thumbF0,thumbF0,thumbF0,thumbF0,thumbF0,thumbF0,
  thumbF4,thumbF4,thumbF4,thumbF4,thumbF4,thumbF4,thumbF4,thumbF4,
  thumbF4,thumbF4,thumbF4,thumbF4,thumbF4,thumbF4,thumbF4,thumbF4,
  thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,  // F8
  thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,
  thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,
  thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,thumbF8,
};

// Wrapper routine (execution loop) ///////////////////////////////////////


int thumbExecute()
{
	#ifdef USE_CACHE_PREFETCH
		// cache the clockTicks, it's used during operations and generates LHS without it
		#ifdef __ANDROID__
			prefetch(&clockTicks);
		#else
			 __dcbt(&clockTicks);
		#endif
	#endif
	 
	int ct = 0;
	
  do {
    
    clockTicks = 0;

    //if ((bus.armNextPC & 0x0803FFFF) == 0x08020000)
    //    bus.busPrefetchCount=0x100;

    u32 opcode = cpuPrefetch[0];
    cpuPrefetch[0] = cpuPrefetch[1];

    bus.busPrefetch = false;
    if (bus.busPrefetchCount & 0xFFFFFF00)
      bus.busPrefetchCount = 0x100 | (bus.busPrefetchCount & 0xFF);
    
    u32 oldArmNextPC = bus.armNextPC;

    bus.armNextPC = bus.reg[15].I;
    bus.reg[15].I += 2;
    THUMB_PREFETCH_NEXT;

    (*thumbInsnTable[opcode>>6])(opcode);

	ct = clockTicks;

	if (ct < 0)
      return 0;

	/// better pipelining
    if (ct==0)
      clockTicks = codeTicksAccessSeq16(oldArmNextPC) + 1;

    cpuTotalTicks += clockTicks;


#ifdef USE_SWITICKS
  } while (cpuTotalTicks < cpuNextEvent && !armState && !holdState && !SWITicks);
#else
  } while ((cpuTotalTicks < cpuNextEvent) & ~armState & ~holdState);
#endif
  return 1;
}
