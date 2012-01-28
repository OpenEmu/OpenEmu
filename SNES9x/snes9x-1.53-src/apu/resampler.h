/* Simple resampler based on bsnes's ruby audio library */

#ifndef __RESAMPLER_H
#define __RESAMPLER_H

#include "ring_buffer.h"

class Resampler : public ring_buffer
{
    public:
        virtual void clear (void)        = 0;
        virtual void time_ratio (double) = 0;
        virtual void read (short *, int) = 0;
        virtual int  avail (void)        = 0;
    
        Resampler (int num_samples) : ring_buffer (num_samples << 1)
        {
        }

        ~Resampler ()
        {
        }

        inline bool
        push (short *src, int num_samples)
        {
            if (max_write () < num_samples)
                return false;

            !num_samples || ring_buffer::push ((unsigned char *) src, num_samples << 1);

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
        
        inline int
        max_write (void)
        {
            return space_empty () >> 1;
        }

        inline void
        resize (int num_samples)
        {
            ring_buffer::resize (num_samples << 1);
        }
};

#endif /* __RESAMPLER_H */
