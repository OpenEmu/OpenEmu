#include "mp3_dec/mpg123.h"
#include "mp3_dec/mpglib.h"
#include "cd_file.h"
#include "cdda_mp3.h"

// CD-ROM drive access
#ifdef GENS_CDROM
#include "cd_aspi.h"
#endif

struct mpstr mp;

extern int Sound_Rate;		// in G_dsound.h...

unsigned int Current_IN_Pos;
unsigned int Current_OUT_Pos;
unsigned int Current_OUT_Size;

char buf_out[8 * 1024];

int freqs_mp3[9] = { 44100, 48000, 32000,
  22050, 24000, 16000,
  11025, 12000, 8000
};


// GENS Re-Recording
// Modif N. -- added variable. Set when MP3 decoding fails, reset when attempting to play a new MP3.
// The point of this is to turn what was previously a crash
// into an error that only stops the MP3 playback part of the program for the particular MP3 that couldn't play
int fatal_mp3_error = 0;

// TODO: Finish porting the async decoding patch.
// _file_track, Tracks[]
extern char played_tracks_linear [101];


int
MP3_Init (void)
{
  InitMP3 (&mp);

  MP3_Reset ();

  return 0;
}


void
MP3_Reset (void)
{
  Current_IN_Pos = 0;
  Current_OUT_Pos = 0;
  Current_OUT_Size = 0;

  memset (buf_out, 0, 8 * 1024);
}


int
MP3_Get_Bitrate (FILE * f)
{
  unsigned int header, br;
  struct frame fr;

  fseek (f, 0, SEEK_SET);

  br = fread (&header, 1, 4, f);
  fseek (f, -3, SEEK_CUR);

  while (br == 4)
    {
      if ((header & 0x0000E0FF) == 0x0000E0FF)
	{
	  return (1000 * decode_header_bitrate (&fr, header));
	}

      br = fread (&header, 1, 4, f);
      fseek (f, -3, SEEK_CUR);
    }

  return -1;
}


int
MP3_Length_LBA (FILE * f)
{
  float len;
  unsigned int header, br;
  struct frame fr;

  fseek (f, 0, SEEK_SET);

  len = 0;

  br = fread (&header, 1, 4, f);
  fseek (f, -3, SEEK_CUR);

  while (br == 4)
    {
      if ((header & 0x0000E0FF) == 0x0000E0FF)
	{
	  len += decode_header_gens (&fr, header);

#ifdef DEBUG_CD
	  fprintf (debug_SCD_file, "mp3 length update = %f\n", len);
#endif

	  fseek (f, fr.framesize, SEEK_CUR);
	}

      br = fread (&header, 1, 4, f);
      fseek (f, -3, SEEK_CUR);
    }

  len *= (float) 0.075;

  return (int) len;
}


int
MP3_Find_Frame (FILE * f, int pos_wanted)
{
  unsigned int header;
  float cur_pos;
  int br, prev_pos;
  struct frame fr;

  fseek (f, 0, SEEK_SET);

//fprintf(debug_SCD_file, "             ****** pos wanted = %d\n", pos_wanted);

  cur_pos = (float) 0;
  prev_pos = 0;

  br = fread (&header, 1, 4, f);
  fseek (f, -3, SEEK_CUR);

  while ((int) cur_pos <= pos_wanted)
    {
      if (br < 4)
	break;

      if ((header & 0x0000E0FF) == 0x0000E0FF)
	{
	  float decode_header_res;
	  
	  prev_pos = ftell (f) - 1;

	  decode_header_res = (float) decode_header_gens (&fr, header) * (float) 0.075;
	  if (!decode_header_res)
	  {
		fatal_mp3_error = 1;
		break;
	  }
	  cur_pos += (float)decode_header_res * (float)0.075;

//fprintf(debug_SCD_file, "             hearder find at= %d     = %.8X\n", ftell(f) - 1, header);
//fprintf(debug_SCD_file, "             current time = %g\n", cur_pos);

	  if (fr.framesize < 0)
	    fr.framesize = 0;

	  fseek (f, fr.framesize + 3, SEEK_CUR);
	}

      br = fread (&header, 1, 4, f);

//fprintf(debug_SCD_file, "             next header at= %d      = %.8X\n", ftell(f) - 4, header);

      fseek (f, -3, SEEK_CUR);
    }

//fprintf(debug_SCD_file, "             pos returned = %d\n", prev_pos);

  return prev_pos;
}


int
MP3_Update_IN (void)
{
  char buf_in[8 * 1024];
  int size_read;

  if (Tracks[Track_Played].F == NULL)
    return -1;

  fseek (Tracks[Track_Played].F, Current_IN_Pos, SEEK_SET);
  size_read = fread (buf_in, 1, 8 * 1024, Tracks[Track_Played].F);
  Current_IN_Pos += size_read;

  if (size_read <= 0 || fatal_mp3_error)
    {
      // go to the next track

      Track_Played++;
      ResetMP3_Gens (&mp);

      if (Track_Played > 99)
	{
	  return 3;
	}
      else if (Tracks[Track_Played].F == NULL)
	{
	  return 3;
	}
      else if (Tracks[Track_Played].Type == TYPE_WAV)
	{
	  // WAV_Play();
	  return 4;
	}
      else if (Tracks[Track_Played].Type != TYPE_MP3)
	{
	  return 5;
	}

      Current_IN_Pos = MP3_Find_Frame (Tracks[Track_Played].F, 0);
      fseek (Tracks[Track_Played].F, Current_IN_Pos, SEEK_SET);
      size_read = fread (buf_in, 1, 8 * 1024, Tracks[Track_Played].F);
      Current_IN_Pos += size_read;
    }

  if (fatal_mp3_error)
	return 1;

  if (decodeMP3 (&mp, buf_in, size_read, buf_out, 8 * 1024, &Current_OUT_Size)
      != MP3_OK)
    {
      fseek (Tracks[Track_Played].F, Current_IN_Pos, SEEK_SET);
      size_read = fread (buf_in, 1, 8 * 1024, Tracks[Track_Played].F);
      Current_IN_Pos += size_read;

      if (decodeMP3
	  (&mp, buf_in, size_read, buf_out, 8 * 1024,
	   &Current_OUT_Size) != MP3_OK)
	{
		fatal_mp3_error = 1;
		return 1;
	}
    }

  return 0;
}


int
MP3_Update_OUT (void)
{
  Current_OUT_Pos = 0;

	if (fatal_mp3_error)
		return 1;

  if (decodeMP3 (&mp, NULL, 0, buf_out, 8 * 1024, &Current_OUT_Size) !=
      MP3_OK)
    {
      return MP3_Update_IN ();
    }

  return 0;
}


// int async added by GENS Re-Recording
int MP3_Play (int track, int lba_pos, int async)
{
  Track_Played = track;

  if (Tracks[Track_Played].F == NULL)
    {
      Current_IN_Pos = 0;
      return -1;
    }

	// TODO
#if 0
	if (!IsAsyncAllowed())
		async = 0;
#endif
	
/*	if (!async)
	{
		Preload_MP3(&Tracks[Track_Played].F_decoded, Track_Played);
	}
	
	if (async && !Tracks[Track_Played].F_decoded)
	{
*/		// start playing MP3 "asynchronously", decoding on the fly... but it won't reliably produce the same sound samples under the same circumstances
		Current_IN_Pos = MP3_Find_Frame (Tracks[Track_Played].F, lba_pos);
		ResetMP3_Gens (&mp);
		MP3_Update_IN ();
//	}

  return 0;
}

/*
int MP3_Update(int **buf, unsigned int length_dest)
{
	int *bufL, *bufR;
	unsigned int byte_mul, length_src, pas_src;
	unsigned int dest, src, limit_src;
	short *buf_mp3;
	float Freq_Div;

	bufL = buf[0];
	bufR = buf[1];

	if (Current_OUT_Size == 0) MP3_Update_IN();
	if (Current_OUT_Size == 0) return -1;

	if (Sound_Rate == 0) return -1;

	Freq_Div = (float) freqs_mp3[mp.fr.sampling_frequency];
	Freq_Div /= (float) Sound_Rate;

	if (Freq_Div == 0) return -1;
	
	length_src = (int) ((float) length_dest * Freq_Div);
	pas_src = (int) ((float) 256 * Freq_Div);

	if (mp.fr.stereo == 2) byte_mul = 2;
	else byte_mul = 1;

	dest = 0;

	while ((length_src << byte_mul) > (Current_OUT_Size - Current_OUT_Pos))
	{
		buf_mp3 = (short *) &buf_out[Current_OUT_Pos];

		if (mp.fr.stereo == 2)
		{
			limit_src = ((Current_OUT_Size - Current_OUT_Pos) >> 2) << 8;

			for(src = 0; src < limit_src; dest++, src += pas_src)
			{
				bufL[dest] += (int) (buf_mp3[((src >> 8) << 1) + 0]);
				bufR[dest] += (int) (buf_mp3[((src >> 8) << 1) + 1]);
			}
		}
		else
		{
			limit_src = ((Current_OUT_Size - Current_OUT_Pos) >> 1) << 8;

			for(src = 0; src < limit_src; dest++, src += pas_src)
			{
				bufL[dest] += (int) (buf_mp3[src >> 8]);
				bufR[dest] += (int) (buf_mp3[src >> 8]);
			}
		}

		length_src -= (limit_src >> 8);

		if (MP3_Update_OUT()) return -1;
	}

	buf_mp3 = (short *) &buf_out[Current_OUT_Pos];

	limit_src = length_src << 8;

	if (mp.fr.stereo == 2)
	{
		for(src = 0; src < limit_src; dest++, src += pas_src)
		{
			bufL[dest] += (int) (buf_mp3[((src >> 8) << 1) + 0]);
			bufR[dest] += (int) (buf_mp3[((src >> 8) << 1) + 1]);
		}
	}
	else
	{
		for(src = 0; src < limit_src; dest++, src += pas_src)
		{
			bufL[dest] += (int) (buf_mp3[src >> 8]);
			bufR[dest] += (int) (buf_mp3[src >> 8]);
		}
	}

	Current_OUT_Pos += length_src << byte_mul;

	return 0;
}
*/

int
MP3_Update (char *buf, int *rate, int *channel, unsigned int length_dest)
{
  unsigned int length_src, size;
  char *buf_mp3;

  if (Current_OUT_Size == 0)
    if (MP3_Update_IN ())
      return -1;
  if (Current_OUT_Size == 0)
    return -1;

  *rate = freqs_mp3[mp.fr.sampling_frequency];

  if (mp.fr.stereo == 2)
    *channel = 2;
  else
    *channel = 1;

  length_src = (*rate / 75) << *channel;

  size = Current_OUT_Size - Current_OUT_Pos;

//      fprintf(debug_SCD_file, "\n*********  rate = %d chan = %d size = %d len = %d\n", *rate, *channel, size, length_src);

  while (length_src > size)
    {
      buf_mp3 = (char *) &buf_out[Current_OUT_Pos];

      memcpy (buf, buf_mp3, size);

      length_src -= size;
      buf += size;

//              fprintf(debug_SCD_file, "size = %d len = %d\n", size, length_src);

      if (MP3_Update_OUT ())
	return -1;
      size = Current_OUT_Size - Current_OUT_Pos;
    }

  buf_mp3 = (char *) &buf_out[Current_OUT_Pos];

  memcpy (buf, buf_mp3, length_src);

//      fprintf(debug_SCD_file, "size = %d len = %d\n", size, length_src);

  Current_OUT_Pos += length_src;

  return 0;
}


/*
void MP3_Test(FILE* f)
{
	MP3_Reset();
	
	Current_MP3_File = f;
	
	if (Current_MP3_File == NULL) return;

}
*/
