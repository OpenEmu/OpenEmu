/* explode.c -- Not copyrighted 1992 by Mark Adler
   version c7, 27 June 1992 */


/* You can do whatever you like with this source file, though I would
   prefer that if you modify it and redistribute it that you include
   comments to that effect with your name and the date.  Thank you.

   History:
   vers    date          who           what
   ----  ---------  --------------  ------------------------------------
    c1   30 Mar 92  M. Adler        explode that uses huft_build from inflate
                                    (this gives over a 70% speed improvement
                                    over the original unimplode.c, which
                                    decoded a bit at a time)
    c2    4 Apr 92  M. Adler        fixed bug for file sizes a multiple of 32k.
    c3   10 Apr 92  M. Adler        added a little memory tracking if DEBUG
    c4   11 Apr 92  M. Adler        added NOMEMCPY do kill use of memcpy()
    c5   21 Apr 92  M. Adler        added the WSIZE #define to allow reducing
                                    the 32K window size for specialized
                                    applications.
    c6   31 May 92  M. Adler        added typecasts to eliminate some warnings
    c7   27 Jun 92  G. Roelofs      added more typecasts
 */


/*
   Explode imploded (PKZIP method 6 compressed) data.  This compression
   method searches for as much of the current string of bytes (up to a length
   of ~320) in the previous 4K or 8K bytes.  If it doesn't find any matches
   (of at least length 2 or 3), it codes the next byte.  Otherwise, it codes
   the length of the matched string and its distance backwards from the
   current position.  Single bytes ("literals") are preceded by a one (a
   single bit) and are either uncoded (the eight bits go directly into the
   compressed stream for a total of nine bits) or Huffman coded with a
   supplied literal code tree.  If literals are coded, then the minimum match
   length is three, otherwise it is two.

   There are therefore four kinds of imploded streams: 8K search with coded
   literals (min match = 3), 4K search with coded literals (min match = 3),
   8K with uncoded literals (min match = 2), and 4K with uncoded literals
   (min match = 2).  The kind of stream is identified in two bits of a
   general purpose bit flag that is outside of the compressed stream.

   Distance-length pairs are always coded.  Distance-length pairs for matched
   strings are preceded by a zero bit (to distinguish them from literals) and
   are always coded.  The distance comes first and is either the low six (4K)
   or low seven (8K) bits of the distance (uncoded), followed by the high six
   bits of the distance coded.  Then the length is six bits coded (0..63 +
   min match length), and if the maximum such length is coded, then it's
   followed by another eight bits (uncoded) to be added to the coded length.
   This gives a match length range of 2..320 or 3..321 bytes.

   The literal, length, and distance codes are all represented in a slightly
   compressed form themselves.  What is sent are the lengths of the codes for
   each value, which is sufficient to construct the codes.  Each byte of the
   code representation is the code length (the low four bits representing
   1..16), and the number of values sequentially with that length (the high
   four bits also representing 1..16).  There are 256 literal code values (if
   literals are coded), 64 length code values, and 64 distance code values,
   in that order at the beginning of the compressed stream.  Each set of code
   values is preceded (redundantly) with a byte indicating how many bytes are
   in the code description that follows, in the range 1..256.

   The codes themselves are decoded using tables made by huft_build() from
   the bit lengths.  That routine and its comments are in the inflate.c
   module.
 */

#include "unz.h"      /* this must supply the slide[] (byte) array */
#include "unzipP.h"
#include <stdlib.h>

#ifndef WSIZE
#  define WSIZE 0x8000  /* window size--must be a power of two, and at least
                           8K for zip's implode method */
#endif /* !WSIZE */


struct huft {
  byte e;               /* number of extra bits or operation */
  byte b;               /* number of bits in this code or subcode */
  union {
    UWORD n;            /* literal, length base, or distance base */
    struct huft *t;     /* pointer to next level of table */
  } v;
};

/* Function prototypes */
/* routines from inflate.c */
extern unsigned hufts;
int huft_build OF((unsigned *, unsigned, unsigned, UWORD *, UWORD *,
                   struct huft **, int *));
int huft_free OF((struct huft *));
void flush OF((unsigned));

/* routines here */
int get_tree OF((unsigned *, unsigned));
int explode_lit8 OF((struct huft *, struct huft *, struct huft *,
                     int, int, int));
int explode_lit4 OF((struct huft *, struct huft *, struct huft *,
                     int, int, int));
int explode_nolit8 OF((struct huft *, struct huft *, int, int));
int explode_nolit4 OF((struct huft *, struct huft *, int, int));
int explode ();

extern file_in_zip_read_info_s *pfile_in_zip_read_info;
extern unz_s *pUnzip;

/* The implode algorithm uses a sliding 4K or 8K byte window on the
   uncompressed stream to find repeated byte strings.  This is implemented
   here as a circular buffer.  The index is updated simply by incrementing
   and then and'ing with 0x0fff (4K-1) or 0x1fff (8K-1).  Here, the 32K
   buffer of inflate is used, and it works just as well to always have
   a 32K circular buffer, so the index is anded with 0x7fff.  This is
   done to allow the window to also be used as the output buffer. */
/* This must be supplied in an external module useable like "byte slide[8192];"
   or "byte *slide;", where the latter would be malloc'ed.  In unzip, slide[]
   is actually a 32K area for use by inflate, which uses a 32K sliding window.
 */


/* Tables for length and distance */
UWORD cplen2[] = {2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
        18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34,
        35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65};
UWORD cplen3[] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18,
        19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35,
        36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
        53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66};
UWORD extra[] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        8};
UWORD cpdist4[] = {1, 65, 129, 193, 257, 321, 385, 449, 513, 577, 641, 705,
        769, 833, 897, 961, 1025, 1089, 1153, 1217, 1281, 1345, 1409, 1473,
        1537, 1601, 1665, 1729, 1793, 1857, 1921, 1985, 2049, 2113, 2177,
        2241, 2305, 2369, 2433, 2497, 2561, 2625, 2689, 2753, 2817, 2881,
        2945, 3009, 3073, 3137, 3201, 3265, 3329, 3393, 3457, 3521, 3585,
        3649, 3713, 3777, 3841, 3905, 3969, 4033};
UWORD cpdist8[] = {1, 129, 257, 385, 513, 641, 769, 897, 1025, 1153, 1281,
        1409, 1537, 1665, 1793, 1921, 2049, 2177, 2305, 2433, 2561, 2689,
        2817, 2945, 3073, 3201, 3329, 3457, 3585, 3713, 3841, 3969, 4097,
        4225, 4353, 4481, 4609, 4737, 4865, 4993, 5121, 5249, 5377, 5505,
        5633, 5761, 5889, 6017, 6145, 6273, 6401, 6529, 6657, 6785, 6913,
        7041, 7169, 7297, 7425, 7553, 7681, 7809, 7937, 8065};


/* Macros for inflate() bit peeking and grabbing.
   The usage is:

        NEEDBITS(j)
        x = b & mask_bits[j];
        DUMPBITS(j)

   where NEEDBITS makes sure that b has at least j bits in it, and
   DUMPBITS removes the bits from b.  The macros use the variable k
   for the number of bits in b.  Normally, b and k are register
   variables for speed.
 */

extern UWORD bytebuf;           /* (use the one in inflate.c) */
#define NEXTBYTE    (ReadByte(&bytebuf), bytebuf)
#define NEEDBITS(n) {while(k<(n)){b|=((ULONG)NEXTBYTE)<<k;k+=8;}}
#define DUMPBITS(n) {b>>=(n);k-=(n);}

/* HERE */
UWORD mask_bits[] = {
    0x0000,
    0x0001, 0x0003, 0x0007, 0x000f, 0x001f, 0x003f, 0x007f, 0x00ff,
    0x01ff, 0x03ff, 0x07ff, 0x0fff, 0x1fff, 0x3fff, 0x7fff, 0xffff
};
union work area;              /* see unzip.h for the definition of work */
ULONG crc32val;
ush bytebuf;
ULONG bitbuf;
int bits_left;
boolean zipeof;

int get_tree(l, n)
unsigned *l;            /* bit lengths */
unsigned n;             /* number expected */
/* Get the bit lengths for a code representation from the compressed
   stream.  If get_tree() returns 4, then there is an error in the data.
   Otherwise zero is returned. */
{
  unsigned i;           /* bytes remaining in list */
  unsigned k;           /* lengths entered */
  unsigned j;           /* number of codes */
  unsigned b;           /* bit length for those codes */


  /* get bit lengths */
  ReadByte(&bytebuf);
  i = bytebuf + 1;                      /* length/count pairs to read */
  k = 0;                                /* next code */
  do {
    ReadByte(&bytebuf);
    b = ((j = bytebuf) & 0xf) + 1;      /* bits in code (1..16) */
    j = ((j & 0xf0) >> 4) + 1;          /* codes with those bits (1..16) */
    if (k + j > n)
      return 4;                         /* don't overflow l[] */
    do {
      l[k++] = b;
    } while (--j);
  } while (--i);
  return k != n ? 4 : 0;                /* should have read n of them */
}



int explode_lit8(tb, tl, td, bb, bl, bd)
struct huft *tb, *tl, *td;      /* literal, length, and distance tables */
int bb, bl, bd;                 /* number of bits decoded by those */
/* Decompress the imploded data using coded literals and an 8K sliding
   window. */
{
  longint s;            /* bytes to decompress */
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned mb, ml, md;  /* masks for bb, bl, and bd bits */
  register ULONG b;     /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */
  unsigned u;           /* true if unflushed */


  /* explode the coded data */
  b = k = w = 0;                /* initialize bit buffer, window */
  u = 1;                        /* buffer unflushed */
  mb = mask_bits[bb];           /* precompute masks for speed */
  ml = mask_bits[bl];
  md = mask_bits[bd];
  s = pUnzip->pfile_in_zip_read->rest_read_uncompressed;
  while (s > 0)                 /* do until ucsize bytes uncompressed */
  {
    NEEDBITS(1)
    if (b & 1)                  /* then literal--decode it */
    {
      DUMPBITS(1)
      s--;
      NEEDBITS((unsigned)bb)    /* get coded literal */
      if ((e = (t = tb + ((~(unsigned)b) & mb))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      slide[w++] = (byte)t->v.n;
      if (w == WSIZE)
      {
        flush(w);
        w = u = 0;
      }
    }
    else                        /* else distance/length */
    {
      DUMPBITS(1)
      NEEDBITS(7)               /* get distance low bits */
      d = (unsigned)b & 0x7f;
      DUMPBITS(7)
      NEEDBITS((unsigned)bd)    /* get coded distance high bits */
      if ((e = (t = td + ((~(unsigned)b) & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      d = w - d - t->v.n;       /* construct offset */
      NEEDBITS((unsigned)bl)    /* get coded length */
      if ((e = (t = tl + ((~(unsigned)b) & ml))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      n = t->v.n;
      if (e)                    /* get length extra bits */
      {
        NEEDBITS(8)
        n += (unsigned)b & 0xff;
        DUMPBITS(8)
      }

      /* do the copy */
      s -= n;
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
        if (u && w <= d)
        {
          memset(slide + w, 0, e);
          w += e;
          d += e;
        }
        else
#ifndef NOMEMCPY
          if (w - d >= e)       /* (this test assumes unsigned comparison) */
          {
            memcpy(slide + w, slide + d, e);
            w += e;
            d += e;
          }
          else                  /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
            do {
              slide[w++] = slide[d++];
            } while (--e);
        if (w == WSIZE)
        {
          flush(w);
          w = u = 0;
        }
      } while (n);
    }
  }

  /* flush out slide */
  flush(w);
  return pfile_in_zip_read_info->rest_read_compressed ? 5 : 0;         /* should have read csize bytes */
}



int explode_lit4(tb, tl, td, bb, bl, bd)
struct huft *tb, *tl, *td;      /* literal, length, and distance tables */
int bb, bl, bd;                 /* number of bits decoded by those */
/* Decompress the imploded data using coded literals and a 4K sliding
   window. */
{
  longint s;            /* bytes to decompress */
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned mb, ml, md;  /* masks for bb, bl, and bd bits */
  register ULONG b;     /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */
  unsigned u;           /* true if unflushed */


  /* explode the coded data */
  b = k = w = 0;                /* initialize bit buffer, window */
  u = 1;                        /* buffer unflushed */
  mb = mask_bits[bb];           /* precompute masks for speed */
  ml = mask_bits[bl];
  md = mask_bits[bd];
  s = pUnzip->pfile_in_zip_read->rest_read_uncompressed;
  while (s > 0)                 /* do until ucsize bytes uncompressed */
  {
    NEEDBITS(1)
    if (b & 1)                  /* then literal--decode it */
    {
      DUMPBITS(1)
      s--;
      NEEDBITS((unsigned)bb)    /* get coded literal */
      if ((e = (t = tb + ((~(unsigned)b) & mb))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      slide[w++] = (byte)t->v.n;
      if (w == WSIZE)
      {
        flush(w);
        w = u = 0;
      }
    }
    else                        /* else distance/length */
    {
      DUMPBITS(1)
      NEEDBITS(6)               /* get distance low bits */
      d = (unsigned)b & 0x3f;
      DUMPBITS(6)
      NEEDBITS((unsigned)bd)    /* get coded distance high bits */
      if ((e = (t = td + ((~(unsigned)b) & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      d = w - d - t->v.n;       /* construct offset */
      NEEDBITS((unsigned)bl)    /* get coded length */
      if ((e = (t = tl + ((~(unsigned)b) & ml))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      n = t->v.n;
      if (e)                    /* get length extra bits */
      {
        NEEDBITS(8)
        n += (unsigned)b & 0xff;
        DUMPBITS(8)
      }

      /* do the copy */
      s -= n;
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
        if (u && w <= d)
        {
          memset(slide + w, 0, e);
          w += e;
          d += e;
        }
        else
#ifndef NOMEMCPY
          if (w - d >= e)       /* (this test assumes unsigned comparison) */
          {
            memcpy(slide + w, slide + d, e);
            w += e;
            d += e;
          }
          else                  /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
            do {
              slide[w++] = slide[d++];
            } while (--e);
        if (w == WSIZE)
        {
          flush(w);
          w = u = 0;
        }
      } while (n);
    }
  }

  /* flush out slide */
  flush(w);
  return pfile_in_zip_read_info->rest_read_compressed ? 5 : 0;         /* should have read csize bytes */
}



int explode_nolit8(tl, td, bl, bd)
struct huft *tl, *td;   /* length and distance decoder tables */
int bl, bd;             /* number of bits decoded by tl[] and td[] */
/* Decompress the imploded data using uncoded literals and an 8K sliding
   window. */
{
  longint s;            /* bytes to decompress */
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned ml, md;      /* masks for bl and bd bits */
  register ULONG b;     /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */
  unsigned u;           /* true if unflushed */


  /* explode the coded data */
  b = k = w = 0;                /* initialize bit buffer, window */
  u = 1;                        /* buffer unflushed */
  ml = mask_bits[bl];           /* precompute masks for speed */
  md = mask_bits[bd];
  s = pUnzip->pfile_in_zip_read->rest_read_uncompressed;
  while (s > 0)                 /* do until ucsize bytes uncompressed */
  {
    NEEDBITS(1)
    if (b & 1)                  /* then literal--get eight bits */
    {
      DUMPBITS(1)
      s--;
      NEEDBITS(8)
      slide[w++] = (byte)b;
      if (w == WSIZE)
      {
        flush(w);
        w = u = 0;
      }
      DUMPBITS(8)
    }
    else                        /* else distance/length */
    {
      DUMPBITS(1)
      NEEDBITS(7)               /* get distance low bits */
      d = (unsigned)b & 0x7f;
      DUMPBITS(7)
      NEEDBITS((unsigned)bd)    /* get coded distance high bits */
      if ((e = (t = td + ((~(unsigned)b) & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      d = w - d - t->v.n;       /* construct offset */
      NEEDBITS((unsigned)bl)    /* get coded length */
      if ((e = (t = tl + ((~(unsigned)b) & ml))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      n = t->v.n;
      if (e)                    /* get length extra bits */
      {
        NEEDBITS(8)
        n += (unsigned)b & 0xff;
        DUMPBITS(8)
      }

      /* do the copy */
      s -= n;
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
        if (u && w <= d)
        {
          memset(slide + w, 0, e);
          w += e;
          d += e;
        }
        else
#ifndef NOMEMCPY
          if (w - d >= e)       /* (this test assumes unsigned comparison) */
          {
            memcpy(slide + w, slide + d, e);
            w += e;
            d += e;
          }
          else                  /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
            do {
              slide[w++] = slide[d++];
            } while (--e);
        if (w == WSIZE)
        {
          flush(w);
          w = u = 0;
        }
      } while (n);
    }
  }

  /* flush out slide */
  flush(w);
  return pfile_in_zip_read_info->rest_read_compressed ? 5 : 0;         /* should have read csize bytes */
}



int explode_nolit4(tl, td, bl, bd)
struct huft *tl, *td;   /* length and distance decoder tables */
int bl, bd;             /* number of bits decoded by tl[] and td[] */
/* Decompress the imploded data using uncoded literals and a 4K sliding
   window. */
{
  longint s;            /* bytes to decompress */
  register unsigned e;  /* table entry flag/number of extra bits */
  unsigned n, d;        /* length and index for copy */
  unsigned w;           /* current window position */
  struct huft *t;       /* pointer to table entry */
  unsigned ml, md;      /* masks for bl and bd bits */
  register ULONG b;     /* bit buffer */
  register unsigned k;  /* number of bits in bit buffer */
  unsigned u;           /* true if unflushed */


  /* explode the coded data */
  b = k = w = 0;                /* initialize bit buffer, window */
  u = 1;                        /* buffer unflushed */
  ml = mask_bits[bl];           /* precompute masks for speed */
  md = mask_bits[bd];
  s = pUnzip->pfile_in_zip_read->rest_read_uncompressed;
  while (s > 0)                 /* do until ucsize bytes uncompressed */
  {
    NEEDBITS(1)
    if (b & 1)                  /* then literal--get eight bits */
    {
      DUMPBITS(1)
      s--;
      NEEDBITS(8)
      slide[w++] = (byte)b;
      if (w == WSIZE)
      {
        flush(w);
        w = u = 0;
      }
      DUMPBITS(8)
    }
    else                        /* else distance/length */
    {
      DUMPBITS(1)
      NEEDBITS(6)               /* get distance low bits */
      d = (unsigned)b & 0x3f;
      DUMPBITS(6)
      NEEDBITS((unsigned)bd)    /* get coded distance high bits */
      if ((e = (t = td + ((~(unsigned)b) & md))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      d = w - d - t->v.n;       /* construct offset */
      NEEDBITS((unsigned)bl)    /* get coded length */
      if ((e = (t = tl + ((~(unsigned)b) & ml))->e) > 16)
        do {
          if (e == 99)
            return 1;
          DUMPBITS(t->b)
          e -= 16;
          NEEDBITS(e)
        } while ((e = (t = t->v.t + ((~(unsigned)b) & mask_bits[e]))->e) > 16);
      DUMPBITS(t->b)
      n = t->v.n;
      if (e)                    /* get length extra bits */
      {
        NEEDBITS(8)
        n += (unsigned)b & 0xff;
        DUMPBITS(8)
      }

      /* do the copy */
      s -= n;
      do {
        n -= (e = (e = WSIZE - ((d &= WSIZE-1) > w ? d : w)) > n ? n : e);
        if (u && w <= d)
        {
          memset(slide + w, 0, e);
          w += e;
          d += e;
        }
        else
#ifndef NOMEMCPY
          if (w - d >= e)       /* (this test assumes unsigned comparison) */
          {
            memcpy(slide + w, slide + d, e);
            w += e;
            d += e;
          }
          else                  /* do it slow to avoid memcpy() overlap */
#endif /* !NOMEMCPY */
            do {
              slide[w++] = slide[d++];
            } while (--e);
        if (w == WSIZE)
        {
          flush(w);
          w = u = 0;
        }
      } while (n);
    }
  }

  /* flush out slide */
  flush(w);
  return pfile_in_zip_read_info->rest_read_compressed ? 5 : 0;         /* should have read csize bytes */
}



int explode ()
/* Explode an imploded compressed stream.  Based on the general purpose
   bit flag, decide on coded or uncoded literals, and an 8K or 4K sliding
   window.  Construct the literal (if any), length, and distance codes and
   the tables needed to decode them (using huft_build() from inflate.c),
   and call the appropriate routine for the type of data in the remainder
   of the stream.  The four routines are nearly identical, differing only
   in whether the literal is decoded or simply read in, and in how many
   bits are read in, uncoded, for the low distance bits. */
{
  unsigned r;           /* return codes */
  struct huft *tb;      /* literal code table */
  struct huft *tl;      /* length code table */
  struct huft *td;      /* distance code table */
  int bb;               /* bits for tb */
  int bl;               /* bits for tl */
  int bd;               /* bits for td */
  unsigned l[256];      /* bit lengths for codes */


  /* Tune base table sizes.  Note: I thought that to truly optimize speed,
     I would have to select different bl, bd, and bb values for different
     compressed file sizes.  I was suprised to find out the the values of
     7, 7, and 9 worked best over a very wide range of sizes, except that
     bd = 8 worked marginally better for large compressed sizes. */
  bl = 7;
  bd = pUnzip->pfile_in_zip_read->rest_read_compressed > 200000L ? 8 : 7;


  /* With literal tree--minimum match length is 3 */
  hufts = 0;                    /* initialze huft's malloc'ed */
  if (pUnzip->cur_file_info.flag & 4)
  {
    bb = 9;                     /* base table size for literals */
    if ((r = get_tree(l, 256)) != 0)
      return r;
    if ((r = huft_build(l, 256, 256, NULL, NULL, &tb, &bb)) != 0)
    {
      if (r == 1)
        huft_free(tb);
      return r;
    }
    if ((r = get_tree(l, 64)) != 0)
      return r;
    if ((r = huft_build(l, 64, 0, cplen3, extra, &tl, &bl)) != 0)
    {
      if (r == 1)
        huft_free(tl);
      huft_free(tb);
      return r;
    }
    if ((r = get_tree(l, 64)) != 0)
      return r;
    if (pUnzip->cur_file_info.flag & 2)      /* true if 8K */
    {
      if ((r = huft_build(l, 64, 0, cpdist8, extra, &td, &bd)) != 0)
      {
        if (r == 1)
          huft_free(td);
        huft_free(tl);
        huft_free(tb);
        return r;
      }
      r = explode_lit8(tb, tl, td, bb, bl, bd);
    }
    else                                        /* else 4K */
    {
      if ((r = huft_build(l, 64, 0, cpdist4, extra, &td, &bd)) != 0)
      {
        if (r == 1)
          huft_free(td);
        huft_free(tl);
        huft_free(tb);
        return r;
      }
      r = explode_lit4(tb, tl, td, bb, bl, bd);
    }
    huft_free(td);
    huft_free(tl);
    huft_free(tb);
  }
  else


  /* No literal tree--minimum match length is 2 */
  {
    if ((r = get_tree(l, 64)) != 0)
      return r;
    if ((r = huft_build(l, 64, 0, cplen2, extra, &tl, &bl)) != 0)
    {
      if (r == 1)
        huft_free(tl);
      return r;
    }
    if ((r = get_tree(l, 64)) != 0)
      return r;
    if (pUnzip->cur_file_info.flag & 2)      /* true if 8K */
    {
      if ((r = huft_build(l, 64, 0, cpdist8, extra, &td, &bd)) != 0)
      {
        if (r == 1)
          huft_free(td);
        huft_free(tl);
        return r;
      }
      r = explode_nolit8(tl, td, bl, bd);
    }
    else                                        /* else 4K */
    {
      if ((r = huft_build(l, 64, 0, cpdist4, extra, &td, &bd)) != 0)
      {
        if (r == 1)
          huft_free(td);
        huft_free(tl);
        return r;
      }
      r = explode_nolit4(tl, td, bl, bd);
    }
    huft_free(td);
    huft_free(tl);
  }
#ifdef DEBUG
  fprintf(stderr, "<%u > ", hufts);
#endif /* DEBUG */
  return r;
}


int ReadByte(x)
    UWORD *x;
{
    /*
     * read a byte; return 8 if byte available, 0 if not
     */

    if (pfile_in_zip_read_info->stream.avail_in == 0)
    {
	unsigned int            uReadThis = UNZ_BUFSIZE;

	if (pfile_in_zip_read_info->rest_read_compressed <= 0)
	    return (0);

	if (pfile_in_zip_read_info->rest_read_compressed < uReadThis)
	    uReadThis = (uInt) pfile_in_zip_read_info->rest_read_compressed;
	if (uReadThis == 0)
	    return UNZ_EOF;
	if (fseek (pfile_in_zip_read_info->file,
		   pfile_in_zip_read_info->pos_in_zipfile +
	pfile_in_zip_read_info->byte_before_the_zipfile, SEEK_SET) != 0)
	    return UNZ_ERRNO;
	if (fread (pfile_in_zip_read_info->read_buffer, uReadThis, 1,
		   pfile_in_zip_read_info->file) != 1)
	    return UNZ_ERRNO;
	pfile_in_zip_read_info->pos_in_zipfile += uReadThis;

	pfile_in_zip_read_info->rest_read_compressed -= uReadThis;

	pfile_in_zip_read_info->stream.next_in =
	    (Bytef *) pfile_in_zip_read_info->read_buffer;
	pfile_in_zip_read_info->stream.avail_in = (uInt) uReadThis;
    }

    *x = *pfile_in_zip_read_info->stream.next_in++;
    pfile_in_zip_read_info->stream.avail_in--;

    return 8;
}

/* If BMAX needs to be larger than 16, then h and x[] should be ulg. */
#define BMAX 16         /* maximum bit length of any code (16 for explode) */
#define N_MAX 288       /* maximum number of codes in any set */

unsigned hufts;         /* track memory usage */


int huft_build(b, n, s, d, e, t, m)
unsigned *b;            /* code lengths in bits (all assumed <= BMAX) */
unsigned n;             /* number of codes (assumed <= N_MAX) */
unsigned s;             /* number of simple-valued codes (0..s-1) */
ush *d;                 /* list of base values for non-simple codes */
ush *e;                 /* list of extra bits for non-simple codes */
struct huft **t;        /* result: starting table */
int *m;                 /* maximum lookup bits, returns actual */
/* Given a list of code lengths and a maximum table size, make a set of
   tables to decode that set of codes.  Return zero on success, one if
   the given code set is incomplete (the tables are still built in this
   case), two if the input is invalid (all zero length codes or an
   oversubscribed set of lengths), and three if not enough memory. */
{
  unsigned a;                   /* counter for codes of length k */
  unsigned c[BMAX+1];           /* bit length count table */
  unsigned f;                   /* i repeats in table every f entries */
  int g;                        /* maximum code length */
  int h;                        /* table level */
  register unsigned i;          /* counter, current code */
  register unsigned j;          /* counter */
  register int k;               /* number of bits in current code */
  int l;                        /* bits per table (returned in m) */
  register unsigned *p;         /* pointer into c[], b[], or v[] */
  register struct huft *q;      /* points to current table */
  struct huft r;                /* table entry for structure assignment */
  struct huft *u[BMAX];         /* table stack */
  unsigned v[N_MAX];            /* values in order of bit length */
  register int w;               /* bits before this table == (l * h) */
  unsigned x[BMAX+1];           /* bit offsets, then code stack */
  unsigned *xp;                 /* pointer into x */
  int y;                        /* number of dummy codes added */
  unsigned z;                   /* number of entries in current table */


  /* Generate counts for each bit length */
  memset(c, 0, sizeof(c));
  p = b;  i = n;
  do {
    c[*p++]++;                  /* assume all entries <= BMAX */
  } while (--i);
  if (c[0] == n)                /* null input--all zero length codes */
  {
    *t = (struct huft *)NULL;
    *m = 0;
    return 0;
  }


  /* Find minimum and maximum length, bound *m by those */
  l = *m;
  for (j = 1; j <= BMAX; j++)
    if (c[j])
      break;
  k = j;                        /* minimum code length */
  if ((unsigned)l < j)
    l = j;
  for (i = BMAX; i; i--)
    if (c[i])
      break;
  g = i;                        /* maximum code length */
  if ((unsigned)l > i)
    l = i;
  *m = l;


  /* Adjust last length count to fill out codes, if needed */
  for (y = 1 << j; j < i; j++, y <<= 1)
    if ((y -= c[j]) < 0)
      return 2;                 /* bad input: more codes than bits */
  if ((y -= c[i]) < 0)
    return 2;
  c[i] += y;


  /* Generate starting offsets into the value table for each length */
  x[1] = j = 0;
  p = c + 1;  xp = x + 2;
  while (--i) {                 /* note that i == g from above */
    *xp++ = (j += *p++);
  }


  /* Make a table of values in order of bit lengths */
  p = b;  i = 0;
  do {
    if ((j = *p++) != 0)
      v[x[j]++] = i;
  } while (++i < n);


  /* Generate the Huffman codes and for each, make the table entries */
  x[0] = i = 0;                 /* first Huffman code is zero */
  p = v;                        /* grab values in bit order */
  h = -1;                       /* no tables yet--level -1 */
  w = -l;                       /* bits decoded == (l * h) */
  u[0] = (struct huft *)NULL;   /* just to keep compilers happy */
  q = (struct huft *)NULL;      /* ditto */
  z = 0;                        /* ditto */

  /* go through the bit lengths (k already is bits in shortest code) */
  for (; k <= g; k++)
  {
    a = c[k];
    while (a--)
    {
      /* here i is the Huffman code of length k bits for value *p */
      /* make tables up to required level */
      while (k > w + l)
      {
        h++;
        w += l;                 /* previous table always l bits */

        /* compute minimum size table less than or equal to l bits */
        z = (z = g - w) > (unsigned)l ? (unsigned)l : z;  /* upper limit on table size */
        if ((f = 1 << (j = k - w)) > a + 1)     /* try a k-w bit table */
        {                       /* too few codes for k-w bit table */
          f -= a + 1;           /* deduct codes from patterns left */
          xp = c + k;
          while (++j < z)       /* try smaller tables up to z bits */
          {
            if ((f <<= 1) <= *++xp)
              break;            /* enough codes to use up j bits */
            f -= *xp;           /* else deduct codes from patterns */
          }
        }
        z = 1 << j;             /* table entries for j-bit table */

        /* allocate and link in new table */
        if ((q = (struct huft *)malloc((z + 1)*sizeof(struct huft))) ==
            (struct huft *)NULL)
        {
          if (h)
            huft_free(u[0]);
          return 3;             /* not enough memory */
        }
        hufts += z + 1;         /* track memory usage */
        *t = q + 1;             /* link to list for huft_free() */
        *(t = &(q->v.t)) = (struct huft *)NULL;
        u[h] = ++q;             /* table starts after link */

        /* connect to last table, if there is one */
        if (h)
        {
          x[h] = i;             /* save pattern for backing up */
          r.b = (uch)l;         /* bits to dump before this table */
          r.e = (uch)(16 + j);  /* bits in this table */
          r.v.t = q;            /* pointer to this table */
          j = i >> (w - l);     /* (get around Turbo C bug) */
          u[h-1][j] = r;        /* connect to last table */
        }
      }

      /* set up table entry in r */
      r.b = (uch)(k - w);
      if (p >= v + n)
        r.e = 99;               /* out of values--invalid code */
      else if (*p < s)
      {
        r.e = (uch)(*p < 256 ? 16 : 15);    /* 256 is end-of-block code */
        r.v.n = *p++;           /* simple code is just the value */
      }
      else
      {
        r.e = (uch)e[*p - s];   /* non-simple--look up in lists */
        r.v.n = d[*p++ - s];
      }

      /* fill code-like entries with r */
      f = 1 << (k - w);
      for (j = i >> w; j < z; j += f)
        q[j] = r;

      /* backwards increment the k-bit code i */
      for (j = 1 << (k - 1); i & j; j >>= 1)
        i ^= j;
      i ^= j;

      /* backup over finished tables */
      while ((i & ((1 << w) - 1)) != x[h])
      {
        h--;                    /* don't need to update q */
        w -= l;
      }
    }
  }


  /* Return true (1) if we were given an incomplete table */
  return y != 0 && g != 1;
}


int huft_free(t)
struct huft *t;         /* table to free */
/* Free the malloc'ed tables built by huft_build(), which makes a linked
   list of the tables it made, with the links in a dummy first entry of
   each table. */
{
  register struct huft *p, *q;


  /* Go through linked list, freeing from the malloced (t[-1]) address. */
  p = t;
  while (p != (struct huft *)NULL)
  {
    q = (--p)->v.t;
    free(p);
    p = q;
  }
  return 0;
}

void flush(w)
unsigned w;             /* number of bytes to flush */
/* Do the equivalent of OUTB for the bytes slide[0..w-1]. */
{
  memmove (pfile_in_zip_read_info->stream.next_out, slide, w);
  pfile_in_zip_read_info->crc32 = crc32 (pfile_in_zip_read_info->crc32,
					 pfile_in_zip_read_info->stream.next_out,
					 w);
  pfile_in_zip_read_info->stream.next_out += w;
  pfile_in_zip_read_info->stream.avail_out -= w;
  pfile_in_zip_read_info->stream.total_out += w;
}

void flush_stack(w)
unsigned w;             /* number of bytes to flush */
/* Do the equivalent of OUTB for the bytes slide[0..w-1]. */
{
  memmove (pfile_in_zip_read_info->stream.next_out, stack, w);
  pfile_in_zip_read_info->crc32 = crc32 (pfile_in_zip_read_info->crc32,
					 pfile_in_zip_read_info->stream.next_out,
					 w);
  pfile_in_zip_read_info->stream.next_out += w;
  pfile_in_zip_read_info->stream.avail_out -= w;
  pfile_in_zip_read_info->stream.total_out += w;
}

/****************************/
/* Function FillBitBuffer() */
/****************************/

int FillBitBuffer()
{
    /*
     * Fill bitbuf, which is 32 bits.  This function is only used by the
     * READBIT and PEEKBIT macros (which are used by all of the uncompression
     * routines).
     */
    UWORD temp;

    zipeof = 1;
    while (bits_left < 25 && ReadByte(&temp) == 8)
    {
      bitbuf |= (ULONG)temp << bits_left;
      bits_left += 8;
      zipeof = 0;
    }
    return 0;
}

