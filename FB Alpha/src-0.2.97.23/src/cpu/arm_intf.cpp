#include "burnint.h"
#include "arm_intf.h"

//#define DEBUG_LOG

#define MAX_MEMORY	0x04000000 // max
#define MAX_MASK	0x03ffffff // max-1
#define PAGE_SIZE	0x00001000 // 400 would be better...
#define PAGE_COUNT	(MAX_MEMORY/PAGE_SIZE)
#define PAGE_SHIFT	12	// 0x1000 -> 12 bits
#define PAGE_BYTE_AND	0x00fff	// 0x1000 - 1 (byte align)
#define PAGE_LONG_AND	0x00ffc // 0x1000 - 4 (ignore last 4 bytes, long align)

#define READ	0
#define WRITE	1
#define FETCH	2

static UINT8 **membase[3]; // 0 read, 1, write, 2 opcode

static void (*pWriteLongHandler)(UINT32, UINT32  ) = NULL;
static void (*pWriteByteHandler)(UINT32, UINT8 ) = NULL;

static UINT32   (*pReadLongHandler)(UINT32) = NULL;
static UINT8  (*pReadByteHandler)(UINT32) = NULL;

UINT32 ArmSpeedHackAddress;
static void (*pArmSpeedHackCallback)();

void ArmSetSpeedHack(UINT32 address, void (*pCallback)())
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmSetSpeedHack called without init\n"));
#endif

	ArmSpeedHackAddress = address;

	pArmSpeedHackCallback = pCallback;
}

void ArmOpen(INT32)
{

}

void ArmClose()
{

}

INT32 ArmGetActive()
{
	return 0; // only one cpu supported
}

static cpu_core_config ArmCheatCpuConfig =
{
	ArmOpen,
	ArmClose,
	Arm_program_read_byte_32le,
	Arm_write_rom_byte,
	ArmGetActive,
	ArmGetTotalCycles,
	ArmNewFrame,
	ArmRun,
	ArmRunEnd,
	ArmReset,
	MAX_MEMORY,
	0
};

void ArmInit(INT32 /*num*/) // only one cpu supported
{
	DebugCPU_ARMInitted = 1;
	
	for (INT32 i = 0; i < 3; i++) {
		membase[i] = (UINT8**)malloc(PAGE_COUNT * sizeof(UINT8**));
		memset (membase[i], 0, PAGE_COUNT * sizeof(UINT8**));
	}

	pWriteLongHandler = NULL;
	pWriteByteHandler = NULL;
	pReadLongHandler = NULL;
	pReadByteHandler = NULL;

	CpuCheatRegister(0, &ArmCheatCpuConfig);

	pArmSpeedHackCallback = NULL;
	ArmSpeedHackAddress = ~0;
}

void ArmExit()
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmExit called without init\n"));
#endif

	for (INT32 i = 0; i < 3; i++) {
		if (membase[i]) {
			free (membase[i]);
			membase[i] = NULL;
		}
	}
	
	DebugCPU_ARMInitted = 0;
}

void ArmMapMemory(UINT8 *src, INT32 start, INT32 finish, INT32 type)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmMapMemory called without init\n"));
#endif

	UINT32 len = (finish-start) >> PAGE_SHIFT;

	for (UINT32 i = 0; i < len+1; i++)
	{
		UINT32 offset = i + (start >> PAGE_SHIFT);
		if (type & (1 <<  READ)) membase[ READ][offset] = src + (i << PAGE_SHIFT);
		if (type & (1 << WRITE)) membase[WRITE][offset] = src + (i << PAGE_SHIFT);
		if (type & (1 << FETCH)) membase[FETCH][offset] = src + (i << PAGE_SHIFT);
	}
}

void ArmSetWriteByteHandler(void (*write)(UINT32, UINT8))
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmSetWriteByteHandler called without init\n"));
#endif

	pWriteByteHandler = write;
}

void ArmSetWriteLongHandler(void (*write)(UINT32, UINT32))
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmSetWriteLongHandler called without init\n"));
#endif

	pWriteLongHandler = write;
}

void ArmSetReadByteHandler(UINT8 (*read)(UINT32))
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmSetReadByteHandler called without init\n"));
#endif

	pReadByteHandler = read;
}

void ArmSetReadLongHandler(UINT32 (*read)(UINT32))
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmSetReadLongHandler called without init\n"));
#endif

	pReadLongHandler = read;
}

void Arm_program_write_byte_32le(UINT32 addr, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("Arm_program_write_byte_32le called without init\n"));
#endif

	addr &= MAX_MASK;

#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, %2.2x wb\n"), addr, data);
#endif

	if (membase[WRITE][addr >> PAGE_SHIFT] != NULL) {
		membase[WRITE][addr >> PAGE_SHIFT][addr & PAGE_BYTE_AND] = data;
		return;
	}

	if (pWriteByteHandler) {
		pWriteByteHandler(addr, data);
	}
}

void Arm_program_write_dword_32le(UINT32 addr, UINT32 data)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("Arm_program_write_dword_32le called without init\n"));
#endif

	addr &= MAX_MASK;

#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, %8.8x wd\n"), addr, data);
#endif

	if (membase[WRITE][addr >> PAGE_SHIFT] != NULL) {
		*((UINT32*)(membase[WRITE][addr >> PAGE_SHIFT] + (addr & PAGE_LONG_AND))) = data;
		return;
	}

	if (pWriteLongHandler) {
		pWriteLongHandler(addr, data);
	}
}


UINT8 Arm_program_read_byte_32le(UINT32 addr)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("Arm_program_read_byte_32le called without init\n"));
#endif

	addr &= MAX_MASK;

#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, rb\n"), addr);
#endif

	if (membase[ READ][addr >> PAGE_SHIFT] != NULL) {
		return membase[READ][addr >> PAGE_SHIFT][addr & PAGE_BYTE_AND];
	}

	if (pReadByteHandler) {
		return pReadByteHandler(addr);
	}

	return 0;
}

UINT32 Arm_program_read_dword_32le(UINT32 addr)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("Arm_program_read_dword_32le called without init\n"));
#endif

	addr &= MAX_MASK;

#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, rl\n"), addr);
#endif

	if (membase[ READ][addr >> PAGE_SHIFT] != NULL) {
		return *((UINT32*)(membase[ READ][addr >> PAGE_SHIFT] + (addr & PAGE_LONG_AND)));
	}

	if (pReadLongHandler) {
		return pReadLongHandler(addr);
	}

	return 0;
}

UINT32 Arm_program_opcode_dword_32le(UINT32 addr)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("Arm_program_opcode_dword_32le called without init\n"));
#endif

	addr &= MAX_MASK;

#ifdef DEBUG_LOG
	bprintf (PRINT_NORMAL, _T("%5.5x, rlo\n"), addr);
#endif

#if 1
	if (ArmSpeedHackAddress == addr) {
//		bprintf (0, _T("speed hack activated! %d cycles killed!\n"), ArmRemainingCycles());
		if (pArmSpeedHackCallback) {
			pArmSpeedHackCallback();
		} else {
			ArmRunEnd();
		}
	}
#else
	if (ArmSpeedHackAddress) {
		bprintf (0, _T("%x, %d\n"), ArmGetPc(), ArmRemainingCycles());
		ArmRunEnd();	
	}
#endif

	if (membase[FETCH][addr >> PAGE_SHIFT] != NULL) {
		return *((UINT32*)(membase[FETCH][addr >> PAGE_SHIFT] + (addr & PAGE_LONG_AND)));
	}

	// good enough for now...
	if (pReadLongHandler) {
		return pReadLongHandler(addr);
	}

	return 0;
}

void ArmSetIRQLine(INT32 line, INT32 state)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("ArmSetIRQLine called without init\n"));
#endif

	if (state == ARM_CLEAR_LINE || state == ARM_ASSERT_LINE) {
		arm_set_irq_line(line, state);
	}
	else if (ARM_HOLD_LINE) {
		arm_set_irq_line(line, ARM_ASSERT_LINE);
		ArmRun(0);
		arm_set_irq_line(line, ARM_CLEAR_LINE);
	}
}

// For cheats/etc

void Arm_write_rom_byte(UINT32 addr, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugCPU_ARMInitted) bprintf(PRINT_ERROR, _T("Arm_write_rom_byte called without init\n"));
#endif

	addr &= MAX_MASK;

	// write to rom & ram
	if (membase[WRITE][addr >> PAGE_SHIFT] != NULL) {
		membase[WRITE][addr >> PAGE_SHIFT][addr & PAGE_BYTE_AND] = data;
	}

	if (membase[READ][addr >> PAGE_SHIFT] != NULL) {
		membase[READ][addr >> PAGE_SHIFT][addr & PAGE_BYTE_AND] = data;
	}

	if (pWriteByteHandler) {
		pWriteByteHandler(addr, data);
	}
}
