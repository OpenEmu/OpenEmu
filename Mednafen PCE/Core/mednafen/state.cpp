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
#include <time.h>

#include <trio/trio.h>
#include "driver.h"
#include "general.h"
#include "state.h"
#include "movie.h"
#include "netplay.h"
#include "video.h"
#include "video/resize.h"

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

 while(sf->size || sf->name)
 {
  SFORMAT *sub_sf = saved_sf;
  while(sub_sf->size || sub_sf->name)
  {
   if(sf != sub_sf)
   {
    if(!strncmp(sf->name, sub_sf->name, 32))
    {
     printf("Duplicate state variable name: %.32s\n", sf->name);
    }
   }
   sub_sf++;
  }

  sf++;
 }
 return(1);
}


static bool SubWrite(StateMem *st, SFORMAT *sf, int data_only, const char *name_prefix = NULL)
{
 // FIXME?  It's kind of slow, and we definitely don't want it on with state rewinding...
 if(!data_only) 
  ValidateSFStructure(sf);

 while(sf->size || sf->name)	// Size can sometimes be zero, so also check for the text name.  These two should both be zero only at the end of a struct.
 {
  if(!sf->size || !sf->v)
  {
   sf++;
   continue;
  }

  if(sf->size == (uint32)~0)		/* Link to another struct.	*/
  {
   if(!SubWrite(st, (SFORMAT *)sf->v, data_only, name_prefix))
    return(0);

   sf++;
   continue;
  }

  int32 bytesize = sf->size;

  // If we're only saving the raw data, and we come across a bool type, we save it as it is in memory, rather than converting it to
  // 1-byte.  In the SFORMAT structure, the size member for bool entries is the number of bool elements, not the total in-memory size,
  // so we adjust it here.
  if(data_only && (sf->flags & MDFNSTATE_BOOL))
  {
   bytesize *= sizeof(bool);
  }
  
  if(!data_only)
  {
   char nameo[1 + 256];
   int slen;

   slen = trio_snprintf(nameo + 1, 256, "%s%s", name_prefix ? name_prefix : "", sf->name);
   nameo[0] = slen;

   if(slen >= 255)
   {
    printf("Warning:  state variable name possibly too long: %s %s %s %d\n", sf->name, name_prefix, nameo, slen);
    slen = 255;
   }

   smem_write(st, nameo, 1 + nameo[0]);
   smem_write32le(st, bytesize);

   /* Flip the byte order... */
   if(sf->flags & MDFNSTATE_BOOL)
   {

   }
   else if(sf->flags & MDFNSTATE_RLSB64)
    Endian_A64_NE_to_LE(sf->v, bytesize / sizeof(uint64));
   else if(sf->flags & MDFNSTATE_RLSB32)
    Endian_A32_NE_to_LE(sf->v, bytesize / sizeof(uint32));
   else if(sf->flags & MDFNSTATE_RLSB16)
    Endian_A16_NE_to_LE(sf->v, bytesize / sizeof(uint16));
   else if(sf->flags & RLSB)
    Endian_V_NE_to_LE(sf->v, bytesize);
  }
    
  // Special case for the evil bool type, to convert bool to 1-byte elements.
  // Don't do it if we're only saving the raw data.
  if((sf->flags & MDFNSTATE_BOOL) && !data_only)
  {
   for(int32 bool_monster = 0; bool_monster < bytesize; bool_monster++)
   {
    uint8 tmp_bool = ((bool *)sf->v)[bool_monster];
    //printf("Bool write: %.31s\n", sf->name);
    smem_write(st, &tmp_bool, 1);
   }
  }
  else
   smem_write(st, (uint8 *)sf->v, bytesize);

  if(!data_only)
  {
   /* Now restore the original byte order. */
   if(sf->flags & MDFNSTATE_BOOL)
   {

   }
   else if(sf->flags & MDFNSTATE_RLSB64)
    Endian_A64_LE_to_NE(sf->v, bytesize / sizeof(uint64));
   else if(sf->flags & MDFNSTATE_RLSB32)
    Endian_A32_LE_to_NE(sf->v, bytesize / sizeof(uint32));
   else if(sf->flags & MDFNSTATE_RLSB16)
    Endian_A16_LE_to_NE(sf->v, bytesize / sizeof(uint16));
   else if(sf->flags & RLSB)
    Endian_V_LE_to_NE(sf->v, bytesize);
  }
  sf++; 
 }

 return(TRUE);
}

static int WriteStateChunk(StateMem *st, const char *sname, SFORMAT *sf, int data_only)
{
 int32 data_start_pos;
 int32 end_pos;

 if(!data_only)
 {
  uint8 sname_tmp[32];

  memset(sname_tmp, 0, sizeof(sname_tmp));
  strncpy((char *)sname_tmp, sname, 32);

  if(strlen(sname) > 32)
   printf("Warning: section name is too long: %s\n", sname);

  smem_write(st, sname_tmp, 32);

  smem_write32le(st, 0);                // We'll come back and write this later.
 }

 data_start_pos = smem_tell(st);

 if(!SubWrite(st, sf, data_only))
  return(0);

 end_pos = smem_tell(st);

 if(!data_only)
 {
  smem_seek(st, data_start_pos - 4, SEEK_SET);
  smem_write32le(st, end_pos - data_start_pos);
  smem_seek(st, end_pos, SEEK_SET);
 }

 return(end_pos - data_start_pos);
}

struct compare_cstr
{
 bool operator()(const char *s1, const char *s2) const
 {
  return(strcmp(s1, s2) < 0);
 }
};

typedef std::map<const char *, SFORMAT *, compare_cstr> SFMap_t;

static void MakeSFMap(SFORMAT *sf, SFMap_t &sfmap)
{
 while(sf->size || sf->name) // Size can sometimes be zero, so also check for the text name.  These two should both be zero only at the end of a struct.
 {
  if(!sf->size || !sf->v)
  {
   sf++;
   continue;
  }

  if(sf->size == (uint32)~0)            /* Link to another SFORMAT structure. */
   MakeSFMap((SFORMAT *)sf->v, sfmap);
  else
  {
   assert(sf->name);

   if(sfmap.find(sf->name) != sfmap.end())
    printf("Duplicate save state variable in internal emulator structures(CLUB THE PROGRAMMERS WITH BREADSTICKS): %s\n", sf->name);

   sfmap[sf->name] = sf;
  }

  sf++;
 }
}

// Fast raw chunk reader
static void DOReadChunk(StateMem *st, SFORMAT *sf)
{
 while(sf->size || sf->name)       // Size can sometimes be zero, so also check for the text name.  
				// These two should both be zero only at the end of a struct.
 {
  if(!sf->size || !sf->v)
  {
   sf++;
   continue;
  }

  if(sf->size == (uint32) ~0) // Link to another SFORMAT struct
  {
   DOReadChunk(st, (SFORMAT *)sf->v);
   sf++;
   continue;
  }

  int32 bytesize = sf->size;

  // Loading raw data, bool types are stored as they appear in memory, not as single bytes in the full state format.
  // In the SFORMAT structure, the size member for bool entries is the number of bool elements, not the total in-memory size,
  // so we adjust it here.
  if(sf->flags & MDFNSTATE_BOOL)
   bytesize *= sizeof(bool);

  smem_read(st, (uint8 *)sf->v, bytesize);
  sf++;
 }
}

static int ReadStateChunk(StateMem *st, SFORMAT *sf, int size, int data_only)
{
 int temp;

 if(data_only)
 {
  DOReadChunk(st, sf);
 }
 else
 {
  SFMap_t sfmap;
  SFMap_t sfmap_found;	// Used for identifying variables that are missing in the save state.

  MakeSFMap(sf, sfmap);

  temp = smem_tell(st);
  while(smem_tell(st) < (temp + size))
  {
   uint32 recorded_size;	// In bytes
   uint8 toa[1 + 256];	// Don't change to char unless cast toa[0] to unsigned to smem_read() and other places.

   if(smem_read(st, toa, 1) != 1)
   {
    puts("Unexpected EOF");
    return(0);
   }

   if(smem_read(st, toa + 1, toa[0]) != toa[0])
   {
    puts("Unexpected EOF?");
    return 0;
   }

   toa[1 + toa[0]] = 0;

   smem_read32le(st, &recorded_size);

   SFMap_t::iterator sfmit;

   sfmit = sfmap.find((char *)toa + 1);

   if(sfmit != sfmap.end())
   {
    SFORMAT *tmp = sfmit->second;
    uint32 expected_size = tmp->size;	// In bytes

    if(recorded_size != expected_size)
    {
     printf("Variable in save state wrong size: %s.  Need: %d, got: %d\n", toa + 1, expected_size, recorded_size);
     if(smem_seek(st, recorded_size, SEEK_CUR) < 0)
     {
      puts("Seek error");
      return(0);
     }
    }
    else
    {
     sfmap_found[tmp->name] = tmp;

     smem_read(st, (uint8 *)tmp->v, expected_size);

     if(tmp->flags & MDFNSTATE_BOOL)
     {
      // Converting downwards is necessary for the case of sizeof(bool) > 1
      for(int32 bool_monster = expected_size - 1; bool_monster >= 0; bool_monster--)
      {
       ((bool *)tmp->v)[bool_monster] = ((uint8 *)tmp->v)[bool_monster];
      }
     }
     if(tmp->flags & MDFNSTATE_RLSB64)
      Endian_A64_LE_to_NE(tmp->v, expected_size / sizeof(uint64));
     else if(tmp->flags & MDFNSTATE_RLSB32)
      Endian_A32_LE_to_NE(tmp->v, expected_size / sizeof(uint32));
     else if(tmp->flags & MDFNSTATE_RLSB16)
      Endian_A16_LE_to_NE(tmp->v, expected_size / sizeof(uint16));
     else if(tmp->flags & RLSB)
      Endian_V_LE_to_NE(tmp->v, expected_size);
    }
   }
   else
   {
    printf("Unknown variable in save state: %s\n", toa + 1);
    if(smem_seek(st, recorded_size, SEEK_CUR) < 0)
    {
     puts("Seek error");
     return(0);
    }
   }
  } // while(...)

  for(SFMap_t::const_iterator it = sfmap.begin(); it != sfmap.end(); it++)
  {
   if(sfmap_found.find(it->second->name) == sfmap_found.end())
   {
    printf("Variable missing from save state: %s\n", it->second->name);
   }
  }

  assert(smem_tell(st) == (temp + size));
 }
 return 1;
}

static int CurrentState = 0;
static int RecentlySavedState = -1;

/* This function is called by the game driver(NES, GB, GBA) to save a state. */
int MDFNSS_StateAction(StateMem *st, int load, int data_only, std::vector <SSDescriptor> &sections)
{
 std::vector<SSDescriptor>::iterator section;

 if(load)
 {
  if(data_only)
  {
   for(section = sections.begin(); section != sections.end(); section++)
   {
     ReadStateChunk(st, section->sf, ~0, 1);
   }
  }
  else
  {
   char sname[32];

   for(section = sections.begin(); section != sections.end(); section++)
   {
    int found = 0;
    uint32 tmp_size;
    uint32 total = 0;

    while(smem_read(st, (uint8 *)sname, 32) == 32)
    {
     if(smem_read32le(st, &tmp_size) != 4)
      return(0);

     total += tmp_size + 32 + 4;

     // Yay, we found the section
     if(!strncmp(sname, section->name, 32))
     {
      if(!ReadStateChunk(st, section->sf, tmp_size, 0))
      {
       printf("Error reading chunk: %s\n", section->name);
       return(0);
      }
      found = 1;
      break;
     } 
     else
     {
      // puts("SEEK");
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
     printf("Section missing:  %.32s\n", section->name);
     return(0);
    }
   }
  }
 }
 else
 {
  for(section = sections.begin(); section != sections.end(); section++)
  {
   if(!WriteStateChunk(st, section->name, section->sf, data_only))
    return(0);
  }
 }

 return(1);
}

int MDFNSS_StateAction(StateMem *st, int load, int data_only, SFORMAT *sf, const char *name, bool optional)
{
 std::vector <SSDescriptor> love;

 love.push_back(SSDescriptor(sf, name, optional));
 return(MDFNSS_StateAction(st, load, data_only, love));
}

int MDFNSS_SaveSM(StateMem *st, int wantpreview_and_ts, int data_only, const MDFN_Surface *surface, const MDFN_Rect *DisplayRect, const MDFN_Rect *LineWidths)
{
	static const char *header_magic = "MDFNSVST";
        uint8 header[32];
	int neowidth = 0, neoheight = 0;

	memset(header, 0, sizeof(header));

	if(wantpreview_and_ts)
	{
	 bool is_multires = FALSE;

	 // We'll want to use the nominal width if the source rectangle is > 25% off on either axis, or the source image has
	 // multiple horizontal resolutions.
	 neowidth = MDFNGameInfo->nominal_width;
	 neoheight = MDFNGameInfo->nominal_height;

	 if(LineWidths[0].w != ~0)
 	 {
	  uint32 first_w = LineWidths[DisplayRect->y].w;

	  for(int y = 0; y < DisplayRect->h; y++)
	   if(LineWidths[DisplayRect->y + y].w != first_w)
	   {
	    puts("Multires!");
	    is_multires = TRUE;
	   }
	 }

	 if(!is_multires)
	 {
	  if(((double)DisplayRect->w / MDFNGameInfo->nominal_width) > 0.75  && ((double)DisplayRect->w / MDFNGameInfo->nominal_width) < 1.25)
	   neowidth = DisplayRect->w;

          if(((double)DisplayRect->h / MDFNGameInfo->nominal_height) > 0.75  && ((double)DisplayRect->h / MDFNGameInfo->nominal_height) < 1.25)
	   neoheight = DisplayRect->h;
	 }
	}

	if(!data_only)
	{
	 memcpy(header, header_magic, 8);

	 if(wantpreview_and_ts)
	  MDFN_en64lsb(header + 8, time(NULL));

	 MDFN_en32lsb(header + 16, MEDNAFEN_VERSION_NUMERIC);
	 MDFN_en32lsb(header + 24, neowidth);
	 MDFN_en32lsb(header + 28, neoheight);
	 smem_write(st, header, 32);
	}

	if(wantpreview_and_ts)
	{
         uint8 *previewbuffer = (uint8 *)malloc(4 * neowidth * neoheight);
	 MDFN_Surface *dest_surface = new MDFN_Surface((uint32 *)previewbuffer, neowidth, neoheight, neowidth, surface->format);
	 MDFN_Rect dest_rect;

	 dest_rect.x = 0;
	 dest_rect.y = 0;
	 dest_rect.w = neowidth;
	 dest_rect.h = neoheight;

	 MDFN_ResizeSurface(surface, DisplayRect, (LineWidths[0].w != ~0) ? LineWidths : NULL, dest_surface, &dest_rect);

	 {
	  uint32 a, b = 0;
	  for(a = 0; a < neowidth * neoheight * 4; a+=4)
	  {
	   uint32 c = *(uint32 *)&previewbuffer[a];
	   int nr, ng, nb;

	   surface->DecodeColor(c, nr, ng, nb);

	   previewbuffer[b + 0] = nr;
	   previewbuffer[b + 1] = ng;
           previewbuffer[b + 2] = nb;
	   b += 3;
	  }
	 }

         smem_write(st, previewbuffer, 3 * neowidth * neoheight);

	 free(previewbuffer);
	 delete dest_surface;
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

int MDFNSS_Save(const char *fname, const char *suffix, const MDFN_Surface *surface, const MDFN_Rect *DisplayRect, const MDFN_Rect *LineWidths)
{
	StateMem st;

	memset(&st, 0, sizeof(StateMem));


	if(!MDFNGameInfo->StateAction)
	{
	 MDFN_DispMessage(_("Module \"%s\" doesn't support save states."), MDFNGameInfo->shortname);
	 return(0);
	}

	if(!MDFNSS_SaveSM(&st, (DisplayRect && LineWidths), 0, surface, DisplayRect, LineWidths))
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
int MDFNSS_SaveFP(gzFile fp, const MDFN_Surface *surface, const MDFN_Rect *DisplayRect, const MDFN_Rect *LineWidths)
{
 StateMem st;

 memset(&st, 0, sizeof(StateMem));

 if(!MDFNSS_SaveSM(&st, (DisplayRect && LineWidths), 0, surface, DisplayRect, LineWidths))
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

         if(memcmp(header, "MEDNAFENSVESTATE", 16) && memcmp(header, "MDFNSVST", 8))
          return(0);

	 stateversion = MDFN_de32lsb(header + 16);
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

        if(!MDFNGameInfo->StateAction)
        {
         MDFN_DispMessage(_("Module \"%s\" doesn't support save states."), MDFNGameInfo->shortname);
         return(0);
        }

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

        if(!MDFNGameInfo->StateAction) 
         return;


	for(int ssel = 0; ssel < 10; ssel++)
        {
	 struct stat stat_buf;

	 SaveStateStatus[ssel] = 0;
	 //printf("%s\n", MDFN_MakeFName(MDFNMKF_STATE, ssel, 0).c_str());
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

void MDFNSS_GetStateInfo(const char *filename, StateStatusStruct *status)
{
 gzFile fp;
 uint32 StateShowPBWidth;
 uint32 StateShowPBHeight;
 uint8 *previewbuffer = NULL;

 fp = gzopen(filename, "rb");
 if(fp)
 {
  uint8 header[32];

  gzread(fp, header, 32);
  uint32 width = MDFN_de32lsb(header + 24);
  uint32 height = MDFN_de32lsb(header + 28);

  if(width > 1024) width = 1024;
  if(height > 1024) height = 1024;

  if(!(previewbuffer = (uint8 *)MDFN_malloc(3 * width * height, _("Save state preview buffer"))))
  {
   StateShowPBWidth = 0;
   StateShowPBHeight = 0;
  }
  else
  {
   gzread(fp, previewbuffer, 3 * width * height);

   StateShowPBWidth = width;
   StateShowPBHeight = height;
  }
  gzclose(fp);
 }
 else
 {
  StateShowPBWidth = MDFNGameInfo->nominal_width;
  StateShowPBHeight = MDFNGameInfo->nominal_height;
 }

 status->gfx = previewbuffer;
 status->w = StateShowPBWidth;
 status->h = StateShowPBHeight;
}

void MDFNI_SelectState(int w)
{
 if(!MDFNGameInfo->StateAction) 
  return;


 if(w == -1) 
 {  
  MDFND_SetStateStatus(NULL);
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

 MDFN_ResetMessages();

 StateStatusStruct *status = (StateStatusStruct*)MDFN_calloc(1, sizeof(StateStatusStruct), _("Save state status"));
 
 memcpy(status->status, SaveStateStatus, 10 * sizeof(int));

 status->current = CurrentState;
 status->recently_saved = RecentlySavedState;

 MDFNSS_GetStateInfo(MDFN_MakeFName(MDFNMKF_STATE,CurrentState,NULL).c_str(), status);
 MDFND_SetStateStatus(status);
}  

void MDFNI_SaveState(const char *fname, const char *suffix, const MDFN_Surface *surface, const MDFN_Rect *DisplayRect, const MDFN_Rect *LineWidths)
{
 if(!MDFNGameInfo->StateAction) 
  return;

 if(MDFNnetplay && (MDFNGameInfo->SaveStateAltersState == true))
 {
  char sb[256];
  trio_snprintf(sb, sizeof(sb), _("Module %s is not compatible with manual state saving during netplay."), MDFNGameInfo->shortname);
  MDFND_NetplayText((const uint8*)sb, false);
  return;
 }

 MDFND_SetStateStatus(NULL);
 MDFNSS_Save(fname, suffix, surface, DisplayRect, LineWidths);
}

void MDFNI_LoadState(const char *fname, const char *suffix)
{
 if(!MDFNGameInfo->StateAction) 
  return;

 MDFND_SetStateStatus(NULL);

 /* For network play and movies, be load the state locally, and then save the state to a temporary buffer,
    and send or record that.  This ensures that if an older state is loaded that is missing some
    information expected in newer save states, desynchronization won't occur(at least not
    from this ;)).
 */
 if(MDFNSS_Load(fname, suffix))
 {
  if(MDFNnetplay)
  {
   NetplaySendState();
  }

  if(MDFNMOV_IsRecording())
   MDFNMOV_RecordState();
 }
}

#include "compress/minilzo.h"
#include "compress/quicklz.h"
#include "compress/blz.h"

static union
{
 char qlz_scratch_compress[/*QLZ_*/SCRATCH_COMPRESS];
 char qlz_scratch_decompress[/*QLZ_*/SCRATCH_DECOMPRESS];
};

enum
{
 SRW_COMPRESSOR_MINILZO = 0,
 SRW_COMPRESSOR_QUICKLZ,
 SRW_COMPRESSOR_BLZ
};

struct StateMemPacket
{
	uint8 *data;
	uint32 compressed_len;
	uint32 uncompressed_len;

	StateMem MovieLove;
};

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
    dst_len = qlz_decompress((char*)bcs[bcspos].data, tmp_buf, qlz_scratch_decompress);
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
    uint32 dst_len;
    uint8 *tmp_buf = (uint8 *)malloc(bcs[prev_bcspos].uncompressed_len + 400);

    dst_len = qlz_compress(bcs[prev_bcspos].data, (char*)tmp_buf, bcs[prev_bcspos].uncompressed_len, qlz_scratch_compress);

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
 if(!MDFNGameInfo->StateAction) 
  return;

 MDFN_StateEvilEnd();

 EvilEnabled = enable;

 MDFN_StateEvilBegin();
}
