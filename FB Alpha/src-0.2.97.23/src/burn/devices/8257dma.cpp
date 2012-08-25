/**********************************************************************

    8257 DMA interface and emulation

    For datasheet http://www.threedee.com/jcm/library/index.html

    2008/05     Miodrag Milanovic

        - added support for autoload mode
        - fixed bug in calculating count

    2007/11     couriersud

        - architecture copied from 8237 DMA
        - significant changes to implementation

    The DMA works like this:

    1.  The device asserts the DRQn line
    2.  The DMA clears the TC (terminal count) line
    3.  The DMA asserts the CPU's HRQ (halt request) line
    4.  Upon acknowledgement of the halt, the DMA will let the device
        know that it needs to send information by asserting the DACKn
        line
    5.  The DMA will read the byte from the device
    6.  The device clears the DRQn line
    7.  The DMA clears the CPU's HRQ line
    8.  (steps 3-7 are repeated for every byte in the chain)

**********************************************************************/

#include "burnint.h"
#include "driver.h"
#include "8257dma.h"

#define I8257_NUM_CHANNELS		(4)

#define I8257_STATUS_UPDATE		0x10
#define I8257_STATUS_TC_CH3		0x08
#define I8257_STATUS_TC_CH2		0x04
#define I8257_STATUS_TC_CH1		0x02
#define I8257_STATUS_TC_CH0		0x01

#define DMA_MODE_AUTOLOAD(mode)		((mode) & 0x80)
#define DMA_MODE_TCSTOP(mode)		((mode) & 0x40)
#define DMA_MODE_EXWRITE(mode)		((mode) & 0x20)
#define DMA_MODE_ROTPRIO(mode)		((mode) & 0x10)
#define DMA_MODE_CH_EN(mode, chan)	((mode) & (1 << (chan)))

#define TIMER_OPERATION			0
#define TIMER_MSBFLIP			1
#define TIMER_DRQ_SYNC			2

static void (*m_out_hrq_func)(INT32 line); // halt
static void (*m_out_tc_func)(INT32 line);
static void (*m_out_mark_func)(INT32 line);

static UINT8 (*m_in_memr_func)(UINT16 address);
static void (*m_out_memw_func)(UINT16 address, UINT8 data);
static ior_in_functs m_in_ior_func[I8257_NUM_CHANNELS];
static ior_out_functs m_out_iow_func[I8257_NUM_CHANNELS];
static INT32 (*m_idle_func)(INT32);

static UINT16 m_registers[I8257_NUM_CHANNELS*2];
static UINT16 m_address[I8257_NUM_CHANNELS];
static UINT16 m_count[I8257_NUM_CHANNELS];
static UINT8  m_rwmode[I8257_NUM_CHANNELS];
static UINT8  m_mode;
static UINT8  m_rr;
static UINT8  m_msb;
static UINT8  m_drq;
static UINT8  m_status;	/* bits  0- 3 :  Terminal count for channels 0-3 */

void i8257_update_status();

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

// fake functions to keep everything safe
static void null_line(INT32){}
static UINT8 null_in(UINT16){ return 0; }
static void null_out(UINT16,UINT8){}
static INT32 null_idle(INT32){return 0;}

static INT32 trigger_transfer = 0;

void i8257Init()
{
	// these aren't used atm.
	m_out_hrq_func  = null_line;
	m_out_tc_func   = null_line;
	m_out_mark_func = null_line;

	m_in_memr_func  = null_in;
	m_out_memw_func = null_out;

	m_idle_func = null_idle;

	for (INT32 i = 0; i < I8257_NUM_CHANNELS; i++) {
		m_in_ior_func[i] = null_in;
		m_out_iow_func[i] = null_out;
	}
}

void i8257Config(UINT8 (*cpuread)(UINT16), void (*cpuwrite)(UINT16,UINT8), INT32 (*idle)(INT32), ior_in_functs *read_f, ior_out_functs *write_f)
{
	m_in_memr_func  = cpuread;
	m_out_memw_func = cpuwrite;

	for (INT32 i = 0; i < I8257_NUM_CHANNELS; i++) {
		if (read_f != NULL) m_in_ior_func[i] = (read_f[i] != NULL) ? read_f[i] : null_in;
		if (write_f != NULL) m_out_iow_func[i] = (write_f[i] != NULL) ? write_f[i] : null_out;
	}

	if (idle != NULL) m_idle_func = idle;
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void i8257Reset()
{
	trigger_transfer = 0;
	m_status &= 0xf0;
	m_mode = 0;
	i8257_update_status();
}

static int i8257_do_operation(int channel)
{
	int done = 0;
	UINT8 data;

	UINT8 mode = m_rwmode[channel];

	if (m_count[channel] == 0x0000)
	{
		m_status |= (0x01 << channel);

		m_out_tc_func(ASSERT_LINE);
	}

	switch (mode)
	{
		case 0:
			m_address[channel]++;
			m_count[channel]--;
			done = (m_count[channel] == 0xFFFF);
		break;

		case 1:
			data = m_in_memr_func(m_address[channel]);
			m_out_iow_func[channel](m_address[channel], data);
			m_address[channel]++;
			m_count[channel]--;
			done = (m_count[channel] == 0xFFFF);
		break;

		case 2:
			data = m_in_ior_func[channel](m_address[channel]);
			m_out_memw_func(m_address[channel], data);
			m_address[channel]++;
			m_count[channel]--;
			done = (m_count[channel] == 0xFFFF);
		break;
	}

	if (done)
	{
		if ((channel==2) && DMA_MODE_AUTOLOAD(m_mode))
		{
			/* in case of autoload at the end channel 3 info is */
			/* copied to channel 2 info                         */
			m_registers[4] = m_registers[6];
			m_registers[5] = m_registers[7];
		}

		m_out_tc_func(CLEAR_LINE);
	}

	return done;
}

static void i8257_timer(INT32 id, int param)
{
	switch (id)
	{
		case TIMER_OPERATION:
		{
			int i, channel = 0, rr;
			int done;

			rr = DMA_MODE_ROTPRIO(m_mode) ? m_rr : 0;
			for (i = 0; i < I8257_NUM_CHANNELS; i++)
			{
				channel = (i + rr) % I8257_NUM_CHANNELS;
				if ((m_status & (1 << channel)) == 0)
				{
					if (m_mode & m_drq & (1 << channel))
					{
						break;
					}
				}
			}

			done = i8257_do_operation(channel);
			m_rr = (channel + 1) & 0x03;

			if (done)
			{
				m_drq &= ~(0x01 << channel);
				trigger_transfer = 1; // i8257_update_status();
				if (!(DMA_MODE_AUTOLOAD(m_mode) && channel==2))
				{
					if (DMA_MODE_TCSTOP(m_mode))
					{
						m_mode &= ~(0x01 << channel);
					}
				}
			}
			break;
		}

		case TIMER_MSBFLIP:
			m_msb ^= 1;
			break;

		case TIMER_DRQ_SYNC:
		{
			int channel = param >> 1;
			int state = param & 0x01;

			/* normalize state */
			if (state)
			{
				m_drq |= 0x01 << channel;
				m_address[channel] =  m_registers[channel * 2];
				m_count[channel] =  m_registers[channel * 2 + 1] & 0x3FFF;
				m_rwmode[channel] =  m_registers[channel * 2 + 1] >> 14;
				/* clear channel TC */
				m_status &= ~(0x01 << channel);
			}
			else
				m_drq &= ~(0x01 << channel);

			trigger_transfer = 1; // i8257_update_status();
			break;
		}
	}
}

void i8257_update_status()
{
	UINT16 pending_transfer;

	/* no transfer is active right now; is there a transfer pending right now? */
	pending_transfer = m_drq & (m_mode & 0x0F);

	while (pending_transfer)
	{
		m_idle_func(4);
		i8257_timer(TIMER_OPERATION, 0);

		pending_transfer = m_drq & (m_mode & 0x0F);
	}

	/* set the halt line */
	m_out_hrq_func(pending_transfer ? ASSERT_LINE : CLEAR_LINE);
}

static void i8257_prepare_msb_flip()
{
	i8257_timer(TIMER_MSBFLIP, 0);
}

UINT8 i8257Read(UINT8 offset)
{
	UINT8 data = 0xFF;

	switch (offset & 0x0f)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			/* DMA address/count register */
			data = ( m_registers[offset] >> (m_msb ? 8 : 0) ) & 0xFF;
			i8257_prepare_msb_flip();
		break;

		case 8:
			/* DMA status register */
			data = (UINT8) m_status;
			/* read resets status ! */
			m_status &= 0xF0;
		break;

		default:
			data = 0xFF;
		break;
	}

	return data;
}

void i8257Write(UINT8 offset, UINT8 data)
{
	switch (offset & 0x0f)
	{
		case 0:
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 6:
		case 7:
			/* DMA address/count register */
			if (m_msb)
			{
				m_registers[offset] |= ((UINT16) data) << 8;
			}
			else
			{
				m_registers[offset] = data;
			}
	
			if (DMA_MODE_AUTOLOAD(m_mode))
			{
				/* in case of autoload when inserting channel 2 info */
				/* it is automaticaly copied to channel 3 info       */
				switch(offset)
				{
					case 4:
					case 5:
						if (m_msb)
						{
							m_registers[offset+2] |= ((UINT16) data) << 8;
						}
						else
						{
							m_registers[offset+2] = data;
						}
				}
			}
	
			i8257_prepare_msb_flip();
		break;

		case 8:
			/* DMA mode register */
			m_mode = data;
		break;
	}
}

void i8257_drq_write(int channel, int state)
{
	int param = (channel << 1) | (state ? 1 : 0);

	i8257_timer(TIMER_DRQ_SYNC, param);
}

void i8257_do_transfer(int)
{
	if (trigger_transfer)
		i8257_update_status();
	trigger_transfer = 0;
}

void i8257Scan()
{
	for (INT32 i = 0; i < I8257_NUM_CHANNELS; i++) {
		SCAN_VAR(m_registers[i * 2 + 0]);
		SCAN_VAR(m_registers[i * 2 + 1]);

		SCAN_VAR(m_address[i]);
		SCAN_VAR(m_count[i]);
		SCAN_VAR(m_rwmode[i]);
	}

	SCAN_VAR(m_mode);
	SCAN_VAR(m_rr);
	SCAN_VAR(m_msb);
	SCAN_VAR(m_drq);
	SCAN_VAR(m_status);

	SCAN_VAR(trigger_transfer);
}
