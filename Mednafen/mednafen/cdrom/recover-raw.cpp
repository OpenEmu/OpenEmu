/*  dvdisaster: Additional error correction for optical media.
 *  Copyright (C) 2004-2007 Carsten Gnoerlich.
 *  Project home page: http://www.dvdisaster.com
 *  Email: carsten@dvdisaster.com  -or-  cgnoerlich@fsfe.org
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

static GaloisTables *gt = NULL;		/* for L-EC Reed-Solomon */
static ReedSolomonTables *rt = NULL;

bool Init_LEC_Correct(void)
{
 gt = CreateGaloisTables(0x11d);
 rt = CreateReedSolomonTables(gt, 0, 1, 10);

 return(1);
}

void Kill_LEC_Correct(void)
{
 FreeGaloisTables(gt);
 FreeReedSolomonTables(rt);
}

/***
 *** CD level CRC calculation
 ***/

/*
 * Test raw sector against its 32bit CRC.
 * Returns TRUE if frame is good.
 */

int CheckEDC(unsigned char *cd_frame, bool xa_mode)
{ 
 unsigned int expected_crc, real_crc;
 unsigned int crc_base = xa_mode ? 2072 : 2064;

 expected_crc = cd_frame[crc_base + 0] << 0;
 expected_crc |= cd_frame[crc_base + 1] << 8;
 expected_crc |= cd_frame[crc_base + 2] << 16;
 expected_crc |= cd_frame[crc_base + 3] << 24;

 if(xa_mode) 
  real_crc = EDCCrc32(cd_frame+16, 2056);
 else
  real_crc = EDCCrc32(cd_frame, 2064);

 if(expected_crc == real_crc)
  return(1);
 else
 {
  //printf("Bad EDC CRC:  Calculated:  %08x,  Recorded:  %08x\n", real_crc, expected_crc);
  return(0);
 }
}

/***
 *** A very simple L-EC error correction.
 ***
 * Perform just one pass over the Q and P vectors to see if everything
 * is okay respectively correct minor errors. This is pretty much the
 * same stuff the drive is supposed to do in the final L-EC stage.
 */

static int simple_lec(unsigned char *frame)
{ 
   unsigned char byte_state[2352];
   unsigned char p_vector[P_VECTOR_SIZE];
   unsigned char q_vector[Q_VECTOR_SIZE];
   unsigned char p_state[P_VECTOR_SIZE];
   int erasures[Q_VECTOR_SIZE], erasure_count;
   int ignore[2];
   int p_failures, q_failures;
   int p_corrected, q_corrected;
   int p,q;

   /* Setup */

   memset(byte_state, 0, 2352);

   p_failures = q_failures = 0;
   p_corrected = q_corrected = 0;

   /* Perform Q-Parity error correction */

   for(q=0; q<N_Q_VECTORS; q++)
   {  int err;

      /* We have no erasure information for Q vectors */

     GetQVector(frame, q_vector, q);
     err = DecodePQ(rt, q_vector, Q_PADDING, ignore, 0);

     /* See what we've got */

     if(err < 0)  /* Uncorrectable. Mark bytes are erasure. */
     {  q_failures++;
        FillQVector(byte_state, 1, q);
     }
     else         /* Correctable */ 
     {  if(err == 1 || err == 2) /* Store back corrected vector */ 
	{  SetQVector(frame, q_vector, q);
	   q_corrected++;
	}
     }
   }

   /* Perform P-Parity error correction */

   for(p=0; p<N_P_VECTORS; p++)
   {  int err,i;

      /* Try error correction without erasure information */

      GetPVector(frame, p_vector, p);
      err = DecodePQ(rt, p_vector, P_PADDING, ignore, 0);

      /* If unsuccessful, try again using erasures.
	 Erasure information is uncertain, so try this last. */

      if(err < 0 || err > 2)
      {  GetPVector(byte_state, p_state, p);
	 erasure_count = 0;

	 for(i=0; i<P_VECTOR_SIZE; i++)
	   if(p_state[i])
	     erasures[erasure_count++] = i;

	 if(erasure_count > 0 && erasure_count <= 2)
	 {  GetPVector(frame, p_vector, p);
	    err = DecodePQ(rt, p_vector, P_PADDING, erasures, erasure_count);
	 }
      }

      /* See what we've got */

      if(err < 0)  /* Uncorrectable. */
      {  p_failures++;
      }
      else         /* Correctable. */ 
      {  if(err == 1 || err == 2) /* Store back corrected vector */ 
	 {  SetPVector(frame, p_vector, p);
	    p_corrected++;
	 }
      }
   }

   /* Sum up */

   if(q_failures || p_failures || q_corrected || p_corrected)
   {
     return 1;
   }

   return 0;
}

/***
 *** Validate CD raw sector
 ***/

int ValidateRawSector(unsigned char *frame, bool xaMode)
{  
 int lec_did_sth = FALSE;

  /* Do simple L-EC.
     It seems that drives stop their internal L-EC as soon as the
     EDC is okay, so we may see uncorrected errors in the parity bytes.
     Since we are also interested in the user data only and doing the
     L-EC is expensive, we skip our L-EC as well when the EDC is fine. */

  if(!CheckEDC(frame, xaMode))
  {
   lec_did_sth = simple_lec(frame);
  }
  /* Test internal sector checksum again */

  if(!CheckEDC(frame, xaMode))
  {  
   /* EDC failure in RAW sector */
   return FALSE;
  }

  return TRUE;
}

