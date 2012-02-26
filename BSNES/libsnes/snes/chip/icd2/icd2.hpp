class ICD2 : public GameBoy::Interface, public Coprocessor {
public:
  unsigned revision;

  static void Enter();
  void enter();

  void init();
  void load();
  void unload();
  void power();
  void reset();

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

  void serialize(serializer&);

private:
  #include "interface/interface.hpp"
  #include "mmio/mmio.hpp"
};

extern ICD2 icd2;
