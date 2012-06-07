/****************************************************************************

	NEC V30MZ emulator

	Stripped out non-V30MZ clock counts and code.

	Small changes made by dox@space.pl (Corrected bug in NEG instruction , different AUX flag handling in some opcodes)	

    (Re)Written June-September 2000 by Bryan McPhail (mish@tendril.co.uk) based
	on code by Oliver Bergmann (Raul_Bloodworth@hotmail.com) who based code
	on the i286 emulator by Fabrice Frances which had initial work based on
	David Hedley's pcemu(!).

****************************************************************************/

/* This NEC V30MZ emulator may be used for purposes both commercial and noncommercial if you give the author, Bryan McPhail, 
   a small credit somewhere(such as in the documentation for an executable package).
*/

/*
 TODO:
	Implement bus lock fully(prevent interrupts from occuring during a REP sequence, I think...), taking into account
		HLT emulation to prevent deadlocks!
	
	Implement better prefix emulation.  It's extremely kludgey right now.

	Implement prefetch/pipeline emulation.
*/

#include "wswan.h"
#include "memory.h"

#include <string.h>

#include "v30mz.h"
#include "v30mz-private.h"
#include "debug.h"

namespace MDFN_IEN_WSWAN
{

static uint16 old_CS, old_IP;

#ifdef WANT_DEBUGGER
 #define ADDBRANCHTRACE(x,y) { if(branch_trace_hook) branch_trace_hook(old_CS, old_IP, x, y, false); }
 #define ADDBRANCHTRACE_INT(x,y) { if(branch_trace_hook) branch_trace_hook(old_CS, old_IP, x,y, true); }
 #define SETOLDCSIP() { old_CS = I.sregs[PS]; old_IP = I.pc; }
#else
 #define ADDBRANCHTRACE(x,y)	{ }
 #define ADDBRANCHTRACE_INT(x,y)	{ }
 #define SETOLDCSIP() {  }
#endif

typedef union
{                   /* eight general registers */
    uint16 w[8];    /* viewed as 16 bits registers */
    uint8  b[16];   /* or as 8 bit registers */
} v30mz_basicregs_t;

typedef struct
{
	v30mz_basicregs_t regs;
 	uint16	sregs[4];

	uint16	pc;

	int32	SignVal;
	uint32  AuxVal, OverVal, ZeroVal, CarryVal, ParityVal; /* 0 or non-0 valued flags */
	uint8 TF, IF, DF;
} v30mz_regs_t;

static void (*cpu_writemem20)(uint32,uint8) = NULL;
static uint8 (*cpu_readport)(uint32) = NULL;
static void (*cpu_writeport)(uint32, uint8) = NULL;
static uint8 (*cpu_readmem20)(uint32) = NULL;

/***************************************************************************/
/* cpu state                                                               */
/***************************************************************************/

uint32 v30mz_timestamp;
int32 v30mz_ICount;

static v30mz_regs_t I;
static bool InHLT;

static uint32 prefix_base;	/* base address of the latest prefix segment */
static char seg_prefix;		/* prefix segment indicator */

#ifdef WANT_DEBUGGER
static void (*cpu_hook)(uint32) = NULL;
static uint8 (*read_hook)(uint32) = NULL;
static void (*write_hook)(uint32, uint8) = NULL;
static uint8 (*port_read_hook)(uint32) = NULL;
static void (*port_write_hook)(uint32, uint8) = NULL;
static bool hookie_hickey = 0;
static void (*branch_trace_hook)(uint16 from_CS, uint16 from_IP, uint16 to_CS, uint16 to_IP, bool interrupt) = NULL;
#endif

#include "v30mz-ea.inc"
#include "v30mz-modrm.inc"

static uint8 parity_table[256];

static INLINE void i_real_pushf(void)
{
 PUSH( CompressFlags() ); 
 CLK(2);
}

static INLINE void i_real_popf(void)
{
 uint32 tmp; 
 POP(tmp); 
 ExpandFlags(tmp); 
 CLK(3);
}

/***************************************************************************/

void v30mz_init(uint8 (*readmem20)(uint32), void (*writemem20)(uint32,uint8), uint8 (*readport)(uint32), void (*writeport)(uint32, uint8))
{
 cpu_readmem20 = readmem20;
 cpu_writemem20 = writemem20;

 cpu_readport = readport;
 cpu_writeport = writeport;
}

void v30mz_reset(void)
{
 const BREGS reg_name[8] = { AL, CL, DL, BL, AH, CH, DH, BH };

 v30mz_ICount = 0;
 v30mz_timestamp = 0;

 memset(&I, 0, sizeof(I));

 I.sregs[PS] = 0xffff;


 for(unsigned int i = 0; i < 256; i++)
 {
  unsigned int c = 0;

  for (unsigned int j = i; j > 0; j >>= 1)
   if (j & 1) c++;

  parity_table[i] = !(c & 1);
 }

 I.ZeroVal = I.ParityVal = 1;

 for(unsigned int i = 0; i < 256; i++)
 {
  Mod_RM.reg.b[i] = reg_name[(i & 0x38) >> 3];
  Mod_RM.reg.w[i] = (WREGS) ( (i & 0x38) >> 3) ;
 }

 for(unsigned int i = 0xc0; i < 0x100; i++)
 {
  Mod_RM.RM.w[i] = (WREGS)( i & 7 );
  Mod_RM.RM.b[i] = (BREGS)reg_name[i & 7];
 }

 prefix_base = 0;
 seg_prefix = 0;
 InHLT = 0;
}

void v30mz_int(uint32 vector, bool IgnoreIF)
{
	InHLT = FALSE; // This is correct!  Standby mode is always exited when there is an INT signal, regardless of whether interrupt are disabled.
	if(I.IF || IgnoreIF)
	{
	        uint32 dest_seg, dest_off;

		PUSH( CompressFlags() );
		I.TF = I.IF = 0;
		dest_off = ReadWord(vector);
		dest_seg = ReadWord(vector+2);
		PUSH(I.sregs[PS]);
		PUSH(I.pc);
		I.pc = (uint16)dest_off;
		I.sregs[PS] = (uint16)dest_seg;
		ADDBRANCHTRACE_INT(I.sregs[PS], I.pc);
		CLK(32);
	}
}

static void nec_interrupt(unsigned int_num)
{
    uint32 dest_seg, dest_off;

	if (int_num == -1)
		return;

	 i_real_pushf();
	I.TF = I.IF = 0;	
	

    dest_off = ReadWord((int_num)*4);
    dest_seg = ReadWord((int_num)*4+2);

	PUSH(I.sregs[PS]);
	PUSH(I.pc);
	I.pc = (uint16)dest_off;
	I.sregs[PS] = (uint16)dest_seg;
	ADDBRANCHTRACE(I.sregs[PS], I.pc);
}

static bool CheckInHLT(void)
{
 if(InHLT)
 {
  WSwan_InterruptCheck();
  if(InHLT)
  {
   int32 tmp = v30mz_ICount;

   if(tmp > 0)
    CLK(tmp);
   return(1);
  }
 }
 return(0);
}


/****************************************************************************/
/*                             OPCODES                                      */
/****************************************************************************/

static INLINE void i_real_insb(void)
{
 PutMemB(DS1,I.regs.w[IY], read_port(I.regs.w[DW])); 
 I.regs.w[IY]+= -2 * I.DF + 1; 
 CLK(6); 
}

static INLINE void i_real_insw(void)
{ 
 PutMemB(DS1,I.regs.w[IY],read_port(I.regs.w[DW])); 
 PutMemB(DS1,(I.regs.w[IY]+1)&0xffff,read_port((I.regs.w[DW]+1)&0xffff)); 
 I.regs.w[IY]+= -4 * I.DF + 2; 
 CLK(6); 
}

static INLINE void i_real_outsb(void)
{ 
 write_port(I.regs.w[DW],GetMemB(DS0,I.regs.w[IX])); 
 I.regs.w[IX]+= -2 * I.DF + 1; 
 CLK(7); 
} 

static INLINE void i_real_outsw(void)
{
 write_port(I.regs.w[DW],GetMemB(DS0,I.regs.w[IX])); 
 write_port((I.regs.w[DW]+1)&0xffff,GetMemB(DS0,(I.regs.w[IX]+1)&0xffff)); 
 I.regs.w[IX]+= -4 * I.DF + 2; 
 CLK(7); 
}

static INLINE void i_real_movsb(void) 
{ 
 uint32 tmp = GetMemB(DS0,I.regs.w[IX]); 
 PutMemB(DS1,I.regs.w[IY], tmp); 
 I.regs.w[IY] += -2 * I.DF + 1; 
 I.regs.w[IX] += -2 * I.DF + 1; 
 CLK(5); 
}

static INLINE void i_real_movsw(void) 
{
 uint32 tmp = GetMemW(DS0,I.regs.w[IX]); PutMemW(DS1,I.regs.w[IY], tmp); I.regs.w[IY] += -4 * I.DF + 2; 
I.regs.w[IX] += -4 * I.DF + 2; CLK(5); 
}

static INLINE void i_real_cmpsb(void) 
{
 uint32 src = GetMemB(DS1, I.regs.w[IY]); uint32 dst = GetMemB(DS0, I.regs.w[IX]); SUBB; I.regs.w[IY] += -2 * I.DF + 1; 
I.regs.w[IX] += -2 * I.DF + 1; CLK(6); 
}

static INLINE void i_real_cmpsw(void) 
{
 uint32 src = GetMemW(DS1, I.regs.w[IY]); uint32 dst = GetMemW(DS0, I.regs.w[IX]); SUBW; I.regs.w[IY] += -4 * I.DF + 2; 
I.regs.w[IX] += -4 * I.DF + 2; CLK(6); 
}

static INLINE void i_real_stosb(void) 
{
 PutMemB(DS1,I.regs.w[IY],I.regs.b[AL]);       I.regs.w[IY] += -2 * I.DF + 1; CLK(3);  
}

static INLINE void i_real_stosw(void) 
{
 PutMemW(DS1,I.regs.w[IY],I.regs.w[AW]);       I.regs.w[IY] += -4 * I.DF + 2; CLK(3);
}

static INLINE void i_real_lodsb(void) 
{
 I.regs.b[AL] = GetMemB(DS0,I.regs.w[IX]); I.regs.w[IX] += -2 * I.DF + 1; CLK(3); 
}

static INLINE void i_real_lodsw(void) 
{
 I.regs.w[AW] = GetMemW(DS0,I.regs.w[IX]); I.regs.w[IX] += -4 * I.DF + 2; CLK(3); 
}

static INLINE void i_real_scasb(void) 
{ 
 uint32 src = GetMemB(DS1, I.regs.w[IY]);      uint32 dst = I.regs.b[AL]; SUBB;
 I.regs.w[IY] += -2 * I.DF + 1; CLK(4); 
}

static INLINE void i_real_scasw(void) 
{ 
 uint32 src = GetMemW(DS1, I.regs.w[IY]);      uint32 dst = I.regs.w[AW]; SUBW; 
 I.regs.w[IY] += -4 * I.DF + 2; CLK(4); 
}

static void DoOP(uint8 opcode)
{
 //#define OP(num,func_name) static void func_name(void)
 #define OP(num, func_name) case num: 
 #define OP_RANGE(num1, num2, func_name) case num1 ... num2:
 #define OP_EPILOGUE break

switch(opcode)
{
default:
        printf("Invalid op: %02x\n", opcode);
        CLK(10);
	break;

OP( 0x00, i_add_br8  ) { DEF_br8;	ADDB;	PutbackRMByte(ModRM,dst);	CLKM(3,1);	 	} OP_EPILOGUE;
OP( 0x01, i_add_wr16 ) { DEF_wr16;	ADDW;	PutbackRMWord(ModRM,dst);	CLKM(3,1);	} OP_EPILOGUE;
OP( 0x02, i_add_r8b  ) { DEF_r8b;	ADDB;	RegByte(ModRM)=dst;			CLKM(2,1);		} OP_EPILOGUE;
OP( 0x03, i_add_r16w ) { DEF_r16w;	ADDW;	RegWord(ModRM)=dst;			CLKM(2,1);	} OP_EPILOGUE;
OP( 0x04, i_add_ald8 ) { DEF_ald8;	ADDB;	I.regs.b[AL]=dst;			CLK(1);				} OP_EPILOGUE;
OP( 0x05, i_add_axd16) { DEF_axd16;	ADDW;	I.regs.w[AW]=dst;			CLK(1);				} OP_EPILOGUE;
OP( 0x06, i_push_ds1  ) { PUSH(I.sregs[DS1]);	CLK(2); 	} OP_EPILOGUE;
OP( 0x07, i_pop_ds1   ) { POP(I.sregs[DS1]);	CLK(3);	} OP_EPILOGUE;

OP( 0x08, i_or_br8   ) { DEF_br8;	ORB;	PutbackRMByte(ModRM,dst);	CLKM(3,1);		} OP_EPILOGUE;
OP( 0x09, i_or_wr16  ) { DEF_wr16;	ORW;	PutbackRMWord(ModRM,dst);	CLKM(3,1);	} OP_EPILOGUE;
OP( 0x0a, i_or_r8b   ) { DEF_r8b;	ORB;	RegByte(ModRM)=dst;			CLKM(2,1);		} OP_EPILOGUE;
OP( 0x0b, i_or_r16w  ) { DEF_r16w;	ORW;	RegWord(ModRM)=dst;			CLKM(2,1);	} OP_EPILOGUE;
OP( 0x0c, i_or_ald8  ) { DEF_ald8;	ORB;	I.regs.b[AL]=dst;			CLK(1);				} OP_EPILOGUE;
OP( 0x0d, i_or_axd16 ) { DEF_axd16;	ORW;	I.regs.w[AW]=dst;			CLK(1);				} OP_EPILOGUE;
OP( 0x0e, i_push_cs  ) { PUSH(I.sregs[PS]);	CLK(2);	} OP_EPILOGUE;

OP( 0x10, i_adc_br8  ) { DEF_br8;	src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	CLKM(3,1); 		} OP_EPILOGUE;
OP( 0x11, i_adc_wr16 ) { DEF_wr16;	src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	CLKM(3,1);	} OP_EPILOGUE;
OP( 0x12, i_adc_r8b  ) { DEF_r8b;	src+=CF;	ADDB;	RegByte(ModRM)=dst;			CLKM(2, 1); 		} OP_EPILOGUE;
OP( 0x13, i_adc_r16w ) { DEF_r16w;	src+=CF;	ADDW;	RegWord(ModRM)=dst;			CLKM(2, 1);	} OP_EPILOGUE;
OP( 0x14, i_adc_ald8 ) { DEF_ald8;	src+=CF;	ADDB;	I.regs.b[AL]=dst;			CLK(1);				} OP_EPILOGUE;
OP( 0x15, i_adc_axd16) { DEF_axd16;	src+=CF;	ADDW;	I.regs.w[AW]=dst;			CLK(1);				} OP_EPILOGUE;
OP( 0x16, i_push_ss  ) { PUSH(I.sregs[SS]);		CLK(2);	} OP_EPILOGUE;
OP( 0x17, i_pop_ss   ) { POP(I.sregs[SS]);		CLK(3);	} OP_EPILOGUE;

OP( 0x18, i_sbb_br8  ) { DEF_br8;	src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	CLKM(3,1); 		} OP_EPILOGUE;
OP( 0x19, i_sbb_wr16 ) { DEF_wr16;	src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	CLKM(3,1);	} OP_EPILOGUE;
OP( 0x1a, i_sbb_r8b  ) { DEF_r8b;	src+=CF;	SUBB;	RegByte(ModRM)=dst;			CLKM(2,1); 		} OP_EPILOGUE;
OP( 0x1b, i_sbb_r16w ) { DEF_r16w;	src+=CF;	SUBW;	RegWord(ModRM)=dst;			CLKM(2,1);	} OP_EPILOGUE;
OP( 0x1c, i_sbb_ald8 ) { DEF_ald8;	src+=CF;	SUBB;	I.regs.b[AL]=dst;			CLK(1); 				} OP_EPILOGUE;
OP( 0x1d, i_sbb_axd16) { DEF_axd16;	src+=CF;	SUBW;	I.regs.w[AW]=dst;			CLK(1);	} OP_EPILOGUE;
OP( 0x1e, i_push_ds  ) { PUSH(I.sregs[DS0]);		CLK(2);	} OP_EPILOGUE;
OP( 0x1f, i_pop_ds   ) { POP(I.sregs[DS0]);		CLK(3);	} OP_EPILOGUE;

OP( 0x20, i_and_br8  ) { DEF_br8;	ANDB;	PutbackRMByte(ModRM,dst);	CLKM(3,1); 		} OP_EPILOGUE;
OP( 0x21, i_and_wr16 ) { DEF_wr16;	ANDW;	PutbackRMWord(ModRM,dst);	CLKM(3,1);	} OP_EPILOGUE;
OP( 0x22, i_and_r8b  ) { DEF_r8b;	ANDB;	RegByte(ModRM)=dst;			CLKM(2,1);		} OP_EPILOGUE;
OP( 0x23, i_and_r16w ) { DEF_r16w;	ANDW;	RegWord(ModRM)=dst;			CLKM(2,1);	} OP_EPILOGUE;
OP( 0x24, i_and_ald8 ) { DEF_ald8;	ANDB;	I.regs.b[AL]=dst;			CLK(1);				} OP_EPILOGUE;
OP( 0x25, i_and_axd16) { DEF_axd16;	ANDW;	I.regs.w[AW]=dst;			CLK(1);	} OP_EPILOGUE;
OP( 0x26, i_ds1      ) { seg_prefix=TRUE;	prefix_base=I.sregs[DS1]<<4;	CLK(1);		DoOP(FETCHOP);	seg_prefix=FALSE; } OP_EPILOGUE;
OP( 0x27, i_daa      ) { ADJ4(6,0x60);									CLK(10);	} OP_EPILOGUE;

OP( 0x28, i_sub_br8  ) { DEF_br8;	SUBB;	PutbackRMByte(ModRM,dst);	CLKM(3,1); 		} OP_EPILOGUE;
OP( 0x29, i_sub_wr16 ) { DEF_wr16;	SUBW;	PutbackRMWord(ModRM,dst);	CLKM(3,1);	} OP_EPILOGUE;
OP( 0x2a, i_sub_r8b  ) { DEF_r8b;	SUBB;	RegByte(ModRM)=dst;			CLKM(2,1); 		} OP_EPILOGUE;
OP( 0x2b, i_sub_r16w ) { DEF_r16w;	SUBW;	RegWord(ModRM)=dst;			CLKM(2,1);	} OP_EPILOGUE;
OP( 0x2c, i_sub_ald8 ) { DEF_ald8;	SUBB;	I.regs.b[AL]=dst;			CLK(1); 	} OP_EPILOGUE;
OP( 0x2d, i_sub_axd16) { DEF_axd16;	SUBW;	I.regs.w[AW]=dst;			CLK(1);		} OP_EPILOGUE;
OP( 0x2e, i_ps       ) { seg_prefix=TRUE;	prefix_base=I.sregs[PS]<<4;	CLK(1);		DoOP(FETCHOP);	seg_prefix=FALSE; } OP_EPILOGUE;
OP( 0x2f, i_das      ) { ADJ4(-6,-0x60);						CLK(10);	} OP_EPILOGUE;

OP( 0x30, i_xor_br8  ) { DEF_br8;	XORB;	PutbackRMByte(ModRM,dst);	CLKM(3,1);		} OP_EPILOGUE;
OP( 0x31, i_xor_wr16 ) { DEF_wr16;	XORW;	PutbackRMWord(ModRM,dst);	CLKM(3,1);		} OP_EPILOGUE;
OP( 0x32, i_xor_r8b  ) { DEF_r8b;	XORB;	RegByte(ModRM)=dst;			CLKM(2,1); 	} OP_EPILOGUE;
OP( 0x33, i_xor_r16w ) { DEF_r16w;	XORW;	RegWord(ModRM)=dst;			CLKM(2,1);	} OP_EPILOGUE;
OP( 0x34, i_xor_ald8 ) { DEF_ald8;	XORB;	I.regs.b[AL]=dst;			CLK(1); 	} OP_EPILOGUE;
OP( 0x35, i_xor_axd16) { DEF_axd16;	XORW;	I.regs.w[AW]=dst;			CLK(1);		} OP_EPILOGUE;
OP( 0x36, i_ss       ) { seg_prefix=TRUE;	prefix_base=I.sregs[SS]<<4;	CLK(1);		DoOP(FETCHOP);	seg_prefix=FALSE; } OP_EPILOGUE;
OP( 0x37, i_aaa      ) { ADJB(6,1);						CLK(9);			} OP_EPILOGUE;

OP( 0x38, i_cmp_br8  ) { DEF_br8;	SUBB;					CLKM(2,1); 		} OP_EPILOGUE;
OP( 0x39, i_cmp_wr16 ) { DEF_wr16;	SUBW;					CLKM(2,1); 		} OP_EPILOGUE;
OP( 0x3a, i_cmp_r8b  ) { DEF_r8b;	SUBB;					CLKM(2,1); 		} OP_EPILOGUE;
OP( 0x3b, i_cmp_r16w ) { DEF_r16w;	SUBW;					CLKM(2,1); 		} OP_EPILOGUE;
OP( 0x3c, i_cmp_ald8 ) { DEF_ald8;	SUBB;					CLK(1); 		} OP_EPILOGUE;
OP( 0x3d, i_cmp_axd16) { DEF_axd16;	SUBW;					CLK(1);			} OP_EPILOGUE;
OP( 0x3e, i_ds0      ) { seg_prefix=TRUE;	prefix_base=I.sregs[DS0]<<4;	CLK(1);		DoOP(FETCHOP);	seg_prefix=FALSE; } OP_EPILOGUE;
OP( 0x3f, i_aas      ) { ADJB(-6,-1);						CLK(9);	} OP_EPILOGUE;

OP( 0x40, i_inc_ax  ) { IncWordReg(AW);		CLK(1);	} OP_EPILOGUE;
OP( 0x41, i_inc_cx  ) { IncWordReg(CW);		CLK(1);	} OP_EPILOGUE;
OP( 0x42, i_inc_dx  ) { IncWordReg(DW);		CLK(1);	} OP_EPILOGUE;
OP( 0x43, i_inc_bx  ) { IncWordReg(BW);		CLK(1);	} OP_EPILOGUE;
OP( 0x44, i_inc_sp  ) { IncWordReg(SP);		CLK(1);	} OP_EPILOGUE;
OP( 0x45, i_inc_bp  ) { IncWordReg(BP);		CLK(1);	} OP_EPILOGUE;
OP( 0x46, i_inc_si  ) { IncWordReg(IX);		CLK(1);	} OP_EPILOGUE;
OP( 0x47, i_inc_di  ) { IncWordReg(IY);		CLK(1);	} OP_EPILOGUE;

OP( 0x48, i_dec_ax  ) { DecWordReg(AW);		CLK(1);	} OP_EPILOGUE;
OP( 0x49, i_dec_cx  ) { DecWordReg(CW);		CLK(1);	} OP_EPILOGUE;
OP( 0x4a, i_dec_dx  ) { DecWordReg(DW);		CLK(1);	} OP_EPILOGUE;
OP( 0x4b, i_dec_bx  ) { DecWordReg(BW);		CLK(1);	} OP_EPILOGUE;
OP( 0x4c, i_dec_sp  ) { DecWordReg(SP);		CLK(1);	} OP_EPILOGUE;
OP( 0x4d, i_dec_bp  ) { DecWordReg(BP);		CLK(1);	} OP_EPILOGUE;
OP( 0x4e, i_dec_si  ) { DecWordReg(IX);		CLK(1);	} OP_EPILOGUE;
OP( 0x4f, i_dec_di  ) { DecWordReg(IY);		CLK(1);	} OP_EPILOGUE;

OP( 0x50, i_push_ax ) { PUSH(I.regs.w[AW]);	CLK(1); } OP_EPILOGUE;
OP( 0x51, i_push_cx ) { PUSH(I.regs.w[CW]);	CLK(1); } OP_EPILOGUE;
OP( 0x52, i_push_dx ) { PUSH(I.regs.w[DW]);	CLK(1); } OP_EPILOGUE;
OP( 0x53, i_push_bx ) { PUSH(I.regs.w[BW]);	CLK(1); } OP_EPILOGUE;
OP( 0x54, i_push_sp ) { PUSH(I.regs.w[SP]);	CLK(1); } OP_EPILOGUE;
OP( 0x55, i_push_bp ) { PUSH(I.regs.w[BP]);	CLK(1); } OP_EPILOGUE;
OP( 0x56, i_push_si ) { PUSH(I.regs.w[IX]);	CLK(1); } OP_EPILOGUE;
OP( 0x57, i_push_di ) { PUSH(I.regs.w[IY]);	CLK(1); } OP_EPILOGUE;

OP( 0x58, i_pop_ax  ) { POP(I.regs.w[AW]);	CLK(1); } OP_EPILOGUE;
OP( 0x59, i_pop_cx  ) { POP(I.regs.w[CW]);	CLK(1); } OP_EPILOGUE;
OP( 0x5a, i_pop_dx  ) { POP(I.regs.w[DW]);	CLK(1); } OP_EPILOGUE;
OP( 0x5b, i_pop_bx  ) { POP(I.regs.w[BW]);	CLK(1); } OP_EPILOGUE;
OP( 0x5c, i_pop_sp  ) { POP(I.regs.w[SP]);	CLK(1); } OP_EPILOGUE;
OP( 0x5d, i_pop_bp  ) { POP(I.regs.w[BP]);	CLK(1); } OP_EPILOGUE;
OP( 0x5e, i_pop_si  ) { POP(I.regs.w[IX]);	CLK(1); } OP_EPILOGUE;
OP( 0x5f, i_pop_di  ) { POP(I.regs.w[IY]);	CLK(1); } OP_EPILOGUE;

OP( 0x60, i_pusha  ) {
	unsigned tmp=I.regs.w[SP];
	PUSH(I.regs.w[AW]);
	PUSH(I.regs.w[CW]);
	PUSH(I.regs.w[DW]);
	PUSH(I.regs.w[BW]);
    PUSH(tmp);
	PUSH(I.regs.w[BP]);
	PUSH(I.regs.w[IX]);
	PUSH(I.regs.w[IY]);
	CLK(9);
} OP_EPILOGUE;

OP( 0x61, i_popa  ) {
    unsigned tmp;
	POP(I.regs.w[IY]);
	POP(I.regs.w[IX]);
	POP(I.regs.w[BP]);
    POP(tmp);
	POP(I.regs.w[BW]);
	POP(I.regs.w[DW]);
	POP(I.regs.w[CW]);
	POP(I.regs.w[AW]);
	CLK(8);
} OP_EPILOGUE;

OP( 0x62, i_chkind  ) 
{
	uint32 low,high,tmp;

	GetModRM;

	low = GetRMWord(ModRM);
	high = GetnextRMWord;
	tmp = RegWord(ModRM);

	CLK(13);
	if (tmp<low || tmp>high) 
	{
		nec_interrupt(5);
    	}
} OP_EPILOGUE;

OP( 0x68, i_push_d16 ) { uint32 tmp;	FETCHuint16(tmp); PUSH(tmp);	CLK(1);	} OP_EPILOGUE;
OP( 0x69, i_imul_d16 ) { uint32 tmp;	DEF_r16w; 	FETCHuint16(tmp); dst = (int32)((int16)src)*(int32)((int16)tmp); I.CarryVal = I.OverVal = (((int32)dst) >> 15 != 0) && (((int32)dst) >> 15 != -1);     RegWord(ModRM)=(uint16)dst;     CLKM(4,3);} OP_EPILOGUE;
OP( 0x6a, i_push_d8  ) { uint32 tmp = (uint16)((int16)((int8)FETCH)); 	PUSH(tmp);	CLK(1);	} OP_EPILOGUE;
OP( 0x6b, i_imul_d8  ) { uint32 src2; DEF_r16w; src2= (uint16)((int16)((int8)FETCH)); dst = (int32)((int16)src)*(int32)((int16)src2); I.CarryVal = I.OverVal = (((int32)dst) >> 15 != 0) && (((int32)dst) >> 15 != -1); RegWord(ModRM)=(uint16)dst; CLKM(4,3); } OP_EPILOGUE;
OP( 0x6c, i_insb     ) { i_real_insb(); } OP_EPILOGUE;
OP( 0x6d, i_insw     ) { i_real_insw(); } OP_EPILOGUE;
OP( 0x6e, i_outsb    ) { i_real_outsb(); } OP_EPILOGUE;
OP( 0x6f, i_outsw    ) { i_real_outsw(); } OP_EPILOGUE;

OP( 0x70, i_jo      ) { JMP( FLAG_O);			CLK(1); } OP_EPILOGUE;
OP( 0x71, i_jno     ) { JMP(!FLAG_O);			CLK(1); } OP_EPILOGUE;
OP( 0x72, i_jc      ) { JMP( CF);			CLK(1); } OP_EPILOGUE;
OP( 0x73, i_jnc     ) { JMP(!CF);			CLK(1); } OP_EPILOGUE;
OP( 0x74, i_jz      ) { JMP( ZF);			CLK(1); } OP_EPILOGUE;
OP( 0x75, i_jnz     ) { JMP(!ZF);			CLK(1); } OP_EPILOGUE;
OP( 0x76, i_jce     ) { JMP(CF || ZF);			CLK(1); } OP_EPILOGUE;
OP( 0x77, i_jnce    ) { JMP(!(CF || ZF));		CLK(1); } OP_EPILOGUE;
OP( 0x78, i_js      ) { JMP( SF);			CLK(1); } OP_EPILOGUE;
OP( 0x79, i_jns     ) { JMP(!SF);			CLK(1); } OP_EPILOGUE;
OP( 0x7a, i_jp      ) { JMP( PF);			CLK(1); } OP_EPILOGUE;
OP( 0x7b, i_jnp     ) { JMP(!PF);			CLK(1); } OP_EPILOGUE;
OP( 0x7c, i_jl      ) { JMP((SF!=FLAG_O)&&(!ZF));	CLK(1); } OP_EPILOGUE;
OP( 0x7d, i_jnl     ) { JMP((ZF)||(SF==FLAG_O));	CLK(1); } OP_EPILOGUE;
OP( 0x7e, i_jle     ) { JMP((ZF)||(SF!=FLAG_O)); 	CLK(1); } OP_EPILOGUE;
OP( 0x7f, i_jnle    ) { JMP((SF==FLAG_O)&&(!ZF));	CLK(1); } OP_EPILOGUE;

OP( 0x80, i_80pre   ) { uint32 dst, src; GetModRM; dst = GetRMByte(ModRM); src = FETCH;
	CLKM(3, 1);
	switch (ModRM & 0x38) {
	    case 0x00: ADDB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x08: ORB;				PutbackRMByte(ModRM,dst);	break;
	    case 0x10: src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	break;
	    case 0x18: src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	break;
		case 0x20: ANDB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x28: SUBB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x30: XORB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x38: SUBB;			break;	/* CMP */
    }
} OP_EPILOGUE;

OP( 0x81, i_81pre   ) { uint32 dst, src; GetModRM; dst = GetRMWord(ModRM); src = FETCH; src+= (FETCH << 8);
	CLKM(3, 1);
    switch (ModRM & 0x38) {
	    case 0x00: ADDW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x08: ORW;				PutbackRMWord(ModRM,dst);	break;
	    case 0x10: src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	break;
	    case 0x18: src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	break;
		case 0x20: ANDW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x28: SUBW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x30: XORW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x38: SUBW;			break;	/* CMP */
    }
} OP_EPILOGUE;

OP( 0x82, i_82pre   ) { uint32 dst, src; GetModRM; dst = GetRMByte(ModRM); src = (uint8)((int8)FETCH);
	CLKM(3,1);
	switch (ModRM & 0x38) {
	    case 0x00: ADDB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x08: ORB;				PutbackRMByte(ModRM,dst);	break;
	    case 0x10: src+=CF;	ADDB;	PutbackRMByte(ModRM,dst);	break;
	    case 0x18: src+=CF;	SUBB;	PutbackRMByte(ModRM,dst);	break;
		case 0x20: ANDB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x28: SUBB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x30: XORB;			PutbackRMByte(ModRM,dst);	break;
	    case 0x38: SUBB;			break;	/* CMP */
    }
} OP_EPILOGUE;

OP( 0x83, i_83pre   ) { uint32 dst, src; GetModRM; dst = GetRMWord(ModRM); src = (uint16)((int16)((int8)FETCH));
	CLKM(3,1);
    switch (ModRM & 0x38) {
	    case 0x00: ADDW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x08: ORW;				PutbackRMWord(ModRM,dst);	break;
	    case 0x10: src+=CF;	ADDW;	PutbackRMWord(ModRM,dst);	break;
	    case 0x18: src+=CF;	SUBW;	PutbackRMWord(ModRM,dst);	break;
		case 0x20: ANDW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x28: SUBW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x30: XORW;			PutbackRMWord(ModRM,dst);	break;
	    case 0x38: SUBW;			break;	/* CMP */
    }
} OP_EPILOGUE;

OP( 0x84, i_test_br8  ) { DEF_br8;	ANDB;	CLKM(2,1);	} OP_EPILOGUE;
OP( 0x85, i_test_wr16 ) { DEF_wr16;	ANDW;	CLKM(2,1);	} OP_EPILOGUE;
OP( 0x86, i_xchg_br8  ) { DEF_br8;	RegByte(ModRM)=dst; PutbackRMByte(ModRM,src); CLKM(5,3); } OP_EPILOGUE;
OP( 0x87, i_xchg_wr16 ) { DEF_wr16;	RegWord(ModRM)=dst; PutbackRMWord(ModRM,src); CLKM(5,3); } OP_EPILOGUE;

OP( 0x88, i_mov_br8   ) { uint8  src; GetModRM; src = RegByte(ModRM); 	PutRMByte(ModRM,src); 	CLK(1);	} OP_EPILOGUE;
OP( 0x89, i_mov_wr16  ) { uint16 src; GetModRM; src = RegWord(ModRM); 	PutRMWord(ModRM,src);	CLK(1); } OP_EPILOGUE;
OP( 0x8a, i_mov_r8b   ) { uint8  src; GetModRM; src = GetRMByte(ModRM);	RegByte(ModRM)=src;	CLK(1);	} OP_EPILOGUE;
OP( 0x8b, i_mov_r16w  ) { uint16 src; GetModRM; src = GetRMWord(ModRM);	RegWord(ModRM)=src; 	CLK(1); } OP_EPILOGUE;
OP( 0x8c, i_mov_wsreg ) { GetModRM; PutRMWord(ModRM,I.sregs[(ModRM & 0x38) >> 3]);		CLK(1);	} OP_EPILOGUE;
OP( 0x8d, i_lea       ) { uint16 ModRM = FETCH; if(ModRM >= 192) { printf("LEA Error: %02x\n", ModRM);} else { (void)(*GetEA[ModRM])(); } RegWord(ModRM)=EO; 	CLK(1);	} OP_EPILOGUE;
OP( 0x8e, i_mov_sregw ) { uint16 src; GetModRM; src = GetRMWord(ModRM); CLKM(3,2);
    switch (ModRM & 0x38) {
	    case 0x00: I.sregs[DS1] = src; break; /* mov ds1,ew */
		case 0x08: I.sregs[PS] = src; break; /* mov cs,ew */
	    case 0x10: I.sregs[SS] = src; break; /* mov ss,ew */
	    case 0x18: I.sregs[DS0] = src; break; /* mov ds0,ew */
    }
} OP_EPILOGUE;

OP( 0x8f, i_popw ) { uint16 tmp; GetModRM; POP(tmp); PutRMWord(ModRM,tmp); CLKM(3,1); } OP_EPILOGUE;
OP( 0x90, i_nop  ) { CLK(3); } OP_EPILOGUE;

OP( 0x91, i_xchg_axcx ) { XchgAWReg(CW); CLK(3); } OP_EPILOGUE;
OP( 0x92, i_xchg_axdx ) { XchgAWReg(DW); CLK(3); } OP_EPILOGUE;
OP( 0x93, i_xchg_axbx ) { XchgAWReg(BW); CLK(3); } OP_EPILOGUE;
OP( 0x94, i_xchg_axsp ) { XchgAWReg(SP); CLK(3); } OP_EPILOGUE;
OP( 0x95, i_xchg_axbp ) { XchgAWReg(BP); CLK(3); } OP_EPILOGUE;
OP( 0x96, i_xchg_axsi ) { XchgAWReg(IX); CLK(3); } OP_EPILOGUE;
OP( 0x97, i_xchg_axdi ) { XchgAWReg(IY); CLK(3); } OP_EPILOGUE;

// AKA CVTBW
OP( 0x98, i_cbw       ) { I.regs.b[AH] = (I.regs.b[AL] & 0x80) ? 0xff : 0;	CLK(1);	} OP_EPILOGUE;

// AKA CVTWL
OP( 0x99, i_cwd       ) { I.regs.w[DW] = (I.regs.b[AH] & 0x80) ? 0xffff : 0;	CLK(1);	} OP_EPILOGUE;

OP( 0x9a, i_call_far  ) { uint32 tmp, tmp2;	FETCHuint16(tmp); FETCHuint16(tmp2); PUSH(I.sregs[PS]); PUSH(I.pc); I.pc = (uint16)tmp; I.sregs[PS] = (uint16)tmp2; ADDBRANCHTRACE(I.sregs[PS], I.pc); CLK(10); } OP_EPILOGUE;
OP( 0x9b, i_poll      ) { puts("POLL"); } OP_EPILOGUE;
OP( 0x9c, i_pushf     ) { i_real_pushf(); } OP_EPILOGUE;
OP( 0x9d, i_popf      ) { i_real_popf();  } OP_EPILOGUE;
OP( 0x9e, i_sahf      ) { uint32 tmp = (CompressFlags() & 0xff00) | (I.regs.b[AH] & 0xd5); ExpandFlags(tmp); CLK(4); } OP_EPILOGUE;
OP( 0x9f, i_lahf      ) { I.regs.b[AH] = CompressFlags() & 0xff; CLK(2); } OP_EPILOGUE;

OP( 0xa0, i_mov_aldisp ) { uint32 addr; FETCHuint16(addr); I.regs.b[AL] = GetMemB(DS0, addr); CLK(1); } OP_EPILOGUE;
OP( 0xa1, i_mov_axdisp ) { uint32 addr; FETCHuint16(addr); I.regs.b[AL] = GetMemB(DS0, addr); I.regs.b[AH] = GetMemB(DS0, (addr+1)&0xffff); CLK(1); } OP_EPILOGUE;
OP( 0xa2, i_mov_dispal ) { uint32 addr; FETCHuint16(addr); PutMemB(DS0, addr, I.regs.b[AL]);  CLK(1); } OP_EPILOGUE;
OP( 0xa3, i_mov_dispax ) { uint32 addr; FETCHuint16(addr); PutMemB(DS0, addr, I.regs.b[AL]);  PutMemB(DS0, (addr+1)&0xffff, I.regs.b[AH]); CLK(1); } OP_EPILOGUE;

OP( 0xa4, i_movsb      ) { i_real_movsb(); } OP_EPILOGUE;
OP( 0xa5, i_movsw      ) { i_real_movsw(); } OP_EPILOGUE;
OP( 0xa6, i_cmpsb      ) { i_real_cmpsb(); } OP_EPILOGUE;
OP( 0xa7, i_cmpsw      ) { i_real_cmpsw(); } OP_EPILOGUE;

OP( 0xa8, i_test_ald8  ) { DEF_ald8;  ANDB; CLK(1); } OP_EPILOGUE;
OP( 0xa9, i_test_axd16 ) { DEF_axd16; ANDW; CLK(1); } OP_EPILOGUE;

OP( 0xaa, i_stosb      ) { i_real_stosb(); } OP_EPILOGUE;
OP( 0xab, i_stosw      ) { i_real_stosw(); } OP_EPILOGUE;
OP( 0xac, i_lodsb      ) { i_real_lodsb(); } OP_EPILOGUE;
OP( 0xad, i_lodsw      ) { i_real_lodsw(); } OP_EPILOGUE;
OP( 0xae, i_scasb      ) { i_real_scasb(); } OP_EPILOGUE;
OP( 0xaf, i_scasw      ) { i_real_scasw(); } OP_EPILOGUE;

OP( 0xb0, i_mov_ald8  ) { I.regs.b[AL] = FETCH;	CLK(1); } OP_EPILOGUE;
OP( 0xb1, i_mov_cld8  ) { I.regs.b[CL] = FETCH; CLK(1); } OP_EPILOGUE;
OP( 0xb2, i_mov_dld8  ) { I.regs.b[DL] = FETCH; CLK(1); } OP_EPILOGUE;
OP( 0xb3, i_mov_bld8  ) { I.regs.b[BL] = FETCH; CLK(1); } OP_EPILOGUE;
OP( 0xb4, i_mov_ahd8  ) { I.regs.b[AH] = FETCH; CLK(1); } OP_EPILOGUE;
OP( 0xb5, i_mov_chd8  ) { I.regs.b[CH] = FETCH; CLK(1); } OP_EPILOGUE;
OP( 0xb6, i_mov_dhd8  ) { I.regs.b[DH] = FETCH; CLK(1); } OP_EPILOGUE;
OP( 0xb7, i_mov_bhd8  ) { I.regs.b[BH] = FETCH;	CLK(1); } OP_EPILOGUE;

OP( 0xb8, i_mov_axd16 ) { I.regs.b[AL] = FETCH;	 I.regs.b[AH] = FETCH;	CLK(1); } OP_EPILOGUE;
OP( 0xb9, i_mov_cxd16 ) { I.regs.b[CL] = FETCH;	 I.regs.b[CH] = FETCH;	CLK(1); } OP_EPILOGUE;
OP( 0xba, i_mov_dxd16 ) { I.regs.b[DL] = FETCH;	 I.regs.b[DH] = FETCH;	CLK(1); } OP_EPILOGUE;
OP( 0xbb, i_mov_bxd16 ) { I.regs.b[BL] = FETCH;	 I.regs.b[BH] = FETCH;	CLK(1); } OP_EPILOGUE;
OP( 0xbc, i_mov_spd16 ) { I.regs.b[SPL] = FETCH; I.regs.b[SPH] = FETCH;	CLK(1); } OP_EPILOGUE;
OP( 0xbd, i_mov_bpd16 ) { I.regs.b[BPL] = FETCH; I.regs.b[BPH] = FETCH; CLK(1); } OP_EPILOGUE;
OP( 0xbe, i_mov_sid16 ) { I.regs.b[IXL] = FETCH; I.regs.b[IXH] = FETCH;	CLK(1); } OP_EPILOGUE;
OP( 0xbf, i_mov_did16 ) { I.regs.b[IYL] = FETCH; I.regs.b[IYH] = FETCH;	CLK(1); } OP_EPILOGUE;

OP( 0xc0, i_rotshft_bd8 ) {
	uint32 src, dst; uint8 c;
	GetModRM; src = (unsigned)GetRMByte(ModRM); dst=src;
	c=FETCH;
	c&=0x1f;
	CLKM(5,3);
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_uint8;  c--; } while (c>0); PutbackRMByte(ModRM,(uint8)dst); break;
		case 0x08: do { ROR_uint8;  c--; } while (c>0); PutbackRMByte(ModRM,(uint8)dst); break;
		case 0x10: do { ROLC_uint8; c--; } while (c>0); PutbackRMByte(ModRM,(uint8)dst); break;
		case 0x18: do { RORC_uint8; c--; } while (c>0); PutbackRMByte(ModRM,(uint8)dst); break;
		case 0x20: SHL_uint8(c);	I.AuxVal = 1; break;//
		case 0x28: SHR_uint8(c);	I.AuxVal = 1; break;//
		case 0x30:  break;
		case 0x38: SHRA_uint8(c); break;
	}
} OP_EPILOGUE;

OP( 0xc1, i_rotshft_wd8 ) {
	uint32 src, dst;  uint8 c;
	GetModRM; src = (unsigned)GetRMWord(ModRM); dst=src;
	c=FETCH;
	c&=0x1f;
	CLKM(5,3);
    if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_uint16;  c--; } while (c>0); PutbackRMWord(ModRM,(uint16)dst); break;
		case 0x08: do { ROR_uint16;  c--; } while (c>0); PutbackRMWord(ModRM,(uint16)dst); break;
		case 0x10: do { ROLC_uint16; c--; } while (c>0); PutbackRMWord(ModRM,(uint16)dst); break;
		case 0x18: do { RORC_uint16; c--; } while (c>0); PutbackRMWord(ModRM,(uint16)dst); break;
		case 0x20: SHL_uint16(c);	I.AuxVal = 1; break;
		case 0x28: SHR_uint16(c);	I.AuxVal = 1; break;
		case 0x30:  break;
		case 0x38: SHRA_uint16(c); break;
	}
} OP_EPILOGUE;

OP( 0xc2, i_ret_d16  ) { uint32 count = FETCH; count += FETCH << 8; POP(I.pc); I.regs.w[SP]+=count; CLK(6); ADDBRANCHTRACE(I.sregs[PS], I.pc); } OP_EPILOGUE;
OP( 0xc3, i_ret      ) { POP(I.pc); CLK(6); ADDBRANCHTRACE(I.sregs[PS], I.pc); } OP_EPILOGUE;
OP( 0xc4, i_les_dw   ) { GetModRM; uint16 tmp = GetRMWord(ModRM); RegWord(ModRM)=tmp; I.sregs[DS1] = GetnextRMWord; CLK(6); } OP_EPILOGUE;
OP( 0xc5, i_lds_dw   ) { GetModRM; uint16 tmp = GetRMWord(ModRM); RegWord(ModRM)=tmp; I.sregs[DS0] = GetnextRMWord; CLK(6); } OP_EPILOGUE;
OP( 0xc6, i_mov_bd8  ) { GetModRM; PutImmRMByte(ModRM); CLK(1); } OP_EPILOGUE;
OP( 0xc7, i_mov_wd16 ) { GetModRM; PutImmRMWord(ModRM); CLK(1); } OP_EPILOGUE;

// NEC calls it "PREPARE"
OP( 0xc8, i_enter ) {
    uint32 nb = FETCH;
    uint32 i,level;

    CLK(19);
    nb += FETCH << 8;

    level = FETCH;
    level &= 0x1F; // Only lower 5 bits are valid on V30MZ

    PUSH(I.regs.w[BP]);
    I.regs.w[BP]=I.regs.w[SP];
    I.regs.w[SP] -= nb;
    for (i=1;i<level;i++) {
	PUSH(GetMemW(SS,I.regs.w[BP]-i*2));
        CLK(4);
    }
    if (level) PUSH(I.regs.w[BP]);
} OP_EPILOGUE;

OP( 0xc9, i_leave ) {
	I.regs.w[SP]=I.regs.w[BP];
	POP(I.regs.w[BP]);
	CLK(2);
} OP_EPILOGUE;

OP( 0xca, i_retf_d16  ) { uint32 count = FETCH; count += FETCH << 8; POP(I.pc); POP(I.sregs[PS]); I.regs.w[SP]+=count; CLK(9); ADDBRANCHTRACE(I.sregs[PS], I.pc); } OP_EPILOGUE;
OP( 0xcb, i_retf      ) { POP(I.pc); POP(I.sregs[PS]); CLK(8); ADDBRANCHTRACE(I.sregs[PS], I.pc); } OP_EPILOGUE;
OP( 0xcc, i_int3      ) { nec_interrupt(3); CLK(9); } OP_EPILOGUE;
OP( 0xcd, i_int       ) { nec_interrupt(FETCH); CLK(10); } OP_EPILOGUE;
OP( 0xce, i_into      ) { if (FLAG_O) { nec_interrupt(4); CLK(13); } else CLK(6); } OP_EPILOGUE;
OP( 0xcf, i_iret      ) { POP(I.pc); POP(I.sregs[PS]); i_real_popf(); CLK(10); ADDBRANCHTRACE(I.sregs[PS], I.pc); } OP_EPILOGUE;

OP( 0xd0, i_rotshft_b ) {
	uint32 src, dst; GetModRM; src = (uint32)GetRMByte(ModRM); dst=src;
	CLKM(3,1);
    switch (ModRM & 0x38) {
		case 0x00: ROL_uint8;  PutbackRMByte(ModRM,(uint8)dst); I.OverVal = (src^dst)&0x80; break;
		case 0x08: ROR_uint8;  PutbackRMByte(ModRM,(uint8)dst); I.OverVal = (src^dst)&0x80; break;
		case 0x10: ROLC_uint8; PutbackRMByte(ModRM,(uint8)dst); I.OverVal = (src^dst)&0x80; break;
		case 0x18: RORC_uint8; PutbackRMByte(ModRM,(uint8)dst); I.OverVal = (src^dst)&0x80; break;
		case 0x20: SHL_uint8(1); I.OverVal = (src^dst)&0x80;I.AuxVal = 1; break;
		case 0x28: SHR_uint8(1); I.OverVal = (src^dst)&0x80;I.AuxVal = 1; break;
		case 0x30:  break;
		case 0x38: SHRA_uint8(1); I.OverVal = 0; break;
	}
} OP_EPILOGUE;

OP( 0xd1, i_rotshft_w ) {
	uint32 src, dst; GetModRM; src = (uint32)GetRMWord(ModRM); dst=src;
	CLKM(3,1);
	switch (ModRM & 0x38) {
		case 0x00: ROL_uint16;  PutbackRMWord(ModRM,(uint16)dst); I.OverVal = (src^dst)&0x8000; break;
		case 0x08: ROR_uint16;  PutbackRMWord(ModRM,(uint16)dst); I.OverVal = (src^dst)&0x8000; break;
		case 0x10: ROLC_uint16; PutbackRMWord(ModRM,(uint16)dst); I.OverVal = (src^dst)&0x8000; break;
		case 0x18: RORC_uint16; PutbackRMWord(ModRM,(uint16)dst); I.OverVal = (src^dst)&0x8000; break;
		case 0x20: SHL_uint16(1); I.AuxVal = 1;I.OverVal = (src^dst)&0x8000;  break;
		case 0x28: SHR_uint16(1); I.AuxVal = 1;I.OverVal = (src^dst)&0x8000;  break;
		case 0x30: break;
		case 0x38: SHRA_uint16(1); I.AuxVal = 1;I.OverVal = 0; break;
	}
} OP_EPILOGUE;

OP( 0xd2, i_rotshft_bcl ) {
	uint32 src, dst; uint8 c; GetModRM; src = (uint32)GetRMByte(ModRM); dst=src;
	c=I.regs.b[CL];
	CLKM(5,3);
	c&=0x1f;
	if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_uint8;  c--; } while (c>0); PutbackRMByte(ModRM,(uint8)dst); break;
		case 0x08: do { ROR_uint8;  c--; } while (c>0); PutbackRMByte(ModRM,(uint8)dst); break;
		case 0x10: do { ROLC_uint8; c--; } while (c>0); PutbackRMByte(ModRM,(uint8)dst); break;
		case 0x18: do { RORC_uint8; c--; } while (c>0); PutbackRMByte(ModRM,(uint8)dst); break;
		case 0x20: SHL_uint8(c);	I.AuxVal = 1; break;
		case 0x28: SHR_uint8(c); I.AuxVal = 1;break;
		case 0x30: break;
		case 0x38: SHRA_uint8(c); break;
	}
} OP_EPILOGUE;

OP( 0xd3, i_rotshft_wcl ) {
	uint32 src, dst; uint8 c; GetModRM; src = (uint32)GetRMWord(ModRM); dst=src;
	c=I.regs.b[CL];
	c&=0x1f;
	CLKM(5,3);
    if (c) switch (ModRM & 0x38) {
		case 0x00: do { ROL_uint16;  c--; } while (c>0); PutbackRMWord(ModRM,(uint16)dst); break;
		case 0x08: do { ROR_uint16;  c--; } while (c>0); PutbackRMWord(ModRM,(uint16)dst); break;
		case 0x10: do { ROLC_uint16; c--; } while (c>0); PutbackRMWord(ModRM,(uint16)dst); break;
		case 0x18: do { RORC_uint16; c--; } while (c>0); PutbackRMWord(ModRM,(uint16)dst); break;
		case 0x20: SHL_uint16(c);	I.AuxVal = 1; break;
		case 0x28: SHR_uint16(c);	I.AuxVal = 1; break;
		case 0x30: break;
		case 0x38: SHRA_uint16(c); break;
	}
} OP_EPILOGUE;

OP( 0xd4, i_aam    ) { uint32 mult=FETCH; mult=0; I.regs.b[AH] = I.regs.b[AL] / 10; I.regs.b[AL] %= 10; SetSZPF_Word(I.regs.w[AW]); CLK(17); } OP_EPILOGUE;
OP( 0xd5, i_aad    ) { uint32 mult=FETCH; mult=0; I.regs.b[AL] = I.regs.b[AH] * 10 + I.regs.b[AL]; I.regs.b[AH] = 0; SetSZPF_Byte(I.regs.b[AL]); CLK(6); } OP_EPILOGUE;
OP( 0xd6, i_setalc ) { I.regs.b[AL] = (CF)?0xff:0x00; CLK(3);  } OP_EPILOGUE;
OP( 0xd7, i_trans  ) { uint32 dest = (I.regs.w[BW]+I.regs.b[AL])&0xffff; I.regs.b[AL] = GetMemB(DS0, dest); CLK(5); } OP_EPILOGUE;

OP_RANGE(0xd8, 0xdf, i_fpo) { /*printf("FPO1, Op:%02x\n", opcode);*/ GetModRM; CLK(1); } OP_EPILOGUE;

OP( 0xe0, i_loopne ) { int8 disp = (int8)FETCH; I.regs.w[CW]--; if (!ZF && I.regs.w[CW]) { I.pc = (uint16)(I.pc+disp);  CLK(6); ADDBRANCHTRACE(I.sregs[PS], I.pc); } else CLK(3); } OP_EPILOGUE;
OP( 0xe1, i_loope  ) { int8 disp = (int8)FETCH; I.regs.w[CW]--; if ( ZF && I.regs.w[CW]) { I.pc = (uint16)(I.pc+disp);  CLK(6); ADDBRANCHTRACE(I.sregs[PS], I.pc); } else CLK(3); } OP_EPILOGUE;
OP( 0xe2, i_loop   ) { int8 disp = (int8)FETCH; I.regs.w[CW]--; if (I.regs.w[CW]) { I.pc = (uint16)(I.pc+disp);  CLK(5); ADDBRANCHTRACE(I.sregs[PS], I.pc); } else CLK(2); } OP_EPILOGUE;
OP( 0xe3, i_jcxz   ) { int8 disp = (int8)FETCH; if (I.regs.w[CW] == 0) { I.pc = (uint16)(I.pc+disp);  CLK(4); ADDBRANCHTRACE(I.sregs[PS], I.pc); } else CLK(1); } OP_EPILOGUE;
OP( 0xe4, i_inal   ) { uint8 port = FETCH; I.regs.b[AL] = read_port(port); CLK(6);				     	} OP_EPILOGUE;
OP( 0xe5, i_inax   ) { uint8 port = FETCH; I.regs.b[AL] = read_port(port); I.regs.b[AH] = read_port(port+1); CLK(6); 	} OP_EPILOGUE;
OP( 0xe6, i_outal  ) { uint8 port = FETCH; write_port(port, I.regs.b[AL]); CLK(6);				     	} OP_EPILOGUE;
OP( 0xe7, i_outax  ) { uint8 port = FETCH; write_port(port, I.regs.b[AL]); write_port(port+1, I.regs.b[AH]); CLK(6);	} OP_EPILOGUE;

OP( 0xe8, i_call_d16 ) { uint32 tmp; FETCHuint16(tmp); PUSH(I.pc); I.pc = (uint16)(I.pc+(int16)tmp); ADDBRANCHTRACE(I.sregs[PS], I.pc); CLK(5); } OP_EPILOGUE;
OP( 0xe9, i_jmp_d16  ) { uint32 tmp; FETCHuint16(tmp); I.pc = (uint16)(I.pc+(int16)tmp); ADDBRANCHTRACE(I.sregs[PS], I.pc); CLK(4); } OP_EPILOGUE;
OP( 0xea, i_jmp_far  ) { uint32 tmp,tmp1; FETCHuint16(tmp); FETCHuint16(tmp1); I.sregs[PS] = (uint16)tmp1; I.pc = (uint16)tmp; ; ADDBRANCHTRACE(I.sregs[PS], I.pc); CLK(7);  } OP_EPILOGUE;
OP( 0xeb, i_jmp_d8   ) { int tmp = (int)((int8)FETCH); CLK(4);I.pc = (uint16)(I.pc+tmp); ADDBRANCHTRACE(I.sregs[PS], I.pc); } OP_EPILOGUE;

OP( 0xec, i_inaldx   ) { I.regs.b[AL] = read_port(I.regs.w[DW]); CLK(6);} OP_EPILOGUE;
OP( 0xed, i_inaxdx   ) { uint32 port = I.regs.w[DW];	I.regs.b[AL] = read_port(port);	I.regs.b[AH] = read_port(port+1); CLK(6); } OP_EPILOGUE;

OP( 0xee, i_outdxal  ) { write_port(I.regs.w[DW], I.regs.b[AL]); CLK(6); } OP_EPILOGUE;
OP( 0xef, i_outdxax  ) { uint32 port = I.regs.w[DW];	write_port(port, I.regs.b[AL]);	write_port(port+1, I.regs.b[AH]); CLK(6); } OP_EPILOGUE;

// NEC calls it "BUSLOCK"
OP( 0xf0, i_lock     ) { CLK(1); DoOP(FETCHOP); } OP_EPILOGUE;

// We put CHK_ICOUNT *after* the first iteration has completed, to match real behavior.
#define CHK_ICOUNT(cond) if(v30mz_ICount < 0 && (cond)) { I.pc -= seg_prefix ? 3 : 2; break; }

OP( 0xf2, i_repne    ) 
{ 
 uint32 next = FETCHOP; 

    switch(next) { /* Segments */
	    case 0x26:	seg_prefix=TRUE;	prefix_base=I.sregs[DS1]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x2e:	seg_prefix=TRUE;	prefix_base=I.sregs[PS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x36:	seg_prefix=TRUE;	prefix_base=I.sregs[SS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x3e:	seg_prefix=TRUE;	prefix_base=I.sregs[DS0]<<4;	next = FETCHOP;	CLK(2); break;
	}

    switch(next) {
	    case 0x6c:	CLK(5); if (I.regs.w[CW]) do { i_real_insb();  I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0x6d:  CLK(5); if (I.regs.w[CW]) do { i_real_insw();  I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0x6e:	CLK(5); if (I.regs.w[CW]) do { i_real_outsb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0x6f:  CLK(5); if (I.regs.w[CW]) do { i_real_outsw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xa4:	CLK(5); if (I.regs.w[CW]) do { i_real_movsb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xa5:  CLK(5); if (I.regs.w[CW]) do { i_real_movsw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xa6:	CLK(5); if (I.regs.w[CW]) do { i_real_cmpsb(); I.regs.w[CW]--; CLK(3); CHK_ICOUNT(I.regs.w[CW] && ZF == 0); /* 6 + 3 = 9 */ } while (I.regs.w[CW]>0 && ZF==0); break;
	    case 0xa7:	CLK(5); if (I.regs.w[CW]) do { i_real_cmpsw(); I.regs.w[CW]--; CLK(3); CHK_ICOUNT(I.regs.w[CW] && ZF == 0); /* 6 + 3 = 9 */ } while (I.regs.w[CW]>0 && ZF==0); break;
	    case 0xaa:	CLK(5); if (I.regs.w[CW]) do { i_real_stosb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xab:  CLK(5); if (I.regs.w[CW]) do { i_real_stosw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xac:	CLK(5); if (I.regs.w[CW]) do { i_real_lodsb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xad:  CLK(5); if (I.regs.w[CW]) do { i_real_lodsw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xae:	CLK(5); if (I.regs.w[CW]) do { i_real_scasb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW] && ZF == 0); } while (I.regs.w[CW]>0 && ZF==0); break;
	    case 0xaf:	CLK(5); if (I.regs.w[CW]) do { i_real_scasw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW] && ZF == 0); } while (I.regs.w[CW]>0 && ZF==0); break;
	    default: DoOP(next); break;
    }
	seg_prefix=FALSE;
} OP_EPILOGUE;

OP( 0xf3, i_repe) 
{ 
 uint32 next = FETCHOP;

    switch(next) { /* Segments */
	    case 0x26:	seg_prefix=TRUE;	prefix_base=I.sregs[DS1]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x2e:	seg_prefix=TRUE;	prefix_base=I.sregs[PS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x36:	seg_prefix=TRUE;	prefix_base=I.sregs[SS]<<4;	next = FETCHOP;	CLK(2); break;
	    case 0x3e:	seg_prefix=TRUE;	prefix_base=I.sregs[DS0]<<4;	next = FETCHOP;	CLK(2); break;
	}
    switch(next) {
	    case 0x6c:	CLK(5); if (I.regs.w[CW]) do { i_real_insb();  I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0x6d:  CLK(5); if (I.regs.w[CW]) do { i_real_insw();  I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0x6e:	CLK(5); if (I.regs.w[CW]) do { i_real_outsb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0x6f:  CLK(5); if (I.regs.w[CW]) do { i_real_outsw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xa4:	CLK(5); if (I.regs.w[CW]) do { i_real_movsb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xa5:  CLK(5); if (I.regs.w[CW]) do { i_real_movsw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xa6:	CLK(5); if (I.regs.w[CW]) do { i_real_cmpsb(); I.regs.w[CW]--; CLK(3); CHK_ICOUNT(I.regs.w[CW] && ZF == 1); /* 6 + 3 = 9 */ } while (I.regs.w[CW]>0 && ZF==1); break;
	    case 0xa7:	CLK(5); if (I.regs.w[CW]) do { i_real_cmpsw(); I.regs.w[CW]--; CLK(3); CHK_ICOUNT(I.regs.w[CW] && ZF == 1);/* 6 + 3 = 9 */ } while (I.regs.w[CW]>0 && ZF==1); break;
	    case 0xaa:	CLK(5); if (I.regs.w[CW]) do { i_real_stosb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xab:  CLK(5); if (I.regs.w[CW]) do { i_real_stosw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xac:	CLK(5); if (I.regs.w[CW]) do { i_real_lodsb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xad:  CLK(5); if (I.regs.w[CW]) do { i_real_lodsw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW]); } while (I.regs.w[CW]>0); break;
	    case 0xae:	CLK(5); if (I.regs.w[CW]) do { i_real_scasb(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW] && ZF == 1); } while (I.regs.w[CW]>0 && ZF==1); break;
	    case 0xaf:	CLK(5); if (I.regs.w[CW]) do { i_real_scasw(); I.regs.w[CW]--; CHK_ICOUNT(I.regs.w[CW] && ZF == 1); } while (I.regs.w[CW]>0 && ZF==1); break;
	    default: DoOP(next); break;
    }
	seg_prefix=FALSE;
} OP_EPILOGUE;

OP( 0xf4, i_hlt ) { InHLT = TRUE; CheckInHLT(); } OP_EPILOGUE;

OP( 0xf5, i_cmc ) { I.CarryVal = !CF; CLK(4); } OP_EPILOGUE;
OP( 0xf6, i_f6pre ) { uint32 tmp; uint32 uresult,uresult2; int32 result,result2;
	GetModRM; tmp = GetRMByte(ModRM);
    switch (ModRM & 0x38) {
		case 0x00: tmp &= FETCH; I.CarryVal = I.OverVal = I.AuxVal=0; SetSZPF_Byte(tmp); CLKM(2,1); break; /* TEST */
		case 0x08:  break;
 		case 0x10: PutbackRMByte(ModRM,~tmp); CLKM(3,1); break; /* NOT */
		
		case 0x18: I.CarryVal=(tmp!=0);tmp=(~tmp)+1; SetSZPF_Byte(tmp); PutbackRMByte(ModRM,tmp&0xff); CLKM(3,1); break; /* NEG */
		case 0x20: uresult = I.regs.b[AL]*tmp; I.regs.w[AW]=(uint16)uresult; I.CarryVal=I.OverVal=(I.regs.b[AH]!=0); CLKM(4,3); break; /* MULU */
		case 0x28: result = (int16)((int8)I.regs.b[AL])*(int16)((int8)tmp); I.regs.w[AW]=(uint16)result; I.CarryVal=I.OverVal=(I.regs.b[AH]!=0); CLKM(4,3); break; /* MUL */
		case 0x30: if (tmp) { DIVUB; } else nec_interrupt(0); CLKM(16,15); break;
		case 0x38: if (tmp) { DIVB;  } else nec_interrupt(0); CLKM(18,17); break;
   }
} OP_EPILOGUE;

OP( 0xf7, i_f7pre   ) { uint32 tmp,tmp2; uint32 uresult,uresult2; int32 result,result2;
	GetModRM; tmp = GetRMWord(ModRM);
    switch (ModRM & 0x38) {
		case 0x00: FETCHuint16(tmp2); tmp &= tmp2; I.CarryVal = I.OverVal = I.AuxVal=0; SetSZPF_Word(tmp); CLKM(2,1); break; /* TEST */
		case 0x08: break;
 		case 0x10: PutbackRMWord(ModRM,~tmp); CLKM(3,1); break; /* NOT */
		case 0x18: I.CarryVal=(tmp!=0); tmp=(~tmp)+1; SetSZPF_Word(tmp); PutbackRMWord(ModRM,tmp&0xffff); CLKM(3,1); break; /* NEG */
		case 0x20: uresult = I.regs.w[AW]*tmp; I.regs.w[AW]=uresult&0xffff; I.regs.w[DW]=((uint32)uresult)>>16; I.CarryVal=I.OverVal=(I.regs.w[DW]!=0); CLKM(4,3); break; /* MULU */
		case 0x28: result = (int32)((int16)I.regs.w[AW])*(int32)((int16)tmp); I.regs.w[AW]=result&0xffff; I.regs.w[DW]=result>>16; I.CarryVal=I.OverVal=(I.regs.w[DW]!=0); CLKM(4,3); break; /* MUL */
		case 0x30: if (tmp) { DIVUW; } else nec_interrupt(0); CLKM(24,23); break;
		case 0x38: if (tmp) { DIVW;  } else nec_interrupt(0); CLKM(25,24); break;
 	}
} OP_EPILOGUE;

OP( 0xf8, i_clc   ) { I.CarryVal = 0;	CLK(4);	} OP_EPILOGUE;
OP( 0xf9, i_stc   ) { I.CarryVal = 1;	CLK(4);	} OP_EPILOGUE;
OP( 0xfa, i_di    ) { SetIF(0);		CLK(4);	} OP_EPILOGUE;
OP( 0xfb, i_ei    ) { SetIF(1);		CLK(4);	} OP_EPILOGUE;
OP( 0xfc, i_cld   ) { SetDF(0);		CLK(4);	} OP_EPILOGUE;
OP( 0xfd, i_std   ) { SetDF(1);		CLK(4);	} OP_EPILOGUE;
OP( 0xfe, i_fepre ) { uint32 tmp, tmp1; GetModRM; tmp=GetRMByte(ModRM);
    switch(ModRM & 0x38) {
    	case 0x00: tmp1 = tmp+1; I.OverVal = (tmp==0x7f); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutbackRMByte(ModRM,(uint8)tmp1); CLKM(3,1); break; /* INC */
		case 0x08: tmp1 = tmp-1; I.OverVal = (tmp==0x80); SetAF(tmp1,tmp,1); SetSZPF_Byte(tmp1); PutbackRMByte(ModRM,(uint8)tmp1); CLKM(3,1); break; /* DEC */
	}
} OP_EPILOGUE;

OP( 0xff, i_ffpre ) { uint32 tmp, tmp1; GetModRM; tmp=GetRMWord(ModRM);
    switch(ModRM & 0x38) {
    	case 0x00: tmp1 = tmp+1; I.OverVal = (tmp==0x7fff); SetAF(tmp1,tmp,1); SetSZPF_Word(tmp1); PutbackRMWord(ModRM,(uint16)tmp1); CLKM(3,1); break; /* INC */
		case 0x08: tmp1 = tmp-1; I.OverVal = (tmp==0x8000); SetAF(tmp1,tmp,1); SetSZPF_Word(tmp1); PutbackRMWord(ModRM,(uint16)tmp1); CLKM(3,1); break; /* DEC */
		case 0x10: PUSH(I.pc);	I.pc = (uint16)tmp; ADDBRANCHTRACE(I.sregs[PS], I.pc); CLKM(6,5); break; /* CALL */
		case 0x18: tmp1 = I.sregs[PS]; I.sregs[PS] = GetnextRMWord; PUSH(tmp1); PUSH(I.pc); I.pc = tmp; ADDBRANCHTRACE(I.sregs[PS], I.pc); CLKM(12,1); break; /* CALL FAR */
		case 0x20: I.pc = tmp; ADDBRANCHTRACE(I.sregs[PS], I.pc); CLKM(5,4); break; /* JMP */
		case 0x28: I.pc = tmp; I.sregs[PS] = GetnextRMWord; ADDBRANCHTRACE(I.sregs[PS], I.pc); CLKM(10,1); break; /* JMP FAR */
		case 0x30: PUSH(tmp); CLKM(2,1); break;
	}
} OP_EPILOGUE;
} // End switch statement


} // End func


/*****************************************************************************/


unsigned v30mz_get_reg(int regnum)
{
	switch( regnum )
	{
		case NEC_PC: return I.pc;
		case NEC_SP: return I.regs.w[SP];
		case NEC_FLAGS: return CompressFlags();
	        case NEC_AW: return I.regs.w[AW];
		case NEC_CW: return I.regs.w[CW];
		case NEC_DW: return I.regs.w[DW];
		case NEC_BW: return I.regs.w[BW];
		case NEC_BP: return I.regs.w[BP];
		case NEC_IX: return I.regs.w[IX];
		case NEC_IY: return I.regs.w[IY];
		case NEC_DS1: return I.sregs[DS1];
		case NEC_PS: return I.sregs[PS];
		case NEC_SS: return I.sregs[SS];
		case NEC_DS0: return I.sregs[DS0];
	}
	return 0;
}

void nec_set_irq_line(int irqline, int state);

void v30mz_set_reg(int regnum, unsigned val)
{
	switch( regnum )
	{
		case NEC_PC: I.pc = val; break;
		case NEC_SP: I.regs.w[SP] = val; break;
		case NEC_FLAGS: ExpandFlags(val); break;
	        case NEC_AW: I.regs.w[AW] = val; break;
		case NEC_CW: I.regs.w[CW] = val; break;
		case NEC_DW: I.regs.w[DW] = val; break;
		case NEC_BW: I.regs.w[BW] = val; break;
		case NEC_BP: I.regs.w[BP] = val; break;
		case NEC_IX: I.regs.w[IX] = val; break;
		case NEC_IY: I.regs.w[IY] = val; break;
		case NEC_DS1: I.sregs[DS1] = val; break;
		case NEC_PS: I.sregs[PS] = val; break;
		case NEC_SS: I.sregs[SS] = val; break;
		case NEC_DS0: I.sregs[DS0] = val; break;
    }
}

#ifdef WANT_DEBUGGER
static void (*save_cpu_writemem20)(uint32,uint8);
static uint8 (*save_cpu_readport)(uint32);
static void (*save_cpu_writeport)(uint32, uint8);
static uint8 (*save_cpu_readmem20)(uint32);

static void test_cpu_writemem20(uint32 A, uint8 V)
{
 if(write_hook)
  write_hook(A, V);
}

static uint8 test_cpu_readmem20(uint32 A)
{
 if(read_hook)
  return(read_hook(A));
 else
  return(save_cpu_readmem20(A));
}

static void test_cpu_writeport(uint32 A, uint8 V)
{
 if(port_write_hook)
  port_write_hook(A, V);
}

static uint8 test_cpu_readport(uint32 A)
{
 if(port_read_hook)
  return(port_read_hook(A));
 else
  return(save_cpu_readport(A));
}
#endif

void v30mz_execute(int cycles)
{
 v30mz_ICount += cycles;

 if(InHLT)
 {
  SETOLDCSIP();
  WSwan_InterruptCheck();
  if(InHLT)
  {
   int32 tmp = v30mz_ICount;

   if(tmp > 0)
    CLK(tmp);

   #ifdef WANT_DEBUGGER
   if(cpu_hook)
    cpu_hook(I.pc);
   #endif
   return;
  }
 }

 while(v30mz_ICount > 0) 
 {
  SETOLDCSIP();

  WSwan_InterruptCheck();

  #ifdef WANT_DEBUGGER
  if(hookie_hickey)
  {
   uint32 save_timestamp = v30mz_timestamp;
   int32 save_ICount = v30mz_ICount;
   v30mz_regs_t save_I = I;
   uint32 save_prefix_base = prefix_base;
   char save_seg_prefix = seg_prefix;
   void (*save_branch_trace_hook)(uint16 from_CS, uint16 from_IP, uint16 to_CS, uint16 to_IP, bool interrupt) = branch_trace_hook;

   branch_trace_hook = NULL;

   save_cpu_writemem20 = cpu_writemem20;
   save_cpu_readport = cpu_readport;
   save_cpu_writeport = cpu_writeport;
   save_cpu_readmem20 = cpu_readmem20;

   cpu_writemem20 = test_cpu_writemem20;
   cpu_readmem20 = test_cpu_readmem20;
   cpu_writeport = test_cpu_writeport;
   cpu_readport = test_cpu_readport;

   DoOP(FETCHOP);

   branch_trace_hook = save_branch_trace_hook;
   v30mz_timestamp = save_timestamp;
   v30mz_ICount = save_ICount;
   I = save_I;
   prefix_base = save_prefix_base;
   seg_prefix = save_seg_prefix;
   cpu_readmem20 = save_cpu_readmem20;
   cpu_writemem20 = save_cpu_writemem20;
   cpu_readport = save_cpu_readport;
   cpu_writeport = save_cpu_writeport;
   InHLT = FALSE;
  }

  if(cpu_hook)
   cpu_hook(I.pc);
  #endif

  DoOP(FETCHOP);
 }

}

#ifdef WANT_DEBUGGER
void v30mz_debug(void (*CPUHook)(uint32), uint8 (*ReadHook)(uint32), void (*WriteHook)(uint32, uint8), uint8 (*PortReadHook)(uint32), 
	void (*PortWriteHook)(uint32, uint8), void (*BranchTraceHook)(uint16 from_CS, uint16 from_IP, uint16 to_CS, uint16 to_IP, bool interrupt))
{
 cpu_hook = CPUHook;
 read_hook = ReadHook;
 write_hook = WriteHook;
 port_read_hook = PortReadHook;
 port_write_hook = PortWriteHook;

 hookie_hickey = read_hook || write_hook || port_read_hook || port_write_hook;

 branch_trace_hook = BranchTraceHook;
}
#endif

int v30mz_StateAction(StateMem *sm, int load, int data_only)
{
 uint16 PSW;

 SFORMAT StateRegs[] =
 {
  SFVARN(I.pc, "IP"),
  SFARRAY16N(I.regs.w, 8, "regs"),
  SFARRAY16N(I.sregs, 4, "sregs"),

  SFVARN(v30mz_ICount, "ICount"),
  SFVARN(InHLT, "InHLT"),
  SFVARN(prefix_base, "prefix_base"),
  SFVARN(seg_prefix, "seg_prefix"),
  SFVAR(PSW),
  SFEND
 };

 PSW = CompressFlags();

 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "V30"))
  return(0);

 if(load)
 {
  ExpandFlags(PSW);
 }

 return(1);
}

}
