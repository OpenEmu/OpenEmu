#ifndef WX_FFMPEG_H
#define WX_FFMPEG_H

// simplified interface for recording audio and/or video from emulator

// unlike the rest of the wx code, this has no wx dependency at all, and
// could be used by other front ends as well.

// this only supports selecting output format via file name extensions;
// maybe some future version will support specifying a format.  wx-2.9
// has an extra widget for the file selector, but 2.8 doesn't.

// the only missing piece that I couldn't figure out how to do generically
// is the code to find the available formats & associated extensions for
// the file dialog.

#include "../common/Types.h"

// return codes
// probably ought to put in own namespace, but this is good enough
enum MediaRet {
    MRET_OK, // no errors
    MRET_ERR_NOMEM, // error allocating buffers or structures
    MRET_ERR_NOCODEC, // error opening codec
    MRET_ERR_FERR, // error writing output file
    MRET_ERR_RECORDING, // attempt to start recording when already doing it
    MRET_ERR_FMTGUESS, // can't guess format from file name
    MRET_ERR_BUFSIZE // buffer overflow (fatal)
};

class MediaRecorder
{
public:
    MediaRecorder();
    virtual ~MediaRecorder();

    // start audio+video (also video-only codecs)
    MediaRet Record(const char *fname, int width, int height, int depth);
    // start audio only
    MediaRet Record(const char *fname);
    // stop both
    void Stop();
    bool IsRecording() { return oc != NULL; }
    // add a frame of video; width+height+depth already given
    // assumes a 1-pixel border on top & right
    // always assumes being passed 1/60th of a second of video
    MediaRet AddFrame(const u8 *vid);
    // add a frame of audio; uses current sample rate to know length
    // always assumes being passed 1/60th of a second of audio.
    MediaRet AddFrame(const u16 *aud);

private:
    static bool did_init;

    // these are to avoid polluting things with avcodec includes
#ifndef priv_AVFormatContext
#define priv_AVFormatContext void
#define priv_AVStream void
#define priv_AVOutputFormat void
#define priv_AVFrame void
#define priv_SwsContext void
#define priv_PixelFormat int
#endif
    priv_AVFormatContext *oc;
    priv_AVStream *vid_st, *aud_st;
    u8 *audio_buf, *video_buf;
    u16 *audio_buf2;
    int frame_len, sample_len, in_audio_buf2;
    int linesize, pixsize;
    priv_PixelFormat pixfmt;
    priv_AVFrame *pic, *convpic;
    priv_SwsContext *converter;
    
    MediaRet setup_sound_stream(const char *fname, priv_AVOutputFormat *fmt);
    MediaRet setup_video_stream(const char *fname, int w, int h, int d);
    MediaRet finish_setup(const char *fname);
};

#endif /* WX_FFMPEG_H */
