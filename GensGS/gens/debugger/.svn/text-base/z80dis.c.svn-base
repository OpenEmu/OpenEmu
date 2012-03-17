#include <stdio.h>
#include <string.h>

#include "gens_core/mem/mem_z80.h"

static const char Mnemonics[256][16] =
{
	"NOP", "LD BC,#h", "LD (BC),A", "INC BC", "INC B", "DEC B", "LD B,*h", "RLCA",
	"EX AF,AF'", "ADD HL,BC", "LD A,(BC)", "DEC BC", "INC C", "DEC C", "LD C,*h", "RRCA",
	"DJNZ *h", "LD DE,#h", "LD (DE),A", "INC DE", "INC D", "DEC D", "LD D,*h", "RLA",
	"JR *h", "ADD HL,DE", "LD A,(DE)", "DEC DE", "INC E", "DEC E", "LD E,*h", "RRA",
	"JR NZ,*h", "LD HL,#h", "LD (#h),HL", "INC HL", "INC H", "DEC H", "LD H,*h", "DAA",
	"JR Z,*h", "ADD HL,HL", "LD HL,(#h)", "DEC HL", "INC L", "DEC L", "LD L,*h", "CPL",
	"JR NC,*h", "LD SP,#h", "LD (#h),A", "INC SP", "INC (HL)", "DEC (HL)", "LD (HL),*h", "SCF",
	"JR C,*h", "ADD HL,SP", "LD A,(#h)", "DEC SP", "INC A", "DEC A", "LD A,*h", "CCF",
	"LD B,B", "LD B,C", "LD B,D", "LD B,E", "LD B,H", "LD B,L", "LD B,(HL)", "LD B,A",
	"LD C,B", "LD C,C", "LD C,D", "LD C,E", "LD C,H", "LD C,L", "LD C,(HL)", "LD C,A",
	"LD D,B", "LD D,C", "LD D,D", "LD D,E", "LD D,H", "LD D,L", "LD D,(HL)", "LD D,A",
	"LD E,B", "LD E,C", "LD E,D", "LD E,E", "LD E,H", "LD E,L", "LD E,(HL)", "LD E,A",
	"LD H,B", "LD H,C", "LD H,D", "LD H,E", "LD H,H", "LD H,L", "LD H,(HL)", "LD H,A",
	"LD L,B", "LD L,C", "LD L,D", "LD L,E", "LD L,H", "LD L,L", "LD L,(HL)", "LD L,A",
	"LD (HL),B", "LD (HL),C", "LD (HL),D", "LD (HL),E", "LD (HL),H", "LD (HL),L", "HALT", "LD (HL),A",
	"LD A,B", "LD A,C", "LD A,D", "LD A,E", "LD A,H", "LD A,L", "LD A,(HL)", "LD A,A",
	"ADD B", "ADD C", "ADD D", "ADD E", "ADD H", "ADD L", "ADD (HL)", "ADD A",
	"ADC B", "ADC C", "ADC D", "ADC E", "ADC H", "ADC L", "ADC (HL)", "ADC,A",
	"SUB B", "SUB C", "SUB D", "SUB E", "SUB H", "SUB L", "SUB (HL)", "SUB A",
	"SBC B", "SBC C", "SBC D", "SBC E", "SBC H", "SBC L", "SBC (HL)", "SBC A",
	"AND B", "AND C", "AND D", "AND E", "AND H", "AND L", "AND (HL)", "AND A",
	"XOR B", "XOR C", "XOR D", "XOR E", "XOR H", "XOR L", "XOR (HL)", "XOR A",
	"OR B", "OR C", "OR D", "OR E", "OR H", "OR L", "OR (HL)", "OR A",
	"CP B", "CP C", "CP D", "CP E", "CP H", "CP L", "CP (HL)", "CP A",
	"RET NZ", "POP BC", "JP NZ,#h", "JP #h", "CALL NZ,#h", "PUSH BC", "ADD *h", "RST 00h",
	"RET Z", "RET", "JP Z,#h", "PFX_CB", "CALL Z,#h", "CALL #h", "ADC *h", "RST 08h",
	"RET NC", "POP DE", "JP NC,#h", "OUTA (*h)", "CALL NC,#h", "PUSH DE", "SUB *h", "RST 10h",
	"RET C", "EXX", "JP C,#h", "INA (*h)", "CALL C,#h", "PFX_DD", "SBC *h", "RST 18h",
	"RET PO", "POP HL", "JP PO,#h", "EX HL,(SP)", "CALL PO,#h", "PUSH HL", "AND *h", "RST 20h",
	"RET PE", "LD PC,HL", "JP PE,#h", "EX DE,HL", "CALL PE,#h", "PFX_ED", "XOR *h", "RST 28h",
	"RET P", "POP AF", "JP P,#h", "DI", "CALL P,#h", "PUSH AF", "OR *h", "RST 30h",
	"RET M", "LD SP,HL", "JP M,#h", "EI", "CALL M,#h", "PFX_FD", "CP *h", "RST 38h"
};

static const char MnemonicsCB[256][16] =
{
	"RLC B", "RLC C", "RLC D", "RLC E", "RLC H", "RLC L", "RLC xHL", "RLC A",
	"RRC B", "RRC C", "RRC D", "RRC E", "RRC H", "RRC L", "RRC xHL", "RRC A",
	"RL B", "RL C", "RL D", "RL E", "RL H", "RL L", "RL xHL", "RL A",
	"RR B", "RR C", "RR D", "RR E", "RR H", "RR L", "RR xHL", "RR A",
	"SLA B", "SLA C", "SLA D", "SLA E", "SLA H", "SLA L", "SLA xHL", "SLA A",
	"SRA B", "SRA C", "SRA D", "SRA E", "SRA H", "SRA L", "SRA xHL", "SRA A",
	"SLL B", "SLL C", "SLL D", "SLL E", "SLL H", "SLL L", "SLL xHL", "SLL A",
	"SRL B", "SRL C", "SRL D", "SRL E", "SRL H", "SRL L", "SRL xHL", "SRL A",
	"BIT 0,B", "BIT 0,C", "BIT 0,D", "BIT 0,E", "BIT 0,H", "BIT 0,L", "BIT 0,(HL)", "BIT 0,A",
	"BIT 1,B", "BIT 1,C", "BIT 1,D", "BIT 1,E", "BIT 1,H", "BIT 1,L", "BIT 1,(HL)", "BIT 1,A",
	"BIT 2,B", "BIT 2,C", "BIT 2,D", "BIT 2,E", "BIT 2,H", "BIT 2,L", "BIT 2,(HL)", "BIT 2,A",
	"BIT 3,B", "BIT 3,C", "BIT 3,D", "BIT 3,E", "BIT 3,H", "BIT 3,L", "BIT 3,(HL)", "BIT 3,A",
	"BIT 4,B", "BIT 4,C", "BIT 4,D", "BIT 4,E", "BIT 4,H", "BIT 4,L", "BIT 4,(HL)", "BIT 4,A",
	"BIT 5,B", "BIT 5,C", "BIT 5,D", "BIT 5,E", "BIT 5,H", "BIT 5,L", "BIT 5,(HL)", "BIT 5,A",
	"BIT 6,B", "BIT 6,C", "BIT 6,D", "BIT 6,E", "BIT 6,H", "BIT 6,L", "BIT 6,(HL)", "BIT 6,A",
	"BIT 7,B", "BIT 7,C", "BIT 7,D", "BIT 7,E", "BIT 7,H", "BIT 7,L", "BIT 7,(HL)", "BIT 7,A",
	"RES 0,B", "RES 0,C", "RES 0,D", "RES 0,E", "RES 0,H", "RES 0,L", "RES 0,(HL)", "RES 0,A",
	"RES 1,B", "RES 1,C", "RES 1,D", "RES 1,E", "RES 1,H", "RES 1,L", "RES 1,(HL)", "RES 1,A",
	"RES 2,B", "RES 2,C", "RES 2,D", "RES 2,E", "RES 2,H", "RES 2,L", "RES 2,(HL)", "RES 2,A",
	"RES 3,B", "RES 3,C", "RES 3,D", "RES 3,E", "RES 3,H", "RES 3,L", "RES 3,(HL)", "RES 3,A",
	"RES 4,B", "RES 4,C", "RES 4,D", "RES 4,E", "RES 4,H", "RES 4,L", "RES 4,(HL)", "RES 4,A",
	"RES 5,B", "RES 5,C", "RES 5,D", "RES 5,E", "RES 5,H", "RES 5,L", "RES 5,(HL)", "RES 5,A",
	"RES 6,B", "RES 6,C", "RES 6,D", "RES 6,E", "RES 6,H", "RES 6,L", "RES 6,(HL)", "RES 6,A",
	"RES 7,B", "RES 7,C", "RES 7,D", "RES 7,E", "RES 7,H", "RES 7,L", "RES 7,(HL)", "RES 7,A",
	"SET 0,B", "SET 0,C", "SET 0,D", "SET 0,E", "SET 0,H", "SET 0,L", "SET 0,(HL)", "SET 0,A",
	"SET 1,B", "SET 1,C", "SET 1,D", "SET 1,E", "SET 1,H", "SET 1,L", "SET 1,(HL)", "SET 1,A",
	"SET 2,B", "SET 2,C", "SET 2,D", "SET 2,E", "SET 2,H", "SET 2,L", "SET 2,(HL)", "SET 2,A",
	"SET 3,B", "SET 3,C", "SET 3,D", "SET 3,E", "SET 3,H", "SET 3,L", "SET 3,(HL)", "SET 3,A",
	"SET 4,B", "SET 4,C", "SET 4,D", "SET 4,E", "SET 4,H", "SET 4,L", "SET 4,(HL)", "SET 4,A",
	"SET 5,B", "SET 5,C", "SET 5,D", "SET 5,E", "SET 5,H", "SET 5,L", "SET 5,(HL)", "SET 5,A",
	"SET 6,B", "SET 6,C", "SET 6,D", "SET 6,E", "SET 6,H", "SET 6,L", "SET 6,(HL)", "SET 6,A",
	"SET 7,B", "SET 7,C", "SET 7,D", "SET 7,E", "SET 7,H", "SET 7,L", "SET 7,(HL)", "SET 7,A"
};

static const char MnemonicsED[256][16] =
{
	"FUCK00", "FUCK01", "FUCK02", "FUCK03", "FUCK04", "FUCK05", "FUCK06", "FUCK07",
	"FUCK08", "FUCK09", "FUCK0A", "FUCK0B", "FUCK0C", "FUCK0D", "FUCK0E", "FUCK0F",
	"FUCK10", "FUCK11", "FUCK12", "FUCK13", "FUCK14", "FUCK15", "FUCK16", "FUCK17",
	"FUCK18", "FUCK19", "FUCK1A", "FUCK1B", "FUCK1C", "FUCK1D", "FUCK1E", "FUCK1F",
	"FUCK20", "FUCK21", "FUCK22", "FUCK23", "FUCK24", "FUCK25", "FUCK26", "FUCK27",
	"FUCK28", "FUCK29", "FUCK2A", "FUCK2B", "FUCK2C", "FUCK2D", "FUCK2E", "FUCK2F",
	"FUCK30", "FUCK31", "FUCK32", "FUCK33", "FUCK34", "FUCK35", "FUCK36", "FUCK37",
	"FUCK38", "FUCK39", "FUCK3A", "FUCK3B", "FUCK3C", "FUCK3D", "FUCK3E", "FUCK3F",
	"IN B,(C)", "OUT (C),B", "SBC HL,BC", "FUCK43", "FUCK44", "RETN", "IM 0", "LD I,A",
	"IN C,(C)", "OUT (C),C", "ADC HL,BC", "FUCK4B", "FUCK4C", "RETI", "FUCK", "LD R,A",
	"IN D,(C)", "OUT (C),D", "SBC HL,DE", "FUCK53", "FUCK54", "FUCK55", "IM 1", "LD A,I",
	"IN E,(C)", "OUT (C),E", "ADC HL,DE", "FUCK5B", "FUCK5C", "FUCK5D", "IM 2", "LD A,R",
	"IN H,(C)", "OUT (C),H", "SBC HL,HL", "FUCK63", "FUCK64", "FUCK65", "FUCK66", "RRD",
	"IN L,(C)", "OUT (C),L", "ADC HL,HL", "FUCK6B", "FUCK6C", "FUCK6D", "FUCK6E", "RLD",
	"IN F,(C)", "FUCK", "SBC HL,SP", "FUCK73", "FUCK74", "FUCK75", "FUCK76", "FUCK77",
	"IN A,(C)", "OUT (C),A", "ADC HL,SP", "FUCK7B", "FUCK7C", "FUCK7D", "FUCK7E", "FUCK7F",
	"FUCK80", "FUCK81", "FUCK82", "FUCK83", "FUCK84", "FUCK85", "FUCK86", "FUCK87",
	"FUCK88", "FUCK89", "FUCK8A", "FUCK8B", "FUCK8C", "FUCK8D", "FUCK8E", "FUCK8F",
	"FUCK90", "FUCK91", "FUCK92", "FUCK93", "FUCK94", "FUCK95", "FUCK96", "FUCK97",
	"FUCK98", "FUCK99", "FUCK9A", "FUCK9B", "FUCK9C", "FUCK9D", "FUCK9E", "FUCK9F",
	"LDI", "CPI", "INI", "OUTI", "FUCKA4", "FUCKA5", "FUCKA6", "FUCKA7",
	"LDD", "CPD", "IND", "OUTD", "FUCKAC", "FUCKAD", "FUCKAE", "FUCKAF",
	"LDIR", "CPIR", "INIR", "OTIR", "FUCKB4", "FUCKB5", "FUCKB6", "FUCKB7",
	"LDDR", "CPDR", "INDR", "OTDR", "FUCKBC", "FUCKBD", "FUCKBE", "FUCKBF",
	"FUCKC0", "FUCKC1", "FUCKC2", "FUCKC3", "FUCKC4", "FUCKC5", "FUCKC6", "FUCKC7",
	"FUCKC8", "FUCKC9", "FUCKCA", "FUCKCB", "FUCKCC", "FUCKCD", "FUCKCE", "FUCKCF",
	"FUCKD0", "FUCKD1", "FUCKD2", "FUCKD3", "FUCKD4", "FUCKD5", "FUCKD6", "FUCKD7",
	"FUCKD8", "FUCKD9", "FUCKDA", "FUCKDB", "FUCKDC", "FUCKDD", "FUCKDE", "FUCKDF",
	"FUCKE0", "FUCKE1", "FUCKE2", "FUCKE3", "FUCKE4", "FUCKE5", "FUCKE6", "FUCKE7",
	"FUCKE8", "FUCKE9", "FUCKEA", "FUCKEB", "FUCKEC", "FUCKED", "FUCKEE", "FUCKEF",
	"FUCKF0", "FUCKF1", "FUCKF2", "FUCKF3", "FUCKF4", "FUCKF5", "FUCKF6", "FUCKF7",
	"FUCKF8", "FUCKF9", "FUCKFA", "FUCKFB", "FUCKFC", "FUCKFD", "FUCKFE", "FUCKFF"
};

static const char MnemonicsXX[256][16] =
{
	"NOP", "LD BC,#h", "LD (BC),A", "INC BC", "INC B", "DEC B", "LD B,*h", "RLCA",
	"EX AF,AF'", "ADD I%,BC", "LD A,(BC)", "DEC BC", "INC C", "DEC C", "LD C,*h", "RRCA",
	"DJNZ *h", "LD DE,#h", "LD (DE),A", "INC DE", "INC D", "DEC D", "LD D,*h", "RLA",
	"JR *h", "ADD I%,DE", "LD A,(DE)", "DEC DE", "INC E", "DEC E", "LD E,*h", "RRA",
	"JR NZ,*h", "LD I%,#h", "LD (#h),I%", "INC I%", "INC I%h", "DEC I%h", "LD I%Xh,*h", "DAA",
	"JR Z,*h", "ADD I%,I%", "LD I%,(#h)", "DEC I%", "INC I%l", "DEC I%l", "LD I%l,*h", "CPL",
	"JR NC,*h", "LD SP,#h", "LD (#h),A", "INC SP", "INC (I%+*h)", "DEC (I%+*h)", "LD (I%+*h),*h", "SCF",
	"JR C,*h", "ADD I%,SP", "LD A,(#h)", "DEC SP", "INC A", "DEC A", "LD A,*h", "CCF",
	"LD B,B", "LD B,C", "LD B,D", "LD B,E", "LD B,I%h", "LD B,I%l", "LD B,(I%+*h)", "LD B,A",
	"LD C,B", "LD C,C", "LD C,D", "LD C,E", "LD C,I%h", "LD C,I%l", "LD C,(I%+*h)", "LD C,A",
	"LD D,B", "LD D,C", "LD D,D", "LD D,E", "LD D,I%h", "LD D,I%l", "LD D,(I%+*h)", "LD D,A",
	"LD E,B", "LD E,C", "LD E,D", "LD E,E", "LD E,I%h", "LD E,I%l", "LD E,(I%+*h)", "LD E,A",
	"LD I%h,B", "LD I%h,C", "LD I%h,D", "LD I%h,E", "LD I%h,I%h", "LD I%h,I%l", "LD H,(I%+*h)", "LD I%h,A",
	"LD I%l,B", "LD I%l,C", "LD I%l,D", "LD I%l,E", "LD I%l,I%h", "LD I%l,I%l", "LD L,(I%+*h)", "LD I%l,A",
	"LD (I%+*h),B", "LD (I%+*h),C", "LD (I%+*h),D", "LD (I%+*h),E", "LD (I%+*h),H", "LD (I%+*h),L", "HALT", "LD (I%+*h),A",
	"LD A,B", "LD A,C", "LD A,D", "LD A,E", "LD A,I%h", "LD A,L", "LD A,(I%+*h)", "LD A,A",
	"ADD B", "ADD C", "ADD D", "ADD E", "ADD I%h", "ADD I%l", "ADD (I%+*h)", "ADD A",
	"ADC B", "ADC C", "ADC D", "ADC E", "ADC I%h", "ADC I%l", "ADC (I%+*h)", "ADC,A",
	"SUB B", "SUB C", "SUB D", "SUB E", "SUB I%h", "SUB I%l", "SUB (I%+*h)", "SUB A",
	"SBC B", "SBC C", "SBC D", "SBC E", "SBC I%h", "SBC I%l", "SBC (I%+*h)", "SBC A",
	"AND B", "AND C", "AND D", "AND E", "AND I%h", "AND I%l", "AND (I%+*h)", "AND A",
	"XOR B", "XOR C", "XOR D", "XOR E", "XOR I%h", "XOR I%l", "XOR (I%+*h)", "XOR A",
	"OR B", "OR C", "OR D", "OR E", "OR I%h", "OR I%l", "OR (I%+*h)", "OR A",
	"CP B", "CP C", "CP D", "CP E", "CP I%h", "CP I%l", "CP (I%+*h)", "CP A",
	"RET NZ", "POP BC", "JP NZ,#h", "JP #h", "CALL NZ,#h", "PUSH BC", "ADD *h", "RST 00h",
	"RET Z", "RET", "JP Z,#h", "PFX_CB", "CALL Z,#h", "CALL #h", "ADC *h", "RST 08h",
	"RET NC", "POP DE", "JP NC,#h", "OUTA (*h)", "CALL NC,#h", "PUSH DE", "SUB *h", "RST 10h",
	"RET C", "EXX", "JP C,#h", "INA (*h)", "CALL C,#h", "PFX_DD", "SBC *h", "RST 18h",
	"RET PO", "POP I%", "JP PO,#h", "EX I%,(SP)", "CALL PO,#h", "PUSH I%", "AND *h", "RST 20h",
	"RET PE", "LD PC,I%", "JP PE,#h", "EX DE,I%", "CALL PE,#h", "PFX_ED", "XOR *h", "RST 28h",
	"RET P", "POP AF", "JP P,#h", "DI", "CALL P,#h", "PUSH AF", "OR *h", "RST 30h",
	"RET M", "LD SP,I%", "JP M,#h", "EI", "CALL M,#h", "PFX_FD", "CP *h", "RST 38h"
};

/**
 * z80dis(): Disassemble Z80 code.
 * @param buf ???
 * @param Counter ???
 * @param str ???
 * @return ???
 */
int z80dis(unsigned char *buf, int *Counter, char str[128])
{
	char S[80], T[80], U[80], *P, *R;
	int I, J;
	
	if ((I = buf[*Counter]) < 0)
		return (0);
	
	memset (S, 0, 80);
	memset (T, 0, 80);
	memset (U, 0, 80);
	memset (str, 0, 128);
	
	sprintf (str, "%.4X: %.2X", (*Counter)++, I);
	
	switch (I)
	{
		case 0xCB:
			if ((I = buf[*Counter]) < 0)
				return 0;
			sprintf(U, "%.2X", I);
			strcpy(S, MnemonicsCB[I]);
			(*Counter)++;
			break;
		case 0xED:
			if ((I = buf[*Counter]) < 0)
				return 0;
			sprintf(U, "%.2X", I);
			strcpy(S, MnemonicsED[I]);
			(*Counter)++;
			break;
		case 0xFD:
			if ((I = buf[*Counter]) < 0)
				return (0);
			sprintf (U, "%.2X", I);
			if (I == 0xCB)
			{
				(*Counter)++;
				if ((I = buf[*Counter]) < 0)
					return 0;
				(*Counter)++;
				if ((J = buf[*Counter]) < 0)
					return 0;
				sprintf(U, "%s%.2X%.2X", U, I, J);
				sprintf(S, "%s, (IY+%.2X)", MnemonicsCB[J], I);
			}
			else
			{
				strcpy(S, MnemonicsXX[I]);
				if (P = strchr(S, '%'))
					*P = 'Y';
			}
			(*Counter)++;
			break;
		case 0xDD:
		if ((I = buf[*Counter]) < 0)
			return 0;
		sprintf (U, "%.2X", I);
		if (I == 0xCB)
		{
			(*Counter)++;
			if ((I = buf[*Counter]) < 0)
				return 0;
			(*Counter)++;
			if ((J = buf[*Counter]) < 0)
				return 0;
			sprintf(U, "%s%.2X%.2X", U, I, J);
			sprintf(S, "%s, (IX+%.2X)", MnemonicsCB[J], I);
		}
		else
		{
			strcpy(S, MnemonicsXX[I]);
			if (P = strchr(S, '%'))
				*P = 'X';
		}
		(*Counter)++;
		break;
	default:
		strcpy(S, Mnemonics[I]);
	}
	
	if (P = strchr(S, '*'))
	{
		if ((I = buf[*Counter]) < 0)
			return 0;
		sprintf(U, "%s%.2X", U, I);
		*P++ = '\0';
		(*Counter)++;
		sprintf(T, "%s%hX", S, I);
		if (R = strchr(P, '*'))
		{
			if ((I = buf[*Counter]) < 0)
				return 0;
			sprintf(U, "%s%.2X", U, I);
			*R++ = '\0';
			(*Counter)++;
			sprintf(strchr(T, '\0'), "%s%hX%s", P, I, R);
		}
		else
			strcat(T, P);
	}
	else if (P = strchr(S, '#'))
	{
		if ((I = buf[*Counter]) < 0)
			return 0;
		(*Counter)++;
		if ((J = buf[*Counter]) < 0)
			return 0;
		sprintf(U, "%s%.2X%.2X", U, I, J);
		*P++ = '\0';
		(*Counter)++;
		sprintf(T, "%s%hX%s", S, 256 * J + I, P);
	}
	else
		strcpy(T, S);
	
	strcat(str, U);
	while (strlen(str) < 18)
		strcat(str, " ");
	strcat(str, T);
	strcat(str, "\n");
	
	return 1;
}

/**
 * z80log(): ???
 * @param PC ???
 * @return ???
 */
int __fastcall z80log (unsigned int PC)
{
	static FILE *F = NULL;
	unsigned int nPC = PC;
	char ch[128];
	
	if (F == NULL)
		F = fopen("z80.txt", "wb");
	
	z80dis(Ram_Z80, &nPC, ch);
	fwrite(ch, 1, strlen (ch), F);
	
	return 0;
}
