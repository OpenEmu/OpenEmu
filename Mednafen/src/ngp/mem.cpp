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
#include "TLCS900h_registers.h"
#include "Z80_interface.h"
#include "bios.h"
#include "gfx.h"
#include "mem.h"
#include "interrupt.h"
#include "sound.h"
#include "flash.h"
#include "rtc.h"

//=============================================================================

//Hack way of returning good Flash status.
bool FlashStatusEnable = FALSE;
static uint32 FlashStatus;	

uint8 CPUExRAM[16384];

bool debug_abort_memory = FALSE;
bool debug_mask_memory_error_messages = FALSE;

bool memory_unlock_flash_write = FALSE;
bool memory_flash_error = FALSE;
bool memory_flash_command = FALSE;


uint8 SC0BUF; // Serial channel 0 buffer.
uint8 COMMStatus;

// In very very very rare conditions(like on embedded platforms with no virtual memory and very limited RAM and
// malloc happens to return a pointer aligned to a 64KiB boundary), a FastReadMap entry may be NULL even if
// it points to valid data when it's added to the address of the read, but
// if this happens, it will only make the emulator slightly slower.
static uint8 *FastReadMap[256], *FastReadMapReal[256];


void SetFRM(void) // Call this function after rom is loaded
{
 for(unsigned int x = 0x20; x <= 0x3f; x++)
 {
  if(ngpc_rom.length > (x * 65536 + 65535 - 0x20000))
   FastReadMapReal[x] = &ngpc_rom.data[x * 65536 - 0x200000] - x * 65536;
 }

 for(unsigned int x = 0x80; x <= 0x9f; x++)
 {
  if(ngpc_rom.length > (x * 65536 + 65535 - 0x80000))
   FastReadMapReal[x] = &ngpc_rom.data[x * 65536 - 0x800000] - x * 65536;
 }

}

void RecacheFRM(void)
{
 for(int x = 0; x < 256; x++)
  FastReadMap[x] = FlashStatusEnable ? NULL : FastReadMapReal[x];
}

static void* translate_address_read(uint32 address)
{
	address &= 0xFFFFFF;

	//Get Flash status?
	if (FlashStatusEnable)
	{
		if ((address >= ROM_START && address <= ROM_END) || (address >= HIROM_START && address <= HIROM_END))
		{
			FlashStatusEnable = FALSE;
			RecacheFRM();
			if (address == 0x220000 || address == 0x230000)
			{
				FlashStatus = 0xFFFFFFFF;
				return &FlashStatus;
			}
		}
	}

	//ROM (LOW)
	if (address >= ROM_START && address <= ROM_END)
	{
		if (address <= ROM_START + ngpc_rom.length)
			return ngpc_rom.data + (address - ROM_START);
		else
			return NULL;
	}

	//ROM (HIGH)
	if (address >= HIROM_START && address <= HIROM_END)
	{
		if (address <= HIROM_START + (ngpc_rom.length - 0x200000))
			return ngpc_rom.data + 0x200000 + (address - HIROM_START);
		else
			return NULL;
	}

	// ===================================

	//BIOS Access?
	if ((address & 0xFF0000) == 0xFF0000)
	{
		return ngpc_bios + (address & 0xFFFF); // BIOS ROM
	}
	return NULL;
}

//=============================================================================

static void* translate_address_write(uint32 address)
{	
	address &= 0xFFFFFF;

	if (memory_unlock_flash_write)
	{
		//ROM (LOW)
		if (address >= ROM_START && address <= ROM_END)
		{
			if (address <= ROM_START + ngpc_rom.length)
				return ngpc_rom.data + (address - ROM_START);
			else
				return NULL;
		}

		//ROM (HIGH)
		if (address >= HIROM_START && address <= HIROM_END)
		{
			if (address <= HIROM_START + (ngpc_rom.length - 0x200000))
				return ngpc_rom.data + 0x200000 + (address - HIROM_START);
			else
				return NULL;
		}
	}
	else
	{
		//ROM (LOW)
		if (address >= ROM_START && address <= ROM_END)
		{
			//Ignore Flash commands
			if (address == 0x202AAA || address == 0x205555)
			{
	//			system_debug_message("%06X: Enable Flash command from %06X", pc, address);
				memory_flash_command = TRUE;
				return NULL;
			}

			//Set Flash status reading?
			if (address == 0x220000 || address == 0x230000)
			{
	//			system_debug_message("%06X: Flash status read from %06X", pc, address);
				FlashStatusEnable = TRUE;
				RecacheFRM();
				return NULL;
			}

			if (memory_flash_command)
			{
				//Write the 256byte block around the flash data
				flash_write(address & 0xFFFF00, 256);
				
				//Need to issue a new command before writing will work again.
				memory_flash_command = FALSE;
		
	//			system_debug_message("%06X: Direct Flash write to %06X", pc, address & 0xFFFF00);
	//			system_debug_stop();

				//Write to the rom itself.
				if (address <= ROM_START + ngpc_rom.length)
					return ngpc_rom.data + (address - ROM_START);
			}
		}
	}

	// ===================================
	return NULL;
}

/* WARNING:  32-bit loads and stores apparently DON'T have to be 4-byte-aligned(so we must +2 instead of |2). */
/* Treat all 32-bit operations as two 16-bit operations */
extern uint32 pc;

uint8 lastpoof = 0;

uint8 loadB(uint32 address)
{
        address &= 0xFFFFFF;

        if(FastReadMap[address >> 16])
         return(FastReadMap[address >> 16][address]);


        uint8* ptr = (uint8*)translate_address_read(address);

        if (ptr)
                return *ptr;

	if(address >= 0x8000 && address <= 0xbfff)
	 return(NGPGfx->read8(address));
	if(address >= 0x4000 && address <= 0x7fff)
	{
	 return(*(uint8 *)(CPUExRAM + address - 0x4000));
	}

	if(address >= 0x70 && address <= 0x7F)
	{
	 return(int_read8(address));
	}

        if(address >= 0x90 && address <= 0x97)
        {
         return(rtc_read8(address));
        }

	if(address >= 0x20 && address <= 0x29)
	{
	 return(timer_read8(address));
	}

	if(address == 0x50)
	 return(SC0BUF);

        if(address == 0xBC)
         return Z80_ReadComm();

	//printf("UNK B R: %08x\n", address);

	return(0);
}

uint16 loadW(uint32 address)
{
        address &= 0xFFFFFF;

	if(address & 1)
	 return(loadB(address) | (loadB(address + 1) << 8));

	if(FastReadMap[address >> 16])
	 return(le16toh(*(uint16*)&FastReadMap[address >> 16][address]));

        uint16* ptr = (uint16*)translate_address_read(address);
	if(ptr)
                return le16toh(*ptr);

        if(address >= 0x8000 && address <= 0xbfff)
         return(NGPGfx->read16(address));

        if(address >= 0x4000 && address <= 0x7fff)
	{
         return(le16toh(*(uint16 *)(CPUExRAM + address - 0x4000)));
	}
	if(address == 0x50)
	 return(SC0BUF);

        if(address >= 0x70 && address <= 0x7F)
         return(int_read8(address) | (int_read8(address + 1) << 8));

	if(address >= 0x90 && address <= 0x97)
	 return(rtc_read8(address) | (rtc_read8(address + 1) << 8));

        if(address >= 0x20 && address <= 0x29)
         return(timer_read8(address) | (timer_read8(address + 1) << 8));

	if(address == 0xBC)
	 return Z80_ReadComm();

	//printf("UNK W R: %08x\n", address);

	return(0);
}

uint32 loadL(uint32 address)
{
	uint32 ret;

	ret = loadW(address);
	ret |= loadW(address + 2) << 16;

	return(ret);
}

//=============================================================================

void storeB(uint32 address, uint8 data)
{
        address &= 0xFFFFFF;


	if(address < 0x80)
	{
	 lastpoof = data;
	}

        if(address >= 0x8000 && address <= 0xbfff)
	{
         NGPGfx->write8(address, data);
	 return;
	}
        if(address >= 0x4000 && address <= 0x7fff)
        {
         *(uint8 *)(CPUExRAM + address - 0x4000) = data;
         return;
        }
	if(address >= 0x70 && address <= 0x7F)
	{
	 int_write8(address, data);
	 return;
	}
        if(address >= 0x20 && address <= 0x29)
	{
	 timer_write8(address, data);
	 return;
	}

	if(address == 0x50)
	{
	 SC0BUF = data;
	 return;
	}

	if(address == 0x6f) // Watchdog timer
	{
	 return;
	}

	if(address == 0xb2) // Comm?
	{
	 COMMStatus = data & 1;
	 return;
	}

	if(address == 0xb9) 
	{
         if(data == 0x55)
          Z80_SetEnable(1);
         else if(data == 0xAA)
          Z80_SetEnable(0);
         return;
	}

        if(address == 0xb8)
        {
	 if(data == 0x55)
	  MDFNNGPCSOUND_SetEnable(1);
	 else if(data == 0xAA)
	  MDFNNGPCSOUND_SetEnable(0);
	 return;
        }

	if (address == 0xBA)
	{
                Z80_nmi();
		return;
	}

	if(address == 0xBC)
	{
		Z80_WriteComm(data);
		return;
	}

	if(address >= 0xa0 && address <= 0xA3)
	{
         if(!Z80_IsEnabled())
         {
          if (address == 0xA1)   Write_SoundChipLeft(data);
          else if (address == 0xA0)      Write_SoundChipRight(data);
         } 
         //DAC Write
         if (address == 0xA2)
         {
          dac_write_left(data);
         }
         else if (address == 0xA3)
         {
          dac_write_right(data);
         }
	 return;
	}

	//printf("%08x %02x\n", address, data);
	uint8* ptr = (uint8*)translate_address_write(address);

	//Write
	if (ptr)
	{
		*ptr = data;
	}
	//else
        //        printf("ACK: %08x %02x\n", address, data);

}

void storeW(uint32 address, uint16 data)
{
        address &= 0xFFFFFF;

	if(address & 1)
	{
	 storeB(address + 0, data & 0xFF);
	 storeB(address + 1, data >> 8);
	 return;
	}

        if(address < 0x80)
        {
         lastpoof = data >> 8;
        }

        if(address >= 0x8000 && address <= 0xbfff)
        {
         NGPGfx->write16(address, data);
	 return;
        }
        if(address >= 0x4000 && address <= 0x7fff)
        {
         *(uint16 *)(CPUExRAM + address - 0x4000) = htole16(data);
         return;
        }
        if(address >= 0x70 && address <= 0x7F)
        {
         int_write8(address, data & 0xFF);
	 int_write8(address + 1, data >> 8);
         return;
        }

        if(address >= 0x20 && address <= 0x29)
	{
	 timer_write8(address, data & 0xFF);
	 timer_write8(address + 1, data >> 8);
	}

	if(address == 0x50)
	{ 
	 SC0BUF = data & 0xFF;
	 return;
	}

        if(address == 0x6e) // Watchdog timer(technically 0x6f)
        {
	 return;
        }

        if(address == 0xb2) // Comm?
        {
 	 COMMStatus = data & 1;
         return;
        }

	if(address == 0xb8)
	{
         if((data & 0xFF00) == 0x5500)
          Z80_SetEnable(1);
         else if((data & 0xFF00) == 0xAA00)
          Z80_SetEnable(0);

         if((data & 0xFF) == 0x55)
          MDFNNGPCSOUND_SetEnable(1);
         else if((data & 0xFF) == 0xAA)
          MDFNNGPCSOUND_SetEnable(0);
	 return;
        }

        if (address == 0xBA)
        {
                Z80_nmi();
                return;
        }

        if(address == 0xBC)
        {
                Z80_WriteComm(data);
                return;
        }

	if(address >= 0xa0 && address <= 0xA3)
	{
	 storeB(address, data & 0xFF);
	 storeB(address + 1, data >> 8);
	 return;
	}

	uint16* ptr = (uint16*)translate_address_write(address);

	//Write
	if (ptr)
	{
		*ptr = htole16(data);
	}
        //else
        //        printf("ACK16: %08x %04x\n", address, data);

}

void storeL(uint32 address, uint32 data)
{
	storeW(address, data & 0xFFFF);
        storeW(address + 2, data >> 16);
}

//=============================================================================

static const uint8 systemMemory[] = 
{
	// 0x00												// 0x08
	0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,		0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x08, 0xFF, 0xFF,
	// 0x10												// 0x18
	0x34, 0x3C, 0xFF, 0xFF, 0xFF, 0x3F, 0x00, 0x00,		0x3F, 0xFF, 0x2D, 0x01, 0xFF, 0xFF, 0x03, 0xB2,
	// 0x20												// 0x28
	0x80, 0x00, 0x01, 0x90, 0x03, 0xB0, 0x90, 0x62,		0x05, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x4C, 0x4C,
	// 0x30												// 0x38
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x30, 0x00, 0x00, 0x00, 0x20, 0xFF, 0x80, 0x7F,
	// 0x40												// 0x48
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x50												// 0x58
	0x00, 0x20, 0x69, 0x15, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF,
	// 0x60												// 0x68
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x17, 0x17, 0x03, 0x03, 0x02, 0x00, 0x00, 0x4E,
	// 0x70												// 0x78
	0x02, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x80												// 0x88
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0x90												// 0x98
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xA0												// 0xA8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xB0												// 0xB8
	0x00, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x00, 0x00,		0xAA, 0xAA, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xC0												// 0xC8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xD0												// 0xD8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xE0												// 0xE8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
	// 0xF0												// 0xF8
	0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//=============================================================================

void reset_memory(void)
{
	unsigned int i;

	FlashStatusEnable = FALSE;
	RecacheFRM();

	memory_flash_command = FALSE;

//=============================================================================
//000000 -> 000100	CPU Internal RAM (Timers/DMA/Z80)
//=====================================================

	for (i = 0; i < sizeof(systemMemory); i++)
		storeB(i, systemMemory[i]);

//=============================================================================
//006C00 -> 006FFF	BIOS Workspace
//==================================

	storeL(0x6C00, rom_header->startPC);		//Start

	storeW(0x6C04, rom_header->catalog);
	storeW(0x6E82, rom_header->catalog);

	storeB(0x6C06, rom_header->subCatalog);
	storeB(0x6E84, rom_header->subCatalog);

	for(i = 0; i < 12; i++)
	 storeB(0x6c08 + i, ngpc_rom.data[0x24 + i]);

	storeB(0x6C58, 0x01);

	//32MBit cart?
	if (ngpc_rom.length > 0x200000)
		storeB(0x6C59, 0x01);
	else
		storeB(0x6C59, 0x00);

	storeB(0x6C55, 1);	//Commercial game

	storeB(0x6F80, 0xFF);	//Lots of battery power!
	storeB(0x6F81, 0x03);

	storeB(0x6F84, 0x40);	// "Power On" startup
	storeB(0x6F85, 0x00);	// No shutdown request
	storeB(0x6F86, 0x00);	// No user answer (?)

	//Language: 0 = Japanese, 1 = English
	storeB(0x6F87, MDFN_GetSettingB("ngp.language"));

	//Color Mode Selection: 0x00 = B&W, 0x10 = Colour
	storeB(0x6F91, rom_header->mode);
	storeB(0x6F95, rom_header->mode);

	//Interrupt table
	for (i = 0; i < 0x12; i++)
		storeL(0x6FB8 + i * 4, 0x00FF23DF);


//=============================================================================
//008000 -> 00BFFF	Video RAM
//=============================

	storeB(0x8000, 0xC0);	// Both interrupts allowed

	//Hardware window
	storeB(0x8002, 0x00);
	storeB(0x8003, 0x00);
	storeB(0x8004, 0xFF);
	storeB(0x8005, 0xFF);

	storeB(0x8006, 0xc6);	// Frame Rate Register

	storeB(0x8012, 0x00);	// NEG / OOWC setting.

	storeB(0x8118, 0x80);	// BGC on!

	storeB(0x83E0, 0xFF);	// Default background colour
	storeB(0x83E1, 0x0F);

	storeB(0x83F0, 0xFF);	// Default window colour
	storeB(0x83F1, 0x0F);

	storeB(0x8400, 0xFF);	// LED on
	storeB(0x8402, 0x80);	// Flash cycle = 1.3s

	storeB(0x87E2, loadB(0x6F95) ? 0x00 : 0x80);
}

//=============================================================================


