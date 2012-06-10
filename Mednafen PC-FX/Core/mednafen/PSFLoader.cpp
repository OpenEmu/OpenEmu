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
/*
 TODO:
	Time string parsing convenience functions.

	Character set autodetect heuristics and conversion for when the "utf8" tag is missing.
*/
#include "mednafen.h"
#include "PSFLoader.h"
#include "mednafen-endian.h"
#include "general.h"

#include <limits.h>
#include <trio/trio.h>
#include <ctype.h>
//#include <iconv.h>


PSFTags::PSFTags()
{


}

PSFTags::~PSFTags()
{

}

void PSFTags::AddTag(char *tag_line)
{
 char *eq;

 // Transform 0x01-0x1F -> 0x20
 for(unsigned int i = 0; i < strlen(tag_line); i++)
  if((unsigned char)tag_line[i] < 0x20)
   tag_line[i] = 0x20;

 eq = strchr(tag_line, '=');

 if(eq)
 {
  *eq = 0;

  MDFN_trim(tag_line);
  MDFN_trim(eq + 1);

  for(unsigned int i = 0; i < strlen(tag_line); i++)
   tag_line[i] = tolower(tag_line[i]);

  if(TagExists(tag_line))
   tags[tag_line] = tags[std::string(tag_line)] + std::string(1, '\n') + std::string(eq + 1);
  else
   tags[tag_line] = std::string(eq + 1);
 }
}

#if 0
static const char *DetectCharset(const uint8 *data, const uint32 data_size)
{
 static const char *TestCharsets[] = { "UTF-8", /*"SJIS",*/ "WINDOWS-1252" };

 for(unsigned int i = 0; i < sizeof(TestCharsets) / sizeof(TestCharsets[0]); i++)
 {
  iconv_t cd;

  cd = iconv_open("UTF-32", TestCharsets[i]);
  if(cd != (iconv_t)-1)
  {
   size_t in_len = data_size;
   size_t out_len = data_size * 4 + 4;
   char *in_ptr = (char *)data;
   char *const out_ptr_mem = new char[out_len];
   char *out_ptr = out_ptr_mem;

   if(iconv(cd, (ICONV_CONST char **)&in_ptr, &in_len, &out_ptr, &out_len) != (size_t)-1)
   { 
    delete[] out_ptr_mem;
    return(TestCharsets[i]);
   }
   delete[] out_ptr_mem;
  }
 }

 return(NULL);
}
#endif

void PSFTags::LoadTags(const uint8 *data_in, uint32 size)
{
 std::vector<char> tags_heap;
 char *data;
 char *spos;
 //const char *detected_charset = DetectCharset(data_in, size);

 tags_heap.resize(size + 1);
 tags_heap[size] = 0;

 memcpy(&tags_heap[0], data_in, size);

 data = &tags_heap[0];
 spos = data;

 while(size)
 {
  if(*data == 0x0A || *data == 0x00)
  {
   *data = 0;

   if(data - spos)
   {
    if(*(data - 1) == 0xD)	// handle \r
     *(data - 1) = 0;

    AddTag(spos);
   }

   spos = data + 1;	// Skip \n for next tag
  }

  size--;
  data++;
 }

}

int64 PSFTags::GetTagI(const char *name)
{
 std::map<std::string, std::string>::iterator it;

 it = tags.find(name);
 if(it != tags.end())
 {
  long long ret = 0;
  std::string &tmp = tags[name];

  trio_sscanf(tmp.c_str(), "%lld", &ret);

  return(ret);
 }
 return(0);	// INT64_MIN
}

bool PSFTags::TagExists(const char *name)
{
 if(tags.find(name) != tags.end())
  return(true);

 return(false);
}


std::string PSFTags::GetTag(const char *name)
{
 std::map<std::string, std::string>::iterator it;

 it = tags.find(name);

 if(it != tags.end())
  return(it->second);

 return("");
}

void PSFTags::EraseTag(const char *name)
{
 std::map<std::string, std::string>::iterator it;

 it = tags.find(name);
 if(it != tags.end())
  tags.erase(it);
}

PSFLoader::PSFLoader()
{


}

PSFLoader::~PSFLoader()
{


}

bool PSFLoader::TestMagic(uint8 version, MDFNFILE *fp)
{
 if(fp->size < (3 + 1 + 4 + 4 + 4))
  return(false);

 if(memcmp(fp->data, "PSF", 3))
  return(false);

 if(fp->data[3] != version)
  return(false);

 return(true);
}

PSFTags PSFLoader::LoadInternal(uint8 version, uint32 max_exe_size, MDFNFILE *fp, uint32 level, bool force_ignore_pcsp)
{
 uint32 reserved_size, compressed_size, compressed_crc32;
 bool _lib_present = false;
 PSFTags tags;

 std::vector<uint8> decompress_buffer;
 uLongf decompress_len;

 if(!TestMagic(version, fp))
  throw(MDFN_Error(0, _("Not a PSF(version=0x%02x) file!"), version));

 reserved_size = MDFN_de32lsb(fp->data + 4);
 compressed_size = MDFN_de32lsb(fp->data + 8);
 compressed_crc32 = MDFN_de32lsb(fp->data + 12);

 if(fp->size < (16 + reserved_size + compressed_size))
  throw(MDFN_Error(0, _("PSF is missing at least %u bytes of data!"), 16 + reserved_size + compressed_size - fp->size));

 if(crc32(0, fp->data + 16 + reserved_size, compressed_size) != compressed_crc32)
  throw(MDFN_Error(0, _("PSF compressed CRC32 mismatch(data is corrupt)!")));


 {
  const uint8 *tag_section = fp->data + 16 + reserved_size + compressed_size;
  uint32 tag_section_size = fp->size - 16 - reserved_size - compressed_size;

  if(tag_section_size > 5 && !memcmp(tag_section, "[TAG]", 5))
   tags.LoadTags(tag_section + 5, tag_section_size - 5);
 }

 //
 // Handle minipsf simple _lib
 //

 if(level < 15)
 {
  if(tags.TagExists("_lib"))
  {
   std::string tp = tags.GetTag("_lib");

   if(!MDFN_IsFIROPSafe(tp))
   {
    throw(MDFN_Error(0, _("Referenced path \"%s\" is potentially unsafe.  See \"filesys.untrusted_fip_check\" setting."), tp.c_str()));
   }

   MDFNFILE subfile(MDFN_MakeFName(MDFNMKF_AUX, 0, tp.c_str()).c_str(), NULL, NULL);

   LoadInternal(version, max_exe_size, &subfile, level + 1);

   _lib_present = true;
  }
 }

 //
 //
 //

 decompress_buffer.resize(max_exe_size);
 decompress_len = max_exe_size;
 switch( uncompress((Bytef *)&decompress_buffer[0], &decompress_len, (const Bytef *)(fp->data + 16 + reserved_size), compressed_size) )
 {
  default:
	throw(MDFN_Error(0, "zlib unknown error"));

  case Z_OK: break;

  case Z_MEM_ERROR:
	throw(MDFN_Error(0, "zlib Z_MEM_ERROR"));

  case Z_BUF_ERROR:
	throw(MDFN_Error(0, _("PSF decompressed size exceeds maximum allowed!")));

  case Z_DATA_ERROR:
	throw(MDFN_Error(0, _("PSF compressed data is bad.")));
 }

 HandleReserved(fp->data + 16, reserved_size);
 HandleEXE(&decompress_buffer[0], decompress_len, force_ignore_pcsp | _lib_present);
 decompress_buffer.resize(0);

 //
 // handle libN
 //
 if(level < 15)
 {
  for(unsigned int n = 2; n <= INT_MAX; n++)
  {
   char tmpbuf[32];

   trio_snprintf(tmpbuf, 32, "_lib%d", (int)n);

   if(tags.TagExists(tmpbuf))
   {
    MDFNFILE subfile(MDFN_MakeFName(MDFNMKF_AUX, 0, tags.GetTag(tmpbuf).c_str()).c_str(), NULL, NULL);

    LoadInternal(version, max_exe_size, &subfile, level + 1, true);
   }
   else
    break;   
  }
 }

 return(tags);
}

PSFTags PSFLoader::Load(uint8 version, uint32 max_exe_size, MDFNFILE *fp)
{
 return(LoadInternal(version, max_exe_size, fp, 0, false));
}

void PSFLoader::HandleReserved(const uint8 *data, uint32 len)
{

}

void PSFLoader::HandleEXE(const uint8 *data, uint32 len, bool ignore_pcsp)
{

}

