string SPC700::disassemble_opcode(uint16 addr) {
  auto read = [&](uint16 addr) -> uint8 {
    return disassembler_read(addr);
  };

  auto relative = [&](unsigned length, int8 offset) -> uint16 {
    uint16 pc = addr + length;
    return pc + offset;
  };

  auto a = [&] { return hex<4>((read(addr + 1) << 0) + (read(addr + 2) << 8)); };
  auto b = [&](unsigned n) { return hex<2>(read(addr + 1 + n)); };
  auto r = [&](unsigned r, unsigned n = 0) { return hex<4>(addr + r + (int8)read(addr + 1 + n)); };
  auto dp = [&](unsigned n) { return hex<3>((regs.p.p << 8) + read(addr + 1 + n)); };
  auto ab = [&] {
    unsigned n = (read(addr + 1) << 0) + (read(addr + 2) << 8);
    return string{ hex<4>(n & 0x1fff), ":", hex<1>(n >> 13) };
  };

  auto mnemonic = [&]() -> string {
    switch(read(addr)) {
    case 0x00: return { "nop" };
    case 0x01: return { "jst $ffde" };
    case 0x02: return { "set $", dp(0), ":0" };
    case 0x03: return { "bbs $", dp(0), ":0=$", r(+3, 1) };
    case 0x04: return { "ora $", dp(0) };
    case 0x05: return { "ora $", a() };
    case 0x06: return { "ora (x)" };
    case 0x07: return { "ora ($", dp(0), ",x)" };
    case 0x08: return { "ora #$", b(0) };
    case 0x09: return { "orr $", dp(1), "=$", dp(0) };
    case 0x0a: return { "orc $", ab() };
    case 0x0b: return { "asl $", dp(0) };
    case 0x0c: return { "asl $", a() };
    case 0x0d: return { "php" };
    case 0x0e: return { "tsb $", a() };
    case 0x0f: return { "brk" };
    case 0x10: return { "bpl $", r(+2) };
    case 0x11: return { "jst $ffdc" };
    case 0x12: return { "clr $", dp(0), ":0" };
    case 0x13: return { "bbc $", dp(0), ":0=$", r(+3, 1) };
    case 0x14: return { "ora $", dp(0), ",x" };
    case 0x15: return { "ora $", a(), ",x" };
    case 0x16: return { "ora $", a(), ",y" };
    case 0x17: return { "ora ($", dp(0), "),y" };
    case 0x18: return { "orr $", dp(1), "=#$", b(0) };
    case 0x19: return { "orr (x)=(y)" };
    case 0x1a: return { "dew $", dp(0) };
    case 0x1b: return { "asl $", dp(0), ",x" };
    case 0x1c: return { "asl" };
    case 0x1d: return { "dex" };
    case 0x1e: return { "cpx $", a() };
    case 0x1f: return { "jmp ($", a(), ",x)" };
    case 0x20: return { "clp" };
    case 0x21: return { "jst $ffda" };
    case 0x22: return { "set $", dp(0), ":1" };
    case 0x23: return { "bbs $", dp(0), ":1=$", r(+3, 1) };
    case 0x24: return { "and $", dp(0) };
    case 0x25: return { "and $", a() };
    case 0x26: return { "and (x)" };
    case 0x27: return { "and ($", dp(0), ",x)" };
    case 0x29: return { "and $", dp(1), "=$", dp(0) };
    case 0x2a: return { "orc !$", ab() };
    case 0x2b: return { "rol $", dp(0) };
    case 0x2c: return { "rol $", a() };
    case 0x2d: return { "pha" };
    case 0x2e: return { "bne $", dp(0), "=$", r(+3, 1) };
    case 0x28: return { "and #$", b(0) };
    case 0x2f: return { "bra $", r(+2) };
    case 0x30: return { "bmi $", r(+2) };
    case 0x31: return { "jst $ffd8" };
    case 0x32: return { "clr $", dp(0), ":1" };
    case 0x33: return { "bbc $", dp(0), ":1=$", r(+3, 1) };
    case 0x34: return { "and $", dp(0), ",x" };
    case 0x35: return { "and $", a(), ",x" };
    case 0x36: return { "and $", a(), ",y" };
    case 0x37: return { "and ($", dp(0), "),y" };
    case 0x38: return { "and $", dp(1), "=#$", b(0) };
    case 0x39: return { "and (x)=(y)" };
    case 0x3a: return { "inw $", dp(0) };
    case 0x3b: return { "rol $", dp(0), ",x" };
    case 0x3c: return { "rol" };
    case 0x3d: return { "inx" };
    case 0x3e: return { "cpx $", dp(0) };
    case 0x3f: return { "jsr $", a() };
    case 0x40: return { "sep" };
    case 0x41: return { "jst $ffd6" };
    case 0x42: return { "set $", dp(0), ":2" };
    case 0x43: return { "bbs $", dp(0), ":2=$", r(+3, 1) };
    case 0x44: return { "eor $", dp(0) };
    case 0x45: return { "eor $", a() };
    case 0x46: return { "eor (x)" };
    case 0x47: return { "eor ($", dp(0), ",x)" };
    case 0x48: return { "eor #$", b(0) };
    case 0x49: return { "eor $", dp(1), "=$", dp(0) };
    case 0x4a: return { "and $", ab() };
    case 0x4b: return { "lsr $", dp(0) };
    case 0x4c: return { "lsr $", a() };
    case 0x4d: return { "phx" };
    case 0x4e: return { "trb $", a() };
    case 0x4f: return { "jsp $ff", b(0) };
    case 0x50: return { "bvc $", r(+2) };
    case 0x51: return { "jst $ffd4" };
    case 0x52: return { "clr $", dp(0), ":2" };
    case 0x53: return { "bbc $", dp(0), ":2=$", r(+3, 1) };
    case 0x54: return { "eor $", dp(0), ",x" };
    case 0x55: return { "eor $", a(), ",x" };
    case 0x56: return { "eor $", a(), ",y" };
    case 0x57: return { "eor ($", dp(0), "),y" };
    case 0x58: return { "eor $", dp(1), "=#$", b(0) };
    case 0x59: return { "eor (x)=(y)" };
    case 0x5a: return { "cpw $", a() };
    case 0x5b: return { "lsr $", dp(0), ",x" };
    case 0x5c: return { "lsr" };
    case 0x5d: return { "tax" };
    case 0x5e: return { "cpy $", a() };
    case 0x5f: return { "jmp $", a() };
    case 0x60: return { "clc" };
    case 0x61: return { "jst $ffd2" };
    case 0x62: return { "set $", dp(0), ":3" };
    case 0x63: return { "bbs $", dp(0), ":3=$", r(+3, 1) };
    case 0x64: return { "cmp $", dp(0) };
    case 0x65: return { "cmp $", a() };
    case 0x66: return { "cmp (x)" };
    case 0x67: return { "cmp ($", dp(0), ",x)" };
    case 0x68: return { "cmp #$", b(0) };
    case 0x69: return { "cmp $", dp(1), "=$", dp(0) };
    case 0x6a: return { "and !$", ab() };
    case 0x6b: return { "ror $", dp(0) };
    case 0x6c: return { "ror $", a() };
    case 0x6d: return { "phy" };
    case 0x6e: return { "bne --$", dp(0), "=$", r(+3, 1) };
    case 0x6f: return { "rts" };
    case 0x70: return { "bvs $", r(+2) };
    case 0x71: return { "jst $ffd0" };
    case 0x72: return { "clr $", dp(0), ":3" };
    case 0x73: return { "bbc $", dp(0), ":3=$", r(+3, 1) };
    case 0x74: return { "cmp $", dp(0), ",x" };
    case 0x75: return { "cmp $", a(), ",x" };
    case 0x76: return { "cmp $", a(), ",y" };
    case 0x77: return { "cmp ($", dp(0), "),y" };
    case 0x78: return { "cmp $", dp(1), "=#$", b(0) };
    case 0x79: return { "cmp (x)=(y)" };
    case 0x7a: return { "adw $", a() };
    case 0x7b: return { "ror $", dp(0), ",x" };
    case 0x7c: return { "ror" };
    case 0x7d: return { "txa" };
    case 0x7e: return { "cpy $", dp(0) };
    case 0x7f: return { "rti" };
    case 0x80: return { "sec" };
    case 0x81: return { "jst $ffce" };
    case 0x82: return { "set $", dp(0), ":4" };
    case 0x83: return { "bbs $", dp(0), ":4=$", r(+3, 1) };
    case 0x84: return { "adc $", dp(0) };
    case 0x85: return { "adc $", a() };
    case 0x86: return { "adc (x)" };
    case 0x87: return { "adc ($", dp(0), ",x)" };
    case 0x88: return { "adc #$", b(0) };
    case 0x89: return { "adc $", dp(1), "=$", dp(0) };
    case 0x8a: return { "eor $", ab() };
    case 0x8b: return { "dec $", dp(0) };
    case 0x8c: return { "dec $", a() };
    case 0x8d: return { "ldy #$", b(0) };
    case 0x8e: return { "plp" };
    case 0x8f: return { "str $", dp(1), "=#$", b(0) };
    case 0x90: return { "bcc $", r(+2) };
    case 0x91: return { "jst $ffcc" };
    case 0x92: return { "clr $", dp(0), ":4" };
    case 0x93: return { "bbc $", dp(0), ":4=$", r(+3, 1) };
    case 0x94: return { "adc $", dp(0), ",x" };
    case 0x95: return { "adc $", a(), ",x" };
    case 0x96: return { "adc $", a(), ",y" };
    case 0x97: return { "adc ($", dp(0), "),y" };
    case 0x98: return { "adc $", dp(1), "=#$", b(0) };
    case 0x99: return { "adc (x)=(y)" };
    case 0x9a: return { "sbw $", a() };
    case 0x9b: return { "dec $", dp(0), ",x" };
    case 0x9c: return { "dec" };
    case 0x9d: return { "tsx" };
    case 0x9e: return { "div" };
    case 0x9f: return { "xcn" };
    case 0xa0: return { "sei" };
    case 0xa1: return { "jst $ffca" };
    case 0xa2: return { "set $", dp(0), ":5" };
    case 0xa3: return { "bbs $", dp(0), ":5=$", r(+3, 1) };
    case 0xa4: return { "sbc $", dp(0) };
    case 0xa5: return { "sbc $", a() };
    case 0xa6: return { "sbc (x)" };
    case 0xa7: return { "sbc ($", dp(0), ",x)" };
    case 0xa8: return { "sbc #$", b(0) };
    case 0xa9: return { "sbc $", dp(1), "=$", dp(0) };
    case 0xaa: return { "ldc $", ab() };
    case 0xab: return { "inc $", dp(0) };
    case 0xac: return { "inc $", a() };
    case 0xad: return { "cpy #$", b(0) };
    case 0xae: return { "pla" };
    case 0xaf: return { "sta (x++)" };
    case 0xb0: return { "bcs $", r(+2) };
    case 0xb1: return { "jst $ffc8" };
    case 0xb2: return { "clr $", dp(0), ":5" };
    case 0xb3: return { "bbc $", dp(0), ":5=$", r(+3, 1) };
    case 0xb4: return { "sbc $", dp(0), ",x" };
    case 0xb5: return { "sbc $", a(), ",x" };
    case 0xb6: return { "sbc $", a(), ",y" };
    case 0xb7: return { "sbc ($", dp(0), "),y" };
    case 0xb8: return { "sbc $", dp(1), "=#$", b(0) };
    case 0xb9: return { "sbc (x)=(y)" };
    case 0xba: return { "ldw $", dp(0) };
    case 0xbb: return { "inc $", dp(0), ",x" };
    case 0xbc: return { "inc" };
    case 0xbd: return { "txs" };
    case 0xbe: return { "das" };
    case 0xbf: return { "lda (x++)" };
    case 0xc0: return { "cli" };
    case 0xc1: return { "jst $ffc6" };
    case 0xc2: return { "set $", dp(0), ":6" };
    case 0xc3: return { "bbs $", dp(0), ":6=$", r(+3, 1) };
    case 0xc4: return { "sta $", dp(0) };
    case 0xc5: return { "sta $", a() };
    case 0xc6: return { "sta (x)" };
    case 0xc7: return { "sta ($", dp(0), ",x)" };
    case 0xc8: return { "cpx #$", b(0) };
    case 0xc9: return { "stx $", a() };
    case 0xca: return { "stc $", ab() };
    case 0xcb: return { "sty $", dp(0) };
    case 0xcc: return { "sty $", a() };
    case 0xcd: return { "ldx #$", b(0) };
    case 0xce: return { "plx" };
    case 0xcf: return { "mul" };
    case 0xd0: return { "bne $", r(+2) };
    case 0xd1: return { "jst $ffc4" };
    case 0xd2: return { "clr $", dp(0), ":6" };
    case 0xd3: return { "bbc $", dp(0), ":6=$", r(+3, 1) };
    case 0xd4: return { "sta $", dp(0), ",x" };
    case 0xd5: return { "sta $", a(), ",x" };
    case 0xd6: return { "sta $", a(), ",y" };
    case 0xd7: return { "sta ($", dp(0), "),y" };
    case 0xd8: return { "stx $", dp(0) };
    case 0xd9: return { "stx $", dp(0), ",y" };
    case 0xda: return { "stw $", dp(0) };
    case 0xdb: return { "sty $", dp(0), ",x" };
    case 0xdc: return { "dey" };
    case 0xdd: return { "tya" };
    case 0xde: return { "bne $", dp(0), ",x=$", r(+3, 1) };
    case 0xdf: return { "daa" };
    case 0xe0: return { "clv" };
    case 0xe1: return { "jst $ffc2" };
    case 0xe2: return { "set $", dp(0), ":7" };
    case 0xe3: return { "bbs $", dp(0), ":7=$", r(+3, 1) };
    case 0xe4: return { "lda $", dp(0) };
    case 0xe5: return { "lda $", a() };
    case 0xe6: return { "lda (x)" };
    case 0xe7: return { "lda ($", dp(0), ",x)" };
    case 0xe8: return { "lda #$", b(0) };
    case 0xe9: return { "ldx $", a() };
    case 0xea: return { "not $", ab() };
    case 0xeb: return { "ldy $", dp(0) };
    case 0xec: return { "ldy $", a() };
    case 0xed: return { "cmc" };
    case 0xee: return { "ply" };
    case 0xef: return { "wai" };
    case 0xf0: return { "beq $", r(+2) };
    case 0xf1: return { "jst $ffc0" };
    case 0xf2: return { "clr $", dp(0), ":7" };
    case 0xf3: return { "bbc $", dp(0), ":7=$", r(+3, 1) };
    case 0xf4: return { "lda $", dp(0), ",x" };
    case 0xf5: return { "lda $", a(), ",x" };
    case 0xf6: return { "lda $", a(), ",y" };
    case 0xf7: return { "lda ($", dp(0), "),y" };
    case 0xf8: return { "ldx $", dp(0) };
    case 0xf9: return { "ldx $", dp(0), ",y" };
    case 0xfa: return { "str $", dp(1), "=$", dp(0) };
    case 0xfb: return { "ldy $", dp(0), ",x" };
    case 0xfc: return { "iny" };
    case 0xfd: return { "tay" };
    case 0xfe: return { "bne --y=$", r(+2) };
    case 0xff: return { "stp" };
    }
    throw;
  };

  string output = { "..", hex<4>(addr), " ", mnemonic() };

  unsigned length = output.length();
  while(length++ < 30) output.append(" ");

  output.append(
    "YA:", hex<4>(regs.ya),
    " A:", hex<2>(regs.a),
    " X:", hex<2>(regs.x),
    " Y:", hex<2>(regs.y),
    " S:", hex<2>(regs.s),
    " ",
    regs.p.n ? "N" : "n",
    regs.p.v ? "V" : "v",
    regs.p.p ? "P" : "p",
    regs.p.b ? "B" : "b",
    regs.p.h ? "H" : "h",
    regs.p.i ? "I" : "i",
    regs.p.z ? "Z" : "z",
    regs.p.c ? "C" : "c"
  );

  return output;
}
