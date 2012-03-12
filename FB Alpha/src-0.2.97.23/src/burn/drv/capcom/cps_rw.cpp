#include "cps.h"
// CPS - Read/Write

// Input bits
#define INP(nnn) UINT8 CpsInp##nnn[8];
CPSINPSET
#undef  INP

// Bytes to return from ports
#define INP(nnn) UINT8 Inp##nnn;
CPSINPSET
#undef  INP

UINT16 CpsInp055 = 0;
UINT16 CpsInp05d = 0;
UINT16 CpsInpPaddle1 = 0;
UINT16 CpsInpPaddle2 = 0;
static INT32 ReadPaddle = 0;
INT32 CpsPaddle1Value = 0;
INT32 CpsPaddle2Value = 0;
INT32 CpsPaddle1 = 0;
INT32 CpsPaddle2 = 0;
static INT32 nDial055, nDial05d;

INT32 PangEEP = 0;
INT32 Forgottn = 0;
INT32 Cps1QsHack = 0;
INT32 Kodh = 0;
INT32 Cawingb = 0;
INT32 Wofh = 0;
INT32 Sf2thndr = 0;
INT32 Pzloop2 = 0;
INT32 Ssf2tb = 0;
INT32 Dinopic = 0;
INT32 Dinohunt = 0;
INT32 Port6SoundWrite = 0;

static INT32 nCalc[2] = {0, 0};

static const bool nCPSExtraNVRAM = false;
static INT32 n664001;

#define INP(nnnn) UINT8 CpsInp##nnnn[8];
CPSINPEX
#undef  INP

#define INP(nnnn) static UINT8 Inp##nnnn;
CPSINPEX
#undef  INP


// Read input port 0x000-0x1ff
static UINT8 CpsReadPort(const UINT32 ia)
{
	UINT8 d = 0xFF;
	
//	bprintf(PRINT_NORMAL, _T("Read Port %x\n"), ia);
	
	if (ia == 0x000) {
		d = (UINT8)~Inp000;
		if (Pzloop2) {
			if (ReadPaddle) {
				d -= CpsPaddle2Value;
			} else {
				d = CpsPaddle2;
			}
		}
		return d;
	}
	if (ia == 0x001) {
		d = (UINT8)~Inp001;
		if (Pzloop2) {
			if (ReadPaddle) {
				d -= CpsPaddle1Value;
			} else {
				d = CpsPaddle1;
			}
		}
		return d;
	}
	if (ia == 0x010) {
		d = (UINT8)~Inp010;
		return d;
	}
	if (ia == 0x011) {
		d = (UINT8)~Inp011;
		return d;
	}
	if (ia == 0x012) {
		d = (UINT8)~Inp012;
		return d;
	}
	if (ia == 0x018) {
		d = (UINT8)~Inp018;
		return d;
	}
	if (ia == 0x019) {
		d = (UINT8)~Inp019;
		return d;
	}
	if (ia == 0x01A) {
		d = (UINT8)~Cpi01A;
		return d;
	}
	if (ia == 0x01C) {
		d = (UINT8)~Cpi01C;
		return d;
	}
	if (ia == 0x01E) {
		d = (UINT8)~Cpi01E;
		return d;
	}

	if (Cps == 2) {
		// Used on CPS2 only I think
		if (ia == 0x020) {
			d = (UINT8)~Inp020;
			return d;
		}
		if (ia == 0x021) {
			d = (UINT8)~Inp021;
			d &= 0xFE;
			d |= EEPROMRead();
			return d;
		}

		// CPS2 Volume control
		if (ia == 0x030) {
			if (Ssf2tb) {
				d = 0x20;
			} else {
				d = 0xe0;
			}
			return d;
		}
		if (ia == 0x031) {
			d = 0x21;
			return d;
		}

		if (ia >= 0x0100 && ia < 0x0200) {
			static INT32 nRasterLine;

//			bprintf(PRINT_NORMAL, _T("  - port 0x%02X (%3i)\n"), ia & 255, SekCurrentScanline());

			// The linecounters seem to return the line at which the last IRQ triggered by this counter is scheduled minus the current line
			if ((ia & 0x0FE) == 0x50) {
				if ((ia & 1) == 0) {
					nRasterLine = nIrqLine50 - SekCurrentScanline();
					return nRasterLine >> 8;
				} else {
					return nRasterLine & 0xFF;
				}
			}
			if ((ia & 0x0FE) == 0x52) {
				if ((ia & 1) == 0) {
					nRasterLine = nIrqLine52 - SekCurrentScanline();
					return nRasterLine >> 8;
				} else {
					return nRasterLine & 0xFF;
				}
			}

		}
	} else {
		// Board ID
		if (ia == 0x100 + CpsBID[0]) {
			d = (UINT8)CpsBID[1];
			return d;
		}
		if (ia == 0x100 + (CpsBID[0] + 1)) {
			d = (UINT8)CpsBID[2];
			return d;
		}
		
		if (Sf2thndr) {
			// this reads the B-ID from here on startup as well as the normal location in-game
			if (ia == 0x1c8) {
				d = (UINT8)CpsBID[1];
				return d;
			}
		
			if (ia == 0x1c9) {
				d = (UINT8)CpsBID[2];
				return d;
			}
		}
		
		// CPS1 EEPROM read
		if (ia == 0xC007) {
			if (Cps1Qs) {
				return EEPROMRead();
			} else {
				return 0;
			}
		}
		
		// Pang3 EEPROM
		if (PangEEP == 1) {
			if (ia == 0x17B) {
				return EEPROMRead();
			}
		}
		
		// Extra Input ports (move from game-to-game)
		if (ia == 0x006) {
			d = (UINT8)~Inp006;
			return d;
		}
		if (ia == 0x007) {
			d = (UINT8)~Inp007;
			return d;
		}
		if (ia == 0x008) {
			d = (UINT8)~Inp008;
			return d;
		}
		if (ia == 0x029) {
			d = (UINT8)~Inp029;
			return d;
		}		
		if (ia == 0x176) {
			d = (UINT8)~Inp176;
			return d;
		}
		if (ia == 0x177) {
			d = (UINT8)~Inp177;
			return d;
		}		
		if (ia == 0x179) {
			d = (UINT8)~Inp179;
			return d;
		}
		if (ia == 0x186) {
			d = (UINT8)~Inp186;
			return d;
		}		
		if (ia == 0x1fd) {
			d = (UINT8)~Inp1fd;
			return d;
		}		
		if (ia == 0xC000) {
			d = (UINT8)~Inpc000;
			return d;
		}
		if (ia == 0xC001) {
			d = (UINT8)~Inpc001;
			return d;
		}
		if (ia == 0xC002) {
			d = (UINT8)~Inpc002;
			return d;
		}
		if (ia == 0xC003) {
			d = (UINT8)~Inpc003;
			return d;
		}
		
		// Forgotten Worlds Dial
		if (Forgottn) {
			if (ia == 0x053) {
				return (nDial055 >>  8) & 0xFF;
			}
			if (ia == 0x055) {
				return (nDial055 >> 16) & 0xFF;
			}
			if (ia == 0x05B) {
				return (nDial05d >>  8) & 0xFF;
			}
			if (ia == 0x05D) {
				return (nDial05d >> 16) & 0xFF;
			}
		}	
	}

	return d;
}

// Write output port 0x000-0x1ff
static void CpsWritePort(const UINT32 ia, UINT8 d)
{
	if ((Cps & 1) && Cps1Qs == 0) {
		// CPS1 sound code
		if (ia == 0x181 || (Port6SoundWrite && (ia == 0x006 || ia == 0x007))) {
			PsndSyncZ80((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles);

			PsndCode = d;
			return;
		}

		// CPS1 sound fade
		if (ia == 0x189) {
			PsndSyncZ80((INT64)SekTotalCycles() * nCpsZ80Cycles / nCpsCycles);

			PsndFade = d;
			return;
		}

		if (ia == 0x041) {
			nDial055 = 0;
		}
		if (ia == 0x049) {
			nDial05d = 0;
		}
	}

	if (Cps == 1 && Cps1QsHack == 1) {
		if (ia == 0x181) {
			// Pass the Sound Code to the Q-Sound Shared Ram
			CpsZRamC0[0x001] = d;
		}
	}

	// CPS registers
	if (ia >= 0x100 && ia < 0x200) {
		//Pang3 EEPROM
		if (PangEEP == 1 && ia == 0x17B) {
			EEPROMWrite(d & 0x40, d & 0x80, d & 0x01);
			return;
		}
		CpsReg[(ia ^ 1) & 0xFF] = d;
		
		if (ia == 0x10b) {
			GetPalette(0, 6);
			CpsPalUpdate(CpsSavePal);
		}
		return;
	}

	if (Cps == 2) {
		if (ia == 0x40) {
			EEPROMWrite(d & 0x20, d& 0x40, d & 0x10);
			return;
		}

		// CPS2 object bank select
		if ((ia & 0x1FF) == 0x0E1) {
//			bprintf(PRINT_NORMAL, _T("  - %2i (%3i)\n"), d & 1, SekCurrentScanline());
//			CpsObjGet();
			CpsMapObjectBanks(d & 1);
			return;
		}
		
		if (ia == 0x41 && Pzloop2) {
			ReadPaddle = d & 0x02;
		}
	}

	if (Cps1Qs == 1) {
		//CPS1 EEPROM write
		if (ia == 0xc007) {
			EEPROMWrite(d & 0x40, d & 0x80, d & 0x01);
			return;
		}
	}
}

UINT8 __fastcall CpsReadByte(UINT32 a)
{
	// Input ports mirrored between 0x800000 and 0x807fff
	if ((a & 0xFF8000) == 0x800000) {
		return CpsReadPort(a & 0x1FF);
	}

	if (Cps == 2) {
		if ((a & 0xFF8000) == 0x660000) {
			if (a == 0x664001) {
				return n664001;
			}
		}

		return 0x00;
	}

	if (a >= 0xF1C000 && a <= 0xF1C007) {
		return CpsReadPort(a & 0xC00F);
	}
	
	if (Dinohunt && a == 0xfc0001) return (UINT8)~Inpc001;
	
//	bprintf(PRINT_NORMAL, _T("Read Byte %x\n"), a);
	
	return 0x00;
}

void __fastcall CpsWriteByte(UINT32 a,UINT8 d)
{
	// Output ports mirrored between 0x800000 and 0x807fff
	if ((a & 0xFF8000) == 0x800000) {
		CpsWritePort(a & 0x1FF, d);
		return;
	}
	
	if (Cps == 2) {
		// 0x400000 registers
		if ((a & 0xFFFFF0) == 0x400000)	{
			CpsFrg[a & 0x0F] = d;
			return;
		}
		if ((a & 0xFF8000) == 0x660000) {
			if (a == 0x664001) {
				// bit 1 toggled on/off each frame
				n664001 = d;
			}
			
			return;
		}

		return;
	}
	
	if (Cps1Qs == 1) {
		// CPS1 EEPROM
		if (a == 0xf1c007) {
			CpsWritePort(a & 0xC00F, d);
			return;
		}
	}
	
//	bprintf(PRINT_NORMAL, _T("Write Byte %x, %x\n"), a, d);
}

UINT16 __fastcall CpsReadWord(UINT32 a)
{
	if ((a & 0xFF8FFF) == 0x800100 + CpsMProt[3]) {
		return (UINT16)((nCalc[0] * nCalc[1]) >> 16);
	}
	// ports mirrored between 0x800000 and 0x807fff
	if ((a & 0xFF8FFF) == 0x800100 + CpsMProt[2]) {
		return (UINT16)((nCalc[0] * nCalc[1]));
	}
	
//	bprintf(PRINT_NORMAL, _T("Read Word %x\n"), a);
	
	SEK_DEF_READ_WORD(0, a);
}

void __fastcall CpsWriteWord(UINT32 a, UINT16 d)
{
	// ports mirrored between 0x800000 and 0x807fff
	if ((a & 0xFF8FFF) == 0x800100 + CpsMProt[0])
		nCalc[0] = d;
	if ((a & 0xFF8FFF) == 0x800100 + CpsMProt[1])
		nCalc[1] = d;

	if (a == 0x804040) {
		if ((d & 0x0008) == 0) {
			ZetReset();
		}
	}
	
	if (Dinopic && a == 0x800222) {
		CpsReg[6] = d & 0xff;
		CpsReg[7] = d >> 8;
		return;
	}
	
//	bprintf(PRINT_NORMAL, _T("Write Word %x, %x\n"), a, d);
	
	SEK_DEF_WRITE_WORD(0, a, d);
}

// Reset all inputs to zero
static INT32 InpBlank()
{
#define INP(nnn) Inp##nnn = 0; memset(CpsInp##nnn, 0, sizeof(CpsInp##nnn));
	CPSINPSET
#undef INP

#define INP(nnnn) Inp##nnnn = 0; memset(CpsInp##nnnn, 0, sizeof(CpsInp##nnnn));
	CPSINPEX
#undef INP

	CpsInp055 = CpsInp05d = 0;

	return 0;
}

INT32 CpsRwInit()
{
	InpBlank();
	return 0;
}

INT32 CpsRwExit()
{
	InpBlank();
	return 0;
}

inline static void StopOpposite(UINT8* pInput)
{
	if ((*pInput & 0x03) == 0x03) {
		*pInput &= ~0x03;
	}
	if ((*pInput & 0x0C) == 0x0C) {
		*pInput &= ~0x0C;
	}
}

INT32 CpsRwGetInp()
{
	// Compile separate buttons into Inpxxx
#define INP(nnn) \
  { INT32 i = 0; Inp##nnn = 0; \
    for (i = 0; i < 8; i++) { Inp##nnn |= (CpsInp##nnn[i] & 1) << i; }  }
	CPSINPSET
#undef INP


#define INP(nnnn) \
  { INT32 i = 0; Inp##nnnn = 0; \
    for (i = 0; i < 8; i++) { Inp##nnnn |= (CpsInp##nnnn[i] & 1) << i; }  }
	CPSINPEX
#undef INP

	if (Forgottn) {
		// Handle analog controls
		nDial055 += (INT32)((INT16)CpsInp055);
		nDial05d += (INT32)((INT16)CpsInp05d);
	}
	
	if (Pzloop2) {
		if (ReadPaddle) {
			CpsPaddle1Value = 0;
			CpsPaddle2Value = 0;
			if (CpsInpPaddle1) {
				if (CpsInpPaddle1 > 0x8000) {
					CpsPaddle1Value = 2;
				}
	
				if (CpsInpPaddle1 < 0x7fff) {
					CpsPaddle1Value = 1;
				}
			}
			
			if (CpsInpPaddle2) {
				if (CpsInpPaddle2 > 0x8000) {
					CpsPaddle2Value = 2;
				}
	
				if (CpsInpPaddle2 < 0x7fff) {
					CpsPaddle2Value = 1;
				}
			}
		}
		
		CpsPaddle1 += (CpsInpPaddle1 >> 8) & 0xff;
		CpsPaddle2 += (CpsInpPaddle2 >> 8) & 0xff;
	}
	
	StopOpposite(&Inp000);
	StopOpposite(&Inp001);

	// Ghouls uses a 4-way stick
	if (Ghouls) {
		static UINT8 nPrevInp000, nPrevInp001;

		if ((Inp000 & 0x03) && (Inp000 & 0x0C)) {
			Inp000 ^= (nPrevInp000 & 0x0F);
		} else {
			nPrevInp000 = Inp000;
		}

		if ((Inp001 & 0x03) && (Inp001 & 0x0C)) {
			Inp001 ^= (nPrevInp001 & 0x0F);
		} else {
			nPrevInp001 = Inp001;
		}
	}

	if (nMaxPlayers > 2) {
		if (Cps == 2) {
			StopOpposite(&Inp011);
			if (nMaxPlayers == 4) {
				StopOpposite(&Inp010);
			}
		} else {
			StopOpposite(&Inp177);
			if (nMaxPlayers == 4) {
				StopOpposite(&Inp179);
			}
			if (Cps1Qs) {
				StopOpposite(&Inpc001);
				if (nMaxPlayers == 4) {
					StopOpposite(&Inpc003);
				}
			}
		}
	}

	return 0;
}

void CpsSoundCmd(UINT16 sound_code) {
//	CpsWritePort(0x181, sound_code);
	PsndCode = sound_code;
}
