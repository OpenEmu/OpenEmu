void BSXCartridge::serialize(serializer &s) {
  s.array(sram.data(), sram.size());
  s.array(psram.data(), psram.size());
}
