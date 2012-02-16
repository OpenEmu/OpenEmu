
#include "osd.h"

int seal_sample_rate =   44100;
int seal_sound_card  =   -1;

HAC hVoice[NUMVOICES];
LPAUDIOWAVE lpWave[NUMVOICES];
AUDIOINFO info;
AUDIOCAPS caps;
int c[MAX_STREAM_CHANNELS];
int nominal_sample_rate;
static int attenuation = 0;
static int master_volume = 256;


void osd_update_audio(void)
{
    if (seal_sample_rate == 0) return;
    AUpdateAudio();
}


/* attenuation in dB */
void osd_set_mastervolume(int _attenuation)
{
    float volume;

    attenuation = _attenuation;

    volume = 256.0; /* range is 0-256 */
    while (_attenuation++ < 0)
        volume /= 1.122018454;  /* = (10 ^ (1/20)) = 1dB */

    master_volume = volume;

    ASetAudioMixerValue(AUDIO_MIXER_MASTER_VOLUME,master_volume);
}


int msdos_init_sound(int *rate, int card)
{
    int i;

    seal_sample_rate = *rate;
    seal_sound_card  = card;

    if (AInitialize() != AUDIO_ERROR_NONE)
        return 1;

    /* Ask the user if no sound card was chosen */
    if (seal_sound_card == -1)
    {
        unsigned int k;

        printf("\n SELECT YOUR AUDIO DEVICE :\n\n"
               " AWE32/64 playback requires onboard DRAM,\n"
               " Sound Blaster playback is the most compatible & better for emulation\n\n");

        for (k = 0;k < AGetAudioNumDevs();k++)
        {
            if (AGetAudioDevCaps(k,&caps) == AUDIO_ERROR_NONE)
                printf("  %2d. %s\n",k,caps.szProductName);
        }
        printf("\n");

        if (k < 10)
        {
            i = getch();
            seal_sound_card = i - '0';
        }
        else
            scanf("%d",&seal_sound_card);
    }

    /* initialize SEAL audio library */
    if (seal_sound_card == 0)     /* silence */
    {
        /* update the Machine structure to show that sound is disabled */
        seal_sample_rate = 0;
        exit(0);
        return 0;
    }

    /* open audio device */
    /*                              info.nDeviceId = AUDIO_DEVICE_MAPPER;*/
    info.nDeviceId = seal_sound_card;
    /* always use 16 bit mixing if possible - better quality and same speed of 8 bit */
    info.wFormat = AUDIO_FORMAT_16BITS | AUDIO_FORMAT_STEREO | AUDIO_FORMAT_RAW_SAMPLE;

    info.nSampleRate = seal_sample_rate;
    if (AOpenAudio(&info) != AUDIO_ERROR_NONE)
    {
        return (1);
    }

    AGetAudioDevCaps(info.nDeviceId,&caps);
    printf("Using `%s' at %d-bit %s %u Hz\n",
			caps.szProductName,
			info.wFormat & AUDIO_FORMAT_16BITS ? 16 : 8,
			info.wFormat & AUDIO_FORMAT_STEREO ? "stereo" : "mono",
			info.nSampleRate);

    /* open and allocate voices, allocate waveforms */
    if (AOpenVoices(NUMVOICES) != AUDIO_ERROR_NONE)
    {
        printf("voices initialization failed\n");
        return 1;
    }

    for (i = 0; i < NUMVOICES; i++)
    {
        if (ACreateAudioVoice(&hVoice[i]) != AUDIO_ERROR_NONE)
        {
            printf("voice #%d creation failed\n",i);
            return 1;
        }

        ASetVoicePanning(hVoice[i],128);

        lpWave[i] = 0;
    }

    /* update the Machine structure to reflect the actual sample rate */
    *rate = seal_sample_rate = info.nSampleRate;

    {
        uclock_t a,b;
        LONG start,end;


        if ((lpWave[0] = (LPAUDIOWAVE)malloc(sizeof(AUDIOWAVE))) == 0)
            return 1;

        lpWave[0]->wFormat = AUDIO_FORMAT_8BITS | AUDIO_FORMAT_MONO;
        lpWave[0]->nSampleRate = seal_sample_rate;
        lpWave[0]->dwLength = 3*seal_sample_rate;
        lpWave[0]->dwLoopStart = 0;
        lpWave[0]->dwLoopEnd = 3*seal_sample_rate;
        if (ACreateAudioData(lpWave[0]) != AUDIO_ERROR_NONE)
        {
            free(lpWave[0]);
            lpWave[0] = 0;

            return 1;
        }

        memset(lpWave[0]->lpData,0,3*seal_sample_rate);
        /* upload the data to the audio DRAM local memory */
        AWriteAudioData(lpWave[0],0,3*seal_sample_rate);
        APrimeVoice(hVoice[0],lpWave[0]);
        ASetVoiceFrequency(hVoice[0],seal_sample_rate);
        ASetVoiceVolume(hVoice[0],0);
        AStartVoice(hVoice[0]);

        a = uclock();
        /* wait some time to let everything stabilize */
        do
        {
            osd_update_audio();
            b = uclock();
        } while (b-a < UCLOCKS_PER_SEC/10);

        a = uclock();
        AGetVoicePosition(hVoice[0],&start);
        do
        {
            osd_update_audio();
            b = uclock();
        } while (b-a < UCLOCKS_PER_SEC);
        AGetVoicePosition(hVoice[0],&end);

        nominal_sample_rate = seal_sample_rate;
        seal_sample_rate = end - start;

        AStopVoice(hVoice[0]);
        ADestroyAudioData(lpWave[0]);
        free(lpWave[0]);
        lpWave[0] = 0;
    }

    osd_set_mastervolume(0);    /* start at maximum volume */

    return 0;
}


void msdos_shutdown_sound(void)
{
    if (seal_sample_rate != 0)
    {
        int n;

        /* stop and release voices */
        for (n = 0; n < NUMVOICES; n++)
        {
            AStopVoice(hVoice[n]);
            ADestroyAudioVoice(hVoice[n]);
            if (lpWave[n])
            {
                ADestroyAudioData(lpWave[n]);
                free(lpWave[n]);
                lpWave[n] = 0;
            }
        }
        ACloseVoices();
        ACloseAudio();
    }
}


void playstreamedsample(int channel,signed char *data,int len,int freq,int volume,int pan,int bits)
{
    static int playing[NUMVOICES];
    static int c[NUMVOICES];

    /* backwards compatibility with old 0-255 volume range */
    if (volume > 100) volume = volume * 25 / 255;

    if (seal_sample_rate == 0 || channel >= NUMVOICES) return;

    if (!playing[channel])
    {
        if (lpWave[channel])
        {
            AStopVoice(hVoice[channel]);
            ADestroyAudioData(lpWave[channel]);
            free(lpWave[channel]);
            lpWave[channel] = 0;
        }

        if ((lpWave[channel] = (LPAUDIOWAVE)malloc(sizeof(AUDIOWAVE))) == 0)
            return;

        lpWave[channel]->wFormat = (bits == 8 ? AUDIO_FORMAT_8BITS : AUDIO_FORMAT_16BITS)
                | AUDIO_FORMAT_MONO | AUDIO_FORMAT_LOOP;
        lpWave[channel]->nSampleRate = nominal_sample_rate;
        lpWave[channel]->dwLength = 3*len;
        lpWave[channel]->dwLoopStart = 0;
        lpWave[channel]->dwLoopEnd = 3*len;
        if (ACreateAudioData(lpWave[channel]) != AUDIO_ERROR_NONE)
        {
            free(lpWave[channel]);
            lpWave[channel] = 0;

            return;
        }

        memset(lpWave[channel]->lpData,0,3*len);
        memcpy(lpWave[channel]->lpData,data,len);
        /* upload the data to the audio DRAM local memory */
        AWriteAudioData(lpWave[channel],0,3*len);
        APrimeVoice(hVoice[channel],lpWave[channel]);
    /* need to cast to double because freq*nominal_sample_rate can exceed the size of an int */
        ASetVoiceFrequency(hVoice[channel],(double)freq*nominal_sample_rate/seal_sample_rate);
        AStartVoice(hVoice[channel]);
        playing[channel] = 1;
        c[channel] = 1;
    }
    else
    {
        LONG pos;

        for(;;)
        {
                AGetVoicePosition(hVoice[channel],&pos);
                if (c[channel] == 0 && pos >= len) break;
                if (c[channel] == 1 && (pos < len || pos >= 2*len)) break;
                if (c[channel] == 2 && pos < 2*len) break;
                osd_update_audio();
        }

        memcpy(&lpWave[channel]->lpData[len * c[channel]],data,len);
        AWriteAudioData(lpWave[channel],len*c[channel],len);
        c[channel]++;
        if (c[channel] == 3) c[channel] = 0;
    }


    ASetVoiceVolume(hVoice[channel],volume * 64 / 100);
    ASetVoicePanning(hVoice[channel],(pan + 100) * 255 / 200);
}

void osd_play_streamed_sample_16(int channel,signed short *data,int len,int freq,int volume,int pan)
{
    playstreamedsample(channel,(signed char *)data,len,freq,volume,pan,16);
}

