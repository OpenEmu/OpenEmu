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

#include "mednafen.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <sndfile.h>

static SNDFILE *soundlog;

void MDFN_WriteWaveData(int16 *Buffer, int Count)
{
 if(!soundlog) return;
 if(!Buffer || !Count) return;

 sf_writef_short(soundlog, Buffer, Count);
}

int MDFNI_EndWaveRecord(void)
{
 if(!soundlog) return 0;

 sf_close(soundlog);
 soundlog = NULL;
 return 1;
}


int MDFNI_BeginWaveRecord(uint32 rate, int channels, char *fn)
{
 int tmpfd;

 tmpfd = open(fn, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR);

 if(tmpfd == -1)
 {
     MDFN_PrintError(_("Error opening %s: %s\n"), fn, strerror(errno));
     return(0);
 }

 SF_INFO slinfo;
 memset(&slinfo, 0, sizeof(SF_INFO));

 slinfo.samplerate = rate;
 slinfo.channels = channels;
 slinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16;

 if(!(soundlog=sf_open_fd(tmpfd, SFM_WRITE, &slinfo, 1)))
 {
     MDFN_PrintError(_("Error opening %s: %s\n"), fn, sf_strerror(soundlog));
     return 0;
 }

 return(1);
}
