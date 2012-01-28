/*
 Copyright (c) 2008 Francisco Muñoz 'Hermes' <www.elotrolado.net>
 All rights reserved.

 Proper (standard) vorbis usage by Tantric, 2009
 Threading modifications/corrections by Tantric, 2009

 Redistribution and use in source and binary forms, with or without modification, are
 permitted provided that the following conditions are met:

 - Redistributions of source code must retain the above copyright notice, this list of
 conditions and the following disclaimer.
 - Redistributions in binary form must reproduce the above copyright notice, this list
 of conditions and the following disclaimer in the documentation and/or other
 materials provided with the distribution.
 - The names of the contributors may not be used to endorse or promote products derived
 from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
 THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "oggplayer.h"
#include <gccore.h>
#include <unistd.h>
#include <string.h>

/* functions to read the Ogg file from memory */

static struct
{
  char *mem;
  int size;
  int pos;
} file[4];

static int f_read(void * punt, int bytes, int blocks, int *f)
{
  int b;
  int c;
  int d;

  if (bytes * blocks <= 0)
    return 0;

  blocks = bytes * blocks;
  c = 0;

  while (blocks > 0)
  {
    b = blocks;
    if (b > 4096)
      b = 4096;

    if (*f >= 0x666 && *f <= 0x669)
    {
      d = (*f) - 0x666;
      if (file[d].size == 0)
        return -1;
      if ((file[d].pos + b) > file[d].size)
        b = file[d].size - file[d].pos;
      if (b > 0)
      {
        memcpy(punt, file[d].mem + file[d].pos, b);
        file[d].pos += b;
      }
    }
    else
      b = read(*f, ((char *) punt) + c, b);

    if (b <= 0)
    {
      return c / bytes;
    }
    c += b;
    blocks -= b;
  }
  return c / bytes;
}

static int f_seek(int *f, ogg_int64_t offset, int mode)
{
  if(f==NULL) return(-1);

  int k, d;
  mode &= 3;
  if (*f >= 0x666 && *f <= 0x669)
  {
    d = (*f) - 0x666;
    k = 0;

    if (file[d].size == 0)
      return -1;

    if (mode == 0)
    {
      if ((offset) >= file[d].size)
      {
        file[d].pos = file[d].size;
        k = -1;
      }
      else if ((offset) < 0)
      {
        file[d].pos = 0;
        k = -1;
      }
      else
        file[d].pos = offset;
    }
    if (mode == 1)
    {
      if ((file[d].pos + offset) >= file[d].size)
      {
        file[d].pos = file[d].size;
        k = -1;
      }
      else if ((file[d].pos + offset) < 0)
      {
        file[d].pos = 0;
        k = -1;
      }
      else
        file[d].pos += offset;
    }
    if (mode == 2)
    {

      if ((file[d].size + offset) >= file[d].size)
      {
        file[d].pos = file[d].size;
        k = -1;
      }
      else if ((file[d].size + offset) < 0)
      {
        file[d].pos = 0;
        k = -1;
      }
      else
        file[d].pos = file[d].size + offset;
    }

  }
  else
    k = lseek(*f, (int) offset, mode);

  if (k < 0)
    k = -1;
  else
    k = 0;
  return k;
}

static int f_close(int *f)
{
  int d;
  if (*f >= 0x666 && *f <= 0x669)
  {
    d = (*f) - 0x666;
    file[d].size = 0;
    file[d].pos = 0;
    if (file[d].mem)
    {
      file[d].mem = (void *) 0;
    }
    return 0;
  }
  else
    return close(*f);
  return 0;
}

static long f_tell(int *f)
{
  int k, d;

  if (*f >= 0x666 && *f <= 0x669)
  {
    d = (*f) - 0x666;
    k = file[d].pos;
  }
  else
    k = lseek(*f, 0, 1);

  return (long) k;
}

static int mem_open(char * ogg, int size)
{
  static int one = 1;
  int n;
  if (one)
  {
    one = 0;
    for (n = 0; n < 4; n++)
      file[n].size = 0;
  }

  for (n = 0; n < 4; n++)
  {
    if (file[n].size == 0)
    {
      file[n].mem = ogg;
      file[n].size = size;
      file[n].pos = 0;
      return (0x666 + n);
    }
  }
  return -1;
}

static int mem_close(int fd)
{
  if (fd >= 0x666 && fd <= 0x669) // it is a memory file descriptor?
  {
    fd -= 0x666;
    file[fd].size = 0;
    return 0;
  }
  else
    return f_close(&fd);
}

static ov_callbacks callbacks = {
  (size_t (*)(void *, size_t, size_t, void *))  f_read,
  (int (*)(void *, ogg_int64_t, int))           f_seek,
  (int (*)(void *))                             f_close,
  (long (*)(void *))                            f_tell
};

/* OGG control */

#define READ_SAMPLES 4096 /* samples that it must read before to send */
#define MAX_PCMOUT 4096 /* minimum size to read ogg samples */
typedef struct
{
  OggVorbis_File vf;
  vorbis_info *vi;
  int current_section;

  /* OGG file operation */
  int fd;
  int mode;
  int eof;
  int flag;
  int volume;
  int seek_time;

  /* OGG buffer control */
  short pcmout[2][READ_SAMPLES + MAX_PCMOUT * 2]; /* take 4k out of the data segment, not the stack */
  int pcmout_pos;
  int pcm_indx;

} private_data_ogg;

static private_data_ogg private_ogg;

/* OGG thread control */

#define STACKSIZE    8192

static u8 oggplayer_stack[STACKSIZE];
static lwpq_t oggplayer_queue = LWP_TQUEUE_NULL;
static lwp_t h_oggplayer = LWP_THREAD_NULL;
static int ogg_thread_running = 0;

static void ogg_add_callback(int voice)
{
  if (!ogg_thread_running)
  {
    ASND_StopVoice(0);
    return;
  }

  if (private_ogg.flag & 128)
    return; /* Ogg is paused */

  if (private_ogg.pcm_indx >= READ_SAMPLES)
  {
    if (ASND_AddVoice(0,
        (void *) private_ogg.pcmout[private_ogg.pcmout_pos],
        private_ogg.pcm_indx << 1) == 0)
    {
      private_ogg.pcmout_pos ^= 1;
      private_ogg.pcm_indx = 0;
      private_ogg.flag = 0;
      LWP_ThreadSignal(oggplayer_queue);
    }
  }
  else
  {
    if (private_ogg.flag & 64)
    {
      private_ogg.flag &= ~64;
      LWP_ThreadSignal(oggplayer_queue);
    }
  }
}

static void * ogg_player_thread(private_data_ogg * priv)
{
  int first_time = 1;
  long ret;

  /* init */
  LWP_InitQueue(&oggplayer_queue);

  priv[0].vi = ov_info(&priv[0].vf, -1);

  ASND_Pause(0);

  priv[0].pcm_indx = 0;
  priv[0].pcmout_pos = 0;
  priv[0].eof = 0;
  priv[0].flag = 0;
  priv[0].current_section = 0;

  ogg_thread_running = 1;

  while (!priv[0].eof && ogg_thread_running)
  {
    if (priv[0].flag)
      LWP_ThreadSleep(oggplayer_queue); /* wait only when i have samples to send */

    if (priv[0].flag == 0) /* wait to all samples are sended */
    {
      if (ASND_TestPointer(0, priv[0].pcmout[priv[0].pcmout_pos])
          && ASND_StatusVoice(0) != SND_UNUSED)
      {
        priv[0].flag |= 64;
        continue;
      }
      if (priv[0].pcm_indx < READ_SAMPLES)
      {
        priv[0].flag = 3;

        if (priv[0].seek_time >= 0)
        {
          ov_time_seek(&priv[0].vf, priv[0].seek_time);
          priv[0].seek_time = -1;
        }

        ret
            = ov_read(
                &priv[0].vf,
                (void *) &priv[0].pcmout[priv[0].pcmout_pos][priv[0].pcm_indx],
                MAX_PCMOUT,/*0,2,1,*/&priv[0].current_section);
        priv[0].flag &= 192;
        if (ret == 0)
        {
          /* EOF */
          if (priv[0].mode & 1)
            ov_time_seek(&priv[0].vf, 0); /* repeat */
          else
            priv[0].eof = 1; /* stops */
        }
        else if (ret < 0)
        {
          /* error in the stream.  Not a problem, just reporting it in
           case we (the app) cares.  In this case, we don't. */
          if (ret != OV_HOLE)
          {
            if (priv[0].mode & 1)
              ov_time_seek(&priv[0].vf, 0); /* repeat */
            else
              priv[0].eof = 1; /* stops */
          }
        }
        else
        {
          /* we don't bother dealing with sample rate changes, etc, but
           you'll have to */
          priv[0].pcm_indx += ret >> 1; /* get 16 bits samples */
        }
      }
      else
        priv[0].flag = 1;
    }

    if (priv[0].flag == 1)
    {
      if (ASND_StatusVoice(0) == SND_UNUSED || first_time)
      {
        first_time = 0;
        if (priv[0].vi->channels == 2)
        {
          ASND_SetVoice(0, VOICE_STEREO_16BIT, priv[0].vi->rate, 0,
              (void *) priv[0].pcmout[priv[0].pcmout_pos],
              priv[0].pcm_indx << 1, priv[0].volume,
              priv[0].volume, ogg_add_callback);
          priv[0].pcmout_pos ^= 1;
          priv[0].pcm_indx = 0;
          priv[0].flag = 0;
        }
        else
        {
          ASND_SetVoice(0, VOICE_MONO_16BIT, priv[0].vi->rate, 0,
              (void *) priv[0].pcmout[priv[0].pcmout_pos],
              priv[0].pcm_indx << 1, priv[0].volume,
              priv[0].volume, ogg_add_callback);
          priv[0].pcmout_pos ^= 1;
          priv[0].pcm_indx = 0;
          priv[0].flag = 0;
        }
      }
    }
    usleep(10);
  }
  ov_clear(&priv[0].vf);
  priv[0].fd = -1;
  priv[0].pcm_indx = 0;

  return 0;
}

void StopOgg()
{
  ASND_StopVoice(0);
  ogg_thread_running = 0;

  if(h_oggplayer != LWP_THREAD_NULL)
  {
    if(oggplayer_queue != LWP_TQUEUE_NULL)
      LWP_ThreadSignal(oggplayer_queue);
    LWP_JoinThread(h_oggplayer, NULL);
    h_oggplayer = LWP_THREAD_NULL;
  }
  if(oggplayer_queue != LWP_TQUEUE_NULL)
  {
    LWP_CloseQueue(oggplayer_queue);
    oggplayer_queue = LWP_TQUEUE_NULL;
  }
}

int PlayOgg(char * buf, int buflen, int time_pos, int mode)
{
  StopOgg();

  private_ogg.fd = mem_open(buf, buflen);
  
  if (private_ogg.fd < 0)
  {
    private_ogg.fd = -1;
    return -1;
  }

  private_ogg.mode = mode;
  private_ogg.eof = 0;
  private_ogg.volume = 127;
  private_ogg.flag = 0;
  private_ogg.seek_time = -1;

  if (time_pos > 0)
    private_ogg.seek_time = time_pos;

  if (ov_open_callbacks((void *) &private_ogg.fd, &private_ogg.vf, NULL, 0, callbacks) < 0)
  {
    mem_close(private_ogg.fd); /* mem_close() can too close files from devices */
    private_ogg.fd = -1;
    ogg_thread_running = 0;
    return -1;
  }

  if (LWP_CreateThread(&h_oggplayer, (void *) ogg_player_thread,
      &private_ogg, oggplayer_stack, STACKSIZE, 80) == -1)
  {
    ogg_thread_running = 0;
    ov_clear(&private_ogg.vf);
    private_ogg.fd = -1;
    return -1;
  }
  return 0;
}

void PauseOgg(int pause)
{
  if (pause)
  {
    private_ogg.flag |= 128;
  }
  else
  {
    if (private_ogg.flag & 128)
    {
      private_ogg.flag |= 64;
      private_ogg.flag &= ~128;
      if (ogg_thread_running > 0)
      {
        LWP_ThreadSignal(oggplayer_queue);
      }
    }
  }
}

int StatusOgg()
{
  if (ogg_thread_running == 0)
    return -1; /* Error */
  else if (private_ogg.eof)
    return 255; /* EOF */
  else if (private_ogg.flag & 128)
    return 2; /* paused */
  else
    return 1; /* running */
}

void SetVolumeOgg(int volume)
{
  private_ogg.volume = volume;
  ASND_ChangeVolumeVoice(0, volume, volume);
}

s32 GetTimeOgg()
{
  int ret;
  if (ogg_thread_running == 0 || private_ogg.fd < 0)
    return 0;
  ret = ((s32) ov_time_tell(&private_ogg.vf));
  if (ret < 0)
    ret = 0;

  return ret;
}

void SetTimeOgg(s32 time_pos)
{
  if (time_pos >= 0)
    private_ogg.seek_time = time_pos;
}

