struct Board;

struct Chip {
  Board &board;
  void tick();
  Chip(Board &board);
};
