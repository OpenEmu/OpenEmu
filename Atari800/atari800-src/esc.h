#ifndef ESC_H_
#define ESC_H_

/* TRUE to enable patched (fast) Serial I/O. */
extern int ESC_enable_sio_patch;

/* Escape codes used to mark places in 6502 code that must
   be handled specially by the emulator. An escape sequence
   is an illegal 6502 opcode 0xF2 or 0xD2 followed
   by one of these escape codes: */
enum ESC_t {

	/* SIO patch. */
	ESC_SIOV,

	/* stdio-based handlers for the BASIC version
	   and handlers for Atari Basic loader. */
	ESC_EHOPEN,
	ESC_EHCLOS,
	ESC_EHREAD,
	ESC_EHWRIT,
	ESC_EHSTAT,
	ESC_EHSPEC,

	ESC_KHOPEN,
	ESC_KHCLOS,
	ESC_KHREAD,
	ESC_KHWRIT,
	ESC_KHSTAT,
	ESC_KHSPEC,

	/* Atari executable loader. */
	ESC_BINLOADER_CONT,

	/* Cassette emulation. */
	ESC_COPENLOAD = 0xa8,
	ESC_COPENSAVE = 0xa9,

	/* Printer. */
	ESC_PHOPEN = 0xb0,
	ESC_PHCLOS = 0xb1,
	ESC_PHREAD = 0xb2,
	ESC_PHWRIT = 0xb3,
	ESC_PHSTAT = 0xb4,
	ESC_PHSPEC = 0xb5,
	ESC_PHINIT = 0xb6,

#ifdef R_IO_DEVICE
	/* R: device. */
	ESC_ROPEN = 0xd0,
	ESC_RCLOS = 0xd1,
	ESC_RREAD = 0xd2,
	ESC_RWRIT = 0xd3,
	ESC_RSTAT = 0xd4,
	ESC_RSPEC = 0xd5,
	ESC_RINIT = 0xd6,
#endif

	/* H: device. */
	ESC_HHOPEN = 0xc0,
	ESC_HHCLOS = 0xc1,
	ESC_HHREAD = 0xc2,
	ESC_HHWRIT = 0xc3,
	ESC_HHSTAT = 0xc4,
	ESC_HHSPEC = 0xc5,
	ESC_HHINIT = 0xc6,

	/* B: device. */
	ESC_BOPEN = 0xe0,
	ESC_BCLOS = 0xe1,
	ESC_BREAD = 0xe2,
	ESC_BWRIT = 0xe3,
	ESC_BSTAT = 0xe4,
	ESC_BSPEC = 0xe5,
	ESC_BINIT = 0xe6
};

/* A function called to handle an escape sequence. */
typedef void (*ESC_FunctionType)(void);

/* Puts an escape sequence at the specified address. */
void ESC_Add(UWORD address, UBYTE esc_code, ESC_FunctionType function);

/* Puts an escape sequence followed by the RTS instruction. */
void ESC_AddEscRts(UWORD address, UBYTE esc_code, ESC_FunctionType function);

/* Puts an escape sequence with an integrated RTS. */
void ESC_AddEscRts2(UWORD address, UBYTE esc_code, ESC_FunctionType function);

/* Unregisters an escape sequence. You must cleanup the Atari memory yourself. */
void ESC_Remove(UBYTE esc_code);

/* Handles an escape sequence. */
void ESC_Run(UBYTE esc_code);

/* Installs SIO patch and disables ROM checksum test. */
void ESC_PatchOS(void);

/* Unregisters all escape sequences */
void ESC_ClearAll(void);

/* Reinitializes patches after enable_*_patch change. */
void ESC_UpdatePatches(void);

#endif /* ESC_H_ */
