void Cartridge::serialize(serializer &s) {
  if(has_sram) s.array(ram.data, ram.size);
  if(has_eeprom) eeprom.serialize(s);
  if(has_flashrom) flashrom.serialize(s);
}
