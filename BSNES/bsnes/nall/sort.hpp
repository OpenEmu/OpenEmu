#ifndef NALL_SORT_HPP
#define NALL_SORT_HPP

#include <algorithm>
#include <nall/utility.hpp>

//class:   merge sort
//average: O(n log n)
//worst:   O(n log n)
//memory:  O(n)
//stack:   O(log n)
//stable?: yes

//note: merge sort was chosen over quick sort, because:
//* it is a stable sort
//* it lacks O(n^2) worst-case overhead

#define NALL_SORT_INSERTION
//#define NALL_SORT_SELECTION

namespace nall {
  template<typename T, typename Comparator>
  void sort(T list[], unsigned size, const Comparator &lessthan) {
    if(size <= 1) return;  //nothing to sort

    //use insertion sort to quickly sort smaller blocks
    if(size < 64) {
      #if defined(NALL_SORT_INSERTION)
      for(signed i = 1, j; i < size; i++) {
        T copy = std::move(list[i]);
        for(j = i - 1; j >= 0; j--) {
          if(lessthan(list[j], copy)) break;
          list[j + 1] = std::move(list[j]);
        }
        list[j + 1] = std::move(copy);
      }
      #elif defined(NALL_SORT_SELECTION)
      for(unsigned i = 0; i < size; i++) {
        unsigned min = i;
        for(unsigned j = i + 1; j < size; j++) {
          if(lessthan(list[j], list[min])) min = j;
        }
        if(min != i) std::swap(list[i], list[min]);
      }
      #endif
      return;
    }

    //split list in half and recursively sort both
    unsigned middle = size / 2;
    sort(list, middle, lessthan);
    sort(list + middle, size - middle, lessthan);

    //left and right are sorted here; perform merge sort
    T *buffer = new T[size];
    unsigned offset = 0, left = 0, right = middle;
    while(left < middle && right < size) {
      if(lessthan(list[left], list[right])) {
        buffer[offset++] = std::move(list[left++]);
      } else {
        buffer[offset++] = std::move(list[right++]);
      }
    }
    while(left < middle) buffer[offset++] = std::move(list[left++]);
    while(right < size)  buffer[offset++] = std::move(list[right++]);

    for(unsigned i = 0; i < size; i++) list[i] = std::move(buffer[i]);
    delete[] buffer;
  }

  template<typename T>
  void sort(T list[], unsigned size) {
    return sort(list, size, [](const T &l, const T &r) { return l < r; });
  }
}

#endif
