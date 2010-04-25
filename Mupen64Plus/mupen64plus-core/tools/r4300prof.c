/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - r4300prof.c                                             *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Richard Goedeken                                   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Global data */
unsigned int instr_samples[132];
char instr_name[][10] =
{
  "reserved", "NI",     "J",      "JAL",    "BEQ",    "BNE",     "BLEZ",    "BGTZ",
  "ADDI",     "ADDIU",  "SLTI",   "SLTIU",  "ANDI",   "ORI",     "XORI",    "LUI",
  "BEQL",     "BNEL",   "BLEZL",  "BGTZL",  "DADDI",  "DADDIU",  "LDL",     "LDR",
  "LB",       "LH",     "LW",     "LWL",    "LBU",    "LHU",     "LWU",     "LWR",
  "SB",       "SH",     "SW",     "SWL",    "SWR",    "SDL",     "SDR",     "LWC1",
  "LDC1",     "LD",     "LL",     "SWC1",   "SDC1",   "SD",      "SC",      "BLTZ",
  "BGEZ",     "BLTZL",  "BGEZL",  "BLTZAL", "BGEZAL", "BLTZALL", "BGEZALL", "SLL",
  "SRL",      "SRA",    "SLLV",   "SRLV",   "SRAV",   "JR",      "JALR",    "SYSCALL",
  "MFHI",     "MTHI",   "MFLO",   "MTLO",   "DSLLV",  "DSRLV",   "DSRAV",   "MULT",
  "MULTU",    "DIV",    "DIVU",   "DMULT",  "DMULTU", "DDIV",    "DDIVU",   "ADD",
  "ADDU",     "SUB",    "SUBU",   "AND",    "OR",     "XOR",     "NOR",     "SLT",
  "SLTU",     "DADD",   "DADDU",  "DSUB",   "DSUBU",  "DSLL",    "DSRL",    "DSRA",
  "TEQ",      "DSLL32", "DSRL32", "DSRA32", "BC1F",   "BC1T",    "BC1FL",   "BC1TL",
  "TLBWI",    "TLBP",   "TLBR",   "TLBWR",  "ERET",   "MFC0",    "MTC0",    "MFC1",
  "DMFC1",    "CFC1",   "MTC1",   "DMTC1",  "CTC1",   "f.CVT",   "f.CMP",   "f.ADD",
  "f.SUB",    "f.MUL",  "f.DIV",  "f.SQRT", "f.ABS",  "f.MOV",   "f.NEG",   "f.ROUND",
  "f.TRUNC",  "f.CEIL", "f.FLOOR"
};
unsigned int instr_type[131] = {  9, 10,  6,  6,  7,  7,  7,  7,  3,  3,  4,  4,  3,  4,  4,  0,
                                  7,  7,  7,  7,  4,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
                                  1,  1,  1,  1,  1,  1,  1,  0,  0,  0,  0,  1,  1,  1,  1,  7,
                                  7,  7,  7,  7,  7,  7,  7,  3,  3,  3,  3,  3,  3,  6,  6, 10,
                                  2,  2,  2,  2,  4,  4,  4,  3,  3,  3,  3,  4,  4,  4,  4,  3,
                                  3,  3,  3,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,
                                  8,  4,  4,  4,  7,  7,  7,  7, 10, 10, 10, 10,  8,  2,  2,  2,
                                  2,  2,  2,  2,  2,  2,  5,  5,  5,  5,  5,  5,  5,  2,  5,  5,
                                  5,  5,  5 };
char instr_typename[][20] = { "Load", "Store", "Data move/convert", "32-bit math", "64-bit math", "Float Math",
                              "Jump", "Branch", "Exceptions", "Reserved", "Other" };

/* Global functions */
int GetInstrType(int opcode);
int AddrCompare(const void *, const void *);
int ParseProfLine(const char *pchIn, long *plAddress, int *piSamples, float *pfPercentage);

/* defined types */
typedef struct __attribute__ ((__packed__))
{
  int   mipsop;
  long  x86addr;
} r4300op;

typedef struct
{
  long x86addr;
  int samples;
} profilehit;

/* static functions */
static int isSpace(char ch)
{
  return (ch == ' ' || ch == '\t' ? 1 : 0);
}

static int isNum(char ch)
{
  return (ch >= '0' && ch <= '9' ? 1 : 0);
}

static int isFloat(char ch)
{
  return ((ch >= '0' && ch <= '9') || ch == '.' || ch == '+' || ch == '-' || ch == 'e'  ? 1 : 0);
}

static int isHex(char ch)
{
  return ((ch >= '0' && ch <= '9') || ((ch & 0xdf) >= 'A' && (ch & 0xdf) <= 'F') ? 1 : 0);
}

/* main */
int main(int argc, void *argv[])
{
  long lOpStart, lOpEnd;
  int flength, oplistlength, totaltime, proflistlength;
  int samp_unknown, samp_blockend, samp_notcompiled, samp_wrappers, samp_flush;
  int i, j;
  FILE *pfIn;
  r4300op *pOpAddrTable;
  profilehit *pProfTable;
  char *pch, *pchSampleData;

  /* check arguments */
  if (argc < 3)
  {
    printf("Usage: r4300prof r4300addr.dat x86profile.txt\n\n");
    printf("r4300addr.dat  - binary table of r4300 opcodes and corresponding x86 starting addresses\n");
    printf("x86profile.txt - text file containing a list of profile sample counts by x86 address on the heap\n\n");
    return 1;
  }

  /* open r4300 opcode/x86 address table generated from emulator run */
  printf("Loading %s...\n", argv[1]);
  pfIn = fopen(argv[1], "rb");
  if (pfIn == NULL)
  {
    printf("Couldn't open input file: %s\n", argv[1]);
    return 2;
  }

  /* get file length and calculate number of r4300op table entries */
  fseek(pfIn, 0L, SEEK_END);
  flength = (int) ftell(pfIn);
  fseek(pfIn, 0L, SEEK_SET);
  oplistlength = flength / sizeof(r4300op);
  
  /* read the file */
  pOpAddrTable = (r4300op *) malloc(flength);
  if (pOpAddrTable == NULL)
  {
    printf("Failed to allocate %i bytes for OpAddrTable!\n", flength);
    fclose(pfIn);
    return 3;
  }
  fread(pOpAddrTable, 1, flength, pfIn);
  fclose(pfIn);
  printf("%i r4300 instruction locations read.\n", oplistlength);

  /* sort the opcode/address table according to x86addr */
  qsort(pOpAddrTable, oplistlength, sizeof(r4300op), AddrCompare);

  /* remove any 0-length r4300 instructions */
  i = 0;
  j = 0;
  while (i < oplistlength)
  {
    pOpAddrTable[j].mipsop = pOpAddrTable[i].mipsop;
    pOpAddrTable[j].x86addr = pOpAddrTable[i].x86addr;
    i++;
    if (pOpAddrTable[j].x86addr != pOpAddrTable[i].x86addr)
      j++;
  }
  oplistlength = j;
  printf("%i non-empty MIPS instructions.\n", oplistlength);

  /* convert each r4300 opcode to an instruction type index */
  for (i = 0; i < oplistlength; i++)
    if (pOpAddrTable[i].mipsop > 0 || pOpAddrTable[i].mipsop < -16)
      pOpAddrTable[i].mipsop = GetInstrType(pOpAddrTable[i].mipsop);

  /* open the profiling sample data file */
  printf("Loading %s...\n", argv[2]);
  pfIn = fopen(argv[2], "rb");
  if (pfIn == NULL)
  {
    printf("Couldn't open input file: %s\n", argv[2]);
    free(pOpAddrTable);
    return 4;
  }

  /* load it */
  fseek(pfIn, 0L, SEEK_END);
  flength = (int) ftell(pfIn);
  fseek(pfIn, 0L, SEEK_SET);
  pchSampleData = (char *) malloc(flength + 16);
  if (pchSampleData == NULL)
  {
    printf("Failed to allocate %i bytes for pchSampleData!\n", flength + 16);
    fclose(pfIn);
    free(pOpAddrTable);
    return 5;
  }
  fread(pchSampleData, 1, flength, pfIn);
  pchSampleData[flength] = 0;
  fclose(pfIn);
  
  /* count the number of newlines in the ascii-formatted sample data file */
  proflistlength = 1;
  pch = pchSampleData;
  while (pch = strchr(pch, '\n'))
  {
    proflistlength++;
    pch++;
  }
  printf("%i lines in sample data file.\n", proflistlength);
  
  /* extract text data into binary table */
  pProfTable = (profilehit *) malloc(proflistlength * sizeof(profilehit));
  if (pProfTable == NULL)
  {
    printf("Failed to allocate %i bytes for pProfTable!\n", proflistlength * sizeof(profilehit));
    free(pOpAddrTable);
    free(pchSampleData);
    return 6;
  }
  pch = pchSampleData;
  j = 0;
  long long llOffset = 0;
  while (j < proflistlength)
  {
    long lAddress;
    int iSamples;
    float fPercentage;
    char *pchNext = strchr(pch, '\n');
    if (pchNext != NULL) *pchNext++ = 0; // null-terminate this line
    if (strstr(pch, "range:0x") != NULL) // search for offset change
    {
      pch = strstr(pch, "range:0x") + 8; // extract hex value and update our offset
      char *pch2 = pch;
      while (isHex(*pch2)) pch2++;
      *pch2 = 0;
      llOffset = strtoll(pch, NULL, 16);
    }
    else // parse line for sample point
    {
      int rval = ParseProfLine(pch, &lAddress, &iSamples, &fPercentage);
      if (rval != 0)
      {
        pProfTable[j].x86addr = (unsigned long) (lAddress + llOffset);
        pProfTable[j].samples = iSamples;
        j++;
      }
    }
    pch = pchNext;
    if (pch == NULL) break;
  }
  free(pchSampleData);
  proflistlength = j;
  printf("Found %i profile hits.\n", proflistlength);

  /* clear r4300 instruction sample data table */
  for (i = 0; i < 132; i++)
    instr_samples[i] = 0;
    
  /* calculate r4300 instruction profiling data by merging the tables */
  samp_unknown  = 0;
  samp_blockend = 0;
  samp_notcompiled = 0;
  samp_wrappers = 0;
  samp_flush = 0;
  i = 0; // i == OpAddrTable index
  lOpStart = pOpAddrTable[0].x86addr;
  lOpEnd   = pOpAddrTable[1].x86addr;
  for (j = 0; j < proflistlength; j++) // j == pProfTable index
  {
    long lOpx86addr = pProfTable[j].x86addr;
    if (lOpx86addr >= lOpStart && lOpx86addr <= lOpEnd) /* these profile samples lie within current r4300 instruction */
    {
      int instr = pOpAddrTable[i].mipsop;
      if (instr == -1) printf("%lx sample point lies between %i/%lx and %i/%lx\n", lOpx86addr, instr, lOpStart, pOpAddrTable[i+1].mipsop, lOpEnd);

      if (instr == -1)
        samp_unknown += pProfTable[j].samples;
      else if (instr == -2)
        samp_notcompiled += pProfTable[j].samples;
      else if (instr == -3)
        samp_blockend += pProfTable[j].samples;
      else if (instr == -4)
        samp_wrappers += pProfTable[j].samples;
      else if (instr == -5)
        samp_flush += pProfTable[j].samples;
      else
        instr_samples[instr] += pProfTable[j].samples;
      continue;
    }
    if (lOpx86addr < pOpAddrTable[0].x86addr || lOpx86addr >= pOpAddrTable[oplistlength-1].x86addr)
    { /* outside the range of all recompiled instructions */
      samp_unknown += pProfTable[j].samples;
      continue;
    } 
    if (lOpx86addr < lOpStart) /* discontinuity in profile list, go back to start */
    {
      i = 0;
      lOpStart = pOpAddrTable[0].x86addr;
      lOpEnd   = pOpAddrTable[1].x86addr;
      j--;
      continue;
    }
    /* this profile point is ahead of current r4300 instruction */
    do  /* race ahead in r4300 opcode list until we hit this profile sample point */
    {
      i++;
    } while (i+1 < oplistlength && lOpx86addr > pOpAddrTable[i+1].x86addr);
    lOpStart = pOpAddrTable[i].x86addr;
    lOpEnd   = pOpAddrTable[i+1].x86addr;
    if (lOpx86addr < lOpStart || lOpx86addr > lOpEnd)
    {
      printf("Error: lOpx86addr = %lx but lOpStart, lOpEnd = %lx, %lx\n", lOpx86addr, lOpStart, lOpEnd);
      return 7;
    }
    /* we have found the correct r4300 instruction corresponding to this profile point */
    j--;
  }

  /* print the results */
  unsigned int iTypeCount[11] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  printf("\nInstruction time (samples):\n");
  totaltime = 0;
  for (i = 0; i < 131; i++)
  {
    printf("%8s: %08i  ", instr_name[i], instr_samples[i]);
    if (i % 5 == 4) printf("\n");
    iTypeCount[instr_type[i]] += instr_samples[i];
    totaltime += instr_samples[i];
  }
  int special = samp_flush + samp_wrappers + samp_notcompiled + samp_blockend;
  printf("\n\nSpecial code samples:\n");
  printf("  Regcache flushing: %i\n", samp_flush);
  printf("  Jump wrappers: %i\n", samp_wrappers);
  printf("  NOTCOMPILED: %i\n", samp_notcompiled);
  printf("  block postfix & link samples: %i\n", samp_blockend);

  printf("\nUnaccounted samples: %i\n", samp_unknown);
  printf("Total accounted instruction samples: %i\n", totaltime + special);
  for (i = 0; i < 11; i++)
  {
    printf("%20s: %04.1f%% (%i)\n", instr_typename[i], (float) iTypeCount[i] * 100.0 / totaltime, iTypeCount[i]);
  }

  free(pOpAddrTable);
  free(pProfTable);
  return 0;
}

int AddrCompare(const void *p1, const void *p2)
{
  const r4300op *pOp1 = (const r4300op *) p1;
  const r4300op *pOp2 = (const r4300op *) p2;

  if (pOp1->x86addr < pOp2->x86addr)
    return -1;
  else if (pOp1->x86addr == pOp2->x86addr)
    return (int) (pOp1 - pOp2); /* this forces qsort to be stable */
  else
    return 1;
}

int ParseProfLine(const char *pchIn, long *plAddress, int *piSamples, float *pfPercentage)
{
  char chVal[128], *pchOut;
  
  /* skip any initial whitespace */
  while (isSpace(*pchIn)) pchIn++;
  if (!isHex(*pchIn)) return 0;
  
  /* parse hexadecimal address value */
  pchOut = chVal;
  while (isHex(*pchIn)) *pchOut++ = *pchIn++;
  *pchOut = 0;
  if (!isSpace(*pchIn)) return 0;
  *plAddress = strtol(chVal, NULL, 16);
  
  /* skip more whitespace */
  while (isSpace(*pchIn)) pchIn++;
  if (!isNum(*pchIn)) return 0;
  
  /* parse decimal sample count value */
  pchOut = chVal;
  while (isNum(*pchIn)) *pchOut++ = *pchIn++;
  *pchOut = 0;
  if (!isSpace(*pchIn)) return 0;
  *piSamples = atoi(chVal);
  
  /* skip more whitespace */
  while (isSpace(*pchIn)) pchIn++;
  if (!isFloat(*pchIn)) return 0;
  
  /* parse floating-point percentage value */
  pchOut = chVal;
  while (isFloat(*pchIn)) *pchOut++ = *pchIn++;
  *pchOut = 0;
  if (!isSpace(*pchIn) && *pchIn != '\r' && *pchIn != '\n' && *pchIn != 0) return 0;
  *pfPercentage = atof(chVal);

  /* if this isn't the end of the line, it's not a valid sample point */
  while (isSpace(*pchIn)) pchIn++;
  if (*pchIn != '\r' && *pchIn != '\n' && *pchIn != 0) return 0;
 
  return 1;
}

static int InstrTypeStd[64] =
{
   -1,  -1,  02,  03,  04,  05,  06,  07,   8,   9,  10,  11,  12,  13,  14,  15,
   -1,  -1,  00,  00,  16,  17,  18,  19,  20,  21,  22,  23,  00,  00,  00,  00,
   24,  25,  27,  26,  28,  29,  31,  30,  32,  33,  35,  34,  37,  38,  36,  01,
   42,  39,  00,  00,  01,  40,  00,  41,  46,  43,  00,  00,  01,  44,  00,  45
};

static int InstrTypeSpecial[64] =
{
  55,  00,  56,  57,  58,  00,  59,  60,
  61,  62,  00,  00,  63,  01,  00,  00,
  64,  65,  66,  67,  68,  00,  69,  70,
  71,  72,  73,  74,  75,  76,  77,  78,
  79,  80,  81,  82,  83,  84,  85,  86,
  00,  00,  87,  88,  89,  90,  91,  92,
  01,  01,  01,  01,  96,  00,  01,  00,
  93,  00,  94,  95,  97,  00,  98,  99
};
                        
static int InstrTypeRegImm[32] =
{
  47, 48, 49, 50, 00, 00, 00, 00, 01, 01, 01, 01, 01, 00, 01, 00,
  51, 52, 53, 54, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
};

static int InstrTypeCop1[32] =
{
   111, 112, 113, 00,  114, 115, 116,  00,
    -1,  00,  00,  00,  00,  00,  00,  00,
    -1,  -1,  00,  00,  -1,  -1,  00,  00,
    00,  00,  00,  00,  00,  00,  00,  00
};

static int InstrTypeCop1Math[64] =
{
  119,  120,  121,  122,  123,  124,  125,  126,
  127,  128,  129,  130,  127,  128,  129,  130,
   00,   00,   00,   00,   00,   00,   00,   00,
   00,   00,   00,   00,   00,   00,   00,   00,
  117,  117,   00,   00,  117,  117,   00,   00,
   00,   00,   00,   00,   00,   00,   00,   00,
  118,  118,  118,  118,  118,  118,  118,  118,
  118,  118,  118,  118,  118,  118,  118,  118
};
                        
            
int GetInstrType(int opcode)
{
  int iType = (opcode >> 26) & 63;
  
  if (iType == 0)
  {
    /* SPECIAL instruction */
    iType = opcode & 63;
    return InstrTypeSpecial[iType];
  }
  else if (iType == 1)
  {
    /* REGIMM instruction */
    iType = (opcode >> 16) & 31;
    return InstrTypeRegImm[iType];
  }
  else if (iType == 16)
  {
    /* COP0 instruction */
    int iType1 = opcode & 0x01FFFFFF;
    int iType2 = (opcode >> 21) & 31;
    if (iType1 == 1)
      return 106; // TLBR
    else if (iType1 == 2)
      return 104; // TLBWI
    else if (iType1 == 6)
      return 107; // TLBWR
    else if (iType1 == 8)
      return 105; // TLBP
    else if (iType1 == 24)
      return 108; // ERET
    else if ((opcode & 0x7FF) == 0 && iType2 == 0)
      return 109; // MFC0
    else if ((opcode & 0x7FF) == 0 && iType2 == 4)
      return 110; // MTC0
    else
      return 0; // reserved
  }
  else if (iType == 17)
  {
    /* COP1 instruction */
    int iType1 = (opcode >> 21) & 31;
    if (iType1 == 8)
    {
      /* conditional branch */
      int iType2 = (opcode >> 16) & 31;
      if (iType2 == 0)
        return 100; // BC1F
      else if (iType2 == 1)
        return 101; // BC1T
      else if (iType2 == 2)
        return 102; // BC1FL
      else if (iType2 == 3)
        return 103; // BC1TL
      else
        return 0; // reserved
    }
    else if (iType1 == 16 || iType1 == 17 || iType1 == 20 || iType1 == 21)
    {
      /* Single, Double, Word, Long instructions */
      int iType2 = opcode & 63;
      return InstrTypeCop1Math[iType2];
    }
    else
    {
      /* other Cop1 (move) */
      return InstrTypeCop1[iType1];
    }
  }

  /* standard MIPS instruction */
  return InstrTypeStd[iType];
}

