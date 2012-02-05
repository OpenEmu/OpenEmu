/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - rom.c                                                   *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2008 Tillin9                                            *
 *   Copyright (C) 2002 Hacktarux                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.          *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define M64P_CORE_PROTOTYPES 1
#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "api/config.h"
#include "api/m64p_config.h"

#include "md5.h"
#include "rom.h"
#include "main.h"
#include "util.h"

#include "memory/memory.h"
#include "osal/preproc.h"
#include "osd/osd.h"

#define DEFAULT 16

#define CHUNKSIZE 1024*128 /* Read files 128KB at a time. */

static _romdatabase g_romdatabase;
romdatabase_entry empty_entry;

/* Global loaded rom memory space. */
unsigned char* rom = NULL;
/* Global loaded rom size. */
int rom_size = 0;

unsigned char isGoldeneyeRom = 0;

rom_header*       ROM_HEADER;
m64p_rom_settings ROM_SETTINGS;

/* Tests if a file is a valid N64 rom by checking the first 4 bytes. */
static int is_valid_rom(const unsigned char *buffer)
{
    /* Test if rom is a native .z64 image with header 0x80371240. [ABCD] */
    if((buffer[0]==0x80)&&(buffer[1]==0x37)&&(buffer[2]==0x12)&&(buffer[3]==0x40))
        return 1;
    /* Test if rom is a byteswapped .v64 image with header 0x37804012. [BADC] */
    else if((buffer[0]==0x37)&&(buffer[1]==0x80)&&(buffer[2]==0x40)&&(buffer[3]==0x12))
        return 1;
    /* Test if rom is a wordswapped .n64 image with header  0x40123780. [DCBA] */
    else if((buffer[0]==0x40)&&(buffer[1]==0x12)&&(buffer[2]==0x37)&&(buffer[3]==0x80))
        return 1;
    else
        return 0;
}

/* If rom is a .v64 or .n64 image, byteswap or wordswap loadlength amount of
 * rom data to native .z64 before forwarding. Makes sure that data extraction
 * and MD5ing routines always deal with a .z64 image.
 */
static void swap_rom(unsigned char* localrom, unsigned char* imagetype, int loadlength)
{
    unsigned char temp;
    int i;

    /* Btyeswap if .v64 image. */
    if(localrom[0]==0x37)
        {
        *imagetype = V64IMAGE;
        for (i = 0; i < loadlength; i+=2)
            {
            temp=localrom[i];
            localrom[i]=localrom[i+1];
            localrom[i+1]=temp;
            }
        }
    /* Wordswap if .n64 image. */
    else if(localrom[0]==0x40)
        {
        *imagetype = N64IMAGE;
        for (i = 0; i < loadlength; i+=4)
            {
            temp=localrom[i];
            localrom[i]=localrom[i+3];
            localrom[i+3]=temp;
            temp=localrom[i+1];
            localrom[i+1]=localrom[i+2];
            localrom[i+2]=temp;
            }
        }
    else
        *imagetype = Z64IMAGE;
}

m64p_error open_rom(const unsigned char* romimage, unsigned int size)
{
    md5_state_t state;
    md5_byte_t digest[16];
    romdatabase_entry* entry;
    char buffer[256];
    unsigned char imagetype;
    int i;

    /* check input requirements */
    if (rom != NULL)
    {
        DebugMessage(M64MSG_ERROR, "open_rom(): previous ROM image was not freed");
        return M64ERR_INTERNAL;
    }
    if (romimage == NULL || !is_valid_rom(romimage))
    {
        DebugMessage(M64MSG_ERROR, "open_rom(): not a valid ROM image");
        return M64ERR_INPUT_INVALID;
    }

    /* Clear Byte-swapped flag, since ROM is now deleted. */
    g_MemHasBeenBSwapped = 0;
    /* allocate new buffer for ROM and copy into this buffer */
    rom_size = size;
    rom = (unsigned char *) malloc(size);
    if (rom == NULL)
        return M64ERR_NO_MEMORY;
    memcpy(rom, romimage, size);
    swap_rom(rom, &imagetype, rom_size);

    /* Calculate MD5 hash  */
    md5_init(&state);
    md5_append(&state, (const md5_byte_t*)rom, rom_size);
    md5_finish(&state, digest);
    for ( i = 0; i < 16; ++i )
        sprintf(buffer+i*2, "%02X", digest[i]);
    buffer[32] = '\0';
    strcpy(ROM_SETTINGS.MD5, buffer);

    /* allocate space for ROM_HEADER object, and copy the first N bytes into new buffer */
    if(ROM_HEADER)
        free(ROM_HEADER);
    ROM_HEADER = (rom_header *) malloc(sizeof(rom_header));
    if(ROM_HEADER==NULL)
        return M64ERR_NO_MEMORY;
    memcpy(ROM_HEADER, rom, sizeof(rom_header));

    /* Remove trailing whitespace from ROM name. */
    trim((char*)ROM_HEADER->nom);

    /* Look up this ROM in the .ini file and fill in goodname, etc */
    if ((entry=ini_search_by_md5(digest)) != &empty_entry ||
        (entry=ini_search_by_crc(sl(ROM_HEADER->CRC1),sl(ROM_HEADER->CRC2))) != &empty_entry)
    {
        strncpy(ROM_SETTINGS.goodname, entry->goodname, 255);
        ROM_SETTINGS.goodname[255] = '\0';
        ROM_SETTINGS.savetype = entry->savetype;
        ROM_SETTINGS.status = entry->status;
        ROM_SETTINGS.players = entry->players;
        ROM_SETTINGS.rumble = entry->rumble;
    }
    else
    {
        strcpy(ROM_SETTINGS.goodname, (char*)ROM_HEADER->nom);
        strcat(ROM_SETTINGS.goodname, " (unknown rom)");
        ROM_SETTINGS.savetype = NONE;
        ROM_SETTINGS.status = 0;
        ROM_SETTINGS.players = 0;
        ROM_SETTINGS.rumble = 0;
    }

    /* print out a bunch of info about the ROM */
    DebugMessage(M64MSG_INFO, "Goodname: %s", ROM_SETTINGS.goodname);
    DebugMessage(M64MSG_INFO, "Name: %s", ROM_HEADER->nom);
    imagestring(imagetype, buffer);
    DebugMessage(M64MSG_INFO, "MD5: %s", ROM_SETTINGS.MD5);
    DebugMessage(M64MSG_INFO, "CRC: %x %x", sl((unsigned int)ROM_HEADER->CRC1), sl((unsigned int)ROM_HEADER->CRC2));
    DebugMessage(M64MSG_INFO, "Imagetype: %s", buffer);
    DebugMessage(M64MSG_INFO, "Rom size: %d bytes (or %d Mb or %d Megabits)", rom_size, rom_size/1024/1024, rom_size/1024/1024*8);
    DebugMessage(M64MSG_VERBOSE, "ClockRate = %x", sl((unsigned int)ROM_HEADER->ClockRate));
    DebugMessage(M64MSG_INFO, "Version: %x", sl((unsigned int)ROM_HEADER->Release));
    if(sl(ROM_HEADER->Manufacturer_ID) == 'N')
        DebugMessage(M64MSG_INFO, "Manufacturer: Nintendo");
    else
        DebugMessage(M64MSG_INFO, "Manufacturer: %x", (unsigned int)(ROM_HEADER->Manufacturer_ID));
    DebugMessage(M64MSG_VERBOSE, "Cartridge_ID: %x", ROM_HEADER->Cartridge_ID);
    countrycodestring(ROM_HEADER->Country_code, buffer);
    DebugMessage(M64MSG_INFO, "Country: %s", buffer);
    DebugMessage(M64MSG_VERBOSE, "PC = %x", sl((unsigned int)ROM_HEADER->PC));
    DebugMessage(M64MSG_VERBOSE, "Save type: %d", ROM_SETTINGS.savetype);

    //Prepare Hack for GOLDENEYE
    isGoldeneyeRom = 0;
    if(strncmp((char *) ROM_HEADER->nom, "GOLDENEYE",9) == 0)
       isGoldeneyeRom = 1;

    return M64ERR_SUCCESS;
}

m64p_error close_rom(void)
{
    if (rom == NULL)
        return M64ERR_INVALID_STATE;

    free(rom);
    rom = NULL;

    if (ROM_HEADER)
    {
        free(ROM_HEADER);
        ROM_HEADER = NULL;
    }

    /* Clear Byte-swapped flag, since ROM is now deleted. */
    g_MemHasBeenBSwapped = 0;
    DebugMessage(M64MSG_STATUS, "Rom closed.");

    return M64ERR_SUCCESS;
}

/********************************************************************************************/
/* INI Rom database functions */

/* Convert two letters representing hexidecimal to the appropriate value: 00->0 - FF->255. */
static unsigned char hexconvert(const char* bigraph)
{
    unsigned char returnvalue = 0;
    char character;
    int digits;

    for(digits = 2; digits > 0; --digits)
        {
        returnvalue = returnvalue << 4;
        character = *bigraph++ | 0x20;
        if(character>='0'&&character<='9')
            returnvalue += character - '0';
        else if(character>='a'&&character<='f')
            returnvalue += character - 'a' + 10;
        }

    return returnvalue;
}

/* Helper function, identify the space of a line before an = sign. */
static int split_property(char* string)
{
    int counter = 0;
    while (string[counter] != '=' && string[counter] != '\0')
        ++counter;
    if(string[counter]=='\0')
        return -1;
    string[counter] = '\0';
    return counter;
}

void romdatabase_open(void)
{
    FILE *fPtr;
    char buffer[256];
    romdatabase_search* search = NULL;
    romdatabase_entry* entry = NULL;

    int stringlength, totallength, namelength, index, counter, value;
    char hashtemp[3] = {0,0,0};
    const char *pathname = ConfigGetSharedDataFilepath("mupen64plus.ini");

    if(g_romdatabase.comment!=NULL)
        return;

    /* Setup empty_entry. */
    empty_entry.goodname = "";
    for(counter=0; counter<16; ++counter)
       empty_entry.md5[counter]=0;
    empty_entry.refmd5 = NULL;
    empty_entry.crc1 = 0;
    empty_entry.crc2 = 0;
    empty_entry.status = 0;
    empty_entry.savetype = DEFAULT;
    empty_entry.players = DEFAULT;
    empty_entry.rumble = DEFAULT;

    /* Open romdatabase. */
    if (pathname == NULL || (fPtr = fopen(pathname, "rb")) == NULL)
    {
        DebugMessage(M64MSG_ERROR, "Unable to open rom database file '%s'.", pathname);
        return;
    }

    /* Move through opening comments, set romdatabase.comment to non-NULL
    to signal we have a database. */
    totallength = 0;
    do
    {
        if (fgets(buffer, 255, fPtr) == NULL)
        {
            DebugMessage(M64MSG_ERROR, "Error reading rom database file '%s'.", pathname);
            return;
        }
        if(buffer[0]!='[')
        {
            stringlength=strlen(buffer);
            totallength+=stringlength;
            if(g_romdatabase.comment==NULL) 
            {
                g_romdatabase.comment = (char*)malloc(stringlength+2);
                snprintf(g_romdatabase.comment, stringlength, "%s", buffer);
            }
            else
            {
                g_romdatabase.comment = (char*)realloc(g_romdatabase.comment, totallength+2);
                snprintf(g_romdatabase.comment, totallength+1, "%s%s", g_romdatabase.comment, buffer);
            }
        }
    }
    while (buffer[0] != '[' && !feof(fPtr));

    /* Clear premade indices. */
    for(counter = 0; counter < 255; ++counter)
        g_romdatabase.crc_lists[counter] = NULL;
    for(counter = 0; counter < 255; ++counter)
        g_romdatabase.md5_lists[counter] = NULL;
    g_romdatabase.list = NULL;

    do
    {
        if(buffer[0]=='[')
            {
            if(g_romdatabase.list==NULL)
                {
                g_romdatabase.list = (romdatabase_search*)malloc(sizeof(romdatabase_search));
                g_romdatabase.list->next_entry = NULL;
                g_romdatabase.list->next_crc = NULL;
                g_romdatabase.list->next_md5 = NULL;
                search = g_romdatabase.list;
                }
            else
                {
                search->next_entry = (romdatabase_search*)malloc(sizeof(romdatabase_search));
                search = search->next_entry;
                search->next_entry = NULL;
                search->next_crc = NULL;
                search->next_md5 = NULL;
                }
            for (counter=0; counter < 16; ++counter)
              {
              hashtemp[0] = buffer[counter*2+1];
              hashtemp[1] = buffer[counter*2+2];
              search->entry.md5[counter] = hexconvert(hashtemp);
              }
            /* Index MD5s by first 8 bits. */
            index = search->entry.md5[0];
            if(g_romdatabase.md5_lists[index]==NULL)
                g_romdatabase.md5_lists[index] = search;
            else
                {
                romdatabase_search* aux = g_romdatabase.md5_lists[index];
                search->next_md5 = aux;
                g_romdatabase.md5_lists[index] = search;
                }
            search->entry.goodname = NULL;
            search->entry.refmd5 = NULL;
            search->entry.crc1 = 0;
            search->entry.crc2 = 0;
            search->entry.status = 0; /* Set default to 0 stars. */
            search->entry.savetype = DEFAULT;
            search->entry.rumble = DEFAULT; 
            search->entry.players = DEFAULT; 
            }
        else
            {
            stringlength = split_property(buffer);
            if(stringlength!=-1)
                {
                if(!strcmp(buffer, "GoodName"))
                    {
                    /* Get length of GoodName since we dynamically allocate. */
                    namelength = strlen(buffer+stringlength+1);
                    search->entry.goodname = (char*)malloc(namelength*sizeof(char));
                    /* Make sure we're null terminated. */
                    if(buffer[stringlength+namelength]=='\n'||buffer[stringlength+namelength]=='\r')
                        buffer[stringlength+namelength] = '\0';
                    snprintf(search->entry.goodname, namelength, "%s", buffer+stringlength+1);
                    }
                else if(!strcmp(buffer, "CRC"))
                    {
                    buffer[stringlength+19] = '\0';
                    sscanf(buffer+stringlength+10, "%X", &search->entry.crc2);
                    buffer[stringlength+9] = '\0';
                    sscanf(buffer+stringlength+1, "%X", &search->entry.crc1);
                    buffer[stringlength+3] = '\0';
                    index = hexconvert(buffer+stringlength+1);
                    /* Index CRCs by first 8 bits. */
                    if(g_romdatabase.crc_lists[index]==NULL)
                        g_romdatabase.crc_lists[index] = search;
                    else
                        {
                        romdatabase_search* aux = g_romdatabase.crc_lists[index];
                        search->next_crc = aux;
                        g_romdatabase.crc_lists[index] = search;
                        }
                    }
                else if(!strcmp(buffer, "RefMD5"))
                    {
                    /* If we have a refernce MD5, dynamically allocate. */
                    search->entry.refmd5 = (md5_byte_t*)malloc(16*sizeof(md5_byte_t));
                    for (counter=0; counter < 16; ++counter)
                        {
                        hashtemp[0] = buffer[stringlength+1+counter*2];
                        hashtemp[1] = buffer[stringlength+2+counter*2];
                        search->entry.refmd5[counter] = hexconvert(hashtemp);
                        }
                    /* Lookup reference MD5 and replace non-default entries. */
                    if((entry = ini_search_by_md5(search->entry.refmd5))!=&empty_entry)
                        {
                        if(entry->savetype!=DEFAULT)
                            search->entry.savetype = entry->savetype;
                        if(entry->status!=0)
                            search->entry.status = entry->status;
                        if(entry->players!=DEFAULT)
                            search->entry.players = entry->players;
                        if(entry->rumble!=DEFAULT)
                            search->entry.rumble = entry->rumble;
                        }
                    }
                else if(!strcmp(buffer, "SaveType"))
                    {
                    if(!strncmp(buffer+stringlength+1, "Eeprom 4KB", 10))
                        search->entry.savetype = EEPROM_4KB;
                    else if(!strncmp(buffer+stringlength+1, "Eeprom 16KB", 10))
                        search->entry.savetype = EEPROM_16KB;
                    else if(!strncmp(buffer+stringlength+1, "SRAM", 4))
                        search->entry.savetype = SRAM;
                    else if(!strncmp(buffer+stringlength+1, "Flash RAM", 9))
                        search->entry.savetype = FLASH_RAM;
                    else if(!strncmp(buffer+stringlength+1, "Controller Pack", 15))
                        search->entry.savetype = CONTROLLER_PACK;
                    else if(!strncmp(buffer+stringlength+1, "None", 4))
                        search->entry.savetype = NONE;
                    }
                else if(!strcmp(buffer, "Status"))
                    {
                    value = (unsigned char)atoi(buffer+stringlength+1);
                    if(value>-1&&value<6)
                        search->entry.status = value;
                    }
                else if(!strcmp(buffer, "Players"))
                    {
                    value = (unsigned char)atoi(buffer+stringlength+1);
                    if(value>-1&&value<8)
                        search->entry.players = value;
                    }
                else if(!strcmp(buffer, "Rumble"))
                    {
                    if(!strncmp(buffer+stringlength+1, "Yes", 3))
                        search->entry.rumble = 1;
                    if(!strncmp(buffer+stringlength+1, "No", 2))
                        search->entry.rumble = 0;
                    }
                }
            }
        if (fgets(buffer, 255, fPtr) == NULL)
            break;
    } while (!feof(fPtr));

    fclose(fPtr);
}

void romdatabase_close(void)
{
    if (g_romdatabase.comment == NULL)
        return;

    free(g_romdatabase.comment);

    while (g_romdatabase.list != NULL)
        {
        romdatabase_search* search = g_romdatabase.list->next_entry;
        if(g_romdatabase.list->entry.goodname)
            free(g_romdatabase.list->entry.goodname);
        if(g_romdatabase.list->entry.refmd5)
            free(g_romdatabase.list->entry.refmd5);
        free(g_romdatabase.list);
        g_romdatabase.list = search;
        }
}

romdatabase_entry* ini_search_by_md5(md5_byte_t* md5)
{
    romdatabase_search* search;

    if(g_romdatabase.comment==NULL)
        return &empty_entry;

    search = g_romdatabase.md5_lists[md5[0]];

    while (search != NULL && memcmp(search->entry.md5, md5, 16) != 0)
        search = search->next_md5;

    if(search==NULL)
        return &empty_entry;
    else
        return &(search->entry);
}

romdatabase_entry* ini_search_by_crc(unsigned int crc1, unsigned int crc2)
{
    romdatabase_search* search;

    if(g_romdatabase.comment==NULL) 
        return &empty_entry;

    search = g_romdatabase.crc_lists[((crc1 >> 24) & 0xff)];

    while (search != NULL && search->entry.crc1 != crc1 && search->entry.crc2 != crc2)
        search = search->next_crc;

    if(search == NULL) 
        return &empty_entry;
    else
        return &(search->entry);
}


