/*************************************************************************

    Atari Slapstic decoding helper

**************************************************************************

    Atari Slapstic FAQ
    Version 1.12
    by Aaron Giles and Frank Palazzolo
    9/12/2002


    What is a slapstic?

    The slapstic was a security chip made by Atari, which was used for
    bank switching and security in several coin-operated video games from
    1984 through 1990.


    What is a SLOOP?

    The SLOOP (or "SLOOPstic") is a follow-on chip to the slapstic. It
    provides a similar type of security, but is programmed onto a GAL6001,
    rather than a custom part. It was created because Atari was running
    out of slapstics to use in their games, and the original masks for the
    slapstic had been lost by the company that manufactured them. A separate
    FAQ for this chip is planned for the future.


    How do I identify a slapstic chip on my board?

    Look for a small, socketed 20-pin DIP on the board. The number on
    the chip will be 137412-1xx.


    Are slapstic chips interchangeable?

    Sadly, no. They were designed to prevent operators from burning
    new EPROMs and "upgrading" their PCBs to a new game without buying
    the necessary kits from Atari. For example, the five System 1 games
    each used a different slapstic, so that you couldn't take, say,
    a <b>Marble Madness</b> machine, burn new EPROMs, and convert it into
    an <b>Indiana Jones</b>.

    That said, however, there are two pairs of the slapstics that appear
    to be functionally identical, despite the fact that they have
    different part numbers:

        137412-103 (<b>Marble Madness</b>) appears to be functionally identical
            to 137412-110 (<b>Road Blasters</b> & <b>APB</b>)

        137412-106 (<b>Gauntlet II</b>) appears to be functionally identical
            to 137412-109 (<b>Championship Sprint</b>)

    Note, however, that I have not tried these swaps to confirm that they
    work. Your mileage may vary.


    How many different slapstics are there?

    All told, a total of 13 actual slapstics have been found. However,
    there are gaps in the numbering which indicate that more may exist.


    Do all slapstics work the same?

    In general, yes. However, matters are complicated by the existence
    of multiple revisions of the chip design:

        SLAPSTIC    Part #137412-101 through 137412-110
        SLAPSTIC-2  Part #137412-111 through 137412-118

    In the simplest case, both revs act the same. However, they differ
    in how the more complex modes of operation are used.


    How is the slapstic connected to the game?

    The slapstic generally sits between the CPU's address bus and one
    of the program ROMs. Here's a pinout:

            A9   1 +-v-+ 20  A8
            A10  2 |   | 19  A7
            A11  3 |   | 18  A6
            A12  4 |   | 17  A5
            A13  5 |   | 16  A4
            /CS  6 |   | 15  A3
            CLK  7 |   | 14  A2
            VCC  8 |   | 13  A1
            BS1  9 |   | 12  A0
            BS0 10 +---+ 11 GND

    A0-A13 are the address lines from the CPU. CLK and /CS together
    trigger a state change. BS0 and BS1 are the bank select outputs,
    which usually connect to the protected program ROM in place of
    two address lines (traditionally A12 and A13).

    Most slapstics were used on 68000 or T-11 based games, which had
    a 16-bit address bus. This meant that A0-A13 on the slapstic were
    generally connected to A1-A14 on the CPU. However, two 8-bit
    games (Tetris and Empire Strikes Back) used the slapstic as well.
    This slapstic (#101) has a slightly different pinout, though it
    operates similarly to the others in its class.

            A8   1 +-v-+ 20  A7
            A9   2 |   | 19  A6
            A10  3 |   | 18  A5
            A11  4 |   | 17  A4
            A12  5 |   | 16  A3
            /CS  6 |   | 15  A2
            CLK  7 |   | 14  A1
            VCC  8 |   | 13  A0
            /BS1 9 |   | 12 GND
            BS1 10 +---+ 11 BS0


    Which games used slapstics?

        137412-101  Empire Strikes Back
        137412-101  Tetris
        137412-103  Marble Madness
        137412-104  Gauntlet
        137412-105  Paperboy
        137412-105  Indiana Jones & the Temple of Doom
        137412-106  Gauntlet II
        137412-107  2-Player Gauntlet
        137412-107  Peter Packrat
        137412-107  720 Degrees
        137412-107  Xybots
        137412-108  Road Runner
        137412-108  Super Sprint
        137412-109  Championship Sprint
        137412-109  Road Blasters (some versions)
        137412-110  Road Blasters
        137412-110  APB
        137412-111  Pit Fighter
        137412-112  Pit Fighter (Europe)
        137412-113  Unknown (Europe)
        137412-115  Race Drivin' DSK board
        137412-116  Hydra
        137412-116  Tournament Cyberball 2072
        137412-117  Race Drivin' main board
        137412-118  Rampart
        137412-118  Vindicators Part II


    How does the slapstic work?

    On power-up, the slapstic starts by pointing to bank 0 or bank 3.
    After that, certain sequences of addresses will trigger a bankswitch.
    Each sequence begins with an access to location $0000, followed by one
    or more special addresses.

    Each slapstic has a 'simple' mode of bankswitching, consisting of an
    access to $0000 followed by an access to one of four bank addresses.
    Other accesses are allowed in between these two accesses without
    affecting the outcome.

    Additionally, each slapstic has a trickier variant of the
    bankswitching, which requires an access to $0000, followed by accesses
    to two specific addresses, followed by one of four alternate bank
    addresses. All three accesses following the $0000 must occur in
    sequence with no interruptions, or else the sequence is invalidated.

    Finally, each slapstic has a mechanism for modifying the value of the
    current bank. Earlier chips (101-110) allowed you to twiddle the
    specific bits of the bank number, clearing or setting bits 0 and 1
    independently. Later chips (111-118) provided a mechanism of adding
    1, 2, or 3 to the number of the current bank.

    Surprisingly, the slapstic appears to have used DRAM cells to store
    the current bank. After 5 or 6 seconds without a clock, the chip
    reverts to the default bank, with the chip reset (bank select
    addresses are enabled). Typically, the slapstic region is accessed
    often enough to cause a problem.

    For full details, see the MAME source code.

    Ported from MAME 0.146u3
    *alt2_kludge may need additional code fixes

*************************************************************************/

#include "burnint.h"
#include "m68000_intf.h"
#include "m68000_debug.h"


/*************************************
 *
 *  Structure of slapstic params
 *
 *************************************/

struct mask_value
{
	INT32 mask, value;
};


struct slapstic_data
{
	INT32 bankstart;
	INT32 bank[4];

	struct mask_value alt1;
	struct mask_value alt2;
	struct mask_value alt3;
	struct mask_value alt4;
	INT32 altshift;

	struct mask_value bit1;
	struct mask_value bit2c0;
	struct mask_value bit2s0;
	struct mask_value bit2c1;
	struct mask_value bit2s1;
	struct mask_value bit3;

	struct mask_value add1;
	struct mask_value add2;
	struct mask_value addplus1;
	struct mask_value addplus2;
	struct mask_value add3;
};


/*************************************
 *
 *  Shorthand
 *
 *************************************/

#define UNKNOWN 0xffff
#define NO_BITWISE			\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN }
#define NO_ADDITIVE			\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN },	\
	{ UNKNOWN,UNKNOWN }

#define MATCHES_MASK_VALUE(val, maskval)	(((val) & (maskval).mask) == (maskval).value)


/*************************************
 *
 *  Constants
 *
 *************************************/

enum
{
	DISABLED,
	ENABLED,
	ALTERNATE1,
	ALTERNATE2,
	ALTERNATE3,
	BITWISE1,
	BITWISE2,
	BITWISE3,
	ADDITIVE1,
	ADDITIVE2,
	ADDITIVE3
};


/*************************************
 *
 *  Slapstic definitions
 *
 *************************************/

/* slapstic 137412-101: Empire Strikes Back/Tetris (NOT confirmed) */
static const struct slapstic_data slapstic101 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0080,0x0090,0x00a0,0x00b0 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,UNKNOWN },				/* 1st mask/value in sequence */
	{ 0x1fff,0x1dff },				/* 2nd mask/value in sequence */
	{ 0x1ffc,0x1b5c },				/* 3rd mask/value in sequence */
	{ 0x1fcf,0x0080 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x1ff0,0x1540 },				/* 1st mask/value in sequence */
	{ 0x1ff3,0x1540 },				/* clear bit 0 value */
	{ 0x1ff3,0x1541 },				/*   set bit 0 value */
	{ 0x1ff3,0x1542 },				/* clear bit 1 value */
	{ 0x1ff3,0x1543 },				/*   set bit 1 value */
	{ 0x1ff8,0x1550 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-103: Marble Madness (confirmed) */
static const struct slapstic_data slapstic103 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0040,0x0050,0x0060,0x0070 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x002d },				/* 1st mask/value in sequence */
	{ 0x3fff,0x3d14 },				/* 2nd mask/value in sequence */
	{ 0x3ffc,0x3d24 },				/* 3rd mask/value in sequence */
	{ 0x3fcf,0x0040 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x34c0 },				/* 1st mask/value in sequence */
	{ 0x3ff3,0x34c0 },				/* clear bit 0 value */
	{ 0x3ff3,0x34c1 },				/*   set bit 0 value */
	{ 0x3ff3,0x34c2 },				/* clear bit 1 value */
	{ 0x3ff3,0x34c3 },				/*   set bit 1 value */
	{ 0x3ff8,0x34d0 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-104: Gauntlet (confirmed) */
static const struct slapstic_data slapstic104 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0020,0x0028,0x0030,0x0038 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0069 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x3735 },				/* 2nd mask/value in sequence */
	{ 0x3ffc,0x3764 },				/* 3rd mask/value in sequence */
	{ 0x3fe7,0x0020 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x3d90 },				/* 1st mask/value in sequence */
	{ 0x3ff3,0x3d90 },				/* clear bit 0 value */
	{ 0x3ff3,0x3d91 },				/*   set bit 0 value */
	{ 0x3ff3,0x3d92 },				/* clear bit 1 value */
	{ 0x3ff3,0x3d93 },				/*   set bit 1 value */
	{ 0x3ff8,0x3da0 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-105: Indiana Jones/Paperboy (confirmed) */
static const struct slapstic_data slapstic105 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0010,0x0014,0x0018,0x001c },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x003d },				/* 1st mask/value in sequence */
	{ 0x3fff,0x0092 },				/* 2nd mask/value in sequence */
	{ 0x3ffc,0x00a4 },				/* 3rd mask/value in sequence */
	{ 0x3ff3,0x0010 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x35b0 },				/* 1st mask/value in sequence */
	{ 0x3ff3,0x35b0 },				/* clear bit 0 value */
	{ 0x3ff3,0x35b1 },				/*   set bit 0 value */
	{ 0x3ff3,0x35b2 },				/* clear bit 1 value */
	{ 0x3ff3,0x35b3 },				/*   set bit 1 value */
	{ 0x3ff8,0x35c0 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-106: Gauntlet II (confirmed) */
static const struct slapstic_data slapstic106 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0008,0x000a,0x000c,0x000e },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x002b },				/* 1st mask/value in sequence */
	{ 0x3fff,0x0052 },				/* 2nd mask/value in sequence */
	{ 0x3ffc,0x0064 },				/* 3rd mask/value in sequence */
	{ 0x3ff9,0x0008 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x3da0 },				/* 1st mask/value in sequence */
	{ 0x3ff3,0x3da0 },				/* clear bit 0 value */
	{ 0x3ff3,0x3da1 },				/*   set bit 0 value */
	{ 0x3ff3,0x3da2 },				/* clear bit 1 value */
	{ 0x3ff3,0x3da3 },				/*   set bit 1 value */
	{ 0x3ff8,0x3db0 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-107: Peter Packrat/Xybots/2p Gauntlet/720 (confirmed) */
static const struct slapstic_data slapstic107 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0018,0x001a,0x001c,0x001e },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x006b },				/* 1st mask/value in sequence */
	{ 0x3fff,0x3d52 },				/* 2nd mask/value in sequence */
	{ 0x3ffc,0x3d64 },				/* 3rd mask/value in sequence */
	{ 0x3ff9,0x0018 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x00a0 },				/* 1st mask/value in sequence */
	{ 0x3ff3,0x00a0 },				/* clear bit 0 value */
	{ 0x3ff3,0x00a1 },				/*   set bit 0 value */
	{ 0x3ff3,0x00a2 },				/* clear bit 1 value */
	{ 0x3ff3,0x00a3 },				/*   set bit 1 value */
	{ 0x3ff8,0x00b0 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-108: Road Runner/Super Sprint (confirmed) */
static const struct slapstic_data slapstic108 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0028,0x002a,0x002c,0x002e },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x001f },				/* 1st mask/value in sequence */
	{ 0x3fff,0x3772 },				/* 2nd mask/value in sequence */
	{ 0x3ffc,0x3764 },				/* 3rd mask/value in sequence */
	{ 0x3ff9,0x0028 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x0060 },				/* 1st mask/value in sequence */
	{ 0x3ff3,0x0060 },				/* clear bit 0 value */
	{ 0x3ff3,0x0061 },				/*   set bit 0 value */
	{ 0x3ff3,0x0062 },				/* clear bit 1 value */
	{ 0x3ff3,0x0063 },				/*   set bit 1 value */
	{ 0x3ff8,0x0070 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-109: Championship Sprint/Road Blasters (confirmed) */
static const struct slapstic_data slapstic109 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0008,0x000a,0x000c,0x000e },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x002b },				/* 1st mask/value in sequence */
	{ 0x3fff,0x0052 },				/* 2nd mask/value in sequence */
	{ 0x3ffc,0x0064 },				/* 3rd mask/value in sequence */
	{ 0x3ff9,0x0008 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x3da0 },				/* 1st mask/value in sequence */
	{ 0x3ff3,0x3da0 },				/* clear bit 0 value */
	{ 0x3ff3,0x3da1 },				/*   set bit 0 value */
	{ 0x3ff3,0x3da2 },				/* clear bit 1 value */
	{ 0x3ff3,0x3da3 },				/*   set bit 1 value */
	{ 0x3ff8,0x3db0 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/* slapstic 137412-110: Road Blasters/APB (confirmed) */
static const struct slapstic_data slapstic110 =
{
	/* basic banking */
	3,						/* starting bank */
	{ 0x0040,0x0050,0x0060,0x0070 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x002d },				/* 1st mask/value in sequence */
	{ 0x3fff,0x3d14 },				/* 2nd mask/value in sequence */
	{ 0x3ffc,0x3d24 },				/* 3rd mask/value in sequence */
	{ 0x3fcf,0x0040 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	{ 0x3ff0,0x34c0 },				/* 1st mask/value in sequence */
	{ 0x3ff3,0x34c0 },				/* clear bit 0 value */
	{ 0x3ff3,0x34c1 },				/*   set bit 0 value */
	{ 0x3ff3,0x34c2 },				/* clear bit 1 value */
	{ 0x3ff3,0x34c3 },				/*   set bit 1 value */
	{ 0x3ff8,0x34d0 },				/* final mask/value in sequence */

	/* additive banking */
	NO_ADDITIVE
};


/*************************************
 *
 *  Slapstic-2 definitions
 *
 *************************************/

/* slapstic 137412-111: Pit Fighter (confirmed) */
static const struct slapstic_data slapstic111 =
{
	/* basic banking */
	0,						/* starting bank */
	{ 0x0042,0x0052,0x0062,0x0072 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x000a },				/* 1st mask/value in sequence */
	{ 0x3fff,0x28a4 },				/* 2nd mask/value in sequence */
	{ 0x0784,0x0080 },				/* 3rd mask/value in sequence */
	{ 0x3fcf,0x0042 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x00a1 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x00a2 },				/* 2nd mask/value in sequence */
	{ 0x3c4f,0x284d },				/* +1 mask/value */
	{ 0x3a5f,0x285d },				/* +2 mask/value */
	{ 0x3ff8,0x2800 }				/* final mask/value in sequence */
};


/* slapstic 137412-112: Pit Fighter (Japan) (confirmed) */
static const struct slapstic_data slapstic112 =
{
	/* basic banking */
	0,						/* starting bank */
	{ 0x002c,0x003c,0x006c,0x007c },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0014 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x29a0 },				/* 2nd mask/value in sequence */
	{ 0x0073,0x0010 },				/* 3rd mask/value in sequence */
	{ 0x3faf,0x002c },				/* 4th mask/value in sequence */
	2,						/* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x2dce },				/* 1st mask/value in sequence */
	{ 0x3fff,0x2dcf },				/* 2nd mask/value in sequence */
	{ 0x3def,0x15e2 },				/* +1 mask/value */
	{ 0x3fbf,0x15a2 },				/* +2 mask/value */
	{ 0x3ffc,0x1450 }				/* final mask/value in sequence */
};


/* slapstic 137412-113: Unknown (Europe) (confirmed) */
static const struct slapstic_data slapstic113 =
{
	/* basic banking */
	0,						/* starting bank */
	{ 0x0008,0x0018,0x0028,0x0038 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0059 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x11a5 },				/* 2nd mask/value in sequence */
	{ 0x0860,0x0800 },				/* 3rd mask/value in sequence */
	{ 0x3fcf,0x0008 },				/* 4th mask/value in sequence */
	3,						/* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x049b },				/* 1st mask/value in sequence */
	{ 0x3fff,0x049c },				/* 2nd mask/value in sequence */
	{ 0x3fcf,0x3ec7 },				/* +1 mask/value */
	{ 0x3edf,0x3ed7 },				/* +2 mask/value */
	{ 0x3fff,0x3fb2 }				/* final mask/value in sequence */
};


/* slapstic 137412-114: Pit Fighter (rev 9) (confirmed) */
static const struct slapstic_data slapstic114 =
{
	/* basic banking */
	0,						/* starting bank */
	{ 0x0040,0x0048,0x0050,0x0058 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0016 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x24de },				/* 2nd mask/value in sequence */
	{ 0x3871,0x0000 },				/* 3rd mask/value in sequence */
	{ 0x3fe7,0x0040 },				/* 4th mask/value in sequence */
	1,						/* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x0ab7 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x0ab8 },				/* 2nd mask/value in sequence */
	{ 0x3f63,0x0d40 },				/* +1 mask/value */
	{ 0x3fd9,0x0dc8 },				/* +2 mask/value */
	{ 0x3fff,0x0ab0 }				/* final mask/value in sequence */
};


/* slapstic 137412-115: Race Drivin' DSK board (confirmed) */
static const struct slapstic_data slapstic115 =
{
	/* basic banking */
	0,						/* starting bank */
	{ 0x0020,0x0022,0x0024,0x0026 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0054 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x3e01 },				/* 2nd mask/value in sequence */
	{ 0x3879,0x0029 },				/* 3rd mask/value in sequence */
	{ 0x3ff9,0x0020 },				/* 4th mask/value in sequence */
	1,						/* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x2591 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x2592 },				/* 2nd mask/value in sequence */
	{ 0x3fe6,0x3402 },				/* +1 mask/value */
	{ 0x3fb4,0x3410 },				/* +2 mask/value */
	{ 0x3fff,0x34a2 }				/* final mask/value in sequence */
};


/* slapstic 137412-116: Hydra (confirmed) */
static const struct slapstic_data slapstic116 =
{
	/* basic banking */
	0,						/* starting bank */
	{ 0x0044,0x004c,0x0054,0x005c },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0069 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x2bab },				/* 2nd mask/value in sequence */
	{ 0x387c,0x0808 },				/* 3rd mask/value in sequence */
	{ 0x3fe7,0x0044 },				/* 4th mask/value in sequence */
	0,						/* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x3f7c },				/* 1st mask/value in sequence */
	{ 0x3fff,0x3f7d },				/* 2nd mask/value in sequence */
	{ 0x3db2,0x3c12 },				/* +1 mask/value */
	{ 0x3fe3,0x3e43 },				/* +2 mask/value */
	{ 0x3fff,0x2ba8 }				/* final mask/value in sequence */
};


/* slapstic 137412-117: Race Drivin' main board (confirmed) */
static const struct slapstic_data slapstic117 =
{
	/* basic banking */
	0,						/* starting bank */
	{ 0x0008,0x001a,0x002c,0x003e },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x007d },				/* 1st mask/value in sequence */
	{ 0x3fff,0x3580 },				/* 2nd mask/value in sequence */
	{ 0x0079,0x0020 },				/* 3rd mask/value in sequence */
	{ 0x3fc9,0x0008 },				/* 4th mask/value in sequence */
	1,						/* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x0676 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x0677 },				/* 2nd mask/value in sequence */
	{ 0x3e62,0x1a42 },				/* +1 mask/value */
	{ 0x3e35,0x1a11 },				/* +2 mask/value */
	{ 0x3fff,0x1a42 }				/* final mask/value in sequence */
};


/* slapstic 137412-118: Rampart/Vindicators II (confirmed) */
static const struct slapstic_data slapstic118 =
{
	/* basic banking */
	0,						/* starting bank */
	{ 0x0014,0x0034,0x0054,0x0074 },		/* bank select values */

	/* alternate banking */
	{ 0x007f,0x0002 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x1950 },				/* 2nd mask/value in sequence */
	{ 0x0067,0x0020 },				/* 3rd mask/value in sequence */
	{ 0x3f9f,0x0014 },				/* 4th mask/value in sequence */
	3,						/* shift to get bank from 3rd */

	/* bitwise banking */
	NO_BITWISE,

	/* additive banking */
	{ 0x3fff,0x1958 },				/* 1st mask/value in sequence */
	{ 0x3fff,0x1959 },				/* 2nd mask/value in sequence */
	{ 0x3f73,0x3052 },				/* +1 mask/value */
	{ 0x3f67,0x3042 },				/* +2 mask/value */
	{ 0x3ff8,0x30e0 }				/* final mask/value in sequence */
};


/*************************************
 *
 *  Master slapstic table
 *
 *************************************/

/* master table */
static const struct slapstic_data *const slapstic_table[] =
{
	&slapstic101,					/* NOT confirmed! */
	NULL,						/* never seen */
	&slapstic103,
	&slapstic104,
	&slapstic105,
	&slapstic106,
	&slapstic107,
	&slapstic108,
	&slapstic109,
	&slapstic110,
	&slapstic111,
	&slapstic112,
	&slapstic113,
	&slapstic114,
	&slapstic115,
	&slapstic116,
	&slapstic117,
	&slapstic118
};


/*************************************
 *
 *  Statics
 *
 *************************************/

static UINT8 state;
static UINT8 current_bank;
static UINT8 access_68k;

static UINT8 alt_bank;
static UINT8 bit_bank;
static UINT8 add_bank;
static UINT8 bit_xor;

static struct slapstic_data slapstic;


/*************************************
 *
 *  Initialization
 *
 *************************************/

void SlapsticReset(void)
{
	/* reset the chip */
	state = DISABLED;

	/* the 111 and later chips seem to reset to bank 0 */
	current_bank = slapstic.bankstart;
}


void SlapsticInit(INT32 chip)
{
	/* only a small number of chips are known to exist */
	if (chip < 101 || chip > 118)
		return;

	/* set up the parameters */
	if (!slapstic_table[chip - 101])
		return;

	slapstic = *slapstic_table[chip - 101];

	/* reset the chip */
	SlapsticReset();

	/* see if we're 68k or 6502/6809 based */
	extern INT32 nSekCount;
	access_68k = (nSekCount != -1); // Ok?
}


/*************************************
 *
 *  Returns active bank without tweaking
 *
 *************************************/

INT32 SlapsticBank(void)
{
	return current_bank;
}


/*************************************
 *
 *  Kludge to catch alt seqeuences
 *
 *************************************/

static INT32 alt2_kludge(UINT32 /*offset*/)
{
	/* Of the 3 alternate addresses, only the middle one needs to actually hit
       in the slapstic region; the first and third ones can be anywhere in the
       address space. For this reason, the read/write handlers usually only
       see the 2nd access. For the 68000-based games, we do the following
       kludge to examine the opcode that is executing and look for the 1st
       and 3rd accesses. */
	if (access_68k)
	{
		static const SekRegister SekRegs[8] = { SEK_REG_A0, SEK_REG_A1, SEK_REG_A2, SEK_REG_A3, SEK_REG_A4, SEK_REG_A5, SEK_REG_A6, SEK_REG_A7 };

		/* first verify that the prefetched PC matches the first alternate */
		if (MATCHES_MASK_VALUE(SekGetPC(-1) >> 1, slapstic.alt1))
		{
			/* now look for a move.w (An),(An) or cmpm.w (An)+,(An)+ */
			UINT16 opcode = SekFetchWord((SekGetPC(-1) - 4) & 0xffffff); // IQ_132 check this!
		//	UINT16 opcode = space->direct().read_decrypted_word(cpu_get_previouspc(&space->device()) & 0xffffff);
			if ((opcode & 0xf1f8) == 0x3090 || (opcode & 0xf1f8) == 0xb148)
			{
				/* fetch the value of the register for the second operand, and see */
				/* if it matches the third alternate */
				UINT32 regval = SekDbgGetRegister(SekRegs[((opcode >> 9) & 7)]) >> 1;
				if (MATCHES_MASK_VALUE(regval, slapstic.alt3))
				{
					alt_bank = (regval >> slapstic.altshift) & 3;
					return ALTERNATE3;
				}
			}
		}

		/* if there's no second memory hit within this instruction, the next */
		/* opcode fetch will botch the operation, so just fall back to */
		/* the enabled state */
		return ENABLED;
	}

	/* kludge for ESB */
	return ALTERNATE2;
}


/*************************************
 *
 *  Call this *after* every access
 *
 *************************************/

INT32 SlapsticTweak(INT32 offset)
{
	/* reset is universal */
	if (offset == 0x0000)
	{
		state = ENABLED;
	}

	/* otherwise, use the state machine */
	else
	{
		switch (state)
		{
			/* DISABLED state: everything is ignored except a reset */
			case DISABLED:
				break;

			/* ENABLED state: the chip has been activated and is ready for a bankswitch */
			case ENABLED:

				/* check for request to enter bitwise state */
				if (MATCHES_MASK_VALUE(offset, slapstic.bit1))
				{
					state = BITWISE1;
				}

				/* check for request to enter additive state */
				else if (MATCHES_MASK_VALUE(offset, slapstic.add1))
				{
					state = ADDITIVE1;
				}

				/* check for request to enter alternate state */
				else if (MATCHES_MASK_VALUE(offset, slapstic.alt1))
				{
					state = ALTERNATE1;
				}

				/* special kludge for catching the second alternate address if */
				/* the first one was missed (since it's usually an opcode fetch) */
				else if (MATCHES_MASK_VALUE(offset, slapstic.alt2))
				{
					state = alt2_kludge(offset);
				}

				/* check for standard bankswitches */
				else if (offset == slapstic.bank[0])
				{
					state = DISABLED;
					current_bank = 0;
				}
				else if (offset == slapstic.bank[1])
				{
					state = DISABLED;
					current_bank = 1;
				}
				else if (offset == slapstic.bank[2])
				{
					state = DISABLED;
					current_bank = 2;
				}
				else if (offset == slapstic.bank[3])
				{
					state = DISABLED;
					current_bank = 3;
				}
				break;

			/* ALTERNATE1 state: look for alternate2 offset, or else fall back to ENABLED */
			case ALTERNATE1:
				if (MATCHES_MASK_VALUE(offset, slapstic.alt2))
				{
					state = ALTERNATE2;
				}
				else
				{
					state = ENABLED;
				}
				break;

			/* ALTERNATE2 state: look for altbank offset, or else fall back to ENABLED */
			case ALTERNATE2:
				if (MATCHES_MASK_VALUE(offset, slapstic.alt3))
				{
					state = ALTERNATE3;
					alt_bank = (offset >> slapstic.altshift) & 3;
				}
				else
				{
					state = ENABLED;
				}
				break;

			/* ALTERNATE3 state: wait for the final value to finish the transaction */
			case ALTERNATE3:
				if (MATCHES_MASK_VALUE(offset, slapstic.alt4))
				{
					state = DISABLED;
					current_bank = alt_bank;
				}
				break;

			/* BITWISE1 state: waiting for a bank to enter the BITWISE state */
			case BITWISE1:
				if (offset == slapstic.bank[0] || offset == slapstic.bank[1] ||
					offset == slapstic.bank[2] || offset == slapstic.bank[3])
				{
					state = BITWISE2;
					bit_bank = current_bank;
					bit_xor = 0;
				}
				break;

			/* BITWISE2 state: watch for twiddling and the escape mechanism */
			case BITWISE2:

				/* check for clear bit 0 case */
				if (MATCHES_MASK_VALUE(offset ^ bit_xor, slapstic.bit2c0))
				{
					bit_bank &= ~1;
					bit_xor ^= 3;
				}

				/* check for set bit 0 case */
				else if (MATCHES_MASK_VALUE(offset ^ bit_xor, slapstic.bit2s0))
				{
					bit_bank |= 1;
					bit_xor ^= 3;
				}

				/* check for clear bit 1 case */
				else if (MATCHES_MASK_VALUE(offset ^ bit_xor, slapstic.bit2c1))
				{
					bit_bank &= ~2;
					bit_xor ^= 3;
				}

				/* check for set bit 1 case */
				else if (MATCHES_MASK_VALUE(offset ^ bit_xor, slapstic.bit2s1))
				{
					bit_bank |= 2;
					bit_xor ^= 3;
				}

				/* check for escape case */
				else if (MATCHES_MASK_VALUE(offset, slapstic.bit3))
				{
					state = BITWISE3;
				}
				break;

			/* BITWISE3 state: waiting for a bank to seal the deal */
			case BITWISE3:
				if (offset == slapstic.bank[0] || offset == slapstic.bank[1] ||
					offset == slapstic.bank[2] || offset == slapstic.bank[3])
				{
					state = DISABLED;
					current_bank = bit_bank;
				}
				break;

			/* ADDITIVE1 state: look for add2 offset, or else fall back to ENABLED */
			case ADDITIVE1:
				if (MATCHES_MASK_VALUE(offset, slapstic.add2))
				{
					state = ADDITIVE2;
					add_bank = current_bank;
				}
				else
				{
					state = ENABLED;
				}
				break;

			/* ADDITIVE2 state: watch for twiddling and the escape mechanism */
			case ADDITIVE2:

				/* check for add 1 case -- can intermix */
				if (MATCHES_MASK_VALUE(offset, slapstic.addplus1))
				{
					add_bank = (add_bank + 1) & 3;
				}

				/* check for add 2 case -- can intermix */
				if (MATCHES_MASK_VALUE(offset, slapstic.addplus2))
				{
					add_bank = (add_bank + 2) & 3;
				}

				/* check for escape case -- can intermix with the above */
				if (MATCHES_MASK_VALUE(offset, slapstic.add3))
				{
					state = ADDITIVE3;
				}
				break;

			/* ADDITIVE3 state: waiting for a bank to seal the deal */
			case ADDITIVE3:
				if (offset == slapstic.bank[0] || offset == slapstic.bank[1] ||
					offset == slapstic.bank[2] || offset == slapstic.bank[3])
				{
					state = DISABLED;
					current_bank = add_bank;
				}
				break;
		}
	}

	/* return the active bank */
	return current_bank;
}


void SlapsticScan(INT32 nAction)
{
	if (nAction & ACB_NVRAM) {
		SCAN_VAR(state);
		SCAN_VAR(current_bank);
		SCAN_VAR(alt_bank);
		SCAN_VAR(bit_bank);
		SCAN_VAR(add_bank);
		SCAN_VAR(bit_xor);
	}
}
