template<int n> void R65816::op_write_addr_b() {
  aa.l = op_readpc();
  aa.h = op_readpc();
L op_writedbr(aa.w, regs.r[n]);
}

template<int n> void R65816::op_write_addr_w() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_writedbr(aa.w + 0, regs.r[n] >> 0);
L op_writedbr(aa.w + 1, regs.r[n] >> 8);
}

template<int n, int i> void R65816::op_write_addrr_b() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
L op_writedbr(aa.w + regs.r[i], regs.r[n]);
}

template<int n, int i> void R65816::op_write_addrr_w() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  op_io();
  op_writedbr(aa.w + regs.r[i] + 0, regs.r[n] >> 0);
L op_writedbr(aa.w + regs.r[i] + 1, regs.r[n] >> 8);
}

template<int i> void R65816::op_write_longr_b() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  aa.b = op_readpc();
L op_writelong(aa.d + regs.r[i], regs.a.l);
}

template<int i> void R65816::op_write_longr_w() {
  aa.l = op_readpc();
  aa.h = op_readpc();
  aa.b = op_readpc();
  op_writelong(aa.d + regs.r[i] + 0, regs.a.l);
L op_writelong(aa.d + regs.r[i] + 1, regs.a.h);
}

template<int n> void R65816::op_write_dp_b() {
  dp = op_readpc();
  op_io_cond2();
L op_writedp(dp, regs.r[n]);
}

template<int n> void R65816::op_write_dp_w() {
  dp = op_readpc();
  op_io_cond2();
  op_writedp(dp + 0, regs.r[n] >> 0);
L op_writedp(dp + 1, regs.r[n] >> 8);
}

template<int n, int i> void R65816::op_write_dpr_b() {
  dp = op_readpc();
  op_io_cond2();
  op_io();
L op_writedp(dp + regs.r[i], regs.r[n]);
}

template<int n, int i> void R65816::op_write_dpr_w() {
  dp = op_readpc();
  op_io_cond2();
  op_io();
  op_writedp(dp + regs.r[i] + 0, regs.r[n] >> 0);
L op_writedp(dp + regs.r[i] + 1, regs.r[n] >> 8);
}

void R65816::op_sta_idp_b() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
L op_writedbr(aa.w, regs.a.l);
}

void R65816::op_sta_idp_w() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  op_writedbr(aa.w + 0, regs.a.l);
L op_writedbr(aa.w + 1, regs.a.h);
}

void R65816::op_sta_ildp_b() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  aa.b = op_readdp(dp + 2);
L op_writelong(aa.d, regs.a.l);
}

void R65816::op_sta_ildp_w() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  aa.b = op_readdp(dp + 2);
  op_writelong(aa.d + 0, regs.a.l);
L op_writelong(aa.d + 1, regs.a.h);
}

void R65816::op_sta_idpx_b() {
  dp = op_readpc();
  op_io_cond2();
  op_io();
  aa.l = op_readdp(dp + regs.x.w + 0);
  aa.h = op_readdp(dp + regs.x.w + 1);
L op_writedbr(aa.w, regs.a.l);
}

void R65816::op_sta_idpx_w() {
  dp = op_readpc();
  op_io_cond2();
  op_io();
  aa.l = op_readdp(dp + regs.x.w + 0);
  aa.h = op_readdp(dp + regs.x.w + 1);
  op_writedbr(aa.w + 0, regs.a.l);
L op_writedbr(aa.w + 1, regs.a.h);
}

void R65816::op_sta_idpy_b() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  op_io();
L op_writedbr(aa.w + regs.y.w, regs.a.l);
}

void R65816::op_sta_idpy_w() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  op_io();
  op_writedbr(aa.w + regs.y.w + 0, regs.a.l);
L op_writedbr(aa.w + regs.y.w + 1, regs.a.h);
}

void R65816::op_sta_ildpy_b() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  aa.b = op_readdp(dp + 2);
L op_writelong(aa.d + regs.y.w, regs.a.l);
}

void R65816::op_sta_ildpy_w() {
  dp = op_readpc();
  op_io_cond2();
  aa.l = op_readdp(dp + 0);
  aa.h = op_readdp(dp + 1);
  aa.b = op_readdp(dp + 2);
  op_writelong(aa.d + regs.y.w + 0, regs.a.l);
L op_writelong(aa.d + regs.y.w + 1, regs.a.h);
}

void R65816::op_sta_sr_b() {
  sp = op_readpc();
  op_io();
L op_writesp(sp, regs.a.l);
}

void R65816::op_sta_sr_w() {
  sp = op_readpc();
  op_io();
  op_writesp(sp + 0, regs.a.l);
L op_writesp(sp + 1, regs.a.h);
}

void R65816::op_sta_isry_b() {
  sp = op_readpc();
  op_io();
  aa.l = op_readsp(sp + 0);
  aa.h = op_readsp(sp + 1);
  op_io();
L op_writedbr(aa.w + regs.y.w, regs.a.l);
}

void R65816::op_sta_isry_w() {
  sp = op_readpc();
  op_io();
  aa.l = op_readsp(sp + 0);
  aa.h = op_readsp(sp + 1);
  op_io();
  op_writedbr(aa.w + regs.y.w + 0, regs.a.l);
L op_writedbr(aa.w + regs.y.w + 1, regs.a.h);
}
