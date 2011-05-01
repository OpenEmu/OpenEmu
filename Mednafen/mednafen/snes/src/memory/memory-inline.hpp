//Memory

unsigned Memory::size() const { return 0; }

//StaticRAM

uint8* StaticRAM::data() { return data_; }
unsigned StaticRAM::size() const { return size_; }

uint8 StaticRAM::read(unsigned addr) { return data_[addr]; }
void StaticRAM::write(unsigned addr, uint8 n) { data_[addr] = n; }
uint8& StaticRAM::operator[](unsigned addr) { return data_[addr]; }
const uint8& StaticRAM::operator[](unsigned addr) const { return data_[addr]; }

StaticRAM::StaticRAM(unsigned n) : size_(n) { data_ = new uint8[size_]; }
StaticRAM::~StaticRAM() { delete[] data_; }

//MappedRAM

void MappedRAM::reset() {
  if(data_) {
    delete[] data_;
    data_ = 0;
  }
  size_ = -1U;
  write_protect_ = false;
}

void MappedRAM::map(uint8 *source, unsigned length) {
  reset();
  data_ = source;
  size_ = data_ && length > 0 ? length : -1U;
}

void MappedRAM::copy(uint8 *data, unsigned size) {
  if(!data_) {
    size_ = (size & ~255) + ((bool)(size & 255) << 8);
    data_ = new uint8[size_]();
  }
  memcpy(data_, data, min(size_, size));
}

void MappedRAM::write_protect(bool status) { write_protect_ = status; }
uint8* MappedRAM::data() { return data_; }
unsigned MappedRAM::size() const { return size_; }

uint8 MappedRAM::read(unsigned addr) { return data_[addr]; }
void MappedRAM::write(unsigned addr, uint8 n) { if(!write_protect_) data_[addr] = n; }
const uint8 MappedRAM::operator[](unsigned addr) const { return data_[addr]; }
MappedRAM::MappedRAM() : data_(0), size_(-1U), write_protect_(false) {}

//Bus

uint8 Bus::read(unsigned addr) {
  #if defined(CHEAT_SYSTEM)
  if(cheat.active() && cheat.exists(addr)) {
    uint8 r;
    if(cheat.read(addr, r)) return r;
  }
  #endif

  Page &p = page[addr >> 8];
  return p.access->read(p.offset + addr);
}

void Bus::write(unsigned addr, uint8 data) {
  Page &p = page[addr >> 8];
  return p.access->write(p.offset + addr, data);
}

bool Bus::load_cart() { return false; }
void Bus::unload_cart() {}

void Bus::power() {}
void Bus::reset() {}
