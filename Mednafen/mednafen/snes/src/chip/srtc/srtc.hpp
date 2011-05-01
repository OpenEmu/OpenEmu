class SRTC : public MMIO {
public:
  void init();
  void enable();
  void power();
  void reset();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  void serialize(serializer&);
  SRTC();

private:
  static const unsigned months[12];
  enum RtcMode { RtcReady, RtcCommand, RtcRead, RtcWrite };
  unsigned rtc_mode;
  signed rtc_index;

  void update_time();
  unsigned weekday(unsigned year, unsigned month, unsigned day);
};

extern SRTC srtc;
