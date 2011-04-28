/* SexyAL - Simple audio abstraction library.

Copyright (c) 2005-2007 Mednafen Team

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

#include "sexyal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>	// For debugging
#include <assert.h>

#ifdef HAVE_GETTIMEOFDAY
#include <sys/time.h>
#endif

#include <time.h>
#include <errno.h>

#include "convert.h"

/* kludge.  yay. */
SexyAL_enumdevice *SexyALI_OSS_EnumerateDevices(void);
SexyAL_device *SexyALI_OSS_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering);
SexyAL_device *SexyALI_JACK_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering);
SexyAL_device *SexyALI_SDL_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering);
SexyAL_device *SexyALI_DSound_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering);
SexyAL_device *SexyALI_Dummy_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering);

#ifdef HAVE_ALSA
SexyAL_enumdevice *SexyALI_ALSA_EnumerateDevices(void);
SexyAL_device *SexyALI_ALSA_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering);
#endif

static uint32_t FtoB(const SexyAL_format *format, uint32_t frames)
{
 return(frames*format->channels*(format->sampformat>>4));
}

static uint32_t BtoF(const SexyAL_format *format, uint32_t bytes)
{
 return(bytes / (format->channels * (format->sampformat>>4)));
}

static uint32_t CanWrite(SexyAL_device *device)
{
 uint32_t bytes;

 if(!device->RawCanWrite(device, &bytes))
  return(0);

 return(BtoF(&device->format, bytes));
}

static int Write(SexyAL_device *device, void *data, uint32_t frames)
{
 // The number of frames to convert and write to the output at once, and the chunk size
 // of writes to the device when no conversion is done.
 // Don't change this much, the drivers aren't written to handle enormous lengths passed to RawWrite().
 #define CONVERT_CHUNK_SIZE	512 //2048	//2048

 uint8_t buffer[CONVERT_CHUNK_SIZE * 4 * 2]; // Maximum frame size, 4 bytes * 2 channels

 if(device->srcformat.sampformat == device->format.sampformat &&
	device->srcformat.channels == device->format.channels &&
	device->srcformat.rate == device->format.rate &&
	device->srcformat.revbyteorder == device->format.revbyteorder)
 {
  const uint8_t *data_in = (const uint8_t *)data;

  while(frames)
  {
   int32_t write_this_iteration;

   write_this_iteration = frames;
   if(write_this_iteration > CONVERT_CHUNK_SIZE)
    write_this_iteration = CONVERT_CHUNK_SIZE;

   if(!device->RawWrite(device, data_in, FtoB(&device->format, write_this_iteration)))
    return(0);

   frames -= write_this_iteration;
   data_in += FtoB(&device->srcformat, write_this_iteration);
  }
 }
 else
 {
  const uint8_t *data_in = (const uint8_t *)data;

  while(frames)
  {
   int32_t convert_this_iteration;

   convert_this_iteration = frames;
   if(convert_this_iteration > CONVERT_CHUNK_SIZE) 
    convert_this_iteration = CONVERT_CHUNK_SIZE;

   SexiALI_Convert(&device->srcformat, &device->format, data_in, buffer, convert_this_iteration);

   if(!device->RawWrite(device, buffer, FtoB(&device->format, convert_this_iteration)))
    return(0);

   frames -= convert_this_iteration;
   data_in += FtoB(&device->srcformat, convert_this_iteration);
  }
 }

 return(1);
}

static int Close(SexyAL_device *device)
{
 return(device->RawClose(device));
}

int SetConvert(struct __SexyAL_device *device, SexyAL_format *format)
{
 memcpy(&device->srcformat,format,sizeof(SexyAL_format));
 return(1);
}

void Destroy(SexyAL *iface)
{
 free(iface);
}

static SexyAL_driver drivers[] = 
{
        #if HAVE_ALSA
        { SEXYAL_TYPE_ALSA, "ALSA", "alsa", SexyALI_ALSA_Open, SexyALI_ALSA_EnumerateDevices },
        #endif

	#if HAVE_OSSDSP
	{ SEXYAL_TYPE_OSSDSP, "OSS(/dev/dsp*)", "oss", SexyALI_OSS_Open, SexyALI_OSS_EnumerateDevices },
	#endif

        #if HAVE_DIRECTSOUND
        { SEXYAL_TYPE_DIRECTSOUND, "DirectSound", "dsound", SexyALI_DSound_Open, NULL },
        #endif

        #if HAVE_SDL
        { SEXYAL_TYPE_SDL, "SDL", "sdl", SexyALI_SDL_Open, NULL },
        #endif

        #if HAVE_JACK
        { SEXYAL_TYPE_JACK, "JACK", "jack", SexyALI_JACK_Open, NULL },
        #endif

	{ SEXYAL_TYPE_DUMMY, "Dummy", "dummy", SexyALI_Dummy_Open, NULL },

	{ 0, NULL, NULL, NULL, NULL }
};

static SexyAL_driver *FindDriver(int type)
{
 int x = 0;

 while(drivers[x].name)
 {
  if(drivers[x].type == type)
   return(&drivers[x]);

  x++;
 }
 return(0);
}

static SexyAL_device *Open(SexyAL *iface, const char *id, SexyAL_format *format, SexyAL_buffering *buffering, int type)
{
 SexyAL_device *ret;
 SexyAL_driver *driver;

 driver = FindDriver(type);
 if(!driver)
  return(0);

 if(id && id[0] == 0) // Set pointer to NULL on empty string.
  id = NULL;

 if(id)
 {
  if(!strcmp(id, "default"))
   id = NULL;
  else if(!strncmp(id, "sexyal-literal-", strlen("sexyal-literal-")))
   id += strlen("sexyal-literal-");
 }

 assert(0 == buffering->buffer_size);
 assert(0 == buffering->period_size);
 assert(0 == buffering->latency);

 if(!(ret = driver->Open(id, format, buffering)))
  return(0);

 assert(0 != buffering->buffer_size);
 //assert(0 != buffering->period_size);
 assert(0 != buffering->latency);

 buffering->ms = (uint64_t)buffering->buffer_size * 1000 / format->rate;
 buffering->period_us = (uint64_t)buffering->period_size * (1000 * 1000) / format->rate;

 ret->Write = Write;
 ret->Close = Close;
 ret->CanWrite = CanWrite;
 ret->SetConvert = SetConvert;

 return(ret);
}

static SexyAL_enumtype *EnumerateTypes(SexyAL *sal)
{
 SexyAL_enumtype *typies;
 int numdrivers = sizeof(drivers) / sizeof(SexyAL_driver);
 int x;

 typies = (SexyAL_enumtype *)malloc(numdrivers * sizeof(SexyAL_enumtype));
 memset(typies, 0, numdrivers * sizeof(SexyAL_enumtype));

 x = 0;

 do
 {
  typies[x].name = drivers[x].name;
  typies[x].short_name = drivers[x].short_name;
  typies[x].type = drivers[x].type;
 } while(drivers[x++].name);

 return(typies);
}

static SexyAL_enumdevice * EnumerateDevices(SexyAL *iface, int type)
{
 SexyAL_driver *driver;

 driver = FindDriver(type);

 if(!driver)
  return(0);

 if(driver->EnumerateDevices)
  return(driver->EnumerateDevices());

 return(0);
}

void *SexyAL_Init(int version)
{
 SexyAL *iface;

 iface = (SexyAL *)malloc(sizeof(SexyAL));

 iface->Open=Open;
 iface->Destroy=Destroy;

 iface->EnumerateTypes = EnumerateTypes;
 iface->EnumerateDevices = EnumerateDevices;
 return((void *)iface);
}


// Source: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
// Rounds up to the nearest power of 2.
uint32_t SexyAL_rupow2(uint32_t v)
{
 v--;
 v |= v >> 1;
 v |= v >> 2;
 v |= v >> 4;
 v |= v >> 8;
 v |= v >> 16;
 v++;

 v += (v == 0);

 return(v);
}

int32_t SexyAL_rnearestpow2(int32_t v, bool round_halfway_up)
{
 int32_t upper, lower;
 int32_t diff_upper, diff_lower;

 upper = SexyAL_rupow2(v);
 lower = upper >> 1;

 if(!lower)
  lower = 1;

 diff_upper = abs(v - upper);
 diff_lower = abs(v - lower);

 if(diff_upper == diff_lower)
 {
  return(round_halfway_up ? upper : lower);
 }
 else if(diff_upper < diff_lower)
  return(upper);

 return(lower);
}

// Returns (preferably-monotonic) time in microseconds.
int64_t SexyAL_Time64(void)
{
 static bool cgt_fail_warning = 0;

 #if HAVE_CLOCK_GETTIME && ( _POSIX_MONOTONIC_CLOCK > 0 || defined(CLOCK_MONOTONIC))
 struct timespec tp;

 if(clock_gettime(CLOCK_MONOTONIC, &tp) == -1)
 {
  if(!cgt_fail_warning)
   printf("clock_gettime() failed: %s\n", strerror(errno));
  cgt_fail_warning = 1;
 }
 else
 {
  static bool res_test = 0;
  struct timespec res;

  if(!res_test && clock_getres(CLOCK_MONOTONIC, &res) != -1)
  {
   printf("%lld %ld\n", (long long)res.tv_sec, (long)res.tv_nsec);
   res_test = 1;
  }

  return((int64_t)tp.tv_sec * (1000 * 1000) + tp.tv_nsec / 1000);
 }
 #else
   #warning "SexyAL: clock_gettime() with CLOCK_MONOTONIC not available"
 #endif


 #if HAVE_GETTIMEOFDAY
 // Warning: gettimeofday() is not guaranteed to be monotonic!!
 struct timeval tv;

 if(gettimeofday(&tv, NULL) == -1)
 {
  puts("gettimeofday() error");
  return(0);
 }

 return((int64_t)tv.tv_sec * 1000000 + tv.tv_usec);
 #else
  #warning "SexyAL: gettimeofday() not available!!!"
 #endif

 // Yeaaah, this isn't going to work so well.
 return((int64_t)time(NULL) * 1000000);
}
