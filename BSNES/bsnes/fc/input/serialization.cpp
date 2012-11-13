void Input::serialize(serializer &s) {
  s.integer((unsigned&)port1);
  s.integer((unsigned&)port2);

  s.integer(latchdata);
  s.integer(counter1);
  s.integer(counter2);
}
