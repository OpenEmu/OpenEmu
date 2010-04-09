#ifdef CX4_CPP

//Sprite Functions
void Cx4::op00() {
  switch(reg[0x4d]) {
    case 0x00: op00_00(); break;
    case 0x03: op00_03(); break;
    case 0x05: op00_05(); break;
    case 0x07: op00_07(); break;
    case 0x08: op00_08(); break;
    case 0x0b: op00_0b(); break;
    case 0x0c: op00_0c(); break;
  }
}

//Draw Wireframe
void Cx4::op01() {
  memset(ram + 0x300, 0, 2304);
  C4DrawWireFrame();
}

//Propulsion
void Cx4::op05() {
  int32 temp = 0x10000;
  if(readw(0x1f83)) {
    temp = sar((temp / readw(0x1f83)) * readw(0x1f81), 8);
  }
  writew(0x1f80, temp);
}

//Set Vector length
void Cx4::op0d() {
  C41FXVal    = readw(0x1f80);
  C41FYVal    = readw(0x1f83);
  C41FDistVal = readw(0x1f86);
  double tanval = sqrt(((double)C41FYVal) * ((double)C41FYVal) + ((double)C41FXVal) * ((double)C41FXVal));
  tanval = (double)C41FDistVal / tanval;
  C41FYVal = (int16)(((double)C41FYVal * tanval) * 0.99);
  C41FXVal = (int16)(((double)C41FXVal * tanval) * 0.98);
  writew(0x1f89, C41FXVal);
  writew(0x1f8c, C41FYVal);
}

//Triangle
void Cx4::op10() {
  r0 = ldr(0);
  r1 = ldr(1);

  r4 = r0 & 0x1ff;
  if(r1 & 0x8000)r1 |= ~0x7fff;

  mul(cos(r4), r1, r5, r2);
  r5 = (r5 >> 16) & 0xff;
  r2 = (r2 << 8) + r5;

  mul(sin(r4), r1, r5, r3);
  r5 = (r5 >> 16) & 0xff;
  r3 = (r3 << 8) + r5;

  str(0, r0);
  str(1, r1);
  str(2, r2);
  str(3, r3);
  str(4, r4);
  str(5, r5);
}

//Triangle
void Cx4::op13() {
  r0 = ldr(0);
  r1 = ldr(1);

  r4 = r0 & 0x1ff;

  mul(cos(r4), r1, r5, r2);
  r5 = (r5 >> 8) & 0xffff;
  r2 = (r2 << 16) + r5;

  mul(sin(r4), r1, r5, r3);
  r5 = (r5 >> 8) & 0xffff;
  r3 = (r3 << 16) + r5;

  str(0, r0);
  str(1, r1);
  str(2, r2);
  str(3, r3);
  str(4, r4);
  str(5, r5);
}

//Pythagorean
void Cx4::op15() {
  C41FXVal = readw(0x1f80);
  C41FYVal = readw(0x1f83);
  C41FDist = (int16)sqrt((double)C41FXVal * (double)C41FXVal + (double)C41FYVal * (double)C41FYVal);
  writew(0x1f80, C41FDist);
}

//Calculate distance
void Cx4::op1f() {
  C41FXVal = readw(0x1f80);
  C41FYVal = readw(0x1f83);
  if(!C41FXVal) {
    C41FAngleRes = (C41FYVal > 0) ? 0x080 : 0x180;
  } else {
    double tanval = ((double)C41FYVal) / ((double)C41FXVal);
    C41FAngleRes = (short)(atan(tanval) / (PI * 2) * 512);
    C41FAngleRes = C41FAngleRes;
    if(C41FXVal < 0) {
      C41FAngleRes += 0x100;
    }
    C41FAngleRes &= 0x1ff;
  }
  writew(0x1f86, C41FAngleRes);
}

//Trapezoid
void Cx4::op22() {
  int16 angle1 = readw(0x1f8c) & 0x1ff;
  int16 angle2 = readw(0x1f8f) & 0x1ff;
  int32 tan1 = Tan(angle1);
  int32 tan2 = Tan(angle2);
  int16 y = readw(0x1f83) - readw(0x1f89);
  int16 left, right;

  for(int32 j = 0; j < 225; j++, y++) {
    if(y >= 0) {
      left  = sar((int32)tan1 * y, 16) - readw(0x1f80) + readw(0x1f86);
      right = sar((int32)tan2 * y, 16) - readw(0x1f80) + readw(0x1f86) + readw(0x1f93);

      if(left < 0 && right < 0) {
        left  = 1;
        right = 0;
      } else if(left < 0) {
        left  = 0;
      } else if(right < 0) {
        right = 0;
      }

      if(left > 255 && right > 255) {
        left  = 255;
        right = 254;
      } else if(left > 255) {
        left  = 255;
      } else if(right > 255) {
        right = 255;
      }
    } else {
      left  = 1;
      right = 0;
    }
    ram[j + 0x800] = (uint8)left;
    ram[j + 0x900] = (uint8)right;
  }
}

//Multiply
void Cx4::op25() {
  r0 = ldr(0);
  r1 = ldr(1);
  mul(r0, r1, r0, r1);
  str(0, r0);
  str(1, r1);
}

//Transform Coords
void Cx4::op2d() {
  C4WFXVal  = readw(0x1f81);
  C4WFYVal  = readw(0x1f84);
  C4WFZVal  = readw(0x1f87);
  C4WFX2Val = read (0x1f89);
  C4WFY2Val = read (0x1f8a);
  C4WFDist  = read (0x1f8b);
  C4WFScale = readw(0x1f90);
  C4TransfWireFrame2();
  writew(0x1f80, C4WFXVal);
  writew(0x1f83, C4WFYVal);
}

//Sum
void Cx4::op40() {
  r0 = 0;
  for(uint32 i=0;i<0x800;i++) {
    r0 += ram[i];
  }
  str(0, r0);
}

//Square
void Cx4::op54() {
  r0 = ldr(0);
  mul(r0, r0, r1, r2);
  str(1, r1);
  str(2, r2);
}

//Immediate Register
void Cx4::op5c() {
  str(0, 0x000000);
  immediate_reg(0);
}

//Immediate Register (Multiple)
void Cx4::op5e() { immediate_reg( 0); }
void Cx4::op60() { immediate_reg( 3); }
void Cx4::op62() { immediate_reg( 6); }
void Cx4::op64() { immediate_reg( 9); }
void Cx4::op66() { immediate_reg(12); }
void Cx4::op68() { immediate_reg(15); }
void Cx4::op6a() { immediate_reg(18); }
void Cx4::op6c() { immediate_reg(21); }
void Cx4::op6e() { immediate_reg(24); }
void Cx4::op70() { immediate_reg(27); }
void Cx4::op72() { immediate_reg(30); }
void Cx4::op74() { immediate_reg(33); }
void Cx4::op76() { immediate_reg(36); }
void Cx4::op78() { immediate_reg(39); }
void Cx4::op7a() { immediate_reg(42); }
void Cx4::op7c() { immediate_reg(45); }

//Immediate ROM
void Cx4::op89() {
  str(0, 0x054336);
  str(1, 0xffffff);
}

#endif
