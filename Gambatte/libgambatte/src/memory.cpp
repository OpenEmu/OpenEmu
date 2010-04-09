/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aam�s                                    *
 *   aamas@stud.ntnu.no                                                    *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License version 2 as     *
 *   published by the Free Software Foundation.                            *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License version 2 for more details.                *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   version 2 along with this program; if not, write to the               *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#include "memory.h"
#include "video.h"
#include "sound.h"
#include "inputstate.h"
#include "inputstategetter.h"
#include "savestate.h"
#include "file/file.h"
#include <cstring>

// static const uint32_t timaClock[4]={ 1024, 16, 64, 256 };
static const unsigned char timaClock[4] = { 10, 4, 6, 8 };

Memory::Memory(const Interrupter &interrupter_in) :
memchunk(NULL),
rambankdata(NULL),
rdisabled_ram(NULL),
wdisabled_ram(NULL),
oamDmaSrc(NULL),
vrambank(vram),
rsrambankptr(NULL),
wsrambankptr(NULL),
getInput(NULL),
div_lastUpdate(0),
tima_lastUpdate(0),
next_timatime(COUNTER_DISABLED),
next_blittime(144*456ul),
nextIntTime(COUNTER_DISABLED),
minIntTime(0),
next_dmatime(COUNTER_DISABLED),
next_hdmaReschedule(COUNTER_DISABLED),
next_unhalttime(COUNTER_DISABLED),
next_endtime(0),
tmatime(COUNTER_DISABLED),
next_serialtime(COUNTER_DISABLED),
lastOamDmaUpdate(COUNTER_DISABLED),
nextOamEventTime(COUNTER_DISABLED),
display(ioamhram, vram),
interrupter(interrupter_in),
romtype(plain),
rombanks(1),
rombank(1),
dmaSource(0),
dmaDestination(0),
rambank(0),
rambanks(1),
oamDmaArea1Lower(0),
oamDmaArea1Width(0),
oamDmaArea2Upper(0),
oamDmaPos(0xFE),
cgb(false),
doubleSpeed(false),
IME(false),
enable_ram(false),
rambank_mode(false),
battery(false),
rtcRom(false),
hdma_transfer(false),
active(false)
{
	romdata[1] = romdata[0] = NULL;
	wramdata[1] = wramdata[0] = NULL;
	std::fill_n(rmem, 0x10, static_cast<unsigned char*>(NULL));
	std::fill_n(wmem, 0x10, static_cast<unsigned char*>(NULL));
	set_irqEvent();
	set_event();
}

void Memory::setStatePtrs(SaveState &state) {
	state.mem.vram.set(vram, sizeof vram);
	state.mem.sram.set(rambankdata, rambanks * 0x2000ul);
	state.mem.wram.set(wramdata[0], isCgb() ? 0x8000 : 0x2000);
	state.mem.ioamhram.set(ioamhram, sizeof ioamhram);

	display.setStatePtrs(state);
	sound.setStatePtrs(state);
}

unsigned long Memory::saveState(SaveState &state, unsigned long cycleCounter) {
	cycleCounter = resetCounters(cycleCounter);
	nontrivial_ff_read(0xFF0F, cycleCounter);
	nontrivial_ff_read(0xFF26, cycleCounter);

	state.mem.div_lastUpdate = div_lastUpdate;
	state.mem.tima_lastUpdate = tima_lastUpdate;
	state.mem.tmatime = tmatime;
	state.mem.next_serialtime = next_serialtime;
	state.mem.lastOamDmaUpdate = lastOamDmaUpdate;
	state.mem.minIntTime = minIntTime;
	state.mem.rombank = rombank;
	state.mem.dmaSource = dmaSource;
	state.mem.dmaDestination = dmaDestination;
	state.mem.rambank = rambank;
	state.mem.oamDmaPos = oamDmaPos;
	state.mem.IME = IME;
	state.mem.enable_ram = enable_ram;
	state.mem.rambank_mode = rambank_mode;
	state.mem.hdma_transfer = hdma_transfer;

	rtc.saveState(state);
	display.saveState(state);
	sound.saveState(state);

	return cycleCounter;
}

void Memory::loadState(const SaveState &state, const unsigned long oldCc) {
	sound.loadState(state);
	display.loadState(state, state.mem.oamDmaPos < 0xA0 ? rdisabled_ram : ioamhram);
	rtc.loadState(state, rtcRom ? state.mem.enable_ram : false);

	div_lastUpdate = state.mem.div_lastUpdate;
	tima_lastUpdate = state.mem.tima_lastUpdate;
	tmatime = state.mem.tmatime;
	next_serialtime = state.mem.next_serialtime;
	lastOamDmaUpdate = state.mem.lastOamDmaUpdate;
	minIntTime = state.mem.minIntTime;
	rombank = state.mem.rombank & (rombanks - 1);
	dmaSource = state.mem.dmaSource;
	dmaDestination = state.mem.dmaDestination;
	rambank = state.mem.rambank & (rambanks - 1);
	oamDmaPos = state.mem.oamDmaPos;
	IME = state.mem.IME;
	enable_ram = state.mem.enable_ram;
	rambank_mode = state.mem.rambank_mode;
	hdma_transfer = state.mem.hdma_transfer;

	const bool oldDs = doubleSpeed;
	doubleSpeed = isCgb() & ioamhram[0x14D] >> 7;
	oamDmaArea2Upper = oamDmaArea1Width = oamDmaArea1Lower = 0;
	vrambank = vram + (ioamhram[0x14F] & 0x01 & isCgb()) * 0x2000;
	wramdata[1] = wramdata[0] + ((isCgb() && (ioamhram[0x170] & 0x07)) ? (ioamhram[0x170] & 0x07) : 1) * 0x1000;
	std::fill_n(rmem, 0x10, static_cast<unsigned char*>(NULL));
	std::fill_n(wmem, 0x10, static_cast<unsigned char*>(NULL));
	setBanks();

	if (lastOamDmaUpdate != COUNTER_DISABLED) {
		oamDmaInitSetup();

		unsigned oamEventPos = 0x100;

		if (oamDmaPos < 0xA0) {
			setOamDmaArea();
			oamEventPos = 0xA0;
		}

		nextOamEventTime = lastOamDmaUpdate + (oamEventPos - oamDmaPos) * 4;
		setOamDmaSrc();
	}

	if (!IME && state.cpu.halted)
		schedule_unhalt();

	next_blittime = (ioamhram[0x140] & 0x80) ? display.nextMode1IrqTime() : static_cast<unsigned long>(COUNTER_DISABLED);

	const unsigned long cycleCounter = state.cpu.cycleCounter;

	if (hdma_transfer) {
		next_dmatime = display.nextHdmaTime(cycleCounter);
		next_hdmaReschedule = display.nextHdmaTimeInvalid();
	} else {
		next_hdmaReschedule = next_dmatime = COUNTER_DISABLED;
	}

	next_timatime = (ioamhram[0x107] & 4) ? tima_lastUpdate + ((256u - ioamhram[0x105]) << timaClock[ioamhram[0x107] & 3]) + 1 : static_cast<unsigned long>(COUNTER_DISABLED);
	set_irqEvent();
	rescheduleIrq(cycleCounter);

	if (oldDs != isDoubleSpeed())
		next_endtime = cycleCounter - (isDoubleSpeed() ?( oldCc - next_endtime) << 1 :( oldCc - next_endtime) >> 1);
	else
		next_endtime = cycleCounter - (oldCc - next_endtime);

// 	set_event();
}

void Memory::schedule_unhalt() {
	next_unhalttime = std::min(next_irqEventTime, display.nextIrqEvent());

	if (next_unhalttime != COUNTER_DISABLED)
		next_unhalttime += isCgb() * 4;

	set_event();
}

void Memory::rescheduleIrq(const unsigned long cycleCounter) {
	if (IME) {
		ioamhram[0x10F] |= display.getIfReg(cycleCounter) & 3;

		nextIntTime = (ioamhram[0x10F] & ioamhram[0x1FF] & 0x1F) ? cycleCounter : std::min(next_irqEventTime, display.nextIrqEvent());

		if (nextIntTime < minIntTime)
			nextIntTime = minIntTime;

		set_event();
	}
}

void Memory::rescheduleHdmaReschedule() {
	if (hdma_transfer && (ioamhram[0x140] & 0x80)) {
		const unsigned long newTime = display.nextHdmaTimeInvalid();

		if (newTime < next_hdmaReschedule) {
			next_hdmaReschedule = newTime;

			if (newTime < next_eventtime) {
				next_eventtime = newTime;
				next_event = HDMA_RESCHEDULE;
			}
		}
	}
}

void Memory::ei(const unsigned long cycleCounter) {
	IME = 1;
	minIntTime = cycleCounter + 1;
	rescheduleIrq(cycleCounter);
}

void Memory::incEndtime(const unsigned long inc) {
	active = true;
	next_endtime += inc << isDoubleSpeed();
	set_event();
}

void Memory::setEndtime(const unsigned long cycleCounter, const unsigned long inc) {
	next_endtime = cycleCounter;
	incEndtime(inc);
}

void Memory::set_irqEvent() {
	next_irqEventTime = next_timatime;
	next_irqEvent = TIMA;

	if (next_serialtime < next_irqEventTime) {
		next_irqEvent = SERIAL;
		next_irqEventTime = next_serialtime;
	}
}

void Memory::update_irqEvents(const unsigned long cc) {
	while (next_irqEventTime <= cc) {
		switch (next_irqEvent) {
		case TIMA:
			ioamhram[0x10F] |= 4;
			next_timatime += (256u - ioamhram[0x106]) << timaClock[ioamhram[0x107] & 3];
			break;
		case SERIAL:
			next_serialtime = COUNTER_DISABLED;
			ioamhram[0x101] = 0xFF;
			ioamhram[0x102] &= 0x7F;
			ioamhram[0x10F] |= 8;
			break;
		}

		set_irqEvent();
	}
}

void Memory::set_event() {
	next_event = INTERRUPTS;
	next_eventtime = nextIntTime;
	if (next_hdmaReschedule < next_eventtime) {
		next_eventtime = next_hdmaReschedule;
		next_event = HDMA_RESCHEDULE;
	}
	if (next_dmatime < next_eventtime) {
		next_eventtime = next_dmatime;
		next_event = DMA;
	}
	if (next_unhalttime < next_eventtime) {
		next_eventtime = next_unhalttime;
		next_event = UNHALT;
	}
	if (nextOamEventTime < next_eventtime) {
		next_eventtime = nextOamEventTime;
		next_event = OAM;
	}
	if (next_blittime < next_eventtime) {
		next_event = BLIT;
		next_eventtime = next_blittime;
	}
	if (next_endtime < next_eventtime) {
		next_eventtime = next_endtime;
		next_event = END;
	}
}

unsigned long Memory::event(unsigned long cycleCounter) {
	if (lastOamDmaUpdate != COUNTER_DISABLED)
		updateOamDma(cycleCounter);

	switch (next_event) {
	case HDMA_RESCHEDULE:
// 		printf("hdma_reschedule\n");
		next_dmatime = display.nextHdmaTime(cycleCounter);
		next_hdmaReschedule = display.nextHdmaTimeInvalid();
		break;
	case DMA:
// 		printf("dma\n");
		{
			const bool doubleSpeed = isDoubleSpeed();
			unsigned dmaSrc = dmaSource;
			unsigned dmaDest = dmaDestination;
			unsigned dmaLength = ((ioamhram[0x155] & 0x7F) + 0x1) * 0x10;

			unsigned length = hdma_transfer ? 0x10 : dmaLength;

			if ((static_cast<unsigned long>(dmaDest) + length) & 0x10000) {
				length = 0x10000 - dmaDest;
				ioamhram[0x155] |= 0x80;
			}

			dmaLength -= length;

			if (!(ioamhram[0x140] & 0x80))
				dmaLength = 0;

			{
				unsigned long lOamDmaUpdate = lastOamDmaUpdate;
				lastOamDmaUpdate = COUNTER_DISABLED;

				while (length--) {
					const unsigned src = dmaSrc++ & 0xFFFF;
					const unsigned data = ((src & 0xE000) == 0x8000 || src > 0xFDFF) ? 0xFF : read(src, cycleCounter);

					cycleCounter += 2 << doubleSpeed;

					if (cycleCounter - 3 > lOamDmaUpdate) {
						oamDmaPos = (oamDmaPos + 1) & 0xFF;
						lOamDmaUpdate += 4;

						if (oamDmaPos < 0xA0) {
							if (oamDmaPos == 0)
								startOamDma(lOamDmaUpdate - 2);

							ioamhram[src & 0xFF] = data;
						} else if (oamDmaPos == 0xA0) {
							endOamDma(lOamDmaUpdate - 2);
							lOamDmaUpdate = COUNTER_DISABLED;
						}
					}

					nontrivial_write(0x8000 | (dmaDest++ & 0x1FFF), data, cycleCounter);
				}

				lastOamDmaUpdate = lOamDmaUpdate;
			}

			cycleCounter += 4;

			dmaSource = dmaSrc;
			dmaDestination = dmaDest;
			ioamhram[0x155] = ((dmaLength / 0x10 - 0x1) & 0xFF) | (ioamhram[0x155] & 0x80);

			if (ioamhram[0x155] & 0x80) {
				next_hdmaReschedule = next_dmatime = COUNTER_DISABLED;
				hdma_transfer = 0;
			}

			if (hdma_transfer) {
				if (lastOamDmaUpdate != COUNTER_DISABLED)
					updateOamDma(cycleCounter);

				next_dmatime = display.nextHdmaTime(cycleCounter);
			}
		}

		break;
	case INTERRUPTS:
// 		printf("interrupts\n");
		update_irqEvents(cycleCounter);
		ioamhram[0x10F] |= display.getIfReg(cycleCounter) & 3;

		{
			/*unsigned interrupt = ioamhram[0x10F] & ioamhram[0x1FF];
			interrupt |= interrupt << 1;
			interrupt |= interrupt << 2;
			interrupt |= interrupt << 1;
			interrupt = ~interrupt;
			++interrupt;
			interrupt &= 0x1F;

			if (interrupt) {
				ioamhram[0x10F] &= ~interrupt;
				display.setIfReg(ioamhram[0x10F], CycleCounter);
				IME = false;

				unsigned address = interrupt;
				interrupt >>= 1;
				address -= interrupt & 0x0C;
				interrupt >>= 1;
				address -= interrupt & 5;
				address += interrupt >> 2;

				address <<= 3;
				address += 0x38;

				z80.interrupt(address);
			}*/

			const unsigned interrupt = ioamhram[0x10F] & ioamhram[0x1FF] & 0x1F;

			if (interrupt) {
				unsigned n;
				unsigned address;

				if ((n = interrupt & 0x01))
					address = 0x40;
				else if ((n = interrupt & 0x02))
					address = 0x48;
				else if ((n = interrupt & 0x04))
					address = 0x50;
				else if ((n = interrupt & 0x08))
					address = 0x58;
				else {
					n = 0x10;
					address = 0x60;
				}

				ioamhram[0x10F] &= ~n;
				display.setIfReg(ioamhram[0x10F], cycleCounter);
				IME = false;
				cycleCounter = interrupter.interrupt(address, cycleCounter, *this);
			}
		}

		nextIntTime = IME ? std::min(next_irqEventTime, display.nextIrqEvent()) : static_cast<unsigned long>(COUNTER_DISABLED);
		break;
	case BLIT:
// 		printf("blit\n");
		display.updateScreen(next_blittime);

		if (ioamhram[0x140] & 0x80)
			next_blittime += 70224 << isDoubleSpeed();
		else
			next_blittime = COUNTER_DISABLED;

		break;
	case UNHALT:
// 		printf("unhalt\n");
		update_irqEvents(cycleCounter);
		ioamhram[0x10F] |= display.getIfReg(cycleCounter) & 3;

		if (ioamhram[0x10F] & ioamhram[0x1FF] & 0x1F) {
			next_unhalttime = COUNTER_DISABLED;
			interrupter.unhalt();
		} else
			next_unhalttime = std::min(next_irqEventTime, display.nextIrqEvent()) + isCgb() * 4;

		break;
	case OAM:
		nextOamEventTime = lastOamDmaUpdate == COUNTER_DISABLED ? static_cast<unsigned long>(COUNTER_DISABLED) : nextOamEventTime + 0xA0 * 4;
		break;
	case END:
		{
			const unsigned long endtime = next_endtime;
			next_endtime = COUNTER_DISABLED;
			set_event();

			while (cycleCounter >= next_eventtime)
				cycleCounter = event(cycleCounter);

			next_endtime = endtime;
			active = false;
		}

		break;
	}

	set_event();

	return cycleCounter;
}

void Memory::speedChange(const unsigned long cycleCounter) {
	if (isCgb() && (ioamhram[0x14D] & 0x1)) {
		std::printf("speedChange\n");

		update_irqEvents(cycleCounter);
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		display.preSpeedChange(cycleCounter);

		ioamhram[0x14D] = ~ioamhram[0x14D] & 0x80;
		doubleSpeed = ioamhram[0x14D] >> 7;

		display.postSpeedChange(cycleCounter);

		if (hdma_transfer) {
			next_dmatime = display.nextHdmaTime(cycleCounter);
			next_hdmaReschedule = display.nextHdmaTimeInvalid();
		}

		next_blittime = (ioamhram[0x140] & 0x80) ? display.nextMode1IrqTime() : static_cast<unsigned long>(COUNTER_DISABLED);
		next_endtime = cycleCounter + (isDoubleSpeed() ?( next_endtime - cycleCounter) << 1 : ((next_endtime - cycleCounter) >> 1));
		set_irqEvent();
		rescheduleIrq(cycleCounter);
		set_event();
	}
}

static void decCycles(unsigned long &counter, const unsigned long dec) {
	if (counter != Memory::COUNTER_DISABLED)
		counter -= dec;
}

unsigned long Memory::resetCounters(unsigned long cycleCounter) {
	std::printf("resetting counters\n");

	if (lastOamDmaUpdate != COUNTER_DISABLED)
		updateOamDma(cycleCounter);

	update_irqEvents(cycleCounter);
	rescheduleIrq(cycleCounter);
	display.preResetCounter(cycleCounter);

	const unsigned long oldCC = cycleCounter;

	{
		const unsigned long divinc = (cycleCounter - div_lastUpdate) >> 8;
		ioamhram[0x104] = (ioamhram[0x104] + divinc) & 0xFF;
		div_lastUpdate += divinc << 8;
	}

	if (ioamhram[0x107] & 0x04) {
		update_tima(cycleCounter);
	}

	const unsigned long dec = cycleCounter < 0x10000 ? 0 : (cycleCounter & ~0x7FFFul) - 0x8000;

	minIntTime = minIntTime < cycleCounter ? 0 : minIntTime - dec;

	if (ioamhram[0x107] & 0x04)
		decCycles(tima_lastUpdate, dec);

	decCycles(div_lastUpdate, dec);
	decCycles(lastOamDmaUpdate, dec);
	decCycles(next_eventtime, dec);
	decCycles(next_irqEventTime, dec);
	decCycles(next_timatime, dec);
	decCycles(next_blittime, dec);
	decCycles(nextOamEventTime, dec);
	decCycles(next_endtime, dec);
	decCycles(next_dmatime, dec);
	decCycles(next_hdmaReschedule, dec);
	decCycles(nextIntTime, dec);
	decCycles(next_serialtime, dec);
	decCycles(tmatime, dec);
	decCycles(next_unhalttime, dec);

	cycleCounter -= dec;

	display.postResetCounter(oldCC, cycleCounter);
	sound.resetCounter(cycleCounter, oldCC, isDoubleSpeed());

	return cycleCounter;
}

void Memory::updateInput() {
	unsigned button = 0xFF;
	unsigned dpad = 0xFF;

	if (getInput) {
		const Gambatte::InputState &is = (*getInput)();

		button ^= is.startButton << 3;
		button ^= is.selectButton << 2;
		button ^= is.bButton << 1;
		button ^= is.aButton;

		dpad ^= is.dpadDown << 3;
		dpad ^= is.dpadUp << 2;
		dpad ^= is.dpadLeft << 1;
		dpad ^= is.dpadRight;
	}

	ioamhram[0x100] |= 0xF;

	if (!(ioamhram[0x100] & 0x10))
		ioamhram[0x100] &= dpad;

	if (!(ioamhram[0x100] & 0x20))
		ioamhram[0x100] &= button;
}

void Memory::setRombank() {
	unsigned bank = rombank;

	if ((romtype == mbc1 && !(bank & 0x1F)) || (romtype == mbc5 && !bank))
		++bank;

	romdata[1] = romdata[0] + bank * 0x4000ul - 0x4000;

	if (oamDmaArea1Lower != 0xA0) {
		rmem[0x7] = rmem[0x6] = rmem[0x5] = rmem[0x4] = romdata[1];
	} else
		setOamDmaSrc();
}

void Memory::setRambank() {
	rmem[0xB] = rmem[0xA] = rsrambankptr = rdisabled_ram - 0xA000;
	wmem[0xB] = wmem[0xA] = wsrambankptr = wdisabled_ram - 0xA000;

	if (enable_ram) {
		if (rtc.getActive()) {
			wmem[0xB] = wmem[0xA] = rmem[0xB] = rmem[0xA] = wsrambankptr = rsrambankptr = NULL;
		} else if (rambanks) {
			wmem[0xB] = rmem[0xB] = wmem[0xA] = rmem[0xA] = wsrambankptr = rsrambankptr = rambankdata + rambank * 0x2000ul - 0xA000;
		}
	}

	if (oamDmaArea1Lower == 0xA0) {
		wmem[0xB] = wmem[0xA] = rmem[0xB] = rmem[0xA] = NULL;
		setOamDmaSrc();
	}
}

void Memory::setBanks() {
	rmem[0x3] = rmem[0x2] = rmem[0x1] = rmem[0x0] = romdata[0];

	setRombank();
	setRambank();

	rmem[0xC] = wmem[0xC] = wramdata[0] - 0xC000;
	rmem[0xD] = wmem[0xD] = wramdata[1] - 0xD000;
	rmem[0xE] = wmem[0xE] = wramdata[0] - 0xE000;
}

void Memory::updateOamDma(const unsigned long cycleCounter) {
	unsigned cycles = (cycleCounter - lastOamDmaUpdate) >> 2;

	while (cycles--) {
		oamDmaPos = (oamDmaPos + 1) & 0xFF;
		lastOamDmaUpdate += 4;

		//TODO: reads from vram while the ppu is reading vram should return whatever the ppu is reading.
		if (oamDmaPos < 0xA0) {
			if (oamDmaPos == 0)
				startOamDma(lastOamDmaUpdate - 2);

			ioamhram[oamDmaPos] = oamDmaSrc ? oamDmaSrc[oamDmaPos] : *rtc.getActive();
		} else if (oamDmaPos == 0xA0) {
			endOamDma(lastOamDmaUpdate - 2);
			lastOamDmaUpdate = COUNTER_DISABLED;
			break;
		}
	}
}

void Memory::setOamDmaArea() {
	if (ioamhram[0x146] < 0xC0) {
		if ((ioamhram[0x146] & 0xE0) != 0x80)
			oamDmaArea2Upper = 0x80;

		oamDmaArea1Width = 0x20;
	} else if (ioamhram[0x146] < 0xE0)
		oamDmaArea1Width = 0x3E;
}

void Memory::oamDmaInitSetup() {
	if (ioamhram[0x146] < 0xC0) {
		if ((ioamhram[0x146] & 0xE0) == 0x80) {
			oamDmaArea1Lower = 0x80;
		} else {
			oamDmaArea1Lower = 0xA0;
			std::fill_n(rmem, 0x8, static_cast<unsigned char*>(NULL));
			rmem[0xB] = rmem[0xA] = NULL;
			wmem[0xB] = wmem[0xA] = NULL;
		}
	} else if (ioamhram[0x146] < 0xE0) {
		oamDmaArea1Lower = 0xC0;
		rmem[0xE] = rmem[0xD] = rmem[0xC] = NULL;
		wmem[0xE] = wmem[0xD] = wmem[0xC] = NULL;
	}
}

void Memory::setOamDmaSrc() {
	oamDmaSrc = NULL;

	if (ioamhram[0x146] < 0xC0) {
		if ((ioamhram[0x146] & 0xE0) == 0x80) {
			oamDmaSrc = vrambank + (ioamhram[0x146] << 8 & 0x1FFF);
		} else {
			if (ioamhram[0x146] < 0x80)
				oamDmaSrc = romdata[ioamhram[0x146] >> 6] + (ioamhram[0x146] << 8);
			else if (rsrambankptr)
				oamDmaSrc = rsrambankptr + (ioamhram[0x146] << 8);
		}
	} else if (ioamhram[0x146] < 0xE0) {
		oamDmaSrc = wramdata[ioamhram[0x146] >> 4 & 1] + (ioamhram[0x146] << 8 & 0xFFF);
	} else
		oamDmaSrc = rdisabled_ram;
}

void Memory::startOamDma(const unsigned long cycleCounter) {
	setOamDmaArea();
	display.oamChange(rdisabled_ram, cycleCounter);

	if (next_unhalttime != COUNTER_DISABLED)
		schedule_unhalt();
	else
		rescheduleIrq(cycleCounter);

	rescheduleHdmaReschedule();
}

void Memory::endOamDma(const unsigned long cycleCounter) {
	oamDmaArea2Upper = oamDmaArea1Width = oamDmaArea1Lower = 0;
	oamDmaPos = 0xFE;
	setBanks();
	display.oamChange(ioamhram, cycleCounter);

	if (next_unhalttime != COUNTER_DISABLED)
		schedule_unhalt();
	else
		rescheduleIrq(cycleCounter);

	rescheduleHdmaReschedule();
}

void Memory::update_tima(const unsigned long cycleCounter) {
	const unsigned long ticks = (cycleCounter - tima_lastUpdate) >> timaClock[ioamhram[0x107] & 3];

	tima_lastUpdate += ticks << timaClock[ioamhram[0x107] & 3];

	if (cycleCounter >= tmatime) {
		if (cycleCounter >= tmatime + 4)
			tmatime = COUNTER_DISABLED;

		ioamhram[0x105] = ioamhram[0x106];
	}

	unsigned long tmp = ioamhram[0x105] + ticks;

	while (tmp > 0x100)
		tmp -= 0x100 - ioamhram[0x106];

	if (tmp == 0x100) {
		tmp = 0;
		tmatime = tima_lastUpdate + 3;

		if (cycleCounter >= tmatime) {
			if (cycleCounter >= tmatime + 4)
				tmatime = COUNTER_DISABLED;

			tmp = ioamhram[0x106];
		}
	}

	ioamhram[0x105] = tmp;
}

unsigned Memory::nontrivial_ff_read(const unsigned P, const unsigned long cycleCounter) {
	if (lastOamDmaUpdate != COUNTER_DISABLED)
		updateOamDma(cycleCounter);

	switch (P & 0x7F) {
	case 0x00:
		updateInput();
		break;
	case 0x04:
// 		printf("div read\n");
		{
			const unsigned long divcycles = (cycleCounter - div_lastUpdate) >> 8;
			ioamhram[0x104] = (ioamhram[0x104] + divcycles) & 0xFF;
			div_lastUpdate += divcycles << 8;
		}

		break;
	case 0x05:
// 		printf("tima read\n");
		if (ioamhram[0x107] & 0x04)
			update_tima(cycleCounter);

		break;
	case 0x0F:
		update_irqEvents(cycleCounter);
		ioamhram[0x10F] |= display.getIfReg(cycleCounter) & 3;
// 		rescheduleIrq(cycleCounter);
		break;
	case 0x26:
// 		printf("sound status read\n");
		if (ioamhram[0x126] & 0x80) {
			sound.generate_samples(cycleCounter, isDoubleSpeed());
			ioamhram[0x126] = 0xF0 | sound.getStatus();
		} else
			ioamhram[0x126] = 0x70;

		break;
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3A:
	case 0x3B:
	case 0x3C:
	case 0x3D:
	case 0x3E:
	case 0x3F:
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		return sound.waveRamRead(P & 0xF);
	case 0x41:
		return ioamhram[0x141] | display.get_stat(ioamhram[0x145], cycleCounter);
	case 0x44:
		return display.getLyReg(cycleCounter/*+4*/);
	case 0x69:
		return display.cgbBgColorRead(ioamhram[0x168] & 0x3F, cycleCounter);
	case 0x6B:
		return display.cgbSpColorRead(ioamhram[0x16A] & 0x3F, cycleCounter);
	default: break;
	}

	return ioamhram[P - 0xFE00];
}

unsigned Memory::nontrivial_read(const unsigned P, const unsigned long cycleCounter) {
	if (P < 0xFF80) {
		if (lastOamDmaUpdate != COUNTER_DISABLED) {
			updateOamDma(cycleCounter);

			if ((P >> 8) - oamDmaArea1Lower < oamDmaArea1Width || P >> 8 < oamDmaArea2Upper)
				return ioamhram[oamDmaPos];
		}

		if (P < 0xC000) {
			if (P < 0x8000)
				return romdata[P >> 14][P];

			if (P < 0xA000) {
				if (!display.vramAccessible(cycleCounter))
					return 0xFF;

				return vrambank[P & 0x1FFF];
			}

			if (rsrambankptr)
				return rsrambankptr[P];

			return *rtc.getActive();
		}

		if (P < 0xFE00)
			return wramdata[P >> 12 & 1][P & 0xFFF];

		if (P & 0x100)
			return nontrivial_ff_read(P, cycleCounter);

		if (!display.oamAccessible(cycleCounter) || oamDmaPos < 0xA0)
			return 0xFF;
	}

	return ioamhram[P - 0xFE00];
}

void Memory::nontrivial_ff_write(const unsigned P, unsigned data, const unsigned long cycleCounter) {
// 	printf("mem[0x%X] = 0x%X\n", P, data);

	if (lastOamDmaUpdate != COUNTER_DISABLED)
		updateOamDma(cycleCounter);

	switch (P & 0xFF) {
	case 0x00:
		data = (ioamhram[0x100] & 0xCF) | (data & 0xF0);
		break;
	case 0x01:
		update_irqEvents(cycleCounter);
		break;
	case 0x02:
		update_irqEvents(cycleCounter);

		if ((data & 0x81) == 0x81) {
			next_serialtime = cycleCounter;
			next_serialtime += (isCgb() && (data & 0x2)) ? 128 : 4096;
			set_irqEvent();
		}

		rescheduleIrq(cycleCounter);
		data |= 0x7C;
		break;
		//If rom is trying to write to DIV register, reset it to 0.
	case 0x04:
// 		printf("DIV write\n");
		ioamhram[0x104] = 0;
		div_lastUpdate = cycleCounter;
		return;
	case 0x05:
		// printf("tima write\n");
		if (ioamhram[0x107] & 0x04) {
			update_irqEvents(cycleCounter);
			update_tima(cycleCounter);

			if (tmatime - cycleCounter < 4)
				tmatime = COUNTER_DISABLED;

			next_timatime = tima_lastUpdate + ((256u - data) << timaClock[ioamhram[0x107] & 3]) + 1;
			set_irqEvent();
			rescheduleIrq(cycleCounter);
		}

		break;
	case 0x06:
		if (ioamhram[0x107] & 0x04) {
			update_irqEvents(cycleCounter);
			update_tima(cycleCounter);
		}

		break;
	case 0x07:
		// printf("tac write: %i\n", data);
		data |= 0xF8;

		if (ioamhram[0x107] ^ data) {
			if (ioamhram[0x107] & 0x04) {
				update_irqEvents(cycleCounter);
				update_tima(cycleCounter);

				tima_lastUpdate -= (1u << (timaClock[ioamhram[0x107] & 3] - 1)) + 3;
				tmatime -= (1u << (timaClock[ioamhram[0x107] & 3] - 1)) + 3;
				next_timatime -= (1u << (timaClock[ioamhram[0x107] & 3] - 1)) + 3;
				set_irqEvent();
				update_tima(cycleCounter);
				update_irqEvents(cycleCounter);

				tmatime = COUNTER_DISABLED;
				next_timatime = COUNTER_DISABLED;
			}

			if (data & 4) {
				tima_lastUpdate = (cycleCounter >> timaClock[data & 3]) << timaClock[data & 3];
				next_timatime = tima_lastUpdate + ((256u - ioamhram[0x105]) << timaClock[data & 3]) + 1;
			}

			set_irqEvent();
			rescheduleIrq(cycleCounter);
		}

		break;
	case 0x0F:
		update_irqEvents(cycleCounter);
		display.setIfReg(data, cycleCounter);
		ioamhram[0x10F] = 0xE0 | data;
		rescheduleIrq(cycleCounter);
		return;
	case 0x10:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr10(data);
		data |= 0x80;
		break;
	case 0x11:
		if(!sound.isEnabled()) {
			if (isCgb())
				return;

			data &= 0x3F;
		}

		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr11(data);
		data |= 0x3F;
		break;
	case 0x12:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr12(data);
		break;
	case 0x13:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr13(data);
		return;
	case 0x14:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr14(data);
		data |= 0xBF;
		break;
	case 0x16:
		if(!sound.isEnabled()) {
			if (isCgb())
				return;

			data &= 0x3F;
		}

		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr21(data);
		data |= 0x3F;
		break;
	case 0x17:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr22(data);
		break;
	case 0x18:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr23(data);
		return;
	case 0x19:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr24(data);
		data |= 0xBF;
		break;
	case 0x1A:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr30(data);
		data |= 0x7F;
		break;
	case 0x1B:
		if(!sound.isEnabled() && isCgb()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr31(data);
		return;
	case 0x1C:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr32(data);
		data |= 0x9F;
		break;
	case 0x1D:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr33(data);
		return;
	case 0x1E:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr34(data);
		data |= 0xBF;
		break;
	case 0x20:
		if(!sound.isEnabled() && isCgb()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr41(data);
		return;
	case 0x21:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr42(data);
		break;
	case 0x22:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr43(data);
		break;
	case 0x23:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr44(data);
		data |= 0xBF;
		break;
	case 0x24:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_so_volume(data);
		break;
	case 0x25:
		if(!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.map_so(data);
		break;
	case 0x26:
		if ((ioamhram[0x126] ^ data) & 0x80) {
			sound.generate_samples(cycleCounter, isDoubleSpeed());

			if (!(data & 0x80)) {
				for (unsigned i = 0xFF10; i < 0xFF26; ++i)
					ff_write(i, 0, cycleCounter);

// 				std::memcpy(memory + 0xFF10, soundRegInitValues, sizeof(soundRegInitValues));
				sound.setEnabled(false);
			} else {
				sound.reset(/*memory + 0xFF00, isDoubleSpeed()*/);
				sound.setEnabled(true);
			}
		}

		data = (data & 0x80) | (ioamhram[0x126] & 0x7F);
		break;
	case 0x30:
	case 0x31:
	case 0x32:
	case 0x33:
	case 0x34:
	case 0x35:
	case 0x36:
	case 0x37:
	case 0x38:
	case 0x39:
	case 0x3A:
	case 0x3B:
	case 0x3C:
	case 0x3D:
	case 0x3E:
	case 0x3F:
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.waveRamWrite(P & 0xF, data);
		break;
	case 0x40:
		if (ioamhram[0x140] != data) {
			if ((ioamhram[0x140] ^ data) & 0x80) {
				update_irqEvents(cycleCounter);
				const unsigned lyc = display.get_stat(ioamhram[0x145], cycleCounter) & 4;
				display.enableChange(cycleCounter);
				ioamhram[0x144] = 0;
// 				enable_display = bool(data & 0x80);
				ioamhram[0x141] &= 0xF8;

				if (data & 0x80) {
					next_blittime = display.nextMode1IrqTime() + (70224 << isDoubleSpeed());
				} else {
					ioamhram[0x141] |= lyc; //Mr. Do! needs conicidence flag preserved.
					next_blittime = cycleCounter + (456 * 4 << isDoubleSpeed());

					if (hdma_transfer)
						next_dmatime = cycleCounter;

					next_hdmaReschedule = COUNTER_DISABLED;
				}

				set_event();
			}

			if ((ioamhram[0x140] ^ data) & 0x4) {
				display.spriteSizeChange(data & 0x4, cycleCounter);
			}

			if ((ioamhram[0x140] ^ data) & 0x20) {
// 				printf("%u: weChange to %u\n", CycleCounter, (data & 0x20) != 0);
				display.weChange(data & 0x20, cycleCounter);
			}

			if ((ioamhram[0x140] ^ data) & 0x40)
				display.wdTileMapSelectChange(data & 0x40, cycleCounter);

			if ((ioamhram[0x140] ^ data) & 0x08)
				display.bgTileMapSelectChange(data & 0x08, cycleCounter);

			if ((ioamhram[0x140] ^ data) & 0x10)
				display.bgTileDataSelectChange(data & 0x10, cycleCounter);

			if ((ioamhram[0x140] ^ data) & 0x02)
				display.spriteEnableChange(data & 0x02, cycleCounter);

			if ((ioamhram[0x140] ^ data) & 0x01)
				display.bgEnableChange(data & 0x01, cycleCounter);

			ioamhram[0x140] = data;
			rescheduleIrq(cycleCounter);
			rescheduleHdmaReschedule();
		}

		return;
	case 0x41:
		display.lcdstatChange(data, cycleCounter);
		rescheduleIrq(cycleCounter);
		data = (ioamhram[0x141] & 0x87) | (data & 0x78);
		break;
	case 0x42:
		display.scyChange(data, cycleCounter);
		break;
	case 0x43:
		display.scxChange(data, cycleCounter);
		rescheduleIrq(cycleCounter);
		rescheduleHdmaReschedule();
		break;
		//If rom is trying to write to LY register, reset it to 0.
	case 0x44:
		if (ioamhram[0x140] & 0x80) {
			std::printf("ly write\n");
			display.lyWrite(cycleCounter);
			rescheduleIrq(cycleCounter);
			rescheduleHdmaReschedule();
		}

		return;
	case 0x45:
		display.lycRegChange(data, cycleCounter);
		rescheduleIrq(cycleCounter);
		break;
	case 0x46:
		if (lastOamDmaUpdate != COUNTER_DISABLED)
			endOamDma(cycleCounter);

		lastOamDmaUpdate = cycleCounter;
		nextOamEventTime = cycleCounter + 8;
		ioamhram[0x146] = data;
		oamDmaInitSetup();
		setOamDmaSrc();
		return;
	case 0x47:
		if (!isCgb()) {
			display.dmgBgPaletteChange(data, cycleCounter);
		}

		break;
	case 0x48:
		if (!isCgb()) {
			display.dmgSpPalette1Change(data, cycleCounter);
		}

		break;
	case 0x49:
		if (!isCgb()) {
			display.dmgSpPalette2Change(data, cycleCounter);
		}

		break;
	case 0x4A:
// 		printf("%u: wyChange to %u\n", CycleCounter, data);
		display.wyChange(data, cycleCounter);
		rescheduleIrq(cycleCounter);
		rescheduleHdmaReschedule();
		break;
	case 0x4B:
// 		printf("%u: wxChange to %u\n", CycleCounter, data);
		display.wxChange(data, cycleCounter);
		rescheduleIrq(cycleCounter);
		rescheduleHdmaReschedule();
		break;

		//cgb stuff:
	case 0x4D:
		ioamhram[0x14D] |= data & 0x01;
		return;
		//Select vram bank
	case 0x4F:
		if (isCgb()) {
			vrambank = vram + (data & 0x01) * 0x2000;

			if (oamDmaArea1Lower == 0x80)
				setOamDmaSrc();

			ioamhram[0x14F] = 0xFE | data;
		}

		return;
	case 0x51:
		dmaSource = data << 8 | (dmaSource & 0xFF);
		return;
	case 0x52:
		dmaSource = (dmaSource & 0xFF00) | (data & 0xF0);
		return;
	case 0x53:
		dmaDestination = data << 8 | (dmaDestination & 0xFF);
		return;
	case 0x54:
		dmaDestination = (dmaDestination & 0xFF00) | (data & 0xF0);
		return;
	case 0x55:
		if (!isCgb())
			return;

		ioamhram[0x155] = data & 0x7F;

		if (hdma_transfer) {
			if (!(data & 0x80)) {
				ioamhram[0x155] |= 0x80;

				if (next_dmatime > cycleCounter) {
					hdma_transfer = 0;
					next_hdmaReschedule = next_dmatime = COUNTER_DISABLED;
					set_event();
				}
			}

			return;
		}

		if (data & 0x80) {
			hdma_transfer = 1;

			if (!(ioamhram[0x140] & 0x80) || display.isHdmaPeriod(cycleCounter)) {
				next_dmatime = cycleCounter;
				next_hdmaReschedule = COUNTER_DISABLED;
			} else {
				next_dmatime = display.nextHdmaTime(cycleCounter);
				next_hdmaReschedule = display.nextHdmaTimeInvalid();
			}
		} else
			next_dmatime = cycleCounter;

		set_event();
		return;
	case 0x56:
		if (isCgb()) {
			ioamhram[0x156] = data | 0x3E;
		}

		return;
		//Set bg palette index
	case 0x68:
		if (isCgb())
			ioamhram[0x168] = data | 0x40;

		return;
		//Write to bg palette data
	case 0x69:
		if (isCgb()) {
			const unsigned index = ioamhram[0x168] & 0x3F;

			display.cgbBgColorChange(index, data, cycleCounter);

			ioamhram[0x168] = (ioamhram[0x168] & ~0x3F) | ((index + (ioamhram[0x168] >> 7)) & 0x3F);
		}

		return;
	case 0x6A:
		if (isCgb())
			ioamhram[0x16A] = data | 0x40;

		return;
		//Write to obj palette data.
	case 0x6B:
		if (isCgb()) {
			const unsigned index = ioamhram[0x16A] & 0x3F;

			display.cgbSpColorChange(index, data, cycleCounter);

			ioamhram[0x16A] = (ioamhram[0x16A] & ~0x3F) | ((index + (ioamhram[0x16A] >> 7)) & 0x3F);
		}

		return;
	case 0x6C:
		if (isCgb())
			ioamhram[0x16C] = data | 0xFE;

		return;
	case 0x70:
		if (isCgb()) {
			wramdata[1] = wramdata[0] + ((data & 0x07) ? (data & 0x07) : 1) * 0x1000;

			if (oamDmaArea1Lower == 0xC0)
				setOamDmaSrc();
			else
				wmem[0xD] = rmem[0xD] = wramdata[1] - 0xD000;

			ioamhram[0x170] = data | 0xF8;
		}

		return;
	case 0x72:
	case 0x73:
	case 0x74:
		if (isCgb())
			break;

		return;
	case 0x75:
		if (isCgb())
			ioamhram[0x175] = data | 0x8F;

		return;
	case 0xFF:
		ioamhram[0x1FF] = data;
		rescheduleIrq(cycleCounter);
		return;
	default:
// 		if (P < 0xFF80)
		return;
	}

	ioamhram[P - 0xFE00] = data;
}

void Memory::mbc_write(const unsigned P, const unsigned data) {
// 	printf("mem[0x%X] = 0x%X\n", P, data);

	switch (P >> 12 & 0x7) {
	case 0x0:
	case 0x1: //Most MBCs write 0x?A to addresses lower than 0x2000 to enable ram.
		if (romtype == mbc2 && (P & 0x0100)) break;

		enable_ram = (data & 0x0F) == 0xA;

		if (rtcRom)
			rtc.setEnabled(enable_ram);

		setRambank();
		break;
		//MBC1 writes ???n nnnn to address area 0x2000-0x3FFF, ???n nnnn makes up the lower digits to determine which rombank to load.
		//MBC3 writes ?nnn nnnn to address area 0x2000-0x3FFF, ?nnn nnnn makes up the lower digits to determine which rombank to load.
		//MBC5 writes nnnn nnnn to address area 0x2000-0x2FFF, nnnn nnnn makes up the lower digits to determine which rombank to load.
		//MBC5 writes bit8 of the number that determines which rombank to load to address 0x3000-0x3FFF.
	case 0x2:
		switch (romtype) {
		case plain:
			return;
		case mbc5:
			rombank = (rombank & 0x100) | data;
			rombank = rombank & (rombanks - 1);
			setRombank();
			return;
		default:
			break; //Only supposed to break one level.
		}
	case 0x3:
		switch (romtype) {
		case mbc1:
			rombank = rambank_mode ? data & 0x1F : ((rombank & 0x60) | (data & 0x1F));
			break;
		case mbc2:
			if (P & 0x0100) {
				rombank = data & 0x0F;
				break;
			}

			return;
		case mbc3:
			rombank = data & 0x7F;
			break;
		case mbc5:
			rombank = (data & 0x1) << 8 | (rombank & 0xFF);
			break;
		default:
			return;
		}

		rombank = rombank & (rombanks - 1);
		setRombank();
		break;
		//MBC1 writes ???? ??nn to area 0x4000-0x5FFF either to determine rambank to load, or upper 2 bits of the rombank number to load, depending on rom-mode.
		//MBC3 writes ???? ??nn to area 0x4000-0x5FFF to determine rambank to load
		//MBC5 writes ???? nnnn to area 0x4000-0x5FFF to determine rambank to load
	case 0x4:
	case 0x5:
		switch (romtype) {
		case mbc1:
			if (rambank_mode) {
				rambank = data & 0x03;
				break;
			}

			rombank = (data & 0x03) << 5 | (rombank & 0x1F);
			rombank = rombank & (rombanks - 1);
			setRombank();
			return;
		case mbc3:
			if (rtcRom)
				rtc.swapActive(data);

			rambank = data & 0x03;
			break;
		case mbc5:
			rambank = data & 0x0F;
			break;
		default:
			return;
		}

		rambank &= rambanks - 1;
		setRambank();
		break;
		//MBC1: If ???? ???1 is written to area 0x6000-0x7FFFF rom will be set to rambank mode.
	case 0x6:
	case 0x7:
		switch (romtype) {
		case mbc1:
			rambank_mode = data & 0x01;
			break;
		case mbc3:
			rtc.latch(data);
			break;
		default:
			break;
		}

		break;
//     default: break;
	}
}

void Memory::nontrivial_write(const unsigned P, const unsigned data, const unsigned long cycleCounter) {
	if (lastOamDmaUpdate != COUNTER_DISABLED) {
		updateOamDma(cycleCounter);

		if ((P >> 8) - oamDmaArea1Lower < oamDmaArea1Width || P >> 8 < oamDmaArea2Upper) {
			ioamhram[oamDmaPos] = data;
			return;
		}
	}

	if (P < 0xFE00) {
		if (P < 0xA000) {
			if (P < 0x8000) {
				mbc_write(P, data);
			} else if (display.vramAccessible(cycleCounter)) {
				display.vramChange(cycleCounter);
				vrambank[P & 0x1FFF] = data;
			}
		} else if (P < 0xC000) {
			if (wsrambankptr)
				wsrambankptr[P] = data;
			else
				rtc.write(data);
		} else
			wramdata[P >> 12 & 1][P & 0xFFF] = data;
	} else if (((P + 1) & 0xFFFF) < 0xFF81) {
		if (P < 0xFF00) {
			if (display.oamAccessible(cycleCounter) && oamDmaPos >= 0xA0) {
				display.oamChange(cycleCounter);
				rescheduleIrq(cycleCounter);
				rescheduleHdmaReschedule();
				ioamhram[P - 0xFE00] = data;
			}
		} else
			nontrivial_ff_write(P, data, cycleCounter);
	} else
		ioamhram[P - 0xFE00] = data;
}

static const std::string stripExtension(const std::string &str) {
	const std::string::size_type lastDot = str.find_last_of('.');
	const std::string::size_type lastSlash = str.find_last_of('/');

	if (lastDot != std::string::npos && (lastSlash == std::string::npos || lastSlash < lastDot))
		return str.substr(0, lastDot);

	return str;
}

static const std::string stripDir(const std::string &str) {
	const std::string::size_type lastSlash = str.find_last_of('/');

	if (lastSlash != std::string::npos)
		return str.substr(lastSlash + 1);

	return str;
}

const std::string Memory::saveBasePath() const {
	return saveDir.empty() ? defaultSaveBasePath : saveDir + stripDir(defaultSaveBasePath);
}

void Memory::set_savedir(const char *dir) {
	saveDir = dir ? dir : "";

	if (!saveDir.empty() && saveDir[saveDir.length() - 1] != '/') {
		saveDir += '/';
	}
}

static void enforce8bit(unsigned char *data, unsigned long sz) {
	if (static_cast<unsigned char>(0x100))
		while (sz--)
			*data++ &= 0xFF;
}

static unsigned pow2ceil(unsigned n) {
	--n;
	n |= n >> 1;
	n |= n >> 2;
	n |= n >> 4;
	n |= n >> 8;
	++n;

	return n;
}

bool Memory::loadROM(const char *romfile, const bool forceDmg) {
	defaultSaveBasePath = stripExtension(romfile);

	File rom(romfile);

	if (!rom.is_open()) {
		return 1;
	}

	{
		unsigned char header[0x150];
		rom.read(reinterpret_cast<char*>(header), sizeof(header));

		cgb = header[0x0143] >> 7 & 1;

		if (cgb & forceDmg) {
			cgb = false;
			defaultSaveBasePath += "_dmg";
		}

		switch (header[0x0147]) {
		case 0x00: std::printf("Plain ROM loaded.\n");
			romtype = plain;
			break;
		case 0x01: std::printf("MBC1 ROM loaded.\n");
			romtype = mbc1;
			break;
		case 0x02: std::printf("MBC1 ROM+RAM loaded.\n");
			romtype = mbc1;
			break;
		case 0x03: std::printf("MBC1 ROM+RAM+BATTERY loaded.\n");
			romtype = mbc1;
			battery = 1;
			break;
		case 0x05: std::printf("MBC2 ROM loaded.\n");
			romtype = mbc2;
			break;
		case 0x06: std::printf("MBC2 ROM+BATTERY loaded.\n");
			romtype = mbc2;
			battery = 1;
			break;
		case 0x08: std::printf("Plain ROM with additional RAM loaded.\n");
			break;
		case 0x09: std::printf("Plain ROM with additional RAM and Battery loaded.\n");
			battery = 1;
			break;
		case 0x0B: /*cout << "MM01 ROM not supported.\n";*/
			return 1;
			break;
		case 0x0C: /*cout << "MM01 ROM not supported.\n";*/
			return 1;
			break;
		case 0x0D: /*cout << "MM01 ROM not supported.\n";*/
			return 1;
			break;
		case 0x0F: std::printf("MBC3 ROM+TIMER+BATTERY loaded.\n");
			romtype = mbc3;
			battery = true;
			rtcRom = true;
			break;
		case 0x10: std::printf("MBC3 ROM+TIMER+RAM+BATTERY loaded.\n");
			romtype = mbc3;
			battery = true;
			rtcRom = true;
			break;
		case 0x11: std::printf("MBC3 ROM loaded.\n");
			romtype = mbc3;
			break;
		case 0x12: std::printf("MBC3 ROM+RAM loaded.\n");
			romtype = mbc3;
			break;
		case 0x13: std::printf("MBC3 ROM+RAM+BATTERY loaded.\n");
			romtype = mbc3;
			battery = 1;
			break;
		case 0x15: /*cout << "MBC4 ROM not supported.\n";*/
			return 1;
			break;
		case 0x16: /*cout << "MBC4 ROM not supported.\n";*/
			return 1;
			break;
		case 0x17: /*cout << "MBC4 ROM not supported.\n";*/
			return 1;
			break;
		case 0x19: std::printf("MBC5 ROM loaded.\n");
			romtype = mbc5;
			break;
		case 0x1A: std::printf("MBC5 ROM+RAM loaded.\n");
			romtype = mbc5;
			break;
		case 0x1B: std::printf("MBC5 ROM+RAM+BATTERY loaded.\n");
			romtype = mbc5;
			battery = 1;
			break;
		case 0x1C: std::printf("MBC5+RUMLE ROM not supported.\n");
			romtype = mbc5;
			break;
		case 0x1D: std::printf("MBC5+RUMLE+RAM ROM not suported.\n");
			romtype = mbc5;
			break;
		case 0x1E: std::printf("MBC5+RUMLE+RAM+BATTERY ROM not supported.\n");
			romtype = mbc5;
			battery = 1;
			break;
		case 0xFC: /*cout << "Pocket Camera ROM not supported.\n";*/
			return 1;
			break;
		case 0xFD: /*cout << "Bandai TAMA5 ROM not supported.\n";*/
			return 1;
			break;
		case 0xFE: /*cout << "HuC3 ROM not supported.\n";*/
			return 1;
			break;
		case 0xFF: /*cout << "HuC1 ROM not supported.\n";*/
			return 1;
			break;
		default: /*cout << "Wrong data-format, corrupt or unsupported ROM loaded.\n";*/
			return 1;
		}

		/*switch (header[0x0148]) {
		case 0x00:
			rombanks = 2;
			break;
		case 0x01:
			rombanks = 4;
			break;
		case 0x02:
			rombanks = 8;
			break;
		case 0x03:
			rombanks = 16;
			break;
		case 0x04:
			rombanks = 32;
			break;
		case 0x05:
			rombanks = 64;
			break;
		case 0x06:
			rombanks = 128;
			break;
		case 0x07:
			rombanks = 256;
			break;
		case 0x08:
			rombanks = 512;
			break;
		case 0x52:
			rombanks = 72;
			break;
		case 0x53:
			rombanks = 80;
			break;
		case 0x54:
			rombanks = 96;
			break;
		default:
			return 1;
		}

		printf("rombanks: %u\n", rombanks);*/

		switch (header[0x0149]) {
		case 0x00: /*cout << "No RAM\n";*/ rambanks = romtype == mbc2; break;
		case 0x01: /*cout << "2kB RAM\n";*/ /*rambankrom=1; break;*/
		case 0x02: /*cout << "8kB RAM\n";*/
			rambanks = 1;
			break;
		case 0x03: /*cout << "32kB RAM\n";*/
			rambanks = 4;
			break;
		case 0x04: /*cout << "128kB RAM\n";*/
			rambanks = 16;
			break;
		case 0x05: /*cout << "undocumented kB RAM\n";*/
			rambanks = 16;
			break;
		default: /*cout << "Wrong data-format, corrupt or unsupported ROM loaded.\n";*/
			rambanks = 16;
			break;
		}
	}

	std::printf("rambanks: %u\n", rambanks);

	rombanks = pow2ceil(rom.size() / 0x4000);
	std::printf("rombanks: %lu\n", rom.size() / 0x4000);

	delete []memchunk;
	memchunk = new unsigned char[0x4000 + rombanks * 0x4000ul + rambanks * 0x2000ul + (isCgb() ? 0x8000 : 0x2000) + 0x4000];

	romdata[0] = memchunk + 0x4000;
	rambankdata = romdata[0] + rombanks * 0x4000ul;
	wramdata[0] = rambankdata + rambanks * 0x2000;
	rdisabled_ram = wramdata[0] + (isCgb() ? 0x8000 : 0x2000);
	wdisabled_ram = rdisabled_ram + 0x2000;

	wramdata[1] = wramdata[0] + 0x1000;
	std::memset(rdisabled_ram, 0xFF, 0x2000);

	rom.rewind();
	rom.read(reinterpret_cast<char*>(romdata[0]), (rom.size() / 0x4000) * 0x4000ul);
	// In case rombanks isn't a power of 2, allocate a disabled area for invalid rombank addresses. This is only based on speculation.
	std::memset(romdata[0] + (rom.size() / 0x4000) * 0x4000ul, 0xFF, (rombanks - rom.size() / 0x4000) * 0x4000ul);
	enforce8bit(romdata[0], rombanks * 0x4000ul);

	if (rom.fail())
		return 1;

	sound.init(isCgb());
	display.reset(ioamhram, isCgb());

	return 0;
}

void Memory::loadSavedata() {
	const std::string &sbp = saveBasePath();

	if (battery) {
		std::ifstream file((sbp + ".sav").c_str(), std::ios::binary | std::ios::in);

		if (file.is_open()) {
			file.read(reinterpret_cast<char*>(rambankdata), rambanks * 0x2000ul);
			enforce8bit(rambankdata, rambanks * 0x2000ul);
		}
	}

	if (rtcRom) {
		std::ifstream file((sbp + ".rtc").c_str(), std::ios::binary | std::ios::in);

		if (file.is_open()) {
			unsigned long basetime = file.get() & 0xFF;

			basetime = basetime << 8 | (file.get() & 0xFF);
			basetime = basetime << 8 | (file.get() & 0xFF);
			basetime = basetime << 8 | (file.get() & 0xFF);

			rtc.setBaseTime(basetime);
		}
	}
}

void Memory::saveSavedata() {
	const std::string &sbp = saveBasePath();

	if (battery) {
		std::ofstream file((sbp + ".sav").c_str(), std::ios::binary | std::ios::out);

		file.write(reinterpret_cast<const char*>(rambankdata), rambanks * 0x2000ul);
	}

	if (rtcRom) {
		std::ofstream file((sbp + ".rtc").c_str(), std::ios::binary | std::ios::out);
		const unsigned long basetime = rtc.getBaseTime();

		file.put(basetime >> 24 & 0xFF);
		file.put(basetime >> 16 & 0xFF);
		file.put(basetime >> 8 & 0xFF);
		file.put(basetime & 0xFF);
	}
}

unsigned Memory::fillSoundBuffer(const unsigned long cycleCounter) {
	sound.generate_samples(cycleCounter, isDoubleSpeed());
	return sound.fillBuffer();
}

void Memory::setVideoBlitter(Gambatte::VideoBlitter *const vb) {
	display.setVideoBlitter(vb);
}

void Memory::videoBufferChange() {
	display.videoBufferChange();
}

void Memory::setVideoFilter(const unsigned int n) {
	display.setVideoFilter(n);
}

void Memory::setDmgPaletteColor(unsigned palNum, unsigned colorNum, unsigned long rgb32) {
	display.setDmgPaletteColor(palNum, colorNum, rgb32);
}

Memory::~Memory() {
	saveSavedata();

	delete []memchunk;
}
