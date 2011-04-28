#include "../ui-base.hpp"
State state;

bool State::save(unsigned slot) {
  if(!allowed()) return false;

  SNES::system.runtosave();
  serializer state = SNES::system.serialize();

  file fp;
  bool result = false;
  if(fp.open(name(slot), file::mode_write)) {
    fp.write(state.data(), state.size());
    fp.close();
    result = true;
  }

  if(result) {
    utility.showMessage(string() << "State " << (slot + 1) << " saved.");
  } else {
    utility.showMessage(string() << "Failed to save state " << (slot + 1) << ".");
  }
  return result;
}

bool State::load(unsigned slot) {
  if(!allowed()) return false;

  file fp;
  bool result = false;
  if(fp.open(name(slot), file::mode_read)) {
    unsigned size = fp.size();
    uint8_t *data = new uint8_t[size];
    fp.read(data, size);
    fp.close();
    serializer state(data, size);
    delete[] data;
    result = SNES::system.unserialize(state);
  }

  if(result) {
    utility.showMessage(string() << "State " << (slot + 1) << " loaded.");
    resetHistory();
  } else {
    utility.showMessage(string() << "Failed to load state " << (slot + 1) << ".");
  }
  return result;
}

void State::frame() {
  if(!allowed()) return;
  if(!config().system.rewindEnabled) return;

  //if a full second has passed, automatically capture state
  if(++frameCounter >= (SNES::system.region() == SNES::System::NTSC ? 60 : 50)) {
    frameCounter = 0;
    historyIndex = (historyIndex + 1) % historySize;
    historyCount = min(historyCount + 1, historySize);
    SNES::system.runtosave();
    history[historyIndex] = SNES::system.serialize();
  }
}

void State::resetHistory() {
  historyIndex = 0;
  historyCount = 0;
  frameCounter = 0;
}

bool State::rewind() {
  if(!allowed()) return false;
  if(!config().system.rewindEnabled) return false;

  if(historyCount == 0) return false;
  serializer state(history[historyIndex].data(), history[historyIndex].size());
  bool result = SNES::system.unserialize(state);
  historyIndex = (historyIndex + historySize - 1) % historySize;  //add historySize to prevent underflow
  historyCount--;
  return true;
}

State::State() {
  historySize = 120;
  history = new serializer[historySize];
  for(unsigned i = 0; i < historySize; i++) history[i] = 0;
}

State::~State() {
  delete[] history;
}

//

bool State::allowed() const {
  if(!SNES::cartridge.loaded() || !application.power) return false;
  if(movie.state != Movie::Inactive) return false;
  return cartridge.saveStatesSupported();
}

string State::name(unsigned slot) const {
  string name = filepath(nall::basename(cartridge.fileName), config().path.state);
  name << "-" << (slot + 1) << ".bst";
  return name;
}
