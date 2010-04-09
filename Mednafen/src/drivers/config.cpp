/* Mednafen - Multi-system Emulator
 *
 * Copyright notice for this file:
 *  Copyright (C) 2002 Xodnizel
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

/****************************************************************/
/*			Mednafen				*/
/*								*/
/*	This file contains routines for reading/writing the     */
/*	configuration file.					*/
/*								*/
/****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../types.h"
#include "config.h"

static int FReadString(FILE *fp, char *str, int n)
{
 int x=0,z;
 for(;;)
 {
  z=fgetc(fp);
  str[x]=z;
  x++;  
  if(z<=0) break;
  if(x>=n) return 0;
 }
 if(z<0) return 0;
 return 1;
}

static void GetValueR(FILE *fp, const char *str, void *v, int c)
{
 char buf[256];
 int s;

 while(FReadString(fp,buf,256))
 {
  fread(&s,1,4,fp);
  if(!strcmp(str, buf))
  {
   if(!c)	// String, allocate some memory.
   {
    if(!(*(char **)v=(char*)malloc(s)))
     goto gogl;
    fread(*(char **)v,1,s,fp);
    continue;
   }
   else if(s>c || s<c)
   {
     gogl:
     fseek(fp,s,SEEK_CUR);
     continue;
   }
   fread((uint8*)v,1,c,fp);
  }
  else
   fseek(fp,s,SEEK_CUR);
 }
 fseek(fp,4,SEEK_SET);
}

static void SetValueR(FILE *fp, const char *str, void *v, int c)
{
 fwrite(str,1,strlen(str)+1,fp);
 fwrite((uint8*)&c,1,4,fp);
 fwrite((uint8*)v,1,c,fp);
}

static void SaveParse(CFGSTRUCT *cfgst, FILE *fp)
{
        int x=0;

	while(cfgst[x].ptr)
        {
         if(!cfgst[x].name)     // Link to new config structure
	 {
	  SaveParse((CFGSTRUCT*)cfgst[x].ptr,fp);	// Recursion is sexy.  I could
					// save a little stack space if I made
					// the file pointer a non-local
					// variable...
	  x++;
	  continue;
	 }

         if(cfgst[x].len)               // Plain data
          SetValueR(fp,cfgst[x].name,cfgst[x].ptr,cfgst[x].len);
         else                           // String
          if(*(char **)cfgst[x].ptr)    // Only save it if there IS a string.
           SetValueR(fp,cfgst[x].name,*(char **)cfgst[x].ptr,
                        strlen(*(char **)cfgst[x].ptr)+1);
         x++;
        }
}

void SaveMDFNConfig(const char *filename, CFGSTRUCT *cfgst)
{
	FILE *fp;

        fp=fopen(filename,"wb");
        if(fp==NULL) return;

	SaveParse(cfgst,fp);

	fclose(fp);
}

static void LoadParse(CFGSTRUCT *cfgst, FILE *fp)
{
	int x=0;

	while(cfgst[x].ptr)
        {
         if(!cfgst[x].name)     // Link to new config structure
         {
	  LoadParse((CFGSTRUCT*)cfgst[x].ptr,fp);
	  x++;
	  continue;
         }
         GetValueR(fp,cfgst[x].name,cfgst[x].ptr,cfgst[x].len);
         x++;
        } 
}

int LoadMDFNConfig(const char *filename, CFGSTRUCT *cfgst)
{
        FILE *fp;

        fp=fopen(filename,"rb");
        if(fp==NULL) return(0);

	LoadParse(cfgst,fp);
        fclose(fp);
	return(1);
}
