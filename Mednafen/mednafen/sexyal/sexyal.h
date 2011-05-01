#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <inttypes.h>

typedef struct
{
        int type;
        const char *name;
	const char *short_name;
} SexyAL_enumtype;

typedef struct
{
	uint32_t sampformat;
	uint32_t channels;	/* 1 = mono, 2 = stereo */
	uint32_t rate;		/* Number of frames per second, 22050, 44100, etc. */
	bool revbyteorder;	/* 0 = Native(to CPU), 1 = Reversed.  PDP can go to hell. */
	bool split_stereo;	/* TODO: 0 = Interleaved stereo, 1 = split streams. */
} SexyAL_format;

typedef struct
{
	/* Inputs(requested) and Outputs(obtained) */
	uint32_t ms;		/* Desired buffer size, in milliseconds. */
	uint32_t period_us;	/* Desired period size, in MICROseconds */

	/* Outputs Only(obtained) */
	uint32_t buffer_size;	/* Buffer size(as in the maximum value returned by CanWrite()). In frames. */
	uint32_t period_size;	/* Period/Fragment size.  In frames. */
	uint32_t latency;	/* Estimated total latency(between first Write() and actual sound output; essentially equal to the maximum value of
				   CanWrite() plus any additional internal or external buffering). In frames. */

        //uint32_t ms;            /* Milliseconds of buffering, approximate(application code should set this value to control buffer size). */
	//uint32_t period_time;	/* If non-zero, specifies the desired period/fragment size, in frames. */
	//uint32_t size;		/* Shouldn't be filled in by application code. */
	//uint32_t latency;	/* Estimated latency between Write() and sound output, in frames. */
} SexyAL_buffering;


// Bits 4 through 7 should reflect the byte count for each sample.
// If the format is integer PCM, bit 0 should be 0 if unsigned, 1 if signed.
enum
{
 SEXYAL_FMT_PCMU8 = 0x10,
 SEXYAL_FMT_PCMS8 = 0x11,
 SEXYAL_FMT_PCMU16 = 0x20,
 SEXYAL_FMT_PCMS16 = 0x21,

 SEXYAL_FMT_PCMU24 = 0x40,
 SEXYAL_FMT_PCMS24 = 0x41,

 SEXYAL_FMT_PCMU32 = 0x42,
 SEXYAL_FMT_PCMS32 = 0x43,

 SEXYAL_FMT_PCMFLOAT = 0x4F // 32-bit floating point
};

#if 0
class SexyAL_Device
{
	public:

	SexyAL_Device();

	virtual ~SexyAL_Device()
	{
	 Close();
	}

	virtual int Open(const char *id, const SexyAL_format *format, const SexyAL_buffering *buffering,
		SexyAL_format *got_format, SexyAL_buffering *got_buffering);

	virtual int Close(void);

	virtual int Pause(int state);
	virtual int Clear(void);

	virtual int RawWrite(uint32_t bytes);
	virtual int RawCanWrite(uint32_t *can_write);

	protected:
	SexyAL_format format;
	SexyAL_buffering buffering;
};
#endif

typedef struct __SexyAL_device
{
	int (*SetConvert)(struct __SexyAL_device *, SexyAL_format *);
	int (*Write)(struct __SexyAL_device *, void *data, uint32_t frames);


	// Returns the number of frames that can be written via Write() without blocking.
	// This number may be partially estimated for some drivers, and it may be higher than the actual
	// amount of data that can be written without blocking.  Additionally, it will not be higher
	// than the buffer size(unless there's a bug somewhere ;) ).
	// So, try to use this function for advisory timing purposes only.
	uint32_t (*CanWrite)(struct __SexyAL_device *);

        int (*Close)(struct __SexyAL_device *);

	// Returns 1 on success, 0 on failure(failure if the new pause state equals the old pause state, or another
	// problem occurs).
	int (*Pause)(struct __SexyAL_device *, int state);

	// Clears all audio data pending play on the output device.
	// Returns 1 on success, 0 on failure.
	int (*Clear)(struct __SexyAL_device *);

	// Writes "bytes" bytes of data from "data" to the device, blocking if necessary.
	// Returns 1 on success, 0 on failure(probably fatal).
        int (*RawWrite)(struct __SexyAL_device *, const void *data, uint32_t bytes);

	// Sets *count to the number of bytes that can be written to the device without blocking.
	// Returns 1 on success, 0 on failure(probably fatal).
        int (*RawCanWrite)(struct __SexyAL_device *, uint32_t *can_write);

	// Closes the device.
	// Returns 1 on success, 0 on failure(failure should indicate some resources may be left open/allocated due to
	// an erro, but calling RawClose() again is illegal).
        int (*RawClose)(struct __SexyAL_device *);

	SexyAL_format format;
	SexyAL_format srcformat;
	SexyAL_buffering buffering;
	void *private_data;
} SexyAL_device;

typedef struct __SexyAL_enumdevice
{
        char *name;
        char *id;
        struct __SexyAL_enumdevice *next;
} SexyAL_enumdevice;

typedef struct
{
	int type;
	const char *name;
	const char *short_name;

	SexyAL_device * (*Open)(const char *id, SexyAL_format *format, SexyAL_buffering *buffering);
	SexyAL_enumdevice *(*EnumerateDevices)(void);
} SexyAL_driver;

enum
{
 SEXYAL_TYPE_OSSDSP = 0x001,
 SEXYAL_TYPE_ALSA = 0x002,
 SEXYAL_TYPE_DIRECTSOUND = 0x010,
 SEXYAL_OSX_COREAUDIO = 0x030,	/* TODO */

 SEXYAL_TYPE_ESOUND = 0x100,
 SEXYAL_TYPE_JACK = 0x101,
 SEXYAL_TYPE_SDL = 0x102,

 SEXYAL_TYPE_DUMMY = 0x1FF
};

typedef struct __SexyAL {
        SexyAL_device * (*Open)(struct __SexyAL *, const char *id, SexyAL_format *, SexyAL_buffering *buffering, int type);
	SexyAL_enumdevice *(*EnumerateDevices)(struct __SexyAL *, int type);

	SexyAL_enumtype * (*EnumerateTypes)(struct __SexyAL *);
	void (*Destroy)(struct __SexyAL *);
} SexyAL;

/* Initializes the library, requesting the interface of the version specified and output type. */
void *SexyAL_Init(int version);



/* Utility functions: */
uint32_t SexyAL_rupow2(uint32_t v);
int32_t SexyAL_rnearestpow2(int32_t v, bool round_halfway_up = true);
int64_t SexyAL_Time64(void);
