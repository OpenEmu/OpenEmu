#ifdef NALL_STRING_INTERNAL_HPP

namespace nall {

static void istring(string &output) {
}

template<typename T, typename... Args>
static void istring(string &output, const T &value, Args&&... args) {
  output.append_(make_string(value));
  istring(output, std::forward<Args>(args)...);
}

void string::reserve(unsigned size_) {
  if(size_ > size) {
    size = size_;
    data = (char*)realloc(data, size + 1);
    data[size] = 0;
  }
}

bool string::empty() const {
  return !*data;
}

template<typename... Args> string& string::assign(Args&&... args) {
  *data = 0;
  istring(*this, std::forward<Args>(args)...);
  return *this;
}

template<typename... Args> string& string::append(Args&&... args) {
  istring(*this, std::forward<Args>(args)...);
  return *this;
}

string& string::assign_(const char *s) {
  unsigned length = strlen(s);
  reserve(length);
  strcpy(data, s);
  return *this;
}

string& string::append_(const char *s) {
  unsigned length = strlen(data) + strlen(s);
  reserve(length);
  strcat(data, s);
  return *this;
}

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
  if(&value == this) return *this;
  assign(value);
  return *this;
}

string& string::operator=(string &&source) {
  if(&source == this) return *this;
  if(data) free(data);
  size = source.size;
  data = source.data;
  source.data = nullptr;
  source.size = 0;
  return *this;
}

template<typename... Args> string::string(Args&&... args) {
  size = 64;
  data = (char*)malloc(size + 1);
  *data = 0;
  istring(*this, std::forward<Args>(args)...);
}

string::string(const string &value) {
  if(&value == this) return;
  size = strlen(value);
  data = strdup(value);
}

string::string(string &&source) {
  if(&source == this) return;
  size = source.size;
  data = source.data;
  source.data = nullptr;
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

optional<unsigned> lstring::find(const char *key) const {
  for(unsigned i = 0; i < size(); i++) {
    if(operator[](i) == key) return { true, i };
  }
  return { false, 0 };
}

string lstring::concatenate(const char *separator) const {
  string output;
  for(unsigned i = 0; i < size(); i++) {
    output.append(operator[](i), i < size() - 1 ? separator : "");
  }
  return output;
}

bool lstring::operator==(const lstring &source) const {
  if(this == &source) return true;
  if(size() != source.size()) return false;
  for(unsigned n = 0; n < size(); n++) {
    if(operator[](n) != source[n]) return false;
  }
  return true;
}

bool lstring::operator!=(const lstring &source) const {
  return !operator==(source);
}

inline lstring::lstring() {
}

inline lstring::lstring(std::initializer_list<string> list) {
  for(auto &data : list) append(data);
}

}

#endif
