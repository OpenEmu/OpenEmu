void APU::FIFO::read() {
  if(size == 0) return;
  size--;
  output = sample[rdoffset++];
}

void APU::FIFO::write(int8 byte) {
  if(size == 32) return;
  size++;
  sample[wroffset++] = byte;
}

void APU::FIFO::reset() {
  for(auto &byte : sample) byte = 0;
  output = 0;

  rdoffset = 0;
  wroffset = 0;
  size = 0;
}

void APU::FIFO::power() {
  reset();

  lenable = 0;
  renable = 0;
  timer = 0;
}
