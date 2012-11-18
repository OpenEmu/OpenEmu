void LR35902::serialize(serializer &s) {
  s.integer(r.a.data);
  s.integer(r.f.z);
  s.integer(r.f.n);
  s.integer(r.f.h);
  s.integer(r.f.c);
  s.integer(r.b.data);
  s.integer(r.c.data);
  s.integer(r.d.data);
  s.integer(r.e.data);
  s.integer(r.h.data);
  s.integer(r.l.data);
  s.integer(r.sp.data);
  s.integer(r.pc.data);

  s.integer(r.halt);
  s.integer(r.stop);
  s.integer(r.ei);
  s.integer(r.ime);
}
