bool Cheat::active() const { return cheat_enabled; }
bool Cheat::exists(unsigned addr) const { return bitmask[addr >> 3] & 1 << (addr & 7); }
