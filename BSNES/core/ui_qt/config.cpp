bool Configuration::load(const char *filename) {
  if(configuration::load(filename) == false) return false;
  SNES::config.superfx.speed = max(0, min(2, SNES::config.superfx.speed));
  video.context = (video.isFullscreen == false) ? &video.windowed : &video.fullscreen;
  return true;
}

void Configuration::attachJoypad(Configuration::Input::Joypad &joypad, const char *name) {
  attach(joypad.up     = "none", string() << name << ".up");
  attach(joypad.down   = "none", string() << name << ".down");
  attach(joypad.left   = "none", string() << name << ".left");
  attach(joypad.right  = "none", string() << name << ".right");
  attach(joypad.a      = "none", string() << name << ".a");
  attach(joypad.b      = "none", string() << name << ".b");
  attach(joypad.x      = "none", string() << name << ".x");
  attach(joypad.y      = "none", string() << name << ".y");
  attach(joypad.l      = "none", string() << name << ".l");
  attach(joypad.r      = "none", string() << name << ".r");
  attach(joypad.select = "none", string() << name << ".select");
  attach(joypad.start  = "none", string() << name << ".start");
}

Configuration::Configuration() {
  //========
  //external
  //========

  attach(SNES::config.controller_port1 = SNES::Input::DeviceJoypad,  "snes.controllerPort1");
  attach(SNES::config.controller_port2 = SNES::Input::DeviceJoypad,  "snes.controllerPort2");
  attach(SNES::config.expansion_port   = SNES::System::ExpansionBSX, "snes.expansionPort");
  attach(SNES::config.region           = SNES::System::Autodetect,   "snes.region");

  attach(SNES::config.cpu.version         =        2, "cpu.version", "Valid version(s) are: 1, 2");
  attach(SNES::config.cpu.ntsc_clock_rate = 21477272, "cpu.ntscClockRate");
  attach(SNES::config.cpu.pal_clock_rate  = 21281370, "cpu.palClockRate");
  attach(SNES::config.cpu.alu_mul_delay   =        2, "cpu.aluMulDelay");
  attach(SNES::config.cpu.alu_div_delay   =        2, "cpu.aluDivDelay");
  attach(SNES::config.cpu.wram_init_value =     0x55, "cpu.wramInitValue");

  attach(SNES::config.smp.ntsc_clock_rate = 24607104, "smp.ntscClockRate");
  attach(SNES::config.smp.pal_clock_rate  = 24607104, "smp.palClockRate");

  attach(SNES::config.ppu1.version = 1, "ppu1.version", "Valid version(s) are: 1");
  attach(SNES::config.ppu2.version = 3, "ppu2.version", "Valid version(s) are: 1, 2, 3");

  attach(SNES::config.superfx.speed = 0, "superfx.speed", "0 = Auto-select, 1 = Force 10.74MHz, 2 = Force 21.48MHz");

  //========
  //internal
  //========

  attach(system.video = "", "system.video");
  attach(system.audio = "", "system.audio");
  attach(system.input = "", "system.input");
  attach(system.crashedOnLastRun = false, "system.crashedOnLastRun");
  attach(system.speed = 2, "system.speed");
  attach(system.autoSaveMemory = false, "system.autoSaveMemory", "Automatically save cartridge back-up RAM once every minute");
  attach(system.autoHideMenus = false, "system.autoHideMenus", "Automatically hide the menu and status bars when entering fullscreen mode");

  attach(diskBrowser.showPanel = true, "diskBrowser.showPanel");

  attach(file.autodetect_type    = false, "file.autodetectType");
  attach(file.bypass_patch_crc32 = false, "file.bypassPatchCrc32");

  attach(path.current = "", "path.current");
  attach(path.rom     = "", "path.rom");
  attach(path.save    = "", "path.save");
  attach(path.state   = "", "path.state");
  attach(path.patch   = "", "path.patch");
  attach(path.cheat   = "", "path.cheat");
  attach(path.data    = "", "path.data");
  attach(path.bsx     = "", "path.bsx");
  attach(path.st      = "", "path.st");
  attach(path.sgb     = "", "path.sgb");

  video.context = &video.windowed;
  attach(video.isFullscreen = false, "video.isFullscreen");
  attach(video.synchronize  = false, "video.synchronize");

  attach(video.contrastAdjust   = 0,    "video.contrastAdjust");
  attach(video.brightnessAdjust = 0,    "video.brightnessAdjust");
  attach(video.gammaAdjust      = 0,    "video.gammaAdjust");
  attach(video.enableGammaRamp  = true, "video.enableGammaRamp");

  attach(video.ntscAspectRatio = 54.0 / 47.0, "video.ntscAspectRatio", "NTSC aspect ratio (x / y)");
  attach(video.palAspectRatio  = 32.0 / 23.0, "video.palAspectRatio",  "PAL aspect ratio (x / y)");

  attach(video.windowed.correctAspectRatio = true, "video.windowed.correctAspectRatio");
  attach(video.windowed.multiplier         =    2, "video.windowed.multiplier");
  attach(video.windowed.region             =    0, "video.windowed.region");

  attach(video.windowed.hwFilter = 1, "video.windowed.hwFilter");
  attach(video.windowed.swFilter = 0, "video.windowed.swFilter");

  attach(video.fullscreen.correctAspectRatio = true, "video.fullscreen.correctAspectRatio");
  attach(video.fullscreen.multiplier         =    9, "video.fullscreen.multiplier");
  attach(video.fullscreen.region             =    0, "video.fullscreen.region");

  attach(video.fullscreen.hwFilter = 1, "video.fullscreen.hwFilter");
  attach(video.fullscreen.swFilter = 0, "video.fullscreen.swFilter");

  attach(audio.synchronize = true,  "audio.synchronize");
  attach(audio.mute        = false, "audio.mute");

  attach(audio.volume          =   100, "audio.volume");
  attach(audio.latency         =    80, "audio.latency");
  attach(audio.outputFrequency = 48000, "audio.outputFrequency");
  attach(audio.inputFrequency  = 32000, "audio.inputFrequency");

  attach(input.focusPolicy = Input::FocusPolicyIgnoreInput, "input.focusPolicy");
  attach(input.allowInvalidInput = false, "input.allowInvalidInput", "Allow up+down / left+right combinations; may trigger bugs in some games");

  attach(input.joypad1.up     = "keyboard00.up",     "input.joypad1.up");
  attach(input.joypad1.down   = "keyboard00.down",   "input.joypad1.down");
  attach(input.joypad1.left   = "keyboard00.left",   "input.joypad1.left");
  attach(input.joypad1.right  = "keyboard00.right",  "input.joypad1.right");
  attach(input.joypad1.a      = "keyboard00.x",      "input.joypad1.a");
  attach(input.joypad1.b      = "keyboard00.z",      "input.joypad1.b");
  attach(input.joypad1.x      = "keyboard00.s",      "input.joypad1.x");
  attach(input.joypad1.y      = "keyboard00.a",      "input.joypad1.y");
  attach(input.joypad1.l      = "keyboard00.d",      "input.joypad1.l");
  attach(input.joypad1.r      = "keyboard00.c",      "input.joypad1.r");
  attach(input.joypad1.select = "keyboard00.rshift", "input.joypad1.select");
  attach(input.joypad1.start  = "keyboard00.return", "input.joypad1.start");

  //these are all mapped to "none" by default
  attachJoypad(input.joypad2,    "input.joypad2");
  attachJoypad(input.multitap1a, "input.multitap1a");
  attachJoypad(input.multitap1b, "input.multitap1b");
  attachJoypad(input.multitap1c, "input.multitap1c");
  attachJoypad(input.multitap1d, "input.multitap1d");
  attachJoypad(input.multitap2a, "input.multitap2a");
  attachJoypad(input.multitap2b, "input.multitap2b");
  attachJoypad(input.multitap2c, "input.multitap2c");
  attachJoypad(input.multitap2d, "input.multitap2d");

  attach(input.mouse1.x     = "mouse00.x",        "input.mouse1.x");
  attach(input.mouse1.y     = "mouse00.y",        "input.mouse1.y");
  attach(input.mouse1.left  = "mouse00.button00", "input.mouse1.left");
  attach(input.mouse1.right = "mouse00.button02", "input.mouse1.right");

  //more likely a user will only use one mouse at a time, than for a system to have two mice
  attach(input.mouse2.x     = "mouse00.x",        "input.mouse2.x");
  attach(input.mouse2.y     = "mouse00.y",        "input.mouse2.y");
  attach(input.mouse2.left  = "mouse00.button00", "input.mouse2.left");
  attach(input.mouse2.right = "mouse00.button02", "input.mouse2.right");

  //unlikely a user will have a five-button mouse for turbo / pause mapping
  attach(input.superscope.x       = "mouse00.x",        "input.superscope.x");
  attach(input.superscope.y       = "mouse00.y",        "input.superscope.y");
  attach(input.superscope.trigger = "mouse00.button00", "input.superscope.trigger");
  attach(input.superscope.cursor  = "mouse00.button02", "input.superscope.cursor");
  attach(input.superscope.turbo   = "keyboard00.t",     "input.superscope.turbo");
  attach(input.superscope.pause   = "keyboard00.p",     "input.superscope.pause");

  attach(input.justifier1.x       = "mouse00.x",        "input.justifier1.x");
  attach(input.justifier1.y       = "mouse00.y",        "input.justifier1.y");
  attach(input.justifier1.trigger = "mouse00.button00", "input.justifier1.trigger");
  attach(input.justifier1.start   = "mouse00.button02", "input.jusitifer1.start");

  attach(input.justifier2.x       = "mouse01.x",        "input.justifier2.x");
  attach(input.justifier2.y       = "mouse01.y",        "input.justifier2.y");
  attach(input.justifier2.trigger = "mouse01.button00", "input.justifier2.trigger");
  attach(input.justifier2.start   = "mouse01.button02", "input.justifier2.start");

  attach(input.uiGeneral.loadCartridge     = "none",                "input.uiGeneral.loadCartridge");
  attach(input.uiGeneral.pauseEmulation    = "keyboard00.pause",    "input.uiGeneral.pauseEmulation");
  attach(input.uiGeneral.resetSystem       = "none",                "input.uiGeneral.resetSystem");
  attach(input.uiGeneral.powerCycleSystem  = "none",                "input.uiGeneral.powerCycleSystem");

  attach(input.uiGeneral.captureScreenshot = "none",          "input.uiGeneral.captureScreenshot");
  attach(input.uiGeneral.showStateManager  = "keyboard00.f3", "input.uiGeneral.showStateManager");

  attach(input.uiGeneral.quickLoad1     = "keyboard00.f4", "input.uiGeneral.quickLoad1");
  attach(input.uiGeneral.quickLoad2     = "none",          "input.uiGeneral.quickLoad2");
  attach(input.uiGeneral.quickLoad3     = "none",          "input.uiGeneral.quickLoad3");
  attach(input.uiGeneral.quickSave1     = "keyboard00.f2", "input.uiGeneral.quickSave1");
  attach(input.uiGeneral.quickSave2     = "none",          "input.uiGeneral.quickSave2");
  attach(input.uiGeneral.quickSave3     = "none",          "input.uiGeneral.quickSave3");

  attach(input.uiGeneral.lowerSpeed        = "keyboard00.divide",   "input.uiGeneral.lowerSpeed");
  attach(input.uiGeneral.raiseSpeed        = "keyboard00.multiply", "input.uiGeneral.raiseSpeed");
  attach(input.uiGeneral.toggleCheatSystem = "none",                "input.uiGeneral.toggleCheatSystem");
  attach(input.uiGeneral.toggleFullscreen  = "keyboard00.f11",      "input.uiGeneral.toggleFullscreen");
  attach(input.uiGeneral.toggleMenu        = "keyboard00.escape",   "input.uiGeneral.toggleMenu");
  attach(input.uiGeneral.toggleStatus      = "keyboard00.escape",   "input.uiGeneral.toggleStatus");
  attach(input.uiGeneral.exitEmulator      = "none",                "input.uiGeneral.exitEmulator");

  attach(geometry.mainWindow              = "", "geometry.mainWindow");
  attach(geometry.loaderWindow            = "", "geometry.loaderWindow");
  attach(geometry.htmlViewerWindow        = "", "geometry.htmlViewerWindow");
  attach(geometry.aboutWindow             = "", "geometry.aboutWindow");
  attach(geometry.diskBrowser             = "", "geometry.diskBrowser");
  attach(geometry.folderCreator           = "", "geometry.folderCreator");
  attach(geometry.settingsWindow          = "", "geometry.settingsWindow");
  attach(geometry.inputCaptureWindow      = "", "geometry.inputCaptureWindow");
  attach(geometry.inputMouseCaptureWindow = "", "geometry.inputMouseCaptureWindow");
  attach(geometry.inputCalibrationWindow  = "", "geometry.inputCalibrationWindow");
  attach(geometry.toolsWindow             = "", "geometry.toolsWindow");
  attach(geometry.debugger                = "", "geometry.debugger");
  attach(geometry.breakpointEditor        = "", "geometry.breakpointEditor");
  attach(geometry.memoryEditor            = "", "geometry.memoryEditor");
  attach(geometry.vramViewer              = "", "geometry.vramViewer");
}
