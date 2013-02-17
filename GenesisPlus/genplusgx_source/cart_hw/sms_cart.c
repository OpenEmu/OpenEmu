/****************************************************************************
 *  Genesis Plus
 *  SG-1000, Master System & Game Gear cartridge hardware support
 *
 *  Copyright (C) 2007-2012  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"
#include "eeprom_93c.h"
#include "terebi_oekaki.h"

#define MAPPER_NONE        (0x00)
#define MAPPER_TEREBI      (0x01)
#define MAPPER_RAM_8K_EXT1 (0x02)
#define MAPPER_RAM_8K_EXT2 (0x03)
#define MAPPER_SEGA        (0x10)
#define MAPPER_SEGA_X      (0x11)
#define MAPPER_93C46       (0x12)
#define MAPPER_CODIES      (0x13)
#define MAPPER_MULTI       (0x14)
#define MAPPER_KOREA       (0x15)
#define MAPPER_KOREA_16K   (0x16)
#define MAPPER_KOREA_8K    (0x20)
#define MAPPER_MSX         (0x21)
#define MAPPER_MSX_NEMESIS (0x22)

#define GAME_DATABASE_CNT (211)

typedef struct
{
  uint32 crc;
  uint8 g_3d;
  uint8 fm;
  uint8 peripheral;
  uint8 mapper;
  uint8 system;
  uint8 region;
} rominfo_t;

typedef struct
{
  uint8 fcr[4];
  uint8 mapper;
  uint8 pages;
} romhw_t;

static const rominfo_t game_list[GAME_DATABASE_CNT] =
{
  /* program requiring Mega Drive VDP (Mode 5) */
  {0x47FA618D, 0, 1, SYSTEM_MS_GAMEPAD, MAPPER_SEGA, SYSTEM_PBC,  REGION_USA}, /* Charles MacDonald's Mode 5 Demo Program */

  /* game requiring SEGA mapper */
  {0xFF67359B, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_SEGA, SYSTEM_SMS2, REGION_USA}, /* DataStorm (homebrew) */

  /* games requiring 315-5124 VDP (Mark-III, Master System I) */
  {0x32759751, 0, 1, SYSTEM_MS_GAMEPAD, MAPPER_SEGA, SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Y's (J) */

  /* games requiring Sega 315-5235 mapper without bank shifting */
  {0x23BAC434, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA_X, SYSTEM_GG, REGION_USA}, /* Shining Force Gaiden - Final Conflict (JP) [T-Eng] */

  /* games using various Korean mappers */
  {0x17AB6883, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_NONE,        SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* FA Tetris (KR) */
  {0x61E8806F, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_NONE,        SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Flash Point (KR) */
  {0x445525E2, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX,         SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Penguin Adventure (KR) */
  {0x83F0EEDE, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX,         SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Street Master (KR) */
  {0xA05258F5, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX,         SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Won-Si-In (KR) */
  {0x06965ED9, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX,          SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* F-1 Spirit - The way to Formula-1 (KR) */
  {0x77EFE84A, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX,         SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Cyborg Z (KR) */
  {0xF89AF3CC, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX,         SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Knightmare II - The Maze of Galious (KR) */
  {0x9195C34C, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX,         SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Super Boy 3 (KR) */
  {0xE316C06D, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX_NEMESIS, SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Nemesis (KR) */
  {0x0A77FA5E, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MSX,         SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Nemesis 2 (KR) */
  {0xA67F2A5C, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_MULTI,       SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* 4-Pak All Action (KR) */
  {0x89B79E77, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_KOREA,       SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Dodgeball King (KR) */
  {0x18FB98A3, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_KOREA,       SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Jang Pung 3 (KR) */
  {0x97D03541, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_KOREA,       SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Sangokushi 3 (KR) */
  {0x67C2F0FF, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_KOREA,       SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Super Boy 2 (KR) */
  {0x192949D5, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_KOREA_8K,    SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Janggun-ui Adeul (KR) */
  {0x9FA727A0, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_KOREA_16K,   SYSTEM_GGMS,        REGION_USA}, /* Street Hero [Proto 0] [SMS-GG] (US) */
  {0xFB481971, 0, 0, SYSTEM_MS_GAMEPAD, MAPPER_KOREA_16K,   SYSTEM_GGMS,        REGION_USA}, /* Street Hero [Proto 1] [SMS-GG] (US) */

  /* games using Codemaster mapper */
  {0x29822980, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_SMS2, REGION_EUROPE}, /* Cosmic Spacehead */
  {0x8813514B, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_SMS2, REGION_EUROPE}, /* Excellent Dizzy Collection, The [Proto] */
  {0xB9664AE1, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_SMS2, REGION_EUROPE}, /* Fantastic Dizzy */
  {0xA577CE46, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_SMS2, REGION_EUROPE}, /* Micro Machines */
  {0xEA5C3A6F, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_SMS2, REGION_USA}, /* Dinobasher - Starring Bignose the Caveman [Proto] */
  {0xAA140C9C, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GGMS, REGION_USA}, /* Excellent Dizzy Collection, The [SMS-GG] */
  {0xC888222B, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GGMS, REGION_USA}, /* Fantastic Dizzy [SMS-GG] */
  {0x76C5BDFB, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GGMS, REGION_USA}, /* Jang Pung 2 [SMS-GG] */
  {0x6CAA625B, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GG,   REGION_USA}, /* Cosmic Spacehead [GG]*/
  {0x152F0DCC, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GG,   REGION_USA}, /* Drop Zone" */
  {0x5E53C7F7, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GG,   REGION_USA}, /* Ernie Els Golf */
  {0xD9A7F170, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GG,   REGION_USA}, /* Man Overboard! */
  {0xF7C524F6, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GG,   REGION_USA}, /* Micro Machines [GG] */
  {0xDBE8895C, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GG,   REGION_USA}, /* Micro Machines 2 - Turbo Tournament */
  {0xC1756BEE, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GG,   REGION_USA}, /* Pete Sampras Tennis */
  {0x72981057, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_CODIES, SYSTEM_GG,   REGION_USA}, /* CJ Elephant Fugitive */

  /* games using serial EEPROM */
  {0x36EBCD6D, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_93C46,  SYSTEM_GG,   REGION_USA}, /* Majors Pro Baseball */
  {0x3D8D0DD6, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_93C46,  SYSTEM_GG,   REGION_USA}, /* World Series Baseball [v0] */
  {0xBB38CFD7, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_93C46,  SYSTEM_GG,   REGION_USA}, /* World Series Baseball [v1] */
  {0x578A8A38, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_93C46,  SYSTEM_GG,   REGION_USA}, /* World Series Baseball '95 */

  /* games using Terebi Oekaki graphic board */
  {0xDD4A661B, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_TEREBI, SYSTEM_SG,   REGION_JAPAN_NTSC}, /* Terebi Oekaki */

  /* games requiring 8K RAM extension adapter */
  {0xCE5648C3, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Bomberman Special [DahJee] (TW) */
  {0x223397A1, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* King's Valley (TW) */
  {0x281D2888, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Knightmare (TW) */
  {0x306D5F78, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Rally-X [DahJee] (TW) */
  {0x29E047CC, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Road Fighter (TW) */
  {0x5CBD1163, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Tank Battalion (TW) */
  {0x2E7166D5, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* The Legend of Kage (TW) */
  {0xC550B4F0, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* TwinBee (TW) */
  {0xFC87463C, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT1, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Yie Ar Kung-Fu II (TW) */
  {0x69FC1494, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT2, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Bomberman Special (TW) */
  {0xFFC4EE3F, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT2, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Magical Kid Wiz (TW) */
  {0x2E366CCF, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT2, SYSTEM_SG, REGION_JAPAN_NTSC}, /* The Castle (TW) */
  {0xAAAC12CF, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT2, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Rally-X (TW) */
  {0xD2EDD329, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_RAM_8K_EXT2, SYSTEM_SG, REGION_JAPAN_NTSC}, /* Road Fighter (TW) */

  /* games requiring Japanese region setting */
  {0x71DEBA5A, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GG,   REGION_JAPAN_NTSC}, /* Pop Breaker */
  {0xC9DD4E5F, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Woody Pop (Super Arkanoid) */

  /* games requiring Mark-III hardware (no Memory Control port) */
  {0xBD1CC7DF, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_MARKIII, REGION_JAPAN_NTSC}, /* Super Tetris (KR) */
  {0x6D309AC5, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_MARKIII, REGION_JAPAN_NTSC}, /* Power Boggle Boggle (KR) */

  /* games requiring random RAM pattern initialization */
  {0x08BF3DE3, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_NONE,   SYSTEM_MARKIII, REGION_JAPAN_NTSC}, /* Alibaba and 40 Thieves (KR) */
  {0x643B6B76, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_NONE,   SYSTEM_MARKIII, REGION_JAPAN_NTSC}, /* Block Hole (KR) */

  /* games requiring PAL timings */
  {0x72420F38, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Addams Familly */
  {0x2D48C1D3, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Back to the Future Part III */
  {0x1CBB7BF1, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Battlemaniacs (BR) */
  {0x1B10A951, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Bram Stoker's Dracula */
  {0xC0E25D62, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* California Games II */
  {0x45C50294, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Jogos de Verao II (BR) */
  {0xC9DBF936, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Home Alone */
  {0x0047B615, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Predator2 */
  {0xF42E145C, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Quest for the Shaven Yak Starring Ren Hoek & Stimpy (BR) */
  {0x9F951756, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* RoboCop 3 */
  {0xF8176918, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Sensible Soccer */
  {0x1575581D, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Shadow of the Beast */
  {0x96B3F29E, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Sonic Blast (BR) */
  {0x5B3B922C, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Sonic the Hedgehog 2 [V0] */
  {0xD6F2BFCA, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Sonic the Hedgehog 2 [V1] */
  {0xCA1D3752, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Space Harrier [50 Hz] */
  {0x85CFC9C9, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* Taito Chase H.Q. */
  {0x332A847D, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,     REGION_EUROPE}, /* NBA Jam [Proto] */

  /* games running in Game Gear MS compatibility mode */
  {0x59840FD6, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Castle of Illusion - Starring Mickey Mouse [SMS-GG] */
  {0x9C76FB3A, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Rastan Saga [SMS-GG] */
  {0xC8381DEF, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Taito Chase H.Q [SMS-GG] */
  {0xDA8E95A9, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* WWF Wrestlemania Steel Cage Challenge [SMS-GG] */
  {0x1D93246E, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Olympic Gold [A][SMS-GG] */
  {0xA2F9C7AF, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Olympic Gold [B][SMS-GG] */
  {0x01EAB89D, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Out Run Europa [SMS-GG] */
  {0xF037EC00, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Out Run Europa (US) [SMS-GG] */
  {0xE5F789B9, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Predator 2 [SMS-GG] */
  {0x311D2863, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Prince of Persia [A][SMS-GG] */
  {0x45F058D6, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Prince of Persia [B][SMS-GG] */
  {0x56201996, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* R.C. Grand Prix [SMS-GG] */
  {0x10DBBEF4, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS,        REGION_USA}, /* Super Kick Off [SMS-GG] */
  {0x9942B69B, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS, REGION_JAPAN_NTSC}, /* Castle of Illusion - Starring Mickey Mouse (J) [SMS-GG] */
  {0x7BB81E3D, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS, REGION_JAPAN_NTSC}, /* Taito Chase H.Q (J) [SMS-GG] */
  {0x6F8E46CF, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS, REGION_JAPAN_NTSC}, /* Alex Kidd in Miracle World (TW) [SMS-GG] */
  {0x3382D73F, 0, 0, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_GGMS, REGION_JAPAN_NTSC}, /* Olympic Gold (TW) [SMS-GG] */

  /* games requiring 3-D Glasses */
  {0x6BD5C2BF, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Space Harrier 3-D */
  {0x8ECD201C, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Blade Eagle 3-D */
  {0xFBF96C81, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Blade Eagle 3-D (BR) */
  {0x58D5FC48, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Blade Eagle 3-D [Proto] */
  {0x31B8040B, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Maze Hunter 3-D */
  {0xABD48AD2, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Poseidon Wars 3-D */
  {0xA3EF13CB, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Zaxxon 3-D */
  {0xBBA74147, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Zaxxon 3-D [Proto] */
  {0xD6F43DDA, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Out Run 3-D */
  {0x871562b0, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Maze Walker */
  {0x156948f9, 1, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Space Harrier 3-D (J) */

  /* games requiring 3-D Glasses & Sega Light Phaser */
  {0xFBE5CFBB, 1, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Missile Defense 3D */
  {0xE79BB689, 1, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Missile Defense 3D [BIOS] */

  /* games requiring Sega Light Phaser */
  {0x861B6E79, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Assault City [Light Phaser] */
  {0x5FC74D2A, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Gangster Town */
  {0xE167A561, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Hang-On / Safari Hunt */
  {0x91E93385, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Hang-On / Safari Hunt [BIOS] */
  {0xE8EA842C, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Marksman Shooting / Trap Shooting */
  {0xE8215C2E, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Marksman Shooting / Trap Shooting / Safari Hunt */
  {0x205CAAE8, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Operation Wolf */
  {0x23283F37, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Operation Wolf [A] */
  {0xDA5A7013, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Rambo 3 */
  {0x79AC8E7F, 0, 1, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Rescue Mission */
  {0x4B051022, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Shooting Gallery */
  {0xA908CFF5, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Spacegun */
  {0x5359762D, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Wanted */
  {0x0CA95637, 0, 0, SYSTEM_LIGHTPHASER, MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Laser Ghost */

  /* games requiring Sega Paddle */
  {0xF9DBB533, 0, 1, SYSTEM_PADDLE,      MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Alex Kidd BMX Trial */
  {0xA6FA42D0, 0, 1, SYSTEM_PADDLE,      MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Galactic Protector */
  {0x29BC7FAD, 0, 1, SYSTEM_PADDLE,      MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Megumi Rescue */
  {0x315917D4, 0, 0, SYSTEM_PADDLE,      MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Woody Pop */

  /* games requiring Sega Sport Pad */
  {0x0CB7E21F, 0, 0, SYSTEM_SPORTSPAD,   MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Great Ice Hockey */
  {0xE42E4998, 0, 0, SYSTEM_SPORTSPAD,   MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Sports Pad Football */
  {0x41C948BF, 0, 0, SYSTEM_SPORTSPAD,   MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Sports Pad Soccer */

  /* games supporting YM2413 FM */
  {0x1C951F8E, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* After Burner */
  {0xC13896D5, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Alex Kidd: The Lost Stars */
  {0x5CBFE997, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Alien Syndrome */
  {0xBBA2FE98, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Altered Beast */
  {0xFF614EB3, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Aztec Adventure */
  {0x3084CF11, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Bomber Raid */
  {0xAC6009A7, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* California Games */
  {0xA4852757, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Captain Silver */
  {0xB81F6FA5, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Captain Silver (U) */
  {0x3CFF6E80, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Casino Games */
  {0xE7F62E6D, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Cloud Master */
  {0x908E7524, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Cyborg Hunter */
  {0xA55D89F3, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Double Dragon */
  {0xB8B141F9, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Fantasy Zone II */
  {0xD29889AD, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Fantasy Zone: The Maze */
  {0xA4AC35D8, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Galaxy Force */
  {0x6C827520, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Galaxy Force (U) */
  {0x1890F407, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Game Box Série Esportes Radicais (BR) */
  {0xB746A6F5, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Global Defense */
  {0x91A0FC4E, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Global Defense [Proto] */
  {0x48651325, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Golfamania */
  {0x5DABFDC3, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Golfamania [Proto] */
  {0xA51376FE, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Golvellius - Valley of Doom */
  {0x98E4AE4A, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Great Golf */
  {0x516ED32E, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Kenseiden */
  {0xE8511B08, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Lord of The Sword */
  {0x0E333B6E, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Miracle Warriors - Seal of The Dark Lord */
  {0x301A59AA, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Miracle Warriors - Seal of The Dark Lord [Proto] */
  {0x01D67C0B, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Mônica no Castelo do Dragão (BR) */
  {0x5589D8D2, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Out Run */
  {0xE030E66C, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Parlour Games */
  {0xF97E9875, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Penguin Land */
  {0x4077EFD9, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Power Strike */
  {0xBB54B6B0, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* R-Type */
  {0x42FC47EE, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Rampage */
  {0xC547EB1B, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Rastan */
  {0x9A8B28EC, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Scramble Spirits */
  {0xAAB67EC3, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Shanghai */
  {0x0C6FAC4E, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Shinobi */
  {0x4752CAE7, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* SpellCaster */
  {0x1A390B93, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Tennis Ace */
  {0xAE920E4B, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Thunder Blade */
  {0x51BD14BE, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Time Soldiers */
  {0x22CCA9BB, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Turma da Mônica em: O Resgate (BR) */
  {0xB52D60C8, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Ultima IV */
  {0xDE9F8517, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Ultima IV [Proto] */
  {0xDFB0B161, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Vigilante */
  {0x679E1676, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Wonder Boy III: The Dragon's Trap */
  {0x8CBEF0C1, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Wonder Boy in Monster Land */
  {0x2F2E3BC9, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS2,        REGION_USA}, /* Zillion II - The Tri Formation */
  {0x48D44A13, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_NONE,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* BIOS (J) */
  {0xD8C4165B, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Aleste */
  {0x4CC11DF9, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Alien Syndrome (J) */
  {0xE421E466, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Chouon Senshi Borgman */
  {0x2BCDB8FA, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Doki Doki Penguin Land - Uchuu-Daibouken */
  {0x56BD2455, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Doki Doki Penguin Land - Uchuu-Daibouken [Proto] */
  {0xC722FB42, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Fantasy Zone II (J) */
  {0x7ABC70E9, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Family Games (Party Games) */
  {0x6586BD1F, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Masters Golf */
  {0x4847BC91, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Masters Golf [Proto] */
  {0xB9FDF6D9, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Haja no Fuuin */
  {0x955A009E, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Hoshi wo Sagashite */
  {0x05EA5353, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Kenseiden (J) */
  {0xD11D32E4, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Kujakuou */
  {0xAA7D6F45, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Lord of Sword */
  {0xBF0411AD, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Maou Golvellius */
  {0x21A21352, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Maou Golvellius [Proto] */
  {0x5B5F9106, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Nekyuu Kousien */
  {0xBEA27D5C, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Opa Opa */
  {0x6605D36A, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Phantasy Star (J) */
  {0xE1FFF1BB, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Shinobi (J) */
  {0x11645549, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Solomon no Kagi - Oujo Rihita no Namida */
  {0x7E0EF8CB, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Super Racing */
  {0xB1DA6A30, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Super Wonder Boy Monster World */
  {0x8132AB2C, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Tensai Bakabon */
  {0xC0CE19B1, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_SMS,  REGION_JAPAN_NTSC}, /* Thunder Blade (J) */
  {0x07301F83, 0, 1, SYSTEM_MS_GAMEPAD,  MAPPER_SEGA,   SYSTEM_PBC,  REGION_JAPAN_NTSC}  /* Phantasy Star [Megadrive] (J) */
};

/* Cartridge & BIOS ROM hardware */
static romhw_t cart_rom;
static romhw_t bios_rom;

/* Current slot */
static struct
{
  uint8 *rom;
  uint8 *fcr;
  uint8 mapper;
  uint8 pages;
} slot;

/* Function prototypes */
static void mapper_reset(void);
static void mapper_8k_w(int offset, unsigned int data);
static void mapper_16k_w(int offset, unsigned int data);
static void write_mapper_none(unsigned int address, unsigned char data);
static void write_mapper_sega(unsigned int address, unsigned char data);
static void write_mapper_codies(unsigned int address, unsigned char data);
static void write_mapper_korea(unsigned int address, unsigned char data);
static void write_mapper_korea_8k(unsigned int address, unsigned char data);
static void write_mapper_korea_16k(unsigned int address, unsigned char data);
static void write_mapper_msx(unsigned int address, unsigned char data);
static void write_mapper_multi(unsigned int address, unsigned char data);
static void write_mapper_93c46(unsigned int address, unsigned char data);
static void write_mapper_terebi(unsigned int address, unsigned char data);
static unsigned char read_mapper_93c46(unsigned int address);
static unsigned char read_mapper_terebi(unsigned int address);
static unsigned char read_mapper_korea_8k(unsigned int address);
static unsigned char read_mapper_default(unsigned int address);

void sms_cart_init(void)
{
  int i;

  /* game CRC */
  uint32 crc = crc32(0, cart.rom, cart.romsize);

  /* use Master System controller by default */
  uint8 device = SYSTEM_MS_GAMEPAD;

  /* unmapped memory return $FF on read (mapped to unused cartridge areas $510000-$5103FF & $510400-$5107FF) */
  memset(cart.rom + 0x510000, 0xFF, 0x800);

  /* default cartridge ROM mapper */
  cart_rom.mapper = (cart.romsize > 0xC000) ? MAPPER_SEGA : MAPPER_NONE;

  /* disable 3-D Glasses by default */
  cart.special = 0;

  /* YM2413 chip in AUTO mode */
  if (config.ym2413 & 2)
  {
    if ((system_hw & SYSTEM_SMS) && (region_code == REGION_JAPAN_NTSC))
    {
      /* japanese Master System has built-in FM chip */
      config.ym2413 = 3;
    }
    else
    {
      /* by default, FM chip is disabled */
      config.ym2413 = 2;
    }
  }

  /* auto-detect game settings */
  for (i=0; i<GAME_DATABASE_CNT; i++)
  {
    if (crc == game_list[i].crc)
    {
      /* auto-detect cartridge mapper */
      cart_rom.mapper = game_list[i].mapper;

      /* auto-detect required peripherals */
      device = game_list[i].peripheral;

      /* auto-detect 3D glasses support */
      cart.special = game_list[i].g_3d;

      /* auto-detect system hardware */
      if (!config.system || ((config.system == SYSTEM_GG) && (game_list[i].system == SYSTEM_GGMS)))
      {
        system_hw = game_list[i].system;
      }

      /* auto-detect YM2413 chip support in AUTO mode */
      if (config.ym2413 & 2)
      {
        config.ym2413 |= game_list[i].fm;
      }

      /* game found, leave loop */
      i = GAME_DATABASE_CNT;
    }
  }

  /* ROM paging */
  if (cart_rom.mapper < MAPPER_SEGA)
  {
    /* 1k ROM banks */
    cart_rom.pages = (cart.romsize + (1 << 10) - 1) >> 10;
  }
  else if (cart_rom.mapper & MAPPER_KOREA_8K)
  {
    /* 8k ROM banks */
    cart_rom.pages = (cart.romsize + (1 << 13) - 1) >> 13;
  }
  else
  {
    /* 16k ROM banks */
    cart_rom.pages = (cart.romsize + (1 << 14) - 1) >> 14;
  }

  /* initialize extra hardware */
  if (cart_rom.mapper == MAPPER_93C46)
  {
    /* 93C46 eeprom */
    eeprom_93c_init();
  }
  else if (cart_rom.mapper == MAPPER_TEREBI)
  {
    /* Terebi Oekaki tablet */
    cart.special |= HW_TEREBI_OEKAKI;
  }

  /* initialize SRAM */
  sram_init();

  /* enable cartridge backup memory by default */
  sram.on = 1;

  /* save current settings */
  if (old_system[0] == -1)
  {
    old_system[0] = input.system[0];
  }
  if (old_system[1] == -1)
  {
    old_system[1] = input.system[1];
  }

  /* force port A & port B configuration */
  input.system[0] = device;
  input.system[1] = SYSTEM_MS_GAMEPAD;

  /* default gun offset */
  input.x_offset = 20; 
  input.y_offset = 0; 

  /* SpaceGun & Gangster Town use different gun offset */
  if ((crc == 0x5359762D) || (crc == 0x5FC74D2A))
  {
    input.x_offset = 16;
  }

  /* BIOS support */
  if (config.bios & 1)
  {
    /* load BIOS file */
    int bios_size = load_bios();

    if (bios_size > 0xC000)
    {
      /* assume SEGA mapper if BIOS ROM is larger than 48k */
      bios_rom.mapper = MAPPER_SEGA;
      bios_rom.pages = bios_size >> 14;
    }
    else if (bios_size >= 0)
    {
      /* default BIOS ROM mapper */
      bios_rom.mapper = MAPPER_NONE;
      bios_rom.pages = bios_size >> 10;
    }

    /* unload cartridge if required & BIOS ROM is loaded */
    if (!(config.bios & 2) && bios_rom.pages)
    {
      cart_rom.pages = 0;
    }
  }
  else 
  {
    /* mark Master System & Game Gear BIOS as unloaded */
    system_bios &= ~(SYSTEM_SMS | SYSTEM_GG);

    /* BIOS ROM is disabled */
    bios_rom.pages = 0;
  }
}

void sms_cart_reset(void)
{
  /* reset BIOS ROM paging (SEGA mapper by default) */
  bios_rom.fcr[0] = 0;
  bios_rom.fcr[1] = 0;
  bios_rom.fcr[2] = 1;
  bios_rom.fcr[3] = 2;

  /* reset cartridge ROM paging */
  switch (cart_rom.mapper)
  {
    case MAPPER_SEGA:
    case MAPPER_SEGA_X:
      cart_rom.fcr[0] = 0;
      cart_rom.fcr[1] = 0;
      cart_rom.fcr[2] = 1;
      cart_rom.fcr[3] = 2;
      break;

    case MAPPER_KOREA_8K:
    case MAPPER_MSX:
    case MAPPER_MSX_NEMESIS:
      cart_rom.fcr[0] = 0;
      cart_rom.fcr[1] = 0;
      cart_rom.fcr[2] = 0;
      cart_rom.fcr[3] = 0;
      break;

    default:
      cart_rom.fcr[0] = 0;
      cart_rom.fcr[1] = 0;
      cart_rom.fcr[2] = 1;
      cart_rom.fcr[3] = 0;
      break;
  }

  /* check if BIOS is larger than 1k */
  if (bios_rom.pages > 1)
  {
    /* enable BIOS ROM */
    slot.rom = cart.rom + 0x400000;
    slot.fcr = bios_rom.fcr;
    slot.mapper = bios_rom.mapper;
    slot.pages = bios_rom.pages;
  }
  else
  {
    /* enable cartridge ROM */
    slot.rom = cart.rom;
    slot.fcr = cart_rom.fcr;
    slot.mapper = cart_rom.mapper;
    slot.pages = cart_rom.pages;

    /* force Memory Control register value in RAM (usually set by Master System BIOS) */
    if (system_hw & SYSTEM_SMS)
    {
      work_ram[0] = 0xA8;
    }
  }

  /* reset Memory Control register (RAM & I/O are enabled, either BIOS or Cartridge ROM are enabled) */
  io_reg[0x0E] = bios_rom.pages ? 0xE0 : 0xA8;

  /* reset Z80 memory map */
  mapper_reset();

  /* 1k BIOS special case (Majesco GG) */
  if (bios_rom.pages == 1)
  {
    /* BIOS ROM is mapped to $0000-$03FF */
    z80_readmap[0] = cart.rom + 0x400000;
  }
}

void sms_cart_switch(uint8 mode)
{
  /* by default, disable cartridge & BIOS ROM */
  slot.pages = 0;

  /* cartridge ROM enabled ? */
  if (mode & 0x40)
  {
    /* check if cartridge is loaded */
    if (cart_rom.pages)
    {
      /* map cartridge ROM */
      slot.rom = cart.rom;
      slot.fcr = cart_rom.fcr;
      slot.mapper = cart_rom.mapper;
      slot.pages = cart_rom.pages;
    }
  }
  else
  {
    /* BIOS ROM enabled ? */
    if (mode & 0x08)
    {
      /* check if BIOS ROM is larger than 1K */
      if (bios_rom.pages > 1)
      {
        /* map BIOS ROM */
        slot.rom = cart.rom + 0x400000;
        slot.fcr = bios_rom.fcr;
        slot.mapper = bios_rom.mapper;
        slot.pages = bios_rom.pages;
      }
      else
      {
        /* by default, map cartridge ROM */
        slot.rom = cart.rom;
        slot.fcr = cart_rom.fcr;
        slot.mapper = cart_rom.mapper;
        slot.pages = cart_rom.pages;
      }
    }

    /* assume only BIOS would disable cartridge slot */
    if (!bios_rom.pages)
    {
      /* max. BIOS ROM size supported is 1MB */
      if (cart.romsize <= 0x100000)
      {
        /* copy to BIOS ROM */
        memcpy(cart.rom + 0x400000, cart.rom, cart.romsize);
        memcpy(bios_rom.fcr, cart_rom.fcr, 4);
        bios_rom.mapper = cart_rom.mapper;
        bios_rom.pages = cart_rom.pages;

        /* unload cartridge  */
        cart_rom.pages = 0;
      }
    }
  }

  /* reset Z80 memory map */
  mapper_reset();

  /* 1k BIOS special case (Majesco GG) */
  if ((bios_rom.pages == 1) && ((mode & 0x48) == 0x08))
  {
    /* BIOS ROM is mapped to $0000-$03FF */
    z80_readmap[0] = cart.rom + 0x400000;
  }
}

int sms_cart_region_detect(void)
{
  int i;

  /* compute CRC */
  uint32 crc = crc32(0, cart.rom, cart.romsize);

  /* Turma da Mônica em: O Resgate & Wonder Boy III enable FM support on japanese hardware only */
  if (config.ym2413 && ((crc == 0x22CCA9BB) || (crc == 0x679E1676)))
  {
    return REGION_JAPAN_NTSC;
  }

  /* game database */
  for (i=0; i<GAME_DATABASE_CNT; i++)
  {
    if (crc == game_list[i].crc)
    {
      return game_list[i].region;
    }
  }

  /* Mark-III hardware */
  if (system_hw == SYSTEM_MARKIII)
  {
    /* Japan */
    return REGION_JAPAN_NTSC;
  }

  /* default region */
  return REGION_USA;
}

int sms_cart_context_save(uint8 *state)
{
  int bufferptr = 0;
  save_param(slot.fcr, 4);
  return bufferptr;
}

int sms_cart_context_load(uint8 *state)
{
  int bufferptr = 0;
  load_param(slot.fcr, 4);
  return bufferptr;
}

static void mapper_reset(void)
{
  int i;

  /* reset internal RAM mapping */
  if (system_hw == SYSTEM_SG)
  {
    /* 8k RAM extension adapter (type B) */
    if (cart_rom.mapper == MAPPER_RAM_8K_EXT2)
    {
      /* $C000-$FFFF mapped to 8k external RAM (mirrored) */
      for (i = 0x30; i < 0x40; i++)
      {
        z80_readmap[i] = z80_writemap[i] = &work_ram[(i & 0x07) << 10];
      }
    }
    else
    {
      /* $C000-$FFFF mapped to 2k mirrored RAM */
      for (i = 0x30; i < 0x40; i++)
      {
        z80_readmap[i] = z80_writemap[i] = &work_ram[(i & 0x01) << 10];
      }
    }
  }
  else
  {
    /* $C000-$FFFF mapped to 8k internal RAM (mirrored) */
    for (i = 0x30; i < 0x40; i++)
    {
      z80_readmap[i] = z80_writemap[i] = &work_ram[(i & 0x07) << 10];
    }
  }

  /* check if ROM is disabled */
  if (!slot.pages)
  {
    /* $0000-$BFFF mapped to unused cartridge areas */
    for(i = 0x00; i < 0x30; i++)
    {
      z80_writemap[i] = cart.rom + 0x510000;
      z80_readmap[i]  = cart.rom + 0x510400;
    }

    /* set default Z80 memory handlers */
    z80_readmem = read_mapper_default;
    z80_writemem = write_mapper_none;
    return;
  }

  /* reset default $0000-$BFFF mapping */
  if (slot.mapper < MAPPER_SEGA)
  {
    /* $0000-$BFFF mapped to ROM (48k) */
    for (i = 0x00; i < 0x30; i++)
    {
      z80_readmap[i] = &slot.rom[i << 10];
      z80_writemap[i] = cart.rom + 0x510000; /* unused area */
    }

    /* cartridge extra RAM enabled by default with 32K ROM */
    if (slot.pages <= 0x20)
    {
      /* $8000-$BFFF mapped to 8k external RAM (mirrored) */
      for (i = 0x20; i < 0x30; i++)
      {
        z80_readmap[i] = z80_writemap[i] = &work_ram[0x2000 + ((i & 0x07) << 10)];
      }
    }

    /* 8k RAM extension adapter (type A) */
    if (slot.mapper == MAPPER_RAM_8K_EXT1)
    {
      /* $2000-$3FFF mapped to 8k external RAM */
      for (i = 0x08; i < 0x10; i++)
      {
        z80_readmap[i] = z80_writemap[i] = &work_ram[0x2000 + ((i & 0x07) << 10)];
      }
    }
  }
  else
  {
    /* $0000-$BFFF mapped to ROM by default */
    for (i = 0x00; i < 0x30; i++)
    {
      z80_readmap[i] = &slot.rom[i << 10];
      z80_writemap[i] = cart.rom + 0x510000; /* unused area */
    }

    /* reset default ROM paging */
    if (slot.mapper & MAPPER_KOREA_8K)
    {
      mapper_8k_w(0,slot.fcr[0]);
      mapper_8k_w(1,slot.fcr[1]);
      mapper_8k_w(2,slot.fcr[2]);
      mapper_8k_w(3,slot.fcr[3]);

      /* Nemesis special case */
      if (slot.mapper == MAPPER_MSX_NEMESIS)
      {
        /* first 8k page is mapped to last 8k ROM bank */
        for (i = 0x00; i < 0x08; i++)
        {
          z80_readmap[i] = &slot.rom[(0x0f << 13) | ((i & 0x07) << 10)];
        }
      }
    }
    else
    {
      mapper_16k_w(0,slot.fcr[0]);
      mapper_16k_w(1,slot.fcr[1]);
      mapper_16k_w(2,slot.fcr[2]);
      mapper_16k_w(3,slot.fcr[3]);
    }
  }

  /* reset Z80 memory handlers */
  switch (slot.mapper)
  {
    case MAPPER_NONE:
    case MAPPER_RAM_8K_EXT1:
    case MAPPER_RAM_8K_EXT2:
      z80_readmem = read_mapper_default;
      z80_writemem = write_mapper_none;
      break;

    case MAPPER_CODIES:
      z80_readmem = read_mapper_default;
      z80_writemem = write_mapper_codies;
      break;

    case MAPPER_KOREA:
      z80_readmem = read_mapper_default;
      z80_writemem = write_mapper_korea;
      break;

    case MAPPER_KOREA_8K:
      z80_readmem = read_mapper_korea_8k;
      z80_writemem = write_mapper_korea_8k;
      break;

    case MAPPER_KOREA_16K:
      z80_readmem = read_mapper_default;
      z80_writemem = write_mapper_korea_16k;
      break;

    case MAPPER_MSX:
    case MAPPER_MSX_NEMESIS:
      z80_readmem = read_mapper_default;
      z80_writemem = write_mapper_msx;
      break;

    case MAPPER_MULTI:
      z80_readmem = read_mapper_default;
      z80_writemem = write_mapper_multi;
      break;

    case MAPPER_93C46:
      z80_readmem = read_mapper_93c46;
      z80_writemem = write_mapper_93c46;
      break;

    case MAPPER_TEREBI:
      z80_readmem = read_mapper_terebi;
      z80_writemem = write_mapper_terebi;
      break;

    default:
      z80_readmem = read_mapper_default;
      z80_writemem = write_mapper_sega;
      break;
  }
}

static void mapper_8k_w(int offset, unsigned int data)
{
  int i;

  /* cartridge ROM page (8k) */
  uint8 page = data % slot.pages;
  
  /* Save frame control register data */
  slot.fcr[offset] = data;

  /* 4 x 8k banks */
  switch (offset & 3)
  {
    case 0: /* cartridge ROM bank (8k) at $8000-$9FFF */
    {
      for (i = 0x20; i < 0x28; i++)
      {
        z80_readmap[i] = &slot.rom[(page << 13) | ((i & 0x07) << 10)];
      }
      break;
    }
    
    case 1: /* cartridge ROM bank (8k) at $A000-$BFFF */
    {
      for (i = 0x28; i < 0x30; i++)
      {
        z80_readmap[i] = &slot.rom[(page << 13) | ((i & 0x07) << 10)];
      }
      break;
    }
    
    case 2: /* cartridge ROM bank (8k) at $4000-$5FFF */
    {
      for (i = 0x10; i < 0x18; i++)
      {
        z80_readmap[i] = &slot.rom[(page << 13) | ((i & 0x07) << 10)];
      }
      break;
    }
    
    case 3: /* cartridge ROM bank (8k) at $6000-$7FFF */
    {
      for (i = 0x18; i < 0x20; i++)
      {
        z80_readmap[i] = &slot.rom[(page << 13) | ((i & 0x07) << 10)];
      }
      break;
    }
  }

#ifdef CHEATS_UPDATE
  /* update ROM patches when banking has changed */
  CHEATS_UPDATE();
#endif
}
    
static void mapper_16k_w(int offset, unsigned int data)
{
  int i;

  /* cartridge ROM page (16k) */
  uint8 page = data % slot.pages;

  /* page index increment (SEGA mapper only) */
  if ((slot.fcr[0] & 0x03) && (slot.mapper == MAPPER_SEGA))
  {
    page = (page + ((4 - (slot.fcr[0] & 0x03)) << 3)) % slot.pages;
  }

  /* save frame control register data */
  slot.fcr[offset] = data;

  switch (offset)
  {
    case 0: /* control register (SEGA mapper only) */
    {
      if (data & 0x08)
      {
        /* external RAM (upper or lower 16K) mapped at $8000-$BFFF */
        for (i = 0x20; i < 0x30; i++)
        {
          z80_readmap[i] = z80_writemap[i] = &sram.sram[((data & 0x04) << 12) + ((i & 0x0F) << 10)];
        }
      }
      else
      {
        /* cartridge ROM page (16k) */
        page = slot.fcr[3] % slot.pages;
        
        /* page index increment (SEGA mapper) */
        if ((data & 0x03) && (slot.mapper == MAPPER_SEGA))
        {
          page = (page + ((4 - (data & 0x03)) << 3)) % slot.pages;
        }

        /* cartridge ROM mapped at $8000-$BFFF */
        for (i = 0x20; i < 0x30; i++)
        {
          z80_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
          z80_writemap[i] = cart.rom + 0x510000; /* unused area */
        }
      }

      if (data & 0x10)
      {
        /* external RAM (lower 16K) mapped at $C000-$FFFF */
        for (i = 0x30; i < 0x40; i++)
        {
          z80_readmap[i] = z80_writemap[i] = &sram.sram[(i & 0x0F) << 10];
        }
      }
      else
      {
        /* internal RAM (8K mirrorred) mapped at $C000-$FFFF */
        for (i = 0x30; i < 0x40; i++)
        {
          z80_readmap[i] = z80_writemap[i] = &work_ram[(i & 0x07) << 10];
        }
      }
      break;
    }

    case 1: /* cartridge ROM bank (16k) at $0000-$3FFF */
    {
      /* first 1k is not fixed (CODEMASTER or MULTI mappers only) */
      if ((slot.mapper == MAPPER_CODIES) || (slot.mapper == MAPPER_MULTI))
      {
        z80_readmap[0] = &slot.rom[(page << 14)];
      }

      for (i = 0x01; i < 0x10; i++)
      {
        z80_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
      }
      break;
    }

    case 2: /* cartridge ROM bank (16k) at $4000-$7FFF */
    {
      for (i = 0x10; i < 0x20; i++)
      {
        z80_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
      }

      /* cartridge RAM switch (CODEMASTER mapper only, see Ernie Elf's Golf) */
      if (slot.mapper == MAPPER_CODIES)
      {
        if (data & 0x80)
        {
          /* external RAM (8k) mapped at $A000-$BFFF */
          for (i = 0x28; i < 0x30; i++)
          {
            z80_readmap[i] = z80_writemap[i] = &sram.sram[(i & 0x0F) << 10];
          }
        }
        else
        {
          /* cartridge ROM page (16k) */
          page = slot.fcr[3] % slot.pages;

          /* cartridge ROM mapped at $A000-$BFFF */
          for (i = 0x28; i < 0x30; i++)
          {
            z80_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
            z80_writemap[i] = cart.rom + 0x510000; /* unused area */
          }
        }
      }
      break;
    }

    case 3: /* cartridge ROM bank (16k) at $8000-$BFFF */
    {
      /* check that external RAM (16k) is not mapped at $8000-$BFFF (SEGA mapper only) */
      if ((slot.fcr[0] & 0x08)) break;

      /* first 8k */
      for (i = 0x20; i < 0x28; i++)
      {
        z80_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
      }

      /* check that cartridge RAM (8k) is not mapped at $A000-$BFFF (CODEMASTER mapper only) */
      if ((slot.mapper == MAPPER_CODIES) && (slot.fcr[2] & 0x80)) break;

      /* last 8k */
      for (i = 0x28; i < 0x30; i++)
      {
        z80_readmap[i] = &slot.rom[(page << 14) | ((i & 0x0F) << 10)];
      }
      break;
    }
  }

#ifdef CHEATS_UPDATE
  /* update ROM patches when banking has changed */
  CHEATS_UPDATE();
#endif
}

static void write_mapper_none(unsigned int address, unsigned char data)
{
  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_sega(unsigned int address, unsigned char data)
{
  if (address >= 0xFFFC)
  {
    mapper_16k_w(address & 3, data);
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_codies(unsigned int address, unsigned char data)
{
  if (address == 0x0000)
  {
    mapper_16k_w(1,data);
    return;
  }

  if (address == 0x4000)
  {
    mapper_16k_w(2,data);
    return;
  }

  if (address == 0x8000)
  {
    mapper_16k_w(3,data);
    return;
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_multi(unsigned int address, unsigned char data)
{
  if (address == 0x3FFE)
  {
    mapper_16k_w(1,data);
    return;
  }

  if (address == 0x7FFF)
  {
    mapper_16k_w(2,data);
    return;
  }

  if (address == 0xBFFF)
  {
    mapper_16k_w(3,(slot.fcr[1] & 0x30) + data);
    return;
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_korea(unsigned int address, unsigned char data)
{
  if (address == 0xA000)
  {
    mapper_16k_w(3,data);
    return;
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_msx(unsigned int address, unsigned char data)
{
  if (address <= 0x0003)
  {
    mapper_8k_w(address,data);
    return;
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_korea_8k(unsigned int address, unsigned char data)
{
  if (address == 0x4000)
  {
    mapper_8k_w(2,data);
    return;
  }

  if (address == 0x6000)
  {
    mapper_8k_w(3,data);
    return;
  }

  if (address == 0x8000)
  {
    mapper_8k_w(0,data);
    return;
  }

  if (address == 0xA000)
  {
    mapper_8k_w(1,data);
    return;
  }

  if (address == 0xFFFE)
  {
    mapper_8k_w(2,(data << 1) & 0xFF);
    mapper_8k_w(3,(1 + (data << 1)) & 0xFF);
  }
  else if (address == 0xFFFF)
  {
    mapper_8k_w(0,(data << 1) & 0xFF);
    mapper_8k_w(1,(1 + (data << 1)) & 0xFF);
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_korea_16k(unsigned int address, unsigned char data)
{
  if (address == 0x4000)
  {
    mapper_16k_w(2,data);
    return;
  }

  if (address == 0x8000)
  {
    mapper_16k_w(3,data);
    return;
  }

  /* SEGA mapper compatibility */
  if (address >= 0xFFFC)
  {
    mapper_16k_w(address & 3, data);
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_93c46(unsigned int address, unsigned char data)
{
  /* EEPROM serial input */
  if ((address == 0x8000) && eeprom_93c.enabled)
  {
    eeprom_93c_write(data);
    return;
  }

  /* EEPROM ctrl */
  if (address == 0xFFFC)
  {
    /* enable/disable EEPROM */
    eeprom_93c.enabled = data & 0x08;

    if (data & 0x80)
    {
      /* reset EEPROM */
      eeprom_93c_init();
    }
  }

  /* SEGA mapper compatibility */
  if (address > 0xFFFC)
  {
    mapper_16k_w(address & 3, data);
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static void write_mapper_terebi(unsigned int address, unsigned char data)
{
  if (address == 0x6000)
  {
    terebi_oekaki_write(data);
    return;
  }

  z80_writemap[address >> 10][address & 0x03FF] = data;
}

static unsigned char read_mapper_93c46(unsigned int address)
{
  if ((address == 0x8000) && eeprom_93c.enabled)
  {
    return eeprom_93c_read();
  }

  return z80_readmap[address >> 10][address & 0x03FF];
}

static unsigned char read_mapper_terebi(unsigned int address)
{
  if (address == 0x8000)
  {
    return (terebi_oekaki_read() >> 8);
  }

  if (address == 0xA000)
  {
    return (terebi_oekaki_read() & 0xFF);
  }

  return z80_readmap[address >> 10][address & 0x03FF];
}

static unsigned char read_mapper_korea_8k(unsigned int address)
{
  unsigned char data = z80_readmap[address >> 10][address & 0x03FF];

  /* 16k page */
  unsigned char page = address >> 14;

  /* $4000-$7FFFF and $8000-$BFFF area are protected */
  if (((page == 1) && (slot.fcr[2] & 0x80)) || ((page == 2) && (slot.fcr[0] & 0x80)))
  {
    /* bit-swapped value */
    data = (((data >> 7) & 0x01) | ((data >> 5) & 0x02) |
            ((data >> 3) & 0x04) | ((data >> 1) & 0x08) |
            ((data << 1) & 0x10) | ((data << 3) & 0x20) |
            ((data << 5) & 0x40) | ((data << 7) & 0x80));
  }

  return data;
}

static unsigned char read_mapper_default(unsigned int address)
{
  return z80_readmap[address >> 10][address & 0x03FF];
}
