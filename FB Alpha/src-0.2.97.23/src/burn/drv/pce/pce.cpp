// FB Alpha PC-Engine / TurboGrafx 16 / SuperGrafx driver module
// Based on MESS driver by Charles MacDonald

#include "tiles_generic.h"
#include "pce.h"
#include "h6280_intf.h"
#include "vdc.h"
#include "c6280.h"
#include "bitswap.h"

/*
Notes:

	There is no CD emulation at all.
	As this driver is based on MESS emulation, compatibility *should* be the same.

	Known emulation issues - also present in MESS unless noted.
	SOUND PROBLEMS
		Bouken Danshaku Don - The Lost Sunheart (not present in mess)

	GRAPHICS PROBLEMS
		Cadash - graphics shaking

	OTHER PROBLEMS
		Niko Niko Pun - hangs in-game
		Benkei Gaiden - hangs after sunsoft logo
		Power Tennis - frozen
		Tennokoe Bank - ??
		Air Zonk / PC Denjin - Punkic Cyborgs - hangs in-game
		Hisou Kihei - Xserd: black screen
*/

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *PCECartROM;
static UINT8 *PCECartRAM;
static UINT8 *PCEUserRAM;
static UINT8 *PCECDBRAM;


static UINT32 *DrvPalette;
UINT8 PCEPaletteRecalc;

static UINT8 joystick_port_select;
static UINT8 joystick_data_select;
static UINT8 joystick_6b_select[5];

static void (*interrupt)();

static UINT16 PCEInputs[5];
UINT8 PCEReset;
UINT8 PCEJoy1[12];
UINT8 PCEJoy2[12];
UINT8 PCEJoy3[12];
UINT8 PCEJoy4[12];
UINT8 PCEJoy5[12];
UINT8 PCEDips[3];

static UINT8 system_identify;
static INT32 pce_sf2 = 0;
static INT32 pce_sf2_bank;
static UINT8 bram_locked = 1;


INT32 PceGetZipName(char** pszName, UINT32 i)
{
	static char szFilename[MAX_PATH];
	char* pszGameName = NULL;

	if (pszName == NULL) {
		return 1;
	}

	if (i == 0) {
		pszGameName = BurnDrvGetTextA(DRV_NAME);
	} else {
		pszGameName = BurnDrvGetTextA(DRV_PARENT);
	}

	if (pszGameName == NULL) {
		*pszName = NULL;
		return 1;
	}

	// remove the "pce_"
	for (UINT32 j = 0; j < strlen(pszGameName); j++) {
		szFilename[j] = pszGameName[j + 4];
	}

	*pszName = szFilename;

	return 0;
}

INT32 TgGetZipName(char** pszName, UINT32 i)
{
	static char szFilename[MAX_PATH];
	char* pszGameName = NULL;

	if (pszName == NULL) {
		return 1;
	}

	if (i == 0) {
		pszGameName = BurnDrvGetTextA(DRV_NAME);
	} else {
		pszGameName = BurnDrvGetTextA(DRV_PARENT);
	}

	if (pszGameName == NULL) {
		*pszName = NULL;
		return 1;
	}

	// remove the "tg_"
	for (UINT32 j = 0; j < strlen(pszGameName); j++) {
		szFilename[j] = pszGameName[j + 3];
	}

	*pszName = szFilename;

	return 0;
}

INT32 SgxGetZipName(char** pszName, UINT32 i)
{
	static char szFilename[MAX_PATH];
	char* pszGameName = NULL;

	if (pszName == NULL) {
		return 1;
	}

	if (i == 0) {
		pszGameName = BurnDrvGetTextA(DRV_NAME);
	} else {
		pszGameName = BurnDrvGetTextA(DRV_PARENT);
	}

	if (pszGameName == NULL) {
		*pszName = NULL;
		return 1;
	}

	// remove the "sgx_"
	for (UINT32 j = 0; j < strlen(pszGameName); j++) {
		szFilename[j] = pszGameName[j + 4];
	}

	*pszName = szFilename;

	return 0;
}

static void sf2_bankswitch(UINT8 offset)
{
	pce_sf2_bank = offset;

	h6280MapMemory(PCECartROM + (offset * 0x80000) + 0x080000, 0x080000, 0x0fffff, H6280_ROM);
}

static void pce_write(UINT32 address, UINT8 data)
{
	address &= 0x1fffff;

	if ((address & 0x1ffff0) == 0x001ff0) {
		if (pce_sf2) sf2_bankswitch(address & 3);
		return;
	}
	
	switch (address & ~0x3ff)
	{
		case 0x1fe000:
			vdc_write(0, address, data);
		return;

		case 0x1fe400:
			vce_write(address, data);
		return;

		case 0x1fe800:
			c6280_write(address, data);
		return;

		case 0x1fec00:
			h6280_timer_w(address & 0x3ff, data);
		return;

		case 0x1ff000:
		{
			h6280io_set_buffer(data);

			INT32 type = (PCEDips[1] << 8) | (PCEDips[0] << 0);

			type = type;

			if (joystick_data_select == 0 && (data & 0x01)) {
				joystick_port_select = (joystick_port_select + 1) & 0x07;
			}

			joystick_data_select = data & 0x01;

			if (data & 0x02) {
				joystick_port_select = 0;

				for (int i = 0; i < 5; i++) {
					if (((type >> (i*2)) & 3) == 2) {
						joystick_6b_select[i] ^= 1;
					}
				}
			}
		}
		return;

		case 0x1ff400:
			h6280_irq_status_w(address & 0x3ff, data);
		return;

		case 0x1ff800:
		
			switch( address & 0xf )
			{
			case 0x07:	/* BRAM unlock / CD status */
				if ( data & 0x80 )
				{
					bram_locked = 0;
				}
				break;
			}
			bprintf(0,_T("CD write %x:%x\n"), address, data );
			// cd system
		return;
	}
	
	if ((address >= 0x1ee000) && (address <= 0x1ee7ff)) {
//			bprintf(0,_T("bram write %x:%x\n"), address & 0x7ff, data );
			if (!bram_locked)
			{
				PCECDBRAM[address & 0x7FF] = data;
			}
			return;
	}	
	
	
	bprintf(0,_T("unknown write %x:%x\n"), address, data );
}

static UINT8 pce_read(UINT32 address)
{

	address &= 0x1fffff;
	
	switch (address & ~0x3ff)
	{

			
		case 0x1fe000:
			return vdc_read(0, address);

		case 0x1fe400:
			return vce_read(address);

		case 0x1fe800:
			return c6280_read();

		case 0x1fec00:
			return h6280_timer_r(address & 0x3ff);

		case 0x1ff000:
		{
			INT32 type = (PCEDips[1] << 8) | (PCEDips[0] << 0);
			UINT16 ret = 0;

			type = (type >> (joystick_port_select << 1)) & 0x03;

			if (joystick_port_select <= 4) {
				if (type == 0) {
					ret = PCEInputs[joystick_port_select] & 0x0ff;
				} else {
					ret = PCEInputs[joystick_port_select] & 0xfff;
					ret >>= joystick_6b_select[joystick_port_select] * 8;
				}

				if (joystick_data_select) ret >>= 4;
			} else {
				ret = 0xff;
			}

			ret &= 0x0f;
			ret |= 0x30; // ?
			ret |= 0x80; // no cd!
			ret |= system_identify; // 0x40 pce, sgx, 0x00 tg16

			return ret;
		}

		case 0x1ff400:
			return h6280_irq_status_r(address & 0x3ff);

		case 0x1ff800:
			switch( address & 0xf )
			{
				case 0x03:	/* BRAM lock / CD status */
					bram_locked = 1;
					break;
			}
			bprintf(0,_T("CD read %x\n"), address );
			return 0; // cd system
	}
	
	if ((address >= 0x1ee000) && (address <= 0x1ee7ff)) {
		//	bprintf(0,_T("bram read %x:%x\n"), address,address & 0x7ff );
			return PCECDBRAM[address & 0x7ff];
	}	
		
	bprintf(0,_T("Unknown read %x\n"), address );
	return 0;
}

static UINT8 sgx_read(UINT32 address)
{
	address &= 0x1fffff;

	switch (address & ~0x3e7)
	{
		case 0x1fe000:
			return vdc_read(0, address & 0x07);

		case 0x1fe008:
			return vpc_read(address & 0x07);

		case 0x1fe010:
			return vdc_read(1, address & 0x07);
	}

	return pce_read(address);
}

static void sgx_write(UINT32 address, UINT8 data)
{
	address &= 0x1fffff;

	switch (address & ~0x3e7)
	{
		case 0x1fe000:
			vdc_write(0, address & 0x07, data);
		return;

		case 0x1fe008:
			vpc_write(address & 0x07, data);
		return;

		case 0x1fe010:
			vdc_write(1, address & 0x07, data);
		return;
	}

	pce_write(address, data);
}

static void pce_write_port(UINT8 port, UINT8 data)
{
	if (port < 4) {
		vdc_write(0, port, data);
	}
}

static void sgx_write_port(UINT8 port, UINT8 data)
{
	if (port < 4) {
		sgx_vdc_write(port, data);
	}
}

static INT32 MemIndex(UINT32 cart_size, INT32 type)
{
	UINT8 *Next; Next = AllMem;

	PCECartROM	= Next; Next += (cart_size <= 0x100000) ? 0x100000 : cart_size;

	DrvPalette	= (UINT32*)Next; Next += 0x0401 * sizeof(UINT32);

	AllRam		= Next;

	PCEUserRAM	= Next; Next += (type == 2) ? 0x008000 : 0x002000; // pce/tg16 0x2000, sgx 0x8000

	PCECartRAM	= Next; Next += 0x008000; // populous
	PCECDBRAM = Next; Next += 0x00800; // Bram thingy
	vce_data	= (UINT16*)Next; Next += 0x200 * sizeof(UINT16);

	vdc_vidram[0]	= Next; Next += 0x010000;
	vdc_vidram[1]	= Next; Next += 0x010000; // sgx

	RamEnd		= Next;

	vdc_tmp_draw	= (UINT16*)Next; Next += 684 * 262 * sizeof(UINT16);

	MemEnd		= Next;

	return 0;
}

static INT32 PCEDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	h6280Open(0);
	h6280Reset();
	h6280Close();

	vdc_reset();
	vce_reset();
	vpc_reset();

	c6280_reset();

	memset (joystick_6b_select, 0, 5);
	joystick_port_select = 0;
	joystick_data_select = 0;

	pce_sf2_bank = 0;

	return 0;
}

static INT32 CommonInit(int type)
{
	struct BurnRomInfo ri;
	BurnDrvGetRomInfo(&ri, 0);
	UINT32 length = ri.nLen;

	AllMem = NULL;
	MemIndex(length, type);
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex(length, type);

	{
		memset (PCECartROM, 0xff, length);

		if (BurnLoadRom(PCECartROM, 0, 1)) return 1;

		if (ri.nLen & 0x200) { // remove header
			memcpy (PCECartROM, PCECartROM + 0x200, ri.nLen - 0x200);
			length -= 0x200;
		}

		if (PCECartROM[0x1fff] < 0xe0) { // decrypt japanese card
			for (UINT32 i = 0; i < length; i++) {
				PCECartROM[i] = BITSWAP08(PCECartROM[i], 0,1,2,3,4,5,6,7);
			}
		}

		if (length == 0x280000) pce_sf2 = 1;

		if (length == 0x60000)
		{
			memcpy (PCECartROM + 0x60000, PCECartROM + 0x40000, 0x20000);
			memcpy (PCECartROM + 0x80000, PCECartROM + 0x40000, 0x40000);
			memcpy (PCECartROM + 0xc0000, PCECartROM + 0x40000, 0x40000);
			memcpy (PCECartROM + 0x40000, PCECartROM + 0x00000, 0x40000);
		}
		else
		{
			if (length <= 0x40000)
			{
				memcpy (PCECartROM + 0x40000, PCECartROM + 0x00000, 0x40000);
			}
	
			if (length <= 0x80000)
			{
				memcpy (PCECartROM + 0x80000, PCECartROM + 0x00000, 0x80000);
			}
		}
	}

	if (type == 0 || type == 1) // pce / tg-16
	{
		h6280Init(0);
		h6280Open(0);
		h6280MapMemory(PCECartROM + 0x000000, 0x000000, 0x0fffff, H6280_ROM);
		h6280MapMemory(PCEUserRAM + 0x000000, 0x1f0000, 0x1f1fff, H6280_RAM); // mirrored
		h6280MapMemory(PCEUserRAM + 0x000000, 0x1f2000, 0x1f3fff, H6280_RAM);
		h6280MapMemory(PCEUserRAM + 0x000000, 0x1f4000, 0x1f5fff, H6280_RAM);
		h6280MapMemory(PCEUserRAM + 0x000000, 0x1f6000, 0x1f7fff, H6280_RAM);
		h6280SetWritePortHandler(pce_write_port);
		h6280SetWriteHandler(pce_write);
		h6280SetReadHandler(pce_read);
		h6280Close();

		interrupt = pce_interrupt;

		if (type == 0) {		// pce
			system_identify = 0x40;
		} else {			// tg16
			system_identify = 0x00;
		}
	}
	else if (type == 2) // sgx
	{
		h6280Init(0);
		h6280Open(0);
		h6280MapMemory(PCECartROM, 0x000000, 0x0fffff, H6280_ROM);
		h6280MapMemory(PCEUserRAM, 0x1f0000, 0x1f7fff, H6280_RAM);
		h6280SetWritePortHandler(sgx_write_port);
		h6280SetWriteHandler(sgx_write);
		h6280SetReadHandler(sgx_read);
		h6280Close();

		interrupt = sgx_interrupt;
		system_identify = 0x40;
	}
	
	bram_locked = 1;
	
	vce_palette_init(DrvPalette);

	c6280_init(3579545, 0);
	c6280_set_route(BURN_SND_C6280_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	c6280_set_route(BURN_SND_C6280_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	PCEDoReset();

	return 0;
}

INT32 PCEInit()
{
//	bprintf (0, _T("booting PCE!\n"));
	return CommonInit(0);
}

INT32 TG16Init()
{
//	bprintf (0, _T("booting TG16!\n"));
	return CommonInit(1);
}

INT32 SGXInit()
{
//	bprintf (0, _T("booting SGX!\n"));
	return CommonInit(2);
}

INT32 populousInit()
{
	int nRet = PCEInit();

	if (nRet == 0) {
		h6280Open(0);
		h6280MapMemory(PCECartRAM, 0x080000, 0x087fff, H6280_RAM);
		h6280Close();
	}

	return nRet;
}

INT32 PCEExit()
{
	GenericTilesExit();

	c6280_exit();
	// video exit

	h6280Exit();

	BurnFree (AllMem);

	pce_sf2 = 0;

	return 0;
}

INT32 PCEDraw()
{
	if (PCEPaletteRecalc) {
		vce_palette_init(DrvPalette);
		PCEPaletteRecalc = 0;
	}

	{
		UINT16 *src = vdc_tmp_draw + (14 * 684) + 86;
		UINT16 *dst = pTransDraw;
	
		for (INT32 y = 0; y < nScreenHeight; y++) {
			for (INT32 x = 0; x < nScreenWidth; x++) {
				dst[x] = src[x];
			}
			dst += nScreenWidth;
			src += 684;
		}
	}

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void PCECompileInputs()
{
	memset (PCEInputs, 0xff, 5 * sizeof(UINT16));

	for (INT32 i = 0; i < 12; i++) {
		PCEInputs[0] ^= (PCEJoy1[i] & 1) << i;
		PCEInputs[1] ^= (PCEJoy2[i] & 1) << i;
		PCEInputs[2] ^= (PCEJoy3[i] & 1) << i;
		PCEInputs[3] ^= (PCEJoy4[i] & 1) << i;
		PCEInputs[4] ^= (PCEJoy5[i] & 1) << i;
	}
}

INT32 PCEFrame()
{
	if (PCEReset) {
		PCEDoReset();
	}

	h6280NewFrame(); // needed for c6280

	PCECompileInputs();

	INT32 nCyclesTotal = (INT32)((INT64)7159090 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	
	h6280Open(0);
	
	for (INT32 i = 0; i < 262; i++)
	{
		h6280Run(nCyclesTotal / 262);
		interrupt();
	}
	
	if (pBurnSoundOut) {
		c6280_update(pBurnSoundOut, nBurnSoundLen);
	}

	h6280Close();

	if (pBurnDraw) {
		PCEDraw();
	}

	return 0;
}

INT32 PCEScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
		*pnMin = 0x029698;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		h6280CpuScan(nAction);

		vdc_scan(nAction, pnMin);
		c6280_scan(nAction, pnMin);

		SCAN_VAR(joystick_port_select);
		SCAN_VAR(joystick_data_select);
		SCAN_VAR(joystick_6b_select[0]);
		SCAN_VAR(joystick_6b_select[1]);
		SCAN_VAR(joystick_6b_select[2]);
		SCAN_VAR(joystick_6b_select[3]);
		SCAN_VAR(joystick_6b_select[4]);
		SCAN_VAR(bram_locked);

		if (pce_sf2) {
			SCAN_VAR(pce_sf2_bank);
			sf2_bankswitch(pce_sf2_bank);
		}
	}

	return 0;
}
