Configuration &config() {
  static Configuration configuration;
  return configuration;
}

bool Configuration::load(const char *filename) {
  if(configuration::load(filename) == false) return false;
  SNES::config.superfx.speed = max(0, min(2, SNES::config.superfx.speed));
  video.context = (video.isFullscreen == false) ? &video.windowed : &video.fullscreen;
  return true;
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
  attach(system.speedSlowest =  50, "system.speedSlowest");
  attach(system.speedSlow    =  75, "system.speedSlow");
  attach(system.speedNormal  = 100, "system.speedNormal");
  attach(system.speedFast    = 150, "system.speedFast");
  attach(system.speedFastest = 200, "system.speedFastest");
  attach(system.autoSaveMemory = false, "system.autoSaveMemory", "Automatically save cartridge back-up RAM once every minute");
  attach(system.rewindEnabled = false, "system.rewindEnabled", "Automatically save states periodically to allow auto-rewind support");

  attach(diskBrowser.showPanel = true, "diskBrowser.showPanel");

  attach(file.autodetect_type    = false, "file.autodetectType");
  attach(file.applyPatches       = true, "file.applyPatches");
  attach(file.bypass_patch_crc32 = false, "file.bypassPatchCrc32");

  attach(path.rom   = "", "path.rom");
  attach(path.save  = "", "path.save");
  attach(path.state = "", "path.state");
  attach(path.patch = "", "path.patch");
  attach(path.cheat = "", "path.cheat");
  attach(path.data  = "", "path.data");
  attach(path.bsx   = "", "path.bsx");
  attach(path.st    = "", "path.st");
  attach(path.sgb   = "", "path.sgb");
  attach(path.fragmentShader = "", "path.fragmentShader");
  attach(path.vertexShader   = "", "path.vertexShader");

  attach(path.current.folder    = "", "path.current.folder");
  attach(path.current.movie     = "", "path.current.movie");
  attach(path.current.shader    = "", "path.current.shader");
  attach(path.current.cartridge = "", "path.current.cartridge");
  attach(path.current.bsx       = "", "path.current.bsx");
  attach(path.current.st        = "", "path.current.st");
  attach(path.current.sgb       = "", "path.current.sgb");
  attach(path.current.filter    = 0,  "path.current.filter");

  video.context = &video.windowed;
  attach(video.isFullscreen = false, "video.isFullscreen");
  attach(video.synchronize  = false, "video.synchronize");

  attach(video.contrastAdjust   = 0,    "video.contrastAdjust");
  attach(video.brightnessAdjust = 0,    "video.brightnessAdjust");
  attach(video.gammaAdjust      = 0,    "video.gammaAdjust");
  attach(video.scanlineAdjust   = 100,  "video.scanlineAdjust");
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

  attach(input.port1 = ControllerPort1::Gamepad, "input.port1");
  attach(input.port2 = ControllerPort2::Gamepad, "input.port2");
  attach(input.focusPolicy = Input::FocusPolicyIgnoreInput, "input.focusPolicy");
  attach(input.allowInvalidInput = false, "input.allowInvalidInput", "Allow up+down / left+right combinations; may trigger bugs in some games");

  attach(debugger.cacheUsageToDisk = false, "debugger.cacheUsageToDisk");

  attach(geometry.mainWindow       = "", "geometry.mainWindow");
  attach(geometry.loaderWindow     = "", "geometry.loaderWindow");
  attach(geometry.htmlViewerWindow = "", "geometry.htmlViewerWindow");
  attach(geometry.aboutWindow      = "", "geometry.aboutWindow");
  attach(geometry.diskBrowser      = "", "geometry.diskBrowser");
  attach(geometry.folderCreator    = "", "geometry.folderCreator");
  attach(geometry.settingsWindow   = "", "geometry.settingsWindow");
  attach(geometry.toolsWindow      = "", "geometry.toolsWindow");

  attach(geometry.debugger         = "", "geometry.debugger");
  attach(geometry.disassembler     = "", "geometry.disassembler");
  attach(geometry.breakpointEditor = "", "geometry.breakpointEditor");
  attach(geometry.memoryEditor     = "", "geometry.memoryEditor");
  attach(geometry.propertiesViewer = "", "geometry.propertiesViewer");
  attach(geometry.layerToggle      = "", "geometry.layerToggle");
  attach(geometry.vramViewer       = "", "geometry.vramViewer");
  attach(geometry.oamViewer        = "", "geometry.oamViewer");
  attach(geometry.cgramViewer      = "", "geometry.cgramViewer");
  attach(geometry.debuggerOptions  = "", "geometry.debuggerOptions");
}
