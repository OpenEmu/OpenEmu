/*****************************************************************************
 *
 *   Portable Hitachi SH-2 (SH7600 family) emulator
 *
 *   Copyright (c) 2000 Juergen Buchmueller <pullmoll@t-online.de>,
 *   all rights reserved.
 *
 *   - This source code is released as freeware for non-commercial purposes.
 *   - You are free to use and redistribute this code in modified or
 *     unmodified form, provided you list me in the credits.
 *   - If you modify this source code, you must add a notice to each modified
 *     source file that it has been changed.  If you're a nice person, you
 *     will clearly mark each change too.  :)
 *   - If you wish to use this for commercial purposes, please contact me at
 *     pullmoll@t-online.de
 *   - The author of this copywritten work reserves the right to change the
 *     terms of its usage and license at any time, including retroactively
 *   - This entire notice must remain in the source code.
 *
 *  This work is based on <tiraniddo@hotmail.com> C/C++ implementation of
 *  the SH-2 CPU core and was adapted to the MAME CPU core requirements.
 *  Thanks also go to Chuck Mason <chukjr@sundail.net> and Olivier Galibert
 *  <galibert@pobox.com> for letting me peek into their SEMU code :-)
 *
 *****************************************************************************
 *
 *  Port to Finalburn Alpha by OopsWare
 *  http://oopsware.googlepages.com/
 *
 *****************************************************************************/

//#include <stdio.h>
//#include <stdlib.h>
//#include <string.h>

#include "burnint.h"
#include "sh2.h"

//#include "tchar.h"
//extern int (__cdecl *bprintf) (int nStatus, TCHAR* szFormat, ...);

int has_sh2;

/*typedef signed char INT8;
typedef unsigned char UINT8;
typedef signed short INT16;
typedef unsigned short UINT16;
typedef signed int INT32;
typedef unsigned int UINT32;
typedef signed long long INT64;
typedef unsigned long long UINT64;*/

#define BUSY_LOOP_HACKS 	1
#define FAST_OP_FETCH		1
#define USE_JUMPTABLE		0

#define SH2_INT_15			15

#define	SH2_INLINE

#if FAST_OP_FETCH

	#define change_pc(newpc)															\
		sh2->pc = (newpc);																\
		pSh2Ext->opbase = pSh2Ext->MemMap[ (sh2->pc >> SH2_SHIFT) + SH2_WADD * 2 ];		\
		pSh2Ext->opbase -= (sh2->pc & ~SH2_PAGEM);

#else

	#define change_pc(newpc)	sh2->pc = (newpc);

#endif

#define COMBINE_DATA(varptr)		(*(varptr) = (*(varptr) & mem_mask) | (data & ~mem_mask))



typedef struct
{
	int irq_vector;
	int irq_priority;
} irq_entry;

typedef struct
{
	UINT32	ppc;
	UINT32	pc;
	UINT32	pr;
	UINT32	sr;
	UINT32	gbr, vbr;
	UINT32	mach, macl;
	UINT32	r[16];
	UINT32	ea;
	UINT32	delay;
	UINT32	cpu_off;
	UINT32	dvsr, dvdnth, dvdntl, dvcr;
	UINT32	pending_irq;
	UINT32    test_irq;
	irq_entry     irq_queue[16];

	INT8	irq_line_state[17];
	UINT32	m[0x200];
	INT8  nmi_line_state;

	UINT16 	frc;
	UINT16 	ocra, ocrb, icr;
	UINT32 	frc_base;

	int		frt_input;
	int 	internal_irq_level;
	int 	internal_irq_vector;

//	emu_timer *timer;
	UINT32 	timer_cycles;
	UINT32 	timer_base;
	int     timer_active;
	
//	emu_timer *dma_timer[2];
	UINT32 	dma_timer_cycles[2];
	UINT32 	dma_timer_base[2];
	int     dma_timer_active[2];

//	int     is_slave, cpu_number;
	
	UINT32	cycle_counts;
	UINT32	sh2_cycles_to_run;
	INT32	sh2_icount;
	int     sh2_total_cycles;
	
	int 	(*irq_callback)(int irqline);

} SH2;

static SH2 * sh2;

static UINT32 sh2_GetTotalCycles()
{
	return sh2->cycle_counts + sh2->sh2_cycles_to_run - sh2->sh2_icount;
}

static const int div_tab[4] = { 3, 5, 3, 0 };

enum {
	ICF  = 0x00800000,
	OCFA = 0x00080000,
	OCFB = 0x00040000,
	OVF  = 0x00020000
};

//static TIMER_CALLBACK( sh2_timer_callback );

#define T	0x00000001
#define S	0x00000002
#define I	0x000000f0
#define Q	0x00000100
#define M	0x00000200

#define AM	0xc7ffffff

#define FLAGS	(M|Q|I|S|T)

#define Rn	((opcode>>8)&15)
#define Rm	((opcode>>4)&15)

static UINT32 sh2_internal_r(UINT32 A, UINT32 mask);
static void sh2_internal_w(UINT32 offset, UINT32 data, UINT32 mem_mask);

//-- sh2 memory handler for Finalburn Alpha ---------------------

#define SH2_BITS		(16)					// 16 = 0x10000 page size
#define SH2_PAGE_COUNT  (1 << (32 - SH2_BITS))	// Number of pages
#define SH2_SHIFT		(SH2_BITS)				// Shift value = page bits
#define SH2_PAGE_SIZE	(1 << SH2_BITS)			// Page size
#define SH2_PAGEM		(SH2_PAGE_SIZE - 1)
#define SH2_WADD		(SH2_PAGE_COUNT)		// Value to add for write section = Number of pages
#define SH2_MASK		(SH2_WADD - 1)

#define	SH2_MAXHANDLER	(8)


typedef struct 
{
	SH2	sh2;
	unsigned char * MemMap[SH2_PAGE_COUNT * 3];
	pSh2ReadByteHandler ReadByte[SH2_MAXHANDLER];
	pSh2WriteByteHandler WriteByte[SH2_MAXHANDLER];
	pSh2ReadWordHandler ReadWord[SH2_MAXHANDLER];
	pSh2WriteWordHandler WriteWord[SH2_MAXHANDLER];
	pSh2ReadLongHandler ReadLong[SH2_MAXHANDLER];
	pSh2WriteLongHandler WriteLong[SH2_MAXHANDLER];
	
	unsigned char * opbase;
	int suspend;
} SH2EXT;

static SH2EXT * pSh2Ext;
static SH2EXT * Sh2Ext = NULL;

/* SH-2 Memory Map:
 * 0x00000000 ~ 0x07ffffff : user
 * 0x08000000 ~ 0x0fffffff : user ( mirror )
 * 0x10000000 ~ 0x17ffffff : user ( mirror )
 * 0x18000000 ~ 0x1fffffff : user ( mirror )
 * 0x20000000 ~ 0x27ffffff : user ( mirror )
 * 0x28000000 ~ 0x2fffffff : user ( mirror )
 * 0x30000000 ~ 0x37ffffff : user ( mirror )
 * 0x38000000 ~ 0x3fffffff : user ( mirror )
 * 0x40000000 ~ 0xbfffffff : fill with 0xa5
 * 0xc0000000 ~ 0xdfffffff : extend user
 * 0xe0000000 ~ 0xffffffff : internal mem
 */
 
int Sh2MapMemory(unsigned char* pMemory, unsigned int nStart, unsigned int nEnd, int nType)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2MapMemory called without init\n"));
#endif

	unsigned char* Ptr = pMemory - nStart;
	unsigned char** pMemMap = pSh2Ext->MemMap + (nStart >> SH2_SHIFT);
	int need_mirror = (nStart < 0x08000000) ? 1 : 0;
		
	for (unsigned long long i = (nStart & ~SH2_PAGEM); i <= nEnd; i += SH2_PAGE_SIZE, pMemMap++) {
		if (nType & 0x01 /*SM_READ*/)  pMemMap[0] 			= Ptr + i;
		if (nType & 0x02 /*SM_WRITE*/) pMemMap[SH2_WADD] 	= Ptr + i;
		if (nType & 0x04 /*SM_FETCH*/) pMemMap[SH2_WADD*2]	= Ptr + i;
		
		if ( need_mirror ) {
			if (nType & 0x01 /*SM_READ*/)  {
				pMemMap[0 + (0x08000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[0 + (0x10000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[0 + (0x18000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[0 + (0x20000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[0 + (0x28000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[0 + (0x30000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[0 + (0x38000000 >> SH2_SHIFT)] = Ptr + i;
			}
			if (nType & 0x02 /*SM_WRITE*/) {
				pMemMap[SH2_WADD + (0x08000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD + (0x10000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD + (0x18000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD + (0x20000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD + (0x28000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD + (0x30000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD + (0x38000000 >> SH2_SHIFT)] = Ptr + i;
			}
			if (nType & 0x04 /*SM_FETCH*/) {
				pMemMap[SH2_WADD*2 + (0x08000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD*2 + (0x10000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD*2 + (0x18000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD*2 + (0x20000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD*2 + (0x28000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD*2 + (0x30000000 >> SH2_SHIFT)] = Ptr + i;
				pMemMap[SH2_WADD*2 + (0x38000000 >> SH2_SHIFT)] = Ptr + i;
			}
		}
	}
	return 0;
}

int Sh2MapHandler(uintptr_t nHandler, unsigned int nStart, unsigned int nEnd, int nType)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2MapHandler called without init\n"));
#endif

	unsigned char** pMemMap = pSh2Ext->MemMap + (nStart >> SH2_SHIFT);
	int need_mirror = (nStart < 0x08000000) ? 1 : 0;
	
	for (unsigned long long i = (nStart & ~SH2_PAGEM); i <= nEnd; i += SH2_PAGE_SIZE, pMemMap++) {
		if (nType & 0x01 /*SM_READ*/)  pMemMap[0]		 	= (unsigned char*)nHandler;
		if (nType & 0x02 /*SM_WRITE*/) pMemMap[SH2_WADD] 	= (unsigned char*)nHandler;
		if (nType & 0x04 /*SM_FETCH*/) pMemMap[SH2_WADD*2]	= (unsigned char*)nHandler;
		
		if ( need_mirror ) {
			if (nType & 0x01 /*SM_READ*/)  {
				pMemMap[0 + (0x08000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[0 + (0x10000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[0 + (0x18000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[0 + (0x20000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[0 + (0x28000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[0 + (0x30000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[0 + (0x38000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
			}
			if (nType & 0x02 /*SM_WRITE*/) {
				pMemMap[SH2_WADD + (0x08000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD + (0x10000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD + (0x18000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD + (0x20000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD + (0x28000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD + (0x30000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD + (0x38000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
			}
			if (nType & 0x04 /*SM_FETCH*/) {
				pMemMap[SH2_WADD*2 + (0x08000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD*2 + (0x10000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD*2 + (0x18000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD*2 + (0x20000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD*2 + (0x28000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD*2 + (0x30000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
				pMemMap[SH2_WADD*2 + (0x38000000 >> SH2_SHIFT)] = (unsigned char*)nHandler;
			}
		}
		
	}
	return 0;
}

int Sh2SetReadByteHandler(int i, pSh2ReadByteHandler pHandler)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2SetReadByteHandler called without init\n"));
	if (i >= SH2_MAXHANDLER) bprintf(PRINT_ERROR, _T("Sh2SetReadByteHandler called with invalid index %x\n"), i);
#endif

	if (i >= SH2_MAXHANDLER) return 1;
	pSh2Ext->ReadByte[i] = pHandler;
	return 0;
}

int Sh2SetWriteByteHandler(int i, pSh2WriteByteHandler pHandler)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2SetWriteByteHandler called without init\n"));
	if (i >= SH2_MAXHANDLER) bprintf(PRINT_ERROR, _T("Sh2SetWriteByteHandler called with invalid index %x\n"), i);
#endif

	if (i >= SH2_MAXHANDLER) return 1;
	pSh2Ext->WriteByte[i] = pHandler;
	return 0;
}

int Sh2SetReadWordHandler(int i, pSh2ReadWordHandler pHandler)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2SetReadWordHandler called without init\n"));
	if (i >= SH2_MAXHANDLER) bprintf(PRINT_ERROR, _T("Sh2SetReadWordHandler called with invalid index %x\n"), i);
#endif

	if (i >= SH2_MAXHANDLER) return 1;
	pSh2Ext->ReadWord[i] = pHandler;
	return 0;
}

int Sh2SetWriteWordHandler(int i, pSh2WriteWordHandler pHandler)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2SetWriteWordHandler called without init\n"));
	if (i >= SH2_MAXHANDLER) bprintf(PRINT_ERROR, _T("Sh2SetWriteWordHandler called with invalid index %x\n"), i);
#endif

	if (i >= SH2_MAXHANDLER) return 1;
	pSh2Ext->WriteWord[i] = pHandler;
	return 0;
}

int Sh2SetReadLongHandler(int i, pSh2ReadLongHandler pHandler)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2SetReadLongHandler called without init\n"));
	if (i >= SH2_MAXHANDLER) bprintf(PRINT_ERROR, _T("Sh2SetReadLongHandler called with invalid index %x\n"), i);
#endif

	if (i >= SH2_MAXHANDLER) return 1;
	pSh2Ext->ReadLong[i] = pHandler;
	return 0;
}

int Sh2SetWriteLongHandler(int i, pSh2WriteLongHandler pHandler)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2SetWriteLongHandler called without init\n"));
	if (i >= SH2_MAXHANDLER) bprintf(PRINT_ERROR, _T("Sh2SetWriteLongHandler called with invalid index %x\n"), i);
#endif

	if (i >= SH2_MAXHANDLER) return 1;
	pSh2Ext->WriteLong[i] = pHandler;
	return 0;
}

unsigned char  __fastcall Sh2InnerReadByte(unsigned int a) 
{ 
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2InnerReadByte called without init\n"));
#endif

	return sh2_internal_r((a & 0x1fc)>>2, ~(0xff << (((~a) & 3)*8))) >> (((~a) & 3)*8); 
}

unsigned short __fastcall Sh2InnerReadWord(unsigned int a) 
{ 
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2InnerReadWord called without init\n"));
#endif

	return sh2_internal_r((a & 0x1fc)>>2, ~(0xffff << (((~a) & 2)*8))) >> (((~a) & 2)*8); 
}

unsigned int   __fastcall Sh2InnerReadLong(unsigned int a) 
{ 
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2InnerReadLong called without init\n"));
#endif

	return sh2_internal_r((a & 0x1fc)>>2, 0); 
}

void __fastcall Sh2InnerWriteByte(unsigned int a, unsigned char d) 
{ 
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2InnerWriteByte called without init\n"));
#endif

	//bprintf(0, _T("Attempt to write byte value   %02x to location %8x   offset: %04x\n"), d, a, (a & 0x1fc)>>2);
	sh2_internal_w((a & 0x1fc)>>2, d << (((~a) & 3)*8), ~(0xff << (((~a) & 3)*8)));
}

void __fastcall Sh2InnerWriteWord(unsigned int a, unsigned short d) 
{ 
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2InnerWriteWord called without init\n"));
#endif

	sh2_internal_w((a & 0x1fc)>>2, d << (((~a) & 2)*8), ~(0xffff << (((~a) & 2)*8)));
}

void __fastcall Sh2InnerWriteLong(unsigned int a, unsigned int d) 
{ 
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2InnerWriteLong called without init\n"));
#endif

	sh2_internal_w((a & 0x1fc)>>2, d, 0);
}

unsigned char  __fastcall Sh2EmptyReadByte(unsigned int) { return 0xa5; }
unsigned short __fastcall Sh2EmptyReadWord(unsigned int) { return 0xa5a5; }
unsigned int   __fastcall Sh2EmptyReadLong(unsigned int) { return 0xa5a5a5a5; }
void __fastcall Sh2EmptyWriteByte(unsigned int, unsigned char) { }
void __fastcall Sh2EmptyWriteWord(unsigned int, unsigned short) { }
void __fastcall Sh2EmptyWriteLong(unsigned int, unsigned int) { }

int Sh2Exit()
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2Exit called without init\n"));
#endif

	has_sh2 = 0;

	if (Sh2Ext) {
		free(Sh2Ext);
		Sh2Ext = NULL;
	}
	pSh2Ext = NULL;
	
	DebugCPU_SH2Initted = 0;

	return 0;
}

static void Sh2CheatWriteByte(UINT32 a, UINT8 d)
{
	Sh2WriteByte(a,d);
}

static UINT8 Sh2CheatReadByte(UINT32 a)
{
	return Sh2ReadByte(a);
}

static cpu_core_config Sh2CheatCpuConfig =
{
	Sh2Open,
	Sh2Close,
	Sh2CheatReadByte,
	Sh2CheatWriteByte,
	Sh2GetActive,
	Sh2TotalCycles,
	Sh2NewFrame,
	Sh2Run,
	Sh2StopRun,
	Sh2Reset,
	0xffffffff,
	0
};

int Sh2Init(int nCount)
{
	DebugCPU_SH2Initted = 1;

	has_sh2 = 1;

	Sh2Ext = (SH2EXT *)malloc(sizeof(SH2EXT) * nCount);
	if (Sh2Ext == NULL) {
		Sh2Exit();
		return 1;
	}
	memset(Sh2Ext, 0, sizeof(SH2EXT) * nCount);

	// init default memory handler
	for (int i=0; i<nCount; i++) {
		pSh2Ext = Sh2Ext + i;
		//sh2 = & pSh2Ext->sh2;

		Sh2MapHandler(SH2_MAXHANDLER - 1, 0xE0000000, 0xFFFFFFFF, 0x07);
		Sh2MapHandler(SH2_MAXHANDLER - 2, 0x40000000, 0xBFFFFFFF, 0x07);
//		Sh2MapHandler(SH2_MAXHANDLER - 3, 0xC0000000, 0xDFFFFFFF, 0x07);

		Sh2SetReadByteHandler (SH2_MAXHANDLER - 1, Sh2InnerReadByte);
		Sh2SetReadWordHandler (SH2_MAXHANDLER - 1, Sh2InnerReadWord);
		Sh2SetReadLongHandler (SH2_MAXHANDLER - 1, Sh2InnerReadLong);
		Sh2SetWriteByteHandler(SH2_MAXHANDLER - 1, Sh2InnerWriteByte);		
		Sh2SetWriteWordHandler(SH2_MAXHANDLER - 1, Sh2InnerWriteWord);
		Sh2SetWriteLongHandler(SH2_MAXHANDLER - 1, Sh2InnerWriteLong);
		
		Sh2SetReadByteHandler (SH2_MAXHANDLER - 2, Sh2EmptyReadByte);
		Sh2SetReadWordHandler (SH2_MAXHANDLER - 2, Sh2EmptyReadWord);
		Sh2SetReadLongHandler (SH2_MAXHANDLER - 2, Sh2EmptyReadLong);
		Sh2SetWriteByteHandler(SH2_MAXHANDLER - 2, Sh2EmptyWriteByte);		
		Sh2SetWriteWordHandler(SH2_MAXHANDLER - 2, Sh2EmptyWriteWord);
		Sh2SetWriteLongHandler(SH2_MAXHANDLER - 2, Sh2EmptyWriteLong);

		CpuCheatRegister(i, &Sh2CheatCpuConfig);
	}

	return 0;
}

void Sh2Open(const int i)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2Open called without init\n"));
#endif

	pSh2Ext = Sh2Ext + i;
	sh2 = & (pSh2Ext->sh2);
}

void Sh2Close()
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2Close called without init\n"));
#endif
}

int Sh2GetActive()
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2GetActive called without init\n"));
#endif

	return 0;
}

void Sh2Reset(unsigned int pc, unsigned r15)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2Reset called without init\n"));
#endif

	memset(sh2, 0, sizeof(SH2) - 4);

	sh2->pc = pc;
	sh2->r[15] = r15;
	sh2->sr = I;
	
	change_pc(sh2->pc & AM);

	sh2->internal_irq_level = -1;
}

//----------------------------------------------------------------

unsigned char program_read_byte_32be(unsigned int /*A*/)
{
	return 0;	
}

unsigned short program_read_word_32be(unsigned int /*A*/)
{
	return 0;
}

unsigned int program_read_dword_32be(unsigned int /*A*/)
{
	return 0;
}

void program_write_byte_32be(unsigned int /*A*/, unsigned char /*V*/)
{
}

void program_write_word_32be(unsigned int /*A*/, unsigned short /*V*/)
{
}

void program_write_dword_32be(unsigned int /*A*/, unsigned int /*V*/)
{
}

//pSh2Ext->opbase

#if FAST_OP_FETCH

#ifdef LSB_FIRST
#define cpu_readop16(A)	*(unsigned short *)(pSh2Ext->opbase + ((A) ^ 0x02))
#else
#define cpu_readop16(A)	(*(unsigned short *)(pSh2Ext->opbase + ((A))))
#endif

#else

SH2_INLINE unsigned short cpu_readop16(unsigned int A)
{
	unsigned char * pr;
	pr = pSh2Ext->MemMap[ (A >> SH2_SHIFT) + SH2_WADD * 2 ];
	if ( (unsigned int)pr >= SH2_MAXHANDLER ) {
#ifdef LSB_FIRST
		A ^= 2;
#endif
		return *((unsigned short *)(pr + (A & SH2_PAGEM)));
	}
	return pSh2Ext->ReadWord[(unsigned int)pr](A);
}

#endif

// ------------------------------------------------------

SH2_INLINE UINT8 RB(UINT32 A)
{
/*	if (A >= 0xe0000000) return sh2_internal_r((A & 0x1fc)>>2, ~(0xff << (((~A) & 3)*8))) >> (((~A) & 3)*8);
	if (A >= 0xc0000000) return program_read_byte_32be(A);
	if (A >= 0x40000000) return 0xa5;
	return program_read_byte_32be(A & AM); */
	
	unsigned char * pr;
	pr = pSh2Ext->MemMap[ A >> SH2_SHIFT ];
	if ( (uintptr_t)pr >= SH2_MAXHANDLER ) {
#ifdef LSB_FIRST
		A ^= 3;
#endif
		return pr[A & SH2_PAGEM];
	}
	return pSh2Ext->ReadByte[(uintptr_t)pr](A);
}

SH2_INLINE UINT16 RW(UINT32 A)
{
/*	if (A >= 0xe0000000) return sh2_internal_r((A & 0x1fc)>>2, ~(0xffff << (((~A) & 2)*8))) >> (((~A) & 2)*8);
	if (A >= 0xc0000000) return program_read_word_32be(A);
	if (A >= 0x40000000) return 0xa5a5;
	return program_read_word_32be(A & AM); */
	
	unsigned char * pr;
	pr = pSh2Ext->MemMap[ A >> SH2_SHIFT ];
	if ( (uintptr_t)pr >= SH2_MAXHANDLER ) {
#ifdef LSB_FIRST
		A ^= 2;
#endif
		//return (pr[A & SH2_PAGEM] << 8) | pr[(A & SH2_PAGEM) + 1];
		return *((unsigned short *)(pr + (A & SH2_PAGEM)));
	}
	return pSh2Ext->ReadWord[(uintptr_t)pr](A);
}

SH2_INLINE UINT16 OPRW(UINT32 A)
{

	unsigned char * pr;
	pr = pSh2Ext->MemMap[ (A >> SH2_SHIFT) + SH2_WADD * 2 ];
	if ( (uintptr_t)pr >= SH2_MAXHANDLER ) {
#ifdef LSB_FIRST
		A ^= 2;
#endif
		return *((unsigned short *)(pr + (A & SH2_PAGEM)));
	}
	
	return 0x0000;
}

SH2_INLINE UINT32 RL(UINT32 A)
{
/*	if (A >= 0xe0000000) return sh2_internal_r((A & 0x1fc)>>2, 0);
	if (A >= 0xc0000000) return program_read_dword_32be(A);
	if (A >= 0x40000000) return 0xa5a5a5a5;
	return program_read_dword_32be(A & AM);		*/
	
	unsigned char * pr;
	pr = pSh2Ext->MemMap[ A >> SH2_SHIFT ];
	if ( (uintptr_t)pr >= SH2_MAXHANDLER ) {
		//return (pr[(A & SH2_PAGEM) + 0] << 24) | (pr[(A & SH2_PAGEM) + 1] << 16) | (pr[(A & SH2_PAGEM) + 2] <<  8) | (pr[(A & SH2_PAGEM) + 3] <<  0);
		return *((unsigned int *)(pr + (A & SH2_PAGEM)));
	}
	return pSh2Ext->ReadLong[(uintptr_t)pr](A);
}

SH2_INLINE void WB(UINT32 A, UINT8 V)
{
/*	if (A >= 0xe0000000) { sh2_internal_w((A & 0x1fc)>>2, V << (((~A) & 3)*8), ~(0xff << (((~A) & 3)*8))); return; }
	if (A >= 0xc0000000) { program_write_byte_32be(A,V); return; }
	if (A >= 0x40000000) return;
	program_write_byte_32be(A & AM,V); */
	
	unsigned char* pr;
	pr = pSh2Ext->MemMap[(A >> SH2_SHIFT) + SH2_WADD];
	if ((uintptr_t)pr >= SH2_MAXHANDLER) {
#ifdef LSB_FIRST
		A ^= 3;
#endif
		pr[A & SH2_PAGEM] = (unsigned char)V;
		return;
	}
	pSh2Ext->WriteByte[(uintptr_t)pr](A, V);
}

SH2_INLINE void WW(UINT32 A, UINT16 V)
{
/*	if (A >= 0xe0000000) { sh2_internal_w((A & 0x1fc)>>2, V << (((~A) & 2)*8), ~(0xffff << (((~A) & 2)*8))); return; }
	if (A >= 0xc0000000) { program_write_word_32be(A,V); return; }
	if (A >= 0x40000000) return;
	program_write_word_32be(A & AM,V); */

	unsigned char * pr;
	pr = pSh2Ext->MemMap[(A >> SH2_SHIFT) + SH2_WADD];
	if ((uintptr_t)pr >= SH2_MAXHANDLER) {
#ifdef LSB_FIRST
		A ^= 2;
#endif
		*((unsigned short *)(pr + (A & SH2_PAGEM))) = (unsigned short)V;
		return;
	}
	pSh2Ext->WriteWord[(uintptr_t)pr](A, V);
}

SH2_INLINE void WL(UINT32 A, UINT32 V)
{
/*	if (A >= 0xe0000000) { sh2_internal_w((A & 0x1fc)>>2, V, 0); return; }
	if (A >= 0xc0000000) { program_write_dword_32be(A,V); return; }
	if (A >= 0x40000000) return;
	program_write_dword_32be(A & AM,V); */
	unsigned char * pr;
	pr = pSh2Ext->MemMap[(A >> SH2_SHIFT) + SH2_WADD];
	if ((uintptr_t)pr >= SH2_MAXHANDLER) {
		*((unsigned int *)(pr + (A & SH2_PAGEM))) = (unsigned int)V;
		return;
	}
	pSh2Ext->WriteLong[(uintptr_t)pr](A, V);
}

SH2_INLINE void sh2_exception(/*const char *message,*/ int irqline)
{
	int vector;

	if (irqline != 16)
	{
		if (irqline <= (signed int)((sh2->sr >> 4) & 15)) /* If the cpu forbids this interrupt */
			return;

		// if this is an sh2 internal irq, use its vector
		if (sh2->internal_irq_level == irqline)
		{
			vector = sh2->internal_irq_vector;
			//LOG(("SH-2 #%d exception #%d (internal vector: $%x) after [%s]\n", cpu_getactivecpu(), irqline, vector, message));
		}
		else
		{
			if(sh2->m[0x38] & 0x00010000)
			{
				//vector = sh2->irq_callback(irqline);
				//LOG(("SH-2 #%d exception #%d (external vector: $%x) after [%s]\n", cpu_getactivecpu(), irqline, vector, message));
				//bprintf(0, _T("SH-2 exception #%d (external vector: $%x)\n"), irqline, vector);
				vector = 64 + irqline/2;
				
			}
			else
			{
				//sh2->irq_callback(irqline);
				vector = 64 + irqline/2;
				//LOG(("SH-2 #%d exception #%d (autovector: $%x) after [%s]\n", cpu_getactivecpu(), irqline, vector, message));
			}
		}
	}
	else
	{
		vector = 11;
		//LOG(("SH-2 #%d nmi exception (autovector: $%x) after [%s]\n", cpu_getactivecpu(), vector, message));
	}

	sh2->r[15] -= 4;
	WL( sh2->r[15], sh2->sr );		/* push SR onto stack */
	sh2->r[15] -= 4;
	WL( sh2->r[15], sh2->pc );		/* push PC onto stack */

	/* set I flags in SR */
	if (irqline > SH2_INT_15)
		sh2->sr = sh2->sr | I;
	else
		sh2->sr = (sh2->sr & ~I) | (irqline << 4);

	/* fetch PC */
	sh2->pc = RL( sh2->vbr + vector * 4 );
	change_pc(sh2->pc & AM);
}

#define CHECK_PENDING_IRQ(/*message*/)			\
do {											\
	int irq = -1;								\
	if (sh2->pending_irq & (1 <<  0)) irq =	0;	\
	if (sh2->pending_irq & (1 <<  1)) irq =	1;	\
	if (sh2->pending_irq & (1 <<  2)) irq =	2;	\
	if (sh2->pending_irq & (1 <<  3)) irq =	3;	\
	if (sh2->pending_irq & (1 <<  4)) irq =	4;	\
	if (sh2->pending_irq & (1 <<  5)) irq =	5;	\
	if (sh2->pending_irq & (1 <<  6)) irq =	6;	\
	if (sh2->pending_irq & (1 <<  7)) irq =	7;	\
	if (sh2->pending_irq & (1 <<  8)) irq =	8;	\
	if (sh2->pending_irq & (1 <<  9)) irq =	9;	\
	if (sh2->pending_irq & (1 << 10)) irq = 10;	\
	if (sh2->pending_irq & (1 << 11)) irq = 11;	\
	if (sh2->pending_irq & (1 << 12)) irq = 12;	\
	if (sh2->pending_irq & (1 << 13)) irq = 13;	\
	if (sh2->pending_irq & (1 << 14)) irq = 14;	\
	if (sh2->pending_irq & (1 << 15)) irq = 15;	\
	if ((sh2->internal_irq_level != -1) && (sh2->internal_irq_level > irq)) irq = sh2->internal_irq_level; \
	if (irq >= 0)								\
		sh2_exception(/*message,*/irq); 			\
} while(0)


#if USE_JUMPTABLE

	#include "sh2op.c"
	
#else

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1100  1       -
 *  ADD     Rm,Rn
 */
SH2_INLINE void ADD(UINT32 m, UINT32 n)
{
	sh2->r[n] += sh2->r[m];
}

/*  code                 cycles  t-bit
 *  0111 nnnn iiii iiii  1       -
 *  ADD     #imm,Rn
 */
SH2_INLINE void ADDI(UINT32 i, UINT32 n)
{
	sh2->r[n] += (INT32)(INT16)(INT8)i;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1110  1       carry
 *  ADDC    Rm,Rn
 */
SH2_INLINE void ADDC(UINT32 m, UINT32 n)
{
	UINT32 tmp0, tmp1;

	tmp1 = sh2->r[n] + sh2->r[m];
	tmp0 = sh2->r[n];
	sh2->r[n] = tmp1 + (sh2->sr & T);
	if (tmp0 > tmp1)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
	if (tmp1 > sh2->r[n])
		sh2->sr |= T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 1111  1       overflow
 *  ADDV    Rm,Rn
 */
SH2_INLINE void ADDV(UINT32 m, UINT32 n)
{
	INT32 dest, src, ans;

	if ((INT32) sh2->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh2->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh2->r[n] += sh2->r[m];
	if ((INT32) sh2->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 0 || src == 2)
	{
		if (ans == 1)
			sh2->sr |= T;
		else
			sh2->sr &= ~T;
	}
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1001  1       -
 *  AND     Rm,Rn
 */
SH2_INLINE void AND(UINT32 m, UINT32 n)
{
	sh2->r[n] &= sh2->r[m];
}


/*  code                 cycles  t-bit
 *  1100 1001 iiii iiii  1       -
 *  AND     #imm,R0
 */
SH2_INLINE void ANDI(UINT32 i)
{
	sh2->r[0] &= i;
}

/*  code                 cycles  t-bit
 *  1100 1101 iiii iiii  1       -
 *  AND.B   #imm,@(R0,GBR)
 */
SH2_INLINE void ANDM(UINT32 i)
{
	UINT32 temp;

	sh2->ea = sh2->gbr + sh2->r[0];
	temp = i & RB( sh2->ea );
	WB( sh2->ea, temp );
	sh2->sh2_icount -= 2;
}

/*  code                 cycles  t-bit
 *  1000 1011 dddd dddd  3/1     -
 *  BF      disp8
 */
SH2_INLINE void BF(UINT32 d)
{
	if ((sh2->sr & T) == 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh2->pc = sh2->ea = sh2->pc + disp * 2 + 2;
		change_pc(sh2->pc & AM);
		sh2->sh2_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1111 dddd dddd  3/1     -
 *  BFS     disp8
 */
SH2_INLINE void BFS(UINT32 d)
{
	if ((sh2->sr & T) == 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh2->delay = sh2->pc;
		sh2->pc = sh2->ea = sh2->pc + disp * 2 + 2;
		sh2->sh2_icount--;
	}
}

/*  code                 cycles  t-bit
 *  1010 dddd dddd dddd  2       -
 *  BRA     disp12
 */
SH2_INLINE void BRA(UINT32 d)
{
	INT32 disp = ((INT32)d << 20) >> 20;

#if BUSY_LOOP_HACKS
	if (disp == -2)
	{
		UINT32 next_opcode = RW(sh2->ppc & AM);
		//UINT32 next_opcode = OPRW(sh2->ppc & AM);
		
		/* BRA  $
         * NOP
         */
		if (next_opcode == 0x0009){
			//bprintf(0, _T("SH2: BUSY_LOOP_HACKS %d\n"), sh2->sh2_icount);
			sh2->sh2_total_cycles += sh2->sh2_icount;
			sh2->sh2_icount %= 3;	/* cycles for BRA $ and NOP taken (3) */
			sh2->sh2_total_cycles -= sh2->sh2_icount;
		}
	}
#endif
	sh2->delay = sh2->pc;
	sh2->pc = sh2->ea = sh2->pc + disp * 2 + 2;
	sh2->sh2_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0010 0011  2       -
 *  BRAF    Rm
 */
SH2_INLINE void BRAF(UINT32 m)
{
	sh2->delay = sh2->pc;
	sh2->pc += sh2->r[m] + 2;
	sh2->sh2_icount--;
}

/*  code                 cycles  t-bit
 *  1011 dddd dddd dddd  2       -
 *  BSR     disp12
 */
SH2_INLINE void BSR(UINT32 d)
{
	INT32 disp = ((INT32)d << 20) >> 20;

	sh2->pr = sh2->pc + 2;
	sh2->delay = sh2->pc;
	sh2->pc = sh2->ea = sh2->pc + disp * 2 + 2;
	sh2->sh2_icount--;
}

/*  code                 cycles  t-bit
 *  0000 mmmm 0000 0011  2       -
 *  BSRF    Rm
 */
SH2_INLINE void BSRF(UINT32 m)
{
	sh2->pr = sh2->pc + 2;
	sh2->delay = sh2->pc;
	sh2->pc += sh2->r[m] + 2;
	sh2->sh2_icount--;
}

/*  code                 cycles  t-bit
 *  1000 1001 dddd dddd  3/1     -
 *  BT      disp8
 */
SH2_INLINE void BT(UINT32 d)
{
	if ((sh2->sr & T) != 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh2->pc = sh2->ea = sh2->pc + disp * 2 + 2;
		change_pc(sh2->pc & AM);
		sh2->sh2_icount -= 2;
	}
}

/*  code                 cycles  t-bit
 *  1000 1101 dddd dddd  2/1     -
 *  BTS     disp8
 */
SH2_INLINE void BTS(UINT32 d)
{
	if ((sh2->sr & T) != 0)
	{
		INT32 disp = ((INT32)d << 24) >> 24;
		sh2->delay = sh2->pc;
		sh2->pc = sh2->ea = sh2->pc + disp * 2 + 2;
		sh2->sh2_icount--;
	}
}

/*  code                 cycles  t-bit
 *  0000 0000 0010 1000  1       -
 *  CLRMAC
 */
SH2_INLINE void CLRMAC(void)
{
	sh2->mach = 0;
	sh2->macl = 0;
}

/*  code                 cycles  t-bit
 *  0000 0000 0000 1000  1       -
 *  CLRT
 */
SH2_INLINE void CLRT(void)
{
	sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0000  1       comparison result
 *  CMP_EQ  Rm,Rn
 */
SH2_INLINE void CMPEQ(UINT32 m, UINT32 n)
{
	if (sh2->r[n] == sh2->r[m])
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0011  1       comparison result
 *  CMP_GE  Rm,Rn
 */
SH2_INLINE void CMPGE(UINT32 m, UINT32 n)
{
	if ((INT32) sh2->r[n] >= (INT32) sh2->r[m])
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0111  1       comparison result
 *  CMP_GT  Rm,Rn
 */
SH2_INLINE void CMPGT(UINT32 m, UINT32 n)
{
	if ((INT32) sh2->r[n] > (INT32) sh2->r[m])
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0110  1       comparison result
 *  CMP_HI  Rm,Rn
 */
SH2_INLINE void CMPHI(UINT32 m, UINT32 n)
{
	if ((UINT32) sh2->r[n] > (UINT32) sh2->r[m])
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0010  1       comparison result
 *  CMP_HS  Rm,Rn
 */
SH2_INLINE void CMPHS(UINT32 m, UINT32 n)
{
	if ((UINT32) sh2->r[n] >= (UINT32) sh2->r[m])
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}


/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0101  1       comparison result
 *  CMP_PL  Rn
 */
SH2_INLINE void CMPPL(UINT32 n)
{
	if ((INT32) sh2->r[n] > 0)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0100 nnnn 0001 0001  1       comparison result
 *  CMP_PZ  Rn
 */
SH2_INLINE void CMPPZ(UINT32 n)
{
	if ((INT32) sh2->r[n] >= 0)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 1100  1       comparison result
 * CMP_STR  Rm,Rn
 */
SH2_INLINE void CMPSTR(UINT32 m, UINT32 n)
 {
  UINT32 temp;
  INT32 HH, HL, LH, LL;
  temp = sh2->r[n] ^ sh2->r[m];
  HH = (temp >> 24) & 0xff;
  HL = (temp >> 16) & 0xff;
  LH = (temp >> 8) & 0xff;
  LL = temp & 0xff;
  if (HH && HL && LH && LL)
   sh2->sr &= ~T;
  else
   sh2->sr |= T;
 }


/*  code                 cycles  t-bit
 *  1000 1000 iiii iiii  1       comparison result
 *  CMP/EQ #imm,R0
 */
SH2_INLINE void CMPIM(UINT32 i)
{
	UINT32 imm = (UINT32)(INT32)(INT16)(INT8)i;

	if (sh2->r[0] == imm)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0010 nnnn mmmm 0111  1       calculation result
 *  DIV0S   Rm,Rn
 */
SH2_INLINE void DIV0S(UINT32 m, UINT32 n)
{
	if ((sh2->r[n] & 0x80000000) == 0)
		sh2->sr &= ~Q;
	else
		sh2->sr |= Q;
	if ((sh2->r[m] & 0x80000000) == 0)
		sh2->sr &= ~M;
	else
		sh2->sr |= M;
	if ((sh2->r[m] ^ sh2->r[n]) & 0x80000000)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  code                 cycles  t-bit
 *  0000 0000 0001 1001  1       0
 *  DIV0U
 */
SH2_INLINE void DIV0U(void)
{
	sh2->sr &= ~(M | Q | T);
}

/*  code                 cycles  t-bit
 *  0011 nnnn mmmm 0100  1       calculation result
 *  DIV1 Rm,Rn
 */
SH2_INLINE void DIV1(UINT32 m, UINT32 n)
{
	UINT32 tmp0;
	UINT32 old_q;

	old_q = sh2->sr & Q;
	if (0x80000000 & sh2->r[n])
		sh2->sr |= Q;
	else
		sh2->sr &= ~Q;

	sh2->r[n] = (sh2->r[n] << 1) | (sh2->sr & T);

	if (!old_q)
	{
		if (!(sh2->sr & M))
		{
			tmp0 = sh2->r[n];
			sh2->r[n] -= sh2->r[m];
			if(!(sh2->sr & Q))
				if(sh2->r[n] > tmp0)
					sh2->sr |= Q;
				else
					sh2->sr &= ~Q;
			else
				if(sh2->r[n] > tmp0)
					sh2->sr &= ~Q;
				else
					sh2->sr |= Q;
		}
		else
		{
			tmp0 = sh2->r[n];
			sh2->r[n] += sh2->r[m];
			if(!(sh2->sr & Q))
			{
				if(sh2->r[n] < tmp0)
					sh2->sr &= ~Q;
				else
					sh2->sr |= Q;
			}
			else
			{
				if(sh2->r[n] < tmp0)
					sh2->sr |= Q;
				else
					sh2->sr &= ~Q;
			}
		}
	}
	else
	{
		if (!(sh2->sr & M))
		{
			tmp0 = sh2->r[n];
			sh2->r[n] += sh2->r[m];
			if(!(sh2->sr & Q))
				if(sh2->r[n] < tmp0)
					sh2->sr |= Q;
				else
					sh2->sr &= ~Q;
			else
				if(sh2->r[n] < tmp0)
					sh2->sr &= ~Q;
				else
					sh2->sr |= Q;
		}
		else
		{
			tmp0 = sh2->r[n];
			sh2->r[n] -= sh2->r[m];
			if(!(sh2->sr & Q))
				if(sh2->r[n] > tmp0)
					sh2->sr &= ~Q;
				else
					sh2->sr |= Q;
			else
				if(sh2->r[n] > tmp0)
					sh2->sr |= Q;
				else
					sh2->sr &= ~Q;
		}
	}

	tmp0 = (sh2->sr & (Q | M));
	if((!tmp0) || (tmp0 == 0x300)) /* if Q == M set T else clear T */
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  DMULS.L Rm,Rn */
SH2_INLINE void DMULS(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) sh2->r[n];
	tempm = (INT32) sh2->r[m];
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	if ((INT32) (sh2->r[n] ^ sh2->r[m]) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	sh2->mach = Res2;
	sh2->macl = Res0;
	sh2->sh2_icount--;
}

/*  DMULU.L Rm,Rn */
SH2_INLINE void DMULU(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;

	RnL = sh2->r[n] & 0x0000ffff;
	RnH = (sh2->r[n] >> 16) & 0x0000ffff;
	RmL = sh2->r[m] & 0x0000ffff;
	RmH = (sh2->r[m] >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	sh2->mach = Res2;
	sh2->macl = Res0;
	sh2->sh2_icount--;
}

/*  DT      Rn */
SH2_INLINE void DT(UINT32 n)
{
	sh2->r[n]--;
	if (sh2->r[n] == 0)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
#if BUSY_LOOP_HACKS
	{
		UINT32 next_opcode = RW(sh2->ppc & AM);
		//UINT32 next_opcode = OPRW(sh2->ppc & AM);
		/* DT   Rn
         * BF   $-2
         */
		if (next_opcode == 0x8bfd)
		{
			//bprintf(0, _T("SH2: BUSY_LOOP_HACKS (%d)--; \n"), sh2->r[n], sh2->sh2_icount);
			while (sh2->r[n] > 1 && sh2->sh2_icount > 4)
			{
				sh2->r[n]--;
				sh2->sh2_icount -= 4;	/* cycles for DT (1) and BF taken (3) */
				sh2->sh2_total_cycles += 4;
			}
		}
	}
#endif
}

/*  EXTS.B  Rm,Rn */
SH2_INLINE void EXTSB(UINT32 m, UINT32 n)
{
	sh2->r[n] = ((INT32)sh2->r[m] << 24) >> 24;
}

/*  EXTS.W  Rm,Rn */
SH2_INLINE void EXTSW(UINT32 m, UINT32 n)
{
	sh2->r[n] = ((INT32)sh2->r[m] << 16) >> 16;
}

/*  EXTU.B  Rm,Rn */
SH2_INLINE void EXTUB(UINT32 m, UINT32 n)
{
	sh2->r[n] = sh2->r[m] & 0x000000ff;
}

/*  EXTU.W  Rm,Rn */
SH2_INLINE void EXTUW(UINT32 m, UINT32 n)
{
	sh2->r[n] = sh2->r[m] & 0x0000ffff;
}

/*  JMP     @Rm */
SH2_INLINE void JMP(UINT32 m)
{
	sh2->delay = sh2->pc;
	sh2->pc = sh2->ea = sh2->r[m];
}

/*  JSR     @Rm */
SH2_INLINE void JSR(UINT32 m)
{
	sh2->delay = sh2->pc;
	sh2->pr = sh2->pc + 2;
	sh2->pc = sh2->ea = sh2->r[m];
	sh2->sh2_icount--;
}


/*  LDC     Rm,SR */
SH2_INLINE void LDCSR(UINT32 m)
{
	sh2->sr = sh2->r[m] & FLAGS;
	sh2->test_irq = 1;
}

/*  LDC     Rm,GBR */
SH2_INLINE void LDCGBR(UINT32 m)
{
	sh2->gbr = sh2->r[m];
}

/*  LDC     Rm,VBR */
SH2_INLINE void LDCVBR(UINT32 m)
{
	sh2->vbr = sh2->r[m];
}

/*  LDC.L   @Rm+,SR */
SH2_INLINE void LDCMSR(UINT32 m)
{
	sh2->ea = sh2->r[m];
	sh2->sr = RL( sh2->ea ) & FLAGS;
	sh2->r[m] += 4;
	sh2->sh2_icount -= 2;
	sh2->test_irq = 1;
}

/*  LDC.L   @Rm+,GBR */
SH2_INLINE void LDCMGBR(UINT32 m)
{
	sh2->ea = sh2->r[m];
	sh2->gbr = RL( sh2->ea );
	sh2->r[m] += 4;
	sh2->sh2_icount -= 2;
}

/*  LDC.L   @Rm+,VBR */
SH2_INLINE void LDCMVBR(UINT32 m)
{
	sh2->ea = sh2->r[m];
	sh2->vbr = RL( sh2->ea );
	sh2->r[m] += 4;
	sh2->sh2_icount -= 2;
}

/*  LDS     Rm,MACH */
SH2_INLINE void LDSMACH(UINT32 m)
{
	sh2->mach = sh2->r[m];
}

/*  LDS     Rm,MACL */
SH2_INLINE void LDSMACL(UINT32 m)
{
	sh2->macl = sh2->r[m];
}

/*  LDS     Rm,PR */
SH2_INLINE void LDSPR(UINT32 m)
{
	sh2->pr = sh2->r[m];
}

/*  LDS.L   @Rm+,MACH */
SH2_INLINE void LDSMMACH(UINT32 m)
{
	sh2->ea = sh2->r[m];
	sh2->mach = RL( sh2->ea );
	sh2->r[m] += 4;
}

/*  LDS.L   @Rm+,MACL */
SH2_INLINE void LDSMMACL(UINT32 m)
{
	sh2->ea = sh2->r[m];
	sh2->macl = RL( sh2->ea );
	sh2->r[m] += 4;
}

/*  LDS.L   @Rm+,PR */
SH2_INLINE void LDSMPR(UINT32 m)
{
	sh2->ea = sh2->r[m];
	sh2->pr = RL( sh2->ea );
	sh2->r[m] += 4;
}

/*  MAC.L   @Rm+,@Rn+ */
SH2_INLINE void MAC_L(UINT32 m, UINT32 n)
{
	UINT32 RnL, RnH, RmL, RmH, Res0, Res1, Res2;
	UINT32 temp0, temp1, temp2, temp3;
	INT32 tempm, tempn, fnLmL;

	tempn = (INT32) RL( sh2->r[n] );
	sh2->r[n] += 4;
	tempm = (INT32) RL( sh2->r[m] );
	sh2->r[m] += 4;
	if ((INT32) (tempn ^ tempm) < 0)
		fnLmL = -1;
	else
		fnLmL = 0;
	if (tempn < 0)
		tempn = 0 - tempn;
	if (tempm < 0)
		tempm = 0 - tempm;
	temp1 = (UINT32) tempn;
	temp2 = (UINT32) tempm;
	RnL = temp1 & 0x0000ffff;
	RnH = (temp1 >> 16) & 0x0000ffff;
	RmL = temp2 & 0x0000ffff;
	RmH = (temp2 >> 16) & 0x0000ffff;
	temp0 = RmL * RnL;
	temp1 = RmH * RnL;
	temp2 = RmL * RnH;
	temp3 = RmH * RnH;
	Res2 = 0;
	Res1 = temp1 + temp2;
	if (Res1 < temp1)
		Res2 += 0x00010000;
	temp1 = (Res1 << 16) & 0xffff0000;
	Res0 = temp0 + temp1;
	if (Res0 < temp0)
		Res2++;
	Res2 = Res2 + ((Res1 >> 16) & 0x0000ffff) + temp3;
	if (fnLmL < 0)
	{
		Res2 = ~Res2;
		if (Res0 == 0)
			Res2++;
		else
			Res0 = (~Res0) + 1;
	}
	if (sh2->sr & S)
	{
		Res0 = sh2->macl + Res0;
		if (sh2->macl > Res0)
			Res2++;
		Res2 += (sh2->mach & 0x0000ffff);
		if (((INT32) Res2 < 0) && (Res2 < 0xffff8000))
		{
			Res2 = 0x00008000;
			Res0 = 0x00000000;
		}
		else if (((INT32) Res2 > 0) && (Res2 > 0x00007fff))
		{
			Res2 = 0x00007fff;
			Res0 = 0xffffffff;
		}
		sh2->mach = Res2;
		sh2->macl = Res0;
	}
	else
	{
		Res0 = sh2->macl + Res0;
		if (sh2->macl > Res0)
			Res2++;
		Res2 += sh2->mach;
		sh2->mach = Res2;
		sh2->macl = Res0;
	}
	sh2->sh2_icount -= 2;
}

/*  MAC.W   @Rm+,@Rn+ */
SH2_INLINE void MAC_W(UINT32 m, UINT32 n)
{{
	INT32 tempm, tempn, dest, src, ans;
	UINT32 templ;

	tempn = (INT32) RW( sh2->r[n] );
	sh2->r[n] += 2;
	tempm = (INT32) RW( sh2->r[m] );
	sh2->r[m] += 2;
	templ = sh2->macl;
	tempm = ((INT32) (short) tempn * (INT32) (short) tempm);
	if ((INT32) sh2->macl >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) tempm >= 0)
	{
		src = 0;
		tempn = 0;
	}
	else
	{
		src = 1;
		tempn = 0xffffffff;
	}
	src += dest;
	sh2->macl += tempm;
	if ((INT32) sh2->macl >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (sh2->sr & S)
	{
		if (ans == 1)
			{
				if (src == 0)
					sh2->macl = 0x7fffffff;
				if (src == 2)
					sh2->macl = 0x80000000;
			}
	}
	else
	{
		sh2->mach += tempn;
		if (templ > sh2->macl)
			sh2->mach += 1;
		}
	sh2->sh2_icount -= 2;
}}

/*  MOV     Rm,Rn */
SH2_INLINE void MOV(UINT32 m, UINT32 n)
{
	sh2->r[n] = sh2->r[m];
}

/*  MOV.B   Rm,@Rn */
SH2_INLINE void MOVBS(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[n];
	WB( sh2->ea, sh2->r[m] & 0x000000ff);
}

/*  MOV.W   Rm,@Rn */
SH2_INLINE void MOVWS(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[n];
	WW( sh2->ea, sh2->r[m] & 0x0000ffff);
}

/*  MOV.L   Rm,@Rn */
SH2_INLINE void MOVLS(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[n];
	WL( sh2->ea, sh2->r[m] );
}

/*  MOV.B   @Rm,Rn */
SH2_INLINE void MOVBL(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[m];
	sh2->r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->ea );
}

/*  MOV.W   @Rm,Rn */
SH2_INLINE void MOVWL(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[m];
	sh2->r[n] = (UINT32)(INT32)(INT16) RW( sh2->ea );
}

/*  MOV.L   @Rm,Rn */
SH2_INLINE void MOVLL(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[m];
	sh2->r[n] = RL( sh2->ea );
}

/*  MOV.B   Rm,@-Rn */
SH2_INLINE void MOVBM(UINT32 m, UINT32 n)
{
	/* SMG : bug fix, was reading sh2->r[n] */
	UINT32 data = sh2->r[m] & 0x000000ff;

	sh2->r[n] -= 1;
	WB( sh2->r[n], data );
}

/*  MOV.W   Rm,@-Rn */
SH2_INLINE void MOVWM(UINT32 m, UINT32 n)
{
	UINT32 data = sh2->r[m] & 0x0000ffff;

	sh2->r[n] -= 2;
	WW( sh2->r[n], data );
}

/*  MOV.L   Rm,@-Rn */
SH2_INLINE void MOVLM(UINT32 m, UINT32 n)
{
	UINT32 data = sh2->r[m];

	sh2->r[n] -= 4;
	WL( sh2->r[n], data );
}

/*  MOV.B   @Rm+,Rn */
SH2_INLINE void MOVBP(UINT32 m, UINT32 n)
{
	sh2->r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->r[m] );
	if (n != m)
		sh2->r[m] += 1;
}

/*  MOV.W   @Rm+,Rn */
SH2_INLINE void MOVWP(UINT32 m, UINT32 n)
{
	sh2->r[n] = (UINT32)(INT32)(INT16) RW( sh2->r[m] );
	if (n != m)
		sh2->r[m] += 2;
}

/*  MOV.L   @Rm+,Rn */
SH2_INLINE void MOVLP(UINT32 m, UINT32 n)
{
	sh2->r[n] = RL( sh2->r[m] );
	if (n != m)
		sh2->r[m] += 4;
}

/*  MOV.B   Rm,@(R0,Rn) */
SH2_INLINE void MOVBS0(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[n] + sh2->r[0];
	WB( sh2->ea, sh2->r[m] & 0x000000ff );
}

/*  MOV.W   Rm,@(R0,Rn) */
SH2_INLINE void MOVWS0(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[n] + sh2->r[0];
	WW( sh2->ea, sh2->r[m] & 0x0000ffff );
}

/*  MOV.L   Rm,@(R0,Rn) */
SH2_INLINE void MOVLS0(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[n] + sh2->r[0];
	WL( sh2->ea, sh2->r[m] );
}

/*  MOV.B   @(R0,Rm),Rn */
SH2_INLINE void MOVBL0(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[m] + sh2->r[0];
	sh2->r[n] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->ea );
}

/*  MOV.W   @(R0,Rm),Rn */
SH2_INLINE void MOVWL0(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[m] + sh2->r[0];
	sh2->r[n] = (UINT32)(INT32)(INT16) RW( sh2->ea );
}

/*  MOV.L   @(R0,Rm),Rn */
SH2_INLINE void MOVLL0(UINT32 m, UINT32 n)
{
	sh2->ea = sh2->r[m] + sh2->r[0];
	sh2->r[n] = RL( sh2->ea );
}

/*  MOV     #imm,Rn */
SH2_INLINE void MOVI(UINT32 i, UINT32 n)
{
	sh2->r[n] = (UINT32)(INT32)(INT16)(INT8) i;
}

/*  MOV.W   @(disp8,PC),Rn */
SH2_INLINE void MOVWI(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0xff;
	sh2->ea = sh2->pc + disp * 2 + 2;
	sh2->r[n] = (UINT32)(INT32)(INT16) RW( sh2->ea );
}

/*  MOV.L   @(disp8,PC),Rn */
SH2_INLINE void MOVLI(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0xff;
	sh2->ea = ((sh2->pc + 2) & ~3) + disp * 4;
	sh2->r[n] = RL( sh2->ea );
}

/*  MOV.B   @(disp8,GBR),R0 */
SH2_INLINE void MOVBLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2->ea = sh2->gbr + disp;
	sh2->r[0] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->ea );
}

/*  MOV.W   @(disp8,GBR),R0 */
SH2_INLINE void MOVWLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2->ea = sh2->gbr + disp * 2;
	sh2->r[0] = (INT32)(INT16) RW( sh2->ea );
}

/*  MOV.L   @(disp8,GBR),R0 */
SH2_INLINE void MOVLLG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2->ea = sh2->gbr + disp * 4;
	sh2->r[0] = RL( sh2->ea );
}

/*  MOV.B   R0,@(disp8,GBR) */
SH2_INLINE void MOVBSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2->ea = sh2->gbr + disp;
	WB( sh2->ea, sh2->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp8,GBR) */
SH2_INLINE void MOVWSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2->ea = sh2->gbr + disp * 2;
	WW( sh2->ea, sh2->r[0] & 0x0000ffff );
}

/*  MOV.L   R0,@(disp8,GBR) */
SH2_INLINE void MOVLSG(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2->ea = sh2->gbr + disp * 4;
	WL( sh2->ea, sh2->r[0] );
}

/*  MOV.B   R0,@(disp4,Rn) */
SH2_INLINE void MOVBS4(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh2->ea = sh2->r[n] + disp;
	WB( sh2->ea, sh2->r[0] & 0x000000ff );
}

/*  MOV.W   R0,@(disp4,Rn) */
SH2_INLINE void MOVWS4(UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh2->ea = sh2->r[n] + disp * 2;
	WW( sh2->ea, sh2->r[0] & 0x0000ffff );
}

/* MOV.L Rm,@(disp4,Rn) */
SH2_INLINE void MOVLS4(UINT32 m, UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh2->ea = sh2->r[n] + disp * 4;
	WL( sh2->ea, sh2->r[m] );
}

/*  MOV.B   @(disp4,Rm),R0 */
SH2_INLINE void MOVBL4(UINT32 m, UINT32 d)
{
	UINT32 disp = d & 0x0f;
	sh2->ea = sh2->r[m] + disp;
	sh2->r[0] = (UINT32)(INT32)(INT16)(INT8) RB( sh2->ea );
}

/*  MOV.W   @(disp4,Rm),R0 */
SH2_INLINE void MOVWL4(UINT32 m, UINT32 d)
{
	UINT32 disp = d & 0x0f;
	sh2->ea = sh2->r[m] + disp * 2;
	sh2->r[0] = (UINT32)(INT32)(INT16) RW( sh2->ea );
}

/*  MOV.L   @(disp4,Rm),Rn */
SH2_INLINE void MOVLL4(UINT32 m, UINT32 d, UINT32 n)
{
	UINT32 disp = d & 0x0f;
	sh2->ea = sh2->r[m] + disp * 4;
	sh2->r[n] = RL( sh2->ea );
}

/*  MOVA    @(disp8,PC),R0 */
SH2_INLINE void MOVA(UINT32 d)
{
	UINT32 disp = d & 0xff;
	sh2->ea = ((sh2->pc + 2) & ~3) + disp * 4;
	sh2->r[0] = sh2->ea;
}

/*  MOVT    Rn */
SH2_INLINE void MOVT(UINT32 n)
{
	sh2->r[n] = sh2->sr & T;
}

/*  MUL.L   Rm,Rn */
SH2_INLINE void MULL(UINT32 m, UINT32 n)
{
	sh2->macl = sh2->r[n] * sh2->r[m];
	sh2->sh2_icount--;
}

/*  MULS    Rm,Rn */
SH2_INLINE void MULS(UINT32 m, UINT32 n)
{
	sh2->macl = (INT16) sh2->r[n] * (INT16) sh2->r[m];
}

/*  MULU    Rm,Rn */
SH2_INLINE void MULU(UINT32 m, UINT32 n)
{
	sh2->macl = (UINT16) sh2->r[n] * (UINT16) sh2->r[m];
}

/*  NEG     Rm,Rn */
SH2_INLINE void NEG(UINT32 m, UINT32 n)
{
	sh2->r[n] = 0 - sh2->r[m];
}

/*  NEGC    Rm,Rn */
SH2_INLINE void NEGC(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = sh2->r[m];
	sh2->r[n] = -temp - (sh2->sr & T);
	if (temp || (sh2->sr & T))
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  NOP */
SH2_INLINE void NOP(void)
{
}

/*  NOT     Rm,Rn */
SH2_INLINE void NOT(UINT32 m, UINT32 n)
{
	sh2->r[n] = ~sh2->r[m];
}

/*  OR      Rm,Rn */
SH2_INLINE void OR(UINT32 m, UINT32 n)
{
	sh2->r[n] |= sh2->r[m];
}

/*  OR      #imm,R0 */
SH2_INLINE void ORI(UINT32 i)
{
	sh2->r[0] |= i;
	sh2->sh2_icount -= 2;
}

/*  OR.B    #imm,@(R0,GBR) */
SH2_INLINE void ORM(UINT32 i)
{
	UINT32 temp;

	sh2->ea = sh2->gbr + sh2->r[0];
	temp = RB( sh2->ea );
	temp |= i;
	WB( sh2->ea, temp );
}

/*  ROTCL   Rn */
SH2_INLINE void ROTCL(UINT32 n)
{
	UINT32 temp;

	temp = (sh2->r[n] >> 31) & T;
	sh2->r[n] = (sh2->r[n] << 1) | (sh2->sr & T);
	sh2->sr = (sh2->sr & ~T) | temp;
}

/*  ROTCR   Rn */
SH2_INLINE void ROTCR(UINT32 n)
{
	UINT32 temp;
	temp = (sh2->sr & T) << 31;
	if (sh2->r[n] & T)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
	sh2->r[n] = (sh2->r[n] >> 1) | temp;
}

/*  ROTL    Rn */
SH2_INLINE void ROTL(UINT32 n)
{
	sh2->sr = (sh2->sr & ~T) | ((sh2->r[n] >> 31) & T);
	sh2->r[n] = (sh2->r[n] << 1) | (sh2->r[n] >> 31);
}

/*  ROTR    Rn */
SH2_INLINE void ROTR(UINT32 n)
{
	sh2->sr = (sh2->sr & ~T) | (sh2->r[n] & T);
	sh2->r[n] = (sh2->r[n] >> 1) | (sh2->r[n] << 31);
}

/*  RTE */
SH2_INLINE void RTE(void)
{
	sh2->ea = sh2->r[15];
	sh2->delay = sh2->pc;
	sh2->pc = RL( sh2->ea );
	sh2->r[15] += 4;
	sh2->ea = sh2->r[15];
	sh2->sr = RL( sh2->ea ) & FLAGS;
	sh2->r[15] += 4;
	sh2->sh2_icount -= 3;
	sh2->test_irq = 1;
}

/*  RTS */
SH2_INLINE void RTS(void)
{
	sh2->delay = sh2->pc;
	sh2->pc = sh2->ea = sh2->pr;
	sh2->sh2_icount--;
}

/*  SETT */
SH2_INLINE void SETT(void)
{
	sh2->sr |= T;
}

/*  SHAL    Rn      (same as SHLL) */
SH2_INLINE void SHAL(UINT32 n)
{
	sh2->sr = (sh2->sr & ~T) | ((sh2->r[n] >> 31) & T);
	sh2->r[n] <<= 1;
}

/*  SHAR    Rn */
SH2_INLINE void SHAR(UINT32 n)
{
	sh2->sr = (sh2->sr & ~T) | (sh2->r[n] & T);
	sh2->r[n] = (UINT32)((INT32)sh2->r[n] >> 1);
}

/*  SHLL    Rn      (same as SHAL) */
SH2_INLINE void SHLL(UINT32 n)
{
	sh2->sr = (sh2->sr & ~T) | ((sh2->r[n] >> 31) & T);
	sh2->r[n] <<= 1;
}

/*  SHLL2   Rn */
SH2_INLINE void SHLL2(UINT32 n)
{
	sh2->r[n] <<= 2;
}

/*  SHLL8   Rn */
SH2_INLINE void SHLL8(UINT32 n)
{
	sh2->r[n] <<= 8;
}

/*  SHLL16  Rn */
SH2_INLINE void SHLL16(UINT32 n)
{
	sh2->r[n] <<= 16;
}

/*  SHLR    Rn */
SH2_INLINE void SHLR(UINT32 n)
{
	sh2->sr = (sh2->sr & ~T) | (sh2->r[n] & T);
	sh2->r[n] >>= 1;
}

/*  SHLR2   Rn */
SH2_INLINE void SHLR2(UINT32 n)
{
	sh2->r[n] >>= 2;
}

/*  SHLR8   Rn */
SH2_INLINE void SHLR8(UINT32 n)
{
	sh2->r[n] >>= 8;
}

/*  SHLR16  Rn */
SH2_INLINE void SHLR16(UINT32 n)
{
	sh2->r[n] >>= 16;
}

/*  SLEEP */
SH2_INLINE void SLEEP(void)
{
	sh2->pc -= 2;
	sh2->sh2_icount -= 2;
	/* Wait_for_exception; */
}

/*  STC     SR,Rn */
SH2_INLINE void STCSR(UINT32 n)
{
	sh2->r[n] = sh2->sr;
}

/*  STC     GBR,Rn */
SH2_INLINE void STCGBR(UINT32 n)
{
	sh2->r[n] = sh2->gbr;
}

/*  STC     VBR,Rn */
SH2_INLINE void STCVBR(UINT32 n)
{
	sh2->r[n] = sh2->vbr;
}

/*  STC.L   SR,@-Rn */
SH2_INLINE void STCMSR(UINT32 n)
{
	sh2->r[n] -= 4;
	sh2->ea = sh2->r[n];
	WL( sh2->ea, sh2->sr );
	sh2->sh2_icount--;
}

/*  STC.L   GBR,@-Rn */
SH2_INLINE void STCMGBR(UINT32 n)
{
	sh2->r[n] -= 4;
	sh2->ea = sh2->r[n];
	WL( sh2->ea, sh2->gbr );
	sh2->sh2_icount--;
}

/*  STC.L   VBR,@-Rn */
SH2_INLINE void STCMVBR(UINT32 n)
{
	sh2->r[n] -= 4;
	sh2->ea = sh2->r[n];
	WL( sh2->ea, sh2->vbr );
	sh2->sh2_icount--;
}

/*  STS     MACH,Rn */
SH2_INLINE void STSMACH(UINT32 n)
{
	sh2->r[n] = sh2->mach;
}

/*  STS     MACL,Rn */
SH2_INLINE void STSMACL(UINT32 n)
{
	sh2->r[n] = sh2->macl;
}

/*  STS     PR,Rn */
SH2_INLINE void STSPR(UINT32 n)
{
	sh2->r[n] = sh2->pr;
}

/*  STS.L   MACH,@-Rn */
SH2_INLINE void STSMMACH(UINT32 n)
{
	sh2->r[n] -= 4;
	sh2->ea = sh2->r[n];
	WL( sh2->ea, sh2->mach );
}

/*  STS.L   MACL,@-Rn */
SH2_INLINE void STSMMACL(UINT32 n)
{
	sh2->r[n] -= 4;
	sh2->ea = sh2->r[n];
	WL( sh2->ea, sh2->macl );
}

/*  STS.L   PR,@-Rn */
SH2_INLINE void STSMPR(UINT32 n)
{
	sh2->r[n] -= 4;
	sh2->ea = sh2->r[n];
	WL( sh2->ea, sh2->pr );
}

/*  SUB     Rm,Rn */
SH2_INLINE void SUB(UINT32 m, UINT32 n)
{
	sh2->r[n] -= sh2->r[m];
}

/*  SUBC    Rm,Rn */
SH2_INLINE void SUBC(UINT32 m, UINT32 n)
{
	UINT32 tmp0, tmp1;

	tmp1 = sh2->r[n] - sh2->r[m];
	tmp0 = sh2->r[n];
	sh2->r[n] = tmp1 - (sh2->sr & T);
	if (tmp0 < tmp1)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
	if (tmp1 < sh2->r[n])
		sh2->sr |= T;
}

/*  SUBV    Rm,Rn */
SH2_INLINE void SUBV(UINT32 m, UINT32 n)
{
	INT32 dest, src, ans;

	if ((INT32) sh2->r[n] >= 0)
		dest = 0;
	else
		dest = 1;
	if ((INT32) sh2->r[m] >= 0)
		src = 0;
	else
		src = 1;
	src += dest;
	sh2->r[n] -= sh2->r[m];
	if ((INT32) sh2->r[n] >= 0)
		ans = 0;
	else
		ans = 1;
	ans += dest;
	if (src == 1)
	{
		if (ans == 1)
			sh2->sr |= T;
		else
			sh2->sr &= ~T;
	}
	else
		sh2->sr &= ~T;
}

/*  SWAP.B  Rm,Rn */
SH2_INLINE void SWAPB(UINT32 m, UINT32 n)
{
	UINT32 temp0, temp1;

	temp0 = sh2->r[m] & 0xffff0000;
	temp1 = (sh2->r[m] & 0x000000ff) << 8;
	sh2->r[n] = (sh2->r[m] >> 8) & 0x000000ff;
	sh2->r[n] = sh2->r[n] | temp1 | temp0;
}

/*  SWAP.W  Rm,Rn */
SH2_INLINE void SWAPW(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = (sh2->r[m] >> 16) & 0x0000ffff;
	sh2->r[n] = (sh2->r[m] << 16) | temp;
}

/*  TAS.B   @Rn */
SH2_INLINE void TAS(UINT32 n)
{
	UINT32 temp;
	sh2->ea = sh2->r[n];
	/* Bus Lock enable */
	temp = RB( sh2->ea );
	if (temp == 0)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
	temp |= 0x80;
	/* Bus Lock disable */
	WB( sh2->ea, temp );
	sh2->sh2_icount -= 3;
}

/*  TRAPA   #imm */
SH2_INLINE void TRAPA(UINT32 i)
{
	UINT32 imm = i & 0xff;

	sh2->ea = sh2->vbr + imm * 4;

	sh2->r[15] -= 4;
	WL( sh2->r[15], sh2->sr );
	sh2->r[15] -= 4;
	WL( sh2->r[15], sh2->pc );

	sh2->pc = RL( sh2->ea );
	change_pc(sh2->pc & AM);

	sh2->sh2_icount -= 7;
}

/*  TST     Rm,Rn */
SH2_INLINE void TST(UINT32 m, UINT32 n)
{
	if ((sh2->r[n] & sh2->r[m]) == 0)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  TST     #imm,R0 */
SH2_INLINE void TSTI(UINT32 i)
{
	UINT32 imm = i & 0xff;

	if ((imm & sh2->r[0]) == 0)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
}

/*  TST.B   #imm,@(R0,GBR) */
SH2_INLINE void TSTM(UINT32 i)
{
	UINT32 imm = i & 0xff;

	sh2->ea = sh2->gbr + sh2->r[0];
	if ((imm & RB( sh2->ea )) == 0)
		sh2->sr |= T;
	else
		sh2->sr &= ~T;
	sh2->sh2_icount -= 2;
}

/*  XOR     Rm,Rn */
SH2_INLINE void XOR(UINT32 m, UINT32 n)
{
	sh2->r[n] ^= sh2->r[m];
}

/*  XOR     #imm,R0 */
SH2_INLINE void XORI(UINT32 i)
{
	UINT32 imm = i & 0xff;
	sh2->r[0] ^= imm;
}

/*  XOR.B   #imm,@(R0,GBR) */
SH2_INLINE void XORM(UINT32 i)
{
	UINT32 imm = i & 0xff;
	UINT32 temp;

	sh2->ea = sh2->gbr + sh2->r[0];
	temp = RB( sh2->ea );
	temp ^= imm;
	WB( sh2->ea, temp );
	sh2->sh2_icount -= 2;
}

/*  XTRCT   Rm,Rn */
SH2_INLINE void XTRCT(UINT32 m, UINT32 n)
{
	UINT32 temp;

	temp = (sh2->r[m] << 16) & 0xffff0000;
	sh2->r[n] = (sh2->r[n] >> 16) & 0x0000ffff;
	sh2->r[n] |= temp;
}

/*****************************************************************************
 *  OPCODE DISPATCHERS
 *****************************************************************************/

SH2_INLINE void op0000(UINT16 opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: NOP();						break;
	case 0x01: NOP();						break;
	case 0x02: STCSR(Rn);					break;
	case 0x03: BSRF(Rn);					break;
	case 0x04: MOVBS0(Rm, Rn);				break;
	case 0x05: MOVWS0(Rm, Rn);				break;
	case 0x06: MOVLS0(Rm, Rn);				break;
	case 0x07: MULL(Rm, Rn);				break;
	case 0x08: CLRT();						break;
	case 0x09: NOP();						break;
	case 0x0a: STSMACH(Rn); 				break;
	case 0x0b: RTS();						break;
	case 0x0c: MOVBL0(Rm, Rn);				break;
	case 0x0d: MOVWL0(Rm, Rn);				break;
	case 0x0e: MOVLL0(Rm, Rn);				break;
	case 0x0f: MAC_L(Rm, Rn);				break;

	case 0x10: NOP();						break;
	case 0x11: NOP();						break;
	case 0x12: STCGBR(Rn);					break;
	case 0x13: NOP();						break;
	case 0x14: MOVBS0(Rm, Rn);				break;
	case 0x15: MOVWS0(Rm, Rn);				break;
	case 0x16: MOVLS0(Rm, Rn);				break;
	case 0x17: MULL(Rm, Rn);				break;
	case 0x18: SETT();						break;
	case 0x19: DIV0U(); 					break;
	case 0x1a: STSMACL(Rn); 				break;
	case 0x1b: SLEEP(); 					break;
	case 0x1c: MOVBL0(Rm, Rn);				break;
	case 0x1d: MOVWL0(Rm, Rn);				break;
	case 0x1e: MOVLL0(Rm, Rn);				break;
	case 0x1f: MAC_L(Rm, Rn);				break;

	case 0x20: NOP();						break;
	case 0x21: NOP();						break;
	case 0x22: STCVBR(Rn);					break;
	case 0x23: BRAF(Rn);					break;
	case 0x24: MOVBS0(Rm, Rn);				break;
	case 0x25: MOVWS0(Rm, Rn);				break;
	case 0x26: MOVLS0(Rm, Rn);				break;
	case 0x27: MULL(Rm, Rn);				break;
	case 0x28: CLRMAC();					break;
	case 0x29: MOVT(Rn);					break;
	case 0x2a: STSPR(Rn);					break;
	case 0x2b: RTE();						break;
	case 0x2c: MOVBL0(Rm, Rn);				break;
	case 0x2d: MOVWL0(Rm, Rn);				break;
	case 0x2e: MOVLL0(Rm, Rn);				break;
	case 0x2f: MAC_L(Rm, Rn);				break;

	case 0x30: NOP();						break;
	case 0x31: NOP();						break;
	case 0x32: NOP();						break;
	case 0x33: NOP();						break;
	case 0x34: MOVBS0(Rm, Rn);				break;
	case 0x35: MOVWS0(Rm, Rn);				break;
	case 0x36: MOVLS0(Rm, Rn);				break;
	case 0x37: MULL(Rm, Rn);				break;
	case 0x38: NOP();						break;
	case 0x39: NOP();						break;
	case 0x3a: NOP();						break;
	case 0x3b: NOP();						break;
	case 0x3c: MOVBL0(Rm, Rn);				break;
	case 0x3d: MOVWL0(Rm, Rn);				break;
	case 0x3e: MOVLL0(Rm, Rn);				break;
	case 0x3f: MAC_L(Rm, Rn);				break;



	}
}

SH2_INLINE void op0001(UINT16 opcode)
{
	MOVLS4(Rm, opcode & 0x0f, Rn);
}

SH2_INLINE void op0010(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: MOVBS(Rm, Rn); 				break;
	case  1: MOVWS(Rm, Rn); 				break;
	case  2: MOVLS(Rm, Rn); 				break;
	case  3: NOP(); 						break;
	case  4: MOVBM(Rm, Rn); 				break;
	case  5: MOVWM(Rm, Rn); 				break;
	case  6: MOVLM(Rm, Rn); 				break;
	case  7: DIV0S(Rm, Rn); 				break;
	case  8: TST(Rm, Rn);					break;
	case  9: AND(Rm, Rn);					break;
	case 10: XOR(Rm, Rn);					break;
	case 11: OR(Rm, Rn);					break;
	case 12: CMPSTR(Rm, Rn);				break;
	case 13: XTRCT(Rm, Rn); 				break;
	case 14: MULU(Rm, Rn);					break;
	case 15: MULS(Rm, Rn);					break;
	}
}

SH2_INLINE void op0011(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: CMPEQ(Rm, Rn); 				break;
	case  1: NOP(); 						break;
	case  2: CMPHS(Rm, Rn); 				break;
	case  3: CMPGE(Rm, Rn); 				break;
	case  4: DIV1(Rm, Rn);					break;
	case  5: DMULU(Rm, Rn); 				break;
	case  6: CMPHI(Rm, Rn); 				break;
	case  7: CMPGT(Rm, Rn); 				break;
	case  8: SUB(Rm, Rn);					break;
	case  9: NOP(); 						break;
	case 10: SUBC(Rm, Rn);					break;
	case 11: SUBV(Rm, Rn);					break;
	case 12: ADD(Rm, Rn);					break;
	case 13: DMULS(Rm, Rn); 				break;
	case 14: ADDC(Rm, Rn);					break;
	case 15: ADDV(Rm, Rn);					break;
	}
}

SH2_INLINE void op0100(UINT16 opcode)
{
	switch (opcode & 0x3F)
	{
	case 0x00: SHLL(Rn);					break;
	case 0x01: SHLR(Rn);					break;
	case 0x02: STSMMACH(Rn);				break;
	case 0x03: STCMSR(Rn);					break;
	case 0x04: ROTL(Rn);					break;
	case 0x05: ROTR(Rn);					break;
	case 0x06: LDSMMACH(Rn);				break;
	case 0x07: LDCMSR(Rn);					break;
	case 0x08: SHLL2(Rn);					break;
	case 0x09: SHLR2(Rn);					break;
	case 0x0a: LDSMACH(Rn); 				break;
	case 0x0b: JSR(Rn); 					break;
	case 0x0c: NOP();						break;
	case 0x0d: NOP();						break;
	case 0x0e: LDCSR(Rn);					break;
	case 0x0f: MAC_W(Rm, Rn);				break;

	case 0x10: DT(Rn);						break;
	case 0x11: CMPPZ(Rn);					break;
	case 0x12: STSMMACL(Rn);				break;
	case 0x13: STCMGBR(Rn); 				break;
	case 0x14: NOP();						break;
	case 0x15: CMPPL(Rn);					break;
	case 0x16: LDSMMACL(Rn);				break;
	case 0x17: LDCMGBR(Rn); 				break;
	case 0x18: SHLL8(Rn);					break;
	case 0x19: SHLR8(Rn);					break;
	case 0x1a: LDSMACL(Rn); 				break;
	case 0x1b: TAS(Rn); 					break;
	case 0x1c: NOP();						break;
	case 0x1d: NOP();						break;
	case 0x1e: LDCGBR(Rn);					break;
	case 0x1f: MAC_W(Rm, Rn);				break;

	case 0x20: SHAL(Rn);					break;
	case 0x21: SHAR(Rn);					break;
	case 0x22: STSMPR(Rn);					break;
	case 0x23: STCMVBR(Rn); 				break;
	case 0x24: ROTCL(Rn);					break;
	case 0x25: ROTCR(Rn);					break;
	case 0x26: LDSMPR(Rn);					break;
	case 0x27: LDCMVBR(Rn); 				break;
	case 0x28: SHLL16(Rn);					break;
	case 0x29: SHLR16(Rn);					break;
	case 0x2a: LDSPR(Rn);					break;
	case 0x2b: JMP(Rn); 					break;
	case 0x2c: NOP();						break;
	case 0x2d: NOP();						break;
	case 0x2e: LDCVBR(Rn);					break;
	case 0x2f: MAC_W(Rm, Rn);				break;

	case 0x30: NOP();						break;
	case 0x31: NOP();						break;
	case 0x32: NOP();						break;
	case 0x33: NOP();						break;
	case 0x34: NOP();						break;
	case 0x35: NOP();						break;
	case 0x36: NOP();						break;
	case 0x37: NOP();						break;
	case 0x38: NOP();						break;
	case 0x39: NOP();						break;
	case 0x3a: NOP();						break;
	case 0x3b: NOP();						break;
	case 0x3c: NOP();						break;
	case 0x3d: NOP();						break;
	case 0x3e: NOP();						break;
	case 0x3f: MAC_W(Rm, Rn);				break;

	}
}

SH2_INLINE void op0101(UINT16 opcode)
{
	MOVLL4(Rm, opcode & 0x0f, Rn);
}

SH2_INLINE void op0110(UINT16 opcode)
{
	switch (opcode & 15)
	{
	case  0: MOVBL(Rm, Rn); 				break;
	case  1: MOVWL(Rm, Rn); 				break;
	case  2: MOVLL(Rm, Rn); 				break;
	case  3: MOV(Rm, Rn);					break;
	case  4: MOVBP(Rm, Rn); 				break;
	case  5: MOVWP(Rm, Rn); 				break;
	case  6: MOVLP(Rm, Rn); 				break;
	case  7: NOT(Rm, Rn);					break;
	case  8: SWAPB(Rm, Rn); 				break;
	case  9: SWAPW(Rm, Rn); 				break;
	case 10: NEGC(Rm, Rn);					break;
	case 11: NEG(Rm, Rn);					break;
	case 12: EXTUB(Rm, Rn); 				break;
	case 13: EXTUW(Rm, Rn); 				break;
	case 14: EXTSB(Rm, Rn); 				break;
	case 15: EXTSW(Rm, Rn); 				break;
	}
}

SH2_INLINE void op0111(UINT16 opcode)
{
	ADDI(opcode & 0xff, Rn);
}

SH2_INLINE void op1000(UINT16 opcode)
{
	switch ( opcode  & (15<<8) )
	{
	case  0 << 8: MOVBS4(opcode & 0x0f, Rm); 	break;
	case  1 << 8: MOVWS4(opcode & 0x0f, Rm); 	break;
	case  2<< 8: NOP(); 				break;
	case  3<< 8: NOP(); 				break;
	case  4<< 8: MOVBL4(Rm, opcode & 0x0f); 	break;
	case  5<< 8: MOVWL4(Rm, opcode & 0x0f); 	break;
	case  6<< 8: NOP(); 				break;
	case  7<< 8: NOP(); 				break;
	case  8<< 8: CMPIM(opcode & 0xff);		break;
	case  9<< 8: BT(opcode & 0xff); 		break;
	case 10<< 8: NOP(); 				break;
	case 11<< 8: BF(opcode & 0xff); 		break;
	case 12<< 8: NOP(); 				break;
	case 13<< 8: BTS(opcode & 0xff);		break;
	case 14<< 8: NOP(); 				break;
	case 15<< 8: BFS(opcode & 0xff);		break;
	}
}


SH2_INLINE void op1001(UINT16 opcode)
{
	MOVWI(opcode & 0xff, Rn);
}

SH2_INLINE void op1010(UINT16 opcode)
{
	BRA(opcode & 0xfff);
}

SH2_INLINE void op1011(UINT16 opcode)
{
	BSR(opcode & 0xfff);
}

SH2_INLINE void op1100(UINT16 opcode)
{
	switch (opcode & (15<<8))
	{
	case  0<<8: MOVBSG(opcode & 0xff); 		break;
	case  1<<8: MOVWSG(opcode & 0xff); 		break;
	case  2<<8: MOVLSG(opcode & 0xff); 		break;
	case  3<<8: TRAPA(opcode & 0xff);		break;
	case  4<<8: MOVBLG(opcode & 0xff); 		break;
	case  5<<8: MOVWLG(opcode & 0xff); 		break;
	case  6<<8: MOVLLG(opcode & 0xff); 		break;
	case  7<<8: MOVA(opcode & 0xff);		break;
	case  8<<8: TSTI(opcode & 0xff);		break;
	case  9<<8: ANDI(opcode & 0xff);		break;
	case 10<<8: XORI(opcode & 0xff);		break;
	case 11<<8: ORI(opcode & 0xff);			break;
	case 12<<8: TSTM(opcode & 0xff);		break;
	case 13<<8: ANDM(opcode & 0xff);		break;
	case 14<<8: XORM(opcode & 0xff);		break;
	case 15<<8: ORM(opcode & 0xff);			break;
	}
}

SH2_INLINE void op1101(UINT16 opcode)
{
	MOVLI(opcode & 0xff, Rn);
}

SH2_INLINE void op1110(UINT16 opcode)
{
	MOVI(opcode & 0xff, Rn);
}

SH2_INLINE void op1111(UINT16 /*opcode*/)
{
	NOP();
}

#endif	// USE_JUMPTABLE

/*****************************************************************************
 *  MAME CPU INTERFACE
 *****************************************************************************/

static void sh2_timer_resync(void)
{
	int divider = div_tab[(sh2->m[5] >> 8) & 3];
	UINT32 cur_time = sh2_GetTotalCycles();

	if(divider)
		sh2->frc += (cur_time - sh2->frc_base) >> divider;
	sh2->frc_base = cur_time;
}

static void sh2_timer_activate(void)
{
	int max_delta = 0xfffff;
	UINT16 frc;

	//timer_adjust(sh2->timer, attotime_never, 0, attotime_zero);
	sh2->timer_active = 0;
//	sh2->timer_cycles = 0;

	frc = sh2->frc;
	if(!(sh2->m[4] & OCFA)) {
		UINT16 delta = sh2->ocra - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(!(sh2->m[4] & OCFB) && (sh2->ocra <= sh2->ocrb || !(sh2->m[4] & 0x010000))) {
		UINT16 delta = sh2->ocrb - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(!(sh2->m[4] & OVF) && !(sh2->m[4] & 0x010000)) {
		int delta = 0x10000 - frc;
		if(delta < max_delta)
			max_delta = delta;
	}

	if(max_delta != 0xfffff) {
		int divider = div_tab[(sh2->m[5] >> 8) & 3];
		if(divider) {
			max_delta <<= divider;
			sh2->frc_base = sh2_GetTotalCycles();
			
			//timer_adjust(sh2->timer, ATTOTIME_IN_CYCLES(max_delta, sh2->cpu_number), sh2->cpu_number, attotime_zero);
			//bprintf(0, _T("SH2 Timer Actived %d\n"), max_delta);
			
			sh2->timer_active = 1;
			sh2->timer_cycles = max_delta;
			sh2->timer_base = sh2->frc_base;
			
		} else {
//			logerror("SH2.%d: Timer event in %d cycles of external clock", sh2->cpu_number, max_delta);
			//bprintf(0, _T("SH2.0: Timer event in %d cycles of external clock\n"), max_delta);
		}
	}
}

static void sh2_recalc_irq(void)
{
	int irq = 0, vector = -1;
	int  level;

	// Timer irqs
	if((sh2->m[4]>>8) & sh2->m[4] & (ICF|OCFA|OCFB|OVF))
	{
		level = (sh2->m[0x18] >> 24) & 15;
		if(level > irq)
		{
			int mask = (sh2->m[4]>>8) & sh2->m[4];
			irq = level;
			if(mask & ICF)
				vector = (sh2->m[0x19] >> 8) & 0x7f;
			else if(mask & (OCFA|OCFB))
				vector = sh2->m[0x19] & 0x7f;
			else
				vector = (sh2->m[0x1a] >> 24) & 0x7f;
		}
	}

	// DMA irqs
	if((sh2->m[0x63] & 6) == 6) {
		level = (sh2->m[0x38] >> 8) & 15;
		if(level > irq) {
			irq = level;
			vector = (sh2->m[0x68] >> 24) & 0x7f;
		}
	}

	if((sh2->m[0x67] & 6) == 6) {
		level = (sh2->m[0x38] >> 8) & 15;
		if(level > irq) {
			irq = level;
			vector = (sh2->m[0x6a] >> 24) & 0x7f;
		}
	}


	sh2->internal_irq_level = irq;
	sh2->internal_irq_vector = vector;
	sh2->test_irq = 1;
}

static void sh2_timer_callback()
{
	UINT16 frc;
//	int cpunum = param;
//	cpuintrf_push_context(cpunum);
	sh2_timer_resync();

	frc = sh2->frc;

	if(frc == sh2->ocrb)
		sh2->m[4] |= OCFB;

	if(frc == 0x0000)
		sh2->m[4] |= OVF;

	if(frc == sh2->ocra)
	{
		sh2->m[4] |= OCFA;

		if(sh2->m[4] & 0x010000)
			sh2->frc = 0;
	}

	sh2_recalc_irq();
	sh2_timer_activate();

//	cpuintrf_pop_context();
}

static void sh2_dmac_callback(int dma)
{
//	cpuintrf_push_context(cpunum);

//	LOG(("SH2.%d: DMA %d complete\n", cpunum, dma));
//	bprintf(0, _T("SH2: DMA %d complete at %d\n"), dma, sh2_GetTotalCycles());

	sh2->m[0x63+4*dma] |= 2;
	sh2->dma_timer_active[dma] = 0;
	sh2_recalc_irq();
	
//	cpuintrf_pop_context();
}

static void sh2_dmac_check(int dma)
{
	if(sh2->m[0x63+4*dma] & sh2->m[0x6c] & 1)
	{
		if(!sh2->dma_timer_active[dma] && !(sh2->m[0x63+4*dma] & 2))
		{
			int incs, incd, size;
			UINT32 src, dst, count;
			incd = (sh2->m[0x63+4*dma] >> 14) & 3;
			incs = (sh2->m[0x63+4*dma] >> 12) & 3;
			size = (sh2->m[0x63+4*dma] >> 10) & 3;
			if(incd == 3 || incs == 3)
			{
//				logerror("SH2: DMA: bad increment values (%d, %d, %d, %04x)\n", incd, incs, size, sh2->m[0x63+4*dma]);
				//bprintf(0, _T("SH2: DMA: bad increment values (%d, %d, %d, %04x)\n"), incd, incs, size, sh2->m[0x63+4*dma]);
				return;
			}
			src   = sh2->m[0x60+4*dma];
			dst   = sh2->m[0x61+4*dma];
			count = sh2->m[0x62+4*dma];
			if(!count)
				count = 0x1000000;

//			LOG(("SH2: DMA %d start %x, %x, %x, %04x, %d, %d, %d\n", dma, src, dst, count, sh2->m[0x63+4*dma], incs, incd, size));
			//bprintf(1, _T("DMA %d start %08x, %08x, %x, %04x, %d, %d, %d : %d cys from %d\n"), dma, src, dst, count, sh2->m[0x63+4*dma], incs, incd, size, 2*count+1, Sh2GetTotalCycles());

			sh2->dma_timer_active[dma] = 1;
			//timer_adjust(sh2->dma_timer[dma], ATTOTIME_IN_CYCLES(2*count+1, sh2->cpu_number), (sh2->cpu_number<<1)|dma, attotime_zero);
			sh2->dma_timer_cycles[dma] = 2 * count + 1;
			sh2->dma_timer_base[dma] = sh2_GetTotalCycles();
			
			src &= AM;
			dst &= AM;

			switch(size)
			{
			case 0:
				for(;count > 0; count --)
				{
					if(incs == 2)
						src --;
					if(incd == 2)
						dst --;
					program_write_byte_32be(dst, program_read_byte_32be(src));
					if(incs == 1)
						src ++;
					if(incd == 1)
						dst ++;
				}
				break;
			case 1:
				src &= ~1;
				dst &= ~1;
				for(;count > 0; count --)
				{
					if(incs == 2)
						src -= 2;
					if(incd == 2)
						dst -= 2;
					program_write_word_32be(dst, program_read_word_32be(src));
					if(incs == 1)
						src += 2;
					if(incd == 1)
						dst += 2;
				}
				break;
			case 2:
				src &= ~3;
				dst &= ~3;
				for(;count > 0; count --)
				{
					if(incs == 2)
						src -= 4;
					if(incd == 2)
						dst -= 4;
						
					//program_write_dword_32be(dst, program_read_dword_32be(src));
					WL(dst, RL(src));
					
					if(incs == 1)
						src += 4;
					if(incd == 1)
						dst += 4;

				}
				break;
			case 3:
				src &= ~3;
				dst &= ~3;
				count &= ~3;
				for(;count > 0; count -= 4)
				{
					if(incd == 2)
						dst -= 16;
					program_write_dword_32be(dst, program_read_dword_32be(src));
					program_write_dword_32be(dst+4, program_read_dword_32be(src+4));
					program_write_dword_32be(dst+8, program_read_dword_32be(src+8));
					program_write_dword_32be(dst+12, program_read_dword_32be(src+12));
					src += 16;
					if(incd == 1)
						dst += 16;
				}
				break;
			}
		}
	}
	else
	{
		if(sh2->dma_timer_active[dma])
		{
//			logerror("SH2: DMA %d cancelled in-flight", dma);
			//bprintf(0, _T("SH2: DMA %d cancelled in-flight"), dma);
			//timer_adjust(sh2->dma_timer[dma], attotime_never, 0, attotime_zero);
			sh2->dma_timer_active[dma] = 0;

		}
	}
}


static void sh2_internal_w(UINT32 offset, UINT32 data, UINT32 mem_mask)
{
	UINT32 old = sh2->m[offset];
	COMBINE_DATA(sh2->m+offset);

	//  if(offset != 0x20)
	//      logerror("sh2_internal_w:  Write %08x (%x), %08x @ %08x\n", 0xfffffe00+offset*4, offset, data, mem_mask);

	switch( offset )
	{
		// Timers
	case 0x04: // TIER, FTCSR, FRC
		if((mem_mask & 0x00ffffff) != 0xffffff)
			sh2_timer_resync();
		//logerror("SH2.%d: TIER write %04x @ %04x\n", sh2->cpu_number, data >> 16, mem_mask>>16);
		sh2->m[4] = (sh2->m[4] & ~(ICF|OCFA|OCFB|OVF)) | (old & sh2->m[4] & (ICF|OCFA|OCFB|OVF));
		COMBINE_DATA(&sh2->frc);
		if((mem_mask & 0x00ffffff) != 0xffffff)
			sh2_timer_activate();
		sh2_recalc_irq();
		break;
	case 0x05: // OCRx, TCR, TOCR
		//logerror("SH2.%d: TCR write %08x @ %08x\n", sh2->cpu_number, data, mem_mask);
		sh2_timer_resync();
		if(sh2->m[5] & 0x10)
			sh2->ocrb = (sh2->ocrb & (mem_mask >> 16)) | ((data & ~mem_mask) >> 16);
		else
			sh2->ocra = (sh2->ocra & (mem_mask >> 16)) | ((data & ~mem_mask) >> 16);
		sh2_timer_activate();
		break;

	case 0x06: // ICR
		break;

		// Interrupt vectors
	case 0x18: // IPRB, VCRA
	case 0x19: // VCRB, VCRC
	case 0x1a: // VCRD
		sh2_recalc_irq();
		break;

		// DMA
	case 0x1c: // DRCR0, DRCR1
		break;

		// Watchdog
	case 0x20: // WTCNT, RSTCSR
		break;

		// Standby and cache
	case 0x24: // SBYCR, CCR
		break;

		// Interrupt vectors cont.
	case 0x38: // ICR, IRPA
		break;
	case 0x39: // VCRWDT
		break;

		// Division box
	case 0x40: // DVSR
		break;
	case 0x41: // DVDNT
		{
			INT32 a = sh2->m[0x41];
			INT32 b = sh2->m[0x40];
//			LOG(("SH2 #%d div+mod %d/%d\n", cpu_getactivecpu(), a, b));
			if (b)
			{
				sh2->m[0x45] = a / b;
				sh2->m[0x44] = a % b;
			}
			else
			{
				sh2->m[0x42] |= 0x00010000;
				sh2->m[0x45] = 0x7fffffff;
				sh2->m[0x44] = 0x7fffffff;
				sh2_recalc_irq();
			}
			break;
		}
	case 0x42: // DVCR
		sh2->m[0x42] = (sh2->m[0x42] & ~0x00001000) | (old & sh2->m[0x42] & 0x00010000);
		sh2_recalc_irq();
		break;
	case 0x43: // VCRDIV
		sh2_recalc_irq();
		break;
	case 0x44: // DVDNTH
		break;
	case 0x45: // DVDNTL
		{
			INT64 a = sh2->m[0x45] | ((UINT64)(sh2->m[0x44]) << 32);
			INT64 b = (INT32)sh2->m[0x40];
//			LOG(("SH2 #%d div+mod %lld/%lld\n", cpu_getactivecpu(), a, b));
			if (b)
			{
				INT64 q = a / b;
				if (q != (INT32)q)
				{
					sh2->m[0x42] |= 0x00010000;
					sh2->m[0x45] = 0x7fffffff;
					sh2->m[0x44] = 0x7fffffff;
					sh2_recalc_irq();
				}
				else
				{
					sh2->m[0x45] = q;
					sh2->m[0x44] = a % b;
				}
			}
			else
			{
				sh2->m[0x42] |= 0x00010000;
				sh2->m[0x45] = 0x7fffffff;
				sh2->m[0x44] = 0x7fffffff;
				sh2_recalc_irq();
			}
			break;
		}

		// DMA controller
	case 0x60: // SAR0
	case 0x61: // DAR0
		break;
	case 0x62: // DTCR0
		sh2->m[0x62] &= 0xffffff;
		break;
	case 0x63: // CHCR0
		sh2->m[0x63] = (sh2->m[0x63] & ~2) | (old & sh2->m[0x63] & 2);
		sh2_dmac_check(0);
		break;
	case 0x64: // SAR1
	case 0x65: // DAR1
		break;
	case 0x66: // DTCR1
		sh2->m[0x66] &= 0xffffff;
		break;
	case 0x67: // CHCR1
		sh2->m[0x67] = (sh2->m[0x67] & ~2) | (old & sh2->m[0x67] & 2);
		sh2_dmac_check(1);
		break;
	case 0x68: // VCRDMA0
	case 0x6a: // VCRDMA1
		sh2_recalc_irq();
		break;
	case 0x6c: // DMAOR
		sh2->m[0x6c] = (sh2->m[0x6c] & ~6) | (old & sh2->m[0x6c] & 6);
		sh2_dmac_check(0);
		sh2_dmac_check(1);
		break;

		// Bus controller
	case 0x78: // BCR1
	case 0x79: // BCR2
	case 0x7a: // WCR
	case 0x7b: // MCR
	case 0x7c: // RTCSR
	case 0x7d: // RTCNT
	case 0x7e: // RTCOR
		break;

	default:
		//logerror("sh2_internal_w:  Unmapped write %08x, %08x @ %08x\n", 0xfffffe00+offset*4, data, mem_mask);
		break;
	}
}

static UINT32 sh2_internal_r(UINT32 offset, UINT32 /*mem_mask*/)
{
	//  logerror("sh2_internal_r:  Read %08x (%x) @ %08x\n", 0xfffffe00+offset*4, offset, mem_mask);
	//bprintf(0, _T("sh2_internal_r:  Read %08x (%x) @ %08x\n"), 0xfffffe00+offset*4, offset, mem_mask);
	
	switch( offset )
	{
	case 0x04: // TIER, FTCSR, FRC
		sh2_timer_resync();
		return (sh2->m[4] & 0xffff0000) | sh2->frc;
	case 0x05: // OCRx, TCR, TOCR
		if(sh2->m[5] & 0x10)
			return (sh2->ocrb << 16) | (sh2->m[5] & 0xffff);
		else
			return (sh2->ocra << 16) | (sh2->m[5] & 0xffff);
	case 0x06: // ICR
		return sh2->icr << 16;

	case 0x38: // ICR, IPRA
//		return (sh2->m[0x38] & 0x7fffffff) | (sh2->nmi_line_state == ASSERT_LINE ? 0 : 0x80000000);
		return (sh2->m[0x38] & 0x7fffffff) | 0x80000000;

	case 0x78: // BCR1
//		return sh2->is_slave ? 0x00008000 : 0;
		return 0;

	case 0x41: // dvdntl mirrors
	case 0x47:
		return sh2->m[0x45];

	case 0x46: // dvdnth mirror
		return sh2->m[0x44];
	}
	return sh2->m[offset];
}

// -------------------------------------------------------

#if USE_JUMPTABLE

int Sh2Run(int cycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2Run called without init\n"));
#endif

	sh2->sh2_icount = cycles;
	sh2->sh2_cycles_to_run = cycles;

	do
	{
		if ( pSh2Ext->suspend ) {
			sh2->sh2_total_cycles += cycles;
			sh2->sh2_icount = 0;
			break;
		}

		UINT16 opcode;

		if (sh2->delay) {
			//opcode = cpu_readop16(WORD_XOR_BE((UINT32)(sh2->delay & AM)));
			opcode = cpu_readop16(sh2->delay & AM);
			change_pc(sh2->pc & AM);
			sh2->delay = 0;
		} else {
			//opcode = cpu_readop16(WORD_XOR_BE((UINT32)(sh2->pc & AM)));
			opcode = cpu_readop16(sh2->pc & AM);
			sh2->pc += 2;
		}

		sh2->ppc = sh2->pc;

		opcode_jumptable[opcode](opcode);

		if(sh2->test_irq && !sh2->delay)
		{
			CHECK_PENDING_IRQ(/*"mame_sh2_execute"*/);
			sh2->test_irq = 0;
		}

		sh2->sh2_icount--;
		
		// timer check 
		
		{
			unsigned int cy = sh2_GetTotalCycles();

			if (sh2->dma_timer_active[0])
				if ((cy - sh2->dma_timer_base[0]) >= sh2->dma_timer_cycles[0])
					sh2_dmac_callback(0);

			if (sh2->dma_timer_active[1])
				if ((cy - sh2->dma_timer_base[1]) >= sh2->dma_timer_cycles[1])
					sh2_dmac_callback(1);
	
			if ( sh2->timer_active )
				if ((cy - sh2->timer_base) >= sh2->timer_cycles)
					sh2_timer_callback();
		}
		
		
	} while( sh2->sh2_icount > 0 );
	
	sh2->cycle_counts += cycles - (UINT32)sh2->sh2_icount;

	return cycles - sh2->sh2_icount;
}

#else

int Sh2Run(int cycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2Run called without init\n"));
#endif

	sh2->sh2_icount = cycles;
	sh2->sh2_cycles_to_run = cycles;
	
	do
	{

		if ( pSh2Ext->suspend ) {
			sh2->sh2_total_cycles += cycles;
			sh2->sh2_icount = 0;
			break;
		}			

		UINT16 opcode;

		if (sh2->delay) {
			//opcode = cpu_readop16(WORD_XOR_BE((UINT32)(sh2->delay & AM)));
			opcode = cpu_readop16(sh2->delay & AM);
			change_pc(sh2->pc & AM);
			sh2->delay = 0;
		} else {
			//opcode = cpu_readop16(WORD_XOR_BE((UINT32)(sh2->pc & AM)));
			opcode = cpu_readop16(sh2->pc & AM);
			sh2->pc += 2;
		}

		sh2->ppc = sh2->pc;

		switch (opcode & ( 15 << 12))
		{
		case  0<<12: op0000(opcode); break;
		case  1<<12: op0001(opcode); break;
		case  2<<12: op0010(opcode); break;
		case  3<<12: op0011(opcode); break;
		case  4<<12: op0100(opcode); break;
		case  5<<12: op0101(opcode); break;
		case  6<<12: op0110(opcode); break;
		case  7<<12: op0111(opcode); break;
		case  8<<12: op1000(opcode); break;
		case  9<<12: op1001(opcode); break;
		case 10<<12: op1010(opcode); break;
		case 11<<12: op1011(opcode); break;
		case 12<<12: op1100(opcode); break;
		case 13<<12: op1101(opcode); break;
		case 14<<12: op1110(opcode); break;
		default: op1111(opcode); break;
		}

#endif

		if(sh2->test_irq && !sh2->delay)
		{
			CHECK_PENDING_IRQ(/*"mame_sh2_execute"*/);
			sh2->test_irq = 0;
		}

		sh2->sh2_total_cycles++;
		sh2->sh2_icount--;
		
		// timer check 
		
		{
			unsigned int cy = sh2_GetTotalCycles();


			if (sh2->dma_timer_active[0])
				if ((cy - sh2->dma_timer_base[0]) >= sh2->dma_timer_cycles[0])
					sh2_dmac_callback(0);

			if (sh2->dma_timer_active[1])
				if ((cy - sh2->dma_timer_base[1]) >= sh2->dma_timer_cycles[1])
					sh2_dmac_callback(1);
	
			if ( sh2->timer_active )
				if ((cy - sh2->timer_base) >= sh2->timer_cycles)
					sh2_timer_callback();
		}
		
		
	} while( sh2->sh2_icount > 0 );
	
	sh2->cycle_counts += cycles - (UINT32)sh2->sh2_icount;
	
	sh2->sh2_cycles_to_run = sh2->sh2_icount;

	return cycles - sh2->sh2_icount;
}

void Sh2SetIRQLine(const int line, const int state)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2SetIRQLine called without init\n"));
#endif

	if (sh2->irq_line_state[line] == state) return;
	sh2->irq_line_state[line] = state;

	if( state == SH2_IRQSTATUS_NONE ) {
		// LOG(("SH-2 #%d cleared irq #%d\n", cpu_getactivecpu(), line));
		sh2->pending_irq &= ~(1 << line);
	} else {
		//LOG(("SH-2 #%d assert irq #%d\n", cpu_getactivecpu(), line));
		sh2->pending_irq |= 1 << line;
		if(sh2->delay)
			sh2->test_irq = 1;
		else
			CHECK_PENDING_IRQ(/*"sh2_set_irq_line"*/);

		pSh2Ext->suspend = 0;
	}

}

unsigned int Sh2GetPC(int)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2GetPC called without init\n"));
#endif

	return (sh2->delay) ? (sh2->delay & AM) : (sh2->pc & AM);
}

void Sh2SetVBR(unsigned int i)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2SetVBR called without init\n"));
#endif

	sh2->vbr = i;
}

void Sh2BurnUntilInt(int)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2BurnUntilInt called without init\n"));
#endif

	pSh2Ext->suspend = 1;
}

void Sh2StopRun()
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2StopRun called without init\n"));
#endif

	sh2->sh2_total_cycles += sh2->sh2_icount;
	sh2->sh2_icount = 0;
	sh2->sh2_cycles_to_run = 0;
}

int Sh2TotalCycles()
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2TotalCycles called without init\n"));
#endif

	return sh2->sh2_total_cycles;
}

void Sh2NewFrame()
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2NewFrame called without init\n"));
#endif

	sh2->sh2_total_cycles = 0;
}

void Sh2BurnCycles(int cycles)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2BurnCycles called without init\n"));
#endif

	sh2->sh2_icount -= cycles;
	sh2->sh2_total_cycles += cycles;
}

void __fastcall Sh2WriteByte(unsigned int a, unsigned char d)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2WriteByte called without init\n"));
#endif

	WB(a, d);
}


unsigned char __fastcall Sh2ReadByte(unsigned int a)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2ReadByte called without init\n"));
#endif

	return RB(a);
}

void Sh2Reset()
{
	Sh2Reset(RL(0), RL(4));
}

#include "state.h"

int Sh2Scan(int nAction)
{
#if defined FBA_DEBUG
	if (!DebugCPU_SH2Initted) bprintf(PRINT_ERROR, _T("Sh2Scan called without init\n"));
#endif

	if (nAction & ACB_DRIVER_DATA) {
	
		char szText[] = "SH2 #0";

		for (int i = 0; i < 1 /*nCPUCount*/; i++) {
			szText[5] = '1' + i;
			ScanVar(& ( Sh2Ext[i].sh2 ), sizeof(SH2) - 4, szText);
			
			SCAN_VAR (Sh2Ext[i].suspend);
			SCAN_VAR (Sh2Ext[i].opbase);
			
#if FAST_OP_FETCH
			//	Sh2Ext[i].opbase
			if (nAction & ACB_WRITE) {
				change_pc(sh2->pc & AM);
			}
#endif
		}

	}
	
	return 0;
}
