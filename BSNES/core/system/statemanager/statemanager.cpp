StateManager statemanager;

void StateManager::list(lstring &l) const {
  l.reset();

  for(unsigned x = 0; x < info.slotcount; x++) {
    for(unsigned y = 0; y < info.slotcount; y++) {
      if(info.slot[y] == x) {
        unsigned n = l.size();
        char slot[8];
        sprintf(slot, "%3u", x + 1);
        l[n] << slot << "\t";

        char datetime[DateTimeSize + 1];
        memcpy(datetime, &info.datetime[y * DateTimeSize], DateTimeSize);
        datetime[DateTimeSize] = 0;
        l[n] << datetime << "\t";

        char desc[DescriptionSize + 1];
        memcpy(desc, &info.description[y * DescriptionSize], DescriptionSize);
        desc[DescriptionSize] = 0;
        l[n] << desc;
      }
    }
  }
}

bool StateManager::set_description(const char *filename, uint8 slot, const char *description) {
  if(load(filename) == false) return false;
  if(info.slotcount <= slot) return false;
  file fp;
  if(fp.open(filename, file::mode_readwrite) == false) return false;

  uint8 index = findslot(slot);
  if(index == SlotInvalid) { fp.close(); return false; }

  char desc[DescriptionSize];
  memset(&desc, 0, DescriptionSize);
  strlcpy(desc, description, DescriptionSize);

  fp.seek(DescIndex + index * DescriptionSize);
  fp.write((uint8*)&desc[0], DescriptionSize);
  fp.close();
  return true;
}

serializer StateManager::load(const char *filename, uint8 slot) {
  if(load(filename) == false) throw;
  if(info.slotcount <= slot) throw;
  uint8 index = findslot(slot);
  if(index == SlotInvalid) throw;
  file fp;
  if(fp.open(filename, file::mode_read) == false) throw;

  fp.seek(HeaderSize + system.serialize_size * index);
  uint8 *data = new uint8[system.serialize_size];
  fp.read(data, system.serialize_size);
  serializer s(data, system.serialize_size);
  delete[] data;
  fp.close();
  return s;
}

bool StateManager::save(const char *filename, uint8 slot, serializer &s, const char *description) {
  //if no state archive exists ...
  if(file::exists(filename) == false) {
    //try and create one
    if(create(filename) == false) return false;
  }
  //if we cannot load the existing state archive ...
  if(load(filename) == false) {
    //it's probably an older version, try and create a new one
    if(create(filename) == false) return false;
    //it still needs to be loaded before we can write to it
    if(load(filename) == false) return false;
  }

  uint8 index = findslot(slot);
  if(index == SlotInvalid) {
    //create new slot instead of over-writing existing slot
    if(info.slotcount >= 255) return false;
    index = info.slotcount;
    slot  = info.slotcount;
  }

  file fp;
  if(fp.open(filename, file::mode_readwrite) == false) return false;

  fp.seek(SlotIndex + index);
  fp.write(slot);

  time_t current = time(0);
  tm *ts = localtime(&current);
  char timestamp[32];
  sprintf(timestamp, "%.4u-%.2u-%.2u %.2u:%.2u:%.2u",
    1900 + ts->tm_year, ts->tm_mon + 1, ts->tm_mday,
    ts->tm_hour, ts->tm_min, ts->tm_sec,
    (ts->tm_hour < 12 ? "AM" : "PM")
  );
  fp.seek(DateTimeIndex + index * DateTimeSize);
  fp.write((uint8*)&timestamp[0], DateTimeSize);

  char desc[DescriptionSize];
  memset(&desc, 0, DescriptionSize);
  strlcpy(desc, description, DescriptionSize);
  fp.seek(DescIndex + index * DescriptionSize);
  fp.write((uint8*)&desc[0], DescriptionSize);

  fp.seek(HeaderSize + index * system.serialize_size);
  fp.write(s.data(), s.size());
  for(unsigned n = 0; n < system.serialize_size - s.size(); n++) fp.write(0x00);

  fp.close();
  return true;
}

bool StateManager::erase(const char *filename, uint8 slot) {
  if(load(filename) == false) return false;
  uint8 index = findslot(slot);
  if(index == SlotInvalid) return false;

  file fp;
  if(fp.open(filename, file::mode_readwrite) == false) return false;
  if(info.slotcount <= slot) return false;

  //copy the very last state to the slot that is to be erased
  uint8 lastslot = info.slotcount - 1;
  info.slot[index] = info.slot[lastslot];
  info.slot[lastslot] = SlotInvalid;

  fp.seek(DateTimeIndex + index * DateTimeSize);
  fp.write((uint8*)&info.datetime[lastslot * DateTimeSize], DateTimeSize);

  fp.seek(DescIndex + index * DescriptionSize);
  fp.write((uint8*)&info.description[lastslot * DescriptionSize], DescriptionSize);

  fp.seek(HeaderSize + system.serialize_size * lastslot);
  uint8 *data = new uint8[system.serialize_size];
  fp.read(data, system.serialize_size);

  fp.seek(HeaderSize + system.serialize_size * index);
  fp.write(data, system.serialize_size);
  delete[] data;

  //decrement all IDs after the deleted one (removes empty slot ID from deletion)
  for(unsigned n = 0; n < lastslot; n++) {
    if(info.slot[n] > slot) info.slot[n]--;
  }

  fp.seek(SlotIndex);
  fp.write(info.slot, 256);

  unsigned size = fp.size();
  fp.truncate(size - system.serialize_size);
  return true;
}

bool StateManager::load(const char *filename) {
  file fp;
  if(fp.open(filename, file::mode_read) == false) return false;
  unsigned filesize = fp.size();
  if(filesize < HeaderSize) return false;
  fp.seek(0);
  if(fp.readl(4) != 0x31415342) return false;
  if(fp.readl(4) != bsnesSaveStateVersion) return false;
  fp.read((uint8*)&info.slot[0], 256);
  fp.read((uint8*)&info.datetime[0], 256 * DateTimeSize);
  fp.read((uint8*)&info.description[0], 256 * DescriptionSize);
  info.slotcount = (filesize - HeaderSize) / system.serialize_size;
  return true;
}

bool StateManager::create(const char *filename) const {
  file fp;
  if(fp.open(filename, file::mode_write) == false) return false;
  fp.writel(0x31415342, 4);             //signature ('BSA1')
  fp.writel(bsnesSaveStateVersion, 4);  //version
  for(unsigned i = 0; i < 256 * SlotSize; i++) fp.write(SlotInvalid);  //slot index
  for(unsigned i = 0; i < 256 * DateTimeSize; i++) fp.write(0x20);     //date / time
  for(unsigned i = 0; i < 256 * DescriptionSize; i++) fp.write(0x00);  //description
  fp.close();
  return true;
}

uint8 StateManager::findslot(uint8 slot) const {
  if(slot == SlotInvalid) return SlotInvalid;
  for(unsigned n = 0; n < info.slotcount; n++) {
    if(info.slot[n] == slot) return n;
  }
  return SlotInvalid;
}
