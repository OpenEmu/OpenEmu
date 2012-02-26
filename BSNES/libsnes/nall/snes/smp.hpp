#ifndef NALL_SNES_SMP_HPP
#define NALL_SNES_SMP_HPP

namespace nall {

struct SNESSMP {
  enum : unsigned {
    Implied,          //
    TVector,          //0
    Direct,           //$00
    DirectRelative,   //$00,+/-$00
    ADirect,          //a,$00
    AAbsolute,        //a,$0000
    AIX,              //a,(x)
    AIDirectX,        //a,($00+x)
    AConstant,        //a,#$00
    DirectDirect,     //$00,$00
    CAbsoluteBit,     //c,$0000:0
    Absolute,         //$0000
    P,                //p
    AbsoluteA,        //$0000,a
    Relative,         //+/-$00
    ADirectX,         //a,$00+x
    AAbsoluteX,       //a,$0000+x
    AAbsoluteY,       //a,$0000+y
    AIDirectY,        //a,($00)+y
    DirectConstant,   //$00,#$00
    IXIY,             //(x),(y)
    DirectX,          //$00+x
    A,                //a
    X,                //x
    XAbsolute,        //x,$0000
    IAbsoluteX,       //($0000+x)
    CNAbsoluteBit,    //c,!$0000:0
    XDirect,          //x,$00
    PVector,          //$ff00
    YaDirect,         //ya,$00
    XA,               //x,a
    YAbsolute,        //y,$0000
    Y,                //y
    AX,               //a,x
    YDirect,          //y,$00
    YConstant,        //y,#$00
    XSp,              //x,sp
    YaX,              //ya,x
    IXPA,             //(x)+,a
    SpX,              //sp,x
    AIXP,             //a,(x)+
    DirectA,          //$00,a
    IXA,              //(x),a
    IDirectXA,        //($00+x),a
    XConstant,        //x,#$00
    AbsoluteX,        //$0000,x
    AbsoluteBitC,     //$0000:0,c
    DirectY,          //$00,y
    AbsoluteY,        //$0000,y
    Ya,               //ya
    DirectXA,         //$00+x,a
    AbsoluteXA,       //$0000+x,a
    AbsoluteYA,       //$0000+y,a
    IDirectYA,        //($00)+y,a
    DirectYX,         //$00+y,x
    DirectYa,         //$00,ya
    DirectXY,         //$00+x,y
    AY,               //a,y
    DirectXRelative,  //$00+x,+/-$00
    XDirectY,         //x,$00+y
    YDirectX,         //y,$00+x
    YA,               //y,a
    YRelative,        //y,+/-$00
  };

  struct OpcodeInfo {
    char name[6];
    unsigned mode;
  };

  static const OpcodeInfo opcodeInfo[256];

  static unsigned getOpcodeLength(uint8_t opcode);
  static string disassemble(uint16_t pc, uint8_t opcode, uint8_t pl, uint8_t ph);
  static string disassemble(uint16_t pc, bool p, uint8_t opcode, uint8_t pl, uint8_t ph);
};

const SNESSMP::OpcodeInfo SNESSMP::opcodeInfo[256] = {
  //0x00 - 0x0f
  { "nop  ", Implied },
  { "tcall", TVector },
  { "set0 ", Direct },
  { "bbs0 ", DirectRelative },

  { "or   ", ADirect },
  { "or   ", AAbsolute },
  { "or   ", AIX },
  { "or   ", AIDirectX },

  { "or   ", AConstant },
  { "or   ", DirectDirect },
  { "or1  ", CAbsoluteBit },
  { "asl  ", Direct },

  { "asl  ", Absolute },
  { "push ", P },
  { "tset ", AbsoluteA },
  { "brk  ", Implied },

  //0x10 - 0x1f
  { "bpl  ", Relative },
  { "tcall", TVector },
  { "clr0 ", Direct },
  { "bbc0 ", DirectRelative },

  { "or   ", ADirectX },
  { "or   ", AAbsoluteX },
  { "or   ", AAbsoluteY },
  { "or   ", AIDirectY },

  { "or   ", DirectConstant },
  { "or   ", IXIY },
  { "decw ", Direct },
  { "asl  ", DirectX },

  { "asl  ", A },
  { "dec  ", X },
  { "cmp  ", XAbsolute },
  { "jmp  ", IAbsoluteX },

  //0x20 - 0x2f
  { "clrp ", Implied },
  { "tcall", TVector },
  { "set1 ", Direct },
  { "bbs1 ", DirectRelative },

  { "and  ", ADirect },
  { "and  ", AAbsolute },
  { "and  ", AIX },
  { "and  ", AIDirectX },

  { "and  ", AConstant },
  { "and  ", DirectDirect },
  { "or1  ", CNAbsoluteBit },
  { "rol  ", Direct },

  { "rol  ", Absolute },
  { "push ", A },
  { "cbne ", DirectRelative },
  { "bra  ", Relative },

  //0x30 - 0x3f
  { "bmi  ", Relative },
  { "tcall", TVector },
  { "clr1 ", Direct },
  { "bbc1 ", DirectRelative },

  { "and  ", ADirectX },
  { "and  ", AAbsoluteX },
  { "and  ", AAbsoluteY },
  { "and  ", AIDirectY },

  { "and  ", DirectConstant },
  { "and  ", IXIY },
  { "incw ", Direct },
  { "rol  ", DirectX },

  { "rol  ", A },
  { "inc  ", X },
  { "cmp  ", XDirect },
  { "call ", Absolute },

  //0x40 - 0x4f
  { "setp ", Implied },
  { "tcall", TVector },
  { "set2 ", Direct },
  { "bbs2 ", DirectRelative },

  { "eor  ", ADirect },
  { "eor  ", AAbsolute },
  { "eor  ", AIX },
  { "eor  ", AIDirectX },

  { "eor  ", AConstant },
  { "eor  ", DirectDirect },
  { "and1 ", CAbsoluteBit },
  { "lsr  ", Direct },

  { "lsr  ", Absolute },
  { "push ", X },
  { "tclr ", AbsoluteA },
  { "pcall", PVector },

  //0x50 - 0x5f
  { "bvc  ", Relative },
  { "tcall", TVector },
  { "clr2 ", Direct },
  { "bbc2 ", DirectRelative },

  { "eor  ", ADirectX },
  { "eor  ", AAbsoluteX },
  { "eor  ", AAbsoluteY },
  { "eor  ", AIDirectY },

  { "eor  ", DirectConstant },
  { "eor  ", IXIY },
  { "cmpw ", YaDirect },
  { "lsr  ", DirectX },

  { "lsr  ", A },
  { "mov  ", XA },
  { "cmp  ", YAbsolute },
  { "jmp  ", Absolute },

  //0x60 - 0x6f
  { "clrc ", Implied },
  { "tcall", TVector },
  { "set3 ", Direct },
  { "bbs3 ", DirectRelative },

  { "cmp  ", ADirect },
  { "cmp  ", AAbsolute },
  { "cmp  ", AIX },
  { "cmp  ", AIDirectX },

  { "cmp  ", AConstant },
  { "cmp  ", DirectDirect },
  { "and1 ", CNAbsoluteBit },
  { "ror  ", Direct },

  { "ror  ", Absolute },
  { "push ", Y },
  { "dbnz ", DirectRelative },
  { "ret  ", Implied },

  //0x70 - 0x7f
  { "bvs  ", Relative },
  { "tcall", TVector },
  { "clr3 ", Direct },
  { "bbc3 ", DirectRelative },

  { "cmp  ", ADirectX },
  { "cmp  ", AAbsoluteX },
  { "cmp  ", AAbsoluteY },
  { "cmp  ", AIDirectY },

  { "cmp  ", DirectConstant },
  { "cmp  ", IXIY },
  { "addw ", YaDirect },
  { "ror  ", DirectX },

  { "ror  ", A },
  { "mov  ", AX },
  { "cmp  ", YDirect },
  { "reti ", Implied },

  //0x80 - 0x8f
  { "setc ", Implied },
  { "tcall", TVector },
  { "set4 ", Direct },
  { "bbs4 ", DirectRelative },

  { "adc  ", ADirect },
  { "adc  ", AAbsolute },
  { "adc  ", AIX },
  { "adc  ", AIDirectX },

  { "adc  ", AConstant },
  { "adc  ", DirectDirect },
  { "eor1 ", CAbsoluteBit },
  { "dec  ", Direct },

  { "dec  ", Absolute },
  { "mov  ", YConstant },
  { "pop  ", P },
  { "mov  ", DirectConstant },

  //0x90 - 0x9f
  { "bcc  ", Relative },
  { "tcall", TVector },
  { "clr4 ", Direct },
  { "bbc4 ", DirectRelative },

  { "adc  ", ADirectX },
  { "adc  ", AAbsoluteX },
  { "adc  ", AAbsoluteY },
  { "adc  ", AIDirectY },

  { "adc  ", DirectRelative },
  { "adc  ", IXIY },
  { "subw ", YaDirect },
  { "dec  ", DirectX },

  { "dec  ", A },
  { "mov  ", XSp },
  { "div  ", YaX },
  { "xcn  ", A },

  //0xa0 - 0xaf
  { "ei   ", Implied },
  { "tcall", TVector },
  { "set5 ", Direct },
  { "bbs5 ", DirectRelative },

  { "sbc  ", ADirect },
  { "sbc  ", AAbsolute },
  { "sbc  ", AIX },
  { "sbc  ", AIDirectX },

  { "sbc  ", AConstant },
  { "sbc  ", DirectDirect },
  { "mov1 ", CAbsoluteBit },
  { "inc  ", Direct },

  { "inc  ", Absolute },
  { "cmp  ", YConstant },
  { "pop  ", A },
  { "mov  ", IXPA },

  //0xb0 - 0xbf
  { "bcs  ", Relative },
  { "tcall", TVector },
  { "clr5 ", Direct },
  { "bbc5 ", DirectRelative },

  { "sbc  ", ADirectX },
  { "sbc  ", AAbsoluteX },
  { "sbc  ", AAbsoluteY },
  { "sbc  ", AIDirectY },

  { "sbc  ", DirectConstant },
  { "sbc  ", IXIY },
  { "movw ", YaDirect },
  { "inc  ", DirectX },

  { "inc  ", A },
  { "mov  ", SpX },
  { "das  ", A },
  { "mov  ", AIXP },

  //0xc0 - 0xcf
  { "di   ", Implied },
  { "tcall", TVector },
  { "set6 ", Direct },
  { "bbs6 ", DirectRelative },

  { "mov  ", DirectA },
  { "mov  ", AbsoluteA },
  { "mov  ", IXA },
  { "mov  ", IDirectXA },

  { "cmp  ", XConstant },
  { "mov  ", AbsoluteX },
  { "mov1 ", AbsoluteBitC },
  { "mov  ", DirectY },

  { "mov  ", AbsoluteY },
  { "mov  ", XConstant },
  { "pop  ", X },
  { "mul  ", Ya },

  //0xd0 - 0xdf
  { "bne  ", Relative },
  { "tcall", TVector },
  { "clr6 ", Relative },
  { "bbc6 ", DirectRelative },

  { "mov  ", DirectXA },
  { "mov  ", AbsoluteXA },
  { "mov  ", AbsoluteYA },
  { "mov  ", IDirectYA },

  { "mov  ", DirectX },
  { "mov  ", DirectYX },
  { "movw ", DirectYa },
  { "mov  ", DirectXY },

  { "dec  ", Y },
  { "mov  ", AY },
  { "cbne ", DirectXRelative },
  { "daa  ", A },

  //0xe0 - 0xef
  { "clrv ", Implied },
  { "tcall", TVector },
  { "set7 ", Direct },
  { "bbs7 ", DirectRelative },

  { "mov  ", ADirect },
  { "mov  ", AAbsolute },
  { "mov  ", AIX },
  { "mov  ", AIDirectX },

  { "mov  ", AConstant },
  { "mov  ", XAbsolute },
  { "not1 ", CAbsoluteBit },
  { "mov  ", YDirect },

  { "mov  ", YAbsolute },
  { "notc ", Implied },
  { "pop  ", Y },
  { "sleep", Implied },

  //0xf0 - 0xff
  { "beq  ", Relative },
  { "tcall", TVector },
  { "clr7 ", Direct },
  { "bbc7 ", DirectRelative },

  { "mov  ", ADirectX },
  { "mov  ", AAbsoluteX },
  { "mov  ", AAbsoluteY },
  { "mov  ", AIDirectY },

  { "mov  ", XDirect },
  { "mov  ", XDirectY },
  { "mov  ", DirectDirect },
  { "mov  ", YDirectX },

  { "inc  ", Y },
  { "mov  ", YA },
  { "dbz  ", YRelative },
  { "stop ", Implied },
};

inline unsigned SNESSMP::getOpcodeLength(uint8_t opcode) {
  switch(opcodeInfo[opcode].mode) { default:
    case Implied:         return 1;  //
    case TVector:         return 1;  //0
    case Direct:          return 2;  //$00
    case DirectRelative:  return 3;  //$00,+/-$00
    case ADirect:         return 2;  //a,$00
    case AAbsolute:       return 3;  //a,$0000
    case AIX:             return 1;  //a,(x)
    case AIDirectX:       return 2;  //a,($00+x)
    case AConstant:       return 2;  //a,#$00
    case DirectDirect:    return 3;  //$00,$00
    case CAbsoluteBit:    return 3;  //c,$0000:0
    case Absolute:        return 3;  //$0000
    case P:               return 1;  //p
    case AbsoluteA:       return 3;  //$0000,a
    case Relative:        return 2;  //+/-$00
    case ADirectX:        return 2;  //a,$00+x
    case AAbsoluteX:      return 3;  //a,$0000+x
    case AAbsoluteY:      return 3;  //a,$0000+y
    case AIDirectY:       return 2;  //a,($00)+y
    case DirectConstant:  return 3;  //$00,#$00
    case IXIY:            return 1;  //(x),(y)
    case DirectX:         return 2;  //$00+x
    case A:               return 1;  //a
    case X:               return 1;  //x
    case XAbsolute:       return 3;  //x,$0000
    case IAbsoluteX:      return 3;  //($0000+x)
    case CNAbsoluteBit:   return 3;  //c,!$0000:0
    case XDirect:         return 2;  //x,$00
    case PVector:         return 2;  //$ff00
    case YaDirect:        return 2;  //ya,$00
    case XA:              return 1;  //x,a
    case YAbsolute:       return 3;  //y,$0000
    case Y:               return 1;  //y
    case AX:              return 1;  //a,x
    case YDirect:         return 2;  //y,$00
    case YConstant:       return 2;  //y,#$00
    case XSp:             return 1;  //x,sp
    case YaX:             return 1;  //ya,x
    case IXPA:            return 1;  //(x)+,a
    case SpX:             return 1;  //sp,x
    case AIXP:            return 1;  //a,(x)+
    case DirectA:         return 2;  //$00,a
    case IXA:             return 1;  //(x),a
    case IDirectXA:       return 2;  //($00+x),a
    case XConstant:       return 2;  //x,#$00
    case AbsoluteX:       return 3;  //$0000,x
    case AbsoluteBitC:    return 3;  //$0000:0,c
    case DirectY:         return 2;  //$00,y
    case AbsoluteY:       return 3;  //$0000,y
    case Ya:              return 1;  //ya
    case DirectXA:        return 2;  //$00+x,a
    case AbsoluteXA:      return 3;  //$0000+x,a
    case AbsoluteYA:      return 3;  //$0000+y,a
    case IDirectYA:       return 2;  //($00)+y,a
    case DirectYX:        return 2;  //$00+y,x
    case DirectYa:        return 2;  //$00,ya
    case DirectXY:        return 2;  //$00+x,y
    case AY:              return 1;  //a,y
    case DirectXRelative: return 3;  //$00+x,+/-$00
    case XDirectY:        return 2;  //x,$00+y
    case YDirectX:        return 2;  //y,$00+x
    case YA:              return 1;  //y,a
    case YRelative:       return 2;  //y,+/-$00
  }
}

inline string SNESSMP::disassemble(uint16_t pc, uint8_t opcode, uint8_t pl, uint8_t ph) {
  string name = opcodeInfo[opcode].name;
  unsigned mode = opcodeInfo[opcode].mode;
  unsigned pa = (ph << 8) + pl;

  if(mode == Implied) return name;
  if(mode == TVector) return { name, " ", opcode >> 4 };
  if(mode == Direct) return { name, " $", hex<2>(pl) };
  if(mode == DirectRelative) return { name, " $", hex<2>(pl), ",$", hex<4>(pc + 3 + (int8_t)ph) };
  if(mode == ADirect) return { name, " a,$", hex<2>(pl) };
  if(mode == AAbsolute) return { name, " a,$", hex<4>(pa) };
  if(mode == AIX) return { name, "a,(x)" };
  if(mode == AIDirectX) return { name, " a,($", hex<2>(pl), "+x)" };
  if(mode == AConstant) return { name, " a,#$", hex<2>(pl) };
  if(mode == DirectDirect) return { name, " $", hex<2>(ph), ",$", hex<2>(pl) };
  if(mode == CAbsoluteBit) return { name, " c,$", hex<4>(pa & 0x1fff), ":", pa >> 13 };
  if(mode == Absolute) return { name, " $", hex<4>(pa) };
  if(mode == P) return { name, " p" };
  if(mode == AbsoluteA) return { name, " $", hex<4>(pa), ",a" };
  if(mode == Relative) return { name, " $", hex<4>(pc + 2 + (int8_t)pl) };
  if(mode == ADirectX) return { name, " a,$", hex<2>(pl), "+x" };
  if(mode == AAbsoluteX) return { name, " a,$", hex<4>(pa), "+x" };
  if(mode == AAbsoluteY) return { name, " a,$", hex<4>(pa), "+y" };
  if(mode == AIDirectY) return { name, " a,($", hex<2>(pl), ")+y" };
  if(mode == DirectConstant) return { name, " $", hex<2>(ph), ",#$", hex<2>(pl) };
  if(mode == IXIY) return { name, " (x),(y)" };
  if(mode == DirectX) return { name, " $", hex<2>(pl), "+x" };
  if(mode == A) return { name, " a" };
  if(mode == X) return { name, " x" };
  if(mode == XAbsolute) return { name, " x,$", hex<4>(pa) };
  if(mode == IAbsoluteX) return { name, " ($", hex<4>(pa), "+x)" };
  if(mode == CNAbsoluteBit) return { name, " c,!$", hex<4>(pa & 0x1fff), ":", pa >> 13 };
  if(mode == XDirect) return { name, " x,$", hex<2>(pl) };
  if(mode == PVector) return { name, " $ff", hex<2>(pl) };
  if(mode == YaDirect) return { name, " ya,$", hex<2>(pl) };
  if(mode == XA) return { name, " x,a" };
  if(mode == YAbsolute) return { name, " y,$", hex<4>(pa) };
  if(mode == Y) return { name, " y" };
  if(mode == AX) return { name, " a,x" };
  if(mode == YDirect) return { name, " y,$", hex<2>(pl) };
  if(mode == YConstant) return { name, " y,#$", hex<2>(pl) };
  if(mode == XSp) return { name, " x,sp" };
  if(mode == YaX) return { name, " ya,x" };
  if(mode == IXPA) return { name, " (x)+,a" };
  if(mode == SpX) return { name, " sp,x" };
  if(mode == AIXP) return { name, " a,(x)+" };
  if(mode == DirectA) return { name, " $", hex<2>(pl), ",a" };
  if(mode == IXA) return { name, " (x),a" };
  if(mode == IDirectXA) return { name, " ($", hex<2>(pl), "+x),a" };
  if(mode == XConstant) return { name, " x,#$", hex<2>(pl) };
  if(mode == AbsoluteX) return { name, " $", hex<4>(pa), ",x" };
  if(mode == AbsoluteBitC) return { name, " $", hex<4>(pa & 0x1fff), ":", pa >> 13, ",c" };
  if(mode == DirectY) return { name, " $", hex<2>(pl), ",y" };
  if(mode == AbsoluteY) return { name, " $", hex<4>(pa), ",y" };
  if(mode == Ya) return { name, " ya" };
  if(mode == DirectXA) return { name, " $", hex<2>(pl), "+x,a" };
  if(mode == AbsoluteXA) return { name, " $", hex<4>(pa), "+x,a" };
  if(mode == AbsoluteYA) return { name, " $", hex<4>(pa), "+y,a" };
  if(mode == IDirectYA) return { name, " ($", hex<2>(pl), ")+y,a" };
  if(mode == DirectYX) return { name, " $", hex<2>(pl), "+y,x" };
  if(mode == DirectYa) return { name, " $", hex<2>(pl), ",ya" };
  if(mode == DirectXY) return { name, " $", hex<2>(pl), "+x,y" };
  if(mode == AY) return { name, " a,y" };
  if(mode == DirectXRelative) return { name, " $", hex<2>(pl), ",$", hex<4>(pc + 3 + (int8_t)ph) };
  if(mode == XDirectY) return { name, " x,$", hex<2>(pl), "+y" };
  if(mode == YDirectX) return { name, " y,$", hex<2>(pl), "+x" };
  if(mode == YA) return { name, " y,a" };
  if(mode == YRelative) return { name, " y,$", hex<4>(pc + 2 + (int8_t)pl) };

  return "";
}

inline string SNESSMP::disassemble(uint16_t pc, bool p, uint8_t opcode, uint8_t pl, uint8_t ph) {
  string name = opcodeInfo[opcode].name;
  unsigned mode = opcodeInfo[opcode].mode;
  unsigned pdl = (p << 8) + pl;
  unsigned pdh = (p << 8) + ph;
  unsigned pa = (ph << 8) + pl;

  if(mode == Implied) return name;
  if(mode == TVector) return { name, " ", opcode >> 4 };
  if(mode == Direct) return { name, " $", hex<3>(pdl) };
  if(mode == DirectRelative) return { name, " $", hex<3>(pdl), ",$", hex<4>(pc + 3 + (int8_t)ph) };
  if(mode == ADirect) return { name, " a,$", hex<3>(pdl) };
  if(mode == AAbsolute) return { name, " a,$", hex<4>(pa) };
  if(mode == AIX) return { name, "a,(x)" };
  if(mode == AIDirectX) return { name, " a,($", hex<3>(pdl), "+x)" };
  if(mode == AConstant) return { name, " a,#$", hex<2>(pl) };
  if(mode == DirectDirect) return { name, " $", hex<3>(pdh), ",$", hex<3>(pdl) };
  if(mode == CAbsoluteBit) return { name, " c,$", hex<4>(pa & 0x1fff), ":", pa >> 13 };
  if(mode == Absolute) return { name, " $", hex<4>(pa) };
  if(mode == P) return { name, " p" };
  if(mode == AbsoluteA) return { name, " $", hex<4>(pa), ",a" };
  if(mode == Relative) return { name, " $", hex<4>(pc + 2 + (int8_t)pl) };
  if(mode == ADirectX) return { name, " a,$", hex<3>(pdl), "+x" };
  if(mode == AAbsoluteX) return { name, " a,$", hex<4>(pa), "+x" };
  if(mode == AAbsoluteY) return { name, " a,$", hex<4>(pa), "+y" };
  if(mode == AIDirectY) return { name, " a,($", hex<3>(pdl), ")+y" };
  if(mode == DirectConstant) return { name, " $", hex<3>(pdh), ",#$", hex<2>(pl) };
  if(mode == IXIY) return { name, " (x),(y)" };
  if(mode == DirectX) return { name, " $", hex<3>(pdl), "+x" };
  if(mode == A) return { name, " a" };
  if(mode == X) return { name, " x" };
  if(mode == XAbsolute) return { name, " x,$", hex<4>(pa) };
  if(mode == IAbsoluteX) return { name, " ($", hex<4>(pa), "+x)" };
  if(mode == CNAbsoluteBit) return { name, " c,!$", hex<4>(pa & 0x1fff), ":", pa >> 13 };
  if(mode == XDirect) return { name, " x,$", hex<3>(pdl) };
  if(mode == PVector) return { name, " $ff", hex<2>(pl) };
  if(mode == YaDirect) return { name, " ya,$", hex<3>(pdl) };
  if(mode == XA) return { name, " x,a" };
  if(mode == YAbsolute) return { name, " y,$", hex<4>(pa) };
  if(mode == Y) return { name, " y" };
  if(mode == AX) return { name, " a,x" };
  if(mode == YDirect) return { name, " y,$", hex<3>(pdl) };
  if(mode == YConstant) return { name, " y,#$", hex<2>(pl) };
  if(mode == XSp) return { name, " x,sp" };
  if(mode == YaX) return { name, " ya,x" };
  if(mode == IXPA) return { name, " (x)+,a" };
  if(mode == SpX) return { name, " sp,x" };
  if(mode == AIXP) return { name, " a,(x)+" };
  if(mode == DirectA) return { name, " $", hex<3>(pdl), ",a" };
  if(mode == IXA) return { name, " (x),a" };
  if(mode == IDirectXA) return { name, " ($", hex<3>(pdl), "+x),a" };
  if(mode == XConstant) return { name, " x,#$", hex<2>(pl) };
  if(mode == AbsoluteX) return { name, " $", hex<4>(pa), ",x" };
  if(mode == AbsoluteBitC) return { name, " $", hex<4>(pa & 0x1fff), ":", pa >> 13, ",c" };
  if(mode == DirectY) return { name, " $", hex<3>(pdl), ",y" };
  if(mode == AbsoluteY) return { name, " $", hex<4>(pa), ",y" };
  if(mode == Ya) return { name, " ya" };
  if(mode == DirectXA) return { name, " $", hex<3>(pdl), "+x,a" };
  if(mode == AbsoluteXA) return { name, " $", hex<4>(pa), "+x,a" };
  if(mode == AbsoluteYA) return { name, " $", hex<4>(pa), "+y,a" };
  if(mode == IDirectYA) return { name, " ($", hex<3>(pdl), ")+y,a" };
  if(mode == DirectYX) return { name, " $", hex<3>(pdl), "+y,x" };
  if(mode == DirectYa) return { name, " $", hex<3>(pdl), ",ya" };
  if(mode == DirectXY) return { name, " $", hex<3>(pdl), "+x,y" };
  if(mode == AY) return { name, " a,y" };
  if(mode == DirectXRelative) return { name, " $", hex<3>(pdl), ",$", hex<4>(pc + 3 + (int8_t)ph) };
  if(mode == XDirectY) return { name, " x,$", hex<3>(pdl), "+y" };
  if(mode == YDirectX) return { name, " y,$", hex<3>(pdl), "+x" };
  if(mode == YA) return { name, " y,a" };
  if(mode == YRelative) return { name, " y,$", hex<4>(pc + 2 + (int8_t)pl) };

  return "";
}

}

#endif
