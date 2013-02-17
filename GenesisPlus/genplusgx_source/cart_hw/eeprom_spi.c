/****************************************************************************
 *  Genesis Plus
 *  SPI Serial EEPROM (25xxx/95xxx) support
 *
 *  Copyright (C) 2012  Eke-Eke (Genesis Plus GX)
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

/* max supported size 64KB (25x512/95x512) */
#define SIZE_MASK 0xffff
#define PAGE_MASK 0x7f

/* hard-coded board implementation (!WP pin not used) */
#define BIT_DATA (0)
#define BIT_CLK  (1)
#define BIT_HOLD (2)
#define BIT_CS   (3)

typedef enum
{
  STANDBY,
  GET_OPCODE,
  GET_ADDRESS,
  WRITE_BYTE,
  READ_BYTE
} T_STATE_SPI;

typedef struct
{
  uint8 cs;           /* !CS line state */
  uint8 clk;          /* SCLK line state */
  uint8 out;          /* SO line state */
  uint8 status;       /* status register */
  uint8 opcode;       /* 8-bit opcode */
  uint8 buffer;       /* 8-bit data buffer */
  uint16 addr;        /* 16-bit address */
  uint32 cycles;      /* current operation cycle */
  T_STATE_SPI state;  /* current operation state */
} T_EEPROM_SPI;

static T_EEPROM_SPI spi_eeprom;

void eeprom_spi_init()
{
  /* reset eeprom state */
  memset(&spi_eeprom, 0, sizeof(T_EEPROM_SPI));
  spi_eeprom.out = 1;
  spi_eeprom.state = GET_OPCODE;

  /* enable backup RAM */
  sram.custom = 2;
  sram.on = 1;
}

void eeprom_spi_write(unsigned char data)
{
  /* Make sure !HOLD is high */
  if (data & (1 << BIT_HOLD))
  {
    /* Check !CS state */
    if (data & (1 << BIT_CS))
    {
      /* !CS high -> end of current operation */
      spi_eeprom.cycles = 0;
      spi_eeprom.out = 1;
      spi_eeprom.opcode = 0;
      spi_eeprom.state = GET_OPCODE;
    }
    else
    {
      /* !CS low -> process current operation */
      switch (spi_eeprom.state)
      {
        case GET_OPCODE:
        {
          /* latch data on CLK positive edge */
          if ((data & (1 << BIT_CLK)) && !spi_eeprom.clk)
          {
            /* 8-bit opcode buffer */
            spi_eeprom.opcode |= ((data >> BIT_DATA) & 1);
            spi_eeprom.cycles++;

            /* last bit ? */
            if (spi_eeprom.cycles == 8)
            {
              /* reset cycles count */
              spi_eeprom.cycles = 0;

              /* Decode instruction */
              switch (spi_eeprom.opcode)
              {
                case 0x01:
                {
                  /* WRITE STATUS */
                  spi_eeprom.buffer = 0;
                  spi_eeprom.state = WRITE_BYTE;
                  break;
                }

                case 0x02:
                {
                  /* WRITE BYTE */
                  spi_eeprom.addr = 0;
                  spi_eeprom.state = GET_ADDRESS;
                  break;
                }

                case 0x03:
                {
                  /* READ BYTE */
                  spi_eeprom.addr = 0;
                  spi_eeprom.state = GET_ADDRESS;
                  break;
                }

                case 0x04:
                {
                  /* WRITE DISABLE */
                  spi_eeprom.status &= ~0x02;
                  spi_eeprom.state = STANDBY;
                  break;
                }

                case 0x05:
                {
                  /* READ STATUS */
                  spi_eeprom.buffer = spi_eeprom.status;
                  spi_eeprom.state = READ_BYTE;
                  break;
                }

                case 0x06:
                {
                  /* WRITE ENABLE */
                  spi_eeprom.status |= 0x02;
                  spi_eeprom.state = STANDBY;
                  break;
                }

                default:
                {
                  /* specific instructions (not supported) */
                  spi_eeprom.state = STANDBY;
                  break;
                }
              }
            }
            else
            {
              /* shift opcode value */
              spi_eeprom.opcode = spi_eeprom.opcode << 1;
            }
          }
          break;
        }

        case GET_ADDRESS:
        {
          /* latch data on CLK positive edge */
          if ((data & (1 << BIT_CLK)) && !spi_eeprom.clk)
          {
            /* 16-bit address */
            spi_eeprom.addr |= ((data >> BIT_DATA) & 1);
            spi_eeprom.cycles++;

            /* last bit ? */
            if (spi_eeprom.cycles == 16)
            {
              /* reset cycles count */
              spi_eeprom.cycles = 0;

              /* mask unused address bits */
              spi_eeprom.addr &= SIZE_MASK;

              /* operation type */
              if (spi_eeprom.opcode & 0x01)
              {
                /* READ operation */
                spi_eeprom.buffer = sram.sram[spi_eeprom.addr];
                spi_eeprom.state = READ_BYTE;
              }
              else
              {
                /* WRITE operation */
                spi_eeprom.buffer = 0;
                spi_eeprom.state = WRITE_BYTE;
              }
            }
            else
            {
              /* shift address value */
              spi_eeprom.addr = spi_eeprom.addr << 1;
            }
          }
          break;
        }

        case WRITE_BYTE:
        {
          /* latch data on CLK positive edge */
          if ((data & (1 << BIT_CLK)) && !spi_eeprom.clk)
          {
            /* 8-bit data buffer */
            spi_eeprom.buffer |= ((data >> BIT_DATA) & 1);
            spi_eeprom.cycles++;

            /* last bit ? */
            if (spi_eeprom.cycles == 8)
            {
              /* reset cycles count */
              spi_eeprom.cycles = 0;

              /* write data to destination */
              if (spi_eeprom.opcode & 0x01)
              {
                /* update status register */
                spi_eeprom.status = (spi_eeprom.status & 0x02) | (spi_eeprom.buffer & 0x0c);

                /* wait for operation end */
                spi_eeprom.state = STANDBY;
              }
              else
              {
                /* Memory Array (write-protected) */
                if (spi_eeprom.status & 2)
                {
                  /* check array protection bits (BP0, BP1) */
                  switch ((spi_eeprom.status >> 2) & 0x03)
                  {
                    case 0x01:
                    {
                      /* $C000-$FFFF (sector #3) is protected */
                      if (spi_eeprom.addr < 0xC000)
                      {
                        sram.sram[spi_eeprom.addr] = spi_eeprom.buffer;
                      }
                      break;
                    }

                    case 0x02:
                    {
                      /* $8000-$FFFF (sectors #2 and #3) is protected */
                      if (spi_eeprom.addr < 0x8000)
                      {
                        sram.sram[spi_eeprom.addr] = spi_eeprom.buffer;
                      }
                      break;
                    }

                    case 0x03:
                    {
                      /* $0000-$FFFF (all sectors) is protected */
                      break;
                    }

                    default:
                    {
                      /* no sectors protected */
                      sram.sram[spi_eeprom.addr] = spi_eeprom.buffer;
                      break;
                    }
                  }
                }

                /* reset data buffer */
                spi_eeprom.buffer = 0;

                /* increase array address (sequential writes are limited within the same page) */
                spi_eeprom.addr = (spi_eeprom.addr & ~PAGE_MASK) | ((spi_eeprom.addr + 1) & PAGE_MASK);
              }
            }
            else
            {
              /* shift data buffer value */
              spi_eeprom.buffer = spi_eeprom.buffer << 1;
            }
          }
          break;
        }

        case READ_BYTE:
        {
          /* output data on CLK positive edge */
          if ((data & (1 << BIT_CLK)) && !spi_eeprom.clk)
          {
            /* read out bits */
            spi_eeprom.out = (spi_eeprom.buffer >> (7 - spi_eeprom.cycles)) & 1;
            spi_eeprom.cycles++;

            /* last bit ? */
            if (spi_eeprom.cycles == 8)
            {
              /* reset cycles count */
              spi_eeprom.cycles = 0;

              /* read from memory array ? */
              if (spi_eeprom.opcode == 0x03)
              {
                /* read next array byte */
                spi_eeprom.addr = (spi_eeprom.addr + 1) & SIZE_MASK;
                spi_eeprom.buffer = sram.sram[spi_eeprom.addr];
              }
            }
          }
          break;
        }

        default:
        {
          /* wait for !CS low->high transition */
          break;
        }
      }
    }
  }

  /* update input lines */
  spi_eeprom.cs  = (data >> BIT_CS) & 1;
  spi_eeprom.clk = (data >> BIT_CLK) & 1;
}

unsigned int eeprom_spi_read(unsigned int address)
{
  return (spi_eeprom.out << BIT_DATA);
}

