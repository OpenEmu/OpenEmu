#ifdef CPUCORE_CPP

void CPUcore::op_nop() {
L op_io_irq();
}

void CPUcore::op_wdm() {
L op_readpc();
}

void CPUcore::op_xba() {
  op_io();
L op_io();
  regs.a.l ^= regs.a.h;
  regs.a.h ^= regs.a.l;
  regs.a.l ^= regs.a.h;
  regs.p.n = (regs.a.l & 0x80);
  regs.p.z = (regs.a.l == 0);
}

template<int adjust> void CPUcore::op_move_b() {
  dp = op_readpc();
  sp = op_readpc();
  regs.db = dp;
  rd.l = op_readlong((sp << 16) | regs.x.w);
  op_writelong((dp << 16) | regs.y.w, rd.l);
  op_io();
  regs.x.l += adjust;
  regs.y.l += adjust;
L op_io();
  if(regs.a.w--) regs.pc.w -= 3;
}

template<int adjust> void CPUcore::op_move_w() {
  dp = op_readpc();
  sp = op_readpc();
  regs.db = dp;
  rd.l = op_readlong((sp << 16) | regs.x.w);
  op_writelong((dp << 16) | regs.y.w, rd.l);
  op_io();
  regs.x.w += adjust;
  regs.y.w += adjust;
L op_io();
  if(regs.a.w--) regs.pc.w -= 3;
}

template<int vectorE, int vectorN> void CPUcore::op_interrupt_e() {
  op_readpc();
  op_writestack(regs.pc.h);
  op_writestack(regs.pc.l);
  op_writestack(regs.p);
  rd.l = op_readlong(vectorE + 0);
  regs.pc.b = 0;
  regs.p.i = 1;
  regs.p.d = 0;
L rd.h = op_readlong(vectorE + 1);
  regs.pc.w = rd.w;
}

template<int vectorE, int vectorN> void CPUcore::op_interrupt_n() {
  op_readpc();
  op_writestack(regs.pc.b);
  op_writestack(regs.pc.h);
  op_writestack(regs.pc.l);
  op_writestack(regs.p);
  rd.l = op_readlong(vectorN + 0);
  regs.pc.b = 0x00;
  regs.p.i = 1;
  regs.p.d = 0;
L rd.h = op_readlong(vectorN + 1);
  regs.pc.w = rd.w;
}

void CPUcore::op_stp() {
  while(regs.wai = true) {
L   op_io();
  }
}

void CPUcore::op_wai() {
  regs.wai = true;
  while(regs.wai) {
L   op_io();
  }
  op_io();
}

void CPUcore::op_xce() {
L op_io_irq();
  bool carry = regs.p.c;
  regs.p.c = regs.e;
  regs.e = carry;
  if(regs.e) {
    regs.p |= 0x30;
    regs.s.h = 0x01;
  }
  if(regs.p.x) {
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  update_table();
}

template<int mask, int value> void CPUcore::op_flag() {
L op_io_irq();
  regs.p = (regs.p & ~mask) | value;
}

template<int mode> void CPUcore::op_pflag_e() {
  rd.l = op_readpc();
L op_io();
  regs.p = (mode ? regs.p | rd.l : regs.p & ~rd.l);
  regs.p |= 0x30;
  if(regs.p.x) {
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  update_table();
}

template<int mode> void CPUcore::op_pflag_n() {
  rd.l = op_readpc();
L op_io();
  regs.p = (mode ? regs.p | rd.l : regs.p & ~rd.l);
  if(regs.p.x) {
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  update_table();
}

template<int from, int to> void CPUcore::op_transfer_b() {
L op_io_irq();
  regs.r[to].l = regs.r[from].l;
  regs.p.n = (regs.r[to].l & 0x80);
  regs.p.z = (regs.r[to].l == 0);
}

template<int from, int to> void CPUcore::op_transfer_w() {
L op_io_irq();
  regs.r[to].w = regs.r[from].w;
  regs.p.n = (regs.r[to].w & 0x8000);
  regs.p.z = (regs.r[to].w == 0);
}

void CPUcore::op_tcs_e() {
L op_io_irq();
  regs.s.l = regs.a.l;
}

void CPUcore::op_tcs_n() {
L op_io_irq();
  regs.s.w = regs.a.w;
}

void CPUcore::op_tsc_e() {
L op_io_irq();
  regs.a.w = regs.s.w;
  regs.p.n = (regs.a.l & 0x80);
  regs.p.z = (regs.a.l == 0);
}

void CPUcore::op_tsc_n() {
L op_io_irq();
  regs.a.w = regs.s.w;
  regs.p.n = (regs.a.w & 0x8000);
  regs.p.z = (regs.a.w == 0);
}

void CPUcore::op_tsx_b() {
L op_io_irq();
  regs.x.l = regs.s.l;
  regs.p.n = (regs.x.l & 0x80);
  regs.p.z = (regs.x.l == 0);
}

void CPUcore::op_tsx_w() {
L op_io_irq();
  regs.x.w = regs.s.w;
  regs.p.n = (regs.x.w & 0x8000);
  regs.p.z = (regs.x.w == 0);
}

void CPUcore::op_txs_e() {
L op_io_irq();
  regs.s.l = regs.x.l;
}

void CPUcore::op_txs_n() {
L op_io_irq();
  regs.s.w = regs.x.w;
}

template<int n> void CPUcore::op_push_b() {
  op_io();
L op_writestack(regs.r[n].l);
}

template<int n> void CPUcore::op_push_w() {
  op_io();
  op_writestack(regs.r[n].h);
L op_writestack(regs.r[n].l);
}

void CPUcore::op_phd_e() {
  op_io();
  op_writestackn(regs.d.h);
L op_writestackn(regs.d.l);
  regs.s.h = 0x01;
}

void CPUcore::op_phd_n() {
  op_io();
  op_writestackn(regs.d.h);
L op_writestackn(regs.d.l);
}

void CPUcore::op_phb() {
  op_io();
L op_writestack(regs.db);
}

void CPUcore::op_phk() {
  op_io();
L op_writestack(regs.pc.b);
}

void CPUcore::op_php() {
  op_io();
L op_writestack(regs.p);
}

template<int n> void CPUcore::op_pull_b() {
  op_io();
  op_io();
L regs.r[n].l = op_readstack();
  regs.p.n = (regs.r[n].l & 0x80);
  regs.p.z = (regs.r[n].l == 0);
}

template<int n> void CPUcore::op_pull_w() {
  op_io();
  op_io();
  regs.r[n].l = op_readstack();
L regs.r[n].h = op_readstack();
  regs.p.n = (regs.r[n].w & 0x8000);
  regs.p.z = (regs.r[n].w == 0);
}

void CPUcore::op_pld_e() {
  op_io();
  op_io();
  regs.d.l = op_readstackn();
L regs.d.h = op_readstackn();
  regs.p.n = (regs.d.w & 0x8000);
  regs.p.z = (regs.d.w == 0);
  regs.s.h = 0x01;
}

void CPUcore::op_pld_n() {
  op_io();
  op_io();
  regs.d.l = op_readstackn();
L regs.d.h = op_readstackn();
  regs.p.n = (regs.d.w & 0x8000);
  regs.p.z = (regs.d.w == 0);
}

void CPUcore::op_plb() {
  op_io();
  op_io();
L regs.db = op_readstack();
  regs.p.n = (regs.db & 0x80);
  regs.p.z = (regs.db == 0);
}

void CPUcore::op_plp_e() {
  op_io();
  op_io();
L regs.p = op_readstack() | 0x30;
  if(regs.p.x) {
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  update_table();
}

void CPUcore::op_plp_n() {
  op_io();
  op_io();
L regs.p = op_readstack();
  if(regs.p.x) {
    regs.x.h = 0x00;
    regs.y.h = 0x00;
  }
  update_table();
}

void CPUcore::op_pea_e() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_writestackn(aa.h);
L op_writestackn(aa.l);
  regs.s.h = 0x01;
}

void CPUcore::op_pea_n() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_writestackn(aa.h);
L op_writestackn(aa.l);
}

void CPUcore::op_pei_e() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  op_writestackn(aa.h);
L op_writestackn(aa.l);
  regs.s.h = 0x01;
}

void CPUcore::op_pei_n() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  op_writestackn(aa.h);
L op_writestackn(aa.l);
}

void CPUcore::op_per_e() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
  rd.w = regs.pc.d + (int16)aa.w;
  op_writestackn(rd.h);
L op_writestackn(rd.l);
  regs.s.h = 0x01;
}

void CPUcore::op_per_n() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
  rd.w = regs.pc.d + (int16)aa.w;
  op_writestackn(rd.h);
L op_writestackn(rd.l);
}

#endif
