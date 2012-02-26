#ifdef CARTRIDGE_CPP

#define READER(f, o) function<uint8 (unsigned)>(f, o)
#define WRITER(f, o) function<void (unsigned, uint8)>(f, o)

void Cartridge::parse_markup(const char *markup) {
  mapping.reset();
  information.nss.setting.reset();

  XML::Document document(markup);
  XML::Node &cartridge = document["cartridge"];
  region.i = cartridge["region"].data != "PAL" ? Region::NTSC : Region::PAL;

  parse_markup_rom(cartridge["rom"]);
  parse_markup_ram(cartridge["ram"]);
  parse_markup_nss(cartridge["nss"]);
  parse_markup_icd2(cartridge["icd2"]);
  parse_markup_sa1(cartridge["sa1"]);
  parse_markup_superfx(cartridge["superfx"]);
  parse_markup_necdsp(cartridge["necdsp"]);
  parse_markup_hitachidsp(cartridge["hitachidsp"]);
  parse_markup_bsx(cartridge["bsx"]);
  parse_markup_sufamiturbo(cartridge["sufamiturbo"]);
  parse_markup_srtc(cartridge["srtc"]);
  parse_markup_sdd1(cartridge["sdd1"]);
  parse_markup_spc7110(cartridge["spc7110"]);
  parse_markup_obc1(cartridge["obc1"]);
  parse_markup_setarisc(cartridge["setarisc"]);
  parse_markup_msu1(cartridge["msu1"]);
  parse_markup_link(cartridge["link"]);
}

//

unsigned Cartridge::parse_markup_integer(string &data) {
  if(strbegin(data, "0x")) return hex(data);
  return decimal(data);
}

void Cartridge::parse_markup_map(Mapping &m, XML::Node &map) {
  m.offset = parse_markup_integer(map["offset"].data);
  m.size = parse_markup_integer(map["size"].data);

  string data = map["mode"].data;
  if(data == "direct") m.mode.i = Bus::MapMode::Direct;
  if(data == "linear") m.mode.i = Bus::MapMode::Linear;
  if(data == "shadow") m.mode.i = Bus::MapMode::Shadow;

  lstring part;
  part.split(":", map["address"].data);
  if(part.size() != 2) return;

  lstring subpart;
  subpart.split("-", part[0]);
  if(subpart.size() == 1) {
    m.banklo = hex(subpart[0]);
    m.bankhi = m.banklo;
  } else if(subpart.size() == 2) {
    m.banklo = hex(subpart[0]);
    m.bankhi = hex(subpart[1]);
  }

  subpart.split("-", part[1]);
  if(subpart.size() == 1) {
    m.addrlo = hex(subpart[0]);
    m.addrhi = m.addrlo;
  } else if(subpart.size() == 2) {
    m.addrlo = hex(subpart[0]);
    m.addrhi = hex(subpart[1]);
  }
}

//

void Cartridge::parse_markup_rom(XML::Node &root) {
  if(root.exists() == false) return;
  foreach(node, root) {
    if(node.name != "map") continue;
    Mapping m(rom);
    parse_markup_map(m, node);
    if(m.size == 0) m.size = rom.size();
    mapping.append(m);
  }
}

void Cartridge::parse_markup_ram(XML::Node &root) {
  if(root.exists() == false) return;
  ram_size = parse_markup_integer(root["size"].data);
  foreach(node, root) {
    Mapping m(ram);
    parse_markup_map(m, node);
    if(m.size == 0) m.size = ram_size;
    mapping.append(m);
  }
}

void Cartridge::parse_markup_nss(XML::Node &root) {
  if(root.exists() == false) return;
  has_nss_dip = true;
  foreach(node, root) {
    if(node.name != "setting") continue;
    unsigned number = information.nss.setting.size();
    if(number >= 16) break;  //more than 16 DIP switches is not physically possible

    information.nss.option[number].reset();
    information.nss.setting[number] = node["name"].data;
    foreach(leaf, node) {
      if(leaf.name != "option") continue;
      string name = leaf["name"].data;
      unsigned value = parse_markup_integer(leaf["value"].data);
      information.nss.option[number].append(string( hex<4>(value), ":", name ));
    }
  }
}

void Cartridge::parse_markup_icd2(XML::Node &root) {
  if(root.exists() == false) return;
  if(mode.i != Mode::SuperGameBoy) return;

  icd2.revision = max(1, parse_markup_integer(root["revision"].data));

  foreach(node, root) {
    if(node.name != "map") continue;
    Mapping m(READER( &ICD2::read, &icd2 ), WRITER( &ICD2::write, &icd2 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_superfx(XML::Node &root) {
  if(root.exists() == false) return;
  has_superfx = true;

  foreach(node, root) {
    if(node.name == "rom") {
      foreach(leaf, node) {
        if(leaf.name != "map") continue;
        Mapping m(superfx.rom);
        parse_markup_map(m, leaf);
        mapping.append(m);
      }
    }
    if(node.name == "ram") {
      foreach(leaf, node) {
        if(leaf.name == "size") {
          ram_size = parse_markup_integer(leaf.data);
          continue;
        }
        if(leaf.name != "map") continue;
        Mapping m(superfx.ram);
        parse_markup_map(m, leaf);
        if(m.size == 0) m.size = ram_size;
        mapping.append(m);
      }
    }
    if(node.name == "mmio") {
      foreach(leaf, node) {
        if(leaf.name != "map") continue;
        Mapping m(READER( &SuperFX::mmio_read, &superfx ), WRITER( &SuperFX::mmio_write, &superfx ));
        parse_markup_map(m, leaf);
        mapping.append(m);
      }
    }
  }
}

void Cartridge::parse_markup_sa1(XML::Node &root) {
  if(root.exists() == false) return;
  has_sa1 = true;

  XML::Node &mcurom = root["mcu"]["rom"];
  XML::Node &mcuram = root["mcu"]["ram"];
  XML::Node &iram = root["iram"];
  XML::Node &bwram = root["bwram"];
  XML::Node &mmio = root["mmio"];

  foreach(node, mcurom) {
    if(node.name != "map") continue;
    Mapping m(READER( &SA1::mmc_read, &sa1 ), WRITER( &SA1::mmc_write, &sa1 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }

  foreach(node, mcuram) {
    if(node.name != "map") continue;
    Mapping m(READER( &SA1::mmc_cpu_read, &sa1 ), WRITER( &SA1::mmc_cpu_write, &sa1 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }

  foreach(node, iram) {
    if(node.name != "map") continue;
    Mapping m(sa1.cpuiram);
    parse_markup_map(m, node);
    if(m.size == 0) m.size = 2048;
    mapping.append(m);
  }

  ram_size = parse_markup_integer(bwram["size"].data);
  foreach(node, bwram) {
    if(node.name != "map") continue;
    Mapping m(sa1.cpubwram);
    parse_markup_map(m, node);
    if(m.size == 0) m.size = ram_size;
    mapping.append(m);
  }

  foreach(node, mmio) {
    if(node.name != "map") continue;
    Mapping m(READER( &SA1::mmio_read, &sa1 ), WRITER( &SA1::mmio_write, &sa1 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_necdsp(XML::Node &root) {
  if(root.exists() == false) return;
  has_necdsp = true;

  for(unsigned n = 0; n < 16384; n++) necdsp.programROM[n] = 0x000000;
  for(unsigned n = 0; n <  2048; n++) necdsp.dataROM[n] = 0x0000;

  necdsp.frequency = parse_markup_integer(root["frequency"].data);
  if(necdsp.frequency == 0) necdsp.frequency = 8000000;
  necdsp.revision.i
  = root["model"].data == "uPD7725"  ? NECDSP::Revision::uPD7725
  : root["model"].data == "uPD96050" ? NECDSP::Revision::uPD96050
  : NECDSP::Revision::uPD7725;
  string firmware = root["firmware"].data;
  string sha256 = root["sha256"].data;

  string path( dir(interface->path(Slot::Base, ".dsp")), firmware );
  unsigned promsize = (necdsp.revision.i == NECDSP::Revision::uPD7725 ? 2048 : 16384);
  unsigned dromsize = (necdsp.revision.i == NECDSP::Revision::uPD7725 ? 1024 :  2048);
  unsigned filesize = promsize * 3 + dromsize * 2;

  file fp;
  if(fp.open(path, file::mode_read) == false) {
    interface->message(string( "Warning: NEC DSP firmware ", firmware, " is missing." ));
  } else if(fp.size() != filesize) {
    interface->message(string( "Warning: NEC DSP firmware ", firmware, " is of the wrong file size." ));
    fp.close();
  } else {
    for(unsigned n = 0; n < promsize; n++) necdsp.programROM[n] = fp.readm(3);
    for(unsigned n = 0; n < dromsize; n++) necdsp.dataROM[n] = fp.readm(2);

    if(sha256 != "") {
      //XML file specified SHA256 sum for program. Verify file matches the hash.
      fp.seek(0);
      linear_vector<uint8_t> data;
      data.reserve(filesize);
      fp.read(&data[0], filesize);

      if(sha256 != nall::sha256(&data[0], filesize)) {
        interface->message(string( "Warning: NEC DSP firmware ", firmware, " SHA256 sum is incorrect." ));
      }
    }

    fp.close();
  }

  foreach(node, root) {
    if(node.name == "dr") {
      foreach(leaf, node) {
        Mapping m(READER( &NECDSP::dr_read, &necdsp ), WRITER( &NECDSP::dr_write, &necdsp ));
        parse_markup_map(m, leaf);
        mapping.append(m);
      }
    }
    if(node.name == "sr") {
      foreach(leaf, node) {
        Mapping m(READER( &NECDSP::sr_read, &necdsp ), WRITER( &NECDSP::sr_write, &necdsp ));
        parse_markup_map(m, leaf);
        mapping.append(m);
      }
    }
    if(node.name == "dp") {
      foreach(leaf, node) {
        Mapping m(READER( &NECDSP::dp_read, &necdsp ), WRITER( &NECDSP::dp_write, &necdsp ));
        parse_markup_map(m, leaf);
        mapping.append(m);
      }
    }
  }
}

void Cartridge::parse_markup_hitachidsp(XML::Node &root) {
  if(root.exists() == false) return;
  has_hitachidsp = true;

  for(unsigned n = 0; n < 1024; n++) hitachidsp.dataROM[n] = 0x000000;

  hitachidsp.frequency = parse_markup_integer(root["frequency"].data);
  if(hitachidsp.frequency == 0) hitachidsp.frequency = 20000000;
  string firmware = root["firmware"].data;
  string sha256 = root["sha256"].data;

  string path( dir(interface->path(Slot::Base, ".dsp")), firmware );
  file fp;
  if(fp.open(path, file::mode_read) == false) {
    interface->message(string( "Warning: Hitachi DSP firmware ", firmware, " is missing." ));
  } else if(fp.size() != 1024 * 3) {
    interface->message(string( "Warning: Hitachi DSP firmware ", firmware, " is of the wrong file size." ));
    fp.close();
  } else {
    for(unsigned n = 0; n < 1024; n++) hitachidsp.dataROM[n] = fp.readl(3);

    if(sha256 != "") {
      //XML file specified SHA256 sum for program. Verify file matches the hash.
      fp.seek(0);
      uint8 data[3072];
      fp.read(data, 3072);

      if(sha256 != nall::sha256(data, 3072)) {
        interface->message(string( "Warning: Hitachi DSP firmware ", firmware, " SHA256 sum is incorrect." ));
      }
    }

    fp.close();
  }

  foreach(node, root) {
    if(node.name == "rom") {
      foreach(leaf, node) {
        if(leaf.name != "map") continue;
        Mapping m(READER( &HitachiDSP::rom_read, &hitachidsp ), WRITER( &HitachiDSP::rom_write, &hitachidsp ));
        parse_markup_map(m, leaf);
        mapping.append(m);
      }
    }
    if(node.name == "mmio") {
      foreach(leaf, node) {
        Mapping m(READER( &HitachiDSP::dsp_read, &hitachidsp ), WRITER( &HitachiDSP::dsp_write, &hitachidsp ));
        parse_markup_map(m, leaf);
        mapping.append(m);
      }
    }
  }
}

void Cartridge::parse_markup_bsx(XML::Node &root) {
  if(root.exists() == false) return;
  if(mode.i != Mode::BsxSlotted && mode.i != Mode::Bsx) return;

  foreach(node, root["slot"]) {
    if(node.name != "map") continue;
    Mapping m(bsxflash.memory);
    parse_markup_map(m, node);
    mapping.append(m);
  }

  foreach(node, root["mmio"]) {
    if(node.name != "map") continue;
    Mapping m(READER( &BSXCartridge::mmio_read, &bsxcartridge ), WRITER( &BSXCartridge::mmio_write, &bsxcartridge ));
    parse_markup_map(m, node);
    mapping.append(m);
  }

  foreach(node, root["mcu"]) {
    if(node.name != "map") continue;
    Mapping m(READER( &BSXCartridge::mcu_read, &bsxcartridge ), WRITER( &BSXCartridge::mcu_write, &bsxcartridge ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_sufamiturbo(XML::Node &root) {
  if(root.exists() == false) return;
  if(mode.i != Mode::SufamiTurbo) return;

  foreach(slot, root) {
    if(slot.name != "slot") continue;
    bool slotid = slot["id"].data == "A" ? 0 : slot["id"].data == "B" ? 1 : 0;
    foreach(node, slot) {
      if(node.name == "rom") {
        foreach(leaf, node) {
          if(leaf.name != "map") continue;
          Memory &memory = slotid == 0 ? sufamiturbo.slotA.rom : sufamiturbo.slotB.rom;
          Mapping m(memory);
          parse_markup_map(m, leaf);
          if(m.size == 0) m.size = memory.size();
          if(m.size) mapping.append(m);
        }
      }
      if(node.name == "ram") {
        unsigned ram_size = parse_markup_integer(node["size"].data);
        foreach(leaf, node) {
          if(leaf.name != "map") continue;
          Memory &memory = slotid == 0 ? sufamiturbo.slotA.ram : sufamiturbo.slotB.ram;
          Mapping m(memory);
          parse_markup_map(m, leaf);
          if(m.size == 0) m.size = ram_size;
          if(m.size) mapping.append(m);
        }
      }
    }
  }
}

void Cartridge::parse_markup_srtc(XML::Node &root) {
  if(root.exists() == false) return;
  has_srtc = true;

  foreach(node, root) {
    if(node.name != "map") continue;
    Mapping m(READER( &SRTC::read, &srtc ), WRITER( &SRTC::write, &srtc ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_sdd1(XML::Node &root) {
  if(root.exists() == false) return;
  has_sdd1 = true;

  foreach(node, root["mmio"]) {
    if(node.name != "map") continue;
    Mapping m(READER( &SDD1::mmio_read, &sdd1 ), WRITER( &SDD1::mmio_write, &sdd1 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }

  foreach(node, root["mcu"]) {
    if(node.name != "map") continue;
    Mapping m(READER( &SDD1::mcu_read, &sdd1 ), WRITER( &SDD1::mcu_write, &sdd1 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_spc7110(XML::Node &root) {
  if(root.exists() == false) return;
  has_spc7110 = true;

  XML::Node &ram = root["ram"];
  XML::Node &mmio = root["mmio"];
  XML::Node &mcu = root["mcu"];
  XML::Node &dcu = root["dcu"];
  XML::Node &rtc = root["rtc"];

  ram_size = parse_markup_integer(ram["size"].data);
  foreach(node, ram) {
    if(node.name != "map") continue;
    Mapping m(READER( &SPC7110::ram_read, &spc7110 ), WRITER( &SPC7110::ram_write, &spc7110 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }

  foreach(node, mmio) {
    if(node.name != "map") continue;
    Mapping m(READER( &SPC7110::mmio_read, &spc7110 ), WRITER( &SPC7110::mmio_write, &spc7110 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }

  spc7110.data_rom_offset = parse_markup_integer(mcu["offset"].data);
  if(spc7110.data_rom_offset == 0) spc7110.data_rom_offset = 0x100000;
  foreach(node, mcu) {
    if(node.name != "map") continue;
    Mapping m(READER( &SPC7110::mcu_read, &spc7110 ), WRITER( &SPC7110::mcu_write, &spc7110 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }

  foreach(node, dcu) {
    if(node.name != "map") continue;
    Mapping m(READER( &SPC7110::dcu_read, &spc7110 ), WRITER( &SPC7110::dcu_write, &spc7110 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }

  foreach(node, rtc) {
    if(node.name != "map") continue;
    Mapping m(READER( &SPC7110::mmio_read, &spc7110 ), WRITER( &SPC7110::mmio_write, &spc7110 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_obc1(XML::Node &root) {
  if(root.exists() == false) return;
  has_obc1 = true;

  foreach(node, root) {
    if(node.name != "map") continue;
    Mapping m(READER( &OBC1::read, &obc1 ), WRITER( &OBC1::write, &obc1 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_setarisc(XML::Node &root) {
  if(root.exists() == false) return;
  has_st0018 = true;

  foreach(node, root) {
    if(node.name != "map") continue;
    Mapping m(READER( &ST0018::mmio_read, &st0018 ), WRITER( &ST0018::mmio_write, &st0018 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_msu1(XML::Node &root) {
  if(root.exists() == false) {
    has_msu1 = file::exists(interface->path(Cartridge::Slot::Base, ".msu"));
    if(has_msu1) {
      Mapping m(READER( &MSU1::mmio_read, &msu1 ), WRITER( &MSU1::mmio_write, &msu1 ));
      m.banklo = 0x00, m.bankhi = 0x3f, m.addrlo = 0x2000, m.addrhi = 0x2007;
      mapping.append(m);
      m.banklo = 0x80, m.bankhi = 0xbf, m.addrlo = 0x2000, m.addrhi = 0x2007;
      mapping.append(m);
    }
    return;
  }

  has_msu1 = true;

  foreach(node, root) {
    if(node.name != "map") continue;
    Mapping m(READER( &MSU1::mmio_read, &msu1 ), WRITER( &MSU1::mmio_write, &msu1 ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

void Cartridge::parse_markup_link(XML::Node &root) {
  if(root.exists() == false) return;
  has_link = true;

  link.frequency = max(1, parse_markup_integer(root["frequency"].data));
  link.program = root["program"].data;

  foreach(node, root) {
    if(node.name != "map") continue;
    Mapping m(READER( &Link::read, &link ), WRITER( &Link::write, &link ));
    parse_markup_map(m, node);
    mapping.append(m);
  }
}

Cartridge::Mapping::Mapping() {
  mode.i = Bus::MapMode::Direct;
  banklo = bankhi = addrlo = addrhi = offset = size = 0;
}

Cartridge::Mapping::Mapping(Memory &memory) {
  read = READER( &Memory::read, &memory );
  write = WRITER( &Memory::write, &memory );
  mode.i = Bus::MapMode::Direct;
  banklo = bankhi = addrlo = addrhi = offset = size = 0;
}

Cartridge::Mapping::Mapping(const function<uint8 (unsigned)> &read_, const function<void (unsigned, uint8)> &write_) {
  read = read_;
  write = write_;
  mode.i = Bus::MapMode::Direct;
  banklo = bankhi = addrlo = addrhi = offset = size = 0;
}

#endif
