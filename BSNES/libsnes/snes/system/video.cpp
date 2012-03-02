#ifdef SYSTEM_CPP

Video video;

unsigned Video::palette30(unsigned color) {
  unsigned l = (color >> 15) & 15;
  unsigned b = (color >> 10) & 31;
  unsigned g = (color >>  5) & 31;
  unsigned r = (color >>  0) & 31;

  double L = (1.0 + l) / 16.0;
  unsigned R = L * ((r << 5) + (r << 0));
  unsigned G = L * ((g << 5) + (g << 0));
  unsigned B = L * ((b << 5) + (b << 0));

  return (R << 20) + (G << 10) + (B << 0);
}

void Video::generate(Format::e format) {
  for(unsigned n = 0; n < (1 << 19); n++) palette[n] = palette30(n);

  if(format == Format::RGB24) {
    for(unsigned n = 0; n < (1 << 19); n++) {
      unsigned color = palette[n];
      palette[n] = ((color >> 6) & 0xff0000) + ((color >> 4) & 0x00ff00) + ((color >> 2) & 0x0000ff);
    }
  }

  if(format == Format::RGB16) {
    for(unsigned n = 0; n < (1 << 19); n++) {
      unsigned color = palette[n];
      palette[n] = ((color >> 14) & 0xf800) + ((color >> 9) & 0x07e0) + ((color >> 5) & 0x001f);
    }
  }

  if(format == Format::RGB15) {
    for(unsigned n = 0; n < (1 << 19); n++) {
      unsigned color = palette[n];
      palette[n] = ((color >> 15) & 0x7c00) + ((color >> 10) & 0x03e0) + ((color >> 5) & 0x001f);
    }
  }
}

Video::Video() {
  palette = new unsigned[1 << 19];
}

Video::~Video() {
  delete[] palette;
}

//internal

const uint8_t Video::cursor[15 * 15] = {
  0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
  0,0,0,0,1,1,2,2,2,1,1,0,0,0,0,
  0,0,0,1,2,2,1,2,1,2,2,1,0,0,0,
  0,0,1,2,1,1,0,1,0,1,1,2,1,0,0,
  0,1,2,1,0,0,0,1,0,0,0,1,2,1,0,
  0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,
  1,2,1,0,0,1,1,2,1,1,0,0,1,2,1,
  1,2,2,1,1,2,2,2,2,2,1,1,2,2,1,
  1,2,1,0,0,1,1,2,1,1,0,0,1,2,1,
  0,1,2,1,0,0,1,2,1,0,0,1,2,1,0,
  0,1,2,1,0,0,0,1,0,0,0,1,2,1,0,
  0,0,1,2,1,1,0,1,0,1,1,2,1,0,0,
  0,0,0,1,2,2,1,2,1,2,2,1,0,0,0,
  0,0,0,0,1,1,2,2,2,1,1,0,0,0,0,
  0,0,0,0,0,0,1,1,1,0,0,0,0,0,0,
};

void Video::draw_cursor(uint16_t color, int x, int y) {
  uint32_t *data = (uint32_t*)ppu.output;
  if(ppu.interlace() && ppu.field()) data += 512;

  for(int cy = 0; cy < 15; cy++) {
    int vy = y + cy - 7;
    if(vy <= 0 || vy >= 240) continue;  //do not draw offscreen

    bool hires = (line_width[vy] == 512);
    for(int cx = 0; cx < 15; cx++) {
      int vx = x + cx - 7;
      if(vx < 0 || vx >= 256) continue;  //do not draw offscreen
      uint8_t pixel = cursor[cy * 15 + cx];
      if(pixel == 0) continue;
      uint32_t pixelcolor = (15 << 15) | ((pixel == 1) ? 0 : color);

      if(hires == false) {
        *((uint32_t*)data + vy * 1024 + vx) = pixelcolor;
      } else {
        *((uint32_t*)data + vy * 1024 + vx * 2 + 0) = pixelcolor;
        *((uint32_t*)data + vy * 1024 + vx * 2 + 1) = pixelcolor;
      }
    }
  }
}

void Video::update() {
  switch(config.controller_port2.i) {
  case Input::Device::SuperScope:
    if(dynamic_cast<SuperScope*>(input.port2)) {
      SuperScope &device = (SuperScope&)*input.port2;
      draw_cursor(0x7c00, device.x, device.y);
    }
    break;
  case Input::Device::Justifier:
  case Input::Device::Justifiers:
    if(dynamic_cast<Justifier*>(input.port2)) {
      Justifier &device = (Justifier&)*input.port2;
      draw_cursor(0x001f, device.player1.x, device.player1.y);
      if(device.chained == false) break;
      draw_cursor(0x02e0, device.player2.x, device.player2.y);
    }
    break;
  }

  uint32_t *data = (uint32_t*)ppu.output;
  if(ppu.interlace() && ppu.field()) data += 512;

  if(hires) {
    //normalize line widths
    for(unsigned y = 0; y < 240; y++) {
      if(line_width[y] == 512) continue;
      uint32_t *buffer = data + y * 1024;
      for(signed x = 255; x >= 0; x--) {
        buffer[(x * 2) + 0] = buffer[(x * 2) + 1] = buffer[x];
      }
    }
  }

  interface->videoRefresh(ppu.surface, hires, ppu.interlace(), ppu.overscan());

  hires = false;
}

void Video::scanline() {
  unsigned y = cpu.vcounter();
  if(y >= 240) return;

  hires |= ppu.hires();
  unsigned width = (ppu.hires() == false ? 256 : 512);
  line_width[y] = width;
}

void Video::init() {
  hires = false;
  foreach(n, line_width) n = 256;
}

#endif
