#ifndef NALL_UPS_HPP
#define NALL_UPS_HPP

#include <nall/crc32.hpp>
#include <nall/file.hpp>
#include <nall/function.hpp>
#include <nall/stdint.hpp>

namespace nall {

struct ups {
  enum class result : unsigned {
    unknown,
    success,
    patch_unwritable,
    patch_invalid,
    source_invalid,
    target_invalid,
    target_too_small,
    patch_checksum_invalid,
    source_checksum_invalid,
    target_checksum_invalid,
  };

  function<void (unsigned offset, unsigned length)> progress;

  result create(
    const uint8_t *sourcedata, unsigned sourcelength,
    const uint8_t *targetdata, unsigned targetlength,
    const char *patchfilename
  ) {
    source_data = (uint8_t*)sourcedata, target_data = (uint8_t*)targetdata;
    source_length = sourcelength, target_length = targetlength;
    source_offset = target_offset = 0;
    source_checksum = target_checksum = patch_checksum = ~0;

    if(patch_file.open(patchfilename, file::mode::write) == false) return result::patch_unwritable;

    patch_write('U');
    patch_write('P');
    patch_write('S');
    patch_write('1');
    encode(source_length);
    encode(target_length);

    unsigned output_length = source_length > target_length ? source_length : target_length;
    unsigned relative = 0;
    for(unsigned offset = 0; offset < output_length;) {
      uint8_t x = source_read();
      uint8_t y = target_read();

      if(x == y) {
        offset++;
        continue;
      }

      encode(offset++ - relative);
      patch_write(x ^ y);

      while(true) {
        if(offset >= output_length) {
          patch_write(0x00);
          break;
        }

        x = source_read();
        y = target_read();
        offset++;
        patch_write(x ^ y);
        if(x == y) break;
      }

      relative = offset;
    }

    source_checksum = ~source_checksum;
    target_checksum = ~target_checksum;
    for(unsigned i = 0; i < 4; i++) patch_write(source_checksum >> (i * 8));
    for(unsigned i = 0; i < 4; i++) patch_write(target_checksum >> (i * 8));
    uint32_t patch_result_checksum = ~patch_checksum;
    for(unsigned i = 0; i < 4; i++) patch_write(patch_result_checksum >> (i * 8));

    patch_file.close();
    return result::success;
  }

  result apply(
    const uint8_t *patchdata, unsigned patchlength,
    const uint8_t *sourcedata, unsigned sourcelength,
    uint8_t *targetdata, unsigned &targetlength
  ) {
    patch_data = (uint8_t*)patchdata, source_data = (uint8_t*)sourcedata, target_data = targetdata;
    patch_length = patchlength, source_length = sourcelength, target_length = targetlength;
    patch_offset = source_offset = target_offset = 0;
    patch_checksum = source_checksum = target_checksum = ~0;

    if(patch_length < 18) return result::patch_invalid;
    if(patch_read() != 'U') return result::patch_invalid;
    if(patch_read() != 'P') return result::patch_invalid;
    if(patch_read() != 'S') return result::patch_invalid;
    if(patch_read() != '1') return result::patch_invalid;

    unsigned source_read_length = decode();
    unsigned target_read_length = decode();

    if(source_length != source_read_length && source_length != target_read_length) return result::source_invalid;
    targetlength = (source_length == source_read_length ? target_read_length : source_read_length);
    if(target_length < targetlength) return result::target_too_small;
    target_length = targetlength;

    while(patch_offset < patch_length - 12) {
      unsigned length = decode();
      while(length--) target_write(source_read());
      while(true) {
        uint8_t patch_xor = patch_read();
        target_write(patch_xor ^ source_read());
        if(patch_xor == 0) break;
      }
    }
    while(source_offset < source_length) target_write(source_read());
    while(target_offset < target_length) target_write(source_read());

    uint32_t patch_read_checksum = 0, source_read_checksum = 0, target_read_checksum = 0;
    for(unsigned i = 0; i < 4; i++) source_read_checksum |= patch_read() << (i * 8);
    for(unsigned i = 0; i < 4; i++) target_read_checksum |= patch_read() << (i * 8);
    uint32_t patch_result_checksum = ~patch_checksum;
    source_checksum = ~source_checksum;
    target_checksum = ~target_checksum;
    for(unsigned i = 0; i < 4; i++) patch_read_checksum  |= patch_read() << (i * 8);

    if(patch_result_checksum != patch_read_checksum) return result::patch_invalid;
    if(source_checksum == source_read_checksum && source_length == source_read_length) {
      if(target_checksum == target_read_checksum && target_length == target_read_length) return result::success;
      return result::target_invalid;
    } else if(source_checksum == target_read_checksum && source_length == target_read_length) {
      if(target_checksum == source_read_checksum && target_length == source_read_length) return result::success;
      return result::target_invalid;
    } else {
      return result::source_invalid;
    }
  }

private:
  uint8_t *patch_data, *source_data, *target_data;
  unsigned patch_length, source_length, target_length;
  unsigned patch_offset, source_offset, target_offset;
  unsigned patch_checksum, source_checksum, target_checksum;
  file patch_file;

  uint8_t patch_read() {
    if(patch_offset < patch_length) {
      uint8_t n = patch_data[patch_offset++];
      patch_checksum = crc32_adjust(patch_checksum, n);
      return n;
    }
    return 0x00;
  }

  uint8_t source_read() {
    if(source_offset < source_length) {
      uint8_t n = source_data[source_offset++];
      source_checksum = crc32_adjust(source_checksum, n);
      return n;
    }
    return 0x00;
  }

  uint8_t target_read() {
    uint8_t result = 0x00;
    if(target_offset < target_length) {
      result = target_data[target_offset];
      target_checksum = crc32_adjust(target_checksum, result);
    }
    if(((target_offset++ & 255) == 0) && progress) {
      progress(target_offset, source_length > target_length ? source_length : target_length);
    }
    return result;
  }

  void patch_write(uint8_t n) {
    patch_file.write(n);
    patch_checksum = crc32_adjust(patch_checksum, n);
  }

  void target_write(uint8_t n) {
    if(target_offset < target_length) {
      target_data[target_offset] = n;
      target_checksum = crc32_adjust(target_checksum, n);
    }
    if(((target_offset++ & 255) == 0) && progress) {
      progress(target_offset, source_length > target_length ? source_length : target_length);
    }
  }

  void encode(uint64_t offset) {
    while(true) {
      uint64_t x = offset & 0x7f;
      offset >>= 7;
      if(offset == 0) {
        patch_write(0x80 | x);
        break;
      }
      patch_write(x);
      offset--;
    }
  }

  uint64_t decode() {
    uint64_t offset = 0, shift = 1;
    while(true) {
      uint8_t x = patch_read();
      offset += (x & 0x7f) * shift;
      if(x & 0x80) break;
      shift <<= 7;
      offset += shift;
    }
    return offset;
  }
};

}

#endif
