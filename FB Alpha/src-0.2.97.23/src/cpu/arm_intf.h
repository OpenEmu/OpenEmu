// use these in arm.cpp
void Arm_program_write_byte_32le(UINT32 addr, UINT8 data);
void Arm_program_write_dword_32le(UINT32 addr, UINT32 data);
UINT8  Arm_program_read_byte_32le(UINT32 addr);
UINT32  Arm_program_read_dword_32le(UINT32 addr);
UINT32  Arm_program_opcode_dword_32le(UINT32 addr);

void arm_set_irq_line(INT32 irqline, INT32 state);

// use these everywhere else
#define ARM_READ		1
#define ARM_WRITE		2
#define ARM_FETCH		4

#define ARM_ROM		(ARM_READ | ARM_FETCH)
#define ARM_RAM		(ARM_READ | ARM_FETCH | ARM_WRITE)

void ArmMapMemory(UINT8 *src, INT32 start, INT32 finish, INT32 type);

void ArmSetWriteByteHandler(void (*write)(UINT32, UINT8));
void ArmSetWriteLongHandler(void (*write)(UINT32, UINT32));
void ArmSetReadByteHandler(UINT8 (*read)(UINT32));
void ArmSetReadLongHandler(UINT32 (*read)(UINT32));

void ArmInit(INT32);
void ArmOpen(INT32);
void ArmReset();
INT32 ArmRun(INT32 cycles);
INT32 ArmScan(INT32 nAction, INT32 *);

#define ARM_IRQ_LINE		0
#define ARM_FIRQ_LINE		1

#define ARM_CLEAR_LINE		0
#define ARM_ASSERT_LINE		1
#define ARM_HOLD_LINE		2

void ArmSetIRQLine(INT32 line, INT32 state);

void ArmExit();
void ArmClose();

extern UINT32 ArmSpeedHackAddress;
void ArmIdleCycles(INT32 cycles);
void ArmSetSpeedHack(UINT32 address, void (*pCallback)());

UINT32 ArmGetPc();

UINT32 ArmRemainingCycles();
INT32 ArmGetTotalCycles();
void ArmRunEnd();
void ArmNewFrame();

// for cheat handling
void Arm_write_rom_byte(UINT32 addr, UINT8 data);



//-----------------------------------------------------------------------------------------------------------------------------------------------
// Macros for reading/writing to 16-bit RAM regions

#define Write16Long(ram, a, b)							\
	if (address >= a && address <= b) {					\
		*((UINT16*)(ram + (((address - a) & ~3)/2))) = BURN_ENDIAN_SWAP_INT16(data);	\
		return;								\
	}

#define Write16Byte(ram, a, b)							\
	if (address >= a && address <= b) {					\
		if (~address & 2) {						\
			INT32 offset = address - a;				\
			ram[(offset & 1) | ((offset & ~3) / 2)] = data;		\
		}								\
		return;								\
	}

#define Read16Long(ram, a, b)							\
	if (address >= a && address <= b) {					\
		return BURN_ENDIAN_SWAP_INT16(*((UINT16*)(ram + (((address - a) & ~3)/2)))) | ~0xffff;	\
	}

#define Read16Byte(ram, a, b)							\
	if (address >= a && address <= b) {					\
		if (~address & 2) {						\
			INT32 offset = address - a;				\
			return ram[(offset & 1) | ((offset & ~3) / 2)];		\
		} else {							\
			return 0xff;						\
		}								\
	}
