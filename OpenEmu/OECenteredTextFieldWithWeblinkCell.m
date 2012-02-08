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

#import "OECenteredTextFieldWithWeblinkCell.h"

@implementation OECenteredTextFieldWithWeblinkCell

- (void)dealloc {
    self.buttonAction = NULL;
    self.buttonTarget = nil;
    
}
- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    if(self.buttonAction && self.buttonTarget)
    {
        cellFrame.size.width -= 18.0;
        
        if(!self.buttonCell)
        {
            OEImageButtonHoverPressed *btnCell = [[OEImageButtonHoverPressed alloc] initTextCell:@""];
            btnCell.splitVertically = YES;
            [btnCell setImage:[NSImage imageNamed:@"closed_weblink_arrow"]];
            btnCell.backgroundColor = nil;
            
            [self setButtonCell:btnCell];
        }
        
        NSRect buttonCellRect = NSMakeRect(cellFrame.size.width-0.0, cellFrame.origin.y-2, 20, 20);
        [self.buttonCell drawWithFrame:buttonCellRect inView:controlView];
    }
    
    [super drawInteriorWithFrame:cellFrame inView:controlView];
}
@synthesize buttonAction;
@synthesize buttonTarget;
@synthesize buttonCell;
@end
