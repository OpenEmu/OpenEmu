#ifndef __MDFN_HUC6280_H
#define __MDFN_HUC6280_H

#include        <trio/trio.h>

class HuC6280_Support
{
	public:

	INLINE HuC6280_Support(void)
	{

	}

	INLINE ~HuC6280_Support()
	{

	}
	virtual int32 Sync(const int32 timestamp) = 0;
};

class HuC6280
{
	public:

	typedef void (*writefunc)(uint32 A, uint8 V);
	typedef uint8 (*readfunc)(uint32 A);


	enum { N_FLAG = 0x80 };
	enum { V_FLAG = 0x40 };
	enum { T_FLAG = 0x20 };
	enum { B_FLAG = 0x10 };
	enum { D_FLAG = 0x08 };
	enum { I_FLAG = 0x04 };
	enum { Z_FLAG = 0x02 };
	enum { C_FLAG = 0x01 };

	// If emulate_wai is true, then the "0xCB" opcode will be handled by waiting for the next high-level event, NOT 
	// for the IRQ line to be asserted as on a 65816.
	// It's mainly a hack intended for less CPU-intensive HES playback.
	HuC6280(const bool emulate_wai = false);
	~HuC6280();

	void Reset(void);
	void Power(void);

	enum { IQIRQ1 = 0x002 };
	enum { IQIRQ2 = 0x001 };
	enum { IQTIMER = 0x004 };
	enum { IQRESET = 0x020 };

	INLINE void IRQBegin(int w)
	{
	 IRQlow |= w;
	}

	INLINE void IRQEnd(int w)
	{
	 IRQlow &= ~w;
	}


	void TimerSync(void);

	INLINE uint8 GetIODataBuffer(void)
	{
	 return(IODataBuffer);
	}

	INLINE void SetIODataBuffer(uint8 v)
	{
	 IODataBuffer = v;
	}

	uint8 TimerRead(unsigned int address, bool peek = FALSE);
	void TimerWrite(unsigned int address, uint8 V);

	uint8 IRQStatusRead(unsigned int address, bool peek = FALSE);
	void IRQStatusWrite(unsigned int address, uint8 V);

	int StateAction(StateMem *sm, int load, int data_only);

	void Run(bool StepMode = FALSE);

	INLINE void Exit(void)
	{
	 runrunrun = 0;
	}

	INLINE void SyncAndResetTimestamp(void)
	{
         TimerSync();

	 timer_lastts = 0;
	 timestamp = 0;
	}

	INLINE bool InBlockMove(void)
	{
	 return(in_block_move);
	}

	void StealCycle(void);
	void StealCycles(const int count);
        void StealMasterCycles(const int count);

	void SetEvent(const int32 cycles) NO_INLINE
	{
	 next_user_event = cycles;
	 CalcNextEvent();
	}

	INLINE void SetEventHandler(HuC6280_Support *new_EventHandler)
	{
	 EventHandler = new_EventHandler;
	}

	INLINE uint32 Timestamp(void)
	{
	 return(timestamp);
	}

	//
	// Debugger support methods:
	//
	INLINE void SetCPUHook(void (*new_CPUHook)(uint32), void (*new_ADDBT)(uint32))
	{
	 CPUHook = new_CPUHook;
	 ADDBT = new_ADDBT;
	}

	INLINE void LoadShadow(const HuC6280 &state)
	{
	 //EmulateWAI = state.EmulateWAI;

         PC = state.PC;
         A = state.A;
         X = state.X;
         Y = state.Y;
         S = state.S;
         P = state.P;

         PIMaskCache = state.PIMaskCache;

         MPR[0] = state.MPR[0];
         MPR[1] = state.MPR[1];
         MPR[2] = state.MPR[2];
         MPR[3] = state.MPR[3];
         MPR[4] = state.MPR[4];
         MPR[5] = state.MPR[5];
         MPR[6] = state.MPR[6];
         MPR[7] = state.MPR[7];

         for(int x = 0; x < 9; x++)
          SetMPR(x, MPR[x & 0x7]);

         IRQlow = state.IRQlow;
         IRQSample = state.IRQSample;
         IFlagSample = state.IFlagSample;

         speed = state.speed;
         speed_shift_cache = state.speed_shift_cache;
         timestamp = state.timestamp;

         IRQMask = state.IRQMask;

	//
	//

	 timer_status = 0;

         next_user_event = 0x1FFFFFFF;
	 next_event = 0x1FFFFFFF;

	 //IRQlow = 0;
	 //IRQSample = 0;
	 //IFlagSample = HuC6280::I_FLAG;
	}

	enum
	{
	 GSREG_PC = 0,
	 GSREG_A,
	 GSREG_X,
	 GSREG_Y,
	 GSREG_SP,
	 GSREG_P,
	 GSREG_MPR0,
	 GSREG_MPR1,
	 GSREG_MPR2,
	 GSREG_MPR3,
	 GSREG_MPR4,
	 GSREG_MPR5,
	 GSREG_MPR6,
	 GSREG_MPR7,
	 GSREG_SPD,
	 GSREG_IRQM,
	 GSREG_TIMS,
	 GSREG_TIMV,
	 GSREG_TIML,
	 GSREG_TIMD,
	 GSREG_STAMP
	};

	INLINE uint32 GetRegister(const unsigned int id, char *special = NULL, const uint32 special_len = 0)
	{
	 uint32 value = 0xDEADBEEF;

	 switch(id)
	 {
	  case GSREG_PC:
		value = PC & 0xFFFF;
		break;

	  case GSREG_A:
		value = A;
		break;

	  case GSREG_X:
		value = X;
		break;

	  case GSREG_Y:
		value = Y;
		break;

	  case GSREG_SP:
		value = S;
		break;

	  case GSREG_P:
		value = P;	
		if(special)
		{
	 	trio_snprintf(special, special_len, "N: %d, V: %d, T: %d, D: %d, I: %d, Z: %d, C: %d", (int)(bool)(value & N_FLAG),
	        	(int)(bool)(value & V_FLAG),
		        (int)(bool)(value & T_FLAG),
			(int)(bool)(value & D_FLAG),
			(int)(bool)(value & I_FLAG),
		        (int)(bool)(value & Z_FLAG),
		        (int)(bool)(value & C_FLAG));
		}
		break;

  	case GSREG_SPD:
		value = speed;
		if(special)
		{
		 trio_snprintf(special, special_len, "%s(%s)", speed ? "High" : "Low", speed ? "7.16MHz" : "1.79MHz");
		}
		break;

	  case GSREG_MPR0:
	  case GSREG_MPR1:
	  case GSREG_MPR2:
	  case GSREG_MPR3:
	  case GSREG_MPR4:
	  case GSREG_MPR5:
	  case GSREG_MPR6:
	  case GSREG_MPR7:
		value = MPR[id - GSREG_MPR0];

		if(special)
		{
		 trio_snprintf(special, special_len, "0x%02X * 0x2000 = 0x%06X", value, (uint32)value * 0x2000);
		}
		break;

	  case GSREG_IRQM:
		value = IRQMask ^ 0x7;

		if(special)
		{
		 trio_snprintf(special, special_len, "IRQ2: %s, IRQ1: %s, Timer: %s", (value & IQIRQ2) ? "Disabled" : "Enabled",
				(value & IQIRQ1) ? "Disabled" : "Enabled", (value & IQTIMER) ? "Disabled" : "Enabled");
		}
		break;

	  case GSREG_TIMS:
		value = timer_status;

		if(special)
		{
		 trio_snprintf(special, special_len, "%s", (value & 1) ? "Enabled" : "Disabled");
		}
		break;

	  case GSREG_TIMV:
		value = timer_value;
		break;

	  case GSREG_TIML:
		value = timer_load;
		if(special)
		{
		 uint32 meowval = (value + 1) * 1024;
	  	 trio_snprintf(special, special_len, "(%d + 1) * 1024 = %d; 7,159,090.90... Hz / %d = %f Hz", value, meowval, meowval, (double)7159090.909090909091 / meowval);
		}
		break;

	  case GSREG_TIMD:
		value = timer_div;
		break;

	  case GSREG_STAMP:
		value = timestamp;
		break;
	 }
	 return(value);
	}

	//uint32 GetRegister(const unsigned int id, char *special = NULL, const uint32 special_len = 0);
	void SetRegister(const unsigned int id, uint32 value);

        INLINE void PokePhysical(uint32 address, uint8 data, bool hl = FALSE)
        {
	 address &= 0x1FFFFF;

         if(hl)
	 {
	  // FIXME: This is a very evil hack.
	  if(FastMap[address >> 13])
	   FastMap[address >> 13][address] = data;
	 }
	 else
          WriteMap[address >> 13](address, data);
        }

        INLINE void PokeLogical(uint16 address, uint8 data, bool hl = FALSE)
        {
         uint8 wmpr = MPR[address >> 13];

         PokePhysical((wmpr << 13) | (address & 0x1FFF), data, hl);
        }

	INLINE uint8 PeekPhysical(uint32 address)
	{
	 address &= 0x1FFFFF;

	 return(ReadMap[address >> 13](address));
	}

	INLINE uint8 PeekLogical(uint16 address)
	{
         uint8 wmpr = MPR[address >> 13];

	 return(PeekPhysical((wmpr << 13) | (address & 0x1FFF)));
	}

	void DumpMem(char *filename, uint32 start, uint32 end); // For debugging
	//
	// End Debugger Support Methods
	//

	INLINE void SetFastRead(unsigned int i, uint8 *ptr)
	{
	 assert(i < 0x100);

	 FastMap[i] = ptr ? (ptr - i * 8192) : NULL;
	}

        INLINE readfunc GetReadHandler(unsigned int i)
	{
	 assert(i < 0x100);
	 return(ReadMap[i]);
	}


	INLINE void SetReadHandler(unsigned int i, readfunc func)
	{
	 assert(i < 0x100);
	 ReadMap[i] = func;
	}

	INLINE void SetWriteHandler(unsigned int i, writefunc func)
	{
	 assert(i < 0x100);
	 WriteMap[i] = func;
	}

	// If external debugging code uses this function, then SetFastRead() must not be used with a pointer other than NULL for it to work
	// properly.
	INLINE uint32 GetLastLogicalReadAddr(void)
	{
	 return(LastLogicalReadAddr);
	}

	INLINE uint32 GetLastLogicalWriteAddr(void)
	{
	 return(LastLogicalWriteAddr);
	}

	private:

        void FlushMPRCache(void);

        INLINE void SetMPR(int i, int v)
        {
         MPR[i] = v;
         FastPageR[i] = FastMap[v] ? (FastMap[v] + v * 8192) - i * 8192 : NULL;
        }


	// Logical
	INLINE uint8 RdMem(unsigned int address)
	{
	 if(FastPageR[address >> 13])
	  return(FastPageR[address >> 13][address]);

	 LastLogicalReadAddr = address;

	 uint8 wmpr = MPR[address >> 13];
	 return(ReadMap[wmpr]((wmpr << 13) | (address & 0x1FFF)));
	}

	// Logical(warning: sets/clears isopread)
	INLINE uint8 RdOp(unsigned int address)
	{
	 if(FastPageR[address >> 13])
	  return(FastPageR[address >> 13][address]);

	 LastLogicalReadAddr = address;

	 isopread = 1;
	 uint8 wmpr = MPR[address >> 13];
	 uint8 ret = ReadMap[wmpr]((wmpr << 13) | (address & 0x1FFF));
	 isopread = 0;
	 return(ret);
	}

	// Logical
	INLINE void WrMem(unsigned int address, uint8 V)
	{
	 uint8 wmpr = MPR[address >> 13];

	 LastLogicalWriteAddr = address;

	 WriteMap[wmpr]((wmpr << 13) | (address & 0x1FFF), V);
	}

	// Used for ST0, ST1, ST2
	// Must not modify address(upper bit is abused for ST0/ST1/ST2 handling).
	INLINE void WrMemPhysical(uint32 address, uint8 data)
	{
	 WriteMap[(address >> 13) & 0xFF](address, data);
	}

	INLINE void REDOPIMCACHE(void)
	{ 
	 PIMaskCache = (P & I_FLAG) ? 0 : ~0; 
	}

	INLINE void REDOSPEEDCACHE(void)
	{
	 speed_shift_cache = (speed ^ 1) << 1;
	}

	INLINE void ADDCYC(int x)
	{
	 int master = (x * 3) << speed_shift_cache;

	 timestamp += master;
	 next_event -= master;
	 next_user_event -= master;

	 if(next_event <= 0)
	  HappySync();
	}

        INLINE void ADDCYC_MASTER(int master)
        {
         timestamp += master;
         next_event -= master;
         next_user_event -= master;

         if(next_event <= 0)
          HappySync();
        }


	void HappySync(void);

	INLINE void CalcNextEvent(void)
	{
	 next_event = timer_div;

	 if(next_event > next_user_event)
	  next_event = next_user_event;
	}

	void X_ZN(const uint8);
	void X_ZNT(const uint8);

	void PUSH(const uint8 V);
	uint8 POP(void);
	void JR(const bool cond);
	void BBRi(const uint8 val, const unsigned int bitto);
	void BBSi(const uint8 val, const unsigned int bitto);

	private:
	int runrunrun;

	uint32 timestamp;
        uint32 PC;		// Program Counter(16-bit, but as a 32-bit variable for performance reasons)
        uint8 A;		// Accumulator
	uint8 X;		// X Index register
	uint8 Y;		// Y Indes register
	uint8 S;		// Stack Pointer
	uint8 P;		// Processor Flags/Status Register
	uint32 PIMaskCache;	// Will be = 0 if (P & I_FLAG) is set, ~0 if (P & I_FLAG) is clear.
	uint8 MPR[9];		// 8, + 1 for PC overflow from $ffff to $10000

	uint8 *FastPageR[9];	// Fast page read cache for each 8KiB in the 16-bit logical address space
				// (Reloaded on corresponding MPR change)

	int32 next_event;	// Next event, period.  Timer, user, ALIENS ARE INVADING SAVE ME HELP

	int32 next_user_event;

	int32 timer_lastts;
	
	uint8 IRQMask;

	uint32 LastLogicalReadAddr;		// Some helper variables for debugging code(external)
	uint32 LastLogicalWriteAddr;		// to know where the read/write occurred in the 16-bit logical space.

	uint8 lastop;

        uint32 IRQlow;          /* Simulated IRQ pin held low(or is it high?).
                                   And other junk hooked on for speed reasons.*/
	int32 IRQSample;
	int32 IFlagSample;	

	uint8 speed;
	uint8 speed_shift_cache;

	bool timer_inreload;
	uint8 timer_status;
	int32 timer_value, timer_load;
	int32 timer_div;

	uint8 IODataBuffer;

	enum
	{
	 IBM_TIA = 1,
	 IBM_TAI = 2,
	 IBM_TDD = 3,
	 IBM_TII = 4,
	 IBM_TIN = 5
	};
	uint32 in_block_move;
	uint16 bmt_src, bmt_dest, bmt_length;
	uint32 bmt_alternate;
	bool isopread;

	void (*CPUHook)(uint32);
	void (*ADDBT)(uint32);

	HuC6280_Support *EventHandler;

	uint8 *FastMap[0x100];		// Direct pointers to memory for mapped RAM and ROM for faster reads(biased to remove the need for an & operation).
	readfunc ReadMap[0x100];	// Read handler pointers for each 8KiB in the 21-bit physical address space.
	writefunc WriteMap[0x100];	// Write handler pointers for each 8KiB in the 21-bit physical address space.

	const bool EmulateWAI;		// For speed hacks
};

#endif
