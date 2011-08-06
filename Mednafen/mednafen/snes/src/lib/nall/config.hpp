#ifndef NALL_CONFIG_HPP
#define NALL_CONFIG_HPP

#include <nall/file.hpp>
#include <nall/string.hpp>
#include <nall/vector.hpp>

namespace nall {
  namespace configuration_traits {
    template<typename T> struct is_boolean { enum { value = false }; };
    template<> struct is_boolean<bool> { enum { value = true }; };

    template<typename T> struct is_signed { enum { value = false }; };
    template<> struct is_signed<signed> { enum { value = true }; };

    template<typename T> struct is_unsigned { enum { value = false }; };
    template<> struct is_unsigned<unsigned> { enum { value = true }; };

    template<typename T> struct is_double { enum { value = false }; };
    template<> struct is_double<double> { enum { value = true }; };

    template<typename T> struct is_string { enum { value = false }; };
    template<> struct is_string<string> { enum { value = true }; };
  }

  class configuration {
  public:
    enum type_t { boolean_t, signed_t, unsigned_t, double_t, string_t, unknown_t };
    struct item_t {
      uintptr_t data;
      string name;
      string desc;
      type_t type;

      string get() const {
        switch(type) {
          case boolean_t:  return string() << *(bool*)data;
          case signed_t:   return string() << *(signed*)data;
          case unsigned_t: return string() << *(unsigned*)data;
          case double_t:   return string() << *(double*)data;
          case string_t:   return string() << "\"" << *(string*)data << "\"";
        }
        return "???";
      }

      void set(string s) {
        switch(type) {
          case boolean_t:  *(bool*)data = (s == "true");      break;
          case signed_t:   *(signed*)data = strsigned(s);     break;
          case unsigned_t: *(unsigned*)data = strunsigned(s); break;
          case double_t:   *(double*)data = strdouble(s);     break;
          case string_t:   trim(s, "\""); *(string*)data = s; break;
        }
      }
    };
    vector<item_t> list;

    template<typename T>
    void attach(T &data, const char *name, const char *desc = "") {
      unsigned n = list.size();
      list[n].data = (uintptr_t)&data;
      list[n].name = name;
      list[n].desc = desc;

      if(configuration_traits::is_boolean<T>::value) list[n].type = boolean_t;
      else if(configuration_traits::is_signed<T>::value) list[n].type = signed_t;
      else if(configuration_traits::is_unsigned<T>::value) list[n].type = unsigned_t;
      else if(configuration_traits::is_double<T>::value) list[n].type = double_t;
      else if(configuration_traits::is_string<T>::value) list[n].type = string_t;
      else list[n].type = unknown_t;
    }

    virtual bool load(const char *filename) {
      string data;
      if(data.readfile(filename) == true) {
        data.replace("\r", "");
        lstring line;
        line.split("\n", data);

        for(unsigned i = 0; i < line.size(); i++) {
          int position = qstrpos(line[i], "#");
          if(position >= 0) line[i][position] = 0;
          if(qstrpos(line[i], " = ") < 0) continue;

          lstring part;
          part.qsplit(" = ", line[i]);
          trim(part[0]);
          trim(part[1]);

          for(unsigned n = 0; n < list.size(); n++) {
            if(part[0] == list[n].name) {
              list[n].set(part[1]);
              break;
            }
          }
        }

        return true;
      } else {
        return false;
      }
    }

    virtual bool save(const char *filename) const {
      file fp;
      if(fp.open(filename, file::mode_write)) {
        for(unsigned i = 0; i < list.size(); i++) {
          string output;
          output << list[i].name << " = " << list[i].get();
          if(list[i].desc != "") output << " # " << list[i].desc;
          output << "\r\n";
          fp.print(output);
        }

        fp.close();
        return true;
      } else {
        return false;
      }
    }
  };
}

#endif
