// 68000 (Sixty Eight K) Interface - header file
#include <stdint.h>
#ifndef FASTCALL
 #undef __fastcall
 #define __fastcall
#endif

#if defined BUILD_A68K
 #define EMU_A68K								// Use A68K Assembler 68000 emulator
#endif

#define EMU_M68K								// Use Musashi 68000 emulator

#define SEK_MAX	(4)								// Maximum number of CPUs supported

#if defined EMU_M68K
 #include "m68k.h"
#endif

// Number of bits used for each page in the fast memory map
#define SEK_BITS		(10)					// 10 = 0x0400 page size
#define SEK_PAGE_COUNT  (1 << (24 - SEK_BITS))	// Number of pages
#define SEK_SHIFT		(SEK_BITS)				// Shift value = page bits
#define SEK_PAGE_SIZE	(1 << SEK_BITS)			// Page size
#define SEK_PAGEM		(SEK_PAGE_SIZE - 1)
#define SEK_WADD		(SEK_PAGE_COUNT)		// Value to add for write section = Number of pages
#define SEK_MASK		(SEK_WADD - 1)
#define SEK_MAXHANDLER	(10)						// Max. number of handlers for memory access

#if SEK_MAXHANDLER < 1
 #error At least one set of handlers for memory access must be used.
#endif

#ifdef EMU_A68K
 extern "C" void __cdecl M68000_RUN();
 extern "C" void __cdecl M68000_RESET();
#endif

#ifdef EMU_A68K
 // The format of the data in a68k.asm (at the _M68000_regs location)
 struct A68KContext {
	UINT32 d[8], a[8];
	UINT32 isp, srh, ccr, xc, pc, irq, sr;
	INT32 (*IrqCallback) (INT32 nIrq);
	UINT32 ppc;
	INT32 (*ResetCallback)();
	INT32 (*RTECallback)();
	INT32 (*CmpCallback)(UINT32 val, INT32 reg);
	UINT32 sfc, dfc, usp, vbr;
	UINT32 nAsmBank, nCpuVersion;
 };
 extern "C" struct A68KContext M68000_regs;
 extern     struct A68KContext* SekRegs[SEK_MAX];

 extern "C" UINT8* OP_ROM;
 extern "C" UINT8* OP_RAM;

 void __fastcall AsekChangePc(UINT32 pc);
#endif

#ifdef EMU_M68K
 extern "C" INT32 nSekM68KContextSize[SEK_MAX];
 extern "C" INT8* SekM68KContext[SEK_MAX];
 extern "C" INT32 m68k_ICount;
#endif

typedef UINT8 (__fastcall *pSekReadByteHandler)(UINT32 a);
typedef void (__fastcall *pSekWriteByteHandler)(UINT32 a, UINT8 d);
typedef UINT16 (__fastcall *pSekReadWordHandler)(UINT32 a);
typedef void (__fastcall *pSekWriteWordHandler)(UINT32 a, UINT16 d);
typedef UINT32 (__fastcall *pSekReadLongHandler)(UINT32 a);
typedef void (__fastcall *pSekWriteLongHandler)(UINT32 a, UINT32 d);

typedef INT32 (__fastcall *pSekResetCallback)();
typedef INT32 (__fastcall *pSekRTECallback)();
typedef INT32 (__fastcall *pSekIrqCallback)(INT32 irq);
typedef INT32 (__fastcall *pSekCmpCallback)(UINT32 val, INT32 reg);

extern INT32 nSekCycles[SEK_MAX], nSekCPUType[SEK_MAX];

// Mapped memory pointers to Rom and Ram areas (Read then Write)
// These memory areas must be allocated multiples of the page size
// with a 4 byte over-run area lookup for each page (*3 for read, write and fetch)
struct SekExt {
	UINT8* MemMap[SEK_PAGE_COUNT * 3];

	// If MemMap[i] < SEK_MAXHANDLER, use the handler functions
	pSekReadByteHandler ReadByte[SEK_MAXHANDLER];
	pSekWriteByteHandler WriteByte[SEK_MAXHANDLER];
	pSekReadWordHandler ReadWord[SEK_MAXHANDLER];
	pSekWriteWordHandler WriteWord[SEK_MAXHANDLER];
	pSekReadLongHandler ReadLong[SEK_MAXHANDLER];
	pSekWriteLongHandler WriteLong[SEK_MAXHANDLER];

	pSekResetCallback ResetCallback;
	pSekRTECallback RTECallback;
	pSekIrqCallback IrqCallback;
	pSekCmpCallback CmpCallback;
};

#define SEK_DEF_READ_WORD(i, a) { UINT16 d; d = (UINT16)(pSekExt->ReadByte[i](a) << 8); d |= (UINT16)(pSekExt->ReadByte[i]((a) + 1)); return d; }
#define SEK_DEF_WRITE_WORD(i, a, d) { pSekExt->WriteByte[i]((a),(UINT8)((d) >> 8)); pSekExt->WriteByte[i]((a) + 1, (UINT8)((d) & 0xff)); }
#define SEK_DEF_READ_LONG(i, a) { UINT32 d; d = pSekExt->ReadWord[i](a) << 16; d |= pSekExt->ReadWord[i]((a) + 2); return d; }
#define SEK_DEF_WRITE_LONG(i, a, d) { pSekExt->WriteWord[i]((a),(UINT16)((d) >> 16)); pSekExt->WriteWord[i]((a) + 2,(UINT16)((d) & 0xffff)); }

extern struct SekExt *SekExt[SEK_MAX], *pSekExt;
extern INT32 nSekActive;										// The cpu which is currently being emulated
extern INT32 nSekCyclesTotal, nSekCyclesScanline, nSekCyclesSegment, nSekCyclesDone, nSekCyclesToDo;

UINT32 SekReadByte(UINT32 a);
UINT32 SekReadWord(UINT32 a);
UINT32 SekReadLong(UINT32 a);

UINT32 SekFetchByte(UINT32 a);
UINT32 SekFetchWord(UINT32 a);
UINT32 SekFetchLong(UINT32 a);

void SekWriteByte(UINT32 a, UINT8 d);
void SekWriteWord(UINT32 a, UINT16 d);
void SekWriteLong(UINT32 a, UINT32 d);

void SekWriteByteROM(UINT32 a, UINT8 d);
void SekWriteWordROM(UINT32 a, UINT16 d);
void SekWriteLongROM(UINT32 a, UINT32 d);

INT32 SekInit(INT32 nCount, INT32 nCPUType);
INT32 SekExit();

void SekNewFrame();
void SekSetCyclesScanline(INT32 nCycles);

void SekClose();
void SekOpen(const INT32 i);
INT32 SekGetActive();

#define SEK_IRQSTATUS_NONE (0x0000)
#define SEK_IRQSTATUS_AUTO (0x2000)
#define SEK_IRQSTATUS_ACK  (0x1000)

void SekSetIRQLine(const INT32 line, const INT32 status);
void SekReset();

void SekRunEnd();
void SekRunAdjust(const INT32 nCycles);
INT32 SekRun(const INT32 nCycles);

inline static INT32 SekIdle(INT32 nCycles)
{
#if defined FBA_DEBUG
	extern UINT8 DebugCPU_SekInitted;
	if (!DebugCPU_SekInitted) bprintf(PRINT_ERROR, (TCHAR*)_T("SekIdle called without init\n"));
	if (nSekActive == -1) bprintf(PRINT_ERROR, (TCHAR*)_T("SekIdle called when no CPU open\n"));
#endif

	nSekCyclesTotal += nCycles;

	return nCycles;
}

inline static INT32 SekSegmentCycles()
{
#if defined FBA_DEBUG
	extern UINT8 DebugCPU_SekInitted;
	if (!DebugCPU_SekInitted) bprintf(PRINT_ERROR, (TCHAR*)_T("SekSegmentCycles called without init\n"));
	if (nSekActive == -1) bprintf(PRINT_ERROR, (TCHAR*)_T("SekSegmentCycles called when no CPU open\n"));
#endif

#ifdef EMU_M68K
	return nSekCyclesDone + nSekCyclesToDo - m68k_ICount;
#else
	return nSekCyclesDone + nSekCyclesToDo;
#endif
}

#if defined FBA_DEBUG
static INT32 SekTotalCycles()
#else
inline static INT32 SekTotalCycles()
#endif
{
#if defined FBA_DEBUG
	extern UINT8 DebugCPU_SekInitted;
	if (!DebugCPU_SekInitted) bprintf(PRINT_ERROR, (TCHAR*)_T("SekTotalCycles called without init\n"));
	if (nSekActive == -1) bprintf(PRINT_ERROR, (TCHAR*)_T("SekTotalCycles called when no CPU open\n"));
#endif

#ifdef EMU_M68K
	return nSekCyclesTotal + nSekCyclesToDo - m68k_ICount;
#else
	return nSekCyclesTotal + nSekCyclesToDo;
#endif
}

inline static INT32 SekCurrentScanline()
{
#if defined FBA_DEBUG
	extern UINT8 DebugCPU_SekInitted;
	if (!DebugCPU_SekInitted) bprintf(PRINT_ERROR, (TCHAR*)_T("SekCurrentScanline called without init\n"));
	if (nSekActive == -1) bprintf(PRINT_ERROR, (TCHAR*)_T("SekCurrentScanline called when no CPU open\n"));
#endif

	return SekTotalCycles() / nSekCyclesScanline;
}

// SekMemory types:
#define SM_READ  (1)
#define SM_WRITE (2)
#define SM_FETCH (4)
#define SM_ROM (SM_READ | SM_FETCH)
#define SM_RAM (SM_READ | SM_WRITE | SM_FETCH)

// Map areas of memory
INT32 SekMapMemory(UINT8* pMemory, UINT32 nStart, UINT32 nEnd, INT32 nType);
INT32 SekMapHandler(uintptr_t nHandler, UINT32 nStart, UINT32 nEnd, INT32 nType);

// Set handlers
INT32 SekSetReadByteHandler(INT32 i, pSekReadByteHandler pHandler);
INT32 SekSetWriteByteHandler(INT32 i, pSekWriteByteHandler pHandler);
INT32 SekSetReadWordHandler(INT32 i, pSekReadWordHandler pHandler);
INT32 SekSetWriteWordHandler(INT32 i, pSekWriteWordHandler pHandler);
INT32 SekSetReadLongHandler(INT32 i, pSekReadLongHandler pHandler);
INT32 SekSetWriteLongHandler(INT32 i, pSekWriteLongHandler pHandler);

// Set callbacks
INT32 SekSetResetCallback(pSekResetCallback pCallback);
INT32 SekSetRTECallback(pSekRTECallback pCallback);
INT32 SekSetIrqCallback(pSekIrqCallback pCallback);
INT32 SekSetCmpCallback(pSekCmpCallback pCallback);

// Get a CPU's PC
INT32 SekGetPC(INT32 n);

INT32 SekScan(INT32 nAction);
