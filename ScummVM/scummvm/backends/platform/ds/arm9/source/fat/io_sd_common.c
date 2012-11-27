/*
	io_sd_common.c

	By chishm (Michael Chisholm)

	Common SD card routines

	SD routines partially based on sd.s by Romman

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

	2006-08-07 - Chishm
		* Moved the SD initialization to a common function
		* Increased timeouts for slower cards
*/

#include "io_sd_common.h"

#define MAX_STARTUP_TRIES 1000	// Arbitrary value, check if the card is ready 20 times before giving up
#define RESPONSE_TIMEOUT 256	// Number of clocks sent to the SD card before giving up

/*
Improved CRC7 function provided by cory1492
Calculates the CRC of an SD command, and includes the end bit in the byte
*/
u8 _SD_CRC7(u8* data, int cnt) {
    int i, a;
    u8 crc, temp;

    crc = 0;
    for (a = 0; a < cnt; a++)
    {
        temp = data[a];
        for (i = 0; i < 8; i++)
        {
            crc <<= 1;
            if ((temp & 0x80) ^ (crc & 0x80)) crc ^= 0x09;
            temp <<= 1;
        }
    }
    crc = (crc << 1) | 1;
    return(crc);
}

/*
Calculates the CRC16 for a sector of data. Calculates it
as 4 separate lots, merged into one buffer. This is used
for 4 SD data lines, not for 1 data line alone.
*/
void _SD_CRC16 (u8* buff, int buffLength, u8* crc16buff) {
	u32 a, b, c, d;
	int count;
	u32 bitPattern = 0x80808080;	// r7
	u32 crcConst = 0x1021;	// r8
	u32 dataByte = 0;	// r2

	a = 0;	// r3
	b = 0;	// r4
	c = 0;	// r5
	d = 0;	// r6

	buffLength = buffLength * 8;


	do {
		if (bitPattern & 0x80) dataByte = *buff++;

		a = a << 1;
		if ( a & 0x10000) a ^= crcConst;
		if (dataByte & (bitPattern >> 24)) a ^= crcConst;

		b = b << 1;
		if (b & 0x10000) b ^= crcConst;
		if (dataByte & (bitPattern >> 25)) b ^= crcConst;

		c = c << 1;
		if (c & 0x10000) c ^= crcConst;
		if (dataByte & (bitPattern >> 26)) c ^= crcConst;

		d = d << 1;
		if (d & 0x10000) d ^= crcConst;
		if (dataByte & (bitPattern >> 27)) d ^= crcConst;

		bitPattern = (bitPattern >> 4) | (bitPattern << 28);
	} while (buffLength-=4);

	count = 16;	// r8

	do {
		bitPattern = bitPattern << 4;
		if (a & 0x8000) bitPattern |= 8;
		if (b & 0x8000) bitPattern |= 4;
		if (c & 0x8000) bitPattern |= 2;
		if (d & 0x8000) bitPattern |= 1;

		a = a << 1;
		b = b << 1;
		c = c << 1;
		d = d << 1;

		count--;

		if (!(count & 0x01)) {
			*crc16buff++ = (u8)(bitPattern & 0xff);
		}
	} while (count != 0);

	return;
}

/*
Initialise the SD card, after it has been sent into an Idle state
cmd_6byte_response: a pointer to a function that sends the SD card a command and gets a 6 byte response
cmd_17byte_response: a pointer to a function that sends the SD card a command and gets a 17 byte response
use4bitBus: initialise card to use a 4 bit data bus when communicating with the card
RCA: a pointer to the location to store the card's Relative Card Address, preshifted up by 16 bits.
*/
bool _SD_InitCard (_SD_FN_CMD_6BYTE_RESPONSE cmd_6byte_response,
					_SD_FN_CMD_17BYTE_RESPONSE cmd_17byte_response,
					bool use4bitBus,
					u32 *RCA)
{
	u8 responseBuffer[17] = {0};
	int i;

	for (i = 0; i < MAX_STARTUP_TRIES ; i++) {
		cmd_6byte_response (responseBuffer, APP_CMD, 0);
		if (
			cmd_6byte_response (responseBuffer, SD_APP_OP_COND, SD_OCR_VALUE) &&
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
	cmd_17byte_response (responseBuffer, ALL_SEND_CID, 0);

	// Get a new address
	for (i = 0; i < MAX_STARTUP_TRIES ; i++) {
		cmd_6byte_response (responseBuffer, SEND_RELATIVE_ADDR, 0);
		*RCA = (responseBuffer[1] << 24) | (responseBuffer[2] << 16);
		if ((responseBuffer[3] & 0x1e) != (SD_STATE_STBY << 1)) {
			break;
		}
	}
 	if (i >= MAX_STARTUP_TRIES) {
		return false;
	}

	// Some cards won't go to higher speeds unless they think you checked their capabilities
	cmd_17byte_response (responseBuffer, SEND_CSD, *RCA);

	// Only this card should respond to all future commands
	cmd_6byte_response (responseBuffer, SELECT_CARD, *RCA);

	if (use4bitBus) {
		// Set a 4 bit data bus
		cmd_6byte_response (responseBuffer, APP_CMD, *RCA);
		cmd_6byte_response (responseBuffer, SET_BUS_WIDTH, 2); // 4-bit mode.
	}

	// Use 512 byte blocks
	cmd_6byte_response (responseBuffer, SET_BLOCKLEN, 512); // 512 byte blocks

	// Wait until card is ready for data
	i = 0;
	do {
		if (i >= RESPONSE_TIMEOUT) {
			return false;
		}
		i++;
	} while (!cmd_6byte_response (responseBuffer, SEND_STATUS, *RCA) && ((responseBuffer[3] & 0x1f) != ((SD_STATE_TRAN << 1) | READY_FOR_DATA)));

	return true;
}
