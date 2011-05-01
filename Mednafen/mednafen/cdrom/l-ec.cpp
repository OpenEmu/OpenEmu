/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2007 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
 *
 *  The Reed-Solomon error correction draws a lot of inspiration - and even code -
 *  from Phil Karn's excellent Reed-Solomon library: http://www.ka9q.net/code/fec/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA,
 *  or direct your browser at http://www.gnu.org.
 */

#include "dvdisaster.h"

#include "galois-inlines.h"

#define MIN(a, b)  (((a) < (b)) ? (a) : (b))

/***
 *** Mapping between cd frame and parity vectors
 ***/

/*
 * Mapping of frame bytes to P/Q Vectors
 */

int PToByteIndex(int p, int i)
{  return 12 + p + i*86;
}

void ByteIndexToP(int b, int *p, int *i)
{  *p = (b-12)%86;
   *i = (b-12)/86;
}

int QToByteIndex(int q, int i)
{  int offset = 12 + (q & 1);

   if(i == 43) return 2248+q;
   if(i == 44) return 2300+q;

   q&=~1;
   return offset + (q*43 + i*88) % 2236;
}

void ByteIndexToQ(int b, int *q, int *i)
{ int x,y,offset;
 
  if(b >= 2300) 
  {  *i = 44;
     *q = (b-2300);
     return;
  }

  if(b >= 2248) 
  {  *i = 43;
     *q = (b-2248);
     return;
  }

  offset = b&1;
  b  = (b-12)/2;
  x  = b/43;
  y  = (b-(x*43))%26;  
  *i = b-(x*43);
  *q = 2*((x+26-y)%26)+offset;
}

/*
 * There are 86 vectors of P-parity, yielding a RS(26,24) code.
 */

void GetPVector(unsigned char *frame, unsigned char *data, int n)
{  int i;
   int w_idx = n+12;

   for(i=0; i<26; i++, w_idx+=86)
     data[i] = frame[w_idx];
}

void SetPVector(unsigned char *frame, unsigned char *data, int n)
{  int i;
   int w_idx = n+12;

   for(i=0; i<26; i++, w_idx+=86)
     frame[w_idx] = data[i];
}

void FillPVector(unsigned char *frame, unsigned char data, int n)
{  int i;
   int w_idx = n+12;

   for(i=0; i<26; i++, w_idx+=86)
     frame[w_idx] = data;
}

void OrPVector(unsigned char *frame, unsigned char value, int n)
{  int i;
   int w_idx = n+12;

   for(i=0; i<26; i++, w_idx+=86)
       frame[w_idx] |= value;
}

void AndPVector(unsigned char *frame, unsigned char value, int n)
{  int i;
   int w_idx = n+12;

   for(i=0; i<26; i++, w_idx+=86)
       frame[w_idx] &= value;
}

/*
 * There are 52 vectors of Q-parity, yielding a RS(45,43) code.
 */

void GetQVector(unsigned char *frame, unsigned char *data, int n)
{  int offset = 12 + (n & 1);
   int w_idx  = (n&~1) * 43;
   int i;

   for(i=0; i<43; i++, w_idx+=88)
     data[i] = frame[(w_idx % 2236) + offset];

   data[43] = frame[2248 + n];
   data[44] = frame[2300 + n];
}

void SetQVector(unsigned char *frame, unsigned char *data, int n)
{  int offset = 12 + (n & 1);
   int w_idx  = (n&~1) * 43;
   int i;

   for(i=0; i<43; i++, w_idx+=88)
     frame[(w_idx % 2236) + offset] = data[i];

   frame[2248 + n] = data[43];
   frame[2300 + n] = data[44];
}

void FillQVector(unsigned char *frame, unsigned char data, int n)
{  int offset = 12 + (n & 1);
   int w_idx  = (n&~1) * 43;
   int i;

   for(i=0; i<43; i++, w_idx+=88)
     frame[(w_idx % 2236) + offset] = data;

   frame[2248 + n] = data;
   frame[2300 + n] = data;
}

void OrQVector(unsigned char *frame, unsigned char data, int n)
{  int offset = 12 + (n & 1);
   int w_idx  = (n&~1) * 43;
   int i;

   for(i=0; i<43; i++, w_idx+=88)
     frame[(w_idx % 2236) + offset] |= data;

   frame[2248 + n] |= data;
   frame[2300 + n] |= data;
}

void AndQVector(unsigned char *frame, unsigned char data, int n)
{  int offset = 12 + (n & 1);
   int w_idx  = (n&~1) * 43;
   int i;

   for(i=0; i<43; i++, w_idx+=88)
     frame[(w_idx % 2236) + offset] &= data;

   frame[2248 + n] &= data;
   frame[2300 + n] &= data;
}

/***
 *** C2 error counting
 ***/

int CountC2Errors(unsigned char *frame)
{  int i,count = 0;
   frame += 2352;

   for(i=0; i<294; i++, frame++)
   {  if(*frame & 0x01) count++;
      if(*frame & 0x02) count++;
      if(*frame & 0x04) count++;
      if(*frame & 0x08) count++;
      if(*frame & 0x10) count++;
      if(*frame & 0x20) count++;
      if(*frame & 0x40) count++;
      if(*frame & 0x80) count++;
   }

   return count;
}

/***
 *** L-EC error correction for CD raw data sectors
 ***/

/*
 * These could be used from ReedSolomonTables,
 * but hardcoding them is faster.
 */

#define NROOTS 2
#define LEC_FIRST_ROOT 0 //GF_ALPHA0
#define LEC_PRIM_ELEM 1
#define LEC_PRIMTH_ROOT 1

/*
 * Calculate the error syndrome
 */

int DecodePQ(ReedSolomonTables *rt, unsigned char *data, int padding,
	     int *erasure_list, int erasure_count)
{  GaloisTables *gt = rt->gfTables;
   int syndrome[NROOTS];
   int lambda[NROOTS+1];
   int omega[NROOTS+1];
   int b[NROOTS+1];
   int reg[NROOTS+1];
   int root[NROOTS];
   int loc[NROOTS];
   int syn_error;
   int deg_lambda,lambda_roots;
   int deg_omega;
   int shortened_size = GF_FIELDMAX - padding;
   int corrected = 0;
   int i,j,k;
   int r,el;
  
   /*** Form the syndromes: Evaluate data(x) at roots of g(x) */

   for(i=0; i<NROOTS; i++)
     syndrome[i] = data[0];

   for(j=1; j<shortened_size; j++)
     for(i=0; i<NROOTS; i++)
       if(syndrome[i] == 0) 
             syndrome[i] = data[j];
        else syndrome[i] = data[j] ^ gt->alphaTo[mod_fieldmax(gt->indexOf[syndrome[i]] 
							      + (LEC_FIRST_ROOT+i)*LEC_PRIM_ELEM)];

   /*** Convert syndrome to index form, check for nonzero condition. */

   syn_error = 0;
   for(i=0; i<NROOTS; i++)
   {  syn_error |= syndrome[i];
      syndrome[i] = gt->indexOf[syndrome[i]];
   }

   /*** If the syndrome is zero, everything is fine. */

   if(!syn_error)
     return 0;

   /*** Initialize lambda to be the erasure locator polynomial */

   lambda[0] = 1;
   lambda[1] = lambda[2] = 0;

   erasure_list[0] += padding;
   erasure_list[1] += padding;

   if(erasure_count > 2)  /* sanity check */
     erasure_count = 0;

   if(erasure_count > 0)
   {  lambda[1] = gt->alphaTo[mod_fieldmax(LEC_PRIM_ELEM*(GF_FIELDMAX-1-erasure_list[0]))];

      for(i=1; i<erasure_count; i++) 
      {  int u = mod_fieldmax(LEC_PRIM_ELEM*(GF_FIELDMAX-1-erasure_list[i]));

         for(j=i+1; j>0; j--) 
	 {  int tmp = gt->indexOf[lambda[j-1]];
	  
            if(tmp != GF_ALPHA0)
	      lambda[j] ^= gt->alphaTo[mod_fieldmax(u + tmp)];
	}
      }
   }	

   for(i=0; i<NROOTS+1; i++)
     b[i] = gt->indexOf[lambda[i]];
  
   /*** Berlekamp-Massey algorithm to determine error+erasure locator polynomial */

   r = erasure_count;   /* r is the step number */
   el = erasure_count;

   /* Compute discrepancy at the r-th step in poly-form */

   while(++r <= NROOTS) 
   {  int discr_r = 0;

      for(i=0; i<r; i++)
	if((lambda[i] != 0) && (syndrome[r-i-1] != GF_ALPHA0))
	      discr_r ^= gt->alphaTo[mod_fieldmax(gt->indexOf[lambda[i]] + syndrome[r-i-1])];

      discr_r = gt->indexOf[discr_r];

      if(discr_r == GF_ALPHA0) 
      {  /* B(x) = x*B(x) */
	 memmove(b+1, b, NROOTS*sizeof(b[0]));
	 b[0] = GF_ALPHA0;
      } 
      else 
      {  int t[NROOTS+1];

	 /* T(x) = lambda(x) - discr_r*x*b(x) */
	 t[0] = lambda[0];
	 for(i=0; i<NROOTS; i++) 
	 {  if(b[i] != GF_ALPHA0)
	         t[i+1] = lambda[i+1] ^ gt->alphaTo[mod_fieldmax(discr_r + b[i])];
	    else t[i+1] = lambda[i+1];
	 }

	 if(2*el <= r+erasure_count-1) 
	 {  el = r + erasure_count - el;

	    /* B(x) <-- inv(discr_r) * lambda(x) */
	    for(i=0; i<=NROOTS; i++)
	      b[i] = (lambda[i] == 0) ? GF_ALPHA0 
		                      : mod_fieldmax(gt->indexOf[lambda[i]] - discr_r + GF_FIELDMAX);
	 } 
	 else 
	 {  /* 2 lines below: B(x) <-- x*B(x) */
	    memmove(b+1, b, NROOTS*sizeof(b[0]));
	    b[0] = GF_ALPHA0;
	 }

	 memcpy(lambda, t, (NROOTS+1)*sizeof(t[0]));
      }
   }

   /*** Convert lambda to index form and compute deg(lambda(x)) */

   deg_lambda = 0;
   for(i=0; i<NROOTS+1; i++)
   {  lambda[i] = gt->indexOf[lambda[i]];
      if(lambda[i] != GF_ALPHA0)
	deg_lambda = i;
   }

   /*** Find roots of the error+erasure locator polynomial by Chien search */

   memcpy(reg+1, lambda+1, NROOTS*sizeof(reg[0]));
   lambda_roots = 0;		/* Number of roots of lambda(x) */

   for(i=1, k=LEC_PRIMTH_ROOT-1; i<=GF_FIELDMAX; i++, k=mod_fieldmax(k+LEC_PRIMTH_ROOT))
   {  int q=1; /* lambda[0] is always 0 */

      for(j=deg_lambda; j>0; j--)
      {  if(reg[j] != GF_ALPHA0) 
	 {  reg[j] = mod_fieldmax(reg[j] + j);
	    q ^= gt->alphaTo[reg[j]];
	 }
      }

      if(q != 0) continue; /* Not a root */

      /* store root in index-form and the error location number */

      root[lambda_roots] = i;
      loc[lambda_roots] = k;

      /* If we've already found max possible roots, abort the search to save time */

      if(++lambda_roots == deg_lambda) break;
   }

   /* deg(lambda) unequal to number of roots => uncorrectable error detected
      This is not reliable for very small numbers of roots, e.g. nroots = 2 */

   if(deg_lambda != lambda_roots)
   {  return -1;
   } 

   /* Compute err+eras evaluator poly omega(x) = syn(x)*lambda(x) 
      (modulo x**nroots). in index form. Also find deg(omega). */

   deg_omega = deg_lambda-1;

   for(i=0; i<=deg_omega; i++)
   {  int tmp = 0;

      for(j=i; j>=0; j--)
      {  if((syndrome[i - j] != GF_ALPHA0) && (lambda[j] != GF_ALPHA0))
	  tmp ^= gt->alphaTo[mod_fieldmax(syndrome[i - j] + lambda[j])];
      }

      omega[i] = gt->indexOf[tmp];
   }

   /* Compute error values in poly-form. 
      num1 = omega(inv(X(l))), 
      num2 = inv(X(l))**(FIRST_ROOT-1) and 
      den  = lambda_pr(inv(X(l))) all in poly-form. */

   for(j=lambda_roots-1; j>=0; j--)
   {  int num1 = 0;
      int num2;
      int den;
      int location = loc[j];

      for(i=deg_omega; i>=0; i--) 
      {  if(omega[i] != GF_ALPHA0)
	 num1 ^= gt->alphaTo[mod_fieldmax(omega[i] + i * root[j])];
      }

      num2 = gt->alphaTo[mod_fieldmax(root[j] * (LEC_FIRST_ROOT - 1) + GF_FIELDMAX)];
      den = 0;
    
      /* lambda[i+1] for i even is the formal derivative lambda_pr of lambda[i] */

      for(i=MIN(deg_lambda, NROOTS-1) & ~1; i>=0; i-=2) 
      {  if(lambda[i+1] != GF_ALPHA0)
	   den ^= gt->alphaTo[mod_fieldmax(lambda[i+1] + i * root[j])];
      }

      /* Apply error to data */

      if(num1 != 0 && location >= padding)
      {  
	 corrected++;
	 data[location-padding] ^= gt->alphaTo[mod_fieldmax(gt->indexOf[num1] + gt->indexOf[num2] 
							    + GF_FIELDMAX - gt->indexOf[den])];

	 /* If no erasures were given, at most one error was corrected.
	    Return its position in erasure_list[0]. */

	 if(!erasure_count)
	    erasure_list[0] = location-padding;
      }
#if 1
      else return -3;
#endif
   }

   /*** Form the syndromes: Evaluate data(x) at roots of g(x) */

   for(i=0; i<NROOTS; i++)
     syndrome[i] = data[0];

   for(j=1; j<shortened_size; j++)
     for(i=0; i<NROOTS; i++)
     {  if(syndrome[i] == 0) 
             syndrome[i] = data[j];
        else syndrome[i] = data[j] ^ gt->alphaTo[mod_fieldmax(gt->indexOf[syndrome[i]] 
							      + (LEC_FIRST_ROOT+i)*LEC_PRIM_ELEM)];
    }

   /*** Convert syndrome to index form, check for nonzero condition. */
#if 1
   for(i=0; i<NROOTS; i++)
     if(syndrome[i])
       return -2;
#endif

   return corrected;
}


