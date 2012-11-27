/*
io_efa2.c by CyteX

Based on io_mpfc.c by chishm (Michael Chisholm)

Hardware Routines for reading the NAND flash located on
EFA2 flash carts

This software is completely free. No warranty is provided.
If you use it, please give me credit and email me about your
project at cytex <at> gmx <dot> de and do not forget to also
drop chishm <at> hotmail <dot> com a line

See gba_nds_fat.txt for help and license details.
*/

#include "io_efa2.h"

#ifdef SUPPORT_EFA2

//
// EFA2 register addresses
//

// RTC registers
#define REG_RTC_CLK        *(vu16*)0x080000c4
#define REG_RTC_EN         *(vu16*)0x080000c8

// "Magic" registers used for unlock/lock sequences
#define REG_EFA2_MAGIC_A   *(vu16*)0x09fe0000
#define REG_EFA2_MAGIC_B   *(vu16*)0x08000000
#define REG_EFA2_MAGIC_C   *(vu16*)0x08020000
#define REG_EFA2_MAGIC_D   *(vu16*)0x08040000
#define REG_EFA2_MAGIC_E   *(vu16*)0x09fc0000

// NAND flash lock/unlock register
#define REG_EFA2_NAND_LOCK *(vu16*)0x09c40000
// NAND flash enable register
#define REG_EFA2_NAND_EN   *(vu16*)0x09400000
// NAND flash command write register
#define REG_EFA2_NAND_CMD   *(vu8*)0x09ffffe2
// NAND flash address/data write register
#define REG_EFA2_NAND_WR    *(vu8*)0x09ffffe0
// NAND flash data read register
#define REG_EFA2_NAND_RD    *(vu8*)0x09ffc000

// ID of Samsung K9K1G NAND flash chip
#define EFA2_NAND_ID 0xEC79A5C0

// first sector of udisk
#define EFA2_UDSK_START 0x40

//
// EFA2 access functions
//

// deactivate RTC ports
inline void efa2_rtc_deactivate(void) {
	REG_RTC_EN = 0;
}

// unlock register access
void efa2_reg_unlock(void) {
	REG_EFA2_MAGIC_A = 0x0d200;
	REG_EFA2_MAGIC_B = 0x01500;
	REG_EFA2_MAGIC_C = 0x0d200;
	REG_EFA2_MAGIC_D = 0x01500;
}

// finish/lock register access
inline void efa2_reg_lock(void) {
	REG_EFA2_MAGIC_E = 0x1500;
}

// global reset/init/enable/unlock ?
void efa2_global_unlock(void) {
	efa2_reg_unlock();
	*(vu16*)0x09880000 = 0x08000;
	efa2_reg_lock();
}

// global lock, stealth mode
void efa2_global_lock(void) {
	// quite sure there is such a sequence, but haven't had
	// a look for it upto now
}

// unlock NAND Flash
void efa2_nand_unlock(void) {
	efa2_reg_unlock();
	REG_EFA2_NAND_LOCK = 0x01500;
	efa2_reg_lock();
}

// lock NAND Flash
void efa2_nand_lock(void) {
	efa2_reg_unlock();
	REG_EFA2_NAND_LOCK = 0x0d200;
	efa2_reg_lock();
}

//
// Set NAND Flash chip enable and write protection bits ?
//
//   val | ~CE | ~WP |
//  -----+-----+-----+
//     0 |  0  |  0  |
//     1 |  1  |  0  |
//     3 |  1  |  1  |
//  -----+-----+-----+
//
void efa2_nand_enable(u16 val) {
	efa2_reg_unlock();
	REG_EFA2_NAND_EN = val;
	efa2_reg_lock();
}

//
// Perform NAND reset
// NAND has to be unlocked and enabled when called
//
inline void efa2_nand_reset(void) {
	REG_EFA2_NAND_CMD = 0xff; // write reset command
}

//
// Read out NAND ID information, could be used for card detection
//
//                    | EFA2 1GBit |
//  ------------------+------------+
//         maker code |    0xEC    |
//        device code |    0x79    |
//         don't care |    0xA5    |
//   multi plane code |    0xC0    |
//  ------------------+------------+
//
u32 efa2_nand_id(void) {
	u8 byte;
	u32 id;

	efa2_nand_unlock();
	efa2_nand_enable(1);

	REG_EFA2_NAND_CMD = 0x90;  // write id command
	REG_EFA2_NAND_WR  = 0x00;  // (dummy) address cycle
	byte = REG_EFA2_NAND_RD;   // read maker code
	id   = byte;
	byte = REG_EFA2_NAND_RD;   // read device code
	id   = (id << 8) | byte;
	byte = REG_EFA2_NAND_RD;   // read don't care
	id   = (id << 8) | byte;
	byte = REG_EFA2_NAND_RD;   // read multi plane code
	id   = (id << 8) | byte;

	efa2_nand_enable(0);
	efa2_nand_lock();
	return (id);
}

//
// Start of gba_nds_fat block device description
//

/*-----------------------------------------------------------------
EFA2_ClearStatus
Reads and checks NAND status information
bool return OUT:  true if NAND is idle
-----------------------------------------------------------------*/
bool EFA2_ClearStatus (void)
{
	// tbd: currently there is no write support, so always return
	// true, there is no possibility for pending operations
	return true;
}

/*-----------------------------------------------------------------
EFA2_IsInserted
Checks to see if the NAND chip used by the EFA2 is present
bool return OUT:  true if the correct NAND chip is found
-----------------------------------------------------------------*/
bool EFA2_IsInserted (void)
{
	EFA2_ClearStatus();
	return (efa2_nand_id() == EFA2_NAND_ID);
}

/*-----------------------------------------------------------------
EFA2_ReadSectors
Read "numSecs" 512 byte sectors starting from "sector" into "buffer"
No error correction, no use of spare cells, no use of R/~B signal
u32 sector IN: number of first 512 byte sector to be read
u8 numSecs IN: number of 512 byte sectors to read,
1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool EFA2_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
	int  i;
	int  j = (numSecs > 0 ? numSecs : 256);

#ifndef _CF_ALLOW_UNALIGNED
	u8  byte;
	u16  word;
#endif

	// NAND page 0x40 (EFA2_UDSK_START) contains the MBR of the
	// udisk and thus is sector 0. The original EFA2 firmware
	// does never look at this, it only watches page 0x60, which
	// contains the boot block of the FAT16 partition. That is
	// fixed, so the EFA2 udisk must not be reformated, else
	// the ARK Octopus and also the original Firmware won't be
	// able to access the udisk anymore and I have to write a
	// recovery tool.
	u32 page = EFA2_UDSK_START + sector;

	// future enhancement: wait for possible write operations to
	// be finisched
	if (!EFA2_ClearStatus()) return false;

	efa2_nand_unlock();
	efa2_nand_enable(1);
	efa2_nand_reset();

	// set NAND to READ1 operation mode and transfer page address
	REG_EFA2_NAND_CMD = 0x00;                // write READ1 command
	REG_EFA2_NAND_WR  = 0x00;                // write address  [7:0]
	REG_EFA2_NAND_WR  = (page      ) & 0xff; // write address [15:8]
	REG_EFA2_NAND_WR  = (page >> 8 ) & 0xff; // write address[23:16]
	REG_EFA2_NAND_WR  = (page >> 16) & 0xff; // write address[26:24]

	// Due to a bug in EFA2 design there is need to waste some cycles
	// "by hand" instead the possibility to check the R/~B port of
	// the NAND flash via a register. The RTC deactivation is only
	// there to make sure the loop won't be optimized by the compiler
	for (i=0 ; i < 3 ; i++) efa2_rtc_deactivate();

	while (j--)
	{
		// read page data
#ifdef _CF_ALLOW_UNALIGNED
		// slow byte access to RAM, but works in principle
		for (i=0 ; i < 512 ; i++)
			((u8*)buffer)[i] = REG_EFA2_NAND_RD;
#else
		// a bit faster, but DMA is not possible
		for (i=0 ; i < 256 ; i++) {
			byte = REG_EFA2_NAND_RD;   // read lo-byte
			word = byte;
			byte = REG_EFA2_NAND_RD;   // read hi-byte
			word = word | (byte << 8);
			((u16*)buffer)[i] = word;
		}
#endif
	}

	efa2_nand_enable(0);
	efa2_nand_lock();
	return true;
}


/*-----------------------------------------------------------------
EFA2_WriteSectors
Write "numSecs" 512 byte sectors starting at "sector" from "buffer"
u32 sector IN: address of 512 byte sector on card to write
u8 numSecs IN: number of 512 byte sectors to write
1 to 256 sectors can be written, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool EFA2_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	// Upto now I focused on reading NAND, write operations
	// will follow
	return false;
}

/*-----------------------------------------------------------------
EFA2_Shutdown
unload the EFA2 interface
-----------------------------------------------------------------*/
bool EFA2_Shutdown(void)
{
	return EFA2_ClearStatus();
}

/*-----------------------------------------------------------------
EFA2_StartUp
initializes the EFA2 card, returns true if successful,
otherwise returns false
-----------------------------------------------------------------*/
bool EFA2_StartUp(void)
{
	efa2_global_unlock();
	return (efa2_nand_id() == EFA2_NAND_ID);
}

/*-----------------------------------------------------------------
the actual interface structure
-----------------------------------------------------------------*/
IO_INTERFACE io_efa2 = {
	DEVICE_TYPE_EFA2,
	FEATURE_MEDIUM_CANREAD | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&EFA2_StartUp,
	(FN_MEDIUM_ISINSERTED)&EFA2_IsInserted,
	(FN_MEDIUM_READSECTORS)&EFA2_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&EFA2_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&EFA2_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&EFA2_Shutdown
};

/*-----------------------------------------------------------------
EFA2_GetInterface
returns the interface structure to host
-----------------------------------------------------------------*/
LPIO_INTERFACE EFA2_GetInterface(void) {
	return &io_efa2;
}

#endif // SUPPORT_EFA2
/*
io_efa2.c by CyteX

Based on io_mpfc.c by chishm (Michael Chisholm)

Hardware Routines for reading the NAND flash located on
EFA2 flash carts

This software is completely free. No warranty is provided.
If you use it, please give me credit and email me about your
project at cytex <at> gmx <dot> de and do not forget to also
drop chishm <at> hotmail <dot> com a line

See gba_nds_fat.txt for help and license details.
*/

#include "io_efa2.h"

#ifdef SUPPORT_EFA2

//
// EFA2 register addresses
//

// RTC registers
#define REG_RTC_CLK        *(vu16*)0x080000c4
#define REG_RTC_EN         *(vu16*)0x080000c8

// "Magic" registers used for unlock/lock sequences
#define REG_EFA2_MAGIC_A   *(vu16*)0x09fe0000
#define REG_EFA2_MAGIC_B   *(vu16*)0x08000000
#define REG_EFA2_MAGIC_C   *(vu16*)0x08020000
#define REG_EFA2_MAGIC_D   *(vu16*)0x08040000
#define REG_EFA2_MAGIC_E   *(vu16*)0x09fc0000

// NAND flash lock/unlock register
#define REG_EFA2_NAND_LOCK *(vu16*)0x09c40000
// NAND flash enable register
#define REG_EFA2_NAND_EN   *(vu16*)0x09400000
// NAND flash command write register
#define REG_EFA2_NAND_CMD   *(vu8*)0x09ffffe2
// NAND flash address/data write register
#define REG_EFA2_NAND_WR    *(vu8*)0x09ffffe0
// NAND flash data read register
#define REG_EFA2_NAND_RD    *(vu8*)0x09ffc000

// ID of Samsung K9K1G NAND flash chip
#define EFA2_NAND_ID 0xEC79A5C0

// first sector of udisk
#define EFA2_UDSK_START 0x40

//
// EFA2 access functions
//

// deactivate RTC ports
inline void efa2_rtc_deactivate(void) {
	REG_RTC_EN = 0;
}

// unlock register access
void efa2_reg_unlock(void) {
	REG_EFA2_MAGIC_A = 0x0d200;
	REG_EFA2_MAGIC_B = 0x01500;
	REG_EFA2_MAGIC_C = 0x0d200;
	REG_EFA2_MAGIC_D = 0x01500;
}

// finish/lock register access
inline void efa2_reg_lock(void) {
	REG_EFA2_MAGIC_E = 0x1500;
}

// global reset/init/enable/unlock ?
void efa2_global_unlock(void) {
	efa2_reg_unlock();
	*(vu16*)0x09880000 = 0x08000;
	efa2_reg_lock();
}

// global lock, stealth mode
void efa2_global_lock(void) {
	// quite sure there is such a sequence, but haven't had
	// a look for it upto now
}

// unlock NAND Flash
void efa2_nand_unlock(void) {
	efa2_reg_unlock();
	REG_EFA2_NAND_LOCK = 0x01500;
	efa2_reg_lock();
}

// lock NAND Flash
void efa2_nand_lock(void) {
	efa2_reg_unlock();
	REG_EFA2_NAND_LOCK = 0x0d200;
	efa2_reg_lock();
}

//
// Set NAND Flash chip enable and write protection bits ?
//
//   val | ~CE | ~WP |
//  -----+-----+-----+
//     0 |  0  |  0  |
//     1 |  1  |  0  |
//     3 |  1  |  1  |
//  -----+-----+-----+
//
void efa2_nand_enable(u16 val) {
	efa2_reg_unlock();
	REG_EFA2_NAND_EN = val;
	efa2_reg_lock();
}

//
// Perform NAND reset
// NAND has to be unlocked and enabled when called
//
inline void efa2_nand_reset(void) {
	REG_EFA2_NAND_CMD = 0xff; // write reset command
}

//
// Read out NAND ID information, could be used for card detection
//
//                    | EFA2 1GBit |
//  ------------------+------------+
//         maker code |    0xEC    |
//        device code |    0x79    |
//         don't care |    0xA5    |
//   multi plane code |    0xC0    |
//  ------------------+------------+
//
u32 efa2_nand_id(void) {
	u8 byte;
	u32 id;

	efa2_nand_unlock();
	efa2_nand_enable(1);

	REG_EFA2_NAND_CMD = 0x90;  // write id command
	REG_EFA2_NAND_WR  = 0x00;  // (dummy) address cycle
	byte = REG_EFA2_NAND_RD;   // read maker code
	id   = byte;
	byte = REG_EFA2_NAND_RD;   // read device code
	id   = (id << 8) | byte;
	byte = REG_EFA2_NAND_RD;   // read don't care
	id   = (id << 8) | byte;
	byte = REG_EFA2_NAND_RD;   // read multi plane code
	id   = (id << 8) | byte;

	efa2_nand_enable(0);
	efa2_nand_lock();
	return (id);
}

//
// Start of gba_nds_fat block device description
//

/*-----------------------------------------------------------------
EFA2_ClearStatus
Reads and checks NAND status information
bool return OUT:  true if NAND is idle
-----------------------------------------------------------------*/
bool EFA2_ClearStatus (void)
{
	// tbd: currently there is no write support, so always return
	// true, there is no possibility for pending operations
	return true;
}

/*-----------------------------------------------------------------
EFA2_IsInserted
Checks to see if the NAND chip used by the EFA2 is present
bool return OUT:  true if the correct NAND chip is found
-----------------------------------------------------------------*/
bool EFA2_IsInserted (void)
{
	EFA2_ClearStatus();
	return (efa2_nand_id() == EFA2_NAND_ID);
}

/*-----------------------------------------------------------------
EFA2_ReadSectors
Read "numSecs" 512 byte sectors starting from "sector" into "buffer"
No error correction, no use of spare cells, no use of R/~B signal
u32 sector IN: number of first 512 byte sector to be read
u8 numSecs IN: number of 512 byte sectors to read,
1 to 256 sectors can be read, 0 = 256
void* buffer OUT: pointer to 512 byte buffer to store data in
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool EFA2_ReadSectors (u32 sector, u8 numSecs, void* buffer)
{
	int  i;
	int  j = (numSecs > 0 ? numSecs : 256);

#ifndef _CF_ALLOW_UNALIGNED
	u8  byte;
	u16  word;
#endif

	// NAND page 0x40 (EFA2_UDSK_START) contains the MBR of the
	// udisk and thus is sector 0. The original EFA2 firmware
	// does never look at this, it only watches page 0x60, which
	// contains the boot block of the FAT16 partition. That is
	// fixed, so the EFA2 udisk must not be reformated, else
	// the ARK Octopus and also the original Firmware won't be
	// able to access the udisk anymore and I have to write a
	// recovery tool.
	u32 page = EFA2_UDSK_START + sector;

	// future enhancement: wait for possible write operations to
	// be finisched
	if (!EFA2_ClearStatus()) return false;

	efa2_nand_unlock();
	efa2_nand_enable(1);
	efa2_nand_reset();

	// set NAND to READ1 operation mode and transfer page address
	REG_EFA2_NAND_CMD = 0x00;                // write READ1 command
	REG_EFA2_NAND_WR  = 0x00;                // write address  [7:0]
	REG_EFA2_NAND_WR  = (page      ) & 0xff; // write address [15:8]
	REG_EFA2_NAND_WR  = (page >> 8 ) & 0xff; // write address[23:16]
	REG_EFA2_NAND_WR  = (page >> 16) & 0xff; // write address[26:24]

	// Due to a bug in EFA2 design there is need to waste some cycles
	// "by hand" instead the possibility to check the R/~B port of
	// the NAND flash via a register. The RTC deactivation is only
	// there to make sure the loop won't be optimized by the compiler
	for (i=0 ; i < 3 ; i++) efa2_rtc_deactivate();

	while (j--)
	{
		// read page data
#ifdef _CF_ALLOW_UNALIGNED
		// slow byte access to RAM, but works in principle
		for (i=0 ; i < 512 ; i++)
			((u8*)buffer)[i] = REG_EFA2_NAND_RD;
#else
		// a bit faster, but DMA is not possible
		for (i=0 ; i < 256 ; i++) {
			byte = REG_EFA2_NAND_RD;   // read lo-byte
			word = byte;
			byte = REG_EFA2_NAND_RD;   // read hi-byte
			word = word | (byte << 8);
			((u16*)buffer)[i] = word;
		}
#endif
	}

	efa2_nand_enable(0);
	efa2_nand_lock();
	return true;
}


/*-----------------------------------------------------------------
EFA2_WriteSectors
Write "numSecs" 512 byte sectors starting at "sector" from "buffer"
u32 sector IN: address of 512 byte sector on card to write
u8 numSecs IN: number of 512 byte sectors to write
1 to 256 sectors can be written, 0 = 256
void* buffer IN: pointer to 512 byte buffer to read data from
bool return OUT: true if successful
-----------------------------------------------------------------*/
bool EFA2_WriteSectors (u32 sector, u8 numSecs, void* buffer)
{
	// Upto now I focused on reading NAND, write operations
	// will follow
	return false;
}

/*-----------------------------------------------------------------
EFA2_Shutdown
unload the EFA2 interface
-----------------------------------------------------------------*/
bool EFA2_Shutdown(void)
{
	return EFA2_ClearStatus();
}

/*-----------------------------------------------------------------
EFA2_StartUp
initializes the EFA2 card, returns true if successful,
otherwise returns false
-----------------------------------------------------------------*/
bool EFA2_StartUp(void)
{
	efa2_global_unlock();
	return (efa2_nand_id() == EFA2_NAND_ID);
}

/*-----------------------------------------------------------------
the actual interface structure
-----------------------------------------------------------------*/
IO_INTERFACE io_efa2 = {
	DEVICE_TYPE_EFA2,
	FEATURE_MEDIUM_CANREAD | FEATURE_SLOT_GBA,
	(FN_MEDIUM_STARTUP)&EFA2_StartUp,
	(FN_MEDIUM_ISINSERTED)&EFA2_IsInserted,
	(FN_MEDIUM_READSECTORS)&EFA2_ReadSectors,
	(FN_MEDIUM_WRITESECTORS)&EFA2_WriteSectors,
	(FN_MEDIUM_CLEARSTATUS)&EFA2_ClearStatus,
	(FN_MEDIUM_SHUTDOWN)&EFA2_Shutdown
};

/*-----------------------------------------------------------------
EFA2_GetInterface
returns the interface structure to host
-----------------------------------------------------------------*/
LPIO_INTERFACE EFA2_GetInterface(void) {
	return &io_efa2;
}

#endif // SUPPORT_EFA2
