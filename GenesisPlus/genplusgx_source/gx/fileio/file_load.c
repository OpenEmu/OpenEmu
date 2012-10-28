/*
 * file_load.c
 * 
 *  ROM File loading support
 *
 *  Copyright Eke-Eke (2008-2012)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"
#include "file_load.h"
#include "gui.h"
#include "history.h"
#include "filesel.h"
#include "file_slot.h"

#include <iso9660.h>
#ifdef HW_RVL
#include <di/di.h>
#else
#include <ogc/dvd.h>
#endif

char rom_filename[256];

/* device root directories */
#ifdef HW_RVL
static const char rootdir[TYPE_RECENT][10] = {"sd:/","usb:/","dvd:/"};
#else
static const char rootdir[TYPE_RECENT][10] = {"/","dvd:/"};
#endif

/* DVD interface */
#ifdef HW_RVL
static const DISC_INTERFACE* dvd = &__io_wiidvd;
#else
static const DISC_INTERFACE* dvd = &__io_gcdvd;
#endif

/* current directory */
static char *fileDir;

/* current device */
static int deviceType = -1;

/* current file type */
static int fileType = -1;

/* DVD status flag */
static u8 dvd_mounted = 0;

/***************************************************************************
 * MountDVD
 *
 * return 0 on error, 1 on success
 ***************************************************************************/ 
static int MountDVD(void)
{
  GUI_MsgBoxOpen("Information", "Mounting DVD ...",1);

  /* check if DVD is already mounted */
  if (dvd_mounted)
  {
		/* unmount DVD */
    ISO9660_Unmount("dvd:");
    dvd_mounted = 0;
  }

  /* check if disc is found */
  if(!dvd->isInserted())
  {
    GUI_WaitPrompt("Error","No Disc inserted !");
    return 0;
  }
		
  /* mount DVD */
  if(!ISO9660_Mount("dvd",dvd))
  {
    GUI_WaitPrompt("Error","Disc can not be read !");
    return 0;
  }

  /* DVD is mounted */
  dvd_mounted = 1;

  GUI_MsgBoxClose();
  return 1;
}

/***************************************************************************
 * FileSortCallback (thanks to Marty Disibio)
 *
 * Quick sort callback to sort file entries with the following order:
 *   .
 *   ..
 *   <dirs>
 *   <files>
 ***************************************************************************/ 
static int FileSortCallback(const void *f1, const void *f2)
{
  /* Special case for implicit directories */
  if(((FILEENTRIES *)f1)->filename[0] == '.' || ((FILEENTRIES *)f2)->filename[0] == '.')
  {
    if(strcmp(((FILEENTRIES *)f1)->filename, ".") == 0) { return -1; }
    if(strcmp(((FILEENTRIES *)f2)->filename, ".") == 0) { return 1; }
    if(strcmp(((FILEENTRIES *)f1)->filename, "..") == 0) { return -1; }
    if(strcmp(((FILEENTRIES *)f2)->filename, "..") == 0) { return 1; }
  }
  
  /* If one is a file and one is a directory the directory is first. */
  if(((FILEENTRIES *)f1)->flags && !((FILEENTRIES *)f2)->flags) return -1;
  if(!((FILEENTRIES *)f1)->flags  && ((FILEENTRIES *)f2)->flags) return 1;
  
  return stricmp(((FILEENTRIES *)f1)->filename, ((FILEENTRIES *)f2)->filename);
}

/***************************************************************************
 * UpdateDirectory
 *
 * Update current browser directory
 * return zero if going up while in root
 * when going up, return previous dir name
 ***************************************************************************/ 
int UpdateDirectory(bool go_up, char *dirname)
{
  /* go up to parent directory */
  if (go_up)
  {
    /* special case */
    if (deviceType == TYPE_RECENT) return 0;

    /* check if we already are at root directory */
    if (!strcmp(rootdir[deviceType], (const char *)fileDir)) return 0;

    int size=0;
    char temp[MAXPATHLEN];

    /* determine last folder name length */
    strcpy(temp, fileDir);
    char *test= strtok(temp,"/");
    while (test != NULL)
    {
      size = strlen(test);
      strncpy(dirname,test,size);
      dirname[size] = 0;
      test = strtok(NULL,"/");
    }

    /* remove last folder from path */
    size = strlen(fileDir) - size;
    fileDir[size - 1] = 0;
  }
  else
  {
    /* by default, simply append folder name */
    sprintf(fileDir, "%s%s/",fileDir, dirname);
  }

  return 1;
}

/***************************************************************************
 * ParseDirectory
 *
 * List files into one directory
 ***************************************************************************/ 
int ParseDirectory(void)
{
  int nbfiles = 0;

  /* open directory */
  DIR *dir = opendir(fileDir);
  if (dir == NULL)
  {
    return -1;
  }

  struct dirent *entry = readdir(dir);

  /* list entries */
  while ((entry != NULL)&& (nbfiles < MAXFILES))
  {
    /* filter entries */
    if ((entry->d_name[0] != '.') 
       && strncasecmp(".wav", &entry->d_name[strlen(entry->d_name) - 4], 4) 
       && strncasecmp(".mp3", &entry->d_name[strlen(entry->d_name) - 4], 4))
    {
      memset(&filelist[nbfiles], 0, sizeof (FILEENTRIES));
      sprintf(filelist[nbfiles].filename,"%s",entry->d_name);
      if (entry->d_type == DT_DIR)
      {
        filelist[nbfiles].flags = 1;
      }
      nbfiles++;
    }

    /* next entry */
    entry = readdir(dir);
  }

  /* close directory */
  closedir(dir);

  /* Sort the file list */
  qsort(filelist, nbfiles, sizeof(FILEENTRIES), FileSortCallback);

  return nbfiles;
}

/****************************************************************************
 * LoadFile
 *
 * This function will load a game file into the ROM buffer.
 * This functions return the actual size of data copied into the buffer
 *
 ****************************************************************************/ 
int LoadFile(int selection) 
{
  int size, cd_mode1, filetype;
  char filename[MAXPATHLEN];

  /* file path */
  char *filepath = (deviceType == TYPE_RECENT) ? history.entries[selection].filepath : fileDir;

  /* full filename */
  sprintf(filename, "%s%s", filepath, filelist[selection].filename);

  /* DVD hot swap  */
  if (!strncmp(filepath, rootdir[TYPE_DVD], strlen(rootdir[TYPE_DVD])))
  {
    /* Check if file is still accessible */
    struct stat filestat;
    if(stat(filename, &filestat) != 0)
    {
      /* If not, try to mount DVD */
      if (!MountDVD()) return 0;
    }
  }

  /* open message box */
  GUI_MsgBoxOpen("Information", "Loading game...", 1);

  /* no cartridge or CD game loaded */
  size = cd_mode1 = 0;

  /* check if virtual CD tray was open */
  if ((system_hw == SYSTEM_MCD) && (cdd.status == CD_OPEN))
  {
    /* swap CD image file in (without changing region, system,...) */
    size = cdd_load(filename, (char *)(cdc.ram));

    /* check if a cartridge is currently loaded  */
    if (scd.cartridge.boot)
    {
      /* CD Mode 1 */
      cd_mode1 = size;
    }
    else
    {
      /* update game informations from CD image file header */
      getrominfo((char *)(cdc.ram));
    }
  }

  /* no CD image file loaded */
  if (!size)
  {
    /* close CD tray to force system reset */
    cdd.status = NO_DISC;

    /* load game file */
    size = load_rom(filename);
  }

  if (size > 0)
  {
    /* do not update game basename if a CD was loaded with a cartridge (Mode 1) */
    if (cd_mode1)
    {
      /* add CD image file to history list */
      filetype = 1;
    }
    else
    {
      /* auto-save previous game state */
      slot_autosave(config.s_default,config.s_device);

      /* update game basename (for screenshot, save & cheat files) */
      if (romtype & SYSTEM_SMS)
      {
        /* Master System ROM file */
        filetype = 2;
        sprintf(rom_filename,"ms/%s",filelist[selection].filename);
      }
      else if (romtype & SYSTEM_GG)
      {
        /* Game Gear ROM file */
        filetype = 3;
        sprintf(rom_filename,"gg/%s",filelist[selection].filename);
      }
      else if (romtype == SYSTEM_SG)
      {
        /* SG-1000 ROM file */
        filetype = 4;
        sprintf(rom_filename,"sg/%s",filelist[selection].filename);
      }
      else if (romtype == SYSTEM_MCD)
      {
        /* CD image file */
        filetype = 1;
        sprintf(rom_filename,"cd/%s",filelist[selection].filename);
      }
      else
      {
        /* by default, Genesis ROM file */
        filetype = 0;
        sprintf(rom_filename,"md/%s",filelist[selection].filename);
      }

      /* remove file extension */
      int i = strlen(rom_filename) - 1;
      while ((i > 0) && (rom_filename[i] != '.')) i--;
      if (i > 0) rom_filename[i] = 0;
    }

    /* add/move the file to the top of the history. */
    history_add_file(filepath, filelist[selection].filename, filetype);

    /* recent file list may have changed */
    if (deviceType == TYPE_RECENT) deviceType = -1;

    /* close message box */
    GUI_MsgBoxClose();

    /* valid image has been loaded */
    return 1;
  }

  GUI_WaitPrompt("Error", "Unable to load game");
  return 0;
}

/****************************************************************************
 * OpenDir
 *
 * Function to open a directory and load ROM file list.
 ****************************************************************************/ 
int OpenDirectory(int device, int type)
{
  int max = 0;

  if (device == TYPE_RECENT)
  {
    /* fetch history list */
    int i;
    for(i=0; i < NUM_HISTORY_ENTRIES; i++)
    {
      if(history.entries[i].filepath[0] > 0)
      {
        filelist[i].flags = 0;
        strncpy(filelist[i].filename,history.entries[i].filename, MAXJOLIET-1);
        filelist[i].filename[MAXJOLIET-1] = '\0';
        max++;
      }
      else
      {
        /* Found the end of the list. */
        break;
      }
    }
  }
  else
  {
    /* only DVD hot swap is supported */
    if (device == TYPE_DVD)
    {
      /* try to access root directory */
      DIR *dir = opendir(rootdir[TYPE_DVD]);
      if (dir == NULL)
      {
        /* mount DVD */
        if (!MountDVD()) return 0;
        deviceType = -1;
      }
      else
      {
        closedir(dir);
      }
    }

    /* parse last directory */
    fileDir = config.lastdir[type][device];
    max = ParseDirectory();
    if (max <= 0)
    {
      /* parse root directory */
      strcpy(fileDir, rootdir[device]);
      max = ParseDirectory();
      if (max < 0)
      {
        GUI_WaitPrompt("Error","Unable to open directory !");
        return 0;
      }
      deviceType = -1;
    }
  }

  if (max == 0)
  {
    GUI_WaitPrompt("Error","No files found !");
    return 0;
  }

  /* check if device or file type has changed */
  if ((device != deviceType) || (type != fileType))
  {
    /* reset current types */
    deviceType = device;
    fileType = type;

    /* reset File selector */
    ClearSelector(max);
  }

  return 1;
}
