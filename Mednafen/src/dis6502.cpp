#include "mednafen.h"
#include "dis6502.h"

Dis6502::Dis6502(bool is_cmos)
{
 IsCMOS = is_cmos;

}

Dis6502::~Dis6502()
{

}

uint8 Dis6502::Read(uint16 A)
{
 return(0);
}

uint8 Dis6502::GetX(void)
{
 return(0);
}

uint8 Dis6502::GetY(void)
{
 return(0);
}


static const char *fstrings[12]=
{ 
        "#$%02X",       // immediate
        "$%04X",        // RELATIVE(jump)
        "$%02X",        // Z
        "$%02X,X",      // Z,x
        "$%02X,Y",      // Z,y
        "$%04X",        //ABS
        "$%04X,X",      // ABS,x
        "$%04X,Y",      // ABS,y
        "($%04X)",      // IND
        "($%02X,X)",    // INX
        "($%02X),Y",    // INY
	""
};
static const int flengths[12]={1,1,1,1,1,2,2,2,2,1,1,0};

#define IMD(x)	((0<<16)|x)
#define	REL(x)	((1<<16)|x)
#define	ZP(x)	((2<<16)|x)
#define	ZPX(x)	((3<<16)|x)
#define	ZPY(x)	((4<<16)|x)
#define	ABS(x)	((5<<16)|x)
#define	ABX(x)	((6<<16)|x)
#define ABY(x)	((7<<16)|x)
#define	IND(x)	((8<<16)|x)
#define	INX(x)	((9<<16)|x)
#define	INY(x)	((10<<16)|x)
#define IMP(x)	((11<<16)|x)

typedef struct {
	const char *name;
        int type;       /* 1 for read, 2 for write, 3 for r then write. */
	int32 modes[10];
} OPS;

#define NUMOPS 56
static OPS optable[NUMOPS]=
{
 {"BRK",0,{IMP(0x00),-1}},
 {"RTI",0,{IMP(0x40),-1}},
 {"RTS",0,{IMP(0x60),-1}},
 {"PHA",2,{IMP(0x48),-1}},
 {"PHP",2,{IMP(0x08),-1}},
 {"PLA",1,{IMP(0x68),-1}},
 {"PLP",1,{IMP(0x28),-1}},
 {"JMP",0,{ABS(0x4C),IND(0x6C),-1}},
 {"JSR",0,{ABS(0x20),-1}},
 {"TAX",0,{IMP(0xAA),-1}},
 {"TXA",0,{IMP(0x8A),-1}},
 {"TAY",0,{IMP(0xA8),-1}},
 {"TYA",0,{IMP(0x98),-1}},
 {"TSX",0,{IMP(0xBA),-1}},
 {"TXS",0,{IMP(0x9A),-1}},
 {"DEX",0,{IMP(0xCA),-1}},
 {"DEY",0,{IMP(0x88),-1}},
 {"INX",0,{IMP(0xE8),-1}},
 {"INY",0,{IMP(0xC8),-1}},
 {"CLC",0,{IMP(0x18),-1}},
 {"CLD",0,{IMP(0xD8),-1}},
 {"CLI",0,{IMP(0x58),-1}},
 {"CLV",0,{IMP(0xB8),-1}},
 {"SEC",0,{IMP(0x38),-1}},
 {"SED",0,{IMP(0xF8),-1}},
 {"SEI",0,{IMP(0x78),-1}},
 {"NOP",0,{IMP(0xEA),-1}},
 {"ASL",1,{IMP(0x0a),ZP(0x06),ZPX(0x16),ABS(0x0E),ABX(0x1E),-1}},
 {"DEC",3,{ZP(0xc6),ZPX(0xd6),ABS(0xcE),ABX(0xdE),-1}},
 {"INC",3,{ZP(0xe6),ZPX(0xf6),ABS(0xeE),ABX(0xfE),-1}},
 {"LSR",3,{IMP(0x4a),ZP(0x46),ZPX(0x56),ABS(0x4E),ABX(0x5E),-1}},
 {"ROL",3,{IMP(0x2a),ZP(0x26),ZPX(0x36),ABS(0x2E),ABX(0x3E),-1}},
 {"ROR",3,{IMP(0x6a),ZP(0x66),ZPX(0x76),ABS(0x6E),ABX(0x7E),-1}},
 {"ADC",1,{IMD(0x69),ZP(0x65),ZPX(0x75),ABS(0x6D),ABX(0x7d),ABY(0x79),
	 INX(0x61),INY(0x71),-1}},
 {"AND",1,{IMD(0x29),ZP(0x25),ZPX(0x35),ABS(0x2D),ABX(0x3d),ABY(0x39),
         INX(0x21),INY(0x31),-1}},
 {"BIT",1,{ZP(0x24),ABS(0x2c),-1}},
 {"CMP",1,{IMD(0xc9),ZP(0xc5),ZPX(0xd5),ABS(0xcD),ABX(0xdd),ABY(0xd9),
         INX(0xc1),INY(0xd1),-1}},
 {"CPX",1,{IMD(0xe0),ZP(0xe4),ABS(0xec),-1}},
 {"CPY",1,{IMD(0xc0),ZP(0xc4),ABS(0xcc),-1}},
 {"EOR",1,{IMD(0x49),ZP(0x45),ZPX(0x55),ABS(0x4D),ABX(0x5d),ABY(0x59),
         INX(0x41),INY(0x51),-1}},
 {"LDA",1,{IMD(0xa9),ZP(0xa5),ZPX(0xb5),ABS(0xaD),ABX(0xbd),ABY(0xb9),
         INX(0xa1),INY(0xb1),-1}},
 {"LDX",1,{IMD(0xa2),ZP(0xa6),ZPY(0xB6),ABS(0xae),ABY(0xbe),-1}},
 {"LDY",1,{IMD(0xa0),ZP(0xa4),ZPX(0xB4),ABS(0xac),ABX(0xbc),-1}},
 {"ORA",1,{IMD(0x09),ZP(0x05),ZPX(0x15),ABS(0x0D),ABX(0x1d),ABY(0x19),
         INX(0x01),INY(0x11),-1}},
 {"SBC",1,{IMD(0xEB),IMD(0xe9),ZP(0xe5),ZPX(0xf5),ABS(0xeD),ABX(0xfd),ABY(0xf9),
         INX(0xe1),INY(0xf1),-1}},
 {"STA",2,{ZP(0x85),ZPX(0x95),ABS(0x8D),ABX(0x9d),ABY(0x99),
         INX(0x81),INY(0x91),-1}},
 {"STX",2,{ZP(0x86),ZPY(0x96),ABS(0x8E),-1}},
 {"STY",2,{ZP(0x84),ZPX(0x94),ABS(0x8C),-1}},
 {"BCC",1,{REL(0x90),-1}},
 {"BCS",1,{REL(0xb0),-1}},
 {"BEQ",1,{REL(0xf0),-1}},
 {"BNE",1,{REL(0xd0),-1}},
 {"BMI",1,{REL(0x30),-1}},
 {"BPL",1,{REL(0x10),-1}},
 {"BVC",1,{REL(0x50),-1}},
 {"BVS",1,{REL(0x70),-1}},
};

void Dis6502::Disassemble(uint16 &a, uint16 SpecialA, char *stringo)
{
 uint8 buf;
 unsigned int arg;
 int32 info;
 int x;
 int y;

 info=-1;

 buf=Read(a);
 a++;

  for(x=0;x<NUMOPS;x++)
  {
   y=0;
   while(optable[x].modes[y]>=0)
   {
    if((optable[x].modes[y]&0xFF)==buf)
    {
     info=optable[x].modes[y];
     goto endy;
    }
    y++;
   }
  }

  endy:
  sprintf(stringo,"%02X ",buf);
  if(info>=0)
  {
   int z=flengths[(info>>16)];
   bool borked = 0;

   if(z)
   {
    arg=Read(a);

    if(a == SpecialA) 
     borked = 1;

    if(z == 2 && ((a + 1) & 0xFFFF) == SpecialA)
    {
     if(!borked)
     {
      sprintf(stringo+strlen(stringo),"%02X ", Read(a));
      a++;
     }
     borked = 1;
    }
    if(!borked)
    {
     sprintf(stringo+strlen(stringo),"%02X ",arg);

     a++;
     if(z == 2)
     {
      arg|=Read(a)<<8;
      sprintf(stringo+strlen(stringo),"%02X ",arg>>8);
      a++;
     }
     else
      strcat(stringo,"   ");
     if((info>>16)==1)   /* Relative branch */
      arg=a+(char)arg;
    }

    if(borked)
     strcat(stringo, "--------");
    else
    {
     sprintf(stringo+strlen(stringo),"%s ",optable[x].name);
     sprintf(stringo+strlen(stringo),fstrings[info>>16],arg);
    }
/*      
        0  "#$%02X",       // immediate
        1  "$%04X",        // RELATIVE(jump)
        2  "$%02X",        // Z
        3  "$%02X,X",      // Z,x
        4  "$%02X,Y",      // Z,y
        5  "$%04X",        //ABS
        6 "$%04X,X",      // ABS,x
        7  "$%04X,Y",      // ABS,y
        8  "($%04X)",      // IND
        9  "($%02X,X)",    // INX
        10 "($%02X),Y",    // INY
        11 #define IMP(x)  ((11<<16)|x)
*/
    if(!borked)
    {
     unsigned int tmp;
     switch(info>>16)
     {
      case 2:tmp=arg;
             if(optable[x].type&1)
             {
              sprintf(stringo+strlen(stringo),"    @ $%04X",tmp);
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             }
             break;
      case 3:tmp=(arg+GetX())&0xff;
             sprintf(stringo+strlen(stringo),"    @ $%04X",tmp);
             if(optable[x].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 4:tmp=(arg+GetY())&0xff;
             sprintf(stringo+strlen(stringo),"    @ $%04X",tmp);
             if(optable[x].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 5:tmp=arg;
             if(optable[x].type&1) 
             {
              sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             }
             break;
      case 6:tmp=(arg+GetX())&0xffff;
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[x].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 7:tmp=(arg+GetY())&0xffff;
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[x].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 8:
	     if(IsCMOS)
	      tmp=Read(arg)|(Read((arg+1)&0xffff)<<8);
	     else
	      tmp=Read(arg)|(Read( ((arg+1)&0x00ff) | (arg & 0xFF00))<<8);
             sprintf(stringo+strlen(stringo)," $%04X",tmp);
             break;
      case 9:tmp=(arg+GetX())&0xFF;
             tmp=Read(tmp) | (Read((tmp+1)&0xFF)<<8);
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[x].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 10:tmp=Read(arg) | (Read((arg+1)&0xFF)<<8);
             tmp=(tmp+GetY())&0xFFFF;
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[x].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;

     }
    }


   }
   else
   {
    strcat(stringo,"      ");
    strcat(stringo,optable[x].name);
   }
  }
  else
   sprintf(stringo+strlen(stringo),"      .db $%02X",buf);
}
