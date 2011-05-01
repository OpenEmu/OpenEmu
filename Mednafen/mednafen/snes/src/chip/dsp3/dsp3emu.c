#ifdef DSP3_CPP

//DSP-3 emulator code
//Copyright (c) 2003-2006 John Weidman, Kris Bleakley, Lancer, z80 gaiden

uint16 DSP3_DataROM[1024] = {
	0x8000, 0x4000, 0x2000, 0x1000, 0x0800, 0x0400, 0x0200, 0x0100,
	0x0080, 0x0040, 0x0020, 0x0010, 0x0008, 0x0004, 0x0002, 0x0001,
	0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080, 0x0100,
	0x0000, 0x000f, 0x0400, 0x0200, 0x0140, 0x0400, 0x0200, 0x0040,
	0x007d, 0x007e, 0x007e, 0x007b, 0x007c, 0x007d, 0x007b, 0x007c,
	0x0002, 0x0020, 0x0030, 0x0000, 0x000d, 0x0019, 0x0026, 0x0032,
	0x003e, 0x004a, 0x0056, 0x0062, 0x006d, 0x0079, 0x0084, 0x008e,
	0x0098, 0x00a2, 0x00ac, 0x00b5, 0x00be, 0x00c6, 0x00ce, 0x00d5,
	0x00dc, 0x00e2, 0x00e7, 0x00ec, 0x00f1, 0x00f5, 0x00f8, 0x00fb,
	0x00fd, 0x00ff, 0x0100, 0x0100, 0x0100, 0x00ff, 0x00fd, 0x00fb,
	0x00f8, 0x00f5, 0x00f1, 0x00ed, 0x00e7, 0x00e2, 0x00dc, 0x00d5,
	0x00ce, 0x00c6, 0x00be, 0x00b5, 0x00ac, 0x00a2, 0x0099, 0x008e,
	0x0084, 0x0079, 0x006e, 0x0062, 0x0056, 0x004a, 0x003e, 0x0032,
	0x0026, 0x0019, 0x000d, 0x0000, 0xfff3, 0xffe7, 0xffdb, 0xffce,
	0xffc2, 0xffb6, 0xffaa, 0xff9e, 0xff93, 0xff87, 0xff7d, 0xff72,
	0xff68, 0xff5e, 0xff54, 0xff4b, 0xff42, 0xff3a, 0xff32, 0xff2b,
	0xff25, 0xff1e, 0xff19, 0xff14, 0xff0f, 0xff0b, 0xff08, 0xff05,
	0xff03, 0xff01, 0xff00, 0xff00, 0xff00, 0xff01, 0xff03, 0xff05,
	0xff08, 0xff0b, 0xff0f, 0xff13, 0xff18, 0xff1e, 0xff24, 0xff2b,
	0xff32, 0xff3a, 0xff42, 0xff4b, 0xff54, 0xff5d, 0xff67, 0xff72,
	0xff7c, 0xff87, 0xff92, 0xff9e, 0xffa9, 0xffb5, 0xffc2, 0xffce,
	0xffda, 0xffe7, 0xfff3, 0x002b, 0x007f, 0x0020, 0x00ff, 0xff00,
	0xffbe, 0x0000, 0x0044, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffc1, 0x0001, 0x0002, 0x0045,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc5, 0x0003, 0x0004, 0x0005, 0x0047, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffca, 0x0006, 0x0007, 0x0008,
	0x0009, 0x004a, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffd0, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0x004e, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd7, 0x000f, 0x0010, 0x0011,
	0x0012, 0x0013, 0x0014, 0x0053, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffdf, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b,
	0x0059, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffe8, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0021, 0x0022, 0x0023, 0x0060, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xfff2, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a,
	0x002b, 0x002c, 0x0068, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfffd, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0x0071,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc7, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d,
	0x003e, 0x003f, 0x0040, 0x0041, 0x007b, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd4, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x0044, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffe2, 0x000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012,
	0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0x0050, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfff1, 0x0019, 0x001a, 0x001b,
	0x001c, 0x001d, 0x001e, 0x001f, 0x0020, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x005d, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffcb, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d,
	0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
	0x006b, 0x0000, 0x0000, 0x0000, 0xffdc, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 0x0044, 0x0000, 0x0000,
	0xffee, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016,
	0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0054, 0x0000, 0xffee, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b,
	0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0065,
	0xffbe, 0x0000, 0xfeac, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffc1, 0x0001, 0x0002, 0xfead,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc5, 0x0003, 0x0004, 0x0005, 0xfeaf, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffca, 0x0006, 0x0007, 0x0008,
	0x0009, 0xfeb2, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffd0, 0x000a, 0x000b, 0x000c, 0x000d, 0x000e, 0xfeb6, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd7, 0x000f, 0x0010, 0x0011,
	0x0012, 0x0013, 0x0014, 0xfebb, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffdf, 0x0015, 0x0016, 0x0017, 0x0018, 0x0019, 0x001a, 0x001b,
	0xfec1, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffe8, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0x0021, 0x0022, 0x0023, 0xfec8, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xfff2, 0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a,
	0x002b, 0x002c, 0xfed0, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfffd, 0x002d, 0x002e, 0x002f,
	0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035, 0x0036, 0xfed9,
	0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffc7, 0x0037, 0x0038, 0x0039, 0x003a, 0x003b, 0x003c, 0x003d,
	0x003e, 0x003f, 0x0040, 0x0041, 0xfee3, 0x0000, 0x0000, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xffd4, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0xfeac, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffe2, 0x000c, 0x000d, 0x000e, 0x000f, 0x0010, 0x0011, 0x0012,
	0x0013, 0x0014, 0x0015, 0x0016, 0x0017, 0x0018, 0xfeb8, 0x0000,
	0x0000, 0x0000, 0x0000, 0x0000, 0xfff1, 0x0019, 0x001a, 0x001b,
	0x001c, 0x001d, 0x001e, 0x001f, 0x0020, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0xfec5, 0x0000, 0x0000, 0x0000, 0x0000,
	0xffcb, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b, 0x002c, 0x002d,
	0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
	0xfed3, 0x0000, 0x0000, 0x0000, 0xffdc, 0x0000, 0x0001, 0x0002,
	0x0003, 0x0004, 0x0005, 0x0006, 0x0007, 0x0008, 0x0009, 0x000a,
	0x000b, 0x000c, 0x000d, 0x000e, 0x000f, 0xfeac, 0x0000, 0x0000,
	0xffee, 0x0010, 0x0011, 0x0012, 0x0013, 0x0014, 0x0015, 0x0016,
	0x0017, 0x0018, 0x0019, 0x001a, 0x001b, 0x001c, 0x001d, 0x001e,
	0x001f, 0x0020, 0xfebc, 0x0000, 0xffee, 0x0021, 0x0022, 0x0023,
	0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002a, 0x002b,
	0x002c, 0x002d, 0x002e, 0x002f, 0x0030, 0x0031, 0x0032, 0xfecd,
	0x0154, 0x0218, 0x0110, 0x00b0, 0x00cc, 0x00b0, 0x0088, 0x00b0,
	0x0044, 0x00b0, 0x0000, 0x00b0, 0x00fe, 0xff07, 0x0002, 0x00ff,
	0x00f8, 0x0007, 0x00fe, 0x00ee, 0x07ff, 0x0200, 0x00ef, 0xf800,
	0x0700, 0x00ee, 0xffff, 0xffff, 0xffff, 0x0000, 0x0000, 0x0001,
	0x0001, 0x0001, 0x0001, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff,
	0xffff, 0x0000, 0x0000, 0x0001, 0x0001, 0x0001, 0x0001, 0x0000,
	0x0000, 0xffff, 0xffff, 0x0000, 0xffff, 0x0001, 0x0000, 0x0001,
	0x0001, 0x0000, 0x0000, 0xffff, 0xffff, 0xffff, 0xffff, 0x0000,
	0xffff, 0x0001, 0x0000, 0x0001, 0x0001, 0x0000, 0x0000, 0xffff,
	0xffff, 0xffff, 0x0000, 0x0000, 0x0000, 0x0044, 0x0088, 0x00cc,
	0x0110, 0x0154, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff,
	0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff, 0xffff
};

void (*SetDSP3)();
void DSP3_Command();

uint16 DSP3_DR;
uint16 DSP3_SR;
uint16 DSP3_MemoryIndex;

void DSP3_Reset()
{
  DSP3_DR = 0x0080;
  DSP3_SR = 0x0084;
  SetDSP3 = &DSP3_Command;
}

void DSP3_MemorySize()
{
	DSP3_DR = 0x0300;
	SetDSP3 = &DSP3_Reset;
}

void DSP3_TestMemory()
{
	DSP3_DR = 0x0000;
	SetDSP3 = &DSP3_Reset;
}

void DSP3_DumpDataROM()
{
	DSP3_DR = DSP3_DataROM[DSP3_MemoryIndex++];
	if (DSP3_MemoryIndex == 1024)
		SetDSP3 = &DSP3_Reset;
}

void DSP3_MemoryDump()
{
	DSP3_MemoryIndex = 0;
	SetDSP3 = &DSP3_DumpDataROM;
	DSP3_DumpDataROM();
}

int16 DSP3_WinLo;
int16 DSP3_WinHi;

void DSP3_OP06()
{
	DSP3_WinLo = (uint8)(DSP3_DR);
	DSP3_WinHi = (uint8)(DSP3_DR >> 8);
	DSP3_Reset();
}

void DSP3_OP03()
{
	int16 Lo = (uint8)(DSP3_DR);
	int16 Hi = (uint8)(DSP3_DR >> 8);
	int16 Ofs = (DSP3_WinLo * Hi << 1) + (Lo << 1);
	DSP3_DR = Ofs >> 1;
	SetDSP3 = &DSP3_Reset;
}

int16 DSP3_AddLo;
int16 DSP3_AddHi;

void DSP3_OP07_B()
{
	int16 Ofs = (DSP3_WinLo * DSP3_AddHi << 1) + (DSP3_AddLo << 1);
	DSP3_DR = Ofs >> 1;
	SetDSP3 = &DSP3_Reset;
}

void DSP3_OP07_A()
{
	int16 Lo = (uint8)(DSP3_DR);
	int16 Hi = (uint8)(DSP3_DR >> 8);

	if (Lo & 1)	Hi += (DSP3_AddLo & 1);

	DSP3_AddLo += Lo;
	DSP3_AddHi += Hi;

	if (DSP3_AddLo < 0)
		DSP3_AddLo += DSP3_WinLo;
	else
		if (DSP3_AddLo >= DSP3_WinLo)
			DSP3_AddLo -= DSP3_WinLo;

	if (DSP3_AddHi < 0)
		DSP3_AddHi += DSP3_WinHi;
	else
		if (DSP3_AddHi >= DSP3_WinHi)
			DSP3_AddHi -= DSP3_WinHi;

	DSP3_DR = DSP3_AddLo | (DSP3_AddHi << 8) | ((DSP3_AddHi >> 8) & 0xff);
	SetDSP3 = &DSP3_OP07_B;
}

void DSP3_OP07()
{
	uint32 dataOfs = ((DSP3_DR << 1) + 0x03b2) & 0x03ff;

	DSP3_AddHi = DSP3_DataROM[dataOfs];
	DSP3_AddLo = DSP3_DataROM[dataOfs + 1];

	SetDSP3 = &DSP3_OP07_A;
	DSP3_SR = 0x0080;
}

uint16 DSP3_Codewords;
uint16 DSP3_Outwords;
uint16 DSP3_Symbol;
uint16 DSP3_BitCount;
uint16 DSP3_Index;
uint16 DSP3_Codes[512];
uint16 DSP3_BitsLeft;
uint16 DSP3_ReqBits;
uint16 DSP3_ReqData;
uint16 DSP3_BitCommand;
uint8  DSP3_BaseLength;
uint16 DSP3_BaseCodes;
uint16 DSP3_BaseCode;
uint8  DSP3_CodeLengths[8];
uint16 DSP3_CodeOffsets[8];
uint16 DSP3_LZCode;
uint8  DSP3_LZLength;

uint16 DSP3_X;
uint16 DSP3_Y;

void DSP3_Coordinate()
{
	DSP3_Index++;

	switch (DSP3_Index)
	{
	case 3:
		{
			if (DSP3_DR == 0xffff)
				DSP3_Reset();
			break;
		}
	case 4:
		{
			DSP3_X = DSP3_DR;
			break;
		}
	case 5:
		{
			DSP3_Y = DSP3_DR;
			DSP3_DR = 1;
			break;
		}
	case 6:
		{
			DSP3_DR = DSP3_X;
			break;
		}
	case 7:
		{
			DSP3_DR = DSP3_Y;
			DSP3_Index = 0;
			break;
		}
	}
}

uint8  DSP3_Bitmap[8];
uint8  DSP3_Bitplane[8];
uint16 DSP3_BMIndex;
uint16 DSP3_BPIndex;
uint16 DSP3_Count;

void DSP3_Convert_A()
{
	if (DSP3_BMIndex < 8)
	{
		DSP3_Bitmap[DSP3_BMIndex++] = (uint8) (DSP3_DR);
		DSP3_Bitmap[DSP3_BMIndex++] = (uint8) (DSP3_DR >> 8);

		if (DSP3_BMIndex == 8)
		{
      short i, j;
			for (i=0; i < 8; i++)
				for (j=0; j < 8; j++)
				{
					DSP3_Bitplane[j] <<= 1;
					DSP3_Bitplane[j] |= (DSP3_Bitmap[i] >> j) & 1;
				}

			DSP3_BPIndex = 0;
			DSP3_Count--;
		}
	}

	if (DSP3_BMIndex == 8)
	{
		if (DSP3_BPIndex == 8)
		{
			if (!DSP3_Count) DSP3_Reset();
			DSP3_BMIndex = 0;
		}
		else
		{
			DSP3_DR = DSP3_Bitplane[DSP3_BPIndex++];
			DSP3_DR |= DSP3_Bitplane[DSP3_BPIndex++] << 8;
		}
	}
}

void DSP3_Convert()
{
	DSP3_Count = DSP3_DR;
	DSP3_BMIndex = 0;
	SetDSP3 = &DSP3_Convert_A;
}

bool DSP3_GetBits(uint8 Count)
{
	if (!DSP3_BitsLeft)
	{
		DSP3_BitsLeft = Count;
		DSP3_ReqBits = 0;
	}

	do {
		if (!DSP3_BitCount)
		{
			DSP3_SR = 0xC0;
			return false;
		}

		DSP3_ReqBits <<= 1;
		if (DSP3_ReqData & 0x8000) DSP3_ReqBits++;
		DSP3_ReqData <<= 1;

		DSP3_BitCount--;
		DSP3_BitsLeft--;

	} while (DSP3_BitsLeft);

	return true;
}

void DSP3_Decode_Data()
{
	if (!DSP3_BitCount)
	{
		if (DSP3_SR & 0x40)
		{
			DSP3_ReqData = DSP3_DR;
			DSP3_BitCount += 16;
		}
		else
		{
			DSP3_SR = 0xC0;
			return;
		}
	}

	if (DSP3_LZCode == 1)
	{
		if (!DSP3_GetBits(1))
			return;

		if (DSP3_ReqBits)
			DSP3_LZLength = 12;
		else
			DSP3_LZLength = 8;

		DSP3_LZCode++;
	}

	if (DSP3_LZCode == 2)
	{
		if (!DSP3_GetBits(DSP3_LZLength))
			return;

		DSP3_LZCode = 0;
		DSP3_Outwords--;
		if (!DSP3_Outwords) SetDSP3 = &DSP3_Reset;

		DSP3_SR = 0x80;
		DSP3_DR = DSP3_ReqBits;
		return;
	}

	if (DSP3_BaseCode == 0xffff)
	{
		if (!DSP3_GetBits(DSP3_BaseLength))
			return;

		DSP3_BaseCode = DSP3_ReqBits;
	}

	if (!DSP3_GetBits(DSP3_CodeLengths[DSP3_BaseCode]))
		return;

	DSP3_Symbol = DSP3_Codes[DSP3_CodeOffsets[DSP3_BaseCode] + DSP3_ReqBits];
	DSP3_BaseCode = 0xffff;

	if (DSP3_Symbol & 0xff00)
	{
		DSP3_Symbol += 0x7f02;
		DSP3_LZCode++;
	}
	else
	{
		DSP3_Outwords--;
		if (!DSP3_Outwords)
			SetDSP3 = &DSP3_Reset;
	}

	DSP3_SR = 0x80;
	DSP3_DR = DSP3_Symbol;
}

void DSP3_Decode_Tree()
{
	if (!DSP3_BitCount)
	{
		DSP3_ReqData = DSP3_DR;
		DSP3_BitCount += 16;
	}

	if (!DSP3_BaseCodes)
	{
		DSP3_GetBits(1);
		if (DSP3_ReqBits)
		{
			DSP3_BaseLength = 3;
			DSP3_BaseCodes = 8;
		}
		else
		{
			DSP3_BaseLength = 2;
			DSP3_BaseCodes = 4;
		}
	}

	while (DSP3_BaseCodes)
	{
		if (!DSP3_GetBits(3))
			return;

		DSP3_ReqBits++;

		DSP3_CodeLengths[DSP3_Index] = (uint8) DSP3_ReqBits;
		DSP3_CodeOffsets[DSP3_Index] = DSP3_Symbol;
		DSP3_Index++;

		DSP3_Symbol += 1 << DSP3_ReqBits;
		DSP3_BaseCodes--;
	}

	DSP3_BaseCode = 0xffff;
	DSP3_LZCode = 0;

	SetDSP3 = &DSP3_Decode_Data;
	if (DSP3_BitCount) DSP3_Decode_Data();
}

void DSP3_Decode_Symbols()
{
	DSP3_ReqData = DSP3_DR;
	DSP3_BitCount += 16;

	do {

		if (DSP3_BitCommand == 0xffff)
		{
			if (!DSP3_GetBits(2)) return;
			DSP3_BitCommand = DSP3_ReqBits;
		}

		switch (DSP3_BitCommand)
		{
		case 0:
			{
				if (!DSP3_GetBits(9)) return;
				DSP3_Symbol = DSP3_ReqBits;
				break;
			}
		case 1:
			{
				DSP3_Symbol++;
				break;
			}
		case 2:
			{
				if (!DSP3_GetBits(1)) return;
				DSP3_Symbol += 2 + DSP3_ReqBits;
				break;
			}
		case 3:
			{
				if (!DSP3_GetBits(4)) return;
				DSP3_Symbol += 4 + DSP3_ReqBits;
				break;
			}
		}

		DSP3_BitCommand = 0xffff;

		DSP3_Codes[DSP3_Index++] = DSP3_Symbol;
		DSP3_Codewords--;

	} while (DSP3_Codewords);

	DSP3_Index = 0;
	DSP3_Symbol = 0;
	DSP3_BaseCodes = 0;

	SetDSP3 = &DSP3_Decode_Tree;
	if (DSP3_BitCount) DSP3_Decode_Tree();
}

void DSP3_Decode_A()
{
	DSP3_Outwords = DSP3_DR;
	SetDSP3 = &DSP3_Decode_Symbols;
	DSP3_BitCount = 0;
	DSP3_BitsLeft = 0;
	DSP3_Symbol = 0;
	DSP3_Index = 0;
	DSP3_BitCommand = 0xffff;
	DSP3_SR = 0xC0;
}

void DSP3_Decode()
{
	DSP3_Codewords = DSP3_DR;
	SetDSP3 = &DSP3_Decode_A;
}


// Opcodes 1E/3E bit-perfect to 'dsp3-intro' log
// src: adapted from SD Gundam X/G-Next

int16 op3e_x;
int16 op3e_y;

int16 op1e_terrain[0x2000];
int16 op1e_cost[0x2000];
int16 op1e_weight[0x2000];

int16 op1e_cell;
int16 op1e_turn;
int16 op1e_search;

int16 op1e_x;
int16 op1e_y;

int16 op1e_min_radius;
int16 op1e_max_radius;

int16 op1e_max_search_radius;
int16 op1e_max_path_radius;

int16 op1e_lcv_radius;
int16 op1e_lcv_steps;
int16 op1e_lcv_turns;

void DSP3_OP3E()
{
	op3e_x = (uint8)(DSP3_DR & 0x00ff);
	op3e_y = (uint8)((DSP3_DR & 0xff00)>>8);

	DSP3_OP03();

	op1e_terrain[ DSP3_DR ] = 0x00;
	op1e_cost[ DSP3_DR ] = 0xff;
	op1e_weight[ DSP3_DR ] = 0;

	op1e_max_search_radius = 0;
	op1e_max_path_radius = 0;
}

void DSP3_OP1E_A();
void DSP3_OP1E_A1();
void DSP3_OP1E_A2();
void DSP3_OP1E_A3();

void DSP3_OP1E_B();
void DSP3_OP1E_B1();
void DSP3_OP1E_B2();

void DSP3_OP1E_C();
void DSP3_OP1E_C1();
void DSP3_OP1E_C2();

void DSP3_OP1E_D( int16, int16 *, int16 * );
void DSP3_OP1E_D1( int16 move, int16 *lo, int16 *hi );

void DSP3_OP1E()
{
	int lcv;

	op1e_min_radius = (uint8)(DSP3_DR & 0x00ff);
	op1e_max_radius = (uint8)((DSP3_DR & 0xff00)>>8);

	if( op1e_min_radius == 0 )
		op1e_min_radius++;

	if( op1e_max_search_radius >= op1e_min_radius )
		op1e_min_radius = op1e_max_search_radius+1;

	if( op1e_max_radius > op1e_max_search_radius )
		op1e_max_search_radius = op1e_max_radius;

	op1e_lcv_radius = op1e_min_radius;
	op1e_lcv_steps = op1e_min_radius;

	op1e_lcv_turns = 6;
	op1e_turn = 0;

	op1e_x = op3e_x;
	op1e_y = op3e_y;

	for( lcv = 0; lcv < op1e_min_radius; lcv++ )
		DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );

	DSP3_OP1E_A();
}

void DSP3_OP1E_A()
{
	int lcv;

	if( op1e_lcv_steps == 0 ) {
		op1e_lcv_radius++;

		op1e_lcv_steps = op1e_lcv_radius;

		op1e_x = op3e_x;
		op1e_y = op3e_y;

		for( lcv = 0; lcv < op1e_lcv_radius; lcv++ )
			DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );
	}

	if( op1e_lcv_radius > op1e_max_radius ) {
		op1e_turn++;
		op1e_lcv_turns--;

		op1e_lcv_radius = op1e_min_radius;
		op1e_lcv_steps = op1e_min_radius;

		op1e_x = op3e_x;
		op1e_y = op3e_y;

		for( lcv = 0; lcv < op1e_min_radius; lcv++ )
			DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );
	}

	if( op1e_lcv_turns == 0 ) {
		DSP3_DR = 0xffff;
		DSP3_SR = 0x0080;
		SetDSP3 = &DSP3_OP1E_B;
		return;
	}

	DSP3_DR = (uint8)(op1e_x) | ((uint8)(op1e_y)<<8);
	DSP3_OP03();

	op1e_cell = DSP3_DR;

	DSP3_SR = 0x0080;
	SetDSP3 = &DSP3_OP1E_A1;
}

void DSP3_OP1E_A1()
{
	DSP3_SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_A2;
}

void DSP3_OP1E_A2()
{
	op1e_terrain[ op1e_cell ] = (uint8)(DSP3_DR & 0x00ff);

	DSP3_SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_A3;
}

void DSP3_OP1E_A3()
{
	op1e_cost[ op1e_cell ] = (uint8)(DSP3_DR & 0x00ff);

	if( op1e_lcv_radius == 1 ) {
		if( op1e_terrain[ op1e_cell ] & 1 ) {
			op1e_weight[ op1e_cell ] = 0xff;
		} else {
			op1e_weight[ op1e_cell ] = op1e_cost[ op1e_cell ];
		}
	}
	else {
		op1e_weight[ op1e_cell ] = 0xff;
	}

	DSP3_OP1E_D( (int16)(op1e_turn+2), &op1e_x, &op1e_y );
	op1e_lcv_steps--;

	DSP3_SR = 0x0080;
	DSP3_OP1E_A();
}


void DSP3_OP1E_B()
{
	op1e_x = op3e_x;
	op1e_y = op3e_y;
	op1e_lcv_radius = 1;

	op1e_search = 0;

	DSP3_OP1E_B1();

	SetDSP3 = &DSP3_OP1E_C;
}


void DSP3_OP1E_B1()
{
	while( op1e_lcv_radius < op1e_max_radius ) {
		op1e_y--;

		op1e_lcv_turns = 6;
		op1e_turn = 5;

		while( op1e_lcv_turns ) {
			op1e_lcv_steps = op1e_lcv_radius;

			while( op1e_lcv_steps ) {
				DSP3_OP1E_D1( op1e_turn, &op1e_x, &op1e_y );

				if( 0 <= op1e_y && op1e_y < DSP3_WinHi &&
						0 <= op1e_x && op1e_x < DSP3_WinLo ) {
					DSP3_DR = (uint8)(op1e_x) | ((uint8)(op1e_y)<<8);
					DSP3_OP03();

					op1e_cell = DSP3_DR;
					if( op1e_cost[ op1e_cell ] < 0x80 &&
							op1e_terrain[ op1e_cell ] < 0x40 ) {
						DSP3_OP1E_B2();
					} // end cell perimeter
				}

				op1e_lcv_steps--;
			} // end search line

			op1e_turn--;
			if( op1e_turn == 0 ) op1e_turn = 6;

			op1e_lcv_turns--;
		} // end circle search

		op1e_lcv_radius++;
	} // end radius search
}


void DSP3_OP1E_B2()
{
	int16 cell;
	int16 path;
	int16 x,y;
	int16 lcv_turns;

	path = 0xff;
	lcv_turns = 6;

	while( lcv_turns ) {
		x = op1e_x;
		y = op1e_y;

		DSP3_OP1E_D1( lcv_turns, &x, &y );

		DSP3_DR = (uint8)(x) | ((uint8)(y)<<8);
		DSP3_OP03();

		cell = DSP3_DR;

		if( 0 <= y && y < DSP3_WinHi &&
				0 <= x && x < DSP3_WinLo  ) {

			if( op1e_terrain[ cell ] < 0x80 || op1e_weight[ cell ] == 0 ) {
				if( op1e_weight[ cell ] < path ) {
					path = op1e_weight[ cell ];
				}
			}
		} // end step travel

		lcv_turns--;
	} // end while turns

	if( path != 0xff ) {
		op1e_weight[ op1e_cell ] = path + op1e_cost[ op1e_cell ];
	}
}


void DSP3_OP1E_C()
{
	int lcv;

	op1e_min_radius = (uint8)(DSP3_DR & 0x00ff);
	op1e_max_radius = (uint8)((DSP3_DR & 0xff00)>>8);

	if( op1e_min_radius == 0 )
		op1e_min_radius++;

	if( op1e_max_path_radius >= op1e_min_radius )
		op1e_min_radius = op1e_max_path_radius+1;

	if( op1e_max_radius > op1e_max_path_radius )
		op1e_max_path_radius = op1e_max_radius;

	op1e_lcv_radius = op1e_min_radius;
	op1e_lcv_steps = op1e_min_radius;

	op1e_lcv_turns = 6;
	op1e_turn = 0;

	op1e_x = op3e_x;
	op1e_y = op3e_y;

	for( lcv = 0; lcv < op1e_min_radius; lcv++ )
		DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );

	DSP3_OP1E_C1();
}


void DSP3_OP1E_C1()
{
	int lcv;

	if( op1e_lcv_steps == 0 ) {
		op1e_lcv_radius++;

		op1e_lcv_steps = op1e_lcv_radius;

		op1e_x = op3e_x;
		op1e_y = op3e_y;

		for( lcv = 0; lcv < op1e_lcv_radius; lcv++ )
			DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );
	}

	if( op1e_lcv_radius > op1e_max_radius ) {
		op1e_turn++;
		op1e_lcv_turns--;

		op1e_lcv_radius = op1e_min_radius;
		op1e_lcv_steps = op1e_min_radius;

		op1e_x = op3e_x;
		op1e_y = op3e_y;

		for( lcv = 0; lcv < op1e_min_radius; lcv++ )
			DSP3_OP1E_D( op1e_turn, &op1e_x, &op1e_y );
	}

	if( op1e_lcv_turns == 0 ) {
		DSP3_DR = 0xffff;
		DSP3_SR = 0x0080;
		SetDSP3 = &DSP3_Reset;
		return;
	}

	DSP3_DR = (uint8)(op1e_x) | ((uint8)(op1e_y)<<8);
	DSP3_OP03();

	op1e_cell = DSP3_DR;

	DSP3_SR = 0x0080;
	SetDSP3 = &DSP3_OP1E_C2;
}


void DSP3_OP1E_C2()
{
	DSP3_DR = op1e_weight[ op1e_cell ];

	DSP3_OP1E_D( (int16)(op1e_turn+2), &op1e_x, &op1e_y );
	op1e_lcv_steps--;

	DSP3_SR = 0x0084;
	SetDSP3 = &DSP3_OP1E_C1;
}


void DSP3_OP1E_D( int16 move, int16 *lo, int16 *hi )
{
	uint32 dataOfs = ((move << 1) + 0x03b2) & 0x03ff;
	int16 Lo;
	int16 Hi;

	DSP3_AddHi = DSP3_DataROM[dataOfs];
	DSP3_AddLo = DSP3_DataROM[dataOfs + 1];

	Lo = (uint8)(*lo);
	Hi = (uint8)(*hi);

	if (Lo & 1)	Hi += (DSP3_AddLo & 1);

	DSP3_AddLo += Lo;
	DSP3_AddHi += Hi;

	if (DSP3_AddLo < 0)
		DSP3_AddLo += DSP3_WinLo;
	else
		if (DSP3_AddLo >= DSP3_WinLo)
			DSP3_AddLo -= DSP3_WinLo;

	if (DSP3_AddHi < 0)
		DSP3_AddHi += DSP3_WinHi;
	else
		if (DSP3_AddHi >= DSP3_WinHi)
			DSP3_AddHi -= DSP3_WinHi;

	*lo = DSP3_AddLo;
	*hi = DSP3_AddHi;
}


void DSP3_OP1E_D1( int16 move, int16 *lo, int16 *hi )
{
	//uint32 dataOfs = ((move << 1) + 0x03b2) & 0x03ff;
	int16 Lo;
	int16 Hi;

	const unsigned short HiAdd[] = {
		0x00, 0xFF, 0x00, 0x01, 0x01, 0x01, 0x00, 0x00,
		0x00, 0xFF, 0xFF, 0x00, 0x01, 0x00, 0xFF, 0x00
	};
	const unsigned short LoAdd[] = {
		0x00, 0x00, 0x01, 0x01, 0x00, 0xFF, 0xFF, 0x00
	};

	if( (*lo) & 1 )
		DSP3_AddHi = HiAdd[ move + 8 ];
	else
		DSP3_AddHi = HiAdd[ move + 0 ];
	DSP3_AddLo = LoAdd[ move ];

	Lo = (uint8)(*lo);
	Hi = (uint8)(*hi);

	if (Lo & 1)	Hi += (DSP3_AddLo & 1);

	DSP3_AddLo += Lo;
	DSP3_AddHi += Hi;

	*lo = DSP3_AddLo;
	*hi = DSP3_AddHi;
}


void DSP3_OP10()
{
	if( DSP3_DR == 0xffff ) {
		DSP3_Reset();
	} else {
		// absorb 2 bytes
		DSP3_DR = DSP3_DR;
	}
}


void DSP3_OP0C_A()
{
	// absorb 2 bytes

	DSP3_DR = 0;
	SetDSP3 = &DSP3_Reset;
}


void DSP3_OP0C()
{
	// absorb 2 bytes

	DSP3_DR = 0;
	//SetDSP3 = &DSP3_OP0C_A;
	SetDSP3 = &DSP3_Reset;
}


void DSP3_OP1C_C()
{
	// return 2 bytes
	DSP3_DR = 0;
	SetDSP3 = &DSP3_Reset;
}


void DSP3_OP1C_B()
{
	// absorb 2 bytes

	// return 2 bytes
	DSP3_DR = 0;
	SetDSP3 = &DSP3_OP1C_C;
}


void DSP3_OP1C_A()
{
	// absorb 2 bytes

	SetDSP3 = &DSP3_OP1C_B;
}


void DSP3_OP1C()
{
	// absorb 2 bytes

	SetDSP3 = &DSP3_OP1C_A;
}


void DSP3_Command()
{
	if (DSP3_DR < 0x40)
	{
		switch (DSP3_DR)
		{
    case 0x02: SetDSP3 = &DSP3_Coordinate; break;
    case 0x03: SetDSP3 = &DSP3_OP03; break;
    case 0x06: SetDSP3 = &DSP3_OP06; break;
    case 0x07: SetDSP3 = &DSP3_OP07; return;
    case 0x0c: SetDSP3 = &DSP3_OP0C; break;
    case 0x0f: SetDSP3 = &DSP3_TestMemory; break;
    case 0x10: SetDSP3 = &DSP3_OP10; break;
    case 0x18: SetDSP3 = &DSP3_Convert; break;
    case 0x1c: SetDSP3 = &DSP3_OP1C; break;
    case 0x1e: SetDSP3 = &DSP3_OP1E; break;
    case 0x1f: SetDSP3 = &DSP3_MemoryDump; break;
    case 0x38: SetDSP3 = &DSP3_Decode; break;
    case 0x3e: SetDSP3 = &DSP3_OP3E; break;
    default:
     return;
		}
		DSP3_SR = 0x0080;
		DSP3_Index = 0;
	}
}

uint8 dsp3_byte;
uint16 dsp3_address;

void DSP3SetByte()
{
  if (dsp3_address < 0xC000)
  {
		if (DSP3_SR & 0x04)
		{
			DSP3_DR = (DSP3_DR & 0xff00) + dsp3_byte;
			(*SetDSP3)();
		}
		else
		{
			DSP3_SR ^= 0x10;

			if (DSP3_SR & 0x10)
				DSP3_DR = (DSP3_DR & 0xff00) + dsp3_byte;
			else
			{
				DSP3_DR = (DSP3_DR & 0x00ff) + (dsp3_byte << 8);
				(*SetDSP3)();
			}
		}
  }
}

void DSP3GetByte()
{
  if (dsp3_address < 0xC000)
  {
		if (DSP3_SR & 0x04)
		{
			dsp3_byte = (uint8) DSP3_DR;
			(*SetDSP3)();
		}
		else
		{
			DSP3_SR ^= 0x10;

			if (DSP3_SR & 0x10)
				dsp3_byte = (uint8) (DSP3_DR);
			else
			{
				dsp3_byte = (uint8) (DSP3_DR >> 8);
				(*SetDSP3)();
			}
		}

  }
  else
  {
    dsp3_byte = (uint8) DSP3_SR;
  }
}

void InitDSP3()
{
  DSP3_Reset();
}

#endif
