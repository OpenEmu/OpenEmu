class Decomp {
public:
  uint8 read();
  void init(unsigned mode, unsigned offset, unsigned index);
  void reset();

  void serialize(serializer&);
  Decomp();
  ~Decomp();

private:
  unsigned decomp_mode;
  unsigned decomp_offset;

  //read() will spool chunks half the size of decomp_buffer_size
  enum { decomp_buffer_size = 64 }; //must be >= 64, and must be a power of two
  uint8 *decomp_buffer;
  unsigned decomp_buffer_rdoffset;
  unsigned decomp_buffer_wroffset;
  unsigned decomp_buffer_length;

  void write(uint8 data);
  uint8 dataread();

  void mode0(bool init);
  void mode1(bool init);
  void mode2(bool init);

  static const uint8 evolution_table[53][4];
  static const uint8 mode2_context_table[32][2];

  struct ContextState {
    uint8 index;
    uint8 invert;
  } context[32];

  uint8 probability(unsigned n);
  uint8 next_lps(unsigned n);
  uint8 next_mps(unsigned n);
  bool toggle_invert(unsigned n);

  unsigned deinterleave_2x8(unsigned data);
  unsigned deinterleave_4x8(unsigned data);
};
