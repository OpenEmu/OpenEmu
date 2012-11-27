/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.

 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 */

#define FORBIDDEN_SYMBOL_ALLOW_ALL

#include <common/scummsys.h>
#include "engines/engine.h"
#include "dc.h"
#include "icon.h"
#include <common/savefile.h>
#include <gui/gui-manager.h>
#include <gui/message.h>
#include <common/zlib.h>


// Savegame can not be bigger than this
#define MAX_SAVE_SIZE (128*1024)


enum vmsaveResult {
  VMSAVE_OK,
  VMSAVE_NOVM,
  VMSAVE_NOSPACE,
  VMSAVE_WRITEERROR
};


static int lastvm=-1;

static void displaySaveResult(vmsaveResult res)
{
  char buf[1024];

  switch(res) {
  case VMSAVE_OK:
    sprintf(buf, "Game saved on unit %c%d", 'A'+(lastvm/6), lastvm%6);
    break;
  case VMSAVE_NOVM:
    strcpy(buf, "No memory card present!");
    break;
  case VMSAVE_NOSPACE:
    strcpy(buf, "Not enough space available!");
    break;
  case VMSAVE_WRITEERROR:
    strcpy(buf, "Write error!!!");
    break;
  default:
    strcpy(buf, "Unknown error!!!");
    break;
  }

  GUI::MessageDialog dialog(buf);
  dialog.runModal();
}

static vmsaveResult trySave(const char *gamename, const char *data, int size,
			    const char *filename, class Icon &icon, int vm)
{
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;
  struct vms_file_header header;
  struct timestamp tstamp;
  struct tm tm;
  time_t t;
  unsigned char iconbuffer[512+32];

  if (!vmsfs_check_unit(vm, 0, &info))
    return VMSAVE_NOVM;
  if (!vmsfs_get_superblock(&info, &super))
    return VMSAVE_NOVM;
  int free_cnt = vmsfs_count_free(&super);
  if (vmsfs_open_file(&super, filename, &file))
    free_cnt += file.blks;
  if (((128+512+size+511)>>9) > free_cnt)
    return VMSAVE_NOSPACE;

  memset(&header, 0, sizeof(header));
  strncpy(header.shortdesc, "ScummVM savegame", 16);
  strncpy(header.longdesc, gamename, 32);
  strncpy(header.id, "ScummVM", 16);
  icon.create_vmicon(iconbuffer);
  header.numicons = 1;
  memcpy(header.palette, iconbuffer, sizeof(header.palette));
  time(&t);
  tm = *localtime(&t);
  tstamp.year = tm.tm_year+1900;
  tstamp.month = tm.tm_mon+1;
  tstamp.day = tm.tm_mday;
  tstamp.hour = tm.tm_hour;
  tstamp.minute = tm.tm_min;
  tstamp.second = tm.tm_sec;
  tstamp.wkday = (tm.tm_wday+6)%7;

  vmsfs_beep(&info, 1);

  vmsfs_errno = 0;
  if (!vmsfs_create_file(&super, filename, &header,
			iconbuffer+sizeof(header.palette), NULL,
			data, size, &tstamp)) {
    fprintf(stderr, "%s\n", vmsfs_describe_error());
    vmsfs_beep(&info, 0);
    return VMSAVE_WRITEERROR;
  }

  vmsfs_beep(&info, 0);
  return VMSAVE_OK;
}

static bool tryLoad(char *&buffer, int &size, const char *filename, int vm)
{
  struct vmsinfo info;
  struct superblock super;
  struct vms_file file;

  if (!vmsfs_check_unit(vm, 0, &info))
    return false;
  if (!vmsfs_get_superblock(&info, &super))
    return false;
  if (!vmsfs_open_file(&super, filename, &file))
    return false;

  buffer = new char[size = file.size];

  if (vmsfs_read_file(&file, (unsigned char *)buffer, size))
    return true;

  delete[] buffer;
  buffer = NULL;
  return false;
}

static bool tryDelete(const char *filename, int vm)
{
  struct vmsinfo info;
  struct superblock super;

  if (!vmsfs_check_unit(vm, 0, &info))
    return false;
  if (!vmsfs_get_superblock(&info, &super))
    return false;

  if (!vmsfs_delete_file(&super, filename))
    return false;

  return true;
}

static bool matches(const char *glob, const char *name)
{
  while(*glob)
    if(*glob == '*') {
      while(*glob == '*')
	glob++;
      do {
	if((*name == *glob || *glob == '?') &&
	   matches(glob, name))
	  return true;
      } while(*name++);
      return false;
    } else if(!*name)
      return false;
    else if(*glob == '?' || *glob == *name) {
      glob++;
      name++;
    }
    else
      return false;
  return !*name;
}

static void tryList(const char *glob, int vm, Common::StringArray &list)
{
  struct vmsinfo info;
  struct superblock super;
  struct dir_iterator iter;
  struct dir_entry de;

  if (!vmsfs_check_unit(vm, 0, &info))
    return;
  if (!vmsfs_get_superblock(&info, &super))
    return;
  vmsfs_open_dir(&super, &iter);
  while (vmsfs_next_dir_entry(&iter, &de))
    if (de.entry[0]) {
      char buf[16];
      strncpy(buf, (char *)de.entry+4, 12);
      buf[12] = 0;
      if (matches(glob, buf))
	list.push_back(buf);
    }
}

vmsaveResult writeSaveGame(const char *gamename, const char *data, int size,
			   const char *filename, class Icon &icon)
{
  vmsaveResult r, res = VMSAVE_NOVM;

  if (lastvm >= 0 &&
     (res = trySave(gamename, data, size, filename, icon, lastvm)) == VMSAVE_OK)
    return res;

  for (int i=0; i<24; i++)
    if ((r = trySave(gamename, data, size, filename, icon, i)) == VMSAVE_OK) {
      lastvm = i;
      return r;
    } else if (r > res)
      res = r;

  return res;
}

bool readSaveGame(char *&buffer, int &size, const char *filename)
{
  if (lastvm >= 0 &&
     tryLoad(buffer, size, filename, lastvm))
    return true;

  for (int i=0; i<24; i++)
    if (tryLoad(buffer, size, filename, i)) {
      lastvm = i;
      return true;
    }

  return false;
}

bool deleteSaveGame(const char *filename)
{
  if (lastvm >= 0 &&
     tryDelete(filename, lastvm))
    return true;

  for (int i=0; i<24; i++)
    if (tryDelete(filename, i)) {
      lastvm = i;
      return true;
    }

  return false;
}


class InVMSave : public Common::InSaveFile {
private:
  char *buffer;
  int _pos, _size;
  bool _eos;

  uint32 read(void *buf, uint32 cnt);
  bool skip(uint32 offset);
  bool seek(int32 offs, int whence);

public:
  InVMSave()
    : _pos(0), buffer(NULL), _eos(false)
  { }

  ~InVMSave()
  {
    delete[] buffer;
  }

  bool eos() const { return _eos; }
  void clearErr() { _eos = false; }
  int32 pos() const { return _pos; }
  int32 size() const { return _size; }

  bool readSaveGame(const char *filename)
  { return ::readSaveGame(buffer, _size, filename); }
};

class OutVMSave : public Common::OutSaveFile {
private:
  char *buffer;
  int pos, size, committed;
  char filename[16];
  bool iofailed;

public:
  uint32 write(const void *buf, uint32 cnt);

  OutVMSave(const char *_filename)
    : pos(0), committed(-1), iofailed(false)
  {
    strncpy(filename, _filename, 16);
    buffer = new char[size = MAX_SAVE_SIZE];
  }

  ~OutVMSave();

  bool err() const { return iofailed; }
  void clearErr() { iofailed = false; }
  void finalize();
};

class VMSaveManager : public Common::SaveFileManager {
public:

  virtual Common::OutSaveFile *openForSaving(const Common::String &filename, bool compress = true) {
	OutVMSave *s = new OutVMSave(filename.c_str());
	return compress ? Common::wrapCompressedWriteStream(s) : s;
  }

  virtual Common::InSaveFile *openForLoading(const Common::String &filename) {
	InVMSave *s = new InVMSave();
	if (s->readSaveGame(filename.c_str())) {
	  return Common::wrapCompressedReadStream(s);
	} else {
	  delete s;
	  return NULL;
	}
  }

  virtual bool removeSavefile(const Common::String &filename) {
	return ::deleteSaveGame(filename.c_str());
  }

  virtual Common::StringArray listSavefiles(const Common::String &pattern);
};

void OutVMSave::finalize()
{
  extern const char *gGameName;
  extern Icon icon;

  if (committed >= pos)
    return;

  char *data = buffer;
  int len = pos;

  vmsaveResult r = writeSaveGame(gGameName, data, len, filename, icon);
  committed = pos;
  if (r != VMSAVE_OK)
    iofailed = true;
  displaySaveResult(r);
}

OutVMSave::~OutVMSave()
{
  finalize();
  delete[] buffer;
}

uint32 InVMSave::read(void *buf, uint32 cnt)
{
  int nbyt = cnt;
  if (_pos + nbyt > _size) {
    cnt = (_size - _pos);
    _eos = true;
    nbyt = cnt;
  }
  if (nbyt)
    memcpy(buf, buffer + _pos, nbyt);
  _pos += nbyt;
  return cnt;
}

bool InVMSave::skip(uint32 offset)
{
  int nbyt = offset;
  if (_pos + nbyt > _size)
    nbyt = (_size - _pos);
  _pos += nbyt;
  return true;
}

bool InVMSave::seek(int32 offs, int whence)
{
  switch(whence) {
  case SEEK_SET:
    _pos = offs;
    break;
  case SEEK_CUR:
    _pos += offs;
    break;
  case SEEK_END:
    _pos = _size + offs;
    break;
  }
  if (_pos < 0)
    _pos = 0;
  else if (_pos > _size)
    _pos = _size;
  _eos = false;
  return true;
}

uint32 OutVMSave::write(const void *buf, uint32 cnt)
{
  int nbyt = cnt;
  if (pos + nbyt > size) {
    cnt = (size - pos);
    nbyt = cnt;
  }
  if (nbyt)
    memcpy(buffer + pos, buf, nbyt);
  pos += nbyt;
  return cnt;
}


Common::StringArray VMSaveManager::listSavefiles(const Common::String &pattern)
{
  Common::StringArray list;

  for (int i=0; i<24; i++)
    tryList(pattern.c_str(), i, list);

  return list;
}

Common::SaveFileManager *OSystem_Dreamcast::createSavefileManager()
{
  return new VMSaveManager();
}
