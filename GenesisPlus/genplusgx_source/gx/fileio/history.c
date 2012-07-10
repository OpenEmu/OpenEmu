/*
 *  history.c
 *
 *   Generic ROM history list managment
 *
 *  Copyright Eke-Eke (2008-2012), based on original code from Martin Disibio (2008)
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
void history_add_file(char *filepath, char *filename, u8 filetype)
{
  /* Create the new entry for this path. */
  t_history_entry newentry;
  strncpy(newentry.filepath, filepath, MAXJOLIET - 1);
  strncpy(newentry.filename, filename, MAXJOLIET - 1);
  newentry.filepath[MAXJOLIET - 1] = '\0';
  newentry.filename[MAXJOLIET - 1] = '\0';
  newentry.filetype = filetype;
  
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
    if (fread(&history, sizeof(history), 1, fp) != 1)
    {
      /* an error ocurred, better clear hoistory */
      memset(&history, 0, sizeof(history));
    }

    /* close file */
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



