
// use these in arm7.cpp
void Arm7_program_write_byte_32le(UINT32 addr, UINT8 data);
void Arm7_program_write_word_32le(UINT32 addr, UINT16 data);
void Arm7_program_write_dword_32le(UINT32 addr, UINT32 data);
UINT8  Arm7_program_read_byte_32le(UINT32 addr);
UINT16 Arm7_program_read_word_32le(UINT32 addr);
UINT32   Arm7_program_read_dword_32le(UINT32 addr);
UINT16 Arm7_program_opcode_word_32le(UINT32 addr);
UINT32   Arm7_program_opcode_dword_32le(UINT32 addr);

void arm7_set_irq_line(INT32 irqline, INT32 state);

// use these everywhere else
#define ARM7_READ		1
#define ARM7_WRITE		2
#define ARM7_FETCH		4

#define ARM7_ROM		(ARM7_READ | ARM7_FETCH)
#define ARM7_RAM		(ARM7_READ | ARM7_FETCH | ARM7_WRITE)

void Arm7MapMemory(UINT8 *src, UINT32 start, UINT32 finish, INT32 type);

void Arm7SetWriteByteHandler(void (*write)(UINT32, UINT8));
void Arm7SetWriteWordHandler(void (*write)(UINT32, UINT16));
void Arm7SetWriteLongHandler(void (*write)(UINT32, UINT32));
void Arm7SetReadByteHandler(UINT8 (*read)(UINT32));
void Arm7SetReadWordHandler(UINT16 (*read)(UINT32));
void Arm7SetReadLongHandler(UINT32 (*read)(UINT32));

INT32 Arm7Scan(INT32 nAction);

void Arm7RunEnd();
void Arm7BurnCycles(INT32 cycles);
INT32 Arm7TotalCycles();
void Arm7NewFrame();

void Arm7Init(INT32);
void Arm7Open(INT32 );
void Arm7Reset();
INT32 Arm7Run(INT32 cycles);

// defined in arm7core.h
#define ARM7_IRQ_LINE		0
#define ARM7_FIRQ_LINE		1

#define ARM7_CLEAR_LINE		0
#define ARM7_ASSERT_LINE	1
#define ARM7_HOLD_LINE		2

void Arm7SetIRQLine(INT32 line, INT32 state);

void Arm7Exit();
void Arm7Close();

// speed hack stuff
void Arm7SetIdleLoopAddress(UINT32 address);

// for cheat handling
void Arm7_write_rom_byte(UINT32 addr, UINT8 data);
