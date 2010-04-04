/*
    This file is part of CrabEmu.

    Copyright (C) 2008 Lawrence Sebald

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

#include <string.h>
#include <stdio.h>
#include "93c46.h"

eeprom93c46_t e93c46;

void eeprom93c46_init(void) {
    memset(e93c46.data, 0xFF, 64 * sizeof(uint16));
    e93c46.enabled = 0;
    e93c46.lines = EEPROM93c46_LINE_DATA_OUT;
    e93c46.readwrite = 0;
    e93c46.mode = EEPROM93c46_MODE_START;
    e93c46.opcode = 0;
    e93c46.bit = 0;
}

void eeprom93c46_reset(void)    {
    e93c46.enabled = 0;
    e93c46.lines = EEPROM93c46_LINE_DATA_OUT;
    e93c46.readwrite = 0;
    e93c46.mode = EEPROM93c46_MODE_START;
    e93c46.opcode = 0;
    e93c46.bit = 0;
}

void eeprom93c46_ctl_write(uint8 data)  {
    if(data & 0x80) {
        eeprom93c46_reset();
    }

    if(data & 0x08) {
        e93c46.enabled = 1;
    }
    else    {
        e93c46.enabled = 0;
    }
}

uint8 eeprom93c46_read(void)    {
    return (e93c46.lines & EEPROM93c46_LINE_CS) | EEPROM93c46_LINE_CLOCK |
        ((e93c46.lines & EEPROM93c46_LINE_DATA_OUT) >> 3);
}

void eeprom93c46_write(uint8 data)  {
    if(!(data & EEPROM93c46_LINE_CS))   {
        if(e93c46.lines & EEPROM93c46_LINE_CS)  {
            e93c46.mode = EEPROM93c46_MODE_START;
        }

        e93c46.lines = EEPROM93c46_LINE_DATA_OUT | (data & 0x07);
        return;
    }

    /* If the clock goes from low to high, perform magic! */
    if(data & EEPROM93c46_LINE_CLOCK &&
       !(e93c46.lines & EEPROM93c46_LINE_CLOCK))    {
        e93c46.lines = (data & 0x07) |
            (e93c46.lines & EEPROM93c46_LINE_DATA_OUT);

        if(e93c46.mode == EEPROM93c46_MODE_START)   {
            /* Check if the Data In bit is set */
            if(!(data & EEPROM93c46_LINE_DATA_IN))  {
                return;
            }

            /* If we got here, the start bit has been clocked in, go into
               opcode reading mode */
            e93c46.mode = EEPROM93c46_MODE_OPCODE;
            e93c46.opcode = 0;
            e93c46.bit = 0;
            return;
        }
        else if(e93c46.mode == EEPROM93c46_MODE_OPCODE) {
            e93c46.opcode = (e93c46.opcode << 1) | (data & 0x01);

            if(++e93c46.bit == 8)   {
                int op = (e93c46.opcode & 0xC0) >> 6;

                if(op == 0x00)  {
                    if((e93c46.opcode & 0x30) == 0x30)  {
                        /* EWEN opcode */
                        e93c46.mode = EEPROM93c46_MODE_DONE;
                        e93c46.readwrite = 1;
                        return;
                    }
                    else if((e93c46.opcode & 0x30) == 0x00) {
                        /* EWDS opcode */
                        e93c46.mode = EEPROM93c46_MODE_DONE;
                        e93c46.readwrite = 0;
                        return;
                    }
                    else if((e93c46.opcode & 0x30) == 0x10) {
                        /* ERAL opcode */
                        e93c46.lines |= EEPROM93c46_LINE_DATA_OUT;
                        e93c46.mode = EEPROM93c46_MODE_DONE;
                        
                        if(e93c46.readwrite)
                            memset(e93c46.data, 0xFF, 64 * sizeof(uint16));

                        return;
                    }
                    else    {
                        /* WRAL opcode */
                        e93c46.mode = EEPROM93c46_MODE_WRITE;
                        e93c46.bit = 0;
                        e93c46.data_in = 0;
                        return;
                    }
                }
                else if(op == 0x01) {
                    /* WRITE opcode */
                    e93c46.mode = EEPROM93c46_MODE_WRITE;
                    e93c46.bit = 0;
                    e93c46.data_in = 0;
                    return;
                }
                else if(op == 0x02) {
                    /* READ opcode */
                    e93c46.mode = EEPROM93c46_MODE_READ;
                    e93c46.lines &= 0x07;
                    e93c46.bit = 0;
                    return;
                }
                else if(op == 0x03) {
                    /* ERASE opcode */
                    if(e93c46.readwrite)
                        e93c46.data[e93c46.opcode & 0x3F] = 0xFFFF;
    
                    e93c46.mode = EEPROM93c46_MODE_DONE;
                    e93c46.lines |= EEPROM93c46_LINE_DATA_OUT;
                    return;
                }
            }
        }
        else if(e93c46.mode == EEPROM93c46_MODE_WRITE)  {
            e93c46.data_in = (e93c46.data_in << 1) | (data & 0x01);
            if(++e93c46.bit < 16)   {
                return;
            }
            else if(!e93c46.readwrite)  {
                e93c46.mode = EEPROM93c46_MODE_DONE;
                e93c46.lines |= EEPROM93c46_LINE_DATA_OUT;
                return;
            }
            else    {
                e93c46.data_in |= (data & 0x01) << (16 - e93c46.bit);

                /* Was it a WRITE or a WRITE ALL instruction? */
                if(e93c46.opcode & 0x40)    {
                    /* It was a WRITE instruction */
                    e93c46.data[e93c46.opcode & 0x3F] = e93c46.data_in;
                    e93c46.lines |= EEPROM93c46_LINE_DATA_OUT;
                    e93c46.mode = EEPROM93c46_MODE_DONE;

                    return;
                }
                else    {
                    /* It was a WRITE ALL instruction */
                    int i;

                    for(i = 0; i < 64; ++i) {
                        e93c46.data[i] = e93c46.data_in;
                    }
                    e93c46.lines |= EEPROM93c46_LINE_DATA_OUT;
                    e93c46.mode = EEPROM93c46_MODE_DONE;

                    return;
                }
            }
        }
        else if(e93c46.mode == EEPROM93c46_MODE_READ)   {
            ++e93c46.bit;

            if(e93c46.data[e93c46.opcode & 0x3F] &
               (1 << (16 - e93c46.bit)))    {
               e93c46.lines |= EEPROM93c46_LINE_DATA_OUT;
            }
            else    {
                e93c46.lines &= 0x07;
            }

            if(e93c46.bit != 16)
                return;

            e93c46.mode = EEPROM93c46_MODE_DONE;
            return;
        }
    }

    e93c46.lines = (data & 0x07) | (e93c46.lines & EEPROM93c46_LINE_DATA_OUT);
}
