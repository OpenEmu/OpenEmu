#include "m6809.h"

typedef UINT8 (*pReadByteHandler)(UINT16 a);
typedef void (*pWriteByteHandler)(UINT16 a, UINT8 d);
typedef UINT8 (*pReadOpHandler)(UINT16 a);
typedef UINT8 (*pReadOpArgHandler)(UINT16 a);

struct M6809Ext {

	m6809_Regs reg;
	
	UINT8* pMemMap[0x100 * 3];

	pReadByteHandler ReadByte;
	pWriteByteHandler WriteByte;
	pReadOpHandler ReadOp;
	pReadOpArgHandler ReadOpArg;
	
	INT32 nCyclesTotal;
	INT32 nCyclesSegment;
	INT32 nCyclesLeft;
};

#define M6809_IRQSTATUS_NONE	0
#define M6809_IRQSTATUS_ACK	1
#define M6809_IRQSTATUS_AUTO	2

#define M6809_READ	1
#define M6809_WRITE	2
#define M6809_FETCH	4

#define M6809_RAM	(M6809_READ | M6809_WRITE | M6809_FETCH)
#define M6809_ROM	(M6809_READ | M6809_FETCH)

extern INT32 nM6809Count;

extern INT32 nM6809CyclesTotal;

void M6809Reset();
void M6809NewFrame();
INT32 M6809Init(INT32 num);
void M6809Exit();
void M6809Open(INT32 num);
void M6809Close();
INT32 M6809GetActive();
void M6809SetIRQLine(INT32 vector, INT32 status);
INT32 M6809Run(INT32 cycles);
void M6809RunEnd();
INT32 M6809MapMemory(UINT8* pMemory, UINT16 nStart, UINT16 nEnd, INT32 nType);
void M6809SetReadHandler(UINT8 (*pHandler)(UINT16));
void M6809SetWriteHandler(void (*pHandler)(UINT16, UINT8));
void M6809SetReadOpHandler(UINT8 (*pHandler)(UINT16));
void M6809SetReadOpArgHandler(UINT8 (*pHandler)(UINT16));
INT32 M6809Scan(INT32 nAction);

void M6809WriteRom(UINT32 Address, UINT8 Data);

inline static INT32 M6809TotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6809Initted) bprintf(PRINT_ERROR, _T("M6809TotalCycles called without init\n"));
#endif

	return nM6809CyclesTotal;
}
