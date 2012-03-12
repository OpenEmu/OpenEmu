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

/*
 * Modified For OpenVMS By:  Robert Alan Byer
 *                           byer@mail.ourservers.net
 */


/* ======================================================================== */
/* ============================ CODE GENERATOR ============================ */
/* ======================================================================== */
/*
 * This is the code generator program which will generate the opcode table
 * and the final opcode handlers.
 *
 * It requires an input file to function (default m68k_in.c), but you can
 * specify your own like so:
 *
 * m68kmake <output path> <input file>
 *
 * where output path is the path where the output files should be placed, and
 * input file is the file to use for input.
 *
 * If you modify the input file greatly from its released form, you may have
 * to tweak the configuration section a bit since I'm using static allocation
 * to keep things simple.
 *
 *
 * TODO: - build a better code generator for the move instruction.
 *       - Add callm and rtm instructions
 *       - Fix RTE to handle other format words
 *       - Add address error (and bus error?) handling
 */


const char* g_version = "3.3";

/* ======================================================================== */
/* =============================== INCLUDES =============================== */
/* ======================================================================== */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>



/* ======================================================================== */
/* ============================= CONFIGURATION ============================ */
/* ======================================================================== */

#define M68K_MAX_PATH 1024
#define M68K_MAX_DIR  1024

#define MAX_LINE_LENGTH                 200	/* length of 1 line */
#define MAX_BODY_LENGTH                 300	/* Number of lines in 1 function */
#define MAX_REPLACE_LENGTH               30	/* Max number of replace strings */
#define MAX_INSERT_LENGTH              5000	/* Max size of insert piece */
#define MAX_NAME_LENGTH                  30	/* Max length of ophandler name */
#define MAX_SPEC_PROC_LENGTH              4	/* Max length of special processing str */
#define MAX_SPEC_EA_LENGTH                5	/* Max length of specified EA str */
#define EA_ALLOWED_LENGTH                11	/* Max length of ea allowed str */
#define MAX_OPCODE_INPUT_TABLE_LENGTH  1000	/* Max length of opcode handler tbl */
#define MAX_OPCODE_OUTPUT_TABLE_LENGTH 3000	/* Max length of opcode handler tbl */

/* Default filenames */
#define FILENAME_INPUT      "m68k_in.c"
#define FILENAME_PROTOTYPE  "m68kops.h"
#define FILENAME_TABLE      "m68kops.c"
#define FILENAME_OPS_AC     "m68kopac.c"
#define FILENAME_OPS_DM     "m68kopdm.c"
#define FILENAME_OPS_NZ     "m68kopnz.c"


/* Identifier sequences recognized by this program */

#define ID_INPUT_SEPARATOR "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"

#define ID_BASE                 "M68KMAKE"
#define ID_PROTOTYPE_HEADER     ID_BASE "_PROTOTYPE_HEADER"
#define ID_PROTOTYPE_FOOTER     ID_BASE "_PROTOTYPE_FOOTER"
#define ID_TABLE_HEADER         ID_BASE "_TABLE_HEADER"
#define ID_TABLE_FOOTER         ID_BASE "_TABLE_FOOTER"
#define ID_TABLE_BODY           ID_BASE "_TABLE_BODY"
#define ID_TABLE_START          ID_BASE "_TABLE_START"
#define ID_OPHANDLER_HEADER     ID_BASE "_OPCODE_HANDLER_HEADER"
#define ID_OPHANDLER_FOOTER     ID_BASE "_OPCODE_HANDLER_FOOTER"
#define ID_OPHANDLER_BODY       ID_BASE "_OPCODE_HANDLER_BODY"
#define ID_END                  ID_BASE "_END"

#define ID_OPHANDLER_NAME       ID_BASE "_OP"
#define ID_OPHANDLER_EA_AY_8    ID_BASE "_GET_EA_AY_8"
#define ID_OPHANDLER_EA_AY_16   ID_BASE "_GET_EA_AY_16"
#define ID_OPHANDLER_EA_AY_32   ID_BASE "_GET_EA_AY_32"
#define ID_OPHANDLER_OPER_AY_8  ID_BASE "_GET_OPER_AY_8"
#define ID_OPHANDLER_OPER_AY_16 ID_BASE "_GET_OPER_AY_16"
#define ID_OPHANDLER_OPER_AY_32 ID_BASE "_GET_OPER_AY_32"
#define ID_OPHANDLER_CC         ID_BASE "_CC"
#define ID_OPHANDLER_NOT_CC     ID_BASE "_NOT_CC"


#ifndef DECL_SPEC
#define DECL_SPEC
#endif /* DECL_SPEC */



/* ======================================================================== */
/* ============================== PROTOTYPES ============================== */
/* ======================================================================== */

enum {
	CPU_TYPE_000 = 0,
	CPU_TYPE_010,
	CPU_TYPE_020,
	NUM_CPUS
};

#define UNSPECIFIED "."
#define UNSPECIFIED_CH '.'

#define HAS_NO_EA_MODE(A) (strcmp(A, "..........") == 0)
#define HAS_EA_AI(A)   ((A)[0] == 'A')
#define HAS_EA_PI(A)   ((A)[1] == '+')
#define HAS_EA_PD(A)   ((A)[2] == '-')
#define HAS_EA_DI(A)   ((A)[3] == 'D')
#define HAS_EA_IX(A)   ((A)[4] == 'X')
#define HAS_EA_AW(A)   ((A)[5] == 'W')
#define HAS_EA_AL(A)   ((A)[6] == 'L')
#define HAS_EA_PCDI(A) ((A)[7] == 'd')
#define HAS_EA_PCIX(A) ((A)[8] == 'x')
#define HAS_EA_I(A)    ((A)[9] == 'I')

enum
{
	EA_MODE_NONE,	/* No special addressing mode */
	EA_MODE_AI,		/* Address register indirect */
	EA_MODE_PI,		/* Address register indirect with postincrement */
	EA_MODE_PI7,	/* Address register 7 indirect with postincrement */
	EA_MODE_PD,		/* Address register indirect with predecrement */
	EA_MODE_PD7,	/* Address register 7 indirect with predecrement */
	EA_MODE_DI,		/* Address register indirect with displacement */
	EA_MODE_IX,		/* Address register indirect with index */
	EA_MODE_AW,		/* Absolute word */
	EA_MODE_AL,		/* Absolute long */
	EA_MODE_PCDI,	/* Program counter indirect with displacement */
	EA_MODE_PCIX,	/* Program counter indirect with index */
	EA_MODE_I		/* Immediate */
};


/* Everything we need to know about an opcode */
typedef struct
{
	char name[MAX_NAME_LENGTH];           /* opcode handler name */
	unsigned char size;                   /* Size of operation */
	char spec_proc[MAX_SPEC_PROC_LENGTH]; /* Special processing mode */
	char spec_ea[MAX_SPEC_EA_LENGTH];     /* Specified effective addressing mode */
	unsigned char bits;                   /* Number of significant bits (used for sorting the table) */
	unsigned short op_mask;               /* Mask to apply for matching an opcode to a handler */
	unsigned short op_match;              /* Value to match after masking */
	char ea_allowed[EA_ALLOWED_LENGTH];   /* Effective addressing modes allowed */
	char cpu_mode[NUM_CPUS];              /* User or supervisor mode */
	char cpus[NUM_CPUS+1];                /* Allowed CPUs */
	unsigned char cycles[NUM_CPUS];       /* cycles for 000, 010, 020 */
} opcode_struct;


/* All modifications necessary for a specific EA mode of an instruction */
typedef struct
{
	const char* fname_add;
	const char* ea_add;
	unsigned int mask_add;
	unsigned int match_add;
} ea_info_struct;


/* Holds the body of a function */
typedef struct
{
	char body[MAX_BODY_LENGTH][MAX_LINE_LENGTH+1];
	int length;
} body_struct;


/* Holds a sequence of search / replace strings */
typedef struct
{
	char replace[MAX_REPLACE_LENGTH][2][MAX_LINE_LENGTH+1];
	int length;
} replace_struct;


/* Function Prototypes */
void error_exit(const char* fmt, ...);
void perror_exit(const char* fmt, ...);
int check_strsncpy(char* dst, char* src, int maxlength);
int check_atoi(char* str, int *result);
int skip_spaces(char* str);
int num_bits(int value);
int atoh(char* buff);
int fgetline(char* buff, int nchars, FILE* file);
int get_oper_cycles(opcode_struct* op, int ea_mode, int cpu_type);
opcode_struct* find_opcode(char* name, int size, char* spec_proc, char* spec_ea);
opcode_struct* find_illegal_opcode(void);
int extract_opcode_info(char* src, char* name, int* size, char* spec_proc, char* spec_ea);
void add_replace_string(replace_struct* replace, const char* search_str, const char* replace_str);
void write_body(FILE* filep, body_struct* body, replace_struct* replace);
void get_base_name(char* base_name, opcode_struct* op);
void write_prototype(FILE* filep, char* base_name);
void write_function_name(FILE* filep, char* base_name);
void add_opcode_output_table_entry(opcode_struct* op, char* name);
static int DECL_SPEC compare_nof_true_bits(const void* aptr, const void* bptr);
void print_opcode_output_table(FILE* filep);
void write_table_entry(FILE* filep, opcode_struct* op);
void set_opcode_struct(opcode_struct* src, opcode_struct* dst, int ea_mode);
void generate_opcode_handler(FILE* filep, body_struct* body, replace_struct* replace, opcode_struct* opinfo, int ea_mode);
void generate_opcode_ea_variants(FILE* filep, body_struct* body, replace_struct* replace, opcode_struct* op);
void generate_opcode_cc_variants(FILE* filep, body_struct* body, replace_struct* replace, opcode_struct* op_in, int offset);
void process_opcode_handlers(void);
void populate_table(void);
void read_insert(char* insert);



/* ======================================================================== */
/* ================================= DATA ================================= */
/* ======================================================================== */

/* Name of the input file */
char g_input_filename[M68K_MAX_PATH] = FILENAME_INPUT;

/* File handles */
FILE* g_input_file = NULL;
FILE* g_prototype_file = NULL;
FILE* g_table_file = NULL;
FILE* g_ops_ac_file = NULL;
FILE* g_ops_dm_file = NULL;
FILE* g_ops_nz_file = NULL;

int g_num_functions = 0;  /* Number of functions processed */
int g_num_primitives = 0; /* Number of function primitives read */
int g_line_number = 1;    /* Current line number */

/* Opcode handler table */
opcode_struct g_opcode_input_table[MAX_OPCODE_INPUT_TABLE_LENGTH];

opcode_struct g_opcode_output_table[MAX_OPCODE_OUTPUT_TABLE_LENGTH];
int g_opcode_output_table_length = 0;

ea_info_struct g_ea_info_table[13] =
{/* fname    ea        mask  match */
	{"",     "",       0x00, 0x00}, /* EA_MODE_NONE */
	{"ai",   "AY_AI",  0x38, 0x10}, /* EA_MODE_AI   */
	{"pi",   "AY_PI",  0x38, 0x18}, /* EA_MODE_PI   */
	{"pi7",  "A7_PI",  0x3f, 0x1f}, /* EA_MODE_PI7  */
	{"pd",   "AY_PD",  0x38, 0x20}, /* EA_MODE_PD   */
	{"pd7",  "A7_PD",  0x3f, 0x27}, /* EA_MODE_PD7  */
	{"di",   "AY_DI",  0x38, 0x28}, /* EA_MODE_DI   */
	{"ix",   "AY_IX",  0x38, 0x30}, /* EA_MODE_IX   */
	{"aw",   "AW",     0x3f, 0x38}, /* EA_MODE_AW   */
	{"al",   "AL",     0x3f, 0x39}, /* EA_MODE_AL   */
	{"pcdi", "PCDI",   0x3f, 0x3a}, /* EA_MODE_PCDI */
	{"pcix", "PCIX",   0x3f, 0x3b}, /* EA_MODE_PCIX */
	{"i",    "I",      0x3f, 0x3c}, /* EA_MODE_I    */
};


const char* g_cc_table[16][2] =
{
	{ "t",  "T"}, /* 0000 */
	{ "f",  "F"}, /* 0001 */
	{"hi", "HI"}, /* 0010 */
	{"ls", "LS"}, /* 0011 */
	{"cc", "CC"}, /* 0100 */
	{"cs", "CS"}, /* 0101 */
	{"ne", "NE"}, /* 0110 */
	{"eq", "EQ"}, /* 0111 */
	{"vc", "VC"}, /* 1000 */
	{"vs", "VS"}, /* 1001 */
	{"pl", "PL"}, /* 1010 */
	{"mi", "MI"}, /* 1011 */
	{"ge", "GE"}, /* 1100 */
	{"lt", "LT"}, /* 1101 */
	{"gt", "GT"}, /* 1110 */
	{"le", "LE"}, /* 1111 */
};

/* size to index translator (0 -> 0, 8 and 16 -> 1, 32 -> 2) */
int g_size_select_table[33] =
{
	0,												/* unsized */
	0, 0, 0, 0, 0, 0, 0, 1,							/*    8    */
	0, 0, 0, 0, 0, 0, 0, 1,							/*   16    */
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 2	/*   32    */
};

/* Extra cycles required for certain EA modes */
int g_ea_cycle_table[13][NUM_CPUS][3] =
{/*       000           010           020   */
	{{ 0,  0,  0}, { 0,  0,  0}, { 0,  0,  0}}, /* EA_MODE_NONE */
	{{ 0,  4,  8}, { 0,  4,  8}, { 0,  4,  4}}, /* EA_MODE_AI   */
	{{ 0,  4,  8}, { 0,  4,  8}, { 0,  4,  4}}, /* EA_MODE_PI   */
	{{ 0,  4,  8}, { 0,  4,  8}, { 0,  4,  4}}, /* EA_MODE_PI7  */
	{{ 0,  6, 10}, { 0,  6, 10}, { 0,  5,  5}}, /* EA_MODE_PD   */
	{{ 0,  6, 10}, { 0,  6, 10}, { 0,  5,  5}}, /* EA_MODE_PD7  */
	{{ 0,  8, 12}, { 0,  8, 12}, { 0,  5,  5}}, /* EA_MODE_DI   */
	{{ 0, 10, 14}, { 0, 10, 14}, { 0,  7,  7}}, /* EA_MODE_IX   */
	{{ 0,  8, 12}, { 0,  8, 12}, { 0,  4,  4}}, /* EA_MODE_AW   */
	{{ 0, 12, 16}, { 0, 12, 16}, { 0,  4,  4}}, /* EA_MODE_AL   */
	{{ 0,  8, 12}, { 0,  8, 12}, { 0,  5,  5}}, /* EA_MODE_PCDI */
	{{ 0, 10, 14}, { 0, 10, 14}, { 0,  7,  7}}, /* EA_MODE_PCIX */
	{{ 0,  4,  8}, { 0,  4,  8}, { 0,  2,  4}}, /* EA_MODE_I    */
};

/* Extra cycles for JMP instruction (000, 010) */
int g_jmp_cycle_table[13] =
{
	 0, /* EA_MODE_NONE */
	 4, /* EA_MODE_AI   */
	 0, /* EA_MODE_PI   */
	 0, /* EA_MODE_PI7  */
	 0, /* EA_MODE_PD   */
	 0, /* EA_MODE_PD7  */
	 6, /* EA_MODE_DI   */
	10, /* EA_MODE_IX   */
	 6, /* EA_MODE_AW   */
	 8, /* EA_MODE_AL   */
	 6, /* EA_MODE_PCDI */
	10, /* EA_MODE_PCIX */
	 0, /* EA_MODE_I    */
};

/* Extra cycles for JSR instruction (000, 010) */
int g_jsr_cycle_table[13] =
{
	 0, /* EA_MODE_NONE */
	 4, /* EA_MODE_AI   */
	 0, /* EA_MODE_PI   */
	 0, /* EA_MODE_PI7  */
	 0, /* EA_MODE_PD   */
	 0, /* EA_MODE_PD7  */
	 6, /* EA_MODE_DI   */
	10, /* EA_MODE_IX   */
	 6, /* EA_MODE_AW   */
	 8, /* EA_MODE_AL   */
	 6, /* EA_MODE_PCDI */
	10, /* EA_MODE_PCIX */
	 0, /* EA_MODE_I    */
};

/* Extra cycles for LEA instruction (000, 010) */
int g_lea_cycle_table[13] =
{
	 0, /* EA_MODE_NONE */
	 4, /* EA_MODE_AI   */
	 0, /* EA_MODE_PI   */
	 0, /* EA_MODE_PI7  */
	 0, /* EA_MODE_PD   */
	 0, /* EA_MODE_PD7  */
	 8, /* EA_MODE_DI   */
	12, /* EA_MODE_IX   */
	 8, /* EA_MODE_AW   */
	12, /* EA_MODE_AL   */
	 8, /* EA_MODE_PCDI */
	12, /* EA_MODE_PCIX */
	 0, /* EA_MODE_I    */
};

/* Extra cycles for PEA instruction (000, 010) */
int g_pea_cycle_table[13] =
{
	 0, /* EA_MODE_NONE */
	 6, /* EA_MODE_AI   */
	 0, /* EA_MODE_PI   */
	 0, /* EA_MODE_PI7  */
	 0, /* EA_MODE_PD   */
	 0, /* EA_MODE_PD7  */
	10, /* EA_MODE_DI   */
	14, /* EA_MODE_IX   */
	10, /* EA_MODE_AW   */
	14, /* EA_MODE_AL   */
	10, /* EA_MODE_PCDI */
	14, /* EA_MODE_PCIX */
	 0, /* EA_MODE_I    */
};

/* Extra cycles for MOVEM instruction (000, 010) */
int g_movem_cycle_table[13] =
{
	 0, /* EA_MODE_NONE */
	 0, /* EA_MODE_AI   */
	 0, /* EA_MODE_PI   */
	 0, /* EA_MODE_PI7  */
	 0, /* EA_MODE_PD   */
	 0, /* EA_MODE_PD7  */
	 4, /* EA_MODE_DI   */
	 6, /* EA_MODE_IX   */
	 4, /* EA_MODE_AW   */
	 8, /* EA_MODE_AL   */
	 0, /* EA_MODE_PCDI */
	 0, /* EA_MODE_PCIX */
	 0, /* EA_MODE_I    */
};

/* Extra cycles for MOVES instruction (010) */
int g_moves_cycle_table[13][3] =
{
	{ 0,  0,  0}, /* EA_MODE_NONE */
	{ 0,  4,  6}, /* EA_MODE_AI   */
	{ 0,  4,  6}, /* EA_MODE_PI   */
	{ 0,  4,  6}, /* EA_MODE_PI7  */
	{ 0,  6, 12}, /* EA_MODE_PD   */
	{ 0,  6, 12}, /* EA_MODE_PD7  */
	{ 0, 12, 16}, /* EA_MODE_DI   */
	{ 0, 16, 20}, /* EA_MODE_IX   */
	{ 0, 12, 16}, /* EA_MODE_AW   */
	{ 0, 16, 20}, /* EA_MODE_AL   */
	{ 0,  0,  0}, /* EA_MODE_PCDI */
	{ 0,  0,  0}, /* EA_MODE_PCIX */
	{ 0,  0,  0}, /* EA_MODE_I    */
};

/* Extra cycles for CLR instruction (010) */
int g_clr_cycle_table[13][3] =
{
	{ 0,  0,  0}, /* EA_MODE_NONE */
	{ 0,  4,  6}, /* EA_MODE_AI   */
	{ 0,  4,  6}, /* EA_MODE_PI   */
	{ 0,  4,  6}, /* EA_MODE_PI7  */
	{ 0,  6,  8}, /* EA_MODE_PD   */
	{ 0,  6,  8}, /* EA_MODE_PD7  */
	{ 0,  8, 10}, /* EA_MODE_DI   */
	{ 0, 10, 14}, /* EA_MODE_IX   */
	{ 0,  8, 10}, /* EA_MODE_AW   */
	{ 0, 10, 14}, /* EA_MODE_AL   */
	{ 0,  0,  0}, /* EA_MODE_PCDI */
	{ 0,  0,  0}, /* EA_MODE_PCIX */
	{ 0,  0,  0}, /* EA_MODE_I    */
};



/* ======================================================================== */
/* =========================== UTILITY FUNCTIONS ========================== */
/* ======================================================================== */

/* Print an error message and exit with status error */
void error_exit(const char* fmt, ...)
{
	va_list args;
	fprintf(stderr, "In %s, near or on line %d:\n\t", g_input_filename, g_line_number);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	fprintf(stderr, "\n");

	if(g_prototype_file) fclose(g_prototype_file);
	if(g_table_file) fclose(g_table_file);
	if(g_ops_ac_file) fclose(g_ops_ac_file);
	if(g_ops_dm_file) fclose(g_ops_dm_file);
	if(g_ops_nz_file) fclose(g_ops_nz_file);
	if(g_input_file) fclose(g_input_file);

	exit(EXIT_FAILURE);
}

/* Print an error message, call perror(), and exit with status error */
void perror_exit(const char* fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
	perror("");

	if(g_prototype_file) fclose(g_prototype_file);
	if(g_table_file) fclose(g_table_file);
	if(g_ops_ac_file) fclose(g_ops_ac_file);
	if(g_ops_dm_file) fclose(g_ops_dm_file);
	if(g_ops_nz_file) fclose(g_ops_nz_file);
	if(g_input_file) fclose(g_input_file);

	exit(EXIT_FAILURE);
}


/* copy until 0 or space and exit with error if we read too far */
int check_strsncpy(char* dst, char* src, int maxlength)
{
	char* p = dst;
	while(*src && *src != ' ')
	{
		*p++ = *src++;
		if(p - dst > maxlength)
			error_exit("Field too long");
	}
	*p = 0;
	return p - dst;
}

/* copy until 0 or specified character and exit with error if we read too far */
int check_strcncpy(char* dst, char* src, char delim, int maxlength)
{
	char* p = dst;
	while(*src && *src != delim)
	{
		*p++ = *src++;
		if(p - dst > maxlength)
			error_exit("Field too long");
	}
	*p = 0;
	return p - dst;
}

/* convert ascii to integer and exit with error if we find invalid data */
int check_atoi(char* str, int *result)
{
	int accum = 0;
	char* p = str;
	while(*p >= '0' && *p <= '9')
	{
		accum *= 10;
		accum += *p++ - '0';
	}
	if(*p != ' ' && *p != 0)
		error_exit("Malformed integer value (%c)", *p);
	*result = accum;
	return p - str;
}

/* Skip past spaces in a string */
int skip_spaces(char* str)
{
	char* p = str;

	while(*p == ' ')
		p++;

	return p - str;
}

/* Count the number of set bits in a value */
int num_bits(int value)
{
    value = ((value & 0xaaaa) >> 1) + (value & 0x5555);
    value = ((value & 0xcccc) >> 2) + (value & 0x3333);
    value = ((value & 0xf0f0) >> 4) + (value & 0x0f0f);
    value = ((value & 0xff00) >> 8) + (value & 0x00ff);
	return value;
}

/* Convert a hex value written in ASCII */
int atoh(char* buff)
{
	int accum = 0;

	for(;;buff++)
	{
		if(*buff >= '0' && *buff <= '9')
		{
			accum <<= 4;
			accum += *buff - '0';
		}
		else if(*buff >= 'a' && *buff <= 'f')
		{
			accum <<= 4;
			accum += *buff - 'a' + 10;
		}
		else break;
	}
	return accum;
}

/* Get a line of text from a file, discarding any end-of-line characters */
int fgetline(char* buff, int nchars, FILE* file)
{
	int length;

	if(fgets(buff, nchars, file) == NULL)
		return -1;
	if(buff[0] == '\r')
		memcpy(buff, buff + 1, nchars - 1);

	length = strlen(buff);
	while(length && (buff[length-1] == '\r' || buff[length-1] == '\n'))
		length--;
	buff[length] = 0;
	g_line_number++;

	return length;
}



/* ======================================================================== */
/* =========================== HELPER FUNCTIONS =========================== */
/* ======================================================================== */

/* Calculate the number of cycles an opcode requires */
int get_oper_cycles(opcode_struct* op, int ea_mode, int cpu_type)
{
	int size = g_size_select_table[op->size];

	if(op->cpus[cpu_type] == '.')
		return 0;

	if(cpu_type < CPU_TYPE_020)
	{
		if(cpu_type == CPU_TYPE_010)
		{
			if(strcmp(op->name, "moves") == 0)
				return op->cycles[cpu_type] + g_moves_cycle_table[ea_mode][size];
			if(strcmp(op->name, "clr") == 0)
				return op->cycles[cpu_type] + g_clr_cycle_table[ea_mode][size];
		}

		/* ASG: added these cases -- immediate modes take 2 extra cycles here */
		/* SV: but only when operating on long, and also on register direct mode */
		if(cpu_type == CPU_TYPE_000 && (ea_mode == EA_MODE_I || ea_mode == EA_MODE_NONE) && op->size == 32 &&
		   ((strcmp(op->name, "add") == 0 && strcmp(op->spec_proc, "er") == 0) ||
			strcmp(op->name, "adda")   == 0                                    ||
			(strcmp(op->name, "and") == 0 && strcmp(op->spec_proc, "er") == 0) ||
			(strcmp(op->name, "or") == 0 && strcmp(op->spec_proc, "er") == 0)  ||
			(strcmp(op->name, "sub") == 0 && strcmp(op->spec_proc, "er") == 0) ||
			strcmp(op->name, "suba")   == 0))
			return op->cycles[cpu_type] + g_ea_cycle_table[ea_mode][cpu_type][size] + 2;

		if(strcmp(op->name, "jmp") == 0)
			return op->cycles[cpu_type] + g_jmp_cycle_table[ea_mode];
		if(strcmp(op->name, "jsr") == 0)
			return op->cycles[cpu_type] + g_jsr_cycle_table[ea_mode];
		if(strcmp(op->name, "lea") == 0)
			return op->cycles[cpu_type] + g_lea_cycle_table[ea_mode];
		if(strcmp(op->name, "pea") == 0)
			return op->cycles[cpu_type] + g_pea_cycle_table[ea_mode];
		if(strcmp(op->name, "movem") == 0)
			return op->cycles[cpu_type] + g_movem_cycle_table[ea_mode];
	}
	return op->cycles[cpu_type] + g_ea_cycle_table[ea_mode][cpu_type][size];
}

/* Find an opcode in the opcode handler list */
opcode_struct* find_opcode(char* name, int size, char* spec_proc, char* spec_ea)
{
	opcode_struct* op;


	for(op = g_opcode_input_table;op->name != NULL;op++)
	{
		if(	strcmp(name, op->name) == 0 &&
			(size == op->size) &&
			strcmp(spec_proc, op->spec_proc) == 0 &&
			strcmp(spec_ea, op->spec_ea) == 0)
				return op;
	}
	return NULL;
}

/* Specifically find the illegal opcode in the list */
opcode_struct* find_illegal_opcode(void)
{
	opcode_struct* op;

	for(op = g_opcode_input_table;op->name != NULL;op++)
	{
		if(strcmp(op->name, "illegal") == 0)
			return op;
	}
	return NULL;
}

/* Parse an opcode handler name */
int extract_opcode_info(char* src, char* name, int* size, char* spec_proc, char* spec_ea)
{
	char* ptr = strstr(src, ID_OPHANDLER_NAME);

	if(ptr == NULL)
		return 0;

	ptr += strlen(ID_OPHANDLER_NAME) + 1;

	ptr += check_strcncpy(name, ptr, ',', MAX_NAME_LENGTH);
	if(*ptr != ',') return 0;
	ptr++;
	ptr += skip_spaces(ptr);

	*size = atoi(ptr);
	ptr = strstr(ptr, ",");
	if(ptr == NULL) return 0;
    ptr++;
	ptr += skip_spaces(ptr);

	ptr += check_strcncpy(spec_proc, ptr, ',', MAX_SPEC_PROC_LENGTH);
	if(*ptr != ',') return 0;
	ptr++;
	ptr += skip_spaces(ptr);

	ptr += check_strcncpy(spec_ea, ptr, ')', MAX_SPEC_EA_LENGTH);
	if(*ptr != ')') return 0;
	ptr++;
	ptr += skip_spaces(ptr);

	return 1;
}


/* Add a search/replace pair to a replace structure */
void add_replace_string(replace_struct* replace, const char* search_str, const char* replace_str)
{
	if(replace->length >= MAX_REPLACE_LENGTH)
		error_exit("overflow in replace structure");

	strcpy(replace->replace[replace->length][0], search_str);
	strcpy(replace->replace[replace->length++][1], replace_str);
}

/* Write a function body while replacing any selected strings */
void write_body(FILE* filep, body_struct* body, replace_struct* replace)
{
	int i;
	int j;
	char* ptr;
	char output[MAX_LINE_LENGTH+1];
	char temp_buff[MAX_LINE_LENGTH+1];
	int found;

	for(i=0;i<body->length;i++)
	{
		strcpy(output, body->body[i]);
		/* Check for the base directive header */
		if(strstr(output, ID_BASE) != NULL)
		{
			/* Search for any text we need to replace */
			found = 0;
			for(j=0;j<replace->length;j++)
			{
				ptr = strstr(output, replace->replace[j][0]);
				if(ptr)
				{
					/* We found something to replace */
					found = 1;
					strcpy(temp_buff, ptr+strlen(replace->replace[j][0]));
					strcpy(ptr, replace->replace[j][1]);
					strcat(ptr, temp_buff);
				}
			}
			/* Found a directive with no matching replace string */
			if(!found)
				error_exit("Unknown " ID_BASE " directive");
		}
		fprintf(filep, "%s\n", output);
	}
	fprintf(filep, "\n\n");
}

/* Generate a base function name from an opcode struct */
void get_base_name(char* base_name, opcode_struct* op)
{
	sprintf(base_name, "m68k_op_%s", op->name);
	if(op->size > 0)
		sprintf(base_name+strlen(base_name), "_%d", op->size);
	if(strcmp(op->spec_proc, UNSPECIFIED) != 0)
		sprintf(base_name+strlen(base_name), "_%s", op->spec_proc);
	if(strcmp(op->spec_ea, UNSPECIFIED) != 0)
		sprintf(base_name+strlen(base_name), "_%s", op->spec_ea);
}

/* Write the prototype of an opcode handler function */
void write_prototype(FILE* filep, char* base_name)
{
	fprintf(filep, "void %s(void);\n", base_name);
}

/* Write the name of an opcode handler function */
void write_function_name(FILE* filep, char* base_name)
{
	fprintf(filep, "void %s(void)\n", base_name);
}

void add_opcode_output_table_entry(opcode_struct* op, char* name)
{
	opcode_struct* ptr;
	if(g_opcode_output_table_length > MAX_OPCODE_OUTPUT_TABLE_LENGTH)
		error_exit("Opcode output table overflow");

	ptr = g_opcode_output_table + g_opcode_output_table_length++;

	*ptr = *op;
	strcpy(ptr->name, name);
	ptr->bits = num_bits(ptr->op_mask);
}

/*
 * Comparison function for qsort()
 * For entries with an equal number of set bits in
 * the mask compare the match values
 */
static int DECL_SPEC compare_nof_true_bits(const void* aptr, const void* bptr)
{
	const opcode_struct *a = aptr, *b = bptr;
	if(a->bits != b->bits)
		return a->bits - b->bits;
	if(a->op_mask != b->op_mask)
		return a->op_mask - b->op_mask;
	return a->op_match - b->op_match;
}

void print_opcode_output_table(FILE* filep)
{
	int i;
	qsort((void *)g_opcode_output_table, g_opcode_output_table_length, sizeof(g_opcode_output_table[0]), compare_nof_true_bits);

	for(i=0;i<g_opcode_output_table_length;i++)
		write_table_entry(filep, g_opcode_output_table+i);
}

/* Write an entry in the opcode handler table */
void write_table_entry(FILE* filep, opcode_struct* op)
{
	int i;

	fprintf(filep, "\t{%-28s, 0x%04x, 0x%04x, {",
		op->name, op->op_mask, op->op_match);

	for(i=0;i<NUM_CPUS;i++)
	{
		fprintf(filep, "%3d", op->cycles[i]);
		if(i < NUM_CPUS-1)
			fprintf(filep, ", ");
	}

	fprintf(filep, "}},\n");
}

/* Fill out an opcode struct with a specific addressing mode of the source opcode struct */
void set_opcode_struct(opcode_struct* src, opcode_struct* dst, int ea_mode)
{
	int i;

	*dst = *src;

	for(i=0;i<NUM_CPUS;i++)
		dst->cycles[i] = get_oper_cycles(dst, ea_mode, i);
	if(strcmp(dst->spec_ea, UNSPECIFIED) == 0 && ea_mode != EA_MODE_NONE)
		sprintf(dst->spec_ea, "%s", g_ea_info_table[ea_mode].fname_add);
	dst->op_mask |= g_ea_info_table[ea_mode].mask_add;
	dst->op_match |= g_ea_info_table[ea_mode].match_add;
}


/* Generate a final opcode handler from the provided data */
void generate_opcode_handler(FILE* filep, body_struct* body, replace_struct* replace, opcode_struct* opinfo, int ea_mode)
{
	char str[MAX_LINE_LENGTH+1];
	opcode_struct* op = malloc(sizeof(opcode_struct));

	/* Set the opcode structure and write the tables, prototypes, etc */
	set_opcode_struct(opinfo, op, ea_mode);
	get_base_name(str, op);
	write_prototype(g_prototype_file, str);
	add_opcode_output_table_entry(op, str);
	write_function_name(filep, str);

	/* Add any replace strings needed */
	if(ea_mode != EA_MODE_NONE)
	{
		sprintf(str, "EA_%s_8()", g_ea_info_table[ea_mode].ea_add);
		add_replace_string(replace, ID_OPHANDLER_EA_AY_8, str);
		sprintf(str, "EA_%s_16()", g_ea_info_table[ea_mode].ea_add);
		add_replace_string(replace, ID_OPHANDLER_EA_AY_16, str);
		sprintf(str, "EA_%s_32()", g_ea_info_table[ea_mode].ea_add);
		add_replace_string(replace, ID_OPHANDLER_EA_AY_32, str);
		sprintf(str, "OPER_%s_8()", g_ea_info_table[ea_mode].ea_add);
		add_replace_string(replace, ID_OPHANDLER_OPER_AY_8, str);
		sprintf(str, "OPER_%s_16()", g_ea_info_table[ea_mode].ea_add);
		add_replace_string(replace, ID_OPHANDLER_OPER_AY_16, str);
		sprintf(str, "OPER_%s_32()", g_ea_info_table[ea_mode].ea_add);
		add_replace_string(replace, ID_OPHANDLER_OPER_AY_32, str);
	}

	/* Now write the function body with the selected replace strings */
	write_body(filep, body, replace);
	g_num_functions++;
	free(op);
}

/* Generate opcode variants based on available addressing modes */
void generate_opcode_ea_variants(FILE* filep, body_struct* body, replace_struct* replace, opcode_struct* op)
{
	int old_length = replace->length;

	/* No ea modes available for this opcode */
	if(HAS_NO_EA_MODE(op->ea_allowed))
	{
		generate_opcode_handler(filep, body, replace, op, EA_MODE_NONE);
		return;
	}

	/* Check for and create specific opcodes for each available addressing mode */
	if(HAS_EA_AI(op->ea_allowed))
		generate_opcode_handler(filep, body, replace, op, EA_MODE_AI);
	replace->length = old_length;
	if(HAS_EA_PI(op->ea_allowed))
	{
		generate_opcode_handler(filep, body, replace, op, EA_MODE_PI);
		replace->length = old_length;
		if(op->size == 8)
			generate_opcode_handler(filep, body, replace, op, EA_MODE_PI7);
	}
	replace->length = old_length;
	if(HAS_EA_PD(op->ea_allowed))
	{
		generate_opcode_handler(filep, body, replace, op, EA_MODE_PD);
		replace->length = old_length;
		if(op->size == 8)
			generate_opcode_handler(filep, body, replace, op, EA_MODE_PD7);
	}
	replace->length = old_length;
	if(HAS_EA_DI(op->ea_allowed))
		generate_opcode_handler(filep, body, replace, op, EA_MODE_DI);
	replace->length = old_length;
	if(HAS_EA_IX(op->ea_allowed))
		generate_opcode_handler(filep, body, replace, op, EA_MODE_IX);
	replace->length = old_length;
	if(HAS_EA_AW(op->ea_allowed))
		generate_opcode_handler(filep, body, replace, op, EA_MODE_AW);
	replace->length = old_length;
	if(HAS_EA_AL(op->ea_allowed))
		generate_opcode_handler(filep, body, replace, op, EA_MODE_AL);
	replace->length = old_length;
	if(HAS_EA_PCDI(op->ea_allowed))
		generate_opcode_handler(filep, body, replace, op, EA_MODE_PCDI);
	replace->length = old_length;
	if(HAS_EA_PCIX(op->ea_allowed))
		generate_opcode_handler(filep, body, replace, op, EA_MODE_PCIX);
	replace->length = old_length;
	if(HAS_EA_I(op->ea_allowed))
		generate_opcode_handler(filep, body, replace, op, EA_MODE_I);
	replace->length = old_length;
}

/* Generate variants of condition code opcodes */
void generate_opcode_cc_variants(FILE* filep, body_struct* body, replace_struct* replace, opcode_struct* op_in, int offset)
{
	char repl[20];
	char replnot[20];
	int i;
	int old_length = replace->length;
	opcode_struct* op = malloc(sizeof(opcode_struct));

	*op = *op_in;

	op->op_mask |= 0x0f00;

	/* Do all condition codes except t and f */
	for(i=2;i<16;i++)
	{
		/* Add replace strings for this condition code */
		sprintf(repl, "COND_%s()", g_cc_table[i][1]);
		sprintf(replnot, "COND_NOT_%s()", g_cc_table[i][1]);

		add_replace_string(replace, ID_OPHANDLER_CC, repl);
		add_replace_string(replace, ID_OPHANDLER_NOT_CC, replnot);

		/* Set the new opcode info */
		strcpy(op->name+offset, g_cc_table[i][0]);

		op->op_match = (op->op_match & 0xf0ff) | (i<<8);

		/* Generate all opcode variants for this modified opcode */
		generate_opcode_ea_variants(filep, body, replace, op);
		/* Remove the above replace strings */
		replace->length = old_length;
	}
	free(op);
}

/* Process the opcode handlers section of the input file */
void process_opcode_handlers(void)
{
	FILE* input_file = g_input_file;
	FILE* output_file;
	char func_name[MAX_LINE_LENGTH+1];
	char oper_name[MAX_LINE_LENGTH+1];
	int  oper_size;
	char oper_spec_proc[MAX_LINE_LENGTH+1];
	char oper_spec_ea[MAX_LINE_LENGTH+1];
	opcode_struct* opinfo;
	replace_struct* replace = malloc(sizeof(replace_struct));
	body_struct* body = malloc(sizeof(body_struct));


	output_file = g_ops_ac_file;

	for(;;)
	{
		/* Find the first line of the function */
		func_name[0] = 0;
		while(strstr(func_name, ID_OPHANDLER_NAME) == NULL)
		{
			if(strcmp(func_name, ID_INPUT_SEPARATOR) == 0)
			{
				free(replace);
				free(body);
				return; /* all done */
			}
			if(fgetline(func_name, MAX_LINE_LENGTH, input_file) < 0)
				error_exit("Premature end of file when getting function name");
		}
		/* Get the rest of the function */
		for(body->length=0;;body->length++)
		{
			if(body->length > MAX_BODY_LENGTH)
				error_exit("Function too long");

			if(fgetline(body->body[body->length], MAX_LINE_LENGTH, input_file) < 0)
				error_exit("Premature end of file when getting function body");

			if(body->body[body->length][0] == '}')
			{
				body->length++;
				break;
			}
		}

		g_num_primitives++;

		/* Extract the function name information */
		if(!extract_opcode_info(func_name, oper_name, &oper_size, oper_spec_proc, oper_spec_ea))
			error_exit("Invalid " ID_OPHANDLER_NAME " format");

		/* Find the corresponding table entry */
		opinfo = find_opcode(oper_name, oper_size, oper_spec_proc, oper_spec_ea);
		if(opinfo == NULL)
			error_exit("Unable to find matching table entry for %s", func_name);

        /* Change output files if we pass 'c' or 'n' */
		if(output_file == g_ops_ac_file && oper_name[0] > 'c')
			output_file = g_ops_dm_file;
		else if(output_file == g_ops_dm_file && oper_name[0] > 'm')
			output_file = g_ops_nz_file;

		replace->length = 0;

		/* Generate opcode variants */
		if(strcmp(opinfo->name, "bcc") == 0 || strcmp(opinfo->name, "scc") == 0)
			generate_opcode_cc_variants(output_file, body, replace, opinfo, 1);
		else if(strcmp(opinfo->name, "dbcc") == 0)
			generate_opcode_cc_variants(output_file, body, replace, opinfo, 2);
		else if(strcmp(opinfo->name, "trapcc") == 0)
			generate_opcode_cc_variants(output_file, body, replace, opinfo, 4);
		else
			generate_opcode_ea_variants(output_file, body, replace, opinfo);
	}

	free(replace);
	free(body);
}


/* Populate the opcode handler table from the input file */
void populate_table(void)
{
	char* ptr;
	char bitpattern[17];
	opcode_struct* op;
	char buff[MAX_LINE_LENGTH];
	int i;
	int temp;

	buff[0] = 0;

	/* Find the start of the table */
	while(strcmp(buff, ID_TABLE_START) != 0)
		if(fgetline(buff, MAX_LINE_LENGTH, g_input_file) < 0)
			error_exit("Premature EOF while reading table");

	/* Process the entire table */
	for(op = g_opcode_input_table;;op++)
	{
		if(fgetline(buff, MAX_LINE_LENGTH, g_input_file) < 0)
			error_exit("Premature EOF while reading table");
		if(strlen(buff) == 0)
			continue;
		/* We finish when we find an input separator */
		if(strcmp(buff, ID_INPUT_SEPARATOR) == 0)
			break;

		/* Extract the info from the table */
		ptr = buff;

		/* Name */
		ptr += skip_spaces(ptr);
		ptr += check_strsncpy(op->name, ptr, MAX_NAME_LENGTH);

		/* Size */
		ptr += skip_spaces(ptr);
		ptr += check_atoi(ptr, &temp);
		op->size = (unsigned char)temp;

		/* Special processing */
		ptr += skip_spaces(ptr);
		ptr += check_strsncpy(op->spec_proc, ptr, MAX_SPEC_PROC_LENGTH);

		/* Specified EA Mode */
		ptr += skip_spaces(ptr);
		ptr += check_strsncpy(op->spec_ea, ptr, MAX_SPEC_EA_LENGTH);

		/* Bit Pattern (more processing later) */
		ptr += skip_spaces(ptr);
		ptr += check_strsncpy(bitpattern, ptr, 17);

		/* Allowed Addressing Mode List */
		ptr += skip_spaces(ptr);
		ptr += check_strsncpy(op->ea_allowed, ptr, EA_ALLOWED_LENGTH);

		/* CPU operating mode (U = user or supervisor, S = supervisor only */
		ptr += skip_spaces(ptr);
		for(i=0;i<NUM_CPUS;i++)
		{
			op->cpu_mode[i] = *ptr++;
			ptr += skip_spaces(ptr);
		}

		/* Allowed CPUs for this instruction */
		for(i=0;i<NUM_CPUS;i++)
		{
			ptr += skip_spaces(ptr);
			if(*ptr == UNSPECIFIED_CH)
			{
				op->cpus[i] = UNSPECIFIED_CH;
				op->cycles[i] = 0;
				ptr++;
			}
			else
			{
				op->cpus[i] = '0' + i;
				ptr += check_atoi(ptr, &temp);
				op->cycles[i] = (unsigned char)temp;
			}
		}

		/* generate mask and match from bitpattern */
		op->op_mask = 0;
		op->op_match = 0;
		for(i=0;i<16;i++)
		{
			op->op_mask |= (bitpattern[i] != '.') << (15-i);
			op->op_match |= (bitpattern[i] == '1') << (15-i);
		}
	}
	/* Terminate the list */
	op->name[0] = 0;
}

/* Read a header or footer insert from the input file */
void read_insert(char* insert)
{
	char* ptr = insert;
	char* overflow = insert + MAX_INSERT_LENGTH - MAX_LINE_LENGTH;
	int length;
	char* first_blank = NULL;

	first_blank = NULL;

	/* Skip any leading blank lines */
	for(length = 0;length == 0;length = fgetline(ptr, MAX_LINE_LENGTH, g_input_file))
		if(ptr >= overflow)
			error_exit("Buffer overflow reading inserts");
	if(length < 0)
		error_exit("Premature EOF while reading inserts");

	/* Advance and append newline */
	ptr += length;
	strcpy(ptr++, "\n");

	/* Read until next separator */
	for(;;)
	{
		/* Read a new line */
		if(ptr >= overflow)
			error_exit("Buffer overflow reading inserts");
		if((length = fgetline(ptr, MAX_LINE_LENGTH, g_input_file)) < 0)
			error_exit("Premature EOF while reading inserts");

		/* Stop if we read a separator */
		if(strcmp(ptr, ID_INPUT_SEPARATOR) == 0)
			break;

		/* keep track in case there are trailing blanks */
		if(length == 0)
		{
			if(first_blank == NULL)
				first_blank = ptr;
		}
		else
			first_blank = NULL;

		/* Advance and append newline */
		ptr += length;
		strcpy(ptr++, "\n");
	}

	/* kill any trailing blank lines */
	if(first_blank)
		ptr = first_blank;
	*ptr++ = 0;
}



/* ======================================================================== */
/* ============================= MAIN FUNCTION ============================ */
/* ======================================================================== */

int main(int argc, char **argv)
{
	/* File stuff */
	char output_path[M68K_MAX_DIR] = "";
	char filename[M68K_MAX_PATH];
	/* Section identifier */
	char section_id[MAX_LINE_LENGTH+1];
	/* Inserts */
	char temp_insert[MAX_INSERT_LENGTH+1];
	char prototype_footer_insert[MAX_INSERT_LENGTH+1];
	char table_footer_insert[MAX_INSERT_LENGTH+1];
	char ophandler_footer_insert[MAX_INSERT_LENGTH+1];
	/* Flags if we've processed certain parts already */
	int prototype_header_read = 0;
	int prototype_footer_read = 0;
	int table_header_read = 0;
	int table_footer_read = 0;
	int ophandler_header_read = 0;
	int ophandler_footer_read = 0;
	int table_body_read = 0;
	int ophandler_body_read = 0;

	printf("\n\t\tMusashi v%s 68000, 68008, 68010, 68EC020, 68020 emulator\n", g_version);
	printf("\t\tCopyright 1998-2000 Karl Stenerud (karl@mame.net)\n\n");

	/* Check if output path and source for the input file are given */
    if(argc > 1)
	{
		char *ptr;
		strcpy(output_path, argv[1]);

		for(ptr = strchr(output_path, '\\'); ptr; ptr = strchr(ptr, '\\'))
			*ptr = '/';

#if !(defined(__DECC) && defined(VMS))
        if(output_path[strlen(output_path)-1] != '/')
			strcat(output_path, "/");
#endif

		if(argc > 2)
			strcpy(g_input_filename, argv[2]);
	}


#if defined(__DECC) && defined(VMS)

	/* Open the files we need */
	sprintf(filename, "%s%s", output_path, FILENAME_PROTOTYPE);
	if((g_prototype_file = fopen(filename, "w")) == NULL)
		perror_exit("Unable to create prototype file (%s)\n", filename);

	sprintf(filename, "%s%s", output_path, FILENAME_TABLE);
	if((g_table_file = fopen(filename, "w")) == NULL)
		perror_exit("Unable to create table file (%s)\n", filename);

	sprintf(filename, "%s%s", output_path, FILENAME_OPS_AC);
	if((g_ops_ac_file = fopen(filename, "w")) == NULL)
		perror_exit("Unable to create ops ac file (%s)\n", filename);

	sprintf(filename, "%s%s", output_path, FILENAME_OPS_DM);
	if((g_ops_dm_file = fopen(filename, "w")) == NULL)
		perror_exit("Unable to create ops dm file (%s)\n", filename);

	sprintf(filename, "%s%s", output_path, FILENAME_OPS_NZ);
	if((g_ops_nz_file = fopen(filename, "w")) == NULL)
		perror_exit("Unable to create ops nz file (%s)\n", filename);

	if((g_input_file=fopen(g_input_filename, "r")) == NULL)
		perror_exit("can't open %s for input", g_input_filename);

#else


	/* Open the files we need */
	sprintf(filename, "%s%s", output_path, FILENAME_PROTOTYPE);
	if((g_prototype_file = fopen(filename, "wt")) == NULL)
		perror_exit("Unable to create prototype file (%s)\n", filename);

	sprintf(filename, "%s%s", output_path, FILENAME_TABLE);
	if((g_table_file = fopen(filename, "wt")) == NULL)
		perror_exit("Unable to create table file (%s)\n", filename);

	sprintf(filename, "%s%s", output_path, FILENAME_OPS_AC);
	if((g_ops_ac_file = fopen(filename, "wt")) == NULL)
		perror_exit("Unable to create ops ac file (%s)\n", filename);

	sprintf(filename, "%s%s", output_path, FILENAME_OPS_DM);
	if((g_ops_dm_file = fopen(filename, "wt")) == NULL)
		perror_exit("Unable to create ops dm file (%s)\n", filename);

	sprintf(filename, "%s%s", output_path, FILENAME_OPS_NZ);
	if((g_ops_nz_file = fopen(filename, "wt")) == NULL)
		perror_exit("Unable to create ops nz file (%s)\n", filename);

	if((g_input_file=fopen(g_input_filename, "rt")) == NULL)
		perror_exit("can't open %s for input", g_input_filename);

#endif

	/* Get to the first section of the input file */
	section_id[0] = 0;
	while(strcmp(section_id, ID_INPUT_SEPARATOR) != 0)
		if(fgetline(section_id, MAX_LINE_LENGTH, g_input_file) < 0)
			error_exit("Premature EOF while reading input file");

	/* Now process all sections */
	for(;;)
	{
		if(fgetline(section_id, MAX_LINE_LENGTH, g_input_file) < 0)
			error_exit("Premature EOF while reading input file");
		if(strcmp(section_id, ID_PROTOTYPE_HEADER) == 0)
		{
			if(prototype_header_read)
				error_exit("Duplicate prototype header");
			read_insert(temp_insert);
			fprintf(g_prototype_file, "%s\n\n", temp_insert);
			prototype_header_read = 1;
		}
		else if(strcmp(section_id, ID_TABLE_HEADER) == 0)
		{
			if(table_header_read)
				error_exit("Duplicate table header");
			read_insert(temp_insert);
			fprintf(g_table_file, "%s", temp_insert);
			table_header_read = 1;
		}
		else if(strcmp(section_id, ID_OPHANDLER_HEADER) == 0)
		{
			if(ophandler_header_read)
				error_exit("Duplicate opcode handler header");
			read_insert(temp_insert);
			fprintf(g_ops_ac_file, "%s\n\n", temp_insert);
			fprintf(g_ops_dm_file, "%s\n\n", temp_insert);
			fprintf(g_ops_nz_file, "%s\n\n", temp_insert);
			ophandler_header_read = 1;
		}
		else if(strcmp(section_id, ID_PROTOTYPE_FOOTER) == 0)
		{
			if(prototype_footer_read)
				error_exit("Duplicate prototype footer");
			read_insert(prototype_footer_insert);
			prototype_footer_read = 1;
		}
		else if(strcmp(section_id, ID_TABLE_FOOTER) == 0)
		{
			if(table_footer_read)
				error_exit("Duplicate table footer");
			read_insert(table_footer_insert);
			table_footer_read = 1;
		}
		else if(strcmp(section_id, ID_OPHANDLER_FOOTER) == 0)
		{
			if(ophandler_footer_read)
				error_exit("Duplicate opcode handler footer");
			read_insert(ophandler_footer_insert);
			ophandler_footer_read = 1;
		}
		else if(strcmp(section_id, ID_TABLE_BODY) == 0)
		{
			if(!prototype_header_read)
				error_exit("Table body encountered before prototype header");
			if(!table_header_read)
				error_exit("Table body encountered before table header");
			if(!ophandler_header_read)
				error_exit("Table body encountered before opcode handler header");

			if(table_body_read)
				error_exit("Duplicate table body");

			populate_table();
			table_body_read = 1;
		}
		else if(strcmp(section_id, ID_OPHANDLER_BODY) == 0)
		{
			if(!prototype_header_read)
				error_exit("Opcode handlers encountered before prototype header");
			if(!table_header_read)
				error_exit("Opcode handlers encountered before table header");
			if(!ophandler_header_read)
				error_exit("Opcode handlers encountered before opcode handler header");
			if(!table_body_read)
				error_exit("Opcode handlers encountered before table body");

			if(ophandler_body_read)
				error_exit("Duplicate opcode handler section");

			process_opcode_handlers();

			ophandler_body_read = 1;
		}
		else if(strcmp(section_id, ID_END) == 0)
		{
			/* End of input file.  Do a sanity check and then write footers */
			if(!prototype_header_read)
				error_exit("Missing prototype header");
			if(!prototype_footer_read)
				error_exit("Missing prototype footer");
			if(!table_header_read)
				error_exit("Missing table header");
			if(!table_footer_read)
				error_exit("Missing table footer");
			if(!table_body_read)
				error_exit("Missing table body");
			if(!ophandler_header_read)
				error_exit("Missing opcode handler header");
			if(!ophandler_footer_read)
				error_exit("Missing opcode handler footer");
			if(!ophandler_body_read)
				error_exit("Missing opcode handler body");

			print_opcode_output_table(g_table_file);

			fprintf(g_prototype_file, "%s\n\n", prototype_footer_insert);
			fprintf(g_table_file, "%s\n\n", table_footer_insert);
			fprintf(g_ops_ac_file, "%s\n\n", ophandler_footer_insert);
			fprintf(g_ops_dm_file, "%s\n\n", ophandler_footer_insert);
			fprintf(g_ops_nz_file, "%s\n\n", ophandler_footer_insert);

			break;
		}
		else
		{
			error_exit("Unknown section identifier: %s", section_id);
		}
	}

	/* Close all files and exit */
	fclose(g_prototype_file);
	fclose(g_table_file);
	fclose(g_ops_ac_file);
	fclose(g_ops_dm_file);
	fclose(g_ops_nz_file);
	fclose(g_input_file);

	printf("Generated %d opcode handlers from %d primitives\n", g_num_functions, g_num_primitives);

	return 0;
}



/* ======================================================================== */
/* ============================== END OF FILE ============================= */
/* ======================================================================== */
