//not all special chips support serialization yet.
//attempting to load such a state would almost certainly crash the game. thus,
//this function is used to disable save state support globally for these titles.
bool Utility::saveStatesSupported() {
  if(SNES::cartridge.mode() == SNES::Cartridge::ModeBsx) return false;
  if(SNES::cartridge.mode() == SNES::Cartridge::ModeSuperGameBoy) return false;

  if(SNES::cartridge.has_superfx()) return false;
  if(SNES::cartridge.has_sa1())     return false;
  if(SNES::cartridge.has_srtc())    return false;
  if(SNES::cartridge.has_sdd1())    return false;
  if(SNES::cartridge.has_cx4())     return false;
  if(SNES::cartridge.has_dsp1())    return false;
  if(SNES::cartridge.has_dsp2())    return false;
  if(SNES::cartridge.has_dsp3())    return false;
  if(SNES::cartridge.has_dsp4())    return false;
  if(SNES::cartridge.has_st010())   return false;
  if(SNES::cartridge.has_st011())   return false;
  if(SNES::cartridge.has_st018())   return false;

  return true;
}

void Utility::quickLoad(uint8 slot) {
  if(SNES::cartridge.loaded() == false || application.power == false) return;
  if(saveStatesSupported() == false) {
    showMessage(utf8() << "Error: cannot save state due to unserializable special chip.");
    return;
  }

  string name;
  name << filepath(basename(cartridge.baseName), config.path.state);
  name << "-" << (unsigned)(slot + 1) << ".bst";

  file fp;
  if(fp.open(name, file::mode_read) == false) {
    showMessage(utf8() << "Error: unable to load quick state " << (unsigned)(slot + 1) << ".");
    return;
  }

  unsigned size = fp.size();
  uint8 *data = new uint8[size];
  fp.read(data, size);
  fp.close();

  serializer state(data, size);
  if(SNES::system.unserialize(state) == true) {
    showMessage(utf8() << "Quick state " << (unsigned)(slot + 1) << " loaded.");
  } else {
    showMessage(utf8() << "Error: unable to load quick state " << (unsigned)(slot + 1) << ".");
  }
}

void Utility::quickSave(uint8 slot) {
  if(SNES::cartridge.loaded() == false || application.power == false) return;
  if(saveStatesSupported() == false) {
    showMessage(utf8() << "Error: cannot save state due to unserializable special chip.");
    return;
  }

  SNES::system.runtosave();

  string name;
  name << filepath(basename(cartridge.baseName), config.path.state);
  name << "-" << (unsigned)(slot + 1) << ".bst";

  serializer state = SNES::system.serialize();
  file fp;
  if(fp.open(name, file::mode_write) == false) {
    showMessage(utf8() << "Error: unable to save quick state " << (unsigned)(slot + 1) << ".");
    return;
  }

  fp.write(state.data(), state.size());
  fp.close();
  showMessage(utf8() << "Quick state " << (unsigned)(slot + 1) << " saved.");
}

void Utility::loadStateInfo(lstring &info) {
  info.reset();
  string name = string() << filepath(basename(cartridge.baseName), config.path.state) << ".bsa";
  if(SNES::statemanager.load(name) == false) return;
  SNES::statemanager.list(info);
}

void Utility::setStateDescription(uint8 slot, const char *description) {
  if(SNES::cartridge.loaded() == false) return;
  string name = string() << filepath(basename(cartridge.baseName), config.path.state) << ".bsa";
  SNES::statemanager.set_description(name, slot, description);
}

void Utility::loadState(uint8 slot) {
  if(SNES::cartridge.loaded() == false || application.power == false) return;
  string name = string() << filepath(basename(cartridge.baseName), config.path.state) << ".bsa";

  try {
    serializer state = SNES::statemanager.load(name, slot);

    if(SNES::system.unserialize(state) == true) {
      showMessage(utf8() << "State loaded.");
    } else {
      showMessage(utf8() << "Error: unable to load state.");
    }
  } catch(...) {
    showMessage(utf8() << "Error: unable to load state.");
    return;
  }
}

void Utility::saveState(uint8 slot, const char *description) {
  if(SNES::cartridge.loaded() == false || application.power == false) return;
  SNES::system.runtosave();

  serializer state = SNES::system.serialize();
  string name = string() << filepath(basename(cartridge.baseName), config.path.state) << ".bsa";
  if(SNES::statemanager.save(name, slot, state, description) == true) {
    showMessage(utf8() << "State saved.");
  } else {
    showMessage(utf8() << "Error: unable to save state.");
  }
}

void Utility::deleteState(uint8 slot) {
  if(SNES::cartridge.loaded() == false) return;
  string name = string() << filepath(basename(cartridge.baseName), config.path.state) << ".bsa";
  if(SNES::statemanager.erase(name, slot) == true) {
    showMessage(utf8() << "State deleted.");
  } else {
    showMessage(utf8() << "Error: unable to delete state.");
  }
}
