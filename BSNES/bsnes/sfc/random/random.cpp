Random random;

void Random::seed(unsigned seed_iter) {
  iter = seed_iter;
}

unsigned Random::operator()(unsigned result) {
  if(config.random == false) return result;
  return iter = (iter >> 1) ^ (((iter & 1) - 1) & 0xedb88320);
}

void Random::serialize(serializer &s) {
  s.integer(iter);
}

Random::Random() {
  iter = 0;
}
