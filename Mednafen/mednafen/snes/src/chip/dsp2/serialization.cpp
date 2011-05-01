#ifdef DSP2_CPP

void DSP2::serialize(serializer &s) {
  s.integer(status.waiting_for_command);
  s.integer(status.command);
  s.integer(status.in_count);
  s.integer(status.in_index);
  s.integer(status.out_count);
  s.integer(status.out_index);

  s.array(status.parameters);
  s.array(status.output);

  s.integer(status.op05transparent);
  s.integer(status.op05haslen);
  s.integer(status.op05len);
  s.integer(status.op06haslen);
  s.integer(status.op06len);
  s.integer(status.op09word1);
  s.integer(status.op09word2);
  s.integer(status.op0dhaslen);
  s.integer(status.op0doutlen);
  s.integer(status.op0dinlen);
}

#endif
