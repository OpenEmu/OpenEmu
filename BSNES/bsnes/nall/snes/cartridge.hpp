#ifndef NALL_SNES_CARTRIDGE_HPP
#define NALL_SNES_CARTRIDGE_HPP

namespace nall {

struct SuperFamicomCartridge {
  string markup;
  inline SuperFamicomCartridge(const uint8_t *data, unsigned size);

//private:
  inline void read_header(const uint8_t *data, unsigned size);
  inline unsigned find_header(const uint8_t *data, unsigned size);
  inline unsigned score_header(const uint8_t *data, unsigned size, unsigned addr);
  inline unsigned gameboy_ram_size(const uint8_t *data, unsigned size);
  inline bool gameboy_has_rtc(const uint8_t *data, unsigned size);

  enum HeaderField {
    CartName    = 0x00,
    Mapper      = 0x15,
    RomType     = 0x16,
    RomSize     = 0x17,
    RamSize     = 0x18,
    CartRegion  = 0x19,
    Company     = 0x1a,
    Version     = 0x1b,
    Complement  = 0x1c,  //inverse checksum
    Checksum    = 0x1e,
    ResetVector = 0x3c,
  };

  enum Mode {
    ModeNormal,
    ModeBsxSlotted,
    ModeBsx,
    ModeSufamiTurbo,
    ModeSuperGameBoy,
  };

  enum Type {
    TypeNormal,
    TypeBsxSlotted,
    TypeBsxBios,
    TypeBsx,
    TypeSufamiTurboBios,
    TypeSufamiTurbo,
    TypeSuperGameBoy1Bios,
    TypeSuperGameBoy2Bios,
    TypeGameBoy,
    TypeUnknown,
  };

  enum Region {
    NTSC,
    PAL,
  };

  enum MemoryMapper {
    LoROM,
    HiROM,
    ExLoROM,
    ExHiROM,
    SuperFXROM,
    SA1ROM,
    SPC7110ROM,
    BSCLoROM,
    BSCHiROM,
    BSXROM,
    STROM,
  };

  enum DSP1MemoryMapper {
    DSP1Unmapped,
    DSP1LoROM1MB,
    DSP1LoROM2MB,
    DSP1HiROM,
  };

  bool loaded;        //is a base cartridge inserted?
  unsigned crc32;     //crc32 of all cartridges (base+slot(s))
  unsigned rom_size;
  unsigned ram_size;

  Mode mode;
  Type type;
  Region region;
  MemoryMapper mapper;
  DSP1MemoryMapper dsp1_mapper;

  bool has_bsx_slot;
  bool has_superfx;
  bool has_sa1;
  bool has_srtc;
  bool has_sdd1;
  bool has_spc7110;
  bool has_spc7110rtc;
  bool has_cx4;
  bool has_dsp1;
  bool has_dsp2;
  bool has_dsp3;
  bool has_dsp4;
  bool has_obc1;
  bool has_st010;
  bool has_st011;
  bool has_st018;
};

SuperFamicomCartridge::SuperFamicomCartridge(const uint8_t *data, unsigned size) {
  read_header(data, size);

  string xml;
  markup = "<?xml version='1.0' encoding='UTF-8'?>\n";

  if(type == TypeBsx) {
    markup.append("<cartridge/>\n");
    return;
  }

  if(type == TypeSufamiTurbo) {
    markup.append("<cartridge/>\n");
    return;
  }

  if(type == TypeGameBoy) {
    markup.append("<cartridge rtc='", gameboy_has_rtc(data, size), "'\n");
    if(gameboy_ram_size(data, size) > 0) {
      markup.append("  <ram size='0x", hex(gameboy_ram_size(data, size)), "'>\n");
    }
    markup.append("</cartridge>\n");
    return;
  }

  const char *range = (rom_size > 0x200000) || (ram_size > 32 * 1024) ? "0000-7fff" : "0000-ffff";
  markup.append("<cartridge region='", region == NTSC ? "NTSC" : "PAL", "'>\n");

  if(type == TypeSuperGameBoy1Bios || type == TypeSuperGameBoy2Bios) markup.append(
    "  <rom>\n"
    "    <map mode='linear' address='00-7f:8000-ffff'/>\n"
    "    <map mode='linear' address='80-ff:8000-ffff'/>\n"
    "  </rom>\n"
    "  <icd2 revision='1'>\n"
    "    <map address='00-3f:6000-7fff'/>\n"
    "    <map address='80-bf:6000-7fff'/>\n"
    "  </icd2>\n"
  );

  else if(has_cx4) markup.append(
    "  <hitachidsp model='HG51B169' frequency='20000000' firmware='cx4.rom' sha256='ae8d4d1961b93421ff00b3caa1d0f0ce7783e749772a3369c36b3dbf0d37ef18'>\n"
    "    <rom>\n"
    "      <map mode='linear' address='00-7f:8000-ffff'/>\n"
    "      <map mode='linear' address='80-ff:8000-ffff'/>\n"
    "    </rom>\n"
    "    <mmio>\n"
    "      <map address='00-3f:6000-7fff'/>\n"
    "      <map address='80-bf:6000-7fff'/>\n"
    "    </mmio>\n"
    "  </hitachidsp>\n"
  );

  else if(has_spc7110) {
    markup.append(
      "  <rom>\n"
      "    <map mode='shadow' address='00-0f:8000-ffff'/>\n"
      "    <map mode='shadow' address='80-bf:8000-ffff'/>\n"
      "    <map mode='linear' address='c0-cf:0000-ffff'/>\n"
      "  </rom>\n"
      "  <spc7110>\n"
      "    <ram size='0x", hex(ram_size), "'>\n"
      "      <map mode='linear' address='00:6000-7fff'/>\n"
      "      <map mode='linear' address='30:6000-7fff'/>\n"
      "    </ram>\n"
      "    <mmio>\n"
      "      <map address='00-3f:4800-483f'/>\n"
      "      <map address='80-bf:4800-483f'/>\n"
      "    </mmio>\n"
      "    <mcu>\n"
      "      <map address='d0-ff:0000-ffff' offset='0x100000' size='0x", hex(size - 0x100000), "'/>\n"
      "    </mcu>\n"
      "    <dcu>\n"
      "      <map address='50:0000-ffff'/>\n"
      "    </dcu>\n"
    );
    if(has_spc7110rtc) markup.append(
      "    <rtc>\n"
      "      <map address='00-3f:4840-4842'/>\n"
      "      <map address='80-bf:4840-4842'/>\n"
      "    </rtc>\n"
    );
    markup.append(
      "  </spc7110>\n"
    );
  }

  else if(mapper == LoROM) {
    markup.append(
      "  <rom>\n"
      "    <map mode='linear' address='00-7f:8000-ffff'/>\n"
      "    <map mode='linear' address='80-ff:8000-ffff'/>\n"
      "  </rom>\n"
    );
    if(ram_size > 0) markup.append(
      "  <ram size='0x", hex(ram_size), "'>\n"
      "    <map mode='linear' address='20-3f:6000-7fff'/>\n"
      "    <map mode='linear' address='a0-bf:6000-7fff'/>\n"
      "    <map mode='linear' address='70-7f:", range, "'/>\n"
      "    <map mode='linear' address='f0-ff:", range, "'/>\n"
      "  </ram>\n"
    );
  }

  else if(mapper == HiROM) {
    markup.append(
      "  <rom>\n"
      "    <map mode='shadow' address='00-3f:8000-ffff'/>\n"
      "    <map mode='linear' address='40-7f:0000-ffff'/>\n"
      "    <map mode='shadow' address='80-bf:8000-ffff'/>\n"
      "    <map mode='linear' address='c0-ff:0000-ffff'/>\n"
      "  </rom>\n"
    );
    if(ram_size > 0) markup.append(
      "  <ram size='0x", hex(ram_size), "'>\n"
      "    <map mode='linear' address='20-3f:6000-7fff'/>\n"
      "    <map mode='linear' address='a0-bf:6000-7fff'/>\n"
      "    <map mode='linear' address='70-7f:", range, "'/>\n"
      "  </ram>\n"
    );
  }

  else if(mapper == ExLoROM) {
    markup.append(
      "  <rom>\n"
      "    <map mode='linear' address='00-3f:8000-ffff'/>\n"
      "    <map mode='linear' address='40-7f:0000-ffff'/>\n"
      "    <map mode='linear' address='80-bf:8000-ffff'/>\n"
      "  </rom>\n"
    );
    if(ram_size > 0) markup.append(
      "  <ram size='0x", hex(ram_size), "'>\n"
      "    <map mode='linear' address='20-3f:6000-7fff'/>\n"
      "    <map mode='linear' address='a0-bf:6000-7fff'/>\n"
      "    <map mode='linear' address='70-7f:0000-7fff'/>\n"
      "  </ram>\n"
    );
  }

  else if(mapper == ExHiROM) {
    markup.append(
      "  <rom>\n"
      "    <map mode='shadow' address='00-3f:8000-ffff' offset='0x400000'/>\n"
      "    <map mode='linear' address='40-7f:0000-ffff' offset='0x400000'/>\n"
      "    <map mode='shadow' address='80-bf:8000-ffff' offset='0x000000'/>\n"
      "    <map mode='linear' address='c0-ff:0000-ffff' offset='0x000000'/>\n"
      "  </rom>\n"
    );
    if(ram_size > 0) markup.append(
      "  <ram size='0x", hex(ram_size), "'>\n"
      "    <map mode='linear' address='20-3f:6000-7fff'/>\n"
      "    <map mode='linear' address='a0-bf:6000-7fff'/>\n"
      "    <map mode='linear' address='70-7f:", range, "'/>\n"
      "  </ram>\n"
    );
  }

  else if(mapper == SuperFXROM) markup.append(
    "  <superfx revision='2'>\n"
    "    <rom>\n"
    "      <map mode='linear' address='00-3f:8000-ffff'/>\n"
    "      <map mode='linear' address='40-5f:0000-ffff'/>\n"
    "      <map mode='linear' address='80-bf:8000-ffff'/>\n"
    "      <map mode='linear' address='c0-df:0000-ffff'/>\n"
    "    </rom>\n"
    "    <ram size='0x", hex(ram_size), "'>\n"
    "      <map mode='linear' address='00-3f:6000-7fff' size='0x2000'/>\n"
    "      <map mode='linear' address='60-7f:0000-ffff'/>\n"
    "      <map mode='linear' address='80-bf:6000-7fff' size='0x2000'/>\n"
    "      <map mode='linear' address='e0-ff:0000-ffff'/>\n"
    "    </ram>\n"
    "    <mmio>\n"
    "      <map address='00-3f:3000-32ff'/>\n"
    "      <map address='80-bf:3000-32ff'/>\n"
    "    </mmio>\n"
    "  </superfx>\n"
  );

  else if(mapper == SA1ROM) markup.append(
    "  <sa1>\n"
    "    <mcu>\n"
    "      <rom>\n"
    "        <map mode='direct' address='00-3f:8000-ffff'/>\n"
    "        <map mode='direct' address='80-bf:8000-ffff'/>\n"
    "        <map mode='direct' address='c0-ff:0000-ffff'/>\n"
    "      </rom>\n"
    "      <ram>\n"
    "        <map mode='direct' address='00-3f:6000-7fff'/>\n"
    "        <map mode='direct' address='80-bf:6000-7fff'/>\n"
    "      </ram>\n"
    "    </mcu>\n"
    "    <iram size='0x800'>\n"
    "      <map mode='linear' address='00-3f:3000-37ff'/>\n"
    "      <map mode='linear' address='80-bf:3000-37ff'/>\n"
    "    </iram>\n"
    "    <bwram size='0x", hex(ram_size), "'>\n"
    "      <map mode='linear' address='40-4f:0000-ffff'/>\n"
    "    </bwram>\n"
    "    <mmio>\n"
    "      <map address='00-3f:2200-23ff'/>\n"
    "      <map address='80-bf:2200-23ff'/>\n"
    "    </mmio>\n"
    "  </sa1>\n"
  );

  else if(mapper == BSCLoROM) markup.append(
    "  <rom>\n"
    "    <map mode='linear' address='00-1f:8000-ffff' offset='0x000000'/>\n"
    "    <map mode='linear' address='20-3f:8000-ffff' offset='0x100000'/>\n"
    "    <map mode='linear' address='80-9f:8000-ffff' offset='0x200000'/>\n"
    "    <map mode='linear' address='a0-bf:8000-ffff' offset='0x100000'/>\n"
    "  </rom>\n"
    "  <ram size='0x", hex(ram_size), "'>\n"
    "    <map mode='linear' address='70-7f:0000-7fff'/>\n"
    "    <map mode='linear' address='f0-ff:0000-7fff'/>\n"
    "  </ram>\n"
    "  <bsx>\n"
    "    <slot>\n"
    "      <map mode='linear' address='c0-ef:0000-ffff'/>\n"
    "    </slot>\n"
    "  </bsx>\n"
  );

  else if(mapper == BSCHiROM) markup.append(
    "  <rom>\n"
    "    <map mode='shadow' address='00-1f:8000-ffff'/>\n"
    "    <map mode='linear' address='40-5f:0000-ffff'/>\n"
    "    <map mode='shadow' address='80-9f:8000-ffff'/>\n"
    "    <map mode='linear' address='c0-df:0000-ffff'/>\n"
    "  </rom>\n"
    "  <ram size='0x", hex(ram_size), "'>\n"
    "    <map mode='linear' address='20-3f:6000-7fff'/>\n"
    "    <map mode='linear' address='a0-bf:6000-7fff'/>\n"
    "  </ram>\n"
    "  <bsx>\n"
    "    <slot>\n"
    "      <map mode='shadow' address='20-3f:8000-ffff'/>\n"
    "      <map mode='linear' address='60-7f:0000-ffff'/>\n"
    "      <map mode='shadow' address='a0-bf:8000-ffff'/>\n"
    "      <map mode='linear' address='e0-ff:0000-ffff'/>\n"
    "    </slot>\n"
    "  </bsx>\n"
  );

  else if(mapper == BSXROM) markup.append(
    "  <bsx>\n"
    "    <mcu>\n"
    "      <map address='00-3f:8000-ffff'/>\n"
    "      <map address='80-bf:8000-ffff'/>\n"
    "      <map address='40-7f:0000-ffff'/>\n"
    "      <map address='c0-ff:0000-ffff'/>\n"
    "      <map address='20-3f:6000-7fff'/>\n"
    "    </mcu>\n"
    "    <mmio>\n"
    "      <map address='00-3f:5000-5fff'/>\n"
    "      <map address='80-bf:5000-5fff'/>\n"
    "    </mmio>\n"
    "  </bsx>\n"
  );

  else if(mapper == STROM) markup.append(
    "  <rom>\n"
    "    <map mode='linear' address='00-1f:8000-ffff'/>\n"
    "    <map mode='linear' address='80-9f:8000-ffff'/>\n"
    "  </rom>\n"
    "  <sufamiturbo>\n"
    "    <slot id='A'>\n"
    "      <rom>\n"
    "        <map mode='linear' address='20-3f:8000-ffff'/>\n"
    "        <map mode='linear' address='a0-bf:8000-ffff'/>\n"
    "      </rom>\n"
    "      <ram size='0x20000'>\n"
    "        <map mode='linear' address='60-63:8000-ffff'/>\n"
    "        <map mode='linear' address='e0-e3:8000-ffff'/>\n"
    "      </ram>\n"
    "    </slot>\n"
    "    <slot id='B'>\n"
    "      <rom>\n"
    "        <map mode='linear' address='40-5f:8000-ffff'/>\n"
    "        <map mode='linear' address='c0-df:8000-ffff'/>\n"
    "      </rom>\n"
    "      <ram size='0x20000'>\n"
    "        <map mode='linear' address='70-73:8000-ffff'/>\n"
    "        <map mode='linear' address='f0-f3:8000-ffff'/>\n"
    "      </ram>\n"
    "    </slot>\n"
    "  </sufamiturbo>\n"
  );

  if(has_srtc) markup.append(
    "  <srtc>\n"
    "    <map address='00-3f:2800-2801'/>\n"
    "    <map address='80-bf:2800-2801'/>\n"
    "  </srtc>\n"
  );

  if(has_sdd1) markup.append(
    "  <sdd1>\n"
    "    <mcu>\n"
    "      <map address='c0-ff:0000-ffff'/>\n"
    "    </mcu>\n"
    "    <mmio>\n"
    "      <map address='00-3f:4800-4807'/>\n"
    "      <map address='80-bf:4800-4807'/>\n"
    "    </mmio>\n"
    "  </sdd1>\n"
  );

  if(has_obc1) markup.append(
    "  <obc1>\n"
    "    <map address='00-3f:6000-7fff'/>\n"
    "    <map address='80-bf:6000-7fff'/>\n"
    "  </obc1>\n"
  );

  if(has_dsp1) {
    //91e87d11e1c30d172556bed2211cce2efa94ba595f58c5d264809ef4d363a97b  dsp1.rom
    markup.append("  <necdsp model='uPD7725' frequency='8000000' firmware='dsp1b.rom' sha256='d789cb3c36b05c0b23b6c6f23be7aa37c6e78b6ee9ceac8d2d2aa9d8c4d35fa9'>\n");
    if(dsp1_mapper == DSP1LoROM1MB) markup.append(
      "    <dr>\n"
      "      <map address='20-3f:8000-bfff'/>\n"
      "      <map address='a0-bf:8000-bfff'/>\n"
      "    </dr>\n"
      "    <sr>\n"
      "      <map address='20-3f:c000-ffff'/>\n"
      "      <map address='a0-bf:c000-ffff'/>\n"
      "    </sr>\n"
    );
    if(dsp1_mapper == DSP1LoROM2MB) markup.append(
      "    <dr>\n"
      "      <map address='60-6f:0000-3fff'/>\n"
      "      <map address='e0-ef:0000-3fff'/>\n"
      "    </dr>\n"
      "    <sr>\n"
      "      <map address='60-6f:4000-7fff'/>\n"
      "      <map address='e0-ef:4000-7fff'/>\n"
      "    </sr>\n"
    );
    if(dsp1_mapper == DSP1HiROM) markup.append(
      "    <dr>\n"
      "      <map address='00-1f:6000-6fff'/>\n"
      "      <map address='80-9f:6000-6fff'/>\n"
      "    </dr>\n"
      "    <sr>\n"
      "      <map address='00-1f:7000-7fff'/>\n"
      "      <map address='80-9f:7000-7fff'/>\n"
      "    </sr>\n"
    );
    markup.append("  </necdsp>\n");
  }

  if(has_dsp2) markup.append(
    "  <necdsp model='uPD7725' frequency='8000000' firmware='dsp2.rom' sha256='03ef4ef26c9f701346708cb5d07847b5203cf1b0818bf2930acd34510ffdd717'>\n"
    "    <dr>\n"
    "      <map address='20-3f:8000-bfff'/>\n"
    "      <map address='a0-bf:8000-bfff'/>\n"
    "    </dr>\n"
    "    <sr>\n"
    "      <map address='20-3f:c000-ffff'/>\n"
    "      <map address='a0-bf:c000-ffff'/>\n"
    "    </sr>\n"
    "  </necdsp>\n"
  );

  if(has_dsp3) markup.append(
    "  <necdsp model='uPD7725' frequency='8000000' firmware='dsp3.rom' sha256='0971b08f396c32e61989d1067dddf8e4b14649d548b2188f7c541b03d7c69e4e'>\n"
    "    <dr>\n"
    "      <map address='20-3f:8000-bfff'/>\n"
    "      <map address='a0-bf:8000-bfff'/>\n"
    "    </dr>\n"
    "    <sr>\n"
    "      <map address='20-3f:c000-ffff'/>\n"
    "      <map address='a0-bf:c000-ffff'/>\n"
    "    </sr>\n"
    "  </necdsp>\n"
  );

  if(has_dsp4) markup.append(
    "  <necdsp model='uPD7725' frequency='8000000' firmware='dsp4.rom' sha256='752d03b2d74441e430b7f713001fa241f8bbcfc1a0d890ed4143f174dbe031da'>\n"
    "    <dr>\n"
    "      <map address='30-3f:8000-bfff'/>\n"
    "      <map address='b0-bf:8000-bfff'/>\n"
    "    </dr>\n"
    "    <sr>\n"
    "      <map address='30-3f:c000-ffff'/>\n"
    "      <map address='b0-bf:c000-ffff'/>\n"
    "    </sr>\n"
    "  </necdsp>\n"
  );

  if(has_st010) markup.append(
    "  <necdsp model='uPD96050' frequency='10000000' firmware='st010.rom' sha256='fa9bced838fedea11c6f6ace33d1878024bdd0d02cc9485899d0bdd4015ec24c'>\n"
    "    <dr>\n"
    "      <map address='60:0000'/>\n"
    "      <map address='e0:0000'/>\n"
    "    </dr>\n"
    "    <sr>\n"
    "      <map address='60:0001'/>\n"
    "      <map address='e0:0001'/>\n"
    "    </sr>\n"
    "    <dp>\n"
    "      <map address='68-6f:0000-0fff'/>\n"
    "      <map address='e8-ef:0000-0fff'/>\n"
    "    </dp>\n"
    "  </necdsp>\n"
  );

  if(has_st011) markup.append(
    "  <necdsp model='uPD96050' frequency='15000000' firmware='st011.rom' sha256='8b2b3f3f3e6e29f4d21d8bc736b400bc988b7d2214ebee15643f01c1fee2f364'>\n"
    "    <dr>\n"
    "      <map address='60:0000'/>\n"
    "      <map address='e0:0000'/>\n"
    "    </dr>\n"
    "    <sr>\n"
    "      <map address='60:0001'/>\n"
    "      <map address='e0:0001'/>\n"
    "    </sr>\n"
    "    <dp>\n"
    "      <map address='68-6f:0000-0fff'/>\n"
    "      <map address='e8-ef:0000-0fff'/>\n"
    "    </dp>\n"
    "  </necdsp>\n"
  );

  if(has_st018) markup.append(
    "  <armdsp firmware='st018.rom' frequency='21477272' sha256='6df209ab5d2524d1839c038be400ae5eb20dafc14a3771a3239cd9e8acd53806'>\n"
    "    <map address='00-3f:3800-38ff'/>\n"
    "    <map address='80-bf:3800-38ff'/>\n"
    "  </armdsp>\n"
  );

  markup.append("</cartridge>\n");
}

void SuperFamicomCartridge::read_header(const uint8_t *data, unsigned size) {
  type        = TypeUnknown;
  mapper      = LoROM;
  dsp1_mapper = DSP1Unmapped;
  region      = NTSC;
  rom_size    = size;
  ram_size    = 0;

  has_bsx_slot   = false;
  has_superfx    = false;
  has_sa1        = false;
  has_srtc       = false;
  has_sdd1       = false;
  has_spc7110    = false;
  has_spc7110rtc = false;
  has_cx4        = false;
  has_dsp1       = false;
  has_dsp2       = false;
  has_dsp3       = false;
  has_dsp4       = false;
  has_obc1       = false;
  has_st010      = false;
  has_st011      = false;
  has_st018      = false;

  //=====================
  //detect Game Boy carts
  //=====================

  if(size >= 0x0140) {
    if(data[0x0104] == 0xce && data[0x0105] == 0xed && data[0x0106] == 0x66 && data[0x0107] == 0x66
    && data[0x0108] == 0xcc && data[0x0109] == 0x0d && data[0x010a] == 0x00 && data[0x010b] == 0x0b) {
      type = TypeGameBoy;
      return;
    }
  }

  if(size < 32768) {
    type = TypeUnknown;
    return;
  }

  const unsigned index = find_header(data, size);
  const uint8_t mapperid = data[index + Mapper];
  const uint8_t rom_type = data[index + RomType];
  const uint8_t rom_size = data[index + RomSize];
  const uint8_t company  = data[index + Company];
  const uint8_t regionid = data[index + CartRegion] & 0x7f;

  ram_size = 1024 << (data[index + RamSize] & 7);
  if(ram_size == 1024) ram_size = 0;  //no RAM present

  //0, 1, 13 = NTSC; 2 - 12 = PAL
  region = (regionid <= 1 || regionid >= 13) ? NTSC : PAL;

  //=======================
  //detect BS-X flash carts
  //=======================

  if(data[index + 0x13] == 0x00 || data[index + 0x13] == 0xff) {
    if(data[index + 0x14] == 0x00) {
      const uint8_t n15 = data[index + 0x15];
      if(n15 == 0x00 || n15 == 0x80 || n15 == 0x84 || n15 == 0x9c || n15 == 0xbc || n15 == 0xfc) {
        if(data[index + 0x1a] == 0x33 || data[index + 0x1a] == 0xff) {
          type = TypeBsx;
          mapper = BSXROM;
          region = NTSC;  //BS-X only released in Japan
          return;
        }
      }
    }
  }

  //=========================
  //detect Sufami Turbo carts
  //=========================

  if(!memcmp(data, "BANDAI SFC-ADX", 14)) {
    if(!memcmp(data + 16, "SFC-ADX BACKUP", 14)) {
      type = TypeSufamiTurboBios;
    } else {
      type = TypeSufamiTurbo;
    }
    mapper = STROM;
    region = NTSC;  //Sufami Turbo only released in Japan
    return;         //RAM size handled outside this routine
  }

  //==========================
  //detect Super Game Boy BIOS
  //==========================

  if(!memcmp(data + index, "Super GAMEBOY2", 14)) {
    type = TypeSuperGameBoy2Bios;
    return;
  }

  if(!memcmp(data + index, "Super GAMEBOY", 13)) {
    type = TypeSuperGameBoy1Bios;
    return;
  }

  //=====================
  //detect standard carts
  //=====================

  //detect presence of BS-X flash cartridge connector (reads extended header information)
  if(data[index - 14] == 'Z') {
    if(data[index - 11] == 'J') {
      uint8_t n13 = data[index - 13];
      if((n13 >= 'A' && n13 <= 'Z') || (n13 >= '0' && n13 <= '9')) {
        if(company == 0x33 || (data[index - 10] == 0x00 && data[index - 4] == 0x00)) {
          has_bsx_slot = true;
        }
      }
    }
  }

  if(has_bsx_slot) {
    if(!memcmp(data + index, "Satellaview BS-X     ", 21)) {
      //BS-X base cart
      type = TypeBsxBios;
      mapper = BSXROM;
      region = NTSC;  //BS-X only released in Japan
      return;         //RAM size handled internally by load_cart_bsx() -> BSXCart class
    } else {
      type = TypeBsxSlotted;
      mapper = (index == 0x7fc0 ? BSCLoROM : BSCHiROM);
      region = NTSC;  //BS-X slotted cartridges only released in Japan
    }
  } else {
    //standard cart
    type = TypeNormal;

    if(index == 0x7fc0 && size >= 0x401000) {
      mapper = ExLoROM;
    } else if(index == 0x7fc0 && mapperid == 0x32) {
      mapper = ExLoROM;
    } else if(index == 0x7fc0) {
      mapper = LoROM;
    } else if(index == 0xffc0) {
      mapper = HiROM;
    } else {  //index == 0x40ffc0
      mapper = ExHiROM;
    }
  }

  if(mapperid == 0x20 && (rom_type == 0x13 || rom_type == 0x14 || rom_type == 0x15 || rom_type == 0x1a)) {
    has_superfx = true;
    mapper = SuperFXROM;
    ram_size = 1024 << (data[index - 3] & 7);
    if(ram_size == 1024) ram_size = 0;
  }

  if(mapperid == 0x23 && (rom_type == 0x32 || rom_type == 0x34 || rom_type == 0x35)) {
    has_sa1 = true;
    mapper = SA1ROM;
  }

  if(mapperid == 0x35 && rom_type == 0x55) {
    has_srtc = true;
  }

  if(mapperid == 0x32 && (rom_type == 0x43 || rom_type == 0x45)) {
    has_sdd1 = true;
  }

  if(mapperid == 0x3a && (rom_type == 0xf5 || rom_type == 0xf9)) {
    has_spc7110 = true;
    has_spc7110rtc = (rom_type == 0xf9);
    mapper = SPC7110ROM;
  }

  if(mapperid == 0x20 && rom_type == 0xf3) {
    has_cx4 = true;
  }

  if((mapperid == 0x20 || mapperid == 0x21) && rom_type == 0x03) {
    has_dsp1 = true;
  }

  if(mapperid == 0x30 && rom_type == 0x05 && company != 0xb2) {
    has_dsp1 = true;
  }

  if(mapperid == 0x31 && (rom_type == 0x03 || rom_type == 0x05)) {
    has_dsp1 = true;
  }

  if(has_dsp1 == true) {
    if((mapperid & 0x2f) == 0x20 && size <= 0x100000) {
      dsp1_mapper = DSP1LoROM1MB;
    } else if((mapperid & 0x2f) == 0x20) {
      dsp1_mapper = DSP1LoROM2MB;
    } else if((mapperid & 0x2f) == 0x21) {
      dsp1_mapper = DSP1HiROM;
    }
  }

  if(mapperid == 0x20 && rom_type == 0x05) {
    has_dsp2 = true;
  }

  if(mapperid == 0x30 && rom_type == 0x05 && company == 0xb2) {
    has_dsp3 = true;
  }

  if(mapperid == 0x30 && rom_type == 0x03) {
    has_dsp4 = true;
  }

  if(mapperid == 0x30 && rom_type == 0x25) {
    has_obc1 = true;
  }

  if(mapperid == 0x30 && rom_type == 0xf6 && rom_size >= 10) {
    has_st010 = true;
  }

  if(mapperid == 0x30 && rom_type == 0xf6 && rom_size < 10) {
    has_st011 = true;
  }

  if(mapperid == 0x30 && rom_type == 0xf5) {
    has_st018 = true;
  }
}

unsigned SuperFamicomCartridge::find_header(const uint8_t *data, unsigned size) {
  unsigned score_lo = score_header(data, size, 0x007fc0);
  unsigned score_hi = score_header(data, size, 0x00ffc0);
  unsigned score_ex = score_header(data, size, 0x40ffc0);
  if(score_ex) score_ex += 4;  //favor ExHiROM on images > 32mbits

  if(score_lo >= score_hi && score_lo >= score_ex) {
    return 0x007fc0;
  } else if(score_hi >= score_ex) {
    return 0x00ffc0;
  } else {
    return 0x40ffc0;
  }
}

unsigned SuperFamicomCartridge::score_header(const uint8_t *data, unsigned size, unsigned addr) {
  if(size < addr + 64) return 0;  //image too small to contain header at this location?
  int score = 0;

  uint16_t resetvector = data[addr + ResetVector] | (data[addr + ResetVector + 1] << 8);
  uint16_t checksum    = data[addr + Checksum   ] | (data[addr + Checksum    + 1] << 8);
  uint16_t complement  = data[addr + Complement ] | (data[addr + Complement  + 1] << 8);

  uint8_t resetop = data[(addr & ~0x7fff) | (resetvector & 0x7fff)];  //first opcode executed upon reset
  uint8_t mapper  = data[addr + Mapper] & ~0x10;                      //mask off irrelevent FastROM-capable bit

  //$00:[000-7fff] contains uninitialized RAM and MMIO.
  //reset vector must point to ROM at $00:[8000-ffff] to be considered valid.
  if(resetvector < 0x8000) return 0;

  //some images duplicate the header in multiple locations, and others have completely
  //invalid header information that cannot be relied upon.
  //below code will analyze the first opcode executed at the specified reset vector to
  //determine the probability that this is the correct header.

  //most likely opcodes
  if(resetop == 0x78  //sei
  || resetop == 0x18  //clc (clc; xce)
  || resetop == 0x38  //sec (sec; xce)
  || resetop == 0x9c  //stz $nnnn (stz $4200)
  || resetop == 0x4c  //jmp $nnnn
  || resetop == 0x5c  //jml $nnnnnn
  ) score += 8;

  //plausible opcodes
  if(resetop == 0xc2  //rep #$nn
  || resetop == 0xe2  //sep #$nn
  || resetop == 0xad  //lda $nnnn
  || resetop == 0xae  //ldx $nnnn
  || resetop == 0xac  //ldy $nnnn
  || resetop == 0xaf  //lda $nnnnnn
  || resetop == 0xa9  //lda #$nn
  || resetop == 0xa2  //ldx #$nn
  || resetop == 0xa0  //ldy #$nn
  || resetop == 0x20  //jsr $nnnn
  || resetop == 0x22  //jsl $nnnnnn
  ) score += 4;

  //implausible opcodes
  if(resetop == 0x40  //rti
  || resetop == 0x60  //rts
  || resetop == 0x6b  //rtl
  || resetop == 0xcd  //cmp $nnnn
  || resetop == 0xec  //cpx $nnnn
  || resetop == 0xcc  //cpy $nnnn
  ) score -= 4;

  //least likely opcodes
  if(resetop == 0x00  //brk #$nn
  || resetop == 0x02  //cop #$nn
  || resetop == 0xdb  //stp
  || resetop == 0x42  //wdm
  || resetop == 0xff  //sbc $nnnnnn,x
  ) score -= 8;

  //at times, both the header and reset vector's first opcode will match ...
  //fallback and rely on info validity in these cases to determine more likely header.

  //a valid checksum is the biggest indicator of a valid header.
  if((checksum + complement) == 0xffff && (checksum != 0) && (complement != 0)) score += 4;

  if(addr == 0x007fc0 && mapper == 0x20) score += 2;  //0x20 is usually LoROM
  if(addr == 0x00ffc0 && mapper == 0x21) score += 2;  //0x21 is usually HiROM
  if(addr == 0x007fc0 && mapper == 0x22) score += 2;  //0x22 is usually ExLoROM
  if(addr == 0x40ffc0 && mapper == 0x25) score += 2;  //0x25 is usually ExHiROM

  if(data[addr + Company] == 0x33) score += 2;        //0x33 indicates extended header
  if(data[addr + RomType] < 0x08) score++;
  if(data[addr + RomSize] < 0x10) score++;
  if(data[addr + RamSize] < 0x08) score++;
  if(data[addr + CartRegion] < 14) score++;

  if(score < 0) score = 0;
  return score;
}

unsigned SuperFamicomCartridge::gameboy_ram_size(const uint8_t *data, unsigned size) {
  if(size < 512) return 0;
  switch(data[0x0149]) {
    case 0x00: return   0 * 1024;
    case 0x01: return   8 * 1024;
    case 0x02: return   8 * 1024;
    case 0x03: return  32 * 1024;
    case 0x04: return 128 * 1024;
    case 0x05: return 128 * 1024;
    default:   return 128 * 1024;
  }
}

bool SuperFamicomCartridge::gameboy_has_rtc(const uint8_t *data, unsigned size) {
  if(size < 512) return false;
  if(data[0x0147] == 0x0f ||data[0x0147] == 0x10) return true;
  return false;
}

}

#endif
