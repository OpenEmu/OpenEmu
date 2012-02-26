#ifndef NALL_STRING_CORE_HPP
#define NALL_STRING_CORE_HPP

namespace nall {

void string::reserve(unsigned size_) {
  if(size_ > size) {
    size = size_;
    data = (char*)realloc(data, size + 1);
    data[size] = 0;
  }
}

unsigned string::length() const {
  return strlen(data);
}

bool string::empty() const {
  return !*data;
}

string& string::assign(const char *s) {
  unsigned length = strlen(s);
  reserve(length);
  strcpy(data, s);
  return *this;
}

string& string::append(const char *s) {
  unsigned length = strlen(data) + strlen(s);
  reserve(length);
  strcat(data, s);
  return *this;
}

string& string::append(bool value) { append(value ? "true" : "false"); return *this; }
string& string::append(signed int value) { append(integer(value)); return *this; }
string& string::append(unsigned int value) { append(decimal(value)); return *this; }
string& string::append(double value) { append(fp(value)); return *this; }

bool string::beginswith(const char *str) const { return strstr(data, str) == data; }

string::operator const char*() const {
  return data;
}

char* string::operator()() {
  return data;
}

char& string::operator[](int index) {
  reserve(index);
  return data[index];
}

bool string::operator==(const char *str) const { return strcmp(data, str) == 0; }
bool string::operator!=(const char *str) const { return strcmp(data, str) != 0; }
bool string::operator< (const char *str) const { return strcmp(data, str)  < 0; }
bool string::operator<=(const char *str) const { return strcmp(data, str) <= 0; }
bool string::operator> (const char *str) const { return strcmp(data, str)  > 0; }
bool string::operator>=(const char *str) const { return strcmp(data, str) >= 0; }

string& string::operator=(const string &value) {
  assign(value);
  return *this;
}

static void istring(string & output){}

template <typename T1>
static void istring(string & output, T1 const & v1)
{ output.append(v1); istring(output); }

template <typename T1,
          typename T2>
static void istring(string & output, T1 const & v1,
                                     T2 const & v2)
{ output.append(v1); istring(output, v2); }

template <typename T1,
          typename T2,
          typename T3>
static void istring(string & output, T1 const & v1,
                                     T2 const & v2,
                                     T3 const & v3)
{ output.append(v1); istring(output, v2, v3); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4>
static void istring(string & output, T1 const & v1,
                                     T2 const & v2,
                                     T3 const & v3,
                                     T4 const & v4)
{ output.append(v1); istring(output, v2, v3, v4); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5>
static void istring(string & output, T1 const & v1,
                                     T2 const & v2,
                                     T3 const & v3,
                                     T4 const & v4,
                                     T5 const & v5)
{ output.append(v1); istring(output, v2, v3, v4, v5); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6>
static void istring(string & output, T1 const & v1,
                                     T2 const & v2,
                                     T3 const & v3,
                                     T4 const & v4,
                                     T5 const & v5,
                                     T6 const & v6)
{ output.append(v1); istring(output, v2, v3, v4, v5, v6); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7>
static void istring(string & output, T1 const & v1,
                                     T2 const & v2,
                                     T3 const & v3,
                                     T4 const & v4,
                                     T5 const & v5,
                                     T6 const & v6,
                                     T7 const & v7)
{ output.append(v1); istring(output, v2, v3, v4, v5, v6, v7); }

template <typename T1,
          typename T2,
          typename T3,
          typename T4,
          typename T5,
          typename T6,
          typename T7,
          typename T8>
static void istring(string & output, T1 const & v1,
                                     T2 const & v2,
                                     T3 const & v3,
                                     T4 const & v4,
                                     T5 const & v5,
                                     T6 const & v6,
                                     T7 const & v7,
                                     T8 const & v8)
{ output.append(v1); istring(output, v2, v3, v4, v5, v6, v7, v8); }

string::string(){
  size = 64;
  data = (char*)malloc(size + 1);
  *data = 0;
}

template<typename T1>
string::string(T1 const & v1) : size(64)
{ data = (char*)malloc(size + 1); *data = 0; istring(*this, v1); }

template<typename T1,
         typename T2>
string::string(T1 const & v1,
               T2 const & v2) : size(64)
{ data = (char*)malloc(size + 1); *data = 0; istring(*this, v1, v2); }

template<typename T1,
         typename T2,
         typename T3>
string::string(T1 const & v1,
               T2 const & v2,
               T3 const & v3) : size(64)
{ data = (char*)malloc(size + 1); *data = 0; istring(*this, v1, v2, v3); }

template<typename T1,
         typename T2,
         typename T3,
         typename T4>
string::string(T1 const & v1,
               T2 const & v2,
               T3 const & v3,
               T4 const & v4) : size(64)
{ data = (char*)malloc(size + 1); *data = 0; istring(*this, v1, v2, v3, v4); }

template<typename T1,
         typename T2,
         typename T3,
         typename T4,
         typename T5>
string::string(T1 const & v1,
               T2 const & v2,
               T3 const & v3,
               T4 const & v4,
               T5 const & v5) : size(64)
{ data = (char*)malloc(size + 1); *data = 0; istring(*this, v1, v2, v3, v4, v5); }

template<typename T1,
         typename T2,
         typename T3,
         typename T4,
         typename T5,
         typename T6>
string::string(T1 const & v1,
               T2 const & v2,
               T3 const & v3,
               T4 const & v4,
               T5 const & v5,
               T6 const & v6) : size(64)
{ data = (char*)malloc(size + 1); *data = 0; istring(*this, v1, v2, v3, v4, v5, v6); }

template<typename T1,
         typename T2,
         typename T3,
         typename T4,
         typename T5,
         typename T6,
         typename T7>
string::string(T1 const & v1,
               T2 const & v2,
               T3 const & v3,
               T4 const & v4,
               T5 const & v5,
               T6 const & v6,
               T7 const & v7) : size(64)
{ data = (char*)malloc(size + 1); *data = 0; istring(*this, v1, v2, v3, v4, v5, v6, v7); }

template<typename T1,
         typename T2,
         typename T3,
         typename T4,
         typename T5,
         typename T6,
         typename T7,
         typename T8>
string::string(T1 const & v1,
               T2 const & v2,
               T3 const & v3,
               T4 const & v4,
               T5 const & v5,
               T6 const & v6,
               T7 const & v7,
               T8 const & v8) : size(64)
{ data = (char*)malloc(size + 1); *data = 0; istring(*this, v1, v2, v3, v4, v5, v6, v7, v8); }

string::string(const string &value) {
  size = strlen(value);
  data = strdup(value);
}

string::~string() {
  if(data) free(data);
}

bool string::readfile(const string &filename) {
  assign("");

  #if !defined(_WIN32)
  FILE *fp = fopen(filename, "rb");
  #else
  FILE *fp = _wfopen(utf16_t(filename), L"rb");
  #endif
  if(!fp) return false;

  fseek(fp, 0, SEEK_END);
  unsigned size = ftell(fp);
  rewind(fp);
  char *fdata = new char[size + 1];
  unsigned unused = fread(fdata, 1, size, fp);
  fclose(fp);
  fdata[size] = 0;
  assign(fdata);
  delete[] fdata;

  return true;
}

optional<unsigned> lstring::find(const char *key) {
  for(unsigned i = 0; i < size(); i++) {
    if(operator[](i) == key) return optional<unsigned>(true, i);
  }
  return optional<unsigned>(false, 0);
}

inline lstring::lstring() {
}

inline lstring::lstring(const lstring & str) {
  for(int i = 0; i < str.size(); i++) operator<<(str[i]);
}

inline lstring::lstring(const string & v1){
  operator<<(v1);
}
inline lstring::lstring(const string & v1, const string & v2){
  operator<<(v1); operator<<(v2);
}
inline lstring::lstring(const string & v1, const string & v2, const string & v3){
  operator<<(v1); operator<<(v2); operator<<(v3);
}
inline lstring::lstring(const string & v1, const string & v2, const string & v3, const string & v4){
  operator<<(v1); operator<<(v2); operator<<(v3); operator<<(v4);
}
inline lstring::lstring(const string & v1, const string & v2, const string & v3, const string & v4, const string & v5){
  operator<<(v1); operator<<(v2); operator<<(v3); operator<<(v4);
  operator<<(v5);
}
inline lstring::lstring(const string & v1, const string & v2, const string & v3, const string & v4, const string & v5, const string & v6){
  operator<<(v1); operator<<(v2); operator<<(v3); operator<<(v4);
  operator<<(v5); operator<<(v6);
}
inline lstring::lstring(const string & v1, const string & v2, const string & v3, const string & v4, const string & v5, const string & v6, const string & v7){
  operator<<(v1); operator<<(v2); operator<<(v3); operator<<(v4);
  operator<<(v5); operator<<(v6); operator<<(v7);
}
inline lstring::lstring(const string & v1, const string & v2, const string & v3, const string & v4, const string & v5, const string & v6, const string & v7, const string & v8){
  operator<<(v1); operator<<(v2); operator<<(v3); operator<<(v4);
  operator<<(v5); operator<<(v6); operator<<(v7); operator<<(v8);
}

inline lstring::lstring(std::initializer_list<string> list) {
  for(const string *s = list.begin(); s != list.end(); ++s) {
    operator<<(*s);
  }
}

}

#endif
