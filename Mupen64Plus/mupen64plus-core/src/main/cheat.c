/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - cheat.c                                                 *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2009 Richard Goedeken                                   *
 *   Copyright (C) 2008 Okaygo                                             *
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

// gameshark and xploder64 reference: http://doc.kodewerx.net/hacking_n64.html 

#include <SDL.h>
#include <SDL_thread.h>

#include "api/m64p_types.h"
#include "api/callbacks.h"
#include "api/config.h"

#include "memory/memory.h"
#include "osal/preproc.h"
#include "cheat.h"
#include "main.h"
#include "rom.h"
#include "eventloop.h"
#include "util.h" // list utilities
#include "ini_reader.h"

// local definitions
#define CHEAT_CODE_MAGIC_VALUE 0xDEAD0000

typedef struct cheat_code {
    unsigned int address;
    int value;
    int old_value;
} cheat_code_t;

typedef struct cheat {
    char *name;
    int enabled;
    int was_enabled;
    list_t cheat_codes;
} cheat_t;

// local variables
static list_t     active_cheats = NULL;
static SDL_mutex *cheat_mutex = NULL;

// private functions
static unsigned short read_address_16bit(unsigned int address)
{
    return *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16)));
}

static unsigned char read_address_8bit(unsigned int address)
{
    return *(unsigned char *)((rdramb + ((address & 0xFFFFFF)^S8)));
}

static void update_address_16bit(unsigned int address, unsigned short new_value)
{
    *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16))) = new_value;
}

static void update_address_8bit(unsigned int address, unsigned char new_value)
{
    *(unsigned char *)((rdramb + ((address & 0xFFFFFF)^S8))) = new_value;
}

static int address_equal_to_8bit(unsigned int address, unsigned char value)
{
    unsigned char value_read;
    value_read = *(unsigned char *)((rdramb + ((address & 0xFFFFFF)^S8)));
    return value_read == value;
}

static int address_equal_to_16bit(unsigned int address, unsigned short value)
{
    unsigned short value_read;
    value_read = *(unsigned short *)((rdramb + ((address & 0xFFFFFF)^S16)));
    return value_read == value;
}

// individual application - returns 0 if we are supposed to skip the next cheat
// (only really used on conditional codes)
static int execute_cheat(unsigned int address, unsigned short value, int *old_value)
{
    switch (address & 0xFF000000)
    {
        case 0x80000000:
        case 0x88000000:
        case 0xA0000000:
        case 0xA8000000:
        case 0xF0000000:
            // if pointer to old value is valid and uninitialized, write current value to it
            if(old_value && (*old_value == CHEAT_CODE_MAGIC_VALUE))
                *old_value = (int) read_address_8bit(address);
            update_address_8bit(address,(unsigned char) value);
            return 1;
        case 0x81000000:
        case 0x89000000:
        case 0xA1000000:
        case 0xA9000000:
        case 0xF1000000:
            // if pointer to old value is valid and uninitialized, write current value to it
            if(old_value && (*old_value == CHEAT_CODE_MAGIC_VALUE))
                *old_value = (int) read_address_16bit(address);
            update_address_16bit(address,value);
            return 1;
        case 0xD0000000:
        case 0xD8000000:
            return address_equal_to_8bit(address,(unsigned char) value);
        case 0xD1000000:
        case 0xD9000000:
            return address_equal_to_16bit(address,value);
        case 0xD2000000:
        case 0xDB000000:
            return !(address_equal_to_8bit(address,(unsigned char) value));
        case 0xD3000000:
        case 0xDA000000:
            return !(address_equal_to_16bit(address,value));
        case 0xEE000000:
            // most likely, this doesnt do anything.
            execute_cheat(0xF1000318, 0x0040, NULL);
            execute_cheat(0xF100031A, 0x0000, NULL);
            return 1;
        default:
            return 1;
    }
}

static cheat_t *find_or_create_cheat(const char *name)
{
    list_t node = NULL;
    cheat_t *cheat = NULL;
    int found = 0;

    list_foreach(active_cheats, node) {
        cheat = (cheat_t *) node->data;
        if (strcmp(cheat->name, name) == 0) {
            found = 1;
            break;
        }
    }

    if (found)
    {
        /* delete any pre-existing cheat codes */
        list_t node2;
        list_foreach(cheat->cheat_codes, node2)
        {
            free(node2->data);
        }
        list_delete(&cheat->cheat_codes);
        cheat->enabled = 0;
        cheat->was_enabled = 0;
    }
    else
    {
        cheat = (cheat_t *) malloc(sizeof(cheat_t));
        cheat->name = strdup(name);
        cheat->cheat_codes = NULL;
        cheat->enabled = 0;
        cheat->was_enabled = 0;
        list_append(&active_cheats, cheat);
    }

    return cheat;
}


// public functions
void cheat_init(void)
{
    cheat_mutex = SDL_CreateMutex();
}

void cheat_uninit(void)
{
    if (cheat_mutex != NULL)
        SDL_DestroyMutex(cheat_mutex);
    cheat_mutex = NULL;
}

void cheat_apply_cheats(int entry)
{
    list_t node1 = NULL;
    list_t node2 = NULL;
    cheat_code_t *code;
    
    if (active_cheats == NULL)
        return;

    if (cheat_mutex == NULL || SDL_LockMutex(cheat_mutex) != 0)
    {
        DebugMessage(M64MSG_ERROR, "Internal error: failed to lock mutex in cheat_apply_cheats()");
        return;
    }

    list_foreach(active_cheats, node1) {
        cheat_t* cheat = (cheat_t*) node1->data;
        if (cheat->enabled)
        {
            cheat->was_enabled = 1;
            switch(entry)
            {
                case ENTRY_BOOT:
                    list_foreach(cheat->cheat_codes, node2)
                    {
                        code = (cheat_code_t *)node2->data;

                        // code should only be written once at boot time
                        if((code->address & 0xF0000000) == 0xF0000000)
                            execute_cheat(code->address, code->value, &code->old_value);
                    }
                    break;
                case ENTRY_VI:
                    list_foreach(cheat->cheat_codes, node2)
                    {
                        code = (cheat_code_t *)node2->data;

                        // conditional cheat codes
                        if((code->address & 0xF0000000) == 0xD0000000)
                        {
                            // if code needs GS button pressed and it's not, skip it
                            if(((code->address & 0xFF000000) == 0xD8000000 ||
                                (code->address & 0xFF000000) == 0xD9000000 ||
                                (code->address & 0xFF000000) == 0xDA000000 ||
                                (code->address & 0xFF000000) == 0xDB000000) &&
                               !event_gameshark_active())
                            {
                                // skip next code
                                if(node2->next != NULL)
                                    node2 = node2->next;
                                continue;
                            }

                            // if condition true, execute next cheat code
                            if(execute_cheat(code->address, code->value, NULL))
                            {
                                node2 = node2->next;
                                code = (cheat_code_t *)node2->data;

                                // if code needs GS button pressed, don't save old value
                                if(((code->address & 0xFF000000) == 0xD8000000 ||
                                    (code->address & 0xFF000000) == 0xD9000000 ||
                                    (code->address & 0xFF000000) == 0xDA000000 ||
                                    (code->address & 0xFF000000) == 0xDB000000))
                                   execute_cheat(code->address, code->value, NULL);
                                else
                                   execute_cheat(code->address, code->value, &code->old_value);
                            }
                            // if condition false, skip next code
                            else
                            {
                                if(node2->next != NULL)
                                    node2 = node2->next;
                                continue;
                            }
                        }
                        // GS button triggers cheat code
                        else if((code->address & 0xFF000000) == 0x88000000 ||
                                (code->address & 0xFF000000) == 0x89000000 ||
                                (code->address & 0xFF000000) == 0xA8000000 ||
                                (code->address & 0xFF000000) == 0xA9000000)
                        {
                            if(event_gameshark_active())
                                execute_cheat(code->address, code->value, NULL);
                        }
                        // normal cheat code
                        else
                        {
                            // exclude boot-time cheat codes
                            if((code->address & 0xF0000000) != 0xF0000000)
                                execute_cheat(code->address, code->value, &code->old_value);
                        }
                    }
                    break;
                default:
                    break;
            }
        }
        // if cheat was enabled, but is now disabled, restore old memory values
        else if (cheat->was_enabled)
        {
            cheat->was_enabled = 0;
            switch(entry)
            {
                case ENTRY_VI:
                    list_foreach(cheat->cheat_codes, node2)
                    {
                        code = (cheat_code_t *)node2->data;
              
                        // set memory back to old value and clear saved copy of old value
                        if(code->old_value != CHEAT_CODE_MAGIC_VALUE)
                        {
                            execute_cheat(code->address, code->old_value, NULL);
                            code->old_value = CHEAT_CODE_MAGIC_VALUE;
                        }
                    }
                    break;
                default:
                    break;
            }
        }
    }

    SDL_UnlockMutex(cheat_mutex);
}


void cheat_delete_all(void)
{
    cheat_t *cheat = NULL;
    list_t node1, node2;
    node1 = node2 = NULL;

    if (active_cheats == NULL)
        return;

    if (cheat_mutex == NULL || SDL_LockMutex(cheat_mutex) != 0)
    {
        DebugMessage(M64MSG_ERROR, "Internal error: failed to lock mutex in cheat_delete_all()");
        return;
    }

    list_foreach(active_cheats, node1)
    {
        cheat = (cheat_t *) node1->data;
        
        free(cheat->name);

        list_foreach(cheat->cheat_codes, node2)
        {
            free(node2->data);
        }
        list_delete(&cheat->cheat_codes);
    }

    list_delete(&active_cheats);
    SDL_UnlockMutex(cheat_mutex);
}

int cheat_set_enabled(const char *name, int enabled)
{
    cheat_t *cheat = NULL;
    list_t node = NULL;

    if (active_cheats == NULL)
        return 0;

    if (cheat_mutex == NULL || SDL_LockMutex(cheat_mutex) != 0)
    {
        DebugMessage(M64MSG_ERROR, "Internal error: failed to lock mutex in cheat_set_enabled()");
        return 0;
    }

    list_foreach(active_cheats, node)
    {
        cheat = (cheat_t *) node->data;

        if (strcmp(name, cheat->name) == 0)
        {
            cheat->enabled = enabled;
            SDL_UnlockMutex(cheat_mutex);
            return 1;
        }
    }

    SDL_UnlockMutex(cheat_mutex);
    return 0;
}

int cheat_add_new(const char *name, m64p_cheat_code *code_list, int num_codes)
{
    cheat_t *cheat;
    int i, j;

    if (cheat_mutex == NULL || SDL_LockMutex(cheat_mutex) != 0)
    {
        DebugMessage(M64MSG_ERROR, "Internal error: failed to lock mutex in cheat_add_new()");
        return 0;
    }

    /* create a new cheat function or erase the codes in an existing cheat function */
    cheat = find_or_create_cheat(name);
    if (cheat == NULL)
    {
        SDL_UnlockMutex(cheat_mutex);
        return 0;
    }

    cheat->enabled = 1; /* default for new cheats is enabled */

    for (i = 0; i < num_codes; i++)
    {
        /* if this is a 'patch' code, convert it and dump out all of the individual codes */
        if ((code_list[i].address & 0xFFFF0000) == 0x50000000 && i < num_codes - 1)
        {
            int code_count = ((code_list[i].address & 0xFF00) >> 8);
            int incr_addr = code_list[i].address & 0xFF;
            int incr_value = code_list[i].value;
            int cur_addr = code_list[i+1].address;
            int cur_value = code_list[i+1].value;
            i += 1;
            for (j = 0; j < code_count; j++)
            {
                cheat_code_t *code = (cheat_code_t *) malloc(sizeof(cheat_code_t));
                code->address = cur_addr;
                code->value = cur_value;
                code->old_value = CHEAT_CODE_MAGIC_VALUE;
                list_append(&cheat->cheat_codes, code);
                cur_addr += incr_addr;
                cur_value += incr_value;
            }
        }
        else
        { /* just a normal code */
            cheat_code_t *code = (cheat_code_t *) malloc(sizeof(cheat_code_t));
            code->address = code_list[i].address;
            code->value = code_list[i].value;
            code->old_value = CHEAT_CODE_MAGIC_VALUE;
            list_append(&cheat->cheat_codes, code);
        }
    }

    SDL_UnlockMutex(cheat_mutex);
    return 1;
}


