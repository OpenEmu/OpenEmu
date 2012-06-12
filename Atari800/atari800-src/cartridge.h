#ifndef CARTRIDGE_H_
#define CARTRIDGE_H_

#include "config.h"
#include "atari.h"

#define CARTRIDGE_NONE            0
#define CARTRIDGE_STD_8           1
#define CARTRIDGE_STD_16          2
#define CARTRIDGE_OSS_16          3
#define CARTRIDGE_5200_32         4
#define CARTRIDGE_DB_32           5
#define CARTRIDGE_5200_EE_16      6
#define CARTRIDGE_5200_40         7
#define CARTRIDGE_WILL_64         8
#define CARTRIDGE_EXP_64          9
#define CARTRIDGE_DIAMOND_64     10
#define CARTRIDGE_SDX_64         11
#define CARTRIDGE_XEGS_32        12
#define CARTRIDGE_XEGS_64        13
#define CARTRIDGE_XEGS_128       14
#define CARTRIDGE_OSS2_16        15
#define CARTRIDGE_5200_NS_16     16
#define CARTRIDGE_ATRAX_128      17
#define CARTRIDGE_BBSB_40        18
#define CARTRIDGE_5200_8         19
#define CARTRIDGE_5200_4         20
#define CARTRIDGE_RIGHT_8        21
#define CARTRIDGE_WILL_32        22
#define CARTRIDGE_XEGS_256       23
#define CARTRIDGE_XEGS_512       24
#define CARTRIDGE_XEGS_1024      25
#define CARTRIDGE_MEGA_16        26
#define CARTRIDGE_MEGA_32        27
#define CARTRIDGE_MEGA_64        28
#define CARTRIDGE_MEGA_128       29
#define CARTRIDGE_MEGA_256       30
#define CARTRIDGE_MEGA_512       31
#define CARTRIDGE_MEGA_1024      32
#define CARTRIDGE_SWXEGS_32      33
#define CARTRIDGE_SWXEGS_64      34
#define CARTRIDGE_SWXEGS_128     35
#define CARTRIDGE_SWXEGS_256     36
#define CARTRIDGE_SWXEGS_512     37
#define CARTRIDGE_SWXEGS_1024    38
#define CARTRIDGE_PHOENIX_8      39
#define CARTRIDGE_BLIZZARD_16    40
#define CARTRIDGE_ATMAX_128      41
#define CARTRIDGE_ATMAX_1024     42
#define CARTRIDGE_SDX_128        43
#define CARTRIDGE_LAST_SUPPORTED 43

#define CARTRIDGE_MAX_SIZE	(1024 * 1024)
extern int CARTRIDGE_kb[CARTRIDGE_LAST_SUPPORTED + 1];
extern int CARTRIDGE_type;
extern int CARTRIDGE_second_type;

int CARTRIDGE_IsFor5200(int type);
int CARTRIDGE_Checksum(const UBYTE *image, int nbytes);

#define CARTRIDGE_CANT_OPEN		-1	/* Can't open cartridge image file */
#define CARTRIDGE_BAD_FORMAT		-2	/* Unknown cartridge format */
#define CARTRIDGE_BAD_CHECKSUM	-3	/* Warning: bad CART checksum */
int CARTRIDGE_Insert(const char *filename);
int CARTRIDGE_Insert_Second(const char *filename);

void CARTRIDGE_Remove(void);
void CARTRIDGE_Remove_Second(void);

void CARTRIDGE_Start(void);
UBYTE CARTRIDGE_GetByte(UWORD addr);
void CARTRIDGE_PutByte(UWORD addr, UBYTE byte);
void CARTRIDGE_BountyBob1(UWORD addr);
void CARTRIDGE_BountyBob2(UWORD addr);
void CARTRIDGE_StateSave(void);
void CARTRIDGE_StateRead(void);
#ifdef PAGED_ATTRIB
UBYTE CARTRIDGE_BountyBob1GetByte(UWORD addr);
UBYTE CARTRIDGE_BountyBob2GetByte(UWORD addr);
void CARTRIDGE_BountyBob1PutByte(UWORD addr, UBYTE value);
void CARTRIDGE_BountyBob2PutByte(UWORD addr, UBYTE value);
#endif

#endif /* CARTRIDGE_H_ */
