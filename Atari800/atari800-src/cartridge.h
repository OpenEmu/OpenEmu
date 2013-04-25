#ifndef CARTRIDGE_H_
#define CARTRIDGE_H_

#include "config.h"
#include "atari.h"

enum { CARTRIDGE_UNKNOWN        = -1,
       CARTRIDGE_NONE           =  0,
       CARTRIDGE_STD_8          =  1,
       CARTRIDGE_STD_16         =  2,
       CARTRIDGE_OSS_034M_16    =  3,
       CARTRIDGE_5200_32        =  4,
       CARTRIDGE_DB_32          =  5,
       CARTRIDGE_5200_EE_16     =  6,
       CARTRIDGE_5200_40        =  7,
       CARTRIDGE_WILL_64        =  8,
       CARTRIDGE_EXP_64         =  9,
       CARTRIDGE_DIAMOND_64     = 10,
       CARTRIDGE_SDX_64         = 11,
       CARTRIDGE_XEGS_32        = 12,
       CARTRIDGE_XEGS_64        = 13,
       CARTRIDGE_XEGS_128       = 14,
       CARTRIDGE_OSS_M091_16    = 15,
       CARTRIDGE_5200_NS_16     = 16,
       CARTRIDGE_ATRAX_128      = 17,
       CARTRIDGE_BBSB_40        = 18,
       CARTRIDGE_5200_8         = 19,
       CARTRIDGE_5200_4         = 20,
       CARTRIDGE_RIGHT_8        = 21,
       CARTRIDGE_WILL_32        = 22,
       CARTRIDGE_XEGS_256       = 23,
       CARTRIDGE_XEGS_512       = 24,
       CARTRIDGE_XEGS_1024      = 25,
       CARTRIDGE_MEGA_16        = 26,
       CARTRIDGE_MEGA_32        = 27,
       CARTRIDGE_MEGA_64        = 28,
       CARTRIDGE_MEGA_128       = 29,
       CARTRIDGE_MEGA_256       = 30,
       CARTRIDGE_MEGA_512       = 31,
       CARTRIDGE_MEGA_1024      = 32,
       CARTRIDGE_SWXEGS_32      = 33,
       CARTRIDGE_SWXEGS_64      = 34,
       CARTRIDGE_SWXEGS_128     = 35,
       CARTRIDGE_SWXEGS_256     = 36,
       CARTRIDGE_SWXEGS_512     = 37,
       CARTRIDGE_SWXEGS_1024    = 38,
       CARTRIDGE_PHOENIX_8      = 39,
       CARTRIDGE_BLIZZARD_16    = 40,
       CARTRIDGE_ATMAX_128      = 41,
       CARTRIDGE_ATMAX_1024     = 42,
       CARTRIDGE_SDX_128        = 43,
       CARTRIDGE_OSS_8          = 44,
       CARTRIDGE_OSS_043M_16    = 45,
       CARTRIDGE_BLIZZARD_4     = 46,
       CARTRIDGE_AST_32         = 47,
       CARTRIDGE_ATRAX_SDX_64   = 48,
       CARTRIDGE_ATRAX_SDX_128  = 49,
       CARTRIDGE_TURBOSOFT_64   = 50,
       CARTRIDGE_TURBOSOFT_128  = 51,
       CARTRIDGE_ULTRACART_32   = 52,
       CARTRIDGE_LOW_BANK_8     = 53,
       CARTRIDGE_SIC_128        = 54,
       CARTRIDGE_SIC_256        = 55,
       CARTRIDGE_SIC_512        = 56,
       CARTRIDGE_STD_2          = 57,
       CARTRIDGE_STD_4          = 58,
       CARTRIDGE_RIGHT_4        = 59,
       CARTRIDGE_LAST_SUPPORTED = 59
};

#define CARTRIDGE_MAX_SIZE	(1024 * 1024)
extern int CARTRIDGE_kb[CARTRIDGE_LAST_SUPPORTED + 1];
extern char *CARTRIDGE_TextDesc[CARTRIDGE_LAST_SUPPORTED + 1];

/* Indicates whether the emulator should automatically reboot (coldstart)
   after inserting/removing a cartridge. (Doesn't affect the piggyback
   cartridge - in this case system will never autoreboot.) */
extern int CARTRIDGE_autoreboot;

typedef struct CARTRIDGE_image_t {
	int type;
	int state; /* Cartridge's state, such as selected bank or switch on/off. */
	int size; /* Size of the image, in kilobytes */
	UBYTE *image;
	char filename[FILENAME_MAX];
} CARTRIDGE_image_t;

extern CARTRIDGE_image_t CARTRIDGE_main;
extern CARTRIDGE_image_t CARTRIDGE_piggyback;

int CARTRIDGE_Checksum(const UBYTE *image, int nbytes);

int CARTRIDGE_ReadConfig(char *string, char *ptr);
void CARTRIDGE_WriteConfig(FILE *fp);
int CARTRIDGE_Initialise(int *argc, char *argv[]);
void CARTRIDGE_Exit(void);

#define CARTRIDGE_CANT_OPEN		-1	/* Can't open cartridge image file */
#define CARTRIDGE_BAD_FORMAT		-2	/* Unknown cartridge format */
#define CARTRIDGE_BAD_CHECKSUM	-3	/* Warning: bad CART checksum */
/* Inserts the left cartrifge. */
int CARTRIDGE_Insert(const char *filename);
/* Inserts the left cartridge and reboots the system if needed. */
int CARTRIDGE_InsertAutoReboot(const char *filename);
/* Inserts the piggyback cartridge. */
int CARTRIDGE_Insert_Second(const char *filename);
/* When the cartridge type is CARTRIDGE_UNKNOWN after a call to
   CARTRIDGE_Insert(), this function should be called to set the
   cartridge's type manually to a value chosen by user. */
void CARTRIDGE_SetType(CARTRIDGE_image_t *cart, int type);
/* Sets type of the cartridge and reboots the system if needed. */
void CARTRIDGE_SetTypeAutoReboot(CARTRIDGE_image_t *cart, int type);

/* Removes the left cartridge. */
void CARTRIDGE_Remove(void);
/* Removes the left cartridge and reboots the system if needed. */
void CARTRIDGE_RemoveAutoReboot(void);
/* Removed the piggyback cartridge. */
void CARTRIDGE_Remove_Second(void);

/* Called on system coldstart. Resets the states of mounted cartridges. */
void CARTRIDGE_ColdStart(void);

UBYTE CARTRIDGE_GetByte(UWORD addr, int no_side_effects);
void CARTRIDGE_PutByte(UWORD addr, UBYTE byte);
void CARTRIDGE_BountyBob1(UWORD addr);
void CARTRIDGE_BountyBob2(UWORD addr);
void CARTRIDGE_StateSave(void);
void CARTRIDGE_StateRead(UBYTE version);
#ifdef PAGED_ATTRIB
UBYTE CARTRIDGE_BountyBob1GetByte(UWORD addr, int no_side_effects);
UBYTE CARTRIDGE_BountyBob2GetByte(UWORD addr, int no_side_effects);
void CARTRIDGE_BountyBob1PutByte(UWORD addr, UBYTE value);
void CARTRIDGE_BountyBob2PutByte(UWORD addr, UBYTE value);
#endif

#endif /* CARTRIDGE_H_ */
