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
#include <stdarg.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>

#ifdef HAVE_MMAP
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif


#include <zlib.h>
#include "compress/unzip.h"

#include "file.h"
#include "endian.h"
#include "memory.h"
#include "general.h"

static const int64 MaxROMImageSize = (int64)1 << 26; // 2 ^ 26 = 64MiB

enum
{
 MDFN_FILETYPE_PLAIN = 0,
 MDFN_FILETYPE_GZIP = 1,
 MDFN_FILETYPE_ZIP = 2,
};

static bool ApplyIPS(FILE *ips, MDFNFILE *dest, const char *path)
{
 uint8 header[5];
 uint32 count=0;
 
 MDFN_printf(_("Applying IPS file \"%s\"...\n"), path);
 MDFN_indent(1);
 if(fread(header,1,5,ips) != 5 || memcmp(header, "PATCH", 5))
 {
  fclose(ips);
  MDFN_PrintError(_("IPS file \"%s\" is not valid!"), path);
  MDFN_indent(-1);
  return(0);
 }

 while(fread(header,1,3,ips)==3)
 {
  uint32 offset=(header[0]<<16)|(header[1]<<8)|header[2];
  uint32 size;

  if(!memcmp(header,"EOF",3))
  {
   MDFN_printf(_("IPS EOF:  Did %d patches\n\n"),count);
   MDFN_indent(-1);
   fclose(ips);
   return(1);
  }

  size=fgetc(ips)<<8;
  size|=fgetc(ips);
  if(!size)	/* RLE */
  {
   uint8 *start;
   uint8 b;
   size=fgetc(ips)<<8;
   size|=fgetc(ips);

   // Is this right?
   if(!size)
    size = 65536;

   //MDFN_printf("  Offset: %8d  Size: %5d RLE\n",offset,size);

   if((offset+size)>dest->size)
   {
    uint8 *tmp;

    if((offset + size) > MaxROMImageSize)
    {
     MDFN_PrintError(_("ROM image will be too large after IPS patch; maximum size allowed is %llu bytes."), (unsigned long long)MaxROMImageSize);
     fclose(ips);
     return(0);
    }

    tmp=(uint8 *)MDFN_realloc(dest->data, offset+size, _("file read buffer"));
    if(!tmp)
    {
     fclose(ips);
     return(0);
    }

    dest->size=offset+size;
    dest->data=tmp;
    memset(dest->data+dest->size,0,offset+size-dest->size);
   }

   b=fgetc(ips);
   start=dest->data+offset;

   while(size--)
   {
    *start=b;
    start++;
   }

  }
  else		/* Normal patch */
  {
   //MDFN_printf("  Offset: %8d  Size: %5d\n",offset,size);
   if((offset+size)>dest->size)
   {
    uint8 *tmp;
    
    tmp=(uint8 *)MDFN_realloc(dest->data, offset+size, _("file read buffer"));
    if(!tmp)
    {
     fclose(ips);
     MDFN_indent(-1);
     return(0);
    }
    dest->data=tmp;
    memset(dest->data+dest->size,0,offset+size-dest->size);
   }
   fread(dest->data+offset,1,size,ips);
  }
  count++;
 }
 fclose(ips);

 MDFN_printf(_("Warning:  IPS ended without an EOF chunk.\n"));
 MDFN_printf(_("IPS EOF:  Did %d patches\n\n"), count);

 MDFN_indent(-1);

 return(1);
}

// This function should ALWAYS close the system file "descriptor"(gzip library, zip library, or FILE *) it's given,
// even if it errors out.
static MDFNFILE *MakeMemWrap(void *tz, int type)
{
 MDFNFILE *tmp = NULL;

 if(!(tmp=(MDFNFILE *)MDFN_malloc(sizeof(MDFNFILE), _("file read buffer"))))
  goto doret;
 
 #ifdef HAVE_MMAP
 tmp->is_mmap = FALSE;
 #endif

 tmp->location = 0;

 if(type == MDFN_FILETYPE_PLAIN)
 {
  fseek((FILE *)tz,0,SEEK_END);
  tmp->size=ftell((FILE *)tz);
  fseek((FILE *)tz,0,SEEK_SET);

  if(tmp->size > MaxROMImageSize)
  {
   MDFN_PrintError(_("ROM image is too large; maximum size allowed is %llu bytes."), (unsigned long long)MaxROMImageSize);
   free(tmp);
   tmp = NULL;
   goto doret;
  }

  #ifdef HAVE_MMAP
  if((void *)-1 != (tmp->data = (uint8 *)mmap(NULL, tmp->size, PROT_READ, MAP_SHARED, fileno((FILE *)tz), 0)))
  {
   //puts("mmap'ed");
   tmp->is_mmap = TRUE;
   #ifdef HAVE_MADVISE
   if(0 == madvise(tmp->data, tmp->size, MADV_SEQUENTIAL | MADV_WILLNEED))
   {
    //puts("madvised");
   }
   #endif
  }
  else
  {
  #endif
   if(!(tmp->data=(uint8*)MDFN_malloc(tmp->size, _("file read buffer"))))
   {
    free(tmp);
    tmp=0;
    goto doret;
   }
   if((int64)fread(tmp->data, 1, tmp->size, (FILE *)tz) != tmp->size)
   {
    free(tmp->data);
    free(tmp);
    tmp = NULL;
    goto doret;
   }
  #ifdef HAVE_MMAP
  }
  #endif
 }
 else if(type == MDFN_FILETYPE_GZIP)
 {
  uint32_t cur_size = 0;
  uint32_t cur_alloced = 65536;
  int howmany;

  if(!(tmp->data=(uint8*)MDFN_malloc(cur_alloced, _("file read buffer"))))
  {
   free(tmp);
   tmp = NULL;
   goto doret;
  }

  while((howmany = gzread(tz, tmp->data + cur_size, cur_alloced - cur_size)) > 0)
  {
   cur_size += howmany;
   cur_alloced <<= 1;
   if(!(tmp->data = (uint8 *)MDFN_realloc(tmp->data, cur_alloced, _("file read buffer")))) 
   {
    free(tmp);
    tmp = NULL;
    goto doret;
   }
  }

  if(!(tmp->data = (uint8 *)MDFN_realloc(tmp->data, cur_size, _("file read buffer")))) 
  {
   free(tmp);
   tmp = NULL;
   goto doret;
  }

  tmp->size = cur_size;
 }
 else if(type == MDFN_FILETYPE_ZIP)
 {
  unz_file_info ufo; 
  unzGetCurrentFileInfo(tz,&ufo,0,0,0,0,0,0);  

  tmp->size=ufo.uncompressed_size;

  if(tmp->size > MaxROMImageSize)
  {
   MDFN_PrintError(_("ROM image is too large; maximum size allowed is %llu bytes."), (unsigned long long)MaxROMImageSize);
   free(tmp);
   tmp = NULL;
   goto doret;
  }

  if(!(tmp->data=(uint8 *)MDFN_malloc(ufo.uncompressed_size, _("file read buffer"))))
  {
   free(tmp);
   tmp = NULL;
   goto doret;
  }
  unzReadCurrentFile(tz,tmp->data,ufo.uncompressed_size);
 }

 doret:

 if(type == MDFN_FILETYPE_PLAIN)
 {
  fclose((FILE *)tz);
 }
 else if(type == MDFN_FILETYPE_GZIP)
 {
  gzclose(tz);
 }
 else if(type == MDFN_FILETYPE_ZIP)
 {
  unzCloseCurrentFile(tz);
  unzClose(tz);
 }

 return(tmp);
}

#ifndef __GNUC__
 #define strcasecmp strcmp
#endif

static const char *unzErrorString(int error_code)
{
 if(error_code == UNZ_OK)
  return("ZIP OK");
 else if(error_code == UNZ_END_OF_LIST_OF_FILE)
  return("ZIP End of file list");
 else if(error_code == UNZ_EOF)
  return("ZIP EOF");
 else if(error_code == UNZ_PARAMERROR)
  return("ZIP Parameter error");
 else if(error_code == UNZ_BADZIPFILE)
  return("ZIP file bad");
 else if(error_code == UNZ_INTERNALERROR)
  return("ZIP Internal error");
 else if(error_code == UNZ_CRCERROR)
  return("ZIP CRC error");
 else if(error_code == UNZ_ERRNO)
  return(strerror(errno));
 else
  return("ZIP Unknown");
}

MDFNFILE * MDFN_fopen(const char *path, const char *ipsfn, const char *mode, const char *ext)
{
 MDFNFILE *fceufp = NULL;
 void *t;
 unzFile tz;

 // Try opening it as a zip file first
 if((tz=unzOpen(path)))
 {
  char tempu[1024];
  int errcode;

  if((errcode = unzGoToFirstFile(tz)) != UNZ_OK)
  {
   MDFN_PrintError(_("Could not seek to first file in ZIP archive: %s"), unzErrorString(errcode));
   unzClose(tz);
   return(NULL);
  }

  if(ext)
  {
   bool FileFound = FALSE;
   while(!FileFound)
   {
    size_t tempu_strlen;
    size_t ttmeow;
    const char *extpoo = ext;

    if((errcode = unzGetCurrentFileInfo(tz, 0, tempu, 1024, 0, 0, 0, 0)) != UNZ_OK)
    {
     MDFN_PrintError(_("Could not get file information in ZIP archive: %s"), unzErrorString(errcode));
     unzClose(tz);
     return(NULL);
    }

    tempu[1023] = 0;
    tempu_strlen = strlen(tempu);

    while((ttmeow = strlen(extpoo)) && !FileFound)
    {
     if(tempu_strlen >= ttmeow)
     {
      if(!strcasecmp(tempu + tempu_strlen - ttmeow, extpoo))
       FileFound = TRUE;
     }
     extpoo += ttmeow + 1;
    }

    if(FileFound)
     break;

    if((errcode = unzGoToNextFile(tz)) != UNZ_OK)
    { 
     if(errcode != UNZ_END_OF_LIST_OF_FILE)
     {
      MDFN_PrintError(_("Error seeking to next file in ZIP archive: %s"), unzErrorString(errcode));
      unzClose(tz);
      return(NULL);
     }

     if((errcode = unzGoToFirstFile(tz)) != UNZ_OK)
     {
      MDFN_PrintError(_("Could not seek to first file in ZIP archive: %s"), unzErrorString(errcode));
      unzClose(tz);
      return(NULL);
     }
     break;     
    }

   } // end to while(!FileFound)
  } // end to if(ext)

  if((errcode = unzOpenCurrentFile(tz)) != UNZ_OK)
  {
   MDFN_PrintError(_("Could not open file in ZIP archive: %s"), unzErrorString(errcode));
   unzClose(tz);
   return(NULL);
  }

  if(!(fceufp=MakeMemWrap(tz, MDFN_FILETYPE_ZIP)))
   return(0);

  char *ld = strrchr(tempu, '.');
  fceufp->ext = strdup(ld ? ld + 1 : "");
 }
 else // If it's not a zip file, handle it as...another type of file!
 {
  t = fopen(path,"rb");
  if(!t)
  {
   MDFN_PrintError(_("Error opening \"%s\": %s"), path, strerror(errno));
   return(0);
  }

  uint32 gzmagic;

  gzmagic=fgetc((FILE *)t);
  gzmagic|=fgetc((FILE *)t)<<8;
  gzmagic|=fgetc((FILE *)t)<<16;

  if(gzmagic!=0x088b1f)   /* Not gzip... */
  {
   fseek((FILE *)t,0,SEEK_SET);

   if(!(fceufp = MakeMemWrap(t, 0)))
    return(0);

   const char *ld = strrchr(path, '.');
   fceufp->ext = strdup(ld ? ld + 1 : "");
  }
  else                  /* Probably gzip */
  {
   int fd;

   fd = dup(fileno( (FILE *)t));
   lseek(fd, 0, SEEK_SET);

   if(!(t=gzdopen(fd, mode)))
   {
    close(fd);
    return(0);
   }

   if(!(fceufp = MakeMemWrap(t, 1)))
   {
    gzclose(t);
    return(0);
   }

   char *tmp_path = strdup(path);
   char *ld = strrchr(tmp_path, '.');

   if(ld && ld > tmp_path)
   {
    char *last_ld = ld;
    *ld = 0;
    ld = strrchr(tmp_path, '.');
    if(!ld) { *last_ld = '.'; ld = last_ld; }
   }
   fceufp->ext = strdup(ld ? ld + 1 : "");
   free(tmp_path);
  } // End gzip handling
 } // End normal and gzip file handling else to zip

 if(strchr(mode,'r') && ipsfn)
 {
  FILE *ipsfile = fopen(ipsfn, "rb");

  if(!ipsfile)
  {
   if(errno != ENOENT)
   {
    MDFN_PrintError(_("Error opening \"%s\": %s"), path, strerror(errno));
    MDFN_fclose(fceufp);
    return(NULL);
   }
  }
  else
  {
   #ifdef HAVE_MMAP
   // If the file is mmap()'d, move it to malloc()'d RAM
   if(fceufp->is_mmap)
   {
    void *tmp_ptr = MDFN_malloc(fceufp->size, _("file read buffer"));
    if(!tmp_ptr)
    {
     MDFN_fclose(fceufp);
     fclose(ipsfile);
    }
    memcpy(tmp_ptr, fceufp->data, fceufp->size);

    munmap(fceufp->data, fceufp->size);

    fceufp->is_mmap = FALSE;
    fceufp->data = (uint8 *)tmp_ptr;
   }
   #endif

   if(!ApplyIPS(ipsfile, fceufp, ipsfn))
   {
    MDFN_fclose(fceufp);
    return(0);
   }
  }
 }

 return(fceufp);
}

int MDFN_fclose(MDFNFILE *fp)
{
 if(fp->ext)
  free(fp->ext);

 if(fp->data)
 {
  #if HAVE_MMAP
  if(fp->is_mmap) 
   munmap(fp->data, fp->size);
  else
  #endif
   free(fp->data);
 }
 free(fp);

 return(1);
}

uint64 MDFN_fread(void *ptr, size_t size, size_t nmemb, MDFNFILE *fp)
{
 uint32 total=size*nmemb;

 if(fp->location>=fp->size) return 0;

 if((fp->location+total)>fp->size)
 {
  int ak=fp->size-fp->location;
  memcpy((uint8*)ptr,fp->data+fp->location,ak);
  fp->location=fp->size;
  return(ak/size);
 }
 else
 {
  memcpy((uint8*)ptr,fp->data+fp->location,total);
  fp->location+=total;
  return nmemb;
 }
}

uint64 MDFN_fwrite(void *ptr, size_t size, size_t nmemb, MDFNFILE *fp)
{
 return(0); // TODO
}

int MDFN_fseek(MDFNFILE *fp, int64 offset, int whence)
{
  switch(whence)
  {
   case SEEK_SET:if(offset>=fp->size)
                  return(-1);
                 fp->location=offset;break;
   case SEEK_CUR:if(offset+fp->location>fp->size)
                  return (-1);
                 fp->location+=offset;
                 break;
  }    
  return 0;
}

uint64 MDFN_ftell(MDFNFILE *fp)
{
 return(fp->location);
}

void MDFN_rewind(MDFNFILE *fp)
{
 fp->location = 0;
}

int MDFN_read16le(uint16 *val, MDFNFILE *fp)
{
 if((fp->location + 2) > fp->size)
  return 0;

 *val = MDFN_de16lsb(fp->data + fp->location);

 fp->location += 2;

 return(1);
}

int MDFN_read32le(uint32 *val, MDFNFILE *fp)
{
 if((fp->location + 4) > fp->size)
  return 0;

 *val = MDFN_de32lsb(fp->data + fp->location);

 fp->location += 4;

 return(1);
}

char *MDFN_fgets(char *s, int size, MDFNFILE *fp)
{
 int pos = 0;

 if(!size) return(NULL);

 if(fp->location >= fp->size) return(NULL);

 while(pos < (size - 1) && fp->location < fp->size)
 {
  int v = fp->data[fp->location];
  s[pos] = v;
  fp->location++;
  pos++;
  if(v == '\n') break;
 }

 if(size)
  s[pos] = 0;
 return(s);
}

int MDFN_fgetc(MDFNFILE *fp)
{
 if(fp->location<fp->size)
  return fp->data[fp->location++];
 return EOF;
}

uint64 MDFN_fgetsize(MDFNFILE *fp)
{
 return fp->size;
}

int MDFN_fisarchive(MDFNFILE *fp)
{
 return 0;
}

static bool MDFN_DumpToFileReal(const char *filename, int compress, const std::vector<PtrLengthPair> &pearpairs)
{
 if(MDFN_GetSettingB("filesys.disablesavegz"))
  compress = 0;

 if(compress)
 {
  char mode[64];
  gzFile gp;

  snprintf(mode, 64, "wb%d", compress);

  gp = gzopen(filename, mode);

  if(!gp)
  {
   MDFN_PrintError(_("Error opening \"%s\": %m"), filename, errno);
   return(0);
  }

  for(unsigned int i = 0; i < pearpairs.size(); i++)
  {
   const void *data = pearpairs[i].GetData();
   const uint64 length = pearpairs[i].GetLength();

   if(gzwrite(gp, data, length) != (int64)length)
   {
    int errnum;

    MDFN_PrintError(_("Error writing to \"%s\": %m"), filename, gzerror(gp, &errnum));
    gzclose(gp);
    return(0);
   }
  }

  if(gzclose(gp) != Z_OK) // FIXME: Huhm, how should we handle this?
  {
   MDFN_PrintError(_("Error closing \"%s\""), filename);
   return(0);
  }
 }
 else
 {
  FILE *fp = fopen(filename, "wb");
  if(!fp)
  {
   MDFN_PrintError(_("Error opening \"%s\": %m"), filename, errno);
   return(0);
  }

  for(unsigned int i = 0; i < pearpairs.size(); i++)
  {
   const void *data = pearpairs[i].GetData();
   const uint64 length = pearpairs[i].GetLength();

   if(fwrite(data, 1, length, fp) != length)
   {
    MDFN_PrintError(_("Error writing to \"%s\": %m"), filename, errno);
    fclose(fp);
    return(0);
   }
  }

  fclose(fp);
 }
 return(1);
}

bool MDFN_DumpToFile(const char *filename, int compress, const std::vector<PtrLengthPair> &pearpairs)
{
 return(MDFN_DumpToFileReal(filename, compress, pearpairs));
}

bool MDFN_DumpToFile(const char *filename, int compress, const void *data, uint64 length)
{
 std::vector<PtrLengthPair> tmp_pairs;
 tmp_pairs.push_back(PtrLengthPair(data, length));
 return(MDFN_DumpToFileReal(filename, compress, tmp_pairs));
}
