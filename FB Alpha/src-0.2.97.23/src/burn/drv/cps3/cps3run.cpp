/**********************************************************************

CPS3 Driver (preliminary)

Decryption by Andreas Naive

Driver by David Haywood
 with help from Tomasz Slanina and ElSemi

Sound emulation by Philip Bennett

SCSI code by ElSemi

**********************************************************************

Port to FBA by OopsWare

**********************************************************************/

#include "cps3.h"
#include "sh2.h"

#define	BE_GFX		1
//#define	FAST_BOOT	1
#define SPEED_HACK	1		// Default should be 1, if not FPS would drop.

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;

static UINT8 *RomBios;
static UINT8 *RomGame;
static UINT8 *RomGame_D;
static UINT8 *RomUser;

static UINT8 *RamMain;
static UINT32 *RamSpr;
static UINT16 *RamPal;

static UINT32 *RamCRam;

static UINT32 *RamSS;

static UINT32 *RamVReg;

static UINT8 *RamC000;
static UINT8 *RamC000_D;

static UINT16 *EEPROM;

UINT16 *Cps3CurPal;
static UINT32 *RamScreen;

UINT8 cps3_reset = 0;
UINT8 cps3_palette_change = 0;

UINT32 cps3_key1, cps3_key2, cps3_isSpecial;
UINT32 cps3_bios_test_hack, cps3_game_test_hack;
UINT32 cps3_speedup_ram_address, cps3_speedup_code_address;
UINT8 cps3_dip;
UINT32 cps3_region_address, cps3_ncd_address;

static UINT32 cps3_data_rom_size;

UINT8 Cps3But1[16];
UINT8 Cps3But2[16];
UINT8 Cps3But3[16];

static UINT16 Cps3Input[4] = {0, 0, 0, 0};

static UINT32 ss_bank_base = 0;
static UINT32 ss_pal_base = 0;

static UINT32 cram_bank = 0;
static UINT16 cps3_current_eeprom_read = 0;
static UINT32 gfxflash_bank = 0;

static UINT32 paldma_source = 0;
static UINT32 paldma_dest = 0;
static UINT32 paldma_fade = 0;
static UINT32 paldma_length = 0;

static UINT32 chardma_source = 0;
static UINT32 chardma_table_address = 0;

static INT32 cps3_gfx_width, cps3_gfx_height;
static INT32 cps3_gfx_max_x, cps3_gfx_max_y;


// -- AMD/Fujitsu 29F016 --------------------------------------------------

enum {
	FM_NORMAL,	// normal read/write
	FM_READID,	// read ID
	FM_READSTATUS,	// read status
	FM_WRITEPART1,	// first half of programming, awaiting second
	FM_CLEARPART1,	// first half of clear, awaiting second
	FM_SETMASTER,	// first half of set master lock, awaiting on/off
	FM_READAMDID1,	// part 1 of alt ID sequence
	FM_READAMDID2,	// part 2 of alt ID sequence
	FM_READAMDID3,	// part 3 of alt ID sequence
	FM_ERASEAMD1,	// part 1 of AMD erase sequence
	FM_ERASEAMD2,	// part 2 of AMD erase sequence
	FM_ERASEAMD3,	// part 3 of AMD erase sequence
	FM_ERASEAMD4,	// part 4 of AMD erase sequence
	FM_BYTEPROGRAM
};

typedef struct
{
	INT32 status;
	INT32 flash_mode;
	INT32 flash_master_lock;
} flash_chip;

static flash_chip main_flash;

void cps3_flash_init(flash_chip * chip/*, void *data*/)
{
	memset(chip, 0, sizeof(flash_chip));
	chip->status = 0x80;
	chip->flash_mode = FM_NORMAL;
	chip->flash_master_lock = 0;
}

UINT32 cps3_flash_read(flash_chip * chip, UINT32 addr)
{
	switch( chip->flash_mode ) {
	case FM_READAMDID3:
	case FM_READID:
		
		switch (addr & 0x7fffff) {
			case 0:	return 0x04040404;	//(c->maker_id << 0) | (c->maker_id << 8) | (c->maker_id << 16) | (c->maker_id << 24);
			case 4: return 0xadadadad;	//(c->device_id << 0) | (c->device_id << 8) | (c->device_id << 16) | (c->device_id << 24);
			case 8: return 0x00000000;
			//case 12: return (c->flash_master_lock) ? 0x01010101 : 0x00000000
		}
		return 0;

	case FM_ERASEAMD4:
		chip->status ^= ( 1 << 6 ) | ( 1 << 2 );
	case FM_READSTATUS:
		return (chip->status << 0) | (chip->status << 8) | (chip->status << 16) | (chip->status << 24);

	case FM_NORMAL:
	default:
		//
		return 0;
	}
}

void cps3_flash_write(flash_chip * chip, UINT32 addr, UINT32 data)
{
	bprintf(1, _T("FLASH to write long value %8x to location %8x\n"), data, addr);
	
	switch( chip->flash_mode )	{
	case FM_NORMAL:
	case FM_READSTATUS:
	case FM_READID:
	case FM_READAMDID3:
		switch( data & 0xff ) {
		case 0xf0:
		case 0xff: chip->flash_mode = FM_NORMAL;	break;
		case 0x90: chip->flash_mode = FM_READID;	break;
		case 0x40:
		case 0x10: chip->flash_mode = FM_WRITEPART1;break;
		case 0x50:	// clear status reg
			chip->status = 0x80;
			chip->flash_mode = FM_READSTATUS;
			break;
		case 0x20: chip->flash_mode = FM_CLEARPART1;break;
		case 0x60: chip->flash_mode = FM_SETMASTER;	break;
		case 0x70: chip->flash_mode = FM_READSTATUS;break;
		case 0xaa:	// AMD ID select part 1
			if ((addr & 0xffff) == (0x555 << 2))
				chip->flash_mode = FM_READAMDID1;
			break;
		default:
			//logerror( "Unknown flash mode byte %x\n", data & 0xff );
			//bprintf(1, _T("FLASH to write long value %8x to location %8x\n"), data, addr);
			break;
		}	
		break;
	case FM_READAMDID1:
		if ((addr & 0xffff) == (0x2aa <<2) && (data & 0xff) == 0x55 ) {
			chip->flash_mode = FM_READAMDID2;
		} else {
			//logerror( "unexpected %08x=%02x in FM_READAMDID1\n", address, data & 0xff );
			chip->flash_mode = FM_NORMAL;
		}
		break;
	case FM_READAMDID2:
		if ((addr & 0xffff) == (0x555<<2) && (data & 0xff) == 0x90) {
			chip->flash_mode = FM_READAMDID3;
		} else if((addr & 0xffff) == (0x555<<2) && (data & 0xff) == 0x80) {
			chip->flash_mode = FM_ERASEAMD1;
		} else if((addr & 0xffff) == (0x555<<2) && (data & 0xff) == 0xa0) {
			chip->flash_mode = FM_BYTEPROGRAM;
		} else if((addr & 0xffff) == (0x555<<2) && (data & 0xff) == 0xf0) {
			chip->flash_mode = FM_NORMAL;
		} else {
			// logerror( "unexpected %08x=%02x in FM_READAMDID2\n", address, data & 0xff );
			chip->flash_mode = FM_NORMAL;
		}
		break;				
	}
}

// ------------------------------------------------------------------------

inline static void Cps3ClearOpposites(UINT16* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

// ------------------------------------------------------------------------

static UINT16 rotate_left(UINT16 value, INT32 n)
{
   INT32 aux = value>>(16-n);
   return ((value<<n)|aux) & 0xffff;
}

static UINT16 rotxor(UINT16 val, UINT16 x)
{
	UINT16 res;
	res = val + rotate_left(val,2);
	res = rotate_left(res,4) ^ (res & (val ^ x));
	return res;
}

static UINT32 cps3_mask(UINT32 address, UINT32 key1, UINT32 key2)
{
	UINT16 val;
	address ^= key1;
	val = (address & 0xffff) ^ 0xffff;
	val = rotxor(val, key2 & 0xffff);
	val ^= (address >> 16) ^ 0xffff;
	val = rotxor(val, key2 >> 16);
	val ^= (address & 0xffff) ^ (key2 & 0xffff);
	return val | (val << 16);
}

static void cps3_decrypt_bios(void)
{
	UINT32 * coderegion = (UINT32 *)RomBios;
	for (INT32 i=0; i<0x20000; i+=4) {
		UINT32 xormask = cps3_mask(i, cps3_key1, cps3_key2);
		/* a bit of a hack, don't decrypt the FLASH commands which are transfered by SH2 DMA */
		if ( (i<0x1ff00) || (i>0x1ff6b) )
			coderegion[i/4] ^= xormask;
	}
}

static void cps3_decrypt_game(void)
{
	UINT32 * coderegion = (UINT32 *)RomGame;
	UINT32 * decrypt_coderegion = (UINT32 *)RomGame_D;
	
	for (INT32 i=0; i<0x1000000; i+=4) {
		UINT32 xormask = cps3_mask(i + 0x06000000, cps3_key1, cps3_key2);
		decrypt_coderegion[i/4] = coderegion[i/4] ^ xormask;
	}
}


static INT32 last_normal_byte = 0;

static UINT32 process_byte( UINT8 real_byte, UINT32 destination, INT32 max_length )
{
	UINT8 * dest = (UINT8 *) RamCRam;
	//printf("process byte for destination %08x\n", destination);
	destination &= 0x7fffff;

	if (real_byte&0x40) {
		INT32 tranfercount = 0;
		//printf("Set RLE Mode\n");
		INT32 cps3_rle_length = (real_byte&0x3f)+1;
		//printf("RLE Operation (length %08x\n", cps3_rle_length );
		while (cps3_rle_length) {
#if BE_GFX
			dest[((destination+tranfercount)&0x7fffff)] = (last_normal_byte&0x3f);
#else
			dest[((destination+tranfercount)&0x7fffff)^3] = (last_normal_byte&0x3f);
#endif
			//cps3_char_ram_dirty[((destination+tranfercount)&0x7fffff)/0x100] = 1;
			//cps3_char_ram_is_dirty = 1;
			//printf("RLE WRite Byte %08x, %02x\n", destination+tranfercount, real_byte);

			tranfercount++;
			cps3_rle_length--;
			max_length--;
			if ((destination+tranfercount) > 0x7fffff)  return max_length;
	//      if (max_length==0) return max_length; // this is meant to abort the transfer if we exceed dest length,, not working
		}
		return tranfercount;
	} else {
		//printf("Write Normal Data\n");
#if BE_GFX
		dest[(destination&0x7fffff)] = real_byte;
#else
		dest[(destination&0x7fffff)^3] = real_byte;
#endif
		last_normal_byte = real_byte;
		//cps3_char_ram_dirty[(destination&0x7fffff)/0x100] = 1;
		//cps3_char_ram_is_dirty = 1;
		return 1;
	}
}

static void cps3_do_char_dma( UINT32 real_source, UINT32 real_destination, UINT32 real_length )
{
	UINT8 * sourcedata = RomUser;
	INT32 length_remaining = real_length;
	last_normal_byte = 0;
	while (length_remaining) {
		UINT8 current_byte = sourcedata[ real_source ^ 0 ];
		real_source++;

		if (current_byte & 0x80) {
			UINT8 real_byte;
			UINT32 length_processed;
			current_byte &= 0x7f;

			real_byte = sourcedata[ (chardma_table_address+current_byte*2+0) ^ 0 ];
			//if (real_byte&0x80) return;
			length_processed = process_byte( real_byte, real_destination, length_remaining );
			length_remaining -= length_processed; // subtract the number of bytes the operation has taken
			real_destination += length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return; // if we've expired, exit

			real_byte = sourcedata[ (chardma_table_address+current_byte*2+1) ^ 0 ];
			//if (real_byte&0x80) return;
			length_processed = process_byte( real_byte, real_destination, length_remaining );
			length_remaining -= length_processed; // subtract the number of bytes the operation has taken
			real_destination += length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return;  // if we've expired, exit
		} else {
			UINT32 length_processed;
			length_processed = process_byte( current_byte, real_destination, length_remaining );
			length_remaining -= length_processed; // subtract the number of bytes the operation has taken
			real_destination += length_processed; // add it onto the destination
			if (real_destination>0x7fffff) return;
			if (length_remaining<=0) return;  // if we've expired, exit
		}
	}
}

static UINT16 lastb;
static UINT16 lastb2;

static UINT32 ProcessByte8(UINT8 b, UINT32 dst_offset)
{
	UINT8 * destRAM = (UINT8 *) RamCRam;
 	INT32 l=0;

 	if(lastb==lastb2) {	//rle
 		INT32 rle=(b+1)&0xff;

 		for(INT32 i=0;i<rle;++i) {
#if BE_GFX
			destRAM[(dst_offset&0x7fffff)] = lastb;
#else
			destRAM[(dst_offset&0x7fffff)^3] = lastb;
#endif
			//cps3_char_ram_dirty[(dst_offset&0x7fffff)/0x100] = 1;
			//cps3_char_ram_is_dirty = 1;

			dst_offset++;
 			++l;
 		}
 		lastb2=0xffff;
 		return l;
 	} else {
 		lastb2=lastb;
 		lastb=b;
#if BE_GFX
		destRAM[(dst_offset&0x7fffff)] = b;
#else
		destRAM[(dst_offset&0x7fffff)^3] = b;
#endif
		//cps3_char_ram_dirty[(dst_offset&0x7fffff)/0x100] = 1;
		//cps3_char_ram_is_dirty = 1;
 		return 1;
 	}
}

static void cps3_do_alt_char_dma(UINT32 src, UINT32 real_dest, UINT32 real_length )
{
	UINT8 * px = RomUser;
	UINT32 start = real_dest;
	UINT32 ds = real_dest;

	lastb=0xfffe;
	lastb2=0xffff;

	while(1) {
		UINT8 ctrl=px[ src ^ 0 ];
 		++src;

		for(INT32 i=0;i<8;++i) {
			UINT8 p = px[ src ^ 0 ];

			if(ctrl&0x80) {
				UINT8 real_byte;
				p &= 0x7f;
				real_byte = px[ (chardma_table_address+p*2+0) ^ 0 ];
				ds += ProcessByte8(real_byte,ds);
				real_byte = px[ (chardma_table_address+p*2+1) ^ 0 ];
				ds += ProcessByte8(real_byte,ds);
 			} else {
 				ds += ProcessByte8(p,ds);
 			}
 			++src;
 			ctrl<<=1;

			if((ds-start)>=real_length)
				return;
 		}
	}
}

static void cps3_process_character_dma(UINT32 address)
{
	for (INT32 i=0; i<0x1000; i+=3) {
		UINT32 dat1 = RamCRam[i+0+(address)];
		UINT32 dat2 = RamCRam[i+1+(address)];
		UINT32 dat3 = RamCRam[i+2+(address)];
		UINT32 real_source      = (dat3<<1)-0x400000;
		UINT32 real_destination =  dat2<<3;
		UINT32 real_length      = (((dat1&0x001fffff)+1)<<3);
		
		if (dat1==0x01000000) break;	// end of list marker
		if (dat1==0x13131313) break;	// our default fill
		
		switch ( dat1 & 0x00e00000 ) {
		case 0x00800000:
			chardma_table_address = real_source;
			Sh2SetIRQLine(10, SH2_IRQSTATUS_AUTO);
			break;
		case 0x00400000:
			cps3_do_char_dma( real_source, real_destination, real_length );
			Sh2SetIRQLine(10, SH2_IRQSTATUS_AUTO);
			break;
		case 0x00600000:
			//bprintf(PRINT_NORMAL, _T("Character DMA (alt) start %08x to %08x with %d\n"), real_source, real_destination, real_length);
			/* 8bpp DMA decompression
			   - this is used on SFIII NG Sean's Stage ONLY */
			cps3_do_alt_char_dma( real_source, real_destination, real_length );
			Sh2SetIRQLine(10, SH2_IRQSTATUS_AUTO);
			break;
		case 0x00000000:
			// Red Earth need this. 8192 byte trans to 0x00003000 (from 0x007ec000???)
			// seems some stars(6bit alpha) without compress
			//bprintf(PRINT_NORMAL, _T("Character DMA (redearth) start %08x to %08x with %d\n"), real_source, real_destination, real_length);
			memcpy( (UINT8 *)RamCRam + real_destination, RomUser + real_source, real_length );
			Sh2SetIRQLine(10, SH2_IRQSTATUS_AUTO);
			break;
		default:
			bprintf(PRINT_NORMAL, _T("Character DMA Unknown DMA List Command Type %08x\n"), dat1);
		}
	}
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	RomBios 	= Next; Next += 0x0080000;

	RomUser		= Next; Next += cps3_data_rom_size;	// 0x5000000;
	
	RamStart	= Next;
	
	RomGame 	= Next; Next += 0x1000000;
	RomGame_D 	= Next; Next += 0x1000000;
	
	RamC000		= Next; Next += 0x0000400;
	RamC000_D	= Next; Next += 0x0000400;

	RamMain		= Next; Next += 0x0080000;

	RamPal		= (UINT16 *) Next; Next += 0x0020000 * sizeof(UINT16);
	RamSpr		= (UINT32 *) Next; Next += 0x0020000 * sizeof(UINT32);

	RamCRam		= (UINT32 *) Next; Next += 0x0200000 * sizeof(UINT32);
	RamSS		= (UINT32 *) Next; Next += 0x0004000 * sizeof(UINT32);
	
	RamVReg		= (UINT32 *) Next; Next += 0x0000040 * sizeof(UINT32);
	
	EEPROM		= (UINT16 *) Next; Next += 0x0000100 * sizeof(UINT16);
	
	RamEnd		= Next;
	
	Cps3CurPal		= (UINT16 *) Next; Next += 0x020001 * sizeof(UINT16); // iq_132 - layer disable
	RamScreen	= (UINT32 *) Next; Next += (512 * 2) * (224 * 2 + 32) * sizeof(UINT32);
	
	MemEnd		= Next;
	return 0;
}

UINT8 __fastcall cps3ReadByte(UINT32 addr)
{
	addr &= 0xc7ffffff;
	
//	switch (addr) {
//
//	default:
//		bprintf(PRINT_NORMAL, _T("Attempt to read byte value of location %8x\n"), addr);
//	}
	return 0;
}

UINT16 __fastcall cps3ReadWord(UINT32 addr)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {
	
	// redearth will read this !!!
#if 0
	case 0x040c0000:
		return RamVReg[0] >> 16;
	case 0x040c0002:
		return RamVReg[0] & 0xffff;
	case 0x040c0004:
		return RamVReg[1] >> 16;
	case 0x040c0006:
		return RamVReg[1] & 0xffff;
#else
	case 0x040c0000:
	case 0x040c0002:
	case 0x040c0004:
	case 0x040c0006:
		return 0;
#endif
	// cps3_vbl_r
	case 0x040c000c:
	case 0x040c000e:
		return 0;

	case 0x05000000: return ~Cps3Input[1];
	case 0x05000002: return ~Cps3Input[0];
	case 0x05000004: return ~Cps3Input[3];
	case 0x05000006: return ~Cps3Input[2];

	case 0x05140000:
	case 0x05140002:
		// cd-rom read
		
		break;
		
	default:
		if ((addr >= 0x05000a00) && (addr < 0x05000a20)) {
			// cps3_unk_io_r
			return 0xffff;
		} else
		if ((addr >= 0x05001000) && (addr < 0x05001204)) {
			// EEPROM
			addr -= 0x05001000;
			if (addr >= 0x100 && addr < 0x180) {
#ifdef LSB_FIRST
			cps3_current_eeprom_read = EEPROM[((addr-0x100) >> 1) ^ 1];
#else
			cps3_current_eeprom_read = EEPROM[((addr-0x100) >> 1)];
#endif
			} else
			if (addr == 0x202)
				return cps3_current_eeprom_read;
		} else
		bprintf(PRINT_NORMAL, _T("Attempt to read word value of location %8x\n"), addr);
	}
	return 0;
}

UINT32 __fastcall cps3ReadLong(UINT32 addr)
{
	addr &= 0xc7ffffff;
		
	switch (addr) {
	case 0x04200000:
		bprintf(PRINT_NORMAL, _T("GFX Read Flash ID, cram bank %04x gfx flash bank: %04x\n"), cram_bank, gfxflash_bank);
		return 0x0404adad;

	default:
		bprintf(PRINT_NORMAL, _T("Attempt to read long value of location %8x\n"), addr);
	}
	return 0;
}

void __fastcall cps3WriteByte(UINT32 addr, UINT8 data)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {
	
	// cps3_ss_bank_base_w
	case 0x05050020: ss_bank_base = ( ss_bank_base  & 0x00ffffff ) | (data << 24); break;
	case 0x05050021: ss_bank_base = ( ss_bank_base  & 0xff00ffff ) | (data << 16); break;
	case 0x05050022: ss_bank_base = ( ss_bank_base  & 0xffff00ff ) | (data <<  8); break;
	case 0x05050023: ss_bank_base = ( ss_bank_base  & 0xffffff00 ) | (data <<  0); break;

	// cps3_ss_pal_base_w
	case 0x05050024: ss_pal_base = ( ss_pal_base & 0x00ff ) | (data << 8); break;
	case 0x05050025: ss_pal_base = ( ss_pal_base & 0xff00 ) | (data << 0); break;
	case 0x05050026: break;
	case 0x05050027: break;

	default:
		if ((addr >= 0x05050000) && (addr < 0x05060000)) {
			// VideoReg

		} else
			bprintf(PRINT_NORMAL, _T("Attempt to write byte value   %02x to location %8x\n"), data, addr);
	}
}

void __fastcall cps3WriteWord(UINT32 addr, UINT16 data)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {
	
	case 0x040c0084: break;
	case 0x040c0086:
		if (cram_bank != data) {
			cram_bank = data & 7;
			//bprintf(PRINT_NORMAL, _T("CRAM bank set to %d\n"), data);
			Sh2MapMemory(((UINT8 *)RamCRam) + (cram_bank << 20), 0x04100000, 0x041fffff, SM_RAM);
		}
		break;

	case 0x040c0088:
	//case 0x040c008a:
		gfxflash_bank = data - 2;
		//bprintf(PRINT_NORMAL, _T("gfxflash bank set to %04x\n"), data);
		break;
	
	// cps3_characterdma_w
	//case 0x040c0094:
	case 0x040c0096: 
		chardma_source = data; 
		break;
	case 0x040c0098:
	//case 0x040c009a:
		if (data & 0x0040)
			cps3_process_character_dma( chardma_source | ((data & 0x003f) << 16) );
		break;

	// cps3_palettedma_w
	case 0x040c00a0: paldma_source = (paldma_source & 0x0000ffff) | (data << 16); break;
	case 0x040c00a2: paldma_source = (paldma_source & 0xffff0000) | (data <<  0); break;
	case 0x040c00a4: paldma_dest = (paldma_dest & 0x0000ffff) | (data << 16); break;
	case 0x040c00a6: paldma_dest = (paldma_dest & 0xffff0000) | (data <<  0); break;
	case 0x040c00a8: paldma_fade = (paldma_fade & 0x0000ffff) | (data << 16); break;
	case 0x040c00aa: paldma_fade = (paldma_fade & 0xffff0000) | (data <<  0); break;
	case 0x040c00ac: paldma_length = data; break;
	case 0x040c00ae:
		//bprintf(PRINT_NORMAL, _T("palettedma [%04x]  from %08x to %08x fade %08x size %d\n"), data, (paldma_source << 1), paldma_dest, paldma_fade, paldma_length);
		if (data & 0x0002) {
			for (UINT32 i=0; i<paldma_length; i++) {
				UINT16 * src = (UINT16 *)RomUser;
				UINT16 coldata = src[(paldma_source - 0x200000 + i)];
				
#ifdef LSB_FIRST
				coldata = (coldata << 8) | (coldata >> 8);
#endif

				UINT32 r = (coldata & 0x001F) >>  0;
				UINT32 g = (coldata & 0x03E0) >>  5;
				UINT32 b = (coldata & 0x7C00) >> 10;
				if (paldma_fade!=0) {
					INT32 fade;
					fade = (paldma_fade & 0x3f000000)>>24; r = (r*fade)>>5; if (r>0x1f) r = 0x1f;
					fade = (paldma_fade & 0x003f0000)>>16; g = (g*fade)>>5; if (g>0x1f) g = 0x1f;
					fade = (paldma_fade & 0x0000003f)>> 0; b = (b*fade)>>5; if (b>0x1f) b = 0x1f;
					coldata = (r << 0) | (g << 5) | (b << 10);
				}
				
				r = r << 3;
				g = g << 3;
				b = b << 3;

				RamPal[(paldma_dest + i) ^ 1] = coldata;
				Cps3CurPal[(paldma_dest + i) ] = BurnHighCol(r, g, b, 0);
			}
			Sh2SetIRQLine(10, SH2_IRQSTATUS_AUTO);
		}
		break;
	
	case 0x04200554:
	case 0x04200aaa:
		// gfx flash command 
		// "0xaa 0x55 0x90" set flash to get id 
		// "0xaa 0x55 0xf0" set flash to normal read
		break;
	
	// cps3_ss_bank_base_w
	case 0x05050020: ss_bank_base = ( ss_bank_base  & 0x0000ffff ) | (data << 16); break;
	case 0x05050022: ss_bank_base = ( ss_bank_base  & 0xffff0000 ) | (data <<  0); break;

	// cps3_ss_pal_base_w
	case 0x05050024: ss_pal_base = data; break;
	case 0x05050026: break;
			
	case 0x05100000:
		Sh2SetIRQLine(12, SH2_IRQSTATUS_NONE);
		break;
	case 0x05110000:
		Sh2SetIRQLine(10, SH2_IRQSTATUS_NONE);
		break;

	case 0x05140000:
	case 0x05140002:
		// cd-rom read
		
		break;
			
	default:
		if ((addr >= 0x040c0000) && (addr < 0x040c0100)) {
			// 0x040C0000 ~ 0x040C001f : cps3_unk_vidregs
			// 0x040C0020 ~ 0x040C002b : tilemap20_regs_base
			// 0x040C0030 ~ 0x040C003b : tilemap30_regs_base
			// 0x040C0040 ~ 0x040C004b : tilemap40_regs_base
			// 0x040C0050 ~ 0x040C005b : tilemap50_regs_base
			// 0x040C0060 ~ 0x040C007f : cps3_fullscreenzoom

			addr &= 0xff;
#ifdef LSB_FIRST
			((UINT16 *)RamVReg)[ (addr >> 1) ^ 1 ] = data;
#else
			((UINT16 *)RamVReg)[ (addr >> 1) ] = data;
#endif
			
		} else
		if ((addr >= 0x05000000) && (addr < 0x05001000)) {
			
			
		} else 
		if ((addr >= 0x05001000) && (addr < 0x05001204)) {
			// EEPROM
			addr -= 0x05001000;
			if ((addr>=0x080) && (addr<0x100))
#ifdef LSB_FIRST
			EEPROM[((addr-0x080) >> 1) ^ 1] = data;
#else
			EEPROM[((addr-0x080) >> 1)] = data;
#endif
		} else
		if ((addr >= 0x05050000) && (addr < 0x05060000)) {
			// unknow i/o

		} else
				
		bprintf(PRINT_NORMAL, _T("Attempt to write word value %04x to location %8x\n"), data, addr);
	}
}

void __fastcall cps3WriteLong(UINT32 addr, UINT32 data)
{
	addr &= 0xc7ffffff;
	
	switch (addr) {
	case 0x07ff000c:
	case 0x07ff0048:
		// some unknown data write by DMA 0 while bootup
		break;

	default:
		bprintf(PRINT_NORMAL, _T("Attempt to write long value %8x to location %8x\n"), data, addr);
	}
}

void __fastcall cps3C0WriteByte(UINT32 addr, UINT8 data)
{
	bprintf(PRINT_NORMAL, _T("C0 Attempt to write byte value %2x to location %8x\n"), data, addr);
}

void __fastcall cps3C0WriteWord(UINT32 addr, UINT16 data)
{
	bprintf(PRINT_NORMAL, _T("C0 Attempt to write word value %4x to location %8x\n"), data, addr);
}

void __fastcall cps3C0WriteLong(UINT32 addr, UINT32 data)
{
	if (addr < 0xc0000400) {
		*(UINT32 *)(RamC000 + (addr & 0x3ff)) = data;
		*(UINT32 *)(RamC000_D + (addr & 0x3ff)) = data ^ cps3_mask(addr, cps3_key1, cps3_key2);
		return ;
	}
	bprintf(PRINT_NORMAL, _T("C0 Attempt to write long value %8x to location %8x\n"), data, addr);
}

// If fastboot != 1 

UINT8 __fastcall cps3RomReadByte(UINT32 addr)
{
//	bprintf(PRINT_NORMAL, _T("Rom Attempt to read byte value of location %8x\n"), addr);
	addr &= 0xc7ffffff;
#ifdef LSB_FIRST
	addr ^= 0x03;
#endif
	return *(RomGame_D + (addr & 0x00ffffff));
}

UINT16 __fastcall cps3RomReadWord(UINT32 addr)
{
//	bprintf(PRINT_NORMAL, _T("Rom Attempt to read word value of location %8x\n"), addr);
	addr &= 0xc7ffffff;
#ifdef LSB_FIRST
	addr ^= 0x02;
#endif
	return *(UINT16 *)(RomGame_D + (addr & 0x00ffffff));
}

UINT32 __fastcall cps3RomReadLong(UINT32 addr)
{
//	bprintf(PRINT_NORMAL, _T("Rom Attempt to read long value of location %8x\n"), addr);
	addr &= 0xc7ffffff;
	
	UINT32 retvalue = cps3_flash_read(&main_flash, addr);
	if ( main_flash.flash_mode == FM_NORMAL )
		retvalue = *(UINT32 *)(RomGame_D + (addr & 0x00ffffff));
	
	UINT32 pc = Sh2GetPC(0);
	if (pc == cps3_bios_test_hack || pc == cps3_game_test_hack){
		if ( main_flash.flash_mode == FM_NORMAL )
			retvalue = *(UINT32 *)(RomGame + (addr & 0x00ffffff));
		bprintf(2, _T("CPS3 Hack : read long from %08x [%08x]\n"), addr, retvalue );
	}
	return retvalue;
}

void __fastcall cps3RomWriteByte(UINT32 addr, UINT8 data)
{
	bprintf(PRINT_NORMAL, _T("Rom Attempt to write byte value %2x to location %8x\n"), data, addr);
}

void __fastcall cps3RomWriteWord(UINT32 addr, UINT16 data)
{
	bprintf(PRINT_NORMAL, _T("Rom Attempt to write word value %4x to location %8x\n"), data, addr);
}

void __fastcall cps3RomWriteLong(UINT32 addr, UINT32 data)
{
//	bprintf(1, _T("Rom Attempt to write long value %8x to location %8x\n"), data, addr);
	addr &= 0x00ffffff;
	cps3_flash_write(&main_flash, addr, data);
	
	if ( main_flash.flash_mode == FM_NORMAL ) {
		bprintf(1, _T("Rom Attempt to write long value %8x to location %8x\n"), data, addr);
		*(UINT32 *)(RomGame + addr) = data;
		*(UINT32 *)(RomGame_D + addr) = data ^ cps3_mask(addr + 0x06000000, cps3_key1, cps3_key2);
	}
}

UINT8 __fastcall cps3RomReadByteSpe(UINT32 addr)
{
//	bprintf(PRINT_NORMAL, _T("Rom Attempt to read byte value of location %8x\n"), addr);
	addr &= 0xc7ffffff;
#ifdef LSB_FIRST
	addr ^= 0x03;
#endif
	return *(RomGame + (addr & 0x00ffffff));
}

UINT16 __fastcall cps3RomReadWordSpe(UINT32 addr)
{
//	bprintf(PRINT_NORMAL, _T("Rom Attempt to read word value of location %8x\n"), addr);
	addr &= 0xc7ffffff;
#ifdef LSB_FIRST
	addr ^= 0x02;
#endif
	return *(UINT16 *)(RomGame + (addr & 0x00ffffff));
}

UINT32 __fastcall cps3RomReadLongSpe(UINT32 addr)
{
//	bprintf(PRINT_NORMAL, _T("Rom Attempt to read long value of location %8x\n"), addr);
	addr &= 0xc7ffffff;
	
	UINT32 retvalue = cps3_flash_read(&main_flash, addr);
	if ( main_flash.flash_mode == FM_NORMAL )
		retvalue = *(UINT32 *)(RomGame + (addr & 0x00ffffff));

	return retvalue;
}

//------------------

UINT8 __fastcall cps3VidReadByte(UINT32 addr)
{
	bprintf(PRINT_NORMAL, _T("Video Attempt to read byte value of location %8x\n"), addr);
//	addr &= 0xc7ffffff;
	return 0;
}

UINT16 __fastcall cps3VidReadWord(UINT32 addr)
{
	bprintf(PRINT_NORMAL, _T("Video Attempt to read word value of location %8x\n"), addr);
//	addr &= 0xc7ffffff;
	return 0;
}

UINT32 __fastcall cps3VidReadLong(UINT32 addr)
{
	bprintf(PRINT_NORMAL, _T("Video Attempt to read long value of location %8x\n"), addr);
//	addr &= 0xc7ffffff;
	return 0;
}

void __fastcall cps3VidWriteByte(UINT32 addr, UINT8 data)
{
	bprintf(PRINT_NORMAL, _T("Video Attempt to write byte value %2x to location %8x\n"), data, addr);
}

void __fastcall cps3VidWriteWord(UINT32 addr, UINT16 data)
{
	addr &= 0xc7ffffff;
	if ((addr >= 0x04080000) && (addr < 0x040c0000)) {
		// Palette
		UINT32 palindex = (addr - 0x04080000) >> 1;
#ifdef LSB_FIRST
		RamPal[palindex ^ 1] = data;
#else
		RamPal[palindex] = data;
#endif

		INT32 r = (data & 0x001F) << 3;	// Red
		INT32 g = (data & 0x03E0) >> 2;	// Green
		INT32 b = (data & 0x7C00) >> 7;	// Blue
		
		r |= r >> 5;
		g |= g >> 5;
		b |= b >> 5;
			
		Cps3CurPal[palindex] = BurnHighCol(r, g, b, 0);
	
	} else
	bprintf(PRINT_NORMAL, _T("Video Attempt to write word value %4x to location %8x\n"), data, addr);
}

void __fastcall cps3VidWriteLong(UINT32 addr, UINT32 data)
{
	addr &= 0xc7ffffff;
	if ((addr >= 0x04080000) && (addr < 0x040c0000)) {

		if ( data != 0 )
			bprintf(PRINT_NORMAL, _T("Video Attempt to write long value %8x to location %8x\n"), data, addr);
		
		
	} else 
	bprintf(PRINT_NORMAL, _T("Video Attempt to write long value %8x to location %8x\n"), data, addr);
}


UINT8 __fastcall cps3RamReadByte(UINT32 addr)
{
	if (addr == cps3_speedup_ram_address )
		if (Sh2GetPC(0) == cps3_speedup_code_address)
			Sh2BurnUntilInt(0);

	addr &= 0x7ffff;
#ifdef LSB_FIRST
	return *(RamMain + (addr ^ 0x03));
#else
	return *(RamMain + addr);
#endif
}

UINT16 __fastcall cps3RamReadWord(UINT32 addr)
{
	//bprintf(PRINT_NORMAL, _T("Ram Attempt to read long value of location %8x\n"), addr);
	addr &= 0x7ffff;

	if (addr == cps3_speedup_ram_address )
		if (Sh2GetPC(0) == cps3_speedup_code_address) {
			bprintf(PRINT_NORMAL, _T("Ram Attempt to read long value of location %8x\n"), addr);
			Sh2BurnUntilInt(0);
		}
	
#ifdef LSB_FIRST
	return *(UINT16 *)(RamMain + (addr ^ 0x02));
#else
	return *(UINT16 *)(RamMain + addr);
#endif
}


UINT32 __fastcall cps3RamReadLong(UINT32 addr)
{
	if (addr == cps3_speedup_ram_address )
		if (Sh2GetPC(0) == cps3_speedup_code_address)
			Sh2BurnUntilInt(0);
		
	addr &= 0x7ffff;
	return *(UINT32 *)(RamMain + addr);
}

// CPS3 Region Patch
static void Cps3PatchRegion()
{
	if ( cps3_region_address ) {

		bprintf(0, _T("Region: %02x -> %02x\n"), RomBios[cps3_region_address], (RomBios[cps3_region_address] & 0xf0) | (cps3_dip & 0x0f));				

#ifdef LSB_FIRST
		RomBios[cps3_region_address] = (RomBios[cps3_region_address] & 0xf0) | (cps3_dip & 0x7f);
#else
		RomBios[cps3_region_address ^ 0x03] = (RomBios[cps3_region_address ^ 0x03] & 0xf0) | (cps3_dip & 0x7f);
#endif
		if ( cps3_ncd_address ) {
			if (cps3_dip & 0x10)
				RomBios[cps3_ncd_address] |= 0x01;
			else
				RomBios[cps3_ncd_address] &= 0xfe;
		}
	}
}

static INT32 Cps3Reset()
{
	// re-map cram_bank
	cram_bank = 0;
	Sh2MapMemory((UINT8 *)RamCRam, 0x04100000, 0x041fffff, SM_RAM);

	Cps3PatchRegion();
	
	// [CD-ROM not emulated] All CHD drivers cause a Guru Meditation with the normal bios boot.
	if(!BurnDrvGetHardwareCode() & HARDWARE_CAPCOM_CPS3_NO_CD){
		// normal boot
		Sh2Reset();
	} else {
		// fast boot
		if (cps3_isSpecial) {
			Sh2Reset( *(UINT32 *)(RomGame + 0), *(UINT32 *)(RomGame + 4) );
			Sh2SetVBR(0x06000000);
		} else {
			Sh2Reset( *(UINT32 *)(RomGame_D + 0), *(UINT32 *)(RomGame_D + 4) );
			Sh2SetVBR(0x06000000);
		}
	}
	
	if (cps3_dip & 0x80) {
		EEPROM[0x11] = 0x100 + (EEPROM[0x11] & 0xff);
		EEPROM[0x29] = 0x100 + (EEPROM[0x29] & 0xff);
	} else {
		EEPROM[0x11] = 0x000 + (EEPROM[0x11] & 0xff);
		EEPROM[0x29] = 0x000 + (EEPROM[0x29] & 0xff);
	}

	cps3_current_eeprom_read = 0;	
	cps3SndReset();	
	cps3_reset = 0;	
	return 0;
}

static void be_to_le(UINT8 * p, INT32 size)
{
	UINT8 c;
	for(INT32 i=0; i<size; i+=4, p+=4) {
		c = *(p+0);	*(p+0) = *(p+3);	*(p+3) = c;
		c = *(p+1);	*(p+1) = *(p+2);	*(p+2) = c;
	}
}

INT32 cps3Init()
{
	INT32 nRet, ii, offset;
	struct BurnRomInfo pri;
	
	// calc graphic and sound roms size
	ii = 0; cps3_data_rom_size = 0;
	while (BurnDrvGetRomInfo(&pri, ii) == 0) {
		if (pri.nType & (BRF_GRA | BRF_SND))
			cps3_data_rom_size += pri.nLen;
		ii++;
	}
	
	// CHD games 
	if (cps3_data_rom_size == 0) cps3_data_rom_size = 0x5000000;	
	
	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();	
	
	// load and decode bios roms
	ii = 0; offset = 0;
	while (BurnDrvGetRomInfo(&pri, ii) == 0) {
		if (pri.nType & BRF_BIOS) {
			nRet = BurnLoadRom(RomBios + offset, ii, 1); 
			if (nRet != 0) return 1;
			offset += pri.nLen;
		}
		ii++;
	}

#ifdef LSB_FIRST
	be_to_le( RomBios, 0x080000 );
#endif
	cps3_decrypt_bios();

	// load and decode sh-2 program roms
	ii = 0;	offset = 0;
	while (BurnDrvGetRomInfo(&pri, ii) == 0) {
		if (pri.nType & BRF_PRG) {
			nRet = BurnLoadRom(RomGame + offset + 0, ii + 0, 4); if (nRet != 0) return 1;
			nRet = BurnLoadRom(RomGame + offset + 1, ii + 1, 4); if (nRet != 0) return 1;
			nRet = BurnLoadRom(RomGame + offset + 2, ii + 2, 4); if (nRet != 0) return 1;
			nRet = BurnLoadRom(RomGame + offset + 3, ii + 3, 4); if (nRet != 0) return 1;
			offset += pri.nLen * 4;
			ii += 4;
		} else {
			ii++;
		}
	}
#ifdef LSB_FIRST
	be_to_le( RomGame, 0x1000000 );
#endif
	cps3_decrypt_game();
	
	// load graphic and sound roms
	ii = 0;	offset = 0;
	while (BurnDrvGetRomInfo(&pri, ii) == 0) {
		if (pri.nType & (BRF_GRA | BRF_SND)) {
			BurnLoadRom(RomUser + offset + 0, ii + 0, 2);
			BurnLoadRom(RomUser + offset + 1, ii + 1, 2);
			offset += pri.nLen * 2;
			ii += 2;
		} else {
			ii++;
		}
	}

	{
		Sh2Init(1);
		Sh2Open(0);

		// Map 68000 memory:
		Sh2MapMemory(RomBios,		0x00000000, 0x0007ffff, SM_ROM);	// BIOS
		Sh2MapMemory(RamMain,		0x02000000, 0x0207ffff, SM_RAM);	// Main RAM
		Sh2MapMemory((UINT8 *) RamSpr,	0x04000000, 0x0407ffff, SM_RAM);
//		Sh2MapMemory(RamCRam,		0x04100000, 0x041fffff, SM_RAM);	// map this while reset
//		Sh2MapMemory(RamGfx,		0x04200000, 0x043fffff, SM_WRITE);
		Sh2MapMemory((UINT8 *) RamSS,	0x05040000, 0x0504ffff, SM_RAM);	// 'SS' RAM (Score Screen) (text tilemap + toles)
		
		Sh2SetReadByteHandler (0, cps3ReadByte);
		Sh2SetReadWordHandler (0, cps3ReadWord);
		Sh2SetReadLongHandler (0, cps3ReadLong);
		Sh2SetWriteByteHandler(0, cps3WriteByte);
		Sh2SetWriteWordHandler(0, cps3WriteWord);
		Sh2SetWriteLongHandler(0, cps3WriteLong);

		Sh2MapMemory(RamC000_D,		0xc0000000, 0xc00003ff, SM_FETCH);	// Executes code from here
		Sh2MapMemory(RamC000,		0xc0000000, 0xc00003ff, SM_READ);
		Sh2MapHandler(1,		0xc0000000, 0xc00003ff, SM_WRITE);

		Sh2SetWriteByteHandler(1, cps3C0WriteByte);
		Sh2SetWriteWordHandler(1, cps3C0WriteWord);
		Sh2SetWriteLongHandler(1, cps3C0WriteLong);

		if( !BurnDrvGetHardwareCode() & HARDWARE_CAPCOM_CPS3_NO_CD ) 
		{		
			if (cps3_isSpecial) {
				Sh2MapMemory(RomGame,	0x06000000, 0x06ffffff, SM_READ);	// Decrypted SH2 Code
				Sh2MapMemory(RomGame_D,	0x06000000, 0x06ffffff, SM_FETCH);	// Decrypted SH2 Code
			} else {
				Sh2MapMemory(RomGame_D,	0x06000000, 0x06ffffff, SM_READ | SM_FETCH);	// Decrypted SH2 Code
			}
		} else {
			Sh2MapMemory(RomGame_D,		0x06000000, 0x06ffffff, SM_FETCH);	// Decrypted SH2 Code
			Sh2MapHandler(2,		0x06000000, 0x06ffffff, SM_READ | SM_WRITE);

			if (cps3_isSpecial) {
				Sh2SetReadByteHandler (2, cps3RomReadByteSpe);
				Sh2SetReadWordHandler (2, cps3RomReadWordSpe);
				Sh2SetReadLongHandler (2, cps3RomReadLongSpe);
				Sh2SetWriteByteHandler(2, cps3RomWriteByte);
				Sh2SetWriteWordHandler(2, cps3RomWriteWord);
				Sh2SetWriteLongHandler(2, cps3RomWriteLong);
			} else {
				Sh2SetReadByteHandler (2, cps3RomReadByte);
				Sh2SetReadWordHandler (2, cps3RomReadWord);
				Sh2SetReadLongHandler (2, cps3RomReadLong);
				Sh2SetWriteByteHandler(2, cps3RomWriteByte);
				Sh2SetWriteWordHandler(2, cps3RomWriteWord);
				Sh2SetWriteLongHandler(2, cps3RomWriteLong);
			}
		}

		Sh2MapHandler(3,			0x040e0000, 0x040e02ff, SM_RAM);
		Sh2SetReadByteHandler (3, cps3SndReadByte);
		Sh2SetReadWordHandler (3, cps3SndReadWord);
		Sh2SetReadLongHandler (3, cps3SndReadLong);
		Sh2SetWriteByteHandler(3, cps3SndWriteByte);
		Sh2SetWriteWordHandler(3, cps3SndWriteWord);
		Sh2SetWriteLongHandler(3, cps3SndWriteLong);
		
		Sh2MapMemory((UINT8 *)RamPal,		0x04080000, 0x040bffff, SM_READ);	// 16bit BE Colors
		Sh2MapHandler(4,			0x04080000, 0x040bffff, SM_WRITE);

		Sh2SetReadByteHandler (4, cps3VidReadByte);
		Sh2SetReadWordHandler (4, cps3VidReadWord);
		Sh2SetReadLongHandler (4, cps3VidReadLong);
		Sh2SetWriteByteHandler(4, cps3VidWriteByte);
		Sh2SetWriteWordHandler(4, cps3VidWriteWord);
		Sh2SetWriteLongHandler(4, cps3VidWriteLong);

#ifdef SPEED_HACK
		// install speedup read handler
		Sh2MapHandler(5,			0x02000000 | (cps3_speedup_ram_address & 0x030000),
							0x0200ffff | (cps3_speedup_ram_address & 0x030000), SM_READ);
		Sh2SetReadByteHandler (5, cps3RamReadByte);
		Sh2SetReadWordHandler (5, cps3RamReadWord);
		Sh2SetReadLongHandler (5, cps3RamReadLong);
#endif

	}
	
	BurnDrvGetVisibleSize(&cps3_gfx_width, &cps3_gfx_height);	
	RamScreen	+= (512 * 2) * 16 + 16; // safe draw	
	cps3SndInit(RomUser);
	
	pBurnDrvPalette = (UINT32*)Cps3CurPal;
		
	Cps3Reset();
	return 0;
}

INT32 cps3Exit()
{
	Sh2Exit();
	
	BurnFree(Mem);

	cps3SndExit();	
	
	return 0;
}


static void cps3_drawgfxzoom_0(UINT32 code, UINT32 pal, INT32 flipx, INT32 flipy, INT32 x, INT32 y)
{
	if ((x > (cps3_gfx_width - 8)) || (y > (cps3_gfx_height - 8))) return;
	UINT16 * dst = (UINT16 *) pBurnDraw;
	UINT8 * src = (UINT8 *)RamSS;
	UINT16 * color = Cps3CurPal + (pal << 4);
	dst += (y * cps3_gfx_width + x);
	src += code * 64;
	
	if ( flipy ) {
		dst += cps3_gfx_width * 7;
#ifdef LSB_FIRST
		if ( flipx )
			for(INT32 i=0; i<8; i++, dst-= cps3_gfx_width, src += 8) {
				if ( src[ 2] & 0xf ) dst[7] = color [ src[ 2] & 0xf ];
				if ( src[ 2] >>  4 ) dst[6] = color [ src[ 2] >>  4 ];
				if ( src[ 0] & 0xf ) dst[5] = color [ src[ 0] & 0xf ];
				if ( src[ 0] >>  4 ) dst[4] = color [ src[ 0] >>  4 ];
				if ( src[ 6] & 0xf ) dst[3] = color [ src[ 6] & 0xf ];
				if ( src[ 6] >>  4 ) dst[2] = color [ src[ 6] >>  4 ];
				if ( src[ 4] & 0xf ) dst[1] = color [ src[ 4] & 0xf ];
				if ( src[ 4] >>  4 ) dst[0] = color [ src[ 4] >>  4 ];
			}
		else
			for(INT32 i=0; i<8; i++, dst-= cps3_gfx_width, src += 8) {
				if ( src[ 2] & 0xf ) dst[0] = color [ src[ 2] & 0xf ];
				if ( src[ 2] >>  4 ) dst[1] = color [ src[ 2] >>  4 ];
				if ( src[ 0] & 0xf ) dst[2] = color [ src[ 0] & 0xf ];
				if ( src[ 0] >>  4 ) dst[3] = color [ src[ 0] >>  4 ];
				if ( src[ 6] & 0xf ) dst[4] = color [ src[ 6] & 0xf ];
				if ( src[ 6] >>  4 ) dst[5] = color [ src[ 6] >>  4 ];
				if ( src[ 4] & 0xf ) dst[6] = color [ src[ 4] & 0xf ];
				if ( src[ 4] >>  4 ) dst[7] = color [ src[ 4] >>  4 ];
			}
		
	} else {
		if ( flipx )
			for(INT32 i=0; i<8; i++, dst+= cps3_gfx_width, src += 8) {
				if ( src[ 2] & 0xf ) dst[7] = color [ src[ 2] & 0xf ];
				if ( src[ 2] >>  4 ) dst[6] = color [ src[ 2] >>  4 ];
				if ( src[ 0] & 0xf ) dst[5] = color [ src[ 0] & 0xf ];
				if ( src[ 0] >>  4 ) dst[4] = color [ src[ 0] >>  4 ];
				if ( src[ 6] & 0xf ) dst[3] = color [ src[ 6] & 0xf ];
				if ( src[ 6] >>  4 ) dst[2] = color [ src[ 6] >>  4 ];
				if ( src[ 4] & 0xf ) dst[1] = color [ src[ 4] & 0xf ];
				if ( src[ 4] >>  4 ) dst[0] = color [ src[ 4] >>  4 ];
			}
		else
			for(INT32 i=0; i<8; i++, dst+= cps3_gfx_width, src += 8) {
				if ( src[ 2] & 0xf ) dst[0] = color [ src[ 2] & 0xf ];
				if ( src[ 2] >>  4 ) dst[1] = color [ src[ 2] >>  4 ];
				if ( src[ 0] & 0xf ) dst[2] = color [ src[ 0] & 0xf ];
				if ( src[ 0] >>  4 ) dst[3] = color [ src[ 0] >>  4 ];
				if ( src[ 6] & 0xf ) dst[4] = color [ src[ 6] & 0xf ];
				if ( src[ 6] >>  4 ) dst[5] = color [ src[ 6] >>  4 ];
				if ( src[ 4] & 0xf ) dst[6] = color [ src[ 4] & 0xf ];
				if ( src[ 4] >>  4 ) dst[7] = color [ src[ 4] >>  4 ];
			}
	}
#else
		if ( flipx )
			for(int i=0; i<8; i++, dst-= cps3_gfx_width, src += 8) {
				if ( src[ 1] & 0xf ) dst[7] = color [ src[ 1] & 0xf ];
				if ( src[ 1] >>  4 ) dst[6] = color [ src[ 1] >>  4 ];
				if ( src[ 3] & 0xf ) dst[5] = color [ src[ 3] & 0xf ];
				if ( src[ 3] >>  4 ) dst[4] = color [ src[ 3] >>  4 ];
				if ( src[ 5] & 0xf ) dst[3] = color [ src[ 5] & 0xf ];
				if ( src[ 5] >>  4 ) dst[2] = color [ src[ 5] >>  4 ];
				if ( src[ 7] & 0xf ) dst[1] = color [ src[ 7] & 0xf ];
				if ( src[ 7] >>  4 ) dst[0] = color [ src[ 7] >>  4 ];
			}
		else
			for(int i=0; i<8; i++, dst-= cps3_gfx_width, src += 8) {
				if ( src[ 1] & 0xf ) dst[0] = color [ src[ 1] & 0xf ];
				if ( src[ 1] >>  4 ) dst[1] = color [ src[ 1] >>  4 ];
				if ( src[ 3] & 0xf ) dst[2] = color [ src[ 3] & 0xf ];
				if ( src[ 3] >>  4 ) dst[3] = color [ src[ 3] >>  4 ];
				if ( src[ 5] & 0xf ) dst[4] = color [ src[ 5] & 0xf ];
				if ( src[ 5] >>  4 ) dst[5] = color [ src[ 5] >>  4 ];
				if ( src[ 7] & 0xf ) dst[6] = color [ src[ 7] & 0xf ];
				if ( src[ 7] >>  4 ) dst[7] = color [ src[ 7] >>  4 ];
			}

	} else {
		if ( flipx )
			for(int i=0; i<8; i++, dst+= cps3_gfx_width, src += 8) {
				if ( src[ 1] & 0xf ) dst[7] = color [ src[ 1] & 0xf ];
				if ( src[ 1] >>  4 ) dst[6] = color [ src[ 1] >>  4 ];
				if ( src[ 3] & 0xf ) dst[5] = color [ src[ 3] & 0xf ];
				if ( src[ 3] >>  4 ) dst[4] = color [ src[ 3] >>  4 ];
				if ( src[ 5] & 0xf ) dst[3] = color [ src[ 5] & 0xf ];
				if ( src[ 5] >>  4 ) dst[2] = color [ src[ 5] >>  4 ];
				if ( src[ 7] & 0xf ) dst[1] = color [ src[ 7] & 0xf ];
				if ( src[ 7] >>  4 ) dst[0] = color [ src[ 7] >>  4 ];
			}
		else
			for(int i=0; i<8; i++, dst+= cps3_gfx_width, src += 8) {
				if ( src[ 1 ] & 0xf ) dst[0] = color [ src[ 1 ] & 0xf ];
				if ( src[ 1 ] >>  4 ) dst[1] = color [ src[ 1 ] >>  4 ];
				if ( src[ 3 ] & 0xf ) dst[2] = color [ src[ 3 ] & 0xf ];
				if ( src[ 3 ] >>  4 ) dst[3] = color [ src[ 3 ] >>  4 ];
				if ( src[ 5 ] & 0xf ) dst[4] = color [ src[ 5 ] & 0xf ];
				if ( src[ 5 ] >>  4 ) dst[5] = color [ src[ 5 ] >>  4 ];
				if ( src[ 7 ] & 0xf ) dst[6] = color [ src[ 7 ] & 0xf ];
				if ( src[ 7 ] >>  4 ) dst[7] = color [ src[ 7 ] >>  4 ];
			}
	}

#endif
	
}

static void cps3_drawgfxzoom_1(UINT32 code, UINT32 pal, INT32 flipx, INT32 flipy, INT32 x, INT32 y, INT32 drawline)
{
	UINT32 * dst = RamScreen;
	UINT8 * src = (UINT8 *) RamCRam;
	dst += (drawline * 1024 + x);

#if BE_GFX

	if ( flipy ) {
		src += code * 256 + 16 * (15 - (drawline - y));
		if ( flipx ) {
			if ( src[ 0] ) dst[15] = src[ 0] | pal;
			if ( src[ 1] ) dst[14] = src[ 1] | pal;
			if ( src[ 2] ) dst[13] = src[ 2] | pal;
			if ( src[ 3] ) dst[12] = src[ 3] | pal;
			if ( src[ 4] ) dst[11] = src[ 4] | pal;
			if ( src[ 5] ) dst[10] = src[ 5] | pal;
			if ( src[ 6] ) dst[ 9] = src[ 6] | pal;
			if ( src[ 7] ) dst[ 8] = src[ 7] | pal;
			if ( src[ 8] ) dst[ 7] = src[ 8] | pal;
			if ( src[ 9] ) dst[ 6] = src[ 9] | pal;
			if ( src[10] ) dst[ 5] = src[10] | pal;
			if ( src[11] ) dst[ 4] = src[11] | pal;
			if ( src[12] ) dst[ 3] = src[12] | pal;
			if ( src[13] ) dst[ 2] = src[13] | pal;
			if ( src[14] ) dst[ 1] = src[14] | pal;
			if ( src[15] ) dst[ 0] = src[15] | pal;
		} else {
			if ( src[ 0] ) dst[ 0] = src[ 0] | pal;
			if ( src[ 1] ) dst[ 1] = src[ 1] | pal;
			if ( src[ 2] ) dst[ 2] = src[ 2] | pal;
			if ( src[ 3] ) dst[ 3] = src[ 3] | pal;
			if ( src[ 4] ) dst[ 4] = src[ 4] | pal;
			if ( src[ 5] ) dst[ 5] = src[ 5] | pal;
			if ( src[ 6] ) dst[ 6] = src[ 6] | pal;
			if ( src[ 7] ) dst[ 7] = src[ 7] | pal;
			if ( src[ 8] ) dst[ 8] = src[ 8] | pal;
			if ( src[ 9] ) dst[ 9] = src[ 9] | pal;
			if ( src[10] ) dst[10] = src[10] | pal;
			if ( src[11] ) dst[11] = src[11] | pal;
			if ( src[12] ) dst[12] = src[12] | pal;
			if ( src[13] ) dst[13] = src[13] | pal;
			if ( src[14] ) dst[14] = src[14] | pal;
			if ( src[15] ) dst[15] = src[15] | pal;
		}
	} else {
		src += code * 256 + 16 * (drawline - y);
		if ( flipx ) {
			if ( src[ 0] ) dst[15] = src[ 0] | pal;
			if ( src[ 1] ) dst[14] = src[ 1] | pal;
			if ( src[ 2] ) dst[13] = src[ 2] | pal;
			if ( src[ 3] ) dst[12] = src[ 3] | pal;
			if ( src[ 4] ) dst[11] = src[ 4] | pal;
			if ( src[ 5] ) dst[10] = src[ 5] | pal;
			if ( src[ 6] ) dst[ 9] = src[ 6] | pal;
			if ( src[ 7] ) dst[ 8] = src[ 7] | pal;
			if ( src[ 8] ) dst[ 7] = src[ 8] | pal;
			if ( src[ 9] ) dst[ 6] = src[ 9] | pal;
			if ( src[10] ) dst[ 5] = src[10] | pal;
			if ( src[11] ) dst[ 4] = src[11] | pal;
			if ( src[12] ) dst[ 3] = src[12] | pal;
			if ( src[13] ) dst[ 2] = src[13] | pal;
			if ( src[14] ) dst[ 1] = src[14] | pal;
			if ( src[15] ) dst[ 0] = src[15] | pal;
		} else {
			if ( src[ 0] ) dst[ 0] = src[ 0] | pal;
			if ( src[ 1] ) dst[ 1] = src[ 1] | pal;
			if ( src[ 2] ) dst[ 2] = src[ 2] | pal;
			if ( src[ 3] ) dst[ 3] = src[ 3] | pal;
			if ( src[ 4] ) dst[ 4] = src[ 4] | pal;
			if ( src[ 5] ) dst[ 5] = src[ 5] | pal;
			if ( src[ 6] ) dst[ 6] = src[ 6] | pal;
			if ( src[ 7] ) dst[ 7] = src[ 7] | pal;
			if ( src[ 8] ) dst[ 8] = src[ 8] | pal;
			if ( src[ 9] ) dst[ 9] = src[ 9] | pal;
			if ( src[10] ) dst[10] = src[10] | pal;
			if ( src[11] ) dst[11] = src[11] | pal;
			if ( src[12] ) dst[12] = src[12] | pal;
			if ( src[13] ) dst[13] = src[13] | pal;
			if ( src[14] ) dst[14] = src[14] | pal;
			if ( src[15] ) dst[15] = src[15] | pal;
		}
	}
	
#else

	if ( flipy ) {
		src += code * 256 + 16 * (15 - (drawline - y));
		if ( flipx ) {
			if ( src[ 3] ) dst[15] = src[ 3] | pal;
			if ( src[ 2] ) dst[14] = src[ 2] | pal;
			if ( src[ 1] ) dst[13] = src[ 1] | pal;
			if ( src[ 0] ) dst[12] = src[ 0] | pal;
			if ( src[ 7] ) dst[11] = src[ 7] | pal;
			if ( src[ 6] ) dst[10] = src[ 6] | pal;
			if ( src[ 5] ) dst[ 9] = src[ 5] | pal;
			if ( src[ 4] ) dst[ 8] = src[ 4] | pal;
			if ( src[11] ) dst[ 7] = src[11] | pal;
			if ( src[10] ) dst[ 6] = src[10] | pal;
			if ( src[ 9] ) dst[ 5] = src[ 9] | pal;
			if ( src[ 8] ) dst[ 4] = src[ 8] | pal;
			if ( src[15] ) dst[ 3] = src[15] | pal;
			if ( src[14] ) dst[ 2] = src[14] | pal;
			if ( src[13] ) dst[ 1] = src[13] | pal;
			if ( src[12] ) dst[ 0] = src[12] | pal;
		} else {
			if ( src[ 3] ) dst[ 0] = src[ 3] | pal;
			if ( src[ 2] ) dst[ 1] = src[ 2] | pal;
			if ( src[ 1] ) dst[ 2] = src[ 1] | pal;
			if ( src[ 0] ) dst[ 3] = src[ 0] | pal;
			if ( src[ 7] ) dst[ 4] = src[ 7] | pal;
			if ( src[ 6] ) dst[ 5] = src[ 6] | pal;
			if ( src[ 5] ) dst[ 6] = src[ 5] | pal;
			if ( src[ 4] ) dst[ 7] = src[ 4] | pal;
			if ( src[11] ) dst[ 8] = src[11] | pal;
			if ( src[10] ) dst[ 9] = src[10] | pal;
			if ( src[ 9] ) dst[10] = src[ 9] | pal;
			if ( src[ 8] ) dst[11] = src[ 8] | pal;
			if ( src[15] ) dst[12] = src[15] | pal;
			if ( src[14] ) dst[13] = src[14] | pal;
			if ( src[13] ) dst[14] = src[13] | pal;
			if ( src[12] ) dst[15] = src[12] | pal;
		}
	} else {
		src += code * 256 + 16 * (drawline - y);
		if ( flipx ) {
			if ( src[ 3] ) dst[15] = src[ 3] | pal;
			if ( src[ 2] ) dst[14] = src[ 2] | pal;
			if ( src[ 1] ) dst[13] = src[ 1] | pal;
			if ( src[ 0] ) dst[12] = src[ 0] | pal;
			if ( src[ 7] ) dst[11] = src[ 7] | pal;
			if ( src[ 6] ) dst[10] = src[ 6] | pal;
			if ( src[ 5] ) dst[ 9] = src[ 5] | pal;
			if ( src[ 4] ) dst[ 8] = src[ 4] | pal;
			if ( src[11] ) dst[ 7] = src[11] | pal;
			if ( src[10] ) dst[ 6] = src[10] | pal;
			if ( src[ 9] ) dst[ 5] = src[ 9] | pal;
			if ( src[ 8] ) dst[ 4] = src[ 8] | pal;
			if ( src[15] ) dst[ 3] = src[15] | pal;
			if ( src[14] ) dst[ 2] = src[14] | pal;
			if ( src[13] ) dst[ 1] = src[13] | pal;
			if ( src[12] ) dst[ 0] = src[12] | pal;
		} else {
			if ( src[ 3] ) dst[ 0] = src[ 3] | pal;
			if ( src[ 2] ) dst[ 1] = src[ 2] | pal;
			if ( src[ 1] ) dst[ 2] = src[ 1] | pal;
			if ( src[ 0] ) dst[ 3] = src[ 0] | pal;
			if ( src[ 7] ) dst[ 4] = src[ 7] | pal;
			if ( src[ 6] ) dst[ 5] = src[ 6] | pal;
			if ( src[ 5] ) dst[ 6] = src[ 5] | pal;
			if ( src[ 4] ) dst[ 7] = src[ 4] | pal;
			if ( src[11] ) dst[ 8] = src[11] | pal;
			if ( src[10] ) dst[ 9] = src[10] | pal;
			if ( src[ 9] ) dst[10] = src[ 9] | pal;
			if ( src[ 8] ) dst[11] = src[ 8] | pal;
			if ( src[15] ) dst[12] = src[15] | pal;
			if ( src[14] ) dst[13] = src[14] | pal;
			if ( src[13] ) dst[14] = src[13] | pal;
			if ( src[12] ) dst[15] = src[12] | pal;
		}
	}

#endif
}

static void cps3_drawgfxzoom_2(UINT32 code, UINT32 pal, INT32 flipx, INT32 flipy, INT32 sx, INT32 sy, INT32 scalex, INT32 scaley, INT32 alpha)
{
	//if (!scalex || !scaley) return;

	UINT8 * source_base = (UINT8 *) RamCRam + code * 256;
	
	INT32 sprite_screen_height = (scaley * 16 + 0x8000) >> 16;
	INT32 sprite_screen_width  = (scalex * 16 + 0x8000) >> 16;	
	if (sprite_screen_width && sprite_screen_height) {
		// compute sprite increment per screen pixel
		INT32 dx = (16 << 16) / sprite_screen_width;
		INT32 dy = (16 << 16) / sprite_screen_height;

		INT32 ex = sx + sprite_screen_width;
		INT32 ey = sy + sprite_screen_height;

		INT32 x_index_base;
		INT32 y_index;

		if( flipx )	{
			x_index_base = (sprite_screen_width - 1) * dx;
			dx = -dx;
		} else
			x_index_base = 0;

		if( flipy )	{
			y_index = (sprite_screen_height - 1) * dy;
			dy = -dy;
		} else 
			y_index = 0;

		{
			if( sx < 0)
			{ /* clip left */
				INT32 pixels = 0-sx;
				sx += pixels;
				x_index_base += pixels*dx;
			}
			if( sy < 0 )
			{ /* clip top */
				INT32 pixels = 0-sy;
				sy += pixels;
				y_index += pixels*dy;
			}
			if( ex > cps3_gfx_max_x+1 )
			{ /* clip right */
				INT32 pixels = ex-cps3_gfx_max_x-1;
				ex -= pixels;
			}
			if( ey > cps3_gfx_max_y+1 )
			{ /* clip bottom */
				INT32 pixels = ey-cps3_gfx_max_y-1;
				ey -= pixels;
			}
		}

		if( ex > sx ) {
			switch( alpha ) {
			case 0:
				for( INT32 y=sy; y<ey; y++ ) {
					UINT8 * source = source_base + (y_index>>16) * 16;
					UINT32 * dest = RamScreen + y * 512 * 2;
					INT32 x_index = x_index_base;
					for(INT32 x=sx; x<ex; x++ ) {
#if BE_GFX
						UINT8 c = source[ (x_index>>16) ];
#else
						UINT8 c = source[ (x_index>>16) ^ 3 ];
#endif
						if( c )	dest[x] = pal | c;
						x_index += dx;
					}
					y_index += dy;
				}
				break;
			case 6:
				for( INT32 y=sy; y<ey; y++ ) {
					UINT8 * source = source_base + (y_index>>16) * 16;
					UINT32 * dest = RamScreen + y * 512 * 2;
					INT32 x_index = x_index_base;
					for(INT32 x=sx; x<ex; x++ ) {
#if BE_GFX
						UINT8 c = source[ (x_index>>16)];
#else
						UINT8 c = source[ (x_index>>16) ^ 3 ];
#endif
						dest[x] |= ((c&0x0000f) << 13);
						x_index += dx;
					}
					y_index += dy;
				}
				break;
			case 8:
				for( INT32 y=sy; y<ey; y++ ) {
					UINT8 * source = source_base + (y_index>>16) * 16;
					UINT32 * dest = RamScreen + y * 512 * 2;
					INT32 x_index = x_index_base;
					for(INT32 x=sx; x<ex; x++ ) {
#if BE_GFX
						UINT8 c = source[ (x_index>>16) ];
#else
						UINT8 c = source[ (x_index>>16) ^ 3 ];
#endif

						if (c) {
							dest[x] |= 0x8000;
							if (pal&0x10000) dest[x] |= 0x10000;
						}
						x_index += dx;
					}
					y_index += dy;
				}
				break;
			}
		}
	}
}

static void cps3_draw_tilemapsprite_line(INT32 drawline, UINT32 * regs )
{
	INT32 scrolly =  ((regs[0]&0x0000ffff)>>0)+4;
	INT32 line = drawline + scrolly;
	line &= 0x3ff;

	if (!(regs[1]&0x00008000)) return;

	{
		UINT32 mapbase =  (regs[2]&0x007f0000)>>16;
		UINT32 linebase=  (regs[2]&0x7f000000)>>24;
		INT32 linescroll_enable = (regs[1]&0x00004000);

		INT32 scrollx;
		INT32 tileline = (line/16)+1;
		INT32 tilesubline = line % 16;

		mapbase = mapbase << 10;
		linebase = linebase << 10;

		if (!linescroll_enable)	{
			scrollx =  (regs[0]&0xffff0000)>>16;
		} else {
			scrollx =  (regs[0]&0xffff0000)>>16;
			scrollx+= (RamSpr[linebase+((line+16-4)&0x3ff)]>>16)&0x3ff;
		}

		if (drawline>cps3_gfx_max_y+4) return;

		for (INT32 x=0;x<(cps3_gfx_max_x/16)+2;x++) {

			UINT32 dat;
			INT32 tileno;
			INT32 colour;
			INT32 bpp;
			INT32 xflip,yflip;

			dat = RamSpr[mapbase+((tileline&63)*64)+((x+scrollx/16)&63)];
			tileno = (dat & 0xffff0000)>>17;
			colour = (dat & 0x000001ff)>>0;
			bpp = (dat & 0x0000200)>>9;
			yflip  = (dat & 0x00000800)>>11;
			xflip  = (dat & 0x00001000)>>12;

			if (!bpp) colour <<= 8;
			else colour <<= 6;

			cps3_drawgfxzoom_1(tileno,colour,xflip,yflip,(x*16)-scrollx%16,drawline-tilesubline, drawline);
		}
	}
}

static INT32 WideScreenFrameDelay = 0;

static void DrvDraw()
{
	INT32 bg_drawn[4] = { 0, 0, 0, 0 };

	UINT32 fullscreenzoom = RamVReg[ 6 * 4 + 3 ] & 0xff;
	UINT32 fullscreenzoomwidecheck = RamVReg[6 * 4 + 1];
	
	if (((fullscreenzoomwidecheck & 0xffff0000) >> 16) == 0x0265) {
		INT32 Width, Height;
		BurnDrvGetVisibleSize(&Width, &Height);
		
		if (Width != 496) {
			BurnDrvSetVisibleSize(496, 224);
			BurnDrvSetAspect(16, 9);
			Reinitialise();
			WideScreenFrameDelay = GetCurrentFrame() + 1;
		}
	} else {
		INT32 Width, Height;
		BurnDrvGetVisibleSize(&Width, &Height);
		
		if (Width != 384) {
			BurnDrvSetVisibleSize(384, 224);
			BurnDrvSetAspect(4, 3);
			Reinitialise();
			WideScreenFrameDelay = GetCurrentFrame() + 1;
		}
	}
	
	if (fullscreenzoom > 0x80) fullscreenzoom = 0x80;
	UINT32 fsz = (fullscreenzoom << (16 - 6));
	
	cps3_gfx_max_x = ((cps3_gfx_width * fsz)  >> 16) - 1;	// 384 ( 496 for SFIII2 Only)
	cps3_gfx_max_y = ((cps3_gfx_height * fsz) >> 16) - 1;	// 224

	if (nBurnLayer & 1)
	{
		UINT32 * pscr = RamScreen;
		INT32 clrsz = (cps3_gfx_max_x + 1) * sizeof(INT32);
		for(INT32 yy = 0; yy<=cps3_gfx_max_y; yy++, pscr += 512*2)
			memset(pscr, 0, clrsz);
	}
	else
	{
		Cps3CurPal[0x20000] = BurnHighCol(0xff, 0x00, 0xff, 0);

		INT32 i;
		for (i = 0; i < 1024 * 448; i++) {
			RamScreen[i] = 0x20000;
		}
	}
	
	// Draw Sprites
	{
		for (INT32 i=0x00000/4;i<0x2000/4;i+=4) {
			INT32 xpos		= (RamSpr[i+1]&0x03ff0000)>>16;
			INT32 ypos		= (RamSpr[i+1]&0x000003ff)>>0;

			INT32 gscroll		= (RamSpr[i+0]&0x70000000)>>28;
			INT32 length		= (RamSpr[i+0]&0x01ff0000)>>14; // how many entries in the sprite table
			UINT32 start		= (RamSpr[i+0]&0x00007ff0)>>4;

			INT32 whichbpp		= (RamSpr[i+2]&0x40000000)>>30; // not 100% sure if this is right, jojo title / characters
			INT32 whichpal		= (RamSpr[i+2]&0x20000000)>>29;
			INT32 global_xflip	= (RamSpr[i+2]&0x10000000)>>28;
			INT32 global_yflip	= (RamSpr[i+2]&0x08000000)>>27;
			INT32 global_alpha	= (RamSpr[i+2]&0x04000000)>>26; // alpha / shadow? set on sfiii2 shadows, and big black image in jojo intro
			INT32 global_bpp	= (RamSpr[i+2]&0x02000000)>>25;
			INT32 global_pal	= (RamSpr[i+2]&0x01ff0000)>>16;

			INT32 gscrollx		= (RamVReg[gscroll]&0x03ff0000)>>16;
			INT32 gscrolly		= (RamVReg[gscroll]&0x000003ff)>>0;
			
			start = (start * 0x100) >> 2;

			if ((RamSpr[i+0]&0xf0000000) == 0x80000000) break;	
		
			for (INT32 j=0; j<length; j+=4) {
				
				UINT32 value1 = (RamSpr[start+j+0]);
				UINT32 value2 = (RamSpr[start+j+1]);
				UINT32 value3 = (RamSpr[start+j+2]);
				UINT32 tileno = (value1&0xfffe0000)>>17;
				INT32 count;
				INT32 xpos2 = (value2 & 0x03ff0000)>>16;
				INT32 ypos2 = (value2 & 0x000003ff)>>0;
				INT32 flipx = (value1 & 0x00001000)>>12;
				INT32 flipy = (value1 & 0x00000800)>>11;
				INT32 alpha = (value1 & 0x00000400)>>10; //? this one is used for alpha effects on warzard
				INT32 bpp =   (value1 & 0x00000200)>>9;
				INT32 pal =   (value1 & 0x000001ff);

				INT32 ysizedraw2 = ((value3 & 0x7f000000)>>24);
				INT32 xsizedraw2 = ((value3 & 0x007f0000)>>16);
				INT32 xx,yy;

				INT32 tilestable[4] = { 8,1,2,4 };
				INT32 ysize2 = ((value3 & 0x0000000c)>>2);
				INT32 xsize2 = ((value3 & 0x00000003)>>0);
				UINT32 xinc,yinc;

				if (ysize2==0) continue;

				if (xsize2==0)
				{
					if (nBurnLayer & 1)
					{
						INT32 tilemapnum = ((value3 & 0x00000030)>>4);
						INT32 startline;
						INT32 endline;
						INT32 height = (value3 & 0x7f000000)>>24;
						UINT32 * regs;

						regs = RamVReg + 8 + tilemapnum * 4;
						endline = value2;
						startline = endline - height;

						startline &=0x3ff;
						endline &=0x3ff;

						if (bg_drawn[tilemapnum]==0)
						{
							UINT32 srcy = 0;
							for (INT32 ry = 0; ry < 224; ry++, srcy += fsz) {
								cps3_draw_tilemapsprite_line( srcy >> 16, regs );
							}
						}

						bg_drawn[tilemapnum] = 1;
					}
				} else {
					if (~nSpriteEnable & 1) continue;

					ysize2 = tilestable[ysize2];
					xsize2 = tilestable[xsize2];

					xinc = ((xsizedraw2+1)<<16) / ((xsize2*0x10));
					yinc = ((ysizedraw2+1)<<16) / ((ysize2*0x10));

					xsize2-=1;
					ysize2-=1;

					flipx ^= global_xflip;
					flipy ^= global_yflip;

					if (!flipx) xpos2+=((xsizedraw2+1)/2);
					else xpos2-=((xsizedraw2+1)/2);

					ypos2+=((ysizedraw2+1)/2);

					if (!flipx) xpos2-= (((xsize2+1)*16*xinc)>>16);
					else  xpos2+= (((xsize2)*16*xinc)>>16);

					if (flipy) ypos2-= ((ysize2*16*yinc)>>16);

					{
						count = 0;
						for (xx=0;xx<xsize2+1;xx++) {
							INT32 current_xpos;

							if (!flipx) current_xpos = (xpos+xpos2+((xx*16*xinc)>>16)  );
							else current_xpos = (xpos+xpos2-((xx*16*xinc)>>16));

							current_xpos += gscrollx;
							current_xpos += 1;
							current_xpos &=0x3ff;
							if (current_xpos&0x200) current_xpos-=0x400;

							for (yy=0;yy<ysize2+1;yy++) {
								INT32 current_ypos;
								INT32 actualpal;

								if (flipy) current_ypos = (ypos+ypos2+((yy*16*yinc)>>16));
								else current_ypos = (ypos+ypos2-((yy*16*yinc)>>16));

								current_ypos += gscrolly;
								current_ypos = 0x3ff-current_ypos;
								current_ypos -= 17;
								current_ypos &=0x3ff;

								if (current_ypos&0x200) current_ypos-=0x400;

								/* use the palette value from the main list or the sublists? */
								if (whichpal) actualpal = global_pal;
								else actualpal = pal;
								
								/* use the bpp value from the main list or the sublists? */
								INT32 color_granularity;
								if (whichbpp) {
									if (!global_bpp) color_granularity = 8;
									else color_granularity = 6;
								} else {
									if (!bpp) color_granularity = 8;
									else color_granularity = 6;
								}
								actualpal <<= color_granularity;

								{
									INT32 realtileno = tileno+count;

									if ( realtileno ) {
										if (global_alpha || alpha) {
											// fix jojo's title in it's intro ???
											if ( global_alpha && (global_pal & 0x100))
												actualpal &= 0x0ffff;
												
											cps3_drawgfxzoom_2(realtileno,actualpal,flipx,flipy,current_xpos,current_ypos,xinc,yinc, color_granularity);
											
										} else {
											cps3_drawgfxzoom_2(realtileno,actualpal,flipx,flipy,current_xpos,current_ypos,xinc,yinc, 0);
										}
									}
									count++;
								}
							}
						}
					}
				}
			}
		}
	}
	
	{
		UINT32 srcx, srcy = 0;
		UINT32 * srcbitmap;
		UINT16 * dstbitmap = (UINT16 * )pBurnDraw;

		for (INT32 rendery=0; rendery<224; rendery++) {
			srcbitmap = RamScreen + (srcy >> 16) * 1024;
			srcx=0;
			for (INT32 renderx=0; renderx<cps3_gfx_width; renderx++, dstbitmap ++) {
				*dstbitmap = Cps3CurPal[ srcbitmap[srcx>>16] ];
				srcx += fsz;
			}
			srcy += fsz;
		}
	}
	
	if (nBurnLayer & 2)
	{
		// bank select? (sfiii2 intro)
		INT32 count = (ss_bank_base & 0x01000000) ? 0x0000 : 0x0800;
		for (INT32 y=0; y<32-4; y++) {
			for (INT32 x=0; x<64; x++, count++) {
				UINT32 data = RamSS[count]; // +0x800 = 2nd bank, used on sfiii2 intro..
				UINT32 tile = (data >> 16) & 0x1ff;
				INT32 pal = (data & 0x003f) >> 1;
				INT32 flipx = data & 0x0080;
				INT32 flipy = data & 0x0040;
				pal += ss_pal_base << 5;

				if (tile == 0) continue; // ok?

				tile+=0x200;
				cps3_drawgfxzoom_0(tile,pal,flipx,flipy,x*8,y*8);
			}
		}
	}
}

static INT32 cps_int10_cnt = 0;

INT32 cps3Frame()
{
	if (cps3_reset)
		Cps3Reset();
		
	if (cps3_palette_change) {
		for(INT32 i=0;i<0x0020000;i++) {
#ifdef LSB_FIRST
			INT32 data = RamPal[i ^ 1];
#else
			INT32 data = RamPal[i];
#endif
			INT32 r = (data & 0x001F) << 3;	// Red
			INT32 g = (data & 0x03E0) >> 2;	// Green
			INT32 b = (data & 0x7C00) >> 7;	// Blue
			r |= r >> 5;
			g |= g >> 5;
			b |= b >> 5;
			Cps3CurPal[i] = BurnHighCol(r, g, b, 0);	
		}
		cps3_palette_change = 0;
	}
	
	if (WideScreenFrameDelay == GetCurrentFrame()) {
		BurnDrvGetVisibleSize(&cps3_gfx_width, &cps3_gfx_height);
		WideScreenFrameDelay = 0;
	}
	
//	EEPROM[0x11] = 0x100 + (EEPROM[0x11] & 0xff);
//	EEPROM[0x29] = 0x100 + (EEPROM[0x29] & 0xff);

	Cps3Input[0] = 0;
	Cps3Input[1] = 0;
	//Cps3Input[2] = 0;
	Cps3Input[3] = 0;
	for (INT32 i=0; i<16; i++) {
		Cps3Input[0] |= (Cps3But1[i] & 1) << i;
		Cps3Input[1] |= (Cps3But2[i] & 1) << i;
		Cps3Input[3] |= (Cps3But3[i] & 1) << i;
	}

	// Clear Opposites
	Cps3ClearOpposites(&Cps3Input[0]);
	Cps3ClearOpposites(&Cps3Input[1]);

	for (INT32 i=0; i<4; i++) {

		Sh2Run(6250000 * 4 / 60 / 4);
		
		if (cps_int10_cnt >= 2) {
			cps_int10_cnt = 0;
			Sh2SetIRQLine(10, SH2_IRQSTATUS_AUTO);
		} else cps_int10_cnt++;

	}
	Sh2SetIRQLine(12, SH2_IRQSTATUS_AUTO);

	cps3SndUpdate();
	
//	bprintf(0, _T("PC: %08x\n"), Sh2GetPC(0));
	
	if (pBurnDraw) DrvDraw();

	return 0;
}

INT32 cps3Scan(INT32 nAction, INT32 *pnMin)
{
	if (pnMin) *pnMin =  0x029672;

	struct BurnArea ba;
	
	if (nAction & ACB_NVRAM) {
		// Save EEPROM configuration
		ba.Data		= EEPROM;
		ba.nLen		= 0x0000400;
		ba.nAddress = 0;
		ba.szName	= "EEPROM RAM";
		BurnAcb(&ba);
	}
	
	if (nAction & ACB_MEMORY_RAM) {
		
		ba.Data		= RamMain;
		ba.nLen		= 0x0080000;
		ba.nAddress = 0;
		ba.szName	= "Main RAM";
		BurnAcb(&ba);

		ba.Data		= RamSpr;
		ba.nLen		= 0x0080000;
		ba.nAddress = 0;
		ba.szName	= "Sprite RAM";
		BurnAcb(&ba);

		ba.Data		= RamSS;
		ba.nLen		= 0x0010000;
		ba.nAddress = 0;
		ba.szName	= "Char ROM";
		BurnAcb(&ba);
		
		ba.Data		= RamVReg;
		ba.nLen		= 0x0000100;
		ba.nAddress = 0;
		ba.szName	= "Video REG";
		BurnAcb(&ba);
		
		ba.Data		= RamC000;
		ba.nLen		= 0x0000400 * 2;
		ba.nAddress = 0;
		ba.szName	= "RAM C000";
		BurnAcb(&ba);				
		
		ba.Data		= RamPal;
		ba.nLen		= 0x0040000;
		ba.nAddress = 0;
		ba.szName	= "Palette";
		BurnAcb(&ba);

		ba.Data		= RamCRam;
		ba.nLen		= 0x0800000;
		ba.nAddress = 0;
		ba.szName	= "Sprite ROM";
		BurnAcb(&ba);

/*		// so huge. need not backup it while NOCD
		// otherwize, need backup gfx also
		ba.Data		= RomGame;
		ba.nLen		= 0x1000000;
		ba.nAddress = 0;
		ba.szName	= "Game ROM";
		BurnAcb(&ba);
*/
	}
	
	if (nAction & ACB_DRIVER_DATA) {
		
		Sh2Scan(nAction);
		cps3SndScan(nAction);
		
		SCAN_VAR(Cps3Input);
		
		SCAN_VAR(ss_bank_base);
		SCAN_VAR(ss_pal_base);
		SCAN_VAR(cram_bank);
		SCAN_VAR(cps3_current_eeprom_read);
		SCAN_VAR(gfxflash_bank);
		
		SCAN_VAR(paldma_source);
		SCAN_VAR(paldma_dest);
		SCAN_VAR(paldma_fade);
		SCAN_VAR(paldma_length);

		SCAN_VAR(chardma_source);
		SCAN_VAR(chardma_table_address);
		
		//SCAN_VAR(main_flash);
		
		//SCAN_VAR(last_normal_byte);
		//SCAN_VAR(lastb);
		//SCAN_VAR(lastb2);
		
		SCAN_VAR(cps_int10_cnt);
				
		if (nAction & ACB_WRITE) {
			
			// rebuild current palette
			cps3_palette_change = 1;
			
			// remap RamCRam
			Sh2MapMemory(((UINT8 *)RamCRam) + (cram_bank << 20), 0x04100000, 0x041fffff, SM_RAM);
			
		}
		
	}
	
	return 0;
}
