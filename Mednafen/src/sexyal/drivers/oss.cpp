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
 ssize_t bytes;

 bytes = write(*(int *)device->private_data, data, len);

 if(bytes != len) 
 {
  puts("Write error?");
  return(0);
 }

 return(1);
}

static int RawCanWrite(SexyAL_device *device, uint32_t *can_write)
{
 struct audio_buf_info ai;

 if(!ioctl(*(int *)device->private_data,SNDCTL_DSP_GETOSPACE,&ai))
 {
  //printf("%d\n\n",ai.bytes);

  if(ai.bytes < 0)
   ai.bytes = 0; // ALSA is weird

  *can_write = ai.bytes;

  return(1);
 }
 else
  return(0);
}

static int Pause(SexyAL_device *device, int state)
{
 return(0);
}

static int Clear(SexyAL_device *device)
{
 ioctl(*(int *)device->private_data,SNDCTL_DSP_RESET,0);
 return(1);
}

static int RawClose(SexyAL_device *device)
{
 if(device)
 {
  if(device->private_data)
  {
   close(*(int*)device->private_data);
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
		if(device)			\
		{				\
		 if(device->private_data)	\
		  free(device->private_data);	\
		 free(device);			\
		 device = NULL;			\
		}

SexyAL_device *SexyALI_OSS_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering)
{
 SexyAL_device *device = NULL;
 int fd = -1;
 unsigned int temp, try_format;

 if((fd = open(id ? id : "/dev/dsp", O_WRONLY)) == -1)
 {
  puts(strerror(errno));
  return(0);
 }

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
 int fragsize = 256;
 
 if(buffering->ms)
 {
  int64_t tc;

  //printf("%d\n",buffering->ms);
  
  /* 2*, >>1, |1 for crude rounding(it will always round 0.5 up, so it is a bit biased). */

  tc=2*buffering->ms * format->rate / 1000 / fragsize;
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

  buffering->totalsize=fragsize * fragcount;
 }
 device->private_data=malloc(sizeof(int));
 device->RawCanWrite = RawCanWrite;
 device->RawWrite = RawWrite;
 device->RawClose = RawClose;
 device->Clear = Clear;
 device->Pause = Pause;

 *(int*)device->private_data=fd;

 {
  uint32_t can_write;

  if(!RawCanWrite(device, &can_write))
  {
   OSS_INIT_ERROR_CLEANUP
   return(0);
  } 

  buffering->latency = can_write / (format->sampformat>>4) / format->channels;
 }

 return(device);
}

