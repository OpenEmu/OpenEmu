#ifndef NALL_PRIORITYQUEUE_HPP
#define NALL_PRIORITYQUEUE_HPP

#include <limits>
#include <nall/function.hpp>
#include <nall/serializer.hpp>
#include <nall/utility.hpp>

namespace nall {
  template<typename type_t> void priority_queue_nocallback(type_t) {}

  //priority queue implementation using binary min-heap array;
  //does not require normalize() function.
  //O(1)     find   (tick)
  //O(log n) insert (enqueue)
  //O(log n) remove (dequeue)
  template<typename type_t> class priority_queue : noncopyable {
  public:
    inline void tick(unsigned ticks) {
      basecounter += ticks;
      while(heapsize && gte(basecounter, heap[0].counter)) callback(dequeue());
    }

    //counter is relative to current time (eg enqueue(64, ...) fires in 64 ticks);
    //counter cannot exceed std::numeric_limits<unsigned>::max() >> 1.
    void enqueue(unsigned counter, type_t event) {
      unsigned child = heapsize++;
      counter += basecounter;

      while(child) {
        unsigned parent = (child - 1) >> 1;
        if(gte(counter, heap[parent].counter)) break;

        heap[child].counter = heap[parent].counter;
        heap[child].event = heap[parent].event;
        child = parent;
      }

      heap[child].counter = counter;
      heap[child].event = event;
    }

    type_t dequeue() {
      type_t event(heap[0].event);
      unsigned parent = 0;
      unsigned counter = heap[--heapsize].counter;

      while(true) {
        unsigned child = (parent << 1) + 1;
        if(child >= heapsize) break;
        if(child + 1 < heapsize && gte(heap[child].counter, heap[child + 1].counter)) child++;
        if(gte(heap[child].counter, counter)) break;

        heap[parent].counter = heap[child].counter;
        heap[parent].event = heap[child].event;
        parent = child;
      }

      heap[parent].counter = counter;
      heap[parent].event = heap[heapsize].event;
      return event;
    }

    void reset() {
      basecounter = 0;
      heapsize = 0;
    }

    void serialize(serializer &s) {
      s.integer(basecounter);
      s.integer(heapsize);
      for(unsigned n = 0; n < heapcapacity; n++) {
        s.integer(heap[n].counter);
        s.integer(heap[n].event);
      }
    }

    priority_queue(unsigned size, function<void (type_t)> callback_ = &priority_queue_nocallback<type_t>)
    : callback(callback_) {
      heap = new heap_t[size];
      heapcapacity = size;
      reset();
    }

    ~priority_queue() {
      delete[] heap;
    }

  private:
    function<void (type_t)> callback;
    unsigned basecounter;
    unsigned heapsize;
    unsigned heapcapacity;
    struct heap_t {
      unsigned counter;
      type_t event;
    } *heap;

    //return true if x is greater than or equal to y
    inline bool gte(unsigned x, unsigned y) {
      return x - y < (std::numeric_limits<unsigned>::max() >> 1);
    }
  };
}

#endif
