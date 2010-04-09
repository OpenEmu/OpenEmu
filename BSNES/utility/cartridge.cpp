string Utility::selectCartridge() {
  audio.clear();
  application.timer->stop();
  QString filename = QFileDialog::getOpenFileName(0,
    "Load Cartridge",
    utf8() << (config.path.rom != "" ? config.path.rom : config.path.current),
    "SNES images (*.smc *.sfc *.swc *.fig *.bs *.st"
    #if defined(GZIP_SUPPORT)
    " *.zip *.gz"
    #endif
    #if defined(JMA_SUPPORT)
    " *.jma"
    #endif
    ");;"
    "All files (*)"
  );
  application.timer->start(0);

  string name = filename.toUtf8().constData();
  if(strlen(name) > 0) config.path.current = basepath(name);
  return name;
}

string Utility::selectFolder(const char *title) {
  audio.clear();
  application.timer->stop();
  QString pathname = QFileDialog::getExistingDirectory(0,
    title, utf8() << config.path.current,
    QFileDialog::ShowDirsOnly);
  application.timer->start(0);

  string path = pathname.toUtf8().constData();
  strtr(path, "\\", "/");
  if(path.length() > 0 && strend(path, "/") == false) path << "/";
  return path;
}

void Utility::loadCartridge(const char *filename) {
  SNES::MappedRAM memory;
  if(loadCartridge(filename, memory) == false) return;
  SNES::Cartridge::Type type = SNES::cartridge.detect_image_type(memory.data(), memory.size());
  memory.reset();

  switch(type) {
    case SNES::Cartridge::TypeNormal:           loadCartridgeNormal(filename);                                     break;
    case SNES::Cartridge::TypeBsxSlotted:       loaderWindow->loadBsxSlottedCartridge(filename, "");                  break;
    case SNES::Cartridge::TypeBsxBios:          loaderWindow->loadBsxCartridge(filename, "");                         break;
    case SNES::Cartridge::TypeBsx:              loaderWindow->loadBsxCartridge(config.path.bsx, filename);            break;
    case SNES::Cartridge::TypeSufamiTurboBios:  loaderWindow->loadSufamiTurboCartridge(filename, "", "");             break;
    case SNES::Cartridge::TypeSufamiTurbo:      loaderWindow->loadSufamiTurboCartridge(config.path.st, filename, ""); break;
    case SNES::Cartridge::TypeSuperGameBoyBios: loaderWindow->loadSuperGameBoyCartridge(filename, "");                break;
    case SNES::Cartridge::TypeGameBoy:          loaderWindow->loadSuperGameBoyCartridge(config.path.sgb, filename);   break;
  }
}

bool Utility::loadCartridgeNormal(const char *base) {
  unloadCartridge();
  if(loadCartridge(cartridge.baseName = base, SNES::memory::cartrom) == false) return false;
  SNES::cartridge.load(SNES::Cartridge::ModeNormal);

  loadMemory(cartridge.baseName, ".srm", SNES::memory::cartram);
  loadMemory(cartridge.baseName, ".rtc", SNES::memory::cartrtc);

  cartridge.name = basename(base);

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

  cartridge.name = basename(base);
  if(*slot) cartridge.name << " + " << basename(slot);

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

  cartridge.name = (*slot ? basename(slot) : basename(base));

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

  if(!*slotA && !*slotB) cartridge.name = basename(base);
  else if(!*slotB) cartridge.name = basename(slotA);
  else if(!*slotA) cartridge.name = basename(slotB);
  else cartridge.name = string() << basename(slotA) << " + " << basename(slotB);

  modifySystemState(LoadCartridge);
  return true;
}

bool Utility::loadCartridgeSuperGameBoy(const char *base, const char *slot) {
  unloadCartridge();
  if(loadCartridge(cartridge.baseName = base, SNES::memory::cartrom) == false) return false;
  loadCartridge(cartridge.slotAName = slot, SNES::memory::gbrom);
  SNES::cartridge.load(SNES::Cartridge::ModeSuperGameBoy);

  loadMemory(cartridge.slotAName, ".sav", SNES::memory::gbram);

  cartridge.name = (*slot ? basename(slot) : basename(base));

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
    } break;
  }
}

void Utility::unloadCartridge() {
  if(SNES::cartridge.loaded() == false) return;
  saveMemory();
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
        QMessageBox::warning(mainWindow->window, "Warning", utf8()
        << "<p><b>Warning:</b><br> The " << chip << " chip was detected, which is not fully emulated yet.<br>"
        << "It is unlikely that this title will work properly.</p>");
      }

      showMessage(utf8()
      << "Loaded " << cartridge.name
      << (cartridge.patchApplied ? ", and applied UPS patch." : "."));
      mainWindow->window->setWindowTitle(utf8() << bsnesTitle << " v" << bsnesVersion << " - " << cartridge.name);
      debugger->echo(utf8() << "Loaded " << cartridge.name << ".\n");
    } break;

    case UnloadCartridge: {
      if(SNES::cartridge.loaded() == false) break;  //no cart to unload?
      saveCheats();

      SNES::cartridge.unload();

      application.power = false;
      application.pause = true;

      showMessage(utf8() << "Unloaded " << cartridge.name << ".");
      mainWindow->window->setWindowTitle(utf8() << bsnesTitle << " v" << bsnesVersion);
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
  debugger->syncUi();
  cheatEditorWindow->reloadList();
  stateManagerWindow->reloadList();
}

bool Utility::loadCartridge(const char *filename, SNES::MappedRAM &memory) {
  if(file::exists(filename) == false) return false;
  Reader::Type filetype = Reader::detect(filename, config.file.autodetect_type);

  uint8_t *data;
  unsigned size;

  switch(filetype) { default:
    case Reader::Normal: {
      FileReader fp(filename);
      if(!fp.ready()) return false;
      size = fp.size();
      data = fp.read();
    } break;

    #ifdef GZIP_SUPPORT
    case Reader::GZIP: {
      GZReader fp(filename);
      if(!fp.ready()) return false;
      size = fp.size();
      data = fp.read();
    } break;

    case Reader::ZIP: {
      ZipReader fp(filename);
      if(!fp.ready()) return false;
      size = fp.size();
      data = fp.read();
    } break;
    #endif

    #ifdef JMA_SUPPORT
    case Reader::JMA: {
      try {
        JMAReader fp(filename);
        size = fp.size();
        data = fp.read();
      } catch(JMA::jma_errors) {
        return false;
      }
    } break;
    #endif
  }

  //remove copier header, if it exists
  if((size & 0x7fff) == 512) memmove(data, data + 512, size -= 512);

  cartridge.patchApplied = false;
  string name;
  name << filepath(basename(filename), config.path.patch);
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
  name << filepath(basename(filename), config.path.save);
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
  name << filepath(basename(filename), config.path.save);
  name << extension;

  file fp;
  if(fp.open(name, file::mode_write) == false) return false;

  fp.write(memory.data(), memory.size());
  fp.close();
  return true;
}

void Utility::loadCheats() {
  string name, data;
  name << filepath(basename(cartridge.baseName), config.path.cheat);
  name << ".cht";

  SNES::cheat.clear();
  if(data.readfile(name)) SNES::cheat.load(data);
}

void Utility::saveCheats() {
  string name;
  name << filepath(basename(cartridge.baseName), config.path.cheat);
  name << ".cht";

  file fp;
  if(SNES::cheat.count() > 0 || file::exists(name)) {
    if(fp.open(name, file::mode_write)) {
      fp.print(SNES::cheat.save());
      fp.close();
    }
  }
}

//

//ensure file path is absolute (eg resolve relative paths)
string Utility::filepath(const char *filename, const char *pathname) {
  //if no pathname, return filename as-is
  string file(filename);
  file.replace("\\", "/");

  string path = (!pathname || !*pathname) ? (const char*)config.path.current : pathname;
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

//remove directory information and file extension ("/foo/bar.ext" -> "bar")
string Utility::basename(const char *filename) {
  string name(filename);

  //remove extension
  for(signed i = strlen(name) - 1; i >= 0; i--) {
    if(name[i] == '.') {
      name[i] = 0;
      break;
    }
  }

  //remove directory information
  for(signed i = strlen(name) - 1; i >= 0; i--) {
    if(name[i] == '/' || name[i] == '\\') {
      i++;
      char *output = name();
      while(true) {
        *output++ = name[i];
        if(!name[i]) break;
        i++;
      }
      break;
    }
  }

  return name;
}

//remove filename and return path only ("/foo/bar.ext" -> "/foo/")
string Utility::basepath(const char *filename) {
  string path(filename);
  path.replace("\\", "/");

  //remove filename
  for(signed i = strlen(path) - 1; i >= 0; i--) {
    if(path[i] == '/') {
      path[i] = 0;
      break;
    }
  }

  if(!strend(path, "/")) path.append("/");
  return path;
}

