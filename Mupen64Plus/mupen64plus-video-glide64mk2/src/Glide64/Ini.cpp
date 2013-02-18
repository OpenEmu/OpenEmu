/*
*   Glide64 - Glide video plugin for Nintendo 64 emulators.
*   Copyright (c) 2002  Dave2001
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public
*   Licence along with this program; if not, write to the Free
*   Software Foundation, Inc., 51 Franklin Street, Fifth Floor, 
*   Boston, MA  02110-1301, USA
*/

//****************************************************************
//
// Glide64 - Glide Plugin for Nintendo 64 emulators (tested mostly with Project64)
// Project started on December 29th, 2001
//
// To modify Glide64:
// * Write your name and (optional)email, commented by your work, so I know who did it, and so that you can find which parts you modified when it comes time to send it to me.
// * Do NOT send me the whole project or file that you modified.  Take out your modified code sections, and tell me where to put them.  If people sent the whole thing, I would have many different versions, but no idea how to combine them all.
//
// Official Glide64 development channel: #Glide64 on EFnet
//
// Original author: Dave2001 (Dave2999@hotmail.com)
// Other authors: Gonetz, Gugaman
//
//****************************************************************

// INI code v1.1

#include "m64p.h"

#include "Ini.h"
#include "Gfx_1.3.h"
#include <limits.h>
#ifndef _WIN32
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <stdlib.h>
#else
#include <io.h>
#endif // _WIN32

#include <errno.h>
#ifndef _WIN32
#include <sys/resource.h>
#endif

#ifdef _WIN32
#define PATH_MAX _MAX_PATH
#endif

FILE *ini;
int sectionstart;
int last_line;      // last good line
int last_line_ret;  // last line ended in return?
wxUint16 cr = 0x0A0D;
static char configdir[PATH_MAX] = {0};


//TODO: move INI_xxxx() function code into class and clean up
Ini *Ini::singleton = 0;

Ini::Ini()
{
	if (!INI_Open())
	{
		printf("Could not find INI file!");
		exit(1);
	}
}

Ini *Ini::OpenIni()
{
	if (!singleton)
		singleton = new Ini();
	return singleton;
}

void Ini::SetPath(const char *path)
{
	if (!INI_FindSection(path, false))
	{
		printf("Could not find [%s] section in INI file!", path);
	}
}


bool Ini::Read(const char *key, int *l)
{
	int undef = 0xDEADBEEF;
	int tmpVal = INI_ReadInt(key, undef, false);
	if (tmpVal == undef)
	{
		return false;
	}
	else
	{
		*l = tmpVal;
		return true;
	}
}

bool Ini::Read(const char *key, int *l, int defaultVal)
{
	*l = INI_ReadInt(key, defaultVal, false);
	return true;
}

int Ini::Read(const char *key, int defaultVal)
{
	return INI_ReadInt(key, defaultVal, false);
}


BOOL INI_Open ()
{
	//TODO: use ConfigGetSharedDataFilepath
	
    // Get the path of the dll, ex: C:\Games\Project64\Plugin\Glide64.dll
    char path[PATH_MAX];
    if(strlen(configdir) > 0)
    {
        strncpy(path, configdir, PATH_MAX);
        // make sure there's a trailing '/'
        //if(path[strlen(path)-1] != '/')
        //    strncat(path, "/", PATH_MAX - strlen(path));
    }
    else
    {
#ifdef _WIN32
    GetModuleFileName (NULL, path, PATH_MAX);
#else // _WIN32
# ifdef __FreeBSD__
   int n = readlink("/proc/curproc/files", path, PATH_MAX);
#else
   int n = readlink("/proc/self/exe", path, PATH_MAX);
#endif
   if (n == -1) strcpy(path, "./");
   else
     {
    char path2[PATH_MAX];
    int i;
    
    path[n] = '\0';
    strcpy(path2, path);
    for (i=strlen(path2)-1; i>0; i--)
      {
         if(path2[i] == '/') break;
      }
    if(i == 0) strcpy(path, "./");
    else
      {
         DIR *dir;
         struct dirent *entry;
         int gooddir = 0;
         
         path2[i+1] = '\0';
         dir = opendir(path2);
         while((entry = readdir(dir)) != NULL)
           {
          if(!strcmp(entry->d_name, "plugins"))
            gooddir = 1;
           }
         closedir(dir);
         if(!gooddir) strcpy(path, "./");
      }
     }

#endif // _WIN32

    // Find the previous backslash
    int i;
    for (i=strlen(path)-1; i>0; i--)
    {
#ifdef _WIN32
        if (path[i] == '\\')
#else // _WIN32
            if (path[i] == '/')
#endif // _WIN32
            break;
    }
    if (path == 0) return FALSE;
    path[i+1] = 0;

#ifndef _WIN32
   strcat(path, "plugins/");
#endif // _WIN32
    }
   
    //strncat (path, "Glide64mk2.ini", PATH_MAX - strlen(path));
    LOG("opening %s\n", path);
    // Open the file
    ini = fopen (path, "rb");
    if (ini == NULL)
    {
        ERRLOG("Could not find Glide64mk2.ini!");
        return FALSE;
        /*
        ini = fopen (path, "w+b");
        if (ini == NULL)
        {
            return FALSE;
        }
        */
    }

    sectionstart = 0;
    last_line = 0;
    last_line_ret = 1;

    return TRUE;
}

void INI_Close ()
{
    //if (ini)
      //  fclose(ini);
}

void INI_InsertSpace(int space)
{
  printf("Inserting space, space to insert is %d\n", space);
    // Since there is no good way to normally insert to or delete from a certain location in
    //  a file, this function was added.  It will insert (or delete) space bytes at the
    //  current location.

    // note: negative count means delete
    char chunk[2048];
    int len, file, start_pos, cur_pos;

#ifdef _WIN32
    file = _fileno(ini);
#else // _WIN32
   file = fileno(ini);
#endif // _WIN32

    start_pos = ftell(ini);
    fseek(ini,0,SEEK_END);

    // if adding, extend the file
    if (space > 0)
#ifdef _WIN32
        _chsize (file, _filelength(file)+space);
#else // _WIN32
     {
    int t1 = ftell(ini);
    fseek(ini, 0L, SEEK_END);
    int t2 = ftell(ini);
    fseek(ini, t1, SEEK_SET);
    ftruncate(file, t2+space);
     }
#endif // _WIN32

    while (1) {
        cur_pos = ftell(ini);
        len = cur_pos - start_pos;
        if (len == 0) break;
        if (len > 2048) len = 2048;

        fseek (ini,-len,SEEK_CUR);
        fread (chunk,1,len,ini);
        fseek (ini,-len+space,SEEK_CUR);
        fwrite (chunk,1,len,ini);
        fseek (ini,-len-space,SEEK_CUR);
    }

    // if deleted, make the file shorter
    if (space < 0)
#ifdef _WIN32
        _chsize (file, _filelength(file)+space);
#else // _WIN32
     {
    int t1 = ftell(ini);
    fseek(ini, 0L, SEEK_END);
    int t2 = ftell(ini);
    fseek(ini, t1, SEEK_SET);
    ftruncate(file, t2+space);
     }
#endif // _WIN32
}

BOOL INI_FindSection (const char *sectionname, BOOL create)
{
    if (ini == NULL)
        return FALSE;
    printf("INI_FindSection trying to find name for %s\n", sectionname);

    char line[256], section[64];
    char *p;
    int  i, sectionfound, ret;

    rewind (ini);

    last_line = 0;
    sectionfound = 0;

    while(!feof(ini)) {
        ret = 0;
        *line=0;
        fgets(line,255,ini);

        // remove enter
        i=strlen(line);
    // ZIGGY there was a bug here if EOL was unix like on a short line (i.e. a line
    // with just EOL), it would write into line[-1]
        if(i>=1 && line[i-1]==0xa) {
      ret=1;
      line[i-1]=0;
      if (i>=2 && line[i-2]==0xd) line[i-2]=0;
    }

        // remove comments
        p=line;
        while(*p)
        {
            if (p[0]=='/' && p[1]=='/')
            {
                p[0]=0;
                break;
            }
            p++;
        }

        // skip starting space
        p=line;
        while(*p<=' ' && *p) p++;

        // empty line
        if(!*p) continue;

        last_line=ftell(ini);   // where to add if not found
        last_line_ret = ret;

        if(*p!='[') continue;

        p++;
        for (i=0;i<63;i++)
        {
            if(*p==']' || !*p) break;
            section[i]=*p++;
        }
        section[i]=0;

#ifdef _WIN32
        if(!stricmp(section,sectionname))
#else // _WIN32
         if (!strcasecmp(section,sectionname))
#endif // _WIN32
        {
            sectionstart=ftell(ini);
            sectionfound=1;
            return TRUE;
        }
    }

    if (!sectionfound && create)
    {
        // create the section
        fseek(ini,last_line,SEEK_SET);
        INI_InsertSpace ((!last_line_ret) * 2 + 6 + strlen(sectionname));
        if (!last_line_ret) fwrite (&cr, 1, 2, ini);
        fwrite (&cr, 1, 2, ini);
        sprintf (section, "[%s]", sectionname);
        fwrite (section, 1, strlen(section), ini);
        fwrite (&cr, 1, 2, ini);
        sectionstart = ftell(ini);
        last_line = sectionstart;
        last_line_ret = 1;
        return TRUE;
    }

    return FALSE;
}

// Reads the value of item 'itemname' as a string.
const char *INI_ReadString (const char *itemname, char *value, const char *def_value, BOOL create)
{
    char line[256], name[64];
    char *p, *n;
    int ret, i;
    *value = 0;

    fseek(ini,sectionstart,SEEK_SET);

    while(!feof(ini)) {
        ret = 0;
        *line=0;
        fgets(line,255,ini);

        // remove enter
        i=strlen(line);
    // ZIGGY there was a bug here if EOL was unix like on a short line (i.e. a line
    // with just EOL), it would write into line[-1]
        // OLD CODE : if(line[i-1]=='\n') ret=1, line[i-2]=0;
        if(i>=1 && line[i-1]==0xa) {
      ret=1;
      line[i-1]=0;
      if (i>=2 && line[i-2]==0xd) line[i-2]=0;
    }

        // remove comments
        p=line;
        while(*p)
        {
            if (p[0]==';')
            {
                p[0]=0;
                break;
            }
            p++;
        }

        // skip starting space
        p=line;
        while(*p<=' ' && *p) p++;

        // empty line
        if(!*p) continue;

        // new section
        if(*p=='[') break;

        last_line=ftell(ini);   // where to add if not found
        last_line_ret = ret;

        // read name
        n = name;
        while(*p && *p!='=' && *p>' ') *n++ = *p++;
        *n = 0;

#ifdef _WIN32
        if(!stricmp(name,itemname))
#else // _WIN32
         if(!strcasecmp(name,itemname))
#endif // _WIN32
        {
            // skip spaces/equal sign
            while(*p<=' ' || *p=='=') p++;

            // read value
            n = value;
            while(*p) *n++ = *p++;

            // remove trailing spaces
            while (*(n-1) == ' ') n--;

            *n=0;

            return value;
        }
    }

    // uh-oh, not found.  we need to create
    if (create)
    {
        fseek(ini,last_line,SEEK_SET);
        INI_InsertSpace ((!last_line_ret) * 2 + strlen(itemname) + strlen(def_value) + 5);
        if (!last_line_ret) fwrite (&cr, 1, 2, ini);
        sprintf (line, "%s = %s", itemname, def_value);
        fwrite (line, 1, strlen(line), ini);
        fwrite (&cr, 1, 2, ini);
        last_line = ftell(ini);
        last_line_ret = 1;
    }

    strcpy (value, def_value);
    return value;
}

// Reads the value of item 'itemname' as a string.
void INI_WriteString (const char *itemname, const char *value)
{
    char line[256], name[64];
    char *p, *n;
    int ret, i;

    fseek(ini,sectionstart,SEEK_SET);

    while(!feof(ini)) {
        ret = 0;
        *line=0;
        fgets(line,255,ini);

        // remove enter
        i=strlen(line);
    // ZIGGY there was a bug here if EOL was unix like on a short line (i.e. a line
    // with just EOL), it would write into line[-1]
        // OLD CODE : if(line[i-1]=='\n') ret=1, line[i-2]=0;
        if(i>=1 && line[i-1]==0xa) {
      ret=1;
      line[i-1]=0;
      if (i>=2 && line[i-2]==0xd) line[i-2]=0;
    }

        // remove comments
        p=line;
        while(*p)
        {
            if (p[0]=='/' && p[1]=='/')
            {
                p[0]=0;
                break;
            }
            p++;
        }

        // skip starting space
        p=line;
        while(*p<=' ' && *p) p++;

        // empty line
        if(!*p) continue;

        // new section
        if(*p=='[') break;

        last_line=ftell(ini);   // where to add if not found
        last_line_ret = ret;

        // read name
        n = name;
        while(*p && *p!='=' && *p>' ') *n++ = *p++;
        *n = 0;

#ifdef _WIN32
        if(!stricmp(name,itemname))
#else // _WIN32
         if(!strcasecmp(name,itemname))
#endif // _WIN32
        {
            INI_InsertSpace (-i + (strlen(itemname) + strlen(value) + 5));
            sprintf (line, "%s = %s", itemname, value);
            fseek (ini, -i, SEEK_CUR);
            fwrite (line, 1, strlen(line), ini);
            fwrite (&cr, 1, 2, ini);
            last_line = ftell(ini);
            last_line_ret = 1;
            return;
        }
    }

    // uh-oh, not found.  we need to create
    fseek(ini,last_line,SEEK_SET);
    INI_InsertSpace ((!last_line_ret) * 2 + strlen(itemname) + strlen(value) + 5);
    if (!last_line_ret) fwrite (&cr, 1, 2, ini);
    sprintf (line, "%s = %s", itemname, value);
    fwrite (line, 1, strlen(line), ini);
    fwrite (&cr, 1, 2, ini);
    last_line = ftell(ini);
    last_line_ret = 1;
    return;
}

int INI_ReadInt (const char *itemname, int def_value, BOOL create)
{
    if (ini == NULL)
        return def_value;

    char value[64], def[64];
#ifdef _WIN32
    _itoa (def_value, def, 10);
#else // _WIN32
   sprintf(def, "%d", def_value);
#endif // _WIN32
    INI_ReadString (itemname, value, def, create);
    return atoi (value);
}

void INI_WriteInt (const char *itemname, int value)
{
    char valstr[64];
#ifdef _WIN32
    _itoa (value, valstr, 10);
#else // _WIN32
   sprintf(valstr, "%d", value);
#endif // _WIN32
    INI_WriteString (itemname, valstr);
}

void SetConfigDir( const char *configDir )
{
    strncpy(configdir, configDir, PATH_MAX);
}

