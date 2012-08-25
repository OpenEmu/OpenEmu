#include "m6502.h"

typedef UINT8 (*pReadPortHandler)(UINT16 a);
typedef void (*pWritePortHandler)(UINT16 a, UINT8 d);
typedef UINT8 (*pReadByteHandler)(UINT16 a);
typedef void (*pWriteByteHandler)(UINT16 a, UINT8 d);
typedef UINT8 (*pReadMemIndexHandler)(UINT16 a);
typedef void (*pWriteMemIndexHandler)(UINT16 a, UINT8 d);
typedef UINT8 (*pReadOpHandler)(UINT16 a);
typedef UINT8 (*pReadOpArgHandler)(UINT16 a);

struct M6502Ext {

	m6502_Regs reg;
	
	INT32 (*execute)(INT32 cycles);
	void (*reset)();
	void (*init)();
	void (*set_irq_line)(INT32 irqline, INT32 state);

	UINT8* pMemMap[0x100 * 3];

	pReadPortHandler ReadPort;
	pWritePortHandler WritePort;
	pReadByteHandler ReadByte;
	pWriteByteHandler WriteByte;
	pReadMemIndexHandler ReadMemIndex;
	pWriteMemIndexHandler WriteMemIndex;
	pReadOpHandler ReadOp;
	pReadOpArgHandler ReadOpArg;
	
	INT32 nCyclesTotal;
	INT32 nCyclesSegment;
	INT32 nCyclesLeft;
};

#define M6502_IRQSTATUS_NONE	0
#define M6502_IRQSTATUS_ACK	1
#define M6502_IRQSTATUS_AUTO	2

#define M6502_READ		1
#define M6502_WRITE	2
#define M6502_FETCH	4

#define M6502_RAM	(M6502_READ | M6502_WRITE | M6502_FETCH)
#define M6502_ROM	(M6502_READ | M6502_FETCH)

extern INT32 nM6502Count;

extern INT32 nM6502CyclesTotal;

void M6502Reset();
void M6502NewFrame();

void n2a03_irq(void); // USED FOR PSG!!

// use these at addresses 0 and 1 for M6510, M6510T, M7501, M8502 map!
UINT8 m6510_read_0000(UINT16 offset);
void m6510_write_0000(UINT16 address, UINT8 data);

// The M6504 only has 13 address bits! use address mirroring!

enum { TYPE_M6502=0, TYPE_M6504, TYPE_M65C02, TYPE_M65SC02, TYPE_N2A03, TYPE_DECO16,
//	 these are the same!
	TYPE_M6510, TYPE_M6510T, TYPE_M7501, TYPE_M8502	 };

INT32 M6502Init(INT32 cpu, INT32 type); // if you're using more than one type
void M6502Exit();
void M6502Open(INT32 num);
void M6502Close();
INT32 M6502GetActive();
void M6502SetIRQLine(INT32 vector, INT32 status);
INT32 M6502Run(INT32 cycles);
void M6502RunEnd();
INT32 M6502MapMemory(UINT8* pMemory, UINT16 nStart, UINT16 nEnd, INT32 nType);
void M6502SetReadPortHandler(UINT8 (*pHandler)(UINT16));
void M6502SetWritePortHandler(void (*pHandler)(UINT16, UINT8));
void M6502SetReadHandler(UINT8 (*pHandler)(UINT16));
void M6502SetWriteHandler(void (*pHandler)(UINT16, UINT8));
void M6502SetReadMemIndexHandler(UINT8 (*pHandler)(UINT16));
void M6502SetWriteMemIndexHandler(void (*pHandler)(UINT16, UINT8));
void M6502SetReadOpHandler(UINT8 (*pHandler)(UINT16));
void M6502SetReadOpArgHandler(UINT8 (*pHandler)(UINT16));
INT32 M6502Scan(INT32 nAction);

UINT32 M6502GetPC();

void M6502WriteRom(UINT32 Address, UINT8 Data);

inline static INT32 M6502TotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6502Initted) bprintf(PRINT_ERROR, _T("M6502TotalCycles called without init\n"));
#endif

	return nM6502CyclesTotal;
}
