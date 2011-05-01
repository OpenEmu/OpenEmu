/* SexyAL - Simple audio abstraction library.

Copyright (c) 2005 Mednafen Team

Permission is hereby granted, free of charge, to any person obtaining
a copy of this software and associated documentation files (the
"Software"), to deal in the Software without restriction, including
without limitation the rights to use, copy, modify, merge, publish,
distribute, sublicense, and/or sell copies of the Software, and to
permit persons to whom the Software is furnished to do so, subject to
the following conditions:

The above copyright notice and this permission notice shall be included
in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include "../sexyal.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sched.h>
#include <sys/soundcard.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct
{
	int fd;
	bool alsa_workaround;		// TRUE if we're using any OSS older than version 4, which includes ALSA(which conforms to 3.x).
					// Applying the workaround on non-ALSA where it's not needed will not hurt too much, it'll
					// just make the stuttering due to buffer underruns a little more severe.
					// (The workaround is to fix a bug which affects, at least, ALSA 1.0.20 when used with a CS46xx card)
	uint8_t *dummy_data;
	uint32_t dummy_data_len;
} OSS_Wrap;


SexyAL_enumdevice *SexyALI_OSS_EnumerateDevices(void)
{
 SexyAL_enumdevice *ret,*tmp,*last;
 struct stat buf;
 char fn[64];
 char numstring[64];
 unsigned int n;

 n = 0;

 ret = tmp = last = 0;

 for(;;)
 {
  snprintf(numstring, 64, "%d", n);
  snprintf(fn, 64, "/dev/dsp%s", numstring);

  if(stat(fn,&buf)!=0) break;

  tmp = (SexyAL_enumdevice *)calloc(1, sizeof(SexyAL_enumdevice));

  tmp->name = strdup(fn);
  tmp->id = strdup(numstring);

  if(!ret) ret = tmp;
  if(last) last->next = tmp;

  last = tmp;
  n++;
 } 
 return(ret);
}

unsigned int Log2(unsigned int value)
{
 int x=0;

 value>>=1;
 while(value)
 {
  value>>=1;
  x++;
 }
 return(x?x:1);
}

static int RawWrite(SexyAL_device *device, const void *data, uint32_t len)
{
 OSS_Wrap *ossw = (OSS_Wrap *)device->private_data;
 const uint8_t *datau8 = (const uint8_t *)data;

 while(len)
 {
  ssize_t bytes = write(ossw->fd, datau8, len);

  if(bytes < 0)
  {
   if(errno == EINTR)
    continue;

   fprintf(stderr, "OSS: %d, %m\n", errno);
   return(0);
  }
  else if(bytes > len)
  {
   fprintf(stderr, "OSS: written bytes > len ???\n");
   bytes = len;
  }
  len -= bytes;
  datau8 += bytes;
 }

 return(1);
}

static int RawCanWrite(SexyAL_device *device, uint32_t *can_write)
{
 OSS_Wrap *ossw = (OSS_Wrap *)device->private_data;
 struct audio_buf_info ai;

 TryAgain:

 if(!ioctl(ossw->fd, SNDCTL_DSP_GETOSPACE, &ai))
 {
  if(ai.bytes < 0)
   ai.bytes = 0; // ALSA is weird

  if(ossw->alsa_workaround && (unsigned int)ai.bytes >= (device->buffering.buffer_size * (device->format.sampformat >> 4) * device->format.channels))
  {
   //puts("Underflow fix");
   //fprintf(stderr, "%d\n",ai.bytes);
   if(!RawWrite(device, ossw->dummy_data, ossw->dummy_data_len))
    return(0);
   goto TryAgain;
  }

  *can_write = ai.bytes;

  return(1);
 }
 else
 {
  puts("Error");
  return(0);
 }
 return(1);
}

static int Pause(SexyAL_device *device, int state)
{
 return(0);
}

static int Clear(SexyAL_device *device)
{
 OSS_Wrap *ossw = (OSS_Wrap *)device->private_data;

 ioctl(ossw->fd, SNDCTL_DSP_RESET, 0);
 return(1);
}

static int RawClose(SexyAL_device *device)
{
 if(device)
 {
  if(device->private_data)
  {
   OSS_Wrap *ossw = (OSS_Wrap *)device->private_data;

   if(ossw->fd != -1)
   {
    close(ossw->fd);
    ossw->fd = -1;
   }
   free(device->private_data);
  }
  free(device);
  return(1);
 }
 return(0);
}

#define OSS_INIT_ERROR_CLEANUP			\
		if(fd != -1)			\
		{				\
		 close(fd);			\
		 fd = -1;			\
		}				\
		if(ossw)			\
		{				\
		 free(ossw);			\
		}				\
		if(device)			\
		{				\
		 free(device);			\
		 device = NULL;			\
		}

SexyAL_device *SexyALI_OSS_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering)
{
 SexyAL_device *device = NULL;
 int fd = -1;
 unsigned int temp, try_format;
 OSS_Wrap *ossw = NULL;
 int version = 0;
 bool alsa_workaround = FALSE;
 int desired_pt;                // Desired period time, in MICROseconds.
 int desired_buffertime;        // Desired buffer time, in milliseconds

 desired_pt = buffering->period_us ? buffering->period_us : 1250;       // 1.25 milliseconds
 desired_buffertime = buffering->ms ? buffering->ms : 32;               // 32 milliseconds

 if((fd = open(id ? id : "/dev/dsp", O_WRONLY)) == -1)
 {
  puts(strerror(errno));
  return(0);
 }
 
 if(ioctl(fd, OSS_GETVERSION, &version) == -1 || version < 0x040000)
 {
  puts("\nALSA SNDCTL_DSP_GETOSPACE internal-state-corruption bug workaround mode used.");
  alsa_workaround = TRUE;
 }

 // Try to force at least 16-bit output and 2 channels so we can get lower period sizes(assuming the low-level device driver
 // expresses minimum period size in bytes).
 if(format->channels < 2)
  format->channels = 2;
 if(format->sampformat == SEXYAL_FMT_PCMU8 || format->sampformat == SEXYAL_FMT_PCMS8)
  format->sampformat = SEXYAL_FMT_PCMS16;

 /* Set sample format. */
 /* TODO:  Handle devices with byte order different from native byte order. */
 /* TODO:  Fix fragment size calculation to work well with lower/higher playback rates,
    as reported by OSS.
 */

 if(format->sampformat == SEXYAL_FMT_PCMU8)
  try_format = AFMT_U8;
 else if(format->sampformat == SEXYAL_FMT_PCMS8)
  try_format = AFMT_S8;
 else if(format->sampformat == SEXYAL_FMT_PCMU16)
  try_format = AFMT_U16_LE;
 else 
  try_format = AFMT_S16_NE;

 format->revbyteorder = 0;

 temp = try_format;
 if(ioctl(fd, SNDCTL_DSP_SETFMT, &temp) == -1 && temp == try_format)
 {
  puts(strerror(errno));
  close(fd);
  return(0);
 }

 switch(temp)
 {
  case AFMT_U8: format->sampformat = SEXYAL_FMT_PCMU8;
		break;

  case AFMT_S8: format->sampformat = SEXYAL_FMT_PCMS8;
		break;

  case AFMT_U16_LE: 
		    #ifndef LSB_FIRST
                    format->revbyteorder=1;
                    #endif
		    format->sampformat = SEXYAL_FMT_PCMU16;
		    break;

  case AFMT_U16_BE: 
		    #ifdef LSB_FIRST
                    format->revbyteorder=1;
                    #endif
		    format->sampformat = SEXYAL_FMT_PCMU16;
		    break;

  case AFMT_S16_LE: 
		    #ifndef LSB_FIRST
		    format->revbyteorder=1;
		    #endif
		    format->sampformat = SEXYAL_FMT_PCMS16;
		    break;

  case AFMT_S16_BE: 
		    #ifdef LSB_FIRST
                    format->revbyteorder=1;
                    #endif
		    format->sampformat = SEXYAL_FMT_PCMS16;
		    break;

  default:	    close(fd);
		    return(0);
 }

 /* Set number of channels. */
 temp=format->channels;
 if(ioctl(fd,SNDCTL_DSP_CHANNELS,&temp)==-1)
 {
  close(fd);
  return(0);
 }

 if(temp < 1 || temp > 2)
 {
  close(fd);
  return(0);
 }

 format->channels = temp;

 /* Set frame rate. */
 temp = format->rate;
 if(ioctl(fd,SNDCTL_DSP_SPEED,&temp)==-1)
 {
  close(fd);
  return(0);
 }
 format->rate = temp;

 device = (SexyAL_device *)malloc(sizeof(SexyAL_device));
 memcpy(&device->format,format,sizeof(SexyAL_format));
 memcpy(&device->buffering,buffering,sizeof(SexyAL_buffering));

 int fragcount = 16;
 int fragsize = SexyAL_rnearestpow2((int64_t)desired_pt * format->rate / (1000 * 1000), FALSE); //128; //256;

 // Going lower than this is unlikely to work, and since we use this value to calculate the number of fragments, we'll get
 // a buffer far larger than we wanted, unless the OSS implementation is nice and adjusts the fragment count when it can't get set fragment size
 // specified.
 if(fragsize < 16)
  fragsize = 16;

 if(fragsize > 512)
  fragsize = 512;

 {
  int64_t tc;

  /* 2*, >>1, |1 for crude rounding(it will always round 0.5 up, so it is a bit biased). */

  tc=2 * desired_buffertime * format->rate / 1000 / fragsize;
  fragcount=(tc>>1)+(tc&1); //1<<Log2(tc);
 }

 temp=Log2(fragsize*(format->sampformat>>4)*format->channels);

 temp|=fragcount<<16;

 ioctl(fd,SNDCTL_DSP_SETFRAGMENT,&temp);

 {
  audio_buf_info info;
  ioctl(fd,SNDCTL_DSP_GETOSPACE,&info);

  fragsize=info.fragsize/(format->sampformat>>4)/format->channels;
  fragcount=info.fragments;

  buffering->buffer_size=fragsize * fragcount;
  buffering->period_size = fragsize;
 }

 if(!(ossw = (OSS_Wrap *)calloc(1, sizeof(OSS_Wrap))))
 {
  OSS_INIT_ERROR_CLEANUP
  return(NULL);
 }

 ossw->dummy_data_len = (format->sampformat >> 4) * format->channels * (format->rate / 128);
 if(!(ossw->dummy_data = (uint8_t *)calloc(1, ossw->dummy_data_len)))
 {
  OSS_INIT_ERROR_CLEANUP
  return(NULL);
 }

 if(format->sampformat == SEXYAL_FMT_PCMU8 || format->sampformat == SEXYAL_FMT_PCMU16)
  memset(ossw->dummy_data, 0x80, ossw->dummy_data_len);
 else
  memset(ossw->dummy_data, 0, ossw->dummy_data_len);

 ossw->fd = fd;

 ossw->alsa_workaround = alsa_workaround;

 device->private_data = ossw;
 device->RawCanWrite = RawCanWrite;
 device->RawWrite = RawWrite;
 device->RawClose = RawClose;
 device->Clear = Clear;
 device->Pause = Pause;

 
 buffering->latency = buffering->buffer_size;

 memcpy(&device->buffering,buffering,sizeof(SexyAL_buffering));
 memcpy(&device->format,format,sizeof(SexyAL_format));

 return(device);
}

