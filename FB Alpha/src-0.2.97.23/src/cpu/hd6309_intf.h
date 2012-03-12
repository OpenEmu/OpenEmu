#include "hd6309.h"

typedef UINT8 (*pReadByteHandler)(UINT16 a);
typedef void (*pWriteByteHandler)(UINT16 a, UINT8 d);
typedef UINT8 (*pReadOpHandler)(UINT16 a);
typedef UINT8 (*pReadOpArgHandler)(UINT16 a);

struct HD6309Ext {

	hd6309_Regs reg;
	
	UINT8* pMemMap[0x100 * 3];

	pReadByteHandler ReadByte;
	pWriteByteHandler WriteByte;
	pReadOpHandler ReadOp;
	pReadOpArgHandler ReadOpArg;
	
	INT32 nCyclesTotal;
	INT32 nCyclesSegment;
	INT32 nCyclesLeft;
};

#define HD6309_IRQSTATUS_NONE	0
#define HD6309_IRQSTATUS_ACK	1
#define HD6309_IRQSTATUS_AUTO	2

#define HD6309_READ	1
#define HD6309_WRITE	2
#define HD6309_FETCH	4

#define HD6309_RAM	(HD6309_READ | HD6309_WRITE | HD6309_FETCH)
#define HD6309_ROM	(HD6309_READ | HD6309_FETCH)

extern INT32 nHD6309Count;

extern INT32 nHD6309CyclesTotal;

void HD6309Reset();
void HD6309NewFrame();
INT32 HD6309Init(INT32 num);
void HD6309Exit();
void HD6309Open(INT32 num);
void HD6309Close();
INT32 HD6309GetActive();
void HD6309SetIRQ(INT32 vector, INT32 status);
INT32 HD6309Run(INT32 cycles);
void HD6309RunEnd();
INT32 HD6309GetPC();
INT32 HD6309MapMemory(UINT8* pMemory, UINT16 nStart, UINT16 nEnd, INT32 nType);
INT32 HD6309MemCallback(UINT16 nStart, UINT16 nEnd, INT32 nType);
void HD6309SetReadByteHandler(UINT8 (*pHandler)(UINT16));
void HD6309SetWriteByteHandler(void (*pHandler)(UINT16, UINT8));
void HD6309SetReadOpHandler(UINT8 (*pHandler)(UINT16));
void HD6309SetReadOpArgHandler(UINT8 (*pHandler)(UINT16));
INT32 HD6309Scan(INT32 nAction);

void HD6309WriteRom(UINT16 Address, UINT8 Data);

inline static INT32 HD6309TotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_HD6309Initted) bprintf(PRINT_ERROR, _T("HD6309TotalCycles called without init\n"));
#endif

	return nHD6309CyclesTotal;
}
