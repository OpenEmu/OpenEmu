#ifdef SYSTEM_CPP

serializer System::serialize() {
  serializer s(serialize_size);

  unsigned signature = 0x31545342, version = bsnesSerializerVersion, crc32 = cartridge.crc32();
  char description[512];
  memset(&description, 0, sizeof description);

  s.integer(signature);
  s.integer(version);
  s.integer(crc32);
  s.array(description);

  serialize_all(s);
  return s;
}

bool System::unserialize(serializer &s) {
  unsigned signature, version, crc32;
  char description[512];

  s.integer(signature);
  s.integer(version);
  s.integer(crc32);
  s.array(description);

  if(signature != 0x31545342) return false;
  if(version != bsnesSerializerVersion) return false;
//if(crc32 != cartridge.crc32()) return false;
  scheduler.init();

  serialize_all(s);
  return true;
}

//========
//internal
//========

void System::serialize(serializer &s) {
  s.integer((unsigned&)region);
  s.integer((unsigned&)expansion);

  s.integer(scheduler.clock.cpu_freq);
  s.integer(scheduler.clock.smp_freq);

  s.integer(scheduler.clock.cpucop);
  s.integer(scheduler.clock.cpuppu);
  s.integer(scheduler.clock.cpusmp);
  s.integer(scheduler.clock.smpdsp);
}

void System::serialize_all(serializer &s) {
  bus.serialize(s);
  cartridge.serialize(s);
  system.serialize(s);
  cpu.serialize(s);
  smp.serialize(s);
  ppu.serialize(s);
  dsp.serialize(s);

  if(cartridge.mode() == Cartridge::ModeSuperGameBoy) supergameboy.serialize(s);

  if(cartridge.has_superfx()) superfx.serialize(s);
  if(cartridge.has_sa1())     sa1.serialize(s);
  if(cartridge.has_srtc())    srtc.serialize(s);
  if(cartridge.has_sdd1())    sdd1.serialize(s);
  if(cartridge.has_spc7110()) spc7110.serialize(s);
  if(cartridge.has_cx4())     cx4.serialize(s);
  if(cartridge.has_dsp1())    dsp1.serialize(s);
  if(cartridge.has_dsp2())    dsp2.serialize(s);
  if(cartridge.has_obc1())    obc1.serialize(s);
  if(cartridge.has_st010())   st010.serialize(s);
  if(cartridge.has_21fx())    s21fx.serialize(s);
}

//called once upon cartridge load event: perform dry-run state save.
//determines exactly how many bytes are needed to save state for this cartridge,
//as amount varies per game (eg different RAM sizes, special chips, etc.)
void System::serialize_init() {
  serializer s;

  unsigned signature = 0, version = 0, crc32 = 0;
  char description[512];

  s.integer(signature);
  s.integer(version);
  s.integer(crc32);
  s.array(description);

  serialize_all(s);
  serialize_size = s.size();
}

#endif
