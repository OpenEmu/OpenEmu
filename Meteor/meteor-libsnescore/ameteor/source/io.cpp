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

#include "ameteor/io.hpp"
#include "ameteor/dma.hpp"
#include "globals.hpp"
#include "ameteor.hpp"
#include <cstring>

#include "debug.hpp"

#define W8(add, val) \
	m_iomem[(add) & 0xFFF] = (val)
#define W16(add, val) \
	*(uint16_t*)(m_iomem + ((add) & 0xFFF)) = (val)
#define W32(add, val) \
	*(uint32_t*)(m_iomem + ((add) & 0xFFF)) = (val)

namespace AMeteor
{
	Io::Io ()
	{
		m_iomem = new uint8_t[IO_SIZE];
		Reset ();
	}

	Io::~Io ()
	{
		delete [] m_iomem;
	}

	void Io::Reset ()
	{
		std::memset(m_iomem, 0, IO_SIZE);

		// TODO use clears intead

		// TODO DISPCNT should be 0x80 by default
		// TODO do it also for clears
		// TODO lcd should draw white lines when hblank forced
		// TODO when passing disabling hblank forced, lcd should start drawing from
		// first line
		W16(SOUNDBIAS, 0x0200); // default value
		W16(KEYINPUT, 0x03FF); // all keys released
		W8(HALTCNT, 0xFF); // normal mode (internal)
		W16(DISPSTAT, 0x0004); // vcount match (since default vcount is 0)
		W16(BG2PA, 0x0100);
		W16(BG2PD, 0x0100);
		W16(BG3PA, 0x0100);
		W16(BG3PD, 0x0100);
		W16(RCNT, 0x8000); // we start in general purpose mode
	}

	void Io::ClearSio ()
	{
		// TODO
		W16(RCNT, 0x8000);
	}

	void Io::ClearSound ()
	{
		// TODO
	}

	void Io::ClearOthers ()
	{
		// FIXME !! shouldn't we call Write*() ?
		// lcd
		for (uint8_t i = 0x0; i < 0x56; i += 2)
			Write16(i, 0x0000);
		// dma
		for (uint8_t i = 0xB0; i < 0xE0; i += 4)
			Write32(i, 0x0000);
		// FIXME : should timers be set to 0 too ? (vba does not)
		W8(HALTCNT, 0xFF); // normal mode (internal)
		W16(IE, 0x0000);
		W16(IF, 0x0000);
		W16(IME, 0x0000);
		Write16(WAITCNT, 0x0000);
		W16(BG2PA, 0x0100);
		W16(BG2PD, 0x0100);
		W16(BG3PA, 0x0100);
		W16(BG3PD, 0x0100);
	}

	// TODO implement unreadable or write-only io
	uint8_t Io::Read8 (uint32_t add)
	{
		//debug ("IO Read8 at " << IOS_ADD << add << " of " << IOS_ADD << (int)*(uint8_t*)(m_iomem + (add & 0xFFF)));
		if ((add & 0xFF0) == 0x100)
			switch (add & 0xF)
			{
				case 0x0:
				case 0x4:
				case 0x8:
				case 0xC:
					met_abort("Misaligned reading of timers");
			}
		return m_iomem[add & 0xFFF];
	}

	uint16_t Io::Read16 (uint32_t add)
	{
		//debug ("IO Read16 at " << IOS_ADD << add << " of " << IOS_ADD << *(uint16_t*)(m_iomem + (add & 0xFFF)));
		// special case, reading timers
		if ((add & 0xFF0) == 0x100)
			switch (add & 0xF)
			{
				case 0x0: return TIMER0.GetCount();
				case 0x4: return TIMER1.GetCount();
				case 0x8: return TIMER2.GetCount();
				case 0xC: return TIMER3.GetCount();
			}
		return *(uint16_t*)(m_iomem + (add & 0xFFF));
	}

	uint32_t Io::Read32 (uint32_t add)
	{
		//debug ("IO Read32 at " << IOS_ADD << add << " of " << IOS_ADD << *(uint32_t*)(m_iomem + (add & 0xFFF)));
		// special case, reading timers
		if ((add & 0xFF0) == 0x100)
			switch (add & 0xF)
			{
				case 0x0: return TIMER0.GetCount();
				case 0x4: return TIMER1.GetCount();
				case 0x8: return TIMER2.GetCount();
				case 0xC: return TIMER3.GetCount();
			}
		return *(uint32_t*)(m_iomem + (add & 0xFFF));
	}

	void Io::Write8 (uint32_t add, uint8_t val)
	{
		//debug ("IO Write8 at " << IOS_ADD << add << " of " << IOS_ADD << (int)val);
		switch (add & 0xFFF)
		{
			case NR10+1:
			case NR52+1:
			case NR52+2:
			case NR52+3:
				break;
			case NR10:
			case NR11:
			case NR13:
			case NR21:
			case NR23:
			case NR41:
			case NR43:
			case NR50:
			case NR51:
			case SOUNDCNT_H:
				W8(add, val);
				break;
			case NR12:
				W8(add, val);
				if (!(val & (0xF << 4)))
					SOUND.ResetSound1Envelope();
				break;
			case NR14:
				W8(add, val & 0xC7);
				if (val & (0x1 << 7))
					SOUND.ResetSound1();
				break;
			case NR22:
				W8(add, val);
				if (!(val & (0xF << 4)))
					SOUND.ResetSound2Envelope();
				break;
			case NR24:
				W8(add, val & 0xC7);
				if (val & (0x1 << 7))
					SOUND.ResetSound2();
				break;
			case NR42:
				W8(add, val);
				if (!(val & (0xF << 4)))
					SOUND.ResetSound4Envelope();
				break;
			case NR44:
				W8(add, val & 0xC7);
				if (val & (0x1 << 7))
					SOUND.ResetSound4();
				break;
			case SOUNDCNT_H+1:
				W8(add, val);
				SOUND.UpdateCntH1(val);
				break;
			case NR52:
				// this will also reset the sound on flags
				W8(add, val & 0x80);
				break;
			case POSTFLG:
				// FIXME is this right, i have no idea about why i should do that
				if (val)
					val &= 0xFE;
				W8(add, val);
				break;
			case HALTCNT:
				W8(add, val);
				break;
			default:
				//W8(add, val);
				// TODO make a function which will apply masks to IO memory and trigger
				// the update functions, this function will be called by write8 and
				// write16
#if 1
				add &= 0xFFF;
				if (add % 2)
					Write16(add & ~0x1, (val << 8) | m_iomem[add & ~0x1]);
				else
					Write16(add, (m_iomem[add | 0x1] << 8) | val);
#endif
				break;
		}
	}

	void Io::Write16 (uint32_t add, uint16_t val)
	{
		//debug ("IO Write16 at " << IOS_ADD << add << " of " << IOS_ADD << val);
		//*(uint16_t*)(m_iomem + (add & 0xFFF)) = val;

		switch (add & 0xFFF)
		{
			case KEYINPUT:
			case VCOUNT:
				break;
			case DMA0CNT_L:
			case DMA1CNT_L:
			case DMA2CNT_L:
			case DMA3CNT_L:
				//W16(add, val);
				DMA.SetReload(((add & 0xFFF) - DMA0CNT_L) / DMA_CHANSIZE, val);
				break;
			case KEYCNT:
				W16(add, val & 0xC3FF);
				break;
			case IME:
				W16(add, val & 0x0001);
				CPU.CheckInterrupt();
				break;
			case IE:
				W16(add, val & 0x3FFF);
				CPU.CheckInterrupt();
				break;
			case IF:
				*((uint16_t*)(m_iomem+IF)) ^= (val & (*((uint16_t*)(m_iomem+IF))));
				CPU.CheckInterrupt();
				break;
			case BG0CNT:
				W16(add, val & 0xFFCF);
				LCD.UpdateBg0Cnt(val & 0xFFCF);
				break;
			case BG1CNT:
				W16(add, val & 0xFFCF);
				LCD.UpdateBg1Cnt(val & 0xFFCF);
				break;
			case BG2CNT:
				W16(add, val & 0xFFCF);
				LCD.UpdateBg2Cnt(val & 0xFFCF);
				break;
			case BG3CNT:
				W16(add, val & 0xFFCF);
				LCD.UpdateBg3Cnt(val & 0xFFCF);
				break;
			case DISPSTAT:
				// the first 3 bits are read only and they are used by the lcd
				// NOTE : we are in LITTLE ENDIAN
				// FIXME : if vcount setting has changed to current vcount, we should
				// update the vcounter flag and eventually trigger an interrupt
				W16(add, (val & 0xFFF8) | (m_iomem[add & 0xFFF] & 0x07));
				break;
			// The BG*OFS are write-only, we don't need to W16()
			case BG0HOFS:
				LCD.UpdateBg0XOff(val & 0x1FF);
				break;
			case BG0VOFS:
				LCD.UpdateBg0YOff(val & 0x1FF);
				break;
			case BG1HOFS:
				LCD.UpdateBg1XOff(val & 0x1FF);
				break;
			case BG1VOFS:
				LCD.UpdateBg1YOff(val & 0x1FF);
				break;
			case BG2HOFS:
				LCD.UpdateBg2XOff(val & 0x1FF);
				break;
			case BG2VOFS:
				LCD.UpdateBg2YOff(val & 0x1FF);
				break;
			case BG3HOFS:
				LCD.UpdateBg3XOff(val & 0x1FF);
				break;
			case BG3VOFS:
				LCD.UpdateBg3YOff(val & 0x1FF);
				break;
			case BG2X_H:
				val &= 0x0FFF;
			case BG2X_L:
				W16(add, val);
				LCD.UpdateBg2RefX(IO.DRead32(Io::BG2X_L));
				break;
			case BG2Y_H:
				val &= 0x0FFF;
			case BG2Y_L:
				W16(add, val);
				LCD.UpdateBg2RefY(IO.DRead32(Io::BG2Y_L));
				break;
			case BG3X_H:
				val &= 0x0FFF;
			case BG3X_L:
				W16(add, val);
				LCD.UpdateBg3RefX(IO.DRead32(Io::BG3X_L));
				break;
			case BG3Y_H:
				val &= 0x0FFF;
			case BG3Y_L:
				W16(add, val);
				LCD.UpdateBg3RefY(IO.DRead32(Io::BG3Y_L));
				break;
			case WIN0H:
			case WIN1H:
			case WIN0V:
			case WIN1V:
			case WININ:
			case WINOUT:
				W16(add, val);
				break;
			case BLDCNT:
				W16(add, val);
				break;
			case MOSAIC:
				W16(add, val);
				break;
			case DISPCNT:
				W16(add, val);
				LCD.UpdateDispCnt(val);
				break;
			case DMA0CNT_H:
			case DMA1CNT_H:
			case DMA2CNT_H:
			case DMA3CNT_H:
				W16(add, val & 0xFFE0);
				DMA.UpdateCnt(((add & 0xFFF) - DMA0CNT_H) / DMA_CHANSIZE);
				break;
			case WAITCNT:
				W16(add, val & 0xDFFF);
				MEM.UpdateWaitStates (val & 0xDFFF);
				break;
			case SOUND1CNT_L:
			case SOUND1CNT_H:
			case SOUND1CNT_X:
			case SOUND2CNT_L:
			case SOUND2CNT_H:
			case SOUND4CNT_L:
			case SOUND4CNT_H:
			case SOUNDCNT_L:
			case SOUNDCNT_H:
			case SOUNDCNT_X:
			case POSTFLG:
				Write8(add, val & 0xFF);
				Write8(add + 1, val >> 8);
				break;
			case TM0CNT_L:
				TIMER0.SetReload(val);
				break;
			case TM1CNT_L:
				TIMER1.SetReload(val);
				break;
			case TM2CNT_L:
				TIMER2.SetReload(val);
				break;
			case TM3CNT_L:
				TIMER3.SetReload(val);
				break;
			case TM0CNT_H:
				W16(add, val & 0x00C7);
				TIMER0.Reload();
				break;
			case TM1CNT_H:
				W16(add, val & 0x00C7);
				TIMER1.Reload();
				break;
			case TM2CNT_H:
				W16(add, val & 0x00C7);
				TIMER2.Reload();
				break;
			case TM3CNT_H:
				W16(add, val & 0x00C7);
				TIMER3.Reload();
				break;
			default:
				//met_abort("Unknown IO at " << IOS_ADD << add);
				W16(add, val);
				break;
		}
	}

	void Io::Write32 (uint32_t add, uint32_t val)
	{
		//debug ("IO Write32 at " << IOS_ADD << add << " of " << IOS_ADD << val);
		switch (add & 0xFF)
		{
			case DMA1DAD:
			case DMA0SAD:
			case DMA1SAD:
			case DMA2SAD:
			case DMA3SAD:
			case DMA0DAD:
			case DMA2DAD:
			case DMA3DAD:
				W32(add, val);
				break;
			case BG0HOFS:
			case BG1HOFS:
			case BG2HOFS:
			case BG3HOFS:
				Write16(add, val & 0xFFFF);
				Write16(add+2, val >> 16);
				break;
			case BG2X_L:
				W32(add, val & 0x0FFFFFFF);
				LCD.UpdateBg2RefX(IO.DRead32(Io::BG2X_L));
				break;
			case BG2Y_L:
				W32(add, val & 0x0FFFFFFF);
				LCD.UpdateBg2RefY(IO.DRead32(Io::BG2Y_L));
				break;
			case BG3X_L:
				W32(add, val & 0x0FFFFFFF);
				LCD.UpdateBg3RefX(IO.DRead32(Io::BG3X_L));
				break;
			case BG3Y_L:
				W32(add, val & 0x0FFFFFFF);
				LCD.UpdateBg3RefY(IO.DRead32(Io::BG3Y_L));
				break;
			case BG2PA:
			case BG2PC:
			case BG3PA:
			case BG3PC:
			case WIN0H:
			case WIN0V:
			case WININ:
				Write16(add, val & 0xFFFF);
				Write16(add+2, val >> 16);
				break;
			case DMA0CNT_L:
			case DMA1CNT_L:
			case DMA2CNT_L:
			case DMA3CNT_L:
				Write16(add, val & 0xFFFF);
				Write16(add+2, val >> 16);
				break;
			case FIFO_A:
			case FIFO_B:
				// TODO
				break;
			default:
				//met_abort("Unknown IO at " << IOS_ADD << add);
				//*(uint32_t*)(m_iomem + (add & 0xFFF)) = val;
				Write16(add, val & 0xFFFF);
				Write16(add+2, val >> 16);
				break;
		}
	}

	bool Io::SaveState (std::ostream& stream)
	{
		SS_WRITE_DATA(m_iomem, IO_SIZE);

		return true;
	}

	bool Io::LoadState (std::istream& stream)
	{
		SS_READ_DATA(m_iomem, IO_SIZE);

		return true;
	}
}
