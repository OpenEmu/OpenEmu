/*
 *  Mii.h
 *  WiiRemoteFramework
 *
 *  Created by Mike Cohen on 3/6/07.
 *  Copyright 2007 MC Development. All rights reserved.
 *
 */

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;

// MII Data Defines
#define WIIMOTE_MII_DATA_BEGIN_ADDR			0x0FCA

#define WIIMOTE_MII_DATA_BEGIN_1			0x0FD2
#define WIIMOTE_MII_DATA_BYTES_PER_SLOT		74
#define WIIMOTE_MII_SLOT_NUM				10

#define WIIMOTE_MII_SECTION1_BEGIN_ADDR		0x0FCA
#define WIIMOTE_MII_SECTION2_BEGIN_ADDR		0x12BA
#define WIIMOTE_MII_SECTION_SIZE			750		// Size in bytes

#define WIIMOTE_MII_CHECKSUM1_ADDR			0x12B8
#define WIIMOTE_MII_CHECKSUM2_ADDR			0x15A8
#define WIIMOTE_MII_CHECKSUM_SIZE			0x0002	// Size in bytes

#define WIIMOTE_MII_PARADESLOTS_ADDR		0x0FCE
#define WIIMOTE_MII_PARADESLOTS_SIZE		0x0002

// MII CRC16 Defines
#define WIIMOTE_MII_CRC16_POLY			0x1021
#define WIIMOTE_MII_CRC16_INITIAL		0xFFFF
#define WIIMOTE_MII_CRC16_POSTXOR		0xEF4C

// MII Data Structure Defines
#define MII_NAME_LENGTH				10
#define MII_CREATOR_NAME_LENGTH		10

#define MII_HEIGHT_MIN				0x00
#define MII_HEIGHT_MAX				0x7F

#define MII_WEIGHT_MIN				0x00
#define MII_WEIGHT_MAX				0x7F

#define MII_DATA_SIZE			WIIMOTE_MII_DATA_BYTES_PER_SLOT

#define MII_OFFSET(slot)		(WIIMOTE_MII_DATA_BEGIN_1 + (slot * WIIMOTE_MII_DATA_BYTES_PER_SLOT))
#define MII_SLOT(addr)			((addr - WIIMOTE_MII_DATA_BEGIN_1) / WIIMOTE_MII_DATA_BYTES_PER_SLOT)
#define MII_SEGMENT(addr)		(addr - (MII_SLOT(addr) * WIIMOTE_MII_DATA_BYTES_PER_SLOT)

typedef struct
{
	// addr: 0x00 & 0x01
	u16 invalid:1;            // doesn't seem to have any effect?
	u16 isGirl:1;
	u16 month:4;
	u16 day:5;
	u16 favColor:4;		   // 0 - 11 (changing to 1111, along with setting the preceeding bit 
              		           // results in a grey shirt, some values over 11 will crash the Wii 
							   // when trying to change the favorite color).
	u16 unknown1:1;
	
	// addr: 0x02 through 0x15
	u16 name[MII_NAME_LENGTH];
	
	// addr: 0x16
	u8 height;
	
	// addr: 0x17
	u8 weight;
	
	// addr: 0x18 - 0x1B
 	u8 miiID1; 	           // Unique Mii identifier. Seems to increment with time. Also can
 	u8 miiID2; 	           // be used to change colour of Mii Trousers (see 'See Also' links)
 	u8 miiID3;
 	u8 miiID4;
	
	// addr: 0x1C & 0x1D & 0x1E & 0x1F
	u8 systemID0;	           // Checksum8 of first 3 bytes of mac addr
	u8 systemID1;	           // mac addr 3rd-to-last byte
	u8 systemID2;	           // mac addr 2nd-to-last byte
	u8 systemID3;	           // mac addr last byte
	
	// addr: 0x20 & 0x21
	u16 faceShape:3;           // 0 - 7
	u16 skinColor:3;           // 0 - 5
	u16 facialFeature:4;       // 0 - 11
	u16 unknown2:3;             // Mii appears unaffected by changes to this data 
	u16 mingleOff:1;           // 0 = Mingle, 1 = Don't Mingle
	u16 unknown3:2;             // Mii appears unaffected by changes to this data
	
	// addr: 0x22 & 0x23
	u16 hairType:7;            // 0 - 71, Value is non-sequential with regard to page, row and column
	u16 hairColor:3;           // 0 - 7
	u16 hairPart:1;            // 0 = Normal, 1 = Reversed
	u16 unknown4:5;
	
	// addr: 0x24 & 0x25 & 0x26 & 0x27
	u32 eyebrowType:5;         // 0 - 23, Value is non-sequential with regard to page, row and column
	u32 unknown5:1;
	u32 eyebrowRotation:4;     // 0 - 11, Default value varies based on eyebrow type
	u32 unknown6:6;
	u32 eyebrowColor:3;        // 0 - 7
	u32 eyebrowSize:4;	   // 0 - 8, Default = 4
	u32 eyebrowVertPos:5;      // 3 - 18, Default = 10
	u32 eyebrowHorizSpacing:4; // 0 - 12, Default = 2
	
	// addr: 0x28 & 0x29 & 0x2A & 0x2B
	u32 eyeType:6;             // 0 - 47, Value is non-sequential with regard to page, row and column
	u32 unknown7:2;
	u32 eyeRotation:3;         // 0 - 7, Default value varies based on eye type
	u32 eyeVertPos:5;          // 0 - 18, Default = 12
	u32 eyeColor:3;            // 0 - 5
	u32 unknown8:1;
	u32 eyeSize:3;             // 0 - 7, Default = 4
	u32 eyeHorizSpacing:4;     // 0 - 12, Default = 2
	u32 unknown9:5;
	
	// addr: 0x2C & 0x2D
	u16 noseType:4;            // 0 - 11, Value is non-sequential with regard to row and column
	u16 noseSize:4;            // 0 - 8, Default = 4
	u16 noseVertPos:5;         // 0 - 18, Default = 9
	u16 unknown10:3;
	
	// addr: 0x2E & 2F
	u16 lipType:5;             // 0 - 23, Value is non-sequential with regard to page, row and column
	u16 lipColor:2;            // 0 - 2
	u16 lipSize:4;             // 0 - 8, Default = 4
	u16 lipVertPos:5;          // 0 - 18, Default = 13
	
	// addr: 0x30 & 0x31
	u16 glassesType:4;         // 0 - 8
	u16 glassesColor:3;        // 0 - 5
	u16 unknown11:1;             // when turned on mii does not appear (use not known)
	u16 glassesSize:3;         // 0 - 7, Default = 4
	u16 glassesVertPos:5;      // 0 - 20, Default = 10
	
	// addr: 0x32 & 33
	u16 mustacheType:2;        // 0 - 3
	u16 beardType:2;           // 0 - 3
	u16 facialHairColor:3;     // 0 - 7
	u16 mustacheSize:4;        // 0 - 8, Default = 4
	u16 mustacheVertPos:4;     // 0 - 16, Default = 10
	
	// addr: 0x34 & 0x35
	u16 moleOn:1;              // 0 = No Mole, 1 = Has Mole
	u16 moleSize:4;            // 0 - 8, Default = 4
	u16 moleVertPos:5;         // 0 - 30, Default = 20
	u16 moleHorizPos:5;        // 0 - 16, Default = 2
	u16 unknown12:1;             
	
	// addr: 0x36
	u16 creatorName[MII_CREATOR_NAME_LENGTH];
} Mii;
