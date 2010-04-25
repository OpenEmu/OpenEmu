/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
*   Mupen64plus - ini_reader.c                                            *
*   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
*   Copyright (C) 2008 slougi                                             *
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

#include "api/m64p_types.h"
#include "api/callbacks.h"

#include "ini_reader.h"

#define BUF_MAX 4096

static int iscomment(char *line);
static ini_entry *parse_entry(char *line);
static ini_section *parse_section(char *line);

static ini_entry *parse_entry(char *line)
{
    int i;
    char *equals_sign = NULL;
    ini_entry *entry = NULL;

    char key[BUF_MAX];
    char value[BUF_MAX];

    memset(key, '\0', BUF_MAX);
    memset(value, '\0', BUF_MAX);

    if ((equals_sign = strstr(line, "=")) != NULL)
    {
        entry = (ini_entry *) malloc(sizeof(ini_entry));
        for (i = 0; line != equals_sign; i++, line++)
        {
            key[i] = *line;
        }
        line = equals_sign + 1;
        strncpy(value, line, BUF_MAX);

        trim(key);
        entry->key = (char *) malloc(strlen(key) + 1);
        strcpy(entry->key, key);

        trim(value);
        entry->value = (char *) malloc(strlen(value) + 1);
        strcpy(entry->value, value);
    }

    return entry;
}

static ini_section *parse_section(char *line)
{
    int i = 0;
    char buf[BUF_MAX];
    char *opening_bracket = NULL;
    char *closing_bracket = NULL;
    ini_section *section = NULL;

    memset(buf, '\0', BUF_MAX);

    if ((opening_bracket = strstr(line, "[")) == line)
    {
        if ((closing_bracket = strstr(line, "]")) != NULL)
        {
            while (++opening_bracket != closing_bracket)
            {
                buf[i++] = *opening_bracket;
            }
            section = (ini_section *) malloc(sizeof(ini_section));
            section->title = (char *) malloc(strlen(buf) + 1);
            strcpy(section->title, buf);
            section->entries = NULL;
        }
    }

    return section;
}

static int iscomment(char *line)
{
    int result = 0;

    if (strstr(line, "//") == line)
    {
        result = 1;
    }
    else if (strstr(line, "#") == line)
    {
        result = 1;
    }

    return result;
}

ini_file* ini_file_parse(const char* filename)
{
    int i = 0;
    ini_file *res = NULL;
    list_t sections = NULL;
    ini_entry* entry = NULL;
    ini_section* section = NULL;
    FILE *fp = fopen(filename, "r");
    char buf[BUF_MAX];

    if (!fp)
    {
        return NULL;
    }

    while (fgets(buf, BUF_MAX, fp)) {
        i++;

        trim(buf);
        if (strlen(buf) == 0)
        {
            continue;
        }

        if ((section = parse_section(buf)))
        {
            list_append(&sections, section);
        }
        else if ((entry = parse_entry(buf)))
        {
            section = (ini_section *) list_last_data(sections);
            if (section) {
                list_append(&section->entries, entry);
            }
        }
        else if (iscomment(buf))
        {
            continue;
        }
        else
        {
            DebugMessage(M64MSG_ERROR, "Couldn't parse line %d in file %s: %s", i, filename, buf);
        }
    }

    if (!list_empty(sections))
    {
        res = (ini_file *) malloc(sizeof(ini_file));
        res->filename = (char *) malloc(strlen(filename) + 1);
        strcpy(res->filename, filename);
        res->sections = sections;
    }

    fclose(fp);
    return res;
}

void ini_file_free(ini_file **ini)
{
    list_t p1, p2;
    ini_section* section;
    ini_entry* entry;

    if (ini == NULL || *ini == NULL)
        return;

    free((*ini)->filename);

    list_foreach((*ini)->sections, p1)
    {
        section = (ini_section *) p1->data;
        free(section->title);
        list_foreach(section->entries, p2)
        {
            entry = (ini_entry *) p2->data;
            free(entry->key);
            free(entry->value);
        }
    }
    free(*ini);
    *ini = 0;
}

