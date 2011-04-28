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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>


// FIXME?
#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#endif


typedef struct
{
 int paused;
 int64_t paused_time;

 int64_t buffering_us;

 int64_t data_written_to;

} Dummy_Driver_t;

static int RawCanWrite(SexyAL_device *device, uint32_t *can_write)
{
 Dummy_Driver_t *dstate = (Dummy_Driver_t *)device->private_data;
 uint32_t ret;
 int64_t curtime = SexyAL_Time64();

 if(dstate->paused)
  curtime = dstate->paused_time;

 if(curtime < dstate->data_written_to)
 {
  ret = 0;
  //printf("%ld\n", curtime - dstate->data_written_to);
 }
 else
  ret = (curtime - dstate->data_written_to) / 1000 * device->format.rate / 1000;

 if(ret > device->buffering.buffer_size)
 {
  ret = device->buffering.buffer_size;
  dstate->data_written_to = curtime - dstate->buffering_us;
 }

 *can_write = ret * device->format.channels * (device->format.sampformat >> 4);

 return(1);
}

static int RawWrite(SexyAL_device *device, const void *data, uint32_t len)
{
 Dummy_Driver_t *dstate = (Dummy_Driver_t *)device->private_data;

 while(len)
 {
  uint32_t can_write = 0;

  RawCanWrite(device, &can_write);

  if(can_write > len) 
   can_write = len;

  dstate->data_written_to += can_write / (device->format.channels * (device->format.sampformat >> 4)) * 1000000 / device->format.rate;

  len -= can_write;

  if(len)
  {
   #ifdef HAVE_USLEEP
   usleep(1000);
   #elif defined(WIN32)
   Sleep(1);
   #endif
  }
 }

 return(1);
}

static int Pause(SexyAL_device *device, int state)
{
 Dummy_Driver_t *dstate = (Dummy_Driver_t *)device->private_data;

 if(state != dstate->paused)
 {
  dstate->paused = state;

  if(state)
  {
   dstate->paused_time = SexyAL_Time64();
  }
  else
  {
   dstate->data_written_to = SexyAL_Time64() - (dstate->paused_time - dstate->data_written_to);
  }
 }

 return(dstate->paused);
}

static int Clear(SexyAL_device *device)
{
 Dummy_Driver_t *dstate = (Dummy_Driver_t *)device->private_data;
 int64_t curtime = SexyAL_Time64();

 if(dstate->paused)
  curtime = dstate->paused_time;

 dstate->data_written_to = curtime - dstate->buffering_us;

 return(1);
}

static int RawClose(SexyAL_device *device)
{
 Dummy_Driver_t *dstate = (Dummy_Driver_t *)device->private_data;

 if(dstate)
 {
  free(dstate);
  device->private_data = NULL;
 }

 return(1);
}


#define DUMMY_INIT_CLEANUP		\
	if(device) free(device);	\
	if(dstate) free(dstate);

SexyAL_device *SexyALI_Dummy_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering)
{
 SexyAL_device *device = NULL;
 Dummy_Driver_t *dstate = NULL;

 if(!(device = (SexyAL_device *)calloc(1, sizeof(SexyAL_device))))
 {
  DUMMY_INIT_CLEANUP
  return(NULL);
 }

 if(!(dstate = (Dummy_Driver_t *)calloc(1, sizeof(Dummy_Driver_t))))
 {
  DUMMY_INIT_CLEANUP
  return(NULL);
 }

 device->private_data = dstate;

 if(!buffering->ms) 
  buffering->ms = 32;

 buffering->buffer_size = buffering->ms * format->rate / 1000;
 buffering->ms = buffering->buffer_size * 1000 / format->rate;
 buffering->latency = buffering->buffer_size;

 dstate->buffering_us = (int64_t)buffering->buffer_size * 1000 * 1000 / format->rate;

 memcpy(&device->format, format, sizeof(SexyAL_format));
 memcpy(&device->buffering, buffering, sizeof(SexyAL_buffering));

 device->RawCanWrite = RawCanWrite;
 device->RawWrite = RawWrite;
 device->RawClose = RawClose;
 device->Clear = Clear;
 device->Pause = Pause;

 return(device);
}

