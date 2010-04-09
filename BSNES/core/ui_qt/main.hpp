#define UNICODE
#define QT_NO_DEBUG
#define QT_CORE_LIB
#define QT_GUI_LIB
#define QT_THREAD_SUPPORT

#include <QApplication>
#include <QtGui>
//Q_IMPORT_PLUGIN(QJpegPlugin)
//Q_IMPORT_PLUGIN(QMngPlugin)

#include <../base.hpp>

#include <nall/base64.hpp>
#include <nall/config.hpp>
#include <nall/input.hpp>
#include <nall/ups.hpp>
using namespace nall;

#include <ruby/ruby.hpp>
using namespace ruby;

#include "config.hpp"
#include "interface.hpp"
#include "application/application.moc"
#include "debugger/debugger.moc"
#include "input/input.hpp"
#include "link/filter.hpp"
#include "link/reader.hpp"
#include "utility/utility.hpp"

struct Style {
  static const char Monospace[];

  enum {
    WindowMargin     = 5,
    WidgetSpacing    = 5,
    SeparatorSpacing = 5,
  };
};

#if defined(PLATFORM_X)
  const char Style::Monospace[] = "Monospace";
#elif defined(PLATFORM_WIN)
  const char Style::Monospace[] = "Lucida Console";
#else
  const char Style::Monospace[] = "Courier New";
#endif
