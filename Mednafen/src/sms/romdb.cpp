#include "shared.h"
#include "romdb.h"

static const rominfo_t game_list[] = {
    {0x29822980, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, -1, "Cosmic Spacehead"},
    {0xB9664AE1, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, -1, "Fantastic Dizzy"},
    {0xA577CE46, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, -1, "Micro Machines"},
    {0x8813514B, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, -1, "Excellent Dizzy (Proto)"},
    {0xAA140C9C, MAPPER_CODIES, DISPLAY_PAL, TERRITORY_EXPORT, -1, "Excellent Dizzy (Proto - GG)"}, 

    // Game Gear
    {0xd9a7f170, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, -1, "S.S. Lucifer" },

    {0x5e53c7f7, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, -1, "Ernie Els Golf" },
    {0xc888222b, MAPPER_CODIES, DISPLAY_NTSC, TERRITORY_EXPORT, CONSOLE_SMS, "Fantastic Dizzy" },


    // SG-1000
    { 0x092f29d6, MAPPER_CASTLE, DISPLAY_NTSC, TERRITORY_DOMESTIC, -1,	"The Castle" },
    {-1        , -1  	      , -1	    , -1              , -1,	NULL},
};

const rominfo_t *find_rom_in_db(uint32 crc)
{
    /* Look up mapper in game list */
    for(int i = 0; game_list[i].name != NULL; i++)
    {
        if(crc == game_list[i].crc)
	 return(&game_list[i]);
    }

 return(NULL);
}

