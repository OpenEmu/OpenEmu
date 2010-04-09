/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2007, 2008, 2009 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif

#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "CrabEmu.h"
#include "sound.h"

#define SOUND_BUFFERS   4

/* BUFFER_LEN must be a multiple of 2048 */
#define BUFFER_LEN 32768

static ALCdevice *device = NULL;
static ALCcontext *context = NULL;
static ALuint source;
static ALuint bufs[SOUND_BUFFERS];

static uint8 buffer[BUFFER_LEN];
static uint32 read_pos = 0;
static uint32 write_pos = 16384;
static int paused = 0;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static int thd_done = 0;
static pthread_t thd;

void *sound_update_thd(void *ptr __UNUSED__)    {
    ALint proc;
    ALuint buf;
    uint8 data[2048];

    while(!thd_done)    {
        /* See if the stream needs updating yet. */
        alGetSourcei(source, AL_BUFFERS_PROCESSED, &proc);

        if(alGetError() != AL_NO_ERROR) {
            continue;
        }

        pthread_mutex_lock(&mutex);

        /* Go through each buffer that needs more data. */
        while(proc--)   {
            /* Unqueue the old buffer, so that it can be filled again. */
            alSourceUnqueueBuffers(source, 1, &buf);

            if(alGetError() != AL_NO_ERROR) {
                continue;
            }

            if(paused || (read_pos + 2048 > write_pos &&
                          write_pos > read_pos))    {
                memset(data, 0, 2048);
            }
            else    {
                memcpy(data, buffer + read_pos, 2048);
                read_pos += 2048;

                if(read_pos == BUFFER_LEN)
                    read_pos = 0;
            }

            alBufferData(buf, AL_FORMAT_STEREO16, data, 2048, 44100);
            alSourceQueueBuffers(source, 1, &buf);
        }

        pthread_mutex_unlock(&mutex);

        /* Sleep for 5ms. */
        usleep(5 * 1000);
    }

    return NULL;
}

void sound_update_buffer(int16 *buf, int length) {
    pthread_mutex_lock(&mutex);

    if(write_pos + length > BUFFER_LEN) {
        int l1, l2;

        l1 = BUFFER_LEN - write_pos;
        l2 = length - l1;

        memcpy(buffer + write_pos, buf, l1);
        memcpy(buffer, buf + l1, l2);

        write_pos = l2;
    }
    else {
        memcpy(buffer + write_pos, buf, length);
        write_pos += length;
    }

    pthread_mutex_unlock(&mutex);
}

int sound_init(void)    {
    int rv = 0, i;

    memset(buffer, 0, BUFFER_LEN);

    /* Attempt to grab the preferred device from OpenAL. */
    device = alcOpenDevice(NULL);

    if(!device) {
        rv = -1;
        goto err1;
    }

    context = alcCreateContext(device, NULL);

    if(!context)    {
        rv = -2;
        goto err2;
    }

    alcMakeContextCurrent(context);

    /* Clear any error states. */
    alGetError();

    /* Create our sound buffers. */
    alGenBuffers(SOUND_BUFFERS, bufs);

    if(alGetError() != AL_NO_ERROR) {
        rv = -3;
        goto err3;
    }

    /* Create the source for the stream. */
    alGenSources(1, &source);

    if(alGetError() != AL_NO_ERROR) {
        rv = -4;
        goto err4;
    }

    /* Set up the source for basic playback. */
    alSource3f(source, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_POSITION, 0.0f, 0.0f, 0.0f);
    alSource3f(source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    alSourcef(source, AL_ROLLOFF_FACTOR, 0.0f);
    alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);

    for(i = 0; i < SOUND_BUFFERS; ++i)  {
        /* Fill the buffer with empty sound. */
        alBufferData(bufs[i], AL_FORMAT_STEREO16, buffer + i * 2048, 2048,
                     44100);
        alSourceQueueBuffers(source, 1, bufs + i);
    }

    /* Start the sound playback. */
    alSourcePlay(source);

    /* Start the update thread. */
    pthread_create(&thd, NULL, &sound_update_thd, NULL);

    return 0;

    /* Error conditions. Errors cause cascading deinitialization, so hence this
       chain of labels. */
err4:
    alDeleteBuffers(SOUND_BUFFERS, bufs);

err3:
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);

err2:
    alcCloseDevice(device);

err1:
    context = NULL;
    device = NULL;
    return rv;
}

void sound_shutdown(void)   {
    /* Stop our update thread. */
    thd_done = 1;
    pthread_join(thd, NULL);

    /* Stop playback. */
    alSourceStop(source);

    /* Clean up our buffers and such. */
    alDeleteSources(1, &source);
    alDeleteBuffers(SOUND_BUFFERS, bufs);

    /* Release our context and close the sound device. */
    alcMakeContextCurrent(NULL);
    alcDestroyContext(context);
    alcCloseDevice(device);

    context = NULL;
    device = NULL;
}

void sound_reset_buffer(void)   {
    memset(buffer, 0, BUFFER_LEN);
    read_pos = 0;
    write_pos = 16384;
}

void sound_pause(void)  {
    paused = 1;
}

void sound_unpause(void)    {
    paused = 0;
}
