/* Simple byte-based ring buffer. Licensed under public domain (C) BearOso. */

#ifndef __RING_BUFFER_H
#define __RING_BUFFER_H

#include <string.h>

#undef MIN
#define MIN(a, b) ((a) < (b) ? (a) : (b))

class ring_buffer
{
protected:
    int size;
    int buffer_size;
    int start;
    unsigned char *buffer;

public:
    ring_buffer (int buffer_size)
    {
        this->buffer_size = buffer_size;
        buffer = new unsigned char[this->buffer_size];
        memset (buffer, 0, this->buffer_size);

        size = 0;
        start = 0;
    }

    ~ring_buffer (void)
    {
        delete[] buffer;
    }

    bool
    push (unsigned char *src, int bytes)
    {
        if (space_empty () < bytes)
            return false;

        int end = (start + size) % buffer_size;
        int first_write_size = MIN (bytes, buffer_size - end);

        memcpy (buffer + end, src, first_write_size);

        if (bytes > first_write_size)
            memcpy (buffer, src + first_write_size, bytes - first_write_size);

        size += bytes;

        return true;
    }

    bool
    pull (unsigned char *dst, int bytes)
    {
        if (space_filled () < bytes)
            return false;

        memcpy (dst, buffer + start, MIN (bytes, buffer_size - start));

        if (bytes > (buffer_size - start))
            memcpy (dst + (buffer_size - start), buffer, bytes - (buffer_size - start));

        start = (start + bytes) % buffer_size;
        size -= bytes;

        return true;
    }

    inline int
    space_empty (void)
    {
        return buffer_size - size;
    }

    inline int
    space_filled (void)
    {
        return size;
    }

    void
    clear (void)
    {
        start = 0;
        size = 0;
        memset (buffer, 0, buffer_size);
    }

    void
    resize (int size)
    {
        delete[] buffer;
        buffer_size = size;
        buffer = new unsigned char[buffer_size];
        memset (buffer, 0, this->buffer_size);

        size = 0;
        start = 0;
    }

    inline void
    cache_silence (void)
    {
        clear ();
        size = buffer_size;
    }
};

#endif
