#ifndef NALL_DICTIONARY_HPP
#define NALL_DICTIONARY_HPP

#include <nall/array.hpp>
#include <nall/string.hpp>
#include <nall/utility.hpp>

namespace nall {
  class dictionary : noncopyable {
  public:
    string operator[](const char *input) {
      for(unsigned i = 0; i < index_input.size(); i++) {
        if(index_input[i] == input) return index_output[i];
      }

      //no match, use input; remove input identifier, if one exists
      if(strbegin(input, "{{")) {
        int pos = strpos(input, "}}");
        if(pos >= 0) {
          string temp = substr(input, pos + 2);
          return temp;
        }
      }

      return input;
    }

    bool import(const char *filename) {
      string data;
      if(data.readfile(filename) == false) return false;
      ltrim_once(data, "\xef\xbb\xbf"); //remove UTF-8 marker, if it exists
      data.replace("\r", "");

      lstring line;
      line.split("\n", data);
      for(unsigned i = 0; i < line.size(); i++) {
        lstring part;
        //format: "Input" = "Output"
        part.qsplit("=", line[i]);
        if(part.size() != 2) continue;

        //remove whitespace
        trim(part[0]);
        trim(part[1]);

        //remove quotes
        trim_once(part[0], "\"");
        trim_once(part[1], "\"");

        unsigned n = index_input.size();
        index_input[n]  = part[0];
        index_output[n] = part[1];
      }

      return true;
    }

    void reset() {
      index_input.reset();
      index_output.reset();
    }

    ~dictionary() {
      reset();
    }

  protected:
    lstring index_input;
    lstring index_output;
  };
}

#endif
