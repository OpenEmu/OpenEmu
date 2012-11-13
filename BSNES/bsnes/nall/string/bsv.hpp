#ifdef NALL_STRING_INTERNAL_HPP

//BSV v1.0 parser
//revision 0.02

namespace nall {

struct BSV {
  static inline string decode(const char *input) {
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

  static inline string encode(const char *input) {
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
};

}

#endif
