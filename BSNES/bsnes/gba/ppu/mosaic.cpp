void PPU::render_mosaic_background(unsigned id) {
  if(regs.mosaic.bghsize == 0) return;
  unsigned width = 1 + regs.mosaic.bghsize;
  auto &buffer = layer[id];

  for(unsigned x = 0; x < 240;) {
    for(unsigned m = 1; m < width; m++) {
      if(x + m >= 240) break;
      buffer[x + m] = buffer[x];
    }
    x += width;
  }
}

void PPU::render_mosaic_object() {
  if(regs.mosaic.objhsize == 0) return;
  unsigned width = 1 + regs.mosaic.objhsize;
  auto &buffer = layer[OBJ];

  Pixel mosaicPixel;
  mosaicPixel.mosaic = false;
  unsigned counter = 0;

  for(unsigned x = 0; x < 240; x++) {
    if(counter == width || mosaicPixel.mosaic == false) {
      mosaicPixel = buffer[x];
      if(counter == width) counter = 0;
    } else {
      if(buffer[x].mosaic) buffer[x] = mosaicPixel;
    }
    counter++;
  }
}
