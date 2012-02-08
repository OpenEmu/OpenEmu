#ifndef GBACPU_H
#define GBACPU_H

#define UPDATE_REG(address, value)	WRITE16LE(((u16 *)&ioMem[address]),value);
#define ARM_PREFETCH_NEXT		cpuPrefetch[1] = CPUReadMemoryQuick(bus.armNextPC+4);
#define THUMB_PREFETCH_NEXT		cpuPrefetch[1] = CPUReadHalfWordQuick(bus.armNextPC+2);

#define ARM_PREFETCH \
  {\
    cpuPrefetch[0] = CPUReadMemoryQuick(bus.armNextPC);\
    cpuPrefetch[1] = CPUReadMemoryQuick(bus.armNextPC+4);\
  }

#define THUMB_PREFETCH \
  {\
    cpuPrefetch[0] = CPUReadHalfWordQuick(bus.armNextPC);\
    cpuPrefetch[1] = CPUReadHalfWordQuick(bus.armNextPC+2);\
  }
 
#ifdef USE_SWITICKS
extern int SWITicks;
#endif
extern u32 mastercode;
extern int cpuNextEvent;
extern bool holdState;
extern u32 cpuPrefetch[2];
extern int cpuTotalTicks;
extern u8 memoryWait[16];
extern u8 memoryWait32[16];
extern u8 memoryWaitSeq[16];
extern u8 memoryWaitSeq32[16];
extern u8 cpuBitsSet[256];
extern u8 cpuLowestBitSet[256];
extern void CPUSwitchMode(int mode, bool saveState, bool breakLoop);
extern void CPUSwitchMode(int mode, bool saveState);
extern void CPUUpdateFlags(bool breakLoop);
extern void CPUUpdateFlags();
extern void CPUSoftwareInterrupt();
extern void CPUSoftwareInterrupt(int comment);


// Waitstates when accessing data
INLINE int dataTicksAccess(u32 address, u8 bit32) // DATA 8/16bits NON SEQ
{
	int addr, value, waitState;

	addr = (address>>24)&15;

	if(bit32)	/* DATA 32bits NON SEQ */
		value = memoryWait32[addr];
	else		/* DATA 8/16bits NON SEQ */
		value =  memoryWait[addr];

	if ((addr>=0x08) || (addr < 0x02))
	{
		bus.busPrefetchCount=0;
		bus.busPrefetch=false;
	}
	else if (bus.busPrefetch)
	{
		waitState = value;
		waitState = (1 & ~waitState) | (waitState & waitState);
		bus.busPrefetchCount = ((bus.busPrefetchCount+1)<<waitState) - 1; 
	}

	return value;
}

INLINE int dataTicksAccessSeq(u32 address, u8 bit32)// DATA 8/16bits SEQ
{
	int addr, value, waitState;

	addr = (address>>24)&15;

	if (bit32)		/* DATA 32bits SEQ */
		value = memoryWaitSeq32[addr];
	else			/* DATA 8/16bits SEQ */
		value = memoryWaitSeq[addr];

	if ((addr>=0x08) || (addr < 0x02))
	{
		bus.busPrefetchCount=0;
		bus.busPrefetch=false;
	}
	else if (bus.busPrefetch)
	{
		waitState = value;
		waitState = (1 & ~waitState) | (waitState & waitState);
		bus.busPrefetchCount = ((bus.busPrefetchCount+1)<<waitState) - 1;
	}

	return value;
}

// Waitstates when executing opcode
static INLINE int codeTicksAccess(u32 address, u8 bit32) // THUMB NON SEQ
{
	int addr, ret;

	addr = (address>>24) & 15;

	if (unsigned(addr - 0x08) <= (0x0D - 0x08))
	{
		if (bus.busPrefetchCount&0x1)
		{
			if (bus.busPrefetchCount&0x2)
			{
				bus.busPrefetchCount = ((bus.busPrefetchCount&0xFF)>>2) | (bus.busPrefetchCount&0xFFFFFF00);
				return 0;
			}
			bus.busPrefetchCount = ((bus.busPrefetchCount&0xFF)>>1) | (bus.busPrefetchCount&0xFFFFFF00);
			return memoryWaitSeq[addr]-1;
		}
	}
	bus.busPrefetchCount = 0;

	if(bit32)		/* ARM NON SEQ */
		ret = memoryWait32[addr];
	else			/* THUMB NON SEQ */
		ret = memoryWait[addr];

	return ret;
}

static INLINE int codeTicksAccessSeq16(u32 address) // THUMB SEQ
{
	int addr = (address>>24) & 15;

	if (unsigned(addr - 0x08) <= (0x0D - 0x08))
	{
		if (bus.busPrefetchCount&0x1)
		{
			bus.busPrefetchCount = ((bus.busPrefetchCount&0xFF)>>1) | (bus.busPrefetchCount&0xFFFFFF00);
			return 0;
		}
		else if (bus.busPrefetchCount>0xFF)
		{
			bus.busPrefetchCount=0;
			return memoryWait[addr];
		}
	}
	else
		bus.busPrefetchCount = 0;

	return memoryWaitSeq[addr];
}

static INLINE int codeTicksAccessSeq32(u32 address) // ARM SEQ
{
	int addr = (address>>24)&15;

	if (unsigned(addr - 0x08) <= (0x0D - 0x08))
	{
		if (bus.busPrefetchCount&0x1)
		{
			if (bus.busPrefetchCount&0x2)
			{
				bus.busPrefetchCount = ((bus.busPrefetchCount&0xFF)>>2) | (bus.busPrefetchCount&0xFFFFFF00);
				return 0;
			}
			bus.busPrefetchCount = ((bus.busPrefetchCount&0xFF)>>1) | (bus.busPrefetchCount&0xFFFFFF00);
			return memoryWaitSeq[addr];
		}
		else if (bus.busPrefetchCount > 0xFF)
		{
			bus.busPrefetchCount=0;
			return memoryWait32[addr];
		}
	}
	return memoryWaitSeq32[addr];
}


// Emulates the Cheat System (m) code
#ifdef HAVE_CHEATS
static INLINE void cpuMasterCodeCheck()
{
  if((mastercode) && (mastercode == bus.armNextPC))
  {
    u32 ext = (joy >> 10);
    cpuTotalTicks += cheatsCheckKeys(P1^0x3FF, ext);
  }
}
#endif

#endif // GBACPU_H
