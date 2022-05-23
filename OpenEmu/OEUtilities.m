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

#import "OEUtilities.m"
#import "OEBuildVersion.h"

void OEPrintFirstResponderChain(void)
{
    NSWindow *keyWindow = [NSApp keyWindow];
    
    if(keyWindow == nil) return;
    
    NSLog(@"responders: %@", OENextRespondersFromResponder([keyWindow firstResponder]));
}

NSArray *OENextRespondersFromResponder(NSResponder *responder)
{
    if(responder == nil) return @[ ];
    
    NSMutableArray *responders = [NSMutableArray array];
    
    while(responder != nil)
    {
        [responders addObject:responder];
        responder = [responder nextResponder];
    }
    
    return responders;
}

#ifdef DebugLocalization
static NSFileHandle *OELocalizationLog = nil;
static NSMutableDictionary *OELocalizationTableLog = nil;
NSString *OELogLocalizedString(NSString *key, NSString *comment, NSString *fileName, int line, const char* function, NSString *table)
{
    NSFileHandle *handle = nil;
    if(table == nil && OELocalizationLog == nil)
    {
        NSURL *url = [NSURL fileURLWithPath:[@"~/Desktop/OpenEmu Runtime Analysis.strings" stringByExpandingTildeInPath]];
        [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
        [[NSFileManager defaultManager] createFileAtPath:[url path] contents:[NSData data] attributes:nil];
        OELocalizationLog = [NSFileHandle fileHandleForWritingToURL:url error:nil];

        NSString *version = [NSString stringWithFormat:@"/*%@*/\n", LONG_BUILD_VERSION];
        [OELocalizationLog writeData:[version dataUsingEncoding:NSUTF8StringEncoding]];
    }
    else if(table != nil && [OELocalizationTableLog objectForKey:table] == nil)
    {
        if(OELocalizationTableLog == nil) OELocalizationTableLog = [NSMutableDictionary dictionary];
        NSString *path = [NSString stringWithFormat:@"~/Desktop/OpenEmu Runtime Analysis (%@).strings", table];
        NSURL *url = [NSURL fileURLWithPath:[path stringByExpandingTildeInPath]];
        [[NSFileManager defaultManager] removeItemAtURL:url error:nil];
        [[NSFileManager defaultManager] createFileAtPath:[url path] contents:[NSData data] attributes:nil];
        NSFileHandle *h = [NSFileHandle fileHandleForWritingToURL:url error:nil];

        NSString *version = [NSString stringWithFormat:@"/*%@*/\n", LONG_BUILD_VERSION];
        [h writeData:[version dataUsingEncoding:NSUTF8StringEncoding]];

        [OELocalizationTableLog setObject:h forKey:table];
    }

    if(table == nil) handle = OELocalizationLog;
    else handle = [OELocalizationTableLog objectForKey:table];

    {
        NSString *escapedKey = [key stringByReplacingOccurrencesOfString:@"\"" withString:@"\\\""];

        NSString *value = [escapedKey copy];
        NSError *error = nil;
        NSRegularExpression *ex = [NSRegularExpression regularExpressionWithPattern:@"%[0-9]*\\.*[0-9]*[hlqLztj]*[%dDuUxXoOfeEgGcCsSpaAF@]" options:0 error:&error];
        NSArray *matches = [ex matchesInString:value options:0 range:NSMakeRange(0, [key length])];
        if([matches count] > 1)
        {
            for(NSInteger i=[matches count]; i > 0; i--)
            {
                NSTextCheckingResult *match = [matches objectAtIndex:i-1];
                NSRange range = [match range];

                NSMutableString *replacement = [NSMutableString stringWithFormat:@"%%%ld$", i];

                NSRegularExpression *ex2 = [NSRegularExpression regularExpressionWithPattern:@"[hlqLztj]*[%dDuUxXoOfeEgGcCsSpaAF@]" options:0 error:nil];
                NSRange r = [[[ex2 matchesInString:value options:0 range:range] lastObject] range];
                [replacement appendString:[value substringWithRange:r]];

                value = [value stringByReplacingCharactersInRange:range withString:replacement];
            }
        } else {
            value = key;
        }


        NSString *string = [NSString stringWithFormat:@"/*r%@%@%@%d%@%@*/ \"%@\" = \"%@\";\n",
                            OELocalizationSeparationString, fileName,
                            OELocalizationSeparationString, line,
                            OELocalizationSeparationString, comment,
                            escapedKey, value];
        NSData *data = [string dataUsingEncoding:NSUTF8StringEncoding];
        [handle writeData:data];
    }

    return [@"[L]" stringByAppendingString:NSLocalizedString(key, comment)];
}
#endif
