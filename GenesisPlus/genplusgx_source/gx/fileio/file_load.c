/*
 * file_load.c
 * 
 *  ROM File loading support
 *
 *  Eke-Eke (2010)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************************/

#include "shared.h"
#include "file_load.h"
#include "gui.h"
#include "history.h"
#include "unzip.h"
#include "filesel.h"

#include <iso9660.h>
#ifdef HW_RVL
#include <di/di.h>
#else
#include <ogc/dvd.h>
#endif

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
 * FileSortCallback (code by Marty Disibio)
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
  if(((FILEENTRIES *)f1)->flags == 1 && ((FILEENTRIES *)f2)->flags == 0) return -1;
  if(((FILEENTRIES *)f1)->flags == 0 && ((FILEENTRIES *)f2)->flags == 1) return 1;
  
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
  char filename[MAXPATHLEN];
  struct stat filestat;

  /* open directory */
  DIR_ITER *dir = diropen(fileDir);
  if (dir == NULL)
  {
    return -1;
  }

  /* list files */
  while ((dirnext(dir, filename, &filestat) == 0) && (nbfiles < MAXFILES))
  {
    if (filename[0] != '.')
    {
      memset(&filelist[nbfiles], 0, sizeof (FILEENTRIES));
      sprintf(filelist[nbfiles].filename,"%s",filename);
      filelist[nbfiles].flags = (filestat.st_mode & S_IFDIR) ? 1 : 0;
      nbfiles++;
    }
  }

  /* close directory */
  dirclose(dir);

  /* Sort the file list */
  qsort(filelist, nbfiles, sizeof(FILEENTRIES), FileSortCallback);

  return nbfiles;
}

/****************************************************************************
 * LoadFile
 *
 * This function will load a BIN, SMD or ZIP file into the ROM buffer.
 * This functions return the actual size of data copied into the buffer
 *
 ****************************************************************************/ 
int LoadFile(u8 *buffer, u32 selection, char *filename) 
{
  char fname[MAXPATHLEN];
  char *filepath;
  int done = 0;
  struct stat filestat;

  /* file path */
  filepath = (deviceType == TYPE_RECENT) ? history.entries[selection].filepath : fileDir;

  /* full filename */
  sprintf(fname, "%s%s", filepath, filelist[selection].filename);

  /* retrieve file status */
  if(stat(fname, &filestat) != 0)
  {
    /* only DVD hot swap is supported */
    if (!strncmp(filepath, rootdir[TYPE_DVD], strlen(rootdir[TYPE_DVD])))
    {
      /* mount DVD */
      if (!MountDVD()) return 0;
    
      /* retrieve file status */
      stat(fname, &filestat);
    }
  }

  /* get file length */
  int length = filestat.st_size;

  if (length > 0)
  {
    /* open file */
    FILE *fd = fopen(fname, "rb");
    if (!fd)
    {
        GUI_WaitPrompt("Error","Unable to open file !");
        return 0;
    }

    /* Read first data chunk */
    unsigned char temp[FILECHUNK];
    fread(temp, FILECHUNK, 1, fd);
    fseek(fd, 0, SEEK_SET);

    /* Determine file type */
    if (!IsZipFile ((char *) temp))
    {
      if (length > MAXROMSIZE)
      {
        GUI_WaitPrompt("Error","File is too large !");
        return 0;
      }

      /* Read file */
      sprintf((char *)temp,"Loading %d bytes ...", length);
      GUI_MsgBoxOpen("Information", (char *)temp, 1);
      while (length > FILECHUNK)
      {
        fread(buffer + done, FILECHUNK, 1, fd);
        length -= FILECHUNK;
        done += FILECHUNK;
      }
      fread(buffer + done, length, 1, fd);
      done += length;
      GUI_MsgBoxClose();

      /* update ROM filename (with extension) */
      sprintf(filename, "%s", filelist[selection].filename);
    }
    else
    {
      /* unzip file */
      done = UnZipBuffer(buffer, fd, filename);
    }

    /* close file */
    fclose(fd);

    if (done)
    {
      /* add/move the file to the top of the history. */
      history_add_file(filepath, filelist[selection].filename);

      /* recent file list has changed */
      if (deviceType == TYPE_RECENT) deviceType = -1;

      /* return loaded size */
      return done;
    }
  }

  return 0;
}

/****************************************************************************
 * OpenDir
 *
 * Function to open a directory and load ROM file list.
 ****************************************************************************/ 
int OpenDirectory(int device)
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
      DIR_ITER *dir = diropen(rootdir[TYPE_DVD]);
      if (dir == NULL)
      {
        /* mount DVD */
        if (!MountDVD()) return 0;
        deviceType = -1;
      }
      else
      {
        dirclose(dir);
      }
    }

    /* parse last directory */
    fileDir = config.lastdir[device];
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

  /* check if device type has changed */
  if (device != deviceType)
  {
    /* reset current device type */
    deviceType = device;

    /* reset File selector */
    ClearSelector(max);
  }

  return 1;
}
