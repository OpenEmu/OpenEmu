struct Cartridge : MMIO, property<Cartridge> {
  #include "mbc0/mbc0.hpp"
  #include "mbc1/mbc1.hpp"
  #include "mbc2/mbc2.hpp"
  #include "mbc3/mbc3.hpp"
  #include "mbc5/mbc5.hpp"
  #include "mmm01/mmm01.hpp"
  #include "huc1/huc1.hpp"
  #include "huc3/huc3.hpp"

  enum Mapper : unsigned {
    MBC0,
    MBC1,
    MBC2,
    MBC3,
    MBC5,
    MMM01,
    HuC1,
    HuC3,
    Unknown,
  };

  struct Information {
    string markup;

    Mapper mapper;
    bool ram;
    bool battery;
    bool rtc;
    bool rumble;

    unsigned romsize;
    unsigned ramsize;
  } information;

  readonly<bool> loaded;
  readonly<string> sha256;

  uint8_t *romdata;
  unsigned romsize;

  uint8_t *ramdata;
  unsigned ramsize;

  MMIO *mapper;
  bool bootrom_enable;

  void load(System::Revision revision, const string &markup, const stream &memory);
  void unload();

  uint8 rom_read(unsigned addr);
  void rom_write(unsigned addr, uint8 data);
  uint8 ram_read(unsigned addr);
  void ram_write(unsigned addr, uint8 data);

  uint8 mmio_read(uint16 addr);
  void mmio_write(uint16 addr, uint8 data);

  void power();

  void serialize(serializer&);
  Cartridge();
  ~Cartridge();
};

extern Cartridge cartridge;
