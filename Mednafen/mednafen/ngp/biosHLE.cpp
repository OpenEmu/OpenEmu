//---------------------------------------------------------------------------
// NEOPOP : Emulator as in Dreamland
//
// Copyright (c) 2001-2002 by neopop_uk
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//	This program is free software; you can redistribute it and/or modify
//	it under the terms of the GNU General Public License as published by
//	the Free Software Foundation; either version 2 of the License, or
//	(at your option) any later version. See also the license.txt file for
//	additional informations.
//---------------------------------------------------------------------------

#include "neopop.h"
#include "bios.h"
#include "TLCS900h_registers.h"
#include "TLCS900h_interpret.h"
#include "mem.h"
#include "flash.h"
#include "dma.h"
#include "interrupt.h"


static uint8 CacheIntPrio[0xB]; // Iinterrupt prio registers at 0x0070-0x007a don't have priority readable.
	 		       // This should probably be stored in BIOS work RAM somewhere instead of a separate array, but I don't know where!


void BIOSHLE_Reset(void)
{
 memset(CacheIntPrio, 0, sizeof(CacheIntPrio));
 CacheIntPrio[0] = 0x02;
 CacheIntPrio[1] = 0x32;

 for(int x = 0; x < 0xB; x++)
  storeB(0x70 + x, CacheIntPrio[x]);
}

/* This is the custom HLE instruction. I considered it was the fastest and
most streamlined way of intercepting a bios call. The operation performed
is dependant on the current program counter. */

void iBIOSHLE(void)
{
	//Only works within the bios
	if ((pc & 0xFF0000) != 0xFF0000)
		return;

	pc --;	//Compensate for processing this instruction.

	cycles = 8;		//TODO: Correct cycle counts (or approx?)

	//if(pc != 0xff1222)
	//printf("SPOON: %08x\n", pc);

	switch (pc & 0xffffff)
	{	

	//default: printf("SPOON: %08x\n", pc); break;
		//VECT_SHUTDOWN
	case 0xFF27A2:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			if (a != 0xBAADC0DE)
				system_debug_message("VECT_SHUTDOWN: called before %06X", a);
			push32(a);
		}

		system_debug_stop();
#endif
		{
			//Cheap bit of code to stop the message appearing repeatedly.
			uint32 a = pop32();
			if (a != 0xBAADC0DE)
				MDFN_printf("IDS_POWER");
			push32(0xBAADC0DE); //Sure is!
		}

		return;	//Don't pop a return address, stay here

		//VECT_CLOCKGEARSET
	case 0xFF1030:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_CLOCKGEARSET: called before %06X", a);
			push32(a);
		}
#endif
		//MDFN_printf("%d\n", rCodeB(0x35));
		//TODO
	//	if (rCodeB(0x35) > 0)
	//		system_message("Unsupported clock gear %d set\nPlease inform the author", rCodeB(0x35));

		break;

		//VECT_RTCGET
	case 0xFF1440:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_RTCGET: called before %06X", a);
			push32(a);
		}
#endif
		
		if (rCodeL(0x3C) < 0xC000)
		{
			//Copy data from hardware area
			storeB(rCodeL(0x3C) + 0, loadB(0x91));
			storeB(rCodeL(0x3C) + 1, loadB(0x92));
			storeB(rCodeL(0x3C) + 2, loadB(0x93));
			storeB(rCodeL(0x3C) + 3, loadB(0x94));
			storeB(rCodeL(0x3C) + 4, loadB(0x95));
			storeB(rCodeL(0x3C) + 5, loadB(0x96));
			storeB(rCodeL(0x3C) + 6, loadB(0x97));
		}

		break; 
	
		//?
	//case 0xFF12B4:
	//	break;
	
		//VECT_INTLVSET
	case 0xFF1222:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_INTLVSET: called before %06X", a);
			push32(a);
		}
#endif

		{

	uint8 level = rCodeB(0x35); //RB3
	uint8 interrupt = rCodeB(0x34);	//RC3

	//   0 - Interrupt from RTC alarm
	//   1 - Interrupt from the Z80 CPU
	//   2 - Interrupt from the 8 bit timer 0
	//   3 - Interrupt from the 8 bit timer 1
	//   4 - Interrupt from the 8 bit timer 2
	//   5 - Interrupt from the 8 bit timer 3
	//   6 - End of transfer interrupt from DMA channel 0
	//   7 - End of transfer interrupt from DMA channel 1
	//   8 - End of transfer interrupt from DMA channel 2
	//   9 - End of transfer interrupt from DMA channel 3

	switch(interrupt)
	{
	case 0x00:	CacheIntPrio[0x0] = (CacheIntPrio[0x0] & 0xf0) |  (level & 0x07);
			storeB(0x70, CacheIntPrio[0x0]);
			break;

	case 0x01:	CacheIntPrio[0x1] = (CacheIntPrio[0x1] & 0x0f) | ((level & 0x07)<<4);
			storeB(0x71, CacheIntPrio[0x1]);
			break;

	case 0x02:	CacheIntPrio[0x3] = (CacheIntPrio[0x3] & 0xf0) |  (level & 0x07);
			storeB(0x73, CacheIntPrio[0x3]);
			break;

	case 0x03:	CacheIntPrio[0x3] = (CacheIntPrio[0x3] & 0x0f) | ((level & 0x07)<<4);
			storeB(0x73, CacheIntPrio[0x3]);
			break;

	case 0x04:	CacheIntPrio[0x4] = (CacheIntPrio[0x4] & 0xf0) |  (level & 0x07);
			storeB(0x74, CacheIntPrio[0x4]);
			break;

	case 0x05:	CacheIntPrio[0x4] = (CacheIntPrio[0x4] & 0x0f) | ((level & 0x07)<<4);
			storeB(0x74, CacheIntPrio[0x4]);
			break;

	case 0x06:	CacheIntPrio[0x9] = (CacheIntPrio[0x9] & 0xf0) |  (level & 0x07);
			storeB(0x79, CacheIntPrio[0x9]);
			break;

	case 0x07:	CacheIntPrio[0x9] = (CacheIntPrio[0x9] & 0x0f) | ((level & 0x07)<<4);
			storeB(0x79, CacheIntPrio[0x9]);
			break;

	case 0x08:	CacheIntPrio[0xa] = (CacheIntPrio[0xa] & 0xf0) |  (level & 0x07);
			storeB(0x7a, CacheIntPrio[0xa]);
			break;

	case 0x09:	CacheIntPrio[0xa] = (CacheIntPrio[0xa] & 0x0f) | ((level & 0x07)<<4);
			storeB(0x7a, CacheIntPrio[0xa]);
			break;
	default: puts("DOH"); break;
	}
		}
		break;	
	
		//VECT_SYSFONTSET
	case 0xFF8D8A:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_SYSFONTSET: called before %06X", a);
			push32(a);
		}
#endif
		{
			uint8 a,b,c, j;
			uint16 i, dst = 0xA000;

			b = rCodeB(0x30) >> 4;
			a = rCodeB(0x30) & 3;

			for (i = 0; i < 0x800; i++)
			{
				c = ngpc_bios[0x8DCF + i];

				for (j = 0; j < 8; j++, c<<=1)
				{
					uint16 data16;

					data16 = loadW(dst);
					data16 <<= 2;
					storeW(dst, data16);

					if (c & 0x80)	storeB(dst, loadB(dst) | a);
					else			storeB(dst, loadB(dst) | b);
				}

				dst += 2;
			}
		}
		
		break;
	
		//VECT_FLASHWRITE
	case 0xFF6FD8:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_FLASHWRITE: called before %06X", a);
			push32(a);
		}
#endif
		{
			uint32 i, bank = 0x200000;

			//Select HI rom?
			if (rCodeB(0x30) == 1)
				bank = 0x800000;

#ifdef NEOPOP_DEBUG
			if (filter_bios)
				system_debug_message("VECT_FLASHWRITE: Copy %06X -> %06X,  %d bytes",
					rCodeL(0x3C), rCodeL(0x38) + bank, rCodeW(0x34) * 256);
#endif
			
			memory_flash_error = FALSE;
			memory_unlock_flash_write = TRUE;
			//Copy as 32 bit values for speed
			for (i = 0; i < rCodeW(0x34) * 64ul; i++)
				storeL(rCodeL(0x38) + bank + (i * 4), loadL(rCodeL(0x3C) + (i * 4)));
			memory_unlock_flash_write = FALSE;

			if (memory_flash_error)
			{
#ifdef NEOPOP_DEBUG
				if (filter_bios)
					system_debug_message("VECT_FLASHWRITE: Error");
#endif
				rCodeB(0x30) = 0xFF;	//RA3 = SYS_FAILURE
			}
			else
			{
				uint32 address = rCodeL(0x38);
				if (rCodeB(0x30) == 1)
					address += 0x800000;
				else
					address += 0x200000;

				//Save this data to an external file
				flash_write(address, rCodeW(0x34) * 256);

				rCodeB(0x30) = 0;		//RA3 = SYS_SUCCESS
			}
		}

		break;
	
		//VECT_FLASHALLERS
	case 0xFF7042:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_FLASHALLERS: called before %06X", a);
			push32(a);
		}
#endif
		//TODO
		rCodeB(0x30) = 0;	//RA3 = SYS_SUCCESS
		break;
	
		//VECT_FLASHERS
	case 0xFF7082:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_FLASHERS: called before %06X", a);
			push32(a);
		}
#endif
#ifdef NEOPOP_DEBUG
		if (filter_bios)
			system_debug_message("VECT_FLASHERS: bank %d, block %d (?)", rCodeB(0x30), rCodeB(0x35));
#endif
		//TODO
		rCodeB(0x30) = 0;	//RA3 = SYS_SUCCESS
		break;
	
		//VECT_ALARMSET
	case 0xFF149B:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_ALARMSET: called before %06X", a);
			push32(a);
		}
#endif
		//TODO
		rCodeB(0x30) = 0;	//RA3 = SYS_SUCCESS
		break;
	
		//?
	//case 0xFF1033: break;
	
		//VECT_ALARMDOWNSET
	case 0xFF1487:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_ALARMDOWNSET: called before %06X", a);
			push32(a);
		}
#endif
		//TODO
		rCodeB(0x30) = 0;	//RA3 = SYS_SUCCESS
		break;

		//?
	//case 0xFF731F: break;
	
		//VECT_FLASHPROTECT
	case 0xFF70CA:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_FLASHPROTECT: called before %06X", a);
			push32(a);
		}
#endif
		//TODO
		rCodeB(0x30) = 0;	//RA3 = SYS_SUCCESS
		break;
	
		//VECT_GEMODESET
	case 0xFF17C4:
#ifdef NEOPOP_DEBUG
		if (filter_bios)
		{
			uint32 a = pop32();
			system_debug_message("VECT_GEMODESET: called before %06X", a);
			push32(a);
		}
#endif
		//TODO
		break;
	
		//?
	//case 0xFF1032: break;
	
		//VECT_COMINIT
	case 0xFF2BBD:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMINIT: called before %06X", a);
			push32(a);
		}
#endif
		// Nothing to do.
		rCodeB(0x30) = 0;	//RA3 = COM_BUF_OK
		break;
	
		//VECT_COMSENDSTART
	case 0xFF2C0C:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMSENDSTART: called before %06X", a);
			push32(a);
		}
#endif
		// Nothing to do.
		break;
	
		//VECT_COMRECIVESTART
	case 0xFF2C44:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMRECIVESTART: called before %06X", a);
			push32(a);
		}
#endif
		// Nothing to do.
		break;
	
		//VECT_COMCREATEDATA
	case 0xFF2C86:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMCREATEDATA: called before %06X", a);
			push32(a);
		}
#endif
		{
			//Write the byte
			uint8 data = rCodeB(0x35);
			system_comms_write(data);
		}

		//Restore $PC after BIOS-HLE instruction
		pc = pop32();
		
		TestIntHDMA(11, 0x18);

		//Always COM_BUF_OK because the write call always succeeds.
		rCodeB(0x30) = 0x0;			//RA3 = COM_BUF_OK
		return;
	
		//VECT_COMGETDATA
	case 0xFF2CB4:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMGETDATA: called before %06X", a);
			push32(a);
		}
#endif
		{
			uint8 data;

			if (system_comms_read(&data))
			{
				rCodeB(0x30) = 0;	//COM_BUF_OK
				rCodeB(0x35) = data;

				pc = pop32();
			
				//Comms. Read interrupt
				storeB(0x50, data);
				TestIntHDMA(12, 0x19);

				return;
			}
			else
			{
				rCodeB(0x30) = 1;	//COM_BUF_EMPTY
			}
		}

		break;
	
		//VECT_COMONRTS
	case 0xFF2D27:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMONRTS: called before %06X", a);
			push32(a);
		}
#endif
		storeB(0xB2, 0);
		break;
	
		//VECT_COMOFFRTS
	case 0xFF2D33: 	
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMOFFRTS: called before %06X", a);
			push32(a);
		}
#endif
		storeB(0xB2, 1);
		break;	
	
		//VECT_COMSENDSTATUS
	case 0xFF2D3A:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMSENDSTATUS: called before %06X", a);
			push32(a);
		}
#endif
		// Nothing to do.
		rCodeW(0x30) = 0;	//Send Buffer Count, never any pending data!
		break;
	
		//VECT_COMRECIVESTATUS
	case 0xFF2D4E:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMRECIVESTATUS: called before %06X", a);
			push32(a);
		}
#endif

		// Receive Buffer Count
		rCodeW(0x30) = system_comms_read(NULL);

		break;
	
		//VECT_COMCREATEBUFDATA
	case 0xFF2D6C:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMCREATEBUFDATA: called before %06X", a);
			push32(a);
		}
#endif
		pc = pop32();

		while(rCodeB(0x35) > 0)
		{
			uint8 data;
			data = loadB(rCodeL(0x3C));

			//Write data from (XHL3++)
			system_comms_write(data);
			rCodeL(0x3C)++; //Next data

			rCodeB(0x35)--;	//RB3 = Count Left
		}

		TestIntHDMA(11, 0x18);
		return;
	
		//VECT_COMGETBUFDATA
	case 0xFF2D85:
#ifdef NEOPOP_DEBUG
		if (filter_comms)
		{
			uint32 a = pop32();
			system_debug_message("VECT_COMGETBUFDATA: called before %06X", a);
			push32(a);
		}
#endif
		{
			pc = pop32();

			while(rCodeB(0x35) > 0)
			{
				uint8 data;

				if (system_comms_read(&data))
				{
					//Read data into (XHL3++)
					storeB(rCodeL(0x3C), data);
					rCodeL(0x3C)++; //Next data
					rCodeB(0x35)--;	//RB3 = Count Left

					//Comms. Read interrupt
					storeB(0x50, data);
					TestIntHDMA(12, 0x19);
					return;
				}
				else
					break;
			}

		}

		return;
	}

	//RET
	pc = pop32();
}

int BIOSHLE_StateAction(StateMem *sm, int load, int data_only)
{
 SFORMAT StateRegs[] =
 {
  SFARRAY(CacheIntPrio, 0xB),
  SFEND
 };
 if(!MDFNSS_StateAction(sm, load, data_only, StateRegs, "BHLE"))
  return(0);

 return(1);
}

//=============================================================================
