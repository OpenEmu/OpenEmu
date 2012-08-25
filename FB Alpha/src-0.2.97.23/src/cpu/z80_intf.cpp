// Z80 (Zed Eight-Ty) Interface
#include "burnint.h"
#include "z80_intf.h"

#define MAX_Z80		8
static struct ZetExt * ZetCPUContext[MAX_Z80] = { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };
 
typedef UINT8 (__fastcall *pZetInHandler)(UINT16 a);
typedef void (__fastcall *pZetOutHandler)(UINT16 a, UINT8 d);
typedef UINT8 (__fastcall *pZetReadHandler)(UINT16 a);
typedef void (__fastcall *pZetWriteHandler)(UINT16 a, UINT8 d);
 
struct ZetExt {
	Z80_Regs reg;
	
	UINT8* pZetMemMap[0x100 * 4];

	pZetInHandler ZetIn;
	pZetOutHandler ZetOut;
	pZetReadHandler ZetRead;
	pZetWriteHandler ZetWrite;
	
	UINT8 BusReq;
};
 
static INT32 nZetCyclesDone[MAX_Z80];
static INT32 nZetCyclesTotal;
static INT32 nZ80ICount[MAX_Z80];
static UINT32 Z80EA[MAX_Z80];

static INT32 nOpenedCPU = -1;
static INT32 nCPUCount = 0;
INT32 nHasZet = -1;

UINT8 __fastcall ZetDummyReadHandler(UINT16) { return 0; }
void __fastcall ZetDummyWriteHandler(UINT16, UINT8) { }
UINT8 __fastcall ZetDummyInHandler(UINT16) { return 0; }
void __fastcall ZetDummyOutHandler(UINT16, UINT8) { }

UINT8 __fastcall ZetReadIO(UINT32 a)
{
	return ZetCPUContext[nOpenedCPU]->ZetIn(a);
}

void __fastcall ZetWriteIO(UINT32 a, UINT8 d)
{
	ZetCPUContext[nOpenedCPU]->ZetOut(a, d);
}

UINT8 __fastcall ZetReadProg(UINT32 a)
{
	// check mem map
	UINT8 * pr = ZetCPUContext[nOpenedCPU]->pZetMemMap[0x000 | (a >> 8)];
	if (pr != NULL) {
		return pr[a & 0xff];
	}
	
	// check handler
	if (ZetCPUContext[nOpenedCPU]->ZetRead != NULL) {
		return ZetCPUContext[nOpenedCPU]->ZetRead(a);
	}
	
	return 0;
}

void __fastcall ZetWriteProg(UINT32 a, UINT8 d)
{
	// check mem map
	UINT8 * pr = ZetCPUContext[nOpenedCPU]->pZetMemMap[0x100 | (a >> 8)];
	if (pr != NULL) {
		pr[a & 0xff] = d;
		return;
	}
	
	// check handler
	if (ZetCPUContext[nOpenedCPU]->ZetWrite != NULL) {
		ZetCPUContext[nOpenedCPU]->ZetWrite(a, d);
		return;
	}
}

UINT8 __fastcall ZetReadOp(UINT32 a)
{
	// check mem map
	UINT8 * pr = ZetCPUContext[nOpenedCPU]->pZetMemMap[0x200 | (a >> 8)];
	if (pr != NULL) {
		return pr[a & 0xff];
	}
	
	// check read handler
	if (ZetCPUContext[nOpenedCPU]->ZetRead != NULL) {
		return ZetCPUContext[nOpenedCPU]->ZetRead(a);
	}
	
	return 0;
}

UINT8 __fastcall ZetReadOpArg(UINT32 a)
{
	// check mem map
	UINT8 * pr = ZetCPUContext[nOpenedCPU]->pZetMemMap[0x300 | (a >> 8)];
	if (pr != NULL) {
		return pr[a & 0xff];
	}
	
	// check read handler
	if (ZetCPUContext[nOpenedCPU]->ZetRead != NULL) {
		return ZetCPUContext[nOpenedCPU]->ZetRead(a);
	}
	
	return 0;
}

void ZetSetReadHandler(UINT8 (__fastcall *pHandler)(UINT16))
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetSetReadHandler called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetSetReadHandler called when no CPU open\n"));
#endif

	ZetCPUContext[nOpenedCPU]->ZetRead = pHandler;
}

void ZetSetWriteHandler(void (__fastcall *pHandler)(UINT16, UINT8))
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetSetWriteHandler called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetSetWriteHandler called when no CPU open\n"));
#endif

	ZetCPUContext[nOpenedCPU]->ZetWrite = pHandler;
}

void ZetSetInHandler(UINT8 (__fastcall *pHandler)(UINT16))
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetSetInHandler called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetSetInHandler called when no CPU open\n"));
#endif

	ZetCPUContext[nOpenedCPU]->ZetIn = pHandler;
}

void ZetSetOutHandler(void (__fastcall *pHandler)(UINT16, UINT8))
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetSetOutHandler called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetSetOutHandler called when no CPU open\n"));
#endif

	ZetCPUContext[nOpenedCPU]->ZetOut = pHandler;
}

void ZetNewFrame()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetNewFrame called without init\n"));
#endif

	for (INT32 i = 0; i < nCPUCount; i++) {
		nZetCyclesDone[i] = 0;
	}
	nZetCyclesTotal = 0;
}

static void ZetCheatWriteROM(UINT32 a, UINT8 d)
{
	ZetWriteRom(a, d);
}

static UINT8 ZetCheatRead(UINT32 a)
{
	return ZetReadByte(a);
}

static cpu_core_config ZetCheatCpuConfig =
{
	ZetOpen,
	ZetClose,
	ZetCheatRead,
	ZetCheatWriteROM,
	ZetGetActive,
	ZetTotalCycles,
	ZetNewFrame,
	ZetRun,
	ZetRunEnd,
	ZetReset,
	(1<<16),	// 0x10000
	0
};

INT32 ZetInit(INT32 nCPU)
{
	DebugCPU_ZetInitted = 1;

	nOpenedCPU = -1;

	ZetCPUContext[nCPU] = (struct ZetExt*)BurnMalloc(sizeof(ZetExt));
	memset (ZetCPUContext[nCPU], 0, sizeof(ZetExt));

	if (nCPU == 0) { // not safe!
		Z80Init();
	}

	{
		ZetCPUContext[nCPU]->ZetIn = ZetDummyInHandler;
		ZetCPUContext[nCPU]->ZetOut = ZetDummyOutHandler;
		ZetCPUContext[nCPU]->ZetRead = ZetDummyReadHandler;
		ZetCPUContext[nCPU]->ZetWrite = ZetDummyWriteHandler;
		ZetCPUContext[nCPU]->BusReq = 0;
		// TODO: Z80Init() will set IX IY F regs with default value, so get them ...
		Z80GetContext(&ZetCPUContext[nCPU]->reg);
		
		nZetCyclesDone[nCPU] = 0;
		nZ80ICount[nCPU] = 0;
		
		for (INT32 j = 0; j < (0x0100 * 4); j++) {
			ZetCPUContext[nCPU]->pZetMemMap[j] = NULL;
		}
	}

	nZetCyclesTotal = 0;

	Z80SetIOReadHandler(ZetReadIO);
	Z80SetIOWriteHandler(ZetWriteIO);
	Z80SetProgramReadHandler(ZetReadProg);
	Z80SetProgramWriteHandler(ZetWriteProg);
	Z80SetCPUOpReadHandler(ZetReadOp);
	Z80SetCPUOpArgReadHandler(ZetReadOpArg);
	
	nCPUCount = (nCPU+1) % MAX_Z80;

	nHasZet = nCPU+1;

	CpuCheatRegister(nCPU, &ZetCheatCpuConfig);

	return 0;
}

#if 0
INT32 ZetInit(INT32 nCount)
{
	DebugCPU_ZetInitted = 1;

	nOpenedCPU = -1;
	
	ZetCPUContext = (struct ZetExt *) malloc(nCount * sizeof(ZetExt));
	if (ZetCPUContext == NULL) return 1;
	memset(ZetCPUContext, 0, nCount * sizeof(ZetExt));
	
	Z80Init();
	
	for (INT32 i = 0; i < nCount; i++) {
		ZetCPUContext[i].ZetIn = ZetDummyInHandler;
		ZetCPUContext[i].ZetOut = ZetDummyOutHandler;
		ZetCPUContext[i].ZetRead = ZetDummyReadHandler;
		ZetCPUContext[i].ZetWrite = ZetDummyWriteHandler;
		ZetCPUContext[i].BusReq = 0;
		// TODO: Z80Init() will set IX IY F regs with default value, so get them ...
		Z80GetContext(&ZetCPUContext[i].reg);
		
		nZetCyclesDone[i] = 0;
		nZ80ICount[i] = 0;
		
		for (INT32 j = 0; j < (0x0100 * 4); j++) {
			ZetCPUContext[i].pZetMemMap[j] = NULL;
		}
	}
	
	nZetCyclesTotal = 0;
	
	Z80SetIOReadHandler(ZetReadIO);
	Z80SetIOWriteHandler(ZetWriteIO);
	Z80SetProgramReadHandler(ZetReadProg);
	Z80SetProgramWriteHandler(ZetWriteProg);
	Z80SetCPUOpReadHandler(ZetReadOp);
	Z80SetCPUOpArgReadHandler(ZetReadOpArg);
	
	nCPUCount = nCount % MAX_Z80;

	nHasZet = nCount;

	for (INT32 i = 0; i < nCount; i++)
		CpuCheatRegister(0x0004, i);

	return 0;
}
#endif

UINT8 ZetReadByte(UINT16 address)
{
	if (nOpenedCPU < 0) return 0;

	return ZetReadProg(address);
}

void ZetWriteByte(UINT16 address, UINT8 data)
{
	if (nOpenedCPU < 0) return;

	ZetWriteProg(address, data);
}

void ZetWriteRom(UINT16 address, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetWriteRom called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetWriteRom called when no CPU open\n"));
#endif

	if (nOpenedCPU < 0) return;

	if (ZetCPUContext[nOpenedCPU]->pZetMemMap[0x200 | (address >> 8)] != NULL) {
		ZetCPUContext[nOpenedCPU]->pZetMemMap[0x200 | (address >> 8)][address] = data;
	}
	
	if (ZetCPUContext[nOpenedCPU]->pZetMemMap[0x300 | (address >> 8)] != NULL) {
		ZetCPUContext[nOpenedCPU]->pZetMemMap[0x300 | (address >> 8)][address] = data;
	}
	
	ZetWriteProg(address, data);
}

void ZetClose()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetClose called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetClose called when no CPU open\n"));
#endif

	Z80GetContext(&ZetCPUContext[nOpenedCPU]->reg);
	nZetCyclesDone[nOpenedCPU] = nZetCyclesTotal;
	nZ80ICount[nOpenedCPU] = z80_ICount;
	Z80EA[nOpenedCPU] = EA;

	nOpenedCPU = -1;
}

void ZetOpen(INT32 nCPU)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetOpen called without init\n"));
	if (nCPU >= nCPUCount) bprintf(PRINT_ERROR, _T("ZetOpen called with invalid index %x\n"), nCPU);
	if (nOpenedCPU != -1) bprintf(PRINT_ERROR, _T("ZetOpen called when CPU already open with index %x\n"), nCPU);
	if (ZetCPUContext[nCPU] == NULL) bprintf (PRINT_ERROR, _T("ZetOpen called for uninitialized cpu %x\n"), nCPU);
#endif

	Z80SetContext(&ZetCPUContext[nCPU]->reg);
	nZetCyclesTotal = nZetCyclesDone[nCPU];
	z80_ICount = nZ80ICount[nCPU];
	EA = Z80EA[nCPU];

	nOpenedCPU = nCPU;
}

INT32 ZetGetActive()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetGetActive called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetGetActive called when no CPU open\n"));
#endif

	return nOpenedCPU;
}

INT32 ZetRun(INT32 nCycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetRun called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetRun called when no CPU open\n"));
#endif

	if (nCycles <= 0) return 0;
	
	if (ZetCPUContext[nOpenedCPU]->BusReq) {
		nZetCyclesTotal += nCycles;
		return nCycles;
	}
	
	nCycles = Z80Execute(nCycles);
	
	nZetCyclesTotal += nCycles;
	
	return nCycles;
}

void ZetRunAdjust(INT32 /*nCycles*/)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetRunAdjust called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetRunAdjust called when no CPU open\n"));
#endif
}

void ZetRunEnd()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetRunEnd called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetRunEnd called when no CPU open\n"));
#endif
}

// This function will make an area callback ZetRead/ZetWrite
INT32 ZetMemCallback(INT32 nStart, INT32 nEnd, INT32 nMode)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetMemCallback called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetMemCallback called when no CPU open\n"));
#endif

	UINT8 cStart = (nStart >> 8);
	UINT8 **pMemMap = ZetCPUContext[nOpenedCPU]->pZetMemMap;

	for (UINT16 i = cStart; i <= (nEnd >> 8); i++) {
		switch (nMode) {
			case 0:
				pMemMap[0     + i] = NULL;
				break;
			case 1:
				pMemMap[0x100 + i] = NULL;
				break;
			case 2:
				pMemMap[0x200 + i] = NULL;
				pMemMap[0x300 + i] = NULL;
				break;
		}
	}

	return 0;
}

INT32 ZetMemEnd()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetMemEnd called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetMemEnd called when no CPU open\n"));
#endif

	return 0;
}

void ZetExit()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetExit called without init\n"));
#endif

	Z80Exit();

	for (INT32 i = 0; i < MAX_Z80; i++) {
		if (ZetCPUContext[i]) {
			BurnFree (ZetCPUContext[i]);
		}
	}

	nCPUCount = 0;
	nHasZet = -1;
	
	DebugCPU_ZetInitted = 0;
}


INT32 ZetMapArea(INT32 nStart, INT32 nEnd, INT32 nMode, UINT8 *Mem)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetMapArea called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetMapArea called when no CPU open\n"));
#endif

	UINT8 cStart = (nStart >> 8);
	UINT8 **pMemMap = ZetCPUContext[nOpenedCPU]->pZetMemMap;

	for (UINT16 i = cStart; i <= (nEnd >> 8); i++) {
		switch (nMode) {
			case 0: {
				pMemMap[0     + i] = Mem + ((i - cStart) << 8);
				break;
			}
		
			case 1: {
				pMemMap[0x100 + i] = Mem + ((i - cStart) << 8);
				break;
			}
			
			case 2: {
				pMemMap[0x200 + i] = Mem + ((i - cStart) << 8);
				pMemMap[0x300 + i] = Mem + ((i - cStart) << 8);
				break;
			}
		}
	}

	return 0;
}

INT32 ZetMapArea(INT32 nStart, INT32 nEnd, INT32 nMode, UINT8 *Mem01, UINT8 *Mem02)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetMapArea called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetMapArea called when no CPU open\n"));
#endif

	UINT8 cStart = (nStart >> 8);
	UINT8 **pMemMap = ZetCPUContext[nOpenedCPU]->pZetMemMap;
	
	if (nMode != 2) {
		return 1;
	}
	
	for (UINT16 i = cStart; i <= (nEnd >> 8); i++) {
		pMemMap[0x200 + i] = Mem01 + ((i - cStart) << 8);
		pMemMap[0x300 + i] = Mem02 + ((i - cStart) << 8);
	}

	return 0;
}

void ZetReset()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetReset called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetReset called when no CPU open\n"));
#endif

	Z80Reset();
}

INT32 ZetPc(INT32 n)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetPc called without init\n"));
	if (nOpenedCPU == -1 && n < 0) bprintf(PRINT_ERROR, _T("ZetPc called when no CPU open\n"));
#endif

	if (n < 0) {
		return ActiveZ80GetPC();
	} else {
		return ZetCPUContext[n]->reg.pc.w.l;
	}
}

INT32 ZetBc(INT32 n)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetBc called without init\n"));
	if (nOpenedCPU == -1 && n < 0) bprintf(PRINT_ERROR, _T("ZetBc called when no CPU open\n"));
#endif

	if (n < 0) {
		return ActiveZ80GetBC();
	} else {
		return ZetCPUContext[n]->reg.bc.w.l;
	}
}

INT32 ZetDe(INT32 n)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetDe called without init\n"));
	if (nOpenedCPU == -1 && n < 0) bprintf(PRINT_ERROR, _T("ZetDe called when no CPU open\n"));
#endif

	if (n < 0) {
		return ActiveZ80GetDE();
	} else {
		return ZetCPUContext[n]->reg.de.w.l;
	}
}

INT32 ZetHL(INT32 n)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetHL called without init\n"));
	if (nOpenedCPU == -1 && n < 0) bprintf(PRINT_ERROR, _T("ZetHL called when no CPU open\n"));
#endif

	if (n < 0) {
		return ActiveZ80GetHL();
	} else {
		return ZetCPUContext[n]->reg.hl.w.l;
	}
}

INT32 ZetScan(INT32 nAction)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetScan called without init\n"));
#endif

	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 0;
	}

	char szText[] = "Z80 #0";
	
	for (INT32 i = 0; i < nCPUCount; i++) {
		szText[5] = '1' + i;

		ScanVar(&ZetCPUContext[i]->reg, sizeof(Z80_Regs), szText);
		SCAN_VAR(Z80EA[i]);
		SCAN_VAR(nZ80ICount[i]);
		SCAN_VAR(nZetCyclesDone[i]);
	}
	
	SCAN_VAR(nZetCyclesTotal);	

	return 0;
}

void ZetSetIRQLine(const INT32 line, const INT32 status)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetSetIRQLine called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetSetIRQLine called when no CPU open\n"));
#endif

	switch ( status ) {
		case ZET_IRQSTATUS_NONE:
			Z80SetIrqLine(0, 0);
			break;
		case ZET_IRQSTATUS_ACK: 	
			Z80SetIrqLine(line, 1);
			break;
		case ZET_IRQSTATUS_AUTO:
			Z80SetIrqLine(line, 1);
			Z80Execute(0);
			Z80SetIrqLine(0, 0);
			Z80Execute(0);
			break;
	}
}

void ZetSetVector(INT32 vector)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetSetVector called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetSetVector called when no CPU open\n"));
#endif

	Z80Vector = vector;
}

INT32 ZetNmi()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetNmi called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetNmi called when no CPU open\n"));
#endif

	Z80SetIrqLine(Z80_INPUT_LINE_NMI, 1);
	Z80Execute(0);
	Z80SetIrqLine(Z80_INPUT_LINE_NMI, 0);
	Z80Execute(0);
	INT32 nCycles = 12;
	nZetCyclesTotal += nCycles;

	return nCycles;
}

INT32 ZetIdle(INT32 nCycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetIdle called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetIdle called when no CPU open\n"));
#endif

	nZetCyclesTotal += nCycles;

	return nCycles;
}

INT32 ZetSegmentCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetSegmentCycles called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetSegmentCycles called when no CPU open\n"));
#endif

	return z80TotalCycles();
}

INT32 ZetTotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetTotalCycles called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetTotalCycles called when no CPU open\n"));
#endif

	return nZetCyclesTotal + z80TotalCycles();
}

void ZetSetBUSREQLine(INT32 nStatus)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ZetInitted) bprintf(PRINT_ERROR, _T("ZetSetBUSREQLine called without init\n"));
	if (nOpenedCPU == -1) bprintf(PRINT_ERROR, _T("ZetSetBUSREQLine called when no CPU open\n"));
#endif

	if (nOpenedCPU < 0) return;
	
	ZetCPUContext[nOpenedCPU]->BusReq = nStatus;
}

#undef MAX_Z80
