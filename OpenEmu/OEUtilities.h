/*
 Copyright (c) 2012, OpenEmu Team
 
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


#import <Cocoa/Cocoa.h>

#define GENERATE_PRAGMA(x) _Pragma(#x)
#define TODO(x) GENERATE_PRAGMA(message("[TODO] " #x))
#define FIXME(x) GENERATE_PRAGMA(message("[FIXME] " #x))
#define NOTE(x) GENERATE_PRAGMA(message("[NOTE] " #x))

#define BOOL_STR(value) ((value) ? "YES" : "NO")

#ifdef DebugLocalization
    #define OELocalizationSeparationString @"|"
    #define OELocalizedString(_KEY_, _COMMENT_) OELogLocalizedString(_KEY_, _COMMENT_, [[NSString stringWithUTF8String:__FILE__] lastPathComponent], __LINE__, __FUNCTION__, nil)
    #define OELocalizedStringFromTable(_KEY_, _TABLE_, _COMMENT_) OELogLocalizedString(_KEY_, _COMMENT_, [[NSString stringWithUTF8String:__FILE__] lastPathComponent], __LINE__, __FUNCTION__, _TABLE_)

    NSString *OELogLocalizedString(NSString *key, NSString *comment, NSString *fileName, int line, const char* function, NSString *table);
#else
    #define OELocalizedString(_KEY_, _COMMENT_) NSLocalizedString(_KEY_,_COMMENT_)
    #define OELocalizedStringFromTable(_KEY_, _TABLE, _COMMENT_) NSLocalizedStringFromTable(_KEY_, _TABLE, _COMMENT_)
#endif

#define OECoreDataMainThreadAssertion() NSAssert([NSThread isMainThread], @"Only call on main thread!")
// output must be at least 2*len+1 bytes
void tohex(const unsigned char *input, size_t len, char *output);
void OEPrintFirstResponderChain(void);
NSArray *OENextRespondersFromResponder(NSResponder *responder);
NSString *temporaryDirectoryForDecompressionOfPath(NSString *aPath);
bool GetSystemVersion( int *major, int *minor, int *bugfix );


