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
#include <errno.h>
#include <poll.h>
#include <sys/time.h>
#include <time.h>
#include <alsa/asoundlib.h>
#include <unistd.h>

#ifndef FALSE
#define FALSE 0
#endif

#ifndef TRUE
#define TRUE 1
#endif

typedef struct
{
	snd_pcm_t *alsa_pcm;

	uint32_t period_size;
	int interleaved;
	//bool heavy_sync;
} ADStruct;


// TODO:
SexyAL_enumdevice *SexyALI_ALSA_EnumerateDevices(void)
{
 return(NULL);
}

static int Pause(SexyAL_device *device, int state)
{
 if(0)
  snd_pcm_pause(((ADStruct *)device->private_data)->alsa_pcm, state);
 else
 {
  snd_pcm_drop(((ADStruct *)device->private_data)->alsa_pcm);
 }
 return(0);
}

static int Clear(SexyAL_device *device)
{
 snd_pcm_drop(((ADStruct *)device->private_data)->alsa_pcm);

 return(1);
}

static int RawCanWrite(SexyAL_device *device, uint32_t *can_write)
{
 ADStruct *ads = (ADStruct *)device->private_data;
 uint32_t ret;
 snd_pcm_sframes_t avail;


 while(/*(ads->heavy_sync && snd_pcm_hwsync(ads->alsa_pcm) < 0) ||*/ (avail = snd_pcm_avail_update(ads->alsa_pcm)) < 0)
 {
  // If the call to snd_pcm_avail() fails, try to figure out the status of the PCM stream and take the best action.
  switch(snd_pcm_state(ads->alsa_pcm))
  {
   // This shouldn't happen, but in case it does...
   default: //puts("What1?"); 
	    *can_write = device->buffering.buffer_size * (device->format.sampformat >> 4) * device->format.channels;
	    return(1);

   //default: break;
   //case SND_PCM_STATE_RUNNING: break;
   //case SND_PCM_STATE_PREPARED: break;

   case SND_PCM_STATE_PAUSED:
   case SND_PCM_STATE_DRAINING:
   case SND_PCM_STATE_OPEN:
   case SND_PCM_STATE_DISCONNECTED: *can_write = device->buffering.buffer_size * (device->format.sampformat >> 4) * device->format.channels;
				    return(1);

   case SND_PCM_STATE_SETUP:
   case SND_PCM_STATE_SUSPENDED:
   case SND_PCM_STATE_XRUN:
			   //puts("XRun1");
			   snd_pcm_prepare(ads->alsa_pcm);
			   *can_write = device->buffering.buffer_size * (device->format.sampformat >> 4) * device->format.channels;
			   return(1);
  }
 }

 ret = avail * (device->format.sampformat >> 4) * device->format.channels;

 if(ret < 0)
  ret = 0;

 *can_write = ret;

 return(1);
}

// TODO:  Provide de-interleaving code in SexyAL outside of individual drivers

static int RawWrite(SexyAL_device *device, const void *data, uint32_t len)
{
 ADStruct *ads = (ADStruct *)device->private_data;

 #if 0
 RawCanWrite(device);
 #endif

 
 //if(ads->heavy_sync)
 //{
 // uint32_t cw;
 // while(RawCanWrite(device, &cw) > 0 && ((int)cw) < len) usleep(750);
 //}

 while(len > 0)
 {
  int snore = 0;

  do
  {
   if(ads->interleaved)
    snore = snd_pcm_writei(ads->alsa_pcm, data, len / (device->format.sampformat>>4) / device->format.channels);
   else
   {
    if(device->format.channels == 1)
    {
     const void *foodata[1];
     foodata[0] = data;
     snore = snd_pcm_writen(ads->alsa_pcm, (void **)foodata, len / (device->format.sampformat>>4) / device->format.channels);
    }
    else
    {
     uint8_t meowdata[device->format.channels][len / device->format.channels] __attribute__ ((aligned(4)));
     void *foodata[device->format.channels];
     int i, ch;
     int max_moo = len / (device->format.sampformat>>4) / device->format.channels;
     int max_ch = device->format.channels;

     for(ch = 0; ch < max_ch; ch++)
      foodata[ch] = meowdata[ch];

     if((device->format.sampformat>>4) == 1)
     {
      for(ch = 0; ch < max_ch; ch++)
       for(i = 0; i < max_moo; i++)
        ((uint8_t *)meowdata[ch])[i] = ((const uint8_t *)data)[i * max_ch + ch];
     }
     else if((device->format.sampformat>>4) == 2)
     {
      for(ch = 0; ch < max_ch; ch++)
       for(i = 0; i < max_moo; i++)
        ((uint16_t *)meowdata[ch])[i] = ((const uint16_t *)data)[i * max_ch + ch];
     }
     else if((device->format.sampformat>>4) == 4)
     {
      for(ch = 0; ch < max_ch; ch++)
       for(i = 0; i < max_moo; i++)
        ((uint32_t *)meowdata[ch])[i] = ((const uint32_t *)data)[i * max_ch + ch];
     }
     snore = snd_pcm_writen(ads->alsa_pcm, foodata, max_moo);
    }
   }

   if(snore <= 0)
   { 
    switch(snd_pcm_state(ads->alsa_pcm))
    {
     // This shouldn't happen, but if it does, and there was an error, exit out of the loopie.
     default: //puts("What2");
	      if(snore < 0)
	       snore = len / (device->format.sampformat>>4) / device->format.channels;
	      break;

     // Don't unplug your sound card, silly human! ;)
     case SND_PCM_STATE_OPEN:
     case SND_PCM_STATE_DISCONNECTED: snore = len / (device->format.sampformat>>4) / device->format.channels; 
				      //usleep(1000);
				      break;

     case SND_PCM_STATE_SETUP:
     case SND_PCM_STATE_SUSPENDED:
     case SND_PCM_STATE_XRUN: //puts("XRun2");
                             snd_pcm_prepare(ads->alsa_pcm);
                             break;
    }
   }

  } while(snore <= 0);

  data = (const uint8_t*)data + snore * (device->format.sampformat>>4) * device->format.channels;

  len -= snore * (device->format.sampformat>>4) * device->format.channels;

  if(snd_pcm_state(ads->alsa_pcm) == SND_PCM_STATE_PREPARED)
   snd_pcm_start(ads->alsa_pcm);
 }

 return(1);
}

static int RawClose(SexyAL_device *device)
{
 if(device)
 {
  if(device->private_data)
  {
   ADStruct *ads = (ADStruct *)device->private_data;
   snd_pcm_close(ads->alsa_pcm);
   free(device->private_data);
  }
  free(device);
  return(1);
 }
 return(0);
}

#define ALSA_INIT_CLEANUP	\
         if(hw_params)  snd_pcm_hw_params_free(hw_params);      \
         if(alsa_pcm) snd_pcm_close(alsa_pcm);  \
         if(ads) free(ads);     \
         if(device) free(device);       

#define ALSA_TRY(func) { 	\
	int error; 	\
	error = func; 	\
	if(error < 0) 	\
	{ 		\
	 printf("ALSA Error: %s %s\n", #func, snd_strerror(error)); 	\
	 ALSA_INIT_CLEANUP	\
	 return(0); 		\
	} }

typedef struct
{
 int sexyal;
 snd_pcm_format_t alsa;
} ALSA_SAL_FMAP;

static ALSA_SAL_FMAP FormatMap[] =
{
 { SEXYAL_FMT_PCMU8, SND_PCM_FORMAT_U8 },
 { SEXYAL_FMT_PCMS8, SND_PCM_FORMAT_S8 },
 { SEXYAL_FMT_PCMU16, SND_PCM_FORMAT_U16 },
 { SEXYAL_FMT_PCMS16, SND_PCM_FORMAT_S16 },
 { SEXYAL_FMT_PCMFLOAT, SND_PCM_FORMAT_FLOAT },

 { SEXYAL_FMT_PCMU24, SND_PCM_FORMAT_U24 },
 { SEXYAL_FMT_PCMS24, SND_PCM_FORMAT_S24 },

 { SEXYAL_FMT_PCMU32, SND_PCM_FORMAT_U32 },
 { SEXYAL_FMT_PCMS32, SND_PCM_FORMAT_S32 },

};

static int Format_ALSA_to_SexyAL(const snd_pcm_format_t alsa_format)
{
 for(unsigned int i = 0; i < sizeof(FormatMap) / sizeof(ALSA_SAL_FMAP); i++)
 {
  if(FormatMap[i].alsa == alsa_format)
   return(FormatMap[i].sexyal);
 }
 printf("ALSA->SexyAL format not found: %d\n", alsa_format);
 return(-1);
}

static snd_pcm_format_t Format_SexyAL_to_ALSA(const int sexyal_format)
{
 for(unsigned int i = 0; i < sizeof(FormatMap) / sizeof(ALSA_SAL_FMAP); i++)
 {
  if(FormatMap[i].sexyal == sexyal_format)
   return(FormatMap[i].alsa);
 }
 printf("SexyAL->ALSA format not found: %d\n", sexyal_format);
 return(SND_PCM_FORMAT_UNKNOWN);
}


SexyAL_device *SexyALI_ALSA_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering)
{
 ADStruct *ads = NULL;
 SexyAL_device *device = NULL;
 snd_pcm_t *alsa_pcm = NULL;
 snd_pcm_hw_params_t *hw_params = NULL;
 snd_pcm_sw_params_t *sw_params = NULL;
 int interleaved = 1;
 int desired_pt;		// Desired period time, in MICROseconds.
 int desired_buffertime;	// Desired buffer time, in milliseconds
 //bool heavy_sync = FALSE;
 snd_pcm_format_t sampformat;


 desired_pt = buffering->period_us ? buffering->period_us : 1250;	// 1.25 milliseconds
 desired_buffertime = buffering->ms ? buffering->ms : 32; 		// 32 milliseconds

 // Try to force at least 2 channels...
 if(format->channels < 2)
  format->channels = 2;

 //...and at least >= 16-bit samples.  Doing so will allow us to achieve lower period sizes(since minimum period sizes in the ALSA core
 // are expressed in bytes).
 if(format->sampformat == SEXYAL_FMT_PCMU8 || format->sampformat == SEXYAL_FMT_PCMS8)
  format->sampformat = SEXYAL_FMT_PCMS16;


 sampformat = Format_SexyAL_to_ALSA(format->sampformat);

 ALSA_TRY(snd_pcm_open(&alsa_pcm, id ? id : "hw:0", SND_PCM_STREAM_PLAYBACK, 0));
 ALSA_TRY(snd_pcm_hw_params_malloc(&hw_params));
 ALSA_TRY(snd_pcm_sw_params_malloc(&sw_params));

 ALSA_TRY(snd_pcm_hw_params_any(alsa_pcm, hw_params));
 ALSA_TRY(snd_pcm_hw_params_set_periods_integer(alsa_pcm, hw_params));

 if(snd_pcm_hw_params_set_access(alsa_pcm, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED) < 0)
 {
  puts("Interleaved format not supported, trying non-interleaved instead. :(");
  ALSA_TRY(snd_pcm_hw_params_set_access(alsa_pcm, hw_params, SND_PCM_ACCESS_RW_NONINTERLEAVED));
  interleaved = 0;
 }

 if(snd_pcm_hw_params_set_format(alsa_pcm, hw_params, sampformat) < 0)
 {
  int try_format;
  #define NUM_TRY_FORMATS	9
  static const snd_pcm_format_t TryFormats[NUM_TRY_FORMATS] =
			    { 
			     SND_PCM_FORMAT_S16,
			     SND_PCM_FORMAT_U16,
                             SND_PCM_FORMAT_S24,
                             SND_PCM_FORMAT_U24,
                             SND_PCM_FORMAT_S32,
                             SND_PCM_FORMAT_U32,
			     SND_PCM_FORMAT_FLOAT,
			     SND_PCM_FORMAT_U8,
			     SND_PCM_FORMAT_S8
			    };
  printf("Desired sample format not supported, trying others...\n");

  for(try_format = 0; try_format < NUM_TRY_FORMATS; try_format++)
  {
   // Don't retry the original format!
   if(TryFormats[try_format] == sampformat)
    continue;

   if(snd_pcm_hw_params_set_format(alsa_pcm, hw_params, TryFormats[try_format]) >= 0)
   {
    sampformat = TryFormats[try_format];
    format->sampformat = Format_ALSA_to_SexyAL(TryFormats[try_format]);
    break;
   }
  }

  if(try_format == NUM_TRY_FORMATS)
  {
   // TODO: Perhaps we should concatenate all the errors for all the formats tried?
   printf("No tried formats supported?!\n");
   ALSA_INIT_CLEANUP
   return(0);
  }
 }

 #if SND_LIB_VERSION >= 0x10009
 ALSA_TRY(snd_pcm_hw_params_set_rate_resample(alsa_pcm, hw_params, 0));
 #endif

 unsigned int rrate = format->rate;
 ALSA_TRY(snd_pcm_hw_params_set_rate_near(alsa_pcm, hw_params, &rrate, 0));
 format->rate = rrate;

 if(snd_pcm_hw_params_set_channels(alsa_pcm, hw_params, format->channels) < 0)
 {
  if(format->channels == 2)
  {
   puts("Warning:  Couldn't set stereo sound, trying mono instead...");
   format->channels = 1;
  }
  else if(format->channels == 1)
   format->channels = 2;

  ALSA_TRY(snd_pcm_hw_params_set_channels(alsa_pcm, hw_params, format->channels));
 }

 // Limit desired_buffertime to what the sound card is capable of at this playback rate.
 {
  unsigned int btm = 0;
  int dir = 0;
  ALSA_TRY(snd_pcm_hw_params_get_buffer_time_max(hw_params, &btm, &dir));

  // btm > 32 may be unnecessary, but it's there in case ALSA returns a bogus value far too small...
  if(btm > 32 && desired_buffertime > btm)
   desired_buffertime = btm;

  //printf("BTM: %d\n", btm);
 }
 {
  int dir = 0;
  unsigned int max_periods;

  ALSA_TRY(snd_pcm_hw_params_get_periods_max(hw_params, &max_periods, &dir));
  if(((int64_t)desired_pt * max_periods) < ((int64_t)1000 * desired_buffertime))
  {
   //puts("\nHRMMM. max_periods is not large enough to meet desired buffering size at desired period time.\n");
   desired_pt = 1000 * desired_buffertime / max_periods;

   if(desired_pt > 5400)
    desired_pt = 5400;
  }
  //printf("Max Periods: %d\n", max_periods);
 }

 {
  snd_pcm_uframes_t tmpps = (int64_t)desired_pt * format->rate / (1000 * 1000);
  int dir = 0;

  snd_pcm_hw_params_set_period_size_near(alsa_pcm, hw_params, &tmpps, &dir);
 }

 snd_pcm_uframes_t tmp_uft;
 tmp_uft = desired_buffertime * format->rate / 1000;
 ALSA_TRY(snd_pcm_hw_params_set_buffer_size_near(alsa_pcm, hw_params, &tmp_uft));

 ALSA_TRY(snd_pcm_hw_params(alsa_pcm, hw_params));
 snd_pcm_uframes_t buffer_size, period_size;
 unsigned int periods;

 ALSA_TRY(snd_pcm_hw_params_get_period_size(hw_params, &period_size, NULL));
 ALSA_TRY(snd_pcm_hw_params_get_periods(hw_params, &periods, NULL));
 snd_pcm_hw_params_free(hw_params);

 ALSA_TRY(snd_pcm_sw_params_current(alsa_pcm, sw_params));

 #if 0
 ALSA_TRY(snd_pcm_sw_params_set_xrun_mode(alsa_pcm, sw_params, SND_PCM_XRUN_NONE));
 #endif

 ALSA_TRY(snd_pcm_sw_params(alsa_pcm, sw_params));
 snd_pcm_sw_params_free(sw_params);

 buffer_size = period_size * periods;

 buffering->period_size = period_size;
 buffering->buffer_size = buffer_size;
 buffering->latency = buffering->buffer_size;

 device = (SexyAL_device *)calloc(1, sizeof(SexyAL_device));
 ads = (ADStruct *)calloc(1, sizeof(ADStruct));

 ads->alsa_pcm = alsa_pcm;
 ads->period_size = period_size;
 ads->interleaved = interleaved;
 //ads->heavy_sync = heavy_sync;

 device->private_data = ads;
 device->RawWrite = RawWrite;
 device->RawCanWrite = RawCanWrite;
 device->RawClose = RawClose;
 device->Pause = Pause;
 device->Clear = Clear;

 memcpy(&device->buffering,buffering,sizeof(SexyAL_buffering));
 memcpy(&device->format,format,sizeof(SexyAL_format));

 ALSA_TRY(snd_pcm_prepare(alsa_pcm));

 return(device);
}

