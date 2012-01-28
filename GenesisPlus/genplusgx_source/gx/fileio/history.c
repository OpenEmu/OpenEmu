/*
 *  history.c
 *
 *   Generic ROM history list managment
 *
 *   Martin Disibio (6/17/08)
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 ********************************************************************************/

#include "shared.h"
#include "history.h"

t_history history;


/****************************************************************************
 * history_add_file
 *
 * Adds the given file path to the top of the history list, shifting each
 * existing entry in the history down one place. If given file path is 
 * already in the list then the existing entry is (in effect) moved to the
 * top instead.
  ****************************************************************************/ 
void history_add_file(char *filepath, char *filename)
{
  /* Create the new entry for this path. */
  t_history_entry newentry;
  strncpy(newentry.filepath, filepath, MAXJOLIET - 1);
  strncpy(newentry.filename, filename, MAXJOLIET - 1);
  newentry.filepath[MAXJOLIET - 1] = '\0';
  newentry.filename[MAXJOLIET - 1] = '\0';
  
  t_history_entry oldentry;  /* Old entry is the one being shuffled down a spot. */  
  t_history_entry currentry;  /* Curr entry is the one that just replaced old path. */
  
  /* Initially set curr entry to the new value. */
  memcpy(&currentry, &newentry, sizeof(t_history_entry));

  int i;
  for(i=0; i < NUM_HISTORY_ENTRIES; i++)
  {
    /* Save off the next entry. */
    memcpy(&oldentry, &history.entries[i], sizeof(t_history_entry));

    /* Overwrite with the previous entry. */
    memcpy(&history.entries[i], &currentry, sizeof(t_history_entry));

    /* Switch the old entry to the curr entry now. */
    memcpy(&currentry, &oldentry, sizeof(t_history_entry));

    /* If the entry in the list at this spot matches
       the new entry then do nothing and let this
       entry get deleted. */
    if(strcmp(newentry.filepath, currentry.filepath) == 0 && strcmp(newentry.filename, currentry.filename) == 0)
      break;
  }

  /* now save to disk */
  history_save();
}

void history_save()
{
  /* open file */
  char fname[MAXPATHLEN];
  sprintf (fname, "%s/history.ini", DEFAULT_PATH);
  FILE *fp = fopen(fname, "wb");
  if (fp)
  {
    /* write file */
    fwrite(&history, sizeof(history), 1, fp);
    fclose(fp);
  }
}

void history_load(void)
{
  /* open file */
  char fname[MAXPATHLEN];
  sprintf (fname, "%s/history.ini", DEFAULT_PATH);
  FILE *fp = fopen(fname, "rb");
  if (fp)
  {
    /* read file */
    fread(&history, sizeof(history), 1, fp);
    fclose(fp);
  }
}

void history_default(void)
{
  int i;
  for(i=0; i < NUM_HISTORY_ENTRIES; i++)
    memset(&history.entries[i], 0, sizeof(t_history_entry));

  /* restore history */
  history_load();
}



