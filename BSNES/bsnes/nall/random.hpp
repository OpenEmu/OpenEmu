#ifndef NALL_RANDOM_HPP
#define NALL_RANDOM_HPP

namespace nall {
  //pseudo-random number generator
  inline unsigned prng() {
    static unsigned n = 0;
    return n = (n >> 1) ^ (((n & 1) - 1) & 0xedb88320);
  }

  struct random_lfsr {
    inline void seed(unsigned seed__) {
      seed_ = seed__;
    }

    inline unsigned operator()() {
      return seed_ = (seed_ >> 1) ^ (((seed_ & 1) - 1) & 0xedb88320);
    }

    random_lfsr() : seed_(0) {
    }

  private:
    unsigned seed_;
  };
}

#endif
