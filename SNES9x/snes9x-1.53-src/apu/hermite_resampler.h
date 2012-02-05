/* Simple resampler based on bsnes's ruby audio library */

#ifndef __HERMITE_RESAMPLER_H
#define __HERMITE_RESAMPLER_H

#include "resampler.h"

#undef CLAMP
#undef SHORT_CLAMP
#define CLAMP(x, low, high) (((x) > (high)) ? (high) : (((x) < (low)) ? (low) : (x)))
#define SHORT_CLAMP(n) ((short) CLAMP((n), -32768, 32767))

class HermiteResampler : public Resampler
{
    protected:

        double r_step;
        double r_frac;
        int    r_left[4], r_right[4];

        double
        hermite (double mu1, double a, double b, double c, double d)
        {
            const double tension = 0.0; //-1 = low, 0 = normal, 1 = high
            const double bias    = 0.0; //-1 = left, 0 = even, 1 = right

            double mu2, mu3, m0, m1, a0, a1, a2, a3;

            mu2 = mu1 * mu1;
            mu3 = mu2 * mu1;

            m0  = (b - a) * (1 + bias) * (1 - tension) / 2;
            m0 += (c - b) * (1 - bias) * (1 - tension) / 2;
            m1  = (c - b) * (1 + bias) * (1 - tension) / 2;
            m1 += (d - c) * (1 - bias) * (1 - tension) / 2;

            a0 = +2 * mu3 - 3 * mu2 + 1;
            a1 =      mu3 - 2 * mu2 + mu1;
            a2 =      mu3 -     mu2;
            a3 = -2 * mu3 + 3 * mu2;

            return (a0 * b) + (a1 * m0) + (a2 * m1) + (a3 * c);
        }

    public:
        HermiteResampler (int num_samples) : Resampler (num_samples)
        {
            clear ();
        }

        ~HermiteResampler ()
        {
        }

        void
        time_ratio (double ratio)
        {
            r_step = ratio;
            clear ();
        }

        void
        clear (void)
        {
            ring_buffer::clear ();
            r_frac = 1.0;
            r_left [0] = r_left [1] = r_left [2] = r_left [3] = 0;
            r_right[0] = r_right[1] = r_right[2] = r_right[3] = 0;
        }

        void
        read (short *data, int num_samples)
        {
            int i_position = start >> 1;
            short *internal_buffer = (short *) buffer;
            int o_position = 0;
            int consumed = 0;

            while (o_position < num_samples && consumed < buffer_size)
            {
                int s_left = internal_buffer[i_position];
                int s_right = internal_buffer[i_position + 1];
                int max_samples = buffer_size >> 1;
                const double margin_of_error = 1.0e-10;

                if (fabs(r_step - 1.0) < margin_of_error)
                {
                    data[o_position] = (short) s_left;
                    data[o_position + 1] = (short) s_right;

                    o_position += 2;
                    i_position += 2;
                    if (i_position >= max_samples)
                        i_position -= max_samples;
                    consumed += 2;

                    continue;
                }

                while (r_frac <= 1.0 && o_position < num_samples)
                {
                    data[o_position]     = SHORT_CLAMP (hermite (r_frac, r_left [0], r_left [1], r_left [2], r_left [3]));
                    data[o_position + 1] = SHORT_CLAMP (hermite (r_frac, r_right[0], r_right[1], r_right[2], r_right[3]));

                    o_position += 2;

                    r_frac += r_step;
                }

                if (r_frac > 1.0)
                {
                    r_left [0] = r_left [1];
                    r_left [1] = r_left [2];
                    r_left [2] = r_left [3];
                    r_left [3] = s_left;
    
                    r_right[0] = r_right[1];
                    r_right[1] = r_right[2];
                    r_right[2] = r_right[3];
                    r_right[3] = s_right;                    
                    
                    r_frac -= 1.0;
                    
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
            return (int) floor (((size >> 2) - r_frac) / r_step) * 2;
        }
};

#endif /* __HERMITE_RESAMPLER_H */
