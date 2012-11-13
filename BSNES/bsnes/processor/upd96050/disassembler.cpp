#ifdef NECDSP_CPP

string NECDSP::disassemble(uint14 ip) {
  string output = { hex<4>(ip), "  " };
  uint24 opcode = programROM[ip];
  uint2 type = opcode >> 22;

  if(type == 0 || type == 1) {  //OP,RT
    uint2 pselect = opcode >> 20;
    uint4 alu     = opcode >> 16;
    uint1 asl     = opcode >> 15;
    uint2 dpl     = opcode >> 13;
    uint4 dphm    = opcode >>  9;
    uint1 rpdcr   = opcode >>  8;
    uint4 src     = opcode >>  4;
    uint4 dst     = opcode >>  0;

    switch(alu) {
      case  0: output.append("nop     "); break;
      case  1: output.append("or      "); break;
      case  2: output.append("and     "); break;
      case  3: output.append("xor     "); break;
      case  4: output.append("sub     "); break;
      case  5: output.append("add     "); break;
      case  6: output.append("sbb     "); break;
      case  7: output.append("adc     "); break;
      case  8: output.append("dec     "); break;
      case  9: output.append("inc     "); break;
      case 10: output.append("cmp     "); break;
      case 11: output.append("shr1    "); break;
      case 12: output.append("shl1    "); break;
      case 13: output.append("shl2    "); break;
      case 14: output.append("shl4    "); break;
      case 15: output.append("xchg    "); break;
    }

    if(alu < 8) {
      switch(pselect) {
        case 0: output.append("ram,"); break;
        case 1: output.append("idb,"); break;
        case 2: output.append("m,"  ); break;
        case 3: output.append("n,"  ); break;
      }
    }

    switch(asl) {
      case 0: output.append("a"); break;
      case 1: output.append("b"); break;
    }

    output.append("\n      mov     ");

    switch(src) {
      case  0: output.append("trb," ); break;
      case  1: output.append("a,"   ); break;
      case  2: output.append("b,"   ); break;
      case  3: output.append("tr,"  ); break;
      case  4: output.append("dp,"  ); break;
      case  5: output.append("rp,"  ); break;
      case  6: output.append("ro,"  ); break;
      case  7: output.append("sgn," ); break;
      case  8: output.append("dr,"  ); break;
      case  9: output.append("drnf,"); break;
      case 10: output.append("sr,"  ); break;
      case 11: output.append("sim," ); break;
      case 12: output.append("sil," ); break;
      case 13: output.append("k,"   ); break;
      case 14: output.append("l,"   ); break;
      case 15: output.append("mem," ); break;
    }

    switch(dst) {
      case  0: output.append("non"); break;
      case  1: output.append("a"  ); break;
      case  2: output.append("b"  ); break;
      case  3: output.append("tr" ); break;
      case  4: output.append("dp" ); break;
      case  5: output.append("rp" ); break;
      case  6: output.append("dr" ); break;
      case  7: output.append("sr" ); break;
      case  8: output.append("sol"); break;
      case  9: output.append("som"); break;
      case 10: output.append("k"  ); break;
      case 11: output.append("klr"); break;
      case 12: output.append("klm"); break;
      case 13: output.append("l"  ); break;
      case 14: output.append("trb"); break;
      case 15: output.append("mem"); break;
    }

    if(dpl) {
      switch(dpl) {
        case 0: output.append("\n      dpnop"); break;
        case 1: output.append("\n      dpinc"); break;
        case 2: output.append("\n      dpdec"); break;
        case 3: output.append("\n      dpclr"); break;
      }
    }

    if(dphm) {
      output.append("\n      m", hex<1>(dphm));
    }

    if(rpdcr == 1) {
      output.append("\n      rpdec");
    }

    if(type == 1) {
      output.append("\n      ret");
    }
  }

  if(type == 2) {  //JP
    uint9 brch = opcode >> 13;
    uint11 na  = opcode >>  2;
    uint8 bank = opcode >>  0;

    uint14 jp = (regs.pc & 0x2000) | (bank << 11) | (na << 0);

    switch(brch) {
      case 0x000: output.append("jmpso   "); jp = 0; break;
      case 0x080: output.append("jnca    "); break;
      case 0x082: output.append("jca     "); break;
      case 0x084: output.append("jncb    "); break;
      case 0x086: output.append("jcb     "); break;
      case 0x088: output.append("jnza    "); break;
      case 0x08a: output.append("jza     "); break;
      case 0x08c: output.append("jnzb    "); break;
      case 0x08e: output.append("jzb     "); break;
      case 0x090: output.append("jnova0  "); break;
      case 0x092: output.append("jova0   "); break;
      case 0x094: output.append("jnovb0  "); break;
      case 0x096: output.append("jovb0   "); break;
      case 0x098: output.append("jnova1  "); break;
      case 0x09a: output.append("jova1   "); break;
      case 0x09c: output.append("jnovb1  "); break;
      case 0x09e: output.append("jovb1   "); break;
      case 0x0a0: output.append("jnsa0   "); break;
      case 0x0a2: output.append("jsa0    "); break;
      case 0x0a4: output.append("jnsb0   "); break;
      case 0x0a6: output.append("jsb0    "); break;
      case 0x0a8: output.append("jnsa1   "); break;
      case 0x0aa: output.append("jsa1    "); break;
      case 0x0ac: output.append("jnsb1   "); break;
      case 0x0ae: output.append("jsb1    "); break;
      case 0x0b0: output.append("jdpl0   "); break;
      case 0x0b1: output.append("jdpln0  "); break;
      case 0x0b2: output.append("jdplf   "); break;
      case 0x0b3: output.append("jdplnf  "); break;
      case 0x0b4: output.append("jnsiak  "); break;
      case 0x0b6: output.append("jsiak   "); break;
      case 0x0b8: output.append("jnsoak  "); break;
      case 0x0ba: output.append("jsoak   "); break;
      case 0x0bc: output.append("jnrqm   "); break;
      case 0x0be: output.append("jrqm    "); break;
      case 0x100: output.append("ljmp    "); jp &= ~0x2000; break;
      case 0x101: output.append("hjmp    "); jp |=  0x2000; break;
      case 0x140: output.append("lcall   "); jp &= ~0x2000; break;
      case 0x141: output.append("hcall   "); jp |=  0x2000; break;
      default:    output.append("??????  "); break;
    }

    output.append("$", hex<4>(jp));
  }

  if(type == 3) {  //LD
    output.append("ld      ");
    uint16 id = opcode >> 6;
    uint4 dst = opcode >> 0;

    output.append("$", hex<4>(id), ",");

    switch(dst) {
      case  0: output.append("non"); break;
      case  1: output.append("a"  ); break;
      case  2: output.append("b"  ); break;
      case  3: output.append("tr" ); break;
      case  4: output.append("dp" ); break;
      case  5: output.append("rp" ); break;
      case  6: output.append("dr" ); break;
      case  7: output.append("sr" ); break;
      case  8: output.append("sol"); break;
      case  9: output.append("som"); break;
      case 10: output.append("k"  ); break;
      case 11: output.append("klr"); break;
      case 12: output.append("klm"); break;
      case 13: output.append("l"  ); break;
      case 14: output.append("trb"); break;
      case 15: output.append("mem"); break;
    }
  }

  return output;
}

#endif
