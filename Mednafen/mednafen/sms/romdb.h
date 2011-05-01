#ifndef __ROMDB_H
#define __ROMDB_H

namespace MDFN_IEN_SMS
{

typedef struct {
    uint32 crc;
    int mapper;
    int display;
    int territory;
    int system;
    const char *name;
} rominfo_t;

const rominfo_t *find_rom_in_db(uint32 crc);

}

#endif
