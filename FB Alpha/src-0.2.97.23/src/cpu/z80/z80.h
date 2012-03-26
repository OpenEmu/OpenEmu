#ifndef _Z80_H_
#define _Z80_H_

#define	CPUINFO_PTR_CPU_SPECIFIC	0x18000
#define Z80_CLEAR_LINE		0
#define Z80_ASSERT_LINE		1
#define Z80_INPUT_LINE_NMI	32

typedef union
{
#ifdef LSB_FIRST
	struct { UINT8 l,h,h2,h3; } b;
	struct { UINT16 l,h; } w;
#else
	struct { UINT8 h3,h2,h,l; } b;
	struct { UINT16 h,l; } w;
#endif
	UINT32 d;
} Z80_PAIR;

typedef struct
{
	Z80_PAIR	prvpc,pc,sp,af,bc,de,hl,ix,iy;
	Z80_PAIR	af2,bc2,de2,hl2;
	UINT8	r,r2,iff1,iff2,halt,im,i;
	UINT8	nmi_state;			/* nmi line state */
	UINT8	nmi_pending;		/* nmi pending */
	UINT8	irq_state;			/* irq line state */
	UINT8	after_ei;			/* are we in the EI shadow? */
	INT32 cycles_left;
	const struct z80_irq_daisy_chain *daisy;
	int		(*irq_callback)(int irqline);
} Z80_Regs;

enum {
	Z80_PC=1, Z80_SP,
	Z80_A, Z80_B, Z80_C, Z80_D, Z80_E, Z80_H, Z80_L,
	Z80_AF, Z80_BC, Z80_DE, Z80_HL,
	Z80_IX, Z80_IY,	Z80_AF2, Z80_BC2, Z80_DE2, Z80_HL2,
	Z80_R, Z80_I, Z80_IM, Z80_IFF1, Z80_IFF2, Z80_HALT,
	Z80_DC0, Z80_DC1, Z80_DC2, Z80_DC3
};

enum {
	Z80_TABLE_op,
	Z80_TABLE_cb,
	Z80_TABLE_ed,
	Z80_TABLE_xy,
	Z80_TABLE_xycb,
	Z80_TABLE_ex	/* cycles counts for taken jr/jp/call and interrupt latency (rst opcodes) */
};

enum
{
	CPUINFO_PTR_Z80_CYCLE_TABLE = CPUINFO_PTR_CPU_SPECIFIC,
	CPUINFO_PTR_Z80_CYCLE_TABLE_LAST = CPUINFO_PTR_Z80_CYCLE_TABLE + Z80_TABLE_ex
};

void Z80Init();
void Z80Reset();
void Z80Exit();
int  Z80Execute(int cycles);
void Z80Burn(int cycles);
void Z80SetIrqLine(int irqline, int state);
void Z80GetContext (void *dst);
void Z80SetContext (void *src);
int Z80Scan(int nAction);
INT32 z80TotalCycles();

extern unsigned char Z80Vector;

extern int z80_ICount;
extern UINT32 EA;

typedef unsigned char (__fastcall *Z80ReadIoHandler)(unsigned int a);
typedef void (__fastcall *Z80WriteIoHandler)(unsigned int a, unsigned char v);
typedef unsigned char (__fastcall *Z80ReadProgHandler)(unsigned int a);
typedef void (__fastcall *Z80WriteProgHandler)(unsigned int a, unsigned char v);
typedef unsigned char (__fastcall *Z80ReadOpHandler)(unsigned int a);
typedef unsigned char (__fastcall *Z80ReadOpArgHandler)(unsigned int a);

void Z80SetIOReadHandler(Z80ReadIoHandler handler);
void Z80SetIOWriteHandler(Z80WriteIoHandler handler);
void Z80SetProgramReadHandler(Z80ReadProgHandler handler);
void Z80SetProgramWriteHandler(Z80WriteProgHandler handler);
void Z80SetCPUOpReadHandler(Z80ReadOpHandler handler);
void Z80SetCPUOpArgReadHandler(Z80ReadOpArgHandler handler);

int ActiveZ80GetPC();
int ActiveZ80GetBC();
int ActiveZ80GetDE();
int ActiveZ80GetHL();

#endif

