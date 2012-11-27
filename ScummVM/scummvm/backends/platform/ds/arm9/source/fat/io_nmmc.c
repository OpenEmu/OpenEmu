/*
	io_nmmc.c

	Hardware Routines for reading an SD or MMC card using
	a Neoflash MK2 or MK3.

	Written by www.neoflash.com

	Submit bug reports for this device to the NeoFlash forums

	This software is completely free. No warranty is provided.
	If you use it, please give me credit and email me about your
	project at chishm@hotmail.com

	See gba_nds_fat.txt for help and license details.

	2006-02-09 - www.neoflash.com:
	 * First stable release

	2006-02-13 - Chishm
	 * Added ReadMK2Config function
	 * Added read config test to init function so no unnecessary card commands are sent
	 * Changed data read and write functions to use multiple block commands
*/

#include "io_nmmc.h"

#ifdef SUPPORT_NMMC

#include <nds/card.h>

int spi_freq = 3;

#define MK2_CONFIG_ZIP_RAM_CLOSE		(1 << 5)
#define MK2_CONFIG_GAME_FLASH_CLOSE		((1 << 4) | (1 << 0))
//#define MK2_CONFIG_ZIP_RAM_CLOSE		((1 << 5) | (1 << 1))
//#define MK2_CONFIG_GAME_FLASH_CLOSE		(1 << 4)

#define MMC_READ_MULTIPLE_BLOCK		18
#define MMC_READ_BLOCK		17
#define MMC_WRITE_MULTIPLE_BLOCK	25
#define MMC_WRITE_BLOCK		24
#define MMC_STOP_TRANSMISSION	12
#define MMC_SET_BLOCKLEN	16
#define MMC_SET_BLOCK_COUNT	23
#define MMC_SEND_CSD	9

// SPI functions

static inline void Neo_OpenSPI( u8 frequency )
{
	CARD_CR1 = 0x0000A040 | frequency;
}

static inline u8 Neo_SPI( u8 dataByte )
{
	CARD_EEPDATA = dataByte;
	while (CARD_CR1 & 0x80);		// card busy
	return CARD_EEPDATA;
}

static inline void Neo_CloseSPI ( void )
{
	CARD_CR1 = 0;
}

static inline void Neo_MK2GameMode()	{
	Neo_OpenSPI(spi_freq);				// Enable DS Card's SPI port
	Neo_SPI(0xF1);				// Switch to game mode
	Neo_CloseSPI();				// Disable DS Card's SPI port
}

static inline void Neo_EnableEEPROM( bool enable )	{
	Neo_OpenSPI(spi_freq);
	if(enable)	Neo_SPI(0x06);
	else 		Neo_SPI(0x0E);
	Neo_CloseSPI();
}

void Neo_WriteMK2Config(u8 config) {
	Neo_EnableEEPROM(true);
	Neo_OpenSPI(spi_freq);
	Neo_SPI(0xFA);					// Send mem conf write command
	Neo_SPI(0x01);					// Send high byte (0x01)
	Neo_SPI(config);				// Send low byte
	Neo_CloseSPI();
	Neo_EnableEEPROM(false);
}

u8 Neo_ReadMK2Config(void)
{
	u8 config;
	Neo_EnableEEPROM(true);
	Neo_OpenSPI(spi_freq);
	Neo_SPI(0xf8);					// Send mem conf read command
	Neo_SPI(0x01);					// Send high byte
	config = Neo_SPI(0x00);			// Get low byte
	Neo_CloseSPI();
	Neo_EnableEEPROM(false);
	return config;
}

// Low level functions

u8 selectMMC_command [8] = {0xFF, 0x00, 0x6A, 0xDF, 0x37, 0x59, 0x33, 0xA3};

void Neo_SelectMMC (u8 dataByte)
{
	selectMMC_command[1] = dataByte;	// Set enable / disable byte
	cardWriteCommand (selectMMC_command);	// Send "5. Use the EEPROM CS to access the MK2 MMC/SD card"
	CARD_CR2 = CARD_ACTIVATE | CARD_nRESET;
	while (CARD_CR2 & CARD_BUSY);
	return;
}

void Neo_EnableMMC( bool enable )
{
	if ( enable == false) {
		Neo_CloseSPI ();
		Neo_SelectMMC (0);
		Neo_SelectMMC (0);
	} else {
		Neo_SelectMMC (1);
		Neo_SelectMMC (1);
		Neo_OpenSPI (spi_freq);
	}
	return;
}

void Neo_SendMMCCommand( u8 command, u32 argument )
{
	Neo_SPI (0xFF);
	Neo_SPI (command | 0x40);
	Neo_SPI ((argument >> 24) & 0xff);
	Neo_SPI ((argument >> 16) & 0xff);
	Neo_SPI ((argument >> 8) & 0xff) ;
	Neo_SPI (argument & 0xff);
	Neo_SPI (0x95);
	Neo_SPI (0xFF);
	return;
}

bool Neo_CheckMMCResponse( u8 response, u8 mask )	{
	u32 i;
	for(i=0;i<256;i++)	{
		if( ( Neo_SPI( 0xFF ) & mask ) == response )
			return true;
	}
	return false;
}

// Neo MMC functions

bool Neo_InitMMC()	{
	Neo_MK2GameMode();
	Neo_WriteMK2Config( MK2_CONFIG_ZIP_RAM_CLOSE | MK2_CONFIG_GAME_FLASH_CLOSE);

	// Make sure the configuration was accepted
	if (Neo_ReadMK2Config() != (MK2_CONFIG_ZIP_RAM_CLOSE | MK2_CONFIG_GAME_FLASH_CLOSE)) {
		return false;	// If not, then it wasn't initialised properly
	}

	return true;
}

// Neo MMC driver functions

bool NMMC_IsInserted(void)	{
	int i;

	Neo_EnableMMC( true );		// Open SPI port to MMC card
	Neo_SendMMCCommand(MMC_SEND_CSD, 0);
	if( Neo_CheckMMCResponse( 0x00, 0xFF ) == false )	{				// Make sure no errors occurred
		Neo_EnableMMC( false );
		return false;
	}
	if( Neo_CheckMMCResponse( 0xFE, 0xFF ) == false )	{			// Check for Start Block token
		Neo_EnableMMC( false );
		return false;
	}

	// consume data from card, and send clocks.
	for (i = 0; i < 28; i++) {
		Neo_SPI(0xff);
	}

	return true;
}

bool NMMC_ClearStatus (void) {
	u32 i;

	Neo_EnableMMC( true );		// Open SPI port to MMC card
	for (i = 0; i < 10; i++) {
		Neo_SPI(0xFF);			// Send 10 0xFF bytes to MMC card
	}
	Neo_SendMMCCommand(0, 0);	// Send GO_IDLE_STATE command
	if( Neo_CheckMMCResponse( 0x01, 0xFF ) == false )	{		// Check that it replied with 0x01 (not idle, no other error)
		Neo_EnableMMC( false );
		return false;
	}
	for(i=0;i<256;i++)	{
		Neo_SendMMCCommand(1, 0);	// Poll with SEND_OP_COND
		if( Neo_CheckMMCResponse( 0x00, 0x01 ) == true ) {	// Check for idle state
			Neo_EnableMMC( false );				// Close SPI port to MMC card
			return true;						// Card is now idle
		}
	}
	Neo_EnableMMC( false );
	return false;
}

bool NMMC_Shutdown(void) {
	return NMMC_ClearStatus();
}

bool NMMC_StartUp(void) {
	int i;
	int transSpeed;
	if (Neo_InitMMC() == false) {
		return false;
	}
	if (NMMC_ClearStatus() == false) {
		return false;
	}
	Neo_EnableMMC( true );		// Open SPI port to MMC card

	// Set block length
	Neo_SendMMCCommand(MMC_SET_BLOCKLEN, BYTE_PER_READ );
	if( Neo_CheckMMCResponse( 0x00, 0xFF ) == false )	{				// Make sure no errors occurred
		Neo_EnableMMC( false );
		return false;
	}

	// Check if we can use a higher SPI frequency
	Neo_SendMMCCommand(MMC_SEND_CSD, 0);
	if( Neo_CheckMMCResponse( 0x00, 0xFF ) == false )	{				// Make sure no errors occurred
		Neo_EnableMMC( false );
		return false;
	}
	if( Neo_CheckMMCResponse( 0xFE, 0xFF ) == false )	{			// Check for Start Block token
		Neo_EnableMMC( false );
		return false;
	}
	for (i = 0; i < 3; i++) {
		Neo_SPI(0xFF);
	}
	transSpeed = Neo_SPI (0xFF);
	for (i = 0; i < 24; i++) {
		Neo_SPI(0xFF);
	}
	if ((transSpeed & 0xf0) >= 0x30) {
		spi_freq = 0;
	}

	Neo_EnableMMC( false );
	return true;
}


bool NMMC_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	u32 i;
	u8 *p=buffer;

	int totalSecs = (numSecs == 0) ? 256 : numSecs;
	sector *= BYTE_PER_READ;

	Neo_EnableMMC( true );												// Open SPI port to MMC card
	Neo_SendMMCCommand( 25, sector );
	if( Neo_CheckMMCResponse( 0x00, 0xFF ) == false )	{				// Make sure no errors occurred
		Neo_EnableMMC( false );
		return false;
	}

	while (totalSecs--) {
		Neo_SPI( 0xFC );												// Send Start Block token
		for( i = 0; i < BYTE_PER_READ; i++ )							// Send a block of data
			Neo_SPI( *p++ );
		Neo_SPI( 0xFF );												// Send fake CRC16
		Neo_SPI( 0xFF );												// Send fake CRC16

		if( ( Neo_SPI( 0xFF ) & 0x0F ) != 0x05 )	{					// Make sure the block was accepted
			Neo_EnableMMC( false );
			return false;
		}
		while( Neo_SPI( 0xFF ) == 0x00 );								// Wait for the block to be written
	}

	// Stop transmission block
	Neo_SPI( 0xFD );													// Send Stop Transmission Block token
	for( i = 0; i < BYTE_PER_READ; i++ )								// Send a block of fake data
		Neo_SPI( 0xFF );
	Neo_SPI( 0xFF );													// Send fake CRC16
	Neo_SPI( 0xFF );													// Send fake CRC16

	Neo_SPI (0xFF); 													// Send 8 clocks
	while( Neo_SPI( 0xFF ) == 0x00 );									// Wait for the busy signal to clear


	for ( i = 0; i < 0x10; i++) {
		Neo_SPI (0xFF);													// Send clocks for the MMC card to finish what it's doing
	}

	Neo_EnableMMC( false );											// Close SPI port to MMC card
	return true;
}

bool NMMC_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
	u32 i;
	u8 *p=buffer;

	int totalSecs = (numSecs == 0) ? 256 : numSecs;
	sector *= BYTE_PER_READ;

	Neo_EnableMMC( true );												// Open SPI port to MMC card

	while (totalSecs--) {
		Neo_SendMMCCommand(MMC_READ_BLOCK, sector );
		if( Neo_CheckMMCResponse( 0x00, 0xFF ) == false )	{			// Make sure no errors occurred
			Neo_EnableMMC( false );
			return false;
		}

		if( Neo_CheckMMCResponse( 0xFE, 0xFF ) == false )	{			// Check for Start Block token
			Neo_EnableMMC( false );
			return false;
		}
		for( i = 0; i < BYTE_PER_READ; i++ )							// Read in a block of data
			*p++ = Neo_SPI( 0xFF );
		Neo_SPI( 0xFF );												// Ignore CRC16
		Neo_SPI( 0xFF );												// Ignore CRC16
		sector += BYTE_PER_READ;
	}

	Neo_EnableMMC( false );											// Close SPI port to MMC card
	return true;
}


IO_INTERFACE io_nmmc = {
	DEVICE_TYPE_NMMC,
	FEATURE_MEDIUM_CANREAD | FEATURE_MEDIUM_CANWRITE,
	(FN_MEDIUM_STARTUP)&NMMC_StartUp,
	(FN_MEDIUM_ISINSERTED)&NMMC_IsInserted,
	(FN_MEDIUM_READSECTORS)&NMMC_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&NMMC_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&NMMC_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&NMMC_Shutdown
} ;


LPIO_INTERFACE NMMC_GetInterface(void) {
	return &io_nmmc ;
}

#endif	// #ifdef SUPPORT_NMMC
