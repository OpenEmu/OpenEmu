class Cache {
public:
  uint8 *tiledata[3];
  uint8 *tilevalid[3];

  uint8* tile_2bpp(unsigned tile);
  uint8* tile_4bpp(unsigned tile);
  uint8* tile_8bpp(unsigned tile);
  uint8* tile(unsigned bpp, unsigned tile);

  void serialize(serializer&);
  Cache(PPU &self);

  PPU &self;
  friend class PPU;
};
