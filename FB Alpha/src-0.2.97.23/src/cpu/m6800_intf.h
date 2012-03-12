#include "m6800.h"

typedef UINT8 (*pReadByteHandler)(UINT16 a);
typedef void (*pWriteByteHandler)(UINT16 a, UINT8 d);
typedef UINT8 (*pReadOpHandler)(UINT16 a);
typedef UINT8 (*pReadOpArgHandler)(UINT16 a);
typedef UINT8 (*pReadPortHandler)(UINT16 a);
typedef void (*pWritePortHandler)(UINT16 a, UINT8 d);

struct M6800Ext {

	m6800_Regs reg;
	
	UINT8* pMemMap[0x100 * 3];

	pReadByteHandler ReadByte;
	pWriteByteHandler WriteByte;
	pReadOpHandler ReadOp;
	pReadOpArgHandler ReadOpArg;
	pReadPortHandler ReadPort;
	pWritePortHandler WritePort;
	
	INT32 nCyclesTotal;
	INT32 nCyclesSegment;
	INT32 nCyclesLeft;
};

#define M6800_IRQSTATUS_NONE	0
#define M6800_IRQSTATUS_ACK	1
#define M6800_IRQSTATUS_AUTO	2

#define HD63701_IRQSTATUS_NONE	M6800_IRQSTATUS_NONE
#define HD63701_IRQSTATUS_ACK	M6800_IRQSTATUS_ACK
#define HD63701_IRQSTATUS_AUTO	M6800_IRQSTATUS_AUTO

#define HD63701_INPUT_LINE_NMI	M6800_INPUT_LINE_NMI
#define HD63701_IRQ_LINE	M6800_IRQ_LINE
#define HD63701_TIN_LINE	M6800_TIN_LINE

#define M6801_IRQSTATUS_NONE	M6800_IRQSTATUS_NONE
#define M6801_IRQSTATUS_ACK	M6800_IRQSTATUS_ACK
#define M6801_IRQSTATUS_AUTO	M6800_IRQSTATUS_AUTO

#define M6801_INPUT_LINE_NMI	M6800_INPUT_LINE_NMI
#define M6801_IRQ_LINE		M6800_IRQ_LINE
#define M6801_TIN_LINE		M6800_TIN_LINE

#define M6803_IRQSTATUS_NONE	M6800_IRQSTATUS_NONE
#define M6803_IRQSTATUS_ACK	M6800_IRQSTATUS_ACK
#define M6803_IRQSTATUS_AUTO	M6800_IRQSTATUS_AUTO

#define M6803_INPUT_LINE_NMI	M6800_INPUT_LINE_NMI
#define M6803_IRQ_LINE		M6800_IRQ_LINE
#define M6803_TIN_LINE		M6800_TIN_LINE

#define M6800_READ	1
#define M6800_WRITE	2
#define M6800_FETCH	4

#define HD63701_READ	1
#define HD63701_WRITE	2
#define HD63701_FETCH	4

#define M6801_READ	1
#define M6801_WRITE	2
#define M6801_FETCH	4

#define M6803_READ	1
#define M6803_WRITE	2
#define M6803_FETCH	4

#define M6800_RAM	(M6800_READ | M6800_WRITE | M6800_FETCH)
#define M6800_ROM	(M6800_READ | M6800_FETCH)

#define HD63701_RAM	(HD63701_READ | HD63701_WRITE | HD63701_FETCH)
#define HD63701_ROM	(HD63701_READ | HD63701_FETCH)

#define M6801_RAM	(M6801_READ | M6801_WRITE | M6801_FETCH)
#define M6801_ROM	(M6801_READ | M6801_FETCH)

#define M6803_RAM	(M6803_READ | M6803_WRITE | M6803_FETCH)
#define M6803_ROM	(M6803_READ | M6803_FETCH)

#define CPU_TYPE_M6800		1
#define CPU_TYPE_HD63701	2
#define CPU_TYPE_M6803		3
#define CPU_TYPE_M6801		4

extern INT32 nM6800Count;

extern INT32 nM6800CyclesTotal;

void M6800Reset();
#define HD63701Reset		M6800Reset
#define M6803Reset		M6800Reset
#define M6801Reset		M6800Reset

void M6800NewFrame();
#define HD63701NewFrame		M6800NewFrame
#define M6803NewFrame		M6800NewFrame
#define M6801NewFrame		M6800NewFrame

INT32 M6800CoreInit(INT32 num, INT32 type);
INT32 M6800Init(INT32 num);
INT32 HD63701Init(INT32 num);
INT32 M6803Init(INT32 num);
INT32 M6801Init(INT32 num);

void M6800Exit();
#define HD63701Exit		M6800Exit
#define M6803Exit		M6800Exit
#define M6801Exit		M6800Exit

void M6800SetIRQ(INT32 vector, INT32 status);
void HD63701SetIRQ(INT32 vector, INT32 status);
void M6803SetIRQ(INT32 vector, INT32 status);
void M6801SetIRQ(INT32 vector, INT32 status);

INT32 M6800Run(INT32 cycles);
INT32 HD63701Run(INT32 cycles);
INT32 M6803Run(INT32 cycles);
#define M6801Run(nCycles)	M6803Run(nCycles)

void M6800RunEnd();
#define HD63701RunEnd		M6800RunEnd
#define M6803RunEnd		M6800RunEnd
#define M6801RunEnd		M6800RunEnd

INT32 M6800GetPC();
#define HD63701GetPC		M6800GetPC
#define M6803GetPC		M6800GetPC
#define M6801GetPC		M6800GetPC

INT32 M6800MapMemory(UINT8* pMemory, UINT16 nStart, UINT16 nEnd, INT32 nType);
#define HD63701MapMemory	M6800MapMemory
#define M6803MapMemory		M6800MapMemory
#define M6801MapMemory		M6800MapMemory

void M6800SetReadByteHandler(UINT8 (*pHandler)(UINT16));
#define HD63701SetReadByteHandler	M6800SetReadByteHandler
#define M6803SetReadByteHandler		M6800SetReadByteHandler
#define M6801SetReadByteHandler		M6800SetReadByteHandler

void M6800SetWriteByteHandler(void (*pHandler)(UINT16, UINT8));
#define HD63701SetWriteByteHandler	M6800SetWriteByteHandler
#define M6803SetWriteByteHandler	M6800SetWriteByteHandler
#define M6801SetWriteByteHandler	M6800SetWriteByteHandler

void M6800SetReadOpHandler(UINT8 (*pHandler)(UINT16));
#define HD63701SetReadOpHandler		M6800SetReadOpHandler
#define M6803SetReadOpHandler		M6800SetReadOpHandler
#define M6801SetReadOpHandler		M6800SetReadOpHandler

void M6800SetReadOpArgHandler(UINT8 (*pHandler)(UINT16));
#define HD63701SetReadOpArgHandler	M6800SetReadOpArgHandler
#define M6803SetReadOpArgHandler	M6800SetReadOpArgHandler
#define M6801SetReadOpArgHandler	M6800SetReadOpArgHandler

void M6800SetReadPortHandler(UINT8 (*pHandler)(UINT16));
#define HD63701SetReadPortHandler	M6800SetReadPortHandler
#define M6803SetReadPortHandler		M6800SetReadPortHandler
#define M6801SetReadPortHandler		M6800SetReadPortHandler

void M6800SetWritePortHandler(void (*pHandler)(UINT16, UINT8));
#define HD63701SetWritePortHandler	M6800SetWritePortHandler
#define M6803SetWritePortHandler	M6800SetWritePortHandler
#define M6801SetWritePortHandler	M6800SetWritePortHandler

INT32 M6800Scan(INT32 nAction);
#define HD63701Scan		M6800Scan
#define M6803Scan		M6800Scan
#define M6801Scan		M6800Scan

void M6800WriteRom(UINT32 Address, UINT8 Data);

inline static INT32 M6800TotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_M6800Initted) bprintf(PRINT_ERROR, _T("M6800TotalCycles called without init\n"));
#endif

	return nM6800CyclesTotal;
}
