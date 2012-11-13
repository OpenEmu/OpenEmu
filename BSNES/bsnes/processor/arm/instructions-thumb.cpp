#ifdef PROCESSOR_ARM_HPP

void ARM::thumb_step() {
  if(pipeline.reload) {
    pipeline.reload = false;
    r(15).data &= ~1;

    sequential() = false;
    pipeline.fetch.address = r(15) & ~1;
    pipeline.fetch.instruction = read(pipeline.fetch.address, Half);

    pipeline_step();
  }

  pipeline_step();

  if(processor.irqline && cpsr().i == 0) {
    vector(0x00000018, Processor::Mode::IRQ);
    r(14) += 2;
    return;
  }

  instructions++;
  if(trace) {
    print(disassemble_registers(), "\n");
    print(disassemble_thumb_instruction(pipeline.execute.address), "\n");
  }

  #define decode(pattern, execute) if( \
    (instruction() & std::integral_constant<uint32, bit::mask(pattern)>::value) \
    == std::integral_constant<uint32, bit::test(pattern)>::value \
  ) return thumb_op_ ## execute()

  decode("0001 10?? ???? ????", adjust_register);
  decode("0001 11?? ???? ????", adjust_immediate);
  decode("000? ???? ???? ????", shift_immediate);
  decode("001? ???? ???? ????", immediate);
  decode("0100 00?? ???? ????", alu);
  decode("0100 0111 0??? ?---", branch_exchange);
  decode("0100 01?? ???? ????", alu_hi);
  decode("0100 1??? ???? ????", load_literal);
  decode("0101 ???? ???? ????", move_register_offset);
  decode("0110 ???? ???? ????", move_word_immediate);
  decode("0111 ???? ???? ????", move_byte_immediate);
  decode("1000 ???? ???? ????", move_half_immediate);
  decode("1001 ???? ???? ????", move_stack);
  decode("1010 ???? ???? ????", add_register_hi);
  decode("1011 0000 ???? ????", adjust_stack);
  decode("1011 ?10? ???? ????", stack_multiple);
  decode("1100 ???? ???? ????", move_multiple);
  decode("1101 1111 ???? ????", software_interrupt);
  decode("1101 ???? ???? ????", branch_conditional);
  decode("1110 0??? ???? ????", branch_short);
  decode("1111 0??? ???? ????", branch_long_prefix);
  decode("1111 1??? ???? ????", branch_long_suffix);

  #undef decode

  crash = true;
}

void ARM::thumb_opcode(uint4 opcode, uint4 d, uint4 m) {
  switch(opcode) {
  case  0: r(d) = bit(r(d) & r(m));          break;  //AND
  case  1: r(d) = bit(r(d) ^ r(m));          break;  //EOR
  case  2: r(d) = bit(lsl(r(d), r(m)));      break;  //LSL
  case  3: r(d) = bit(lsr(r(d), r(m)));      break;  //LSR
  case  4: r(d) = bit(asr(r(d), r(m)));      break;  //ASR
  case  5: r(d) = add(r(d), r(m), cpsr().c); break;  //ADC
  case  6: r(d) = sub(r(d), r(m), cpsr().c); break;  //SBC
  case  7: r(d) = bit(ror(r(d), r(m)));      break;  //ROR
  case  8:        bit(r(d) & r(m));          break;  //TST
  case  9: r(d) = sub(0, r(m), 1);           break;  //NEG
  case 10:        sub(r(d), r(m), 1);        break;  //CMP
  case 11:        add(r(d), r(m), 0);        break;  //CMN
  case 12: r(d) = bit(r(d) | r(m));          break;  //ORR
  case 13: r(d) = mul(0, r(d), r(m));        break;  //MUL
  case 14: r(d) = bit(r(d) & ~r(m));         break;  //BIC
  case 15: r(d) = bit(~r(m));                break;  //MVN
  }
}

//(add,sub) rd,rn,rm
//0001 10om mmnn nddd
//o = opcode
//m = rm
//n = rn
//d = rd
void ARM::thumb_op_adjust_register() {
  uint1 opcode = instruction() >> 9;
  uint3 m = instruction() >> 6;
  uint3 n = instruction() >> 3;
  uint3 d = instruction() >> 0;

  switch(opcode) {
  case 0: r(d) = add(r(n), r(m), 0); break;
  case 1: r(d) = sub(r(n), r(m), 1); break;
  }
}

//(add,sub) rd,rn,#immediate
//0001 11oi iinn nddd
//o = opcode
//i = immediate
//n = rn
//d = rd
void ARM::thumb_op_adjust_immediate() {
  uint1 opcode = instruction() >> 9;
  uint3 immediate = instruction() >> 6;
  uint3 n = instruction() >> 3;
  uint3 d = instruction() >> 0;

  switch(opcode) {
  case 0: r(d) = add(r(n), immediate, 0); break;
  case 1: r(d) = sub(r(n), immediate, 1); break;
  }
}

//(lsl,lsr,asr) rd,rm,#immediate
//000o oiii iimm mddd
//o = opcode
//i = immediate
//m = rm
//d = rd
void ARM::thumb_op_shift_immediate() {
  uint2 opcode = instruction() >> 11;
  uint5 immediate = instruction() >> 6;
  uint3 m = instruction() >> 3;
  uint3 d = instruction() >> 0;

  switch(opcode) {
  case 0: r(d) = bit(lsl(r(m), immediate)); break;
  case 1: r(d) = bit(lsr(r(m), immediate == 0 ? 32u : (unsigned)immediate)); break;
  case 2: r(d) = bit(asr(r(m), immediate == 0 ? 32u : (unsigned)immediate)); break;
  }
}

//(mov,cmp,add,sub) (rd,rn),#immediate
//001o orrr iiii iiii
//o = opcode
//r = (rd,rn)
//i = immediate
void ARM::thumb_op_immediate() {
  uint2 opcode = instruction() >> 11;
  uint3 d = instruction() >> 8;
  uint8 immediate = instruction();

  switch(opcode) {
  case 0: r(d) = bit(      immediate   ); break;
  case 1:        sub(r(d), immediate, 1); break;
  case 2: r(d) = add(r(d), immediate, 0); break;
  case 3: r(d) = sub(r(d), immediate, 1); break;
  }
}

//{opcode} rd,rm
//0100 00oo oomm mddd
//o = opcode
//m = rm
//d = rd
void ARM::thumb_op_alu() {
  uint4 opcode = instruction() >> 6;
  uint3 m = instruction() >> 3;
  uint3 d = instruction();

  thumb_opcode(opcode, d, m);
}

//bx rm
//0100 0111 0mmm m---
//m = rm
void ARM::thumb_op_branch_exchange() {
  uint4 m = instruction() >> 3;

  cpsr().t = r(m) & 1;
  r(15) = r(m);
}

//{opcode} rd,rm
//0100 01oo DMmm mddd
//o = opcode
//M:m = rm
//D:d = rd
void ARM::thumb_op_alu_hi() {
  uint2 opcode = instruction() >> 8;
  uint4 m = instruction() >> 3;
  uint3 dl = instruction();
  uint1 dh = instruction() >> 7;

  uint4 d = (dh << 3) + (dl << 0);
  switch(opcode) {
  case 0: r(d) = r(d) + r(m); break;  //ADD (does not modify flags)
  case 1: sub(r(d), r(m), 1); break;  //SUB
  case 2: r(d) = r(m);        break;  //MOV (does not modify flags)
  }
}

//ldr rd,[pc,#+/-offset]
//0100 1ddd oooo oooo
//d = rd
//o = offset
void ARM::thumb_op_load_literal() {
  uint3 d = instruction() >> 8;
  uint8 displacement = instruction();

  unsigned rm = (r(15) & ~3) + displacement * 4;
  r(d) = load(rm, Word);
}

//(ld(r,s),str){b,h} rd,[rn,rm]
//0101 ooom mmnn nddd
//o = opcode
//m = rm
//n = rn
//d = rd
void ARM::thumb_op_move_register_offset() {
  uint3 opcode = instruction() >> 9;
  uint3 m = instruction() >> 6;
  uint3 n = instruction() >> 3;
  uint3 d = instruction() >> 0;

  switch(opcode) {
  case 0: store(r(n) + r(m), Word, r(d));        break;  //STR
  case 1: store(r(n) + r(m), Half, r(d));        break;  //STRH
  case 2: store(r(n) + r(m), Byte, r(d));        break;  //STRB
  case 3: r(d) =  (int8)load(r(n) + r(m), Byte); break;  //LDSB
  case 4: r(d) = load(r(n) + r(m), Word);        break;  //LDR
  case 5: r(d) = load(r(n) + r(m), Half);        break;  //LDRH
  case 6: r(d) = load(r(n) + r(m), Byte);        break;  //LDRB
  case 7: r(d) = (int16)load(r(n) + r(m), Half); break;  //LDSH
  }
}

//(ldr,str) rd,[rn,#offset]
//0110 looo oonn nddd
//l = load
//o = offset
//n = rn
//d = rd
void ARM::thumb_op_move_word_immediate() {
  uint1 l = instruction() >> 11;
  uint5 offset = instruction() >> 6;
  uint3 n = instruction() >> 3;
  uint3 d = instruction() >> 0;

  if(l == 1) r(d) = load(r(n) + offset * 4, Word);
  if(l == 0) store(r(n) + offset * 4, Word, r(d));
}

//(ldr,str)b rd,[rn,#offset]
//0111 looo oonn nddd
//l = load
//o = offset
//n = rn
//d = rd
void ARM::thumb_op_move_byte_immediate() {
  uint1 l = instruction() >> 11;
  uint5 offset = instruction() >> 6;
  uint3 n = instruction() >> 3;
  uint3 d = instruction() >> 0;

  if(l == 1) r(d) = load(r(n) + offset, Byte);
  if(l == 0) store(r(n) + offset, Byte, r(d));
}

//(ldr,str)h rd,[rn,#offset]
//1000 looo oonn nddd
//l = load
//o = offset
//n = rn
//d = rd
void ARM::thumb_op_move_half_immediate() {
  uint1 l = instruction() >> 11;
  uint5 offset = instruction() >> 6;
  uint3 n = instruction() >> 3;
  uint3 d = instruction() >> 0;

  if(l == 1) r(d) = load(r(n) + offset * 2, Half);
  if(l == 0) store(r(n) + offset * 2, Half, r(d));
}

//(ldr,str) rd,[sp,#immediate]
//1001 oddd iiii iiii
//l = load
//d = rd
//i = immediate
void ARM::thumb_op_move_stack() {
  uint1 l = instruction() >> 11;
  uint3 d = instruction() >> 8;
  uint8 immediate = instruction();

  if(l == 1) r(d) = load(r(13) + immediate * 4, Word);
  if(l == 0) store(r(13) + immediate * 4, Word, r(d));
}

//add rd,{pc,sp},#immediate
//1010 sddd iiii iiii
//s = sp (0 = pc)
//d = rd
//i = immediate
void ARM::thumb_op_add_register_hi() {
  uint1 sp = instruction() >> 11;
  uint3 d = instruction() >> 8;
  uint8 immediate = instruction();

  if(sp == 0) r(d) = (r(15) & ~2) + immediate * 4;
  if(sp == 1) r(d) = r(13) + immediate * 4;
}

//(add,sub) sp,#immediate
//1011 0000 oiii iiii
//o = opcode
//i = immediate
void ARM::thumb_op_adjust_stack() {
  uint1 opcode = instruction() >> 7;
  uint7 immediate = instruction();

  if(opcode == 0) r(13) += immediate * 4;
  if(opcode == 1) r(13) -= immediate * 4;
}

//push {r...{,lr}}
//pop {r...{,pc}}
//1011 o10r llll llll
//o = opcode (0 = push, 1 = pop)
//r = push lr -or- pop pc
//l = register list
void ARM::thumb_op_stack_multiple() {
  uint1 l = instruction() >> 11;
  uint1 branch = instruction() >> 8;
  uint8 list = instruction();

  uint32 sp = 0;
  if(l == 1) sp = r(13);
  if(l == 0) sp = r(13) - (bit::count(list) + branch) * 4;

  sequential() = false;
  for(unsigned m = 0; m < 8; m++) {
    if(list & (1 << m)) {
      if(l == 1) r(m) = read(sp, Word);  //POP
      if(l == 0) write(sp, Word, r(m));  //PUSH
      sp += 4;
    }
  }

  if(branch) {
    //note: ARMv5+ POP sets cpsr().t
    if(l == 1) r(15) = read(sp, Word);  //POP
    if(l == 0) write(sp, Word, r(14));  //PUSH
    sp += 4;
  }

  if(l == 1) idle();
  if(l == 1) r(13) += (bit::count(list) + branch) * 4;
  if(l == 0) r(13) -= (bit::count(list) + branch) * 4;
}

//(ldmia,stmia) rn!,{r...}
//1100 lnnn llll llll
//l = load (0 = save)
//n = rn
//l = register list
void ARM::thumb_op_move_multiple() {
  uint1 l = instruction() >> 11;
  uint3 n = instruction() >> 8;
  uint8 list = instruction();

  sequential() = false;
  for(unsigned m = 0; m < 8; m++) {
    if(list & (1 << m)) {
      if(l == 1) r(m) = read(r(n), Word);  //LDMIA
      if(l == 0) write(r(n), Word, r(m));  //STMIA
      r(n) += 4;
    }
  }

  if(l == 1) idle();
}

//swi #immediate
//1101 1111 iiii iiii
//i = immediate
void ARM::thumb_op_software_interrupt() {
  uint8 immediate = instruction();

  vector(0x00000008, Processor::Mode::SVC);
}

//b{condition}
//1101 cccc dddd dddd
//c = condition
//d = displacement
void ARM::thumb_op_branch_conditional() {
  uint4 flagcondition = instruction() >> 8;
  int8 displacement = instruction();

  if(condition(flagcondition) == false) return;
  r(15) = r(15) + displacement * 2;
}

//b address
//1110 0ooo oooo oooo
//o = offset
void ARM::thumb_op_branch_short() {
  int11 displacement = instruction();

  r(15) += displacement * 2;
}

//bl address
//1111 0ooo oooo oooo
//o = offset
void ARM::thumb_op_branch_long_prefix() {
  int11 offsethi = instruction();

  r(14) = r(15) + ((offsethi * 2) << 11);
}

//bl address
//1111 1ooo oooo oooo
//o = offset
void ARM::thumb_op_branch_long_suffix() {
  uint11 offsetlo = instruction();

  r(15) = r(14) + (offsetlo * 2);
  r(14) = pipeline.decode.address | 1;
}

#endif
