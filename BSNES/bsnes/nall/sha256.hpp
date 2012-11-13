#ifndef NALL_SHA256_HPP
#define NALL_SHA256_HPP

//author: vladitx

#include <nall/stdint.hpp>

namespace nall {
  #define PTR(t, a) ((t*)(a))

  #define SWAP32(x) ((uint32_t)(           \
    (((uint32_t)(x) & 0x000000ff) << 24) | \
    (((uint32_t)(x) & 0x0000ff00) <<  8) | \
    (((uint32_t)(x) & 0x00ff0000) >>  8) | \
    (((uint32_t)(x) & 0xff000000) >> 24)   \
  ))

  #define ST32(a, d) *PTR(uint32_t, a) = (d)
  #define ST32BE(a, d) ST32(a, SWAP32(d))

  #define LD32(a) *PTR(uint32_t, a)
  #define LD32BE(a) SWAP32(LD32(a))

  #define LSL32(x, n) ((uint32_t)(x) << (n))
  #define LSR32(x, n) ((uint32_t)(x) >> (n))
  #define ROR32(x, n) (LSR32(x, n) | LSL32(x, 32 - (n)))

  //first 32 bits of the fractional parts of the square roots of the first 8 primes 2..19
  static const uint32_t T_H[8] = {
    0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
  };

  //first 32 bits of the fractional parts of the cube roots of the first 64 primes 2..311
  static const uint32_t T_K[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
  };

  struct sha256_ctx {
    uint8_t in[64];
    unsigned inlen;

    uint32_t w[64];
    uint32_t h[8];
    uint64_t len;
  };

  inline void sha256_init(sha256_ctx *p) {
    memset(p, 0, sizeof(sha256_ctx));
    memcpy(p->h, T_H, sizeof(T_H));
  }

  static void sha256_block(sha256_ctx *p) {
    unsigned i;
    uint32_t s0, s1;
    uint32_t a, b, c, d, e, f, g, h;
    uint32_t t1, t2, maj, ch;

    for(i = 0; i < 16; i++) p->w[i] = LD32BE(p->in + i * 4);

    for(i = 16; i < 64; i++) {
      s0 = ROR32(p->w[i - 15],  7) ^ ROR32(p->w[i - 15], 18) ^ LSR32(p->w[i - 15],  3);
      s1 = ROR32(p->w[i -  2], 17) ^ ROR32(p->w[i -  2], 19) ^ LSR32(p->w[i -  2], 10);
      p->w[i] = p->w[i - 16] + s0 + p->w[i - 7] + s1;
    }

    a = p->h[0]; b = p->h[1]; c = p->h[2]; d = p->h[3];
    e = p->h[4]; f = p->h[5]; g = p->h[6]; h = p->h[7];

    for(i = 0; i < 64; i++) {
      s0 = ROR32(a, 2) ^ ROR32(a, 13) ^ ROR32(a, 22);
      maj = (a & b) ^ (a & c) ^ (b & c);
      t2 = s0 + maj;
      s1 = ROR32(e, 6) ^ ROR32(e, 11) ^ ROR32(e, 25);
      ch = (e & f) ^ (~e & g);
      t1 = h + s1 + ch + T_K[i] + p->w[i];

      h = g; g = f; f = e; e = d + t1;
      d = c; c = b; b = a; a = t1 + t2;
    }

    p->h[0] += a; p->h[1] += b; p->h[2] += c; p->h[3] += d;
    p->h[4] += e; p->h[5] += f; p->h[6] += g; p->h[7] += h;

    //next block
    p->inlen = 0;
  }

  inline void sha256_chunk(sha256_ctx *p, const uint8_t *s, unsigned len) {
    unsigned l;
    p->len += len;

    while(len) {
      l = 64 - p->inlen;
      l = (len < l) ? len : l;

      memcpy(p->in + p->inlen, s, l);
      s += l;
      p->inlen += l;
      len -= l;

      if(p->inlen == 64) sha256_block(p);
    }
  }

  inline void sha256_final(sha256_ctx *p) {
    uint64_t len;
    p->in[p->inlen++] = 0x80;

    if(p->inlen > 56) {
      memset(p->in + p->inlen, 0, 64 - p->inlen);
      sha256_block(p);
    }

    memset(p->in + p->inlen, 0, 56 - p->inlen);

    len = p->len << 3;
    ST32BE(p->in + 56, len >> 32);
    ST32BE(p->in + 60, len);
    sha256_block(p);
  }

  inline void sha256_hash(sha256_ctx *p, uint8_t *s) {
    uint32_t *t = (uint32_t*)s;
    for(unsigned i = 0; i < 8; i++) ST32BE(t++, p->h[i]);
  }

  #undef PTR
  #undef SWAP32
  #undef ST32
  #undef ST32BE
  #undef LD32
  #undef LD32BE
  #undef LSL32
  #undef LSR32
  #undef ROR32
}

#endif
