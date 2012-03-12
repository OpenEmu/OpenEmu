#include "m6805.h"

#define M6805_READ		1
#define M6805_WRITE		2
#define M6805_FETCH		4

#define M6805_ROM		(M6805_READ | M6805_FETCH)
#define M6805_RAM		(M6805_ROM | M6805_WRITE)

void m6805MapMemory(UINT8 *ptr, INT32 nStart, INT32 nEnd, INT32 nType);

void m6805SetWriteHandler(void (*write)(UINT16, UINT8));
void m6805SetReadHandler(UINT8 (*read)(UINT16));

void m6805_write(UINT16 address, UINT8 data);
UINT8 m6805_read(UINT16 address);
UINT8 m6805_fetch(UINT16 address);
void m6805_write_rom(UINT32 address, UINT8 data);

void m6805Init(INT32 num, INT32 address_range);
void m6805Exit();
void m6805Open(INT32 );
void m6805Close();

INT32 m6805Scan(INT32 nAction, INT32 *);
#define m68705Scan(x, y) m6805Scan(x, y)

void m6805Reset();
void m6805SetIrqLine(INT32 , INT32 state);
INT32 m6805Run(INT32 cycles);

void m6805RunEnd();

void m68705Reset();
void m68705SetIrqLine(INT32 irqline, INT32 state);

void hd63705Reset(void);
void hd63705SetIrqLine(INT32 irqline, INT32 state);

void m6805NewFrame();
INT32 m6805TotalCycles();

