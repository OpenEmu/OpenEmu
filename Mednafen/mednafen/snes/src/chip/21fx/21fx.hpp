class S21fx : public MMIO {
public:
  void enter();

  void init();
  void enable();
  void power();
  void reset();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  void base(const string &path);
  bool exists();

  void serialize(serializer&);
  S21fx();

private:
  string basepath;
  file datafile;
  file audiofile;

  enum Flag {
    DataPortBusy = 0x80,
    AudioBusy    = 0x40,
    AudioPlaying = 0x20,
  };

  struct MMIO {
    uint8 status;
    uint64 shift_register;

    uint32 data_offset;
    uint32 audio_offset;
    uint16 audio_track;
    uint8 audio_volume_left;
    uint8 audio_volume_right;
    bool audio_repeat;
    bool audio_pause;
  } mmio;
};

extern S21fx s21fx;
