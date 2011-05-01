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

/****************************************************************/
/*			Mednafen				*/
/*								*/
/*	This file contains code for parsing command-line    	*/
/*	options.						*/
/*								*/
/****************************************************************/

#include "../types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "args.h"

static int ParseEA(int x, int argc, char *argv[], ARGPSTRUCT *argsps)
{
  int y=0;
  int ret = 0;

  do
  {
   if(!argsps[y].name)
   {
    int tmpie = ParseEA(x,argc,argv,(ARGPSTRUCT*)argsps[y].var);
    if(-1 == tmpie) return(-1);
    ret += tmpie;
    y++;
    if(ret)
     return(ret);
    continue;
   } 
   if((argv[x][0] == '-' && argv[x][1] == '-' && !strcmp(argv[x]+2,argsps[y].name)) ||
	(argv[x][0] == '-' && !strcmp(argv[x]+1, argsps[y].name)) )	// A match.
   {
    ret++;

    if(argsps[y].subs)
    {
     if((x+1)>=argc) 
	// || argv[x+1][0] == '-') // Breaks negative numbers!
     {
      printf("Argument %s requires a parameter!\n", argsps[y].name);
      return(-1);
     }

     if(argsps[y].substype&0x2000) // Function
     {
      int funcretval = ((int (*)(const char *, const char *))argsps[y].subs)(argsps[y].name, argv[x+1]);
      if(!funcretval)
      {
       //printf("Invalid parameter \"%s\" to argument \"%s\"!\n", argv[x+1], argsps[y].name);
       return(-1);
      }
     }

     ret++;

     if(argsps[y].substype&0x8000)	// Bool
     {
      *(int *)argsps[y].subs&=~(argsps[y].substype&(~0x8000));
      *(int *)argsps[y].subs|=atoi(argv[x+1])?(argsps[y].substype&(~0x8000)):0;
     }
     else	// Value
      switch(argsps[y].substype&(~0x4000))
      {
       case SUBSTYPE_INTEGER:
	      {
	       char *endptr = NULL;
 	       *(int *)argsps[y].subs = strtol(argv[x+1], &endptr, 10);
	       if(endptr == argv[x+1] || endptr[0] != 0)
	       {
		printf("Argument %s requires a valid integer, not: %s\n", argv[x], argv[x+1]);
		return(-1);
	       }
	      }
	      break;

       case SUBSTYPE_DOUBLE:
	      {
		char *endptr = NULL;
	       *(double *)argsps[y].subs = strtod(argv[x+1], &endptr);
	       if(endptr == argv[x+1] || endptr[0] != 0)
	       {
		printf("Argument %s requires a valid decimal number, not: %s\n", argv[x], argv[x+1]);
		return(-1);
	       }
	      }
	      break;

       case SUBSTYPE_STRING:		// String
	      if(argsps[y].substype&0x4000)
	      {
               if(*(char **)argsps[y].subs)
		free(*(char **)argsps[y].subs);
	       if(!( *(char **)argsps[y].subs=(char*)malloc(strlen(argv[x+1])+1) ))
		break;
	      }	
	      strcpy(*(char **)argsps[y].subs,argv[x+1]);
	      break;
      }
    }
    if(argsps[y].var)
     *argsps[y].var=1;
    return(ret);
   }
   y++;
  } while(argsps[y].var || argsps[y].subs);

 return(0);
}

int ParseArguments(int argc, char *argv[], ARGPSTRUCT *argsps, char **first_not)
{
 int x = 0;

 while(x<argc)
 {
  if(argv[x][0] != '-')
  {
   *first_not = argv[x];
   return(1);
  }
  int retval = ParseEA(x,argc,argv,argsps);

  if(retval == -1)
   return(0);
  else if(!retval)
  {
   printf("Unrecognized argument: %s\n", argv[x]);
   return(0);
  }
  else
   x += retval;
 }

 return(1);
}

void ShowArgumentsHelpSub(ARGPSTRUCT *argsps, bool show_linked)
{
 while(argsps->var || argsps->subs)
 {
  if(!argsps->name)
  {
   if(show_linked)
    ShowArgumentsHelpSub((ARGPSTRUCT *)argsps->var, show_linked);
  }
  else
  {
   if(argsps->subs)
    printf(" -%s <parameter>\n\t%s\n", argsps->name, argsps->description);
   else
    printf(" -%s\n\t%s\n", argsps->name, argsps->description);
  }
  argsps++;
 }

}

int ShowArgumentsHelp(ARGPSTRUCT *argsps, bool show_linked)
{
 printf("\nOptions:\n\n");

 ShowArgumentsHelpSub(argsps, show_linked);

 return(1);
}
