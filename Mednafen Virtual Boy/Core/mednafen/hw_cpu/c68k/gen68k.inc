/*  Copyright 2003-2004 Stephane Dallongeville

    This file is part of Yabause.

    Yabause is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    Yabause is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Yabause; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdarg.h>
#include <string.h>

#define	EA_DREG         0
#define	EA_AREG         1
#define	EA_AIND         2
#define	EA_AINC         3
#define	EA_ADEC         4
#define	EA_D16A         5
#define	EA_D8AX         6
#define	EA_A16          7
#define	EA_A32          8
#define	EA_D16P         9
#define	EA_D8PX		    10
#define	EA_IMM		    11
#define	EA_AINC7        12
#define	EA_ADEC7	    13
#define	EA_ILLEGAL      15

#define	SIZE_BYTE       0
#define	SIZE_WORD       1
#define	SIZE_LONG       2

#define COND_TR         0
#define COND_FA         1
#define COND_HI         2
#define COND_LS         3
#define COND_CC         4
#define COND_CS         5
#define COND_NE         6
#define COND_EQ         7
#define COND_VC         8
#define COND_VS         9
#define COND_PL         10
#define COND_MI         11
#define COND_GE         12
#define COND_LT         13
#define COND_GT         14
#define COND_LE         15

#define COND_NOT_TR     COND_FA
#define COND_NOT_FA     COND_TR
#define COND_NOT_HI     COND_LS
#define COND_NOT_LS     COND_HI
#define COND_NOT_CC     COND_CS
#define COND_NOT_CS     COND_CC
#define COND_NOT_NE     COND_EQ
#define COND_NOT_EQ     COND_NE
#define COND_NOT_VC     COND_VS
#define COND_NOT_VS     COND_VC
#define COND_NOT_PL     COND_MI
#define COND_NOT_MI     COND_PL
#define COND_NOT_GE     COND_LT
#define COND_NOT_LT     COND_GE
#define COND_NOT_GT     COND_LE
#define COND_NOT_LE     COND_GT

#define	OP_ILLEGAL      0x4AFC

static void GenORI();
static void GenORICCR();
static void GenORISR();
static void GenANDI();
static void GenANDICCR();
static void GenANDISR();
static void GenEORI();
static void GenEORICCR();
static void GenEORISR();
static void GenSUBI();
static void GenADDI();
static void GenCMPI();
static void GenBTSTn();
static void GenBCHGn();
static void GenBCLRn();
static void GenBSETn();
static void GenBTST();
static void GenBCHG();
static void GenBCLR();
static void GenBSET();
static void GenMOVEPWaD();
static void GenMOVEPLaD();
static void GenMOVEPWDa();
static void GenMOVEPLDa();
static void GenMOVEB();
static void GenMOVEL();
static void GenMOVEW();
static void GenMOVEAL();
static void GenMOVEAW();
static void GenNEGX();
static void GenCLR();
static void GenNEG();
static void GenNOT();
static void GenMOVESRa();
static void GenMOVEaSR();
static void GenMOVEaCCR();
static void GenNBCD();
static void GenPEA();
static void GenSWAP();
static void GenMOVEMaR();
static void GenEXT();
static void GenTST();
static void GenTAS();
static void GenILLEGAL();
static void GenMOVEMRa();
static void GenTRAP();
static void GenLINK();
static void GenLINKA7();
static void GenULNK();
static void GenULNKA7();
static void GenMOVEAUSP();
static void GenMOVEUSPA();
static void GenRESET();
static void GenNOP();
static void GenSTOP();
static void GenRTE();
static void GenRTS();
static void GenTRAPV();
static void GenRTR();
static void GenJSR();
static void GenJMP();
static void GenCHK();
static void GenLEA();
static void GenSTCC();
static void GenDBCC();
static void GenADDQ();
static void GenSUBQ();
static void GenBCC();
static void GenBCC16();
static void GenBRA();
static void GenBRA16();
static void GenBSR();
static void GenBSR16();
static void GenMOVEQ();
static void GenORaD();
static void GenORDa();
static void GenSBCD();
static void GenSBCDM();
static void GenSBCD7M();
static void GenSBCDM7();
static void GenSBCD7M7();
static void GenDIVU();
static void GenDIVS();
static void GenSUBaD();
static void GenSUBDa();
static void GenSUBX();
static void GenSUBXM();
static void GenSUBX7M();
static void GenSUBXM7();
static void GenSUBX7M7();
static void GenSUBA();
static void GenCMP();
static void GenCMPM();
static void GenCMP7M();
static void GenCMPM7();
static void GenCMP7M7();
static void GenEORDa();
static void GenCMPA();
static void GenANDaD();
static void GenANDDa();
static void GenABCD();
static void GenABCDM();
static void GenABCD7M();
static void GenABCDM7();
static void GenABCD7M7();
static void GenMULU();
static void GenMULS();
static void GenEXGDD();
static void GenEXGAA();
static void GenEXGAD();
static void GenADDaD();
static void GenADDDa();
static void GenADDX();
static void GenADDXM();
static void GenADDX7M();
static void GenADDXM7();
static void GenADDX7M7();
static void GenADDA();
static void GenASRk();
static void GenLSRk();
static void GenROXRk();
static void GenRORk();
static void GenASLk();
static void GenLSLk();
static void GenROXLk();
static void GenROLk();
static void GenASRD();
static void GenLSRD();
static void GenROXRD();
static void GenRORD();
static void GenASLD();
static void GenLSLD();
static void GenROXLD();
static void GenROLD();
static void GenASR();
static void GenLSR();
static void GenROXR();
static void GenROR();
static void GenASL();
static void GenLSL();
static void GenROXL();
static void GenROL();

static void Gen1010();
static void Gen1111();

#define OP_INFO_TABLE_LEN   (142 + 2)

static c68k_op_info_struc op_info_table[OP_INFO_TABLE_LEN] =
{   //                                                       DAAAAddaaddi    DAAAAddaaddi
    //                                                         iid181318m      iid181318m
    //                              siz siz eam ear eam ear    nne6A626Pm      nne6A626Pm

    //  opname      opbase  opmask  typ sft  1   1   2   2     dccAX  PX       dccAX  PX    GenFunc
    {   "1010",     0xA000, 0xF000, 0,  0,  -1, -1, -1, -1,  "------------", "------------", Gen1010  },
    {   "1111",     0xF000, 0xF000, 0,  0,  -1, -1, -1, -1,  "------------", "------------", Gen1111  },

    {   "ORI",      0x0000, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenORI      },
    {   "ORICCR",   0x003C, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenORICCR   },
    {   "ORISR",    0x007C, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenORISR    },
    {   "ANDI",     0x0200, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenANDI     },
    {   "ANDICCR",  0x023C, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenANDICCR  },
    {   "ANDISR",   0x027C, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenANDISR   },
    {   "EORI",     0x0A00, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenEORI     },
    {   "EORICCR",  0x0A3C, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenEORICCR  },
    {   "EORISR",   0x0A7C, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenEORISR   },

    {   "SUBI",     0x0400, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenSUBI     },
    {   "ADDI",     0x0600, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenADDI     },
    {   "CMPI",     0x0C00, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenCMPI     },

    {   "BTSTn",    0x0800, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-ooooooooo-", "------------", GenBTSTn    },
    {   "BCHGn",    0x0840, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenBCHGn    },
    {   "BCLRn",    0x0880, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenBCLRn    },
    {   "BSETn",    0x08C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenBSETn    },

    {   "BTST",     0x0100, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-oooooooooo", "------------", GenBTST     },
    {   "BCHG",     0x0140, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-ooooooo---", "------------", GenBCHG     },
    {   "BCLR",     0x0180, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-ooooooo---", "------------", GenBCLR     },
    {   "BSET",     0x01C0, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-ooooooo---", "------------", GenBSET     },

    {   "MOVEPWaD", 0x0108, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenMOVEPWaD },
    {   "MOVEPLaD", 0x0148, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenMOVEPLaD },
    {   "MOVEPWDa", 0x0188, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenMOVEPWDa },
    {   "MOVEPLDa", 0x01C8, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenMOVEPLDa },

    {   "MOVEB",    0x1000, 0xF000, 0,  0,  3,  0,  6,  9,  "oooooooooooo", "o-ooooooo---", GenMOVEB    },
    {   "MOVEL",    0x2000, 0xF000, 0,  0,  3,  0,  6,  9,  "oooooooooooo", "o-ooooooo---", GenMOVEL    },
    {   "MOVEW",    0x3000, 0xF000, 0,  0,  3,  0,  6,  9,  "oooooooooooo", "o-ooooooo---", GenMOVEW    },
    {   "MOVEAL",   0x2040, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "oooooooooooo", "------------", GenMOVEAL   },
    {   "MOVEAW",   0x3040, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "oooooooooooo", "------------", GenMOVEAW   },

    {   "NEGX",     0x4000, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenNEGX     },
    {   "CLR",      0x4200, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenCLR      },
    {   "NEG",      0x4400, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenNEG      },
    {   "NOT",      0x4600, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenNOT      },

    {   "MOVESRa",  0x40C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenMOVESRa  },
    {   "MOVEaCCR", 0x44C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-oooooooooo", "------------", GenMOVEaCCR },
    {   "MOVEaSR",  0x46C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-oooooooooo", "------------", GenMOVEaSR  },

    {   "NBCD",     0x4800, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenNBCD     },
    {   "PEA",      0x4840, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--o--oooooo-", "------------", GenPEA      },
    {   "SWAP",     0x4840, 0xFFF8, 0,  0,  -1, 0,  -1, -1, "------------", "------------", GenSWAP     },

    {   "MOVEMRa",  0x4880, 0xFF80, 1,  6,  3,  0,  -1, -1, "--o-ooooo---", "------------", GenMOVEMRa  },
    {   "EXT",      0x4880, 0xFFB8, 1,  6,  -1, 0,  -1, -1, "------------", "------------", GenEXT      },
    {   "TST",      0x4A00, 0xFF00, 2,  6,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenTST      },
    {   "TAS",      0x4AC0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenTAS      },
    {   "ILLEGAL",  0x4AFC, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenILLEGAL  },
    {   "MOVEMaR",  0x4C80, 0xFF80, 1,  6,  3,  0,  -1, -1, "--oo-oooooo-", "------------", GenMOVEMaR  },

    {   "TRAP",     0x4E40, 0xFFF0, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenTRAP     },
    {   "LINK",     0x4E50, 0xFFF8, 0,  0,  -1, 0,  -1, -1, "------------", "------------", GenLINK     },
    {   "LINKA7",   0x4E57, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenLINKA7   },
    {   "ULNK",     0x4E58, 0xFFF8, 0,  0,  -1, 0,  -1, -1, "------------", "------------", GenULNK     },
    {   "ULNKA7",   0x4E5F, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenULNKA7   },
    {   "MOVEAUSP", 0x4E60, 0xFFF8, 0,  0,  -1, 0,  -1, -1, "------------", "------------", GenMOVEAUSP },
    {   "MOVEUSPA", 0x4E68, 0xFFF8, 0,  0,  -1, 0,  -1, -1, "------------", "------------", GenMOVEUSPA },

    {   "RESET",    0x4E70, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenRESET    },
    {   "NOP",      0x4E71, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenNOP      },
    {   "STOP",     0x4E72, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenSTOP     },
    {   "RTE",      0x4E73, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenRTE      },
    {   "RTS",      0x4E75, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenRTS      },
    {   "TRAPV",    0x4E76, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenTRAPV    },
    {   "RTR",      0x4E77, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenRTR      },

    {   "JSR",      0x4E80, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--o--oooooo-", "------------", GenJSR      },
    {   "JMP",      0x4EC0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--o--oooooo-", "------------", GenJMP      },

    {   "CHK",      0x4180, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-oooooooooo", "------------", GenCHK      },
    {   "LEA",      0x41C0, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "--o--oooooo-", "------------", GenLEA      },

    {   "STCC",     0x50C0, 0xF0C0, 0,  0,  3,  0,  -1, -1, "o-ooooooo---", "------------", GenSTCC     },
    {   "DBCC",     0x50C8, 0xF0F8, 0,  0,  -1, 0,  -1, -1, "------------", "------------", GenDBCC     },

    {   "ADDQ",     0x5000, 0xF100, 2,  6,  3,  0,  -1, -1, "ooooooooo---", "------------", GenADDQ     },
    {   "SUBQ",     0x5100, 0xF100, 2,  6,  3,  0,  -1, -1, "ooooooooo---", "------------", GenSUBQ     },

    {   "BCC",      0x6000, 0xF000, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenBCC      },
    {   "BCC16",    0x6000, 0xF0FF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenBCC16    },
    {   "BRA",      0x6000, 0xFF00, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenBRA      },
    {   "BRA16",    0x6000, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenBRA16    },
    {   "BSR",      0x6100, 0xFF00, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenBSR      },
    {   "BSR16",    0x6100, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenBSR16    },

    {   "MOVEQ",    0x7000, 0xF100, 0,  0,  -1, 9,  -1, -1, "------------", "------------", GenMOVEQ    },

    {   "ORaD",     0x8000, 0xF100, 2,  6,  3,  0,  -1, 9,  "o-oooooooooo", "------------", GenORaD     },
    {   "ORDa",     0x8100, 0xF100, 2,  6,  3,  0,  -1, 9,  "--ooooooo---", "------------", GenORDa     },
    {   "SBCD",     0x8100, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenSBCD     },
    {   "SBCDM",    0x8108, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenSBCDM    },
    {   "SBCD7M",   0x810F, 0xF1FF, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenSBCD7M   },
    {   "SBCDM7",   0x8F08, 0xFFF8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenSBCDM7   },
    {   "SBCD7M7",  0x8F0F, 0xFFFF, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenSBCD7M7  },
    {   "DIVU",     0x80C0, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-oooooooooo", "------------", GenDIVU     },
    {   "DIVS",     0x81C0, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-oooooooooo", "------------", GenDIVS     },

    {   "SUBaD",    0x9000, 0xF100, 2,  6,  3,  0,  -1, 9,  "oooooooooooo", "------------", GenSUBaD    },
    {   "SUBDa",    0x9100, 0xF100, 2,  6,  3,  0,  -1, 9,  "--ooooooo---", "------------", GenSUBDa    },
    {   "SUBX",     0x9100, 0xF138, 2,  6,  -1, 0,  -1, 9,  "------------", "------------", GenSUBX     },
    {   "SUBXM",    0x9108, 0xF138, 2,  6,  -1, 0,  -1, 9,  "------------", "------------", GenSUBXM    },
    {   "SUBX7M",   0x910F, 0xF13F, 2,  6,  -1, -1, -1, 9,  "------------", "------------", GenSUBX7M   },
    {   "SUBXM7",   0x9F08, 0xFF38, 2,  6,  -1, 0,  -1, -1, "------------", "------------", GenSUBXM7   },
    {   "SUBX7M7",  0x9F0F, 0xFF3F, 2,  6,  -1, -1, -1, -1, "------------", "------------", GenSUBX7M7  },
    {   "SUBA",     0x90C0, 0xF0C0, 1,  8,  3,  0,  -1, 9,  "oooooooooooo", "------------", GenSUBA     },

    {   "CMP",      0xB000, 0xF100, 2,  6,  3,  0,  -1, 9,  "oooooooooooo", "------------", GenCMP      },
    {   "CMPM",     0xB108, 0xF138, 2,  6,  -1, 0,  -1, 9,  "------------", "------------", GenCMPM     },
    {   "CMP7M",    0xB10F, 0xF13F, 2,  6,  -1, -1, -1, 9,  "------------", "------------", GenCMP7M    },
    {   "CMPM7",    0xBF08, 0xFF38, 2,  6,  -1, 0,  -1, -1, "------------", "------------", GenCMPM7    },
    {   "CMP7M7",   0xBF0F, 0xFF3F, 2,  6,  -1, -1, -1, -1, "------------", "------------", GenCMP7M7   },
    {   "EORDa",    0xB100, 0xF100, 2,  6,  3,  0,  -1, 9,  "o-ooooooo---", "------------", GenEORDa    },
    {   "CMPA",     0xB0C0, 0xF0C0, 1,  8,  3,  0,  -1, 9,  "oooooooooooo", "------------", GenCMPA     },

    {   "ANDaD",    0xC000, 0xF100, 2,  6,  3,  0,  -1, 9,  "o-oooooooooo", "------------", GenANDaD    },
    {   "ANDDa",    0xC100, 0xF100, 2,  6,  3,  0,  -1, 9,  "--ooooooo---", "------------", GenANDDa    },
    {   "ABCD",     0xC100, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenABCD     },
    {   "ABCDM",    0xC108, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenABCDM    },
    {   "ABCD7M",   0xC10F, 0xF1FF, 0,  0,  -1, -1, -1, 9,  "------------", "------------", GenABCD7M   },
    {   "ABCDM7",   0xCF08, 0xFFF8, 0,  0,  -1, 0,  -1, -1, "------------", "------------", GenABCDM7   },
    {   "ABCD7M7",  0xCF0F, 0xFFFF, 0,  0,  -1, -1, -1, -1, "------------", "------------", GenABCD7M7  },
    {   "MULU",     0xC0C0, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-oooooooooo", "------------", GenMULU     },
    {   "MULS",     0xC1C0, 0xF1C0, 0,  0,  3,  0,  -1, 9,  "o-oooooooooo", "------------", GenMULS     },
    {   "EXGDD",    0xC140, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenEXGDD    },
    {   "EXGAA",    0xC148, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenEXGAA    },
    {   "EXGAD",    0xC188, 0xF1F8, 0,  0,  -1, 0,  -1, 9,  "------------", "------------", GenEXGAD    },

    {   "ADDaD",    0xD000, 0xF100, 2,  6,  3,  0,  -1, 9,  "oooooooooooo", "------------", GenADDaD    },
    {   "ADDDa",    0xD100, 0xF100, 2,  6,  3,  0,  -1, 9,  "--ooooooo---", "------------", GenADDDa    },
    {   "ADDX",     0xD100, 0xF138, 2,  6,  -1, 0,  -1, 9,  "------------", "------------", GenADDX     },
    {   "ADDXM",    0xD108, 0xF138, 2,  6,  -1, 0,  -1, 9,  "------------", "------------", GenADDXM    },
    {   "ADDX7M",   0xD10F, 0xF13F, 2,  6,  -1, -1, -1, 9,  "------------", "------------", GenADDX7M   },
    {   "ADDXM7",   0xDF08, 0xFF38, 2,  6,  -1, 0,  -1, -1, "------------", "------------", GenADDXM7   },
    {   "ADDX7M7",  0xDF0F, 0xFF3F, 2,  6,  -1, -1, -1, -1, "------------", "------------", GenADDX7M7  },
    {   "ADDA",     0xD0C0, 0xF0C0, 1,  8,  3,  0,  -1, 9,  "oooooooooooo", "------------", GenADDA     },

    {   "ASRk",     0xE000, 0xF138, 2,  6,  -1, 0, -1, -1,  "o-----------", "------------", GenASRk     },
    {   "LSRk",     0xE008, 0xF138, 2,  6,  -1, 0, -1, -1,  "o-----------", "------------", GenLSRk     },
    {   "ROXRk",    0xE010, 0xF138, 2,  6,  -1, 0, -1, -1,  "o-----------", "------------", GenROXRk    },
    {   "RORk",     0xE018, 0xF138, 2,  6,  -1, 0, -1, -1,  "o-----------", "------------", GenRORk     },
    {   "ASLk",     0xE100, 0xF138, 2,  6,  -1, 0, -1, -1,  "o-----------", "------------", GenASLk     },
    {   "LSLk",     0xE108, 0xF138, 2,  6,  -1, 0, -1, -1,  "o-----------", "------------", GenLSLk     },
    {   "ROXLk",    0xE110, 0xF138, 2,  6,  -1, 0, -1, -1,  "o-----------", "------------", GenROXLk    },
    {   "ROLk",     0xE118, 0xF138, 2,  6,  -1, 0, -1, -1,  "o-----------", "------------", GenROLk     },

    {   "ASRD",     0xE020, 0xF138, 2,  6,  -1, 0,  -1, 9,  "o-----------", "o-----------", GenASRD     },
    {   "LSRD",     0xE028, 0xF138, 2,  6,  -1, 0,  -1, 9,  "o-----------", "o-----------", GenLSRD     },
    {   "ROXRD",    0xE030, 0xF138, 2,  6,  -1, 0,  -1, 9,  "o-----------", "o-----------", GenROXRD    },
    {   "RORD",     0xE038, 0xF138, 2,  6,  -1, 0,  -1, 9,  "o-----------", "o-----------", GenRORD     },
    {   "ASLD",     0xE120, 0xF138, 2,  6,  -1, 0,  -1, 9,  "o-----------", "o-----------", GenASLD     },
    {   "LSLD",     0xE128, 0xF138, 2,  6,  -1, 0,  -1, 9,  "o-----------", "o-----------", GenLSLD     },
    {   "ROXLD",    0xE130, 0xF138, 2,  6,  -1, 0,  -1, 9,  "o-----------", "o-----------", GenROXLD    },
    {   "ROLD",     0xE138, 0xF138, 2,  6,  -1, 0,  -1, 9,  "o-----------", "o-----------", GenROLD     },

    {   "ASR",      0xE0C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--ooooooo---", "------------", GenASR      },
    {   "LSR",      0xE2C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--ooooooo---", "------------", GenLSR      },
    {   "ROXR",     0xE4C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--ooooooo---", "------------", GenROXR     },
    {   "ROR",      0xE6C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--ooooooo---", "------------", GenROR      },
    {   "ASL",      0xE1C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--ooooooo---", "------------", GenASL      },
    {   "LSL",      0xE3C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--ooooooo---", "------------", GenLSL      },
    {   "ROXL",     0xE5C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--ooooooo---", "------------", GenROXL     },
    {   "ROL",      0xE7C0, 0xFFC0, 0,  0,  3,  0,  -1, -1, "--ooooooo---", "------------", GenROL      }
};

static u16 op_jump_table[0x10000];

// files where code is generated
static FILE* ini_file = NULL;
static FILE* opcode_file = NULL;

// current generated instruction infos
static c68k_op_info_struc *current_op;
static u32 current_ea;
static u32 current_eam;
static u32 current_reg;
static u32 current_ea2;
static u32 current_eam2;
static u32 current_reg2;
static u32 current_size;
static u32 current_cycle;

static char current_cond_char[128];

static char szc[20];
static char szcs[20];
static char szcf[20];
static char szcsf[20];

static u32  current_bits_mask;
static u8   current_sft_mask;

#define	EA_DREG         0
#define	EA_AREG         1
#define	EA_AIND         2
#define	EA_AINC         3
#define	EA_ADEC         4
#define	EA_D16A         5
#define	EA_D8AX         6
#define	EA_A16          7
#define	EA_A32          8
#define	EA_D16P         9
#define	EA_D8PX		    10
#define	EA_IMM		    11
#define	EA_AINC7        12
#define	EA_ADEC7	    13
#define	EA_ILLEGAL      15

static const u32 jmp_jsr_cycle_table[16] =
{
    0, 0,
    4,
    0,
    0,
    6,
    10,
    6,
    8,
    6,
    10,
    0, 0, 0, 0
};

static const u32 lea_pea_cycle_table[16] =
{
    0, 0,
    0,
    0,
    0,
    4,
    8,
    4,
    8,
    4,
    8,
    0, 0, 0, 0
};

static const u32 movem_cycle_table[16] =
{
    0, 0,
    0,
    0,
    0,
    4,
    6,
    4,
    8,
    4,
    6,
    0, 0, 0, 0
};

// general emitter function
////////////////////////////

static u32 prepare_generate()
{
    char filename[32];

    sprintf(filename, "c68k_op%.1X.inc", (current_op->op_base >> 12) & 0xF);
    if (opcode_file != NULL)
    {
        fclose(opcode_file);
        opcode_file = NULL;
    }
	opcode_file = fopen(filename, "at");
	if (opcode_file == NULL)
	{
	    printf("Can't open %s\n", filename);
	    return 1;
	}
	return 0;
}

static void wf_op(char* fmt, ...)
{
	va_list args;

    if (opcode_file == NULL) return;

	va_start(args, fmt);
	vfprintf(opcode_file, fmt, args);
	va_end(args);
}

static void gen_jumptable(u32 base, u32 start1, u32 end1, u32 step1, u32 start2, u32 end2, u32 step2, u32 start3, u32 end3, u32 step3, u32 op)
{
#ifdef C68K_CONST_JUMP_TABLE
    u32 i, j, k;
#endif

    base &= 0xFFFF;
    start1 &= 0xFFFF;
    end1 &= 0xFFFF;
    step1 &= 0xFFFF;
    if (end1 < start1) end1 = start1;
    start2 &= 0xFFFF;
    end2 &= 0xFFFF;
    step2 &= 0xFFFF;
    if (end2 < start2) end2 = start2;
    op &= 0xFFFF;

#ifndef C68K_NO_JUMP_TABLE
#ifdef C68K_CONST_JUMP_TABLE
    if (step1 == 0) step1 = 1;
    if (step2 == 0) step2 = 1;
    if (step3 == 0) step3 = 1;

    for(i = start1; i <= end1; i += step1)
        for(j = start2; j <= end2; j += step2)
            for(k = start3; k <= end3; k += step3)
                op_jump_table[base + i + j + k] = op;
                
#else
    if (ini_file == NULL) return;

    if (start1 != end1)
    {
        fprintf(ini_file, "\t\tfor(i = 0x%.4X; i <= 0x%.4X; i += 0x%.4X)\n", (int)start1, (int)end1, (int)step1);
    	if (start2 != end2)
        {
            fprintf(ini_file, "\t\t\tfor(j = 0x%.4X; j <= 0x%.4X; j += 0x%.4X)\n\t", (int)start2, (int)end2, (int)step2);
        	if (start3 != end3)
            {
                fprintf(ini_file, "\t\t\t\tfor(k = 0x%.4X; k <= 0x%.4X; k += 0x%.4X)\n\t", (int)start3, (int)end3, (int)step3);
                fprintf(ini_file, "\t\t\t\t\tJumpTable[0x%.4X + i + j + k] = &&OP_0x%.4X;\n", (int)base, (int)op);
            }
            else fprintf(ini_file, "\t\t\t\tJumpTable[0x%.4X + i + j] = &&OP_0x%.4X;\n", (int)base, (int)op);
        }
        else fprintf(ini_file, "\t\t\tJumpTable[0x%.4X + i] = &&OP_0x%.4X;\n", (int)base, (int)op);
    }
    else fprintf(ini_file, "\t\tJumpTable[0x%.4X] = &&OP_0x%.4X;\n", (int)base, (int)op);
#endif
#else
    u32 i, j, k;

    if (step1 == 0) step1 = 1;
    if (step2 == 0) step2 = 1;
    if (step3 == 0) step3 = 1;

    for(i = start1; i <= end1; i += step1)
        for(j = start2; j <= end2; j += step2)
            for(k = start3; k <= end3; k += step3)
            {
                u32 temp=(base + i + j + k);

		if(temp == 0x4AFC)
		 wf_op("default:\n");

                if (temp != op && temp != 0x4E57 && temp != 0x4E5F)
                   wf_op("case 0x%.4X:\n", base + i + j + k);
            }    
#endif
}

static void gen_opjumptable_ext(u32 base, u32 start3, u32 end3, u32 step3, u32 op)
{
    u32 start1, end1, step1, start2, end2, step2;

    start1 = end1 = step1 = 0;
    start2 = end2 = step2 = 0;

    if ((current_op->reg_sft != -1) && (current_ea < 7))
    {
        if ((current_ea == EA_AINC) || (current_ea == EA_ADEC)) end1 = 6 << current_op->reg_sft;
        else end1 = 7 << current_op->reg_sft;
        step1 = 1 << current_op->reg_sft;
    }
    if ((current_op->reg2_sft != -1) && (current_ea2 < 7))
    {
        if ((current_ea2 == EA_AINC) || (current_ea2 == EA_ADEC)) end2 = 6 << current_op->reg2_sft;
        else end2 = 7 << current_op->reg2_sft;
        step2 = 1 << current_op->reg2_sft;
    }

    if (start1 != end1)
    {
        if (start2 != end2) gen_jumptable(base, start1, end1, step1, start2, end2, step2, start3, end3, step3, op);
        else gen_jumptable(base, start1, end1, step1, start3, end3, step3, start2, end2, step2, op);
    }
    else if (start2 != end2) gen_jumptable(base, start2, end2, step2, start3, end3, step3, start1, end1, step1, op);
    else gen_jumptable(base, start3, end3, step3, start2, end2, step2, start1, end1, step1, op);
}

static void gen_opjumptable(u32 op)
{
    gen_opjumptable_ext(op, 0, 0, 0, op);
}

#define GEN_ADR 1
#define GEN_RES 2
#define GEN_SRC 4
#define GEN_DST 8
#define GEN_ALL 15

static void start_op(u32 op, int v)
{
    current_cycle = 0;

    wf_op("\n// %s\n", current_op->op_name);
#ifndef C68K_NO_JUMP_TABLE
    wf_op("OP_0x%.4X:\n", op & 0xFFFF);
#else
    wf_op("case 0x%.4X:\n", op & 0xFFFF);
#endif
    wf_op("{\n");
    if (v & GEN_ADR) wf_op("\tu32 adr;\n");
    if (v & GEN_RES) wf_op("\tu32 res;\n");
    if (v & GEN_DST) wf_op("\tu32 dst;\n");
    if (v & GEN_SRC) wf_op("\tu32 src;\n");
}

static void add_CCnt(u32 cycle)
{
    wf_op("\ttimestamp += %d;\n", cycle);
}

static void adds_CCnt(char *str)
{
    wf_op("\ttimestamp += %s;\n", str);
}

static void quick_fterminate_op(u32 cycle)
{
    wf_op("\ttimestamp += %d;\n", current_cycle + cycle);
    wf_op("\tgoto C68k_Exec_End;\n");
}

static void fterminate_op(u32 cycle)
{
    wf_op("}\n");
    wf_op("timestamp += %d;\n", current_cycle + cycle);
    wf_op("goto C68k_Exec_End;\n");
}

static void quick_terminate_op(u32 cycle)
{
    wf_op("\tRET(%d)\n", current_cycle + cycle);
}

static void terminate_op(u32 cycle)
{
    wf_op("}\n");
    wf_op("RET(%d)\n", current_cycle + cycle);
}
#include <assert.h>

static void IO_CTHULHU(char *test_string) //int evil_babies)
{
 int numb = 0;
 int evil_babies = 0;
 char *rs;

 if((rs = strstr(test_string, "_BYTE_")))
 {
  evil_babies = 4;
  numb = 8;
 }
 else if((rs = strstr(test_string, "_WORD_")))
 {
  evil_babies = 4;
  numb = 16;
 }
 else if((rs = strstr(test_string, "_LONG_")))
 {
  evil_babies = 8;
  numb = 32;
 }

 if(!evil_babies)
 {
  //printf("%d %s\n", evil_babies, test_string);
  return;
 }

 if(current_cycle < evil_babies)
 {
  //puts("Too many clowns!");
  return;
 }

 // assert(current_cycle >= 4);
 //wf_op("\tCCnt -= %d;\n", current_cycle - 2);
 //current_cycle = 2;

 rs[3] = 'a';
 rs[4] = 't';
 //sprintf(&rs[1], "NT%02d", numb);

 current_cycle -= evil_babies;
 if(current_cycle)
 {
  wf_op("\ttimestamp += %d;\n", current_cycle);
  current_cycle = 0;
 }
}

static void mem_op(char* fmt, ...)
{
	static char lovelykoala[65536];
	va_list args;

    if (opcode_file == NULL) return;

//	va_start(args, fmt);
//	vfprintf(opcode_file, fmt, args);
//	va_end(args);

    va_start(args, fmt);
    vsnprintf(lovelykoala, sizeof(lovelykoala), fmt, args);
    va_end(args);

    IO_CTHULHU(lovelykoala);
    fwrite(lovelykoala, strlen(lovelykoala), 1, opcode_file);
}

// flag emitter function
/////////////////////////

static void set_logic_flag()
{
    wf_op("\tCPU->flag_C = 0;\n");
    wf_op("\tCPU->flag_V = 0;\n");
    wf_op("\tCPU->flag_notZ = res;\n");
    switch(current_size)
    {
        case SIZE_BYTE:
            wf_op("\tCPU->flag_N = res;\n");
        	break;

        case SIZE_WORD:
            wf_op("\tCPU->flag_N = res >> 8;\n");
        	break;

        case SIZE_LONG:
            wf_op("\tCPU->flag_N = res >> 24;\n");
        	break;
	}
}

static void set_logicl_flag()
{
    wf_op("\tCPU->flag_C = 0;\n");
    wf_op("\tCPU->flag_V = 0;\n");
    switch(current_size)
    {
        case SIZE_BYTE:
            wf_op("\tCPU->flag_N = res;\n");
            wf_op("\tCPU->flag_notZ = res & 0xFF;\n");
        	break;

        case SIZE_WORD:
            wf_op("\tCPU->flag_notZ = res & 0xFFFF;\n");
            wf_op("\tCPU->flag_N = res >> 8;\n");
        	break;

        case SIZE_LONG:
            wf_op("\tCPU->flag_notZ = res;\n");
            wf_op("\tCPU->flag_N = res >> 24;\n");
        	break;
	}
}

static void set_add_flag()
{
    switch(current_size)
    {
        case SIZE_BYTE:
        	wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res;\n");
        	wf_op("\tCPU->flag_V = (src ^ res) & (dst ^ res);\n");
            	wf_op("\tCPU->flag_notZ = res & 0xFF;\n");
        	break;

        case SIZE_WORD:
        	wf_op("\tCPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;\n");
        	wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;\n");
            	wf_op("\tCPU->flag_notZ = res & 0xFFFF;\n");
        	break;

        case SIZE_LONG:
            	wf_op("\tCPU->flag_notZ = res;\n");
		wf_op("\tCPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;\n");
        	wf_op("\tCPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;\n");
            	wf_op("\tCPU->flag_N = res >> 24;\n");
        	break;
	}
}

static void set_addx_flag()
{
    switch(current_size)
    {
        case SIZE_BYTE:
        	wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res;\n");
        	wf_op("\tCPU->flag_V = (src ^ res) & (dst ^ res);\n");
                wf_op("\tCPU->flag_notZ |= res & 0xFF;\n");
        	break;

        case SIZE_WORD:
        	wf_op("\tCPU->flag_V = ((src ^ res) & (dst ^ res)) >> 8;\n");
        	wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;\n");
                wf_op("\tCPU->flag_notZ |= res & 0xFFFF;\n");
        	break;

        case SIZE_LONG:
                wf_op("\tCPU->flag_notZ |= res;\n");
                wf_op("\tCPU->flag_X = CPU->flag_C = ((src & dst) | (~res & (src | dst))) >> 23;\n");
        	wf_op("\tCPU->flag_V = ((src ^ res) & (dst ^ res)) >> 24;\n");
                wf_op("\tCPU->flag_N = res >> 24;\n");
        	break;
	}
}

static void set_sub_flag()
{
    switch(current_size)
    {
        case SIZE_BYTE:
        	wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res;\n");
        	wf_op("\tCPU->flag_V = (src ^ dst) & (res ^ dst);\n");
            	wf_op("\tCPU->flag_notZ = res & 0xFF;\n");
        	break;

        case SIZE_WORD:
        	wf_op("\tCPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;\n");
        	wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;\n");
        	wf_op("\tCPU->flag_notZ = res & 0xFFFF;\n");
	       	break;

        case SIZE_LONG:
            	wf_op("\tCPU->flag_notZ = res;\n");
		wf_op("\tCPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;\n");
        	wf_op("\tCPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;\n");
            	wf_op("\tCPU->flag_N = res >> 24;\n");
        	break;
	}
}

static void set_subx_flag()
{
    switch(current_size)
    {
        case SIZE_BYTE:
        	wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res;\n");
        	wf_op("\tCPU->flag_V = (src ^ dst) & (res ^ dst);\n");
            	wf_op("\tCPU->flag_notZ |= res & 0xFF;\n");
        	break;

        case SIZE_WORD:
        	wf_op("\tCPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;\n");
        	wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;\n");
            	wf_op("\tCPU->flag_notZ |= res & 0xFFFF;\n");
        	break;

        case SIZE_LONG:
            	wf_op("\tCPU->flag_notZ |= res;\n");
                wf_op("\tCPU->flag_X = CPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;\n");
        	wf_op("\tCPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;\n");
            	wf_op("\tCPU->flag_N = res >> 24;\n");
        	break;
	}
}

static void set_cmp_flag()
{
    switch(current_size)
    {
        case SIZE_BYTE:
        	wf_op("\tCPU->flag_N = CPU->flag_C = res;\n");
        	wf_op("\tCPU->flag_V = (src ^ dst) & (res ^ dst);\n");
            	wf_op("\tCPU->flag_notZ = res & 0xFF;\n");
        	break;

        case SIZE_WORD:
        	wf_op("\tCPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 8;\n");
        	wf_op("\tCPU->flag_N = CPU->flag_C = res >> 8;\n");
            	wf_op("\tCPU->flag_notZ = res & 0xFFFF;\n");
        	break;

        case SIZE_LONG:
            	wf_op("\tCPU->flag_notZ = res;\n");
		wf_op("\tCPU->flag_C = ((src & res) | (~dst & (src | res))) >> 23;\n");
        	wf_op("\tCPU->flag_V = ((src ^ dst) & (res ^ dst)) >> 24;\n");
            	wf_op("\tCPU->flag_N = res >> 24;\n");
        	break;
	}
}

static void set_negx_flag()
{
    switch(current_size)
    {
        case SIZE_BYTE:
            wf_op("\tCPU->flag_V = res & src;\n");
            wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res;\n");
            wf_op("\tCPU->flag_notZ |= res & 0xFF;\n");
        	break;

        case SIZE_WORD:
            wf_op("\tCPU->flag_V = (res & src) >> 8;\n");
            wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;\n");
            wf_op("\tCPU->flag_notZ |= res & 0xFFFF;\n");
        	break;

        case SIZE_LONG:
            wf_op("\tCPU->flag_notZ |= res;\n");
            wf_op("\tCPU->flag_V = (res & src) >> 24;\n");
	    wf_op("\tCPU->flag_X = CPU->flag_C = (src | res) >> 23;\n");
            wf_op("\tCPU->flag_N = res >> 24;\n");
        	break;
	}
}

static void set_neg_flag()
{
    switch(current_size)
    {
        case SIZE_BYTE:
            wf_op("\tCPU->flag_V = res & src;\n");
            wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res;\n");
            wf_op("\tCPU->flag_notZ = res & 0xFF;\n");
        	break;

        case SIZE_WORD:
            wf_op("\tCPU->flag_V = (res & src) >> 8;\n");
            wf_op("\tCPU->flag_N = CPU->flag_X = CPU->flag_C = res >> 8;\n");
            wf_op("\tCPU->flag_notZ = res & 0xFFFF;\n");
        	break;

        case SIZE_LONG:
            wf_op("\tCPU->flag_notZ = res;\n");
            wf_op("\tCPU->flag_V = (res & src) >> 24;\n");
	    wf_op("\tCPU->flag_X = CPU->flag_C = (src | res) >> 23;\n");
            wf_op("\tCPU->flag_N = res >> 24;\n");
            break;
	}
}

char* get_cond_as_cond(u32 cond, u32 notvar)
{
    if (notvar) cond ^= 1;

    switch(cond)
    {
        case COND_TR:
            sprintf(current_cond_char, "(1)");
            break;

        case COND_FA:
            sprintf(current_cond_char, "(0)");
            break;

        case COND_HI:
            sprintf(current_cond_char, "(CPU->flag_notZ && (!(CPU->flag_C & 0x100)))");
            break;

        case COND_LS:
            sprintf(current_cond_char, "((!CPU->flag_notZ) || (CPU->flag_C & 0x100))");
            break;

        case COND_CC:
            sprintf(current_cond_char, "(!(CPU->flag_C & 0x100))");
            break;

        case COND_CS:
            sprintf(current_cond_char, "(CPU->flag_C & 0x100)");
            break;

        case COND_NE:
            sprintf(current_cond_char, "(CPU->flag_notZ)");
            break;

        case COND_EQ:
            sprintf(current_cond_char, "(!CPU->flag_notZ)");
            break;

        case COND_VC:
            sprintf(current_cond_char, "(!(CPU->flag_V & 0x80))");
            break;

        case COND_VS:
            sprintf(current_cond_char, "(CPU->flag_V & 0x80)");
            break;

        case COND_PL:
            sprintf(current_cond_char, "(!(CPU->flag_N & 0x80))");
            break;

        case COND_MI:
            sprintf(current_cond_char, "(CPU->flag_N & 0x80)");
            break;

        case COND_GE:
            sprintf(current_cond_char, "(!((CPU->flag_N ^ CPU->flag_V) & 0x80))");
            break;

        case COND_LT:
            sprintf(current_cond_char, "((CPU->flag_N ^ CPU->flag_V) & 0x80)");
            break;

        case COND_GT:
            sprintf(current_cond_char, "(CPU->flag_notZ && (!((CPU->flag_N ^ CPU->flag_V) & 0x80)))");
            break;

        case COND_LE:
            sprintf(current_cond_char, "((!CPU->flag_notZ) || ((CPU->flag_N ^ CPU->flag_V) & 0x80))");
            break;
    }

    return current_cond_char;
}

// effective address related function
//////////////////////////////////////

static u32 has_ea(u32 ea)
{
    if (ea == EA_AINC7) return (current_op->ea_supported[EA_AINC] == 'o');
    if (ea == EA_ADEC7) return (current_op->ea_supported[EA_ADEC] == 'o');
    if (ea <= EA_IMM) return (current_op->ea_supported[ea] == 'o');
    return 0;
}

static u32 has_ea2(u32 ea)
{
    if (ea == EA_AINC7) return (current_op->ea2_supported[EA_AINC] == 'o');
    if (ea == EA_ADEC7) return (current_op->ea2_supported[EA_ADEC] == 'o');
    if (ea <= EA_IMM) return (current_op->ea2_supported[ea] == 'o');
    return 0;
}

static u32 _eamreg_to_ea(u32 eam, u32 reg)
{
    if ((eam > 7) || (reg > 7)) return EA_ILLEGAL;
	if ((eam == 3) && (reg == 7)) return EA_AINC7;
	if ((eam == 4) && (reg == 7)) return EA_ADEC7;
	if (eam != 7) return eam;
	if (reg < 5) return (eam + reg);
	return EA_ILLEGAL;
}

static u32 _ea_to_eamreg(u32 ea)
{
    if (ea < 7) return (ea << 3) | 0;
    if (ea == EA_AINC7) return (EA_AINC << 3) | 7;
    if (ea == EA_ADEC7) return (EA_ADEC << 3) | 7;
    if (ea <= EA_IMM) return (7 << 3) | (ea - 7);
	return (7 << 3) | 7;
}

static u32 is_ea_memory(u32 ea)
{
    if ((ea > EA_AREG) && (ea != EA_IMM)) return 1;
    else return 0;
}

static void _ea_calc_free(u32 ea, u32 rsft)
{
	u32 step;

    step = 0;
    switch (current_size)
    {
        case SIZE_BYTE:
            if ((ea == EA_AINC7) || (ea == EA_ADEC7)) step = 2;
            else step = 1;
            break;

        case SIZE_WORD:
            step = 2;
            break;

        case SIZE_LONG:
            step = 4;
            break;
    }

  	switch (ea)
	{
		case EA_DREG:
//            wf_op("\tadr = (u32)(&CPU->D[(Opcode >> %d) & 7]);\n", rsft);
			break;

		case EA_AREG:
//            wf_op("\tadr = (u32)(&CPU->A[(Opcode >> %d) & 7]);\n", rsft);
			break;

		case EA_AIND:
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7];\n", rsft);
			break;

		case EA_AINC:
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7];\n", rsft);
            wf_op("\tCPU->A[(Opcode >> %d) & 7] += %d;\n", rsft, step);
			break;

		case EA_AINC7:
            wf_op("\tadr = CPU->A[7];\n");
            wf_op("\tCPU->A[7] += %d;\n", step);
			break;

		case EA_ADEC:
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7] - %d;\n", rsft, step);
            wf_op("\tCPU->A[(Opcode >> %d) & 7] = adr;\n", rsft);
			break;

		case EA_ADEC7:
            wf_op("\tadr = CPU->A[7] - %d;\n", step);
            wf_op("\tCPU->A[7] = adr;\n");
			break;

		case EA_D16A:
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7] + (s32)(s16)FETCH_WORD;\n", rsft);
            wf_op("\tPC += 2;\n");
			break;

		case EA_D8AX:
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7];\n", rsft);
            wf_op("\tDECODE_EXT_WORD\n");
			break;

		case EA_A16:
            wf_op("\tadr = (s32)(s16)FETCH_WORD;\n");
            wf_op("\tPC += 2;\n");
			break;

		case EA_A32:
            wf_op("\tadr = (s32)FETCH_LONG;\n");
            wf_op("\tPC += 4;\n");
			break;

		case EA_D16P:
            wf_op("\tadr = PC + (s32)(s16)FETCH_WORD;\n");
            wf_op("\tPC += 2;\n");
			break;

		case EA_D8PX:
            wf_op("\tadr = PC;\n");
            wf_op("\tDECODE_EXT_WORD\n");
			break;
	}
}

static void _ea_calc(u32 ea, u32 rsft)
{
	u32 step;
	u32 cycle_sft;

    step = 0;
  	cycle_sft = 0;
    switch (current_size)
    {
        case SIZE_BYTE:
            if ((ea == EA_AINC7) || (ea == EA_ADEC7)) step = 2;
            else step = 1;
            break;

        case SIZE_WORD:
            step = 2;
            break;

        case SIZE_LONG:
        	cycle_sft = 1;
            step = 4;
            break;
    }

  	switch (ea)
	{
		case EA_DREG:
//            wf_op("\tadr = (u32)(&CPU->D[(Opcode >> %d) & 7]);\n", rsft);
			break;

		case EA_AREG:
//            wf_op("\tadr = (u32)(&CPU->A[(Opcode >> %d) & 7]);\n", rsft);
			break;

		case EA_IMM:
            current_cycle += (4 << cycle_sft) + 0;
            break;

		case EA_AIND:
            current_cycle += (4 << cycle_sft) + 0;
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7];\n", rsft);
			break;

		case EA_AINC:
            current_cycle += (4 << cycle_sft) + 0;
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7];\n", rsft);
            wf_op("\tCPU->A[(Opcode >> %d) & 7] += %d;\n", rsft, step);
			break;

		case EA_AINC7:
            current_cycle += (4 << cycle_sft) + 0;
            wf_op("\tadr = CPU->A[7];\n");
            wf_op("\tCPU->A[7] += %d;\n", step);
			break;

		case EA_ADEC:
            current_cycle += (4 << cycle_sft) + 2;
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7] - %d;\n", rsft, step);
            wf_op("\tCPU->A[(Opcode >> %d) & 7] = adr;\n", rsft);
			break;

		case EA_ADEC7:
            current_cycle += (4 << cycle_sft) + 2;
            wf_op("\tadr = CPU->A[7] - %d;\n", step);
            wf_op("\tCPU->A[7] = adr;\n");
			break;

		case EA_D16A:
            current_cycle += (4 << cycle_sft) + 4;
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7] + (s32)(s16)FETCH_WORD;\n", rsft);
            wf_op("\tPC += 2;\n");
			break;

		case EA_D8AX:
            current_cycle += (4 << cycle_sft) + 6;
            wf_op("\tadr = CPU->A[(Opcode >> %d) & 7];\n", rsft);
            wf_op("\tDECODE_EXT_WORD\n");
			break;

		case EA_A16:
            current_cycle += (4 << cycle_sft) + 4;
            wf_op("\tadr = (s32)(s16)FETCH_WORD;\n");
            wf_op("\tPC += 2;\n");
			break;

		case EA_A32:
            current_cycle += (4 << cycle_sft) + 8;
            wf_op("\tadr = (s32)FETCH_LONG;\n");
            wf_op("\tPC += 4;\n");
			break;

		case EA_D16P:
            current_cycle += (4 << cycle_sft) + 4;
            wf_op("\tadr = PC + (s32)(s16)FETCH_WORD;\n");
            wf_op("\tPC += 2;\n");
			break;

		case EA_D8PX:
            current_cycle += (4 << cycle_sft) + 6;
            wf_op("\tadr = PC;\n");
            wf_op("\tDECODE_EXT_WORD\n");
			break;
	}
}

static void _ea_read_(u32 ea, u32 rsft, char dest[4])
{
	char sz[8];

    switch (current_size)
    {
        case SIZE_BYTE:
            strcpy(sz, "BYTE");
            break;

        case SIZE_WORD:
            strcpy(sz, "WORD");
            break;

        case SIZE_LONG:
            strcpy(sz, "LONG");
            break;
    }

	switch (ea)
	{
		case EA_DREG:
            wf_op("\t%s = (%s)CPU->D[(Opcode >> %d) & 7];\n", dest, szc, rsft);
			break;

		case EA_AREG:
            if (current_size == SIZE_BYTE)
            {
                wf_op("\t// can't read byte from Ax registers !\n");
                wf_op("\tCPU->Status |= C68K_FAULTED;\n");
                //wf_op("\tCCnt = 0;\n");
                wf_op("\tgoto C68k_Exec_Really_End;\n");
            }
            else wf_op("\t%s = (%s)CPU->A[(Opcode >> %d) & 7];\n", dest, szc, rsft);
			break;
/*
		case EA_DREG:
		case EA_AREG:
            wf_op("\t%s = %s(adr);\n", dest, szcf);
            break;
*/
		case EA_A32:
		case EA_D8AX:
		case EA_D8PX:
		case EA_D16A:
		case EA_D16P:
		case EA_A16:
		case EA_ADEC:
		case EA_ADEC7:
		case EA_AIND:
		case EA_AINC:
		case EA_AINC7:
            mem_op("\tREAD_%s_F(adr, %s)\n", sz, dest);
			break;

		case EA_IMM:
            switch (current_size)
            {
                case SIZE_BYTE:
                    wf_op("\t%s = FETCH_BYTE;\n", dest);
                    wf_op("\tPC += 2;\n");
                    break;

                case SIZE_WORD:
                    wf_op("\t%s = FETCH_WORD;\n", dest);
                    wf_op("\tPC += 2;\n");
                    break;

                case SIZE_LONG:
                    wf_op("\t%s = FETCH_LONG;\n", dest);
                    wf_op("\tPC += 4;\n");
                    break;
            }
			break;
	}
}

static void _ea_read(u32 ea, u32 rsft)
{
    _ea_read_(ea, rsft, "res");
}

static void _ea_read_src(u32 ea, u32 rsft)
{
    _ea_read_(ea, rsft, "src");
}

static void _ea_read_dst(u32 ea, u32 rsft)
{
    _ea_read_(ea, rsft, "dst");
}

static void _ea_read_sx_(u32 ea, u32 rsft, char dest[4])
{
	char sz[8];

    switch (current_size)
    {
        case SIZE_BYTE:
            strcpy(sz, "BYTE");
            break;

        case SIZE_WORD:
            strcpy(sz, "WORD");
            break;

        case SIZE_LONG:
            strcpy(sz, "LONG");
            break;
    }

	switch (ea)
	{
		case EA_DREG:
            wf_op("\t%s = (s32)(%s)CPU->D[(Opcode >> %d) & 7];\n", dest, szcs, rsft);
			break;

		case EA_AREG:
            if (current_size == SIZE_BYTE)
            {
                wf_op("\t// can't read byte from Ax registers !\n");
                wf_op("\tCPU->Status |= C68K_FAULTED;\n");
                //wf_op("\tCCnt = 0;\n");
                wf_op("\tgoto C68k_Exec_Really_End;\n");
            }
            else wf_op("\t%s = (s32)(%s)CPU->A[(Opcode >> %d) & 7];\n", dest, szcs, rsft);
			break;
/*
		case EA_DREG:
		case EA_AREG:
            wf_op("\t%s = (s32)(%s(adr));\n", dest, szcsf);
			break;
*/
		case EA_A32:
		case EA_D8AX:
		case EA_D8PX:
		case EA_D16A:
		case EA_D16P:
		case EA_A16:
		case EA_ADEC:
		case EA_ADEC7:
		case EA_AIND:
		case EA_AINC:
		case EA_AINC7:
            mem_op("\tREADSX_%s_F(adr, %s)\n", sz, dest);
			break;

		case EA_IMM:
            switch (current_size)
            {
                case SIZE_BYTE:
                    wf_op("\t%s = (s32)(%s(PC)));\n", dest, szcsf);
                    wf_op("\tPC += 2;\n");
                    break;

                case SIZE_WORD:
                    wf_op("\t%s = (s32)(%s)FETCH_WORD;\n", dest, szcs);
                    wf_op("\tPC += 2;\n");
                    break;

                case SIZE_LONG:
                    wf_op("\t%s = (s32)(%s)FETCH_LONG;\n", dest, szcs);
                    wf_op("\tPC += 4;\n");
                    break;
            }
			break;
	}
}

static void _ea_read_sx(u32 ea, u32 rsft)
{
    _ea_read_sx_(ea, rsft, "res");
}

static void _ea_read_src_sx(u32 ea, u32 rsft)
{
    _ea_read_sx_(ea, rsft, "src");
}

static void _ea_write_sarsie(u32 ea, u32 rsft, int move_pd)
{
	char sz[8];

    switch (current_size)
    {
        case SIZE_BYTE:
            strcpy(sz, "BYTE");
            break;

        case SIZE_WORD:
            strcpy(sz, "WORD");
            break;

        case SIZE_LONG:
            strcpy(sz, "LONG");
            break;
    }

	if(current_size == SIZE_LONG && move_pd && (ea == EA_ADEC || ea == EA_ADEC7))
	{
         mem_op("\tWRITE_LONG_DEC_F(adr, res)\n");
	}
	else switch (ea)
	{
		case EA_DREG:
            wf_op("\t%s(&CPU->D[(Opcode >> %d) & 7])) = res;\n", szcf, 
rsft);
			break;

		case EA_AREG:
            // writes in Ax registers are always 32 bits sized
            wf_op("\tCPU->A[(Opcode >> %d) & 7] = res;\n", rsft);
			break;
/*
		case EA_DREG:
		case EA_AREG:
            wf_op("\t%s(adr) = res;\n", szcf);
			break;
*/
		case EA_A32:
		case EA_D8AX:
		case EA_D8PX:
		case EA_D16A:
		case EA_D16P:
		case EA_A16:
		case EA_ADEC:
		case EA_ADEC7:
		case EA_AIND:
		case EA_AINC:
		case EA_AINC7:
            mem_op("\tWRITE_%s_F(adr, res)\n", sz);
			break;
	}
}

static void _ea_write(u32 ea, u32 rsft)
{
 _ea_write_sarsie(ea, rsft, 0);
}

// misc function
/////////////////

static u32 get_current_opcode_base()
{
    u32 base;

    base = current_op->op_base;
    if (current_op->eam_sft != -1) base += (current_eam & 7) << current_op->eam_sft;
    if (current_op->reg_sft != -1) base += (current_reg & 7) << current_op->reg_sft;
    if (current_op->eam2_sft != -1) base += (current_eam2 & 7) << current_op->eam2_sft;
    if (current_op->reg2_sft != -1) base += (current_reg2 & 7) << current_op->reg2_sft;
    if (current_op->size_type == 1) base += (current_size - 1) << current_op->size_sft;
    else if (current_op->size_type == 2) base += (current_size & 3) << current_op->size_sft;

    return base;
}

static void start_all(int v)
{
    u32 base;

    base = get_current_opcode_base();

    // generate jump table
    gen_opjumptable(base);

    // generate label & declarations
    start_op(base, v);
}

static void set_current_size(u32 sz)
{
    current_size = sz;
    switch(current_size)
    {
        case SIZE_BYTE:
            current_bits_mask = 0xFF;
            current_sft_mask = 7;
            strcpy(szc, "u8");
            strcpy(szcf, "*(BYTE_OFF + (u8*)");
            strcpy(szcs, "s8");
            strcpy(szcsf, "*(BYTE_OFF + (s8*)");
            break;

        case SIZE_WORD:
            current_bits_mask = 0xFFFF;
            current_sft_mask = 15;
            strcpy(szc, "u16");
            strcpy(szcf, "*(WORD_OFF + (u16*)");
            strcpy(szcs, "s16");
            strcpy(szcsf, "*(WORD_OFF + (s16*)");
            break;

        case SIZE_LONG:
            current_bits_mask = 0xFFFFFFFF;
            current_sft_mask = 31;
            strcpy(szc, "u32");
            strcpy(szcf, "*((u32*)");
            strcpy(szcs, "s32");
            strcpy(szcsf, "*((s32*)");
            break;
    }
}

// gen privilege exception (happen when S flag is not set)
static void gen_privilege_exception(char *pre)
{
    // swap A7 and USP (because S not set)
    wf_op("%sres = CPU->USP;\n", pre);
    wf_op("%sCPU->USP = CPU->A[7];\n", pre);
    wf_op("%sCPU->A[7] = res;\n", pre);

    // get vector & add cycle
    wf_op("%sres = C68K_PRIVILEGE_VIOLATION_EX;\n", pre);
    adds_CCnt("c68k_exception_cycle_table[res]");

    // we will do some mem/io access
    // do_pre_io();

    // push PC and SR
    mem_op("%sPUSH_32_F(PC)\n", pre);
    mem_op("%sPUSH_16_F(GET_SR)\n", pre);

    // adjust SR
    wf_op("%sCPU->flag_S = C68K_SR_S;\n", pre);

    // fetch new PC
    mem_op("%sREAD_LONG_F(res * 4, PC)\n", pre);
    wf_op("%sSET_PC(PC)\n", pre);
}

static void gen_exception(char *pre, char* exception)
{
    // swap A7 and USP if needed
    wf_op("%sif (!CPU->flag_S)\n", pre);
    wf_op("%s{\n", pre);
        wf_op("%s\tres = CPU->USP;\n", pre);
        wf_op("%s\tCPU->USP = CPU->A[7];\n", pre);
        wf_op("%s\tCPU->A[7] = res;\n", pre);
    wf_op("%s}\n", pre);

    // get vector & add cycle
    wf_op("%sres = %s;\n", pre, exception);
    adds_CCnt("c68k_exception_cycle_table[res]");

    // we will do some mem/io access
    // do_pre_io();

    // push PC and SR
    mem_op("%sPUSH_32_F(PC)\n", pre);
    mem_op("%sPUSH_16_F(GET_SR)\n", pre);

    // adjust SR
    wf_op("%sCPU->flag_S = C68K_SR_S;\n", pre);

    // fetch new PC
    mem_op("%sREAD_LONG_F(res * 4, PC)\n", pre);
    wf_op("%sSET_PC(PC)\n", pre);
}

