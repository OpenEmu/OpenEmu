#ifndef __MDFN_DEBUG_H
#define __MDFN_DEBUG_H

#ifdef WANT_DEBUGGER

enum
{
 BPOINT_READ = 1,
 BPOINT_WRITE,
 BPOINT_PC,
 BPOINT_IO_READ,
 BPOINT_IO_WRITE,
 BPOINT_AUX_READ,
 BPOINT_AUX_WRITE,
 BPOINT_OP	// Opcode
};

struct RegType
{
	~RegType();
	const unsigned int id;
        std::string name;
	std::string long_name;
        unsigned int bsize; // Byte size, 1, 2, 4
};

typedef struct
{
 const char *name;
 RegType *Regs;

 // GetRegister() should modify the string at special if special is non-NULL to provide
 // more details about the register.
 uint32 (*GetRegister)(const unsigned int id, char *special, const uint32 special_len);
 void (*SetRegister)(const unsigned int id, uint32 value);

 // Used if SetRegister and GetRegister are both NULL.  Deprecated.
 uint32 (*OLDGetRegister)(const std::string &name, std::string *special);
 void (*OLDSetRegister)(const std::string &name, uint32 value);
} RegGroupType;

typedef enum
{
 ASPACE_WFMT_UNSIGNED = 0,	// Unsigned, wooo.
 ASPACE_WFMT_SIGNED,		// Signed two's complement
 ASPACE_WMFT_SIGNED_MAG_0NEG,	// Signed/magnitude. 0 in the sign bit means negative.
 ASPACE_WFMT_SIGNED_MAG_1NEG,	// Sign/magnitude style. 1 in the sign bit means negative.
 ASPACE_WFMT_SIGNED_ONES,	// Signed one's complement
} ASpace_WFMT;

// Visible to CPU, physical, RAM, ROM, ADPCM RAM, etc etc.
typedef struct
{
	// The short name, all lowercase, 0-9 a-z _
	char *name;

	// The longer, descriptive name for this address space.
	char *long_name;

	// The number of address bits for this address space.
	uint32 TotalBits;

	// (Segmentation bits TODO)
	bool IsSegmented;
	uint32 SegmentBits;
	uint32 OffsetBits;
	uint32 BitsOverlapped; // For future use, maybe.

	// Non-power-of-2 size.  Normally 0, unless the size of the address space isn't a power of 2!
	uint32 NP2Size;

	bool IsWave;
	ASpace_WFMT WaveFormat;	// TODO
	uint32 WaveBits;	// Total number of bits(including sign) per waveform sample.  Only <= 8 for now...
	
	void (*GetAddressSpaceBytes)(const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
	void (*PutAddressSpaceBytes)(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer);

	void *private_data;

	// Internal use...
	void (*EnableUsageMap)(bool);

	uint64 ****UsageMapRead;	// [a.24:31][a.16:23][a.8:15][a.0:7], pointer tables allocated as needed.
	uint64 ****UsageMapWrite;	// (same)

	uint64 UsageReadMemUsed;	// Keep track of how much memory we've allocated for UsageMap, so we don't go kaka-kookoo and use up an 					// excessive amount of RAM.
	uint64 UsageWriteMemUsed;
} AddressSpaceType;

typedef struct
{
 const char *DefaultCharEnc;	// Default(or most common) internal character encoding for text for the system.

 uint32 MaxInstructionSize;	// Maximum instruction size in bytes
 uint32 InstructionAlignment;	// Required instruction alignment, in bytes(frequently 1 on CISC, and other powers of 2 on RISC, but not always ;)).
 uint32 LogAddrBits; // Logical address bits
 uint32 PhysAddrBits; // Physical address bits

 uint32 DefaultWatchAddr;

 uint32 ZPAddr; // Set to ~0 to disable

 // If logical is true, then do the peek/poke on logical address A, else do the
 // peek/poke on physical address A.  For now, this distinction only exists
 // on CPUs with built-in bank-switching, like the HuC6280.

 // If hl is true, do not cause any change in the underlying hardware state.
 uint32 (*MemPeek)(uint32 A, unsigned int bsize, bool hl, bool logical);

 // Disassemble one instruction at logical address A, and increment A to point to the next instruction.
 // TextBuf should point to at least 256 bytes of space!
 void (*Disassemble)(uint32 &A, uint32 SpecialA, char *TextBuf);

 // Toggle syntax mode(example: for WonderSwan x86 decoding, Intel or AT&T)
 void (*ToggleSyntax)(void);

 // Force an IRQ at the desired level(IRQ0, IRQ1, or whatever).  Pass -1 to cause an NMI, if available.
 // Note that this should cause an interrupt regardless of any flag settings or interrupt status.
 void (*IRQ)(int level);

 // Get the vector for the specified IRQ level.  -1 is NMI(if available), and -2 is RESET.
 uint32 (*GetVector)(int level);

 void (*FlushBreakPoints)(int type);

 void (*AddBreakPoint)(int type, unsigned int A1, unsigned int A2, bool logical);

 void (*SetCPUCallback)(void (*callb)(uint32 PC));

 void (*SetBPCallback)(void (*callb)(uint32 PC));

 std::vector<std::string> (*GetBranchTrace)(void);

 // If surface is NULL, disable decoding.
 // If line is -1, do decoding instantaneously, before this function returns.
 // "which" is the same index as passed to MDFNI_ToggleLayer()
 void (*SetGraphicsDecode)(MDFN_Surface *surface, int line, int which, int xscroll, int yscroll, int pbn);

 void (*SetLogFunc)(void (*logfunc)(const char *type, const char *text));

 // Game emulation code shouldn't touch these directly.
 std::vector<AddressSpaceType> *AddressSpaces;
 std::vector<RegGroupType*> *RegGroups;
} DebuggerInfoStruct;

// ASpace_Add() functions return an ID that should be used with with MDFNDBG_ASpace_Read()
// and ASpace_Write() functions.  The ID is guaranteed to be 0 for the first address space,
// and increment by 1 for each address space added thereafter.
// An address space should only be added during Load() or LoadCD().
int ASpace_Add(void (*gasb)(const char *name, uint32 Address, uint32 Length, uint8 *Buffer),
        void (*pasb)(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer), const char *name, const char *long_name,
        uint32 TotalBits, uint32 NP2Size = 0, bool IsSegmented = 0, uint32 SegmentBits = 0, uint32 OffsetBits = 0, uint32 BitsOverlapped = 0);

int ASpace_Add(const AddressSpaceType &);

// Removes all registered address spaces.
void ASpace_Reset(void);

// pre_bpoint should be TRUE if these are "estimated" read/writes that will occur when the current instruction
// is actually executed/committed.
// size is the size of the read/write(ex: 1 byte, 2 bytes, 4 bytes), defaulting to 1 byte.
//
// The return value is always FALSE if pre_bpoint is FALSE.  If pre_bpoint is TRUE, the return value will be
// TRUE if the "estimated" read/write matches a registered breakpoint.
bool ASpace_Read(const int id, const uint32 address, const unsigned int size = 1, const bool pre_bpoint = FALSE);
bool ASpace_Write(const int id, const uint32 address, const uint32 value, const unsigned int size = 1, const bool pre_bpoint = FALSE);

// Clears read/write usage maps.
void ASpace_ClearReadMap(const int id);
void ASpace_ClearWriteMap(const int id);


void ASpace_AddBreakPoint(const int id, const int type, const uint32 A1, const uint32 A2, const bool logical);
void ASpace_FlushBreakPoints(const int id);


void MDFNDBG_ResetRegGroupsInfo(void);
void MDFNDBG_AddRegGroup(RegGroupType *groupie);


void MDFNDBG_Init(void);
void MDFNDBG_PostGameLoad(void);
void MDFNDBG_Kill(void);

#endif

#endif
