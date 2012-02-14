/*
* util.c - utility functions
*
* Copyright (c) 2005 Piotr Fusik
* Copyright (c) 2005 Atari800 development team (see DOC/CREDITS)
*
* This file is part of the Atari800 emulator project which emulates
* the Atari 400, 800, 800XL, 130XE, and 5200 8-bit computers.
*
* Atari800 is free software; you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation; either version 2 of the License, or
* (at your option) any later version.
*
* Atari800 is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with Atari800; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <ctype.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "wii_util.h"

void Util_chomp(char *s)
{
  int len;

  len = strlen(s);
  if (len >= 2 && s[len - 1] == '\n' && s[len - 2] == '\r')
    s[len - 2] = '\0';
  else if (len >= 1 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
    s[len - 1] = '\0';
}

void Util_trim(char *s)
{
  char *p = s;
  char *q;
  /* skip leading whitespace */
  while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n')
    p++;
  /* now p points at the first non-whitespace character */

  if (*p == '\0') {
    /* only whitespace */
    *s = '\0';
    return;
  }

  q = s + strlen(s);
  /* skip trailing whitespace */
  /* we have found p < q such that *p is non-whitespace,
  so this loop terminates with q >= p */
  do
  q--;
  while (*q == ' ' || *q == '\t' || *q == '\r' || *q == '\n');

  /* now q points at the last non-whitespace character */
  /* cut off trailing whitespace */
  *++q = '\0';

  /* move to string */
  memmove(s, p, q + 1 - p);
}

int Util_sscandec(const char *s)
{
  int neg = 0;
  int result;
  if (*s == '\0')
    return -1;
  result = 0;
  for (;;) {
    if (*s >= '0' && *s <= '9')
      result = 10 * result + *s - '0';
    else if (*s == '\0')
      return neg ? -result : result;
    else if (*s == '-')
      neg = 1;
    else
      return -1;
    s++;
  }
}

char *Util_strlcpy(char *dest, const char *src, size_t size)
{
  strncpy(dest, src, size);
  dest[size - 1] = '\0';
  return dest;
}

int Util_fileexists( char *filename )
{
  struct stat buf;
  int i = stat( filename, &buf );       
  return i == 0;
}

void Util_splitpath(const char *path, char *dir_part, char *file_part)
{
  const char *p;
  /* find the last DIR_SEP_CHAR except the last character */
  for (p = path + strlen(path) - 2; p >= path; p--) {
    if (*p == DIR_SEP_CHAR
#ifdef BACK_SLASH
      /* on DOSish systems slash can be also used as a directory separator */
      || *p == '/'
#endif
      ) {
        if (dir_part != NULL) {
          int len = p - path;
          if (p == path || (p == path + 2 && path[1] == ':'))
            /* root dir: include DIR_SEP_CHAR in dir_part */
            len++;
          memcpy(dir_part, path, len);
          dir_part[len] = '\0';
        }
        if (file_part != NULL)
          strcpy(file_part, p + 1);
        return;
    }
  }
  /* no DIR_SEP_CHAR: current dir */
  if (dir_part != NULL)
    dir_part[0] = '\0';
  if (file_part != NULL)
    strcpy(file_part, path);
}

void Util_getextension( char *filename, char *ext )
{
  char *ptr = strrchr( filename, '.' );
  int index = 0;
  if( ptr != NULL )
  {
    ++ptr;    
    while( *ptr != '\0' )
    {
      ext[index++] = tolower((unsigned char)*ptr);        
      ++ptr;
    }
  }
  ext[index] = '\0';
}

int Util_hextodec( const char* hex )
{
  if( !hex ) return 0;
    
  char *ptr = (char *)hex;   
  
  int val = 0;     
  while( *ptr != '\0' )
  {
    val <<= 4;        
    char c = *ptr;    
    if( c >= 'A' && c <= 'F' )
    {
      val |= ( 10 + ( c - 'A' ) );      
    }
    else if( c >= 'a' && c <= 'f' )
    {
      val |= ( 10 + ( c - 'a' ) );
    }   
    else if( c >= '0' && c <= '9' )
    {
      val |= ( c - '0' );
    }
                 
    ptr++;
  }    
  
  return val;
}

void Util_hextorgba( const char* hex, RGBA* rgba )
{
  memset( rgba, 0, sizeof( RGBA ) );
  if( !hex || strlen( hex ) != 8 ) return;    
  
  char tmp[3];
  tmp[2] = '\0';
    
  int index = 0;
  tmp[0] = hex[index++]; tmp[1] = hex[index++];  
  rgba->R = Util_hextodec( tmp );
  tmp[0] = hex[index++]; tmp[1] = hex[index++];  
  rgba->G = Util_hextodec( tmp );
  tmp[0] = hex[index++]; tmp[1] = hex[index++];  
  rgba->B = Util_hextodec( tmp );
  tmp[0] = hex[index++]; tmp[1] = hex[index];  
  rgba->A = Util_hextodec( tmp );
}

void Util_dectohex( int dec, char *hex, int fill )
{    
  if( fill > 0 )
  {
    memset( hex, '0', fill );
    hex[fill] = '\0';
  }
  
  int tdec = dec;
  int len = 0;  
  while( tdec > 0 )
  {
    tdec >>= 4;
    len++;
  }      
  
  if( fill < len )
  {  
    hex[len] = '\0';
  }
  
  int i;
  for( i = len - 1; i >= 0; i-- )
  {            
    int val = ( dec & 0xF );
    hex[ fill > 0 ? ( fill - len + i ) : i ] = 
      val >= 10 ? ( 'A' + ( val - 10 ) ) : ( '0' + val ); 
    dec >>= 4;
  }    
}

void Util_rgbatohex( RGBA* rgba, char *hex )
{
  hex[8] = '\0';
  
  char tmp[3];    
  tmp[2] = '\0';   
  
  int index = 0;  
  Util_dectohex( rgba->R, tmp, 2 );
  hex[index++] = tmp[0];
  hex[index++] = tmp[1];
  Util_dectohex( rgba->G, tmp, 2 );
  hex[index++] = tmp[0];
  hex[index++] = tmp[1];
  Util_dectohex( rgba->B, tmp, 2 );
  hex[index++] = tmp[0];
  hex[index++] = tmp[1];
  Util_dectohex( rgba->A, tmp, 2 );
  hex[index++] = tmp[0];
  hex[index] = tmp[1];  
}
