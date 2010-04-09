#define _FP_W_TYPE_SIZE         32
#define _FP_W_TYPE              uint32
#define _FP_WS_TYPE             int32
#define _FP_I_TYPE              int32

#define _FP_MUL_MEAT_S(R,X,Y)                           \
  _FP_MUL_MEAT_1_wide(_FP_WFRACBITS_S,R,X,Y,umul_ppmm)
#define _FP_MUL_MEAT_D(R,X,Y)                           \
  _FP_MUL_MEAT_2_wide(_FP_WFRACBITS_D,R,X,Y,umul_ppmm)
#define _FP_MUL_MEAT_Q(R,X,Y)                           \
  _FP_MUL_MEAT_4_wide(_FP_WFRACBITS_Q,R,X,Y,umul_ppmm)

#define _FP_DIV_MEAT_S(R,X,Y)   _FP_DIV_MEAT_1_loop(S,R,X,Y)
#define _FP_DIV_MEAT_D(R,X,Y)   _FP_DIV_MEAT_2_udiv(D,R,X,Y)
#define _FP_DIV_MEAT_Q(R,X,Y)   _FP_DIV_MEAT_4_udiv(Q,R,X,Y)

#define _FP_NANFRAC_S           ((_FP_QNANBIT_S << 1) - 1)
#define _FP_NANFRAC_D           ((_FP_QNANBIT_D << 1) - 1), -1
#define _FP_NANFRAC_Q           ((_FP_QNANBIT_Q << 1) - 1), -1, -1, -1
#define _FP_NANSIGN_S           0
#define _FP_NANSIGN_D           0
#define _FP_NANSIGN_Q           0

#define _FP_KEEPNANFRACP 1

/* Someone please check this.  */
#define _FP_CHOOSENAN(fs, wc, R, X, Y, OP)                      \
  do {                                                          \
    if ((_FP_FRAC_HIGH_RAW_##fs(X) & _FP_QNANBIT_##fs)          \
        && !(_FP_FRAC_HIGH_RAW_##fs(Y) & _FP_QNANBIT_##fs))     \
      {                                                         \
        R##_s = Y##_s;                                          \
        _FP_FRAC_COPY_##wc(R,Y);                                \
      }                                                         \
    else                                                        \
      {                                                         \
        R##_s = X##_s;                                          \
        _FP_FRAC_COPY_##wc(R,X);                                \
      }                                                         \
    R##_c = FP_CLS_NAN;                                         \
  } while (0)

#define FP_EX_DENORM	      (1 << 5)
#define FP_EX_INVALID         (1 << 4)
#define FP_EX_OVERFLOW        (1 << 3)
#define FP_EX_UNDERFLOW       (1 << 2)
#define FP_EX_DIVZERO         (1 << 1)
#define FP_EX_INEXACT         (1 << 0)

// _fex
#define FP_HANDLE_EXCEPTIONS
#define FP_ROUNDMODE FP_RND_NEAREST

#define umul_ppmm(high_prod, low_prod, multiplier, multiplicand) high_prod = ((uint64)(multiplier) * (multiplicand)) >> 32; low_prod = ((uint64)(multiplier) * (multiplicand)) & 0xFFFFFFFF;

#if (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4))

#else

 #if _FP_W_TYPE_SIZE < 64
 /* this is just to shut the compiler up about shifts > word length -- PMM 02/1998 */
 #define __FP_CLZ(r, x) \
  do { \
    _FP_W_TYPE _t = (x); \
    r = _FP_W_TYPE_SIZE - 1; \
    if (_t > 0xffff) r -= 16; \
    if (_t > 0xffff) _t >>= 16; \
    if (_t > 0xff) r -= 8; \
    if (_t > 0xff) _t >>= 8; \
    if (_t & 0xf0) r -= 4; \
    if (_t & 0xf0) _t >>= 4; \
    if (_t & 0xc) r -= 2; \
    if (_t & 0xc) _t >>= 2; \
    if (_t & 0x2) r -= 1; \
  } while (0)
 #else /* not _FP_W_TYPE_SIZE < 64 */
 #define __FP_CLZ(r, x) \
  do { \
    _FP_W_TYPE _t = (x); \
    r = _FP_W_TYPE_SIZE - 1; \
    if (_t > 0xffffffff) r -= 32; \
    if (_t > 0xffffffff) _t >>= 32; \
    if (_t > 0xffff) r -= 16; \
    if (_t > 0xffff) _t >>= 16; \
    if (_t > 0xff) r -= 8; \
    if (_t > 0xff) _t >>= 8; \
    if (_t & 0xf0) r -= 4; \
    if (_t & 0xf0) _t >>= 4; \
    if (_t & 0xc) r -= 2; \
    if (_t & 0xc) _t >>= 2; \
    if (_t & 0x2) r -= 1; \
   } while (0)
 #endif /* not _FP_W_TYPE_SIZE < 64 */ 

#endif
