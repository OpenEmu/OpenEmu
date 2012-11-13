#ifndef NALL_STRING_HPP
#define NALL_STRING_HPP

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <algorithm>
#include <initializer_list>

#include <nall/atoi.hpp>
#include <nall/function.hpp>
#include <nall/platform.hpp>
#include <nall/sha256.hpp>
#include <nall/stdint.hpp>
#include <nall/utility.hpp>
#include <nall/varint.hpp>
#include <nall/vector.hpp>

#include <nall/windows/utf8.hpp>

#define NALL_STRING_INTERNAL_HPP
#include <nall/string/base.hpp>
#include <nall/string/bml.hpp>
#include <nall/string/bsv.hpp>
#include <nall/string/cast.hpp>
#include <nall/string/compare.hpp>
#include <nall/string/convert.hpp>
#include <nall/string/core.hpp>
#include <nall/string/cstring.hpp>
#include <nall/string/filename.hpp>
#include <nall/string/math-fixed-point.hpp>
#include <nall/string/math-floating-point.hpp>
#include <nall/string/platform.hpp>
#include <nall/string/strm.hpp>
#include <nall/string/strpos.hpp>
#include <nall/string/trim.hpp>
#include <nall/string/replace.hpp>
#include <nall/string/split.hpp>
#include <nall/string/utf8.hpp>
#include <nall/string/utility.hpp>
#include <nall/string/variadic.hpp>
#include <nall/string/wildcard.hpp>
#include <nall/string/wrapper.hpp>
#include <nall/string/xml.hpp>
#undef NALL_STRING_INTERNAL_HPP

#endif
