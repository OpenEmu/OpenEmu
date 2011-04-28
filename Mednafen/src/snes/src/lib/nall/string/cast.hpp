#ifndef NALL_STRING_CAST_HPP
#define NALL_STRING_CAST_HPP

namespace nall {
  //this is needed, as C++98 does not support explicit template specialization inside classes;
  //redundant memory allocation should hopefully be avoided via compiler optimizations.
  template<> inline string to_string<bool>         (bool v)          { return v ? "true" : "false"; }
  template<> inline string to_string<signed int>   (signed int v)    { return strsigned(v); }
  template<> inline string to_string<unsigned int> (unsigned int v)  { return strunsigned(v); }
  template<> inline string to_string<double>       (double v)        { return strdouble(v); }
  template<> inline string to_string<char*>        (char *v)         { return v; }
  template<> inline string to_string<const char*>  (const char *v)   { return v; }
  template<> inline string to_string<string>       (string v)        { return v; }
  template<> inline string to_string<const string&>(const string &v) { return v; }

  template<typename T> string& string::operator= (T value) { return assign(to_string<T>(value)); }
  template<typename T> string& string::operator<<(T value) { return append(to_string<T>(value)); }

  template<typename T> lstring& lstring::operator<<(T value) {
    operator[](size()).assign(to_string<T>(value));
    return *this;
  }

  #if defined(QT_CORE_LIB)
  template<> inline string to_string<const QString&>(const QString &v) { return v.toUtf8().constData(); }
  string::operator QString() const { return QString::fromUtf8(*this); }
  #endif
};

#endif
