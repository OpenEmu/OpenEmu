#include "ui-base.hpp"
#include "resource.rcc"

#if defined(PLATFORM_X)
  #include "platform/platform_x.cpp"
  const char Style::Monospace[64] = "Monospace";
#elif defined(PLATFORM_OSX)
  #include "platform/platform_osx.cpp"
  const char Style::Monospace[64] = "Courier New";
#elif defined(PLATFORM_WIN)
  #include "platform/platform_win.cpp"
  const char Style::Monospace[64] = "Lucida Console";
#else
  #error "unsupported platform"
#endif

#include "config.cpp"
#include "interface.cpp"

const char defaultStylesheet[] =
  "#backdrop {"
  "  background: #000000;"
  "}\n";

#include "application/application.cpp"
#include "link/filter.cpp"
#include "link/reader.cpp"
#include "utility/utility.cpp"

//override filename's path with filepath, but only if filepath isn't empty
//used for GUI's "path selection" functionality
string filepath(const char *filename, const char *filepath) {
  if(!filepath || !*filepath) return filename;
  return string() << dir(filepath) << notdir(filename);
}

int main(int argc, char **argv) {
  return application.main(argc, argv);
}
