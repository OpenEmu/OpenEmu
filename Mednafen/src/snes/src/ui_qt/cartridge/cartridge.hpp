class Cartridge {
public:
  string name;        //printable name
  string fileName;    //ideal file name for saving data to disk
  string baseName;    //physical cartridge file name
  string slotAName;   //Sufami Turbo slot A file name or BS-X slot file name
  string slotBName;   //Sufami Turbo slot B file name
  bool patchApplied;  //true if UPS patch was applied to image

  struct Information {
    string name;
    string region;
    unsigned romSize;
    unsigned ramSize;
  };

  bool information(const char*, Information&);
  bool saveStatesSupported();

  bool loadNormal(const char*);
  bool loadBsxSlotted(const char*, const char*);
  bool loadBsx(const char*, const char*);
  bool loadSufamiTurbo(const char*, const char *, const char*);
  bool loadSuperGameBoy(const char*, const char*);
  void saveMemory();
  void unload();

  void loadCheats();
  void saveCheats();

private:
  bool loadCartridge(string&, SNES::MappedRAM&);
  bool loadMemory(const char*, const char*, SNES::MappedRAM&);
  bool saveMemory(const char*, const char*, SNES::MappedRAM&);
  string decodeShiftJIS(const char*);
};

extern Cartridge cartridge;
