#ifndef NALL_STRING_REPLACE_HPP
#define NALL_STRING_REPLACE_HPP

namespace nall {

string& string::replace(const char *key, const char *token) {
  int i, z, ksl = strlen(key), tsl = strlen(token), ssl = length();
  unsigned int replace_count = 0, size = ssl;
  char *buffer;

  if(ksl <= ssl) {
    if(tsl > ksl) {                      //the new string may be longer than the old string...
      for(i = 0; i <= ssl - ksl;) {      //so let's find out how big of a string we'll need...
        if(!memcmp(data + i, key, ksl)) {
          replace_count++;
          i += ksl;
        } else i++;
      }
      size = ssl + ((tsl - ksl) * replace_count);
      reserve(size);
    }

    buffer = new char[size + 1];
    for(i = z = 0; i < ssl;) {
      if(i <= ssl - ksl) {
        if(!memcmp(data + i, key, ksl)) {
          memcpy(buffer + z, token, tsl);
          z += tsl;
          i += ksl;
        } else buffer[z++] = data[i++];
      } else buffer[z++] = data[i++];
    }
    buffer[z] = 0;

    assign(buffer);
    delete[] buffer;
  }

  return *this;
}

string& string::qreplace(const char *key, const char *token) {
  int i, l, z, ksl = strlen(key), tsl = strlen(token), ssl = length();
  unsigned int replace_count = 0, size = ssl;
  uint8_t x;
  char *buffer;

  if(ksl <= ssl) {
    if(tsl > ksl) {
      for(i = 0; i <= ssl - ksl;) {
        x = data[i];
        if(x == '\"' || x == '\'') {
          l = i;
          i++;
          while(data[i++] != x) {
            if(i == ssl) {
              i = l;
              break;
            }
          }
        }
        if(!memcmp(data + i, key, ksl)) {
          replace_count++;
          i += ksl;
        } else i++;
      }
      size = ssl + ((tsl - ksl) * replace_count);
      reserve(size);
    }

    buffer = new char[size + 1];
    for(i = z = 0; i < ssl;) {
      x = data[i];
      if(x == '\"' || x == '\'') {
        l = i++;
        while(data[i] != x && i < ssl)i++;
        if(i >= ssl)i = l;
        else {
          memcpy(buffer + z, data + l, i - l);
          z += i - l;
        }
      }
      if(i <= ssl - ksl) {
        if(!memcmp(data + i, key, ksl)) {
          memcpy(buffer + z, token, tsl);
          z += tsl;
          i += ksl;
          replace_count++;
        } else buffer[z++] = data[i++];
      } else buffer[z++] = data[i++];
    }
    buffer[z] = 0;

    assign(buffer);
    delete[] buffer;
  }

  return *this;
}

};

#endif
