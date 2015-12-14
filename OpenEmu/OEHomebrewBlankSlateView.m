/*
 Copyright (c) 2014, OpenEmu Team

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

#import "OEHomebrewBlankSlateView.h"
#import "OEBlankSlateSpinnerView.h"

@implementation OEHomebrewBlankSlateView

- (void)setupViewForRepresentedObject
{
    id representedObject = [self representedObject];

    NSString *text = NSLocalizedString(@"Check out some excellent homebrew games.", @"");
    if([representedObject isKindOfClass:[NSString class]])
    {
        OEBlankSlateSpinnerView *spinner = [[OEBlankSlateSpinnerView alloc] initWithFrame:NSZeroRect];
        [self setupBoxWithText:representedObject andImageView:spinner];
        [self addLeftHeadlineWithText:NSLocalizedString(@"Homebrew Games", @"")];
        [self addInformationalText:text];
    }
    else if([representedObject isKindOfClass:[NSError class]])
    {
        NSImageView *warningImageView = [[NSImageView alloc] initWithFrame:NSZeroRect];
        [warningImageView setImage:[NSImage imageNamed:@"blank_slate_warning"]];
        [warningImageView setImageScaling:NSImageScaleNone];
        [warningImageView setImageAlignment:NSImageAlignTop];
        [warningImageView unregisterDraggedTypes];

        // TODO: extract real error reason here. Make sure it's not much longer than this!
        NSString *reason = NSLocalizedString(@"No Internet Connection", @"Homebrew Blank Slate View Error Info");
        [self setupBoxWithText:reason andImageView:warningImageView];
        [self addLeftHeadlineWithText:NSLocalizedString(@"Homebrew Games", @"")];
        [self addInformationalText:text];
    }
    else
    {
        DLog(@"Unknown Represented Object!");
    }
}
@end
