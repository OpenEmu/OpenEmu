
#include "shared.h"

t_vce vce;

void vce_w(int address, int data)
{
    int msb = (address & 1);

    switch(address & ~1)
    {
        case 0x404: /* Data */
            {
                if(data != vce.data[((vce.addr & 0x1FF) << 1) | (msb)])
                {
                    vce.data[((vce.addr & 0x1FF) << 1) | (msb)] = data;

                    if((vce.addr & 0x0F) != 0x00)
                    {
                        uint16 temp = *(uint16 *)&vce.data[(vce.addr << 1)];
#ifndef LSB_FIRST
                        temp = (temp >> 8) | (temp << 8);
#endif
                        pixel[(vce.addr >> 8) & 1][(vce.addr & 0xFF)] = pixel_lut[temp];
                        temp = (temp >> 1) & 0xFF;
                        xlat[(vce.addr >> 8) & 1][(vce.addr & 0xFF)] = temp;
                    }

                    /* Update overscan color */
                    if((vce.addr & 0x0F) == 0x00)
                    {
                        int n;
                        uint16 temp = *(uint16 *)&vce.data[0];
#ifndef LSB_FIRST
                        temp = (temp >> 8) | (temp << 8);
#endif
                        for(n = 0; n < 0x10; n += 1)
                            pixel[0][(n << 4)] = pixel_lut[temp];
                        temp = (temp >> 1) & 0xFF;
                        for(n = 0; n < 0x10; n += 1)
                            xlat[0][(n << 4)] = temp;
                    }
                }
            }

            /* Increment VCE address on access to the MSB data port */
            if(msb) vce.addr += 1;
            break;

        case 0x402: /* Address */
            if(msb)
                vce.addr = (vce.addr & 0x00FF) | ((data & 1) << 8);
            else
                vce.addr = (vce.addr & 0x0100) | (data);
            break;

        case 0x0400: /* Control */
            if(!msb) vce.ctrl = (data & 1);
            break;
    }
}


int vce_r(int address)
{
    int msb = (address & 1);

    if((address & ~1) == 0x0404)
    {
        uint8 temp = vce.data[((vce.addr & 0x1FF) << 1) | (msb)];
        if(msb) vce.addr += 1;
        return (temp);
    }

    return (0xFF);
}

