#ifndef NALL_STRING_BSV_HPP
#define NALL_STRING_BSV_HPP

//BSV parser
//version 0.01

namespace nall {

inline string bsv_decode(const char *input) {
  string output;
  unsigned offset = 0;
  while(*input) {
    //illegal characters
    if(*input == '}' ) return "";
    if(*input == '\r') return "";
    if(*input == '\n') return "";

    //normal characters
    if(*input != '{') { output[offset++] = *input++;  continue; }

    //entities
    if(strbegin(input, "{lf}")) { output[offset++] = '\n'; input += 4; continue; }
    if(strbegin(input, "{lb}")) { output[offset++] = '{';  input += 4; continue; }
    if(strbegin(input, "{rb}")) { output[offset++] = '}';  input += 4; continue; }

    //illegal entities
    return "";
  }
  output[offset] = 0;
  return output;
}

inline string bsv_encode(const char *input) {
  string output;
  unsigned offset = 0;
  while(*input) {
    //illegal characters
    if(*input == '\r') return "";

    if(*input == '\n') {
      output[offset++] = '{';
      output[offset++] = 'l';
      output[offset++] = 'f';
      output[offset++] = '}';
      input++;
      continue;
    }

    if(*input == '{') {
      output[offset++] = '{';
      output[offset++] = 'l';
      output[offset++] = 'b';
      output[offset++] = '}';
      input++;
      continue;
    }

    if(*input == '}') {
      output[offset++] = '{';
      output[offset++] = 'r';
      output[offset++] = 'b';
      output[offset++] = '}';
      input++;
      continue;
    }

    output[offset++] = *input++;
  }
  output[offset] = 0;
  return output;
}

}

#endif
