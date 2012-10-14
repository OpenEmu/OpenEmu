/****************************************************************************
 *  Genesis Plus
 *  Microwire Serial EEPROM (93C46 only) support
 *
 *  Copyright (C) 2011  Eke-Eke (Genesis Plus GX)
 *
 *  Redistribution and use of this code or any derivative works are permitted
 *  provided that the following conditions are met:
 *
 *   - Redistributions may not be sold, nor may they be used in a commercial
 *     product or activity.
 *
 *   - Redistributions that are modified from the original source must include the
 *     complete source code, including the source code for all components used by a
 *     binary built from the modified sources. However, as a special exception, the
 *     source code distributed need not include anything that is normally distributed
 *     (in either source or binary form) with the major components (compiler, kernel,
 *     and so on) of the operating system on which the executable runs, unless that
 *     component itself accompanies the executable.
 *
 *   - Redistributions must reproduce the above copyright notice, this list of
 *     conditions and the following disclaimer in the documentation and/or other
 *     materials provided with the distribution.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************************/

#include "shared.h"
#include "eeprom_93c.h"

/* fixed board implementation */
#define BIT_DATA (0)
#define BIT_CLK  (1)
#define BIT_CS   (2)


T_EEPROM_93C eeprom_93c;

void eeprom_93c_init()
{
  /* default eeprom state */
  memset(&eeprom_93c, 0, sizeof(T_EEPROM_93C));
  eeprom_93c.data = 1;
  eeprom_93c.state = WAIT_START;
  sram.custom = 3;
}

void eeprom_93c_write(unsigned char data)
{
  /* Make sure CS is HIGH */
  if (data & (1 << BIT_CS))
  {
    /* Data latched on CLK postive edge */
    if ((data & (1 << BIT_CLK)) && !eeprom_93c.clk)
    {
      /* Current EEPROM state */
      switch (eeprom_93c.state)
      {
        case WAIT_START:
        {
          /* Wait for START bit */
          if (data & (1 << BIT_DATA))
          {
            eeprom_93c.opcode = 0;
            eeprom_93c.cycles = 0;
            eeprom_93c.state = GET_OPCODE;
          }
          break;
        }

        case GET_OPCODE:
        {
          /* 8-bit buffer (opcode + address) */
          eeprom_93c.opcode |= ((data >> BIT_DATA) & 1) << (7 - eeprom_93c.cycles);
          eeprom_93c.cycles++;

          if (eeprom_93c.cycles == 8)
          {
            /* Decode instruction */
            switch ((eeprom_93c.opcode >> 6) & 3)
            {
              case 1:
              {
                /* WRITE */
                eeprom_93c.buffer = 0;
                eeprom_93c.cycles = 0;
                eeprom_93c.state = WRITE_WORD;
                break;
              }

              case 2:
              {
                /* READ */
                eeprom_93c.buffer = *(uint16 *)(sram.sram + ((eeprom_93c.opcode & 0x3F) << 1));
                eeprom_93c.cycles = 0;
                eeprom_93c.state = READ_WORD;

                /* Force DATA OUT */
                eeprom_93c.data = 0;
                break;
              }

              case 3:
              {
                /* ERASE */
                if (eeprom_93c.we)
                {
                  *(uint16 *)(sram.sram + ((eeprom_93c.opcode & 0x3F) << 1)) = 0xFFFF;
                }

                /* wait for next command */
                eeprom_93c.state = WAIT_STANDBY;
                break;
              }

              default:
              {
                /* special command */
                switch ((eeprom_93c.opcode >> 4) & 3)
                {
                  case 1:
                  {
                    /* WRITE ALL */
                    eeprom_93c.buffer = 0;
                    eeprom_93c.cycles = 0;
                    eeprom_93c.state = WRITE_WORD;
                    break;
                  }

                  case 2:
                  {
                    /* ERASE ALL */
                    if (eeprom_93c.we)
                    {
                      memset(sram.sram, 0xFF, 128);
                    }

                    /* wait for next command */
                    eeprom_93c.state = WAIT_STANDBY;
                    break;
                  }

                  default:
                  {
                    /* WRITE ENABLE/DISABLE */
                    eeprom_93c.we = (eeprom_93c.opcode >> 4) & 1;

                    /* wait for next command */
                    eeprom_93c.state = WAIT_STANDBY;
                    break;
                  }
                }
                break;
              }
            }
          }
          break;
        }

        case WRITE_WORD:
        {
          /* 16-bit data buffer */
          eeprom_93c.buffer |= ((data >> BIT_DATA) & 1) << (15 - eeprom_93c.cycles);
          eeprom_93c.cycles++;

          if (eeprom_93c.cycles == 16)
          {
            /* check EEPROM write protection */
            if (eeprom_93c.we)
            {
              if (eeprom_93c.opcode & 0x40)
              {
                /* write one word */
                *(uint16 *)(sram.sram + ((eeprom_93c.opcode & 0x3F) << 1)) = eeprom_93c.buffer;
              }
              else
              {
                /* write 64 words */
                int i;
                for (i=0; i<64; i++)
                {
                  *(uint16 *)(sram.sram + (i << 1)) = eeprom_93c.buffer;

                }
              }
            }

            /* wait for next command */
            eeprom_93c.state = WAIT_STANDBY;
          }
          break;
        }

        case READ_WORD:
        {
          /* set DATA OUT */
          eeprom_93c.data = ((eeprom_93c.buffer >> (15 - eeprom_93c.cycles)) & 1);
          eeprom_93c.cycles++;

          if (eeprom_93c.cycles == 16)
          {
            /* read next word (93C46B) */
            eeprom_93c.opcode++;
            eeprom_93c.cycles = 0;
            eeprom_93c.buffer = *(uint16 *)(sram.sram + ((eeprom_93c.opcode & 0x3F) << 1));
          }
          break;
        }

        default:
        {
          /* wait for STANDBY mode */
          break;
        }
      }
    }
  }
  else
  {
    /* CS HIGH->LOW transition */
    if (eeprom_93c.cs)
    {
      /* standby mode */
      eeprom_93c.data = 1;
      eeprom_93c.state = WAIT_START;
    }
  }

  /* Update input lines */
  eeprom_93c.cs  = (data >> BIT_CS) & 1;
  eeprom_93c.clk = (data >> BIT_CLK) & 1;
}

unsigned char eeprom_93c_read(void)
{
  return ((eeprom_93c.cs << BIT_CS) | (eeprom_93c.data << BIT_DATA) | (1 << BIT_CLK));
}

