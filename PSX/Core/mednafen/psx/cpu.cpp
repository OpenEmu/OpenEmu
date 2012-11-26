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

#include "psx.h"
#include "cpu.h"

/* TODO
	Make sure load delays are correct.

	Consider preventing IRQs being taken while in a branch delay slot, to prevent potential problems with games that like to be too clever and perform
	un-restartable sequences of instructions.
*/

namespace MDFN_IEN_PSX
{


PS_CPU::PS_CPU()
{
 Halted = false;

 memset(FastMap, 0, sizeof(FastMap));
 memset(DummyPage, 0xFF, sizeof(DummyPage));	// 0xFF to trigger an illegal instruction exception, so we'll know what's up when debugging.

 for(uint64 a = 0x00000000; a < (1ULL << 32); a += FAST_MAP_PSIZE)
  SetFastMap(DummyPage, a, FAST_MAP_PSIZE);

 CPUHook = NULL;
 ADDBT = NULL;
}

PS_CPU::~PS_CPU()
{


}

void PS_CPU::SetFastMap(void *region_mem, uint32 region_address, uint32 region_size)
{
 // FAST_MAP_SHIFT
 // FAST_MAP_PSIZE

 for(uint64 A = region_address; A < (uint64)region_address + region_size; A += FAST_MAP_PSIZE)
 {
  FastMap[A >> FAST_MAP_SHIFT] = ((uint8 *)region_mem - region_address);
 }
}

INLINE void PS_CPU::RecalcIPCache(void)
{
 IPCache = 0;

 if((CP0.SR & CP0.CAUSE & 0xFF00) && (CP0.SR & 1))
  IPCache = 0x80;

 if(Halted)
  IPCache = 0x80;
}

void PS_CPU::SetHalt(bool status)
{
 Halted = status;
 RecalcIPCache();
}

void PS_CPU::Power(void)
{
 memset(GPR, 0, sizeof(GPR));
 memset(&CP0, 0, sizeof(CP0));
 LO = 0;
 HI = 0;

 gte_ts_done = 0;

 BACKED_PC = 0xBFC00000;
 BACKED_new_PC = 4;
 BACKED_new_PC_mask = ~0U;

 BACKED_LDWhich = 0x20;
 BACKED_LDValue = 0;
 
 CP0.SR |= (1 << 22);	// BEV
 CP0.SR |= (1 << 21);	// TS

 CP0.PRID = 0x300;	// PRId: FIXME(test on real thing)

 RecalcIPCache();

 GTE_Power();
}

int PS_CPU::StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY32(GPR, 32),
  SFVAR(LO),
  SFVAR(HI),
  SFVAR(BACKED_PC),
  SFVAR(BACKED_new_PC),
  SFVAR(BACKED_new_PC_mask),

  SFVAR(IPCache),
  SFVAR(Halted),

  SFVAR(BACKED_LDWhich),
  SFVAR(BACKED_LDValue),

  SFVAR(next_event_ts),
  SFVAR(gte_ts_done),

  SFARRAY32(CP0.Regs, 32),

  SFEND
 };
 int ret = MDFNSS_StateAction(sm, load, data_only, StateRegs, "CPU");

 ret &= GTE_StateAction(sm, load, data_only);

 if(load)
 {

 }

 return(ret);
}

void PS_CPU::AssertIRQ(int which, bool asserted)
{
 assert(which >= 0 && which <= 5);

 CP0.CAUSE &= ~(1 << (10 + which));

 if(asserted)
  CP0.CAUSE |= 1 << (10 + which);

 RecalcIPCache();
}

template<typename T>
INLINE T PS_CPU::ReadMemory(pscpu_timestamp_t &timestamp, uint32 address, bool DS24)
{
 T ret;

 timestamp += 2;
 //timestamp++;
 //assert(!(CP0.SR & 0x10000));

 if(sizeof(T) == 1)
  ret = PSX_MemRead8(timestamp, address);
 else if(sizeof(T) == 2)
  ret = PSX_MemRead16(timestamp, address);
 else
 {
  if(DS24)
   ret = PSX_MemRead24(timestamp, address) & 0xFFFFFF;
  else
   ret = PSX_MemRead32(timestamp, address);
 }

 return(ret);
}

template<typename T>
INLINE void PS_CPU::WriteMemory(pscpu_timestamp_t &timestamp, uint32 address, uint32 value, bool DS24)
{
 if(!(CP0.SR & 0x10000))
 {
  if(sizeof(T) == 1)
   PSX_MemWrite8(timestamp, address, value);
  else if(sizeof(T) == 2)
   PSX_MemWrite16(timestamp, address, value);
  else
  {
   if(DS24)
    PSX_MemWrite24(timestamp, address, value);
   else
    PSX_MemWrite32(timestamp, address, value);
  }
 }
 //else
 // printf("ISC WRITE%d %08x %08x\n", (int)sizeof(T), address, value);
}

uint32 PS_CPU::Exception(uint32 code, uint32 PC, const uint32 NPM)
{
 const bool InBDSlot = !(NPM & 0x3);
 uint32 handler = 0x80000080;

 assert(code < 16);

 if(code != EXCEPTION_INT && code != EXCEPTION_BP && code != EXCEPTION_SYSCALL)
 {
  printf("Exception: %08x @ PC=0x%08x(IBDS=%d) -- IPCache=0x%02x -- IPEND=0x%02x -- SR=0x%08x ; IRQC_Status=0x%04x -- IRQC_Mask=0x%04x\n", code, PC, InBDSlot, IPCache, (CP0.CAUSE >> 8) & 0xFF, CP0.SR,
	IRQ_GetRegister(IRQ_GSREG_STATUS, NULL, 0), IRQ_GetRegister(IRQ_GSREG_MASK, NULL, 0));
  //assert(0);
 }

 if(CP0.SR & (1 << 22))	// BEV
  handler = 0xBFC00180;

 CP0.EPC = PC;
 if(InBDSlot)
  CP0.EPC -= 4;

 if(ADDBT)
  ADDBT(PC, handler, true);

 // "Push" IEc and KUc(so that the new IEc and KUc are 0)
 CP0.SR = (CP0.SR & ~0x3F) | ((CP0.SR << 2) & 0x3F);

 // Setup cause register
 CP0.CAUSE &= 0x0000FF00;
 CP0.CAUSE |= code << 2;

 // If EPC was adjusted -= 4 because we were in a branch delay slot, set the bit.
 if(InBDSlot)
  CP0.CAUSE |= 0x80000000;

 RecalcIPCache();

 return(handler);
}

#define BACKING_TO_ACTIVE			\
	PC = BACKED_PC;				\
	new_PC = BACKED_new_PC;			\
	new_PC_mask = BACKED_new_PC_mask;	\
	LDWhich = BACKED_LDWhich;		\
	LDValue = BACKED_LDValue;

#define ACTIVE_TO_BACKING			\
	BACKED_PC = PC;				\
	BACKED_new_PC = new_PC;			\
	BACKED_new_PC_mask = new_PC_mask;	\
	BACKED_LDWhich = LDWhich;		\
	BACKED_LDValue = LDValue;


template<bool DebugMode, bool ILHMode>
pscpu_timestamp_t PS_CPU::RunReal(pscpu_timestamp_t timestamp_in)
{
 register pscpu_timestamp_t timestamp = timestamp_in;

 register uint32 PC;
 register uint32 new_PC;
 register uint32 new_PC_mask;
 register uint32 LDWhich;
 register uint32 LDValue;
 
 BACKING_TO_ACTIVE;

 do
 {
  //printf("Running: %d %d\n", timestamp, next_event_ts);
  while(timestamp < next_event_ts)
  {
   uint32 instr;
   uint32 opf;

   // Zero must be zero...until the Master Plan is enacted.
   GPR[0] = 0;

   if(DebugMode && CPUHook)
   {
    ACTIVE_TO_BACKING;

    CPUHook(PC);

    BACKING_TO_ACTIVE;
   }

   if(!ILHMode)
   {
    if(PC == 0xB0)
    {
     if(GPR[9] == 0x3D)
     {
      //if(GPR[4] == 'L')
      // DBG_Break();
      fputc(GPR[4], stderr);
      //if(GPR[4] == '\n')
      //{
      // fputc('%', stderr);
      // fputc(' ', stderr);
      //}
     }
    }
   }

/*
   if(PC == 0xB0)
   {
    if(GPR[9] != 0xB)
    PSX_WARNING("[BIOS] 0xB0 t1=0x%02x", GPR[9]);
   }
*/
   instr = LoadU32_LE((uint32 *)&FastMap[PC >> FAST_MAP_SHIFT][PC]);

   //printf("PC=%08x, SP=%08x - op=0x%02x - funct=0x%02x - instr=0x%08x\n", PC, GPR[29], instr >> 26, instr & 0x3F, instr);
   //for(int i = 0; i < 32; i++)
   // printf("%02x : %08x\n", i, GPR[i]);
   //printf("\n");

   opf = instr & 0x3F;

   if(instr & (0x3F << 26))
    opf = 0x40 | (instr >> 26);

   opf |= IPCache;

   timestamp++;

   #define DO_LDS() { GPR[LDWhich] = LDValue; LDWhich = 0x20; }
   #define BEGIN_OPF(name, arg_op, arg_funct) { op_##name: /*assert( ((arg_op) ? (0x40 | (arg_op)) : (arg_funct)) == opf); */
   #define END_OPF goto OpDone; }

   #define DO_BRANCH(offset, mask)			\
	{						\
	 if(ILHMode)					\
	 {								\
	  uint32 old_PC = PC;						\
	  PC = (PC & new_PC_mask) + new_PC;				\
	  if(old_PC == ((PC & (mask)) + (offset)))			\
	  {								\
	   if(*(uint32 *)&FastMap[PC >> FAST_MAP_SHIFT][PC] == 0)	\
	   {								\
	    if(next_event_ts > timestamp) /* Necessary since next_event_ts might be set to something like "0" to force a call to the event handler. */		\
	    {								\
	     timestamp = next_event_ts;					\
	    }								\
	   }								\
	  }								\
	 }						\
	 else						\
	  PC = (PC & new_PC_mask) + new_PC;		\
	 new_PC = (offset);				\
	 new_PC_mask = (mask) & ~3;			\
	 /* Lower bits of new_PC_mask being clear signifies being in a branch delay slot. (overloaded behavior for performance) */	\
							\
         if(DebugMode && ADDBT)                 	\
	 {						\
          ADDBT(PC, (PC & new_PC_mask) + new_PC, false);	\
	 }						\
	 goto SkipNPCStuff;				\
	}

   #define ITYPE uint32 rs __attribute__((unused)) = (instr >> 21) & 0x1F; uint32 rt __attribute__((unused)) = (instr >> 16) & 0x1F; int32 immediate = (int16)(instr & 0xFFFF); /*printf(" rs=%02x(%08x), rt=%02x(%08x), immediate=(%08x) ", rs, GPR[rs], rt, GPR[rt], immediate);*/
   #define ITYPE_ZE uint32 rs __attribute__((unused)) = (instr >> 21) & 0x1F; uint32 rt __attribute__((unused)) = (instr >> 16) & 0x1F; uint32 immediate = instr & 0xFFFF; /*printf(" rs=%02x(%08x), rt=%02x(%08x), immediate=(%08x) ", rs, GPR[rs], rt, GPR[rt], immediate);*/
   #define JTYPE uint32 target = instr & ((1 << 26) - 1); /*printf(" target=(%08x) ", target);*/
   #define RTYPE uint32 rs __attribute__((unused)) = (instr >> 21) & 0x1F; uint32 rt __attribute__((unused)) = (instr >> 16) & 0x1F; uint32 rd __attribute__((unused)) = (instr >> 11) & 0x1F; uint32 shamt __attribute__((unused)) = (instr >> 6) & 0x1F; /*printf(" rs=%02x(%08x), rt=%02x(%08x), rd=%02x(%08x) ", rs, GPR[rs], rt, GPR[rt], rd, GPR[rd]);*/

   static const void *const op_goto_table[256] =
   {
    &&op_SLL, &&op_ILL, &&op_SRL, &&op_SRA, &&op_SLLV, &&op_ILL, &&op_SRLV, &&op_SRAV,
    &&op_JR, &&op_JALR, &&op_ILL, &&op_ILL, &&op_SYSCALL, &&op_BREAK, &&op_ILL, &&op_ILL,
    &&op_MFHI, &&op_MTHI, &&op_MFLO, &&op_MTLO, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,
    &&op_MULT, &&op_MULTU, &&op_DIV, &&op_DIVU, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,
    &&op_ADD, &&op_ADDU, &&op_SUB, &&op_SUBU, &&op_AND, &&op_OR, &&op_XOR, &&op_NOR,
    &&op_ILL, &&op_ILL, &&op_SLT, &&op_SLTU, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,
    &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,
    &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,

    NULL, &&op_BCOND, &&op_J, &&op_JAL, &&op_BEQ, &&op_BNE, &&op_BLEZ, &&op_BGTZ,
    &&op_ADDI, &&op_ADDIU, &&op_SLTI, &&op_SLTIU, &&op_ANDI, &&op_ORI, &&op_XORI, &&op_LUI,
    &&op_COP0, &&op_COP1, &&op_COP2, &&op_COP3, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,
    &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,
    &&op_LB, &&op_LH, &&op_LWL, &&op_LW, &&op_LBU, &&op_LHU, &&op_LWR, &&op_ILL,
    &&op_SB, &&op_SH, &&op_SWL, &&op_SW, &&op_ILL, &&op_ILL, &&op_SWR, &&op_ILL,
    &&op_LWC0, &&op_LWC1, &&op_LWC2, &&op_LWC3, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,
    &&op_SWC0, &&op_SWC1, &&op_SWC2, &&op_SWC3, &&op_ILL, &&op_ILL, &&op_ILL, &&op_ILL,

    // Interrupt portion of this table is constructed so that an interrupt won't be taken when the PC is pointing to a GTE instruction,
    // to avoid problems caused by pipeline vs coprocessor nuances that aren't emulated.
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,

    NULL, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_COP2, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
    &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT, &&op_INTERRUPT,
   };

   goto *op_goto_table[opf];

   {
    BEGIN_OPF(ILL, 0, 0);
	     PSX_WARNING("[CPU] Unknown instruction @%08x = %08x, op=%02x, funct=%02x", PC, instr, instr >> 26, (instr & 0x3F));
	     DO_LDS();
	     new_PC = Exception(EXCEPTION_RI, PC, new_PC_mask);
	     new_PC_mask = 0;
    END_OPF;

    //
    // ADD - Add Word
    //
    BEGIN_OPF(ADD, 0, 0x20);
	RTYPE;
	uint32 result = GPR[rs] + GPR[rt];
	bool ep = ((~(GPR[rs] ^ GPR[rt])) & (GPR[rs] ^ result)) & 0x80000000;

	DO_LDS();

	if(ep)
	{
	 new_PC = Exception(EXCEPTION_OV, PC, new_PC_mask);
         new_PC_mask = 0;
	}
	else
	 GPR[rd] = result;

    END_OPF;

    //
    // ADDI - Add Immediate Word
    //
    BEGIN_OPF(ADDI, 0x08, 0);
	ITYPE;
        uint32 result = GPR[rs] + immediate;
	bool ep = ((~(GPR[rs] ^ immediate)) & (GPR[rs] ^ result)) & 0x80000000;

	DO_LDS();

        if(ep)
	{
	 new_PC = Exception(EXCEPTION_OV, PC, new_PC_mask);
         new_PC_mask = 0;
	}
        else
         GPR[rt] = result;

    END_OPF;

    //
    // ADDIU - Add Immediate Unsigned Word
    //
    BEGIN_OPF(ADDIU, 0x09, 0);
	ITYPE;
	uint32 result = GPR[rs] + immediate;

	DO_LDS();

	GPR[rt] = result;

    END_OPF;

    //
    // ADDU - Add Unsigned Word
    //
    BEGIN_OPF(ADDU, 0, 0x21);
	RTYPE;
	uint32 result = GPR[rs] + GPR[rt];

	DO_LDS();

	GPR[rd] = result;

    END_OPF;

    //
    // AND - And
    //
    BEGIN_OPF(AND, 0, 0x24);
	RTYPE;
	uint32 result = GPR[rs] & GPR[rt];

	DO_LDS();

	GPR[rd] = result;

    END_OPF;

    //
    // ANDI - And Immediate
    //
    BEGIN_OPF(ANDI, 0x0C, 0);
	ITYPE_ZE;
	uint32 result = GPR[rs] & immediate;

	DO_LDS();

	GPR[rt] = result;

    END_OPF;

    //
    // BEQ - Branch on Equal
    //
    BEGIN_OPF(BEQ, 0x04, 0);
	ITYPE;
	bool result = (GPR[rs] == GPR[rt]);

	DO_LDS();

	if(result)
	{
	 DO_BRANCH((immediate << 2), ~0U);
	}
    END_OPF;

    // Bah, why does MIPS encoding have to be funky like this. :(
    // Handles BGEZ, BGEZAL, BLTZ, BLTZAL
    BEGIN_OPF(BCOND, 0x01, 0);
	const uint32 tv = GPR[(instr >> 21) & 0x1F];
	uint32 riv = (instr >> 16) & 0x1F;
	int32 immediate = (int16)(instr & 0xFFFF);
	bool result = (int32)(tv ^ (riv << 31)) < 0;
	//if(riv & ~(0x11))
	 //PSX_WARNING("[CPU] Unknown instruction %08x, op=%02x, funct=%02x", instr, instr >> 26, (instr & 0x3F))

	DO_LDS();

	if(riv & 0x10)	// Unconditional link reg setting.
	 GPR[31] = PC + 8;

        if(result)
	{
	 DO_BRANCH((immediate << 2), ~0U);
	}

    END_OPF;


    //
    // BGTZ - Branch on Greater than Zero
    //
    BEGIN_OPF(BGTZ, 0x07, 0);
	ITYPE;
	bool result = (int32)GPR[rs] > 0;

	DO_LDS();

	if(result)
	{
	 DO_BRANCH((immediate << 2), ~0U);
	}
    END_OPF;

    //
    // BLEZ - Branch on Less Than or Equal to Zero
    //
    BEGIN_OPF(BLEZ, 0x06, 0);
	ITYPE;
	bool result = (int32)GPR[rs] <= 0;

	DO_LDS();

	if(result)
	{
	 DO_BRANCH((immediate << 2), ~0U);
	}

    END_OPF;

    //
    // BNE - Branch on Not Equal
    //
    BEGIN_OPF(BNE, 0x05, 0);
	ITYPE;
	bool result = GPR[rs] != GPR[rt];

	DO_LDS();

	if(result)
	{
	 DO_BRANCH((immediate << 2), ~0U);
	}

    END_OPF;

    //
    // BREAK - Breakpoint
    //
    BEGIN_OPF(BREAK, 0, 0x0D);
	PSX_WARNING("[CPU] BREAK BREAK BREAK BREAK DAAANCE -- PC=0x%08x", PC);

	DO_LDS();
	new_PC = Exception(EXCEPTION_BP, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;

    // Cop "instructions":	CFCz(no CP0), COPz, CTCz(no CP0), LWCz(no CP0), MFCz, MTCz, SWCz(no CP0)
    //
    // COP0 instructions
    BEGIN_OPF(COP0, 0x10, 0);
	uint32 sub_op = (instr >> 21) & 0x1F;

	if(sub_op & 0x10)
	 sub_op = 0x10 + (instr & 0x3F);

	//printf("COP0 thing: %02x\n", sub_op);
	switch(sub_op)
	{
	 default:
		DO_LDS();
		break;

	 case 0x00:		// MFC0	- Move from Coprocessor
		{
		 uint32 rt = (instr >> 16) & 0x1F;
		 uint32 rd = (instr >> 11) & 0x1F;

		 //printf("MFC0: rt=%d <- rd=%d(%08x)\n", rt, rd, CP0.Regs[rd]);
		 DO_LDS();

		 LDWhich = rt;
		 LDValue = CP0.Regs[rd];
		}
		break;

	 case 0x04:		// MTC0	- Move to Coprocessor
		{
		 uint32 rt = (instr >> 16) & 0x1F;
		 uint32 rd = (instr >> 11) & 0x1F;
		 uint32 val = GPR[rt];

		 if(rd != CP0REG_CAUSE && rd != CP0REG_SR && val)
		 {
	 	  PSX_WARNING("[CPU] Unimplemented MTC0: rt=%d(%08x) -> rd=%d", rt, GPR[rt], rd);
		 }

		 switch(rd)
		 {
		  case CP0REG_BPC:
			CP0.BPC = val;
			break;

		  case CP0REG_BDA:
			CP0.BDA = val;
			break;

		  case CP0REG_TAR:
			CP0.TAR = val;
			break;

		  case CP0REG_DCIC:
			CP0.DCIC = val & 0xFF80003F;
			break;

  		  case CP0REG_BDAM:
			CP0.BDAM = val;
			break;

  		  case CP0REG_BPCM:
			CP0.BPCM = val;
			break;

		  case CP0REG_CAUSE:
			CP0.CAUSE &= ~(0x3 << 8);
			CP0.CAUSE |= val & (0x3 << 8);
			RecalcIPCache();
			break;

		  case CP0REG_SR:
			CP0.SR = val & ~( (0x3 << 26) | (0x3 << 23) | (0x3 << 6));
			RecalcIPCache();

			if(CP0.SR & 0x10000)
			 PSX_WARNING("[CPU] IsC set");
			break;
		 }
		}
		DO_LDS();
		break;

	 case (0x10 + 0x10):	// RFE
		// "Pop"
		DO_LDS();
		CP0.SR = (CP0.SR & ~0x0F) | ((CP0.SR >> 2) & 0x0F);
		RecalcIPCache();
		break;
	}
    END_OPF;

    //
    // COP1
    //
    BEGIN_OPF(COP1, 0x11, 0);
	DO_LDS();
        new_PC = Exception(EXCEPTION_COPU, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;

    //
    // COP2
    //
    BEGIN_OPF(COP2, 0x12, 0);
	uint32 sub_op = (instr >> 21) & 0x1F;

	switch(sub_op)
	{
	 default:
		DO_LDS();
		break;

	 case 0x00:		// MFC2	- Move from Coprocessor
		{
		 uint32 rt = (instr >> 16) & 0x1F;
		 uint32 rd = (instr >> 11) & 0x1F;

	         if(timestamp < gte_ts_done)
	          timestamp = gte_ts_done;

 	 	 DO_LDS();

		 LDWhich = rt;
		 LDValue = GTE_ReadDR(rd);
		}
		break;

	 case 0x04:		// MTC2	- Move to Coprocessor
		{
		 uint32 rt = (instr >> 16) & 0x1F;
		 uint32 rd = (instr >> 11) & 0x1F;
		 uint32 val = GPR[rt];

	         if(timestamp < gte_ts_done)
	          timestamp = gte_ts_done;

		 //printf("GTE WriteDR: %d %d\n", rd, val);
		 GTE_WriteDR(rd, val);
	         DO_LDS();
		}
		break;

	 case 0x02:		// CFC2
		{
		 uint32 rt = (instr >> 16) & 0x1F;
		 uint32 rd = (instr >> 11) & 0x1F;

	         if(timestamp < gte_ts_done)
	          timestamp = gte_ts_done;

	 	 DO_LDS();

		 LDWhich = rt;
		 LDValue = GTE_ReadCR(rd);
		//printf("GTE ReadCR: %d %d\n", rd, GPR[rt]);
		}		
		break;

	 case 0x06:		// CTC2
		{
		 uint32 rt = (instr >> 16) & 0x1F;
		 uint32 rd = (instr >> 11) & 0x1F;
		 uint32 val = GPR[rt];

		//printf("GTE WriteCR: %d %d\n", rd, val);

 	         if(timestamp < gte_ts_done)
	          timestamp = gte_ts_done;

		 GTE_WriteCR(rd, val);		 
	 	 DO_LDS();
		}
		break;

	 case 0x10 ... 0x1F:
		//printf("%08x\n", PC);
	        if(timestamp < gte_ts_done)
	         timestamp = gte_ts_done;
		gte_ts_done = timestamp + GTE_Instruction(instr);
		DO_LDS();
		break;
	}
    END_OPF;

    //
    // COP3
    //
    BEGIN_OPF(COP3, 0x13, 0);
	DO_LDS();
        new_PC = Exception(EXCEPTION_COPU, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;

    //
    // LWC0
    //
    BEGIN_OPF(LWC0, 0x30, 0);
	DO_LDS();
        new_PC = Exception(EXCEPTION_COPU, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;

    //
    // LWC1
    //
    BEGIN_OPF(LWC1, 0x31, 0);
	DO_LDS();
        new_PC = Exception(EXCEPTION_COPU, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;

    //
    // LWC2
    //
    BEGIN_OPF(LWC2, 0x32, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        if(address & 3)
	{
         new_PC = Exception(EXCEPTION_ADEL, PC, new_PC_mask);
         new_PC_mask = 0;
	}
        else
	{
         if(timestamp < gte_ts_done)
          timestamp = gte_ts_done;

         GTE_WriteDR(rt, PSX_MemRead32(timestamp, address));
	}
	DO_LDS();
	// GTE stuff here
    END_OPF;

    //
    // LWC3
    //
    BEGIN_OPF(LWC3, 0x33, 0);
	DO_LDS();
        new_PC = Exception(EXCEPTION_COPU, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;


    //
    // SWC0
    //
    BEGIN_OPF(SWC0, 0x38, 0);
	DO_LDS();
	new_PC = Exception(EXCEPTION_COPU, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;

    //
    // SWC1
    //
    BEGIN_OPF(SWC1, 0x39, 0);
	DO_LDS();
        new_PC = Exception(EXCEPTION_COPU, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;

    //
    // SWC2
    //
    BEGIN_OPF(SWC2, 0x3A, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	if(address & 0x3)
	{
	 new_PC = Exception(EXCEPTION_ADES, PC, new_PC_mask);
         new_PC_mask = 0;
	}
	else if(!(CP0.SR & 0x10000))
	{
         if(timestamp < gte_ts_done)
          timestamp = gte_ts_done;

	 PSX_MemWrite32(timestamp, address, GTE_ReadDR(rt));
	}
	DO_LDS();
    END_OPF;

    //
    // SWC3
    ///
    BEGIN_OPF(SWC3, 0x3B, 0);
	DO_LDS();
        new_PC = Exception(EXCEPTION_RI, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;


    //
    // DIV - Divide Word
    //
    BEGIN_OPF(DIV, 0, 0x1A);
	RTYPE;

        if(!GPR[rt])
        {
         //PSX_WARNING("[CPU] Division(signed) by zero at PC=%08x", PC);

	 if(GPR[rs] & 0x80000000)
	  LO = 1;
	 else
	  LO = 0xFFFFFFFF;

	 HI = GPR[rs];
        }
	else if(GPR[rs] == 0x80000000 && GPR[rt] == 0xFFFFFFFF)
	{
	 LO = 0x80000000;
	 HI = 0;
	}
        else
        {
         LO = (int32)GPR[rs] / (int32)GPR[rt];
         HI = (int32)GPR[rs] % (int32)GPR[rt];
        }

	DO_LDS();

    END_OPF;


    //
    // DIVU - Divide Unsigned Word
    //
    BEGIN_OPF(DIVU, 0, 0x1B);
	RTYPE;

	if(!GPR[rt])
	{
	 //PSX_WARNING("[CPU] Division(unsigned) by zero at PC=%08x", PC);

	 LO = 0xFFFFFFFF;
	 HI = GPR[rs];
	}
	else
	{
	 LO = GPR[rs] / GPR[rt];
	 HI = GPR[rs] % GPR[rt];
	}

	DO_LDS();
    END_OPF;

    //
    // J - Jump
    //
    BEGIN_OPF(J, 0x02, 0);
	JTYPE;

	DO_LDS();

	DO_BRANCH(target << 2, 0xF0000000);
    END_OPF;

    //
    // JAL - Jump and Link
    //
    BEGIN_OPF(JAL, 0x03, 0);
	JTYPE;

	DO_LDS();

	GPR[31] = PC + 8;

	DO_BRANCH(target << 2, 0xF0000000);
    END_OPF;

    //
    // JALR - Jump and Link Register
    //
    BEGIN_OPF(JALR, 0, 0x09);
	RTYPE;
	uint32 tmp = GPR[rs];

	DO_LDS();

	GPR[rd] = PC + 8;

	DO_BRANCH(tmp, 0);

    END_OPF;

    //
    // JR - Jump Register
    //
    BEGIN_OPF(JR, 0, 0x08);
	RTYPE;
	uint32 bt = GPR[rs];

	DO_LDS();

	DO_BRANCH(bt, 0);

    END_OPF;

    //
    // LUI - Load Upper Immediate
    //
    BEGIN_OPF(LUI, 0x0F, 0);
	ITYPE_ZE;		// Actually, probably would be sign-extending...if we were emulating a 64-bit MIPS chip :b

	DO_LDS();

	GPR[rt] = immediate << 16;

    END_OPF;

    //
    // MFHI - Move from HI
    //
    BEGIN_OPF(MFHI, 0, 0x10);
	RTYPE;

	DO_LDS();

	GPR[rd] = HI;

    END_OPF;


    //
    // MFLO - Move from LO
    //
    BEGIN_OPF(MFLO, 0, 0x12);
	RTYPE;

	DO_LDS();

	GPR[rd] = LO;

    END_OPF;


    //
    // MTHI - Move to HI
    //
    BEGIN_OPF(MTHI, 0, 0x11);
	RTYPE;

	HI = GPR[rs];

	DO_LDS();

    END_OPF;

    //
    // MTLO - Move to LO
    //
    BEGIN_OPF(MTLO, 0, 0x13);
	RTYPE;

	LO = GPR[rs];

	DO_LDS();

    END_OPF;


    //
    // MULT - Multiply Word
    //
    BEGIN_OPF(MULT, 0, 0x18);
	RTYPE;
	uint64 result;

	result = (int64)(int32)GPR[rs] * (int32)GPR[rt];

	DO_LDS();

	LO = result;
	HI = result >> 32;

    END_OPF;

    //
    // MULTU - Multiply Unsigned Word
    //
    BEGIN_OPF(MULTU, 0, 0x19);
	RTYPE;
	uint64 result;

	result = (uint64)GPR[rs] * GPR[rt];

	DO_LDS();

	LO = result;
	HI = result >> 32;

    END_OPF;


    //
    // NOR - NOR
    //
    BEGIN_OPF(NOR, 0, 0x27);
	RTYPE;
	uint32 result = ~(GPR[rs] | GPR[rt]);

	DO_LDS();

	GPR[rd] = result;

    END_OPF;

    //
    // OR - OR
    //
    BEGIN_OPF(OR, 0, 0x25);
	RTYPE;
	uint32 result = GPR[rs] | GPR[rt];

	DO_LDS();

	GPR[rd] = result;

    END_OPF;


    //
    // ORI - OR Immediate
    //
    BEGIN_OPF(ORI, 0x0D, 0);
	ITYPE_ZE;
	uint32 result = GPR[rs] | immediate;

	DO_LDS();

	GPR[rt] = result;

    END_OPF;


    //
    // SLL - Shift Word Left Logical
    //
    BEGIN_OPF(SLL, 0, 0x00);	// SLL
	RTYPE;
	uint32 result = GPR[rt] << shamt;

	DO_LDS();

	GPR[rd] = result;

    END_OPF;


    //
    // SLLV - Shift Word Left Logical Variable
    //
    BEGIN_OPF(SLLV, 0, 0x04);
	RTYPE;
	uint32 result = GPR[rt] << (GPR[rs] & 0x1F);

	DO_LDS();

	GPR[rd] = result;

    END_OPF;

    //
    // SLT - Set on Less Than
    //
    BEGIN_OPF(SLT, 0, 0x2A);
	RTYPE;
	uint32 result = (bool)((int32)GPR[rs] < (int32)GPR[rt]);

	DO_LDS();

	GPR[rd] = result;

    END_OPF;


    //
    // SLTI - Set on Less Than Immediate
    //
    BEGIN_OPF(SLTI, 0x0A, 0);
	ITYPE;
	uint32 result = (bool)((int32)GPR[rs] < immediate);

	DO_LDS();

	GPR[rt] = result;

    END_OPF;


    //
    // SLTIU - Set on Less Than Immediate, Unsigned
    //
    BEGIN_OPF(SLTIU, 0x0B, 0);
	ITYPE;
	uint32 result = (bool)(GPR[rs] < (uint32)immediate);

	DO_LDS();

	GPR[rt] = result;

    END_OPF;


    //
    // SLTU - Set on Less Than, Unsigned
    //
    BEGIN_OPF(SLTU, 0, 0x2B);
	RTYPE;
	uint32 result = (bool)(GPR[rs] < GPR[rt]);

	DO_LDS();

	GPR[rd] = result;

    END_OPF;


    //
    // SRA - Shift Word Right Arithmetic
    //
    BEGIN_OPF(SRA, 0, 0x03);
	RTYPE;
	uint32 result = ((int32)GPR[rt]) >> shamt;

	DO_LDS();

	GPR[rd] = result;

    END_OPF;


    //
    // SRAV - Shift Word Right Arithmetic Variable
    //
    BEGIN_OPF(SRAV, 0, 0x07);
	RTYPE;
	uint32 result = ((int32)GPR[rt]) >> (GPR[rs] & 0x1F);

	DO_LDS();

	GPR[rd] = result;

    END_OPF;


    //
    // SRL - Shift Word Right Logical
    //
    BEGIN_OPF(SRL, 0, 0x02);
	RTYPE;
	uint32 result = GPR[rt] >> shamt;

	DO_LDS();

	GPR[rd] = result;

    END_OPF;

    //
    // SRLV - Shift Word Right Logical Variable
    //
    BEGIN_OPF(SRLV, 0, 0x06);
	RTYPE;
	uint32 result = GPR[rt] >> (GPR[rs] & 0x1F);

	DO_LDS();

	GPR[rd] = result;

    END_OPF;


    //
    // SUB - Subtract Word
    //
    BEGIN_OPF(SUB, 0, 0x22);
	RTYPE;
	uint32 result = GPR[rs] - GPR[rt];
	bool ep = (((GPR[rs] ^ GPR[rt])) & (GPR[rs] ^ result)) & 0x80000000;

	DO_LDS();

	if(ep)
	{
	 new_PC = Exception(EXCEPTION_OV, PC, new_PC_mask);
         new_PC_mask = 0;
	}
	else
	 GPR[rd] = result;

    END_OPF;


    //
    // SUBU - Subtract Unsigned Word
    //
    BEGIN_OPF(SUBU, 0, 0x23); // SUBU
	RTYPE;
	uint32 result = GPR[rs] - GPR[rt];

	DO_LDS();

	GPR[rd] = result;

    END_OPF;


    //
    // SYSCALL
    //
    BEGIN_OPF(SYSCALL, 0, 0x0C);
	DO_LDS();

	new_PC = Exception(EXCEPTION_SYSCALL, PC, new_PC_mask);
        new_PC_mask = 0;
    END_OPF;


    //
    // XOR
    //
    BEGIN_OPF(XOR, 0, 0x26);
	RTYPE;
	uint32 result = GPR[rs] ^ GPR[rt];

	DO_LDS();

	GPR[rd] = result;

    END_OPF;

    //
    // XORI - Exclusive OR Immediate
    //
    BEGIN_OPF(XORI, 0x0E, 0);
	ITYPE_ZE;
	uint32 result = GPR[rs] ^ immediate;

	DO_LDS();

	GPR[rt] = result;
    END_OPF;

    //
    // Memory access instructions(besides the coprocessor ones) follow:
    //

    //
    // LB - Load Byte
    //
    BEGIN_OPF(LB, 0x20, 0);
	ITYPE;
	uint32 address = GPR[rs] + immediate;

	DO_LDS();

	LDWhich = rt;
	LDValue = (int32)ReadMemory<int8>(timestamp, address);
    END_OPF;

    //
    // LBU - Load Byte Unsigned
    //
    BEGIN_OPF(LBU, 0x24, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	DO_LDS();

        LDWhich = rt;
	LDValue = ReadMemory<uint8>(timestamp, address);
    END_OPF;

    //
    // LH - Load Halfword
    //
    BEGIN_OPF(LH, 0x21, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	DO_LDS();

	if(address & 1)
	{
	 new_PC = Exception(EXCEPTION_ADEL, PC, new_PC_mask);
         new_PC_mask = 0;
	}
	else
	{
	 LDWhich = rt;
         LDValue = (int32)ReadMemory<int16>(timestamp, address);
	}
    END_OPF;

    //
    // LHU - Load Halfword Unsigned
    //
    BEGIN_OPF(LHU, 0x25, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	DO_LDS();

        if(address & 1)
	{
         new_PC = Exception(EXCEPTION_ADEL, PC, new_PC_mask);
         new_PC_mask = 0;
	}
	else
	{
	 LDWhich = rt;
         LDValue = ReadMemory<uint16>(timestamp, address);
	}
    END_OPF;


    //
    // LW - Load Word
    //
    BEGIN_OPF(LW, 0x23, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	DO_LDS();

        if(address & 3)
	{
         new_PC = Exception(EXCEPTION_ADEL, PC, new_PC_mask);
         new_PC_mask = 0;
	}
        else
	{
	 LDWhich = rt;
         LDValue = ReadMemory<uint32>(timestamp, address);
	}
    END_OPF;

    //
    // SB - Store Byte
    //
    BEGIN_OPF(SB, 0x28, 0);
	ITYPE;
	uint32 address = GPR[rs] + immediate;

	WriteMemory<uint8>(timestamp, address, GPR[rt]);

	DO_LDS();
    END_OPF;

    // 
    // SH - Store Halfword
    //
    BEGIN_OPF(SH, 0x29, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	if(address & 0x1)
	{
	 new_PC = Exception(EXCEPTION_ADES, PC, new_PC_mask);
         new_PC_mask = 0;
	}
	else
	 WriteMemory<uint16>(timestamp, address, GPR[rt]);

	DO_LDS();
    END_OPF;

    // 
    // SW - Store Word
    //
    BEGIN_OPF(SW, 0x2B, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	if(address & 0x3)
	{
	 new_PC = Exception(EXCEPTION_ADES, PC, new_PC_mask);
         new_PC_mask = 0;
	}
	else
	 WriteMemory<uint32>(timestamp, address, GPR[rt]);

	DO_LDS();
    END_OPF;

    // LWL and LWR load delay slot tomfoolery appears to apply even to MFC0! (and probably MFCn and CFCn as well, though they weren't explicitly tested)

    //
    // LWL - Load Word Left
    //
    BEGIN_OPF(LWL, 0x22, 0);
	ITYPE;
	uint32 address = GPR[rs] + immediate;
	uint32 v = GPR[rt];

	if(LDWhich == rt)
	{
	 v = LDValue;
	}
	else
	{
	 DO_LDS();
	}

	LDWhich = rt;
	switch(address & 0x3)
	{
	 case 0: LDValue = (v & ~(0xFF << 24)) | (ReadMemory<uint8>(timestamp, address & ~3) << 24);
		 break;

	 case 1: LDValue = (v & ~(0xFFFF << 16)) | (ReadMemory<uint16>(timestamp, address & ~3) << 16);
	         break;

	 case 2: LDValue = (v & ~(0xFFFFFF << 8)) | (ReadMemory<uint32>(timestamp, address & ~3, true) << 8);
		 break;

	 case 3: LDValue = (v & ~(0xFFFFFFFF << 0)) | (ReadMemory<uint32>(timestamp, address & ~3) << 0);
		 break;
	}
    END_OPF;

    //
    // SWL - Store Word Left
    //
    BEGIN_OPF(SWL, 0x2A, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	switch(address & 0x3)
	{
	 case 0: WriteMemory<uint8>(timestamp, address & ~3, GPR[rt] >> 24);
		 break;

	 case 1: WriteMemory<uint16>(timestamp, address & ~3, GPR[rt] >> 16);
	         break;

	 case 2: WriteMemory<uint32>(timestamp, address & ~3, GPR[rt] >> 8, true);
		 break;

	 case 3: WriteMemory<uint32>(timestamp, address & ~3, GPR[rt] >> 0);
		 break;
	}
	DO_LDS();

    END_OPF;

    //
    // LWR - Load Word Right
    //
    BEGIN_OPF(LWR, 0x26, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;
	uint32 v = GPR[rt];

	if(LDWhich == rt)
	{
	 v = LDValue;
	}
	else
	{
	 DO_LDS();
	}

	LDWhich = rt;
	switch(address & 0x3)
	{
	 case 0: LDValue = (v & ~(0xFFFFFFFF)) | ReadMemory<uint32>(timestamp, address);
		 break;

	 case 1: LDValue = (v & ~(0xFFFFFF)) | ReadMemory<uint32>(timestamp, address, true);
		 break;

	 case 2: LDValue = (v & ~(0xFFFF)) | ReadMemory<uint16>(timestamp, address);
	         break;

	 case 3: LDValue = (v & ~(0xFF)) | ReadMemory<uint8>(timestamp, address);
		 break;
	}
    END_OPF;

    //
    // SWR - Store Word Right
    //
    BEGIN_OPF(SWR, 0x2E, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	switch(address & 0x3)
	{
	 case 0: WriteMemory<uint32>(timestamp, address, GPR[rt]);
		 break;

	 case 1: WriteMemory<uint32>(timestamp, address, GPR[rt], true);
		 break;

	 case 2: WriteMemory<uint16>(timestamp, address, GPR[rt]);
	         break;

	 case 3: WriteMemory<uint8>(timestamp, address, GPR[rt]);
		 break;
	}

	DO_LDS();

    END_OPF;

    //
    // Mednafen special instruction
    //
    BEGIN_OPF(INTERRUPT, 0x3F, 0);
	if(Halted)
	{
	 goto SkipNPCStuff;
	}
	else
	{
 	 DO_LDS();

	 new_PC = Exception(EXCEPTION_INT, PC, new_PC_mask);
         new_PC_mask = 0;
	}
    END_OPF;
   }

   OpDone: ;

   PC = (PC & new_PC_mask) + new_PC;
   new_PC_mask = ~0U;
   new_PC = 4;

   SkipNPCStuff:	;

   //printf("\n");
  }
 } while(PSX_EventHandler(timestamp));

 if(gte_ts_done > 0)
  gte_ts_done -= timestamp;

 ACTIVE_TO_BACKING;

 return(timestamp);
}

pscpu_timestamp_t PS_CPU::Run(pscpu_timestamp_t timestamp_in, bool ILHMode)
{
 if(CPUHook || ADDBT)
  return(RunReal<true, false>(timestamp_in));
 else
 {
  if(ILHMode)
   return(RunReal<false, true>(timestamp_in));
  else
   return(RunReal<false, false>(timestamp_in));
 }
}

void PS_CPU::SetCPUHook(void (*cpuh)(uint32 pc), void (*addbt)(uint32 from, uint32 to, bool exception))
{
 ADDBT = addbt;
 CPUHook = cpuh;
}

uint32 PS_CPU::GetRegister(unsigned int which, char *special, const uint32 special_len)
{
 uint32 ret = 0;

 if(which >= GSREG_GPR && which < (GSREG_GPR + 32))
  ret = GPR[which];
 else switch(which)
 {
  case GSREG_PC:
	ret = BACKED_PC;
	break;

  case GSREG_PC_NEXT:
	ret = BACKED_new_PC;
	break;

  case GSREG_IN_BD_SLOT:
	ret = !(BACKED_new_PC_mask & 3);
	break;

  case GSREG_LO:
	ret = LO;
	break;

  case GSREG_HI:
	ret = HI;
	break;

  case GSREG_SR:
	ret = CP0.SR;
	break;

  case GSREG_CAUSE:
	ret = CP0.CAUSE;
	break;

  case GSREG_EPC:
	ret = CP0.EPC;
	break;

 }

 return(ret);
}

void PS_CPU::SetRegister(unsigned int which, uint32 value)
{
 if(which >= GSREG_GPR && which < (GSREG_GPR + 32))
 {
  if(which != (GSREG_GPR + 0))
   GPR[which] = value;
 }
 else switch(which)
 {
  case GSREG_PC:
        BACKED_PC = value & ~0x3;	// Remove masking if we ever add proper misaligned PC exception
        break;

  case GSREG_LO:
        LO = value;
        break;

  case GSREG_HI:
        HI = value;
        break;

  case GSREG_SR:
        CP0.SR = value;		// TODO: mask
        break;

  case GSREG_CAUSE:
	CP0.CAUSE = value;
	break;

  case GSREG_EPC:
	CP0.EPC = value & ~0x3U;
	break;


 }
}

#undef BEGIN_OPF
#undef END_OPF
#undef MK_OPF

#define MK_OPF(op, funct)	((op) ? (0x40 | (op)) : (funct))
#define BEGIN_OPF(op, funct) case MK_OPF(op, funct): {
#define END_OPF } break;

// FIXME: should we breakpoint on an illegal address?  And with LWC2/SWC2 if CP2 isn't enabled?
void PS_CPU::CheckBreakpoints(void (*callback)(bool write, uint32 address, unsigned int len), uint32 instr)
{
 uint32 opf;

 opf = instr & 0x3F;

 if(instr & (0x3F << 26))
  opf = 0x40 | (instr >> 26);


 switch(opf)
 {
  default:
	break;

    //
    // LB - Load Byte
    //
    BEGIN_OPF(0x20, 0);
	ITYPE;
	uint32 address = GPR[rs] + immediate;

        callback(false, address, 1);
    END_OPF;

    //
    // LBU - Load Byte Unsigned
    //
    BEGIN_OPF(0x24, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        callback(false, address, 1);
    END_OPF;

    //
    // LH - Load Halfword
    //
    BEGIN_OPF(0x21, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        callback(false, address, 2);
    END_OPF;

    //
    // LHU - Load Halfword Unsigned
    //
    BEGIN_OPF(0x25, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        callback(false, address, 2);
    END_OPF;


    //
    // LW - Load Word
    //
    BEGIN_OPF(0x23, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        callback(false, address, 4);
    END_OPF;

    //
    // SB - Store Byte
    //
    BEGIN_OPF(0x28, 0);
	ITYPE;
	uint32 address = GPR[rs] + immediate;

        callback(true, address, 1);
    END_OPF;

    // 
    // SH - Store Halfword
    //
    BEGIN_OPF(0x29, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        callback(true, address, 2);
    END_OPF;

    // 
    // SW - Store Word
    //
    BEGIN_OPF(0x2B, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        callback(true, address, 4);
    END_OPF;

    //
    // LWL - Load Word Left
    //
    BEGIN_OPF(0x22, 0);
	ITYPE;
	uint32 address = GPR[rs] + immediate;

	do
	{
         callback(false, address, 1);
	} while((address--) & 0x3);

    END_OPF;

    //
    // SWL - Store Word Left
    //
    BEGIN_OPF(0x2A, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        do
        {
	 callback(true, address, 1);
        } while((address--) & 0x3);

    END_OPF;

    //
    // LWR - Load Word Right
    //
    BEGIN_OPF(0x26, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        do
        {
	 callback(false, address, 1);
        } while((++address) & 0x3);

    END_OPF;

    //
    // SWR - Store Word Right
    //
    BEGIN_OPF(0x2E, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

        do
        {
	 callback(true, address, 1);
        } while((++address) & 0x3);

    END_OPF;

    //
    // LWC2
    //
    BEGIN_OPF(0x32, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	callback(false, address, 4);
    END_OPF;

    //
    // SWC2
    //
    BEGIN_OPF(0x3A, 0);
        ITYPE;
        uint32 address = GPR[rs] + immediate;

	callback(true, address, 4);
    END_OPF;

 }
}


}
