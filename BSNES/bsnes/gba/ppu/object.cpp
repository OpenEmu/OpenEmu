void PPU::render_objects() {
  if(regs.control.enable[OBJ] == false) return;
  for(unsigned n = 0; n < 128; n++) render_object(object[n]);
}

//px,py = pixel coordinates within sprite [0,0 - width,height)
//fx,fy = affine pixel coordinates
//pa,pb,pc,pd = affine pixel adjustments
//x,y = adjusted coordinates within sprite (linear = vflip/hflip, affine = rotation/zoom)
void PPU::render_object(Object &obj) {
  uint8 py = regs.vcounter - obj.y;
  if(obj.affine == 0 && obj.affinesize == 1) return;  //hidden
  if(py >= obj.height << obj.affinesize) return;  //offscreen

  auto &output = layer[OBJ];
  unsigned rowsize = regs.control.objmapping == 0 ? 32 >> obj.colors : obj.width / 8;
  unsigned baseaddr = obj.character * 32;

  if(obj.mosaic && regs.mosaic.objvsize) {
    signed mosaicy = (regs.vcounter / (1 + regs.mosaic.objvsize)) * (1 + regs.mosaic.objvsize);
    py = obj.y >= 160 || mosaicy - obj.y >= 0 ? mosaicy - obj.y : 0;
  }

  int16 pa = objectparam[obj.affineparam].pa;
  int16 pb = objectparam[obj.affineparam].pb;
  int16 pc = objectparam[obj.affineparam].pc;
  int16 pd = objectparam[obj.affineparam].pd;

  //center-of-sprite coordinates
  int16 centerx = obj.width  / 2;
  int16 centery = obj.height / 2;

  //origin coordinates (top-left of sprite)
  int28 originx = -(centerx << obj.affinesize);
  int28 originy = -(centery << obj.affinesize) + py;

  //fractional pixel coordinates
  int28 fx = originx * pa + originy * pb;
  int28 fy = originx * pc + originy * pd;

  for(unsigned px = 0; px < (obj.width << obj.affinesize); px++) {
    unsigned x, y;
    if(obj.affine == 0) {
      x = px;
      y = py;
      if(obj.hflip) x ^= obj.width  - 1;
      if(obj.vflip) y ^= obj.height - 1;
    } else {
      x = (fx >> 8) + centerx;
      y = (fy >> 8) + centery;
    }

    uint9 ox = obj.x + px;
    if(ox < 240 && x < obj.width && y < obj.height) {
      unsigned offset = (y / 8) * rowsize + (x / 8);
      offset = offset * 64 + (y & 7) * 8 + (x & 7);

      uint8 color = object_vram_read(baseaddr + (offset >> !obj.colors));
      if(obj.colors == 0) color = (x & 1) ? color >> 4 : color & 15;
      if(color) {
        if(obj.mode & 2) {
          windowmask[Obj][ox] = true;
        } else if(output[ox].enable == false || obj.priority < output[ox].priority) {
          if(obj.colors == 0) color = obj.palette * 16 + color;
          output[ox].write(true, obj.priority, pram[256 + color], obj.mode == 1, obj.mosaic);
        }
      }
    }

    fx += pa;
    fy += pc;
  }
}

uint8 PPU::object_vram_read(unsigned addr) const {
  if(regs.control.bgmode == 3 || regs.control.bgmode == 4 || regs.control.bgmode == 5) {
    if(addr <= 0x3fff) return 0u;
  }
  return vram[0x10000 + (addr & 0x7fff)];
}
