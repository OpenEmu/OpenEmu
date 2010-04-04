#ifndef _GIT_H
#define _GIT_H

#include <string>

#include "file.h"

enum
{
 MDFN_ROTATE0 = 0,
 MDFN_ROTATE90,
 MDFN_ROTATE270
};

typedef enum
{
 VIDSYS_NONE, // Can be used internally in system emulation code, but it is an error condition to let it continue to be
	      // after the Load() or LoadCD() function returns!
 VIDSYS_PAL,
 VIDSYS_PAL_M, // Same timing as NTSC, but uses PAL-style colour encoding
 VIDSYS_NTSC,
 VIDSYS_SECAM
} VideoSystems;

typedef enum
{
 GMT_CART,	// Self-explanatory!
 GMT_ARCADE,	// VS Unisystem, PC-10...
 GMT_DISK,	// Famicom Disk System, mostly
 GMT_CDROM,	// PC Engine CD, PC-FX
 GMT_PLAYER	// Music player(NSF, HES, GSF)
} GameMediumTypes;

#include "state.h"
#include "settings-common.h"

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

typedef struct
{
        std::string name;
	std::string long_name;
        unsigned int bsize; // Byte size, 1, 2, 4
} RegType;

typedef struct
{
 RegType *Regs;

 // GetRegister() should modify the string at *special if special is non-NULL to provide
 // more details about the register.
 uint32 (*GetRegister)(const std::string &name, std::string *special);

 void (*SetRegister)(const std::string &name, uint32 value);
} RegGroupType;

// Visible to CPU, physical, RAM, ROM, ADPCM RAM, etc etc.
typedef struct
{
	char *name;
	char *long_name;

	uint32 TotalBits;
	bool IsSegmented;
	uint32 SegmentBits;
	uint32 OffsetBits;
	uint32 BitsOverlapped; // For future use, maybe.
	uint32 NP2Size;

	void (*GetAddressSpaceBytes)(const char *name, uint32 Address, uint32 Length, uint8 *Buffer);
	void (*PutAddressSpaceBytes)(const char *name, uint32 Address, uint32 Length, uint32 Granularity, bool hl, const uint8 *Buffer);

	void *private_data;
} AddressSpaceType;

typedef struct
{
 uint32 MaxInstructionSize; // Maximum instruction size in bytes

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

 void (*SetGraphicsDecode)(int line, int which, int w, int h, int xscroll, int yscroll, int pbn);
						 // If line is -1, disable decoding(the default).  If line is 0xB00B13,
						 // do decoding instantaneously, before this function returns. "which" is the same index as passed to 
						 // MDFNI_ToggleLayer()
 uint32 *(*GetGraphicsDecodeBuffer)(void); // Returns NULL if graphics decoding is off.

 void (*SetLogFunc)(void (*logfunc)(const char *type, const char *text));

 // Game emulation code shouldn't touch these directly.
 std::vector<AddressSpaceType> *AddressSpaces;
 std::vector<RegGroupType*> *RegGroups;
} DebuggerInfoStruct;

#include "debug.h"

#endif

typedef enum
{
 IDIT_BUTTON,		// 1-bit
 IDIT_BUTTON_CAN_RAPID, // 1-bit
 IDIT_BUTTON_BYTE, // 8-bits, Button as a byte instead of a bit.
 IDIT_X_AXIS,	   // 32-bits
 IDIT_Y_AXIS,	   // 32-bits
 IDIT_X_AXIS_REL,  // 32-bits, signed
 IDIT_Y_AXIS_REL,  // 32-bits, signed
 IDIT_BYTE_SPECIAL,
} InputDeviceInputType;

typedef struct
{
	const char *SettingName;	// No spaces, shouldbe all a-z0-9 and _. Definitely no ~!
	const char *Name;
        const int ConfigOrder;          // Configuration order during in-game config process, -1 for no config.
	const InputDeviceInputType Type;
	const char *ExcludeName;	// SettingName of a button that can't be pressed at the same time as this button
					// due to physical limitations.
} InputDeviceInputInfoStruct;

typedef struct
{
 const char *ShortName;
 const char *FullName;

 //struct InputPortInfoStruct *PortExpanderDeviceInfo;
 const void *PortExpanderDeviceInfo;
 int NumInputs; // Usually just the number of buttons....OR if PortExpanderDeviceInfo is non-NULL, it's the number of input
		// ports this port expander device provides.
 const InputDeviceInputInfoStruct *IDII;
} InputDeviceInfoStruct;

typedef struct
{
 const int pid_offset;
 const char *ShortName;
 const char *FullName;
 int NumTypes; // Number of unique input devices available for this input port
 InputDeviceInfoStruct *DeviceInfo;
} InputPortInfoStruct;

typedef struct
{
 int InputPorts;
 const InputPortInfoStruct *Types;
} InputInfoStruct;

typedef struct
{
	uint32 *pixels;
	MDFN_Rect *LineWidths;
	bool *IsFMV;
	int skip;

	int16 **SoundBuf;
	int32 *SoundBufSize;

	float soundmultiplier;

	bool NeedRewind;
	bool NeedSoundReverse; // Sound reversal during state rewinding is normally done in mednafen.cpp, but
			       // individual system emulation code can also do it if this is set, and clear it after it's done.
			       // (Also, the driver code shouldn't touch this variable)
} EmulateSpecStruct;

typedef struct
{
 /* Private functions to Mednafen.  Do not call directly
    from the driver code, or else bad things shall happen.  Maybe.  Probably not, but don't
    do it(yet)!
 */
 const char *shortname;
 #ifdef WANT_DEBUGGER
 DebuggerInfoStruct *Debugger;
 #endif
 InputInfoStruct *InputInfo;
 int (*Load)(const char *name, MDFNFILE *fp);
 int (*LoadCD)(void);
 void (*CloseGame)(void);
 bool (*ToggleLayer)(int which);
 const char *LayerNames;
 void (*InstallReadPatch)(uint32 address);
 void (*RemoveReadPatches)(void);
 uint8 (*MemRead)(uint32 addr);

 int (*StateAction)(StateMem *sm, int load, int data_only);
 void (*Emulate)(EmulateSpecStruct *espec); //uint32 *pXBuf, MDFN_Rect *LineWidths, int16 **SoundBuf, int32 *SoundBufSize, int skip);
 void (*SetPixelFormat)(int rshift, int gshift, int bshift);
 void (*SetInput)(int port, const char *type, void *ptr);
 int (*DiskInsert)(int oride);
 int (*DiskEject)(void);
 int (*DiskSelect)(void);

 void (*SetSoundMultiplier)(double multiplier);
 void (*SetSoundVolume)(uint32 volume);
 void (*Sound)(int32 Rate);
 void (*DoSimpleCommand)(int cmd);

 MDFNSetting *Settings;

 uint32 fps; // frames per second * 65536 * 256, truncated
 void *fb;
 int width, height;
 int ss_preview_width; // Width for save state previews
 int pitch;
 MDFN_Rect DisplayRect;

 int rotated;
 uint8 *name;    /* Game name, UTF8 encoding */
 uint8 MD5[16];
 uint8 GameSetMD5[16];	/* A unique ID for the game set this CD belongs to, only used in PC-FX emulation. */
 bool GameSetMD5Valid; /* True if GameSetMD5 is valid. */

 int soundrate;  /* For Ogg Vorbis expansion sound wacky support.  0 for default. */
 int soundchan;  /* Number of sound channels. */

 VideoSystems VideoSystem;
 GameMediumTypes GameType;

 const char *cspecial;  /* Special cart expansion: DIP switches, barcode reader, etc. */

 std::vector<const char *>DesiredInput; // Desired input device for the input ports, NULL for don't care

 double mouse_sensitivity;
 /* Private to Mednafen: */
 int rshift, gshift, bshift;
} MDFNGI;
#endif
