template<int bit, int val> void CPUcore::op_branch() {
  if((bool)(regs.p & bit) != val) {
L   rd.l = op_readpc();
  } else {
    rd.l = op_readpc();
    aa.w = regs.pc.d + (int8)rd.l;
    op_io_cond6(aa.w);
L   op_io();
    regs.pc.w = aa.w;
  }
}

void CPUcore::op_bra() {
  rd.l = op_readpc();
  aa.w = regs.pc.d + (int8)rd.l;
  op_io_cond6(aa.w);
L op_io();
  regs.pc.w = aa.w;
}

void CPUcore::op_brl() {
  rd.l = op_readpc();
  rd.h = op_readpc();
L op_io();
  regs.pc.w = regs.pc.d + (int16)rd.w;
}

void CPUcore::op_jmp_addr() {
  rd.l = op_readpc();
L rd.h = op_readpc();
  regs.pc.w = rd.w;
}

void CPUcore::op_jmp_long() {
  rd.l = op_readpc();
  rd.h = op_readpc();
L rd.b = op_readpc();
  regs.pc.d = rd.d & 0xffffff;
}

void CPUcore::op_jmp_iaddr() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  rd.l = op_readaddr(aa.w + 0);
L rd.h = op_readaddr(aa.w + 1);
  regs.pc.w = rd.w;
}

void CPUcore::op_jmp_iaddrx() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
  rd.l = op_readpbr(aa.w + regs.x.w + 0);
L rd.h = op_readpbr(aa.w + regs.x.w + 1);
  regs.pc.w = rd.w;
}

void CPUcore::op_jmp_iladdr() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  rd.l = op_readaddr(aa.w + 0);
  rd.h = op_readaddr(aa.w + 1);
L rd.b = op_readaddr(aa.w + 2);
  regs.pc.d = rd.d & 0xffffff;
}

void CPUcore::op_jsr_addr() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
  regs.pc.w--;
  op_writestack(regs.pc.h);
L op_writestack(regs.pc.l);
  regs.pc.w = aa.w;
}

void CPUcore::op_jsr_long_e() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_writestackn(regs.pc.b);
  op_io();
  aa.b = op_readpc();
  regs.pc.w--;
  op_writestackn(regs.pc.h);
L op_writestackn(regs.pc.l);
  regs.pc.d = aa.d & 0xffffff;
  regs.s.h = 0x01;
}

void CPUcore::op_jsr_long_n() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_writestackn(regs.pc.b);
  op_io();
  aa.b = op_readpc();
  regs.pc.w--;
  op_writestackn(regs.pc.h);
L op_writestackn(regs.pc.l);
  regs.pc.d = aa.d & 0xffffff;
}

void CPUcore::op_jsr_iaddrx_e() {
  aa.l = op_readpc();
  op_writestackn(regs.pc.h);
  op_writestackn(regs.pc.l);
  aa.h = op_readpc();
  op_io();
  rd.l = op_readpbr(aa.w + regs.x.w + 0);
L rd.h = op_readpbr(aa.w + regs.x.w + 1);
  regs.pc.w = rd.w;
  regs.s.h = 0x01;
}

void CPUcore::op_jsr_iaddrx_n() {
  aa.l = op_readpc();
  op_writestackn(regs.pc.h);
  op_writestackn(regs.pc.l);
  aa.h = op_readpc();
  op_io();
  rd.l = op_readpbr(aa.w + regs.x.w + 0);
L rd.h = op_readpbr(aa.w + regs.x.w + 1);
  regs.pc.w = rd.w;
}

void CPUcore::op_rti_e() {
  op_io();
  op_io();
  regs.p = op_readstack() | 0x30;
  rd.l = op_readstack();
L rd.h = op_readstack();
  regs.pc.w = rd.w;
}

void CPUcore::op_rti_n() {
  op_io();
  op_io();
  regs.p = op_readstack();
  if(regs.p.x) {
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  rd.l = op_readstack();
  rd.h = op_readstack();
L rd.b = op_readstack();
  regs.pc.d = rd.d & 0xffffff;
  update_table();
}

void CPUcore::op_rts() {
  op_io();
  op_io();
  rd.l = op_readstack();
  rd.h = op_readstack();
L op_io();
  regs.pc.w = ++rd.w;
}

void CPUcore::op_rtl_e() {
  op_io();
  op_io();
  rd.l = op_readstackn();
  rd.h = op_readstackn();
L rd.b = op_readstackn();
  regs.pc.b = rd.b;
  regs.pc.w = ++rd.w;
  regs.s.h = 0x01;
}

void CPUcore::op_rtl_n() {
  op_io();
  op_io();
  rd.l = op_readstackn();
  rd.h = op_readstackn();
L rd.b = op_readstackn();
  regs.pc.b = rd.b;
  regs.pc.w = ++rd.w;
}
