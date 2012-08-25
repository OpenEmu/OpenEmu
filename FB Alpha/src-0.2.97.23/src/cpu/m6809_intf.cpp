#include "burnint.h"
#include "m6809_intf.h"

#define MAX_CPU		8

INT32 nM6809Count = 0;
static INT32 nActiveCPU = 0;

static M6809Ext *m6809CPUContext;

static INT32 nM6809CyclesDone[MAX_CPU];
INT32 nM6809CyclesTotal;

static UINT8 M6809ReadByteDummyHandler(UINT16)
{
	return 0;
}

static void M6809WriteByteDummyHandler(UINT16, UINT8)
{
}

static UINT8 M6809ReadOpDummyHandler(UINT16)
{
	return 0;
}

static UINT8 M6809ReadOpArgDummyHandler(UINT16)
{
	return 0;
}

void M6809Reset()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809Reset called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809Reset called when no CPU open\n"));
#endif

	m6809_reset();
}

void M6809NewFrame()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809NewFrame called without init\n"));
#endif

	for (INT32 i = 0; i < nM6809Count; i++) {
		nM6809CyclesDone[i] = 0;
	}
	nM6809CyclesTotal = 0;
}

static UINT8 M6809CheatRead(UINT32 a)
{
	return M6809ReadByte(a);
}

static cpu_core_config M6809CheatCpuConfig =
{
	M6809Open,
	M6809Close,
	M6809CheatRead,
	M6809WriteRom,
	M6809GetActive,
	M6809TotalCycles,
	M6809NewFrame,
	M6809Run,
	M6809RunEnd,
	M6809Reset,
	1<<16,
	0
};

INT32 M6809Init(INT32 num)
{
	DebugCPU_M6809Initted = 1;
	
	nActiveCPU = -1;
	nM6809Count = num % MAX_CPU;
	
	m6809CPUContext = (M6809Ext*)malloc(num * sizeof(M6809Ext));
	if (m6809CPUContext == NULL) {
		return 1;
	}

	memset(m6809CPUContext, 0, num * sizeof(M6809Ext));
	
	for (INT32 i = 0; i < num; i++) {
		m6809CPUContext[i].ReadByte = M6809ReadByteDummyHandler;
		m6809CPUContext[i].WriteByte = M6809WriteByteDummyHandler;
		m6809CPUContext[i].ReadOp = M6809ReadOpDummyHandler;
		m6809CPUContext[i].ReadOpArg = M6809ReadOpArgDummyHandler;
		
		nM6809CyclesDone[i] = 0;
	
		for (INT32 j = 0; j < (0x0100 * 3); j++) {
			m6809CPUContext[i].pMemMap[j] = NULL;
		}
	}
	
	nM6809CyclesTotal = 0;
	
	m6809_init(NULL);

	for (INT32 i = 0; i < num; i++)
		CpuCheatRegister(i, &M6809CheatCpuConfig);

	return 0;
}

void M6809Exit()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809Exit called without init\n"));
#endif

	nM6809Count = 0;

	if (m6809CPUContext) {
		free(m6809CPUContext);
		m6809CPUContext = NULL;
	}
	
	DebugCPU_M6809Initted = 0;
}

void M6809Open(INT32 num)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809Open called without init\n"));
	if (num >= nM6809Count) bprintf(PRINT_ERROR, _T("M6809Open called with invalid index %x\n"), num);
	if (nActiveCPU != -1) bprintf(PRINT_ERROR, _T("M6809Open called when CPU already open with index %x\n"), num);
#endif

	nActiveCPU = num;
	
	m6809_set_context(&m6809CPUContext[nActiveCPU].reg);
	
	nM6809CyclesTotal = nM6809CyclesDone[nActiveCPU];
}

void M6809Close()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809Close called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809Close called when no CPU open\n"));
#endif

	m6809_get_context(&m6809CPUContext[nActiveCPU].reg);
	
	nM6809CyclesDone[nActiveCPU] = nM6809CyclesTotal;
	
	nActiveCPU = -1;
}

INT32 M6809GetActive()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809GetActive called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809GetActive called when no CPU open\n"));
#endif

	return nActiveCPU;
}

void M6809SetIRQLine(INT32 vector, INT32 status)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809SetIRQLine called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809SetIRQLine called when no CPU open\n"));
#endif

	if (status == M6809_IRQSTATUS_NONE) {
		m6809_set_irq_line(vector, 0);
	}
	
	if (status == M6809_IRQSTATUS_ACK) {
		m6809_set_irq_line(vector, 1);
	}
	
	if (status == M6809_IRQSTATUS_AUTO) {
		m6809_set_irq_line(vector, 1);
		m6809_execute(0);
		m6809_set_irq_line(vector, 0);
		m6809_execute(0);
	}
}

INT32 M6809Run(INT32 cycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809Run called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809Run called when no CPU open\n"));
#endif

	cycles = m6809_execute(cycles);
	
	nM6809CyclesTotal += cycles;
	
	return cycles;
}

void M6809RunEnd()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809RunEnd called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809RunEnd called when no CPU open\n"));
#endif
}

INT32 M6809MapMemory(UINT8* pMemory, UINT16 nStart, UINT16 nEnd, INT32 nType)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809MapMemory called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809MapMemory called when no CPU open\n"));
#endif

	UINT8 cStart = (nStart >> 8);
	UINT8 **pMemMap = m6809CPUContext[nActiveCPU].pMemMap;

	for (UINT16 i = cStart; i <= (nEnd >> 8); i++) {
		if (nType & M6809_READ)	{
			pMemMap[0     + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & M6809_WRITE) {
			pMemMap[0x100 + i] = pMemory + ((i - cStart) << 8);
		}
		if (nType & M6809_FETCH) {
			pMemMap[0x200 + i] = pMemory + ((i - cStart) << 8);
		}
	}
	return 0;

}

void M6809SetReadHandler(UINT8 (*pHandler)(UINT16))
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809SetReadHandler called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809SetReadHandler called when no CPU open\n"));
#endif

	m6809CPUContext[nActiveCPU].ReadByte = pHandler;
}

void M6809SetWriteHandler(void (*pHandler)(UINT16, UINT8))
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809SetWriteHandler called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809SetWriteHandler called when no CPU open\n"));
#endif

	m6809CPUContext[nActiveCPU].WriteByte = pHandler;
}

void M6809SetReadOpHandler(UINT8 (*pHandler)(UINT16))
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809SetReadOpHandler called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809SetReadOpHandler called when no CPU open\n"));
#endif

	m6809CPUContext[nActiveCPU].ReadOp = pHandler;
}

void M6809SetReadOpArgHandler(UINT8 (*pHandler)(UINT16))
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809SetReadOpArgHandler called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809SetReadOpArgHandler called when no CPU open\n"));
#endif

	m6809CPUContext[nActiveCPU].ReadOpArg = pHandler;
}

UINT8 M6809ReadByte(UINT16 Address)
{
	// check mem map
	UINT8 * pr = m6809CPUContext[nActiveCPU].pMemMap[0x000 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (m6809CPUContext[nActiveCPU].ReadByte != NULL) {
		return m6809CPUContext[nActiveCPU].ReadByte(Address);
	}
	
	return 0;
}

void M6809WriteByte(UINT16 Address, UINT8 Data)
{
	// check mem map
	UINT8 * pr = m6809CPUContext[nActiveCPU].pMemMap[0x100 | (Address >> 8)];
	if (pr != NULL) {
		pr[Address & 0xff] = Data;
		return;
	}
	
	// check handler
	if (m6809CPUContext[nActiveCPU].WriteByte != NULL) {
		m6809CPUContext[nActiveCPU].WriteByte(Address, Data);
		return;
	}
}

UINT8 M6809ReadOp(UINT16 Address)
{
	// check mem map
	UINT8 * pr = m6809CPUContext[nActiveCPU].pMemMap[0x200 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (m6809CPUContext[nActiveCPU].ReadOp != NULL) {
		return m6809CPUContext[nActiveCPU].ReadOp(Address);
	}
	
	return 0;
}

UINT8 M6809ReadOpArg(UINT16 Address)
{
	// check mem map
	UINT8 * pr = m6809CPUContext[nActiveCPU].pMemMap[0x000 | (Address >> 8)];
	if (pr != NULL) {
		return pr[Address & 0xff];
	}
	
	// check handler
	if (m6809CPUContext[nActiveCPU].ReadOpArg != NULL) {
		return m6809CPUContext[nActiveCPU].ReadOpArg(Address);
	}
	
	return 0;
}

void M6809WriteRom(UINT32 Address, UINT8 Data)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809WriteRom called without init\n"));
	if (nActiveCPU == -1) bprintf(PRINT_ERROR, _T("M6809WriteRom called when no CPU open\n"));
#endif
	Address &= 0xffff;

	UINT8 * pr = m6809CPUContext[nActiveCPU].pMemMap[0x000 | (Address >> 8)];
	UINT8 * pw = m6809CPUContext[nActiveCPU].pMemMap[0x100 | (Address >> 8)];
	UINT8 * pf = m6809CPUContext[nActiveCPU].pMemMap[0x200 | (Address >> 8)];

	if (pr != NULL) {
		pr[Address & 0xff] = Data;
	}
	
	if (pw != NULL) {
		pw[Address & 0xff] = Data;
	}

	if (pf != NULL) {
		pf[Address & 0xff] = Data;
	}

	// check handler
	if (m6809CPUContext[nActiveCPU].WriteByte != NULL) {
		m6809CPUContext[nActiveCPU].WriteByte(Address, Data);
		return;
	}
}

INT32 M6809Scan(INT32 nAction)
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809Scan called without init\n"));
#endif

	struct BurnArea ba;
	
	if ((nAction & ACB_DRIVER_DATA) == 0) {
		return 1;
	}

	for (INT32 i = 0; i < nM6809Count; i++) {

		M6809Ext *ptr = &m6809CPUContext[i];

		INT32 (*Callback)(INT32 irqline);

		Callback = ptr->reg.irq_callback;

		char szName[] = "M6809 #n";
		szName[7] = '0' + i;

		ba.Data = &m6809CPUContext[i].reg;
		ba.nLen = sizeof(m6809CPUContext[i].reg);
		ba.szName = szName;
		BurnAcb(&ba);

		// necessary?
		SCAN_VAR(ptr->nCyclesTotal);
		SCAN_VAR(ptr->nCyclesSegment);
		SCAN_VAR(ptr->nCyclesLeft);

		ptr->reg.irq_callback = Callback;
	}
	
	return 0;
}
