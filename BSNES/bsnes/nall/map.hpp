#ifndef NALL_MAP_HPP
#define NALL_MAP_HPP

#include <nall/vector.hpp>

namespace nall {

template<typename LHS, typename RHS>
struct map {
  struct pair {
    LHS name;
    RHS data;
  };

  inline void reset() {
    list.reset();
  }

  inline unsigned size() const {
    return list.size();
  }

  //O(log n) find
  inline optional<unsigned> find(const LHS &name) const {
    signed first = 0, last = size() - 1;
    while(first <= last) {
      signed middle = (first + last) / 2;
           if(name < list[middle].name) last  = middle - 1;  //search lower half
      else if(list[middle].name < name) first = middle + 1;  //search upper half
      else return { true, (unsigned)middle };                //match found
    }
    return { false, 0u };
  }

  //O(n) insert + O(log n) find
  inline RHS& insert(const LHS &name, const RHS &data) {
    if(auto position = find(name)) {
      list[position()].data = data;
      return list[position()].data;
    }
    signed offset = size();
    for(unsigned n = 0; n < size(); n++) {
      if(name < list[n].name) { offset = n; break; }
    }
    list.insert(offset, { name, data });
    return list[offset].data;
  }

  //O(log n) find
  inline void modify(const LHS &name, const RHS &data) {
    if(auto position = find(name)) list[position()].data = data;
  }

  //O(n) remove + O(log n) find
  inline void remove(const LHS &name) {
    if(auto position = find(name)) list.remove(position());
  }

  //O(log n) find
  inline RHS& operator[](const LHS &name) {
    if(auto position = find(name)) return list[position()].data;
    throw;
  }

  inline const RHS& operator[](const LHS &name) const {
    if(auto position = find(name)) return list[position()].data;
    throw;
  }

  inline RHS& operator()(const LHS &name) {
    if(auto position = find(name)) return list[position()].data;
    return insert(name, RHS());
  }

  inline const RHS& operator()(const LHS &name, const RHS &data) const {
    if(auto position = find(name)) return list[position()].data;
    return data;
  }

  inline pair* begin() { return list.begin(); }
  inline pair* end() { return list.end(); }
  inline const pair* begin() const { return list.begin(); }
  inline const pair* end() const { return list.end(); }

protected:
  vector<pair> list;
};

template<typename LHS, typename RHS>
struct bidirectional_map {
  const map<LHS, RHS> &lhs;
  const map<RHS, LHS> &rhs;

  inline void reset() {
    llist.reset();
    rlist.reset();
  }

  inline unsigned size() const {
    return llist.size();
  }

  inline void insert(const LHS &ldata, const RHS &rdata) {
    llist.insert(ldata, rdata);
    rlist.insert(rdata, ldata);
  }

  inline bidirectional_map() : lhs(llist), rhs(rlist) {}

protected:
  map<LHS, RHS> llist;
  map<RHS, LHS> rlist;
};

}

#endif
