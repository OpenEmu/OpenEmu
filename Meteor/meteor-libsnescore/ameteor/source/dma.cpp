// Meteor - A Nintendo Gameboy Advance emulator
// Copyright (C) 2009-2011 Philippe Daouadi
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "ameteor/dma.hpp"
#include "ameteor/io.hpp"
#include "ameteor/memory.hpp"
#include "globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

namespace AMeteor
{
	void Dma::Reset ()
	{
		for (Channel* chan = m_chans; chan < m_chans+4; ++chan)
		{
			chan->src = 0;
			chan->dest = 0;
			chan->count = 0;
			chan->control.w = 0;
		}
		m_graphic = 0;
	}

	void Dma::UpdateCnt (uint8_t channum)
	{
		Channel& chan = m_chans[channum];

		if (chan.control.w !=
				IO.DRead16(Io::DMA0CNT_H + channum * Io::DMA_CHANSIZE))
		{
			if (!chan.control.b.enable &&
					(IO.DRead16(Io::DMA0CNT_H + channum * Io::DMA_CHANSIZE)
					 & (0x1 << 15)))
				// if we changed enable from 0 to 1
			{
				chan.dest = IO.DRead32(Io::DMA0DAD + channum * Io::DMA_CHANSIZE);
				if (channum == 3)
					chan.dest &= 0x0FFFFFFF;
				else
					chan.dest &= 0x07FFFFFF;

				chan.src = IO.DRead32(Io::DMA0SAD + channum * Io::DMA_CHANSIZE);
				if (channum == 0)
					chan.src &= 0x07FFFFFF;
				else
					chan.src &= 0x0FFFFFFF;

				chan.count = chan.reload;
				if (channum != 3)
					chan.count &= 0x3FFF;

				chan.control.w =
					IO.DRead16(Io::DMA0CNT_H + channum * Io::DMA_CHANSIZE);

				Check(channum, Immediately);
			}
			else
				chan.control.w =
					IO.DRead16(Io::DMA0CNT_H + channum * Io::DMA_CHANSIZE);

			if (chan.control.b.start == Special)
			{
				switch (channum)
				{
					case 0:
						met_abort("prohibited !");
						break;
					case 1:
					case 2:
						// sound dma
						if (chan.dest != 0x040000A0 && chan.dest != 0x040000A4)
							met_abort("Special DMA 1 or 2 with unauthorized address : "
									<< IOS_ADD << chan.dest);
						if (!chan.control.b.repeat)
							met_abort("Special DMA 1 or 2 without repeat");

						// 4 words of 32 bits
						chan.count = 4;
						chan.control.b.type = 1;
						// no increment
						chan.control.b.dest = 2;
						break;
					case 3:
						met_abort("not implemented");
						break;
				}
			}
		}
	}

	void Dma::Check (uint8_t channum, uint8_t reason)
	{
		register Channel::Control cnt = m_chans[channum].control;

		if (cnt.b.enable &&
				cnt.b.start == reason)
			Process(channum);
	}

	void Dma::Process (uint8_t channum)
	{
		Channel& chan = m_chans[channum];

		int8_t s_inc, d_inc;
		s_inc = d_inc = 2; // increment or increment reload

		if (chan.control.b.src == 1) // decrement
			s_inc = -2;
		else if (chan.control.b.src == 2) // fixed
			s_inc = 0;
		else if (chan.control.b.src == 3)
			met_abort("prohibited");

		if (chan.control.b.dest == 1)
			d_inc = -2;
		else if (chan.control.b.dest == 2)
			d_inc = 0;
		//else if (chan.control.b.dest == 3)
		// same as 0, but we do something at the end

		if (chan.control.b.type) // 32 bits transfer
		{
			s_inc <<= 1;
			d_inc <<= 1;
		}

		if (chan.count == 0)
		{
			if (channum != 3)
				chan.count = 0x4000;
			// 0x10000 doesn't fill in 16 bits, we treat this case in the Copy() call
		}

		//printf("DMA%hhu from %08X to %08X of %hu %s\n", channum, chan.src, chan.dest, chan.count, chan.control.b.type ? "words":"halfwords");
		//if (i > debut)
		debug ("DMA" << IOS_NOR << (int)channum << ", from " << IOS_ADD << chan.src
				<< " to " << IOS_ADD << chan.dest
				<< " of " << IOS_NOR << (chan.count ? chan.count : 0x10000)
				<< (chan.control.b.type ? " words" : " halfwords"));
#if 0
		if (channum == 3 && (chan.dest >> 24) == 0x0D || (chan.src >> 24) == 0x0D)
		{
			if (chan.control.b.type)
				met_abort("Word copy for EEPROM DMA3");
			if (d_inc != 2 || s_inc != 2)
				met_abort("Source or destination not incremeting in EEPROM DMA3");
			if ((chan.dest >> 24) == 0x0D)
				MEM.WriteEepromDma(chan.src, chan.count ? chan.count : 0x10000);
			else if ((chan.src >> 24) == 0x0D)
				MEM.ReadEepromDma(chan.dest, chan.count ? chan.count : 0x10000);
			chan.src += chan.count * 2;
			chan.dest += chan.count * 2;
		}
		else
#endif
		if (channum == 3 && (chan.dest >> 24) == 0x0D)
		{
			if (chan.control.b.type)
				met_abort("Word copy for EEPROM DMA3");
			if (d_inc != 2 || s_inc != 2)
				met_abort("Source or destination not incremeting in EEPROM DMA3");
			MEM.WriteEepromDma(chan.src, chan.count);
			chan.src += chan.count * 2;
			chan.dest += chan.count * 2;
		}
		else
			Copy(chan.src, chan.dest, s_inc, d_inc,
					chan.count ? chan.count : 0x10000, chan.control.b.type);

		if (chan.control.b.type)
		{
			CYCLES32NSeq(chan.src, chan.count);
			CYCLES32NSeq(chan.dest, chan.count);
			ICYCLES(2);
		}
		else
		{
			CYCLES16NSeq(chan.src, chan.count);
			CYCLES16NSeq(chan.dest, chan.count);
			ICYCLES(2);
		}
		uint32_t d = chan.dest >> 24;
		uint32_t s = chan.src >> 24;
		if (d >= 0x08 && d <= 0x0D && s >= 0x08 && s <= 0x0D)
			// if both source and destination are in GamePak
			ICYCLES(2);

		if (chan.control.b.irq)
			CPU.SendInterrupt(0x1 << (8 + channum));

		if (chan.control.b.dest == 3)
		{
			chan.dest = IO.DRead32(Io::DMA0DAD + channum * Io::DMA_CHANSIZE);
			if (channum == 3)
				chan.dest &= 0x0FFFFFFF;
			else
				chan.dest &= 0x07FFFFFF;
		}

		// if repeat but not sound dma
		//if (!((channum == 1 || channum == 2) && chan.control.b.start == Special) && chan.control.b.repeat)
		if (((channum != 1 && channum != 2) || chan.control.b.start != Special)
				&& chan.control.b.repeat)
		{
			chan.count = chan.reload;
		}

		if (!chan.control.b.repeat || chan.control.b.start == Immediately)
		{
			chan.control.b.enable = 0;
			IO.GetRef16(Io::DMA0CNT_H + channum * Io::DMA_CHANSIZE) &= 0x7FFF;
		}
	}

	void Dma::Copy (uint32_t& src, uint32_t& dest, int8_t s_inc, int8_t d_inc,
			uint32_t count, bool word)
	{
		uint32_t basedest = dest;

		if (word)
		{
			src &= 0xFFFFFFFC;
			dest &= 0xFFFFFFFC;
		}
		else
		{
			src &= 0xFFFFFFFE;
			dest &= 0xFFFFFFFE;
		}

		// sound
		if (dest == 0x040000A0)
		{
			SOUND.SendDigitalA((uint8_t*)MEM.GetRealAddress(src));
			src += 4*4;
			if (d_inc != 0)
				met_abort("dinc != 0 on dma sound, should not happen");
			return;
		}
		if (dest == 0x040000A4)
		{
			SOUND.SendDigitalB((uint8_t*)MEM.GetRealAddress(src));
			src += 4*4;
			if (d_inc != 0)
				met_abort("dinc != 0 on dma sound, should not happen");
			return;
		}

		if ((dest >> 24) >= 0x05 &&
				(dest >> 24) <= 0x07)
			m_graphic = true;

		if (word)
		{
			for (uint32_t cur = 0; cur < count; ++cur)
			{
				MEM.Write32(dest, MEM.Read32(src));
				src += s_inc;
				dest += d_inc;
			}
		}
		else
		{
			for (uint32_t cur = 0; cur < count; ++cur)
			{
				MEM.Write16(dest, MEM.Read16(src));
				src += s_inc;
				dest += d_inc;
			}
		}

		m_graphic = false;
		if ((basedest >> 24) == 0x07)
			LCD.OamWrite(basedest, dest);
	}

	bool Dma::SaveState (std::ostream& stream)
	{
		SS_WRITE_ARRAY(m_chans);
		// no need to save or load m_graphic since we shouldn't save or load during
		// a dma

		return true;
	}

	bool Dma::LoadState (std::istream& stream)
	{
		SS_READ_ARRAY(m_chans);

		return true;
	}
}
