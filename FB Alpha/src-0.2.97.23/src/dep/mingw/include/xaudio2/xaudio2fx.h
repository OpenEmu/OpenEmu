/*-------------------------------------------------------------------------
  Moddified by CaptainCPS for use with MinGW / GCC 4.6.1
  
  Thanks to PortAudio for their 'sal.h' compatible with MinGW !
-------------------------------------------------------------------------*/

/**************************************************************************
 *
 * Copyright (c) Microsoft Corporation.  All rights reserved.
 *
 * File:    xaudio2fx.h
 * Content: Declarations for the audio effects included with XAudio2.
 *
 **************************************************************************/

#ifndef __XAUDIO2FX_INCLUDED__
#define __XAUDIO2FX_INCLUDED__

#if __GNUC__ >=3
#pragma GCC system_header
#endif

/**************************************************************************
 *
 * XAudio2 effect class IDs.
 *
 **************************************************************************/

//#include "comdecl.h"        // For DEFINE_CLSID and DEFINE_IID

// XAudio 2.0 (March 2008 SDK)
//DEFINE_CLSID(AudioVolumeMeter, C0C56F46, 29B1, 44E9, 99, 39, A3, 2C, E8, 68, 67, E2);
//DEFINE_CLSID(AudioVolumeMeter_Debug, C0C56F46, 29B1, 44E9, 99, 39, A3, 2C, E8, 68, 67, DB);
//DEFINE_CLSID(AudioReverb, 6F6EA3A9, 2CF5, 41CF, 91, C1, 21, 70, B1, 54, 00, 63);
//DEFINE_CLSID(AudioReverb_Debug, 6F6EA3A9, 2CF5, 41CF, 91, C1, 21, 70, B1, 54, 00, DB);

// XAudio 2.1 (June 2008 SDK)
//DEFINE_CLSID(AudioVolumeMeter, c1e3f122, a2ea, 442c, 85, 4f, 20, d9, 8f, 83, 57, a1);
//DEFINE_CLSID(AudioVolumeMeter_Debug, 6d97a461, b02d, 48ae, b5, 43, 82, bc, 35, fd, fa, e2);
//DEFINE_CLSID(AudioReverb, f4769300, b949, 4df9, b3, 33, 00, d3, 39, 32, e9, a6);
//DEFINE_CLSID(AudioReverb_Debug, aea2cabc, 8c7c, 46aa, ba, 44, 0e, 6d, 75, 88, a1, f2);

// XAudio 2.2 (August 2008 SDK)
//DEFINE_CLSID(AudioVolumeMeter, f5ca7b34, 8055, 42c0, b8, 36, 21, 61, 29, eb, 7e, 30);
//DEFINE_CLSID(AudioVolumeMeter_Debug, f796f5f7, 6059, 4a9f, 98, 2d, 61, ee, c2, ed, 67, ca);
//DEFINE_CLSID(AudioReverb, 629cf0de, 3ecc, 41e7, 99, 26, f7, e4, 3e, eb, ec, 51);
//DEFINE_CLSID(AudioReverb_Debug, 4aae4299, 3260, 46d4, 97, cc, 6c, c7, 60, c8, 53, 29);

// XAudio 2.3 (November 2008 SDK)
//DEFINE_CLSID(AudioVolumeMeter, e180344b, ac83, 4483, 95, 9e, 18, a5, c5, 6a, 5e, 19);
//DEFINE_CLSID(AudioVolumeMeter_Debug, 922a0a56, 7d13, 40ae, a4, 81, 3c, 6c, 60, f1, 14, 01);
//DEFINE_CLSID(AudioReverb, 9cab402c, 1d37, 44b4, 88, 6d, fa, 4f, 36, 17, 0a, 4c);
//DEFINE_CLSID(AudioReverb_Debug, eadda998, 3be6, 4505, 84, be, ea, 06, 36, 5d, b9, 6b);

// XAudio 2.4 (March 2009 SDK)
//DEFINE_CLSID(AudioVolumeMeter, c7338b95, 52b8, 4542, aa, 79, 42, eb, 01, 6c, 8c, 1c);
//DEFINE_CLSID(AudioVolumeMeter_Debug, 524bd872, 5c0b, 4217, bd, b8, 0a, 86, 81, 83, 0b, a5);
//DEFINE_CLSID(AudioReverb, 8bb7778b, 645b, 4475, 9a, 73, 1d, e3, 17, 0b, d3, af);
//DEFINE_CLSID(AudioReverb_Debug, da7738a2, cd0c, 4367, 9a, ac, d7, ea, d7, c6, 4f, 98);

// XAudio 2.5 (March 2009 SDK)
//DEFINE_CLSID(AudioVolumeMeter, 2139e6da, c341, 4774, 9a, c3, b4, e0, 26, 34, 7f, 64);
//DEFINE_CLSID(AudioVolumeMeter_Debug, a5cc4e13, ca00, 416b, a6, ee, 49, fe, e7, b5, 43, d0);
//DEFINE_CLSID(AudioReverb, d06df0d0, 8518, 441e, 82, 2f, 54, 51, d5, c5, 95, b8);
//DEFINE_CLSID(AudioReverb_Debug, 613604ec, 304c, 45ec, a4, ed, 7a, 1c, 61, 2e, 9e, 72);

// XAudio 2.6 (February 2010 SDK)
//DEFINE_CLSID(AudioVolumeMeter, e48c5a3f, 93ef, 43bb, a0, 92, 2c, 7c, eb, 94, 6f, 27);
//DEFINE_CLSID(AudioVolumeMeter_Debug, 9a9eaef7, a9e0, 4088, 9b, 1b, 9c, a0, 3a, 1a, ec, d4);
//DEFINE_CLSID(AudioReverb, cecec95a, d894, 491a, be, e3, 5e, 10, 6f, b5, 9f, 2d);
//DEFINE_CLSID(AudioReverb_Debug, 99a1c72e, 364c, 4c1b, 96, 23, fd, 5c, 8a, bd, 90, c7);

// XAudio 2.7 (June 2010 SDK)
DEFINE_GUID(CLSID_AudioVolumeMeter, 0xcac1105f, 0x619b, 0x4d04, 0x83, 0x1a, 0x44, 0xe1, 0xcb, 0xf1, 0x2d, 0x57);
DEFINE_GUID(CLSID_AudioVolumeMeter_Debug, 0x2d9a0f9c, 0xe67b, 0x4b24, 0xab, 0x44, 0x92, 0xb3, 0xe7, 0x70, 0xc0, 0x20);
DEFINE_GUID(CLSID_AudioReverb, 0x6a93130e, 0x1d53, 0x41d1, 0xa9, 0xcf, 0xe7, 0x58, 0x80, 0x0b, 0xb1, 0x79);
DEFINE_GUID(CLSID_AudioReverb_Debug, 0xc4f82dd4, 0xcb4e, 0x4ce1, 0x8b, 0xdb, 0xee, 0x32, 0xd4, 0x19, 0x82, 0x69);

// Ignore the rest of this header if only the GUID definitions were requested
#ifndef GUID_DEFS_ONLY

#ifdef _XBOX
    #include <xobjbase.h>   // Xbox COM declarations (IUnknown, etc)
#else
    #include <objbase.h>    // Windows COM declarations
#endif
#include <math.h>           // For log10()

// All structures defined in this file should use tight packing
#pragma pack(push, 1)


/**************************************************************************
 *
 * Effect creation functions.  On Windows, these are just inline functions
 * that call CoCreateInstance and Initialize; the XAUDIO2FX_DEBUG flag can
 * be used to select the debug version of the effects.  On Xbox, these map
 * to real functions included in xaudio2.lib, and the XAUDIO2FX_DEBUG flag
 * is ignored; the application must link with the debug library to use the
 * debug functionality.
 *
 **************************************************************************/

// Use default values for some parameters if building C++ code
#ifdef __cplusplus
    #define DEFAULT(x) =x
#else
    #define DEFAULT(x)
#endif

#define XAUDIO2FX_DEBUG 1   // To select the debug version of an effect

#ifdef _XBOX

    STDAPI CreateAudioVolumeMeter(__deref_out IUnknown** ppApo);
    STDAPI CreateAudioReverb(__deref_out IUnknown** ppApo);

    __inline HRESULT XAudio2CreateVolumeMeter(__deref_out IUnknown** ppApo, UINT32 /*Flags*/ DEFAULT(0))
    {
        return CreateAudioVolumeMeter(ppApo);
    }

    __inline HRESULT XAudio2CreateReverb(__deref_out IUnknown** ppApo, UINT32 /*Flags*/ DEFAULT(0))
    {
        return CreateAudioReverb(ppApo);
    }

#else // Windows

    __inline HRESULT XAudio2CreateVolumeMeter(__deref_out IUnknown** ppApo, UINT32 Flags DEFAULT(0))
    {
        #ifdef __cplusplus
            return CoCreateInstance((Flags & XAUDIO2FX_DEBUG) ? CLSID_AudioVolumeMeter_Debug
                                                              : CLSID_AudioVolumeMeter,
                                    NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)ppApo);
        #else
            return CoCreateInstance((Flags & XAUDIO2FX_DEBUG) ? &CLSID_AudioVolumeMeter_Debug
                                                              : &CLSID_AudioVolumeMeter,
                                    NULL, CLSCTX_INPROC_SERVER, &IID_IUnknown, (void**)ppApo);
        #endif
    }

    __inline HRESULT XAudio2CreateReverb(__deref_out IUnknown** ppApo, UINT32 Flags DEFAULT(0))
    {
        #ifdef __cplusplus
            return CoCreateInstance((Flags & XAUDIO2FX_DEBUG) ? CLSID_AudioReverb_Debug
                                                              : CLSID_AudioReverb,
                                    NULL, CLSCTX_INPROC_SERVER, IID_IUnknown, (void**)ppApo);
        #else
            return CoCreateInstance((Flags & XAUDIO2FX_DEBUG) ? &CLSID_AudioReverb_Debug
                                                              : &CLSID_AudioReverb,
                                    NULL, CLSCTX_INPROC_SERVER, &IID_IUnknown, (void**)ppApo);
        #endif
    }

#endif // #ifdef _XBOX



/**************************************************************************
 *
 * Volume meter parameters.
 * The volume meter supports FLOAT32 audio formats and must be used in-place.
 *
 **************************************************************************/

// XAUDIO2FX_VOLUMEMETER_LEVELS: Receives results from GetEffectParameters().
// The user is responsible for allocating pPeakLevels, pRMSLevels, and
// initializing ChannelCount accordingly.
// The volume meter does not support SetEffectParameters().
typedef struct XAUDIO2FX_VOLUMEMETER_LEVELS
{
    float* pPeakLevels;  // Peak levels table: receives maximum absolute level for each channel
                         // over a processing pass; may be NULL if pRMSLevls != NULL,
                         // otherwise must have at least ChannelCount elements.
    float* pRMSLevels;   // Root mean square levels table: receives RMS level for each channel
                         // over a processing pass; may be NULL if pPeakLevels != NULL,
                         // otherwise must have at least ChannelCount elements.
    UINT32 ChannelCount; // Number of channels being processed by the volume meter APO
} XAUDIO2FX_VOLUMEMETER_LEVELS;



/**************************************************************************
 *
 * Reverb parameters.
 * The reverb supports only FLOAT32 audio with the following channel
 * configurations:
 *     Input: Mono   Output: Mono
 *     Input: Mono   Output: 5.1
 *     Input: Stereo Output: Stereo
 *     Input: Stereo Output: 5.1
 * The framerate must be within [20000, 48000] Hz.
 *
 * When using mono input, delay filters associated with the right channel
 * are not executed.  In this case, parameters such as PositionRight and
 * PositionMatrixRight have no effect.  This also means the reverb uses
 * less CPU when hosted in a mono submix.
 *
 **************************************************************************/

#define XAUDIO2FX_REVERB_MIN_FRAMERATE 20000
#define XAUDIO2FX_REVERB_MAX_FRAMERATE 48000

// XAUDIO2FX_REVERB_PARAMETERS: Native parameter set for the reverb effect

typedef struct XAUDIO2FX_REVERB_PARAMETERS
{
    // ratio of wet (processed) signal to dry (original) signal
    float WetDryMix;            // [0, 100] (percentage)

    // Delay times
    UINT32 ReflectionsDelay;    // [0, 300] in ms
    BYTE ReverbDelay;           // [0, 85] in ms
    BYTE RearDelay;             // [0, 5] in ms

    // Indexed parameters
    BYTE PositionLeft;          // [0, 30] no units
    BYTE PositionRight;         // [0, 30] no units, ignored when configured to mono
    BYTE PositionMatrixLeft;    // [0, 30] no units
    BYTE PositionMatrixRight;   // [0, 30] no units, ignored when configured to mono
    BYTE EarlyDiffusion;        // [0, 15] no units
    BYTE LateDiffusion;         // [0, 15] no units
    BYTE LowEQGain;             // [0, 12] no units
    BYTE LowEQCutoff;           // [0, 9] no units
    BYTE HighEQGain;            // [0, 8] no units
    BYTE HighEQCutoff;          // [0, 14] no units

    // Direct parameters
    float RoomFilterFreq;       // [20, 20000] in Hz
    float RoomFilterMain;       // [-100, 0] in dB
    float RoomFilterHF;         // [-100, 0] in dB
    float ReflectionsGain;      // [-100, 20] in dB
    float ReverbGain;           // [-100, 20] in dB
    float DecayTime;            // [0.1, inf] in seconds
    float Density;              // [0, 100] (percentage)
    float RoomSize;             // [1, 100] in feet
} XAUDIO2FX_REVERB_PARAMETERS;


// Maximum, minimum and default values for the parameters above
#define XAUDIO2FX_REVERB_MIN_WET_DRY_MIX            0.0f
#define XAUDIO2FX_REVERB_MIN_REFLECTIONS_DELAY      0
#define XAUDIO2FX_REVERB_MIN_REVERB_DELAY           0
#define XAUDIO2FX_REVERB_MIN_REAR_DELAY             0
#define XAUDIO2FX_REVERB_MIN_POSITION               0
#define XAUDIO2FX_REVERB_MIN_DIFFUSION              0
#define XAUDIO2FX_REVERB_MIN_LOW_EQ_GAIN            0
#define XAUDIO2FX_REVERB_MIN_LOW_EQ_CUTOFF          0
#define XAUDIO2FX_REVERB_MIN_HIGH_EQ_GAIN           0
#define XAUDIO2FX_REVERB_MIN_HIGH_EQ_CUTOFF         0
#define XAUDIO2FX_REVERB_MIN_ROOM_FILTER_FREQ       20.0f
#define XAUDIO2FX_REVERB_MIN_ROOM_FILTER_MAIN       -100.0f
#define XAUDIO2FX_REVERB_MIN_ROOM_FILTER_HF         -100.0f
#define XAUDIO2FX_REVERB_MIN_REFLECTIONS_GAIN       -100.0f
#define XAUDIO2FX_REVERB_MIN_REVERB_GAIN            -100.0f
#define XAUDIO2FX_REVERB_MIN_DECAY_TIME             0.1f
#define XAUDIO2FX_REVERB_MIN_DENSITY                0.0f
#define XAUDIO2FX_REVERB_MIN_ROOM_SIZE              0.0f

#define XAUDIO2FX_REVERB_MAX_WET_DRY_MIX            100.0f
#define XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY      300
#define XAUDIO2FX_REVERB_MAX_REVERB_DELAY           85
#define XAUDIO2FX_REVERB_MAX_REAR_DELAY             5
#define XAUDIO2FX_REVERB_MAX_POSITION               30
#define XAUDIO2FX_REVERB_MAX_DIFFUSION              15
#define XAUDIO2FX_REVERB_MAX_LOW_EQ_GAIN            12
#define XAUDIO2FX_REVERB_MAX_LOW_EQ_CUTOFF          9
#define XAUDIO2FX_REVERB_MAX_HIGH_EQ_GAIN           8
#define XAUDIO2FX_REVERB_MAX_HIGH_EQ_CUTOFF         14
#define XAUDIO2FX_REVERB_MAX_ROOM_FILTER_FREQ       20000.0f
#define XAUDIO2FX_REVERB_MAX_ROOM_FILTER_MAIN       0.0f
#define XAUDIO2FX_REVERB_MAX_ROOM_FILTER_HF         0.0f
#define XAUDIO2FX_REVERB_MAX_REFLECTIONS_GAIN       20.0f
#define XAUDIO2FX_REVERB_MAX_REVERB_GAIN            20.0f
#define XAUDIO2FX_REVERB_MAX_DENSITY                100.0f
#define XAUDIO2FX_REVERB_MAX_ROOM_SIZE              100.0f

#define XAUDIO2FX_REVERB_DEFAULT_WET_DRY_MIX        100.0f
#define XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_DELAY  5
#define XAUDIO2FX_REVERB_DEFAULT_REVERB_DELAY       5
#define XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY         5
#define XAUDIO2FX_REVERB_DEFAULT_POSITION           6
#define XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX    27
#define XAUDIO2FX_REVERB_DEFAULT_EARLY_DIFFUSION    8
#define XAUDIO2FX_REVERB_DEFAULT_LATE_DIFFUSION     8
#define XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_GAIN        8
#define XAUDIO2FX_REVERB_DEFAULT_LOW_EQ_CUTOFF      4
#define XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_GAIN       8
#define XAUDIO2FX_REVERB_DEFAULT_HIGH_EQ_CUTOFF     4
#define XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_FREQ   5000.0f
#define XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_MAIN   0.0f
#define XAUDIO2FX_REVERB_DEFAULT_ROOM_FILTER_HF     0.0f
#define XAUDIO2FX_REVERB_DEFAULT_REFLECTIONS_GAIN   0.0f
#define XAUDIO2FX_REVERB_DEFAULT_REVERB_GAIN        0.0f
#define XAUDIO2FX_REVERB_DEFAULT_DECAY_TIME         1.0f
#define XAUDIO2FX_REVERB_DEFAULT_DENSITY            100.0f
#define XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE          100.0f


// XAUDIO2FX_REVERB_I3DL2_PARAMETERS: Parameter set compliant with the I3DL2 standard

typedef struct XAUDIO2FX_REVERB_I3DL2_PARAMETERS
{
    // ratio of wet (processed) signal to dry (original) signal
    float WetDryMix;            // [0, 100] (percentage)

    // Standard I3DL2 parameters
    INT32 Room;                 // [-10000, 0] in mB (hundredths of decibels)
    INT32 RoomHF;               // [-10000, 0] in mB (hundredths of decibels)
    float RoomRolloffFactor;    // [0.0, 10.0]
    float DecayTime;            // [0.1, 20.0] in seconds
    float DecayHFRatio;         // [0.1, 2.0]
    INT32 Reflections;          // [-10000, 1000] in mB (hundredths of decibels)
    float ReflectionsDelay;     // [0.0, 0.3] in seconds
    INT32 Reverb;               // [-10000, 2000] in mB (hundredths of decibels)
    float ReverbDelay;          // [0.0, 0.1] in seconds
    float Diffusion;            // [0.0, 100.0] (percentage)
    float Density;              // [0.0, 100.0] (percentage)
    float HFReference;          // [20.0, 20000.0] in Hz
} XAUDIO2FX_REVERB_I3DL2_PARAMETERS;


// ReverbConvertI3DL2ToNative: Utility function to map from I3DL2 to native parameters

__inline void ReverbConvertI3DL2ToNative
(
    __in const XAUDIO2FX_REVERB_I3DL2_PARAMETERS* pI3DL2,
    __out XAUDIO2FX_REVERB_PARAMETERS* pNative
)
{
    float reflectionsDelay;
    float reverbDelay;

    // RoomRolloffFactor is ignored

    // These parameters have no equivalent in I3DL2
    pNative->RearDelay = XAUDIO2FX_REVERB_DEFAULT_REAR_DELAY; // 5
    pNative->PositionLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION; // 6
    pNative->PositionRight = XAUDIO2FX_REVERB_DEFAULT_POSITION; // 6
    pNative->PositionMatrixLeft = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX; // 27
    pNative->PositionMatrixRight = XAUDIO2FX_REVERB_DEFAULT_POSITION_MATRIX; // 27
    pNative->RoomSize = XAUDIO2FX_REVERB_DEFAULT_ROOM_SIZE; // 100
    pNative->LowEQCutoff = 4;
    pNative->HighEQCutoff = 6;

    // The rest of the I3DL2 parameters map to the native property set
    pNative->RoomFilterMain = (float)pI3DL2->Room / 100.0f;
    pNative->RoomFilterHF = (float)pI3DL2->RoomHF / 100.0f;

    if (pI3DL2->DecayHFRatio >= 1.0f)
    {
        INT32 index = (INT32)(-4.0 * log10(pI3DL2->DecayHFRatio));
        if (index < -8) index = -8;
        pNative->LowEQGain = (BYTE)((index < 0) ? index + 8 : 8);
        pNative->HighEQGain = 8;
        pNative->DecayTime = pI3DL2->DecayTime * pI3DL2->DecayHFRatio;
    }
    else
    {
        INT32 index = (INT32)(4.0 * log10(pI3DL2->DecayHFRatio));
        if (index < -8) index = -8;
        pNative->LowEQGain = 8;
        pNative->HighEQGain = (BYTE)((index < 0) ? index + 8 : 8);
        pNative->DecayTime = pI3DL2->DecayTime;
    }

    reflectionsDelay = pI3DL2->ReflectionsDelay * 1000.0f;
    if (reflectionsDelay >= XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY) // 300
    {
        reflectionsDelay = (float)(XAUDIO2FX_REVERB_MAX_REFLECTIONS_DELAY - 1);
    }
    else if (reflectionsDelay <= 1)
    {
        reflectionsDelay = 1;
    }
    pNative->ReflectionsDelay = (UINT32)reflectionsDelay;

    reverbDelay = pI3DL2->ReverbDelay * 1000.0f;
    if (reverbDelay >= XAUDIO2FX_REVERB_MAX_REVERB_DELAY) // 85
    {
        reverbDelay = (float)(XAUDIO2FX_REVERB_MAX_REVERB_DELAY - 1);
    }
    pNative->ReverbDelay = (BYTE)reverbDelay;

    pNative->ReflectionsGain = pI3DL2->Reflections / 100.0f;
    pNative->ReverbGain = pI3DL2->Reverb / 100.0f;
    pNative->EarlyDiffusion = (BYTE)(15.0f * pI3DL2->Diffusion / 100.0f);
    pNative->LateDiffusion = pNative->EarlyDiffusion;
    pNative->Density = pI3DL2->Density;
    pNative->RoomFilterFreq = pI3DL2->HFReference;

    pNative->WetDryMix = pI3DL2->WetDryMix;
}


/**************************************************************************
 *
 * Standard I3DL2 reverb presets (100% wet).
 *
 **************************************************************************/

#define XAUDIO2FX_I3DL2_PRESET_DEFAULT         {100,-10000,    0,0.0f, 1.00f,0.50f,-10000,0.020f,-10000,0.040f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_GENERIC         {100, -1000, -100,0.0f, 1.49f,0.83f, -2602,0.007f,   200,0.011f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_PADDEDCELL      {100, -1000,-6000,0.0f, 0.17f,0.10f, -1204,0.001f,   207,0.002f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_ROOM            {100, -1000, -454,0.0f, 0.40f,0.83f, -1646,0.002f,    53,0.003f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_BATHROOM        {100, -1000,-1200,0.0f, 1.49f,0.54f,  -370,0.007f,  1030,0.011f,100.0f, 60.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_LIVINGROOM      {100, -1000,-6000,0.0f, 0.50f,0.10f, -1376,0.003f, -1104,0.004f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_STONEROOM       {100, -1000, -300,0.0f, 2.31f,0.64f,  -711,0.012f,    83,0.017f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_AUDITORIUM      {100, -1000, -476,0.0f, 4.32f,0.59f,  -789,0.020f,  -289,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_CONCERTHALL     {100, -1000, -500,0.0f, 3.92f,0.70f, -1230,0.020f,    -2,0.029f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_CAVE            {100, -1000,    0,0.0f, 2.91f,1.30f,  -602,0.015f,  -302,0.022f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_ARENA           {100, -1000, -698,0.0f, 7.24f,0.33f, -1166,0.020f,    16,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_HANGAR          {100, -1000,-1000,0.0f,10.05f,0.23f,  -602,0.020f,   198,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_CARPETEDHALLWAY {100, -1000,-4000,0.0f, 0.30f,0.10f, -1831,0.002f, -1630,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_HALLWAY         {100, -1000, -300,0.0f, 1.49f,0.59f, -1219,0.007f,   441,0.011f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_STONECORRIDOR   {100, -1000, -237,0.0f, 2.70f,0.79f, -1214,0.013f,   395,0.020f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_ALLEY           {100, -1000, -270,0.0f, 1.49f,0.86f, -1204,0.007f,    -4,0.011f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_FOREST          {100, -1000,-3300,0.0f, 1.49f,0.54f, -2560,0.162f,  -613,0.088f, 79.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_CITY            {100, -1000, -800,0.0f, 1.49f,0.67f, -2273,0.007f, -2217,0.011f, 50.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_MOUNTAINS       {100, -1000,-2500,0.0f, 1.49f,0.21f, -2780,0.300f, -2014,0.100f, 27.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_QUARRY          {100, -1000,-1000,0.0f, 1.49f,0.83f,-10000,0.061f,   500,0.025f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_PLAIN           {100, -1000,-2000,0.0f, 1.49f,0.50f, -2466,0.179f, -2514,0.100f, 21.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_PARKINGLOT      {100, -1000,    0,0.0f, 1.65f,1.50f, -1363,0.008f, -1153,0.012f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_SEWERPIPE       {100, -1000,-1000,0.0f, 2.81f,0.14f,   429,0.014f,   648,0.021f, 80.0f, 60.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_UNDERWATER      {100, -1000,-4000,0.0f, 1.49f,0.10f,  -449,0.007f,  1700,0.011f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_SMALLROOM       {100, -1000, -600,0.0f, 1.10f,0.83f,  -400,0.005f,   500,0.010f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_MEDIUMROOM      {100, -1000, -600,0.0f, 1.30f,0.83f, -1000,0.010f,  -200,0.020f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_LARGEROOM       {100, -1000, -600,0.0f, 1.50f,0.83f, -1600,0.020f, -1000,0.040f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_MEDIUMHALL      {100, -1000, -600,0.0f, 1.80f,0.70f, -1300,0.015f,  -800,0.030f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_LARGEHALL       {100, -1000, -600,0.0f, 1.80f,0.70f, -2000,0.030f, -1400,0.060f,100.0f,100.0f,5000.0f}
#define XAUDIO2FX_I3DL2_PRESET_PLATE           {100, -1000, -200,0.0f, 1.30f,0.90f,     0,0.002f,     0,0.010f,100.0f, 75.0f,5000.0f}


// Undo the #pragma pack(push, 1) at the top of this file
#pragma pack(pop)

#endif // #ifndef GUID_DEFS_ONLY
#endif // #ifndef __XAUDIO2FX_INCLUDED__
