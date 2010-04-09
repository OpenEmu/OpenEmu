/*
    This file is part of CrabEmu.

    Copyright (C) 2005, 2006, 2007, 2008 Lawrence Sebald

    CrabEmu is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2 
    as published by the Free Software Foundation.

    CrabEmu is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CrabEmu; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stdio.h>
#include <string.h>

#include "sn76489.h"

/* These constants came from Maxim's core (then doubled). */
static const int volume_values[16] = { 
    1784, 1548, 1338, 1150,  984,  834,  702,  584,
     478,  384,  300,  226,  160,  100,   48,    0
};

int sn76489_init(sn76489_t *psg, float clock, float sample_rate,
                 uint16 noise_bits, uint16 tapped) {
    psg->volume[0] = 0xF;
    psg->volume[1] = 0xF;
    psg->volume[2] = 0xF;
    psg->volume[3] = 0xF;

    psg->tone[0] = 0x00;
    psg->tone[1] = 0x00;
    psg->tone[2] = 0x00;
    psg->noise = 0x00;

    psg->latched_reg = LATCH_TONE0;

    psg->counter[0] = 0x00;
    psg->counter[1] = 0x00;
    psg->counter[2] = 0x00;
    psg->counter[3] = 0x00;

    psg->tone_state[0] = 1;
    psg->tone_state[1] = 1;
    psg->tone_state[2] = 1;
    psg->tone_state[3] = 1;

    psg->enabled_channels = 0x0F;

    psg->output_channels = 0xFF; /* All Channels, both sides */

    memset(psg->channel_masks[0], 0xFFFFFFFF, 4 * sizeof(uint32));
    memset(psg->channel_masks[1], 0xFFFFFFFF, 4 * sizeof(uint32));

    psg->clocks_per_sample = clock / 16.0f / sample_rate;

    psg->noise_shift = (1 << (noise_bits - 1));
    psg->noise_tapped = tapped;
    psg->noise_bits = noise_bits;

    return 0;
}

void sn76489_write(sn76489_t *psg, uint8 byte)  {
    if(byte & 0x80) {
        /* This is a LATCH/DATA byte */
        psg->latched_reg = (byte & 0x70);

        switch(psg->latched_reg)    {
            case LATCH_TONE0:
                psg->tone[0] = (psg->tone[0] & 0x3F0) | (byte & 0x0F);
                break;
            case LATCH_TONE1:
                psg->tone[1] = (psg->tone[1] & 0x3F0) | (byte & 0x0F);
                break;
            case LATCH_TONE2:
                psg->tone[2] = (psg->tone[2] & 0x3F0) | (byte & 0x0F);
                break;
            case LATCH_NOISE:
                psg->noise = (byte & 0x07);
                psg->noise_shift = 1 << (psg->noise_bits - 1);
                break;
            case LATCH_VOL0:
                psg->volume[0] = (byte & 0x0F);
                break;
            case LATCH_VOL1:
                psg->volume[1] = (byte & 0x0F);
                break;
            case LATCH_VOL2:
                psg->volume[2] = (byte & 0x0F);
                break;
            case LATCH_VOL3:
                psg->volume[3] = (byte & 0x0F);
                break;
        }
    }
    else    {
        /* This is a DATA byte */
        switch(psg->latched_reg)    {
            case LATCH_TONE0:
                psg->tone[0] = (psg->tone[0] & 0x000F) | ((byte & 0x3F) << 4);
                break;
            case LATCH_TONE1:
                psg->tone[1] = (psg->tone[1] & 0x000F) | ((byte & 0x3F) << 4);
                break;
            case LATCH_TONE2:
                psg->tone[2] = (psg->tone[2] & 0x000F) | ((byte & 0x3F) << 4);
                break;
            case LATCH_NOISE:
                psg->noise = (byte & 0x07);
                psg->noise_shift = 1 << (psg->noise_bits - 1);
                break;
            case LATCH_VOL0:
                psg->volume[0] = (byte & 0x0F);
                break;
            case LATCH_VOL1:
                psg->volume[1] = (byte & 0x0F);
                break;
            case LATCH_VOL2:
                psg->volume[2] = (byte & 0x0F);
                break;
            case LATCH_VOL3:
                psg->volume[3] = (byte & 0x0F);
                break;
        }
    }
}

/* This is pretty much taken directly from Maxim's SN76489 document. */
static inline int parity(uint16 input)    {
    input ^= input >> 8;
    input ^= input >> 4;
    input ^= input >> 2;
    input ^= input >> 1;
    return input & 1;
}

#ifndef _arch_dreamcast

void sn76489_execute_samples(sn76489_t *psg, int16 *buf,
                             uint32 samples)    {
    int32 channels[4];
    uint32 i, j;

    for(i = 0; i < samples; ++i)    {
        for(j = 0; j < 3; ++j)  {
            psg->counter[j] -= psg->clocks_per_sample;
            channels[j] = ((psg->enabled_channels >> j) & 0x01) *
                          psg->tone_state[j] * volume_values[psg->volume[j]];
            if(psg->counter[j] <= 0.0f) {
                if(psg->tone[j] < 7)    {
                    /* The PSG doesn't change states if the tone isn't at least
                       7, this fixes the "Sega" at the beginning of Sonic The
                       Hedgehog 2 for the Game Gear. */
                    psg->tone_state[j] = 1;
                }
                else    {
                    psg->tone_state[j] = -psg->tone_state[j];
                }

                psg->counter[j] += psg->tone[j];
            }
        }

        channels[3] = ((psg->enabled_channels >> 3) & 0x01) *
                      (psg->noise_shift & 0x01) * volume_values[psg->volume[3]];

        psg->counter[3] -= psg->clocks_per_sample;
        
        if(psg->counter[3] < 0.0f)  {
            psg->tone_state[3] = -psg->tone_state[3];
            if((psg->noise & 0x03) == 0x03) {
                psg->counter[3] = psg->counter[2];
            }
            else    {
                psg->counter[3] += 0x10 << (psg->noise & 0x03);
            }

            if(psg->tone_state[3] == 1) {
                if(psg->noise & 0x04)   {
                    psg->noise_shift = (psg->noise_shift >> 1) |
                        (parity(psg->noise_shift & psg->noise_tapped) <<
                        (psg->noise_bits - 1));
                }
                else    {
                    psg->noise_shift = (psg->noise_shift >> 1) |
                        ((psg->noise_shift & 0x01) << (psg->noise_bits - 1));
                }
            }
        }

        buf[i << 1] = (channels[0] & psg->channel_masks[0][0]) +
                      (channels[1] & psg->channel_masks[0][1]) +
                      (channels[2] & psg->channel_masks[0][2]) +
                      (channels[3] & psg->channel_masks[0][3]);

        buf[(i << 1) + 1] = (channels[0] & psg->channel_masks[1][0]) +
                            (channels[1] & psg->channel_masks[1][1]) +
                            (channels[2] & psg->channel_masks[1][2]) +
                            (channels[3] & psg->channel_masks[1][3]);
    }
}

#else

void sn76489_execute_samples_dc(sn76489_t *psg, int16 *bufl,
                                int16 *bufr, uint32 samples) {
    int32 channels[4];
    uint32 i, j;

    for(i = 0; i < samples; ++i)    {
        for(j = 0; j < 3; ++j)  {
            psg->counter[j] -= psg->clocks_per_sample;
            channels[j] = ((psg->enabled_channels >> j) & 0x01) *
                psg->tone_state[j] * volume_values[psg->volume[j]];
            if(psg->counter[j] <= 0.0f) {
                if(psg->tone[j] < 7)    {
                    /* The PSG doesn't change states if the tone isn't at least
                       7, this fixes the "Sega" at the beginning of Sonic The
                       Hedgehog 2 for the Game Gear. */
                    psg->tone_state[j] = 1;
                }
                else    {
                    psg->tone_state[j] = -psg->tone_state[j];
                }

                psg->counter[j] += psg->tone[j];
            }
        }

        channels[3] = ((psg->enabled_channels >> 3) & 0x01) *
            (psg->noise_shift & 0x01) * volume_values[psg->volume[3]];

        psg->counter[3] -= psg->clocks_per_sample;

        if(psg->counter[3] < 0.0f)  {
            psg->tone_state[3] = -psg->tone_state[3];
            if((psg->noise & 0x03) == 0x03) {
                psg->counter[3] = psg->counter[2];
            }
            else    {
                psg->counter[3] += 0x10 << (psg->noise & 0x03);
            }

            if(psg->tone_state[3] == 1) {
                if(psg->noise & 0x04)   {
                    psg->noise_shift = (psg->noise_shift >> 1) |
                        (parity(psg->noise_shift & psg->noise_tapped) <<
                        (psg->noise_bits - 1));
                }
                else    {
                    psg->noise_shift = (psg->noise_shift >> 1) |
                        ((psg->noise_shift & 0x01) << (psg->noise_bits - 1));
                }
            }
        }

        *bufl++ = (channels[0] & psg->channel_masks[0][0]) +
                  (channels[1] & psg->channel_masks[0][1]) +
                  (channels[2] & psg->channel_masks[0][2]) +
                  (channels[3] & psg->channel_masks[0][3]);

        *bufr++ = (channels[0] & psg->channel_masks[1][0]) +
                  (channels[1] & psg->channel_masks[1][1]) +
                  (channels[2] & psg->channel_masks[1][2]) +
                  (channels[3] & psg->channel_masks[1][3]);
    }
}
#endif

void sn76489_set_output_channels(sn76489_t *psg, uint8 data)  {
    psg->output_channels = data;

    memset(psg->channel_masks[0], 0, 4 * sizeof(uint32));
    memset(psg->channel_masks[1], 0, 4 * sizeof(uint32));

    if(psg->output_channels & TONE0_LEFT)
        psg->channel_masks[0][0] = 0xFFFFFFFF;

    if(psg->output_channels & TONE1_LEFT)
        psg->channel_masks[0][1] = 0xFFFFFFFF;

    if(psg->output_channels & TONE2_LEFT)
        psg->channel_masks[0][2] = 0xFFFFFFFF;

    if(psg->output_channels & NOISE_LEFT)
        psg->channel_masks[0][3] = 0xFFFFFFFF;

    if(psg->output_channels & TONE0_RIGHT)
        psg->channel_masks[1][0] = 0xFFFFFFFF;

    if(psg->output_channels & TONE1_RIGHT)
        psg->channel_masks[1][1] = 0xFFFFFFFF;

    if(psg->output_channels & TONE2_RIGHT)
        psg->channel_masks[1][2] = 0xFFFFFFFF;

    if(psg->output_channels & NOISE_RIGHT)
        psg->channel_masks[1][3] = 0xFFFFFFFF;
}
