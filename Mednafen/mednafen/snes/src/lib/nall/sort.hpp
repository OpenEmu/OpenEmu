#ifndef NALL_SORT_HPP
#define NALL_SORT_HPP

#include <nall/utility.hpp>

//class:   merge sort
//average: O(n log n)
//worst:   O(n log n)
//memory:  O(n)
//stack:   O(log n)
//stable?: yes

//notes:
//there are two primary reasons for choosing merge sort
//over the (usually) faster quick sort*:
//1: it is a stable sort.
//2: it lacks O(n^2) worst-case overhead.
//(* which is also O(n log n) in the average case.)

namespace nall {
  template<typename T>
  void sort(T list[], unsigned length) {
    if(length <= 1) return;  //nothing to sort

    //use insertion sort to quickly sort smaller blocks
    if(length < 64) {
      for(unsigned i = 0; i < length; i++) {
        unsigned min = i;
        for(unsigned j = i + 1; j < length; j++) {
          if(list[j] < list[min]) min = j;
        }
        if(min != i) swap(list[i], list[min]);
      }
      return;
    }

    //split list in half and recursively sort both
    unsigned middle = length / 2;
    sort(list, middle);
    sort(list + middle, length - middle);

    //left and right are sorted here; perform merge sort
    T *buffer = new T[length];
    unsigned offset = 0;
    unsigned left   = 0;
    unsigned right  = middle;
    while(left < middle && right < length) {
      if(list[left] < list[right]) {
        buffer[offset++] = list[left++];
      } else {
        buffer[offset++] = list[right++];
      }
    }
    while(left  < middle) buffer[offset++] = list[left++];
    while(right < length) buffer[offset++] = list[right++];

    for(unsigned i = 0; i < length; i++) list[i] = buffer[i];
    delete[] buffer;
  }
}

#endif
