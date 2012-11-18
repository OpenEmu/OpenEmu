string R6502::disassemble() {
  string output = { hex<4>(regs.pc), "  " };

  auto abs = [&]() -> string { return { "$", hex<2>(debugger_read(regs.pc + 2)), hex<2>(debugger_read(regs.pc + 1)) }; };
  auto abx = [&]() -> string { return { "$", hex<2>(debugger_read(regs.pc + 2)), hex<2>(debugger_read(regs.pc + 1)), ",x" }; };
  auto aby = [&]() -> string { return { "$", hex<2>(debugger_read(regs.pc + 2)), hex<2>(debugger_read(regs.pc + 1)), ",y" }; };
  auto iab = [&]() -> string { return { "($", hex<2>(debugger_read(regs.pc + 2)), hex<2>(debugger_read(regs.pc + 1)), ")" }; };
  auto imm = [&]() -> string { return { "#$", hex<2>(debugger_read(regs.pc + 1)) }; };
  auto imp = [&]() -> string { return ""; };
  auto izx = [&]() -> string { return { "($", hex<2>(debugger_read(regs.pc + 1)), ",x)" }; };
  auto izy = [&]() -> string { return { "($", hex<2>(debugger_read(regs.pc + 1)), "),y" }; };
  auto rel = [&]() -> string { return { "$", hex<4>((regs.pc + 2) + (int8)debugger_read(regs.pc + 1)) }; };
  auto zpg = [&]() -> string { return { "$", hex<2>(debugger_read(regs.pc + 1)) }; };
  auto zpx = [&]() -> string { return { "$", hex<2>(debugger_read(regs.pc + 1)), ",x" }; };
  auto zpy = [&]() -> string { return { "$", hex<2>(debugger_read(regs.pc + 1)), ",y" }; };

  #define op(byte, prefix, mode) \
    case byte: output.append(#prefix, " ", mode()); \
    break

  uint8 opcode = debugger_read(regs.pc);
  switch(opcode) {
    op(0x00, brk, imm);
    op(0x01, ora, izx);
    op(0x05, ora, zpg);
    op(0x06, asl, zpg);
    op(0x08, php, imp);
    op(0x09, ora, imm);
    op(0x0a, asl, imp);
    op(0x0d, ora, abs);
    op(0x0e, asl, abs);
    op(0x10, bpl, rel);
    op(0x11, ora, izy);
    op(0x15, ora, zpx);
    op(0x16, asl, zpx);
    op(0x18, clc, imp);
    op(0x19, ora, aby);
    op(0x1d, ora, abx);
    op(0x1e, asl, abx);
    op(0x20, jsr, abs);
    op(0x21, and, izx);
    op(0x24, bit, zpg);
    op(0x25, and, zpg);
    op(0x26, rol, zpg);
    op(0x28, plp, imp);
    op(0x29, and, imm);
    op(0x2a, rol, imp);
    op(0x2c, bit, abs);
    op(0x2d, and, abs);
    op(0x2e, rol, abs);
    op(0x30, bmi, rel);
    op(0x31, and, izy);
    op(0x35, and, zpx);
    op(0x36, rol, zpx);
    op(0x38, sec, imp);
    op(0x39, and, aby);
    op(0x3d, and, abx);
    op(0x3e, rol, abx);
    op(0x40, rti, imp);
    op(0x41, eor, izx);
    op(0x45, eor, zpg);
    op(0x46, lsr, zpg);
    op(0x48, pha, imp);
    op(0x49, eor, imm);
    op(0x4a, lsr, imp);
    op(0x4c, jmp, abs);
    op(0x4d, eor, abs);
    op(0x4e, lsr, abs);
    op(0x50, bvc, rel);
    op(0x51, eor, izy);
    op(0x55, eor, zpx);
    op(0x56, lsr, zpx);
    op(0x58, cli, imp);
    op(0x59, eor, aby);
    op(0x5a, phy, imp);
    op(0x5d, eor, abx);
    op(0x5e, lsr, abx);
    op(0x60, rts, imp);
    op(0x61, adc, izx);
    op(0x65, adc, zpg);
    op(0x66, ror, zpg);
    op(0x68, pla, imp);
    op(0x69, adc, imm);
    op(0x6a, ror, imp);
    op(0x6c, jmp, iab);
    op(0x6d, adc, abs);
    op(0x6e, ror, abs);
    op(0x70, bvs, rel);
    op(0x71, adc, izy);
    op(0x75, adc, zpx);
    op(0x76, ror, zpx);
    op(0x78, sei, imp);
    op(0x79, adc, aby);
    op(0x7a, ply, imp);
    op(0x7d, adc, abx);
    op(0x7e, ror, abx);
    op(0x81, sta, izx);
    op(0x84, sty, zpg);
    op(0x85, sta, zpg);
    op(0x86, stx, zpg);
    op(0x88, dey, imp);
    op(0x8a, txa, imp);
    op(0x8c, sty, abs);
    op(0x8d, sta, abs);
    op(0x8e, stx, abs);
    op(0x90, bcc, rel);
    op(0x91, sta, izy);
    op(0x94, sty, zpx);
    op(0x95, sta, zpx);
    op(0x96, stx, zpy);
    op(0x98, tya, imp);
    op(0x99, sta, aby);
    op(0x9a, txs, imp);
    op(0x9d, sta, abx);
    op(0xa0, ldy, imm);
    op(0xa1, lda, izx);
    op(0xa2, ldx, imm);
    op(0xa4, ldy, zpg);
    op(0xa5, lda, zpg);
    op(0xa6, ldx, zpg);
    op(0xa8, tay, imp);
    op(0xa9, lda, imm);
    op(0xaa, tax, imp);
    op(0xac, ldy, abs);
    op(0xad, lda, abs);
    op(0xae, ldx, abs);
    op(0xb0, bcs, rel);
    op(0xb1, lda, izy);
    op(0xb4, ldy, zpx);
    op(0xb5, lda, zpx);
    op(0xb6, ldx, zpy);
    op(0xb8, clv, imp);
    op(0xb9, lda, aby);
    op(0xba, tsx, imp);
    op(0xbc, ldy, abx);
    op(0xbd, lda, abx);
    op(0xbe, ldx, aby);
    op(0xc0, cpy, imm);
    op(0xc1, cmp, izx);
    op(0xc4, cpy, zpg);
    op(0xc5, cmp, zpg);
    op(0xc6, dec, zpg);
    op(0xc8, iny, imp);
    op(0xc9, cmp, imm);
    op(0xca, dex, imp);
    op(0xcc, cpy, abs);
    op(0xcd, cmp, abs);
    op(0xce, dec, abs);
    op(0xd0, bne, rel);
    op(0xd1, cmp, izy);
    op(0xd5, cmp, zpx);
    op(0xd6, dec, zpx);
    op(0xd8, cld, imp);
    op(0xd9, cmp, aby);
    op(0xda, phx, imp);
    op(0xdd, cmp, abx);
    op(0xde, dec, abx);
    op(0xe0, cpx, imm);
    op(0xe1, sbc, izx);
    op(0xe4, cpx, zpg);
    op(0xe5, sbc, zpg);
    op(0xe6, inc, zpg);
    op(0xe8, inx, imp);
    op(0xe9, sbc, imm);
    op(0xea, nop, imp);
    op(0xec, cpx, abs);
    op(0xed, sbc, abs);
    op(0xee, inc, abs);
    op(0xf0, beq, rel);
    op(0xf1, sbc, izy);
    op(0xf5, sbc, zpx);
    op(0xf6, inc, zpx);
    op(0xf8, sed, imp);
    op(0xf9, sbc, aby);
    op(0xfa, plx, imp);
    op(0xfd, sbc, abx);
    op(0xfe, inc, abx);

    default: output.append("$", hex<2>(opcode)); break;
  }

  #undef op

  output.append("                ");
  output[20] = 0;

  output.append(
    "A:", hex<2>(regs.a), " X:", hex<2>(regs.x), " Y:", hex<2>(regs.y), " S:", hex<2>(regs.s), " ",
    regs.p.n ? "N" : "n", regs.p.v ? "V" : "v", regs.p.d ? "D" : "d",
    regs.p.i ? "I" : "i", regs.p.z ? "Z" : "z", regs.p.c ? "C" : "c"
  );

  return output;
}
