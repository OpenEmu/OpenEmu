/* Simple fixed-point linear resampler by BearOso*/

#ifndef __LINEAR_RESAMPLER_H
#define __LINEAR_RESAMPLER_H

#include "resampler.h"
#include "snes9x.h"

static const int    f_prec = 15;
static const uint32 f__one = (1 << f_prec);

#define lerp(t, a, b) (((((b) - (a)) * (t)) >> f_prec) + (a))

class LinearResampler : public Resampler
{
    protected:
        uint32 f__r_step;
        uint32 f__inv_r_step;
        uint32 f__r_frac;
        int    r_left, r_right;

    public:
        LinearResampler (int num_samples) : Resampler (num_samples)
        {
            f__r_frac = 0;
        }

        ~LinearResampler ()
        {
        }

        void
        time_ratio (double ratio)
        {
            if (ratio == 0.0)
                ratio = 1.0;
            f__r_step = (uint32) (ratio * f__one);
            f__inv_r_step = (uint32) (f__one / ratio);
            clear ();
        }

        void
        clear (void)
        {
            ring_buffer::clear ();
            f__r_frac = 0;
            r_left = 0;
            r_right = 0;
        }

        void
        read (short *data, int num_samples)
        {
            int i_position = start >> 1;
            short *internal_buffer = (short *) buffer;
            int o_position = 0;
            int consumed = 0;
            int max_samples = (buffer_size >> 1);

            while (o_position < num_samples && consumed < buffer_size)
            {
                if (f__r_step == f__one)
                {
                    data[o_position] = internal_buffer[i_position];
                    data[o_position + 1] = internal_buffer[i_position + 1];

                    o_position += 2;
                    i_position += 2;
                    if (i_position >= max_samples)
                        i_position -= max_samples;
                    consumed += 2;

                    continue;
                }

                while (f__r_frac <= f__one  && o_position < num_samples)
                {
                    data[o_position]     = lerp (f__r_frac,
                                                 r_left,
                                                 internal_buffer[i_position]);
                    data[o_position + 1] = lerp (f__r_frac,
                                                 r_right,
                                                 internal_buffer[i_position + 1]);

                    o_position += 2;

                    f__r_frac += f__r_step;
                }

                if (f__r_frac > f__one)
                {
                    f__r_frac -= f__one;
                    r_left = internal_buffer[i_position];
                    r_right = internal_buffer[i_position + 1];
                    i_position += 2;
                    if (i_position >= max_samples)
                        i_position -= max_samples;
                    consumed += 2;
                }
            }

            size -= consumed << 1;
            start += consumed << 1;
            if (start >= buffer_size)
                start -= buffer_size;
        }

        inline int
        avail (void)
        {
            return (((size >> 2) * f__inv_r_step) - ((f__r_frac * f__inv_r_step) >> f_prec)) >> (f_prec - 1);
        }
};

#endif /* __LINEAR_RESAMPLER_H */
