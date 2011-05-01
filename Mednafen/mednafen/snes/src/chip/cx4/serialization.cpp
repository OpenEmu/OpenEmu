#ifdef CX4_CPP

void Cx4::serialize(serializer &s) {
  s.array(ram);
  s.array(reg);

  s.integer(r0);
  s.integer(r1);
  s.integer(r2);
  s.integer(r3);
  s.integer(r4);
  s.integer(r5);
  s.integer(r6);
  s.integer(r7);
  s.integer(r8);
  s.integer(r9);
  s.integer(r10);
  s.integer(r11);
  s.integer(r12);
  s.integer(r13);
  s.integer(r14);
  s.integer(r15);

  s.integer(C4WFXVal);
  s.integer(C4WFYVal);
  s.integer(C4WFZVal);
  s.integer(C4WFX2Val);
  s.integer(C4WFY2Val);
  s.integer(C4WFDist);
  s.integer(C4WFScale);

  s.integer(C41FXVal);
  s.integer(C41FYVal);
  s.integer(C41FAngleRes);
  s.integer(C41FDist);
  s.integer(C41FDistVal);
}

#endif
