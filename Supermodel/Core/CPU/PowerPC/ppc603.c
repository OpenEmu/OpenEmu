/**
 ** Supermodel
 ** A Sega Model 3 Arcade Emulator.
 ** Copyright 2011 Bart Trzynadlowski, Nik Henson
 **
 ** This file is part of Supermodel.
 **
 ** Supermodel is free software: you can redistribute it and/or modify it under
 ** the terms of the GNU General Public License as published by the Free 
 ** Software Foundation, either version 3 of the License, or (at your option)
 ** any later version.
 **
 ** Supermodel is distributed in the hope that it will be useful, but WITHOUT
 ** ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 ** FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 ** more details.
 **
 ** You should have received a copy of the GNU General Public License along
 ** with Supermodel.  If not, see <http://www.gnu.org/licenses/>.
 **/

/*
 * ppc603.c
 *
 * PowerPC 603e functions. Included from ppc.cpp; do not compile separately.
 */

void ppc603_exception(int exception)
{
#ifdef SUPERMODEL_DEBUGGER
		if (PPCDebug != NULL)
			PPCDebug->CPUException(exception);
#endif

	switch( exception )
	{
		case EXCEPTION_IRQ:		/* External Interrupt */
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr & 0xff73;

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0500;
				else
					ppc.npc = 0x00000000 | 0x0500;

				ppc.interrupt_pending &= ~0x1;
				ppc_change_pc(ppc.npc);
			}
			break;

		case EXCEPTION_DECREMENTER:		/* Decrementer overflow exception */
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr & 0xff73;

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0900;
				else
					ppc.npc = 0x00000000 | 0x0900;

				ppc.interrupt_pending &= ~0x2;
				ppc_change_pc(ppc.npc);
			}
			break;

		case EXCEPTION_TRAP:			/* Program exception / Trap */
			{
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.pc;
				SRR1 = (msr & 0xff73) | 0x20000;	/* 0x20000 = TRAP bit */

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0700;
				else
					ppc.npc = 0x00000000 | 0x0700;
				ppc_change_pc(ppc.npc);
			}
			break;

		case EXCEPTION_SYSTEM_CALL:		/* System call */
			{
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = (msr & 0xff73);

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0c00;
				else
					ppc.npc = 0x00000000 | 0x0c00;
				ppc_change_pc(ppc.npc);
			}
			break;

		case EXCEPTION_SMI:
			if( ppc_get_msr() & MSR_EE ) {
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr & 0xff73;

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x1400;
				else
					ppc.npc = 0x00000000 | 0x1400;

				ppc.interrupt_pending &= ~0x4;
				ppc_change_pc(ppc.npc);
			}
			break;

		case EXCEPTION_DSI:
			{
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr & 0xff73;

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0300;
				else
					ppc.npc = 0x00000000 | 0x0300;

				ppc.interrupt_pending &= ~0x4;
				ppc_change_pc(ppc.npc);
			}
			break;

		case EXCEPTION_ISI:
			{
				UINT32 msr = ppc_get_msr();

				SRR0 = ppc.npc;
				SRR1 = msr & 0xff73;

				msr &= ~(MSR_POW | MSR_EE | MSR_PR | MSR_FP | MSR_FE0 | MSR_SE | MSR_BE | MSR_FE1 | MSR_IR | MSR_DR | MSR_RI);
				if( msr & MSR_ILE )
					msr |= MSR_LE;
				else
					msr &= ~MSR_LE;
				ppc_set_msr(msr);

				if( msr & MSR_IP )
					ppc.npc = 0xfff00000 | 0x0400;
				else
					ppc.npc = 0x00000000 | 0x0400;

				ppc.interrupt_pending &= ~0x4;
				ppc_change_pc(ppc.npc);
			}
			break;

		default:
			ErrorLog("PowerPC triggered an unknown exception. Emulation halted until reset.");
			DebugLog("PowerPC triggered an unknown exception (%d).\n", exception);
			ppc.fatalError = true;
			break;
	}
}

static void ppc603_set_smi_line(int state)
{
	if( state ) {
		ppc.interrupt_pending |= 0x4;
	}
}

static void ppc603_check_interrupts(void)
{
	if (MSR & MSR_EE)
	{
		if (ppc.interrupt_pending != 0)
		{
			if (ppc.interrupt_pending & 0x1)
			{
				ppc603_exception(EXCEPTION_IRQ);
			}
			else if (ppc.interrupt_pending & 0x2)
			{
				ppc603_exception(EXCEPTION_DECREMENTER);
			}	
			else if (ppc.interrupt_pending & 0x4)
			{
				ppc603_exception(EXCEPTION_SMI);
			}
		}
	}
}

void ppc_reset(void)
{
	ppc.fatalError = false;	// reset the fatal error flag
	
	ppc.pc = ppc.npc = 0xfff00100;

	ppc_set_msr(0x40);
	ppc_change_pc(ppc.pc);

	ppc.hid0 = 1;

	ppc.interrupt_pending = 0;
}

int ppc_execute(int cycles)
{
	UINT32 opcode;
	ppc_icount = cycles;
	ppc_tb_base_icount = cycles;
	ppc_dec_base_icount = cycles + ppc.dec_frac;

	// check if decrementer exception occurs during execution
	if ((UINT32)(DEC - (cycles / (bus_freq_multiplier * 2))) > (UINT32)(DEC))
	{
		ppc_dec_trigger_cycle = ((cycles / (bus_freq_multiplier * 2)) - DEC) * 4;
	}
	else
	{
		ppc_dec_trigger_cycle = 0x7fffffff;
	}

	ppc_change_pc(ppc.npc);

	/*{
		char string1[200];
		char string2[200];
		opcode = BSWAP32(*ppc.op);
		DisassemblePowerPC(opcode, ppc.npc, string1, string2, true);
		printf("%08X: %s %s\n", ppc.npc, string1, string2);
	}*/
//	printf("trigger cycle %d (%08X)\n", ppc_dec_trigger_cycle, ppc_dec_trigger_cycle);
//	printf("tb = %08X %08X\n", (UINT32)(ppc.tb >> 32), (UINT32)(ppc.tb));

	ppc603_check_interrupts();

#ifdef SUPERMODEL_DEBUGGER
	if (PPCDebug != NULL)
		PPCDebug->CPUActive();
#endif // SUPERMODEL_DEBUGGER

	while( ppc_icount > 0 && !ppc.fatalError)
	{
		ppc.pc = ppc.npc;
		
		// Debug breakpoints
		//if (ppc.pc == 0x9e4d4)
		//	printf("%X R0=%08X\n", ppc.pc, REG(0));
			
		opcode = *ppc.op++;	// Supermodel byte reverses each aligned word (converting them to little endian) so they can be fetched directly
		ppc.npc = ppc.pc + 4;

#ifdef SUPERMODEL_DEBUGGER
		if (PPCDebug != NULL)
		{
			while (PPCDebug->CPUExecute(ppc.pc, opcode, (PPCDebug->instrCount > 0 ? 1 : 0)))
				opcode = *ppc.op++;
		}
#endif // SUPERMODEL_DEBUGGER

		switch(opcode >> 26)
		{
			case 19:	optable19[(opcode >> 1) & 0x3ff](opcode); break;
			case 31:	optable31[(opcode >> 1) & 0x3ff](opcode); break;
			case 59:	optable59[(opcode >> 1) & 0x3ff](opcode); break;
			case 63:	optable63[(opcode >> 1) & 0x3ff](opcode); break;
			default:	optable[opcode >> 26](opcode); break;
		}

		ppc_icount--;

		if(ppc_icount == ppc_dec_trigger_cycle)
		{
//			printf("dec int at %d\n", ppc_icount);
			ppc.interrupt_pending |= 0x2;
			ppc603_check_interrupts();
		}

		//ppc603_check_interrupts();
	}

#ifdef SUPERMODEL_DEBUGGER
	if (PPCDebug != NULL)
		PPCDebug->CPUInactive();
#endif // SUPERMODEL_DEBUGGER

	// update timebase
	// timebase is incremented once every four core clock cycles, so adjust the cycles accordingly
	ppc.tb += ((ppc_tb_base_icount - ppc_icount) / 4);

	// update decrementer
	ppc.dec_frac = ((ppc_dec_base_icount - ppc_icount) % (bus_freq_multiplier * 2));
	DEC -= ((ppc_dec_base_icount - ppc_icount) / (bus_freq_multiplier * 2));

	/*
	{
		char string1[200];
		char string2[200];
		opcode = BSWAP32(*ppc.op);
		DisassemblePowerPC(opcode, ppc.npc, string1, string2, true);
		printf("%08X: %s %s\n", ppc.npc, string1, string2);
	}
	*/

	return cycles - ppc_icount;
}
