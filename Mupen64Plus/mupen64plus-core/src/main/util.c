/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *   Mupen64plus - util.c                                                  *
 *   Mupen64Plus homepage: http://code.google.com/p/mupen64plus/           *
 *   Copyright (C) 2012 CasualJames                                        *
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
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <limits.h>

#include "rom.h"
#include "util.h"
#include "osal/files.h"
#include "osal/preproc.h"

/**********************
     File utilities
 **********************/

file_status_t read_from_file(const char *filename, void *data, size_t size)
{
    FILE *f = fopen(filename, "rb");
    if (f == NULL)
    {
        return file_open_error;
    }

    if (fread(data, 1, size, f) != size)
    {
        fclose(f);
        return file_read_error;
    }

    fclose(f);
    return file_ok;
}

file_status_t write_to_file(const char *filename, const void *data, size_t size)
{
    FILE *f = fopen(filename, "wb");
    if (f == NULL)
    {
        return file_open_error;
    }

    if (fwrite(data, 1, size, f) != size)
    {
        fclose(f);
        return file_read_error;
    }

    fclose(f);
    return file_ok;
}

/**********************
   Byte swap utilities
 **********************/
void swap_buffer(void *buffer, size_t length, size_t count)
{
    size_t i;
    if (length == 2)
    {
        unsigned short *pun = (unsigned short *)buffer;
        for (i = 0; i < count; i++)
            pun[i] = m64p_swap16(pun[i]);
    }
    else if (length == 4)
    {
        unsigned int *pun = (unsigned int *)buffer;
        for (i = 0; i < count; i++)
            pun[i] = m64p_swap32(pun[i]);
    }
    else if (length == 8)
    {
        unsigned long long *pun = (unsigned long long *)buffer;
        for (i = 0; i < count; i++)
            pun[i] = m64p_swap64(pun[i]);
    }
}

void to_little_endian_buffer(void *buffer, size_t length, size_t count)
{
    #ifdef M64P_BIG_ENDIAN
    swap_buffer(buffer, length, count);
    #endif
}

void to_big_endian_buffer(void *buffer, size_t length, size_t count)
{
    #ifndef M64P_BIG_ENDIAN
    swap_buffer(buffer, length, count);
    #endif
}

/**********************
     GUI utilities
 **********************/
void countrycodestring(char countrycode, char *string)
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
        sprintf(string, "Australia (0x%02X)", countrycode);
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

/**********************
     Path utilities
 **********************/

/* Looks for an instance of ANY of the characters in 'needles' in 'haystack',
 * starting from the end of 'haystack'. Returns a pointer to the last position
 * of some character on 'needles' on 'haystack'. If not found, returns NULL.
 */
static const char* strpbrk_reverse(const char* needles, const char* haystack)
{
    size_t stringlength = strlen(haystack), counter;

    for (counter = stringlength; counter > 0; --counter)
    {
        if (strchr(needles, haystack[counter-1]))
            break;
    }

    if (counter == 0)
        return NULL;

    return haystack + counter - 1;
}

const char* namefrompath(const char* path)
{
    const char* last_separator_ptr = strpbrk_reverse(OSAL_DIR_SEPARATORS, path);
    
    if (last_separator_ptr != NULL)
        return last_separator_ptr + 1;
    else
        return path;
}

static int is_path_separator(char c)
{
    return strchr(OSAL_DIR_SEPARATORS, c) != NULL;
}

char* combinepath(const char* first, const char *second)
{
    size_t len_first = strlen(first), off_second = 0;

    if (first == NULL || second == NULL)
        return NULL;

    while (is_path_separator(first[len_first-1]))
        len_first--;

    while (is_path_separator(second[off_second]))
        off_second++;

    return formatstr("%.*s%c%s", (int) len_first, first, OSAL_DIR_SEPARATORS[0], second + off_second);
}

/**********************
    String utilities
 **********************/
char *trim(char *str)
{
    char *start = str, *end = str + strlen(str);

    while (start < end && isspace(*start))
        start++;

    while (end > start && isspace(*(end-1)))
        end--;

    memmove(str, start, end - start);
    str[end - start] = '\0';

    return str;
}

int string_to_int(const char *str, int *result)
{
    char *endptr;
    long int n;
    if (*str == '\0' || isspace(*str))
        return 0;
    errno = 0;
    n = strtol(str, &endptr, 10);
    if (*endptr != '\0' || errno != 0 || n < INT_MIN || n > INT_MAX)
        return 0;
    *result = (int)n;
    return 1;
}

static unsigned char char2hex(char c)
{
    c = tolower(c);
    if(c >= '0' && c <= '9')
        return c - '0';
    else if(c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else
        return 0xFF;
}

int parse_hex(const char *str, unsigned char *output, size_t output_size)
{
    size_t i, j;
    for (i = 0; i < output_size; i++)
    {
        output[i] = 0;
        for (j = 0; j < 2; j++)
        {
            unsigned char h = char2hex(*str++);
            if (h == 0xFF)
                return 0;

            output[i] = (output[i] << 4) | h;
        }
    }

    if (*str != '\0')
        return 0;

    return 1;
}

char *formatstr(const char *fmt, ...)
{
	int size = 128, ret;
	char *str = (char *)malloc(size), *newstr;
	va_list args;

	/* There are two implementations of vsnprintf we have to deal with:
	 * C99 version: Returns the number of characters which would have been written
	 *              if the buffer had been large enough, and -1 on failure.
	 * Windows version: Returns the number of characters actually written,
	 *                  and -1 on failure or truncation.
	 * NOTE: An implementation equivalent to the Windows one appears in glibc <2.1.
	 */
	while (str != NULL)
	{
		va_start(args, fmt);
		ret = vsnprintf(str, size, fmt, args);
		va_end(args);

		// Successful result?
		if (ret >= 0 && ret < size)
			return str;

		// Increment the capacity of the buffer
		if (ret >= size)
			size = ret + 1; // C99 version: We got the needed buffer size
		else
			size *= 2; // Windows version: Keep guessing

		newstr = (char *)realloc(str, size);
		if (newstr == NULL)
			free(str);
		str = newstr;
	}

	return NULL;
}

ini_line ini_parse_line(char **lineptr)
{
    char *line = *lineptr, *endline = strchr(*lineptr, '\n'), *equal;
    ini_line l;

    // Null terminate the current line and point to the next line
    if (endline != NULL)
        *endline = '\0';
    *lineptr = line + strlen(line) + 1;

    // Parse the line contents
    trim(line);

    if (line[0] == '#' || line[0] == ';')
    {
        line++;

        l.type = INI_COMMENT;
        l.name = NULL;
        l.value = trim(line);
    }
    else if (line[0] == '[' && line[strlen(line)-1] == ']')
    {
        line[strlen(line)-1] = '\0';
        line++;

        l.type = INI_SECTION;
        l.name = trim(line);
        l.value = NULL;
    }
    else if ((equal = strchr(line, '=')) != NULL)
    {
        char *name = line, *value = equal + 1;
        *equal = '\0';

        l.type = INI_PROPERTY;
        l.name = trim(name);
        l.value = trim(value);
    }
    else
    {
        l.type = (*line == '\0') ? INI_BLANK : INI_TRASH;
        l.name = NULL;
        l.value = NULL;
    }

    return l;
}
