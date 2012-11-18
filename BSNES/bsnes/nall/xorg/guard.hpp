#ifndef NALL_XORG_GUARD_HPP
#define NALL_XORG_GUARD_HPP

#define None
#undef XlibNone
#define XlibNone 0L
#define Button1 XlibButton1
#define Button2 XlibButton2
#define Button3 XlibButton3
#define Button4 XlibButton4
#define Button5 XlibButton5
#define Display XlibDisplay
#define Screen XlibScreen
#define Window XlibWindow

#else
#undef NALL_XORG_GUARD_HPP

#undef None
#undef Button1
#undef Button2
#undef Button3
#undef Button4
#undef Button5
#undef Display
#undef Screen
#undef Window

#endif
