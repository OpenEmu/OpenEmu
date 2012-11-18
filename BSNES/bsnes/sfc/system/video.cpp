#ifdef SYSTEM_CPP

Video video;

void Video::generate_palette() {
  for(unsigned color = 0; color < (1 << 19); color++) {
    unsigned l = (color >> 15) & 15;
    unsigned b = (color >> 10) & 31;
    unsigned g = (color >>  5) & 31;
    unsigned r = (color >>  0) & 31;

    double L = (1.0 + l) / 16.0;
    if(l == 0) L *= 0.5;
    unsigned R = L * (r << 11 | r << 6 | r << 1 | r >> 4);
    unsigned G = L * (g << 11 | g << 6 | g << 1 | g >> 4);
    unsigned B = L * (b << 11 | b << 6 | b << 1 | b >> 4);

    palette[color] = interface->videoColor(color, R, G, B);
  }
}

Video::Video() {
  palette = new unsigned[1 << 19]();
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
        *((uint32_t*)data + vy * 1024 + vx) = palette[pixelcolor];
      } else {
        *((uint32_t*)data + vy * 1024 + vx * 2 + 0) = palette[pixelcolor];
        *((uint32_t*)data + vy * 1024 + vx * 2 + 1) = palette[pixelcolor];
      }
    }
  }
}

void Video::update() {
  switch(config.controller_port2) {
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

  //overscan: when disabled, shift image down (by scrolling video buffer up) to center image onscreen
  //(memory before ppu.output is filled with black scanlines)
  interface->videoRefresh(
    ppu.output - (ppu.overscan() ? 0 : 7 * 1024),
    4 * (1024 >> ppu.interlace()),
    256 << hires,
    240 << ppu.interlace()
  );

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
  for(auto &n : line_width) n = 256;
}

#endif
