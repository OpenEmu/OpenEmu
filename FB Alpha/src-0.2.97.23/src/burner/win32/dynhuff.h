//
//  Dynamic Huffman Encoder/Decoder
//
//  implemented by Gangta
//
//-----------------------------------------------------------
//
//  Version History
//
//  version 0.1  - first release
//  version 0.1a - fixed a bug where the bit remainder in
//                 the EncodeBuffer() function didn't reset
//                 to 32 if encoding is restarted before
//                 program exit
//  version 0.1b - fixed right shift
//               - fixed some utility functions
//
//  version 0.1c - fixed PrintResult()
//               - fixed a fatal bug in CorrectDHT() 15% slower
//               - optimized compression/decompression 95% faster
//               - overall 80% faster :)
//
//  version 0.2  - optimized even more, now O(lg 2) :)
//
//-----------------------------------------------------------
//
//  Memory usage:
//
//     128 KB for both encoding/decoding
//
//     I tested buffer size 16K, 32K, 64K, 128K, etc
//     The performance started to drop after 512K,
//     so I chose 128K.
//
//  Comments:
//
//     I tried to optimize the speed as best I can in my
//     knowledge.
//
//     It has buffer overflow protection for encoding and
//     decoding.
//
//     It has a frequency reset function which prevents
//     frequency overflow.
//
//     It can compress text files as well as any type of
//     binary files.  The compression is not as good as
//     zlib, or bzlibb2, but it supports "on the fly"
//     encoding/decoding, and I believe it is faster than
//     bzlibb2.
//
//
///////////////////////////////////////////////////////////////

#ifndef _DYNHUFF_H_
#define _DYNHUFF_H_

#include <stdio.h>

#define MAX_FREQ ((unsigned int)0xFFFFFFFF) // max unsigned int
#define MAX_BUFFER_LEN 32768     // 4 B * 32768  = 128 KB
#define MAX_LIST_LEN 512

//#
//#             Buffer Handling
//#
//##############################################################################

// 1 if decoding is finished, 0 otherwise
extern int end_of_buffer;

// takes a data and compress it to the buffer
void EncodeBuffer(unsigned char data);

// decompresses and returns the next data from buffer
unsigned char DecodeBuffer();

//#
//#             File I/O
//#
//##############################################################################

// takes a decompressed file and compresses the whole thing into another file
int Compress(char *d_file_name, char *c_file_name);

// takes a compressed file and decompresses the whole thing into another file
int Decompress(char *c_file_name, char *d_file_name);

// returns 0 if the compressed file is opened successfully, 1 otherwise
int OpenCompressedFile(char *file_name, char *mode);

// Always returns 0
int EmbedCompressedFile(FILE *pEmb, int nOffset);

// returns 0 if the decompressed file is opened successfully, 1 otherwise
int OpenDecompressedFile(char *file_name, char *mode);

// loads the compressed file into c_buffer
// - use it only if the compressed file was opened successfully
void LoadCompressedFile();

// writes whatever is left in c_buffer to the compressed file,
// and closes the compressed file
// - use this if the compressed file was opened in 'write' mode
void WriteCompressedFile();

// writes whatever is left in d_buffer to the decompressed file,
// and closes the decompressed file
// - use this if the decompressed file was opened in 'write' mode
void WriteDecompressedFile(int bytes_remain);

// closes the compressed file
// - use this if the compressed file was opened in 'read' mode
void CloseCompressedFile();

// closes the compressed file
// - use this if the decompressed file was opened in 'read' mode
void CloseDecompressedFile();

//#
//#             Utility Functions
//#
//##############################################################################

// print the encoded buffer (upto the current buffer item)
void PrintBuffer(); // use this while encoding

// print all nodes while reverse level traversing
void PrintFreqTraverse(); // use this before DestroyDHT() happens

// print the tree in breath order (rotated -90 degrees)
void PrintTree(); // use this before DestroyDHT() happens

// print compression result
void PrintResult(); // use this after finish encoding

#endif // end of '#ifndef _DYNHUFF_H_'
