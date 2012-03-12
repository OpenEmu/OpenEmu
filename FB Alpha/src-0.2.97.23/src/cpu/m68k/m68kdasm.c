/* ======================================================================== */
/* ========================= LICENSING & COPYRIGHT ======================== */
/* ======================================================================== */
/*
 *                                  MUSASHI
 *                                Version 3.3
 *
 * A portable Motorola M680x0 processor emulation engine.
 * Copyright 1998-2001 Karl Stenerud.  All rights reserved.
 *
 * This code may be freely used for non-commercial purposes as long as this
 * copyright notice remains unaltered in the source code and any binary files
 * containing this code in compiled form.
 *
 * All other lisencing terms must be negotiated with the author
 * (Karl Stenerud).
 *
 * The latest version of this code can be obtained at:
 * http://kstenerud.cjb.net
 */



/* ======================================================================== */
/* ================================ INCLUDES ============================== */
/* ======================================================================== */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "m68k.h"

#ifndef DECL_SPEC
#define DECL_SPEC
#endif

/* ======================================================================== */
/* ============================ GENERAL DEFINES =========================== */
/* ======================================================================== */

/* unsigned int and int must be at least 32 bits wide */
#undef uint
#define uint unsigned int

/* Bit Isolation Functions */
#define BIT_0(A)  ((A) & 0x00000001)
#define BIT_1(A)  ((A) & 0x00000002)
#define BIT_2(A)  ((A) & 0x00000004)
#define BIT_3(A)  ((A) & 0x00000008)
#define BIT_4(A)  ((A) & 0x00000010)
#define BIT_5(A)  ((A) & 0x00000020)
#define BIT_6(A)  ((A) & 0x00000040)
#define BIT_7(A)  ((A) & 0x00000080)
#define BIT_8(A)  ((A) & 0x00000100)
#define BIT_9(A)  ((A) & 0x00000200)
#define BIT_A(A)  ((A) & 0x00000400)
#define BIT_B(A)  ((A) & 0x00000800)
#define BIT_C(A)  ((A) & 0x00001000)
#define BIT_D(A)  ((A) & 0x00002000)
#define BIT_E(A)  ((A) & 0x00004000)
#define BIT_F(A)  ((A) & 0x00008000)
#define BIT_10(A) ((A) & 0x00010000)
#define BIT_11(A) ((A) & 0x00020000)
#define BIT_12(A) ((A) & 0x00040000)
#define BIT_13(A) ((A) & 0x00080000)
#define BIT_14(A) ((A) & 0x00100000)
#define BIT_15(A) ((A) & 0x00200000)
#define BIT_16(A) ((A) & 0x00400000)
#define BIT_17(A) ((A) & 0x00800000)
#define BIT_18(A) ((A) & 0x01000000)
#define BIT_19(A) ((A) & 0x02000000)
#define BIT_1A(A) ((A) & 0x04000000)
#define BIT_1B(A) ((A) & 0x08000000)
#define BIT_1C(A) ((A) & 0x10000000)
#define BIT_1D(A) ((A) & 0x20000000)
#define BIT_1E(A) ((A) & 0x40000000)
#define BIT_1F(A) ((A) & 0x80000000)

/* These are the CPU types understood by this disassembler */
#define TYPE_68000 1
#define TYPE_68008 2
#define TYPE_68010 4
#define TYPE_68020 8
#define TYPE_68030 16
#define TYPE_68040 32

#define M68000_ONLY		(TYPE_68000 | TYPE_68008)

#define M68010_ONLY		TYPE_68010
#define M68010_LESS		(TYPE_68000 | TYPE_68008 | TYPE_68010)
#define M68010_PLUS		(TYPE_68010 | TYPE_68020 | TYPE_68030 | TYPE_68040)

#define M68020_ONLY 	TYPE_68020
#define M68020_LESS 	(TYPE_68010 | TYPE_68020)
#define M68020_PLUS		(TYPE_68020 | TYPE_68030 | TYPE_68040)

#define M68030_ONLY 	TYPE_68030
#define M68030_LESS 	(TYPE_68010 | TYPE_68020 | TYPE_68030)
#define M68030_PLUS		(TYPE_68030 | TYPE_68040)

#define M68040_PLUS		TYPE_68040


/* Extension word formats */
#define EXT_8BIT_DISPLACEMENT(A)          ((A)&0xff)
#define EXT_FULL(A)                       BIT_8(A)
#define EXT_EFFECTIVE_ZERO(A)             (((A)&0xe4) == 0xc4 || ((A)&0xe2) == 0xc0)
#define EXT_BASE_REGISTER_PRESENT(A)      (!BIT_7(A))
#define EXT_INDEX_REGISTER_PRESENT(A)     (!BIT_6(A))
#define EXT_INDEX_REGISTER(A)             (((A)>>12)&7)
#define EXT_INDEX_PRE_POST(A)             (EXT_INDEX_PRESENT(A) && (A)&3)
#define EXT_INDEX_PRE(A)                  (EXT_INDEX_PRESENT(A) && ((A)&7) < 4 && ((A)&7) != 0)
#define EXT_INDEX_POST(A)                 (EXT_INDEX_PRESENT(A) && ((A)&7) > 4)
#define EXT_INDEX_SCALE(A)                (((A)>>9)&3)
#define EXT_INDEX_LONG(A)                 BIT_B(A)
#define EXT_INDEX_AR(A)                   BIT_F(A)
#define EXT_BASE_DISPLACEMENT_PRESENT(A)  (((A)&0x30) > 0x10)
#define EXT_BASE_DISPLACEMENT_WORD(A)     (((A)&0x30) == 0x20)
#define EXT_BASE_DISPLACEMENT_LONG(A)     (((A)&0x30) == 0x30)
#define EXT_OUTER_DISPLACEMENT_PRESENT(A) (((A)&3) > 1 && ((A)&0x47) < 0x44)
#define EXT_OUTER_DISPLACEMENT_WORD(A)    (((A)&3) == 2 && ((A)&0x47) < 0x44)
#define EXT_OUTER_DISPLACEMENT_LONG(A)    (((A)&3) == 3 && ((A)&0x47) < 0x44)


/* Opcode flags */
#if M68K_COMPILE_FOR_MAME == OPT_ON
#define SET_OPCODE_FLAGS(x)	g_opcode_type = x;
#define COMBINE_OPCODE_FLAGS(x) ((x) | g_opcode_type | DASMFLAG_SUPPORTED)
#else
#define SET_OPCODE_FLAGS(x)
#define COMBINE_OPCODE_FLAGS(x) (x)
#endif


/* ======================================================================== */
/* =============================== PROTOTYPES ============================= */
/* ======================================================================== */

/* Read data at the PC and increment PC */
uint  read_imm_8(void);
uint  read_imm_16(void);
uint  read_imm_32(void);

/* Read data at the PC but don't imcrement the PC */
uint  peek_imm_8(void);
uint  peek_imm_16(void);
uint  peek_imm_32(void);

/* make signed integers 100% portably */
static int make_int_8(int value);
static int make_int_16(int value);

/* make a string of a hex value */
static char* make_signed_hex_str_8(uint val);
static char* make_signed_hex_str_16(uint val);
static char* make_signed_hex_str_32(uint val);

/* make string of ea mode */
static char* get_ea_mode_str(uint instruction, uint size);

char* get_ea_mode_str_8(uint instruction);
char* get_ea_mode_str_16(uint instruction);
char* get_ea_mode_str_32(uint instruction);

/* make string of immediate value */
static char* get_imm_str_s(uint size);
static char* get_imm_str_u(uint size);

char* get_imm_str_s8(void);
char* get_imm_str_s16(void);
char* get_imm_str_s32(void);

/* Stuff to build the opcode handler jump table */
static void  build_opcode_table(void);
static int   valid_ea(uint opcode, uint mask);
static int DECL_SPEC compare_nof_true_bits(const void *aptr, const void *bptr);

/* used to build opcode handler jump table */
typedef struct
{
	void (*opcode_handler)(void); /* handler function */
	uint mask;                    /* mask on opcode */
	uint match;                   /* what to match after masking */
	uint ea_mask;                 /* what ea modes are allowed */
} opcode_struct;



/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

/* Opcode handler jump table */
static void (*g_instruction_table[0x10000])(void);
/* Flag if disassembler initialized */
static int  g_initialized = 0;

/* Address mask to simulate address lines */
static unsigned int g_address_mask = 0xffffffff;

static char g_dasm_str[100]; /* string to hold disassembly */
static char g_helper_str[100]; /* string to hold helpful info */
static uint g_cpu_pc;        /* program counter */
static uint g_cpu_ir;        /* instruction register */
static uint g_cpu_type;
static uint g_opcode_type;

/* used by ops like asr, ror, addq, etc */
static uint g_3bit_qdata_table[8] = {8, 1, 2, 3, 4, 5, 6, 7};

static uint g_5bit_data_table[32] =
{
	32,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31
};

static const char* g_cc[16] =
{"t", "f", "hi", "ls", "cc", "cs", "ne", "eq", "vc", "vs", "pl", "mi", "ge", "lt", "gt", "le"};

static const char* g_cpcc[64] =
{/* 000    001    010    011    100    101    110    111 */
	  "f",  "eq", "ogt", "oge", "olt", "ole", "ogl",  "or", /* 000 */
	 "un", "ueq", "ugt", "uge", "ult", "ule",  "ne",   "t", /* 001 */
	 "sf", "seq",  "gt",  "ge",  "lt",  "le",  "gl"  "gle", /* 010 */
  "ngle", "ngl", "nle", "nlt", "nge", "ngt", "sne",  "st", /* 011 */
	  "?",   "?",   "?",   "?",   "?",   "?",   "?",   "?", /* 100 */
	  "?",   "?",   "?",   "?",   "?",   "?",   "?",   "?", /* 101 */
	  "?",   "?",   "?",   "?",   "?",   "?",   "?",   "?", /* 110 */
	  "?",   "?",   "?",   "?",   "?",   "?",   "?",   "?"  /* 111 */
};


/* ======================================================================== */
/* =========================== UTILITY FUNCTIONS ========================== */
/* ======================================================================== */

#define LIMIT_CPU_TYPES(ALLOWED_CPU_TYPES)	\
	if(!(g_cpu_type & ALLOWED_CPU_TYPES))	\
	{										\
		d68000_illegal();					\
		return;								\
	}

#define read_imm_8()  (m68k_read_disassembler_16(((g_cpu_pc+=2)-2)&g_address_mask)&0xff)
#define read_imm_16() m68k_read_disassembler_16(((g_cpu_pc+=2)-2)&g_address_mask)
#define read_imm_32() m68k_read_disassembler_32(((g_cpu_pc+=4)-4)&g_address_mask)

#define peek_imm_8()  (m68k_read_disassembler_16(g_cpu_pc & g_address_mask)&0xff)
#define peek_imm_16() m68k_read_disassembler_16(g_cpu_pc & g_address_mask)
#define peek_imm_32() m68k_read_disassembler_32(g_cpu_pc & g_address_mask)

/* Fake a split interface */
#define get_ea_mode_str_8(instruction) get_ea_mode_str(instruction, 0)
#define get_ea_mode_str_16(instruction) get_ea_mode_str(instruction, 1)
#define get_ea_mode_str_32(instruction) get_ea_mode_str(instruction, 2)

#define get_imm_str_s8() get_imm_str_s(0)
#define get_imm_str_s16() get_imm_str_s(1)
#define get_imm_str_s32() get_imm_str_s(2)

#define get_imm_str_u8() get_imm_str_u(0)
#define get_imm_str_u16() get_imm_str_u(1)
#define get_imm_str_u32() get_imm_str_u(2)


/* 100% portable signed int generators */
static int make_int_8(int value)
{
	return (value & 0x80) ? value | ~0xff : value & 0xff;
}

static int make_int_16(int value)
{
	return (value & 0x8000) ? value | ~0xffff : value & 0xffff;
}


/* Get string representation of hex values */
static char* make_signed_hex_str_8(uint val)
{
	static char str[20];

	val &= 0xff;

	if(val == 0x80)
		sprintf(str, "-$80");
	else if(val & 0x80)
		sprintf(str, "-$%x", (0-val) & 0x7f);
	else
		sprintf(str, "$%x", val & 0x7f);

	return str;
}

static char* make_signed_hex_str_16(uint val)
{
	static char str[20];

	val &= 0xffff;

	if(val == 0x8000)
		sprintf(str, "-$8000");
	else if(val & 0x8000)
		sprintf(str, "-$%x", (0-val) & 0x7fff);
	else
		sprintf(str, "$%x", val & 0x7fff);

	return str;
}

static char* make_signed_hex_str_32(uint val)
{
	static char str[20];

	val &= 0xffffffff;

	if(val == 0x80000000)
		sprintf(str, "-$80000000");
	else if(val & 0x80000000)
		sprintf(str, "-$%x", (0-val) & 0x7fffffff);
	else
		sprintf(str, "$%x", val & 0x7fffffff);

	return str;
}


/* make string of immediate value */
static char* get_imm_str_s(uint size)
{
	static char str[15];
	if(size == 0)
		sprintf(str, "#%s", make_signed_hex_str_8(read_imm_8()));
	else if(size == 1)
		sprintf(str, "#%s", make_signed_hex_str_16(read_imm_16()));
	else
		sprintf(str, "#%s", make_signed_hex_str_32(read_imm_32()));
	return str;
}

static char* get_imm_str_u(uint size)
{
	static char str[15];
	if(size == 0)
		sprintf(str, "#$%x", read_imm_8() & 0xff);
	else if(size == 1)
		sprintf(str, "#$%x", read_imm_16() & 0xffff);
	else
		sprintf(str, "#$%x", read_imm_32() & 0xffffffff);
	return str;
}

/* Make string of effective address mode */
static char* get_ea_mode_str(uint instruction, uint size)
{
	static char b1[64];
	static char b2[64];
	static char* mode = b2;
	uint extension;
	uint base;
	uint outer;
	char base_reg[4];
	char index_reg[8];
	uint preindex;
	uint postindex;
	uint comma = 0;
	uint temp_value;
	char invalid_mode = 0;

	/* Switch buffers so we don't clobber on a double-call to this function */
	mode = mode == b1 ? b2 : b1;

	switch(instruction & 0x3f)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07:
		/* data register direct */
			sprintf(mode, "D%d", instruction&7);
			break;
		case 0x08: case 0x09: case 0x0a: case 0x0b: case 0x0c: case 0x0d: case 0x0e: case 0x0f:
		/* address register direct */
			sprintf(mode, "A%d", instruction&7);
			break;
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17:
		/* address register indirect */
			sprintf(mode, "(A%d)", instruction&7);
			break;
		case 0x18: case 0x19: case 0x1a: case 0x1b: case 0x1c: case 0x1d: case 0x1e: case 0x1f:
		/* address register indirect with postincrement */
			sprintf(mode, "(A%d)+", instruction&7);
			break;
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27:
		/* address register indirect with predecrement */
			sprintf(mode, "-(A%d)", instruction&7);
			break;
		case 0x28: case 0x29: case 0x2a: case 0x2b: case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		/* address register indirect with displacement*/
			sprintf(mode, "(%s,A%d)", make_signed_hex_str_16(read_imm_16()), instruction&7);
			break;
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37:
		/* address register indirect with index */
			extension = read_imm_16();

			if((g_cpu_type & M68010_LESS) && EXT_INDEX_SCALE(extension))
			{
				invalid_mode = 1;
				break;
			}

			if(EXT_FULL(extension))
			{
				if(g_cpu_type & M68010_LESS)
				{
					invalid_mode = 1;
					break;
				}

				if(EXT_EFFECTIVE_ZERO(extension))
				{
					strcpy(mode, "0");
					break;
				}

				base = EXT_BASE_DISPLACEMENT_PRESENT(extension) ? (EXT_BASE_DISPLACEMENT_LONG(extension) ? read_imm_32() : read_imm_16()) : 0;
				outer = EXT_OUTER_DISPLACEMENT_PRESENT(extension) ? (EXT_OUTER_DISPLACEMENT_LONG(extension) ? read_imm_32() : read_imm_16()) : 0;
				if(EXT_BASE_REGISTER_PRESENT(extension))
					sprintf(base_reg, "A%d", instruction&7);
				else
					*base_reg = 0;
				if(EXT_INDEX_REGISTER_PRESENT(extension))
				{
					sprintf(index_reg, "%c%d.%c", EXT_INDEX_AR(extension) ? 'A' : 'D', EXT_INDEX_REGISTER(extension), EXT_INDEX_LONG(extension) ? 'l' : 'w');
					if(EXT_INDEX_SCALE(extension))
						sprintf(index_reg+strlen(index_reg), "*%d", 1 << EXT_INDEX_SCALE(extension));
				}
				else
					*index_reg = 0;
				preindex = (extension&7) > 0 && (extension&7) < 4;
				postindex = (extension&7) > 4;

				strcpy(mode, "(");
				if(preindex || postindex)
					strcat(mode, "[");
				if(base)
				{
					strcat(mode, make_signed_hex_str_16(base));
					comma = 1;
				}
				if(*base_reg)
				{
					if(comma)
						strcat(mode, ",");
					strcat(mode, base_reg);
					comma = 1;
				}
				if(postindex)
				{
					strcat(mode, "]");
					comma = 1;
				}
				if(*index_reg)
				{
					if(comma)
						strcat(mode, ",");
					strcat(mode, index_reg);
					comma = 1;
				}
				if(preindex)
				{
					strcat(mode, "]");
					comma = 1;
				}
				if(outer)
				{
					if(comma)
						strcat(mode, ",");
					strcat(mode, make_signed_hex_str_16(outer));
				}
				strcat(mode, ")");
				break;
			}

			if(EXT_8BIT_DISPLACEMENT(extension) == 0)
				sprintf(mode, "(A%d,%c%d.%c", instruction&7, EXT_INDEX_AR(extension) ? 'A' : 'D', EXT_INDEX_REGISTER(extension), EXT_INDEX_LONG(extension) ? 'l' : 'w');
			else
				sprintf(mode, "(%s,A%d,%c%d.%c", make_signed_hex_str_8(extension), instruction&7, EXT_INDEX_AR(extension) ? 'A' : 'D', EXT_INDEX_REGISTER(extension), EXT_INDEX_LONG(extension) ? 'l' : 'w');
			if(EXT_INDEX_SCALE(extension))
				sprintf(mode+strlen(mode), "*%d", 1 << EXT_INDEX_SCALE(extension));
			strcat(mode, ")");
			break;
		case 0x38:
		/* absolute short address */
			sprintf(mode, "$%x.w", read_imm_16());
			break;
		case 0x39:
		/* absolute long address */
			sprintf(mode, "$%x.l", read_imm_32());
			break;
		case 0x3a:
		/* program counter with displacement */
			temp_value = read_imm_16();
			sprintf(mode, "(%s,PC)", make_signed_hex_str_16(temp_value));
			sprintf(g_helper_str, "; ($%x)", (make_int_16(temp_value) + g_cpu_pc-2) & 0xffffffff);
			break;
		case 0x3b:
		/* program counter with index */
			extension = read_imm_16();

			if((g_cpu_type & M68010_LESS) && EXT_INDEX_SCALE(extension))
			{
				invalid_mode = 1;
				break;
			}

			if(EXT_FULL(extension))
			{
				if(g_cpu_type & M68010_LESS)
				{
					invalid_mode = 1;
					break;
				}

				if(EXT_EFFECTIVE_ZERO(extension))
				{
					strcpy(mode, "0");
					break;
				}
				base = EXT_BASE_DISPLACEMENT_PRESENT(extension) ? (EXT_BASE_DISPLACEMENT_LONG(extension) ? read_imm_32() : read_imm_16()) : 0;
				outer = EXT_OUTER_DISPLACEMENT_PRESENT(extension) ? (EXT_OUTER_DISPLACEMENT_LONG(extension) ? read_imm_32() : read_imm_16()) : 0;
				if(EXT_BASE_REGISTER_PRESENT(extension))
					strcpy(base_reg, "PC");
				else
					*base_reg = 0;
				if(EXT_INDEX_REGISTER_PRESENT(extension))
				{
					sprintf(index_reg, "%c%d.%c", EXT_INDEX_AR(extension) ? 'A' : 'D', EXT_INDEX_REGISTER(extension), EXT_INDEX_LONG(extension) ? 'l' : 'w');
					if(EXT_INDEX_SCALE(extension))
						sprintf(index_reg+strlen(index_reg), "*%d", 1 << EXT_INDEX_SCALE(extension));
				}
				else
					*index_reg = 0;
				preindex = (extension&7) > 0 && (extension&7) < 4;
				postindex = (extension&7) > 4;

				strcpy(mode, "(");
				if(preindex || postindex)
					strcat(mode, "[");
				if(base)
				{
					strcat(mode, make_signed_hex_str_16(base));
					comma = 1;
				}
				if(*base_reg)
				{
					if(comma)
						strcat(mode, ",");
					strcat(mode, base_reg);
					comma = 1;
				}
				if(postindex)
				{
					strcat(mode, "]");
					comma = 1;
				}
				if(*index_reg)
				{
					if(comma)
						strcat(mode, ",");
					strcat(mode, index_reg);
					comma = 1;
				}
				if(preindex)
				{
					strcat(mode, "]");
					comma = 1;
				}
				if(outer)
				{
					if(comma)
						strcat(mode, ",");
					strcat(mode, make_signed_hex_str_16(outer));
				}
				strcat(mode, ")");
				break;
			}

			if(EXT_8BIT_DISPLACEMENT(extension) == 0)
				sprintf(mode, "(PC,%c%d.%c", EXT_INDEX_AR(extension) ? 'A' : 'D', EXT_INDEX_REGISTER(extension), EXT_INDEX_LONG(extension) ? 'l' : 'w');
			else
				sprintf(mode, "(%s,PC,%c%d.%c", make_signed_hex_str_8(extension), EXT_INDEX_AR(extension) ? 'A' : 'D', EXT_INDEX_REGISTER(extension), EXT_INDEX_LONG(extension) ? 'l' : 'w');
			if(EXT_INDEX_SCALE(extension))
				sprintf(mode+strlen(mode), "*%d", 1 << EXT_INDEX_SCALE(extension));
			strcat(mode, ")");
			break;
		case 0x3c:
		/* Immediate */
			sprintf(mode, "%s", get_imm_str_u(size));
			break;
		default:
			invalid_mode = 1;
	}

	if(invalid_mode)
		sprintf(mode, "INVALID %x", instruction & 0x3f);

	return mode;
}



/* ======================================================================== */
/* ========================= INSTRUCTION HANDLERS ========================= */
/* ======================================================================== */
/* Instruction handler function names follow this convention:
 *
 * d68000_NAME_EXTENSIONS(void)
 * where NAME is the name of the opcode it handles and EXTENSIONS are any
 * extensions for special instances of that opcode.
 *
 * Examples:
 *   d68000_add_er_8(): add opcode, from effective address to register,
 *                      size = byte
 *
 *   d68000_asr_s_8(): arithmetic shift right, static count, size = byte
 *
 *
 * Common extensions:
 * 8   : size = byte
 * 16  : size = word
 * 32  : size = long
 * rr  : register to register
 * mm  : memory to memory
 * r   : register
 * s   : static
 * er  : effective address -> register
 * re  : register -> effective address
 * ea  : using effective address mode of operation
 * d   : data register direct
 * a   : address register direct
 * ai  : address register indirect
 * pi  : address register indirect with postincrement
 * pd  : address register indirect with predecrement
 * di  : address register indirect with displacement
 * ix  : address register indirect with index
 * aw  : absolute word
 * al  : absolute long
 */

static void d68000_illegal(void)
{
	sprintf(g_dasm_str, "dc.w $%04x; ILLEGAL", g_cpu_ir);
}

static void d68000_1010(void)
{
	sprintf(g_dasm_str, "dc.w    $%04x; opcode 1010", g_cpu_ir);
}


static void d68000_1111(void)
{
	sprintf(g_dasm_str, "dc.w    $%04x; opcode 1111", g_cpu_ir);
}


static void d68000_abcd_rr(void)
{
	sprintf(g_dasm_str, "abcd    D%d, D%d", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}


static void d68000_abcd_mm(void)
{
	sprintf(g_dasm_str, "abcd    -(A%d), -(A%d)", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_add_er_8(void)
{
	sprintf(g_dasm_str, "add.b   %s, D%d", get_ea_mode_str_8(g_cpu_ir), (g_cpu_ir>>9)&7);
}


static void d68000_add_er_16(void)
{
	sprintf(g_dasm_str, "add.w   %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_add_er_32(void)
{
	sprintf(g_dasm_str, "add.l   %s, D%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_add_re_8(void)
{
	sprintf(g_dasm_str, "add.b   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_add_re_16(void)
{
	sprintf(g_dasm_str, "add.w   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_add_re_32(void)
{
	sprintf(g_dasm_str, "add.l   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_adda_16(void)
{
	sprintf(g_dasm_str, "adda.w  %s, A%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_adda_32(void)
{
	sprintf(g_dasm_str, "adda.l  %s, A%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_addi_8(void)
{
	char* str = get_imm_str_s8();
	sprintf(g_dasm_str, "addi.b  %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_addi_16(void)
{
	char* str = get_imm_str_s16();
	sprintf(g_dasm_str, "addi.w  %s, %s", str, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_addi_32(void)
{
	char* str = get_imm_str_s32();
	sprintf(g_dasm_str, "addi.l  %s, %s", str, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_addq_8(void)
{
	sprintf(g_dasm_str, "addq.b  #%d, %s", g_3bit_qdata_table[(g_cpu_ir>>9)&7], get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_addq_16(void)
{
	sprintf(g_dasm_str, "addq.w  #%d, %s", g_3bit_qdata_table[(g_cpu_ir>>9)&7], get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_addq_32(void)
{
	sprintf(g_dasm_str, "addq.l  #%d, %s", g_3bit_qdata_table[(g_cpu_ir>>9)&7], get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_addx_rr_8(void)
{
	sprintf(g_dasm_str, "addx.b  D%d, D%d", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_addx_rr_16(void)
{
	sprintf(g_dasm_str, "addx.w  D%d, D%d", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_addx_rr_32(void)
{
	sprintf(g_dasm_str, "addx.l  D%d, D%d", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_addx_mm_8(void)
{
	sprintf(g_dasm_str, "addx.b  -(A%d), -(A%d)", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_addx_mm_16(void)
{
	sprintf(g_dasm_str, "addx.w  -(A%d), -(A%d)", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_addx_mm_32(void)
{
	sprintf(g_dasm_str, "addx.l  -(A%d), -(A%d)", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_and_er_8(void)
{
	sprintf(g_dasm_str, "and.b   %s, D%d", get_ea_mode_str_8(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_and_er_16(void)
{
	sprintf(g_dasm_str, "and.w   %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_and_er_32(void)
{
	sprintf(g_dasm_str, "and.l   %s, D%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_and_re_8(void)
{
	sprintf(g_dasm_str, "and.b   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_and_re_16(void)
{
	sprintf(g_dasm_str, "and.w   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_and_re_32(void)
{
	sprintf(g_dasm_str, "and.l   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_andi_8(void)
{
	char* str = get_imm_str_u8();
	sprintf(g_dasm_str, "andi.b  %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_andi_16(void)
{
	char* str = get_imm_str_u16();
	sprintf(g_dasm_str, "andi.w  %s, %s", str, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_andi_32(void)
{
	char* str = get_imm_str_u32();
	sprintf(g_dasm_str, "andi.l  %s, %s", str, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_andi_to_ccr(void)
{
	sprintf(g_dasm_str, "andi    %s, CCR", get_imm_str_u8());
}

static void d68000_andi_to_sr(void)
{
	sprintf(g_dasm_str, "andi    %s, SR", get_imm_str_u16());
}

static void d68000_asr_s_8(void)
{
	sprintf(g_dasm_str, "asr.b   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_asr_s_16(void)
{
	sprintf(g_dasm_str, "asr.w   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_asr_s_32(void)
{
	sprintf(g_dasm_str, "asr.l   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_asr_r_8(void)
{
	sprintf(g_dasm_str, "asr.b   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_asr_r_16(void)
{
	sprintf(g_dasm_str, "asr.w   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_asr_r_32(void)
{
	sprintf(g_dasm_str, "asr.l   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_asr_ea(void)
{
	sprintf(g_dasm_str, "asr.w   %s", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_asl_s_8(void)
{
	sprintf(g_dasm_str, "asl.b   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_asl_s_16(void)
{
	sprintf(g_dasm_str, "asl.w   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_asl_s_32(void)
{
	sprintf(g_dasm_str, "asl.l   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_asl_r_8(void)
{
	sprintf(g_dasm_str, "asl.b   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_asl_r_16(void)
{
	sprintf(g_dasm_str, "asl.w   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_asl_r_32(void)
{
	sprintf(g_dasm_str, "asl.l   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_asl_ea(void)
{
	sprintf(g_dasm_str, "asl.w   %s", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_bcc_8(void)
{
	uint temp_pc = g_cpu_pc;
	sprintf(g_dasm_str, "b%-2s     %x", g_cc[(g_cpu_ir>>8)&0xf], temp_pc + make_int_8(g_cpu_ir));
}

static void d68000_bcc_16(void)
{
	uint temp_pc = g_cpu_pc;
	sprintf(g_dasm_str, "b%-2s     %x", g_cc[(g_cpu_ir>>8)&0xf], temp_pc + make_int_16(read_imm_16()));
}

static void d68020_bcc_32(void)
{
	uint temp_pc = g_cpu_pc;
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "b%-2s     %x; (2+)", g_cc[(g_cpu_ir>>8)&0xf], temp_pc + read_imm_32());
}

static void d68000_bchg_r(void)
{
	sprintf(g_dasm_str, "bchg    D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_bchg_s(void)
{
	char* str = get_imm_str_u8();
	sprintf(g_dasm_str, "bchg    %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_bclr_r(void)
{
	sprintf(g_dasm_str, "bclr    D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_bclr_s(void)
{
	char* str = get_imm_str_u8();
	sprintf(g_dasm_str, "bclr    %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68010_bkpt(void)
{
	LIMIT_CPU_TYPES(M68010_PLUS);
	sprintf(g_dasm_str, "bkpt #%d; (1+)", g_cpu_ir&7);
}

static void d68020_bfchg(void)
{
	uint extension;
	char offset[3];
	char width[3];

	LIMIT_CPU_TYPES(M68020_PLUS);

	extension = read_imm_16();

	if(BIT_B(extension))
		sprintf(offset, "D%d", (extension>>6)&7);
	else
		sprintf(offset, "%d", (extension>>6)&31);
	if(BIT_5(extension))
		sprintf(width, "D%d", extension&7);
	else
		sprintf(width, "%d", g_5bit_data_table[extension&31]);
	sprintf(g_dasm_str, "bfchg   %s {%s:%s}; (2+)", get_ea_mode_str_8(g_cpu_ir), offset, width);
}

static void d68020_bfclr(void)
{
	uint extension;
	char offset[3];
	char width[3];

	LIMIT_CPU_TYPES(M68020_PLUS);

	extension = read_imm_16();

	if(BIT_B(extension))
		sprintf(offset, "D%d", (extension>>6)&7);
	else
		sprintf(offset, "%d", (extension>>6)&31);
	if(BIT_5(extension))
		sprintf(width, "D%d", extension&7);
	else
		sprintf(width, "%d", g_5bit_data_table[extension&31]);
	sprintf(g_dasm_str, "bfclr   %s {%s:%s}; (2+)", get_ea_mode_str_8(g_cpu_ir), offset, width);
}

static void d68020_bfexts(void)
{
	uint extension;
	char offset[3];
	char width[3];

	LIMIT_CPU_TYPES(M68020_PLUS);

	extension = read_imm_16();

	if(BIT_B(extension))
		sprintf(offset, "D%d", (extension>>6)&7);
	else
		sprintf(offset, "%d", (extension>>6)&31);
	if(BIT_5(extension))
		sprintf(width, "D%d", extension&7);
	else
		sprintf(width, "%d", g_5bit_data_table[extension&31]);
	sprintf(g_dasm_str, "bfexts  D%d, %s {%s:%s}; (2+)", (extension>>12)&7, get_ea_mode_str_8(g_cpu_ir), offset, width);
}

static void d68020_bfextu(void)
{
	uint extension;
	char offset[3];
	char width[3];

	LIMIT_CPU_TYPES(M68020_PLUS);

	extension = read_imm_16();

	if(BIT_B(extension))
		sprintf(offset, "D%d", (extension>>6)&7);
	else
		sprintf(offset, "%d", (extension>>6)&31);
	if(BIT_5(extension))
		sprintf(width, "D%d", extension&7);
	else
		sprintf(width, "%d", g_5bit_data_table[extension&31]);
	sprintf(g_dasm_str, "bfextu  D%d, %s {%s:%s}; (2+)", (extension>>12)&7, get_ea_mode_str_8(g_cpu_ir), offset, width);
}

static void d68020_bfffo(void)
{
	uint extension;
	char offset[3];
	char width[3];

	LIMIT_CPU_TYPES(M68020_PLUS);

	extension = read_imm_16();

	if(BIT_B(extension))
		sprintf(offset, "D%d", (extension>>6)&7);
	else
		sprintf(offset, "%d", (extension>>6)&31);
	if(BIT_5(extension))
		sprintf(width, "D%d", extension&7);
	else
		sprintf(width, "%d", g_5bit_data_table[extension&31]);
	sprintf(g_dasm_str, "bfffo   D%d, %s {%s:%s}; (2+)", (extension>>12)&7, get_ea_mode_str_8(g_cpu_ir), offset, width);
}

static void d68020_bfins(void)
{
	uint extension;
	char offset[3];
	char width[3];

	LIMIT_CPU_TYPES(M68020_PLUS);

	extension = read_imm_16();

	if(BIT_B(extension))
		sprintf(offset, "D%d", (extension>>6)&7);
	else
		sprintf(offset, "%d", (extension>>6)&31);
	if(BIT_5(extension))
		sprintf(width, "D%d", extension&7);
	else
		sprintf(width, "%d", g_5bit_data_table[extension&31]);
	sprintf(g_dasm_str, "bfins   D%d, %s {%s:%s}; (2+)", (extension>>12)&7, get_ea_mode_str_8(g_cpu_ir), offset, width);
}

static void d68020_bfset(void)
{
	uint extension;
	char offset[3];
	char width[3];

	LIMIT_CPU_TYPES(M68020_PLUS);

	extension = read_imm_16();

	if(BIT_B(extension))
		sprintf(offset, "D%d", (extension>>6)&7);
	else
		sprintf(offset, "%d", (extension>>6)&31);
	if(BIT_5(extension))
		sprintf(width, "D%d", extension&7);
	else
		sprintf(width, "%d", g_5bit_data_table[extension&31]);
	sprintf(g_dasm_str, "bfset   %s {%s:%s}; (2+)", get_ea_mode_str_8(g_cpu_ir), offset, width);
}

static void d68020_bftst(void)
{
	uint extension;
	char offset[3];
	char width[3];

	LIMIT_CPU_TYPES(M68020_PLUS);

	extension = read_imm_16();

	if(BIT_B(extension))
		sprintf(offset, "D%d", (extension>>6)&7);
	else
		sprintf(offset, "%d", (extension>>6)&31);
	if(BIT_5(extension))
		sprintf(width, "D%d", extension&7);
	else
		sprintf(width, "%d", g_5bit_data_table[extension&31]);
	sprintf(g_dasm_str, "bftst   %s {%s:%s}; (2+)", get_ea_mode_str_8(g_cpu_ir), offset, width);
}

static void d68000_bra_8(void)
{
	uint temp_pc = g_cpu_pc;
	sprintf(g_dasm_str, "bra     %x", temp_pc + make_int_8(g_cpu_ir));
}

static void d68000_bra_16(void)
{
	uint temp_pc = g_cpu_pc;
	sprintf(g_dasm_str, "bra     %x", temp_pc + make_int_16(read_imm_16()));
}

static void d68020_bra_32(void)
{
	uint temp_pc = g_cpu_pc;
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "bra     %x; (2+)", temp_pc + read_imm_32());
}

static void d68000_bset_r(void)
{
	sprintf(g_dasm_str, "bset    D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_bset_s(void)
{
	char* str = get_imm_str_u8();
	sprintf(g_dasm_str, "bset    %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_bsr_8(void)
{
	uint temp_pc = g_cpu_pc;
	sprintf(g_dasm_str, "bsr     %x", temp_pc + make_int_8(g_cpu_ir));
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68000_bsr_16(void)
{
	uint temp_pc = g_cpu_pc;
	sprintf(g_dasm_str, "bsr     %x", temp_pc + make_int_16(read_imm_16()));
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68020_bsr_32(void)
{
	uint temp_pc = g_cpu_pc;
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "bsr     %x; (2+)", temp_pc + peek_imm_32());
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68000_btst_r(void)
{
	sprintf(g_dasm_str, "btst    D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_btst_s(void)
{
	char* str = get_imm_str_u8();
	sprintf(g_dasm_str, "btst    %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_callm(void)
{
	char* str;
	LIMIT_CPU_TYPES(M68020_ONLY);
	str = get_imm_str_u8();

	sprintf(g_dasm_str, "callm   %s, %s; (2)", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_cas_8(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();
	sprintf(g_dasm_str, "cas.b   D%d, D%d, %s; (2+)", extension&7, (extension>>8)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_cas_16(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();
	sprintf(g_dasm_str, "cas.w   D%d, D%d, %s; (2+)", extension&7, (extension>>8)&7, get_ea_mode_str_16(g_cpu_ir));
}

static void d68020_cas_32(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();
	sprintf(g_dasm_str, "cas.l   D%d, D%d, %s; (2+)", extension&7, (extension>>8)&7, get_ea_mode_str_32(g_cpu_ir));
}

static void d68020_cas2_16(void)
{
/* CAS2 Dc1:Dc2,Du1:Dc2:(Rn1):(Rn2)
f e d c b a 9 8 7 6 5 4 3 2 1 0
 DARn1  0 0 0  Du1  0 0 0  Dc1
 DARn2  0 0 0  Du2  0 0 0  Dc2
*/

	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_32();
	sprintf(g_dasm_str, "cas2.w  D%d:D%d:D%d:D%d, (%c%d):(%c%d); (2+)",
		(extension>>16)&7, extension&7, (extension>>22)&7, (extension>>6)&7,
		BIT_1F(extension) ? 'A' : 'D', (extension>>28)&7,
		BIT_F(extension) ? 'A' : 'D', (extension>>12)&7);
}

static void d68020_cas2_32(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_32();
	sprintf(g_dasm_str, "cas2.l  D%d:D%d:D%d:D%d, (%c%d):(%c%d); (2+)",
		(extension>>16)&7, extension&7, (extension>>22)&7, (extension>>6)&7,
		BIT_1F(extension) ? 'A' : 'D', (extension>>28)&7,
		BIT_F(extension) ? 'A' : 'D', (extension>>12)&7);
}

static void d68000_chk_16(void)
{
	sprintf(g_dasm_str, "chk.w   %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68020_chk_32(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "chk.l   %s, D%d; (2+)", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68020_chk2_cmp2_8(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();
	sprintf(g_dasm_str, "%s.b  %s, %c%d; (2+)", BIT_B(extension) ? "chk2" : "cmp2", get_ea_mode_str_8(g_cpu_ir), BIT_F(extension) ? 'A' : 'D', (extension>>12)&7);
}

static void d68020_chk2_cmp2_16(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();
	sprintf(g_dasm_str, "%s.w  %s, %c%d; (2+)", BIT_B(extension) ? "chk2" : "cmp2", get_ea_mode_str_16(g_cpu_ir), BIT_F(extension) ? 'A' : 'D', (extension>>12)&7);
}

static void d68020_chk2_cmp2_32(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();
	sprintf(g_dasm_str, "%s.l  %s, %c%d; (2+)", BIT_B(extension) ? "chk2" : "cmp2", get_ea_mode_str_32(g_cpu_ir), BIT_F(extension) ? 'A' : 'D', (extension>>12)&7);
}

static void d68040_cinv(void)
{
	LIMIT_CPU_TYPES(M68040_PLUS);
	switch((g_cpu_ir>>3)&3)
	{
		case 0:
			sprintf(g_dasm_str, "cinv (illegal scope); (4)");
			break;
		case 1:
			sprintf(g_dasm_str, "cinvl   %d, (A%d); (4)", (g_cpu_ir>>6)&3, g_cpu_ir&7);
			break;
		case 2:
			sprintf(g_dasm_str, "cinvp   %d, (A%d); (4)", (g_cpu_ir>>6)&3, g_cpu_ir&7);
			break;
		case 3:
			sprintf(g_dasm_str, "cinva   %d; (4)", (g_cpu_ir>>6)&3);
			break;
	}
}

static void d68000_clr_8(void)
{
	sprintf(g_dasm_str, "clr.b   %s", get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_clr_16(void)
{
	sprintf(g_dasm_str, "clr.w   %s", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_clr_32(void)
{
	sprintf(g_dasm_str, "clr.l   %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_cmp_8(void)
{
	sprintf(g_dasm_str, "cmp.b   %s, D%d", get_ea_mode_str_8(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_cmp_16(void)
{
	sprintf(g_dasm_str, "cmp.w   %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_cmp_32(void)
{
	sprintf(g_dasm_str, "cmp.l   %s, D%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_cmpa_16(void)
{
	sprintf(g_dasm_str, "cmpa.w  %s, A%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_cmpa_32(void)
{
	sprintf(g_dasm_str, "cmpa.l  %s, A%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_cmpi_8(void)
{
	char* str = get_imm_str_s8();
	sprintf(g_dasm_str, "cmpi.b  %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_cmpi_pcdi_8(void)
{
	char* str;
	LIMIT_CPU_TYPES(M68010_PLUS);
	str = get_imm_str_s8();
	sprintf(g_dasm_str, "cmpi.b  %s, %s; (2+)", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_cmpi_pcix_8(void)
{
	char* str;
	LIMIT_CPU_TYPES(M68010_PLUS);
	str = get_imm_str_s8();
	sprintf(g_dasm_str, "cmpi.b  %s, %s; (2+)", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_cmpi_16(void)
{
	char* str;
	str = get_imm_str_s16();
	sprintf(g_dasm_str, "cmpi.w  %s, %s", str, get_ea_mode_str_16(g_cpu_ir));
}

static void d68020_cmpi_pcdi_16(void)
{
	char* str;
	LIMIT_CPU_TYPES(M68010_PLUS);
	str = get_imm_str_s16();
	sprintf(g_dasm_str, "cmpi.w  %s, %s; (2+)", str, get_ea_mode_str_16(g_cpu_ir));
}

static void d68020_cmpi_pcix_16(void)
{
	char* str;
	LIMIT_CPU_TYPES(M68010_PLUS);
	str = get_imm_str_s16();
	sprintf(g_dasm_str, "cmpi.w  %s, %s; (2+)", str, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_cmpi_32(void)
{
	char* str;
	str = get_imm_str_s32();
	sprintf(g_dasm_str, "cmpi.l  %s, %s", str, get_ea_mode_str_32(g_cpu_ir));
}

static void d68020_cmpi_pcdi_32(void)
{
	char* str;
	LIMIT_CPU_TYPES(M68010_PLUS);
	str = get_imm_str_s32();
	sprintf(g_dasm_str, "cmpi.l  %s, %s; (2+)", str, get_ea_mode_str_32(g_cpu_ir));
}

static void d68020_cmpi_pcix_32(void)
{
	char* str;
	LIMIT_CPU_TYPES(M68010_PLUS);
	str = get_imm_str_s32();
	sprintf(g_dasm_str, "cmpi.l  %s, %s; (2+)", str, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_cmpm_8(void)
{
	sprintf(g_dasm_str, "cmpm.b  (A%d)+, (A%d)+", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_cmpm_16(void)
{
	sprintf(g_dasm_str, "cmpm.w  (A%d)+, (A%d)+", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_cmpm_32(void)
{
	sprintf(g_dasm_str, "cmpm.l  (A%d)+, (A%d)+", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68020_cpbcc_16(void)
{
	uint extension;
	uint new_pc = g_cpu_pc;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();
	new_pc += make_int_16(peek_imm_16());
	sprintf(g_dasm_str, "%db%-4s  %s; %x (extension = %x) (2-3)", (g_cpu_ir>>9)&7, g_cpcc[g_cpu_ir&0x3f], get_imm_str_s16(), new_pc, extension);
}

static void d68020_cpbcc_32(void)
{
	uint extension;
	uint new_pc = g_cpu_pc;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();
	new_pc += peek_imm_32();
	sprintf(g_dasm_str, "%db%-4s  %s; %x (extension = %x) (2-3)", (g_cpu_ir>>9)&7, g_cpcc[g_cpu_ir&0x3f], get_imm_str_s16(), new_pc, extension);
}

static void d68020_cpdbcc(void)
{
	uint extension1;
	uint extension2;
	uint new_pc = g_cpu_pc;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension1 = read_imm_16();
	extension2 = read_imm_16();
	new_pc += make_int_16(peek_imm_16());
	sprintf(g_dasm_str, "%ddb%-4s D%d,%s; %x (extension = %x) (2-3)", (g_cpu_ir>>9)&7, g_cpcc[extension1&0x3f], g_cpu_ir&7, get_imm_str_s16(), new_pc, extension2);
}

static void d68020_cpgen(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "%dgen    %s; (2-3)", (g_cpu_ir>>9)&7, get_imm_str_u32());
}

static void d68020_cprestore(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "%drestore %s; (2-3)", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_cpsave(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "%dsave   %s; (2-3)", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_cpscc(void)
{
	uint extension1;
	uint extension2;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension1 = read_imm_16();
	extension2 = read_imm_16();
	sprintf(g_dasm_str, "%ds%-4s  %s; (extension = %x) (2-3)", (g_cpu_ir>>9)&7, g_cpcc[extension1&0x3f], get_ea_mode_str_8(g_cpu_ir), extension2);
}

static void d68020_cptrapcc_0(void)
{
	uint extension1;
	uint extension2;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension1 = read_imm_16();
	extension2 = read_imm_16();
	sprintf(g_dasm_str, "%dtrap%-4s; (extension = %x) (2-3)", (g_cpu_ir>>9)&7, g_cpcc[extension1&0x3f], extension2);
}

static void d68020_cptrapcc_16(void)
{
	uint extension1;
	uint extension2;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension1 = read_imm_16();
	extension2 = read_imm_16();
	sprintf(g_dasm_str, "%dtrap%-4s %s; (extension = %x) (2-3)", (g_cpu_ir>>9)&7, g_cpcc[extension1&0x3f], get_imm_str_u16(), extension2);
}

static void d68020_cptrapcc_32(void)
{
	uint extension1;
	uint extension2;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension1 = read_imm_16();
	extension2 = read_imm_16();
	sprintf(g_dasm_str, "%dtrap%-4s %s; (extension = %x) (2-3)", (g_cpu_ir>>9)&7, g_cpcc[extension1&0x3f], get_imm_str_u32(), extension2);
}

static void d68040_cpush(void)
{
	LIMIT_CPU_TYPES(M68040_PLUS);
	switch((g_cpu_ir>>3)&3)
	{
		case 0:
			sprintf(g_dasm_str, "cpush (illegal scope); (4)");
			break;
		case 1:
			sprintf(g_dasm_str, "cpushl  %d, (A%d); (4)", (g_cpu_ir>>6)&3, g_cpu_ir&7);
			break;
		case 2:
			sprintf(g_dasm_str, "cpushp  %d, (A%d); (4)", (g_cpu_ir>>6)&3, g_cpu_ir&7);
			break;
		case 3:
			sprintf(g_dasm_str, "cpusha  %d; (4)", (g_cpu_ir>>6)&3);
			break;
	}
}

static void d68000_dbra(void)
{
	uint temp_pc = g_cpu_pc;
	sprintf(g_dasm_str, "dbra    D%d, %x", g_cpu_ir & 7, temp_pc + make_int_16(read_imm_16()));
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68000_dbcc(void)
{
	uint temp_pc = g_cpu_pc;
	sprintf(g_dasm_str, "db%-2s    D%d, %x", g_cc[(g_cpu_ir>>8)&0xf], g_cpu_ir & 7, temp_pc + make_int_16(read_imm_16()));
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68000_divs(void)
{
	sprintf(g_dasm_str, "divs.w  %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_divu(void)
{
	sprintf(g_dasm_str, "divu.w  %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68020_divl(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();

	if(BIT_A(extension))
		sprintf(g_dasm_str, "div%c.l  %s, D%d:D%d; (2+)", BIT_B(extension) ? 's' : 'u', get_ea_mode_str_32(g_cpu_ir), extension&7, (extension>>12)&7);
	else if((extension&7) == ((extension>>12)&7))
		sprintf(g_dasm_str, "div%c.l  %s, D%d; (2+)", BIT_B(extension) ? 's' : 'u', get_ea_mode_str_32(g_cpu_ir), (extension>>12)&7);
	else
		sprintf(g_dasm_str, "div%cl.l %s, D%d:D%d; (2+)", BIT_B(extension) ? 's' : 'u', get_ea_mode_str_32(g_cpu_ir), extension&7, (extension>>12)&7);
}

static void d68000_eor_8(void)
{
	sprintf(g_dasm_str, "eor.b   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_eor_16(void)
{
	sprintf(g_dasm_str, "eor.w   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_eor_32(void)
{
	sprintf(g_dasm_str, "eor.l   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_eori_8(void)
{
	char* str = get_imm_str_u8();
	sprintf(g_dasm_str, "eori.b  %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_eori_16(void)
{
	char* str = get_imm_str_u16();
	sprintf(g_dasm_str, "eori.w  %s, %s", str, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_eori_32(void)
{
	char* str = get_imm_str_u32();
	sprintf(g_dasm_str, "eori.l  %s, %s", str, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_eori_to_ccr(void)
{
	sprintf(g_dasm_str, "eori    %s, CCR", get_imm_str_u8());
}

static void d68000_eori_to_sr(void)
{
	sprintf(g_dasm_str, "eori    %s, SR", get_imm_str_u16());
}

static void d68000_exg_dd(void)
{
	sprintf(g_dasm_str, "exg     D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_exg_aa(void)
{
	sprintf(g_dasm_str, "exg     A%d, A%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_exg_da(void)
{
	sprintf(g_dasm_str, "exg     D%d, A%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_ext_16(void)
{
	sprintf(g_dasm_str, "ext.w   D%d", g_cpu_ir&7);
}

static void d68000_ext_32(void)
{
	sprintf(g_dasm_str, "ext.l   D%d", g_cpu_ir&7);
}

static void d68020_extb_32(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "extb.l  D%d; (2+)", g_cpu_ir&7);
}

static void d68000_jmp(void)
{
	sprintf(g_dasm_str, "jmp     %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_jsr(void)
{
	sprintf(g_dasm_str, "jsr     %s", get_ea_mode_str_32(g_cpu_ir));
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68000_lea(void)
{
	sprintf(g_dasm_str, "lea     %s, A%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_link_16(void)
{
	sprintf(g_dasm_str, "link    A%d, %s", g_cpu_ir&7, get_imm_str_s16());
}

static void d68020_link_32(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "link    A%d, %s; (2+)", g_cpu_ir&7, get_imm_str_s32());
}

static void d68000_lsr_s_8(void)
{
	sprintf(g_dasm_str, "lsr.b   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_lsr_s_16(void)
{
	sprintf(g_dasm_str, "lsr.w   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_lsr_s_32(void)
{
	sprintf(g_dasm_str, "lsr.l   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_lsr_r_8(void)
{
	sprintf(g_dasm_str, "lsr.b   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_lsr_r_16(void)
{
	sprintf(g_dasm_str, "lsr.w   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_lsr_r_32(void)
{
	sprintf(g_dasm_str, "lsr.l   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_lsr_ea(void)
{
	sprintf(g_dasm_str, "lsr.w   %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_lsl_s_8(void)
{
	sprintf(g_dasm_str, "lsl.b   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_lsl_s_16(void)
{
	sprintf(g_dasm_str, "lsl.w   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_lsl_s_32(void)
{
	sprintf(g_dasm_str, "lsl.l   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_lsl_r_8(void)
{
	sprintf(g_dasm_str, "lsl.b   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_lsl_r_16(void)
{
	sprintf(g_dasm_str, "lsl.w   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_lsl_r_32(void)
{
	sprintf(g_dasm_str, "lsl.l   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_lsl_ea(void)
{
	sprintf(g_dasm_str, "lsl.w   %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_move_8(void)
{
	char* str = get_ea_mode_str_8(g_cpu_ir);
	sprintf(g_dasm_str, "move.b  %s, %s", str, get_ea_mode_str_8(((g_cpu_ir>>9) & 7) | ((g_cpu_ir>>3) & 0x38)));
}

static void d68000_move_16(void)
{
	char* str = get_ea_mode_str_16(g_cpu_ir);
	sprintf(g_dasm_str, "move.w  %s, %s", str, get_ea_mode_str_16(((g_cpu_ir>>9) & 7) | ((g_cpu_ir>>3) & 0x38)));
}

static void d68000_move_32(void)
{
	char* str = get_ea_mode_str_32(g_cpu_ir);
	sprintf(g_dasm_str, "move.l  %s, %s", str, get_ea_mode_str_32(((g_cpu_ir>>9) & 7) | ((g_cpu_ir>>3) & 0x38)));
}

static void d68000_movea_16(void)
{
	sprintf(g_dasm_str, "movea.w %s, A%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_movea_32(void)
{
	sprintf(g_dasm_str, "movea.l %s, A%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_move_to_ccr(void)
{
	sprintf(g_dasm_str, "move    %s, CCR", get_ea_mode_str_8(g_cpu_ir));
}

static void d68010_move_fr_ccr(void)
{
	LIMIT_CPU_TYPES(M68010_PLUS);
	sprintf(g_dasm_str, "move    CCR, %s; (1+)", get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_move_fr_sr(void)
{
	sprintf(g_dasm_str, "move    SR, %s", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_move_to_sr(void)
{
	sprintf(g_dasm_str, "move    %s, SR", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_move_fr_usp(void)
{
	sprintf(g_dasm_str, "move    USP, A%d", g_cpu_ir&7);
}

static void d68000_move_to_usp(void)
{
	sprintf(g_dasm_str, "move    A%d, USP", g_cpu_ir&7);
}

static void d68010_movec(void)
{
	uint extension;
	const char* reg_name;
	const char* processor;
	LIMIT_CPU_TYPES(M68010_PLUS);
	extension = read_imm_16();

	switch(extension & 0xfff)
	{
		case 0x000:
			reg_name = "SFC";
			processor = "1+";
			break;
		case 0x001:
			reg_name = "DFC";
			processor = "1+";
			break;
		case 0x800:
			reg_name = "USP";
			processor = "1+";
			break;
		case 0x801:
			reg_name = "VBR";
			processor = "1+";
			break;
		case 0x002:
			reg_name = "CACR";
			processor = "2+";
			break;
		case 0x802:
			reg_name = "CAAR";
			processor = "2,3";
			break;
		case 0x803:
			reg_name = "MSP";
			processor = "2+";
			break;
		case 0x804:
			reg_name = "ISP";
			processor = "2+";
			break;
		case 0x003:
			reg_name = "TC";
			processor = "4+";
			break;
		case 0x004:
			reg_name = "ITT0";
			processor = "4+";
			break;
		case 0x005:
			reg_name = "ITT1";
			processor = "4+";
			break;
		case 0x006:
			reg_name = "DTT0";
			processor = "4+";
			break;
		case 0x007:
			reg_name = "DTT1";
			processor = "4+";
			break;
		case 0x805:
			reg_name = "MMUSR";
			processor = "4+";
			break;
		case 0x806:
			reg_name = "URP";
			processor = "4+";
			break;
		case 0x807:
			reg_name = "SRP";
			processor = "4+";
			break;
		default:
			reg_name = make_signed_hex_str_16(extension & 0xfff);
			processor = "?";
	}

	if(BIT_1(g_cpu_ir))
		sprintf(g_dasm_str, "movec   %c%d, %s; (%s)", BIT_F(extension) ? 'A' : 'D', (extension>>12)&7, reg_name, processor);
	else
		sprintf(g_dasm_str, "movec   %s, %c%d; (%s)", reg_name, BIT_F(extension) ? 'A' : 'D', (extension>>12)&7, processor);
}

static void d68000_movem_pd_16(void)
{
	uint data = read_imm_16();
	char buffer[40];
	uint first;
	uint run_length;
	uint i;

	buffer[0] = 0;
	for(i=0;i<8;i++)
	{
		if(data&(1<<(15-i)))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(15-(i+1)))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "D%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-D%d", first + run_length);
		}
	}
	for(i=0;i<8;i++)
	{
		if(data&(1<<(7-i)))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(7-(i+1)))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "A%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-A%d", first + run_length);
		}
	}
	sprintf(g_dasm_str, "movem.w %s, %s", buffer, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_movem_pd_32(void)
{
	uint data = read_imm_16();
	char buffer[40];
	uint first;
	uint run_length;
	uint i;

	buffer[0] = 0;
	for(i=0;i<8;i++)
	{
		if(data&(1<<(15-i)))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(15-(i+1)))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "D%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-D%d", first + run_length);
		}
	}
	for(i=0;i<8;i++)
	{
		if(data&(1<<(7-i)))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(7-(i+1)))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "A%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-A%d", first + run_length);
		}
	}
	sprintf(g_dasm_str, "movem.l %s, %s", buffer, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_movem_er_16(void)
{
	uint data = read_imm_16();
	char buffer[40];
	uint first;
	uint run_length;
	uint i;

	buffer[0] = 0;
	for(i=0;i<8;i++)
	{
		if(data&(1<<i))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(i+1))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "D%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-D%d", first + run_length);
		}
	}
	for(i=0;i<8;i++)
	{
		if(data&(1<<(i+8)))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(i+8+1))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "A%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-A%d", first + run_length);
		}
	}
	sprintf(g_dasm_str, "movem.w %s, %s", get_ea_mode_str_16(g_cpu_ir), buffer);
}

static void d68000_movem_er_32(void)
{
	uint data = read_imm_16();
	char buffer[40];
	uint first;
	uint run_length;
	uint i;

	buffer[0] = 0;
	for(i=0;i<8;i++)
	{
		if(data&(1<<i))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(i+1))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "D%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-D%d", first + run_length);
		}
	}
	for(i=0;i<8;i++)
	{
		if(data&(1<<(i+8)))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(i+8+1))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "A%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-A%d", first + run_length);
		}
	}
	sprintf(g_dasm_str, "movem.l %s, %s", get_ea_mode_str_32(g_cpu_ir), buffer);
}

static void d68000_movem_re_16(void)
{
	uint data = read_imm_16();
	char buffer[40];
	uint first;
	uint run_length;
	uint i;

	buffer[0] = 0;
	for(i=0;i<8;i++)
	{
		if(data&(1<<i))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(i+1))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "D%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-D%d", first + run_length);
		}
	}
	for(i=0;i<8;i++)
	{
		if(data&(1<<(i+8)))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(i+8+1))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "A%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-A%d", first + run_length);
		}
	}
	sprintf(g_dasm_str, "movem.w %s, %s", buffer, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_movem_re_32(void)
{
	uint data = read_imm_16();
	char buffer[40];
	uint first;
	uint run_length;
	uint i;

	buffer[0] = 0;
	for(i=0;i<8;i++)
	{
		if(data&(1<<i))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(i+1))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "D%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-D%d", first + run_length);
		}
	}
	for(i=0;i<8;i++)
	{
		if(data&(1<<(i+8)))
		{
			first = i;
			run_length = 0;
			while(i<7 && (data&(1<<(i+8+1))))
			{
				i++;
				run_length++;
			}
			if(buffer[0] != 0)
				strcat(buffer, "/");
			sprintf(buffer+strlen(buffer), "A%d", first);
			if(run_length > 0)
				sprintf(buffer+strlen(buffer), "-A%d", first + run_length);
		}
	}
	sprintf(g_dasm_str, "movem.l %s, %s", buffer, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_movep_re_16(void)
{
	sprintf(g_dasm_str, "movep.w D%d, ($%x,A%d)", (g_cpu_ir>>9)&7, read_imm_16(), g_cpu_ir&7);
}

static void d68000_movep_re_32(void)
{
	sprintf(g_dasm_str, "movep.l D%d, ($%x,A%d)", (g_cpu_ir>>9)&7, read_imm_16(), g_cpu_ir&7);
}

static void d68000_movep_er_16(void)
{
	sprintf(g_dasm_str, "movep.w ($%x,A%d), D%d", read_imm_16(), g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_movep_er_32(void)
{
	sprintf(g_dasm_str, "movep.l ($%x,A%d), D%d", read_imm_16(), g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68010_moves_8(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68010_PLUS);
	extension = read_imm_16();
	if(BIT_B(extension))
		sprintf(g_dasm_str, "moves.b %c%d, %s; (1+)", BIT_F(extension) ? 'A' : 'D', (extension>>12)&7, get_ea_mode_str_8(g_cpu_ir));
	else
		sprintf(g_dasm_str, "moves.b %s, %c%d; (1+)", get_ea_mode_str_8(g_cpu_ir), BIT_F(extension) ? 'A' : 'D', (extension>>12)&7);
}

static void d68010_moves_16(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68010_PLUS);
	extension = read_imm_16();
	if(BIT_B(extension))
		sprintf(g_dasm_str, "moves.w %c%d, %s; (1+)", BIT_F(extension) ? 'A' : 'D', (extension>>12)&7, get_ea_mode_str_16(g_cpu_ir));
	else
		sprintf(g_dasm_str, "moves.w %s, %c%d; (1+)", get_ea_mode_str_16(g_cpu_ir), BIT_F(extension) ? 'A' : 'D', (extension>>12)&7);
}

static void d68010_moves_32(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68010_PLUS);
	extension = read_imm_16();
	if(BIT_B(extension))
		sprintf(g_dasm_str, "moves.l %c%d, %s; (1+)", BIT_F(extension) ? 'A' : 'D', (extension>>12)&7, get_ea_mode_str_32(g_cpu_ir));
	else
		sprintf(g_dasm_str, "moves.l %s, %c%d; (1+)", get_ea_mode_str_32(g_cpu_ir), BIT_F(extension) ? 'A' : 'D', (extension>>12)&7);
}

static void d68000_moveq(void)
{
	sprintf(g_dasm_str, "moveq   #%s, D%d", make_signed_hex_str_8(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68040_move16_pi_pi(void)
{
	LIMIT_CPU_TYPES(M68040_PLUS);
	sprintf(g_dasm_str, "move16  (A%d)+, (A%d)+; (4)", g_cpu_ir&7, (read_imm_16()>>12)&7);
}

static void d68040_move16_pi_al(void)
{
	LIMIT_CPU_TYPES(M68040_PLUS);
	sprintf(g_dasm_str, "move16  (A%d)+, %s; (4)", g_cpu_ir&7, get_imm_str_u32());
}

static void d68040_move16_al_pi(void)
{
	LIMIT_CPU_TYPES(M68040_PLUS);
	sprintf(g_dasm_str, "move16  %s, (A%d)+; (4)", get_imm_str_u32(), g_cpu_ir&7);
}

static void d68040_move16_ai_al(void)
{
	LIMIT_CPU_TYPES(M68040_PLUS);
	sprintf(g_dasm_str, "move16  (A%d), %s; (4)", g_cpu_ir&7, get_imm_str_u32());
}

static void d68040_move16_al_ai(void)
{
	LIMIT_CPU_TYPES(M68040_PLUS);
	sprintf(g_dasm_str, "move16  %s, (A%d); (4)", get_imm_str_u32(), g_cpu_ir&7);
}

static void d68000_muls(void)
{
	sprintf(g_dasm_str, "muls.w  %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_mulu(void)
{
	sprintf(g_dasm_str, "mulu.w  %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68020_mull(void)
{
	uint extension;
	LIMIT_CPU_TYPES(M68020_PLUS);
	extension = read_imm_16();

	if(BIT_A(extension))
		sprintf(g_dasm_str, "mul%c.l %s, D%d-D%d; (2+)", BIT_B(extension) ? 's' : 'u', get_ea_mode_str_32(g_cpu_ir), extension&7, (extension>>12)&7);
	else
		sprintf(g_dasm_str, "mul%c.l  %s, D%d; (2+)", BIT_B(extension) ? 's' : 'u', get_ea_mode_str_32(g_cpu_ir), (extension>>12)&7);
}

static void d68000_nbcd(void)
{
	sprintf(g_dasm_str, "nbcd    %s", get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_neg_8(void)
{
	sprintf(g_dasm_str, "neg.b   %s", get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_neg_16(void)
{
	sprintf(g_dasm_str, "neg.w   %s", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_neg_32(void)
{
	sprintf(g_dasm_str, "neg.l   %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_negx_8(void)
{
	sprintf(g_dasm_str, "negx.b  %s", get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_negx_16(void)
{
	sprintf(g_dasm_str, "negx.w  %s", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_negx_32(void)
{
	sprintf(g_dasm_str, "negx.l  %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_nop(void)
{
	sprintf(g_dasm_str, "nop");
}

static void d68000_not_8(void)
{
	sprintf(g_dasm_str, "not.b   %s", get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_not_16(void)
{
	sprintf(g_dasm_str, "not.w   %s", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_not_32(void)
{
	sprintf(g_dasm_str, "not.l   %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_or_er_8(void)
{
	sprintf(g_dasm_str, "or.b    %s, D%d", get_ea_mode_str_8(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_or_er_16(void)
{
	sprintf(g_dasm_str, "or.w    %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_or_er_32(void)
{
	sprintf(g_dasm_str, "or.l    %s, D%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_or_re_8(void)
{
	sprintf(g_dasm_str, "or.b    D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_or_re_16(void)
{
	sprintf(g_dasm_str, "or.w    D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_or_re_32(void)
{
	sprintf(g_dasm_str, "or.l    D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_ori_8(void)
{
	char* str = get_imm_str_u8();
	sprintf(g_dasm_str, "ori.b   %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_ori_16(void)
{
	char* str = get_imm_str_u16();
	sprintf(g_dasm_str, "ori.w   %s, %s", str, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_ori_32(void)
{
	char* str = get_imm_str_u32();
	sprintf(g_dasm_str, "ori.l   %s, %s", str, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_ori_to_ccr(void)
{
	sprintf(g_dasm_str, "ori     %s, CCR", get_imm_str_u8());
}

static void d68000_ori_to_sr(void)
{
	sprintf(g_dasm_str, "ori     %s, SR", get_imm_str_u16());
}

static void d68020_pack_rr(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "pack    D%d, D%d, %s; (2+)", g_cpu_ir&7, (g_cpu_ir>>9)&7, get_imm_str_u16());
}

static void d68020_pack_mm(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "pack    -(A%d), -(A%d), %s; (2+)", g_cpu_ir&7, (g_cpu_ir>>9)&7, get_imm_str_u16());
}

static void d68000_pea(void)
{
	sprintf(g_dasm_str, "pea     %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_reset(void)
{
	sprintf(g_dasm_str, "reset");
}

static void d68000_ror_s_8(void)
{
	sprintf(g_dasm_str, "ror.b   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_ror_s_16(void)
{
	sprintf(g_dasm_str, "ror.w   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7],g_cpu_ir&7);
}

static void d68000_ror_s_32(void)
{
	sprintf(g_dasm_str, "ror.l   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_ror_r_8(void)
{
	sprintf(g_dasm_str, "ror.b   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_ror_r_16(void)
{
	sprintf(g_dasm_str, "ror.w   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_ror_r_32(void)
{
	sprintf(g_dasm_str, "ror.l   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_ror_ea(void)
{
	sprintf(g_dasm_str, "ror.w   %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_rol_s_8(void)
{
	sprintf(g_dasm_str, "rol.b   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_rol_s_16(void)
{
	sprintf(g_dasm_str, "rol.w   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_rol_s_32(void)
{
	sprintf(g_dasm_str, "rol.l   #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_rol_r_8(void)
{
	sprintf(g_dasm_str, "rol.b   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_rol_r_16(void)
{
	sprintf(g_dasm_str, "rol.w   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_rol_r_32(void)
{
	sprintf(g_dasm_str, "rol.l   D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_rol_ea(void)
{
	sprintf(g_dasm_str, "rol.w   %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_roxr_s_8(void)
{
	sprintf(g_dasm_str, "roxr.b  #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_roxr_s_16(void)
{
	sprintf(g_dasm_str, "roxr.w  #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}


static void d68000_roxr_s_32(void)
{
	sprintf(g_dasm_str, "roxr.l  #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_roxr_r_8(void)
{
	sprintf(g_dasm_str, "roxr.b  D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_roxr_r_16(void)
{
	sprintf(g_dasm_str, "roxr.w  D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_roxr_r_32(void)
{
	sprintf(g_dasm_str, "roxr.l  D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_roxr_ea(void)
{
	sprintf(g_dasm_str, "roxr.w  %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_roxl_s_8(void)
{
	sprintf(g_dasm_str, "roxl.b  #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_roxl_s_16(void)
{
	sprintf(g_dasm_str, "roxl.w  #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_roxl_s_32(void)
{
	sprintf(g_dasm_str, "roxl.l  #%d, D%d", g_3bit_qdata_table[(g_cpu_ir>>9)&7], g_cpu_ir&7);
}

static void d68000_roxl_r_8(void)
{
	sprintf(g_dasm_str, "roxl.b  D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_roxl_r_16(void)
{
	sprintf(g_dasm_str, "roxl.w  D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_roxl_r_32(void)
{
	sprintf(g_dasm_str, "roxl.l  D%d, D%d", (g_cpu_ir>>9)&7, g_cpu_ir&7);
}

static void d68000_roxl_ea(void)
{
	sprintf(g_dasm_str, "roxl.w  %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68010_rtd(void)
{
	LIMIT_CPU_TYPES(M68010_PLUS);
	sprintf(g_dasm_str, "rtd     %s; (1+)", get_imm_str_s16());
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OUT);
}

static void d68000_rte(void)
{
	sprintf(g_dasm_str, "rte");
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OUT);
}

static void d68020_rtm(void)
{
	LIMIT_CPU_TYPES(M68020_ONLY);
	sprintf(g_dasm_str, "rtm     %c%d; (2+)", BIT_3(g_cpu_ir) ? 'A' : 'D', g_cpu_ir&7);
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OUT);
}

static void d68000_rtr(void)
{
	sprintf(g_dasm_str, "rtr");
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OUT);
}

static void d68000_rts(void)
{
	sprintf(g_dasm_str, "rts");
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OUT);
}

static void d68000_sbcd_rr(void)
{
	sprintf(g_dasm_str, "sbcd    D%d, D%d", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_sbcd_mm(void)
{
	sprintf(g_dasm_str, "sbcd    -(A%d), -(A%d)", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_scc(void)
{
	sprintf(g_dasm_str, "s%-2s     %s", g_cc[(g_cpu_ir>>8)&0xf], get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_stop(void)
{
	sprintf(g_dasm_str, "stop    %s", get_imm_str_s16());
}

static void d68000_sub_er_8(void)
{
	sprintf(g_dasm_str, "sub.b   %s, D%d", get_ea_mode_str_8(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_sub_er_16(void)
{
	sprintf(g_dasm_str, "sub.w   %s, D%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_sub_er_32(void)
{
	sprintf(g_dasm_str, "sub.l   %s, D%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_sub_re_8(void)
{
	sprintf(g_dasm_str, "sub.b   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_sub_re_16(void)
{
	sprintf(g_dasm_str, "sub.w   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_sub_re_32(void)
{
	sprintf(g_dasm_str, "sub.l   D%d, %s", (g_cpu_ir>>9)&7, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_suba_16(void)
{
	sprintf(g_dasm_str, "suba.w  %s, A%d", get_ea_mode_str_16(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_suba_32(void)
{
	sprintf(g_dasm_str, "suba.l  %s, A%d", get_ea_mode_str_32(g_cpu_ir), (g_cpu_ir>>9)&7);
}

static void d68000_subi_8(void)
{
	char* str = get_imm_str_s8();
	sprintf(g_dasm_str, "subi.b  %s, %s", str, get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_subi_16(void)
{
	char* str = get_imm_str_s16();
	sprintf(g_dasm_str, "subi.w  %s, %s", str, get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_subi_32(void)
{
	char* str = get_imm_str_s32();
	sprintf(g_dasm_str, "subi.l  %s, %s", str, get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_subq_8(void)
{
	sprintf(g_dasm_str, "subq.b  #%d, %s", g_3bit_qdata_table[(g_cpu_ir>>9)&7], get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_subq_16(void)
{
	sprintf(g_dasm_str, "subq.w  #%d, %s", g_3bit_qdata_table[(g_cpu_ir>>9)&7], get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_subq_32(void)
{
	sprintf(g_dasm_str, "subq.l  #%d, %s", g_3bit_qdata_table[(g_cpu_ir>>9)&7], get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_subx_rr_8(void)
{
	sprintf(g_dasm_str, "subx.b  D%d, D%d", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_subx_rr_16(void)
{
	sprintf(g_dasm_str, "subx.w  D%d, D%d", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_subx_rr_32(void)
{
	sprintf(g_dasm_str, "subx.l  D%d, D%d", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_subx_mm_8(void)
{
	sprintf(g_dasm_str, "subx.b  -(A%d), -(A%d)", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_subx_mm_16(void)
{
	sprintf(g_dasm_str, "subx.w  -(A%d), -(A%d)", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_subx_mm_32(void)
{
	sprintf(g_dasm_str, "subx.l  -(A%d), -(A%d)", g_cpu_ir&7, (g_cpu_ir>>9)&7);
}

static void d68000_swap(void)
{
	sprintf(g_dasm_str, "swap    D%d", g_cpu_ir&7);
}

static void d68000_tas(void)
{
	sprintf(g_dasm_str, "tas     %s", get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_trap(void)
{
	sprintf(g_dasm_str, "trap    #$%x", g_cpu_ir&0xf);
}

static void d68020_trapcc_0(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "trap%-2s; (2+)", g_cc[(g_cpu_ir>>8)&0xf]);
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68020_trapcc_16(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "trap%-2s  %s; (2+)", g_cc[(g_cpu_ir>>8)&0xf], get_imm_str_u16());
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68020_trapcc_32(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "trap%-2s  %s; (2+)", g_cc[(g_cpu_ir>>8)&0xf], get_imm_str_u32());
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68000_trapv(void)
{
	sprintf(g_dasm_str, "trapv");
	SET_OPCODE_FLAGS(DASMFLAG_STEP_OVER);
}

static void d68000_tst_8(void)
{
	sprintf(g_dasm_str, "tst.b   %s", get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_tst_pcdi_8(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.b   %s; (2+)", get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_tst_pcix_8(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.b   %s; (2+)", get_ea_mode_str_8(g_cpu_ir));
}

static void d68020_tst_i_8(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.b   %s; (2+)", get_ea_mode_str_8(g_cpu_ir));
}

static void d68000_tst_16(void)
{
	sprintf(g_dasm_str, "tst.w   %s", get_ea_mode_str_16(g_cpu_ir));
}

static void d68020_tst_a_16(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.w   %s; (2+)", get_ea_mode_str_16(g_cpu_ir));
}

static void d68020_tst_pcdi_16(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.w   %s; (2+)", get_ea_mode_str_16(g_cpu_ir));
}

static void d68020_tst_pcix_16(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.w   %s; (2+)", get_ea_mode_str_16(g_cpu_ir));
}

static void d68020_tst_i_16(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.w   %s; (2+)", get_ea_mode_str_16(g_cpu_ir));
}

static void d68000_tst_32(void)
{
	sprintf(g_dasm_str, "tst.l   %s", get_ea_mode_str_32(g_cpu_ir));
}

static void d68020_tst_a_32(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.l   %s; (2+)", get_ea_mode_str_32(g_cpu_ir));
}

static void d68020_tst_pcdi_32(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.l   %s; (2+)", get_ea_mode_str_32(g_cpu_ir));
}

static void d68020_tst_pcix_32(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.l   %s; (2+)", get_ea_mode_str_32(g_cpu_ir));
}

static void d68020_tst_i_32(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "tst.l   %s; (2+)", get_ea_mode_str_32(g_cpu_ir));
}

static void d68000_unlk(void)
{
	sprintf(g_dasm_str, "unlk    A%d", g_cpu_ir&7);
}

static void d68020_unpk_rr(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "unpk    D%d, D%d, %s; (2+)", g_cpu_ir&7, (g_cpu_ir>>9)&7, get_imm_str_u16());
}

static void d68020_unpk_mm(void)
{
	LIMIT_CPU_TYPES(M68020_PLUS);
	sprintf(g_dasm_str, "unpk    -(A%d), -(A%d), %s; (2+)", g_cpu_ir&7, (g_cpu_ir>>9)&7, get_imm_str_u16());
}



/* ======================================================================== */
/* ======================= INSTRUCTION TABLE BUILDER ====================== */
/* ======================================================================== */

/* EA Masks:
800 = data register direct
400 = address register direct
200 = address register indirect
100 = ARI postincrement
 80 = ARI pre-decrement
 40 = ARI displacement
 20 = ARI index
 10 = absolute short
  8 = absolute long
  4 = immediate / sr
  2 = pc displacement
  1 = pc idx
*/

static opcode_struct g_opcode_info[] =
{
/*  opcode handler    mask    match   ea mask */
	{d68000_1010         , 0xf000, 0xa000, 0x000},
	{d68000_1111         , 0xf000, 0xf000, 0x000},
	{d68000_abcd_rr      , 0xf1f8, 0xc100, 0x000},
	{d68000_abcd_mm      , 0xf1f8, 0xc108, 0x000},
	{d68000_add_er_8     , 0xf1c0, 0xd000, 0xbff},
	{d68000_add_er_16    , 0xf1c0, 0xd040, 0xfff},
	{d68000_add_er_32    , 0xf1c0, 0xd080, 0xfff},
	{d68000_add_re_8     , 0xf1c0, 0xd100, 0x3f8},
	{d68000_add_re_16    , 0xf1c0, 0xd140, 0x3f8},
	{d68000_add_re_32    , 0xf1c0, 0xd180, 0x3f8},
	{d68000_adda_16      , 0xf1c0, 0xd0c0, 0xfff},
	{d68000_adda_32      , 0xf1c0, 0xd1c0, 0xfff},
	{d68000_addi_8       , 0xffc0, 0x0600, 0xbf8},
	{d68000_addi_16      , 0xffc0, 0x0640, 0xbf8},
	{d68000_addi_32      , 0xffc0, 0x0680, 0xbf8},
	{d68000_addq_8       , 0xf1c0, 0x5000, 0xbf8},
	{d68000_addq_16      , 0xf1c0, 0x5040, 0xff8},
	{d68000_addq_32      , 0xf1c0, 0x5080, 0xff8},
	{d68000_addx_rr_8    , 0xf1f8, 0xd100, 0x000},
	{d68000_addx_rr_16   , 0xf1f8, 0xd140, 0x000},
	{d68000_addx_rr_32   , 0xf1f8, 0xd180, 0x000},
	{d68000_addx_mm_8    , 0xf1f8, 0xd108, 0x000},
	{d68000_addx_mm_16   , 0xf1f8, 0xd148, 0x000},
	{d68000_addx_mm_32   , 0xf1f8, 0xd188, 0x000},
	{d68000_and_er_8     , 0xf1c0, 0xc000, 0xbff},
	{d68000_and_er_16    , 0xf1c0, 0xc040, 0xbff},
	{d68000_and_er_32    , 0xf1c0, 0xc080, 0xbff},
	{d68000_and_re_8     , 0xf1c0, 0xc100, 0x3f8},
	{d68000_and_re_16    , 0xf1c0, 0xc140, 0x3f8},
	{d68000_and_re_32    , 0xf1c0, 0xc180, 0x3f8},
	{d68000_andi_to_ccr  , 0xffff, 0x023c, 0x000},
	{d68000_andi_to_sr   , 0xffff, 0x027c, 0x000},
	{d68000_andi_8       , 0xffc0, 0x0200, 0xbf8},
	{d68000_andi_16      , 0xffc0, 0x0240, 0xbf8},
	{d68000_andi_32      , 0xffc0, 0x0280, 0xbf8},
	{d68000_asr_s_8      , 0xf1f8, 0xe000, 0x000},
	{d68000_asr_s_16     , 0xf1f8, 0xe040, 0x000},
	{d68000_asr_s_32     , 0xf1f8, 0xe080, 0x000},
	{d68000_asr_r_8      , 0xf1f8, 0xe020, 0x000},
	{d68000_asr_r_16     , 0xf1f8, 0xe060, 0x000},
	{d68000_asr_r_32     , 0xf1f8, 0xe0a0, 0x000},
	{d68000_asr_ea       , 0xffc0, 0xe0c0, 0x3f8},
	{d68000_asl_s_8      , 0xf1f8, 0xe100, 0x000},
	{d68000_asl_s_16     , 0xf1f8, 0xe140, 0x000},
	{d68000_asl_s_32     , 0xf1f8, 0xe180, 0x000},
	{d68000_asl_r_8      , 0xf1f8, 0xe120, 0x000},
	{d68000_asl_r_16     , 0xf1f8, 0xe160, 0x000},
	{d68000_asl_r_32     , 0xf1f8, 0xe1a0, 0x000},
	{d68000_asl_ea       , 0xffc0, 0xe1c0, 0x3f8},
	{d68000_bcc_8        , 0xf000, 0x6000, 0x000},
	{d68000_bcc_16       , 0xf0ff, 0x6000, 0x000},
	{d68020_bcc_32       , 0xf0ff, 0x60ff, 0x000},
	{d68000_bchg_r       , 0xf1c0, 0x0140, 0xbf8},
	{d68000_bchg_s       , 0xffc0, 0x0840, 0xbf8},
	{d68000_bclr_r       , 0xf1c0, 0x0180, 0xbf8},
	{d68000_bclr_s       , 0xffc0, 0x0880, 0xbf8},
	{d68020_bfchg        , 0xffc0, 0xeac0, 0xa78},
	{d68020_bfclr        , 0xffc0, 0xecc0, 0xa78},
	{d68020_bfexts       , 0xffc0, 0xebc0, 0xa7b},
	{d68020_bfextu       , 0xffc0, 0xe9c0, 0xa7b},
	{d68020_bfffo        , 0xffc0, 0xedc0, 0xa7b},
	{d68020_bfins        , 0xffc0, 0xefc0, 0xa78},
	{d68020_bfset        , 0xffc0, 0xeec0, 0xa78},
	{d68020_bftst        , 0xffc0, 0xe8c0, 0xa7b},
	{d68010_bkpt         , 0xfff8, 0x4848, 0x000},
	{d68000_bra_8        , 0xff00, 0x6000, 0x000},
	{d68000_bra_16       , 0xffff, 0x6000, 0x000},
	{d68020_bra_32       , 0xffff, 0x60ff, 0x000},
	{d68000_bset_r       , 0xf1c0, 0x01c0, 0xbf8},
	{d68000_bset_s       , 0xffc0, 0x08c0, 0xbf8},
	{d68000_bsr_8        , 0xff00, 0x6100, 0x000},
	{d68000_bsr_16       , 0xffff, 0x6100, 0x000},
	{d68020_bsr_32       , 0xffff, 0x61ff, 0x000},
	{d68000_btst_r       , 0xf1c0, 0x0100, 0xbff},
	{d68000_btst_s       , 0xffc0, 0x0800, 0xbfb},
	{d68020_callm        , 0xffc0, 0x06c0, 0x27b},
	{d68020_cas_8        , 0xffc0, 0x0ac0, 0x3f8},
	{d68020_cas_16       , 0xffc0, 0x0cc0, 0x3f8},
	{d68020_cas_32       , 0xffc0, 0x0ec0, 0x3f8},
	{d68020_cas2_16      , 0xffff, 0x0cfc, 0x000},
	{d68020_cas2_32      , 0xffff, 0x0efc, 0x000},
	{d68000_chk_16       , 0xf1c0, 0x4180, 0xbff},
	{d68020_chk_32       , 0xf1c0, 0x4100, 0xbff},
	{d68020_chk2_cmp2_8  , 0xffc0, 0x00c0, 0x27b},
	{d68020_chk2_cmp2_16 , 0xffc0, 0x02c0, 0x27b},
	{d68020_chk2_cmp2_32 , 0xffc0, 0x04c0, 0x27b},
	{d68040_cinv         , 0xff20, 0xf400, 0x000},
	{d68000_clr_8        , 0xffc0, 0x4200, 0xbf8},
	{d68000_clr_16       , 0xffc0, 0x4240, 0xbf8},
	{d68000_clr_32       , 0xffc0, 0x4280, 0xbf8},
	{d68000_cmp_8        , 0xf1c0, 0xb000, 0xbff},
	{d68000_cmp_16       , 0xf1c0, 0xb040, 0xfff},
	{d68000_cmp_32       , 0xf1c0, 0xb080, 0xfff},
	{d68000_cmpa_16      , 0xf1c0, 0xb0c0, 0xfff},
	{d68000_cmpa_32      , 0xf1c0, 0xb1c0, 0xfff},
	{d68000_cmpi_8       , 0xffc0, 0x0c00, 0xbf8},
	{d68020_cmpi_pcdi_8  , 0xffff, 0x0c3a, 0x000},
	{d68020_cmpi_pcix_8  , 0xffff, 0x0c3b, 0x000},
	{d68000_cmpi_16      , 0xffc0, 0x0c40, 0xbf8},
	{d68020_cmpi_pcdi_16 , 0xffff, 0x0c7a, 0x000},
	{d68020_cmpi_pcix_16 , 0xffff, 0x0c7b, 0x000},
	{d68000_cmpi_32      , 0xffc0, 0x0c80, 0xbf8},
	{d68020_cmpi_pcdi_32 , 0xffff, 0x0cba, 0x000},
	{d68020_cmpi_pcix_32 , 0xffff, 0x0cbb, 0x000},
	{d68000_cmpm_8       , 0xf1f8, 0xb108, 0x000},
	{d68000_cmpm_16      , 0xf1f8, 0xb148, 0x000},
	{d68000_cmpm_32      , 0xf1f8, 0xb188, 0x000},
	{d68020_cpbcc_16     , 0xf1c0, 0xf080, 0x000},
	{d68020_cpbcc_32     , 0xf1c0, 0xf0c0, 0x000},
	{d68020_cpdbcc       , 0xf1f8, 0xf048, 0x000},
	{d68020_cpgen        , 0xf1c0, 0xf000, 0x000},
	{d68020_cprestore    , 0xf1c0, 0xf140, 0x37f},
	{d68020_cpsave       , 0xf1c0, 0xf100, 0x2f8},
	{d68020_cpscc        , 0xf1c0, 0xf040, 0xbf8},
	{d68020_cptrapcc_0   , 0xf1ff, 0xf07c, 0x000},
	{d68020_cptrapcc_16  , 0xf1ff, 0xf07a, 0x000},
	{d68020_cptrapcc_32  , 0xf1ff, 0xf07b, 0x000},
	{d68040_cpush        , 0xff20, 0xf420, 0x000},
	{d68000_dbcc         , 0xf0f8, 0x50c8, 0x000},
	{d68000_dbra         , 0xfff8, 0x51c8, 0x000},
	{d68000_divs         , 0xf1c0, 0x81c0, 0xbff},
	{d68000_divu         , 0xf1c0, 0x80c0, 0xbff},
	{d68020_divl         , 0xffc0, 0x4c40, 0xbff},
	{d68000_eor_8        , 0xf1c0, 0xb100, 0xbf8},
	{d68000_eor_16       , 0xf1c0, 0xb140, 0xbf8},
	{d68000_eor_32       , 0xf1c0, 0xb180, 0xbf8},
	{d68000_eori_to_ccr  , 0xffff, 0x0a3c, 0x000},
	{d68000_eori_to_sr   , 0xffff, 0x0a7c, 0x000},
	{d68000_eori_8       , 0xffc0, 0x0a00, 0xbf8},
	{d68000_eori_16      , 0xffc0, 0x0a40, 0xbf8},
	{d68000_eori_32      , 0xffc0, 0x0a80, 0xbf8},
	{d68000_exg_dd       , 0xf1f8, 0xc140, 0x000},
	{d68000_exg_aa       , 0xf1f8, 0xc148, 0x000},
	{d68000_exg_da       , 0xf1f8, 0xc188, 0x000},
	{d68020_extb_32      , 0xfff8, 0x49c0, 0x000},
	{d68000_ext_16       , 0xfff8, 0x4880, 0x000},
	{d68000_ext_32       , 0xfff8, 0x48c0, 0x000},
	{d68000_illegal      , 0xffff, 0x4afc, 0x000},
	{d68000_jmp          , 0xffc0, 0x4ec0, 0x27b},
	{d68000_jsr          , 0xffc0, 0x4e80, 0x27b},
	{d68000_lea          , 0xf1c0, 0x41c0, 0x27b},
	{d68000_link_16      , 0xfff8, 0x4e50, 0x000},
	{d68020_link_32      , 0xfff8, 0x4808, 0x000},
	{d68000_lsr_s_8      , 0xf1f8, 0xe008, 0x000},
	{d68000_lsr_s_16     , 0xf1f8, 0xe048, 0x000},
	{d68000_lsr_s_32     , 0xf1f8, 0xe088, 0x000},
	{d68000_lsr_r_8      , 0xf1f8, 0xe028, 0x000},
	{d68000_lsr_r_16     , 0xf1f8, 0xe068, 0x000},
	{d68000_lsr_r_32     , 0xf1f8, 0xe0a8, 0x000},
	{d68000_lsr_ea       , 0xffc0, 0xe2c0, 0x3f8},
	{d68000_lsl_s_8      , 0xf1f8, 0xe108, 0x000},
	{d68000_lsl_s_16     , 0xf1f8, 0xe148, 0x000},
	{d68000_lsl_s_32     , 0xf1f8, 0xe188, 0x000},
	{d68000_lsl_r_8      , 0xf1f8, 0xe128, 0x000},
	{d68000_lsl_r_16     , 0xf1f8, 0xe168, 0x000},
	{d68000_lsl_r_32     , 0xf1f8, 0xe1a8, 0x000},
	{d68000_lsl_ea       , 0xffc0, 0xe3c0, 0x3f8},
	{d68000_move_8       , 0xf000, 0x1000, 0xbff},
	{d68000_move_16      , 0xf000, 0x3000, 0xfff},
	{d68000_move_32      , 0xf000, 0x2000, 0xfff},
	{d68000_movea_16     , 0xf1c0, 0x3040, 0xfff},
	{d68000_movea_32     , 0xf1c0, 0x2040, 0xfff},
	{d68000_move_to_ccr  , 0xffc0, 0x44c0, 0xbff},
	{d68010_move_fr_ccr  , 0xffc0, 0x42c0, 0xbf8},
	{d68000_move_to_sr   , 0xffc0, 0x46c0, 0xbff},
	{d68000_move_fr_sr   , 0xffc0, 0x40c0, 0xbf8},
	{d68000_move_to_usp  , 0xfff8, 0x4e60, 0x000},
	{d68000_move_fr_usp  , 0xfff8, 0x4e68, 0x000},
	{d68010_movec        , 0xfffe, 0x4e7a, 0x000},
	{d68000_movem_pd_16  , 0xfff8, 0x48a0, 0x000},
	{d68000_movem_pd_32  , 0xfff8, 0x48e0, 0x000},
	{d68000_movem_re_16  , 0xffc0, 0x4880, 0x2f8},
	{d68000_movem_re_32  , 0xffc0, 0x48c0, 0x2f8},
	{d68000_movem_er_16  , 0xffc0, 0x4c80, 0x37b},
	{d68000_movem_er_32  , 0xffc0, 0x4cc0, 0x37b},
	{d68000_movep_er_16  , 0xf1f8, 0x0108, 0x000},
	{d68000_movep_er_32  , 0xf1f8, 0x0148, 0x000},
	{d68000_movep_re_16  , 0xf1f8, 0x0188, 0x000},
	{d68000_movep_re_32  , 0xf1f8, 0x01c8, 0x000},
	{d68010_moves_8      , 0xffc0, 0x0e00, 0x3f8},
	{d68010_moves_16     , 0xffc0, 0x0e40, 0x3f8},
	{d68010_moves_32     , 0xffc0, 0x0e80, 0x3f8},
	{d68000_moveq        , 0xf100, 0x7000, 0x000},
	{d68040_move16_pi_pi , 0xfff8, 0xf620, 0x000},
	{d68040_move16_pi_al , 0xfff8, 0xf600, 0x000},
	{d68040_move16_al_pi , 0xfff8, 0xf608, 0x000},
	{d68040_move16_ai_al , 0xfff8, 0xf610, 0x000},
	{d68040_move16_al_ai , 0xfff8, 0xf618, 0x000},
	{d68000_muls         , 0xf1c0, 0xc1c0, 0xbff},
	{d68000_mulu         , 0xf1c0, 0xc0c0, 0xbff},
	{d68020_mull         , 0xffc0, 0x4c00, 0xbff},
	{d68000_nbcd         , 0xffc0, 0x4800, 0xbf8},
	{d68000_neg_8        , 0xffc0, 0x4400, 0xbf8},
	{d68000_neg_16       , 0xffc0, 0x4440, 0xbf8},
	{d68000_neg_32       , 0xffc0, 0x4480, 0xbf8},
	{d68000_negx_8       , 0xffc0, 0x4000, 0xbf8},
	{d68000_negx_16      , 0xffc0, 0x4040, 0xbf8},
	{d68000_negx_32      , 0xffc0, 0x4080, 0xbf8},
	{d68000_nop          , 0xffff, 0x4e71, 0x000},
	{d68000_not_8        , 0xffc0, 0x4600, 0xbf8},
	{d68000_not_16       , 0xffc0, 0x4640, 0xbf8},
	{d68000_not_32       , 0xffc0, 0x4680, 0xbf8},
	{d68000_or_er_8      , 0xf1c0, 0x8000, 0xbff},
	{d68000_or_er_16     , 0xf1c0, 0x8040, 0xbff},
	{d68000_or_er_32     , 0xf1c0, 0x8080, 0xbff},
	{d68000_or_re_8      , 0xf1c0, 0x8100, 0x3f8},
	{d68000_or_re_16     , 0xf1c0, 0x8140, 0x3f8},
	{d68000_or_re_32     , 0xf1c0, 0x8180, 0x3f8},
	{d68000_ori_to_ccr   , 0xffff, 0x003c, 0x000},
	{d68000_ori_to_sr    , 0xffff, 0x007c, 0x000},
	{d68000_ori_8        , 0xffc0, 0x0000, 0xbf8},
	{d68000_ori_16       , 0xffc0, 0x0040, 0xbf8},
	{d68000_ori_32       , 0xffc0, 0x0080, 0xbf8},
	{d68020_pack_rr      , 0xf1f8, 0x8140, 0x000},
	{d68020_pack_mm      , 0xf1f8, 0x8148, 0x000},
	{d68000_pea          , 0xffc0, 0x4840, 0x27b},
	{d68000_reset        , 0xffff, 0x4e70, 0x000},
	{d68000_ror_s_8      , 0xf1f8, 0xe018, 0x000},
	{d68000_ror_s_16     , 0xf1f8, 0xe058, 0x000},
	{d68000_ror_s_32     , 0xf1f8, 0xe098, 0x000},
	{d68000_ror_r_8      , 0xf1f8, 0xe038, 0x000},
	{d68000_ror_r_16     , 0xf1f8, 0xe078, 0x000},
	{d68000_ror_r_32     , 0xf1f8, 0xe0b8, 0x000},
	{d68000_ror_ea       , 0xffc0, 0xe6c0, 0x3f8},
	{d68000_rol_s_8      , 0xf1f8, 0xe118, 0x000},
	{d68000_rol_s_16     , 0xf1f8, 0xe158, 0x000},
	{d68000_rol_s_32     , 0xf1f8, 0xe198, 0x000},
	{d68000_rol_r_8      , 0xf1f8, 0xe138, 0x000},
	{d68000_rol_r_16     , 0xf1f8, 0xe178, 0x000},
	{d68000_rol_r_32     , 0xf1f8, 0xe1b8, 0x000},
	{d68000_rol_ea       , 0xffc0, 0xe7c0, 0x3f8},
	{d68000_roxr_s_8     , 0xf1f8, 0xe010, 0x000},
	{d68000_roxr_s_16    , 0xf1f8, 0xe050, 0x000},
	{d68000_roxr_s_32    , 0xf1f8, 0xe090, 0x000},
	{d68000_roxr_r_8     , 0xf1f8, 0xe030, 0x000},
	{d68000_roxr_r_16    , 0xf1f8, 0xe070, 0x000},
	{d68000_roxr_r_32    , 0xf1f8, 0xe0b0, 0x000},
	{d68000_roxr_ea      , 0xffc0, 0xe4c0, 0x3f8},
	{d68000_roxl_s_8     , 0xf1f8, 0xe110, 0x000},
	{d68000_roxl_s_16    , 0xf1f8, 0xe150, 0x000},
	{d68000_roxl_s_32    , 0xf1f8, 0xe190, 0x000},
	{d68000_roxl_r_8     , 0xf1f8, 0xe130, 0x000},
	{d68000_roxl_r_16    , 0xf1f8, 0xe170, 0x000},
	{d68000_roxl_r_32    , 0xf1f8, 0xe1b0, 0x000},
	{d68000_roxl_ea      , 0xffc0, 0xe5c0, 0x3f8},
	{d68010_rtd          , 0xffff, 0x4e74, 0x000},
	{d68000_rte          , 0xffff, 0x4e73, 0x000},
	{d68020_rtm          , 0xfff0, 0x06c0, 0x000},
	{d68000_rtr          , 0xffff, 0x4e77, 0x000},
	{d68000_rts          , 0xffff, 0x4e75, 0x000},
	{d68000_sbcd_rr      , 0xf1f8, 0x8100, 0x000},
	{d68000_sbcd_mm      , 0xf1f8, 0x8108, 0x000},
	{d68000_scc          , 0xf0c0, 0x50c0, 0xbf8},
	{d68000_stop         , 0xffff, 0x4e72, 0x000},
	{d68000_sub_er_8     , 0xf1c0, 0x9000, 0xbff},
	{d68000_sub_er_16    , 0xf1c0, 0x9040, 0xfff},
	{d68000_sub_er_32    , 0xf1c0, 0x9080, 0xfff},
	{d68000_sub_re_8     , 0xf1c0, 0x9100, 0x3f8},
	{d68000_sub_re_16    , 0xf1c0, 0x9140, 0x3f8},
	{d68000_sub_re_32    , 0xf1c0, 0x9180, 0x3f8},
	{d68000_suba_16      , 0xf1c0, 0x90c0, 0xfff},
	{d68000_suba_32      , 0xf1c0, 0x91c0, 0xfff},
	{d68000_subi_8       , 0xffc0, 0x0400, 0xbf8},
	{d68000_subi_16      , 0xffc0, 0x0440, 0xbf8},
	{d68000_subi_32      , 0xffc0, 0x0480, 0xbf8},
	{d68000_subq_8       , 0xf1c0, 0x5100, 0xbf8},
	{d68000_subq_16      , 0xf1c0, 0x5140, 0xff8},
	{d68000_subq_32      , 0xf1c0, 0x5180, 0xff8},
	{d68000_subx_rr_8    , 0xf1f8, 0x9100, 0x000},
	{d68000_subx_rr_16   , 0xf1f8, 0x9140, 0x000},
	{d68000_subx_rr_32   , 0xf1f8, 0x9180, 0x000},
	{d68000_subx_mm_8    , 0xf1f8, 0x9108, 0x000},
	{d68000_subx_mm_16   , 0xf1f8, 0x9148, 0x000},
	{d68000_subx_mm_32   , 0xf1f8, 0x9188, 0x000},
	{d68000_swap         , 0xfff8, 0x4840, 0x000},
	{d68000_tas          , 0xffc0, 0x4ac0, 0xbf8},
	{d68000_trap         , 0xfff0, 0x4e40, 0x000},
	{d68020_trapcc_0     , 0xf0ff, 0x50fc, 0x000},
	{d68020_trapcc_16    , 0xf0ff, 0x50fa, 0x000},
	{d68020_trapcc_32    , 0xf0ff, 0x50fb, 0x000},
	{d68000_trapv        , 0xffff, 0x4e76, 0x000},
	{d68000_tst_8        , 0xffc0, 0x4a00, 0xbf8},
	{d68020_tst_pcdi_8   , 0xffff, 0x4a3a, 0x000},
	{d68020_tst_pcix_8   , 0xffff, 0x4a3b, 0x000},
	{d68020_tst_i_8      , 0xffff, 0x4a3c, 0x000},
	{d68000_tst_16       , 0xffc0, 0x4a40, 0xbf8},
	{d68020_tst_a_16     , 0xfff8, 0x4a48, 0x000},
	{d68020_tst_pcdi_16  , 0xffff, 0x4a7a, 0x000},
	{d68020_tst_pcix_16  , 0xffff, 0x4a7b, 0x000},
	{d68020_tst_i_16     , 0xffff, 0x4a7c, 0x000},
	{d68000_tst_32       , 0xffc0, 0x4a80, 0xbf8},
	{d68020_tst_a_32     , 0xfff8, 0x4a88, 0x000},
	{d68020_tst_pcdi_32  , 0xffff, 0x4aba, 0x000},
	{d68020_tst_pcix_32  , 0xffff, 0x4abb, 0x000},
	{d68020_tst_i_32     , 0xffff, 0x4abc, 0x000},
	{d68000_unlk         , 0xfff8, 0x4e58, 0x000},
	{d68020_unpk_rr      , 0xf1f8, 0x8180, 0x000},
	{d68020_unpk_mm      , 0xf1f8, 0x8188, 0x000},
	{0, 0, 0, 0}
};

/* Check if opcode is using a valid ea mode */
static int valid_ea(uint opcode, uint mask)
{
	if(mask == 0)
		return 1;

	switch(opcode & 0x3f)
	{
		case 0x00: case 0x01: case 0x02: case 0x03:
		case 0x04: case 0x05: case 0x06: case 0x07:
			return (mask & 0x800) != 0;
		case 0x08: case 0x09: case 0x0a: case 0x0b:
		case 0x0c: case 0x0d: case 0x0e: case 0x0f:
			return (mask & 0x400) != 0;
		case 0x10: case 0x11: case 0x12: case 0x13:
		case 0x14: case 0x15: case 0x16: case 0x17:
			return (mask & 0x200) != 0;
		case 0x18: case 0x19: case 0x1a: case 0x1b:
		case 0x1c: case 0x1d: case 0x1e: case 0x1f:
			return (mask & 0x100) != 0;
		case 0x20: case 0x21: case 0x22: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
			return (mask & 0x080) != 0;
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
			return (mask & 0x040) != 0;
		case 0x30: case 0x31: case 0x32: case 0x33:
		case 0x34: case 0x35: case 0x36: case 0x37:
			return (mask & 0x020) != 0;
		case 0x38:
			return (mask & 0x010) != 0;
		case 0x39:
			return (mask & 0x008) != 0;
		case 0x3a:
			return (mask & 0x002) != 0;
		case 0x3b:
			return (mask & 0x001) != 0;
		case 0x3c:
			return (mask & 0x004) != 0;
	}
	return 0;

}

/* Used by qsort */
static int DECL_SPEC compare_nof_true_bits(const void *aptr, const void *bptr)
{
	uint a = ((const opcode_struct*)aptr)->mask;
	uint b = ((const opcode_struct*)bptr)->mask;

	a = ((a & 0xAAAA) >> 1) + (a & 0x5555);
	a = ((a & 0xCCCC) >> 2) + (a & 0x3333);
	a = ((a & 0xF0F0) >> 4) + (a & 0x0F0F);
	a = ((a & 0xFF00) >> 8) + (a & 0x00FF);

	b = ((b & 0xAAAA) >> 1) + (b & 0x5555);
	b = ((b & 0xCCCC) >> 2) + (b & 0x3333);
	b = ((b & 0xF0F0) >> 4) + (b & 0x0F0F);
	b = ((b & 0xFF00) >> 8) + (b & 0x00FF);

	return b - a; /* reversed to get greatest to least sorting */
}

/* build the opcode handler jump table */
static void build_opcode_table(void)
{
	uint i;
	uint opcode;
	opcode_struct* ostruct;
	uint opcode_info_length = 0;

	for(ostruct = g_opcode_info;ostruct->opcode_handler != 0;ostruct++)
		opcode_info_length++;

	qsort((void *)g_opcode_info, opcode_info_length, sizeof(g_opcode_info[0]), compare_nof_true_bits);

	for(i=0;i<0x10000;i++)
	{
		g_instruction_table[i] = d68000_illegal; /* default to illegal */
		opcode = i;
		/* search through opcode info for a match */
		for(ostruct = g_opcode_info;ostruct->opcode_handler != 0;ostruct++)
		{
			/* match opcode mask and allowed ea modes */
			if((opcode & ostruct->mask) == ostruct->match)
			{
				/* Handle destination ea for move instructions */
				if((ostruct->opcode_handler == d68000_move_8 ||
					 ostruct->opcode_handler == d68000_move_16 ||
					 ostruct->opcode_handler == d68000_move_32) &&
					 !valid_ea(((opcode>>9)&7) | ((opcode>>3)&0x38), 0xbf8))
						continue;
				if(valid_ea(opcode, ostruct->ea_mask))
				{
					g_instruction_table[i] = ostruct->opcode_handler;
					break;
				}
			}
		}
	}
}



/* ======================================================================== */
/* ================================= API ================================== */
/* ======================================================================== */

/* Disasemble one instruction at pc and store in str_buff */
unsigned int m68k_disassemble(char* str_buff, unsigned int pc, unsigned int cpu_type)
{
	if(!g_initialized)
	{
		build_opcode_table();
		g_initialized = 1;
	}
	switch(cpu_type)
	{
		case M68K_CPU_TYPE_68000:
			g_cpu_type = TYPE_68000;
			g_address_mask = 0x00ffffff;
			break;
		case M68K_CPU_TYPE_68008:
			g_cpu_type = TYPE_68008;
			g_address_mask = 0x003fffff;
			break;
		case M68K_CPU_TYPE_68010:
			g_cpu_type = TYPE_68010;
			g_address_mask = 0x00ffffff;
			break;
		case M68K_CPU_TYPE_68EC020:
			g_cpu_type = TYPE_68020;
			g_address_mask = 0x00ffffff;
			break;
		case M68K_CPU_TYPE_68020:
			g_cpu_type = TYPE_68020;
			g_address_mask = 0xffffffff;
			break;
		case M68K_CPU_TYPE_68030:
			g_cpu_type = TYPE_68030;
			g_address_mask = 0xffffffff;
			break;
		case M68K_CPU_TYPE_68040:
			g_cpu_type = TYPE_68040;
			g_address_mask = 0xffffffff;
			break;
		default:
			return 0;
	}

	g_cpu_pc = pc;
	g_helper_str[0] = 0;
	g_cpu_ir = read_imm_16();
	g_opcode_type = 0;
	g_instruction_table[g_cpu_ir]();
	sprintf(str_buff, "%s%s", g_dasm_str, g_helper_str);
	return COMBINE_OPCODE_FLAGS(g_cpu_pc - pc);
}

char* m68ki_disassemble_quick(unsigned int pc, unsigned int cpu_type)
{
	static char buff[100];
	buff[0] = 0;
	m68k_disassemble(buff, pc, cpu_type);
	return buff;
}

/* Check if the instruction is a valid one */
unsigned int m68k_is_valid_instruction(unsigned int instruction, unsigned int cpu_type)
{
	if(!g_initialized)
	{
		build_opcode_table();
		g_initialized = 1;
	}

	instruction &= 0xffff;
	if(g_instruction_table[instruction] == d68000_illegal)
		return 0;

	switch(cpu_type)
	{
		case M68K_CPU_TYPE_68000:
		case M68K_CPU_TYPE_68008:
			if(g_instruction_table[instruction] == d68010_bkpt)
				return 0;
			if(g_instruction_table[instruction] == d68010_move_fr_ccr)
				return 0;
			if(g_instruction_table[instruction] == d68010_movec)
				return 0;
			if(g_instruction_table[instruction] == d68010_moves_8)
				return 0;
			if(g_instruction_table[instruction] == d68010_moves_16)
				return 0;
			if(g_instruction_table[instruction] == d68010_moves_32)
				return 0;
			if(g_instruction_table[instruction] == d68010_rtd)
				return 0;
		case M68K_CPU_TYPE_68010:
			if(g_instruction_table[instruction] == d68020_bcc_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_bfchg)
				return 0;
			if(g_instruction_table[instruction] == d68020_bfclr)
				return 0;
			if(g_instruction_table[instruction] == d68020_bfexts)
				return 0;
			if(g_instruction_table[instruction] == d68020_bfextu)
				return 0;
			if(g_instruction_table[instruction] == d68020_bfffo)
				return 0;
			if(g_instruction_table[instruction] == d68020_bfins)
				return 0;
			if(g_instruction_table[instruction] == d68020_bfset)
				return 0;
			if(g_instruction_table[instruction] == d68020_bftst)
				return 0;
			if(g_instruction_table[instruction] == d68020_bra_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_bsr_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_callm)
				return 0;
			if(g_instruction_table[instruction] == d68020_cas_8)
				return 0;
			if(g_instruction_table[instruction] == d68020_cas_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_cas_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_cas2_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_cas2_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_chk_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_chk2_cmp2_8)
				return 0;
			if(g_instruction_table[instruction] == d68020_chk2_cmp2_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_chk2_cmp2_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_cmpi_pcdi_8)
				return 0;
			if(g_instruction_table[instruction] == d68020_cmpi_pcix_8)
				return 0;
			if(g_instruction_table[instruction] == d68020_cmpi_pcdi_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_cmpi_pcix_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_cmpi_pcdi_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_cmpi_pcix_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_cpbcc_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_cpbcc_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_cpdbcc)
				return 0;
			if(g_instruction_table[instruction] == d68020_cpgen)
				return 0;
			if(g_instruction_table[instruction] == d68020_cprestore)
				return 0;
			if(g_instruction_table[instruction] == d68020_cpsave)
				return 0;
			if(g_instruction_table[instruction] == d68020_cpscc)
				return 0;
			if(g_instruction_table[instruction] == d68020_cptrapcc_0)
				return 0;
			if(g_instruction_table[instruction] == d68020_cptrapcc_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_cptrapcc_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_divl)
				return 0;
			if(g_instruction_table[instruction] == d68020_extb_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_link_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_mull)
				return 0;
			if(g_instruction_table[instruction] == d68020_pack_rr)
				return 0;
			if(g_instruction_table[instruction] == d68020_pack_mm)
				return 0;
			if(g_instruction_table[instruction] == d68020_rtm)
				return 0;
			if(g_instruction_table[instruction] == d68020_trapcc_0)
				return 0;
			if(g_instruction_table[instruction] == d68020_trapcc_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_trapcc_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_pcdi_8)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_pcix_8)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_i_8)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_a_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_pcdi_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_pcix_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_i_16)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_a_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_pcdi_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_pcix_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_tst_i_32)
				return 0;
			if(g_instruction_table[instruction] == d68020_unpk_rr)
				return 0;
			if(g_instruction_table[instruction] == d68020_unpk_mm)
				return 0;
		case M68K_CPU_TYPE_68EC020:
		case M68K_CPU_TYPE_68020:
		case M68K_CPU_TYPE_68030:
			if(g_instruction_table[instruction] == d68040_cinv)
				return 0;
			if(g_instruction_table[instruction] == d68040_cpush)
				return 0;
			if(g_instruction_table[instruction] == d68040_move16_pi_pi)
				return 0;
			if(g_instruction_table[instruction] == d68040_move16_pi_al)
				return 0;
			if(g_instruction_table[instruction] == d68040_move16_al_pi)
				return 0;
			if(g_instruction_table[instruction] == d68040_move16_ai_al)
				return 0;
			if(g_instruction_table[instruction] == d68040_move16_al_ai)
				return 0;
	}
	if(cpu_type != M68K_CPU_TYPE_68020 && cpu_type != M68K_CPU_TYPE_68EC020 &&
	  (g_instruction_table[instruction] == d68020_callm ||
	  g_instruction_table[instruction] == d68020_rtm))
		return 0;

	return 1;
}



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */
