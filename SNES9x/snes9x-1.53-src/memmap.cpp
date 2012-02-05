/***********************************************************************************
  Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.

  (c) Copyright 1996 - 2002  Gary Henderson (gary.henderson@ntlworld.com),
                             Jerremy Koot (jkoot@snes9x.com)

  (c) Copyright 2002 - 2004  Matthew Kendora

  (c) Copyright 2002 - 2005  Peter Bortas (peter@bortas.org)

  (c) Copyright 2004 - 2005  Joel Yliluoma (http://iki.fi/bisqwit/)

  (c) Copyright 2001 - 2006  John Weidman (jweidman@slip.net)

  (c) Copyright 2002 - 2006  funkyass (funkyass@spam.shaw.ca),
                             Kris Bleakley (codeviolation@hotmail.com)

  (c) Copyright 2002 - 2010  Brad Jorsch (anomie@users.sourceforge.net),
                             Nach (n-a-c-h@users.sourceforge.net),

  (c) Copyright 2002 - 2011  zones (kasumitokoduck@yahoo.com)

  (c) Copyright 2006 - 2007  nitsuja

  (c) Copyright 2009 - 2011  BearOso,
                             OV2


  BS-X C emulator code
  (c) Copyright 2005 - 2006  Dreamer Nom,
                             zones

  C4 x86 assembler and some C emulation code
  (c) Copyright 2000 - 2003  _Demo_ (_demo_@zsnes.com),
                             Nach,
                             zsKnight (zsknight@zsnes.com)

  C4 C++ code
  (c) Copyright 2003 - 2006  Brad Jorsch,
                             Nach

  DSP-1 emulator code
  (c) Copyright 1998 - 2006  _Demo_,
                             Andreas Naive (andreasnaive@gmail.com),
                             Gary Henderson,
                             Ivar (ivar@snes9x.com),
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora,
                             Nach,
                             neviksti (neviksti@hotmail.com)

  DSP-2 emulator code
  (c) Copyright 2003         John Weidman,
                             Kris Bleakley,
                             Lord Nightmare (lord_nightmare@users.sourceforge.net),
                             Matthew Kendora,
                             neviksti

  DSP-3 emulator code
  (c) Copyright 2003 - 2006  John Weidman,
                             Kris Bleakley,
                             Lancer,
                             z80 gaiden

  DSP-4 emulator code
  (c) Copyright 2004 - 2006  Dreamer Nom,
                             John Weidman,
                             Kris Bleakley,
                             Nach,
                             z80 gaiden

  OBC1 emulator code
  (c) Copyright 2001 - 2004  zsKnight,
                             pagefault (pagefault@zsnes.com),
                             Kris Bleakley
                             Ported from x86 assembler to C by sanmaiwashi

  SPC7110 and RTC C++ emulator code used in 1.39-1.51
  (c) Copyright 2002         Matthew Kendora with research by
                             zsKnight,
                             John Weidman,
                             Dark Force

  SPC7110 and RTC C++ emulator code used in 1.52+
  (c) Copyright 2009         byuu,
                             neviksti

  S-DD1 C emulator code
  (c) Copyright 2003         Brad Jorsch with research by
                             Andreas Naive,
                             John Weidman

  S-RTC C emulator code
  (c) Copyright 2001 - 2006  byuu,
                             John Weidman

  ST010 C++ emulator code
  (c) Copyright 2003         Feather,
                             John Weidman,
                             Kris Bleakley,
                             Matthew Kendora

  Super FX x86 assembler emulator code
  (c) Copyright 1998 - 2003  _Demo_,
                             pagefault,
                             zsKnight

  Super FX C emulator code
  (c) Copyright 1997 - 1999  Ivar,
                             Gary Henderson,
                             John Weidman

  Sound emulator code used in 1.5-1.51
  (c) Copyright 1998 - 2003  Brad Martin
  (c) Copyright 1998 - 2006  Charles Bilyue'

  Sound emulator code used in 1.52+
  (c) Copyright 2004 - 2007  Shay Green (gblargg@gmail.com)

  SH assembler code partly based on x86 assembler code
  (c) Copyright 2002 - 2004  Marcus Comstedt (marcus@mc.pp.se)

  2xSaI filter
  (c) Copyright 1999 - 2001  Derek Liauw Kie Fa

  HQ2x, HQ3x, HQ4x filters
  (c) Copyright 2003         Maxim Stepin (maxim@hiend3d.com)

  NTSC filter
  (c) Copyright 2006 - 2007  Shay Green

  GTK+ GUI code
  (c) Copyright 2004 - 2011  BearOso

  Win32 GUI code
  (c) Copyright 2003 - 2006  blip,
                             funkyass,
                             Matthew Kendora,
                             Nach,
                             nitsuja
  (c) Copyright 2009 - 2011  OV2

  Mac OS GUI code
  (c) Copyright 1998 - 2001  John Stiles
  (c) Copyright 2001 - 2011  zones


  Specific ports contains the works of other authors. See headers in
  individual files.


  Snes9x homepage: http://www.snes9x.com/

  Permission to use, copy, modify and/or distribute Snes9x in both binary
  and source form, for non-commercial purposes, is hereby granted without
  fee, providing that this license information and copyright notice appear
  with all copies and any derived work.

  This software is provided 'as-is', without any express or implied
  warranty. In no event shall the authors be held liable for any damages
  arising from the use of this software or it's derivatives.

  Snes9x is freeware for PERSONAL USE only. Commercial users should
  seek permission of the copyright holders first. Commercial use includes,
  but is not limited to, charging money for Snes9x or software derived from
  Snes9x, including Snes9x or derivatives in commercial game bundles, and/or
  using Snes9x as a promotion for your commercial product.

  The copyright holders request that bug fixes and improvements to the code
  should be forwarded to them so everyone can benefit from the modifications
  in future versions.

  Super NES and Super Nintendo Entertainment System are trademarks of
  Nintendo Co., Limited and its subsidiary companies.
 ***********************************************************************************/


#include <string>
#include <numeric>
#include <assert.h>

#ifdef UNZIP_SUPPORT
#include "unzip/unzip.h"
#endif

#ifdef JMA_SUPPORT
#include "jma/s9x-jma.h"
#endif

#include "snes9x.h"
#include "memmap.h"
#include "apu/apu.h"
#include "fxemu.h"
#include "sdd1.h"
#include "srtc.h"
#include "controls.h"
#include "cheats.h"
#include "movie.h"
#include "reader.h"
#include "display.h"

#ifndef SET_UI_COLOR
#define SET_UI_COLOR(r, g, b) ;
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

static bool8	stopMovie = TRUE;
static char		LastRomFilename[PATH_MAX + 1] = "";

// from NSRT
static const char	*nintendo_licensees[] =
{
	"Unlicensed",
	"Nintendo",
	"Rocket Games/Ajinomoto",
	"Imagineer-Zoom",
	"Gray Matter",
	"Zamuse",
	"Falcom",
	NULL,
	"Capcom",
	"Hot B Co.",
	"Jaleco",
	"Coconuts Japan",
	"Coconuts Japan/G.X.Media",
	"Micronet",
	"Technos",
	"Mebio Software",
	"Shouei System",
	"Starfish",
	NULL,
	"Mitsui Fudosan/Dentsu",
	NULL,
	"Warashi Inc.",
	NULL,
	"Nowpro",
	NULL,
	"Game Village",
	"IE Institute",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Banarex",
	"Starfish",
	"Infocom",
	"Electronic Arts Japan",
	NULL,
	"Cobra Team",
	"Human/Field",
	"KOEI",
	"Hudson Soft",
	"S.C.P./Game Village",
	"Yanoman",
	NULL,
	"Tecmo Products",
	"Japan Glary Business",
	"Forum/OpenSystem",
	"Virgin Games (Japan)",
	"SMDE",
	"Yojigen",
	NULL,
	"Daikokudenki",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Creatures Inc.",
	"TDK Deep Impresion",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Destination Software/KSS",
	"Sunsoft/Tokai Engineering",
	"POW (Planning Office Wada)/VR 1 Japan",
	"Micro World",
	NULL,
	"San-X",
	"Enix",
	"Loriciel/Electro Brain",
	"Kemco Japan",
	"Seta Co.,Ltd.",
	"Culture Brain",
	"Irem Corp.",
	"Palsoft",
	"Visit Co., Ltd.",
	"Intec",
	"System Sacom",
	"Poppo",
	"Ubisoft Japan",
	NULL,
	"Media Works",
	"NEC InterChannel",
	"Tam",
	"Gajin/Jordan",
	"Smilesoft",
	NULL,
	NULL,
	"Mediakite",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Viacom",
	"Carrozzeria",
	"Dynamic",
	NULL,
	"Magifact",
	"Hect",
	"Codemasters",
	"Taito/GAGA Communications",
	"Laguna",
	"Telstar Fun & Games/Event/Taito",
	NULL,
	"Arcade Zone Ltd.",
	"Entertainment International/Empire Software",
	"Loriciel",
	"Gremlin Graphics",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Seika Corp.",
	"UBI SOFT Entertainment Software",
	"Sunsoft US",
	NULL,
	"Life Fitness",
	NULL,
	"System 3",
	"Spectrum Holobyte",
	NULL,
	"Irem",
	NULL,
	"Raya Systems",
	"Renovation Products",
	"Malibu Games",
	NULL,
	"Eidos/U.S. Gold",
	"Playmates Interactive",
	NULL,
	NULL,
	"Fox Interactive",
	"Time Warner Interactive",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Disney Interactive",
	NULL,
	"Black Pearl",
	NULL,
	"Advanced Productions",
	NULL,
	NULL,
	"GT Interactive",
	"RARE",
	"Crave Entertainment",
	"Absolute Entertainment",
	"Acclaim",
	"Activision",
	"American Sammy",
	"Take 2/GameTek",
	"Hi Tech",
	"LJN Ltd.",
	NULL,
	"Mattel",
	NULL,
	"Mindscape/Red Orb Entertainment",
	"Romstar",
	"Taxan",
	"Midway/Tradewest",
	NULL,
	"American Softworks Corp.",
	"Majesco Sales Inc.",
	"3DO",
	NULL,
	NULL,
	"Hasbro",
	"NewKidCo",
	"Telegames",
	"Metro3D",
	NULL,
	"Vatical Entertainment",
	"LEGO Media",
	NULL,
	"Xicat Interactive",
	"Cryo Interactive",
	NULL,
	NULL,
	"Red Storm Entertainment",
	"Microids",
	NULL,
	"Conspiracy/Swing",
	"Titus",
	"Virgin Interactive",
	"Maxis",
	NULL,
	"LucasArts Entertainment",
	NULL,
	NULL,
	"Ocean",
	NULL,
	"Electronic Arts",
	NULL,
	"Laser Beam",
	NULL,
	NULL,
	"Elite Systems",
	"Electro Brain",
	"The Learning Company",
	"BBC",
	NULL,
	"Software 2000",
	NULL,
	"BAM! Entertainment",
	"Studio 3",
	NULL,
	NULL,
	NULL,
	"Classified Games",
	NULL,
	"TDK Mediactive",
	NULL,
	"DreamCatcher",
	"JoWood Produtions",
	"SEGA",
	"Wannado Edition",
	"LSP (Light & Shadow Prod.)",
	"ITE Media",
	"Infogrames",
	"Interplay",
	"JVC (US)",
	"Parker Brothers",
	NULL,
	"SCI (Sales Curve Interactive)/Storm",
	NULL,
	NULL,
	"THQ Software",
	"Accolade Inc.",
	"Triffix Entertainment",
	NULL,
	"Microprose Software",
	"Universal Interactive/Sierra/Simon & Schuster",
	NULL,
	"Kemco",
	"Rage Software",
	"Encore",
	NULL,
	"Zoo",
	"Kiddinx",
	"Simon & Schuster Interactive",
	"Asmik Ace Entertainment Inc./AIA",
	"Empire Interactive",
	NULL,
	NULL,
	"Jester Interactive",
	NULL,
	"Rockstar Games",
	"Scholastic",
	"Ignition Entertainment",
	"Summitsoft",
	"Stadlbauer",
	NULL,
	NULL,
	NULL,
	"Misawa",
	"Teichiku",
	"Namco Ltd.",
	"LOZC",
	"KOEI",
	NULL,
	"Tokuma Shoten Intermedia",
	"Tsukuda Original",
	"DATAM-Polystar",
	NULL,
	NULL,
	"Bullet-Proof Software",
	"Vic Tokai Inc.",
	NULL,
	"Character Soft",
	"I'Max",
	"Saurus",
	NULL,
	NULL,
	"General Entertainment",
	NULL,
	NULL,
	"I'Max",
	"Success",
	NULL,
	"SEGA Japan",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Takara",
	"Chun Soft",
	"Video System Co., Ltd./McO'River",
	"BEC",
	NULL,
	"Varie",
	"Yonezawa/S'pal",
	"Kaneko",
	NULL,
	"Victor Interactive Software/Pack-in-Video",
	"Nichibutsu/Nihon Bussan",
	"Tecmo",
	"Imagineer",
	NULL,
	NULL,
	"Nova",
	"Den'Z",
	"Bottom Up",
	NULL,
	"TGL (Technical Group Laboratory)",
	NULL,
	"Hasbro Japan",
	NULL,
	"Marvelous Entertainment",
	NULL,
	"Keynet Inc.",
	"Hands-On Entertainment",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Telenet",
	"Hori",
	NULL,
	NULL,
	"Konami",
	"K.Amusement Leasing Co.",
	"Kawada",
	"Takara",
	NULL,
	"Technos Japan Corp.",
	"JVC (Europe/Japan)/Victor Musical Industries",
	NULL,
	"Toei Animation",
	"Toho",
	NULL,
	"Namco",
	"Media Rings Corp.",
	"J-Wing",
	NULL,
	"Pioneer LDC",
	"KID",
	"Mediafactory",
	NULL,
	NULL,
	NULL,
	"Infogrames Hudson",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Acclaim Japan",
	"ASCII Co./Nexoft",
	"Bandai",
	NULL,
	"Enix",
	NULL,
	"HAL Laboratory/Halken",
	"SNK",
	NULL,
	"Pony Canyon Hanbai",
	"Culture Brain",
	"Sunsoft",
	"Toshiba EMI",
	"Sony Imagesoft",
	NULL,
	"Sammy",
	"Magical",
	"Visco",
	NULL,
	"Compile",
	NULL,
	"MTO Inc.",
	NULL,
	"Sunrise Interactive",
	NULL,
	"Global A Entertainment",
	"Fuuki",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Taito",
	NULL,
	"Kemco",
	"Square",
	"Tokuma Shoten",
	"Data East",
	"Tonkin House",
	NULL,
	"KOEI",
	NULL,
	"Konami/Ultra/Palcom",
	"NTVIC/VAP",
	"Use Co., Ltd.",
	"Meldac",
	"Pony Canyon (Japan)/FCI (US)",
	"Angel/Sotsu Agency/Sunrise",
	"Yumedia/Aroma Co., Ltd.",
	NULL,
	NULL,
	"Boss",
	"Axela/Crea-Tech",
	"Sekaibunka-Sha/Sumire kobo/Marigul Management Inc.",
	"Konami Computer Entertainment Osaka",
	NULL,
	NULL,
	"Enterbrain",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Taito/Disco",
	"Sofel",
	"Quest Corp.",
	"Sigma",
	"Ask Kodansha",
	NULL,
	"Naxat",
	"Copya System",
	"Capcom Co., Ltd.",
	"Banpresto",
	"TOMY",
	"Acclaim/LJN Japan",
	NULL,
	"NCS",
	"Human Entertainment",
	"Altron",
	"Jaleco",
	"Gaps Inc.",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Elf",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Jaleco",
	NULL,
	"Yutaka",
	"Varie",
	"T&ESoft",
	"Epoch Co., Ltd.",
	NULL,
	"Athena",
	"Asmik",
	"Natsume",
	"King Records",
	"Atlus",
	"Epic/Sony Records (Japan)",
	NULL,
	"IGS (Information Global Service)",
	NULL,
	"Chatnoir",
	"Right Stuff",
	NULL,
	"NTT COMWARE",
	NULL,
	"Spike",
	"Konami Computer Entertainment Tokyo",
	"Alphadream Corp.",
	NULL,
	"Sting",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"A Wave",
	"Motown Software",
	"Left Field Entertainment",
	"Extreme Entertainment Group",
	"TecMagik",
	NULL,
	NULL,
	NULL,
	NULL,
	"Cybersoft",
	NULL,
	"Psygnosis",
	NULL,
	NULL,
	"Davidson/Western Tech.",
	"Unlicensed",
	NULL,
	NULL,
	NULL,
	NULL,
	"The Game Factory Europe",
	"Hip Games",
	"Aspyr",
	NULL,
	NULL,
	"Mastiff",
	"iQue",
	"Digital Tainment Pool",
	"XS Games",
	"Daiwon",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"PCCW Japan",
	NULL,
	NULL,
	"KiKi Co. Ltd.",
	"Open Sesame Inc.",
	"Sims",
	"Broccoli",
	"Avex",
	"D3 Publisher",
	NULL,
	"Konami Computer Entertainment Japan",
	NULL,
	"Square-Enix",
	"KSG",
	"Micott & Basara Inc.",
	NULL,
	"Orbital Media",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"The Game Factory USA",
	NULL,
	NULL,
	"Treasure",
	"Aruze",
	"Ertain",
	"SNK Playmore",
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	NULL,
	"Yojigen"
};

static const uint32	crc32Table[256] =
{
	0x00000000, 0x77073096, 0xee0e612c, 0x990951ba, 0x076dc419, 0x706af48f,
	0xe963a535, 0x9e6495a3, 0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
	0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91, 0x1db71064, 0x6ab020f2,
	0xf3b97148, 0x84be41de, 0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
	0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec, 0x14015c4f, 0x63066cd9,
	0xfa0f3d63, 0x8d080df5, 0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
	0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b, 0x35b5a8fa, 0x42b2986c,
	0xdbbbc9d6, 0xacbcf940, 0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
	0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116, 0x21b4f4b5, 0x56b3c423,
	0xcfba9599, 0xb8bda50f, 0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
	0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d, 0x76dc4190, 0x01db7106,
	0x98d220bc, 0xefd5102a, 0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
	0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818, 0x7f6a0dbb, 0x086d3d2d,
	0x91646c97, 0xe6635c01, 0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
	0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457, 0x65b0d9c6, 0x12b7e950,
	0x8bbeb8ea, 0xfcb9887c, 0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
	0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2, 0x4adfa541, 0x3dd895d7,
	0xa4d1c46d, 0xd3d6f4fb, 0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
	0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9, 0x5005713c, 0x270241aa,
	0xbe0b1010, 0xc90c2086, 0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
	0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4, 0x59b33d17, 0x2eb40d81,
	0xb7bd5c3b, 0xc0ba6cad, 0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
	0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683, 0xe3630b12, 0x94643b84,
	0x0d6d6a3e, 0x7a6a5aa8, 0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
	0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe, 0xf762575d, 0x806567cb,
	0x196c3671, 0x6e6b06e7, 0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
	0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5, 0xd6d6a3e8, 0xa1d1937e,
	0x38d8c2c4, 0x4fdff252, 0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
	0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60, 0xdf60efc3, 0xa867df55,
	0x316e8eef, 0x4669be79, 0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
	0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f, 0xc5ba3bbe, 0xb2bd0b28,
	0x2bb45a92, 0x5cb36a04, 0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
	0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a, 0x9c0906a9, 0xeb0e363f,
	0x72076785, 0x05005713, 0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
	0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21, 0x86d3d2d4, 0xf1d4e242,
	0x68ddb3f8, 0x1fda836e, 0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
	0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c, 0x8f659eff, 0xf862ae69,
	0x616bffd3, 0x166ccf45, 0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
	0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db, 0xaed16a4a, 0xd9d65adc,
	0x40df0b66, 0x37d83bf0, 0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
	0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6, 0xbad03605, 0xcdd70693,
	0x54de5729, 0x23d967bf, 0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
	0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d
};

static void S9xDeinterleaveType1 (int, uint8 *);
static void S9xDeinterleaveType2 (int, uint8 *);
static void S9xDeinterleaveGD24 (int, uint8 *);
static bool8 allASCII (uint8 *, int);
static bool8 is_SufamiTurbo_BIOS (uint8 *, uint32);
static bool8 is_SufamiTurbo_Cart (uint8 *, uint32);
static bool8 is_SameGame_BIOS (uint8 *, uint32);
static bool8 is_SameGame_Add_On (uint8 *, uint32);
static uint32 caCRC32 (uint8 *, uint32, uint32 crc32 = 0xffffffff);
static uint32 ReadUPSPointer (const uint8 *, unsigned &, unsigned);
static bool8 ReadUPSPatch (Reader *, long, int32 &);
static long ReadInt (Reader *, unsigned);
static bool8 ReadIPSPatch (Reader *, long, int32 &);
#ifdef UNZIP_SUPPORT
static int unzFindExtension (unzFile &, const char *, bool restart = TRUE, bool print = TRUE);
#endif

// deinterleave

static void S9xDeinterleaveType1 (int size, uint8 *base)
{
	Settings.DisplayColor = BUILD_PIXEL(0, 31, 0);
	SET_UI_COLOR(0, 255, 0);

	uint8	blocks[256];
	int		nblocks = size >> 16;

	for (int i = 0; i < nblocks; i++)
	{
		blocks[i * 2] = i + nblocks;
		blocks[i * 2 + 1] = i;
	}

	uint8	*tmp = (uint8 *) malloc(0x8000);
	if (tmp)
	{
		for (int i = 0; i < nblocks * 2; i++)
		{
			for (int j = i; j < nblocks * 2; j++)
			{
				if (blocks[j] == i)
				{
					memmove(tmp, &base[blocks[j] * 0x8000], 0x8000);
					memmove(&base[blocks[j] * 0x8000], &base[blocks[i] * 0x8000], 0x8000);
					memmove(&base[blocks[i] * 0x8000], tmp, 0x8000);
					uint8	b = blocks[j];
					blocks[j] = blocks[i];
					blocks[i] = b;
					break;
				}
			}
		}

		free(tmp);
	}
}

static void S9xDeinterleaveType2 (int size, uint8 *base)
{
	// for odd Super FX images
	Settings.DisplayColor = BUILD_PIXEL(31, 14, 6);
	SET_UI_COLOR(255, 119, 25);

	uint8	blocks[256];
	int		nblocks = size >> 16;
	int		step = 64;

	while (nblocks <= step)
		step >>= 1;
	nblocks = step;

	for (int i = 0; i < nblocks * 2; i++)
		blocks[i] = (i & ~0xf) | ((i & 3) << 2) | ((i & 12) >> 2);

	uint8	*tmp = (uint8 *) malloc(0x10000);
	if (tmp)
	{
		for (int i = 0; i < nblocks * 2; i++)
		{
			for (int j = i; j < nblocks * 2; j++)
			{
				if (blocks[j] == i)
				{
					memmove(tmp, &base[blocks[j] * 0x10000], 0x10000);
					memmove(&base[blocks[j] * 0x10000], &base[blocks[i] * 0x10000], 0x10000);
					memmove(&base[blocks[i] * 0x10000], tmp, 0x10000);
					uint8	b = blocks[j];
					blocks[j] = blocks[i];
					blocks[i] = b;
					break;
				}
			}
		}

		free(tmp);
	}
}

static void S9xDeinterleaveGD24 (int size, uint8 *base)
{
	// for 24Mb images dumped with Game Doctor
	if (size != 0x300000)
		return;

	Settings.DisplayColor = BUILD_PIXEL(0, 31, 31);
	SET_UI_COLOR(0, 255, 255);

	uint8	*tmp = (uint8 *) malloc(0x80000);
	if (tmp)
	{
		memmove(tmp, &base[0x180000], 0x80000);
		memmove(&base[0x180000], &base[0x200000], 0x80000);
		memmove(&base[0x200000], &base[0x280000], 0x80000);
		memmove(&base[0x280000], tmp, 0x80000);

		free(tmp);

		S9xDeinterleaveType1(size, base);
	}
}

// allocation and deallocation

bool8 CMemory::Init (void)
{
    RAM	 = (uint8 *) malloc(0x20000);
    SRAM = (uint8 *) malloc(0x20000);
    VRAM = (uint8 *) malloc(0x10000);
    ROM  = (uint8 *) malloc(MAX_ROM_SIZE + 0x200 + 0x8000);

	IPPU.TileCache[TILE_2BIT]       = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT]       = (uint8 *) malloc(MAX_4BIT_TILES * 64);
	IPPU.TileCache[TILE_8BIT]       = (uint8 *) malloc(MAX_8BIT_TILES * 64);
	IPPU.TileCache[TILE_2BIT_EVEN]  = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_2BIT_ODD]   = (uint8 *) malloc(MAX_2BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT_EVEN]  = (uint8 *) malloc(MAX_4BIT_TILES * 64);
	IPPU.TileCache[TILE_4BIT_ODD]   = (uint8 *) malloc(MAX_4BIT_TILES * 64);

	IPPU.TileCached[TILE_2BIT]      = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_4BIT]      = (uint8 *) malloc(MAX_4BIT_TILES);
	IPPU.TileCached[TILE_8BIT]      = (uint8 *) malloc(MAX_8BIT_TILES);
	IPPU.TileCached[TILE_2BIT_EVEN] = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_2BIT_ODD]  = (uint8 *) malloc(MAX_2BIT_TILES);
	IPPU.TileCached[TILE_4BIT_EVEN] = (uint8 *) malloc(MAX_4BIT_TILES);
	IPPU.TileCached[TILE_4BIT_ODD]  = (uint8 *) malloc(MAX_4BIT_TILES);

	if (!RAM || !SRAM || !VRAM || !ROM ||
		!IPPU.TileCache[TILE_2BIT]       ||
		!IPPU.TileCache[TILE_4BIT]       ||
		!IPPU.TileCache[TILE_8BIT]       ||
		!IPPU.TileCache[TILE_2BIT_EVEN]  ||
		!IPPU.TileCache[TILE_2BIT_ODD]   ||
		!IPPU.TileCache[TILE_4BIT_EVEN]  ||
		!IPPU.TileCache[TILE_4BIT_ODD]   ||
		!IPPU.TileCached[TILE_2BIT]      ||
		!IPPU.TileCached[TILE_4BIT]      ||
		!IPPU.TileCached[TILE_8BIT]      ||
		!IPPU.TileCached[TILE_2BIT_EVEN] ||
		!IPPU.TileCached[TILE_2BIT_ODD]  ||
		!IPPU.TileCached[TILE_4BIT_EVEN] ||
		!IPPU.TileCached[TILE_4BIT_ODD])
    {
		Deinit();
		return (FALSE);
    }

	ZeroMemory(RAM,  0x20000);
	ZeroMemory(SRAM, 0x20000);
	ZeroMemory(VRAM, 0x10000);
	ZeroMemory(ROM,  MAX_ROM_SIZE + 0x200 + 0x8000);

	ZeroMemory(IPPU.TileCache[TILE_2BIT],       MAX_2BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_4BIT],       MAX_4BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_8BIT],       MAX_8BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_2BIT_EVEN],  MAX_2BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_2BIT_ODD],   MAX_2BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_4BIT_EVEN],  MAX_4BIT_TILES * 64);
	ZeroMemory(IPPU.TileCache[TILE_4BIT_ODD],   MAX_4BIT_TILES * 64);

	ZeroMemory(IPPU.TileCached[TILE_2BIT],      MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT],      MAX_4BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_8BIT],      MAX_8BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_2BIT_EVEN], MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_2BIT_ODD],  MAX_2BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT_EVEN], MAX_4BIT_TILES);
	ZeroMemory(IPPU.TileCached[TILE_4BIT_ODD],  MAX_4BIT_TILES);

	// FillRAM uses first 32K of ROM image area, otherwise space just
	// wasted. Might be read by the SuperFX code.

	FillRAM = ROM;

	// Add 0x8000 to ROM image pointer to stop SuperFX code accessing
	// unallocated memory (can cause crash on some ports).

	ROM += 0x8000;

	C4RAM   = ROM + 0x400000 + 8192 * 8; // C4
	OBC1RAM = ROM + 0x400000; // OBC1
	BIOSROM = ROM + 0x300000; // BS
	BSRAM   = ROM + 0x400000; // BS

	SuperFX.pvRegisters = FillRAM + 0x3000;
	SuperFX.nRamBanks   = 2; // Most only use 1.  1=64KB=512Mb, 2=128KB=1024Mb
	SuperFX.pvRam       = SRAM;
	SuperFX.nRomBanks   = (2 * 1024 * 1024) / (32 * 1024);
	SuperFX.pvRom       = (uint8 *) ROM;

	PostRomInitFunc = NULL;

	return (TRUE);
}

void CMemory::Deinit (void)
{
	if (RAM)
	{
		free(RAM);
		RAM = NULL;
	}

	if (SRAM)
	{
		free(SRAM);
		SRAM = NULL;
	}

	if (VRAM)
	{
		free(VRAM);
		VRAM = NULL;
	}

	if (ROM)
	{
		ROM -= 0x8000;
		free(ROM);
		ROM = NULL;
	}

	for (int t = 0; t < 7; t++)
	{
		if (IPPU.TileCache[t])
		{
			free(IPPU.TileCache[t]);
			IPPU.TileCache[t] = NULL;
		}

		if (IPPU.TileCached[t])
		{
			free(IPPU.TileCached[t]);
			IPPU.TileCached[t] = NULL;
		}
	}

	Safe(NULL);
	SafeANK(NULL);
}

// file management and ROM detection

static bool8 allASCII (uint8 *b, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (b[i] < 32 || b[i] > 126)
			return (FALSE);
	}

	return (TRUE);
}

static bool8 is_SufamiTurbo_BIOS (uint8 *data, uint32 size)
{
	if (size == 0x40000 &&
		strncmp((char *) data, "BANDAI SFC-ADX", 14) == 0 && strncmp((char * ) (data + 0x10), "SFC-ADX BACKUP", 14) == 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_SufamiTurbo_Cart (uint8 *data, uint32 size)
{
	if (size >= 0x80000 && size <= 0x100000 &&
		strncmp((char *) data, "BANDAI SFC-ADX", 14) == 0 && strncmp((char * ) (data + 0x10), "SFC-ADX BACKUP", 14) != 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_SameGame_BIOS (uint8 *data, uint32 size)
{
	if (size == 0x100000 && strncmp((char *) (data + 0xffc0), "Same Game Tsume Game", 20) == 0)
		return (TRUE);
	else
		return (FALSE);
}

static bool8 is_SameGame_Add_On (uint8 *data, uint32 size)
{
	if (size == 0x80000)
		return (TRUE);
	else
		return (FALSE);
}

int CMemory::ScoreHiROM (bool8 skip_header, int32 romoff)
{
	uint8	*buf = ROM + 0xff00 + romoff + (skip_header ? 0x200 : 0);
	int		score = 0;

	if (buf[0xd5] & 0x1)
		score += 2;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score -= 2;

	if (buf[0xd4] == 0x20)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (0 != (buf[0xde] + (buf[0xdf] << 8)))
			score++;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced after looking at a scan by Cowering

	if (CalculatedSize > 1024 * 1024 * 3)
		score += 4;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

int CMemory::ScoreLoROM (bool8 skip_header, int32 romoff)
{
	uint8	*buf = ROM + 0x7f00 + romoff + (skip_header ? 0x200 : 0);
	int		score = 0;

	if (!(buf[0xd5] & 0x1))
		score += 3;

	// Mode23 is SA-1
	if (buf[0xd5] == 0x23)
		score += 2;

	if ((buf[0xdc] + (buf[0xdd] << 8)) + (buf[0xde] + (buf[0xdf] << 8)) == 0xffff)
	{
		score += 2;
		if (0 != (buf[0xde] + (buf[0xdf] << 8)))
			score++;
	}

	if (buf[0xda] == 0x33)
		score += 2;

	if ((buf[0xd5] & 0xf) < 4)
		score += 2;

	if (!(buf[0xfd] & 0x80))
		score -= 6;

	if ((buf[0xfc] + (buf[0xfd] << 8)) > 0xffb0)
		score -= 2; // reduced per Cowering suggestion

	if (CalculatedSize <= 1024 * 1024 * 16)
		score += 2;

	if ((1 << (buf[0xd7] - 7)) > 48)
		score -= 1;

	if (!allASCII(&buf[0xb0], 6))
		score -= 1;

	if (!allASCII(&buf[0xc0], ROM_NAME_LEN - 1))
		score -= 1;

	return (score);
}

uint32 CMemory::HeaderRemove (uint32 size, int32 &headerCount, uint8 *buf)
{
	uint32	calc_size = (size / 0x2000) * 0x2000;

	if ((size - calc_size == 512 && !Settings.ForceNoHeader) || Settings.ForceHeader)
	{
		uint8	*NSRTHead = buf + 0x1D0; // NSRT Header Location

		// detect NSRT header
		if (!strncmp("NSRT", (char *) &NSRTHead[24], 4))
		{
			if (NSRTHead[28] == 22)
			{
				if (((std::accumulate(NSRTHead, NSRTHead + sizeof(NSRTHeader), 0) & 0xFF) == NSRTHead[30]) &&
					(NSRTHead[30] + NSRTHead[31] == 255) && ((NSRTHead[0] & 0x0F) <= 13) &&
					(((NSRTHead[0] & 0xF0) >> 4) <= 3) && ((NSRTHead[0] & 0xF0) >> 4))
					memcpy(NSRTHeader, NSRTHead, sizeof(NSRTHeader));
			}
		}

		memmove(buf, buf + 512, calc_size);
		headerCount++;
		size -= 512;
	}

	return (size);
}

uint32 CMemory::FileLoader (uint8 *buffer, const char *filename, int32 maxsize)
{
	// <- ROM size without header
	// ** Memory.HeaderCount
	// ** Memory.ROMFilename

	int32	totalSize = 0;
    char	fname[PATH_MAX + 1];
    char	drive[_MAX_DRIVE + 1], dir[_MAX_DIR + 1], name[_MAX_FNAME + 1], exts[_MAX_EXT + 1];
	char	*ext;

#if defined(__WIN32__) || defined(__MACOSX__)
	ext = &exts[1];
#else
	ext = &exts[0];
#endif

	memset(NSRTHeader, 0, sizeof(NSRTHeader));
	HeaderCount = 0;

	_splitpath(filename, drive, dir, name, exts);
	_makepath(fname, drive, dir, name, exts);

	int	nFormat = FILE_DEFAULT;
	if (strcasecmp(ext, "zip") == 0)
		nFormat = FILE_ZIP;
	else
	if (strcasecmp(ext, "jma") == 0)
		nFormat = FILE_JMA;

	switch (nFormat)
	{
		case FILE_ZIP:
		{
		#ifdef UNZIP_SUPPORT
			if (!LoadZip(fname, &totalSize, &HeaderCount, buffer))
			{
			 	S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Invalid Zip archive.");
				return (0);
			}

			strcpy(ROMFilename, fname);
		#else
			S9xMessage(S9X_ERROR, S9X_ROM_INFO, "This binary was not created with Zip support.");
			return (0);
		#endif
			break;
		}

		case FILE_JMA:
		{
		#ifdef JMA_SUPPORT
			size_t	size = load_jma_file(fname, buffer);
			if (!size)
			{
			 	S9xMessage(S9X_ERROR, S9X_ROM_INFO, "Invalid JMA archive.");
				return (0);
			}

			totalSize = HeaderRemove(size, HeaderCount, buffer);

			strcpy(ROMFilename, fname);
		#else
			S9xMessage(S9X_ERROR, S9X_ROM_INFO, "This binary was not created with JMA support.");
			return (0);
		#endif
			break;
		}

		case FILE_DEFAULT:
		default:
		{
			STREAM	fp = OPEN_STREAM(fname, "rb");
			if (!fp)
				return (0);

			strcpy(ROMFilename, fname);

			int 	len  = 0;
			uint32	size = 0;
			bool8	more = FALSE;
			uint8	*ptr = buffer;

			do
			{
				size = READ_STREAM(ptr, maxsize + 0x200 - (ptr - buffer), fp);
				CLOSE_STREAM(fp);

				size = HeaderRemove(size, HeaderCount, ptr);
				totalSize += size;
				ptr += size;

				// check for multi file roms
				if (ptr - buffer < maxsize + 0x200 &&
					(isdigit(ext[0]) && ext[1] == 0 && ext[0] < '9'))
				{
					more = TRUE;
					ext[0]++;
					_makepath(fname, drive, dir, name, exts);
				}
				else
				if (ptr - buffer < maxsize + 0x200 &&
					(((len = strlen(name)) == 7 || len == 8) &&
					strncasecmp(name, "sf", 2) == 0 &&
					isdigit(name[2]) && isdigit(name[3]) && isdigit(name[4]) && isdigit(name[5]) &&
					isalpha(name[len - 1])))
				{
					more = TRUE;
					name[len - 1]++;
					_makepath(fname, drive, dir, name, exts);
				}
				else
					more = FALSE;

			}	while (more && (fp = OPEN_STREAM(fname, "rb")) != NULL);

			break;
		}
	}

    if (HeaderCount == 0)
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "No ROM file header found.");
    else
    if (HeaderCount == 1)
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "Found ROM file header (and ignored it).");
	else
		S9xMessage(S9X_INFO, S9X_HEADERS_INFO, "Found multiple ROM file headers (and ignored them).");

	return ((uint32) totalSize);
}

bool8 CMemory::LoadROM (const char *filename)
{
	int	retry_count = 0;

	if (!filename || !*filename)
		return (FALSE);

	ZeroMemory(ROM, MAX_ROM_SIZE);
	ZeroMemory(&Multi, sizeof(Multi));
 
again:
	Settings.DisplayColor = BUILD_PIXEL(31, 31, 31);
	SET_UI_COLOR(255, 255, 255);

	CalculatedSize = 0;
	ExtendedFormat = NOPE;

	int32 totalFileSize;

	totalFileSize = FileLoader(ROM, filename, MAX_ROM_SIZE);
	if (!totalFileSize)
		return (FALSE);

	if (!Settings.NoPatch)
		CheckForAnyPatch(filename, HeaderCount != 0, totalFileSize);

	int	hi_score, lo_score;

	hi_score = ScoreHiROM(FALSE);
	lo_score = ScoreLoROM(FALSE);

	if (HeaderCount == 0 && !Settings.ForceNoHeader &&
		((hi_score >  lo_score && ScoreHiROM(TRUE) > hi_score) ||
		 (hi_score <= lo_score && ScoreLoROM(TRUE) > lo_score)))
	{
		memmove(ROM, ROM + 512, totalFileSize - 512);
		totalFileSize -= 512;
		S9xMessage(S9X_INFO, S9X_HEADER_WARNING, "Try 'force no-header' option if the game doesn't work");
		// modifying ROM, so we need to rescore
		hi_score = ScoreHiROM(FALSE);
		lo_score = ScoreLoROM(FALSE);
	}

	CalculatedSize = (totalFileSize / 0x2000) * 0x2000;

	if (CalculatedSize > 0x400000 &&
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x4332 && // exclude S-DD1
		(ROM[0x7fd5] + (ROM[0x7fd6] << 8)) != 0x4532 &&
		(ROM[0xffd5] + (ROM[0xffd6] << 8)) != 0xF93a && // exclude SPC7110
		(ROM[0xffd5] + (ROM[0xffd6] << 8)) != 0xF53a)
		ExtendedFormat = YEAH;

	// if both vectors are invalid, it's type 1 interleaved LoROM
	if (ExtendedFormat == NOPE &&
		((ROM[0x7ffc] + (ROM[0x7ffd] << 8)) < 0x8000) &&
		((ROM[0xfffc] + (ROM[0xfffd] << 8)) < 0x8000))
	{
		if (!Settings.ForceInterleaved && !Settings.ForceNotInterleaved)
			S9xDeinterleaveType1(totalFileSize, ROM);
	}

	// CalculatedSize is now set, so rescore
	hi_score = ScoreHiROM(FALSE);
	lo_score = ScoreLoROM(FALSE);

	uint8	*RomHeader = ROM;

	if (ExtendedFormat != NOPE)
	{
		int	swappedhirom, swappedlorom;

		swappedhirom = ScoreHiROM(FALSE, 0x400000);
		swappedlorom = ScoreLoROM(FALSE, 0x400000);

		// set swapped here
		if (max(swappedlorom, swappedhirom) >= max(lo_score, hi_score))
		{
			ExtendedFormat = BIGFIRST;
			hi_score = swappedhirom;
			lo_score = swappedlorom;
			RomHeader += 0x400000;
		}
		else
			ExtendedFormat = SMALLFIRST;
	}

	bool8	interleaved, tales = FALSE;

    interleaved = Settings.ForceInterleaved || Settings.ForceInterleaved2 || Settings.ForceInterleaveGD24;

	if (Settings.ForceLoROM || (!Settings.ForceHiROM && lo_score >= hi_score))
	{
		LoROM = TRUE;
		HiROM = FALSE;

		// ignore map type byte if not 0x2x or 0x3x
		if ((RomHeader[0x7fd5] & 0xf0) == 0x20 || (RomHeader[0x7fd5] & 0xf0) == 0x30)
		{
			switch (RomHeader[0x7fd5] & 0xf)
			{
				case 1:
					interleaved = TRUE;
					break;

				case 5:
					interleaved = TRUE;
					tales = TRUE;
					break;
			}
		}
	}
	else
	{
		LoROM = FALSE;
		HiROM = TRUE;

		if ((RomHeader[0xffd5] & 0xf0) == 0x20 || (RomHeader[0xffd5] & 0xf0) == 0x30)
		{
			switch (RomHeader[0xffd5] & 0xf)
			{
				case 0:
				case 3:
					interleaved = TRUE;
					break;
			}
		}
	}

	// this two games fail to be detected
	if (!Settings.ForceHiROM && !Settings.ForceLoROM)
	{
		if (strncmp((char *) &ROM[0x7fc0], "YUYU NO QUIZ DE GO!GO!", 22) == 0 ||
		   (strncmp((char *) &ROM[0xffc0], "BATMAN--REVENGE JOKER",  21) == 0))
		{
			LoROM = TRUE;
			HiROM = FALSE;
			interleaved = FALSE;
			tales = FALSE;
		}
	}

	if (!Settings.ForceNotInterleaved && interleaved)
	{
		S9xMessage(S9X_INFO, S9X_ROM_INTERLEAVED_INFO, "ROM image is in interleaved format - converting...");

		if (tales)
		{
			if (ExtendedFormat == BIGFIRST)
			{
				S9xDeinterleaveType1(0x400000, ROM);
				S9xDeinterleaveType1(CalculatedSize - 0x400000, ROM + 0x400000);
			}
			else
			{
				S9xDeinterleaveType1(CalculatedSize - 0x400000, ROM);
				S9xDeinterleaveType1(0x400000, ROM + CalculatedSize - 0x400000);
			}

			LoROM = FALSE;
			HiROM = TRUE;
		}
		else
		if (Settings.ForceInterleaveGD24 && CalculatedSize == 0x300000)
		{
			bool8	t = LoROM;
			LoROM = HiROM;
			HiROM = t;
			S9xDeinterleaveGD24(CalculatedSize, ROM);
		}
		else
		if (Settings.ForceInterleaved2)
			S9xDeinterleaveType2(CalculatedSize, ROM);
		else
		{
			bool8	t = LoROM;
			LoROM = HiROM;
			HiROM = t;
			S9xDeinterleaveType1(CalculatedSize, ROM);
		}

		hi_score = ScoreHiROM(FALSE);
		lo_score = ScoreLoROM(FALSE);

		if ((HiROM && (lo_score >= hi_score || hi_score < 0)) ||
			(LoROM && (hi_score >  lo_score || lo_score < 0)))
		{
			if (retry_count == 0)
			{
				S9xMessage(S9X_INFO, S9X_ROM_CONFUSING_FORMAT_INFO, "ROM lied about its type! Trying again.");
				Settings.ForceNotInterleaved = TRUE;
				Settings.ForceInterleaved = FALSE;
				retry_count++;
				goto again;
			}
		}
    }

	if (ExtendedFormat == SMALLFIRST)
		tales = TRUE;

	if (tales)
	{
		uint8	*tmp = (uint8 *) malloc(CalculatedSize - 0x400000);
		if (tmp)
		{
			S9xMessage(S9X_INFO, S9X_ROM_INTERLEAVED_INFO, "Fixing swapped ExHiROM...");
			memmove(tmp, ROM, CalculatedSize - 0x400000);
			memmove(ROM, ROM + CalculatedSize - 0x400000, 0x400000);
			memmove(ROM + 0x400000, tmp, CalculatedSize - 0x400000);
			free(tmp);
		}
	}

	if (strncmp(LastRomFilename, filename, PATH_MAX + 1))
	{
		strncpy(LastRomFilename, filename, PATH_MAX + 1);
		LastRomFilename[PATH_MAX] = 0;
	}

	ZeroMemory(&SNESGameFixes, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;

	S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR));

	InitROM();

	S9xInitCheatData();
	S9xApplyCheats();

	S9xReset();

    return (TRUE);
}

bool8 CMemory::LoadMultiCart (const char *cartA, const char *cartB)
{
	bool8	r = TRUE;

	ZeroMemory(ROM, MAX_ROM_SIZE);
	ZeroMemory(&Multi, sizeof(Multi));

	Settings.DisplayColor = BUILD_PIXEL(31, 31, 31);
	SET_UI_COLOR(255, 255, 255);

	CalculatedSize = 0;
	ExtendedFormat = NOPE;

	if (cartA && cartA[0])
		Multi.cartSizeA = FileLoader(ROM, cartA, MAX_ROM_SIZE);

	if (Multi.cartSizeA == 0)
	{
		if (cartB && cartB[0])
			Multi.cartSizeB = FileLoader(ROM, cartB, MAX_ROM_SIZE);
	}

	if (Multi.cartSizeA)
	{
		if (is_SufamiTurbo_Cart(ROM, Multi.cartSizeA))
			Multi.cartType = 4;
		else
		if (is_SameGame_BIOS(ROM, Multi.cartSizeA))
			Multi.cartType = 3;
	}
	else
	if (Multi.cartSizeB)
	{
		if (is_SufamiTurbo_Cart(ROM, Multi.cartSizeB))
			Multi.cartType = 4;
	}
	else
		Multi.cartType = 4; // assuming BIOS only

	switch (Multi.cartType)
	{
		case 4:
			r = LoadSufamiTurbo(cartA, cartB);
			break;

		case 3:
			r = LoadSameGame(cartA, cartB);
			break;

		default:
			r = FALSE;
	}

	if (!r)
	{
		ZeroMemory(&Multi, sizeof(Multi));
		return (FALSE);
	}

	ZeroMemory(&SNESGameFixes, sizeof(SNESGameFixes));
	SNESGameFixes.SRAMInitialValue = 0x60;

	S9xLoadCheatFile(S9xGetFilename(".cht", CHEAT_DIR));

	InitROM();

	S9xInitCheatData();
	S9xApplyCheats();

	S9xReset();

	return (TRUE);
}

bool8 CMemory::LoadSufamiTurbo (const char *cartA, const char *cartB)
{
	Multi.cartOffsetA = 0x100000;
	Multi.cartOffsetB = 0x200000;
	Multi.sramA = SRAM;
	Multi.sramB = SRAM + 0x10000;

	if (Multi.cartSizeA)
	{
		Multi.sramSizeA = 4; // ROM[0x37]?
		Multi.sramMaskA = Multi.sramSizeA ? ((1 << (Multi.sramSizeA + 3)) * 128 - 1) : 0;

		if (!Settings.NoPatch)
			CheckForAnyPatch(cartA, HeaderCount != 0, Multi.cartSizeA);

		strcpy(Multi.fileNameA, cartA);
		memcpy(ROM + Multi.cartOffsetA, ROM, Multi.cartSizeA);
	}

	if (Multi.cartSizeA && !Multi.cartSizeB)
	{
		if (cartB && cartB[0])
			Multi.cartSizeB = FileLoader(ROM, cartB, MAX_ROM_SIZE);

		if (Multi.cartSizeB)
		{
			if (!is_SufamiTurbo_Cart(ROM, Multi.cartSizeB))
				Multi.cartSizeB = 0;
		}
	}

	if (Multi.cartSizeB)
	{
		Multi.sramSizeB = 4; // ROM[0x37]?
		Multi.sramMaskB = Multi.sramSizeB ? ((1 << (Multi.sramSizeB + 3)) * 128 - 1) : 0;

		if (!Settings.NoPatch)
			CheckForAnyPatch(cartB, HeaderCount != 0, Multi.cartSizeB);

		strcpy(Multi.fileNameB, cartB);
		memcpy(ROM + Multi.cartOffsetB, ROM, Multi.cartSizeB);
	}

	FILE	*fp;
	size_t	size;
	char	path[PATH_MAX + 1];

	strcpy(path, S9xGetDirectory(BIOS_DIR));
	strcat(path, SLASH_STR);
	strcat(path, "STBIOS.bin");

	fp = fopen(path, "rb");
	if (fp)
	{
		size = fread((void *) ROM, 1, 0x40000, fp);
		fclose(fp);
		if (!is_SufamiTurbo_BIOS(ROM, size))
			return (FALSE);
	}
	else
		return (FALSE);

	if (Multi.cartSizeA)
		strcpy(ROMFilename, Multi.fileNameA);
	else
	if (Multi.cartSizeB)
		strcpy(ROMFilename, Multi.fileNameB);
	else
		strcpy(ROMFilename, path);

	LoROM = TRUE;
	HiROM = FALSE;
	CalculatedSize = 0x40000;

	return (TRUE);
}

bool8 CMemory::LoadSameGame (const char *cartA, const char *cartB)
{
	Multi.cartOffsetA = 0;
	Multi.cartOffsetB = 0x200000;
	Multi.sramA = SRAM;
	Multi.sramB = NULL;

	Multi.sramSizeA = ROM[0xffd8];
	Multi.sramMaskA = Multi.sramSizeA ? ((1 << (Multi.sramSizeA + 3)) * 128 - 1) : 0;
	Multi.sramSizeB = 0;
	Multi.sramMaskB = 0;

	if (!Settings.NoPatch)
		CheckForAnyPatch(cartA, HeaderCount != 0, Multi.cartSizeA);

	strcpy(Multi.fileNameA, cartA);

	if (cartB && cartB[0])
		Multi.cartSizeB = FileLoader(ROM + Multi.cartOffsetB, cartB, MAX_ROM_SIZE - Multi.cartOffsetB);

	if (Multi.cartSizeB)
	{
		if (!is_SameGame_Add_On(ROM + Multi.cartOffsetB, Multi.cartSizeB))
			Multi.cartSizeB = 0;
		else
			strcpy(Multi.fileNameB, cartB);
	}

	strcpy(ROMFilename, Multi.fileNameA);

	LoROM = FALSE;
	HiROM = TRUE;
	CalculatedSize = Multi.cartSizeA;

	return (TRUE);
}

bool8 CMemory::LoadSRTC (void)
{
	FILE	*fp;
	size_t	ignore;

	fp = fopen(S9xGetFilename(".rtc", SRAM_DIR), "rb");
	if (!fp)
		return (FALSE);

	ignore = fread(RTCData.reg, 1, 20, fp);
	fclose(fp);

	return (TRUE);
}

bool8 CMemory::SaveSRTC (void)
{
	FILE	*fp;
	size_t	ignore;

	fp = fopen(S9xGetFilename(".rtc", SRAM_DIR), "wb");
	if (!fp)
		return (FALSE);

	ignore = fwrite(RTCData.reg, 1, 20, fp);
	fclose(fp);

	return (TRUE);
}

void CMemory::ClearSRAM (bool8 onlyNonSavedSRAM)
{
	if (onlyNonSavedSRAM)
		if (!(Settings.SuperFX && ROMType < 0x15) && !(Settings.SA1 && ROMType == 0x34)) // can have SRAM
			return;

	memset(SRAM, SNESGameFixes.SRAMInitialValue, 0x20000);
}

bool8 CMemory::LoadSRAM (const char *filename)
{
	FILE	*file;
	int		size, len;
	char	sramName[PATH_MAX + 1];

	strcpy(sramName, filename);

	ClearSRAM();

	if (Multi.cartType && Multi.sramSizeB)
	{
		char	temp[PATH_MAX + 1];

		strcpy(temp, ROMFilename);
		strcpy(ROMFilename, Multi.fileNameB);

		size = (1 << (Multi.sramSizeB + 3)) * 128;

		file = fopen(S9xGetFilename(".srm", SRAM_DIR), "rb");
		if (file)
		{
			len = fread((char *) Multi.sramB, 1, 0x10000, file);
			fclose(file);
			if (len - size == 512)
				memmove(Multi.sramB, Multi.sramB + 512, size);
		}

		strcpy(ROMFilename, temp);
	}

	size = SRAMSize ? (1 << (SRAMSize + 3)) * 128 : 0;
	if (size > 0x20000)
		size = 0x20000;

	if (size)
	{
		file = fopen(sramName, "rb");
		if (file)
		{
			len = fread((char *) SRAM, 1, 0x20000, file);
			fclose(file);
			if (len - size == 512)
				memmove(SRAM, SRAM + 512, size);

			if (Settings.SRTC || Settings.SPC7110RTC)
				LoadSRTC();

			return (TRUE);
		}
		else
		if (Settings.BS && !Settings.BSXItself)
		{
			// The BS game's SRAM was not found
			// Try to read BS-X.srm instead
			char	path[PATH_MAX + 1];

			strcpy(path, S9xGetDirectory(SRAM_DIR));
			strcat(path, SLASH_STR);
			strcat(path, "BS-X.srm");

			file = fopen(path, "rb");
			if (file)
			{
				len = fread((char *) SRAM, 1, 0x20000, file);
				fclose(file);
				if (len - size == 512)
					memmove(SRAM, SRAM + 512, size);

				S9xMessage(S9X_INFO, S9X_ROM_INFO, "The SRAM file wasn't found: BS-X.srm was read instead.");
				return (TRUE);
			}
			else
			{
				S9xMessage(S9X_INFO, S9X_ROM_INFO, "The SRAM file wasn't found, BS-X.srm wasn't found either.");
				return (FALSE);
			}
		}

		return (FALSE);
	}

	return (TRUE);
}

bool8 CMemory::SaveSRAM (const char *filename)
{
	if (Settings.SuperFX && ROMType < 0x15) // doesn't have SRAM
		return (TRUE);

	if (Settings.SA1 && ROMType == 0x34)    // doesn't have SRAM
		return (TRUE);

	FILE	*file;
	int		size;
	char	sramName[PATH_MAX + 1];

	strcpy(sramName, filename);

	if (Multi.cartType && Multi.sramSizeB)
	{
		char	name[PATH_MAX + 1], temp[PATH_MAX + 1];

		strcpy(temp, ROMFilename);
		strcpy(ROMFilename, Multi.fileNameB);
		strcpy(name, S9xGetFilename(".srm", SRAM_DIR));

		size = (1 << (Multi.sramSizeB + 3)) * 128;

		file = fopen(name, "wb");
		if (file)
		{
			size_t	ignore;
			ignore = fwrite((char *) Multi.sramB, size, 1, file);
			fclose(file);
		#ifdef __linux
			ignore = chown(name, getuid(), getgid());
		#endif
		}

		strcpy(ROMFilename, temp);
    }

    size = SRAMSize ? (1 << (SRAMSize + 3)) * 128 : 0;
	if (size > 0x20000)
		size = 0x20000;

	if (size)
	{
		file = fopen(sramName, "wb");
		if (file)
		{
			size_t	ignore;
			ignore = fwrite((char *) SRAM, size, 1, file);
			fclose(file);
		#ifdef __linux
			ignore = chown(sramName, getuid(), getgid());
		#endif

			if (Settings.SRTC || Settings.SPC7110RTC)
				SaveSRTC();

			return (TRUE);
		}
	}

	return (FALSE);
}

// initialization

static uint32 caCRC32 (uint8 *array, uint32 size, uint32 crc32)
{
	for (uint32 i = 0; i < size; i++)
		crc32 = ((crc32 >> 8) & 0x00FFFFFF) ^ crc32Table[(crc32 ^ array[i]) & 0xFF];

	return (~crc32);
}

char * CMemory::Safe (const char *s)
{
	static char	*safe = NULL;
	static int	safe_len = 0;

	if (s == NULL)
	{
		if (safe)
		{
			free(safe);
			safe = NULL;
		}

		return (NULL);
	}

	int	len = strlen(s);
	if (!safe || len + 1 > safe_len)
	{
		if (safe)
			free(safe);

		safe_len = len + 1;
		safe = (char *) malloc(safe_len);
	}

	for (int i = 0; i < len; i++)
	{
		if (s[i] >= 32 && s[i] < 127)
			safe[i] = s[i];
		else
			safe[i] = '_';
	}

	safe[len] = 0;

	return (safe);
}

char * CMemory::SafeANK (const char *s)
{
	static char	*safe = NULL;
	static int	safe_len = 0;

	if (s == NULL)
	{
		if (safe)
		{
			free(safe);
			safe = NULL;
		}

		return (NULL);
	}

	int	len = strlen(s);
	if (!safe || len + 1 > safe_len)
	{
		if (safe)
			free(safe);

		safe_len = len + 1;
		safe = (char *) malloc(safe_len);
	}

	for (int i = 0; i < len; i++)
	{
		if (s[i] >= 32 && s[i] < 127) // ASCII
			safe [i] = s[i];
		else
		if (ROMRegion == 0 && ((uint8) s[i] >= 0xa0 && (uint8) s[i] < 0xe0)) // JIS X 201 - Katakana
			safe [i] = s[i];
		else
			safe [i] = '_';
	}

	safe [len] = 0;

	return (safe);
}

void CMemory::ParseSNESHeader (uint8 *RomHeader)
{
	bool8	bs = Settings.BS & !Settings.BSXItself;

	strncpy(ROMName, (char *) &RomHeader[0x10], ROM_NAME_LEN - 1);
	if (bs)
		memset(ROMName + 16, 0x20, ROM_NAME_LEN - 17);

	if (bs)
	{
		if (!(((RomHeader[0x29] & 0x20) && CalculatedSize <  0x100000) ||
			 (!(RomHeader[0x29] & 0x20) && CalculatedSize == 0x100000)))
			printf("BS: Size mismatch\n");

		// FIXME
		int	p = 0;
		while ((1 << p) < (int) CalculatedSize)
			p++;
		ROMSize = p - 10;
	}
	else
		ROMSize = RomHeader[0x27];

	SRAMSize  = bs ? 5 /* BS-X */    : RomHeader[0x28];
	ROMSpeed  = bs ? RomHeader[0x28] : RomHeader[0x25];
	ROMType   = bs ? 0xE5 /* BS-X */ : RomHeader[0x26];
	ROMRegion = bs ? 0               : RomHeader[0x29];

	ROMChecksum           = RomHeader[0x2E] + (RomHeader[0x2F] << 8);
	ROMComplementChecksum = RomHeader[0x2C] + (RomHeader[0x2D] << 8);

	memmove(ROMId, &RomHeader[0x02], 4);

	if (RomHeader[0x2A] != 0x33)
		CompanyId = ((RomHeader[0x2A] >> 4) & 0x0F) * 36 + (RomHeader[0x2A] & 0x0F);
	else
	if (isalnum(RomHeader[0x00]) && isalnum(RomHeader[0x01]))
	{
		int	l, r, l2, r2;
		l = toupper(RomHeader[0x00]);
		r = toupper(RomHeader[0x01]);
		l2 = (l > '9') ? l - '7' : l - '0';
		r2 = (r > '9') ? r - '7' : r - '0';
		CompanyId = l2 * 36 + r2;
	}
}

void CMemory::InitROM (void)
{
	Settings.SuperFX = FALSE;
	Settings.DSP = 0;
	Settings.SA1 = FALSE;
	Settings.C4 = FALSE;
	Settings.SDD1 = FALSE;
	Settings.SPC7110 = FALSE;
	Settings.SPC7110RTC = FALSE;
	Settings.OBC1 = FALSE;
	Settings.SETA = 0;
	Settings.SRTC = FALSE;
	Settings.BS = FALSE;
	
	SuperFX.nRomBanks = CalculatedSize >> 15;

	//// Parse ROM header and read ROM informatoin

	CompanyId = -1;
	memset(ROMId, 0, 5);

	uint8	*RomHeader = ROM + 0x7FB0;
	if (ExtendedFormat == BIGFIRST)
		RomHeader += 0x400000;
	if (HiROM)
		RomHeader += 0x8000;

	S9xInitBSX(); // Set BS header before parsing

	ParseSNESHeader(RomHeader);

	//// Detect and initialize chips
	//// detection codes are compatible with NSRT

	// DSP1/2/3/4
	if (ROMType == 0x03)
	{
		if (ROMSpeed == 0x30)
			Settings.DSP = 4; // DSP4
		else
			Settings.DSP = 1; // DSP1
	}
	else
	if (ROMType == 0x05)
	{
		if (ROMSpeed == 0x20)
			Settings.DSP = 2; // DSP2
		else
		if (ROMSpeed == 0x30 && RomHeader[0x2a] == 0xb2)
			Settings.DSP = 3; // DSP3
		else
			Settings.DSP = 1; // DSP1
	}

	switch (Settings.DSP)
	{
		case 1:	// DSP1
			if (HiROM)
			{
				DSP0.boundary = 0x7000;
				DSP0.maptype = M_DSP1_HIROM;
			}
			else
			if (CalculatedSize > 0x100000)
			{
				DSP0.boundary = 0x4000;
				DSP0.maptype = M_DSP1_LOROM_L;
			}
			else
			{
				DSP0.boundary = 0xc000;
				DSP0.maptype = M_DSP1_LOROM_S;
			}

			SetDSP = &DSP1SetByte;
			GetDSP = &DSP1GetByte;
			break;

		case 2: // DSP2
			DSP0.boundary = 0x10000;
			DSP0.maptype = M_DSP2_LOROM;
			SetDSP = &DSP2SetByte;
			GetDSP = &DSP2GetByte;
			break;

		case 3: // DSP3
			DSP0.boundary = 0xc000;
			DSP0.maptype = M_DSP3_LOROM;
			SetDSP = &DSP3SetByte;
			GetDSP = &DSP3GetByte;
			break;

		case 4: // DSP4
			DSP0.boundary = 0xc000;
			DSP0.maptype = M_DSP4_LOROM;
			SetDSP = &DSP4SetByte;
			GetDSP = &DSP4GetByte;
			break;

		default:
			SetDSP = NULL;
			GetDSP = NULL;
			break;
	}

	uint32	identifier = ((ROMType & 0xff) << 8) + (ROMSpeed & 0xff);

	switch (identifier)
	{
	    // SRTC
		case 0x5535:
			Settings.SRTC = TRUE;
			S9xInitSRTC();
			break;

		// SPC7110
		case 0xF93A:
			Settings.SPC7110RTC = TRUE;
		case 0xF53A:
			Settings.SPC7110 = TRUE;
			S9xInitSPC7110();
			break;

		// OBC1
		case 0x2530:
			Settings.OBC1 = TRUE;
			break;

		// SA1
		case 0x3423:
		case 0x3523:
			Settings.SA1 = TRUE;
			break;

		// SuperFX
		case 0x1320:
		case 0x1420:
		case 0x1520:
		case 0x1A20:
			Settings.SuperFX = TRUE;
			S9xInitSuperFX();
			if (ROM[0x7FDA] == 0x33)
				SRAMSize = ROM[0x7FBD];
			else
				SRAMSize = 5;
			break;

		// SDD1
		case 0x4332:
		case 0x4532:
			Settings.SDD1 = TRUE;
			break;

		// ST018
		case 0xF530:
			Settings.SETA = ST_018;
			SetSETA = NULL;
			GetSETA = NULL;
			SRAMSize = 2;
			SNESGameFixes.SRAMInitialValue = 0x00;
			break;

		// ST010/011
		case 0xF630:
			if (ROM[0x7FD7] == 0x09)
			{
				Settings.SETA = ST_011;
				SetSETA = &S9xSetST011;
				GetSETA = &S9xGetST011;
			}
			else
			{
				Settings.SETA = ST_010;
				SetSETA = &S9xSetST010;
				GetSETA = &S9xGetST010;
			}

			SRAMSize = 2;
			SNESGameFixes.SRAMInitialValue = 0x00;
			break;

		// C4
		case 0xF320:
			Settings.C4 = TRUE;
			break;
	}

	//// Map memory and calculate checksum

	Map_Initialize();
	CalculatedChecksum = 0;

	if (HiROM)
    {
		if (Settings.BS)
			/* Do nothing */;
		else
		if (Settings.SPC7110)
			Map_SPC7110HiROMMap();
		else
		if (ExtendedFormat != NOPE)
			Map_ExtendedHiROMMap();
		else
		if (Multi.cartType == 3)
			Map_SameGameHiROMMap();
		else
			Map_HiROMMap();
    }
    else
    {
		if (Settings.BS)
			/* Do nothing */;
		else
		if (Settings.SETA && Settings.SETA != ST_018)
			Map_SetaDSPLoROMMap();
		else
		if (Settings.SuperFX)
			Map_SuperFXLoROMMap();
		else
		if (Settings.SA1)
			Map_SA1LoROMMap();
		else
		if (Settings.SDD1)
			Map_SDD1LoROMMap();
		else
		if (ExtendedFormat != NOPE)
			Map_JumboLoROMMap();
		else
		if (strncmp(ROMName, "WANDERERS FROM YS", 17) == 0)
			Map_NoMAD1LoROMMap();
		else
		if (strncmp(ROMName, "SOUND NOVEL-TCOOL", 17) == 0 ||
			strncmp(ROMName, "DERBY STALLION 96", 17) == 0)
			Map_ROM24MBSLoROMMap();
		else
		if (strncmp(ROMName, "THOROUGHBRED BREEDER3", 21) == 0 ||
			strncmp(ROMName, "RPG-TCOOL 2", 11) == 0)
			Map_SRAM512KLoROMMap();
		else
		if (strncmp(ROMName, "ADD-ON BASE CASSETE", 19) == 0)
		{
			if (Multi.cartType == 4)
			{
				SRAMSize = Multi.sramSizeA;
				Map_SufamiTurboLoROMMap();
			}
			else
			{
				SRAMSize = 5;
				Map_SufamiTurboPseudoLoROMMap();
			}
		}
		else
			Map_LoROMMap();
    }

	Checksum_Calculate();

	bool8 isChecksumOK = (ROMChecksum + ROMComplementChecksum == 0xffff) &
						 (ROMChecksum == CalculatedChecksum);

	//// Build more ROM information

	// CRC32
	if (!Settings.BS || Settings.BSXItself) // Not BS Dump
		ROMCRC32 = caCRC32(ROM, CalculatedSize);
	else // Convert to correct format before scan
	{
		int offset = HiROM ? 0xffc0 : 0x7fc0;
		// Backup
		uint8 BSMagic0 = ROM[offset + 22],
			  BSMagic1 = ROM[offset + 23];
		// uCONSRT standard
		ROM[offset + 22] = 0x42;
		ROM[offset + 23] = 0x00;
		// Calc
		ROMCRC32 = caCRC32(ROM, CalculatedSize);
		// Convert back
		ROM[offset + 22] = BSMagic0;
		ROM[offset + 23] = BSMagic1;
	}

	// NTSC/PAL
	if (Settings.ForceNTSC)
		Settings.PAL = FALSE;
	else
	if (Settings.ForcePAL)
		Settings.PAL = TRUE;
	else
	if (!Settings.BS && (ROMRegion >= 2) && (ROMRegion <= 12))
		Settings.PAL = TRUE;
	else
		Settings.PAL = FALSE;

	if (Settings.PAL)
	{
		Settings.FrameTime = Settings.FrameTimePAL;
		ROMFramesPerSecond = 50;
	}
	else
	{
		Settings.FrameTime = Settings.FrameTimeNTSC;
		ROMFramesPerSecond = 60;
	}

	// truncate cart name
	ROMName[ROM_NAME_LEN - 1] = 0;
	if (strlen(ROMName))
	{
		char *p = ROMName + strlen(ROMName);
		if (p > ROMName + 21 && ROMName[20] == ' ')
			p = ROMName + 21;
		while (p > ROMName && *(p - 1) == ' ')
			p--;
		*p = 0;
	}

	// SRAM size
	SRAMMask = SRAMSize ? ((1 << (SRAMSize + 3)) * 128) - 1 : 0;

	// checksum
	if (!isChecksumOK || ((uint32) CalculatedSize > (uint32) (((1 << (ROMSize - 7)) * 128) * 1024)))
	{
		Settings.DisplayColor = BUILD_PIXEL(31, 31, 0);
		SET_UI_COLOR(255, 255, 0);
	}

	if (Multi.cartType == 4)
	{
		Settings.DisplayColor = BUILD_PIXEL(0, 16, 31);
		SET_UI_COLOR(0, 128, 255);
	}

	//// Initialize emulation

	Timings.H_Max_Master = SNES_CYCLES_PER_SCANLINE;
	Timings.H_Max        = Timings.H_Max_Master;
	Timings.HBlankStart  = SNES_HBLANK_START_HC;
	Timings.HBlankEnd    = SNES_HBLANK_END_HC;
	Timings.HDMAInit     = SNES_HDMA_INIT_HC;
	Timings.HDMAStart    = SNES_HDMA_START_HC;
	Timings.RenderPos    = SNES_RENDER_START_HC;
	Timings.V_Max_Master = Settings.PAL ? SNES_MAX_PAL_VCOUNTER : SNES_MAX_NTSC_VCOUNTER;
	Timings.V_Max        = Timings.V_Max_Master;
	/* From byuu: The total delay time for both the initial (H)DMA sync (to the DMA clock),
	   and the end (H)DMA sync (back to the last CPU cycle's mcycle rate (6, 8, or 12)) always takes between 12-24 mcycles.
	   Possible delays: { 12, 14, 16, 18, 20, 22, 24 }
	   XXX: Snes9x can't emulate this timing :( so let's use the average value... */
	Timings.DMACPUSync   = 18;
	/* If the CPU is halted (i.e. for DMA) while /NMI goes low, the NMI will trigger
	   after the DMA completes (even if /NMI goes high again before the DMA
	   completes). In this case, there is a 24-30 cycle delay between the end of DMA
	   and the NMI handler, time enough for an instruction or two. */
	// Wild Guns, Mighty Morphin Power Rangers - The Fighting Edition
	Timings.NMIDMADelay  = 24;
	Timings.IRQPendCount = 0;

	IPPU.TotalEmulatedFrames = 0;

	//// Hack games

	ApplyROMFixes();

	//// Show ROM information
	char displayName[ROM_NAME_LEN];

	strcpy(RawROMName, ROMName);
	sprintf(displayName, "%s", SafeANK(ROMName));
	sprintf(ROMName, "%s", Safe(ROMName));
	sprintf(ROMId, "%s", Safe(ROMId));

	sprintf(String, "\"%s\" [%s] %s, %s, %s, %s, SRAM:%s, ID:%s, CRC32:%08X",
		displayName, isChecksumOK ? "checksum ok" : ((Multi.cartType == 4) ? "no checksum" : "bad checksum"),
		MapType(), Size(), KartContents(), Settings.PAL ? "PAL" : "NTSC", StaticRAMSize(), ROMId, ROMCRC32);
	S9xMessage(S9X_INFO, S9X_ROM_INFO, String);

	Settings.ForceLoROM = FALSE;
	Settings.ForceHiROM = FALSE;
	Settings.ForceHeader = FALSE;
	Settings.ForceNoHeader = FALSE;
	Settings.ForceInterleaved = FALSE;
	Settings.ForceInterleaved2 = FALSE;
	Settings.ForceInterleaveGD24 = FALSE;
	Settings.ForceNotInterleaved = FALSE;
	Settings.ForcePAL = FALSE;
	Settings.ForceNTSC = FALSE;

	Settings.TakeScreenshot = FALSE;

	if (stopMovie)
		S9xMovieStop(TRUE);

	if (PostRomInitFunc)
		PostRomInitFunc();

    S9xVerifyControllers();
}

// memory map

uint32 CMemory::map_mirror (uint32 size, uint32 pos)
{
	// from bsnes
	if (size == 0)
		return (0);
	if (pos < size)
		return (pos);

	uint32	mask = 1 << 31;
	while (!(pos & mask))
		mask >>= 1;

	if (size <= (pos & mask))
		return (map_mirror(size, pos - mask));
	else
		return (mask + map_mirror(size - mask, pos - mask));
}

void CMemory::map_lorom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = (c & 0x7f) * 0x8000;
			Map[p] = ROM + map_mirror(size, addr) - (i & 0x8000);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_hirom (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = c << 16;
			Map[p] = ROM + map_mirror(size, addr);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_lorom_offset (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size, uint32 offset)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = ((c - bank_s) & 0x7f) * 0x8000;
			Map[p] = ROM + offset + map_mirror(size, addr) - (i & 0x8000);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_hirom_offset (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint32 size, uint32 offset)
{
	uint32	c, i, p, addr;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			addr = (c - bank_s) << 16;
			Map[p] = ROM + offset + map_mirror(size, addr);
			BlockIsROM[p] = TRUE;
			BlockIsRAM[p] = FALSE;
		}
	}
}

void CMemory::map_space (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, uint8 *data)
{
	uint32	c, i, p;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			Map[p] = data;
			BlockIsROM[p] = FALSE;
			BlockIsRAM[p] = TRUE;
		}
	}
}

void CMemory::map_index (uint32 bank_s, uint32 bank_e, uint32 addr_s, uint32 addr_e, int index, int type)
{
	uint32	c, i, p;
	bool8	isROM, isRAM;

	isROM = ((type == MAP_TYPE_I_O) || (type == MAP_TYPE_RAM)) ? FALSE : TRUE;
	isRAM = ((type == MAP_TYPE_I_O) || (type == MAP_TYPE_ROM)) ? FALSE : TRUE;

	for (c = bank_s; c <= bank_e; c++)
	{
		for (i = addr_s; i <= addr_e; i += 0x1000)
		{
			p = (c << 4) | (i >> 12);
			Map[p] = (uint8 *) index;
			BlockIsROM[p] = isROM;
			BlockIsRAM[p] = isRAM;
		}
	}
}

void CMemory::map_System (void)
{
	// will be overwritten
	map_space(0x00, 0x3f, 0x0000, 0x1fff, RAM);
	map_index(0x00, 0x3f, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x00, 0x3f, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
	map_space(0x80, 0xbf, 0x0000, 0x1fff, RAM);
	map_index(0x80, 0xbf, 0x2000, 0x3fff, MAP_PPU, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x4000, 0x5fff, MAP_CPU, MAP_TYPE_I_O);
}

void CMemory::map_WRAM (void)
{
	// will overwrite others
	map_space(0x7e, 0x7e, 0x0000, 0xffff, RAM);
	map_space(0x7f, 0x7f, 0x0000, 0xffff, RAM + 0x10000);
}

void CMemory::map_LoROMSRAM (void)
{
	map_index(0x70, 0x7f, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	map_index(0xf0, 0xff, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::map_HiROMSRAM (void)
{
	map_index(0x20, 0x3f, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	map_index(0xa0, 0xbf, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
}

void CMemory::map_DSP (void)
{
	switch (DSP0.maptype)
	{
		case M_DSP1_LOROM_S:
			map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP1_LOROM_L:
			map_index(0x60, 0x6f, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xe0, 0xef, 0x0000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP1_HIROM:
			map_index(0x00, 0x1f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0x80, 0x9f, 0x6000, 0x7fff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP2_LOROM:
			map_index(0x20, 0x3f, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0x20, 0x3f, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x6000, 0x6fff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xbfff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP3_LOROM:
			map_index(0x20, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xa0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;

		case M_DSP4_LOROM:
			map_index(0x30, 0x3f, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			map_index(0xb0, 0xbf, 0x8000, 0xffff, MAP_DSP, MAP_TYPE_I_O);
			break;
	}
}

void CMemory::map_C4 (void)
{
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_C4, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_C4, MAP_TYPE_I_O);
}

void CMemory::map_OBC1 (void)
{
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_OBC_RAM, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_OBC_RAM, MAP_TYPE_I_O);
}

void CMemory::map_SetaRISC (void)
{
	map_index(0x00, 0x3f, 0x3000, 0x3fff, MAP_SETA_RISC, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x3000, 0x3fff, MAP_SETA_RISC, MAP_TYPE_I_O);
}

void CMemory::map_SetaDSP (void)
{
	// where does the SETA chip access, anyway?
	// please confirm this?
	map_index(0x68, 0x6f, 0x0000, 0x7fff, MAP_SETA_DSP, MAP_TYPE_RAM);
	// and this!
	map_index(0x60, 0x67, 0x0000, 0x3fff, MAP_SETA_DSP, MAP_TYPE_I_O);

	// ST-0010:
	// map_index(0x68, 0x6f, 0x0000, 0x0fff, MAP_SETA_DSP, ?);
}

void CMemory::map_WriteProtectROM (void)
{
	memmove((void *) WriteMap, (void *) Map, sizeof(Map));

	for (int c = 0; c < 0x1000; c++)
	{
		if (BlockIsROM[c])
			WriteMap[c] = (uint8 *) MAP_NONE;
	}
}

void CMemory::Map_Initialize (void)
{
	for (int c = 0; c < 0x1000; c++)
	{
		Map[c]      = (uint8 *) MAP_NONE;
		WriteMap[c] = (uint8 *) MAP_NONE;
		BlockIsROM[c] = FALSE;
		BlockIsRAM[c] = FALSE;
	}
}

void CMemory::Map_LoROMMap (void)
{
	printf("Map_LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	if (Settings.DSP)
		map_DSP();
	else
	if (Settings.C4)
		map_C4();
	else
	if (Settings.OBC1)
		map_OBC1();
	else
	if (Settings.SETA == ST_018)
		map_SetaRISC();

    map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_NoMAD1LoROMMap (void)
{
	printf("Map_NoMAD1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	map_index(0x70, 0x7f, 0x0000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	map_index(0xf0, 0xff, 0x0000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_JumboLoROMMap (void)
{
	// XXX: Which game uses this?
	printf("Map_JumboLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_lorom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_lorom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x400000, 0);
	map_lorom_offset(0xc0, 0xff, 0x0000, 0xffff, 0x400000, 0x200000);

	map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_ROM24MBSLoROMMap (void)
{
	// PCB: BSC-1A5M-01, BSC-1A7M-10
	printf("Map_ROM24MBSLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x100000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x100000, 0x200000);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, 0x100000, 0x100000);

	map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SRAM512KLoROMMap (void)
{
	printf("Map_SRAM512KLoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	map_space(0x70, 0x70, 0x0000, 0xffff, SRAM);
	map_space(0x71, 0x71, 0x0000, 0xffff, SRAM + 0x8000);
	map_space(0x72, 0x72, 0x0000, 0xffff, SRAM + 0x10000);
	map_space(0x73, 0x73, 0x0000, 0xffff, SRAM + 0x18000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SufamiTurboLoROMMap (void)
{
	printf("Map_SufamiTurboLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_lorom_offset(0x40, 0x5f, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_lorom_offset(0xc0, 0xdf, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);

	if (Multi.sramSizeA)
	{
		map_index(0x60, 0x63, 0x8000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
		map_index(0xe0, 0xe3, 0x8000, 0xffff, MAP_LOROM_SRAM, MAP_TYPE_RAM);
	}

	if (Multi.sramSizeB)
	{
		map_index(0x70, 0x73, 0x8000, 0xffff, MAP_LOROM_SRAM_B, MAP_TYPE_RAM);
		map_index(0xf0, 0xf3, 0x8000, 0xffff, MAP_LOROM_SRAM_B, MAP_TYPE_RAM);
	}

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SufamiTurboPseudoLoROMMap (void)
{
	// for combined images
	printf("Map_SufamiTurboPseudoLoROMMap\n");
	map_System();

	map_lorom_offset(0x00, 0x1f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0x20, 0x3f, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0x40, 0x5f, 0x8000, 0xffff, 0x100000, 0x200000);
	map_lorom_offset(0x80, 0x9f, 0x8000, 0xffff, 0x40000, 0);
	map_lorom_offset(0xa0, 0xbf, 0x8000, 0xffff, 0x100000, 0x100000);
	map_lorom_offset(0xc0, 0xdf, 0x8000, 0xffff, 0x100000, 0x200000);

	// I don't care :P
	map_space(0x60, 0x63, 0x8000, 0xffff, SRAM - 0x8000);
	map_space(0xe0, 0xe3, 0x8000, 0xffff, SRAM - 0x8000);
	map_space(0x70, 0x73, 0x8000, 0xffff, SRAM + 0x4000 - 0x8000);
	map_space(0xf0, 0xf3, 0x8000, 0xffff, SRAM + 0x4000 - 0x8000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SuperFXLoROMMap (void)
{
	printf("Map_SuperFXLoROMMap\n");
	map_System();

	// Replicate the first 2Mb of the ROM at ROM + 2MB such that each 32K
	// block is repeated twice in each 64K block.
	for (int c = 0; c < 64; c++)
	{
		memmove(&ROM[0x200000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
		memmove(&ROM[0x208000 + c * 0x10000], &ROM[c * 0x8000], 0x8000);
	}

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0);

	map_space(0x00, 0x3f, 0x6000, 0x7fff, SRAM - 0x6000);
	map_space(0x80, 0xbf, 0x6000, 0x7fff, SRAM - 0x6000);
	map_space(0x70, 0x70, 0x0000, 0xffff, SRAM);
	map_space(0x71, 0x71, 0x0000, 0xffff, SRAM + 0x10000);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SetaDSPLoROMMap (void)
{
	printf("Map_SetaDSPLoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x40, 0x7f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0xc0, 0xff, 0x8000, 0xffff, CalculatedSize);

	map_SetaDSP();

    map_LoROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SDD1LoROMMap (void)
{
	printf("Map_SDD1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0); // will be overwritten dynamically

	map_index(0x70, 0x7f, 0x0000, 0x7fff, MAP_LOROM_SRAM, MAP_TYPE_RAM);

	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SA1LoROMMap (void)
{
	printf("Map_SA1LoROMMap\n");
	map_System();

	map_lorom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_lorom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);

	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize, 0);

	map_space(0x00, 0x3f, 0x3000, 0x3fff, FillRAM);
	map_space(0x80, 0xbf, 0x3000, 0x3fff, FillRAM);
	map_index(0x00, 0x3f, 0x6000, 0x7fff, MAP_BWRAM, MAP_TYPE_I_O);
	map_index(0x80, 0xbf, 0x6000, 0x7fff, MAP_BWRAM, MAP_TYPE_I_O);

	for (int c = 0x40; c < 0x80; c++)
		map_space(c, c, 0x0000, 0xffff, SRAM + (c & 1) * 0x10000);

	map_WRAM();

	map_WriteProtectROM();

	// Now copy the map and correct it for the SA1 CPU.
	memmove((void *) SA1.Map, (void *) Map, sizeof(Map));
	memmove((void *) SA1.WriteMap, (void *) WriteMap, sizeof(WriteMap));

	// SA-1 Banks 00->3f and 80->bf
	for (int c = 0x000; c < 0x400; c += 0x10)
	{
		SA1.Map[c + 0] = SA1.Map[c + 0x800] = FillRAM + 0x3000;
		SA1.Map[c + 1] = SA1.Map[c + 0x801] = (uint8 *) MAP_NONE;
		SA1.WriteMap[c + 0] = SA1.WriteMap[c + 0x800] = FillRAM + 0x3000;
		SA1.WriteMap[c + 1] = SA1.WriteMap[c + 0x801] = (uint8 *) MAP_NONE;
	}

	// SA-1 Banks 60->6f
	for (int c = 0x600; c < 0x700; c++)
		SA1.Map[c] = SA1.WriteMap[c] = (uint8 *) MAP_BWRAM_BITMAP;

	BWRAM = SRAM;
}

void CMemory::Map_HiROMMap (void)
{
	printf("Map_HiROMMap\n");
	map_System();

	map_hirom(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize);
	map_hirom(0x80, 0xbf, 0x8000, 0xffff, CalculatedSize);
	map_hirom(0xc0, 0xff, 0x0000, 0xffff, CalculatedSize);

	if (Settings.DSP)
		map_DSP();

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_ExtendedHiROMMap (void)
{
	printf("Map_ExtendedHiROMMap\n");
	map_System();

	map_hirom_offset(0x00, 0x3f, 0x8000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_hirom_offset(0x40, 0x7f, 0x0000, 0xffff, CalculatedSize - 0x400000, 0x400000);
	map_hirom_offset(0x80, 0xbf, 0x8000, 0xffff, 0x400000, 0);
	map_hirom_offset(0xc0, 0xff, 0x0000, 0xffff, 0x400000, 0);

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SameGameHiROMMap (void)
{
	printf("Map_SameGameHiROMMap\n");
	map_System();

	map_hirom_offset(0x00, 0x1f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0x20, 0x3f, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0x40, 0x5f, 0x0000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0x60, 0x7f, 0x0000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0x80, 0x9f, 0x8000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0xa0, 0xbf, 0x8000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);
	map_hirom_offset(0xc0, 0xdf, 0x0000, 0xffff, Multi.cartSizeA, Multi.cartOffsetA);
	map_hirom_offset(0xe0, 0xff, 0x0000, 0xffff, Multi.cartSizeB, Multi.cartOffsetB);

	map_HiROMSRAM();
	map_WRAM();

	map_WriteProtectROM();
}

void CMemory::Map_SPC7110HiROMMap (void)
{
	printf("Map_SPC7110HiROMMap\n");
	map_System();

	map_index(0x00, 0x00, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	map_hirom(0x00, 0x0f, 0x8000, 0xffff, CalculatedSize);	
	map_index(0x30, 0x30, 0x6000, 0x7fff, MAP_HIROM_SRAM, MAP_TYPE_RAM);
	map_index(0x50, 0x50, 0x0000, 0xffff, MAP_SPC7110_DRAM, MAP_TYPE_ROM);
	map_hirom(0x80, 0x8f, 0x8000, 0xffff, CalculatedSize);
	map_hirom_offset(0xc0, 0xcf, 0x0000, 0xffff, CalculatedSize, 0);
	map_index(0xd0, 0xff, 0x0000, 0xffff, MAP_SPC7110_ROM,  MAP_TYPE_ROM);

	map_WRAM();

	map_WriteProtectROM();
}

// checksum

uint16 CMemory::checksum_calc_sum (uint8 *data, uint32 length)
{
	uint16	sum = 0;

	for (uint32 i = 0; i < length; i++)
		sum += data[i];

	return (sum);
}

uint16 CMemory::checksum_mirror_sum (uint8 *start, uint32 &length, uint32 mask)
{
	// from NSRT
	while (!(length & mask))
		mask >>= 1;

	uint16	part1 = checksum_calc_sum(start, mask);
	uint16	part2 = 0;

	uint32	next_length = length - mask;
	if (next_length)
	{
		part2 = checksum_mirror_sum(start + mask, next_length, mask >> 1);

		while (next_length < mask)
		{
			next_length += next_length;
			part2 += part2;
		}

		length = mask + mask;
	}

	return (part1 + part2);
}

void CMemory::Checksum_Calculate (void)
{
	// from NSRT
	uint16	sum = 0;

	if (Settings.BS && !Settings.BSXItself)
		sum = checksum_calc_sum(ROM, CalculatedSize) - checksum_calc_sum(ROM + (HiROM ? 0xffb0 : 0x7fb0), 48);
	else
	if (Settings.SPC7110)
	{
		sum = checksum_calc_sum(ROM, CalculatedSize);
		if (CalculatedSize == 0x300000)
			sum += sum;
	}
	else
	{
		if (CalculatedSize & 0x7fff)
			sum = checksum_calc_sum(ROM, CalculatedSize);
		else
		{
			uint32	length = CalculatedSize;
			sum = checksum_mirror_sum(ROM, length);
		}
	}

	CalculatedChecksum = sum;
}

// information

const char * CMemory::MapType (void)
{
	return (HiROM ? ((ExtendedFormat != NOPE) ? "ExHiROM": "HiROM") : "LoROM");
}

const char * CMemory::StaticRAMSize (void)
{
	static char	str[20];

	if (SRAMSize > 16)
		strcpy(str, "Corrupt");
	else
		sprintf(str, "%dKbits", 8 * (SRAMMask + 1) / 1024);

	return (str);
}

const char * CMemory::Size (void)
{
	static char	str[20];

	if (Multi.cartType == 4)
		strcpy(str, "N/A");
	else
	if (ROMSize < 7 || ROMSize - 7 > 23)
		strcpy(str, "Corrupt");
	else
		sprintf(str, "%dMbits", 1 << (ROMSize - 7));

	return (str);
}

const char * CMemory::Revision (void)
{
	static char	str[20];

	sprintf(str, "1.%d", HiROM ? ((ExtendedFormat != NOPE) ? ROM[0x40ffdb] : ROM[0xffdb]) : ROM[0x7fdb]);

	return (str);
}

const char * CMemory::KartContents (void)
{
	static char			str[64];
	static const char	*contents[3] = { "ROM", "ROM+RAM", "ROM+RAM+BAT" };

	char	chip[16];

	if (ROMType == 0 && !Settings.BS)
		return ("ROM");

	if (Settings.BS)
		strcpy(chip, "+BS");
	else
	if (Settings.SuperFX)
		strcpy(chip, "+Super FX");
	else
	if (Settings.SDD1)
		strcpy(chip, "+S-DD1");
	else
	if (Settings.OBC1)
		strcpy(chip, "+OBC1");
	else
	if (Settings.SA1)
		strcpy(chip, "+SA-1");
	else
	if (Settings.SPC7110RTC)
		strcpy(chip, "+SPC7110+RTC");
	else
	if (Settings.SPC7110)
		strcpy(chip, "+SPC7110");
	else
	if (Settings.SRTC)
		strcpy(chip, "+S-RTC");
	else
	if (Settings.C4)
		strcpy(chip, "+C4");
	else
	if (Settings.SETA == ST_010)
		strcpy(chip, "+ST-010");
	else
	if (Settings.SETA == ST_011)
		strcpy(chip, "+ST-011");
	else
	if (Settings.SETA == ST_018)
		strcpy(chip, "+ST-018");
	else
	if (Settings.DSP)
		sprintf(chip, "+DSP-%d", Settings.DSP);
	else
		strcpy(chip, "");

	sprintf(str, "%s%s", contents[(ROMType & 0xf) % 3], chip);

	return (str);
}

const char * CMemory::Country (void)
{
	switch (ROMRegion)
	{
		case 0:		return("Japan");
		case 1:		return("USA and Canada");
		case 2:		return("Oceania, Europe and Asia");
		case 3:		return("Sweden");
		case 4:		return("Finland");
		case 5:		return("Denmark");
		case 6:		return("France");
		case 7:		return("Holland");
		case 8:		return("Spain");
		case 9:		return("Germany, Austria and Switzerland");
		case 10:	return("Italy");
		case 11:	return("Hong Kong and China");
		case 12:	return("Indonesia");
		case 13:	return("South Korea");
		default:	return("Unknown");
	}
}

const char * CMemory::PublishingCompany (void)
{
	if (CompanyId >= (int) (sizeof(nintendo_licensees) / sizeof(nintendo_licensees[0])) || CompanyId < 0)
		return ("Unknown");

	if (nintendo_licensees[CompanyId] == NULL)
		return ("Unknown");

	return (nintendo_licensees[CompanyId]);
}

void CMemory::MakeRomInfoText (char *romtext)
{
	char	temp[256];

	romtext[0] = 0;

	sprintf(temp,   "            Cart Name: %s", ROMName);
	strcat(romtext, temp);
	sprintf(temp, "\n            Game Code: %s", ROMId);
	strcat(romtext, temp);
	sprintf(temp, "\n             Contents: %s", KartContents());
	strcat(romtext, temp);
	sprintf(temp, "\n                  Map: %s", MapType());
	strcat(romtext, temp);
	sprintf(temp, "\n                Speed: 0x%02X (%s)", ROMSpeed, (ROMSpeed & 0x10) ? "FastROM" : "SlowROM");
	strcat(romtext, temp);
	sprintf(temp, "\n                 Type: 0x%02X", ROMType);
	strcat(romtext, temp);
	sprintf(temp, "\n    Size (calculated): %dMbits", CalculatedSize / 0x20000);
	strcat(romtext, temp);
	sprintf(temp, "\n        Size (header): %s", Size());
	strcat(romtext, temp);
	sprintf(temp, "\n            SRAM size: %s", StaticRAMSize());
	strcat(romtext, temp);
	sprintf(temp, "\nChecksum (calculated): 0x%04X", CalculatedChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n    Checksum (header): 0x%04X", ROMChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n  Complement (header): 0x%04X", ROMComplementChecksum);
	strcat(romtext, temp);
	sprintf(temp, "\n         Video Output: %s", (ROMRegion > 12 || ROMRegion < 2) ? "NTSC 60Hz" : "PAL 50Hz");
	strcat(romtext, temp);
	sprintf(temp, "\n             Revision: %s", Revision());
	strcat(romtext, temp);
	sprintf(temp, "\n             Licensee: %s", PublishingCompany());
	strcat(romtext, temp);
	sprintf(temp, "\n               Region: %s", Country());
	strcat(romtext, temp);
	sprintf(temp, "\n                CRC32: 0x%08X", ROMCRC32);
	strcat(romtext, temp);
}

// hack

bool8 CMemory::match_na (const char *str)
{
	return (strcmp(ROMName, str) == 0);
}

bool8 CMemory::match_nn (const char *str)
{
	return (strncmp(ROMName, str, strlen(str)) == 0);
}

bool8 CMemory::match_nc (const char *str)
{
	return (strncasecmp(ROMName, str, strlen(str)) == 0);
}

bool8 CMemory::match_id (const char *str)
{
	return (strncmp(ROMId, str, strlen(str)) == 0);
}

void CMemory::ApplyROMFixes (void)
{
	Settings.BlockInvalidVRAMAccess = Settings.BlockInvalidVRAMAccessMaster;

	//// Warnings

	// Reject strange hacked games
	if ((ROMCRC32 == 0x6810aa95) ||
		(ROMCRC32 == 0x340f23e5) ||
		(ROMCRC32 == 0x77fd806a) ||
		(match_nn("HIGHWAY BATTLE 2")) ||
		(match_na("FX SKIING NINTENDO 96") && (ROM[0x7fda] == 0)) ||
		(match_nn("HONKAKUHA IGO GOSEI")   && (ROM[0xffd5] != 0x31)))
	{
		Settings.DisplayColor = BUILD_PIXEL(31, 0, 0);
		SET_UI_COLOR(255, 0, 0);
	}

	//// APU timing hacks :(

	Timings.APUSpeedup = 0;
	Timings.APUAllowTimeOverflow = FALSE;

	if (!Settings.DisableGameSpecificHacks)
	{
		if (match_id("AVCJ"))                                      // Rendering Ranger R2
			Timings.APUSpeedup = 4;

		if (match_na("GAIA GENSOUKI 1 JPN")                     || // Gaia Gensouki
			match_id("JG  ")                                    || // Illusion of Gaia
			match_id("CQ  ")                                    || // Stunt Race FX
			match_na("SOULBLADER - 1")                          || // Soul Blader
			match_na("SOULBLAZER - 1 USA")                      || // Soul Blazer
			match_na("SLAP STICK 1 JPN")                        || // Slap Stick
			match_id("E9 ")                                     || // Robotrek
			match_nn("ACTRAISER")                               || // Actraiser
			match_nn("ActRaiser-2")                             || // Actraiser 2
			match_id("AQT")                                     || // Tenchi Souzou, Terranigma
			match_id("ATV")                                     || // Tales of Phantasia
			match_id("ARF")                                     || // Star Ocean
			match_id("APR")                                     || // Zen-Nippon Pro Wrestling 2 - 3-4 Budoukan
			match_id("A4B")                                     || // Super Bomberman 4
			match_id("Y7 ")                                     || // U.F.O. Kamen Yakisoban - Present Ban
			match_id("Y9 ")                                     || // U.F.O. Kamen Yakisoban - Shihan Ban
			match_id("APB")                                     || // Super Bomberman - Panic Bomber W
			match_na("DARK KINGDOM")                            || // Dark Kingdom
			match_na("ZAN3 SFC")                                || // Zan III Spirits
			match_na("HIOUDEN")                                 || // Hiouden - Mamono-tachi Tono Chikai
			match_na("\xC3\xDD\xBC\xC9\xB3\xC0")                || // Tenshi no Uta
			match_na("FORTUNE QUEST")                           || // Fortune Quest - Dice wo Korogase
			match_na("FISHING TO BASSING")                      || // Shimono Masaki no Fishing To Bassing
			match_na("OHMONO BLACKBASS")                        || // Oomono Black Bass Fishing - Jinzouko Hen
			match_na("MASTERS")                                 || // Harukanaru Augusta 2 - Masters
			match_na("SFC \xB6\xD2\xDD\xD7\xB2\xC0\xDE\xB0")    || // Kamen Rider
			match_na("ZENKI TENCHIMEIDOU")					    || // Kishin Douji Zenki - Tenchi Meidou
			match_nn("TokyoDome '95Battle 7")                   || // Shin Nippon Pro Wrestling Kounin '95 - Tokyo Dome Battle 7
			match_nn("SWORD WORLD SFC")                         || // Sword World SFC/2
			match_nn("LETs PACHINKO(")                          || // BS Lets Pachinko Nante Gindama 1/2/3/4
			match_nn("THE FISHING MASTER")                      || // Mark Davis The Fishing Master
			match_nn("Parlor")                                  || // Parlor mini/2/3/4/5/6/7, Parlor Parlor!/2/3/4/5
			match_na("HEIWA Parlor!Mini8")                      || // Parlor mini 8
			match_nn("SANKYO Fever! \xCC\xA8\xB0\xCA\xDE\xB0!"))   // SANKYO Fever! Fever!
			Timings.APUSpeedup = 1;

		if (match_na ("EARTHWORM JIM 2")						|| // Earthworm Jim 2
			match_na ("NBA Hangtime")							|| // NBA Hang Time
			match_na ("MSPACMAN")								|| // Ms Pacman
			match_na ("THE MASK")								|| // The Mask
			match_na ("PRIMAL RAGE")							|| // Primal Rage
			match_na ("DOOM TROOPERS"))							   // Doom Troopers
			Timings.APUAllowTimeOverflow = TRUE;
	}

	S9xAPUTimingSetSpeedup(Timings.APUSpeedup);
	S9xAPUAllowTimeOverflow(Timings.APUAllowTimeOverflow);

	//// Other timing hacks :(

	Timings.HDMAStart   = SNES_HDMA_START_HC + Settings.HDMATimingHack - 100;
	Timings.HBlankStart = SNES_HBLANK_START_HC + Timings.HDMAStart - SNES_HDMA_START_HC;
	Timings.IRQTriggerCycles = 10;

	if (!Settings.DisableGameSpecificHacks)
	{
		// The delay to sync CPU and DMA which Snes9x cannot emulate.
		// Some games need really severe delay timing...
		if (match_na("BATTLE GRANDPRIX")) // Battle Grandprix
		{
			Timings.DMACPUSync = 20;
			printf("DMA sync: %d\n", Timings.DMACPUSync);
		}
	}

	if (!Settings.DisableGameSpecificHacks)
	{
		// An infinite loop reads $4212 and waits V-blank end, whereas VIRQ is set V=0.
		// If Snes9x succeeds to escape from the loop before jumping into the IRQ handler, the game goes further.
		// If Snes9x jumps into the IRQ handler before escaping from the loop,
		// Snes9x cannot escape from the loop permanently because the RTI is in the next V-blank.
		if (match_na("Aero the AcroBat 2"))
		{
			Timings.IRQPendCount = 2;
			printf("IRQ count hack: %d\n", Timings.IRQPendCount);
		}
	}

	if (!Settings.DisableGameSpecificHacks)
	{
		// XXX: What's happening?
		if (match_na("X-MEN")) // Spider-Man and the X-Men
		{
			Settings.BlockInvalidVRAMAccess = FALSE;
			printf("Invalid VRAM access hack\n");
		}
	}

	//// SRAM initial value

	if (!Settings.DisableGameSpecificHacks)
	{
		if (match_na("HITOMI3"))
		{
			SRAMSize = 1;
			SRAMMask = ((1 << (SRAMSize + 3)) * 128) - 1;
		}

		// SRAM value fixes
		if (match_na("SUPER DRIFT OUT")      || // Super Drift Out
			match_na("SATAN IS OUR FATHER!") ||
			match_na("goemon 4"))               // Ganbare Goemon Kirakira Douchuu
			SNESGameFixes.SRAMInitialValue = 0x00;

		// Additional game fixes by sanmaiwashi ...
		// XXX: unnecessary?
		if (match_na("SFX \xC5\xB2\xC4\xB6\xDE\xDD\xC0\xDE\xD1\xD3\xC9\xB6\xDE\xC0\xD8 1")) // SD Gundam Gaiden - Knight Gundam Monogatari
			SNESGameFixes.SRAMInitialValue = 0x6b;

		// others: BS and ST-01x games are 0x00.
	}

	//// OAM hacks :(

	if (!Settings.DisableGameSpecificHacks)
	{
		// OAM hacks because we don't fully understand the behavior of the SNES.
		// Totally wacky display in 2P mode...
		// seems to need a disproven behavior, so we're definitely overlooking some other bug?
		if (match_nn("UNIRACERS")) // Uniracers
		{
			SNESGameFixes.Uniracers = TRUE;
			printf("Applied Uniracers hack.\n");
		}
	}
}

// UPS % IPS

static uint32 ReadUPSPointer (const uint8 *data, unsigned &addr, unsigned size)
{
	uint32 offset = 0, shift = 1;
	while(addr < size) {
		uint8 x = data[addr++];
		offset += (x & 0x7f) * shift;
		if(x & 0x80) break;
		shift <<= 7;
		offset += shift;
	}
	return offset;
}

//NOTE: UPS patches are *never* created against a headered ROM!
//this is per the UPS file specification. however, do note that it is
//technically possible for a non-compliant patcher to ignore this requirement.
//therefore, it is *imperative* that no emulator support such patches.
//thusly, we ignore the "long offset" parameter below. failure to do so would
//completely invalidate the purpose of UPS; which is to avoid header vs
//no-header patching errors that result in IPS patches having a 50/50 chance of
//being applied correctly.

static bool8 ReadUPSPatch (Reader *r, long, int32 &rom_size)
{
	//Reader lacks size() and rewind(), so we need to read in the file to get its size
	uint8 *data = new uint8[8 * 1024 * 1024];  //allocate a lot of memory, better safe than sorry ...
	uint32 size = 0;
	while(true) {
		int value = r->get_char();
		if(value == EOF) break;
		data[size++] = value;
		if(size >= 8 * 1024 * 1024) {
			//prevent buffer overflow: SNES-made UPS patches should never be this big anyway ...
			delete[] data;
			return false;
		}
	}

	//4-byte header + 1-byte input size + 1-byte output size + 4-byte patch CRC32 + 4-byte unpatched CRC32 + 4-byte patched CRC32
	if(size < 18) { delete[] data; return false; }  //patch is too small

	uint32 addr = 0;
	if(data[addr++] != 'U') { delete[] data; return false; }  //patch has an invalid header
	if(data[addr++] != 'P') { delete[] data; return false; }  //...
	if(data[addr++] != 'S') { delete[] data; return false; }  //...
	if(data[addr++] != '1') { delete[] data; return false; }  //...

	uint32 patch_crc32 = caCRC32(data, size - 4);  //don't include patch CRC32 itself in CRC32 calculation
	uint32 rom_crc32 = caCRC32(Memory.ROM, rom_size);
	uint32 px_crc32 = (data[size - 12] << 0) + (data[size - 11] << 8) + (data[size - 10] << 16) + (data[size -  9] << 24);
	uint32 py_crc32 = (data[size -  8] << 0) + (data[size -  7] << 8) + (data[size -  6] << 16) + (data[size -  5] << 24);
	uint32 pp_crc32 = (data[size -  4] << 0) + (data[size -  3] << 8) + (data[size -  2] << 16) + (data[size -  1] << 24);
	if(patch_crc32 != pp_crc32) { delete[] data; return false; }  //patch is corrupted
	if((rom_crc32 != px_crc32) && (rom_crc32 != py_crc32)) { delete[] data; return false; }  //patch is for a different ROM

	uint32 px_size = ReadUPSPointer(data, addr, size);
	uint32 py_size = ReadUPSPointer(data, addr, size);
	uint32 out_size = ((uint32) rom_size == px_size) ? py_size : px_size;
	if(out_size > CMemory::MAX_ROM_SIZE) { delete[] data; return false; }  //applying this patch will overflow Memory.ROM buffer

	//fill expanded area with 0x00s; so that XORing works as expected below.
	//note that this is needed (and works) whether output ROM is larger or smaller than pre-patched ROM
	for(unsigned i = min((uint32) rom_size, out_size); i < max((uint32) rom_size, out_size); i++) {
		Memory.ROM[i] = 0x00;
	}

	uint32 relative = 0;
	while(addr < size - 12) {
		relative += ReadUPSPointer(data, addr, size);
		while(addr < size - 12) {
			uint8 x = data[addr++];
			Memory.ROM[relative++] ^= x;
			if(!x) break;
		}
	}

	rom_size = out_size;
	delete[] data;

	uint32 out_crc32 = caCRC32(Memory.ROM, rom_size);
	if(((rom_crc32 == px_crc32) && (out_crc32 == py_crc32))
	|| ((rom_crc32 == py_crc32) && (out_crc32 == px_crc32))
	) {
		return true;
	} else {
		//technically, reaching here means that patching has failed.
		//we should return false, but unfortunately Memory.ROM has already
		//been modified above and cannot be undone. to do this properly, we
		//would need to make a copy of Memory.ROM, apply the patch, and then
		//copy that back to Memory.ROM.
		//
		//however, the only way for this case to happen is if the UPS patch file
		//itself is corrupted, which should be detected by the patch CRC32 check
		//above anyway. errors due to the wrong ROM or patch file being used are
		//already caught above.
		fprintf(stderr, "WARNING: UPS patching appears to have failed.\nGame may not be playable.\n");
		return true;
	}
}

static long ReadInt (Reader *r, unsigned nbytes)
{
	long	v = 0;

	while (nbytes--)
	{
		int	c = r->get_char();
		if (c == EOF)
			return (-1);
		v = (v << 8) | (c & 0xFF);
	}

	return (v);
}

static bool8 ReadIPSPatch (Reader *r, long offset, int32 &rom_size)
{
	const int32	IPS_EOF = 0x00454F46l;
	int32		ofs;
	char		fname[6];

	fname[5] = 0;
	for (int i = 0; i < 5; i++)
	{
		int	c = r->get_char();
		if (c == EOF)
			return (0);
		fname[i] = (char) c;
	}

	if (strncmp(fname, "PATCH", 5))
		return (0);

	for (;;)
	{
		long	len, rlen;
		int		rchar;

		ofs = ReadInt(r, 3);
		if (ofs == -1)
			return (0);

		if (ofs == IPS_EOF)
			break;

		ofs -= offset;

		len = ReadInt(r, 2);
		if (len == -1)
			return (0);

		if (len)
		{
			if (ofs + len > CMemory::MAX_ROM_SIZE)
				return (0);

			while (len--)
			{
				rchar = r->get_char();
				if (rchar == EOF)
					return (0);
				Memory.ROM[ofs++] = (uint8) rchar;
			}

			if (ofs > rom_size)
				rom_size = ofs;
		}
		else
		{
			rlen = ReadInt(r, 2);
			if (rlen == -1)
				return (0);

			rchar = r->get_char();
			if (rchar == EOF)
				return (0);

			if (ofs + rlen > CMemory::MAX_ROM_SIZE)
				return (0);

			while (rlen--)
				Memory.ROM[ofs++] = (uint8) rchar;

			if (ofs > rom_size)
				rom_size = ofs;
		}
	}

	ofs = ReadInt(r, 3);
	if (ofs != -1 && ofs - offset < rom_size)
		rom_size = ofs - offset;

	return (1);
}

#ifdef UNZIP_SUPPORT
static int unzFindExtension (unzFile &file, const char *ext, bool restart, bool print)
{
	unz_file_info	info;
	int				port, l = strlen(ext);

	if (restart)
		port = unzGoToFirstFile(file);
	else
		port = unzGoToNextFile(file);

	while (port == UNZ_OK)
	{
		int		len;
		char	name[132];

		unzGetCurrentFileInfo(file, &info, name, 128, NULL, 0, NULL, 0);
		len = strlen(name);

		if (len >= l + 1 && name[len - l - 1] == '.' && strcasecmp(name + len - l, ext) == 0 && unzOpenCurrentFile(file) == UNZ_OK)
		{
			if (print)
				printf("Using IPS or UPS patch %s", name);

			return (port);
		}

		port = unzGoToNextFile(file);
	}

	return (port);
}
#endif

void CMemory::CheckForAnyPatch (const char *rom_filename, bool8 header, int32 &rom_size)
{
	if (Settings.NoPatch)
		return;

	STREAM		patch_file  = NULL;
	uint32		i;
	long		offset = header ? 512 : 0;
	int			ret;
	bool		flag;
	char		dir[_MAX_DIR + 1], drive[_MAX_DRIVE + 1], name[_MAX_FNAME + 1], ext[_MAX_EXT + 1], ips[_MAX_EXT + 3], fname[PATH_MAX + 1];
	const char	*n;

	// UPS

	_splitpath(rom_filename, drive, dir, name, ext);
	_makepath(fname, drive, dir, name, "ups");

	if ((patch_file = OPEN_STREAM(fname, "rb")) != NULL)
	{
		printf("Using UPS patch %s", fname);

		ret = ReadUPSPatch(new fReader(patch_file), 0, rom_size);
		CLOSE_STREAM(patch_file);

		if (ret)
		{
			printf("!\n");
			return;
		}
		else
			printf(" failed!\n");
	}

#ifdef UNZIP_SUPPORT
	if (!strcasecmp(ext, "zip") || !strcasecmp(ext, ".zip"))
	{
		unzFile	file = unzOpen(rom_filename);
		if (file)
		{
			int	port = unzFindExtension(file, "ups");
			if (port == UNZ_OK)
			{
				printf(" in %s", rom_filename);

				ret = ReadUPSPatch(new unzReader(file), offset, rom_size);
				unzCloseCurrentFile(file);

				if (ret)
					printf("!\n");
				else
					printf(" failed!\n");
			}
		}
	}
#endif

	n = S9xGetFilename(".ups", IPS_DIR);

	if ((patch_file = OPEN_STREAM(n, "rb")) != NULL)
	{
		printf("Using UPS patch %s", n);

		ret = ReadUPSPatch(new fReader(patch_file), 0, rom_size);
		CLOSE_STREAM(patch_file);

		if (ret)
		{
			printf("!\n");
			return;
		}
		else
			printf(" failed!\n");
	}

	// IPS

	_splitpath(rom_filename, drive, dir, name, ext);
	_makepath(fname, drive, dir, name, "ips");

	if ((patch_file = OPEN_STREAM(fname, "rb")) != NULL)
	{
		printf("Using IPS patch %s", fname);

		ret = ReadIPSPatch(new fReader(patch_file), offset, rom_size);
		CLOSE_STREAM(patch_file);

		if (ret)
		{
			printf("!\n");
			return;
		}
		else
			printf(" failed!\n");
	}

	if (_MAX_EXT > 6)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, 8, "%03d.ips", i);
			_makepath(fname, drive, dir, name, ips);

			if (!(patch_file = OPEN_STREAM(fname, "rb")))
				break;

			printf("Using IPS patch %s", fname);

			ret = ReadIPSPatch(new fReader(patch_file), offset, rom_size);
			CLOSE_STREAM(patch_file);

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i < 1000);

		if (flag)
			return;
	}

	if (_MAX_EXT > 3)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, _MAX_EXT + 2, "ips%d", i);
			if (strlen(ips) > _MAX_EXT)
				break;
			_makepath(fname, drive, dir, name, ips);

			if (!(patch_file = OPEN_STREAM(fname, "rb")))
				break;

			printf("Using IPS patch %s", fname);

			ret = ReadIPSPatch(new fReader(patch_file), offset, rom_size);
			CLOSE_STREAM(patch_file);

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i != 0);

		if (flag)
			return;
	}

	if (_MAX_EXT > 2)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, 4, "ip%d", i);
			_makepath(fname, drive, dir, name, ips);

			if (!(patch_file = OPEN_STREAM(fname, "rb")))
				break;

			printf("Using IPS patch %s", fname);

			ret = ReadIPSPatch(new fReader(patch_file), offset, rom_size);
			CLOSE_STREAM(patch_file);

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i < 10);

		if (flag)
			return;
	}

#ifdef UNZIP_SUPPORT
	if (!strcasecmp(ext, "zip") || !strcasecmp(ext, ".zip"))
	{
		unzFile	file = unzOpen(rom_filename);
		if (file)
		{
			int	port = unzFindExtension(file, "ips");
			while (port == UNZ_OK)
			{
				printf(" in %s", rom_filename);

				ret = ReadIPSPatch(new unzReader(file), offset, rom_size);
				unzCloseCurrentFile(file);

				if (ret)
				{
					printf("!\n");
					flag = true;
				}
				else
					printf(" failed!\n");

				port = unzFindExtension(file, "ips", false);
			}

			if (!flag)
			{
				i = 0;

				do
				{
					snprintf(ips, 8, "%03d.ips", i);

					if (unzFindExtension(file, ips) != UNZ_OK)
						break;

					printf(" in %s", rom_filename);

					ret = ReadIPSPatch(new unzReader(file), offset, rom_size);
					unzCloseCurrentFile(file);

					if (ret)
					{
						printf("!\n");
						flag = true;
					}
					else
					{
						printf(" failed!\n");
						break;
					}

					if (unzFindExtension(file, ips, false, false) == UNZ_OK)
						printf("WARNING: Ignoring extra .%s files!\n", ips);
				} while (++i < 1000);
			}

			if (!flag)
			{
				i = 0;

				do
				{
					snprintf(ips, _MAX_EXT + 2, "ips%d", i);
					if (strlen(ips) > _MAX_EXT)
						break;

					if (unzFindExtension(file, ips) != UNZ_OK)
						break;

					printf(" in %s", rom_filename);

					ret = ReadIPSPatch(new unzReader(file), offset, rom_size);
					unzCloseCurrentFile(file);

					if (ret)
					{
						printf("!\n");
						flag = true;
					}
					else
					{
						printf(" failed!\n");
						break;
					}

					if (unzFindExtension(file, ips, false, false) == UNZ_OK)
						printf("WARNING: Ignoring extra .%s files!\n", ips);
				} while (++i != 0);
			}

			if (!flag)
			{
				i = 0;

				do
				{
					snprintf(ips, 4, "ip%d", i);

					if (unzFindExtension(file, ips) != UNZ_OK)
						break;

					printf(" in %s", rom_filename);

					ret = ReadIPSPatch(new unzReader(file), offset, rom_size);
					unzCloseCurrentFile(file);

					if (ret)
					{
						printf("!\n");
						flag = true;
					}
					else
					{
						printf(" failed!\n");
						break;
					}

					if (unzFindExtension(file, ips, false, false) == UNZ_OK)
						printf("WARNING: Ignoring extra .%s files!\n", ips);
				} while (++i < 10);
			}

			assert(unzClose(file) == UNZ_OK);

			if (flag)
				return;
		}
	}
#endif

	n = S9xGetFilename(".ips", IPS_DIR);

	if ((patch_file = OPEN_STREAM(n, "rb")) != NULL)
	{
		printf("Using IPS patch %s", n);

		ret = ReadIPSPatch(new fReader(patch_file), offset, rom_size);
		CLOSE_STREAM(patch_file);

		if (ret)
		{
			printf("!\n");
			return;
		}
		else
			printf(" failed!\n");
	}

	if (_MAX_EXT > 6)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, 9, ".%03d.ips", i);
			n = S9xGetFilename(ips, IPS_DIR);

			if (!(patch_file = OPEN_STREAM(n, "rb")))
				break;

			printf("Using IPS patch %s", n);

			ret = ReadIPSPatch(new fReader(patch_file), offset, rom_size);
			CLOSE_STREAM(patch_file);

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i < 1000);

		if (flag)
			return;
	}

	if (_MAX_EXT > 3)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, _MAX_EXT + 3, ".ips%d", i);
			if (strlen(ips) > _MAX_EXT + 1)
				break;
			n = S9xGetFilename(ips, IPS_DIR);

			if (!(patch_file = OPEN_STREAM(n, "rb")))
				break;

			printf("Using IPS patch %s", n);

			ret = ReadIPSPatch(new fReader(patch_file), offset, rom_size);
			CLOSE_STREAM(patch_file);

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i != 0);

		if (flag)
			return;
	}

	if (_MAX_EXT > 2)
	{
		i = 0;
		flag = false;

		do
		{
			snprintf(ips, 5, ".ip%d", i);
			n = S9xGetFilename(ips, IPS_DIR);

			if (!(patch_file = OPEN_STREAM(n, "rb")))
				break;

			printf("Using IPS patch %s", n);

			ret = ReadIPSPatch(new fReader(patch_file), offset, rom_size);
			CLOSE_STREAM(patch_file);

			if (ret)
			{
				printf("!\n");
				flag = true;
			}
			else
			{
				printf(" failed!\n");
				break;
			}
		} while (++i < 10);

		if (flag)
			return;
	}
}
