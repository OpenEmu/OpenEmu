#define MAX_S2650	4
extern INT32 nActiveS2650;
extern INT32 s2650Count;

void s2650_write(UINT16 address, UINT8 data);
UINT8 s2650_read(UINT16 address);
UINT8 s2650_fetch(UINT16 address);
void s2650_write_port(UINT16 port, UINT8 data);
UINT8 s2650_read_port(UINT16 port);

void s2650_write_rom(UINT32 address, UINT8 data);

typedef INT32 (*s2650irqcallback)(INT32);
extern s2650irqcallback s2650_irqcallback[MAX_S2650];
void s2650SetIrqCallback(INT32 (*irqcallback)(INT32));

void s2650_open(INT32 num);
void s2650_close();
void s2650_init(INT32 num);
void s2650_exit();
void s2650_reset(void);
void s2650_set_irq_line(INT32 irqline, INT32 state);
INT32 s2650_get_pc();

#define S2650_READ	1
#define S2650_WRITE	2
#define S2650_FETCH	4
#define S2650_ROM	(S2650_READ | S2650_FETCH)
#define S2650_RAM	(S2650_ROM | S2650_WRITE)

void s2650MapMemory(UINT8 *src, INT32 start, INT32 end, INT32 type);

void s2650SetWriteHandler(void (*write)(UINT16, UINT8));
void s2650SetReadHandler(UINT8 (*read)(UINT16));
void s2650SetOutHandler(void (*write)(UINT16, UINT8));
void s2650SetInHandler(UINT8 (*read)(UINT16));

INT32 s2650Run(INT32 cycles);
#define s2650Reset()	 s2650_reset()
void s2650Open(INT32 num);
void s2650Close();
void s2650Exit();
void s2650Init(INT32 num);
INT32 s2650GetPc();

INT32 s2650GetActive();

INT32 s2650Scan(INT32 nAction,INT32 *);

/* fake control port   M/~IO=0 D/~C=0 E/~NE=0 */
#define S2650_CTRL_PORT 0x100

/* fake data port      M/~IO=0 D/~C=1 E/~NE=0 */
#define S2650_DATA_PORT 0x101

/* extended i/o ports  M/~IO=0 D/~C=x E/~NE=1 */
#define S2650_EXT_PORT	0xff

/* Fake Sense Line */
#define S2650_SENSE_PORT 0x102
