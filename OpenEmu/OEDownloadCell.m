/*
 Copyright (c) 2010, OpenEmu Team
 
 
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

#import "OEDownloadCell.h"
#import "OEDownload.h"

@implementation OEDownloadCell

- (void)drawInteriorWithFrame:(NSRect)cellFrame inView:(NSView *)controlView
{
    OEDownload *download = [self objectValue];
    BOOL elementDisabled = NO;
    NSColor *primaryColor = ([self isHighlighted]
                             ? [NSColor alternateSelectedControlTextColor]
                             : (elementDisabled
                                ? [NSColor disabledControlTextColor]
                                : [NSColor textColor]));
    
    NSString *primaryText = [NSString stringWithFormat:@"%@ %@",
                             download.downloadTitle,
                             ([download.appcastItem title] ? : @"")];
    
    NSDictionary *primaryTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                           primaryColor                    , NSForegroundColorAttributeName,
                                           [NSFont boldSystemFontOfSize:13], NSFontAttributeName,
                                           nil];
    
    CGFloat secondColumn = cellFrame.origin.x + 80;
    CGFloat currentLine  = cellFrame.origin.y;
    
    [primaryText drawAtPoint:NSMakePoint(secondColumn, currentLine)
              withAttributes:primaryTextAttributes];
    
    currentLine += 20;
    
    NSString *secondaryText = [download downloadDescription];
    if([secondaryText length] > 0)
    {
        NSColor      *secondaryColor          = primaryColor;
        NSDictionary *secondaryTextAttributes = [NSDictionary dictionaryWithObjectsAndKeys:
                                                 secondaryColor              , NSForegroundColorAttributeName,
                                                 [NSFont systemFontOfSize:11], NSFontAttributeName,
                                                 nil];
        
        [secondaryText drawAtPoint:NSMakePoint(secondColumn, currentLine)
                    withAttributes:secondaryTextAttributes];
    }
    
    currentLine += 20;
    
    if(![download isDownloading])
    {
        NSButton *button = download.startDownloadButton;
        [controlView addSubview:button];
        [button setFrame:NSMakeRect(secondColumn, currentLine, 14, 14)];
    }
    else
    {
        [download.startDownloadButton removeFromSuperview];
        
        NSProgressIndicator *progressIndicator = download.progressBar;
        [controlView addSubview:progressIndicator];
        [progressIndicator setFocusRingType:NSFocusRingTypeNone];
        [progressIndicator setFrame:NSMakeRect(secondColumn,
                                               currentLine,
                                               cellFrame.size.width - 88,
                                               NSProgressIndicatorPreferredThickness)];
    }
    
    NSImage *icon = [[download downloadIcon] copy];
    
    if(icon != nil)
    {
        [icon setSize:NSMakeSize(64, 64)];
        [icon setFlipped:YES];
        [icon drawAtPoint:NSMakePoint(cellFrame.origin.x + 4, cellFrame.origin.y + cellFrame.size.height / 2 - 32)
                 fromRect:NSMakeRect(0, 0, 64, 64)
                operation:NSCompositeSourceOver
                 fraction:1.0f];
        [icon release];
    }
}

@end
