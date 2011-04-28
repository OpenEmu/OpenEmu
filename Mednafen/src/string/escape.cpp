/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "../mednafen.h"
#include "escape.h"

static unsigned int hex_nibble_to_val(char nibble)
{
 unsigned int ret = 0;
 nibble = tolower(nibble);

 if(nibble >= '0' && nibble <= '9')
  ret = nibble - '0';
 else
  ret = nibble - 'a';

 return(ret);
}

void unescape_string(char *string)
{
 char *src = string;
 bool inescape = 0;
 uint8 hoval = 0;
 int inhex = 0;
 int inoctal = 0;

 while(*src)
 {
  if(*src == '\\')
  {
   inescape = TRUE;
   inhex = 0;
   inoctal = 0;
  }
  else if(inhex)
  {
   if(inhex == 1) 
   {
    hoval = hex_nibble_to_val(*src) << 4;
    inhex++;
   }
   else if(inhex == 2)
   {
    hoval |= hex_nibble_to_val(*src);
    *string = hoval;
    string++;
    hoval = 0;
    inhex = 0;
   }
  }
  else if(inoctal)
  {
   if(inoctal == 1)
   {
    hoval = (*src - '0') * 8 * 8;
   }
   else if(inoctal == 2)
   {
    hoval += (*src - '0') * 8;
   }
   else
   {
    hoval += *src - '0';
    *string = hoval;
    string++;
    hoval = 0;
    inoctal = 0;
   }
  }
  else if(inescape)
  {
   switch(*src)
   {
    case 'a': *string = 7; string++; break;
    case 'b': *string = 8; string++; break;
    case 'f': *string = 12; string++; break;
    case 'n': *string = 10; string++; break;
    case 'r': *string = 13; string++; break;
    case 't': *string = 9; string++; break;
    case 'v': *string = 11; string++; break;

    case '\\': *string = '\\'; string++; break;
    case '?': *string = '?'; string++; break;
    case '\'': *string = '\''; string++; break;
    case '"': *string = '"'; string++; break;

    case 'o': inoctal = 1; break;
    case 'x': inhex = 1; break;
    

    default: *string = *src; string++; break;
   }
   inescape = 0;
  }
  else
  {
   *string = *src;
   string++;
  }
  src++;
 }
 *string = 0;
}

char *escape_string(const char *text)
{
 uint32 slen = strlen(text);
 char *ret = (char*)malloc(slen * 4 + 1); // \xFF
 char *outoo = ret;

 for(uint32 x = 0; x < slen; x++)
 {
  int c = (uint8)text[x];

  if(c < 0x20 || c == 0x7F || c == '\\' || c == '\'' || c == '"')
  {
   *outoo++ = '\\';

   switch(c)
   {
    case '\\': *outoo++ = '\\'; break;
    case '\'': *outoo++ = '\''; break;
    case '"': *outoo++ = '"'; break;
    case 7: *outoo++ = 'a'; break;
    case 8: *outoo++ = 'b'; break;
    case 12: *outoo++ = 'f'; break;
    case 10: *outoo++ = 'n'; break;
    case 13: *outoo++ = 'r'; break;
    case 9: *outoo++ = 't'; break;
    case 11: *outoo++ = 'v'; break;

    default: outoo += sprintf(outoo, "x%02x", c); break;
   }
  }
  else
   *outoo++ = c;
 }

 *outoo = 0;

 return(ret);
}
