///////////////////////////////////////////////////////////////
// File:  v810_opt.h
// 
// Description:  Defines used in v810_dis.cpp
//

#ifndef V810_OPT_H_
#define V810_OPT_H_  

#define sign_26(num) ((uint32)sign_x_to_s32(26, num))
#define sign_16(num) ((uint32)(int16)(num))
#define sign_14(num) ((uint32)sign_x_to_s32(14, num))
#define sign_12(num) ((uint32)sign_x_to_s32(12, num))
#define sign_9(num) ((uint32)sign_x_to_s32(9, num))
#define sign_8(_value) ((uint32)(int8)(_value))
#define sign_5(num) ((uint32)sign_x_to_s32(5, num))

///////////////////////////////////////////////////////////////////
// Define Modes
#define AM_I    0x01
#define AM_II   0x02
#define AM_III  0x03
#define AM_IV   0x04
#define AM_V    0x05
#define AM_VIa  0x06    // Mode6 form1
#define AM_VIb  0x0A    // Mode6 form2
#define AM_VII  0x07
#define AM_VIII 0x08
#define AM_IX   0x09
#define AM_BSTR 0x0B  // Bit String Instructions
#define AM_FPP  0x0C  // Floating Point Instructions
#define AM_UDEF 0x0D  // Unknown/Undefined Instructions

///////////////////////////////////////////////////////////////////
// Table of Instructions Address Modes

static const int addr_mode[80] = {
  AM_I,  AM_I,  AM_I,  AM_I,  AM_I,  AM_I,  AM_I,  AM_I,
  AM_I,  AM_I,  AM_I,  AM_I,  AM_I,  AM_I,  AM_I,  AM_I,
  AM_II, AM_II, AM_II, AM_II, AM_II, AM_II, AM_II, AM_II,
  AM_II, AM_IX, AM_IX, AM_UDEF, AM_II, AM_II, AM_II, AM_BSTR,
  AM_UDEF, AM_UDEF, AM_UDEF, AM_UDEF, AM_UDEF, AM_UDEF, AM_UDEF, AM_UDEF,
  AM_V,  AM_V,  AM_IV, AM_IV, AM_V,  AM_V,  AM_V,  AM_V,
  AM_VIa, AM_VIa, AM_UDEF, AM_VIa, AM_VIb, AM_VIb, AM_UDEF, AM_VIb,
  AM_VIa, AM_VIa, AM_VIa, AM_VIa, AM_VIb, AM_VIb, AM_FPP, AM_VIb, 
  AM_III, AM_III, AM_III, AM_III, AM_III, AM_III, AM_III, AM_III,
  AM_III, AM_III, AM_III, AM_III, AM_III, AM_III, AM_III, AM_III
};
// All instructions greater than 0x50 are undefined (this should not be posible of cource)


///////////////////////////////////////////////////////////////////
// Opcodes for V810 Instruction set
#define         MOV                             0x00
#define         ADD                             0x01
#define         SUB                             0x02
#define         CMP                             0x03
#define         SHL                             0x04
#define         SHR                             0x05
#define         JMP                             0x06
#define         SAR                             0x07
#define         MUL                             0x08
#define         DIV                             0x09
#define         MULU                            0x0A
#define         DIVU                            0x0B
#define         OR                              0x0C
#define         AND                             0x0D
#define         XOR                             0x0E
#define         NOT                             0x0F
#define         MOV_I                           0x10
#define         ADD_I                           0x11
#define         SETF                            0x12
#define         CMP_I                           0x13
#define         SHL_I                           0x14
#define         SHR_I                           0x15
#define         EI                              0x16
#define         SAR_I                           0x17
#define         TRAP                            0x18
#define         RETI                            0x19
#define         HALT                            0x1A
                                              //0x1B
#define         LDSR                            0x1C
#define         STSR                            0x1D
#define         DI                              0x1E
#define         BSTR                            0x1F  //Special Bit String Inst
                                              //0x20 - 0x27  // Lost to Branch Instructions
#define         MOVEA                           0x28
#define         ADDI                            0x29
#define         JR                              0x2A
#define         JAL                             0x2B
#define         ORI                             0x2C
#define         ANDI                            0x2D
#define         XORI                            0x2E
#define         MOVHI                           0x2F
#define         LD_B                            0x30
#define         LD_H                            0x31
                                              //0x32
#define         LD_W                            0x33
#define         ST_B                            0x34
#define         ST_H                            0x35
                                              //0x36
#define         ST_W                            0x37
#define         IN_B                            0x38
#define         IN_H                            0x39
#define         CAXI                            0x3A
#define         IN_W                            0x3B
#define         OUT_B                           0x3C
#define         OUT_H                           0x3D
#define         FPP                             0x3E  //Special Float Inst
#define         OUT_W                           0x3F


//      Branch Instructions ( Extended opcode only for Branch command)
//  Common instrcutions commented out

#define         BV                              0x40
#define         BL                              0x41
#define         BE                              0x42
#define         BNH                             0x43
#define         BN                              0x44
#define         BR                              0x45
#define         BLT                             0x46
#define         BLE                             0x47
#define         BNV                             0x48
#define         BNL                             0x49
#define         BNE                             0x4A
#define         BH                              0x4B
#define         BP                              0x4C
#define         NOP                             0x4D
#define         BGE                             0x4E
#define         BGT                             0x4F

//#define       BC                              0x41
//#define       BZ                              0x42
//#define       BNC                             0x49
//#define       BNZ                             0x4A

//  Bit String Subopcodes
#define         SCH0BSU                         0x00
#define         SCH0BSD                         0x01
#define         SCH1BSU                         0x02
#define         SCH1BSD                         0x03

#define         ORBSU                           0x08
#define         ANDBSU                          0x09
#define         XORBSU                          0x0A
#define         MOVBSU                          0x0B
#define         ORNBSU                          0x0C
#define         ANDNBSU                         0x0D
#define         XORNBSU                         0x0E
#define         NOTBSU                          0x0F


//  Floating Point Subopcodes
#define         CMPF_S                          0x00

#define         CVT_WS                          0x02
#define         CVT_SW                          0x03
#define         ADDF_S                          0x04
#define         SUBF_S                          0x05
#define         MULF_S                          0x06
#define         DIVF_S                          0x07
#define         XB                              0x08
#define         XH                              0x09
#define         REV                             0x0A
#define         TRNC_SW                         0x0B
#define         MPYHW                           0x0C

#endif //DEFINE_H

