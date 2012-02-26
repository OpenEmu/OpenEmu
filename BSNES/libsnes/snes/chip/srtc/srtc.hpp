class SRTC {
public:
  uint8 rtc[20];

  void init();
  void load();
  void unload();
  void power();
  void reset();

  uint8 read(unsigned addr);
  void write(unsigned addr, uint8 data);

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
