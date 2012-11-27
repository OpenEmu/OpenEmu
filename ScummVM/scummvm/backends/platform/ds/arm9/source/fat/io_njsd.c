/*
	io_njsd.c

	Hardware Routines for reading an SD card using
	a NinjaDS SD adapter.

	Original code supplied by NinjaMod

 Copyright (c) 2006 Michael "Chishm" Chisholm

 Redistribution and use in source and binary forms, with or without modification,
 are permitted provided that the following conditions are met:

  1. Redistributions of source code must retain the above copyright notice,
     this list of conditions and the following disclaimer.
  2. Redistributions in binary form must reproduce the above copyright notice,
     this list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.
  3. The name of the author may not be used to endorse or promote products derived
     from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
 AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE
 LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

	2006-08-05 - Chishm
		* First release
*/

#include "io_njsd.h"

#ifdef SUPPORT_NJSD
#ifdef NDS

#include <nds.h>
#include <string.h>
#include "io_sd_common.h"

#define BYTES_PER_READ 512
//#define _NJSD_SYNC
// #define _NJSD_DEBUG

//---------------------------------------------------------------
// Card communication speeds
#define SD_CLK_200KHz		00
#define SD_CLK_5MHz		01
#define SD_CLK_8MHz			02
#define SD_CLK_12MHz		03

//---------------------------------------------------------------
// Response types
#define SD_RSP_48		0
#define SD_RSP_136		1
#define SD_RSP_DATA		2
#define SD_RSP_STREAM	3

//---------------------------------------------------------------
// Send / receive timeouts, to stop infinite wait loops
#define IRQ_TIMEOUT 1000000
#define RESET_TIMEOUT 1000
#define COMMAND_TIMEOUT 10000
#define MAX_STARTUP_TRIES 2000	// Arbitrary value, check if the card is ready 20 times before giving up
#define WRITE_TIMEOUT	300	// Time to wait for the card to finish writing


static const u8 _NJSD_read_cmd[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x40};
static const u8 _NJSD_read_end_cmd[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x4C, 0x00, 0x41};

static int _NJSD_speed = SD_CLK_8MHz;		// Default speed;

static u32 _NJSD_cardFlags;

static u32 _NJSD_relativeCardAddress = 0;

#define _NJSD_irqFlag		(*(volatile unsigned long*)(0x027FFEB0))


static inline bool _NJSD_waitIRQ(void) {
/*#ifdef _NJSD_SYNC
	int i = IRQ_TIMEOUT;
	while (!(REG_IF & 0x100000) && --i);
	REG_IF = 0x100000;
	if (i <= 0) {
		return false;
	} else {
		return true;
	}
#else*/
	int i = IRQ_TIMEOUT;
	//if (!(REG_IME & 1))
	//{
		// irq's disabled...
		while (!(REG_IF & 0x100000) && (!(_NJSD_irqFlag)) && --i);
		_NJSD_irqFlag = 0;
		REG_IF = 0x100000;
		if (i <= 0) {
			return false;
		} else {
			return true;
		}
	//} else {
		// irq's enabled
	//	while (!(_NJSD_irqFlag) && --i);
	//	_NJSD_irqFlag = 0;
	//	REG_IF = 0x100000;
	//	if (i <= 0) {
	//		return false;
	//	} else {
	//		return true;
	//	}
	//}
//#endif
}

static inline void _NJSD_writeCardCommand
	(u8 cmd0, u8 cmd1, u8 cmd2, u8 cmd3, u8 cmd4, u8 cmd5, u8 cmd6, u8 cmd7)
{
	CARD_COMMAND[0] = cmd0;
	CARD_COMMAND[1] = cmd1;
	CARD_COMMAND[2] = cmd2;
	CARD_COMMAND[3] = cmd3;
	CARD_COMMAND[4] = cmd4;
	CARD_COMMAND[5] = cmd5;
	CARD_COMMAND[6] = cmd6;
	CARD_COMMAND[7] = cmd7;
}


static bool _NJSD_reset (void) {
	int i;
	CARD_CR1H = CARD_CR1_ENABLE;
	_NJSD_writeCardCommand (0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	CARD_CR2 = 0xA0406000;
	i = RESET_TIMEOUT;
	while ((CARD_CR2 & CARD_BUSY) && --i);
	if (i <= 0) {
		return false;
	}

	return true;
}

static void _NJSD_irqHandler (void) {
	_NJSD_irqFlag = 1;
}

static bool _NJSD_init (u32 flags) {
	_NJSD_cardFlags = flags;

	REG_IF = 0x100000; // Clear cart IRQ.

	_NJSD_irqFlag = 0;
#ifdef _NJSD_SYNC
    irqDisable (IRQ_CARD_LINE);
#else
    irqSet(IRQ_CARD_LINE, _NJSD_irqHandler);
    irqEnable (IRQ_CARD_LINE);
#endif

	if (! _NJSD_reset() ) {
		return false;
	}
	return true;
}

static bool _NJSD_sendCMDR (int speed, u8 *rsp_buf, int type, u8 cmd, u32 param) {
	int i;
	u32 data;

#ifdef _NJSD_SYNC
	u32 old_REG_IME;
	old_REG_IME = REG_IME;
	REG_IME = 0;
#endif

	REG_IE &= ~0x100000;
	REG_IF =   0x100000;

	CARD_CR1H = CARD_CR1_ENABLE;

	if ((type & 3) < 2) {
		CARD_COMMAND[0] = 0xF0 | (speed << 2) | 1 | (type << 1);
	} else if ((type & 3) == 2) {
		CARD_COMMAND[0] = 0xE0 | (speed << 2) | 0 | (1 << 1);
	} else {
		CARD_COMMAND[0] = 0xF0 | (speed << 2) | 0 | (1 << 1);
	}

	CARD_COMMAND[1] = (type & 0x40) | ((( type >> 2) & 7) << 3);
	CARD_COMMAND[2] = 0x40 | cmd;
	CARD_COMMAND[3] = (param>>24) & 0xFF;
	CARD_COMMAND[4] = (param>>16) & 0xFF;
	CARD_COMMAND[5] = (param>>8) & 0xFF;
	CARD_COMMAND[6] = (param>>0) & 0xFF;
	CARD_COMMAND[7] = 0; // offset = 0

	if ((type & 3) < 2) {
		CARD_CR2 = _NJSD_cardFlags | 0x01000000;

		// wait for ninja DS to be done!
		if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
			REG_IME = old_REG_IME;
#endif
			return false;
		}

		i = 0;
		do {
			// Read data if available
			if (CARD_CR2 & CARD_DATA_READY) {
				data=CARD_DATA_RD;
				if (rsp_buf != NULL) {
					if (i == 4) {
						rsp_buf[0] = (data>>0)&0xFF;
						rsp_buf[1] = (data>>8)&0xFF;
						rsp_buf[2] = (data>>16)&0xFF;
						rsp_buf[3] = (data>>24)&0xFF;
					} else if (i == 5) {
						rsp_buf[4] = (data>>0)&0xFF;
						rsp_buf[5] = (data>>8)&0xFF;
					}
				}
				i++;
			}
		} while (CARD_CR2 & CARD_BUSY);

#ifdef _NJSD_DEBUG
		iprintf ("r: ");
		for (i = 0; i < 6; i++)
			iprintf ("%02X ", rsp_buf[i]);
		iprintf ("\n");
#endif
	} else {
		CARD_CR2 = _NJSD_cardFlags;
		while (CARD_CR2 & CARD_BUSY);

		// wait for ninja DS to be done!
		if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
			REG_IME = old_REG_IME;
#endif
			return false;
		}
	}

#ifdef _NJSD_SYNC
	REG_IME = old_REG_IME;
#endif
	return true;
}

static bool _NJSD_writeSector (u8 *buffer, u8 *crc_buf, u32 offset) {
	int i;
	u8 responseBuffer[6];
	u32 data;

#ifdef _NJSD_SYNC
	u32 old_REG_IME;
	old_REG_IME = REG_IME;
	REG_IME = 0;
#endif

	CARD_CR1H = CARD_CR1_ENABLE;
	_NJSD_writeCardCommand (0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00);
	CARD_CR2 = 0xA0406000;
	i = COMMAND_TIMEOUT;
	while ((CARD_CR2 & CARD_BUSY) && --i);
	if (i <= 0) {
#ifdef _NJSD_SYNC
		REG_IME = old_REG_IME;
#endif
		return false;
	}

	for (i = 0; i < 65; i++)
	{
		CARD_CR1H = CARD_CR1_ENABLE; // | CARD_CR1_IRQ;
		if (i < 64)
		{
			_NJSD_writeCardCommand (buffer[i*8+0], buffer[i*8+1], buffer[i*8+2], buffer[i*8+3],
				buffer[i*8+4], buffer[i*8+5], buffer[i*8+6], buffer[i*8+7]);
		} else {
			_NJSD_writeCardCommand (crc_buf[0], crc_buf[1], crc_buf[2], crc_buf[3],
				crc_buf[4], crc_buf[5], crc_buf[6], crc_buf[7]);
		}
		CARD_CR2 = 0xA7406000;

		do {
			// Read data if available
			if (CARD_CR2 & CARD_DATA_READY) {
				data=CARD_DATA_RD;
			}
		} while (CARD_CR2 & CARD_BUSY);
	}


	REG_IE &= ~0x100000;
	REG_IF =   0x100000;

	CARD_CR1H = CARD_CR1_ENABLE;
	_NJSD_writeCardCommand (0xF0 | (1 << 2) | 1, 0x80, 0x40 | WRITE_BLOCK, (u8)(offset>>24),
		(u8)(offset>>16), (u8)(offset>>8), (u8)(offset>>0), 0x00);
	CARD_CR2 = 0xA7406000;

	// wait for ninja DS to be done!
	if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
		REG_IME = old_REG_IME;
#endif
		return false;
	}

	i = 0;
	do {
		// Read data if available
		if (CARD_CR2 & CARD_DATA_READY) {
			data = CARD_DATA_RD;
			if (i == 2) {
				responseBuffer[0] = (u8)(data>>0);
				responseBuffer[1] = (u8)(data>>8);
				responseBuffer[2] = (u8)(data>>16);
				responseBuffer[3] = (u8)(data>>24);
			} else if (i == 3) {
				responseBuffer[4] = (u8)(data>>0);
				responseBuffer[5] = (u8)(data>>8);
			}
			i++;
		}
	} while (CARD_CR2 & CARD_BUSY);

	i = WRITE_TIMEOUT;
	responseBuffer[3] = 0;
	do {
		_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, SEND_STATUS, _NJSD_relativeCardAddress);
		i--;
		if (i <= 0) {
#ifdef _NJSD_SYNC
			REG_IME = old_REG_IME;
#endif
			return false;
		}
	} while (((responseBuffer[3] & 0x1f) != ((SD_STATE_TRAN << 1) | READY_FOR_DATA)));


#ifdef _NJSD_SYNC
	REG_IME = old_REG_IME;
#endif

	return true;
}

static bool _NJSD_sendCLK (int speed, int count) {
	int i;

#ifdef _NJSD_SYNC
	u32 old_REG_IME;
	old_REG_IME = REG_IME;
	REG_IME = 0;

#endif
	REG_IE &= ~0x100000;
	REG_IF =   0x100000;

	//CARD_CR1H = CARD_CR1_ENABLE; // | CARD_CR1_IRQ;
	_NJSD_writeCardCommand (0xE0 | ((speed & 3) << 2), 0, (count - 1), 0, 0, 0, 0, 0);

	CARD_CR2 = _NJSD_cardFlags;
	i = COMMAND_TIMEOUT;
	while ((CARD_CR2 & CARD_BUSY) && --i);
	if (i <= 0) {
#ifdef _NJSD_SYNC
		REG_IME = old_REG_IME;
#endif
		return false;
	}

	// wait for ninja DS to be done!
	if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
	REG_IME = old_REG_IME;
#endif
		return false;
	}

#ifdef _NJSD_SYNC
	REG_IME = old_REG_IME;
#endif
	return true;
}

static bool _NJSD_sendCMDN (int speed, u8 cmd, u32 param) {
	int i;

#ifdef _NJSD_SYNC
	u32 old_REG_IME;
	old_REG_IME = REG_IME;
	REG_IME = 0;
#endif

	REG_IE &= ~0x100000;
	REG_IF =   0x100000;

	CARD_CR1H = CARD_CR1_ENABLE; // | CARD_CR1_IRQ;
	_NJSD_writeCardCommand (0xF0 | ((speed & 3) << 2), 0x00, 0x40 | cmd, (param>>24) & 0xFF,
		(param>>16) & 0xFF, (param>>8) & 0xFF, (param>>0) & 0xFF, 0x00);

	CARD_CR2 = _NJSD_cardFlags;
	i = COMMAND_TIMEOUT;
	while ((CARD_CR2 & CARD_BUSY) && --i);
	if (i <= 0) {
#ifdef _NJSD_SYNC
		REG_IME = old_REG_IME;
#endif
		return false;
	}

	// wait for ninja DS to be done!
	if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
	REG_IME = old_REG_IME;
#endif
		return false;
	}

#ifdef _NJSD_SYNC
	REG_IME = old_REG_IME;
#endif
	return true;
}

static bool _NJSD_cardInit (void) {
	u8 responseBuffer[17];
	int i;

	// If the commands succeed the first time, assume they'll always succeed
	if (! _NJSD_sendCLK (SD_CLK_200KHz, 256) ) return false;
	if (! _NJSD_sendCMDN (SD_CLK_200KHz, GO_IDLE_STATE, 0) ) return false;
	_NJSD_sendCLK (SD_CLK_200KHz, 8);

	_NJSD_sendCLK (SD_CLK_200KHz, 256);
 	_NJSD_sendCMDN (SD_CLK_200KHz, GO_IDLE_STATE, 0);
	_NJSD_sendCLK (SD_CLK_200KHz, 8);

	for (i = 0; i < MAX_STARTUP_TRIES ; i++) {
		_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, APP_CMD, 0);
		if (
			_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, SD_APP_OP_COND, SD_OCR_VALUE) &&
			((responseBuffer[1] & 0x80) != 0))
		{
			// Card is ready to receive commands now
			break;
		}
	}
	if (i >= MAX_STARTUP_TRIES) {
		return false;
	}

	// The card's name, as assigned by the manufacturer
	_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_136, ALL_SEND_CID, 0);

	// Get a new address
	for (i = 0; i < MAX_STARTUP_TRIES ; i++) {
		_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, SEND_RELATIVE_ADDR, 0);
		_NJSD_relativeCardAddress = (responseBuffer[1] << 24) | (responseBuffer[2] << 16);
		if ((responseBuffer[3] & 0x1e) != (SD_STATE_STBY << 1)) {
			break;
		}
	}
 	if (i >= MAX_STARTUP_TRIES) {
		return false;
	}

	// Some cards won't go to higher speeds unless they think you checked their capabilities
	_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_136, SEND_CSD, _NJSD_relativeCardAddress);

	// Only this card should respond to all future commands
	_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, SELECT_CARD, _NJSD_relativeCardAddress);

	// Set a 4 bit data bus
	_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, APP_CMD, _NJSD_relativeCardAddress);
	_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, SET_BUS_WIDTH, 2); // 4-bit mode.

	// Use 512 byte blocks
	_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, SET_BLOCKLEN, 512); // 512 byte blocks

	return true;
}


bool _NJSD_isInserted(void) {
	u8 responseBuffer [8];
	_NJSD_sendCMDR (SD_CLK_200KHz, responseBuffer, SD_RSP_48, SEND_STATUS, 0);

	// Make sure the card responded correctly
	if (responseBuffer[0] != SEND_STATUS) {
		return false;
	}
	return true;
}

bool _NJSD_clearStatus (void) {
	return _NJSD_reset();
}

bool _NJSD_shutdown(void) {
	return _NJSD_clearStatus();
}

bool _NJSD_startup(void) {
	if (! _NJSD_init(0xA0406000) ) {
		return false;
	}
	if (! _NJSD_cardInit() ) {
		return false;
	}
	return true;
}


bool _NJSD_writeSectors (u32 sector, u32 numSectors, const void* buffer) {
 	u8 crc[8];
	u32 offset = sector * BYTES_PER_READ;
	u8* data = (u8*) buffer;

	while (numSectors--) {
		_SD_CRC16 ( data, BYTES_PER_READ, crc);

		if (! _NJSD_writeSector (data, crc, offset)) {
			return false;
		}
		offset += BYTES_PER_READ;
		data += BYTES_PER_READ;
	}
	return true;
}

#ifdef _IO_ALLOW_UNALIGNED
bool _NJSD_readSectors (u32 sector, u32 numSectors, void* buffer) {
	u32 tmp[BYTES_PER_READ>>2];
	int i;

#ifdef _NJSD_SYNC
	u32 old_REG_IME;
#endif

	u8* tbuf = (u8*)buffer;

	if (numSectors == 0) {
		return false;
	}

#ifdef _NJSD_SYNC
	old_REG_IME = REG_IME;
	REG_IME = 0;
#endif

	if (numSectors > 1) {
		_NJSD_sendCMDR (_NJSD_speed, NULL, SD_RSP_DATA, READ_MULTIPLE_BLOCK, sector * BYTES_PER_READ);
		for (i = 0; i < numSectors - 2; i++) {
			if (((int)buffer & 0x03) != 0){
				cardPolledTransfer (0xA1406000, tmp, BYTES_PER_READ, (u8*)_NJSD_read_cmd);
				memcpy (tbuf + i * BYTES_PER_READ, tmp, BYTES_PER_READ);
			} else {
				cardPolledTransfer (0xA1406000, (u32*)(tbuf + i * BYTES_PER_READ), BYTES_PER_READ, (u8*)_NJSD_read_cmd);
			}
			if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
				REG_IME = old_REG_IME;
#endif
				return false;
			}
		}
		if (((int)buffer & 0x03) != 0){
			cardPolledTransfer (0xA1406000, tmp, BYTES_PER_READ, (u8*)_NJSD_read_end_cmd);
			memcpy (tbuf + (numSectors - 2) * BYTES_PER_READ, tmp, BYTES_PER_READ);
		} else {
			cardPolledTransfer (0xA1406000, (u32*)(tbuf + (numSectors - 2) * BYTES_PER_READ), BYTES_PER_READ, (u8*)_NJSD_read_end_cmd);
		}
		if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
			REG_IME = old_REG_IME;
#endif
			return false;
		}

		if (((int)buffer & 0x03) != 0){
			cardPolledTransfer (0xA1406000, tmp, BYTES_PER_READ, (u8*)_NJSD_read_cmd);
			memcpy (tbuf + (numSectors - 1) * BYTES_PER_READ, tmp, BYTES_PER_READ);
		} else {
			cardPolledTransfer (0xA1406000, (u32*)(tbuf + (numSectors - 1) * BYTES_PER_READ), BYTES_PER_READ, (u8*)_NJSD_read_cmd);
		}
	} else {
		_NJSD_sendCMDR (_NJSD_speed, NULL, SD_RSP_STREAM, READ_SINGLE_BLOCK, sector * BYTES_PER_READ);
		if (((int)buffer & 0x03) != 0){
			cardPolledTransfer (0xA1406000, tmp, BYTES_PER_READ, (u8*)_NJSD_read_cmd);
			memcpy (tbuf, tmp, BYTES_PER_READ);
		} else {
			cardPolledTransfer (0xA1406000, (u32*)tbuf, BYTES_PER_READ, (u8*)_NJSD_read_cmd);
		}
	}

#ifdef _NJSD_SYNC
	REG_IME = old_REG_IME;
#endif
	return true;
}
#else // not defined _IO_ALLOW_UNALIGNED
bool _NJSD_readSectors (u32 sector, u32 numSectors, void* buffer) {
	int i;

#ifdef _NJSD_SYNC
	u32 old_REG_IME;
#endif

	u8* tbuf = (u8*)buffer;

	if (numSectors == 0) {
		return false;
	}

#ifdef _NJSD_SYNC
	old_REG_IME = REG_IME;
	REG_IME = 0;
#endif

	if (numSectors > 1) {
		_NJSD_sendCMDR (_NJSD_speed, NULL, SD_RSP_DATA, READ_MULTIPLE_BLOCK, sector * BYTES_PER_READ);
		for (i = 0; i < numSectors - 2; i++) {
			cardPolledTransfer (0xA1406000, (u32*)(tbuf + i * BYTES_PER_READ), BYTES_PER_READ, (u8*)_NJSD_read_cmd);
			if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
				REG_IME = old_REG_IME;
#endif
				return false;
			}
		}
		cardPolledTransfer (0xA1406000, (u32*)(tbuf + (numSectors - 2) * BYTES_PER_READ), BYTES_PER_READ, (u8*)_NJSD_read_end_cmd);
		if (!_NJSD_waitIRQ ()) {
#ifdef _NJSD_SYNC
			REG_IME = old_REG_IME;
#endif
			return false;
		}

		cardPolledTransfer (0xA1406000, (u32*)(tbuf + (numSectors - 1) * BYTES_PER_READ), BYTES_PER_READ, (u8*)_NJSD_read_cmd);
	} else {
		_NJSD_sendCMDR (_NJSD_speed, NULL, SD_RSP_STREAM, READ_SINGLE_BLOCK, sector * BYTES_PER_READ);
		cardPolledTransfer (0xA1406000, (u32*)tbuf, BYTES_PER_READ, (u8*)_NJSD_read_cmd);
	}

#ifdef _NJSD_SYNC
	REG_IME = old_REG_IME;
#endif
	return true;
}
#endif // _IO_ALLOW_UNALIGNED

IO_INTERFACE io_njsd = {
	DEVICE_TYPE_NJSD,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE | FEATURE_SLOT_NDS,
	(FN_MEDIUM_STARTUP)&_NJSD_startup,
	(FN_MEDIUM_ISINSERTED)&_NJSD_isInserted,
	(FN_MEDIUM_READSECTORS)&_NJSD_readSectors,
	(FN_MEDIUM_WRITESECTORS)&_NJSD_writeSectors,
	(FN_MEDIUM_CLEARSTATUS)&_NJSD_clearStatus,
	(FN_MEDIUM_SHUTDOWN)&_NJSD_shutdown
} ;

LPIO_INTERFACE NJSD_GetInterface(void) {
	return &io_njsd ;
} ;

#endif // defined NDS
#endif
