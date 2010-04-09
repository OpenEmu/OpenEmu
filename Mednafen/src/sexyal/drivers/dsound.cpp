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

#include <windows.h>
#include <windowsx.h>
#include <stdio.h>

#undef  WINNT
#define NONAMELESSUNION

#define DIRECTSOUND_VERSION  0x0300

#include <dsound.h>

typedef struct 
{
	LPDIRECTSOUND ppDS;		/* DirectSound interface object. */
	LPDIRECTSOUNDBUFFER ppbuf;	/* Primary buffer. */
	LPDIRECTSOUNDBUFFER ppbufsec;	/* Secondary buffer. */
	LPDIRECTSOUNDBUFFER ppbufw;	/* Buffer to do writes to. */
	WAVEFORMATEX wf;		/* Format of the primary and secondary buffers. */
	long DSBufferSize;		/* The size of the buffer that we can write to, in bytes. */

	long BufHowMuch;		/* How many bytes we should try to buffer. */
	DWORD ToWritePos;		/* Position which the next write to the buffer
					   should write to.
					*/
} DSFobby;


static int Close(SexyAL_device *device);
static int RawCanWrite(SexyAL_device *device, uint32_t *can_write);
static int RawWrite(SexyAL_device *device, const void *data, uint32_t len);

static int CheckStatus(DSFobby *tmp)
{
 DWORD status = 0;

 if(IDirectSoundBuffer_GetStatus(tmp->ppbufw, &status) != DS_OK)
  return(0);

 if(status & DSBSTATUS_BUFFERLOST)
  IDirectSoundBuffer_Restore(tmp->ppbufw);

 if(!(status&DSBSTATUS_PLAYING))
 {
  tmp->ToWritePos = 0;
  IDirectSoundBuffer_SetCurrentPosition(tmp->ppbufsec, 0);
  IDirectSoundBuffer_SetFormat(tmp->ppbufw, &tmp->wf);
  IDirectSoundBuffer_Play(tmp->ppbufw, 0, 0, DSBPLAY_LOOPING);
 }

 return(1);
}

static int Pause(SexyAL_device *device, int state)
{
 return(0);
}



SexyAL_device *SexyALI_DSound_Open(const char *id, SexyAL_format *format, SexyAL_buffering *buffering)
{
 SexyAL_device *dev;
 DSFobby *fobby;

 DSBUFFERDESC DSBufferDesc;
 DSCAPS dscaps;
 //DSBCAPS dsbcaps;

 dev = (SexyAL_device *)malloc(sizeof(SexyAL_device));
 fobby = (DSFobby *)malloc(sizeof(DSFobby));
 memset(fobby,0,sizeof(DSFobby));

 memset(&fobby->wf,0,sizeof(WAVEFORMATEX));
 fobby->wf.wFormatTag = WAVE_FORMAT_PCM;
 fobby->wf.nChannels = format->channels;
 fobby->wf.nSamplesPerSec = format->rate;

 if(DirectSoundCreate(0,&fobby->ppDS,0) != DS_OK)
 {
  free(dev);
  free(fobby);
  return(0);
 }

 {
  //HWND hWnd = GetForegroundWindow();    // Ugly.
  //if(!hWnd)
  //{ hWnd=GetDesktopWindow(); exit(1); }
  HWND hWnd;
  hWnd = GetDesktopWindow();
  IDirectSound_SetCooperativeLevel(fobby->ppDS, hWnd, DSSCL_PRIORITY);
 }
 memset(&dscaps,0x00,sizeof(dscaps));
 dscaps.dwSize=sizeof(dscaps);
 IDirectSound_GetCaps(fobby->ppDS,&dscaps);
 IDirectSound_Compact(fobby->ppDS);

 /* Create primary buffer */
 memset(&DSBufferDesc,0x00,sizeof(DSBUFFERDESC));
 DSBufferDesc.dwSize=sizeof(DSBufferDesc);
 DSBufferDesc.dwFlags=DSBCAPS_PRIMARYBUFFER;

 if(IDirectSound_CreateSoundBuffer(fobby->ppDS,&DSBufferDesc,&fobby->ppbuf,0) != DS_OK)
 {
  IDirectSound_Release(fobby->ppDS);
  free(dev);
  free(fobby);
  return(0);
 }

 /* Set primary buffer format. */
 if(format->sampformat == SEXYAL_FMT_PCMU8)
  fobby->wf.wBitsPerSample=8;
 else // if(format->sampformat == SEXYAL_FMT_PCMS16)
 {
  fobby->wf.wBitsPerSample=16;
  format->sampformat=SEXYAL_FMT_PCMS16;
 }

 fobby->wf.nBlockAlign = fobby->wf.nChannels * (fobby->wf.wBitsPerSample / 8);
 fobby->wf.nAvgBytesPerSec=fobby->wf.nSamplesPerSec*fobby->wf.nBlockAlign;
 if(IDirectSoundBuffer_SetFormat(fobby->ppbuf,&fobby->wf) != DS_OK)
 {
  IDirectSound_Release(fobby->ppbuf);
  IDirectSound_Release(fobby->ppDS);
  free(dev);
  free(fobby);
  return(0);
 }

 /* Create secondary sound buffer */
 IDirectSoundBuffer_GetFormat(fobby->ppbuf,&fobby->wf,sizeof(WAVEFORMATEX),0);
 memset(&DSBufferDesc,0x00,sizeof(DSBUFFERDESC));
 DSBufferDesc.dwSize=sizeof(DSBufferDesc);
 DSBufferDesc.dwFlags=DSBCAPS_GETCURRENTPOSITION2;
 DSBufferDesc.dwFlags|=DSBCAPS_GLOBALFOCUS;
 DSBufferDesc.dwBufferBytes=65536;
 DSBufferDesc.lpwfxFormat=&fobby->wf;
 if(IDirectSound_CreateSoundBuffer(fobby->ppDS, &DSBufferDesc, &fobby->ppbufsec, 0) != DS_OK)
 {
  IDirectSound_Release(fobby->ppbuf);
  IDirectSound_Release(fobby->ppDS);
  free(dev);
  free(fobby);
  return(0);
 }

 fobby->DSBufferSize=65536;
 IDirectSoundBuffer_SetCurrentPosition(fobby->ppbufsec,0);
 fobby->ppbufw=fobby->ppbufsec;

 memcpy(&dev->format,format,sizeof(SexyAL_format));

 if(!buffering->ms)
  buffering->ms=53;

 buffering->totalsize=(int64_t)format->rate*buffering->ms/1000;
 fobby->BufHowMuch=buffering->totalsize* format->channels * (format->sampformat>>4);

 buffering->latency = buffering->totalsize; // TODO:  Add estimated WaveOut latency when using an emulated DirectSound device.

 //printf("%d\n",fobby->BufHowMuch);
 //fflush(stdout);

 dev->private_data=fobby;
 timeBeginPeriod(1);
 
 dev->RawWrite = RawWrite;
 dev->RawCanWrite = RawCanWrite;
 dev->RawClose = Close;
 dev->Pause = Pause;

 return(dev);
}

static int RawCanWrite(SexyAL_device *device, uint32_t *can_write)
{
 DSFobby *tmp = (DSFobby *)device->private_data;
 DWORD CurWritePos, CurPlayPos = 0;

 if(!CheckStatus(tmp))
  return(0);

 CurWritePos=0;

 if(IDirectSoundBuffer_GetCurrentPosition(tmp->ppbufw,&CurPlayPos,&CurWritePos)==DS_OK)
 {
   //MDFN_DispMessage("%d",CurWritePos-CurPlayPos);
 }
 CurWritePos=(CurPlayPos+tmp->BufHowMuch)%tmp->DSBufferSize;

 /*  If the current write pos is >= half the buffer size less than the to write pos,
     assume DirectSound has wrapped around.
 */

 if(((int32_t)tmp->ToWritePos-(int32_t)CurWritePos) >= (tmp->DSBufferSize/2))
 {
  CurWritePos += tmp->DSBufferSize;
  //printf("Fixit: %d,%d,%d\n",tmp->ToWritePos,CurWritePos,CurWritePos-tmp->DSBufferSize);
 }

 if(tmp->ToWritePos < CurWritePos)
 {
  int32_t howmuch = (int32_t)CurWritePos - (int32_t)tmp->ToWritePos;

  if(howmuch > tmp->BufHowMuch)      /* Oopsie.  Severe buffer overflow... */
  {
   tmp->ToWritePos = CurWritePos % tmp->DSBufferSize;
  }

  *can_write = CurWritePos - tmp->ToWritePos;
 }
 else
  *can_write = 0;

 return(1);
}

static int RawWrite(SexyAL_device *device, const void *data, uint32_t len)
{
 DSFobby *tmp = (DSFobby *)device->private_data;

 //printf("Pre: %d\n",SexyALI_DSound_RawCanWrite(device));
 //fflush(stdout);

 if(!CheckStatus(tmp))
  return(0);

 /* In this block, we write as much data as we can, then we write
    the rest of it in >=1ms chunks.
 */
 while(len)
 {
  VOID *LockPtr[2]={0,0};
  DWORD LockLen[2]={0,0};
  uint32_t curlen;
  int rcw_rv;

  while((rcw_rv = RawCanWrite(device, &curlen)) && !curlen)
   Sleep(1);

  // If RawCanWrite() failed, RawWrite must fail~
  if(!rcw_rv)
   return(0);

  if(curlen > len)
   curlen = len;

  if(IDirectSoundBuffer_Lock(tmp->ppbufw, tmp->ToWritePos, curlen, &LockPtr[0], &LockLen[0], &LockPtr[1], &LockLen[1], 0) != DS_OK)
  {
   return(0);
  }

  if(LockPtr[1] != 0 && LockPtr[1] != LockPtr[0])
  {
   memcpy(LockPtr[0], data, LockLen[0]);
   memcpy(LockPtr[1], (uint8_t *)data + LockLen[0], len - LockLen[0]);
  }
  else if(LockPtr[0])
  {
   memcpy(LockPtr[0], data, curlen);
  }

  IDirectSoundBuffer_Unlock(tmp->ppbufw, LockPtr[0], LockLen[0], LockPtr[1], LockLen[1]);

  tmp->ToWritePos = (tmp->ToWritePos + curlen) % tmp->DSBufferSize;

  len -= curlen;
  data = (uint8_t *)data + curlen;

  if(len)
   Sleep(1);
 } // end while(len) loop


 return(1);
}



static int Close(SexyAL_device *device)
{
 if(device)
 {
  if(device->private_data)
  {
   DSFobby *tmp = (DSFobby *)device->private_data;
   if(tmp->ppbufsec)
   {
    IDirectSoundBuffer_Stop(tmp->ppbufsec);
    IDirectSoundBuffer_Release(tmp->ppbufsec);
   }
   if(tmp->ppbuf)
   {
    IDirectSoundBuffer_Stop(tmp->ppbuf);
    IDirectSoundBuffer_Release(tmp->ppbuf);
   }
   if(tmp->ppDS)
   {
    IDirectSound_Release(tmp->ppDS);
   }
   free(device->private_data);
  }
  free(device);
  timeEndPeriod(1);
  return(1);
 }
 return(0);
}

