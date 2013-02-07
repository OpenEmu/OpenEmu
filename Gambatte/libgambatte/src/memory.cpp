/***************************************************************************
 *   Copyright (C) 2007 by Sindre Aamås                                    *
 *   sinamas@users.sourceforge.net                                         *
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
#include "inputgetter.h"
#include "savestate.h"
#include <cstring>

namespace gambatte {

Memory::Memory(const Interrupter &interrupter_in)
: getInput(0),
  divLastUpdate(0),
  lastOamDmaUpdate(DISABLED_TIME),
  display(ioamhram, 0, VideoInterruptRequester(&intreq)),
  interrupter(interrupter_in),
  dmaSource(0),
  dmaDestination(0),
  oamDmaPos(0xFE),
  serialCnt(0),
  blanklcd(false)
{
	intreq.setEventTime<BLIT>(144*456ul);
	intreq.setEventTime<END>(0);
}

void Memory::setStatePtrs(SaveState &state) {
	state.mem.ioamhram.set(ioamhram, sizeof ioamhram);

	cart.setStatePtrs(state);
	display.setStatePtrs(state);
	sound.setStatePtrs(state);
}

unsigned long Memory::saveState(SaveState &state, unsigned long cycleCounter) {
	cycleCounter = resetCounters(cycleCounter);
	nontrivial_ff_read(0xFF05, cycleCounter);
	nontrivial_ff_read(0xFF0F, cycleCounter);
	nontrivial_ff_read(0xFF26, cycleCounter);

	state.mem.divLastUpdate = divLastUpdate;
	state.mem.nextSerialtime = intreq.eventTime(SERIAL);
	state.mem.unhaltTime = intreq.eventTime(UNHALT);
	state.mem.lastOamDmaUpdate = lastOamDmaUpdate;
	state.mem.dmaSource = dmaSource;
	state.mem.dmaDestination = dmaDestination;
	state.mem.oamDmaPos = oamDmaPos;

	intreq.saveState(state);
	cart.saveState(state);
	tima.saveState(state);
	display.saveState(state);
	sound.saveState(state);

	return cycleCounter;
}

static inline int serialCntFrom(const unsigned long cyclesUntilDone, const bool cgbFast) {
	return cgbFast ? (cyclesUntilDone + 0xF) >> 4 : (cyclesUntilDone + 0x1FF) >> 9;
}

void Memory::loadState(const SaveState &state) {
	sound.loadState(state);
	display.loadState(state, state.mem.oamDmaPos < 0xA0 ? cart.rdisabledRam() : ioamhram);
	tima.loadState(state, TimaInterruptRequester(intreq));
	cart.loadState(state);
	intreq.loadState(state);

	divLastUpdate = state.mem.divLastUpdate;
	intreq.setEventTime<SERIAL>(state.mem.nextSerialtime > state.cpu.cycleCounter ? state.mem.nextSerialtime : state.cpu.cycleCounter);
	intreq.setEventTime<UNHALT>(state.mem.unhaltTime);
	lastOamDmaUpdate = state.mem.lastOamDmaUpdate;
	dmaSource = state.mem.dmaSource;
	dmaDestination = state.mem.dmaDestination;
	oamDmaPos = state.mem.oamDmaPos;
	serialCnt = intreq.eventTime(SERIAL) != DISABLED_TIME
			? serialCntFrom(intreq.eventTime(SERIAL) - state.cpu.cycleCounter, ioamhram[0x102] & isCgb() * 2)
			: 8;

	cart.setVrambank(ioamhram[0x14F] & isCgb());
	cart.setOamDmaSrc(OAM_DMA_SRC_OFF);
	cart.setWrambank(isCgb() && (ioamhram[0x170] & 0x07) ? ioamhram[0x170] & 0x07 : 1);

	if (lastOamDmaUpdate != DISABLED_TIME) {
		oamDmaInitSetup();

		const unsigned oamEventPos = oamDmaPos < 0xA0 ? 0xA0 : 0x100;

		intreq.setEventTime<OAM>(lastOamDmaUpdate + (oamEventPos - oamDmaPos) * 4);
	}

	intreq.setEventTime<BLIT>((ioamhram[0x140] & 0x80) ? display.nextMode1IrqTime() : state.cpu.cycleCounter);
	blanklcd = false;
	
	if (!isCgb())
		std::memset(cart.vramdata() + 0x2000, 0, 0x2000);
}

void Memory::setEndtime(const unsigned long cycleCounter, const unsigned long inc) {
	if (intreq.eventTime(BLIT) <= cycleCounter)
		intreq.setEventTime<BLIT>(intreq.eventTime(BLIT) + (70224 << isDoubleSpeed()));
	
	intreq.setEventTime<END>(cycleCounter + (inc << isDoubleSpeed()));
}

void Memory::updateSerial(const unsigned long cc) {
	if (intreq.eventTime(SERIAL) != DISABLED_TIME) {
		if (intreq.eventTime(SERIAL) <= cc) {
			ioamhram[0x101] = (((ioamhram[0x101] + 1) << serialCnt) - 1) & 0xFF;
			ioamhram[0x102] &= 0x7F;
			intreq.setEventTime<SERIAL>(DISABLED_TIME);
			intreq.flagIrq(8);
		} else {
			const int targetCnt = serialCntFrom(intreq.eventTime(SERIAL) - cc, ioamhram[0x102] & isCgb() * 2);
			ioamhram[0x101] = (((ioamhram[0x101] + 1) << (serialCnt - targetCnt)) - 1) & 0xFF;
			serialCnt = targetCnt;
		}
	}
}

void Memory::updateTimaIrq(const unsigned long cc) {
	while (intreq.eventTime(TIMA) <= cc)
		tima.doIrqEvent(TimaInterruptRequester(intreq));
}

void Memory::updateIrqs(const unsigned long cc) {
	updateSerial(cc);
	updateTimaIrq(cc);
	display.update(cc);
}

unsigned long Memory::event(unsigned long cycleCounter) {
	if (lastOamDmaUpdate != DISABLED_TIME)
		updateOamDma(cycleCounter);

	switch (intreq.minEventId()) {
	case UNHALT:
		intreq.unhalt();
		intreq.setEventTime<UNHALT>(DISABLED_TIME);
		break;
	case END:
		intreq.setEventTime<END>(DISABLED_TIME - 1);

		while (cycleCounter >= intreq.minEventTime() && intreq.eventTime(END) != DISABLED_TIME)
			cycleCounter = event(cycleCounter);
		
		intreq.setEventTime<END>(DISABLED_TIME);

		break;
	case BLIT:
		{
			const bool lcden = ioamhram[0x140] >> 7 & 1;
			unsigned long blitTime = intreq.eventTime(BLIT);
			
			if (lcden | blanklcd) {
				display.updateScreen(blanklcd, cycleCounter);
				intreq.setEventTime<BLIT>(DISABLED_TIME);
				intreq.setEventTime<END>(DISABLED_TIME);
				
				while (cycleCounter >= intreq.minEventTime())
					cycleCounter = event(cycleCounter);
			} else
				blitTime += 70224 << isDoubleSpeed();
			
			blanklcd = lcden ^ 1;
			intreq.setEventTime<BLIT>(blitTime);
		}
		break;
	case SERIAL:
		updateSerial(cycleCounter);
		break;
	case OAM:
		intreq.setEventTime<OAM>(lastOamDmaUpdate == DISABLED_TIME ?
				static_cast<unsigned long>(DISABLED_TIME) : intreq.eventTime(OAM) + 0xA0 * 4);
		break;
	case DMA:
		{
			const bool doubleSpeed = isDoubleSpeed();
			unsigned dmaSrc = dmaSource;
			unsigned dmaDest = dmaDestination;
			unsigned dmaLength = ((ioamhram[0x155] & 0x7F) + 0x1) * 0x10;
			unsigned length = hdmaReqFlagged(intreq) ? 0x10 : dmaLength;
			
			ackDmaReq(&intreq);

			if ((static_cast<unsigned long>(dmaDest) + length) & 0x10000) {
				length = 0x10000 - dmaDest;
				ioamhram[0x155] |= 0x80;
			}

			dmaLength -= length;

			if (!(ioamhram[0x140] & 0x80))
				dmaLength = 0;

			{
				unsigned long lOamDmaUpdate = lastOamDmaUpdate;
				lastOamDmaUpdate = DISABLED_TIME;

				while (length--) {
					const unsigned src = dmaSrc++ & 0xFFFF;
					const unsigned data = ((src & 0xE000) == 0x8000 || src > 0xFDFF) ? 0xFF : read(src, cycleCounter);

					cycleCounter += 2 << doubleSpeed;

					if (cycleCounter - 3 > lOamDmaUpdate) {
						oamDmaPos = (oamDmaPos + 1) & 0xFF;
						lOamDmaUpdate += 4;

						if (oamDmaPos < 0xA0) {
							if (oamDmaPos == 0)
								startOamDma(lOamDmaUpdate - 1);

							ioamhram[src & 0xFF] = data;
						} else if (oamDmaPos == 0xA0) {
							endOamDma(lOamDmaUpdate - 1);
							lOamDmaUpdate = DISABLED_TIME;
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

			if ((ioamhram[0x155] & 0x80) && display.hdmaIsEnabled()) {
				if (lastOamDmaUpdate != DISABLED_TIME)
					updateOamDma(cycleCounter);
				
				display.disableHdma(cycleCounter);
			}
		}

		break;
	case TIMA:
		tima.doIrqEvent(TimaInterruptRequester(intreq));
		break;
	case VIDEO:
		display.update(cycleCounter);
		break;
	case INTERRUPTS:
		if (halted()) {
			if (isCgb())
				cycleCounter += 4;
			
			intreq.unhalt();
			intreq.setEventTime<UNHALT>(DISABLED_TIME);
		}
		
		if (ime()) {
			unsigned address;
			const unsigned pendingIrqs = intreq.pendingIrqs();
			const unsigned n = pendingIrqs & -pendingIrqs;
			
			if (n < 8) {
				static const unsigned char lut[] = { 0x40, 0x48, 0x48, 0x50 };
				address = lut[n-1];
			} else
				address = 0x50 + n;
			
			intreq.ackIrq(n);
			cycleCounter = interrupter.interrupt(address, cycleCounter, *this);
		}
		
		break;
	}

	return cycleCounter;
}

unsigned long Memory::stop(unsigned long cycleCounter) {
	cycleCounter += 4 << isDoubleSpeed();
	
	if (ioamhram[0x14D] & isCgb()) {
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		
		display.speedChange(cycleCounter);
		ioamhram[0x14D] = ~ioamhram[0x14D] & 0x80;

		intreq.setEventTime<BLIT>((ioamhram[0x140] & 0x80) ? display.nextMode1IrqTime() : cycleCounter + (70224 << isDoubleSpeed()));
		
		if (intreq.eventTime(END) > cycleCounter) {
			intreq.setEventTime<END>(cycleCounter + (isDoubleSpeed() ?
					(intreq.eventTime(END) - cycleCounter) << 1 : (intreq.eventTime(END) - cycleCounter) >> 1));
		}
	}
	
	intreq.halt();
	intreq.setEventTime<UNHALT>(cycleCounter + 0x20000 + isDoubleSpeed() * 8);
	
	return cycleCounter;
}

static void decCycles(unsigned long &counter, const unsigned long dec) {
	if (counter != DISABLED_TIME)
		counter -= dec;
}

void Memory::decEventCycles(const MemEventId eventId, const unsigned long dec) {
	if (intreq.eventTime(eventId) != DISABLED_TIME)
		intreq.setEventTime(eventId, intreq.eventTime(eventId) - dec);
}

unsigned long Memory::resetCounters(unsigned long cycleCounter) {
	if (lastOamDmaUpdate != DISABLED_TIME)
		updateOamDma(cycleCounter);

	updateIrqs(cycleCounter);

	const unsigned long oldCC = cycleCounter;

	{
		const unsigned long divinc = (cycleCounter - divLastUpdate) >> 8;
		ioamhram[0x104] = (ioamhram[0x104] + divinc) & 0xFF;
		divLastUpdate += divinc << 8;
	}

	const unsigned long dec = cycleCounter < 0x10000 ? 0 : (cycleCounter & ~0x7FFFul) - 0x8000;

	decCycles(divLastUpdate, dec);
	decCycles(lastOamDmaUpdate, dec);
	decEventCycles(SERIAL, dec);
	decEventCycles(OAM, dec);
	decEventCycles(BLIT, dec);
	decEventCycles(END, dec);
	decEventCycles(UNHALT, dec);

	cycleCounter -= dec;
	
	intreq.resetCc(oldCC, cycleCounter);
	tima.resetCc(oldCC, cycleCounter, TimaInterruptRequester(intreq));
	display.resetCc(oldCC, cycleCounter);
	sound.resetCounter(cycleCounter, oldCC, isDoubleSpeed());

	return cycleCounter;
}

void Memory::updateInput() {
	unsigned button = 0xFF;
	unsigned dpad = 0xFF;

	if (getInput) {
		const unsigned is = (*getInput)();
		button ^= is      & 0x0F;
		dpad   ^= is >> 4 & 0x0F;
	}

	ioamhram[0x100] |= 0xF;

	if (!(ioamhram[0x100] & 0x10))
		ioamhram[0x100] &= dpad;

	if (!(ioamhram[0x100] & 0x20))
		ioamhram[0x100] &= button;
}

void Memory::updateOamDma(const unsigned long cycleCounter) {
	const unsigned char *const oamDmaSrc = oamDmaSrcPtr();
	unsigned cycles = (cycleCounter - lastOamDmaUpdate) >> 2;

	while (cycles--) {
		oamDmaPos = (oamDmaPos + 1) & 0xFF;
		lastOamDmaUpdate += 4;

		if (oamDmaPos < 0xA0) {
			if (oamDmaPos == 0)
				startOamDma(lastOamDmaUpdate - 1);

			ioamhram[oamDmaPos] = oamDmaSrc ? oamDmaSrc[oamDmaPos] : cart.rtcRead();
		} else if (oamDmaPos == 0xA0) {
			endOamDma(lastOamDmaUpdate - 1);
			lastOamDmaUpdate = DISABLED_TIME;
			break;
		}
	}
}

void Memory::oamDmaInitSetup() {
	if (ioamhram[0x146] < 0xA0) {
		cart.setOamDmaSrc(ioamhram[0x146] < 0x80 ? OAM_DMA_SRC_ROM : OAM_DMA_SRC_VRAM);
	} else if (ioamhram[0x146] < 0xFE - isCgb() * 0x1E) {
		cart.setOamDmaSrc(ioamhram[0x146] < 0xC0 ? OAM_DMA_SRC_SRAM : OAM_DMA_SRC_WRAM);
	} else
		cart.setOamDmaSrc(OAM_DMA_SRC_INVALID);
}

static const unsigned char * oamDmaSrcZero() {
	static unsigned char zeroMem[0xA0];
	return zeroMem;
}

const unsigned char * Memory::oamDmaSrcPtr() const {
	switch (cart.oamDmaSrc()) {
	case OAM_DMA_SRC_ROM:  return cart.romdata(ioamhram[0x146] >> 6) + (ioamhram[0x146] << 8);
	case OAM_DMA_SRC_SRAM: return cart.rsrambankptr() ? cart.rsrambankptr() + (ioamhram[0x146] << 8) : 0;
	case OAM_DMA_SRC_VRAM: return cart.vrambankptr() + (ioamhram[0x146] << 8);
	case OAM_DMA_SRC_WRAM: return cart.wramdata(ioamhram[0x146] >> 4 & 1) + (ioamhram[0x146] << 8 & 0xFFF);
	case OAM_DMA_SRC_INVALID:
	case OAM_DMA_SRC_OFF:  break;
	}
	
	return ioamhram[0x146] == 0xFF && !isCgb() ? oamDmaSrcZero() : cart.rdisabledRam();
}

void Memory::startOamDma(const unsigned long cycleCounter) {
	display.oamChange(cart.rdisabledRam(), cycleCounter);
}

void Memory::endOamDma(const unsigned long cycleCounter) {
	oamDmaPos = 0xFE;
	cart.setOamDmaSrc(OAM_DMA_SRC_OFF);
	display.oamChange(ioamhram, cycleCounter);
}

unsigned Memory::nontrivial_ff_read(const unsigned P, const unsigned long cycleCounter) {
	if (lastOamDmaUpdate != DISABLED_TIME)
		updateOamDma(cycleCounter);

	switch (P & 0x7F) {
	case 0x00:
		updateInput();
		break;
	case 0x01:
	case 0x02:
		updateSerial(cycleCounter);
		break;
	case 0x04:
		{
			const unsigned long divcycles = (cycleCounter - divLastUpdate) >> 8;
			ioamhram[0x104] = (ioamhram[0x104] + divcycles) & 0xFF;
			divLastUpdate += divcycles << 8;
		}

		break;
	case 0x05:
		ioamhram[0x105] = tima.tima(cycleCounter);
		break;
	case 0x0F:
		updateIrqs(cycleCounter);
		ioamhram[0x10F] = intreq.ifreg();
		break;
	case 0x26:
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
		return ioamhram[0x141] | display.getStat(ioamhram[0x145], cycleCounter);
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

static bool isInOamDmaConflictArea(const OamDmaSrc oamDmaSrc, const unsigned addr, const bool cgb) {
	struct Area { unsigned short areaUpper, exceptAreaLower, exceptAreaWidth, pad; };
	
	static const Area cgbAreas[] = {
		{ 0xC000, 0x8000, 0x2000, 0 },
		{ 0xC000, 0x8000, 0x2000, 0 },
		{ 0xA000, 0x0000, 0x8000, 0 },
		{ 0xFE00, 0x0000, 0xC000, 0 },
		{ 0xC000, 0x8000, 0x2000, 0 },
		{ 0x0000, 0x0000, 0x0000, 0 }
	};
	
	static const Area dmgAreas[] = {
		{ 0xFE00, 0x8000, 0x2000, 0 },
		{ 0xFE00, 0x8000, 0x2000, 0 },
		{ 0xA000, 0x0000, 0x8000, 0 },
		{ 0xFE00, 0x8000, 0x2000, 0 },
		{ 0xFE00, 0x8000, 0x2000, 0 },
		{ 0x0000, 0x0000, 0x0000, 0 }
	};
	
	const Area *const a = cgb ? cgbAreas : dmgAreas;

	return addr < a[oamDmaSrc].areaUpper && addr - a[oamDmaSrc].exceptAreaLower >= a[oamDmaSrc].exceptAreaWidth;
}

unsigned Memory::nontrivial_read(const unsigned P, const unsigned long cycleCounter) {
	if (P < 0xFF80) {
		if (lastOamDmaUpdate != DISABLED_TIME) {
			updateOamDma(cycleCounter);
			
			if (isInOamDmaConflictArea(cart.oamDmaSrc(), P, isCgb()) && oamDmaPos < 0xA0)
				return ioamhram[oamDmaPos];
		}

		if (P < 0xC000) {
			if (P < 0x8000)
				return cart.romdata(P >> 14)[P];

			if (P < 0xA000) {
				if (!display.vramAccessible(cycleCounter))
					return 0xFF;

				return cart.vrambankptr()[P];
			}

			if (cart.rsrambankptr())
				return cart.rsrambankptr()[P];

			return cart.rtcRead();
		}

		if (P < 0xFE00)
			return cart.wramdata(P >> 12 & 1)[P & 0xFFF];

		if (P >= 0xFF00)
			return nontrivial_ff_read(P, cycleCounter);

		if (!display.oamReadable(cycleCounter) || oamDmaPos < 0xA0)
			return 0xFF;
	}

	return ioamhram[P - 0xFE00];
}

void Memory::nontrivial_ff_write(const unsigned P, unsigned data, const unsigned long cycleCounter) {
	if (lastOamDmaUpdate != DISABLED_TIME)
		updateOamDma(cycleCounter);

	switch (P & 0xFF) {
	case 0x00:
		data = (ioamhram[0x100] & 0xCF) | (data & 0xF0);
		break;
	case 0x01:
		updateSerial(cycleCounter);
		break;
	case 0x02:
		updateSerial(cycleCounter);

		serialCnt = 8;
		intreq.setEventTime<SERIAL>((data & 0x81) == 0x81
				? (data & isCgb() * 2 ? (cycleCounter & ~0x7ul) + 0x10 * 8 : (cycleCounter & ~0xFFul) + 0x200 * 8)
				: static_cast<unsigned long>(DISABLED_TIME));

		data |= 0x7E - isCgb() * 2;
		break;
	case 0x04:
		ioamhram[0x104] = 0;
		divLastUpdate = cycleCounter;
		return;
	case 0x05:
		tima.setTima(data, cycleCounter, TimaInterruptRequester(intreq));
		break;
	case 0x06:
		tima.setTma(data, cycleCounter, TimaInterruptRequester(intreq));
		break;
	case 0x07:
		data |= 0xF8;
		tima.setTac(data, cycleCounter, TimaInterruptRequester(intreq));
		break;
	case 0x0F:
		updateIrqs(cycleCounter);
		intreq.setIfreg(0xE0 | data);
		return;
	case 0x10:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr10(data);
		data |= 0x80;
		break;
	case 0x11:
		if (!sound.isEnabled()) {
			if (isCgb())
				return;

			data &= 0x3F;
		}

		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr11(data);
		data |= 0x3F;
		break;
	case 0x12:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr12(data);
		break;
	case 0x13:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr13(data);
		return;
	case 0x14:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr14(data);
		data |= 0xBF;
		break;
	case 0x16:
		if (!sound.isEnabled()) {
			if (isCgb())
				return;

			data &= 0x3F;
		}

		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr21(data);
		data |= 0x3F;
		break;
	case 0x17:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr22(data);
		break;
	case 0x18:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr23(data);
		return;
	case 0x19:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr24(data);
		data |= 0xBF;
		break;
	case 0x1A:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr30(data);
		data |= 0x7F;
		break;
	case 0x1B:
		if (!sound.isEnabled() && isCgb()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr31(data);
		return;
	case 0x1C:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr32(data);
		data |= 0x9F;
		break;
	case 0x1D:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr33(data);
		return;
	case 0x1E:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr34(data);
		data |= 0xBF;
		break;
	case 0x20:
		if (!sound.isEnabled() && isCgb()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr41(data);
		return;
	case 0x21:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr42(data);
		break;
	case 0x22:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr43(data);
		break;
	case 0x23:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_nr44(data);
		data |= 0xBF;
		break;
	case 0x24:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.set_so_volume(data);
		break;
	case 0x25:
		if (!sound.isEnabled()) return;
		sound.generate_samples(cycleCounter, isDoubleSpeed());
		sound.map_so(data);
		break;
	case 0x26:
		if ((ioamhram[0x126] ^ data) & 0x80) {
			sound.generate_samples(cycleCounter, isDoubleSpeed());

			if (!(data & 0x80)) {
				for (unsigned i = 0xFF10; i < 0xFF26; ++i)
					ff_write(i, 0, cycleCounter);

				sound.setEnabled(false);
			} else {
				sound.reset();
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
				const unsigned lyc = display.getStat(ioamhram[0x145], cycleCounter) & 4;
				const bool hdmaEnabled = display.hdmaIsEnabled();
				
				display.lcdcChange(data, cycleCounter);
				ioamhram[0x144] = 0;
				ioamhram[0x141] &= 0xF8;

				if (data & 0x80) {
					intreq.setEventTime<BLIT>(display.nextMode1IrqTime() + (blanklcd ? 0 : 70224 << isDoubleSpeed()));
				} else {
					ioamhram[0x141] |= lyc;
					intreq.setEventTime<BLIT>(cycleCounter + (456 * 4 << isDoubleSpeed()));

					if (hdmaEnabled)
						flagHdmaReq(&intreq);
				}
			} else
				display.lcdcChange(data, cycleCounter);

			ioamhram[0x140] = data;
		}

		return;
	case 0x41:
		display.lcdstatChange(data, cycleCounter);
		data = (ioamhram[0x141] & 0x87) | (data & 0x78);
		break;
	case 0x42:
		display.scyChange(data, cycleCounter);
		break;
	case 0x43:
		display.scxChange(data, cycleCounter);
		break;
	case 0x45:
		display.lycRegChange(data, cycleCounter);
		break;
	case 0x46:
		if (lastOamDmaUpdate != DISABLED_TIME)
			endOamDma(cycleCounter);

		lastOamDmaUpdate = cycleCounter;
		intreq.setEventTime<OAM>(cycleCounter + 8);
		ioamhram[0x146] = data;
		oamDmaInitSetup();
		return;
	case 0x47:
		if (!isCgb())
			display.dmgBgPaletteChange(data, cycleCounter);

		break;
	case 0x48:
		if (!isCgb())
			display.dmgSpPalette1Change(data, cycleCounter);

		break;
	case 0x49:
		if (!isCgb())
			display.dmgSpPalette2Change(data, cycleCounter);

		break;
	case 0x4A:
		display.wyChange(data, cycleCounter);
		break;
	case 0x4B:
		display.wxChange(data, cycleCounter);
		break;

	case 0x4D:
		ioamhram[0x14D] |= data & 0x01;
		return;
	case 0x4F:
		if (isCgb()) {
			cart.setVrambank(data & 1);
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
		if (isCgb()) {
			ioamhram[0x155] = data & 0x7F;

			if (display.hdmaIsEnabled()) {
				if (!(data & 0x80)) {
					ioamhram[0x155] |= 0x80;
					display.disableHdma(cycleCounter);
				}
			} else {
				if (data & 0x80) {
					if (ioamhram[0x140] & 0x80) {
						display.enableHdma(cycleCounter);
					} else
						flagHdmaReq(&intreq);
				} else
					flagGdmaReq(&intreq);
			}
		}

		return;
	case 0x56:
		if (isCgb())
			ioamhram[0x156] = data | 0x3E;

		return;
	case 0x68:
		if (isCgb())
			ioamhram[0x168] = data | 0x40;

		return;
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
			cart.setWrambank((data & 0x07) ? (data & 0x07) : 1);
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
		intreq.setIereg(data);
		break;
	default:
		return;
	}

	ioamhram[P - 0xFE00] = data;
}

void Memory::nontrivial_write(const unsigned P, const unsigned data, const unsigned long cycleCounter) {
	if (lastOamDmaUpdate != DISABLED_TIME) {
		updateOamDma(cycleCounter);
		
		if (isInOamDmaConflictArea(cart.oamDmaSrc(), P, isCgb()) && oamDmaPos < 0xA0) {
			ioamhram[oamDmaPos] = data;
			return;
		}
	}

	if (P < 0xFE00) {
		if (P < 0xA000) {
			if (P < 0x8000) {
				cart.mbcWrite(P, data);
			} else if (display.vramAccessible(cycleCounter)) {
				display.vramChange(cycleCounter);
				cart.vrambankptr()[P] = data;
			}
		} else if (P < 0xC000) {
			if (cart.wsrambankptr())
				cart.wsrambankptr()[P] = data;
			else
				cart.rtcWrite(data);
		} else
			cart.wramdata(P >> 12 & 1)[P & 0xFFF] = data;
	} else if (P - 0xFF80u >= 0x7Fu) {
		if (P < 0xFF00) {
			if (display.oamWritable(cycleCounter) && oamDmaPos >= 0xA0 && (P < 0xFEA0 || isCgb())) {
				display.oamChange(cycleCounter);
				ioamhram[P - 0xFE00] = data;
			}
		} else
			nontrivial_ff_write(P, data, cycleCounter);
	} else
		ioamhram[P - 0xFE00] = data;
}

LoadRes Memory::loadROM(std::string const &romfile, bool const forceDmg, bool const multicartCompat) {
	if (LoadRes const fail = cart.loadROM(romfile, forceDmg, multicartCompat))
		return fail;

	sound.init(cart.isCgb());
	display.reset(ioamhram, cart.vramdata(), cart.isCgb());
	interrupter.setGameShark(std::string());

	return LOADRES_OK;
}

unsigned Memory::fillSoundBuffer(const unsigned long cycleCounter) {
	sound.generate_samples(cycleCounter, isDoubleSpeed());
	return sound.fillBuffer();
}

void Memory::setDmgPaletteColor(unsigned palNum, unsigned colorNum, unsigned long rgb32) {
	display.setDmgPaletteColor(palNum, colorNum, rgb32);
}

}
