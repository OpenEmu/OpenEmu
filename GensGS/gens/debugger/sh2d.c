/*
 * sh2d
 * Bart Trzynadlowski, July 24, 2000
 * Public domain
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SH2D_VERSION "0.2"

#define ZERO_F  0	/* 0 format */
#define N_F     1	/* n format */
#define M_F     2	/* m format */
#define NM_F    3	/* nm format */
#define MD_F    4	/* md format */
#define ND4_F   5	/* nd4 format */
#define NMD_F   6	/* nmd format */
#define D_F     7	/* d format */
#define D12_F   8	/* d12 format */
#define ND8_F   9	/* nd8 format */
#define I_F     10	/* i format */
#define NI_F    11	/* ni format */

typedef struct
{
	int format;
	char *mnem;
	unsigned short mask;	/* mask used to obtain opcode bits */
	unsigned short bits;	/* opcode bits */
	int dat;		/* specific data for situation */
	int sh2;		/* SH-2 specific */
} i_descr;

static i_descr tab[] =
{
	{ZERO_F, "clrt", 0xFFFF, 0x8, 0, 0},
	{ZERO_F, "clrmac", 0xFFFF, 0x28, 0, 0},
	{ZERO_F, "div0u", 0xFFFF, 0x19, 0, 0},
	{ZERO_F, "nop", 0xFFFF, 0x9, 0, 0},
	{ZERO_F, "rte", 0xFFFF, 0x2B, 0, 0},
	{ZERO_F, "rts", 0xFFFF, 0xB, 0, 0},
	{ZERO_F, "sett", 0xFFFF, 0x18, 0, 0},
	{ZERO_F, "sleep", 0xFFFF, 0x1B, 0, 0},
	{N_F, "cmp/pl r%d", 0xF0FF, 0x4015, 0, 0},
	{N_F, "cmp/pz r%d", 0xF0FF, 0x4011, 0, 0},
	{N_F, "dt r%d", 0xF0FF, 0x4010, 0, 1},
	{N_F, "movt r%d", 0xF0FF, 0x0029, 0, 0},
	{N_F, "rotl r%d", 0xF0FF, 0x4004, 0, 0},
	{N_F, "rotr r%d", 0xF0FF, 0x4005, 0, 0},
	{N_F, "rotcl r%d", 0xF0FF, 0x4024, 0, 0},
	{N_F, "rotcr r%d", 0xF0FF, 0x4025, 0, 0},
	{N_F, "shal r%d", 0xF0FF, 0x4020, 0, 0},
	{N_F, "shar r%d", 0xF0FF, 0x4021, 0, 0},
	{N_F, "shll r%d", 0xF0FF, 0x4000, 0, 0},
	{N_F, "shlr r%d", 0xF0FF, 0x4001, 0, 0},
	{N_F, "shll2 r%d", 0xF0FF, 0x4008, 0, 0},
	{N_F, "shlr2 r%d", 0xF0FF, 0x4009, 0, 0},
	{N_F, "shll8 r%d", 0xF0FF, 0x4018, 0, 0},
	{N_F, "shlr8 r%d", 0xF0FF, 0x4019, 0, 0},
	{N_F, "shll16 r%d", 0xF0FF, 0x4028, 0, 0},
	{N_F, "shlr16 r%d", 0xF0FF, 0x4029, 0, 0},
	{N_F, "stc sr, r%d", 0xF0FF, 0x0002, 0, 0},
	{N_F, "stc gbr, r%d", 0xF0FF, 0x0012, 0, 0},
	{N_F, "stc vbr, r%d", 0xF0FF, 0x0022, 0, 0},
	{N_F, "sts mach, r%d", 0xF0FF, 0x000A, 0, 0},
	{N_F, "sts macl, r%d", 0xF0FF, 0x001A, 0, 0},
	{N_F, "sts pr, r%d", 0xF0FF, 0x002A, 0, 0},
	{N_F, "tas.b (r%d)", 0xF0FF, 0x401B, 0, 0},
	{N_F, "stc.l sr, -(r%d)", 0xF0FF, 0x4003, 0, 0},
	{N_F, "stc.l gbr, -(r%d)", 0xF0FF, 0x4013, 0, 0},
	{N_F, "stc.l vbr, -(r%d)", 0xF0FF, 0x4023, 0, 0},
	{N_F, "sts.l mach, -(r%d)", 0xF0FF, 0x4002, 0, 0},
	{N_F, "sts.l macl, -(r%d)", 0xF0FF, 0x4012, 0, 0},
	{N_F, "sts.l pr, -(r%d)", 0xF0FF, 0x4022, 0, 0},
	{M_F, "ldc r%d, sr", 0xF0FF, 0x400E, 0, 0},
	{M_F, "ldc r%d, gbr", 0xF0FF, 0x401E, 0, 0},
	{M_F, "ldc r%d, vbr", 0xF0FF, 0x402E, 0, 0},
	{M_F, "lds r%d, mach", 0xF0FF, 0x400A, 0, 0},
	{M_F, "lds r%d, macl", 0xF0FF, 0x401A, 0, 0},
	{M_F, "lds r%d, pr", 0xF0FF, 0x402A, 0, 0},
	{M_F, "jmp (r%d)", 0xF0FF, 0x402B, 0, 0},
	{M_F, "jsr (r%d)", 0xF0FF, 0x400B, 0, 0},
	{M_F, "ldc.l (r%d)+, sr", 0xF0FF, 0x4007, 0, 0},
	{M_F, "ldc.l (r%d)+, gbr", 0xF0FF, 0x4017, 0, 0},
	{M_F, "ldc.l (r%d)+, vbr", 0xF0FF, 0x4027, 0, 0},
	{M_F, "lds.l (r%d)+, mach", 0xF0FF, 0x4006, 0, 0},
	{M_F, "lds.l (r%d)+, macl", 0xF0FF, 0x4016, 0, 0},
	{M_F, "lds.l (r%d)+, pr", 0xF0FF, 0x4026, 0, 0},
	{M_F, "braf r%d", 0xF0FF, 0x0023, 0, 1},
	{M_F, "bsrf r%d", 0xF0FF, 0x0003, 0, 1},
	{NM_F, "add r%d, r%d", 0xF00F, 0x300C, 0, 0},
	{NM_F, "addc r%d, r%d", 0xF00F, 0x300E, 0, 0},
	{NM_F, "addv r%d, r%d", 0xF00F, 0x300F, 0, 0},
	{NM_F, "and r%d, r%d", 0xF00F, 0x2009, 0, 0},
	{NM_F, "cmp/eq r%d, r%d", 0xF00F, 0x3000, 0, 0},
	{NM_F, "cmp/hs r%d, r%d", 0xF00F, 0x3002, 0, 0},
	{NM_F, "cmp/ge r%d, r%d", 0xF00F, 0x3003, 0, 0},
	{NM_F, "cmp/hi r%d, r%d", 0xF00F, 0x3006, 0, 0},
	{NM_F, "cmp/gt r%d, r%d", 0xF00F, 0x3007, 0, 0},
	{NM_F, "cmp/str r%d, r%d", 0xF00F, 0x200C, 0, 0},
	{NM_F, "div1 r%d, r%d", 0xF00F, 0x3004, 0, 0},
	{NM_F, "div0s r%d, r%d", 0xF00F, 0x2007, 0, 0},
	{NM_F, "dmuls.l r%d, r%d", 0xF00F, 0x300D, 0, 1},
	{NM_F, "dmulu.l r%d, r%d", 0xF00F, 0x3005, 0, 1},
	{NM_F, "exts.b r%d, r%d", 0xF00F, 0x600E, 0, 0},
	{NM_F, "exts.w r%d, r%d", 0xF00F, 0x600F, 0, 0},
	{NM_F, "extu.b r%d, r%d", 0xF00F, 0x600C, 0, 0},
	{NM_F, "extu.w r%d, r%d", 0xF00F, 0x600D, 0, 0},
	{NM_F, "mov r%d, r%d", 0xF00F, 0x6003, 0, 0},
	{NM_F, "mul.l r%d, r%d", 0xF00F, 0x0007, 0, 1},
	{NM_F, "muls.w r%d, r%d", 0xF00F, 0x200F, 0, 0},
	{NM_F, "mulu.w r%d, r%d", 0xF00F, 0x200E, 0, 0},
	{NM_F, "neg r%d, r%d", 0xF00F, 0x600B, 0, 0},
	{NM_F, "negc r%d, r%d", 0xF00F, 0x600A, 0, 0},
	{NM_F, "not r%d, r%d", 0xF00F, 0x6007, 0, 0},
	{NM_F, "or r%d, r%d", 0xF00F, 0x200B, 0, 0},
	{NM_F, "sub r%d, r%d", 0xF00F, 0x3008, 0, 0},
	{NM_F, "subc r%d, r%d", 0xF00F, 0x300A, 0, 0},
	{NM_F, "subv r%d, r%d", 0xF00F, 0x300B, 0, 0},
	{NM_F, "swap.b r%d, r%d", 0xF00F, 0x6008, 0, 0},
	{NM_F, "swap.w r%d, r%d", 0xF00F, 0x6009, 0, 0},
	{NM_F, "tst r%d, r%d", 0xF00F, 0x2008, 0, 0},
	{NM_F, "xor r%d, r%d", 0xF00F, 0x200A, 0, 0},
	{NM_F, "xtrct r%d, r%d", 0xF00F, 0x200D, 0, 0},
	{NM_F, "mov.b r%d, (r%d)", 0xF00F, 0x2000, 0, 0},
	{NM_F, "mov.w r%d, (r%d)", 0xF00F, 0x2001, 0, 0},
	{NM_F, "mov.l r%d, (r%d)", 0xF00F, 0x2002, 0, 0},
	{NM_F, "mov.b (r%d), r%d", 0xF00F, 0x6000, 0, 0},
	{NM_F, "mov.w (r%d), r%d", 0xF00F, 0x6001, 0, 0},
	{NM_F, "mov.l (r%d), r%d", 0xF00F, 0x6002, 0, 0},
	{NM_F, "mac.l (r%d)+, (r%d)+", 0xF00F, 0x000F, 0, 1},
	{NM_F, "mac.w (r%d)+, (r%d)+", 0xF00F, 0x400F, 0, 0},
	{NM_F, "mov.b (r%d)+, r%d", 0xF00F, 0x6004, 0, 0},
	{NM_F, "mov.w (r%d)+, r%d", 0xF00F, 0x6005, 0, 0},
	{NM_F, "mov.l (r%d)+, r%d", 0xF00F, 0x6006, 0, 0},
	{NM_F, "mov.b r%d, -(r%d)", 0xF00F, 0x2004, 0, 0},
	{NM_F, "mov.w r%d, -(r%d)", 0xF00F, 0x2005, 0, 0},
	{NM_F, "mov.l r%d, -(r%d)", 0xF00F, 0x2006, 0, 0},
	{NM_F, "mov.b r%d, (r0, r%d)", 0xF00F, 0x0004, 0, 0},
	{NM_F, "mov.w r%d, (r0, r%d)", 0xF00F, 0x0005, 0, 0},
	{NM_F, "mov.l r%d, (r0, r%d)", 0xF00F, 0x0006, 0, 0},
	{NM_F, "mov.b (r0, r%d), r%d", 0xF00F, 0x000C, 0, 0},
	{NM_F, "mov.w (r0, r%d), r%d", 0xF00F, 0x000D, 0, 0},
	{NM_F, "mov.l (r0, r%d), r%d", 0xF00F, 0x000Ee, 0, 0},
	{MD_F, "mov.b (%03X, r%d), r0", 0xFF00, 0x8400, 0, 0},
	{MD_F, "mov.w (%03X, r%d), r0", 0xFF00, 0x8500, 0, 0},
	{ND4_F, "mov.b r0, (%03X, r%d)", 0xFF00, 0x8000, 0, 0},
	{ND4_F, "mov.w r0, (%03X, r%d)", 0xFF00, 0x8100, 0, 0},
	{NMD_F, "mov.l r%d, (%03X, r%d)", 0xF000, 0x1000, 0, 0},
	{NMD_F, "mov.l (%03X, r%d), r%d", 0xF000, 0x5000, 0, 0},
	{D_F, "mov.b r0, (%03X, gbr)", 0xFF00, 0xC000, 1, 0},
	{D_F, "mov.w r0, (%03X, gbr)", 0xFF00, 0xC100, 2, 0},
	{D_F, "mov.l r0, (%03X, gbr)", 0xFF00, 0xC200, 4, 0},
	{D_F, "mov.b (%03X, gbr), r0", 0xFF00, 0xC400, 1, 0},
	{D_F, "mov.w (%03X, gbr), r0", 0xFF00, 0xC500, 2, 0},
	{D_F, "mov.l (%03X, gbr), r0", 0xFF00, 0xC600, 4, 0},
	{D_F, "mova (%03X, pc), r0", 0xFF00, 0xC700, 4, 0},
	{D_F, "bf %08X", 0xFF00, 0x8B00, 5, 0},
	{D_F, "bf/s %08X", 0xFF00, 0x8F00, 5, 1},
	{D_F, "bt %08X", 0xFF00, 0x8900, 5, 0},
	{D_F, "bt/s %08X", 0xFF00, 0x8D00, 5, 1},
	{D12_F, "bra %08X", 0xF000, 0xA000, 0, 0},
	{D12_F, "bsr %08X", 0xF000, 0xB000, 0, 0},
	{ND8_F, "mov.w (%03X, pc), r%d", 0xF000, 0x9000, 2, 0},
	{ND8_F, "mov.l (%03X, pc), r%d", 0xF000, 0xD000, 4, 0},
	{I_F, "and.b #%02X, (r0, gbr)", 0xFF00, 0xCD00, 0, 0},
	{I_F, "or.b #%02X, (r0, gbr)", 0xFF00, 0xCF00, 0, 0},
	{I_F, "tst.b #%02X, (r0, gbr)", 0xFF00, 0xCC00, 0, 0},
	{I_F, "xor.b #%02X, (r0, gbr)", 0xFF00, 0xCE00, 0, 0},
	{I_F, "and #%02X, r0", 0xFF00, 0xC900, 0, 0},
	{I_F, "cmp/eq #%02X, r0", 0xFF00, 0x8800, 0, 0},
	{I_F, "or #%02X, r0", 0xFF00, 0xCB00, 0, 0},
	{I_F, "tst #%02X, r0", 0xFF00, 0xC800, 0, 0},
	{I_F, "xor #%02X, r0", 0xFF00, 0xCA00, 0, 0},
	{I_F, "trapa #%X", 0xFF00, 0xC300, 0, 0},
	{NI_F, "add #%02X, r%d", 0xF000, 0x7000, 0, 0},
	{NI_F, "mov #%02X, r%d", 0xF000, 0xE000, 0, 0},
	{0, NULL, 0, 0, 0, 0}
};

/**
 * SH2Disasm(): SH-1/SH-2 disassembler.
 * @param cdeb Output buffer.
 * @param v_addr Address.
 * @param op Opcode.
 * @param mode If mode == 0 then SH-2 mode; otherwise, SH-1 mode
 */
void SH2Disasm(char *cdeb, unsigned int v_addr, unsigned short op, int mode)
{
	int i;
	char s_addr[32], s_op[128];
	
	// Invalid Opcode
	const char* InvalidOpcode = "Invalid Opcode";
	
	sprintf (s_addr, "%08X:%04X   ", v_addr, op);
	
	for (i = 0; tab[i].mnem != NULL; i++)	/* 0 format */
	{
		if ((op & tab[i].mask) != tab[i].bits)
			continue;
		
		if (tab[i].sh2 && mode)
		{
			// SH-2 instruction in SH-1 mode. This is invalid!
			strcpy(cdeb, s_addr);
			strcat(cdeb, InvalidOpcode);
			return;
		}
		
		// Check the instruction format.
		switch (tab[i].format)
		{
			case ZERO_F:
				strcpy(s_op, tab[i].mnem);
				break;
			
			case N_F:
				sprintf(s_op, tab[i].mnem, (op >> 8) & 0xF);
				break;
			
			case M_F:
				sprintf(s_op, tab[i].mnem, (op >> 8) & 0xF);
				break;
			
			case NM_F:
				sprintf(s_op, tab[i].mnem, (op >> 4) & 0xF, (op >> 8) & 0xF);
				break;
			
			case MD_F:
				if (op & 0x100)
					sprintf(s_op, tab[i].mnem, (op & 0xF) * 2, (op >> 4) & 0xF);
				else
					sprintf(s_op, tab[i].mnem, op & 0xF, (op >> 4) & 0xF);
				break;
			
			case ND4_F:
				if (op & 0x100)
					sprintf(s_op, tab[i].mnem, (op & 0xF) * 2, (op >> 4) & 0xF);
				else
					sprintf(s_op, tab[i].mnem, (op & 0xF), (op >> 4) & 0xF);
				break;
			
			case NMD_F:
				if ((op & 0xF000) == 0x1000)
					sprintf(s_op, tab[i].mnem, (op >> 4) & 0xF, (op & 0xF) * 4, (op >> 8) & 0xF);
				else
					sprintf(s_op, tab[i].mnem, (op & 0xF) * 4, (op >> 4) & 0xF, (op >> 8) & 0xF);
				break;
			
			case D_F:
				if (tab[i].dat <= 4)
				{
					if ((op & 0xFF00) == 0xC700)
						sprintf(s_op, tab[i].mnem, (op & 0xFF) * tab[i].dat + 4);
					else
						sprintf(s_op, tab[i].mnem, (op & 0xFF) * tab[i].dat);
				}
				else
				{
					if (op & 0x80)	/* sign extend */
						sprintf(s_op, tab[i].mnem, (((op & 0xFF) + 0xFFFFFF00) * 2) + v_addr + 4);
					else
						sprintf(s_op, tab[i].mnem, ((op & 0xFF) * 2) + v_addr + 4);
				}
				break;
			
			case D12_F:
				if (op & 0x800)	/* sign extend */
					sprintf(s_op, tab[i].mnem, ((op & 0xFFF) + 0xFFFFF000) * 2 + v_addr + 4);
				else
					sprintf(s_op, tab[i].mnem, (op & 0xFFF) * 2 + v_addr + 4);
				break;
			
			case ND8_F:
				if ((op & 0xF000) == 0x9000) /* .W */
					sprintf(s_op, tab[i].mnem, (op & 0xFF) * tab[i].dat + 4, (op >> 8) & 0xF);
				else /* .L */
					sprintf(s_op, tab[i].mnem, (op & 0xFF) * tab[i].dat + 4, (op >> 8) & 0xF);
				break;
			
			case I_F:
				sprintf(s_op, tab[i].mnem, op & 0xFF);
				break;
			
			case NI_F:
				sprintf(s_op, tab[i].mnem, op & 0xFF, (op >> 8) & 0xF);
				break;
			
			default:
				strcpy(s_op, InvalidOpcode);
				break;
		}
		
		strcpy(cdeb, s_addr);
		strcat(cdeb, s_op);
		return;
	}
	
	// Invalid Opcode
	strcpy(cdeb, s_addr);
	strcat(cdeb, InvalidOpcode);
}
