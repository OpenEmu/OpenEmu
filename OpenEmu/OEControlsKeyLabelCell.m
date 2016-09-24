/*
 Copyright (c) 2011, OpenEmu Team
 
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

#import "OEControlsKeyLabelCell.h"

@implementation OEControlsKeyLabelCell

- (void)setupAttributes
{
    NSMutableDictionary *attributes = [[NSMutableDictionary alloc] init];
    
    [self setFontFamily:nil];

    NSShadow *shadow = [[NSShadow alloc] init];
    [shadow setShadowBlurRadius:1.0];
    [shadow setShadowColor:[NSColor colorWithDeviceWhite:1.0 alpha:0.25]];
    [shadow setShadowOffset:NSMakeSize(0, -1)];
    
    [attributes setObject:[NSColor colorWithDeviceWhite:0.0 alpha:1.0] forKey:NSForegroundColorAttributeName];
    [attributes setObject:shadow forKey:NSShadowAttributeName];
    
    
    NSMutableParagraphStyle *style = [[NSMutableParagraphStyle alloc] init];
    [style setAlignment:NSRightTextAlignment];
    [attributes setObject:style forKey:NSParagraphStyleAttributeName];
    
    self.textAttributes = attributes;
    
    [super setupAttributes];
}

- (void)setFontFamily:(NSString *)aFontFamily
{
    NSFontManager *fontManager = [NSFontManager sharedFontManager];
    NSFont *font = [fontManager fontWithFamily:aFontFamily traits:NSFontBoldTrait weight:9.0 size:11.0];
    if(!font) {
        font = [NSFont boldSystemFontOfSize:11];
    }

    [self setFont:font];
    _fontFamily = aFontFamily;
}

@end
