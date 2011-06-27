class Cartridge : property<Cartridge> {
public:
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

  readonly<bool> loaded;     //is a base cartridge inserted?
  readonly<unsigned> crc32;  //crc32 of all cartridges (base+slot(s))

  readonly<Mode> mode;
  readonly<Type> type;
  readonly<Region> region;
  readonly<MemoryMapper> mapper;
  readonly<DSP1MemoryMapper> dsp1_mapper;

  readonly<bool> has_bsx_slot;
  readonly<bool> has_superfx;
  readonly<bool> has_sa1;
  readonly<bool> has_srtc;
  readonly<bool> has_sdd1;
  readonly<bool> has_spc7110;
  readonly<bool> has_spc7110rtc;
  readonly<bool> has_cx4;
  readonly<bool> has_dsp1;
  readonly<bool> has_dsp2;
  readonly<bool> has_dsp3;
  readonly<bool> has_dsp4;
  readonly<bool> has_obc1;
  readonly<bool> has_st010;
  readonly<bool> has_st011;
  readonly<bool> has_st018;
  bool has_21fx() const;

  void load(Mode);
  void unload();

  void serialize(serializer&);
  Cartridge();
  ~Cartridge();

private:
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

  unsigned ram_size;
  void read_header(const uint8_t *data, unsigned size);
  unsigned find_header(const uint8_t *data, unsigned size) const;
  unsigned score_header(const uint8_t *data, unsigned size, unsigned addr) const;

  unsigned gameboy_ram_size() const;
  unsigned gameboy_rtc_size() const;
};

namespace memory {
  extern MappedRAM cartrom, cartram, cartrtc;
  extern MappedRAM bsxflash, bsxram, bsxpram;
  extern MappedRAM stArom, stAram;
  extern MappedRAM stBrom, stBram;
  extern MappedRAM gbrom, gbram, gbrtc;
};

extern Cartridge cartridge;
