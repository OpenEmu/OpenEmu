/*---------------------------------------------------------------------------

  unreduce.c

  The Reducing algorithm is actually a combination of two distinct algorithms.
  The first algorithm compresses repeated byte sequences, and the second al-
  gorithm takes the compressed stream from the first algorithm and applies a
  probabilistic compression method.

  ---------------------------------------------------------------------------*/


#include "unz.h"
#include "unzipP.h"

/**************************************/
/*  UnReduce Defines, Typedefs, etc.  */
/**************************************/

#define DLE    144

typedef byte f_array[64];       /* for followers[256][64] */

static void LoadFollowers ();
void flush OF((unsigned));      /* routine from inflate.c */

extern file_in_zip_read_info_s *pfile_in_zip_read_info;
extern unz_s *pUnzip;

/*******************************/
/*  UnReduce Global Variables  */
/*******************************/

#if (defined(MACOS) || defined(MTS))
   f_array *followers;     /* shared work space */
#else
   f_array *followers = (f_array *) (slide + 0x4000);
#endif

byte Slen[256];
int factor;

int L_table[] =
{0, 0x7f, 0x3f, 0x1f, 0x0f};

int D_shift[] =
{0, 0x07, 0x06, 0x05, 0x04};
int D_mask[] =
{0, 0x01, 0x03, 0x07, 0x0f};

int B_table[] =
{8, 1, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4, 5,
 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 6, 6, 6,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6,
 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 7, 7, 7, 7, 7, 7, 7,
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7,
 7, 7, 7, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8,
 8, 8, 8, 8};





/*************************/
/*  Function unReduce()  */
/*************************/

void unReduce()   /* expand probabilistically reduced data */
{
    register int lchar = 0;
    int nchar;
    int ExState = 0;
    int V = 0;
    int Len = 0;
    longint s = pUnzip->pfile_in_zip_read->rest_read_compressed;
    unsigned w = 0;      /* position in output window slide[] */
    unsigned u = 1;      /* true if slide[] unflushed */


#if (defined(MACOS) || defined(MTS))
    followers = (f_array *) (slide + 0x4000);
#endif

    factor = pUnzip->cur_file_info.compression_method;
    LoadFollowers();

    while (s > 0 /* && (!zipeof) */) {
        if (Slen[lchar] == 0)
            READBIT(8, nchar)   /* ; */
        else {
            READBIT(1, nchar);
            if (nchar != 0)
                READBIT(8, nchar)       /* ; */
            else {
                int follower;
                int bitsneeded = B_table[Slen[lchar]];
                READBIT(bitsneeded, follower);
                nchar = followers[lchar][follower];
            }
        }
        /* expand the resulting byte */
        switch (ExState) {

        case 0:
            if (nchar != DLE) {
                s--;
                slide[w++] = (byte) nchar;
                if (w == 0x4000) {
                    flush(w);
                    w = u = 0;
                }
            }
            else
                ExState = 1;
            break;

        case 1:
            if (nchar != 0) {
                V = nchar;
                Len = V & L_table[factor];
                if (Len == L_table[factor])
                    ExState = 2;
                else
                    ExState = 3;
            } else {
                s--;
                slide[w++] = DLE;
                if (w == 0x4000)
                {
                  flush(w);
                  w = u = 0;
                }
                ExState = 0;
            }
            break;

        case 2:{
                Len += nchar;
                ExState = 3;
            }
            break;

        case 3:{
                register unsigned e;
                register unsigned n = Len + 3;
                register unsigned d = w - ((((V >> D_shift[factor]) &
                               D_mask[factor]) << 8) + nchar + 1);

                s -= n;
                do {
                  n -= (e = (e = 0x4000 - ((d &= 0x3fff) > w ? d : w)) > n ?
                        n : e);
                  if (u && w <= d)
                  {
                    memset(slide + w, 0, e);
                    w += e;
                    d += e;
                  }
                  else
                    if (w - d < e)      /* (assume unsigned comparison) */
                      do {              /* slow to avoid memcpy() overlap */
                        slide[w++] = slide[d++];
                      } while (--e);
                    else
                    {
                      memcpy(slide + w, slide + d, e);
                      w += e;
                      d += e;
                    }
                  if (w == 0x4000)
                  {
                    flush(w);
                    w = u = 0;
                  }
                } while (n);

                ExState = 0;
            }
            break;
        }

        /* store character for next iteration */
        lchar = nchar;
    }

    /* flush out slide */
    flush(w);
}





/******************************/
/*  Function LoadFollowers()  */
/******************************/

static void LoadFollowers()
{
    register int x;
    register int i;

    for (x = 255; x >= 0; x--) {
        READBIT(6, Slen[x]);
        for (i = 0; (byte) i < Slen[x]; i++) {
            READBIT(8, followers[x][i]);
        }
    }
}
