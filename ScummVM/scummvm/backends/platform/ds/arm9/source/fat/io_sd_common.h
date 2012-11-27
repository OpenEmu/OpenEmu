/*
 io_sd_common.h

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

	2006-07-11 - Chishm
		* Original release

	2006-07-28 - Chishm
		* Changed voltage range that the SD card can use
*/

#ifndef IO_SD_COMMON_H
#define IO_SD_COMMON_H

#include "disc_io.h"

/* SD commands */
#define GO_IDLE_STATE 0
#define ALL_SEND_CID 2
#define SEND_RELATIVE_ADDR 3
#define SELECT_CARD 7
#define SEND_CSD 9
#define STOP_TRANSMISSION 12
#define SEND_STATUS 13
#define GO_INACTIVE_STATE 15
#define SET_BLOCKLEN 16
#define READ_SINGLE_BLOCK 17
#define READ_MULTIPLE_BLOCK 18
#define WRITE_BLOCK 24
#define WRITE_MULTIPLE_BLOCK 25
#define APP_CMD 55

/* SD App commands */
#define SET_BUS_WIDTH 6
#define SD_APP_OP_COND 41

/* OCR (Operating Conditions Register) send value */
//#define SD_OCR_VALUE 0x00030000 /* 2.8V to 3.0V */
//#define SD_OCR_VALUE 0x003F8000 /* 2.7V to 3.4V */
#define SD_OCR_VALUE 0x00FC0000

/* SD Data repsonses */
#define SD_CARD_BUSY 0xff

/* SD states */
#define SD_STATE_IDLE 0		// Idle state, after power on or GO_IDLE_STATE command
#define SD_STATE_READY 1	// Ready state, after card replies non-busy to SD_APP_OP_COND
#define SD_STATE_IDENT 2	// Identification state, after ALL_SEND_CID
#define SD_STATE_STBY 3		// Standby state, when card is deselected
#define SD_STATE_TRAN 4		// Transfer state, after card is selected and ready for data transfer
#define SD_STATE_DATA 5		//
#define SD_STATE_RCV 6		// Receive data state
#define SD_STATE_PRG 7		// Programming state
#define SD_STATE_DIS 8		// Disconnect state
#define SD_STATE_INA 9		// Inactive state, after GO_INACTIVE_STATE

#define READY_FOR_DATA 1	// bit 8 in card status

/*
Calculate the CRC7 of a command and return it preshifted with
an end bit added
*/
extern u8 _SD_CRC7(u8* data, int size);

/*
Calculate the CRC16 of a block of data, ready for transmission on
four data lines at once
*/
extern void _SD_CRC16 (u8* buff, int buffLength, u8* crc16buff);

typedef bool (*_SD_FN_CMD_6BYTE_RESPONSE) (u8* responseBuffer, u8 command, u32 data);
typedef bool (*_SD_FN_CMD_17BYTE_RESPONSE) (u8* responseBuffer, u8 command, u32 data);

/*
Initialise the SD card, after it has been sent into an Idle state
cmd_6byte_response: a pointer to a function that sends the SD card a command and gets a 6 byte response
cmd_17byte_response: a pointer to a function that sends the SD card a command and gets a 17 byte response
use4bitBus: initialise card to use a 4 bit data bus when communicating with the card
RCA: a pointer to the location to store the card's Relative Card Address, preshifted up by 16 bits.
*/
extern bool _SD_InitCard (_SD_FN_CMD_6BYTE_RESPONSE cmd_6byte_response,
							_SD_FN_CMD_17BYTE_RESPONSE cmd_17byte_response,
							bool use4bitBus,
							u32 *RCA);

#endif // define IO_SD_COMMON_H
