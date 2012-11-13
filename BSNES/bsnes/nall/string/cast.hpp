#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

//convert any (supported) type to a const char* without constructing a new nall::string
//this is used inside istring(...) to build nall::string values
template<typename T> struct stringify;

// base types

template<> struct stringify<bool> {
  bool value;
  operator const char*() const { return value ? "true" : "false"; }
  stringify(bool value) : value(value) {}
};

template<> struct stringify<char> {
  char data[256];
  operator const char*() const { return data; }
  stringify(char value) { integer(data, value); }
};

// signed integers

template<> struct stringify<signed char> {
  char data[256];
  operator const char*() const { return data; }
  stringify(signed char value) { integer(data, value); }
};

template<> struct stringify<signed short> {
  char data[256];
  operator const char*() const { return data; }
  stringify(signed short value) { integer(data, value); }
};

template<> struct stringify<signed int> {
  char data[256];
  operator const char*() const { return data; }
  stringify(signed int value) { integer(data, value); }
};

template<> struct stringify<signed long> {
  char data[256];
  operator const char*() const { return data; }
  stringify(signed long value) { integer(data, value); }
};

template<> struct stringify<signed long long> {
  char data[256];
  operator const char*() const { return data; }
  stringify(signed long long value) { integer(data, value); }
};

template<unsigned bits> struct stringify<int_t<bits>> {
  char data[256];
  operator const char*() const { return data; }
  stringify(int_t<bits> value) { integer(data, value); }
};

// unsigned integers

template<> struct stringify<unsigned char> {
  char data[256];
  operator const char*() const { return data; }
  stringify(unsigned char value) { decimal(data, value); }
};

template<> struct stringify<unsigned short> {
  char data[256];
  operator const char*() const { return data; }
  stringify(unsigned short value) { decimal(data, value); }
};

template<> struct stringify<unsigned int> {
  char data[256];
  operator const char*() const { return data; }
  stringify(unsigned int value) { decimal(data, value); }
};

template<> struct stringify<unsigned long> {
  char data[256];
  operator const char*() const { return data; }
  stringify(unsigned long value) { decimal(data, value); }
};

template<> struct stringify<unsigned long long> {
  char data[256];
  operator const char*() const { return data; }
  stringify(unsigned long long value) { decimal(data, value); }
};

template<unsigned bits> struct stringify<uint_t<bits>> {
  char data[256];
  operator const char*() const { return data; }
  stringify(uint_t<bits> value) { decimal(data, value); }
};

// floating-point

template<> struct stringify<float> {
  char data[256];
  operator const char*() const { return data; }
  stringify(float value) { fp(data, value); }
};

template<> struct stringify<double> {
  char data[256];
  operator const char*() const { return data; }
  stringify(double value) { fp(data, value); }
};

template<> struct stringify<long double> {
  char data[256];
  operator const char*() const { return data; }
  stringify(long double value) { fp(data, value); }
};

// strings

template<> struct stringify<char*> {
  const char *value;
  operator const char*() const { return value; }
  stringify(char *value) : value(value) {}
};

template<> struct stringify<const char*> {
  const char *value;
  operator const char*() const { return value; }
  stringify(const char *value) : value(value) {}
};

template<> struct stringify<string> {
  const string &value;
  operator const char*() const { return value; }
  stringify(const string &value) : value(value) {}
};

template<> struct stringify<const string&> {
  const string &value;
  operator const char*() const { return value; }
  stringify(const string &value) : value(value) {}
};

template<> struct stringify<cstring> {
  const char *value;
  operator const char*() const { return value; }
  stringify(const cstring &value) : value(value) {}
};

template<> struct stringify<const cstring&> {
  const char *value;
  operator const char*() const { return value; }
  stringify(const cstring &value) : value(value) {}
};

#if defined(QSTRING_H)

template<> struct stringify<QString> {
  const QString &value;
  operator const char*() const { return value.toUtf8().constData(); }
  stringify(const QString &value) : value(value) {}
};

template<> struct stringify<const QString&> {
  const QString &value;
  operator const char*() const { return value.toUtf8().constData(); }
  stringify(const QString &value) : value(value) {}
};

string::operator QString() const {
  return QString::fromUtf8(*this);
}

#endif

//

template<typename T> stringify<T> make_string(T value) {
  return stringify<T>(std::forward<T>(value));
}

}

#endif
