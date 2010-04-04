class StateManager : noncopyable {
public:
  enum {
    SlotInvalid     = 0xff,

    SlotSize        = 1,
    DateTimeSize    = 19,
    DescriptionSize = 512,

    HeaderSize      = 8 + (256 * SlotSize) + (256 * DateTimeSize) + (256 * DescriptionSize),
    SlotIndex       = 8,
    DateTimeIndex   = SlotIndex + (256 * SlotSize),
    DescIndex       = DateTimeIndex + (256 * DateTimeSize),
  };

  void list(lstring&) const;
  bool load(const char *filename);
  bool set_description(const char *filename, uint8 slot, const char *description);
  serializer load(const char *filename, uint8 slot);
  bool save(const char *filename, uint8 slot, serializer&, const char *description);
  bool erase(const char *filename, uint8 slot);

private:
  struct Info {
    unsigned slotcount;
    uint8 slot[256 * SlotSize];
    char datetime[256 * DateTimeSize];
    char description[256 * DescriptionSize];
  } info;

  bool create(const char *filename) const;
  uint8 findslot(uint8 slot) const;
};

extern StateManager statemanager;
