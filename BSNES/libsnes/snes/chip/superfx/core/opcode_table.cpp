#ifdef SUPERFX_CPP

void SuperFX::initialize_opcode_table() {
  #define op4(id, name) \
    op(id+ 0, name< 1>) op(id+ 1, name< 2>) op(id+ 2, name< 3>) op(id+ 3, name< 4>)

  #define op6(id, name) \
    op(id+ 0, name< 8>) op(id+ 1, name< 9>) op(id+ 2, name<10>) op(id+ 3, name<11>) \
    op(id+ 4, name<12>) op(id+ 5, name<13>)

  #define op12(id, name) \
    op(id+ 0, name< 0>) op(id+ 1, name< 1>) op(id+ 2, name< 2>) op(id+ 3, name< 3>) \
    op(id+ 4, name< 4>) op(id+ 5, name< 5>) op(id+ 6, name< 6>) op(id+ 7, name< 7>) \
    op(id+ 8, name< 8>) op(id+ 9, name< 9>) op(id+10, name<10>) op(id+11, name<11>)

  #define op15l(id, name) \
    op(id+ 0, name< 0>) op(id+ 1, name< 1>) op(id+ 2, name< 2>) op(id+ 3, name< 3>) \
    op(id+ 4, name< 4>) op(id+ 5, name< 5>) op(id+ 6, name< 6>) op(id+ 7, name< 7>) \
    op(id+ 8, name< 8>) op(id+ 9, name< 9>) op(id+10, name<10>) op(id+11, name<11>) \
    op(id+12, name<12>) op(id+13, name<13>) op(id+14, name<14>)

  #define op15h(id, name) \
    op(id+ 0, name< 1>) op(id+ 1, name< 2>) op(id+ 2, name< 3>) op(id+ 3, name< 4>) \
    op(id+ 4, name< 5>) op(id+ 5, name< 6>) op(id+ 6, name< 7>) op(id+ 7, name< 8>) \
    op(id+ 8, name< 9>) op(id+ 9, name<10>) op(id+10, name<11>) op(id+11, name<12>) \
    op(id+12, name<13>) op(id+13, name<14>) op(id+14, name<15>)

  #define op16(id, name) \
    op(id+ 0, name< 0>) op(id+ 1, name< 1>) op(id+ 2, name< 2>) op(id+ 3, name< 3>) \
    op(id+ 4, name< 4>) op(id+ 5, name< 5>) op(id+ 6, name< 6>) op(id+ 7, name< 7>) \
    op(id+ 8, name< 8>) op(id+ 9, name< 9>) op(id+10, name<10>) op(id+11, name<11>) \
    op(id+12, name<12>) op(id+13, name<13>) op(id+14, name<14>) op(id+15, name<15>)

  //======
  // ALT0
  //======

  #define op(id, name) opcode_table[  0 + id] = &SuperFX::op_##name;
  op   (0x00, stop)
  op   (0x01, nop)
  op   (0x02, cache)
  op   (0x03, lsr)
  op   (0x04, rol)
  op   (0x05, bra)
  op   (0x06, blt)
  op   (0x07, bge)
  op   (0x08, bne)
  op   (0x09, beq)
  op   (0x0a, bpl)
  op   (0x0b, bmi)
  op   (0x0c, bcc)
  op   (0x0d, bcs)
  op   (0x0e, bvc)
  op   (0x0f, bvs)
  op16 (0x10, to_r)
  op16 (0x20, with_r)
  op12 (0x30, stw_ir)
  op   (0x3c, loop)
  op   (0x3d, alt1)
  op   (0x3e, alt2)
  op   (0x3f, alt3)
  op12 (0x40, ldw_ir)
  op   (0x4c, plot)
  op   (0x4d, swap)
  op   (0x4e, color)
  op   (0x4f, not)
  op16 (0x50, add_r)
  op16 (0x60, sub_r)
  op   (0x70, merge)
  op15h(0x71, and_r)
  op16 (0x80, mult_r)
  op   (0x90, sbk)
  op4  (0x91, link)
  op   (0x95, sex)
  op   (0x96, asr)
  op   (0x97, ror)
  op6  (0x98, jmp_r)
  op   (0x9e, lob)
  op   (0x9f, fmult)
  op16 (0xa0, ibt_r)
  op16 (0xb0, from_r)
  op   (0xc0, hib)
  op15h(0xc1, or_r)
  op15l(0xd0, inc_r)
  op   (0xdf, getc)
  op15l(0xe0, dec_r)
  op   (0xef, getb)
  op16 (0xf0, iwt_r)
  #undef op

  //======
  // ALT1
  //======

  #define op(id, name) opcode_table[256 + id] = &SuperFX::op_##name;
  op   (0x00, stop)
  op   (0x01, nop)
  op   (0x02, cache)
  op   (0x03, lsr)
  op   (0x04, rol)
  op   (0x05, bra)
  op   (0x06, blt)
  op   (0x07, bge)
  op   (0x08, bne)
  op   (0x09, beq)
  op   (0x0a, bpl)
  op   (0x0b, bmi)
  op   (0x0c, bcc)
  op   (0x0d, bcs)
  op   (0x0e, bvc)
  op   (0x0f, bvs)
  op16 (0x10, to_r)
  op16 (0x20, with_r)
  op12 (0x30, stb_ir)
  op   (0x3c, loop)
  op   (0x3d, alt1)
  op   (0x3e, alt2)
  op   (0x3f, alt3)
  op12 (0x40, ldb_ir)
  op   (0x4c, rpix)
  op   (0x4d, swap)
  op   (0x4e, cmode)
  op   (0x4f, not)
  op16 (0x50, adc_r)
  op16 (0x60, sbc_r)
  op   (0x70, merge)
  op15h(0x71, bic_r)
  op16 (0x80, umult_r)
  op   (0x90, sbk)
  op4  (0x91, link)
  op   (0x95, sex)
  op   (0x96, div2)
  op   (0x97, ror)
  op6  (0x98, ljmp_r)
  op   (0x9e, lob)
  op   (0x9f, lmult)
  op16 (0xa0, lms_r)
  op16 (0xb0, from_r)
  op   (0xc0, hib)
  op15h(0xc1, xor_r)
  op15l(0xd0, inc_r)
  op   (0xdf, getc)
  op15l(0xe0, dec_r)
  op   (0xef, getbh)
  op16 (0xf0, lm_r)
  #undef op

  //======
  // ALT2
  //======

  #define op(id, name) opcode_table[512 + id] = &SuperFX::op_##name;
  op   (0x00, stop)
  op   (0x01, nop)
  op   (0x02, cache)
  op   (0x03, lsr)
  op   (0x04, rol)
  op   (0x05, bra)
  op   (0x06, blt)
  op   (0x07, bge)
  op   (0x08, bne)
  op   (0x09, beq)
  op   (0x0a, bpl)
  op   (0x0b, bmi)
  op   (0x0c, bcc)
  op   (0x0d, bcs)
  op   (0x0e, bvc)
  op   (0x0f, bvs)
  op16 (0x10, to_r)
  op16 (0x20, with_r)
  op12 (0x30, stw_ir)
  op   (0x3c, loop)
  op   (0x3d, alt1)
  op   (0x3e, alt2)
  op   (0x3f, alt3)
  op12 (0x40, ldw_ir)
  op   (0x4c, plot)
  op   (0x4d, swap)
  op   (0x4e, color)
  op   (0x4f, not)
  op16 (0x50, add_i)
  op16 (0x60, sub_i)
  op   (0x70, merge)
  op15h(0x71, and_i)
  op16 (0x80, mult_i)
  op   (0x90, sbk)
  op4  (0x91, link)
  op   (0x95, sex)
  op   (0x96, asr)
  op   (0x97, ror)
  op6  (0x98, jmp_r)
  op   (0x9e, lob)
  op   (0x9f, fmult)
  op16 (0xa0, sms_r)
  op16 (0xb0, from_r)
  op   (0xc0, hib)
  op15h(0xc1, or_i)
  op15l(0xd0, inc_r)
  op   (0xdf, ramb)
  op15l(0xe0, dec_r)
  op   (0xef, getbl)
  op16 (0xf0, sm_r)
  #undef op

  //======
  // ALT3
  //======

  #define op(id, name) opcode_table[768 + id] = &SuperFX::op_##name;
  op   (0x00, stop)
  op   (0x01, nop)
  op   (0x02, cache)
  op   (0x03, lsr)
  op   (0x04, rol)
  op   (0x05, bra)
  op   (0x06, blt)
  op   (0x07, bge)
  op   (0x08, bne)
  op   (0x09, beq)
  op   (0x0a, bpl)
  op   (0x0b, bmi)
  op   (0x0c, bcc)
  op   (0x0d, bcs)
  op   (0x0e, bvc)
  op   (0x0f, bvs)
  op16 (0x10, to_r)
  op16 (0x20, with_r)
  op12 (0x30, stb_ir)
  op   (0x3c, loop)
  op   (0x3d, alt1)
  op   (0x3e, alt2)
  op   (0x3f, alt3)
  op12 (0x40, ldb_ir)
  op   (0x4c, rpix)
  op   (0x4d, swap)
  op   (0x4e, cmode)
  op   (0x4f, not)
  op16 (0x50, adc_i)
  op16 (0x60, cmp_r)
  op   (0x70, merge)
  op15h(0x71, bic_i)
  op16 (0x80, umult_i)
  op   (0x90, sbk)
  op4  (0x91, link)
  op   (0x95, sex)
  op   (0x96, div2)
  op   (0x97, ror)
  op6  (0x98, ljmp_r)
  op   (0x9e, lob)
  op   (0x9f, lmult)
  op16 (0xa0, lms_r)
  op16 (0xb0, from_r)
  op   (0xc0, hib)
  op15h(0xc1, xor_i)
  op15l(0xd0, inc_r)
  op   (0xdf, romb)
  op15l(0xe0, dec_r)
  op   (0xef, getbs)
  op16 (0xf0, lm_r)
  #undef op

  #undef op4
  #undef op6
  #undef op12
  #undef op15l
  #undef op15h
  #undef op16
}

#endif
