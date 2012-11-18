#ifndef NALL_COMPOSITOR_HPP
#define NALL_COMPOSITOR_HPP

#include <nall/intrinsics.hpp>

namespace nall {

struct compositor {
  inline static bool enabled();
  inline static bool enable(bool status);

  #if defined(PLATFORM_X)
  enum class Compositor : unsigned { Unknown, Metacity, Xfwm4 };
  inline static Compositor detect();

  inline static bool enabled_metacity();
  inline static bool enable_metacity(bool status);

  inline static bool enabled_xfwm4();
  inline static bool enable_xfwm4(bool status);
  #endif
};

#if defined(PLATFORM_X)

//Metacity

bool compositor::enabled_metacity() {
  FILE *fp = popen("gconftool-2 --get /apps/metacity/general/compositing_manager", "r");
  if(fp == 0) return false;

  char buffer[512];
  if(fgets(buffer, sizeof buffer, fp) == 0) return false;

  if(!memcmp(buffer, "true", 4)) return true;
  return false;
}

bool compositor::enable_metacity(bool status) {
  FILE *fp;
  if(status) {
    fp = popen("gconftool-2 --set --type bool /apps/metacity/general/compositing_manager true", "r");
  } else {
    fp = popen("gconftool-2 --set --type bool /apps/metacity/general/compositing_manager false", "r");
  }
  if(fp == 0) return false;
  pclose(fp);
  return true;
}

//Xfwm4

bool compositor::enabled_xfwm4() {
  FILE *fp = popen("xfconf-query -c xfwm4 -p '/general/use_compositing'", "r");
  if(fp == 0) return false;

  char buffer[512];
  if(fgets(buffer, sizeof buffer, fp) == 0) return false;

  if(!memcmp(buffer, "true", 4)) return true;
  return false;
}

bool compositor::enable_xfwm4(bool status) {
  FILE *fp;
  if(status) {
    fp = popen("xfconf-query -c xfwm4 -p '/general/use_compositing' -t 'bool' -s 'true'", "r");
  } else {
    fp = popen("xfconf-query -c xfwm4 -p '/general/use_compositing' -t 'bool' -s 'false'", "r");
  }
  if(fp == 0) return false;
  pclose(fp);
  return true;
}

//General

compositor::Compositor compositor::detect() {
  Compositor result = Compositor::Unknown;

  FILE *fp;
  char buffer[512];

  fp = popen("pidof metacity", "r");
  if(fp && fgets(buffer, sizeof buffer, fp)) result = Compositor::Metacity;
  pclose(fp);

  fp = popen("pidof xfwm4", "r");
  if(fp && fgets(buffer, sizeof buffer, fp)) result = Compositor::Xfwm4;
  pclose(fp);

  return result;
}

bool compositor::enabled() {
  switch(detect()) {
  case Compositor::Metacity: return enabled_metacity();
  case Compositor::Xfwm4: return enabled_xfwm4();
  default: return false;
  }
}

bool compositor::enable(bool status) {
  switch(detect()) {
  case Compositor::Metacity: return enable_metacity(status);
  case Compositor::Xfwm4: return enable_xfwm4(status);
  default: return false;
  }
}

#elif defined(PLATFORM_WINDOWS)

bool compositor::enabled() {
  HMODULE module = GetModuleHandleW(L"dwmapi");
  if(module == 0) module = LoadLibraryW(L"dwmapi");
  if(module == 0) return false;

  auto pDwmIsCompositionEnabled = (HRESULT (WINAPI*)(BOOL*))GetProcAddress(module, "DwmIsCompositionEnabled");
  if(pDwmIsCompositionEnabled == 0) return false;

  BOOL result;
  if(pDwmIsCompositionEnabled(&result) != S_OK) return false;
  return result;
}

bool compositor::enable(bool status) {
  HMODULE module = GetModuleHandleW(L"dwmapi");
  if(module == 0) module = LoadLibraryW(L"dwmapi");
  if(module == 0) return false;

  auto pDwmEnableComposition = (HRESULT (WINAPI*)(UINT))GetProcAddress(module, "DwmEnableComposition");
  if(pDwmEnableComposition == 0) return false;

  if(pDwmEnableComposition(status) != S_OK) return false;
  return true;
}

#else

bool compositor::enabled() {
  return false;
}

bool compositor::enable(bool) {
  return false;
}

#endif

}

#endif
