/*
    This file is part of CrabEmu.

    Copyright (C) 2008 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef _arch_dreamcast
#include <zlib/zlib.h>
#else
#include <zlib.h>
#endif

#ifdef DEBUG
#include <stdio.h>
#endif

#include "smsmem.h"
#include "mappers.h"

typedef struct srom_s   {
    uint32 adler32;
    uint32 crc32;
    uint32 mapper;
} special_rom_t;

static const int rom_count = 8;
static special_rom_t romlist[] = {
    { 0xBF3A0EDC, 0x092F29D6, SMS_MAPPER_CASTLE }, /* The Castle - SG-1000 */
    { 0x86429577, 0x4ED45BDA, SMS_MAPPER_93C46 }, /* Nomo's World Series Baseball */
    { 0x503B0A79, 0x578A8A38, SMS_MAPPER_93C46 }, /* World Series Baseball 95 */
    { 0x7216AA8E, 0x3D8D0DD6, SMS_MAPPER_93C46 }, /* World Series Baseball v1.0 */
    { 0xAB42DB3F, 0xBB38CFD7, SMS_MAPPER_93C46 }, /* World Series Baseball v1.1 */
    { 0x52A77FCF, 0x36EBCD6D, SMS_MAPPER_93C46 }, /* The Majors Pro Baseball */
    { 0x09455AFE, 0x2DA8E943, SMS_MAPPER_93C46 }, /* Pro Yakyuu GG League */
    { 0x7C31B39D, 0xDD4A661B, SMS_MAPPER_TEREBI_OEKAKI } /* Terebi Oekaki - SG-1000 */
};

int sms_find_mapper(const uint8 *rom, uint32 len)   {
    uint32 adler, crc;
    int i;

    adler = adler32(0L, Z_NULL, 0);
    adler = adler32(adler, rom, len);

    crc = crc32(0L, Z_NULL, 0);
    crc = crc32(crc, rom, len);

#ifdef DEBUG
    printf("Checksums: Adler-32: 0x%08X CRC32: 0x%08X\n", adler, crc);
#endif

    for(i = 0; i < rom_count; ++i)  {
        if(romlist[i].adler32 == adler && romlist[i].crc32 == crc)  {
            return romlist[i].mapper;
        }
    }

    return (uint32) -1;
}
