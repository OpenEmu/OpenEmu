#ifndef __MD_MAP_EEPROM_H
#define __MD_MAP_EEPROM_H

enum
{
 EEP_NONE = 0,
 EEP_ACCLAIM_24C02_OLD = 1,
 EEP_ACCLAIM_24C02,
 EEP_ACCLAIM_24C16,
 EEP_ACCLAIM_24C65,

 EEP_EA_24C01,

 EEP_SEGA_24C01,
 EEP_SEGA_24C01_ALT,

 EEP_CM_24C01,
 EEP_CM_24C08,
 EEP_CM_24C16,
 EEP_CM_24C65,
 EEP_TOTAL
};

MD_Cart_Type *MD_Make_Cart_Type_EEPROM(const md_game_info *ginfo, const uint8 *ROM, const uint32 ROM_size, const uint32 iparam, const char *sparam);

#endif
