unsigned Cheat::count() const { return code.size(); }
bool Cheat::active() const { return cheat_enabled; }
bool Cheat::exists(unsigned addr) const { return mask[addr >> 3] & 1 << (addr & 7); }
