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

#include "ameteor/lcd.hpp"
#include "ameteor/io.hpp"
#include "globals.hpp"
#include "ameteor.hpp"

#include "debug.hpp"

namespace AMeteor
{
	Lcd::Lcd () :
		m_screen(MEM, IO)
	{
		Reset();
	}

	void Lcd::Reset ()
	{
		CLOCK.AddLcd(960); // call me at first H-Blank
	}

	void Lcd::TimeEvent ()
	{
		uint16_t& dispstat = IO.GetRef16(Io::DISPSTAT);
		uint16_t& vcount = IO.GetRef16(Io::VCOUNT);

		if (dispstat & 0x2) // if we were H-Blanking
		{
			debug("hblank end");
			// we are not anymore, we're on next line
			dispstat ^= 0x2;
			// call me when we are H-Blanking again
			CLOCK.AddLcd(960);

			// we have finished drawing a line, do our stuff...
			if (vcount == 227) // this was the last line
			{
				vcount = 0; // we're now at first
				// we reload the reference points
				m_screen.UpdateBg2RefX(IO.DRead32(Io::BG2X_L));
				m_screen.UpdateBg2RefY(IO.DRead32(Io::BG2Y_L));
				m_screen.UpdateBg3RefX(IO.DRead32(Io::BG3X_L));
				m_screen.UpdateBg3RefY(IO.DRead32(Io::BG3Y_L));
				// and we draw the line 0
				m_screen.DrawLine(0);
				// FIXME see below, vblank finished
				dispstat ^= 0x1;
			}
			else
			{
				++vcount; // we're on next line
				if (vcount < 160) // we draw normally
					m_screen.DrawLine(vcount);
				else if (vcount == 160) // We enter V-Blank
				{
					dispstat |= 0x1;
					if (dispstat & (0x1 << 3)) // if V-Blank irq is enabled
						CPU.SendInterrupt(0x1);
					DMA.CheckAll(Dma::VBlank);

					KEYPAD.VBlank();

					// we send the vblank signal
					sig_vblank.emit();
				}
				// NOTE : v-blank finishes on line 227, not 0
				// FIXME on vba, it finishes on 0
				//if (vcount == 227) // V-Blank finished
					//dispstat ^= 0x1;
			}

			// check for vcount match
			if (vcount == ((dispstat >> 8) & 0xFF)) // vcount match
			{
				dispstat |= 0x4; // enable vcount match bit
				if (dispstat & (0x1 << 5)) // if V-Counter irq is enabled
					CPU.SendInterrupt(0x4);
			}
			else // no vcount match
				dispstat &= ~(uint16_t)0x4;
		}
		else // if we were not H-Blanking
		{
			debug("hblank vcount : " << std::dec << vcount);
			// now, we are
			dispstat |= 0x2;
			// call me when we are not H-Blanking anymore
			CLOCK.AddLcd(272);

			// NOTE : H-Blank interrupts are not generated during V-Blank
			// FIXME vba generates hblank interrupts even during vblank
			// if H-Blank irq is enabled //and we're not in V-Blank
			if ((dispstat & 0x10) == 0x10)
				CPU.SendInterrupt(0x2);
			// NOTE : hblank DMAs are not triggered during vblank
			// (seen on vba)
			if (!(dispstat & 0x1))
				// if we're not vblanking
				DMA.CheckAll(Dma::HBlank);
		}
	}

	bool Lcd::SaveState (std::ostream& stream)
	{
		if (!m_screen.SaveState(stream))
			return false;

		return true;
	}

	bool Lcd::LoadState (std::istream& stream)
	{
		if (!m_screen.LoadState(stream))
			return false;

		UpdateDispCnt (IO.DRead16(Io::DISPCNT));
		UpdateBg0Cnt (IO.DRead16(Io::BG0CNT));
		UpdateBg1Cnt (IO.DRead16(Io::BG1CNT));
		UpdateBg2Cnt (IO.DRead16(Io::BG2CNT));
		UpdateBg3Cnt (IO.DRead16(Io::BG3CNT));
		UpdateBg0XOff (IO.DRead16(Io::BG0HOFS));
		UpdateBg0YOff (IO.DRead16(Io::BG0VOFS));
		UpdateBg1XOff (IO.DRead16(Io::BG1HOFS));
		UpdateBg1YOff (IO.DRead16(Io::BG1VOFS));
		UpdateBg2XOff (IO.DRead16(Io::BG2HOFS));
		UpdateBg2YOff (IO.DRead16(Io::BG2VOFS));
		UpdateBg3XOff (IO.DRead16(Io::BG3HOFS));
		UpdateBg3YOff (IO.DRead16(Io::BG3VOFS));
		OamWrite (0x07000000, 0x07000400);

		return true;
	}
}
