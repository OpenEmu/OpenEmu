bool Utility::loadCartridgeNormal(const char *base) {
  unloadCartridge();
  if(loadCartridge(cartridge.baseName = base, SNES::memory::cartrom) == false) return false;
  SNES::cartridge.load(SNES::Cartridge::ModeNormal);

  loadMemory(cartridge.baseName, ".srm", SNES::memory::cartram);
  loadMemory(cartridge.baseName, ".rtc", SNES::memory::cartrtc);

  cartridge.name = notdir(nall::basename(cartridge.baseName));

  modifySystemState(LoadCartridge);
  return true;
}

bool Utility::loadCartridgeBsxSlotted(const char *base, const char *slot) {
  unloadCartridge();
  if(loadCartridge(cartridge.baseName = base, SNES::memory::cartrom) == false) return false;
  loadCartridge(cartridge.slotAName = slot, SNES::memory::bsxflash);
  SNES::cartridge.load(SNES::Cartridge::ModeBsxSlotted);

  loadMemory(cartridge.baseName, ".srm", SNES::memory::cartram);
  loadMemory(cartridge.baseName, ".rtc", SNES::memory::cartrtc);

  cartridge.name = notdir(nall::basename(cartridge.baseName));
  if(*slot) cartridge.name << " + " << notdir(nall::basename(cartridge.slotAName));

  modifySystemState(LoadCartridge);
  return true;
}

bool Utility::loadCartridgeBsx(const char *base, const char *slot) {
  unloadCartridge();
  if(loadCartridge(cartridge.baseName = base, SNES::memory::cartrom) == false) return false;
  loadCartridge(cartridge.slotAName = slot, SNES::memory::bsxflash);
  SNES::cartridge.load(SNES::Cartridge::ModeBsx);

  loadMemory(cartridge.baseName, ".srm", SNES::memory::bsxram );
  loadMemory(cartridge.baseName, ".psr", SNES::memory::bsxpram);

  cartridge.name = *slot
  ? notdir(nall::basename(cartridge.slotAName))
  : notdir(nall::basename(cartridge.baseName));

  modifySystemState(LoadCartridge);
  return true;
}

bool Utility::loadCartridgeSufamiTurbo(const char *base, const char *slotA, const char *slotB) {
  unloadCartridge();
  if(loadCartridge(cartridge.baseName = base, SNES::memory::cartrom) == false) return false;
  loadCartridge(cartridge.slotAName = slotA, SNES::memory::stArom);
  loadCartridge(cartridge.slotBName = slotB, SNES::memory::stBrom);
  SNES::cartridge.load(SNES::Cartridge::ModeSufamiTurbo);

  loadMemory(cartridge.slotAName, ".srm", SNES::memory::stAram);
  loadMemory(cartridge.slotBName, ".srm", SNES::memory::stBram);

  if(!*slotA && !*slotB) cartridge.name = notdir(nall::basename(cartridge.baseName));
  else if(!*slotB) cartridge.name = notdir(nall::basename(cartridge.slotAName));
  else if(!*slotA) cartridge.name = notdir(nall::basename(cartridge.slotBName));
  else cartridge.name = notdir(nall::basename(cartridge.slotAName)) << " + " << notdir(nall::basename(cartridge.slotBName));

  modifySystemState(LoadCartridge);
  return true;
}

bool Utility::loadCartridgeSuperGameBoy(const char *base, const char *slot) {
  unloadCartridge();
  if(loadCartridge(cartridge.baseName = base, SNES::memory::cartrom) == false) return false;
  loadCartridge(cartridge.slotAName = slot, SNES::memory::gbrom);
  SNES::cartridge.load(SNES::Cartridge::ModeSuperGameBoy);

  loadMemory(cartridge.slotAName, ".sav", SNES::memory::gbram);
  loadMemory(cartridge.slotBName, ".rtc", SNES::memory::gbrtc);

  cartridge.name = *slot
  ? notdir(nall::basename(cartridge.slotAName))
  : notdir(nall::basename(cartridge.baseName));

  modifySystemState(LoadCartridge);
  return true;
}

void Utility::saveMemory() {
  if(SNES::cartridge.loaded() == false) return;

  switch(SNES::cartridge.mode()) {
    case SNES::Cartridge::ModeNormal:
    case SNES::Cartridge::ModeBsxSlotted: {
      saveMemory(cartridge.baseName, ".srm", SNES::memory::cartram);
      saveMemory(cartridge.baseName, ".rtc", SNES::memory::cartrtc);
    } break;

    case SNES::Cartridge::ModeBsx: {
      saveMemory(cartridge.baseName, ".srm", SNES::memory::bsxram );
      saveMemory(cartridge.baseName, ".psr", SNES::memory::bsxpram);
    } break;

    case SNES::Cartridge::ModeSufamiTurbo: {
      saveMemory(cartridge.slotAName, ".srm", SNES::memory::stAram);
      saveMemory(cartridge.slotBName, ".srm", SNES::memory::stBram);
    } break;

    case SNES::Cartridge::ModeSuperGameBoy: {
      saveMemory(cartridge.slotAName, ".sav", SNES::memory::gbram);
      saveMemory(cartridge.slotAName, ".rtc", SNES::memory::gbrtc);
    } break;
  }
}

void Utility::unloadCartridge() {
  if(SNES::cartridge.loaded() == false) return;
  modifySystemState(UnloadCartridge);
}

void Utility::modifySystemState(system_state_t state) {
  video.clear();
  audio.clear();

  switch(state) {
    case LoadCartridge: {
      //must call cartridge.load_cart_...() before calling modifySystemState(LoadCartridge)
      if(SNES::cartridge.loaded() == false) break;
      loadCheats();

      application.power = true;
      application.pause = false;
      SNES::system.power();

      //warn if unsupported hardware detected
      string chip;
      if(0);
      else if(SNES::cartridge.has_dsp3())  chip = "DSP3";
      else if(SNES::cartridge.has_st011()) chip = "ST011";
      else if(SNES::cartridge.has_st018()) chip = "ST018";
      if(chip != "") {
        QMessageBox::warning(mainWindow, "Warning", utf8()
        << "<p><b>Warning:</b><br> The " << chip << " chip was detected, which is not fully emulated yet.<br>"
        << "It is unlikely that this title will work properly.</p>");
      }

      showMessage(utf8()
      << "Loaded " << cartridge.name
      << (cartridge.patchApplied ? ", and applied UPS patch." : "."));
      mainWindow->setWindowTitle(utf8() << bsnesTitle << " v" << bsnesVersion << " - " << cartridge.name);
      debugger->echo(utf8() << "Loaded " << cartridge.name << ".<br>");
    } break;

    case UnloadCartridge: {
      if(SNES::cartridge.loaded() == false) break;  //no cart to unload?
      saveCheats();

      SNES::system.unload();     //flush all memory to memory::* devices
      saveMemory();              //save memory to disk
      SNES::cartridge.unload();  //deallocate memory

      application.power = false;
      application.pause = true;

      showMessage(utf8() << "Unloaded " << cartridge.name << ".");
      mainWindow->setWindowTitle(utf8() << bsnesTitle << " v" << bsnesVersion);
    } break;

    case PowerOn: {
      if(SNES::cartridge.loaded() == false || application.power == true) break;

      application.power = true;
      application.pause = false;
      SNES::system.power();

      showMessage("Power on.");
    } break;

    case PowerOff: {
      if(SNES::cartridge.loaded() == false || application.power == false) break;

      application.power = false;
      application.pause = true;

      showMessage("Power off.");
    } break;

    case PowerCycle: {
      if(SNES::cartridge.loaded() == false) break;

      application.power = true;
      application.pause = false;
      SNES::system.power();

      showMessage("System power was cycled.");
    } break;

    case Reset: {
      if(SNES::cartridge.loaded() == false || application.power == false) break;

      application.pause = false;
      SNES::system.reset();

      showMessage("System was reset.");
    } break;
  }

  mainWindow->syncUi();
  debugger->synchronize();
  cheatEditorWindow->reloadList();
  cheatFinderWindow->synchronize();
  stateManagerWindow->reloadList();
}

bool Utility::loadCartridge(string &filename, SNES::MappedRAM &memory) {
  if(file::exists(filename) == false) return false;

  uint8_t *data;
  unsigned size;
  audio.clear();
  if(reader.load(filename, data, size) == false) return false;

  cartridge.patchApplied = false;
  string name;
  name << filepath(nall::basename(filename), config.path.patch);
  name << ".ups";

  file fp;
  if(fp.open(name, file::mode_read) == true) {
    unsigned patchsize = fp.size();
    uint8_t *patchdata = new uint8_t[patchsize];
    fp.read(patchdata, patchsize);
    fp.close();

    uint8_t *outdata = 0;
    unsigned outsize = 0;
    ups patcher;
    ups::result result = patcher.apply(patchdata, patchsize, data, size, outdata, outsize);
    delete[] patchdata;

    bool apply = false;
    if(result == ups::ok) apply = true;
    if(config.file.bypass_patch_crc32) {
      if(result == ups::input_crc32_invalid ) apply = true;
      if(result == ups::output_crc32_invalid) apply = true;
    }

    if(apply == true) {
      delete[] data;
      data = outdata;
      size = outsize;
      cartridge.patchApplied = true;
    } else {
      delete[] outdata;
    }
  }

  memory.copy(data, size);
  delete[] data;
  return true;
}

bool Utility::loadMemory(const char *filename, const char *extension, SNES::MappedRAM &memory) {
  if(memory.size() == 0 || memory.size() == -1U) return false;

  string name;
  name << filepath(nall::basename(filename), config.path.save);
  name << extension;

  file fp;
  if(fp.open(name, file::mode_read) == false) return false;

  unsigned size = fp.size();
  uint8_t *data = new uint8_t[size];
  fp.read(data, size);
  fp.close();

  memory.copy(data, size);
  delete[] data;
  return true;
}

bool Utility::saveMemory(const char *filename, const char *extension, SNES::MappedRAM &memory) {
  if(memory.size() == 0 || memory.size() == -1U) return false;

  string name;
  name << filepath(nall::basename(filename), config.path.save);
  name << extension;

  file fp;
  if(fp.open(name, file::mode_write) == false) return false;

  fp.write(memory.data(), memory.size());
  fp.close();
  return true;
}

void Utility::loadCheats() {
  string name, data;
  name << filepath(nall::basename(cartridge.baseName), config.path.cheat);
  name << ".cht";

  SNES::cheat.clear();
  if(data.readfile(name)) SNES::cheat.load(data);
}

void Utility::saveCheats() {
  string name;
  name << filepath(nall::basename(cartridge.baseName), config.path.cheat);
  name << ".cht";

  file fp;
  if(SNES::cheat.count() > 0 || file::exists(name)) {
    if(fp.open(name, file::mode_write)) {
      fp.print(SNES::cheat.save());
      fp.close();
    }
  }
}

//ensure file path is absolute (eg resolve relative paths)
string Utility::filepath(const char *filename, const char *pathname) {
  //if no pathname, return filename as-is
  string file(filename);
  file.replace("\\", "/");

  string path = (!pathname || !*pathname) ? (const char*)dir(filename) : pathname;
  //ensure path ends with trailing '/'
  path.replace("\\", "/");
  if(!strend(path, "/")) path.append("/");

  //replace relative path with absolute path
  if(strbegin(path, "./")) {
    ltrim(path, "./");
    path = string() << config.path.base << path;
  }

  //remove folder part of filename
  lstring part;
  part.split("/", file);
  return path << part[part.size() - 1];
}
