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

#include "mednafen.h"
#include "dis6280.h"

Dis6280::Dis6280(void)
{

}

Dis6280::~Dis6280()
{

}

uint8 Dis6280::Read(uint16 A)
{
 return(0);
}

uint8 Dis6280::GetX(void)
{
 return(0);
}

uint8 Dis6280::GetY(void)
{
 return(0);
}


static const char *fstrings[20]=
{ 
        "#$%02X",       // immediate
        "$%04X",        // RELATIVE(jump)
        "$%02X",        // Z
        "$%02X,X",      // Z,x
        "$%02X,Y",      // Z,y
        "$%04X",        //ABS
        "$%04X,X",      // ABS,x
        "$%04X,Y",      // ABS,y
        "($%02X)",      // IND
        "($%02X,X)",    // INX
        "($%02X),Y",    // INY
	"",		// IMP

	// Here come the really fubar(as far as 6502-goes) modes:
	"$%04X, $%04X, $%04X", // BMT
	"#$%02X, $%02X",	    // IMM ZP
	"#$%02X, $%02X, X",  // IMM ZP, X
	"#$%02X, $%04X",     // IMM ABS
	"#$%02X, $%04X, X",  // IMM ABS, X
        "$%02X, $%04X",     // ZP REL


	"($%04X)",		// JMP indirect
	"($%04X, X)",		// JMP indirect X
};

static const int flengths[20]={1,1,1,1,1,2,2,2,1,1,1,0, 6, 2, 2, 3, 3, 2, 2, 2};

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
#define BMT(x)	((12<<16)|x)
#define IMDZP(x) ((13 << 16)|x)
#define IMDZPX(x) ((14 << 16)|x)
#define IMDABS(x) ((15 << 16)|x)
#define IMDABX(x) ((16 << 16)|x)
#define ZPREL(x) ((17 << 16)|x)
#define JMPIND(x) ((18 << 16)|x)
#define JMPINDX(x) ((19 << 16)|x)

typedef struct {
	const char *name;
        int type;       /* 1 for read, 2 for write, 3 for r then write. */
	int32 modes[11];
} OPS;

#define NUMOPS 116
static OPS optable[NUMOPS]=
{
 {"TST",1,{IMDZP(0x83),IMDZPX(0xA3),IMDABS(0x93),IMDABX(0xb3),-1}},

 {"BRK",0,{IMP(0x00),-1}},
 {"RTI",0,{IMP(0x40),-1}},
 {"RTS",0,{IMP(0x60),-1}},

 {"PHA",2,{IMP(0x48),-1}},
 {"PHP",2,{IMP(0x08),-1}},
 {"PHX",2,{IMP(0xDA),-1}},
 {"PHY",2,{IMP(0x5A),-1}},

 {"PLA",1,{IMP(0x68),-1}},
 {"PLP",1,{IMP(0x28),-1}},
 {"PLX",1,{IMP(0xFA),-1}},
 {"PLY",1,{IMP(0x7A),-1}},

 {"JMP",0,{ABS(0x4C),JMPIND(0x6C),JMPINDX(0x7C),-1}},
 {"JSR",0,{ABS(0x20),-1}},

 {"SAX",0,{IMP(0x22),-1}},
 {"SAY",0,{IMP(0x42),-1}},
 {"SXY",0,{IMP(0x02),-1}},

 {"TAX",0,{IMP(0xAA),-1}},
 {"TXA",0,{IMP(0x8A),-1}},
 {"TAY",0,{IMP(0xA8),-1}},
 {"TYA",0,{IMP(0x98),-1}},
 {"TSX",0,{IMP(0xBA),-1}},
 {"TXS",0,{IMP(0x9A),-1}},

 {"TMA",0,{IMD(0x43),-1}},
 {"TAM",0,{IMD(0x53),-1}},

 {"ST0",0,{IMD(0x03),-1}},
 {"ST1",0,{IMD(0x13),-1}},
 {"ST2",0,{IMD(0x23),-1}},

 {"TII",3,{BMT(0x73),-1}},
 {"TDD",3,{BMT(0xC3),-1}},
 {"TIN",3,{BMT(0xD3),-1}},
 {"TIA",3,{BMT(0xE3),-1}},
 {"TAI",3,{BMT(0xF3),-1}},

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

 {"DEC",3,{IMP(0x3A), ZP(0xc6),ZPX(0xd6),ABS(0xcE),ABX(0xdE),-1}},

 {"INC",3,{IMP(0x1A),ZP(0xe6),ZPX(0xf6),ABS(0xeE),ABX(0xfE),-1}},

 {"LSR",3,{IMP(0x4a),ZP(0x46),ZPX(0x56),ABS(0x4E),ABX(0x5E),-1}},

 {"ROL",3,{IMP(0x2a),ZP(0x26),ZPX(0x36),ABS(0x2E),ABX(0x3E),-1}},

 {"ROR",3,{IMP(0x6a),ZP(0x66),ZPX(0x76),ABS(0x6E),ABX(0x7E),-1}},

 {"ADC",1,{IMD(0x69),ZP(0x65),ZPX(0x75),ABS(0x6D),ABX(0x7d),ABY(0x79),
	 IND(0x72), INX(0x61),INY(0x71),-1}},

 {"AND",1,{IMD(0x29),ZP(0x25),ZPX(0x35),ABS(0x2D),ABX(0x3d),ABY(0x39),
         IND(0x32), INX(0x21),INY(0x31),-1}},

 {"BIT",1,{IMD(0x89),ZP(0x24),ZPX(0x34),ABS(0x2c),ABX(0x3C),-1}},

 {"CMP",1,{IMD(0xc9),ZP(0xc5),ZPX(0xd5),ABS(0xcD),ABX(0xdd),ABY(0xd9),
         IND(0xD2), INX(0xc1),INY(0xd1),-1}},

 {"CPX",1,{IMD(0xe0),ZP(0xe4),ABS(0xec),-1}},

 {"CPY",1,{IMD(0xc0),ZP(0xc4),ABS(0xcc),-1}},

 {"EOR",1,{IMD(0x49),ZP(0x45),ZPX(0x55),ABS(0x4D),ABX(0x5d),ABY(0x59),
         IND(0x52), INX(0x41),INY(0x51),-1}},

 {"LDA",1,{IMD(0xa9),ZP(0xa5),ZPX(0xb5),ABS(0xaD),ABX(0xbd),ABY(0xb9),
         IND(0xb2), INX(0xa1),INY(0xb1),-1}},

 {"LDX",1,{IMD(0xa2),ZP(0xa6),ZPY(0xB6),ABS(0xae),ABY(0xbe),-1}},

 {"LDY",1,{IMD(0xa0),ZP(0xa4),ZPX(0xB4),ABS(0xac),ABX(0xbc),-1}},

 {"ORA",1,{IMD(0x09),ZP(0x05),ZPX(0x15),ABS(0x0D),ABX(0x1d),ABY(0x19),
         IND(0x12),INX(0x01),INY(0x11),-1}},

 {"SBC",1,{IMD(0xe9),ZP(0xe5),ZPX(0xf5),ABS(0xeD),ABX(0xfd),ABY(0xf9),
         IND(0xf2),INX(0xe1),INY(0xf1),-1}},

 {"STA",2,{ZP(0x85),ZPX(0x95),ABS(0x8D),ABX(0x9d),ABY(0x99),
         IND(0x92),INX(0x81),INY(0x91),-1}},

 {"STX",2,{ZP(0x86),ZPY(0x96),ABS(0x8E),-1}},

 {"STY",2,{ZP(0x84),ZPX(0x94),ABS(0x8C),-1}},


 {"STZ",2,{ZP(0x64),ZPX(0x74),ABS(0x9C),ABX(0x9E),-1}},

 {"TRB",3,{ZP(0x14),ABS(0x1C),-1}},

 {"TSB",3,{ZP(0x04),ABS(0x0c),-1}},

 {"BBR0",1,{ZPREL(0x0F), -1}},
 {"BBR1",1,{ZPREL(0x1F), -1}},
 {"BBR2",1,{ZPREL(0x2F), -1}},
 {"BBR3",1,{ZPREL(0x3F), -1}},
 {"BBR4",1,{ZPREL(0x4F), -1}},
 {"BBR5",1,{ZPREL(0x5F), -1}},
 {"BBR6",1,{ZPREL(0x6F), -1}},
 {"BBR7",1,{ZPREL(0x7F), -1}},

 {"BBS0",1,{ZPREL(0x8F), -1}},
 {"BBS1",1,{ZPREL(0x9F), -1}},
 {"BBS2",1,{ZPREL(0xAF), -1}},
 {"BBS3",1,{ZPREL(0xBF), -1}},
 {"BBS4",1,{ZPREL(0xCF), -1}},
 {"BBS5",1,{ZPREL(0xDF), -1}},
 {"BBS6",1,{ZPREL(0xEF), -1}},
 {"BBS7",1,{ZPREL(0xFF), -1}},

 {"CLA",1,{IMP(0x62),-1}},
 {"CLX",1,{IMP(0x82),-1}},
 {"CLY",1,{IMP(0xC2),-1}},

 {"CSL",1,{IMP(0x54),-1}},
 {"CSH",1,{IMP(0xD4),-1}},

 {"RMB0",3,{ZP(0x07), -1}},
 {"RMB1",3,{ZP(0x17), -1}},
 {"RMB2",3,{ZP(0x27), -1}},
 {"RMB3",3,{ZP(0x37), -1}},
 {"RMB4",3,{ZP(0x47), -1}},
 {"RMB5",3,{ZP(0x57), -1}},
 {"RMB6",3,{ZP(0x67), -1}},
 {"RMB7",3,{ZP(0x77), -1}},

 {"SMB0",3,{ZP(0x87), -1}},
 {"SMB1",3,{ZP(0x97), -1}},
 {"SMB2",3,{ZP(0xa7), -1}},
 {"SMB3",3,{ZP(0xb7), -1}},
 {"SMB4",3,{ZP(0xc7), -1}},
 {"SMB5",3,{ZP(0xd7), -1}},
 {"SMB6",3,{ZP(0xe7), -1}},
 {"SMB7",3,{ZP(0xf7), -1}},

 {"BRA",1,{REL(0x80),-1}},
 
 {"BSR",1,{REL(0x44),-1}},

 {"BCC",1,{REL(0x90),-1}},

 {"BCS",1,{REL(0xb0),-1}},

 {"BEQ",1,{REL(0xf0),-1}},

 {"BNE",1,{REL(0xd0),-1}},

 {"BMI",1,{REL(0x30),-1}},

 {"BPL",1,{REL(0x10),-1}},

 {"BVC",1,{REL(0x50),-1}},

 {"BVS",1,{REL(0x70),-1}},

};

void Dis6280::Disassemble(uint16 &a, uint16 SpecialA, char *stringo)
{
 uint8 buf;
 uint64 arg;
 int32 info;
 int op_index;
 int y;

 info=-1;

 buf=Read(a);
 a++;

  for(op_index=0; op_index < NUMOPS; op_index++)
  {
   y=0;
   while(optable[op_index].modes[y]>=0)
   {
    if((optable[op_index].modes[y]&0xFF)==buf)
    {
     info=optable[op_index].modes[y];
     goto endy;
    }
    y++;
   }
  }

  endy:
  stringo[0] = 0;
  if(info>=0)
  {
   int z=flengths[(info>>16)];
   bool borked = 0;
   int pow = 0;
   uint64 arg_orig;

   arg_orig = arg = 0;

   if(z)
   {
    for(pow = 0; pow < z; pow++)
    {
     if(a == SpecialA)
     {
      borked = 1;
      break;
     }
     uint8 rb = Read(a);
     arg |= (uint64)rb << (pow * 8);
     a++;
    }
    arg_orig = arg;
    if((info>>16)==1)   /* Relative branch */
     arg = a + (char)arg;

    if(borked)
     strcat(stringo, "--------");
    else
    {
     sprintf(stringo+strlen(stringo),"%s ",optable[op_index].name);

     if((info>>16) < 12 || (info >> 16) == 18 || (info >> 16) == 19) // ...or JMP indirect
      sprintf(stringo+strlen(stringo), fstrings[info>>16], (unsigned int)arg);

        //"$04X, $04X, $04X", // BMT
        //"#$02X, $%02X",     // IMM ZP
        //"#$02X, $%02X, X",  // IMM ZP, X
        //"#$02X, $%04X",     // IMM ABS
        //"#$02X, $%04X, X",  // IMM ABS, X
        //"$02X, #$%02X",     // ZP REL

	//#define BMT(x)  ((12<<16)|x)
	//#define IMDZP(x) ((13 << 16)|x)
	//#define IMDZPX(x) ((14 << 16)|x)
	//#define IMDABS(x) ((15 << 16)|x)
	//#define IMDABX(x) ((16 << 16)|x)
	//#define ZPREL(x) ((17 << 16)|x)

     unsigned int tmp;
     switch(info>>16)
     {
      case 12:
	      sprintf(stringo + strlen(stringo), fstrings[info >> 16], (unsigned int)(arg & 0xFFFF), (unsigned int)((arg >> 16) & 0xFFFF),
			(unsigned int)((arg >> 32) & 0xFFFF));
	      break;
      case 16:
      case 15:
      case 14:
      case 13:
	      sprintf(stringo + strlen(stringo), fstrings[info >> 16], (unsigned int)(arg & 0xFF), (unsigned int)((arg >> 8) & 0xFFFF));
	      break;
      case 17:
              sprintf(stringo + strlen(stringo), fstrings[info >> 16], (unsigned int)(arg & 0xFF), (a + (char)((arg >> 8) & 0xFF)) & 0xFFFF);
	      sprintf(stringo + strlen(stringo), " @ $%04X = $%02X", (unsigned int)(arg & 0xFF) + 0x2000, Read((arg & 0xFF) + 0x2000));
              break;
      case 2:tmp = arg + 0x2000;
             if(optable[op_index].type&1)
             {
              sprintf(stringo+strlen(stringo),"    @ $%04X",tmp);
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             }
             break;
      case 3:tmp=0x2000 + ((arg+GetX())&0xff);
             sprintf(stringo+strlen(stringo),"    @ $%04X",tmp);
             if(optable[op_index].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 4:tmp=0x2000 + ((arg+GetY())&0xff);
             sprintf(stringo+strlen(stringo),"    @ $%04X",tmp);
             if(optable[op_index].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 5:tmp=arg;
             if(optable[op_index].type&1) 
             {
              sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             }
             break;
      case 6:tmp=(arg+GetX())&0xffff;
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[op_index].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 7:tmp=(arg+GetY())&0xffff;
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[op_index].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 8: // Indirect
	     tmp = (arg&0xFF);
             tmp=Read(0x2000 + tmp) | (Read(0x2000 + ((tmp+1)&0xFF))<<8);
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[op_index].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 18: // JMP indirect
 	     tmp=Read(arg)|(Read((arg+1)&0xffff)<<8);
             sprintf(stringo+strlen(stringo)," $%04X",tmp);
             break;
      case 19: // JMP indirect, X
	     {
	      uint16 tmp_addr = arg + GetX();

              tmp = Read(tmp_addr) | (Read((tmp_addr + 1) & 0xFFFF) << 8);
              sprintf(stringo+strlen(stringo)," $%04X",tmp);
	     }
             break;
      case 9:tmp = ((arg+GetX())&0xFF);
             tmp=Read(0x2000 + tmp) | (Read(0x2000 + ((tmp+1)&0xFF))<<8);
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[op_index].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;
      case 10:
	     tmp = Read(0x2000 + arg) | (Read(0x2000 + ((arg+1)&0xFF))<<8);
             tmp=(tmp+GetY())&0xFFFF;
             sprintf(stringo+strlen(stringo),"  @ $%04X",tmp);
             if(optable[op_index].type&1)
              sprintf(stringo+strlen(stringo)," = $%02X",Read(tmp));
             break;

     }
    }
   }
   else
   {
    strcat(stringo,optable[op_index].name);
   }

   for(int x = strlen(stringo); x < 30; x++)
   {
    stringo[x] = ' ';
    stringo[x + 1] = 0;
   }
   sprintf(stringo+strlen(stringo),";%02X ", buf);

   for(int x = 0; x < pow; x++)
    sprintf(stringo + strlen(stringo), (pow > 4) ? "%02X" : "%02X ", (unsigned int)((arg_orig >> x * 8) & 0xFF));
  }
  else
   sprintf(stringo+strlen(stringo),".db $%02X                       ;%02X", buf, buf);

}
