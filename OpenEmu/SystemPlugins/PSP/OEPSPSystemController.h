/*
 Copyright (c) 2013, OpenEmu Team
 
 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright
       notice, this list of conditions and the following disclaimer in the
       documentation and/or other materials provided with the distribution.
     * Neither the name of the OpenEmu Team nor the
       names of its contributors may be used to endorse or promote products
       derived from this software without specific prior written permission.
 
 THIS SOFTWARE IS PROVIDED BY OpenEmu Team ''AS IS'' AND ANY
 EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL OpenEmu Team BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
  SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

@import OpenEmuSystem;

// complessed ISO(9660) header format
typedef struct ciso_header
{
    unsigned char magic[4];             /* +00 : 'C','I','S','O'                 */
    uint32_t header_size;               /* +04 : header size (==0x18)            */
    unsigned long long total_bytes;     /* +08 : number of original data size    */
    uint32_t block_size;                /* +10 : number of compressed block size */
    unsigned char ver;                  /* +14 : version 01                      */
    unsigned char align;                /* +15 : align of index value            */
    unsigned char rsv_06[2];            /* +16 : reserved                        */
#if 0
    // INDEX BLOCK
    unsigned int index[0];              /* +18 : block[0] index                  */
    unsigned int index[1];              /* +1C : block[1] index                  */
    :
    :
    unsigned int index[last];           /* +?? : block[last]                     */
    unsigned int index[last+1];         /* +?? : end of last data point          */
    // DATA BLOCK
    unsigned char data[];               /* +?? : compressed or plain sector data */
#endif
}CISO_H;

OE_EXPORTED_CLASS
@interface OEPSPSystemController : OESystemController

@end
