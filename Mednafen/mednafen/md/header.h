#ifndef __MDFN_MD_HEADER_H
#define __MDFN_MD_HEADER_H

namespace MDFN_IEN_MD
{

enum
{
 REGIONMASK_JAPAN_NTSC = 1,
 REGIONMASK_JAPAN_PAL = 2,
 REGIONMASK_OVERSEAS_NTSC = 4,
 REGIONMASK_OVERSEAS_PAL = 8
};

enum
{
 IOS_GAMEPAD = 0,
 IOS_GAMEPAD6B,
 IOS_KEYBOARD,
 IOS_PRINTER,
 IOS_BALL,
 IOS_FLOPPY,
 IOS_ACTIVATOR,
 IOS_TEAM_PLAY,
 IOS_GAMEPAD_MS,
 IOS_RS232C,
 IOS_TABLET,
 IOS_PADDLE,
 IOS_CDROM,
 IOS_MEGA_MOUSE
};

typedef struct
{
 const int id;
 const char code_char;
 const char *name;
} IO_type_t;

static const IO_type_t IO_types[] =
{
 { IOS_GAMEPAD, 'J', "3-Button Gamepad" },
 { IOS_GAMEPAD6B, '6', "6-Button Gamepad" },
 { IOS_KEYBOARD, 'K', "Keyboard" },
 { IOS_PRINTER, 'P', "Printer" },
 { IOS_BALL, 'B', "Ball Controller" },
 { IOS_FLOPPY, 'F', "Floppy disk drive" },
 { IOS_ACTIVATOR, 'L', "Activator" },
 { IOS_TEAM_PLAY, '4', "Team Play" },
 { IOS_GAMEPAD_MS, '0', "2-Button Master System Gamepad" },
 { IOS_RS232C, 'R', "RS232C Serial Port" },
 { IOS_TABLET, 'T', "Tablet" },
 { IOS_PADDLE, 'V', "Paddle Controller" },
 { IOS_CDROM, 'C', "CD-ROM" },
 { IOS_MEGA_MOUSE, 'M', "Mega Mouse" },
};

typedef struct
{
	uint32 rom_size;

	uint32 io_support;
	char copyright[0x10 + 1];
	char domestic_name[0x30 + 1];
	char overseas_name[0x30 + 1];
	char product_code[0xC + 1];
	uint16 checksum; // Recorded game checksum
	uint8 region_support;

	uint32 sram_type;
	uint32 sram_start;
	uint32 sram_end;

        // Convenience, not set by the header parsing code:
        uint8 md5[16];
        uint8 info_header_md5[16];
	uint16 checksum_real;
} md_game_info;

void MD_ReadSegaHeader(const uint8 *header, md_game_info *ginfo);

}

#endif
