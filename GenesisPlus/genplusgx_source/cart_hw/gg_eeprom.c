/****************************************************************************
 *  Genesis Plus
 *  Microwire Serial EEPROM (93C46) support
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
#include "gg_eeprom.h"

#define BIT_DATA (0)
#define BIT_CLK  (1)
#define BIT_CS   (2)


T_EEPROM_93C gg_eeprom;

void gg_eeprom_init()
{
  /* default eeprom state */
  memset(&gg_eeprom, 0, sizeof(T_EEPROM_93C));
  gg_eeprom.data = 1;
  gg_eeprom.state = WAIT_START;
}

void gg_eeprom_ctrl(unsigned char data)
{
  /* Reset EEPROM */
  if (data & 0x80)
  {
    gg_eeprom_init();
    return;
  }

  /* Enable EEPROM */
  gg_eeprom.enabled = data & 0x08;
}

void gg_eeprom_write(unsigned char data)
{
  /* Make sure CS is HIGH */
  if (data & (1 << BIT_CS))
  {
    /* Data latched on CLK postive edge */
    if ((data & (1 << BIT_CLK)) && !gg_eeprom.clk)
    {
      /* Current EEPROM state */
      switch (gg_eeprom.state)
      {
        case WAIT_START:
        {
          /* Wait for START bit */
          if (data & (1 << BIT_DATA))
          {
            gg_eeprom.opcode = 0;
            gg_eeprom.cycles = 0;
            gg_eeprom.state = GET_OPCODE;
          }
          break;
        }

        case GET_OPCODE:
        {
          /* 8-bit buffer (opcode + address) */
          gg_eeprom.opcode |= ((data >> BIT_DATA) & 1) << (7 - gg_eeprom.cycles);
          gg_eeprom.cycles++;

          if (gg_eeprom.cycles == 8)
          {
            /* Decode instruction */
            switch ((gg_eeprom.opcode >> 6) & 3)
            {
              case 1:
              {
                /* WRITE */
                gg_eeprom.buffer = 0;
                gg_eeprom.cycles = 0;
                gg_eeprom.state = WRITE_WORD;
                break;
              }

              case 2:
              {
                /* READ */
                gg_eeprom.buffer = *(uint16 *)(sram.sram + ((gg_eeprom.opcode & 0x3F) << 1));
                gg_eeprom.cycles = 0;
                gg_eeprom.state = READ_WORD;

                /* Force DATA OUT */
                gg_eeprom.data = 0;
                break;
              }

              case 3:
              {
                /* ERASE */
                if (gg_eeprom.we)
                {
                  *(uint16 *)(sram.sram + ((gg_eeprom.opcode & 0x3F) << 1)) = 0xFFFF;
                }

                /* wait for next command */
                gg_eeprom.state = WAIT_STANDBY;
                break;
              }

              default:
              {
                /* special command */
                switch ((gg_eeprom.opcode >> 4) & 3)
                {
                  case 1:
                  {
                    /* WRITE ALL */
                    gg_eeprom.buffer = 0;
                    gg_eeprom.cycles = 0;
                    gg_eeprom.state = WRITE_WORD;
                    break;
                  }

                  case 2:
                  {
                    /* ERASE ALL */
                    if (gg_eeprom.we)
                    {
                      memset(sram.sram, 0xFF, 128);
                    }

                    /* wait for next command */
                    gg_eeprom.state = WAIT_STANDBY;
                    break;
                  }

                  default:
                  {
                    /* WRITE ENABLE/DISABLE */
                    gg_eeprom.we = (gg_eeprom.opcode >> 4) & 1;

                    /* wait for next command */
                    gg_eeprom.state = WAIT_STANDBY;
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
          gg_eeprom.buffer |= ((data >> BIT_DATA) & 1) << (15 - gg_eeprom.cycles);
          gg_eeprom.cycles++;

          if (gg_eeprom.cycles == 16)
          {
            /* check EEPROM write protection */
            if (gg_eeprom.we)
            {
              if (gg_eeprom.opcode & 0x40)
              {
                /* write one word */
                *(uint16 *)(sram.sram + ((gg_eeprom.opcode & 0x3F) << 1)) = gg_eeprom.buffer;
              }
              else
              {
                /* write 64 words */
                int i;
                for (i=0; i<64; i++)
                {
                  *(uint16 *)(sram.sram + (i << 1)) = gg_eeprom.buffer;

                }
              }
            }

            /* wait for next command */
            gg_eeprom.state = WAIT_STANDBY;
          }
          break;
        }

        case READ_WORD:
        {
          /* set DATA OUT */
          gg_eeprom.data = ((gg_eeprom.buffer >> (15 - gg_eeprom.cycles)) & 1);
          gg_eeprom.cycles++;

          if (gg_eeprom.cycles == 16)
          {
            /* read next word (93C46B) */
            gg_eeprom.opcode++;
            gg_eeprom.cycles = 0;
            gg_eeprom.buffer = *(uint16 *)(sram.sram + ((gg_eeprom.opcode & 0x3F) << 1));
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
    if (gg_eeprom.cs)
    {
      /* standby mode */
      gg_eeprom.data = 1;
      gg_eeprom.state = WAIT_START;
    }
  }

  /* Update input lines */
  gg_eeprom.cs  = (data >> BIT_CS) & 1;
  gg_eeprom.clk = (data >> BIT_CLK) & 1;
}

unsigned char gg_eeprom_read(void)
{
  return ((gg_eeprom.cs << BIT_CS) | (gg_eeprom.data << BIT_DATA) | (1 << BIT_CLK));
}

