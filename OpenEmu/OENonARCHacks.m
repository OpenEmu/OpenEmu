//
//  OENonARCHacks.m
//  OpenEmu
//
//  Created by Alexander Strange on 2/7/12.
//  Copyright (c) 2012 Apple Inc. All rights reserved.
//

#import "OESidebarOutlineView.h"

#if 0

@implementation OESidebarOutlineView (OEPrivate)

- (void)setupOutlineCell{
    // Analyzer warns about leaking object here, that is not the case.
    // We release the current instance variable and replace the pointer
    // so NSOutlineView should release our cell twice in its dealloc
    
    // This probably breaks with ARC!
    
    OESidebarOutlineButtonCell *sidebarOutlineCell = [[OESidebarOutlineButtonCell alloc] init];
    [sidebarOutlineCell retain];
    
    void *currentCell;
    object_getInstanceVariable(self, "_outlineCell", &currentCell);
    [(id)currentCell release];
    object_getInstanceVariable(self, "_trackingOutlineCell", &currentCell);
    [(id)currentCell release];
    
    object_setInstanceVariable(self, "_outlineCell", sidebarOutlineCell);
    object_setInstanceVariable(self, "_trackingOutlineCell", sidebarOutlineCell);
    
    // Read note above!
}

@end

#endif