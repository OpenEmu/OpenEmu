/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

// Disable symbol overrides so that we can use system headers.
#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <kernel.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sifrpc.h>
#include <loadfile.h>
#include <malloc.h>
#include <assert.h>
#include <iopcontrol.h>
#include <iopheap.h>

#include <sjpcm.h>
#include <libhdd.h>
#include <libmc.h>
#include <libpad.h>
#include <fileXio_rpc.h>
#include "eecodyvdfs.h"

#include "common/config-manager.h"
#include "common/events.h"
#include "common/file.h"
#include "common/scummsys.h"

#include "backends/platform/ps2/asyncfio.h"
#include "backends/platform/ps2/cd.h"
#include "backends/platform/ps2/fileio.h"
#include "backends/platform/ps2/Gs2dScreen.h"
#include "backends/platform/ps2/irxboot.h"
#include "backends/platform/ps2/ps2debug.h"
#include "backends/platform/ps2/ps2input.h"
#include "backends/platform/ps2/savefilemgr.h"
#include "backends/platform/ps2/sysdefs.h"
#include "backends/platform/ps2/systemps2.h"

#include "backends/fs/ps2/ps2-fs-factory.h"
#include "backends/plugins/ps2/ps2-provider.h"

#include "backends/timer/default/default-timer.h"

#include "audio/mixer_intern.h"

#include "engines/engine.h"

#include "graphics/fonts/bdf.h"
#include "graphics/surface.h"

#include "icon.h"
#include "ps2temp.h"

#ifdef __PS2_DEBUG__
#include <debug.h>
#endif

// asm("mfc0	%0, $9\n" : "=r"(tickStart));

extern void *_gp;

#define TIMER_STACK_SIZE (1024 * 32)
#define SOUND_STACK_SIZE (1024 * 32)
#define SMP_PER_BLOCK 800
#define BUS_CLOCK 147456000 // bus clock, a little less than 150 mhz
#define CLK_DIVIS 5760	// the timer IRQ handler gets called (BUS_CLOCK / 256) / CLK_DIVIS times per second (100 times)

static int g_TimerThreadSema = -1, g_SoundThreadSema = -1;
static int g_MainWaitSema = -1, g_TimerWaitSema = -1;
static volatile int32 g_MainWakeUp = 0, g_TimerWakeUp = 0;
volatile uint32 msecCount = 0;

OSystem_PS2 *g_systemPs2;

#define FOREVER 2147483647

namespace Graphics {
	extern const BdfFont g_sysfont;
};

PS2Device detectBootPath(const char *elfPath, char *bootPath);

extern AsyncFio fio;

#ifdef __PS2_DEBUG__
extern "C" int gdb_stub_main(int argc, char *argv[]);
extern "C" void breakpoint(void);
#endif

extern "C" int scummvm_main(int argc, char *argv[]);

extern "C" int main(int argc, char *argv[]) {
	SifInitRpc(0);
	ee_thread_t thisThread;
	int tid = GetThreadId();
	ReferThreadStatus(tid, &thisThread);

	sioprintf("Thread Start Priority = %d\n", thisThread.current_priority);
	if ((thisThread.current_priority < 5) || (thisThread.current_priority > 80)) {
		/* Depending on the way ScummVM is run, we may get here with different
		   thread priorities.
		   The PS2 BIOS executes it with priority = 0, ps2link uses priority 64.
		   Don't know about NapLink, etc.
		   The priority doesn't matter too much, but we need to be at least at prio 3,
		   so we can have the timer thread run at prio 2 and the sound thread at prio 1	*/
		sioprintf("Changing thread priority\n");
		int res = ChangeThreadPriority(tid, 20);
		sioprintf("Result = %d\n", res);
	}

#ifdef __PS2_DEBUG__
	gdb_stub_main(argc, argv);
#endif

	sioprintf("Creating system\n");
	g_system = g_systemPs2 = new OSystem_PS2(argv[0]);

#ifdef DYNAMIC_MODULES
	PluginManager::instance().addPluginProvider(new PS2PluginProvider());
#endif

	g_systemPs2->init();
	sioprintf("init done. starting ScummVM.\n");
	int res = scummvm_main(argc, argv);
	sioprintf("scummvm_main terminated: %d\n", res);

	g_systemPs2->quit();

	// control never gets here
	return res;
}

s32 timerInterruptHandler(s32 cause) {
	T0_MODE = 0xDC2; // same value as in initialization.
	msecCount += 10;

	iSignalSema(g_SoundThreadSema);
	iSignalSema(g_TimerThreadSema);

	if (g_MainWakeUp) {
		g_MainWakeUp -= 10;
		if (g_MainWakeUp <= 0) {
			iSignalSema(g_MainWaitSema);
			g_MainWakeUp = 0;
		}
	}
	if (g_TimerWakeUp) {
		g_TimerWakeUp -= 10;
		if (g_TimerWakeUp <= 0) {
			iSignalSema(g_TimerWaitSema);
			g_TimerWakeUp = 0;
		}
	}
	return 0;
}

void systemTimerThread(OSystem_PS2 *system) {
	system->timerThreadCallback();
}

void systemSoundThread(OSystem_PS2 *system) {
	system->soundThreadCallback();
}

void gluePowerOffCallback(void *system) {
	((OSystem_PS2*)system)->powerOffCallback();
}

void OSystem_PS2::startIrxModules(int numModules, IrxReference *modules) {

	_usbMassLoaded = _useMouse = _useKbd = _useHdd = _useNet = false;

	int res = 0, rv = 0;
	for (int i = 0; i < numModules; i++) {
		if (modules[i].loc == IRX_FILE) {
			res = SifLoadModule(modules[i].path, modules[i].argSize, modules[i].args);
			sioprintf("Module \"%s\": %d\n", modules[i].path, res);
			if (res < 0) {
				msgPrintf(FOREVER, "\"%s\"\nnot found: %d", modules[i].path, res);
				delayMillis(5000);
				quit();
			}
		} else if (modules[i].loc == IRX_BUFFER) {
			if (modules[i].errorCode == 0) {
				res = SifExecModuleBuffer(modules[i].buffer, modules[i].size, modules[i].argSize, modules[i].args, &rv);
				sioprintf("Module \"%s\": EE=%d, IOP=%d\n", modules[i].path, res, rv);
				if ((res >= 0) && (rv >= 0)) {
					switch (modules[i].fileRef->purpose) {
						case MASS_DRIVER:
							_usbMassLoaded = true;
							break;
						case MOUSE_DRIVER:
							_useMouse = true;
							break;
						case KBD_DRIVER:
							_useKbd = true;
							break;
						case HDD_DRIVER:
							_useHdd = true;
							break;
						case NET_DRIVER:
							_useNet = true;
							break;
						default:
							break;
					}
				}
			} else
				sioprintf("Module \"%s\" wasn't found: %d\n", modules[i].path, modules[i].errorCode);

			if ((modules[i].errorCode < 0) || (res < 0) || (rv < 0)) {
				if (!(modules[i].fileRef->flags & OPTIONAL)) {
					if (modules[i].errorCode < 0)
						msgPrintf(FOREVER, "\"%s\"\nnot found: %d", modules[i].path, modules[i].errorCode);
					else
						msgPrintf(FOREVER, "Couldn't start\n\"%s\"\nEE=%d IOP=%d", modules[i].path, res, rv);
					delayMillis(5000);
					quit();
				}
			}

			if (modules[i].buffer)
				free(modules[i].buffer);
		} else {
			sioprintf("module %d of %d damaged, loc %d, path %s\n", i, numModules, modules[i].loc, modules[i].path);
		}
		free(modules[i].path);
	}
	free(modules);
	sioprintf("done\n");
	sioprintf("UsbMass: %sloaded\n", _usbMassLoaded ? "" : "not ");
	sioprintf("Mouse:   %sloaded\n", _useMouse ? "" : "not ");
	sioprintf("Kbd:     %sloaded\n", _useKbd ? "" : "not ");
	sioprintf("Hdd:     %sloaded\n", _useHdd ? "" : "not ");
}

OSystem_PS2::OSystem_PS2(const char *elfPath) {
	_soundStack = _timerStack = NULL;
	_printY = 0;
	_msgClearTime = 0;
	_systemQuit = false;
	_modeChanged = false;
	_screenChangeCount = 0;

	_screen = new Gs2dScreen(320, 200, TV_DONT_CARE);

	sioprintf("Initializing system...\n");

	_screen->wantAnim(true);

	_bootPath = (char *)malloc(128);
	_bootDevice = detectBootPath(elfPath, _bootPath);

	IrxReference *modules;
	int numModules = loadIrxModules(_bootDevice, _bootPath, &modules);

	if (_bootDevice != HOST_DEV) {
		sioprintf("Resetting IOP.\n");
		cdvdInit(CDVD_EXIT);
		cdvdExit();
		SifExitIopHeap();
		SifLoadFileExit();
		SifExitRpc();
		SifIopReset("rom0:UDNL rom0:EELOADCNF", 0);
		while (!SifIopSync())
			;
		sioprintf("IOP synced.\n");
		SifInitRpc(0);
		SifLoadFileInit();
		cdvdInit(CDVD_INIT_WAIT);
	}
	else {
		// romeo : HOST : pre-load

		// TODO: avoid re-loading USB_MASS.IRX -> it will jam mass:

		// TODO: ps2link 1.46 will stall on "poweroff" init / cb
	}

	startIrxModules(numModules, modules);

	int res;
	if ((res = fileXioInit()) < 0) {
		msgPrintf(FOREVER, "FXIO Init failed: %d", res);
		quit();
	}

	if ((res = initCdvdFs()) < 0) {
		msgPrintf(FOREVER, "CoDyVDfs bind failed: %d", res);
		quit();
	}

	if ((res = SjPCM_Init(0)) < 0) {
		msgPrintf(FOREVER, "SjPCM Bind failed: %d\n", res);
		quit();
	}

	if (_useHdd) {
		if ((hddCheckPresent() < 0) || (hddCheckFormatted() < 0))
			_useHdd = false;

		//hddPreparePoweroff();
		poweroffInit();

		//hddSetUserPoweroffCallback(gluePowerOffCallback, this);
		poweroffSetCallback(gluePowerOffCallback, this);
	}

	fileXioSetBlockMode(FXIO_NOWAIT);

	_mouseVisible = false;

	sioprintf("reading RTC\n");
	readRtcTime();

	if (_useHdd) {
		// TODO : make partition path configurable
		if (fio.mount("pfs0:", "hdd0:+ScummVM", 0) >= 0)
			printf("Successfully mounted!\n");
		else
			_useHdd = false;
	}

	initMutexes();
}

void OSystem_PS2::init(void) {
	sioprintf("Timer...\n");
	_timerManager = new DefaultTimerManager();
	_scummMixer = new Audio::MixerImpl(this, 48000);
	_scummMixer->setReady(true);

	initTimer();

	sioprintf("Starting SavefileManager\n");
	_savefileManager = new Ps2SaveFileManager(this, _screen);

	sioprintf("Initializing ps2Input\n");
	_input = new Ps2Input(this, _useMouse, _useKbd);

	prepMC();
	makeConfigPath();

	_screen->wantAnim(false);
	fillScreen(0);
}

OSystem_PS2::~OSystem_PS2(void) {
	free(_bootPath);
	free(_configFile);
}

void OSystem_PS2::initTimer(void) {
	// first setup the two threads that get activated by the timer:
	// the timerthread and the soundthread
	ee_sema_t threadSema;
	threadSema.init_count = 0;
	threadSema.max_count = 255;
	g_TimerThreadSema = CreateSema(&threadSema);
	g_SoundThreadSema = CreateSema(&threadSema);
	assert((g_TimerThreadSema >= 0) && (g_SoundThreadSema >= 0));

	ee_thread_t timerThread, soundThread, thisThread;
	ReferThreadStatus(GetThreadId(), &thisThread);

	_timerStack = (uint8 *)malloc(TIMER_STACK_SIZE);
	_soundStack = (uint8 *)malloc(SOUND_STACK_SIZE);

	// give timer thread a higher priority than main thread
	timerThread.initial_priority = thisThread.current_priority - 1;
	timerThread.stack            = _timerStack;
	timerThread.stack_size       = TIMER_STACK_SIZE;
	timerThread.func             = (void *)systemTimerThread;
	timerThread.gp_reg			 = &_gp;

	// soundthread's priority is higher than main- and timerthread
	soundThread.initial_priority = thisThread.current_priority - 2;
	soundThread.stack            = _soundStack;
	soundThread.stack_size       = SOUND_STACK_SIZE;
	soundThread.func             = (void *)systemSoundThread;
	soundThread.gp_reg			 = &_gp;

	_timerTid = CreateThread(&timerThread);
	_soundTid = CreateThread(&soundThread);

	assert((_timerTid >= 0) && (_soundTid >= 0));

	StartThread(_timerTid, this);
	StartThread(_soundTid, this);

	// these semaphores are used for OSystem::delayMillis()
	threadSema.init_count = 0;
	threadSema.max_count = 1;
	g_MainWaitSema = CreateSema(&threadSema);
	g_TimerWaitSema = CreateSema(&threadSema);
	assert((g_MainWaitSema >= 0) && (g_TimerWaitSema >= 0));

	// threads done, start the interrupt handler
	_intrId = AddIntcHandler( INT_TIMER0, timerInterruptHandler, 0); // 0=first handler
	assert(_intrId >= 0);
	EnableIntc(INT_TIMER0);
	T0_HOLD = 0;
	T0_COUNT = 0;
	T0_COMP = CLK_DIVIS; // (BUS_CLOCK / 256) / CLK_DIVIS = 100
	T0_MODE = TIMER_MODE( 2, 0, 0, 0, 1, 1, 1, 0, 1, 1);
}

void OSystem_PS2::timerThreadCallback(void) {
	while (!_systemQuit) {
		WaitSema(g_TimerThreadSema);
		((DefaultTimerManager *)_timerManager)->handler();
	}
	ExitThread();
}

void OSystem_PS2::soundThreadCallback(void) {
	int16 *soundBufL = (int16 *)memalign(64, SMP_PER_BLOCK * sizeof(int16) * 2);
	int16 *soundBufR = soundBufL + SMP_PER_BLOCK;

	int bufferedSamples = 0;
	int cycles = 0;

	while (!_systemQuit) {
		WaitSema(g_SoundThreadSema);

		if (!(cycles & 31))
			bufferedSamples = SjPCM_Buffered();
		else
			bufferedSamples -= 480;
		cycles++;

		if (bufferedSamples <= 8 * SMP_PER_BLOCK) {
			// we have to produce more samples, call sound mixer
			// the scratchpad at 0x70000000 is used as temporary soundbuffer
			//_scummSoundProc(_scummSoundParam, (uint8 *)0x70000000, SMP_PER_BLOCK * 2 * sizeof(int16));
			// Audio::Mixer::mixCallback(_scummMixer, (byte *)0x70000000, SMP_PER_BLOCK * 2 * sizeof(int16));
			_scummMixer->mixCallback((byte *)0x70000000, SMP_PER_BLOCK * 2 * sizeof(int16));

			// demux data into 2 buffers, L and R
			 __asm__ (
				"move  $t2, %1\n\t"			// dest buffer right
				"move  $t3, %0\n\t"			// dest buffer left
				"lui   $t8, 0x7000\n\t"		// muxed buffer, fixed at 0x70000000
				"addiu $t9, $0, 100\n\t"	// number of loops
				"mtsab $0, 2\n\t"			// set qword shift = 2 byte

				"loop:\n\t"
				"  lq $t4,  0($t8)\n\t"		// load 8 muxed samples
				"  lq $t5, 16($t8)\n\t"		// load 8 more muxed samples

				"  qfsrv $t6, $0, $t4\n\t"	// shift right for second
				"  qfsrv $t7, $0, $t5\n\t"	// packing step (right channel)

				"  ppach $t4, $t5, $t4\n\t"	// combine left channel data
				"  ppach $t6, $t7, $t6\n\t"	// right channel data

				"  sq $t4, 0($t3)\n\t"		// write back
				"  sq $t6, 0($t2)\n\t"		//

				"  addiu $t9, -1\n\t"		// decrement loop counter
				"  addiu $t2, 16\n\t"		// increment pointers
				"  addiu $t3, 16\n\t"
				"  addiu $t8, 32\n\t"
				"  bnez  $t9, loop\n\t"		// loop
					:  // outputs
			 		: "r"(soundBufL), "r"(soundBufR)  // inputs
				//  : "$t2", "$t3", "$t4", "$t5", "$t6", "$t7", "$t8", "$t9"  // destroyed
					: "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25"  // destroyed
			);
			// and feed it into the SPU
			// non-blocking call, the function will return before the buffer's content
			// was transferred.
			SjPCM_Enqueue((short int*)soundBufL, (short int*)soundBufR, SMP_PER_BLOCK, 0);
			bufferedSamples += SMP_PER_BLOCK;
		}
	}
	free(soundBufL);
	ExitThread();
}

bool OSystem_PS2::mcPresent(void) {
	int fd = fio.dopen("mc0:");

	if (fd > 0) {
		fio.dclose(fd);
		return true;
	}

	return false;
}

bool OSystem_PS2::hddPresent(void) {
	return _useHdd;
}

bool OSystem_PS2::usbMassPresent(void) {
	if (_usbMassLoaded) {
		int fd = fio.dopen("mass:");

		if (fd > 0) {
			fio.dclose(fd);
			return true;
		}
	}

	return false;
}

bool OSystem_PS2::netPresent(void) {
	return _useNet;
}

void OSystem_PS2::initSize(uint width, uint height, const Graphics::PixelFormat *format) {
	printf("initializing new size: (%d/%d)...", width, height);
	_screen->newScreenSize(width, height);
	_screen->setMouseXy(width / 2, height / 2);
	_input->newRange(0, 0, width - 1, height - 1);
	_input->warpTo(width / 2, height / 2);

	_oldMouseX = width / 2;
	_oldMouseY = height / 2;

	_modeChanged = true;
	_screenChangeCount++;
	printf("done\n");
}

void OSystem_PS2::setPalette(const byte *colors, uint start, uint num) {
	_screen->setPalette(colors, (uint8)start, (uint16)num);
}

void OSystem_PS2::grabPalette(byte *colors, uint start, uint num) {
	_screen->grabPalette(colors, (uint8)start, (uint16)num);
}

void OSystem_PS2::copyRectToScreen(const void *buf, int pitch, int x, int y, int w, int h) {
	_screen->copyScreenRect((const uint8*)buf, pitch, x, y, w, h);
}

void OSystem_PS2::updateScreen(void) {
	if (_msgClearTime && (_msgClearTime < getMillis())) {
		_screen->clearPrintfOverlay();
		_msgClearTime = 0;
	}
	_screen->updateScreen();
}

void OSystem_PS2::displayMessageOnOSD(const char *msg) {
	/* TODO : check */
	printf("displayMessageOnOSD: %s\n", msg);
}

uint32 OSystem_PS2::getMillis(void) {
	return msecCount;
}

void OSystem_PS2::delayMillis(uint msecs) {
	if (msecs == 0)
		return;

	int tid = GetThreadId();
	if (tid == _soundTid) {
		dbg_printf("ERROR: delayMillis() from sound thread!\n");
		return;
	}

	if (tid == _timerTid) {
		g_TimerWakeUp = (int32)msecs;
		WaitSema(g_TimerWaitSema);
	} else {
		g_MainWakeUp = (int32)msecs;
		WaitSema(g_MainWaitSema);
	}
}

Audio::Mixer *OSystem_PS2::getMixer() {
	return _scummMixer;
}

FilesystemFactory *OSystem_PS2::getFilesystemFactory() {
	return &Ps2FilesystemFactory::instance();
}

void OSystem_PS2::setShakePos(int shakeOffset) {
	_screen->setShakePos(shakeOffset);
}

bool OSystem_PS2::showMouse(bool visible) {
	bool retVal = _mouseVisible;
	_screen->showMouse(visible);
	_mouseVisible = visible;
	return retVal;
}

void OSystem_PS2::warpMouse(int x, int y) {
	_input->warpTo((uint16)x, (uint16)y);
	_screen->setMouseXy(x, y);
}

void OSystem_PS2::setMouseCursor(const void *buf, uint w, uint h, int hotspot_x, int hotspot_y, uint32 keycolor, bool dontScale, const Graphics::PixelFormat *format) {
	_screen->setMouseOverlay((const byte *)buf, w, h, hotspot_x, hotspot_y, keycolor);
}

void OSystem_PS2::showOverlay(void) {
	_screen->showOverlay();
}

void OSystem_PS2::hideOverlay(void) {
	_screen->hideOverlay();
}

void OSystem_PS2::clearOverlay(void) {
	_screen->clearOverlay();
}

void OSystem_PS2::grabOverlay(void *buf, int pitch) {
	_screen->grabOverlay((byte *)buf, (uint16)pitch);
}

void OSystem_PS2::copyRectToOverlay(const void *buf, int pitch, int x, int y, int w, int h) {
	_screen->copyOverlayRect((const byte *)buf, (uint16)pitch, (uint16)x, (uint16)y, (uint16)w, (uint16)h);
}

Graphics::PixelFormat OSystem_PS2::getOverlayFormat(void) const {
	return _screen->getOverlayFormat();
}


int16 OSystem_PS2::getOverlayWidth(void) {
	return _screen->getOverlayWidth();
}

int16 OSystem_PS2::getOverlayHeight(void) {
	return _screen->getOverlayHeight();
}

Graphics::Surface *OSystem_PS2::lockScreen(void) {
	return _screen->lockScreen();
}

void OSystem_PS2::unlockScreen(void) {
	_screen->unlockScreen();
}

const OSystem::GraphicsMode OSystem_PS2::_graphicsMode = { NULL, NULL, 0 };

const OSystem::GraphicsMode *OSystem_PS2::getSupportedGraphicsModes(void) const {
    return &_graphicsMode;
}

bool OSystem_PS2::setGraphicsMode(int mode) {
	return (mode == 0);
}

int OSystem_PS2::getGraphicsMode(void) const {
	return 0;
}

int OSystem_PS2::getDefaultGraphicsMode(void) const {
	return 0;
}

bool OSystem_PS2::pollEvent(Common::Event &event) {
	bool res;

	if (_modeChanged) {
		_modeChanged = false;
		event.type = Common::EVENT_SCREEN_CHANGED;
		return true;
	}

	res = _input->pollEvent(&event);

	if (res && (event.type == Common::EVENT_MOUSEMOVE))
		_screen->setMouseXy(event.mouse.x, event.mouse.y);
	return res;
}

int16 OSystem_PS2::getHeight(void) {
	return _screen->getHeight();
}

int16 OSystem_PS2::getWidth(void) {
	return _screen->getWidth();
}

void OSystem_PS2::msgPrintf(int millis, const char *format, ...) {
	va_list ap;
	char resStr[1024];
	memset(resStr, 0, 1024);

	va_start(ap, format);
	vsnprintf(resStr, 1023, format, ap);
	va_end(ap);

	uint16 posY = 2;
	int maxWidth = 0;

	Graphics::Surface surf;
	surf.create(300, 200, Graphics::PixelFormat::createFormatCLUT8());

	char *lnSta = resStr;
	while (*lnSta && (posY < 180)) {
		char *lnEnd = lnSta;
		while ((*lnEnd) && (*lnEnd != '\n'))
			lnEnd++;
		*lnEnd = '\0';

		Common::String str(lnSta);
		int width = Graphics::g_sysfont.getStringWidth(str);
		if (width > maxWidth)
			maxWidth = width;
		int posX = (300 - width) / 2;
		Graphics::g_sysfont.drawString(&surf, str, posX, posY, 300 - posX, 1);
		posY += 14;

        lnSta = lnEnd + 1;
	}

	uint8 *scrBuf = (uint8 *)memalign(64, 320 * 200);
	memset(scrBuf, 4, 320 * 200);

	uint8 *dstPos = scrBuf + ((200 - posY) >> 1) * 320 + (320 - maxWidth) / 2;
	for (int y = 0; y < posY; y++) {
		uint8 *srcPos = (uint8 *)surf.getBasePtr((300 - maxWidth) / 2, y);
		for (int x = 0; x < maxWidth; x++)
			dstPos[x] = srcPos[x] + 5;
		dstPos += 320;
	}
	surf.free();
	_screen->copyPrintfOverlay(scrBuf);
	free(scrBuf);
	_msgClearTime = millis + getMillis();
}

void OSystem_PS2::powerOffCallback(void) {
	sioprintf("powerOffCallback\n");
	// _savefileManager->quit(); // romeo
	if (_useHdd) {
		sioprintf("umount\n");
		fio.umount("pfs0:");
	}
	sioprintf("fxio\n");
	// enable blocking FXIO so libhdd will correctly close drive, etc.
	fileXioSetBlockMode(FXIO_WAIT);
	sioprintf("done\n");
}

void OSystem_PS2::quit(void) {
	printf("OSystem_PS2::quit called\n");
	if (_bootDevice == HOST_DEV) {
		printf("OSystem_PS2::quit (HOST)\n");
		SleepThread();
	} else {
		printf("OSystem_PS2::quit (bootdev=%d)\n", _bootDevice);
		if (_useHdd) {
			driveStandby();
			fio.umount("pfs0:");
		}
		//setTimerCallback(NULL, 0);
		_screen->wantAnim(false);
		_systemQuit = true;
		ee_thread_t statSound, statTimer;
		printf("Waiting for timer and sound thread to end\n");
		do {	// wait until both threads called ExitThread()
			ReferThreadStatus(_timerTid, &statTimer);
			ReferThreadStatus(_soundTid, &statSound);
		} while ((statSound.status != 0x10) || (statTimer.status != 0x10));
		printf("Done\n");
		DeleteThread(_timerTid);
		DeleteThread(_soundTid);
		free(_timerStack);
		free(_soundStack);
		printf("Stopping timer\n");
		DisableIntc(INT_TIMER0);
		RemoveIntcHandler(INT_TIMER0, _intrId);

		// _savefileManager->quit(); // romeo
		_screen->quit();

		padEnd(); // stop pad library
		cdvdInit(CDVD_EXIT);
		printf("resetting iop\n");
		SifIopReset(NULL, 0);
		SifExitRpc();
		while (!SifIopSync());
		SifInitRpc(0);
		cdvdExit();
		SifExitRpc();
		FlushCache(0);
		SifLoadFileExit();

		// TODO : let user choose from reboot and forking an ELF on exit

		// reboot (default)
		#if 1

		LoadExecPS2("", 0, NULL);

		// LoadExecPS2("rom0:OSDSYS",0,NULL);

		// ("rom0:OSDSYS", NULL)
		// ("", 0, NULL);

		/* back to PS2 Browser */
/*
		__asm__ __volatile__(
			"   li $3, 0x04;"
			"   syscall;"
			"   nop;"
        );
*/

/*
		SifIopReset("rom0:UNDL ", 0);
		while (!SifIopSync()) ;
		// SifIopReboot(...);
*/
		#else
		// reset + load ELF from CD
		printf("Restarting ScummVM\n");
		LoadExecPS2("cdrom0:\\SCUMMVM.ELF", 0, NULL);
		#endif
	}
}

bool OSystem_PS2::runningFromHost(void) {
	return (_bootDevice == HOST_DEV);
}

bool OSystem_PS2::prepMC() {
	FILE *f;
	bool prep = false;

	if (!mcPresent())
		return prep;

	printf("prepMC 0\n");
	// Common::String str("mc0:ScummVM/")
	// Common::FSNode scumDir(str);
	Common::FSNode scumDir("mc0:ScummVM/");

	printf("prepMC 00\n");

	if (!scumDir.exists()) {
		uint16 *data, size;

		PS2Icon _ico;
		mcIcon icon;

		printf("prepMC I\n");

		size = _ico.decompressData(&data);

		printf("prepMC II\n");

		_ico.setup(&icon);

#ifdef __USE_LIBMC__
		int res;
		mcInit(MC_TYPE_MC);
		mcSync(0, NULL, NULL);
		mcMkDir(0,0,"ScummVM");
		mcSync(0, NULL, &res);
		// TODO : icon
#else
		fio.mkdir("mc0:ScummVM");
		f = ps2_fopen("mc0:ScummVM/scummvm.icn", "w");

		printf("f = %p\n", (const void *)f);

		ps2_fwrite(data, size, 2, f);
		ps2_fclose(f);

		f = ps2_fopen("mc0:ScummVM/icon.sys", "w");

		printf("f = %p\n", (const void *)f);

		ps2_fwrite(&icon, sizeof(icon), 1, f);
		ps2_fclose(f);
#endif
		free(data);

		printf("prepMC II\n");

		prep = true;
	}

	return prep;
}

void OSystem_PS2::makeConfigPath() {
    FILE *src, *dst;
	char path[128], *buf;
	int32 size;

	// Common::FSNode scumIni("mc0:ScummVM/ScummVM.ini"); // gcc bug !

	switch (_bootDevice) {
	case CD_DEV:
	{
		Common::FSNode scumIni("mc0:ScummVM/ScummVM.ini");
		if (!scumIni.exists()) {

			src = ps2_fopen("cdfs:ScummVM.ini", "r");
			if (src) {
				size = ((Ps2File *)src)->size();
				buf = (char *)malloc(size);
				ps2_fread(buf, size, 1, src);
				ps2_fclose(src);

				dst = ps2_fopen("mc0:ScummVM/ScummVM.ini", "w");
				if (dst) {
					ps2_fwrite(buf, size, 1, dst);
					ps2_fclose(dst);
					sprintf(path, "mc0:ScummVM/ScummVM.ini");
				} else {
					sprintf(path, "cdfs:ScummVM.ini");
				}

				free(buf);
			}
		}
	}
	break;

	case MC_DEV:
		sprintf(path, "mc0:ScummVM/ScummVM.ini");
	break;

	case HD_DEV:
	case USB_DEV:
	case HOST_DEV:
		sprintf(path, "%sScummVM.ini", _bootPath);
	break;
	}

	src = ps2_fopen(path, "r");
	if (!src)
		sprintf(path, "mc0:ScummVM/ScummVM.ini");
	else
		ps2_fclose(src);

	_configFile = strdup(path);
}

Common::String OSystem_PS2::getDefaultConfigFileName() {
	return _configFile;
}

void OSystem_PS2::logMessage(LogMessageType::Type type, const char *message) {
	printf("%s", message);
	sioprintf("%s", message);
}
