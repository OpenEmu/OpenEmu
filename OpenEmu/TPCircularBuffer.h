//
//  TPCircularBuffer.h
//  Circular/Ring buffer implementation
//
//  Created by Michael Tyson on 10/12/2011.
//  Copyright 2011 A Tasty Pixel. All rights reserved.
//
//
//  This implementation makes use of a virtual memory mapping technique that inserts a virtual copy
//  of the buffer memory directly after the buffer's end, negating the need for any buffer wrap-around
//  logic. Clients can simply use the returned memory address as if it were contiguous space.
//  
//  The implementation is thread-safe in the case of a single producer and single consumer.
//
//  Virtual memory technique originally proposed by Philip Howard (http://vrb.slashusr.org/), and
//  adapted to Darwin by Kurt Revis (http://www.snoize.com,
//  http://www.snoize.com/Code/PlayBufferedSoundFile.tar.gz)
//
//  MIT license:
// 
//  Permission is hereby granted, free of charge, to any person obtaining a copy of this software and 
//  associated documentation files (the "Software"), to deal in the Software without restriction, 
//  including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, 
//  and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, 
//  subject to the following conditions:
//  
//  The above copyright notice and this permission notice shall be included in all copies or substantial 
//  portions of the Software.
//  
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT 
//  LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. 
//  IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
//  WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE 
//  SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#include <libkern/OSAtomic.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif
    
    typedef struct {
        void             *buffer;
        int32_t           length;
        int32_t           tail;
        int32_t           head;
        volatile int32_t  fillCount;
    } TPCircularBuffer;
    
    bool  TPCircularBufferInit(TPCircularBuffer *buffer, int32_t length);
    void  TPCircularBufferCleanup(TPCircularBuffer *buffer);
    void  TPCircularBufferClear(TPCircularBuffer *buffer);
    
    // Reading (consuming)
    
    static __inline__ __attribute__((always_inline)) void* TPCircularBufferTail(TPCircularBuffer *buffer, int32_t* availableBytes) {
        *availableBytes = buffer->fillCount;
        return (void*)((char*)buffer->buffer + buffer->tail);
    }
    
    static __inline__ __attribute__((always_inline)) void TPCircularBufferConsume(TPCircularBuffer *buffer, int32_t amount) {
        buffer->tail = (buffer->tail + amount) % buffer->length;
        OSAtomicAdd32Barrier(-amount, &buffer->fillCount);
    }
    
    static __inline__ __attribute__((always_inline)) void* TPCircularBufferHead(TPCircularBuffer *buffer, int32_t* availableBytes) {
        *availableBytes = (buffer->length - buffer->fillCount);
        return (void*)((char*)buffer->buffer + buffer->head);
    }
    
    // Writing (producing)
    
    static __inline__ __attribute__((always_inline)) void TPCircularBufferProduce(TPCircularBuffer *buffer, int amount) {
        buffer->head = (buffer->head + amount) % buffer->length;
        OSAtomicAdd32Barrier(amount, &buffer->fillCount);
    }
    
    static __inline__ __attribute__((always_inline)) int TPCircularBufferProduceBytes(TPCircularBuffer *buffer, const void* src, int32_t len) {
        int32_t space;
        void *ptr = TPCircularBufferHead(buffer, &space);
        int copied = len > space ? space : len;
        memcpy(ptr, src, copied);
        TPCircularBufferProduce(buffer, copied);
        return copied;
    }
    
#ifdef __cplusplus
}
#endif