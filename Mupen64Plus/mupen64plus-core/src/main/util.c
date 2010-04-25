/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - util.c                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
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

/**
 * Provides common utilities to the rest of the code:
 *  -String functions
 *  -Doubly-linked list
 */

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <sys/stat.h>

#include "rom.h"
#include "util.h"
#include "osal/files.h"
#include "osal/preproc.h"

/** trim
 *    Removes leading and trailing whitespace from str. Function modifies str
 *    and also returns modified string.
 */
char *trim(char *str)
{
    unsigned int i;
    char *p = str;

    while (isspace(*p))
        p++;

    if (str != p)
        {
        for (i = 0; i <= strlen(p); ++i)
            str[i]=p[i];
        }

    p = str + strlen(str) - 1;
    if (p > str)
    {
        while (isspace(*p))
            p--;
        p[1] = '\0';
    }

    return str;
}

char* strnstrip(char* string, int size)
{
    int counter, place;
    for (counter = place = 0; counter < size && string[counter] != '\0'; counter++)
        {
        string[place] = string[counter];
        if (string[counter] != ' ')
            place++;
        }
    string[place] = '\0';
    return string;
}

/** file utilities **/

/** copyfile
 *    copies file at src to a new file dest. If dest exists, its contents will be truncated and replaced.
 */
int copyfile(char *src, char *dest)
{
    FILE *to, *from;
    char c;

    if((from = fopen(src, "r")) == NULL)
        return -1;

    if((to = fopen(dest, "w")) == NULL)
    {
        fclose(from);
        return -2;
    }

    while(!feof(from))
    {
        c = fgetc(from);
        if(ferror(from))
        {
            fclose(from);
            fclose(to);
            unlink(dest);
            return -3;
        }
        if(!feof(from))
            fputc(c, to);

        if(ferror(to))
        {
            fclose(from);
            fclose(to);
            unlink(dest);
            return -4;
        }
    }

    fclose(from);
    fclose(to);

    return 0;
}

/** linked list functions **/

/** list_prepend
 *    Allocates a new list node, attaches it to the beginning of list and sets the
 *    node data pointer to data.
 *    Returns - the new list node.
 */
list_node_t *list_prepend(list_t *list, void *data)
{
    list_node_t *new_node,
                *first_node;

    if(list_empty(*list))
    {
        (*list) = (list_t) malloc(sizeof(list_node_t));
        (*list)->data = data;
        (*list)->prev = NULL;
        (*list)->next = NULL;
        return *list;
    }

    // create new node and prepend it to the list
    first_node = *list;
    new_node = (list_node_t *) malloc(sizeof(list_node_t));
    first_node->prev = new_node;
    *list = new_node;

    // set members in new node and return it
    new_node->data = data;
    new_node->prev = NULL;
    new_node->next = first_node;

    return new_node;
}

/** list_append
 *    Allocates a new list node, attaches it to the end of list and sets the
 *    node data pointer to data.
 *    Returns - the new list node.
 */
list_node_t *list_append(list_t *list, void *data)
{
    list_node_t *new_node,
                *last_node;

    if(list_empty(*list))
    {
        (*list) = (list_t) malloc(sizeof(list_node_t));
        (*list)->data = data;
        (*list)->prev = NULL;
        (*list)->next = NULL;
        return *list;
    }

    // find end of list
    last_node = *list;
    while(last_node->next != NULL)
        last_node = last_node->next;

    // create new node and return it
    last_node->next = new_node = (list_node_t *) malloc(sizeof(list_node_t));
    new_node->data = data;
    new_node->prev = last_node;
    new_node->next = NULL;

    return new_node;
}

/** list_node_delete
 *    Deallocates and removes given node from the given list. It is up to the
 *    user to free any memory allocated for the node data before calling this
 *    function. Also, it is assumed that node is an element of list.
 */
void list_node_delete(list_t *list, list_node_t *node)
{
    if(node == NULL || *list == NULL) return;

    if(node->prev != NULL)
        node->prev->next = node->next;
    else
        *list = node->next; // node is first node, update list pointer

    if(node->next != NULL)
        node->next->prev = node->prev;

    free(node);
}

/** list_delete
 *    Deallocates and removes all nodes from the given list. It is up to the
 *    user to free any memory allocated for all node data before calling this
 *    function.
 */
void list_delete(list_t *list)
{
    list_node_t *prev = NULL,
                *curr = NULL;

    // delete all list nodes in the list
    list_foreach(*list, curr)
    {
        if(prev != NULL)
            free(prev);

        prev = curr;
    }
    
    // the last node wasn't deleted, do it now
    if (prev != NULL)
        free(prev);

    *list = NULL;
}

/** list_node_move_front
 *    Moves the given node to the first position of list. It is assumed that
 *    node is an element of list.
 */
void list_node_move_front(list_t *list, list_node_t *node)
{
    list_node_t *tmp;

    if(node == NULL ||
       *list == NULL ||
       node == *list)
        return;

    tmp = *list;
    node->prev->next = node->next;
    if(node->next != NULL)
        node->next->prev = node->prev;
    node->prev = NULL;
    node->next = *list;
    (*list)->prev = node;
    *list = node;
}

/** list_node_move_back
 *    Moves the given node to the last position of list. It is assumed that
 *    node is an element of list.
 */
void list_node_move_back(list_t *list, list_node_t *node)
{
    list_node_t *tmp;

    tmp = list_last_node(*list);

    if(node == NULL ||
       *list == NULL ||
       node == tmp)
        return;

    node->next->prev = node->prev;
    if(node->prev != NULL)
        node->prev->next = node->next;
    else
        *list = node->next; // first node is being moved, update list pointer
    tmp->next = node;
    node->prev = tmp;
    node->next = NULL;
}

/** list_nth_node_data
 *    Returns the nth node in list. If n is out of range, NULL is returned.
 */
void *list_nth_node_data(list_t list, int n)
{
    list_node_t *curr = NULL;

    list_foreach(list, curr)
    {
        if(n-- == 0)
            break;
    }

    return curr != NULL ? curr->data : curr;
}

/** list_first_node
 *    Returns the first node in list.
 */
list_node_t *list_first_node(list_t list)
{
    return list;
}

/** list_first_data
 *    Returns the data pointer of the first node in list.
 */
void *list_first_data(list_t list)
{
    if(list) return list->data;
    return NULL;
}

/** list_last_node
 *    Returns the last node in list.
 */
list_node_t *list_last_node(list_t list)
{
    if(list != NULL)
    {
        while(list->next != NULL)
            list = list->next;
    }

    return list;
}

/** list_last_data
 *    Returns the data pointer of the last node in list.
 */
void *list_last_data(list_t list)
{
    list_node_t *node = list_last_node(list);
    if(node) return node->data;
    return NULL;
}

/** list_empty
 *    Returns 1 if list is empty, else 0.
 */
int list_empty(list_t list)
{
    return list == NULL;
}

/** list_length
 *    Returns the number of elements in list
 */
int list_length(list_t list)
{
    int len = 0;
    list_node_t *curr;

    list_foreach(list, curr)
    {
        len++;
    }

    return len;
}

/** list_find_node
 *    Searches the given list for a node whose data pointer matches the given data pointer.
 *    If found, returns a pointer to the node, else, returns NULL.
 */
list_node_t *list_find_node(list_t list, void *data)
{
    list_node_t *node = NULL;

    list_foreach(list, node)
        if(node->data == data)
            break;

    return node;
}

void countrycodestring(unsigned short countrycode, char *string)
{
    switch (countrycode)
    {
    case 0:    /* Demo */
        strcpy(string, "Demo");
        break;

    case '7':  /* Beta */
        strcpy(string, "Beta");
        break;

    case 0x41: /* Japan / USA */
        strcpy(string, "USA/Japan");
        break;

    case 0x44: /* Germany */
        strcpy(string, "Germany");
        break;

    case 0x45: /* USA */
        strcpy(string, "USA");
        break;

    case 0x46: /* France */
        strcpy(string, "France");
        break;

    case 'I':  /* Italy */
        strcpy(string, "Italy");
        break;

    case 0x4A: /* Japan */
        strcpy(string, "Japan");
        break;

    case 'S':  /* Spain */
        strcpy(string, "Spain");
        break;

    case 0x55: case 0x59:  /* Australia */
        sprintf(string, "Australia (0x%2.2X)", countrycode);
        break;

    case 0x50: case 0x58: case 0x20:
    case 0x21: case 0x38: case 0x70:
        sprintf(string, "Europe (0x%02X)", countrycode);
        break;

    default:
        sprintf(string, "Unknown (0x%02X)", countrycode);
        break;
    }
}

void compressionstring(unsigned char compressiontype, char *string)
{
    switch (compressiontype)
    {
    case UNCOMPRESSED:
        strcpy(string, "Uncompressed");
        break;
    case ZIP_COMPRESSION:
        strcpy(string, "Zip");
        break;
    case GZIP_COMPRESSION:
        strcpy(string, "Gzip");
        break;
    case BZIP2_COMPRESSION:
        strcpy(string, "Bzip2");
        break;
    case LZMA_COMPRESSION:
        strcpy(string, "LZMA");
        break;
    case SZIP_COMPRESSION:
        strcpy(string, "7zip");
        break;
    default:
        string[0] = '\0';
    }
}

void imagestring(unsigned char imagetype, char *string)
{
    switch (imagetype)
    {
    case Z64IMAGE:
        strcpy(string, ".z64 (native)");
        break;
    case V64IMAGE:
        strcpy(string, ".v64 (byteswapped)");
        break;
    case N64IMAGE:
        strcpy(string, ".n64 (wordswapped)");
        break;
    default:
        string[0] = '\0';
    }
}

void cicstring(unsigned char cic, char *string)
{
    switch (cic)
    {
    case CIC_NUS_6101:
        strcpy(string, "CIC-NUS-6101");
        break;
    case CIC_NUS_6102:
        strcpy(string, "CIC-NUS-6102");
        break;
    case CIC_NUS_6103:
        strcpy(string, "CIC-NUS-6103");
        break;
    case CIC_NUS_6105:
        strcpy(string, "CIC-NUS-6105");
        break;
    case CIC_NUS_6106:
        strcpy(string, "CIC-NUS-6106");
        break;
    default:
        string[0] = '\0';
    }
}

void rumblestring(unsigned char rumble, char *string)
{
    switch (rumble)
    {
    case 1:
        strcpy(string, "Yes");
        break;
    case 0:
        strcpy(string, "No");
        break;
    default:
        string[0] = '\0';
    }
}

void savestring(unsigned char savetype, char *string)
{
    switch (savetype)
    {
    case EEPROM_4KB:
        strcpy(string, "Eeprom 4KB");
        break;
    case EEPROM_16KB:
        strcpy(string, "Eeprom 16KB");
        break;
    case SRAM:
        strcpy(string, "SRAM");
        break;
    case FLASH_RAM:
        strcpy(string, "Flash RAM");
        break;
    case CONTROLLER_PACK:
        strcpy(string, "Controller Pack");
        break;
    case NONE:
        strcpy(string, "None");
        break;
    default:
        string[0] = '\0';
    }
}

void playersstring(unsigned char players, char *string)
{
    unsigned short netplay=0;

    if (players > 7)
        {
        string[0] = '\0';
        return;
        }

    if (players > 4)
        {
        players-=3;
        netplay=1;
        }

    sprintf(string, "%d %s", players, (netplay) ? "Netplay" : "");
}

char* dirfrompath(const char* string)
{
    int stringlength, counter;
    char* buffer;

    stringlength = strlen(string);

    for(counter = stringlength; counter > 0; --counter)
        {
        if (string[counter-1] == '/')
            break;
        }

    buffer = (char*)malloc((counter+1)*sizeof(char));
    snprintf(buffer, counter+1, "%s", string);
    buffer[counter] = '\0';

    return buffer;
}

list_t tokenize_string(const char *string, const char* delim)
{
    list_t list = NULL;
    char *token = NULL, *wrk = NULL;
    char buf[4096]; // some of those strings are really long
    strncpy(buf, string, 4096);

    token = strtok(buf, delim);
    if (token)
    {
        wrk = (char *) malloc(strlen(token) + 1);
        strcpy(wrk, token);
        list_append(&list, wrk);
    }

    while ((token = strtok(NULL, delim)))
    {
        wrk = (char *) malloc(strlen(token) + 1);
        strcpy(wrk, token);
        list_append(&list, wrk);
    }
    return list;
}

