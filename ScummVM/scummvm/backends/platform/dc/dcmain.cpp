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
#include <engines/engine.h>
#include <base/main.h>
#include <base/plugins.h>
#include "dc.h"
#include "icon.h"
#include "DCLauncherDialog.h"
#include <common/config-manager.h>
#include <common/memstream.h>

#include "audio/mixer_intern.h"


Icon icon;
const char *gGameName;


OSystem_Dreamcast::OSystem_Dreamcast()
  : _devpoll(0), screen(NULL), mouse(NULL), overlay(NULL), _softkbd(this),
    _ms_buf(NULL), _mixer(NULL),
    _current_shake_pos(0), _aspect_stretch(false), _softkbd_on(false),
    _softkbd_motion(0), _enable_cursor_palette(false), _screenFormat(0)
{
  memset(screen_tx, 0, sizeof(screen_tx));
  memset(mouse_tx, 0, sizeof(mouse_tx));
  memset(ovl_tx, 0, sizeof(ovl_tx));
  _fsFactory = this;
}

void OSystem_Dreamcast::initBackend()
{
  ConfMan.setInt("autosave_period", 0);
  _savefileManager = createSavefileManager();
  _timerManager = new DefaultTimerManager();

  uint sampleRate = initSound();
  _mixer = new Audio::MixerImpl(this, sampleRate);
  _mixer->setReady(true);

  _audiocdManager = new DCCDManager();

  EventsBaseBackend::initBackend();
}


/* CD Audio */
static bool find_track(int track, int &first_sec, int &last_sec)
{
  struct TOC *toc = cdfs_gettoc();
  if (!toc)
    return false;
  int i, first, last;
  first = TOC_TRACK(toc->first);
  last = TOC_TRACK(toc->last);
  if (first < 1 || last > 99 || first > last)
    return false;
  for (i=first; i<=last; i++)
    if (!(TOC_CTRL(toc->entry[i-1])&4))
      if (track==1) {
	first_sec = TOC_LBA(toc->entry[i-1]);
	last_sec = TOC_LBA(toc->entry[i]);
	return true;
      } else
	--track;
  return false;
}

void DCCDManager::playCD(int track, int num_loops, int start_frame, int duration)
{
  int first_sec, last_sec;
#if 1
  if (num_loops)
    --num_loops;
#endif
  if (num_loops>14) num_loops=14;
  else if (num_loops<0) num_loops=15; // infinity
  if (!find_track(track, first_sec, last_sec))
    return;
  if (duration)
    last_sec = first_sec + start_frame + duration;
  first_sec += start_frame;
  play_cdda_sectors(first_sec, last_sec, num_loops);
}

void DCCDManager::stopCD()
{
  stop_cdda();
}

bool DCCDManager::pollCD()
{
  extern int getCdState();
  return getCdState() == 3;
}

void DCCDManager::updateCD()
{
  // Dummy.  The CD drive takes care of itself.
}

bool DCCDManager::openCD(int drive)
{
  // Dummy.
  return true;
}

void OSystem_Dreamcast::setWindowCaption(const char *caption)
{
  gGameName = caption;
}

void OSystem_Dreamcast::quit() {
  (*(void(**)(int))0x8c0000e0)(0);
}

/* Mutex handling */
OSystem::MutexRef OSystem_Dreamcast::createMutex()
{
  return NULL;
}

void OSystem_Dreamcast::lockMutex(MutexRef mutex)
{
}

void OSystem_Dreamcast::unlockMutex(MutexRef mutex)
{
}

void OSystem_Dreamcast::deleteMutex(MutexRef mutex)
{
}


/* Features */
bool OSystem_Dreamcast::hasFeature(Feature f)
{
  switch(f) {
  case kFeatureAspectRatioCorrection:
  case kFeatureVirtualKeyboard:
  case kFeatureOverlaySupportsAlpha:
  case kFeatureCursorPalette:
    return true;
  default:
    return false;
  }
}

void OSystem_Dreamcast::setFeatureState(Feature f, bool enable)
{
  switch(f) {
  case kFeatureAspectRatioCorrection:
    _aspect_stretch = enable;
    if (screen)
      setScaling();
    break;
  case kFeatureVirtualKeyboard:
    _softkbd_on = enable;
    break;
  case kFeatureCursorPalette:
    _enable_cursor_palette = enable;
    break;
  default:
    break;
  }
}

bool OSystem_Dreamcast::getFeatureState(Feature f)
{
  switch(f) {
  case kFeatureAspectRatioCorrection:
    return _aspect_stretch;
  case kFeatureVirtualKeyboard:
    return _softkbd_on;
  case kFeatureCursorPalette:
    return _enable_cursor_palette;
  default:
    return false;
  }
}

void OSystem_Dreamcast::getTimeAndDate(TimeDate &td) const {
  time_t curTime;
  time(&curTime);
  struct tm t = *localtime(&curTime);
  td.tm_sec = t.tm_sec;
  td.tm_min = t.tm_min;
  td.tm_hour = t.tm_hour;
  td.tm_mday = t.tm_mday;
  td.tm_mon = t.tm_mon;
  td.tm_year = t.tm_year;
  td.tm_wday = t.tm_wday;
}

Common::SeekableReadStream *OSystem_Dreamcast::createConfigReadStream() {
  Common::FSNode file("/scummvm.ini");
  Common::SeekableReadStream *s = file.createReadStream();
  return s? s : new Common::MemoryReadStream((const byte *)"", 0);
}

Common::WriteStream *OSystem_Dreamcast::createConfigWriteStream() {
  return 0;
}

void OSystem_Dreamcast::logMessage(LogMessageType::Type type, const char *message) {
#ifndef NOSERIAL
  report(message);
#endif
}

namespace DC_Flash {
  static int syscall_info_flash(int sect, int *info)
  {
    return (*(int (**)(int, void*, int, int))0x8c0000b8)(sect,info,0,0);
  }

  static int syscall_read_flash(int offs, void *buf, int cnt)
  {
    return (*(int (**)(int, void*, int, int))0x8c0000b8)(offs,buf,cnt,1);
  }

  static int flash_crc(const char *buf, int size)
  {
    int i, c, n = -1;
    for(i=0; i<size; i++) {
      n ^= (buf[i]<<8);
      for(c=0; c<8; c++)
	if(n & 0x8000)
	  n = (n << 1) ^ 4129;
	else
	  n <<= 1;
    }
    return (unsigned short)~n;
  }

  static int flash_read_sector(int partition, int sec, unsigned char *dst)
  {
    int s, r, n, b, bmb, got=0;
    int info[2];
    char buf[64];
    char bm[64];

    if((r = syscall_info_flash(partition, info))<0)
      return r;

    if((r = syscall_read_flash(info[0], buf, 64))<0)
      return r;

    if(memcmp(buf, "KATANA_FLASH", 12) ||
       buf[16] != partition || buf[17] != 0)
      return -2;

    n = (info[1]>>6)-1-((info[1] + 0x7fff)>>15);
    bmb = n+1;
    for(b = 0; b < n; b++) {
      if(!(b&511)) {
	if((r = syscall_read_flash(info[0] + (bmb++ << 6), bm, 64))<0)
	  return r;
      }
      if(!(bm[(b>>3)&63] & (0x80>>(b&7))))
	if((r = syscall_read_flash(info[0] + ((b+1) << 6), buf, 64))<0)
	  return r;
	else if((s=*(unsigned short *)(buf+0)) == sec &&
		flash_crc(buf, 62) == *(unsigned short *)(buf+62)) {
	  memcpy(dst+(s-sec)*60, buf+2, 60);
	  got=1;
	}
    }
    return got;
  }

  static int get_locale_setting()
  {
    unsigned char data[60];
    if (flash_read_sector(2,5,data) == 1)
      return data[5];
    else
      return -1;
  }
} // End of namespace DC_Flash

Common::String OSystem_Dreamcast::getSystemLanguage() const {
  static const char *languages[] = {
    "ja_JP",
    "en_US",
    "de_DE",
    "fr_FR",
    "es_ES",
    "it_IT"
  };
  int l = DC_Flash::get_locale_setting();
  if (l<0 || ((unsigned)l)>=sizeof(languages)/sizeof(languages[0]))
    l = 1;
  return Common::String(languages[l]);
}


void DCHardware::dc_init_hardware()
{
#ifndef NOSERIAL
  serial_init(57600);
  usleep(2000000);
  printf("Serial OK\r\n");
#endif

  cdfs_init();
  maple_init();
  dc_setup_ta();
  init_arm();
}

static OSystem_Dreamcast osys_dc;

int main()
{
  static const char *argv[] = { "scummvm", NULL, };
  static int argc = 1;

  g_system = &osys_dc;

#ifdef DYNAMIC_MODULES
  PluginManager::instance().addPluginProvider(&osys_dc);
#endif

  scummvm_main(argc, argv);

  g_system->quit();
}

int DCLauncherDialog::runModal()
{
  char *base = NULL, *dir = NULL;
  Common::Language language = Common::UNK_LANG;
  Common::Platform platform = Common::kPlatformUnknown;

  if (!selectGame(base, dir, language, platform, icon))
    g_system->quit();

  // Set the game path.
  ConfMan.addGameDomain(base);
  if (dir != NULL)
    ConfMan.set("path", dir, base);

  // Set the game language.
  if (language != Common::UNK_LANG)
    ConfMan.set("language", Common::getLanguageCode(language), base);

  // Set the game platform.
  if (platform != Common::kPlatformUnknown)
    ConfMan.set("platform", Common::getPlatformCode(platform), base);

  // Set the target.
  ConfMan.setActiveDomain(base);

  return 0;
}
