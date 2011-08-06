#ifdef ST010_CPP

//ST-010 emulation code - Copyright (C) 2003 The Dumper, Matthew Kendora, Overload, Feather
//bsnes port - Copyright (C) 2007 byuu

void ST010::op_01(int16 x0, int16 y0, int16 &x1, int16 &y1, int16 &quadrant, int16 &theta) {
  if((x0 < 0) && (y0 < 0)) {
    x1 = -x0;
    y1 = -y0;
    quadrant = -0x8000;
  } else if(x0 < 0) {
    x1 = y0;
    y1 = -x0;
    quadrant = -0x4000;
  } else if(y0 < 0) {
    x1 = -y0;
    y1 = x0;
    quadrant = 0x4000;
  } else {
    x1 = x0;
    y1 = y0;
    quadrant = 0x0000;
  }

  while((x1 > 0x1f) || (y1 > 0x1f)) {
    if(x1 > 1) { x1 >>= 1; }
    if(y1 > 1) { y1 >>= 1; }
  }

  if(y1 == 0) { quadrant += 0x4000; }

  theta = (arctan[y1][x1] << 8) ^ quadrant;
}

//

void ST010::op_01() {
  int16 x0 = readw(0x0000);
  int16 y0 = readw(0x0002);
  int16 x1, y1, quadrant, theta;

  op_01(x0, y0, x1, y1, quadrant, theta);

  writew(0x0000, x1);
  writew(0x0002, y1);
  writew(0x0004, quadrant);
//writew(0x0006, y0);  //Overload's docs note this write occurs, SNES9x disagrees
  writew(0x0010, theta);
}

void ST010::op_02() {
  int16 positions = readw(0x0024);
  uint16 *places  = (uint16*)(ram + 0x0040);
  uint16 *drivers = (uint16*)(ram + 0x0080);

  bool sorted;
  uint16 temp;
  if(positions > 1) {
    do {
      sorted = true;
      for(int i = 0; i < positions - 1; i++) {
        if(places[i] < places[i + 1]) {
          temp = places[i + 1];
          places[i + 1] = places[i];
          places[i] = temp;

          temp = drivers[i + 1];
          drivers[i + 1] = drivers[i];
          drivers[i] = temp;

          sorted = false;
        }
      }
      positions--;
    } while(!sorted);
  }
}

void ST010::op_03() {
  int16 x0 = readw(0x0000);
  int16 y0 = readw(0x0002);
  int16 multiplier = readw(0x0004);
  int32 x1, y1;

  x1 = x0 * multiplier << 1;
  y1 = y0 * multiplier << 1;

  writed(0x0010, x1);
  writed(0x0014, y1);
}

void ST010::op_04() {
  int16 x = readw(0x0000);
  int16 y = readw(0x0002);
  int16 square;
  //calculate the vector length of (x,y)
  square = (int16)sqrt((double)(y * y + x * x));

  writew(0x0010, square);
}

void ST010::op_05() {
  int32 dx, dy;
  int16 a1, b1, c1;
  uint16 o1;
  bool wrap = false;

  //target (x,y) coordinates
  int16 ypos_max = readw(0x00c0);
  int16 xpos_max = readw(0x00c2);

  //current coordinates and direction
  int32 ypos = readd(0x00c4);
  int32 xpos = readd(0x00c8);
  uint16 rot = readw(0x00cc);

  //physics
  uint16 speed = readw(0x00d4);
  uint16 accel = readw(0x00d6);
  uint16 speed_max = readw(0x00d8);

  //special condition acknowledgement
  int16 system = readw(0x00da);
  int16 flags = readw(0x00dc);

  //new target coordinates
  int16 ypos_new = readw(0x00de);
  int16 xpos_new = readw(0x00e0);

  //mask upper bit
  xpos_new &= 0x7fff;

  //get the current distance
  dx = xpos_max - (xpos >> 16);
  dy = ypos_max - (ypos >> 16);

  //quirk: clear and move in9
  writew(0x00d2, 0xffff);
  writew(0x00da, 0x0000);

  //grab the target angle
  op_01(dy, dx, a1, b1, c1, (int16&)o1);

  //check for wrapping
  if(abs(o1 - rot) > 0x8000) {
    o1 += 0x8000;
    rot += 0x8000;
    wrap = true;
  }

  uint16 old_speed = speed;

  //special case
  if(abs(o1 - rot) == 0x8000) {
    speed = 0x100;
  }

  //slow down for sharp curves
  else if(abs(o1 - rot) >= 0x1000) {
  uint32 slow = abs(o1 - rot);
    slow >>= 4;  //scaling
    speed -= slow;
  }

  //otherwise accelerate
  else {
    speed += accel;
    if(speed > speed_max) {
      speed = speed_max;  //clip speed
    }
  }

  //prevent negative/positive overflow
  if(abs(old_speed - speed) > 0x8000) {
    if(old_speed < speed) { speed = 0; }
    else speed = 0xff00;
  }

  //adjust direction by so many degrees
  //be careful of negative adjustments
  if((o1 > rot && (o1 - rot) > 0x80) || (o1 < rot && (rot - o1) >= 0x80)) {
    if(o1 < rot) { rot -= 0x280; }
    else if(o1 > rot) { rot += 0x280; }
  }

  //turn off wrapping
  if(wrap) { rot -= 0x8000; }

  //now check the distances (store for later)
  dx = (xpos_max << 16) - xpos;
  dy = (ypos_max << 16) - ypos;
  dx >>= 16;
  dy >>= 16;

  //if we're in so many units of the target, signal it
  if((system && (dy <= 6 && dy >= -8) && (dx <= 126 && dx >= -128)) || (!system && (dx <= 6 && dx >= -8) && (dy <= 126 && dy >= -128))) {
    //announce our new destination and flag it
    xpos_max = xpos_new & 0x7fff;
    ypos_max = ypos_new;
    flags |= 0x08;
  }

  //update position
  xpos -= (cos(rot) * 0x400 >> 15) * (speed >> 8) << 1;
  ypos -= (sin(rot) * 0x400 >> 15) * (speed >> 8) << 1;

  //quirk: mask upper byte
  xpos &= 0x1fffffff;
  ypos &= 0x1fffffff;

  writew(0x00c0, ypos_max);
  writew(0x00c2, xpos_max);
  writed(0x00c4, ypos);
  writed(0x00c8, xpos);
  writew(0x00cc, rot);
  writew(0x00d4, speed);
  writew(0x00dc, flags);
}

void ST010::op_06() {
  int16 multiplicand = readw(0x0000);
  int16 multiplier = readw(0x0002);
  int32 product;

  product = multiplicand * multiplier << 1;

  writed(0x0010, product);
}

void ST010::op_07() {
  int16 theta = readw(0x0000);

  int16 data;
  for(int i = 0, offset = 0; i < 176; i++) {
    data = mode7_scale[i] * cos(theta) >> 15;
    writew(0x00f0 + offset, data);
    writew(0x0510 + offset, data);

    data = mode7_scale[i] * sin(theta) >> 15;
    writew(0x0250 + offset, data);
    if(data) { data = ~data; }
    writew(0x03b0 + offset, data);

    offset += 2;
  }
}

void ST010::op_08() {
  int16 x0 = readw(0x0000);
  int16 y0 = readw(0x0002);
  int16 theta = readw(0x0004);
  int16 x1, y1;

  x1 = (y0 * sin(theta) >> 15) + (x0 * cos(theta) >> 15);
  y1 = (y0 * cos(theta) >> 15) - (x0 * sin(theta) >> 15);

  writew(0x0010, x1);
  writew(0x0012, y1);
}

#endif
