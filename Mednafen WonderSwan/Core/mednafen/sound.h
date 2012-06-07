#if 0

class
{
        public:

        MDFN_SoundBuffer(void);
        ~MDFN_SoundBuffer();

	void Clear(void);

	void SetChannels(int ch);

        private:

        int channels;

        int32 Alloced;        // Size of memory backing the buffer, in sound frames.
        int32 InBufferCount;  // Number of sound frames currently in buffer.

        int16 *buffer;
} MDFN_SoundBuffer;

#endif
