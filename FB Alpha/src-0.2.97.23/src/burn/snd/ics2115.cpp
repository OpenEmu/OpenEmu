/******************************************************
 ICS 2115 sound synthesizer.
 
   ICS WaveFront ICS2115V Wavetable Midi Synthesizer, 
 clocked at 33.8688MHz 
 
 Original ics2115.c in MAME
   By O. Galibert, with a lot of help from the nebula
   ics emulation by Elsemi.
 
 Port to FB Alpha by OopsWare
 ******************************************************/

#include "burnint.h"
#include "z80_intf.h"
#include "msm6295.h"
#include "burn_sound.h"
#include "ics2115.h"

UINT8 *ICSSNDROM;
UINT32 nICSSNDROMLen;

enum { V_ON = 1, V_DONE = 2 };

struct ics2115 {
	UINT8 * rom;
	INT16 ulaw[256];
	struct {
		UINT16 fc, addrh, addrl, strth, endh, volacc;
		UINT8 strtl, endl, saddr, pan, conf, ctl;
		UINT8 vstart, vend, vctl;
		UINT8 state;
	} voice[32];
	struct {
		UINT8 scale, preset;
		//mame_timer *timer;
		bool active;
		UINT32 period;
	} timer[2];
	UINT8 reg, osc;
	UINT8 irq_en, irq_pend;
	INT32 irq_on;
	//sound_stream * stream;
};

static struct ics2115 * chip = NULL;
static INT16 * sndbuffer = NULL;
static UINT16 nSoundlatch[3] = {0, 0, 0};
static UINT8 bSoundlatchRead[3] = {0, 0, 0};

#define	ICS2115_RATE				33075
#define ICS2115_FRAME_BUFFER_SIZE	(ICS2115_RATE / 60 + 1)

static UINT32 nSoundDelta;

/* ICS2115V chip emu */

static void recalc_irq()
{
    INT32 irq = 0;
	if(chip->irq_en & chip->irq_pend) irq = 1;
	for(INT32 i=0; !irq && i<32; i++)
		if(chip->voice[i].state & V_DONE) irq = 1;
	if(irq != chip->irq_on) {
		chip->irq_on = irq;
// IRQ callback
//		if(chip->intf->irq_cb)
//			chip->intf->irq_cb(irq ? ASSERT_LINE : CLEAR_LINE);

		if (irq) {
			ZetSetIRQLine(0xFF, ZET_IRQSTATUS_ACK);
		} else
			ZetSetIRQLine(0x00, ZET_IRQSTATUS_NONE);
	}
}

static void timer_cb_0()
{
	chip->irq_pend |= 1<<0;
	recalc_irq();
}

static void timer_cb_1()
{
	chip->irq_pend |= 1<<1;
	recalc_irq();
}

static void recalc_timer(INT32 timer)
{
	float period = 0;

	if(chip->timer[timer].scale) {
		INT32 sc = chip->timer[timer].scale;
		float counter = (float)((((sc & 31)+1) * (chip->timer[timer].preset+1)) << (4+(sc >> 5)));
		period = 1000000000 * counter / 33868800;
	} else {
		period = 0;
	}

	if (chip->timer[timer].period != (UINT32)period) {
		chip->timer[timer].period = (UINT32)period;
		if(period) chip->timer[timer].active = true;
		else  chip->timer[timer].active = false;
	}
}

UINT16 ics2115read_reg(UINT8 reg)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115read_reg called without init\n"));
#endif

	switch (reg) {
	case 0x0d: // [osc] Volume Enveloppe Control
		return 0x100;
	case 0x0f: {// [osc] Interrupt source/oscillator
		UINT8 res = 0xff;
		for(INT32 osc=0; osc<32; osc++)
			if(chip->voice[osc].state & V_DONE) {
				chip->voice[osc].state &= ~V_DONE;
				recalc_irq();
				res = 0x40 | osc; // 0x40 ? 0x80 ?
				break;
			}
		return res << 8; }
	case 0x40: // Timer 0 clear irq
		chip->irq_pend &= ~(1<<0);
		recalc_irq();
		return chip->timer[0].preset;
	case 0x41: // Timer 1 clear irq
		chip->irq_pend &= ~(1<<1);
		recalc_irq();
		return chip->timer[1].preset;
	case 0x43: // Timer status
		return chip->irq_pend & 3;
	case 0x4a: // IRQ Pending
		return chip->irq_pend;
	case 0x4b: // Address of Interrupting Oscillator
		return 0x80;
	case 0x4c: // Chip revision
		return 0x01;
	default:
		return 0;
	}
}

void ics2115write_reg(UINT8 reg, UINT8 data, INT32 msb)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115write_reg called without init\n"));
#endif

//	bprintf(PRINT_NORMAL, _T("ics2115write_reg(%02x, %02x, %d);  %4.1f%%\n"), reg, data, msb, 6.0 * ZetTotalCycles() / 8468.0 ); 
	
	switch (reg) {
	case 0x00: // [osc] Oscillator Configuration
		if( msb ) chip->voice[chip->osc].conf = data;
		break;
	case 0x01: // [osc] Wavesample frequency
		if(msb)	chip->voice[chip->osc].fc = (chip->voice[chip->osc].fc & 0xff) | (data << 8);
		else	chip->voice[chip->osc].fc = (chip->voice[chip->osc].fc & 0xff00) | data;
		//bprintf(PRINT_NORMAL, _T("ICS2115: %2d: fc = %04x (%dHz)\n"), chip->osc,chip->voice[chip->osc].fc, chip->voice[chip->osc].(fc*33075+512)/1024);
		break;
	case 0x02: // [osc] Wavesample loop start address 19-4
		if(msb)	chip->voice[chip->osc].strth = (chip->voice[chip->osc].strth & 0xff) | (data << 8);
		else	chip->voice[chip->osc].strth = (chip->voice[chip->osc].strth & 0xff00) | data;
		break;
	case 0x03: // [osc] Wavesample loop start address 3-0.3-0
		if(msb) chip->voice[chip->osc].strtl = data;
		break;
	case 0x04: // [osc] Wavesample loop end address 19-4
		if(msb)	chip->voice[chip->osc].endh = (chip->voice[chip->osc].endh & 0xff)|(data << 8);
		else	chip->voice[chip->osc].endh = (chip->voice[chip->osc].endh & 0xff00)|data;
		break;
	case 0x05: // [osc] Wavesample loop end address 3-0.3-0
		if(msb)	chip->voice[chip->osc].endl = data;
		break;
	case 0x07: // [osc] Volume Start
		if(msb) chip->voice[chip->osc].vstart = data;
		break;
	case 0x08: // [osc] Volume End
		if(msb) chip->voice[chip->osc].vend = data;
		break;
	case 0x09: // [osc] Volume accumulator
		if(msb)	chip->voice[chip->osc].volacc = (chip->voice[chip->osc].volacc & 0xff)|(data << 8);
		else	chip->voice[chip->osc].volacc = (chip->voice[chip->osc].volacc & 0xff00)|data;
		//bprintf(PRINT_NORMAL, _T("ICS2115: %2d: volacc = %04x\n"), chip->osc,chip->voice[chip->osc].volacc);
		break;
	case 0x0a: // [osc] Wavesample address 19-4
		if(msb)	chip->voice[chip->osc].addrh = (chip->voice[chip->osc].addrh & 0xff)|(data << 8);
		else	chip->voice[chip->osc].addrh = (chip->voice[chip->osc].addrh & 0xff00)|data;
		break;
	case 0x0b: // [osc] Wavesample address 3-0.8-0
		if(msb)	chip->voice[chip->osc].addrl = (chip->voice[chip->osc].addrl & 0xff)|(data << 8);
		else	chip->voice[chip->osc].addrl = (chip->voice[chip->osc].addrl & 0xff00)|data;
		break;
	case 0x0c: // [osc] Pan
		if(msb) chip->voice[chip->osc].pan = data;
		break;
	case 0x0d: // [osc] Volume Enveloppe Control
		if(msb) chip->voice[chip->osc].vctl = data;
		break;
	case 0x10: // [osc] Oscillator Control
		if(msb) {
			chip->voice[chip->osc].ctl = data;
			if(data == 0) {
				chip->voice[chip->osc].state |= V_ON;
//				bprintf(PRINT_NORMAL, _T("ICS2115: KEYON %2d volacc = %04x fc = %04x (%dHz)\n"),
//						chip->osc, chip->voice[chip->osc].volacc, chip->voice[chip->osc].fc, (chip->voice[chip->osc].fc*33075 + 512) / 1024  );
			}
		} break;
	case 0x11: // [osc] Wavesample static address 27-20
		if(msb) chip->voice[chip->osc].saddr = data;
		break;
	case 0x40: // Timer 1 Preset
		if(!msb) {
			chip->timer[0].preset = data;
			recalc_timer(0);
		} break;
	case 0x41: // Timer 2 Preset
		if(!msb) {
			chip->timer[1].preset = data;
			recalc_timer(1);
		} break;
	case 0x42: // Timer 1 Prescaler
		if(!msb) {
			chip->timer[0].scale = data;
			recalc_timer(0);
		} break;
	case 0x43: // Timer 2 Prescaler
		if(!msb) {
			chip->timer[1].scale = data;
			recalc_timer(1);
		} break;
	case 0x4a: // IRQ Enable
		if(!msb) {
			chip->irq_en = data;
			recalc_irq();
		} break;
	case 0x4f: // Oscillator Address being Programmed
		if(!msb) chip->osc = data & 31;
		break;
	}
}

UINT8 ics2115read(UINT8 offset)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115read called without init\n"));
#endif

	switch ( offset ) {
	case 0x00: {
		UINT8 res = 0;
		if(chip->irq_on) {
			res |= 0x80;
			if(chip->irq_en & chip->irq_pend & 3) res |= 1; // Timer irq
			for(INT32 i=0; i<32; i++)
				if(chip->voice[i].state & V_DONE) {
					res |= 2;
					break;
				}
		}
		return res; }
	case 0x01:
		return chip->reg;
	case 0x02:
		return ics2115read_reg(chip->reg) & 0xff;
	case 0x03:
	default:
		return ics2115read_reg(chip->reg) >> 8;
	}
}

void ics2115write(UINT8 offset, UINT8 data)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115write called without init\n"));
#endif

	switch (offset) {
	case 0x01:
		chip->reg = data;
		break;
	case 0x02:
		ics2115write_reg(chip->reg, data, 0);
		break;
	case 0x03:
		ics2115write_reg(chip->reg, data, 1);
		break;
	default:
		break;	
	}
}

INT32 ics2115_init()
{
	DebugSnd_ICS2115Initted = 1;
	
	chip = (struct ics2115 *)BurnMalloc(sizeof(struct ics2115));	// ICS2115V
	if (chip == NULL) return 1;
	
	sndbuffer = (INT16*)BurnMalloc(ICS2115_FRAME_BUFFER_SIZE * sizeof(INT16*));
	if (sndbuffer == NULL) return 1;

	return 0;
}

void ics2115_exit()
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115_exit called without init\n"));
#endif

	BurnFree(chip);

	nICSSNDROMLen = 0;

	BurnFree(ICSSNDROM);
	BurnFree(sndbuffer);
	
	DebugSnd_ICS2115Initted = 0;
}

static void recalculate_ulaw()
{
	for(INT32 i=0; i<256; i++) {
		UINT8 c = ((~i) & 0xFF);
		INT32 v = ((c & 15) << 1) + 33;
		v <<= ((c & 0x70) >> 4);
		if(c & 0x80) v = 33-v;
		else		 v = v-33;
		chip->ulaw[i] = v;
	}
}

void ics2115_reset()
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115_reset called without init\n"));
#endif

	memset(chip, 0, sizeof(struct ics2115));

	chip->rom = ICSSNDROM;
//	chip->timer[0].timer = timer_alloc_ptr(timer_cb_0, chip);
//	chip->timer[1].timer = timer_alloc_ptr(timer_cb_1, chip);
//	chip->stream = stream_create(0, 2, 33075, chip, update);
//	if(!chip->timer[0].timer || !chip->timer[1].timer) return NULL;

	recalculate_ulaw();
	
	if (nBurnSoundLen) {
		nSoundDelta = ICS2115_FRAME_BUFFER_SIZE * 0x10000 / nBurnSoundLen;
	} else {
		nSoundDelta = ICS2115_FRAME_BUFFER_SIZE * 0x10000 / 184; // 11025Hz
	}
	
	recalc_irq();
	
	memset(nSoundlatch, 0, sizeof(nSoundlatch));
	memset(bSoundlatchRead, 0, sizeof(bSoundlatchRead));
}

UINT16 ics2115_soundlatch_r(INT32 i)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115_soundlatch_r called without init\n"));
#endif

//	bprintf(PRINT_NORMAL, _T("soundlatch_r(%d)  %4.1f%% of frame\n"), i, 6.0 * SekTotalCycles() / 20000.0 ); 
	bSoundlatchRead[i] = 1;
	return nSoundlatch[i];
}

void ics2115_soundlatch_w(INT32 i, UINT16 d)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115_soundlatch_w called without init\n"));
#endif

//	if  ( !bSoundlatchRead[i] && nSoundlatch[i] != d )
//		bprintf(PRINT_ERROR, _T("soundlatch_w(%d, %04x)  %4.1f%% of frame\n"), i, d, 6.0 * SekTotalCycles() / 20000.0);
//	else
//		bprintf(PRINT_NORMAL, _T("soundlatch_w(%d, %04x)  %4.1f%% of frame\n"), i, d, 6.0 * SekTotalCycles() / 20000.0);
	nSoundlatch[i] = d;
	bSoundlatchRead[i] = 0;
}

void ics2115_frame()
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115_frame called without init\n"));
#endif

	if (chip->timer[0].active ) timer_cb_0();
	if (chip->timer[1].active ) timer_cb_1();	
}

void ics2115_update(INT32 /*length*/)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115_update called without init\n"));
#endif

	INT32 rec_irq = 0;
	
	//short* pSoundBuf = pBurnSoundOut;
	
	memset(sndbuffer, 0, ICS2115_FRAME_BUFFER_SIZE * sizeof(INT16));

	for(INT32 osc=0; osc<32; osc++)
		if(chip->voice[osc].state & V_ON) {
			UINT32 badr = (chip->voice[osc].saddr << 20) & 0x0f00000;
			UINT32 adr = (chip->voice[osc].addrh << 16) | chip->voice[osc].addrl;
			UINT32 end = (chip->voice[osc].endh << 16) | (chip->voice[osc].endl << 8);
			UINT32 loop = (chip->voice[osc].strth << 16) | (chip->voice[osc].strtl << 8);
			UINT32 conf = chip->voice[osc].conf;
			INT32 vol = chip->voice[osc].volacc;
			vol = (((vol & 0xff0)|0x1000)<<(vol>>12))>>12;
			UINT32 delta = chip->voice[osc].fc << 2;

			for(INT32 i=0; i<ICS2115_FRAME_BUFFER_SIZE; i++) {
				INT32 v = chip->rom[(badr|(adr >> 12))];
				
				if(conf & 1)v = chip->ulaw[v];
				else		v = ((INT8)v) << 6;

				v = (v*vol)>>(16+5);
				
				sndbuffer[i] += v;

				adr += delta;
				if(adr >= end) {
					//if (ICS2115LOGERROR) logerror("ICS2115: KEYDONE %2d\n", osc);
					adr -= (end-loop);
					chip->voice[osc].state &= ~V_ON;
					chip->voice[osc].state |= V_DONE;
					rec_irq = 1;
					break;
				}
			}
			
			chip->voice[osc].addrh = adr >> 16;
			chip->voice[osc].addrl = adr;
		}

	if(rec_irq) recalc_irq();
	
	if (pBurnSoundOut) {
		INT32 pos = 0;
		INT16 * pOut = (INT16*)pBurnSoundOut;
		for(INT32 i=0; i<nBurnSoundLen; i++, pOut+=2, pos+=nSoundDelta)
			pOut[0] = pOut[1] = sndbuffer[ pos >> 16 ] << 4;
	}
	
}

void ics2115_scan(INT32 nAction,INT32 * /*pnMin*/)
{
#if defined FBA_DEBUG
	if (!DebugSnd_ICS2115Initted) bprintf(PRINT_ERROR, _T("ics2115_scan called without init\n"));
#endif

	struct BurnArea ba;
	
	if ( nAction & ACB_DRIVER_DATA ) {
		UINT8 *rom = chip->rom;

		ba.Data		= chip;
		ba.nLen		= sizeof(struct ics2115);
		ba.nAddress = 0;
		ba.szName	= "ICS 2115";
		BurnAcb(&ba);

		chip->rom = rom;

		SCAN_VAR(nSoundlatch);
		SCAN_VAR(bSoundlatchRead);
	}
}
