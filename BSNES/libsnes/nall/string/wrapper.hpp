#ifndef NALL_STRING_WRAPPER_HPP
#define NALL_STRING_WRAPPER_HPP

namespace nall {

bool string::wildcard(const char *str) const { return nall::wildcard(data, str); }
bool string::iwildcard(const char *str) const { return nall::iwildcard(data, str); }

}

#endif

