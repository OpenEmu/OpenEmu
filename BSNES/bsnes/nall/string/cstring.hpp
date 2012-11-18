#ifdef NALL_STRING_INTERNAL_HPP

//const string:
//bind a const char* pointer to an object that has various testing functionality;
//yet lacks the memory allocation and modification functionality of the string class

namespace nall {

cstring::operator const char*() const { return data; }
unsigned cstring::length() const { return strlen(data); }
bool cstring::operator==(const char *s) const { return !strcmp(data, s); }
bool cstring::operator!=(const char *s) const { return  strcmp(data, s); }
optional<unsigned> cstring::position (const char *key) const { return  strpos(data, key); }
optional<unsigned> cstring::iposition(const char *key) const { return istrpos(data, key); }
cstring& cstring::operator=(const char *data) { this->data = data; return *this; }
cstring::cstring(const char *data) : data(data) {}
cstring::cstring() : data("") {}

}

#endif
