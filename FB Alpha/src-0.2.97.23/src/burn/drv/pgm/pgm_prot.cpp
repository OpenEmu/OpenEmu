
#include "pgm.h"
#include "bitswap.h"

static UINT8 *USER1;
static UINT16 *sharedprotram;

static INT32 ptr;

INT32 pstarsScan(INT32 nAction,INT32 *);
INT32 killbldScan(INT32 nAction,INT32 *);
INT32 kov_asic27Scan(INT32 nAction,INT32 *);
INT32 asic3Scan(INT32 nAction,INT32 *);
INT32 asic27aScan(INT32 nAction,INT32 *);
INT32 oldsScan(INT32 nAction, INT32 *);
INT32 oldsplus_asic27aScan(INT32 nAction, INT32 *);
INT32 kovsh_asic27aScan(INT32 nAction,INT32 *);
INT32 svg_asic27aScan(INT32 nAction,INT32 *);
INT32 ddp3Scan(INT32 nAction, INT32 *);

//-----------------------------------------------------------------------------------------------------
// ASIC3 - Oriental Legends

static UINT8 asic3_reg, asic3_latch[3], asic3_x, asic3_y, asic3_z, asic3_h1, asic3_h2;
static UINT16 asic3_hold;

static UINT32 bt(UINT32 v, INT32 bit)
{
	return (v & (1<<bit)) != 0;
}

static void asic3_compute_hold()
{
	// The mode is dependant on the region
	static INT32 modes[4] = { 1, 1, 3, 2 };
	INT32 mode = modes[PgmInput[7] & 3];

	switch(mode) {
	case 1:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 10)^bt(asic3_hold, 8)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 1)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 14);
		break;
	case 2:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 7)^bt(asic3_hold, 6)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 4)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 12);
		break;
	case 3:
		asic3_hold =
			(asic3_hold << 1)
			^0x2bad
			^bt(asic3_hold, 15)^bt(asic3_hold, 10)^bt(asic3_hold, 8)^bt(asic3_hold, 5)
			^bt(asic3_z, asic3_y)
			^(bt(asic3_x, 0) << 4)^(bt(asic3_x, 1) << 6)^(bt(asic3_x, 2) << 10)^(bt(asic3_x, 3) << 12);
		break;
	}
}

static UINT8 pgm_asic3_r()
{
	UINT8 res = 0;
	/* region is supplied by the protection device */
	switch(asic3_reg) {
	case 0x00: res = (asic3_latch[0] & 0xf7) | ((PgmInput[7] << 3) & 0x08); break;
	case 0x01: res = asic3_latch[1]; break;
	case 0x02: res = (asic3_latch[2] & 0x7f) | ((PgmInput[7] << 6) & 0x80); break;
	case 0x03:
		res = (bt(asic3_hold, 15) << 0)
			| (bt(asic3_hold, 12) << 1)
			| (bt(asic3_hold, 13) << 2)
			| (bt(asic3_hold, 10) << 3)
			| (bt(asic3_hold,  7) << 4)
			| (bt(asic3_hold,  9) << 5)
			| (bt(asic3_hold,  2) << 6)
			| (bt(asic3_hold,  5) << 7);
		break;
	case 0x20: res = 0x49; break;
	case 0x21: res = 0x47; break;
	case 0x22: res = 0x53; break;
	case 0x24: res = 0x41; break;
	case 0x25: res = 0x41; break;
	case 0x26: res = 0x7f; break;
	case 0x27: res = 0x41; break;
	case 0x28: res = 0x41; break;
	case 0x2a: res = 0x3e; break;
	case 0x2b: res = 0x41; break;
	case 0x2c: res = 0x49; break;
	case 0x2d: res = 0xf9; break;
	case 0x2e: res = 0x0a; break;
	case 0x30: res = 0x26; break;
	case 0x31: res = 0x49; break;
	case 0x32: res = 0x49; break;
	case 0x33: res = 0x49; break;
	case 0x34: res = 0x32; break;
	}
	return res;
}

static void pgm_asic3_w(UINT16 data)
{
	{
		if(asic3_reg < 3)
			asic3_latch[asic3_reg] = data << 1;
		else if(asic3_reg == 0xa0) {
			asic3_hold = 0;
		} else if(asic3_reg == 0x40) {
			asic3_h2 = asic3_h1;
			asic3_h1 = data;
		} else if(asic3_reg == 0x48) {
			asic3_x = 0;
			if(!(asic3_h2 & 0x0a)) asic3_x |= 8;
			if(!(asic3_h2 & 0x90)) asic3_x |= 4;
			if(!(asic3_h1 & 0x06)) asic3_x |= 2;
			if(!(asic3_h1 & 0x90)) asic3_x |= 1;
		} else if(asic3_reg >= 0x80 && asic3_reg <= 0x87) {
			asic3_y = asic3_reg & 7;
			asic3_z = data;
			asic3_compute_hold();
		}
	}
}

static void pgm_asic3_reg_w(UINT16 data)
{
	asic3_reg = data & 0xff;
}

static void __fastcall asic3_write_word(UINT32 address, UINT16 data)
{
	if (address == 0xc04000) {
		pgm_asic3_reg_w(data);
		return;
	}

	if (address == 0xc0400e) {
		pgm_asic3_w(data);
		return;
	}
}

static UINT16 __fastcall asic3_read_word(UINT32 address)
{
	if (address == 0xc0400e) {
		return pgm_asic3_r();
	}

	return 0;
}

static void reset_asic3()
{
	asic3_latch[0] = asic3_latch[1] = asic3_latch[2] = 0;
	asic3_hold = asic3_reg = asic3_x = asic3_y, asic3_z = asic3_h1 = asic3_h2 = 0;
}

void install_protection_asic3_orlegend()
{
	pPgmScanCallback = asic3Scan;
	pPgmResetCallback = reset_asic3;

	SekOpen(0);
	SekMapHandler(4,	0xc04000, 0xc0400f, SM_READ | SM_WRITE);
		
	SekSetReadWordHandler(4, asic3_read_word);
	SekSetWriteWordHandler(4, asic3_write_word);
	SekClose();
}


//-----------------------------------------------------------------------------------------------------
// ASIC27 - Knights of Valour

static const UINT8 BATABLE[0x40] = {
	0x00,0x29,0x2c,0x35,0x3a,0x41,0x4a,0x4e,0x57,0x5e,0x77,0x79,0x7a,0x7b,0x7c,0x7d,
	0x7e,0x7f,0x80,0x81,0x82,0x85,0x86,0x87,0x88,0x89,0x8a,0x8b,0x8c,0x8d,0x8e,0x90,
	0x95,0x96,0x97,0x98,0x99,0x9a,0x9b,0x9c,0x9e,0xa3,0xd4,0xa9,0xaf,0xb5,0xbb,0xc1
};

static const UINT8 B0TABLE[16] = { 2, 0, 1, 4, 3 }; // Maps char portraits to tables

static UINT32 kov_slots[16];
static UINT16 kov_internal_slot;
static UINT16 kov_key;
static UINT32 kov_response;
static UINT16 kov_value;

static UINT16 kov_c0_value;
static UINT16 kov_cb_value;
static UINT16 kov_fe_value;

void __fastcall kov_asic27_write(UINT32 offset, UINT16 data)
{
	switch (offset & 0x06)
	{
		case 0: kov_value = data; return;

		case 2:
		{
			if ((data >> 8) == 0xff) kov_key = 0xffff;

			kov_value ^= kov_key;

		//	bprintf (PRINT_NORMAL, _T("ASIC27 command: %2.2x data: %4.4x\n"), (data ^ kov_key) & 0xff, kov_value);

			switch ((data ^ kov_key) & 0xff)
			{
				case 0x67: // unknown or status check?
				case 0x8e:
				case 0xa3:
				case 0x33: // kovsgqyz (a3)
				case 0x3a: // kovplus
				case 0xc5: // kovplus
					kov_response = 0x880000;
				break;

				case 0x99: // Reset
					kov_response = 0x880000;
					kov_key = 0;
				break;

				case 0x9d: // Sprite palette offset
					kov_response = 0xa00000 + ((kov_value & 0x1f) * 0x40);
				break;

				case 0xb0: // Read from data table
					kov_response = B0TABLE[kov_value & 0x0f];
				break;

				case 0xb4: // Copy slot 'a' to slot 'b'
				case 0xb7: // kovsgqyz (b4)
				{
					kov_response = 0x880000;

					if (kov_value == 0x0102) kov_value = 0x0100; // why?

					kov_slots[(kov_value >> 8) & 0x0f] = kov_slots[(kov_value >> 0) & 0x0f];
				}
				break;

				case 0xba: // Read from data table
					kov_response = BATABLE[kov_value & 0x3f];
				break;

				case 0xc0: // Text layer 'x' select
					kov_response = 0x880000;
					kov_c0_value = kov_value;
				break;

				case 0xc3: // Text layer offset
					kov_response = 0x904000 + ((kov_c0_value + (kov_value * 0x40)) * 4);
				break;

				case 0xcb: // Background layer 'x' select
					kov_response = 0x880000;
					kov_cb_value = kov_value;
				break;

				case 0xcc: // Background layer offset
					if (kov_value & 0x400) kov_value = -(0x400 - (kov_value & 0x3ff));
					kov_response = 0x900000 + ((kov_cb_value + (kov_value * 0x40)) * 4);
				break;

				case 0xd0: // Text palette offset
				case 0xcd: // kovsgqyz (d0)
					kov_response = 0xa01000 + (kov_value * 0x20);
				break;

				case 0xd6: // Copy slot to slot 0
					kov_response = 0x880000;
					kov_slots[0] = kov_slots[kov_value & 0x0f];
				break;

				case 0xdc: // Background palette offset
				case 0x11: // kovsgqyz (dc)
					kov_response = 0xa00800 + (kov_value * 0x40);
				break;

				case 0xe0: // Sprite palette offset
				case 0x9e: // kovsgqyz (e0)
					kov_response = 0xa00000 + ((kov_value & 0x1f) * 0x40);
				break;

				case 0xe5: // Write slot (low)
				{
					kov_response = 0x880000;

					INT32 sel = (kov_internal_slot >> 12) & 0x0f;
					kov_slots[sel] = (kov_slots[sel] & 0x00ff0000) | ((kov_value & 0xffff) <<  0);
				}
				break;

				case 0xe7: // Write slot (and slot select) (high)
				{
					kov_response = 0x880000;
					kov_internal_slot = kov_value;

					INT32 sel = (kov_internal_slot >> 12) & 0x0f;
					kov_slots[sel] = (kov_slots[sel] & 0x0000ffff) | ((kov_value & 0x00ff) << 16);
				}
				break;

				case 0xf0: // Some sort of status read?
					kov_response = 0x00c000;
				break;

				case 0xf8: // Read slot
				case 0xab: // kovsgqyz (f8)
					kov_response = kov_slots[kov_value & 0x0f] & 0x00ffffff;
				break;

				case 0xfc: // Adjust damage level to char experience level
					kov_response = (kov_value * kov_fe_value) >> 6;
				break;

				case 0xfe: // Damage level adjust
					kov_response = 0x880000;
					kov_fe_value = kov_value;
				break;

				default:
					kov_response = 0x880000;
		//			bprintf (PRINT_NORMAL, _T("Unknown ASIC27 command: %2.2x data: %4.4x\n"), (data ^ kov_key) & 0xff, kov_value);
				break;
			}

			kov_key = (kov_key + 0x0100) & 0xff00;
			if (kov_key == 0xff00) kov_key = 0x0100;
			kov_key |= kov_key >> 8;
		}
		return;

		case 4: return;
	}
}

static UINT16 __fastcall kov_asic27_read(UINT32 offset)
{
	switch (offset & 0x02)
	{
		case 0: return (kov_response >>  0) ^ kov_key;
		case 2: return (kov_response >> 16) ^ kov_key;
	}

	return 0;
}

static void kov_asic27_reset()
{
//	memset(kov_slots, 0, 16 * sizeof(INT32));

	kov_internal_slot = 0;
	kov_key = 0;
	kov_response = 0;
	kov_value = 0;

	kov_c0_value = 0;
	kov_cb_value = 0;
	kov_fe_value = 0;

	memset (PGMUSER0, 0, 0x400);

	*((UINT16*)(PGMUSER0 + 0x00008)) = PgmInput[7]; // region
}

void install_protection_asic27_kov()
{
	pPgmScanCallback = kov_asic27Scan;
	pPgmResetCallback = kov_asic27_reset;

	SekOpen(0);
	SekMapMemory(PGMUSER0,	0x4f0000, 0x4f003f | 0x3ff, SM_READ);

	SekMapHandler(4,	0x500000, 0x500003, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, kov_asic27_read);
	SekSetWriteWordHandler(4, kov_asic27_write);
	SekClose();
}


// preliminary

static INT32 puzzli_54_trigger = 0;

static void __fastcall puzzli2_asic_write(UINT32 offset, UINT16 data)
{
	switch (offset & 0x06)
	{
		case 0: kov_value = data; return;

		case 2:
		{
			if ((data >> 8) == 0xff) kov_key = 0xffff;

			kov_value ^= kov_key;

		//	bprintf (0, _T("ASIC Command: %2.2x, Value: %4.4x,\n"), (data ^ kov_key) & 0xff, kov_value);

			switch ((data ^ kov_key) & 0xff)
			{
				case 0x13: // ASIC status?
					kov_response = 0x74<<16; // 2d or 74! (based on?)
				break;

				case 0x31:
				{
					// how is this selected? command 54?

					// just a wild guess
					if (puzzli_54_trigger) {
						// pc == 1387de
						kov_response = 0x63<<16; // ?
					} else {
						// pc == 14cf58
						kov_response = 0xd2<<16;
					}

					puzzli_54_trigger = 0;
				}
				break;

				case 0x38: // Reset
					kov_response = 0x78<<16;
					kov_key = 0;
					puzzli_54_trigger = 0;
				break;

				case 0x41: // ASIC status?
					kov_response = 0x74<<16;
				break;

				case 0x47: // ASIC status?
					kov_response = 0x74<<16;
				break;

				case 0x52: // ASIC status?
				{
					// how is this selected?

					//if (kov_value == 6) {
						kov_response = (0x74<<16)|1; // |1?
					//} else {
					//	kov_response = 0x74<<16;
					//}
				}
				break;

				case 0x54: // ??
					puzzli_54_trigger = 1;
					kov_response = 0x36<<16;
				break;

				case 0x61: // ??
					kov_response = 0x36<<16;
				break;

				case 0x63: // probably read from a data table?
					kov_response = 0; // wrong...
				break;

				case 0x67: // probably read from a data table?
					kov_response = 0;
				break;

				default:
		//			bprintf (0, _T("ASIC Command %2.2x unknown!\n"), (data ^ kov_key) & 0xff);
					kov_response = 0x74<<16;
				break;
			}

			kov_key = (kov_key + 0x0100) & 0xff00;
			if (kov_key == 0xff00) kov_key = 0x0100;
			kov_key |= kov_key >> 8;
		}
		return;

		case 4: return;
	}
}

void install_protection_asic27a_puzzli2()
{
	pPgmScanCallback = kov_asic27Scan;
	pPgmResetCallback = kov_asic27_reset;

	SekOpen(0);
	SekMapMemory(PGMUSER0,	0x4f0000, 0x4f003f | 0x3ff, SM_READ);

	SekMapHandler(4,	0x500000, 0x500003, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, kov_asic27_read);
	SekSetWriteWordHandler(4, puzzli2_asic_write);
	SekClose();
}


//-----------------------------------------------------------------------------------------------------
// Dragon World 2

#define DW2BITSWAP(s,d,bs,bd)  d=((d&(~(1<<bd)))|(((s>>bs)&1)<<bd))

static UINT16 __fastcall dw2_read_word(UINT32)
{
	// The value at 0x80EECE is computed in the routine at 0x107c18

	UINT16 d = SekReadWord(0x80EECE);
	UINT16 d2 = 0;

	d=(d>>8)|(d<<8);
	DW2BITSWAP(d,d2,7 ,0);
	DW2BITSWAP(d,d2,4 ,1);
	DW2BITSWAP(d,d2,5 ,2);
	DW2BITSWAP(d,d2,2 ,3);
	DW2BITSWAP(d,d2,15,4);
	DW2BITSWAP(d,d2,1 ,5);
	DW2BITSWAP(d,d2,10,6);
	DW2BITSWAP(d,d2,13,7);

	// ... missing bitswaps here (8-15) there is not enough data to know them
	// the code only checks the lowest 8 bits

	return d2;
}

void install_protection_asic25_asic12_dw2()
{
	SekOpen(0);
	SekMapHandler(4,		0xd80000, 0xd80003, SM_READ);
	SekSetReadWordHandler(4,	dw2_read_word);
	SekClose();
}


//-----------------------------------------------------------------------------------------------------
// Killing Blade

static UINT16 kb_cmd;
static UINT16 kb_reg;
static UINT16 kb_ptr;
static UINT32   kb_regs[0x100]; //?

static void IGS022_do_dma(UINT16 src, UINT16 dst, UINT16 size, UINT16 mode)
{
	UINT16 param = mode >> 8;
	UINT16 *PROTROM = (UINT16*)USER1;

	mode &= 0x0f;

	switch (mode)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		{
			for (INT32 x = 0; x < size; x++)
			{
				UINT16 dat2 = BURN_ENDIAN_SWAP_INT16(PROTROM[src + x]);

				UINT8 extraoffset = param & 0xfe;
				UINT8* dectable = (UINT8 *)PROTROM;
				UINT16 extraxor = ((dectable[((x*2)+0+extraoffset)&0xff]) << 8) | (dectable[((x*2)+1+extraoffset)&0xff] << 0);

				dat2 = ((dat2 & 0x00ff)<<8) | ((dat2 & 0xff00)>>8);

				//  mode==0 plain
				if (mode==3) dat2 ^= extraxor;
				if (mode==2) dat2 += extraxor;
				if (mode==1) dat2 -= extraxor;

				sharedprotram[dst + x] = BURN_ENDIAN_SWAP_INT16(dat2);
			}

			if ((mode==3) && (param==0x54) && (src*2==0x2120) && (dst*2==0x2600)) sharedprotram[0x2600 / 2] = BURN_ENDIAN_SWAP_INT16(0x4e75); // hack
		}
		break;

		case 5: // copy
		{
			for (INT32 x = 0; x < size; x++) {
				sharedprotram[dst + x] = PROTROM[src + x];
			}
		}
		break;

		case 6: // swap bytes and nibbles
		{
			for (INT32 x = 0; x < size; x++)
			{
				UINT16 dat = PROTROM[src + x];

				dat = ((dat & 0xf000) >> 12) | ((dat & 0x0f00) >> 4) | ((dat & 0x00f0) << 4) | ((dat & 0x000f) << 12);

				sharedprotram[dst + x] = dat;
			}
		}
		break;

		case 4: // fixed value xor?
		case 7: // params left in memory? nop?
		default: // ?
		break;
	}
}

static void IGS022_handle_command()
{
	UINT16 cmd = BURN_ENDIAN_SWAP_INT16(sharedprotram[0x200/2]);

	if (cmd == 0x6d) // Store values to asic ram
	{
		UINT32 p1 = BURN_ENDIAN_SWAP_INT16((sharedprotram[0x298/2] << 16)) | BURN_ENDIAN_SWAP_INT16(sharedprotram[0x29a/2]);
		UINT32 p2 = BURN_ENDIAN_SWAP_INT16((sharedprotram[0x29c/2] << 16)) | BURN_ENDIAN_SWAP_INT16(sharedprotram[0x29e/2]);

		if ((p2 & 0xffff) == 0x9)	// Set value
		{
			INT32 reg = (p2 >> 16) & 0xffff;
			if (reg & 0x200) kb_regs[reg & 0xff] = p1;
		}
		if ((p2 & 0xffff) == 0x6)	// Add value
		{
			INT32 src1 = (p1 >> 16) & 0x00ff;
			INT32 src2 = (p1 >>  0) & 0x00ff;
			INT32 dst  = (p2 >> 16) & 0x00ff;
			kb_regs[dst] = kb_regs[src2] - kb_regs[src1];
		}
		if ((p2 & 0xffff) == 0x1)	// Add Imm?
		{
			INT32 reg = (p2 >> 16) & 0x00ff;
			INT32 imm = (p1 >>  0) & 0xffff;
			kb_regs[reg] += imm;
		}
		if ((p2 & 0xffff) == 0xa)	// Get value
		{
			INT32 reg = (p1 >> 16) & 0x00ff;
			sharedprotram[0x29c/2] = BURN_ENDIAN_SWAP_INT16((kb_regs[reg] >> 16) & 0xffff);
			sharedprotram[0x29e/2] = BURN_ENDIAN_SWAP_INT16((kb_regs[reg] >>  0) & 0xffff);
		}
	}

	if (cmd == 0x4f) // memcpy with encryption / scrambling
	{
		UINT16 src  = BURN_ENDIAN_SWAP_INT16(sharedprotram[0x290 / 2]) >> 1; // ?
		UINT16 dst  = BURN_ENDIAN_SWAP_INT16(sharedprotram[0x292 / 2]);
		UINT16 size = BURN_ENDIAN_SWAP_INT16(sharedprotram[0x294 / 2]);
		UINT16 mode = BURN_ENDIAN_SWAP_INT16(sharedprotram[0x296 / 2]);

		IGS022_do_dma(src, dst, size, mode);
	}
}

static void killbld_igs025_prot_write(UINT32 offset, UINT16 data)
{
	offset &= 0xf;

	if (offset == 0) {
		kb_cmd = data;
		return;
	}

	switch (kb_cmd)
	{
		case 0:
			kb_reg = data;
		break;

		case 2:
			if (data == 1) {
				IGS022_handle_command();
				kb_reg++;
			}
		break;

		case 4:
			kb_ptr = data;
		break;

		case 0x20:
			kb_ptr++;
		break;

		default:
			break;
	}
}

static UINT16 killbld_igs025_prot_read(UINT32 offset)
{
	offset &= 0xf;

	if (offset == 1)
	{
		if (kb_cmd == 1)
		{
			return (kb_reg & 0x7f);
		}
		else if (kb_cmd == 5)
		{
			UINT32 protvalue = 0x89911400 | PgmInput[7];
			return (protvalue >> (8 * (kb_ptr - 1))) & 0xff;
		}
	}

	return 0;
}

static void __fastcall killbld_write_word(UINT32 address, UINT16 data)
{
	killbld_igs025_prot_write(address / 2, data);
}

static UINT16 __fastcall killbld_read_word(UINT32 address)
{
	return killbld_igs025_prot_read(address / 2);
}

static void IGS022Reset()
{
	sharedprotram = (UINT16*)PGMUSER0;
	USER1 = PGMUSER0 + 0x10000;

	if (strcmp(BurnDrvGetTextA(DRV_NAME), "killbld") == 0) {
		BurnLoadRom(USER1, 11, 1); // load protection data
	} else {
		BurnLoadRom(USER1, 14, 1); // load protection data
	}

	BurnByteswap(USER1, 0x10000);

	UINT16 *PROTROM = (UINT16*)USER1;

	for (INT32 i = 0; i < 0x4000/2; i++)
		sharedprotram[i] = 0xa55a;

	UINT16 src  = PROTROM[0x100 / 2];
	UINT16 dst  = PROTROM[0x102 / 2];
	UINT16 size = PROTROM[0x104 / 2];
	UINT16 mode = PROTROM[0x106 / 2];
	UINT16 tmp  = PROTROM[0x114 / 2];

	src  = (( src & 0xff00) >> 8) | (( src & 0x00ff) << 8);
	dst  = (( dst & 0xff00) >> 8) | (( dst & 0x00ff) << 8);
	size = ((size & 0xff00) >> 8) | ((size & 0x00ff) << 8);
	tmp  = (( tmp & 0xff00) >> 8) | (( tmp & 0x00ff) << 8);
	mode &= 0xff;

	src >>= 1;

	IGS022_do_dma(src, dst, size, mode);

	sharedprotram[0x2a2/2] = tmp; // crc check?

	kb_cmd = kb_reg = kb_ptr = 0;
	memset (kb_regs, 0, 0x100 * sizeof(INT32));
}

void install_protection_asic25_asic22_killbld()
{
	pPgmScanCallback = killbldScan;
	pPgmResetCallback = IGS022Reset;

	sharedprotram = (UINT16*)PGMUSER0;

	SekOpen(0);
	SekMapMemory(PGMUSER0,	0x300000, 0x303fff, SM_RAM);
	SekMapHandler(4,	0xd40000, 0xd40003, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, killbld_read_word);
	SekSetWriteWordHandler(4, killbld_write_word);
	SekClose();
}

//-------------------------------------------------------------------------
// Puzzle Stars


static UINT16 PSTARSKEY;
static UINT16 PSTARSINT[2];
static UINT32 PSTARS_REGS[16];
static UINT32 PSTARS_VAL;

static UINT16 pstar_e7,pstar_b1,pstar_ce;
static UINT16 pstar_ram[3];

static INT32 Pstar_ba[0x1E]={
	0x02,0x00,0x00,0x01,0x00,0x03,0x00,0x00, //0
	0x02,0x00,0x06,0x00,0x22,0x04,0x00,0x03, //8
	0x00,0x00,0x06,0x00,0x20,0x07,0x00,0x03, //10
	0x00,0x21,0x01,0x00,0x00,0x63
};

static INT32 Pstar_b0[0x10]={
	0x09,0x0A,0x0B,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x00,0x00,0x00,0x00
};

static INT32 Pstar_ae[0x10]={
	0x5D,0x86,0x8C ,0x8B,0xE0,0x8B,0x62,0xAF,
	0xB6,0xAF,0x10A,0xAF,0x00,0x00,0x00,0x00
};

static INT32 Pstar_a0[0x10]={
	0x02,0x03,0x04,0x05,0x06,0x01,0x0A,0x0B,
	0x0C,0x0D,0x0E,0x09,0x00,0x00,0x00,0x00,
};

static INT32 Pstar_9d[0x10]={
	0x05,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
};

static INT32 Pstar_90[0x10]={
	0x0C,0x10,0x0E,0x0C,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};
static INT32 Pstar_8c[0x23]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x02,0x02,0x02,0x02,
	0x02,0x02,0x03,0x03,0x03,0x04,0x04,0x04,
	0x03,0x03,0x03
};

static INT32 Pstar_80[0x1a3]={
	0x03,0x03,0x04,0x04,0x04,0x04,0x05,0x05,
	0x05,0x05,0x06,0x06,0x03,0x03,0x04,0x04,
	0x05,0x05,0x05,0x05,0x06,0x06,0x07,0x07,
	0x03,0x03,0x04,0x04,0x05,0x05,0x05,0x05,
	0x06,0x06,0x07,0x07,0x06,0x06,0x06,0x06,
	0x06,0x06,0x06,0x07,0x07,0x07,0x07,0x07,
	0x06,0x06,0x06,0x06,0x06,0x06,0x07,0x07,
	0x07,0x07,0x08,0x08,0x05,0x05,0x05,0x05,
	0x05,0x05,0x05,0x06,0x06,0x06,0x07,0x07,
	0x06,0x06,0x06,0x07,0x07,0x07,0x08,0x08,
	0x09,0x09,0x09,0x09,0x07,0x07,0x07,0x07,
	0x07,0x08,0x08,0x08,0x08,0x09,0x09,0x09,
	0x06,0x06,0x07,0x07,0x07,0x08,0x08,0x08,
	0x08,0x08,0x09,0x09,0x05,0x05,0x06,0x06,
	0x06,0x07,0x07,0x08,0x08,0x08,0x08,0x09,
	0x07,0x07,0x07,0x07,0x07,0x08,0x08,0x08,
	0x08,0x09,0x09,0x09,0x06,0x06,0x07,0x03,
	0x07,0x06,0x07,0x07,0x08,0x07,0x05,0x04,
	0x03,0x03,0x04,0x04,0x05,0x05,0x06,0x06,
	0x06,0x06,0x06,0x06,0x03,0x04,0x04,0x04,
	0x04,0x05,0x05,0x06,0x06,0x06,0x06,0x07,
	0x04,0x04,0x05,0x05,0x06,0x06,0x06,0x06,
	0x06,0x07,0x07,0x08,0x05,0x05,0x06,0x07,
	0x07,0x08,0x08,0x08,0x08,0x08,0x08,0x08,
	0x05,0x05,0x05,0x07,0x07,0x07,0x07,0x07,
	0x07,0x08,0x08,0x08,0x08,0x08,0x09,0x09,
	0x09,0x09,0x03,0x04,0x04,0x05,0x05,0x05,
	0x06,0x06,0x07,0x07,0x07,0x07,0x08,0x08,
	0x08,0x09,0x09,0x09,0x03,0x04,0x05,0x05,
	0x04,0x03,0x04,0x04,0x04,0x05,0x05,0x04,
	0x03,0x03,0x03,0x03,0x03,0x03,0x03,0x03,
	0x03,0x03,0x03,0x04,0x04,0x04,0x04,0x04,
	0x04,0x04,0x04,0x04,0x04,0x03,0x03,0x03,
	0x03,0x03,0x03,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0xFF,0x00,0x00,0x00,
	0x00,0xFF,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00
};

static UINT16 PSTARS_protram_r(UINT32 offset)
{
	offset >>= 1;

	if (offset == 4)		//region
		return PgmInput[7];
	else if (offset >= 0x10)  //timer
	{
		return pstar_ram[offset-0x10]--;
	}
	return 0x0000;
}

static UINT16 PSTARS_r16(UINT32 offset)
{
	offset >>= 1;

	if(offset==0)
	{
		UINT16 d=PSTARS_VAL&0xffff;
		UINT16 realkey;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		d^=realkey;
		return d;
	}
	else if(offset==1)
	{
		UINT16 d=PSTARS_VAL>>16;
		UINT16 realkey;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		d^=realkey;
		return d;

	}
	return 0xff;
}

static void PSTARS_w16(UINT32 offset, UINT16 data)
{
	offset >>= 1;

	if(offset==0)
	{
		PSTARSINT[0]=data;
		return;
	}

	if(offset==1)
	{
		UINT16 realkey;
		if((data>>8)==0xff) PSTARSKEY=0xff00;
		realkey=PSTARSKEY>>8;
		realkey|=PSTARSKEY;
		{
			PSTARSKEY+=0x100;
			PSTARSKEY&=0xff00;
			if(PSTARSKEY==0xff00)PSTARSKEY=0x100;
		}
		data^=realkey;
		PSTARSINT[1]=data;
		PSTARSINT[0]^=realkey;

		switch(PSTARSINT[1]&0xff)
			{
				case 0x99:
				{
					PSTARSKEY=0x100;
					PSTARS_VAL=0x880000;
				}
				break;

				case 0xE0:
					{
						PSTARS_VAL=0xa00000+(PSTARSINT[0]<<6);
					}
					break;
				case 0xDC:
					{
						PSTARS_VAL=0xa00800+(PSTARSINT[0]<<6);
					}
					break;
				case 0xD0:
					{
						PSTARS_VAL=0xa01000+(PSTARSINT[0]<<5);
					}
					break;

				case 0xb1:
					{
						pstar_b1=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xbf:
					{
						PSTARS_VAL=pstar_b1*PSTARSINT[0];
					}
					break;

				case 0xc1: //TODO:TIMER  0,1,2,FIX TO 0 should be OK?
					{
						PSTARS_VAL=0;
					}
					break;
				case 0xce: //TODO:TIMER  0,1,2
					{
						pstar_ce=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xcf: //TODO:TIMER  0,1,2
					{
						pstar_ram[pstar_ce]=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;


				case 0xe7:
					{
						pstar_e7=(PSTARSINT[0]>>12)&0xf;
						PSTARS_REGS[pstar_e7]&=0xffff;
						PSTARS_REGS[pstar_e7]|=(PSTARSINT[0]&0xff)<<16;
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xe5:
					{

						PSTARS_REGS[pstar_e7]&=0xff0000;
						PSTARS_REGS[pstar_e7]|=PSTARSINT[0];
						PSTARS_VAL=0x890000;
					}
					break;
				case 0xf8: //@73C
	   			{
	    			PSTARS_VAL=PSTARS_REGS[PSTARSINT[0]&0xf]&0xffffff;
	   			}
	   			break;


				case 0xba:
	   			{
	    			PSTARS_VAL=Pstar_ba[PSTARSINT[0]];
	   			}
	   			break;
				case 0xb0:
	   			{
	    			PSTARS_VAL=Pstar_b0[PSTARSINT[0]];
	   			}
	   			break;
				case 0xae:
	   			{
	    			PSTARS_VAL=Pstar_ae[PSTARSINT[0]];
	   			}
	   			break;
				case 0xa0:
	   			{
	    			PSTARS_VAL=Pstar_a0[PSTARSINT[0]];
	   			}
	   			break;
				case 0x9d:
	   			{
	    			PSTARS_VAL=Pstar_9d[PSTARSINT[0]];
	   			}
	   			break;
				case 0x90:
	   			{
	    			PSTARS_VAL=Pstar_90[PSTARSINT[0]];
	   			}
	   			break;
				case 0x8c:
	   			{
	    			PSTARS_VAL=Pstar_8c[PSTARSINT[0]];
	   			}
	   			break;
				case 0x80:
	   			{
	    			PSTARS_VAL=Pstar_80[PSTARSINT[0]];
	   			}
	   			break;
				default:
					 PSTARS_VAL=0x890000;
		}
	}
}

void __fastcall pstars_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfffffc) == 0x500000) {
		PSTARS_w16(address & 3, data);
	}
}

UINT8 __fastcall pstars_read_byte(UINT32 address)
{
	if ((address & 0xff0000) == 0x4f0000) {
		return PSTARS_protram_r(address & 0xffff);
	}

	if ((address & 0xfffffc) == 0x500000) {
		return PSTARS_r16(address & 3);
	}

	return 0;
}

UINT16 __fastcall pstars_read_word(UINT32 address)
{
	if ((address & 0xff0000) == 0x4f0000) {
		return PSTARS_protram_r(address & 0xffff);
	}

	if ((address & 0xfffffc) == 0x500000) {
		return PSTARS_r16(address & 3);
	}

	return 0;
}

static void reset_puzlstar()
{
	PSTARSKEY = 0;
	PSTARS_VAL = 0;
	PSTARSINT[0] = PSTARSINT[1] = 0;
	pstar_e7 = pstar_b1 = pstar_ce = 0;

	memset(PSTARS_REGS, 0, 16);
	memset(pstar_ram,   0,  3);
}

void install_protection_asic27a_puzlstar()
{
	pPgmScanCallback = pstarsScan;
	pPgmResetCallback = reset_puzlstar;

	SekOpen(0);
	SekMapHandler(4,		0x4f0000, 0x500003, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, 	pstars_read_word);
	SekSetReadByteHandler(4, 	pstars_read_byte);
	SekSetWriteWordHandler(4, 	pstars_write_word);
	SekClose();
}


//-----------------------------------------------------------------------------------------------------
// ASIC27A - Kov2, Martmast, etc

static UINT8 asic27a_to_arm = 0;
static UINT8 asic27a_to_68k = 0;

static inline void pgm_cpu_sync()
{
	INT32 nCycles = SekTotalCycles() - Arm7TotalCycles();

	if (nCycles > 0) {
		Arm7Run(nCycles);
	}
}

static void __fastcall asic27a_write_byte(UINT32 address, UINT8 data)
{
#if 0
	if ((address & 0xff0000) == 0xd00000) {
		//pgm_cpu_sync();
		PGMARMShareRAM[(address & 0xffff)^1] = data;
		return;
	}
#endif

	if ((address & 0xfffffe) == 0xd10000) {	// ddp2
		pgm_cpu_sync();
		asic27a_to_arm = data;
		Arm7SetIRQLine(ARM7_FIRQ_LINE, ARM7_ASSERT_LINE);
		return;
	}
}

static void __fastcall asic27a_write_word(UINT32 address, UINT16 data)
{
#if 0
	if ((address & 0xff0000) == 0xd00000) {
		//pgm_cpu_sync();
		*((UINT16*)(PGMARMShareRAM + (address & 0xfffe))) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
#endif

	if ((address & 0xfffffe) == 0xd10000) {
		pgm_cpu_sync();
		asic27a_to_arm = data & 0xff;
		Arm7SetIRQLine(ARM7_FIRQ_LINE, ARM7_ASSERT_LINE);
		return;
	}
}

static UINT8 __fastcall asic27a_read_byte(UINT32 address)
{
#if 0
	if ((address & 0xff0000) == 0xd00000) {
		//pgm_cpu_sync();
		return PGMARMShareRAM[(address & 0xffff)^1];
	}
#endif

	if ((address & 0xfffffc) == 0xd10000) {
		pgm_cpu_sync();
		return asic27a_to_68k;
	}

	return 0;
}

static UINT16 __fastcall asic27a_read_word(UINT32 address)
{
#if 0
	if ((address & 0xff0000) == 0xd00000) {
		//pgm_cpu_sync();
		return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(PGMARMShareRAM + (address & 0xfffe))));
	}
#endif

	if ((address & 0xfffffc) == 0xd10000) {
		pgm_cpu_sync();
		return asic27a_to_68k;
	}

	return 0;
}

static void asic27a_arm7_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x38000000:
			asic27a_to_68k = data;
		return;
	}
}

static UINT8 asic27a_arm7_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x38000000:
			Arm7SetIRQLine(ARM7_FIRQ_LINE, ARM7_CLEAR_LINE);
			return asic27a_to_arm;
	}

	return 0;
}

void install_protection_asic27a_martmast()
{
	nPGMArm7Type = 2;
	pPgmScanCallback = asic27aScan;

	SekOpen(0);

	SekMapMemory(PGMARMShareRAM,	0xd00000, 0xd0ffff, SM_RAM);

	SekMapHandler(4,		0xd10000, 0xd10003, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, asic27a_read_word);
	SekSetReadByteHandler(4, asic27a_read_byte);
	SekSetWriteWordHandler(4, asic27a_write_word);
	SekSetWriteByteHandler(4, asic27a_write_byte);
	SekClose();

	Arm7Init(1);
	Arm7Open(0);
	Arm7MapMemory(PGMARMROM,	0x00000000, 0x00003fff, ARM7_ROM);
	Arm7MapMemory(PGMUSER0,		0x08000000, 0x08000000+(nPGMExternalARMLen-1), ARM7_ROM);
	Arm7MapMemory(PGMARMRAM0,	0x10000000, 0x100003ff, ARM7_RAM);
	Arm7MapMemory(PGMARMRAM1,	0x18000000, 0x1800ffff, ARM7_RAM);
	Arm7MapMemory(PGMARMShareRAM,	0x48000000, 0x4800ffff, ARM7_RAM);
	Arm7MapMemory(PGMARMRAM2,	0x50000000, 0x500003ff, ARM7_RAM);
	Arm7SetWriteByteHandler(asic27a_arm7_write_byte);
	Arm7SetReadByteHandler(asic27a_arm7_read_byte);
	Arm7Close();
}


//----------------------------------------------------------------------------------------------------------
// Kovsh/Photoy2k/Photoy2k2 asic27a emulation... (thanks to XingXing!)

static UINT16 kovsh_highlatch_arm_w = 0;
static UINT16 kovsh_lowlatch_arm_w = 0;
static UINT16 kovsh_highlatch_68k_w = 0;
static UINT16 kovsh_lowlatch_68k_w = 0;
static UINT32 kovsh_counter = 1;

static void __fastcall kovsh_asic27a_write_word(UINT32 address, UINT16 data)
{
	switch (address)
	{
		case 0x500000:
		case 0x600000:
			kovsh_lowlatch_68k_w = data;
		return;

		case 0x500002:
		case 0x600002:
			kovsh_highlatch_68k_w = data;
		return;
	}
}

static UINT16 __fastcall kovsh_asic27a_read_word(UINT32 address)
{
	if ((address & 0xffffc0) == 0x4f0000) {
		return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(PGMARMShareRAM + (address & 0x3e))));
	}

	switch (address)
	{
		case 0x500000:
		case 0x600000:
			pgm_cpu_sync();
			return kovsh_lowlatch_arm_w;

		case 0x500002:
		case 0x600002:
			pgm_cpu_sync();
			return kovsh_highlatch_arm_w;
	}

	return 0;
}

static void kovsh_asic27a_arm7_write_word(UINT32 address, UINT16 data)
{
	// written... but never read?
	if ((address & 0xffffff80) == 0x50800000) {
		*((UINT16*)(PGMARMShareRAM + ((address>>1) & 0x3e))) = BURN_ENDIAN_SWAP_INT16(data);
		return;
	}
}

static void kovsh_asic27a_arm7_write_long(UINT32 address, UINT32 data)
{
	switch (address)
	{
		case 0x40000000:
		{
			kovsh_highlatch_arm_w = data >> 16;
			kovsh_lowlatch_arm_w = data;

			kovsh_highlatch_68k_w = 0;
			kovsh_lowlatch_68k_w = 0;
		}
		return;
	}
}

static UINT32 kovsh_asic27a_arm7_read_long(UINT32 address)
{
	switch (address)
	{
		case 0x40000000:
			return (kovsh_highlatch_68k_w << 16) | (kovsh_lowlatch_68k_w);

		case 0x4000000c:
			return kovsh_counter++;
	}

	return 0;
}

void install_protection_asic27a_kovsh()
{
	nPGMArm7Type = 1;
	pPgmScanCallback = kovsh_asic27aScan;

	SekOpen(0);
	SekMapMemory(PGMARMShareRAM,	0x4f0000, 0x4f003f, SM_RAM);

	SekMapHandler(4,		0x500000, 0x600005, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, 	kovsh_asic27a_read_word);
	SekSetWriteWordHandler(4, 	kovsh_asic27a_write_word);
	SekClose();

	Arm7Init(1);
	Arm7Open(0);
	Arm7MapMemory(PGMARMROM,	0x00000000, 0x00003fff, ARM7_ROM);
	Arm7MapMemory(PGMARMRAM0,	0x10000000, 0x100003ff, ARM7_RAM);
	Arm7MapMemory(PGMARMRAM2,	0x50000000, 0x500003ff, ARM7_RAM);
	Arm7SetWriteWordHandler(kovsh_asic27a_arm7_write_word);
	Arm7SetWriteLongHandler(kovsh_asic27a_arm7_write_long);
	Arm7SetReadLongHandler(kovsh_asic27a_arm7_read_long);
	Arm7Close();
}


//-------------------------------------------------------------------------------------------
// Kovshp hack -- Intercept commands and translate them to those used by kovsh

void __fastcall kovshp_asic27a_write_word(UINT32 address, UINT16 data)
{
	switch (address & 6)
	{
		case 0:
			kovsh_lowlatch_68k_w = data;
		return;

		case 2:
		{
			unsigned char asic_key = data >> 8;
			unsigned char asic_cmd = (data & 0xff) ^ asic_key;

			switch (asic_cmd)
			{
				case 0x9a: asic_cmd = 0x99; break; // kovassga
				case 0xa6: asic_cmd = 0xa9; break; // kovassga
				case 0xaa: asic_cmd = 0x56; break; // kovassga
				case 0xf8: asic_cmd = 0xf3; break; // kovassga

		                case 0x38: asic_cmd = 0xad; break;
		                case 0x43: asic_cmd = 0xca; break;
		                case 0x56: asic_cmd = 0xac; break;
		                case 0x73: asic_cmd = 0x93; break;
		                case 0x84: asic_cmd = 0xb3; break;
		                case 0x87: asic_cmd = 0xb1; break;
		                case 0x89: asic_cmd = 0xb6; break;
		                case 0x93: asic_cmd = 0x73; break;
		                case 0xa5: asic_cmd = 0xa9; break;
		                case 0xac: asic_cmd = 0x56; break;
		                case 0xad: asic_cmd = 0x38; break;
		                case 0xb1: asic_cmd = 0x87; break;
		                case 0xb3: asic_cmd = 0x84; break;
		                case 0xb4: asic_cmd = 0x90; break;
		                case 0xb6: asic_cmd = 0x89; break;
		                case 0xc5: asic_cmd = 0x8c; break;
		                case 0xca: asic_cmd = 0x43; break;
		                case 0xcc: asic_cmd = 0xf0; break;
		                case 0xd0: asic_cmd = 0xe0; break;
		                case 0xe0: asic_cmd = 0xd0; break;
		                case 0xe7: asic_cmd = 0x70; break;
		                case 0xed: asic_cmd = 0xcb; break;
		                case 0xf0: asic_cmd = 0xcc; break;
		                case 0xf1: asic_cmd = 0xf5; break;
		                case 0xf2: asic_cmd = 0xf1; break;
		                case 0xf4: asic_cmd = 0xf2; break;
		                case 0xf5: asic_cmd = 0xf4; break;
		                case 0xfc: asic_cmd = 0xc0; break;
		                case 0xfe: asic_cmd = 0xc3; break;
			}

			kovsh_highlatch_68k_w = asic_cmd ^ (asic_key | (asic_key << 8));
		}
		return;
	}
}

void install_protection_asic27a_kovshp()
{
	nPGMArm7Type = 1;
	pPgmScanCallback = kovsh_asic27aScan;

	SekOpen(0);
	SekMapMemory(PGMARMShareRAM,	0x4f0000, 0x4f003f, SM_RAM);

	SekMapHandler(4,		0x500000, 0x600005, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, 	kovsh_asic27a_read_word);
	SekSetWriteWordHandler(4, 	kovshp_asic27a_write_word);
	SekClose();

	Arm7Init(1);
	Arm7Open(0);
	Arm7MapMemory(PGMARMROM,	0x00000000, 0x00003fff, ARM7_ROM);
	Arm7MapMemory(PGMARMRAM0,	0x10000000, 0x100003ff, ARM7_RAM);
	Arm7MapMemory(PGMARMRAM2,	0x50000000, 0x500003ff, ARM7_RAM);
	Arm7SetWriteWordHandler(kovsh_asic27a_arm7_write_word);
	Arm7SetWriteLongHandler(kovsh_asic27a_arm7_write_long);
	Arm7SetReadLongHandler(kovsh_asic27a_arm7_read_long);
	Arm7Close();
}


//----------------------------------------------------------------------------------------------------------
// olds

static INT32 rego;
static UINT16 olds_bs,olds_cmd3;

static UINT32 olds_prot_addr( UINT16 addr )
{
	UINT32 mode = addr & 0xff;
	UINT32 offset = addr >> 8;
	UINT32 realaddr;

	switch(mode)
	{
		case 0x0:
		case 0x5:
		case 0xa:
			realaddr = 0x402a00 + (offset << 2);
			break;

		case 0x2:
		case 0x8:
			realaddr = 0x402e00 + (offset << 2);
			break;

		case 0x1:
			realaddr = 0x40307e;
			break;

		case 0x3:
			realaddr = 0x403090;
			break;

		case 0x4:
			realaddr = 0x40309a;
			break;

		case 0x6:
			realaddr = 0x4030a4;
			break;

		case 0x7:
			realaddr = 0x403000;
			break;

		case 0x9:
			realaddr = 0x40306e;
			break;

		default:
			realaddr = 0;
	}
	return realaddr;
}

static UINT32 olds_read_reg(UINT16 addr)
{
	UINT32 protaddr = (olds_prot_addr(addr) - 0x400000) / 2;
	return sharedprotram[protaddr] << 16 | sharedprotram[protaddr + 1];
}

static void olds_write_reg(UINT16 addr, UINT32 val)
{
	sharedprotram[(olds_prot_addr(addr) - 0x400000) / 2]     = val >> 16;
	sharedprotram[(olds_prot_addr(addr) - 0x400000) / 2 + 1] = val & 0xffff;
}

UINT16 __fastcall olds_protection_read(UINT32 address)
{
	UINT16 res = 0;

	if (address & 2)
	{
		if (kb_cmd == 1)
			res = kb_reg & 0x7f;
		if (kb_cmd == 2)
			res = olds_bs | 0x80;
		if (kb_cmd == 3)
			res = olds_cmd3;
		else if (kb_cmd == 5)
		{
			UINT32 protvalue = 0x900000 | PgmInput[7]; // region from protection device.
			res = (protvalue >> (8 * (kb_ptr - 1))) & 0xff; // includes region 1 = taiwan , 2 = china, 3 = japan (title = orlegend special), 4 = korea, 5 = hongkong, 6 = world

		}
	}

	return res;
}

void __fastcall olds_protection_write(UINT32 address, UINT16 data)
{
	if ((address & 2) == 0)
		kb_cmd = data;
	else //offset==2
	{
		if (kb_cmd == 0)
			kb_reg = data;
		else if(kb_cmd == 2)	//a bitswap=
		{
			INT32 reg = 0;
			if (data & 0x01) reg |= 0x40;
			if (data & 0x02) reg |= 0x80;
			if (data & 0x04) reg |= 0x20;
			if (data & 0x08) reg |= 0x10;
			olds_bs = reg;
		}
		else if (kb_cmd == 3)
		{
			UINT16 cmd = sharedprotram[0x3026 / 2];

			switch (cmd)
			{
				case 0x11:
				case 0x12:
						break;
				case 0x64:
					{
						UINT16 cmd0 = sharedprotram[0x3082 / 2];
						UINT16 val0 = sharedprotram[0x3050 / 2];	//CMD_FORMAT
						{
							if ((cmd0 & 0xff) == 0x2)
								olds_write_reg(val0, olds_read_reg(val0) + 0x10000);
						}
						break;
					}

				default:
						break;
			}
			olds_cmd3 = ((data >> 4) + 1) & 0x3;
		}
		else if (kb_cmd == 4)
			kb_ptr = data;
		else if(kb_cmd == 0x20)
		  kb_ptr++;
	}
}

static UINT16 __fastcall olds_mainram_read_word(UINT32 address)
{
	if (SekGetPC(-1) >= 0x100000 && address != 0x8178d8) SekWriteWord(0x8178f4, SekReadWord(0x8178D8));

	return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(PGM68KRAM + (address & 0x1fffe))));
}

static UINT8 __fastcall olds_mainram_read_byte(UINT32 address)
{
	return PGM68KRAM[(address & 0x1ffff)^1];
}

static void reset_olds()
{
	olds_bs = olds_cmd3 = kb_cmd = ptr = rego = 0;
	memcpy (PGMUSER0, PGMUSER0 + 0x10000, 0x04000);
}

void install_protection_asic25_asic28_olds()
{
	pPgmScanCallback = oldsScan;
	pPgmResetCallback = reset_olds;

	sharedprotram = (UINT16*)PGMUSER0;

	// no protection rom and different offset for olds100a
	if (strcmp(BurnDrvGetTextA(DRV_NAME), "olds100a") == 0) {
		BurnLoadRom(PGMUSER0 + 0x10000,	16, 1);
	} else {
		BurnLoadRom(PGMUSER0 + 0x10000,	20, 1);
		BurnLoadRom(PGMUSER0 + 0x20000,	19, 1);
		BurnByteswap(PGMUSER0 + 0x20000, 0x10000);

		// copy in some 68k code from protection rom
		memcpy (PGMUSER0 + 0x10200, PGMUSER0 + 0x20300, 0x6B4);
	}

	{
		UINT16 *gptr = (UINT16*)(PGMUSER0 + 0x10000);

		for(INT32 i = 0; i < 0x4000 / 2; i++) {
			if (gptr[i] == (0xffff - i)) gptr[i] = BURN_ENDIAN_SWAP_INT16(0x4e75);
		}
	}

	SekOpen(0);

	SekMapMemory(PGMUSER0,	0x400000, 0x403fff, SM_RAM);

	SekMapHandler(4,	0xdcb400, 0xdcb403, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, olds_protection_read);
	SekSetWriteWordHandler(4, olds_protection_write);

	SekMapHandler(5,	0x8178f4, 0x8178f5, SM_READ | SM_FETCH);
	SekSetReadWordHandler(5, olds_mainram_read_word);
	SekSetReadByteHandler(5, olds_mainram_read_byte);

	SekClose();
}

//-------------------------------------------------------------------------------------------
// Oriental Legends Special Plus! (Creamy Mami)

static UINT16 m_oldsplus_key;
static UINT16 m_oldsplus_int[2];
static UINT32 m_oldsplus_val;
static UINT32 m_oldsplus_regs[0x100];
static UINT32 m_oldsplus_ram[0x100];

static const INT32 oldsplus_80[0x5]={
	0xbb8,0x1770,0x2328,0x2ee0,0xf4240
};

static const UINT8 oldsplus_fc[0x20]={
	0x00,0x00,0x0a,0x3a,0x4e,0x2e,0x03,0x40,
	0x33,0x43,0x26,0x2c,0x00,0x00,0x00,0x00,
	0x00,0x00,0x44,0x4d,0x0b,0x27,0x3d,0x0f,
	0x37,0x2b,0x02,0x2f,0x15,0x45,0x0e,0x30
};

static const UINT16 oldsplus_a0[0x20]={
	0x000,0x023,0x046,0x069,0x08c,0x0af,0x0d2,0x0f5,
	0x118,0x13b,0x15e,0x181,0x1a4,0x1c7,0x1ea,0x20d,
	0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,
	0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,0x20d,
};

static const UINT16 oldsplus_90[0x7]={
	0x50,0xa0,0xc8,0xf0,0x190,0x1f4,0x258
};

static const UINT8 oldsplus_5e[0x20]={
	0x04,0x04,0x04,0x04,0x04,0x03,0x03,0x03,
	0x02,0x02,0x02,0x01,0x01,0x01,0x01,0x01,
	0x01,0x01,0x01,0x01,0x01,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
};

static const UINT8 oldsplus_b0[0xe0]={
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
	0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,

	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
	0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,

	0x00,0x00,0x00,0x00,0x01,0x02,0x03,0x04,
	0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,
	0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,0x14,
	0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,0x1c,

	0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,
	0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,
	0x10,0x11,0x12,0x13,0x14,0x15,0x16,0x17,
	0x18,0x19,0x1a,0x1b,0x1c,0x1d,0x1e,0x1f,

	0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,
	0x0c,0x0d,0x0e,0x0f,0x10,0x11,0x12,0x13,
	0x14,0x15,0x16,0x17,0x18,0x19,0x1a,0x1b,
	0x1c,0x1d,0x1e,0x1f,0x1f,0x1f,0x1f,0x1f
};

static const UINT8 oldsplus_ae[0xe0]={
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,

	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,
	0x50,0x50,0x50,0x50,0x50,0x50,0x50,0x50,

	0x1E,0x1F,0x20,0x21,0x22,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,

	0x1F,0x20,0x21,0x22,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,

	0x20,0x21,0x22,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,

	0x21,0x22,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,

	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23,
	0x23,0x23,0x23,0x23,0x23,0x23,0x23,0x23
};

static const UINT16 oldsplus_ba[0x4]={
	0x3138,0x2328,0x1C20,0x1518
};

static const UINT16 oldsplus_9d[0x111]={
	0x0000,0x0064,0x00c8,0x012c,0x0190,0x01f4,0x0258,0x02bc,
	0x02f8,0x0334,0x0370,0x03ac,0x03e8,0x0424,0x0460,0x049c,
	0x04d8,0x0514,0x0550,0x058c,0x05c8,0x0604,0x0640,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x0000,
	0x0064,0x00c8,0x012c,0x0190,0x01f4,0x0258,0x02bc,0x0302,
	0x0348,0x038e,0x03d4,0x041a,0x0460,0x04a6,0x04ec,0x0532,
	0x0578,0x05be,0x0604,0x064a,0x0690,0x06d6,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x0000,0x0064,
	0x00c8,0x012c,0x0190,0x01f4,0x0258,0x02bc,0x0316,0x0370,
	0x03ca,0x0424,0x047e,0x04d8,0x0532,0x058c,0x05e6,0x0640,
	0x069a,0x06f4,0x074e,0x07a8,0x0802,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x0000,0x0064,0x00c8,
	0x012c,0x0190,0x01f4,0x0258,0x02bc,0x032a,0x0398,0x0406,
	0x0474,0x04e2,0x0550,0x05be,0x062c,0x069a,0x0708,0x0776,
	0x07e4,0x0852,0x08c0,0x092e,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x0000,0x0064,0x00c8,0x012c,
	0x0190,0x01f4,0x0258,0x02bc,0x0348,0x03d4,0x0460,0x04ec,
	0x0578,0x0604,0x0690,0x071c,0x07a8,0x0834,0x08c0,0x094c,
	0x09d8,0x0a64,0x0af0,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x0000,0x0064,0x00c8,0x012c,0x0190,
	0x01f4,0x0258,0x02bc,0x0384,0x044c,0x0514,0x05dc,0x06a4,
	0x076c,0x0834,0x08fc,0x09c4,0x0a8c,0x0b54,0x0c1c,0x0ce4,
	0x0dac,0x0e74,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x0000,0x0064,0x00c8,0x012c,0x0190,0x01f4,
	0x0258,0x02bc,0x030c,0x035c,0x03ac,0x03fc,0x044c,0x049c,
	0x04ec,0x053c,0x058c,0x05dc,0x062c,0x067c,0x06cc,0x071c,
	0x076c,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,0x06bc,
	0x06bc
};

static const UINT16 oldsplus_8c[0x20]={
	0x0032,0x0032,0x0064,0x0096,0x0096,0x00fa,0x012c,0x015e,
	0x0032,0x0064,0x0096,0x00c8,0x00c8,0x012c,0x015e,0x0190,
	0x0064,0x0096,0x00c8,0x00fa,0x00fa,0x015e,0x0190,0x01c2,
	0x0096,0x00c8,0x00fa,0x012c,0x012c,0x0190,0x01c2,0x01f4
};

UINT16 __fastcall oldsplus_prot_read(UINT32 address)
{
	if (address == 0x500000)
	{
		UINT16 d = m_oldsplus_val & 0xffff;
		UINT16 realkey = m_oldsplus_key >> 8;
		realkey |= m_oldsplus_key;
		d ^= realkey;
		return d;
	}
	else if (address == 0x500002)
	{
		UINT16 d = m_oldsplus_val >> 16;
		UINT16 realkey = m_oldsplus_key >> 8;
		realkey |= m_oldsplus_key;
		d ^= realkey;
		return d;

	}
	return 0xff;
}

void __fastcall oldsplus_prot_write(UINT32 address, UINT16 data)
{
	if (address == 0x500000)
	{
		m_oldsplus_int[0] = data;
		return;
	}

	if (address == 0x500002)
	{
		UINT16 realkey;
		if ((data >> 8) == 0xff) m_oldsplus_key = 0xff00;
		realkey = m_oldsplus_key >> 8;
		realkey |= m_oldsplus_key;
		{
			m_oldsplus_key += 0x100;
			m_oldsplus_key &= 0xff00;
			if (m_oldsplus_key == 0xff00) m_oldsplus_key = 0x100;
		}
		data ^= realkey;
		m_oldsplus_int[1] = data;
		m_oldsplus_int[0] ^= realkey;

		switch (m_oldsplus_int[1] & 0xff)
		{
			case 0x88:
				m_oldsplus_key = 0x100;
				m_oldsplus_val = 0x990000;
				break;

			case 0xd0:
				m_oldsplus_val = 0xa01000 + (m_oldsplus_int[0] << 5);
				break;

			case 0xc0:
				m_oldsplus_val = 0xa00000 + (m_oldsplus_int[0] << 6);
				break;

			case 0xc3:
				m_oldsplus_val = 0xa00800 + (m_oldsplus_int[0] << 6);
				break;

			case 0x36:
				m_oldsplus_ram[0x36] = m_oldsplus_int[0];
				m_oldsplus_val = 0x990000;
				break;

			case 0x33:
				m_oldsplus_ram[0x33] = m_oldsplus_int[0];
				m_oldsplus_val = 0x990000;
				break;

			case 0x35:
				m_oldsplus_ram[0x36] += m_oldsplus_int[0];
				m_oldsplus_val = 0x990000;
				break;

			case 0x37:
				m_oldsplus_ram[0x33] += m_oldsplus_int[0];
				m_oldsplus_val = 0x990000;
				break;

			case 0x34:
				m_oldsplus_val = m_oldsplus_ram[0x36];
				break;

			case 0x38:
				m_oldsplus_val = m_oldsplus_ram[0x33];
				break;

			case 0x80:
				m_oldsplus_val = oldsplus_80[m_oldsplus_int[0]];
				break;

			case 0xe7:
				m_oldsplus_ram[0xe7] = m_oldsplus_int[0];
				m_oldsplus_val = 0x990000;
				break;

			case 0xe5:
				switch (m_oldsplus_ram[0xe7])
				{
					case 0xb000:
						m_oldsplus_regs[0xb] = m_oldsplus_int[0];
						m_oldsplus_regs[0xc] = 0;
						break;

					case 0xc000:
						m_oldsplus_regs[0xc] = m_oldsplus_int[0];
						break;

					case 0xd000:
						m_oldsplus_regs[0xd] = m_oldsplus_int[0];
						break;

					case 0xf000:
						m_oldsplus_regs[0xf] = m_oldsplus_int[0];
						break;
				}
				m_oldsplus_val = 0x990000;
				break;

			case 0xf8:
				m_oldsplus_val = m_oldsplus_regs[m_oldsplus_int[0]];
				break;

			case 0xfc:
				m_oldsplus_val = oldsplus_fc[m_oldsplus_int[0]];
				break;

			case 0xc5:
				m_oldsplus_regs[0xd] --;
				m_oldsplus_val = 0x990000;
				break;

			case 0xd6:
				m_oldsplus_regs[0xb] ++;
				m_oldsplus_val = 0x990000;
				break;

			case 0x3a:
				m_oldsplus_regs[0xf] = 0;
				m_oldsplus_val = 0x990000;
				break;

			case 0xf0:
				m_oldsplus_ram[0xf0] = m_oldsplus_int[0];
				m_oldsplus_val = 0x990000;
				break;

			case 0xed:
				m_oldsplus_val = m_oldsplus_int[0] << 0x6;
				m_oldsplus_val += m_oldsplus_ram[0xf0];
				m_oldsplus_val = m_oldsplus_val << 0x2;
				m_oldsplus_val += 0x900000;
				break;

			case 0xe0:
				m_oldsplus_ram[0xe0] = m_oldsplus_int[0];
				m_oldsplus_val = 0x990000;
				break;

			case 0xdc:
				m_oldsplus_val = m_oldsplus_int[0] << 0x6;
				m_oldsplus_val += m_oldsplus_ram[0xe0];
				m_oldsplus_val = m_oldsplus_val << 2;
				m_oldsplus_val += 0x904000;
				break;

			case 0xcb:
				m_oldsplus_val =  0xc000;
				break;

			case 0xa0:
				m_oldsplus_val = oldsplus_a0[m_oldsplus_int[0]];
				break;

			case 0xba:
				m_oldsplus_val = oldsplus_ba[m_oldsplus_int[0]];
				break;

			case 0x5e:
				m_oldsplus_val = oldsplus_5e[m_oldsplus_int[0]];
				break;

			case 0xb0:
				m_oldsplus_val = oldsplus_b0[m_oldsplus_int[0]];
				break;

			case 0xae:
				m_oldsplus_val = oldsplus_ae[m_oldsplus_int[0]];
				break;

 			case 0x9d:
				m_oldsplus_val = oldsplus_9d[m_oldsplus_int[0]];
				break;

			case 0x90:
				m_oldsplus_val = oldsplus_90[m_oldsplus_int[0]];
				break;

			case 0x8c:
				m_oldsplus_val = oldsplus_8c[m_oldsplus_int[0]];
				break;

			default:
				m_oldsplus_val = 0x990000;
				break;
		}
	}
}

static void reset_asic27a_oldsplus()
{
	m_oldsplus_key = 0;
	m_oldsplus_val = 0;
	memset (m_oldsplus_int,  0, sizeof(INT16) * 2);
	memset (m_oldsplus_regs, 0, sizeof(INT32) * 0x100);
	memset (m_oldsplus_ram,  0, sizeof(INT32) * 0x100);

	memset (PGMUSER0, 0, 0x400);

	*((UINT16*)(PGMUSER0 + 0x00008)) = PgmInput[7]; // region
}

void install_protection_asic27a_oldsplus()
{
	pPgmScanCallback = oldsplus_asic27aScan;
	pPgmResetCallback = reset_asic27a_oldsplus;

	SekOpen(0);
	SekMapMemory(PGMUSER0,	0x4f0000, 0x4f003f | 0x3ff, SM_READ); // ram

	SekMapHandler(4,	0x500000, 0x500003, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, oldsplus_prot_read);
	SekSetWriteWordHandler(4, oldsplus_prot_write);
	SekClose();
}



// add this to the bottom of pgm_prot.cpp (i added it after the oldsScan function)


INT32 oldsplus_asic27aScan(INT32 nAction, INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= (UINT8*)m_oldsplus_ram;
		ba.nLen		= 0x0000100 * sizeof(INT32);
		ba.nAddress	= 0xfffc00;
		ba.szName	= "Prot RAM";
		BurnAcb(&ba);

		ba.Data		= (UINT8*)m_oldsplus_regs;
		ba.nLen		= 0x0000100 * sizeof(INT32);
		ba.nAddress	= 0xfff800;
		ba.szName	= "Prot REGs";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(m_oldsplus_key);
		SCAN_VAR(m_oldsplus_val);
		SCAN_VAR(m_oldsplus_int[0]);
		SCAN_VAR(m_oldsplus_int[1]);
	}

	return 0;
}

//-------------------------------------------------------------------------------------------
// S.V.G. - Spectral vs Generation / Demon Front / The Gladiator / The Killing Blade EX / Happy 6in1

static UINT8 svg_ram_sel = 0;
static UINT8 *svg_ram[2];

static void svg_set_ram_bank(INT32 data)
{
	svg_ram_sel = data & 1;
	Arm7MapMemory(svg_ram[svg_ram_sel],	0x38000000, 0x3801ffff, ARM7_RAM);
	SekMapMemory(svg_ram[svg_ram_sel^1],	0x500000, 0x51ffff, SM_FETCH);
}

static void __fastcall svg_write_byte(UINT32 address, UINT8 data)
{
	pgm_cpu_sync();

	if ((address & 0xffe0000) == 0x0500000) {
		svg_ram[svg_ram_sel^1][(address & 0x1ffff)^1] = data;
		return;
	}

	switch (address)
	{
		case 0x5c0000:
		case 0x5c0001:
			Arm7SetIRQLine(ARM7_FIRQ_LINE, ARM7_HOLD_LINE);
		return;
	}
}

static void __fastcall svg_write_word(UINT32 address, UINT16 data)
{
	pgm_cpu_sync();

	if ((address & 0xffe0000) == 0x0500000) {
		*((UINT16*)(svg_ram[svg_ram_sel^1] + (address & 0x1fffe))) = BURN_ENDIAN_SWAP_INT16(data);
		
		return;
	}

	switch (address)
	{
		case 0x5c0000:
			Arm7SetIRQLine(ARM7_FIRQ_LINE, ARM7_HOLD_LINE);
		return;

		case 0x5c0300:
			asic27a_to_arm = data;
		return;
	}
}

static UINT8 __fastcall svg_read_byte(UINT32 address)
{
	if ((address & 0xffe0000) == 0x0500000) {
		pgm_cpu_sync();

		INT32 d = svg_ram[svg_ram_sel^1][(address & 0x1ffff)^1];
		return d;
	}

	switch (address)
	{
		case 0x5c0000:
		case 0x5c0001:
			return 0;
	}

	return 0;
}

static UINT16 __fastcall svg_read_word(UINT32 address)
{
	if ((address & 0xffe0000) == 0x0500000) {
		pgm_cpu_sync();

		return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(svg_ram[svg_ram_sel^1] + (address & 0x1fffe))));
	}

	switch (address)
	{
		case 0x5c0000:
		case 0x5c0001:
			return 0;

		case 0x5c0300:
			pgm_cpu_sync();
			return asic27a_to_68k;
	}

	return 0;
}

static void svg_arm7_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x40000018:
			svg_set_ram_bank(data);
		return;

		case 0x48000000:
			asic27a_to_68k = data;
		return;
	}
}

static void svg_arm7_write_word(UINT32 /*address*/, UINT16 /*data*/)
{

}

static void svg_arm7_write_long(UINT32 address, UINT32 data)
{
	switch (address)
	{
		case 0x40000018:
			svg_set_ram_bank(data);
		return;

		case 0x48000000:
			asic27a_to_68k = data;
		return;
	}
}

static UINT8 svg_arm7_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x48000000:
		case 0x48000001:
		case 0x48000002:
		case 0x48000003:
			return asic27a_to_arm;
	}

	return 0;
}

static UINT16 svg_arm7_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x48000000:
		case 0x48000002:
			return asic27a_to_arm;
	}

	return 0;
}

static UINT32 svg_arm7_read_long(UINT32 address)
{
	switch (address)
	{
		case 0x48000000:
			return asic27a_to_arm;
	}

	return 0;
}

void install_protection_asic27a_svg()
{
	nPGMArm7Type = 3;

	pPgmScanCallback = svg_asic27aScan;

	svg_ram_sel = 0;
	svg_ram[0] = PGMARMShareRAM;
	svg_ram[1] = PGMARMShareRAM2;

	SekOpen(0);
	SekMapHandler(5,		0x500000, 0x5fffff, SM_RAM);
	SekSetReadWordHandler(5, 	svg_read_word);
	SekSetReadByteHandler(5, 	svg_read_byte);
	SekSetWriteWordHandler(5, 	svg_write_word);
	SekSetWriteByteHandler(5, 	svg_write_byte);
	SekClose();

	Arm7Init(1);
	Arm7Open(0);
	Arm7MapMemory(PGMARMROM,	0x00000000, 0x00003fff, ARM7_ROM);
	Arm7MapMemory(PGMUSER0,		0x08000000, 0x08000000 | (nPGMExternalARMLen-1), ARM7_ROM);
	Arm7MapMemory(PGMARMRAM0,	0x10000000, 0x100003ff, ARM7_RAM);
	Arm7MapMemory(PGMARMRAM1,	0x18000000, 0x1803ffff, ARM7_RAM);
	Arm7MapMemory(svg_ram[1],	0x38000000, 0x3801ffff, ARM7_RAM);
	Arm7MapMemory(PGMARMRAM2,	0x50000000, 0x500003ff, ARM7_RAM);
	Arm7SetWriteByteHandler(svg_arm7_write_byte);
	Arm7SetWriteWordHandler(svg_arm7_write_word);
	Arm7SetWriteLongHandler(svg_arm7_write_long);
	Arm7SetReadByteHandler(svg_arm7_read_byte);
	Arm7SetReadWordHandler(svg_arm7_read_word);
	Arm7SetReadLongHandler(svg_arm7_read_long);
	Arm7Close();
}


//-------------------------------------------------------------------------------------------
// ketsui / espgaluda / ddp3

static UINT16 ddp3value;
static UINT16 ddp3key;
static UINT32   ddp3response;
static UINT8  ddp3internal_slot;
static UINT32   ddp3slots[0x100];

void __fastcall ddp3_asic_write(UINT32 offset, UINT16 data)
{
	switch (offset & 0x06)
	{
		case 0: ddp3value = data; return;

		case 2:
		{
			if ((data >> 8) == 0xff) ddp3key = 0xffff;

			ddp3value   ^= ddp3key;
			ddp3response = 0x880000;

			switch ((data ^ ddp3key) & 0xff)
			{
				case 0x40:
					ddp3slots[(ddp3value >> 10) & 0x1f] = (ddp3slots[(ddp3value >> 5) & 0x1f] + ddp3slots[(ddp3value >> 0) & 0x1f]) & 0xffffff;
				break;

				case 0x67:
					ddp3internal_slot = ddp3value >> 8;
					ddp3slots[ddp3internal_slot] = (ddp3value & 0x00ff) << 16;
				break;
		
				case 0xe5:
					ddp3slots[ddp3internal_slot] |= ddp3value;
				break;
	
				case 0x8e:
					ddp3response = ddp3slots[ddp3value & 0xff];
				break;

				case 0x99: // reset?
					ddp3key = 0;
				break;
			}

			ddp3key = (ddp3key + 0x0100) & 0xff00;
			if (ddp3key == 0xff00) ddp3key = 0x0100;
			ddp3key |= ddp3key >> 8;
		}
		return;

		case 4: return;
	}
}

static UINT16 __fastcall ddp3_asic_read(UINT32 offset)
{
	switch (offset & 0x02)
	{
		case 0: return (ddp3response >>  0) ^ ddp3key;
		case 2: return (ddp3response >> 16) ^ ddp3key;
	}

	return 0;
}

static void reset_ddp3()
{
	ddp3value = 0;
	ddp3key = 0;
	ddp3response = 0;
	ddp3internal_slot = 0;

	memset (ddp3slots, 0, 0x100 * sizeof(INT32));
}

void install_protection_asic27a_ketsui()
{
	pPgmResetCallback = reset_ddp3;
	pPgmScanCallback = ddp3Scan;

	SekOpen(0);
	SekMapHandler(4,		0x400000, 0x400005, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, 	ddp3_asic_read);
	SekSetWriteWordHandler(4, 	ddp3_asic_write);
	SekClose();
}

void install_protection_asic27a_ddp3()
{
	pPgmResetCallback = reset_ddp3;
	pPgmScanCallback = ddp3Scan;

	SekOpen(0);
	SekMapHandler(4,		0x500000, 0x500005, SM_READ | SM_WRITE);
	SekSetReadWordHandler(4, 	ddp3_asic_read);
	SekSetWriteWordHandler(4, 	ddp3_asic_write);
	SekClose();
}


//-----------------------------------------------------------------------------------------------------
// Save states

INT32 kov_asic27Scan(INT32 nAction, INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= (UINT8*)kov_slots;
		ba.nLen		= 0x0000010 * sizeof(INT32);
		ba.nAddress	= 0xff0000;
		ba.szName	= "Asic Slot Registers";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(kov_internal_slot);
		SCAN_VAR(kov_key);
		SCAN_VAR(kov_response);
		SCAN_VAR(kov_value);

		SCAN_VAR(kov_c0_value);
		SCAN_VAR(kov_cb_value);
		SCAN_VAR(kov_fe_value);
	}

	return 0;
}

INT32 asic3Scan(INT32 nAction, INT32 *)
{
	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(asic3_reg);
		SCAN_VAR(asic3_latch[0]);
		SCAN_VAR(asic3_latch[1]);
		SCAN_VAR(asic3_latch[2]);
		SCAN_VAR(asic3_x);
		SCAN_VAR(asic3_y);
		SCAN_VAR(asic3_z);
		SCAN_VAR(asic3_h1);
		SCAN_VAR(asic3_h2);
		SCAN_VAR(asic3_hold);
	}

	return 0;
}

INT32 killbldScan(INT32 nAction, INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= PGMUSER0 + 0x000000;
		ba.nLen		= 0x0004000;
		ba.nAddress	= 0x300000;
		ba.szName	= "ProtRAM";
		BurnAcb(&ba);

		ba.Data		= (UINT8*)kb_regs;
		ba.nLen		= 0x00100 * sizeof(INT32);
		ba.nAddress	= 0xfffffc00;
		ba.szName	= "Protection Registers";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(kb_cmd);
		SCAN_VAR(kb_reg);
		SCAN_VAR(kb_ptr);
	}

	return 0;
}

INT32 pstarsScan(INT32 nAction, INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= (UINT8*)PSTARS_REGS;
		ba.nLen		= 0x0000010 * sizeof(INT32);
		ba.nAddress	= 0xff1000;
		ba.szName	= "Asic Register";
		BurnAcb(&ba);

		ba.Data		= (UINT8*)pstar_ram;
		ba.nLen		= 0x0000003 * sizeof(INT16);
		ba.nAddress	= 0xff2000;
		ba.szName	= "Asic RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(PSTARSKEY);
		SCAN_VAR(PSTARS_VAL);
		SCAN_VAR(PSTARSINT[0]);
		SCAN_VAR(PSTARSINT[1]);
		SCAN_VAR(pstar_e7);
		SCAN_VAR(pstar_b1);
		SCAN_VAR(pstar_ce);
	}

	return 0;
}

INT32 asic27aScan(INT32 nAction,INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= PGMARMShareRAM;
		ba.nLen		= 0x0010000;
		ba.nAddress	= 0xd00000;
		ba.szName	= "ARM SHARE RAM";
		BurnAcb(&ba);

		ba.Data		= PGMARMRAM0;
		ba.nLen		= 0x0000400;
		ba.nAddress	= 0;
		ba.szName	= "ARM RAM 0";
		BurnAcb(&ba);

		ba.Data		= PGMARMRAM1;
		ba.nLen		= 0x0010000;
		ba.nAddress	= 0;
		ba.szName	= "ARM RAM 1";
		BurnAcb(&ba);

		ba.Data		= PGMARMRAM2;
		ba.nLen		= 0x0000400;
		ba.nAddress	= 0;
		ba.szName	= "ARM RAM 2";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		Arm7Scan(nAction);

		SCAN_VAR(asic27a_to_arm);
		SCAN_VAR(asic27a_to_68k);
	}

 	return 0;
}

INT32 oldsScan(INT32 nAction, INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= PGMUSER0 + 0x000000;
		ba.nLen		= 0x0004000;
		ba.nAddress	= 0x400000;
		ba.szName	= "ProtRAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(olds_cmd3);
		SCAN_VAR(rego);
		SCAN_VAR(olds_bs);
		SCAN_VAR(ptr);
		SCAN_VAR(kb_cmd);
	}

	return 0;
}

INT32 kovsh_asic27aScan(INT32 nAction,INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= PGMARMShareRAM;
		ba.nLen		= 0x0000040;
		ba.nAddress	= 0x400000;
		ba.szName	= "ARM SHARE RAM";
		BurnAcb(&ba);

		ba.Data		= PGMARMRAM0;
		ba.nLen		= 0x0000400;
		ba.nAddress	= 0;
		ba.szName	= "ARM RAM 0";
		BurnAcb(&ba);

		ba.Data		= PGMARMRAM2;
		ba.nLen		= 0x0000400;
		ba.nAddress	= 0;
		ba.szName	= "ARM RAM 1";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		Arm7Scan(nAction);

		SCAN_VAR(kovsh_highlatch_arm_w);
		SCAN_VAR(kovsh_lowlatch_arm_w);
		SCAN_VAR(kovsh_highlatch_68k_w);
		SCAN_VAR(kovsh_lowlatch_68k_w);
		SCAN_VAR(kovsh_counter);
	}

 	return 0;
}

INT32 svg_asic27aScan(INT32 nAction,INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= PGMARMShareRAM;
		ba.nLen		= 0x0020000;
		ba.nAddress	= 0x400000;
		ba.szName	= "ARM SHARE RAM #0 (address 500000)";
		BurnAcb(&ba);

		ba.Data		= PGMARMShareRAM2;
		ba.nLen		= 0x0020000;
		ba.nAddress	= 0x500000;
		ba.szName	= "ARM SHARE RAM #1";
		BurnAcb(&ba);

		ba.Data		= PGMARMRAM0;
		ba.nLen		= 0x0000400;
		ba.nAddress	= 0;
		ba.szName	= "ARM RAM 0";
		BurnAcb(&ba);

		ba.Data		= PGMARMRAM1;
		ba.nLen		= 0x0040000;
		ba.nAddress	= 0;
		ba.szName	= "ARM RAM 1";
		BurnAcb(&ba);

		ba.Data		= PGMARMRAM2;
		ba.nLen		= 0x0000400;
		ba.nAddress	= 0;
		ba.szName	= "ARM RAM 2";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		Arm7Scan(nAction);
		SCAN_VAR(asic27a_to_arm);
		SCAN_VAR(asic27a_to_68k);

		SCAN_VAR(svg_ram_sel);
		svg_set_ram_bank(svg_ram_sel);
	}

 	return 0;
}

INT32 ddp3Scan(INT32 nAction, INT32 *)
{
	struct BurnArea ba;

	if (nAction & ACB_MEMORY_RAM) {
		ba.Data		= (UINT8*)ddp3slots;
		ba.nLen		= 0x0000100 * sizeof(INT32);
		ba.nAddress	= 0xff00000;
		ba.szName	= "ProtRAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SCAN_VAR(ddp3value);
		SCAN_VAR(ddp3key);
		SCAN_VAR(ddp3response);
		SCAN_VAR(ddp3internal_slot);
	}

	return 0;
}
