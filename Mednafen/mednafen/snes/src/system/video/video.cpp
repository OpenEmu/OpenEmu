#ifdef SYSTEM_CPP

Video video;

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
  for(int cy = 0; cy < 15; cy++) {
    int vy = y + cy - 7;
    if(vy <= 0 || vy >= 240) continue;  //do not draw offscreen

    bool hires = (pline_width[vy] == 512);
    for(int cx = 0; cx < 15; cx++) {
      int vx = x + cx - 7;
      if(vx < 0 || vx >= 256) continue;  //do not draw offscreen
      uint8_t pixel = cursor[cy * 15 + cx];
      if(pixel == 0) continue;
      uint16_t pixelcolor = (pixel == 1) ? 0 : color;

      if(hires == false) {
        *((uint16_t*)ppu.output + vy * 1024 +   0 + vx) = pixelcolor;
        *((uint16_t*)ppu.output + vy * 1024 + 512 + vx) = pixelcolor;
      } else {
        *((uint16_t*)ppu.output + vy * 1024 +   0 + vx * 2 + 0) = pixelcolor;
        *((uint16_t*)ppu.output + vy * 1024 + 512 + vx * 2 + 0) = pixelcolor;
        *((uint16_t*)ppu.output + vy * 1024 +   0 + vx * 2 + 1) = pixelcolor;
        *((uint16_t*)ppu.output + vy * 1024 + 512 + vx * 2 + 1) = pixelcolor;
      }
    }
  }
}

void Video::update() {
  uint16_t *data = (uint16_t*)ppu.output;
  unsigned width, height;

  switch(input.port[1].device) {
    case Input::DeviceSuperScope: draw_cursor(0x001f, input.port[1].superscope.x, input.port[1].superscope.y); break;
    case Input::DeviceJustifiers: draw_cursor(0x02e0, input.port[1].justifier.x2, input.port[1].justifier.y2); //fallthrough
    case Input::DeviceJustifier:  draw_cursor(0x001f, input.port[1].justifier.x1, input.port[1].justifier.y1); break;
  }

  unsigned yoffset = 1;  //scanline 0 is always black, skip this line for video output
  if(mode == ModeNTSC && ppu.overscan()) yoffset += 8;  //NTSC overscan centers x240 height image

  switch(mode) { default:
    case ModeNTSC: { width = 256; height = 224; } break;
    case ModePAL:  { width = 256; height = 239; } break;
  }

  if(frame_hires) width <<= 1;
  if(frame_interlace) height <<= 1;

  system.interface->video_refresh(
    data + yoffset * 1024,
    /* pitch  = */ height <= 240 ? 2048 : 1024,
    /* line[] = */ height <= 240 ? (pline_width + yoffset) : (iline_width + yoffset * 2),
    width, height
  );

  frame_hires = false;
  frame_interlace = false;
}

void Video::scanline() {
  unsigned y = cpu.vcounter();
  if(y >= 240) return;

  unsigned width = (ppu.hires() == false ? 256 : 512);
  pline_width[y] = width;
  iline_width[y * 2 + (int)cpu.field()] = width;

  frame_hires |= ppu.hires();
  frame_interlace |= ppu.interlace();
}

void Video::set_mode(Mode mode_) {
  mode = mode_;
}

void Video::init() {
  for(unsigned i = 0; i < 240; i++) pline_width[i] = 256;
  for(unsigned i = 0; i < 480; i++) iline_width[i] = 256;
  frame_hires = false;
  frame_interlace = false;
  set_mode(ModeNTSC);
}

#endif
