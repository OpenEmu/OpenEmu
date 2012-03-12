// konami.cpp
void konami_set_irq_line(INT32 irqline, INT32 state);
void konami_init(INT32 (*irqcallback)(INT32));

// konami_intf.cpp
#define KON_READ		1
#define KON_WRITE		2
#define KON_FETCH		4

#define KON_ROM			(KON_READ | KON_FETCH)
#define KON_RAM			(KON_READ | KON_FETCH | KON_WRITE)

void konamiMapMemory(UINT8 *src, UINT16 start, UINT16 finish, INT32 type);

void konamiSetIrqCallbackHandler(INT32 (*callback)(INT32));
void konamiSetlinesCallback(void  (*setlines_callback)(INT32 lines));

void konamiSetWriteHandler(void (*write)(UINT16, UINT8));
void konamiSetReadHandler(UINT8 (*read)(UINT16));

void konami_write(UINT16 address, UINT8 data);
UINT8 konami_read(UINT16 address);
UINT8 konami_fetch(UINT16 address);

void konamiInit(INT32 );
void konamiOpen(INT32 );
void konamiReset();
INT32 konamiRun(INT32 cycles);
void konamiClose();
void konamiExit();

extern INT32 nKonamiCpuCount;

#define KONAMI_IRQ_LINE		0
#define KONAMI_FIRQ_LINE	1

#define KONAMI_CLEAR_LINE	0
#define KONAMI_HOLD_LINE	1
#define KONAMI_INPUT_LINE_NMI	2

void konamiSetIrqLine(INT32 line, INT32 state);

INT32 konamiCpuScan(INT32 nAction, INT32 *);

INT32 konamiTotalCycles();
void konamiNewFrame();

INT32 konamiGetActive();

void konami_write_rom(UINT16 address, UINT8 data);
