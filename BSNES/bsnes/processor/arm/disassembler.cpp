#ifdef PROCESSOR_ARM_HPP

string ARM::disassemble_arm_instruction(uint32 pc) {
  static string conditions[] = {
    "eq", "ne", "cs", "cc",
    "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt",
    "gt", "le", "",   "nv",
  };

  static string opcodes[] = {
    "and", "eor", "sub", "rsb",
    "add", "adc", "sbc", "rsc",
    "tst", "teq", "cmp", "cmn",
    "orr", "mov", "bic", "mvn",
  };

  static string registers[] = {
    "r0",  "r1", "r2",  "r3",
    "r4",  "r5", "r6",  "r7",
    "r8",  "r9", "r10", "r11",
    "r12", "sp", "lr",  "pc",
  };

  static string indices[] = {
    "da", "ia", "db", "ib",
  };

  static auto is_move = [](uint4 opcode) { return opcode == 13 || opcode == 15; };
  static auto is_comp = [](uint4 opcode) { return opcode >= 8 && opcode <= 11; };
  static auto is_math = [](uint4 opcode) { return opcode < 8 || opcode == 12 || opcode == 14; };

  string output{hex<8>(pc), "  "};

  uint32 instruction = read(pc & ~3, Word);
  output.append(hex<8>(instruction), "  ");

  //multiply()
  //mul{condition}{s} rd,rm,rs
  //mla{condition}{s} rd,rm,rs,rn
  if((instruction & 0x0fc000f0) == 0x00000090) {
    uint4 condition = instruction >> 28;
    uint1 accumulate = instruction >> 21;
    uint1 save = instruction >> 20;
    uint4 rd = instruction >> 16;
    uint4 rn = instruction >> 12;
    uint4 rs = instruction >> 8;
    uint4 rm = instruction;

    output.append(accumulate ? "mla" : "mul", conditions[condition], save ? "s " : " ");
    output.append(registers[rd], ",", registers[rm], ",", registers[rs]);
    if(accumulate) output.append(",", registers[rn]);

    return output;
  }

  //multiply_long()
  //(u,s)mull{condition}{s} rdlo,rdhi,rm,rs
  //(u,s)mlal{condition}{s} rdlo,rdhi,rm,rs
  if((instruction & 0x0f8000f0) == 0x00800090) {
    uint4 condition = instruction >> 28;
    uint1 signextend = instruction >> 22;
    uint1 accumulate = instruction >> 21;
    uint1 save = instruction >> 20;
    uint4 rdhi = instruction >> 16;
    uint4 rdlo = instruction >> 12;
    uint4 rs = instruction >> 8;
    uint4 rm = instruction;

    output.append(signextend ? "s" : "u", accumulate ? "mlal" : "mull", conditions[condition], save ? "s " : " ");
    output.append(registers[rdlo], ",", registers[rdhi], ",", registers[rm], ",", registers[rs]);

    return output;
  }

  //memory_swap()
  //swp{condition}{b} rd,rm,[rn]
  if((instruction & 0x0fb000f0) == 0x01000090) {
    uint4 condition = instruction >> 28;
    uint1 byte = instruction >> 22;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint4 rm = instruction;

    output.append("swp", conditions[condition], byte ? "b " : " ");
    output.append(registers[rd], ",", registers[rm], "[", registers[rn], "]");

    return output;
  }

  //move_half_register()
  //(ldr,str){condition}h rd,[rn,rm]{!}
  //(ldr,str){condition}h rd,[rn],rm
  if((instruction & 0x0e4000f0) == 0x000000b0) {
    uint4 condition = instruction >> 28;
    uint1 pre = instruction >> 24;
    uint1 up = instruction >> 23;
    uint1 writeback = instruction >> 21;
    uint1 load = instruction >> 20;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint4 rm = instruction;

    output.append(load ? "ldr" : "str", conditions[condition], "h ");
    output.append(registers[rd], ",[", registers[rn]);
    if(pre == 0) output.append("]");
    output.append(",", up ? "+" : "-", registers[rm]);
    if(pre == 1) output.append("]");
    if(pre == 0 || writeback == 1) output.append("!");

    return output;
  }

  //move_half_immediate()
  //(ldr,str){condition}h rd,[rd{,+/-offset}]{!}
  //(ldr,str){condition}h rd,[rn]{,+/-offset}
  if((instruction & 0x0e4000f0) == 0x004000b0) {
    uint4 condition = instruction >> 28;
    uint1 pre = instruction >> 24;
    uint1 up = instruction >> 23;
    uint1 writeback = instruction >> 21;
    uint1 load = instruction >> 20;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint4 ih = instruction >> 8;
    uint4 il = instruction >> 0;

    uint8 immediate = (ih << 4) + (il << 0);

    output.append(load ? "ldr" : "str", conditions[condition], "h ");
    output.append(registers[rd], ",[", registers[rn]);
    if(pre == 0) output.append("]");
    if(immediate) output.append(",", up ? "+" : "-", "0x", hex<2>(immediate));
    if(pre == 1) output.append("]");
    if(pre == 0 || writeback == 1) output.append("!");

    if(rn == 15) output.append(" =0x", hex<4>(read(pc + 8 + (up ? +immediate : -immediate), Half)));
    return output;
  }

  //load_register()
  //ldr{condition}s(h,b) rd,[rn,rm]{!}
  //ldr{condition}s(h,b) rd,[rn],rm
  if((instruction & 0x0e5000d0) == 0x001000b0) {
    uint4 condition = instruction >> 28;
    uint1 pre = instruction >> 24;
    uint1 up = instruction >> 23;
    uint1 writeback = instruction >> 21;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint1 half = instruction >> 5;
    uint4 rm = instruction;

    output.append("ldr", conditions[condition], half ? "sh " : "sb ");
    output.append(registers[rd], ",[", registers[rn]);
    if(pre == 0) output.append("]");
    output.append(",", up ? "+" : "-", registers[rm]);
    if(pre == 1) output.append("]");
    if(pre == 0 || writeback == 1) output.append("!");

    return output;
  }

  //load_immediate()
  //ldr{condition}s(h,b) rd,[rn{,+/-offset}]{!}
  //ldr{condition}s(h,b) rd,[rn]{,+/-offset}
  if((instruction & 0x0e5000b0) == 0x005000b0) {
    uint4 condition = instruction >> 28;
    uint1 pre = instruction >> 24;
    uint1 up = instruction >> 23;
    uint1 writeback = instruction >> 21;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint4 ih = instruction >> 8;
    uint1 half = instruction >> 5;
    uint4 il = instruction;

    uint8 immediate = (ih << 4) + (il << 0);

    output.append("ldr", conditions[condition], half ? "sh " : "sb ");
    output.append(registers[rd], ",[", registers[rn]);
    if(pre == 0) output.append("]");
    if(immediate) output.append(",", up ? "+" : "-", "0x", hex<2>(immediate));
    if(pre == 1) output.append("]");
    if(pre == 0 || writeback == 1) output.append("!");

    if(rn == 15 && half == 1) output.append(" =0x", hex<4>(read(pc + 8 + (up ? +immediate : -immediate), Half)));
    if(rn == 15 && half == 0) output.append(" =0x", hex<2>(read(pc + 8 + (up ? +immediate : -immediate), Byte)));
    return output;
  }

  //move_to_register_from_status()
  //mrs{condition} rd,(c,s)psr
  if((instruction & 0x0fb000f0) == 0x01000000) {
    uint4 condition = instruction >> 28;
    uint1 psr = instruction >> 22;
    uint4 rd = instruction >> 12;

    output.append("mrs", conditions[condition], " ");
    output.append(registers[rd], ",", psr ? "spsr" : "cpsr");

    return output;
  }

  //move_to_status_from_register()
  //msr{condition} (c,s)psr:{fields},rm
  if((instruction & 0x0fb000f0) == 0x01200000) {
    uint4 condition = instruction >> 28;
    uint1 psr = instruction >> 22;
    uint4 field = instruction >> 16;
    uint4 rm = instruction;

    output.append("msr", conditions[condition], " ");
    output.append(psr ? "spsr:" : "cpsr:");
    output.append(
      field & 1 ? "c" : "",
      field & 2 ? "x" : "",
      field & 4 ? "s" : "",
      field & 8 ? "f" : ""
    );
    output.append(",", registers[rm]);

    return output;
  }

  //branch_exchange_register()
  //bx{condition} rm
  if((instruction & 0x0ff000f0) == 0x01200010) {
    uint4 condition = instruction >> 28;
    uint4 rm = instruction;

    output.append("bx", conditions[condition], " ");
    output.append(registers[rm]);

    return output;
  }

  //move_to_status_from_immediate()
  //msr{condition} (c,s)psr:{fields},#immediate
  if((instruction & 0x0fb00000) == 0x03200000) {
    uint4 condition = instruction >> 28;
    uint1 psr = instruction >> 22;
    uint4 field = instruction >> 16;
    uint4 rotate = instruction >> 8;
    uint8 immediate = instruction;

    uint32 rm = (immediate >> (rotate * 2)) | (immediate << (32 - (rotate * 2)));

    output.append("msr", conditions[condition], " ");
    output.append(psr ? "spsr:" : "cpsr:");
    output.append(
      field & 1 ? "c" : "",
      field & 2 ? "x" : "",
      field & 4 ? "s" : "",
      field & 8 ? "f" : ""
    );
    output.append(",#0x", hex<8>(immediate));

    return output;
  }

  //data_immediate_shift()
  //{opcode}{condition}{s} rd,rm {shift} #immediate
  //{opcode}{condition} rn,rm {shift} #immediate
  //{opcode}{condition}{s} rd,rn,rm {shift} #immediate
  if((instruction & 0x0e000010) == 0x00000000) {
    uint4 condition = instruction >> 28;
    uint4 opcode = instruction >> 21;
    uint1 save = instruction >> 20;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint5 shift = instruction >> 7;
    uint2 op = instruction >> 5;
    uint4 rm = instruction;

    output.append(opcodes[opcode], conditions[condition]);
    if(is_move(opcode)) output.append(save ? "s " : " ", registers[rd]);
    if(is_comp(opcode)) output.append(" ", registers[rn]);
    if(is_math(opcode)) output.append(save ? "s " : " ", registers[rd], ",", registers[rn]);
    output.append(",", registers[rm]);
    if(op == 0 && shift != 0) output.append(" lsl #", shift);
    if(op == 1) output.append(" lsr #", shift == 0 ? 32u : (unsigned)shift);
    if(op == 2) output.append(" asr #", shift == 0 ? 32u : (unsigned)shift);
    if(op == 3 && shift != 0) output.append(" ror #", shift);
    if(op == 3 && shift == 0) output.append(" rrx");

    return output;
  }

  //data_register_shift()
  //{opcode}{condition}{s} rd,rm {shift} rs
  //{opcode}{condition} rn,rm {shift} rs
  //{opcode}{condition}{s} rd,rn,rm {shift} rs
  if((instruction & 0x0e000090) == 0x00000010) {
    uint4 condition = instruction >> 28;
    uint4 opcode = instruction >> 21;
    uint1 save = instruction >> 20;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint4 rs = instruction >> 8;
    uint2 mode = instruction >> 5;
    uint4 rm = instruction;

    output.append(opcodes[opcode], conditions[condition]);
    if(is_move(opcode)) output.append(save ? "s " : " ", registers[rd], ",");
    if(is_comp(opcode)) output.append(registers[rn], ",");
    if(is_math(opcode)) output.append(save ? "s " : " ", registers[rd], ",", registers[rn], ",");
    output.append(registers[rm]);
    if(mode == 0) output.append(" lsl ");
    if(mode == 1) output.append(" lsr ");
    if(mode == 2) output.append(" asr ");
    if(mode == 3) output.append(" ror ");
    output.append(registers[rs]);

    return output;
  }

  //data_immediate()
  //{opcode}{condition}{s} rd,#immediate
  //{opcode}{condition} rn,#immediate
  //{opcode}{condition}{s} rd,rn,#immediate
  if((instruction & 0x0e000000) == 0x02000000) {
    uint4 condition = instruction >> 28;
    uint4 opcode = instruction >> 21;
    uint1 save = instruction >> 20;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint4 rotate = instruction >> 8;
    uint8 immediate = instruction;

    uint32 rm = (immediate >> (rotate << 1)) | (immediate << (32 - (rotate << 1)));
    output.append(opcodes[opcode], conditions[condition]);
    if(is_move(opcode)) output.append(save ? "s " : " ", registers[rd]);
    if(is_comp(opcode)) output.append(" ", registers[rn]);
    if(is_math(opcode)) output.append(save ? "s " : " ", registers[rd], ",", registers[rn]);
    output.append(",#0x", hex<8>(rm));

    return output;
  }

  //move_immediate_offset()
  //(ldr,str){condition}{b} rd,[rn{,+/-offset}]{!}
  //(ldr,str){condition}{b} rd,[rn]{,+/-offset}
  if((instruction & 0x0e000000) == 0x04000000) {
    uint4 condition = instruction >> 28;
    uint1 pre = instruction >> 24;
    uint1 up = instruction >> 23;
    uint1 byte = instruction >> 22;
    uint1 writeback = instruction >> 21;
    uint1 load = instruction >> 20;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint12 immediate = instruction;

    output.append(load ? "ldr" : "str", conditions[condition], byte ? "b " : " ");
    output.append(registers[rd], ",[", registers[rn]);
    if(pre == 0) output.append("]");
    if(immediate) output.append(",", up ? "+" : "-", "0x", hex<3>(immediate));
    if(pre == 1) output.append("]");
    if(pre == 0 || writeback == 1) output.append("!");

    if(rn == 15) output.append(" =0x", hex<8>(read(pc + 8 + (up ? +immediate : -immediate), byte ? Byte : Word)));
    return output;
  }

  //move_register_offset()
  //(ldr,str){condition}{b} rd,[rn,rm {mode} #immediate]{1}
  //(ldr,str){condition}{b} rd,[rn],rm {mode} #immediate
  if((instruction & 0x0e000010) == 0x06000000) {
    uint4 condition = instruction >> 28;
    uint1 pre = instruction >> 24;
    uint1 up = instruction >> 23;
    uint1 byte = instruction >> 22;
    uint1 writeback = instruction >> 21;
    uint1 load = instruction >> 20;
    uint4 rn = instruction >> 16;
    uint4 rd = instruction >> 12;
    uint5 shift = instruction >> 7;
    uint2 mode = instruction >> 5;
    uint4 rm = instruction;

    output.append(load ? "ldr" : "str", conditions[condition], byte ? "b " : " ");
    output.append(registers[rd], ",[", registers[rn]);
    if(pre == 0) output.append("]");
    output.append(",", up ? "+" : "-", registers[rm]);
    if(mode == 0 && shift != 0) output.append(" lsl #", shift);
    if(mode == 1) output.append(" lsr #", shift == 0 ? 32u : (unsigned)shift);
    if(mode == 2) output.append(" asr #", shift == 0 ? 32u : (unsigned)shift);
    if(mode == 3 && shift != 0) output.append(" ror #", shift);
    if(mode == 3 && shift == 0) output.append(" rrx");
    if(pre == 1) output.append("]");
    if(pre == 0 || writeback == 1) output.append("!");

    return output;
  }

  //move_multiple()
  //(ldm,stm) {condition}{mode} rn{!},{r...}{^}
  if((instruction & 0x0e000000) == 0x08000000) {
    uint4 condition = instruction >> 28;
    uint2 index = instruction >> 23;
    uint1 s = instruction >> 22;
    uint1 writeback = instruction >> 21;
    uint1 load = instruction >> 20;
    uint4 rn = instruction >> 16;
    uint16 list = instruction;

    output.append(load ? "ldm" : "stm", conditions[condition], indices[index], " ");
    output.append(registers[rn], writeback ? "!" : "", ",{");
    for(unsigned n = 0; n < 16; n++) if(list & (1 << n)) output.append(registers[n], ",");
    output.rtrim<1>(",");
    output.append("}", s ? "^" : "");

    return output;
  }

  //branch()
  //b{l}{condition} address
  if((instruction & 0x0e000000) == 0x0a000000) {
    uint4 condition = instruction >> 28;
    uint1 link = instruction >> 24;

    output.append("b", link ? "l" : "", conditions[condition], " ");
    output.append("0x", hex<8>(pc + 8 + (int24)instruction * 4));

    return output;
  }

  //software_interrupt()
  //swi #immediate
  if((instruction & 0x0f000000) == 0x0f000000) {
    uint24 immediate = instruction;

    output.append("swi #0x", hex<6>(immediate));

    return output;
  }

  output.append("???");
  return output;
}

string ARM::disassemble_thumb_instruction(uint32 pc) {
  static string conditions[] = {
    "eq", "ne", "cs", "cc",
    "mi", "pl", "vs", "vc",
    "hi", "ls", "ge", "lt",
    "gt", "le", "", "",
  };

  static string registers[] = {
    "r0",  "r1", "r2",  "r3",
    "r4",  "r5", "r6",  "r7",
    "r8",  "r9", "r10", "r11",
    "r12", "sp", "lr",  "pc",
  };

  string output{hex<8>(pc), "  "};

  uint16 instruction = read(pc & ~1, Half);
  output.append(hex<4>(instruction), "  ");

  //adjust_register()
  //(add,sub) rd,rn,rm
  if((instruction & 0xfc00) == 0x1800) {
    uint1 opcode = instruction >> 9;
    uint3 rm = instruction >> 6;
    uint3 rn = instruction >> 3;
    uint3 rd = instruction >> 0;

    output.append(opcode == 0 ? "add" : "sub", " ", registers[rd], ",", registers[rn], ",", registers[rm]);

    return output;
  }

  //adjust_immediate()
  //(add,sub) rd,rn,#immediate
  if((instruction & 0xfc00) == 0x1c00) {
    uint1 opcode = instruction >> 9;
    uint3 immediate = instruction >> 6;
    uint3 rn = instruction >> 3;
    uint3 rd = instruction >> 0;

    output.append(opcode == 0 ? "add" : "sub", " ", registers[rd], ",", registers[rn], ",#", hex<1>(immediate));

    return output;
  }

  //shift_immediate()
  //(lsl,lsr,asar) rd,rm,#immmediate
  if((instruction & 0xe000) == 0x0000) {
    static string opcodes[] = { "lsl", "lsr", "asr", "" };

    uint2 opcode = instruction >> 11;
    uint5 immediate = instruction >> 6;
    uint3 rm = instruction >> 3;
    uint3 rd = instruction >> 0;

    output.append(opcodes[opcode], " ", registers[rd], ",", registers[rm], ",#", immediate);

    return output;
  }

  //immediate()
  //(mov,cmp,add,sub) (rd,rn),#immediate
  if((instruction & 0xe000) == 0x2000) {
    static string opcodes[] = { "mov", "cmp", "add", "sub" };

    uint2 opcode = instruction >> 11;
    uint3 rd = instruction >> 8;
    uint8 immediate = instruction;

    output.append(opcodes[opcode], " ", registers[rd], ",#0x", hex<2>(immediate));

    return output;
  }

  //alu()
  //{opcode} rd,rm
  if((instruction & 0xfc00) == 0x4000) {
    static string opcodes[] = {
      "and", "eor", "lsl", "lsr",
      "asr", "adc", "sbc", "ror",
      "tst", "neg", "cmp", "cmn",
      "orr", "mul", "bic", "mvn",
    };

    uint4 opcode = instruction >> 6;
    uint3 rm = instruction >> 3;
    uint3 rd = instruction >> 0;

    output.append(opcodes[opcode], " ", registers[rd], ",", registers[rm]);

    return output;
  }

  //branch_exchange()
  //bx rm
  if((instruction & 0xff80) == 0x4700) {
    uint4 rm = instruction >> 3;

    output.append("bx ", registers[rm]);

    return output;
  }

  //alu_hi()
  //{opcode} rd,rm
  if((instruction & 0xfc00) == 0x4400) {
    static string opcodes[] = { "add", "sub", "mov", "" };

    uint2 opcode = instruction >> 8;
    uint4 rm = instruction >> 3;
    uint4 rd = ((uint1)(instruction >> 7) << 3) + (uint3)instruction;

    if(opcode == 2 && rm == 8 && rd == 8) {
      output.append("nop");
      return output;
    }

    output.append(opcodes[opcode], " ", registers[rd], ",", registers[rm]);

    return output;
  }

  //load_literal()
  //ldr rd,[pc,#+/-offset]
  if((instruction & 0xf800) == 0x4800) {
    uint3 rd = instruction >> 8;
    uint8 displacement = instruction;

    unsigned rm = ((pc + 4) & ~3) + displacement * 4;
    output.append("ldr ", registers[rd], ",[pc,#0x", hex<3>(rm), "]");
    output.append(" =0x", hex<8>(read(rm, Word)));

    return output;
  }

  //move_register_offset()
  //(ld(r,s),str){b,h} rd,[rn,rm]
  if((instruction & 0xf000) == 0x5000) {
    static string opcodes[] = {
      "str", "strh", "strb", "ldsb",
      "ldr", "ldrh", "ldrb", "ldsh",
    };

    uint3 opcode = instruction >> 9;
    uint3 rm = instruction >> 6;
    uint3 rn = instruction >> 3;
    uint3 rd = instruction >> 0;

    output.append(opcodes[opcode], " ", registers[rd], ",[", registers[rn], ",", registers[rm], "]");

    return output;
  }

  //move_word_immediate()
  //(ldr,str) rd,[rn,#offset]
  if((instruction & 0xf000) == 0x6000) {
    uint1 load = instruction >> 11;
    uint5 offset = instruction >> 6;
    uint3 rn = instruction >> 3;
    uint3 rd = instruction >> 0;

    output.append(load ? "ldr " : "str ", registers[rd], ",[", registers[rn], ",#0x", hex<2>(offset * 4), "]");

    return output;
  }

  //move_byte_immediate()
  //(ldr,str)b rd,[rn,#offset]
  if((instruction & 0xf000) == 0x7000) {
    uint1 load = instruction >> 11;
    uint5 offset = instruction >> 6;
    uint3 rn = instruction >> 3;
    uint3 rd = instruction >> 0;

    output.append(load ? "ldrb " : "strb ", registers[rd], ",[", registers[rn], ",#0x", hex<2>(offset), "]");

    return output;
  }

  //move_half_immediate()
  //(ldr,str)h rd,[rn,#offset]
  if((instruction & 0xf000) == 0x8000) {
    uint1 load = instruction >> 11;
    uint5 offset = instruction >> 6;
    uint3 rn = instruction >> 3;
    uint3 rd = instruction >> 0;

    output.append(load ? "ldrh " : "strh ", registers[rd], ",[", registers[rn], ",#0x", hex<2>(offset * 2), "]");

    return output;
  }

  //move_stack()
  //(ldr,str) rd,[sp,#relative]
  if((instruction & 0xf000) == 0x9000) {
    uint1 opcode = instruction >> 11;
    uint3 rd = instruction >> 8;
    int8 relative = instruction;

    output.append(opcode ? "ldr" : "str", " ", registers[rd], ",[sp,#0x", hex<3>(relative * 4), "]");

    return output;
  }

  //add_register_hi()
  //add rd,{pc,sp},#immediate
  if((instruction & 0xf000) == 0xa000) {
    uint1 sp = instruction >> 11;
    uint3 rd = instruction >> 8;
    uint8 immediate = instruction;

    output.append("add ", registers[rd], ",", sp ? "sp" : "pc", ",#0x", hex<2>(immediate));

    return output;
  }

  //adjust_stack()
  //(add,sub) sp,#immediate
  if((instruction & 0xff00) == 0xb000) {
    uint1 opcode = instruction >> 7;
    uint7 immediate = instruction;

    output.append(opcode == 0 ? "add" : "sub", " sp,#0x", hex<3>(immediate * 4));

    return output;
  }

  //stack_multiple()
  //push {r...{,lr}}
  //pop {r...{,pc}}
  if((instruction & 0xf600) == 0xb400) {
    uint1 load = instruction >> 11;
    uint1 branch = instruction >> 8;
    uint8 list = instruction;

    output.append(load == 0 ? "push" : "pop", " {");
    for(unsigned l = 0; l < 8; l++) {
      if(list & (1 << l)) output.append(registers[l], ",");
    }
    if(branch) output.append(load == 0 ? "lr," : "pc,");
    output.rtrim<1>(",");
    output.append("}");

    return output;
  }

  //move_multiple()
  //(ldmia,stmia) rn,{r...}
  if((instruction & 0xf000) == 0xc000) {
    uint1 load = instruction >> 11;
    uint3 rn = instruction >> 8;
    uint8 list = instruction;

    output.append(load ? "ldmia " : "stmia ", registers[rn], "!,{");
    for(unsigned l = 0; l < 8; l++) {
      if(list & (1 << l)) output.append(registers[l], ",");
    }
    output.rtrim<1>(",");
    output.append("}");

    return output;
  }

  //software_interrupt()
  //swi #immediate
  if((instruction & 0xff00) == 0xdf00) {
    uint8 immediate = instruction;

    output.append("swi #0x", hex<2>(immediate));

    return output;
  }

  //branch_conditional()
  //b{condition} address
  if((instruction & 0xf000) == 0xd000) {
    uint4 condition = instruction >> 8;
    int8 displacement = instruction;

    uint32 offset = pc + 4 + displacement * 2;
    output.append("b", conditions[condition], " 0x", hex<8>(offset));

    return output;
  }

  //branch_short()
  //b address
  if((instruction & 0xf800) == 0xe000) {
    int11 displacement = instruction;

    output.append("b 0x", hex<8>(pc + 4 + displacement * 2));

    return output;
  }

  //branch_long_prefix()
  //bl address
  if((instruction & 0xf800) == 0xf000) {
    uint11 offsethi = instruction;
    instruction = read((pc & ~1) + 2, Half);
    uint11 offsetlo = instruction;

    int22 displacement = (offsethi << 11) | (offsetlo << 0);
    output.append("bl 0x", hex<8>(pc + 4 + displacement * 2));

    return output;
  }

  //branch_long_suffix()
  //bl address
  if((instruction & 0xf800) == 0xf800) {
    output.append("...");

    return output;
  }

  output.append("???");
  return output;
}

string ARM::disassemble_registers() {
  string output;
  output.append( "r0:", hex<8>(r( 0)), " r1:", hex<8>(r( 1)), "  r2:", hex<8>(r( 2)), "  r3:", hex<8>(r( 3)), "  ");
  output.append( "r4:", hex<8>(r( 4)), " r5:", hex<8>(r( 5)),  " r6:", hex<8>(r( 6)),  " r7:", hex<8>(r( 7)), " ");
  output.append("cpsr:", cpsr().n ? "N" : "n", cpsr().z ? "Z" : "z", cpsr().c ? "C" : "c", cpsr().v ? "V" : "v");
  output.append("/", cpsr().i ? "I" : "i", cpsr().f ? "F" : "f", cpsr().t ? "T" : "t");
  output.append("/", hex<2>(cpsr().m), "\n");
  output.append( "r8:", hex<8>(r( 8)), " r9:", hex<8>(r( 9)), " r10:", hex<8>(r(10)), " r11:", hex<8>(r(11)), " ");
  output.append("r12:", hex<8>(r(12)), " sp:", hex<8>(r(13)),  " lr:", hex<8>(r(14)),  " pc:", hex<8>(r(15)), " ");
  output.append("spsr:");
  if(mode() == Processor::Mode::USR || mode() == Processor::Mode::SYS) { output.append("----/---/--"); return output; }
  output.append(         spsr().n ? "N" : "n", spsr().z ? "Z" : "z", spsr().c ? "C" : "c", spsr().v ? "V" : "v");
  output.append("/", spsr().i ? "I" : "i", spsr().f ? "F" : "f", spsr().t ? "T" : "t");
  output.append("/", hex<2>(spsr().m));
  return output;
}

#endif
