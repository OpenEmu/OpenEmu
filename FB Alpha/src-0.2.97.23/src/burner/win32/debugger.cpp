#include "burner.h"

HWND hDbgDlg = NULL;

#if defined (FBA_DEBUG)

#include "m68000_intf.h"
#include "m68000_debug.h"
#include "richedit.h"

static bool bLargeWindow;
static bool bOldPause;

static HFONT hCommandFont = NULL;
static unsigned int nMouseWheelScrollLines;

static unsigned int nActiveWindow;
static unsigned int nDisassWindow;
static unsigned int nMemdumpWindow;

static int nDisassCPUType;

static unsigned int nDisassAddress[16];
static int nDisassArea;
static int nDisassAttrib;

static int nMemdumpAddress;
static int nMemdumpArea;
static int nMemdumpAttrib;

static bool bBreakpointHit;

#if defined FBA_DEBUG
extern UINT8 DebugCPU_SekInitted;
#endif

struct DbgM68000State {
	unsigned int a[8]; unsigned int d[8];
	unsigned int pc;
	unsigned int sr;
	unsigned int sp; unsigned int usp; unsigned int isp; unsigned int msp;
	unsigned int vbr;
	unsigned int sfc; unsigned int dfc;
	unsigned int cacr; unsigned int caar;

	unsigned int irq;
};

static DbgM68000State curr_m68000, prev_m68000;

static int nDbgMemoryAreaCount;
static int nDbgCurrentMemoryArea;
static struct BurnArea* DbgMemoryAreaInfo = NULL;

// ----------------------------------------------------------------------------
// Callbacks for m68k's disassembler

static unsigned int DbgFetchByte(unsigned int nAddress) { return SekFetchByte(nAddress); }
static unsigned int DbgFetchWord(unsigned int nAddress) { return SekFetchWord(nAddress); }
static unsigned int DbgFetchLong(unsigned int nAddress) { return SekFetchLong(nAddress); }

static unsigned int DbgFetchByteArea(unsigned int nAddress)
{
	if (nAddress < DbgMemoryAreaInfo[nDisassArea].nLen) {
		return ((unsigned char*)(DbgMemoryAreaInfo[nDisassArea].Data))[nAddress ^ nDisassAttrib];
	}

	return 0;
}
static unsigned int DbgFetchWordArea(unsigned int nAddress)
{
	return (DbgFetchByteArea(nAddress) << 8) | DbgFetchByteArea(nAddress + 1);
}
static unsigned int DbgFetchLongArea(unsigned int nAddress)
{
	return (DbgFetchWordArea(nAddress) << 16) | DbgFetchWordArea(nAddress + 2);
}

// ----------------------------------------------------------------------------
// Message pump used when a breakpoint is hit

static void BreakpointMessagePump()
{
	MSG msg;

	while (GetMessage(&msg, NULL, 0, 0)) {

		if (IsDialogMessage(hDbgDlg, &msg)) {
			continue;
		}
		if (msg.message == (WM_APP + 0)) {
			bBreakpointHit = false;
			return;
		}

		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

// ----------------------------------------------------------------------------
// Feedback functions

static void InitFeedback(HWND hWindow)
{
	TCHAR szBuffer[16384] = _T("");
	TCHAR* pszBuf = szBuffer;

	// RTF header and initial settings
	pszBuf += _stprintf(pszBuf, _T("{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl(\\f0\\fmodern\\charset0\\fprq0 Courier New;)}{\\colortbl\\red0\\green0\\blue0;\\red96\\green96\\blue96;\\red255\\green0\\blue0;\\red0\\green0\\blue255;}\\deflang1033\\horzdoc\\pard\\plain\\f0\\fs18\\cf2 Welcome to the FB Alpha debugger.\\par}"));
	// Send the new data to the RichEdit control
	SendMessage(hWindow, WM_SETTEXT, (WPARAM)0, (LPARAM)szBuffer);
}

static int AddFeedback(HWND hWindow, TCHAR* pszText)
{
#ifdef UNICODE
	SETTEXTEX settext = { ST_SELECTION, 1200 };
#else
	SETTEXTEX settext = { ST_SELECTION, 1252 };
#endif

	// Add the new info to the top
	SendMessage(hWindow, EM_SETSEL, (WPARAM)0, (LPARAM)0);
	// Doesn't work if you send it Unicode data...
	SendMessageA(hWindow, EM_SETTEXTEX, (WPARAM)&settext, (LPARAM)TCHARToANSI(pszText, NULL, 0));

	// Trim the bottom lines
	SendMessage(hWindow, EM_SETSEL, (WPARAM)SendMessage(hWindow, EM_LINEINDEX, 18, 0) - 1, (LPARAM)-1);
	SendMessage(hWindow, EM_SETTEXTEX, (WPARAM)&settext, (LPARAM)_T(""));

	return 0;
}

// ----------------------------------------------------------------------------
// Trace back instructions starting at a given address

static unsigned int trace_pc, trace_size, trace_max;

static void TraceBackDo(unsigned int start_pc, unsigned int trace_level, int cpu_type)
{
	unsigned int pc;
	unsigned int trace;

	// Go over all possible instruction sizes (the longest 68020 instruction is 30 bytes)
	for (unsigned int size = (cpu_type == M68K_CPU_TYPE_68000) ? 10 : 30; size != 0 ; size -= 2) {
		pc = start_pc;
		trace = trace_level;

		// Make sure we don't go past the start of memory
		if (size > pc) {
			break;
		}

		// Check if there is a valid instruction at the current address
		if (m68k_is_valid_instruction(SekFetchWord(pc - size),  cpu_type) != 0) {
			char buf[100] = "";

			// Check if the instruction at the current address has the right size
			if (m68k_disassemble(buf, pc - size,  cpu_type) == size) {
				pc -= size;
				trace++;

				// Update best result
				if (trace_size < trace) {
					trace_size = trace;
					trace_pc = pc;
				}

				// Recurse until we've traced back enough instructions
				if (trace < trace_max) {
					TraceBackDo(pc, trace,  cpu_type);
				}
			}
		}
	}
}

static unsigned int TraceBack(unsigned int nStartAddress, unsigned int nTotalInstructions, int nCPU)
{
	unsigned int pc = nStartAddress;
	unsigned int trace = nTotalInstructions;

	if (nStartAddress == 0 || nTotalInstructions == 0) {
		return 0;
	}

	// If there's no valid instruction at the start adress, increase it
	while (m68k_is_valid_instruction(SekFetchWord(pc), nCPU) == 0 && (pc - nStartAddress) < ((nCPU == M68K_CPU_TYPE_68000) ? 10 : 30) - 2) {
		pc += 2;
	}

	do {
		trace_size = 0;
		trace_pc = pc;
		// Trace back a few extra instructions to ensure we'll return a valid code path
		trace_max = trace + 4;

		TraceBackDo(pc, 0, nCPU);

		pc -= 2;

	} while (trace_size == 0 && pc >= nStartAddress);

	pc = trace_pc;

	// If we've traced back more instructions than needed, skip the extra ones
	while (trace_size > trace) {
		char buf[100] = "";
		pc += m68k_disassemble(buf, pc, nCPU);
		trace++;
	}

	// If we can't trace back at all, simply decrease the address
	if (trace_size == 0) {
		return nTotalInstructions * 2 < nStartAddress ? nStartAddress - nTotalInstructions * 2 : 0;
	}

	return pc;
}

// ----------------------------------------------------------------------------
// Write a disassembly to a file

static int CreateDisassFile(TCHAR* filename, unsigned int start, unsigned int end, int nCPU)
{
	unsigned int size;
	unsigned int pc = start;
	FILE* fp = _tfopen(filename, _T("wt"));

	if (fp == NULL) {
		return 1;
	}

	// Print the disassembly
	while (pc <= end) {
		char buf[100] = "";
		unsigned int j;

		if (pc > 0x00FFFFFE) {
			break;
		}

		// Get the mnemonic
		size = m68k_disassemble(buf, pc, nCPU);

		// Print the address
		_ftprintf(fp, _T("%06x "), pc);

		// Print the word values
		if (nCPU == M68K_CPU_TYPE_68000) {
			for (j = 10; j > size; j -= 2) {
				_ftprintf(fp, _T("     "));
			}
			for (j =  0; j < size; j += 2) {
				_ftprintf(fp, _T("%04x "), SekFetchWord(pc + j));
			}
		} else {
			if (size > 20) {
				for (j =  0; j < 18; j += 2) {
					_ftprintf(fp, _T("%04x "), SekFetchWord(pc + j));
				}
				_ftprintf(fp, _T("...  "));
			} else {
				for (j = 20; j > size; j -= 2) {
					_ftprintf(fp, _T("     "));
				}
				for (j =  0; j < size; j += 2) {
					_ftprintf(fp, _T("%04x "), SekFetchWord(pc + j));
				}
			}
		}

		// Print the instruction
		_ftprintf(fp, _T("%hs\n"), buf);

		pc += size;
	}

	fclose(fp);

	return 0;
}

// ----------------------------------------------------------------------------
// Create a disassembly in RTF format

// Tabs are specified in twips (a single 16pt cell = 100twips)

static void CreateDisass(HWND hWindow, unsigned int nAddress, int nCPU)
{
	int nPageSize = bLargeWindow ? 16 : 12;
	TCHAR szBuffer[16384] = _T("");
	TCHAR* pszBuf = szBuffer;
	unsigned int pc, nSize;

	pc = nAddress;

	// RTF header and initial settings
	pszBuf += _stprintf(pszBuf, _T("{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl(\\f0\\fmodern\\charset0\\fprq0 Courier New;)}{\\colortbl\\red0\\green0\\blue0;\\red96\\green96\\blue96;\\red255\\green0\\blue0;\\red255\\green96\\blue96;\\red224\\green224\\blue224;}\\deflang1033\\horzdoc\\pard\\plain\\f0"));
	if (bLargeWindow) {
		if (nCPU == M68K_CPU_TYPE_68000) {
			// Tabstops							5555    4444    3333    2222    1111    Mnemonic
			pszBuf += _stprintf(pszBuf, _T("\\tx1050\\tx1513\\tx1976\\tx2429\\tx2892\\tx3450"));
		} else {
			// Tabstops							6666    5555    4444    3333    2222    1111    Mnemonic
			pszBuf += _stprintf(pszBuf, _T("\\tx1050\\tx1435\\tx1820\\tx2205\\tx2590\\tx2975\\tx3450"));
		}
		// Indentation
		pszBuf += _stprintf(pszBuf, _T("\\fi-4665\\li4665 "));
	} else {
		// Tabstops
		pszBuf += _stprintf(pszBuf, _T("\\tx735"));
	}

	// Print the disassembly
	for (int i = 0; i < nPageSize; i++) {
		char buf[100] = "";
		unsigned int j;
		unsigned int nCol;

		if (pc > 0x00FFFFFE) {

			do {
				nDisassAddress[i++] = 0x00FFFFFE;
			} while (i < nPageSize);

			break;
		}

		nDisassAddress[i] = pc;

		// Get the mnemonic
		nSize = m68k_disassemble(buf, pc, nCPU);
		// Display the instruction at the current PC in red
		nCol = (pc == SekDbgGetRegister(SEK_REG_PC)) ? 2 : 0;

		// Print the address
		pszBuf += _stprintf(pszBuf, _T("\\fs%i\\cf%i %06x\t"), bLargeWindow ? 24 : 16, nCol, pc);

		if (bLargeWindow) {
			// Print the word values
			if (nCPU == M68K_CPU_TYPE_68000) {
				// Print a filler
				pszBuf += _stprintf(pszBuf, _T("\\fs16\\cf4 "));
				for (j = 10; j > nSize; j -= 2) {
					pszBuf += _stprintf(pszBuf, _T(" -- \t"));
				}
				// Print the words that make up the instruction
				pszBuf += _stprintf(pszBuf, _T("\\cf%i "), nCol + 1);
				for (j =  0; j < nSize; j += 2) {
					pszBuf += _stprintf(pszBuf, _T("%04x\t"), SekFetchWord(pc + j));
				}
			} else {
				if (nSize > 12) {
					// There's not enough room for all words, so print the first 5 words that make up the instruction
					pszBuf += _stprintf(pszBuf, _T("\\fs13\\cf%i "), nCol + 1);
					for (j =  0; j < 10; j += 2) {
						pszBuf += _stprintf(pszBuf, _T("%04x\t"), SekFetchWord(pc + j));
					}
					// Print an ellipsis
					pszBuf += _stprintf(pszBuf, _T("... \t"));
				} else {
					// Print a filler
					pszBuf += _stprintf(pszBuf, _T("\\fs13\\cf4 "));
					for (j = 12; j > nSize; j -= 2) {
						pszBuf += _stprintf(pszBuf, _T(" -- \t"));
					}
					// Print the words that make up the instruction
					pszBuf += _stprintf(pszBuf, _T("\\cf%i "), nCol + 1);
					for (j =  0; j < nSize; j += 2) {
						pszBuf += _stprintf(pszBuf, _T("%04x\t"), SekFetchWord(pc + j));
					}
				}
			}
		}

		// Colour Invalid instructions gray
		if (nCol == 0) {
			if (m68k_is_valid_instruction(SekFetchWord(pc), nCPU) == 0) {
				nCol++;
			}
		}
		// Print the instruction
		pszBuf += _stprintf(pszBuf, _T("\\fs%i\\cf%i %hs\\par"), bLargeWindow ? 24 : 16, nCol, buf);

		pc += nSize;
	}
	pszBuf += _stprintf(pszBuf, _T("}\0"));

	// Send the new data to the RichEdit control
	SendMessage(hWindow, WM_SETTEXT, (WPARAM)0, (LPARAM)szBuffer);

#if 0
	FILE* fp = _tfopen(_T("disass.rtf"), _T("wb"));
	if (fp) {
		fwrite(szBuffer, 1, _tcslen(szBuffer), fp);
		fclose(fp);
	}
#endif
}

// ----------------------------------------------------------------------------
// Create a memory dump in RTF format

static unsigned int DbgReadByteArea(unsigned int nAddress)
{
	if (nAddress < DbgMemoryAreaInfo[nDbgCurrentMemoryArea].nLen) {
		return ((unsigned char*)(DbgMemoryAreaInfo[nDbgCurrentMemoryArea].Data))[nAddress ^ nMemdumpAttrib];
	}

	return 0;
}

static void CreateMemdump(HWND hWindow, int nArea, unsigned int nAddress)
{
	int nPageSize = bLargeWindow ? 16 : 12;
	unsigned int (*DbgReadByte)(unsigned int);
	TCHAR szBuffer[16384] = _T("");
	TCHAR* pszBuf = szBuffer;
	unsigned int pc;

	nDbgCurrentMemoryArea = nArea;

	if (nDbgCurrentMemoryArea) {
		DbgReadByte = DbgReadByteArea;
	} else {
		DbgReadByte = SekReadByte;
	}

	pc = nAddress;

	// RTF header and initial settings
	pszBuf += _stprintf(pszBuf, _T("{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl(\\f0\\fmodern\\fprq0 Courier New\\cpg1252;)}{\\colortbl\\red96\\green96\\blue96;\\red255\\green255\\blue255;\\red0\\green0\\blue0;\\red160\\green160\\blue160;\\red0\\green0\\blue0;\\red224\\green224\\blue224;}\\deflang1033\\horzdoc\\pard\\f0\\plain"));

	if (bLargeWindow) {
		// Tabstops						        00     11       22      33      44      55      66      77      88      99      AA      BB      CC      DD      EE      FF    abcdefghijklnmop
		pszBuf += _stprintf(pszBuf, _T("\\tx1050\\tx1375\\tx1700\\tx2025\\tx2400\\tx2725\\tx3050\\tx3375\\tx3750\\tx4075\\tx4400\\tx4725\\tx5100\\tx5425\\tx5750\\tx6075\\tx6500"));

		// Print the memory dump
		for (int i = 0; i < nPageSize; i++, pc += 16) {
			pszBuf += _stprintf(pszBuf, _T("\\cf4 %06x\t"), pc);
			for (int j = 0; j < 16; j += 2) {
				pszBuf += _stprintf(pszBuf, _T("\\cf0\\cb1 %02x\t\\cf2\\cb3 %02x\t"), DbgReadByte(pc + j), DbgReadByte(pc + j + 1));
			}
			pszBuf += _stprintf(pszBuf, _T("\\cf4 "));
			for (int j = 0; j < 16; j++) {
				unsigned char c = DbgReadByte(pc + j);

				if (c == 0x00 || (c >= 0x09 && c <= 0x0D)) {
					pszBuf += _stprintf(pszBuf, _T("{\\cf5 -}"));
				} else {
					pszBuf += _stprintf(pszBuf, _T("\\\'%02x"), c);
				}
			}
			pszBuf += _stprintf(pszBuf, _T("\\par "));
		}
		pszBuf += _stprintf(pszBuf, _T("}\0"));
	} else {
		pszBuf += _stprintf(pszBuf, _T("\\fs16"));
		for (int i = 0; i < nPageSize; i++, pc += 16) {
			pszBuf += _stprintf(pszBuf, _T("\\cf4 %06x "), pc);
			for (int j = 0; j < 16; j += 2) {
				pszBuf += _stprintf(pszBuf, _T("\\cf0\\cb1 %02x\\cf2\\cb3 %02x"), DbgReadByte(pc + j), DbgReadByte(pc + j + 1));
			}
			pszBuf += _stprintf(pszBuf, _T("\\cf4  "));
			for (int j = 0; j < 16; j++) {
				unsigned char c = DbgReadByte(pc + j);

				if (c == 0x00 || (c >= 0x09 && c <= 0x0D)) {
					pszBuf += _stprintf(pszBuf, _T("{\\cf5 -}"));
				} else {
					pszBuf += _stprintf(pszBuf, _T("\\\'%02x"), c);
				}
			}
			pszBuf += _stprintf(pszBuf, _T("\\par "));
		}
		pszBuf += _stprintf(pszBuf, _T("}\0"));
	}

	// Send the new data to the RichEdit control
	SendMessage(hWindow, WM_SETTEXT, (WPARAM)0, (LPARAM)szBuffer);

#if 0
	FILE* fp = _tfopen(_T("memdump.rtf"), _T("wb"));
	if (fp) {
		fwrite(szBuffer, 1, _tcslen(szBuffer), fp);
		fclose(fp);
	}
#endif
}

// ----------------------------------------------------------------------------
// Memory area setup

static int __cdecl GetMemoryAcb(struct BurnArea* pba)
{
	TCHAR szAreaName[256] = _T("");

	memcpy(DbgMemoryAreaInfo + nDbgMemoryAreaCount, pba, sizeof(BurnArea));

	_stprintf(szAreaName, _T("%hs - %x bytes"), pba->szName, pba->nLen);
	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP1, CB_ADDSTRING, 0, (LPARAM)szAreaName);
	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP1, CB_ADDSTRING, 0, (LPARAM)szAreaName);

	nDbgMemoryAreaCount++;

	return 0;
}

static int GetMemoryAreas()
{
	nDbgMemoryAreaCount = 1;
	DbgMemoryAreaInfo = (BurnArea*)malloc(256 * sizeof(BurnArea));
	if (DbgMemoryAreaInfo == NULL) {
		return 1;
	}

	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP1, CB_ADDSTRING, 0, (LPARAM)_T("Main CPU memory map"));
	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP1, CB_ADDSTRING, 0, (LPARAM)_T("Main CPU memory map"));

	BurnAcb = GetMemoryAcb;
	BurnAreaScan(ACB_MEMORY_ROM | ACB_NVRAM | ACB_MEMCARD | ACB_MEMORY_RAM | ACB_READ, NULL);

	return 0;
}

static int SetupDisass(int nArea)
{
	nDisassAddress[0] = 0;
	nDisassArea = nArea;
	nDisassAttrib = 0;

	nDisassCPUType = SekDbgGetCPUType();

	if (nDisassArea) {
		SekDbgFetchByteDisassembler = DbgFetchByteArea;
		SekDbgFetchWordDisassembler = DbgFetchWordArea;
		SekDbgFetchLongDisassembler = DbgFetchLongArea;
	} else {
		SekDbgFetchByteDisassembler = DbgFetchByte;
		SekDbgFetchWordDisassembler = DbgFetchWord;
		SekDbgFetchLongDisassembler = DbgFetchLong;

		nDisassAddress[0] = TraceBack(SekDbgGetRegister(SEK_REG_PC), 5, nDisassCPUType);
	}

	CreateDisass(GetDlgItem(hDbgDlg, nDisassWindow), nDisassAddress[0], nDisassCPUType);
	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP1, CB_SETCURSEL, (WPARAM)nDisassArea, (LPARAM)0);

	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP2, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	if (nArea) {
		SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP2, CB_ADDSTRING, (WPARAM)0, (LPARAM)_T("little-endian 8-bit / big-endian"));
		SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP2, CB_ADDSTRING, (WPARAM)0, (LPARAM)_T("little-endian 16-bit"));
		SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP2, CB_ADDSTRING, (WPARAM)0, (LPARAM)_T("little-endian 32-bit"));
	} else {
		SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP2, CB_ADDSTRING, (WPARAM)0, (LPARAM)_T("Auto"));
	}
	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN1_DRP2, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	return 0;
}

static int SetupMemdump(int nArea)
{
	nMemdumpArea = nArea;
	nMemdumpAddress = 0x000000;
	nMemdumpAttrib = 0;

	CreateMemdump(GetDlgItem(hDbgDlg, nMemdumpWindow), nMemdumpArea, nMemdumpAddress);
	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP1, CB_SETCURSEL, (WPARAM)nMemdumpArea, (LPARAM)0);

	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP2, CB_RESETCONTENT, (WPARAM)0, (LPARAM)0);
	if (nArea) {
		SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP2, CB_ADDSTRING, (WPARAM)0, (LPARAM)_T("little-endian 8-bit / big-endian"));
		SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP2, CB_ADDSTRING, (WPARAM)0, (LPARAM)_T("little-endian 16-bit"));
		SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP2, CB_ADDSTRING, (WPARAM)0, (LPARAM)_T("little-endian 32-bit"));
	} else {
		SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP2, CB_ADDSTRING, (WPARAM)0, (LPARAM)_T("Auto"));
	}
	SendDlgItemMessage(hDbgDlg, IDC_DBG_MAINWN2_DRP2, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

	return 0;
}

// ----------------------------------------------------------------------------
// Create CPU info in RTF format

int UpdateCPUState(DbgM68000State* Curr_M68000) {
	Curr_M68000->a[0] = SekDbgGetRegister(SEK_REG_A0);
	Curr_M68000->a[1] = SekDbgGetRegister(SEK_REG_A1);
	Curr_M68000->a[2] = SekDbgGetRegister(SEK_REG_A2);
	Curr_M68000->a[3] = SekDbgGetRegister(SEK_REG_A3);
	Curr_M68000->a[4] = SekDbgGetRegister(SEK_REG_A4);
	Curr_M68000->a[5] = SekDbgGetRegister(SEK_REG_A5);
	Curr_M68000->a[6] = SekDbgGetRegister(SEK_REG_A6);
	Curr_M68000->a[7] = SekDbgGetRegister(SEK_REG_A7);

	Curr_M68000->d[0] = SekDbgGetRegister(SEK_REG_D0);
	Curr_M68000->d[1] = SekDbgGetRegister(SEK_REG_D1);
	Curr_M68000->d[2] = SekDbgGetRegister(SEK_REG_D2);
	Curr_M68000->d[3] = SekDbgGetRegister(SEK_REG_D3);
	Curr_M68000->d[4] = SekDbgGetRegister(SEK_REG_D4);
	Curr_M68000->d[5] = SekDbgGetRegister(SEK_REG_D5);
	Curr_M68000->d[6] = SekDbgGetRegister(SEK_REG_D6);
	Curr_M68000->d[7] = SekDbgGetRegister(SEK_REG_D7);

	Curr_M68000->pc = SekDbgGetRegister(SEK_REG_PC);
	Curr_M68000->sr = SekDbgGetRegister(SEK_REG_SR);

	Curr_M68000->sp = SekDbgGetRegister(SEK_REG_SP);
	Curr_M68000->usp = SekDbgGetRegister(SEK_REG_USP);
	Curr_M68000->isp = SekDbgGetRegister(SEK_REG_ISP);

	Curr_M68000->irq = SekDbgGetPendingIRQ();

	return 0;
}

static void CreateCPUInfo(HWND hWindow, bool bUpdate, DbgM68000State* Curr_M68000, DbgM68000State* Prev_M68000)
{
	int col;
	TCHAR szBuffer[16384] = _T("");
	TCHAR* pszBuf = szBuffer;

	int curr_sr, diff_sr;

	if (bUpdate) {
		UpdateCPUState(Curr_M68000);
	}

	curr_sr = Curr_M68000->sr;
	diff_sr = Prev_M68000->sr ^ curr_sr;

	// RTF header and initial settings
	pszBuf += _stprintf(pszBuf, _T("{\\rtf1\\ansi\\ansicpg1252\\deff0{\\fonttbl(\\f0\\fmodern\\fprq0 Courier New\\cpg1252;)}{\\colortbl\\red0\\green0\\blue0;\\red128\\green128\\blue128;\\red255\\green0\\blue0;\\red255\\green96\\blue96;}\\deflang1033\\horzdoc\\pard\\f0\\plain"));
	// Tabstops
	if (bLargeWindow) {
		pszBuf += _stprintf(pszBuf, _T("\\tx1950\\fs24 "));
	} else {
		pszBuf += _stprintf(pszBuf, _T("\\tx1365\\fs16 "));
	}

	// Print SR
	pszBuf += _stprintf(pszBuf, _T("SR  "));
	if (nDisassCPUType >= M68K_CPU_TYPE_68EC020) {
		// T
		if (diff_sr & 0xC000) {
			pszBuf += _stprintf(pszBuf, _T("\\cf%i T%i"), (diff_sr & 0xC000) ? 2 : 0, curr_sr >> 14);
		} else {
			pszBuf += _stprintf(pszBuf, _T("\\cf%i \\\'95\\\'95"), (diff_sr & 0xC000) ? 2 : 0);
		}
	} else {
		// T
		pszBuf += _stprintf(pszBuf, _T("\\cf%i %s"), (diff_sr & 0x8000) ? 2 : 0, (curr_sr & 0x8000) ? _T("T") : _T("\\\'95"));
		// Unused
		pszBuf += _stprintf(pszBuf, _T("{\\cf1\\\'B7}"));
	}
	// S
	pszBuf += _stprintf(pszBuf, _T("\\cf%i %s"), (diff_sr & 0x2000) ? 2 : 0, (curr_sr & 0x2000) ? _T("S") : _T("\\\'95"));
	if (nDisassCPUType >= M68K_CPU_TYPE_68EC020) {
		// M
		pszBuf += _stprintf(pszBuf, _T("\\cf%i %s"), (diff_sr & 0x1000) ? 2 : 0, (curr_sr & 0x1000) ? _T("M") : _T("\\\'95"));
	} else {
		// Unused
		pszBuf += _stprintf(pszBuf, _T("{\\cf1\\\'B7}"));
	}
	// Unused
	pszBuf += _stprintf(pszBuf, _T("{\\cf1\\\'B7}"));
	// I
	pszBuf += _stprintf(pszBuf, _T("\\cf%i IM%i"), (diff_sr & 0x0700) ? 2 : 0, (curr_sr & 0x0700) >> 8);
	// Unused
	pszBuf += _stprintf(pszBuf, _T("{\\cf1\\\'B7\\\'B7\\\'B7}"));
	// X
	pszBuf += _stprintf(pszBuf, _T("\\cf%i %s"), (diff_sr & 0x10) ? 2 : 0, (curr_sr & 0x10) ? _T("X") : _T("\\\'95"));
	// N
	pszBuf += _stprintf(pszBuf, _T("\\cf%i %s"), (diff_sr & 0x08) ? 2 : 0, (curr_sr & 0x08) ? _T("N") : _T("\\\'95"));
	// Z
	pszBuf += _stprintf(pszBuf, _T("\\cf%i %s"), (diff_sr & 0x04) ? 2 : 0, (curr_sr & 0x04) ? _T("Z") : _T("\\\'95"));
	// V
	pszBuf += _stprintf(pszBuf, _T("\\cf%i %s"), (diff_sr & 0x02) ? 2 : 0, (curr_sr & 0x02) ? _T("V") : _T("\\\'95"));
	// C
	pszBuf += _stprintf(pszBuf, _T("\\cf%i %s "), (diff_sr & 0x01) ? 2 : 0, (curr_sr & 0x01) ? _T("C") : _T("\\\'95"));
	// Hex value
	pszBuf += _stprintf(pszBuf, _T("\\cf%i %02x\\cf%i %02x\\par "), (diff_sr & 0xFF00) ? 2 : 0, curr_sr >> 8, (diff_sr & 0xFF) ? 2 : 0, curr_sr & 0xFF);

#if 1
	// Print pending IRQ
	{
		int s1, s2;

		if (bLargeWindow) {
			s1 = 16;
			s2 = 22;
		} else {
			s1 = 12;
			s2 = 14;
		}
		if (Curr_M68000->irq & 7) {
			pszBuf += _stprintf(pszBuf, _T("\\cf0 IRQ %i{\\fs%i  \\fs%i\\cf1 pending}\t"), Curr_M68000->irq & 7, s1, s2);
		} else {
			pszBuf += _stprintf(pszBuf, _T("\\cf1 IRQ 0{\\fs%i  \\fs%i pending}\t"), s1, s2);
		}
	}
#else
	if (nDisassCPUType >= M68K_CPU_TYPE_68010) {
		// Print SFC/DFC
		col = (Curr_M68000->sfc ^ Prev_M68000->sfc) ? 2 : 0;
		pszBuf += _stprintf(pszBuf, _T("\\cf0 SFC \\cf%i %i "), col + 1, Curr_M68000->sfc);
		col = (Curr_M68000->dfc ^ Prev_M68000->dfc) ? 2 : 0;
		pszBuf += _stprintf(pszBuf, _T("\\cf0 DFC \\cf%i %i  "), col + 1, Curr_M68000->dfc);
	} else {
		pszBuf += _stprintf(pszBuf, _T("               "));
	}
#endif

	// Print PC
	col = (Curr_M68000->pc ^ Prev_M68000->pc) ? 2 : 0;
	pszBuf += _stprintf(pszBuf, _T("\\cf0 PC  \\cf%i %02x\\cf%i %06x\\par\\par "), col + 1, Curr_M68000->pc >> 24, col, Curr_M68000->pc & 0x00FFFFFF);

	// Print data and address registers
	for (int i = 0; i < 8; i++) {
		col = (Curr_M68000->d[i] ^ Prev_M68000->d[i]) ? 2 : 0;
		pszBuf += _stprintf(pszBuf, _T("\\cf0 D%i  \\cf%i %08x "), i, col, Curr_M68000->d[i]);

		col = (Curr_M68000->a[i] ^ Prev_M68000->a[i]) ? 2 : 0;
		pszBuf += _stprintf(pszBuf, _T("\\cf0 A%i  \\cf%i %02x\\cf%i %06x\\par "), i, col + 1, Curr_M68000->a[i] >> 24, col, Curr_M68000->a[i] & 0x00FFFFFF);
	}
	pszBuf += _stprintf(pszBuf, _T("\\par"));

	if (nDisassCPUType >= M68K_CPU_TYPE_68EC020) {
		// Print vector base register
		col = (Curr_M68000->vbr ^ Prev_M68000->vbr) ? 2 : 0;
		pszBuf += _stprintf(pszBuf, _T("\\cf0 VBR \\cf%i %02x\\cf%i %06x "), col + 1, Curr_M68000->vbr >> 24, col, Curr_M68000->vbr & 0x00FFFFFF);
		// Print master stack pointer
		col = (Curr_M68000->msp ^ Prev_M68000->msp) ? 2 : 0;
		pszBuf += _stprintf(pszBuf, _T("\\cf0 MSP \\cf%i %02x\\cf%i %06x\\par "), col + 1, Curr_M68000->msp >> 24, col, Curr_M68000->msp & 0x00FFFFFF);
	}

	// Print interrupt stack pointer
	col = (Curr_M68000->isp ^ Prev_M68000->isp) ? 2 : 0;
	pszBuf += _stprintf(pszBuf, _T("\\cf0 ISP \\cf%i %02x\\cf%i %06x "), col + 1, Curr_M68000->isp >> 24, col, Curr_M68000->isp & 0x00FFFFFF);
	// Print user stack pointer
	col = (Curr_M68000->usp ^ Prev_M68000->usp) ? 2 : 0;
	pszBuf += _stprintf(pszBuf, _T("\\cf0 USP \\cf%i %02x\\cf%i %06x\\par "), col + 1, Curr_M68000->usp >> 24, col, Curr_M68000->usp & 0x00FFFFFF);

	// Cycles done this frame
	pszBuf += _stprintf(pszBuf, _T("\\par\\cf0 cycle%7i "), SekTotalCycles());
	if (nSekCyclesScanline) {
		pszBuf += _stprintf(pszBuf, _T("scanline %3i\\par"), SekCurrentScanline());
	} else {
		pszBuf += _stprintf(pszBuf, _T("\\cf1 scanline ---\\par"));
	}

	SendMessage(hWindow, WM_SETTEXT, (WPARAM)0, (LPARAM)szBuffer);

	if (bUpdate) {
		memcpy(Prev_M68000, Curr_M68000, sizeof(DbgM68000State));
	}

#if 0
	FILE* fp = _ftopen(_T("cpuinfo.rtf"), _T("wb");
	if (fp) {
		fwrite(szBuffer, 1, strlen(szBuffer), fp);
		fclose(fp);
	}
#endif
}

// ----------------------------------------------------------------------------
// Callbacks for when a breakpoint is hit

void BreakpointProcessing(TCHAR* pszMessage)
{
	bBreakpointHit = true;
	AudBlankSound();

	// Make sure single stepping is disabled
	SekDbgEnableBreakpoints();

	// Update the feedback area
	AddFeedback(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN2), pszMessage);

	// Update info
	CreateCPUInfo(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN1), true, &curr_m68000, &prev_m68000);
	CreateDisass(GetDlgItem(hDbgDlg, nDisassWindow), TraceBack(SekDbgGetRegister(SEK_REG_PC), 5, nDisassCPUType), nDisassCPUType);

	SetWindowText(GetDlgItem(hDbgDlg, IDC_DBG_GO), _T("Go!"));
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEPFRAME), TRUE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEPINTO), TRUE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEP), TRUE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_SKIP), TRUE);

	// Activate message pump
	BreakpointMessagePump();
}

void BreakpointHitRead(unsigned int address, int breakpoint)
{
	TCHAR szBuffer[256] = _T("");
	_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;\\red0\\green0\\blue255;}\\pard\\f0\\fs20\\cf1 Breakpoint R%i! read <- %06x\\par}"), breakpoint, address);

	BreakpointProcessing(szBuffer);
}

void BreakpointHitFetch(unsigned int address, int breakpoint)
{
	TCHAR szBuffer[256] = _T("");

	if (breakpoint == 0) {

		// Remove step into breakpoint
		SekDbgSetBreakpointFetch(0, 0);

		_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;\\red0\\green0\\blue255;}\\pard\\f0\\fs20\\cf1 Step complete! PC == %06x\\par}"), address);
	} else {
		_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;\\red0\\green0\\blue255;}\\pard\\f0\\fs20\\cf1 Breakpoint P%i! PC == %06x\\par}"), breakpoint, address);
	}

	BreakpointProcessing(szBuffer);
}

void BreakpointHitWrite(unsigned int address, int breakpoint)
{
	TCHAR szBuffer[256] = _T("");
	_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;\\red0\\green0\\blue255;}\\pard\\f0\\fs20\\cf1 Breakpoint W%i! write -> %06x\\par}"), breakpoint, address);

	BreakpointProcessing(szBuffer);
}

// ----------------------------------------------------------------------------
// Command parsing

static int cmd_exit(TCHAR* /*arg*/)
{
	SendMessage(hDbgDlg, WM_CLOSE, 0, 0);

	return 0;
}

static int cmd_disass(TCHAR* arg)
{
	TCHAR* pos = arg;
	long address;

	// Check if we need to write to a file
	if (*pos == _T('>')) {
		unsigned int start, end;
		TCHAR filename[MAX_PATH] = _T("");
		TCHAR* fn;

		pos++;

		SKIP_WS(pos);

		fn = pos;

		if (*pos == _T('\"')) {
			fn++;
			pos++;
			FIND_QT(pos);
			_tcsncpy(filename, fn, pos - fn);
			pos++;
		} else {
			FIND_WS(pos);
			_tcsncpy(filename, fn, pos - fn);
		}

		start = _tcstol(pos, &pos, 16);
		end = _tcstol(pos, &pos, 16);

		CreateDisassFile(filename, start, end, nDisassCPUType);

		return 0;
	}

	// Just set the active window when no arguments are specified
	if (*pos == _T('\0')) {
		nActiveWindow = nDisassWindow;

		return 0;
	}

	// Check for keywords
	if (!_tcsicmp(pos, _T("pc"))) {
		CreateDisass(GetDlgItem(hDbgDlg, nDisassWindow), TraceBack(SekDbgGetRegister(SEK_REG_PC), 5, nDisassCPUType), nDisassCPUType);

		nActiveWindow = nDisassWindow;

		return 0;
	}

	// Disassemble to the window
	address = _tcstol(pos, &pos, 16);
	if (address > 0x00FFFFFE || address < 0) {
		return 1;
	}

	CreateDisass(GetDlgItem(hDbgDlg, nDisassWindow), address & 0xFFFFFE, nDisassCPUType);

	nActiveWindow = nDisassWindow;

	return 0;
}

static int cmd_memdump(TCHAR* arg)
{
	TCHAR* pos = arg;
	long address = 0;

	// Just set the active window when no arguments are specified
	if (*pos == _T('\0')) {
		nActiveWindow = nMemdumpWindow;

		return 0;
	}

	// Check for keywords
	if (!_tcsicmp(pos, _T("pc"))) {
		address = SekDbgGetRegister(SEK_REG_PC) - 5 * 16;
	}
	if (!_tcsicmp(pos, _T("sp"))) {
		address = SekDbgGetRegister(SEK_REG_SP);
	}

	if (!address) {
		address = _tcstol(pos, &pos, 16);
	}

	if (address > 0x00FFFFFE || address < 0) {
		return 1;
	}

	nMemdumpAddress = address;
	CreateMemdump(GetDlgItem(hDbgDlg, nMemdumpWindow), nMemdumpArea, nMemdumpAddress);

	nActiveWindow = nMemdumpWindow;

	return 0;
}

static int cmd_go(TCHAR* arg)
{
	TCHAR szBuffer[256];
	TCHAR* pos = arg;
	long address = _tcstol(pos, &pos, 16);

	curr_m68000 = prev_m68000;
	if (address) {
		SekDbgSetRegister(SEK_REG_PC, address);
	}

	SetWindowText(GetDlgItem(hDbgDlg, IDC_DBG_GO), _T("Halt"));
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEPFRAME), FALSE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEPINTO), FALSE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEP), FALSE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_SKIP), FALSE);

	// Update the feedback area
	_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;}\\pard\\f0\\fs20\\cf0 Running, start at PC = %06x\\par}"), SekDbgGetRegister(SEK_REG_PC));
	AddFeedback(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN2), szBuffer);

	if (bRunPause) {
		SetPauseMode(false);
	}
	if (bBreakpointHit) {
		PostMessage(hScrnWnd, WM_APP + 0, 0, 0);
	}

	return 0;
}

static int cmd_halt(TCHAR* /*arg*/)
{
	TCHAR szBuffer[256];

	// remove temporary breakpoint (if any)
	SekDbgSetBreakpointFetch(0, 0);

	SetWindowText(GetDlgItem(hDbgDlg, IDC_DBG_GO), _T("Go!"));
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEPFRAME), TRUE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEPINTO), TRUE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_STEP), TRUE);
	EnableWindow(GetDlgItem(hDbgDlg, IDC_DBG_SKIP), TRUE);

	if (!bRunPause) {
		SetPauseMode(true);

		SekOpen(0);

		CreateDisass(GetDlgItem(hDbgDlg, nDisassWindow), TraceBack(SekGetPC(-1), 5, nDisassCPUType), nDisassCPUType);
		CreateCPUInfo(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN1), true, &curr_m68000, &prev_m68000);

		SekClose();
	}

	// Update the feedback area
	_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;}\\pard\\f0\\fs20\\cf0 Stopped, PC = %06x\\par}"), SekDbgGetRegister(SEK_REG_PC));
	AddFeedback(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN2), szBuffer);

	return 0;
}

static int cmd_setbp(TCHAR* arg)
{
	TCHAR* pos = arg;
	TCHAR szBuffer[256] = _T("");

	long number, address;

	SKIP_WS(pos);

	switch (*pos) {
		case _T('r'): {
			number = _tcstol(pos + 1, &pos, 16);
			address = _tcstol(pos, &pos, 16);

			if (number < 1 || number > 8) {
				return 1;
			}

			if (address) {
				_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;}\\pard\\f0\\fs20\\cf0 Breakpoint R%i: read <- %06x\\par}"), number, address);
			} else {
				_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;}\\pard\\f0\\fs20\\cf0 Breakpoint R%i: disabled\\par}"), number);
			}

			SekDbgSetBreakpointDataRead(address, number);

			break;
		}
		case _T('w'): {
			number = _tcstol(pos + 1, &pos, 16);
			address = _tcstol(pos, &pos, 16);

			if (number < 1 || number > 8) {
				return 1;
			}

			if (address) {
				_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;}\\pard\\f0\\fs20\\cf0 Breakpoint W%i: write -> %06x\\par}"), number, address);
			} else {
				_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;}\\pard\\f0\\fs20\\cf0 Breakpoint W%i: disabled\\par}"), number);
			}

			SekDbgSetBreakpointDataWrite(address, number);

			break;
		}
		case _T('p'): {
			number = _tcstol(pos + 1, &pos, 16);
			address = _tcstol(pos, &pos, 16);

			if (number < 1 || number > 8) {
				return 1;
			}

			if (address) {
				_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;}\\pard\\f0\\fs20\\cf0 Breakpoint P%i: PC == %06x\\par}"), number, address);
			} else {
				_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252{\\colortbl;}\\pard\\f0\\fs20\\cf0 Breakpoint P%i: disabled\\par}"), number);
			}

			SekDbgSetBreakpointFetch(address, number);

			break;
		}
	}

	// Update the feedback area
	AddFeedback(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN2), szBuffer);

	return 0;
}

static int cmd_frame(TCHAR* /*arg*/)
{
	TCHAR szBuffer[256] = _T("");

	SetPauseMode(true);

	prev_m68000 = curr_m68000;

	// Run until the next frame starts
	if (bBreakpointHit) {
		PostMessage(hScrnWnd, WM_APP + 0, 0, 0);
		return 0;
	} else {
		VidFrame();
	}

	// Display the image
	VidPaint(0);

	// Update the cpu info & disassembly
	SekOpen(0);
	CreateDisass(GetDlgItem(hDbgDlg, nDisassWindow), TraceBack(SekGetPC(-1), 5, nDisassCPUType), nDisassCPUType);
	CreateCPUInfo(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN1), true, &curr_m68000, &prev_m68000);
	SekClose();

	// Update the feedback area
	_stprintf(szBuffer, _T("{\\rtf1\\ansi\\ansicpg1252\\pard\\f0\\fs20\\cf0 Stepped frame, PC = %06x\\par}"), SekDbgGetRegister(SEK_REG_PC));
	AddFeedback(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN2), szBuffer);

	return 0;
}

static int cmd_single(TCHAR* /*arg*/)
{
	char buf[100] = "";

	int size = m68k_disassemble(buf, SekDbgGetRegister(SEK_REG_PC), nDisassCPUType);

	SekDbgSetBreakpointFetch(SekDbgGetRegister(SEK_REG_PC) + size, 0);

	return cmd_go(_T("0"));
}

static int cmd_into(TCHAR* /*arg*/)
{
	SekDbgEnableSingleStep();

	return cmd_go(_T("0"));
}

static int cmd_skip(TCHAR* /*arg*/)
{
	char buf[100] = "";

	int size = m68k_disassemble(buf, SekDbgGetRegister(SEK_REG_PC), nDisassCPUType);

	prev_m68000 = curr_m68000;

	SekDbgSetRegister(SEK_REG_PC, SekDbgGetRegister(SEK_REG_PC) + size);
	// Update active CPU context
	SekClose();

	CreateCPUInfo(GetDlgItem(hDbgDlg, IDC_DBG_INFOWN1), true, &curr_m68000, &prev_m68000);
	CreateDisass(GetDlgItem(hDbgDlg, nDisassWindow), TraceBack(SekDbgGetRegister(SEK_REG_PC), 5, nDisassCPUType), nDisassCPUType);

	return 0;
}

static struct { TCHAR* cmdstring; int (*cmdfunction)(TCHAR*); } CommandList[] = {
	{ _T(""),		NULL },

	{ _T("x"),		cmd_exit	}, { _T("exit"),		cmd_exit	},
	{ _T("d"),		cmd_disass	}, { _T("dasm"),		cmd_disass	},
	{ _T("m"),		cmd_memdump	}, { _T("mem"),			cmd_memdump	},

	{ _T("g"),		cmd_go		}, { _T("go"),			cmd_go		}, { _T("go!"),		cmd_go		},
	{ _T("h"),		cmd_halt	}, { _T("halt"),		cmd_halt	},

	{ _T("ss"),		cmd_single	}, { _T("step"),		cmd_single	},
	{ _T("si"),		cmd_into	}, { _T("step into"),	cmd_into	},
	{ _T("sk"),		cmd_skip	}, { _T("skip"),		cmd_skip	},

	{ _T("sf"),		cmd_frame	}, { _T("step frame"),	cmd_frame	},

	{ _T("sb"),		cmd_setbp	},

	{ NULL,		NULL }
};

static int ExecuteCommand(TCHAR* cmd)
{
	TCHAR cmdonly[8] = _T("");
	TCHAR* pos = cmd;

	SKIP_WS(pos);

	for (int i = 0; *pos && !isspace(*pos); i++) {
		cmdonly[i] = *pos++;
	}

	SKIP_WS(pos);

	for (int i = 0; CommandList[i].cmdstring; i++) {
		if (!_tcsicmp(cmdonly, CommandList[i].cmdstring)) {
			if (CommandList[i].cmdfunction) {
				return CommandList[i].cmdfunction(pos);
			}

			return 0;
		}
	}

	return 1;
}

// ----------------------------------------------------------------------------
// Message processing

static INT_PTR CALLBACK DialogProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_INITDIALOG) {

		hDbgDlg = hDlg;

		nDisassWindow = IDC_DBG_MAINWN1;
		nMemdumpWindow = IDC_DBG_MAINWN2;

		nActiveWindow = nDisassWindow;

		SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nMouseWheelScrollLines, 0);

		SendDlgItemMessage(hDlg, IDC_DBG_COMMAND, EM_SETTEXTMODE, TM_PLAINTEXT, 0);
		SendDlgItemMessage(hDlg, IDC_DBG_COMMAND, EM_LIMITTEXT, 63, 0);

		hCommandFont = CreateFont(bLargeWindow ? 18 : 14, 0, 0, 0, FW_NORMAL, 0, 0, 0, 0, 0, 0, ANTIALIASED_QUALITY, FF_MODERN, _T("Courier New"));
		SendDlgItemMessage(hDlg, IDC_DBG_COMMAND, WM_SETFONT, (WPARAM)hCommandFont, (LPARAM)0);

		SendDlgItemMessage(hDlg, IDC_DBG_MAINWN1, EM_SETMARGINS, EC_LEFTMARGIN, 3);
		SendDlgItemMessage(hDlg, IDC_DBG_MAINWN2, EM_SETMARGINS, EC_LEFTMARGIN, 3);
		SendDlgItemMessage(hDlg, IDC_DBG_INFOWN1, EM_SETMARGINS, EC_LEFTMARGIN, 3);
		SendDlgItemMessage(hDlg, IDC_DBG_INFOWN2, EM_SETMARGINS, EC_LEFTMARGIN, 3);
		SendDlgItemMessage(hDlg, IDC_DBG_COMMAND, EM_SETMARGINS, EC_LEFTMARGIN, 3);

		SendDlgItemMessage(hDlg, IDC_DBG_MAINWN1, EM_SETEVENTMASK, 0, ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_SCROLLEVENTS | ENM_PROTECTED);
		SendDlgItemMessage(hDlg, IDC_DBG_MAINWN2, EM_SETEVENTMASK, 0, ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_SCROLLEVENTS | ENM_PROTECTED);
		SendDlgItemMessage(hDlg, IDC_DBG_INFOWN1, EM_SETEVENTMASK, 0, ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_SCROLLEVENTS | ENM_PROTECTED);
		SendDlgItemMessage(hDlg, IDC_DBG_INFOWN2, EM_SETEVENTMASK, 0, ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_SCROLLEVENTS | ENM_PROTECTED);
		SendDlgItemMessage(hDlg, IDC_DBG_COMMAND, EM_SETEVENTMASK, 0, ENM_KEYEVENTS | ENM_MOUSEEVENTS | ENM_SCROLLEVENTS);

		// Initialise feedback window
		InitFeedback(GetDlgItem(hDlg, IDC_DBG_INFOWN2));

		// Get the list of memory areas
		GetMemoryAreas();

		// Initialise disassembly window
		SekOpen(0);
		SetupDisass(0);

		// Initialise memory dump window
		SetupMemdump(0);

		UpdateCPUState(&prev_m68000);
		CreateCPUInfo(GetDlgItem(hDlg, IDC_DBG_INFOWN1), true, &curr_m68000, &prev_m68000);

		SekClose();

		SendDlgItemMessage(hDlg, IDC_DBG_MAINWN1_DRP1, CB_SETCURSEL, (WPARAM)0, (LPARAM)0);

		SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDC_DBG_COMMAND), TRUE);

		WndInMid(hDlg, NULL);
		ShowWindow(hDlg, SW_NORMAL);

		return FALSE;
	}

	if (Msg == WM_CLOSE) {
		DebugExit();

		return 0;
	}

	if (Msg == WM_COMMAND) {

		// Exit the debugger
		if (LOWORD(wParam) == IDCANCEL && HIWORD(wParam) == BN_CLICKED) {
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return 0;
		}

		if (LOWORD(wParam) == IDC_DBG_GO && HIWORD(wParam) == BN_CLICKED) {
			if (bRunPause || bBreakpointHit) {
				ExecuteCommand(_T("g"));
			} else {
				ExecuteCommand(_T("h"));
			}
			return 0;
		}

		if (LOWORD(wParam) == IDC_DBG_STEP && HIWORD(wParam) == BN_CLICKED) {
			ExecuteCommand(_T("ss"));
			return 0;
		}
		if (LOWORD(wParam) == IDC_DBG_STEPINTO && HIWORD(wParam) == BN_CLICKED) {
			ExecuteCommand(_T("si"));
			return 0;
		}
		if (LOWORD(wParam) == IDC_DBG_SKIP && HIWORD(wParam) == BN_CLICKED) {
			ExecuteCommand(_T("sk"));
			return 0;
		}

		if (LOWORD(wParam) == IDC_DBG_STEPFRAME && HIWORD(wParam) == BN_CLICKED) {
			ExecuteCommand(_T("sf"));
			return 0;
		}
		if (LOWORD(wParam) == IDC_DBG_SHOT && HIWORD(wParam) == BN_CLICKED) {
			MakeScreenShot();
			return 0;
		}

		// Execute the comnmand entered in the command text control
		if (LOWORD(wParam) == IDOK) {
			TCHAR CommandText[64] = _T("");
			SendDlgItemMessage(hDlg, IDC_DBG_COMMAND, WM_GETTEXT, (WPARAM)64, (LPARAM)CommandText);
			SendDlgItemMessage(hDlg, IDC_DBG_COMMAND, WM_SETTEXT, (WPARAM)0, (LPARAM)_T(""));

			ExecuteCommand(CommandText);

			SendMessage(hDlg, WM_NEXTDLGCTL, (WPARAM)GetDlgItem(hDlg, IDC_DBG_COMMAND), TRUE);
		}

		if (HIWORD(wParam) == CBN_SELENDOK) {
			if (LOWORD(wParam) == IDC_DBG_MAINWN1_DRP1) {
				SetupDisass(SendMessage((HWND)lParam, CB_GETCURSEL, (WPARAM)0, (LPARAM)0));
			}
			if (LOWORD(wParam) == IDC_DBG_MAINWN1_DRP2) {
				nDisassAttrib = SendMessage((HWND)lParam, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				CreateDisass(GetDlgItem(hDlg, nDisassWindow), nDisassAddress[0], nDisassCPUType);
			}
			if (LOWORD(wParam) == IDC_DBG_MAINWN2_DRP1) {
				SetupMemdump(SendMessage((HWND)lParam, CB_GETCURSEL, (WPARAM)0, (LPARAM)0));
			}
			if (LOWORD(wParam) == IDC_DBG_MAINWN2_DRP2) {
				nMemdumpAttrib = SendMessage((HWND)lParam, CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
				CreateMemdump(GetDlgItem(hDlg, nMemdumpWindow), nMemdumpArea, nMemdumpAddress);
			}
		}
	}

	if (Msg == WM_NOTIFY) {
		int nPageSize = bLargeWindow ? 16 : 12;
		unsigned int nWindow = 0;
		bool bAffectCaret = true;
		int nScrollLines = 0;

		if (((NMHDR*)lParam)->code == EN_MSGFILTER) {
			nWindow = ((NMHDR*)lParam)->idFrom;

			if (nWindow == IDC_DBG_COMMAND) {
				nWindow = nActiveWindow;

				bAffectCaret = false;
			}
		}

		if (((NMHDR*)lParam)->code == EN_MSGFILTER && ((MSGFILTER*)lParam)->msg == WM_MOUSEWHEEL) {
			static int nMouseWheelDelta;

//			if (wParam & (MK_SHIFT | MK_CONTROL)) {
//				break;
//			}

			nMouseWheelDelta -= (short)HIWORD(((MSGFILTER*)lParam)->wParam);

			if (abs(nMouseWheelDelta) >= WHEEL_DELTA) {
				if (nMouseWheelScrollLines == WHEEL_PAGESCROLL) {
					nScrollLines = 2 * (nPageSize - ((nWindow == nDisassWindow) ? 1 : 0));
					if (nMouseWheelDelta < 0) {
						nScrollLines = -nScrollLines;
					}
				} else {
					nScrollLines = 2 * (nMouseWheelDelta / WHEEL_DELTA);
				}
				nMouseWheelDelta %= WHEEL_DELTA;

				bAffectCaret = false;
			}
		}

		if ((bBreakpointHit || bRunPause) && ((NMHDR*)lParam)->code == EN_MSGFILTER && (((MSGFILTER*)lParam)->msg == WM_KEYDOWN || ((MSGFILTER*)lParam)->msg == WM_SYSKEYDOWN)) {

			if (((NMHDR*)lParam)->idFrom != IDC_DBG_COMMAND && ((NMHDR*)lParam)->idFrom != nDisassWindow && ((NMHDR*)lParam)->idFrom != nMemdumpWindow) {
				nWindow = 0;
			}

			if (((MSGFILTER*)lParam)->wParam == VK_UP) {
				if (KEY_DOWN(VK_CONTROL)) {
					nScrollLines = -1;
				} else {
					nScrollLines = -2;
				}
			}
			if (((MSGFILTER*)lParam)->wParam == VK_DOWN) {
				if (KEY_DOWN(VK_CONTROL)) {
					nScrollLines = 1;
				} else {
					nScrollLines = 2;
				}
			}
			if (((MSGFILTER*)lParam)->wParam == VK_PRIOR) {
				nScrollLines = -2 * (nPageSize - ((nWindow == nDisassWindow) ? 1 : 0));
			}
			if (((MSGFILTER*)lParam)->wParam == VK_NEXT) {
				nScrollLines = 2 * (nPageSize - ((nWindow == nDisassWindow) ? 1 : 0));
			}
		}

		if (nScrollLines && nWindow) {
			DWORD dwStart = 0, dwEnd = 0;
			bool bResetCaret = false;

			if (nWindow == nDisassWindow) {

				SendDlgItemMessage(hDlg, nWindow, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

				if (nScrollLines < 0) {
					int nLine = SendDlgItemMessage(hDlg, nWindow, EM_LINEFROMCHAR, (WPARAM)((dwStart != (DWORD)-1) ? dwStart : dwEnd), (LPARAM)0);

					if (nScrollLines & 1) {
						CreateDisass(GetDlgItem(hDlg, nWindow), nDisassAddress[0] < 2 ? 0 : nDisassAddress[0] - 2, nDisassCPUType);

						dwStart = dwEnd = 2 * nPageSize + SendDlgItemMessage(hDlg, nWindow, EM_LINEINDEX, nLine, 0);
						bResetCaret = true;
					} else {
						if (!bAffectCaret || nScrollLines < -2 || nLine == 0) {
							CreateDisass(GetDlgItem(hDlg, nWindow), TraceBack(nDisassAddress[0], -nScrollLines / 2, nDisassCPUType), nDisassCPUType);

							if (bAffectCaret) {
								dwStart = dwEnd = 2 * nPageSize;
							}
							bResetCaret = true;
						}
					}
				}
				if (nScrollLines > 0) {
					int nLine = SendDlgItemMessage(hDlg, nWindow, EM_LINEFROMCHAR, (WPARAM)dwEnd, (LPARAM)0);

					if (nScrollLines & 1) {
						CreateDisass(GetDlgItem(hDlg, nWindow), nDisassAddress[0] >= 0x00FFFFFE ? 0x00FFFFFE : nDisassAddress[0] + 2, nDisassCPUType);

						dwStart = dwEnd = 32 + SendDlgItemMessage(hDlg, nWindow, EM_LINEINDEX, nLine, 0);
						bResetCaret = true;
					} else {
						if (!bAffectCaret || nScrollLines > 2 || SendDlgItemMessage(hDlg, nWindow, EM_LINEFROMCHAR, (WPARAM)dwEnd, (LPARAM)0) >= (nPageSize - 1)) {

							// Make sure we don't go past the end of memory
							if (nDisassAddress[0] >= 0x00FFFFFE) {
								nDisassAddress[1] = 0x00FFFFFE;
							}

							CreateDisass(GetDlgItem(hDlg, nWindow), nDisassAddress[nScrollLines / 2], nDisassCPUType);

							if (bAffectCaret) {
								dwStart = dwEnd = 32 + SendDlgItemMessage(hDlg, nWindow, EM_LINEINDEX, nPageSize - 1, 0);
							}
							bResetCaret = true;
						}
					}
				}
			}

			if (nWindow == nMemdumpWindow) {
				int nAreaSize = nMemdumpArea ? DbgMemoryAreaInfo[nMemdumpArea].nLen : 0x01000000;
				bool bRefresh = true;

				SendDlgItemMessage(hDlg, nWindow, EM_GETSEL, (WPARAM)&dwStart, (LPARAM)&dwEnd);

				if (nScrollLines < 0) {
					if (nScrollLines & 1) {
						nMemdumpAddress -= 1;
					} else {
						if (!bAffectCaret || nScrollLines < -2 || SendDlgItemMessage(hDlg, nWindow, EM_LINEFROMCHAR, (WPARAM)dwStart, (LPARAM)0) == 0) {
							nMemdumpAddress += 8 * nScrollLines;
						} else {
							bRefresh = false;
						}
					}
				}
				if (nScrollLines > 0) {
					if (nScrollLines & 1) {
						nMemdumpAddress += 1;
					} else {
						if (!bAffectCaret || nScrollLines > 2 || SendDlgItemMessage(hDlg, nWindow, EM_LINEFROMCHAR, (WPARAM)dwEnd, (LPARAM)0) >= nPageSize - 1) {
							nMemdumpAddress += 8 * nScrollLines;
						} else {
							bRefresh = false;
						}
					}
				}

				if (bRefresh) {
					// Make sure we don't go past the end of memory
					if (nMemdumpAddress > nAreaSize - nPageSize * 16) {
						nMemdumpAddress = nAreaSize - nPageSize * 16;
					}
					if (nMemdumpAddress < 0) {
						nMemdumpAddress = 0;
					}

					CreateMemdump(GetDlgItem(hDlg, nWindow), nMemdumpArea, nMemdumpAddress);
					bResetCaret = true;
				}
			}

			// If we've re-filled the RichEdit control, the caret position is reset as well, so restore it
			if (bResetCaret) {
				SendDlgItemMessage(hDlg, nWindow, EM_SETSEL, (WPARAM)dwStart, (LPARAM)dwEnd);
			}
		}
	}

	return 0;
}

// ----------------------------------------------------------------------------
// Public functions

#endif

int DebugExit()
{

#if defined (FBA_DEBUG)

	if (DbgMemoryAreaInfo) {
		free(DbgMemoryAreaInfo);
		DbgMemoryAreaInfo = NULL;
	}

	EnableWindow(hScrnWnd, TRUE);
	DestroyWindow(hDbgDlg);

	DeleteObject(hCommandFont);
	hCommandFont = NULL;

	SekDbgDisableBreakpoints();
	if (bBreakpointHit) {
		bBreakpointHit = false;
		POST_INITIALISE_MESSAGE;
	}

	SetPauseMode(bOldPause);
	MenuUpdate();

#endif

	return 0;
}

int DebugCreate()
{

#if defined (FBA_DEBUG)

	if (DebugCPU_SekInitted == 0) {
		return 1;
	}

	if (bDrvOkay == 0) {
		return 1;
	}

	AudBlankSound();

	DestroyWindow(hDbgDlg);

	SystemParametersInfo(SPI_GETWORKAREA, 0, &SystemWorkArea, 0);
	bLargeWindow = (SystemWorkArea.right - SystemWorkArea.left >= 1024 && SystemWorkArea.bottom - SystemWorkArea.top >= 768) ? true : false;
	hDbgDlg = FBACreateDialog(hAppInst, MAKEINTRESOURCE(bLargeWindow ? IDD_DEBUG_LRG : IDD_DEBUG_SML), hScrnWnd, (DLGPROC)DialogProc);
	if (hDbgDlg == NULL) {
		return 1;
	}

	SekDbgBreakpointHandlerRead = BreakpointHitRead;
	SekDbgBreakpointHandlerFetch = BreakpointHitFetch;
	SekDbgBreakpointHandlerWrite = BreakpointHitWrite;

	bOldPause = bRunPause;

	SetPauseMode(true);

	return 0;

#else

	POST_INITIALISE_MESSAGE;

	return 1;

#endif

}
