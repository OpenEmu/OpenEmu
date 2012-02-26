class ST0018 {
public:
  void init();
  void load();
  void unload();
  void power();
  void reset();

  uint8 mmio_read(unsigned addr);
  void mmio_write(unsigned addr, uint8 data);

  enum mode_t { Waiting, BoardUpload };
  struct regs_t {
    mode_t mode;

    uint8 r3800;
    uint8 r3800_01;
    uint8 r3804;

    unsigned w3804;
    unsigned counter;
  } regs;

  enum PieceID {
    Pawn   = 0x00,  //foot soldier
    Lance  = 0x04,  //incense chariot
    Knight = 0x08,  //cassia horse
    Silver = 0x0c,  //silver general
    Gold   = 0x10,  //gold general
    Rook   = 0x14,  //flying chariot
    Bishop = 0x18,  //angle mover
    King   = 0x1c,  //king
  };

  enum PieceFlag {
    PlayerA = 0x20,
    PlayerB = 0x40,
  };

  uint8 board[9 * 9 + 16];

private:
  void op_board_upload();
  void op_board_upload(uint8 data);
  void op_b2();
  void op_b3();
  void op_b4();
  void op_b5();
  void op_query_chip();
};

extern ST0018 st0018;
