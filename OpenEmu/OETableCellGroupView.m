//
//  OETableCellGroupView.m
//  OpenEmu
//
//  Created by Christoph Leimbrock on 20/08/14.
//
//

#import "OETableCellGroupView.h"

#import "OETheme.h"
#import "NSImage+OEDrawingAdditions.h"

@implementation OETableCellGroupView

- (instancetype)initWithFrame:(NSRect)frame {
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
    }
    return self;
}

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];

    const NSRect bounds = [self bounds];
    NSImage *backgroundImage = [[OETheme sharedTheme] imageForKey:@"table_header_background" forState:OEThemeStateDefault];
    [backgroundImage setMatchesOnlyOnBestFittingAxisWithoutCrashing:YES];
    [backgroundImage drawInRect:bounds fromRect:NSZeroRect operation:NSCompositeCopy fraction:1.0 respectFlipped:YES hints:nil];
}

@end
