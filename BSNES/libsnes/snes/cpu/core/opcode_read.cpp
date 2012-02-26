#ifdef CPUCORE_CPP

template<void (CPUcore::*op)()> void CPUcore::op_read_const_b() {
L rd.l = op_readpc();
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_const_w() {
  rd.l = op_readpc();
L rd.h = op_readpc();
  call(op);
}

void CPUcore::op_read_bit_const_b() {
L rd.l = op_readpc();
  regs.p.z = ((rd.l & regs.a.l) == 0);
}

void CPUcore::op_read_bit_const_w() {
  rd.l = op_readpc();
L rd.h = op_readpc();
  regs.p.z = ((rd.w & regs.a.w) == 0);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_addr_b() {
  aa.l = op_readpc();
  aa.h = op_readpc();
L rd.l = op_readdbr(aa.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_addr_w() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  rd.l = op_readdbr(aa.w + 0);
L rd.h = op_readdbr(aa.w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_addrx_b() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io_cond4(aa.w, aa.w + regs.x.w);
L rd.l = op_readdbr(aa.w + regs.x.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_addrx_w() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io_cond4(aa.w, aa.w + regs.x.w);
  rd.l = op_readdbr(aa.w + regs.x.w + 0);
L rd.h = op_readdbr(aa.w + regs.x.w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_addry_b() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io_cond4(aa.w, aa.w + regs.y.w);
L rd.l = op_readdbr(aa.w + regs.y.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_addry_w() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io_cond4(aa.w, aa.w + regs.y.w);
  rd.l = op_readdbr(aa.w + regs.y.w + 0);
L rd.h = op_readdbr(aa.w + regs.y.w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_long_b() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  aa.b = op_readpc();
L rd.l = op_readlong(aa.d);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_long_w() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  aa.b = op_readpc();
  rd.l = op_readlong(aa.d + 0);
L rd.h = op_readlong(aa.d + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_longx_b() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  aa.b = op_readpc();
L rd.l = op_readlong(aa.d + regs.x.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_longx_w() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  aa.b = op_readpc();
  rd.l = op_readlong(aa.d + regs.x.w + 0);
L rd.h = op_readlong(aa.d + regs.x.w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_dp_b() {
  dp = op_readpc();
  op_io_cond2();
L rd.l = op_readdp(dp);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_dp_w() {
  dp = op_readpc();
  op_io_cond2();
  rd.l = op_readdp(dp + 0);
L rd.h = op_readdp(dp + 1);
  call(op);
}

template<void (CPUcore::*op)(), int n> void CPUcore::op_read_dpr_b() {
  dp = op_readpc();
  op_io_cond2();
  op_io();
L rd.l = op_readdp(dp + regs.r[n].w);
  call(op);
}

template<void (CPUcore::*op)(), int n> void CPUcore::op_read_dpr_w() {
  dp = op_readpc();
  op_io_cond2();
  op_io();
  rd.l = op_readdp(dp + regs.r[n].w + 0);
L rd.h = op_readdp(dp + regs.r[n].w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_idp_b() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
L rd.l = op_readdbr(aa.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_idp_w() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  rd.l = op_readdbr(aa.w + 0);
L rd.h = op_readdbr(aa.w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_idpx_b() {
  dp = op_readpc();
  op_io_cond2();
  op_io();
  aa.l = op_readdp(dp + regs.x.w + 0);
  aa.h = op_readdp(dp + regs.x.w + 1);
L rd.l = op_readdbr(aa.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_idpx_w() {
  dp = op_readpc();
  op_io_cond2();
  op_io();
  aa.l = op_readdp(dp + regs.x.w + 0);
  aa.h = op_readdp(dp + regs.x.w + 1);
  rd.l = op_readdbr(aa.w + 0);
L rd.h = op_readdbr(aa.w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_idpy_b() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  op_io_cond4(aa.w, aa.w + regs.y.w);
L rd.l = op_readdbr(aa.w + regs.y.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_idpy_w() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  op_io_cond4(aa.w, aa.w + regs.y.w);
  rd.l = op_readdbr(aa.w + regs.y.w + 0);
L rd.h = op_readdbr(aa.w + regs.y.w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_ildp_b() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  aa.b = op_readdp(dp + 2);
L rd.l = op_readlong(aa.d);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_ildp_w() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  aa.b = op_readdp(dp + 2);
  rd.l = op_readlong(aa.d + 0);
L rd.h = op_readlong(aa.d + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_ildpy_b() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  aa.b = op_readdp(dp + 2);
L rd.l = op_readlong(aa.d + regs.y.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_ildpy_w() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  aa.b = op_readdp(dp + 2);
  rd.l = op_readlong(aa.d + regs.y.w + 0);
L rd.h = op_readlong(aa.d + regs.y.w + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_sr_b() {
  sp = op_readpc();
  op_io();
L rd.l = op_readsp(sp);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_sr_w() {
  sp = op_readpc();
  op_io();
  rd.l = op_readsp(sp + 0);
L rd.h = op_readsp(sp + 1);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_isry_b() {
  sp = op_readpc();
  op_io();
  aa.l = op_readsp(sp + 0);
  aa.h = op_readsp(sp + 1);
  op_io();
L rd.l = op_readdbr(aa.w + regs.y.w);
  call(op);
}

template<void (CPUcore::*op)()> void CPUcore::op_read_isry_w() {
  sp = op_readpc();
  op_io();
  aa.l = op_readsp(sp + 0);
  aa.h = op_readsp(sp + 1);
  op_io();
  rd.l = op_readdbr(aa.w + regs.y.w + 0);
L rd.h = op_readdbr(aa.w + regs.y.w + 1);
  call(op);
}

#endif
