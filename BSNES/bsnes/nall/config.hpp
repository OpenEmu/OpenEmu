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

      inline string get() const {
        switch(type) {
          case boolean_t:  return { *(bool*)data };
          case signed_t:   return { *(signed*)data };
          case unsigned_t: return { *(unsigned*)data };
          case double_t:   return { *(double*)data };
          case string_t:   return { "\"", *(string*)data, "\"" };
        }
        return "???";
      }

      inline void set(string s) {
        switch(type) {
          case boolean_t:  *(bool*)data = (s == "true");     break;
          case signed_t:   *(signed*)data = integer(s);      break;
          case unsigned_t: *(unsigned*)data = decimal(s);    break;
          case double_t:   *(double*)data = fp(s);           break;
          case string_t:   s.trim("\""); *(string*)data = s; break;
        }
      }
    };
    vector<item_t> list;

    template<typename T>
    inline void append(T &data, const char *name, const char *desc = "") {
      item_t item = { (uintptr_t)&data, name, desc };
      if(configuration_traits::is_boolean<T>::value) item.type = boolean_t;
      else if(configuration_traits::is_signed<T>::value) item.type = signed_t;
      else if(configuration_traits::is_unsigned<T>::value) item.type = unsigned_t;
      else if(configuration_traits::is_double<T>::value) item.type = double_t;
      else if(configuration_traits::is_string<T>::value) item.type = string_t;
      else item.type = unknown_t;
      list.append(item);
    }

    //deprecated
    template<typename T>
    inline void attach(T &data, const char *name, const char *desc = "") {
      append(data, name, desc);
    }

    inline virtual bool load(const string &filename) {
      string data;
      if(data.readfile(filename) == true) {
        data.replace("\r", "");
        lstring line;
        line.split("\n", data);

        for(unsigned i = 0; i < line.size(); i++) {
          if(auto position = qstrpos(line[i], "#")) line[i][position()] = 0;
          if(!qstrpos(line[i], " = ")) continue;

          lstring part;
          part.qsplit(" = ", line[i]);
          part[0].trim();
          part[1].trim();

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

    inline virtual bool save(const string &filename) const {
      file fp;
      if(fp.open(filename, file::mode::write)) {
        for(unsigned i = 0; i < list.size(); i++) {
          string output;
          output.append(list[i].name, " = ", list[i].get());
          if(list[i].desc != "") output.append(" # ", list[i].desc);
          output.append("\r\n");
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
