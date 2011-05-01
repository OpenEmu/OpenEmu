#ifdef CX4_CPP

//Build OAM
void Cx4::op00_00() {
  uint32 oamptr = ram[0x626] << 2;
  for(int32 i = 0x1fd; i > oamptr && i >= 0; i -= 4) {
    //clear oam-to-be
    if(i >= 0) ram[i] = 0xe0;
  }

  uint16 globalx, globaly;
  uint32 oamptr2;
  int16  sprx, spry;
  uint8  sprname, sprattr;
  uint8  sprcount;

  globalx = readw(0x621);
  globaly = readw(0x623);
  oamptr2 = 0x200 + (ram[0x626] >> 2);

  if(!ram[0x620]) return;

  sprcount = 128 - ram[0x626];
  uint8 offset = (ram[0x626] & 3) * 2;
  uint32 srcptr = 0x220;

  for(int i = ram[0x620]; i > 0 && sprcount > 0; i--, srcptr += 16) {
    sprx = readw(srcptr)     - globalx;
    spry = readw(srcptr + 2) - globaly;
    sprname = ram[srcptr + 5];
    sprattr = ram[srcptr + 4] | ram[srcptr + 6];

    uint32 spraddr = readl(srcptr + 7);
    if(bus.read(spraddr)) {
      int16 x, y;
      for(int sprcnt = bus.read(spraddr++); sprcnt > 0 && sprcount > 0; sprcnt--, spraddr += 4) {
        x = (int8)bus.read(spraddr + 1);
        if(sprattr & 0x40) {
          x = -x - ((bus.read(spraddr) & 0x20) ? 16 : 8);
        }
        x += sprx;
        if(x >= -16 && x <= 272) {
          y = (int8)bus.read(spraddr + 2);
          if(sprattr & 0x80) {
            y = -y - ((bus.read(spraddr) & 0x20) ? 16 : 8);
          }
          y += spry;
          if(y >= -16 && y <= 224) {
            ram[oamptr    ] = (uint8)x;
            ram[oamptr + 1] = (uint8)y;
            ram[oamptr + 2] = sprname + bus.read(spraddr + 3);
            ram[oamptr + 3] = sprattr ^ (bus.read(spraddr) & 0xc0);
            ram[oamptr2] &= ~(3 << offset);
            if(x & 0x100) ram[oamptr2] |= 1 << offset;
            if(bus.read(spraddr) & 0x20) ram[oamptr2] |= 2 << offset;
            oamptr += 4;
            sprcount--;
            offset = (offset + 2) & 6;
            if(!offset)oamptr2++;
          }
        }
      }
    } else if(sprcount > 0) {
      ram[oamptr    ] = (uint8)sprx;
      ram[oamptr + 1] = (uint8)spry;
      ram[oamptr + 2] = sprname;
      ram[oamptr + 3] = sprattr;
      ram[oamptr2] &= ~(3 << offset);
      if(sprx & 0x100) ram[oamptr2] |= 3 << offset;
      else ram[oamptr2] |= 2 << offset;
      oamptr += 4;
      sprcount--;
      offset = (offset + 2) & 6;
      if(!offset) oamptr2++;
    }
  }
}

//Scale and Rotate
void Cx4::op00_03() {
  C4DoScaleRotate(0);
}

//Transform Lines
void Cx4::op00_05() {
  C4WFX2Val = read(0x1f83);
  C4WFY2Val = read(0x1f86);
  C4WFDist  = read(0x1f89);
  C4WFScale = read(0x1f8c);

//Transform Vertices
uint32 ptr = 0;
  for(int32 i = readw(0x1f80); i > 0; i--, ptr += 0x10) {
    C4WFXVal = readw(ptr + 1);
    C4WFYVal = readw(ptr + 5);
    C4WFZVal = readw(ptr + 9);
    C4TransfWireFrame();

    //Displace
    writew(ptr + 1, C4WFXVal + 0x80);
    writew(ptr + 5, C4WFYVal + 0x50);
  }

  writew(0x600,     23);
  writew(0x602,     0x60);
  writew(0x605,     0x40);
  writew(0x600 + 8, 23);
  writew(0x602 + 8, 0x60);
  writew(0x605 + 8, 0x40);

  ptr = 0xb02;
  uint32 ptr2 = 0;

  for(int32 i = readw(0xb00); i > 0; i--, ptr += 2, ptr2 += 8) {
    C4WFXVal  = readw((read(ptr + 0) << 4) + 1);
    C4WFYVal  = readw((read(ptr + 0) << 4) + 5);
    C4WFX2Val = readw((read(ptr + 1) << 4) + 1);
    C4WFY2Val = readw((read(ptr + 1) << 4) + 5);
    C4CalcWireFrame();
    writew(ptr2 + 0x600, C4WFDist ? C4WFDist : 1);
    writew(ptr2 + 0x602, C4WFXVal);
    writew(ptr2 + 0x605, C4WFYVal);
  }
}

//Scale and Rotate
void Cx4::op00_07() {
  C4DoScaleRotate(64);
}

//Draw Wireframe
void Cx4::op00_08() {
  C4DrawWireFrame();
}

//Disintegrate
void Cx4::op00_0b() {
  uint8  width, height;
  uint32 startx, starty;
  uint32 srcptr;
  uint32 x, y;
  int32  scalex, scaley;
  int32  cx, cy;
  int32  i, j;

  width  = read(0x1f89);
  height = read(0x1f8c);
  cx     = readw(0x1f80);
  cy     = readw(0x1f83);

  scalex = (int16)readw(0x1f86);
  scaley = (int16)readw(0x1f8f);
  startx = -cx * scalex + (cx << 8);
  starty = -cy * scaley + (cy << 8);
  srcptr = 0x600;

  for(i = 0; i < (width * height) >> 1; i++) {
    write(i, 0);
  }

  for(y = starty, i = 0;i < height; i++, y += scaley) {
    for(x = startx, j = 0;j < width; j++, x += scalex) {
      if((x >> 8) < width && (y >> 8) < height && (y >> 8) * width + (x >> 8) < 0x2000) {
        uint8 pixel = (j & 1) ? (ram[srcptr] >> 4) : (ram[srcptr]);
        int32 index = (y >> 11) * width * 4 + (x >> 11) * 32 + ((y >> 8) & 7) * 2;
        uint8 mask = 0x80 >> ((x >> 8) & 7);

        if(pixel & 1) ram[index     ] |= mask;
        if(pixel & 2) ram[index +  1] |= mask;
        if(pixel & 4) ram[index + 16] |= mask;
        if(pixel & 8) ram[index + 17] |= mask;
      }
      if(j & 1) srcptr++;
    }
  }
}

//Bitplane Wave
void Cx4::op00_0c() {
  uint32 destptr = 0;
  uint32 waveptr = read(0x1f83);
  uint16 mask1   = 0xc0c0;
  uint16 mask2   = 0x3f3f;

  for(int j = 0; j < 0x10; j++) {
    do {
      int16 height = -((int8)read(waveptr + 0xb00)) - 16;
      for(int i = 0; i < 40; i++) {
        uint16 temp = readw(destptr + wave_data[i]) & mask2;
        if(height >= 0) {
          if(height < 8) {
            temp |= mask1 & readw(0xa00 + height * 2);
          } else {
            temp |= mask1 & 0xff00;
          }
        }
        writew(destptr + wave_data[i], temp);
        height++;
      }
      waveptr = (waveptr + 1) & 0x7f;
      mask1   = (mask1 >> 2) | (mask1 << 6);
      mask2   = (mask2 >> 2) | (mask2 << 6);
    } while(mask1 != 0xc0c0);
    destptr += 16;

    do {
      int16 height = -((int8)read(waveptr + 0xb00)) - 16;
      for(int i = 0; i < 40; i++) {
        uint16 temp = readw(destptr + wave_data[i]) & mask2;
        if(height >= 0) {
          if(height < 8) {
            temp |= mask1 & readw(0xa10 + height * 2);
          } else {
            temp |= mask1 & 0xff00;
          }
        }
        writew(destptr + wave_data[i], temp);
        height++;
      }
      waveptr = (waveptr + 1) & 0x7f;
      mask1   = (mask1 >> 2) | (mask1 << 6);
      mask2   = (mask2 >> 2) | (mask2 << 6);
    } while(mask1 != 0xc0c0);
    destptr += 16;
  }
}

#endif
