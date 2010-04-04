/* Mednafen - Multi-system Emulator
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "mednafen.h"

#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "driver.h"
#include "endian.h"
#include "general.h"
#include "state.h"
#include "movie.h"
#include "memory.h"
#include "netplay.h"
#include "video.h"

static int SaveStateStatus[10];

#define RLSB 		MDFNSTATE_RLSB	//0x80000000

int32 smem_read(StateMem *st, void *buffer, uint32 len)
{
 if((len + st->loc) > st->len)
  return(0);

 memcpy(buffer, st->data + st->loc, len);
 st->loc += len;

 return(len);
}

int32 smem_write(StateMem *st, void *buffer, uint32 len)
{
 if((len + st->loc) > st->malloced)
 {
  uint32 newsize = (st->malloced >= 32768) ? st->malloced : (st->initial_malloc ? st->initial_malloc : 32768);

  while(newsize < (len + st->loc))
   newsize *= 2;
  st->data = (uint8 *)realloc(st->data, newsize);
  st->malloced = newsize;
 }
 memcpy(st->data + st->loc, buffer, len);
 st->loc += len;

 if(st->loc > st->len) st->len = st->loc;

 return(len);
}

int32 smem_putc(StateMem *st, int value)
{
 uint8 tmpval = value;
 if(smem_write(st, &tmpval, 1) != 1)
  return(-1);
 return(1);
}

int32 smem_tell(StateMem *st)
{
 return(st->loc);
}

int32 smem_seek(StateMem *st, uint32 offset, int whence)
{
 switch(whence)
 {
  case SEEK_SET: st->loc = offset; break;
  case SEEK_END: st->loc = st->len - offset; break;
  case SEEK_CUR: st->loc += offset; break;
 }

 if(st->loc > st->len)
 {
  st->loc = st->len;
  return(-1);
 }

 if(st->loc < 0)
 {
  st->loc = 0;
  return(-1);
 }

 return(0);
}

int smem_write32le(StateMem *st, uint32 b)
{
 uint8 s[4];
 s[0]=b;
 s[1]=b>>8;
 s[2]=b>>16;
 s[3]=b>>24;
 return((smem_write(st, s, 4)<4)?0:4);
}

int smem_read32le(StateMem *st, uint32 *b)
{
 uint8 s[4];

 if(smem_read(st, s, 4) < 4)
  return(0);

 *b = s[0] | (s[1] << 8) | (s[2] << 16) | (s[3] << 24);

 return(4);
}


static bool ValidateSFStructure(SFORMAT *sf)
{
 SFORMAT *saved_sf = sf;

 while(sf->s || sf->desc)
 {
  SFORMAT *sub_sf = saved_sf;
  while(sub_sf->s || sub_sf->desc)
  {
   if(sf != sub_sf)
   {
    if(!strncmp(sf->desc, sub_sf->desc, 32))
    {
     printf("Duplicate state variable name: %.32s\n", sf->desc);
    }
   }
   sub_sf++;
  }

  sf++;
 }
 return(1);
}


static int SubWrite(StateMem *st, SFORMAT *sf, int data_only, gzFile fp)
{
 uint32 acc=0;

 // FIXME?  It's kind of slow, and we definitely don't want it on with state rewinding...
 //ValidateSFStructure(sf);

 while(sf->s || sf->desc)	// Size can sometimes be zero, so also check for the text description.  These two should both be zero only at the end of a struct.
 {
  if(!sf->s || !sf->v)
  {
   sf++;
   continue;
  }
  if(sf->s == (uint32)~0)		/* Link to another struct.	*/
  {
   uint32 tmp;

   if(!(tmp=SubWrite(st,(SFORMAT *)sf->v, data_only, fp)))
    return(0);
   acc+=tmp;
   sf++;
   continue;
  }

  if(!data_only)
   acc+=32 + 4;			/* Description + size */

  int32 bytesize = sf->s&(~(MDFNSTATE_RLSB32 | MDFNSTATE_RLSB16 | RLSB));

  acc += bytesize;
  //printf("%d %d %d\n", bytesize, data_only, fp);
  if(st || fp)			/* Are we writing or calculating the size of this block? */
  {
   if(!data_only)
   {
    char desco[32];
    int slen = strlen(sf->desc);

    memset(desco, 0, 32);

    if(slen > 32)
    {
     printf("Warning:  state variable name too long: %s %d\n", sf->desc, slen);
     slen = 32;
    }

    memcpy(desco, sf->desc, slen);
    smem_write(st, desco, 32);
    smem_write32le(st, bytesize);

    /* Flip the byte order... */
    if(sf->s & MDFNSTATE_RLSB32)
     Endian_A32_NE_to_LE(sf->v, bytesize / sizeof(uint32));
    else if(sf->s & MDFNSTATE_RLSB16)
     Endian_A16_NE_to_LE(sf->v, bytesize / sizeof(uint16));
    else if(sf->s&RLSB)
     Endian_V_NE_to_LE(sf->v, bytesize);
   }

   if(fp)
   {
    //printf("Wrote: %d\n", bytesize);
    gzwrite(fp, sf->v, bytesize);
   }
   else
   {
    smem_write(st, (uint8 *)sf->v, bytesize);
   }

   if(!data_only)
   {
    /* Now restore the original byte order. */
    if(sf->s & MDFNSTATE_RLSB32)
     Endian_A32_LE_to_NE(sf->v, bytesize / sizeof(uint32));
    else if(sf->s & MDFNSTATE_RLSB16)
     Endian_A16_LE_to_NE(sf->v, bytesize / sizeof(uint16));
    else if(sf->s&RLSB)
     Endian_V_LE_to_NE(sf->v, bytesize);
   }
  }
  sf++; 
 }

 return(acc);
}

static int WriteStateChunk(StateMem *st, const char *sname, SFORMAT *sf, int data_only)
{
 int bsize;

 if(!data_only)
  smem_write(st, (uint8 *)sname, 4);

 bsize=SubWrite(0,sf, data_only, NULL);

 if(!data_only)
  smem_write32le(st, bsize);

 if(!SubWrite(st,sf, data_only, NULL)) return(0);

 if(data_only)
  return(bsize);
 else
  return (bsize + 4 + 4);
}

static SFORMAT *CheckS(SFORMAT *sf, uint32 tsize, const char *desc)
{
 while(sf->s || sf->desc) // Size can sometimes be zero, so also check for the text description.  These two should both be zero only at the end of a struct.
 {
  if(!sf->s || !sf->v)
  {
   sf++;
   continue;
  }
  if(sf->s==(uint32)~0)		/* Link to another SFORMAT structure. */
  {
   SFORMAT *tmp;
   if((tmp= CheckS((SFORMAT *)sf->v, tsize, desc) ))
    return(tmp);
   sf++;
   continue;
  }
  char check_str[32];
  memset(check_str, 0, sizeof(check_str));

  strncpy(check_str, sf->desc, 32);

  if(!memcmp(desc, check_str, 32))
  {
   uint32 bytesize = sf->s&(~(MDFNSTATE_RLSB32 | MDFNSTATE_RLSB16 | RLSB));

   if(tsize != bytesize)
   {
    printf("tsize != bytesize: %.32s\n", desc);
    return(0);
   }
   return(sf);
  }
  sf++;
 }
 return(0);
}

// Fast raw chunk reader
static void DOReadChunk(StateMem *st, SFORMAT *sf)
{
 while(sf->s || sf->desc)       // Size can sometimes be zero, so also check for the text description.  
				// These two should both be zero only at the end of a struct.
 {
  if(!sf->s || !sf->v)
  {
   sf++;
   continue;
  }

  if(sf->s == (uint32) ~0) // Link to another SFORMAT struct
  {
   DOReadChunk(st, (SFORMAT *)sf->v);
   sf++;
   continue;
  }

  int32 bytesize = sf->s&(~(MDFNSTATE_RLSB32 | MDFNSTATE_RLSB16 | RLSB));

  smem_read(st, (uint8 *)sf->v, bytesize);
  sf++;
 }
}

static int ReadStateChunk(StateMem *st, SFORMAT *sf, int size, int data_only)
{
 SFORMAT *tmp;
 int temp;

 if(data_only)
 {
  DOReadChunk(st, sf);
 }
 else
 {
  temp = smem_tell(st);
  while(smem_tell(st) < temp + size)
  {
   uint32 tsize;
   char toa[32];

   if(smem_read(st, toa, 32) <= 0)
   {
    puts("Unexpected EOF?");
    return 0;
   }

   smem_read32le(st, &tsize);
   if((tmp=CheckS(sf,tsize,toa)))
   {
    int32 bytesize = tmp->s&(~(MDFNSTATE_RLSB32 | MDFNSTATE_RLSB16 | RLSB));

    smem_read(st, (uint8 *)tmp->v, bytesize);

    if(tmp->s & MDFNSTATE_RLSB32)
     Endian_A32_LE_to_NE(tmp->v, bytesize / sizeof(uint32));
    else if(tmp->s & MDFNSTATE_RLSB16)
     Endian_A16_LE_to_NE(tmp->v, bytesize / sizeof(uint16));
    else if(tmp->s&RLSB)
     Endian_V_LE_to_NE(tmp->v, bytesize);
   }
   else
    if(smem_seek(st,tsize,SEEK_CUR) < 0)
    {
     puts("Seek error");
     return(0);
    }
  } // while(...)
 }
 return 1;
}

static int CurrentState = 0;
static int RecentlySavedState = -1;

static void MakeStatePreview(uint8 *dest, uint32 *fb, MDFN_Rect *LineWidths)
{
 int x, y;

 if(!fb || !LineWidths) return;

 for(y = MDFNGameInfo->DisplayRect.y; y < (MDFNGameInfo->DisplayRect.y + MDFNGameInfo->DisplayRect.h); y++)
 {
  uint32 mw;
  uint32 whole_pix;

  if(LineWidths[0].w != ~0)
   mw = LineWidths[y].w;
  else
   mw = MDFNGameInfo->DisplayRect.w;

  whole_pix = 256 * mw / MDFNGameInfo->ss_preview_width;

  // TODO:  Add support for scaling when (mw / MDFNGameInfo->ss_preview_width) > 2 and it's not an integer
  // (no emulated systems as of Jan 31, 2007 are like this, though)
  // int whole_pix_i = (whole_pix >> 8) - 2;
  // if(whole_pix_i < 0) whole_pix_i = 0;

  for(x = 0; x < MDFNGameInfo->ss_preview_width; x++)
  {
   uint32 accum_r, accum_g, accum_b;
   uint32 pixel;
   int nr, ng, nb;
   uint32 real_x;

   accum_r = 0;
   accum_g = 0;
   accum_b = 0;

   real_x = x * 256 * mw / MDFNGameInfo->ss_preview_width;

   if(!(whole_pix & 0xFF) && whole_pix)
   {
    for(unsigned int px = 0; px < (whole_pix >> 8); px++)
    {
     pixel = fb[(real_x >> 8) + px + LineWidths[y].x + y * MDFNGameInfo->pitch / sizeof(uint32)];
     DECOMP_COLOR(pixel, nr, ng, nb);
  
     accum_r += nr * 256 / (whole_pix >> 8);
     accum_g += ng * 256 / (whole_pix >> 8);
     accum_b += nb * 256 / (whole_pix >> 8);
    }
   }
   else
   {
    pixel = fb[(real_x >> 8) + LineWidths[y].x + y * MDFNGameInfo->pitch / sizeof(uint32)];
    DECOMP_COLOR(pixel, nr, ng, nb);

    accum_r += (256 - (real_x & 0xFF)) * nr;
    accum_g += (256 - (real_x & 0xFF)) * ng;
    accum_b += (256 - (real_x & 0xFF)) * nb;

    pixel = fb[(real_x >> 8) + 1 + LineWidths[y].x + y * MDFNGameInfo->pitch / sizeof(uint32)];
    DECOMP_COLOR(pixel, nr, ng, nb);

    accum_r += (real_x & 0xFF) * nr;
    accum_g += (real_x & 0xFF) * ng;
    accum_b += (real_x & 0xFF) * nb;
   }

   {
    int r = (accum_r + 127) >> 8;
    int g = (accum_g + 127) >> 8;
    int b = (accum_b + 127) >> 8;

    if(r > 255) r = 255;
    if(g > 255) g = 255;
    if(b > 255) b = 255;

    *dest++ = r;
    *dest++ = g;
    *dest++ = b;
   }

  }
 }
}

/* This function is called by the game driver(NES, GB, GBA) to save a state. */
int MDFNSS_StateAction(StateMem *st, int load, int data_only, std::vector <SSDescriptor> &sections)
{
 std::vector<SSDescriptor>::iterator section;

 if(load)
 {
  char sname[4];

  for(section = sections.begin(); section != sections.end(); section++)
  {
   if(data_only)
   {
    ReadStateChunk(st, section->sf, ~0, 1);
   }
   else
   {
    int found = 0;
    uint32 tmp_size;
    uint32 total = 0;
    while(smem_read(st, (uint8 *)sname, 4) == 4)
    {
     if(!smem_read32le(st, &tmp_size)) return(0);
     total += tmp_size + 8;
     // Yay, we found the section
     if(!memcmp(sname, section->name, 4))
     {
      if(!ReadStateChunk(st, section->sf, tmp_size, 0))
      {
       printf("Error reading chunk: %.4s\n", section->name);
       return(0);
      }
      found = 1;
      break;
     } 
     else
     {
	//puts("SEEK");
      if(smem_seek(st, tmp_size, SEEK_CUR) < 0)
      {
       puts("Chunk seek failure");
       return(0);
      }
     }
    }
    if(smem_seek(st, -total, SEEK_CUR) < 0)
    {
     puts("Reverse seek error");
     return(0);
    }
    if(!found && !section->optional) // Not found.  We are sad!
    {
	printf("Chunk missing:  %.4s\n", section->name);
	return(0);
    }
   }
  }
 }
 else
  for(section = sections.begin(); section != sections.end(); section++)
  {
   if(!WriteStateChunk(st, section->name, section->sf, data_only))
    return(0);
  }
 return(1);
}

int MDFNSS_StateAction(StateMem *st, int load, int data_only, SFORMAT *sf, const char *name, bool optional)
{
 std::vector <SSDescriptor> love;

 love.push_back(SSDescriptor(sf, name, optional));
 return(MDFNSS_StateAction(st, load, data_only, love));
}

int MDFNSS_SaveSM(StateMem *st, int wantpreview, int data_only, uint32 *fb, MDFN_Rect *LineWidths)
{
        static uint8 header[32]="MEDNAFENSVESTATE";
	int neowidth, neoheight;

	neowidth = MDFNGameInfo->ss_preview_width;
	neoheight = MDFNGameInfo->DisplayRect.h;

	if(!data_only)
	{
         memset(header+16,0,16);
	 MDFN_en32lsb(header + 16, MEDNAFEN_VERSION_NUMERIC);
	 MDFN_en32lsb(header + 24, neowidth);
	 MDFN_en32lsb(header + 28, neoheight);
	 smem_write(st, header, 32);
	}

	if(wantpreview)
	{
         uint8 *previewbuffer = (uint8 *)malloc(3 * neowidth * neoheight);

         MakeStatePreview(previewbuffer, fb, LineWidths);
         smem_write(st, previewbuffer, 3 * neowidth * neoheight);

	 free(previewbuffer);
	}

        // State rewinding code path hack, FIXME
        if(data_only)
        {
         if(!MDFN_RawInputStateAction(st, 0, data_only))
          return(0);
        }

	if(!MDFNGameInfo->StateAction(st, 0, data_only))
	 return(0);

	if(!data_only)
	{
	 uint32 sizy = smem_tell(st);
	 smem_seek(st, 16 + 4, SEEK_SET);
	 smem_write32le(st, sizy);
	}
	return(1);
}

int MDFNSS_Save(const char *fname, const char *suffix, uint32 *fb, MDFN_Rect *LineWidths)
{
	StateMem st;

	memset(&st, 0, sizeof(StateMem));

	if(!MDFNSS_SaveSM(&st, 1, 0, fb, LineWidths))
	{
	 if(st.data)
	  free(st.data);
	 if(!fname && !suffix)
 	  MDFN_DispMessage(_("State %d save error."), CurrentState);
	 return(0);
	}

	if(!MDFN_DumpToFile(fname ? fname : MDFN_MakeFName(MDFNMKF_STATE,CurrentState,suffix).c_str(), 6, st.data, st.len))
	{
         SaveStateStatus[CurrentState] = 0;
	 free(st.data);

         if(!fname && !suffix)
          MDFN_DispMessage(_("State %d save error."),CurrentState);

	 return(0);
	}

	free(st.data);

	SaveStateStatus[CurrentState] = 1;
        RecentlySavedState = CurrentState;

	if(!fname && !suffix)
	 MDFN_DispMessage(_("State %d saved."),CurrentState);

	return(1);
}

// Convenience function for movie.cpp
int MDFNSS_SaveFP(gzFile fp, uint32 *fb, MDFN_Rect *LineWidths)
{
 StateMem st;

 memset(&st, 0, sizeof(StateMem));

 if(!MDFNSS_SaveSM(&st, 1, 0, fb, LineWidths))
 {
  if(st.data)
   free(st.data);
  return(0);
 }

 if(gzwrite(fp, st.data, st.len) != (int32)st.len)
 {
  if(st.data)
   free(st.data);
  return(0);
 }

 if(st.data)
  free(st.data);

 return(1);
}


int MDFNSS_LoadSM(StateMem *st, int haspreview, int data_only)
{
        uint8 header[32];
	uint32 stateversion;

	if(data_only)
	{
	 stateversion = MEDNAFEN_VERSION_NUMERIC;
	}
	else
	{
         smem_read(st, header, 32);
         if(memcmp(header,"MEDNAFENSVESTATE",16))
          return(0);

	 stateversion = MDFN_de32lsb(header + 16);

	 if(stateversion < 0x0600)
 	 {
	  printf("State too old: %08x\n", stateversion);
	  return(0);
	 }
	}

	if(haspreview)
        {
         uint32 width = MDFN_de32lsb(header + 24);
         uint32 height = MDFN_de32lsb(header + 28);
	 uint32 psize;

	 psize = width * height * 3;
	 smem_seek(st, psize, SEEK_CUR);	// Skip preview
 	}

	// State rewinding code path hack, FIXME
	if(data_only)
	{
	 if(!MDFN_RawInputStateAction(st, stateversion, data_only))
	  return(0);
	}

	return(MDFNGameInfo->StateAction(st, stateversion, data_only));
}

int MDFNSS_LoadFP(gzFile fp)
{
 uint8 header[32];
 StateMem st;
 
 memset(&st, 0, sizeof(StateMem));

 if(gzread(fp, header, 32) != 32)
 {
  return(0);
 }
 st.len = MDFN_de32lsb(header + 16 + 4);

 if(st.len < 32)
  return(0);

 if(!(st.data = (uint8 *)malloc(st.len)))
  return(0);

 memcpy(st.data, header, 32);
 if(gzread(fp, st.data + 32, st.len - 32) != ((int32)st.len - 32))
 {
  free(st.data);
  return(0);
 }
 if(!MDFNSS_LoadSM(&st, 1, 0))
 {
  free(st.data);
  return(0);
 }
 free(st.data);
 return(1);
}

int MDFNSS_Load(const char *fname, const char *suffix)
{
	gzFile st;

        if(fname)
         st=gzopen(fname, "rb");
        else
        {
         st=gzopen(MDFN_MakeFName(MDFNMKF_STATE,CurrentState,suffix).c_str(),"rb");
	}

	if(st == NULL)
	{
	 if(!fname && !suffix)
	 {
          MDFN_DispMessage(_("State %d load error."),CurrentState);
          SaveStateStatus[CurrentState]=0;
	 }
	 return(0);
	}

	if(MDFNSS_LoadFP(st))
	{
	 if(!fname && !suffix)
	 {
          SaveStateStatus[CurrentState]=1;
          MDFN_DispMessage(_("State %d loaded."),CurrentState);
          SaveStateStatus[CurrentState]=1;
	 }
	 gzclose(st);
         return(1);
        }   
        else
        {
         SaveStateStatus[CurrentState]=1;
         MDFN_DispMessage(_("State %d read error!"),CurrentState);
	 gzclose(st);
         return(0);
        }
}

void MDFNSS_CheckStates(void)
{
        time_t last_time = 0;

        for(int ssel = 0; ssel < 10; ssel++)
        {
         struct stat stat_buf;

         SaveStateStatus[ssel] = 0;
         if(stat(MDFN_MakeFName(MDFNMKF_STATE, ssel, 0).c_str(), &stat_buf) == 0)
         {
          SaveStateStatus[ssel] = 1;
          if(stat_buf.st_mtime > last_time)
          {
           RecentlySavedState = ssel;
           last_time = stat_buf.st_mtime;
          }
         }
        }

        CurrentState = 0;
        MDFND_SetStateStatus(NULL);
}

void MDFNI_SelectState(int w)
{
 gzFile fp;
 uint32 StateShow;
 uint32 *StateShowPB = NULL;
 uint32 StateShowPBWidth;
 uint32 StateShowPBHeight;

 if(w == -1) 
 {  
  MDFND_SetStateStatus(NULL);
  StateShow = 0; 
  return; 
 }
 MDFNI_SelectMovie(-1);

 if(w == 666 + 1)
  CurrentState = (CurrentState + 1) % 10;
 else if(w == 666 - 1)
 {
  CurrentState--;

  if(CurrentState < 0 || CurrentState > 9)
   CurrentState = 9;
 }
 else
  CurrentState = w;
 StateShow = MDFND_GetTime() + 2000;

 fp = gzopen(MDFN_MakeFName(MDFNMKF_STATE,CurrentState,NULL).c_str(),"rb");
 if(fp)
 {
  uint8 header[32];

  gzread(fp, header, 32);
  uint32 width = MDFN_de32lsb(header + 24);
  uint32 height = MDFN_de32lsb(header + 28);

  if(width > 512) width = 512;
  if(height > 512) height = 512;

  {
   uint8 previewbuffer[3 * width * height];
   uint8 *rptr = previewbuffer;

   gzread(fp, previewbuffer, 3 * width * height);

   if(StateShowPB)
   {
    free(StateShowPB);
    StateShowPB = NULL;
   }
   StateShowPB = (uint32 *)malloc(4 * width * height);
   StateShowPBWidth = width;
   StateShowPBHeight = height;

   for(unsigned int y=0; y<height; y++)
    for(unsigned int x=0; x<width; x++)
    {
     StateShowPB[x + y * width] = MK_COLORA(rptr[0],rptr[1],rptr[2], 0xFF);
     rptr+=3;
    }

   gzclose(fp);
  }
 }
 else
 {
  if(StateShowPB)
  {
   free(StateShowPB);
   StateShowPB = NULL;
  }
  StateShowPBWidth = MDFNGameInfo->ss_preview_width;
  StateShowPBHeight = MDFNGameInfo->DisplayRect.h;
 }
 MDFN_ResetMessages();

 StateStatusStruct *status = (StateStatusStruct*)calloc(1, sizeof(StateStatusStruct));
 
 memcpy(status->status, SaveStateStatus, 10 * sizeof(int));
 status->current = CurrentState;
 status->recently_saved = RecentlySavedState;
 status->gfx = StateShowPB;
 status->w = StateShowPBWidth;
 status->h = StateShowPBHeight;
 status->pitch = StateShowPBWidth;

 MDFND_SetStateStatus(status);
}  

void MDFNI_SaveState(const char *fname, const char *suffix, uint32 *fb, MDFN_Rect *LineWidths)
{
 MDFND_SetStateStatus(NULL);
 MDFNSS_Save(fname, suffix, fb, LineWidths);
}

void MDFNI_LoadState(const char *fname, const char *suffix)
{
 MDFND_SetStateStatus(NULL);

 /* For network play and movies, be load the state locally, and then save the state to a temporary buffer,
    and send or record that.  This ensures that if an older state is loaded that is missing some
    information expected in newer save states, desynchronization won't occur(at least not
    from this ;)).
 */
 if(MDFNSS_Load(fname, suffix))
 {
  #ifdef NETWORK
  if(MDFNnetplay)
   MDFNNET_SendState();
  #endif
  if(MDFNMOV_IsRecording())
   MDFNMOV_RecordState();
 }
}

#include "compress/minilzo.h"
#include "compress/quicklz.h"
#include "compress/blz.h"

enum
{
 SRW_COMPRESSOR_MINILZO = 0,
 SRW_COMPRESSOR_QUICKLZ,
 SRW_COMPRESSOR_BLZ
};

typedef struct
{
	uint8 *data;
	uint32 compressed_len;
	uint32 uncompressed_len;

	StateMem MovieLove;
} StateMemPacket;

static int SRW_NUM = 600;
static int SRWCompressor;
static int EvilEnabled = 0;
static StateMemPacket *bcs;
static int32 bcspos;

void MDFN_StateEvilBegin(void)
{
 int x;
 std::string srwcompstring;

 if(!EvilEnabled)
  return;

 SRW_NUM = MDFN_GetSettingUI("srwframes");

 SRWCompressor = SRW_COMPRESSOR_MINILZO;
 srwcompstring = MDFN_GetSettingS("srwcompressor");

 if(srwcompstring == "minilzo")
  SRWCompressor = SRW_COMPRESSOR_MINILZO;
 else if(srwcompstring == "quicklz")
  SRWCompressor  = SRW_COMPRESSOR_QUICKLZ;
 else if(srwcompstring == "blz")
  SRWCompressor = SRW_COMPRESSOR_BLZ;

 bcs = (StateMemPacket *)calloc(SRW_NUM, sizeof(StateMemPacket));
 bcspos = 0;

 for(x=0;x<SRW_NUM;x++)
 {
  bcs[x].data = NULL;
  bcs[x].compressed_len = 0;
  bcs[x].uncompressed_len = 0;
  memset(&bcs[x].MovieLove, 0, sizeof(StateMem));
 }
}

bool MDFN_StateEvilIsRunning(void)
{
 return(EvilEnabled);
}

void MDFN_StateEvilEnd(void)
{
 int x;

 if(!EvilEnabled)
  return;

 if(bcs)
 {
  if(MDFNMOV_IsRecording())
   MDFN_StateEvilFlushMovieLove();

  for(x = 0;x < SRW_NUM; x++)
  {

   if(bcs[x].data)
    free(bcs[x].data);
   bcs[x].data = NULL;
   bcs[x].compressed_len = 0;
  }
  free(bcs);
 }
}

void MDFN_StateEvilFlushMovieLove(void)
{
 int bahpos = (bcspos + 1) % SRW_NUM;
 for(int x = 0; x < SRW_NUM; x++)
 {
  if(bcs[bahpos].MovieLove.data)
  {
   if(bcs[x].data)
    MDFNMOV_ForceRecord(&bcs[bahpos].MovieLove);
   free(bcs[bahpos].MovieLove.data);
   bcs[bahpos].MovieLove.data = NULL;
  }
  bahpos = (bahpos + 1) % SRW_NUM;
 }
}

int MDFN_StateEvil(int rewind)
{
 if(!EvilEnabled)
  return(0);

 if(rewind)
 {
  int32 next_bcspos = bcspos;
  bool NeedDataFlush = FALSE;

  bcspos--;
  if(bcspos < 0) bcspos += SRW_NUM;

  if(!bcs[bcspos].data)
   bcspos = (bcspos + 1) % SRW_NUM;
  else
   NeedDataFlush = TRUE;

  if(bcs[bcspos].compressed_len)
  {
   uint8 *tmp_buf;
   lzo_uint dst_len = bcs[bcspos].uncompressed_len;

   tmp_buf = (uint8 *)malloc(bcs[bcspos].uncompressed_len);

   if(SRWCompressor == SRW_COMPRESSOR_QUICKLZ)
    dst_len = qlz_decompress((char*)bcs[bcspos].data, tmp_buf);
   else if(SRWCompressor == SRW_COMPRESSOR_MINILZO)
    lzo1x_decompress(bcs[bcspos].data, bcs[bcspos].compressed_len, tmp_buf, &dst_len, NULL);
   else if(SRWCompressor == SRW_COMPRESSOR_BLZ)
   {
    dst_len = blz_unpack(bcs[bcspos].data, tmp_buf);
   }
   for(uint32 x = 0; x < bcs[bcspos].uncompressed_len && x < bcs[next_bcspos].uncompressed_len; x++)
    tmp_buf[x] ^= bcs[next_bcspos].data[x];

   free(bcs[bcspos].data);
   bcs[bcspos].data = tmp_buf;
   bcs[bcspos].compressed_len = 0;
  }

  if(NeedDataFlush)
  {
   if(bcs[next_bcspos].MovieLove.data)
   {
    free(bcs[next_bcspos].MovieLove.data);
    bcs[next_bcspos].MovieLove.data = NULL;
   }
   free(bcs[next_bcspos].data);
   bcs[next_bcspos].data = NULL;
   bcs[next_bcspos].compressed_len = 0;
   bcs[next_bcspos].uncompressed_len = 0;
  }

  if(bcs[bcspos].uncompressed_len)
  {
   StateMem sm;

   sm.data = bcs[bcspos].data;
   sm.loc = 0;
   sm.initial_malloc = 0;
   sm.malloced = sm.len = bcs[bcspos].uncompressed_len;

   MDFNSS_LoadSM(&sm, 0, 1);

   free(MDFNMOV_GrabRewindJoy().data);
   return(1);
  }
 }
 else
 {
  StateMem sm;
  int32 prev_bcspos = bcspos;

  bcspos = (bcspos + 1) % SRW_NUM;

  if(MDFNMOV_IsRecording())
  {
   if(bcs[bcspos].data && bcs[bcspos].MovieLove.data)
   {
    //printf("Force: %d\n", bcspos);
    MDFNMOV_ForceRecord(&bcs[bcspos].MovieLove);
    free(bcs[bcspos].MovieLove.data);
    bcs[bcspos].MovieLove.data = NULL;
   }
  }
  if(bcs[bcspos].data)
  {
   free(bcs[bcspos].data);
   bcs[bcspos].data = NULL;
  }
  if(bcs[bcspos].MovieLove.data)
  {
   free(bcs[bcspos].MovieLove.data);
   bcs[bcspos].MovieLove.data = NULL;
  }

  memset(&sm, 0, sizeof(sm));

  MDFNSS_SaveSM(&sm, 0, 1);

  bcs[bcspos].data = sm.data;
  bcs[bcspos].compressed_len = 0;
  bcs[bcspos].uncompressed_len = sm.len;

  // Compress the previous save state.
  if(bcs[prev_bcspos].data)
  {
   for(uint32 x = 0; x < bcs[prev_bcspos].uncompressed_len && x < sm.len; x++)
    bcs[prev_bcspos].data[x] ^= sm.data[x];

   if(SRWCompressor == SRW_COMPRESSOR_QUICKLZ)
   {
    uint8 *tmp_buf = (uint8 *)malloc(bcs[prev_bcspos].uncompressed_len + 36000);
    uint32 dst_len = bcs[prev_bcspos].uncompressed_len + 36000;

    dst_len = qlz_compress(bcs[prev_bcspos].data, (char*)tmp_buf, bcs[prev_bcspos].uncompressed_len);

    free(bcs[prev_bcspos].data);
    bcs[prev_bcspos].data = (uint8 *)realloc(tmp_buf, dst_len);
    bcs[prev_bcspos].compressed_len = dst_len;
   }
   else if(SRWCompressor == SRW_COMPRESSOR_MINILZO)
   {
    uint8 workmem[LZO1X_1_MEM_COMPRESS];
    uint8 * tmp_buf = (uint8 *)malloc((size_t)(1.10 * bcs[prev_bcspos].uncompressed_len));
    lzo_uint dst_len = (lzo_uint)(1.10 * bcs[prev_bcspos].uncompressed_len);

    lzo1x_1_compress(bcs[prev_bcspos].data, bcs[prev_bcspos].uncompressed_len, tmp_buf, &dst_len, workmem);

    free(bcs[prev_bcspos].data);
    bcs[prev_bcspos].data = (uint8 *)realloc(tmp_buf, dst_len);
    bcs[prev_bcspos].compressed_len = dst_len;
   }
   else if(SRWCompressor == SRW_COMPRESSOR_BLZ)
   {
    blz_pack_t workmem;

    uint8 * tmp_buf = (uint8 *)malloc((size_t)(bcs[prev_bcspos].uncompressed_len + blz_pack_extra));
    uint32 dst_len = bcs[prev_bcspos].uncompressed_len + blz_pack_extra;

    dst_len = blz_pack(bcs[prev_bcspos].data, bcs[prev_bcspos].uncompressed_len, tmp_buf, &workmem);

    free(bcs[prev_bcspos].data);
    bcs[prev_bcspos].data = (uint8 *)realloc(tmp_buf, dst_len);
    bcs[prev_bcspos].compressed_len = dst_len;
   }
  }

  if(MDFNMOV_IsRecording())
   bcs[bcspos].MovieLove = MDFNMOV_GrabRewindJoy();
 }
 return(0);
}

void MDFNI_EnableStateRewind(int enable)
{
 if(MDFNGameInfo)
  MDFN_StateEvilEnd();

 EvilEnabled = enable;

 if(MDFNGameInfo)
  MDFN_StateEvilBegin();
}
