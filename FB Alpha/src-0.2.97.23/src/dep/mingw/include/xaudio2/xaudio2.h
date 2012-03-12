/*-------------------------------------------------------------------------
  Moddified by CaptainCPS for use with MinGW / GCC 4.6.1
  
  Thanks to PortAudio for their 'sal.h' compatible with MinGW !
-------------------------------------------------------------------------*/

/**************************************************************************
 *
 * Copyright (c) Microsoft Corporation.  All rights reserved.
 *
 * File:    xaudio2.h
 * Content: Declarations for the XAudio2 game audio API.
 *
 **************************************************************************/

#ifndef __XAUDIO2_INCLUDED__
#define __XAUDIO2_INCLUDED__

#if __GNUC__ >=3
#pragma GCC system_header
#endif

/**************************************************************************
 *
 * XAudio2 COM object class and interface IDs.
 *
 **************************************************************************/

//#include <comdecl.h>        // For DEFINE_CLSID and DEFINE_IID

// XAudio 2.0 (March 2008 SDK)
//DEFINE_CLSID(XAudio2, fac23f48, 31f5, 45a8, b4, 9b, 52, 25, d6, 14, 01, aa);
//DEFINE_CLSID(XAudio2_Debug, fac23f48, 31f5, 45a8, b4, 9b, 52, 25, d6, 14, 01, db);

// XAudio 2.1 (June 2008 SDK)
//DEFINE_CLSID(XAudio2, e21a7345, eb21, 468e, be, 50, 80, 4d, b9, 7c, f7, 08);
//DEFINE_CLSID(XAudio2_Debug, f7a76c21, 53d4, 46bb, ac, 53, 8b, 45, 9c, ae, 46, bd);

// XAudio 2.2 (August 2008 SDK)
//DEFINE_CLSID(XAudio2, b802058a, 464a, 42db, bc, 10, b6, 50, d6, f2, 58, 6a);
//DEFINE_CLSID(XAudio2_Debug, 97dfb7e7, 5161, 4015, 87, a9, c7, 9e, 6a, 19, 52, cc);

// XAudio 2.3 (November 2008 SDK)
//DEFINE_CLSID(XAudio2, 4c5e637a, 16c7, 4de3, 9c, 46, 5e, d2, 21, 81, 96, 2d);
//DEFINE_CLSID(XAudio2_Debug, ef0aa05d, 8075, 4e5d, be, ad, 45, be, 0c, 3c, cb, b3);

// XAudio 2.4 (March 2009 SDK)
//DEFINE_CLSID(XAudio2, 03219e78, 5bc3, 44d1, b9, 2e, f6, 3d, 89, cc, 65, 26);
//DEFINE_CLSID(XAudio2_Debug, 4256535c, 1ea4, 4d4b, 8a, d5, f9, db, 76, 2e, ca, 9e);

// XAudio 2.5 (August 2009 SDK)
//DEFINE_CLSID(XAudio2, 4c9b6dde, 6809, 46e6, a2, 78, 9b, 6a, 97, 58, 86, 70);
//DEFINE_CLSID(XAudio2_Debug, 715bdd1a, aa82, 436b, b0, fa, 6a, ce, a3, 9b, d0, a1);

// XAudio 2.6 (February 2010 SDK)
//DEFINE_CLSID(XAudio2, 3eda9b49, 2085, 498b, 9b, b2, 39, a6, 77, 84, 93, de);
//DEFINE_CLSID(XAudio2_Debug, 47199894, 7cc2, 444d, 98, 73, ce, d2, 56, 2c, c6, 0e);

// XAudio 2.7 (June 2010 SDK)
DEFINE_GUID(CLSID_XAudio2, 0x5a508685, 0xa254, 0x4fba, 0x9b, 0x82, 0x9a, 0x24, 0xb0, 0x03, 0x06, 0xaf);
DEFINE_GUID(CLSID_XAudio2_Debug, 0xdb05ea35, 0x0329, 0x4d4b, 0xa5, 0x3a, 0x6d, 0xea, 0xd0, 0x3d, 0x38, 0x52);
DEFINE_GUID(IID_IXAudio2, 0x8bcf1f58, 0x9fe7, 0x4583, 0x8a, 0xc6, 0xe2, 0xad, 0xc4, 0x65, 0xc8, 0xbb);

// Ignore the rest of this header if only the GUID definitions were requested
#ifndef GUID_DEFS_ONLY

#ifdef _XBOX
    #include <xobjbase.h>   // Xbox COM declarations (IUnknown, etc)
#else
    #include <objbase.h>    // Windows COM declarations
#endif

#include <sal.h>            // Markers for documenting API semantics
#include <audiodefs.h>      // Basic audio data types and constants
#include <xma2defs.h>       // Data types and constants for XMA2 audio

// All structures defined in this file use tight field packing
#pragma pack(push, 1)


/**************************************************************************
 *
 * XAudio2 constants, flags and error codes.
 *
 **************************************************************************/

// Numeric boundary values
#define XAUDIO2_MAX_BUFFER_BYTES        0x80000000    // Maximum bytes allowed in a source buffer
#define XAUDIO2_MAX_QUEUED_BUFFERS      64            // Maximum buffers allowed in a voice queue
#define XAUDIO2_MAX_BUFFERS_SYSTEM      2             // Maximum buffers allowed for system threads (Xbox 360 only)
#define XAUDIO2_MAX_AUDIO_CHANNELS      64            // Maximum channels in an audio stream
#define XAUDIO2_MIN_SAMPLE_RATE         1000          // Minimum audio sample rate supported
#define XAUDIO2_MAX_SAMPLE_RATE         200000        // Maximum audio sample rate supported
#define XAUDIO2_MAX_VOLUME_LEVEL        16777216.0f   // Maximum acceptable volume level (2^24)
#define XAUDIO2_MIN_FREQ_RATIO          (1/1024.0f)   // Minimum SetFrequencyRatio argument
#define XAUDIO2_MAX_FREQ_RATIO          1024.0f       // Maximum MaxFrequencyRatio argument
#define XAUDIO2_DEFAULT_FREQ_RATIO      2.0f          // Default MaxFrequencyRatio argument
#define XAUDIO2_MAX_FILTER_ONEOVERQ     1.5f          // Maximum XAUDIO2_FILTER_PARAMETERS.OneOverQ
#define XAUDIO2_MAX_FILTER_FREQUENCY    1.0f          // Maximum XAUDIO2_FILTER_PARAMETERS.Frequency
#define XAUDIO2_MAX_LOOP_COUNT          254           // Maximum non-infinite XAUDIO2_BUFFER.LoopCount
#define XAUDIO2_MAX_INSTANCES           8             // Maximum simultaneous XAudio2 objects on Xbox 360

// For XMA voices on Xbox 360 there is an additional restriction on the MaxFrequencyRatio
// argument and the voice's sample rate: the product of these numbers cannot exceed 600000
// for one-channel voices or 300000 for voices with more than one channel.
#define XAUDIO2_MAX_RATIO_TIMES_RATE_XMA_MONO         600000
#define XAUDIO2_MAX_RATIO_TIMES_RATE_XMA_MULTICHANNEL 300000

// Numeric values with special meanings
#define XAUDIO2_COMMIT_NOW              0             // Used as an OperationSet argument
#define XAUDIO2_COMMIT_ALL              0             // Used in IXAudio2::CommitChanges
#define XAUDIO2_INVALID_OPSET           (UINT32)(-1)  // Not allowed for OperationSet arguments
#define XAUDIO2_NO_LOOP_REGION          0             // Used in XAUDIO2_BUFFER.LoopCount
#define XAUDIO2_LOOP_INFINITE           255           // Used in XAUDIO2_BUFFER.LoopCount
#define XAUDIO2_DEFAULT_CHANNELS        0             // Used in CreateMasteringVoice
#define XAUDIO2_DEFAULT_SAMPLERATE      0             // Used in CreateMasteringVoice

// Flags
#define XAUDIO2_DEBUG_ENGINE            0x0001        // Used in XAudio2Create on Windows only
#define XAUDIO2_VOICE_NOPITCH           0x0002        // Used in IXAudio2::CreateSourceVoice
#define XAUDIO2_VOICE_NOSRC             0x0004        // Used in IXAudio2::CreateSourceVoice
#define XAUDIO2_VOICE_USEFILTER         0x0008        // Used in IXAudio2::CreateSource/SubmixVoice
#define XAUDIO2_VOICE_MUSIC             0x0010        // Used in IXAudio2::CreateSourceVoice
#define XAUDIO2_PLAY_TAILS              0x0020        // Used in IXAudio2SourceVoice::Stop
#define XAUDIO2_END_OF_STREAM           0x0040        // Used in XAUDIO2_BUFFER.Flags
#define XAUDIO2_SEND_USEFILTER          0x0080        // Used in XAUDIO2_SEND_DESCRIPTOR.Flags

// Default parameters for the built-in filter
#define XAUDIO2_DEFAULT_FILTER_TYPE     LowPassFilter
#define XAUDIO2_DEFAULT_FILTER_FREQUENCY XAUDIO2_MAX_FILTER_FREQUENCY
#define XAUDIO2_DEFAULT_FILTER_ONEOVERQ 1.0f

// Internal XAudio2 constants
#ifdef _XBOX
    #define XAUDIO2_QUANTUM_NUMERATOR   2             // On Xbox 360, XAudio2 processes audio
    #define XAUDIO2_QUANTUM_DENOMINATOR 375           //  in 5.333ms chunks (= 2/375 seconds)
#else
    #define XAUDIO2_QUANTUM_NUMERATOR   1             // On Windows, XAudio2 processes audio
    #define XAUDIO2_QUANTUM_DENOMINATOR 100           //  in 10ms chunks (= 1/100 seconds)
#endif
#define XAUDIO2_QUANTUM_MS (1000.0f * XAUDIO2_QUANTUM_NUMERATOR / XAUDIO2_QUANTUM_DENOMINATOR)

// XAudio2 error codes
#define FACILITY_XAUDIO2 0x896
#define XAUDIO2_E_INVALID_CALL          0x88960001    // An API call or one of its arguments was illegal
#define XAUDIO2_E_XMA_DECODER_ERROR     0x88960002    // The XMA hardware suffered an unrecoverable error
#define XAUDIO2_E_XAPO_CREATION_FAILED  0x88960003    // XAudio2 failed to initialize an XAPO effect
#define XAUDIO2_E_DEVICE_INVALIDATED    0x88960004    // An audio device became unusable (unplugged, etc)


/**************************************************************************
 *
 * Forward declarations for the XAudio2 interfaces.
 *
 **************************************************************************/

#ifdef __cplusplus
    #define FWD_DECLARE(x) interface x
#else
    #define FWD_DECLARE(x) typedef interface x x
#endif

FWD_DECLARE(IXAudio2);
FWD_DECLARE(IXAudio2Voice);
FWD_DECLARE(IXAudio2SourceVoice);
FWD_DECLARE(IXAudio2SubmixVoice);
FWD_DECLARE(IXAudio2MasteringVoice);
FWD_DECLARE(IXAudio2EngineCallback);
FWD_DECLARE(IXAudio2VoiceCallback);


/**************************************************************************
 *
 * XAudio2 structures and enumerations.
 *
 **************************************************************************/

// Used in IXAudio2::Initialize
#ifdef _XBOX
    typedef enum XAUDIO2_XBOX_HWTHREAD_SPECIFIER
    {
        XboxThread0 = 0x01,
        XboxThread1 = 0x02,
        XboxThread2 = 0x04,
        XboxThread3 = 0x08,
        XboxThread4 = 0x10,
        XboxThread5 = 0x20,
        XAUDIO2_ANY_PROCESSOR = XboxThread4,
        XAUDIO2_DEFAULT_PROCESSOR = XAUDIO2_ANY_PROCESSOR
    } XAUDIO2_XBOX_HWTHREAD_SPECIFIER, XAUDIO2_PROCESSOR;
#else
    typedef enum XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER
    {
        Processor1  = 0x00000001,
        Processor2  = 0x00000002,
        Processor3  = 0x00000004,
        Processor4  = 0x00000008,
        Processor5  = 0x00000010,
        Processor6  = 0x00000020,
        Processor7  = 0x00000040,
        Processor8  = 0x00000080,
        Processor9  = 0x00000100,
        Processor10 = 0x00000200,
        Processor11 = 0x00000400,
        Processor12 = 0x00000800,
        Processor13 = 0x00001000,
        Processor14 = 0x00002000,
        Processor15 = 0x00004000,
        Processor16 = 0x00008000,
        Processor17 = 0x00010000,
        Processor18 = 0x00020000,
        Processor19 = 0x00040000,
        Processor20 = 0x00080000,
        Processor21 = 0x00100000,
        Processor22 = 0x00200000,
        Processor23 = 0x00400000,
        Processor24 = 0x00800000,
        Processor25 = 0x01000000,
        Processor26 = 0x02000000,
        Processor27 = 0x04000000,
        Processor28 = 0x08000000,
        Processor29 = 0x10000000,
        Processor30 = 0x20000000,
        Processor31 = 0x40000000,
        Processor32 = 0x80000000,
        XAUDIO2_ANY_PROCESSOR = 0xffffffff,
        XAUDIO2_DEFAULT_PROCESSOR = XAUDIO2_ANY_PROCESSOR
    } XAUDIO2_WINDOWS_PROCESSOR_SPECIFIER, XAUDIO2_PROCESSOR;
#endif

// Used in XAUDIO2_DEVICE_DETAILS below to describe the types of applications
// that the user has specified each device as a default for.  0 means that the
// device isn't the default for any role.
typedef enum XAUDIO2_DEVICE_ROLE
{
    NotDefaultDevice            = 0x0,
    DefaultConsoleDevice        = 0x1,
    DefaultMultimediaDevice     = 0x2,
    DefaultCommunicationsDevice = 0x4,
    DefaultGameDevice           = 0x8,
    GlobalDefaultDevice         = 0xf,
    InvalidDeviceRole = ~GlobalDefaultDevice
} XAUDIO2_DEVICE_ROLE;

// Returned by IXAudio2::GetDeviceDetails
typedef struct XAUDIO2_DEVICE_DETAILS
{
    WCHAR DeviceID[256];                // String identifier for the audio device.
    WCHAR DisplayName[256];             // Friendly name suitable for display to a human.
    XAUDIO2_DEVICE_ROLE Role;           // Roles that the device should be used for.
    WAVEFORMATEXTENSIBLE OutputFormat;  // The device's native PCM audio output format.
} XAUDIO2_DEVICE_DETAILS;

// Returned by IXAudio2Voice::GetVoiceDetails
typedef struct XAUDIO2_VOICE_DETAILS
{
    UINT32 CreationFlags;               // Flags the voice was created with.
    UINT32 InputChannels;               // Channels in the voice's input audio.
    UINT32 InputSampleRate;             // Sample rate of the voice's input audio.
} XAUDIO2_VOICE_DETAILS;

// Used in XAUDIO2_VOICE_SENDS below
typedef struct XAUDIO2_SEND_DESCRIPTOR
{
    UINT32 Flags;                       // Either 0 or XAUDIO2_SEND_USEFILTER.
    IXAudio2Voice* pOutputVoice;        // This send's destination voice.
} XAUDIO2_SEND_DESCRIPTOR;

// Used in the voice creation functions and in IXAudio2Voice::SetOutputVoices
typedef struct XAUDIO2_VOICE_SENDS
{
    UINT32 SendCount;                   // Number of sends from this voice.
    XAUDIO2_SEND_DESCRIPTOR* pSends;    // Array of SendCount send descriptors.
} XAUDIO2_VOICE_SENDS;

// Used in XAUDIO2_EFFECT_CHAIN below
typedef struct XAUDIO2_EFFECT_DESCRIPTOR
{
    IUnknown* pEffect;                  // Pointer to the effect object's IUnknown interface.
    BOOL InitialState;                  // TRUE if the effect should begin in the enabled state.
    UINT32 OutputChannels;              // How many output channels the effect should produce.
} XAUDIO2_EFFECT_DESCRIPTOR;

// Used in the voice creation functions and in IXAudio2Voice::SetEffectChain
typedef struct XAUDIO2_EFFECT_CHAIN
{
    UINT32 EffectCount;                 // Number of effects in this voice's effect chain.
    XAUDIO2_EFFECT_DESCRIPTOR* pEffectDescriptors; // Array of effect descriptors.
} XAUDIO2_EFFECT_CHAIN;

// Used in XAUDIO2_FILTER_PARAMETERS below
typedef enum XAUDIO2_FILTER_TYPE
{
    LowPassFilter,                      // Attenuates frequencies above the cutoff frequency.
    BandPassFilter,                     // Attenuates frequencies outside a given range.
    HighPassFilter,                     // Attenuates frequencies below the cutoff frequency.
    NotchFilter                         // Attenuates frequencies inside a given range.
} XAUDIO2_FILTER_TYPE;

// Used in IXAudio2Voice::Set/GetFilterParameters and Set/GetOutputFilterParameters
typedef struct XAUDIO2_FILTER_PARAMETERS
{
    XAUDIO2_FILTER_TYPE Type;           // Low-pass, band-pass or high-pass.
    float Frequency;                    // Radian frequency (2 * sin(pi*CutoffFrequency/SampleRate));
                                        //  must be >= 0 and <= XAUDIO2_MAX_FILTER_FREQUENCY
                                        //  (giving a maximum CutoffFrequency of SampleRate/6).
    float OneOverQ;                     // Reciprocal of the filter's quality factor Q;
                                        //  must be > 0 and <= XAUDIO2_MAX_FILTER_ONEOVERQ.
} XAUDIO2_FILTER_PARAMETERS;

// Used in IXAudio2SourceVoice::SubmitSourceBuffer
typedef struct XAUDIO2_BUFFER
{
    UINT32 Flags;                       // Either 0 or XAUDIO2_END_OF_STREAM.
    UINT32 AudioBytes;                  // Size of the audio data buffer in bytes.
    const BYTE* pAudioData;             // Pointer to the audio data buffer.
    UINT32 PlayBegin;                   // First sample in this buffer to be played.
    UINT32 PlayLength;                  // Length of the region to be played in samples,
                                        //  or 0 to play the whole buffer.
    UINT32 LoopBegin;                   // First sample of the region to be looped.
    UINT32 LoopLength;                  // Length of the desired loop region in samples,
                                        //  or 0 to loop the entire buffer.
    UINT32 LoopCount;                   // Number of times to repeat the loop region,
                                        //  or XAUDIO2_LOOP_INFINITE to loop forever.
    void* pContext;                     // Context value to be passed back in callbacks.
} XAUDIO2_BUFFER;

// Used in IXAudio2SourceVoice::SubmitSourceBuffer when submitting XWMA data.
// NOTE: If an XWMA sound is submitted in more than one buffer, each buffer's
// pDecodedPacketCumulativeBytes[PacketCount-1] value must be subtracted from
// all the entries in the next buffer's pDecodedPacketCumulativeBytes array.
// And whether a sound is submitted in more than one buffer or not, the final
// buffer of the sound should use the XAUDIO2_END_OF_STREAM flag, or else the
// client must call IXAudio2SourceVoice::Discontinuity after submitting it.
typedef struct XAUDIO2_BUFFER_WMA
{
    const UINT32* pDecodedPacketCumulativeBytes; // Decoded packet's cumulative size array.
                                                 //  Each element is the number of bytes accumulated
                                                 //  when the corresponding XWMA packet is decoded in
                                                 //  order.  The array must have PacketCount elements.
    UINT32 PacketCount;                          // Number of XWMA packets submitted. Must be >= 1 and
                                                 //  divide evenly into XAUDIO2_BUFFER.AudioBytes.
} XAUDIO2_BUFFER_WMA;

// Returned by IXAudio2SourceVoice::GetState
typedef struct XAUDIO2_VOICE_STATE
{
    void* pCurrentBufferContext;        // The pContext value provided in the XAUDIO2_BUFFER
                                        //  that is currently being processed, or NULL if
                                        //  there are no buffers in the queue.
    UINT32 BuffersQueued;               // Number of buffers currently queued on the voice
                                        //  (including the one that is being processed).
    UINT64 SamplesPlayed;               // Total number of samples produced by the voice since
                                        //  it began processing the current audio stream.
} XAUDIO2_VOICE_STATE;

// Returned by IXAudio2::GetPerformanceData
typedef struct XAUDIO2_PERFORMANCE_DATA
{
    // CPU usage information
    UINT64 AudioCyclesSinceLastQuery;   // CPU cycles spent on audio processing since the
                                        //  last call to StartEngine or GetPerformanceData.
    UINT64 TotalCyclesSinceLastQuery;   // Total CPU cycles elapsed since the last call
                                        //  (only counts the CPU XAudio2 is running on).
    UINT32 MinimumCyclesPerQuantum;     // Fewest CPU cycles spent processing any one
                                        //  audio quantum since the last call.
    UINT32 MaximumCyclesPerQuantum;     // Most CPU cycles spent processing any one
                                        //  audio quantum since the last call.

    // Memory usage information
    UINT32 MemoryUsageInBytes;          // Total heap space currently in use.

    // Audio latency and glitching information
    UINT32 CurrentLatencyInSamples;     // Minimum delay from when a sample is read from a
                                        //  source buffer to when it reaches the speakers.
    UINT32 GlitchesSinceEngineStarted;  // Audio dropouts since the engine was started.

    // Data about XAudio2's current workload
    UINT32 ActiveSourceVoiceCount;      // Source voices currently playing.
    UINT32 TotalSourceVoiceCount;       // Source voices currently existing.
    UINT32 ActiveSubmixVoiceCount;      // Submix voices currently playing/existing.

    UINT32 ActiveResamplerCount;        // Resample xAPOs currently active.
    UINT32 ActiveMatrixMixCount;        // MatrixMix xAPOs currently active.

    // Usage of the hardware XMA decoder (Xbox 360 only)
    UINT32 ActiveXmaSourceVoices;       // Number of source voices decoding XMA data.
    UINT32 ActiveXmaStreams;            // A voice can use more than one XMA stream.
} XAUDIO2_PERFORMANCE_DATA;

// Used in IXAudio2::SetDebugConfiguration
typedef struct XAUDIO2_DEBUG_CONFIGURATION
{
    UINT32 TraceMask;                   // Bitmap of enabled debug message types.
    UINT32 BreakMask;                   // Message types that will break into the debugger.
    BOOL LogThreadID;                   // Whether to log the thread ID with each message.
    BOOL LogFileline;                   // Whether to log the source file and line number.
    BOOL LogFunctionName;               // Whether to log the function name.
    BOOL LogTiming;                     // Whether to log message timestamps.
} XAUDIO2_DEBUG_CONFIGURATION;

// Values for the TraceMask and BreakMask bitmaps.  Only ERRORS and WARNINGS
// are valid in BreakMask.  WARNINGS implies ERRORS, DETAIL implies INFO, and
// FUNC_CALLS implies API_CALLS.  By default, TraceMask is ERRORS and WARNINGS
// and all the other settings are zero.
#define XAUDIO2_LOG_ERRORS     0x0001   // For handled errors with serious effects.
#define XAUDIO2_LOG_WARNINGS   0x0002   // For handled errors that may be recoverable.
#define XAUDIO2_LOG_INFO       0x0004   // Informational chit-chat (e.g. state changes).
#define XAUDIO2_LOG_DETAIL     0x0008   // More detailed chit-chat.
#define XAUDIO2_LOG_API_CALLS  0x0010   // Public API function entries and exits.
#define XAUDIO2_LOG_FUNC_CALLS 0x0020   // Internal function entries and exits.
#define XAUDIO2_LOG_TIMING     0x0040   // Delays detected and other timing data.
#define XAUDIO2_LOG_LOCKS      0x0080   // Usage of critical sections and mutexes.
#define XAUDIO2_LOG_MEMORY     0x0100   // Memory heap usage information.
#define XAUDIO2_LOG_STREAMING  0x1000   // Audio streaming information.


/**************************************************************************
 *
 * IXAudio2: Top-level XAudio2 COM interface.
 *
 **************************************************************************/

// Use default arguments if compiling as C++
#ifdef __cplusplus
    #define X2DEFAULT(x) =x
#else
    #define X2DEFAULT(x)
#endif

#undef INTERFACE
#define INTERFACE IXAudio2
DECLARE_INTERFACE_(IXAudio2, IUnknown)
{
    // NAME: IXAudio2::QueryInterface
    // DESCRIPTION: Queries for a given COM interface on the XAudio2 object.
    //              Only IID_IUnknown and IID_IXAudio2 are supported.
    //
    // ARGUMENTS:
    //  riid - IID of the interface to be obtained.
    //  ppvInterface - Returns a pointer to the requested interface.
    //
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, __deref_out void** ppvInterface) PURE;

    // NAME: IXAudio2::AddRef
    // DESCRIPTION: Adds a reference to the XAudio2 object.
    //
    STDMETHOD_(ULONG, AddRef) (THIS) PURE;

    // NAME: IXAudio2::Release
    // DESCRIPTION: Releases a reference to the XAudio2 object.
    //
    STDMETHOD_(ULONG, Release) (THIS) PURE;

    // NAME: IXAudio2::GetDeviceCount
    // DESCRIPTION: Returns the number of audio output devices available.
    //
    // ARGUMENTS:
    //  pCount - Returns the device count.
    //
    STDMETHOD(GetDeviceCount) (THIS_ __out UINT32* pCount) PURE;

    // NAME: IXAudio2::GetDeviceDetails
    // DESCRIPTION: Returns information about the device with the given index.
    //
    // ARGUMENTS:
    //  Index - Index of the device to be queried.
    //  pDeviceDetails - Returns the device details.
    //
    STDMETHOD(GetDeviceDetails) (THIS_ UINT32 Index, __out XAUDIO2_DEVICE_DETAILS* pDeviceDetails) PURE;

    // NAME: IXAudio2::Initialize
    // DESCRIPTION: Sets global XAudio2 parameters and prepares it for use.
    //
    // ARGUMENTS:
    //  Flags - Flags specifying the XAudio2 object's behavior.  Currently unused.
    //  XAudio2Processor - An XAUDIO2_PROCESSOR enumeration value that specifies
    //  the hardware thread (Xbox) or processor (Windows) that XAudio2 will use.
    //  The enumeration values are platform-specific; platform-independent code
    //  can use XAUDIO2_DEFAULT_PROCESSOR to use the default on each platform.
    //
    STDMETHOD(Initialize) (THIS_ UINT32 Flags X2DEFAULT(0),
                           XAUDIO2_PROCESSOR XAudio2Processor X2DEFAULT(XAUDIO2_DEFAULT_PROCESSOR)) PURE;

    // NAME: IXAudio2::RegisterForCallbacks
    // DESCRIPTION: Adds a new client to receive XAudio2's engine callbacks.
    //
    // ARGUMENTS:
    //  pCallback - Callback interface to be called during each processing pass.
    //
    STDMETHOD(RegisterForCallbacks) (__in IXAudio2EngineCallback* pCallback) PURE;

    // NAME: IXAudio2::UnregisterForCallbacks
    // DESCRIPTION: Removes an existing receiver of XAudio2 engine callbacks.
    //
    // ARGUMENTS:
    //  pCallback - Previously registered callback interface to be removed.
    //
    STDMETHOD_(void, UnregisterForCallbacks) (__in IXAudio2EngineCallback* pCallback) PURE;

    // NAME: IXAudio2::CreateSourceVoice
    // DESCRIPTION: Creates and configures a source voice.
    //
    // ARGUMENTS:
    //  ppSourceVoice - Returns the new object's IXAudio2SourceVoice interface.
    //  pSourceFormat - Format of the audio that will be fed to the voice.
    //  Flags - XAUDIO2_VOICE flags specifying the source voice's behavior.
    //  MaxFrequencyRatio - Maximum SetFrequencyRatio argument to be allowed.
    //  pCallback - Optional pointer to a client-provided callback interface.
    //  pSendList - Optional list of voices this voice should send audio to.
    //  pEffectChain - Optional list of effects to apply to the audio data.
    //
    STDMETHOD(CreateSourceVoice) (THIS_ __deref_out IXAudio2SourceVoice** ppSourceVoice,
                                  __in const WAVEFORMATEX* pSourceFormat,
                                  UINT32 Flags X2DEFAULT(0),
                                  float MaxFrequencyRatio X2DEFAULT(XAUDIO2_DEFAULT_FREQ_RATIO),
                                  __in_opt IXAudio2VoiceCallback* pCallback X2DEFAULT(NULL),
                                  __in_opt const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL),
                                  __in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;

    // NAME: IXAudio2::CreateSubmixVoice
    // DESCRIPTION: Creates and configures a submix voice.
    //
    // ARGUMENTS:
    //  ppSubmixVoice - Returns the new object's IXAudio2SubmixVoice interface.
    //  InputChannels - Number of channels in this voice's input audio data.
    //  InputSampleRate - Sample rate of this voice's input audio data.
    //  Flags - XAUDIO2_VOICE flags specifying the submix voice's behavior.
    //  ProcessingStage - Arbitrary number that determines the processing order.
    //  pSendList - Optional list of voices this voice should send audio to.
    //  pEffectChain - Optional list of effects to apply to the audio data.
    //
    STDMETHOD(CreateSubmixVoice) (THIS_ __deref_out IXAudio2SubmixVoice** ppSubmixVoice,
                                  UINT32 InputChannels, UINT32 InputSampleRate,
                                  UINT32 Flags X2DEFAULT(0), UINT32 ProcessingStage X2DEFAULT(0),
                                  __in_opt const XAUDIO2_VOICE_SENDS* pSendList X2DEFAULT(NULL),
                                  __in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;


    // NAME: IXAudio2::CreateMasteringVoice
    // DESCRIPTION: Creates and configures a mastering voice.
    //
    // ARGUMENTS:
    //  ppMasteringVoice - Returns the new object's IXAudio2MasteringVoice interface.
    //  InputChannels - Number of channels in this voice's input audio data.
    //  InputSampleRate - Sample rate of this voice's input audio data.
    //  Flags - XAUDIO2_VOICE flags specifying the mastering voice's behavior.
    //  DeviceIndex - Identifier of the device to receive the output audio.
    //  pEffectChain - Optional list of effects to apply to the audio data.
    //
    STDMETHOD(CreateMasteringVoice) (THIS_ __deref_out IXAudio2MasteringVoice** ppMasteringVoice,
                                     UINT32 InputChannels X2DEFAULT(XAUDIO2_DEFAULT_CHANNELS),
                                     UINT32 InputSampleRate X2DEFAULT(XAUDIO2_DEFAULT_SAMPLERATE),
                                     UINT32 Flags X2DEFAULT(0), UINT32 DeviceIndex X2DEFAULT(0),
                                     __in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain X2DEFAULT(NULL)) PURE;

    // NAME: IXAudio2::StartEngine
    // DESCRIPTION: Creates and starts the audio processing thread.
    //
    STDMETHOD(StartEngine) (THIS) PURE;

    // NAME: IXAudio2::StopEngine
    // DESCRIPTION: Stops and destroys the audio processing thread.
    //
    STDMETHOD_(void, StopEngine) (THIS) PURE;

    // NAME: IXAudio2::CommitChanges
    // DESCRIPTION: Atomically applies a set of operations previously tagged
    //              with a given identifier.
    //
    // ARGUMENTS:
    //  OperationSet - Identifier of the set of operations to be applied.
    //
    STDMETHOD(CommitChanges) (THIS_ UINT32 OperationSet) PURE;

    // NAME: IXAudio2::GetPerformanceData
    // DESCRIPTION: Returns current resource usage details: memory, CPU, etc.
    //
    // ARGUMENTS:
    //  pPerfData - Returns the performance data structure.
    //
    STDMETHOD_(void, GetPerformanceData) (THIS_ __out XAUDIO2_PERFORMANCE_DATA* pPerfData) PURE;

    // NAME: IXAudio2::SetDebugConfiguration
    // DESCRIPTION: Configures XAudio2's debug output (in debug builds only).
    //
    // ARGUMENTS:
    //  pDebugConfiguration - Structure describing the debug output behavior.
    //  pReserved - Optional parameter; must be NULL.
    //
    STDMETHOD_(void, SetDebugConfiguration) (THIS_ __in_opt const XAUDIO2_DEBUG_CONFIGURATION* pDebugConfiguration,
                                             __in_opt __reserved void* pReserved X2DEFAULT(NULL)) PURE;
};


/**************************************************************************
 *
 * IXAudio2Voice: Base voice management interface.
 *
 **************************************************************************/

#undef INTERFACE
#define INTERFACE IXAudio2Voice
DECLARE_INTERFACE(IXAudio2Voice)
{
    // These methods are declared in a macro so that the same declarations
    // can be used in the derived voice types (IXAudio2SourceVoice, etc).

    #define Declare_IXAudio2Voice_Methods() \
    \
    /* NAME: IXAudio2Voice::GetVoiceDetails
    // DESCRIPTION: Returns the basic characteristics of this voice.
    //
    // ARGUMENTS:
    //  pVoiceDetails - Returns the voice's details.
    */\
    STDMETHOD_(void, GetVoiceDetails) (THIS_ __out XAUDIO2_VOICE_DETAILS* pVoiceDetails) PURE; \
    \
    /* NAME: IXAudio2Voice::SetOutputVoices
    // DESCRIPTION: Replaces the set of submix/mastering voices that receive
    //              this voice's output.
    //
    // ARGUMENTS:
    //  pSendList - Optional list of voices this voice should send audio to.
    */\
    STDMETHOD(SetOutputVoices) (THIS_ __in_opt const XAUDIO2_VOICE_SENDS* pSendList) PURE; \
    \
    /* NAME: IXAudio2Voice::SetEffectChain
    // DESCRIPTION: Replaces this voice's current effect chain with a new one.
    //
    // ARGUMENTS:
    //  pEffectChain - Structure describing the new effect chain to be used.
    */\
    STDMETHOD(SetEffectChain) (THIS_ __in_opt const XAUDIO2_EFFECT_CHAIN* pEffectChain) PURE; \
    \
    /* NAME: IXAudio2Voice::EnableEffect
    // DESCRIPTION: Enables an effect in this voice's effect chain.
    //
    // ARGUMENTS:
    //  EffectIndex - Index of an effect within this voice's effect chain.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    */\
    STDMETHOD(EnableEffect) (THIS_ UINT32 EffectIndex, \
                             UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    \
    /* NAME: IXAudio2Voice::DisableEffect
    // DESCRIPTION: Disables an effect in this voice's effect chain.
    //
    // ARGUMENTS:
    //  EffectIndex - Index of an effect within this voice's effect chain.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    */\
    STDMETHOD(DisableEffect) (THIS_ UINT32 EffectIndex, \
                              UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    \
    /* NAME: IXAudio2Voice::GetEffectState
    // DESCRIPTION: Returns the running state of an effect.
    //
    // ARGUMENTS:
    //  EffectIndex - Index of an effect within this voice's effect chain.
    //  pEnabled - Returns the enabled/disabled state of the given effect.
    */\
    STDMETHOD_(void, GetEffectState) (THIS_ UINT32 EffectIndex, __out BOOL* pEnabled) PURE; \
    \
    /* NAME: IXAudio2Voice::SetEffectParameters
    // DESCRIPTION: Sets effect-specific parameters.
    //
    // REMARKS: Unlike IXAPOParameters::SetParameters, this method may
    //          be called from any thread.  XAudio2 implements
    //          appropriate synchronization to copy the parameters to the
    //          realtime audio processing thread.
    //
    // ARGUMENTS:
    //  EffectIndex - Index of an effect within this voice's effect chain.
    //  pParameters - Pointer to an effect-specific parameters block.
    //  ParametersByteSize - Size of the pParameters array  in bytes.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    */\
    STDMETHOD(SetEffectParameters) (THIS_ UINT32 EffectIndex, \
                                    __in_bcount(ParametersByteSize) const void* pParameters, \
                                    UINT32 ParametersByteSize, \
                                    UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    \
    /* NAME: IXAudio2Voice::GetEffectParameters
    // DESCRIPTION: Obtains the current effect-specific parameters.
    //
    // ARGUMENTS:
    //  EffectIndex - Index of an effect within this voice's effect chain.
    //  pParameters - Returns the current values of the effect-specific parameters.
    //  ParametersByteSize - Size of the pParameters array in bytes.
    */\
    STDMETHOD(GetEffectParameters) (THIS_ UINT32 EffectIndex, \
                                    __out_bcount(ParametersByteSize) void* pParameters, \
                                    UINT32 ParametersByteSize) PURE; \
    \
    /* NAME: IXAudio2Voice::SetFilterParameters
    // DESCRIPTION: Sets this voice's filter parameters.
    //
    // ARGUMENTS:
    //  pParameters - Pointer to the filter's parameter structure.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    */\
    STDMETHOD(SetFilterParameters) (THIS_ __in const XAUDIO2_FILTER_PARAMETERS* pParameters, \
                                    UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    \
    /* NAME: IXAudio2Voice::GetFilterParameters
    // DESCRIPTION: Returns this voice's current filter parameters.
    //
    // ARGUMENTS:
    //  pParameters - Returns the filter parameters.
    */\
    STDMETHOD_(void, GetFilterParameters) (THIS_ __out XAUDIO2_FILTER_PARAMETERS* pParameters) PURE; \
    \
    /* NAME: IXAudio2Voice::SetOutputFilterParameters
    // DESCRIPTION: Sets the filter parameters on one of this voice's sends.
    //
    // ARGUMENTS:
    //  pDestinationVoice - Destination voice of the send whose filter parameters will be set.
    //  pParameters - Pointer to the filter's parameter structure.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    */\
    STDMETHOD(SetOutputFilterParameters) (THIS_ __in_opt IXAudio2Voice* pDestinationVoice, \
                                          __in const XAUDIO2_FILTER_PARAMETERS* pParameters, \
                                          UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    \
    /* NAME: IXAudio2Voice::GetOutputFilterParameters
    // DESCRIPTION: Returns the filter parameters from one of this voice's sends.
    //
    // ARGUMENTS:
    //  pDestinationVoice - Destination voice of the send whose filter parameters will be read.
    //  pParameters - Returns the filter parameters.
    */\
    STDMETHOD_(void, GetOutputFilterParameters) (THIS_ __in_opt IXAudio2Voice* pDestinationVoice, \
                                                 __out XAUDIO2_FILTER_PARAMETERS* pParameters) PURE; \
    \
    /* NAME: IXAudio2Voice::SetVolume
    // DESCRIPTION: Sets this voice's overall volume level.
    //
    // ARGUMENTS:
    //  Volume - New overall volume level to be used, as an amplitude factor.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    */\
    STDMETHOD(SetVolume) (THIS_ float Volume, \
                          UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    \
    /* NAME: IXAudio2Voice::GetVolume
    // DESCRIPTION: Obtains this voice's current overall volume level.
    //
    // ARGUMENTS:
    //  pVolume: Returns the voice's current overall volume level.
    */\
    STDMETHOD_(void, GetVolume) (THIS_ __out float* pVolume) PURE; \
    \
    /* NAME: IXAudio2Voice::SetChannelVolumes
    // DESCRIPTION: Sets this voice's per-channel volume levels.
    //
    // ARGUMENTS:
    //  Channels - Used to confirm the voice's channel count.
    //  pVolumes - Array of per-channel volume levels to be used.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    */\
    STDMETHOD(SetChannelVolumes) (THIS_ UINT32 Channels, __in_ecount(Channels) const float* pVolumes, \
                                  UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    \
    /* NAME: IXAudio2Voice::GetChannelVolumes
    // DESCRIPTION: Returns this voice's current per-channel volume levels.
    //
    // ARGUMENTS:
    //  Channels - Used to confirm the voice's channel count.
    //  pVolumes - Returns an array of the current per-channel volume levels.
    */\
    STDMETHOD_(void, GetChannelVolumes) (THIS_ UINT32 Channels, __out_ecount(Channels) float* pVolumes) PURE; \
    \
    /* NAME: IXAudio2Voice::SetOutputMatrix
    // DESCRIPTION: Sets the volume levels used to mix from each channel of this
    //              voice's output audio to each channel of a given destination
    //              voice's input audio.
    //
    // ARGUMENTS:
    //  pDestinationVoice - The destination voice whose mix matrix to change.
    //  SourceChannels - Used to confirm this voice's output channel count
    //   (the number of channels produced by the last effect in the chain).
    //  DestinationChannels - Confirms the destination voice's input channels.
    //  pLevelMatrix - Array of [SourceChannels * DestinationChannels] send
    //   levels.  The level used to send from source channel S to destination
    //   channel D should be in pLevelMatrix[S + SourceChannels * D].
    //  OperationSet - Used to identify this call as part of a deferred batch.
    */\
    STDMETHOD(SetOutputMatrix) (THIS_ __in_opt IXAudio2Voice* pDestinationVoice, \
                                UINT32 SourceChannels, UINT32 DestinationChannels, \
                                __in_ecount(SourceChannels * DestinationChannels) const float* pLevelMatrix, \
                                UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE; \
    \
    /* NAME: IXAudio2Voice::GetOutputMatrix
    // DESCRIPTION: Obtains the volume levels used to send each channel of this
    //              voice's output audio to each channel of a given destination
    //              voice's input audio.
    //
    // ARGUMENTS:
    //  pDestinationVoice - The destination voice whose mix matrix to obtain.
    //  SourceChannels - Used to confirm this voice's output channel count
    //   (the number of channels produced by the last effect in the chain).
    //  DestinationChannels - Confirms the destination voice's input channels.
    //  pLevelMatrix - Array of send levels, as above.
    */\
    STDMETHOD_(void, GetOutputMatrix) (THIS_ __in_opt IXAudio2Voice* pDestinationVoice, \
                                       UINT32 SourceChannels, UINT32 DestinationChannels, \
                                       __out_ecount(SourceChannels * DestinationChannels) float* pLevelMatrix) PURE; \
    \
    /* NAME: IXAudio2Voice::DestroyVoice
    // DESCRIPTION: Destroys this voice, stopping it if necessary and removing
    //              it from the XAudio2 graph.
    */\
    STDMETHOD_(void, DestroyVoice) (THIS) PURE

    Declare_IXAudio2Voice_Methods();
};


/**************************************************************************
 *
 * IXAudio2SourceVoice: Source voice management interface.
 *
 **************************************************************************/

#undef INTERFACE
#define INTERFACE IXAudio2SourceVoice
DECLARE_INTERFACE_(IXAudio2SourceVoice, IXAudio2Voice)
{
    // Methods from IXAudio2Voice base interface
    Declare_IXAudio2Voice_Methods();

    // NAME: IXAudio2SourceVoice::Start
    // DESCRIPTION: Makes this voice start consuming and processing audio.
    //
    // ARGUMENTS:
    //  Flags - Flags controlling how the voice should be started.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    //
    STDMETHOD(Start) (THIS_ UINT32 Flags X2DEFAULT(0), UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE;

    // NAME: IXAudio2SourceVoice::Stop
    // DESCRIPTION: Makes this voice stop consuming audio.
    //
    // ARGUMENTS:
    //  Flags - Flags controlling how the voice should be stopped.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    //
    STDMETHOD(Stop) (THIS_ UINT32 Flags X2DEFAULT(0), UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE;

    // NAME: IXAudio2SourceVoice::SubmitSourceBuffer
    // DESCRIPTION: Adds a new audio buffer to this voice's input queue.
    //
    // ARGUMENTS:
    //  pBuffer - Pointer to the buffer structure to be queued.
    //  pBufferWMA - Additional structure used only when submitting XWMA data.
    //
    STDMETHOD(SubmitSourceBuffer) (THIS_ __in const XAUDIO2_BUFFER* pBuffer, __in_opt const XAUDIO2_BUFFER_WMA* pBufferWMA X2DEFAULT(NULL)) PURE;

    // NAME: IXAudio2SourceVoice::FlushSourceBuffers
    // DESCRIPTION: Removes all pending audio buffers from this voice's queue.
    //
    STDMETHOD(FlushSourceBuffers) (THIS) PURE;

    // NAME: IXAudio2SourceVoice::Discontinuity
    // DESCRIPTION: Notifies the voice of an intentional break in the stream of
    //              audio buffers (e.g. the end of a sound), to prevent XAudio2
    //              from interpreting an empty buffer queue as a glitch.
    //
    STDMETHOD(Discontinuity) (THIS) PURE;

    // NAME: IXAudio2SourceVoice::ExitLoop
    // DESCRIPTION: Breaks out of the current loop when its end is reached.
    //
    // ARGUMENTS:
    //  OperationSet - Used to identify this call as part of a deferred batch.
    //
    STDMETHOD(ExitLoop) (THIS_ UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE;

    // NAME: IXAudio2SourceVoice::GetState
    // DESCRIPTION: Returns the number of buffers currently queued on this voice,
    //              the pContext value associated with the currently processing
    //              buffer (if any), and other voice state information.
    //
    // ARGUMENTS:
    //  pVoiceState - Returns the state information.
    //
    STDMETHOD_(void, GetState) (THIS_ __out XAUDIO2_VOICE_STATE* pVoiceState) PURE;

    // NAME: IXAudio2SourceVoice::SetFrequencyRatio
    // DESCRIPTION: Sets this voice's frequency adjustment, i.e. its pitch.
    //
    // ARGUMENTS:
    //  Ratio - Frequency change, expressed as source frequency / target frequency.
    //  OperationSet - Used to identify this call as part of a deferred batch.
    //
    STDMETHOD(SetFrequencyRatio) (THIS_ float Ratio,
                                  UINT32 OperationSet X2DEFAULT(XAUDIO2_COMMIT_NOW)) PURE;

    // NAME: IXAudio2SourceVoice::GetFrequencyRatio
    // DESCRIPTION: Returns this voice's current frequency adjustment ratio.
    //
    // ARGUMENTS:
    //  pRatio - Returns the frequency adjustment.
    //
    STDMETHOD_(void, GetFrequencyRatio) (THIS_ __out float* pRatio) PURE;

    // NAME: IXAudio2SourceVoice::SetSourceSampleRate
    // DESCRIPTION: Reconfigures this voice to treat its source data as being
    //              at a different sample rate than the original one specified
    //              in CreateSourceVoice's pSourceFormat argument.
    //
    // ARGUMENTS:
    //  UINT32 - The intended sample rate of further submitted source data.
    //
    STDMETHOD(SetSourceSampleRate) (THIS_ UINT32 NewSourceSampleRate) PURE;
};


/**************************************************************************
 *
 * IXAudio2SubmixVoice: Submixing voice management interface.
 *
 **************************************************************************/

#undef INTERFACE
#define INTERFACE IXAudio2SubmixVoice
DECLARE_INTERFACE_(IXAudio2SubmixVoice, IXAudio2Voice)
{
    // Methods from IXAudio2Voice base interface
    Declare_IXAudio2Voice_Methods();

    // There are currently no methods specific to submix voices.
};


/**************************************************************************
 *
 * IXAudio2MasteringVoice: Mastering voice management interface.
 *
 **************************************************************************/

#undef INTERFACE
#define INTERFACE IXAudio2MasteringVoice
DECLARE_INTERFACE_(IXAudio2MasteringVoice, IXAudio2Voice)
{
    // Methods from IXAudio2Voice base interface
    Declare_IXAudio2Voice_Methods();

    // There are currently no methods specific to mastering voices.
};


/**************************************************************************
 *
 * IXAudio2EngineCallback: Client notification interface for engine events.
 *
 * REMARKS: Contains methods to notify the client when certain events happen
 *          in the XAudio2 engine.  This interface should be implemented by
 *          the client.  XAudio2 will call these methods via the interface
 *          pointer provided by the client when it calls XAudio2Create or
 *          IXAudio2::Initialize.
 *
 **************************************************************************/

#undef INTERFACE
#define INTERFACE IXAudio2EngineCallback
DECLARE_INTERFACE(IXAudio2EngineCallback)
{
    // Called by XAudio2 just before an audio processing pass begins.
    STDMETHOD_(void, OnProcessingPassStart) (THIS) PURE;

    // Called just after an audio processing pass ends.
    STDMETHOD_(void, OnProcessingPassEnd) (THIS) PURE;

    // Called in the event of a critical system error which requires XAudio2
    // to be closed down and restarted.  The error code is given in Error.
    STDMETHOD_(void, OnCriticalError) (THIS_ HRESULT Error) PURE;
};


/**************************************************************************
 *
 * IXAudio2VoiceCallback: Client notification interface for voice events.
 *
 * REMARKS: Contains methods to notify the client when certain events happen
 *          in an XAudio2 voice.  This interface should be implemented by the
 *          client.  XAudio2 will call these methods via an interface pointer
 *          provided by the client in the IXAudio2::CreateSourceVoice call.
 *
 **************************************************************************/

#undef INTERFACE
#define INTERFACE IXAudio2VoiceCallback
DECLARE_INTERFACE(IXAudio2VoiceCallback)
{
    // Called just before this voice's processing pass begins.
    STDMETHOD_(void, OnVoiceProcessingPassStart) (THIS_ UINT32 BytesRequired) PURE;

    // Called just after this voice's processing pass ends.
    STDMETHOD_(void, OnVoiceProcessingPassEnd) (THIS) PURE;

    // Called when this voice has just finished playing a buffer stream
    // (as marked with the XAUDIO2_END_OF_STREAM flag on the last buffer).
    STDMETHOD_(void, OnStreamEnd) (THIS) PURE;

    // Called when this voice is about to start processing a new buffer.
    STDMETHOD_(void, OnBufferStart) (THIS_ void* pBufferContext) PURE;

    // Called when this voice has just finished processing a buffer.
    // The buffer can now be reused or destroyed.
    STDMETHOD_(void, OnBufferEnd) (THIS_ void* pBufferContext) PURE;

    // Called when this voice has just reached the end position of a loop.
    STDMETHOD_(void, OnLoopEnd) (THIS_ void* pBufferContext) PURE;

    // Called in the event of a critical error during voice processing,
    // such as a failing xAPO or an error from the hardware XMA decoder.
    // The voice may have to be destroyed and re-created to recover from
    // the error.  The callback arguments report which buffer was being
    // processed when the error occurred, and its HRESULT code.
    STDMETHOD_(void, OnVoiceError) (THIS_ void* pBufferContext, HRESULT Error) PURE;
};


/**************************************************************************
 *
 * Macros to make it easier to use the XAudio2 COM interfaces in C code.
 *
 **************************************************************************/

#ifndef __cplusplus

// IXAudio2
#define IXAudio2_QueryInterface(This,riid,ppvInterface) ((This)->lpVtbl->QueryInterface(This,riid,ppvInterface))
#define IXAudio2_AddRef(This) ((This)->lpVtbl->AddRef(This))
#define IXAudio2_Release(This) ((This)->lpVtbl->Release(This))
#define IXAudio2_GetDeviceCount(This,puCount) ((This)->lpVtbl->GetDeviceCount(This,puCount))
#define IXAudio2_GetDeviceDetails(This,Index,pDeviceDetails) ((This)->lpVtbl->GetDeviceDetails(This,Index,pDeviceDetails))
#define IXAudio2_Initialize(This,Flags,XAudio2Processor) ((This)->lpVtbl->Initialize(This,Flags,XAudio2Processor))
#define IXAudio2_CreateSourceVoice(This,ppSourceVoice,pSourceFormat,Flags,MaxFrequencyRatio,pCallback,pSendList,pEffectChain) ((This)->lpVtbl->CreateSourceVoice(This,ppSourceVoice,pSourceFormat,Flags,MaxFrequencyRatio,pCallback,pSendList,pEffectChain))
#define IXAudio2_CreateSubmixVoice(This,ppSubmixVoice,InputChannels,InputSampleRate,Flags,ProcessingStage,pSendList,pEffectChain) ((This)->lpVtbl->CreateSubmixVoice(This,ppSubmixVoice,InputChannels,InputSampleRate,Flags,ProcessingStage,pSendList,pEffectChain))
#define IXAudio2_CreateMasteringVoice(This,ppMasteringVoice,InputChannels,InputSampleRate,Flags,DeviceIndex,pEffectChain) ((This)->lpVtbl->CreateMasteringVoice(This,ppMasteringVoice,InputChannels,InputSampleRate,Flags,DeviceIndex,pEffectChain))
#define IXAudio2_StartEngine(This) ((This)->lpVtbl->StartEngine(This))
#define IXAudio2_StopEngine(This) ((This)->lpVtbl->StopEngine(This))
#define IXAudio2_CommitChanges(This,OperationSet) ((This)->lpVtbl->CommitChanges(This,OperationSet))
#define IXAudio2_GetPerformanceData(This,pPerfData) ((This)->lpVtbl->GetPerformanceData(This,pPerfData))
#define IXAudio2_SetDebugConfiguration(This,pDebugConfiguration,pReserved) ((This)->lpVtbl->SetDebugConfiguration(This,pDebugConfiguration,pReserved))

// IXAudio2Voice
#define IXAudio2Voice_GetVoiceDetails(This,pVoiceDetails) ((This)->lpVtbl->GetVoiceDetails(This,pVoiceDetails))
#define IXAudio2Voice_SetOutputVoices(This,pSendList) ((This)->lpVtbl->SetOutputVoices(This,pSendList))
#define IXAudio2Voice_SetEffectChain(This,pEffectChain) ((This)->lpVtbl->SetEffectChain(This,pEffectChain))
#define IXAudio2Voice_EnableEffect(This,EffectIndex,OperationSet) ((This)->lpVtbl->EnableEffect(This,EffectIndex,OperationSet))
#define IXAudio2Voice_DisableEffect(This,EffectIndex,OperationSet) ((This)->lpVtbl->DisableEffect(This,EffectIndex,OperationSet))
#define IXAudio2Voice_GetEffectState(This,EffectIndex,pEnabled) ((This)->lpVtbl->GetEffectState(This,EffectIndex,pEnabled))
#define IXAudio2Voice_SetEffectParameters(This,EffectIndex,pParameters,ParametersByteSize, OperationSet) ((This)->lpVtbl->SetEffectParameters(This,EffectIndex,pParameters,ParametersByteSize,OperationSet))
#define IXAudio2Voice_GetEffectParameters(This,EffectIndex,pParameters,ParametersByteSize) ((This)->lpVtbl->GetEffectParameters(This,EffectIndex,pParameters,ParametersByteSize))
#define IXAudio2Voice_SetFilterParameters(This,pParameters,OperationSet) ((This)->lpVtbl->SetFilterParameters(This,pParameters,OperationSet))
#define IXAudio2Voice_GetFilterParameters(This,pParameters) ((This)->lpVtbl->GetFilterParameters(This,pParameters))
#define IXAudio2Voice_SetOutputFilterParameters(This,pDestinationVoice,pParameters,OperationSet) ((This)->lpVtbl->SetOutputFilterParameters(This,pDestinationVoice,pParameters,OperationSet))
#define IXAudio2Voice_GetOutputFilterParameters(This,pDestinationVoice,pParameters) ((This)->lpVtbl->GetOutputFilterParameters(This,pDestinationVoice,pParameters))
#define IXAudio2Voice_SetVolume(This,Volume,OperationSet) ((This)->lpVtbl->SetVolume(This,Volume,OperationSet))
#define IXAudio2Voice_GetVolume(This,pVolume) ((This)->lpVtbl->GetVolume(This,pVolume))
#define IXAudio2Voice_SetChannelVolumes(This,Channels,pVolumes,OperationSet) ((This)->lpVtbl->SetChannelVolumes(This,Channels,pVolumes,OperationSet))
#define IXAudio2Voice_GetChannelVolumes(This,Channels,pVolumes) ((This)->lpVtbl->GetChannelVolumes(This,Channels,pVolumes))
#define IXAudio2Voice_SetOutputMatrix(This,pDestinationVoice,SourceChannels,DestinationChannels,pLevelMatrix,OperationSet) ((This)->lpVtbl->SetOutputMatrix(This,pDestinationVoice,SourceChannels,DestinationChannels,pLevelMatrix,OperationSet))
#define IXAudio2Voice_GetOutputMatrix(This,pDestinationVoice,SourceChannels,DestinationChannels,pLevelMatrix) ((This)->lpVtbl->GetOutputMatrix(This,pDestinationVoice,SourceChannels,DestinationChannels,pLevelMatrix))
#define IXAudio2Voice_DestroyVoice(This) ((This)->lpVtbl->DestroyVoice(This))

// IXAudio2SourceVoice
#define IXAudio2SourceVoice_GetVoiceDetails IXAudio2Voice_GetVoiceDetails
#define IXAudio2SourceVoice_SetOutputVoices IXAudio2Voice_SetOutputVoices
#define IXAudio2SourceVoice_SetEffectChain IXAudio2Voice_SetEffectChain
#define IXAudio2SourceVoice_EnableEffect IXAudio2Voice_EnableEffect
#define IXAudio2SourceVoice_DisableEffect IXAudio2Voice_DisableEffect
#define IXAudio2SourceVoice_GetEffectState IXAudio2Voice_GetEffectState
#define IXAudio2SourceVoice_SetEffectParameters IXAudio2Voice_SetEffectParameters
#define IXAudio2SourceVoice_GetEffectParameters IXAudio2Voice_GetEffectParameters
#define IXAudio2SourceVoice_SetFilterParameters IXAudio2Voice_SetFilterParameters
#define IXAudio2SourceVoice_GetFilterParameters IXAudio2Voice_GetFilterParameters
#define IXAudio2SourceVoice_SetOutputFilterParameters IXAudio2Voice_SetOutputFilterParameters
#define IXAudio2SourceVoice_GetOutputFilterParameters IXAudio2Voice_GetOutputFilterParameters
#define IXAudio2SourceVoice_SetVolume IXAudio2Voice_SetVolume
#define IXAudio2SourceVoice_GetVolume IXAudio2Voice_GetVolume
#define IXAudio2SourceVoice_SetChannelVolumes IXAudio2Voice_SetChannelVolumes
#define IXAudio2SourceVoice_GetChannelVolumes IXAudio2Voice_GetChannelVolumes
#define IXAudio2SourceVoice_SetOutputMatrix IXAudio2Voice_SetOutputMatrix
#define IXAudio2SourceVoice_GetOutputMatrix IXAudio2Voice_GetOutputMatrix
#define IXAudio2SourceVoice_DestroyVoice IXAudio2Voice_DestroyVoice
#define IXAudio2SourceVoice_Start(This,Flags,OperationSet) ((This)->lpVtbl->Start(This,Flags,OperationSet))
#define IXAudio2SourceVoice_Stop(This,Flags,OperationSet) ((This)->lpVtbl->Stop(This,Flags,OperationSet))
#define IXAudio2SourceVoice_SubmitSourceBuffer(This,pBuffer,pBufferWMA) ((This)->lpVtbl->SubmitSourceBuffer(This,pBuffer,pBufferWMA))
#define IXAudio2SourceVoice_FlushSourceBuffers(This) ((This)->lpVtbl->FlushSourceBuffers(This))
#define IXAudio2SourceVoice_Discontinuity(This) ((This)->lpVtbl->Discontinuity(This))
#define IXAudio2SourceVoice_ExitLoop(This,OperationSet) ((This)->lpVtbl->ExitLoop(This,OperationSet))
#define IXAudio2SourceVoice_GetState(This,pVoiceState) ((This)->lpVtbl->GetState(This,pVoiceState))
#define IXAudio2SourceVoice_SetFrequencyRatio(This,Ratio,OperationSet) ((This)->lpVtbl->SetFrequencyRatio(This,Ratio,OperationSet))
#define IXAudio2SourceVoice_GetFrequencyRatio(This,pRatio) ((This)->lpVtbl->GetFrequencyRatio(This,pRatio))
#define IXAudio2SourceVoice_SetSourceSampleRate(This,NewSourceSampleRate) ((This)->lpVtbl->SetSourceSampleRate(This,NewSourceSampleRate))

// IXAudio2SubmixVoice
#define IXAudio2SubmixVoice_GetVoiceDetails IXAudio2Voice_GetVoiceDetails
#define IXAudio2SubmixVoice_SetOutputVoices IXAudio2Voice_SetOutputVoices
#define IXAudio2SubmixVoice_SetEffectChain IXAudio2Voice_SetEffectChain
#define IXAudio2SubmixVoice_EnableEffect IXAudio2Voice_EnableEffect
#define IXAudio2SubmixVoice_DisableEffect IXAudio2Voice_DisableEffect
#define IXAudio2SubmixVoice_GetEffectState IXAudio2Voice_GetEffectState
#define IXAudio2SubmixVoice_SetEffectParameters IXAudio2Voice_SetEffectParameters
#define IXAudio2SubmixVoice_GetEffectParameters IXAudio2Voice_GetEffectParameters
#define IXAudio2SubmixVoice_SetFilterParameters IXAudio2Voice_SetFilterParameters
#define IXAudio2SubmixVoice_GetFilterParameters IXAudio2Voice_GetFilterParameters
#define IXAudio2SubmixVoice_SetOutputFilterParameters IXAudio2Voice_SetOutputFilterParameters
#define IXAudio2SubmixVoice_GetOutputFilterParameters IXAudio2Voice_GetOutputFilterParameters
#define IXAudio2SubmixVoice_SetVolume IXAudio2Voice_SetVolume
#define IXAudio2SubmixVoice_GetVolume IXAudio2Voice_GetVolume
#define IXAudio2SubmixVoice_SetChannelVolumes IXAudio2Voice_SetChannelVolumes
#define IXAudio2SubmixVoice_GetChannelVolumes IXAudio2Voice_GetChannelVolumes
#define IXAudio2SubmixVoice_SetOutputMatrix IXAudio2Voice_SetOutputMatrix
#define IXAudio2SubmixVoice_GetOutputMatrix IXAudio2Voice_GetOutputMatrix
#define IXAudio2SubmixVoice_DestroyVoice IXAudio2Voice_DestroyVoice

// IXAudio2MasteringVoice
#define IXAudio2MasteringVoice_GetVoiceDetails IXAudio2Voice_GetVoiceDetails
#define IXAudio2MasteringVoice_SetOutputVoices IXAudio2Voice_SetOutputVoices
#define IXAudio2MasteringVoice_SetEffectChain IXAudio2Voice_SetEffectChain
#define IXAudio2MasteringVoice_EnableEffect IXAudio2Voice_EnableEffect
#define IXAudio2MasteringVoice_DisableEffect IXAudio2Voice_DisableEffect
#define IXAudio2MasteringVoice_GetEffectState IXAudio2Voice_GetEffectState
#define IXAudio2MasteringVoice_SetEffectParameters IXAudio2Voice_SetEffectParameters
#define IXAudio2MasteringVoice_GetEffectParameters IXAudio2Voice_GetEffectParameters
#define IXAudio2MasteringVoice_SetFilterParameters IXAudio2Voice_SetFilterParameters
#define IXAudio2MasteringVoice_GetFilterParameters IXAudio2Voice_GetFilterParameters
#define IXAudio2MasteringVoice_SetOutputFilterParameters IXAudio2Voice_SetOutputFilterParameters
#define IXAudio2MasteringVoice_GetOutputFilterParameters IXAudio2Voice_GetOutputFilterParameters
#define IXAudio2MasteringVoice_SetVolume IXAudio2Voice_SetVolume
#define IXAudio2MasteringVoice_GetVolume IXAudio2Voice_GetVolume
#define IXAudio2MasteringVoice_SetChannelVolumes IXAudio2Voice_SetChannelVolumes
#define IXAudio2MasteringVoice_GetChannelVolumes IXAudio2Voice_GetChannelVolumes
#define IXAudio2MasteringVoice_SetOutputMatrix IXAudio2Voice_SetOutputMatrix
#define IXAudio2MasteringVoice_GetOutputMatrix IXAudio2Voice_GetOutputMatrix
#define IXAudio2MasteringVoice_DestroyVoice IXAudio2Voice_DestroyVoice

#endif // #ifndef __cplusplus


/**************************************************************************
 *
 * Utility functions used to convert from pitch in semitones and volume
 * in decibels to the frequency and amplitude ratio units used by XAudio2.
 * These are only defined if the client #defines XAUDIO2_HELPER_FUNCTIONS
 * prior to #including xaudio2.h.
 *
 **************************************************************************/

#ifdef XAUDIO2_HELPER_FUNCTIONS

#define _USE_MATH_DEFINES   // Make math.h define M_PI
#include <math.h>           // For powf, log10f, sinf and asinf

// Calculate the argument to SetVolume from a decibel value
__inline float XAudio2DecibelsToAmplitudeRatio(float Decibels)
{
    return powf(10.0f, Decibels / 20.0f);
}

// Recover a volume in decibels from an amplitude factor
__inline float XAudio2AmplitudeRatioToDecibels(float Volume)
{
    if (Volume == 0)
    {
        return -3.402823466e+38f; // Smallest float value (-FLT_MAX)
    }
    return 20.0f * log10f(Volume);
}

// Calculate the argument to SetFrequencyRatio from a semitone value
__inline float XAudio2SemitonesToFrequencyRatio(float Semitones)
{
    // FrequencyRatio = 2 ^ Octaves
    //                = 2 ^ (Semitones / 12)
    return powf(2.0f, Semitones / 12.0f);
}

// Recover a pitch in semitones from a frequency ratio
__inline float XAudio2FrequencyRatioToSemitones(float FrequencyRatio)
{
    // Semitones = 12 * log2(FrequencyRatio)
    //           = 12 * log2(10) * log10(FrequencyRatio)
    return 39.86313713864835f * log10f(FrequencyRatio);
}

// Convert from filter cutoff frequencies expressed in Hertz to the radian
// frequency values used in XAUDIO2_FILTER_PARAMETERS.Frequency.  Note that
// the highest CutoffFrequency supported is SampleRate/6.  Higher values of
// CutoffFrequency will return XAUDIO2_MAX_FILTER_FREQUENCY.
__inline float XAudio2CutoffFrequencyToRadians(float CutoffFrequency, UINT32 SampleRate)
{
    if ((UINT32)(CutoffFrequency * 6.0f) >= SampleRate)
    {
        return XAUDIO2_MAX_FILTER_FREQUENCY;
    }
    return 2.0f * sinf((float)M_PI * CutoffFrequency / SampleRate);
}

// Convert from radian frequencies back to absolute frequencies in Hertz
__inline float XAudio2RadiansToCutoffFrequency(float Radians, float SampleRate)
{
    return SampleRate * asinf(Radians / 2.0f) / (float)M_PI;
}
#endif // #ifdef XAUDIO2_HELPER_FUNCTIONS


/**************************************************************************
 *
 * XAudio2Create: Top-level function that creates an XAudio2 instance.
 *
 * On Windows this is just an inline function that calls CoCreateInstance
 * and Initialize.  The arguments are described above, under Initialize,
 * except that the XAUDIO2_DEBUG_ENGINE flag can be used here to select
 * the debug version of XAudio2.
 *
 * On Xbox, this function is implemented in the XAudio2 library, and the
 * XAUDIO2_DEBUG_ENGINE flag has no effect; the client must explicitly
 * link with the debug version of the library to obtain debug behavior.
 *
 **************************************************************************/

#ifdef _XBOX

STDAPI XAudio2Create(__deref_out IXAudio2** ppXAudio2, UINT32 Flags X2DEFAULT(0),
                     XAUDIO2_PROCESSOR XAudio2Processor X2DEFAULT(XAUDIO2_DEFAULT_PROCESSOR));

#else // Windows

__inline HRESULT XAudio2Create(__deref_out IXAudio2** ppXAudio2, UINT32 Flags X2DEFAULT(0),
                               XAUDIO2_PROCESSOR XAudio2Processor X2DEFAULT(XAUDIO2_DEFAULT_PROCESSOR))
{
    // Instantiate the appropriate XAudio2 engine
    IXAudio2* pXAudio2;

    #ifdef __cplusplus

        HRESULT hr = CoCreateInstance((Flags & XAUDIO2_DEBUG_ENGINE) ? CLSID_XAudio2_Debug : CLSID_XAudio2,
                                      NULL, CLSCTX_INPROC_SERVER, IID_IXAudio2, (void**)&pXAudio2);
        if (SUCCEEDED(hr))
        {
            hr = pXAudio2->Initialize(Flags, XAudio2Processor);

            if (SUCCEEDED(hr))
            {
                *ppXAudio2 = pXAudio2;
            }
            else
            {
                pXAudio2->Release();
            }
        }

    #else

        HRESULT hr = CoCreateInstance((Flags & XAUDIO2_DEBUG_ENGINE) ? &CLSID_XAudio2_Debug : &CLSID_XAudio2,
                                      NULL, CLSCTX_INPROC_SERVER, &IID_IXAudio2, (void**)&pXAudio2);
        if (SUCCEEDED(hr))
        {
            hr = pXAudio2->lpVtbl->Initialize(pXAudio2, Flags, XAudio2Processor);

            if (SUCCEEDED(hr))
            {
                *ppXAudio2 = pXAudio2;
            }
            else
            {
                pXAudio2->lpVtbl->Release(pXAudio2);
            }
        }

    #endif // #ifdef __cplusplus

    return hr;
}

#endif // #ifdef _XBOX


// Undo the #pragma pack(push, 1) directive at the top of this file
#pragma pack(pop)

#endif // #ifndef GUID_DEFS_ONLY
#endif // #ifndef __XAUDIO2_INCLUDED__
